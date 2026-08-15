#include "pch.h"
#include "DevIconDialog.h"
#include <algorithm>

#define RESOURCE_NUM_START		 1000
#define TEXT_RESOURCENUM		 (RESOURCE_NUM_START + 1)
#define COMBOBOX_RESOURCENUM	 (RESOURCE_NUM_START + 2)
#define OKBTN_RESOURCENUM		 (RESOURCE_NUM_START + 3)

BEGIN_MESSAGE_MAP(DevIconDialog, CDynDialogEx)
	ON_BN_CLICKED(OKBTN_RESOURCENUM, &DevIconDialog::OnBnClickedOk)
END_MESSAGE_MAP()

void DevIconDialog::OnBnClickedOk()
{
	CComboBox* pBox = ((CComboBox*)GetDlgItem(COMBOBOX_RESOURCENUM));
	if(pBox)
		m_SelectedIcon = GetComboboxItemFromResource();

	CDynDialogEx::OnOK();
}

DevIconDialog::DevIconDialog(CWnd* pParentWnd)
	: CDynDialogEx(pParentWnd)
{

}

DevIconDialog::~DevIconDialog()
{

}

DID_ICON_LIST DevIconDialog::ShowWindow()
{
	CFont* pft = AfxGetMainWnd()->GetFont();
	SetFont(pft);
	SetUseSystemButtons(FALSE);

	SetWindowTitle("프로그램 아이콘 변경");

	AddDlgControl(
		STATICTEXT,
		_T("아이콘 선택"),
		STYLE_STATIC,
		EXSTYLE_STATIC,
		CRect(15, 12, 60, 35),
		NULL,
		TEXT_RESOURCENUM
	);

	AddDlgControl(
		COMBOBOX,
		_T(""),
		STYLE_COMBOBOX_DROPDOWNLIST_NOSORT,
		EXSTYLE_COMBOBOX,
		CRect(80, 10, 150, 33),
		NULL,
		COMBOBOX_RESOURCENUM
	);
 
  	AddDlgControl(
  		BUTTON,
  		_T("확인"),
  		STYLE_BUTTON,
  		EXSTYLE_BUTTON,
  		CRect(80, 30, 150, 45),
  		NULL,
  		OKBTN_RESOURCENUM
  	);

	if (DoModal() == IDOK)
	{
		return GetComboboxItem();
	}

	return DID_ICON_LIST::DID_NONE;
}

void DevIconDialog::SetDefaultIcon(DID_ICON_LIST Item)
{
	m_DefaultIcon = Item;
}

BOOL DevIconDialog::OnInitDialog()
{
	CDynDialogEx::OnInitDialog();

	SetWindowPos(NULL, 0, 0, 315 + 15, 100 + 38, SWP_NOZORDER | SWP_NOMOVE); // 15, 38: 다이얼로그 크기 맞추기 위한 가산치

	CDynDialogItemEx* pDynDialogItemEx = NULL;
	int nsize = (int)m_arrDlgItemPtr.GetSize();
	long l = GetNumberOfConrols();
	for (int i = 0; i < m_arrDlgItemPtr.GetSize(); i++)
	{
		pDynDialogItemEx = m_arrDlgItemPtr[i];
		if (pDynDialogItemEx != NULL)
		{
			switch (pDynDialogItemEx->GetControlID())
			{
			case COMBOBOX_RESOURCENUM:
				InsertAvailableItem(((CComboBox*)pDynDialogItemEx));
				SelectIcon((CComboBox*)pDynDialogItemEx, m_DefaultIcon);
				break;
			}
		}
	}
	return TRUE;
}

void DevIconDialog::PushComboboxItem(DID_ICON_LIST Item)
{
	m_vIconList.push_back(Item);
}

DID_ICON_LIST DevIconDialog::GetComboboxItem()
{
	return m_SelectedIcon;
}

DID_ICON_LIST DevIconDialog::GetComboboxItemFromResource()
{
	CComboBox* pBox = ((CComboBox*)GetDlgItem(COMBOBOX_RESOURCENUM));
	if (pBox)
	{
		const int nCur = pBox->GetCurSel();
		if (0 <= nCur && nCur < pBox->GetCount())
		{
			CString str;
			pBox->GetLBText(nCur, str);
			return StringToIcon(str);
		}
	}
	return DID_ICON_LIST::DID_NONE;
}

void DevIconDialog::InsertAvailableItem(CComboBox* pBox)
{
	if (pBox)
	{
		if (m_vIconList.size() <= 0)
			pBox->InsertString(0, IconToString(DID_ICON_LIST::DID_NONE));
		else
		{
			std::sort(m_vIconList.begin(), m_vIconList.end()); // 렉스젠 텍스트가 가장 위로 오도록 하고

			int nSel = 0;
			for (const DID_ICON_LIST& icon : m_vIconList)
			{
				pBox->InsertString(nSel++, IconToString(icon));
			}
		}
	}
}

void DevIconDialog::SelectIcon(CComboBox* pBox, const DID_ICON_LIST& Icon)
{
	if (pBox)
	{
		for (int i = 0; i < pBox->GetCount(); i++)
		{
			CString str;
			pBox->GetLBText(i, str);

			if (StringToIcon(str) == Icon)
			{
				pBox->SetCurSel(i);
				return;
			}
		}
		pBox->SetCurSel(0);
	}
	return;
}

DID_ICON_LIST DevIconDialog::StringToIcon(const CString& str)
{
	if (str == _T("Rexgen"))
		return DID_ICON_LIST::DID_REXGEN;
	else if (str == _T("Traworks"))
		return DID_ICON_LIST::DID_TRAWORKS;
	else if (str == _T("Koreanet"))
		return DID_ICON_LIST::DID_KOREANET;
		
	return DID_ICON_LIST::DID_NONE;
}

CString DevIconDialog::IconToString(const DID_ICON_LIST& icon)
{
	switch (icon)
	{
	case DID_ICON_LIST::DID_REXGEN:
		return _T("Rexgen");
	case DID_ICON_LIST::DID_TRAWORKS:
		return _T("Traworks");
	case DID_ICON_LIST::DID_KOREANET:
		return _T("Koreanet");
	default:
		return _T("항목 없음");
	}
}

