#if !defined(AFX_HOVERBUTTON_H__04E71DF2_DB04_49F6_8F40_E5DB66772AFF__INCLUDED_)
#define AFX_HOVERBUTTON_H__04E71DF2_DB04_49F6_8F40_E5DB66772AFF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HoverButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHoverButton window

class CHoverButton : public CBitmapButton
{
	DECLARE_DYNAMIC(CHoverButton);

	// Construction
public:
	CHoverButton();

	// Attributes
private:
	BOOL			m_bHover;
	BOOL			m_bTracking;
	BOOL			m_bEnable;
	BOOL			m_bActiveMode;
	int				m_nActivate;
	BOOL			m_bMasking;
	CSize			m_ButtonSize;
	CBitmap			m_UpBitmap;
	CBitmap			m_DownBitmap;
	CBitmap			m_OverBitmap;
	CBitmap			m_DisableBitmap;
	CBitmap			m_MaskBitmap;
	CToolTipCtrl	m_ToolTip;
	CRgn			m_MaskRgn;

public:
	DWORD			m_dwToggleGroup;
	HBITMAP hUpBitmap;
	HBITMAP hDownBitmap;
	HBITMAP hOverBitmap;
	HBITMAP hDisableBitmap;
	HBITMAP	hMaskBitmap;

	// Operations
public:
	VOID ToggleMode(DWORD dwToggleGroup);
	HRESULT	SetToolTipText(CString strText, BOOL bActivate = TRUE);
	HRESULT	SetToolTipText(UINT nStringId, BOOL bActivate = TRUE);
	HRESULT AttachBitmap(HBITMAP hUpBitmap, HBITMAP hDownBitmap, HBITMAP hOverBitmap, HBITMAP hDisableBitmap, HBITMAP hMaskBitmap);
	HRESULT LoadBitmap(CString lpszUpBitmap, CString lpszDownBitmap, CString lpszOverBitmap, CString lpszDisableBitmap = NULL, CString lpszMaskBitmap = NULL);
	HRESULT LoadBitmap(UINT nUpBitmap, UINT nDownBitmap, UINT nOverBitmap, UINT nDisableBitmap = 0, UINT nMaskBitmap = 0);
	HRESULT DeleteBitmap();

protected:
	HRESULT	ActivateToolTip(int nActivate = 0);
	HRESULT	InitToolTip();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHoverButton)
protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

	// Implementation
public:
	void SetActiveMode(bool bActiveMode);
	//	void DrawBitmap(CDC *pDC, HBITMAP hBitmap, CRect rc);
	void DrawBitmap(CDC *pDC, CBitmap *pBitmap, CRect rc);
	HRGN CreateRgnFromMaskBitmap(HBITMAP hMaskBitmap, COLORREF crMask);
	VOID UnActivateButton(int nActivate);
	//	HRESULT UnToggleOtherButtons();
	virtual ~CHoverButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CHoverButton)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseHover(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEnable(BOOL bEnable);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HOVERBUTTON_H__04E71DF2_DB04_49F6_8F40_E5DB66772AFF__INCLUDED_)
