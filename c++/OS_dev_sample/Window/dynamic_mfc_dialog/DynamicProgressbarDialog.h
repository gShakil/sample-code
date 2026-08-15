#pragma once

#ifndef ENF_DYM_DEFINE
#ifdef DYNAMICVERSIONDIALOG_EXPORTS
#define ENF_DYM_DEFINE __declspec(dllexport)
#else
#define ENF_DYM_DEFINE __declspec(dllimport)
#endif
#endif

namespace __ENF
{
	namespace UI
	{
		class ENF_DYM_DEFINE DynamicProgressbarDialog
		{
		public:
			DynamicProgressbarDialog(CWnd* pParentWnd);
			virtual ~DynamicProgressbarDialog();

			void ShowModaless();
			void ShowModal();

			void SetTopText(const wchar_t* szText);
			void SetBottomText(const wchar_t* szText);

		protected:
			void setsize(int w, int h);

		private:
			void Createdialog(CWnd* pParentWnd);
			void Destroy();
			void* m_pDialog;
		};
	}
}