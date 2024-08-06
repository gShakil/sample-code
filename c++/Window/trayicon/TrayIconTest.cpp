// TrayIconTest.cpp : ���� �����Դϴ�.
//

#include "pch.h"
#include "TrayIconTest.h"

//RefreshTrayIcon()���� ���ȴ�
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



// CTrayIconTest �޽��� ó�����Դϴ�.
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
	NIF_ICON	: hicon ���.
	NIF_MESSAGE : uCallbackMessage ���.
	NIF_TIP		: szTip ���.
	NIF_INFO	: �˸� ���

	Shell_NotifyIcon �Լ��� 3���� �뵵�� ���ȴ�.
	Shell_NotifyIcon(NIM_ADD, &nid)      - Ʈ���� �߰�
	Shell_NotifyIcon(NIM_MODIFY, &nid)   - Ʈ���� ����
	Shell_NotifyIcon(NIM_DELETE, &nid)   - Ʈ���� ����

	*/
	ASSERT(m_pParentWnd && m_uCallbackMessage && m_IconResourceID != 0);
	if (m_pParentWnd == NULL)
		return FALSE;

	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));

	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.uID = 0;    // Ʈ���� ����ü ���̵�.
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	nid.hWnd = m_pParentWnd->GetSafeHwnd();
	//m_NotiData.hIcon = AfxGetApp()->LoadIconW(IDR_MAINFRAME);
	nid.hIcon = LoadIconFromID();
	nid.uCallbackMessage = m_uCallbackMessage;

	if (pwszMessage == NULL)
	{
		CString strWindow = _T("");
		m_pParentWnd->GetWindowText(strWindow);
		lstrcpy(nid.szTip, strWindow.GetBuffer());// Ʈ���̾����ܿ� ���콺�� �÷��ξ��� �� ǥ�õǴ� �ؽ�Ʈ
	}
	else
		lstrcpy(nid.szTip, pwszMessage);// Ʈ���̾����ܿ� ���콺�� �÷��ξ��� �� ǥ�õǴ� �ؽ�Ʈ

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

	if (Shell_NotifyIcon(NIM_DELETE, &nid)) //Ʈ���̾����� ����
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

	//BOOL bRet = ::Shell_NotifyIcon(NIM_DELETE, &nid); //Ʈ���̾����� ����
	//if (!bRet)
	//{
	//	::AfxMessageBox(_T("Ʈ���̾����� ���Ž���"));
	//	return;
	//}
	//AfxGetApp()->m_pMainWnd->ShowWindow(SW_SHOW); //������ Ȱ��ȭ
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
	NIIF_ERROR �� �ϳ�
	*/
	if (m_pParentWnd == NULL)
		return FALSE;

	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));

	nid.cbSize				= sizeof(nid);
	nid.dwInfoFlags			= dwInfoFlag;        // ǳ�� �˸������� ����
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
	//������ NIM_ADD �� ���� �����ϴ� ���̱� ������ NIM_MODIFY�� ����մϴ�.
	return ::Shell_NotifyIcon(NIM_MODIFY, &nid);
}
BOOL CTrayIconTest::RefreshTrayIcon()
{
	//���α׷��� ������ ����� ���� ��������ִ� Ʈ���� �������� ����������
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

																										// WinXP ������ SysPager ���� ����            
			if (hWnd_SysPager)
			{
				hWnd_ToolbarWindow32 = ::FindWindowEx(hWnd_SysPager, NULL, _T("ToolbarWindow32"), NULL);
			}

			// Win2000 �� ��쿡�� SysPager �� ���� TrayNotifyWnd -> ToolbarWindow32 �� �Ѿ��
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

																										// WinXP ������ SysPager ���� ����            
			if (hWnd_SysPager)
			{
				hWnd_ToolbarWindow32 = ::FindWindowEx(hWnd_SysPager, NULL, _T("ToolbarWindow32"), NULL);
			}

			// Win2000 �� ��쿡�� SysPager �� ���� TrayNotifyWnd -> ToolbarWindow32 �� �Ѿ��
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
	// ������ 10
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
	nid.dwInfoFlags = NIIF_INFO;        // ǳ�� �˸������� ����
	nid.uFlags = NIF_MESSAGE | NIF_INFO | NIF_ICON;
	nid.uTimeout = 0;
	nid.hWnd = m_pParentWnd->GetSafeHwnd();
	nid.uCallbackMessage = m_uCallbackMessage;
	nid.hIcon = LoadIconFromID();

	swprintf_s(nid.szInfo, L"");

	return ::Shell_NotifyIcon(NIM_MODIFY, &nid);
}
