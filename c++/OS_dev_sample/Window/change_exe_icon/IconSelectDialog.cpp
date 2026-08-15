#include "pch.h"
#include "IconSelectDialog.h"
#include "DevIconDialog.h"


namespace __ENF
{
	namespace UI
	{
		#define DIALOG ((DevIconDialog*)m_pDialog)

		IconSelectDialog::IconSelectDialog(CWnd* pParentWnd)
		{
			m_pDialog = new DevIconDialog(pParentWnd);
		}
		IconSelectDialog::~IconSelectDialog()
		{
			if (m_pDialog)
				delete DIALOG;
		}

		IconSelectDialog& IconSelectDialog::InsertAvailableItem(const ENF_ICON& Icon)
		{
			DID_ICON_LIST conv;

			switch (Icon)
			{
			case ENF_ICON::ENF_ICON_REXGEN:
				conv = DID_ICON_LIST::DID_REXGEN;
				break;
			case ENF_ICON::ENF_ICON_TRAWORKS:
				conv = DID_ICON_LIST::DID_TRAWORKS;
				break;
			case ENF_ICON::ENF_ICON_KOREANET:
				conv = DID_ICON_LIST::DID_KOREANET;
				break;
			case ENF_ICON::ENF_ICON_SONET:
				conv = DID_ICON_LIST::DID_SONET;
				break;
			default:
				return *this;
				break;
			}

			DIALOG->PushComboboxItem(conv);

			return *this;
		}


		IconSelectDialog& IconSelectDialog::SetDefaultIcon(const ENF_ICON& Icon)
		{
			DID_ICON_LIST conv;

			switch (Icon)
			{
			case ENF_ICON::ENF_ICON_REXGEN:
				conv = DID_ICON_LIST::DID_REXGEN;
				break;
			case ENF_ICON::ENF_ICON_TRAWORKS:
				conv = DID_ICON_LIST::DID_TRAWORKS;
				break;
			case ENF_ICON::ENF_ICON_KOREANET:
				conv = DID_ICON_LIST::DID_KOREANET;
				break;
			case ENF_ICON::ENF_ICON_SONET:
				conv = DID_ICON_LIST::DID_SONET;
				break;
			default:
				return *this;
				break;
			}

			DIALOG->SetDefaultIcon(conv);
			return *this;
		}

		ENF_ICON IconSelectDialog::DoModal()
		{
			DID_ICON_LIST icon = DIALOG->ShowWindow();

			switch (icon)
			{
			case DID_ICON_LIST::DID_REXGEN:
				return ENF_ICON::ENF_ICON_REXGEN;
			case DID_ICON_LIST::DID_TRAWORKS:
				return ENF_ICON::ENF_ICON_TRAWORKS;
			case DID_ICON_LIST::DID_KOREANET:
				return ENF_ICON::ENF_ICON_KOREANET;
			case DID_ICON_LIST::DID_SONET:
				return ENF_ICON::ENF_ICON_SONET;
			default:
				return ENF_ICON::ENF_NONE;
			}

		}
	}
}