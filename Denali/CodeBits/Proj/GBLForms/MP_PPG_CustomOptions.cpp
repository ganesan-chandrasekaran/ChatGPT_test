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
 *  WAPNING:  Unauthorized reproduction of this program as well as
 *  unauthorized preparation of derivative works based upon the program or
 *  distribution of copies by sale, rental, lease or lending are violations
 *  of federal copyright laws and state trade secret laws, punishable by
 *  civil and criminal penalties.
 *
 ******************************************************************************
 ******************************************************************************
 *	Date:		9/10/2012 - Custom Reports/Custom Menus
 *	Author:		InTimeTec: Kapil Paliwal/Hitesh Sharma
 ******************************************************************************/
#include "stdafx.h"
#include "MP_CFV_ModPrefs.h"
#include "MP_PPG_CustomOptions.h"
#include "MP_GRD_CustomOptions.h"

 // CMP_PPG_CustomOptions dialog
IMPLEMENT_DYNAMIC(CMP_PPG_CustomOptions, CCMSPropertyPageEx)
//----------------------------------------------------------------------------
CMP_PPG_CustomOptions::CMP_PPG_CustomOptions(CMP_CFV_ModPrefs& clsForm)
	: CCMSPropertyPageEx(CMP_PPG_CustomOptions::IDD)
	, m_clsForm(clsForm)
	, m_grdGridMenu()
	, m_grdGrid()
	, m_bInit(false)
{
	// Changing the way grid is attached to the form. This is in order to fix scroll bar issues
	// Also changed these grids to pointer type. So changed all . with ->
	m_grdGrid.reset(new CGRD_CustomOptionsEx(*this));
	m_grdGridMenu.reset(new CGRD_CustomMenuEx(*this));

	// InTimeTec: 12.20.2012.HS - Custom Menus
	m_bInitCustomReports = true;
	m_grdGrid->SetTopGridFlag(true);
	m_grdGridMenu->SetTopGridFlag(false);
	// InTimeTec: End
}

//----------------------------------------------------------------------------
CMP_PPG_CustomOptions::~CMP_PPG_CustomOptions()
{
}

//----------------------------------------------------------------------------
void CMP_PPG_CustomOptions::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
}
//----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CMP_PPG_CustomOptions, CCMSPropertyPageEx)
END_MESSAGE_MAP()
//----------------------------------------------------------------------------

// Initialize form controls
void CMP_PPG_CustomOptions::InitControls()
{
	__super::InitControls();

	m_grdGrid->AttachGrid(this, IDC_GBL_GRIDFRAME_BOTTOM);
	m_grdGrid->ModifyStyle(0, WS_VISIBLE);

	m_grdGrid->SetMultiSelectMode(UG_MULTISELECT_ROW);
	m_grdGrid->DragDropTarget(TRUE);

	m_grdGridMenu->AttachGrid(this, IDC_GBL_GRIDFRAME_TOP);
	m_grdGridMenu->ModifyStyle(0, WS_VISIBLE);
}

// load string resources
void CMP_PPG_CustomOptions::LoadResources()
{
	__super::LoadResources();

	// set tab title
	SetPageCaption(IDS_CUSTOM_OPTIONS);
}
//----------------------------------------------------------------------------

void CMP_PPG_CustomOptions::SetupLayoutManager()
{
	__super::SetupLayoutManager();

	m_pEXLayoutManager->AnchorControlTL(IDC_GBL_GRP_CUSTOM_TOP);
	m_pEXLayoutManager->AnchorControlRight(IDC_GBL_GRP_CUSTOM_TOP);
	m_pEXLayoutManager->AnchorControlTL(IDC_GBL_GRP_CUSTOM_BOTTOM, IDC_GBL_GRP_CUSTOM_TOP);
	m_pEXLayoutManager->AnchorControlRight(IDC_GBL_GRP_CUSTOM_BOTTOM);
	m_pEXLayoutManager->AnchorControlInGroupBox(IDC_GBL_GRP_CUSTOM_TOP, IDC_GBL_GRIDFRAME_TOP, NULL, NULL);
	m_pEXLayoutManager->AnchorControlRight(IDC_GBL_GRIDFRAME_TOP, 0, 5);
	m_pEXLayoutManager->AnchorControlBottom(IDC_GBL_GRIDFRAME_TOP, IDC_GBL_GRP_CUSTOM_BOTTOM, 9);
	m_pEXLayoutManager->AnchorControlInGroupBox(IDC_GBL_GRP_CUSTOM_BOTTOM, IDC_GBL_GRIDFRAME_BOTTOM, NULL, NULL);
	m_pEXLayoutManager->AnchorControlRight(IDC_GBL_GRIDFRAME_BOTTOM, 0, 5);
	m_pEXLayoutManager->AnchorControlBottom(IDC_GBL_GRIDFRAME_BOTTOM, NULL, 7);
}
//----------------------------------------------------------------------------

void CMP_PPG_CustomOptions::OnEvent(CCMSSubject * pSubject, UINT uiEvent)
{
	switch (uiEvent)
	{
		// get message from grid that rows deletion was requested
	case GBLEventID::eRowDeleted:

		break;
	};
}
//----------------------------------------------------------------------------

void CMP_PPG_CustomOptions::GetFormBounds(CWnd*& pRightBound, CWnd*& pBottomBound)
{
	pRightBound = GetDlgItem(IDC_GBL_GRP_CUSTOM_BOTTOM);
	pBottomBound = GetDlgItem(IDC_GBL_GRP_CUSTOM_BOTTOM);

	__super::GetFormBounds(pRightBound, pBottomBound);
}
//----------------------------------------------------------------------------

void CMP_PPG_CustomOptions::OnAddFieldMaps(CCMSMaintDoc& clsDoc)
{
	//Sets the default parent base.  This prevents us from having to always pass in
	//"this" in the below AddFieldMap calls.  It is only used by the AddFieldMap methods.
	clsDoc.SetParentBase(this);
	// InTimeTec: 10/31/2013 : HS : Bug 9608: DEV_CT_Custom Options tab, Module Preferences form, Location/File Name field focus not moving using mouse click on next detail line
	// added the custom menus before custom reports in the Maintenance form map
	// IsValidData should validate the upper grid first
	// InTimeTec: 12.20.2012.HS - Custom Menus
	clsDoc.AddFieldMap(*m_grdGridMenu, _T("Table9"), DMSTYLE_NOFORMMODE);
	m_grdGridMenu->SetXMLReturnTable(_T("Table9"));
	// InTimeTec: End
	clsDoc.AddFieldMap(*m_grdGrid, _T("Table8"), DMSTYLE_NOFORMMODE);
	m_grdGrid->SetXMLReturnTable(_T("Table8"));
}
//----------------------------------------------------------------------------

// receive message to grid for the server result
void CMP_PPG_CustomOptions::Notify(CCMSMessage& objMessage)
{
	if (objMessage.GetClassID() == CCMSMaintMessageID)
	{
		CCMSMaintMsg* pMaintMsg = DYNAMIC_DOWNCAST(CCMSMaintMsg, &objMessage);
		if (pMaintMsg->GetMsgType() == MaintMsg_OnSvrRecordSaved)
		{
			// When server fails to publish files with less than 100 MB of size, move the focus to the last row.
			CCMSMaintMsg_SvrRecordSaved* pMsg = DYNAMIC_DOWNCAST(CCMSMaintMsg_SvrRecordSaved, pMaintMsg);
			if (pMsg->GetRC() == ERR_GB_INSUFFICIENT_MEMORY)
			{
				m_grdGridMenu->OnServerFileSizeError();
			}
		}
	}
	__super::Notify(objMessage);
}
//----------------------------------------------------------------------------

// get modified flag from grid
void CMP_PPG_CustomOptions::SetModifiedFlagFromGrid(BOOL bModified)
{
	m_clsForm.SetModifiedFlag(bModified);
}
//----------------------------------------------------------------------------

//Overrides MUST create a CCMSLTBase-derived line type object on the heap and return it.
//DO NOT RETAIN OR DESTROY THE NEWLY-CREATED POINTER!!!
//CCMSGridDataSource will take care of managing and destroying it for you.
CCMSLTBase* CMP_PPG_CustomOptions::CreateNewRow(long lRow, CString& szLineType, CCMSGridEntry& clsGrid)
{
	// InTimeTec: 12.20.2012.HS - Custom Menus

	if (clsGrid.GetTopGridFlag())
	{
		return new CLTCustomOptions(*this, szLineType, clsGrid);
	}
	else
	{
		return new CLTCustomMenu(*this, szLineType, clsGrid);
	}
	// InTimeTec: End
}
//----------------------------------------------------------------------------

// setup grid
void CMP_PPG_CustomOptions::SetupGridProps(CCMSGridEntry& clsGrid, CString& szLineTypeField/* = ""*/)
{
	clsGrid.SetAllowVScrollbar(true);
	clsGrid.SetFixedGrid(false);
}
//----------------------------------------------------------------------------

// set additional properties after grid columns are setup
void CMP_PPG_CustomOptions::PostSetupGridProps(CCMSGridEntry& clsGrid)
{
	clsGrid.SetAllowVScrollbar(true);
}
//----------------------------------------------------------------------------

//Add column ID's to the grid.
void CMP_PPG_CustomOptions::AddGridColumns(CCMSGridEntry& clsGrid)
{
	// InTimeTec: 12.20.2012.HS - Custom Menus
	if (m_bInitCustomReports)
	{
		// Custom Reorts
		clsGrid.AddColumn(CLTCustomOptions::COL_REPORTTYPE);
		clsGrid.AddColumn(CLTCustomOptions::COL_REPORTLOC);
		clsGrid.AddColumn(CLTCustomOptions::COL_REPORTLOC_DRILLDOWN);
		clsGrid.AddColumn(CLTCustomOptions::COL_REPORTNAME);
		clsGrid.AddColumn(CLTCustomOptions::COL_PUBLISHTOSRV);
		m_bInitCustomReports = false;
	}
	else
	{
		// Custom Menus
		clsGrid.AddColumn(CLTCustomMenu::COL_MODULE);
		clsGrid.AddColumn(CLTCustomMenu::COL_CATEGORY);
		clsGrid.AddColumn(CLTCustomMenu::COL_MENUITEMNAME);
		clsGrid.AddColumn(CLTCustomMenu::COL_LOCATION);
		clsGrid.AddColumn(CLTCustomMenu::COL_MENULOC_DRILLDOWN);
		clsGrid.AddColumn(CLTCustomMenu::COL_ISTHISURL);
		clsGrid.AddColumn(CLTCustomMenu::COL_PUBLISHTOSRV);
	}
	// InTimeTec: End
}
//----------------------------------------------------------------------------

// setup the passed-in column
void CMP_PPG_CustomOptions::SetupColumn(int nColumnID, CCMSGridControlInfo& clsControlInfo, CCMSGridEntry& clsGrid)
{
	int nWidth = 9;
	CCMSGrid::COLPROPS& clsColProps = clsControlInfo.m_clsColProps;

	switch (nColumnID)
	{
	case CLTCustomOptions::COL_REPORTTYPE:
	{
		// JEPK 2/24/2015 PBI 27673 added Custom Report Options
		clsGrid.SetColumnText(clsColProps, IDS_REPORTTYPE);
		CCMSGridComboInfo* pComboInfo = clsControlInfo.m_pComboInfo = new CCMSGridComboInfo(nColumnID);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_AP_STRING) + '-' + CGBLResources::GetResourceString(IDS_AGING), CLTCustomOptions::RPT_APAGING);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_AP_STRING) + '-' + CGBLResources::GetResourceString(IDS_ENTER_BILLS_PRINT_EDIT_REPORT), CLTCustomOptions::RPT_APENTERBILLS);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_AP_STRING) + '-' + CGBLResources::GetResourceString(IDS_PAY_BILLS_PRINT_EDIT_REPORT), CLTCustomOptions::RPT_APPAYBILLS);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_AP_STRING) + '-' + CGBLResources::GetResourceString(IDS_AP_PRINTCHECKS1), CLTCustomOptions::RPT_APPRINTCHECKS);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_AP_STRING) + '-' + CGBLResources::GetResourceString(IDS_AP_PRINTIMMEDIATECHECKS1), CLTCustomOptions::RPT_APIMMEDIATECHECKS);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_AR_MODULE) + '-' + CGBLResources::GetResourceString(IDS_CTRL_CUST_STATEMENTS), CLTCustomOptions::RPT_ARCUSTSTATEMENTS);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_AR_MODULE) + '-' + CGBLResources::GetResourceString(IDS_DATED_INVOICE_PAYMENT_REPORT), CLTCustomOptions::RPT_ARDATEDINVOICETOPAYMENT);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_AR_MODULE) + '-' + CGBLResources::GetResourceString(IDS_CTRL_AGED_ANALYSIS), CLTCustomOptions::RPT_ARAGEDANALYSIS);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_BR_MODULE) + '-' + CGBLResources::GetResourceString(IDS_AP_PRINTCHECKS1), CLTCustomOptions::RPT_BRPRINTCHECKS);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_GL_MODULE) + '-' + CGBLResources::GetResourceString(IDS_AUDITTRAILREPORT), CLTCustomOptions::RPT_GLAUDITTRIAL);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_GL_MODULE) + '-' + CGBLResources::GetResourceString(IDS_GLBALANCESHEETREPORT), CLTCustomOptions::RPT_GLBALANCESHEET);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_GL_MODULE) + '-' + CGBLResources::GetResourceString(IDS_GLFBUDGETPERFORMANCEREPORT), CLTCustomOptions::RPT_GLBUDGETPERFORMANCE);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_GL_MODULE) + '-' + CGBLResources::GetResourceString(IDS_BUDGET_TRACKING_REPORT), CLTCustomOptions::RPT_GLBUDGETTRACKING);
		if (CMS::GetSI()->CompanySettings.IsFund)
			pComboInfo->AddString(CGBLResources::GetResourceString(IDS_GL_MODULE) + '-' + CGBLResources::GetResourceString(IDS_REVENUE_AND_EXPENSE_REPORT), CLTCustomOptions::RPT_GLREVENUEEXPENSEINCOMESTATEMENT);
		else
			pComboInfo->AddString(CGBLResources::GetResourceString(IDS_GL_MODULE) + '-' + CGBLResources::GetResourceString(IDS_INCOME_STATEMENT_REPORT), CLTCustomOptions::RPT_GLREVENUEEXPENSEINCOMESTATEMENT);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_IN_MODULE) + '-' + CGBLResources::GetResourceString(IDS_PO_CAP_HISTORY_REPORT), CLTCustomOptions::RPT_INHISTORY);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_IN_MODULE) + '-' + CGBLResources::GetResourceString(IDS_ONHANDREPORT), CLTCustomOptions::RPT_INONHAND);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_IN_MODULE) + '-' + CGBLResources::GetResourceString(IDS_IN_ON_ORDER_REPORT_TITLE), CLTCustomOptions::RPT_INONORDER);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_PO_MODULE) + '-' + CGBLResources::GetResourceString(IDS_PRINT_PO_RPT), CLTCustomOptions::RPT_POPRINTPURCHASE);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_PO_MODULE) + '-' + CGBLResources::GetResourceString(IDS_POMENU_REPRINT_POS_STATUSBAR), CLTCustomOptions::RPT_POREPRINTPURCHASE);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_OE_MODULE) + '-' + CGBLResources::GetResourceString(IDS_MSG_PRINT_EDIT_REPORT), CLTCustomOptions::RPT_OEEDITREPORT);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_OE_MODULE) + '-' + CGBLResources::GetResourceString(IDS_CT_PRINT_INVOICES), CLTCustomOptions::RPT_OEPRINTINVOICES);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_OE_MODULE) + '-' + CGBLResources::GetResourceString(IDS_CT_REPRINT_INVOICES_FROM_HISTORY), CLTCustomOptions::RPT_OEPRINTINVOICESFRMHISTORY);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_OE_MODULE) + '-' + CGBLResources::GetResourceString(IDS_PRINT_PACKING_SLIPS), CLTCustomOptions::RPT_OEPACKINGSLIPS);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_OE_MODULE) + '-' + CGBLResources::GetResourceString(IDS_SA_PICKING_TITLE), CLTCustomOptions::RPT_OEPICKLIST);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_PR_MODULE) + '-' + CGBLResources::GetResourceString(IDS_AP_PRINTCHECKS1), CLTCustomOptions::RPT_PRCHECK);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_PR_MODULE) + '-' + CGBLResources::GetResourceString(IDS_MSG_PRINT_EDIT_REPORT), CLTCustomOptions::RPT_PREDITREPORT);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_PR_MODULE) + '-' + CGBLResources::GetResourceString(IDS_EMPLOYEE_HISTORY_REPORT), CLTCustomOptions::RPT_PREMPLOYEEHISTORY);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_PR_MODULE) + '-' + CGBLResources::GetResourceString(IDS_EMPLOYEE_LEAVE_REPORT), CLTCustomOptions::RPT_PREMPLOYEELEAVE);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_PS_MODULE) + '-' + CGBLResources::GetResourceString(IDS_MSG_PRINT_EDIT_REPORT), CLTCustomOptions::RPT_PSEDITREPORT);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_SA_MODULE) + '-' + CGBLResources::GetResourceString(IDS_SA_PROFIT_MARGIN_REPORT), CLTCustomOptions::RPT_SAPROFITMARGIN);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_SA_MODULE) + '-' + CGBLResources::GetResourceString(IDS_SA_SALES_BY_SALESPERSON_CAP), CLTCustomOptions::RPT_SASALESBYSALESPERSON);
		nWidth = 19;
		break;
	}

	case CLTCustomOptions::COL_REPORTLOC_DRILLDOWN:
		clsGrid.SetColumnText(clsColProps, IDS_REPORT_LOCATION);
		clsGrid.SetCmdButtonProps(clsColProps);
		nWidth = 2;
		break;
	case CLTCustomOptions::COL_REPORTLOC:
		clsGrid.SetColumnText(clsColProps, IDS_REPORT_LOCATION);
		clsColProps.nMaxDataWidth = 256;
		nWidth = 17;
		break;
	case CLTCustomOptions::COL_REPORTNAME:
	{
		clsGrid.SetColumnText(clsColProps, IDS_REPORT_NAME);
		clsColProps.nMaxDataWidth = 40;
		nWidth = 11;
		break;
	}
	case CLTCustomOptions::COL_PUBLISHTOSRV:
	{
		clsGrid.SetColumnText(clsColProps, IDS_PUBLISH_SVR);
		clsColProps.nDecimalDigitCount = 1;
		clsColProps.nDataType = UGCELLDATA_BOOL;
		clsColProps.nAlignment = UG_ALIGNCENTER;
		clsColProps.nCellType = UGCT_CHECKBOX;
		clsColProps.nTranslateType = GRD_XLAT_ENCUMBCHECK;
		nWidth = 10;
		break;
	}
	}
	if (nColumnID <= 4)
	{
		clsColProps.nDefaultColumnWidth = m_grdGrid->GetColWidthToShow(nWidth);
	}

	// InTimeTec: 12.20.2012.HS - Custom Menus
	switch (nColumnID)
	{
	case CLTCustomMenu::COL_MODULE:
	{
		clsGrid.SetColumnText(clsColProps, IDS_MODULE);
		CCMSGridComboInfo* pComboInfo = clsControlInfo.m_pComboInfo = new CCMSGridComboInfo(nColumnID);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_MODULE_AP), CLTCustomMenu::MOD_AP);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_MODULE_AR), CLTCustomMenu::MOD_AR);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_MODULE_BR), CLTCustomMenu::MOD_BR);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_MODULE_CT), CLTCustomMenu::MOD_CT);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_CMS_GENERALLEDGER), CLTCustomMenu::MOD_GL);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_MODULE_IN), CLTCustomMenu::MOD_IN);
		//RS - 06/22/2026 - PBI 65707/65708 - Add custom menus to JC and PR
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_MODULE_JC), CLTCustomMenu::MOD_JC);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_MODULE_PR), CLTCustomMenu::MOD_PR);
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_MODULE_PO), CLTCustomMenu::MOD_PO);		
		pComboInfo->AddString(CGBLResources::GetResourceString(IDS_SALES), CLTCustomMenu::MOD_SA);
		nWidth = 12;
		break;
	}

	case CLTCustomMenu::COL_CATEGORY:
		clsGrid.SetColumnText(clsColProps, IDS_CATEGORY);
		clsColProps.nMaxDataWidth = 15;
		nWidth = 6;
		break;
	case CLTCustomMenu::COL_MENUITEMNAME:
		clsGrid.SetColumnText(clsColProps, IDS_MENU_ITEM);
		clsColProps.nMaxDataWidth = 40;
		nWidth = 10;
		break;
	case CLTCustomMenu::COL_LOCATION:
	{
		clsGrid.SetColumnText(clsColProps, IDS_LOCATION_FILE);
		clsColProps.nMaxDataWidth = 256;
		nWidth = 11;
		break;
	}
	case CLTCustomMenu::COL_MENULOC_DRILLDOWN:
	{
		clsGrid.SetColumnText(clsColProps, IDS_LOCATION_FILE);
		clsGrid.SetCmdButtonProps(clsColProps);
		nWidth = 2;
		break;
	}
	case CLTCustomMenu::COL_ISTHISURL:
	{
		clsGrid.SetColumnText(clsColProps, IDS_URL);
		clsColProps.nDecimalDigitCount = 1;
		clsColProps.nDataType = UGCELLDATA_BOOL;
		clsColProps.nAlignment = UG_ALIGNCENTER;
		clsColProps.nCellType = UGCT_CHECKBOX;
		clsColProps.nTranslateType = GRD_XLAT_ENCUMBCHECK;
		nWidth = 8;
		break;
	}
	case CLTCustomMenu::COL_PUBLISHTOSRV:
	{
		clsGrid.SetColumnText(clsColProps, IDS_PUBLISH_SVR);
		clsColProps.nDecimalDigitCount = 1;
		clsColProps.nDataType = UGCELLDATA_BOOL;
		clsColProps.nAlignment = UG_ALIGNCENTER;
		clsColProps.nCellType = UGCT_CHECKBOX;
		clsColProps.nTranslateType = GRD_XLAT_ENCUMBCHECK;
		nWidth = 10;
		break;
	}
	}

	if (nColumnID >= 5)
	{
		clsColProps.nDefaultColumnWidth = m_grdGridMenu->GetColWidthToShow(nWidth);
	}
	// InTimeTec: End
}
//----------------------------------------------------------------------------

BOOL CMP_PPG_CustomOptions::OnSetActive(void)
{
	if (!m_bInit) LoadCustomReportsGrid();
	// InTimeTec: 12.20.2012.HS - Custom Menus
	if (!m_bInit) LoadCustomMenusGrid();
	m_bInit = true;

	SetCustomReportsGrid();
	SetCustomMenusGrid();

	m_grdGridMenu->EnableWindow(TRUE);
	m_grdGrid->EnableWindow(TRUE);
	// InTimeTec: End
	return __super::OnSetActive();
}

// Call the server to get the records to load custom reports grid.
void CMP_PPG_CustomOptions::LoadCustomReportsGrid(void)
{
	using namespace CMSStrings;

	CCMSDataAdapter	objDA;

	objDA.SetGetCommand(XMLTags::GET);
	objDA.SetGetModule(CMS::GetSI()->ApplicationSettings.ModuleStringID);
	objDA.SetGetObject(_T("CMS_CUSTOMREPORTS"));

	CDataSet objDS;
	CXMLDocument xmlDoc(XMLTags::Root);
	CXMLElement root = xmlDoc.GetDocumentElement();
	//-------------------------------------
	const bool bolStatus = objDA.GetData(xmlDoc, objDS) == rcSuccess && objDS.TableExists(XMLTags::Table);
	const stuCTNode& CTRights = si->UserRight.m_stuControllerNode;
	if (bolStatus)
	{
		m_grdGrid->LoadFromDB(objDS, XMLTags::Table);

	}
	// Change form mode if grid has no records.
	else
	{
		if (!CTRights.Options.ModulePreference.CustomOptions.bCustomReports)
		{
			m_grdGrid->ChangeMode(MODE_VIEW);
		}
	}
	xmlDoc.Dispose();
	objDS.Dispose();
}

// InTimeTec: 12.20.2012.HS - Custom Menus
// Call the server to get the records to load custom menus grid.
void CMP_PPG_CustomOptions::LoadCustomMenusGrid(void)
{
	using namespace CMSStrings;
	CCMSDataAdapter	objDA;
	objDA.SetGetCommand(XMLTags::GET);
	objDA.SetGetModule(CMS::GetSI()->ApplicationSettings.ModuleStringID);
	objDA.SetGetObject(_T("CMS_CUSTOMMENUS"));
	CDataSet objDS;
	CXMLDocument xmlDoc(XMLTags::Root);
	CXMLElement root = xmlDoc.GetDocumentElement();
	const bool bolStatus = objDA.GetData(xmlDoc, objDS) == rcSuccess && objDS.TableExists(XMLTags::Table);

	const stuCTNode& CTRights = si->UserRight.m_stuControllerNode;
	if (bolStatus)
	{
		m_grdGridMenu->LoadFromDB(objDS, XMLTags::Table);
	}
	// Change form mode if grid has no records.
	else
	{
		if (!CTRights.Options.ModulePreference.CustomOptions.bCustomMenus)
		{
			m_grdGridMenu->ChangeMode(MODE_VIEW);
		}
	}
	xmlDoc.Dispose();
	objDS.Dispose();
}
// InTimeTec: End

// Validate grid data before save.
bool CMP_PPG_CustomOptions::IsValid()
{
	// InTimeTec: 12.20.2012.HS - Custom Menus
	return (m_grdGridMenu->IsValid() && m_grdGrid->IsValid(m_grdGridMenu->m_ulPublishedMenuSize));
	// InTimeTec: End
}

// Appent virtual row at startup
void CMP_PPG_CustomOptions::SetCustomReportsGrid()
{
	const stuCTNode& CTRights = si->UserRight.m_stuControllerNode;
	if (CTRights.Options.ModulePreference.CustomOptions.bCustomReports)
	{
		long lNumberRows = m_grdGrid->GetTrueRowCount();
		if (lNumberRows >= 5)
		{
			m_grdGrid->SetNumberRows(lNumberRows + 1);
			m_grdGrid->m_nRows = m_grdGrid->GetNumberRows();
			m_grdGrid->AdjustComponentSizes();
			m_grdGrid->RedrawAll();
		}
	}
}

// Appent virtual row at startup
void CMP_PPG_CustomOptions::SetCustomMenusGrid()
{
	const stuCTNode& CTRights = si->UserRight.m_stuControllerNode;
	if (CTRights.Options.ModulePreference.CustomOptions.bCustomMenus)
	{
		long lNumberRows = m_grdGridMenu->GetTrueRowCount();
		if (lNumberRows >= 5)
		{
			m_grdGridMenu->SetNumberRows(lNumberRows + 1);
			m_grdGridMenu->m_nRows = m_grdGridMenu->GetNumberRows();
			m_grdGridMenu->AdjustComponentSizes();
			m_grdGridMenu->RedrawAll();
		}
	}
}
