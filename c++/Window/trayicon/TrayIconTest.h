#pragma once


// CTrayIconTest

class CTrayIconTest //: public CWnd
{
	//DECLARE_DYNAMIC(CTrayIconTest)
	//DECLARE_MESSAGE_MAP()
public:
	CTrayIconTest();
	virtual ~CTrayIconTest();

	//void SetTrayData(CWnd *pParent, UINT uCallbackMessage, UINT uIconResourceID); // Ʈ���� ��뿡 ���ʰ� �Ǵ� ������ �Է�
	void SetTrayData(CWnd *pParent, UINT uCallbackMessage, HICON hIcon); // Ʈ���� ��뿡 ���ʰ� �Ǵ� ������ �Է�
	BOOL RefreshTrayIcon();

	BOOL ShowTray(WCHAR *pwszMessage = NULL);
	BOOL HideTray(); // Ʈ���� �����ϸ鼭 ���̾�α� ǥ��

	// dwInfoFlag: NIIF_NONE, NIIF_INFO, NIIF_WARNING, NIIF_ERROR
	// dwTimer : Window 2000, Window XP������ ��ȿ
	BOOL ShowAlertMessage(DWORD dwInfoFlag, DWORD dwTimer, const wchar_t* pwszTitle,const  wchar_t* pwszInfo); // Ʈ���̰� ��������� �� ǳ�� �˸��� �޴´�.
	BOOL DeleteAlertMessage();

protected:
	void CleanTrayIcon(HWND hWnd);
	HWND GetTrayAreaType1();
	HWND GetTrayAreaType2();

	HICON LoadIconFromID(); // SetTrayData���� ������ uIconResourceID�κ��� HICON�� ���Ϲ޴´�
	HWND FindTrayToolbarWindow();

private:
	//NOTIFYICONDATA m_NotiData;

	BOOL m_bIsOnTray;
	CWnd *m_pParentWnd;
	UINT m_uCallbackMessage;
	//UINT m_IconResourceID;
	HICON m_IconResourceID;
};


