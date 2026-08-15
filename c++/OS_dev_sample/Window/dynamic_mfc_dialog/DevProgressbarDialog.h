#pragma once
#include "DynDialogEx.h"

class DevProgressbarDialog :
    public CDynDialogEx
{
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL			OnInitDialog() override;
    afx_msg LRESULT         OnReceiveSetText(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT         OnNcHitTest(CPoint point);

public:
    DevProgressbarDialog(CWnd* pParentWnd);
    virtual ~DevProgressbarDialog();

    void ShowWindow();

	void SetTopText     (const wchar_t* text);
	void SetBottomText  (const wchar_t* text);
protected:

private:
    CString m_strTopText;
    CString m_strBottomText;
};

