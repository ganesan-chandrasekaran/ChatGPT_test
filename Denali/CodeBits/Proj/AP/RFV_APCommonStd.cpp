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
 * Initial code - mgregg
 ******************************************************************************/
#include "stdafx.h"
#include "RFV_APCommonStd.h"

using namespace UDFConstants;


// convenience helper functions:

void CRFV_APCommonStd::ConformSize(ESizeModes mode, CWnd* w1, CWnd* w2)
{	// This makes the two controls both as wide and/or tall as the wider/taller of them.
	CRect r1, r2;
	int diff;

	w1->GetWindowRect(r1);
	w2->GetWindowRect(r2);
	if (mode & eSZ_WIDTH)
	{
		diff = r1.Width() - r2.Width();
		if (diff < 0) r1.right -= diff; else r2.right += diff;
	}
	if (mode & eSZ_HEIGHT)
	{
		diff = r1.Height() - r2.Height();
		if (diff < 0) r1.bottom -= diff; else r2.bottom += diff;
	}
	w1->MoveWindow(r1, 0);
	w2->MoveWindow(r2, 0);
	w1->GetWindowRect(r1);
	w2->GetWindowRect(r2);
}

// count MUST list the accurate number of window pointers passed into the function!
void CRFV_APCommonStd::ConformSize(ESizeModes mode, size_t count, ...)
{
	CRect rect;
	std::vector<CWnd*> wndps;
	std::vector<CRect> rects;
	va_list marker;
	size_t i;
	int w, h;

	va_start(marker, count);
	for (i = 0; i < count; i++)
	{
		wndps.push_back(va_arg(marker, CWnd*));
		wndps[i]->GetWindowRect(rect);
		rects.push_back(rect);
	}
	va_end(marker);

	w = h = 0;
	for (i = 0; i < count; i++)
	{
		if (mode & eSZ_WIDTH) w = max(w, rects[i].Width());
		if (mode & eSZ_HEIGHT) h = max(h, rects[i].Height());
	}

	for (i = 0; i < count; i++)
	{
		rect = rects[i];
		if (mode & eSZ_WIDTH) rect.right = rect.left + w;
		if (mode & eSZ_HEIGHT) rect.bottom = rect.top + h;
		wndps[i]->MoveWindow(rect, 0);
	}
}


IMPLEMENT_DYNCREATE(CRFV_APCommonStd, CReportFormDrillDownView)

CRFV_APCommonStd::CRFV_APCommonStd(UINT IDD)
	: CReportFormDrillDownView(IDD)//,CGBLReportUserDefinesStd()
{
	m_pLookup = NULL;
	m_bEXMode = true;
	//Setup the user defines
	// begbert 11-18-2009 1-31429 : Set them up for the AP Module specifically
	//   in case these forms are accessed from other modules.
	SetupUserDefines(IDS_AP_MODULE);
	SetUsePageBreak(false);
}

CRFV_APCommonStd::~CRFV_APCommonStd()
{
}

void CRFV_APCommonStd::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	//Sort By Combo
	DDX_CMSControl(pDX, IDC_APF_CBO_SORTBY, m_cboSortBy, IDC_APF_LBL_SORTBY);
	//Vendor Status Combo
	DDX_CMSControl(pDX, IDC_APF_CBO_VENDOR_STATUS, m_cboVendorStatus, IDC_APF_LBL_VENDOR_STATUS);
	//Additional Filter Combo
	DDX_CMSControl(pDX, IDC_APF_CBO_ADDITIONAL_FILTER, m_cboFilter, IDC_APF_LBL_ADDITIONAL_FILTER);

	//Start label
	DDX_Control(pDX, IDC_APF_LBL_START, m_lblStart);
	//End Label
	DDX_Control(pDX, IDC_APF_LBL_END, m_lblEnd);
	//Vendor Number start autofill edit control
	DDX_CMSControl(pDX, IDC_APF_TXT_START_VENDOR_NUMBER, m_txtVendNumStart, IDC_APF_LBL_VENDOR_NUMBER, IDC_APF_LBL_START, true);
	//Vendor Number end autofill edit control
	DDX_CMSControl(pDX, IDC_APF_TXT_END_VENDOR_NUMBER, m_txtVendNumEnd, IDC_APF_LBL_VENDOR_NUMBER, IDC_APF_LBL_END, false);
	//Vendor Name start autofill edit control
	DDX_CMSControl(pDX, IDC_APF_TXT_START_VENDOR_NAME, m_txtVendNameStart, IDC_APF_LBL_VENDOR_NAME, IDC_APF_LBL_START, true);
	//Vendor Name end autofill edit control
	DDX_CMSControl(pDX, IDC_APF_TXT_END_VENDOR_NAME, m_txtVendNameEnd, IDC_APF_LBL_VENDOR_NAME, IDC_APF_LBL_END, false);
	//Product Type start autofill edit control
	DDX_CMSControl(pDX, IDC_APF_TXT_START_PRODUCT_TYPE, m_txtProdTypeStart, IDC_APF_LBL_PRODUCT_TYPE, IDC_APF_LBL_START, true);
	//Product Type end autofill edit control
	DDX_CMSControl(pDX, IDC_APF_TXT_END_PRODUCT_TYPE, m_txtProdTypeEnd, IDC_APF_LBL_PRODUCT_TYPE, IDC_APF_LBL_END, false);
	//AP Code start autofill edit control
	DDX_CMSControl(pDX, IDC_APF_TXT_START_APCODE, m_txtAPCodeStart, IDC_APF_LBL_APCODE, IDC_APF_LBL_START, true);
	//AP Code start autofill edit control
	DDX_CMSControl(pDX, IDC_APF_TXT_END_APCODE, m_txtAPCodeEnd, IDC_APF_LBL_APCODE, IDC_APF_LBL_END, false);
	//Group By AP checkbox
	DDX_Control(pDX, IDC_APF_CHK_GROUP_BY_APCODE, m_chkGroupByAPCode);


	//Sort By UDF start autofill edit control
	DDX_CMSControl(pDX, IDC_ARF_TXT_START_SORT_UDF, m_txtSortByUDFStart, IDC_ARF_LBL_UDF1, IDC_APF_LBL_START, true);
	//Sort By UDF end autofill edit control
	DDX_CMSControl(pDX, IDC_ARF_TXT_END_SORT_UDF, m_txtSortByUDFEnd, IDC_ARF_LBL_UDF1, IDC_APF_LBL_END, false);

	DDX_Control(pDX, IDC_ARF_DTE_START_SORT_UDF, m_dteSortByUDFStart);
	DDX_CMSDate(pDX, IDC_ARF_DTE_START_SORT_UDF, m_odtSortByUDFStart);
	DDX_Control(pDX, IDC_ARF_DTE_END_SORT_UDF, m_dteSortByUDFEnd);
	DDX_CMSDate(pDX, IDC_ARF_DTE_END_SORT_UDF, m_odtSortByUDFEnd);

	DDX_Control(pDX, IDC_ARF_TXT_START_SORTBYUDF_NUMBER, m_txtNumSortByUDFStart);
	DDX_Text(pDX, IDC_ARF_TXT_START_SORTBYUDF_NUMBER, m_szNumSortByUDFStart);
	DDX_Control(pDX, IDC_ARF_TXT_END_SORTBYUDF_NUMBER, m_txtNumSortByUDFEnd);
	DDX_Text(pDX, IDC_ARF_TXT_END_SORTBYUDF_NUMBER, m_szNumSortByUDFEnd);

	//Additional Filter UDF start autofill edit control
	DDX_CMSControl(pDX, IDC_ARF_TXT_START_FILTER_UDF, m_txtFilterUDFStart, IDC_ARF_LBL_UDF2, IDC_APF_LBL_START, true);
	//Additional Filter UDF end autofill edit control
	DDX_CMSControl(pDX, IDC_ARF_TXT_END_FILTER_UDF, m_txtFilterUDFEnd, IDC_ARF_LBL_UDF2, IDC_APF_LBL_END, false);

	DDX_Control(pDX, IDC_ARF_DTE_START_FILTER_UDF, m_dteFilterUDFStart);
	DDX_CMSDate(pDX, IDC_ARF_DTE_START_FILTER_UDF, m_odtFilterUDFStart);
	DDX_Control(pDX, IDC_ARF_DTE_END_FILTER_UDF, m_dteFilterUDFEnd);
	DDX_CMSDate(pDX, IDC_ARF_DTE_END_FILTER_UDF, m_odtFilterUDFEnd);

	DDX_Control(pDX, IDC_ARF_TXT_START_ADDFILTERUDF_NUMBER, m_txtNumFilterUDFStart);
	DDX_Text(pDX, IDC_ARF_TXT_START_ADDFILTERUDF_NUMBER, m_szNumFilterUDFStart);
	DDX_Control(pDX, IDC_ARF_TXT_END_ADDFILTERUDF_NUMBER, m_txtNumFilterUDFEnd);
	DDX_Text(pDX, IDC_ARF_TXT_END_ADDFILTERUDF_NUMBER, m_szNumFilterUDFEnd);

	//OK Button
	// 1-28836 BK 4/4/08 changed buttonID from IDOK to work with changes in resource file
	DDX_CMSControl(pDX, IDC_APF_BTN_OK, m_btnOK);
	//Cancel Button
	DDX_CMSControl(pDX, IDCANCEL, m_btnCancel);
	//Help Button
	DDX_CMSControl(pDX, IDHELP, m_btnHelp);
}

BEGIN_MESSAGE_MAP(CRFV_APCommonStd, CReportFormDrillDownView)

	ON_EN_SETFOCUS(IDC_ARF_TXT_START_SORTBYUDF_NUMBER, &CRFV_APCommonStd::OnEnSetFocusNumSortByUDFStart)
	ON_EN_SETFOCUS(IDC_ARF_TXT_END_SORTBYUDF_NUMBER, &CRFV_APCommonStd::OnEnSetFocusNumSortByUDFEnd)
	ON_EN_SETFOCUS(IDC_ARF_TXT_START_ADDFILTERUDF_NUMBER, &CRFV_APCommonStd::OnEnSetFocusNumFilterUDFStart)
	ON_EN_SETFOCUS(IDC_ARF_TXT_END_ADDFILTERUDF_NUMBER, &CRFV_APCommonStd::OnEnSetFocusNumFilterUDFEnd)
	ON_CBN_SELCHANGE(IDC_APF_CBO_SORTBY, &CRFV_APCommonStd::OnCbnSelchangeCboSortby)
	ON_CBN_SELCHANGE(IDC_APF_CBO_ADDITIONAL_FILTER, &CRFV_APCommonStd::OnCbnSelchangeCboAddfilter)

END_MESSAGE_MAP()


// CRFV_ARCommonStd diagnostics

#ifdef _DEBUG
void CRFV_APCommonStd::AssertValid() const
{
	CReportFormDrillDownView::AssertValid();
}

void CRFV_APCommonStd::Dump(CDumpContext& dc) const
{
	CReportFormDrillDownView::Dump(dc);
}
#endif //_DEBUG


// CRFV_ARCommonStd message handlers

void CRFV_APCommonStd::OnInitialUpdate()
{
	__super::OnInitialUpdate();
}

//**************************************************************************
//* Procedure:     LoadResources
//*
//* Description:   Contains code to set the control captions.
//*
//* Parameters:    None
//*
//* Date Created:  10/25/2005 by Mike Gregg - adapted by begbert 12-02-2005
//**************************************************************************
void CRFV_APCommonStd::LoadResources()
{
	__super::LoadResources();

	m_cboSortBy.SetCaption(IDS_LBL_SORTBY);
	m_cboVendorStatus.SetCaption(IDS_AP_VENDOR_STATUS);
	m_cboFilter.SetCaption(IDS_LBL_ADDFILTER);
	m_txtVendNumStart.SetCaption(IDS_VENDOR_NUMBER);
	m_txtVendNameStart.SetCaption(IDS_VENDOR_NAME);
	m_txtProdTypeStart.SetCaption(IDS_PRODUCTTYPE);
	m_txtAPCodeStart.SetCaption(IDS_APCODE);

	SetResourceText(IDC_APF_CHK_GROUP_BY_APCODE, IDS_AP_GROUP_BY_APCODE);

	SetResourceText(IDC_APF_LBL_START, IDS_START);
	SetResourceText(IDC_APF_LBL_END, IDS_END);

	//Version 2.5 Feature 10 AES
	if (GetDlgItem(IDC_LBL_DATE_RANGE))
		SetResourceText(IDC_LBL_DATE_RANGE, IDS_SA_DATERANGE);
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
void CRFV_APCommonStd::InitControls()
{
	//Make a call to the base
	__super::InitControls();

	//LoadAPCommonCombos();

	//Load the Sort By combo
	LoadSortByCombo();
	//Load the Vendor Status combo
	LoadVendorStatusCombo();
	//Load the Additional Filter UDFs into the combo
	LoadAddFilterUDF(m_cboFilter);
	//Set up the autofill edit controls
	InitAutofillControls();
	//Set up the UDF autofill edit controls
	InitSortByUDFAutofill();
	//Set up the Additional Filter UDF edit controls
	InitAddFilterUDFAutofill();
	//Set up the Sort By UDF date controls
	InitDateUDFControls(m_dteSortByUDFStart, m_dteSortByUDFEnd);
	//Set up the Additional Filter UDF date controls
	InitDateUDFControls(m_dteFilterUDFStart, m_dteFilterUDFEnd);

	InitNumUDFControls(m_txtNumSortByUDFStart, m_txtNumSortByUDFEnd);
	InitNumUDFControls(m_txtNumFilterUDFStart, m_txtNumFilterUDFEnd);

	// properties for vendor number controls
	m_txtVendNumStart.SetControlProperties(50, 30);
	m_txtVendNumEnd.SetControlProperties(50, 30);

	// properties for vendor name controls
	m_txtVendNameStart.SetControlProperties(50, 30);
	m_txtVendNameEnd.SetControlProperties(50, 30);

	// properties for product type controls
	m_txtProdTypeStart.SetControlProperties(15, 15);
	m_txtProdTypeEnd.SetControlProperties(15, 15);

	m_txtProdTypeStart.SetDataDefaultStart();
	m_txtProdTypeEnd.SetDataDefaultEnd();

	// properties for product type controls
	m_txtAPCodeStart.SetControlProperties(6, 6);
	m_txtAPCodeEnd.SetControlProperties(6, 6);

	//HideUDFControls();

	//Hide the UDF controls until a UDF is selected from the sort by combo or the additional filter combo
	m_txtSortByUDFStart.GetWndLabelPtr()->ShowWindow(SW_HIDE);
	m_txtFilterUDFStart.GetWndLabelPtr()->ShowWindow(SW_HIDE);
	m_txtSortByUDFStart.ShowWindow(SW_HIDE);
	m_txtSortByUDFEnd.ShowWindow(SW_HIDE);
	m_txtFilterUDFStart.ShowWindow(SW_HIDE);
	m_txtFilterUDFEnd.ShowWindow(SW_HIDE);
	m_txtNumSortByUDFStart.ShowWindow(SW_HIDE);
	m_txtNumSortByUDFEnd.ShowWindow(SW_HIDE);
	m_txtNumFilterUDFStart.ShowWindow(SW_HIDE);
	m_txtNumFilterUDFEnd.ShowWindow(SW_HIDE);
	m_dteSortByUDFStart.ShowWindow(SW_HIDE);
	m_dteSortByUDFEnd.ShowWindow(SW_HIDE);
	m_dteFilterUDFStart.ShowWindow(SW_HIDE);
	m_dteFilterUDFEnd.ShowWindow(SW_HIDE);

	//1-24574 AES need to set the range and dates
	COleDateTime dteOleStarting(1980, 01, 01, 0, 0, 0);
	if (m_txtDueDateStart.GetSafeHwnd())
	{
		m_txtDueDateStart.SetRange(&dteOleStarting, &si->FiscalCalendar.EndDate);
		m_txtDueDateEnd.SetRange(&dteOleStarting, &si->FiscalCalendar.EndDate);
		m_txtDueDateStart.SetDate(si->FiscalCalendar.StartDate);
		m_txtDueDateEnd.SetDate(si->FiscalCalendar.EndDate);
	}

	//Version 2.5 Feature 10 AES
	if (GetDlgItem(IDC_LBL_DATE_RANGE))
		m_pCMSDate_RangeComboBox.InitializeDateControls(&m_txtDueDateStart, &m_txtDueDateEnd, false, true);
}

//**************************************************************************
//* Procedure:     SetControlSizes
//*
//* Description:   Sets control sizes on controls that resize only at startup.
//*				   If the user could resize the form, these controls would not resize.
//*
//* Parameters:    None
//*
//* Date Created:  10/25/2005 by Mike Gregg
//**************************************************************************
void CRFV_APCommonStd::SetControlSizes()
{
	__super::SetControlSizes();

	m_pEXLayoutManager->SizeToFit(&m_cboSortBy, SIZ_ComboBox, 15);
	m_pEXLayoutManager->SizeToFit(&m_cboVendorStatus, SIZ_ComboBox, 15);
	m_pEXLayoutManager->SizeToFit(&m_cboFilter, SIZ_ComboBox, 15);
	m_pEXLayoutManager->SizeToFit(&m_txtVendNumStart, SIZ_TextBox, 30);
	m_pEXLayoutManager->SizeToFit(&m_txtVendNumEnd, SIZ_TextBox, 30);
	m_pEXLayoutManager->SizeToFit(&m_txtVendNameStart, SIZ_TextBox, 30);
	m_pEXLayoutManager->SizeToFit(&m_txtVendNameEnd, SIZ_TextBox, 30);
	m_pEXLayoutManager->SizeToFit(&m_txtProdTypeStart, SIZ_TextBox, 20);
	m_pEXLayoutManager->SizeToFit(&m_txtProdTypeEnd, SIZ_TextBox, 20);
	m_pEXLayoutManager->SizeToFit(&m_txtAPCodeStart, SIZ_TextBox, 6);
	m_pEXLayoutManager->SizeToFit(&m_txtAPCodeEnd, SIZ_TextBox, 6);
	m_pEXLayoutManager->SizeToFit(&m_chkGroupByAPCode, SIZ_CheckBox);

	m_pEXLayoutManager->AddLBLControlToGroup(0, &m_lblStart);
	m_pEXLayoutManager->AddLBLControlToGroup(0, &m_lblEnd);

	// 1-24969 BK 7/14/06 added text to UDF label so sizing will increase width enough to hold 15 characters
	m_txtSortByUDFStart.GetWndLabelPtr()->SetWindowTextW(_T("XXXXXXXXXXXXXXX"));

	m_pEXLayoutManager->AddLBLControlToGroup(1, m_txtSortByUDFStart.GetWndLabelPtr());
	m_pEXLayoutManager->AddLBLControlToGroup(1, m_txtFilterUDFStart.GetWndLabelPtr());
	m_pEXLayoutManager->AddLBLControlToGroup(1, m_cboSortBy.GetWndLabelPtr());
	m_pEXLayoutManager->AddLBLControlToGroup(1, m_cboVendorStatus.GetWndLabelPtr());
	m_pEXLayoutManager->AddLBLControlToGroup(1, m_cboFilter.GetWndLabelPtr());
	m_pEXLayoutManager->AddLBLControlToGroup(1, m_txtVendNameStart.GetWndLabelPtr());
	m_pEXLayoutManager->AddLBLControlToGroup(1, m_txtProdTypeStart.GetWndLabelPtr());
	m_pEXLayoutManager->AddLBLControlToGroup(1, m_txtVendNumStart.GetWndLabelPtr());
	m_pEXLayoutManager->AddLBLControlToGroup(1, m_txtAPCodeStart.GetWndLabelPtr());

}

void CRFV_APCommonStd::LayOutUDFs(CWnd* pAboveStart, CWnd* pAboveEnd)
{
	/**Sort By UDF Controls**/
	m_pEXLayoutManager->AnchorLBLControlTL(&m_txtSortByUDFStart, m_txtSortByUDFStart.GetWndLabelPtr(), pAboveStart);
	m_pEXLayoutManager->AnchorLBLControlSameLeft(&m_txtSortByUDFStart, m_txtSortByUDFStart.GetWndLabelPtr(), pAboveStart);
	m_pEXLayoutManager->AnchorControlSameTop(&m_txtSortByUDFEnd, &m_txtSortByUDFStart);
	m_pEXLayoutManager->AnchorControlSameLeft(&m_txtSortByUDFEnd, pAboveEnd);
	/**Sort By Number UDF controls**/
	m_pEXLayoutManager->AnchorLBLControlTL(&m_txtNumSortByUDFStart, m_txtSortByUDFStart.GetWndLabelPtr(), pAboveStart);
	m_pEXLayoutManager->AnchorLBLControlSameLeft(&m_txtNumSortByUDFStart, m_txtSortByUDFStart.GetWndLabelPtr(), pAboveStart);
	m_pEXLayoutManager->AnchorControlSameTop(&m_txtNumSortByUDFEnd, &m_txtNumSortByUDFStart);
	m_pEXLayoutManager->AnchorControlSameLeft(&m_txtNumSortByUDFEnd, pAboveEnd);
	/**Sort By Date UDF controls**/
	m_pEXLayoutManager->AnchorLBLControlTL(&m_dteSortByUDFStart, m_txtSortByUDFStart.GetWndLabelPtr(), pAboveStart);
	m_pEXLayoutManager->AnchorLBLControlSameLeft(&m_dteSortByUDFStart, m_txtSortByUDFStart.GetWndLabelPtr(), pAboveStart);
	m_pEXLayoutManager->AnchorControlSameTop(&m_dteSortByUDFEnd, &m_dteSortByUDFStart);
	m_pEXLayoutManager->AnchorControlSameLeft(&m_dteSortByUDFEnd, pAboveEnd);
	/**Additional Filter UDF controls**/
	m_pEXLayoutManager->AnchorLBLControlTL(&m_txtFilterUDFStart, m_txtFilterUDFStart.GetWndLabelPtr(), pAboveStart);
	m_pEXLayoutManager->AnchorLBLControlSameLeft(&m_txtFilterUDFStart, m_txtFilterUDFStart.GetWndLabelPtr(), pAboveStart);
	m_pEXLayoutManager->AnchorControlSameTop(&m_txtFilterUDFEnd, &m_txtFilterUDFStart);
	m_pEXLayoutManager->AnchorControlSameLeft(&m_txtFilterUDFEnd, pAboveEnd);
	/**Additional Filter Number UDF controls**/
	m_pEXLayoutManager->AnchorLBLControlTL(&m_txtNumFilterUDFStart, m_txtFilterUDFStart.GetWndLabelPtr(), pAboveStart);
	m_pEXLayoutManager->AnchorLBLControlSameLeft(&m_txtNumFilterUDFStart, m_txtFilterUDFStart.GetWndLabelPtr(), pAboveStart);
	m_pEXLayoutManager->AnchorControlSameTop(&m_txtNumFilterUDFEnd, &m_txtNumFilterUDFStart);
	m_pEXLayoutManager->AnchorControlSameLeft(&m_txtNumFilterUDFEnd, pAboveEnd);
	/**Additional Filter Date UDF controls**/
	m_pEXLayoutManager->AnchorLBLControlTL(&m_dteFilterUDFStart, m_txtFilterUDFStart.GetWndLabelPtr(), pAboveStart);
	m_pEXLayoutManager->AnchorLBLControlSameLeft(&m_dteFilterUDFStart, m_txtFilterUDFStart.GetWndLabelPtr(), pAboveStart);
	m_pEXLayoutManager->AnchorControlSameTop(&m_dteFilterUDFEnd, &m_dteFilterUDFStart);
	m_pEXLayoutManager->AnchorControlSameLeft(&m_dteFilterUDFEnd, pAboveEnd);

	if (m_txtFilterUDFStart.IsWindowVisible()) m_pUDFStartPos = &m_txtFilterUDFStart, m_pUDFEndPos = &m_txtFilterUDFEnd;
	else if (m_txtNumFilterUDFStart.IsWindowVisible()) m_pUDFStartPos = &m_txtNumFilterUDFStart, m_pUDFEndPos = &m_txtNumFilterUDFEnd;
	else if (m_dteFilterUDFStart.IsWindowVisible()) m_pUDFStartPos = &m_dteFilterUDFStart, m_pUDFEndPos = &m_dteFilterUDFEnd;
	else                                               m_pUDFStartPos = pAboveStart, m_pUDFEndPos = pAboveEnd;

	__super::SetupLayoutManager();
}

void CRFV_APCommonStd::SetReportValues(CMSGBL::IReportInstantiator * pReportInstantiator)
{
	CMS_VALID_POINTER(pReportInstantiator, CMSGBL::IReportInstantiator);

	pReportInstantiator->CancelReport(!Validate());
	if (pReportInstantiator->CancelReport())
		return;

	pReportInstantiator->MakeAccountSegmentInformation();

	CString szSortByField = _T("");
	CString szReportField = _T("");
	CString szTemp = _T("");
	int intTemp;
	int nSortByUDFType = 0, nAddFilterUDFType = 0;
	int nItemData;
	bool bSortByUDF = false;

	nItemData = (int)m_cboSortBy.GetItemData(m_cboSortBy.GetCurSel());
	switch (nItemData)	//sb - 1-25172
	{
	case 0:
		szReportField = CGBLResources::GetResourceString(IDS_VENDOR_NUMBER);
		szSortByField = _T("strVendor");
		break;
	case 1:
		szReportField = CGBLResources::GetResourceString(IDS_AP_VENDORNAME);
		szSortByField = _T("strVendorName");
		break;
	case 2:
		szReportField = CGBLResources::GetResourceString(IDS_LBL_PRODUCTTYPE);
		szSortByField = _T("strProductType");
		break;
	default:
	{
		// Check for UDF fields
		if ((nItemData > m_nUDFOffset) && (nItemData <= (m_nUDFOffset + NBR_UDFstd)))
		{
			// the sort field is a UDF field
			bSortByUDF = true;
			szSortByField = m_UDFInfo[(size_t)((nItemData - m_nUDFOffset) - 1)].szField;
			szReportField = m_UDFInfo[(size_t)((nItemData - m_nUDFOffset) - 1)].szField; //DRuiz 1-24821 12/18/08
			nSortByUDFType = nItemData - m_nUDFOffset;
		}
	}
	}

	// begbert 0B5-18-2006 : allow derived classes to add extra fields before the UDFs.
	if (nItemData < 3 || ((nItemData > m_nUDFOffset) && (nItemData <= (m_nUDFOffset + NBR_UDFstd))))
	{
		// begbert 07-06-2006 1-24821 : changed the report's @szSortByField to a more useful value.
		pReportInstantiator->MakeReportValue(_T("szSortByField#"), szSortByField);
		pReportInstantiator->MakeReportValue(_T("@szSortByField"), szReportField);
		//1-24926 - RCG - 7/12/2006 - Get the text of the Sort By selection
		m_cboSortBy.GetLBText(m_cboSortBy.GetCurSel(), szTemp);
		pReportInstantiator->MakeReportValue(_T("@szSortByFieldCaption"), szTemp);
		//    pReportInstantiator->MakeReportValue(_T("@szSortByField"), szTemp);
	}

	pReportInstantiator->MakeReportValue(_T("@bSortByUDF#"), bSortByUDF);
	pReportInstantiator->MakeReportValue(_T("nSortByUDFType#"), nSortByUDFType);

	szTemp = _T("");
	int nIndex = m_cboFilter.GetCurSel(); //Get the current selection from the additional filter field
	nItemData = (int)m_cboFilter.GetItemData(nIndex); //Set the data
	if ((nItemData > m_nUDFOffset) && (nItemData <= (m_nUDFOffset + UDFConstants::NBR_UDFstd)) && (m_bAddFilterVisible))
	{
		nAddFilterUDFType = nItemData - m_nUDFOffset;
		szTemp = m_UDFInfo[(size_t)((nItemData - m_nUDFOffset) - 1)].szField; //Get the field name
	}

	// begbert 07-06-2006 1-24821 : sent szAdditionalFilterField to the report as well.
	pReportInstantiator->MakeReportValue(_T("@szAdditionalFilterField#"), szTemp);
	pReportInstantiator->MakeReportValue(_T("nAddFilterUDFType#"), nAddFilterUDFType);

	switch (m_cboVendorStatus.GetItemData(m_cboVendorStatus.GetCurSel()))
	{
	case IDS_AP_ALL_VENDORS: intTemp = 3; break;
	case IDS_AP_ACTIVE_VENDORS: intTemp = 0; break;
	case IDS_AP_INACTIVE_VENDORS: intTemp = 1; break;
	case IDS_AP_TEMPORARY_VENDORS: intTemp = 2; break;
	default: intTemp = 3; break;
	}

	pReportInstantiator->MakeReportValue(_T("bytVendorStatus#"), intTemp);

	m_cboVendorStatus.GetLBText(m_cboVendorStatus.GetCurSel(), szTemp);
	pReportInstantiator->MakeReportValue(_T("@szVendorStatus"), szTemp);

	pReportInstantiator->MakeReportValue(_T("szStartVendorNumber#"), m_txtVendNumStart.GetServerValueString());
	pReportInstantiator->MakeReportValue(_T("@szStartVendorNumber"), m_txtVendNumStart);
	pReportInstantiator->MakeReportValue(_T("szEndVendorNumber#"), m_txtVendNumEnd.GetServerValueString());
	pReportInstantiator->MakeReportValue(_T("@szEndVendorNumber"), m_txtVendNumEnd);

	pReportInstantiator->MakeReportValue(_T("szStartVendorName#"), m_txtVendNameStart.GetServerValueString());
	pReportInstantiator->MakeReportValue(_T("@szStartVendorName"), m_txtVendNameStart);
	pReportInstantiator->MakeReportValue(_T("szEndVendorName#"), m_txtVendNameEnd.GetServerValueString());
	pReportInstantiator->MakeReportValue(_T("@szEndVendorName"), m_txtVendNameEnd);

	pReportInstantiator->MakeReportValue(_T("szStartProductType#"), m_txtProdTypeStart.GetServerValueString());
	pReportInstantiator->MakeReportValue(_T("@szStartProductType"), m_txtProdTypeStart);
	pReportInstantiator->MakeReportValue(_T("szEndProductType#"), m_txtProdTypeEnd.GetServerValueString());
	pReportInstantiator->MakeReportValue(_T("@szEndProductType"), m_txtProdTypeEnd);

	pReportInstantiator->MakeReportValue(_T("szStartAPCode#"), m_txtAPCodeStart.GetServerValueString());
	pReportInstantiator->MakeReportValue(_T("@szStartAPCode"), m_txtAPCodeStart);
	pReportInstantiator->MakeReportValue(_T("szEndAPCode#"), m_txtAPCodeEnd.GetServerValueString());
	pReportInstantiator->MakeReportValue(_T("@szEndAPCode"), m_txtAPCodeEnd);

	pReportInstantiator->MakeReportValue(_T("@bGroupByAPCode#"), m_chkGroupByAPCode.GetCheck() == BST_CHECKED);

	if (m_bSortByVisible)
	{
		nItemData = (int)m_cboSortBy.GetItemData(m_cboSortBy.GetCurSel());
		pReportInstantiator->MakeReportValue(_T("@szUDFLabel1"), m_UDFInfo[(size_t)((nItemData - m_nUDFOffset) - 1)].szCaption);
		pReportInstantiator->MakeReportValue(_T("@szUDFCaption1"), m_UDFInfo[(size_t)((nItemData - m_nUDFOffset) - 1)].szCaption);
	}
	else
		pReportInstantiator->MakeReportValue(_T("@szUDFCaption1"), CString(""));

	if (m_bAddFilterVisible)
	{
		nItemData = (int)m_cboFilter.GetItemData(m_cboFilter.GetCurSel());
		pReportInstantiator->MakeReportValue(_T("@szUDFCaption2"), m_UDFInfo[(size_t)((nItemData - m_nUDFOffset) - 1)].szCaption);
	}
	else
	{
		pReportInstantiator->MakeReportValue(_T("@szUDFCaption2"), CString(""));
	}

	SetupUDFParams(pReportInstantiator);
}

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
bool CRFV_APCommonStd::Validate(void)
{
	// validate range start/end Customer
	if (!CGBLForm::ValidateRange(this, m_txtVendNumStart.GetServerValueString(), m_txtVendNumEnd.GetServerValueString(), IDS_AP_ERR_INVALID_VERDOR_NUMBER_RANGE))
	{
		m_txtVendNumEnd.SetFocus();
		m_txtVendNumEnd.SetSel(0, -1);
		return false;
	}

	// validate range start/end Customer Name
	if (!CGBLForm::ValidateRange(this, m_txtVendNameStart.GetServerValueString(), m_txtVendNameEnd.GetServerValueString(), IDS_AP_ERR_INVALID_VENDOR_NAME_RANGE))
	{
		m_txtVendNameEnd.SetFocus();
		m_txtVendNameEnd.SetSel(0, -1);
		return false;
	}

	// validate range start/end Customer Type
	if (!CGBLForm::ValidateRange(this, m_txtProdTypeStart.GetServerValueString(), m_txtProdTypeEnd.GetServerValueString(), IDS_AP_ERR_INVALID_PRODUCT_TYPE_RANGE))
	{
		m_txtProdTypeEnd.SetFocus();
		m_txtProdTypeEnd.SetSel(0, -1);
		return false;
	}

	// validate range start/end AP Code
	if (!CGBLForm::ValidateRange(this, m_txtAPCodeStart.GetServerValueString(), m_txtAPCodeEnd.GetServerValueString(), IDS_AP_ERR_INVALID_APCODE_RANGE))
	{
		m_txtAPCodeEnd.SetFocus();
		m_txtAPCodeEnd.SetSel(0, -1);
		return false;
	}

	CString szSortByUDF = _T("");
	CString szAddFilterUDF = _T("");
	//get the sort by udf label text
	m_txtSortByUDFStart.GetWndLabelPtr()->GetWindowText(szSortByUDF);
	//get the additional filter udf label text
	m_txtFilterUDFStart.GetWndLabelPtr()->GetWindowText(szAddFilterUDF);

	//Validate start/end sort by udf controls
	if (!ValidateAutofillUDF(szSortByUDF, m_txtSortByUDFStart, m_txtSortByUDFEnd))
		return false;

	//validate start/end additional filter udf controls
	if (!ValidateAutofillUDF(szAddFilterUDF, m_txtFilterUDFStart, m_txtFilterUDFEnd))
		return false;

	//validate start/end sort by number udf controls
	if (!ValidateNumberUDF(szSortByUDF, m_txtNumSortByUDFStart, m_txtNumSortByUDFEnd))
		return false;

	//validate start/end additional filter number udf controls
	if (!ValidateNumberUDF(szAddFilterUDF, m_txtNumFilterUDFStart, m_txtNumFilterUDFEnd))
		return false;

	//validate the start/end sort by date udf controls
	if (m_dteSortByUDFStart.IsWindowVisible())
	{
		m_dteSortByUDFStart.GetDate(m_odtSortByUDFStart);
		m_dteSortByUDFEnd.GetDate(m_odtSortByUDFEnd);
		if (!ValidateDateUDF(szSortByUDF, m_odtSortByUDFStart, m_odtSortByUDFEnd))
		{
			m_dteSortByUDFEnd.SetFocus();
			return false;
		}
	}

	//validate the start/end additional filter date udf controls
	if (m_dteFilterUDFStart.IsWindowVisible())
	{
		m_dteFilterUDFStart.GetDate(m_odtFilterUDFStart);
		m_dteFilterUDFEnd.GetDate(m_odtFilterUDFEnd);
		if (!ValidateDateUDF(szAddFilterUDF, m_odtFilterUDFStart, m_odtFilterUDFEnd))
		{
			m_dteFilterUDFEnd.SetFocus();
			return false;
		}
	}

	//validation passes
	return true;
}

//**************************************************************************
//* Procedure:		LoadSortByCombo
//*
//* Description:		Populates the Sort By combo box
//*
//* Parameters:    none
//*
//* Date Created:  10/25/2005 by Mike Gregg
//**************************************************************************
int CRFV_APCommonStd::LoadSortByCombo()
{
	m_cboSortBy.ResetContent();
	AddIDSToComboN(m_cboSortBy, IDS_VENDOR_NUMBER);
	AddIDSToComboN(m_cboSortBy, IDS_VENDOR_NAME);
	AddIDSToComboN(m_cboSortBy, IDS_PRODUCTTYPE);
	m_cboSortBy.SetCurSel(0);
	return m_cboSortBy.GetCount();
}

int CRFV_APCommonStd::LoadVendorStatusCombo()
{
	m_cboVendorStatus.ResetContent();
	AddIDSToCombo(m_cboVendorStatus, IDS_AP_ALL_VENDORS);
	AddIDSToCombo(m_cboVendorStatus, IDS_AP_ACTIVE_VENDORS);
	AddIDSToCombo(m_cboVendorStatus, IDS_AP_INACTIVE_VENDORS);
	AddIDSToCombo(m_cboVendorStatus, IDS_AP_TEMPORARY_VENDORS);
	m_cboVendorStatus.SetCurSel(0);
	return m_cboVendorStatus.GetCount();
}

// 1-21948 BK 11/30/05 fixed all autofills to work with standardized autofill controls
// when creating autofills don't use:
//	m_txtARCode.SetDataFieldName(_T("strARCode")) ;
//	m_txtARCode.SetFlagAutoFill(&m_bIsAutofill) ;
//	m_txtARCode.SetFlagLookup(&m_bIsLookup) ;
//	m_txtARCode.SetDataPtrXML(&m_clsXML) ;

// use the new methods:
//	m_txtARCode.SetAutoFillKey(_T("strARCode") );
//	m_txtARCode.SetAutoFillControlType();

void CRFV_APCommonStd::InitAutofillControls()
{
	m_txtVendNumStart.SetDataAutoFillName(_T("AP_VENDORMASTER"));	// begbert 07-06-2006 1-24852 : corrected form name for lookup.
	m_txtVendNumStart.SetDataLookupID(LKP_AP_VENDOR);
	m_txtVendNumStart.SetDataFormName(_T("AP_VendorMaster"));
	m_txtVendNumStart.SetDataModuleID(si->ApplicationSettings.ModuleID);
	m_txtVendNumStart.SetDataModuleName(si->ApplicationSettings.ModuleStringID);
	m_txtVendNumStart.SetDataPtrXMLParams(&m_clsXMLParams);
	m_txtVendNumStart.SetDataPtrLookup(&m_pLookup);
	m_txtVendNumStart.SetDataDefaultString(CGBLResources::GetResourceString(IDS_START));
	m_txtVendNumStart.SetDataFetchRecord(false);
	m_txtVendNumStart.SetAddonFlyFromLookup(false);
	m_txtVendNumStart.SetViewFromLookup(false);
	m_txtVendNumStart.SetAutoFillKey(_T("strVendor"));
	m_txtVendNumStart.SetAutoFillControlType();

	m_txtVendNumEnd = m_txtVendNumStart;
	m_txtVendNumEnd.SetDataDefaultString(CGBLResources::GetResourceString(IDS_END));
	m_txtVendNumStart.SetAsStartCtrl();
	m_txtVendNumStart.SetEndCtrlPtr(&m_txtVendNumEnd);
	/**********************************************************************************************************/

	// 1-24901 BK 7/11/06 corrected Vendor Name autofill
	m_txtVendNameStart.SetDataLookupID(LKP_AP_VENDOR_NAME);
	m_txtVendNameStart.SetDataFormName(_T("AP_VENDORMASTER.STRVENDORNAME.VENDORNAME"));
	m_txtVendNameStart.SetDataModuleID(si->ApplicationSettings.ModuleID);
	m_txtVendNameStart.SetDataModuleName(si->ApplicationSettings.ModuleStringID);
	m_txtVendNameStart.SetDataPtrLookup(&m_pLookup);
	m_txtVendNameStart.SetDataDefaultString(CGBLResources::GetResourceString(IDS_START));
	m_txtVendNameStart.SetDataFetchRecord(false);
	m_txtVendNameStart.SetAddonFlyFromLookup(false);
	m_txtVendNameStart.SetViewFromLookup(false);
	m_txtVendNameStart.SetAutoFillKey(_T("strVendorName"));

	m_txtVendNameEnd = m_txtVendNameStart;
	m_txtVendNameEnd.SetDataDefaultString(CGBLResources::GetResourceString(IDS_END));
	m_txtVendNameStart.SetAsStartCtrl();
	m_txtVendNameStart.SetEndCtrlPtr(&m_txtVendNameEnd);
	/**********************************************************************************************************/

	m_txtAPCodeStart.SetDataAutoFillName(_T("AP_CODE"));
	m_txtAPCodeStart.SetDataLookupID(LKP_AP_CODE);
	m_txtAPCodeStart.SetDataFormName(_T("AP_Code"));
	m_txtAPCodeStart.SetDataModuleID(si->ApplicationSettings.ModuleID);
	m_txtAPCodeStart.SetDataModuleName(si->ApplicationSettings.ModuleStringID);
	m_txtAPCodeStart.SetDataPtrXMLParams(&m_clsXMLParams);
	m_txtAPCodeStart.SetDataPtrLookup(&m_pLookup);
	m_txtAPCodeStart.SetDataDefaultString(CGBLResources::GetResourceString(IDS_START));
	m_txtAPCodeStart.SetDataFetchRecord(false);
	m_txtAPCodeStart.SetAddonFlyFromLookup(false);
	m_txtAPCodeStart.SetViewFromLookup(false);
	m_txtAPCodeStart.SetAutoFillKey(_T("strAPCode"));
	m_txtAPCodeStart.SetAutoFillControlType();

	m_txtAPCodeEnd = m_txtAPCodeStart;
	m_txtAPCodeEnd.SetDataDefaultString(CGBLResources::GetResourceString(IDS_END));
	m_txtAPCodeStart.SetAsStartCtrl();
	m_txtAPCodeStart.SetEndCtrlPtr(&m_txtAPCodeEnd);
}

void CRFV_APCommonStd::OnEnSetFocusNumSortByUDFStart()
{
	//Preform the set focus message of the start sort by number control
	OnSetFocusNumUDF(m_txtNumSortByUDFStart);
}

void CRFV_APCommonStd::OnEnSetFocusNumSortByUDFEnd()
{
	//Preform the set focus message of the end sort by number control
	OnSetFocusNumUDF(m_txtNumSortByUDFEnd);
}

void CRFV_APCommonStd::OnEnSetFocusNumFilterUDFStart()
{
	//Preform the set focus message of the start additional filter number control
	OnSetFocusNumUDF(m_txtNumFilterUDFStart);
}

void CRFV_APCommonStd::OnEnSetFocusNumFilterUDFEnd()
{
	//Preform the set focus message of the end additional filter number control
	OnSetFocusNumUDF(m_txtNumFilterUDFEnd);
}

void CRFV_APCommonStd::OnCbnSelchangeCboSortby()
{
	OnCboSelChangeSortByUDF(m_cboSortBy, m_cboFilter);

	if (GetUsePageBreak())
	{
		if (GetDlgItem(IDC_APF_PAGEBREAKON))
		{
			CString szTemp;
			m_cboSortBy.GetLBText(m_cboSortBy.GetCurSel(), szTemp);
			if (m_bSortByVisible)
				szTemp = m_UDFInfo[(size_t)(m_nSortByUDF - 1)].szCaption;

			CString szPageBreakText = CGBLResources::GetResourceString(IDS_AP_LBL_PAGE_BREAK_ON);
			szPageBreakText.Replace(_T("<TEXT1>"), szTemp);
			GetDlgItem(IDC_APF_PAGEBREAKON)->SetWindowTextW(szPageBreakText);
		}
	}
}

void CRFV_APCommonStd::OnCbnSelchangeCboAddfilter()
{
	OnCboSelChangeAddFilterUDF(m_cboFilter);
}

void CRFV_APCommonStd::SetupUDFControls()
{
	//Hide the UDF controls
	if (m_txtSortByUDFStart.GetWndLabelPtr()->IsWindowVisible()) m_txtSortByUDFStart.GetWndLabelPtr()->ShowWindow(SW_HIDE);
	if (m_txtSortByUDFStart.IsWindowVisible()) m_txtSortByUDFStart.ShowWindow(SW_HIDE);
	if (m_txtSortByUDFEnd.IsWindowVisible()) m_txtSortByUDFEnd.ShowWindow(SW_HIDE);
	if (m_txtNumSortByUDFStart.IsWindowVisible()) m_txtNumSortByUDFStart.ShowWindow(SW_HIDE);
	if (m_txtNumSortByUDFEnd.IsWindowVisible()) m_txtNumSortByUDFEnd.ShowWindow(SW_HIDE);
	if (m_dteSortByUDFStart.IsWindowVisible()) m_dteSortByUDFStart.ShowWindow(SW_HIDE);
	if (m_dteSortByUDFEnd.IsWindowVisible()) m_dteSortByUDFEnd.ShowWindow(SW_HIDE);

	if (m_txtFilterUDFStart.GetWndLabelPtr()->IsWindowVisible()) m_txtFilterUDFStart.GetWndLabelPtr()->ShowWindow(SW_HIDE);
	if (m_txtFilterUDFStart.IsWindowVisible()) m_txtFilterUDFStart.ShowWindow(SW_HIDE);
	if (m_txtFilterUDFEnd.IsWindowVisible()) m_txtFilterUDFEnd.ShowWindow(SW_HIDE);
	if (m_txtNumFilterUDFStart.IsWindowVisible()) m_txtNumFilterUDFStart.ShowWindow(SW_HIDE);
	if (m_txtNumFilterUDFEnd.IsWindowVisible()) m_txtNumFilterUDFEnd.ShowWindow(SW_HIDE);
	if (m_dteFilterUDFStart.IsWindowVisible()) 	m_dteFilterUDFStart.ShowWindow(SW_HIDE);
	if (m_dteFilterUDFEnd.IsWindowVisible()) m_dteFilterUDFEnd.ShowWindow(SW_HIDE);

	if (m_bSortByVisible)
	{
		switch (m_nSortByUDF)
		{
		case 1: case 2:
			//Setup(init) sort by udfs 1 & 2
			InitUDF1UDF2(m_txtSortByUDFStart, m_txtSortByUDFEnd);
			break;
		case 3: case 4:
			//Setup(init) sort by udfs 3 & 4
			InitUDF3UDF4(m_txtSortByUDFStart, m_txtSortByUDFEnd);
			break;
		case 5: case 6:
			//Setup(init) sort by udfs 5 & 6
			InitUDF5UDF6(m_dteSortByUDFStart, m_dteSortByUDFEnd);
			break;
		case 7: case 8:
			//Setup(init) sort by udfs 7 & 8
			InitUDF7UDF8(m_txtNumSortByUDFStart, m_txtNumSortByUDFEnd);
			m_pEXLayoutManager->SizeToControlWidth(&m_txtNumSortByUDFStart, m_txtVendNameStart.GetWnd());
			m_pEXLayoutManager->SizeToControlWidth(&m_txtNumSortByUDFEnd, m_txtVendNameEnd.GetWnd());
			break;
		case 9: case 10:
			//Setup(init) sort by udfs 9 & 10
			InitUDF9UDF10(m_txtSortByUDFStart, m_txtSortByUDFEnd);
			break;
		}
		//Set the caption of the UDF label
		m_txtSortByUDFStart.GetWndLabelPtr()->SetWindowText(m_UDFInfo[(size_t)(m_nSortByUDF - 1)].szCaption);
		//Show the UDF label
		m_txtSortByUDFStart.GetWndLabelPtr()->ShowWindow(SW_SHOWNORMAL);
	}

	if (m_bAddFilterVisible)
	{
		switch (m_nAddFilterUDF)
		{
		case 1: case 2:
			//Setup(init) additional filter udfs 1 & 2
			InitUDF1UDF2(m_txtFilterUDFStart, m_txtFilterUDFEnd);
			break;
		case 3: case 4:
			//Setup(init) additional filter udfs 3 & 4
			InitUDF3UDF4(m_txtFilterUDFStart, m_txtFilterUDFEnd);
			break;
		case 5: case 6:
			//Setup(init) additional filter udfs 5 & 6
			InitUDF5UDF6(m_dteFilterUDFStart, m_dteFilterUDFEnd);
			break;
		case 7: case 8:
			//Setup(init) additional filter udfs 7 & 8
			InitUDF7UDF8(m_txtNumFilterUDFStart, m_txtNumFilterUDFEnd);
			m_pEXLayoutManager->SizeToControlWidth(&m_txtNumFilterUDFStart, m_txtVendNameStart.GetWnd());
			m_pEXLayoutManager->SizeToControlWidth(&m_txtNumFilterUDFEnd, m_txtVendNameEnd.GetWnd());
			break;
		case 9: case 10:
			//Setup(init) additional filter udfs 9 & 10
			InitUDF9UDF10(m_txtFilterUDFStart, m_txtFilterUDFEnd);
			break;
		}
		m_txtFilterUDFStart.GetWndLabelPtr()->SetWindowText(m_UDFInfo[(size_t)(m_nAddFilterUDF - 1)].szCaption);
		m_txtFilterUDFStart.GetWndLabelPtr()->ShowWindow(SW_SHOWNORMAL);
	}
}

void CRFV_APCommonStd::PlaceUDFControls()
{
	if (m_txtSortByUDFStart.GetWndLabelPtr()->IsWindowVisible())
		SetTopWnd(&m_txtSortByUDFStart);
	else
		if (!GetTopWnd())
			SetTopWnd(&m_txtAPCodeStart);

	//Declare a right and left bound variables so that we can reset the form bounds
	CWnd* pRightBound = NULL;
	CWnd* pBottomBound = GetTopWnd();

	if (m_txtFilterUDFStart.IsWindowVisible())
	{
		pBottomBound = &m_txtFilterUDFStart;

		/**Additional Filter UDF controls**/
		m_pEXLayoutManager->AnchorLBLControlTL(m_txtFilterUDFStart.GetWnd(), m_txtFilterUDFStart.GetWndLabelPtr(), GetTopWnd());
		m_pEXLayoutManager->AnchorLBLControlSameLeft(&m_txtFilterUDFStart, m_txtFilterUDFStart.GetWndLabelPtr(), GetTopWnd());
		m_pEXLayoutManager->AnchorControlSameTop(m_txtFilterUDFEnd, m_txtFilterUDFStart);
		m_pEXLayoutManager->AnchorControlSameLeft(m_txtFilterUDFEnd, m_txtVendNumEnd);
	}
	else if (m_txtNumFilterUDFStart.IsWindowVisible())
	{
		pBottomBound = &m_txtNumFilterUDFStart;

		/**Additional Filter Number UDF controls**/
		m_pEXLayoutManager->AnchorLBLControlTL(&m_txtNumFilterUDFStart, m_txtFilterUDFStart.GetWndLabelPtr(), GetTopWnd());
		m_pEXLayoutManager->AnchorLBLControlSameLeft(&m_txtNumFilterUDFStart, m_txtFilterUDFStart.GetWndLabelPtr(), GetTopWnd());
		m_pEXLayoutManager->AnchorControlSameTop(&m_txtNumFilterUDFEnd, &m_txtNumFilterUDFStart);
		m_pEXLayoutManager->AnchorControlSameLeft(&m_txtNumFilterUDFEnd, &m_txtVendNumEnd);
	}
	else if (m_dteFilterUDFStart.IsWindowVisible())
	{
		pBottomBound = &m_dteFilterUDFStart;

		/**Additional Filter Date UDF controls**/
		m_pEXLayoutManager->AnchorLBLControlTL(&m_dteFilterUDFStart, m_txtFilterUDFStart.GetWndLabelPtr(), GetTopWnd());
		m_pEXLayoutManager->AnchorLBLControlSameLeft(&m_dteFilterUDFStart, m_txtFilterUDFStart.GetWndLabelPtr(), GetTopWnd());
		m_pEXLayoutManager->AnchorControlSameTop(&m_dteFilterUDFEnd, &m_dteFilterUDFStart);
		m_pEXLayoutManager->AnchorControlSameLeft(&m_dteFilterUDFEnd, &m_txtVendNumEnd);
	}

	m_pEXLayoutManager->RedrawLayout();

	//Call the method that will set the bounds of the form based upon the right and bottom bound
	//controls that are set up in the method.
	GetFormBounds(pRightBound, pBottomBound);
	m_pEXLayoutManager->WrapContainer(NULL, pBottomBound);

	ResetTopWnd();
}
//Defect ID 1-31225, PGP(10-02-2009) Please reader header notes for use/maintenance
void CRFV_APCommonStd::InvokeIDOK()
{
	CButton* pBtnOK = (CButton*)GetDlgItem(IDOK);
	if (pBtnOK)
		CMSGBL::stuInvokeTraits<CButton>::Invoke(pBtnOK);
}
