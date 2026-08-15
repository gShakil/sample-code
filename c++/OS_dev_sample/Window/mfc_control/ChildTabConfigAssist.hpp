#pragma once
#include <string>
#include <afxdialogex.h> // CDialogEx

namespace __ENF
{
	namespace TEMPLATE
	{
		namespace VER1_ALPHA
		{
			class ChildTabConfigBase
			{
			public:
				virtual std::string getConfigTypeName() = 0;
				virtual std::string getTabName() = 0;
				virtual unsigned int getIDTemplate() = 0; // 리소스 ID 리턴.
				virtual bool isAdvance() { return false; }

				template <typename U>
				bool IsConfigType()
				{
					return getConfigTypeName() == typeid(U).name();
				}
			};

			template<typename T>
			class ChildTabConfigAssist
				: public ChildTabConfigBase
			{
			public:
				std::string getConfigTypeName() override final;

				virtual bool setToResource(const T& cfg) { return false; };
				virtual T	 getFromResource() { return T(); };

				bool		 setToMemory(const T& cfg) { memcpy(&m_config, &cfg, sizeof(T)); return true; };
				T			 getFromMemory() { return m_config; };

			protected:
				// 설정의 유효성 판단.
				virtual bool IsValidOption(const T& cfg) { return false; };
				T m_config;

			private:
			};

			template<typename T>
			std::string ChildTabConfigAssist<T>::getConfigTypeName()
			{
				return typeid(T).name();
			}
		}
	}
}