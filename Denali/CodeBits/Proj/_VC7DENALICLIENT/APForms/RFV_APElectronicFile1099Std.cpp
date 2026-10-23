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
#include "RFV_APElectronicFile1099Std.h"
#include <regex>		//RS - 09/24/2025 - PBI 64621 - Added IRIS
#include <windows.h>	//RS - 01/15/2026 - PBI 64621 - Added CSV UTF-8

#define SHOW_STATEID false

 // CRFV_APElectronicFile1099
//RS - 11/18/2025 - PBI 65630
const CString CRFV_APElectronicFile1099Std::TAXFILEFIRE(_T("FIRE_"));
//RS - 11/14/2025 - PBI 64621 - Added IRIS
const CString CRFV_APElectronicFile1099Std::TAXFILEMISC(_T("1099-MISC"));
const CString CRFV_APElectronicFile1099Std::TAXFILENEC(_T("1099-NEC"));

IMPLEMENT_DYNCREATE(CRFV_APElectronicFile1099Std, CRFV_APBasicStd)

CRFV_APElectronicFile1099Std::CRFV_APElectronicFile1099Std()
	: CRFV_APBasicStd(CRFV_APElectronicFile1099Std::IDD)
{
	m_szSubForm = _T("9A310BD7-BA15-44d2-AACA-64AC8258E9CD");
	ResetTotals();					//12/10/2007 CJB - 1-28229
}
//------------------------------------------------------------------------------
CRFV_APElectronicFile1099Std::~CRFV_APElectronicFile1099Std()
{
}
//------------------------------------------------------------------------------
void CRFV_APElectronicFile1099Std::DoDataExchange(CDataExchange* pDX)
{
	CReportFormView::DoDataExchange(pDX);

	//Controls
	DDX_CMSControl(pDX, IDC_APF_TXT_PATH, m_txtPath, IDC_APF_LBL_PATH);
	DDX_Text(pDX, IDC_APF_TXT_PATH, m_szPath);
	//RCG - 09/24/2025 - PBI 64621 - Added IRIS
	DDX_CMSControl(pDX, IDC_APF_CBO_EFILE_WITH, m_cboEFileWith, IDC_APF_LBL_EFILE_WITH);
	DDX_Text(pDX, IDC_APF_CBO_EFILE_WITH, m_szEFileWith);
	DDX_CMSControl(pDX, IDC_APF_CBO_1099_FORM, m_cbo1099Form, IDC_APF_LBL_1099_FORM);
	DDX_Text(pDX, IDC_APF_CBO_1099_FORM, m_sz1099Form);
	DDX_Control(pDX, IDC_APF_LBL_1099_FORM, m_lbl1099Form);
	//RCG - 10/27/2023 - PBI 63016 - Changed Payment Year from textbox to drop-down listbox
	DDX_CMSControl(pDX, IDC_APF_CBO_PAYMENT_YEAR, m_cboPaymentYear, IDC_APF_LBL_PAYMENT_YEAR);
	DDX_Text(pDX, IDC_APF_CBO_PAYMENT_YEAR, m_szPaymentYear);
	DDX_CMSControl(pDX, IDC_APF_TXT_BREAK_AMOUNT, m_txtBreakAmount, IDC_APF_LBL_BREAK_AMOUNT);
	DDX_Text(pDX, IDC_APF_TXT_BREAK_AMOUNT, m_szBreakAmount);
	DDX_CMSControl(pDX, IDC_APF_TXT_PAYER_TIN, m_txtPayerTIN, IDC_APF_LBL_PAYER_TIN);
	DDX_Text(pDX, IDC_APF_TXT_PAYER_TIN, m_szPayerTIN);

	DDX_CMSControl(pDX, IDC_APF_TXT_PAYER_NAME_CTRL, m_txtPayerNameCtrl, IDC_APF_LBL_PAYER_NAME_CTRL);
	DDX_Text(pDX, IDC_APF_TXT_PAYER_NAME_CTRL, m_szPayerNameCtrl);
	DDX_CMSControl(pDX, IDC_APF_TXT_START_VENDOR_NUMBER, m_txtStartVendorNumber);	// begbert 06-23-2006 1-24737 : was previously associated with label, causing problems therewith.
	DDX_Text(pDX, IDC_APF_TXT_START_VENDOR_NUMBER, m_szStartVendorNumber);
	DDX_CMSControl(pDX, IDC_APF_TXT_END_VENDOR_NUMBER, m_txtEndVendorNumber);
	DDX_Text(pDX, IDC_APF_TXT_END_VENDOR_NUMBER, m_szEndVendorNumber);

	DDX_Control(pDX, IDC_APF_CHK_USE_LAST_YEAR, m_chkUseLastYear);
	DDX_Check(pDX, IDC_APF_CHK_USE_LAST_YEAR, m_bchkUseLastYear);
	DDX_Control(pDX, IDC_APF_CHK_TEST_INDICATOR, m_chkTestIndicator);
	DDX_Check(pDX, IDC_APF_CHK_TEST_INDICATOR, m_bchkTestIndicator);
	DDX_Control(pDX, IDC_APF_CHK_SERVICE_BUREAU, m_chkServiceBureauIndicator);
	DDX_Check(pDX, IDC_APF_CHK_SERVICE_BUREAU, m_bchkServiceBureauIndicator);
	DDX_Control(pDX, IDC_APF_CHK_LAST_FILING_INDICATOR, m_chkLastFilingIndicator);
	DDX_Check(pDX, IDC_APF_CHK_LAST_FILING_INDICATOR, m_bchkLastFilingIndicator);

	//12/26/2006 CJB - 1-25109  //Added Cartridge Tale File Indicator
	DDX_Control(pDX, IDC_APF_CHK_CARTRIDGE_TAPE_FILE_INDICATOR, m_chkCartridgeTapeFileIndicator);
	DDX_Check(pDX, IDC_APF_CHK_CARTRIDGE_TAPE_FILE_INDICATOR, m_bchkCartridgeTapeFileIndicator);

	//12/05/2007 CJB - 1-28229  //Added Combined Federal/State Filing Program
	DDX_Control(pDX, IDC_APF_CHK_COMBINED_FED_STATE, m_chkCombinedFederalState);
	DDX_Check(pDX, IDC_APF_CHK_COMBINED_FED_STATE, m_bchkCombinedFederalState);
}
//------------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CRFV_APElectronicFile1099Std, CReportFormView)
	ON_COMMAND(IDC_APF_BTN_PATH, &CRFV_APElectronicFile1099Std::SelectDestDir)
	ON_EN_KILLFOCUS(IDC_APF_TXT_PATH, &CRFV_APElectronicFile1099Std::OnEnKillfocusApfTxtPath)
	ON_EN_KILLFOCUS(IDC_APF_TXT_PAYER_NAME_CTRL, &CRFV_APElectronicFile1099Std::OnEnKillfocusApfTxtPayerNameCtrl)
	ON_BN_CLICKED(IDC_APF_BTN_OK, &CRFV_APElectronicFile1099Std::OnFakeOK)
	ON_CBN_SELCHANGE(IDC_APF_CBO_PAYMENT_YEAR, &CRFV_APElectronicFile1099Std::OnCbnSelchangeApfCboPaymentYear)
	ON_CBN_SELCHANGE(IDC_APF_CBO_EFILE_WITH, &CRFV_APElectronicFile1099Std::OnCbnSelchangeApfCboEfileWith)
END_MESSAGE_MAP()
//------------------------------------------------------------------------------
// CRFV_APElectronicFile1099 diagnostics
#ifdef _DEBUG
void CRFV_APElectronicFile1099Std::AssertValid() const
{
	CReportFormView::AssertValid();
}

void CRFV_APElectronicFile1099Std::Dump(CDumpContext& dc) const
{
	CReportFormView::Dump(dc);
}
#endif //_DEBUG
//------------------------------------------------------------------------------
// CRFV_APElectronicFile1099 message handlers
void CRFV_APElectronicFile1099Std::InitControls()
{
	//	FillControlMap();
	InitAutofillFields();
	SetControlTextLimits();

	//RCG - 10/27/2023 - PBI 63016 - Changed Payment Year from textbox to drop-down listbox
	LoadComboBox();
	GetDlgItem(IDC_APF_BTN_PATH)->SetFocus();
	CString szFedID;
	szFedID = si->ModuleSettings.APSettings.FederalID1099;
	szFedID.Replace(_T("-"), _T(""));
	szFedID = szFedID.Left(2) + "-" + szFedID.Mid(2, 7);
	m_txtPayerTIN.SetWindowText(szFedID);
	m_txtPath.SetWindowText(_T("C:\\"));

	// begbert 05-05-2006 1-24247 : removed the Service Bureau Indicator checkbox.
	m_chkServiceBureauIndicator.ShowWindow(SW_HIDE);

	//Product Backlog Item 1807:D_AP_1099 eFile file needs checked
	m_chkCartridgeTapeFileIndicator.ShowWindow(SW_HIDE);

	//RCG - 09/24/2025 - PBI 64621 - Added IRIS
	OnCbnSelchangeApfCboEfileWith();
}
//------------------------------------------------------------------------------
	//RCG - 10/27/2023 - PBI 63016 - Changed Payment Year from textbox to drop-down listbox
void CRFV_APElectronicFile1099Std::LoadComboBox(void)
{
	//RCG - 09/24/2025 - PBI 64621 - Added IRIS
	//Load E-file with combo box
	m_cboEFileWith.ResetContent();
	m_cboEFileWith.AddString(CGBLResources::GetResourceString(IDS_FIRE));
	m_cboEFileWith.AddString(CGBLResources::GetResourceString(IDS_IRIS));
	m_cboEFileWith.SetCurSel(0);

	//Load Form combo box
	m_cbo1099Form.ResetContent();
	m_cbo1099Form.AddString(CGBLResources::GetResourceString(IDS_1099_MISC));
	m_cbo1099Form.AddString(CGBLResources::GetResourceString(IDS_1099_NEC));
	m_cbo1099Form.SetCurSel(0);

	//Load Payment Year combo box
	m_cboPaymentYear.ResetContent();
}
//------------------------------------------------------------------------------
void CRFV_APElectronicFile1099Std::LoadCombinedFedStateMap()
{
	//RCG - 10/27/2023 - PBI 63016 - Changed Payment Year from textbox to drop-down listbox
	//Clear first since it now gets populated with each change of Payment Year selection
	mapCombinedFedState.RemoveAll();
	int	nPayYear = _wtoi(m_szPaymentYear);
	//07/25/2006 CJB - 1-25084
	mapCombinedFedState.SetAt(_T("AL"), _T("01"));	//Alabama
	mapCombinedFedState.SetAt(_T("AZ"), _T("04"));	//Arizona
	mapCombinedFedState.SetAt(_T("AR"), _T("05"));	//Arkansas
	mapCombinedFedState.SetAt(_T("CA"), _T("06"));	//California
	mapCombinedFedState.SetAt(_T("CO"), _T("07"));	//Colorado
	mapCombinedFedState.SetAt(_T("CT"), _T("08"));	//Connecticut
	mapCombinedFedState.SetAt(_T("DE"), _T("10"));	//Delaware
	//RCG - 10/20/2023 - PBI 62409 - Added for tax year 2023
	mapCombinedFedState.SetAt(_T("DC"), _T("11"));	//District of Columbia
	mapCombinedFedState.SetAt(_T("GA"), _T("13"));	//Georgia
	mapCombinedFedState.SetAt(_T("HI"), _T("15"));	//Hawaii
	mapCombinedFedState.SetAt(_T("ID"), _T("16"));	//Idaho
	mapCombinedFedState.SetAt(_T("IN"), _T("18"));	//Indiana
	mapCombinedFedState.SetAt(_T("KS"), _T("20"));	//Kansas
	mapCombinedFedState.SetAt(_T("LA"), _T("22"));	//Louisiana
	mapCombinedFedState.SetAt(_T("ME"), _T("23"));	//Maine
	mapCombinedFedState.SetAt(_T("MD"), _T("24"));	//Maryland
	mapCombinedFedState.SetAt(_T("MA"), _T("25"));	//Massachusetts
	//11.18.2013 - Bug 12744:D_AP_Magnetic Media 1099-MISC: add Michigan (code 26) and Vermont (code 50) to Combined Federal/State Filing Program
	mapCombinedFedState.SetAt(_T("MI"), _T("26"));	//Michigan
	mapCombinedFedState.SetAt(_T("MN"), _T("27"));	//Minnesota
	mapCombinedFedState.SetAt(_T("MS"), _T("28"));	//Mississippi
	mapCombinedFedState.SetAt(_T("MT"), _T("30"));	//Montana
	mapCombinedFedState.SetAt(_T("NE"), _T("31"));	//Nebraska
	mapCombinedFedState.SetAt(_T("NJ"), _T("34"));	//New Jersey
	mapCombinedFedState.SetAt(_T("NM"), _T("35"));	//New Mexico
	mapCombinedFedState.SetAt(_T("NC"), _T("37"));	//North Carolina
	mapCombinedFedState.SetAt(_T("ND"), _T("38"));	//North Dakota
	mapCombinedFedState.SetAt(_T("OH"), _T("39"));	//Ohio
	//	PBI 44536	08/22/2018	mvt - Added Oklahoma
	mapCombinedFedState.SetAt(_T("OK"), _T("40"));	//Oklahoma
	//RS - 01/15/2026 - PBI 65630 - Added for tax year 2025
	mapCombinedFedState.SetAt(_T("OR"), _T("41"));	//Oregon
	//RCG - 10/20/2023 - PBI 62409 - Added for tax year 2023
	mapCombinedFedState.SetAt(_T("PA"), _T("42"));	//Pennsylvania
	//RCG - 11/21/2024 - PBI 64417 - Added for  tax year 2024
	mapCombinedFedState.SetAt(_T("RI"), _T("44"));	//Rhode Island
	mapCombinedFedState.SetAt(_T("SC"), _T("45"));	//South Carolina
	mapCombinedFedState.SetAt(_T("WI"), _T("55"));	//Wisconsin
}
//------------------------------------------------------------------------------
void CRFV_APElectronicFile1099Std::LoadResources(void)
{
	CGBLForm::SetFormTitle(this, IDS_AP_ELECTRONIC_FILE_TEN_NINETY_NINE_REPORT_TITLE);
	//	m_icoTop.SetIcon(si->Drawing.Icons.Reports());
	SetFormDescription(IDS_AP_ELECTRONIC_FILE_TEN_NINETY_NINE_REPORT_HEADER);

	SetResourceText(IDC_APF_LBL_PATH, IDS_AP_DESTINATION_DRIVE_C_SL_PATH);
	//RCG - 09/24/2025 - PBI 64621 - Added IRIS
	SetResourceText(IDC_APF_LBL_EFILE_WITH, IDS_EFILE_WITH);
	SetResourceText(IDC_APF_LBL_1099_FORM, IDS_FORM);
	SetResourceText(IDC_APF_LBL_PAYMENT_YEAR, IDS_AP_TAX_YEAR);
	SetResourceText(IDC_APF_LBL_BREAK_AMOUNT, IDS_AP_BREAK_AMOUNT);
	SetResourceText(IDC_APF_LBL_PAYER_TIN, IDS_AP_PAYER_TIN);
	////	PBI 41369	10/20/2017	mvt - Added State taxpayer ID
	//SetResourceText(IDC_APF_LBL_PAYER_STATEID, IDS_SE_IDTYPE_STATE_ID);
	SetResourceText(IDC_APF_LBL_PAYER_NAME_CTRL, IDS_AP_PAYER_NAME_CONTROL);
	SetResourceText(IDC_APF_CHK_USE_LAST_YEAR, IDS_AP_LBL_USELASTYEAR_1099_AMOUNT);
	SetResourceText(IDC_APF_CHK_TEST_INDICATOR, IDS_AP_TEST_INDICATOR);
	SetResourceText(IDC_APF_CHK_SERVICE_BUREAU, IDS_AP_SERVICE_BUREAU_INDICATOR);
	SetResourceText(IDC_APF_CHK_LAST_FILING_INDICATOR, IDS_AP_LAST_FILING_INDICATOR);
	SetResourceText(IDC_APF_CHK_CARTRIDGE_TAPE_FILE_INDICATOR, IDS_AP_CARTRIDGE_TAPE_FILE_INDICATOR); //Product Backlog Item 1807:D_AP_1099 eFile file needs checked //12/26/2006 CJB - 1-25109  //Added Cartridge Tale File Indicator
	SetResourceText(IDC_APF_CHK_COMBINED_FED_STATE, IDS_AP_COMBINED_FED_STATE);  //12/05/2007 CJB - 1-28229  //Added Combined Federal/State Filing Program
	SetResourceText(IDC_APF_LBL_START, IDS_START);
	SetResourceText(IDC_APF_LBL_END, IDS_END);
	SetResourceText(IDC_APF_LBL_VENDOR_NUMBER, IDS_VENDOR_NUMBER);
	SetResourceText(IDC_APF_BTN_OK, IDS_OK);
}
//------------------------------------------------------------------------------
/*
void CRFV_APMagneticMedia1099MISCStd::FillControlMap(void)
{
	const CString szVendor(_T("strVendor") );

	m_clsXML.MySetElement(_T("0"), szVendor, &m_txtStartVendorNumber, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("1"), szVendor, &m_txtEndVendorNumber, NULL, STRINGTYPE);
}
*/
//------------------------------------------------------------------------------
void CRFV_APElectronicFile1099Std::InitAutofillFields(void)
{
	m_txtStartVendorNumber.SetDataAutoFillName(_T("AP_VENDORMASTER"));
	m_txtStartVendorNumber.SetDataLookupID(LKP_AP_VENDOR_AP_ONLY);
	m_txtStartVendorNumber.SetDataFormName(_T("AP_VendorMaster"));
	m_txtStartVendorNumber.SetDataModuleID(si->ApplicationSettings.ModuleID);
	m_txtStartVendorNumber.SetDataModuleName(si->ApplicationSettings.ModuleStringID);
	m_txtStartVendorNumber.SetDataPtrXMLParams(&m_clsXMLParams);
	m_txtStartVendorNumber.SetDataPtrLookup(&m_pLookup);
	m_txtStartVendorNumber.SetDataDefaultString(CGBLResources::GetResourceString(IDS_START));
	m_txtStartVendorNumber.SetDataFetchRecord(false);
	m_txtStartVendorNumber.SetAddonFlyFromLookup(false);
	m_txtStartVendorNumber.SetViewFromLookup(false);
	m_txtStartVendorNumber.SetAutoFillKey(_T("strVendor"));
	m_txtStartVendorNumber.SetAutoFillControlType();

	m_txtEndVendorNumber = m_txtStartVendorNumber;
	m_txtEndVendorNumber.SetDataDefaultString(CGBLResources::GetResourceString(IDS_END));
	m_txtStartVendorNumber.SetAsStartCtrl();
	m_txtStartVendorNumber.SetEndCtrlPtr(&m_txtEndVendorNumber);
}
//------------------------------------------------------------------------------
void CRFV_APElectronicFile1099Std::SetControlTextLimits(void)
{
	m_txtPayerTIN.SetLimitText(10);
	////	PBI 41369	10/20/2017	mvt - Added State taxpayer ID
	//m_txtPayerStateID.SetLimitText(10);

	m_txtPayerNameCtrl.SetLimitText(10);

	m_txtBreakAmount.SetHighlightOnSetFocus(TRUE);
	m_txtBreakAmount.m_bDefaultZeros = true;
	m_txtBreakAmount.m_bNoNegativeNumber = true;
	m_txtBreakAmount.SetNegativeFormat(_T("-$1.1"));
}
//------------------------------------------------------------------------------
void CRFV_APElectronicFile1099Std::SetControlSizes()
{
	m_pEXLayoutManager->AddLBLControlToGroup(0, IDC_APF_LBL_PATH);
	m_pEXLayoutManager->AddLBLControlToGroup(1, IDC_APF_LBL_EFILE_WITH);	//RCG - 09/24/2025 - PBI 64621 - Added IRIS
	m_pEXLayoutManager->AddLBLControlToGroup(1, IDC_APF_LBL_PAYMENT_YEAR);
	m_pEXLayoutManager->AddLBLControlToGroup(1, IDC_APF_LBL_BREAK_AMOUNT);
	m_pEXLayoutManager->AddLBLControlToGroup(2, IDC_APF_LBL_1099_FORM);	//RCG - 09/24/2025 - PBI 64621 - Added IRIS
	m_pEXLayoutManager->AddLBLControlToGroup(2, IDC_APF_LBL_PAYER_TIN);
	m_pEXLayoutManager->AddLBLControlToGroup(2, IDC_APF_LBL_PAYER_NAME_CTRL);
	m_pEXLayoutManager->AddLBLControlToGroup(0, IDC_APF_LBL_START);
	m_pEXLayoutManager->AddLBLControlToGroup(0, IDC_APF_LBL_END);
	m_pEXLayoutManager->AddLBLControlToGroup(1, IDC_APF_LBL_VENDOR_NUMBER);

	m_pEXLayoutManager->SizeToFit(IDC_APF_BTN_PATH, SIZ_TextBox, 3);
	m_pEXLayoutManager->SizeToFit(IDC_APF_TXT_PATH, SIZ_TextBox, 60);

	//RCG - 09/24/2025 - PBI 64621 - Added IRIS
	m_pEXLayoutManager->SizeToFit(IDC_APF_CBO_EFILE_WITH, SIZ_ComboBox, 2);
	m_pEXLayoutManager->SizeToFit(IDC_APF_CBO_1099_FORM, SIZ_ComboBox, 8);

	//RCG - 10/27/2023 - PBI 63016 - Changed Payment Year from textbox to drop-down listbox
	m_pEXLayoutManager->SizeToFit(IDC_APF_CBO_PAYMENT_YEAR, SIZ_ComboBox, 2);
	m_pEXLayoutManager->SizeToFit(IDC_APF_TXT_BREAK_AMOUNT, SIZ_TextBox, 12);
	m_pEXLayoutManager->SizeToFit(IDC_APF_TXT_PAYER_TIN, SIZ_TextBox, 12);
	////	PBI 41369	10/20/2017	mvt - Added State taxpayer ID
	//m_pEXLayoutManager->SizeToFit(IDC_APF_TXT_PAYER_STATEID, SIZ_TextBox, 12);
	m_pEXLayoutManager->SizeToFit(IDC_APF_TXT_PAYER_NAME_CTRL, SIZ_TextBox, 12);
	m_pEXLayoutManager->SizeToFit(IDC_APF_TXT_START_VENDOR_NUMBER, SIZ_TextBox, 20);
	m_pEXLayoutManager->SizeToFit(IDC_APF_TXT_END_VENDOR_NUMBER, SIZ_TextBox, 20);
	m_pEXLayoutManager->SizeToFit(IDC_APF_CHK_USE_LAST_YEAR, SIZ_CheckBox);
	m_pEXLayoutManager->SizeToFit(IDC_APF_CHK_TEST_INDICATOR, SIZ_CheckBox);
	m_pEXLayoutManager->SizeToFit(IDC_APF_CHK_SERVICE_BUREAU, SIZ_CheckBox);
	m_pEXLayoutManager->SizeToFit(IDC_APF_CHK_LAST_FILING_INDICATOR, SIZ_CheckBox);
	//m_pEXLayoutManager->SizeToFit(IDC_APF_CHK_CARTRIDGE_TAPE_FILE_INDICATOR , SIZ_CheckBox    ); //Product Backlog Item 1807:D_AP_1099 eFile file needs checked //12/26/2006 CJB - 1-25109  //Added Cartridge Tale File Indicator
	m_pEXLayoutManager->SizeToFit(IDC_APF_CHK_COMBINED_FED_STATE, SIZ_CheckBox);  //12/05/2007 CJB - 1-28229  //Added Combined Federal/State Filing Program
}

//**************************************************************************
//* Procedure:     SetupLayoutManager
//*
//* Description:   Positions the controls and sets up resizing rules.
//*
//* Parameters:    None
//*
//* Date Created:  10/26/2005 by Mike Gregg
//**************************************************************************
void CRFV_APElectronicFile1099Std::SetupLayoutManager()
{
	m_pEXLayoutManager->AnchorControlTop(IDC_APF_LBL_PATH, NULL);
	m_pEXLayoutManager->AnchorControlTop(IDC_APF_BTN_PATH, IDC_APF_LBL_PATH);
	m_pEXLayoutManager->AnchorControlTop(IDC_APF_TXT_PATH, IDC_APF_LBL_PATH);

	//RCG - 09/24/2025 - PBI 64621 - Added IRIS
	m_pEXLayoutManager->AnchorLBLControlTop(IDC_APF_LBL_EFILE_WITH, IDC_APF_CBO_EFILE_WITH, IDC_APF_TXT_PATH);
	m_pEXLayoutManager->AnchorLBLControlTop(IDC_APF_LBL_1099_FORM, IDC_APF_CBO_1099_FORM, IDC_APF_TXT_PATH);
	m_pEXLayoutManager->AnchorLBLControlTop(IDC_APF_LBL_PAYER_NAME_CTRL, IDC_APF_TXT_PAYER_NAME_CTRL, IDC_APF_TXT_PAYER_TIN);
	m_pEXLayoutManager->AnchorLBLControlTop(IDC_APF_LBL_PAYER_TIN, IDC_APF_TXT_PAYER_TIN, IDC_APF_CBO_1099_FORM);

	//RCG - 10/27/2023 - PBI 63016 - Changed Payment Year from textbox to drop-down listbox
	m_pEXLayoutManager->AnchorLBLControlTop(IDC_APF_LBL_PAYMENT_YEAR, IDC_APF_CBO_PAYMENT_YEAR, IDC_APF_CBO_EFILE_WITH);
	m_pEXLayoutManager->AnchorLBLControlTop(IDC_APF_LBL_BREAK_AMOUNT, IDC_APF_TXT_BREAK_AMOUNT, IDC_APF_CBO_PAYMENT_YEAR);
	m_pEXLayoutManager->AnchorControlTop(IDC_APF_CHK_USE_LAST_YEAR, IDC_APF_TXT_BREAK_AMOUNT);
	m_pEXLayoutManager->AnchorControlTop(IDC_APF_CHK_TEST_INDICATOR, IDC_APF_CHK_USE_LAST_YEAR);
	//	m_pEXLayoutManager->AnchorControlTop       (IDC_APF_CHK_SERVICE_BUREAU                , IDC_APF_CHK_TEST_INDICATOR  );

		//12/26/2006 CJB - 1-25109  //Added Cartridge Tale File Indicator, and align with other check boxes
	m_pEXLayoutManager->AnchorControlSameTop(IDC_APF_CHK_LAST_FILING_INDICATOR, IDC_APF_CHK_USE_LAST_YEAR);
	m_pEXLayoutManager->AnchorControlSameTop(IDC_APF_CHK_COMBINED_FED_STATE, IDC_APF_CHK_TEST_INDICATOR);//Product Backlog Item 1807:D_AP_1099 eFile file needs checked

	//12/05/2007 CJB - 1-28229  //Added Combined Federal/State Filing Program, and align with other check boxes
	m_pEXLayoutManager->AnchorControlTop(IDC_APF_LBL_END, IDC_APF_CHK_COMBINED_FED_STATE);
	m_pEXLayoutManager->AnchorControlTop(IDC_APF_TXT_END_VENDOR_NUMBER, IDC_APF_LBL_END);

	//12/05/2007 CJB - 1-28229  //Added Combined Federal/State Filing Program, and align with other check boxes
	m_pEXLayoutManager->AnchorControlTop(IDC_APF_LBL_START, IDC_APF_CHK_COMBINED_FED_STATE);
	m_pEXLayoutManager->AnchorLBLControlTop(IDC_APF_TXT_START_VENDOR_NUMBER, IDC_APF_LBL_VENDOR_NUMBER, IDC_APF_LBL_START);

	//RCG - 10/27/2023 - PBI 63016 - Changed Payment Year from textbox to drop-down listbox
	//RCG - 09/24/2025 - PBI 64621 - Added IRIS
	m_pEXLayoutManager->AnchorLBLControlLeft(IDC_APF_CBO_EFILE_WITH, IDC_APF_LBL_EFILE_WITH, NULL);
	m_pEXLayoutManager->AnchorLBLControlLeft(IDC_APF_CBO_PAYMENT_YEAR, IDC_APF_LBL_PAYMENT_YEAR, NULL);
	m_pEXLayoutManager->AnchorLBLControlLeft(IDC_APF_TXT_BREAK_AMOUNT, IDC_APF_LBL_BREAK_AMOUNT, NULL);

	m_pEXLayoutManager->AnchorControlSameLeft(IDC_APF_LBL_START, IDC_APF_TXT_START_VENDOR_NUMBER);
	m_pEXLayoutManager->AnchorLBLControlLeft(IDC_APF_TXT_START_VENDOR_NUMBER, IDC_APF_LBL_VENDOR_NUMBER, NULL);
	m_pEXLayoutManager->AnchorControlSameLeft(IDC_APF_LBL_PATH, IDC_APF_TXT_START_VENDOR_NUMBER);
	m_pEXLayoutManager->AnchorControlSameLeft(IDC_APF_TXT_PATH, IDC_APF_TXT_START_VENDOR_NUMBER);
	m_pEXLayoutManager->AnchorControlFollowX(IDC_APF_BTN_PATH, IDC_APF_TXT_PATH);

	//12/06/2007 CJB - 1-28229  //Added Combined Federal/State Filing Program, and align with other check boxes
	m_pEXLayoutManager->AnchorControlSameLeft(IDC_APF_CHK_USE_LAST_YEAR, IDC_APF_BTN_PATH);
	m_pEXLayoutManager->AnchorControlSameLeft(IDC_APF_CHK_TEST_INDICATOR, IDC_APF_BTN_PATH);

	m_pEXLayoutManager->AnchorControlSameRight(IDC_APF_CBO_1099_FORM, IDC_APF_TXT_PATH);
	m_pEXLayoutManager->AnchorControlSameRight(IDC_APF_TXT_PAYER_TIN, IDC_APF_TXT_PATH);
	m_pEXLayoutManager->AnchorControlSameRight(IDC_APF_TXT_PAYER_NAME_CTRL, IDC_APF_TXT_PATH);
	m_pEXLayoutManager->AnchorControlFollowX(IDC_APF_LBL_1099_FORM, IDC_APF_CBO_1099_FORM);
	m_pEXLayoutManager->AnchorControlFollowX(IDC_APF_LBL_PAYER_TIN, IDC_APF_TXT_PAYER_TIN);

	m_pEXLayoutManager->AnchorControlFollowX(IDC_APF_LBL_PAYER_NAME_CTRL, IDC_APF_TXT_PAYER_NAME_CTRL);
	m_pEXLayoutManager->AnchorControlSameRight(IDC_APF_TXT_PAYER_NAME_CTRL, IDC_APF_TXT_PATH);

	//12/26/2006 CJB - 1-25109  //Added Cartridge Tale File Indicator, and align with other check boxes
	//12/06/2007 CJB - 1-28229  //Added Combined Federal/State Filing Program, and align with other check boxes
	m_pEXLayoutManager->AnchorControlSameRight(IDC_APF_CHK_COMBINED_FED_STATE, IDC_APF_TXT_PATH);
	m_pEXLayoutManager->AnchorControlSameLeft(IDC_APF_CHK_LAST_FILING_INDICATOR, IDC_APF_CHK_COMBINED_FED_STATE);

	m_pEXLayoutManager->AnchorControlLeft(IDC_APF_TXT_END_VENDOR_NUMBER, IDC_APF_TXT_START_VENDOR_NUMBER, 10);
	m_pEXLayoutManager->AnchorControlSameLeft(IDC_APF_LBL_END, IDC_APF_TXT_END_VENDOR_NUMBER);
}

//**************************************************************************
//* Procedure:     GetFormBounds
//*
//* Description:   Retrieves the form bounds.
//*
//* Parameters:    pRightBound - Control that the form should size its right side to.
//*				   pBottomBound - Control that the form should size its bottom side to.
//*
//* Date Created:  10/25/2005 by Mike Gregg
//**************************************************************************
void CRFV_APElectronicFile1099Std::GetFormBounds(CWnd*& pRightBound, CWnd*& pBottomBound)
{
	pRightBound = &m_txtPath;
	pBottomBound = &m_txtEndVendorNumber;

	__super::GetFormBounds(pRightBound, pBottomBound);
}

//------------------------------------------------------------------------------
//Brings up a folder selection dialog using SHBrowseForFolder.
void CRFV_APElectronicFile1099Std::SelectDestDir()
{
	//RCG - 4/3/2007 - 1-26183/1-26213 - Changed Browse for Folder description
	//CString szTitle = CGBLResources::GetResourceString(IDS_SELECT_FOLDER_FOR_BACKUP) ;
	CString szTitle = CGBLResources::GetResourceString(IDS_BROWSE_FOR_FOLDER_MESSAGE);
	CString szDir;
	GetDlgItemText(IDC_APF_TXT_PATH, szDir);

	szDir = CGBLForm::BrowseForDir(szTitle.GetBuffer(), szDir, GetSafeHwnd());
	if (!szDir.IsEmpty())
	{
		SetDlgItemText(IDC_APF_TXT_PATH, szDir);
		m_txtPath.SetSel(m_txtPath.LineLength(), m_txtPath.LineLength());
	}
}

//Defect ID 1-24466, PGP(05/30/2006) Commented.
//------------------------------------------------------------------------------
//void CRFV_APMagneticMedia1099MISCStd::OnEnSetfocusApfTxtBreakAmount()
//{
//	int nPos  = 1 ;
//	m_txtBreakAmount.PostMessage(EM_SETSEL, (WPARAM)nPos, (LPARAM)-1) ;
//}
//------------------------------------------------------------------------------
void CRFV_APElectronicFile1099Std::OnEnKillfocusApfTxtPath()
{
	UpdateData();
	//Must set path to app path if less than 3 chars
	if (m_szPath.GetLength() < 3) {
		m_txtPath.SetWindowText(si->ApplicationPaths.szApplicationPath);
	}

	m_txtPath.GetWindowText(m_szPath);
	if (m_szPath.Right(1) != _T("\\")) {
		m_szPath.Append(_T("\\"));
		m_txtPath.SetWindowText(m_szPath);
	}
}
//------------------------------------------------------------------------------
void CRFV_APElectronicFile1099Std::OnEnKillfocusApfTxtPayerNameCtrl()
{
	UpdateData();
	//Must make exactly 4 chars...either take the left 4 chars or pad with spaces after to make 4 chars
	if (m_szPayerNameCtrl.GetLength() < 4) {
		m_szPayerNameCtrl.Append(_T("    "));
	}

	m_txtPayerNameCtrl.SetWindowText(m_szPayerNameCtrl.Left(4));
}
//------------------------------------------------------------------------------
// 1-33838 BK 12/02/09 electronic records are limited by the IRS to 2.5 million records (server code was changed to return up to 10,001 records with each server call)
// if 10,0001 records are returned from server this code will make additional server calls using <strVendor> from the last record as the StartVendor for the next server call
// if 2.5 million records are reached client will display a message informing the user to reset the StartVendor to <strVendor> from the last record
void CRFV_APElectronicFile1099Std::OnFakeOK()
{
	m_nServerCallCount = 0;
	m_nNumberARecords = 0;		//RS - 11/21/2024 - PBI 64417 - Added
	m_nNumberBMiscRecords = 0;
	m_nNumberBNecRecords = 0;
	m_nServerStatus = SERVER_BEGIN;
	m_szNextVendor = EMPTY_STRING;
	bool bSuccess = false;

	// 1-33838 BK 12/02/09 moved from Write1099Data()
	m_bCancel = false;
	m_bNewDisk = true;
	m_nFilecount = 0;
	m_nRecordSequenceNumber = 0;	//12/07/2007 CJB - 1-28229
	ResetTotals();

	try
	{
		m_dlgCMSReportButtons.m_bReturnBackToFormDoNotRunReport = true; // Do not Run this Report at this point.
		if (Validate())
		{
			do
			{
				// records exceed 2.5 million display message: "The file size has exceeded the maximum records allowed. To complete the process, you must run the report again beginning with <m_szDisplayVendor>."
				m_nServerCallCount++;
				if (m_nServerCallCount > MAX_SERVER_CALLS)
				{
					CGBLForm::CMSMessageBox(this, IDS_MAX_FILE_EXCEEDED_MSG, IDS_MAX_FILE_EXCEEDED, MB_OK | MB_ICONINFORMATION, m_szNextVendor);
					m_nServerStatus = SERVER_FINISHED;
				}
				else
				{
					// Go Get the Data
					if (!Load1099Data())
						return;
				}

					//Write the file information
					bSuccess = Write1099Data();
				
				if (!bSuccess) break;

			} while (m_nServerStatus != SERVER_FINISHED);

			if (bSuccess)
			{
				// Now Run 4804 Report.
				//RCG - 12/07/2016 - PBI 30627 - Removed Form 4804 functionality
				//Create4804Report();
				//Display Message: "Finished Electronic File 1099-MISC Processing."
				CGBLForm::CMSMessageBox(this, IDS_AP_FINISHED_ELECTRONIC_FILE_1099_PROCESS, IDS_AP_1099_PROCESSING, MB_OK | MB_ICONINFORMATION);
			}
			else if (m_bCancel)
			{
				//Display Message:  "Electronic File 1099-MISC Processing Is Incomplete."
				CGBLForm::CMSMessageBox(this, IDS_AP_INCOMPLETE_ELECTRONIC_FILE_1099_PROCESS, IDS_AP_1099_PROCESSING, MB_OK | MB_ICONSTOP);
			}
		}
	}
	catch (_com_error& e)
	{
		CGBLForm::HandleError(e);
	}
}
//------------------------------------------------------------------------------
bool CRFV_APElectronicFile1099Std::Validate(void)
{
	UpdateData();
	CString		szPayerTIN(m_szPayerTIN);
	szPayerTIN.Replace(_T("-"), _T(""));

	bool bValid = true;
	UINT	nMsg = IDS_EMPTY;
	UINT	nTitle = IDS_EMPTY;
	CString szText1;
	UINT unDriveType;

	unDriveType = GetDriveType(m_szPath);

	if (unDriveType == DRIVE_REMOVABLE) {
		m_bRemovableDrive = true;
	}
	else {
		m_bRemovableDrive = false;
	}

	if (unDriveType == DRIVE_NO_ROOT_DIR)
	{
		//Make sure drive exists
		m_txtPath.SetFocus();
		m_txtPath.SetSel(0, -1);
		nMsg = IDS_AP_ERR_MUST_ENTER_DRIVE_LETTER;
		nTitle = IDS_AP_ERR_AP_1099_MAGNETIC_MEDIA;
	}
	else if (unDriveType == DRIVE_REMOVABLE && m_szPath.GetLength() != 3)
	{//(m_szPath.Left(1) == "A" || m_szPath.Left(1) == "B") && m_szPath.GetLength() != 3){
	//Validate Root specified if removable media drive
		m_txtPath.SetFocus();
		m_txtPath.SetSel(0, -1);
		nMsg = IDS_AP_ERR_MUST_WRITE_TO_ROOT;
		nTitle = IDS_AP_ERR_AP_1099_MAGNETIC_MEDIA;
	}
	else if (szPayerTIN.GetLength() != 9)
	{
		//Validate Payer TIN is 9 characters (EIN)
		m_txtPayerTIN.SetFocus();
		m_txtPayerTIN.SetSel(0, -1);
		nMsg = IDS_AP_ERR_INVALID_TIN_MSG;
		nTitle = IDS_AP_ERR_INVALID_TIN_CAP;
	}
	else if (!CGBLForm::ValidateRange(this, m_szStartVendorNumber, m_szEndVendorNumber, IDS_AP_ERR_INVALID_VERDOR_NUMBER_RANGE))
	{
		//Validate Vendor Number Range (Start < End)
		m_txtEndVendorNumber.SetFocus();
		m_txtEndVendorNumber.SetSel(0, -1);
		bValid = false;
	}
	else if (!ValidateMagMedia(nMsg))
	{
		//Check our Mag.Media constants for proper information...(Emp. ID and all Basic Return Address Info)
		if (CMS::GetSI()->CompanySettings.IsFund)
			szText1 = CGBLResources::GetResourceString(IDS_AP_ERR_INVALID_ELECTRONIC_CONSTANTS_FUND_MSG);
		else
			szText1 = CGBLResources::GetResourceString(IDS_AP_ERR_INVALID_ELECTRONIC_CONSTANTS_MSG);

		szText1.Replace(_T("<TEXT1>"), CGBLResources::GetResourceString(nMsg));
		nTitle = IDS_AP_ERR_INVALID_ELECTRONIC_CONSTANTS_CAP;
		//Set nMsg to 0 so we can use szText1 as the error msg.
		nMsg = 0;
	}

	if (nTitle != IDS_EMPTY)
	{
		CGBLForm::CMSMessageBox(this, nMsg, nTitle, MB_ICONSTOP, szText1);
		bValid = false;
	}

	return bValid;
}
//------------------------------------------------------------------------------
bool CRFV_APElectronicFile1099Std::ValidateMagMedia(UINT& nMsg)
{
	CString szTemp; //Used to convert numbers to strings

	si->ModuleSettings.APSettings.LoadMagMediaData();

	//07/24/2006 CJB - 1-25084  //Validate for 9 numbers, not counting the dash
	szTemp = si->MagneticMediaConstants.EmployerID;
	szTemp.Replace(_T("-"), _T(""));
	if (szTemp.GetLength() != 9) {
		//Validate Employee ID is 9 characters
		nMsg = IDS_AP_ERR_INVALID_EMPLOYER_ID_MSG;
	}
	else if (si->MagneticMediaConstants.ReturnName.Trim().GetLength() == 0) {
		//Validate Return Name not blank
		nMsg = 0;
		return false;
	}
	else if (si->MagneticMediaConstants.ReturnAddress.Trim().GetLength() == 0) {
		//Validate Return Address not blank
		nMsg = IDS_AP_ERR_INVALID_RETURN_ADDRESS_MSG;
	}
	else if (si->MagneticMediaConstants.ReturnCity.Trim().GetLength() == 0) {
		//Validate Return City not blank
		nMsg = IDS_AP_ERR_INVALID_RETURN_CITY_MSG;
	}
	else if (si->MagneticMediaConstants.ReturnState.Trim().GetLength() == 0) {
		//Validate Return State not blank
		nMsg = IDS_AP_ERR_INVALID_RETURN_STATE_MSG;
	}
	else if (si->MagneticMediaConstants.ReturnZip.Trim().GetLength() == 0) {
		//Validate Return PostalCode not blank
		nMsg = IDS_AP_ERR_INVALID_RETURN_ZIP_CODE_MSG;
	}

	return (nMsg == IDS_EMPTY ? true : false);
}
//------------------------------------------------------------------------------
//Load 1099 Data Returns true if records found else false
bool CRFV_APElectronicFile1099Std::Load1099Data()
{
	bool bSuccess = true;
	UpdateData();

	m_clsXML.m_szXMLCommand.Empty();
	m_clsXML.m_szXMLData.Empty();
	m_clsXML.MakeXMLCommand(_T("REPORT"), _T("AP"), _T("AP1099ELECTRONICFILE"));
	m_clsXML.m_szXMLFormat = _T("");
	m_clsXML.MakeXMLConnection(_T("Connection"), _T("10"), _T("300"), CTY_COMPANYSERVER, true);
	m_clsXML.MakeCOMPANYINFO();

	// bolTestReport, bolUseLastYear1099MiscAmt, szStartVendorNumber, szEndVendorNumber, szAdditionalFilterField, szStartUDF2, szEndUDF2
	CString dPrintAmounts = CGBLForm::CurrencyToNumeric(m_szBreakAmount);
	//RS - 11/25/2020 - PBI 56232 - Added 1099-NEC to Electronic File for 2020
	int	nPaymentYear = _wtoi(m_szPaymentYear);

	// 1-33838 BK 12/02/09 replace m_szStartVendorNumber with m_szNextVendor if continuing to make server calls and the NextVendor isn't empty
	if (m_nServerStatus == SERVER_CONTINUE && !m_szNextVendor.IsEmpty())
		m_szStartVendorNumber = m_szNextVendor;

	m_clsXML.MakeNode(_T("bolTestReport"), _T("0"), 3, XML_ATTRIBUTE); // bolTestReport = false
	m_clsXML.MakeNode(_T("szPrintAmountsOfAtLeast"), dPrintAmounts, 3, XML_ATTRIBUTE, true);
	m_clsXML.MakeNode(_T("bolUseLastYear1099Amt"), m_bchkUseLastYear ? _T("1") : _T("0"), 3, XML_ATTRIBUTE);
	m_clsXML.MakeNode(_T("szStartVendorNumber"), ReplaceStartEnd(m_szStartVendorNumber), 3, XML_ATTRIBUTE, true);
	m_clsXML.MakeNode(_T("szEndVendorNumber"), ReplaceStartEnd(m_szEndVendorNumber), 3, XML_ATTRIBUTE, true);
	m_clsXML.MakeNode(_T("szAdditionalFilterField"), _T(""), 3, XML_ATTRIBUTE, true);
	m_clsXML.MakeNode(_T("szUDF2Start"), _T(""), 3, XML_ATTRIBUTE, true);
	m_clsXML.MakeNode(_T("szUDF2End"), _T(""), 3, XML_ATTRIBUTE, true);
	//RS - 11/25/2020 - PBI 56232 - Added 1099-NEC to Electronic File for 2020
	m_clsXML.MakeNode(_T("bolAtLeastYear2020"), nPaymentYear >= PAYMENT_YEAR_2020 ? _T("1") : _T("0"), 3, XML_ATTRIBUTE);
	m_clsXML.MakeNode(_T("bolElectronicFile"), _T("1"), 3, XML_ATTRIBUTE); // bolElectronicFile = true
	//RCG - 11/30/2020 - PBI 52809 - Added 1099-NEC option button setting
	m_clsXML.MakeNode(_T("bolNEC"), _T("0"), 3, XML_ATTRIBUTE); // bolNEC = false
	//RS - 11/17/2021 - PBI 59039 - Added new 1099 Type to Electronic File for 2021
	m_clsXML.MakeNode(_T("bolAtLeastYear2021"), nPaymentYear >= PAYMENT_YEAR_2021 ? _T("1") : _T("0"), 3, XML_ATTRIBUTE);
	//RS - 12/5/2025 - PBI 65628/65629 - Added IRIS option to Electronic File for 2025
	BOOL bIsIRIS = (m_cboEFileWith.GetCurSel() == 1);
	BOOL b1099FormMISC = (m_cbo1099Form.GetCurSel() == 0);
	m_clsXML.MakeNode(_T("bolAtLeastYear2025"), nPaymentYear >= PAYMENT_YEAR_2025 ? _T("1") : _T("0"), 3, XML_ATTRIBUTE);
	m_clsXML.MakeNode(_T("bolIRIS"), bIsIRIS ? _T("1") : _T("0"), 3, XML_ATTRIBUTE);
	m_clsXML.MakeNode(_T("bolIRISMISC"), b1099FormMISC ? _T("1") : _T("0"), 3, XML_ATTRIBUTE);
	m_clsXML.GetXMLFormat();

	_bstr_t szReturnString1099 = m_clsXML.SendXML();

	//Check for errors
	if (m_clsXML.GetErrorReturnID() != 0) {
		CGBLForm::CMSMessageBox(this, 0, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONERROR, _T("There was an error in the 1099 Data retrieval."), _T(""), 0, false);
		bSuccess = false;
	}

	m_ds1099DataSet.Dispose();
	m_ds1099DataSet.LoadFromXML(szReturnString1099);

	/*int iTestVal =*/ m_ds1099DataSet.getTableCount();

	//Check for records
	if ((m_ds1099DataSet.getTableCount() <= 1))
	{
		// msg: "No electronic file 1099 records exist within the specified ranges."
		CGBLForm::CMSMessageBox(this, IDS_AP_NO_ELECTRONIC_FILE_EXISTS_WITHIN_RANGES, IDS_AP_1099_PROCESSING, MB_OK | MB_ICONSTOP);
		return false;
	}

	//set data to the data file
	m_ds1099DataSet.GetTable(_T("Table"), &m_dtTable);

	//RCG - 12/14/2020 - PBI 56232 - Check if table exists for 1099-MISC records
	if (m_ds1099DataSet.TableExists(_T("Table1")))
	{
		m_ds1099DataSet.GetTable(_T("Table1"), &m_dtBMiscRecords);

		CDataRow clsDRMisc;
		//m_NumberBRecords = 0; // 1-33838 BK 12/02/09 moved to OnFakeOK()

		//Set number of B Records for 1099-MISC
		for (int nRow = 0; nRow < m_dtBMiscRecords.getRowCount(); nRow++)
		{
			m_dtBMiscRecords.GetRow(nRow, &clsDRMisc);
			// 1-33838 BK 12/02/09 electronic records are limited by the IRS to 2.5 million records (server code was changed to return up to 10,001 records with each server call)
			// if 10,0001 records are returned from server this code will make additional server calls using <strVendor> from the last record as the StartVendor for the next server call
			// if 2.5 million records are reached client will display a message informing the user to reset the StartVendor to <strVendor> from the last record
			if (nRow < MAX_SERVER_RECORDS)
			{
				//RCG - 12/11/2014 - Bug 25692 - Include vendors with blank Federal ID number
				//if (clsDR.XMLStr(_T("strFederalIDNumber"), false) != _T(""))
				m_nNumberBMiscRecords++;
				m_nServerStatus = SERVER_FINISHED;
			}
			else if (nRow == MAX_SERVER_RECORDS)
			{
				m_szNextVendor = clsDRMisc.XMLStr(_T("strVendor"), false);
				m_nServerStatus = SERVER_CONTINUE;
			}
		}

		clsDRMisc.Dispose();
	}

	//RCG - 12/14/2020 - PBI 56232 - Check if table exists for 1099-NEC records
	if (m_ds1099DataSet.TableExists(_T("Table2")))
	{
		m_ds1099DataSet.GetTable(_T("Table2"), &m_dtBNecRecords);

		CDataRow clsDRNec;

		//Set number of B Records for 1099-NEC
		for (int nRow = 0; nRow < m_dtBNecRecords.getRowCount(); nRow++)
		{
			m_dtBNecRecords.GetRow(nRow, &clsDRNec);
			// 1-33838 BK 12/02/09 electronic records are limited by the IRS to 2.5 million records (server code was changed to return up to 10,001 records with each server call)
			// if 10,0001 records are returned from server this code will make additional server calls using <strVendor> from the last record as the StartVendor for the next server call
			// if 2.5 million records are reached client will display a message informing the user to reset the StartVendor to <strVendor> from the last record
			if (nRow < MAX_SERVER_RECORDS)
			{
				//RCG - 12/11/2014 - Bug 25692 - Include vendors with blank Federal ID number
				//if (clsDR.XMLStr(_T("strFederalIDNumber"), false) != _T(""))
				m_nNumberBNecRecords++;
				m_nServerStatus = SERVER_FINISHED;
			}
			else if (nRow == MAX_SERVER_RECORDS)
			{
				m_szNextVendor = clsDRNec.XMLStr(_T("strVendor"), false);
				m_nServerStatus = SERVER_CONTINUE;
			}
		}

		clsDRNec.Dispose();
	}

	return bSuccess;
}
//------------------------------------------------------------------------------
//RS - 11/25/2020 - PBI 56232 - Added 1099-NEC to Electronic File for 2020
bool CRFV_APElectronicFile1099Std::Write1099Data()
{
	//1-33838 BK 12/02/09 moved to OnFakeOK()
	//m_bCancel = false;
	//m_bNewDisk = true;
	//m_nFilecount = 0;
	//m_nRecordSequenceNumber = 0;		//12/07/2007 CJB - 1-28229
	//ResetTotals();					//12/10/2007 CJB - 1-28229
	
	//RS - 09/24/2025 - PBI 64621 - Added IRIS
	BOOL b1099Form = (m_cbo1099Form.GetCurSel() == 0);
	BOOL bEnable = (m_cboEFileWith.GetCurSel() == 0);

	if (!bEnable)
	{
		//IRIS
		if (b1099Form)
		{
			//MISC
			Write_IRIS_1099_MISC();
		}
		else
		{
			//NEC
			Write_IRIS_1099_NEC();
		}
	}
	// FIRE 
	else 
	{
		//1-33838 BK 12/02/09 if this is the first server call write record T to file
		if (m_nServerCallCount == 1)
		{
			if (!m_bCancel)
				Write1099T();
		}

		//Set flag for 1099-MISC
		m_b1099NEC = false;
		//Check if there are 1099-MISC "B" records to process
		if (m_nNumberBMiscRecords > 0)
		{
			//1-33838 BK 12/02/09 if this is the first server call write record A for 1099-MISC to file
			if (m_nServerCallCount == 1)
			{
				if (!m_bCancel)
					Write1099A();
			}

			//1-33838 BK 12/02/09 skip this if file is maxed out
			if (m_nServerCallCount <= MAX_SERVER_CALLS && !m_bCancel)
				Write1099B_MISC();

			//1-33838 BK 12/02/09 if this is the last server call write summary records to file
			if (m_nServerStatus == SERVER_FINISHED)
			{
				if (!m_bCancel)
					Write1099C_MISC();

				if (!m_bCancel && m_bchkCombinedFederalState)//DRR 12/14/09
					Write1099K();
			}
		}

		//Set flag for 1099-NEC
		m_b1099NEC = true;
		//Check if there are 1099-NEC "B" records to process
		if (m_nNumberBNecRecords > 0)
		{
			//RS 1/17/22 - PBI 59406 - Added reset for K Record
			ResetTotals();

			//1-33838 BK 12/02/09 if this is the first server call write record A for 1099-NEC to file
			if (m_nServerCallCount == 1)
			{
				if (!m_bCancel)
					Write1099A();
			}

			//1-33838 BK 12/02/09 skip this if file is maxed out
			if (m_nServerCallCount <= MAX_SERVER_CALLS && !m_bCancel)
				Write1099B_NEC();

			//1-33838 BK 12/02/09 if this is the last server call write summary records to file
			if (m_nServerStatus == SERVER_FINISHED)
			{
				if (!m_bCancel)
					Write1099C_NEC();

				//RS 1/17/22 - PBI 59406 - Added K Record 
				if (!m_bCancel && m_bchkCombinedFederalState)
					Write1099K();
			}
		}

		//1-33838 BK 12/02/09 if this is the last server call write summary records to file
		if (m_nServerStatus == SERVER_FINISHED)
		{
			if (!m_bCancel)
				Write1099F();
		}
	}

	return !m_bCancel;
}
//------------------------------------------------------------------------------
//Create and Write 1099T Record
bool CRFV_APElectronicFile1099Std::Write1099T()
{
	CString szBuffer;
	CString szTemp; //Used to convert numbers to strings

	m_nRecordSequenceNumber++;	//12/07/2007 CJB - 1-28229
	//Fill Buffer with data
	//Position = 1, Length = 1, Description = T, Value = "T"
	szBuffer.Append(_T("T"));
	//Position = 2-5, Length = 4, Description = Payment Year, Value = Use Payment Year Entered
	szBuffer.Append(PadRight(m_szPaymentYear, 4, _T(" ")));
	//Position = 6, Length = 1, Description = Prior Year Data Indicator, Value = Use Payment Year
	//RCG - 12/11/2020 - PBI 56232 - Added 1099-NEC to Electronic File for 2020
	//RCG - 11/04/2022 - PBI 60862 - Updated so any prior year contains the correct value
	int	nPaymentYear = _wtoi(m_szPaymentYear);
	szBuffer.Append(nPaymentYear >= CURRENT_PAYMENT_YEAR ? _T(" ") : _T("P"));
	//Position = 7-15, Length = 9, Description = Transmitter TIN, Value = From MagConstants EmployerID
	szTemp = si->MagneticMediaConstants.EmployerID;
	szTemp.Replace(_T("-"), _T(""));
	szBuffer.Append(PadRight(szTemp, 9, _T(" ")));
	//Position = 16-20, Length = 5, Description = Transmitter Control Code, Value = From MagConstants ControlCode
	szBuffer.Append(PadRight(si->MagneticMediaConstants.ControlCode, 5, _T(" ")));
	//01/10/2007 CJB - 1-25109  //Updated 1099 Magnetic Media for Tax Year 2006.
	//Position = 21-27, Length = 7, Description = Blank, Value = Enter 7 blanks
	szBuffer.Append(PadRight(_T(" "), 7, _T(" ")));
	//Position = 28, Length = 1, Description = Test File Indicator, Value = Use Test File Indicator Checkbox
	szBuffer.Append(m_bchkTestIndicator ? _T("T") : _T(" "));
	//Position = 29, Length = 1, Description = Foreign Entity Indicator, Value = based on missing checkbox?
	szBuffer.Append(_T(" "));
	//Position = 30-69, Length = 40, Description = Transmitter Name, Value = From MagConstants TranName (50 chars)
	//Position = 70-109, Length = 40, Description = Transmitter Name, Value = From MagConstants
	szBuffer.Append(PadRight(si->MagneticMediaConstants.Transmittername, 80, _T(" ")));
	//Position = 110-149, Length = 40, Description = Company Name, Value = From CompanyInfoSettings
	//Position = 150-189, Length = 40, Description = Company Name, Value = From CompanyInfoSettings
	szBuffer.Append(PadRight(si->CompanySettings.Name, 80, _T(" ")));
	//Position = 190-229, Length = 40, Description = Company Mailing Address, Value = From CompanyInfoSettings
	szBuffer.Append(PadRight(si->CompanySettings.BillingInformation.Address1 + " " + si->CompanySettings.BillingInformation.Address2, 40, _T(" ")));
	//Position = 230-269, Length = 40, Description = City, Value = From CompanyInfoSettings
	szBuffer.Append(PadRight(si->CompanySettings.BillingInformation.City, 40, _T(" ")));
	//Position = 270-271, Length = 2, Description = State, Value = From CompanyInfoSettings
	szBuffer.Append(PadRight(si->CompanySettings.BillingInformation.State, 2, _T(" ")));
	//Position = 272-280, Length = 9, Description = Zip, Value = 991561234
	szTemp = si->CompanySettings.BillingInformation.PostalCode;
	szTemp.Replace(_T("-"), _T(""));
	if (szTemp.GetLength() > 9) {
		szTemp = szTemp.Left(9);
		if (IDNO == CGBLForm::CMSMessageBox(this, IDS_AP_COMPANY_POSTAL_CODE_GREATER_THAN_NINE, IDS_AP_COMPANY_POSTAL_CODE_INVALID_CAP, MB_YESNO | MB_ICONQUESTION, si->CompanySettings.BillingInformation.PostalCode, szTemp)) {
			CGBLForm::CMSMessageBox(this, IDS_AP_INCORRECT_COMPANY_POSTAL_CODE, IDS_AP_COMPANY_POSTAL_CODE_INVALID_CAP, MB_OK | MB_ICONSTOP);
			m_bCancel = true;
			return false;
		}
	}
	if (!CMS::IsNumeric(szTemp)) {
		CString szAlphaChars = szTemp.SpanExcluding(CGBLResources::GetResourceString(IDS_GBL_NUMBERS));
		while (szAlphaChars.GetLength() > 0) {
			szTemp.Remove(szAlphaChars.GetAt(0));
			szAlphaChars.Remove(szAlphaChars.GetAt(0));
		}
		if (szTemp.GetLength() == 0) {
			szTemp = _T(" ");
		}
		if (IDNO == CGBLForm::CMSMessageBox(this, IDS_AP_COMPANY_POSTAL_CODE_INVALID, IDS_AP_COMPANY_POSTAL_CODE_INVALID_CAP, MB_YESNO | MB_ICONQUESTION, si->CompanySettings.BillingInformation.PostalCode, szTemp)) {
			CGBLForm::CMSMessageBox(this, IDS_AP_INCORRECT_COMPANY_POSTAL_CODE, IDS_AP_COMPANY_POSTAL_CODE_INVALID_CAP, MB_OK | MB_ICONSTOP);
			m_bCancel = true;
			return false;
		}
	}
	szBuffer.Append(PadRight(szTemp, 9, _T(" ")));
	//Position = 281-295, Length = 15, Description = Blank, Value = Enter 15 Blanks
	szBuffer.Append(PadRight(_T(" "), 15, _T(" ")));
	//Position = 296-303, Length = 8, Description = Total Number of Payees, Value = Count from return data
	szTemp.Format(_T("%d"), m_nNumberBMiscRecords + m_nNumberBNecRecords);
	szBuffer.Append(PadLeft(szTemp, 8, _T("0")));
	//Position = 304-343, Length = 40, Description = Contact Name, Value = From CompanyInfoSettings
	szTemp = si->CompanySettings.BillingInformation.Contact;
	if (szTemp == _T(""))
	{
		CGBLForm::CMSMessageBox(this, IDS_BLANK_COMPANY_CONTACT_MSG, IDS_BLANK_COMPANY_CONTACT,  MB_OK | MB_ICONSTOP);
		m_bCancel = true;
		return false;
	}
	szBuffer.Append(PadRight(si->CompanySettings.BillingInformation.Contact, 40, _T(" ")));
	//Position = 344-358, Length = 15, Description = Contact Phone Number & Ext, Value = From CompanyInfoSettings
	//Check Phone Number to make sure it's digits...If not, remove everything but digits, and give user chance to cancel.
	szTemp = si->CompanySettings.Phone;
	szTemp.Append(si->CompanySettings.Extension);
	if (!ReformatPhoneNumber(szTemp)) {
		if (IDNO == CGBLForm::CMSMessageBox(this, IDS_AP_COMPANY_PHONE_NUMBER_INCORRECTLY_FORMATTED, IDS_AP_COMPANY_PHONE_NUMBER_INVALID_CAP, MB_YESNO | MB_ICONQUESTION, si->CompanySettings.Phone + si->CompanySettings.Extension, szTemp)) {
			CGBLForm::CMSMessageBox(this, IDS_AP_INCORRECT_COMPANY_PHONE_NUMBER, IDS_AP_COMPANY_PHONE_NUMBER_INVALID_CAP, MB_OK | MB_ICONSTOP);
			m_bCancel = true;
			return false;
		}
	}
	szBuffer.Append(PadRight(szTemp, 15, _T(" ")));
	//01/10/2007 CJB - 1-25109  //Updated 1099 Magnetic Media for Tax Year 2006.
	//Position = 359-408, Length = 50, Description = Contact Email Address, Value = From MagConstants
	szBuffer.Append(PadRight(si->MagneticMediaConstants.Email, 50, _T(" ")));
	//Position = 409-499, Length = 91, Description = Blank, Value = Enter 91 Blanks
	szBuffer.Append(PadRight(_T(" "), 91, _T(" ")));
	//Position = 500-507, Length = 8, Description = Record Sequence Number, Value = 1
	//szBuffer.Append(PadLeft(_T("1"), 8, _T("0")));
	szTemp.Format(_T("%d"), m_nRecordSequenceNumber);	//12/06/2007 CJB - 1-28229
	szBuffer.Append(PadLeft(szTemp, 8, _T("0")));
	//Position = 508-517, Length = 10, Description = Blank, Value = Enter 10 Blanks
	szBuffer.Append(PadRight(_T(" "), 10, _T(" ")));
	//Position = 518, Length = 1, Description = Vendor Indicator, Value = Enter V
	szBuffer.Append(_T("V"));
	//Position = 519-558, Length = 40, Description = Vendor Name, Value = Cougar Mountain Software
	szBuffer.Append(PadRight(CGBLResources::GetResourceString(IDS_MAG_MEDIA_VENDOR_NAME), 40, _T(" ")));
	//Position = 559-598, Length = 40, Description = Vendor Mailing Address, Value = 7180 Potomac Dr.
	szBuffer.Append(PadRight(CGBLResources::GetResourceString(IDS_MAG_MEDIA_VENDOR_MAILING_ADDRESS), 40, _T(" ")));
	//Position = 599-638, Length = 40, Description = Vendor City, Value = Boise
	szBuffer.Append(PadRight(CGBLResources::GetResourceString(IDS_MAG_MEDIA_VENDOR_CITY), 40, _T(" ")));
	//Position = 639-640, Length = 2, Description = Vendor State, Value = ID
	szBuffer.Append(PadRight(CGBLResources::GetResourceString(IDS_MAG_MEDIA_VENDOR_STATE), 2, _T(" ")));
	//Position = 641-649, Length = 9, Description = Vendor Zip Code, Value = 83704
	szBuffer.Append(PadRight(CGBLResources::GetResourceString(IDS_MAG_MEDIA_VENDOR_POSTAL), 9, _T(" ")));
	//Position = 650-689, Length = 40, Description = Vendor Contact, Value = Dave Galligher
	szBuffer.Append(PadRight(CGBLResources::GetResourceString(IDS_MAG_MEDIA_VENDOR_CONTACT), 40, _T(" ")));
	//Position = 690-704, Length = 15, Description = Vendor Phone and Ext., Value = 2083754455
	szBuffer.Append(PadRight(CGBLResources::GetResourceString(IDS_MAG_MEDIA_VENDOR_PHONE), 15, _T(" ")));
	//12/26/2006 CJB - 1-25109  //Updated 1099 Magnetic Media for Tax Year 2006.
	//Position = 705-739, Length = 35, Description = Blank, Value = Enter 35 Blanks
	szBuffer.Append(PadRight(_T(" "), 35, _T(" ")));
	//Position = 740, Length = 1, Description = Vendor Foreign Entity Indicator, Value = Enter 1 Blank
	szBuffer.Append(_T(" "));
	//Position = 741-748, Length = 8, Description = Blank, Value = Enter 8 Blanks
	szBuffer.Append(PadRight(_T(" "), 8, _T(" ")));
	//Position = 749-750, Length = 2, Description = Blank, Value = CR/LF
	szBuffer.Append(_T("\r\n"));

	while (!FileWrite(szBuffer) && !m_bCancel) {
		//Do this until either successful, or canceled!
	}

	return !m_bCancel;
}
//------------------------------------------------------------------------------
//Create and Write 1099A Record
bool CRFV_APElectronicFile1099Std::Write1099A()
{
	CString szBuffer;
	CString szTemp; //Used to convert numbers to strings

	m_nRecordSequenceNumber++;	//12/06/2007 CJB - 1-28229
	//Fill Buffer with data
	//Position = 1, Length = 1, Field = Record Type, Value = A
	szBuffer.Append(_T("A"));
	//Position = 2-5, Length = 4, Field = Payment Year, Value = Use Payment Year Entered
	szBuffer.Append(PadRight(m_szPaymentYear, 4, _T(" ")));		
	//Position = 6, Length = 1, Field = Combined Fed./State Filer, Value = Use checkbox Combined Federal/State Filing Program
	szBuffer.Append(m_bchkCombinedFederalState ? _T("1") : _T(" "));	
	//Position = 7-11, Length = 5, Field = Blanks, Value = Enter 5 Blanks
	szBuffer.Append(PadRight(_T(" "), 5, _T(" ")));
	//Position = 12-20, Length = 9, Field = Payer TIN, Value = Use TIN Strip Hyphen
	szTemp = m_szPayerTIN;
	szTemp.Replace(_T("-"), _T(""));
	szBuffer.Append(PadRight(szTemp, 9, _T(" ")));
	//Position = 21-24, Length = 4, Field = Payer Name Control, Value = Use Payer Name Control Entered
	szBuffer.Append(PadRight(m_szPayerNameCtrl, 4, _T(" ")));
	//Position = 25, Length = 1, Field = Last Filing Indicator, Value = Use checkbox Last Filing Indicator
	szBuffer.Append(m_bchkLastFilingIndicator ? _T("1") : _T(" "));
	//RCG - 12/17/2020 - PBI 56232 - Add 1099-NEC for 2020	m_szMISC
	int	nPayYear = _wtoi(m_szPaymentYear);
	if (m_b1099NEC && nPayYear >= PAYMENT_YEAR_2020)
	{
		//Position = 26-27, Length = 2, Field = Type of Return, Value = NE for NEC
		szBuffer.Append(_T("NE"));
	}
	else
	{
		//Position = 26-27, Length = 2, Field = Type of Return, Value = A for MISC
		szBuffer.Append(_T("A "));
	}
	//RCG - 12/17/2020 - PBI 56232 - Add 1099-NEC for 2020
	if (nPayYear >= PAYMENT_YEAR_2020)
	{
		//RCG - 01/13/2021 - Bug 57012 - Do not include Amount Code G for 1099-MISC for Payment Year 2020 and beyond
		if (m_b1099NEC)
		{
			//Position = 28-45, Length = 18, Field = Amount Codes, Value = 14
			szBuffer.Append(PadRight(_T("14"), 18, _T(" ")));
		}
		else
		{
			//RS - 11/17/2021 - PBI 59039 - Added new 1099 Type to Electronic File for 2021
			//RS - 09/30/2025 - PBI 65630 - Do not include Amount Code "B"
			if (nPayYear >= PAYMENT_YEAR_2021)
			{
				//Position = 28-45, Length = 18, Field = Amount Codes, Value = 1234568ACDEF
				szBuffer.Append(PadRight(_T("1234568ACDEF"), 18, _T(" ")));
			}
			else
			{
				//Position = 28-45, Length = 18, Field = Amount Codes, Value = 1234568ACDE
				szBuffer.Append(PadRight(_T("1234568ACDE"), 18, _T(" ")));
			}			
		}
	}
	else
	{
		//Position = 28-45, Length = 18, Field = Amount Codes, Value = 1234568ACDEFG
		szBuffer.Append(PadRight(_T("1234568ACDEFG"), 18, _T(" ")));
	}
	//12/05/2007 CJB - 1-28706  //Updated 1099 Magnetic Media for Tax Year 2007.
	//Position = 46-51, Length = 6, Field = Blanks, Value = Enter 6 Blanks
	szBuffer.Append(PadRight(_T(" "), 6, _T(" ")));
	//Position = 52, Length = 1, Field = Foreign Entity Indicator, Value = Enter 1 blank
	szBuffer.Append(_T(" "));
	//Position = 53-92, Length = 40, Field = First Payer Name Line, Value = From CompanyInfoSettings
	//Position = 93-132, Length = 40, Field = Second Payer Name Line, Value = From CompanyInfoSettings
	szBuffer.Append(PadRight(si->CompanySettings.Name, 80, _T(" ")));
	//Position = 133, Length = 1, Field = Transfer Agent Indicator, Value = 0
	szBuffer.Append(_T("0"));
	//Position = 134-173, Length = 40, Field = Payer Address, Value = From CompanyInfoSettings
	szBuffer.Append(PadRight(si->CompanySettings.BillingInformation.Address1 + " " + si->CompanySettings.BillingInformation.Address2, 40, _T(" ")));
	//Position = 174-213, Length = 40, Field = Payer City, Value = From CompanyInfoSettings
	szBuffer.Append(PadRight(si->CompanySettings.BillingInformation.City, 40, _T(" ")));
	//Position = 214-215, Length = 2, Field = Payer State, Value = From CompanyInfoSettings
	szBuffer.Append(PadRight(si->CompanySettings.BillingInformation.State, 2, _T(" ")));
	//Position = 216-224, Length = 9, Field = Payer Zip Code, Value = From CompanyInfoSettings
	szTemp = si->CompanySettings.BillingInformation.PostalCode;
	szTemp.Replace(_T("-"), _T(""));
	CString szAlphaChars = szTemp.SpanExcluding(CGBLResources::GetResourceString(IDS_GBL_NUMBERS));
	while (szAlphaChars.GetLength() > 0) {
		szTemp.Remove(szAlphaChars.GetAt(0));
		szAlphaChars.Remove(szAlphaChars.GetAt(0));
	}
	if (szTemp.GetLength() == 0) {
		szTemp = _T(" ");
	}
	szBuffer.Append(PadRight(szTemp, 9, _T(" ")));
	//Position = 225-239, Length = 15, Field = Payer Phone Number and Ext, Value = From CompanyInfoSettings
	//Check Phone Number to make sure it's digits...If not, remove everything but digits, and user already had chance to cancel in T Record.
	szTemp = si->CompanySettings.Phone;
	szTemp.Append(si->CompanySettings.Extension);
	ReformatPhoneNumber(szTemp);
	szBuffer.Append(PadRight(szTemp, 15, _T(" ")));
	//Position = 240-499, Length = 260, Field = Blanks, Value = Enter 260 Blanks
	szBuffer.Append(PadRight(_T(" "), 260, _T(" ")));
	//Position = 500-507, Length = 8, Field = Record Sequence Number, Value = 00000002
	//szBuffer.Append(PadLeft(_T("2"), 8, _T("0")));
	szTemp.Format(_T("%d"), m_nRecordSequenceNumber);	//12/06/2007 CJB - 1-28229
	szBuffer.Append(PadLeft(szTemp, 8, _T("0")));
	//Position = 508-748, Length = 241, Field = Blanks, Value = Enter 241 Blanks
	szBuffer.Append(PadRight(_T(" "), 241, _T(" ")));
	//Position = 749-750, Length = 2, Field = Blanks, Value = CR/LF
	szBuffer.Append(_T("\r\n"));

	while (!FileWrite(szBuffer) && !m_bCancel) {
		//Do this until either successful, or canceled!
	}

	//RS - 11/21/2024 - PBI 64417 - Added
	m_nNumberARecords++;

	return !m_bCancel;
}
//------------------------------------------------------------------------------
//RS - 11/25/2020 - PBI 56232 - Create and Write B Records for 1099-MISC
bool CRFV_APElectronicFile1099Std::Write1099B_MISC()
{
	CString szBuffer;
	CString szTemp; //Used to convert numbers to strings
	CString szState;
	CString szFedStateCode;
	int nFedStCode;
	double dType1Amount = 0;	//Local Amount for Type 1
	double dType2Amount = 0;	//Local Amount for Type 2
	double dType3Amount = 0;	//Local Amount for Type 3
	double dType4Amount = 0;	//Local Amount for Type 4
	double dType5Amount = 0;	//Local Amount for Type 5
	double dType6Amount = 0;	//Local Amount for Type 6
	double dType8Amount = 0;	//Local Amount for Type 8
	double dType9Amount = 0;	//Local Amount for Type 9 currently always 0.
	double dTypeAAmount = 0;	//Local Amount for Type A
	double dTypeBAmount = 0;	//Local Amount for Type B
	double dTypeCAmount = 0;	//Local Amount for Type C
	double dTypeDAmount = 0;	//Local Amount for Type D
	double dTypeEAmount = 0;	//Local Amount for Type E
	double dTypeFAmount = 0;	//Local Amount for Type F
	double dTypeGAmount = 0;	//Local Amount for Type G
	double dTypeHAmount = 0;	//Local Amount for Type H
	double dTypeJAmount = 0;	//Local Amount for Type J

	CDataRow clsDR;

	// 1-33838 BK 12/02/09 decrease the rowcount by 1 if there will be more server calls because the last row will be the first row of the next call
	long lRowCount = m_dtBMiscRecords.getRowCount();
	if (m_nServerStatus == SERVER_CONTINUE)
		lRowCount--;

	//Cycle through each B Record row
	for (int nRow = 0; nRow < lRowCount; nRow++)
	{
		//szRecordNumber.Format(_T("%d"), m_NumberBRecords + 3);	//12/06/2007 CJB - 1-28229	//Always add 3, the first B Record is the 3rd record total (T = 1, A = 2)
		m_dtBMiscRecords.GetRow(nRow, &clsDR);
		m_nRecordSequenceNumber++;	//12/06/2007 CJB - 1-28229
		//Initialize Buffer
		szBuffer = _T("");
		//12/06/2007 CJB - 1-28229  //Separate K Records for each State
		szState = _T("");
		szFedStateCode = _T("");
		nFedStCode = 0;
		szState = clsDR.XMLStr(_T("strProvince"), false);
		szFedStateCode = GetCombCode(szState);
		if (szFedStateCode.Trim() != _T("")) {
			nFedStCode = _tstoi(szFedStateCode);
			m_ElectronicFileKrecordTotals[nFedStCode].bUsed = true;
			m_ElectronicFileKrecordTotals[nFedStCode].szState = szState;
			m_ElectronicFileKrecordTotals[nFedStCode].szFedStateCode = szFedStateCode;
			m_ElectronicFileKrecordTotals[nFedStCode].nNumberStateBRecords++;
		}
		dType1Amount = clsDR.XMLDbl(_T("curAP1099PMT1"), false);
		dType2Amount = clsDR.XMLDbl(_T("curAP1099PMT2"), false);
		dType3Amount = clsDR.XMLDbl(_T("curAP1099PMT3"), false);
		dType4Amount = clsDR.XMLDbl(_T("curAP1099PMT4"), false);
		dType5Amount = clsDR.XMLDbl(_T("curAP1099PMT5"), false);
		dType6Amount = clsDR.XMLDbl(_T("curAP1099PMT6"), false);
		dType8Amount = clsDR.XMLDbl(_T("curAP1099PMT8"), false);
		dType9Amount = 0;
		dTypeAAmount = clsDR.XMLDbl(_T("curAP1099PMT9"), false);
		dTypeBAmount = clsDR.XMLDbl(_T("curAP1099PMT10"), false);
		dTypeCAmount = clsDR.XMLDbl(_T("curAP1099PMT11"), false);
		dTypeDAmount = clsDR.XMLDbl(_T("curAP1099PMT12"), false);
		dTypeEAmount = clsDR.XMLDbl(_T("curAP1099PMT13"), false);
		dTypeFAmount = clsDR.XMLDbl(_T("curAP1099PMT14"), false);
		dTypeGAmount = clsDR.XMLDbl(_T("curAP1099PMT7"), false);
		dTypeHAmount = 0;
		dTypeJAmount = 0;
		//Fill Buffer with data
		//Position = 1, Length = 1, Field = Record Type, Value = B
		szBuffer.Append(_T("B"));
		//Position = 2-5, Length = 4, Field = Payment Year, Value = Use Payment Year Entered
		szBuffer.Append(PadRight(m_szPaymentYear, 4, _T(" ")));
		//Position = 6, Length = 1, Field = Corrected Return Indicator, Value = Enter 1 Blank
		szBuffer.Append(_T(" "));
		//Position = 7-10, Length = 4, Field = Name Control, Value = First 4 Characters of MasterTable Name
		//RCG - 12/15/2014 - Bug 25757 - Remove blanks
		//szBuffer.Append(PadRight(clsDR.XMLStr(_T("strVendorName"), false), 4, _T(" ")));
		szTemp = clsDR.XMLStr(_T("strVendorName"), false);
		szTemp.Replace(_T(" "), _T(""));
		szBuffer.Append(PadRight(szTemp, 4, _T(" ")));
		//Position = 11, Length = 1, Field = Type of TIN, Value = Use 1099 Type (Business or Individual)
		szBuffer.Append(clsDR.XMLBOOL(_T("bol1099Flag"), false) ? _T("2") : _T("1"));  //if true then value is 1, an individual (so send a 2) //0726/2006 CJB - 1-25184
		//Position = 12-20, Length = 9, Field = Payee's TIN, Value = MasterTable Fed. ID
		szTemp = clsDR.XMLStr(_T("strFederalIDNumber"), false);
		szTemp.Replace(_T("-"), _T(""));
		szBuffer.Append(PadRight(szTemp, 9, _T(" ")));
		//Position = 21-40, Length = 20, Field = Payers account Number, Value = MasterTable Vendor Number
		szBuffer.Append(PadRight(clsDR.XMLStr(_T("strVendor"), false), 20, _T(" ")));
		//Position = 41-44, Length = 4, Field = Payers Office Code, Value = Enter 4 Blanks
		szBuffer.Append(PadRight(_T(" "), 4, _T(" ")));
		//Position = 45-54, Length = 10, Field = Blank, Value = Enter 10 Blanks
		szBuffer.Append(PadRight(_T(" "), 10, _T(" ")));
		//Position = 55-66, Length = 12, Field = Payment Amount 1, Value = MasterTable Data type 0
		szBuffer.Append(FormatNumber(dType1Amount, 12));
		//add total to Master Total
		m_dMiscType1Total += dType1Amount;
		if (szFedStateCode.Trim() != _T("")) {
			m_ElectronicFileKrecordTotals[nFedStCode].dType1StateTotal += dType1Amount;
		}
		//Position = 67-78, Length = 12, Field = Payment Amount 2, Value = MasterTable Data type 1
		szBuffer.Append(FormatNumber(dType2Amount, 12));
		//add total to Master Total
		m_dMiscType2Total += dType2Amount;
		if (szFedStateCode.Trim() != _T("")) {
			m_ElectronicFileKrecordTotals[nFedStCode].dType2StateTotal += dType2Amount;
		}
		//Position = 79-90, Length = 12, Field = Payment Amount 3, Value = MasterTable Data type 2
		szBuffer.Append(FormatNumber(dType3Amount, 12));
		//add total to Master Total
		m_dMiscType3Total += dType3Amount;
		if (szFedStateCode.Trim() != _T("")) {
			m_ElectronicFileKrecordTotals[nFedStCode].dType3StateTotal += dType3Amount;
		}
		//Position = 91-102, Length = 12, Field = Payment Amount 4, Value = MasterTable Data type 3
		szBuffer.Append(FormatNumber(dType4Amount, 12));
		//add total to Master Total
		m_dMiscType4Total += dType4Amount;
		if (szFedStateCode.Trim() != _T("")) {
			m_ElectronicFileKrecordTotals[nFedStCode].dType4StateTotal += dType4Amount;
		}
		//Position = 103-114, Length = 12, Field = Payment Amount 5, Value = MasterTable Data type 4
		szBuffer.Append(FormatNumber(dType5Amount, 12));
		//add total to Master Total
		m_dMiscType5Total += dType5Amount;
		if (szFedStateCode.Trim() != _T("")) {
			m_ElectronicFileKrecordTotals[nFedStCode].dType5StateTotal += dType5Amount;
		}
		//Position = 115-126, Length = 12, Field = Payment Amount 6, Value = MasterTable Data type 5
		szBuffer.Append(FormatNumber(dType6Amount, 12));
		//add total to Master Total
		m_dMiscType6Total += dType6Amount;
		if (szFedStateCode.Trim() != _T("")) {
			m_ElectronicFileKrecordTotals[nFedStCode].dType6StateTotal += dType6Amount;
		}
		//Position = 127-138, Length = 12, Field = Payment Amount 7, Value = Enter 12 zeros
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));
		//Position = 139-150, Length = 12, Field = Payment Amount 8, Value = MasterTable Data type 7
		szBuffer.Append(FormatNumber(dType8Amount, 12));
		//add total to Master Total
		m_dMiscType8Total += dType8Amount;
		if (szFedStateCode.Trim() != _T("")) {
			m_ElectronicFileKrecordTotals[nFedStCode].dType8StateTotal += dType8Amount;
		}
		//Position = 151-162, Length = 12, Field = Payment Amount 9, Value = MasterTable Data type 8
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));
		//Position = 163-174, Length = 12, Field = Payment Amount A, Value = MasterTable Data type 9
		szBuffer.Append(FormatNumber(dTypeAAmount, 12));
		//add total to Master Total
		m_dMiscTypeATotal += dTypeAAmount;
		if (szFedStateCode.Trim() != _T("")) {
			m_ElectronicFileKrecordTotals[nFedStCode].dTypeAStateTotal += dTypeAAmount;
		}
		//Position = 175-186, Length = 12, Field = Payment Amount B, Value = Enter 12 zeros
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));		//RCG - 11/17/2025 - PBI 65630 - Removed Payment Amount B
		//Position = 187-198, Length = 12, Field = Payment Amount C, Value = MasterTable Data type 11
		szBuffer.Append(FormatNumber(dTypeCAmount, 12));
		//add total to Master Total
		m_dMiscTypeCTotal += dTypeCAmount;
		if (szFedStateCode.Trim() != _T("")) {
			m_ElectronicFileKrecordTotals[nFedStCode].dTypeCStateTotal += dTypeCAmount;
		}
		//Position = 199-210, Length = 12, Field = Payment Amount D, Value = MasterTable Data type 12
		szBuffer.Append(FormatNumber(dTypeDAmount, 12));
		//add total to Master Total
		m_dMiscTypeDTotal += dTypeDAmount;
		if (szFedStateCode.Trim() != _T("")) {
			m_ElectronicFileKrecordTotals[nFedStCode].dTypeDStateTotal += dTypeDAmount;
		}
		//Position = 211-222, Length = 12, Field = Payment Amount E, Value = MasterTable Data type 13
		szBuffer.Append(FormatNumber(dTypeEAmount, 12));
		//add total to Master Total
		m_dMiscTypeETotal += dTypeEAmount;
		if (szFedStateCode.Trim() != _T("")) {
			m_ElectronicFileKrecordTotals[nFedStCode].dTypeEStateTotal += dTypeEAmount;
		}
		//DRR 12/01/09 1-33838
		//RS - 11/17/2021 - PBI 59039 - Added new 1099 Type to Electronic File for 2021
		//Position = 223-234, Length = 12, Field = Payment Amount F, Value = Enter 12 zeros
		szBuffer.Append(FormatNumber(dTypeFAmount, 12));
		//add total to Master Total
		m_dMiscTypeFTotal += dTypeFAmount;
		if (szFedStateCode.Trim() != _T(""))
		{
			m_ElectronicFileKrecordTotals[nFedStCode].dTypeFStateTotal += dTypeFAmount;
		}
		//Position = 235-246, Length = 12, Field = Payment Amount G, Value = MasterTable Data type 6
		szBuffer.Append(FormatNumber(dTypeGAmount, 12));
		//Position = 247-258, Length = 12, Field = Payment Amount H, Value = MasterTable Data type H
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));		
		//Position = 259-270, Length = 12, Field = Payment Amount J, Value = MasterTable Data type J
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));		
		//Position = 271-286, Length = 16, Field = Blank, Value = Enter 16 Blanks
		szBuffer.Append(PadRight(_T(" "), 16, _T(" ")));
		//add total to Master Total
		m_dMiscTypeGTotal += dTypeGAmount;
		if (szFedStateCode.Trim() != _T("")) {
			m_ElectronicFileKrecordTotals[nFedStCode].dTypeGStateTotal += dTypeGAmount;
		}
		//Position = 287, Length = 1, Field = Foreign Country Indicator, Value = Enter 1 Blank
		szBuffer.Append(_T(" "));
		//Position = 288-327, Length = 40, Field = First Payee Name, Value = MasterTable Name
		//Position = 328-367, Length = 40, Field = Second Payee Name, Value = MasterTable Name
		szBuffer.Append(PadRight(clsDR.XMLStr(_T("strVendorName"), false), 80, _T(" ")));	
		//Position = 368-407, Length = 40, Field = Payee Mailing Address, Value = MasterTable Mailing Address
		szBuffer.Append(PadRight(clsDR.XMLStr(_T("strAddress1"), false) + clsDR.XMLStr(_T("strAddress2"), false), 40, _T(" ")));
		//Position = 408-447, Length = 40, Field = Blank, Value = Enter 40 Blanks
		szBuffer.Append(PadRight(_T(" "), 40, _T(" ")));
		//Position = 448-487, Length = 40, Field = Payee City, Value = MasterTable Mailing City
		szBuffer.Append(PadRight(clsDR.XMLStr(_T("strCity"), false), 40, _T(" ")));
		//Position = 488-489, Length = 2, Field = Payee State, Value = MasterTable Mailing State
		//12/06/2007 CJB - 1-28229  //State from Vendor Master Table
		//szBuffer.Append(PadRight(clsDR.XMLStr(_T("strProvince"), false), 2, _T(" ")));
		szBuffer.Append(PadRight(szState, 2, _T(" ")));
		//Position = 490-498, Length = 9, Field = Payee Zip, Value = MasterTable Mailing Zip
		szBuffer.Append(PadRight(clsDR.XMLStr(_T("strPostalCode"), false), 9, _T(" ")));
		//Position = 499, Length = 1, Field = Blank, Value = Enter 1 Blank
		szBuffer.Append(_T(" "));
		//Position = 500-507, Length = 8, Field = Record Sequence Number, Value = Incremented as records are created
		//szBuffer.Append(PadLeft(szRecordNumber, 8, _T("0")));
		szTemp.Format(_T("%d"), m_nRecordSequenceNumber);	//12/06/2007 CJB - 1-28229
		szBuffer.Append(PadLeft(szTemp, 8, _T("0")));
		//Position = 508-543, Length = 36, Field = Blank, Value = Enter 36 Blanks
		szBuffer.Append(PadRight(_T(" "), 36, _T(" ")));
		//Position = 544, Length = 1, Field = Second TIN, Value = MasterTable 2nd TIN
		szBuffer.Append(clsDR.XMLBOOL(_T("bol2ndTIN"), false) ? _T("2") : _T(" "));
		//Position = 545-546, Length = 2, Field = Blank, Value = Enter 2 Blanks
		szBuffer.Append(PadRight(_T(" "), 2, _T(" ")));
		//Position = 547, Length = 1, Field = Direct Sales Indicator, Value = MasterTable bol5000DirSales
		szBuffer.Append(clsDR.XMLBOOL(_T("bol5000DirSales"), false) ? _T("1") : _T(" "));
		//	PBI 30669	12/1/2015	mvt - Added FATCA Filing field
		//	Position = 548, Length - 1, Field = FATCA Filing Requirement, Value = MasterTable.bolFATCAFiling
		szBuffer.Append(clsDR.XMLBOOL(_T("bolFATCAFiling"), false) ? _T("1") : _T(" "));
		//Position = 549-662, Length = 114, Field = Blank, Value = Enter 114 Blanks
		szBuffer.Append(PadRight(_T(" "), 114, _T(" ")));
		//Position = 663-722, Length = 60, Field = Special Data Entries, Value = Enter 60 Blanks
		szBuffer.Append(PadRight(_T(" "), 60, _T(" ")));
		//Position = 723-734, Length = 12, Field = State Income Tax Withheld, Value = Enter 12 Blanks
		szBuffer.Append(PadRight(_T(" "), 12, _T(" ")));
		//Position = 735-746, Length = 12, Field = Local Income Tax Withheld, Value = Enter 12 Blanks
		szBuffer.Append(PadRight(_T(" "), 12, _T(" ")));
		//Position = 747-748, Length = 2, Field = Combined Federal/State Code, Value = Use Select Case in 16 bit
		//12/06/2007 CJB - 1-28229  //State from Vendor Master Table
		//szBuffer.Append(GetCombCode());
		// DRR 12/14/09
		if (m_bchkCombinedFederalState) {
			szBuffer.Append(PadRight(szFedStateCode, 2, _T(" ")));
		}
		else {
			szBuffer.Append(PadRight(_T(" "), 2, _T(" ")));
		}
		//Position = 749-750, Length = 2, Field = Blanks, Value = CR/LF
		szBuffer.Append(_T("\r\n"));

		while (!FileWrite(szBuffer) && !m_bCancel) {
			//Do this until either successful, or canceled!
		}
		if (m_bCancel) {
			//We canceled!
			return false;
		}
		//m_NumberBRecords++;	//12/06/2007 CJB - 1-28229
	}

	return true;
}
//------------------------------------------------------------------------------
//RS - 11/25/2020 - PBI 56232 - Create and Write C Record for 1099-MISC
bool CRFV_APElectronicFile1099Std::Write1099C_MISC()
{
	CString szBuffer;
	CString szTemp; //Used to convert numbers to strings

	m_nRecordSequenceNumber++;	//12/06/2007 CJB - 1-28229
	//Fill Buffer with data
	//Position = 1, Length = 1, Field = Record Type, Value = C
	szBuffer.Append(_T("C"));
	//Position = 2-9, Length = 8, Field = Number of Payees, Value = From Tracked Amount
	szTemp.Format(_T("%d"), m_nNumberBMiscRecords);
	szBuffer.Append(PadLeft(szTemp, 8, _T("0")));
	//Position = 10-15, Length = 6, Field = Blank, Value = Enter 6 Blanks
	szBuffer.Append(PadRight(_T(" "), 6, _T(" ")));
	//Position = 16-33, Length = 18, Field = B Record Totals of Type 1, Value = From Tracked Amount
	szBuffer.Append(FormatNumber(m_dMiscType1Total, 18));
	//Position = 34-51, Length = 18, Field = B Record Totals of Type 2, Value = From Tracked Amount
	szBuffer.Append(FormatNumber(m_dMiscType2Total, 18));
	//Position = 52-69, Length = 18, Field = B Record Totals of Type 3, Value = From Tracked Amount
	szBuffer.Append(FormatNumber(m_dMiscType3Total, 18));
	//Position = 70-87, Length = 18, Field = B Record Totals of Type 4, Value = From Tracked Amount
	szBuffer.Append(FormatNumber(m_dMiscType4Total, 18));
	//Position = 88-105, Length = 18, Field = B Record Totals of Type 5, Value = From Tracked Amount
	szBuffer.Append(FormatNumber(m_dMiscType5Total, 18));
	//Position = 106-123, Length = 18, Field = B Record Totals of Type 6, Value = From Tracked Amount
	szBuffer.Append(FormatNumber(m_dMiscType6Total, 18));
	//Position = 124-141, Length = 18, Field = B Record Totals of Type 7, Value = Enter 18 zeros
	szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
	//Position = 142-159, Length = 18, Field = B Record Totals of Type 8, Value = From Tracked Amount
	szBuffer.Append(FormatNumber(m_dMiscType8Total, 18));
	//Position = 160-177, Length = 18, Field = B Record Totals of Type 9, Value = From Tracked Amount
	szBuffer.Append(FormatNumber(m_dMiscType9Total, 18));
	//Position = 178-195, Length = 18, Field = B Record Totals of Type A, Value = From Tracked Amount
	szBuffer.Append(FormatNumber(m_dMiscTypeATotal, 18));
	//Position = 196-213, Length = 18, Field = B Record Totals of Type B, Value = From Tracked Amount
	szBuffer.Append(FormatNumber(m_dMiscTypeBTotal, 18));
	//Position = 214-231, Length = 18, Field = B Record Totals of Type C, Value = From Tracked Amount
	szBuffer.Append(FormatNumber(m_dMiscTypeCTotal, 18));
	//Position = 232-249, Length = 18, Field = B Record Totals of Type D, Value = From Tracked Amount
	szBuffer.Append(FormatNumber(m_dMiscTypeDTotal, 18));
	//Position = 250-267, Length = 18, Field = B Record Totals of Type E, Value = From Tracked Amount
	szBuffer.Append(FormatNumber(m_dMiscTypeETotal, 18));
	//DRR 12/01/09 1-33838
	//RS - 11/17/2021 - PBI 59039 - Added new 1099 Type to Electronic File for 2021
	//Position = 268-285, Length = 18, Field = B Record Totals of Type F, Value = From Tracked Amount
	szBuffer.Append(FormatNumber(m_dMiscTypeFTotal, 18));
	//Position = 286-303, Length = 18, Field = B Record Totals of Type G, Value = From Tracked Amount
	szBuffer.Append(FormatNumber(m_dMiscTypeGTotal, 18));
	//Position = 304-321, Length = 18, Field = B Record Totals of Type H, Value = Enter 18 zeros	
	szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
	//Position = 322-339, Length = 18, Field = B Record Totals of Type J, Value = Enter 18 zeros	
	szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
	//Position = 340-499, Length = 160, Field = Blank, Value = Enter 160 Blanks
	szBuffer.Append(PadRight(_T(" "), 160, _T(" ")));
	//Position = 500-507, Length = 8, Field = Record Sequence Number, Value = Incremented as records are created
	//szTemp.Format(_T("%d"), m_NumberBRecords + 3);
	szTemp.Format(_T("%d"), m_nRecordSequenceNumber);	//12/06/2007 CJB - 1-28229
	szBuffer.Append(PadLeft(szTemp, 8, _T("0")));
	//Position = 508-748, Length = 241, Field = Blank, Value = Enter 241 Blanks
	szBuffer.Append(PadRight(_T(" "), 241, _T(" ")));
	//Position = 749-750, Length = 2, Field = Blank, Value = CR/LF
	szBuffer.Append(_T("\r\n"));

	while (!FileWrite(szBuffer) && !m_bCancel) {
		//Do this until either successful, or canceled!
	}

	return !m_bCancel;
}
//------------------------------------------------------------------------------
//Create and Write B Records for 1099-NEC
bool CRFV_APElectronicFile1099Std::Write1099B_NEC()
{
	CString szBuffer;
	CString szTemp; //Used to convert numbers to strings
	double dType1Amount = 0;	//Local Amount for Type 1
	double dType4Amount = 0;	//Local Amount for Type 4
	CDataRow clsDR;
	//RS - 11/17/2021 - PBI 59039 - Added new 1099 Type to Electronic File for 2021
	CString szState;
	CString szFedStateCode;
	int nFedStCode;

	//Decrease the rowcount by 1 if there will be more server calls because the last row will be the first row of the next call
	long lRowCount = m_dtBNecRecords.getRowCount();
	if (m_nServerStatus == SERVER_CONTINUE)
		lRowCount--;

	//Cycle through each B Record row
	for (int nRow = 0; nRow < lRowCount; nRow++)
	{
		m_dtBNecRecords.GetRow(nRow, &clsDR);
		m_nRecordSequenceNumber++;
		//Initialize Buffer
		szBuffer = _T("");
		//RS - 11/17/2021 - PBI 59039 - Added new 1099 Type to Electronic File for 2021
		szState = _T("");
		szFedStateCode = _T("");
		nFedStCode = 0;
		szState = clsDR.XMLStr(_T("strProvince"), false);
		szFedStateCode = GetCombCode(szState);
		if (szFedStateCode.Trim() != _T(""))
		{
			nFedStCode = _tstoi(szFedStateCode);
			m_ElectronicFileKrecordTotals[nFedStCode].bUsed = true;
			m_ElectronicFileKrecordTotals[nFedStCode].szState = szState;
			m_ElectronicFileKrecordTotals[nFedStCode].szFedStateCode = szFedStateCode;
			m_ElectronicFileKrecordTotals[nFedStCode].nNumberStateBRecords++;
		}
		dType1Amount = clsDR.XMLDbl(_T("curAP1099PMT1"), false);
		dType4Amount = clsDR.XMLDbl(_T("curAP1099PMT4"), false);
		//Fill Buffer with data
		//Position = 1, Length = 1, Field = Record Type, Value = B
		szBuffer.Append(_T("B"));
		//Position = 2-5, Length = 4, Field = Payment Year, Value = Use Payment Year Entered
		szBuffer.Append(PadRight(m_szPaymentYear, 4, _T(" ")));
		//Position = 6, Length = 1, Field = Corrected Return Indicator, Value = Enter 1 Blank
		szBuffer.Append(_T(" "));
		//Position = 7-10, Length = 4, Field = Name Control, Value = First 4 Characters of MasterTable Name
		//Remove blanks
		szTemp = clsDR.XMLStr(_T("strVendorName"), false);
		szTemp.Replace(_T(" "), _T(""));
		szBuffer.Append(PadRight(szTemp, 4, _T(" ")));
		//Position = 11, Length = 1, Field = Type of TIN, Value = Use 1099 Type (Business or Individual)
		szBuffer.Append(clsDR.XMLBOOL(_T("bol1099Flag"), false) ? _T("2") : _T("1"));  //if true then value is 1, an individual (so send a 2) //0726/2006 CJB - 1-25184
		//Position = 12-20, Length = 9, Field = Payee's TIN, Value = MasterTable Fed. ID
		szTemp = clsDR.XMLStr(_T("strFederalIDNumber"), false);
		szTemp.Replace(_T("-"), _T(""));
		szBuffer.Append(PadRight(szTemp, 9, _T(" ")));
		//Position = 21-40, Length = 20, Field = Payers account Number, Value = MasterTable Vendor Number
		szBuffer.Append(PadRight(clsDR.XMLStr(_T("strVendor"), false), 20, _T(" ")));
		//Position = 41-44, Length = 4, Field = Payers Office Code, Value = Enter 4 Blanks
		szBuffer.Append(PadRight(_T(" "), 4, _T(" ")));
		//Position = 45-54, Length = 10, Field = Blank, Value = Enter 10 Blanks
		szBuffer.Append(PadRight(_T(" "), 10, _T(" ")));
		//Position = 55-66, Length = 12, Field = Payment Amount 1, Value = MasterTable Data type 0
		szBuffer.Append(FormatNumber(dType1Amount, 12));
		//Add total to Master Total
		m_dNecType1Total += dType1Amount;
		//RS 1/17/22 - PBI 59406 - Added missing Type 1
		if (szFedStateCode.Trim() != _T(""))
		{
			m_ElectronicFileKrecordTotals[nFedStCode].dType1StateTotal += dType1Amount;
		}
		//Position = 67-78, Length = 12, Field = Payment Amount 2, Value = Enter 12 zeros
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));
		//Position = 79-90, Length = 12, Field = Payment Amount 3, Value = Enter 12 zeros
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));
		//Position = 91-102, Length = 12, Field = Payment Amount 4, Value = MasterTable Data type 3
		szBuffer.Append(FormatNumber(dType4Amount, 12));
		//Add total to Master Total
		m_dNecType4Total += dType4Amount;
		//RS 1/17/22 - PBI 59406 - Added missing Type 4
		if (szFedStateCode.Trim() != _T(""))
		{
			m_ElectronicFileKrecordTotals[nFedStCode].dType4StateTotal += dType4Amount;
		}
		//Position = 103-114, Length = 12, Field = Payment Amount 5, Value = Enter 12 zeros
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));
		//Position = 115-126, Length = 12, Field = Payment Amount 6, Value = Enter 12 zeros
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));
		//Position = 127-138, Length = 12, Field = Payment Amount 7, Value = Enter 12 zeros
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));
		//Position = 139-150, Length = 12, Field = Payment Amount 8, Value = Enter 12 zeros
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));
		//Position = 151-162, Length = 12, Field = Payment Amount 9, Value = Enter 12 zeros
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));
		//Position = 163-174, Length = 12, Field = Payment Amount A, Value = Enter 12 zeros
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));
		//Position = 175-186, Length = 12, Field = Payment Amount B, Value = Enter 12 zeros
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));
		//Position = 187-198, Length = 12, Field = Payment Amount C, Value = Enter 12 zeros
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));
		//Position = 199-210, Length = 12, Field = Payment Amount D, Value = Enter 12 zeros
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));
		//Position = 211-222, Length = 12, Field = Payment Amount E, Value = Enter 12 zeros
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));
		//Position = 223-234, Length = 12, Field = Payment Amount F, Value = Enter 12 zeros
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));
		//Position = 234-246, Length = 12, Field = Payment Amount G, Value = Enter 12 zeros
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));
		//Position = 247-258, Length = 12, Field = Payment Amount H, Value = Enter 12 zeros
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));
		//Position = 259-270, Length = 12, Field = Payment Amount J, Value = Enter 12 zeros
		szBuffer.Append(PadLeft(_T("0"), 12, _T("0")));
		//Position = 271-286, Length = 16, Field = Blank, Value = Enter 16 Blanks
		szBuffer.Append(PadRight(_T(" "), 16, _T(" ")));
		//Position = 287, Length = 1, Field = Foreign Country Indicator, Value = Enter 1 Blank
		szBuffer.Append(_T(" "));
		//Position = 288-327, Length = 40, Field = First Payee Name, Value = MasterTable Name
		//Position = 328-367, Length = 40, Field = Second Payee Name, Value = MasterTable Name
		szBuffer.Append(PadRight(clsDR.XMLStr(_T("strVendorName"), false), 80, _T(" ")));		
		//Position = 368-407, Length = 40, Field = Payee Mailing Address, Value = MasterTable Mailing Address
		szBuffer.Append(PadRight(clsDR.XMLStr(_T("strAddress1"), false) + clsDR.XMLStr(_T("strAddress2"), false), 40, _T(" ")));
		//Position = 408-447, Length = 40, Field = Blank, Value = Enter 40 Blanks
		szBuffer.Append(PadRight(_T(" "), 40, _T(" ")));
		//Position = 448-487, Length = 40, Field = Payee City, Value = MasterTable Mailing City
		szBuffer.Append(PadRight(clsDR.XMLStr(_T("strCity"), false), 40, _T(" ")));
		//Position = 488-489, Length = 2, Field = Payee State, Value = MasterTable Mailing State
		szBuffer.Append(PadRight(clsDR.XMLStr(_T("strProvince"), false), 2, _T(" ")));
		//Position = 490-498, Length = 9, Field = Payee Zip, Value = MasterTable Mailing Zip
		szBuffer.Append(PadRight(clsDR.XMLStr(_T("strPostalCode"), false), 9, _T(" ")));
		//Position = 499, Length = 1, Field = Blank, Value = Enter 1 Blank
		szBuffer.Append(_T(" "));
		//Position = 500-507, Length = 8, Field = Record Sequence Number, Value = Incremented as records are created
		szTemp.Format(_T("%d"), m_nRecordSequenceNumber);
		szBuffer.Append(PadLeft(szTemp, 8, _T("0")));
		//Position = 508-543, Length = 36, Field = Blank, Value = Enter 36 Blanks
		szBuffer.Append(PadRight(_T(" "), 36, _T(" ")));
		//Position = 544, Length = 1, Field = Second TIN, Value = MasterTable 2nd TIN
		szBuffer.Append(clsDR.XMLBOOL(_T("bol2ndTIN"), false) ? _T("2") : _T(" "));
		//RS - 11/17/2021 - PBI 59039 - Added new 1099 Type to Electronic File for 2021
		//Position = 545-546, Length = 2, Field = Blank, Value = Enter 2 Blanks
		szBuffer.Append(PadRight(_T(" "), 2, _T(" ")));
		//Position = 547, Length = 1, Field = Direct Sales Indicator, Value = MasterTable bol5000DirSales
		szBuffer.Append(clsDR.XMLBOOL(_T("bol5000DirSales"), false) ? _T("1") : _T(" "));
		//RS - 9/30/2025 - PBI 65630 - Added Special Data Entries for year 2025
		//Position = 548-662, Length = 115, Field = Blank, Value = Enter 115 Blanks
		szBuffer.Append(PadRight(_T(" "), 115, _T(" ")));
		//Position = 663-722, Length = 60, Field = Special Data Entries, Value = Enter 60 Blanks
		szBuffer.Append(PadRight(_T(" "), 60, _T(" ")));
		//Position = 723-734, Length = 12, Field = State Income Tax Withheld, Value = Enter 12 Blanks
		szBuffer.Append(PadRight(_T(" "), 12, _T(" ")));
		//Position = 735-746, Length = 12, Field = Local Income Tax Withheld, Value = Enter 12 Blanks
		szBuffer.Append(PadRight(_T(" "), 12, _T(" ")));
		//Position = 747-748, Length = 2, Field = Combined Federal/State Code, Value = Use Select Case in 16 bit			
		if (m_bchkCombinedFederalState)
		{
			szBuffer.Append(PadRight(szFedStateCode, 2, _T(" ")));
		}
		else
		{
			szBuffer.Append(PadRight(_T(" "), 2, _T(" ")));
		}
		//Position = 749-750, Length = 2, Field = Blanks, Value = CR/LF
		szBuffer.Append(_T("\r\n"));

		while (!FileWrite(szBuffer) && !m_bCancel) {
			//Do this until either successful, or canceled!
		}
		if (m_bCancel) {
			//We canceled!
			return false;
		}
	}

	return true;
}
//------------------------------------------------------------------------------
//Create and Write C Records for 1099-NEC
bool CRFV_APElectronicFile1099Std::Write1099C_NEC()
{
	CString szBuffer;
	CString szTemp; //Used to convert numbers to strings

	m_nRecordSequenceNumber++;	//12/06/2007 CJB - 1-28229
	//Fill Buffer with data
	//Position = 1, Length = 1, Field = Record Type, Value = C
	szBuffer.Append(_T("C"));
	//Position = 2-9, Length = 8, Field = Number of Payees, Value = From Tracked Amount
	szTemp.Format(_T("%d"), m_nNumberBNecRecords);
	szBuffer.Append(PadLeft(szTemp, 8, _T("0")));
	//Position = 10-15, Length = 6, Field = Blank, Value = Enter 6 Blanks
	szBuffer.Append(PadRight(_T(" "), 6, _T(" ")));
	//Position = 16-33, Length = 18, Field = B Record Totals of Type 1, Value = From Tracked Amount
	szBuffer.Append(FormatNumber(m_dNecType1Total, 18));
	//Position = 34-51, Length = 18, Field = B Record Totals of Type 2, Value = From Tracked Amount
	szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
	//Position = 52-69, Length = 18, Field = B Record Totals of Type 3, Value = From Tracked Amount
	szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
	//Position = 70-87, Length = 18, Field = B Record Totals of Type 4, Value = From Tracked Amount
	szBuffer.Append(FormatNumber(m_dNecType4Total, 18));
	//Position = 88-105, Length = 18, Field = B Record Totals of Type 5, Value = From Tracked Amount
	szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
	//Position = 106-123, Length = 18, Field = B Record Totals of Type 6, Value = From Tracked Amount
	szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
	//Position = 124-141, Length = 18, Field = B Record Totals of Type 7, Value = From Tracked Amount
	szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
	//Position = 142-159, Length = 18, Field = B Record Totals of Type 8, Value = From Tracked Amount
	szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
	//Position = 160-177, Length = 18, Field = B Record Totals of Type 9, Value = From Tracked Amount
	szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
	//Position = 178-195, Length = 18, Field = B Record Totals of Type A, Value = From Tracked Amount
	szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
	//Position = 196-213, Length = 18, Field = B Record Totals of Type B, Value = From Tracked Amount
	szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
	//Position = 214-231, Length = 18, Field = B Record Totals of Type C, Value = From Tracked Amount
	szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
	//Position = 232-249, Length = 18, Field = B Record Totals of Type D, Value = From Tracked Amount
	szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
	//Position = 250-267, Length = 18, Field = B Record Totals of Type E, Value = From Tracked Amount
	szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
	//DRR 12/01/09 1-33838
	//Position = 268-285, Length = 18, Field = B Record Totals of Type F, Value = Enter 18 zeros
	szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
	//Position = 286-303, Length = 18, Field = B Record Totals of Type G, Value = Enter 18 zeros
	szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
	//Position = 304-321, Length = 18, Field = B Record Totals of Type H, Value = Enter 18 zeros
	szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
	//Position = 322-339, Length = 18, Field = B Record Totals of Type J, Value = Enter 18 zeros
	szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
	//Position = 340-499, Length = 160, Field = Blank, Value = Enter 160 Blanks
	szBuffer.Append(PadRight(_T(" "), 160, _T(" ")));
	//Position = 500-507, Length = 8, Field = Record Sequence Number, Value = Incremented as records are created
	//szTemp.Format(_T("%d"), m_NumberBRecords + 3);
	szTemp.Format(_T("%d"), m_nRecordSequenceNumber);	//12/06/2007 CJB - 1-28229
	szBuffer.Append(PadLeft(szTemp, 8, _T("0")));
	//Position = 508-748, Length = 241, Field = Blank, Value = Enter 241 Blanks
	szBuffer.Append(PadRight(_T(" "), 241, _T(" ")));
	//Position = 749-750, Length = 2, Field = Blank, Value = CR/LF
	szBuffer.Append(_T("\r\n"));

	while (!FileWrite(szBuffer) && !m_bCancel) {
		//Do this until either successful, or canceled!
	}

	return !m_bCancel;
}
//------------------------------------------------------------------------------
//Create and Write 1099K Records
bool CRFV_APElectronicFile1099Std::Write1099K()
{
	CString szBuffer;
	CString szTemp; //Used to convert numbers to strings

	//12/06/2007 CJB - 1-28229  //Separate K Records for each State
	for (int nFedStCode = 1; nFedStCode <= NBR_FED_STATE; nFedStCode++) {
		if (m_ElectronicFileKrecordTotals[nFedStCode].bUsed == true) {
			m_nRecordSequenceNumber++;
			//Initialize Buffer
			szBuffer = _T("");
			//Fill Buffer with data
			//Position = 1, Length = 1, Field = Record Type, Value = K
			szBuffer.Append(_T("K"));
			//Position = 2-9, Length = 8, Field = Number of Payees, Value = Number of B Records
			szTemp.Format(_T("%d"), m_ElectronicFileKrecordTotals[nFedStCode].nNumberStateBRecords);
			szBuffer.Append(PadLeft(szTemp, 8, _T("0")));
			//Position = 10-15, Length = 6, Field = Blank, Value = Enter 6 Blanks
			szBuffer.Append(PadRight(_T(" "), 6, _T(" ")));
			//Position = 16-33, Length = 18, Field = B Record Totals of Type 1, Value = From Tracked Amount
			szBuffer.Append(FormatNumber(m_ElectronicFileKrecordTotals[nFedStCode].dType1StateTotal, 18));
			//Position = 34-51, Length = 18, Field = B Record Totals of Type 2 , Value = From Tracked Amount
			szBuffer.Append(FormatNumber(m_ElectronicFileKrecordTotals[nFedStCode].dType2StateTotal, 18));
			//Position = 52-69, Length = 18, Field = B Record Totals of Type 3, Value = From Tracked Amount
			szBuffer.Append(FormatNumber(m_ElectronicFileKrecordTotals[nFedStCode].dType3StateTotal, 18));
			//Position = 70-87, Length = 18, Field = B Record Totals of Type 4, Value = From Tracked Amount
			szBuffer.Append(FormatNumber(m_ElectronicFileKrecordTotals[nFedStCode].dType4StateTotal, 18));
			//Position = 88-105, Length = 18, Field = B Record Totals of Type 5, Value = From Tracked Amount
			szBuffer.Append(FormatNumber(m_ElectronicFileKrecordTotals[nFedStCode].dType5StateTotal, 18));
			//Position = 106-123, Length = 18, Field = B Record Totals of Type 6, Value = From Tracked Amount
			szBuffer.Append(FormatNumber(m_ElectronicFileKrecordTotals[nFedStCode].dType6StateTotal, 18));
			//Position = 124-141, Length = 18, Field = B Record Totals of Type 7, Value = Enter 18 zeros
			szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
			//Position = 142-159, Length = 18, Field = B Record Totals of Type 8, Value = From Tracked Amount
			szBuffer.Append(FormatNumber(m_ElectronicFileKrecordTotals[nFedStCode].dType8StateTotal, 18));
			//Position = 160-177, Length = 18, Field = B Record Totals of Type 9, Value = From Tracked Amount
			szBuffer.Append(FormatNumber(m_ElectronicFileKrecordTotals[nFedStCode].dType9StateTotal, 18));
			//Position = 178-195, Length = 18, Field = B Record Totals of Type A, Value = From Tracked Amount
			szBuffer.Append(FormatNumber(m_ElectronicFileKrecordTotals[nFedStCode].dTypeAStateTotal, 18));
			//Position = 196-213, Length = 18, Field = B Record Totals of Type B, Value = From Tracked Amount
			szBuffer.Append(FormatNumber(m_ElectronicFileKrecordTotals[nFedStCode].dTypeBStateTotal, 18));
			//Position = 214-231, Length = 18, Field = B Record Totals of Type C, Value = From Tracked Amount
			szBuffer.Append(FormatNumber(m_ElectronicFileKrecordTotals[nFedStCode].dTypeCStateTotal, 18));
			//Position = 232-249, Length = 18, Field = B Record Totals of Type D, Value = From Tracked Amount
			szBuffer.Append(FormatNumber(m_ElectronicFileKrecordTotals[nFedStCode].dTypeDStateTotal, 18));
			//Position = 250-267, Length = 18, Field = B Record Totals of Type E, Value = From Tracked Amount
			szBuffer.Append(FormatNumber(m_ElectronicFileKrecordTotals[nFedStCode].dTypeEStateTotal, 18));
			//DRR 12/01/09 1-33838
			//RS - 11/17/2021 - PBI 59039 - Added new 1099 Type to Electronic File for 2021
			//Position = 268-285, Length = 18, Field = B Record Totals of Type F, Value = From Tracked Amount
		    szBuffer.Append(FormatNumber(m_ElectronicFileKrecordTotals[nFedStCode].dTypeFStateTotal, 18));
			//Position = 286-303, Length = 18, Field = B Record Totals of Type G, Value = From Tracked Amount
			szBuffer.Append(FormatNumber(m_ElectronicFileKrecordTotals[nFedStCode].dTypeGStateTotal, 18));			
			//Position = 304-321, Length = 18, Field = B Record Totals of Type H, Value = Enter 18 zeros
			szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
			//Position = 322-339, Length = 18, Field = B Record Totals of Type J, Value = Enter 18 zeros
			szBuffer.Append(PadRight(_T("0"), 18, _T("0")));
			//Position = 340-499, Length = 160, Field = Blank, Value = Enter 160 Blanks
			szBuffer.Append(PadRight(_T(" "), 160, _T(" ")));
			//Position = 500-507, Length = 8, Field = Record Sequence Number, Value = Incremented as records are created
			//szTemp.Format(_T("%d"), m_NumberBRecords + 4);
			szTemp.Format(_T("%d"), m_nRecordSequenceNumber);	//12/06/2007 CJB - 1-28229
			szBuffer.Append(PadLeft(szTemp, 8, _T("0")));
			//Position = 508-706, Length = 199, Field = Blank, Value = Enter 199 Blank
			szBuffer.Append(PadRight(_T(" "), 199, _T(" ")));
			//Position = 707-724, Length = 18, Field = State Income Tax Withheld, Value = Enter 18 Blanks
			szBuffer.Append(PadRight(_T(" "), 18, _T(" ")));
			//Position = 725-742, Length = 18, Field = Local Income Tax Withheld, Value = Enter 18 Blanks
			szBuffer.Append(PadRight(_T(" "), 18, _T(" ")));
			//Position = 743-746, Length = 4, Field = Blank, Value = Enter 4 Blanks
			szBuffer.Append(PadRight(_T(" "), 4, _T(" ")));
			//Position = 747-748, Length = 2, Field = Combined Federal/State Code, Value = Use Select Case in 16 bit
			//12/06/2007 CJB - 1-28229  //State from Vendor Master Table
			//szBuffer.Append(GetCombCode());
			szBuffer.Append(PadRight(m_ElectronicFileKrecordTotals[nFedStCode].szFedStateCode, 2, _T(" ")));
			//Position = 749-750, Length = 2, Field = Blank, Value = CR/LF
			szBuffer.Append(_T("\r\n"));

			while (!FileWrite(szBuffer) && !m_bCancel) {
				//Do this until either successful, or canceled!
			}
			if (m_bCancel) {
				//We canceled!
				return false;
			}
		}
	}

	return true;
}
//------------------------------------------------------------------------------
//Create and Write 1099F Records
bool CRFV_APElectronicFile1099Std::Write1099F()
{
	CString szBuffer;
	CString szTemp;  //Used to convert numbers to strings for formatting

	m_nRecordSequenceNumber++;	//12/06/2007 CJB - 1-28229
	//Fill Buffer with data
	//Position = 1, Length = 1, Field = Record Type, Value = F
	szBuffer.Append(_T("F"));
	//Position = 2-9, Length = 8, Field = Number of A Records, Value = Tracked Number
	//RS - 11/21/2024 - PBI 64417 - Corrected
	szTemp.Format(_T("%d"), m_nNumberARecords);
	szBuffer.Append(PadLeft(szTemp, 8, _T("0")));
	//Position = 10-30, Length = 21, Field = Zeros, Value = Enter 21 Zeros
	szBuffer.Append(PadLeft(_T("0"), 21, _T("0")));
	//Position = 31-49, Length = 19, Field = Blank, Value = Enter 19 Blanks
	szBuffer.Append(PadRight(_T(" "), 19, _T(" ")));
	//Position = 50-57, Length = 8, Field = Total Number of Payees, Value = Tracked Number
	//RCG - 12/15/2014 - Bug 25757 - Total Number of Payees already in "T" Record so leave blank
	//szTemp.Format(_T("%d"), m_nNumberBRecords);
	//szBuffer.Append(PadLeft(szTemp, 8, _T("0")));
	szBuffer.Append(PadRight(_T(" "), 8, _T(" ")));
	//Position = 58-499, Length = 442, Field = Blank, Value = Enter 442 Blanks
	szBuffer.Append(PadRight(_T(" "), 442, _T(" ")));
	//Position = 500-507, Length = 8, Field = Record Sequence Number, Value = Incremented as records are created
	//szTemp.Format(_T("%d"), m_NumberBRecords + 5);
	szTemp.Format(_T("%d"), m_nRecordSequenceNumber);	//12/06/2007 CJB - 1-28229
	szBuffer.Append(PadLeft(szTemp, 8, _T("0")));
	//Position = 508-748, Length = 241, Field = Blank, Value = Enter 241 Blank
	szBuffer.Append(PadRight(_T(" "), 241, _T(" ")));
	//Position = 749-750, Length = 2, Field = Blank, Value = CR/LF
	szBuffer.Append(_T("\r\n"));

	while (!FileWrite(szBuffer) && !m_bCancel) {
		//Do this until either successful, or canceled!
	}

	return !m_bCancel;
}
//------------------------------------------------------------------------------
//Pad left side of szStringToPad with szPadString to nFinalLength
CString CRFV_APElectronicFile1099Std::PadLeft(CString szStringToPad, int nFinalLength, CString szPadString, bool bMakeExact)
{
	CString szFinal = szStringToPad;

	while (szFinal.GetLength() < nFinalLength) {
		szFinal = szPadString + szFinal;
	}
	if (bMakeExact) {
		szFinal = szFinal.Right(nFinalLength);
	}
	return szFinal;
}
//------------------------------------------------------------------------------
//Pad right side of szStringToPad with szPadString to nFinalLength
CString CRFV_APElectronicFile1099Std::PadRight(CString szStringToPad, int nFinalLength, CString szPadString, bool bMakeExact)
{
	CString szFinal = szStringToPad;

	while (szFinal.GetLength() < nFinalLength) {
		szFinal.Append(szPadString);
	}
	if (bMakeExact) {
		szFinal = szFinal.Left(nFinalLength);
	}

	return szFinal;
}
//------------------------------------------------------------------------------
//Format number for B Record Totals
CString CRFV_APElectronicFile1099Std::FormatNumber(double dNumber, int nFinalLength)
{
	double   dTemp;  //07/25/2006 CJB - 1-25084
	CString szTemp;
	bool   bNeg;

	dTemp = dNumber * 100;
	bNeg = (dTemp < 0);
	if (bNeg) {
		//Get Absolute value
		dTemp = dTemp * -1;
	}

	//07/25/2006 CJB - 1-25084
	szTemp.Format(_T("%.0f"), dTemp);	// begbert 06-19-2006 1-21100 : this line was missing
	//szTemp = PadLeft(szTemp, nFinalLength - 1, _T("0"));
	//szTemp = (bNeg? _T("-") : _T("+") ) + szTemp;
	szTemp = PadLeft(szTemp, nFinalLength, _T("0"));

	return szTemp;
}
//------------------------------------------------------------------------------
//Get Combined Federal/State Code
CString CRFV_APElectronicFile1099Std::GetCombCode(CString szState)
{
	CString szTemp;
	//12/06/2007 CJB - 1-28229  //State from Vendor Master Table
	//mapCombinedFedState.Lookup(si->MagneticMediaConstants.TransState, szTemp);
	mapCombinedFedState.Lookup(szState, szTemp);
	if (szTemp == _T("")) {
		szTemp = PadRight(_T(" "), 2, _T(" "));  //07/25/2006 CJB - 1-25084
	}
	return szTemp;
}
//------------------------------------------------------------------------------
//Reformat PhoneNumber and return boolean - original has digits only
bool CRFV_APElectronicFile1099Std::ReformatPhoneNumber(CString &szPhoneNumber)
{
	CString szTmp;
	bool	bDigitsOnly = true;

	//First strip hyphens, parenthesis, plus signs(international), and spaces
	szPhoneNumber.Replace(_T("-"), _T(""));
	szPhoneNumber.Replace(_T("("), _T(""));
	szPhoneNumber.Replace(_T(")"), _T(""));
	szPhoneNumber.Replace(_T("+"), _T(""));
	szPhoneNumber.Replace(_T(" "), _T(""));

	for (int nChar = 0; nChar < szPhoneNumber.GetLength(); nChar++) {
		if (!isdigit(szPhoneNumber.GetAt(nChar))) {
			bDigitsOnly = false;
		}
		else {
			szTmp = szTmp + szPhoneNumber.GetAt(nChar);
		}
	}

	PadRight(szTmp, 15, _T(" "));
	szTmp.Trim();
	szPhoneNumber = szTmp;


	return bDigitsOnly;
}
//------------------------------------------------------------------------------
//Write line to file
bool CRFV_APElectronicFile1099Std::FileWrite(CString szData)
{
	CFile fil1099File;
	CString szfil1099Extension = _T(""); //Initialize to blank for non-removable media
	try {
		ULARGE_INTEGER lnFreeBytesAvail;
		ULARGE_INTEGER lnTotalBytes;
		ULARGE_INTEGER lnFreeBytes;

		//First check string length to match (750 chars)
		/*int nLength =*/ szData.GetLength();
		if (szData.GetLength() != 750) {
			//CGBLForm::CMSMessageBox(this, 0, 0, MB_OK| MB_ICONWARNING, "Improper field length on record " + szData.Left(1) + " of - " + szData.GetLength());
			TRACE("Improper field length on record %s of - %d\n", szData.Left(1), szData.GetLength());
		}
		if (m_bRemovableDrive) {
			//Removable Drive
			if (m_nFilecount == 0) {
				//Ask to insert Disk
				if (IDCANCEL == CGBLForm::CMSMessageBox(this, IDS_AP_INSERT_A_DISKETTE_INTO_THE_FLOPPY_DRIVE, IDS_AP_MAGNETIC_MEDIA, MB_OKCANCEL | MB_ICONSTOP)) {
					m_bCancel = true;
					return false;
				}
				//First time through, must ask for diskette!
				while (!CheckNewDisk() && !m_bCancel) {
				}
			}
			if (m_bCancel) {
				return false;
			}
			//Get the remaining space on the disk
			GetDiskFreeSpaceEx(m_szPath, &lnFreeBytesAvail, &lnTotalBytes, &lnFreeBytes);
			//Create extension for file to open (ie. .001, .002, etc.)
			szfil1099Extension.Format(_T("%d"), m_nFilecount);
			szfil1099Extension = PadLeft(szfil1099Extension, 3, _T("0"));
			szfil1099Extension = _T(".") + szfil1099Extension;
			//Is there room on the disk for the record?
			if (lnFreeBytesAvail.QuadPart < 750) {
				//NO Ask for a new disk
				if (IDCANCEL == CGBLForm::CMSMessageBox(this, IDS_AP_THIS_DISK_HAS_NO_FREE_SPACE, IDS_AP_MAGNETIC_MEDIA, MB_OKCANCEL | MB_ICONSTOP)) {
					m_bCancel = true;
					return false;
				}
				//Make sure new disk is blank and we haven't canceled!
				while (!CheckNewDisk() && !m_bCancel) {
				}
			}
		}
		if (m_bCancel) {
			return false;
		}

		//RS - 11/18/2025 - PBI 65630
		CString fireFilePath;
		//If first write to disk, delete file if it exists
		//12/10/2007 CJB - 1-28229  //reset the NewDisk flag even if it did not exist
		//if (m_bNewDisk && CMS::FileExists(m_szPath + TAXFILENAME)){
		if (m_bNewDisk && (m_cboEFileWith.GetCurSel() == 0)) {
			if (m_bchkCombinedFederalState)
				fireFilePath = m_szPath + TAXFILEFIRE + m_szPaymentYear + _T("_CFSFP");
			else
				fireFilePath = m_szPath + TAXFILEFIRE + m_szPaymentYear;

			if (CMS::FileExists(fireFilePath)) {
				CFile::Remove(fireFilePath);
			}
			m_bNewDisk = false;
		}

		//RS - 09/24/2025 - PBI 64621 - Added IRIS
		CCMSWaitCursor c;
		CStdioFile file;
		BOOL bEnable = (m_cboEFileWith.GetCurSel() == 0);
		BOOL b1099Form = (m_cbo1099Form.GetCurSel() == 0);
		CString csvFilePath;

		if (!bEnable)
		{
			if (b1099Form)
			{
				if(m_bchkCombinedFederalState)
					csvFilePath = m_szPath + _T("IRIS_") + m_szPaymentYear + _T("_") + TAXFILEMISC + _T("_CFSFP") + _T(".csv");
				else
					csvFilePath = m_szPath + _T("IRIS_") + m_szPaymentYear + _T("_") + TAXFILEMISC + _T(".csv");
			}
			else
			{
				if (m_bchkCombinedFederalState)
					csvFilePath = m_szPath + _T("IRIS_") + m_szPaymentYear + _T("_") + TAXFILENEC + _T("_CFSFP") + _T(".csv");
				else
					csvFilePath = m_szPath + _T("IRIS_") + m_szPaymentYear + _T("_") + TAXFILENEC + _T(".csv");
			}
			
			//RS - 01/15/2026 - PBI 64621 - Added CSV UTF-8 for IRIS
			WriteCsvUtf8_WithBOM(csvFilePath, szData);
		}
		else
		{
			//RS - 01/14/2026 - PBI 65630 - Create filename depending on what checkboxes are selected
			if (m_bchkTestIndicator && m_bchkUseLastYear && m_bchkCombinedFederalState)
				fireFilePath = m_szPath + TAXFILEFIRE + m_szPaymentYear + _T("_LYA") + _T("_Test") + _T("_CFSFP") + szfil1099Extension;
			else if (m_bchkTestIndicator && m_bchkUseLastYear)
				fireFilePath = m_szPath + TAXFILEFIRE + m_szPaymentYear + _T("_LYA") + _T("_Test") + szfil1099Extension;
			else if (m_bchkUseLastYear && m_bchkCombinedFederalState)
				fireFilePath = m_szPath + TAXFILEFIRE + m_szPaymentYear + _T("_LYA") + _T("_CFSFP") + szfil1099Extension;
			else if (m_bchkTestIndicator && m_bchkCombinedFederalState)
				fireFilePath = m_szPath + TAXFILEFIRE + m_szPaymentYear + _T("_Test") + _T("_CFSFP") + szfil1099Extension;
			else if (m_bchkTestIndicator)
				fireFilePath = m_szPath + TAXFILEFIRE + m_szPaymentYear + _T("_Test") + szfil1099Extension;
			else if (m_bchkUseLastYear)
				fireFilePath = m_szPath + TAXFILEFIRE + m_szPaymentYear + _T("_LYA") + szfil1099Extension;
			else if (m_bchkCombinedFederalState)
				fireFilePath = m_szPath + TAXFILEFIRE + m_szPaymentYear + _T("_CFSFP") + szfil1099Extension;
			else
				fireFilePath = m_szPath + TAXFILEFIRE + m_szPaymentYear + szfil1099Extension;

			//Write to data to file...
			fil1099File.Open(fireFilePath, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite);
			fil1099File.SeekToEnd();
			CW2A lpszData(szData.MakeUpper().GetString());
			lpszData[szData.GetLength()] = NULL;
			fil1099File.Write(lpszData, (UINT)::strlen(lpszData));
			fil1099File.Close();
		}

		c.Restore();
		return true;
	}

	catch (...)
	{
		//display what the problem was
		LPVOID lpMsgBuf = NULL;
		::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
			, NULL
			, ::GetLastError()
			, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)	// Default language
			, (LPTSTR)&lpMsgBuf
			, 0
			, NULL);
		CString szMsg = (LPTSTR)lpMsgBuf;
		szMsg.Append(_T("Please fix the error and try again!"));
		CGBLForm::CMSMessageBox(this, 0, 0, MB_OK | MB_ICONEXCLAMATION, szMsg, _T("Error!"));
		::LocalFree(lpMsgBuf);
		/*return false;*/
		if (m_bRemovableDrive) {
			if (IDCANCEL == CGBLForm::CMSMessageBox(this, IDS_AP_INSERT_A_DISKETTE_INTO_THE_FLOPPY_DRIVE, IDS_AP_MAGNETIC_MEDIA, MB_OKCANCEL | MB_ICONSTOP)) {
				m_bCancel = true;
				return false;
			}
		}
		fil1099File.Close();
		return false;
	}
}
//RS - 01/15/2026 - PBI 64621 - Added CSV UTF-8 for IRIS
void CRFV_APElectronicFile1099Std::WriteCsvUtf8_WithBOM(const CString& filePath, const CString& data)
{
	CFile file;
	if (!file.Open(filePath,
				   CFile::modeCreate |
				   CFile::modeWrite |
				   CFile::typeBinary))
	{
		return;
	}

	// 1️. UTF-8 BOM (Excel requirement)
	BYTE bom[] = {0xEF, 0xBB, 0xBF};
	file.Write(bom, 3);

	// 2️. Convert CString (UTF-16) -> UTF-8
	int utf8Len = WideCharToMultiByte(
		CP_UTF8,
		0,
		data,
		-1,
		nullptr,
		0,
		nullptr,
		nullptr
	);

	if (utf8Len > 1)
	{
		std::string utf8Data(utf8Len - 1, 0);

		WideCharToMultiByte(
			CP_UTF8,
			0,
			data,
			-1,
			&utf8Data[0],
			utf8Len,
			nullptr,
			nullptr
		);

		// 3️. Write CSV content
		file.Write(utf8Data.c_str(), (UINT)utf8Data.length());
	}

	file.Close();
}
//------------------------------------------------------------------------------
bool CRFV_APElectronicFile1099Std::CheckNewDisk()
{
	ULARGE_INTEGER lnFreeBytesAvail;
	ULARGE_INTEGER lnTotalBytes;
	ULARGE_INTEGER lnFreeBytes;
	//Get Disk Free Space
	BOOL BValid = GetDiskFreeSpaceEx(m_szPath, &lnFreeBytesAvail, &lnTotalBytes, &lnFreeBytes);
	if (!BValid) {
		//display what the problem was
		LPVOID lpMsgBuf = NULL;
		::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
			, NULL
			, ::GetLastError()
			, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)	// Default language
			, (LPTSTR)&lpMsgBuf
			, 0
			, NULL);
		CString szMsg = (LPTSTR)lpMsgBuf;
		szMsg.Append(_T("Please fix the error and try again!"));
		if (IDCANCEL == CGBLForm::CMSMessageBox(this, 0, 0, MB_OKCANCEL | MB_ICONEXCLAMATION, szMsg, _T("Disk Error!"))) {
			m_bCancel = true;
		}
		::LocalFree(lpMsgBuf);
		return false;
	}
	else if (lnTotalBytes.QuadPart != lnFreeBytes.QuadPart) {
		//Is disk empty?
		if (IDCANCEL == CGBLForm::CMSMessageBox(this, IDS_AP_THIS_DISKETTE_IS_NOT_EMPTY, IDS_AP_MAGNETIC_MEDIA, MB_OKCANCEL | MB_ICONSTOP)) {
			m_bCancel = true;
		}
		return false;
	}
	if (lnFreeBytesAvail.QuadPart < 750) {
		//Do not have enough space on the disk... ask for different disk
		if (IDCANCEL == CGBLForm::CMSMessageBox(this, IDS_AP_THIS_DISK_HAS_NO_FREE_SPACE, IDS_AP_MAGNETIC_MEDIA, MB_OKCANCEL | MB_ICONSTOP)) {
			m_bCancel = true;
		}
		return false;
	}
	m_nFilecount++;
	return true;
}
//------------------------------------------------------------------------------
void CRFV_APElectronicFile1099Std::ResetTotals(void)
{
	m_dMiscType1Total = 0;
	m_dMiscType2Total = 0;
	m_dMiscType3Total = 0;
	m_dMiscType4Total = 0;
	m_dMiscType5Total = 0;
	m_dMiscType6Total = 0;
	m_dMiscType8Total = 0;
	m_dMiscType9Total = 0;
	m_dMiscTypeATotal = 0;
	m_dMiscTypeBTotal = 0;
	m_dMiscTypeCTotal = 0;
	m_dMiscTypeDTotal = 0;
	m_dMiscTypeETotal = 0;
	m_dMiscTypeFTotal = 0;
	m_dMiscTypeGTotal = 0;
	m_dMiscTypeHTotal = 0;
	m_dMiscTypeJTotal = 0;

	//RS - 11/25/2020 - PBI 56232 - Added 1099-NEC to Electronic File for 2020
	m_dNecType1Total = 0;
	m_dNecType4Total = 0;

	for (int nFedStCode = 0; nFedStCode <= NBR_FED_STATE; nFedStCode++) {
		m_ElectronicFileKrecordTotals[nFedStCode].bUsed = false;
		m_ElectronicFileKrecordTotals[nFedStCode].szState = _T("");
		m_ElectronicFileKrecordTotals[nFedStCode].szFedStateCode = _T("");
		m_ElectronicFileKrecordTotals[nFedStCode].nNumberStateBRecords = 0;
		m_ElectronicFileKrecordTotals[nFedStCode].dType1StateTotal = 0;
		m_ElectronicFileKrecordTotals[nFedStCode].dType2StateTotal = 0;
		m_ElectronicFileKrecordTotals[nFedStCode].dType3StateTotal = 0;
		m_ElectronicFileKrecordTotals[nFedStCode].dType4StateTotal = 0;
		m_ElectronicFileKrecordTotals[nFedStCode].dType5StateTotal = 0;
		m_ElectronicFileKrecordTotals[nFedStCode].dType6StateTotal = 0;
		m_ElectronicFileKrecordTotals[nFedStCode].dType8StateTotal = 0;
		m_ElectronicFileKrecordTotals[nFedStCode].dType9StateTotal = 0;
		m_ElectronicFileKrecordTotals[nFedStCode].dTypeAStateTotal = 0;
		m_ElectronicFileKrecordTotals[nFedStCode].dTypeBStateTotal = 0;
		m_ElectronicFileKrecordTotals[nFedStCode].dTypeCStateTotal = 0;
		m_ElectronicFileKrecordTotals[nFedStCode].dTypeDStateTotal = 0;
		m_ElectronicFileKrecordTotals[nFedStCode].dTypeEStateTotal = 0;
		m_ElectronicFileKrecordTotals[nFedStCode].dTypeFStateTotal = 0;
		m_ElectronicFileKrecordTotals[nFedStCode].dTypeGStateTotal = 0;
		m_ElectronicFileKrecordTotals[nFedStCode].dTypeHStateTotal = 0;
		m_ElectronicFileKrecordTotals[nFedStCode].dTypeJStateTotal = 0;
	}
}
//------------------------------------------------------------------------------
//RCG - 10/27/2023 - PBI 63016 - Changed Payment Year from textbox to drop-down listbox
void CRFV_APElectronicFile1099Std::OnCbnSelchangeApfCboPaymentYear()
{
	m_cboPaymentYear.GetLBText(m_cboPaymentYear.GetCurSel(), m_szPaymentYear);

	//RS - 11/14/2025 - PBI 64621
	if(si->ModuleSettings.APSettings.CurrentYearEnd != m_szPaymentYear)
		m_chkUseLastYear.SetCheck(1);
	else
		m_chkUseLastYear.SetCheck(0);

	LoadCombinedFedStateMap();
}

//RCG - 09/24/2025 - PBI 64621 - Added IRIS
void CRFV_APElectronicFile1099Std::OnCbnSelchangeApfCboEfileWith()
{
	BOOL bEnable = (m_cboEFileWith.GetCurSel() == 0);
	int nYear, nYearCountMax;

	if (bEnable)
	{
		//Hide IRIS combo box
		m_lbl1099Form.ShowWindow(SW_HIDE);
		m_cbo1099Form.ShowWindow(SW_HIDE);
		nYearCountMax = 10;
		//Show FIRE related checkboxes
		m_chkTestIndicator.ShowWindow(SW_SHOW);
		m_chkLastFilingIndicator.ShowWindow(SW_SHOW);
		m_chkCombinedFederalState.ShowWindow(SW_SHOW);
	}
	else
	{
		//Show Form combo box
		m_lbl1099Form.ShowWindow(SW_SHOW);
		m_cbo1099Form.ShowWindow(SW_SHOW);
		nYearCountMax = 3;
		//Hide and uncheck FIRE related checkboxes
		m_chkTestIndicator.ShowWindow(SW_HIDE);
		m_chkLastFilingIndicator.ShowWindow(SW_HIDE);
		m_chkCombinedFederalState.ShowWindow(SW_HIDE);
		m_chkTestIndicator.SetCheck(0);
		m_chkLastFilingIndicator.SetCheck(0);
		m_chkCombinedFederalState.SetCheck(0);
	}
	m_cboPaymentYear.ResetContent();
	for (int nYearCount = 0; nYearCount <= nYearCountMax; nYearCount++)
	{
		nYear = CURRENT_PAYMENT_YEAR - nYearCount;
		CString szYear;
		szYear.Format(_T("%d"), nYear);
		m_cboPaymentYear.AddString(szYear);
	}

	m_cboPaymentYear.SetCurSel(0);

	OnCbnSelchangeApfCboPaymentYear();
}
//RS - 01/15/2026 - PBI 64621 - Added CSV UTF-8 for IRIS
static CString EscapeCsvField(const CString& value)
{
	CString out = value;
	out.Replace(_T("\""), _T("\"\""));  // escape quotes

	if (out.FindOneOf(_T(",\"\r\n")) != -1)
	{
		out = _T("\"") + out + _T("\"");
	}
	return out;
}
//RS - 01/15/2026 - PBI 64621 - Added CSV UTF-8 for IRIS
static void AppendCsv(CString& buffer, const CString& value, bool comma = true)
{
	buffer += EscapeCsvField(value);
	if (comma)
		buffer += _T(",");
}
//RS - 11/19/2025 - PBI 64621 - Added IRIS
bool CRFV_APElectronicFile1099Std::Write_IRIS_1099_MISC()
{
	CString strBuffer;
	CString szTemp;
	CDataRow clsDR;
	CString szTempNameType;
	CString szFedStateCode;
	CString szState;
	szFedStateCode = _T("");
	szState = _T("");
	int nFedStCode;
	double dType1Amount = 0;	//Local Amount for Type 1
	double dType2Amount = 0;	//Local Amount for Type 2
	double dType3Amount = 0;	//Local Amount for Type 3
	double dType4Amount = 0;	//Local Amount for Type 4
	double dType5Amount = 0;	//Local Amount for Type 5
	double dType6Amount = 0;	//Local Amount for Type 6
	double dType8Amount = 0;	//Local Amount for Type 8
	double dType9Amount = 0;	//Local Amount for Type 9 currently always 0.
	double dTypeAAmount = 0;	//Local Amount for Type A
	double dTypeBAmount = 0;	//Local Amount for Type B
	double dTypeCAmount = 0;	//Local Amount for Type C
	double dTypeDAmount = 0;	//Local Amount for Type D
	double dTypeEAmount = 0;	//Local Amount for Type E
	double dTypeFAmount = 0;	//Local Amount for Type F
	double dTypeGAmount = 0;	//Local Amount for Type G

	CStringArray	arrErrors;
	CString		szError;
	CString		sz_ColName;
	CString		sz_RecipientName;
	CString		sz_PayerName;
	CString		strValue;
	int			TransCount = 0;

	long lRowCount = m_dtBMiscRecords.getRowCount();
	if (m_nServerStatus == SERVER_CONTINUE)
		lRowCount--;

	// Append column headers (FIRST ROW)
// -------------------------
	AppendCsv(strBuffer, _T("Form Type"));										//Column A
	AppendCsv(strBuffer, _T("Tax Year"));										//Column B
	AppendCsv(strBuffer, _T("Payer TIN Type"));									//Column C
	AppendCsv(strBuffer, _T("Payer Taxpayer ID Number"));						//Column D
	AppendCsv(strBuffer, _T("Payer Name Type"));								//Column E
	AppendCsv(strBuffer, _T("Payer Business or Entity Name Line 1"));			//Column F
	AppendCsv(strBuffer, _T("Payer Business or Entity Name Line 2"));			//Column G
	AppendCsv(strBuffer, _T("Payer First Name"));								//Column H
	AppendCsv(strBuffer, _T("Payer Middle Name"));								//Column I
	AppendCsv(strBuffer, _T("Payer Last Name (Surname)"));						//Column J
	AppendCsv(strBuffer, _T("Payer Suffix"));									//Column K
	AppendCsv(strBuffer, _T("Payer Country"));									//Column L
	AppendCsv(strBuffer, _T("Payer Address Line 1"));							//Column M
	AppendCsv(strBuffer, _T("Payer Address Line 2"));							//Column N
	AppendCsv(strBuffer, _T("Payer City/Town"));								//Column O
	AppendCsv(strBuffer, _T("Payer State/Province/Territory"));					//Column P
	AppendCsv(strBuffer, _T("Payer ZIP/Postal Code"));							//Column Q
	AppendCsv(strBuffer, _T("Payer Phone Type"));								//Column R
	AppendCsv(strBuffer, _T("Payer Phone"));									//Column S
	AppendCsv(strBuffer, _T("Recipient TIN Type"));								//Column T
	AppendCsv(strBuffer, _T("Recipient Taxpayer ID Number"));					//Column U
	AppendCsv(strBuffer, _T("Recipient Name Type"));							//Column V
	AppendCsv(strBuffer, _T("Recipient Business or Entity Name Line 1"));		//Column W
	AppendCsv(strBuffer, _T("Recipient Business or Entity Name Line 2"));		//Column X
	AppendCsv(strBuffer, _T("Recipient First Name"));							//Column Y
	AppendCsv(strBuffer, _T("Recipient Middle Name"));							//Column Z
	AppendCsv(strBuffer, _T("Recipient Last Name (Surname)"));					//Column AA
	AppendCsv(strBuffer, _T("Recipient Suffix"));								//Column AB
	AppendCsv(strBuffer, _T("Recipient Country"));								//Column AC
	AppendCsv(strBuffer, _T("Recipient Address Line 1"));						//Column AD
	AppendCsv(strBuffer, _T("Recipient Address Line 2"));						//Column AE
	AppendCsv(strBuffer, _T("Recipient City/Town"));							//Column AF
	AppendCsv(strBuffer, _T("Recipient State/Province/Territory"));				//Column AG
	AppendCsv(strBuffer, _T("Recipient ZIP/Postal Code"));						//Column AH
	AppendCsv(strBuffer, _T("Office Code"));									//Column AI
	AppendCsv(strBuffer, _T("Form Account Number"));							//Column AJ
	AppendCsv(strBuffer, _T("FATCA Filing Requirements"));						//Column AK
	AppendCsv(strBuffer, _T("2nd TIN Notice"));									//Column AL
	AppendCsv(strBuffer, _T("Box 1 - Rents"));									//Column AM
	AppendCsv(strBuffer, _T("Box 2 - Royalties"));								//Column AN
	AppendCsv(strBuffer, _T("Box 3 - Other income"));							//Column AO
	AppendCsv(strBuffer, _T("Box 4 - Federal income tax withheld"));			//Column AP
	AppendCsv(strBuffer, _T("Box 5 - Fishing boat proceeds"));					//Column AQ
	AppendCsv(strBuffer, _T("Box 6 - Medical and health care payments"));		//Column AR
	AppendCsv(strBuffer, _T("Box 7 - Direct sales of $5,000 or more of consumer products to a recipient for resale"));	//Column AS
	AppendCsv(strBuffer, _T("Box 8 - Substitute payments in lieu of dividends or interest"));							//Column AT
	AppendCsv(strBuffer, _T("Box 9 - Crop insurance proceeds"));				//Column AU
	AppendCsv(strBuffer, _T("Box 10 - Gross proceeds paid to an attorney"));	//Column AV
	AppendCsv(strBuffer, _T("Box 11 - Fish purchased for resale"));				//Column AW
	AppendCsv(strBuffer, _T("Box 12 - Section 409A deferrals"));				//Column AX
	if (_tstoi(m_szPaymentYear) < PAYMENT_YEAR_2025)
	{
		AppendCsv(strBuffer, _T("Box 14 - Excess golden parachute payments"));	//Column AY
	}
	AppendCsv(strBuffer, _T("Box 15 - Nonqualified deferred compensation"));	//Column AZ or AY for 2025 and newer
	AppendCsv(strBuffer, _T("Combined Federal/State Filing"));					//Column BA or AZ for 2025 and newer
	AppendCsv(strBuffer, _T("State 1"));										//Column BB or BA for 2025 and newer
	AppendCsv(strBuffer, _T("State 1 - State tax withheld"));					//Column BC or BB for 2025 and newer
	AppendCsv(strBuffer, _T("State 1 - State/Payer state number"));				//Column BD or BC for 2025 and newer
	AppendCsv(strBuffer, _T("State 1 - State income"));							//Column BE or BD for 2025 and newer
	AppendCsv(strBuffer, _T("State 1 - Local income tax withheld"));			//Column BF or BE for 2025 and newer
	AppendCsv(strBuffer, _T("State 1 - Special Data Entries"));					//Column BG or BF for 2025 and newer
	AppendCsv(strBuffer, _T("State 2"));										//Column BH or BG for 2025 and newer
	AppendCsv(strBuffer, _T("State 2 - State tax withheld"));					//Column BI or BH for 2025 and newer
	AppendCsv(strBuffer, _T("State 2 - State/Payer state number"));				//Column BJ or BI for 2025 and newer
	AppendCsv(strBuffer, _T("State 2 - State income"));							//Column BK or BJ for 2025 and newer
	AppendCsv(strBuffer, _T("State 2 - Local income tax withheld"));			//Column BL or BK for 2025 and newer
	AppendCsv(strBuffer, _T("State 2 - Special Data Entries"),false);			//Column BM or BL for 2025 and newer

	// End of header row
	strBuffer.AppendFormat(_T("\r\n"));
//------------------------
	
	for (int nRow = 0; nRow < lRowCount; nRow++)
	{
		m_dtBMiscRecords.GetRow(nRow, &clsDR);
		TransCount++;
		szState = _T("");
		szFedStateCode = _T("");
		nFedStCode = 0;
		szState = clsDR.XMLStr(_T("strProvince"), false);
		szFedStateCode = GetCombCode(szState);

		dType1Amount = clsDR.XMLDbl(_T("curAP1099PMT1"), false);
		dType2Amount = clsDR.XMLDbl(_T("curAP1099PMT2"), false);
		dType3Amount = clsDR.XMLDbl(_T("curAP1099PMT3"), false);
		dType4Amount = clsDR.XMLDbl(_T("curAP1099PMT4"), false);
		dType5Amount = clsDR.XMLDbl(_T("curAP1099PMT5"), false);
		dType6Amount = clsDR.XMLDbl(_T("curAP1099PMT6"), false);
		dType8Amount = clsDR.XMLDbl(_T("curAP1099PMT8"), false);
		dType9Amount = 0;
		dTypeAAmount = clsDR.XMLDbl(_T("curAP1099PMT9"), false);
		dTypeBAmount = clsDR.XMLDbl(_T("curAP1099PMT10"), false);
		dTypeCAmount = clsDR.XMLDbl(_T("curAP1099PMT11"), false);
		dTypeDAmount = clsDR.XMLDbl(_T("curAP1099PMT12"), false);
		dTypeEAmount = clsDR.XMLDbl(_T("curAP1099PMT13"), false);
		dTypeFAmount = clsDR.XMLDbl(_T("curAP1099PMT14"), false);
		dTypeGAmount = clsDR.XMLDbl(_T("curAP1099PMT7"), false);

		sz_ColName = clsDR.XMLStr(_T("strVendorName"), false);
		sz_RecipientName = clsDR.XMLStr(_T("strVendor"), false);
		sz_PayerName = si->CompanySettings.Name;

		//Append each row
		//Column A - Form Type
		AppendCsv(strBuffer, _T("1099-MISC"));
		//Column B - Tax Year
		AppendCsv(strBuffer, m_szPaymentYear);
		//Column C - Payer TIN Type*
		szTemp = (clsDR.XMLBOOL(_T("bol1099Flag"), false) ? _T("SSN") : _T("EIN"));
		AppendCsv(strBuffer, szTemp);

		//Column D - Payer Taxpayer ID Number*
		if (szTemp == "SSN")
		{
			//Format SSN (XXX-XX-XXXX)
			//si->ModuleSettings.APSettings.FederalID1099;
			szTemp = clsDR.XMLStr(_T("strFederalIDNumber"), false);
			//conversion & validate ID
			std::string idNumber = CT2A(szTemp);
			std::regex ssnPattern(R"(\d{3}-\d{2}-\d{4})");

			if (!std::regex_match(idNumber, ssnPattern) && !szTemp.IsEmpty())
			{
				strValue.Format(_T("Invalid Taxpayer ID Pattern for Payer '%s'"), sz_PayerName + " for Vendor - " + sz_RecipientName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("D"), strValue);
				arrErrors.Add(szError);
			}
		}
		else if (szTemp == "EIN")
		{
			//Format EIN (XX-XXXXXXX)
			szTemp = si->ModuleSettings.APSettings.FederalID1099;
			szTemp.Replace(_T("-"), _T(""));
			szTemp = szTemp.Left(2) + "-" + szTemp.Mid(2, 7);
			//conversion
			std::string idNumber = CT2A(szTemp);
			std::regex einPattern(R"(\d{2}-\d{7})");

			if (!std::regex_match(idNumber, einPattern) && !szTemp.IsEmpty())
			{
				strValue.Format(_T("Invalid Taxpayer ID Pattern for Payer '%s'"), sz_PayerName + " for Vendor - " + sz_RecipientName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("D"), strValue);
				arrErrors.Add(szError);
			}
		}
		AppendCsv(strBuffer, szTemp);

		//Column E - Payer Name Type*
		szTemp = (clsDR.XMLBOOL(_T("bol1099Flag"), false) ? _T("I") : _T("B"));
		AppendCsv(strBuffer, szTemp);
		szTempNameType = szTemp;

		//Column F - Payer Business or Entity Name Line 1
		CString fullName = si->MagneticMediaConstants.Transmittername;
		CString colF, colG;

		if (fullName.GetLength() <= 40)
		{
			//Fits entirely in Column F
			colF = fullName;
			colG = _T("");	//blank
		}
		else
		{
			//Need to split into Column F and G
			int splitPos = 40;

			//Move back to last space before 40 chars
			while (splitPos > 0 && fullName[splitPos] != ' ')
			{
				splitPos--;
			}

			if (splitPos == 0)
			{
				//No space found, just split hard at 40
				splitPos = 40;
			}

			colF = fullName.Left(splitPos).Trim();
			colG = fullName.Mid(splitPos).Trim();
		}

		if (szTempNameType == "B")
		{
			//szTemp = si->MagneticMediaConstants.Transmittername;
			if (IsValidEntityName(colF))
				AppendCsv(strBuffer, colF);
			else
				AppendCsv(strBuffer, _T(""));

			//Column G - Payer Business or Entity Name Line 2
			if (IsValidEntityName(colG))
				AppendCsv(strBuffer, colG);
			else
				AppendCsv(strBuffer, _T(""));
		}

		//Column H - Payer First Name
		if (szTempNameType == "I")
		{
			szTemp = si->CompanySettings.BillingInformation.Contact;
			if (IsValidPersonName(szTemp))
				AppendCsv(strBuffer, szTemp);
			else
				AppendCsv(strBuffer, _T(""));
		}
		else
			AppendCsv(strBuffer, _T(""));

		//Column I - Payer Middle Name
		szTemp = si->CompanySettings.Name;
		if (IsValidPersonName(szTemp) && !szTemp.IsEmpty())
			AppendCsv(strBuffer, szTemp);
		else
			AppendCsv(strBuffer, _T(""));

		//Column J - Payer Last Name (Surname)
		if (szTempNameType == "I")
		{
			szTemp = _T(" ");

			if (IsValidPersonName(szTemp) && !szTemp.IsEmpty())
				AppendCsv(strBuffer, szTemp);
			else
				AppendCsv(strBuffer, _T(""));
		}
		else
			AppendCsv(strBuffer, _T(""));

		//Column K - Payer Suffix
		szTemp = _T(" ");
		if (IsValidPersonName(szTemp) && !szTemp.IsEmpty())
			AppendCsv(strBuffer, szTemp);
		else
			AppendCsv(strBuffer, _T(""));

		//Column L - Payer Country*
		szTemp = si->CompanySettings.BillingInformation.Country;
		CString PayerCountryCode= GetCountryCode(szTemp);

		if (!PayerCountryCode.IsEmpty())
			AppendCsv(strBuffer, PayerCountryCode);
		else
		{
			strValue.Format(_T("Payer Country should not be empty for '%s'"), sz_PayerName + " for Vendor - " + sz_RecipientName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("L"), strValue);
			arrErrors.Add(szError);
		}

		//Column M - Payer Address Line 1*
		szTemp = si->CompanySettings.BillingInformation.Address1;	
		if (!szTemp.IsEmpty())
		{
			if (IsValidAddress(szTemp, 35))
				AppendCsv(strBuffer, szTemp);
			else
			{
				strValue.Format(
							_T("Invalid format in Payer Address Line 1 of '%s' for '%s'. "
							   "Format should comply to the following: 1) Have a max length of 35 characters, "
							   "2) Only be alpha-numeric characters [A-Z, 0-9] and following special characters: [hyphen ( - ), ampersand ( & ), apostrophe ( ' ), period ( . ), hash ( # ), space ( )], and "
							   "3) Not contain leading, trailing, or adjacent spaces."),
								szTemp,
								sz_PayerName + " for Vendor - " + sz_RecipientName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("M"), strValue);
				arrErrors.Add(szError);
			}
		}
		else
		{
			strValue.Format(_T("Payer Address Line 1 should not be empty for '%s'"), sz_PayerName + " for Vendor - " + sz_RecipientName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("M"), strValue);
			arrErrors.Add(szError);
		}

		//Column N - Payer Address Line 2
		szTemp = si->CompanySettings.BillingInformation.Address2;
		if (!szTemp.IsEmpty())
		{
			if (IsValidAddress(szTemp, 35))
				AppendCsv(strBuffer, szTemp);
			else
			{
				strValue.Format(
								_T("Invalid format in Payer Address Line 2 of '%s' for '%s'. "
								   "Format should comply to the following: 1) Have a max length of 35 characters, "
								   "2) Only be alpha-numeric characters [A-Z, 0-9] and following special characters: [hyphen ( - ), ampersand ( & ), apostrophe ( ' ), period ( . ), hash ( # ), space ( )], and "
								   "3) Not contain leading, trailing, or adjacent spaces."),
									szTemp,
									sz_PayerName + " for Vendor - " + sz_RecipientName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("N"), strValue);
				arrErrors.Add(szError);
			}
		}
		else
			AppendCsv(strBuffer, _T(""));

		//Column O - Payer City/Town*
		szTemp = si->CompanySettings.BillingInformation.City;
		if (!szTemp.IsEmpty())
		{
			if (IsValidCity(szTemp, 40))
				AppendCsv(strBuffer, szTemp);
			else
			{
				strValue.Format(
								_T("Invalid format in Payer City/Town of '%s' for '%s'. "
								   "Format should comply to the following: 1) Have a max length of 40 characters, "
								   "2) Only be alpha-numeric characters [A-Z, 0-9] and following special characters: [hyphen ( - ), ampersand ( & ), apostrophe ( ' ), period ( . ), space ( )], and "
								   "3) Not contain leading, trailing, or adjacent spaces."),
									szTemp,
									sz_PayerName + " for Vendor - " + sz_RecipientName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("O"), strValue);
				arrErrors.Add(szError);
			}
		}
		else
		{
			strValue.Format(_T("Payer City/Town should not be empty for '%s'"), sz_PayerName + " for Vendor - " + sz_RecipientName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("O"), strValue);
			arrErrors.Add(szError);
		}

		//Column P - Payer State / Province / Territory*
		szTemp = si->CompanySettings.BillingInformation.State;
		if (!szTemp.IsEmpty())
		{
			bool bValidState = IsValidState(szTemp);
			if (bValidState)
			{
				AppendCsv(strBuffer, szTemp);
			}
			else
			{
				strValue.Format(
								_T("Invalid format in Payer State/Province/Territory of '%s' for '%s'. "
								   "Format should comply to the following: 1)Domestic: Must be entered as their 2 letter abbreviation (A-Z), "
								   "2) Foreign: Have a max length of 17 characters, "
								   "3) Only be alpha-numeric characters [A-Z, 0-9] and following special characters: [space ( )], and "
								   "4) Not contain leading, trailing, or adjacent spaces."),
									szTemp,
									sz_PayerName + " for Vendor - " + sz_RecipientName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("P"), strValue);
				arrErrors.Add(szError);
			}
		}
		else
		{
			strValue.Format(_T("Payer State/Province/Territory should not be empty for '%s'"), sz_PayerName + " for Vendor - " + sz_RecipientName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("P"), strValue);
			arrErrors.Add(szError);
		}

		//Column Q - Payer ZIP/Postal Code*
		szTemp = si->CompanySettings.BillingInformation.PostalCode;		
		if (!szTemp.IsEmpty())
		{
			bool bPayerValid = IsValidPostalCode(szTemp);
			if(bPayerValid)
				AppendCsv(strBuffer, szTemp);
			else
			{
				strValue.Format(
								_T("Invalid format in Payer ZIP/Postal Code of '%s' for '%s'. "
								   "Format should comply to the following: 1) Domestic: ZIP/Postal Code must be exactly 5, 9 or 12 digits, "
								   "or Foreign: Have a max length of 9 characters, "
								   "2) Only be alpha-numeric characters [A-Z, 0-9]."),
									szTemp,
									sz_PayerName + " for Vendor - " + sz_RecipientName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("Q"), strValue);
				arrErrors.Add(szError);
			}
		}
		else
		{
			strValue.Format(_T("Payer ZIP/Postal Code should not be empty for '%s'"), sz_PayerName + " for Vendor - " + sz_RecipientName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("Q"), strValue);
			arrErrors.Add(szError);
		}

		//Column R - Payer Phone Type
		szTemp = (si->CompanySettings.BillingInformation.Country == "United States" ? _T("D") : _T("I"));
		AppendCsv(strBuffer, szTemp);

		//Column S - Payer Phone
		szTemp = si->CompanySettings.Phone;
		AppendCsv(strBuffer, szTemp);
				
		//-----------------------------------------------
		//Column T - Recipient TIN Type
		szTemp = (clsDR.XMLBOOL(_T("bol1099Flag"), false) ? _T("SSN") : _T("EIN"));
		AppendCsv(strBuffer, szTemp);

		//Column U - Recipient Taxpayer ID Number
		if (szTemp == "SSN")
		{
			// Format SSN (XXX-XX-XXXX)
			szTemp = clsDR.XMLStr(_T("strFederalIDNumber"), false);
			//convertion & validate ID
			std::string idNumber = CT2A(szTemp);
			std::regex ssnPattern(R"(\d{3}-\d{2}-\d{4})");

			if (!std::regex_match(idNumber, ssnPattern) && !szTemp.IsEmpty())
			{
				strValue.Format(_T("Invalid Taxpayer ID Pattern for Recipient '%s'"), sz_RecipientName + " - " + sz_ColName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("U"), strValue);
				arrErrors.Add(szError);
			}
		}
		else if (szTemp == "EIN")
		{
			// Format EIN (XX-XXXXXXX)
			szTemp = si->MagneticMediaConstants.EmployerID;
			//convertion
			std::string idNumber = CT2A(szTemp);
			std::regex einPattern(R"(\d{2}-\d{7})");

			if (!std::regex_match(idNumber, einPattern) && !szTemp.IsEmpty())
			{
				strValue.Format(_T("Invalid Taxpayer ID Pattern for Recipient '%s'"), sz_RecipientName + " - " + sz_ColName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("U"), strValue);
				arrErrors.Add(szError);
			}
		}
		else if (szTemp == "UND")
		{
			// Format UND (XXXXXXXXX)
			szTemp = clsDR.XMLStr(_T("strFederalIDNumber"), false);
			szTemp.Replace(_T("-"), _T(""));
		}
		AppendCsv(strBuffer, szTemp);

		//Column V - Recipient Name Type*
		szTemp = (clsDR.XMLBOOL(_T("bol1099Flag"), false) ? _T("I") : _T("B"));
		AppendCsv(strBuffer, szTemp);
		szTempNameType = szTemp;

		//Column W - Recipient Business or Entity Name Line 1
		CString recipientName = clsDR.XMLStr(_T("strVendorName"), false);
		CString colX, colY;

		if (recipientName.GetLength() <= 40)
		{
			// Fits entirely in Column X
			colX = recipientName;
			colY = _T("");   // blank
		}
		else
		{
			// Need to split into Column X and Y
			int splitPos = 40;

			// Move back to last space before 40 chars
			while (splitPos > 0 && recipientName[splitPos] != ' ')
			{
				splitPos--;
			}

			if (splitPos == 0)
			{
				// no space found, just split hard at 40
				splitPos = 40;
			}

			colX = recipientName.Left(splitPos).Trim();
			colY = recipientName.Mid(splitPos).Trim();
		}

		if (szTempNameType == "B")
		{
			if (IsValidEntityName(colX))
				AppendCsv(strBuffer, colX);
			else
				AppendCsv(strBuffer, _T(""));

			//Column X - Recipient Business or Entity Name Line 2
			if (IsValidEntityName(colY))
				AppendCsv(strBuffer, colY);
			else
				AppendCsv(strBuffer, _T(""));
		}

		//Column Y - Recipient First Name
		if (szTempNameType == "I")
		{
			szTemp = clsDR.XMLStr(_T("strVendorName"), false);

			if (IsValidPersonName(szTemp))
				AppendCsv(strBuffer, szTemp);
			else
				AppendCsv(strBuffer, _T(""));
		}
		else
			AppendCsv(strBuffer, _T(""));


		//Column Z - Recipient Middle Name
		szTemp = _T("");

		if (IsValidPersonName(szTemp) && !szTemp.IsEmpty())
			AppendCsv(strBuffer, szTemp);
		else
			AppendCsv(strBuffer, _T(""));


		//Column AA - Recipient Last Name (Surname)
		if (szTempNameType == "I")
		{
			szTemp = _T("");

			if (IsValidPersonName(szTemp) && !szTemp.IsEmpty())
				AppendCsv(strBuffer, szTemp);
			else
				AppendCsv(strBuffer, _T(""));
		}
		else
			AppendCsv(strBuffer, _T(""));

		//Column AB - Recipient Suffix
		szTemp = _T("");
		if (IsValidPersonName(szTemp) && !szTemp.IsEmpty())
			AppendCsv(strBuffer, szTemp);
		else
			AppendCsv(strBuffer, _T(""));

		//Column AC - Recipient Country*
		szTemp = si->CompanySettings.BillingInformation.Country;
		CString RecipientCountryCode = GetCountryCode(szTemp);

		if (!RecipientCountryCode.IsEmpty())
			AppendCsv(strBuffer, RecipientCountryCode);
		else
		{
			strValue.Format(_T("Recipient Country should not be empty for Vendor '%s'"), sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AC"), strValue);
			arrErrors.Add(szError);
		}

		//Column AD - Recipient Address Line 1 *
		szTemp = clsDR.XMLStr(_T("strAddress1"), false);
		if(!szTemp.IsEmpty())
		{
			if (IsValidRecipientAddress(szTemp, 35))
				AppendCsv(strBuffer, szTemp);
			else
			{
				strValue.Format(
								_T("Invalid format in Recipient Address Line 1 of '%s' for Vendor '%s'. "
								   " Format should comply to the following: 1) Have a max length of 35 characters, "
								   "2) Only be alpha-numeric characters [A-Z, 0-9] and following special characters: [hyphen ( - ), slash ( / ), space ( )], and "
								   "3) Not contain leading, trailing, or adjacent spaces."),
									szTemp,
									sz_RecipientName + " - " + sz_ColName);				
				szError.Format(_T("%d|%s|%s"), TransCount, _T("AD"), strValue);
				arrErrors.Add(szError);
			}
		}
		else
		{
			strValue.Format(_T("Recipient Address Line 1 should not be empty for Vendor '%s'"), sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AD"), strValue);
			arrErrors.Add(szError);
		}

		//Column AE - Recipient Address Line 2
		szTemp = clsDR.XMLStr(_T("strAddress2"), false);
		if (!szTemp.IsEmpty())
		{
			if (IsValidRecipientAddress(szTemp, 35))
				AppendCsv(strBuffer, szTemp);
			else
			{		
				strValue.Format(
								_T("Invalid format in Recipient Address Line 2 of '%s' for Vendor '%s'. "
								   " Format should comply to the following: 1) Have a max length of 35 characters, "
								   "2) Only be alpha-numeric characters [A-Z, 0-9] and following special characters: [hyphen ( - ), slash ( / ), space ( )], and "
								   "3) Not contain leading, trailing, or adjacent spaces."),
									szTemp,
									sz_RecipientName + " - " + sz_ColName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("AE"), strValue);
				arrErrors.Add(szError);
			}
		}
		else
			AppendCsv(strBuffer, _T(""));

		//Column AF - Recipient City/Town*
		szTemp = clsDR.XMLStr(_T("strCity"), false);
		if (!szTemp.IsEmpty())
		{
			if (IsValidRecipientCity(szTemp, 40))
				AppendCsv(strBuffer, szTemp);
			else
			{
				strValue.Format(
								_T("Invalid format in Recipient City/Town of '%s' for Vendor '%s'. "
								   "Format should comply to the following: 1) Have a max length of 40 characters, "
								   "2) Only be alpha-numeric characters [A-Z, 0-9] and following special characters: [space ( )], and "
								   "3) Not contain leading, trailing, or adjacent spaces."),
									szTemp,
									sz_RecipientName + " - " + sz_ColName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("AF"), strValue);
				arrErrors.Add(szError);
			}
		}
		else
		{
			strValue.Format(_T("Recipient City/Town should not be empty for Vendor '%s'"), sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AF"), strValue);
			arrErrors.Add(szError);
		}

		//Column AG - Recipient State/Province/Territory*
		szTemp = clsDR.XMLStr(_T("strProvince"), false);
		if (!szTemp.IsEmpty())
		{
			bool bValidState = IsValidState(szTemp);
			if (bValidState)
			{
				AppendCsv(strBuffer, szTemp);
			}
			else
			{
				strValue.Format(
								_T("Invalid format in Recipient State/Province/Territory of '%s' for Vendor '%s'. "
								   "Format should comply to the following: 1) Domestic: Must be entered as their 2 letter abbreviation (A-Z), "
								   "or Foreign: Have a max length of 17 characters, "
								   "2) Only be alpha-numeric characters [A-Z, 0-9] and following special characters: [space ( )], and "
								   "3) Not contain leading, trailing, or adjacent spaces."),
									szTemp,
									sz_RecipientName + " - " + sz_ColName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("AG"), strValue);
				arrErrors.Add(szError);
			}
		}
		else
		{
			strValue.Format(_T("Recipient State/Province/Territory should not be empty for Vendor '%s'"), sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AG"), strValue);
			arrErrors.Add(szError);
		}
		 
		//Column AH - Recipient ZIP/Postal Code*
		 szTemp = clsDR.XMLStr(_T("strPostalCode"), false);		 
		 if (!szTemp.IsEmpty())
		 {
			 bool bRecipientValid = IsValidPostalCode(szTemp);
			 if(bRecipientValid)
				 AppendCsv(strBuffer, szTemp);
			 else
			 {
				 strValue.Format(
								_T("Invalid format in Recipient ZIP/Postal Code of '%s' for Vendor '%s'. "
								   "Format should comply to the following: 1) Domestic: ZIP/Postal Code must be exactly 5, 9 or 12 digits, "
								   "or Foreign: Have a max length of 9 characters, "
								   "2) Only be alpha-numeric characters [A-Z, 0-9]."),
									szTemp,
									sz_RecipientName + " - " + sz_ColName);
				 szError.Format(_T("%d|%s|%s"), TransCount, _T("AH"), strValue);
				 arrErrors.Add(szError);
			 }
		 }
		 else
		 {
			 strValue.Format(_T("Recipient ZIP/Postal Code should not be empty for Vendor '%s'"), sz_RecipientName + " - " + sz_ColName);
			 szError.Format(_T("%d|%s|%s"), TransCount, _T("AH"), strValue);
			 arrErrors.Add(szError);			 
		 }
		 
		//Column AI - Office Code
		 AppendCsv(strBuffer, _T(""));
		//Column AJ -Form Account Number
		 AppendCsv(strBuffer, _T(""));

		//Column AK - FATCA Filing Requirements
		szTemp = (clsDR.XMLBOOL(_T("bolFATCAFiling"), false) ? _T("Y") : _T("N"));
		AppendCsv(strBuffer, szTemp);

		//Column AL - 2nd TIN Notice
		szTemp = (clsDR.XMLBOOL(_T("bol2ndTIN"), false) ? _T("Y") : _T("N"));		
		AppendCsv(strBuffer, szTemp);

		CString strTemp;
		CString strAmount;
		strAmount.Empty();
		//Column AM - Box 1
		if (isValidBoxAmount(dType1Amount))
		{
			strAmount.Format(_T("%.2f"), dType1Amount);
			AppendCsv(strBuffer, strAmount);
		}
		else
		{
			strTemp.Format(_T("%.f"), dType1Amount);			
			strValue.Format(
						   _T("Invalid format in Box 1 - Rents of '%s' for Vendor '%s'. "
							  "Format should be between $0 and $999,999."
							  " 1) Cannot be negative"
							  " 2) Digits only except 1 decimal is allowed"
							  " 3) Must have no more than two digits after the decimal.Check for long"
							  " numbers in money amount fields that may have been formatted in"
							  " scientific notation.For example : 1.045E+5."),
							   strTemp,
							   sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AM"), strValue);
			arrErrors.Add(szError);
		}

		//Column AN - Box 2
		strAmount.Empty();
		if (isValidBoxAmount(dType2Amount))
		{
			strAmount.AppendFormat(_T("%.2f"), dType2Amount);
			AppendCsv(strBuffer, strAmount);
		}
		else
		{
			strTemp.Format(_T("%.f"), dType2Amount);
			strValue.Format(
						   _T("Invalid format in Box 2 - Royalties of '%s' for Vendor '%s'. "
							  "Format should be between $0 and $999,999."
							  " 1) Cannot be negative"
							  " 2) Digits only except 1 decimal is allowed"
							  " 3) Must have no more than two digits after the decimal.Check for long"
							  " numbers in money amount fields that may have been formatted in"
							  " scientific notation.For example : 1.045E+5."),
							   strTemp,
							   sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AN"), strValue);
			arrErrors.Add(szError);
		}
		//Column AO - Box 3
		strAmount.Empty();
		if (isValidBoxAmount(dType3Amount))
		{
			strAmount.AppendFormat(_T("%.2f"), dType3Amount);
			AppendCsv(strBuffer, strAmount);
		}
		else
		{
			strTemp.Format(_T("%.f"), dType3Amount);
			strValue.Format(
						   _T("Invalid format in Box 3 - Other income of '%s' for Vendor '%s'. "
							  "Format should be between $0 and $999,999."
							  " 1) Cannot be negative"
							  " 2) Digits only except 1 decimal is allowed"
							  " 3) Must have no more than two digits after the decimal.Check for long"
							  " numbers in money amount fields that may have been formatted in"
							  " scientific notation.For example : 1.045E+5."),
							   strTemp,
							   sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AO"), strValue);
			arrErrors.Add(szError);
		}
		//Column AP - Box 4
		strAmount.Empty();
		if (isValidBoxAmount(dType4Amount))
		{
			strAmount.AppendFormat(_T("%.2f"), dType4Amount);
			AppendCsv(strBuffer, strAmount);
		}
		else
		{
			strTemp.Format(_T("%.f"), dType4Amount);
			strValue.Format(
						   _T("Invalid format in Box 4 - Federal income tax withheld of '%s' for Vendor '%s'. "
							  "Format should be between $0 and $999,999."
							  " 1) Cannot be negative"
							  " 2) Digits only except 1 decimal is allowed"
							  " 3) Must have no more than two digits after the decimal.Check for long"
							  " numbers in money amount fields that may have been formatted in"
							  " scientific notation.For example : 1.045E+5."),
							   strTemp,
							   sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AP"), strValue);
			arrErrors.Add(szError);
		}
		//Column AQ - Box 5
		strAmount.Empty();
		if (isValidBoxAmount(dType5Amount))
		{
			strAmount.AppendFormat(_T("%.2f"), dType5Amount);
			AppendCsv(strBuffer, strAmount);
		}
		else
		{
			strTemp.Format(_T("%.f"), dType5Amount);
			strValue.Format(
						  _T("Invalid format in Box 5 - Fishing boat proceeds of '%s' for Vendor '%s'. "
							 "Format should be between $0 and $999,999."
							 " 1) Cannot be negative"
							 " 2) Digits only except 1 decimal is allowed"
							 " 3) Must have no more than two digits after the decimal.Check for long"
							 " numbers in money amount fields that may have been formatted in"
							 " scientific notation.For example : 1.045E+5."),
							   strTemp,
							   sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AQ"), strValue);
			arrErrors.Add(szError);
		}
		//Column AR - Box 6
		strAmount.Empty();
		if (isValidBoxAmount(dType6Amount))
		{
			strAmount.AppendFormat(_T("%.2f"), dType6Amount);
			AppendCsv(strBuffer, strAmount);
		}
		else
		{
			strTemp.Format(_T("%.f"), dType6Amount);
			strValue.Format(
						   _T("Invalid format in Box 6 - Medical and health care payments of '%s' for Vendor '%s'. "
							  "Format should be between $0 and $999,999."
							  " 1) Cannot be negative"
							  " 2) Digits only except 1 decimal is allowed"
							  " 3) Must have no more than two digits after the decimal.Check for long"
							  " numbers in money amount fields that may have been formatted in"
							  " scientific notation.For example : 1.045E+5."),
							   strTemp,
							   sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AR"), strValue);
			arrErrors.Add(szError);
		}
		//Column AS - Box 7
		szTemp = (clsDR.XMLBOOL(_T("bol5000DirSales"), false) ? _T("Y") : _T("N"));
		AppendCsv(strBuffer, szTemp);
		//Column AT - Box 8
		strAmount.Empty();
		if (isValidBoxAmount(dType8Amount))
		{
			strAmount.AppendFormat(_T("%.2f"), dType8Amount);
			AppendCsv(strBuffer, strAmount);
		}
		else
		{
			strTemp.Format(_T("%.f"), dType8Amount);
			strValue.Format(
						   _T("Invalid format in Box 8 - Substitute payments in lieu of dividends or interest of '%s' for Vendor '%s'. "
							  "Format should be between $0 and $999,999."
							  " 1) Cannot be negative"
							  " 2) Digits only except 1 decimal is allowed"
							  " 3) Must have no more than two digits after the decimal.Check for long"
							  " numbers in money amount fields that may have been formatted in"
							  " scientific notation.For example : 1.045E+5."),
							   strTemp,
							   sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AT"), strValue);
			arrErrors.Add(szError);
		}
		//Column AU - Box 9
		strAmount.Empty();
		if (isValidBoxAmount(dTypeAAmount))
		{
			strAmount.AppendFormat(_T("%.2f"), dTypeAAmount);
			AppendCsv(strBuffer, strAmount);
		}
		else
		{
			strTemp.Format(_T("%.f"), dTypeAAmount);
			strValue.Format(
						   _T("Invalid format in Box 9 - Crop insurance proceeds of '%s' for Vendor '%s'. "
							  "Format should be between $0 and $999,999."
							  " 1) Cannot be negative"
							  " 2) Digits only except 1 decimal is allowed"
							  " 3) Must have no more than two digits after the decimal.Check for long"
							  " numbers in money amount fields that may have been formatted in"
							  " scientific notation.For example : 1.045E+5."),
							   strTemp,
							   sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AU"), strValue);
			arrErrors.Add(szError);
		}
		//Column AV - Box 10
		strAmount.Empty();
		if (isValidBoxAmount(dTypeCAmount))
		{
			strAmount.AppendFormat(_T("%.2f"), dTypeCAmount);
			AppendCsv(strBuffer, strAmount);
		}
		else
		{
			strTemp.Format(_T("%.f"), dTypeCAmount);
			strValue.Format(
						   _T("Invalid format in Box 10 - Gross proceeds paid to an attorney of '%s' for Vendor '%s'. "
							  "Format should be between $0 and $999,999."
							  " 1) Cannot be negative"
							  " 2) Digits only except 1 decimal is allowed"
							  " 3) Must have no more than two digits after the decimal.Check for long"
							  " numbers in money amount fields that may have been formatted in"
							  " scientific notation.For example : 1.045E+5."),
							   strTemp,
							   sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AV"), strValue);
			arrErrors.Add(szError);
		}
		//Column AW - Box 11
		strAmount.Empty();
		if (isValidBoxAmount(dTypeFAmount))
		{
			strAmount.AppendFormat(_T("%.2f"), dTypeFAmount);
			AppendCsv(strBuffer, strAmount);
		}
		else
		{
			strTemp.Format(_T("%.f"), dTypeFAmount);
			strValue.Format(
						   _T("Invalid format in Box 11 - Fish purchased for resale of '%s' for Vendor '%s'. "
							  "Format should be between $0 and $999,999."
							  " 1) Cannot be negative"
							  " 2) Digits only except 1 decimal is allowed"
							  " 3) Must have no more than two digits after the decimal.Check for long"
							  " numbers in money amount fields that may have been formatted in"
							  " scientific notation.For example : 1.045E+5."),
							   strTemp,
							   sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AW"), strValue);
			arrErrors.Add(szError);
		}
		//Column AX - Box 12
		strAmount.Empty();
		if (isValidBoxAmount(dTypeDAmount))
		{
			strAmount.AppendFormat(_T("%.2f"), dTypeDAmount);
			AppendCsv(strBuffer, strAmount);
		}
		else
		{
			strTemp.Format(_T("%.f"), dTypeDAmount);
			strValue.Format(
						   _T("Invalid format in Box 12 - Section 409A deferrals of '%s' for Vendor '%s'. "
							  "Format should be between $0 and $999,999."
							  " 1) Cannot be negative"
							  " 2) Digits only except 1 decimal is allowed"
							  " 3) Must have no more than two digits after the decimal.Check for long"
							  " numbers in money amount fields that may have been formatted in"
							  " scientific notation.For example : 1.045E+5."),
							   strTemp,
							   sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AX"), strValue);
			arrErrors.Add(szError);
		}

		//Column AY - Box 14
		strAmount.Empty();
		if (_tstoi(m_szPaymentYear) < PAYMENT_YEAR_2025)
		{
			if (isValidBoxAmount(dTypeBAmount))
			{
				strAmount.AppendFormat(_T("%.2f"), dTypeBAmount);
				AppendCsv(strBuffer, strAmount);
			}
			else
			{
				strTemp.Format(_T("%.f"), dTypeBAmount);
				strValue.Format(
							   _T("Invalid format in Box 14 - Excess golden parachute payments of '%s' for Vendor '%s'. "
								  "Format should be between $0 and $999,999."
								  " 1) Cannot be negative"
								  " 2) Digits only except 1 decimal is allowed"
								  " 3) Must have no more than two digits after the decimal.Check for long"
								  " numbers in money amount fields that may have been formatted in"
								  " scientific notation.For example : 1.045E+5."),
								   strTemp,
								   sz_RecipientName + " - " + sz_ColName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("AY"), strValue);
				arrErrors.Add(szError);
			}

			//Column AZ - Box 15
			strAmount.Empty();
			if (isValidBoxAmount(dTypeEAmount))
			{
				strAmount.AppendFormat(_T("%.2f"), dTypeEAmount);
				AppendCsv(strBuffer, strAmount);
			}
			else
			{
				strTemp.Format(_T("%.f"), dTypeEAmount);
				strValue.Format(
							   _T("Invalid format in Box 15 - Nonqualified deferred compensation of '%s' for Vendor '%s'. "
								  "Format should be between $0 and $999,999."
								  " 1) Cannot be negative"
								  " 2) Digits only except 1 decimal is allowed"
								  " 3) Must have no more than two digits after the decimal.Check for long"
								  " numbers in money amount fields that may have been formatted in"
								  " scientific notation.For example : 1.045E+5."),
								   strTemp,
								   sz_RecipientName + " - " + sz_ColName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("AZ"), strValue);
				arrErrors.Add(szError);
			}
		}
		else
		{
			//Column AZ - Box 15
			strAmount.Empty();
			if (isValidBoxAmount(dTypeEAmount))
			{
				strAmount.AppendFormat(_T("%.2f"), dTypeEAmount);
				AppendCsv(strBuffer, strAmount);
			}
			else
			{
				strTemp.Format(_T("%.f"), dTypeEAmount);
				strValue.Format(
							   _T("Invalid format in Box 15 - Nonqualified deferred compensation of '%s' for Vendor '%s'. "
								  "Format should be between $0 and $999,999."
								  " 1) Cannot be negative"
								  " 2) Digits only except 1 decimal is allowed"
								  " 3) Must have no more than two digits after the decimal.Check for long"
								  " numbers in money amount fields that may have been formatted in"
								  " scientific notation.For example : 1.045E+5."),
								   strTemp,
								   sz_RecipientName + " - " + sz_ColName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("AZ"), strValue);
				arrErrors.Add(szError);
			}
		}
				
		if (m_bchkCombinedFederalState)
		{
			//Column BA - Combined Federal/State Filing(CF / SF) Check all states that apply
				szTemp = clsDR.XMLStr(_T("strProvince"), false);
			if (!szTemp.IsEmpty() && szTemp.GetLength() == 2)
				AppendCsv(strBuffer, szTemp);
			else
				AppendCsv(strBuffer, _T(""));
			
			double strTaxWithheld = clsDR.XMLDbl(_T("curStateTaxWithheld"), false);
			if (strTaxWithheld > 0)
			{
				//Column BB - State 1
				szTemp = clsDR.XMLStr(_T("strProvince"), false);
				if (!szTemp.IsEmpty() && szTemp.GetLength() == 2)
					AppendCsv(strBuffer, szTemp);
				else
					AppendCsv(strBuffer, _T(""));

				//Column BC - State 1 - State Tax Withheld
				szTemp = clsDR.XMLStr(_T("curStateTaxWithheld"), false);
				if (!szTemp.IsEmpty())
					AppendCsv(strBuffer, szTemp);
				else
					AppendCsv(strBuffer, _T(""));
			}
			else
			{
				//Column BD - State 1
				AppendCsv(strBuffer, _T(""));
				//Column BE - State 1 - State Tax Withheld
				AppendCsv(strBuffer, _T(""));
			}

			//Column BF - State 1 - State/Payer state number
			AppendCsv(strBuffer, _T(""));
			//Column BG - State 1 - State income
			AppendCsv(strBuffer, _T(""));
			//Column BH - State 1 - Local income tax withheld
			AppendCsv(strBuffer, _T(""));
			//Column BI - State 1 - Special Data Entries
			AppendCsv(strBuffer, _T(""));

			//Column BJ - State 2
			AppendCsv(strBuffer, _T(""));
			//Column BK - State 2 - State Tax Withheld			
			AppendCsv(strBuffer, _T(""));
			//Column BL - State 2 - State/Payer state number
			AppendCsv(strBuffer, _T(""));
			//Column BM - State 2 - State income
			AppendCsv(strBuffer, _T(""));
			//Column BN - State 2 - Local income tax withheld
			AppendCsv(strBuffer, _T(""));
			//Column BO - State 2 - Special Data Entries
			AppendCsv(strBuffer, _T(""),false);
		}
		else
		{
			//Column BA - Combined Federal/State Filing(CF / SF) Check all states that apply
			AppendCsv(strBuffer, _T(""));
			//Column BB - State 1
			AppendCsv(strBuffer, _T(""));
			//Column BC - State 1 - State Tax Withheld			
			AppendCsv(strBuffer, _T(""));
			//Column BD - State 1 - State/Payer state number
			AppendCsv(strBuffer, _T(""));
			//Column BE - State 1 - State income
			AppendCsv(strBuffer, _T(""));
			//Column BF - State 1 - Local income tax withheld
			AppendCsv(strBuffer, _T(""));
			//Column BG - State 1 - Special Data Entries
			AppendCsv(strBuffer, _T(""));

			//Column BH - State 2
			AppendCsv(strBuffer, _T(""));
			//Column BI - State 2 - State Tax Withheld			
			AppendCsv(strBuffer, _T(""));
			//Column BJ - State 2 - State/Payer state number
			AppendCsv(strBuffer, _T(""));
			//Column BK - State 2 - State income
			AppendCsv(strBuffer, _T(""));
			//Column BL - State 2 - Local income tax withheld
			AppendCsv(strBuffer, _T(""));
			//Column BM - State 2 - Special Data Entries
			AppendCsv(strBuffer, _T(""), false);
		}

		strBuffer.Append(_T("\r\n"));
	}

	// Validation error msg
	if (arrErrors.GetSize() >= 1)
	{
		Do1099ErrorReport(arrErrors);
	}

	while (!FileWrite(strBuffer) && !m_bCancel)
	{
		//Do this until either successful, or canceled!
	}

	return !m_bCancel;
}

//RS - 11/19/2025 - PBI 64621 - Added IRIS
bool CRFV_APElectronicFile1099Std::Write_IRIS_1099_NEC()
{
	CString strBuffer;
	CString szTemp;
	CString szTempNameType;
	double dType1Amount = 0;	//Local Amount for Type 1
	double dType10Amount = 0;	//Local Amount for Type 10
	double dType4Amount = 0;	//Local Amount for Type 4
	CDataRow clsDR;
	CString szState;
	CString szFedStateCode;
	int nFedStCode;
	CStringArray	arrErrors;
	CString		szError;
	CString		sz_ColName;
	CString		sz_RecipientName;
	CString		sz_PayerName;
	CString		strValue;
	int			TransCount = 0;

	long lRowCount = m_dtBNecRecords.getRowCount();
	if (m_nServerStatus == SERVER_CONTINUE)
		lRowCount--;

	// Append column headers (FIRST ROW)
// -------------------------
	AppendCsv(strBuffer, _T("Form Type"));										//Column A
	AppendCsv(strBuffer, _T("Tax Year"));										//Column B
	AppendCsv(strBuffer, _T("Payer TIN Type"));									//Column C
	AppendCsv(strBuffer, _T("Payer Taxpayer ID Number"));						//Column D
	AppendCsv(strBuffer, _T("Payer Name Type"));								//Column E
	AppendCsv(strBuffer, _T("Payer Business or Entity Name Line 1"));			//Column F
	AppendCsv(strBuffer, _T("Payer Business or Entity Name Line 2"));			//Column G
	AppendCsv(strBuffer, _T("Payer First Name"));								//Column H
	AppendCsv(strBuffer, _T("Payer Middle Name"));								//Column I
	AppendCsv(strBuffer, _T("Payer Last Name (Surname)"));						//Column J
	AppendCsv(strBuffer, _T("Payer Suffix"));									//Column K
	AppendCsv(strBuffer, _T("Payer Country"));									//Column L
	AppendCsv(strBuffer, _T("Payer Address Line 1"));							//Column M
	AppendCsv(strBuffer, _T("Payer Address Line 2"));							//Column N
	AppendCsv(strBuffer, _T("Payer City/Town"));								//Column O
	AppendCsv(strBuffer, _T("Payer State/Province/Territory"));					//Column P
	AppendCsv(strBuffer, _T("Payer ZIP/Postal Code"));							//Column Q
	AppendCsv(strBuffer, _T("Payer Phone Type"));								//Column R
	AppendCsv(strBuffer, _T("Payer Phone"));									//Column S
	AppendCsv(strBuffer, _T("Recipient TIN Type"));								//Column T
	AppendCsv(strBuffer, _T("Recipient Taxpayer ID Number"));					//Column U
	AppendCsv(strBuffer, _T("Recipient Name Type"));							//Column V
	AppendCsv(strBuffer, _T("Recipient Business or Entity Name Line 1"));		//Column W
	AppendCsv(strBuffer, _T("Recipient Business or Entity Name Line 2"));		//Column X
	AppendCsv(strBuffer, _T("Recipient First Name"));							//Column Y
	AppendCsv(strBuffer, _T("Recipient Middle Name"));							//Column Z
	AppendCsv(strBuffer, _T("Recipient Last Name (Surname)"));					//Column AA
	AppendCsv(strBuffer, _T("Recipient Suffix"));								//Column AB
	AppendCsv(strBuffer, _T("Recipient Country"));								//Column AC
	AppendCsv(strBuffer, _T("Recipient Address Line 1"));						//Column AD
	AppendCsv(strBuffer, _T("Recipient Address Line 2"));						//Column AE
	AppendCsv(strBuffer, _T("Recipient City/Town"));							//Column AF
	AppendCsv(strBuffer, _T("Recipient State/Province/Territory"));				//Column AG
	AppendCsv(strBuffer, _T("Recipient ZIP/Postal Code"));						//Column AH
	AppendCsv(strBuffer, _T("Office Code"));									//Column AI
	AppendCsv(strBuffer, _T("Form Account Number"));							//Column AJ
	AppendCsv(strBuffer, _T("2nd TIN Notice"));									//Column AK
	AppendCsv(strBuffer, _T("Box 1 - Nonemployee Compensation"));				//Column AL
	AppendCsv(strBuffer, _T("Box 2 - Payer made direct sales totaling $5,000 or more of consumer products to a recipient for resale"));	//Column AM
	if (_tstoi(m_szPaymentYear) >= PAYMENT_YEAR_2025)
	{
		AppendCsv(strBuffer, _T("Box 3 - Excess golden parachute payments"));	//Column AN for 2025
	}
	AppendCsv(strBuffer, _T("Box 4 - Federal income tax withheld"));			//Column AN or Column AO for 2025
	AppendCsv(strBuffer, _T("Combined Federal/State Filing"));					//Column AO or Column AP for 2025
	AppendCsv(strBuffer, _T("State 1"));										//Column AP or Column AQ for 2025
	AppendCsv(strBuffer, _T("State 1 - State tax withheld"));					//Column AQ or Column AR for 2025
	AppendCsv(strBuffer, _T("State 1 - State/Payer state number"));				//Column AR or Column AS for 2025
	AppendCsv(strBuffer, _T("State 1 - State income"));							//Column AS or Column AT for 2025
	AppendCsv(strBuffer, _T("State 1 - Local income tax withheld"));			//Column AT or Column AU for 2025
	AppendCsv(strBuffer, _T("State 1 - Special Data Entries"));					//Column AU or Column AV for 2025
	AppendCsv(strBuffer, _T("State 2"));										//Column AV or Column AW for 2025
	AppendCsv(strBuffer, _T("State 2 - State tax withheld"));					//Column AW or Column AX for 2025
	AppendCsv(strBuffer, _T("State 2 - State/Payer state number"));				//Column AX or Column AY for 2025
	AppendCsv(strBuffer, _T("State 2 - State income"));							//Column AY or Column AZ for 2025
	AppendCsv(strBuffer, _T("State 2 - Local income tax withheld"));			//Column AZ or Column BA for 2025
	AppendCsv(strBuffer, _T("State 2 - Special Data Entries"), false);			//Column BA or Column BB for 2025

	// End of header row
	strBuffer.AppendFormat(_T("\r\n"));
	//------------------------

	for (int nRow = 0; nRow < lRowCount; nRow++)
	{
		m_dtBNecRecords.GetRow(nRow, &clsDR);

		szState = _T("");
		szFedStateCode = _T("");
		nFedStCode = 0;
		szState = clsDR.XMLStr(_T("strProvince"), false);
		szFedStateCode = GetCombCode(szState);

		dType1Amount = clsDR.XMLDbl(_T("curAP1099PMT1"), false);
		dType4Amount = clsDR.XMLDbl(_T("curAP1099PMT4"), false);
		dType10Amount = clsDR.XMLDbl(_T("curAP1099PMT10"), false);

		sz_ColName = clsDR.XMLStr(_T("strVendorName"), false);
		sz_RecipientName = clsDR.XMLStr(_T("strVendor"), false);
		sz_PayerName = si->MagneticMediaConstants.Transmittername;

		// Append each row
		//Column A - Form Type
		AppendCsv(strBuffer, _T("1099-NEC"));
		//Column B - Tax Year
		AppendCsv(strBuffer, m_szPaymentYear);
		//Column C - Payer TIN Type*
		szTemp = (clsDR.XMLBOOL(_T("bol1099Flag"), false) ? _T("SSN") : _T("EIN"));
		AppendCsv(strBuffer, szTemp);

		//Column D - Payer Taxpayer ID Number*
		if (szTemp == "SSN")
		{
			// Format SSN (XXX-XX-XXXX)
			szTemp = clsDR.XMLStr(_T("strFederalIDNumber"), false);
			//conversion & validate ID
			std::string idNumber = CT2A(szTemp);
			std::regex ssnPattern(R"(\d{3}-\d{2}-\d{4})");

			if (!std::regex_match(idNumber, ssnPattern) && !szTemp.IsEmpty())
			{
				strValue.Format(_T("Invalid Taxpayer ID Pattern for Payer '%s'"), sz_PayerName + " for Vendor - " + sz_RecipientName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("D"), strValue);
				arrErrors.Add(szError);
			}
		}
		else if (szTemp == "EIN")
		{
			// Format EIN (XX-XXXXXXX)
			//szTemp = clsDR.XMLStr(_T("strFederalIDNumber"), false);
			szTemp = si->ModuleSettings.APSettings.FederalID1099;
			szTemp.Replace(_T("-"), _T(""));
			szTemp = szTemp.Left(2) + "-" + szTemp.Mid(2, 7);
			//conversion
			std::string idNumber = CT2A(szTemp);
			std::regex einPattern(R"(\d{2}-\d{7})");

			if (!std::regex_match(idNumber, einPattern) && !szTemp.IsEmpty())
			{
				strValue.Format(_T("Invalid Taxpayer ID Pattern for Payer '%s'"), sz_PayerName + " for Vendor - " + sz_RecipientName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("D"), strValue);
				arrErrors.Add(szError);
			}
		}
		AppendCsv(strBuffer, szTemp);

		//Column E - Payer Name Type*
		szTemp = (clsDR.XMLBOOL(_T("bol1099Flag"), false) ? _T("I") : _T("B"));
		AppendCsv(strBuffer, szTemp);
		szTempNameType = szTemp;

		//Column F - Payer Business or Entity Name Line 1
		CString fullName = si->MagneticMediaConstants.Transmittername;
		CString colF, colG;

		if (fullName.GetLength() <= 40)
		{
			//Fits entirely in Column F
			colF = fullName;
			colG = _T("");	//blank
		}
		else
		{
			//Need to split into Column F and G
			int splitPos = 40;

			//Move back to last space before 40 chars
			while (splitPos > 0 && fullName[splitPos] != ' ')
			{
				splitPos--;
			}

			if (splitPos == 0)
			{
				//No space found, just split hard at 40
				splitPos = 40;
			}

			colF = fullName.Left(splitPos).Trim();
			colG = fullName.Mid(splitPos).Trim();
		}


		if (szTempNameType == "B")
		{
			if (IsValidEntityName(colF))
				AppendCsv(strBuffer, colF);
			else
				AppendCsv(strBuffer, _T(""));

			//Column G - Payer Business or Entity Name Line 2
			if (IsValidEntityName(colG))
				AppendCsv(strBuffer, colG);
			else
				AppendCsv(strBuffer, _T(""));
		}

		//Column H - Payer First Name
		if (szTempNameType == "I")
		{
			szTemp = si->CompanySettings.Name;

			if (IsValidPersonName(szTemp))
				AppendCsv(strBuffer, szTemp);
			else
				AppendCsv(strBuffer, _T(""));
		}
		else
			AppendCsv(strBuffer, _T(""));

		//Column I - Payer Middle Name
		szTemp = si->CompanySettings.Name;

		if (IsValidPersonName(szTemp))
			AppendCsv(strBuffer, szTemp);
		else
			AppendCsv(strBuffer, _T(""));

		//Column J - Payer Last Name (Surname)
		if (szTempNameType == "I")
		{
			szTemp = si->CompanySettings.Name;

			if (IsValidPersonName(szTemp))
				AppendCsv(strBuffer, szTemp);
			else
				AppendCsv(strBuffer, _T(""));
		}
		else
			AppendCsv(strBuffer, _T(""));

		//Column K - Payer Suffix
		szTemp = si->CompanySettings.Name;
		if (IsValidPersonName(szTemp))
			AppendCsv(strBuffer, szTemp);
		else
			AppendCsv(strBuffer, _T(""));

		//Column L - Payer Country*
		szTemp = si->CompanySettings.BillingInformation.Country;
		CString PayerCountryCode = GetCountryCode(szTemp);
		if (!PayerCountryCode.IsEmpty())
			AppendCsv(strBuffer, PayerCountryCode);
		else
		{
			strValue.Format(_T("Payer Country should not be empty for '%s'"), sz_PayerName + " for Vendor - " + sz_RecipientName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("L"), strValue);
			arrErrors.Add(szError);
		}
		//Column M - Payer Address Line 1*
		szTemp = si->CompanySettings.BillingInformation.Address1;
		if (!szTemp.IsEmpty())
		{
			if (IsValidAddress(szTemp, 35))
				AppendCsv(strBuffer, szTemp);
			else
			{
				strValue.Format(
							_T("Invalid format in Payer Address Line 1 of '%s' for '%s'. "
							   "Format should comply to the following: 1) Have a max length of 35 characters, "
							   "2) Only be alpha-numeric characters [A-Z, 0-9] and following special characters: [hyphen ( - ), slash ( / ), space ( )], and "
							   "3) Not contain leading, trailing, or adjacent spaces."),
								szTemp,
								sz_PayerName + " for Vendor - " + sz_RecipientName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("M"), strValue);
				arrErrors.Add(szError);
			}
		}
		else
		{
			strValue.Format(_T("Payer Address Line 1 should not be empty for '%s'"), sz_PayerName + " for Vendor - " + sz_RecipientName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("M"), strValue);
			arrErrors.Add(szError);
		}

		//Column N - Payer Address Line 2
		szTemp = si->CompanySettings.BillingInformation.Address2;
		if (IsValidAddress(szTemp, 35))
			AppendCsv(strBuffer, szTemp);
		else
		{
			strValue.Format(
							_T("Invalid format in Payer Address Line 2 of '%s' for '%s'. "
							   "Format should comply to the following: 1) Have a max length of 35 characters, "
							   "2) Only be alpha-numeric characters [A-Z, 0-9] and following special characters: [hyphen ( - ), slash ( / ), space ( )], and "
							   "3) Not contain leading, trailing, or adjacent spaces."),
								szTemp,
								sz_PayerName + " for Vendor - " + sz_RecipientName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("N"), strValue);
			arrErrors.Add(szError);
		}

		//Column O -Payer City/Town*
		szTemp = si->CompanySettings.BillingInformation.City;
		if (!szTemp.IsEmpty())
		{
			if (IsValidCity(szTemp, 40))
				AppendCsv(strBuffer, szTemp);
			else
			{
				strValue.Format(
								_T("Invalid format in Payer City/Town of '%s' for '%s'. "
								   "Format should comply to the following: 1) Have a max length of 40 characters, "
								   "2) Only be alpha-numeric characters [A-Z, 0-9] and following special characters: [space ( )], and "
								   "3) Not contain leading, trailing, or adjacent spaces."),
									szTemp,
									sz_PayerName + " for Vendor - " + sz_RecipientName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("O"), strValue);
				arrErrors.Add(szError);
			}
		}
		else
		{
			strValue.Format(_T("Payer City/Town should not be empty for '%s'"), sz_PayerName + " for Vendor - " + sz_RecipientName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("O"), strValue);
			arrErrors.Add(szError);
		}

		//Column P - Payer State / Province / Territory*
		szTemp = si->CompanySettings.BillingInformation.State;
		if (!szTemp.IsEmpty())
		{
			bool bValidState = IsValidState(szTemp);
			if(bValidState)
				AppendCsv(strBuffer, szTemp);
			else
			{
				strValue.Format(
								_T("Invalid format in Payer State/Province/Territory of '%s' for '%s'. "
								   "Format should comply to the following: 1) Domestic: Must be entered as their 2 letter abbreviation (A-Z), "
								   "or Foreign: Have a max length of 17 characters, "
								   "2) Only be alpha-numeric characters [A-Z, 0-9] and following special characters: [space ( )], and "
								   "3) Not contain leading, trailing, or adjacent spaces."),
									szTemp,
									sz_PayerName + " for Vendor - " + sz_RecipientName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("P"), strValue);
				arrErrors.Add(szError);
			}
		}
		else
		{
			strValue.Format(_T("Payer State/Province/Territory should not be empty for '%s'"), sz_PayerName + " for Vendor - " + sz_RecipientName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("P"), strValue);
			arrErrors.Add(szError);
		}

		//Column Q - Payer ZIP/Postal Code*
		szTemp = si->CompanySettings.BillingInformation.PostalCode;		
		if (!szTemp.IsEmpty())
		{
			bool bPayerValid = IsValidPostalCode(szTemp);
			if(bPayerValid)
				AppendCsv(strBuffer, szTemp);
			else
			{
				strValue.Format(
								_T("Invalid format in Payer ZIP/Postal Code of '%s' for '%s'. "
								   "Format should comply to the following: 1) Domestic: ZIP/Postal Code must be exactly 5, 9 or 12 digits, "
								   "or Foreign: Have a max length of 9 characters, "
								   "3) Only be alpha-numeric characters [A-Z, 0-9]."),
									szTemp,
									sz_PayerName + " for Vendor - " + sz_RecipientName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("Q"), strValue);
				arrErrors.Add(szError);
			}
		}
		else
		{
			strValue.Format(_T("Payer ZIP/Postal Code should not be empty for '%s'"), sz_PayerName + " for Vendor - " + sz_RecipientName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("Q"), strValue);
			arrErrors.Add(szError);
		}

		//Column R - Payer Phone Type
		szTemp = (si->CompanySettings.BillingInformation.Country == "United States" ? _T("D") : _T("I"));
		AppendCsv(strBuffer, szTemp);

		//Column S - Payer Phone
		szTemp = si->CompanySettings.Phone;
		AppendCsv(strBuffer, szTemp);

		//Column U - Recipient TIN Type
		szTemp = (clsDR.XMLBOOL(_T("bol1099Flag"), false) ? _T("SSN") : _T("EIN"));
		AppendCsv(strBuffer, szTemp);
		
		//Column T - Recipient Taxpayer ID Number
		if (szTemp == "SSN")
		{
			//Format SSN (XXX-XX-XXXX)
			szTemp = clsDR.XMLStr(_T("strFederalIDNumber"), false);
			//conversion & validate ID
			std::string idNumber = CT2A(szTemp);
			std::regex ssnPattern(R"(\d{3}-\d{2}-\d{4})");

			if (!std::regex_match(idNumber, ssnPattern) && !szTemp.IsEmpty())
			{
				strValue.Format(_T("Invalid Taxpayer ID Pattern for Recipient Vendor '%s'"), sz_RecipientName + " - " + sz_ColName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("T"), strValue);
				arrErrors.Add(szError);
			}
		}
		else if (szTemp == "EIN")
		{
			//Format EIN (XX-XXXXXXX)
			szTemp = si->MagneticMediaConstants.EmployerID;
			//conversion
			std::string idNumber = CT2A(szTemp);
			std::regex einPattern(R"(\d{2}-\d{7})");

			if (!std::regex_match(idNumber, einPattern) && !szTemp.IsEmpty())
			{
				strValue.Format(_T("Invalid Taxpayer ID Pattern for Recipient Vendor '%s'"), sz_RecipientName + " - " + sz_ColName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("T"), strValue);
				arrErrors.Add(szError);
			}
		}
		else if (szTemp == "UND")
		{
			//Format UND (XXXXXXXXX)
			szTemp = clsDR.XMLStr(_T("strFederalIDNumber"), false);
			szTemp.Replace(_T("-"), _T(""));
		}
		AppendCsv(strBuffer, szTemp);

		//Column V - Recipient Name Type*
		szTemp = (clsDR.XMLBOOL(_T("bol1099Flag"), false) ? _T("I") : _T("B"));
		AppendCsv(strBuffer, szTemp);
		szTempNameType = szTemp;

		//Column W - Recipient Business or Entity Name Line 1
		CString recipientName = clsDR.XMLStr(_T("strVendorName"), false);
		CString colX, colY;

		if (recipientName.GetLength() <= 40)
		{
			//Fits entirely in Column X
			colX = recipientName;
			colY = _T("");	// blank
		}
		else
		{
			//Need to split into Column X and Y
			int splitPos = 40;

			//Move back to last space before 40 chars
			while (splitPos > 0 && recipientName[splitPos] != ' ')
			{
				splitPos--;
			}

			if (splitPos == 0)
			{
				//No space found, just split hard at 40
				splitPos = 40;
			}

			colX = recipientName.Left(splitPos).Trim();
			colY = recipientName.Mid(splitPos).Trim();
		}

		if (szTempNameType == "B")
		{
			if (IsValidEntityName(colX))
				AppendCsv(strBuffer, colX);
			else
				AppendCsv(strBuffer, _T(""));

			//Column X - Recipient Business or Entity Name Line 2

			if (IsValidEntityName(colY))
				AppendCsv(strBuffer, colY);
			else
				AppendCsv(strBuffer, _T(""));
		}

		//Column Y - Recipient First Name
		if (szTempNameType == "I")
		{
			szTemp = clsDR.XMLStr(_T("strVendorName"), false);

			if (IsValidPersonName(szTemp))
				AppendCsv(strBuffer, szTemp);
			else
				AppendCsv(strBuffer, _T(""));
		}
		else
			AppendCsv(strBuffer, _T(""));

		//Column Z - Recipient Middle Name
		szTemp = _T("");
		if (IsValidPersonName(szTemp))
			AppendCsv(strBuffer, szTemp);
		else
			AppendCsv(strBuffer, _T(""));

		//Column AA -Recipient Last Name (Surname)
		if (szTempNameType == "I")
		{
			szTemp = _T("");
			if (IsValidPersonName(szTemp))
				AppendCsv(strBuffer, szTemp);
			else
				AppendCsv(strBuffer, _T(""));
		}
		else
			AppendCsv(strBuffer, _T(""));

		//Column AB - Recipient Suffix
		szTemp = _T("");
		if (IsValidPersonName(szTemp))
			AppendCsv(strBuffer, szTemp);
		else
			AppendCsv(strBuffer, _T(""));

		//Column AC - Recipient Country*
		szTemp = si->CompanySettings.BillingInformation.Country;
		CString RecipientCountryCode = GetCountryCode(szTemp);
		if (!RecipientCountryCode.IsEmpty())
			AppendCsv(strBuffer, RecipientCountryCode);
		else
		{
			strValue.Format(_T("Recipient Country should not be empty for Vendor '%s'"), sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AC"), strValue);
			arrErrors.Add(szError);
		}

		//Column AD - Recipient Address Line 1 *
		szTemp = clsDR.XMLStr(_T("strAddress1"), false);
		if (!szTemp.IsEmpty())
		{
			if (IsValidRecipientAddress(szTemp, 35))
				AppendCsv(strBuffer, szTemp);
			else
			{
				strValue.Format(
								_T("Invalid format in Recipient Address Line 1 of '%s' for Vendor '%s'. "
								   " Format should comply to the following: 1) Have a max length of 35 characters, "
								   "2) Only be alpha-numeric characters [A-Z, 0-9] and following special characters: [hyphen ( - ), slash ( / ), space ( )], and "
								   "3) Not contain leading, trailing, or adjacent spaces."),
									szTemp,
									sz_RecipientName + " - " + sz_ColName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("AD"), strValue);
				arrErrors.Add(szError);
			}
		}
		else
		{
			strValue.Format(_T("Recipient Address Line 1 should not be empty for Vendor '%s'"), sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AD"), strValue);
			arrErrors.Add(szError);
		}

		//Column AE - Recipient Address Line 2
		szTemp = clsDR.XMLStr(_T("strAddress2"), false);
		if (IsValidRecipientAddress(szTemp, 35))
			AppendCsv(strBuffer, szTemp);
		else
		{
			strValue.Format(
								_T("Invalid format in Recipient Address Line 2 of '%s' for Vendor '%s'. "
								   " Format should comply to the following: 1) Have a max length of 35 characters, "
								   "2) Only be alpha-numeric characters [A-Z, 0-9] and following special characters: [hyphen ( - ), slash ( / ), space ( )], and "
								   "3) Not contain leading, trailing, or adjacent spaces."),
									szTemp,
									sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AE"), strValue);
			arrErrors.Add(szError);
		}

		//Column AF - Recipient City/Town*		
		szTemp = clsDR.XMLStr(_T("strCity"), false);
		if (!szTemp.IsEmpty())
		{
			if (IsValidRecipientCity(szTemp, 40))
				AppendCsv(strBuffer, szTemp);
			else
			{
				strValue.Format(
								_T("Invalid format in Recipient City/Town of '%s' for Vendor '%s'. "
								   "Format should comply to the following: 1) Have a max length of 40 characters, "
								   "2) Only be alpha-numeric characters [A-Z, 0-9] and following special characters: [space ( )], and "
								   "3) Not contain leading, trailing, or adjacent spaces."),
									szTemp,
									sz_RecipientName + " - " + sz_ColName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("AF"), strValue);
				arrErrors.Add(szError);
			}
		}
		else
		{
			strValue.Format(_T("Recipient City/Town should not be empty for Vendor '%s'"), sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AF"), strValue);
			arrErrors.Add(szError);
		}

		//Column AG - Recipient State / Province / Territory*		
		szTemp = clsDR.XMLStr(_T("strProvince"), false);
		if (!szTemp.IsEmpty())
		{
			bool bValidState = IsValidState(szTemp);
			if(bValidState)
				AppendCsv(strBuffer, szTemp);
			else
			{
				strValue.Format(
								_T("Invalid format in Recipient State/Province/Territory of '%s' for Vendor '%s'. "
								   "Format should comply to the following: 1) Domestic: Must be entered as their 2 letter abbreviation (A-Z), "
								   "or Foreign: Have a max length of 17 characters, "
								   "2) Only be alpha-numeric characters [A-Z, 0-9] and following special characters: [space ( )], and "
								   "3) Not contain leading, trailing, or adjacent spaces."),
									szTemp,
									sz_RecipientName + " - " + sz_ColName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("AG"), strValue);
				arrErrors.Add(szError);
			}
		}
		else
		{
			strValue.Format(_T("Recipient State/Province/Territory should not be empty for Vendor '%s'"), sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AG"), strValue);
			arrErrors.Add(szError);
		}

		//Column AH - Recipient ZIP/Postal Code*
		szTemp = clsDR.XMLStr(_T("strPostalCode"), false);
		if (!szTemp.IsEmpty())
		{
			bool bRecipientValid = IsValidPostalCode(szTemp);
			if(bRecipientValid)
				AppendCsv(strBuffer, szTemp);
			else
			{
				strValue.Format(
								_T("Invalid format in Recipient ZIP/Postal Code of '%s' for Vendor '%s'. "
								   "Format should comply to the following: 1) Domestic: ZIP/Postal Code must be exactly 5, 9 or 12 digits, "
								   "or Foreign: Have a max length of 9 characters, "
								   "2) Only be alpha-numeric characters [A-Z, 0-9]."),
									szTemp,
									sz_RecipientName + " - " + sz_ColName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("AH"), strValue);
				arrErrors.Add(szError);
			}
		}
		else
		{
			strValue.Format(_T("Recipient ZIP/Postal Code should not be empty for Vendor '%s'"), sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AH"), strValue);
			arrErrors.Add(szError);
		}

		//Column AI - Office Code
		AppendCsv(strBuffer, _T(""));

		//Column AJ - Form Account Number
		AppendCsv(strBuffer, _T(""));
		
		//Column AK - 2nd TIN Notice
		szTemp = (clsDR.XMLBOOL(_T("bol2ndTIN"), false) ? _T("Y") : _T("N"));
		AppendCsv(strBuffer, szTemp);

		CString strTemp;
		CString strAmount;
		strAmount.Empty();
		//Column AL - Box 1- Nonemployee Compensation
		if (isValidBoxAmount(dType1Amount))
		{
			strAmount.AppendFormat(_T("%.2f"), dType1Amount);
			AppendCsv(strBuffer, strAmount);
		}
		else
		{
			strTemp.Format(_T("%.f"), dType1Amount);
			strValue.Format(
							_T("Invalid format in Box 1 - Nonemployee Compensation of '%s' for Vendor '%s'. "
							   "Format should have no more than two digits after the decimal."
							   "Check for long numbers in money amount fields that may have been formatted in scientific notation."
							   "For example: 1.045E+5."
							),
								strTemp,
								sz_RecipientName + " - " + sz_ColName);
			szError.Format(_T("%d|%s|%s"), TransCount, _T("AL"), strValue);
			arrErrors.Add(szError);
		}

		//Column AM - Box 2 - Payer made direct sales totaling
		szTemp = (clsDR.XMLBOOL(_T("bol5000DirSales"), false) ? _T("Y") : _T("N"));
		AppendCsv(strBuffer, szTemp);

		if (_tstoi(m_szPaymentYear) >= PAYMENT_YEAR_2025)
		{
			strAmount.Empty();
			//Column AN - Box 3 - Excess golden parachute payments
			if (isValidBoxAmount(dType10Amount))
			{
				strAmount.AppendFormat(_T("%.2f"), dType10Amount);
				AppendCsv(strBuffer, strAmount);
			}
			else
			{
				strTemp.Format(_T("%.f"), dType10Amount);
				strValue.Format(
								_T("Invalid format in Box 3 - Excess golden parachute payments of '%s' for Vendor '%s'. "
									"Format should have no more than two digits after the decimal."
									"Check for long numbers in money amount fields that may have been formatted in scientific notation."
									"For example: 1.045E+5."
								),
									strTemp,
									sz_RecipientName + " - " + sz_ColName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("AN"), strValue);
				arrErrors.Add(szError);
			}

			strAmount.Empty();
			//Column AO - Box 4 -  Federal income tax withheld			
			if (isValidBoxAmount(dType4Amount))
			{
				strAmount.AppendFormat(_T("%.2f"), dType4Amount);
				AppendCsv(strBuffer, strAmount);
			}
			else
			{
				strTemp.Format(_T("%.f"), dType4Amount);
				strValue.Format(
								_T("Invalid format in Box 4 - Federal income tax withheld of '%s' for Vendor '%s'. "
									"Format should have no more than two digits after the decimal."
									"Check for long numbers in money amount fields that may have been formatted in scientific notation."
									"For example: 1.045E+5."
								),
									strTemp,
									sz_RecipientName + " - " + sz_ColName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("AO"), strValue);
				arrErrors.Add(szError);
			}
		}
		else
		{
			strAmount.Empty();
			//Column AN - Box 4 -  Federal income tax withheld			
			if (isValidBoxAmount(dType4Amount))
			{
				strAmount.AppendFormat(_T("%.2f"), dType4Amount);
				AppendCsv(strBuffer, strAmount);
			}
			else
			{
				strTemp.Format(_T("%.f"), dType4Amount);
				strValue.Format(
								_T("Invalid format in Box 4 - Federal income tax withheld of '%s' for Vendor '%s'. "
									"Format should have no more than two digits after the decimal."
									"Check for long numbers in money amount fields that may have been formatted in scientific notation."
									"For example: 1.045E+5."
								),
									strTemp,
									sz_RecipientName + " - " + sz_ColName);
				szError.Format(_T("%d|%s|%s"), TransCount, _T("AN"), strValue);
				arrErrors.Add(szError);
			}
		}

		if (m_bchkCombinedFederalState)
		{
			//Column AO - Combined Federal/State Filing(CF / SF) Check all states that apply
			szTemp = clsDR.XMLStr(_T("strProvince"), false);
			if (m_bchkCombinedFederalState)
				AppendCsv(strBuffer, szTemp);
			else
				AppendCsv(strBuffer, _T(""));

			double strTaxWithheld = clsDR.XMLDbl(_T("curStateTaxWithheld"), false);
			if (strTaxWithheld > 0)
			{
				//Column AP - State 1
				szTemp = clsDR.XMLStr(_T("strProvince"), false);
				if (!szTemp.IsEmpty() && szTemp.GetLength() == 2)
					AppendCsv(strBuffer, szTemp);
				else
					AppendCsv(strBuffer, _T(""));

				//Column AQ - State 1 - State Tax Withheld		
				szTemp = clsDR.XMLStr(_T("curStateTaxWithheld"), false);
				if (!szTemp.IsEmpty())
					AppendCsv(strBuffer, szTemp);
				else
					AppendCsv(strBuffer, _T(""));
			}
			else
			{
				//Column AR - Combined Federal/State Filing(CF / SF) Check all states that apply
				AppendCsv(strBuffer, _T(""));
				//Column AS - State 1			
				AppendCsv(strBuffer, _T(""));
				//Column AT - State 1 - State Tax Withheld	
				AppendCsv(strBuffer, _T(""));
			}
		}
		else
		{
			//Column AO - Combined Federal/State Filing(CF / SF) Check all states that apply
			AppendCsv(strBuffer, _T(""));
			//Column AP - State 1			
			AppendCsv(strBuffer, _T(""));
			//Column AQ - State 1 - State Tax Withheld	
			AppendCsv(strBuffer, _T(""));
		}
		//Column AR - State 1 - State/Payer state number
		AppendCsv(strBuffer, _T(""));
		//Column AS - State 1 - State income
		AppendCsv(strBuffer, _T(""));
		//Column AT - Column H -State 1 - Local income tax withheld
		AppendCsv(strBuffer, _T(""));
		//Column AU - State 1 - Special Data Entries
		AppendCsv(strBuffer, _T(""));
		//Column AV - State 2
		AppendCsv(strBuffer, _T(""));
		//Column AW - State 2 - State Tax Withheld
		AppendCsv(strBuffer, _T(""));
		//Column AX - State 2 - State/Payer state number
		AppendCsv(strBuffer, _T(""));
		//Column AY - State 2 - State income
		AppendCsv(strBuffer, _T(""));
		//Column AZ - State 2 - Local income tax withheld
		AppendCsv(strBuffer, _T(""));
		//Column BA - State 2 - Special Data Entries
		AppendCsv(strBuffer, _T(""),false);
	
		strBuffer.Append(_T("\r\n"));
	}

	// Validation error msg
	if (arrErrors.GetSize() >= 1)
	{
		Do1099ErrorReport(arrErrors);
	}

	while (!FileWrite(strBuffer) && !m_bCancel)
	{
		//Do this until either successful, or canceled!
	}

	return !m_bCancel;
}

bool CRFV_APElectronicFile1099Std::IsValidEntityName(const CString& str)
{
	CString trimmedStr = str;
	trimmedStr.Trim();

	// Check length
	if (trimmedStr.GetLength() > 40)
		return false;

	// Check for invalid characters
	for (int i = 0; i < trimmedStr.GetLength(); ++i)
	{
		TCHAR ch = trimmedStr[i];
		if (!(isalpha(ch) || isdigit(ch) || ch == ' ' || ch == '-' || ch == '#' ||
			  ch == '(' || ch == ')' || ch == '&' || ch == '\''))
		{
			return false;
		}
	}

	// Check for leading, trailing, or consecutive spaces
	if (str.Left(1) == _T(" ") || str.Right(1) == _T(" "))
		return false;
	if (str.Find(_T("  ")) != -1) // double spaces
		return false;

	return true;
}

bool CRFV_APElectronicFile1099Std::IsValidPersonName(const CString& str)
{
	CString trimmedStr = str;
	trimmedStr.Trim();

	// Check length
	if (trimmedStr.GetLength() > 20)
		return false;

	// Check for invalid characters
	for (int i = 0; i < trimmedStr.GetLength(); ++i)
	{
		TCHAR ch = trimmedStr[i];
		if (!(isalpha(ch) || ch == ' ' || ch == '-'))
		{
			return false;
		}
	}

	// Check for leading, trailing, or consecutive spaces
	if (str.Left(1) == _T(" ") || str.Right(1) == _T(" "))
		return false;
	if (str.Find(_T("  ")) != -1) // double spaces
		return false;

	return true;
}

bool CRFV_APElectronicFile1099Std::IsValidAddress(const CString& str, int maxLen)
{
	CString trimmed = str;
	trimmed.Trim(); 

	// Check length
	if (trimmed.GetLength() > maxLen) return false;

	// Check for adjacent spaces
	if (trimmed.Find(_T("  ")) != -1) return false;

	// Check for valid characters
	for (int i = 0; i < trimmed.GetLength(); ++i)
	{
		TCHAR ch = trimmed[i];
		if (!(isalnum(ch) || ch == _T('-') || ch == _T('&') || ch == _T('\'') ||
			  ch == _T('.') || ch == _T('#') || ch == _T(' ')))
		{
			return false;
		}
	}
	return true;
}
//---------------------------------------------------------------
//RS - 10/08/2025 - PBI 64621
bool CRFV_APElectronicFile1099Std::IsValidCity(const CString& str, int maxLen)
{
	CString trimmed = str;
	trimmed.Trim();

	// Check length
	if (trimmed.GetLength() > maxLen) return false;

	// Check for adjacent spaces
	if (trimmed.Find(_T("  ")) != -1) return false;

	// Check for valid characters
	for (int i = 0; i < trimmed.GetLength(); ++i)
	{
		TCHAR ch = trimmed[i];
		if (!(isalnum(ch) || ch == _T('-') || ch == _T('&') || ch == _T('\'') ||
			  ch == _T('.') || ch == _T(' ')))
		{
			return false;
		}
	}
	return true;
}
//----------------------------------------------------------------
bool CRFV_APElectronicFile1099Std::IsValidRecipientAddress(const CString& str, int maxLen)
{
	CString trimmed = str;
	trimmed.Trim();

	// Check length
	if (trimmed.GetLength() > maxLen) return false;

	// Check for adjacent spaces
	if (trimmed.Find(_T("  ")) != -1) return false;

	// Check for valid characters
	for (int i = 0; i < trimmed.GetLength(); ++i)
	{
		TCHAR ch = trimmed[i];
		if (!(isalnum(ch) || ch == _T('-') || ch == _T('/') || ch == _T(' ')))
		{
			return false;
		}
	}
	return true;
}
//---------------------------------------------------------------
//RS - 10/08/2025 - PBI 64621
bool CRFV_APElectronicFile1099Std::IsValidRecipientCity(const CString& str, int maxLen)
{
	CString trimmed = str;
	trimmed.Trim();

	// Check length
	if (trimmed.GetLength() > maxLen) return false;

	// Check for adjacent spaces
	if (trimmed.Find(_T("  ")) != -1) return false;

	// Check for valid characters
	for (int i = 0; i < trimmed.GetLength(); ++i)
	{
		TCHAR ch = trimmed[i];
		if (!(isalnum(ch) || ch == _T(' ')))
		{
			return false;
		}
	}
	return true;
}
//---------------------------------------------------------------
bool CRFV_APElectronicFile1099Std::isValidBoxAmount(double amount)
{
	CString str;
	str.Format(_T("%.2f"), amount);

	// Check length <= 18
	if (str.GetLength() > 18)
		return false;

	// Check for scientific notation
	if (str.Find(_T("E")) != -1 || str.Find(_T("e")) != -1)
		return false;

	return true;
}

bool CRFV_APElectronicFile1099Std::Do1099ErrorReport(const CStringArray& arrErrors)
{
	CString tempFilePath;
	TCHAR tempPath[MAX_PATH];
	CString TransCount;
	CString ColumnName;
	CString Description;
	int ErrorCount = 0;

	if (GetTempPath(MAX_PATH, tempPath))
	{
		tempFilePath.Format(_T("Errors_IRIS_1099"), tempPath);

		CStdioFile file;		
		CXMLParams objXMLDataParams(true);
		objXMLDataParams.SetRoot(_T("NewDataSet"));
		CXMLParams	objXMLParamNode(true);

		if (file.Open(tempFilePath, CFile::modeCreate | CFile::modeWrite))
		{
			for (int i = 0; i < arrErrors.GetSize(); ++i)
			{
				CString strError = arrErrors[i];
				CString token;
				int Pos = 0;

				file.WriteString(arrErrors[i] + _T("\n\n"));
				ErrorCount++;

				token = strError.Tokenize(_T("|"),Pos);
				TransCount = token;
				token = strError.Tokenize(_T("|"), Pos);
				ColumnName = token;
				token = strError.Tokenize(_T("|"), Pos);
				Description = token;

				objXMLParamNode.SetRoot(_T("Table"));				
				objXMLParamNode.MakeParam(_T("intTranNo"), TransCount);
				objXMLParamNode.MakeParam(_T("intTranDetailLine"), ColumnName);
				objXMLParamNode.MakeParam(_T("strDescription"), Description);
				objXMLDataParams.AppendXMLParam(&objXMLParamNode);
			}
			file.Close();
		}

		//RS
		BOOL b1099Form = (m_cbo1099Form.GetCurSel() == 0);			

		CString szErrorMsg;
		CString szXMLData(objXMLDataParams.GetXML());
		CString szXMLCommand;
		if (b1099Form)  //MISC
		{
			szErrorMsg = CGBLResources::GetResourceString(ErrorCount > 1 ? (UINT)IDS_IRIS_MISC_1099_RECORD_ERRORS : (UINT)IDS_IMPORT_RECORD_ERROR);
			szErrorMsg.Replace(_T("<TEXT1>"), CGBLForm::ToString((int)ErrorCount));
			CGBLForm::CMSMessageBox(this, 0, IDS_1099_MISC_ERR_REPORTTITLE, MB_OK + MB_ICONERROR, szErrorMsg);
			szXMLCommand = CGBLPrint::MakeAP1099ErrorReportXML(IDS_1099_MISC_ERR_REPORTTITLE, IDS_1099_MISC_ERR_REPORTTITLE);
		}
		else  //NEC
		{
			szErrorMsg = CGBLResources::GetResourceString(ErrorCount > 1 ? (UINT)IDS_IRIS_NEC_1099_RECORD_ERRORS : (UINT)IDS_IMPORT_RECORD_ERROR);
			szErrorMsg.Replace(_T("<TEXT1>"), CGBLForm::ToString((int)ErrorCount));
			CGBLForm::CMSMessageBox(this, 0, IDS_1099_NEC_ERR_REPORTTITLE, MB_OK + MB_ICONERROR, szErrorMsg);
			szXMLCommand = CGBLPrint::MakeAP1099ErrorReportXML(IDS_1099_NEC_ERR_REPORTTITLE, IDS_1099_NEC_ERR_REPORTTITLE);
		}

		CGBLPrint::PrintReport(&szXMLData, &szXMLCommand, eREPORTS, false);
	}
	m_bCancel = true;
	return false;
}
//-------------------
CString CRFV_APElectronicFile1099Std::GetCountryCode(CString szCountryCode)
{
	CString szTemp;
	auto& countrylist = CCMSCountryComboBox::InitCountries();
	CMap<CString, LPCTSTR, CString, LPCTSTR> countrymap;	
	szCountryCode.MakeLower();

	for (int i = 0; i < countrylist.GetCount(); ++i)
	{
		CString name = countrylist[i].Name();
		name.MakeLower();
		if (name == szCountryCode)
		{
			szTemp = countrylist[i].Code();
			break;
		}
	}
	return szTemp;
}
//-----------------------------------------------------------------
//RS - 10/08/2025 - PBI 64621
bool CRFV_APElectronicFile1099Std::IsValidState(const CString& str)
{
	bool bValidState = false;
	CString szTemp = str;

	if (si->CompanySettings.BillingInformation.Country == "United States")
	{

		// Domestic: must be exactly 2 letters (A–Z)
		int len = szTemp.GetLength();
		if (len == 2)
		{
			bool allLetters = true;
			for (int i = 0; i < len; ++i)
			{
				if (!_istalpha(szTemp[i]))
				{
					allLetters = false;
					break;
				}
			}

			if (allLetters)
				bValidState = true;
		}
	}
	else
	{
		// Foreign: max 17 characters, alphanumeric + spaces allowed (no leading/trailing)
		szTemp.Trim(); // removes leading/trailing spaces
		int len = szTemp.GetLength();
		if (len > 0 && len <= 17)
		{
			bool validChars = true;
			for (int i = 0; i < len; ++i)
			{
				TCHAR ch = szTemp[i];
				if (!_istalnum(ch) && ch != _T(' '))
				{
					validChars = false;
					break;
				}
			}
			if (validChars)
				bValidState = true;
		}
	}
	return bValidState;
}
//------------------------------------
bool CRFV_APElectronicFile1099Std::IsValidPostalCode(const CString& str)
{
	CString szTemp = str;
	bool bPayerValid = false;
	if (si->CompanySettings.BillingInformation.Country == "United States")
	{
		// Domestic: must be exactly 5, 9, or 12 digits
		int len = szTemp.GetLength();
		if (len == 5 || len == 9 || len == 12)
			bPayerValid = true;
	}
	else
	{
		// Foreign: must be alphanumeric only (A-Z, 0-9), max 9 chars
		bool bAlphaNum = true;
		for (int i = 0; i < szTemp.GetLength(); i++)
		{
			if (!isalnum((unsigned char)szTemp[i]))
			{
				bAlphaNum = false;
				break;
			}
		}
		if (bAlphaNum && szTemp.GetLength() <= 9)
			bPayerValid = true;
	}
	return bPayerValid;
}
