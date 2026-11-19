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
 *	Created:	10/13/2006
 *	Author:		Andy Swanson
 *  Version 2.5 Feature 12
 ******************************************************************************/
#include "stdafx.h"
#include "PPG_APAgingReportStd.h"
#include "RFV_APAgingReportStd.h"
 // CPPG_APAgingReportStd

IMPLEMENT_DYNCREATE(CPPG_APAgingReportStd, CPPG_APTotalDebtReportStd)

CPPG_APAgingReportStd::CPPG_APAgingReportStd()
	: CPPG_APTotalDebtReportStd(CPPG_APAgingReportStd::IDD)
{
	// InTimeTec: 10.25.2012.HS - Custom Reports
	m_bolAgingReport = true;
	// InTimeTec: End
}

CPPG_APAgingReportStd::~CPPG_APAgingReportStd()
{
}

void CPPG_APAgingReportStd::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_APF_CHK_RDO_AGE_BY_DUE_DATE, m_optAgeByDueDate);
	DDX_Control(pDX, IDC_APF_CHK_RDO_AGE_BY_INVOICE_DATE, m_optAgeByInvoiceDate);
	// InTimeTec: 10.22.2012.HS - Custom Reports
	DDX_Control(pDX, IDC_APF_CHK_USE_CUSTOM_REPORT, m_chkUseCustomReport);
	DDX_CMSControl(pDX, IDC_APF_CBO_CUSTOM_REPORTS, m_cboCustomReports);
	// InTimeTec: End
}

BEGIN_MESSAGE_MAP(CPPG_APAgingReportStd, CPPG_APTotalDebtReportStd)
	ON_BN_CLICKED(IDC_APF_CHK_RDO_AGE_BY_DUE_DATE, &CPPG_APAgingReportStd::OnClickedAgeByDate)
	ON_BN_CLICKED(IDC_APF_CHK_RDO_AGE_BY_INVOICE_DATE, &CPPG_APAgingReportStd::OnClickedAgeByDate)
	ON_EN_KILLFOCUS(IDC_APF_TXT_AGINGPERIODSSTART, OnEnKillfocusTxtPeriodStart)
	ON_EN_KILLFOCUS(IDC_APF_TXT_AGINGPERIODSEND, OnEnKillfocusTxtPeriodEnd)
	ON_EN_SETFOCUS(IDC_APF_TXT_AGINGPERIODSSTART, OnEnSetfocusTxtPeriodStart)
	ON_EN_SETFOCUS(IDC_APF_TXT_AGINGPERIODSEND, OnEnSetfocusTxtPeriodEnd)
	ON_EN_CHANGE(IDC_APF_TXT_AGINGPERIODSSTART, OnEnChangeTxtPeriodStart)
	ON_EN_CHANGE(IDC_APF_TXT_AGINGPERIODSEND, OnEnChangeTxtPeriodEnd)
	// InTimeTec: 10.22.2012.HS - Custom Reports
	ON_BN_CLICKED(IDC_APF_CHK_USE_CUSTOM_REPORT, &CPPG_APAgingReportStd::EnableCustomReportCombo)
	// InTimeTec: End
END_MESSAGE_MAP()

// CPPG_APTotalDebtReportStd diagnostics

#ifdef _DEBUG
void CPPG_APAgingReportStd::AssertValid() const
{
	__super::AssertValid();
}

void CPPG_APAgingReportStd::Dump(CDumpContext& dc) const
{
	__super::Dump(dc);
}
#endif //_DEBUG

//**************************************************************************
//* Procedure:     LoadResources
//*
//* Description:   Contains code to set the control Captions.
//*
//* Parameters:    None
//*
//* Date Created:  10/26/2005 by Mike Gregg
//**************************************************************************
void CPPG_APAgingReportStd::LoadResources()
{
	__super::LoadResources();

	SetResourceText(IDC_APF_CHK_RDO_AGE_BY_DUE_DATE, IDS_AP_LBL_AGE_BY_DUE_DATE);
	SetResourceText(IDC_APF_CHK_RDO_AGE_BY_INVOICE_DATE, IDS_AP_LBL_AGE_BY_INVOICE_DATE);
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
void CPPG_APAgingReportStd::InitControls()
{
	//Make a call to the base; this does most of the work.
	__super::InitControls();

	m_optAgeByDueDate.SetCheck(1);
	m_optAgeByDueDate.ShowWindow(true);
	m_optAgeByInvoiceDate.ShowWindow(true);

	// InTimeTec: 10.23.2012.HS - Custom Reports
	m_cboCustomReports.EnableWindow(false);
	m_clsCustomOptions.m_szModule = CMS::GetSI()->ApplicationSettings.ModuleStringID;
	m_clsCustomOptions.m_szReportType = CGBLResources::GetResourceString(IDS_AGING);
	m_clsCustomOptions.SetCustomReportsCheckBox(m_cboCustomReports, m_chkUseCustomReport);
	// InTimeTec: End
}


int CPPG_APAgingReportStd::LoadReportTypeCombo()
{
	m_cboReportType.ResetContent();
	AddIDSToCombo(m_cboReportType, IDS_AR_DETAILED);
	AddIDSToCombo(m_cboReportType, IDS_SUMMARY);
	AddIDSToCombo(m_cboReportType, IDS_AR_CONDENSED);
	AddIDSToCombo(m_cboReportType, IDS_AR_SINGLE_LINE);
	AddIDSToCombo(m_cboReportType, IDS_SA_DETAILED_WITH_NOTES);
	m_cboReportType.SetCurSel(0);
	return m_cboReportType.GetCount();
}

void CPPG_APAgingReportStd::SetControlSizes()
{
	__super::SetControlSizes();

	m_pEXLayoutManager->SizeToFit(&m_optAgeByDueDate, SIZ_CheckBox);
	m_pEXLayoutManager->SizeToFit(&m_optAgeByInvoiceDate, SIZ_CheckBox);
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
void CPPG_APAgingReportStd::SetupLayoutManager()
{
	m_pEXLayoutManager->AnchorControlTL(&m_optAgeByDueDate, &m_cboReportType, &m_cboReportType, 7, 20);
	m_pEXLayoutManager->AnchorControlTL(&m_optAgeByInvoiceDate, &m_optAgeByDueDate, &m_cboReportType, 0, 20);


	__super::SetupLayoutManager();

	// InTimeTec: 10.23.2012.HS - Custom Reports
	m_pEXLayoutManager->AnchorControlTop(&m_chkUseCustomReport, m_pUDFStartPos);
	m_pEXLayoutManager->AnchorControlSameLeft(&m_chkUseCustomReport, m_pUDFStartPos);
	m_pEXLayoutManager->AnchorControlSameTop(&m_cboCustomReports, &m_chkUseCustomReport);
	m_pEXLayoutManager->AnchorControlLeft(&m_cboCustomReports, &m_chkUseCustomReport);
	m_pEXLayoutManager->AnchorControlRight(&m_cboCustomReports);
	// InTimeTec: End
}

void CPPG_APAgingReportStd::OnClickedAgeByDate()
{
	if (m_optAgeByDueDate.GetCheck())
		m_lblDueDate.SetWindowText(GetResStr(IDS_AP_LBL_DUE_DATE));
	else
		m_lblDueDate.SetWindowText(GetResStr(IDS_TRANSACDATE));
}

bool    CPPG_APAgingReportStd::GetAgeByDueDate() { return m_optAgeByDueDate.GetCheck() == BST_CHECKED; }
CString CPPG_APAgingReportStd::GetReportName() { return _T("Aging"); }
UINT    CPPG_APAgingReportStd::GetReportTitleID() { return IDS_AP_PO_AGING_REPORT; }

void CPPG_APAgingReportStd::SetReportValues(CMSGBL::IReportInstantiator * pReportInstantiator)
{
	// The aging report defers its report processing to its parent class, CPPG_APTotalDebtReportStd.

	// InTimeTec: 10.24.2012.HS - Custom Reports
	if (m_chkUseCustomReport.GetCheck() == BST_CHECKED)
	{
		m_clsCustomOptions.SetCustomReportPath(pReportInstantiator, m_cboCustomReports);
	}
	// InTimeTec: End

  // begbert 06-29-2006 1-24614 : there is a separate report file for 'single line' reports.
	if (m_cboReportType.GetItemData(m_cboReportType.GetCurSel()) == IDS_AR_SINGLE_LINE)
		pReportInstantiator->SetReportName(RPT_AP_AGING_SINGLE_LINE);
	else
		pReportInstantiator->SetReportName(RPT_AP_AGING);

	__super::SetReportValues(pReportInstantiator);
	pReportInstantiator->MakeReportResource(_T("@IDS_AP_NON_1099_PAYMENTS"), IDS_AP_NON_1099_PAYMENTS);
	pReportInstantiator->MakeReportResource(_T("@IDS_AP_RENTS"), IDS_AP_RENTS);
	pReportInstantiator->MakeReportResource(_T("@IDS_AP_ROYALTIES"), IDS_AP_ROYALTIES);
	pReportInstantiator->MakeReportResource(_T("@IDS_AP_OTHER_INCOME"), IDS_AP_OTHER_INCOME);
	pReportInstantiator->MakeReportResource(_T("@IDS_FED_INCOMETAX_WITHHELD"), IDS_FED_INCOMETAX_WITHHELD);
	pReportInstantiator->MakeReportResource(_T("@IDS_AP_FISHING_BOAT_PROCEEDS"), IDS_AP_FISHING_BOAT_PROCEEDS);
	pReportInstantiator->MakeReportResource(_T("@IDS_AP_MEDICAL_HEALTH_PAYMENTS"), IDS_AP_MEDICAL_HEALTH_PAYMENTS);
	pReportInstantiator->MakeReportResource(_T("@IDS_AP_NONEMPLOYEE_COMPENSATION"), IDS_AP_NONEMPLOYEE_COMPENSATION);
	pReportInstantiator->MakeReportResource(_T("@IDS_AP_SUBSTITUTE_PAYMENTS"), IDS_AP_SUBSTITUTE_PAYMENTS);
	pReportInstantiator->MakeReportResource(_T("@IDS_AP_CROP_INSURANCE_PROCEEDS"), IDS_AP_CROP_INSURANCE_PROCEEDS);
	pReportInstantiator->MakeReportResource(_T("@IDS_AP_EXCESS_GOLDEN_PARACHUTE"), IDS_AP_EXCESS_GOLDEN_PARACHUTE);
	pReportInstantiator->MakeReportResource(_T("@IDS_AP_GROSS_PROCEEDS_TO_ATTORNEY"), IDS_AP_GROSS_PROCEEDS_TO_ATTORNEY);
	pReportInstantiator->MakeReportResource(_T("@IDS_AP_SECTION_409A_DEFERRALS"), IDS_AP_SECTION_409A_DEFERRALS);
	//RCG - 11/17/2020 - PBI 56395 - Section 409A Income was renamed for 2020
	pReportInstantiator->MakeReportResource(_T("@IDS_AP_NONQUALIFIED_DEFERRED_COMPENSATION"), IDS_AP_NONQUALIFIED_DEFERRED_COMPENSATION);
	//JB - 12/07/2021 - PBI 59015 - Added new 1099 type
	pReportInstantiator->MakeReportResource(_T("@IDS_AP_FISH_PURCHASED_FOR_RESALE"), IDS_AP_FISH_PURCHASED_FOR_RESALE);
	pReportInstantiator->MakeReportResource(_T("@IDS_UNAPPLIED_TRANSACTION"), IDS_UNAPPLIED_TRANSACTION);
	//HP - 7/7/2020 - Bug 53716 - Added strBillNotes
	if (m_cboReportType.GetItemData(m_cboReportType.GetCurSel()) != IDS_AR_SINGLE_LINE)
	{
		pReportInstantiator->MakeReportResource(_T("@IDS_VENDOR_NOTES"), IDS_VENDORNOTES);
		pReportInstantiator->MakeReportResource(_T("@IDS_BILL_NOTES"), IDS_AP_BILL_NOTES);
	}
}

// InTimeTec: 10.24.2012.HS - Custom Reports
void CPPG_APAgingReportStd::GetFormBounds(CWnd*& pRightBound, CWnd*& pBottomBound)
{
	pRightBound = &m_chkGroupByAPCode;

	if (pBottomBound)
	{
		m_pEXLayoutManager->AnchorControlTop(&m_chkUseCustomReport, pBottomBound);
		m_pEXLayoutManager->AnchorControlSameLeft(&m_chkUseCustomReport, pBottomBound);

		m_pEXLayoutManager->RedrawLayout();
	}

	pBottomBound = &m_chkUseCustomReport;

	__super::GetFormBounds(pRightBound, pBottomBound);
}

void CPPG_APAgingReportStd::AdjustDialog(CWnd* pBottomControlWnd)
{
	GetUDFBottomControl(pBottomControlWnd);
	pBottomControlWnd = &m_chkUseCustomReport;
	__super::AdjustDialog(pBottomControlWnd);
}

// This function enables/disables the 'custom reports Combo Box' based
// on whether 'Use Custom Reports CheckBox' is checked/unchecked.
void CPPG_APAgingReportStd::EnableCustomReportCombo()
{
	if (m_chkUseCustomReport.GetCheck() == BST_CHECKED)
	{
		m_cboCustomReports.EnableWindow(true);
	}
	else
	{
		m_cboCustomReports.EnableWindow(false);
	}
}
// InTimeTec: End

//void CPPG_APAgingReportStd::AdjustDialog()
//{
//	if(!m_bIntDialog)
//	{
//		CWnd* pControlWnd = &m_txtAPCodeStart ;
//		GetUDFBottomControl(pControlWnd);
//
//		CWnd* pWnd = this->GetParent()->GetParent();
//		CArray<HWND, HWND> aChildren;
//		CGBLForm::GetChildWindows(pWnd, &aChildren);
//		CCMSFormEx *pDialog = (CCMSFormEx*)this->GetParent()->GetParent();
//		CButtonPanel*& pButtonPanel = pDialog->GetButtonPanel();
//		if (pWnd->GetDlgItem(IDC_CMS_BUTTONPANEL))
//		{
//			CRect rect;
//			CRect rectCtrl;
//			CRect rectSheet;
//			CRect rectBtn;
//			CGBLForm::GetTrueWindowRect(pWnd->GetParent()->GetParent(), pDialog->GetParent(), rect);
//			CGBLForm::GetTrueWindowRect(pControlWnd->GetParent()->GetParent()->GetParent()
//																, pControlWnd, rectCtrl);
//
//			CPropertySheet *pPlaceHolder = (CPropertySheet*)pDialog->GetDlgItem(pDialog->m_clsPropSheet.GetDlgCtrlID());
//			pPlaceHolder->GetWindowRect(rectSheet);
//			pWnd->ScreenToClient(rectSheet);
//			rectSheet.bottom = rectCtrl.bottom +20;
//			CGBLForm::EkPositionEmbeddedPropertySheet(pWnd, pPlaceHolder, rectSheet);
//
//			CGBLForm::GetTrueWindowRect(pControlWnd->GetParent()->GetParent()->GetParent()->GetParent()->GetParent()->GetParent()
//													, pControlWnd, rectCtrl);
//
//			rect.bottom = rectCtrl.bottom +40;
//			pDialog->GetParent()->MoveWindow(&rect);
//
//			pButtonPanel->GetWindowRect(rectBtn);
//			pWnd->ScreenToClient(rectBtn);
//			int nHeight = rectBtn.bottom - rectBtn.top;
//			rectBtn.top = rectSheet.bottom + 5;
//			rectBtn.bottom = nHeight + rectBtn.top;
//			pButtonPanel->MoveWindow(rectBtn);
//
//			pDialog->RedrawWindow();
//		}
//	}

//}
