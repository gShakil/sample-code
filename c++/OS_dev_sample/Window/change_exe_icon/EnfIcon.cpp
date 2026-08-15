// EnfIcon.cpp : 프로그램/exe 아이콘 변경 라이브러리 구현.
//

#include "pch.h"
#include "framework.h"
#include "EnfIcon.h"
#include "resource.h"
#include <shlobj.h>  // SHChangeNotify (셸 아이콘 캐시 갱신)
#include <vector>
#include <cassert> // std::assert
#define assertm(exp, msg) assert(((void)msg, exp))

#pragma comment(lib, "Shell32.lib")

#pragma pack(push, 1)
typedef struct
{
	BYTE    bWidth;
	BYTE    bHeight;
	BYTE    bColorCount;
	BYTE    bReserved;
	WORD    wPlanes;
	WORD    wBitCount;
	DWORD   dwBytesInRes;
	DWORD   dwImageOffset; // 이 필드는 ICO 파일에서만 사용됩니다.
} ICONDIRENTRY;

typedef struct
{
	WORD           idReserved;
	WORD           idType;
	WORD           idCount;
	ICONDIRENTRY   idEntries[1];
} ICONDIR;

typedef struct
{
	BYTE    bWidth;
	BYTE    bHeight;
	BYTE    bColorCount;
	BYTE    bReserved;
	WORD    wPlanes;
	WORD    wBitCount;
	DWORD   dwBytesInRes;
	WORD    nID; // 리소스 ID
} GRPICONDIRENTRY;

typedef struct
{
	WORD            idReserved;
	WORD            idType;
	WORD            idCount;
	GRPICONDIRENTRY idEntries[1];
} GRPICONDIR;
#pragma pack(pop)


CStringW thisDllDirPath()
{
	CStringW thisPath = L"";
	WCHAR path[MAX_PATH];
	HMODULE hm;
	if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPWSTR)&thisDllDirPath, &hm))
	{
		GetModuleFileNameW(hm, path, MAX_PATH);
		PathRemoveFileSpecW(path);
		thisPath = CStringW(path);
		if (!thisPath.IsEmpty() &&
			thisPath.GetAt(thisPath.GetLength() - 1) != '\\')
			thisPath += L"\\";
	}
	return thisPath;
}

HMODULE GetThisDllModule()
{
	HMODULE hm;
	if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPWSTR)&thisDllDirPath, &hm))
	{
		return hm;
	}
	return NULL;
}

namespace
{
	// EnumResourceLanguages 콜백: 첫 번째 언어 ID를 얻고 열거 중단.
	// 대상 exe의 기존 아이콘 리소스 언어를 그대로 따라가기 위해 사용.
	// (언어가 다르면 UpdateResource가 교체가 아닌 "추가"로 동작하여 파일이 커짐)
	BOOL CALLBACK FirstLanguageProc(HMODULE /*hModule*/, LPCTSTR /*lpszType*/, LPCTSTR /*lpszName*/, WORD wLanguage, LONG_PTR lParam)
	{
		*reinterpret_cast<WORD*>(lParam) = wLanguage;
		return FALSE; // 첫 번째 언어만 필요하므로 열거 중단
	}

	// EnumResourceNames 콜백: 대상 exe에 존재하는 RT_ICON 정수 ID의 최대값 수집.
	// 새 아이콘을 기존 다른 아이콘 그룹의 ID와 충돌하지 않는 ID로 추가하기 위해 사용.
	BOOL CALLBACK MaxIconIdProc(HMODULE /*hModule*/, LPCTSTR /*lpszType*/, LPTSTR lpszName, LONG_PTR lParam)
	{
		if (IS_INTRESOURCE(lpszName))
		{
			WORD* pMaxId = reinterpret_cast<WORD*>(lParam);
			const WORD wId = static_cast<WORD>(reinterpret_cast<ULONG_PTR>(lpszName));
			if (wId > *pMaxId)
				*pMaxId = wId;
		}
		return TRUE;
	}

	// EnumResourceNames 콜백: RT_GROUP_ICON 정수 ID의 최소값 수집.
	// uPreviousIconResourceID == 0(자동 감지)일 때, 탐색기가 exe 아이콘으로 표시하는
	// 그룹(= 가장 낮은 그룹 아이콘 ID)을 찾기 위해 사용. (문자열 이름 리소스는 무시)
	struct MinGroupIdCtx { WORD wMinId; bool bFound; };
	BOOL CALLBACK MinGroupIconIdProc(HMODULE /*hModule*/, LPCTSTR /*lpszType*/, LPTSTR lpszName, LONG_PTR lParam)
	{
		if (IS_INTRESOURCE(lpszName))
		{
			MinGroupIdCtx* p = reinterpret_cast<MinGroupIdCtx*>(lParam);
			const WORD wId = static_cast<WORD>(reinterpret_cast<ULONG_PTR>(lpszName));
			if (!p->bFound || wId < p->wMinId)
			{
				p->wMinId = wId;
				p->bFound = true;
			}
		}
		return TRUE;
	}
}

namespace __ENF
{
	namespace UI
	{
		bool EnfIcon::ChangeIcon(HWND hWnd, const ENF_ICON& Icon, unsigned int uCustomIConResourceID /*= 0*/, HICON* phDialogIcon /*= nullptr*/)
		{
			assertm(::IsWindow(hWnd), "Is NOT Window !");
			HICON hIcon = GetIcon(Icon, uCustomIConResourceID);

			if (hIcon != NULL)
			{
				::SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
				::SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

				// 다이얼로그 클래스의 m_hIcon도 갱신할 수 있도록 포인터로 돌려준다.
				// (기존 void* 파라미터는 로컬 복사본에 대입되어 아무 효과가 없던 버그 수정)
				if (phDialogIcon)
					*phDialogIcon = hIcon;
				return true;
			}
			return false;
		}

		bool EnfIcon::ChangeExeIcon(
			const char* sFilePath,
			const unsigned int& uToChangeGroupResourceID,
			const ENF_ICON& Icon,
			const unsigned int& uCustomGroupResourceID/* = 0*/,
			const WORD wLanguage /*= MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT)*/)
		{
			CString strFilePath(sFilePath);

			// 1. 새로 넣을 그룹 아이콘 리소스를 이 DLL(또는 CUSTOM인 경우 호출 프로그램)에서 로드
			HMODULE hInstance = GetInstance(Icon);
			HRSRC hGrpIconRes = GetIconType2(Icon, uCustomGroupResourceID);
			if (hGrpIconRes == NULL)
				return false;

			HGLOBAL hGrpIconResLoad = LoadResource(hInstance, hGrpIconRes);
			if (hGrpIconResLoad == NULL)
				return false;

			const GRPICONDIR* pSrcGrpDir = (const GRPICONDIR*)LockResource(hGrpIconResLoad);
			if (pSrcGrpDir == NULL)
				return false;

			const DWORD grpIconSize = SizeofResource(hInstance, hGrpIconRes);

			// 2. 대상 exe의 기존 아이콘 정보 조사
			//    - 기존 그룹 아이콘의 언어 ID (교체가 아닌 추가로 동작하여 파일이 커지는 것 방지)
			//    - 기존 그룹이 참조하던 RT_ICON ID 목록 (교체 후 남는 고아 리소스 삭제용)
			//    - 기존 RT_ICON ID 최대값 (다른 아이콘 그룹과의 ID 충돌 방지)
			WORD wTargetLang = wLanguage;
			WORD wMaxIconId = 0;
			std::vector<WORD> oldIconIds;

			// 대상 exe를 읽기 전용으로 매핑하여 기존 아이콘 정보를 조사한 뒤,
			// BeginUpdateResource 전에 반드시 매핑을 해제한다.
			// (같은 파일을 읽기용으로 매핑한 채 쓰기를 시작하면 공유 위반으로 실패)
			HMODULE hOldExe = LoadLibraryEx(strFilePath, NULL,
				LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
			if (hOldExe == NULL)
				return false; // 대상 exe를 열 수 없음 (경로 오류/파일 사용 중)

			// 대상 exe에서 가장 낮은 기존 그룹 아이콘 ID(=탐색기가 표시하는 아이콘) 감지.
			MinGroupIdCtx lowest = { 0, false };
			EnumResourceNames(hOldExe, RT_GROUP_ICON, MinGroupIconIdProc, (LONG_PTR)&lowest);

			// 교체할 그룹 아이콘 ID 결정.
			//  - uToChangeGroupResourceID != 0 : 입력된 그룹 아이콘 ID 사용 (없으면 그 ID로 새로 생성)
			//  - uToChangeGroupResourceID == 0 : 자동 감지 → 가장 낮은 기존 그룹 ID.
			//    탐색기는 exe 아이콘으로 가장 낮은 ID의 그룹을 표시하므로 아이콘 그룹이 여러 개여도
			//    실제 표시 아이콘을 교체한다. (C# WinForms/WPF exe는 보통 그룹 ID 32512 하나 → 그대로 처리)
			unsigned int uGroupId = uToChangeGroupResourceID;
			if (uGroupId == 0)
			{
				if (!lowest.bFound)
				{
					FreeLibrary(hOldExe);
					return false; // 자동 감지인데 아이콘 그룹이 하나도 없음 → 만들 기준 없음
				}
				uGroupId = lowest.wMinId;
			}

			// 요청/결정된 그룹을 조회.
			//  - 있으면(교체): 언어를 감지하고, 교체 후 삭제할 기존 아이콘 ID 목록을 수집.
			//  - 없으면(생성): oldIconIds는 비운 채 진행. 새 그룹 언어는 exe의 기존 그룹 언어를
			//    따라간다(있을 때). C#(lang 0) 등에서 언어 불일치로 아이콘이 깨지거나 중복되는 것 방지.
			bool bGroupExists = false;
			HRSRC hOldGrpRes = FindResource(hOldExe, MAKEINTRESOURCE(uGroupId), RT_GROUP_ICON);
			if (hOldGrpRes != NULL)
			{
				bGroupExists = true;
				EnumResourceLanguages(hOldExe, RT_GROUP_ICON, MAKEINTRESOURCE(uGroupId),
					FirstLanguageProc, (LONG_PTR)&wTargetLang);

				HGLOBAL hOldGrpLoad = LoadResource(hOldExe, hOldGrpRes);
				const GRPICONDIR* pOldGrpDir = (hOldGrpLoad != NULL) ? (const GRPICONDIR*)LockResource(hOldGrpLoad) : NULL;
				if (pOldGrpDir != NULL)
				{
					for (int i = 0; i < pOldGrpDir->idCount; ++i)
						oldIconIds.push_back(pOldGrpDir->idEntries[i].nID);
				}
			}
			else if (lowest.bFound)
			{
				// 지정한 그룹 ID는 없지만 exe에 다른 그룹이 있으면, 그 언어를 따라 새 그룹을 생성한다.
				EnumResourceLanguages(hOldExe, RT_GROUP_ICON, MAKEINTRESOURCE(lowest.wMinId),
					FirstLanguageProc, (LONG_PTR)&wTargetLang);
			}
			(void)bGroupExists; // 없으면 아래 UpdateResource가 새로 생성하므로 별도 실패 처리 없음

			EnumResourceNames(hOldExe, RT_ICON, MaxIconIdProc, (LONG_PTR)&wMaxIconId);

			// 조사 완료 → 쓰기 시작 전에 매핑 해제 (필수)
			FreeLibrary(hOldExe);

			// 3. 대상 exe에 직접 리소스 업데이트 (임시 파일/백업 없이 제자리 변경).
			//    백업이 필요하면 호출하는 쪽에서 이 함수 호출 전에 파일을 복사해 둘 것.
			HANDLE hUpdateRes = BeginUpdateResource(strFilePath, FALSE);
			if (hUpdateRes == NULL)
				return false; // 대상 exe가 사용 중이면 여기서 실패 (실행 중인 exe는 변경 불가)

			// 실패 시 공통 정리: 변경 내용을 버림(EndUpdateResource TRUE)
			auto fnFail = [&]() -> bool
			{
				EndUpdateResource(hUpdateRes, TRUE);
				return false;
			};

			// 4. 기존 그룹이 참조하던 개별 아이콘(RT_ICON) 삭제
			//    삭제하지 않으면 그룹만 교체되고 옛 아이콘 이미지가 고아 리소스로 남아 파일이 커진다.
			for (size_t i = 0; i < oldIconIds.size(); ++i)
			{
				// lpData = NULL 이면 해당 리소스 삭제
				UpdateResource(hUpdateRes, RT_ICON, MAKEINTRESOURCE(oldIconIds[i]), wTargetLang, NULL, 0);
			}

			// 5. 새 그룹 아이콘 디렉터리 사본을 만들어 RT_ICON ID를 재할당
			//    DLL 쪽 ID를 그대로 쓰면 대상 exe의 다른 아이콘 그룹이 쓰는 ID와 충돌(덮어쓰기)할 수 있다.
			std::vector<BYTE> newGrpBuf(grpIconSize);
			memcpy(newGrpBuf.data(), pSrcGrpDir, grpIconSize);
			GRPICONDIR* pNewGrpDir = (GRPICONDIR*)newGrpBuf.data();

			// 6. 개별 아이콘 리소스를 새 ID로 추가하고 그룹 디렉터리에 반영
			for (int i = 0; i < pSrcGrpDir->idCount; ++i)
			{
				const GRPICONDIRENTRY& entry = pSrcGrpDir->idEntries[i];
				const WORD wNewId = (WORD)(wMaxIconId + 1 + i);

				HRSRC hIconRes = FindResource(hInstance, MAKEINTRESOURCE(entry.nID), RT_ICON);
				if (hIconRes == NULL)
					return fnFail();

				HGLOBAL hIconResLoad = LoadResource(hInstance, hIconRes);
				if (hIconResLoad == NULL)
					return fnFail();

				LPVOID pIconResLock = LockResource(hIconResLoad);
				if (pIconResLock == NULL)
					return fnFail();

				DWORD iconResSize = SizeofResource(hInstance, hIconRes);

				if (!UpdateResource(hUpdateRes, RT_ICON, MAKEINTRESOURCE(wNewId), wTargetLang, pIconResLock, iconResSize))
					return fnFail();

				pNewGrpDir->idEntries[i].nID = wNewId;
			}

			// 7. 그룹 아이콘 리소스 반영.
			//    - 같은 ID+언어가 이미 있으면 교체, 없으면 새로 생성(요청 ID가 대상 exe에 없던 경우).
			//    - 참고: 탐색기는 그룹 ID가 가장 낮은 것을 exe 아이콘으로 표시하므로,
			//      새로 만든 ID가 기존 최저 ID보다 낮을 때만 표시 아이콘이 실제로 바뀐다.
			if (!UpdateResource(hUpdateRes, RT_GROUP_ICON, MAKEINTRESOURCE(uGroupId), wTargetLang, newGrpBuf.data(), grpIconSize))
				return fnFail();

			// 8. 리소스 업데이트 완료 (여기서 실제 파일에 반영)
			if (!EndUpdateResource(hUpdateRes, FALSE))
				return false;

			// 9. 탐색기 아이콘 캐시 갱신 → 변경된 아이콘 즉시 확인 가능
			CStringA pathA(strFilePath);
			RefreshShellIconCache(pathA.GetBuffer());

			return true;
		}

		void EnfIcon::RefreshShellIconCache(const char* sFilePath/* = ""*/)
		{
			// 탐색기 재시작(taskkill /f /im explorer.exe)이나 IconCache.db 수동 삭제 없이
			// 셸에 변경 통지를 보내 아이콘 캐시를 갱신하는 표준 방식.
			// SHCNF_FLUSH: 통지가 처리될 때까지 동기 대기 → 함수 반환 직후 탐색기에서 확인 가능
			CString path(sFilePath);
			if (!path.IsEmpty() && PathFileExists(path))
			{
				// 해당 파일 항목 갱신 통지
				SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH | SHCNF_FLUSH, (LPCVOID)(LPCTSTR)path, NULL);
			}
			else
			{
				// 전체 아이콘 표시 갱신 통지 (탐색기가 아이콘 캐시를 다시 만들도록 함)
				SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST | SHCNF_FLUSH, NULL, NULL);
			}
		}

		HMODULE EnfIcon::GetInstance(const ENF_ICON& Type)
		{
			HMODULE hModule;
			switch (Type)
			{
			case ENF_ICON::ENF_ICON_REXGEN:
			case ENF_ICON::ENF_ICON_TRAWORKS:
			case ENF_ICON::ENF_ICON_KOREANET:
			case ENF_ICON::ENF_ICON_SONET: // 기존에 누락되어 호출 프로그램에서 리소스를 찾던 버그 수정
			{
				// 기본 제공 아이콘은 이 DLL의 리소스에 있다.
				hModule = GetThisDllModule();
				break;
			}
			case ENF_ICON::ENF_ICON_CUSTOM:
			default:
				// CUSTOM 아이콘은 호출한 프로그램의 리소스에 있다.
				hModule = AfxGetInstanceHandle();
				break;
			}
			return hModule;
		}

		HICON EnfIcon::GetIcon(const ENF_ICON& Icon, unsigned int uCustomIConResourID /*= 0*/)
		{
			HICON hIcon = NULL;
			switch (Icon)
			{
			default:
				break;
			case ENF_ICON::ENF_ICON_REXGEN:
			{
				hIcon = LoadIcon(GetInstance(Icon), MAKEINTRESOURCE(IDI_ICON_REXGEN));
				break;
			}
			case ENF_ICON::ENF_ICON_TRAWORKS:
			{
				hIcon = LoadIcon(GetInstance(Icon), MAKEINTRESOURCE(IDI_ICON_TRAWORKS));
				break;
			}
			case ENF_ICON::ENF_ICON_KOREANET:
			{
				hIcon = LoadIcon(GetInstance(Icon), MAKEINTRESOURCE(IDI_ICON_KOREANET));
				break;
			}
			case ENF_ICON::ENF_ICON_SONET:
			{
				hIcon = LoadIcon(GetInstance(Icon), MAKEINTRESOURCE(IDI_ICON_SONET));
				break;
			}
			case ENF_ICON::ENF_ICON_CUSTOM:
				hIcon = LoadIcon(GetInstance(Icon), MAKEINTRESOURCE(uCustomIConResourID));
				break;
			}

			return hIcon;
		}

		HRSRC EnfIcon::GetIconType2(const ENF_ICON& Icon, unsigned int uCustomIConResourID /*= 0*/)
		{
			LPCTSTR ResourceID;
			switch (Icon)
			{
			case ENF_ICON::ENF_ICON_REXGEN:
			{
				ResourceID = MAKEINTRESOURCE(IDI_ICON_REXGEN);
				break;
			}
			case ENF_ICON::ENF_ICON_TRAWORKS:
			{
				ResourceID = MAKEINTRESOURCE(IDI_ICON_TRAWORKS);
				break;
			}
			case ENF_ICON::ENF_ICON_KOREANET:
			{
				ResourceID = MAKEINTRESOURCE(IDI_ICON_KOREANET);
				break;
			}
			case ENF_ICON::ENF_ICON_SONET:
			{
				ResourceID = MAKEINTRESOURCE(IDI_ICON_SONET);
				break;
			}
			case ENF_ICON::ENF_ICON_CUSTOM:
				ResourceID = MAKEINTRESOURCE(uCustomIConResourID);
				break;
			default:
				return NULL;
			}

			return FindResource(GetInstance(Icon), ResourceID, RT_GROUP_ICON);
		}

	}
}
