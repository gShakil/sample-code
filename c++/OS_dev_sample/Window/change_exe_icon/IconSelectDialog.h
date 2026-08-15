#pragma once
#include "EnfDllAPI.h"
#include "EnfIcon.h"

/*
* Icon 선택 다이얼로그 출력시 사용.
*
*/
namespace __ENF
{
	namespace UI
	{
		class ENF_DLL_DEFINE IconSelectDialog
		{
		public:
			IconSelectDialog(CWnd* pParentWnd);
			virtual ~IconSelectDialog();

			/*
			InsertComboboxItem: 다이얼로그에 출력되는 콤보 박스에 입력되는 아이템 입력.
			*/
			IconSelectDialog& InsertAvailableItem(const ENF_ICON& Icon);

			// DefaultIcon: 다이얼로그 생성시 콤보박스 초기 선택지 입력
			IconSelectDialog& SetDefaultIcon(const ENF_ICON& Icon);
			ENF_ICON		  DoModal();

		private:
			void* m_pDialog;
		};
	}
}