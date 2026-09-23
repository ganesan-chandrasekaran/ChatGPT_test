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

#include "StdAfx.h"
#include "..\Common\LangIds.h"
#include "..\cmsdll\CMSStandardTaskProcessor.h"


 // PBI 11394 BK 10/28/13 per Product Owner removed additional security for attachments, returned rights back to original
 //initiated so I can get global variable values. Such as if it is currently fund or act.
extern CGBLSystemInformation* g_pGBLSystemInformationCMSDll;

typedef void (CGBLUserRightsInformation::* MemberFunction)(void);
bool CCMSTask<CGBLUserRightsInformation, MemberFunction>::Execute(ICMSTaskProcessor* pProcessor)
{
	(m_pObj->*m_pMethod)();
	pProcessor->Scheduler->Remove(this);//Forgot to add this. Thanks Sonny.
	return true;
}
// removed MagneticMediaConstants

// default constructor
CGBLUserRightsInformation::CGBLUserRightsInformation()
	:m_pCompanyUserRights(NULL)
	, m_szDecryptedSecurityXML(EMPTY_STRING)
{
	// setting member variables
	m_hGBLResourcesHandle = CGBLResources::GetResourceHandle();
	CreateObjectList(); // creating object tree and connecting to flags in the structures
}

// destructor
CGBLUserRightsInformation::~CGBLUserRightsInformation()
{
	m_clsUserRightsProcessor.Scheduler->Empty();
	// deleting root node
	delete m_pUserRightsRootNode;
}

// this function creates the instances of all objects / nodes in the tree
void CGBLUserRightsInformation::CreateObjectList()
{
	// creating root node
	m_pUserRightsRootNode = new CUserRightsBaseNode(NULL);
	m_pUserRightsRootNode->m_szTagName = "UserRightsRoot";
	m_pUserRightsRootNode->m_szTreeItemDescription = "Full Denali Access";	// begbert 03-12-2010 spec "PR Integration with Denali" : Updated display text for the node
	CreateControllerObjectList(); // controller

	// Error: I9096 on   11/25/2003  Added Conditions to Check Modules Installed or not before creating Nodes.
	if (g_pGBLSystemInformationCMSDll != NULL)
	{
		if (g_pGBLSystemInformationCMSDll->ModulesInstalled.AccountsPayable.IsUsable)
			CreateAccountsPayableObjectList();		// Accounts Payable
		if (g_pGBLSystemInformationCMSDll->ModulesInstalled.AccountsReceivable.IsUsable)
			CreateAccountsReceivableObjectList();	// Accounts Receivable
		if (g_pGBLSystemInformationCMSDll->ModulesInstalled.BankReconciliation.IsUsable)
			CreateBankReconciliationObjectList();	// Bank Reconciliation
		CreateCougarDtailsObjectList();				// CougarDtails // begbert 02-05-2010 : Added this
		if (g_pGBLSystemInformationCMSDll->ModulesInstalled.GeneralLedger.IsUsable)
			CreateGeneralLedgerObjectList();		// General Ledger
		if (g_pGBLSystemInformationCMSDll->ModulesInstalled.Inventory.IsUsable)
			CreateInventoryObjectList();			// Inventory
		if (g_pGBLSystemInformationCMSDll->ModulesInstalled.JobCost.IsUsable)
			CreateJobCostObjectList();				// Job Cost
		if (g_pGBLSystemInformationCMSDll->ModulesInstalled.Payroll.IsUsable)
			CreatePayRollObjectList();				// Payroll
		if (g_pGBLSystemInformationCMSDll->ModulesInstalled.PurchaseOrder.IsUsable)
			CreatePurchaseOrderObjectList();		// Purchase Order
		if (g_pGBLSystemInformationCMSDll->ModulesInstalled.SalesEntry.IsUsable)
			CreateSalesMaintenanceObjectList();		// Sales Maintenance
	}
	else
	{
		CreateAccountsPayableObjectList();		// Accounts Payable
		CreateAccountsReceivableObjectList();	// Accounts Receivable
		CreateBankReconciliationObjectList();	// Bank Reconciliation
		CreateCougarDtailsObjectList();			// CougarDtails	// begbert 02-05-2010 : Added this
		CreateGeneralLedgerObjectList();		// General Ledger
		CreateInventoryObjectList();			// Inventory
		CreateJobCostObjectList();				// Job Cost
		CreatePayRollObjectList();				// Payroll
		CreatePurchaseOrderObjectList();		// Purchase Order
		CreateSalesMaintenanceObjectList();		// Sales Maintenance
	}
}

// creates tree node
// szTagName is used as the XML tag
// szTreeItemDescription is what is used in the tree control as the description
// pbUserRightsFlag is a pointer to a bool in the structure used to store the flags.  the object will write directly to the structure
CUserRightsBaseNode* CGBLUserRightsInformation::CreateNode(const CString& szTagName, UINT uTreeItemDescriptionResource, bool* pbUserRightsFlag)
{
	CUserRightsBaseNode* pChildNode = new CUserRightsBaseNode(pbUserRightsFlag);
	pChildNode->m_szTagName = szTagName;
	pChildNode->m_szTreeItemDescription = CGBLResources::GetResourceString(uTreeItemDescriptionResource, m_hGBLResourcesHandle); // m_hGBLResourcesHandle set once in the constructor
	//Defect ID 1-14292, PGP(11/15/2004) - Removed code to remove & from string.
	//Replaced strings so that passed strings does not contain extra &s.

	pChildNode->m_szTreeItemDescription.Remove('.');
	return pChildNode;
}

// creates object list for the accounts payable node
//=================================== Start Of AP ================================================================//
//================================================================================================================//
void CGBLUserRightsInformation::CreateAccountsPayableObjectList()
{
	// creating accounts payable node
	CUserRightsBaseNode* pAccountsPayableNode = CreateNode("AccountsPayable", IDS_MODULE_AP, &m_stuModulesNode.bAccountsPayable); // Accounts Payable
	m_pUserRightsRootNode->m_tplUserRightsNodeList.AddTail(pAccountsPayableNode);

	//Vendor
	CUserRightsBaseNode* pAPVendorNode = CreateNode("Vendor", IDS_VENDOR, &m_stuAccountsPayableNode.Vendor.bTotal);
	pAccountsPayableNode->m_tplUserRightsNodeList.AddTail(pAPVendorNode);

	// PBI 17468 BK 9/12/14 Change user rights to separate Add/Edit and View Only settings, Vendor Master Records
	CUserRightsBaseNode* pVendorMasterNode = CreateNode("VendorMaster", IDS_VENDOR_MASTER_RECORDS, &m_stuAccountsPayableNode.Vendor.VendorMaster.bTotal);
	pAPVendorNode->m_tplUserRightsNodeList.AddTail(pVendorMasterNode);
	pVendorMasterNode->m_tplUserRightsNodeList.AddTail(CreateNode("AddEditVendor", IDS_ADD_EDIT_VENDOR_RECORDS, &m_stuAccountsPayableNode.Vendor.VendorMaster.bAddEditVendor));
	pVendorMasterNode->m_tplUserRightsNodeList.AddTail(CreateNode("ViewVendor", IDS_VIEW_VENDOR_RECORDS, &m_stuAccountsPayableNode.Vendor.VendorMaster.bViewVendor));
	//RS - 09/04/2024 - PBI 63268 - Added security to not view/edit ACH Information
	pVendorMasterNode->m_tplUserRightsNodeList.AddTail(CreateNode("EditRemitTo", IDS_EDIT_REMIT_TO, &m_stuAccountsPayableNode.Vendor.VendorMaster.bEditRemitTo));

	pAPVendorNode->m_tplUserRightsNodeList.AddTail(CreateNode("CopyVendors", IDS_RIGHTS_COPY_VENDORS, &m_stuAccountsPayableNode.Vendor.bCopyVendors)); // Copy Vendors
	pAPVendorNode->m_tplUserRightsNodeList.AddTail(CreateNode("RenumberVendors", IDS_RIGHTS_RENUMBER_VENDORS, &m_stuAccountsPayableNode.Vendor.bRenumberVendors)); //Renumber Vendors
	pAPVendorNode->m_tplUserRightsNodeList.AddTail(CreateNode("DefaultVendorTemplate", IDS_RIGHTS_DEFAULT_VENDOR_TEMPLATE, &m_stuAccountsPayableNode.Vendor.bDefualtVendorTemplate)); //Default Vendor Template...
	CUserRightsBaseNode* pAPImportExportNode = CreateNode("ImportExport", IDS_IMPORT_EXPORT, &m_stuAccountsPayableNode.Vendor.ImportExport.bTotal); // File Maintenance
	pAPVendorNode->m_tplUserRightsNodeList.AddTail(pAPImportExportNode);
	pAPImportExportNode->m_tplUserRightsNodeList.AddTail(CreateNode("Import", IDS_IE_IMPORT, &m_stuAccountsPayableNode.Vendor.ImportExport.bImport)); // Import information okay
	pAPImportExportNode->m_tplUserRightsNodeList.AddTail(CreateNode("Export", IDS_IE_EXPORT, &m_stuAccountsPayableNode.Vendor.ImportExport.bExport)); // Export information okay
	//RCG - 06/11/2021 - PBI 57746 - Added Import/Export ArcBill
	CUserRightsBaseNode* pAPImportExportArcBillNode = CreateNode("ImportExportArcBill", IDS_IMPORT_EXPORT_ARC_BILL, &m_stuAccountsPayableNode.Vendor.ImportExportArcBill.bTotal); // Import/Export ArcBill
	pAPVendorNode->m_tplUserRightsNodeList.AddTail(pAPImportExportArcBillNode);
	pAPImportExportArcBillNode->m_tplUserRightsNodeList.AddTail(CreateNode("ImportVendors", IDS_IMPORT_VENDORS, &m_stuAccountsPayableNode.Vendor.ImportExportArcBill.bImportVendors)); // Import Vendors
	pAPImportExportArcBillNode->m_tplUserRightsNodeList.AddTail(CreateNode("ExportVendors", IDS_EXPORT_VENDORS, &m_stuAccountsPayableNode.Vendor.ImportExportArcBill.bExportVendors)); // Export Vendors
	pAPImportExportArcBillNode->m_tplUserRightsNodeList.AddTail(CreateNode("ExportGLAccounts", IDS_EXPORT_GL_ACCOUNTS, &m_stuAccountsPayableNode.Vendor.ImportExportArcBill.bExportGLAccounts)); // Export GL Accounts

	////////////////////////
	//Enter Bills
	CUserRightsBaseNode* pAPEnterBillsNode = CreateNode("APEnterBills", IDS_RIGHTS_ENTER_BILLS, &m_stuAccountsPayableNode.EnterBills.bTotal); // Enter Bills
	pAccountsPayableNode->m_tplUserRightsNodeList.AddTail(pAPEnterBillsNode);
	pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("EnterBills", IDS_RIGHTS_ENTER_BILLS, &m_stuAccountsPayableNode.EnterBills.bEnterBills)); // Enter Bills
	// generate Bills from node
	CUserRightsBaseNode* pAPGenerateBillsFromNode = CreateNode("GenerateBillsFrom", IDS_RIGHTS_GENERATE_BILLS_FROM, &m_stuAccountsPayableNode.EnterBills.GenerateBillsFrom.bTotal); // Generate Bills From
	pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(pAPGenerateBillsFromNode);
	pAPGenerateBillsFromNode->m_tplUserRightsNodeList.AddTail(CreateNode("Recurring", IDS_RIGHTS_RECURRING, &m_stuAccountsPayableNode.EnterBills.GenerateBillsFrom.bRecurring)); // Recurring
	pAPGenerateBillsFromNode->m_tplUserRightsNodeList.AddTail(CreateNode("ExternalFile", IDS_RIGHTS_EXTERNAL_FILE, &m_stuAccountsPayableNode.EnterBills.GenerateBillsFrom.bExternalFile)); //External File
	//	PBI 49849	06/14/2019	mvt - Added security for Expensify
	pAPGenerateBillsFromNode->m_tplUserRightsNodeList.AddTail(CreateNode("Expensify", IDS_EXTERNAL_BILLING, &m_stuAccountsPayableNode.EnterBills.GenerateBillsFrom.bExpensify)); //	Expensify
	//******************************//
	pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintEditReport", IDS_RIGHTS_PRINT_EDIT_REPORT, &m_stuAccountsPayableNode.EnterBills.bPrintEditReport)); //Print Edit Report
	//	PBI 46406	09/26/2018	mvt - Added security for Internal Control Alerts in AP
	pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("SendInternalControlAlerts", IDS_SEND_INTERNAL_CONTROL_ALERTS, &m_stuAccountsPayableNode.EnterBills.bSendInternalControlAlerts));
	pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintInventoryLabels", IDS_RIGHTS_PRINT_INVENTORY_LABELS, &m_stuAccountsPayableNode.EnterBills.bPrintInventoryLabels)); //Print Inventory Labels
	pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintImmediateChecks", IDS_RIGHTS_PRINT_IMMEDIATE_CHECKS, &m_stuAccountsPayableNode.EnterBills.bPrintImmediateChecks)); //Print Immediate Checks
	pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ElectronicFundsTransfer", IDS_RIGHTS_ELECTRONIC_FUNDS_TRANSFER, &m_stuAccountsPayableNode.EnterBills.bElectronicFundsTransfer)); //Electronic Funds Transfer
	// PBI 267
	pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PositivePay", IDS_POSITIVE_PAY, &m_stuAccountsPayableNode.EnterBills.bPostivePay));
	pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ImmediateCheckRegister", IDS_RIGHTS_IMMEDIATE_CHECK_REGISTER, &m_stuAccountsPayableNode.EnterBills.bImmediateCheckRegister)); //Immediate Check Register
	pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PostBills", IDS_RIGHTS_POST_BILLS, &m_stuAccountsPayableNode.EnterBills.bPostBills)); //Post Bills
	// batch control
	CUserRightsBaseNode* pAPEnterBillsBatchControlNode = CreateNode("BatchControl", IDS_RIGHTS_BATCH_CONTROL, &m_stuAccountsPayableNode.EnterBills.BatchControl.bTotal); // Batch Control
	pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(pAPEnterBillsBatchControlNode);
	pAPEnterBillsBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SelectBatch", IDS_RIGHTS_SELECT_BATCH, &m_stuAccountsPayableNode.EnterBills.BatchControl.bSelectBatch)); //Select Batch
	pAPEnterBillsBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SetupBatch", IDS_RIGHTS_SETUP_BATCH, &m_stuAccountsPayableNode.EnterBills.BatchControl.bSetupBatch)); //Setup Batch
	//********************************//
	pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ClearBills", IDS_RIGHTS_CLEAR_BILLS, &m_stuAccountsPayableNode.EnterBills.bClearBills)); //Clear Bills
	pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ChangeLocalTransactionDate", IDS_RIGHTS_CHANGE_LOCAL_TRANSACTION_DATE, &m_stuAccountsPayableNode.EnterBills.bChangeLocalTransactionDate)); //Change Local Tran Date

	////////////////////////
	//AP Pay Bills
	CUserRightsBaseNode* pAPPayBillsNode = CreateNode("APPayBills", IDS_RIGHTS_PAY_BILLS, &m_stuAccountsPayableNode.PayBills.bTotal); //Pay Bills
	pAccountsPayableNode->m_tplUserRightsNodeList.AddTail(pAPPayBillsNode);
	pAPPayBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PreparePaymentsAutomatically", IDS_RIGHTS_PREPARE_PAYMENTS_AUTO, &m_stuAccountsPayableNode.PayBills.bPreparePaymentsAutomatically)); //Prepare Payments Automatically
	pAPPayBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("AdjustPayBills", IDS_RIGHTS_ADJUST_PAY_BILLS, &m_stuAccountsPayableNode.PayBills.bAdjustPayBills)); //Adjust/Pay Bills
	pAPPayBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintEditReport", IDS_RIGHTS_PRINT_EDIT_REPORT, &m_stuAccountsPayableNode.PayBills.bPrintEditReport)); //Print Edit Report
	//	PBI 46406	09/26/2018	mvt - Added security for Internal Control Alerts in AP
	pAPPayBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("SendInternalControlAlerts", IDS_SEND_INTERNAL_CONTROL_ALERTS, &m_stuAccountsPayableNode.PayBills.bSendInternalControlAlerts));
	pAPPayBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ElectronicFundsTransfer", IDS_RIGHTS_ELECTRONIC_FUNDS_TRANSFER, &m_stuAccountsPayableNode.PayBills.bElectronicFundsTransfer)); //Electronic Funds Transfer
	pAPPayBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintChecks", IDS_RIGHTS_PRINT_CHECKS, &m_stuAccountsPayableNode.PayBills.bPrintChecks)); //Print Checks
	//	JEPK PBI 23495 9/18/2014 Fixed strings and comment for Positive Pay
	pAPPayBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PositivePay", IDS_POSITIVE_PAY, &m_stuAccountsPayableNode.PayBills.bPositivePay)); // Positive Pay
	pAPPayBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("CheckRegister", IDS_CHECK_REGISTER, &m_stuAccountsPayableNode.PayBills.bCheckRegister)); //Check Register
	pAPPayBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PostPayments", IDS_POST_PAYMENTS, &m_stuAccountsPayableNode.PayBills.bPostPayments)); //Post Payments
	// batch control
	CUserRightsBaseNode* pAPPayBillsBatchControlNode = CreateNode("BatchControl", IDS_RIGHTS_BATCH_CONTROL, &m_stuAccountsPayableNode.PayBills.BatchControl.bTotal); // Batch Control
	pAPPayBillsNode->m_tplUserRightsNodeList.AddTail(pAPPayBillsBatchControlNode);
	pAPPayBillsBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SelectBatch", IDS_RIGHTS_SELECT_BATCH, &m_stuAccountsPayableNode.PayBills.BatchControl.bSelectBatch)); //Select Batch
	pAPPayBillsBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SetupBatch", IDS_RIGHTS_SETUP_BATCH, &m_stuAccountsPayableNode.PayBills.BatchControl.bSetupBatch)); //Setup Batch
	//********************************//
	pAPPayBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ClearPaymentsAdjustments", IDS_CLEAR_PAYMENTS_ADJUSTMENTS, &m_stuAccountsPayableNode.PayBills.bClearPaymentsAdjustments)); //Clear Payments/Adjustments
	pAPPayBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ChangeLocalTransactionDate", IDS_RIGHTS_CHANGE_LOCAL_TRANSACTION_DATE, &m_stuAccountsPayableNode.PayBills.bChangeLocalTransactionDate)); //Change Local Tran Date

	////////////////////////
	//AP Reports
	CUserRightsBaseNode* pAPReportsNode = CreateNode("APReports", IDS_CTRL_REPORTS, &m_stuAccountsPayableNode.Reports.bTotal); // Reports
	pAccountsPayableNode->m_tplUserRightsNodeList.AddTail(pAPReportsNode);
	pAPReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("QuickVendorList", IDS_QUICK_VENDOR_LIST, &m_stuAccountsPayableNode.Reports.bQuickVendorList)); //Quick Vendor List
	pAPReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("VendorLabels", IDS_VENDOR_LABELS, &m_stuAccountsPayableNode.Reports.bVendorLabels)); //Vendor Labels
	pAPReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("MasterVendor", IDS_MASTER_VENDOR, &m_stuAccountsPayableNode.Reports.bMasterVendor)); //Master Vendor
	pAPReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("Aging", IDS_AGING, &m_stuAccountsPayableNode.Reports.bAging)); //Aging
	//	pAPReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("TotalDebt", IDS_TOTAL_DEBT, &m_stuAccountsPayableNode.Reports.bTotalDebt)); //Total Debt
		//	PBI 41382	08/31/2017	mvt - Added debt report
	pAPReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("Debt", IDS_AP_DEBT, &m_stuAccountsPayableNode.Reports.bDebt));  // Debt
	pAPReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("CashRequirements", IDS_CASH_REQUIREMENTS, &m_stuAccountsPayableNode.Reports.bCashRequirements)); //Cash Requirements
	pAPReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("Overdue", IDS_OVERDUE, &m_stuAccountsPayableNode.Reports.bOverdue)); //Overdue
	pAPReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("Prepaid", IDS_PREPAID, &m_stuAccountsPayableNode.Reports.bPrepaid)); //Prepaid
	pAPReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("FutureLiability", IDS_FUTURE_LIABILITY, &m_stuAccountsPayableNode.Reports.bFutureLiability)); //Future Liability
	pAPReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("DatedInvoicePayment", IDS_DATED_INVOICE_PAYMENT, &m_stuAccountsPayableNode.Reports.bDatedInvoicePayment)); //Dated Invoice/Payment
	pAPReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("GLExpenseDistribution", IDS_GL_EXPENSE_DIST, &m_stuAccountsPayableNode.Reports.bGLExpenseDistribution)); //GL Expense Distribution
	pAPReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("History", IDS_CTRL_HISTORY, &m_stuAccountsPayableNode.Reports.bHistory)); //History
	pAPReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("MISC1099", IDS_AP_1099, &m_stuAccountsPayableNode.Reports.b1099)); //1099
	pAPReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("MagneticMedia1099MISC", IDS_ELECTRONIC_FILE_1099, &m_stuAccountsPayableNode.Reports.bElectronicFile1099)); //Electronic File 1099
	pAPReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("Control", IDS_CTRL_CONTROL, &m_stuAccountsPayableNode.Reports.bControl)); //Control

	////////////////////////
	//Period End
	CUserRightsBaseNode* pAPPeriodEndNode = CreateNode("APPeriodEnd", IDS_CTRL_PERIOD_END, &m_stuAccountsPayableNode.PeriodEnd.bTotal); //Period End
	pAccountsPayableNode->m_tplUserRightsNodeList.AddTail(pAPPeriodEndNode);
	pAPPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("ProcessPrepaidsFutures", IDS_PROCESS_PREPAID_FUTURES, &m_stuAccountsPayableNode.PeriodEnd.bProcessPrepaidsFuture)); //Process Prepaids & Futures
	pAPPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("PurgeOpenItems", IDS_CTRL_PURGE_OPEN_ITEMS, &m_stuAccountsPayableNode.PeriodEnd.bPurgeOpenItems)); //Purge Open Items
	pAPPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("PurgeHistory", IDS_CTRL_PURGE_HISTORY, &m_stuAccountsPayableNode.PeriodEnd.bPurgeHistory)); //Purge History
	pAPPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("ResetYTDVendorTotals", IDS_RESET_YTD_VENDOR_TOTALS_STR, &m_stuAccountsPayableNode.PeriodEnd.bResetYTDVendorTotals)); //Reset YTD Vendor Totals
	pAPPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("RemoveTemporaryVendors", IDS_REMOVE_TEMPORARY_VENDORS, &m_stuAccountsPayableNode.PeriodEnd.bRemoveTempVendors)); //Remove Temporary Vendors

	////////////////////////
	//Options
	CUserRightsBaseNode* pAPOptionsNode = CreateNode("APOptions", IDS_CTRL_OPTIONS, &m_stuAccountsPayableNode.Options.bTotal); //Options
	pAccountsPayableNode->m_tplUserRightsNodeList.AddTail(pAPOptionsNode);
	pAPOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ModulePreferences", IDS_CTRL_MODULE_PREFERENCES, &m_stuAccountsPayableNode.Options.bModulePreferences)); //Module Preferences
	pAPOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("APCodes", IDS_AP_CODES, &m_stuAccountsPayableNode.Options.bAPCodes)); //AP Codes
	pAPOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("UserDefinedField1", IDS_CTRL_UDF_FIELD1, &m_stuAccountsPayableNode.Options.bUserDefinedField1)); // User Defined Field1
	pAPOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("UserDefinedField2", IDS_CTRL_UDF_FIELD2, &m_stuAccountsPayableNode.Options.bUserDefinedField2)); // User Defined Field2
	pAPOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("APAllocationCodes", IDS_AP_ALLOCATIONCODES, &m_stuAccountsPayableNode.Options.bAPAllocationCodes)); //AP Allocation Codes
	pAPOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("APFrequencyCodes", IDS_FREQUENCYCODES, &m_stuAccountsPayableNode.Options.bAPFrequencyCodes)); //AP Frequency Codes
	pAPOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("SetupRecurringBills", IDS_SETUP_RECURRING_BILLS, &m_stuAccountsPayableNode.Options.bSetupRecurringBills)); //Setup Recurring Bills
	//	PBI 46406	09/26/2018	mvt - Added security for Internal Control Alerts in AP
	CUserRightsBaseNode* pAPInternalControlAlertsNode = CreateNode("ManageInternalControlAlerts", IDS_MANAGE_INTERNAL_CONTROL_ALERTS, &m_stuAccountsPayableNode.Options.bManageInternalControlAlerts); // File Maintenance
	pAPOptionsNode->m_tplUserRightsNodeList.AddTail(pAPInternalControlAlertsNode);
	pAPInternalControlAlertsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("EnterBills"), IDS_AP_MANAGEINTERNALCONTROLALERTS_ENTERBILLS, &m_stuAccountsPayableNode.Options.InternalControlAlerts.bEnterBills));
	pAPInternalControlAlertsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PayBills"), IDS_AP_MANAGEINTERNALCONTROLALERTS_PAYBILLS, &m_stuAccountsPayableNode.Options.InternalControlAlerts.bPayBills));
	// file maintenance
	CUserRightsBaseNode* pAPFileMaintenanceNode = CreateNode("FileMaintenance", IDS_CT_TABLE_MAINTENANCE, &m_stuAccountsPayableNode.Options.FileMaintenance.bTotal); // File Maintenance
	pAPOptionsNode->m_tplUserRightsNodeList.AddTail(pAPFileMaintenanceNode);
	pAPFileMaintenanceNode->m_tplUserRightsNodeList.AddTail(CreateNode("DisplayFileInformation", IDS_CTRL_DISPLAY_FILE_INFO, &m_stuAccountsPayableNode.Options.FileMaintenance.bDisplayFileInformation)); // Display File Information
	pAPFileMaintenanceNode->m_tplUserRightsNodeList.AddTail(CreateNode("RecalculateVendorBalances", IDS_RECALCULATE_VENDOR_BALANCES, &m_stuAccountsPayableNode.Options.FileMaintenance.bRecalculateVendorBalances)); // Recalculate Vendor Balances
	pAPFileMaintenanceNode->m_tplUserRightsNodeList.AddTail(CreateNode("AdjustPostedItemDueDate", IDS_ADJUST_POSTED_ITEM_DUE_DATE, &m_stuAccountsPayableNode.Options.FileMaintenance.bAdjustPostedItemDueDate)); //Adjust Posted Item Due Date

	// InTimeTec: 11.12.2012.HS - Custom Menus
	// Custom Menus Node
	pAccountsPayableNode->m_tplUserRightsNodeList.AddTail(CreateNode("CustomOptions", IDS_CUSTOM_OPTIONS, &m_stuAccountsPayableNode.bCustomOptions));
	// InTimeTec: End
}

//=================================== End Of AP ================================================================//
//==============================================================================================================//

// creates object list for the accounts receivable node
//=================================== Start Of AR ================================================================//
//================================================================================================================//
void CGBLUserRightsInformation::CreateAccountsReceivableObjectList()
{
	// creating accounts receivable node
	CUserRightsBaseNode* pAccountsReceivableNode = CreateNode("AccountsReceivable", IDS_MODULE_AR, &m_stuModulesNode.bAccountsReceivable);
	m_pUserRightsRootNode->m_tplUserRightsNodeList.AddTail(pAccountsReceivableNode);

	// Customer
	CUserRightsBaseNode* pARCustomerNode = CreateNode("Customer", IDS_CTRL_CUSTOMER, &m_stuAccountsReceivableNode.Customer.bTotal);
	pAccountsReceivableNode->m_tplUserRightsNodeList.AddTail(pARCustomerNode);

	// PBI 17468 BK 9/12/14 Change user rights to separate Add/Edit and View Only settings, Customer Master Records
	CUserRightsBaseNode* pCustomerMasterNode = CreateNode("CustomerMaster", IDS_CUSTOMER_MASTER_RECORDS, &m_stuAccountsReceivableNode.Customer.CustomerMaster.bTotal);
	pARCustomerNode->m_tplUserRightsNodeList.AddTail(pCustomerMasterNode);
	pCustomerMasterNode->m_tplUserRightsNodeList.AddTail(CreateNode("AddEditCustomer", IDS_ADD_EDIT_CUSTOMER_RECORDS, &m_stuAccountsReceivableNode.Customer.CustomerMaster.bAddEditCustomer));
	pCustomerMasterNode->m_tplUserRightsNodeList.AddTail(CreateNode("ViewCustomer", IDS_VIEW_CUSTOMER_RECORDS, &m_stuAccountsReceivableNode.Customer.CustomerMaster.bViewCustomer));
	//	PBI 32793	01/11/2016	mvt - Added Edit Payer/Payee menu item
	pCustomerMasterNode->m_tplUserRightsNodeList.AddTail(CreateNode("EditPayersPayees", IDS_EDIT_PAYERS_PAYEES, &m_stuAccountsReceivableNode.Customer.CustomerMaster.bEditPayersPayees));
	//	PBI 34919	06/02/2016	mvt - Added security for Leased Items and Prepaid Services
	pCustomerMasterNode->m_tplUserRightsNodeList.AddTail(CreateNode("EditLeasedItems", IDS_EDIT_LEASED_ITEMS, &m_stuAccountsReceivableNode.Customer.CustomerMaster.bEditLeasedItems));
	pCustomerMasterNode->m_tplUserRightsNodeList.AddTail(CreateNode("EditPrepaidServices", IDS_EDIT_PREPAID_SERVICES, &m_stuAccountsReceivableNode.Customer.CustomerMaster.bEditPrepaidServices));
	//RCG - 11/19/2020 - PBI 55792 - Added security for Bill To Other Organization checkbox on AR Customers window
	pCustomerMasterNode->m_tplUserRightsNodeList.AddTail(CreateNode("EditBillToOtherOrganization", IDS_EDIT_BILL_TO_OTHER_ORGANIZATION, &m_stuAccountsReceivableNode.Customer.CustomerMaster.bEditBillToOtherOrganization));
	//RS - 09/04/2024 - PBI 63268 - Added security to not view/edit ACH Information
	pCustomerMasterNode->m_tplUserRightsNodeList.AddTail(CreateNode("EditEFT", IDS_EDIT_EFT, &m_stuAccountsReceivableNode.Customer.CustomerMaster.bEditEFT));

	pARCustomerNode->m_tplUserRightsNodeList.AddTail(CreateNode("CopyCustomers", IDS_CTRL_COPY_CUST, &m_stuAccountsReceivableNode.Customer.bCopyCustomers));			 // Copy Customers
	pARCustomerNode->m_tplUserRightsNodeList.AddTail(CreateNode("RenumberCustomers", IDS_RENUMBER_CUSTOMERS, &m_stuAccountsReceivableNode.Customer.bRenumberCustomers)); // Renumber Customers

	//RCG - 02/28/2018 - PBI 44247 - Added Import Accounts From NeonCRM for FUND when AR or GL modules are installed
	if (g_pGBLSystemInformationCMSDll != NULL && g_pGBLSystemInformationCMSDll->CompanySettings.IsFund && (g_pGBLSystemInformationCMSDll->ModulesInstalled.GeneralLedger.IsUsable || g_pGBLSystemInformationCMSDll->ModulesInstalled.AccountsReceivable.IsUsable))
		pARCustomerNode->m_tplUserRightsNodeList.AddTail(CreateNode("ImportAccountsFromNeonCRM", IDS_INPUT_ACCOUNTS_FROM_NEONCRM, &m_stuAccountsReceivableNode.Customer.bImportAccountsFromNeonCRM));

	//RS 05/26/2022 - PBI 58628 - Added Import/Export ArcBill Customers
	//CUserRightsBaseNode* pARImportExportArcBillNode = CreateNode("ImportExportArcBill", IDS_IMPORT_EXPORT_ARC_BILL, &m_stuAccountsReceivableNode.Customer.ImportExportArcBill.bTotal); // Import/Export ArcBill Customers
	//pARCustomerNode->m_tplUserRightsNodeList.AddTail(pARImportExportArcBillNode);
	//pARImportExportArcBillNode->m_tplUserRightsNodeList.AddTail(CreateNode("ImportCustomers", IDS_IMPORT_CUSTOMERS, &m_stuAccountsReceivableNode.Customer.ImportExportArcBill.bImportCustomers)); // Import Customers
	//pARImportExportArcBillNode->m_tplUserRightsNodeList.AddTail(CreateNode("ExportCustomers", IDS_EXPORT_CUSTOMERS, &m_stuAccountsReceivableNode.Customer.ImportExportArcBill.bExportCustomers)); // Export Customers

	////////////////////////
	// transaction
	CUserRightsBaseNode* pARTransactionNode = CreateNode("Transaction", IDS_TRANSACTION, &m_stuAccountsReceivableNode.Transaction.bTotal); // Transaction
	pAccountsReceivableNode->m_tplUserRightsNodeList.AddTail(pARTransactionNode);
	pARTransactionNode->m_tplUserRightsNodeList.AddTail(CreateNode("EnterPaymentsAdjustments", IDS_CTRL_ENTER_PAYMENT_ADJUST, &m_stuAccountsReceivableNode.Transaction.bEnterPaymentsAdjustments)); // Enter Payments/Adjustments
	// generate transactions from node
	CUserRightsBaseNode* pARGenerateTransactionsFromNode = CreateNode("GenerateTransactionsFrom", IDS_CTRL_GEN_TRANS_FROM, &m_stuAccountsReceivableNode.Transaction.GenerateTransactionsFrom.bTotal); // Generate Transactions From
	pARTransactionNode->m_tplUserRightsNodeList.AddTail(pARGenerateTransactionsFromNode);
	pARGenerateTransactionsFromNode->m_tplUserRightsNodeList.AddTail(CreateNode("Recurring", IDS_CTRL_RECURRING, &m_stuAccountsReceivableNode.Transaction.GenerateTransactionsFrom.bRecurring)); // Recurring
	pARGenerateTransactionsFromNode->m_tplUserRightsNodeList.AddTail(CreateNode("ExternalFile", IDS_CTRL_EXT_FILE, &m_stuAccountsReceivableNode.Transaction.GenerateTransactionsFrom.bExternalFile)); // External File
	//RCG - 09/05/2019 - PBI 50961 - Added Generate Payments from PayPal
	pARGenerateTransactionsFromNode->m_tplUserRightsNodeList.AddTail(CreateNode("PayPal", IDS_PAYPAL, &m_stuAccountsReceivableNode.Transaction.GenerateTransactionsFrom.bPayPal)); // PayPal
	////RS 05/27/2022 - PBI 59620 - Added ArcBill
	//pARGenerateTransactionsFromNode->m_tplUserRightsNodeList.AddTail(CreateNode("ArcBill", IDS_ARC_BILL, &m_stuAccountsReceivableNode.Transaction.GenerateTransactionsFrom.bArcBill)); // ArcBill
	pARTransactionNode->m_tplUserRightsNodeList.AddTail(CreateNode("ElectronicFundsTransfer", IDS_CTRL_ELECTRN_FUND_TRANS, &m_stuAccountsReceivableNode.Transaction.bElectronicFundsTransfer)); // Electronic Funds Transfer
	pARTransactionNode->m_tplUserRightsNodeList.AddTail(CreateNode("CalculateFinanceCharges", IDS_CTRL_CALC_FINANCE_CHRGS, &m_stuAccountsReceivableNode.Transaction.bCalculateFinanceCharges)); // Calculate Finance Charges
	pARTransactionNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintEditReport", IDS_RIGHTS_PRINT_EDIT_REPORT, &m_stuAccountsReceivableNode.Transaction.bPrintEditReport)); // Print Edit Report
	// 1-33966 BK 5/10/10 PA-DSS removed credit card items
	//pARTransactionNode->m_tplUserRightsNodeList.AddTail(CreateNode("AuthorizeCreditCards", IDS_CT_AUTHORIZE_CREDIT_CARDS, &m_stuAccountsReceivableNode.Transaction.bAuthorizeCreditCards)); // Authorize Credit Cards
	pARTransactionNode->m_tplUserRightsNodeList.AddTail(CreateNode("Post", IDS_CTRL_POST, &m_stuAccountsReceivableNode.Transaction.bPost)); // Post
	// batch control
	CUserRightsBaseNode* pARBatchControlNode = CreateNode("BatchControl", IDS_CTRL_BATCH_CONTROL, &m_stuAccountsReceivableNode.Transaction.BatchControl.bTotal); // Batch Control
	pARTransactionNode->m_tplUserRightsNodeList.AddTail(pARBatchControlNode);
	pARBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SelectBatch", IDS_RIGHTS_SELECT_BATCH, &m_stuAccountsReceivableNode.Transaction.BatchControl.bSelectBatch)); // Select Batch
	pARBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SetupBatch", IDS_RIGHTS_SETUP_BATCH, &m_stuAccountsReceivableNode.Transaction.BatchControl.bSetupBatch)); // Setup Batch
	pARTransactionNode->m_tplUserRightsNodeList.AddTail(CreateNode("ClearCurrentTransactions", IDS_CTRL_CLEAR_CURR_TRANS, &m_stuAccountsReceivableNode.Transaction.bClearCurrentTransactions)); // Clear Current Transactions
	pARTransactionNode->m_tplUserRightsNodeList.AddTail(CreateNode("ChangeLocalTransactionDate", IDS_RIGHTS_CHANGE_LOCAL_TRANSACTION_DATE, &m_stuAccountsReceivableNode.Transaction.bChangeLocalTransactionDate)); // Change Local Transaction Date

	////////////////////////
	// reports
	CUserRightsBaseNode* pARReportsNode = CreateNode("Reports", IDS_CTRL_REPORTS, &m_stuAccountsReceivableNode.Reports.bTotal); // Reports
	pAccountsReceivableNode->m_tplUserRightsNodeList.AddTail(pARReportsNode);
	pARReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("CustomerQuickList", IDS_CUSTOMER_QUICK_LIST, &m_stuAccountsReceivableNode.Reports.bCustomerQuickList)); // Customer Quick List
	pARReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("CustomerLabels", IDS_CUSTOMER_LABELS, &m_stuAccountsReceivableNode.Reports.bCustomerLabels)); // Customer Labels
	// PBI 22952 BK 9/12/14 Added Customer Contacts Report
	pARReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("CustomerContacts", IDS_CUST_CONTACTS, &m_stuAccountsReceivableNode.Reports.bCustomerContacts));
	//	PBI 25449	12/2/2014	mvt - Changed CustomerMaster to CustomerMasterReport due to name collision
	pARReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("CustomerMasterReport", IDS_CUSTOMER_MASTER, &m_stuAccountsReceivableNode.Reports.bCustomerMasterReport)); // Customer Master
	//	PBI 33653	06/08/2016	mvt - Added Customer Leased Item report
	pARReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("CustomerLeasedItem", IDS_CUSTOMER_LEASED_ITEM, &m_stuAccountsReceivableNode.Reports.bCustomerLeasedItem));
	//	PBI 34414	06/13/2016	mvt - Added Prepaid Services report
	pARReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrepaidServices", IDS_PREPAID_SERVICES, &m_stuAccountsReceivableNode.Reports.bPrepaidServices));
	//	PBI 34414	06/13/2016	mvt - Added Prepaid Services report
	pARReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrepaidServicesHistory", IDS_PREPAID_SERVICES_HISTORY, &m_stuAccountsReceivableNode.Reports.bPrepaidServicesHistory));
	pARReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("CustomerStatements", IDS_CUSTOMER_STATEMENTS, &m_stuAccountsReceivableNode.Reports.bCustomerStatements)); // Customer Statements
	//	PBI 50961	09/16/2019	mvt - Added AR Automated Payment Collection
	pARReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("AutomatedPaymentCollection", IDS_AUTOMATED_PAYMENT_COLLECTION, &m_stuAccountsReceivableNode.Reports.bAutomatedPaymentCollection));
	pARReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("Collection", IDS_CTRL_COLLECTION, &m_stuAccountsReceivableNode.Reports.bCollection)); // Collection
	pARReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("AgedAnalysis", IDS_AGED_ANALYSIS, &m_stuAccountsReceivableNode.Reports.bAgedAnalysis)); // Aged Analysis
	//	PBI 32828	01/19/2016	mvt - Added Past Due report
	pARReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PastDue", IDS_PAST_DUE, &m_stuAccountsReceivableNode.Reports.bPastDue));
	pARReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("DatedInvoicePayment", IDS_DATED_INVOICE_PAYMENT, &m_stuAccountsReceivableNode.Reports.bDatedInvoicePayment)); // Dated Invoice/Payment
	pARReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("History", IDS_CTRL_HISTORY, &m_stuAccountsReceivableNode.Reports.bHistory)); // History
	//	PBI 49433	06/04/2019	mvt - Added reprint posting audit report
	pARReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ReprintPostingAudit", IDS_REPRINT_POSTING_AUDIT, &m_stuAccountsReceivableNode.Reports.bReprintPostingAudit));
	pARReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("Control", IDS_CTRL_CONTROL, &m_stuAccountsReceivableNode.Reports.bControl)); // Control

	////////////////////////
	// period end
	CUserRightsBaseNode* pARPeriodEndNode = CreateNode("PeriodEnd", IDS_CTRL_PERIOD_END, &m_stuAccountsReceivableNode.PeriodEnd.bTotal); // Period End
	pAccountsReceivableNode->m_tplUserRightsNodeList.AddTail(pARPeriodEndNode);
	pARPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("PurgeOpenItems", IDS_CTRL_PURGE_OPEN_ITEMS, &m_stuAccountsReceivableNode.PeriodEnd.bPurgeOpenItems)); // Purge Open Items
	//Defect ID 1-22943, PGP(03/09/2006)
	pARPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("ResetYTDCustomerTotals", IDS_RESET_YTD_CUSTOMER_LABELS_STRING, &m_stuAccountsReceivableNode.PeriodEnd.bResetYTDCustomerTotals)); // Reset YTD Customer Totals
	pARPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("ResetCreditCardTotals", IDS_CT_RESET_CREDIT_CARD_TOTALS, &m_stuAccountsReceivableNode.PeriodEnd.bResetCreditCardTotals)); // Reset Credit Card Totals
	pARPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("PurgeHistory", IDS_CTRL_PURGE_HISTORY, &m_stuAccountsReceivableNode.PeriodEnd.bPurgeHistory)); // Purge History

	////////////////////////
	// options
	CUserRightsBaseNode* pAROptionsNode = CreateNode("Options", IDS_CTRL_OPTIONS, &m_stuAccountsReceivableNode.Options.bTotal); // Options
	pAccountsReceivableNode->m_tplUserRightsNodeList.AddTail(pAROptionsNode);
	pAROptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ModulePreferences", IDS_CTRL_MODULE_PREFERENCES, &m_stuAccountsReceivableNode.Options.bModulePreferences)); // Module Preferences
	// codes
	CUserRightsBaseNode* pARCodesNode = CreateNode("Codes", IDS_CTRL_CODES, &m_stuAccountsReceivableNode.Options.Codes.bTotal); // Codes
	pAROptionsNode->m_tplUserRightsNodeList.AddTail(pARCodesNode);
	pARCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("ARCodes", IDS_ARCODES, &m_stuAccountsReceivableNode.Options.Codes.bARCodes)); // AR Codes
	pARCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("Terms", IDS_CTRL_TERMS, &m_stuAccountsReceivableNode.Options.Codes.bTerms)); // Terms
	pARCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("CollectionStatus", IDS_COLL_STATUS, &m_stuAccountsReceivableNode.Options.Codes.bCollectionStatus));
	pARCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("ARType", IDS_TYPE, &m_stuAccountsReceivableNode.Options.Codes.bARType)); // AR Type
	pARCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("Transaction", IDS_TRANSACTION, &m_stuAccountsReceivableNode.Options.Codes.bTransaction)); // Transaction
	pARCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("ARClerk", IDS_USERRIGHTS_ARCLERK, &m_stuAccountsReceivableNode.Options.Codes.bARClerk)); // AR Type
	// PBI 23132 BK 9/12/14 Added Customer Contacts Department Codes
	pARCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("CustomerContactsDepartment", IDS_CUST_CONTACTS_DEPT, &m_stuAccountsReceivableNode.Options.Codes.bCustomerContactsDepartment));
	pARCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("Frequency", IDS_FREQUENCYCODES, &m_stuAccountsReceivableNode.Options.Codes.bFrequencyCodes)); // Frequency
	pARCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("Recurring", IDS_CTRL_RECURRING, &m_stuAccountsReceivableNode.Options.Codes.bRecurring)); // Recurring
	pARCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("SalesPerson", IDS_CTRL_SALESPERSON, &m_stuAccountsReceivableNode.Options.Codes.bSalesPerson)); // Sales Person
	pARCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("Tax", IDS_CTRL_TAX, &m_stuAccountsReceivableNode.Options.Codes.bTax)); // Tax
	pARCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("CreditCard", IDS_CREDIT_CARD, &m_stuAccountsReceivableNode.Options.Codes.bCreditCard)); // Tax
	pARCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("UserDefinedField1", IDS_CTRL_UDF_FIELD1, &m_stuAccountsReceivableNode.Options.Codes.bUserDefinedField1)); // User Defined Field1
	pARCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("UserDefinedField2", IDS_CTRL_UDF_FIELD2, &m_stuAccountsReceivableNode.Options.Codes.bUserDefinedField2)); // User Defined Field2

	//	PBI 49433	06/03/2019	mvt - Added Manage Internal Control Alerts node
	//	options - manage internal control alerts node
	CUserRightsBaseNode* pARManageInternalControlAlerts = CreateNode("ManageInternalControlAlerts", IDS_MANAGE_INTERNAL_CONTROL_ALERTS, &m_stuAccountsReceivableNode.Options.ManageInternalControlAlerts.bManageInternalControlAlerts);
	pAROptionsNode->m_tplUserRightsNodeList.AddTail(pARManageInternalControlAlerts);
	pARManageInternalControlAlerts->m_tplUserRightsNodeList.AddTail(CreateNode("APIPosting", IDS_API_POSTING, &m_stuAccountsReceivableNode.Options.ManageInternalControlAlerts.bAPIPosting));

	// 1-33966 BK 5/10/10 PA-DSS removed credit card items
	//pARCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("CreditCardDevice", IDS_USERRIGHTS_CREDIT_CARD_DEVICE, &m_stuAccountsReceivableNode.Options.Codes.bCreditCardDevice)); // Tax
	// file maintenance
	CUserRightsBaseNode* pARFileMaintenanceNode = CreateNode("FileMaintenance", IDS_CT_TABLE_MAINTENANCE, &m_stuAccountsReceivableNode.Options.FileMaintenance.bTotal); // File Maintenance
	pAROptionsNode->m_tplUserRightsNodeList.AddTail(pARFileMaintenanceNode);
	pARFileMaintenanceNode->m_tplUserRightsNodeList.AddTail(CreateNode("DisplayFileInformation", IDS_CTRL_DISPLAY_FILE_INFO, &m_stuAccountsReceivableNode.Options.FileMaintenance.bDisplayFileInformation)); // Display File Information
	pARFileMaintenanceNode->m_tplUserRightsNodeList.AddTail(CreateNode("RecalculateMasterFile", IDS_CT_RECALCULATEMASTERTABLE, &m_stuAccountsReceivableNode.Options.FileMaintenance.bRecalculateMasterFile)); // Recalculate Master File
	CUserRightsBaseNode* pARImportExportNode = CreateNode("ImportExport", IDS_IMPORT_EXPORT, &m_stuAccountsReceivableNode.Options.FileMaintenance.ImportExport.bTotal); // File Maintenance
	pARFileMaintenanceNode->m_tplUserRightsNodeList.AddTail(pARImportExportNode);
	pARImportExportNode->m_tplUserRightsNodeList.AddTail(CreateNode("Import", IDS_IE_IMPORT, &m_stuAccountsReceivableNode.Options.FileMaintenance.ImportExport.bImport)); // Import information okay
	pARImportExportNode->m_tplUserRightsNodeList.AddTail(CreateNode("Export", IDS_IE_EXPORT, &m_stuAccountsReceivableNode.Options.FileMaintenance.ImportExport.bExport)); // Export information okay

	// InTimeTec: 11.12.2012.HS - Custom Menus
	pAccountsReceivableNode->m_tplUserRightsNodeList.AddTail(CreateNode("CustomOptions", IDS_CUSTOM_OPTIONS, &m_stuAccountsReceivableNode.bCustomOptions));
}

//=================================== End Of AR ================================================================//
//==============================================================================================================//

// creates object list for the bank reconciliation node
//=================================== Start Of BR ================================================================//
//================================================================================================================//
void CGBLUserRightsInformation::CreateBankReconciliationObjectList()
{
	// creating Bank Reconciliation node
	CUserRightsBaseNode* pBRNode = CreateNode(_T("BankReconciliation"), IDS_MODULE_BR, &m_stuModulesNode.bBankReconciliation);
	m_pUserRightsRootNode->m_tplUserRightsNodeList.AddTail(pBRNode);

	// Bank Account
	CUserRightsBaseNode* pBankAccountNode = CreateNode(_T("BankAccount"), IDS_BANKACCOUNT, &m_stuBankReconciliationNode.BankAccount.bTotal); //Bank Account
	pBRNode->m_tplUserRightsNodeList.AddTail(pBankAccountNode);
	pBankAccountNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("AccountMaintenance"), IDS_ACCOUNT_MAINTENANCE_STR, &m_stuBankReconciliationNode.BankAccount.bAccountMaintenance)); //Account Maintenance
	pBankAccountNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("BankRegister"), IDS_BR_BANK_REGISTER, &m_stuBankReconciliationNode.BankAccount.bBankRegister)); //Bank Register
	pBankAccountNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("RebuildBankAccountTotals"), IDS_REBUILD_BANK_TOTALS, &m_stuBankReconciliationNode.BankAccount.bRebuildBankAccountTotals));

	// Enter Activity
	CUserRightsBaseNode* pEnterActivityNode = CreateNode(_T("EnterActivity"), IDS_ENTER_ACTIVITY, &m_stuBankReconciliationNode.EnterActivity.bTotal);
	pBRNode->m_tplUserRightsNodeList.AddTail(pEnterActivityNode);
	pEnterActivityNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("EnterBankActivity"), IDS_ENTER_BANK_ACTIVITY, &m_stuBankReconciliationNode.EnterActivity.bEnterBankActivity));
	pEnterActivityNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("VoidBankActivity"), IDS_VOID_BANK_ACTIVITY, &m_stuBankReconciliationNode.EnterActivity.bVoidBankActivity));
	// Generate Activity From
	CUserRightsBaseNode* pGenActivityNode = CreateNode(_T("GenerateActivityFrom"), IDS_GENERATE_ACTIVITY_FROM, &m_stuBankReconciliationNode.EnterActivity.GenerateActivity.bTotal);
	pEnterActivityNode->m_tplUserRightsNodeList.AddTail(pGenActivityNode);
	pGenActivityNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("Recurring"), IDS_CTRL_RECURRING, &m_stuBankReconciliationNode.EnterActivity.GenerateActivity.bRecurring));
	pGenActivityNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ExternalFile"), IDS_CTRL_EXT_FILE, &m_stuBankReconciliationNode.EnterActivity.GenerateActivity.bExternalFile));

	pEnterActivityNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PrintEditReport"), IDS_RIGHTS_PRINT_EDIT_REPORT, &m_stuBankReconciliationNode.EnterActivity.bPrintEditReport));
	//	PBI 46406	09/27/2018	mvt - Added security for Internal Control Alerts in BR
	pEnterActivityNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("SendInternalControlAlerts"), IDS_SEND_INTERNAL_CONTROL_ALERTS, &m_stuBankReconciliationNode.EnterActivity.bSendInternalControlAlerts));
	pEnterActivityNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PrintChecks"), IDS_RIGHTS_PRINT_CHECKS, &m_stuBankReconciliationNode.EnterActivity.bPrintChecks));
	//RS - 01/30/2025 - PBI 64592 - Added Positive Pay
	pEnterActivityNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PositivePay"), IDS_POSITIVE_PAY, &m_stuBankReconciliationNode.EnterActivity.bPositivePay));
	pEnterActivityNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PrintDepositSlips"), IDS_PRINT_DEPOSIT_SLIPS, &m_stuBankReconciliationNode.EnterActivity.bPrintDepositSlips));
	pEnterActivityNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PostActivity"), IDS_POST_ACTIVITY, &m_stuBankReconciliationNode.EnterActivity.bPost));
	// Batch Control
	CUserRightsBaseNode* pBatchNode = CreateNode(_T("BatchControl"), IDS_RIGHTS_BATCH_CONTROL, &m_stuBankReconciliationNode.EnterActivity.BatchControl.bTotal);
	pEnterActivityNode->m_tplUserRightsNodeList.AddTail(pBatchNode);
	pBatchNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("SelectBatch"), IDS_RIGHTS_SELECT_BATCH, &m_stuBankReconciliationNode.EnterActivity.BatchControl.bSelectBatch));
	pBatchNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("SetupBatch"), IDS_RIGHTS_SETUP_BATCH, &m_stuBankReconciliationNode.EnterActivity.BatchControl.bSetupBatch));

	pEnterActivityNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ClearActivity"), IDS_CLEAR_ACTIVITY, &m_stuBankReconciliationNode.EnterActivity.bClearActivity));
	pEnterActivityNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ChangeLocalTransactionDate"), IDS_RIGHTS_CHANGE_LOCAL_TRANSACTION_DATE, &m_stuBankReconciliationNode.EnterActivity.bChangeLocalTransactionDate));

	// Reconciliation
	CUserRightsBaseNode* pReconciliationNode = CreateNode(_T("Reconciliation"), IDS_RECONCILIATION, &m_stuBankReconciliationNode.Reconciliation.bTotal);
	pBRNode->m_tplUserRightsNodeList.AddTail(pReconciliationNode);
	pReconciliationNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ReconcileBankStatement"), IDS_RECONCILE_BANK_STATEMENT, &m_stuBankReconciliationNode.Reconciliation.bReconcileBankStatement));
	pReconciliationNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PrintEditReport"), IDS_RIGHTS_PRINT_EDIT_REPORT, &m_stuBankReconciliationNode.Reconciliation.bPrintEditReport));
	pReconciliationNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PrintBankReconciliation"), IDS_RIGHTS_PRINT_BANK_REC, &m_stuBankReconciliationNode.Reconciliation.bPrintBankReconciliation));
	pReconciliationNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PostReconciliation"), IDS_BR_POST_RECONCILIATION, &m_stuBankReconciliationNode.Reconciliation.bPostReconciliation));
	// Batch Control
	pBatchNode = CreateNode(_T("BatchControl"), IDS_RIGHTS_BATCH_CONTROL, &m_stuBankReconciliationNode.Reconciliation.BatchControl.bTotal);
	pReconciliationNode->m_tplUserRightsNodeList.AddTail(pBatchNode);
	pBatchNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("SelectBatch"), IDS_RIGHTS_SELECT_BATCH, &m_stuBankReconciliationNode.Reconciliation.BatchControl.bSelectBatch));
	pBatchNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("SetupBatch"), IDS_RIGHTS_SETUP_BATCH, &m_stuBankReconciliationNode.Reconciliation.BatchControl.bSetupBatch));

	pReconciliationNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ClearReconciliation"), IDS_CLEAR_RECONCILIATION, &m_stuBankReconciliationNode.Reconciliation.bClearReconciliation));
	pReconciliationNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ChangeLocalTransactionDate"), IDS_RIGHTS_CHANGE_LOCAL_TRANSACTION_DATE, &m_stuBankReconciliationNode.Reconciliation.bChangeLocalTransactionDate));

	// Reports
	CUserRightsBaseNode* pReportsNode = CreateNode(_T("Reports"), IDS_CTRL_REPORTS, &m_stuBankReconciliationNode.Reports.bTotal);
	pBRNode->m_tplUserRightsNodeList.AddTail(pReportsNode);
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("BankRegister"), IDS_BR_BANK_REGISTER, &m_stuBankReconciliationNode.Reports.bRegister));
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("MissingCheck"), IDS_MISSING_CHECK, &m_stuBankReconciliationNode.Reports.bMissingCheck));
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PayeeCard"), IDS_PAYEE_CARD, &m_stuBankReconciliationNode.Reports.bPayeeCard));

	// Period End
	CUserRightsBaseNode* pPeriodEndNode = CreateNode(_T("PeriodEnd"), IDS_CTRL_PERIOD_END, &m_stuBankReconciliationNode.PeriodEnd.bTotal);
	pBRNode->m_tplUserRightsNodeList.AddTail(pPeriodEndNode);
	pPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PurgeProcessing"), IDS_PURGE_PROCESSING, &m_stuBankReconciliationNode.PeriodEnd.bPurgeProcessing));

	// Options
	CUserRightsBaseNode* pOptionsNode = CreateNode(_T("Options"), IDS_CTRL_OPTIONS, &m_stuBankReconciliationNode.Options.bTotal);
	pBRNode->m_tplUserRightsNodeList.AddTail(pOptionsNode);
	pOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ModulePreferences"), IDS_CTRL_MODULE_PREFERENCES, &m_stuBankReconciliationNode.Options.bModulePreferences));
	pOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PayeeCardFile"), IDS_PAYEE_CARD_FILE, &m_stuBankReconciliationNode.Options.bPayeeCardFile));
	pOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("CategoryCode"), IDS_CATEGORY_CODE, &m_stuBankReconciliationNode.Options.bCategoryCode));
	pOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("FrequencyCode"), IDS_FREQUENCYCODE, &m_stuBankReconciliationNode.Options.bFrequencyCode));
	pOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("SetupRecurringActivity"), IDS_SETUP_RECURRING_ACTIVITY, &m_stuBankReconciliationNode.Options.bSetupRecurringActivity));
	// PBI 46406	09/26/2018	mvt - Added security for Internal Control Alerts in BR
	pOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ManageInternalControlAlerts"), IDS_MANAGE_INTERNAL_CONTROL_ALERTS, &m_stuBankReconciliationNode.Options.bManageInternalControlAlerts));
	// 1-26150 BK 12/12/08 changed to be consistent with all other modules
	//pOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("DisplayFileInformation"), IDS_CT_DISPLAY_FILE_INFO, &m_stuBankReconciliationNode.Options.bDisplayFileInformation)) ;
	// file maintenance
	CUserRightsBaseNode* pFileMaintenanceNode = CreateNode("FileMaintenance", IDS_CT_TABLE_MAINTENANCE, &m_stuBankReconciliationNode.Options.FileMaintenance.bTotal); // File Maintenance
	pOptionsNode->m_tplUserRightsNodeList.AddTail(pFileMaintenanceNode);
	pFileMaintenanceNode->m_tplUserRightsNodeList.AddTail(CreateNode("DisplayFileInformation", IDS_CTRL_DISPLAY_FILE_INFO, &m_stuBankReconciliationNode.Options.FileMaintenance.bDisplayFileInformation)); // Display File Information

	// InTimeTec: 11.12.2012.HS - Custom Menus
	// Custom Menus Node
	pBRNode->m_tplUserRightsNodeList.AddTail(CreateNode("CustomOptions", IDS_CUSTOM_OPTIONS, &m_stuBankReconciliationNode.bCustomOptions));
	// InTimeTec: End
}

//=================================== End Of BR ================================================================//
//==============================================================================================================//

// creates object list for the controller node
//=================================== Start Of CT ================================================================//
//================================================================================================================//
void CGBLUserRightsInformation::CreateControllerObjectList()
{
	// creating controller node
	CUserRightsBaseNode* pControllerNode = CreateNode("Controller", IDS_MODULE_CT, &m_stuModulesNode.bController); // Controller
	m_pUserRightsRootNode->m_tplUserRightsNodeList.AddTail(pControllerNode);
	////////////////////////
	// controller - company
	CUserRightsBaseNode* pCTCompanyNode = CreateNode("Company", IDS_COMPANY, &m_stuControllerNode.Company.bTotal); // Company
	pControllerNode->m_tplUserRightsNodeList.AddTail(pCTCompanyNode);
	pCTCompanyNode->m_tplUserRightsNodeList.AddTail(CreateNode("SetupSelectCompany", IDS_RIGHTS_SETUP_SELECT_COMPANY, &m_stuControllerNode.Company.bSetupSelectCompany)); // Setup/Select Company
	pCTCompanyNode->m_tplUserRightsNodeList.AddTail(CreateNode("InstallModules", IDS_INSTALL_MODULES, &m_stuControllerNode.Company.bInstallModules)); // Install Modules
	pCTCompanyNode->m_tplUserRightsNodeList.AddTail(CreateNode("ChangeDate", IDS_CT_CHANGE_DATE, &m_stuControllerNode.Company.bChangeDate)); // Change Date
	pCTCompanyNode->m_tplUserRightsNodeList.AddTail(CreateNode("FiscalCalendar", IDS_FISCAL_CALENDAR, &m_stuControllerNode.Company.bFiscalCalendar)); // Fiscal Calendar
	// PBI 17783 BK 3/11/14 added new Merchant Warehouse setup form
	pCTCompanyNode->m_tplUserRightsNodeList.AddTail(CreateNode("MerchantWare", IDS_MERCHANT_WAREHOUSE_SETUP, &m_stuControllerNode.Company.bMerchantWarehouse)); // MerchantWare Setup
	//RS - 04/26/2024 - PBI 63797 - Added user security and menu item to Organization menu
	pCTCompanyNode->m_tplUserRightsNodeList.AddTail(CreateNode("CardPointe", IDS_CARDPOINTE_SETUP, &m_stuControllerNode.Company.bCloverConnect));
	//	PBI 45155	06/06/2018	mvt - Added security for Denali BI Setup
	pCTCompanyNode->m_tplUserRightsNodeList.AddTail(CreateNode("DenaliBISetup", IDS_DENALI_BI_SETUP, &m_stuControllerNode.Company.bDenaliBISetup)); // Denali BI Setup
	//	PBI 52317	11/19/2019	mvt - Added security for Donately integration
	pCTCompanyNode->m_tplUserRightsNodeList.AddTail(CreateNode("DonatelyIntegrationSetup", IDS_SET_UP_DONATELY_INTEGRATION, &m_stuControllerNode.Company.bDonatelyIntegrationSetup));
	//	PBI 47520	12/04/2018	mvt - Added security for Yodlee integration setup
	if (g_pGBLSystemInformationCMSDll != NULL &&
		g_pGBLSystemInformationCMSDll->ApplicationSettings.YodleeUsable)
		pCTCompanyNode->m_tplUserRightsNodeList.AddTail(CreateNode("YodleeIntegrationSetup", IDS_SET_UP_YODLEE_INTEGRATION, &m_stuControllerNode.Company.bYodleeIntegrationSetup)); // Yodlee Setup
	//	PBI 50961	09/04/2019	mvt - Added security for PayPal integration setup
	pCTCompanyNode->m_tplUserRightsNodeList.AddTail(CreateNode("PayPalIntegrationSetup", IDS_SET_UP_PAYPAL_INTEGRATION, &m_stuControllerNode.Company.bPayPalIntegrationSetup)); // PayPal Setup
	//	PBI 46406	09/26/2018	mvt - Added security for Internal Control Alerts in Controller
	pCTCompanyNode->m_tplUserRightsNodeList.AddTail(CreateNode("InternalControlAlertsSetup", IDS_CT_INTERNALCONTROLALERTSSETUP_STATUS, &m_stuControllerNode.Company.bInternalControlAlertsSetup));
	//RCG - 02/28/2018 - PBI 44247 - Added NeonCRM Setup for FUND when AR or GL modules are installed
	if (g_pGBLSystemInformationCMSDll != NULL && g_pGBLSystemInformationCMSDll->CompanySettings.IsFund && (g_pGBLSystemInformationCMSDll->ModulesInstalled.AccountsReceivable.IsUsable || g_pGBLSystemInformationCMSDll->ModulesInstalled.GeneralLedger.IsUsable))
		pCTCompanyNode->m_tplUserRightsNodeList.AddTail(CreateNode("NeonCRM", IDS_NEONCRM_SETUP, &m_stuControllerNode.Company.bNeonCRM)); //NeonCRM Setup

	// 1-35038 BK 11/11/10 set node for electronic constants menu item when AP or PR modules are installed
	if (g_pGBLSystemInformationCMSDll != NULL)
		if (g_pGBLSystemInformationCMSDll->ModulesInstalled.AccountsPayable.IsUsable ||
			g_pGBLSystemInformationCMSDll->ModulesInstalled.Payroll.IsUsable)
			pCTCompanyNode->m_tplUserRightsNodeList.AddTail(CreateNode("MagneticMediaConstants", IDS_CTRL_MAGNETIC_MEDIA_CONST, &m_stuControllerNode.Company.bMagneticMediaConstants)); // Magnetic Media Constants

	// controller - security node
	CUserRightsBaseNode* pCTSecurityNode = CreateNode("Security", IDS_CTRL_SECURITY, &m_stuControllerNode.Security.bTotal); // Security
	pControllerNode->m_tplUserRightsNodeList.AddTail(pCTSecurityNode);
	pCTSecurityNode->m_tplUserRightsNodeList.AddTail(CreateNode("AddChangeUsers", IDS_RIGHTS_ADD_CHANGE_USERS, &m_stuControllerNode.Security.bAddChangeUsers)); // Add/Change Users
	// 1-6113, DTG, added groups to the tree and struct.
	pCTSecurityNode->m_tplUserRightsNodeList.AddTail(CreateNode("AddChangeGroups", IDS_RIGHTS_ADD_CHANGE_GROUPS, &m_stuControllerNode.Security.bAddChangeGroups)); // Add/Change Groups
	pCTSecurityNode->m_tplUserRightsNodeList.AddTail(CreateNode("ChangePassword", IDS_CHANGE_PASSWORD, &m_stuControllerNode.Security.bChangePassword)); // Change Password
	pCTSecurityNode->m_tplUserRightsNodeList.AddTail(CreateNode("ResetPassword", IDS_CT_CISP_CAPTION_RESETPASSWORD, &m_stuControllerNode.Security.bResetPassword)); // Reset Password
	//1-9514, PGP(01/02/2004)
	pCTSecurityNode->m_tplUserRightsNodeList.AddTail(CreateNode("CopyUsersFromCompany", IDS_SECURITY_COPY_USERS_COMPANY, &m_stuControllerNode.Security.bCopyUsersFromCompany)); // Copy users from company
	pCTSecurityNode->m_tplUserRightsNodeList.AddTail(CreateNode("CopyGroupsFromCompany", IDS_SECURITY_COPY_GROUPS_COMPANY, &m_stuControllerNode.Security.bCopyGroupsFromCompany)); // Copy groups from company

	////////////////////////
	// controller - reports (1-33827 BK 11/24/09 added reports to security)
	CUserRightsBaseNode* pCTReports = CreateNode("Reports", IDS_CTRL_REPORTS, &m_stuControllerNode.Reports.bTotal);
	pControllerNode->m_tplUserRightsNodeList.AddTail(pCTReports);
	CUserRightsBaseNode* pIntegration = CreateNode("Integration", IDS_CA_MDI_INTEGRATION, &m_stuControllerNode.Reports.Integration.bTotal);
	pCTReports->m_tplUserRightsNodeList.AddTail(pIntegration);
	pIntegration->m_tplUserRightsNodeList.AddTail(CreateNode("IntegrationErrorReport", IDS_RGT_INTEG_ERROR_RPT, &m_stuControllerNode.Reports.Integration.bIntegErrorReport));
	pIntegration->m_tplUserRightsNodeList.AddTail(CreateNode("RemoveIntegrationReports", IDS_RGT_REMOVE_INTEG_RPTS, &m_stuControllerNode.Reports.Integration.bRemoveIntegReports));
	CUserRightsBaseNode* pConversion = CreateNode("Conversion", IDS_RGT_CONVERSION, &m_stuControllerNode.Reports.Conversion.bTotal);
	pCTReports->m_tplUserRightsNodeList.AddTail(pConversion);
	pConversion->m_tplUserRightsNodeList.AddTail(CreateNode("PreConversionDataChecklist", IDS_TITLE_PRE_CONVERSION_DATA_CHECKLIST, &m_stuControllerNode.Reports.Conversion.bPreConversionDataChklst));
	pConversion->m_tplUserRightsNodeList.AddTail(CreateNode("CodesValuesAddedReport", IDS_RGT_CODESVALUES_ADDED_RPT, &m_stuControllerNode.Reports.Conversion.bCodesValuesAddedReport));
	pConversion->m_tplUserRightsNodeList.AddTail(CreateNode("IntegrationStatusReport", IDS_RGT_INTEG_STATUS_RPT, &m_stuControllerNode.Reports.Conversion.bIntegrationStatusReport));
	pConversion->m_tplUserRightsNodeList.AddTail(CreateNode("RemoveConversionReports", IDS_RGT_REMOVE_CONV_RPT, &m_stuControllerNode.Reports.Conversion.bRemoveConverionReport));
	// PBI 22186 BK 8/19/14 Added red flag reports
	pCTReports->m_tplUserRightsNodeList.AddTail(CreateNode("RedFlagReports", IDS_RED_FLAG_REPORTS, &m_stuControllerNode.Reports.bRedFlagReports));

	////////////////////////
	// controller - options
	CUserRightsBaseNode* pCTOptionsNode = CreateNode("Options", IDS_CTRL_OPTIONS, &m_stuControllerNode.Options.bTotal); // Options
	pControllerNode->m_tplUserRightsNodeList.AddTail(pCTOptionsNode);

	// InTimeTec: 09.04.2013.HS - Controller Module Preferences
	CUserRightsBaseNode* pModulePreferences = CreateNode("ModulePrefrences", IDS_CTRL_MODULE_PREFERENCES, &m_stuControllerNode.Options.ModulePreference.bTotal);
	pCTOptionsNode->m_tplUserRightsNodeList.AddTail(pModulePreferences);

	pModulePreferences->m_tplUserRightsNodeList.AddTail(CreateNode("General", IDS_GENERAL, &m_stuControllerNode.Options.ModulePreference.bGeneral));

	CUserRightsBaseNode* pCustomOptions = CreateNode("CustomOptions", IDS_CUSTOM_OPTIONS, &m_stuControllerNode.Options.ModulePreference.CustomOptions.bTotal);
	pModulePreferences->m_tplUserRightsNodeList.AddTail(pCustomOptions);
	pCustomOptions->m_tplUserRightsNodeList.AddTail(CreateNode("CustomMenus", IDS_CUSTOM_MENUS, &m_stuControllerNode.Options.ModulePreference.CustomOptions.bCustomMenus));
	pCustomOptions->m_tplUserRightsNodeList.AddTail(CreateNode("CustomReports", IDS_CUSTOM_REPORTS, &m_stuControllerNode.Options.ModulePreference.CustomOptions.bCustomReports));

	pModulePreferences->m_tplUserRightsNodeList.AddTail(CreateNode("PrintOptions", IDS_REPORTPRINTOPTIONS, &m_stuControllerNode.Options.ModulePreference.bPrintOptions));
	// InTimeTec: End

	// I-6118 & 6119 By HK on 07/15/2003 Added Two New Nodes 1.Currencies    2.Exchange Rates
	pCTOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("Currencies", IDS_CAPTION_CURRENCIES, &m_stuControllerNode.Options.bCurrencies)); // Display Currencies
	pCTOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ExchangeRates", IDS_EXCHANGE_RATES, &m_stuControllerNode.Options.bExchangeRates)); // Display Exchange Rates
	// 1-6116,1-6114, DTG, options - table maintenance, remove File Maintenance branch.
	pCTOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("DisplayFileInformation", IDS_CTRL_DISPLAY_FILE_INFO, &m_stuControllerNode.Options.bFileMaintenance)); // Display File Information
	pCTOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ProcessPayrollTransactions", IDS_PAY_PROCESS_PAYROLL, &m_stuControllerNode.Options.bProcessPayrollTransactions)); // begbert 03-12-2010 Process Payroll Transactions : Added security node
	// Error: I-6117 By HK on 07/08/2003: Not to show ReindexDataFiles.
	CUserRightsBaseNode* pBackupCompany = CreateNode("BackupCompany", IDS_BACKUP_COMPANY, &m_stuControllerNode.Options.BackupCompany.bTotal); // Backup Company
	pCTOptionsNode->m_tplUserRightsNodeList.AddTail(pBackupCompany);
	pBackupCompany->m_tplUserRightsNodeList.AddTail(CreateNode("Backup", IDS_BACK_UP, &m_stuControllerNode.Options.BackupCompany.bBackup)); // Backup
	pBackupCompany->m_tplUserRightsNodeList.AddTail(CreateNode("Restore", IDS_RESTORE, &m_stuControllerNode.Options.BackupCompany.bRestore)); // Restore

	// InTimeTec: 03.05.2013.HS - Copy Organization Utility
	pCTOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("CopyOrganization", IDS_COPY_ORGANIZATION_UTILITY, &m_stuControllerNode.Options.bCopyOrganizationUtility)); // Backup Company
	// InTimeTec: End

	// InTimeTec: 11.12.2012.HS - Custom Menus
	// Custom Menus Node
	pControllerNode->m_tplUserRightsNodeList.AddTail(CreateNode("CustomOptions", IDS_CUSTOM_OPTIONS, &m_stuControllerNode.bCustomOptions));
	// InTimeTec: End
}

//=================================== End Of CT ================================================================//
//==============================================================================================================//

// creates object tree for the general ledger
//=================================== Start Of GL ================================================================//
//================================================================================================================//
void CGBLUserRightsInformation::CreateGeneralLedgerObjectList()
{
	bool bFundOrg = false;
	if (g_pGBLSystemInformationCMSDll != NULL && g_pGBLSystemInformationCMSDll->CompanySettings.IsFund)
		bFundOrg = true;

	// General Ledger
	CUserRightsBaseNode* pGeneralLedgerNode = CreateNode("GeneralLedger", IDS_MODULE_GL, &m_stuModulesNode.bGeneralLedger);
	m_pUserRightsRootNode->m_tplUserRightsNodeList.AddTail(pGeneralLedgerNode);

	// general ledger - account
	CUserRightsBaseNode* pGLAccountNode = CreateNode("Account", IDS_GL_ACCOUNT, &m_stuGeneralLedgerNode.Account.bTotal); // Account
	pGeneralLedgerNode->m_tplUserRightsNodeList.AddTail(pGLAccountNode);

	// PBI 17468 BK 9/12/14 Change user rights to separate Add/Edit and View Only settings, Account Master Records
	CUserRightsBaseNode* pAccountMasterNode = CreateNode("AccountMaster", IDS_ACCOUNT_MASTER_RECORDS, &m_stuGeneralLedgerNode.Account.AccountMaster.bTotal);
	pGLAccountNode->m_tplUserRightsNodeList.AddTail(pAccountMasterNode);
	pAccountMasterNode->m_tplUserRightsNodeList.AddTail(CreateNode("AddEditAccount", IDS_ADD_EDIT_ACCOUNT_RECORDS, &m_stuGeneralLedgerNode.Account.AccountMaster.bAddEditAccount));
	pAccountMasterNode->m_tplUserRightsNodeList.AddTail(CreateNode("ViewAccount", IDS_VIEW_ACCOUNT_RECORDS, &m_stuGeneralLedgerNode.Account.AccountMaster.bViewAccount));
	//pGLAccountNode->m_tplUserRightsNodeList.AddTail(CreateNode("LedgerAccountMaintenance", IDS_CTRL_LDGR_ACCT_MAINTN, &m_stuGeneralLedgerNode.Account.bLedgerAccountMaintenance)); //Ledger Account Maintenance
	//pGLAccountNode->m_tplUserRightsNodeList.AddTail(CreateNode("LAMInquiryOnly", IDS_CTRL_LAM_INQ, &m_stuGeneralLedgerNode.Account.bLAMInquiryOnly)); // LAM Inquiry Only
	//RCG - 01/26/2021 - PBI 55792 - Added Cost Centers
	if (g_pGBLSystemInformationCMSDll != NULL && g_pGBLSystemInformationCMSDll->ModulesInstalled.CostCenter.IsUsable)
		pGLAccountNode->m_tplUserRightsNodeList.AddTail(CreateNode("CostCenters", IDS_COST_CENTERS, &m_stuGeneralLedgerNode.Account.bCostCenters));
	pGLAccountNode->m_tplUserRightsNodeList.AddTail(CreateNode("SetupGLAccountGroups", IDS_SETUPGL_ACCOUNT_GROUPS, &m_stuGeneralLedgerNode.Account.bSetupGLAccountGroups));		// Setup GL Account Groups
	pGLAccountNode->m_tplUserRightsNodeList.AddTail(CreateNode("GlobalBudgetUpdate", IDS_GL_BUDGET_MANAGEMENT, &m_stuGeneralLedgerNode.Account.bGlobalBudgetUpdate));			// Budget Management (stored as "GlobalBudgetUpdate" for legacy reasons)
	//	PBI 47610	03/11/2019	mvt - Added option to Manage Budgets/Expense Controls
	pGLAccountNode->m_tplUserRightsNodeList.AddTail(CreateNode("ManageBudgetsExpenseControls", IDS_MANAGE_BUDGETS_EXPENSE_CONTROLS, &m_stuGeneralLedgerNode.Account.bManageBudgetsExpenseControls));
	pGLAccountNode->m_tplUserRightsNodeList.AddTail(CreateNode("CopyChartOfAccounts", IDS_CTRL_COPY_CHART_OF_ACCTS, &m_stuGeneralLedgerNode.Account.bCopyChartOfAccounts));		// Copy Chart Of Accounts
	pGLAccountNode->m_tplUserRightsNodeList.AddTail(CreateNode("RenumberGLAccounts", IDS_RENUMBER_GL_ACCOUNTS, &m_stuGeneralLedgerNode.Account.bRenumberGLAccounts));			// Renumber GL Accounts
	pGLAccountNode->m_tplUserRightsNodeList.AddTail(CreateNode("AdjustAccountSegments", IDS_CTRL_ADJUST_ACCT_SEG, &m_stuGeneralLedgerNode.Account.bAdjustAccountSegments));		// Adjust Account Segments

	////////////////////////
	// general ledger - journal
	//CUserRightsBaseNode *pGLJournalNode = CreateNode("Journal",IDS_CTRL_JOURNAL,NULL); // Journal
	// EK - 07/22/03 - 1-6245 - Pass in pointer to general Journal menu flag
	CUserRightsBaseNode* pGLJournalNode = CreateNode("Journal", IDS_CTRL_JOURNAL, &m_stuGeneralLedgerNode.Journal.bTotal); // Journal
	pGeneralLedgerNode->m_tplUserRightsNodeList.AddTail(pGLJournalNode);
	pGLJournalNode->m_tplUserRightsNodeList.AddTail(CreateNode("JournalEntry", IDS_CTRL_JOURNAL_ENTRY, &m_stuGeneralLedgerNode.Journal.bJournalEntry)); // Journal Entry
	pGLJournalNode->m_tplUserRightsNodeList.AddTail(CreateNode("ProcessAllocations", IDS_CTRL_PROC_ALLOCATIONS, &m_stuGeneralLedgerNode.Journal.bProcessAllocations)); // Process Allocations
	CUserRightsBaseNode* pGLGenerateJournalEntriesFromNode = CreateNode("GenerateJournalEntriesFrom", IDS_GENERATE_JOURNAL_ENTRIES_FROM, &m_stuGeneralLedgerNode.Journal.GenerateJournalEntriesFrom.bTotal); // Generate Journal Entries From
	pGLJournalNode->m_tplUserRightsNodeList.AddTail(pGLGenerateJournalEntriesFromNode);
	pGLGenerateJournalEntriesFromNode->m_tplUserRightsNodeList.AddTail(CreateNode("Recurring", IDS_CTRL_RECURRING, &m_stuGeneralLedgerNode.Journal.GenerateJournalEntriesFrom.bRecurring)); // recurring
	pGLGenerateJournalEntriesFromNode->m_tplUserRightsNodeList.AddTail(CreateNode("ExternalFile", IDS_CTRL_EXT_FILE, &m_stuGeneralLedgerNode.Journal.GenerateJournalEntriesFrom.bExternalFile)); // external file
	//RCG - 02/28/2018 - PBI 44247 - Added Import Donations From NeonCRM for FUND when AR or GL modules are installed
	if (bFundOrg && (g_pGBLSystemInformationCMSDll->ModulesInstalled.GeneralLedger.IsUsable || g_pGBLSystemInformationCMSDll->ModulesInstalled.AccountsReceivable.IsUsable))
		pGLGenerateJournalEntriesFromNode->m_tplUserRightsNodeList.AddTail(CreateNode("NeonCRM", IDS_NEONCRM, &m_stuGeneralLedgerNode.Journal.GenerateJournalEntriesFrom.bNeonCRM));
	//	PBI 52439	11/14/2019	mvt - Added security when importing donations from Facebook
	pGLGenerateJournalEntriesFromNode->m_tplUserRightsNodeList.AddTail(CreateNode("Facebook", IDS_FACEBOOK_DONATION_FILE, &m_stuGeneralLedgerNode.Journal.GenerateJournalEntriesFrom.bFacebook));
	//	PBI 52317	11/19/2019	mvt - Added security when importing donations from Donately
	pGLGenerateJournalEntriesFromNode->m_tplUserRightsNodeList.AddTail(CreateNode("Donately", IDS_DONATELY, &m_stuGeneralLedgerNode.Journal.GenerateJournalEntriesFrom.bDonately));
	if (bFundOrg)
		pGLJournalNode->m_tplUserRightsNodeList.AddTail(CreateNode("GenerateDueToDueFromEntries", IDS_STATUS_MENU_GL_GENERATE_DUE_TO, &m_stuGeneralLedgerNode.Journal.bGenerateDueToDueFromEntries));
	pGLJournalNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintEditReport", IDS_MSG_PRINT_EDIT_REPORT, &m_stuGeneralLedgerNode.Journal.bPrintEditReport)); // Print Edit Report
	pGLJournalNode->m_tplUserRightsNodeList.AddTail(CreateNode("PostJournalEntries", IDS_CTRL_POST_JOURNAL_ENTRIES, &m_stuGeneralLedgerNode.Journal.bPostJournalEntries)); // Post Journal Entries
	pGLJournalNode->m_tplUserRightsNodeList.AddTail(CreateNode("AllowOutOfBalancePosting", IDS_ALLOW_OUT_OF_BALANCE_POSTING, &m_stuGeneralLedgerNode.Journal.bAllowOutOfBalancePosting)); // Allow Out Of Balance Posting
	m_stuGeneralLedgerNode.Journal.bAllowOutOfBalancePosting = false;
	// journal - batch control node
	CUserRightsBaseNode* pGLBatchControlNode = CreateNode("BatchControl", IDS_CTRL_BATCH_CONTROL, &m_stuGeneralLedgerNode.Journal.BatchControl.bTotal); // Batch Control
	pGLJournalNode->m_tplUserRightsNodeList.AddTail(pGLBatchControlNode);
	pGLBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SelectBatch", IDS_SELECTBATCH, &m_stuGeneralLedgerNode.Journal.BatchControl.bSelectBatch)); // Select Batch
	pGLBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SetupBatch", IDS_CTRL_SETUP_BATCH, &m_stuGeneralLedgerNode.Journal.BatchControl.bSetupBatch)); // Setup Batch
	pGLJournalNode->m_tplUserRightsNodeList.AddTail(CreateNode("ClearJournalEntry", IDS_CTRL_CLEAR_JOURNAL_ENTRY, &m_stuGeneralLedgerNode.Journal.bClearJournalEntry)); // Clear Journal Entry
	pGLJournalNode->m_tplUserRightsNodeList.AddTail(CreateNode("ChangeLocalTransactionDate", IDS_CT_CLT_DATE, &m_stuGeneralLedgerNode.Journal.bChangeLocalTransactionDate)); // Change Local Transaction Date
	////////////////////////
	// general ledger - reports
	CUserRightsBaseNode* pGLReportNode = CreateNode("Reports", IDS_CTRL_REPORTS, &m_stuGeneralLedgerNode.Report.bTotal); // Reports
	pGeneralLedgerNode->m_tplUserRightsNodeList.AddTail(pGLReportNode);
	pGLReportNode->m_tplUserRightsNodeList.AddTail(CreateNode("ChartOfAccounts", IDS_CTRL_CHART_OF_ACCTS, &m_stuGeneralLedgerNode.Report.bChartOfAccounts)); // Chart Of Accounts
	pGLReportNode->m_tplUserRightsNodeList.AddTail(CreateNode("AuditTrail", IDS_CTRL_AUDIT_TRAIL, &m_stuGeneralLedgerNode.Report.bAuditTrail)); // Audit Trail
	pGLReportNode->m_tplUserRightsNodeList.AddTail(CreateNode("TrialBalance", IDS_CTRL_TRIAL_BALANCE, &m_stuGeneralLedgerNode.Report.bTrialBalance)); // Trial Balance
	pGLReportNode->m_tplUserRightsNodeList.AddTail(CreateNode("BalanceSheet", IDS_CTRL_BALANCE_SHEET, &m_stuGeneralLedgerNode.Report.bBalanceSheet)); // Balance Sheet

	if (bFundOrg)
	{
		pGLReportNode->m_tplUserRightsNodeList.AddTail(CreateNode("RevenueExpense", IDS_GL_REVENUE_AND_EXPENSE, &m_stuGeneralLedgerNode.Report.bRevenueExpense));
		//RCG - 03/01/2021 - PBI 56230 - Added Cost Center
		if (g_pGBLSystemInformationCMSDll->ModulesInstalled.CostCenter.IsUsable)
			pGLReportNode->m_tplUserRightsNodeList.AddTail(CreateNode("CostCenterRevenueExpense", IDS_COST_CENTER_REVENUE_AND_EXPENSE, &m_stuGeneralLedgerNode.Report.bCostCenterRevenueAndExpense)); // Cost Center Revenue and Expense
		// FASB Reports for fund
		CUserRightsBaseNode* pFASBReports = CreateNode("FASBReports", IDS_CTRL_FABS_REPORT, &m_stuGeneralLedgerNode.Report.FASBReports.bTotal);
		pGLReportNode->m_tplUserRightsNodeList.AddTail(pFASBReports);
		pFASBReports->m_tplUserRightsNodeList.AddTail(CreateNode("StatementActivities ", IDS_CTRL_STATEMENT_OF_ACTIVITIES, &m_stuGeneralLedgerNode.Report.FASBReports.bStatementActivities));
		pFASBReports->m_tplUserRightsNodeList.AddTail(CreateNode("StatementCashFlows", IDS_CTRL_STATEMENT_OF_CASH_FLOWS, &m_stuGeneralLedgerNode.Report.FASBReports.bStatementCashFlows));
		pFASBReports->m_tplUserRightsNodeList.AddTail(CreateNode("StatementFinancialPos", IDS_CTRL_STATEMENT_OF_FIN_POS, &m_stuGeneralLedgerNode.Report.FASBReports.bStatementFinancialPosition));
		//	PBI 51151	08/27/2019	mvt - Added Statement of Functional Expenses
		pFASBReports->m_tplUserRightsNodeList.AddTail(CreateNode("StatementOfFunctionalExpenses", IDS_REPORTS_STATEMENT_OF_FUNCTIONAL_EXPENSES, &m_stuGeneralLedgerNode.Report.FASBReports.bStatementOfFunctionalExpenses));
	}
	else
	{
		pGLReportNode->m_tplUserRightsNodeList.AddTail(CreateNode("IncomeStatement", IDS_INCOME_STATEMENT, &m_stuGeneralLedgerNode.Report.bIncomeStatement)); // Income Statement
		//RCG - 02/02/2021 - PBI 56230 - Added Cost Center
		if (g_pGBLSystemInformationCMSDll != NULL && g_pGBLSystemInformationCMSDll->ModulesInstalled.CostCenter.IsUsable)
			pGLReportNode->m_tplUserRightsNodeList.AddTail(CreateNode("CostCenterIncomeStatement", IDS_COST_CENTER_INCOME_STATEMENT, &m_stuGeneralLedgerNode.Report.bCostCenterIncomeStatement)); // Cost Center Income Statement
		pGLReportNode->m_tplUserRightsNodeList.AddTail(CreateNode("CashFlow", IDS_CASHFLOW, &m_stuGeneralLedgerNode.Report.bCashFlow)); // Cash Flow
	}
	pGLReportNode->m_tplUserRightsNodeList.AddTail(CreateNode("BudgetPerformance", IDS_BUDGET_PERFORMANCE, &m_stuGeneralLedgerNode.Report.bBudgetPerformance)); // Budget Performance
	pGLReportNode->m_tplUserRightsNodeList.AddTail(CreateNode("BudgetReports", IDS_BUDGET_REPORTS, &m_stuGeneralLedgerNode.Report.bBudgetReports)); // Budget Reports
	pGLReportNode->m_tplUserRightsNodeList.AddTail(CreateNode("BudgetTracking", IDS_BUDGET_TRACKING_REPORT, &m_stuGeneralLedgerNode.Report.bBudgetTracking)); //	PBI 47610	03/05/2019	mvt - Added budget tracking report.
	//	PBI 51105	01/23/2020	mvt - Added Missing Accounts report.
	pGLReportNode->m_tplUserRightsNodeList.AddTail(CreateNode("MissingAccounts", IDS_MISSING_ACCOUNTS_REPORT, &m_stuGeneralLedgerNode.Report.bMissingAccounts));
	pGLReportNode->m_tplUserRightsNodeList.AddTail(CreateNode("ReprintPosting", IDS_CTRL_REPRINT_POSTING, &m_stuGeneralLedgerNode.Report.bReprintPosting)); // Reprint Posting
	pGLReportNode->m_tplUserRightsNodeList.AddTail(CreateNode("SpreadsheetExport", IDS_CTRL_SPREADSHEET_EXPORT, &m_stuGeneralLedgerNode.Report.bSpreadsheetExport)); // Spreadsheet Export
	// Error I-8087: 09/23/2003  HK, Changed String ID to "IDS_CTRL_CONSOLIDATED_LDGRS"
	if (!bFundOrg)
		pGLReportNode->m_tplUserRightsNodeList.AddTail(CreateNode("ConsolidateLedgers", IDS_CTRL_CONSOLIDATED_LDGRS, &m_stuGeneralLedgerNode.Report.bConsolidateLedgers)); // Consolidate Ledgers
	////////////////////////
	// general ledger - year end
	CUserRightsBaseNode* pGLYearEndNode = CreateNode("YearEnd", IDS_YEAR_END, &m_stuGeneralLedgerNode.YearEnd.bTotal); // Year End
	pGeneralLedgerNode->m_tplUserRightsNodeList.AddTail(pGLYearEndNode);
	pGLYearEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("PurgeHistory", IDS_CTRL_PURGE_HISTORY, &m_stuGeneralLedgerNode.YearEnd.bPurgeHistory)); // Purge History
	if (bFundOrg)
	{
		pGLYearEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("CloseFundYear", IDS_CTRL_CLOSE_FUND_YEAR, &m_stuGeneralLedgerNode.YearEnd.bCloseFundYear));
		pGLYearEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("CloseOrgYear", IDS_CTRL_CLOSE_ORG, &m_stuGeneralLedgerNode.YearEnd.bCloseOrganizationYear));
	}
	else
		pGLYearEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("CloseYear", IDS_CLOSE_YEAR, &m_stuGeneralLedgerNode.YearEnd.bCloseYear)); // Close Year
	////////////////////////
	// general ledger - options
	CUserRightsBaseNode* pGLOptions = CreateNode("Options", IDS_CTRL_OPTIONS, NULL); // Options
	pGeneralLedgerNode->m_tplUserRightsNodeList.AddTail(pGLOptions);
	pGLOptions->m_tplUserRightsNodeList.AddTail(CreateNode("ModulePreferences", IDS_CTRL_MODULE_PREFERENCES, &m_stuGeneralLedgerNode.Options.bModulePreferences)); // Module Preferences
	pGLOptions->m_tplUserRightsNodeList.AddTail(CreateNode("GLInterfaceCodes", IDS_GLINTERFACECODES, &m_stuGeneralLedgerNode.Options.bInterfaceCodes)); // GL Interface Codes
	if (bFundOrg)
		pGLOptions->m_tplUserRightsNodeList.AddTail(CreateNode("SetUpFund", IDS_CTRL_SETUP_FUND, &m_stuGeneralLedgerNode.Options.bSetupFund));

	// options - codes node
	CUserRightsBaseNode* pGLCodes = CreateNode("Codes", IDS_CTRL_CODES, &m_stuGeneralLedgerNode.Options.Codes.bTotal);
	pGLOptions->m_tplUserRightsNodeList.AddTail(pGLCodes);
	pGLCodes->m_tplUserRightsNodeList.AddTail(CreateNode("FrequencyCodes", IDS_FREQUENCYCODES, &m_stuGeneralLedgerNode.Options.Codes.bFrequencyCodes)); //Frequency Codes
	pGLCodes->m_tplUserRightsNodeList.AddTail(CreateNode("AllocationCodes", IDS_CTRL_ALLOCATION_CODES, &m_stuGeneralLedgerNode.Options.Codes.bAllocationCodes)); // Allocation Codes
	pGLCodes->m_tplUserRightsNodeList.AddTail(CreateNode("AdvancedReportGroupCodes", IDS_ADVANCED_REPORT_GROUP_CODES, &m_stuGeneralLedgerNode.Options.Codes.bAdvancedReportGroupCodes)); // Advanced Report Group Codes
	pGLCodes->m_tplUserRightsNodeList.AddTail(CreateNode("AdvancedFilterCodes", IDS_ADVANCED_FILTER_CODES, &m_stuGeneralLedgerNode.Options.Codes.bAdvancedFilterCodes)); // Advanced Filter Codes
	pGLCodes->m_tplUserRightsNodeList.AddTail(CreateNode("UserDefinedClassifications", IDS_GL_USER_DEFINED_CLASSIFICATIONS, &m_stuGeneralLedgerNode.Options.Codes.bUserDefinedClassifications)); // Advanced Filter Codes

	//	PBI 49433	06/03/2019	mvt - Added Manage Internal Control Alerts node
	//	options - manage internal control alerts node
	CUserRightsBaseNode* pGLManageInternalControlAlerts = CreateNode("ManageInternalControlAlerts", IDS_MANAGE_INTERNAL_CONTROL_ALERTS, &m_stuGeneralLedgerNode.Options.ManageInternalControlAlerts.bManageInternalControlAlerts);
	pGLOptions->m_tplUserRightsNodeList.AddTail(pGLManageInternalControlAlerts);
	pGLManageInternalControlAlerts->m_tplUserRightsNodeList.AddTail(CreateNode("APIPosting", IDS_API_POSTING, &m_stuGeneralLedgerNode.Options.ManageInternalControlAlerts.bAPIPosting));

	// options - file maintenance node
	CUserRightsBaseNode* pGLFileMaintenance = CreateNode("FileMaintenance", IDS_CT_TABLE_MAINTENANCE, &m_stuGeneralLedgerNode.Options.FileMaintenance.bTotal); // File Maintenance
	pGLOptions->m_tplUserRightsNodeList.AddTail(pGLFileMaintenance);
	pGLFileMaintenance->m_tplUserRightsNodeList.AddTail(CreateNode("DisplayFileInformation", IDS_CTRL_DISPLAY_FILE_INFO, &m_stuGeneralLedgerNode.Options.FileMaintenance.bDisplayFileInformation)); // Display File Information
	pGLFileMaintenance->m_tplUserRightsNodeList.AddTail(CreateNode("RecalculatePeriodFile", IDS_CT_RECALCULATEPERIODTABLE, &m_stuGeneralLedgerNode.Options.FileMaintenance.bRecalculatePeriodFile)); // Recalculate Period File
	CUserRightsBaseNode* pGLImportExportNode = CreateNode("ImportExport", IDS_IMPORT_EXPORT, &m_stuGeneralLedgerNode.Options.FileMaintenance.ImportExport.bTotal); // File Maintenance
	pGLFileMaintenance->m_tplUserRightsNodeList.AddTail(pGLImportExportNode);
	pGLImportExportNode->m_tplUserRightsNodeList.AddTail(CreateNode("Import", IDS_IE_IMPORT, &m_stuGeneralLedgerNode.Options.FileMaintenance.ImportExport.bImport)); // Import information okay
	pGLImportExportNode->m_tplUserRightsNodeList.AddTail(CreateNode("Export", IDS_IE_EXPORT, &m_stuGeneralLedgerNode.Options.FileMaintenance.ImportExport.bExport)); // Export information okay

	// InTimeTec: 11.12.2012.HS - Custom Menus
	// Custom Menus Node
	pGeneralLedgerNode->m_tplUserRightsNodeList.AddTail(CreateNode("CustomOptions", IDS_CUSTOM_OPTIONS, &m_stuGeneralLedgerNode.bCustomOptions));
	// InTimeTec: End
}

//=================================== End Of GL ================================================================//
//==============================================================================================================//

// creates object list for the Inventory node
//=================================== Start Of IN ================================================================//
//================================================================================================================//
void CGBLUserRightsInformation::CreateInventoryObjectList()
{
	// creating inventory node
	CUserRightsBaseNode* pInventoryNode = CreateNode("Inventory", IDS_MODULE_IN, &m_stuModulesNode.bInventory);		// Inventory
	m_pUserRightsRootNode->m_tplUserRightsNodeList.AddTail(pInventoryNode);

	// inventory - stock
	CUserRightsBaseNode* pINStockNode = CreateNode("Stock", IDS_CTRL_STOCK, &m_stuInventoryNode.Stock.bTotal);	// Stock
	pInventoryNode->m_tplUserRightsNodeList.AddTail(pINStockNode);

	// PBI 17468 BK 9/12/14 Change user rights to separate Add/Edit and View Only settings, Stock Master Records
	CUserRightsBaseNode* pStockMasterNode = CreateNode("StockMaster", IDS_STOCK_MASTER_RECORDS, &m_stuInventoryNode.Stock.StockMaster.bTotal);
	pINStockNode->m_tplUserRightsNodeList.AddTail(pStockMasterNode);
	pStockMasterNode->m_tplUserRightsNodeList.AddTail(CreateNode("AddEditStock", IDS_ADD_EDIT_STOCK_RECORDS, &m_stuInventoryNode.Stock.StockMaster.bAddEditStock));
	pStockMasterNode->m_tplUserRightsNodeList.AddTail(CreateNode("ViewStock", IDS_VIEW_STOCK_RECORDS, &m_stuInventoryNode.Stock.StockMaster.bViewStock));
	// pINStockNode->m_tplUserRightsNodeList.AddTail(CreateNode("StockMaintenance", IDS_CTRL_STOCK_MAINTENANCE, &m_stuInventoryNode.Stock.bStockMaintenance));			// Stock Maintenance
	// pINStockNode->m_tplUserRightsNodeList.AddTail(CreateNode("StockInquiryOnly", IDS_STOCK_INQUIRY_ONLY, &m_stuInventoryNode.Stock.bStockInquiryOnly));				// Stock Inquiry Only

	pINStockNode->m_tplUserRightsNodeList.AddTail(CreateNode("SetupStockAlias", IDS_CTRL_SETUP_STOCK_ALIAS, &m_stuInventoryNode.Stock.bSetupStockAlias));			// Setup Stock Alias
	//	PBI 4406	09/26/2014	mvt - Update Stock Min/Max Quantities
	pINStockNode->m_tplUserRightsNodeList.AddTail(CreateNode("UpdateMinMaxQuantities", IDS_UPDATE_QUANTITIES, &m_stuInventoryNode.Stock.bUpdateQuantities));
	pINStockNode->m_tplUserRightsNodeList.AddTail(CreateNode("CopyStockItems", IDS_COPY_STOCK_ITEMS, &m_stuInventoryNode.Stock.bCopyStockItems));					// Copy Stock Items
	pINStockNode->m_tplUserRightsNodeList.AddTail(CreateNode("RenumberStockItems", IDS_CTRL_RENUMBER_STOCK_ITEMS, &m_stuInventoryNode.Stock.bRenumberStockItems));	// Renumber Stock Items
	pINStockNode->m_tplUserRightsNodeList.AddTail(CreateNode("GlobalPriceChange", IDS_CTRL_GLOBAL_PRICE_CHNG, &m_stuInventoryNode.Stock.bGlobalPriceChange));		// Global Price Change

	//RS 06/02/2022 - PBI 59622 - Added Import/Export ArcBill
	//CUserRightsBaseNode* pINImportExportArcBillNode = CreateNode("ImportExportArcBill", IDS_IMPORT_EXPORT_ARC_BILL, &m_stuInventoryNode.Stock.ImportExportArcBill.bTotal); // Import/Export ArcBill
	//pINStockNode->m_tplUserRightsNodeList.AddTail(pINImportExportArcBillNode);
	//pINImportExportArcBillNode->m_tplUserRightsNodeList.AddTail(CreateNode("ImportStockItems", IDS_IMPORT_STOCK_ITEMS, &m_stuInventoryNode.Stock.ImportExportArcBill.bImportStockItems)); // Import Stock Items
	//pINImportExportArcBillNode->m_tplUserRightsNodeList.AddTail(CreateNode("ExportStockItems", IDS_EXPORT_STOCK_ITEMS, &m_stuInventoryNode.Stock.ImportExportArcBill.bExportStockItems)); // Export Stock Items

	// inventory - transaction
	CUserRightsBaseNode* pINAdjustmentsNode = CreateNode("Adjustments", IDS_CTRL_ADJUSTMENTS, &m_stuInventoryNode.Adjustments.bTotal); // Transaction
	pInventoryNode->m_tplUserRightsNodeList.AddTail(pINAdjustmentsNode);
	//RS - 01/10/2022 - PBI 57653 -	Added ability to hide Last Cost field on IN Adjust Stock Quantity window
	CUserRightsBaseNode* pINAdjustmentsSubNode = CreateNode("AdjustStockQuantities", IDS_ADJUST_STOCK_QUANTITIES, &m_stuInventoryNode.Adjustments.bAdjustStockQuantities);	// Adjust Stock Quantities
	pINAdjustmentsNode->m_tplUserRightsNodeList.AddTail(pINAdjustmentsSubNode);
	pINAdjustmentsSubNode->m_tplUserRightsNodeList.AddTail(CreateNode("Add", IDS_IN_ADD, &m_stuInventoryNode.Adjustments.bInAdd));				// Adjust Add
	pINAdjustmentsSubNode->m_tplUserRightsNodeList.AddTail(CreateNode("Subtract", IDS_IN_SUBTRACT, &m_stuInventoryNode.Adjustments.bInSubtract));				// Adjust Subtract
	if (g_pGBLSystemInformationCMSDll != NULL)
	{
		if (g_pGBLSystemInformationCMSDll->ModulesInstalled.MultiLocationInventory.IsUsable)
			pINAdjustmentsSubNode->m_tplUserRightsNodeList.AddTail(CreateNode("Transfer", IDS_IN_TRANSFER, &m_stuInventoryNode.Adjustments.bInTransfer));				// Adjust Transfer
	}
	pINAdjustmentsNode->m_tplUserRightsNodeList.AddTail(CreateNode("AdjustTransaction", IDS_ADJUST_TRANSACTION, &m_stuInventoryNode.Adjustments.bAdjustTransaction));				// Adjust Transaction
	CUserRightsBaseNode* pINImportAdjusmtentsFromNode = CreateNode("ImportAdjustmentsFrom", IDS_CT_GENERATE_ADJUSTMENTS_FROM, &m_stuInventoryNode.Adjustments.ImportAdjustmentsFrom.bTotal); // Import Adjustments From
	pINAdjustmentsNode->m_tplUserRightsNodeList.AddTail(pINImportAdjusmtentsFromNode);
	pINImportAdjusmtentsFromNode->m_tplUserRightsNodeList.AddTail(CreateNode("HandHeldScanner", IDS_HAND_HELD_SCANNER, &m_stuInventoryNode.Adjustments.ImportAdjustmentsFrom.bHandHeldScanner));	// Hand Held Scanner
	pINImportAdjusmtentsFromNode->m_tplUserRightsNodeList.AddTail(CreateNode("ExternalFile", IDS_CTRL_EXT_FILE, &m_stuInventoryNode.Adjustments.ImportAdjustmentsFrom.bExternalFile));				// External File
	pINAdjustmentsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintShrinkageReport", IDS_PRINT_SHRINKAGE_REPORT, &m_stuInventoryNode.Adjustments.bPrintShrinkageReport));					// Print Shrinkage Report
	pINAdjustmentsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintEditReport", IDS_MSG_PRINT_EDIT_REPORT, &m_stuInventoryNode.Adjustments.bPrintEditReport));					// Print Edit Report
	pINAdjustmentsNode->m_tplUserRightsNodeList.AddTail(CreateNode("Post", IDS_CTRL_POST, &m_stuInventoryNode.Adjustments.bPost));										// Post
	CUserRightsBaseNode* pINBatchControlNode = CreateNode("BatchControl", IDS_CTRL_BATCH_CONTROL, &m_stuInventoryNode.Adjustments.BatchControl.bTotal);					// Batch Control
	pINAdjustmentsNode->m_tplUserRightsNodeList.AddTail(pINBatchControlNode);
	pINBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SelectBatch", IDS_SELECTBATCH, &m_stuInventoryNode.Adjustments.BatchControl.bSelectBatch));	// Select Batch
	pINBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SetupBatch", IDS_CTRL_SETUP_BATCH, &m_stuInventoryNode.Adjustments.BatchControl.bSetupBatch)); // Setup Batch
	pINAdjustmentsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ClearAdjustments", IDS_CTRL_CLEAR_ADJUSTMENTS, &m_stuInventoryNode.Adjustments.bClearAdjustments)); // Clear Adjustments
	// inventory - import adjustments from node
	pINAdjustmentsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ChangeLocalTransactionDate", IDS_CT_CLT_DATE, &m_stuInventoryNode.Adjustments.bChangeLocalTransactionDate)); // Change Local Transaction Date

	////////////////////////
	// inventory - reports
	CUserRightsBaseNode* pINReportsNode = CreateNode("Reports", IDS_CTRL_REPORTS, &m_stuInventoryNode.Reports.bTotal); // Reports
	pInventoryNode->m_tplUserRightsNodeList.AddTail(pINReportsNode);
	pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("QuickStockList", IDS_CTRL_QUICK_STOCK_LIST, &m_stuInventoryNode.Reports.bQuickStockList));	// Quick Stock List
	pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("StockLabels", IDS_CTRL_STOCK_LABELS, &m_stuInventoryNode.Reports.bStockLabels));			// Stock Labels
	pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("MasterStock", IDS_CTRL_MASTER_STOCK, &m_stuInventoryNode.Reports.bMasterStock));			// Master Stock
	pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PhysicalCountWorksheet", IDS_CTRL_PHYSICAL_COUNT_WORKSHEET, &m_stuInventoryNode.Reports.bPhysicalCountWorksheet)); // Physical Count Worksheet
	pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PriceBook", IDS_CTRL_PRICE_BOOK, &m_stuInventoryNode.Reports.bPriceBook));					// Price Book
	pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("OnHand", IDS_CTRL_ON_HAND, &m_stuInventoryNode.Reports.bOnHand));							// On Hand
	pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("StockValue", IDS_CTRL_STOCK_VALUE, &m_stuInventoryNode.Reports.bStockValue));				// Stock Value
	pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("Understock", IDS_OPT_UNDERSTOCK, &m_stuInventoryNode.Reports.bUnderstock));					// Understock
	pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("Overstock", IDS_CTRL_OVERSTOCK, &m_stuInventoryNode.Reports.bOverstock));					// Overstock
	//	PBI 4406	09/26/2014	mvt - Below Minimum Quantity Report
	pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("BelowMinimumQuantity", IDS_REPORTS_BELOW_MINIMUM_QUANTITY, &m_stuInventoryNode.Reports.bBelowMinimumQuantity));
	pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("OnOrder", IDS_CTRL_ON_ORDER, &m_stuInventoryNode.Reports.bOnOrder));						// On Order
	pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("BackOrder", IDS_BACK_ORDER, &m_stuInventoryNode.Reports.bBackOrder));						// Back Order
	pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("LotExpiration", IDS_LOT_EXPIRATION, &m_stuInventoryNode.Reports.bLotExpiration));				// Lot Expiration
	pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("History", IDS_AR_HISTORY, &m_stuInventoryNode.Reports.bHistory));							// History
	pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("Movement", IDS_MOVEMENT, &m_stuInventoryNode.Reports.bMovement));
	// 1-26531 BK 12/12/06 Monthly Movement Report
	pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("MonthlyMovement", IDS_MONTHLY_MOVEMENT, &m_stuInventoryNode.Reports.bMonthlyMovement));

	//JKC DefectID: 1-8027 Check to see if Specialty Shop Installed before adding tail.
	if (g_pGBLSystemInformationCMSDll != NULL)
	{
		if (g_pGBLSystemInformationCMSDll->ModulesInstalled.SpecialtyShop.IsUsable)
			pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("AdvancedSalesInfo", IDS_CTRL_ADV_SALES_INFO, &m_stuInventoryNode.Reports.bAdvancedSalesInfo)); // Advanced Sales Info

		// 1-26658 BK 12/18/08 show only if sales module is installed
		if (g_pGBLSystemInformationCMSDll->ModulesInstalled.SalesEntry.IsUsable)
			// begbert 11-27-2006 1-26351 : moved "Promotional Sales" to after "Advanced Sales Info"
			pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PromotionalSales", IDS_PROMOTIONAL_SALES, &m_stuInventoryNode.Reports.bPromotionalSales));	// Promotional Sales  // begbert 11-18-2006 Feature 28
	}
	else
	{
		pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("AdvancedSalesInfo", IDS_CTRL_ADV_SALES_INFO, &m_stuInventoryNode.Reports.bAdvancedSalesInfo)); // Advanced Sales Info
		// 1-31594 BK 12/22/08 need to load this when g_pGBLSystemInformationCMSDll is null or will never become enabled
		pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PromotionalSales", IDS_PROMOTIONAL_SALES, &m_stuInventoryNode.Reports.bPromotionalSales));	// Promotional Sales  // begbert 11-18-2006 Feature 28
	}

	//Defect ID 1-14293, PGP(11/02/2004) - Moved "control" to last item under reports to maintain consistency with
	//menu appearance in Inventory.
	pINReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("Control", IDS_CTRL_CONTROL, &m_stuInventoryNode.Reports.bControl));							// Control

	////////////////////////
	// inventory - period end
	CUserRightsBaseNode* pINPeriodEndNode = CreateNode("PeriodEnd", IDS_CTRL_PERIOD_END, &m_stuInventoryNode.PeriodEnd.bTotal); // Period End
	pInventoryNode->m_tplUserRightsNodeList.AddTail(pINPeriodEndNode);
	pINPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("VarianceAmounts", IDS_CTRL_VAR_AMOUNT, &m_stuInventoryNode.PeriodEnd.bVarianceAmounts)); // Variance Amounts
	pINPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("PurgeHistory", IDS_CTRL_PURGE_HISTORY, &m_stuInventoryNode.PeriodEnd.bPurgeHistory)); // Purge History

	////////////////////////
	// inventory - setup kits
	CUserRightsBaseNode* pINSetupKitsNode = CreateNode("SetupKits", IDS_CTRL_SETUP_KITS, &m_stuInventoryNode.SetupKits.bTotal); // Setup Kits
	pInventoryNode->m_tplUserRightsNodeList.AddTail(pINSetupKitsNode);
	pINSetupKitsNode->m_tplUserRightsNodeList.AddTail(CreateNode("SetupKits", IDS_CTRL_SETUP_KITS, &m_stuInventoryNode.SetupKits.bSetupKits)); // Setup Kits
	pINSetupKitsNode->m_tplUserRightsNodeList.AddTail(CreateNode("KitAssemblyReport", IDS_CTRL_KIT_ASSEMBLY_REPORT, &m_stuInventoryNode.SetupKits.bKitAssemblyReport)); // Kit Assembly Report
	pINSetupKitsNode->m_tplUserRightsNodeList.AddTail(CreateNode("MasterKitReport", IDS_CTRL_MASTER_KIT_RPT, &m_stuInventoryNode.SetupKits.bMasterKitReport)); // Master Kit Report
	pINSetupKitsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ComponentRequirementReport", IDS_CTRL_COMPONENT_REQ_RPT, &m_stuInventoryNode.SetupKits.bComponentRequirementReport)); // Component Requirement Report

	////////////////////////
	// inventory - options
	CUserRightsBaseNode* pINOptionsNode = CreateNode("Options", IDS_CTRL_OPTIONS, NULL); // Options
	pInventoryNode->m_tplUserRightsNodeList.AddTail(pINOptionsNode);
	pINOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ModulePreferences", IDS_CTRL_MODULE_PREFERENCES, &m_stuInventoryNode.Options.bModulePreferences)); // Module Preferences
	pINOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("InventoryCodes", IDS_INVENTORY_CODES, &m_stuInventoryNode.Options.bInventoryCodes)); // Inventory Codes
	pINOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("TransactionCodes", IDS_CTRL_TRANS_CODE, &m_stuInventoryNode.Options.bTransactionCodes)); // Transaction Codes

	//JKC DefectID: 1-8026 Check to see if MLI Installed before adding tail.
	if (g_pGBLSystemInformationCMSDll != NULL)
	{
		if (g_pGBLSystemInformationCMSDll->ModulesInstalled.MultiLocationInventory.IsUsable)
			pINOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("LocationCodes", IDS_LOCATION_CODES, &m_stuInventoryNode.Options.bLocationCodes)); // Location Codes
		// Erick Korsten - 05/04/04 - 1-11075 - Add Multi-Pack Codes if specialty shop is installed
		if (g_pGBLSystemInformationCMSDll->ModulesInstalled.SpecialtyShop.IsUsable)
			pINOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("MultiPackCodes", IDS_IN_MULTIPACKCODES, &m_stuInventoryNode.Options.bMultiPackCodes)); // Multi-Pack Codes
	}
	else
	{
		pINOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("LocationCodes", IDS_LOCATION_CODES, &m_stuInventoryNode.Options.bLocationCodes)); // Location Codes
		// Erick Korsten - 05/04/04 - 1-11075 - Add Multi-Pack Codes
		pINOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("MultiPackCodes", IDS_IN_MULTIPACKCODES, &m_stuInventoryNode.Options.bMultiPackCodes)); // Multi-Pack Codes
	}

	pINOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("UserDefinedField1", IDS_CTRL_UDF_FIELD1, &m_stuInventoryNode.Options.bUserDefinedField1)); // User Defined Field1
	pINOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("UserDefinedField2", IDS_CTRL_UDF_FIELD2, &m_stuInventoryNode.Options.bUserDefinedField2)); // User Defined Field2
	pINOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PromotionalPricing", IDS_CTRL_PROMO_PRICING, &m_stuInventoryNode.Options.bPromotionalPricing)); // Promotional Pricing

	// inventory - file maintenance node
	CUserRightsBaseNode* pINFileMaintenanceNode = CreateNode("FileMaintenance", IDS_CT_TABLE_MAINTENANCE, &m_stuInventoryNode.Options.FileMaintenance.bTotal); // File Maintenance
	pINOptionsNode->m_tplUserRightsNodeList.AddTail(pINFileMaintenanceNode);
	pINFileMaintenanceNode->m_tplUserRightsNodeList.AddTail(CreateNode("DisplayFileInformation", IDS_CTRL_DISPLAY_FILE_INFO, &m_stuInventoryNode.Options.FileMaintenance.bDisplayFileInformation)); // Display File Information
	pINFileMaintenanceNode->m_tplUserRightsNodeList.AddTail(CreateNode("RecalculateInventoryFiles", IDS_CT_RECALCULATEINVENTORYFILES, &m_stuInventoryNode.Options.FileMaintenance.bRecalculateInventoryFiles)); // Recalculate Inventory Files
	CUserRightsBaseNode* pINImportExportNode = CreateNode("ImportExport", IDS_IMPORT_EXPORT, &m_stuInventoryNode.Options.FileMaintenance.ImportExport.bTotal); // File Maintenance
	pINFileMaintenanceNode->m_tplUserRightsNodeList.AddTail(pINImportExportNode);
	pINImportExportNode->m_tplUserRightsNodeList.AddTail(CreateNode("Import", IDS_IE_IMPORT, &m_stuInventoryNode.Options.FileMaintenance.ImportExport.bImport)); // Import information okay
	pINImportExportNode->m_tplUserRightsNodeList.AddTail(CreateNode("Export", IDS_IE_EXPORT, &m_stuInventoryNode.Options.FileMaintenance.ImportExport.bExport)); // Export information okay

	// InTimeTec: 11.12.2012.HS - Custom Menus
	// Custom Menus Node
	pInventoryNode->m_tplUserRightsNodeList.AddTail(CreateNode("CustomOptions", IDS_CUSTOM_OPTIONS, &m_stuInventoryNode.bCustomOptions));
	// InTimeTec: End
}

//=================================== End Of IN ================================================================//
//==============================================================================================================//

// creates object tree for the Job Cost
//=================================== Start Of JC ================================================================//
//================================================================================================================//
void CGBLUserRightsInformation::CreateJobCostObjectList()
{
	// creating Job Cost node
	CUserRightsBaseNode* pJobCostNode = CreateNode("JobCost", IDS_MODULE_JC, &m_stuModulesNode.bJobCost);		// Job Cost
	m_pUserRightsRootNode->m_tplUserRightsNodeList.AddTail(pJobCostNode);

	//	PBI xxxxx	05/11/2015	mvt - Added Job Cost security
	//	Job menu
	CUserRightsBaseNode* pJobNode = CreateNode("Job", IDS_JOB, &m_stuJobCostNode.Job.bTotal);
	pJobCostNode->m_tplUserRightsNodeList.AddTail(pJobNode);
	pJobNode->m_tplUserRightsNodeList.AddTail(CreateNode("JobManagement", IDS_JOB_MANAGEMENT, &m_stuJobCostNode.Job.bJobManagement));
	//	PBI 29528	08/07/2015	mvt - Hid Copy Job and Change Job Detail Order for first release of Job Cost
	//	pJobNode->m_tplUserRightsNodeList.AddTail(CreateNode("CopyJob", IDS_COPY_JOB, &m_stuJobCostNode.Job.bCopyJob));
		//	PBI 27400	08/06/2015	mvt - Changed from Recalculate Job Cost Files to Change Job Detail Order
	//	pJobNode->m_tplUserRightsNodeList.AddTail(CreateNode("ChangeJobDetailOrder", IDS_CHANGE_JOB_DETAIL_ORDER, &m_stuJobCostNode.Job.bChangeJobDetailOrder));

		//	Cost Activity menu
	CUserRightsBaseNode* pCostActivityNode = CreateNode("CostActivity", IDS_COST_ACTIVITY_TITLE, &m_stuJobCostNode.CostActivity.bTotal);
	pJobCostNode->m_tplUserRightsNodeList.AddTail(pCostActivityNode);
	//	Cost Activity submenu
	CUserRightsBaseNode* pCostActivitySubNode = CreateNode("CostActivity", IDS_COST_ACTIVITY_TITLE, &m_stuJobCostNode.CostActivity.CostActivityNode.bTotal);
	pCostActivityNode->m_tplUserRightsNodeList.AddTail(pCostActivitySubNode);
	pCostActivitySubNode->m_tplUserRightsNodeList.AddTail(CreateNode("CostActivity", IDS_COST_ACTIVITY_TITLE, &m_stuJobCostNode.CostActivity.CostActivityNode.bCostActivity));
	pCostActivitySubNode->m_tplUserRightsNodeList.AddTail(CreateNode("ReversalActivity", IDS_REVERSAL_ACTIVITY, &m_stuJobCostNode.CostActivity.CostActivityNode.bReversalActivity));
	pCostActivitySubNode->m_tplUserRightsNodeList.AddTail(CreateNode("TransferActivity", IDS_TRANSFER_ACTIVITY, &m_stuJobCostNode.CostActivity.CostActivityNode.bTransferActivity));
	pCostActivitySubNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintEditReport", IDS_MSG_PRINT_EDIT_REPORT, &m_stuJobCostNode.CostActivity.CostActivityNode.bPrintEditReport));
	pCostActivitySubNode->m_tplUserRightsNodeList.AddTail(CreateNode("PostActivity", IDS_POST_ACTIVITY, &m_stuJobCostNode.CostActivity.CostActivityNode.bPostActivity));
	//	PBI 27400	08/06/2015	mvt - Added batch selection/setup nodes
//	pCostActivitySubNode->m_tplUserRightsNodeList.AddTail(CreateNode("BatchControl", IDS_RIGHTS_BATCH_CONTROL, &m_stuJobCostNode.CostActivity.CostActivityNode.bBatchControl));
	CUserRightsBaseNode* pCostActivityBatchControlNode = CreateNode("BatchControl", IDS_CTRL_BATCH_CONTROL, &m_stuJobCostNode.CostActivity.CostActivityNode.bTotal);					// Batch Control
	pCostActivitySubNode->m_tplUserRightsNodeList.AddTail(pCostActivityBatchControlNode);
	pCostActivityBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SelectBatch", IDS_SELECTBATCH, &m_stuJobCostNode.CostActivity.CostActivityNode.BatchControl.bSelectBatch));	// Select Batch
	pCostActivityBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SetupBatch", IDS_CTRL_SETUP_BATCH, &m_stuJobCostNode.CostActivity.CostActivityNode.BatchControl.bSetupBatch)); // Setup Batch
	//	PBI 27400	08/06/2015	mvt - Changed from Clear Cost Activity to Clear Activity Transactions
	pCostActivitySubNode->m_tplUserRightsNodeList.AddTail(CreateNode("ClearActivityTransactions", IDS_CLEAR_ACTIVITY_TRANSACTIONS, &m_stuJobCostNode.CostActivity.CostActivityNode.bClearActivityTransactions));
	//	AP Enter Bills submenu
	CUserRightsBaseNode* pAPEnterBillsNode = CreateNode("APEnterBills", IDS_ENTER_BILLS_IN_AP, &m_stuJobCostNode.CostActivity.APEnterBills.bTotal);
	pCostActivityNode->m_tplUserRightsNodeList.AddTail(pAPEnterBillsNode);
	pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("EnterBills", IDS_AP_ENTER_BILLS, &m_stuJobCostNode.CostActivity.APEnterBills.bEnterBills));
	pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintEditReport", IDS_MSG_PRINT_EDIT_REPORT, &m_stuJobCostNode.CostActivity.APEnterBills.bPrintEditReport));
	//	PBI 29528	08/07/2015	mvt - Hid Print Immediate Checks, Immediate Check Register, and Batch Control for first release of Job Cost
	//	pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintImmediateChecks", IDS_AP_PRINTIMMEDIATECHECKS1, &m_stuJobCostNode.CostActivity.APEnterBills.bPrintImmediateChecks));
	//	pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ImmediateCheckRegister", IDS_RIGHTS_IMMEDIATE_CHECK_REGISTER, &m_stuJobCostNode.CostActivity.APEnterBills.bImmediateCheckRegister));
	pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PostBills", IDS_AP_POST_BILLS, &m_stuJobCostNode.CostActivity.APEnterBills.bPostBills));
	////	PBI 29724	08/13/2015	mvt - Added batch selection/setup nodes
	//pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("BatchControl", IDS_RIGHTS_BATCH_CONTROL, &m_stuJobCostNode.CostActivity.APEnterBills.bBatchControl));
	CUserRightsBaseNode* pAPEnterBillsBatchControlNode = CreateNode("BatchControl", IDS_CTRL_BATCH_CONTROL, &m_stuJobCostNode.CostActivity.APEnterBills.bBatchControl);					// Batch Control
	pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(pAPEnterBillsBatchControlNode);
	pAPEnterBillsBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SelectBatch", IDS_SELECTBATCH, &m_stuJobCostNode.CostActivity.APEnterBills.BatchControl.bSelectBatch));	// Select Batch
	pAPEnterBillsBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SetupBatch", IDS_CTRL_SETUP_BATCH, &m_stuJobCostNode.CostActivity.APEnterBills.BatchControl.bSetupBatch)); // Setup Batch
	pAPEnterBillsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ClearBills", IDS_RIGHTS_CLEAR_BILLS, &m_stuJobCostNode.CostActivity.APEnterBills.bClearBills));

	//	PBI 27400	08/06/2015	mvt - Changed from Generate Purchase Orders to PO Generate Purchase Orders
	pCostActivityNode->m_tplUserRightsNodeList.AddTail(CreateNode("POGeneratePurchaseOrders", IDS_PO_GENERATE_PURCHASE_ORDERS, &m_stuJobCostNode.CostActivity.bPOGeneratePurchaseOrders));
	pCostActivityNode->m_tplUserRightsNodeList.AddTail(CreateNode("ChangeLocalTransactionDate", IDS_CT_CLT_DATE, &m_stuJobCostNode.CostActivity.bChangeLocalTransactionDate));

	//	Invoicing
	//	PBI 27400	08/06/2015	mvt - Changed from Billing to Invoicing
	CUserRightsBaseNode* pBillingNode = CreateNode("Invoicing", IDS_INVOICING, &m_stuJobCostNode.BillingInventory.bInvoicing);
	pJobCostNode->m_tplUserRightsNodeList.AddTail(pBillingNode);
	//	PBI 27400	08/06/2015	mvt - Added Make To Order submenu
	CUserRightsBaseNode* pMakeToOrderSubNode = CreateNode("MakeToOrder", IDS_MAKE_TO_ORDER, &m_stuJobCostNode.BillingInventory.bMakeToOrder);
	pBillingNode->m_tplUserRightsNodeList.AddTail(pMakeToOrderSubNode);
	pMakeToOrderSubNode->m_tplUserRightsNodeList.AddTail(CreateNode("EnterInvoiceInOE", IDS_ENTER_INVOICE_IN_OE, &m_stuJobCostNode.BillingInventory.MakeToOrderNode.bEnterInvoiceInOE));
	pMakeToOrderSubNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintEditReport", IDS_RIGHTS_PRINT_EDIT_REPORT, &m_stuJobCostNode.BillingInventory.MakeToOrderNode.bPrintEditReport));
	pMakeToOrderSubNode->m_tplUserRightsNodeList.AddTail(CreateNode("PostInvoices", IDS_CT_POST_INVOICES, &m_stuJobCostNode.BillingInventory.MakeToOrderNode.bPostInvoices));
	//	PBI 27400	08/06/2015	mvt - Added batch selection/setup nodes
//	pMakeToOrderSubNode->m_tplUserRightsNodeList.AddTail(CreateNode("BatchControl", IDS_RIGHTS_BATCH_CONTROL, &m_stuJobCostNode.BillingInventory.MakeToOrderNode.bBatchControl));
	CUserRightsBaseNode* pMakeToOrderBatchControlNode = CreateNode("BatchControl", IDS_CTRL_BATCH_CONTROL, &m_stuJobCostNode.BillingInventory.MakeToOrderNode.bBatchControl);					// Batch Control
	pMakeToOrderSubNode->m_tplUserRightsNodeList.AddTail(pMakeToOrderBatchControlNode);
	pMakeToOrderBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SelectBatch", IDS_SELECTBATCH, &m_stuJobCostNode.BillingInventory.MakeToOrderNode.BatchControl.bSelectBatch));	// Select Batch
	pMakeToOrderBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SetupBatch", IDS_CTRL_SETUP_BATCH, &m_stuJobCostNode.BillingInventory.MakeToOrderNode.BatchControl.bSetupBatch)); // Setup Batch

	pMakeToOrderSubNode->m_tplUserRightsNodeList.AddTail(CreateNode("ClearInvoiceTransactions", IDS_CLEAR_INVOICE_TRANSACTIONS, &m_stuJobCostNode.BillingInventory.MakeToOrderNode.bClearInvoiceTransactions));
	CUserRightsBaseNode* pMakeToStockSubNode = CreateNode("MakeToStock", IDS_MAKE_TO_STOCK, &m_stuJobCostNode.BillingInventory.bMakeToStock);
	pBillingNode->m_tplUserRightsNodeList.AddTail(pMakeToStockSubNode);
	pMakeToStockSubNode->m_tplUserRightsNodeList.AddTail(CreateNode("TransferToInventory", IDS_TRANSFER_TO_INVENTORY, &m_stuJobCostNode.BillingInventory.MakeToStockNode.bTransferToInventory));
	pMakeToStockSubNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintEditReport", IDS_RIGHTS_PRINT_EDIT_REPORT, &m_stuJobCostNode.BillingInventory.MakeToStockNode.bPrintEditReport));
	pMakeToStockSubNode->m_tplUserRightsNodeList.AddTail(CreateNode("PostTransfers", IDS_POST_TRANSFERS, &m_stuJobCostNode.BillingInventory.MakeToStockNode.bPostTransfers));
	//	PBI 27400	08/06/2015	mvt - Added batch selection/setup nodes
//	pMakeToStockSubNode->m_tplUserRightsNodeList.AddTail(CreateNode("BatchControl", IDS_RIGHTS_BATCH_CONTROL, &m_stuJobCostNode.BillingInventory.MakeToStockNode.bBatchControl));
	CUserRightsBaseNode* pMakeToStockBatchControlNode = CreateNode("BatchControl", IDS_CTRL_BATCH_CONTROL, &m_stuJobCostNode.BillingInventory.MakeToStockNode.bBatchControl);					// Batch Control
	pMakeToStockSubNode->m_tplUserRightsNodeList.AddTail(pMakeToStockBatchControlNode);
	pMakeToStockBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SelectBatch", IDS_SELECTBATCH, &m_stuJobCostNode.BillingInventory.MakeToStockNode.BatchControl.bSelectBatch));	// Select Batch
	pMakeToStockBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SetupBatch", IDS_CTRL_SETUP_BATCH, &m_stuJobCostNode.BillingInventory.MakeToStockNode.BatchControl.bSetupBatch)); // Setup Batch

	pMakeToStockSubNode->m_tplUserRightsNodeList.AddTail(CreateNode("ClearTransfers", IDS_CLEAR_TRANSFERS, &m_stuJobCostNode.BillingInventory.MakeToStockNode.bClearTransfers));
	pBillingNode->m_tplUserRightsNodeList.AddTail(CreateNode("ChangeLocalTransactionDate", IDS_CT_CLT_DATE, &m_stuJobCostNode.BillingInventory.bChangeLocalTransactionDate));

	//	Reports
	CUserRightsBaseNode* pReportsNode = CreateNode("Reports", IDS_CTRL_REPORTS, &m_stuJobCostNode.Reports.bTotal);
	pJobCostNode->m_tplUserRightsNodeList.AddTail(pReportsNode);
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("JobReport", IDS_JOB_REPORT, &m_stuJobCostNode.Reports.bJobReport));
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("JobSchedule", IDS_JOB_SCHEDULE, &m_stuJobCostNode.Reports.bJobSchedule));
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("JobListing", IDS_JOB_LISTING, &m_stuJobCostNode.Reports.bJobListing));
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("JobRequirements", IDS_JOB_REQUIREMENTS, &m_stuJobCostNode.Reports.bJobRequirements));
	//	PBI 27400	08/06/2015	mvt - Changed from Job Estimate to Job Price Report
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("JobPriceReport", IDS_JOB_PRICE_ESTIMATE_REPORT, &m_stuJobCostNode.Reports.bJobPriceReport));
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("CostActivityAnalysis", IDS_COST_ACTIVITY_ANALYSIS, &m_stuJobCostNode.Reports.bCostActivityAnalysis));
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("WorkInProcess", IDS_WORK_IN_PROCESS, &m_stuJobCostNode.Reports.bWorkInProcess));
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("EstimatedVSActualCost", IDS_ESTIMATED_VS_ACTUAL_COST_REPORT, &m_stuJobCostNode.Reports.bEstimatedVSActualCostReport));
	//	PBI 37137	09/12/2017	mvt - Added WIP Recalculation
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("WIPControl", IDS_CONTROLRPT, &m_stuJobCostNode.Reports.bWIPControl));

	//	Period End
//	PBI 29528	08/07/2015	mvt - Hid Period End for first release of Job Cost
	// begbert 11-18-2015 PBI 31292 : As of now, Purge Jobs is included and Purge History alone is hidden.
	CUserRightsBaseNode* pPeriodEndNode = CreateNode("PeriodEnd", IDS_CTRL_PERIOD_END, &m_stuJobCostNode.PeriodEnd.bTotal);
	pJobCostNode->m_tplUserRightsNodeList.AddTail(pPeriodEndNode);
	pPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("PurgeJobs", IDS_PURGE_JOBS, &m_stuJobCostNode.PeriodEnd.bPurgeJobs));
	//pPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("PurgeHistory", IDS_CTRL_PURGE_HISTORY, &m_stuJobCostNode.PeriodEnd.bPurgeHistory));

	//	Options
	CUserRightsBaseNode* pOptionsNode = CreateNode("Options", IDS_CTRL_OPTIONS, &m_stuJobCostNode.Options.bTotal);
	pJobCostNode->m_tplUserRightsNodeList.AddTail(pOptionsNode);
	pOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ModulePreferences", IDS_CTRL_MODULE_PREFERENCES, &m_stuJobCostNode.Options.bModulePreferences));
	//	PBI 30035	08/26/2015	mvt - Removed unneeded menu entries
//	pOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("StayInAddModeOnSave", IDS_STAY_IN_ADD_MODE_ON_SAVE_STR, &m_stuJobCostNode.Options.bStayInAddModeOnSave));
//	pOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("DisplayToolbar", IDS_DISPLAY_TOOLBAR_STR, &m_stuJobCostNode.Options.bDisplayToolbar));
//	pOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("DisplayStatusBar", IDS_DISPLAY_STATUSBAR, &m_stuJobCostNode.Options.bDisplayStatusbar));
	pOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PhaseTemplates", IDS_PHASE_TEMPLATES, &m_stuJobCostNode.Options.bPhaseTemplates));
	//	Cost Code Templates submenu
	CUserRightsBaseNode* pCostCodeTemplatesNode = CreateNode("CostCodeTemplates", IDS_COST_CODE_TEMPLATES, &m_stuJobCostNode.Options.CostCodeTemplates.bTotal);
	pOptionsNode->m_tplUserRightsNodeList.AddTail(pCostCodeTemplatesNode);
	pCostCodeTemplatesNode->m_tplUserRightsNodeList.AddTail(CreateNode("Material", IDS_AP_MATERIAL, &m_stuJobCostNode.Options.CostCodeTemplates.bMaterial));
	pCostCodeTemplatesNode->m_tplUserRightsNodeList.AddTail(CreateNode("Subcontractor", IDS_AP_SUBCONTRACTOR, &m_stuJobCostNode.Options.CostCodeTemplates.bSubcontractor));
	pCostCodeTemplatesNode->m_tplUserRightsNodeList.AddTail(CreateNode("General", IDS_GENERAL, &m_stuJobCostNode.Options.CostCodeTemplates.bGeneral));
	pCostCodeTemplatesNode->m_tplUserRightsNodeList.AddTail(CreateNode("Process", IDS_PROCESS, &m_stuJobCostNode.Options.CostCodeTemplates.bProcess));
	//	Table Maintenance submenu
	CUserRightsBaseNode* pTableMaintenanceNode = CreateNode("TableMaintenance", IDS_CT_TABLE_MAINTENANCE, &m_stuJobCostNode.Options.TableMaintenance.bTotal);
	pOptionsNode->m_tplUserRightsNodeList.AddTail(pTableMaintenanceNode);
	pTableMaintenanceNode->m_tplUserRightsNodeList.AddTail(CreateNode("DisplayTableInformation", IDS_CT_DISPLAY_FILE_INFO, &m_stuJobCostNode.Options.TableMaintenance.bDisplayTableInformation));
	//	PBI 37137	09/12/2017	mvt - Added WIP Recalculation to security
	pTableMaintenanceNode->m_tplUserRightsNodeList.AddTail(CreateNode("WIPRecalculation", IDS_RECALCULATE_WIP, &m_stuJobCostNode.Options.TableMaintenance.bWIPRecalculation));


	// InTimeTec: 11.12.2012.HS - Custom Menus
	// Custom Menus Node
	pJobCostNode->m_tplUserRightsNodeList.AddTail(CreateNode("CustomOptions", IDS_CUSTOM_OPTIONS, &m_stuJobCostNode.bCustomOptions));
	// InTimeTec: End
}

//=================================== End Of JC ================================================================//
//==============================================================================================================//

// creates object tree for the Payroll
//=================================== Start Of PR ================================================================//
//================================================================================================================//
// PBI 137 BK 2/20/14 updated payroll security to match menu items being used in the 1st release
void CGBLUserRightsInformation::CreatePayRollObjectList()
{
	// creating Payroll node
	CUserRightsBaseNode* pPayrollNode = CreateNode("Payroll", IDS_MODULE_PR, &m_stuModulesNode.bPayroll);		// Payroll
	m_pUserRightsRootNode->m_tplUserRightsNodeList.AddTail(pPayrollNode);

	// Employee
	CUserRightsBaseNode* pEmployeeNode = CreateNode(_T("Employee"), IDS_EMPLOYEES, &m_stuPayrollNode.Employee.bTotal);
	pPayrollNode->m_tplUserRightsNodeList.AddTail(pEmployeeNode);
	//RS - 09/04/2024 - PBI 63268 - Added security to not view/edit ACH Information
	CUserRightsBaseNode* pEmployeeMaintenanceNode = CreateNode(_T("EmployeeMaintenance"), IDS_EMPLOYEE_MANAGEMENT, &m_stuPayrollNode.Employee.EmployeeMaintenance.bTotal);
	pEmployeeNode->m_tplUserRightsNodeList.AddTail(pEmployeeMaintenanceNode);
	pEmployeeMaintenanceNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("AddEditEmployeeRecords"), IDS_ADD_EDIT_EMPLOYEE_RECORDS, &m_stuPayrollNode.Employee.EmployeeMaintenance.bAddEditEmployeeRecords));
	pEmployeeMaintenanceNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ViewEmployeeRecords"), IDS_VIEW_EMPLOYEE_RECORDS, &m_stuPayrollNode.Employee.EmployeeMaintenance.bViewEmployeeRecords));
	pEmployeeMaintenanceNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("EditDirectDeposit"), IDS_EDIT_DIRECT_DEPOSIT, &m_stuPayrollNode.Employee.EmployeeMaintenance.bEditDirectDeposit));
	pEmployeeNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("RenumberRecords"), IDS_RENUMBER_EMP_RECORDS, &m_stuPayrollNode.Employee.bRenumberEmployeeRecords));
	pEmployeeNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("CopyEmployeeRecords"), IDS_COPY_EMP_RECORDS, &m_stuPayrollNode.Employee.bCopyEmployeeRecords));
	pEmployeeNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ImportExportEmployeeRecords"), IDS_IMPORT_EXPORT, &m_stuPayrollNode.Employee.bImportExportEmployeeRecords));
	//RCG - 11/12/2020 - PBI 56233 - Added security for ArcTime
	pEmployeeNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("SetupArcTimeTracking"), IDS_SET_UP_ARC_TIME, &m_stuPayrollNode.Employee.bSetupArcTimeTracking));
	pEmployeeNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ExportEmployeesToArcTimeTracking"), IDS_EXPORT_EMPLOYEES_TO_ARC_TIME, &m_stuPayrollNode.Employee.bExportEmployeesToArcTimeTracking));
	//	PBI 22459	10/13/2014	mvt - Added security for Export Employees to TSheets menu item
	pEmployeeNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ImportEmployeesTSheets"), IDS_IMPORT_EMPLOYEES_FROM_QUICKBOOKS_TIME, &m_stuPayrollNode.Employee.bImportEmployeesFromTSheets));
	pEmployeeNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ExportEmployeesToTSheets"), IDS_EXPORT_EMPLOYEES_TO_QUICKBOOKS_TIME, &m_stuPayrollNode.Employee.bExportEmployeesToTSheets));
	//	PBI 23754	10/13/2014	mvt - Added security for Reset TSheets Login menu item
	pEmployeeNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ResetTSheetsLogin"), IDS_RESET_QUICKBOOKS_TIME_LOGIN, &m_stuPayrollNode.Employee.bResetTSheetsLogin));

	// Enter Payroll
	CUserRightsBaseNode* pEnterPayrollNode = CreateNode(_T("EnterPayroll"), IDS_ENTER_PAYROLL, &m_stuPayrollNode.EnterPayroll.bTotal);
	pPayrollNode->m_tplUserRightsNodeList.AddTail(pEnterPayrollNode);
	pEnterPayrollNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ProcessPayroll"), IDS_PROCESS_PAYROLL, &m_stuPayrollNode.EnterPayroll.bProcessPayroll));
	pEnterPayrollNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("Adjustments"), IDS_CTRL_ADJUSTMENTS, &m_stuPayrollNode.EnterPayroll.bAdjustments));
	pEnterPayrollNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("VoidCheck"), IDS_VOID_PAYROLL_TRANS, &m_stuPayrollNode.EnterPayroll.bVoidCheck));

	// PBI 140 BK daily units submenu security
	CUserRightsBaseNode* pDailyUnits = CreateNode(_T("DailyUnits"), IDS_DAILYUNITS, &m_stuPayrollNode.EnterPayroll.DailyUnits.bTotal);
	pEnterPayrollNode->m_tplUserRightsNodeList.AddTail(pDailyUnits);
	pDailyUnits->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ImportDailyUnits"), IDS_IMPORT_DAILY_UNITS, &m_stuPayrollNode.EnterPayroll.DailyUnits.bImport));
	//RCG - 11/12/2020 - PBI 56233 - Added security for ArcTime
	pDailyUnits->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ImportDailyUnitsFromArcTimeTracking"), IDS_IMPORT_DAILY_UNITS_FROM_ARC_TIME, &m_stuPayrollNode.EnterPayroll.DailyUnits.bImportDailyUnitsFromArcTimeTracking));
	//	PBI 23873	10/13/2014	mvt - Added security for Import Daily Units From TSheets menu item
	pDailyUnits->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ImportDailyUnitsFromTSheets"), IDS_IMPORT_DAILY_UNITS_FROM_QUICKBOOKS_TIME, &m_stuPayrollNode.EnterPayroll.DailyUnits.bImportDailyUnitsFromTSheets));
	pDailyUnits->m_tplUserRightsNodeList.AddTail(CreateNode(_T("EnterEditDailyUnits"), IDS_ENTER_EDIT_DAILY_UNITS, &m_stuPayrollNode.EnterPayroll.DailyUnits.bEnterEdit));
	pDailyUnits->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PrintDailyUnits"), IDS_PRINT_DAILY_UNITS, &m_stuPayrollNode.EnterPayroll.DailyUnits.bPrint));
	//	PBI 24915	04/11/2016	mvt - Added security for Daily Units Expense report
	pDailyUnits->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PrintDailyUnitsExpense"), IDS_PRINT_DAILY_UNITS_EXPENSE, &m_stuPayrollNode.EnterPayroll.DailyUnits.bPrintDailyUnitsExpense));

	pDailyUnits->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ClearDailyUnits"), IDS_CLEAR_DAILY_UNITS, &m_stuPayrollNode.EnterPayroll.DailyUnits.bClear));

	pEnterPayrollNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PrintEditReport"), IDS_RIGHTS_PRINT_EDIT_REPORT, &m_stuPayrollNode.EnterPayroll.bPrintEditReport));
	// PBI 46406	09/28/2018	mvt - Added security for Internal Control Alerts in PR
	pEnterPayrollNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("SendInternalControlAlerts"), IDS_SEND_INTERNAL_CONTROL_ALERTS, &m_stuPayrollNode.EnterPayroll.bSendInternalControlAlerts));
	pEnterPayrollNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PrintChecks"), IDS_AP_PRINTCHECKS1, &m_stuPayrollNode.EnterPayroll.bPrintChecks));
	pEnterPayrollNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PrintCheckRegister"), IDS_AP_STATUSBAR_PRINT_CHECKS_REGISTER, &m_stuPayrollNode.EnterPayroll.bPrintCheckRegister));
	pEnterPayrollNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PrintPrePosting"), IDS_PREPOSTING_REPORTS, &m_stuPayrollNode.EnterPayroll.bPrintPrePosting));
	pEnterPayrollNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PrintEFTPSReport"), IDS_PRINT_EFTPS_REPORT, &m_stuPayrollNode.EnterPayroll.bPrintEFTPSReport));
	pEnterPayrollNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("GenerateDirectDeposit"), IDS_GENERATE_DIRECT_DEPOSIT, &m_stuPayrollNode.EnterPayroll.bGenerateDirectDeposit));
	pEnterPayrollNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("Post"), IDS_POST_PAYROLL, &m_stuPayrollNode.EnterPayroll.bPost));

	// Enter Payroll - Batch Control
	CUserRightsBaseNode* pBatchNode = CreateNode(_T("BatchControl"), IDS_RIGHTS_BATCH_CONTROL, &m_stuPayrollNode.EnterPayroll.BatchControl.bTotal);
	pEnterPayrollNode->m_tplUserRightsNodeList.AddTail(pBatchNode);
	pBatchNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("SelectBatch"), IDS_RIGHTS_SELECT_BATCH, &m_stuPayrollNode.EnterPayroll.BatchControl.bSelectBatch));
	pBatchNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("SetupBatch"), IDS_RIGHTS_SETUP_BATCH, &m_stuPayrollNode.EnterPayroll.BatchControl.bSetupBatch));

	pEnterPayrollNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ClearTransactions"), IDS_CLEAR_PAYROLL_TRANS, &m_stuPayrollNode.EnterPayroll.bClearTransactions));
	pEnterPayrollNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ChangeLocalTransactionDate"), IDS_RIGHTS_CHANGE_LOCAL_TRANSACTION_DATE, &m_stuPayrollNode.EnterPayroll.bChangeLocalTransactionDate));

	// Reports
	CUserRightsBaseNode* pReportsNode = CreateNode(_T("Reports"), IDS_CTRL_REPORTS, &m_stuPayrollNode.Reports.bTotal);
	pPayrollNode->m_tplUserRightsNodeList.AddTail(pReportsNode);
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("EmployeeList"), IDS_EMPLOYEE_LIST, &m_stuPayrollNode.Reports.bEmployeeList));
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("TaxableWage"), IDS_TAXABLE_WAGE, &m_stuPayrollNode.Reports.bTaxableWage));
	//pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("EmployeeLabels"),	IDS_EMPLOYEE_LABELS,	 &m_stuPayrollNode.Reports.bEmployeeLabels));
	// PBI 22081 jhicks 08/11/2014 program codes if in fund
	if (g_pGBLSystemInformationCMSDll != NULL &&
		g_pGBLSystemInformationCMSDll->CompanySettings.IsFund)
		pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("EmployeeDepartment", IDS_EMPLOYEE_PROGRAM_CODES, &m_stuPayrollNode.Reports.bEmployeeDepartment));
	else
		pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("EmployeeDepartment", IDS_EMPLOYEE_DEPARTMENT_CODES, &m_stuPayrollNode.Reports.bEmployeeDepartment));
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("EmployeeBenDeductions"), IDS_BENEFITS_DEDUCTIONS, &m_stuPayrollNode.Reports.bEmployeeBenDeductions));
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("EmployeeProjectCodes"), IDS_EMP_PROJECT_CODE, &m_stuPayrollNode.Reports.bEmployeeProjectCodes));
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("EmployeeLeave"), IDS_EMPLOYEE_LEAVE, &m_stuPayrollNode.Reports.bEmployeeLeave));
	//	PBI 33584	03/21/2017	mvt - Added security for Estimated Leave Value report
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("EstimatedLeaveValue"), IDS_ESTIMATED_LEAVE_VALUE, &m_stuPayrollNode.Reports.bEstimatedLeaveValue));
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("EmployeeHistory"), IDS_EMPLOYEE_HISTORY, &m_stuPayrollNode.Reports.bEmployeeHistory));
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ReprintPayStubs"), IDS_REPRINT_PAY_STUBS, &m_stuPayrollNode.Reports.bReprintPayStubs));
	//pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ReprintPosting"),	IDS_REPRINT_POSTING,	 &m_stuPayrollNode.Reports.bReprintPosting));
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("TaxesPayable"), IDS_TAXES_PAYABLE, &m_stuPayrollNode.Reports.bTaxesPayable));
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("EFTPSHistory"), IDS_EFTPS_HISTORY, &m_stuPayrollNode.Reports.bEFTPSHistory));
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("QuarterlyTaxPreview"), IDS_QUARTERLY_TAX_PREVIEW, &m_stuPayrollNode.Reports.bQuarterlyTaxPreview));
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("W2Preview"), IDS_W2_PREVIEW, &m_stuPayrollNode.Reports.bW2Preview));
	pReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PayrollTaxForms"), IDS_PAYROLL_TAX_FORMS, &m_stuPayrollNode.Reports.bPayrollTaxForms));

	// Period End
	CUserRightsBaseNode* pPeriodEndNode = CreateNode(_T("PeriodEnd"), IDS_CTRL_PERIOD_END, &m_stuPayrollNode.PeriodEnd.bTotal);
	pPayrollNode->m_tplUserRightsNodeList.AddTail(pPeriodEndNode);
	pPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ProcessPeriodicLeave"), IDS_PROCESS_PERIODIC_LEAVE, &m_stuPayrollNode.PeriodEnd.bProcessPeriodicLeave));
	pPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ResetYearToDateTotals"), IDS_RESET_YEAR_TO_DATE_TOTALS, &m_stuPayrollNode.PeriodEnd.bResetYearToDateTotals));
	//pPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("PurgeHistory"),			IDS_CTRL_PURGE_HISTORY,		&m_stuPayrollNode.PeriodEnd.bPurgeHistory));

	// Options
	CUserRightsBaseNode* pOptionsNode = CreateNode(_T("Options"), IDS_CTRL_OPTIONS, &m_stuPayrollNode.Options.bTotal);
	pPayrollNode->m_tplUserRightsNodeList.AddTail(pOptionsNode);
	pOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ModulePreferences"), IDS_CTRL_MODULE_PREFERENCES, &m_stuPayrollNode.Options.bModulePreferences));

	// PBI 137 BK 11/5/13 Added Codes submenu
	CUserRightsBaseNode* pCodesNode = CreateNode("Codes", IDS_CTRL_CODES, &m_stuPayrollNode.Options.Codes.bTotal);
	pOptionsNode->m_tplUserRightsNodeList.AddTail(pCodesNode);
	// PBI 17830 BK 3/11/14 program codes if in fund
	if (g_pGBLSystemInformationCMSDll != NULL &&
		g_pGBLSystemInformationCMSDll->CompanySettings.IsFund)
		pCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("DeptCodes", IDS_PROGRAM_CODES, &m_stuPayrollNode.Options.Codes.bDeptCodes));
	else
		pCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("DeptCodes", IDS_DEPARTMENT_CODES, &m_stuPayrollNode.Options.Codes.bDeptCodes));
	pCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("ProjectCodes", IDS_PROJECT_CODES, &m_stuPayrollNode.Options.Codes.bProjectCodes));
	pCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("TaxCodes", IDS_TAX_CODES, &m_stuPayrollNode.Options.Codes.bTaxCodes));
	pCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("BenDedCodes", IDS_BEN_DED_CODES, &m_stuPayrollNode.Options.Codes.bBenDedCodes));
	pCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("ShelterCodes", IDS_SHELTER_CODES, &m_stuPayrollNode.Options.Codes.bShelterCodes));
	pCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("LeaveCodes", IDS_LEAVE_CODES, &m_stuPayrollNode.Options.Codes.bLeaveCodes));
	pCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("StatusCodes", IDS_STATUS_CODES, &m_stuPayrollNode.Options.Codes.bStatusCodes));
	//RCG - 05/12/2020 - PBI 51506 - Added security for Overtime Codes
	pCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("OvertimeCodes", IDS_OVERTIME_CODES, &m_stuPayrollNode.Options.Codes.bOvertimeCodes));
	//pCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("RenumberCodes",	IDS_STATUS_CODES,		&m_stuPayrollNode.Options.Codes.bRenumberCodes));
	pCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("UdfCodes", IDS_UDF_CODES, &m_stuPayrollNode.Options.Codes.bUdfCodes));

	//pOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("SetupRecurring"),  IDS_SETUP_RECURRING_PAYROLL,  &m_stuPayrollNode.Options.bSetupRecurring));
	pOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("W2Formats"), IDS_W2_FORMATS, &m_stuPayrollNode.Options.bW2Formats));
	// PBI 46406	09/28/2018	mvt - Added security for Internal Control Alerts in PR
	pOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("ManageInternalControlAlerts"), IDS_MANAGE_INTERNAL_CONTROL_ALERTS, &m_stuPayrollNode.Options.bManageInternalControlAlerts));

	// file maintenance
	CUserRightsBaseNode* pFileMaintenanceNode = CreateNode("FileMaintenance", IDS_CT_TABLE_MAINTENANCE, &m_stuPayrollNode.Options.FileMaintenance.bTotal);
	pOptionsNode->m_tplUserRightsNodeList.AddTail(pFileMaintenanceNode);
	pFileMaintenanceNode->m_tplUserRightsNodeList.AddTail(CreateNode("DisplayFileInformation", IDS_CTRL_DISPLAY_FILE_INFO, &m_stuPayrollNode.Options.FileMaintenance.bDisplayFileInformation));
	//	PBI 13780	10/13/2014	mvt - Added security for Update Taxes menu item
	pFileMaintenanceNode->m_tplUserRightsNodeList.AddTail(CreateNode("UpdateTaxes", IDS_UPDATE_TAX_CODES, &m_stuPayrollNode.Options.FileMaintenance.bUpdateTaxes));
	pFileMaintenanceNode->m_tplUserRightsNodeList.AddTail(CreateNode("RecalculateWages", IDS_RECALCULATE_WAGES, &m_stuPayrollNode.Options.FileMaintenance.bRecalculateWages));

	//// InTimeTec: 11.12.2012.HS - Custom Menus
	//// Custom Menus Node
	//pPayrollNode->m_tplUserRightsNodeList.AddTail(CreateNode("CustomOptions", IDS_CUSTOM_OPTIONS, &m_stuPayrollNode.bCustomOptions));
	//// InTimeTec: End
}
//=================================== End Of PR ================================================================//
//==============================================================================================================//

// creates object tree for the Purchase Order
//=================================== Start Of PO ================================================================//
//================================================================================================================//
// BK 10/04/07 setup PO user rights in tree control
void CGBLUserRightsInformation::CreatePurchaseOrderObjectList()
{
	// Purchase Order (creating Purchase Order node)
	CUserRightsBaseNode* pPurchaseOrderNode = CreateNode("PurchaseOrder", IDS_MODULE_PO, &m_stuModulesNode.bPurchaseOrder);
	m_pUserRightsRootNode->m_tplUserRightsNodeList.AddTail(pPurchaseOrderNode);
	//----------------------------------------------------------------

	// Purchasing (Enter POs)
	CUserRightsBaseNode* pPurchasingNode = CreateNode("Purchasing", IDS_PURCHASING, &m_stuPurchaseOrderNode.EnterPOs.bTotal);
	pPurchaseOrderNode->m_tplUserRightsNodeList.AddTail(pPurchasingNode);
	pPurchasingNode->m_tplUserRightsNodeList.AddTail(CreateNode("EnterEditPO", IDS_ENTER_EDIT_PO, &m_stuPurchaseOrderNode.EnterPOs.bEnterEditPOs));
	pPurchasingNode->m_tplUserRightsNodeList.AddTail(CreateNode("EnterEditReqs", IDS_ENTER_EDIT_REQS, &m_stuPurchaseOrderNode.EnterPOs.bEnterEditReqs));
	pPurchasingNode->m_tplUserRightsNodeList.AddTail(CreateNode("SetupRecurring", IDS_SETUP_RECURRING_PO, &m_stuPurchaseOrderNode.EnterPOs.bSetupRecurringPOs));
	// Generate From
	CUserRightsBaseNode* pGenerateFromNode = CreateNode("GenerateFrom", IDS_GENERATE_PO_FROM, &m_stuPurchaseOrderNode.EnterPOs.ImportFrom.bTotal);
	pPurchasingNode->m_tplUserRightsNodeList.AddTail(pGenerateFromNode);
	pGenerateFromNode->m_tplUserRightsNodeList.AddTail(CreateNode("Inventory", IDS_INVENTORY1, &m_stuPurchaseOrderNode.EnterPOs.ImportFrom.bInventory));
	pGenerateFromNode->m_tplUserRightsNodeList.AddTail(CreateNode("Recurring", IDS_RIGHTS_RECURRING, &m_stuPurchaseOrderNode.EnterPOs.ImportFrom.bRecurring));
	//	PBI 46406	09/27/2018	mvt - Added security for Internal Control Alerts in PO
	pPurchasingNode->m_tplUserRightsNodeList.AddTail(CreateNode("SendInternalControlAlerts", IDS_SEND_INTERNAL_CONTROL_ALERTS, &m_stuPurchaseOrderNode.EnterPOs.bSendInternalControlAlerts));

	// 1-35270 BK 3/7/11 fund (encumbrances) modifications
	if (g_pGBLSystemInformationCMSDll != NULL &&
		g_pGBLSystemInformationCMSDll->CompanySettings.IsFund)
	{
		pPurchasingNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintEditEntry", IDS_RIGHTS_PRINT_EDIT_REPORT, &m_stuPurchaseOrderNode.EnterPOs.bPrintEditReport));
		pPurchasingNode->m_tplUserRightsNodeList.AddTail(CreateNode("PostEntry", IDS_POST_PURCHASE_ORDERS, &m_stuPurchaseOrderNode.EnterPOs.bPost));
	}
	pPurchasingNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintPO", IDS_PRINT_PO_RPT, &m_stuPurchaseOrderNode.EnterPOs.bPrintPOReport));
	pPurchasingNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintRecurring", IDS_MNU_PRINT_RECURRING_PO_REPORT_STATUSBAR, &m_stuPurchaseOrderNode.EnterPOs.bPrintRecurringReport));
	// Active Batch
	CUserRightsBaseNode* pActiveBatchNode = CreateNode("ActiveBatch", IDS_CT_SET_ACTIVE_BATCH_TO, &m_stuPurchaseOrderNode.EnterPOs.ActiveBatch.bTotal);
	pPurchasingNode->m_tplUserRightsNodeList.AddTail(pActiveBatchNode);
	pActiveBatchNode->m_tplUserRightsNodeList.AddTail(CreateNode("Open", IDS_OPEN, &m_stuPurchaseOrderNode.EnterPOs.ActiveBatch.bOpen));
	pActiveBatchNode->m_tplUserRightsNodeList.AddTail(CreateNode("Recurring", IDS_RIGHTS_RECURRING, &m_stuPurchaseOrderNode.EnterPOs.ActiveBatch.bRecurring));
	// 1-35270 BK 3/7/11 fund (encumbrances) modifications
	if (g_pGBLSystemInformationCMSDll != NULL &&
		g_pGBLSystemInformationCMSDll->CompanySettings.IsFund)
	{
		// Batch Control
		CUserRightsBaseNode* pBatchControlNode = CreateNode("BatchControl", IDS_RIGHTS_BATCH_CONTROL, &m_stuPurchaseOrderNode.EnterPOs.BatchControl.bTotal);
		pPurchasingNode->m_tplUserRightsNodeList.AddTail(pBatchControlNode);
		pBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SelectBatch", IDS_RIGHTS_SELECT_BATCH, &m_stuPurchaseOrderNode.EnterPOs.BatchControl.bSelectBatch));
		pBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SetupBatch", IDS_RIGHTS_SETUP_BATCH, &m_stuPurchaseOrderNode.EnterPOs.BatchControl.bSetupBatch));
		pPurchasingNode->m_tplUserRightsNodeList.AddTail(CreateNode("ClearPOs", IDS_MENU_CLEAR_PO, &m_stuPurchaseOrderNode.EnterPOs.bClearPOs));
	}
	pPurchasingNode->m_tplUserRightsNodeList.AddTail(CreateNode("ClearRecurring", IDS_MNU_CLEAR_RECURRING_POS_STATUSBAR, &m_stuPurchaseOrderNode.EnterPOs.bClearRecurringTrans));
	pPurchasingNode->m_tplUserRightsNodeList.AddTail(CreateNode("LocalPurchaseDate", IDS_CT_CLT_DATE, &m_stuPurchaseOrderNode.EnterPOs.bChangeLocalTransactionDate));
	//----------------------------------------------------------------

	// Receiving
	CUserRightsBaseNode* pReceivingNode = CreateNode("Receiving", IDS_RECEIVING, &m_stuPurchaseOrderNode.ReceivePOs.bTotal);
	pPurchaseOrderNode->m_tplUserRightsNodeList.AddTail(pReceivingNode);
	pReceivingNode->m_tplUserRightsNodeList.AddTail(CreateNode("ReceiveItems", IDS_PO_RECEIVE_ITEMS, &m_stuPurchaseOrderNode.ReceivePOs.bReceiveItems));
	pReceivingNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintReceiving", IDS_PRINT_RECEIVING_REPORT, &m_stuPurchaseOrderNode.ReceivePOs.bPrintReceivingReport));
	pReceivingNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintLabels", IDS_PRINT_LABELS, &m_stuPurchaseOrderNode.ReceivePOs.bPrintLabelsReport));
	pReceivingNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintEdit", IDS_RIGHTS_PRINT_EDIT_REPORT, &m_stuPurchaseOrderNode.ReceivePOs.bPrintEditReport));
	//	PBI 46406	09/27/2018	mvt - Added security for Internal Control Alerts in PO
	pReceivingNode->m_tplUserRightsNodeList.AddTail(CreateNode("SendInternalControlAlerts", IDS_SEND_INTERNAL_CONTROL_ALERTS, &m_stuPurchaseOrderNode.ReceivePOs.bSendInternalControlAlerts));
	pReceivingNode->m_tplUserRightsNodeList.AddTail(CreateNode("PostReceiving", IDS_POMENU_POST_RECEIVED_POS_STATUSBAR, &m_stuPurchaseOrderNode.ReceivePOs.bPost));
	// Batch Control
	CUserRightsBaseNode* pBatchControlNode = CreateNode("BatchControl", IDS_RIGHTS_BATCH_CONTROL, &m_stuPurchaseOrderNode.ReceivePOs.BatchControl.bTotal);
	pReceivingNode->m_tplUserRightsNodeList.AddTail(pBatchControlNode);
	pBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SelectBatch", IDS_RIGHTS_SELECT_BATCH, &m_stuPurchaseOrderNode.ReceivePOs.BatchControl.bSelectBatch));
	pBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SetupBatch", IDS_RIGHTS_SETUP_BATCH, &m_stuPurchaseOrderNode.ReceivePOs.BatchControl.bSetupBatch));
	pReceivingNode->m_tplUserRightsNodeList.AddTail(CreateNode("ClearReceiving", IDS_CLEAR_RECEIVING, &m_stuPurchaseOrderNode.ReceivePOs.bClearReceiving));
	pReceivingNode->m_tplUserRightsNodeList.AddTail(CreateNode("LocalReceiveDate", IDS_CT_CLT_DATE, &m_stuPurchaseOrderNode.ReceivePOs.bChangeLocalTransactionDate));
	//----------------------------------------------------------------

	// Reports
	CUserRightsBaseNode* pPOReportsNode = CreateNode("POReports", IDS_CTRL_REPORTS, &m_stuPurchaseOrderNode.Reports.bTotal);
	pPurchaseOrderNode->m_tplUserRightsNodeList.AddTail(pPOReportsNode);
	pPOReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("OpenPORpt", IDS_OPEN_PO, &m_stuPurchaseOrderNode.Reports.bOpenPurchaseOrderReport));
	//	PBI 40936	06/01/2017	mvt - Added security for Open Requisition report
	pPOReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode(_T("OpenRequisitionRpt"), IDS_OPEN_REQUISITION_REPORT, &m_stuPurchaseOrderNode.Reports.bOpenRequisitionReport));
	pPOReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ExpectedItemRpt", IDS_EXPECTED_LATE_ITEMS, &m_stuPurchaseOrderNode.Reports.bExpectedLateItemsReport));
	pPOReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("VendorPerformRpt", IDS_VENDOR_PERFORM, &m_stuPurchaseOrderNode.Reports.bVendorPerformanceReport));
	pPOReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("HistoryRpt", IDS_SE_ENTRY_SLIDER_HISTORY, &m_stuPurchaseOrderNode.Reports.bHistoryReport));
	pPOReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ReprintPO", IDS_POMENU_REPRINT_POS_STATUSBAR, &m_stuPurchaseOrderNode.Reports.bReprintPurchaseOrdersReport));
	//----------------------------------------------------------------

	// Period End
	CUserRightsBaseNode* pPOPeriodEndNode = CreateNode("POPeriodEnd", IDS_CTRL_PERIOD_END, &m_stuPurchaseOrderNode.PeriodEnd.bTotal);
	pPurchaseOrderNode->m_tplUserRightsNodeList.AddTail(pPOPeriodEndNode);
	pPOPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("PurgeHistory", IDS_CTRL_PURGE_HISTORY, &m_stuPurchaseOrderNode.PeriodEnd.bPurgeHistory));
	//----------------------------------------------------------------

	// Options
	CUserRightsBaseNode* pPOOptionsNode = CreateNode("POOptions", IDS_CTRL_OPTIONS, &m_stuPurchaseOrderNode.Options.bTotal);
	pPurchaseOrderNode->m_tplUserRightsNodeList.AddTail(pPOOptionsNode);
	pPOOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("POModPrefs", IDS_CTRL_MODULE_PREFERENCES, &m_stuPurchaseOrderNode.Options.bModulePreferences));
	// Codes
	CUserRightsBaseNode* pPOCodesNode = CreateNode("POCodes", IDS_CTRL_CODES, &m_stuPurchaseOrderNode.Options.Codes.bTotal);
	pPOOptionsNode->m_tplUserRightsNodeList.AddTail(pPOCodesNode);
	pPOCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("Buyer", IDS_BUYER_CODES, &m_stuPurchaseOrderNode.Options.Codes.bBuyer));
	pPOCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("Comment", IDS_COMMENTCODES, &m_stuPurchaseOrderNode.Options.Codes.bComment));
	pPOCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("Department", IDS_DEPARTMENT_CODES, &m_stuPurchaseOrderNode.Options.Codes.bDepartment));
	pPOCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("Frequency", IDS_FREQUENCYCODES, &m_stuPurchaseOrderNode.Options.Codes.bFrequency));
	pPOCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("ShipVia", IDS_SA_SHIP_VIA_CODE_TITLE, &m_stuPurchaseOrderNode.Options.Codes.bShipVia));
	pPOCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("UdfCodes", IDS_UDF_CODES, &m_stuPurchaseOrderNode.Options.Codes.bUdfCodes));
	pPOOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ScreenLayout", IDS_SET_UP_PUR__RCV_LAYOUT_STATUSBAR, &m_stuPurchaseOrderNode.Options.bSetupScreenLayout));
	//	PBI 46406	09/27/2018	mvt - Added security for Internal Control Alerts in PO
	CUserRightsBaseNode* pInternalControlAlertsNode = CreateNode("InternalControlAlerts", IDS_MANAGE_INTERNAL_CONTROL_ALERTS, &m_stuPurchaseOrderNode.Options.bManageInternalControlAlerts);
	pPOOptionsNode->m_tplUserRightsNodeList.AddTail(pInternalControlAlertsNode);
	pInternalControlAlertsNode->m_tplUserRightsNodeList.AddTail(CreateNode("Receiving", IDS_RECEIVING, &m_stuPurchaseOrderNode.Options.InternalControlAlerts.bReceiving));
	if (g_pGBLSystemInformationCMSDll != NULL &&
		g_pGBLSystemInformationCMSDll->CompanySettings.IsFund)
		pInternalControlAlertsNode->m_tplUserRightsNodeList.AddTail(CreateNode("Purchasing", IDS_PURCHASING, &m_stuPurchaseOrderNode.Options.InternalControlAlerts.bPurchasing));
	pPOOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PreIssuePO", IDS_PRE_ISSUEPO, &m_stuPurchaseOrderNode.Options.bPreIssuePOs));
	// Table Maintenance
	CUserRightsBaseNode* pTableMaintNode = CreateNode("TableMaint", IDS_CT_TABLE_MAINTENANCE, &m_stuPurchaseOrderNode.Options.FileMaintenanceNode.bTotal);
	pPOOptionsNode->m_tplUserRightsNodeList.AddTail(pTableMaintNode);
	pTableMaintNode->m_tplUserRightsNodeList.AddTail(CreateNode("DisplayTable", IDS_CT_DISPLAY_FILE_INFO, &m_stuPurchaseOrderNode.Options.FileMaintenanceNode.bDisplayFileInformation));

	// InTimeTec: 11.12.2012.HS - Custom Menus
	// Custom Menus Node
	pPurchaseOrderNode->m_tplUserRightsNodeList.AddTail(CreateNode("CustomOptions", IDS_CUSTOM_OPTIONS, &m_stuPurchaseOrderNode.bCustomOptions));
	// InTimeTec: End
}
//=================================== End Of PO ================================================================//
//==============================================================================================================//

// creates object tree for the Sales Maintenance
//=================================== Start Of SA ================================================================//
//================================================================================================================//
void CGBLUserRightsInformation::CreateSalesMaintenanceObjectList()
{
	// creating Sales Maintenance node
	CUserRightsBaseNode* pSalesMaintenanceNode = CreateNode("SalesMaintenance", IDS_SALES, &m_stuModulesNode.bSalesMaintenance); //1-29345 Sales Maintenance changed to Sales
	m_pUserRightsRootNode->m_tplUserRightsNodeList.AddTail(pSalesMaintenanceNode);

	// Order Entry Menu
	CUserRightsBaseNode* pSMOrderEntryNode = CreateNode("ORDERENTRY", IDS_USER_RIGHTS_ORDER_ENTRY, &m_stuSalesMaintenanceNode.OrderEntry.bTotal);
	pSalesMaintenanceNode->m_tplUserRightsNodeList.AddTail(pSMOrderEntryNode);

	pSMOrderEntryNode->m_tplUserRightsNodeList.AddTail(CreateNode("EnterOrders", IDS_USER_RIGHTS_ENTER_ORDERS, &m_stuSalesMaintenanceNode.OrderEntry.bEnterOrders));
	// Generate Orders From	: Has Children
	CUserRightsBaseNode* pGenerateOrdersFromNode = CreateNode("GenerateOrdersFrom", IDS_CT_GENERATE_ORDERS_FROM, &m_stuSalesMaintenanceNode.OrderEntry.GenerateOrdersFrom.bTotal);
	pSMOrderEntryNode->m_tplUserRightsNodeList.AddTail(pGenerateOrdersFromNode);
	pGenerateOrdersFromNode->m_tplUserRightsNodeList.AddTail(CreateNode("Recurring", IDS_CTRL_RECURRING, &m_stuSalesMaintenanceNode.OrderEntry.GenerateOrdersFrom.bRecurring));
	pGenerateOrdersFromNode->m_tplUserRightsNodeList.AddTail(CreateNode("ExternalFiles", IDS_CT_EXTERNAL_FILES, &m_stuSalesMaintenanceNode.OrderEntry.GenerateOrdersFrom.bExternalFiles));
	pGenerateOrdersFromNode->m_tplUserRightsNodeList.AddTail(CreateNode("b3dCart", IDS_3DCART, &m_stuSalesMaintenanceNode.OrderEntry.GenerateOrdersFrom.b3dCart)); // JEPK 2/19/2015 pbi 27598 user rights
	//			pGenerateOrdersFromNode->m_tplUserRightsNodeList.AddTail(CreateNode("ECommerce", IDS_CT_EXTERNAL_FILES, &m_stuSalesMaintenanceNode.OrderEntry.GenerateOrdersFrom.bECommerce));
	//End generate orders node.
	pSMOrderEntryNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintEditReport", IDS_MSG_PRINT_EDIT_REPORT, &m_stuSalesMaintenanceNode.OrderEntry.bPrintEditReport));
	pSMOrderEntryNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintPickingReport", IDS_CT_PRINT_PICKING_REPORT, &m_stuSalesMaintenanceNode.OrderEntry.bPrintPickingReport));
	pSMOrderEntryNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintInvoices", IDS_CT_PRINT_INVOICES, &m_stuSalesMaintenanceNode.OrderEntry.bPrintInvoices));
	pSMOrderEntryNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintShippingLabels", IDS_CT_PRINT_SHIPPING_LABELS, &m_stuSalesMaintenanceNode.OrderEntry.bPrintShippingLabels));
	pSMOrderEntryNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintPackingSlips", IDS_CT_PRINT_PACKING_SLIPS, &m_stuSalesMaintenanceNode.OrderEntry.bPrintPackingSlips));
	// 1-33966 BK 5/10/10 PA-DSS removed credit card items
	//pSMOrderEntryNode->m_tplUserRightsNodeList.AddTail(CreateNode("AuthorizeCreditCards", IDS_CT_AUTHORIZE_CREDIT_CARDS, &m_stuSalesMaintenanceNode.OrderEntry.bAuthorizeCreditCards));
	pSMOrderEntryNode->m_tplUserRightsNodeList.AddTail(CreateNode("PostInvoices", IDS_CT_POST_INVOICES, &m_stuSalesMaintenanceNode.OrderEntry.bPostInvoices));
	// Set Active Batch To : Has Children
	CUserRightsBaseNode* pSetActiveBatchToNode = CreateNode("SetActiveBatchTo", IDS_CT_SET_ACTIVE_BATCH_TO, &m_stuSalesMaintenanceNode.OrderEntry.SetActiveBatchTo.bTotal);
	pSMOrderEntryNode->m_tplUserRightsNodeList.AddTail(pSetActiveBatchToNode);
	pSetActiveBatchToNode->m_tplUserRightsNodeList.AddTail(CreateNode("BackOrder", IDS_CT_BACKORDER, &m_stuSalesMaintenanceNode.OrderEntry.SetActiveBatchTo.bBackOrder));
	pSetActiveBatchToNode->m_tplUserRightsNodeList.AddTail(CreateNode("Quote", IDS_CT_QUOTE, &m_stuSalesMaintenanceNode.OrderEntry.SetActiveBatchTo.bQuote));
	pSetActiveBatchToNode->m_tplUserRightsNodeList.AddTail(CreateNode("Recurring", IDS_CTRL_RECURRING, &m_stuSalesMaintenanceNode.OrderEntry.SetActiveBatchTo.bRecurrng));
	pSetActiveBatchToNode->m_tplUserRightsNodeList.AddTail(CreateNode("System", IDS_USER_RIGHTS_SET_ACTIVE_BATCH_TO_SYSTEM, &m_stuSalesMaintenanceNode.OrderEntry.SetActiveBatchTo.bSystem));
	// End Set Active Batch To
	// Batch Control	: Has Children
	CUserRightsBaseNode* pBatchControlNode = CreateNode("BatchControl", IDS_CTRL_BATCH_CONTROL, &m_stuSalesMaintenanceNode.OrderEntry.BatchControl.bTotal);
	pSMOrderEntryNode->m_tplUserRightsNodeList.AddTail(pBatchControlNode);
	pBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SelectBatch", IDS_SELECTBATCH, &m_stuSalesMaintenanceNode.OrderEntry.BatchControl.bSelectBatch));
	pBatchControlNode->m_tplUserRightsNodeList.AddTail(CreateNode("SetUpBatch", IDS_SET_UP_BATCH, &m_stuSalesMaintenanceNode.OrderEntry.BatchControl.bSetupBatch));
	// Clear Orders
	pSMOrderEntryNode->m_tplUserRightsNodeList.AddTail(CreateNode("ClearOrders", IDS_CT_CLEAR_ORDERS, &m_stuSalesMaintenanceNode.OrderEntry.bClearOrders));
	// Change Local Transaction Date
	pSMOrderEntryNode->m_tplUserRightsNodeList.AddTail(CreateNode("ChangeLocalTransactionDate", IDS_CT_CLT_DATE, &m_stuSalesMaintenanceNode.OrderEntry.bChangeLocalTransactionDate));
	//Repair Batch Utility DAJ 5/6/5 err: 1-17058
	pSMOrderEntryNode->m_tplUserRightsNodeList.AddTail(CreateNode("RepairBatchUtility", IDS_SA_RBU_TITLE, &m_stuSalesMaintenanceNode.OrderEntry.bRepairBatchUtility));

	// Point Of Sale Menu
	CUserRightsBaseNode* pSMPointOfSaleNode = CreateNode("POSMANAGER", IDS_USER_RIGHTS_POS, &m_stuSalesMaintenanceNode.PointOfSale.bTotal);
	pSalesMaintenanceNode->m_tplUserRightsNodeList.AddTail(pSMPointOfSaleNode);
	pSMPointOfSaleNode->m_tplUserRightsNodeList.AddTail(CreateNode("EnterSales", IDS_USER_RIGHTS_ENTER_SALES, &m_stuSalesMaintenanceNode.PointOfSale.bEnterSales));
	pSMPointOfSaleNode->m_tplUserRightsNodeList.AddTail(CreateNode("RegisterStatus", IDS_CT_REGISTER_STATUS, &m_stuSalesMaintenanceNode.PointOfSale.bRegisterStatus));
	pSMPointOfSaleNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintEditReport", IDS_MSG_PRINT_EDIT_REPORT, &m_stuSalesMaintenanceNode.PointOfSale.bPrintEditReport));
	pSMPointOfSaleNode->m_tplUserRightsNodeList.AddTail(CreateNode("BalanceRegister", IDS_CT_BALANCE_REGISTER, &m_stuSalesMaintenanceNode.PointOfSale.bBalanceRegister));
	pSMPointOfSaleNode->m_tplUserRightsNodeList.AddTail(CreateNode("PostTransactions", IDS_CT_POST_TRANSACTIONS, &m_stuSalesMaintenanceNode.PointOfSale.bPostTransactions));
	pSMPointOfSaleNode->m_tplUserRightsNodeList.AddTail(CreateNode("PrintDailyRegisterTotals", IDS_CT_PRINT_DAILY_REGISTER_TOTALS, &m_stuSalesMaintenanceNode.PointOfSale.bPrintDailyRegisterTotals));
	pSMPointOfSaleNode->m_tplUserRightsNodeList.AddTail(CreateNode("ClearSales", IDS_CT_CLEAR_SALES, &m_stuSalesMaintenanceNode.PointOfSale.bClearSales));
	pSMPointOfSaleNode->m_tplUserRightsNodeList.AddTail(CreateNode("RepairRegisterUtility", IDS_CT_REPAIR_REGISTER_UTILITY, &m_stuSalesMaintenanceNode.PointOfSale.bRepairRegisterUtility));

	// Reports Menu
	CUserRightsBaseNode* pSMReportsNode = CreateNode("Reports", IDS_CTRL_REPORTS, &m_stuSalesMaintenanceNode.Reports.bTotal);
	pSalesMaintenanceNode->m_tplUserRightsNodeList.AddTail(pSMReportsNode);
	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("SalesBySalesperson", IDS_CT_SALES_BY_SALESPERSON, &m_stuSalesMaintenanceNode.Reports.bSalesBySalesPerson));
	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("SalesByDepartment", IDS_CT_SALES_BY_DEPARTMENT, &m_stuSalesMaintenanceNode.Reports.bSalesByDepartment));
	// PBI 21523 jhicks 09/02/2014 Pure Product Sales Report
	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PureProductSalesReport", IDS_CT_PURE_PRODUCT_SALES, &m_stuSalesMaintenanceNode.Reports.bPureProductSales));

	// 1-19511 BK 6/7/05 changed from Sales Tax to Sales by Item
	//pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("SalesByItem", IDS_SALES_TAX, &m_stuSalesMaintenanceNode.Reports.bSalesByItem));
	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("SalesByItem", IDS_SALES_BY_ITEM, &m_stuSalesMaintenanceNode.Reports.bSalesByItem));
	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("SalesByRegister", IDS_CT_SALES_BY_REGISTER, &m_stuSalesMaintenanceNode.Reports.bSalesByRegister));
	//	PBI 50684	06/25/2019	mvt - Added Sales by State/Region report
	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("SalesByStateRegion", IDS_SA_SALES_BY_STATE_REGION, &m_stuSalesMaintenanceNode.Reports.bSalesByStateRegion));

	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("CashReceipts", IDS_CT_CASH_RECEIPTS, &m_stuSalesMaintenanceNode.Reports.bCashReceipts));
	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("CreditCardReceipts", IDS_CT_CREDIT_CARD_RECEIPTS, &m_stuSalesMaintenanceNode.Reports.bCreditCardReceipts));
	// PBI 19188 BK 5/6/14 Added new MerchantWARE Transactions Report form
	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("MerchantWareReport", IDS_MERCHANTWARE_TRANSACTIONS, &m_stuSalesMaintenanceNode.Reports.bMerchantWareReport));
	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PriceVariance", IDS_USER_RIGHTS_PRICE_VARIANCE, &m_stuSalesMaintenanceNode.Reports.bPriceVariance));

	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("SalesTax", IDS_SALES_TAX, &m_stuSalesMaintenanceNode.Reports.bSalesTax));
	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ProfitMargin", IDS_CT_PROFIT_MARGIN, &m_stuSalesMaintenanceNode.Reports.bProfitMargin));
	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("CouponBuyDown", IDS_USER_RIGHTS_COUPON_BUY_DOWN, &m_stuSalesMaintenanceNode.Reports.bCouponBuy));
	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("PromotionalSales", IDS_PROMOTIONAL_SALES, &m_stuSalesMaintenanceNode.Reports.bPromotionalSales)); // begbert 11-18-2006 Feature 28

	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("SalesHistory", IDS_CT_SALES_HISTORY, &m_stuSalesMaintenanceNode.Reports.bSalesHistory));
	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("SalesLabels", IDS_CT_SALES_LABELS, &m_stuSalesMaintenanceNode.Reports.bSalesLabels));
	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("InvoiceTracking", IDS_USER_RIGHTS_INV_RCPT_TRACKING, &m_stuSalesMaintenanceNode.Reports.bInvoiceTracking));
	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("ReprintInvoicesFromHistory", IDS_USER_RIGHTS_REPRINT_INV_RCPT, &m_stuSalesMaintenanceNode.Reports.bReprintInvoicesFromHistory));

	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("BackOrderTracking", IDS_CT_BACK_ORDER_TRACKING, &m_stuSalesMaintenanceNode.Reports.bBackOrderTracking));
	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("BackOrderFulfillment", IDS_CT_BACK_ORDER_FULFILLMENT, &m_stuSalesMaintenanceNode.Reports.bBackOrderFulfillment));
	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("DepositJournalReport", IDS_CT_DEPOSIT_JOURNAL_REPORT, &m_stuSalesMaintenanceNode.Reports.bDepositJournalReport));
	//	PBI 36910	09/30/2016	mvt - Added Recurring Invoice Report
	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("RecurringInvoiceReport", IDS_CT_RECURRING_INVOICE_REPORT, &m_stuSalesMaintenanceNode.Reports.bRecurringInvoiceReport));

	// DG.01.19.2005 - Modification made for Gift Certificate Report - root cause of error 16001
	pSMReportsNode->m_tplUserRightsNodeList.AddTail(CreateNode("GiftCertificateReport", IDS_USER_RIGHTS_GIFT_CERT_REPORT, &m_stuSalesMaintenanceNode.Reports.bGiftCertificateReport));

	// Layaway : Has Children
	CUserRightsBaseNode* pSMReportsLayawayNode = CreateNode("Layaway", IDS_LAYAWAY, &m_stuSalesMaintenanceNode.Reports.Layaway.bTotal);
	pSMReportsNode->m_tplUserRightsNodeList.AddTail(pSMReportsLayawayNode);
	//RCG - 12/22/2005 - 1-21353
	//pSMReportsLayawayNode->m_tplUserRightsNodeList.AddTail(CreateNode("LayawayTranReport", IDS_LAYAWAYS_BY_TRANS_RPT, &m_stuSalesMaintenanceNode.Reports.Layaway.bLayawayTransReport)) ;
	pSMReportsLayawayNode->m_tplUserRightsNodeList.AddTail(CreateNode("LayawayTranReport", IDS_LAYAWAY_HISTORY_REPORT, &m_stuSalesMaintenanceNode.Reports.Layaway.bLayawayTransReport));
	pSMReportsLayawayNode->m_tplUserRightsNodeList.AddTail(CreateNode("ByStockItem", IDS_USER_RIGHTS_LAYAWAY_RPT_BY_STOCK_ITEM, &m_stuSalesMaintenanceNode.Reports.Layaway.bByStockItem));
	pSMReportsLayawayNode->m_tplUserRightsNodeList.AddTail(CreateNode("BySalesPerson", IDS_USER_RIGHTS_LAYAWAY_RPT_BY_SALESPERSON, &m_stuSalesMaintenanceNode.Reports.Layaway.bBySalesPerson));
	pSMReportsLayawayNode->m_tplUserRightsNodeList.AddTail(CreateNode("ByRegister", IDS_USER_RIGHTS_LAYAWAY_RPT_BY_REGISTER, &m_stuSalesMaintenanceNode.Reports.Layaway.bByRegister));
	pSMReportsLayawayNode->m_tplUserRightsNodeList.AddTail(CreateNode("Delinquencies", IDS_USER_RIGHTS_LAYAWAY_RPT_DELINQUENCIES, &m_stuSalesMaintenanceNode.Reports.Layaway.bDelinquencies));
	pSMReportsLayawayNode->m_tplUserRightsNodeList.AddTail(CreateNode("Schedule", IDS_USER_RIGHTS_LAYAWAY_RPT_SCHEDULE, &m_stuSalesMaintenanceNode.Reports.Layaway.bSchedule));
	// End Layaway

	// Period End Menu
	CUserRightsBaseNode* pSMPeriodEndNode = CreateNode("PeriodEnd", IDS_CTRL_PERIOD_END, &m_stuSalesMaintenanceNode.PeriodEnd.bTotal);
	pSalesMaintenanceNode->m_tplUserRightsNodeList.AddTail(pSMPeriodEndNode);

	pSMPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("PurgeSettledChecks", IDS_CT_PURGE_SETTLED_CHECKS, &m_stuSalesMaintenanceNode.PeriodEnd.bPurgeSettledChecks));
	pSMPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("PurgeHoldQuotes", IDS_CT_PURGE_HOLD_QUOTES, &m_stuSalesMaintenanceNode.PeriodEnd.bPurgeHoldQuotes));
	pSMPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("ResetCreditCardTotals", IDS_CT_RESET_CREDIT_CARD_TOTALS, &m_stuSalesMaintenanceNode.PeriodEnd.bResetCreditCardTotals));
	pSMPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("PurgeDepositJournal", IDS_CT_PURGE_DEPOSIT_JOURNAL, &m_stuSalesMaintenanceNode.PeriodEnd.bPurgeDepositJournal));
	pSMPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("PurgeClosedLayaways", IDS_USER_RIGHTS_PURGE_CLOSED_LAYAWAYS, &m_stuSalesMaintenanceNode.PeriodEnd.bPurgeClosedLayaways));
	pSMPeriodEndNode->m_tplUserRightsNodeList.AddTail(CreateNode("PurgeHistory", IDS_CTRL_PURGE_HISTORY, &m_stuSalesMaintenanceNode.PeriodEnd.bPurgeHistory));

	// Options Menu
	CUserRightsBaseNode* pSMOptionsNode = CreateNode("Options", IDS_CTRL_OPTIONS, &m_stuSalesMaintenanceNode.Options.bTotal);
	pSalesMaintenanceNode->m_tplUserRightsNodeList.AddTail(pSMOptionsNode);
	pSMOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("OEPreferences", IDS_USER_RIGHTS_OE_PREFERENCES, &m_stuSalesMaintenanceNode.Options.bOEPreferences));
	pSMOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("POSPreferences", IDS_USER_RIGHTS_POS_PREFERENCES, &m_stuSalesMaintenanceNode.Options.bPOSPreferences));
	pSMOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("SetupSalesEntryScreen", IDS_CT_SETUP_SALES_ENTRY_SCREEN, &m_stuSalesMaintenanceNode.Options.bSetupSalesEntryScreen));
	pSMOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("SetUpItemButtons", IDS_CT_ITEM_BUTTONS, &m_stuSalesMaintenanceNode.Options.bSetUpItemButtons));
	pSMOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("b3dCartIntegration", IDS_3DCART_INTEGRATION, &m_stuSalesMaintenanceNode.Options.b3dCartIntegration)); // JEPK 2/19/2015 pbi 27598 user rights
	// TEMP BK 10/15/03 added to user Layaway Menu (DIDN'T ADD Struct)
	pSMOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("LayawayPreferences", IDS_LAYAWAY_PREFS, &m_stuSalesMaintenanceNode.Options.bLayawayPreferences));
	pSMOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("GiftCertificateOptions", IDS_USER_RIGHTS_GIFT_CERT_OPTIONS, &m_stuSalesMaintenanceNode.Options.bGiftCertificateOptions));
	pSMOptionsNode->m_tplUserRightsNodeList.AddTail(CreateNode("bAvataxIntegration", IDS_AVATAX_INTEGRATION, &m_stuSalesMaintenanceNode.Options.bAvataxIntegration)); // JEPK 6/30/2015 pbi 27453 AvaTax user rights

	// Codes : Has Children
	CUserRightsBaseNode* pSMCodesNode = CreateNode("Codes", IDS_CTRL_CODES, &m_stuSalesMaintenanceNode.Options.Codes.bTotal);
	pSMOptionsNode->m_tplUserRightsNodeList.AddTail(pSMCodesNode);
	pSMCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("SalesDepartment", IDS_SAF_SALES_DEPARTMENT, &m_stuSalesMaintenanceNode.Options.Codes.bSalesDepartment));
	//GC 1/20/2005 1-15858 Change IDS_SALES_PERSON to IDS_CTRL_SALESPERSON
	pSMCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("SalesPerson", IDS_CTRL_SALESPERSON, &m_stuSalesMaintenanceNode.Options.Codes.bSalesPerson));
	pSMCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("SalesTax", IDS_SALES_TAX, &m_stuSalesMaintenanceNode.Options.Codes.bSalesTax));
	// Non Inventory :Has Children
	CUserRightsBaseNode* pSMNonInventoryNode = CreateNode("NonInventory", IDS_NON_INVENTORY, &m_stuSalesMaintenanceNode.Options.Codes.NonInventory.bTotal);
	pSMCodesNode->m_tplUserRightsNodeList.AddTail(pSMNonInventoryNode);
	pSMNonInventoryNode->m_tplUserRightsNodeList.AddTail(CreateNode("AlternateTender", IDS_CT_ALTERNATE_TENDER, &m_stuSalesMaintenanceNode.Options.Codes.NonInventory.bAlternateTender));
	pSMNonInventoryNode->m_tplUserRightsNodeList.AddTail(CreateNode("Normal", IDS_CT_NORMAL, &m_stuSalesMaintenanceNode.Options.Codes.NonInventory.bNormal));
	pSMNonInventoryNode->m_tplUserRightsNodeList.AddTail(CreateNode("PaidOut", IDS_CT_PAID_OUT, &m_stuSalesMaintenanceNode.Options.Codes.NonInventory.bPaidOut));
	pSMNonInventoryNode->m_tplUserRightsNodeList.AddTail(CreateNode("CouponBuyDown", IDS_CT_COUPONBUY_DOWN, &m_stuSalesMaintenanceNode.Options.Codes.NonInventory.bCouponBuyDown));
	pSMCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("CreditCards", IDS_CT_CREDIT_CARDS, &m_stuSalesMaintenanceNode.Options.Codes.bCreditCards));

	pSMCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("Discount", IDS_DISCOUNT, &m_stuSalesMaintenanceNode.Options.Codes.bDiscount));
	pSMCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("PriceLevel", IDS_LBL_PRICELEVEL, &m_stuSalesMaintenanceNode.Options.Codes.bPriceLevel));
	pSMCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("MarkupMarginCodes", IDS_CT_MARKUPMARGIN_CODES, &m_stuSalesMaintenanceNode.Options.Codes.bMarkupMarginCodes));
	pSMCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("CustomerSpecialPricing", IDS_CT_CUSTOMER_SPECIAL_PRICING, &m_stuSalesMaintenanceNode.Options.Codes.bCustomerSpecialPricing));

	pSMCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("Register", IDS_CT_REGISTER, &m_stuSalesMaintenanceNode.Options.Codes.bRegister));
	pSMCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("InvoiceFormat", IDS_CT_INVOICE_FORMAT, &m_stuSalesMaintenanceNode.Options.Codes.bInvoiceFormat));
	pSMCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("Devices", IDS_DEVICES, &m_stuSalesMaintenanceNode.Options.Codes.bDevices));
	pSMCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("Printer", IDS_PRINTER, &m_stuSalesMaintenanceNode.Options.Codes.bPrinter));

	pSMCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("FrequencyCodes", IDS_FREQUENCYCODES, &m_stuSalesMaintenanceNode.Options.Codes.bFrequencyCodes)); //Frequency Codes
	pSMCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("Comment", IDS_COMMENT, &m_stuSalesMaintenanceNode.Options.Codes.bComment));
	pSMCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("CustomTrackingCCCode", IDS_USER_RIGHTS_CUSTOM_TRACKING_CODES, &m_stuSalesMaintenanceNode.Options.Codes.bCustomTrackingCCCode));
	pSMCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("MaintainFlaggedChecks", IDS_CT_MAINTAIN_FLAGGED_CHECKS, &m_stuSalesMaintenanceNode.Options.Codes.bMaintainFlaggedChecks));
	pSMCodesNode->m_tplUserRightsNodeList.AddTail(CreateNode("ShipVia", IDS_SHIP_VIA, &m_stuSalesMaintenanceNode.Options.Codes.bShipVia));
	//Table Maintenance
	CUserRightsBaseNode* pSMTableMaintenanceNode = CreateNode("TableMaintenance", IDS_CT_TABLE_MAINTENANCE, &m_stuSalesMaintenanceNode.Options.TableMaintenance.bTotal);
	pSMOptionsNode->m_tplUserRightsNodeList.AddTail(pSMTableMaintenanceNode);
	pSMTableMaintenanceNode->m_tplUserRightsNodeList.AddTail(CreateNode("DisplayTableInformation", IDS_CTRL_DISPLAY_FILE_INFO, &m_stuSalesMaintenanceNode.Options.TableMaintenance.bDisplayTableInformation));

	// InTimeTec: 11.12.2012.HS - Custom Menus
	// Custom Menus Node
	pSalesMaintenanceNode->m_tplUserRightsNodeList.AddTail(CreateNode("CustomOptions", IDS_CUSTOM_OPTIONS, &m_stuSalesMaintenanceNode.bCustomOptions));
	// InTimeTec: End
}

//=================================== End Of SA ================================================================//
//==============================================================================================================//
// 1-34642/1-34645 BK 8/19/10 added submenus to Cougar Dtails, financial & sales dashboards
// begbert 02-05-2010 : Added this
void CGBLUserRightsInformation::CreateCougarDtailsObjectList()
{
	// creating cougar dtails node
	CUserRightsBaseNode* pCougarDtailsNode = CreateNode("CougarDtails", IDS_COUGAR_DTAILS, &m_stuModulesNode.CougarDtails.bTotal); // Cougar Dtails
	m_pUserRightsRootNode->m_tplUserRightsNodeList.AddTail(pCougarDtailsNode);

	// financial dashboard node
	pCougarDtailsNode->m_tplUserRightsNodeList.AddTail(CreateNode("FinancialDashboard", IDS_FINANCIAL_DASHBOARD, &m_stuModulesNode.CougarDtails.bFinancialDashboard));
	// sales dashboard node
	pCougarDtailsNode->m_tplUserRightsNodeList.AddTail(CreateNode("SalesDashboard", IDS_SALES_DASHBOARD, &m_stuModulesNode.CougarDtails.bSalesDashboard));
}
//==============================================================================================================//

// this function will save user the data to the server
// this is the single public function called by outside functions to save changes to the server
CString CGBLUserRightsInformation::UpdateUserToServer(const CString& szCompanyID, const CString& szUserID)
{
	CXMLParams clsCommand;
	clsCommand.MakeParam("COMMAND", "UPDATE");
	clsCommand.MakeParam("MODULE", "GB");
	clsCommand.MakeParam("OBJECT", "CMSUSERCOMPANIES");
	CXMLParams clsData;
	CXMLParams clsTable(true);
	clsTable.SetRoot("TABLE");
	clsTable.MakeParam("strTenantID", g_pGBLSystemInformationCMSDll->ApplicationSettings.TenantID);
	clsTable.MakeParam("strCompanyID", szCompanyID);
	clsTable.MakeParam("strUserID", szUserID);
	CXMLParams userRightsRoot(true);
	userRightsRoot.SetRoot("UserRightsInformation");
	m_pUserRightsRootNode->BuildXML(&userRightsRoot);
	// encrypting data
	//	CGBLMath gblMath;
	CString szXMLParameterString = userRightsRoot.GetXML();
	CString szReturn;
	szReturn = CGBLMath::EncryptData(szXMLParameterString);
	clsTable.MakeParam("txtSecuritySettings", szReturn);
	clsData.AppendXMLParam(&clsTable);

	CXMLParams::AppendXMLConnection(&clsData, CXMLParams::CN_PRIMARY, "CONNECTION");
	CXMLParams::AppendCompanyInfo(&clsData);
	CXML clsXML;

	return clsXML.SendXML(clsCommand.GetXML(), clsData.GetXML()).copy();
}

// this routine will build and encrypt the XML string to be stored in the txtSecuritySettings field
CString CGBLUserRightsInformation::BuildXML()
{
	CXMLParams userRightsRoot(true);
	userRightsRoot.SetRoot("UserRightsInformation");
	m_pUserRightsRootNode->BuildXML(&userRightsRoot);
	CString szXMLParameterString = userRightsRoot.GetXML();
	return CGBLMath::EncryptData(szXMLParameterString);
}

// this function will save group the data to the server
// this is the single public function called by outside functions to save changes to the server
CString CGBLUserRightsInformation::UpdateGroupToServer(const CString& szCompanyID, const CString& szGroupID)
{
	CXMLParams clsCommand;
	clsCommand.MakeParam("COMMAND", "UPDATE");
	clsCommand.MakeParam("MODULE", "GB");
	clsCommand.MakeParam("OBJECT", "CMSUSERS");
	CXMLParams clsData;
	CXMLParams clsTable(true);
	clsTable.SetRoot("TABLE");
	clsTable.MakeParam("strTenantID", g_pGBLSystemInformationCMSDll->ApplicationSettings.TenantID);
	clsTable.MakeParam("strCompanyID", szCompanyID);
	clsTable.MakeParam("strGroupID", szGroupID);
	CXMLParams userRightsRoot(true);
	userRightsRoot.SetRoot("UserRightsInformation");
	m_pUserRightsRootNode->BuildXML(&userRightsRoot);
	// encrypting data
	// CGBLMath gblMath;
	CString szXMLParameterString = userRightsRoot.GetXML();
	CString szReturn;
	szReturn = CGBLMath::EncryptData(szXMLParameterString);
	clsTable.MakeParam("txtSecuritySettings", szReturn);
	clsData.AppendXMLParam(&clsTable);

	CXMLParams::AppendXMLConnection(&clsData, CXMLParams::CN_PRIMARY, "CONNECTION");
	CXMLParams::AppendCompanyInfo(&clsData);
	CXML clsXML;

	return clsXML.SendXML(clsCommand.GetXML(), clsData.GetXML()).copy();
}

// this will retrieve the XML string from the server for a user
// this is a private function only called from within this object
CString CGBLUserRightsInformation::GetUserXMLFromServer(const CString& szCompanyID, const CString& szUserID)
{
	CXMLParams clsCommand;
	clsCommand.MakeParam("COMMAND", "GET");
	clsCommand.MakeParam("MODULE", "GB");
	clsCommand.MakeParam("OBJECT", "CMSUSERS");

	CXMLParams clsData;
	clsData.MakeParam("DIRECTION", "CURRENT");
	CXMLParams clsTable(true);
	clsTable.SetRoot("TABLE");
	CXMLParams clsRecord(true);
	clsRecord.SetRoot("RECORD");
	clsTable.MakeParam("strTenantID", g_pGBLSystemInformationCMSDll->ApplicationSettings.TenantID);
	clsRecord.MakeParam("strCompanyID", szCompanyID);
	clsRecord.MakeParam("strUserID", szUserID);
	clsTable.AppendXMLParam(&clsRecord);
	clsData.AppendXMLParam(&clsTable);
	CXMLParams::AppendXMLConnection(&clsData, CXMLParams::CN_PRIMARY, "CONNECTION");
	CXMLParams::AppendCompanyInfo(&clsData);
	CXML clsXML;

	return clsXML.SendXML(clsCommand.GetXML(), clsData.GetXML()).copy();
}

// this will retrieve the XML string from the server a group
// this is a private function only called from within this object
CString CGBLUserRightsInformation::GetGroupXMLFromServer(const CString& szCompanyID, const CString& szGroupID)
{
	CXMLParams clsCommand;
	clsCommand.MakeParam("COMMAND", "GET");
	clsCommand.MakeParam("MODULE", "GB");
	clsCommand.MakeParam("OBJECT", "CMSGROUPS");

	CXMLParams clsData;
	clsData.MakeParam("DIRECTION", "CURRENT");
	CXMLParams clsTable(true);
	clsTable.SetRoot("TABLE");
	CXMLParams clsRecord(true);
	clsRecord.SetRoot("RECORD");
	clsTable.MakeParam("strTenantID", g_pGBLSystemInformationCMSDll->ApplicationSettings.TenantID);
	clsRecord.MakeParam("strCompanyID", szCompanyID);
	clsRecord.MakeParam("strGroupID", szGroupID);
	clsTable.AppendXMLParam(&clsRecord);
	clsData.AppendXMLParam(&clsTable);
	CXMLParams::AppendXMLConnection(&clsData, CXMLParams::CN_PRIMARY, "CONNECTION");
	CXMLParams::AppendCompanyInfo(&clsData);
	CXML clsXML;

	return clsXML.SendXML(clsCommand.GetXML(), clsData.GetXML()).copy();
}

// gets XML from server and loads data into the objects
// this is the function called by outside functions to load the xml
// returns false if valid data was not retrieved from the server
// the bGetGroupData flag determines whether the data is loaded for a group or a user
// if bGetGroupData is true, szUserID will actually contain the groupID
bool CGBLUserRightsInformation::LoadDataFromServer(const CString& szCompanyID, const CString& szUserID, bool bGetGroupData)
{
	CString szXMLString;
	if (bGetGroupData) // pull data for a group
		szXMLString = GetGroupXMLFromServer(szCompanyID, szUserID);
	else
		szXMLString = GetUserXMLFromServer(szCompanyID, szUserID);

	return LoadDataFromServer(szXMLString);
}

// this version takes an XML string that has already been pulled from the server
// this is the function called by outside functions to load the xml
// returns false if valid data was not retrieved from the server
// the bGetGroupData flag determines whether the data is loaded for a group or a user
// if bGetGroupData is true, szUserID will actually contain the groupID
bool CGBLUserRightsInformation::LoadDataFromServer(const CString& szXMLString)
{
	m_clsUserRightsProcessor.Notifier->Process();
	// have to load it to a data set in order to convert it back to a regular xml string
	CDataSet dataSet;
	dataSet.LoadFromXML(szXMLString);
	if (!dataSet.IsEmpty())
	{
		CDataTable tableDataTable;
		dataSet.GetTable("Table", &tableDataTable);
		if (!tableDataTable.IsEmpty())
		{
			CDataRow dataRow;
			tableDataTable.GetRow(0, &dataRow);
			CDataCell txtSecuritySettingsdataCell;
			dataRow.GetDataCell("txtSecuritySettings", &txtSecuritySettingsdataCell, false);
			if (!txtSecuritySettingsdataCell.IsEmpty())
			{
				CString szXMLUserRights = tableDataTable.XMLStr(0, "txtSecuritySettings", false); // this is the XML String that was passed in to the server originally
				// decrypting data
				if (!szXMLUserRights.IsEmpty())
				{
					m_szDecryptedSecurityXML = CGBLMath::DecryptData(szXMLUserRights);

					// begbert 11-12-2010 : Notify the Navigator that the security settings may have been changed.
					CGBLForm::SendMessageToMainFrame(WM_NOTIFY_NAVIGATOR_INTERFACE, eSecurityXMLChanged);

					CXMLParams rootXMLParams(true);
					rootXMLParams.SetXML(m_szDecryptedSecurityXML);
					m_pUserRightsRootNode->LoadDataFromXML(&rootXMLParams);

					return true;
				}
			}
		}
	}
	return false;
}

// takes the encrypted txtSecuritySettings xml string from the server and builds the tree
bool CGBLUserRightsInformation::LoadDataFromXML(const CString& szTxtSecuritySettingsXMLString)
{
	if (!szTxtSecuritySettingsXMLString.IsEmpty())
	{
		CString szReturn;
		szReturn = CGBLMath::DecryptData(szTxtSecuritySettingsXMLString);
		CXMLParams rootXMLParams(true);
		rootXMLParams.SetXML(szReturn);
		m_pUserRightsRootNode->LoadDataFromXML(&rootXMLParams);
		return true;
	}
	return true;
}

// this will set the default values for the supervisor user
void CGBLUserRightsInformation::SetDefaultValuesForUserSUP()
{
	m_pUserRightsRootNode->SetAllFlagsToTrue();
	m_stuGeneralLedgerNode.Journal.bAllowOutOfBalancePosting = false;
}

// 1-26149 BK 12/12/06 clear entire struct when different user is logged in
void CGBLUserRightsInformation::ClearAll()
{
	// 1-35038 BK 11/11/10 create new user rights node list when company is refreshed or user is changed
	CreateObjectList();
	m_pUserRightsRootNode->SetAllFlagsToFalse();
}

bool CGBLUserRightsInformation::CurrentModuleCanImport(void) const
{
	return ModuleCanImport(g_pGBLSystemInformationCMSDll->ApplicationSettings.ModuleID);
}

bool CGBLUserRightsInformation::CurrentModuleCanExport(void) const
{
	return ModuleCanExport(g_pGBLSystemInformationCMSDll->ApplicationSettings.ModuleID);
}

bool CGBLUserRightsInformation::ModuleCanImport(UINT uiModuleID) const
{
	switch (uiModuleID)
	{
	case IDS_AP_MODULE:
		return m_stuAccountsPayableNode.Vendor.ImportExport.bImport;
		break;
	case IDS_AR_MODULE:
		return m_stuAccountsReceivableNode.Options.FileMaintenance.ImportExport.bImport;
		break;
	case IDS_GL_MODULE:
		return m_stuGeneralLedgerNode.Options.FileMaintenance.ImportExport.bImport;
		break;
	case IDS_IN_MODULE:
		return m_stuInventoryNode.Options.FileMaintenance.ImportExport.bImport;
		break;
	default:
		return false;
		break;
	};
}

bool CGBLUserRightsInformation::ModuleCanExport(UINT uiModuleID) const
{
	switch (uiModuleID)
	{
	case IDS_AP_MODULE:
		return m_stuAccountsPayableNode.Vendor.ImportExport.bExport;
		break;
	case IDS_AR_MODULE:
		return m_stuAccountsReceivableNode.Options.FileMaintenance.ImportExport.bExport;
		break;
	case IDS_GL_MODULE:
		return m_stuGeneralLedgerNode.Options.FileMaintenance.ImportExport.bExport;
		break;
	case IDS_IN_MODULE:
		return m_stuInventoryNode.Options.FileMaintenance.ImportExport.bExport;
		break;
	default:
		return false;
		break;
	};
}
// 1-35038 BK 11/11/10 removed not necessary and didn't work (was supposed to set node for electronic constants menu item when AP or PR modules are installed )
void CGBLUserRightsInformation::AddElectronicConstants()
{
	//Defect ID 1-24335, PGP(05/19/2006)
	// 1-19580 BK 6/7/05 only show if AP or PR are installed
	//if(m_pCompanyUserRights && g_pGBLSystemInformationCMSDll != NULL)
	//{
	//	if (g_pGBLSystemInformationCMSDll->ModulesInstalled.AccountsPayable.IsUsable || g_pGBLSystemInformationCMSDll->ModulesInstalled.Payroll.IsUsable)
	//	{
	//		if(AfxIsValidAddress(m_pCompanyUserRights, sizeof(CUserRightsBaseNode::userRightsNodeList), TRUE))
	//			m_pCompanyUserRights->AddTail(g_pGBLSystemInformationCMSDll->UserRight.CreateNode("MagneticMediaConstants", IDS_CTRL_MAGNETIC_MEDIA_CONST, &g_pGBLSystemInformationCMSDll->UserRight.m_stuControllerNode.Company.bMagneticMediaConstants)); // Magnetic Media Constants
	//	}
	//}
	//m_pCompanyUserRights = NULL ;
}

//IMPLEMENT_SERIAL(CGBLUserRightsInformation, CObject, 1)
