#pragma once
#include "DynDialogEx.h"
#include <vector>

enum class DID_ICON_LIST : int { DID_NONE, DID_REXGEN, DID_TRAWORKS, DID_KOREANET, DID_SONET };

class DevIconDialog :
    public CDynDialogEx
{

	DECLARE_MESSAGE_MAP()
	afx_msg void			OnBnClickedOk();
	afx_msg BOOL			OnInitDialog() override;

public:
    DevIconDialog(CWnd* pParentWnd);
    virtual ~DevIconDialog();

    DID_ICON_LIST           ShowWindow();

    void                    SetDefaultIcon(DID_ICON_LIST Item);
    void                    PushComboboxItem(DID_ICON_LIST Item);

protected:
    DID_ICON_LIST           GetComboboxItem();
	DID_ICON_LIST           GetComboboxItemFromResource();

    void                    InsertAvailableItem(CComboBox* pBox);
    void                    SelectIcon(CComboBox* pBox, const DID_ICON_LIST& Icon);
private:
    DID_ICON_LIST           StringToIcon(const CString& str);
    CString                 IconToString(const DID_ICON_LIST& icon);

    std::vector<DID_ICON_LIST> m_vIconList;
	DID_ICON_LIST           m_DefaultIcon;
	DID_ICON_LIST           m_SelectedIcon;
};

