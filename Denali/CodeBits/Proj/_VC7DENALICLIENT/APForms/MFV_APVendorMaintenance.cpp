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
#include "StdAfx.h"
#include ".\resource.h"
#include ".\mfv_apvendormaintenance.h"

IMPLEMENT_DYNCREATE(CMFV_APVendorMaintenance, CMaintFormView)
CMFV_APVendorMaintenance::CMFV_APVendorMaintenance(void)
	: CMaintFormView(CMFV_APVendorMaintenance::IDD)
	, m_clsPropSheet(_T(""))
	// begbert 04-15-2009 1-29227 : restored this functionality, but only if the module's CMainFrame toggles it on.
	, m_bolViewOnly(false)	// 1-24757 BK 6/29/06 removed per design change
{
	if ((si->ModuleSettings.APSettings.GLIntegration > 9 &&
		si->ModuleSettings.APSettings.GLIntegration < 100) ||
		si->ModuleSettings.APSettings.GLIntegration == 0)
	{
		m_bGLIntegrated = false;
	}
	else
	{
		m_bGLIntegrated = true;
	}

	//m_bDefaultVendor = false ;

	m_szSubForm = _T("1D78A191-9BAE-4301-9D78-C9786EAFC757");
}

CMFV_APVendorMaintenance::~CMFV_APVendorMaintenance(void)
{
	m_pXMLParams = NULL;
}

void CMFV_APVendorMaintenance::DoDataExchange(CDataExchange* pDX)
{
	CMaintFormView::DoDataExchange(pDX);
	//HEADER INFO
	DDX_Control(pDX, IDC_APF_ICO_TOP, m_icoTop);
	DDX_Control(pDX, IDC_APF_SEP_TOP, m_sepTop);

	//Combo Controls
	DDX_Control(pDX, IDC_APF_CBO_DISCOUNT, m_cboDiscount);
	DDX_Text(pDX, IDC_APF_CBO_DISCOUNT, m_szDiscount);
	DDX_Control(pDX, IDC_APF_CBO_INVOICE, m_cboInvoice);
	DDX_Text(pDX, IDC_APF_CBO_INVOICE, m_szInvoice);
	DDX_Control(pDX, IDC_APF_CBO_STATUS, m_cboStatus);
	DDX_Text(pDX, IDC_APF_CBO_STATUS, m_szStatus);

	//CMSDate Controls
	DDX_Control(pDX, IDC_APF_DTE_CREATED, m_dteCreated);
	DDX_CMSDate(pDX, IDC_APF_DTE_CREATED, m_dtCreated);

	//CheckBox Controls
	DDX_Control(pDX, IDC_APF_CHK_1099, m_chk1099);
	DDX_Check(pDX, IDC_APF_CHK_1099, m_n1099);

	//AutofillEdit Controls
	DDX_Control(pDX, IDC_APF_TXT_NUMBER, m_txtNumber);
	DDX_Text(pDX, IDC_APF_TXT_NUMBER, m_szNumber);
	DDX_Control(pDX, IDC_APF_TXT_CONAME, m_txtName);
	DDX_Text(pDX, IDC_APF_TXT_CONAME, m_szName);
	DDX_CMSControl(pDX, IDC_TXT_COSTCENTER, m_txtCostCenter, IDC_LBL_COSTCENTER);
	DDX_Text(pDX, IDC_TXT_COSTCENTER, m_szCostCenter);

	//Edit Controls
	DDX_Control(pDX, IDC_APF_TXT_CREDIT_LIMIT, m_txtCreditLimit);
	DDX_Text(pDX, IDC_APF_TXT_CREDIT_LIMIT, m_szCreditLimit);
	DDX_Control(pDX, IDC_APF_TXT_DISCOUNT_AMOUNT, m_txtDiscountPercent);
	DDX_Text(pDX, IDC_APF_TXT_DISCOUNT_AMOUNT, m_szDiscountPercent);
	DDX_Control(pDX, IDC_APF_TXT_DIS_DAYS, m_txtDiscountDays);
	DDX_Text(pDX, IDC_APF_TXT_DIS_DAYS, m_szDiscountDays);
	DDX_Control(pDX, IDC_APF_TXT_INV_DAYS, m_txtInvoiceDays);
	DDX_Text(pDX, IDC_APF_TXT_INV_DAYS, m_szInvoiceDays);
	DDX_Control(pDX, IDC_APF_TXT_APCODE, m_txtAPCode);
	DDX_Text(pDX, IDC_APF_TXT_APCODE, m_szAPCode);
	DDX_Control(pDX, IDC_APF_TXT_PRODTYPE, m_txtProdType);
	DDX_Text(pDX, IDC_APF_TXT_PRODTYPE, m_szProdType);
}

BEGIN_MESSAGE_MAP(CMFV_APVendorMaintenance, CMaintFormView)
	ON_CBN_SELCHANGE(IDC_APF_CBO_INVOICE, &CMFV_APVendorMaintenance::LoadCboDiscount)
	ON_BN_CLICKED(IDC_APF_CHK_1099, &CMFV_APVendorMaintenance::OnBnClickedApfChk1099)
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_USER_KILLFOCUS, &CMFV_APVendorMaintenance::OnEnKillfocusCtlGlAccount)		// GL Account control lost focus
	ON_REGISTERED_MESSAGE(WM_LOOKUP_SELECTION_MADE, &CMFV_APVendorMaintenance::OnEnSelectionMadeCtlAPCode)		// AP code selected in lookup : begbert 07-25-2006
	ON_REGISTERED_MESSAGE(WM_CMSLISTCTRL_CUSTOMIZE_LOOK, &CMFV_APVendorMaintenance::OnLookupCustomize)
	ON_REGISTERED_MESSAGE(WM_CMSLISTCTRL_SELECTIONS, &CMFV_APVendorMaintenance::OnLookupSelection)


	//ON_EN_SETFOCUS(IDC_APF_TXT_CREDIT_LIMIT, OnEnSetfocusApfTxtCreditLimit)
END_MESSAGE_MAP()

LRESULT CMFV_APVendorMaintenance::OnEnKillfocusCtlGlAccount(WPARAM wParam, LPARAM lParam)
{
	//// if GL integration is on and if account is valid fill static control with account description
	if (m_bGLIntegrated)
	{
		if (m_txtGLExpense.IsAccountValid())
			GetDlgItem(IDC_APF_LBL_GL_EXPENSE_DESCRIPTION)->SetWindowText(m_txtGLExpense.GetDescription());
		else
			// user entered an invalid account fill static control with Not Defined
			GetDlgItem(IDC_APF_LBL_GL_EXPENSE_DESCRIPTION)->SetWindowText(CGBLResources::GetResourceString(IDS_NOT_DEFINED));
	}
	return 0;
}

void CMFV_APVendorMaintenance::OnInitialUpdate()
{
	CMaintFormView::OnInitialUpdate();

	CreatePropertySheet();//Create embedded property sheet.

	LoadResources();//Load language specific strings.

	FillControlMap();//Set XML control map

	InitAutofillFields();//Init autofill fields

	SetControlProperties();//Sets properties of form controls

	m_bIsAutofill = false;

	//GetDefaultTemplate();

	m_bFirstTimeFocus = true;
	m_txtNumber.SetDataFetchRecord(false);

	CreateControlButtons();//Create control buttons

	ResizeParentToFit(FALSE);

	m_clsPropSheet.SetActivePage(CPSH_APVendorMaintenance::PAGE_ADDRESS);

	// begbert 06-15-2006 1-24616, 1-24617 : Something was interfering with the focus; this sidesteps the problem.
	// begbert 06-21-2006 1-24658 : Improved above solution; simulate click rather than set focus.
//	m_txtNumber.SetFocus() ;
//	m_txtNumber.PostMessageW(WM_SETFOCUS);
	m_txtNumber.PostMessageW(WM_LBUTTONDOWN);
	m_txtNumber.PostMessageW(WM_LBUTTONUP);

	/*m_cboInvoice.SetCurSel(0) ;
	m_cboDiscount.SetCurSel(0) ;
	m_cboStatus.SetCurSel(0) ;*/

	// 1-31878 BK 1/28-09 override user settings with buyer settings when in PO Entry
	if (!CMS::GetSI()->ApplicationSettings.IsPOE())
	{
		// PBI 17468 BK 9/12/14 Change user rights to separate Add/Edit and View Only settings
		if (si->UserRight.m_stuAccountsPayableNode.Vendor.VendorMaster.bViewVendor && !si->UserRight.m_stuAccountsPayableNode.Vendor.VendorMaster.bAddEditVendor)
			ReadOnlyUserRight();
	}
	//1-24659 AES need to set to system date on startup.
	m_dteCreated.SetDate(si->ApplicationSettings.SystemDate);
	//Defect ID 1-24462, PGP(05/30/2006)
	m_dteCreated.GetDate(m_dtCreated);

	// InTimeTec: 05/23/2013 : HS : Attach Files to Master Records
	m_clsPropSheet.m_clsAttachments.m_clsForm = this;
	// InTimeTec: End
}

//Create the embedded property sheet.
void CMFV_APVendorMaintenance::CreatePropertySheet()
{
	//Set member pointers
	m_clsPropSheet.m_pParentView = this;
	m_clsPropSheet.m_pXML = &m_clsXML;
	m_clsPropSheet.m_pXMLParams = &m_clsXMLParams;
	m_clsPropSheet.m_pLookup = m_pLookup;
	m_clsPropSheet.m_pnMode = (INT32*)&m_nMode;

	//Create modeless property sheet using global functions.
	CGBLForm::EkCreateEmbeddedPropertySheet(this, &m_clsPropSheet);
	//Position property sheet according to form placeholder
	CGBLForm::EkPositionEmbeddedPropertySheet(this, &m_clsPropSheet, IDC_APF_PPSHEET_HOLDER);

	//If no udfs on page, remove it!
	if (m_clsPropSheet.m_clsUserDefinedPage.IsEmptyPage())
	{
		m_clsPropSheet.RemovePage(&m_clsPropSheet.m_clsUserDefinedPage);
	}

}

//Creates control buttons, sets properties
void CMFV_APVendorMaintenance::CreateControlButtons()
{
	try
	{
		if (m_dlgCMSControlButtons.BindControl(GetDlgItem(IDC_APF_BUTTON_HOLDER), true))
		{
			m_dlgCMSControlButtons.m_szForm = "AP_VENDORMASTER";
			m_dlgCMSControlButtons.m_szModule = "AP";
			m_dlgCMSControlButtons.m_szSQLField = "strVendor";
			m_dlgCMSControlButtons.m_nSQLFieldLength = 50; //Set the length of the edit controls on the Print Codes form.
			m_dlgCMSControlButtons.m_szXMLTable = "MASTER";
			m_dlgCMSControlButtons.m_pXML = &m_clsXML;
			// InTimeTec: 05/21/2013 : HS : Attach Files to Master Records
			m_dlgCMSControlButtons.m_pXML->m_bSaveSpecial = true;
			// InTimeTec: End
			m_dlgCMSControlButtons.pstrCommandValues = &strCommandValues;

			m_dlgCMSControlButtons.m_szFormTitle = CGBLResources::GetResourceString(IDS_LBL_VENDOR);
			m_dlgCMSControlButtons.m_szReportFileID = RPT_AP_VENDORMASTERMAINT;

			m_pXMLParams = &m_clsXMLParams;
			m_pXMLKeyInfo = new CXMLParams(true);
			m_pXMLSearchFor = new CXMLParams(true);

			m_dlgCMSControlButtons.m_bAddOnFly = false;
			m_dlgCMSControlButtons.m_bViewForm = false;
			m_pXMLKeyInfo->SetRoot(_T("KEY_INFORMATION"));
			m_dlgCMSControlButtons.m_uinLookupID = LKP_AP_VENDOR;
			m_dlgCMSControlButtons.m_uinModuleID = IDS_AP_MODULE;
			m_dlgCMSControlButtons.m_pclsParams = m_pXMLParams;

			SetLabels();
		}
	}
	catch (_com_error& e)
	{
		CGBLForm::HandleError(e);
	}
}

void CMFV_APVendorMaintenance::SetLabels()
{
	SetCompanyArray(FALSE);
}

void CMFV_APVendorMaintenance::LoadResources()
{
	CGBLForm::SetFormTitle(this, CGBLResources::GetResourceString(IDS_AP_APVENDORS));
	m_icoTop.SetIcon(si->Drawing.Icons.APVendor());//set top icon
	SetResourceText(IDC_APF_TOPHEADER, IDS_AP_TOPHEADER_APVENDOR);
	SetResourceText(IDC_APF_LBL_NUMBER, IDS_NUMBER);
	// 1-25504 BK 12/29/08 removed hot key as per standards
	//SetResourceText(IDC_APF_LBL_APCODE, IDS_AP_APCODE) ;
	SetResourceText(IDC_APF_LBL_APCODE, IDS_APCODE);

	SetResourceText(IDC_APF_LBL_CONAME, IDS_AP_CONAME);
	SetResourceText(IDC_APF_LBL_DATE_CREATED, IDS_CREATED);
	SetResourceText(IDC_APF_LBL_STATUS, IDS_AP_APSTATUS);
	SetResourceText(IDC_APF_LBL_PRODTYPE, IDS_AP_PROD_TYPE);
	SetResourceText(IDC_APF_LBL_CREDIT_LIMIT, IDS_CREDIT_LIMIT);
	SetResourceText(IDC_APF_LBL_GL_EXPENSE, IDS_AP_GL_EXPENSE);
	SetResourceText(IDC_APF_LBL_GL_EXPENSE_DESCRIPTION, IDS_EMPTY);
	SetResourceText(IDC_LBL_COSTCENTER, IDS_COST_CENTER);
	SetResourceText(IDC_APF_LBL_CALCULATION_METHOD, IDS_AP_DUE_DATE_CAL_METHOD);
	SetResourceText(IDC_APF_LBL_DAYS, IDS_DAYS);
	SetResourceText(IDC_APF_CHK_1099, IDS_AP_1099_APPLICABLE);
	SetResourceText(IDC_APF_LBL_INVOICE, IDS_INVOICE);
	SetResourceText(IDC_APF_LBL_DISCOUNT, IDS_DISCOUNT);
	SetResourceText(IDC_APF_LBL_DISCOUNT_AMOUNT, IDS_DISCOUNT);
}

void CMFV_APVendorMaintenance::FillControlMap()
{
	m_clsXML.m_bMultipleDetails = true;	// EK - 06/07/04 - Added to load multiple detail tables

	m_clsXML.MySetElement(_T("0"), _T("bytDiscountType"), &m_cboDiscount, NULL, COMBOITEMDATATYPE);
	m_clsXML.MySetElement(_T("0"), _T("bytDueDateControl"), &m_cboInvoice, NULL, COMBOITEMDATATYPE);
	m_clsXML.MySetElement(_T("0"), _T("bytStatus"), &m_cboStatus, NULL, COMBOBOX_INDEX);
	m_clsXML.MySetElement(_T("0"), _T("curCreditLimit"), &m_txtCreditLimit, NULL, OXCURRENCYTYPE);
	m_clsXML.MySetElement(_T("0"), _T("decDiscountPercent"), &m_txtDiscountPercent, NULL, OX_NUMERIC_TYPE);
	m_clsXML.MySetElement(_T("0"), _T("dteDateCreated"), &m_dteCreated, NULL, CMSDATETYPE);
	m_clsXML.MySetElement(_T("0"), _T("bol1099ApplicableFlag"), &m_chk1099, NULL, CHECKTYPE);
	m_clsXML.MySetElement(_T("0"), _T("intDiscountDays"), &m_txtDiscountDays, NULL, OX_NUMERIC_TYPE);
	m_clsXML.MySetElement(_T("0"), _T("intDueDateDays"), &m_txtInvoiceDays, NULL, OX_NUMERIC_TYPE);
	m_clsXML.MySetElement(_T("0"), _T("strAPCode"), &m_txtAPCode, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strDefaultGLExpenseAccount"), &m_txtGLExpense, NULL, ACCOUNTTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strCostCenter"), &m_txtCostCenter, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strProductType"), &m_txtProdType, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strVendor"), &m_txtNumber, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strVendorName"), &m_txtName, NULL, STRINGTYPE);

	m_clsPropSheet.FillControlMap();//trigger fill control map for all the property pages.
}

void CMFV_APVendorMaintenance::InitAutofillFields()
{
	m_nMode = MODE_ADD;
	m_bIsLookup = false;
	m_bIsAutofill = true;

	m_txtNumber.SetWndLabel(GetDlgItem(IDC_APF_LBL_NUMBER));
	m_txtNumber.SetDataLookupID(LKP_AP_VENDOR);
	m_txtNumber.SetDataFormName(_T("AP_VENDORMASTER"));
	m_txtNumber.SetDataXMLTableName(_T("MASTER"));
	m_txtNumber.SetDataFieldName(_T("strVendor"));
	m_txtNumber.m_bAddOnFly = false;
	m_txtNumber.m_bViewForm = false;
	m_txtNumber.SetDataFetchRecord(true);
	InitCommonAutoFillFields(&m_txtNumber);

	m_txtAPCode.SetWndLabel(GetDlgItem(IDC_APF_LBL_APCODE));
	m_txtAPCode.SetDataLookupID(LKP_AP_CODE);
	m_txtAPCode.SetDataFormName(_T("AP_CODE"));
	m_txtAPCode.SetDataFieldName(_T("strAPCode"));
	m_txtAPCode.m_bAddOnFly = true;
	m_txtAPCode.m_bViewForm = true;
	m_txtAPCode.SetDataFetchRecord(false);
	m_txtAPCode.SetChangeModeAfterFetch(false);
	InitCommonAutoFillFields(&m_txtAPCode);

	m_txtCostCenter.SetWndLabel(GetDlgItem(IDC_LBL_COSTCENTER));
	m_txtCostCenter.SetDataModuleID(IDS_GL_MODULE);
	m_txtCostCenter.SetDataModuleName(_T("GL"));
	m_txtCostCenter.SetDataFormName(_T("GL_CostCenters"));
	m_txtCostCenter.SetAutoFillKey(_T("strCostCenter"));
	m_txtCostCenter.SetDataLookupID(LKP_GL_COSTCENTERS);
	m_txtCostCenter.SetDataPtrLookup(&m_pLookup);
}

//Set common members for autofill objects.
void CMFV_APVendorMaintenance::InitCommonAutoFillFields(CCMSAutoFillEdit* pAFEdit)
{
	pAFEdit->SetDataModuleID(IDS_AP_MODULE);
	pAFEdit->SetDataModuleName(_T("AP"));

	pAFEdit->SetFlagAutoFill(&m_bIsAutofill);
	pAFEdit->SetFlagDirty(&m_dlgCMSControlButtons.m_bIsDirty);
	pAFEdit->SetFlagLookup(&m_bIsLookup);
	pAFEdit->SetFlagMode((int*)&m_nMode);

	pAFEdit->SetWndFormView(this);
	pAFEdit->SetWndControlButtons(&m_dlgCMSControlButtons);

	pAFEdit->SetDataPtrXML(&m_clsXML);
	pAFEdit->SetDataPtrXMLParams(&m_clsXMLParams);
	pAFEdit->SetDataPtrLookup(&m_pLookup);
}

//Set Control Properties
void CMFV_APVendorMaintenance::SetControlProperties()
{
	//Set Text Limits
	m_txtNumber.SetLimitText(50);
	m_txtAPCode.SetLimitText(6);
	m_txtName.SetLimitText(50);
	m_txtProdType.SetLimitText(15);
	m_txtCostCenter.SetControlProperties(50L, 50L);
	if (!CMS::GetSI()->ModulesInstalled.CostCenter.IsUsable)
	{
		m_txtCostCenter.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LBL_COSTCENTER)->ShowWindow(SW_HIDE);
	}

	//Set Numeric Limits
	//Defect ID 1-13745, PGP(09/17/2004) -
	//Set credit limit not to allow negative numbers, removed negative formatting.
	m_txtCreditLimit.SetDecimalDigitCount(11);
	m_txtCreditLimit.SetFractionalDigitCount(2);
	m_txtCreditLimit.SetHighlightOnSetFocus(TRUE);
	m_txtCreditLimit.m_bDefaultZeros = true;
	m_txtCreditLimit.m_bNoNegativeNumber = true;

	//Defect ID 1-13745, PGP(09/17/2004) -
	//Set InvoiceDays and DiscountDays not to allow negative numbers.
	m_txtInvoiceDays.SetDecimalDigitCount(2);
	m_txtInvoiceDays.SetFractionalDigitCount(0);
	m_txtInvoiceDays.m_bNoNegativeNumber = true;

	m_txtDiscountDays.SetDecimalDigitCount(2);
	m_txtDiscountDays.SetFractionalDigitCount(0);
	m_txtDiscountDays.m_bNoNegativeNumber = true;

	//Defect ID 1-13745, PGP(09/17/2004) -
	//Set DiscountPercent not to allow negative numbers.
	m_txtDiscountPercent.SetDecimalDigitCount(3);
	m_txtDiscountPercent.SetFractionalDigitCount(4);
	m_txtDiscountPercent.SetMask(_T("#%"));
	m_txtDiscountPercent.m_bNoNegativeNumber = true;

	//Load Cbo Boxes
	m_cboStatus.AddString(CGBLResources::GetResourceString(IDS_AP_ACTIVE));
	m_cboStatus.AddString(CGBLResources::GetResourceString(IDS_INACTIVE));
	m_cboStatus.AddString(CGBLResources::GetResourceString(IDS_AP_TEMPORARY));

	m_cboInvoice.AddString(CGBLResources::GetResourceString(IDS_AP_DAYS_FROM_INVOICE));
	m_cboInvoice.AddString(CGBLResources::GetResourceString(IDS_AP_DAYS_MONTH));
	m_cboInvoice.AddString(CGBLResources::GetResourceString(IDS_AP_AFTER_END_MONTH));
	m_cboInvoice.AddString(CGBLResources::GetResourceString(IDS_AP_BEFORE_END_MONTH));
	m_cboInvoice.SetItemData(0, 0);
	m_cboInvoice.SetItemData(1, 1);
	m_cboInvoice.SetItemData(2, 3);
	m_cboInvoice.SetItemData(3, 2);

	LoadDefaultCboDiscount();

	LoadCboDiscount();

	//Setup Account Ctrl
	m_txtGLExpense.BindControl(GetDlgItem(IDC_APF_TXT_GL_EXPENSE));
	m_txtGLExpense.SetLabelWnd(GetDlgItem(IDC_APF_LBL_GL_EXPENSE));
	m_txtGLExpense.SetHandleSelectionFromLookup(true);
	m_txtGLExpense.SetDescriptionWnd(GetDlgItem(IDC_APF_LBL_GL_EXPENSE_DESCRIPTION));
	m_txtGLExpense.SetFetchDescription(true);
	m_txtGLExpense.AllowLookups(m_bGLIntegrated);
	m_txtGLExpense.AllowAutofill(m_bGLIntegrated);


	m_txtGLExpense.EnableGLIntegration(m_bGLIntegrated);
	m_txtGLExpense.m_bGridMode = false;
	m_txtGLExpense.m_pLookup = NULL;
}

void CMFV_APVendorMaintenance::LoadDefaultCboDiscount()
{
	m_cboDiscount.ResetContent();
	m_cboDiscount.AddString(CGBLResources::GetResourceString(IDS_AP_DAYS_FROM_INVOICE));
	m_cboDiscount.AddString(CGBLResources::GetResourceString(IDS_AP_DAYS_MONTH));
	m_cboDiscount.AddString(CGBLResources::GetResourceString(IDS_AP_AFTER_END_MONTH));
	m_cboDiscount.AddString(CGBLResources::GetResourceString(IDS_AP_BEFORE_END_MONTH));
	m_cboDiscount.SetItemData(0, 0);
	m_cboDiscount.SetItemData(1, 1);
	m_cboDiscount.SetItemData(2, 3);
	m_cboDiscount.SetItemData(3, 2);
}

void CMFV_APVendorMaintenance::ChangeMode(enmFormModes nMode)
{
	double dAmt;
	switch (nMode)
	{
	case MODE_ADD:
		m_clsXML.EnableDisable(ENABLE, _T("0"));
		m_clsXML.ClearControls(_T("ALL"));
		//Get Default Vendor Template entries
		GetDefaultTemplate();
		//m_txtNumber.SetSel(0, -1) ;
		OnBnClickedApfChk1099();
		m_bIsAutofill = true;
		m_dlgCMSControlButtons.m_bIsDirty = false;
		// 1-24784 BK 6/28/06 clear GL Expense Description
		GetDlgItem(IDC_APF_LBL_GL_EXPENSE_DESCRIPTION)->SetWindowTextW(_T(""));
		//1-25049 AES date needs to reflect current date
		m_dteCreated.SetDate(si->ApplicationSettings.SystemDate);
		//Defect ID 1-25043, PGP(07/18/2006)
		m_txtNumber.ForceFocus();
		break;
	case MODE_EDIT:
		m_txtNumber.EnableWindow(FALSE);
		m_dteCreated.EnableWindow(FALSE);
		m_dteCreated.GetDate(m_dtCreated);
		if (m_dtCreated.GetStatus() != COleDateTime::DateTimeStatus::valid)
			m_dteCreated.SetDate(si->ApplicationSettings.SystemDate);
		m_bIsAutofill = true;
		SetButtonControlDirty(false);
		//m_txtName.SetFocus() ;
		//::PostMessage(m_txtName,EM_SETSEL,0,-1) ;

		dAmt = CGBLForm::CurrencyToValue(m_clsPropSheet.m_clsVendorStatisticsPage.m_szTotalDue);
		if (si->ModuleSettings.APSettings.SecureVendor || dAmt != 0)
		{
			m_dlgCMSControlButtons.m_btnEdit.EnableWindow(FALSE);
		}
		if (dAmt != 0)
		{
			m_txtAPCode.EnableWindow(FALSE);
		}
		//Defect ID 1-25043, PGP(07/18/2006)
		PostMessage(WM_NEXTDLGCTL, (WPARAM)m_txtName.GetSafeHwnd(), TRUE);
		break;
	case MODE_VIEW:
		m_txtAPCode.EnableWindow(FALSE);
		m_dteCreated.GetDate(m_dtCreated);
		if (m_dtCreated.GetStatus() != COleDateTime::DateTimeStatus::valid)
			m_dteCreated.SetDate(si->ApplicationSettings.SystemDate);
		UpdateData();
		m_txtGLExpense.IsAccountValid();
		OnBnClickedApfChk1099();
		SetButtonControlDirty(false);
		// 1-24757 BK 6/29/06 removed per design change
		// begbert 04-15-2009 1-29227 : restored this functionality, but only if the module's CMainFrame toggles it on.
		if (m_bolViewOnly)
		{
			m_dlgCMSControlButtons.m_btnEdit.EnableWindow(FALSE);
			m_dlgCMSControlButtons.m_btnClear.EnableWindow(FALSE);
		}
		break;
	}
	m_clsPropSheet.ChangeMode(nMode);

	CMaintFormView::ChangeMode(nMode);
}

void CMFV_APVendorMaintenance::LoadCboDiscount()
{
	int nCurInvSel = m_cboInvoice.GetCurSel();
	//Defect ID 1-14418, PGP(11/11/2004) - Remember
	//Save the current displayed string in discount combo
	//so we can revert to that after fill.
	int nCurDiscSel = m_cboDiscount.GetCurSel();
	CString szDiscountSelText;
	if (nCurDiscSel != CB_ERR)
		m_cboDiscount.GetLBText(nCurDiscSel, szDiscountSelText);
	switch (nCurInvSel)
	{
	case -1:
	case 0:
	{
		m_cboDiscount.ResetContent();
		m_cboDiscount.AddString(CGBLResources::GetResourceString(IDS_AP_DAYS_FROM_INVOICE));
		m_cboDiscount.AddString(CGBLResources::GetResourceString(IDS_AP_DAYS_MONTH));
		m_cboDiscount.SetItemData(0, 0);
		m_cboDiscount.SetItemData(1, 1);
		break;
	}
	case 1:
	{
		m_cboDiscount.ResetContent();
		LoadDefaultCboDiscount();
		break;
	}
	case 2:
	{
		m_cboDiscount.ResetContent();
		m_cboDiscount.AddString(CGBLResources::GetResourceString(IDS_AP_DAYS_FROM_INVOICE));
		m_cboDiscount.AddString(CGBLResources::GetResourceString(IDS_AP_AFTER_END_MONTH));
		m_cboDiscount.AddString(CGBLResources::GetResourceString(IDS_AP_BEFORE_END_MONTH));
		m_cboDiscount.SetItemData(0, 0);
		m_cboDiscount.SetItemData(1, 3);
		m_cboDiscount.SetItemData(2, 2);
		nCurInvSel = 1;
		break;
	}
	case 3:
	{
		m_cboDiscount.ResetContent();
		m_cboDiscount.AddString(CGBLResources::GetResourceString(IDS_AP_DAYS_FROM_INVOICE));
		m_cboDiscount.AddString(CGBLResources::GetResourceString(IDS_AP_BEFORE_END_MONTH));
		m_cboDiscount.SetItemData(0, 0);
		m_cboDiscount.SetItemData(1, 2);
		nCurInvSel = 1;
		break;
	}
	}
	if (m_cboDiscount.FindStringExact(-1, szDiscountSelText) != CB_ERR)
		m_cboDiscount.SelectString(-1, szDiscountSelText);
	else
		m_cboDiscount.SetCurSel(0);
}

BOOL CMFV_APVendorMaintenance::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	try
	{
		NMHDR* pMessage = (NMHDR*)lParam;
		CString szTextBoxText = _T("");

		switch (pMessage->code)
		{
		case WM_SET_ARRAY:
			SetArrayValues(0);
			break;
		case WM_BTN_SAVE:
			SetArrayValues(RECORD_UPDATE);
			break;
		case WM_PRINT_DONE: // set print values
		{
			CMaintFormView::OnNotify(wParam, lParam, pResult);
			break;
		}
		case WM_DIRTY:
			m_dlgCMSControlButtons.m_bIsDirty = true;
			break;
		}
	}
	catch (_com_error& e)
	{
		CGBLForm::HandleError(e);
	}
	return CMaintFormView::OnNotify(wParam, lParam, pResult);
}

void CMFV_APVendorMaintenance::SetArrayValues(int nType)
{
	UpdateData();

	CString szText;

	strCommandValues.RemoveAll();
	strCommandValues.Add(_T("strVendor"));
	strCommandValues.Add(m_szNumber);
	// InTimeTec: 05/24/2013 : HS : Attach Files to Master Records
	strCommandValues.Add(_T("strMasterKey"));
	strCommandValues.Add(m_szNumber);
	// InTimeTec: End

	if ((nType == RECORD_INSERT) || (nType == RECORD_UPDATE))
	{
		strCommandValues.Add(_T("strAPCode"));
		strCommandValues.Add(m_szAPCode);
		strCommandValues.Add(_T("strVendorName"));
		strCommandValues.Add(m_szName);
		strCommandValues.Add(_T("dteDateCreated"));
		strCommandValues.Add(m_dtCreated.Format(LOCALE_NOUSEROVERRIDE | VAR_DATEVALUEONLY));

		strCommandValues.Add(_T("bytStatus"));

		if (m_cboStatus.GetCurSel() == -1)
			m_szStatus.Format(_T("%d"), 0);
		else
			m_szStatus.Format(_T("%d"), m_cboStatus.GetCurSel());

		strCommandValues.Add(m_szStatus);

		strCommandValues.Add(_T("strProductType"));
		strCommandValues.Add(m_szProdType);
		strCommandValues.Add(_T("curCreditLimit"));
		strCommandValues.Add(m_szCreditLimit);
		strCommandValues.Add(_T("strDefaultGLExpenseAccount"));
		strCommandValues.Add(m_txtGLExpense.GetAccount());
		strCommandValues.Add(_T("strCostCenter"));
		strCommandValues.Add(m_szCostCenter);
		strCommandValues.Add(_T("bytDueDateControl"));
		if (m_cboInvoice.GetCurSel() == -1)
			m_szInvoice.Format(_T("%d"), 0);
		else
			m_szInvoice.Format(_T("%d"), m_cboInvoice.GetItemData(m_cboInvoice.GetCurSel()));

		strCommandValues.Add(m_szInvoice);

		strCommandValues.Add(_T("intDueDateDays"));
		strCommandValues.Add(m_szInvoiceDays);
		strCommandValues.Add(_T("bol1099ApplicableFlag"));
		strCommandValues.Add(m_n1099 == BST_CHECKED ? _T("1") : _T("0"));

		strCommandValues.Add(_T("bytDiscountType"));

		if (m_cboDiscount.GetCurSel() == -1)
			m_szDiscount.Format(_T("%d"), 0);
		else
			m_szDiscount.Format(_T("%d"), m_cboDiscount.GetItemData(m_cboDiscount.GetCurSel()));

		strCommandValues.Add(m_szDiscount);

		strCommandValues.Add(_T("intDiscountDays"));
		strCommandValues.Add(m_szDiscountDays);
		strCommandValues.Add(_T("decDiscountPercent"));
		m_szDiscountPercent.Remove(_T('%'));
		strCommandValues.Add(m_szDiscountPercent);
	}
	m_clsPropSheet.SetArrayValues(nType, strCommandValues);
}

// *********************************************************
// * Function   	LookupGetRecord
// * Purpose    	Call to record when view button on lookup is selected
// * Parameters 	none
// * Returns    	none
// *********************************************************
bool CMFV_APVendorMaintenance::LookupGetRecord(CLookup *pLookup)
{
	CCMSWaitCursor c;
	CMaintFormView::LookupGetRecord(pLookup);
	//Set return to strCustomer from xml data
	m_pFlyLookup = pLookup;
	m_szNumber = (m_clsDataRow.XMLStr(_T("strVendor")));
	UpdateData(false);
	//Don't need this anymore!
	//CGBLForm::DoEvents();
	m_clsDataRow.Dispose();
	c.Restore();

	// begbert 04-15-2009 1-29227 : restored this functionality, but only if the module's CMainFrame toggles it on.
	m_bolViewOnly = (AfxGetMainWnd()->SendMessage(WM_MAINTVIEWONLY, 0, IDD) ? true : false);
	return true;
}

// *********************************************************
// * Function   	LookupSendKeyValue
// * Purpose    	//AES 1-4079 Call to record when view button on lookup is selected
// * Parameters 	none
// * Returns    	none
// *********************************************************
bool CMFV_APVendorMaintenance::LookupSendKeyValue(CLookup *pLookup)
{
	// BK 10/6/04 changed code so Add-On-Fly will select the new code that was just added
	m_pFlyLookup->m_clsDetailGrid.RefreshLookupForAddOnFly(m_szNumber);
	//m_pFlyLookup->m_clsDetailGrid.m_txtSearch.SetWindowText((m_szNumber));
	CMaintFormView::LookupSendKeyValue(m_pFlyLookup);

	return true;
}

void CMFV_APVendorMaintenance::OnBnClickedApfChk1099()
{
	m_clsPropSheet.EnablePage(m_chk1099.GetCheck() == BST_CHECKED, m_clsPropSheet.PAGE_1099);
}

bool CMFV_APVendorMaintenance::Validate()
{
	UpdateData();

	UINT unErrorID = 0;
	UINT unTitleID = 0;
	HWND hWndFocus = NULL;

	bool bFail = false;
	//Must have Vendor Number
	if (m_szNumber == "")
	{
		unErrorID = IDS_AP_ERR_MSG_VENDOR_NUM_BLANK;
		unTitleID = IDS_AP_ERR_CAP_VENDOR_NUM_BLANK;
		hWndFocus = m_txtNumber.GetSafeHwnd();
		bFail++;
	}
	//APCode exists?
	else if (!m_txtAPCode.IsDataValid(m_szAPCode))
	{
		unErrorID = IDS_AP_ERR_MSG_APCODE_INVALID;
		unTitleID = IDS_AP_ERR_CAP_APCODE_INVALID;
		hWndFocus = m_txtAPCode.GetSafeHwnd();
		bFail++;
	}
	//Discount less than 100?
	else if (_tstof(m_szDiscountPercent) > 100)
	{
		unErrorID = IDS_AP_ERR_MSG_INVALID_DISCOUNT;
		unTitleID = IDS_AP_ERR_MSG_INVALID_DISCOUNT;
		hWndFocus = m_txtDiscountPercent.GetSafeHwnd();
		bFail++;
	}
	//Check Invoice Cbo selection against days entered
	else if (m_szInvoice == CGBLResources::GetResourceString(IDS_AP_DAYS_MONTH) && _tstoi(m_szInvoiceDays) > 31)
	{
		unErrorID = IDS_AP_ERR_MSG_INVALID_INOVOICEDAY;
		unTitleID = IDS_ERR_CAP_VALIDATION_ERROR;
		hWndFocus = m_txtInvoiceDays.GetSafeHwnd();
		bFail++;
	}
	//Check Discount Cbo selection against days entered
	else if (m_szDiscount == CGBLResources::GetResourceString(IDS_AP_DAYS_MONTH) && _tstoi(m_szDiscountDays) > 31)
	{
		unErrorID = IDS_AP_ERR_MSG_INVALID_DISCOUNTDAY;
		unTitleID = IDS_ERR_CAP_VALIDATION_ERROR;
		hWndFocus = m_txtDiscountDays.GetSafeHwnd();
		bFail++;
	}
	//Check if Discount and Invoice cbo selections Day BEFORE of month that Discount days is greater than Invoice Days
	else if (m_szDiscount == CGBLResources::GetResourceString(IDS_AP_BEFORE_END_MONTH) && m_szInvoice == CGBLResources::GetResourceString(IDS_AP_BEFORE_END_MONTH) && (_tstoi(m_szDiscountDays) < _tstoi(m_szInvoiceDays)))
	{
		unErrorID = IDS_AP_MSG_INVALID_DAYSBEFORE_MONTH;
		unTitleID = IDS_ERR_CAP_VALIDATION_ERROR;
		hWndFocus = m_txtDiscountDays.GetSafeHwnd();
		bFail++;
	}
	//Check if Discount and Invoice cbo selections Day AFTER of month that Discount days is less than Invoice Days
	else if (m_szDiscount == CGBLResources::GetResourceString(IDS_AP_AFTER_END_MONTH) && m_szInvoice == CGBLResources::GetResourceString(IDS_AP_AFTER_END_MONTH) && (_tstoi(m_szDiscountDays) > _tstoi(m_szInvoiceDays)))
	{
		unErrorID = IDS_AP_MSG_INVALID_DAYSAFTER_MONTH;
		unTitleID = IDS_ERR_CAP_VALIDATION_ERROR;
		hWndFocus = m_txtDiscountDays.GetSafeHwnd();
		bFail++;
	}

	//Check if Discount and Invoice cbo selections Day BEFORE of month that Discount days is greater 31
	//Defect ID 1-3936, PGP(10/01/2004)
	else if (m_szDiscount == CGBLResources::GetResourceString(IDS_AP_BEFORE_END_MONTH) && m_szInvoice == CGBLResources::GetResourceString(IDS_AP_BEFORE_END_MONTH) && (_tstoi(m_szDiscountDays) > 31))
	{
		unErrorID = IDS_AP_ERR_MSG_INVALID_DISCOUNTDAY;
		unTitleID = IDS_ERR_CAP_VALIDATION_ERROR;
		hWndFocus = m_txtDiscountDays.GetSafeHwnd();
		bFail++;
	}
	//Check if Discount and Invoice cbo selections Day AFTER of month that Discount days is greater than 31
	else if (m_szDiscount == CGBLResources::GetResourceString(IDS_AP_AFTER_END_MONTH) && m_szInvoice == CGBLResources::GetResourceString(IDS_AP_AFTER_END_MONTH) && (_tstoi(m_szDiscountDays) > 31))
	{
		unErrorID = IDS_AP_ERR_MSG_INVALID_DISCOUNTDAY;
		unTitleID = IDS_ERR_CAP_VALIDATION_ERROR;
		hWndFocus = m_txtDiscountDays.GetSafeHwnd();
		bFail++;
	}
	// 1-31803 BK 1/22/08 check GL integration
	// 1-27981 BK 6/20/07 check if GL Expense Account is valid
	else if (CGBLModuleIntegrationHelper::IsGLIntegrated() && !m_txtGLExpense.GetAccount().IsEmpty() && !m_txtGLExpense.IsAccountValid())
		//else if (!m_txtGLExpense.GetAccount().IsEmpty() && !m_txtGLExpense.IsAccountValid())
	{
		unErrorID = IDS_AP_ERR_MSG_ENTERBILLS_GLEXPENSE;
		unTitleID = IDS_AP_ERR_CAP_ENTERBILLS_GLEXPENSE;
		hWndFocus = m_txtGLExpense.GetSafeHwnd();
		bFail++;
	}
	else if (!m_txtCostCenter.GetText().IsEmpty() && !m_txtCostCenter.IsDataValid(m_szCostCenter))
	{
		CGBLForm::CMSMessageBox(this, 0, 0, MB_OK + MB_ICONERROR, L"The Cost Center that you have specified is not a valid Cost Center.", L"Error", false);
		PostMessage(WM_NEXTDLGCTL, (WPARAM)m_txtCostCenter.GetSafeHwnd(), TRUE);
		return false;
	}
	if (bFail)
	{
		CGBLForm::CMSMessageBox(this, unErrorID, unTitleID, MB_ICONERROR | MB_OK, _T(""), _T(""), 0, false);
		if (hWndFocus)
			PostMessage(WM_NEXTDLGCTL, (WPARAM)hWndFocus, TRUE);
		return false;
	}

	return m_clsPropSheet.Validate();
}

void CMFV_APVendorMaintenance::OnSetFocus(CWnd* pOldWnd)
{
	// BK 1/21/09 added m_bFirstTimeFocus to prevent crash when open another window in AP, cancel, then open this window
	if (!m_bFirstTimeFocus)
	{
		// 1-34963 BK 11/2/10 save lookup pointer because CMaintFormView::OnSetFocus() clears these pointers then code below is never reached
		CLookup* pSaveLookupPointer = m_pLookup;
		CLookup* pSaveButtonsLookup = m_dlgCMSControlButtons.m_pLookup;
		// BK 1/21/09 added m_bFirstTimeFocus to prevent crash when open another window in AP, cancel, then open this window
		CMaintFormView::OnSetFocus(pOldWnd);
		m_pLookup = pSaveLookupPointer;
		m_dlgCMSControlButtons.m_pLookup = pSaveButtonsLookup;
	}

	//Need to keep record retrieval from happening
	//until we actually leave the Customer Number Field the first time
	if (m_bFirstTimeFocus)
	{
		m_bFirstTimeFocus = false;
		m_txtNumber.SetDataFetchRecord(true);
	}

	m_bIsAutofill = true;

	if (m_pLookup || m_dlgCMSControlButtons.m_pLookup)
	{
		//sb - following causes the form to erase when we don't select anything and the lookup loses focus
		//CString szValue = m_pXMLParams->GetXML();
		//if(szValue.Find("CANCELLED", 0) < 0){
		//	m_clsXML.ClearControls("ALL");
		//}
		m_txtNumber.FetchRecordFromLookup((m_pLookup) ? &m_pLookup : &m_dlgCMSControlButtons.m_pLookup);
	}
	else if (m_txtGLExpense.m_pLookup)
	{
		m_txtGLExpense.GetValueFromLookup();
		GetDlgItem(IDC_APF_LBL_GL_EXPENSE_DESCRIPTION)->SetWindowText(m_txtGLExpense.GetDescription());
		m_txtGLExpense.m_pLookup = NULL;
		m_dlgCMSControlButtons.ResetAllButtonStyles();
	}

	// DTG, reset pointer to avoid assertion errors.
	m_dlgCMSControlButtons.m_pLookup = NULL;
}

void CMFV_APVendorMaintenance::GetDefaultTemplate()
{
	// 1-13535/1-13536 BK 9/2/04 when used as Add-the-the-Fly from IN cannot use global ModuleID
	//m_clsXML.GetXMLData(RECORD_CURRENT, si->ApplicationSettings.ModuleStringID, _T("AP_DEFAULTVENDOR"), &strCommandValues, _T("0"), _T("MASTER"));
	m_clsXML.GetXMLData(RECORD_CURRENT, _T("AP"), _T("AP_DEFAULTVENDOR"), &strCommandValues, _T("0"), _T("MASTER"));
	CString szTmp = m_clsPropSheet.m_clsVendor1099Page.m_cls1099Grid.QuickGetText(1, 2);
	if (szTmp == "")
	{
		m_clsPropSheet.m_clsVendor1099Page.m_cls1099Grid.ResetGrid(MODE_ADD);
		m_txtDiscountDays.SetWindowText(_T("10"));
		m_txtInvoiceDays.SetWindowText(_T("30"));
		// begbert 06-30-2006 1-24486 : no longer necessary
//		m_clsPropSheet.m_clsVendorAddressPage.m_txtAddrCntr.SetWindowText(_T("1"));
//		m_clsPropSheet.m_clsVendorAddressPage.m_txtPrimaryCntr.SetWindowText(_T("1"));
//		m_clsPropSheet.m_clsVendorAddressPage.m_txtAlternateCntr.SetWindowText(_T("1"));
//		m_clsPropSheet.m_clsVendorAddressPage.m_txtFAXCntr.SetWindowText(_T("1"));
//		m_clsPropSheet.m_clsVendorAddressPage.m_txtPagerCntr.SetWindowText(_T("1"));
//		m_clsPropSheet.m_clsVendorAddressPage.m_txtEmailCntr.SetWindowText(_T("1"));
//		m_clsPropSheet.m_clsVendorRemitToPage.m_txtRemitCntr.SetWindowText(_T("2"));
	}
}

// begbert 05-31-2006 1-34474 : added function
void CMFV_APVendorMaintenance::OnPrint(void)
{
	UpdateData(TRUE);
	m_dlgCMSControlButtons.m_szCodeOnly = m_szNumber;
}

void CMFV_APVendorMaintenance::OnPrintAfter()
{
	CMaintFormView::OnPrintAfter();
	m_pPrintCodes = m_dlgCMSControlButtons.m_pPrintCodes;

	SetCompanyArray();
	SetAccountSegArray();

	//1-24504 - RCG - 6/5/2006 - Get Current Year End from AP Module Preferences
	//MakePramValue(_T("@strCurrentYearEnd"), si->FiscalCalendar.EndDate.Format(VAR_DATEVALUEONLY));
	MakePramValue(_T("@strCurrentYearEnd"), si->ModuleSettings.APSettings.CurrentYearEnd);

	MakePramValue(_T("@szUDF1Label"), si->ModuleSettings.APSettings.UDF[0].Caption);
	MakePramValue(_T("@szUDF2Label"), si->ModuleSettings.APSettings.UDF[1].Caption);
	MakePramValue(_T("@szUDF3Label"), si->ModuleSettings.APSettings.UDF[2].Caption);
	MakePramValue(_T("@szUDF4Label"), si->ModuleSettings.APSettings.UDF[3].Caption);
	MakePramValue(_T("@szUDF5Label"), si->ModuleSettings.APSettings.UDF[4].Caption);
	MakePramValue(_T("@szUDF6Label"), si->ModuleSettings.APSettings.UDF[5].Caption);
	MakePramValue(_T("@szUDF7Label"), si->ModuleSettings.APSettings.UDF[6].Caption);
	MakePramValue(_T("@szUDF8Label"), si->ModuleSettings.APSettings.UDF[7].Caption);
	MakePramValue(_T("@szUDF9Label"), si->ModuleSettings.APSettings.UDF[8].Caption);
	MakePramValue(_T("@szUDF10Label"), si->ModuleSettings.APSettings.UDF[9].Caption);

	MakePramValue(_T("@IDS_REPORT_SUMMARY"), CGBLResources::GetResourceString(IDS_REPORT_SUMMARY));
	MakePramValue(_T("@IDS_LBL_REPORTSELECTIONCRITERIA"), CGBLResources::GetResourceString(IDS_LBL_REPORTSELECTIONCRITERIA));
	MakePramValue(_T("@IDS_PRINT_CURRENT_CODE_ONLY"), CGBLResources::GetResourceString(IDS_PRINT_CURRENT_CODE_ONLY));
	MakePramValue(_T("@IDS_CURRENT_CODE"), CGBLResources::GetResourceString(IDS_CURRENT_CODE));
	MakePramValue(_T("@IDS_STARTING_CODE_C"), CGBLResources::GetResourceString(IDS_STARTING_CODE_C));
	MakePramValue(_T("@IDS_ENDING_CODE"), CGBLResources::GetResourceString(IDS_ENDING_CODE));
	MakePramValue(_T("@IDS_IN_NA"), CGBLResources::GetResourceString(IDS_IN_NA));
	MakePramValue(_T("@IDS_YES"), CGBLResources::GetResourceString(IDS_YES));
	MakePramValue(_T("@IDS_NO"), CGBLResources::GetResourceString(IDS_NO));
	MakePramValue(_T("@IDS_PAGE"), CGBLResources::GetResourceString(IDS_PAGE));
	MakePramValue(_T("@IDS_VENDORID"), CGBLResources::GetResourceString(IDS_VENDORID));
	MakePramValue(_T("@IDS_NAME"), CGBLResources::GetResourceString(IDS_NAME));
	MakePramValue(_T("@IDS_NUMBER"), CGBLResources::GetResourceString(IDS_NUMBER));
	MakePramValue(_T("@IDS_APCODE"), CGBLResources::GetResourceString(IDS_APCODE));
	MakePramValue(_T("@IDS_LBL_STATUS"), CGBLResources::GetResourceString(IDS_LBL_STATUS));
	MakePramValue(_T("@IDS_DUE"), CGBLResources::GetResourceString(IDS_DUE));
	MakePramValue(_T("@IDS_CREATED"), CGBLResources::GetResourceString(IDS_CREATED));
	MakePramValue(_T("@IDS_ACTIVE_VENDOR"), CGBLResources::GetResourceString(IDS_ACTIVE_VENDOR));
	MakePramValue(_T("@IDS_INACTIVE_VENDOR"), CGBLResources::GetResourceString(IDS_INACTIVE_VENDOR));
	MakePramValue(_T("@IDS_TEMPORARY_VENDOR"), CGBLResources::GetResourceString(IDS_TEMPORARY_VENDOR));
	MakePramValue(_T("@IDS_CONTACT"), CGBLResources::GetResourceString(IDS_CONTACT));
	MakePramValue(_T("@IDS_LINE_1"), CGBLResources::GetResourceString(IDS_LINE_1));
	MakePramValue(_T("@IDS_LINE_2"), CGBLResources::GetResourceString(IDS_LINE_2));
	MakePramValue(_T("@IDS_CITY"), CGBLResources::GetResourceString(IDS_CITY));
	MakePramValue(_T("@IDS_STATE"), CGBLResources::GetResourceString(IDS_STATE));
	MakePramValue(_T("@IDS_POSTAL_CODE"), CGBLResources::GetResourceString(IDS_POSTAL_CODE));
	MakePramValue(_T("@IDS_ADDRESS_INFORMATION"), CGBLResources::GetResourceString(IDS_ADDRESS_INFORMATION));
	MakePramValue(_T("@IDS_COUNTRY"), CGBLResources::GetResourceString(IDS_COUNTRY));
	MakePramValue(_T("@IDS_REMIT_TO_INFORMATION"), CGBLResources::GetResourceString(IDS_REMIT_TO_INFORMATION));
	MakePramValue(_T("@IDS_AP_INTERNET"), CGBLResources::GetResourceString(IDS_AP_INTERNET));
	MakePramValue(_T("@IDS_CONTACT_INFORMATION"), CGBLResources::GetResourceString(IDS_CONTACT_INFORMATION));
	MakePramValue(_T("@IDS_EMAIL1"), CGBLResources::GetResourceString(IDS_EMAIL1));
	MakePramValue(_T("@IDS_EXT"), CGBLResources::GetResourceString(IDS_EXT));
	MakePramValue(_T("@IDS_GENERAL_INFORMATION"), CGBLResources::GetResourceString(IDS_GENERAL_INFORMATION));
	MakePramValue(_T("@IDS_DISCOUNT"), CGBLResources::GetResourceString(IDS_DISCOUNT));
	MakePramValue(_T("@IDS_LBL_PRODUCTTYPE"), CGBLResources::GetResourceString(IDS_LBL_PRODUCTTYPE));
	MakePramValue(_T("@IDS_AP_GL_EXPENSE"), CGBLResources::GetResourceString(IDS_AP_GL_EXPENSE));
	MakePramValue(_T("@IDS_CUSTOMER_NUMBER"), CGBLResources::GetResourceString(IDS_CUSTOMER_NUMBER));
	MakePramValue(_T("@IDS_CREDIT_LIMIT"), CGBLResources::GetResourceString(IDS_CREDIT_LIMIT));
	MakePramValue(_T("@IDS_AP_RETURNS"), CGBLResources::GetResourceString(IDS_AP_RETURNS));
	MakePramValue(_T("@IDS_LAST_CHECK"), CGBLResources::GetResourceString(IDS_LAST_CHECK));
	MakePramValue(_T("@IDS_AP_DUE_DATE_CAL_METHOD"), CGBLResources::GetResourceString(IDS_AP_DUE_DATE_CAL_METHOD));
	MakePramValue(_T("@IDS_DAYS"), CGBLResources::GetResourceString(IDS_DAYS));
	MakePramValue(_T("@IDS_DISCOUNTS"), CGBLResources::GetResourceString(IDS_DISCOUNTS));
	MakePramValue(_T("@IDS_AP_LAST"), CGBLResources::GetResourceString(IDS_AP_LAST));
	MakePramValue(_T("@IDS_YTD"), CGBLResources::GetResourceString(IDS_YTD));
	MakePramValue(_T("@IDS_INVOICE"), CGBLResources::GetResourceString(IDS_INVOICE));
	MakePramValue(_T("@IDS_TAKEN"), CGBLResources::GetResourceString(IDS_TAKEN));
	MakePramValue(_T("@IDS_AP_PAYMENTS"), CGBLResources::GetResourceString(IDS_AP_PAYMENTS));
	MakePramValue(_T("@IDS_AP_DAYS_FROM_INVOICE"), CGBLResources::GetResourceString(IDS_AP_DAYS_FROM_INVOICE));
	MakePramValue(_T("@IDS_AP_BEFORE_END_MONTH"), CGBLResources::GetResourceString(IDS_AP_BEFORE_END_MONTH));
	MakePramValue(_T("@IDS_AP_AFTER_END_MONTH"), CGBLResources::GetResourceString(IDS_AP_AFTER_END_MONTH));
	MakePramValue(_T("@IDS_AP_DAYS_MONTH"), CGBLResources::GetResourceString(IDS_AP_DAYS_MONTH));
	MakePramValue(_T("@IDS_ALLOWED"), CGBLResources::GetResourceString(IDS_ALLOWED));
	MakePramValue(_T("@IDS_AP_PURCHASES"), CGBLResources::GetResourceString(IDS_AP_PURCHASES));
	//RCG - 11/27/2020 - PBI 56490 - Renamed strings containing 1099-MISC to 1099
	MakePramValue(_T("@IDS_1099_INFORMATION_FOR_BUSINESS"), CGBLResources::GetResourceString(IDS_1099_INFORMATION_FOR_BUSINESS));
	MakePramValue(_T("@IDS_1099_INFORMATION_FOR_INDIVIDUAL"), CGBLResources::GetResourceString(IDS_1099_INFORMATION_FOR_INDIVIDUAL));
	MakePramValue(_T("@IDS_AP_FEDERAL_ID"), CGBLResources::GetResourceString(IDS_AP_FEDERAL_ID));
	MakePramValue(_T("@IDS_DEFAULT_1099_TYPE"), CGBLResources::GetResourceString(IDS_DEFAULT_1099_TYPE));
	MakePramValue(_T("@IDS_CURRENT_YEAR_END"), CGBLResources::GetResourceString(IDS_CURRENT_YEAR_END));
	MakePramValue(_T("@IDS_AP_RENTS"), CGBLResources::GetResourceString(IDS_AP_RENTS));
	MakePramValue(_T("@IDS_AP_ROYALTIES"), CGBLResources::GetResourceString(IDS_AP_ROYALTIES));
	MakePramValue(_T("@IDS_AP_OTHER_INCOME"), CGBLResources::GetResourceString(IDS_AP_OTHER_INCOME));
	MakePramValue(_T("@IDS_AP_FED_INCOME_TAX_WITHHELD"), CGBLResources::GetResourceString(IDS_AP_FED_INCOME_TAX_WITHHELD));
	MakePramValue(_T("@IDS_AP_FISHING_BOAT_PROCEEDS"), CGBLResources::GetResourceString(IDS_AP_FISHING_BOAT_PROCEEDS));
	MakePramValue(_T("@IDS_AP_MEDICAL_HEALTH_PAYMENTS"), CGBLResources::GetResourceString(IDS_AP_MEDICAL_HEALTH_PAYMENTS));
	MakePramValue(_T("@IDS_AP_NON_EMPLOYEE_COMPENSATION"), CGBLResources::GetResourceString(IDS_AP_NON_EMPLOYEE_COMPENSATION));
	MakePramValue(_T("@IDS_AP_SUBSTITUTE_PAYMENTS"), CGBLResources::GetResourceString(IDS_AP_SUBSTITUTE_PAYMENTS));
	MakePramValue(_T("@IDS_AP_CROP_INSURANCE_PROCEEDS"), CGBLResources::GetResourceString(IDS_AP_CROP_INSURANCE_PROCEEDS));
	MakePramValue(_T("@IDS_AP_EXCESS_GOLDEN_PARACHUTE"), CGBLResources::GetResourceString(IDS_AP_EXCESS_GOLDEN_PARACHUTE));
	MakePramValue(_T("@IDS_AP_GROSS_PROCEEDS_TO_ATTORNEY"), CGBLResources::GetResourceString(IDS_AP_GROSS_PROCEEDS_TO_ATTORNEY));
	MakePramValue(_T("@IDS_AP_SECTION_409A_DEFERRALS"), CGBLResources::GetResourceString(IDS_AP_SECTION_409A_DEFERRALS));
	//RCG - 11/17/2020 - PBI 56395 - Section 409A Income was renamed for 2020
	MakePramValue(_T("@IDS_AP_NONQUALIFIED_DEFERRED_COMPENSATION"), CGBLResources::GetResourceString(IDS_AP_NONQUALIFIED_DEFERRED_COMPENSATION));
	//RCG - 11/17/2021 - PBI 59015 - Added new 1099 Type for 2021
	MakePramValue(_T("@IDS_AP_FISH_PURCHASED_FOR_RESALE"), CGBLResources::GetResourceString(IDS_AP_FISH_PURCHASED_FOR_RESALE));
	MakePramValue(_T("@IDS_AP_NON_1099_PAYMENTS"), CGBLResources::GetResourceString(IDS_AP_NON_1099_PAYMENTS));
	MakePramValue(_T("@IDS_TELEPHONE"), CGBLResources::GetResourceString(IDS_TELEPHONE));
	MakePramValue(_T("@IDS_ALTERNATE"), CGBLResources::GetResourceString(IDS_ALTERNATE));
	MakePramValue(_T("@IDS_AP_FAX"), CGBLResources::GetResourceString(IDS_AP_FAX));
	MakePramValue(_T("@IDS_CELL"), CGBLResources::GetResourceString(IDS_CELL));
	MakePramValue(_T("@IDS_AP_1099_TYPE"), CGBLResources::GetResourceString(IDS_AP_1099_TYPE));
	MakePramValue(_T("@IDS_CURRENT_YEAR1"), CGBLResources::GetResourceString(IDS_CURRENT_YEAR1));
	MakePramValue(_T("@IDS_AP_LAST_YEAR"), CGBLResources::GetResourceString(IDS_AP_LAST_YEAR));
	MakePramValue(_T("@IDS_NOTES_TITLE"), CGBLResources::GetResourceString(IDS_NOTES_TITLE));
	MakePramValue(_T("@IDS_DAY_OF_MONTH"), CGBLResources::GetResourceString(IDS_DAY_OF_MONTH));
	//DD - 07/21/2025 - PBI 43386 - Added Remit To email address
	MakePramValue(_T("@IDS_REMIT_TO_EMAIL"), CGBLResources::GetResourceString(IDS_REMIT_TO_EMAIL));

	//begbert 05-24-2006 1-24342 : changed to remove double 'AP'.
	//SetReportTitle(IDS_AP_MASTERVENDOR_REPORT);
	SetReportTitle(IDS_MASTER_VENDOR_REPORT);
}

void CMFV_APVendorMaintenance::OnEnSetfocusApfTxtCreditLimit()
{
	int nPos = 1;
	//Do not highlight the currency symbol when setting focus
	//to the control.  MLG
	if (m_txtCreditLimit.IsNegativeValue())
	{
		nPos = nPos + 1;
	}
	m_txtCreditLimit.PostMessage(EM_SETSEL, (WPARAM)nPos, (LPARAM)-1);
}

LRESULT CMFV_APVendorMaintenance::OnEnSelectionMadeCtlAPCode(WPARAM wParam, LPARAM lParam)
{	// begbert 07-25-2006 : One presumes that there is supposed to be code somewhere that
	//   automatically moves the focus to the next cell on a successful lookup selection.
	//   In this case, though, the focus wasn't going anywhere, so we move it manually.
	m_txtName.PostMessageW(WM_LBUTTONDOWN);
	m_txtName.PostMessageW(WM_LBUTTONUP);
	return 0;
}

LRESULT CMFV_APVendorMaintenance::OnLookupCustomize(WPARAM wParam, LPARAM lParam)
{
	CCMSDetailGridCustomize * const pMsg = (CCMSDetailGridCustomize *)lParam;
	ASSERT_VALID(pMsg);
	//pMsg->DetailGrid->ListControl.DefaultColumnText[0] = _T("Pay");	//.SetCheckboxes(TRUE);
	//pMsg->DetailGrid->ListControl.DefaultCheckState[0] = false;	//.SetCheckboxes(TRUE);
	//pMsg->DetailGrid->ListControl.DefaultColumnWidth[0] = 5;	//.SetCheckboxes(TRUE);
	return (LRESULT)1;
}

LRESULT CMFV_APVendorMaintenance::OnLookupSelection(WPARAM wParam, LPARAM lParam)
{
	CCMSDataSelectionArray * const pMsg = (CCMSDataSelectionArray *)lParam;
	ASSERT_VALID(pMsg);

	for (UINT nRow = 0; nRow < pMsg->Selections->size(); nRow++)
	{
		CSelectedLine&	line = (*pMsg->Selections)[nRow];
		for (UINT nCol = 0; nCol < line.size(); nCol++)
		{
			CSelectedValue&	col = line[nCol];
			TRACE(_T("Column = %s, Value = %s\n"), col.Tag, col.Value);
		}
	}

	return (LRESULT)1;
}
