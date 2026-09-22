/*****************************************************************************
 *
 *  (C) Copyright 2003-2025 Cougar Mountain Software
 *  All Rights reserved.
 *
 *  This program is an unpublished copyrighted work which is proprietary to
 *  Cougar Mountain Software and contains confidential information that is not
 *  to be reproduced or disclosed to any other person or entity without prior
 *  written consent from Cougar Mountain Software in each and every instance.
 *
 *  WARNING:  Unauthorized reproduction of this program as well as
 *  unauthorized preparation of derivative works based upon the program or
 *  distribution of copies by sale, rental, lease or lending are violations
 *  of federal copyright laws and state trade secret laws, punishable by
 *  civil and criminal penalties.
 *
 ******************************************************************************/
#pragma once

 // Version 3.0
 // #include "..\GBLForms\CMSSDIMainFrame.h"
#include "..\GBLForms\CMSMDIMainFrame.h"
#include "..\GBLForms\TipOfTheDay.h"
#include "..\CTForms\DLG_CT16to32.h"
#include "..\CTForms\DLG_CTOrgCompany.h"
#include "..\CTForms\CFV_CTBackupCompanyInfo.h"
#include "..\CTForms\CreateDBDlg.h"
// InTimeTec: 11.20.2012.HS - Custom Menus
#include "..\GBLForms\CMSCustomOptions.h"
// InTimeTec: End

class __declspec(dllexport) CMainFrame
	: public CCMSMDIMainFrame		// Version 3.0 CCMSMDIMainFrame = CCMSSDIMainFrame
{
	DECLARE_CLASS_NOCOPY(CMainFrame)
	DECLARE_DYNAMIC(CMainFrame)

	enum enmTBButtonIndexes		//this enum purposely picks up where ICMSMainFrameWnd::enmTBButtonIndexes left off
	{
		enuTBStartIdx = ((int)ICMSMainFrameWnd::enuTBEndIdx) - 1,
		enuTBSelectCompanyIdx,
		enuTBAddChangeUsersIdx,
		enuTBEndIdx								//add new items before enuTBEndIdx
	};

	enum enmCTGUIItem
	{
		SetupSelectCompanyButton,
		SetupWizardButton,
		ChangeUsersButton
	};

	friend class CCMSCTReportsMenu;
	friend class CCMSCT32App;

	typedef std::map<enmCTGUIItem, bool>	CTGUIAccessMap;
	mutable CTGUIAccessMap	m_CTGUIAccess;

	UINT m_nRawNeonCRMMenuPos;
	UINT m_nDenaliBIMenuPos;
	UINT m_nRawDonatelyMenuPos;

	__declspec(property(get = get_m_nNeonCRMMenuPos))		UINT m_nNeonCRMMenuPos;

	UINT get_m_nNeonCRMMenuPos() const { return ShouldDenaliBISetupMenuShow() ? m_nRawNeonCRMMenuPos : m_nRawNeonCRMMenuPos - 1; }

	// begbert 11/19/2019 PBI 52317 : Added the Donately import
	__declspec(property(get = get_m_nDonatelyMenuPos))		UINT m_nDonatelyMenuPos;
		UINT get_m_nDonatelyMenuPos() const
	{
		int pos = m_nRawDonatelyMenuPos;
		if (!ShouldDenaliBISetupMenuShow()) pos--;
		if (!ShouldNeonCRMSetupMenuShow()) pos--;
		return pos;
	}


public:
	CMainFrame(void);
	virtual ~CMainFrame(void);

	//RCG - 09/12/2014 - Bug 8130/18883 - Routine called to check if no companies are installed
	virtual bool  GetNoCompaniesInstalled() { return m_bNoInstalledCompanies; }

protected:
	virtual bool	CreateMenu(void);				// contains all of the commands needed to create the menu
	virtual void	MenuSecurityDisableItems(void);			// called to disable menu items based on security settings
	virtual void	CreateModuleToolBar(void);				// Routine to override in each module
	virtual void	StartProcess(int) { };

	bool get_CanCTAccess(enmCTGUIItem eItem) const { return m_CTGUIAccess[eItem]; }
	void put_CanCTAccess(enmCTGUIItem eItem, bool bolAccess) { m_CTGUIAccess[eItem] = bolAccess; }

	__declspec(property(get = get_CanCTAccess, put = put_CanCTAccess))		bool CanCTAccess[];

	void	OnViewModifyCompany(const CString& szCompanyCode, bool bIsFund);
	void	OnAddNewCompany(void);
	void	OnCompanyDetails(void);
	void	OnCustomDetails(void);
	void	OnOrganizationDetails(bool bIsFund);
	void	ShowDialogSelectFundAct(void);
	void	MenuUpdateElectronicConstants(void);
	void	MenuUpdateInternalControlAlerts(void);
	void	MenuUpdateReports(void);
	void	ShowLicenseManagerMenuItem();
	void	ShowConvertPayrollFromCMSProMenuItem();	// begbert 02-10-2014 PBI 138 : Added this

	// InTimeTec: 11.20.2012.HS - Custom Menus
	virtual void	CreateCustomMenu(void);
	virtual void	CustomMenuSecurityDisableItem(void);
	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void * pExtra, AFX_CMDHANDLERINFO * pHandlerInfo);
	CMSCustomOptions m_clsCustomOptions;
	// InTimeTec: 04.17.2013.HS - Custom Menus
	void UpdateCustomMenu();
	// InTimeTec: End
	bool			DoPayrollTablesExist(void);	//RCG - 03/30/2017 - Bug 32830 - Check if Payroll tables exist
	//RCG - 02/28/2018 - PBI 44247 - Added user rights setting for NeonCRM Setup
	bool ShouldNeonCRMSetupMenuShow() const
	{
		return si->CompanySettings.IsFund && (si->ModulesInstalled.GeneralLedger.IsUsable || si->ModulesInstalled.AccountsReceivable.IsUsable) && si->UserRight.m_stuControllerNode.Company.bNeonCRM ? TRUE : FALSE;
	}
	// begbert 11/19/2019 PBI 52317 : Added the Donately import
	bool ShouldDonatelySetupMenuShow() const
	{
		return si->ModulesInstalled.GeneralLedger.IsUsable && si->UserRight.m_stuControllerNode.Company.bDonatelyIntegrationSetup ? TRUE : FALSE;
	}

	bool ShouldDenaliBISetupMenuShow() const
	{
		return si->ModulesInstalled.DenaliBI.Licensed && si->UserRight.m_stuControllerNode.Company.bDenaliBISetup ? TRUE : FALSE;
	}

	DECLARE_MESSAGE_MAP()

	virtual int		OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL	OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL	OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual void	OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg BOOL	BackupcompanyBackup(void);		// DWP - 4866 - 7/29/13 - If the backup does not start then we want to return FALSE here
	afx_msg void	OnBackupcompanyBackup(void);
	afx_msg void	OnCompanyChangedate(void);
	afx_msg void	OnCompanyFiscalcalendar(void);
	afx_msg void	OnCompanyMerchantWareSetup(void);	// begbert 04-09-2014 PBI 17178 : added this
	afx_msg void	OnCompanyCardPointeSetup(void);		//RS - 04/26/2024 - PBI 63797 - Added user security and menu item to Organization menu
	afx_msg void	OnCompanyDenaliBISetup(void);
	afx_msg void	OnCompanyNeonCRMSetup(void);
	afx_msg void	OnCompanyDonatelySetup(void);
	afx_msg void	OnCompanySetUpYodleeIntegration(void);
	afx_msg void	OnCompanySetUpPayPalIntegration(void);
	afx_msg void	OnCompanyInstallmodules(void);
	afx_msg void	OnCompanyConvertPayrollFromCMSPro(void);	// begbert 02-10-2014 PBI 138 : Added this
	afx_msg void	OnCompanyMagneticmediaconstraints(void);
	afx_msg void	OnCompanyInternalControlAlerts(void);
	afx_msg void	OnCompanySetupSelectCompany(void);
	afx_msg void	OnCompanyCloseLogoff(void);
	afx_msg void	OnSecurityAdd(void);
	afx_msg void	OnSecurityChangepassword(void);
	afx_msg void	OnSecurityResetPassword(void);
	afx_msg void	OnSecurityActivityLog(void);
	afx_msg void	OnOptionsModulePrefs(void);
	afx_msg void	OnOptionsCurrencies(void);
	afx_msg void	OnOptionsCurrencies(CXMLParams* pXML, UINT uiMode = 0);	// BK 4/28/03 Overloaded for Lookup View/Modify
	afx_msg void	OnOptionsDisplayfileinformaion(void);
	afx_msg void	OnOptionsProcessPayroll(void);
	afx_msg void	OnOptionsExchangerates(void);
	afx_msg void    OnOptionsChangeURLtoArcharinaApps(void);
	afx_msg void	OnBackupcompanyRestore(void);
	afx_msg void	OnSecurityGroup(void);
	afx_msg void	OnSecurityCopyUsers(void);
	afx_msg void	OnSecurityCopyGroups(void);
	afx_msg void	OnOpenLicenseManager();

	//afx_msg void	OnEnableReportsMenu(CCmdUI *pCmdUI);
	afx_msg void	OnEnableRemoveIntegrationReports(CCmdUI *pCmdUI);
	afx_msg void	OnEnableRemoveConversionReports(CCmdUI *pCmdUI);
	afx_msg void	OnReportRemoveIntegrationChecklist(void);
	afx_msg void	OnReportIntegrationChecklistDate1(void);
	afx_msg void	OnReportIntegrationChecklistDate2(void);
	afx_msg void	OnReportIntegrationChecklistDate3(void);
	afx_msg void	OnReportIntegrationChecklistDate4(void);
	afx_msg void	OnReportIntegrationChecklistDate5(void);

	// 1-33328 BK 9/23/09 new reports added with CR 1-33386
	afx_msg void	OnReportPreConversionChecklist();
	afx_msg void	OnReportCodesValuesAdded();
	afx_msg void	OnReportIntegrationStatus();

	// PBI 22186 BK 8/19/14 Added red flag reports
	afx_msg void	OnReportAPControlReport();
	afx_msg void	OnReportAPGLExpenseDistributionReport();
	afx_msg void	OnReportARControlReport();
	afx_msg void	OnReportBRMissingCheckReport();
	afx_msg void	OnReportGLReprintPostingReport();
	afx_msg void	OnReportINControlReport();
	afx_msg void	OnReportINMovementReport();
	afx_msg void	OnReportPOOpenPurchaseOrderReport();
	afx_msg void	OnReportSAPriceVarianceReport();
	afx_msg void	OnReportSAInvoiceTrackingReport();

	// 1-33328 BK 9/23/09 enable/disable conversion menu items depending on whether report exists
	afx_msg void	OnEnablePreConversionChecklist(CCmdUI *pCmdUI);
	afx_msg void	OnEnableCodesValuesAddedReport(CCmdUI *pCmdUI);
	afx_msg void	OnEnableIntegrationStatusReport(CCmdUI *pCmdUI);
	afx_msg void	OnRemoveConversionReports(void);

	afx_msg void	OnEnableChangeUsersButton(CCmdUI *pCmdUI);
	afx_msg void	OnEnableSetupSelectCompanyButton(CCmdUI *pCmdUI);
	afx_msg void	OnEnableSetupWizardButton(CCmdUI *pCmdUI);
	afx_msg void	OnEnableCopyUsers(CCmdUI *pCmdUI);
	afx_msg void	OnEnableCopyGroups(CCmdUI *pCmdUI);
	afx_msg void	OnEnableCompanyInstallmodules(CCmdUI *pCmdUI);
	afx_msg void	OnEnableCompanyConvertPayrollFromCMSPro(CCmdUI *pCmdUI);	// begbert 02-10-2014 PBI 138 : Added this
	afx_msg void	OnEnableCompanyChangedate(CCmdUI *pCmdUI);
	afx_msg void	OnEnableCompanyFiscalcalendar(CCmdUI *pCmdUI);
	afx_msg void	OnEnableCompanDenaliBISetup(CCmdUI *pCmdUI);
	afx_msg void	OnEnableCompanyMerchantWareSetup(CCmdUI *pCmdUI);	// begbert 04-09-2014 PBI 17178 : added this
	afx_msg void	OnEnableCompanyCardPointeSetup(CCmdUI* pCmdUI);		//RS - 04/26/2024 - PBI 63797 - Added user security and menu item to Organization menu
	afx_msg void	OnEnableCompanyNeonCRMSetup(CCmdUI *pCmdUI);
	afx_msg void	OnEnableCompanyDonatelySetup(CCmdUI *pCmdUI);
	//	PBI 45155	06/06/2018	mvt - Added security for Denali BI Setup
	afx_msg void	OnEnableCompanyDenaliBISetup(CCmdUI *pCmdUI);
	//	PBI 47520	12/04/2018	mvt - Added security for Yodlee integration setup
	afx_msg void	OnEnableCompanySetUpYodleeIntegration(CCmdUI *pCmdUI);
	afx_msg void	OnEnableCompanySetUpPayPalIntegration(CCmdUI *pCmdUI);
	// PBI 46406	09/26/2018	mvt - Added security for Internal Control Alerts in Controller
	afx_msg void	OnEnableCompanyInternalControlAlertsSetup(CCmdUI *pCmdUI);
	afx_msg void	OnEnableCompanyMagneticmediaconstraints(CCmdUI *pCmdUI);
	afx_msg void	OnEnableSecurityGroup(CCmdUI *pCmdUI);
	afx_msg void	OnEnableSecurityChangePassword(CCmdUI *pCmdUI);
	afx_msg void	OnEnableSecurityResetPassword(CCmdUI *pCmdUI);
	afx_msg void	OnEnableSecurityCopyUsers(CCmdUI *pCmdUI);
	afx_msg void	OnEnableSecurityCopyGroups(CCmdUI *pCmdUI);
	afx_msg void	OnEnableModulePrefs(CCmdUI *pCmdUI);
	afx_msg void	OnEnableOptionsCurrencies(CCmdUI *pCmdUI);
	afx_msg void	OnEnableOptionsExchangerates(CCmdUI *pCmdUI);
	afx_msg void	OnEnableOptionsDisplayfileinformaion(CCmdUI *pCmdUI);
	afx_msg void	OnEnableOptionsProcessPayroll(CCmdUI *pCmdUI);
	afx_msg void	OnEnableOptionsChangeURLtoArcharinaApps(CCmdUI* pCmdUI);
	afx_msg void	OnEnableBackupcompanyBackup(CCmdUI *pCmdUI);
	afx_msg void	OnEnableBackupcompanyRestore(CCmdUI *pCmdUI);
	// InTimeTec: 03.05.2013.HS - Copy Organization Utility
	afx_msg void	OnEnableCopyOrganizationUtility(CCmdUI *pCmdUI);
	afx_msg void	OnCopyOrganizationUtility(void);
	// InTimeTec: End

	// PBI 22186 BK 8/19/14 Added red flag reports
	afx_msg void	OnEnableReportAPControlReport(CCmdUI *pCmdUI);
	afx_msg void	OnEnableReportAPGLExpenseDistributionReport(CCmdUI *pCmdUI);
	afx_msg void	OnEnableReportARControlReport(CCmdUI *pCmdUI);
	afx_msg void	OnEnableReportBRMissingCheckReport(CCmdUI *pCmdUI);
	afx_msg void	OnEnableReportGLReprintPostingReport(CCmdUI *pCmdUI);
	afx_msg void	OnEnableReportINControlReport(CCmdUI *pCmdUI);
	afx_msg void	OnEnableReportINMovementReport(CCmdUI *pCmdUI);
	afx_msg void	OnEnableReportPOOpenPurchaseOrderReport(CCmdUI *pCmdUI);
	afx_msg void	OnEnableReportSAPriceVarianceReport(CCmdUI *pCmdUI);
	afx_msg void	OnEnableReportSAInvoiceTrackingReport(CCmdUI *pCmdUI);

	afx_msg LRESULT DelayAddNewCompany(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLaunchDelayedCompanies(WPARAM wParam, LPARAM lParam);

	// BK 4/30/08 implemented Opening Denali Modules Routine spec
	bool m_bNoInstalledCompanies;
	bool m_bProcessingAddCompanies;
	BOOL ShowCreateDB();

	void InstallModules(bool bNewCompany, bool bConvertPayrollIntoExistingCompany);	//RCG - 03/30/2017 - Bug 32830 - Added second parameter for Convert Payroll into Denali

	// begbert 08-09-2010 3.0 Interface : command handlers
	virtual void OnNav_CTSetupSelectaNewCompany();
	virtual void OnNav_CTInstallModules();
	virtual void OnNav_CTConvertPayrollFromCMSPro();	// begbert 02-10-2014 PBI 138 : Added this
	virtual void OnNav_CTChangeDate();
	virtual void OnNav_CTFiscalCalendar();
	virtual void OnNav_CTElectronicConstants();
	virtual void OnNav_CTAddChangeUsers();
	virtual void OnNav_CTAddChangeGroups();
	virtual void OnNav_CTChangePassword();
	virtual void OnNav_CTResetPassword();
	virtual void OnNav_CTCopyUsersFromCompany();
	virtual void OnNav_CTCopyGroupsFromCompany();
	virtual void OnNav_CTCurrencyValues();
	virtual void OnNav_CTExchangeRates();
	virtual void OnNav_CTDisplayControllerTableInformation();
	virtual void OnNav_CTProcessPayrollTransactions();
	virtual void OnNav_CTBackupCompany();
	virtual void OnNav_CTRestoreCompany();
	virtual void OnNav_ControllerModulePreferences();
	virtual void OnNav_CTCopyOrganizationUtility(); // begbert 04-16-2013 PBI 4036 : Added this
	//	PBI 45155	06/06/2018	mvt - Added security for Denali BI Setup
	virtual void OnNav_CTDenaliBusinessIntelligence();
	//	PBI 47520	12/04/2018	mvt - Added security for Yodlee integration setup
	virtual void OnNav_CTYodleeIntegrationSetup();
	//	PBI 50961	09/04/2019	mvt - Added security for PayPal integration setup
	virtual void OnNav_CTPayPalIntegrationSetup();
	//	PBI 52317	11/19/2019	mvt - Added security for Donately integration setup
	virtual void OnNav_CTDonatelyIntegrationSetup();
	//	PBI 46406	09/26/2018	mvt - Added security for Internal Control Alerts in Controller
	virtual void OnNav_CTInternalControlAlertsSetup();
	virtual void OnNav_CTMerchantWareSetup();	// begbert 04-09-2014 PBI 17178 : added this
	//RS - 04/26/2024 - PBI 63797 - Added user security and menu item to Organization menu
	virtual void OnNav_CTCloverConnectSetup();
	virtual void OnNav_CTNeonCRMSetup();

	virtual CNavigatorCodeBatchInfo GetCodeBatchInfo(const CString& szDataType);

};
