#pragma once
#include "ChildTabConfigAssist.hpp"
#include <vector>
#include <memory>
#include <map>
#include <string>
#include <afxcmn.h> // CTabCtrl

/*
ParentTabConfigAssist, ChildTabConfigBase 를 함께 사용.

사용법:

DevRenderView: 탭 컨트롤 보유 다이얼로그를 호출하는 클래스
DlgAlgorithmConfig: 탭 컨트롤 보유 다이얼로그 클래스
DlgTabAlgorithmVLoop: 탭 컨트롤에 출력될 다이얼로그 클래스



void DevRenderView::OpenAlgorithmConfig()
{
	DlgAlgorithmConfig dlg;
	DIALOG_ALGORITHM_VLOOP vLoop;

	// vLoop 초기화 및 초기 데이터 입력
	dlg.setInitialOption(vLoop);

	if (dlg.DoModal() == IDOK)
	{
		if (dlg.getOption(vLoop))
		{
			// ini 수정 과정 입력 ..
		}
	}

}



class DlgTabAlgorithmVLoop
	: public CDialogEx
	, public __ENF::TEMPLATE::VER1_ALPHA::ChildTabConfigAssist<DIALOG_ALGORITHM_VLOOP>
{
	// setToResource, getFromResource, getIDTemplate 구현 ..

	// Destroy 직전에 메모리에서 데이터 입력하도록 구현.
	void DlgTabAlgorithmVLoop::OnDestroy()
	{
		m_config = getFromResource();
		CDialogEx::OnDestroy();
	}
}



class DlgAlgorithmConfig
	 : public CDialogEx
	 , public __ENF::TEMPLATE::VER1_ALPHA::ParentTabConfigAssist<__ENF::TEMPLATE::VER1_ALPHA::ChildTabConfigBase>
{
	std::shared_ptr<DlgTabAlgorithmVLoop> m_pTabVLoop; // DIALOG_ALGORITHM_VLOOP

	// 생성자에서 탭 컨트롤 유닛을 메모리에 넣고 .. 
	DlgAlgorithmConfig()
	{
		m_pTabVLoop = std::make_shared<DlgTabAlgorithmVLoop>();
		std::vector<std::shared_ptr<ChildTabConfigBase>> v = { m_pTabVLoop };
		ParentTabConfigAssist::insertChildTabControl(std::move(v));
	}

	// 다이얼로그 생성시 탭 컨트롤에 추가되도록 한다.
	BOOL DlgAlgorithmConfig::OnInitDialog()
	{
		ParentTabConfigAssist::setTabCtrl(&m_ctlTabControl);
		const bool bAdv = ((GetKeyState(VK_CONTROL) < 0) && (GetKeyState(VK_SHIFT) < 0));
		ParentTabConfigAssist::operationChildTabControl(bAdv);
	}

	// 탭 변경시 탭 변경 이벤트 추가.
	void DlgAlgorithmConfig::OnTcnSelchangeTabAlgorithm(NMHDR* pNMHDR, LRESULT* pResult)
	{
		ParentTabConfigAssist::onTabSelChanged();
	}
}
*/

namespace __ENF
{
	namespace TEMPLATE
	{
		namespace VER1_ALPHA
		{
			template <typename T>
			class ParentTabConfigAssist
			{
			public:
				ParentTabConfigAssist()
					:m_pbase(nullptr)
					, m_pcurShowing(nullptr)
					, m_OperationOnAdv(false)
				{}

				/*
				setTabCtrl -> insertChildTabControl -> operationChildTabControl 순으로 호출
				*/

				// 필수 포인터 입력.
				void setTabCtrl(CTabCtrl* ptab);

				// 탭 컨트롤에 추가할 탭 컨트롤 입력.
				bool insertChildTabControl(std::vector<std::shared_ptr<T>> vChildTab);

				// 추가된 탭 컨트롤로 탭 구동.
				void operationChildTabControl(bool doAdvance = false);

				void onTabSelChanged();

				template <typename U>
				bool setInitialOption(const U& cfg);

				template <typename U>
				bool getOption(U& cfg);
			protected:
				template <typename U>
				ChildTabConfigAssist<U>* findChildTabControl();

				std::shared_ptr<CDialog> findChildTabControlIndex(int zerobaseIndex, bool bIncludeAdvanced = false);

			private:
				bool m_OperationOnAdv;
				std::shared_ptr<CDialog> m_pcurShowing;
				CTabCtrl* m_pbase;
				std::vector<std::shared_ptr<T>> m_vToListupTabControl;
				std::shared_ptr<CDialog> ConvertToDialog(std::shared_ptr<T>& pt);

			};

			template <typename T>
			std::shared_ptr<CDialog> __ENF::TEMPLATE::VER1_ALPHA::ParentTabConfigAssist<T>::ConvertToDialog(std::shared_ptr<T>& pt)
			{
				auto p = std::dynamic_pointer_cast<CDialog>(pt);
				ASSERT(p);
				return p;
			}

			template <typename T>
			void ParentTabConfigAssist<T>::setTabCtrl(CTabCtrl* ptab)
			{
				m_pbase = ptab;
			}

			template <typename T>
			bool ParentTabConfigAssist<T>::insertChildTabControl(std::vector<std::shared_ptr<T>> vChildTab)
			{
				// 타입체크
				std::map<std::string, int> dupMap;
				for (auto& child : vChildTab)
				{
					dupMap[child->getConfigTypeName()]++;
					auto p = std::dynamic_pointer_cast<CDialogEx>(child);
					ASSERT(p);
					if (!p)
						return false;
				}

				for (auto& dat : dupMap)
				{
					ASSERT(dat.second == 1);
					if (dat.second > 1)
						return false;
				}


				m_vToListupTabControl = std::move(vChildTab);
				return true;
			}

			template <typename T>
			void ParentTabConfigAssist<T>::operationChildTabControl(bool doAdvance /* = false */)
			{
				ASSERT(m_pbase);

				int nSeq = 0;
				for (auto& child : m_vToListupTabControl)
				{
					if (doAdvance)
						m_pbase->InsertItem(nSeq++, CString(child->getTabName().c_str()));
					else
					{
						if (!child->isAdvance())
							m_pbase->InsertItem(nSeq++, CString(child->getTabName().c_str()));
					}

					auto pt = ConvertToDialog(child);
					if (pt)
					{
						pt->Create(child->getIDTemplate(), m_pbase);
					}
				}

				CRect rcClient;
				m_pbase->GetClientRect(&rcClient);
				m_pbase->AdjustRect(FALSE, &rcClient);
				for (auto& child : m_vToListupTabControl)
				{
					auto pt = ConvertToDialog(child);
					if (pt)
					{
						pt->MoveWindow(&rcClient);
					}
				}


				auto pT = m_vToListupTabControl.front();
				auto pD = ConvertToDialog(pT);
				m_pcurShowing = pD;
				m_pcurShowing->ShowWindow(SW_SHOW);
			}

			template <typename T>
			void ParentTabConfigAssist<T>::onTabSelChanged()
			{
				if (m_pbase)
				{
					const int nsel = m_pbase->GetCurSel();
					if (m_pcurShowing)
					{
						m_pcurShowing->ShowWindow(SW_HIDE);
					}

					auto child = findChildTabControlIndex(nsel, m_OperationOnAdv);
					child->ShowWindow(SW_SHOW);
					m_pcurShowing = child;
				}
			}

			template <typename T>
			template <typename U>
			bool ParentTabConfigAssist<T>::setInitialOption(const U& cfg)
			{
				auto* child = findChildTabControl<U>();
				if (child)
				{
					child->setToMemory(cfg);
					return true;
				}

				return false;
			}

			template <typename T>
			template <typename U>
			bool ParentTabConfigAssist<T>::getOption(U& cfg)
			{
				auto* child = findChildTabControl<U>();
				if (child)
				{
					cfg = child->getFromMemory();
					return true;
				}
				return false;
			}

			template <typename T>
			template <typename U>
			ChildTabConfigAssist<U>* ParentTabConfigAssist<T>::findChildTabControl()
			{
				for (auto& child : m_vToListupTabControl)
				{
					if (child->IsConfigType<U>())
						return dynamic_cast<ChildTabConfigAssist<U>*>(child.get());
				}

				return nullptr;
			}


			template <typename T>
			std::shared_ptr<CDialog> ParentTabConfigAssist<T>::findChildTabControlIndex(int zerobaseIndex, bool bIncludeAdvanced /*= false*/)
			{
				int nforIndex = 0;
				for (auto& child : m_vToListupTabControl)
				{
					if (bIncludeAdvanced)
					{
						if (nforIndex++ == zerobaseIndex)
							return std::dynamic_pointer_cast<CDialog>(child);
					}
					else
					{
						if (child->isAdvance())
							continue;

						if (nforIndex++ == zerobaseIndex)
							return std::dynamic_pointer_cast<CDialog>(child);
					}

				}
				return nullptr;
			}
		}
	}
}