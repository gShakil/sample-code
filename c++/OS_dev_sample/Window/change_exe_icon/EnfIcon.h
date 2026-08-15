#pragma once
#include "EnfDllAPI.h"

/*
프로그램 Icon 변경하는 라이브러리.
*/

namespace __ENF
{
	namespace UI
	{
		enum class ENF_ICON : int
		{
			ENF_NONE = 0,

			ENF_ICON_REXGEN,
			ENF_ICON_TRAWORKS,
			ENF_ICON_KOREANET,
			ENF_ICON_SONET,

			ENF_ICON_CUSTOM,
		};

		class ENF_DLL_DEFINE EnfIcon
		{
		public:
			/*
			* ChangeIcon
			* : hWnd로 입력된 창의 아이콘을 변경 (작업 표시줄, 창 좌상단 아이콘). exe 파일은 바뀌지 않음.
			*
			* hWnd : 변경할 다이얼로그/창의 핸들 입력
			* Icon : 사용할 아이콘을 선택
			* uCustomIConResourceID : Icon = custom 아이콘으로 사용 시 변경할 아이콘 ID (프로젝트에 등록되어있는 아이콘 ID) 입력
			* phDialogIcon : 다이얼로그 베이스인 경우 다이얼로그 클래스의 &m_hIcon 입력 (변경된 아이콘 핸들을 돌려받음)
			*/
			bool ChangeIcon(HWND hWnd, const ENF_ICON& Icon, unsigned int uCustomIConResourceID = 0, HICON* phDialogIcon = nullptr);

			/*
			* strFilePath             : 변경하고자 하는 exe 파일 경로
			* uToChangeGroupResourceID : 교체할 그룹 아이콘의 리소스 ID.
			*                           0 입력 시 대상 exe에서 그룹 아이콘 ID가 가장 낮은 아이콘 교체
			*                           (탐색기가 표시하는 아이콘: 가장 낮은 ID의 그룹)
			*                           0이 아니면 입력된 ID의 그룹을 교체하며, 그 ID가 대상 exe에 없으면 새로 생성.			
			* Icon                    : 변경하고자 하는 아이콘
			* uCustomGroupResourceID  : Icon이 ENF_ICON_CUSTOM일 때 사용하고자 하는 호출 프로그램 리소스의 그룹 아이콘 ID
			* wLanguage               : 리소스 언어 식별자. 대상 exe의 기존 아이콘 언어를 자동 감지하며,
			*                           감지 실패 시에만 이 값을 사용 (기본: 한국어 1042)
			*
			* 변경 성공 시 RefreshShellIconCache()를 내부에서 호출
			*/
			bool ChangeExeIcon(
				const char* sFilePath,
				const unsigned int& uToChangeGroupResourceID,
				const ENF_ICON& Icon,
				const unsigned int& uCustomGroupResourceID = 0,
				const WORD wLanguage = MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT)
			);
			
			// strFilePath : 아이콘 캐시를 갱신할 파일. 공백인 경우 전체 갱신
			static void RefreshShellIconCache(const char* sFilePath = "");

		protected:

		private:
			HMODULE GetInstance(const ENF_ICON& Type);
			HICON GetIcon(const ENF_ICON& Icon, unsigned int uCustomIConResourID = 0);
			HRSRC GetIconType2(const ENF_ICON& Icon, unsigned int uCustomIConResourID = 0);
		};



	}
}
