/*****************************************************************************
 *
 *  (C) Copyright 2003-2026 Cougar Mountain Software
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
#include "CTDllFormManager.h"

 //Header files needed to add templates that are used within this class.
#include "DLG_CTInstallModule.h" //1-11369 DTG, 6.2.2004
#include "DLG_CTFiscalCalendar.h" // 1-11356 DTG, 6.2.2004, commented out
#include "CFV_CTMagneticMediaConstants.h"
#include "MFV_CTCurrencies.h"
#include "CFV_CTCurrencyCodes.h"
#include "CFV_CTExchangeRates.h"
#include "CFV_CTBackupCompanyInfo.h"
#include "CFV_CTRestoreCompanyInfo.h"
#include "CFV_CTNewCustomDetails.h"
#include "CFV_CTAddNewCompany.h"
#include "CFV_CTEditUserorGroup.h"
#include "DLG_CTOrgCompany.h"
#include "CFV_CTCopyUsersOrGroupsFromCompany.h"
#include "CFV_CTCopyUsersOrGroups.h"
#include "CFV_CTModPrefs.h"

// BK 01/13/11 THESE FILES IS NO LONGER USED (they're duplicate Setup Company files)
//#include "CFV_CTNewOrganizationDetails.h"
//#include "CFV_CTNewCompanyDetails.h"

// BK 01/13/11 THESE FILES IS NO LONGER USED (form that contains TEMPLATE LIST for creating new companies)
//#include "CFV_CTAddNewCompany2.h"


CCTDllFormManager::CCTDllFormManager(CCMSWinApp* pWinApp) :CDllFormManager(pWinApp)
{
}

CCTDllFormManager::~CCTDllFormManager(void)
{
}

//Register document templates for forms that are only used within this dll.
//Those forms need not be exported outside this dll, so once they are
//added here, the AFX_EXT_CLASS can be removed from the header declaration of those
//respective classes.
bool CCTDllFormManager::AddPrivateDocTemplates()
{
	try
	{
		// template for Install Modules
		//1-11369 DTG, 6.2.2004, commented out
		//RegisterForm(IDD_INSTALL_MODULE, RUNTIME_CLASS(CCFV_CTInstallModule)) ;
		// 1-11356 DTG, 6.2.2004, commented out
		// template for Fiscal Calendar
		//RegisterForm(IDD_DLG_FISCAL_CAL, RUNTIME_CLASS(CCFV_CTFiscalCalendar)) ;
		// template for Magnetic Media Constants
		RegisterForm(IDD_CMS_MAGNETIC_MEDIA_CONSTANTS, RUNTIME_CLASS(CCFV_CTMagneticMediaConstants));
		// template for Add Change Users
		RegisterForm(IDD_EDIT_USER_OR_GROUP, RUNTIME_CLASS(CCFV_CTEditUserorGroup), RUNTIME_CLASS(CCMSMaintDoc), FORM_USERGROUP_USERS);
		// template for Currencies
		//RegisterForm(IDD_CURRENCIES, RUNTIME_CLASS(CMFV_CTCurrencies)) ;
		RegisterForm(IDD_CTF_CFV_CURRENCYCODE, RUNTIME_CLASS(CCFV_CTCurrencyCodes), RUNTIME_CLASS(CCMSMaintDoc));
		// template for Exchange Rates
		//RegisterForm(IDD_EXCHANGE_RATES, RUNTIME_CLASS(CCFV_CTExchangeRates), RUNTIME_CLASS(CDOC_CTExchangeRates)) ;
		RegisterForm(IDD_EXCHANGE_RATES, RUNTIME_CLASS(CCFV_CTExchangeRates), RUNTIME_CLASS(CCMSMaintDoc));
		// template for Backup Company Info
		RegisterForm(IDD_BACKUP_COMPANY_INFO, RUNTIME_CLASS(CCFV_CTBackupCompanyInfo));
		// template for Restore Company Info
		RegisterForm(IDD_CTF_RESTORE_COMPANY_INFO, RUNTIME_CLASS(CCFV_CTRestoreCompanyInfo));
		// template for Change Groups
		RegisterForm(IDD_EDIT_USER_OR_GROUP, RUNTIME_CLASS(CCFV_CTEditUserorGroup), RUNTIME_CLASS(CCMSMaintDoc), FORM_USERGROUP_GROUPS);
		// BK 01/13/11 THIS FILE IS NO LONGER USED - View/Modify Company Info
		// template for Company Info
		RegisterForm(IDD_ADD_NEW_COMPANY, RUNTIME_CLASS(CCFV_CTAddNewCompany));
		// BK 01/13/11 THIS FILE IS NO LONGER USED
		// template for Company Details
		//RegisterForm(IDD_NEW_COMPANY_DETAILS, RUNTIME_CLASS(CFV_CTNEWCompanyDetails)) ;
		// template for Custom Details
		RegisterForm(IDD_NEW_CUSTOM_DETAILS, RUNTIME_CLASS(CCFV_CTNewCustomDetails));
		// BK 01/13/11 THIS FILE IS NO LONGER USED
		// template for Organization Details
		//RegisterForm(IDD_NEW_ORGANIZATION_DETAILS, RUNTIME_CLASS(CCFV_CTNewOrganizationDetails)) ;
		// template for Copy users or groups from company
		RegisterForm(IDD_COPY_USERS_OR_GROUPS_FROM_COMPANY, RUNTIME_CLASS(CCFV_CTCopyUsersOrGroupsFromCompany), FORM_COPY_GROUPS);
		// template for Copy users or groups from company
		RegisterForm(IDD_COPY_USERS_OR_GROUPS_FROM_COMPANY, RUNTIME_CLASS(CCFV_CTCopyUsersOrGroupsFromCompany), FORM_COPY_USERS);
		// template for Copy users or groups form.(from company)
		RegisterForm(IDD_COPY_USERS_OR_GROUPS, RUNTIME_CLASS(CCFV_CTCopyUsersOrGroups));

		// Options menu - Module Preferences
		RegisterForm(IDD_GBL_MODPREFS_STD, RUNTIME_CLASS(CCFV_CTModPrefs), RUNTIME_CLASS(CCMSMaintDoc));
	}
	catch (...)
	{
		TRACE("AddDocTemplates - Exception error\n");
		return false;
	}
	return true;
}
