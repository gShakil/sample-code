// TrayIconTest.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "TrayIconTest.h"

//RefreshTrayIcon()에서 사용된다
struct TRAYDATA
{
	HWND hwnd;
	UINT uID;
	UINT uCallbackMessage;
	DWORD Reserved[2];
	HICON hIcon;
};
// CTrayIconTest

//IMPLEMENT_DYNAMIC(CTrayIconTest, CWnd)

CTrayIconTest::CTrayIconTest()
{
	m_pParentWnd = NULL;
	m_uCallbackMessage = 0;
	m_IconResourceID = 0;
	m_bIsOnTray = FALSE;
}

CTrayIconTest::~CTrayIconTest()
{
}


//BEGIN_MESSAGE_MAP(CTrayIconTest, CWnd)
//END_MESSAGE_MAP()



// CTrayIconTest 메시지 처리기입니다.
void CTrayIconTest::SetTrayData(CWnd *pParent, UINT uCallbackMessage, HICON uIconResourceID)
{
	m_pParentWnd = pParent;
	m_uCallbackMessage = uCallbackMessage;
	m_IconResourceID = uIconResourceID;

}

BOOL CTrayIconTest::ShowTray(WCHAR *pwszMessage /* = NULL */)
{
	/*
	nid.uFlags
	NIF_ICON	: hicon 사용.
	NIF_MESSAGE : uCallbackMessage 사용.
	NIF_TIP		: szTip 사용.
	NIF_INFO	: 알림 사용

	Shell_NotifyIcon 함수는 3가지 용도로 사용된다.
	Shell_NotifyIcon(NIM_ADD, &nid)      - 트레이 추가
	Shell_NotifyIcon(NIM_MODIFY, &nid)   - 트레이 변경
	Shell_NotifyIcon(NIM_DELETE, &nid)   - 트레이 삭제

	*/
	ASSERT(m_pParentWnd && m_uCallbackMessage && m_IconResourceID != 0);
	if (m_pParentWnd == NULL)
		return FALSE;

	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));

	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.uID = 0;    // 트레이 구조체 아이디.
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	nid.hWnd = m_pParentWnd->GetSafeHwnd();
	//m_NotiData.hIcon = AfxGetApp()->LoadIconW(IDR_MAINFRAME);
	nid.hIcon = LoadIconFromID();
	nid.uCallbackMessage = m_uCallbackMessage;

	if (pwszMessage == NULL)
	{
		CString strWindow = _T("");
		m_pParentWnd->GetWindowText(strWindow);
		lstrcpy(nid.szTip, strWindow.GetBuffer());// 트레이아이콘에 마우스를 올려두었을 때 표시되는 텍스트
	}
	else
		lstrcpy(nid.szTip, pwszMessage);// 트레이아이콘에 마우스를 올려두었을 때 표시되는 텍스트

	if (::Shell_NotifyIcon(NIM_ADD, &nid))
	{
		m_bIsOnTray = TRUE;
		return TRUE;
	}

	return FALSE;

}
BOOL CTrayIconTest::HideTray()
{
	if (m_pParentWnd == NULL)
		return FALSE;

	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.uID = 0;
	nid.hWnd = m_pParentWnd->GetSafeHwnd();

	if (Shell_NotifyIcon(NIM_DELETE, &nid)) //트레이아이콘 제거
	{
		m_bIsOnTray = FALSE;
		return TRUE;
	}

	return FALSE;
	//---------------------------------------------------------------
	//NOTIFYICONDATA nid;
	//ZeroMemory(&nid, sizeof(nid));
	//nid.cbSize = sizeof(nid);
	//nid.uID = 0;
	//nid.hWnd = GetSafeHwnd();

	//BOOL bRet = ::Shell_NotifyIcon(NIM_DELETE, &nid); //트레이아이콘 제거
	//if (!bRet)
	//{
	//	::AfxMessageBox(_T("트레이아이콘 제거실패"));
	//	return;
	//}
	//AfxGetApp()->m_pMainWnd->ShowWindow(SW_SHOW); //윈도우 활성화
	//KillTimer(TRAY_BALLOON);
}
HICON CTrayIconTest::LoadIconFromID()
{
	return m_IconResourceID;// == NULL ? NULL : AfxGetApp()->LoadIconW(m_IconResourceID);
	//return m_IconResourceID == NULL ? NULL : AfxGetApp()->LoadIconW(m_IconResourceID);
}
BOOL CTrayIconTest::ShowAlertMessage(DWORD dwInfoFlag, DWORD dwTimer, const wchar_t *pwszTitle, const wchar_t *pwszInfo)
{
	ASSERT(m_bIsOnTray);

// 	m_nBallonSeq = (m_nBallonSeq + 1) % 4;
// 	DWORD dwInfoFlags = 0;
// 	wchar_t szInfo[256] = L"Ballon nid.szInfo";
// 	wchar_t szInfoTitle[256] = L"Ballon nid.szInfoTitle";
// 	switch (m_nBallonSeq)
// 	{
// 	case 0:
// 		dwInfoFlags = NIIF_NONE;
// 		swprintf_s(szInfo, L"Info_NIIF_NONE");
// 		break;
// 	case 1:
// 		dwInfoFlags = NIIF_INFO;
// 		swprintf_s(szInfo, L"Info_NIIF_INFO");
// 		break;
// 	case 2:
// 		dwInfoFlags = NIIF_WARNING;
// 		swprintf_s(szInfo, L"Info_NIIF_WARNING");
// 		break;
// 	case 3:
// 		dwInfoFlags = NIIF_ERROR;
// 		swprintf_s(szInfo, L"Info_NIIF_ERROR");
// 		break;
// 	}
// 	swprintf_s(szInfoTitle, L"Title_Seq: %d", m_nBallonSeq);

	/*
	dwInfoFlag :
	NIIF_NONE
	NIIF_INFO
	NIIF_WARNING
	NIIF_ERROR 중 하나
	*/
	if (m_pParentWnd == NULL)
		return FALSE;

	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));

	nid.cbSize				= sizeof(nid);
	nid.dwInfoFlags			= dwInfoFlag;        // 풍선 알림아이콘 종류
	nid.uFlags				= NIF_MESSAGE | NIF_INFO | NIF_ICON;
	nid.uTimeout			= dwTimer;
	nid.hWnd				= m_pParentWnd->GetSafeHwnd();
	nid.uCallbackMessage	= m_uCallbackMessage;
	nid.hIcon				= LoadIconFromID();

	if(pwszTitle != NULL)
		lstrcpy(nid.szInfoTitle, pwszTitle);
	if(pwszInfo != NULL)
		lstrcpy(nid.szInfo, pwszInfo);

	//::Shell_NotifyIcon(NIM_MODIFY, &nid)
	//기존에 NIM_ADD 된 것을 수정하는 것이기 때문에 NIM_MODIFY를 사용합니다.
	return ::Shell_NotifyIcon(NIM_MODIFY, &nid);
}
BOOL CTrayIconTest::RefreshTrayIcon()
{
	//프로그램의 비정상 종료로 인해 만들어져있는 트레이 아이콘을 삭제해주자
	CleanTrayIcon(GetTrayAreaType1());
	CleanTrayIcon(GetTrayAreaType2());
	
	return TRUE;
}
HWND CTrayIconTest::FindTrayToolbarWindow()
{
	HWND hWnd_ToolbarWindow32 = NULL;
	HWND hWnd_ShellTrayWnd;

	hWnd_ShellTrayWnd = ::FindWindow(_T("Shell_TrayWnd"), NULL);
	if (hWnd_ShellTrayWnd)
	{
		HWND hWnd_TrayNotifyWnd = ::FindWindowEx(hWnd_ShellTrayWnd, NULL, _T("TrayNotifyWnd"), NULL);

		if (hWnd_TrayNotifyWnd)
		{
			HWND hWnd_SysPager = ::FindWindowEx(hWnd_TrayNotifyWnd, NULL, _T("SysPager"), NULL);        // WinXP

																										// WinXP 에서는 SysPager 까지 추적            
			if (hWnd_SysPager)
			{
				hWnd_ToolbarWindow32 = ::FindWindowEx(hWnd_SysPager, NULL, _T("ToolbarWindow32"), NULL);
			}

			// Win2000 일 경우에는 SysPager 가 없이 TrayNotifyWnd -> ToolbarWindow32 로 넘어간다
			else
			{
				hWnd_ToolbarWindow32 = ::FindWindowEx(hWnd_TrayNotifyWnd, NULL, _T("ToolbarWindow32"), NULL);
			}
		}
	}

	return hWnd_ToolbarWindow32;
}
HWND CTrayIconTest::GetTrayAreaType1()
{
	HWND hWnd_ToolbarWindow32 = NULL;
	HWND hWnd_ShellTrayWnd = NULL;

	hWnd_ShellTrayWnd = ::FindWindow(_T("Shell_TrayWnd"), NULL);
	if (hWnd_ShellTrayWnd)
	{
		HWND hWnd_TrayNotifyWnd = ::FindWindowEx(hWnd_ShellTrayWnd, NULL, _T("TrayNotifyWnd"), NULL);

		if (hWnd_TrayNotifyWnd)
		{
			HWND hWnd_SysPager = ::FindWindowEx(hWnd_TrayNotifyWnd, NULL, _T("SysPager"), NULL);        // WinXP

																										// WinXP 에서는 SysPager 까지 추적            
			if (hWnd_SysPager)
			{
				hWnd_ToolbarWindow32 = ::FindWindowEx(hWnd_SysPager, NULL, _T("ToolbarWindow32"), NULL);
			}

			// Win2000 일 경우에는 SysPager 가 없이 TrayNotifyWnd -> ToolbarWindow32 로 넘어간다
			else
			{
				hWnd_ToolbarWindow32 = ::FindWindowEx(hWnd_TrayNotifyWnd, NULL, _T("ToolbarWindow32"), NULL);
			}
		}
	}

	return hWnd_ToolbarWindow32;
}
HWND CTrayIconTest::GetTrayAreaType2()
{
	// 윈도우 10
	HWND hWnd = NULL;

	hWnd = ::FindWindow(_T("NotifyIconOverflowWindow"), NULL);

	if (hWnd != NULL)
		hWnd = ::FindWindowEx(hWnd, NULL, _T("ToolbarWindow32"), NULL);

	return hWnd;
}
void CTrayIconTest::CleanTrayIcon(HWND hWnd)
{
	if (hWnd == NULL)
		return;

	RECT rcClient;
	if (!::GetClientRect(hWnd, &rcClient)) return;

	for (int i = rcClient.bottom - 16; i >= 0; i -= 16)
	{
		for (int q = rcClient.right - 16; q >= 0; q -= 16)
			::PostMessage(hWnd, WM_MOUSEMOVE, NULL, MAKELPARAM(i, q));

	}
}

BOOL CTrayIconTest::DeleteAlertMessage()
{
	ASSERT(m_bIsOnTray);

	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));

	nid.cbSize = sizeof(nid);
	nid.dwInfoFlags = NIIF_INFO;        // 풍선 알림아이콘 종류
	nid.uFlags = NIF_MESSAGE | NIF_INFO | NIF_ICON;
	nid.uTimeout = 0;
	nid.hWnd = m_pParentWnd->GetSafeHwnd();
	nid.uCallbackMessage = m_uCallbackMessage;
	nid.hIcon = LoadIconFromID();

	swprintf_s(nid.szInfo, L"");

	return ::Shell_NotifyIcon(NIM_MODIFY, &nid);
}
