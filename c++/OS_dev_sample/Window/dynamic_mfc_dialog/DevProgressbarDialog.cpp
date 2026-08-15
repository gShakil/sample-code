#include "pch.h"
#include "DevProgressbarDialog.h"

constexpr int RESOURCE_NUM_START = 1100;
constexpr int TOPTEXT_RESOURCENUM = RESOURCE_NUM_START + 1;
constexpr int BOTTEXT_RESOURCENUM = RESOURCE_NUM_START + 2;
constexpr int PROGRESSBAR_RESOURCENUM = RESOURCE_NUM_START + 3;

#define UM_SET_CUSTOM_TEXT (WM_USER + 0x0001)

BEGIN_MESSAGE_MAP(DevProgressbarDialog, CDynDialogEx)
	//ON_BN_CLICKED(EXPORT_BTN_RESOURCENUM, &DevVersionDialog::OnBnClickedExport)
	ON_MESSAGE(UM_SET_CUSTOM_TEXT, &DevProgressbarDialog::OnReceiveSetText)
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()



DevProgressbarDialog::DevProgressbarDialog(CWnd* pParentWnd)
	:CDynDialogEx(pParentWnd)
	, m_strTopText(_T("잠시만 기다려주세요 .."))
	, m_strBottomText(_T(""))
{
}

DevProgressbarDialog::~DevProgressbarDialog()
{

}

void DevProgressbarDialog::ShowWindow()
{
	CFont* pft = AfxGetMainWnd()->GetFont();
	SetFont(pft);
	SetUseSystemButtons(FALSE);
	//원하는 사이즈보다 크게 지정되어 InitDialog에서 사이즈를 재조정한다.
	CRect rc;
	rc.left = 0;
	rc.top = 0;
	rc.right = 200;
	rc.bottom = 10;
	SetDlgRect(&rc);

	// Top Text
	AddDlgControl(
		STATICTEXT,
		m_strTopText,
		STYLE_STATIC,
		EXSTYLE_STATIC,
		CRect(10, 5, 125, 12),
		NULL,
		TOPTEXT_RESOURCENUM
	);
	// ProgressBar
	AddDlgControl(
		PROGRES,
		_T(""),
		STYLE_STATIC,
		EXSTYLE_STATIC,
		CRect(10, 16, 195, 31),
		NULL,
		PROGRESSBAR_RESOURCENUM
	);

 	// Bot Text
	AddDlgControl(
		STATICTEXT,
		m_strBottomText,
		STYLE_STATIC,
		EXSTYLE_STATIC,
		CRect(10, 34, 125, 41),
		NULL,
		BOTTEXT_RESOURCENUM
	);

	DoModal();
}

void DevProgressbarDialog::SetTopText(const wchar_t* text)
{
	m_strTopText = text;
	if (IsWindow(GetSafeHwnd()))
		PostMessage(UM_SET_CUSTOM_TEXT, NULL, NULL);
}

void DevProgressbarDialog::SetBottomText(const wchar_t* text)
{
	m_strBottomText = text;
	if (IsWindow(GetSafeHwnd()))
		PostMessage(UM_SET_CUSTOM_TEXT, NULL, NULL);
}

BOOL DevProgressbarDialog::OnInitDialog()
{
	CDynDialogEx::OnInitDialog();

	// 다이얼로그 기본 스타일 제거 (캡션, 테두리)
	ModifyStyle(WS_CAPTION | WS_BORDER, 0, SWP_FRAMECHANGED);  
	// 확장 스타일 제거 (얇은 테두리 제거)
	ModifyStyleEx(WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE, 0, SWP_FRAMECHANGED);
	// WS_DLGFRAME도 제거
	SetWindowLong(GetSafeHwnd(), GWL_STYLE, GetWindowLong(GetSafeHwnd(), GWL_STYLE) & ~WS_DLGFRAME);
	//변경 적용
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);

// 	CRect rc;
// 	GetDlgRect(&rc);
 	SetWindowPos(&CWnd::wndTopMost, 0, 0, 365, 95, SWP_NOMOVE); // 15, 38: 다이얼로그 크기 맞추기 위한 가산치

 	CProgressCtrl* pBar = (CProgressCtrl*)GetDlgItem(PROGRESSBAR_RESOURCENUM);
	pBar->ModifyStyle(0, PBS_MARQUEE, 0);
 	pBar->SetMarquee(TRUE, 30);


// 	CDynDialogItemEx* pDynDialogItemEx = NULL;
// 	for (int i = 0; i < m_arrDlgItemPtr.GetSize(); i++)
// 	{
// 		pDynDialogItemEx = m_arrDlgItemPtr[i];
// 		if (pDynDialogItemEx != NULL)
// 		{
// 			switch (pDynDialogItemEx->GetControlID())
// 			{
// 
// 			case PROGRESSBAR_RESOURCENUM:
// 				BOOL b1 = ModifyStyle(pDynDialogItemEx->GetSafeHwnd(), 0, PBS_MARQUEE, 0);
// 				BOOL b2 = ::SendMessage(pDynDialogItemEx->GetSafeHwnd(), PBM_SETMARQUEE, (WPARAM)TRUE, (LPARAM)30);
// 				break;
// 			}
// 
// 
// 
// 			UINT nid = pDynDialogItemEx->GetControlID();
// 			UINT nid2 = pDynDialogItemEx->GetControlID();
// 
// 			//if (nid != 3)
// 			//pDynDialogItemEx->::MoveWindow(0, 0, 200, 200);
// 		}
// 	}
	return TRUE;
}

LRESULT DevProgressbarDialog::OnReceiveSetText(WPARAM wParam, LPARAM lParam)
{
	SetDlgItemText(TOPTEXT_RESOURCENUM, m_strTopText);
	SetDlgItemText(BOTTEXT_RESOURCENUM, m_strBottomText);

	return TRUE;
}

LRESULT DevProgressbarDialog::OnNcHitTest(CPoint point)
{
	LRESULT hit = CDialog::OnNcHitTest(point);

	// 다이얼로그를 마우스로 드래그할 수 있도록 설정
	return HTCAPTION;
}
