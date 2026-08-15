// HoverButton.cpp : implementation file
//

#include "pch.h"
#include "HoverButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHoverButton

CHoverButton::CHoverButton()
{
	m_bHover		= FALSE;
	m_bTracking		= FALSE;
	m_nActivate		= 0;
	m_dwToggleGroup	= 0;
	m_bEnable		= TRUE;
	m_bMasking		= FALSE;
	m_bActiveMode	= FALSE;
	hUpBitmap = NULL;
	hDownBitmap = NULL;
	hOverBitmap = NULL;
	hDisableBitmap = NULL;
	hMaskBitmap = NULL;
}

CHoverButton::~CHoverButton()
{
}

IMPLEMENT_DYNAMIC(CHoverButton, CBitmapButton)

BEGIN_MESSAGE_MAP(CHoverButton, CBitmapButton)
	//{{AFX_MSG_MAP(CHoverButton)
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_ENABLE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover)
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHoverButton message handlers

HRESULT	CHoverButton::SetToolTipText(CString strText, BOOL bActivate)
{
	if(strText.IsEmpty() == TRUE)
		return E_FAIL;

	InitToolTip();

	if(m_ToolTip.GetToolCount() == 0)
	{
		CRect	rcButton;
		GetClientRect(rcButton);
		m_ToolTip.AddTool(this, (LPCTSTR)strText, rcButton, 1);
	}

	m_ToolTip.UpdateTipText((LPCTSTR)strText, this, 1);
	m_ToolTip.Activate(bActivate);

	return S_OK;
}

HRESULT	CHoverButton::SetToolTipText(UINT nStringId, BOOL bActivate)
{
	CString	strText;

	strText.LoadString(nStringId);

	if(strText.IsEmpty() == FALSE)
		SetToolTipText(strText, bActivate);
	else
		return E_FAIL;

	return S_OK;
}

HRESULT CHoverButton::AttachBitmap(HBITMAP hUpBitmap, HBITMAP hDownBitmap, HBITMAP hOverBitmap, HBITMAP hDisableBitmap, HBITMAP hMaskBitmap)
{
	if(hDisableBitmap != NULL)
	{
		if (m_DisableBitmap.m_hObject != NULL) {
			m_DisableBitmap.Detach();
		}
		m_DisableBitmap.Attach(hDisableBitmap);
	}

	if (hMaskBitmap != NULL)
	{
		if (m_MaskBitmap.m_hObject != NULL) {
			m_MaskBitmap.Detach();
		}
		m_MaskBitmap.Attach(hMaskBitmap);

		HRGN hMaskRgn = CreateRgnFromMaskBitmap(hMaskBitmap, RGB(255, 255, 255));
		//HRGN hMaskRgn = GenerateRegion(hMaskBitmap, 255, 255, 255);
		if (hMaskRgn)
		{
			m_MaskRgn.Attach(hMaskRgn);
			SetWindowRgn(m_MaskRgn, TRUE);
			SelectClipRgn((HDC)GetDC(), m_MaskRgn);
			m_bMasking = TRUE;
		}
	}

	if (hUpBitmap != NULL)
	{
		if (m_UpBitmap.m_hObject != NULL) {
			m_UpBitmap.Detach();
		}
		m_UpBitmap.Attach(hUpBitmap);
	}

	if (hDownBitmap != NULL)
	{
		if (m_DownBitmap.m_hObject != NULL) {
			m_DownBitmap.Detach();
		}
		m_DownBitmap.Attach(hDownBitmap);
	}

	if (hOverBitmap != NULL)
	{
		if (m_OverBitmap.m_hObject != NULL) {
			m_OverBitmap.Detach();
		}
		m_OverBitmap.Attach(hOverBitmap);
	}

	if (m_UpBitmap.m_hObject != NULL)
	{
		BITMAP	bitmap;
		m_UpBitmap.GetBitmap(&bitmap);
		SetWindowPos(NULL, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SWP_NOMOVE | SWP_NOZORDER);
	}

	return S_OK;
}

HRESULT CHoverButton::LoadBitmap(CString lpszUpBitmap, CString lpszDownBitmap, CString lpszOverBitmap, CString lpszDisableBitmap, CString lpszMaskBitmap)
{
	BITMAP	bitmap;
	/*HBITMAP hUpBitmap = NULL;
	HBITMAP hDownBitmap = NULL;
	HBITMAP hOverBitmap = NULL;
	HBITMAP hDisableBitmap = NULL;
	HBITMAP	hMaskBitmap = NULL;*/

	if(lpszDisableBitmap != _T(""))
	{
		if(hDisableBitmap){
			DeleteObject(hDisableBitmap);
			hDisableBitmap = NULL;
		}
		hDisableBitmap = (HBITMAP)LoadImage(NULL, lpszDisableBitmap, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE|LR_CREATEDIBSECTION|LR_DEFAULTSIZE);
		if(!hDisableBitmap)
			return E_FAIL;
		if(m_DisableBitmap.m_hObject != NULL){
			m_DisableBitmap.Detach();
		}
		m_DisableBitmap.Attach(hDisableBitmap);
	}

	if(lpszMaskBitmap != _T(""))
	{
		if(hMaskBitmap){
			DeleteObject(hMaskBitmap);
			hMaskBitmap = NULL;
		}
		hMaskBitmap = (HBITMAP)LoadImage(NULL, lpszMaskBitmap, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE|LR_CREATEDIBSECTION|LR_DEFAULTSIZE);
		if(!hMaskBitmap)
			return E_FAIL;

		if(m_MaskBitmap.m_hObject != NULL){
			m_MaskBitmap.Detach();
		}
		m_MaskBitmap.Attach(hMaskBitmap);

		HRGN hMaskRgn = CreateRgnFromMaskBitmap(hMaskBitmap, RGB(255, 255, 255));
		//HRGN hMaskRgn = GenerateRegion(hMaskBitmap, 255, 255, 255);
		if(hMaskRgn)
		{
			m_MaskRgn.Attach(hMaskRgn);
			SetWindowRgn(m_MaskRgn, TRUE);
			SelectClipRgn((HDC)GetDC(), m_MaskRgn);
			m_bMasking = TRUE;
		}	
	}
	if(hUpBitmap){
		DeleteObject(hUpBitmap);
		hUpBitmap = NULL;
	}
	hUpBitmap = (HBITMAP)LoadImage(NULL, lpszUpBitmap, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE|LR_CREATEDIBSECTION|LR_DEFAULTSIZE);
 	if(!hUpBitmap)
		return E_FAIL;

	if(m_UpBitmap.m_hObject != NULL){
		m_UpBitmap.Detach();
	}
	m_UpBitmap.Attach(hUpBitmap);

	if(hDownBitmap){
		DeleteObject(hDownBitmap);
		hDownBitmap = NULL;
	}
	hDownBitmap = (HBITMAP)LoadImage(NULL, lpszDownBitmap, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE|LR_CREATEDIBSECTION|LR_DEFAULTSIZE);
	if(!hDownBitmap)
		return E_FAIL;

	if(m_DownBitmap.m_hObject != NULL){
		m_DownBitmap.Detach();
	}
	m_DownBitmap.Attach(hDownBitmap);

	if(hOverBitmap){
		DeleteObject(hOverBitmap);
		hOverBitmap = NULL;
	}
	hOverBitmap = (HBITMAP)LoadImage(NULL, (LPCTSTR)lpszOverBitmap, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE|LR_CREATEDIBSECTION|LR_DEFAULTSIZE);
	if(!hOverBitmap)
		return E_FAIL;

	if(m_OverBitmap.m_hObject != NULL){
		m_OverBitmap.Detach();
	}
	m_OverBitmap.Attach(hOverBitmap);

	m_UpBitmap.GetBitmap(&bitmap);
	SetWindowPos(NULL, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SWP_NOMOVE|SWP_NOZORDER);

	return S_OK;
}

HRESULT CHoverButton::LoadBitmap(UINT nUpBitmap, UINT nDownBitmap, UINT nOverBitmap, UINT nDisableBitmap, UINT nMaskBitmap)
{
	BITMAP	bitmap;

	if(nDisableBitmap != 0)
		m_DisableBitmap.LoadBitmap(nDisableBitmap);

	if(nMaskBitmap != 0)
	{
		m_MaskBitmap.LoadBitmap(nMaskBitmap);
		HRGN hMaskRgn = CreateRgnFromMaskBitmap((HBITMAP)m_MaskBitmap, RGB(255, 255, 255));
		if(hMaskRgn)
		{
			m_MaskRgn.Attach(hMaskRgn);
			SetWindowRgn(m_MaskRgn, TRUE);
			SelectClipRgn((HDC)GetDC(), m_MaskRgn);
			m_bMasking = TRUE;
		}	
	}

	m_UpBitmap.LoadBitmap(nUpBitmap);
	m_DownBitmap.LoadBitmap(nDownBitmap);
	m_OverBitmap.LoadBitmap(nOverBitmap);

	m_UpBitmap.GetBitmap(&bitmap);

	SetWindowPos(NULL, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SWP_NOMOVE|SWP_NOOWNERZORDER);

	return S_OK;
}

HRESULT CHoverButton::DeleteBitmap()
{
	BOOL bRet(FALSE);
	bRet &= m_UpBitmap.DeleteObject();
	bRet &= m_DownBitmap.DeleteObject();
	bRet &= m_OverBitmap.DeleteObject();

	return bRet ? S_OK : S_FALSE;
}

HRESULT	CHoverButton::ActivateToolTip(BOOL bActivate)
{
	if(m_ToolTip.GetToolCount() == 0)
		return E_FAIL;

	m_ToolTip.Activate(bActivate);

	return S_OK;
}

HRESULT	CHoverButton::InitToolTip()
{
	if(m_ToolTip.m_hWnd == NULL)
	{
		m_ToolTip.Create(this);
		m_ToolTip.Activate(FALSE);
	}

	return S_OK;
}

void CHoverButton::OnMouseMove(UINT nFlags, CPoint point) 
{
	if(!m_bTracking)
	{
		TRACKMOUSEEVENT	tme;
		tme.cbSize		= sizeof(TRACKMOUSEEVENT);
		tme.hwndTrack	= m_hWnd;
		tme.dwFlags		= TME_LEAVE|TME_HOVER;
		tme.dwHoverTime	= 1;
		m_bTracking = _TrackMouseEvent(&tme);
	}
	CBitmapButton::OnMouseMove(nFlags, point);
}

BOOL CHoverButton::PreTranslateMessage(MSG* pMsg) 
{
	InitToolTip();
	m_ToolTip.RelayEvent(pMsg);

	return CBitmapButton::PreTranslateMessage(pMsg);
}

LRESULT CHoverButton::OnMouseHover(WPARAM wParam, LPARAM lParam)
{
	m_bHover = TRUE;
	Invalidate(FALSE);
	
	return 0;
}

LRESULT CHoverButton::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	m_bHover	= FALSE;
	m_bTracking = FALSE;
	Invalidate(FALSE);
	
	return 0;
}


void CHoverButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	ASSERT(lpDrawItemStruct != NULL);

	CBitmap *pBitmap = NULL;
	UINT state = lpDrawItemStruct->itemState;

	if((state & ODS_SELECTED) && m_DownBitmap.m_hObject != NULL)
	{
		pBitmap = &m_DownBitmap;
	}
	else 
	{
		if(m_bHover && m_OverBitmap.m_hObject != NULL)
		{
			if(!m_bActiveMode) 
				pBitmap = &m_OverBitmap;
			else
			{
				if(m_nActivate == 0 && m_UpBitmap.m_hObject != NULL)
				{
					pBitmap = &m_UpBitmap;
				}
				else if(m_nActivate == 1 && m_OverBitmap.m_hObject != NULL)
				{
					pBitmap = &m_OverBitmap;
				}
			}
		}
		else if(m_DisableBitmap.m_hObject != NULL && !m_bEnable)
		{
			pBitmap = &m_DisableBitmap;
		}
		else if(m_UpBitmap.m_hObject != NULL && m_bEnable)
		{
			if(m_nActivate == 0 && m_UpBitmap.m_hObject != NULL)
			{
				pBitmap = &m_UpBitmap;
			}
			else if(m_nActivate == 1 && m_OverBitmap.m_hObject != NULL)
			{
				pBitmap = &m_OverBitmap;
			}
			else if(m_nActivate == 2 && m_DownBitmap.m_hObject != NULL)
			{
				pBitmap = &m_DownBitmap;
			}
			else if(m_nActivate == 3 && m_DisableBitmap.m_hObject != NULL)
			{
				pBitmap = &m_DisableBitmap;
			}
		}
	}
	
	CRect	rect;
	CDC	*pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	rect.CopyRect(&lpDrawItemStruct->rcItem);

	DrawBitmap(pDC, pBitmap, rect);
}

BOOL CHoverButton::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CHoverButton::OnEnable(BOOL bEnable) 
{
	CBitmapButton::OnEnable(bEnable);
	
	m_bEnable = bEnable;	
	Invalidate(FALSE);
}

/*
void CHoverButton::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if(m_dwToggleGroup != 0)
	{
		m_bActivate ^= TRUE;
		UnToggleOtherButtons();
		Invalidate(FALSE);
	}

	CBitmapButton::OnLButtonDown(nFlags, point);
}

HRESULT CHoverButton::UnToggleOtherButtons()
{
	CWnd *pParent = GetParent();
	ASSERT(pParent);

	CWnd *pWnd = pParent->GetNextDlgTabItem((CWnd *)this);
	while(pWnd && (pWnd != this))
	{
		if ((((CButton*)pWnd)->GetButtonStyle() & BS_OWNERDRAW) && 
			 ((CHoverButton *)pWnd)->m_dwToggleGroup == m_dwToggleGroup)
		{
			((CHoverButton *)pWnd)->UnActivateButton(FALSE);
		}
		pWnd = pParent->GetNextDlgTabItem(pWnd);
	}

	return S_OK;
}
*/
VOID CHoverButton::ToggleMode(DWORD dwToggleGroup) 
{ 
	m_dwToggleGroup = dwToggleGroup; 
}

VOID CHoverButton::UnActivateButton(int nActivate)
{
	m_nActivate = nActivate;
	Invalidate(FALSE);
}

HRGN CHoverButton::CreateRgnFromMaskBitmap(HBITMAP hMaskBitmap, COLORREF crMask)
{
	if(!hMaskBitmap)
		return NULL;

	BITMAP bm;
	GetObjectW(hMaskBitmap, sizeof(bm), &bm);

	CDC bitmapDC;
	bitmapDC.CreateCompatibleDC(GetDC());
	bitmapDC.SelectObject(hMaskBitmap);

	const DWORD RDHDR = sizeof(RGNDATAHEADER);
	const DWORD MAX_BUF = 40;

	LPRECT	lpRects;
	DWORD	dwBlocks = 0;

	int		ii, jj;
	int		nFirst = 0;

	BOOL	bWasFirst = FALSE;
	BOOL	bIsMask;

	RGNDATAHEADER *pRgnData = (RGNDATAHEADER *)new BYTE[RDHDR + ++dwBlocks * MAX_BUF * sizeof(RECT)];
	memset(pRgnData, 0, RDHDR + dwBlocks * MAX_BUF * sizeof(RECT));

	// Fill it by default.
	pRgnData->dwSize	= RDHDR;
	pRgnData->iType		= RDH_RECTANGLES;
	pRgnData->nCount	= 0;

	for(ii = 0; ii < bm.bmHeight; ii++)
	{
		for(jj = 0; jj < bm.bmWidth; jj++)
		{
			bIsMask = (bitmapDC.GetPixel(jj, bm.bmHeight - ii - 1) != crMask);
			if(bWasFirst && ((bIsMask && (jj == (bm.bmWidth - 1))) || (bIsMask ^ (jj < bm.bmWidth))))
			{
				lpRects = (LPRECT)((LPBYTE)pRgnData + RDHDR);
				lpRects[pRgnData->nCount++] = CRect(nFirst, bm.bmHeight - ii - 1, jj + (jj = (bm.bmWidth - 1)), bm.bmHeight - ii);

				if(pRgnData->nCount >= dwBlocks * MAX_BUF)
				{
					LPBYTE pRgnDataNew = new BYTE[RDHDR + ++dwBlocks * MAX_BUF * sizeof(RECT)];
					memcpy(pRgnDataNew, pRgnData, RDHDR + (dwBlocks - 1) * MAX_BUF * sizeof(RECT));
					delete pRgnData;
					pRgnData = (RGNDATAHEADER *)pRgnDataNew;
				}
				bWasFirst = FALSE;
			}
			else if(!bWasFirst && bIsMask)
			{
				nFirst = jj;
				bWasFirst = TRUE;
			}
		}
	}

	bitmapDC.DeleteDC();

	HRGN hRgn = CreateRectRgn(0, 0, 0, 0);
	ASSERT(hRgn != NULL);
	lpRects = (LPRECT)((LPBYTE)pRgnData + RDHDR);
	for(ii = 0; ii < (int)pRgnData->nCount; ii++)
	{
		HRGN hTempRgn = CreateRectRgn(lpRects[ii].left, lpRects[ii].top, lpRects[ii].right, lpRects[ii].bottom);
		VERIFY(CombineRgn(hRgn, hRgn, hTempRgn, RGN_OR) != ERROR);
		if(hTempRgn)
			DeleteObject(hTempRgn);
	}

	ASSERT(hRgn != NULL);

	delete pRgnData;
	return hRgn;
}

//void CHoverButton::DrawBitmap(CDC *pDC, HBITMAP hBitmap, CRect rc)
void CHoverButton::DrawBitmap(CDC *pDC, CBitmap *pBitmap, CRect rc)
{
	if(!pBitmap)
		return;

	if(!m_bMasking)
	{
		CDC memDC;
		memDC.CreateCompatibleDC(pDC);

		CBitmap *pOldBitmap = (CBitmap *)memDC.SelectObject(pBitmap);
		if(pOldBitmap == NULL)
			return;

		pDC->BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);
		memDC.SelectObject(pOldBitmap);
	}
	else
	{
		CDC BitmapDC, MaskDC;
		BitmapDC.CreateCompatibleDC(pDC);
		BitmapDC.SelectObject(pBitmap);

		MaskDC.CreateCompatibleDC(pDC);
		MaskDC.SelectObject(&m_MaskBitmap);

		CDC MemDC;
		if(!MemDC.CreateCompatibleDC(pDC))
			return;
		
		CBitmap bitmap;
		bitmap.CreateCompatibleBitmap(pDC, rc.Width(), rc.Height());
		MemDC.SelectObject(&bitmap);

		MemDC.BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), pDC, 0, 0, SRCCOPY);

		MemDC.BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), &BitmapDC, 0, 0, SRCINVERT);
		MemDC.BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), &MaskDC, 0, 0, SRCAND);
		MemDC.BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), &BitmapDC, 0, 0, SRCINVERT);
		
		pDC->BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), &MemDC, 0, 0, SRCCOPY);

		MemDC.DeleteDC();
		bitmap.DeleteObject();
		MaskDC.DeleteDC();
		BitmapDC.DeleteDC();
	}
}

void CHoverButton::SetActiveMode(bool bActiveMode)
{
	m_bActiveMode = bActiveMode;
}
