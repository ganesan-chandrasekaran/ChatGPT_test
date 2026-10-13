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
#include "PPG_APVendor1099.h"
#include "../APForms/psh_apvendormaintenance.h"
#include "../APForms/mfv_apvendormaintenance.h"

 // CPPG_APVendor1099 dialog

IMPLEMENT_DYNAMIC(CPPG_APVendor1099, CCMSPropertyPage)
CPPG_APVendor1099::CPPG_APVendor1099()
	: CCMSPropertyPage(CPPG_APVendor1099::IDD)
	, m_szFederalID(_T(""))
	, m_szDefaultType(_T(""))
	, m_n2TIN(0)
	, m_nOver5000(0)
	, m_nFATCAFiling(0)	//	PBI 30667	12/1/2015	mvt - Added FATCA Filing checkbox
	, m_nrdoAcctType(0)
	, m_bGridAttached(FALSE)
{
}

CPPG_APVendor1099::~CPPG_APVendor1099()
{
}

void CPPG_APVendor1099::DoDataExchange(CDataExchange* pDX)
{
	CCMSPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_APF_TXT_FED_ID, m_txtFederalID);
	DDX_Control(pDX, IDC_APF_CBO_DEFAULT_TYPE, m_cboDefaultType);
	DDX_Control(pDX, IDC_APF_CHK_TIN_NOTICE, m_chk2TIN);
	DDX_Control(pDX, IDC_APF_CHK_OVER_5000, m_chkOver5000);
	//	PBI 30667	12/1/2015	mvt - Added FATCA Filing checkbox
	DDX_Control(pDX, IDC_APF_CHK_FATCA_FILING, m_chkFATCAFiling);
	DDX_Control(pDX, IDC_APF_RAD_BUSINESS, m_rdoBusiness);
	DDX_Control(pDX, IDC_APF_RAD_INDIVIDUAL, m_rdoIndividual);

	DDX_Text(pDX, IDC_APF_TXT_FED_ID, m_szFederalID);
	DDX_Text(pDX, IDC_APF_CBO_DEFAULT_TYPE, m_szDefaultType);
	DDX_Check(pDX, IDC_APF_CHK_TIN_NOTICE, m_n2TIN);
	DDX_Check(pDX, IDC_APF_CHK_OVER_5000, m_nOver5000);

	//	PBI 30667	12/1/2015	mvt - Added FATCA Filing checkbox
	DDX_Check(pDX, IDC_APF_CHK_FATCA_FILING, m_nFATCAFiling);
}


BEGIN_MESSAGE_MAP(CPPG_APVendor1099, CCMSPropertyPage)
	ON_BN_CLICKED(IDC_APF_RAD_INDIVIDUAL, &CPPG_APVendor1099::FormatFedID)
	ON_BN_CLICKED(IDC_APF_RAD_BUSINESS, &CPPG_APVendor1099::FormatFedID)
	ON_EN_KILLFOCUS(IDC_APF_TXT_FED_ID, &CPPG_APVendor1099::FormatFedID)
	ON_EN_SETFOCUS(IDC_APF_TXT_FED_ID, &CPPG_APVendor1099::OnEnSetfocusApfTxtFedId)
END_MESSAGE_MAP()


// CPPG_APVendor1099 message handlers
BOOL CPPG_APVendor1099::OnInitDialog()
{
	CCMSPropertyPage::OnInitDialog();

	LoadResources();

	SetControlProperties();

	InitGrouperControls();

	InitGrid();

	m_cboDefaultType.ModifyStyle(CBS_SORT, NULL);
	for (int nCount = 0; nCount < si->ModuleSettings.APSettings.arsz1099Descriptions.GetCount(); nCount++) {
		m_cboDefaultType.AddString(si->ModuleSettings.APSettings.arsz1099Descriptions.GetAt(nCount));
	}

	UpdateData();

	m_cboDefaultType.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
//------------------------------------------------------------------------------
void CPPG_APVendor1099::InitGrid(void)
{
	if (!m_bGridAttached)
	{
		m_bGridAttached = m_cls1099Grid.AttachGrid(this, IDC_APF_GRID_HOLDER);
		VERIFY(m_bGridAttached);
		m_cls1099Grid.AdjustComponentSizes();
		m_cls1099Grid.SetUserSizingMode(FALSE);
		m_cls1099Grid.RedrawAll();
		m_cls1099Grid.m_bEditMode = TRUE;
		m_cls1099Grid.ResetGrid(MODE_ADD);
		m_cls1099Grid.m_pbParentDirty = &(((CPSH_APVendorMaintenance*)GetParent())->m_pParentView->m_dlgCMSControlButtons.m_bIsDirty);

	}
	m_cls1099Grid.ResetGrid(MODE_ADD);
}

/*********************************************************
Function    LoadResources
Type        Homemade Member Function
Purpose     Used to load the resource strings/images from the
			resource file
Parameters  void
Returns     void
Author      Jason K Cicero
Date        6 Nov 2003
***********************************************************/
void CPPG_APVendor1099::LoadResources()
{
	SetResourceText(IDC_APF_LBL_FED_ID, IDS_AP_FEDERAL_ID);
	SetResourceText(IDC_APF_LBL_DEFAULT_TYPE, IDS_AP_DEFAULT_TYPE);
	SetResourceText(IDC_APF_CHK_TIN_NOTICE, IDS_AP_SECOND_TIN_NOTICE);
	SetResourceText(IDC_APF_CHK_OVER_5000, IDS_AP_OVER_5000);
	SetResourceText(IDC_APF_RAD_BUSINESS, IDS_BUSINESS);
	SetResourceText(IDC_APF_RAD_INDIVIDUAL, IDS_AP_INDIVIDUAL);
	//	PBI 30667	12/1/2015	mvt - Added FATCA Filing checkbox
	SetResourceText(IDC_APF_CHK_FATCA_FILING, IDS_AP_FATCA_FILING_REQUIREMENT);
}
void CPPG_APVendor1099::SetControlProperties()
{
	m_txtFederalID.SetLimitText(11);
}
//Initialize Grouper class for Radio buttons,
//enables XML data transfer and enable/disable
void CPPG_APVendor1099::InitGrouperControls()
{
	m_rdoBusiness.SetRadioValue(_T("false"));
	m_rdoIndividual.SetRadioValue(_T("true"));

	//Form arrays for grouper and controls within grouper.
	//as initialization is the same for both sets

	m_clsGrouperType.SetGroupType(CCMSGrouper::CSTR_GROUP);
	m_clsGrouperType.SetParentWindow(this);
	//Add all radio button controls to grouper
	m_clsGrouperType.AddControl(IDC_APF_RAD_BUSINESS);
	m_clsGrouperType.AddControl(IDC_APF_RAD_INDIVIDUAL);
	//Initialize the default selection
	m_clsGrouperType.InitDefault(0, _T("false"));
	//Set the default selection
	m_clsGrouperType.SetDefault();
}
void CPPG_APVendor1099::FillControlMap(CXML* pXML)
{
	pXML->MySetElement(_T("0"), _T("strFederalIDNumber"), &m_txtFederalID, NULL, STRINGTYPE);
	pXML->MySetElement(_T("0"), _T("bol2ndTIN"), &m_chk2TIN, NULL, CHECKTYPE);
	pXML->MySetElement(_T("0"), _T("bol5000DirSales"), &m_chkOver5000, NULL, CHECKTYPE);
	pXML->MySetElement(_T("0"), _T("byt1099Type"), &m_cboDefaultType, NULL, COMBOBOX_INDEX);

	pXML->MySetElement(_T("0"), _T("bol1099Flag"), &m_clsGrouperType, NULL, GROUPTYPE);

	pXML->MySetElement(_T("0"), _T("MASTER"), &m_cls1099Grid, READWRITE, GRIDTYPE);

	//	PBI 30667	12/1/2015	mvt - Added FATCA Filing checkbox
	pXML->MySetElement(_T("0"), _T("bolFATCAFiling"), &m_chkFATCAFiling, NULL, CHECKTYPE);
}
void CPPG_APVendor1099::SetArrayValues(int nType, CStringArray& arszCommandValues)
{
	UpdateData();
	if ((nType == RECORD_INSERT) || (nType == RECORD_UPDATE))
	{
		arszCommandValues.Add(_T("strFederalIDNumber"));
		arszCommandValues.Add(m_szFederalID);

		arszCommandValues.Add(_T("byt1099Type"));

		int nCurSel = m_cboDefaultType.GetCurSel();
		m_szDefaultType.Format(_T("%d"), nCurSel == -1 ? 0 : nCurSel);
		arszCommandValues.Add(m_szDefaultType);

		arszCommandValues.Add(_T("bol2ndTIN"));
		arszCommandValues.Add((m_n2TIN == 1 ? _T("1") : _T("0")));
		arszCommandValues.Add(_T("bol5000DirSales"));
		arszCommandValues.Add((m_nOver5000 == 1 ? _T("1") : _T("0")));
		arszCommandValues.Add(_T("bol1099Flag"));
		arszCommandValues.Add((m_clsGrouperType.GetStringValue() == _T("true") ? _T("1") : _T("0")));
		arszCommandValues.Add(_T("bol1099ApplicableFlag"));
		arszCommandValues.Add(m_clsGrouperType.GetStringValue() == _T("true") ? _T("1") : _T("0"));

		//	PBI 30667	12/1/2015	mvt - Added FATCA Filing checkbox
		arszCommandValues.Add(_T("bolFATCAFiling"));
		arszCommandValues.Add((m_nFATCAFiling == 1 ? _T("1") : _T("0")));
	}
}
CString CPPG_APVendor1099::GetCurrentYearSum()
{
	return m_cls1099Grid.GetCurrentYearSum();
}

void CPPG_APVendor1099::FormatFedID()
{
	m_txtFederalID.SetLimitText(11);
	//Defect ID 1-13854, PGP(09/28/2004)
	//Changed GetCheck call so the correct radio value is returned
	// begbert 11-26-2012 Bug 1390 : Incorporated the individual radio button click handlers into this
	//   function, which is called instead when either radio button is clicked.  This simplifies code
	//   and neutralizes an odd click event that erroneously occurs when the tab loses focus.
	CString szTmp;
	m_txtFederalID.GetWindowText(m_szFederalID);
	m_szFederalID.Remove('-');
	if (m_rdoBusiness.GetCheck() == BST_CHECKED)
	{
		//Format entry for Business (EIN)
		if (m_szFederalID.GetLength() == 9)
			szTmp = m_szFederalID.Left(2) + "-" + m_szFederalID.Right(7);
	}
	else {
		//Format entry for individual (SSN)
		if (m_szFederalID.GetLength() == 9)
			szTmp = m_szFederalID.Left(3) + "-" + m_szFederalID.Mid(3, 2) + "-" + m_szFederalID.Right(4);
	}
	m_txtFederalID.SetWindowText(szTmp);
}
void CPPG_APVendor1099::ChangeMode(enmFormModes nMode)
{
	if (nMode == MODE_ADD) {
		m_cls1099Grid.SetTableDirty();
		//m_cls1099Grid.ResetGrid(0);
	}
	else if (nMode == MODE_VIEW) {
		m_cls1099Grid.m_bEditMode = false;
		CString szCellText;
		m_cls1099Grid.QuickGetText(2, 3, &szCellText);
		if ("" == szCellText) {
			m_cls1099Grid.ResetGrid(0);
		}
	}
	else if (nMode == MODE_EDIT) {
		m_cls1099Grid.m_bEditMode = true;
	}
}
bool CPPG_APVendor1099::Validate()
{
	UpdateData();
	CString szTmp = m_szFederalID;
	szTmp.Replace(_T("-"), _T(""));
	if (szTmp.GetLength() != 9 && szTmp.GetLength() > 0) {
		((CPropertySheet*)GetParent())->SetActivePage(this);
		CGBLForm::CMSMessageBox(this, IDS_AP_ERR_INVALID_FEDID, IDS_AP_ERR_INVALID_FEDID_CAP, MB_OK | MB_ICONSTOP, m_szFederalID);
		m_txtFederalID.SetFocus();
		return false;
	}

	return true;
}


void CPPG_APVendor1099::OnEnSetfocusApfTxtFedId()
{
	UpdateData();
	m_txtFederalID.SetLimitText(9);
	m_szFederalID.Replace(_T("-"), _T(""));
	m_txtFederalID.SetWindowText(m_szFederalID);
	m_txtFederalID.PostMessage(EM_SETSEL, 0, -1);
}
