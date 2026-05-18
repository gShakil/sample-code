
// postgreSQLsampleDlg.h: 헤더 파일
//

#pragma once
#include "postgresTester.h"


// CpostgreSQLsampleDlg 대화 상자
class CpostgreSQLsampleDlg : public CDialogEx
{
// 생성입니다.
public:
	CpostgreSQLsampleDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_POSTGRESQLSAMPLE_DIALOG };
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

private:
	nsdatabase::nspostgreSQL::postgresTester m_postgresTester;
public:
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonDisconnect();
	afx_msg void OnBnClickedButtonCreateTable();
	afx_msg void OnBnClickedButtonDropTable();
	afx_msg void OnBnClickedButtonInsertItem();
	afx_msg void OnBnClickedButtonInsertMulItem();
	afx_msg void OnBnClickedButtonClearAllItem();
	afx_msg void OnBnClickedButtonSelectOneItem();
	afx_msg void OnBnClickedButtonSelectMulItem();
	afx_msg void OnBnClickedButtonSelectMulItemStream();
	afx_msg void OnBnClickedButtonCreateDb();
	afx_msg void OnBnClickedButtonDeleteDb();
};
