#pragma once


// CTrayIconTest

class CTrayIconTest //: public CWnd
{
	//DECLARE_DYNAMIC(CTrayIconTest)
	//DECLARE_MESSAGE_MAP()
public:
	CTrayIconTest();
	virtual ~CTrayIconTest();

	//void SetTrayData(CWnd *pParent, UINT uCallbackMessage, UINT uIconResourceID); // 트레이 사용에 기초가 되는 변수들 입력
	void SetTrayData(CWnd *pParent, UINT uCallbackMessage, HICON hIcon); // 트레이 사용에 기초가 되는 변수들 입력
	BOOL RefreshTrayIcon();

	BOOL ShowTray(WCHAR *pwszMessage = NULL);
	BOOL HideTray(); // 트레이 제거하면서 다이얼로그 표시

	// dwInfoFlag: NIIF_NONE, NIIF_INFO, NIIF_WARNING, NIIF_ERROR
	// dwTimer : Window 2000, Window XP에서만 유효
	BOOL ShowAlertMessage(DWORD dwInfoFlag, DWORD dwTimer, const wchar_t* pwszTitle,const  wchar_t* pwszInfo); // 트레이가 숨김상태일 때 풍선 알림을 받는다.
	BOOL DeleteAlertMessage();

protected:
	void CleanTrayIcon(HWND hWnd);
	HWND GetTrayAreaType1();
	HWND GetTrayAreaType2();

	HICON LoadIconFromID(); // SetTrayData에서 설정된 uIconResourceID로부터 HICON을 리턴받는다
	HWND FindTrayToolbarWindow();

private:
	//NOTIFYICONDATA m_NotiData;

	BOOL m_bIsOnTray;
	CWnd *m_pParentWnd;
	UINT m_uCallbackMessage;
	//UINT m_IconResourceID;
	HICON m_IconResourceID;
};


