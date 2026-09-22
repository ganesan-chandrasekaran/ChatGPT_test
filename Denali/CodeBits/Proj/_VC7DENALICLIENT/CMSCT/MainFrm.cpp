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

#include "stdafx.h"
#include "CMSCT32.h"

#include "MainFrm.h"
#include "..\GBLForms\Resource.h"
#include "..\CTForms\PPG_CTOrganizationalDetails.h"
#include "..\CMSCT\CMSCTReportsMenu.h"
#include "..\CTForms\CTDllFormManager.h"
#include "..\GBLForms\CMSMenuBuilder.h"
#include "..\CTForms\DLG_CTInstallModule.h" //1-11369 DTG, 6.2.2004
#include "..\CTForms\MFV_CTCurrencies.h"
#include "..\CTForms\CFV_CTCurrencyCodes.h"
 // BK 01/13/11 CFV_CTAddNewCompany2 IS NO LONGER USED (this file contains the template list for company setup )
 // #include "..\CTForms\CFV_CTAddNewCompany2.h"
#include "..\CTForms\DLG_CTCompanyCreation.h"
#include "..\CTForms\DLG_CTImportFilesFromQB.h"
#include "..\CTForms\DLG_CTImportTranFilesFromQB.h"
#include "..\CTForms\CFV_CTNewCustomDetails.h"
#include "..\CTForms\DLG_CTFiscalcalendar.h"
#include "..\CTForms\DLG_CTResetPassword.h"
#include "..\CTForms\DLG_CTImportPayroll.h"
#include "..\CTForms\DLG_CTMerchantWarehouse.h"	// begbert 04-09-2014 PBI 17178 : added this
#include "..\CTForms\DLG_CTDenaliBISetup.h"
#include "..\CTForms\DLG_CTNeonCRMSetup.h"
#include "..\CTForms\DLG_CTInternalControlAlertsSetup.h"
#include "..\CTForms\DlgSetUpYodleeIntegration.h"
#include "..\CTForms\DlgSetUpPayPalIntegration.h"
#include "..\CTForms\DLG_CTDonatelySetup.h"
#include "..\CTForms\CloseProgramAndLogOff.h"
#include "..\GBLForms\CFV_GBLChangePassword.h"
#include "..\GBLForms\CMSMessagebox.h"
#include "..\cmsinclude\GBLDataVersion.h"
#include "..\CTForms\DLG_CTCardPointeSetup.h"
// PBI 22186 BK 8/21/14 Added red flag reports
#include "..\APFORMS\Resource.h"
#include "..\ARFORMS\Resource.h"
#include "..\BRFORMS\Resource.h"
#include "..\GLFORMS\resource.h"
#include "..\INFORMS\Resource.h"
#include "..\POFORMS\Resource.h"
#include "..\SAFORMS\resource.h"
#include "..\CTForms\DLG_ChangeArcharinaAppsURL.h"

// InTimeTec: 11.20.2012.HS - Custom Menus
#define IDS_CUSTOM_MENU_START			10001
// InTimeTec: End

extern CCMSCT32App theApp;

IMPLEMENT_DYNAMIC(CMainFrame, CCMSMDIMainFrame) // Version 3.0 CCMSMDIMainFrame = CCMSSDIMainFrame

// -------------------------------------------------------------
BEGIN_MESSAGE_MAP(CMainFrame, CCMSMDIMainFrame) // Version 3.0 CCMSMDIMainFrame = CCMSSDIMainFrame
	ON_WM_CREATE()
	ON_WM_INITMENUPOPUP()

	ON_COMMAND(IDS_CHANGE_DATE, &CMainFrame::OnCompanyChangedate)
	ON_COMMAND(IDS_MERCHANT_WAREHOUSE_SETUP, &CMainFrame::OnCompanyMerchantWareSetup)	// begbert 04-09-2014 PBI 17178 : added this
	//RS - 04/26/2024 - PBI 63797 - Added user security and menu item to Organization menu
	ON_COMMAND(IDS_CARDPOINTE_SETUP, &CMainFrame::OnCompanyCardPointeSetup)
	ON_COMMAND(IDS_DENALIBI_MENU, &CMainFrame::OnCompanyDenaliBISetup)
	ON_COMMAND(IDS_NEONCRM_SETUP, &CMainFrame::OnCompanyNeonCRMSetup)	// begbert 04-09-2014 PBI 17178 : added this
	ON_COMMAND(IDS_DONATELYSETUP_MENU, &CMainFrame::OnCompanyDonatelySetup)
	ON_COMMAND(IDS_MENU_SETUPYODLEEINTEGRATION, &CMainFrame::OnCompanySetUpYodleeIntegration)
	ON_COMMAND(IDS_MENU_SETUPPAYPALINTEGRATION, &CMainFrame::OnCompanySetUpPayPalIntegration)
	ON_COMMAND(IDS_FISCAL_CALENDAR, &CMainFrame::OnCompanyFiscalcalendar)
	ON_COMMAND(IDS_INSTALL_MODULE, &CMainFrame::OnCompanyInstallmodules)
	ON_COMMAND(IDS_CONVERT_PAYROLL_FROM_CMSPRO, &CMainFrame::OnCompanyConvertPayrollFromCMSPro) // begbert 02-10-2014 PBI 138 : Added this
	ON_COMMAND(IDS_CTRL_MAGNETIC_MEDIA_CONST, &CMainFrame::OnCompanyMagneticmediaconstraints)
	ON_COMMAND(IDS_CT_INTERNALCONTROLALERTSSETUP_MENU, &CMainFrame::OnCompanyInternalControlAlerts)
	ON_COMMAND(IDS_MNU_SETUP_SELECT_ORG, &CMainFrame::OnCompanySetupSelectCompany)
	ON_COMMAND(IDS_MENU_CLOSE_PROGRAM_LOG_OFF, &CMainFrame::OnCompanyCloseLogoff)
	ON_COMMAND(IDS_CTRL_ADD_CHNG_USER, &CMainFrame::OnSecurityAdd)
	ON_COMMAND(IDS_CTRL_CHNG_PASSWD, &CMainFrame::OnSecurityChangepassword)
	ON_COMMAND(IDS_CT_CISP_RESETPASSWORD, &CMainFrame::OnSecurityResetPassword)
	ON_COMMAND(IDS_CT_CISP_ACTIVITYLOG, &CMainFrame::OnSecurityActivityLog)
	ON_COMMAND(IDS_MODULE_PREFERENCES, &CMainFrame::OnOptionsModulePrefs)
	ON_COMMAND(IDS_CAPTION_CURRENCIES, &CMainFrame::OnOptionsCurrencies)
	ON_COMMAND(IDS_CTRL_DISPLAY_FILE_INFO, &CMainFrame::OnOptionsDisplayfileinformaion)
	ON_COMMAND(IDS_PAY_PROCESS_PAYROLL, &CMainFrame::OnOptionsProcessPayroll)
	ON_COMMAND(IDS_MENU_CHANGE_ARCHARINA_APPS_URL, &CMainFrame::OnOptionsChangeURLtoArcharinaApps)//IDS_MENU_CHANGE_URL_TO_ARCHARINA_APPS
	ON_COMMAND(IDS_EXCHANGE_RATES, &CMainFrame::OnOptionsExchangerates)
	ON_COMMAND(IDS_CTRL_BKUP_DATA, &CMainFrame::OnBackupcompanyBackup)
	ON_COMMAND(IDS_CTRL_RESTORE_DATA, &CMainFrame::OnBackupcompanyRestore)
	ON_COMMAND(IDS_GROUPS, &CMainFrame::OnSecurityGroup)
	ON_COMMAND(IDS_CTRL_COPY_USERS, &CMainFrame::OnSecurityCopyUsers)
	ON_COMMAND(IDS_CTRL_COPY_GROUPS, &CMainFrame::OnSecurityCopyGroups)
	ON_COMMAND(IDS_MNU_OPEN_LICENSE_MGR, &CMainFrame::OnOpenLicenseManager)

	// integration failed reports from mod prefs
	ON_COMMAND(IDS_MNU_REMOVE_INTEG_REPORTS, &CMainFrame::OnReportRemoveIntegrationChecklist)
	ON_COMMAND(IDS_CTRL_INTEGRATION_CHECKLIST_REPORT_DATE1, &CMainFrame::OnReportIntegrationChecklistDate1)
	ON_COMMAND(IDS_CTRL_INTEGRATION_CHECKLIST_REPORT_DATE2, &CMainFrame::OnReportIntegrationChecklistDate2)
	ON_COMMAND(IDS_CTRL_INTEGRATION_CHECKLIST_REPORT_DATE3, &CMainFrame::OnReportIntegrationChecklistDate3)
	ON_COMMAND(IDS_CTRL_INTEGRATION_CHECKLIST_REPORT_DATE4, &CMainFrame::OnReportIntegrationChecklistDate4)
	ON_COMMAND(IDS_CTRL_INTEGRATION_CHECKLIST_REPORT_DATE5, &CMainFrame::OnReportIntegrationChecklistDate5)

	// 1-33328 BK 9/23/09 call the only report for each menu item (see CR 1-33386) replacing dated lists below
	ON_COMMAND(IDS_PRECONVERSION_CHECKLIST, &CMainFrame::OnReportPreConversionChecklist)
	ON_COMMAND(IDS_CTRL_CONVERSION_CODES_ADDED, &CMainFrame::OnReportCodesValuesAdded)
	ON_COMMAND(IDS_INTEGRATION_STATUS_RPT, &CMainFrame::OnReportIntegrationStatus)
	ON_COMMAND(IDS_MNU_REMOVE_CONVERSION_RPTS, &CMainFrame::OnRemoveConversionReports)

	// InTimeTec: 03.05.2013.HS - Copy Organization Utility
	ON_COMMAND(IDS_COPY_ORGANIZATION_UTILITY, &CMainFrame::OnCopyOrganizationUtility)
	// InTimeTec: End

	// 1-33827 BK 12/1/09 for CR 1-33454 added user rights to report menu items
	// 1-33328 BK 9/23/09 enable/disable conversion menu items depending on whether report exists
	//ON_UPDATE_COMMAND_UI(IDS_REPORTS,									&CMainFrame::OnEnableReportsMenu)
	ON_UPDATE_COMMAND_UI(IDS_PRECONVERSION_CHECKLIST, &CMainFrame::OnEnablePreConversionChecklist)
	ON_UPDATE_COMMAND_UI(IDS_CTRL_CONVERSION_CODES_ADDED, &CMainFrame::OnEnableCodesValuesAddedReport)
	ON_UPDATE_COMMAND_UI(IDS_INTEGRATION_STATUS_RPT, &CMainFrame::OnEnableIntegrationStatusReport)
	// 1-33328 BK 9/23/09 remove integration/conversion reports will always be enabled
	ON_UPDATE_COMMAND_UI(IDS_MNU_REMOVE_INTEG_REPORTS, &CMainFrame::OnEnableRemoveIntegrationReports)
	ON_UPDATE_COMMAND_UI(IDS_MNU_REMOVE_CONVERSION_RPTS, &CMainFrame::OnEnableRemoveConversionReports)

	ON_UPDATE_COMMAND_UI(IDS_MNU_SETUP_SELECT_ORG, &CMainFrame::OnEnableSetupSelectCompanyButton)
	ON_UPDATE_COMMAND_UI(IDS_CTRL_ADD_CHNG_USER, &CMainFrame::OnEnableChangeUsersButton)
	ON_UPDATE_COMMAND_UI(IDS_SETUP_WIZARD, &CMainFrame::OnEnableSetupWizardButton)
	ON_UPDATE_COMMAND_UI(IDS_ADD_CHANGE_USERS_CAPTION, &CMainFrame::OnEnableChangeUsersButton)
	ON_UPDATE_COMMAND_UI(IDS_INSTALL_MODULE, &CMainFrame::OnEnableCompanyInstallmodules)
	ON_UPDATE_COMMAND_UI(IDS_CONVERT_PAYROLL_FROM_CMSPRO, &CMainFrame::OnEnableCompanyConvertPayrollFromCMSPro) // begbert 02-10-2014 PBI 138 : Added this
	ON_UPDATE_COMMAND_UI(IDS_CHANGE_DATE, &CMainFrame::OnEnableCompanyChangedate)
	ON_UPDATE_COMMAND_UI(IDS_FISCAL_CALENDAR, &CMainFrame::OnEnableCompanyFiscalcalendar)
	ON_UPDATE_COMMAND_UI(IDS_MERCHANT_WAREHOUSE_SETUP, &CMainFrame::OnEnableCompanyMerchantWareSetup)	// begbert 04-09-2014 PBI 17178 : added this
	//RS - 04/26/2024 - PBI 63797 - Added user security and menu item to Organization menu
	ON_UPDATE_COMMAND_UI(IDS_CARDPOINTE_SETUP, &CMainFrame::OnEnableCompanyCardPointeSetup)
	//	PBI 45144	06/06/2018	mvt - Added security for Denali BI Setup
	ON_UPDATE_COMMAND_UI(IDS_DENALIBI_MENU, &CMainFrame::OnEnableCompanyDenaliBISetup)
	//	PBI 47520	12/04/2018	mvt - Added security for Yodlee integration setup
	ON_UPDATE_COMMAND_UI(IDS_MENU_SETUPYODLEEINTEGRATION, &CMainFrame::OnEnableCompanySetUpYodleeIntegration)
	ON_UPDATE_COMMAND_UI(IDS_MENU_SETUPPAYPALINTEGRATION, &CMainFrame::OnEnableCompanySetUpPayPalIntegration)
	// PBI 46406	09/26/2018	mvt - Added security for Internal Control Alerts in Controller
	ON_UPDATE_COMMAND_UI(IDS_CT_INTERNALCONTROLALERTSSETUP_MENU, &CMainFrame::OnEnableCompanyInternalControlAlertsSetup)
	ON_UPDATE_COMMAND_UI(IDS_NEONCRM_SETUP, &CMainFrame::OnEnableCompanyNeonCRMSetup)	// begbert 04-09-2014 PBI 17178 : added this
	ON_UPDATE_COMMAND_UI(IDS_DONATELYSETUP_MENU, &CMainFrame::OnEnableCompanyDonatelySetup)
	ON_UPDATE_COMMAND_UI(IDS_CTRL_MAGNETIC_MEDIA_CONST, &CMainFrame::OnEnableCompanyMagneticmediaconstraints)
	ON_UPDATE_COMMAND_UI(IDS_GROUPS, &CMainFrame::OnEnableSecurityGroup)
	ON_UPDATE_COMMAND_UI(IDS_CTRL_CHNG_PASSWD, &CMainFrame::OnEnableSecurityChangePassword)
	ON_UPDATE_COMMAND_UI(IDS_CT_CISP_RESETPASSWORD, &CMainFrame::OnEnableSecurityResetPassword)
	ON_UPDATE_COMMAND_UI(IDS_CTRL_COPY_USERS, &CMainFrame::OnEnableSecurityCopyUsers)
	ON_UPDATE_COMMAND_UI(IDS_CTRL_COPY_GROUPS, &CMainFrame::OnEnableSecurityCopyGroups)
	ON_UPDATE_COMMAND_UI(IDS_MODULE_PREFERENCES, &CMainFrame::OnEnableModulePrefs)
	ON_UPDATE_COMMAND_UI(IDS_CAPTION_CURRENCIES, &CMainFrame::OnEnableOptionsCurrencies)
	ON_UPDATE_COMMAND_UI(IDS_EXCHANGE_RATES, &CMainFrame::OnEnableOptionsExchangerates)
	ON_UPDATE_COMMAND_UI(IDS_CTRL_DISPLAY_FILE_INFO, &CMainFrame::OnEnableOptionsDisplayfileinformaion)
	ON_UPDATE_COMMAND_UI(IDS_PAY_PROCESS_PAYROLL, &CMainFrame::OnEnableOptionsProcessPayroll)
	ON_UPDATE_COMMAND_UI(IDS_MENU_CHANGE_ARCHARINA_APPS_URL, &CMainFrame::OnEnableOptionsChangeURLtoArcharinaApps)
	ON_UPDATE_COMMAND_UI(IDS_CTRL_BKUP_DATA, &CMainFrame::OnEnableBackupcompanyBackup)
	ON_UPDATE_COMMAND_UI(IDS_CTRL_RESTORE_DATA, &CMainFrame::OnEnableBackupcompanyRestore)
	// InTimeTec: 03.05.2013.HS - Copy Organization Utility
	ON_UPDATE_COMMAND_UI(IDS_COPY_ORGANIZATION_UTILITY, &CMainFrame::OnEnableCopyOrganizationUtility)
	// InTimeTec: End

	// PBI 22186 BK 8/19/14 Added red flag reports
	ON_UPDATE_COMMAND_UI(IDS_AP_CAP_CONTROL, &CMainFrame::OnEnableReportAPControlReport)
	ON_UPDATE_COMMAND_UI(IDS_AP_GL_EXPENSE_DISTRIBUTION_REPORT, &CMainFrame::OnEnableReportAPGLExpenseDistributionReport)
	ON_UPDATE_COMMAND_UI(IDS_AR_CONTROL_REPORT, &CMainFrame::OnEnableReportARControlReport)
	ON_UPDATE_COMMAND_UI(IDS_BR_MISSING_CHECK, &CMainFrame::OnEnableReportBRMissingCheckReport)
	ON_UPDATE_COMMAND_UI(IDS_GLREPRINTPOSTINGREPORT, &CMainFrame::OnEnableReportGLReprintPostingReport)
	ON_UPDATE_COMMAND_UI(IDS_CAP_INCONTROL_REPORT_PREVIEW, &CMainFrame::OnEnableReportINControlReport)
	ON_UPDATE_COMMAND_UI(IDS_CAP_INMOVEMENT_REPORT_PREVIEW, &CMainFrame::OnEnableReportINMovementReport)
	ON_UPDATE_COMMAND_UI(IDS_PO_OPEN_PURCHASE_ORDER, &CMainFrame::OnEnableReportPOOpenPurchaseOrderReport)
	ON_UPDATE_COMMAND_UI(IDS_SA_PRICE_VARIANCE, &CMainFrame::OnEnableReportSAPriceVarianceReport)
	ON_UPDATE_COMMAND_UI(IDS_SA_INVOICE_TRACKING_RPT, &CMainFrame::OnEnableReportSAInvoiceTrackingReport)

	ON_COMMAND(IDS_AP_CAP_CONTROL, &CMainFrame::OnReportAPControlReport)
	ON_COMMAND(IDS_AP_GL_EXPENSE_DISTRIBUTION_REPORT, &CMainFrame::OnReportAPGLExpenseDistributionReport)
	ON_COMMAND(IDS_AR_CONTROL_REPORT, &CMainFrame::OnReportARControlReport)
	ON_COMMAND(IDS_BR_MISSING_CHECK, &CMainFrame::OnReportBRMissingCheckReport)
	ON_COMMAND(IDS_GLREPRINTPOSTINGREPORT, &CMainFrame::OnReportGLReprintPostingReport)
	ON_COMMAND(IDS_CAP_INCONTROL_REPORT_PREVIEW, &CMainFrame::OnReportINControlReport)
	ON_COMMAND(IDS_CAP_INMOVEMENT_REPORT_PREVIEW, &CMainFrame::OnReportINMovementReport)
	ON_COMMAND(IDS_PO_OPEN_PURCHASE_ORDER, &CMainFrame::OnReportPOOpenPurchaseOrderReport)
	ON_COMMAND(IDS_SA_PRICE_VARIANCE, &CMainFrame::OnReportSAPriceVarianceReport)
	ON_COMMAND(IDS_SA_INVOICE_TRACKING_RPT, &CMainFrame::OnReportSAInvoiceTrackingReport)

	ON_MESSAGE(WM_USER_MESSAGE_1, &CMainFrame::DelayAddNewCompany)
	ON_MESSAGE(WM_USER_MESSAGE_3, &CMainFrame::OnLaunchDelayedCompanies)

END_MESSAGE_MAP()

// -------------------------------------------------------------
CMainFrame::CMainFrame(void)
	: m_CTGUIAccess()
{
	m_bNoInstalledCompanies = false;
	m_bProcessingAddCompanies = false;
	CanCTAccess[SetupSelectCompanyButton] = true;
	CanCTAccess[SetupWizardButton] = true;
	CanCTAccess[ChangeUsersButton] = true;

	ExitStatusMsgID = IDS_STATUS_EXIT_CONTROLLER;
	TitleBarID = IDS_MAIN_MENU;
	// 1-33328 BK 9/23/09 initialize reports menu class
	CCMSCTReportsMenu::SetMainFrame(*this);
}

// -------------------------------------------------------------
CMainFrame::~CMainFrame(void)
{
	si->IApplicationMainFrame = NULL;
}

// going through entire menu and toolbar to disable items based on user / company security settings
// -------------------------------------------------------------
// 1-26148 BK 12/07/06 changed to new code that disables top level menu items when user doesn't have rights
// old code doesn't work is replaced here and an OnEnablexxx() was added for each individual menu item
void CMainFrame::MenuSecurityDisableItems(void)
{
	__super::MenuSecurityDisableItems();

	const stuCTNode& CTRights = si->UserRight.m_stuControllerNode;
	int	  nPos = -1;

	// 1-26597 BK 12/18/06 set user rights items into CanCTAccess[] items
	CanCTAccess[SetupSelectCompanyButton] = CTRights.Company.bSetupSelectCompany ? true : false;
	CanCTAccess[ChangeUsersButton] = CTRights.Security.bAddChangeUsers ? true : false;
	CanCTAccess[SetupWizardButton] = false; // not using yet

	// Company Menu
	CMenu* const pCompanyMenu = Menu[IDS_ORGANIZATION_OPTION];
	if (pCompanyMenu)
	{
		nPos = MenuData[IDS_ORGANIZATION_OPTION].MenuIndex;
		if (si->UserInformation.ID.IsEmpty())
			MenuData[IDS_ORGANIZATION_OPTION].ParentMenu->EnableMenuItem(nPos, MF_BYPOSITION | MF_GRAYED);
		else
			MenuData[IDS_ORGANIZATION_OPTION].ParentMenu->EnableMenuItem(nPos, MF_BYPOSITION | (CTRights.Company.bTotal ? MF_ENABLED : MF_GRAYED));

		// 1-35038 BK 11/11/10 Electronic Constants Menu - enable/disable menu based on user rights
		CMenu* const pElectronicConstants = Menu[IDS_CTRL_MAGNETIC_MEDIA_CONST];
		if (pElectronicConstants)
		{
			nPos = MenuData[IDS_CTRL_MAGNETIC_MEDIA_CONST].MenuIndex;
			if (si->UserInformation.ID.IsEmpty())
				MenuData[IDS_CTRL_MAGNETIC_MEDIA_CONST].ParentMenu->EnableMenuItem(nPos, MF_BYPOSITION | MF_GRAYED);
			else
				MenuData[IDS_CTRL_MAGNETIC_MEDIA_CONST].ParentMenu->EnableMenuItem(nPos, MF_BYPOSITION | (CTRights.Company.bMagneticMediaConstants ? MF_ENABLED : MF_GRAYED));
		}
	}

	// Security Menu
	CMenu* const pSecurityMenu = Menu[IDS_MENU_SECURITY];
	if (pSecurityMenu)
	{
		nPos = MenuData[IDS_MENU_SECURITY].MenuIndex;
		if (si->UserInformation.ID.IsEmpty())
			MenuData[IDS_MENU_SECURITY].ParentMenu->EnableMenuItem(nPos, MF_BYPOSITION | MF_GRAYED);
		else
			MenuData[IDS_MENU_SECURITY].ParentMenu->EnableMenuItem(nPos, MF_BYPOSITION | (CTRights.Security.bTotal ? MF_ENABLED : MF_GRAYED));
	}

	// Reports Menu
	// 1-33827 BK 12/1/09 for CR 1-33454 added user rights to report menu items - Reports Submenus are handled in MenuUpdateReports()
	CMenu* const pReportsMenu = Menu[IDS_REPORTS];
	if (pReportsMenu)
	{
		nPos = MenuData[IDS_REPORTS].MenuIndex;
		if (si->UserInformation.ID.IsEmpty())
			MenuData[IDS_REPORTS].ParentMenu->EnableMenuItem(nPos, MF_BYPOSITION | MF_GRAYED);
		else
			MenuData[IDS_REPORTS].ParentMenu->EnableMenuItem(nPos, MF_BYPOSITION | (CTRights.Reports.bTotal ? MF_ENABLED : MF_GRAYED));

		// PBI 22186 BK 8/19/14 Added red flag reports
		nPos = MenuData[IDS_RED_FLAG_REPORTS].MenuIndex;
		if (si->UserInformation.ID.IsEmpty())
			MenuData[IDS_RED_FLAG_REPORTS].ParentMenu->EnableMenuItem(nPos, MF_BYPOSITION | MF_GRAYED);
		else
		{
			bool bRedFlagReports = (CTRights.Reports.bRedFlagReports) ? true : false;
			MenuData[IDS_RED_FLAG_REPORTS].ParentMenu->EnableMenuItem(nPos, MF_BYPOSITION | (bRedFlagReports ? MF_ENABLED : MF_GRAYED));
		}
	}

	// Options Menu
	CMenu* const pOptionsMenu = Menu[IDS_OPTIONS];
	if (pOptionsMenu)
	{
		CMenu* const pBackupMenu = Menu[IDS_CT_BACK_UP_COMPANY];
		if (pBackupMenu)
		{
			nPos = MenuData[IDS_CT_BACK_UP_COMPANY].MenuIndex;
			if (si->UserInformation.ID.IsEmpty())
				MenuData[IDS_CT_BACK_UP_COMPANY].ParentMenu->EnableMenuItem(nPos, MF_BYPOSITION | MF_GRAYED);
			else
			{
				bool bBackupCompanyMenu = false;
				if (CTRights.Options.BackupCompany.bBackup || CTRights.Options.BackupCompany.bRestore)
					bBackupCompanyMenu = true;
				MenuData[IDS_CT_BACK_UP_COMPANY].ParentMenu->EnableMenuItem(nPos, MF_BYPOSITION | (bBackupCompanyMenu ? MF_ENABLED : MF_GRAYED));
			}
		}
	}

	// InTimeTec: 04.16.2013.HS - Custom Menus
	// Apply Security settings when user login changed
	CustomMenuSecurityDisableItem();
	// InTimeTec: End

	DrawMenuBar();
}

// -------------------------------------------------------------
bool CMainFrame::CreateMenu(void)
{
	if (!AddMenu(&MainMenu, IDS_ORGANIZATION_OPTION))																				return false; // company main popup (BK 1/13/11 changed to &Organization)
	if (!AddMenu(Menu[IDS_ORGANIZATION_OPTION], IDS_MNU_SETUP_SELECT_ORG, IDS_MNU_SETUP_SELECT_ORG, IDS_STATUS_SETUP_SELECT_ORG))	return false;
	if (!AddMenu(Menu[IDS_ORGANIZATION_OPTION], IDS_MENU_CLOSE_PROGRAM_LOG_OFF, IDS_MENU_CLOSE_PROGRAM_LOG_OFF, IDS_STATUS_CLOSE_PROGRAM_LOG_OFF))	return false;
	if (!AddSeparator(Menu[IDS_ORGANIZATION_OPTION]))																				return false;
	if (!AddMenu(Menu[IDS_ORGANIZATION_OPTION], IDS_INSTALL_MODULE, IDS_CT_INSTALL_MODULES_ELIPSIS, IDS_STATUS_INSTALL_MODULES))	return false;
	if (!AddMenu(Menu[IDS_ORGANIZATION_OPTION], IDS_CONVERT_PAYROLL_FROM_CMSPRO, IDS_CT_CONVERT_PAYROLL_FROM_CMSPRO_ELIPSIS, IDS_STATUS_CONVERT_PAYROLL_FROM_CMSPRO))	return false; // begbert 02-10-2014 PBI 138 : Added this
	if (!AddSeparator(Menu[IDS_ORGANIZATION_OPTION]))																				return false;
	if (!AddMenu(Menu[IDS_ORGANIZATION_OPTION], IDS_CHANGE_DATE, IDS_CT_CHANGE_DATE_ELIPSIS, IDS_STATUS_CHANGE_DATE))				return false;
	if (!AddSeparator(Menu[IDS_ORGANIZATION_OPTION]))																				return false;
	if (!AddMenu(Menu[IDS_ORGANIZATION_OPTION], IDS_FISCAL_CALENDAR, IDS_CT_FISCAL_CALENDAR_ELIPSIS, IDS_STATUS_FISCAL_CALENDAR))	return false;
	// PBI 17783 BK 3/11/14 added new Merchant Warehouse setup form
	if (!AddSeparator(Menu[IDS_ORGANIZATION_OPTION]))																				return false;
	if (!AddMenu(Menu[IDS_ORGANIZATION_OPTION], IDS_MERCHANT_WAREHOUSE_SETUP, IDS_MERCHWARE_MENU, IDS_MERCHWARE_STATUS))			return false;
	//RS - 04/26/2024 - PBI 63797 - Added user security and menu item to Organization menu
	if (!AddMenu(Menu[IDS_ORGANIZATION_OPTION], IDS_CARDPOINTE_SETUP, IDS_CARDPOINTE_MENU, IDS_CARDPOINTE_STATUS))			return false;
	m_nDenaliBIMenuPos = (UINT)Menu[IDS_ORGANIZATION_OPTION]->GetMenuItemCount();
	if (!AddMenu(Menu[IDS_ORGANIZATION_OPTION], IDS_DENALIBI_MENU, IDS_DENALIBI_MENU, IDS_DENALIBI_STATUS))							return false;
	m_nRawNeonCRMMenuPos = (UINT)Menu[IDS_ORGANIZATION_OPTION]->GetMenuItemCount();
	if (!AddMenu(Menu[IDS_ORGANIZATION_OPTION], IDS_NEONCRM_SETUP, IDS_NEONCRMSETUP_MENU, IDS_NEONCRMSETUP_STATUS))					return false;
	m_nRawDonatelyMenuPos = (UINT)Menu[IDS_ORGANIZATION_OPTION]->GetMenuItemCount();
	if (!AddMenu(Menu[IDS_ORGANIZATION_OPTION], IDS_DONATELYSETUP_MENU, IDS_DONATELYSETUP_MENU, IDS_DONATELYSETUP_STATUS))	return false;

	if (!AddMenu(&MainMenu, IDS_MENU_SECURITY))																						return false; // security main popup
	if (!AddMenu(Menu[IDS_MENU_SECURITY], IDS_CTRL_ADD_CHNG_USER, IDS_CT_ADD_CHANGE_USERS_ELIPSIS, IDS_STATUS_ADD_CHANGE_USERS))	return false; // add / change user
	if (!AddMenu(Menu[IDS_MENU_SECURITY], IDS_GROUPS, IDS_CT_ADD_CHANGE_GROUPS_ELIPSIS, IDS_STATUS_ADD_CHANGE_GROUPS))				return false; // change password
	if (!AddMenu(Menu[IDS_MENU_SECURITY], IDS_CTRL_CHNG_PASSWD, IDS_CT_CHANGE_PASSWORD_ELIPSIS, IDS_STATUS_CHANGE_PASSWORD))		return false; // change password

	if (!AddMenu(Menu[IDS_MENU_SECURITY], IDS_CT_CISP_RESETPASSWORD, IDS_CT_CISP_RESETPASSWORD, IDS_CT_CISP_RESETPASSWORD_STATUSMSG))	return false; // reset password

	if (!AddSeparator(Menu[IDS_MENU_SECURITY]))																						return false;
	if (!AddMenu(Menu[IDS_MENU_SECURITY], IDS_CTRL_COPY_USERS, IDS_CTRL_COPY_USERS, IDS_STATUS_COPY_USERS))							return false;
	if (!AddMenu(Menu[IDS_MENU_SECURITY], IDS_CTRL_COPY_GROUPS, IDS_CTRL_COPY_GROUPS, IDS_STATUS_COPY_GROUPS))						return false;

#ifdef DENALI_POST_2_3	//Defect ID 1-22902, PGP(01/19/2006) - As per error - removed till 2.3.
	if (!AddSeparator(Menu[IDS_MENU_SECURITY]))																						return false;
	if (!AddMenu(Menu[IDS_MENU_SECURITY], IDS_CT_CISP_ACTIVITYLOG, IDS_CT_CISP_ACTIVITYLOG, IDS_CT_CISP_AUTH_LOG_STATUSMSG))		return false;
#endif

	// 1-33328 BK 9/15/09 reports menu for integration/conversion reports
	// reports
	if (!AddMenu(&MainMenu, IDS_REPORTS))																							return false;
	// reports - integration
	if (!AddMenu(Menu[IDS_REPORTS], IDS_CTRL_INTEGRATION))																			return false;
	if (!AddMenu(Menu[IDS_CTRL_INTEGRATION], IDS_INTEGRATION_ERROR_RPT))															return false;
	AddSeparator(Menu[IDS_CTRL_INTEGRATION]);
	if (!AddMenu(Menu[IDS_CTRL_INTEGRATION], IDS_MNU_REMOVE_INTEG_REPORTS, IDS_MNU_REMOVE_INTEG_REPORTS, IDS_STATUS_REMOVE_INTEG_RPTS))	return false;
	// reports - conversion
	if (!AddMenu(Menu[IDS_REPORTS], IDS_CTRL_CONVERSION))																			return false;
	if (!AddMenu(Menu[IDS_CTRL_CONVERSION], IDS_PRECONVERSION_CHECKLIST, IDS_PRECONVERSION_CHECKLIST, IDS_STATUS_PRECONVERSION_CHECKLIST))	return false;
	if (!AddMenu(Menu[IDS_CTRL_CONVERSION], IDS_CTRL_CONVERSION_CODES_ADDED, IDS_CTRL_CONVERSION_CODES_ADDED, IDS_STATUS_CODESVALUES_RPT))	return false;
	if (!AddMenu(Menu[IDS_CTRL_CONVERSION], IDS_INTEGRATION_STATUS_RPT, IDS_INTEGRATION_STATUS_RPT, IDS_STATUS_INTEGRATION_STATUS_RPT))		return false;
	AddSeparator(Menu[IDS_CTRL_CONVERSION]);
	if (!AddMenu(Menu[IDS_CTRL_CONVERSION], IDS_MNU_REMOVE_CONVERSION_RPTS, IDS_MNU_REMOVE_CONVERSION_RPTS, IDS_STATUS_REMOVE_CONVERSION))	return false;

	// PBI 22186 BK 8/19/14 Added red flag reports
	if (!AddSeparator(Menu[IDS_REPORTS]))																							return false;
	if (!AddMenu(Menu[IDS_REPORTS], IDS_RED_FLAG_REPORTS))																			return false;
	if (!AddMenu(Menu[IDS_RED_FLAG_REPORTS], IDS_AP_CAP_CONTROL, IDS_AP_CAP_CONTROL, IDS_AP_CAP_CONTROL))							return false;
	if (!AddMenu(Menu[IDS_RED_FLAG_REPORTS], IDS_AP_GL_EXPENSE_DISTRIBUTION_REPORT, IDS_AP_GL_EXPENSE_DISTRIBUTION_REPORT, IDS_AP_GL_EXPENSE_DISTRIBUTION_REPORT))	return false;
	if (!AddMenu(Menu[IDS_RED_FLAG_REPORTS], IDS_AR_CONTROL_REPORT, IDS_AR_CONTROL_REPORT, IDS_AR_CONTROL_REPORT))					return false;
	if (!AddMenu(Menu[IDS_RED_FLAG_REPORTS], IDS_BR_MISSING_CHECK, IDS_BR_MISSING_CHECK, IDS_BR_MISSING_CHECK))						return false;
	if (!AddMenu(Menu[IDS_RED_FLAG_REPORTS], IDS_GLREPRINTPOSTINGREPORT, IDS_GLREPRINTPOSTINGREPORT, IDS_GLREPRINTPOSTINGREPORT))	return false;
	if (!AddMenu(Menu[IDS_RED_FLAG_REPORTS], IDS_CAP_INCONTROL_REPORT_PREVIEW, IDS_CAP_INCONTROL_REPORT_PREVIEW, IDS_CAP_INCONTROL_REPORT_PREVIEW))	return false;
	if (!AddMenu(Menu[IDS_RED_FLAG_REPORTS], IDS_CAP_INMOVEMENT_REPORT_PREVIEW, IDS_CAP_INMOVEMENT_REPORT_PREVIEW, IDS_CAP_INMOVEMENT_REPORT_PREVIEW))	return false;
	if (!AddMenu(Menu[IDS_RED_FLAG_REPORTS], IDS_PO_OPEN_PURCHASE_ORDER, IDS_PO_OPEN_PURCHASE_ORDER, IDS_PO_OPEN_PURCHASE_ORDER))	return false;
	if (!AddMenu(Menu[IDS_RED_FLAG_REPORTS], IDS_SA_PRICE_VARIANCE, IDS_SA_PRICE_VARIANCE, IDS_SA_PRICE_VARIANCE))					return false;
	if (!AddMenu(Menu[IDS_RED_FLAG_REPORTS], IDS_SA_INVOICE_TRACKING_RPT, IDS_SA_INVOICE_TRACKING_RPT, IDS_SA_INVOICE_TRACKING_RPT))	return false;

	// options
	if (!AddMenu(&MainMenu, IDS_OPTIONS))																							return false; // options main popup
	if (!AddMenu(Menu[IDS_OPTIONS], IDS_MODULE_PREFERENCES, IDS_MODULE_PREFERENCES, IDS_STATUS_CT_PREF))							return false;
	if (!AddMenu(Menu[IDS_OPTIONS], IDS_STAY_IN_ADD_MODE_ON_SAVE, IDS_STAY_IN_ADD_MODE_ON_SAVE, IDS_STAY_IN_ADD_MODE_ON_SAVE))		return false;
	Menu[IDS_OPTIONS]->CheckMenuItem(IDS_STAY_IN_ADD_MODE_ON_SAVE, MF_CHECKED | MF_BYCOMMAND);

	if (!AddMenu(Menu[IDS_OPTIONS], IDS_DISPLAY_TOOLBAR, IDS_DISPLAY_TOOLBAR, IDS_DISPLAY_TOOLBAR))									return false;
	Menu[IDS_OPTIONS]->CheckMenuItem(IDS_DISPLAY_TOOLBAR, MF_CHECKED | MF_BYCOMMAND);

	if (!AddMenu(Menu[IDS_OPTIONS], IDS_MENU_GL_DISPLAY_STATUSBAR, IDS_MENU_GL_DISPLAY_STATUSBAR, IDS_MENU_GL_DISPLAY_STATUSBAR))	return false;
	Menu[IDS_OPTIONS]->CheckMenuItem(IDS_MENU_GL_DISPLAY_STATUSBAR, MF_CHECKED | MF_BYCOMMAND);

	AddSeparator(Menu[IDS_OPTIONS]);

	if (!AddMenu(Menu[IDS_OPTIONS], IDS_CAPTION_CURRENCIES, IDS_CT_CURRENCIES_ELIPSIS, IDS_CAPTION_CURRENCIES))						return false; // currencies
	if (!AddMenu(Menu[IDS_OPTIONS], IDS_EXCHANGE_RATES, IDS_CT_EXCHANGE_RATE_ELIPSIS, IDS_EXCHANGE_RATES))							return false; // exchange rates
	if (!AddMenu(Menu[IDS_OPTIONS], IDS_CTRL_DISPLAY_FILE_INFO, IDS_MENU_DISPLAY_FILE_INFO, IDS_STATUS_DISPLAY_FILE_INFORMATION))	return false; // display file info
	if (!AddMenu(Menu[IDS_OPTIONS], IDS_PAY_PROCESS_PAYROLL, IDS_PAY_PROCESS_PAYROLL, IDS_PAY_PROCESS_PAYROLL))						return false; // Process Payroll Transaction

	if (!AddMenu(Menu[IDS_OPTIONS], IDS_CT_BACK_UP_COMPANY))																		return false;// backup company info popup
	if (!AddMenu(Menu[IDS_CT_BACK_UP_COMPANY], IDS_CTRL_BKUP_DATA, IDS_CT_BACK_UP_ELIPSIS, IDS_STATUS_BACKUP_COMPANY))				return false; // backup
	if (!AddMenu(Menu[IDS_CT_BACK_UP_COMPANY], IDS_CTRL_RESTORE_DATA, IDS_CT_RESTORE_ELIPSIS, IDS_STATUS_RESTORE_COMPANY_DATA))		return false; // backup

	// InTimeTec: 03.05.2013.HS - Copy Organization Utility
	// Add Copy Organization Utility menu
	if (!AddMenu(Menu[IDS_OPTIONS], IDS_COPY_ORGANIZATION_UTILITY, IDS_COPY_ORGANIZATION_UTILITY, IDS_CANCEL_COPY_ORGANIZATION_MESSAGE)) return false;
	// InTimeTec: End

	return true;
}

// -------------------------------------------------------------
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)	return -1;

	return 0;
}

// CMainFrame message handlers
// -------------------------------------------------------------
void CMainFrame::OnCompanyChangedate(void)
{
	CGBLFormWrap::ShowChangeDate();
}

// -------------------------------------------------------------
void CMainFrame::OnCompanyFiscalcalendar(void)
{
	CDLG_CTFiscalCalendar dlg;
	dlg.DoModal();
}

// -------------------------------------------------------------
// PBI 17783 BK 3/11/14 added new Merchant Warehouse setup form
void CMainFrame::OnCompanyMerchantWareSetup(void)
{
	CDLG_CTMerchantWarehouse dlg;
	dlg.DoModal();
}

//--------------------------------------------------------------
//RS - 04/26/2024 - PBI 63797 - Added user security and menu item to Organization menu
void CMainFrame::OnCompanyCardPointeSetup(void)
{
	CDLG_CTCardPointeSetup dlg;
	dlg.DoModal();
}

// -------------------------------------------------------------
void CMainFrame::OnCompanyDenaliBISetup(void)
{
	if (ShouldDenaliBISetupMenuShow())
	{
		CDLG_CTDenaliBISetup dlg;
		dlg.DoModal();
	}
}

// -------------------------------------------------------------
void CMainFrame::OnCompanyNeonCRMSetup(void)
{
	//RCG - 03/02/2018 - PBI 44247 - Added check so user, mainly SUP, cannot display window from Navigation without meeting criteria
	if (ShouldNeonCRMSetupMenuShow())
	{
		CDLG_CTNeonCRMSetup dlg;
		dlg.DoModal();
	}
}

// -------------------------------------------------------------
// begbert 11/19/2019 PBI 52317 : Added the Donately import
void CMainFrame::OnCompanyDonatelySetup(void)
{
	if (ShouldDonatelySetupMenuShow())
	{
		CDLG_CTDonatelySetup dlg;
		dlg.DoModal();
	}
}

// -------------------------------------------------------------
void CMainFrame::OnCompanySetUpYodleeIntegration(void)
{
	CDlgSetUpYodleeIntegration dlg;
	dlg.DoModal();
}

// -------------------------------------------------------------
void CMainFrame::OnCompanySetUpPayPalIntegration(void)
{
	CDlgSetUpPayPalIntegration dlg;
	dlg.DoModal();
}

// -------------------------------------------------------------
void CMainFrame::OnCompanyInstallmodules(void)
{
	InstallModules(false, false);	//RCG - 03/30/2017 - Bug 32830 - Added second parameter for Convert Payroll into Denali
}

// -------------------------------------------------------------
// 1-31712 BK created this function to differentiate installing modules in new or existing company
// don't show message to backup files when creating a new company
//RCG - 03/30/2017 - Bug 32830 - Added second parameter for Convert Payroll into Denali
void CMainFrame::InstallModules(bool bNewCompany, bool bConvertPayrollIntoExistingCompany)
{
	// 1-11369 DTG, 6.2.2004
	CDLG_CTInstallModule dlg;

	// locking record
	CXML clsXML;
	clsXML.m_bUseDefinedServer = true;
	clsXML.m_uinDefinedServer = CTY_PRIMARYSERVER;
	CStringArray strCommandValues;
	strCommandValues.Add(_T("strCompanyID")); //add key to lock record
	strCommandValues.Add(si->CompanySettings.ID);
	// 1-11369 DTG, 6.2.2004
	if (clsXML.LockRecord(RECORD_LOCK, _T("GB"), _T("CMSCOMPANIES"), &strCommandValues))
	{
		// 1-11369 DTG, 6.2.2004
		dlg.GUID(clsXML.m_szGUID);
		// 1-31712 BK set variable in dialog to know if creating new company
		dlg.SetCreatingNewCompany(bNewCompany);
		//RCG - 03/30/2017 - Bug 32830 - Make server call if Convert Payroll into Denali
		if (!bConvertPayrollIntoExistingCompany)
		{
			dlg.DoModal();
		}
		else
		{
			dlg.MakePayrollModuleUninstallServerCall();
		}
	}
}

// -------------------------------------------------------------
// begbert 02-10-2014 PBI 138 : Added this
void CMainFrame::OnCompanyConvertPayrollFromCMSPro(void)
{
	if (!CMS::GetSI()->ModulesInstalled.Payroll.Licensed) return;	// Shouldn't be possible thanks to the menu item only appearing if licensed, so just bail.

	if (si->ModulesInstalled.Payroll.InCompany)
	{
		if (CGBLForm::CMSMessageBox(this, IDS_CT_CONVERT_PAYROLL_OVER_EXISTING_COMPANY_WARNING, IDS_WARNING, MB_OKCANCEL | MB_ICONWARNING) == IDCANCEL)
			return;
		//Uninstall Payroll module
		InstallModules(false, true);
	}
	//RCG - 03/30/2017 - Bug 32830 - Check if Payroll tables exist, even though Payroll module is not installed
	else
	{
		//Check if Payroll tables exist
		bool bTablesExist = DoPayrollTablesExist();
		if (bTablesExist)
		{
			if (CGBLForm::CMSMessageBox(this, IDS_CT_CONVERT_PAYROLL_OVER_EXISTING_COMPANY_WARNING, IDS_WARNING, MB_OKCANCEL | MB_ICONWARNING) == IDCANCEL)
				return;
		}
	}

	//Display CMS Professional to Denali Conversion window
	CDLG_CT16to32 dlg16to32;
	dlg16to32.SetIsThisNewFund(si->CompanySettings.IsFund);
	dlg16to32.SetConvertPayrollIntoExistingCompany(true);
	dlg16to32.DoModal();

	//RCG - 04/04/2017 - Bug 32830 - Refresh menu/toolbar after closing Conversion window to accurately display modules that were converted
	// Refreshing company info
	si->ResetModuleLicenses(CGBLSystemInformation::IsUsable);
	CCompanyLogin::RefreshCompanyInfo(si, NULL, NULL, true);

	// Send message to mainframe to update menu/toolbar
	CGBLForm::SendMessageToMainFrame(WM_UPDATE_MODULE, 0);
}

// -------------------------------------------------------------
void CMainFrame::OnCompanyMagneticmediaconstraints(void)
{
	theApp.m_pFormManager->ShowFormExclusive(IDD_CMS_MAGNETIC_MEDIA_CONSTANTS);
}

// -------------------------------------------------------------
void CMainFrame::OnCompanyInternalControlAlerts(void)
{
	CDLG_CTInternalControlAlertsSetup dlg;
	dlg.DoModal();
}

//1-7153, PGP(08/26/2003), Added checks for return value from showCompanyLogin.
// -------------------------------------------------------------
void CMainFrame::OnCompanySetupSelectCompany(void)
{
	//sb - 1-11355
	if (theApp.m_pFormManager->GetFormCount() > 0)
	{
		CGBLForm::CMSMessageBox(this, IDS_CT_STATUS_CLOSE_ALL, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		bool bResult = CGBLFormWrap::ShowCompanyLogin();
		// InTimeTec: 04.17.2013.HS - Custom Menus
		// Update Custom Menus when user change company login
		if (bResult)
		{
			UpdateCustomMenu();

			Menu[IDS_ORGANIZATION_OPTION]->RemoveMenu(IDS_DENALIBI_MENU, MF_BYCOMMAND);
			if (ShouldDenaliBISetupMenuShow()) Menu[IDS_ORGANIZATION_OPTION]->InsertMenu(m_nDenaliBIMenuPos, MF_STRING | MF_BYPOSITION, IDS_DENALIBI_MENU, RESSTRING(IDS_DENALIBI_MENU));

			Menu[IDS_ORGANIZATION_OPTION]->RemoveMenu(IDS_NEONCRM_SETUP, MF_BYCOMMAND);
			if (ShouldNeonCRMSetupMenuShow()) Menu[IDS_ORGANIZATION_OPTION]->InsertMenu(m_nNeonCRMMenuPos, MF_STRING | MF_BYPOSITION, IDS_NEONCRM_SETUP, RESSTRING(IDS_NEONCRMSETUP_MENU));

			Menu[IDS_ORGANIZATION_OPTION]->RemoveMenu(IDS_DONATELYSETUP_MENU, MF_BYCOMMAND);
			if (ShouldDonatelySetupMenuShow()) Menu[IDS_ORGANIZATION_OPTION]->InsertMenu(m_nDonatelyMenuPos, MF_STRING | MF_BYPOSITION, IDS_DONATELYSETUP_MENU, RESSTRING(IDS_DONATELYSETUP_MENU));

			//RCG - 08/10/2018 - PBI 42384 - Removed the Convert Payroll To Denali menu item in 64-bit Denali
#if defined _M_IX86
			//Do not remove the Convert Payroll To Denali menu item in 32-bit Denali
#elif defined _M_X64
			Menu[IDS_ORGANIZATION_OPTION]->RemoveMenu(IDS_CONVERT_PAYROLL_FROM_CMSPRO, MF_BYCOMMAND);
#endif
		}
		// InTimeTec: End
		if (!bResult && !si->SkipLogin && si->CompanySettings.ID.IsEmpty())
		{
			CGBLForm::CMSMessageBox(this, IDS_FAILED_LOGIN, IDS_INVALID_LOGIN, MB_OK | MB_ICONERROR);
			PostMessage(WM_CLOSE);
		}
	}
}

// -------------------------------------------------------------
// begbert 05-27-2020 PBI 55193 : This tries to close all the other modules and then send you back to the company login form.
//   If you then cancel out of the company login form it closes the program.
void CMainFrame::OnCompanyCloseLogoff()
{
	if (theApp.m_pFormManager->GetFormCount() > 0)
	{
		CGBLForm::CMSMessageBox(this, NULL, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONINFORMATION, L"All forms must be closed before logging out.");
		return;
	}

	CCloseProgramAndLogOff logoffform(ModuleTracker.get(), this);
	if (logoffform.DoModal() == IDOK)
	{
		si->CompanySettings.ID = si->UserInformation.ID = si->UserInformation.Password = L"";
		OnCompanySetupSelectCompany();
	}
}

// -------------------------------------------------------------
void CMainFrame::OnSecurityChangepassword(void)
{
	// 1-8923 BK 11/18/03 set parameter to true so title will read Change Password
	CCFV_GBLChangePassword clsChangePassword;
	clsChangePassword.DoModal();
}

// BK 11/04/05 Reset Password
// -------------------------------------------------------------
void CMainFrame::OnSecurityResetPassword(void)
{
	CDLG_CTResetPassword* const	pDlg = new CDLG_CTResetPassword();
	pDlg->Create(CDLG_CTResetPassword::IDD, NULL);
	pDlg->SetParent(this);
}

// BK 11/04/05 Activity Log
// -------------------------------------------------------------
void CMainFrame::OnSecurityActivityLog(void)
{
	//	theApp.m_pFormManager->ShowForm(IDD_CTF_ACTIVITYLOG);
}
// -------------------------------------------------------------
// Options > Module Preferences
void CMainFrame::OnOptionsModulePrefs(void)
{
	if (WindowCount == 0)
		theApp.m_pFormManager->ShowFormExclusive(IDD_GBL_MODPREFS_STD);
	else
		CGBLForm::CMSMessageBox(this, IDS_ERR_MSG_MODULEPREF, IDS_COUGAR_MOUNTAIN, MB_ICONSTOP | MB_OK);
}

// -------------------------------------------------------------
void CMainFrame::OnOptionsCurrencies(void)
{
	OnOptionsCurrencies(NULL);
}

// -------------------------------------------------------------
void CMainFrame::OnOptionsCurrencies(CXMLParams* pXML, UINT uiMode)
{
	theApp.m_pFormManager->ShowForm(IDD_CTF_CFV_CURRENCYCODE);
}

// -------------------------------------------------------------
void CMainFrame::OnOptionsDisplayfileinformaion(void)
{
	CGBLFormWrap::ShowFileInformation();
}
// -------------------------------------------------------------
void CMainFrame::OnOptionsProcessPayroll(void)
{
	//TODO: remove from form manager
	//theApp.m_pFormManager->ShowForm(IDD_DLG_IMPORT_PAYROLL);
	if (theApp.m_pFormManager->AreAnyFormsOpen())
	{
		CGBLForm::CMSMessageBox(this, IDS_ERR_MSG_CLOSE_ALL_OPEN_FORMS_FOR_POSTING, IDS_COUGAR_MOUNTAIN, MB_ICONSTOP | MB_OK);
		return;
	}

	CDLG_CTImportPayroll dlg;
	dlg.DoModal();
}

// -------------------------------------------------------------
void CMainFrame::OnOptionsExchangerates(void)
{
	theApp.m_pFormManager->ShowFormExclusive(IDD_EXCHANGE_RATES);
}

//RCG - 07/13/2021 - PBI 57710 - Display window when menu item is selected
// -------------------------------------------------------------
void CMainFrame::OnOptionsChangeURLtoArcharinaApps(void)
{
	//AR - 09/28/2021 - PBI 57710 - Create window with options to set Archarina URL
	CDLG_ChangeArcharinaAppsURL dlg;
	dlg.DoModal();
}

// -------------------------------------------------------------
BOOL CMainFrame::BackupcompanyBackup(void)
{
	// DWP - 4866 - 7/29/13 - If the backup does not start then we want to return FALSE here
	BOOL Results = TRUE;
	if (si->CommandLineSettings.CT_Backup)
	{
		if (si->CommandLineSettings.CT_Quiet)
		{
			//Make the default file name to backup to.
			CString szCompany = si->CompanySettings.ID;
			CString szFileName = szCompany + _T("DATA.CAB");

			SetDlgItemText(IDC_CTF_TXT_FILE_NAME, szFileName);

			//Make the default dir to backup to.
			//default directory is in the format: APPDIR\COMPANYNAME\BACKUP\yyyymmddhhmm"(ref:Dave Parvin)

			CString szBackUpPath = _T("");

			if (!si->CommandLineSettings.CT_BackUpPath.IsEmpty())
				szBackUpPath = si->CommandLineSettings.CT_BackUpPath;

			if (szBackUpPath.IsEmpty())
			{
				szBackUpPath = si->ApplicationPaths.szApplicationPath;
				szBackUpPath += szCompany;
				szBackUpPath += _T("\\BACKUP\\");
				//get date
				CTime clsCurrentTime = CTime::GetCurrentTime();
				CString szBackupDirName = clsCurrentTime.Format(_T("%Y%m%d%H%M"));//original
				szBackUpPath += szBackupDirName + _T("\\");
			}

			CCFV_CTBackupCompanyInfo frm;
			Results = frm.StartSilentBackup(szBackUpPath, szFileName);
		}
		else
		{
			theApp.m_pFormManager->ShowFormExclusive(IDD_BACKUP_COMPANY_INFO);
		}
	}
	else
	{
		theApp.m_pFormManager->ShowFormExclusive(IDD_BACKUP_COMPANY_INFO);
	}
	return Results;
}

//----------------------------------------------------------------------------
void CMainFrame::OnBackupcompanyBackup(void)
{
	// DWP - 4866 - 7/29/13 - If the backup does not start then we want to return FALSE here
	BackupcompanyBackup();
}

//----------------------------------------------------------------------------
void CMainFrame::OnBackupcompanyRestore(void)
{
	theApp.m_pFormManager->ShowFormExclusive(IDD_CTF_RESTORE_COMPANY_INFO);
}

//----------------------------------------------------------------------------
void CMainFrame::OnSecurityAdd(void)
{
	if (theApp.m_pFormManager->IsFormOpened(IDD_EDIT_USER_OR_GROUP, CCTDllFormManager::FORM_USERGROUP_USERS))
		theApp.m_pFormManager->ShowFormExclusive(IDD_EDIT_USER_OR_GROUP, NULL, CCTDllFormManager::FORM_USERGROUP_USERS);
	else
	{
		CXMLParams clsXMLParams;
		clsXMLParams.MakeParam(_T("USER"), (int)1);//true for user.
		theApp.m_pFormManager->ShowFormExclusive(IDD_EDIT_USER_OR_GROUP, (LPARAM)&clsXMLParams, CCTDllFormManager::FORM_USERGROUP_USERS);
	}
}

//----------------------------------------------------------------------------
void CMainFrame::OnSecurityGroup(void)
{
	if (theApp.m_pFormManager->IsFormOpened(IDD_EDIT_USER_OR_GROUP, CCTDllFormManager::FORM_USERGROUP_GROUPS))
		theApp.m_pFormManager->ShowFormExclusive(IDD_EDIT_USER_OR_GROUP, NULL, CCTDllFormManager::FORM_USERGROUP_GROUPS);
	else
	{
		CXMLParams clsXMLParams;
		clsXMLParams.MakeParam(_T("USER"), (int)0);//false for group.
		theApp.m_pFormManager->ShowFormExclusive(IDD_EDIT_USER_OR_GROUP, (LPARAM)&clsXMLParams, CCTDllFormManager::FORM_USERGROUP_GROUPS);
	}
}

//Bring up copy users from company form.
//----------------------------------------------------------------------------
void CMainFrame::OnSecurityCopyUsers(void)
{
	if (theApp.m_pFormManager->IsFormOpened(IDD_COPY_USERS_OR_GROUPS_FROM_COMPANY, CCTDllFormManager::FORM_COPY_USERS))
		theApp.m_pFormManager->ShowFormExclusive(IDD_COPY_USERS_OR_GROUPS_FROM_COMPANY, NULL, CCTDllFormManager::FORM_COPY_USERS);
	else
	{
		CXMLParams clsXMLParams;
		clsXMLParams.MakeParam(_T("GROUP"), (int)0);//false for user.
		theApp.m_pFormManager->ShowFormExclusive(IDD_COPY_USERS_OR_GROUPS_FROM_COMPANY, (LPARAM)&clsXMLParams, CCTDllFormManager::FORM_COPY_USERS);
	}
}

//Bring up copy groups from company form.
//----------------------------------------------------------------------------
void CMainFrame::OnSecurityCopyGroups(void)
{
	if (theApp.m_pFormManager->IsFormOpened(IDD_COPY_USERS_OR_GROUPS_FROM_COMPANY, CCTDllFormManager::FORM_COPY_GROUPS))
		theApp.m_pFormManager->ShowFormExclusive(IDD_COPY_USERS_OR_GROUPS_FROM_COMPANY, NULL, CCTDllFormManager::FORM_COPY_GROUPS);
	else
	{
		CXMLParams clsXMLParams;
		clsXMLParams.MakeParam(_T("GROUP"), (int)1);//true for group
		theApp.m_pFormManager->ShowFormExclusive(IDD_COPY_USERS_OR_GROUPS_FROM_COMPANY, (LPARAM)&clsXMLParams, CCTDllFormManager::FORM_COPY_GROUPS);
	}
}

//----------------------------------------------------------------------------
// BK 5/23/08 implemented Opening Denali Modules Routine design change spec	- open license manager
void CMainFrame::OnOpenLicenseManager(void)
{
	if (CMS::GetSI()->ApplicationSettings.IsCT() &&
		CGBLForm::LaunchExecutable(_T("NetReg.exe")))
	{
		_bstr_t bszLicenseInfo;
		if (CGBLNetDll::IsLicenseServerRunning(bszLicenseInfo))
		{
			CXMLDocument docReturnXML;
			docReturnXML.LoadXML(bszLicenseInfo);
			// still invalid key for current module - display message and exit app
			if (!CMS::GetSI()->GetPackageInstalled(&docReturnXML))
			{
				// msg 7: Either you have not entered your licensing key for this module or the license has expired. To update your module licensing, open the Controller module and follow the on-screen instructions. To purchase additional licenses, contact your Cougar Mountain Account Executive at (800) 390-7053.
				CGBLForm::CMSMessageBox(
					CMS::GetSI()->ApplicationMainFrame,
					IDS_NO_LICENSEKEYS_MSG,
					IDS_NO_LICENSEKEYS,
					MB_OK | MB_ICONERROR);
			}
			else
			{
				CString szProductType = CMS::GetSI()->ModulesInstalled.PackagesInstalled.FUND.InCompany ? _T("Fund") : _T("Act");
				bszLicenseInfo = CGBLNetDll::GetLicense(
					_bstr_t(CMS::GetSI()->ApplicationSettings.TenantID),
					_bstr_t(CMS::GetSI()->ApplicationSettings.MachineGUID),
					_bstr_t(CMS::GetSI()->ApplicationSettings.AppGUID),
					_bstr_t(CMS::GetSI()->ApplicationSettings.GetLicenseModule(CMS::GetSI()->ApplicationSettings.ModuleID)),
					_T("CMS_32Bit"),
					_bstr_t(szProductType),
					CAV_LICENSEVERSION, false);
				CString szReturn((LPCWSTR)bszLicenseInfo);
				CXMLDocument docReturnXML;
				if (szReturn != EMPTY_STRING)
				{
					docReturnXML.LoadXML(bszLicenseInfo);

					// see what is installed on the system
					if (!CMS::GetSI()->GetPackageInstalled(&docReturnXML))
					{
						// If the module is not licensed then tell the user
						//  message 7: Either you have not entered your licensing key for this module or the license has expired. To update your module licensing, open the Controller module and follow the on-screen instructions. To purchase additional licenses, contact your Cougar Mountain Account Executive at (800) 390-7053.
						CGBLForm::CMSMessageBox(CMS::GetSI()->ApplicationMainFrame, IDS_NO_LICENSEKEYS_MSG, IDS_NO_LICENSEKEYS, MB_OK | MB_ICONERROR);
					}
					// If the user has exceeded the maximum number of users, then tell them and return.
					if (CMS::GetSI()->ApplicationSettings.bMaxReached)
					{
						// InTimeTec: 12/12/2013: HS : All_Intuitive Error message for Users Exceeded
						// Added new message to let the user know that maximum number of licensed users exceeded
						//  message: "The number of concurrent users has exceeded the maximum number licensed."
						CGBLForm::CMSMessageBox(CMS::GetSI()->ApplicationMainFrame, IDS_MAXIMUM_USERS_EXCEEDED_MESSAGE, IDS_MAXIMUM_USERS_EXCEEDED_TITLE, MB_OK | MB_ICONERROR);
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
BOOL CMainFrame::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// message sent by modules to indicate that the module (tried)
	// to integrate to another module and possibly a new integration
	// report was created. Call function to update Integration report
	// menu items.
	if (message == UpdateIntegration)
	{
		CCMSCTReportsMenu::RefreshMenu();		// EK - 05/05/04 - Refresh the Reports menu
		MenuSecurityDisableItems();
	}
	return __super::OnWndMsg(message, wParam, lParam, pResult);
}

//----------------------------------------------------------------------------
BOOL CMainFrame::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* const plMessage = (NMHDR*)lParam;
	NMHDR* const pwMessage = (NMHDR*)wParam;

	switch (plMessage->code)
	{
	case WM_UPDATE_MODULE:
		Menu[IDS_ORGANIZATION_OPTION]->RemoveMenu(IDS_DENALIBI_MENU, MF_BYCOMMAND);
		if (ShouldDenaliBISetupMenuShow()) Menu[IDS_ORGANIZATION_OPTION]->InsertMenu(m_nDenaliBIMenuPos, MF_STRING | MF_BYPOSITION, IDS_DENALIBI_MENU, RESSTRING(IDS_DENALIBI_MENU));

		Menu[IDS_ORGANIZATION_OPTION]->RemoveMenu(IDS_NEONCRM_SETUP, MF_BYCOMMAND);
		if (ShouldNeonCRMSetupMenuShow()) Menu[IDS_ORGANIZATION_OPTION]->InsertMenu(m_nNeonCRMMenuPos, MF_STRING | MF_BYPOSITION, IDS_NEONCRM_SETUP, RESSTRING(IDS_NEONCRMSETUP_MENU));

		Menu[IDS_ORGANIZATION_OPTION]->RemoveMenu(IDS_DONATELYSETUP_MENU, MF_BYCOMMAND);
		if (ShouldDonatelySetupMenuShow()) Menu[IDS_ORGANIZATION_OPTION]->InsertMenu(m_nDonatelyMenuPos, MF_STRING | MF_BYPOSITION, IDS_DONATELYSETUP_MENU, RESSTRING(IDS_DONATELYSETUP_MENU));

		//RCG - 08/10/2018 - PBI 42384 - Removed the Convert Payroll To Denali menu item in 64-bit Denali
#if defined _M_IX86
		//Do not remove the Convert Payroll To Denali menu item in 32-bit Denali
#elif defined _M_X64
		Menu[IDS_ORGANIZATION_OPTION]->RemoveMenu(IDS_CONVERT_PAYROLL_FROM_CMSPRO, MF_BYCOMMAND);
#endif
		break;
	case WM_VIEWMODIFY_COMPANY:
	case WM_VIEWMODIFY_FUND:
	{
		// BK 1/14/11 set company code and if a fund type is selected from CompanyLogin list
		CString* const pszCompanyCode = (CString*)pwMessage;
		OnViewModifyCompany(*pszCompanyCode, (plMessage->code == WM_VIEWMODIFY_FUND) ? true : false);
	}
	break;
	case WM_SETUPSELECT_COMPANY:
		OnCompanySetupSelectCompany();
		return TRUE;
		break;
	case WN_RESET_SECURITY_SETTINGS:
		MenuSecurityDisableItems();
		break;
	case WN_RESET_REPORT_MENU:
		//DAJ 1-27-5 per error 1-15591 the report menu is to be hidden for the first release
		CCMSCTReportsMenu::RefreshMenu();		// EK - 04/12/04 - Refresh the Reports menu
		break;
		// BK 4/30/08 implemented Opening Denali Modules Routine spec
	case WM_ADDNEW_COMPANY_NOCOMPANIES:
		m_bNoInstalledCompanies = true;
		m_bProcessingAddCompanies = false;
		PostMessage(WM_USER_MESSAGE_1, 0, 0);// OnAddNewCompany();
		break;
	case WM_ADDNEW_COMPANY:
		m_bNoInstalledCompanies = false;
		m_bProcessingAddCompanies = false;
		PostMessage(WM_USER_MESSAGE_1, 0, 0);// OnAddNewCompany();
		break;
		// BK 1/13/2011 this hasn't been used in years (contains list of business templates to automatically create chart of accounts)
		//case WM_COMPANY_DETAILS:
		//	OnCompanyDetails();
		//	break;
	case WM_CUSTOM_DETAILS:
		OnCustomDetails();
		break;
	case WM_ORGANIZATION_DETAILS:
		OnOrganizationDetails(true);
		break;
		// Dialog to choose fund or act company to create
	case WM_SELECT_FUND_OR_ACT:
		ShowDialogSelectFundAct();
		break;
	case WM_TIPS_CLOSE:
		/*if (m_pdlgTip != NULL)
		delete m_pdlgTip;*/
		break;
	case WM_VIEWMODIFY:		// Lookup opening form to view/modify or add on the fly
	case WM_ADDONFLY:
	{
		// make pointer to global structure (which contains elements FormID, XMLParams) and get values
		CGBLSystemInformation::stuLookupSelected* const pLookupSelected = (CGBLSystemInformation::stuLookupSelected*)wParam;
		// create local pointer for the xml params so copy of xml param address can be used instead of global struct
		CXMLParams* const m_pXMLParam = &pLookupSelected->XMLParams;

		switch (pLookupSelected->FormID)
		{
		case IDD_CURRENCIES:
			OnOptionsCurrencies(m_pXMLParam, plMessage->code);
			break;
		}
		break;
	}
	case WM_FISCALCALENDAR:
		OnCompanyFiscalcalendar();
		break;
	case WM_INSTALLMODULES:
		// 1-31712 BK don't show message to backup files when creating a new company
		//OnCompanyInstallmodules();
		InstallModules(true, false);	//RCG - 03/30/2017 - Bug 32830 - Added second parameter for Convert Payroll into Denali
		break;
	case WM_USER_MESSAGE_2:
		PostMessage(WM_USER_MESSAGE_3, 0, 0);
		break;
		// BK 5/23/08 implemented Opening Denali Modules Routine design change spec	- when logged in as SUP show license manager menu item
	case WM_SHOW_LICENSE_MGR_MENU:
		ShowLicenseManagerMenuItem();
		break;
	case WM_UPDATE_CUSTOM_MENU:
		UpdateCustomMenu();
		break;
	case WM_SHOW_CONVERT_PAYROLL_FROM_CMSPRO:	// begbert 02-10-2014 PBI 138 : Added this
		ShowConvertPayrollFromCMSProMenuItem();
	}

	return __super::OnNotify(wParam, lParam, pResult);
}

//----------------------------------------------------------------------------
LRESULT CMainFrame::OnLaunchDelayedCompanies(WPARAM wParam, LPARAM lParam)
{
	OnCompanySetupSelectCompany();
	return 0;
}

//----------------------------------------------------------------------------
// View/Modify Organization
void CMainFrame::OnViewModifyCompany(const CString& szCompanyCode, bool bIsFund)
{
	CPPG_CTOrganizationalDetails::SetCompanyCode(szCompanyCode, bIsFund);
	theApp.m_pFormManager->ShowForm(IDD_ADD_NEW_COMPANY);
}

//----------------------------------------------------------------------------
// display Company Details dialog
void CMainFrame::OnCompanyDetails(void)
{
	// BK 01/13/11 file hasn't been used for years (contains list of business templates to automatically create chart of accounts)
//	theApp.m_pFormManager->ShowForm(IDD_NEW_COMPANY_DETAILS);
}

//----------------------------------------------------------------------------
// act - display Company Details dialog
void CMainFrame::OnCustomDetails(void)
{
	theApp.m_pFormManager->ShowForm(IDD_NEW_CUSTOM_DETAILS);
}

//----------------------------------------------------------------------------
//Since new company dialog is modal, this message is Posted from
//OnNotify so that normal processing would continue in OnNotify.
LRESULT CMainFrame::DelayAddNewCompany(WPARAM wParam, LPARAM lParam)
{
	OnAddNewCompany();
	return 0;
}

//----------------------------------------------------------------------------
// display Add New Company dialog
// BK 4/30/08 implemented Opening Denali Modules Routine spec
void CMainFrame::OnAddNewCompany(void)
{
	// if already in here don't come back until processing is complete
	if (m_bProcessingAddCompanies) return;

	INT_PTR nAns = IDNEW;
	if (m_bNoInstalledCompanies)
	{
		// using global message box class to show error message with unique buttons
		// msg 8: No companies exist to open. If you want to create a new company or convert a company from the CMS Professional product line, select New. If you want to install the demonstration company, select Create Demonstration Company.
		CCMSMessageBox dlgMsg(RESSTRING(IDS_NO_COMPANIES_FOUND_MSG), IDS_NO_COMPANIES_FOUND, IDS_MASTER_NEW, IDS_CREAT_COMPANY_BUTTON);
		nAns = dlgMsg.DoModal();
	}
	switch (nAns)
	{
	case IDCANCEL:
		// msg 9: You must create at least one company in order to use Denali. Because you elected to cancel creating a company at this time, Denali will close. You will have to re-enter the program to set up a company.
		CGBLForm::CMSMessageBox(this, IDS_NO_COMPANIES_INSTALLED_MSG, IDS_NO_COMPANIES_INSTALLED, MB_OK | MB_ICONSTOP, EMPTY_STRING, EMPTY_STRING, 0, false);
		__super::OnClose();
		break;

	case IDNEW:
	{
		// 1-35472 BK 3/18/11 fund modification to create new company when there are no companies and only fund or act is licensed
		bool bIsFund = false;
		m_bProcessingAddCompanies = true;

		// 1-35601 BK 4/20/11 this function is used to create an ACT company unless there are no companies then it can create ACT or FUND
		// if only fund is licensed create a new FUND company
		if (si->ModulesInstalled.PackagesInstalled.FUND.Licensed && !si->ModulesInstalled.PackagesInstalled.ACT.Licensed)
			bIsFund = true;

		OnOrganizationDetails(bIsFund);
		break;
	}
	case IDDEMO:
	{
		m_bProcessingAddCompanies = true;

		ShowCreateDB();
		// 1-32154 BK 3/11/09 refresh keys from license server
		_bstr_t bszLicenseInfo = EMPTY_STRING;
		CGBLNetDll::SetValidLicenseModuleKeys(bszLicenseInfo);
		// login to selected company
		if (!CGBLFormWrap::PerformLoginRoutineAtStartup())
			__super::OnClose();
		break;
	}
	};
}

//----------------------------------------------------------------------------
// fund - display Organization Details dialog
void CMainFrame::OnOrganizationDetails(bool bIsFund)
{
	CDLG_CTCompanyCreation dlgCompanyCreation;
	CDLG_CT16to32 cls16to32;

	CCFV_CTNewCustomDetails::sm_QBimportdata.SetIsFund(bIsFund);
	CCFV_CTNewCustomDetails::sm_QBimportdata.Reset();	// Clear the QB import data - it will either get populated or it won't.

	if (dlgCompanyCreation.DoModal() != IDOK) return;

	if (dlgCompanyCreation.m_bNewCompany)
	{
		CPPG_CTOrganizationalDetails::SetCompanyCode(EMPTY_STRING, bIsFund);
		theApp.m_pFormManager->ShowForm(IDD_NEW_CUSTOM_DETAILS);
	}

	if (dlgCompanyCreation.m_bCMSPro)
	{
		cls16to32.SetIsThisNewFund(bIsFund);
		cls16to32.DoModal();
		return;
	}

	if (dlgCompanyCreation.m_bQuickBooks)
	{
		//CCFV_CTNewCustomDetails::sm_QBimportdata.Reset(bIsFund);
		CDLG_CTImportFilesFromQB dlgQBFiles(CCFV_CTNewCustomDetails::sm_QBimportdata);
		if (dlgQBFiles.DoModal() != IDOK) return;

		CDLG_CTImportTranFilesFromQB dlgQBTranFiles(CCFV_CTNewCustomDetails::sm_QBimportdata);
		if (dlgQBTranFiles.DoModal() != IDOK) return;

		CPPG_CTOrganizationalDetails::SetCompanyCode(EMPTY_STRING, bIsFund);
		theApp.m_pFormManager->ShowForm(IDD_NEW_CUSTOM_DETAILS);
	}

	//CString szTitle = RESSTRING(IDS_LBL_WIN16);
	//// message 10: Attention <TEXT1> users. Do you want to convert your existing CMS Professional company data to Denali?
	//if (IDYES == CGBLForm::CMSMessageBox(this, IDS_MSG_16_TO_32_CONV_REPLACETEXT, 0, MB_YESNO | MB_ICONINFORMATION, RESSTRING(IDS_LBL_WIN16), szTitle) )
	//{
	//	cls16to32.SetIsThisNewFund(bIsFund);
	//	cls16to32.DoModal();
	//}
	//else
	//{
	//	CPPG_CTOrganizationalDetails::SetCompanyCode(EMPTY_STRING, bIsFund);
	//	theApp.m_pFormManager->ShowForm(IDD_NEW_CUSTOM_DETAILS);
	//}
}

//------------------------------------------------------------------------------
// display dialog for user to select Non-Fund Accounting or Fund Accounting (act or fund)
void CMainFrame::ShowDialogSelectFundAct(void)
{
	CDLG_CTOrgCompany clsSelectFundAct;
	clsSelectFundAct.DoModal();
}

//------------------------------------------------------------------------------
void CMainFrame::CreateModuleToolBar(void)
{
	__super::ToolBarAddToButtonArray(IDS_MNU_SETUP_SELECT_ORG, ToolBarImages.GetImageCount(), si->Drawing.Icons.SelectCompany());
	__super::ToolBarAddToButtonArray(IDS_CTRL_ADD_CHNG_USER, ToolBarImages.GetImageCount(), si->Drawing.Icons.AddChangeUser());

	StatusMessages.AddMessage(IDS_MNU_SETUP_SELECT_ORG, IDS_MNU_SETUP_SELECT_ORG);
	StatusMessages.AddMessage(IDS_CTRL_ADD_CHNG_USER, IDS_CTRL_ADD_CHNG_USER);
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableChangeUsersButton(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CanCTAccess[ChangeUsersButton] ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableSetupSelectCompanyButton(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CanCTAccess[SetupSelectCompanyButton] ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableSetupWizardButton(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CanCTAccess[SetupWizardButton] ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
// 1-33827 BK 12/1/09 for CR 1-33454 added user rights to report menu items
//void CMainFrame::OnEnableReportsMenu(CCmdUI *pCmdUI)
//{
//	pCmdUI->Enable(CMS::GetSI()->UserRight.m_stuControllerNode.Reports.bTotal ? TRUE : FALSE);
//}

//------------------------------------------------------------------------------
// 1-33328 BK 9/23/09 enable/disable conversion menu items depending on whether report exists
void CMainFrame::OnEnablePreConversionChecklist(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CCMSCTReportsMenu::GetDoesPreConversionChecklistExist());
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableCodesValuesAddedReport(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CCMSCTReportsMenu::GetDoesCodesValuesAddedReportExist());
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableIntegrationStatusReport(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CCMSCTReportsMenu::GetDoesIntegrationStatusReportExist());
}

//------------------------------------------------------------------------------
// 1-33827 BK 12/1/09 for CR 1-33454 added user rights to report menu items
void CMainFrame::OnEnableRemoveIntegrationReports(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CMS::GetSI()->UserRight.m_stuControllerNode.Reports.Integration.bRemoveIntegReports ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
// 1-33827 BK 12/1/09 for CR 1-33454 added user rights to report menu items
void CMainFrame::OnEnableRemoveConversionReports(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CMS::GetSI()->UserRight.m_stuControllerNode.Reports.Conversion.bRemoveConverionReport ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
void CMainFrame::OnReportRemoveIntegrationChecklist(void)
{
	CCMSCTReportsMenu::RemoveReportsIntegrationChecklist();
}

//------------------------------------------------------------------------------
void CMainFrame::OnReportIntegrationChecklistDate1(void)
{
	CCMSCTReportsMenu::RunReportsIntegrationChecklist(CCMSCTReportsMenu::eINTEGRATION_CHKLST_REPORT_1);
}

//------------------------------------------------------------------------------
void CMainFrame::OnReportIntegrationChecklistDate2(void)
{
	CCMSCTReportsMenu::RunReportsIntegrationChecklist(CCMSCTReportsMenu::eINTEGRATION_CHKLST_REPORT_2);
}

//------------------------------------------------------------------------------
void CMainFrame::OnReportIntegrationChecklistDate3(void)
{
	CCMSCTReportsMenu::RunReportsIntegrationChecklist(CCMSCTReportsMenu::eINTEGRATION_CHKLST_REPORT_3);
}

//------------------------------------------------------------------------------
void CMainFrame::OnReportIntegrationChecklistDate4(void)
{
	CCMSCTReportsMenu::RunReportsIntegrationChecklist(CCMSCTReportsMenu::eINTEGRATION_CHKLST_REPORT_4);
}

//------------------------------------------------------------------------------
void CMainFrame::OnReportIntegrationChecklistDate5(void)
{
	CCMSCTReportsMenu::RunReportsIntegrationChecklist(CCMSCTReportsMenu::eINTEGRATION_CHKLST_REPORT_5);
}

//------------------------------------------------------------------------------
// 1-33328 BK 9/23/09 call the only report for each menu item (see CR 1-33386) replacing dated lists below
void CMainFrame::OnReportPreConversionChecklist(void)
{
	CCMSCTReportsMenu::RunReportsConversionChecklist();
}

//------------------------------------------------------------------------------
void CMainFrame::OnReportCodesValuesAdded(void)
{
	CCMSCTReportsMenu::RunReportsConversionCodesAdded();
}

//------------------------------------------------------------------------------
void CMainFrame::OnReportIntegrationStatus(void)
{
	CCMSCTReportsMenu::RunReportsConversionIntegrationStatus();
}

//------------------------------------------------------------------------------
void CMainFrame::OnRemoveConversionReports(void)
{
	CCMSCTReportsMenu::RemoveConversionReports();
}

//------------------------------------------------------------------------------
// PBI 22186 BK 8/19/14 Added red flag reports, enable if module is installed and Red Flag Reports submenu is enabled
void CMainFrame::OnEnableReportAPControlReport(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CMS::GetSI()->ModulesInstalled.AccountsPayable.IsUsable && si->UserRight.m_stuAccountsPayableNode.Reports.bControl);
}

void CMainFrame::OnEnableReportAPGLExpenseDistributionReport(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CMS::GetSI()->ModulesInstalled.AccountsPayable.IsUsable && si->UserRight.m_stuAccountsPayableNode.Reports.bGLExpenseDistribution);
}

void CMainFrame::OnEnableReportARControlReport(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CMS::GetSI()->ModulesInstalled.AccountsReceivable.IsUsable && si->UserRight.m_stuAccountsReceivableNode.Reports.bControl);
}

void CMainFrame::OnEnableReportBRMissingCheckReport(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CMS::GetSI()->ModulesInstalled.BankReconciliation.IsUsable && si->UserRight.m_stuBankReconciliationNode.Reports.bMissingCheck);
}

void CMainFrame::OnEnableReportGLReprintPostingReport(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CMS::GetSI()->ModulesInstalled.GeneralLedger.IsUsable && si->UserRight.m_stuGeneralLedgerNode.Report.bReprintPosting);
}

void CMainFrame::OnEnableReportINControlReport(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CMS::GetSI()->ModulesInstalled.Inventory.IsUsable && si->UserRight.m_stuInventoryNode.Reports.bControl);
}

void CMainFrame::OnEnableReportINMovementReport(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CMS::GetSI()->ModulesInstalled.Inventory.IsUsable && si->UserRight.m_stuInventoryNode.Reports.bMovement);
}

void CMainFrame::OnEnableReportPOOpenPurchaseOrderReport(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CMS::GetSI()->ModulesInstalled.PurchaseOrder.IsUsable && si->UserRight.m_stuPurchaseOrderNode.Reports.bOpenPurchaseOrderReport);
}

void CMainFrame::OnEnableReportSAPriceVarianceReport(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CMS::GetSI()->ModulesInstalled.SalesEntry.IsUsable && si->UserRight.m_stuSalesMaintenanceNode.Reports.bPriceVariance);
}

void CMainFrame::OnEnableReportSAInvoiceTrackingReport(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CMS::GetSI()->ModulesInstalled.SalesEntry.IsUsable && si->UserRight.m_stuSalesMaintenanceNode.Reports.bInvoiceTracking);
}

//------------------------------------------------------------------------------
// PBI 22186 BK 8/21/14 Added red flag reports - AP Control Report
void CMainFrame::OnReportAPControlReport()
{
	theApp.m_pFormManager->ShowFormExclusive(IDD_APF_RFV_CONTROL_STD);
}

// PBI 22186 BK 8/21/14 Added red flag reports - AP GL Expense Distribution Report
void CMainFrame::OnReportAPGLExpenseDistributionReport()
{
	theApp.m_pFormManager->ShowFormExclusive(IDD_APF_RFV_GL_EXPENSE_DISTRIBUTION_STD);
}

// PBI 22186 BK 8/21/14 Added red flag reports - AR Control Report
void CMainFrame::OnReportARControlReport()
{
	theApp.m_pFormManager->ShowFormExclusive(IDD_ARF_RFV_CONTROL_STD);
}

// PBI 22186 BK 8/21/14 Added red flag reports - BR Missing Check Report
void CMainFrame::OnReportBRMissingCheckReport()
{
	theApp.m_pFormManager->ShowFormExclusive(IDD_RFV_MISSINGCHECKREPORT);
}

// PBI 22186 BK 8/21/14 Added red flag reports - GL Reprint Posting Report
void CMainFrame::OnReportGLReprintPostingReport()
{
	theApp.m_pFormManager->ShowFormExclusive(IDD_GLF_RFV_REPRINT_POSTING);
}

// PBI 22186 BK 8/21/14 Added red flag reports - IN Control Report
void CMainFrame::OnReportINControlReport()
{
	theApp.m_pFormManager->ShowFormExclusive(IDD_INRCONTROL);
}

// PBI 22186 BK 8/21/14 Added red flag reports - IN Movement Report
void CMainFrame::OnReportINMovementReport()
{
	theApp.m_pFormManager->ShowFormExclusive(IDD_INF_RMOVEMENT);
}

// PBI 22186 BK 8/21/14 Added red flag reports - PO Open Purchase Order Report
void CMainFrame::OnReportPOOpenPurchaseOrderReport()
{
	theApp.m_pFormManager->ShowFormExclusive(IDD_POF_RFV_POOPENPURCHASEORDERREPORT);
}

// PBI 22186 BK 8/21/14 Added red flag reports -
void CMainFrame::OnReportSAPriceVarianceReport()
{
	theApp.m_pFormManager->ShowFormExclusive(IDD_RPT_PRICEVARIANCE);
}

// PBI 22186 BK 8/21/14 Added red flag reports -
void CMainFrame::OnReportSAInvoiceTrackingReport()
{
	theApp.m_pFormManager->ShowFormExclusive(IDD_SA_RPT_INVOICETRACKING);
}

//------------------------------------------------------------------------------
void CMainFrame::MenuUpdateElectronicConstants(void)
{
	const int nItem = MenuData[IDS_CTRL_MAGNETIC_MEDIA_CONST].MenuIndex;	//MenuFindItem(&m_companyMenuItem, IDS_CT_MAGNETIC_MEDIA_CONSTANTS_ELIPSIS);
	// PBI 17783 BK 3/11/14 added new Merchant Warehouse setup form with a separator, so removed separator from here
	// 1-35038 BK 11/11/10 menu should be created/deleted based on modules AP or PR being installed not on user rights
	if (si->ModulesInstalled.AccountsPayable.IsUsable || si->ModulesInstalled.Payroll.IsUsable)
		//if (si->ModulesInstalled.AccountsPayable.IsUsable && si->UserRight.m_stuControllerNode.Company.bMagneticMediaConstants)
	{
		if (nItem == -1)
			AddMenu(Menu[IDS_ORGANIZATION_OPTION], IDS_CTRL_MAGNETIC_MEDIA_CONST, IDS_CT_MAGNETIC_MEDIA_CONSTANTS_ELIPSIS, IDS_STATUS_MAGNETIC_MEDIA_CONSTANTS);
	}
	else
	{
		if (nItem != -1)
			Menu[IDS_ORGANIZATION_OPTION]->DeleteMenu((UINT)nItem, MF_BYPOSITION);
	}
}

//------------------------------------------------------------------------------
void CMainFrame::MenuUpdateInternalControlAlerts(void)
{
	const int nItem = MenuData[IDS_CT_INTERNALCONTROLALERTSSETUP_MENU].MenuIndex;
	if (nItem == -1)
		AddMenu(Menu[IDS_ORGANIZATION_OPTION], IDS_CT_INTERNALCONTROLALERTSSETUP_MENU, IDS_CT_INTERNALCONTROLALERTSSETUP_MENU, IDS_CT_INTERNALCONTROLALERTSSETUP_MENU);
}

//------------------------------------------------------------------------------
// 1-33328/1-33521 BK 10/15/09 when Reports top menu receives focus check if there are any integration/conversion reports
void CMainFrame::MenuUpdateReports(void)
{
	CCMSCTReportsMenu::InitializeReportMenu();

	CMenu* const pReportsMenu = Menu[IDS_REPORTS];
	if (pReportsMenu)
	{
		CMenu* const pIntegration = Menu[IDS_CTRL_INTEGRATION];
		if (pIntegration)
		{
			int nPos = MenuData[IDS_CTRL_INTEGRATION].MenuIndex;
			MenuData[IDS_CTRL_INTEGRATION].ParentMenu->EnableMenuItem((UINT)nPos, MF_BYPOSITION | (CCMSCTReportsMenu::EnableIntegrationMenu() ? (UINT)MF_ENABLED : (UINT)MF_GRAYED));
			// 1-33827 BK 12/1/09 for CR 1-33454 added user rights to report menu items
			CMenu* const pIntegrationError = Menu[IDS_INTEGRATION_ERROR_RPT];
			if (pIntegrationError)
			{
				int nPos2 = MenuData[IDS_INTEGRATION_ERROR_RPT].MenuIndex;
				MenuData[IDS_INTEGRATION_ERROR_RPT].ParentMenu->EnableMenuItem((UINT)nPos2, MF_BYPOSITION | (CCMSCTReportsMenu::GetDoesIntegrationErrorReportExist() ? (UINT)MF_ENABLED : (UINT)MF_GRAYED));
			}
		}
		CMenu* const pConvert = Menu[IDS_CTRL_CONVERSION];
		if (pConvert)
		{
			int nPos = MenuData[IDS_CTRL_CONVERSION].MenuIndex;
			MenuData[IDS_CTRL_CONVERSION].ParentMenu->EnableMenuItem((UINT)nPos, MF_BYPOSITION | (CCMSCTReportsMenu::EnableConversionMenu() ? (UINT)MF_ENABLED : (UINT)MF_GRAYED));
		}
	}
}

//------------------------------------------------------------------------------
void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	if (Menu[IDS_ORGANIZATION_OPTION] == pPopupMenu)
	{
		// begbert 10-25-2018 PBI 47520 : Add the Yodlee setup form, if necessary.
		if (si->ApplicationSettings.YodleeUsable && MenuData[IDS_MENU_SETUPYODLEEINTEGRATION].MenuIndex == -1)
			AddMenu(Menu[IDS_ORGANIZATION_OPTION], IDS_MENU_SETUPYODLEEINTEGRATION, IDS_MENU_SETUPYODLEEINTEGRATION, IDS_MENU_SETUPYODLEEINTEGRATION_STATUSBAR);

		if (MenuData[IDS_MENU_SETUPPAYPALINTEGRATION].MenuIndex == -1)
			AddMenu(Menu[IDS_ORGANIZATION_OPTION], IDS_MENU_SETUPPAYPALINTEGRATION, IDS_MENU_SETUPPAYPALINTEGRATION, IDS_MENU_SETUPPAYPALINTEGRATION_STATUSBAR);

		MenuUpdateElectronicConstants();
		MenuUpdateInternalControlAlerts();
	}

	// 1-33328/1-33521 BK 10/15/09 when Reports top menu receives focus check if there are any integration/conversion reports
	if (Menu[IDS_REPORTS] == pPopupMenu)
		MenuUpdateReports();

	if (Menu[IDS_OPTIONS] == pPopupMenu)
	{
		//RCG - 07/13/2021 – PBI 57710 – Display menu item if license key is activated
		if (CMS::GetSI()->CustomModInstalled(CHANGE_URL_TO_ARCHARINA_APPS) && MenuData[IDS_MENU_CHANGE_ARCHARINA_APPS_URL].MenuIndex == -1)
		{
			AddSeparator(Menu[IDS_OPTIONS]);
			AddMenu(Menu[IDS_OPTIONS], IDS_MENU_CHANGE_ARCHARINA_APPS_URL, IDS_MENU_CHANGE_ARCHARINA_APPS_URL, IDS_STATUS_CHANGE_ARCHARINA_APPS_URL);
		}
	}

	__super::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableCompanyInstallmodules(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Company.bInstallModules ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
// begbert 02-10-2014 PBI 138 : Added this
void CMainFrame::OnEnableCompanyConvertPayrollFromCMSPro(CCmdUI* pCmdUI)
{
	//RCG - 08/10/2018 - PBI 42384 - Removed the Convert Payroll To Denali menu item in 64-bit Denali
#if defined _M_IX86
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Company.bInstallModules ? TRUE : FALSE);
#elif defined _M_X64
	//Do not enable the Convert Payroll To Denali menu item in 64-bit Denali
#endif
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableCompanyChangedate(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Company.bChangeDate ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableCompanyFiscalcalendar(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Company.bFiscalCalendar ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
// PBI 17783 BK 3/11/14 added new Merchant Warehouse setup form
void CMainFrame::OnEnableCompanyMerchantWareSetup(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Company.bMerchantWarehouse ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
//RS - 04/26/2024 - PBI 63797 - Added user security and menu item to Organization menu
void CMainFrame::OnEnableCompanyCardPointeSetup(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Company.bCloverConnect ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableCompanyNeonCRMSetup(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(ShouldNeonCRMSetupMenuShow());
}

//------------------------------------------------------------------------------
// begbert 11/19/2019 PBI 52317 : Added the Donately import
void CMainFrame::OnEnableCompanyDonatelySetup(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(ShouldDonatelySetupMenuShow());
}

//	PBI 47520	12/04/2018	mvt - Added security for Yodlee integration setup
// -------------------------------------------------------------
void CMainFrame::OnEnableCompanySetUpYodleeIntegration(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->ApplicationSettings.YodleeUsable &&
				   si->UserRight.m_stuControllerNode.Company.bYodleeIntegrationSetup ? TRUE : FALSE);
}

// -------------------------------------------------------------
void CMainFrame::OnEnableCompanySetUpPayPalIntegration(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Company.bPayPalIntegrationSetup ? TRUE : FALSE);
}

// PBI 46406	09/26/2018	mvt - Added security for Internal Control Alerts in Controller
//------------------------------------------------------------------------------
void CMainFrame::OnEnableCompanyInternalControlAlertsSetup(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Company.bInternalControlAlertsSetup ? TRUE : FALSE);
}

//	PBI 45155	06/06/2018	mvt - Added security for Denali BI Setup
//------------------------------------------------------------------------------
void CMainFrame::OnEnableCompanyDenaliBISetup(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Company.bDenaliBISetup ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableCompanyMagneticmediaconstraints(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Company.bMagneticMediaConstants ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableSecurityGroup(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Security.bAddChangeGroups && si->UserRight.m_stuModulesNode.bController ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableSecurityChangePassword(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Security.bChangePassword ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableSecurityResetPassword(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Security.bResetPassword ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableSecurityCopyUsers(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Security.bCopyUsersFromCompany ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableSecurityCopyGroups(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Security.bCopyGroupsFromCompany ? TRUE : FALSE);
}
//------------------------------------------------------------------------------
void CMainFrame::OnEnableModulePrefs(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	// InTimeTec: 10.8.2012.KP - Custom Reports
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Options.ModulePreference.bTotal ? TRUE : FALSE);
	// InTimeTec: End
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableOptionsCurrencies(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Options.bCurrencies && si->UserRight.m_stuModulesNode.bController && si->UserRight.m_stuControllerNode.Options.bTotal ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableOptionsExchangerates(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Options.bExchangeRates && si->UserRight.m_stuModulesNode.bController && si->UserRight.m_stuControllerNode.Options.bTotal ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableOptionsDisplayfileinformaion(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Options.bFileMaintenance && si->UserRight.m_stuModulesNode.bController && si->UserRight.m_stuControllerNode.Options.bTotal ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableOptionsProcessPayroll(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	// begbert 03-12-2010 Process Payroll Transactions : Set up security node for this menu
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Options.bProcessPayrollTransactions && si->UserRight.m_stuModulesNode.bController && si->UserRight.m_stuControllerNode.Options.bTotal ? TRUE : FALSE);
}

//RCG - 07/13/2021 - PBI 57710 - Enable menu item if license key is activated
//------------------------------------------------------------------------------
void CMainFrame::OnEnableOptionsChangeURLtoArcharinaApps(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(CMS::GetSI()->CustomModInstalled(CHANGE_URL_TO_ARCHARINA_APPS) ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableBackupcompanyBackup(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Options.BackupCompany.bBackup ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
void CMainFrame::OnEnableBackupcompanyRestore(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Options.BackupCompany.bRestore ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
// InTimeTec: 03.05.2013.HS - Copy Organization Utility
// Enable/Disable Copy Organization Utility menu based on user rights
void CMainFrame::OnEnableCopyOrganizationUtility(CCmdUI* pCmdUI)
{
	if (!pCmdUI->m_pMenu) return;
	pCmdUI->Enable(si->UserRight.m_stuControllerNode.Options.bCopyOrganizationUtility ? TRUE : FALSE);
}

// Launch Copy Organization Utility window
void CMainFrame::OnCopyOrganizationUtility(void)
{
	// Launch the Copy Organization Utility
	CGBLForm::LaunchModule(_T("CMSCopyOrganizationUtility.exe"));
}
// InTimeTec: End

//------------------------------------------------------------------------------
// BK 4/30/08 implemented Opening Denali Modules Routine spec
// this is modified code from CreateDB.cpp InitInstance()
// also added CreateDBDlg.cpp/h to CTForms and modified files in CTForms to work from inside the app
BOOL CMainFrame::ShowCreateDB()
{
	CGBLSystemInformation* pSI = CMS::GetSI();
	// begbert 04-19-2010 1-34418 : Load the license info into si.ModulesInstalled so
	//   that the restore can uninstall unlicensed modules from the restored companies.
	if (CGBLForm::IsLicenseServerRequiredForModule())
	{
		CString szProductType;
		// 1-35472 BK 3/21/11 fund modification to create demos check if fund or act is licensed
		if (pSI->ModulesInstalled.PackagesInstalled.ACT.Licensed)
			szProductType = _T("Act");
		if (pSI->ModulesInstalled.PackagesInstalled.FUND.Licensed)
			szProductType = _T("Fund");

		// BK 5/07/08 implemented Opening Denali Modules Routine design change spec
		_bstr_t szLicenseInfo = CGBLNetDll::GetLicense(_bstr_t(pSI->ApplicationSettings.TenantID), _bstr_t(pSI->ApplicationSettings.MachineGUID), _bstr_t(pSI->ApplicationSettings.AppGUID), _bstr_t(pSI->ApplicationSettings.GetLicenseModule(pSI->ApplicationSettings.ModuleID)), _T("CMS_32Bit"), _bstr_t(szProductType), CAV_LICENSEVERSION, false);
		CString szReturn((LPCWSTR)szLicenseInfo);
		CXMLDocument docReturnXML;
		if (szReturn != EMPTY_STRING)
		{
			docReturnXML.LoadXML(szLicenseInfo);

			// see what is installed on the system
			if (!pSI->GetPackageInstalled(&docReturnXML))
			{
				// If the module is not licensed then tell the user
				//  message 7: Either you have not entered your licensing key for this module or the license has expired. To update your module licensing, open the Controller module and follow the on-screen instructions. To purchase additional licenses, contact your Cougar Mountain Account Executive at (800) 390-7053.
				CGBLForm::CMSMessageBox(pSI->ApplicationMainFrame, IDS_NO_LICENSEKEYS_MSG, IDS_NO_LICENSEKEYS, MB_OK | MB_ICONERROR);
				return false;
			}
			// If the user has exceeded the maximum number of users, then tell them and return.
			if (pSI->ApplicationSettings.bMaxReached)
			{
				CString szNumUsers = CGBLForm::ToString(pSI->ApplicationSettings.nMaxUserCount);
				//  message 7: Either you have not entered your licensing key for this module or the license has expired. To update your module licensing, open the Controller module and follow the on-screen instructions. To purchase additional licenses, contact your Cougar Mountain Account Executive at (800) 390-7053.
				CGBLForm::CMSMessageBox(pSI->ApplicationMainFrame, IDS_NO_LICENSEKEYS_MSG, IDS_NO_LICENSEKEYS, MB_OK | MB_ICONERROR, szNumUsers);
				return false;
			}
		}
	}

	CCreateDBDlg* pDlg = new CCreateDBDlg;
	pDlg->Create(IDD_CREATEDB_DIALOG);

	//Set up dialog variables to start restore.
	CCreateDBDlg::TEMPLATE_TYPES eType = CCreateDBDlg::TEMPLATE_NONE;
	CString szDBType, szServer, szDBName, szUserName, szPass, szFileName;
	szDBType = pSI->Servers.PrimaryServer.DBType;
	szServer = pSI->Servers.PrimaryServer.Name;
	szUserName = pSI->Servers.PrimaryServer.User;

	if (pSI->ModulesInstalled.PackagesInstalled.ACT.Licensed)
	{
		pDlg->m_bRestore = true;
		//Install DMOACT Company
		eType = CCreateDBDlg::TEMPLATE_DMOA;
		szDBName = _T("DMOACT");
		szFileName = _T("DMOACTDATA.cab");
		pDlg->ShowWindow(SW_SHOW);
		pDlg->SetWindowText(CGBLResources::GetResourceString(IDS_DMO_ACT_DATA));
		pDlg->CreateDB(eType, szFileName, szDBType, szServer, szDBName, szUserName, szPass);

		while (pDlg->m_bRestore)
		{
			CGBLForm::DoEvents();
			Sleep(10);	// begbert 06-06-2006 : Releases the processor to other, working apps
		}

		pDlg->m_bRestore = true;
		//Install MLIACT Company
		eType = CCreateDBDlg::TEMPLATE_MLIA;
		szDBName = _T("MLIACT");
		szFileName = _T("MLIACTDATA.cab");
		pDlg->SetWindowText(CGBLResources::GetResourceString(IDS_DMO_MLI_DATA));
		pDlg->CreateDB(eType, szFileName, szDBType, szServer, szDBName, szUserName, szPass);

		while (pDlg->m_bRestore)
		{
			CGBLForm::DoEvents();
			Sleep(10);	// begbert 06-06-2006 : Releases the processor to other, working apps
		}

		pDlg->m_bRestore = true;
		//Install SSCACT Company
		eType = CCreateDBDlg::TEMPLATE_SSCA;
		szDBName = _T("SSCACT");
		szFileName = _T("SSCACTDATA.cab");
		pDlg->SetWindowText(CGBLResources::GetResourceString(IDS_DMO_SPS_DATA));
		pDlg->CreateDB(eType, szFileName, szDBType, szServer, szDBName, szUserName, szPass);

		while (pDlg->m_bRestore)
		{
			CGBLForm::DoEvents();
			Sleep(10);	// begbert 06-06-2006 : Releases the processor to other, working apps
		}
	}
	if (pSI->ModulesInstalled.PackagesInstalled.FUND.Licensed)
	{
		pDlg->m_bRestore = true;
		//Install DMOFND Company
		eType = CCreateDBDlg::TEMPLATE_DMOF;
		szDBName = "DMOFND";
		szFileName = "DMOFNDDATA.cab";
		//RCG - 09/12/2014 - Bug 8130/18883 - Check if already visible
		if (!pDlg->IsWindowVisible())
			pDlg->ShowWindow(SW_SHOW);
		pDlg->SetWindowText(CGBLResources::GetResourceString(IDS_DMO_FUND_DATA));
		pDlg->CreateDB(eType, szFileName, szDBType, szServer, szDBName, szUserName, szPass);

		while (pDlg->m_bRestore)
		{
			CGBLForm::DoEvents();
			Sleep(10);	// begbert 06-06-2006 : Releases the processor to other, working apps
		}
	}

	pDlg->CloseDialog();
	return TRUE;
}

// -------------------------------------------------------------
// BK 5/23/08 implemented Opening Denali Modules Routine design change spec	- when logged in as SUP show license manager menu item
void CMainFrame::ShowLicenseManagerMenuItem()
{
	// 1-31745 BK 1/16/09 moved from options menu to help menu
	CMenu* const pHelpMenu = Menu[IDS_HELP];
	if (!pHelpMenu)
		return;

	CMenu* const pMenu = Menu[IDS_MNU_OPEN_LICENSE_MGR];
	if (CMS::GetSI()->UserInformation.ID != RESSTRING(IDS_SUP))
	{
		// delete menu item
		if (pMenu)
			DeleteMenu(pMenu, IDS_MNU_OPEN_LICENSE_MGR);
	}
	else
	{
		// 1-31744 BK 1/16/09 only add menu item if NetReg.exe file exists
		CString szProgram = CGBLForm::GetPath() + _T("NetReg.exe");
		CFileFind fileFind;
		if (!fileFind.FindFile(szProgram))
			return;

		// insert pre-issue menu item
		if (!pMenu)
		{
			int	nPos = MenuData[IDS_ABOUT_TITLE].MenuIndex + 1;
			if (!InsertMenu(pHelpMenu, (UINT)nPos, IDS_MNU_OPEN_LICENSE_MGR, RESSTRING(IDS_MNU_OPEN_LICENSE_MGR), RESSTRING(IDS_MNU_OPEN_LICENSE_MGR_STATUS)))
				return;
			// uncomment and remove AddSeparator when fixing defect 1-29852 that's currently on hold
			// InsertSeparator(Menu[IDS_OPTIONS], IDS_MNU_OPEN_LICENSE_MGR);
			pHelpMenu->EnableMenuItem(IDS_MNU_OPEN_LICENSE_MGR, MF_BYCOMMAND | MF_ENABLED);
		}
	}
}

// begbert 02-10-2014 PBI 138 : Added this
void CMainFrame::ShowConvertPayrollFromCMSProMenuItem()
{
	//RCG - 08/07/2018 - PBI 42384 - Removed the Convert Payroll To Denali menu item in 64-bit Denali
#if defined _M_IX86
	CString strPath = CMS::GetSI()->ApplicationPaths.szApplicationPath;
	CFile Module;
	CFileStatus stat;

	CMenu* const pMenu = Menu[IDS_ORGANIZATION_OPTION];
	if (pMenu && !Module.GetStatus(strPath + "CMSPR.exe", stat) || !si->ModulesInstalled.Payroll.Licensed)
	{
		// delete menu item
		if (pMenu)
			DeleteMenu(pMenu, IDS_CONVERT_PAYROLL_FROM_CMSPRO);
	}
#elif defined _M_X64
	CMenu* const pMenu = Menu[IDS_ORGANIZATION_OPTION];
	// delete menu item
	if (pMenu)
		DeleteMenu(pMenu, IDS_CONVERT_PAYROLL_FROM_CMSPRO);
#endif
}

//------------------------------------------------------------------------------

// begbert 08-09-2010 Navigator Interface : command handlers
void CMainFrame::OnNav_CTSetupSelectaNewCompany() { SimulateClickOn(IDS_MNU_SETUP_SELECT_ORG); }
void CMainFrame::OnNav_CTInstallModules() { SimulateClickOn(IDS_INSTALL_MODULE); }
void CMainFrame::OnNav_CTConvertPayrollFromCMSPro() { SimulateClickOn(IDS_CONVERT_PAYROLL_FROM_CMSPRO); }
void CMainFrame::OnNav_CTChangeDate() { SimulateClickOn(IDS_CHANGE_DATE); }
void CMainFrame::OnNav_CTFiscalCalendar() { SimulateClickOn(IDS_FISCAL_CALENDAR); }
void CMainFrame::OnNav_CTElectronicConstants() { SimulateClickOn(IDS_CTRL_MAGNETIC_MEDIA_CONST); }
void CMainFrame::OnNav_CTAddChangeUsers() { SimulateClickOn(IDS_CTRL_ADD_CHNG_USER); }
void CMainFrame::OnNav_CTAddChangeGroups() { SimulateClickOn(IDS_GROUPS); }
void CMainFrame::OnNav_CTChangePassword() { SimulateClickOn(IDS_CTRL_CHNG_PASSWD); }
void CMainFrame::OnNav_CTResetPassword() { SimulateClickOn(IDS_CT_CISP_RESETPASSWORD); }
void CMainFrame::OnNav_CTCopyUsersFromCompany() { SimulateClickOn(IDS_CTRL_COPY_USERS); }
void CMainFrame::OnNav_CTCopyGroupsFromCompany() { SimulateClickOn(IDS_CTRL_COPY_GROUPS); }
void CMainFrame::OnNav_CTCurrencyValues() { SimulateClickOn(IDS_CAPTION_CURRENCIES); }
void CMainFrame::OnNav_CTExchangeRates() { SimulateClickOn(IDS_EXCHANGE_RATES); }
void CMainFrame::OnNav_CTDisplayControllerTableInformation() { SimulateClickOn(IDS_CTRL_DISPLAY_FILE_INFO); }
void CMainFrame::OnNav_CTProcessPayrollTransactions() { SimulateClickOn(IDS_PAY_PROCESS_PAYROLL); }
void CMainFrame::OnNav_CTBackupCompany() { SimulateClickOn(IDS_CTRL_BKUP_DATA); }
void CMainFrame::OnNav_CTRestoreCompany() { SimulateClickOn(IDS_CTRL_RESTORE_DATA); }
void CMainFrame::OnNav_ControllerModulePreferences() { SimulateClickOn(IDS_MODULE_PREFERENCES); }
void CMainFrame::OnNav_CTCopyOrganizationUtility() { SimulateClickOn(IDS_COPY_ORGANIZATION_UTILITY); } // begbert 04-16-2013 PBI 4036 : Added this

// PBI 17783 BK 3/11/14 added new Merchant Warehouse setup form
void CMainFrame::OnNav_CTMerchantWareSetup() { SimulateClickOn(IDS_MERCHANT_WAREHOUSE_SETUP); }
//RS - 04/26/2024 - PBI 63797 - Added user security and menu item to Organization menu
void CMainFrame::OnNav_CTCloverConnectSetup() { SimulateClickOn(IDS_CARDPOINTE_SETUP); }
void CMainFrame::OnNav_CTNeonCRMSetup() { SimulateClickOn(IDS_NEONCRM_SETUP); }

//	PBI 45155	06/06/2018	mvt - Added security for Denali BI Setup
void CMainFrame::OnNav_CTDenaliBusinessIntelligence() { SimulateClickOn(IDS_DENALIBI_MENU); }

//	PBI 47520	12/04/2018	mvt - Added security for Yodlee integration setup
void CMainFrame::OnNav_CTYodleeIntegrationSetup() { SimulateClickOn(IDS_MENU_SETUPYODLEEINTEGRATION); }

//	PBI 50961	09/04/2019	mvt - Added security for PayPal integration setup
void CMainFrame::OnNav_CTPayPalIntegrationSetup() { SimulateClickOn(IDS_MENU_SETUPPAYPALINTEGRATION); }

//	PBI 52317	11/19/2019	mvt - Added security for Donately integration setup
void CMainFrame::OnNav_CTDonatelyIntegrationSetup() { SimulateClickOn(IDS_DONATELYSETUP_MENU); }

//	PBI 46406	09/26/2018	mvt - Added security for Internal Control Alerts in Controller
void CMainFrame::OnNav_CTInternalControlAlertsSetup() { SimulateClickOn(IDS_CT_INTERNALCONTROLALERTSSETUP_MENU); }

CMainFrame::CNavigatorCodeBatchInfo CMainFrame::GetCodeBatchInfo(const CString& szDataType)
{
	//	if (szDataType == "ControllerBatches")
	//		return GetCodesBatchesForNavigator(_T("CT"), _T("BATCHLIST" ), 0, _T("strBatchCode"), _T("strDescription"));
	//	else
	return CNavigatorCodeBatchInfo();
}

// InTimeTec: 11.20.2012.HS - Custom Menus
// Calls the server to fetch custom menus defined for this module and
// add the custom menus to the main menu bar.
void CMainFrame::CreateCustomMenu(void)
{
	CCMSDataAdapter	objDA;

	objDA.SetGetCommand(CMSStrings::XMLTags::GET);
	objDA.SetGetModule(_T("CT"));
	objDA.SetGetObject(_T("CMS_CUSTOMMENUS"));

	CDataSet objDS;
	CDataTable dataTable;
	CDataRow   dataRow;
	CString szMenuName;
	CString szCategory;
	CString szHashValue;
	CString szFilePath;
	bool bolIsUrl;
	CXMLDocument xmlDoc(CMSStrings::XMLTags::Root);
	CXMLElement root = xmlDoc.GetDocumentElement();
	CXMLElement table = root.AddNewChild(CMSStrings::XMLTags::TABLE);
	CXMLElement record = table.AddNewChild(CMSStrings::XMLTags::Record);
	record.AddNewChild(_T("strModule"), CMS::GetSI()->ApplicationSettings.ModuleStringID);
	//-------------------------------------

	const bool bolStatus = objDA.GetData(xmlDoc, objDS) == rcSuccess && objDS.TableExists(CMSStrings::XMLTags::Table);
	if (!bolStatus)
	{
		CMenu* const pMenu = Menu[IDS_CUSTOM_OPTIONS];
		if (pMenu)
			DeleteMenu(&MainMenu, IDS_CUSTOM_OPTIONS, true);
		this->Frame->DrawMenuBar();
		return;
	}

	objDS.GetTable(CMSStrings::XMLTags::Table, &dataTable);
	if (!dataTable.IsEmpty())
	{
		m_clsCustomOptions.ResetPathMaps();
		CMenu* const pMenu = Menu[IDS_CUSTOM_OPTIONS];
		if (!pMenu)
			if (!AddMenu(&MainMenu, IDS_CUSTOM_OPTIONS)) return;

		for (int lRow = 0; lRow < dataTable.getRowCount(); lRow++)
		{
			dataTable.GetRow(lRow, &dataRow);
			szCategory = dataRow.XMLStr(_T("strCategory"));
			szMenuName = dataRow.XMLStr(_T("strMenuName"));
			szHashValue = dataRow.XMLStr(_T("strFileHash"));
			bolIsUrl = dataRow.XMLBOOL(_T("bolURL"));

			if (!szCategory.CompareNoCase(_T("None")))
				if (!AddMenu(Menu[IDS_CUSTOM_OPTIONS], (UINT)(IDS_CUSTOM_MENU_START + lRow), szMenuName, 0)) break;
				else
				{
					CMenu* const pMenu = this->Menu[IDS_CUSTOM_OPTIONS];
					CMenu* pObjMenu = m_clsCustomOptions.GetCustomMenuPopup(pMenu, szCategory);

					if (pObjMenu == NULL)
					{
						COXBitmapMenu menu;
						menu.CreatePopupMenu();
						pMenu->AppendMenu(MF_POPUP, (UINT_PTR)menu.Detach(), szCategory);
						pObjMenu = m_clsCustomOptions.GetCustomMenuPopup(pMenu, szCategory);
						if (!AddMenu(pObjMenu, (UINT)(IDS_CUSTOM_MENU_START + lRow), szMenuName, 0)) break;
					}
					else
						if (!AddMenu(pObjMenu, (UINT)(IDS_CUSTOM_MENU_START + lRow), szMenuName, 0)) break;
				}
			if (bolIsUrl)
			{
				szFilePath = dataRow.XMLStr(_T("strPath"));
				m_clsCustomOptions.AddToPathMap(IDS_CUSTOM_MENU_START + lRow, szFilePath);
			}
			else
			{
				if (!szHashValue.IsEmpty())
					m_clsCustomOptions.AddToPublishedMap(IDS_CUSTOM_MENU_START + lRow, dataRow.XMLInt(_T("intRecordID")));
				else
				{
					szFilePath = dataRow.XMLStr(_T("strPath")) + '\\' + dataRow.XMLStr(_T("strFileName"));
					m_clsCustomOptions.AddToPathMap(IDS_CUSTOM_MENU_START + lRow, szFilePath);
				}
			}
		}
	}
	xmlDoc.Dispose();
	objDS.Dispose();

	this->Frame->DrawMenuBar();
}
// InTimeTec: End

// InTimeTec: 11.20.2012.HS - Custom Menus
// Overloaded to receive the command messages for custom menus.
BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (pHandlerInfo == NULL)
	{
		if (m_clsCustomOptions.IsIDFound(nID))
		{
			if (nCode == CN_COMMAND)
				// Handle WM_COMMAND message
				m_clsCustomOptions.OnProcessCustomMenu(nID);
			else if (nCode == CN_UPDATE_COMMAND_UI)
				// Update UI element state
				m_clsCustomOptions.OnUpdateCustomOptions((CCmdUI*)pExtra);
			return TRUE;
		}
	}
	return __super::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
// InTimeTec: End

// InTimeTec: 11.20.2012.HS - Custom Menus
// Enable/Disable the custom menu popup based on user security rights.
void CMainFrame::CustomMenuSecurityDisableItem()
{
	int	  nPos = -1;
	const stuCTNode& CTRights = si->UserRight.m_stuControllerNode;
	CMenu* const pCustomOptionsMenu = Menu[IDS_CUSTOM_OPTIONS];
	if (pCustomOptionsMenu)
	{
		nPos = MenuData[IDS_CUSTOM_OPTIONS].MenuIndex;
		if (si->UserInformation.ID.IsEmpty())
			MenuData[IDS_REPORTS].ParentMenu->EnableMenuItem((UINT)nPos, MF_BYPOSITION | MF_GRAYED);
		else
			MenuData[IDS_CUSTOM_OPTIONS].ParentMenu->EnableMenuItem((UINT)nPos, MF_BYPOSITION | (CTRights.bCustomOptions ? (UINT)MF_ENABLED : (UINT)MF_GRAYED));
	}

	DrawMenuBar();
}

// InTimeTec: 04.17.2013.HS - Custom Menus
void CMainFrame::UpdateCustomMenu()
{
	CMenu* const pMenu = Menu[IDS_CUSTOM_OPTIONS];

	if (!pMenu)
	{
		CreateCustomMenu();
		CustomMenuSecurityDisableItem();
		return;
	}

	int nMenuItemCount = pMenu->GetMenuItemCount();
	for (int nPos = 0; nPos < nMenuItemCount; nPos++)
		pMenu->DeleteMenu(0, MF_BYPOSITION);
	CreateCustomMenu();
	CustomMenuSecurityDisableItem();
}
// InTimeTec: End

//RCG - 03/30/2017 - Bug 32830 - Check if Payroll tables exist
bool CMainFrame::DoPayrollTablesExist(void)
{
	CXML		 clsXML;
	CXMLParams   clsXMLCommand, clsXMLData, clsXMLCompany(true);

	clsXML.m_szXMLCommand.Empty();
	clsXML.m_szXMLData.Empty();

	// xml Command
	CXMLParams::MakeXMLCommand(&clsXMLCommand, _T("SPECIALPROCESS"), _T("PR"), _T("PR_DOPAYROLLTABLESEXIST"));
	clsXML.m_szXMLCommand = (clsXMLCommand.GetXML());

	// append xml connection
	CXMLParams::AppendXMLConnection(&clsXMLData, CXMLParams::CN_COMPANY);

	// append company info
	clsXML.MakeCOMPANYINFO();
	clsXMLCompany.SetXML(clsXML.m_szXMLFormat);
	clsXMLData.AppendXMLParam(&clsXMLCompany);

	// put xml Params into variable and send XML
	clsXML.m_szXMLData = clsXMLData.GetXML();
	CString szReturn = clsXML.SendXML().copy();

	// if Payroll tables exist, return true
	if (szReturn.Find(CMSStrings::XMLTags::FoundRecord) > 0)
		return true;

	return false;
}