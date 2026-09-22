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
#include "stdafx.h"
#include "..\CMSCT\MainFrm.h"
#include ".\cmsctreportsmenu.h"


CMainFrame * CCMSCTReportsMenu::m_pclsFrame = NULL;

CCMSCTReportsMenu::MNUIDS CCMSCTReportsMenu::m_vecReportsIntegrationChecklistIDs;
CCMSCTReportsMenu::MNUIDS CCMSCTReportsMenu::m_vecReportsConversionChecklistIDs;
CCMSCTReportsMenu::MNUIDS CCMSCTReportsMenu::m_vecReportsConversionCodesAddedIDs;
CCMSCTReportsMenu::MNUIDS CCMSCTReportsMenu::m_vecReportsConversionIntegrationIDs;

CCMSCTReportsMenu::MNUDATA CCMSCTReportsMenu::m_mapIntegrationErrorReportXML;
CCMSCTReportsMenu::MNUDATA CCMSCTReportsMenu::m_mapConversionChecklistXML;
CCMSCTReportsMenu::MNUDATA CCMSCTReportsMenu::m_mapConversionCodesAddedXML;
CCMSCTReportsMenu::MNUDATA CCMSCTReportsMenu::m_mapConversionIntegrationXML;

CCMSCTReportsMenu::CMenuItemData CCMSCTReportsMenu::m_stuItem;

bool CCMSCTReportsMenu::m_bEnableIntegrationMenu = false;
bool CCMSCTReportsMenu::m_bEnableConversionMenu = false;
bool CCMSCTReportsMenu::m_bIntegrationErrorReportExists = false;
bool CCMSCTReportsMenu::m_bPreConversionReportExists = false;
bool CCMSCTReportsMenu::m_bCodeValuesAddedReportExists = false;
bool CCMSCTReportsMenu::m_bConversionIntegReportExists = false;


//this should never be called; that is why it is private
//------------------------------------------------------------------------------
CCMSCTReportsMenu::CCMSCTReportsMenu(void)
{
}

//this should never be called; that is why it is private
//------------------------------------------------------------------------------
CCMSCTReportsMenu::~CCMSCTReportsMenu(void)
{
}

//this should never be called; that is why it is private
//------------------------------------------------------------------------------
CCMSCTReportsMenu::CCMSCTReportsMenu(const CCMSCTReportsMenu& inRHS)
{
}

//this should never be called; that is why it is private
//------------------------------------------------------------------------------
const CCMSCTReportsMenu& CCMSCTReportsMenu::operator= (const CCMSCTReportsMenu&)
{
	return *this;
}

//------------------------------------------------------------------------------
// 1-33521 BK 10/15/09 added function to make one server call every time the reports menu receives focus
// rather than trying to receive messages from other modules when reports are added or conversion is completed
bool CCMSCTReportsMenu::InitializeReportMenu(void)
{
	// 1-34692 BK 8/20/10 added module prefs to controller, reports older than 30 days will be deleted
	if (CMS::GetSI()->ModuleSettings.CTSettings.PurgeReports30Days)
		RemoveAllReportsOlderThan30Days();

	// 1-33827 BK 12/1/09 for CR 1-33454 added user rights to report menu items
	bool bRightsIntegrationMenu = CMS::GetSI()->UserRight.m_stuControllerNode.Reports.Integration.bTotal;
	bool bRightsConversionMenu = CMS::GetSI()->UserRight.m_stuControllerNode.Reports.Conversion.bTotal;
	if (!bRightsIntegrationMenu && !bRightsConversionMenu)  return false;

	CCMSWaitCursor c;

	CXML		clsXML;
	MakeQueryCommand(clsXML);

	// Send XML to the server, get back result
	CDataSet	objDS;
	CString		szResult(EMPTY_STRING);
	objDS.LoadFromXML(szResult = clsXML.SendXML().copy());
	c.Restore();

	// clear menu then if no integration error reports exist return false to disable entire menu
	RefreshMenu();
	if (szResult.Find(_T("NODATA")) > 0) return false;
	ASSERT(objDS.HasData());

	// Integration Reports
	if (objDS.TableExists(CMSStrings::XMLTags::Table))
	{
		CDataTable	objDT;
		objDS.GetTable(CMSStrings::XMLTags::Table, &objDT);

		CDataRow objDR;
		CString szModule(EMPTY_STRING);
		CString szDateTime(EMPTY_STRING);
		CString szMenuItem(EMPTY_STRING);
		COleDateTime objDateTime;

		const MNUIDS& mnuIDs = GetReportsIntegrationChecklistMenuIDs();

		// load the submenu with the number of menu items returned, up to 5
		for (int nRow = 0; nRow < __min((int)objDT.getRowCount(), (int)mnuIDs.size()); nRow++)
		{
			// 1-33827 BK 12/1/09 for CR 1-33454 added user rights to report menu items
			m_bEnableIntegrationMenu = bRightsIntegrationMenu;
			m_bIntegrationErrorReportExists = CMS::GetSI()->UserRight.m_stuControllerNode.Reports.Integration.bIntegErrorReport;
			objDT.GetRow(nRow, &objDR);

			// 1-34228 BK 6/24 CR 1-34143 add module to timestamp for menuitem
			if (objDR.ColumnExists(_T("strModule")))
				szModule = objDR.XMLStr(_T("strModule")) + _T(" ");
			szDateTime = objDR.XMLStr(_T("dteTimestamp"));
			objDateTime.ParseDateTime(szDateTime);
			szDateTime = objDateTime.Format();

			// 1-34228 BK 6/24 CR 1-34143 add module to timestamp for menuitem
			szMenuItem = szModule + szDateTime;

			// locate the sub menu to which we will be adding report menu items
			CMenu * const pChecklistMenu = GetReportsIntegrationChecklistMenu();
			pChecklistMenu->AppendMenu(MF_STRING, (UINT_PTR)mnuIDs[(size_t)nRow], szMenuItem);
			m_pclsFrame->AddStatusBarMessage((UINT)mnuIDs[(size_t)nRow], (RESSTRING(IDS_STATUS_INTEG_ERROR_RPT) + _T(" - ") + szMenuItem));

			m_stuItem = objDR;
			m_mapIntegrationErrorReportXML[(enmINTEGRATIONREPORTS)nRow] = m_stuItem;
		}
	}
	// Conversion Reports
	if (objDS.TableExists(CMSStrings::XMLTags::Table1))
	{
		CDataTable	objDT;
		objDS.GetTable(CMSStrings::XMLTags::Table1, &objDT);

		CDataRow objDR;
		int	nReportType = 0;

		// check which reports are available
		for (int nRow = 0; nRow < objDT.getRowCount(); nRow++)
		{
			m_bEnableConversionMenu = bRightsConversionMenu;
			objDT.GetRow(nRow, &objDR);
			nReportType = objDR.XMLByte(_T("bytReportType"));

			m_stuItem = objDR;
			// 1-33827 BK 12/1/09 for CR 1-33454 added user rights to report menu items
			switch (nReportType)
			{
			case CGBLPrint::PreConversionDataChecklist:
				m_bPreConversionReportExists = CMS::GetSI()->UserRight.m_stuControllerNode.Reports.Conversion.bPreConversionDataChklst;
				m_mapConversionChecklistXML[0] = m_stuItem;
				break;
			case CGBLPrint::CodesorValuesAdded:
				m_bCodeValuesAddedReportExists = CMS::GetSI()->UserRight.m_stuControllerNode.Reports.Conversion.bCodesValuesAddedReport;
				m_mapConversionCodesAddedXML[0] = m_stuItem;
				break;
			case CGBLPrint::IntegrationReport:
				m_bConversionIntegReportExists = CMS::GetSI()->UserRight.m_stuControllerNode.Reports.Conversion.bIntegrationStatusReport;
				m_mapConversionIntegrationXML[0] = m_stuItem;
				break;
			}
		}
	}
	return true;
}
//------------------------------------------------------------------------------
// remove integration reports menu items
void CCMSCTReportsMenu::RemoveReportsIntegrationChecklistMenu(void)
{
	m_mapIntegrationErrorReportXML.erase(m_mapIntegrationErrorReportXML.begin(), m_mapIntegrationErrorReportXML.end());

	const MNUIDS& mnuIDs = GetReportsIntegrationChecklistMenuIDs();
	CMenu * const pCheckListMenu = GetReportsIntegrationChecklistMenu();

	for (int nID = 0; nID < (int)mnuIDs.size(); nID++)
	{
		pCheckListMenu->DeleteMenu((UINT)mnuIDs[(size_t)nID], MF_BYCOMMAND);
	}
}
//------------------------------------------------------------------------------
// delete integration reports
void CCMSCTReportsMenu::RemoveReportsIntegrationChecklist(void)
{
	CXML clsXML;
	MakeDeleteCommand(clsXML, 1);
	clsXML.SendXML();

	RemoveReportsIntegrationChecklistMenu();
}
//------------------------------------------------------------------------------
// delete conversion reports
void CCMSCTReportsMenu::RemoveConversionReports(void)
{
	CXML clsXML;
	MakeDeleteCommand(clsXML, 0);
	clsXML.SendXML();
}
//------------------------------------------------------------------------------
// delete conversion and integration reports older than 30 days
void CCMSCTReportsMenu::RemoveAllReportsOlderThan30Days(void)
{
	CXML clsXML;
	MakeDeleteCommand(clsXML, 2);
	clsXML.SendXML();
}
//------------------------------------------------------------------------------
// run integration report
void CCMSCTReportsMenu::RunReportsIntegrationChecklist(CCMSCTReportsMenu::enmINTEGRATIONREPORTS eReport)
{
	MNUDATA::const_iterator iter = m_mapIntegrationErrorReportXML.find(eReport);

	ASSERT(iter != m_mapIntegrationErrorReportXML.end());
	m_stuItem = iter->second;

	CString *pszReport = &m_stuItem.m_szReportXML;
	CString szIntReport = CGBLPrint::MakeConvIntReport(m_stuItem.m_eReportType, m_stuItem.m_szCompanyCode16
		, m_stuItem.m_szCompanyName16
		, m_stuItem.m_szDateTime);
	CString *pIntReport = &szIntReport;
	// 1-33612 BK 10/21/09 disable print parameters checkbox
	CGBLPrint::PrintReport(pszReport, pIntReport, eREPORTS, false);

}
//------------------------------------------------------------------------------
// 1-33328 BK 9/25/09 run conversion report - Pre-Conversion Checklist Report
void CCMSCTReportsMenu::RunReportsConversionChecklist()
{
	MNUDATA::const_iterator iter = m_mapConversionChecklistXML.find(0);
	ASSERT(iter != m_mapConversionChecklistXML.end());

	m_stuItem = iter->second;

	CString *pszReportXML = &m_stuItem.m_szReportXML;
	CString szIntReport = CGBLPrint::MakeConvIntReport(m_stuItem.m_eReportType
		, m_stuItem.m_szCompanyCode16
		, m_stuItem.m_szCompanyName16
		, m_stuItem.m_szDateTime);
	CString *pszIntReport = &szIntReport;
	// 1-33612 BK 10/21/09 disable print parameters checkbox
	CGBLPrint::PrintReport(pszReportXML, pszIntReport, eREPORTS, false);
}
//------------------------------------------------------------------------------
// 1-33328 BK 9/25/09 run conversion report - Codes/Values Report
void CCMSCTReportsMenu::RunReportsConversionCodesAdded()
{
	MNUDATA::const_iterator iter = m_mapConversionCodesAddedXML.find(0);
	ASSERT(iter != m_mapConversionCodesAddedXML.end());

	m_stuItem = iter->second;

	CString *pszReportXML = &m_stuItem.m_szReportXML;
	CString szIntReport = CGBLPrint::MakeConvIntReport(m_stuItem.m_eReportType, m_stuItem.m_szCompanyCode16
		, m_stuItem.m_szCompanyName16
		, m_stuItem.m_szDateTime);
	CString *pszIntReport = &szIntReport;
	// 1-33612 BK 10/21/09 disable print parameters checkbox
	CGBLPrint::PrintReport(pszReportXML, pszIntReport, eREPORTS, false);
}
//------------------------------------------------------------------------------
// 1-33328 BK 9/25/09 run conversion report - Conversion Integration Status Report
void CCMSCTReportsMenu::RunReportsConversionIntegrationStatus()
{
	MNUDATA::const_iterator iter = m_mapConversionIntegrationXML.find(0);
	ASSERT(iter != m_mapConversionIntegrationXML.end());

	m_stuItem = iter->second;

	CString *pszReportXML = &m_stuItem.m_szReportXML;
	CString szIntReport = CGBLPrint::MakeConvIntReport(m_stuItem.m_eReportType, m_stuItem.m_szCompanyCode16
		, m_stuItem.m_szCompanyName16
		, m_stuItem.m_szDateTime);
	CString *pszIntReport = &szIntReport;
	// 1-33612 BK 10/21/09 disable print parameters checkbox
	CGBLPrint::PrintReport(pszReportXML, pszIntReport, eREPORTS, false);
}

//------------------------------------------------------------------------------
void CCMSCTReportsMenu::SetMainFrame(CMainFrame& clsFrame)
{
	if (!m_pclsFrame)
		m_pclsFrame = &clsFrame;
}

//------------------------------------------------------------------------------
//this method will clean the reports off of the menu and empty the report data maps.
//when the user tries to select the menu, the command handler will then reload the menu and the report data maps.
void CCMSCTReportsMenu::RefreshMenu(void)
{
	m_bEnableIntegrationMenu = false;
	m_bEnableConversionMenu = false;
	m_bIntegrationErrorReportExists = false;
	m_bPreConversionReportExists = false;
	m_bCodeValuesAddedReportExists = false;
	m_bConversionIntegReportExists = false;

	RemoveReportsIntegrationChecklistMenu();
}
//------------------------------------------------------------------------------
void CCMSCTReportsMenu::MakeQueryCommand(CXML& clsXML)
{
	// xml Command
	CXMLParams clsCommand;
	CXMLParams clsRoot;
	clsCommand.MakeXMLCommand(&clsCommand, _T("SPECIALPROCESS"), CMS::GetSI()->ApplicationSettings.ModuleStringID, _T("GETCONVERSIONREPORTS"));

	clsXML.m_szXMLCommand = clsCommand.GetXML();
	clsRoot.AppendXMLConnection(&clsRoot, CXMLParams::CN_COMPANY);

	CXMLParams clsCompany(true);
	clsXML.MakeCOMPANYINFO();
	clsCompany.SetXML(clsXML.m_szXMLFormat);
	clsRoot.AppendXMLParam(&clsCompany);
	clsXML.m_szXMLData = clsRoot.GetXML();
}

//------------------------------------------------------------------------------
void CCMSCTReportsMenu::MakeDeleteCommand(CXML& clsXML, int nTypeReportBeingDeleted)
{
	CString szXML(EMPTY_STRING);

	//wrap the column information with the required <TABLE> and <ROOT> tags
	szXML = CXML::MakeTableElement(szXML);
	szXML = CXML::MakeRootElement(szXML);

	//push the column information into a CXMLParams instance and attach
	//the required company information and server login information to it
	CXMLParams clsRoot(true);
	clsRoot.SetXML(szXML);
	CXMLParams::AppendCompanyInfo(&clsRoot);
	CXMLParams::AppendXMLConnection(&clsRoot, CXMLParams::CN_COMPANY);

	// Make command XML.
	CXMLParams clsXMLCommand;
	switch (nTypeReportBeingDeleted)
	{
		// delete conversion reports (bytReportType IN (0,2,3))
	case 0:
		CXMLParams::MakeXMLCommand(&clsXMLCommand, _T("SPECIALPROCESS"), _T("CT"), _T("DELETECONVERSIONREPORT"));
		break;
		// delete integration reports (bytReportType = 1)
	case 1:
		CXMLParams::MakeXMLCommand(&clsXMLCommand, _T("SPECIALPROCESS"), _T("CT"), _T("DELETEINTEGRATIONREPORTS"));
		break;
		// delete conversion and integration reports older than 30 days (bytReportType < 4)
	case 2:
		CXMLParams::MakeXMLCommand(&clsXMLCommand, _T("SPECIALPROCESS"), _T("CT"), _T("DELETEREPORTS_PURGE30DAYS"));
		break;
	};

	clsXML.m_szXMLCommand = clsXMLCommand.GetXML();
	clsXML.m_szXMLData = clsRoot.GetXML();
}

//------------------------------------------------------------------------------
CMenu * CCMSCTReportsMenu::GetIntegrationMainMenu(void)
{
	ASSERT_VALID(m_pclsFrame);	//if assert fails, check to see if SetFrame has been called

	//locate the submenu to which we will be adding report menu items
	CMenu * const pTopMenu = m_pclsFrame->GetMenu();
	ASSERT_VALID(pTopMenu);

	//CMenu * const pReportsMenu = pTopMenu->GetMenuItemID(IDS_CTRL_INTEGRATION);
	CMenu * const pReportsMenu = pTopMenu->GetSubMenu(0);
	ASSERT_VALID(pReportsMenu);

	return pReportsMenu;
}
//------------------------------------------------------------------------------
CMenu * CCMSCTReportsMenu::GetReportsIntegrationChecklistMenu(void)
{
	ASSERT_VALID(m_pclsFrame);	//if assert fails, check to see if SetFrame has been called

	//locate the submenu to which we will be adding report menu items
	CMenu * const pTopMenu = m_pclsFrame->GetMenu();
	ASSERT_VALID(pTopMenu);

	CMenu * const pReportsMenu = pTopMenu->GetSubMenu(3);
	ASSERT_VALID(pReportsMenu);

	// 1-33328 BK 9/25/09 changed report menus
	CMenu * const pIntegrationMenu = pReportsMenu->GetSubMenu(0);
	ASSERT_VALID(pIntegrationMenu);

	CMenu * const pChecklistMenu = pIntegrationMenu->GetSubMenu(0);
	ASSERT_VALID(pChecklistMenu);

	return pChecklistMenu;
}

//------------------------------------------------------------------------------
CMenu * CCMSCTReportsMenu::GetReportsConversionCodesAddedMenu(void)
{
	ASSERT_VALID(m_pclsFrame);	//if assert fails, check to see if SetFrame has been called

	//locate the submenu to which we will be adding report menu items
	CMenu * const pTopMenu = m_pclsFrame->GetMenu();
	ASSERT_VALID(pTopMenu);

	CMenu * const pReportsMenu = pTopMenu->GetSubMenu(3);
	ASSERT_VALID(pReportsMenu);

	CMenu * const pConversionMenu = pReportsMenu->GetSubMenu(1);
	ASSERT_VALID(pConversionMenu);

	CMenu * const pCodesMenu = pConversionMenu->GetSubMenu(0);
	ASSERT_VALID(pCodesMenu);

	return pCodesMenu;
}
//------------------------------------------------------------------------------
const CCMSCTReportsMenu::MNUIDS& CCMSCTReportsMenu::GetReportsIntegrationChecklistMenuIDs(void)
{
	if (m_vecReportsIntegrationChecklistIDs.empty())
	{
		m_vecReportsIntegrationChecklistIDs.reserve(eINTEGRATION_CHKLST_END);
		m_vecReportsIntegrationChecklistIDs.push_back(IDS_CTRL_INTEGRATION_CHECKLIST_REPORT_DATE1);
		m_vecReportsIntegrationChecklistIDs.push_back(IDS_CTRL_INTEGRATION_CHECKLIST_REPORT_DATE2);
		m_vecReportsIntegrationChecklistIDs.push_back(IDS_CTRL_INTEGRATION_CHECKLIST_REPORT_DATE3);
		m_vecReportsIntegrationChecklistIDs.push_back(IDS_CTRL_INTEGRATION_CHECKLIST_REPORT_DATE4);
		m_vecReportsIntegrationChecklistIDs.push_back(IDS_CTRL_INTEGRATION_CHECKLIST_REPORT_DATE5);
	}

	return m_vecReportsIntegrationChecklistIDs;
}

//------------------------------------------------------------------------------
// PreConversion Data Checklist Report
// 1-33328 BK 9/24/09 these reports are now single reports only
const CCMSCTReportsMenu::MNUIDS& CCMSCTReportsMenu::GetReportsConversionChecklistMenuIDs(void)
{
	if (m_vecReportsConversionChecklistIDs.empty())
	{
		m_vecReportsConversionChecklistIDs.reserve(1);
		m_vecReportsConversionChecklistIDs.push_back(IDS_CTRL_CONVERSION_CHECKLIST_REPORT_DATE1);
	}
	return m_vecReportsConversionChecklistIDs;
}
//------------------------------------------------------------------------------
// Codes/Values Added Report
const CCMSCTReportsMenu::MNUIDS& CCMSCTReportsMenu::GetReportsConversionCodesAddedMenuIDs(void)
{
	if (m_vecReportsConversionCodesAddedIDs.empty())
	{
		m_vecReportsConversionCodesAddedIDs.reserve(1);
		m_vecReportsConversionCodesAddedIDs.push_back(IDS_CTRL_CONVERSION_CODES_ADDED_REPORT_DATE1);
	}
	return m_vecReportsConversionCodesAddedIDs;
}
//------------------------------------------------------------------------------
// Conversion Integration Status Report
const CCMSCTReportsMenu::MNUIDS& CCMSCTReportsMenu::GetReportsConversionIntegrationMenuIDs(void)
{
	if (m_vecReportsConversionIntegrationIDs.empty())
	{
		m_vecReportsConversionIntegrationIDs.reserve(1);
		m_vecReportsConversionIntegrationIDs.push_back(IDS_CTRL_CONVERSION_CHECKLIST_REPORT_DATE1);
	}
	return m_vecReportsConversionIntegrationIDs;
}
//------------------------------------------------------------------------------
const CCMSCTReportsMenu::CMenuItemData& CCMSCTReportsMenu::CMenuItemData::operator= (const CCMSCTReportsMenu::CMenuItemData& inRHS)
{
	if (this != &inRHS)
	{
		m_eReportType = inRHS.m_eReportType;
		m_szCompanyCode16 = inRHS.m_szCompanyCode16;
		m_szCompanyName16 = inRHS.m_szCompanyName16;
		m_szReportXML = inRHS.m_szReportXML;
		m_szDateTime = inRHS.m_szDateTime;
	}

	return *this;
}

//------------------------------------------------------------------------------
const CCMSCTReportsMenu::CMenuItemData& CCMSCTReportsMenu::CMenuItemData::operator= (CDataRow& objDR)
{
	m_eReportType = (CGBLPrint::enmConvIntReportTypes)objDR.XMLByte(_T("bytReportType"));
	// EK - 05/05/04 - Check if the values are NULL
	if (objDR.ColumnExists(_T("strCompanyCode")))
		m_szCompanyCode16 = objDR.XMLStr(_T("strCompanyCode"));
	else
		m_szCompanyCode16.Empty();
	if (objDR.ColumnExists(_T("strCompanyName")))
		m_szCompanyName16 = objDR.XMLStr(_T("strCompanyName"));
	else
		m_szCompanyCode16.Empty();
	m_szReportXML = objDR.XMLStr(_T("txtReportXML"));
	m_szDateTime = objDR.XMLStr(_T("dteTimestamp"));

	return *this;
}

//------------------------------------------------------------------------------
BOOL CCMSCTReportsMenu::CMenuItemData::IsEmpty(void) const
{
	return (m_szCompanyCode16.IsEmpty() && m_szCompanyName16.IsEmpty() && m_szReportXML.IsEmpty() && m_szDateTime.IsEmpty());
}
//------------------------------------------------------------------------------