#pragma once
#include <vector>

#ifndef ENF_DYM_DEFINE
#ifdef DYNAMICVERSIONDIALOG_EXPORTS
#define ENF_DYM_DEFINE __declspec(dllexport)
#else
#define ENF_DYM_DEFINE __declspec(dllimport)
#endif
#endif

/*
1.  MFC 프로젝트에 리소스 추가 없이 동적으로 버전 정보를 출력해주는 클래스.
2.	렉스젠 흰색 로고, 렉스젠 검은색 로고, 트라웍스 검은색 로고 기본 포함 (로고 사이즈: 120 x 39)
	: bmp 파일을 별도 추가할 필요가 없음
3.	추가되는 버전 정보 갯수에 따라 다이얼로그 크기 자동 조절
4.	버전 정보의 출력 방식이 프로젝트별로 상이할 수 있으므로 관련 기능 인터페이스화

*/

namespace __ENF
{
	namespace UI
	{
		enum class VD_TEXT_TYPE
		{
			VD_APP_NAME = 0,
			VD_VERSION,
			VD_BUILD_DATE,
			VD_BUILD_TIME,
			VD_MAX
		};
		enum class VD_LOGOTYPE : int { VD_REX_BLACK_LOGO, VD_REX_WHITE_LOGO, VD_TRA_BLACK_LOGO, VD_CUSTOM_LOGO };

		class DynamicVersionDialog;
		class IVersionExport
		{
		public:
			virtual void Export(DynamicVersionDialog* pDyn) = 0;
		};

		class ENF_DYM_DEFINE DynamicVersionDialog
		{
		public:
			DynamicVersionDialog(CWnd* pParentWnd, IVersionExport* pIExport = NULL);
			virtual ~DynamicVersionDialog();

			void						DoModal();

			DynamicVersionDialog&		SetLogo(const VD_LOGOTYPE& Type, const CString& strLogoPath = _T("")); // strLogoPath: Type == VD_CUSTOM_LOGO일 경우에만 유효

			DynamicVersionDialog&		SetDialogName(const CString& strValue);
			DynamicVersionDialog&		SetAppName(const CString& strValue);
			DynamicVersionDialog&		SetVersion(const CString& strValue);
			DynamicVersionDialog&		SetBuildDate(const CString& strValue);
			DynamicVersionDialog&		SetBuildTime(const CString& strValue);
			DynamicVersionDialog&		PushModuleVersion(const CString& strValue);

			DynamicVersionDialog&		AddDialogWidth(const UINT& uValue);
			DynamicVersionDialog&		AddDialogHeight(const UINT& uValue);
			DynamicVersionDialog&		AsModaless();

			const CString&				GetText(VD_TEXT_TYPE Type);
			const std::vector<CString>& GetModuleVersionVector();
		protected:

		private:
			void						OnBnExport();
			IVersionExport*				m_pExport;
			CWnd*						m_pParentWnd;
			void*						m_pDialog;

		};
	}
}

