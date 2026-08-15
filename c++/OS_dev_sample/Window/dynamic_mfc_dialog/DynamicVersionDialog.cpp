#include "pch.h"
#include "DynamicVersionDialog.h"
#include "DevVersionDialog.h"

namespace __ENF
{
	namespace UI
	{
#define DIALOG ((DevVersionDialog*)m_pDialog)

		DynamicVersionDialog::DynamicVersionDialog(CWnd* pParentWnd, IVersionExport* pIExport /* = NULL */)
			:m_pParentWnd(pParentWnd)
		{
			m_pDialog = new DevVersionDialog(pParentWnd);
			m_pExport = pIExport;
		}

		DynamicVersionDialog::~DynamicVersionDialog()
		{
			if (m_pDialog)
				delete DIALOG;
		}

		void DynamicVersionDialog::DoModal()
		{
			DIALOG->EnableExportButton(m_pExport != NULL);
			DIALOG->SetOnBnClickExport(std::bind(&DynamicVersionDialog::OnBnExport, this));
			DIALOG->ShowWindow();
		}

		DynamicVersionDialog& DynamicVersionDialog::SetDialogName(const CString& strValue)
		{
			DIALOG->SetDialogName(strValue);
			return *this;
		}

		DynamicVersionDialog& DynamicVersionDialog::SetLogo(const VD_LOGOTYPE& Type, const CString& strLogoPath /*= _T("")*/)
		{
			DVD_LOGO_TYPE conv;
			switch (Type)
			{
			case VD_LOGOTYPE::VD_REX_BLACK_LOGO:
				conv = DVD_LOGO_TYPE::DVD_REX_BLACK_LOGO;
				break;
			case VD_LOGOTYPE::VD_CUSTOM_LOGO:
				conv = DVD_LOGO_TYPE::DVD_CUSTOM_LOGO;
				break;
			case VD_LOGOTYPE::VD_TRA_BLACK_LOGO:
				conv = DVD_LOGO_TYPE::DVD_TRA_BLACK_LOGO;
				break;
			case VD_LOGOTYPE::VD_REX_WHITE_LOGO:
			default:
				conv = DVD_LOGO_TYPE::DVD_REX_WHITE_LOGO;
				break;
			}

			DIALOG->SetLogo(conv, strLogoPath);
			return *this;
		}

		DynamicVersionDialog& DynamicVersionDialog::SetAppName(const CString& strValue)
		{
			DIALOG->SetAppName(strValue);
			return *this;
		}

		DynamicVersionDialog& DynamicVersionDialog::SetVersion(const CString& strValue)
		{
			DIALOG->SetVersion(strValue);
			return *this;
		}

		DynamicVersionDialog& DynamicVersionDialog::SetBuildDate(const CString& strValue)
		{
			DIALOG->SetBuildDate(strValue);
			return *this;
		}

		DynamicVersionDialog& DynamicVersionDialog::SetBuildTime(const CString& strValue)
		{
			DIALOG->SetBuildTime(strValue);
			return *this;
		}

		DynamicVersionDialog& DynamicVersionDialog::AddDialogWidth(const UINT& uValue)
		{
			DIALOG->AddWidth(uValue);
			return *this;
		}

		DynamicVersionDialog& DynamicVersionDialog::AddDialogHeight(const UINT& uValue)
		{
			DIALOG->AddHeight(uValue);
			return *this;
		}

		DynamicVersionDialog& DynamicVersionDialog::AsModaless()
		{
			DIALOG->SetUseModeless(true);
			return *this;
		}

		DynamicVersionDialog& DynamicVersionDialog::PushModuleVersion(const CString& strValue)
		{
			DIALOG->PushModuleVersion(strValue);
			return *this;
		}

		const CString& DynamicVersionDialog::GetText(VD_TEXT_TYPE Type)
		{
			DVD_TEXT_TYPE conv;
			switch (Type)
			{
			default:
			case VD_TEXT_TYPE::VD_APP_NAME:
				conv = DVD_TEXT_TYPE::DVD_APP_NAME;
				break;
			case VD_TEXT_TYPE::VD_VERSION:
				conv = DVD_TEXT_TYPE::DVD_VERSION;
				break;
			case VD_TEXT_TYPE::VD_BUILD_DATE:
				conv = DVD_TEXT_TYPE::DVD_BUILD_DATE;
				break;
			case VD_TEXT_TYPE::VD_BUILD_TIME:
				conv = DVD_TEXT_TYPE::DVD_BUILD_TIME;
				break;
			}

			return DIALOG->GetText(conv);
		}

		const std::vector<CString>& DynamicVersionDialog::GetModuleVersionVector()
		{
			return DIALOG->GetModuleVersionVector();
		}

		void DynamicVersionDialog::OnBnExport()
		{
			ASSERT(m_pExport);
			if (m_pExport)
				m_pExport->Export(this);
		}
	}
}