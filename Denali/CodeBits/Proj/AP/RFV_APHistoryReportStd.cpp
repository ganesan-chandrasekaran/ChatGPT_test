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
 *******************************************************************************
 * Initial code - mgregg - adapted by begbert 12-02-2005
 ******************************************************************************/
#include "stdafx.h"
#include "RFV_APHistoryReportStd.h"

 // CRFV_APHistoryReportStd

IMPLEMENT_DYNCREATE(CRFV_APHistoryReportStd, CRFV_APCommonStd)

CRFV_APHistoryReportStd::CRFV_APHistoryReportStd()
	: CRFV_APCommonStd(CRFV_APHistoryReportStd::IDD)
{
	m_szReportName = RPT_AP_AGING; // begbert 07-06-2006 : This takes care of the Total Debt report, with Param "szReportName" ;
	m_szSubForm = _T("18011A70-FEBA-4828-847B-A67491F0A2F5");
	SetUsePageBreak(true);

}

CRFV_APHistoryReportStd::~CRFV_APHistoryReportStd()
{
}

void CRFV_APHistoryReportStd::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_APF_RDO_HISTORY, m_optHistory);
	DDX_Control(pDX, IDC_APF_RDO_INVOICETOPAYMENT, m_optInvoiceToPayment);
	DDX_Control(pDX, IDC_APF_RDO_PAYMENTTOINVOICE, m_optPaymentToInvoice);
	// for drilldown reporting
	DDX_Control(pDX, IDC_ARF_CHK_ISDRILLDOWN, m_chkIsDrillDown);
	DDX_Control(pDX, IDC_ARF_TXT_REFERENCE, m_txtReference);

	DDX_Control(pDX, IDC_APF_LBL_DUE_DATE, m_lblDueDate);
	DDX_Control(pDX, IDC_APF_LBL_PERIOD, m_lblPeriod);
	DDX_Control(pDX, IDC_APF_LBL_TOTALDUE, m_lblVendTotDue);
	DDX_Control(pDX, IDC_APF_LBL_ITEMAMTREM, m_lblItemAmtRem);

	DDX_Control(pDX, IDC_APF_PAGEBREAKON, m_chkPageBreakOnVendor);

	DDX_CMSControl(pDX, IDC_APF_CBO_REPORTTYPE, m_cboReportType, IDC_APF_LBL_REPORTTYPE);
	DDX_CMSControl(pDX, IDC_APF_CBO_TRANSACTION_TYPE, m_cboTransactionType, IDC_APF_LBL_TRANSACTION_TYPE);

	//Version 2.5 Feature 10 AES
	DDX_Control(pDX, IDC_LBL_DATE_RANGE, m_lblDateRange);
	DDX_Control(pDX, IDC_CBO_DATE_RANGE, m_pCMSDate_RangeComboBox);

	DDX_Control(pDX, IDC_APF_DTE_START_DUE_DATE, m_txtDueDateStart);
	DDX_Control(pDX, IDC_APF_DTE_END_DUE_DATE, m_txtDueDateEnd);

	DDX_Control(pDX, IDC_APF_TXT_AGINGPERIODSSTART, m_txtPeriodStart);
	DDX_Control(pDX, IDC_APF_TXT_AGINGPERIODSEND, m_txtPeriodEnd);

	DDX_Control(pDX, IDC_APF_TXT_TOTALDUESTART, m_txtVendTotDueStart);
	DDX_Control(pDX, IDC_APF_TXT_TOTALDUEEND, m_txtVendTotDueEnd);
	DDX_Control(pDX, IDC_APF_TXT_ITEMAMTREMSTART, m_txtItemAmtRemStart);
	DDX_Control(pDX, IDC_APF_TXT_ITEMAMTREMEND, m_txtItemAmtRemEnd);

	DDX_Control(pDX, IDC_APF_CHK_EXLUDE_ZERO_VTD, m_chkVendTotDueExclZero);
	DDX_Control(pDX, IDC_APF_CHK_EXLUDE_ZERO_IAR, m_chkItemAmtRemExclZero);
}

BEGIN_MESSAGE_MAP(CRFV_APHistoryReportStd, CRFV_APCommonStd)
	ON_BN_CLICKED(IDC_APF_RDO_HISTORY, &CRFV_APHistoryReportStd::OnClickedHistoryRadio)
	ON_BN_CLICKED(IDC_APF_RDO_INVOICETOPAYMENT, &CRFV_APHistoryReportStd::OnClickedHistoryRadio)
	ON_BN_CLICKED(IDC_APF_RDO_PAYMENTTOINVOICE, &CRFV_APHistoryReportStd::OnClickedHistoryRadio)

	ON_EN_KILLFOCUS(IDC_APF_TXT_AGINGPERIODSSTART, &CRFV_APHistoryReportStd::OnEnKillfocusTxtPeriodStart)
	ON_EN_KILLFOCUS(IDC_APF_TXT_AGINGPERIODSEND, &CRFV_APHistoryReportStd::OnEnKillfocusTxtPeriodEnd)
	ON_EN_SETFOCUS(IDC_APF_TXT_AGINGPERIODSSTART, &CRFV_APHistoryReportStd::OnEnSetfocusTxtPeriodStart)
	ON_EN_SETFOCUS(IDC_APF_TXT_AGINGPERIODSEND, &CRFV_APHistoryReportStd::OnEnSetfocusTxtPeriodEnd)
	ON_EN_CHANGE(IDC_APF_TXT_AGINGPERIODSSTART, &CRFV_APHistoryReportStd::OnEnChangeTxtPeriodStart)
	ON_EN_CHANGE(IDC_APF_TXT_AGINGPERIODSEND, &CRFV_APHistoryReportStd::OnEnChangeTxtPeriodEnd)
END_MESSAGE_MAP()

// CRFV_APHistoryReportStd diagnostics

#ifdef _DEBUG
void CRFV_APHistoryReportStd::AssertValid() const
{
	__super::AssertValid();
}

void CRFV_APHistoryReportStd::Dump(CDumpContext& dc) const
{
	__super::Dump(dc);
}
#endif //_DEBUG

//**************************************************************************
//* Procedure:     Validate
//*
//* Description:   Validates the information of the client form before the parameters
//*				   are setup and set to the server components to get the data to give
//*				   to the report.
//*
//* Parameters:    none
//*
//* Date Created:  10/25/2005 by Mike Gregg
//**************************************************************************
bool CRFV_APHistoryReportStd::Validate(void)
{
	COleDateTime startDate, endDate;
	m_txtDueDateStart.GetDate(startDate);
	m_txtDueDateEnd.GetDate(endDate);

	//Defect ID 1-14345, PGP(04/28/2006) - changed message resource id.
	if (!CGBLForm::ValidateRange(this, startDate, endDate, IDS_ERR_MSG_DATERANGE))
	{
		m_txtDueDateEnd.SetFocus();
		m_txtDueDateEnd.SetSel(0, -1);
		return false;
	}

	if (!CGBLForm::ValidateRange(this, m_txtPeriodStart.GetText(), m_txtPeriodEnd.GetText(), IDS_ERR_MSG_INVAL_PERIOD_RANGE))
	{
		m_txtPeriodEnd.SetFocus();
		m_txtPeriodEnd.SetSel(0, -1);
		return false;
	}

	if (!CGBLForm::ValidateRange(this, m_txtVendTotDueStart.GetValue(), m_txtVendTotDueEnd.GetValue(), IDS_INVALID_RANGE_MSG))
	{
		m_txtVendTotDueEnd.SetFocus();
		m_txtVendTotDueEnd.SetSel(0, -1);
		return false;
	}

	if (!CGBLForm::ValidateRange(this, m_txtItemAmtRemStart.GetValue(), m_txtItemAmtRemEnd.GetValue(), IDS_INVALID_RANGE_MSG))
	{
		m_txtItemAmtRemEnd.SetFocus();
		m_txtItemAmtRemEnd.SetSel(0, -1);
		return false;
	}

	return __super::Validate();
}
//**************************************************************************
//* Procedure:     LoadResources
//*
//* Description:   Contains code to set the control Captions.
//*
//* Parameters:    None
//*
//* Date Created:  10/26/2005 by Mike Gregg
//**************************************************************************
void CRFV_APHistoryReportStd::LoadResources()
{
	__super::LoadResources();

	//set text for the title of the form
	CGBLForm::SetFormTitle(this, IDS_AP_HISTORY_REPORT);
	//set text for the top header of the form
	SetFormDescription(IDS_APF_RFV_HISTORY_TITLE);

	SetResourceText(IDC_APF_RDO_HISTORY, IDS_CTRL_HISTORY);
	SetResourceText(IDC_APF_RDO_INVOICETOPAYMENT, IDS_AR_INVOICE_TO_PAYMENT);
	SetResourceText(IDC_APF_RDO_PAYMENTTOINVOICE, IDS_AR_PAYMENT_TO_INVOICE);

	//Version 2.5 Feature 10 AES
	SetResourceText(IDC_LBL_DATE_RANGE, IDS_SA_DATERANGE);

	SetResourceText(IDC_APF_LBL_DUE_DATE, IDS_AP_LBL_DUE_DATE);
	SetResourceText(IDC_APF_LBL_PERIOD, IDS_AP_LBL_PERIOD);
	SetResourceText(IDC_APF_LBL_TOTALDUE, IDS_AP_VENDOR_TOTAL_DUE);
	SetResourceText(IDC_APF_LBL_ITEMAMTREM, IDS_AP_LBL_ITEM_AMT_REMAINING);

	SetResourceText(IDC_APF_PAGEBREAKON, IDS_AP_LBL_PAGE_BREAK_ON_VENDOR_NUMBER);

	SetResourceText(IDC_APF_LBL_REPORTTYPE, IDS_REPORTTYPE);
	SetResourceText(IDC_APF_LBL_TRANSACTION_TYPE, IDS_SA_TRANSACTION_TYPE);

	SetResourceText(IDC_APF_CHK_EXLUDE_ZERO_VTD, IDS_AP_EXCLUDE_ZERO);
	SetResourceText(IDC_APF_CHK_EXLUDE_ZERO_IAR, IDS_AP_EXCLUDE_ZERO);
}

//**************************************************************************
//* Procedure:     InitControls
//*
//* Description:   Populates combo boxes and other controls.
//*
//* Parameters:    None
//*
//* Date Created:  10/25/2005 by Mike Gregg
//**************************************************************************
void CRFV_APHistoryReportStd::InitControls()
{

	//Make a call to the base; this does most of the work.
	__super::InitControls();

	m_lblDueDate.SetWindowText(GetResStr(IDS_AP_LBL_TRANSDATE_RANGE));

	m_lblPeriod.ShowWindow(false);
	m_txtPeriodStart.ShowWindow(false);
	m_txtPeriodEnd.ShowWindow(false);
	m_lblVendTotDue.ShowWindow(false);
	m_txtVendTotDueStart.ShowWindow(false);
	m_txtVendTotDueEnd.ShowWindow(false);
	m_lblItemAmtRem.ShowWindow(false);
	m_txtItemAmtRemStart.ShowWindow(false);
	m_txtItemAmtRemEnd.ShowWindow(false);
	m_chkVendTotDueExclZero.ShowWindow(false);
	m_chkItemAmtRemExclZero.ShowWindow(false);

	m_optHistory.SetCheck(1);
	m_optHistory.ShowWindow(true);
	m_optInvoiceToPayment.ShowWindow(true);
	m_optPaymentToInvoice.ShowWindow(true);

	OnClickedHistoryRadio();
	//1-24574 AES need to set the range and dates
	m_txtDueDateEnd.SetDate(si->ApplicationSettings.SystemDate);

	LoadReportTypeCombo();
	LoadTransactionTypeCombo();

	//	m_txtDueDateStart.SetDate(

	//	m_txtPeriodStart.SetLimitText(1);		//Only allow user to enter 2 digits
	//	m_txtPeriodEnd  .SetLimitText(1);		//Only allow user to enter 2 digits

	//	m_txtPeriodStart.SetText(GetResStr(IDS_CURRENT));
	//	m_txtPeriodEnd  .SetText(GetResStr(IDS_AP_PERIOD_FIVE));

		// This populates are neatly formats the two 'period' edits.
	m_txtPeriodStart.SetWindowText(_T("0"));
	m_txtPeriodEnd.SetWindowText(_T("5"));
	OnEnKillfocusTxtPeriodStart();
	OnEnKillfocusTxtPeriodEnd();

	InitCurrencyEdit(m_txtVendTotDueStart, _T("-99999999999.99"));
	InitCurrencyEdit(m_txtVendTotDueEnd, _T(" 99999999999.99"));
	InitCurrencyEdit(m_txtItemAmtRemStart, _T("-99999999999.99"));
	InitCurrencyEdit(m_txtItemAmtRemEnd, _T(" 99999999999.99"));

	//Version 2.5 Feature 10 AES
	m_pCMSDate_RangeComboBox.InitializeDateControls(&m_txtDueDateStart, &m_txtDueDateEnd, false, true);

	// the 'exclude zero' checkboxes start out checked.
	m_chkVendTotDueExclZero.SetCheck(1);
	m_chkItemAmtRemExclZero.SetCheck(1);

	//1-24574 AES need to set the range and dates
	COleDateTime dteOleStarting(1980, 01, 01, 0, 0, 0);
	m_txtDueDateStart.SetRange(&dteOleStarting, &si->FiscalCalendar.EndDate);
	m_txtDueDateEnd.SetRange(&dteOleStarting, &si->FiscalCalendar.EndDate);
	m_txtDueDateStart.SetDate(si->FiscalCalendar.StartDate);
	//	20028 06/12/2014 mvt - Changed end date to system date
	m_txtDueDateEnd.SetDate(si->ApplicationSettings.SystemDate);
}

int CRFV_APHistoryReportStd::LoadSortByCombo()
{
	__super::LoadSortByCombo();

	LoadSortByUDF(m_cboSortBy, m_cboSortBy.GetCount());
	return m_cboSortBy.GetCount();
}

int CRFV_APHistoryReportStd::LoadReportTypeCombo()
{
	m_cboReportType.ResetContent();

	m_cboReportType.ResetContent();
	AddIDSToCombo(m_cboReportType, IDS_AR_DETAILED);
	AddIDSToCombo(m_cboReportType, IDS_SUMMARY);
	AddIDSToCombo(m_cboReportType, IDS_AR_CONDENSED);
	AddIDSToCombo(m_cboReportType, IDS_SA_DETAILED_WITH_NOTES);
	m_cboReportType.SetCurSel(0);
	return m_cboReportType.GetCount();
}

int CRFV_APHistoryReportStd::LoadTransactionTypeCombo()
{
	m_cboTransactionType.ResetContent();
	AddIDSToCombo(m_cboTransactionType, IDS_AP_LBL_CMB_TTYPE_ALL_TRANSACTIONS);
	AddIDSToCombo(m_cboTransactionType, IDS_AP_LBL_CMB_TTYPE_INVOICE);
	AddIDSToCombo(m_cboTransactionType, IDS_AP_LBL_CMB_TTYPE_CREDIT_ADJUSTMENT);
	AddIDSToCombo(m_cboTransactionType, IDS_AP_LBL_CMB_TTYPE_PAYMENT_CORRECTION);
	AddIDSToCombo(m_cboTransactionType, IDS_PAYMENT_CORR_CC);				// 11/08/06 BK Vers 2.5 Feature 24 Add credit card payments (Payment Correction - Credit Card)
	AddIDSToCombo(m_cboTransactionType, IDS_AP_LBL_CMB_TTYPE_PAYMENT);
	AddIDSToCombo(m_cboTransactionType, IDS_PAYMENTCC);						// 11/08/06 BK Vers 2.5 Feature 24 Add credit card payments  (Payment - Credit Card)
	AddIDSToCombo(m_cboTransactionType, IDS_AP_LBL_CMB_TTYPE_DEBIT_ADJUSTMENT);
	AddIDSToCombo(m_cboTransactionType, IDS_AP_LBL_CMB_TTYPE_UNAPPLIED_DEBIT);
	AddIDSToCombo(m_cboTransactionType, IDS_AP_LBL_CMB_TTYPE_DEBIT_INVOICE);
	m_cboTransactionType.SetCurSel(0);
	return m_cboTransactionType.GetCount();
}

void CRFV_APHistoryReportStd::InitCurrencyEdit(CCMSCurrencyEdit &edit, CString value)
{
	edit.SetText(value);

	//	edit.SetHighlightOnSetFocus(TRUE);
	//	edit.m_bDefaultZeros = true;
	//	edit.SetDecimalDigitCount(11);
	//	edit.SetFractionalDigitCount(2);
	//	edit.m_bNoNegativeNumber = true;
	//	edit.SetNegativeFormat(_T("-$1.1"));

	edit.SetHideCurrencySymbolOnFocus(false);
	edit.SetNegativeFormat(_T("-$1.1"));
}
void CRFV_APHistoryReportStd::SetControlSizes()
{
	__super::SetControlSizes();

	m_pEXLayoutManager->SizeToFit(&m_optHistory, SIZ_CheckBox);
	m_pEXLayoutManager->SizeToFit(&m_optInvoiceToPayment, SIZ_CheckBox);
	m_pEXLayoutManager->SizeToFit(&m_optPaymentToInvoice, SIZ_CheckBox);

	// begbert 06-22-2006 1-24665 : lay out checkboxes by space between, not by left edge.
//	ConformSize(eSZ_WIDTH, 3,
//					&m_optHistory ,
//					&m_optInvoiceToPayment ,
//					&m_optPaymentToInvoice );

	//	m_pEXLayoutManager->SizeToFit(&m_chkPageBreakOnVendorNumber , SIZ_CheckBox );
//	m_pEXLayoutManager->SizeToFit(&m_chkPrintUDFs               , SIZ_CheckBox );

	m_pEXLayoutManager->SizeToFit(&m_chkPageBreakOnVendor, SIZ_CheckBox);

	// 11/08/06 BK Vers 2.5 Feature 24 Add credit card payments - increased combo boxes' widths from 15 to 20
	//m_pEXLayoutManager->SizeToFit(&m_cboReportType         , SIZ_ComboBox , 15 );
	//m_pEXLayoutManager->SizeToFit(&m_cboTransactionType    , SIZ_ComboBox , 15 );
	m_pEXLayoutManager->SizeToFit(&m_cboReportType, SIZ_ComboBox, 25);
	m_pEXLayoutManager->SizeToFit(&m_cboTransactionType, SIZ_ComboBox, 25);
	m_pEXLayoutManager->SizeToFit(&m_cboSortBy, SIZ_ComboBox, 25);
	m_pEXLayoutManager->SizeToFit(&m_cboVendorStatus, SIZ_ComboBox, 25);
	m_pEXLayoutManager->SizeToFit(&m_cboFilter, SIZ_ComboBox, 25);


	//Version 2.5 Feature 10 AES
	m_pEXLayoutManager->AddLBLControlToGroup(1, &m_lblDateRange);
	m_pEXLayoutManager->SizeToFit(&m_pCMSDate_RangeComboBox, SIZ_ComboBox, 19);

	m_pEXLayoutManager->SizeToFit(&m_txtDueDateStart, SIZ_DropDownText, 3);
	m_pEXLayoutManager->SizeToFit(&m_txtDueDateEnd, SIZ_DropDownText, 3);

	m_pEXLayoutManager->SizeToFit(&m_txtPeriodStart, SIZ_TextBox, 20);
	m_pEXLayoutManager->SizeToFit(&m_txtPeriodEnd, SIZ_TextBox, 20);

	m_pEXLayoutManager->SizeToFit(&m_txtVendTotDueStart, SIZ_TextBox, 20);
	m_pEXLayoutManager->SizeToFit(&m_txtVendTotDueEnd, SIZ_TextBox, 20);
	m_pEXLayoutManager->SizeToFit(&m_txtItemAmtRemStart, SIZ_TextBox, 20);
	m_pEXLayoutManager->SizeToFit(&m_txtItemAmtRemEnd, SIZ_TextBox, 20);

	m_pEXLayoutManager->SizeToFit(&m_chkVendTotDueExclZero, SIZ_CheckBox);
	m_pEXLayoutManager->SizeToFit(&m_chkItemAmtRemExclZero, SIZ_CheckBox);

	m_pEXLayoutManager->SizeToFit(&m_chkGroupByAPCode, SIZ_CheckBox);

	m_pEXLayoutManager->AddLBLControlToGroup(1, m_cboReportType.GetWndLabelPtr());
	m_pEXLayoutManager->AddLBLControlToGroup(1, m_cboTransactionType.GetWndLabelPtr());
	m_pEXLayoutManager->AddLBLControlToGroup(1, &m_lblDueDate);
	m_pEXLayoutManager->AddLBLControlToGroup(1, &m_lblPeriod);
	m_pEXLayoutManager->AddLBLControlToGroup(1, &m_lblVendTotDue);
	m_pEXLayoutManager->AddLBLControlToGroup(1, &m_lblItemAmtRem);
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
void CRFV_APHistoryReportStd::SetupLayoutManager()
{
	__super::SetupLayoutManager();

	m_pEXLayoutManager->AnchorControlTL(m_cboSortBy);
	m_pEXLayoutManager->AnchorControlTL(m_cboReportType, &m_cboSortBy);
	m_pEXLayoutManager->AnchorControlTL(m_cboVendorStatus, &m_cboReportType);
	m_pEXLayoutManager->AnchorControlTL(m_cboTransactionType, &m_cboVendorStatus);
	m_pEXLayoutManager->AnchorControlTL(m_cboFilter, &m_cboTransactionType);

	m_pEXLayoutManager->AnchorControlSameTop(&m_chkPageBreakOnVendor, m_cboSortBy.GetWndLabelPtr());
	m_pEXLayoutManager->AnchorControlLeft(&m_chkPageBreakOnVendor, &m_cboSortBy);

	//**Vendor Number and Start/End Controls**
	m_pEXLayoutManager->AnchorControlTop(&m_lblStart, &m_cboFilter);
	m_pEXLayoutManager->AnchorLBLControlTL(&m_txtVendNumStart, m_txtVendNumStart.GetWndLabelPtr(), &m_lblStart);
	m_pEXLayoutManager->AnchorControlSameLeft(&m_lblStart, &m_txtVendNumStart);
	m_pEXLayoutManager->AnchorControlSameTop(&m_txtVendNumEnd, &m_txtVendNumStart);
	m_pEXLayoutManager->AnchorControlLeft(&m_txtVendNumEnd, &m_txtVendNumStart);
	m_pEXLayoutManager->AnchorControlSameTop(&m_lblEnd, &m_lblStart);
	m_pEXLayoutManager->AnchorControlSameLeft(&m_lblEnd, &m_txtVendNumEnd);
	//**Customer Name Controls**
	m_pEXLayoutManager->AnchorLBLControlTL(&m_txtVendNameStart, m_txtVendNameStart.GetWndLabelPtr(), &m_txtVendNumStart);
	m_pEXLayoutManager->AnchorLBLControlSameLeft(&m_txtVendNameStart, m_txtVendNameStart.GetWndLabelPtr(), &m_txtVendNumStart);
	m_pEXLayoutManager->AnchorControlSameTop(&m_txtVendNameEnd, &m_txtVendNameStart);
	m_pEXLayoutManager->AnchorControlLeft(&m_txtVendNameEnd, &m_txtVendNameStart);
	//**Product Type Controls**
	m_pEXLayoutManager->AnchorLBLControlTL(&m_txtProdTypeStart, m_txtProdTypeStart.GetWndLabelPtr(), &m_txtVendNameStart);
	m_pEXLayoutManager->AnchorLBLControlSameLeft(&m_txtProdTypeStart, m_txtProdTypeStart.GetWndLabelPtr(), &m_txtVendNameStart);
	m_pEXLayoutManager->AnchorControlSameTop(&m_txtProdTypeEnd, &m_txtProdTypeStart);
	m_pEXLayoutManager->AnchorControlSameLeft(&m_txtProdTypeEnd, &m_txtVendNameEnd);
	//Version 2.5 Feature 10 AES
	m_pEXLayoutManager->AnchorLBLControlTL(&m_pCMSDate_RangeComboBox, &m_lblDateRange, &m_txtProdTypeStart);
	//**Due Date Controls**
	m_pEXLayoutManager->AnchorLBLControlTL(&m_txtDueDateStart, &m_lblDueDate, &m_pCMSDate_RangeComboBox);
	m_pEXLayoutManager->AnchorLBLControlSameLeft(&m_txtDueDateStart, &m_lblDueDate, &m_pCMSDate_RangeComboBox);
	m_pEXLayoutManager->AnchorControlSameTop(&m_txtDueDateEnd, &m_txtDueDateStart);
	m_pEXLayoutManager->AnchorControlSameLeft(&m_txtDueDateEnd, &m_txtProdTypeEnd);
	//**Period Controls**
	m_pEXLayoutManager->AnchorLBLControlTL(&m_txtPeriodStart, &m_lblPeriod, &m_txtDueDateStart);
	m_pEXLayoutManager->AnchorLBLControlSameLeft(&m_txtPeriodStart, &m_lblPeriod, &m_txtDueDateStart);
	m_pEXLayoutManager->AnchorControlSameTop(&m_txtPeriodEnd, &m_txtPeriodStart);
	m_pEXLayoutManager->AnchorControlSameLeft(&m_txtPeriodEnd, &m_txtDueDateEnd);
	//**Vendor Total Due Controls (including check box)**
	m_pEXLayoutManager->AnchorLBLControlTL(&m_txtVendTotDueStart, &m_lblVendTotDue, &m_txtPeriodStart);
	m_pEXLayoutManager->AnchorLBLControlSameLeft(&m_txtVendTotDueStart, &m_lblVendTotDue, &m_txtPeriodStart);
	m_pEXLayoutManager->AnchorControlSameTop(&m_txtVendTotDueEnd, &m_txtVendTotDueStart);
	m_pEXLayoutManager->AnchorControlSameLeft(&m_txtVendTotDueEnd, &m_txtPeriodEnd);
	m_pEXLayoutManager->AnchorControlSameTop(&m_chkVendTotDueExclZero, &m_lblVendTotDue);
	m_pEXLayoutManager->AnchorControlLeft(&m_chkVendTotDueExclZero, &m_txtVendTotDueEnd);
	//**Item Amount Remaining Controls (including check box)**
	m_pEXLayoutManager->AnchorLBLControlTL(&m_txtItemAmtRemStart, &m_lblItemAmtRem, &m_txtVendTotDueStart);
	m_pEXLayoutManager->AnchorLBLControlSameLeft(&m_txtItemAmtRemStart, &m_lblItemAmtRem, &m_txtVendTotDueStart);
	m_pEXLayoutManager->AnchorControlSameTop(&m_txtItemAmtRemEnd, &m_txtItemAmtRemStart);
	m_pEXLayoutManager->AnchorControlSameLeft(&m_txtItemAmtRemEnd, &m_txtVendTotDueEnd);
	m_pEXLayoutManager->AnchorControlSameTop(&m_chkItemAmtRemExclZero, &m_lblItemAmtRem);
	m_pEXLayoutManager->AnchorControlLeft(&m_chkItemAmtRemExclZero, &m_txtItemAmtRemEnd);
	//**AP Code Controls (including check box)**
	m_pEXLayoutManager->AnchorLBLControlTL(&m_txtAPCodeStart, m_txtAPCodeStart.GetWndLabelPtr(), &m_txtItemAmtRemStart);
	m_pEXLayoutManager->AnchorLBLControlSameLeft(&m_txtAPCodeStart, m_txtAPCodeStart.GetWndLabelPtr(), &m_txtItemAmtRemStart);
	m_pEXLayoutManager->AnchorControlSameTop(&m_txtAPCodeEnd, &m_txtAPCodeStart);
	m_pEXLayoutManager->AnchorControlSameLeft(&m_txtAPCodeEnd, &m_txtItemAmtRemEnd);
	m_pEXLayoutManager->AnchorControlSameTop(&m_chkGroupByAPCode, m_txtAPCodeStart.GetWndLabelPtr());
	m_pEXLayoutManager->AnchorControlSameLeft(&m_chkGroupByAPCode, &m_chkItemAmtRemExclZero);

	m_pEXLayoutManager->AnchorControlTop(&m_optHistory);
	m_pEXLayoutManager->AnchorControlSameLeft(&m_optHistory, &m_cboSortBy);
	m_pEXLayoutManager->AnchorControlTL(&m_optInvoiceToPayment, NULL, &m_optHistory, 0, 50);	// begbert 06-22-2006 1-24665 : added a uniform space between checkboxes.
	m_pEXLayoutManager->AnchorControlTL(&m_optPaymentToInvoice, NULL, &m_optInvoiceToPayment, 0, 50);	// begbert 06-22-2006 1-24665 : added a uniform space between checkboxes.
	m_pEXLayoutManager->AnchorControlTop(&m_cboSortBy, &m_optPaymentToInvoice);
	m_pEXLayoutManager->AnchorControlTop(&m_txtAPCodeStart, &m_txtDueDateStart);
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
void CRFV_APHistoryReportStd::GetFormBounds(CWnd*& pRightBound, CWnd*& pBottomBound)
{
	pRightBound = &m_chkGroupByAPCode;

	// 1-31599 BK 12/29/08 uncommented code that checks if a valid pBottomBound pointer, form wouldn't expand when an additional filter is selected
	if (!pBottomBound)
		pBottomBound = &m_txtAPCodeEnd;

	__super::GetFormBounds(pRightBound, pBottomBound);
}

void CRFV_APHistoryReportStd::OnClickedHistoryRadio()
{
	m_cboReportType.ResetContent();
	m_cboTransactionType.ResetContent();

	AddIDSToCombo(m_cboReportType, IDS_AR_DETAILED);
	AddIDSToCombo(m_cboReportType, IDS_AR_CONDENSED);
	AddIDSToCombo(m_cboReportType, IDS_SUMMARY);
	AddIDSToCombo(m_cboReportType, IDS_SA_DETAILED_WITH_NOTES);

	m_cboReportType.EnableWindow(m_optHistory.GetCheck());
	m_chkGroupByAPCode.EnableWindow(m_optHistory.GetCheck());

	AddIDSToCombo(m_cboTransactionType, IDS_AP_LBL_CMB_TTYPE_ALL_TRANSACTIONS);

	if (!m_optPaymentToInvoice.GetCheck())
	{
		AddIDSToCombo(m_cboTransactionType, IDS_AP_LBL_CMB_TTYPE_INVOICE);
		AddIDSToCombo(m_cboTransactionType, IDS_AP_LBL_CMB_TTYPE_CREDIT_ADJUSTMENT);
		AddIDSToCombo(m_cboTransactionType, IDS_AP_LBL_CMB_TTYPE_PAYMENT_CORRECTION);
		AddIDSToCombo(m_cboTransactionType, IDS_PAYMENT_CORR_CC);				// 11/08/06 BK Vers 2.5 Feature 24 Add credit card payments (Payment Correction - Credit Card)
	}

	if (!m_optInvoiceToPayment.GetCheck())
	{
		AddIDSToCombo(m_cboTransactionType, IDS_AP_LBL_CMB_TTYPE_PAYMENT);
		AddIDSToCombo(m_cboTransactionType, IDS_PAYMENTCC);						// 11/08/06 BK Vers 2.5 Feature 24 Add credit card payments  (Payment - Credit Card)
		AddIDSToCombo(m_cboTransactionType, IDS_AP_LBL_CMB_TTYPE_DEBIT_ADJUSTMENT);
		AddIDSToCombo(m_cboTransactionType, IDS_AP_LBL_CMB_TTYPE_DEBIT_INVOICE);
		AddIDSToCombo(m_cboTransactionType, IDS_AP_LBL_CMB_TTYPE_UNAPPLIED_DEBIT);
	}

	m_cboReportType.SetCurSel(0);
	m_cboTransactionType.SetCurSel(0);
}

void CRFV_APHistoryReportStd::OnEnKillfocusTxtPeriodStart()
{
	OnEnKillfocusTxtPeriod(m_txtPeriodStart);
}
void CRFV_APHistoryReportStd::OnEnKillfocusTxtPeriodEnd()
{
	OnEnKillfocusTxtPeriod(m_txtPeriodEnd);
}
void CRFV_APHistoryReportStd::OnEnKillfocusTxtPeriod(COXCoolCtrl<CCMSEdit>& edit)
{
	/* On account of the fact that it IS possible to get in here without first calling
	   OnEnSetFocus, we do so now.  It makes sure the data is in the expected format.*/
	OnEnSetfocusTxtPeriod(edit);

	CString szvalue = edit.GetText();
	szvalue = (szvalue == "" || szvalue == "0") ? _T("Current") : _T("Period ") + szvalue;
	edit.SetLimitText(10);
	edit.SetWindowText(szvalue);
	edit.SetLimitText(1);
}

void CRFV_APHistoryReportStd::OnEnSetfocusTxtPeriodStart()
{
	OnEnSetfocusTxtPeriod(m_txtPeriodStart);
}
void CRFV_APHistoryReportStd::OnEnSetfocusTxtPeriodEnd()
{
	OnEnSetfocusTxtPeriod(m_txtPeriodEnd);
}
void CRFV_APHistoryReportStd::OnEnSetfocusTxtPeriod(COXCoolCtrl<CCMSEdit>& edit)
{
	//	 This reduces the string in the box down to the appropriate integer.
		// The value is whatever integer is contained in the last digit.
	CString szvalue = edit.GetText();
	szvalue.Format(_T("%d"), _tstoi(szvalue.Right(1)));
	edit.SetText(szvalue);
}

void CRFV_APHistoryReportStd::OnEnChangeTxtPeriodStart()
{
	OnEnChangeTxtPeriod(m_txtPeriodStart, m_PrevPeriodStart);
}
void CRFV_APHistoryReportStd::OnEnChangeTxtPeriodEnd()
{
	OnEnChangeTxtPeriod(m_txtPeriodEnd, m_PrevPeriodEnd);
}
void CRFV_APHistoryReportStd::OnEnChangeTxtPeriod(COXCoolCtrl<CCMSEdit>& edit, CString& was)
{
	if (edit.GetLimitText() > 1) return;

	CString szvalue = edit.GetText();
	if (szvalue != ""  &&
		szvalue != "0" && szvalue != "1" && szvalue != "2" &&
		szvalue != "3" && szvalue != "4" && szvalue != "5")
		edit.SetText(was);
	else {
		was = szvalue;
		//AR 06/16/2026 PBI 66872 - Focus issue when tabing to a number field
		//edit.SetSel(0, -1);	// This line appears not to work, dunno why.
		::PostMessage(edit.m_hWnd, EM_SETSEL, 0, -1);
	}
}
void CRFV_APHistoryReportStd::SetReportValues(CMSGBL::IReportInstantiator * pReportInstantiator)
{
	// for drilldown reporting
	pReportInstantiator->MakeReportValue(_T("@bIsDrillDown#"), m_chkIsDrillDown.GetCheck());
	pReportInstantiator->MakeReportValue(_T("@szReference#"), m_txtReference.GetText());

	if (m_optHistory.GetCheck() == BST_CHECKED)
		pReportInstantiator->SetReportName(RPT_AP_HISTORY);
	else if (m_optInvoiceToPayment.GetCheck() == BST_CHECKED)
		pReportInstantiator->SetReportName(RPT_AP_INVOICETOPAYMENT);
	else if (m_optPaymentToInvoice.GetCheck() == BST_CHECKED)
		pReportInstantiator->SetReportName(RPT_AP_PAYMENTTOINVOICE);

	CRFV_APCommonStd::SetReportValues(pReportInstantiator);
	pReportInstantiator->MakeReportTitle(IDS_CAP_INHISTORY_REPORT_PREVIEW);

	//Version 2.5 Feature 10 AES
	CString szDateRange(_T(""));
	m_pCMSDate_RangeComboBox.GetWindowTextW(szDateRange);
	pReportInstantiator->MakeReportValue(_T("@IDS_DATE_RANGE_VALUE"), szDateRange);
	pReportInstantiator->MakeReportResource(_T("@IDS_IN_TRAN_DESCRIPTION"), IDS_AP_DATE_RANGE_C);

	COleDateTime odtDateTime;
	m_txtDueDateStart.GetDate(odtDateTime);
	pReportInstantiator->MakeReportValue(_T("@dteStartTranDate#"), odtDateTime.Format(VAR_DATEVALUEONLY));
	m_txtDueDateEnd.GetDate(odtDateTime);
	pReportInstantiator->MakeReportValue(_T("@dteEndTranDate#"), odtDateTime.Format(VAR_DATEVALUEONLY));


	// Transaction Type
	//  '	1=Invoice, 2=Credit Adjustment, 3=Payment Correction,
	//  '	4=Payment, 5=Debit Adjustment,
	//  '	6=Debit Invoice (Credit issued by Vendor - entered via Invoicing),
	//  '	7=Unapplied Debit (Credit issued by Vendor - entered via Payments & Adjustments)
// 11/08/06 BK Vers 2.5 Feature 24 Added credit card payment types
	// 3 =  3=Payment Correction and bolCCPayment = 0
	// 8 =  3=Payment Correction and bolCCPayment = 1 (paid by credit card)
	// 4 =  4=Payment and bolCCPayment = 0
	// 9 =  4=Payment and bolCCPayment = 1	(paid by credit card)
	int intTemp = 0;
	switch (m_cboTransactionType.GetItemData(m_cboTransactionType.GetCurSel()))
	{
	case IDS_AP_LBL_CMB_TTYPE_ALL_TRANSACTIONS: intTemp = 0; break;
	case IDS_AP_LBL_CMB_TTYPE_INVOICE: intTemp = 1; break;
	case IDS_AP_LBL_CMB_TTYPE_CREDIT_ADJUSTMENT: intTemp = 2; break;
	case IDS_AP_LBL_CMB_TTYPE_PAYMENT_CORRECTION: intTemp = 3; break;
	case IDS_AP_LBL_CMB_TTYPE_PAYMENT: intTemp = 4; break;
	case IDS_AP_LBL_CMB_TTYPE_DEBIT_ADJUSTMENT: intTemp = 5; break;
	case IDS_AP_LBL_CMB_TTYPE_DEBIT_INVOICE: intTemp = 6; break;
	case IDS_AP_LBL_CMB_TTYPE_UNAPPLIED_DEBIT: intTemp = 7; break;
		// 11/08/06 BK Vers 2.5 Feature 24 Added credit card payment types
	case IDS_PAYMENT_CORR_CC: intTemp = 8; break;
	case IDS_PAYMENTCC: intTemp = 9; break;
	}

	CString szTemp = _T("");
	//int nTransTypeSel = m_cboTransactionType.GetCurSel();
	//if (nTransTypeSel >= 1 && nTransTypeSel <= 7 )
	//	szTemp.Format(_T("%i"), nTransTypeSel);

	//pReportInstantiator->MakeReportValue(_T("intTransactionType#"), szTemp);
	pReportInstantiator->MakeReportValue(_T("intTransactionType#"), intTemp);

	//m_cboTransactionType.GetLBText(nTransTypeSel, szTemp);
	m_cboTransactionType.GetLBText(m_cboTransactionType.GetCurSel(), szTemp);
	pReportInstantiator->MakeReportValue(_T("@szTransactionType"), szTemp);

	pReportInstantiator->MakeReportValue(_T("@bPageBreak"), m_chkPageBreakOnVendor.GetCheck() == BST_CHECKED);

	// History reports (condensed or otherwise)
	if (m_optHistory.GetCheck() == BST_CHECKED)
	{
		pReportInstantiator->MakeReportValue(_T("@nReportType"), m_cboReportType.GetCurSel());

		m_cboReportType.GetLBText(m_cboReportType.GetCurSel(), m_szReportType);
		pReportInstantiator->MakeReportValue(_T("@szReportType"), m_szReportType);
		pReportInstantiator->MakeReportValue(_T("@bJCInstalled"), CMS::GetSI()->ModulesInstalled.JobCost.IsUsable);

		pReportInstantiator->MakeReportResource(_T("@IDS_AP_HISTORY_REPORT"), IDS_AP_HISTORY_REPORT);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_APVENDOR"), IDS_AP_APCODE_VENDOR);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_TYPE_INVOICENUMBER_DESCRIPTION"), IDS_AP_TYPE_INVOICENUMBER_DESCRIPTION);
		pReportInstantiator->MakeReportResource(_T("@IDS_LBL_PRODUCTTYPE"), IDS_LBL_PRODUCTTYPE);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_TRAN_DATE"), IDS_AP_TRAN_DATE);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_GLDATE"), IDS_AP_GLDATE);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_DSC_DATE"), IDS_AP_DSC_DATE);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_DUEDATE"), IDS_AP_DUEDATE);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_DISC_DED_AMOUNT"), IDS_AP_DISC_DED_AMOUNT);
		pReportInstantiator->MakeReportResource(_T("@IDS_AR_ORIGINAL_AMOUNT"), IDS_AR_ORIGINAL_AMOUNT);
		pReportInstantiator->MakeReportResource(_T("@IDS_INVOICE"), IDS_INVOICE);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_PAYMENT"), IDS_AP_PAYMENT);
		pReportInstantiator->MakeReportResource(_T("@IDS_LINE"), IDS_LINE);
		pReportInstantiator->MakeReportResource(_T("@IDS_TYPE"), IDS_TYPE);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_DIS"), IDS_AP_DIS);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_STOCK_DESC_COSTCODE"), IDS_AP_STOCK_DESC_COSTCODE);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_STOCKDESCRIPTION"), IDS_AP_STOCKDESCRIPTION);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_GLEXPENSEACCT_JOBPHASE"), IDS_AP_GLEXPENSEACCT_JOBPHASE);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_GL_EXPENSE_ACCT"), IDS_AP_GL_EXPENSE_ACCT);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_EXP_DATE"), IDS_AP_EXP_DATE);
		pReportInstantiator->MakeReportResource(_T("@IDS_QUANTITY"), IDS_QUANTITY);
		pReportInstantiator->MakeReportResource(_T("@IDS_UNIT_COST"), IDS_UNIT_COST);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_EXT_COST"), IDS_AP_EXT_COST);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_TOTALS_FOR_VENDOR"), IDS_AP_TOTALS_FOR_VENDOR);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_NET_ADJUSTMENT"), IDS_AP_NET_ADJUSTMENT);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_CREDIT_ADJ"), IDS_AP_CREDIT_ADJ);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_PAYMENT_CORR"), IDS_AP_PAYMENT_CORR);
		pReportInstantiator->MakeReportResource(_T("@IDS_DISCOUNT"), IDS_DISCOUNT);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_DEBIT_ADJ"), IDS_AP_DEBIT_ADJ);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_DR_INVOICE"), IDS_AP_DR_INVOICE);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_UNAPPLIED_DR"), IDS_AP_UNAPPLIED_DR);
		pReportInstantiator->MakeReportResource(_T("@IDS_AR_GRAND_TOTALS"), IDS_AR_GRAND_TOTALS);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_APCODE_C"), IDS_AP_APCODE_C);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_TRANSACTION_DATE_RANGE_C"), IDS_AP_TRANSACTION_DATE_RANGE_C);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_PRODUCT_TYPE_C"), IDS_AP_PRODUCT_TYPE_C);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_VENDOR_NAME_C"), IDS_AP_VENDOR_NAME_C);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_VENDOR_NUMBER_C"), IDS_AP_VENDOR_NUMBER_C);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_GROUP_BY_APCODE_C"), IDS_AP_GROUP_BY_APCODE_C);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_TRANSACTION_TYPE_C"), IDS_AP_TRANSACTION_TYPE_C);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_VENDOR_STATUS_C"), IDS_AP_VENDOR_STATUS_C);
		pReportInstantiator->MakeReportResource(_T("@IDS_REPORT_TYPE"), IDS_REPORT_TYPE);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_PAGE_BREAK_C"), IDS_AP_PAGE_BREAK_C);
		pReportInstantiator->MakeReportResource(_T("@IDS_SORT_BY"), IDS_SORT_BY);
		pReportInstantiator->MakeReportResource(_T("@IDS_LBL_REPORTSELECTIONCRITERIA"), IDS_REPORT_SELECTION_CRITERIA);
		pReportInstantiator->MakeReportResource(_T("@IDS_REPORT_SUMMARY"), IDS_REPORT_SUMMARY);
		pReportInstantiator->MakeReportResource(_T("@IDS_END"), IDS_END);
		pReportInstantiator->MakeReportResource(_T("@IDS_START"), IDS_START);
		pReportInstantiator->MakeReportResource(_T("@IDS_YES"), IDS_YES);
		pReportInstantiator->MakeReportResource(_T("@IDS_NO"), IDS_NO);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_TOTALS_FOR_APCODE"), IDS_AP_TOTALS_FOR_APCODE);
		pReportInstantiator->MakeReportResource(_T("@IDS_INVOICE"), IDS_INVOICE);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_LBL_CMB_TTYPE_CREDIT_ADJUSTMENT"), IDS_AP_LBL_CMB_TTYPE_CREDIT_ADJUSTMENT);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_PAYMENT_CORRECTION"), IDS_AP_PAYMENT_CORRECTION);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_PAYMENT"), IDS_AP_PAYMENT);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_LBL_CMB_TTYPE_DEBIT_ADJUSTMENT"), IDS_AP_LBL_CMB_TTYPE_DEBIT_ADJUSTMENT);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_DEBIT_INVOICES"), IDS_AP_DEBIT_INVOICES);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_UNAPPLIED_DEBITS"), IDS_AP_UNAPPLIED_DEBITS);
		pReportInstantiator->MakeReportResource(_T("@IDS_UNAPPLIED_TRANSACTION"), IDS_UNAPPLIED_TRANSACTION);
		pReportInstantiator->MakeReportResource(_T("@IDS_AR_UNAPPLIED_PAYMENT"), IDS_AR_UNAPPLIED_PAYMENT);
		//DD - 04/29/2021 - Bug 57320 - Add Paid Outs
		pReportInstantiator->MakeReportResource(_T("@IDS_PAIDOUTID"), IDS_PAIDOUTID);
	}
	else if ((m_optInvoiceToPayment.GetCheck() == BST_CHECKED) || (m_optPaymentToInvoice.GetCheck() == BST_CHECKED))
	{
		if (m_optInvoiceToPayment.GetCheck() == BST_CHECKED)
			pReportInstantiator->MakeReportResource(_T("@IDS_INVOICE_TO_PAYMENT_REPORT"), IDS_INVOICE_TO_PAYMENT_REPORT);

		if (m_optPaymentToInvoice.GetCheck() == BST_CHECKED)
			pReportInstantiator->MakeReportResource(_T("@IDS_PAYMENT_TO_INVOICE_REPORT"), IDS_PAYMENT_TO_INVOICE_REPORT);

		pReportInstantiator->MakeReportResource(_T("@IDS_AP_APVENDOR"), IDS_AP_APCODE_VENDOR);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_REFERENCENUM_DESCRIPTION"), IDS_AP_REFERENCENUM_DESCRIPTION);
		pReportInstantiator->MakeReportResource(_T("@IDS_DATE"), IDS_DATE);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_APPLY_DISC_AMT"), IDS_AP_APPLY_DISC_AMT);
		pReportInstantiator->MakeReportResource(_T("@IDS_AMOUNT"), IDS_AMOUNT);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_INVOICES"), IDS_LKP_INVOICES); // DRuiz 1-31704 01/12/09
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_CREDIT_ADJUSTMENTS"), IDS_AP_CREDIT_ADJUSTMENTS);
		pReportInstantiator->MakeReportResource(_T("@IDS_PAYMENT_CORRECTIONS"), IDS_PAYMENT_CORRECTIONS);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_PAYMENTS"), IDS_AP_PAYMENTS);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_DEBIT_ADJUSTMENTS"), IDS_AP_DEBIT_ADJUSTMENTS);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_DEBIT_INVOICES"), IDS_AP_DEBIT_INVOICES);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_UNAPPLIED_DEBITS"), IDS_AP_UNAPPLIED_DEBITS);
		pReportInstantiator->MakeReportResource(_T("@IDS_FOR"), IDS_FOR);
		pReportInstantiator->MakeReportResource(_T("@IDS_INVOICE"), IDS_INVOICE);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_LBL_CMB_TTYPE_CREDIT_ADJUSTMENT"), IDS_AP_LBL_CMB_TTYPE_CREDIT_ADJUSTMENT);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_PAYMENT_CORRECTION"), IDS_AP_PAYMENT_CORRECTION);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_PAYMENT"), IDS_AP_PAYMENT);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_LBL_CMB_TTYPE_UNAPPLIED_DEBIT"), IDS_AP_LBL_CMB_TTYPE_UNAPPLIED_DEBIT);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_DR_INVOICE"), IDS_AP_DR_INVOICE);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_DISCOUNT_C"), IDS_AP_DISCOUNT_C);
		pReportInstantiator->MakeReportResource(_T("@IDS_REPORT_SUMMARY"), IDS_REPORT_SUMMARY);
		pReportInstantiator->MakeReportResource(_T("@IDS_LBL_REPORTSELECTIONCRITERIA"), IDS_LBL_REPORTSELECTIONCRITERIA);
		pReportInstantiator->MakeReportResource(_T("@IDS_SORT_BY"), IDS_SORT_BY);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_PAGE_BREAK_C"), IDS_AP_PAGE_BREAK_C);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_VENDOR_STATUS_C"), IDS_AP_VENDOR_STATUS_C);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_TRANSACTION_TYPE_C"), IDS_AP_TRANSACTION_TYPE_C);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_VENDOR_NUMBER_C"), IDS_AP_VENDOR_NUMBER_C);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_VENDOR_NAME_C"), IDS_AP_VENDOR_NAME_C);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_PRODUCT_TYPE_C"), IDS_AP_PRODUCT_TYPE_C);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_TRANSACTION_DATE_RANGE_C"), IDS_AP_TRANSACTION_DATE_RANGE_C);
		pReportInstantiator->MakeReportResource(_T("@IDS_AP_APCODE_C"), IDS_AP_APCODE_C);
		pReportInstantiator->MakeReportResource(_T("@IDS_END"), IDS_END);
		pReportInstantiator->MakeReportResource(_T("@IDS_START"), IDS_START);
		pReportInstantiator->MakeReportResource(_T("@IDS_YES"), IDS_YES);
		pReportInstantiator->MakeReportResource(_T("@IDS_NO"), IDS_NO);
		pReportInstantiator->MakeReportResource(_T("@IDS_DISCOUNT"), IDS_DISCOUNT);
		// for drilldown reporting
		pReportInstantiator->MakeReportResource(_T("@IDS_DRILLDOWNSUBHEADING"), IDS_AP_DRILL_DOWN_TO_VENDOR_HISTORY);
	}
}
