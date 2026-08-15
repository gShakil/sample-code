#pragma once
#include "DynDialogEx.h"
#include "HoverButton.h"
#include <vector>
#include <functional>

enum class DVD_LOGO_TYPE : int {DVD_REX_BLACK_LOGO, DVD_REX_WHITE_LOGO, DVD_TRA_BLACK_LOGO, DVD_CUSTOM_LOGO};
enum class DVD_TEXT_TYPE
{
	DVD_APP_NAME = 0,
	DVD_VERSION,
	DVD_BUILD_DATE,
	DVD_BUILD_TIME,
	DVD_MAX
};

class  DevVersionDialog
	: public CDynDialogEx
{
	DECLARE_MESSAGE_MAP()
	afx_msg void			OnBnClickedExport();
	afx_msg BOOL			OnInitDialog() override;

public:
	DevVersionDialog(CWnd* pParentWnd);
	virtual ~DevVersionDialog();

	void						ShowWindow();

	void						SetOnBnClickExport		(std::function<void()> pFunc);
	void						SetDialogName			(const CString& strValue);
	void						SetLogo					(const DVD_LOGO_TYPE& Type, const CString& strLogoPath = _T("")); // strLogoPath: Type == VD_CUSTOM_LOGO일 경우에만 유효
	void						SetAppName				(const CString& strValue);
	void						SetVersion				(const CString& strValue);
	void						SetBuildDate			(const CString& strValue);
	void						SetBuildTime			(const CString& strValue);
	void						AddWidth				(const UINT& uValue);
	void						AddHeight				(const UINT& uValue);
	void						EnableExportButton		(const bool& bValue);
	void						PushModuleVersion		(const CString& strValue);

	const CString&				GetText					(DVD_TEXT_TYPE Type);
	const std::vector<CString>& GetModuleVersionVector	();

protected:
	void						ArrangeResourcePosition(const CSize& rcDlg);
	CSize						GetDialogSize			();
	CString						MakeVersionString		(const CString& strVersion, const CString& strDate, const CString& strTime);
	void						ShowLogo				();

private:
	CString&					GetTextRef				(DVD_TEXT_TYPE Type);
	HMODULE						GetThisDllModule		();


	CHoverButton				m_Logo;
	DVD_LOGO_TYPE				m_LogoType;
	bool						m_bEnableExportBtn;
	CSize						m_AddSize;
	CString						m_strCustomLogoPath;
	CString						m_strTextArray[(unsigned int)DVD_TEXT_TYPE::DVD_MAX];
	std::vector<CString>		m_vModuleVersion;
	std::function<void()>		m_pOnBnExport;
};

