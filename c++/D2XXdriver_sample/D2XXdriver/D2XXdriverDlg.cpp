
// D2XXdriverDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "D2XXdriver.h"
#include "D2XXdriverDlg.h"
#include "afxdialogex.h"
#include <string>
#include <iostream>

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


// CD2XXdriverDlg 대화 상자



CD2XXdriverDlg::CD2XXdriverDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_D2XXDRIVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CD2XXdriverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CD2XXdriverDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CD2XXdriverDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, &CD2XXdriverDlg::OnBnClickedButtonDisconnect)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CD2XXdriverDlg::OnBnClickedButtonSend)
	ON_BN_CLICKED(IDC_BUTTON_SEND2, &CD2XXdriverDlg::OnBnClickedButtonSend2)
	ON_BN_CLICKED(IDC_BUTTON_START_RECV, &CD2XXdriverDlg::OnBnClickedButtonStartRecv)
	ON_BN_CLICKED(IDC_BUTTON_STOP_RECV, &CD2XXdriverDlg::OnBnClickedButtonStopRecv)
	ON_BN_CLICKED(IDC_BUTTON_SET_SPEED, &CD2XXdriverDlg::OnBnClickedButtonSetSpeed)
END_MESSAGE_MAP()

// CD2XXdriverDlg 메시지 처리기
BOOL CD2XXdriverDlg::OnInitDialog()
{
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

	SetDlgItemText(IDC_EDIT_COM, _T("COM11"));

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CD2XXdriverDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CD2XXdriverDlg::OnPaint()
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
HCURSOR CD2XXdriverDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CD2XXdriverDlg::OnBnClickedButtonConnect()
{
	CString s;
	GetDlgItemText(IDC_EDIT_COM, s);
	m_pcon = new __ENF::nsconnector_v2::ftdideviceConnector(CStringA(s));
	const bool ret = m_pcon->connect();

	CString str;
	str.Format(_T("con ret: %d"), ret? 1 : 0);
	AfxMessageBox(str);
}


void CD2XXdriverDlg::OnBnClickedButtonDisconnect()
{
	if (m_pcon)
	{
		OnBnClickedButtonStopRecv();
		m_pcon->disconnect();
		delete m_pcon;
		AfxMessageBox(_T("disconnected"));
	}
}


void CD2XXdriverDlg::OnBnClickedButtonSend()
{
	if (m_pcon)
	{
		std::vector<unsigned char> v{0xFF, 0xFA, 0x02, 0xA0, 0x8A, 0x2C};
		m_pcon->send(std::move(v));
	}
}


void CD2XXdriverDlg::OnBnClickedButtonSend2()
{
	if (m_pcon)
	{
		std::vector<unsigned char> v{ 0xFF, 0xFA, 0x02, 0xA0, 0xDB, 0x7D };
		m_pcon->send(std::move(v));
	}
}


void CD2XXdriverDlg::OnBnClickedButtonStartRecv()
{
	if (m_pcon)
	{
		m_bdoRecv = true;
		memset(m_buffer, NULL, _countof(m_buffer));
		m_thread = std::thread(
			[&]()
			{
				while (m_bdoRecv)
				{
					if (m_pcon->waitrecvsignal())
					{
						std::int64_t recv = 0;
						do 
						{
							recv = m_pcon->recv(m_buffer, _countof(m_buffer));
						} while (recv > 0);
						
					}
				
				}
			}
		);
	}
}


void CD2XXdriverDlg::OnBnClickedButtonStopRecv()
{
	if (m_pcon)
	{
		m_bdoRecv = false;
		if (m_thread.joinable())
		{
			m_thread.join();
		}
	}
}


void CD2XXdriverDlg::OnBnClickedButtonSetSpeed()
{
	if (m_pcon)
	{
		std::vector<unsigned char> v{ 0xFF, 0xFA, 0x03, 0xB7, 0x06, 0x00, 0xC0 };
		m_pcon->send(std::move(v));
	}
}
