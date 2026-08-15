#include "pch.h"
#include "DynamicProgressbarDialog.h"
#include "DevProgressbarDialog.h"

namespace __ENF
{
	namespace UI
	{
#define DIALOG ((DevProgressbarDialog*)m_pDialog)
		DynamicProgressbarDialog::DynamicProgressbarDialog(CWnd* pParentWnd)
		{
			Createdialog(pParentWnd);
		}

		DynamicProgressbarDialog::~DynamicProgressbarDialog()
		{
			Destroy();
		}

		void DynamicProgressbarDialog::ShowModaless()
		{
			if (DIALOG)
			{
				DIALOG->SetUseModeless(true);
				DIALOG->ShowWindow();
			}
		}

		void DynamicProgressbarDialog::ShowModal()
		{
			if (DIALOG)
			{
				DIALOG->SetUseModeless(false);
				DIALOG->ShowWindow();
			}
		}

		void DynamicProgressbarDialog::Destroy()
		{
			if (DIALOG)
			{
				if (::IsWindow(DIALOG->GetSafeHwnd()))
					DIALOG->DestroyWindow();
				delete DIALOG;
				m_pDialog = NULL;
			}
		}

		void DynamicProgressbarDialog::SetTopText(const wchar_t* szText)
		{
			if (DIALOG)
				DIALOG->SetTopText(szText);
		}

		void DynamicProgressbarDialog::SetBottomText(const wchar_t* szText)
		{
			if (DIALOG)
				DIALOG->SetBottomText(szText);
		}

		void DynamicProgressbarDialog::setsize(int w, int h)
		{
			if (DIALOG)
				DIALOG->SetWindowPos(&CWnd::wndTopMost, 0, 0, w, h, SWP_NOMOVE); // 15, 38: 다이얼로그 크기 맞추기 위한 가산치
		}

		void DynamicProgressbarDialog::Createdialog(CWnd* pParentWnd)
		{
			m_pDialog = new DevProgressbarDialog(pParentWnd);
		}

	}
}