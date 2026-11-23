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
 *	Date:		9/10/2012
 *	Author:	    InTimeTec: KP/HS
 ******************************************************************************/
#include "stdafx.h"
#include "MP_PPG_CustomOptions.h"
#include "MP_GRD_CustomOptions.h"
#include "..\cmsdll\gblcommondialog.h"
#include "..\cmsdll\gblfile.h"
#include "..\cmsdll\gblpath.h"

 // Max File Size(Bytes) for Publish[100 MB]
#define ULMAX_SIZE 100000000

IMPLEMENT_DYNAMIC(CLTCustomOptions, CCMSLTBase)
//---------------------------------------------------------------------------

CGRD_CustomOptionsEx::CGRD_CustomOptionsEx(ICMSGridEntryBL& clsBL)
	: CCMSGridEntry(clsBL)
{
	m_bmpDrillDown.Attach(CMS::GetSI()->Drawing.Bitmaps.Drilldown16);
	m_bDeleteRow = false;
	m_bAppendRow = true;
	m_ulPublishedReportSize = 0;
	// InTimeTec: 10/31/2013 : HS : Bug 9608: DEV_CT_Custom Options tab, Module Preferences form, Location/File Name field focus not moving using mouse click on next detail line
	// added the falg to prevent the tab out from a detail line if validation fails
	m_bPreventTabOut = true;
}

void CGRD_CustomOptionsEx::OnSetup()
{
	__super::OnSetup();

	EnableJoins(TRUE);
	JoinCells(CLTCustomOptions::COL_REPORTLOC, -1, CLTCustomOptions::COL_REPORTLOC_DRILLDOWN, -1);
}

CGRD_CustomOptionsEx::~CGRD_CustomOptionsEx()
{
	m_bmpDrillDown.Detach();
}

//===========================================================================
CLTCustomOptions::CLTCustomOptions(CMP_PPG_CustomOptions& clsPage, const CString& szLineType, CCMSGridEntry& clsGrid)
	: CCMSLTBase(szLineType, clsGrid)
	, m_clsPage(clsPage)
	, m_szReportLoc(EMPTY_STRING)
	, m_szReportName(EMPTY_STRING)
	, m_bPublishToServer(false)
	, m_bVerified(false)
	, m_bModified(false)
	, m_intRowID(-1)
	, m_eModuleType(MOD_NONE)
	, m_eReportType(RPT_NONE)
	, m_bIsUpdated(false)
	, m_bRowDirty(false)
{
}
//---------------------------------------------------------------------------

CLTCustomOptions::~CLTCustomOptions(void)
{

}
//---------------------------------------------------------------------------

// get text into grid
CString CLTCustomOptions::GetCellText(int nColumnID, int nLTMapType)
{
	CString	szValue(EMPTY_STRING);
	switch (nColumnID)
	{
	case COL_REPORTTYPE:
		break;
	case COL_REPORTLOC:
		szValue = m_szReportLoc;
		break;
	case COL_REPORTNAME:
		szValue = m_szReportName;
		break;
	case COL_REPORTLOC_DRILLDOWN:
		szValue = _T("...");
		break;
	}
	if (!szValue.IsEmpty())
	{
		return szValue;
	}
	return __super::GetCellText(nColumnID, nLTMapType);
}
//---------------------------------------------------------------------------

// set text from grid
bool CLTCustomOptions::SetCellText(int nColumnID, CString& szCellText, CString& szNewLineType, int nLTMapType)
{
	switch (nColumnID)
	{
	case COL_REPORTLOC:
		m_szReportLoc = szCellText;
		m_bRowDirty = true;
		if (!(CMS::FileExists(m_szReportLoc) && (m_szReportLoc.Find(_T(".")) > 0)))
		{
			m_bIsUpdated = true;
		}
		/*if(m_szSavedLocation.CompareNoCase(szCellText))
		{
			m_bLocationChanged = true;
		}
		else
		{
			m_bLocationChanged = false;
		}*/
		break;
	case COL_REPORTNAME:
		m_szReportName = szCellText;
		m_bRowDirty = true;
		break;
	}
	return __super::SetCellText(nColumnID, szCellText, szNewLineType, nLTMapType);
}
//---------------------------------------------------------------------------

// Get bool into grid
bool CLTCustomOptions::GetCellBool(int nColumnID, int nLTMapType)
{
	switch (nColumnID)
	{
	case COL_PUBLISHTOSRV:
		return m_bPublishToServer;
		break;
	}
	return __super::GetCellBool(nColumnID, nLTMapType);
}
//---------------------------------------------------------------------------

// Set bool from grid
bool CLTCustomOptions::SetCellBool(int nColumnID, bool& bValue, CString& szNewLineType, int nLTMapType)
{
	switch (nColumnID)
	{
	case COL_PUBLISHTOSRV:
		m_bPublishToServer = bValue;
		m_bRowDirty = true;
		if (m_bPublishToServer)
		{
			SetCellEnabled(1, true);
			SetUpdate(true);
		}
		/*if(m_bSavedPublished != bValue)
		{
			m_bPublishedChanged = true;
		}
		else
		{
			m_bPublishedChanged = false;
		}*/
		break;
	}
	return __super::SetCellBool(nColumnID, bValue, szNewLineType, nLTMapType);
}
//---------------------------------------------------------------------------

// set combo value into grid
CString CLTCustomOptions::GetCellComboValue(int nColumnID, int nLTMapType)
{
	switch (nColumnID)
	{
	case COL_REPORTTYPE:
		return CGBLForm::ToString(int(m_eReportType));

	}
	return __super::GetCellComboValue(nColumnID, nLTMapType);
}

// save combo value from grid
bool CLTCustomOptions::SetCellComboValue(int nColumnID, CString& szValue, CString& szNewLineType, int nLTMapType)
{
	int nValue = _tstoi(szValue);
	switch (nColumnID)
	{
	case COL_REPORTTYPE:
		m_eReportType = enuReportType(nValue);
		m_bRowDirty = true;
		break;
	}
	return __super::SetCellComboValue(nColumnID, szValue, szNewLineType, nLTMapType);
}
//---------------------------------------------------------------------------

// Check if this line type is empty
bool CLTCustomOptions::IsLineEmpty(void) const
{
	if (m_szReportLoc.IsEmpty() || m_szReportName.IsEmpty() || m_eReportType < 1) return true;
	return false;
}
//---------------------------------------------------------------------------

bool CLTCustomOptions::CanAddNewRowAtEnd()
{
	const stuCTNode& CTRights = si->UserRight.m_stuControllerNode;
	if (!IsLineEmpty() && CTRights.Options.ModulePreference.CustomOptions.bCustomReports)	return true;
	return false;
}
//---------------------------------------------------------------------------

// InTimeTec: 10/25/2013 : TKG : Bug 9608: DEV_CT_Custom Options tab, Module Preferences form, Location/File Name field focus not moving using mouse click on next detail line
// Method for validation of required fields on a detail line
bool CLTCustomOptions::ValidateRowBeforeLeave(long lRow)
{
	if (m_szReportLoc.IsEmpty() && m_szReportName.IsEmpty() && m_eReportType < 1) return true;
	if (IsLineEmpty())
	{
		if (m_eReportType < 1)
		{
			CGBLForm::CMSMessageBox(AfxGetMainWnd(), IDS_PLEASE_ENTER_REPORT_TYPE, IDS_REPORT_TYPE_REQUIRED, MB_OK | MB_ICONWARNING, NULL, 0, false);
			GetTypedGrid().ValFailSetFocus(COL_REPORTTYPE, lRow);
			return false;
		}
		if (m_szReportName.IsEmpty())
		{
			CGBLForm::CMSMessageBox(AfxGetMainWnd(), IDS_ENTER_REPORT_NAME, IDS_REPORT_NAME_REQUIRED, MB_OK | MB_ICONWARNING, NULL, 0, false);
			GetTypedGrid().ValFailSetFocus(COL_REPORTNAME, lRow);
			return false;
		}
		if (m_szReportLoc.IsEmpty())
		{
			CGBLForm::CMSMessageBox(AfxGetMainWnd(), IDS_PLEASE_ENTER_REPORT_LOCATION, IDS_REPORT_LOCATION_REQUIRED, MB_OK | MB_ICONWARNING, NULL, 0, false);
			GetTypedGrid().ValFailSetFocus(COL_REPORTLOC, lRow);
			return false;
		}

	}
	return true;
}
// InTimeTec: End
//---------------------------------------------------------------------------

bool CLTCustomOptions::CanDeleteRow(long lRow, CString& szNewLineType)
{
	return __super::CanDeleteRow(lRow, szNewLineType);
}
//---------------------------------------------------------------------------

bool CLTCustomOptions::IsValidData(int nLTMapType, long lRow, int& nFailColumnID)
{
	// InTimeTec: 10/25/2013 : TKG : Bug 9608: DEV_CT_Custom Options tab, Module Preferences form, Location/File Name field focus not moving using mouse click on next detail line
	//If all the fields in a row on the grid is empty then it save without showing error
	if (m_szReportLoc.IsEmpty() && m_szReportName.IsEmpty() && m_eReportType < 1) return true;
	if (!ValidateRowBeforeLeave(lRow))
	{
		if (m_eReportType < 1)
		{
			GetTypedGrid().ValFailSetFocus(COL_REPORTTYPE, lRow);
			return false;
		}

		if (m_szReportName.IsEmpty())
		{
			GetTypedGrid().ValFailSetFocus(COL_REPORTNAME, lRow);
			return false;
		}

		if (m_szReportLoc.IsEmpty())
		{
			GetTypedGrid().ValFailSetFocus(COL_REPORTLOC, lRow);
			return false;
		}


	}
	//InTimeTec: End
	return __super::IsValidData(nLTMapType, lRow, nFailColumnID);
}
//---------------------------------------------------------------------------

// send grid data to server
void CLTCustomOptions::SaveToDB(CXMLElement& eleRow, long lDetailNo, int nLTMapType, bool& bProcessed)
{
	if (IsLineEmpty()) return;

	CString szModule;
	CString szReport;
	szModule = GetModuleName(m_eReportType);
	szReport = GetReportName(m_eReportType);
	eleRow.AddNewChild(_T("intRecordID"), m_intRowID);
	eleRow.AddNewChild(_T("strModule"), szModule);
	eleRow.AddNewChild(_T("strReportType"), szReport);
	eleRow.AddNewChild(_T("strPath"), m_szReportLoc);
	eleRow.AddNewChild(_T("strReportName"), m_szReportName);
	eleRow.AddNewChild(_T("intOrder"), lDetailNo);
	eleRow.AddNewChild(_T("strMachineName"), CMS::GetSI()->SystemData.ComputerName);
	eleRow.AddNewChild(_T("strUserID"), CMS::GetSI()->UserInformation.ID);
	eleRow.AddNewChild(_T("bolPublished"), m_bPublishToServer);
	eleRow.AddNewChild(_T("bIsUpdated"), m_bIsUpdated);
	eleRow.AddNewChild(_T("bIsRowModified"), m_bRowDirty);

	__super::SaveToDB(eleRow, lDetailNo, nLTMapType, bProcessed);
}
//---------------------------------------------------------------------------

// load data from db
void CLTCustomOptions::LoadFromDB(CDataRow& drRow, int nLTMapType, bool& bProcessed)
{
	m_intOrder = 0;

	const stuCTNode& CTRights = si->UserRight.m_stuControllerNode;
	if (!CTRights.Options.ModulePreference.CustomOptions.bCustomReports)
	{
		SetCellEnabled(COL_REPORTTYPE, false);
		SetCellEnabled(COL_REPORTLOC, false);
		SetCellEnabled(COL_REPORTLOC_DRILLDOWN, false);
		SetCellEnabled(COL_REPORTNAME, false);
		SetCellEnabled(COL_PUBLISHTOSRV, false);
	}

	if (drRow.IsEmpty()) return;

	m_intRowID = drRow.XMLInt(_T("intRecordID"));
	CString szModule = drRow.XMLStr(_T("strModule"));
	CString szReportType = drRow.XMLStr(_T("strReportType"));
	m_eReportType = (enuReportType)GetReportValue(szModule, szReportType);
	m_szReportLoc = drRow.XMLStr(_T("strPath"));
	m_szReportName = drRow.XMLStr(_T("strReportName"));
	m_szFileName = drRow.XMLStr(_T("strFileName"));
	m_szHashData = drRow.XMLStr(_T("strFileHash"));
	m_dteLastModified = drRow.XMLDate(_T("dteLastModified"));

	if (m_szReportLoc.GetAt(m_szReportLoc.GetLength() - 1) == '\\')
	{
		m_szReportLoc = m_szReportLoc + m_szFileName;
	}
	else
	{
		m_szReportLoc = m_szReportLoc + '\\' + m_szFileName;
	}

	if (!m_szHashData.IsEmpty())
	{
		m_bPublishToServer = true;
	}

	__super::LoadFromDB(drRow, nLTMapType, bProcessed);
}
//---------------------------------------------------------------------------

//Handle commands from picture/drilldown columns
void CLTCustomOptions::OnCommandBtn(int nColumnID)
{
	switch (nColumnID)
	{
	case COL_REPORTLOC_DRILLDOWN:
	{
		if (!GetTypedGrid().m_bEditMode) return;
		//correctly terminated the filetype string
		wchar_t Filter[] = _T("Crystal Reports\0*.rpt\0\0");
		CString szFilePath;
		szFilePath = CGBLCommonDialog::ShowFile(true, m_szReportLoc, Filter, _T("Select Report"), _T(".rpt"));

		if (!szFilePath.IsEmpty())
		{
			m_bRowDirty = true;
			SetUpdate(true);
			long lCurrentRow = GetTypedGrid().GetCurrentRow();
			m_szReportLoc = szFilePath;
			GetTypedGrid().ValFailSetFocus(COL_REPORTLOC, lCurrentRow);
			SetVerified(false);
			SetModified(true);
			m_clsPage.SetModifiedFlagFromGrid(TRUE);
		}
	}

	break;
	};
}

//---------------------------------------------------------------------------
CString CLTCustomOptions::GetModuleName(int enuModuleValue)
{
	// JEPK 2/24/2015 PBI 27673 added Custom Report Options
	switch (enuModuleValue)
	{
	case RPT_APAGING:
	case RPT_APIMMEDIATECHECKS:
	case RPT_APPRINTCHECKS:
	case RPT_APENTERBILLS:
	case RPT_APPAYBILLS:
		return CGBLResources::GetResourceString(IDS_AP_STRING);

	case RPT_ARCUSTSTATEMENTS:
	case RPT_ARAGEDANALYSIS:
	case RPT_ARDATEDINVOICETOPAYMENT:
		return CGBLResources::GetResourceString(IDS_AR_MODULE);

	case RPT_BRPRINTCHECKS:
		return CGBLResources::GetResourceString(IDS_BR_MODULE);

	case RPT_GLAUDITTRIAL:
	case RPT_GLBALANCESHEET:
	case RPT_GLBUDGETPERFORMANCE:
	case RPT_GLBUDGETTRACKING:
	case RPT_GLREVENUEEXPENSEINCOMESTATEMENT:
		return CGBLResources::GetResourceString(IDS_GL_MODULE);

	case RPT_INHISTORY:
	case RPT_INONHAND:
	case RPT_INONORDER:
		return CGBLResources::GetResourceString(IDS_IN_MODULE);

	case RPT_POPRINTPURCHASE:
	case RPT_POREPRINTPURCHASE:
		return CGBLResources::GetResourceString(IDS_PO_MODULE);

	case RPT_PRCHECK:
	case RPT_PREDITREPORT:
	case RPT_PREMPLOYEEHISTORY:
	case RPT_PREMPLOYEELEAVE:
		return CGBLResources::GetResourceString(IDS_PR_MODULE);

	case RPT_OEEDITREPORT:
	case RPT_OEPRINTINVOICES:
	case RPT_OEPRINTINVOICESFRMHISTORY:
	case RPT_OEPACKINGSLIPS:
	case RPT_OEPICKLIST:
		return CGBLResources::GetResourceString(IDS_OE_MODULE);

	case RPT_SAPROFITMARGIN:
	case RPT_SASALESBYSALESPERSON:
		return CGBLResources::GetResourceString(IDS_SA_MODULE);

	case RPT_PSEDITREPORT:
		return CGBLResources::GetResourceString(IDS_PS_MODULE);

	default:
		return _T("");
	}
}

CString CLTCustomOptions::GetReportName(int enuReportValue)
{
	CString szReportName;
	switch (enuReportValue)
	{
	case RPT_APAGING:
		szReportName = CGBLResources::GetResourceString(IDS_AGING);
		break;

	case RPT_APENTERBILLS:
		szReportName = CGBLResources::GetResourceString(IDS_ENTER_BILLS_PRINT_EDIT_REPORT);
		break;

	case RPT_APPAYBILLS:
		szReportName = CGBLResources::GetResourceString(IDS_PAY_BILLS_PRINT_EDIT_REPORT);
		break;

	case RPT_APIMMEDIATECHECKS:
		szReportName = CGBLResources::GetResourceString(IDS_AP_PRINTIMMEDIATECHECKS1);
		break;

	case RPT_APPRINTCHECKS:
		szReportName = CGBLResources::GetResourceString(IDS_AP_PRINTCHECKS1);
		break;

	case RPT_ARCUSTSTATEMENTS:
		szReportName = CGBLResources::GetResourceString(IDS_CTRL_CUST_STATEMENTS);
		break;

	case RPT_ARAGEDANALYSIS:
		szReportName = CGBLResources::GetResourceString(IDS_CTRL_AGED_ANALYSIS);
		break;

	case RPT_ARDATEDINVOICETOPAYMENT:
		szReportName = CGBLResources::GetResourceString(IDS_DATED_INVOICE_PAYMENT_REPORT);
		break;

	case RPT_BRPRINTCHECKS:
		szReportName = CGBLResources::GetResourceString(IDS_AP_PRINTCHECKS1);
		break;

	case RPT_GLAUDITTRIAL:
		szReportName = CGBLResources::GetResourceString(IDS_AUDITTRAILREPORT);
		break;

	case RPT_GLBALANCESHEET:
		szReportName = CGBLResources::GetResourceString(IDS_GLBALANCESHEETREPORT);
		break;

	case RPT_GLBUDGETPERFORMANCE:
		szReportName = CGBLResources::GetResourceString(IDS_GLFBUDGETPERFORMANCEREPORT);
		break;

	//	PBI 47610	03/05/2019	mvt - Added budget tracking report.
	case RPT_GLBUDGETTRACKING:
		szReportName = CGBLResources::GetResourceString(IDS_BUDGET_TRACKING_REPORT);
		break;

	case RPT_GLREVENUEEXPENSEINCOMESTATEMENT:
		if (CMS::GetSI()->CompanySettings.IsFund)
			szReportName = CGBLResources::GetResourceString(IDS_REVENUE_AND_EXPENSE_REPORT);
		else
			szReportName = CGBLResources::GetResourceString(IDS_INCOME_STATEMENT_REPORT);
		break;

	case RPT_INHISTORY:
		szReportName = CGBLResources::GetResourceString(IDS_PO_CAP_HISTORY_REPORT);
		break;

	case RPT_INONHAND:
		szReportName = CGBLResources::GetResourceString(IDS_ONHANDREPORT);
		break;

	case RPT_INONORDER:
		szReportName = CGBLResources::GetResourceString(IDS_IN_ON_ORDER_REPORT_TITLE);
		break;

	case RPT_OEEDITREPORT:
		szReportName = CGBLResources::GetResourceString(IDS_MSG_PRINT_EDIT_REPORT);
		break;

	case RPT_OEPRINTINVOICES:
		szReportName = CGBLResources::GetResourceString(IDS_CT_PRINT_INVOICES);
		break;

	case RPT_OEPRINTINVOICESFRMHISTORY:
		szReportName = CGBLResources::GetResourceString(IDS_CT_REPRINT_INVOICES_FROM_HISTORY);
		break;

	case RPT_OEPACKINGSLIPS:
		szReportName = CGBLResources::GetResourceString(IDS_PRINT_PACKING_SLIPS);
		break;

	case RPT_OEPICKLIST:
		szReportName = CGBLResources::GetResourceString(IDS_SA_PICKING_TITLE);
		break;

	case RPT_POPRINTPURCHASE:
		szReportName = CGBLResources::GetResourceString(IDS_PRINT_PO_RPT);
		break;

	case RPT_POREPRINTPURCHASE:
		szReportName = CGBLResources::GetResourceString(IDS_POMENU_REPRINT_POS_STATUSBAR);
		break;

	case RPT_PRCHECK:
		szReportName = CGBLResources::GetResourceString(IDS_AP_PRINTCHECKS1);
		break;

	case RPT_PREDITREPORT:
		szReportName = CGBLResources::GetResourceString(IDS_MSG_PRINT_EDIT_REPORT);
		break;

	case RPT_PREMPLOYEEHISTORY:
		szReportName = CGBLResources::GetResourceString(IDS_EMPLOYEE_HISTORY_REPORT);
		break;

	case RPT_PREMPLOYEELEAVE:
		szReportName = CGBLResources::GetResourceString(IDS_EMPLOYEE_LEAVE_REPORT);
		break;

	case RPT_PSEDITREPORT:
		szReportName = CGBLResources::GetResourceString(IDS_MSG_PRINT_EDIT_REPORT);
		break;

	case RPT_SAPROFITMARGIN:
		szReportName = CGBLResources::GetResourceString(IDS_SA_PROFIT_MARGIN_REPORT);
		break;

	case RPT_SASALESBYSALESPERSON:
		szReportName = CGBLResources::GetResourceString(IDS_SA_SALES_BY_SALESPERSON_CAP);
		break;
	}
	return szReportName;
}

int CLTCustomOptions::GetReportValue(CString szModule, CString szReport)
{
	// JEPK 2/24/2015 PBI 27673 added Custom Report Options
	if (szModule == CGBLResources::GetResourceString(IDS_AP_STRING))
	{
		if (szReport == CGBLResources::GetResourceString(IDS_AGING))
		{
			return RPT_APAGING;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_AP_PRINTCHECKS1))
		{
			return RPT_APPRINTCHECKS;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_AP_PRINTIMMEDIATECHECKS1))
		{
			return RPT_APIMMEDIATECHECKS;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_ENTER_BILLS_PRINT_EDIT_REPORT))
		{
			return RPT_APENTERBILLS;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_PAY_BILLS_PRINT_EDIT_REPORT))
		{
			return RPT_APPAYBILLS;
		}
	}

	else if (szModule == CGBLResources::GetResourceString(IDS_AR_MODULE))
	{
		if (szReport == CGBLResources::GetResourceString(IDS_CTRL_CUST_STATEMENTS))
		{
			return RPT_ARCUSTSTATEMENTS;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_CTRL_AGED_ANALYSIS))
		{
			return RPT_ARAGEDANALYSIS;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_DATED_INVOICE_PAYMENT_REPORT))
		{
			return RPT_ARDATEDINVOICETOPAYMENT;
		}
	}

	else if (szModule == CGBLResources::GetResourceString(IDS_BR_MODULE))
	{
		return RPT_BRPRINTCHECKS;
	}

	else if (szModule == CGBLResources::GetResourceString(IDS_GL_MODULE))
	{
		if (szReport == CGBLResources::GetResourceString(IDS_AUDITTRAILREPORT))
		{
			return RPT_GLAUDITTRIAL;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_GLBALANCESHEETREPORT))
		{
			return RPT_GLBALANCESHEET;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_GLFBUDGETPERFORMANCEREPORT))
		{
			return RPT_GLBUDGETPERFORMANCE;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_BUDGET_TRACKING_REPORT))
		{
			return RPT_GLBUDGETTRACKING;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_REVENUE_AND_EXPENSE_REPORT) || szReport == CGBLResources::GetResourceString(IDS_INCOME_STATEMENT_REPORT))
		{
			return RPT_GLREVENUEEXPENSEINCOMESTATEMENT;
		}
	}

	else if (szModule == CGBLResources::GetResourceString(IDS_IN_MODULE))
	{
		if (szReport == CGBLResources::GetResourceString(IDS_PO_CAP_HISTORY_REPORT))
		{
			return RPT_INHISTORY;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_ONHANDREPORT))
		{
			return RPT_INONHAND;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_IN_ON_ORDER_REPORT_TITLE))
		{
			return RPT_INONORDER;
		}
	}

	else if (szModule == CGBLResources::GetResourceString(IDS_OE_MODULE))
	{
		if (szReport == CGBLResources::GetResourceString(IDS_MSG_PRINT_EDIT_REPORT))
		{
			return RPT_OEEDITREPORT;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_CT_PRINT_INVOICES))
		{
			return RPT_OEPRINTINVOICES;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_CT_REPRINT_INVOICES_FROM_HISTORY))
		{
			return RPT_OEPRINTINVOICESFRMHISTORY;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_PRINT_PACKING_SLIPS))
		{
			return RPT_OEPACKINGSLIPS;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_SA_PICKING_TITLE))
		{
			return RPT_OEPICKLIST;
		}
	}

	else if (szModule == CGBLResources::GetResourceString(IDS_PO_MODULE))
	{
		if (szReport == CGBLResources::GetResourceString(IDS_PRINT_PO_RPT))
		{
			return RPT_POPRINTPURCHASE;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_POMENU_REPRINT_POS_STATUSBAR))
		{
			return RPT_POREPRINTPURCHASE;
		}
	}

	else if (szModule == CGBLResources::GetResourceString(IDS_PR_MODULE))
	{
		if (szReport == CGBLResources::GetResourceString(IDS_AP_PRINTCHECKS1))
		{
			return RPT_PRCHECK;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_MSG_PRINT_EDIT_REPORT))
		{
			return RPT_PREDITREPORT;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_EMPLOYEE_HISTORY_REPORT))
		{
			return RPT_PREMPLOYEEHISTORY;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_EMPLOYEE_LEAVE_REPORT))
		{
			return RPT_PREMPLOYEELEAVE;
		}
	}

	else if (szModule == CGBLResources::GetResourceString(IDS_PS_MODULE))
	{
		return RPT_PSEDITREPORT;
	}

	else if (szModule == CGBLResources::GetResourceString(IDS_SA_MODULE))
	{
		if (szReport == CGBLResources::GetResourceString(IDS_SA_PROFIT_MARGIN_REPORT))
		{
			return RPT_SAPROFITMARGIN;
		}
		else if (szReport == CGBLResources::GetResourceString(IDS_SA_SALES_BY_SALESPERSON_CAP))
		{
			return RPT_SASALESBYSALESPERSON;
		}
	}

	return 0;
}

// InTimeTec: 10/31/2013 : HS : Bug 9608: DEV_CT_Custom Options tab, Module Preferences form, Location/File Name field focus not moving using mouse click on next detail line
// validate if it is ok to leave the detail line
bool CGRD_CustomOptionsEx::CanLeaveRow(long lRow, int& nNewCol, bool* pbRowWasDeleted)
{
	CLTCustomOptions* const pCurrentRow = static_cast<CLTCustomOptions *>(GetDataSource().GetLTBase(lRow));

	if (pCurrentRow && !pCurrentRow->ValidateRowBeforeLeave(lRow)) return false;
	return true;
}

//To get Context menu
void CGRD_CustomOptionsEx::GetContextMenu(CMenu* pPopup, long lRow, int nCol)
{
	//__super::GetContextMenu(CMenu* pPopup, long lRow, int nCol);
	if (lRow < 0) return;

	//Remove all items from menu
	if (pPopup->GetSafeHmenu())
		while (pPopup->DeleteMenu(0, MF_BYPOSITION) != FALSE);
	//Use common routine to add items to menu
	PopulateMenuItems(lRow, pPopup);
}
//---------------------------------------------------------------------------

void CGRD_CustomOptionsEx::PopulateMenuItems(long lRow, CMenu* pMenu/*=NULL*/)
{
	CLTCustomOptions* const pCurrentRow = static_cast<CLTCustomOptions *>(GetDataSource().GetLTBase(GetCurrentRow()));
	CString szHashData = pCurrentRow->GetHashValue();
	bool bIsPublished = pCurrentRow->IsPublished();
	bool bEmpty = pCurrentRow->GetReportLoc().IsEmpty();
	DWORD dwStyle = pCurrentRow->GetCellStyle(CLTCustomOptions::COL_PUBLISHTOSRV);

	AddMenuItem(RMV_SRV, CGBLResources::GetResourceString(IDS_REMOVE_SVR), pMenu, bIsPublished && !szHashData.IsEmpty());
	AddMenuItem(RMV_GRDSRV, CGBLResources::GetResourceString(IDS_REMOVE_GRID_SVR), pMenu, true);
	AddMenuItem(DWN_EDIT, CGBLResources::GetResourceString(IDS_DOWNLOAD_FOR_EDITING), pMenu, !bEmpty && (dwStyle != 10));
}

void CGRD_CustomOptionsEx::AddMenuItem(const UINT unID, const CString& szString, CMenu* pMenu, bool bEnable)
{
	pMenu->AppendMenu(MF_BYCOMMAND, unID, szString);
	if (!bEnable)
	{
		pMenu->EnableMenuItem(unID, MF_DISABLED | MF_GRAYED | MF_BYCOMMAND);
	}
}

int CGRD_CustomOptionsEx::OnCMSMenuCommand(int col, long row, int item)
{
	CLTCustomOptions* const pCurrentRow = static_cast<CLTCustomOptions *>(GetDataSource().GetLTBase(GetCurrentRow()));
	int lCurrentRowID = pCurrentRow->GetRowID();
	bool bResult;
	switch (item)
	{
	case RMV_SRV:
		bResult = RemoveFromServer(lCurrentRowID, false);
		if (bResult)
		{
			pCurrentRow->SetPublished(!bResult);
			pCurrentRow->SetHashValue(_T(""));
			RedrawAll();
		}
		break;

	case RMV_GRDSRV:
		bResult = RemoveFromServer(lCurrentRowID, true);
		if (bResult)
		{
			long lOldCurrentRow = GetCurrentRow();
			__super::OnDeleteRow(lOldCurrentRow);

			long lNumberRows = GetNumberRows();
			if (lNumberRows >= 5)
			{
				SetNumberRows(lNumberRows + 1);
			}
			RedrawAll();
		}
		break;

	case DWN_EDIT:
		const long lRow = GetCurrentRow();
		CLTCustomOptions* const pRow = static_cast<CLTCustomOptions *>(GetDataSource().GetLTBase(lRow));

		wchar_t szFilter[] = _T("Crystal Report (*.rpt)\0*.rpt");
		CString szReturn = CGBLCommonDialog::ShowFile(false, _T(""), szFilter, CGBLResources::GetResourceString(IDS_DOWNLOAD_FOR_EDITING), _T("rpt"));
		if (szReturn != "")
		{
			CString szSource;
			szSource = pRow->GetReportLoc();
			CGBLFile::CMSCopyFile(szSource, szReturn);
		}
		break;
	}
	return 1;
}
//---------------------------------------------------------------------------

//Remove report from server
bool CGRD_CustomOptionsEx::RemoveFromServer(int lCurrentRowID, bool bRemoveBoth, bool bMsgBox /* = TRUE*/)
{
	using namespace CMSStrings;
	bool bReturn = false;
	CString szMsg((RESSTRING(IDS_AREYOUSURE)));
	int nAnswer;
	if (bMsgBox)
	{
		nAnswer = CGBLForm::CMSMessageBox(::AfxGetMainWnd(), 0, IDS_DEL_ALLOCATIONCODEMSG1, MB_YESNO + MB_ICONQUESTION, szMsg);
	}
	else
	{
		nAnswer = IDYES;
	}

	switch (nAnswer)
	{
	case IDYES:
	{
		CXMLDocument docXMLData(CMSStrings::XMLTags::Root), docXMLReturn;
		CXMLElement eleRoot = docXMLData.GetDocumentElement();
		CXMLElement eleTable = eleRoot.AddNewChild(CMSStrings::XMLTags::TABLE);
		eleTable.AddNewChild(_T("intRecordID"), lCurrentRowID);
		eleTable.AddNewChild(_T("bolBoth"), bRemoveBoth);
		eleTable.AddNewChild(_T("bolMenu"), 0);

		CCMSDataAdapter clsAdapter(CMSStrings::XMLTags::UPDATE, _T("CT"), _T("CMS_REMOVEFILE"));
		if (clsAdapter.GetData(docXMLData, docXMLReturn) == rcSuccess)
			return true;
	}
	break;
	}

	return bReturn;
}
//---------------------------------------------------------------------------

// Overloaded OnEditFinish for validating if file already exists on server.
int CGRD_CustomOptionsEx::OnEditFinish(int col, long row, CWnd *edit, LPCTSTR string, BOOL cancelFlag)
{
	CLTCustomOptions* const pRow = static_cast<CLTCustomOptions *>(GetDataSource().GetLTBase(row));
	if (col == CLTCustomOptions::COL_REPORTLOC && !pRow->GetHashValue().IsEmpty() && pRow->IsModified())
	{
		CFileFind find;
		CString szPath = pRow->GetFileName();
		CString szText(string);
		int nValue = 0;
		int nLength = szText.GetLength() - 1;
		nValue = szText.ReverseFind('\\');

		if (!pRow->IsVerified())
		{
			pRow->SetVerified(true);
			if (find.FindFile(szText) && (szText.Find(_T(".")) > 0) && nValue > 0)
			{
				pRow->SetUpdate(true);
				szText = szText.Right(nLength - nValue);
				if (_tcslen(szText) != 0 && !szPath.CompareNoCase(szText))
				{
					CString szMsg((RESSTRING(IDS_SERVER_EXISTING_COPY)));
					szMsg.Replace(_T("<FILENAME>"), pRow->GetFileName());
					const int nAnswer = CGBLForm::CMSMessageBox(this, 0, IDS_SERVER_FILE_EXISTS, MB_YESNO + MB_ICONQUESTION, szMsg);
					if (nAnswer == IDNO)
					{
						pRow->SetUpdate(false);
					}
				}
			}
		}
	}
	return __super::OnEditFinish(col, row, edit, string, cancelFlag);
}

// Validate grid data before saving.
bool CGRD_CustomOptionsEx::IsValid(ULONGLONG& ulPublishedFileSize)
{
	m_ulPublishedReportSize = 0;
	const stuCTNode& CTRights = si->UserRight.m_stuControllerNode;
	if (!CTRights.Options.ModulePreference.CustomOptions.bCustomReports) return true;
	int nFailCol;
	int numrows = GetTrueRowCount();

	for (int nRow = 0; nRow < numrows; nRow++)
	{
		CLTCustomOptions* pLine = DYNAMIC_DOWNCAST(CLTCustomOptions, GetRowAt(nRow));
		if (nRow == numrows - 1 && pLine->IsLineEmpty()) return true;
		if (pLine && !pLine->IsValid(nRow, nFailCol, this))
		{
			ValFailSetFocus(nFailCol, nRow);
			return false;
		}
		if (pLine && !pLine->IsAccessAllowed(nRow, nFailCol, ulPublishedFileSize, this))
		{
			ValFailSetFocus(nFailCol, nRow);
			return false;
		}
	}
	return true;
}

// Validate line type before saving.
bool CLTCustomOptions::IsValid(long lRow, int& nFailColumnID, CWnd* Parent /*=NULL*/)
{
	if (this == NULL) return false;
	if (m_bPublishToServer && !m_bIsUpdated && !m_bRowDirty) return true;

	CString szMessage;
	CFileFind find;
	if (!find.FindFile(m_szReportLoc) || m_szReportLoc.Find(_T(".")) < 0)
	{
		szMessage = RESSTRING(IDS_GBL_INVALID_PATH__FILE_NAME_MSG);
		CGBLForm::CMSMessageBox(AfxGetMainWnd(), 0, IDS_GBL_INVALID_PATH__FILE_NAME, MB_OK | MB_ICONSTOP, szMessage);
		nFailColumnID = COL_REPORTLOC;
		GetTypedGrid().m_ulPublishedReportSize = 0;
		return false;
	}
	return true;
}

// Validate line type before saving.
bool CLTCustomOptions::IsAccessAllowed(long lRow, int& nFailColumnID, ULONGLONG& ulPublishedFileSize, CWnd* Parent /*=NULL*/)
{
	if (this == NULL) return false;
	if (m_bPublishToServer && !m_bIsUpdated && !m_bRowDirty) return true;

	CString szMessage;
	CFile myFile;
	CFileStatus fileStatus;
	CFileException fileException;
	if (m_bPublishToServer  &&  m_bIsUpdated)
	{
		if (!myFile.Open(m_szReportLoc, CFile::modeRead, &fileException))
		{
			szMessage = RESSTRING(IDS_SERVER_ACCESS);
			CGBLForm::CMSMessageBox(AfxGetMainWnd(), 0, IDS_SERVER_INACCESSIBLE, MB_OK | MB_ICONSTOP, szMessage);
			nFailColumnID = COL_REPORTLOC;
			ulPublishedFileSize = 0;
			return false;
		}
		myFile.GetStatus(fileStatus);
		ulPublishedFileSize = ulPublishedFileSize + fileStatus.m_size;
		if (ulPublishedFileSize > ULMAX_SIZE)
		{
			CGBLForm::CMSMessageBox(AfxGetMainWnd(), IDS_FILE_SIZE_ERROR_MESSAGE, IDS_FILE_SIZE_ERROR_TITLE, MB_OK | MB_ICONEXCLAMATION);
			ulPublishedFileSize = 0;
			myFile.Close();
			nFailColumnID = COL_REPORTLOC;
			return false;
		}
		myFile.Close();
	}
	return true;
}

BOOL CGRD_CustomOptionsEx::OnCtrlCharDown(UINT wParam, long row, int col)
{
	if (col == CLTCustomOptions::COL_REPORTLOC)
	{
		CLTCustomOptions* pLine = DYNAMIC_DOWNCAST(CLTCustomOptions, GetRowAt(row));
		if (!pLine->IsModified())
		{
			pLine->SetModified(true);
		}
	}
	return __super::OnCtrlCharDown(wParam, row, col);
}

int CGRD_CustomOptionsEx::OnEditStart(int nCol, long row, CWnd **edit)
{
	if (nCol == CLTCustomOptions::COL_REPORTLOC)
	{
		CLTCustomOptions* pLine = DYNAMIC_DOWNCAST(CLTCustomOptions, GetRowAt(row));
		pLine->SetVerified(false);
		pLine->SetModified(false);
	}
	return __super::OnEditStart(nCol, row, edit);
}


void CGRD_CustomOptionsEx::OnRowChange(long oldrow, long newrow)
{
	__super::OnRowChange(oldrow, newrow);
	if (newrow <= -1) return;

	if (oldrow < newrow || newrow == (GetTrueRowCount() - 1))
	{
		//if the row we are going to is valid and is the last row in the grid and
		//we are in edit mode, then create a new empty row for future entries.
		if (newrow == GetNumberRows() - 1 && m_bEditMode)
		{

			SetNumberRows(GetNumberRows() + 1);
			m_nRows = GetNumberRows();
		}
		if (m_GI->m_currentRow >= 4 && m_GI->m_currentRow == GetTrueRowCount() - 1)
			m_GI->m_topRow = m_GI->m_currentRow - 3;
	}
}
