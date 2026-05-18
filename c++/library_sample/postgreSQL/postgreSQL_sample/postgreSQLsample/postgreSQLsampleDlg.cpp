
// postgreSQLsampleDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "postgreSQLsample.h"
#include "postgreSQLsampleDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CpostgreSQLsampleDlg 대화 상자



CpostgreSQLsampleDlg::CpostgreSQLsampleDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_POSTGRESQLSAMPLE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CpostgreSQLsampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CpostgreSQLsampleDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CpostgreSQLsampleDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, &CpostgreSQLsampleDlg::OnBnClickedButtonDisconnect)
	ON_BN_CLICKED(IDC_BUTTON_CREATE_TABLE, &CpostgreSQLsampleDlg::OnBnClickedButtonCreateTable)
	ON_BN_CLICKED(IDC_BUTTON_DROP_TABLE, &CpostgreSQLsampleDlg::OnBnClickedButtonDropTable)
	ON_BN_CLICKED(IDC_BUTTON_INSERT_ITEM, &CpostgreSQLsampleDlg::OnBnClickedButtonInsertItem)
	ON_BN_CLICKED(IDC_BUTTON_INSERT_MUL_ITEM, &CpostgreSQLsampleDlg::OnBnClickedButtonInsertMulItem)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_ALL_ITEM, &CpostgreSQLsampleDlg::OnBnClickedButtonClearAllItem)
	ON_BN_CLICKED(IDC_BUTTON_SELECT_ONE_ITEM, &CpostgreSQLsampleDlg::OnBnClickedButtonSelectOneItem)
	ON_BN_CLICKED(IDC_BUTTON_SELECT_MUL_ITEM, &CpostgreSQLsampleDlg::OnBnClickedButtonSelectMulItem)
	ON_BN_CLICKED(IDC_BUTTON_SELECT_MUL_ITEM_STREAM, &CpostgreSQLsampleDlg::OnBnClickedButtonSelectMulItemStream)
	ON_BN_CLICKED(IDC_BUTTON_CREATE_DB, &CpostgreSQLsampleDlg::OnBnClickedButtonCreateDb)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_DB, &CpostgreSQLsampleDlg::OnBnClickedButtonDeleteDb)
END_MESSAGE_MAP()


// CpostgreSQLsampleDlg 메시지 처리기

BOOL CpostgreSQLsampleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	SetDlgItemText(IDC_EDIT_DB_IP, _T("localhost"));
	SetDlgItemText(IDC_EDIT_DB_PORT, _T("5432"));
	SetDlgItemText(IDC_EDIT_DB_DBNAME, _T("postgres"));
	SetDlgItemText(IDC_EDIT_DB_USERNAME, _T("postgres"));
	SetDlgItemText(IDC_EDIT_DB_PASSWORD, _T("vhtmsus12!"));
	SetDlgItemText(IDC_EDIT_DB_CON_TIMEOUT, _T("10"));


	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CpostgreSQLsampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CpostgreSQLsampleDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CpostgreSQLsampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CpostgreSQLsampleDlg::OnBnClickedButtonConnect()
{
	CString ip, dbname, user, pw;
	GetDlgItemText(IDC_EDIT_DB_IP, ip);
	GetDlgItemText(IDC_EDIT_DB_DBNAME, dbname);
	GetDlgItemText(IDC_EDIT_DB_USERNAME, user);
	GetDlgItemText(IDC_EDIT_DB_PASSWORD, pw);
	int port = GetDlgItemInt(IDC_EDIT_DB_PORT);
	int timeout = GetDlgItemInt(IDC_EDIT_DB_CON_TIMEOUT);

	nsdatabase::nspostgreSQL::connectionInfo coninfo;
	coninfo.host = CT2A(ip);
	coninfo.port = port;
	coninfo.dbname = CT2A(dbname);
	coninfo.user = CT2A(user);
	coninfo.password = CT2A(pw);;
	coninfo.connect_timeout = timeout;

	const bool ret = m_postgresTester.connect(std::move(coninfo));
	AfxMessageBox(_T("connect result : ") + CString(ret ? _T("success") : _T("fail")));
}

void CpostgreSQLsampleDlg::OnBnClickedButtonDisconnect()
{
	m_postgresTester.disconnect();
	AfxMessageBox(_T("disconnected"));
}

void CpostgreSQLsampleDlg::OnBnClickedButtonCreateTable()
{
	const bool ret = m_postgresTester.createTable();
	AfxMessageBox(_T("create table : ") + CString(ret ? _T("success") : _T("fail")));
}

void CpostgreSQLsampleDlg::OnBnClickedButtonDropTable()
{
	const bool ret = m_postgresTester.dropTable();
	AfxMessageBox(_T("drop table : ") + CString(ret ? _T("success") : _T("fail")));
}

void CpostgreSQLsampleDlg::OnBnClickedButtonInsertItem()
{
	const bool ret = m_postgresTester.insert_one_item();
	AfxMessageBox(_T("insert_one_item : ") + CString(ret ? _T("success") : _T("fail")));
}

void CpostgreSQLsampleDlg::OnBnClickedButtonInsertMulItem()
{
	const bool ret = m_postgresTester.insert_five_item();
	AfxMessageBox(_T("insert_one_item : ") + CString(ret ? _T("success") : _T("fail")));
}

void CpostgreSQLsampleDlg::OnBnClickedButtonClearAllItem()
{
	const bool ret = m_postgresTester.delete_all_item();
	AfxMessageBox(_T("delete_all_item : ") + CString(ret ? _T("success") : _T("fail")));
}

void CpostgreSQLsampleDlg::OnBnClickedButtonSelectOneItem()
{
	const bool ret = m_postgresTester.select_one_item();
	AfxMessageBox(_T("delete_all_item : ") + CString(ret ? _T("success") : _T("fail")));
}

void CpostgreSQLsampleDlg::OnBnClickedButtonSelectMulItem()
{
	const bool ret = m_postgresTester.select_five_item();
	AfxMessageBox(_T("select_five_item : ") + CString(ret ? _T("success") : _T("fail")));
}

void CpostgreSQLsampleDlg::OnBnClickedButtonSelectMulItemStream()
{
	const bool ret = m_postgresTester.select_five_item_stream();
	AfxMessageBox(_T("select_five_item_stream : ") + CString(ret ? _T("success") : _T("fail")));
}

void CpostgreSQLsampleDlg::OnBnClickedButtonCreateDb()
{
	const bool ret = m_postgresTester.createDatabase();
	AfxMessageBox(_T("createDatabase : ") + CString(ret ? _T("success") : _T("fail")));
}

void CpostgreSQLsampleDlg::OnBnClickedButtonDeleteDb()
{
	const bool ret = m_postgresTester.dropDatabase();
	AfxMessageBox(_T("dropDatabase : ") + CString(ret ? _T("success") : _T("fail")));
}
