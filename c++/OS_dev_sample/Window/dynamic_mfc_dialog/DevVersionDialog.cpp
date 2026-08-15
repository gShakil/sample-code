#include "pch.h"
#include "DevVersionDialog.h"
#include "resource.h"

constexpr int MINIMUM_LEFT_GAP = 10;
constexpr int MINIMUM_TOP_GAP = 5;

constexpr int LOGO_WIDTH = 120;
constexpr int LOGO_HEIGHT = 39;

constexpr int EXPORT_WIDTH = 110;
constexpr int EXPORT_HEIGHT = 25;

constexpr int MODULE_TEXT_GAP = 3;
constexpr int MODULE_TEXT_HEIGHT = 15;
constexpr int LOGO_N_MODULE_START_GAP = 15;

constexpr int RESOURCE_NUM_START			= 1000;
constexpr int APPNAME_RESOURCENUM			= RESOURCE_NUM_START + 1;
constexpr int VERSION_RESOURCENUM			= RESOURCE_NUM_START + 2;
constexpr int LOGO_RESOURCENUM				= RESOURCE_NUM_START + 3;
constexpr int COPYRIGHT_RESOURCENUM			= RESOURCE_NUM_START + 4;
constexpr int EXPORT_BTN_RESOURCENUM		= RESOURCE_NUM_START + 5;
constexpr int START_MODULE_RESOURCE_NUM		= RESOURCE_NUM_START + 100;

BEGIN_MESSAGE_MAP(DevVersionDialog, CDynDialogEx)
	ON_BN_CLICKED(EXPORT_BTN_RESOURCENUM, &DevVersionDialog::OnBnClickedExport)
END_MESSAGE_MAP()

DevVersionDialog::DevVersionDialog(CWnd* pParentWnd)
	: CDynDialogEx(pParentWnd)
	, m_LogoType(DVD_LOGO_TYPE::DVD_REX_WHITE_LOGO)
	, m_AddSize(0,0)
	, m_bEnableExportBtn(false)
{
	SetWindowTitle("버전정보");
}

DevVersionDialog::~DevVersionDialog()
{
	m_Logo.DeleteBitmap();
}


void DevVersionDialog::ShowWindow()
{
	CFont* pft = AfxGetMainWnd()->GetFont();
	SetFont(pft);
	SetUseSystemButtons(FALSE);

	
	if (!GetTextRef(DVD_TEXT_TYPE::DVD_APP_NAME).IsEmpty())
	{
		AddDlgControl(
			STATICTEXT,
			GetTextRef(DVD_TEXT_TYPE::DVD_APP_NAME),
			STYLE_STATIC,
			EXSTYLE_STATIC,
			CRect(MINIMUM_LEFT_GAP, MINIMUM_TOP_GAP, 125, 25),
			NULL,
			APPNAME_RESOURCENUM
		);
	}

	if (!GetTextRef(DVD_TEXT_TYPE::DVD_VERSION).IsEmpty() && !GetTextRef(DVD_TEXT_TYPE::DVD_BUILD_DATE).IsEmpty())
	{
		AddDlgControl(
			STATICTEXT,
			MakeVersionString(GetTextRef(DVD_TEXT_TYPE::DVD_VERSION), GetTextRef(DVD_TEXT_TYPE::DVD_BUILD_DATE), GetTextRef(DVD_TEXT_TYPE::DVD_BUILD_TIME)),
			STYLE_STATIC_RIGHT,
			EXSTYLE_STATIC,
			CRect(MINIMUM_LEFT_GAP + 125, MINIMUM_TOP_GAP, 250, 25),
			NULL,
			VERSION_RESOURCENUM
		);
	}

	AddDlgControl(
		STATICTEXT,
		_T("CopyRight (C) 2017 RexGen Co.LTD"),
		STYLE_STATIC,
		EXSTYLE_STATIC,
		CRect(MINIMUM_LEFT_GAP + 125, MINIMUM_TOP_GAP, 250, 25),
		NULL,
		COPYRIGHT_RESOURCENUM
	);

	CRect rcTemp(0, 0, 10, 10);
	AddDlgControl(
		BUTTON,
		_T(""),
		STYLE_BUTTON,
		EXSTYLE_BUTTON,
		CRect(0,0,200,200),
		&m_Logo,
		LOGO_RESOURCENUM
	);

	AddDlgControl(
		BUTTON,
		_T("출력"),
		STYLE_BUTTON,
		EXSTYLE_BUTTON,
		rcTemp,
		NULL,
		EXPORT_BTN_RESOURCENUM
	);


	UINT uNum = START_MODULE_RESOURCE_NUM;
	for (const CString& Module : m_vModuleVersion)
	{
		AddDlgControl(
			STATICTEXT,
			Module,
			STYLE_STATIC,
			EXSTYLE_STATIC,
			rcTemp,
			NULL,
			uNum++
		);
	}

	DoModal();
}

void DevVersionDialog::SetOnBnClickExport(std::function<void()> pFunc)
{
	m_pOnBnExport = pFunc;
}

BOOL DevVersionDialog::OnInitDialog()
{
	CDynDialogEx::OnInitDialog();

	CSize size = GetDialogSize();
	SetWindowPos(NULL, 0, 0, size.cx + m_AddSize.cx + 15, size.cy + m_AddSize.cy + 38, SWP_NOZORDER | SWP_NOMOVE); // 15, 38: 다이얼로그 크기 맞추기 위한 가산치
	ArrangeResourcePosition(size);

	//SetWindowPos(NULL, 0, 0, 500, 50, SWP_NOZORDER | SWP_NOMOVE);

	ShowLogo();

	return TRUE;
}
void DevVersionDialog::SetDialogName(const CString& strValue)
{
	SetWindowTitle(CStringA(strValue));
}

void DevVersionDialog::SetLogo(const DVD_LOGO_TYPE& Type, const CString& strLogoPath /*= _T("")*/)
{
	m_LogoType = Type;
	m_strCustomLogoPath = strLogoPath;
}

void DevVersionDialog::SetAppName(const CString& strValue)
{
	GetTextRef(DVD_TEXT_TYPE::DVD_APP_NAME) = strValue;
}

void DevVersionDialog::SetVersion(const CString& strValue)
{
	GetTextRef(DVD_TEXT_TYPE::DVD_VERSION) = strValue;
}

void DevVersionDialog::SetBuildDate(const CString& strValue)
{
	GetTextRef(DVD_TEXT_TYPE::DVD_BUILD_DATE) = strValue;
}

void DevVersionDialog::SetBuildTime(const CString& strValue)
{
	GetTextRef(DVD_TEXT_TYPE::DVD_BUILD_TIME) = strValue;
}

void DevVersionDialog::AddWidth(const UINT& uValue)
{
	m_AddSize.cx += uValue;
}

void DevVersionDialog::AddHeight(const UINT& uValue)
{
	m_AddSize.cy += uValue;
}

void DevVersionDialog::EnableExportButton(const bool& bValue)
{
	m_bEnableExportBtn = bValue;
}

void DevVersionDialog::PushModuleVersion(const CString& strValue)
{
	m_vModuleVersion.push_back(strValue);
}

const CString& DevVersionDialog::GetText(DVD_TEXT_TYPE Type)
{
	return m_strTextArray[(unsigned int)Type];
}

const std::vector<CString>& DevVersionDialog::GetModuleVersionVector()
{
	return m_vModuleVersion;
}

void DevVersionDialog::OnBnClickedExport()
{
	if (m_pOnBnExport)
		m_pOnBnExport();
}

void DevVersionDialog::ArrangeResourcePosition(const CSize& rcDlg)
{
	CDynDialogItemEx* pDynDialogItemEx = NULL;
	int nsize = (int)m_arrDlgItemPtr.GetSize();
	long l = GetNumberOfConrols();
	for (int i = 0; i < m_arrDlgItemPtr.GetSize(); i++)
	{
		pDynDialogItemEx = m_arrDlgItemPtr[i];
		if (pDynDialogItemEx != NULL)
		{
			switch (pDynDialogItemEx->GetControlID())
			{
			case APPNAME_RESOURCENUM:
				::MoveWindow(pDynDialogItemEx->GetSafeHwnd(), MINIMUM_LEFT_GAP, MINIMUM_TOP_GAP, rcDlg.cx / 2 - MINIMUM_LEFT_GAP, MODULE_TEXT_HEIGHT, TRUE);
				break;
			case VERSION_RESOURCENUM:
				::MoveWindow(pDynDialogItemEx->GetSafeHwnd(), MINIMUM_LEFT_GAP + rcDlg.cx / 2 + m_AddSize.cx, MINIMUM_TOP_GAP, rcDlg.cx / 2 - MINIMUM_LEFT_GAP * 2, MODULE_TEXT_HEIGHT, TRUE);
				break;
			case COPYRIGHT_RESOURCENUM:
				::MoveWindow(pDynDialogItemEx->GetSafeHwnd(), MINIMUM_LEFT_GAP, rcDlg.cy - MINIMUM_TOP_GAP * 2 - MODULE_TEXT_HEIGHT, rcDlg.cx - MINIMUM_LEFT_GAP - EXPORT_WIDTH, MODULE_TEXT_HEIGHT, TRUE);
				break;
			case LOGO_RESOURCENUM:
				::MoveWindow(m_Logo.GetSafeHwnd(), MINIMUM_LEFT_GAP, MINIMUM_TOP_GAP + MODULE_TEXT_HEIGHT + LOGO_N_MODULE_START_GAP, LOGO_WIDTH, LOGO_HEIGHT, TRUE);
				break;
			case EXPORT_BTN_RESOURCENUM:
				::EnableWindow(pDynDialogItemEx->GetSafeHwnd(), m_bEnableExportBtn);
				::MoveWindow(pDynDialogItemEx->GetSafeHwnd(), rcDlg.cx - EXPORT_WIDTH - 5 + m_AddSize.cx, rcDlg.cy - EXPORT_HEIGHT - MINIMUM_TOP_GAP, EXPORT_WIDTH, EXPORT_HEIGHT, TRUE);
				break;
			default:
			{
				for (size_t i = 0; i < m_vModuleVersion.size(); i++)
				{
					if (pDynDialogItemEx->GetControlID() == START_MODULE_RESOURCE_NUM + i)
					{
						::MoveWindow(pDynDialogItemEx->GetSafeHwnd(), 
							MINIMUM_LEFT_GAP + rcDlg.cx / 2, 
							(int)(MINIMUM_TOP_GAP + MODULE_TEXT_HEIGHT + LOGO_N_MODULE_START_GAP + ( i * (MODULE_TEXT_GAP + MODULE_TEXT_HEIGHT))),
							rcDlg.cx / 2 - MINIMUM_LEFT_GAP * 2 + m_AddSize.cx, 
							MODULE_TEXT_HEIGHT,
							TRUE);
					}
				}
				break;
			}
			}



			UINT nid = pDynDialogItemEx->GetControlID();
			UINT nid2 = pDynDialogItemEx->GetControlID();

			//if (nid != 3)
			//pDynDialogItemEx->::MoveWindow(0, 0, 200, 200);
		}
	}
}

CSize DevVersionDialog::GetDialogSize()
{
	if (m_vModuleVersion.size() < 2)
	{
		return CSize(400,
			MINIMUM_TOP_GAP
			+ MODULE_TEXT_HEIGHT
			+ LOGO_N_MODULE_START_GAP
			+ LOGO_HEIGHT
			+ LOGO_N_MODULE_START_GAP
			+ MODULE_TEXT_HEIGHT
			+ MINIMUM_TOP_GAP
		);
	}
	else
	{
		return CSize(400,
			MINIMUM_TOP_GAP
			+ MODULE_TEXT_HEIGHT
			+ LOGO_N_MODULE_START_GAP
			+ (int)(m_vModuleVersion.size() * MODULE_TEXT_HEIGHT)
			+ (int)((m_vModuleVersion.size() - 1) * MODULE_TEXT_GAP)
			+ EXPORT_HEIGHT
			+ LOGO_N_MODULE_START_GAP
			+ MINIMUM_TOP_GAP
		);
	}
}

CString DevVersionDialog::MakeVersionString(const CString& strVersion, const CString& strDate, const CString& strTime)
{
	CString str;
	str.Format(_T("%s (%s %s)"), strVersion, strDate, strTime);
	return str;
}

void DevVersionDialog::ShowLogo()
{
	m_Logo.ModifyStyle(0, BS_OWNERDRAW); // DrawItem 함수 호출을 위해 추가


	switch (m_LogoType)
	{
	case DVD_LOGO_TYPE::DVD_REX_BLACK_LOGO:
	{
		HMODULE hModule = GetThisDllModule();
		HBITMAP hBitmap = (HBITMAP)::LoadBitmap(hModule, MAKEINTRESOURCE(IDB_BITMAP_REX_BLK_LOGO));
		m_Logo.AttachBitmap(NULL, NULL, NULL, hBitmap, NULL);
		break;
	}
	case DVD_LOGO_TYPE::DVD_REX_WHITE_LOGO:
	{
		HMODULE hModule = GetThisDllModule();
		HBITMAP hBitmap = (HBITMAP)::LoadBitmap(hModule, MAKEINTRESOURCE(IDB_BITMAP_REX_WHITE_LOGO));
		m_Logo.AttachBitmap(NULL, NULL, NULL, hBitmap, NULL);
		break;
	}
	case DVD_LOGO_TYPE::DVD_TRA_BLACK_LOGO:
	{
		HMODULE hModule = GetThisDllModule();
		HBITMAP hBitmap = (HBITMAP)::LoadBitmap(hModule, MAKEINTRESOURCE(IDB_BITMAP_TRA_BLK_LOGO));
		m_Logo.AttachBitmap(NULL, NULL, NULL, hBitmap, NULL);
		break;
	}

	case DVD_LOGO_TYPE::DVD_CUSTOM_LOGO:
	{
		m_Logo.LoadBitmap(NULL, NULL, NULL, m_strCustomLogoPath, NULL);
		break;
	}
	default:
		break;
	}
	// 동적 생성된 다이얼로그에서 버튼을 누르면 다이얼로그가 사라지는 증상 발생.
	// 버튼을 비활성화시켜 버튼 이벤트를 막고, 비활성화시에 비트맵을 할당한다.
	m_Logo.EnableWindow(FALSE);

	// 	
	// 
	// // 	HMODULE hm = ::GetModuleHandle(0);
	//  	HMODULE hm3 = ::GetModuleHandle(_T("DynamicVersionDialog_v142x64_d.dll"));
	// // 	HMODULE hmm = AfxGetInstanceHandle();
	// // 	LPWSTR t = MAKEINTRESOURCE(IDB_BITMAP_REX_BLK_LOGO);
	// 
	// 	HMODULE hm;
	// 	GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
	// 		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
	// 		NULL, &hm);

	// 	m_Logo.ModifyStyle(0, BS_OWNERDRAW); // DrawItem 함수 호출을 위해 추가
	// 	// 동적 생성된 다이얼로그에서 버튼을 누르면 다이얼로그가 사라지는 증상 발생.
	// 	// 버튼을 비활성화시켜 버튼 이벤트를 막고, 비활성화시에 비트맵을 할당한다.
	// 	m_Logo.EnableWindow(FALSE);
	// 	HBITMAP hBitmap = (HBITMAP)::LoadBitmap(hm3, MAKEINTRESOURCE(IDB_BITMAP_REX_BLK_LOGO));
	// 	m_Logo.AttachBitmap(NULL, NULL, NULL, hBitmap, NULL);

}

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
// 	else if (_DEBUG) std::wcout << L"GetModuleHandle Error: " << GetLastError() << std::endl;
// 
// 	if (_DEBUG) std::wcout << L"thisDllDirPath: [" << CStringW::PCXSTR(thisPath) << L"]" << std::endl;
	return thisPath;
}

CString& DevVersionDialog::GetTextRef(DVD_TEXT_TYPE Type)
{
	return m_strTextArray[(unsigned int)Type];
}


HMODULE DevVersionDialog::GetThisDllModule()
{
	CStringW thisPath = L"";
	//WCHAR path[MAX_PATH];
	HMODULE hm;
	if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPWSTR)&thisDllDirPath, &hm))
	{
		return hm;
	}
	return NULL;
}

