
// D2XXdriverDlg.h: 헤더 파일
//

#pragma once
#include <thread>

// CD2XXdriverDlg 대화 상자
class CD2XXdriverDlg : public CDialogEx
{
// 생성입니다.
public:
	CD2XXdriverDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_D2XXDRIVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonDisconnect();

	bool m_bdoRecv;
	unsigned char m_buffer[1024];
	std::thread m_thread;
	__ENF::nsconnector_v2::ftdideviceConnector *m_pcon;
	afx_msg void OnBnClickedButtonSend();
	afx_msg void OnBnClickedButtonSend2();
	afx_msg void OnBnClickedButtonStartRecv();
	afx_msg void OnBnClickedButtonStopRecv();
	afx_msg void OnBnClickedButtonSetSpeed();
};
