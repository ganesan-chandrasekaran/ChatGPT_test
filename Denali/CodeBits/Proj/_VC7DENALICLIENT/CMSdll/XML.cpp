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

 //KPM, 10/16/2003, remove for new way of calling COM libraries
 // #import "..\Build\Release\SXMLPARS.tlb" rename_namespace("SXMLPARS")
 //using namespace SXMLPARS;

#include "XML.h"
// #include "..\cmsinclude\ControlsMap.h"
// #include "..\cmsdll\ErrorViewer.h"
// #include "..\GBLForms\cmsgrid.h"
#include "..\cmsdll\ICMSGrid.h"
#include "..\cmsdll\ThreadSendXML.h"
#include "..\cmsdll\BusyProcessing.h"
#include "..\cmsdll\FileLock.h"
#include "..\cmsdll\CMSListCtrl.h"
#include "..\cmsdll\CMSControlData.h"
#include "..\cmsdll\DataSet.h"
#include "..\cmsdll\GBLEnvironment.h"
#include "..\Common\LangIds.h"
#include "CMSUtility.h"
#include "CMSDBObject.h"
//Global Variables KPM, 10/23/2002

#include "XMLViewer.h"
#include "CMSGrouper.h"
#include "CMSXMLFieldValue.h"
#include "CMSXMLClearField.h"
#include "CMSMessageXML.h"

#include "..\cmsdll\ServerSide.h"
#include "..\cmsdll\CMSDataAdapter.h"
#include "..\cmsdll\CMSDataBridge.h"
#include "..\cmsdll\CMSDataBridgeTrigger.h"

//extern CGBLSystemInformation* g_pGBLSystemInformationCMSDll; on account control!@%@!%@#$@
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const CString szLT(_T("<"));
static const CString szGT(_T(">"));
static const CString szSlash(_T("/"));

/////////////////////////////////////////////////////////////////////////////
// CXML

IMPLEMENT_DYNCREATE(CXML, CWnd)

CXML::CXML()
	: m_bUsePaddedCtrl(false) // DTG, 12.15.2003
	, m_objPublisher()
	, m_hForm(NULL)
	, m_IContainerReset(NULL)
	, bMakeServerCallLowerCaseToMatchNewCode(false)
	, m_bSaveSpecial(false) // InTimeTec: 05/20/2013 : HS : Attach Files to Master Records
{
	CoInitialize(NULL);
	m_bShowXmlViewer = false;		//default setting for Viewer
	if (m_docPtr != NULL)
	{								//check for xml doc
		m_docPtr.Release();			//release doc, to create new doc
		m_docPtr = NULL;
	}
	//m_docPtr.CreateInstance("msxml2.domdocument");		// create new doc for XML
	CCMSXMLDomDoc::GetPolicy().CreateObject(m_docPtr, CCMSXMLDomDoc::GetProgID());

	m_pNumEdit = NULL;		// numeric edit
	m_pCharEdit = NULL;		// character edit
	m_pCurEdit = NULL;		// currency edit
	m_bStart = false;	// bool for report elements AES
	m_szXMLReport = _T("");
	m_bNoNegativeNumber = false;	// use neg values
	m_intTableCount = 0;
	m_bUseDefinedServer = false;	// set so that is will use the sever pass in to make the connection nodes, if you want to change server do it at the form level. AES
	m_bServerError = false;
	m_bTableEnd = false;
	m_bGridOnly = false;
	m_bMultipleDetails = false;	//use so you can have multiple controls to fill with from detail lines. AES 6.04.04

	m_bNoRecordLock = false;	// Used to turn on and off record locking
	m_szGUID = _T("");
	m_szXMLFormat = _T("");
	m_szXMLReport = _T("");
	m_szXMLCommand = _T("");
	m_szXMLData = _T("");
	m_szFormTitle = _T("");
	m_szUser = _T("");
	m_szNumber = _T("");
	m_szText = _T("");
	m_szReturn = _T("");
	m_szXMLUserInfo = _T("");
	m_szXMLSendData = EMPTY_STRING;		// 1-32267 BK 3/7/09 created new variable to hold data to send to server when trying to re-lock record

	m_IDataCallBackProvider = NULL;
	m_IDataHookProvider = NULL;
	m_bDontClearAllControl = false;//AES
	// InTimeTec: 08/26/2013 : HS :Bug 8417 : D_AR - Credit Card Codes form does not clear to blank form when deleting an existing record - goes to the next record
	m_bClearAllControls = false;
	// InTimeTec: 11/21/2013: HS : Defect 9074 : D_IN_Save IN Stock Information from the Stock Item Maintenance Screen When Creating New Item
	m_bKeepAfterSave = false;
	// InTimeTec: End
}

//---------------------------------------------------------------------------
CXML::~CXML()
{
	if (m_docPtr != NULL)
	{								// check for object to release
		m_docPtr.Release();			// release doc when class is destroyed
		m_docPtr = NULL;
	}
	//delete pointer to the cms edit class
	delete m_pNumEdit;
	delete m_pCharEdit;
	delete m_pCurEdit;
}

BEGIN_MESSAGE_MAP(CXML, CWnd)
	//{{AFX_MSG_MAP(CXML)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXML message handlers
//
void CXML::ClearReturnValue()
{
	m_szReturn = _T("<ROOT/>");
}

//****************************************************************
//MakeNode - This method is use to make a XML string AES
//	szSingle - CString,	XML node element use for single, first, and last node.
//	szInner - CString,	XML inner node, Data or parameter going to the server
//	nSpace - int,		XML spaces use to format the XML in Developer mode, makes XML
//						easier to read.
//	nType - int,		Type of node
//						XML_STARTELEMENT beginning or single node,
//						XML_ENDELEMENT ending node
//						XML_ATTRIBUTE a node the constants both a single and inner node, and
//						XML_SHORTNODE use so you do not have to enter start and end nodes
//****************************************************************
void CXML::MakeNode(
	CString szSingle,
	CString szInner,
	int nSpace,
	int nType,
	bool bPreserveSpace)
{
	CString szSpaces;
	CString szValue = "";
	//MSXML2::IXMLDOMNodePtr pNode;
	try
	{
		szInner.TrimLeft();
		CString szReturn = "\n";
		CString szBackSlash;
		if (szSingle != "NEWTABLE")
		{
			if (szSingle == "SET_DATA_CHANGE")
				m_szXMLFormat = m_szXMLFormat + szSpaces + szLT + szInner + " " + szSlash + szGT + szReturn;
			else if (szSingle == "SET_START_CHANGE")
				m_szXMLFormat = m_szXMLFormat + szSpaces + szLT + szInner + szGT + szReturn;
			else if (szSingle == "SET_END_CHANGE")
			{
				m_szXMLFormat = m_szXMLFormat + szSpaces + szLT + szSlash + szInner + szGT + szReturn;
				if (m_intTableCount > 0)							//check table count
					szValue.Format(_T("%d"), m_intTableCount);	//set to table number
				if (szInner == "TABLE" + szValue)
					m_bTableEnd = true;
			}
			else
			{
				if (!g_pGBLSystemInformationCMSDll->ApplicationSettings.DeveloperMode)
				{											//look to see if in developer mode to format the Xml to read AES
					nSpace = 0;
					szReturn = "";
				}

				szSpaces = "";
				switch (nSpace)
				{											//add spaces to XML AES
				case 4: szSpaces += "  ";
				case 3: szSpaces += "  ";
				case 2: szSpaces += "  ";
				case 1: szSpaces += "  ";
				}

				switch (nType)
				{
				case XML_STARTELEMENT:						//Starting node. <node> szValue
					m_szXMLFormat = m_szXMLFormat + szSpaces + szLT + szSingle + szGT + szReturn;
					break;
				case XML_ENDELEMENT:						//ending node	</node>
					m_szXMLFormat = m_szXMLFormat + szSpaces + szLT + szSlash + szSingle + szGT + szReturn;
					break;
				case XML_ATTRIBUTE:							//node with start, inner and ending node
					m_szXMLFormat = m_szXMLFormat + szSpaces + (LPCTSTR)GetXMLNodeFormat(szSingle, szInner, bPreserveSpace) + szReturn;
					break;
				case XML_SHORTNODE:							//short node <node/> both start and ending
					m_szXMLFormat = m_szXMLFormat + szSpaces + szLT + szSingle + " " + szSlash + szGT + szReturn;
					break;
				case XML_REPORTELEMENT:						//report nodes for report parameters to netdll AES
					if (!m_bStart) m_szXMLReport = "<ROOT>" + szReturn + "<REPORT>" + szReturn;	//add report node, this will be used in the NETDLL AES
					if (szSingle.Find('#') > 0)
					{										//looking for server parameters
						szSingle.Remove('#');				//remove the server ID char AES
						if (szSingle.Find('@') == 0)
						{									//looking if the parameters is a report parameter to AES
							szSingle.Remove('@');			//remove the report ID char, server does not need it AES
							m_szXMLReport = m_szXMLReport + szSpaces + (LPCTSTR)GetXMLNodeFormat(szSingle, szInner, bPreserveSpace) + szReturn;
							m_szXMLFormat = m_szXMLFormat + szSpaces + (LPCTSTR)GetXMLNodeFormat(szSingle, szInner, bPreserveSpace) + szReturn;
						}
						else								//just a sever parameter AES
							m_szXMLFormat = m_szXMLFormat + szSpaces + (LPCTSTR)GetXMLNodeFormat(szSingle, szInner, bPreserveSpace) + szReturn;
					}
					else
					{										//just a report parameter AES
						szSingle.Remove('@');				//remove the report ID char, server does not need it AES
						m_szXMLReport = m_szXMLReport + szSpaces + (LPCTSTR)GetXMLNodeFormat(szSingle, szInner, bPreserveSpace) + szReturn;
					}
					m_bStart = true;						//bool for report elements AES
					break;
				}
			}
		}
		else
		{
			if (m_intTableCount > 0)							//check table count
				szValue.Format(_T("%d"), m_intTableCount);	//set to table number
			if (!m_bTableEnd)
				m_szXMLFormat = m_szXMLFormat + szSpaces + "</" + "TABLE" + szValue + szGT + szReturn;	//end table node
			else
				m_bTableEnd = false;
			if (szInner == "NEWTABLE")
				m_intTableCount++;							//next table number
			szValue.Format(_T("%d"), m_intTableCount);		//format to string
			m_szXMLFormat = m_szXMLFormat + szSpaces + szLT + "TABLE" + szValue + szGT + szReturn;	//set next table start
		}
	}
	catch (_com_error e)
	{
		stuErrorReturn clsErrorReturn;
		CGBLError clsError(clsErrorReturn);
		clsError.HandleException(&e);
	}
}

//****************************************************************
//GetXMLNodeFormat - Used to format the XML nodes so that they can contain
//					char(``!@#$%^&*()_+-=[]\;',./?><":|}{)
//		CString szSingle,   node name
//		CString szInner     inner node or value you are sending
//****************************************************************
_bstr_t CXML::GetXMLNodeFormat(
	CString szSingle,
	CString szInner,
	bool bPreserveSpace)
{
	CCMSXMLDomNode::ComponentTypePtr pNode;
	_bstr_t bstrReturn;
	try
	{
		//Erick Korsten - 05/06/03 - Defect ID 1-4516 - Allow value to have spaces if requested
		if (bPreserveSpace)
		{   // Keep spaces in node value CCMSXMLDomElement
			CCMSXMLDomElement::ComponentTypePtr  pElementNode;
			pElementNode = m_docPtr->createElement(_bstr_t(szSingle));
			pElementNode->text = _bstr_t(szInner);
			// Set element attribute xml:space so that space will not be ignored
			pElementNode->setAttribute(_bstr_t("xml:space"), _bstr_t("preserve"));
			bstrReturn = pElementNode->Getxml();
		}
		else
		{   // Ignore spaces in node value
			pNode = m_docPtr->createNode("element", _bstr_t(szSingle), "");		//add node name
			pNode->text = _bstr_t(szInner);								//add node inner value
			bstrReturn = pNode->Getxml();
		}
	}
	catch (_com_error e)
	{
		stuErrorReturn clsErrorReturn;
		CGBLError clsError(clsErrorReturn);
		clsError.HandleException(&e);
	}
	return bstrReturn;                                             //return formatted XML
}

//****************************************************************
//GetXMLFormat - This method is use to format XML with ROOT another it AES
//****************************************************************
void CXML::GetXMLFormat()
{
	m_szXMLFormat = m_szXMLFormat;								//ADD report server parameters AES
	m_szXMLFormat = "<ROOT>\n" + m_szXMLFormat + "</ROOT>";		//add ROOT around the XML

	m_szXMLData = m_szXMLFormat;								//assign the XML to variable the is use when all formatting is complete. AES
}

//****************************************************************
//MakeXMLCommand - This command method is use for all xml command but record locking AES
//	szCommand - CString,	Command object of the XML, GET, UPDATE,
//	szModule - CString,		Current Module that the XML is called form GL, AP , AR
//	szObjTable - CString,	Object that the XML is getting or update, usually is the data table in SQL AES
//****************************************************************/
void CXML::MakeXMLCommand(
	CString szCommand,
	CString szModule,
	CString szObjTable)
{
	m_szXMLFormat = "";
	MakeNode("COMMAND", szCommand, 1, XML_ATTRIBUTE);	//add command object
	MakeNode("MODULE", szModule, 1, XML_ATTRIBUTE);	//add module
	MakeNode("OBJECT", szObjTable, 1, XML_ATTRIBUTE);	//add object

	m_szXMLCommand = "<ROOT>" + m_szXMLFormat + "</ROOT>";	//assign to the variable that is used
}

//****************************************************************
//MakeXMLCommand - This Command method is use for record locking AES
//	szCommand - CString,		Command object of the XML, GET, UPDATE,
//	szCommandType - CString,	Command Type, type of record lock, locking or unlocking
//	szModule - CString,			Current Module that the XML is called form GL, AP , AR
//	szObjTable - CString,		Object that the XML is getting or update, usually is the data table in SQL AES
//****************************************************************
void CXML::MakeXMLCommand(
	CString szCommand,
	CString szCommandType,
	CString szModule,
	CString szObjTable)
{
	m_szXMLFormat = "";
	MakeXMLCommand(szCommand, szModule, szObjTable);
	MakeNode("LOCKTYPE", szCommandType, 1, XML_ATTRIBUTE);		//add lock record type
	m_szXMLCommand = "<ROOT>\n" + m_szXMLFormat + "</ROOT>\n";	//assign to the variable that is used
}

//---------------------------------------------------------------------------
void CXML::MakeXMLConnection(UINT uinConnectionType, bool bSchema)
{
	MakeXMLConnection("", "", "", uinConnectionType, bSchema);
	m_szXMLData = "<ROOT>\n" + m_szXMLFormat + "</ROOT>\n";
}

//****************************************************************
//MakeXMLConnection - This Command method is use connection nodes in the data XML AES
//	szType - CString,				DB type
//	szTimeout - CString,			time of DataBase
//	szPoolTime - CString,			Pool time to set the ATL  object
//	uinConnectionType - CString,	Connection type
//****************************************************************
void CXML::MakeXMLConnection(
	CString szType,
	CString szTimeout,
	CString szPoolTime,
	UINT uinConnectionType,
	bool bSchema,
	bool bMaintForm/* = false*/)
{
	//KPM, 11/16/02 Changed these to use the globals and to select the correct item based on the connection type
	CGBLSystemInformation::stuServerInformation* stuServer = NULL;

	//AES 4/11/2003 Added so that the server could be overwritten on a maint form at the form level.
	if (m_bUseDefinedServer) uinConnectionType = m_uinDefinedServer;

	switch (uinConnectionType)
	{
	case CTY_PRIMARYSERVER:
		stuServer = &g_pGBLSystemInformationCMSDll->Servers.PrimaryServer;
		break;
	case CTY_COMPANYSERVER:
		stuServer = &g_pGBLSystemInformationCMSDll->Servers.CompanyServer;
		break;
	case CTY_ARCHIVESERVER:
		stuServer = &g_pGBLSystemInformationCMSDll->Servers.ArchiveServer;
		break;
	}

	m_szXMLFormat = "";		//clear variable
	//KPM, 11/16/02 Commented some items out because they are unnecessary
	MakeNode("CONNECTION", "", 1, XML_STARTELEMENT);
	// 1-30178 BK 6/17/08 added bMakeServerCallLowerCaseToMatchNewCode variable to be able to set CONNECTION commands in regular case instead of all upper case to be consistent with new code
	// see CCFV_INRecalcInventoryFiles::OnFormOk()
	if (bMakeServerCallLowerCaseToMatchNewCode)
	{
		MakeNode("GetSchema", "", 1, XML_SHORTNODE);
		MakeNode("DBType", stuServer->DBType, 2, XML_ATTRIBUTE);							// Make DB type node
		MakeNode("Server", stuServer->Name, 2, XML_ATTRIBUTE);								// Make server name node
		MakeNode("Database", stuServer->Database, 2, XML_ATTRIBUTE);							// Make database name node
		MakeNode("User", stuServer->User, 2, XML_ATTRIBUTE);								// Make user id node
		if (stuServer->Password != "") MakeNode("Password", stuServer->Password, 2, XML_ATTRIBUTE);	// Make password node
		//MakeNode("POOLTIME", szPoolTime, 2, XML_ATTRIBUTE);
		CString szTimeOut;
		szTimeOut.Format(_T("%d"), stuServer->Timeout);
		MakeNode("Timeout", szTimeOut, 2, XML_ATTRIBUTE);
		//PTR.06.28.2005 - PK errors in maintenance forms are handled differently.
		if (bMaintForm)
			MakeNode("MaintForm", "", 1, XML_SHORTNODE);

	}
	else
	{
		//KPM.2005.06.08 Removed because we now need the schema for
		//Peter's xml changes.  The IsNull function must have schema
		//to determine if something actually is null.
		//if(bSchema)
		MakeNode("GETSCHEMA", "", 1, XML_SHORTNODE);
		MakeNode("DBTYPE", stuServer->DBType, 2, XML_ATTRIBUTE);							// Make DB type node
		MakeNode("SERVER", stuServer->Name, 2, XML_ATTRIBUTE);								// Make server name node
		MakeNode("CATALOG", stuServer->Database, 2, XML_ATTRIBUTE);							// Make database name node
		MakeNode("USER", stuServer->User, 2, XML_ATTRIBUTE);								// Make user id node
		if (stuServer->Password != "") MakeNode("PASSWORD", stuServer->Password, 2, XML_ATTRIBUTE);	// Make password node
		//MakeNode("POOLTIME", szPoolTime, 2, XML_ATTRIBUTE);
		CString szTimeOut;
		szTimeOut.Format(_T("%d"), stuServer->Timeout);
		MakeNode("TIMEOUT", szTimeOut, 2, XML_ATTRIBUTE);
		//PTR.06.28.2005 - PK errors in maintenance forms are handled differently.
		if (bMaintForm)
			MakeNode("MAINTFORM", "", 1, XML_SHORTNODE);
	}
	MakeNode("CONNECTION", "", 1, XML_ENDELEMENT);										// Make end connection node
}

UINT CXML::GetServerType(CXMLParams::enmConnectionType p_nServer)
{
	switch (p_nServer)
	{ // begbert 02-15-2006 VS8: warning C4482
	case CXMLParams::CN_PRIMARY:
		return CTY_PRIMARYSERVER;
		break;
	case CXMLParams::CN_COMPANY:
		return CTY_COMPANYSERVER;
		break;
	case CXMLParams::CN_ARCHIVE:
		return CTY_ARCHIVESERVER;
		break;
	}
	return CTY_COMPANYSERVER;
}

CXMLParams::enmConnectionType CXML::GetServerType(UINT p_uinConnectionType)
{
	switch (p_uinConnectionType)
	{ // begbert 02-15-2006 VS8: warning C4482
	case CTY_PRIMARYSERVER:
		return CXMLParams::CN_PRIMARY;
		break;
	case CTY_COMPANYSERVER:
		return CXMLParams::CN_COMPANY;
		break;
	case CTY_ARCHIVESERVER:
		return CXMLParams::CN_ARCHIVE;
		break;
	}
	return CXMLParams::CN_COMPANY;
}

//****************************************************************
//GetReportXMLParameters - This Command method is use return XML string for Reports AES
//	szModule - CString,		The 2 letter module code
//	szReportName - CString,		The name of the report for server object
//	*strCommandValues -		CStringArray, XML command set Array
//  uinConnectionType       The database (PRIMARY, COMPANY, etc.) to connect to. Default = COMPANY
//
//****************************************************************
bool CXML::SetReportXMLParameters(
	CString szModule,
	CString szReportName,
	CStringArray* strCommandValues,
	UINT uinConnectionType)
{
	CString		szServer;
	_bstr_t		szReturn;
	int			nStringCount;
	m_szXMLReport = _T("");
	nStringCount = szReportName.GetLength();	//remove the .rpt from object code
	if (szReportName.Find(_T(".")) > 0)
	{
		nStringCount = nStringCount - 4;
		szReportName = szReportName.Left(nStringCount);
	}
	//Make the Command string
	MakeXMLCommand("REPORT", szModule, szReportName);
	//Make the Data string
	MakeXMLConnection("Connection", "10", "300", uinConnectionType, true);

	MakeCOMPANYINFO();

	for (int ii = 0; ii < strCommandValues->GetSize(); ii += 2)		// get nodes from string array
		MakeNode(strCommandValues->GetAt(ii), strCommandValues->GetAt(ii + 1), 2, XML_REPORTELEMENT);
	m_szXMLReport = m_szXMLReport + "</REPORT>\n</ROOT>\n";			// report ending node AES
	GetXMLFormat();
	szReturn = m_szXMLCommand + m_szXMLData;						// set return value with XML

	m_bStart = false;							// bool for report elements AES
	return true;
}

bool	CXML::SetReportXMLParameters(
	const CString& szModule,
	const CString& szReportName,
	const CString& szDMReportName,
	const CStringArray& strCommandValues,
	UINT uinConnectionType)
{
	CString		szServer;
	_bstr_t		szReturn;

	CString		szReport(szReportName);

	m_szXMLReport.Empty();

	int	 nStringCount = szReport.GetLength();	//remove the .rpt from object code
	if (szReport.Find(_T(".")) > 0)
	{
		nStringCount = nStringCount - 4;
		szReport = szReport.Left(nStringCount);
	}

	//Make the Command string
	MakeXMLCommand(CMSStrings::XMLTags::REPORT, szModule, szReport);

	//Make the Data string
	MakeXMLConnection(CMSStrings::XMLTags::Connection, _T("10"), _T("300"), uinConnectionType, true);

	MakeCOMPANYINFO();

	for (int ii = 0; ii < strCommandValues.GetSize(); ii += 2)		// get nodes from string array
		MakeNode(strCommandValues.GetAt(ii), strCommandValues.GetAt(ii + 1), 2, XML_REPORTELEMENT);

	m_szXMLReport = m_szXMLReport + _T("</REPORT>\n</ROOT>\n");			// report ending node AES

	GetXMLFormat();
	szReturn = m_szXMLCommand + m_szXMLData;						// set return value with XML

	m_bStart = false;							// bool for report elements AES

	return true;
}

//****************************************************************
//SendXML - This Command method is use send the XML to the server AES
//	bShowErrorDialog - CString,
//	uinSendXMLType - UINT,
//****************************************************************
_bstr_t CXML::SendXML(
	bool bShowErrorDialog,
	UINT uinSendXMLType,
	bool bShowViewer,
	stuErrorReturn* pErrorReturn,
	CString szServerCallDisplay)
{
	//KPM, 4/30/2003, initialize error found flag to false
	m_bServerError = false;

	CServerSideInput clsInput;
	clsInput.m_bShowErrorDialog = bShowErrorDialog;
	clsInput.m_bShowViewer = bShowViewer;
	clsInput.m_szServerCallDisplay = szServerCallDisplay;
	clsInput.m_uiSendXMLType = uinSendXMLType;
	UINT uiRC = CServerSide::DoXMLCommandP(m_szXMLCommand, m_szXMLData, m_szReturn, clsInput, this);
	m_bServerError = (uiRC != rcSuccess);
	m_clsErrorReturn = clsInput.m_clsErrorReturn;
	if (pErrorReturn)
		*pErrorReturn = clsInput.m_clsErrorReturn;

	m_szXMLUserInfo = (m_szReturn.GetBSTR());				//for locking
	return m_szReturn;
}

// ********************************************************
// * Procedure:    SendXML (Overload #2)
// *
// * Description:  Executes the SendXML with the error return structure being the
// * first parameter.
// *
// * Parameters:   See Overload #1
// *
// * Returns:      See Overload #1
// *
// * Date Created: 12/30/2003 by Peter Ringering
// ********************************************************
_bstr_t CXML::SendXML(
	stuErrorReturn* pErrorReturn,
	bool bShowErrorDialog,
	UINT uinSendXMLType,
	bool bShowViewer,
	CString szServerCallDisplay)
{
	return SendXML(bShowErrorDialog, uinSendXMLType, bShowViewer, pErrorReturn, szServerCallDisplay);
}

//---------------------------------------------------------------------------
//Ensure changes in below method default values, be reflected in 4th overload.
_bstr_t CXML::SendXML(
	CString szCommand,
	CString szData				/* = ""*/,
	bool bShowErrorDialog		/* = true*/,
	UINT uinSendXMLType			/* = SXM_SAMEPROCESS*/,
	bool bShowViewer			/* = true*/,
	stuErrorReturn* pErrorReturn/* = NULL*/,
	CString szServerCallDisplay)
{
	this->m_szXMLCommand = szCommand;
	this->m_szXMLData = szData;
	return SendXML(bShowErrorDialog, uinSendXMLType, bShowViewer, pErrorReturn, szServerCallDisplay);
}

_bstr_t CXML::SendXML(const CString& szCommand, const CString& szData, stuErrorReturn& clsErrorReturn)
{
	return SendXML(szCommand, szData, true, SXM_SAMEPROCESS, true, &clsErrorReturn);
}

//****************************************************************
//GetXMLData - This function is used to get the data for the form,
//			all data select should got through this function. AES
//	nMode - int,			The current form mode.  If it is
//							MODE_EDIT then the current record
//							will be deleted.
//	szModule - CString,		The 2 letter module code
//	szForm - CString,		The name of the form the data is for
//	*strCommandValues -		CStringArray, XML command set Array
//	szControlID - CString,	Map ID for control to fill
//	szXMLTable - CString,	XML Table name, so used to ID the grid in the control map
//
// begbert 03-14-2007 : added strExtraReturnFields to allow extra fields to be returned by autofills.
//****************************************************************
bool CXML::GetXMLData(
	int nDirection,
	CString szModule,
	CString szForm,
	CStringArray* strCommandValues,
	CString szControlID,
	CString szXMLTable,
	UINT uinConnectionType,
	bool bHeaderOnly			/*=false*/,
	CStringArray* strExtraReturnFields /*=NULL*/)
{
	CCMSWaitCursor clsWaitCursor;
	//This function is used to get the data for the form, all data select should got through this function. AES
	CoInitialize(NULL);
	CString szItemValue = "";
	CString	szDirection = "";
	CString	szCommand = "";
	m_GottenData.clear();
	szCommand = "GET";
	try
	{
		switch (nDirection)
		{																	//	set the direction for XML
		case RECORD_FIRST:
			szDirection = "FIRST";											//	get first record
			break;
		case RECORD_NEXT:
			szDirection = "NEXT";											//	get next record
			break;
		case RECORD_PREVIOUS:
			szDirection = "PREVIOUS";										//	get previous record
			break;
		case RECORD_CURRENT:
			szDirection = "CURRENT";										//	get current record
			break;
		case RECORD_AUTOFILL:												//	get a single field for Autofill
			szCommand = "AUTOFILL";
			break;
		}

		//Make the Command string
		MakeXMLCommand(szCommand, szModule, szForm);

		MakeXMLConnection("Connection", "10", "300", uinConnectionType);

		MakeCOMPANYINFO();

		CCMSXMLDomNodeList::ComponentTypePtr pNodeList;						//	pointers used in XML  CCMSXMLDomElement
		CCMSXMLDomNode::ComponentTypePtr pNode;
		CCMSXMLDomNode::ComponentTypePtr pInnerNode;

		//Clear fields before fetch.
		if (m_IDataCallBackProvider)
			m_IDataCallBackProvider->ClearFields();

		NotifyClearField();

		if (nDirection == RECORD_AUTOFILL)
		{																	//	GetData for the Autofill.

			MakeNode("DIRECTION", "FIRST", 1, XML_ATTRIBUTE);				//	format the Data xml
			MakeNode("TABLE", "", 1, XML_STARTELEMENT);						//	need a table node
			MakeNode("RECORD", "", 2, XML_STARTELEMENT);					//	need a record node

			for (int ii = 0; ii < strCommandValues->GetSize(); ii += 2)		//	get nodes from string array
				MakeNode(strCommandValues->GetAt(ii), strCommandValues->GetAt(ii + 1), 3, XML_ATTRIBUTE, true);
			MakeNode("RECORD", "", 2, XML_ENDELEMENT);						//	complete the ending node record
			// begbert 03-14-2007 : added strExtraReturnFields to allow extra fields to be returned by autofills.
			if (strExtraReturnFields && !strExtraReturnFields->IsEmpty())
			{
				MakeNode("EXTRARETURNFIELDS", "", 2, XML_STARTELEMENT);
				for (int ii = 0; ii < strExtraReturnFields->GetSize(); ii++)//	get nodes from string array
					MakeNode(strExtraReturnFields->GetAt(ii), "", 3, XML_ATTRIBUTE, true);
				MakeNode("EXTRARETURNFIELDS", "", 2, XML_ENDELEMENT);
			}
			MakeNode("TABLE", "", 1, XML_ENDELEMENT);						//	complete the end node table
			GetXMLFormat();													//	format the xml with root around it

			m_docPtr->loadXML(SendXML());									//	send xml to server

			pNodeList = m_docPtr->getElementsByTagName(CMSStrings::XMLTags::NoData);	//look for no data on return
			pNode = pNodeList->Getitem(0);

			if (pNode == NULL)
			{																//	this means there was data so fill the control with autofill data AES
				FillControls(szControlID, nDirection);						//	fill control
				return true;
			}
			else
				return false;												//	no data return false AES
		}
		else
		{																	//	GetData no-autofill.
																			//	format the Data xml
			MakeNode("DIRECTION", szDirection, 1, XML_ATTRIBUTE);			//	add direction

			//default param passed in so that a query on tables with detail items
			//will return only the header part. PGP(08/09/2004)
			if (bHeaderOnly)
				MakeNode(_T("bolHeaderOnly"), _T(""), 1, XML_SHORTNODE);

			if (szXMLTable != "")											//	used for get data for a single table were a form has multiple data sets
				MakeNode(szXMLTable, "", 1, XML_SHORTNODE);					//	set the short node of the table needed
			MakeNode("TABLE", "", 1, XML_STARTELEMENT);						//	need a table node
			MakeNode("RECORD", "", 2, XML_STARTELEMENT);					//	need a record node

			for (int ii = 0; ii < strCommandValues->GetSize(); ii += 2)		//	get nodes from string array
			{
				szItemValue = strCommandValues->GetAt(ii);					//	remove any unwanted values here
				if ((szItemValue == "SET_START_CHANGE") ||
					(szItemValue == "SET_DATA_CHANGE") ||
					(szItemValue == "SET_END_CHANGE") ||
					(szItemValue == "NEWTABLE"))
				{
					strCommandValues->RemoveAt(ii + 1);
					strCommandValues->RemoveAt(ii);
				}
				else
					MakeNode(strCommandValues->GetAt(ii), strCommandValues->GetAt(ii + 1), 3, XML_ATTRIBUTE);
			}
			MakeNode("RECORD", "", 2, XML_ENDELEMENT);						//	complete the ending node record
			MakeNode("TABLE", "", 1, XML_ENDELEMENT);						//	complete the end node table
			GetXMLFormat();													//	format the xml with root around it

			DWORD dwTick = GetTickCount();									// use as counter
			TRACE("START LOADING");

			m_docPtr->loadXML(SendXML());									//	send xml to server

			TRACE("Done Loading = %d\n", GetTickCount() - dwTick);

			pNodeList = m_docPtr->getElementsByTagName(CMSStrings::XMLTags::Error);	//	look for no data on return
			pNode = pNodeList->Getitem(0);
			if (pNode != NULL)
			{
				m_bServerError = true;
				return false;
			}
			pNodeList = m_docPtr->getElementsByTagName(CMSStrings::XMLTags::NoData);	//	look for no data on return
			pNode = pNodeList->Getitem(0);

			if (pNode == NULL)
			{
				//PGP(11/02/2004) - Call hook only when there is data,
				if (HasDataHookProvider())
					GetDataHookProvider()->HookGetData(*this, szControlID);

				TRACE("START FILLING");										//	is there data to fill controls AES
				FillControls(szControlID, nDirection);						//	this called to fill controls(not grids)
				TRACE("Header done");

				if (LookupElement(szControlID, szXMLTable, m_pMyElement) == TRUE)
				{															//	lookup if there is a grid control to fill AES
					CString szTable = szControlID;							//	Control is were we start
					int nTable = _tstoi(szControlID);						//	convert to number
					if (m_pMyElement.bValXML == true)
					{														//	check for bool to see if there is a header detail record
						do
						{
							nTable += 1;									//	ad on to the table
							szTable.Format(_T("%.1d"), nTable);				//	convert back to string
							bool bResult = false;

							if (m_pMyElement.nType == GRIDTYPE)
								bResult = FillGridWithData(m_pMyElement.pObjVal, szTable);
							else if (m_pMyElement.nType == LIST_TYPE)
								bResult = FillListWithData(m_pMyElement.pObjVal, szTable); // added list PGP
							else
								bResult = true;

							if (!bResult)
							{
								CGBLForm::CMSMessageBox(g_pGBLSystemInformationCMSDll->ApplicationMainFrame, IDS_ERR_FILLING_DATAGRID, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONERROR);
								return false;								//	return false
							}
						} while (LookupElement(szTable, szXMLTable, m_pMyElement));	//	look if there is more then one grid on the form
					}
					else
					{
						do
						{													//	loop through multiple grids
							if (!FillGridWithData(m_pMyElement.pObjVal, szTable))
							{
								CGBLForm::CMSMessageBox(g_pGBLSystemInformationCMSDll->ApplicationMainFrame, IDS_ERR_FILLING_DATAGRID, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONERROR);
								return false;								//	return false
							}
							nTable += 1;									//	add one to the table
							szTable.Format(_T("%.1d"), nTable);				//	convert back to string
						} while (LookupElement(szTable, szXMLTable, m_pMyElement));
					}
				}
				return true;												//	all way good return true
			}
			else
			{
				if (LookupElement(szControlID, szXMLTable, m_pMyElement) == TRUE)
				{
					//Peter Ringering - 02/11/2004 - 1-10018 - Controller, Security, Users was trying to pass in a
					//list control pointer during GetCurrent which was causing the app to crash at ClearGridCells.
					if (m_pMyElement.nType == GRIDTYPE)
					{
						//If we are here, we didn't get data, check to see if Grid is dirty before clearing!
						ICMSGrid* pCMSGrid = GetCMSGrid(m_pMyElement.pObjVal);
						if (!pCMSGrid->CMSGetIsParentDirty())
						{
							//01/30/2004 MLG - Added code so that we pass in the ID of the control and the table
							//name from your inheriting class's FillControlMap().
							if (!ClearGridCells(m_pMyElement.pObjVal))		//	clear grid of old data
								CGBLForm::CMSMessageBox(g_pGBLSystemInformationCMSDll->ApplicationMainFrame, IDS_ERR_CLEARING_GRID, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONERROR);
						}
					}
					else if (m_pMyElement.nType == LIST_TYPE)
					{
						//Peter Ringering - 02/11/2004 - 1-10018 - Code here to clear the list control.
						//We haven't needed it yet but if we do in the future, then this is where the code should go.
					}
				}
				return false;											//	return false
			}
		}
		//return true;
	}
	catch (_com_error e)													//catch com errors
	{
		CGBLForm::CMSMessageBox(g_pGBLSystemInformationCMSDll->ApplicationMainFrame, 0, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONERROR,
			e.ErrorMessage());
		return false;
	}
}

// ********************************************************
// * Procedure: 	MakeCOMPANYINFO
// *
// * Description:	This function adds the COMPANYINFO data
// *        		to the end of  the currently created
// *        		m_szXMLFormat variable.
// *
// * Parameters:	none
// ********************************************************
void CXML::MakeCOMPANYINFO(void)
// ********************************************************
// * Date Created: 2/13/2003 by David W. Parvin
// ********************************************************
{
	//company info node for Xml
	MakeNode("COMPANYINFO", "", 2, XML_STARTELEMENT);																													//insert table node
	MakeNode("strTenantID", g_pGBLSystemInformationCMSDll->ApplicationSettings.TenantID, 2, XML_ATTRIBUTE);																//insert table node
	MakeNode("strCompanyCode", g_pGBLSystemInformationCMSDll->CompanySettings.ID, 2, XML_ATTRIBUTE);																	//insert table node
	MakeNode("strCompanyName", g_pGBLSystemInformationCMSDll->CompanySettings.Name, 2, XML_ATTRIBUTE);																	//insert table node
	MakeNode("bolFundCompany", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->CompanySettings.IsFund), 2, XML_ATTRIBUTE);							//insert table node
	MakeNode("strLocation", g_pGBLSystemInformationCMSDll->CompanySettings.DefaultLocation, 2, XML_ATTRIBUTE);															//insert table node
	MakeNode("bolMultiBatch", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ApplicationSettings.IsMultiBatch()), 2, XML_ATTRIBUTE);					//insert table node
	MakeNode("bolInstalledAP", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ModulesInstalled.AccountsPayable.IsUsable), 2, XML_ATTRIBUTE);			//insert table node
	MakeNode("bolInstalledAR", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ModulesInstalled.AccountsReceivable.IsUsable), 2, XML_ATTRIBUTE);		//insert table node
	MakeNode("bolInstalledBR", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ModulesInstalled.BankReconciliation.IsUsable), 2, XML_ATTRIBUTE);		//insert table node
	MakeNode("bolInstalledBM", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ModulesInstalled.BillOfMaterials.IsUsable), 2, XML_ATTRIBUTE);			//insert table node
	MakeNode("bolInstalledCC", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ModulesInstalled.CostCenter.IsUsable), 2, XML_ATTRIBUTE);				//insert table node
	MakeNode("bolInstalledCM", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ModulesInstalled.Commissions.IsUsable), 2, XML_ATTRIBUTE);				//insert table node
	MakeNode("bolInstalledEC", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ModulesInstalled.ECommerce.IsUsable), 2, XML_ATTRIBUTE);				//insert table node
	MakeNode("bolInstalledFA", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ModulesInstalled.FinancialAnalysis.IsUsable), 2, XML_ATTRIBUTE);		//insert table node
	MakeNode("bolInstalledGL", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ModulesInstalled.GeneralLedger.IsUsable), 2, XML_ATTRIBUTE);			//insert table node
	MakeNode("bolInstalledIN", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ModulesInstalled.Inventory.IsUsable), 2, XML_ATTRIBUTE);				//insert table node
	MakeNode("bolInstalledJC", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ModulesInstalled.JobCost.IsUsable), 2, XML_ATTRIBUTE);					//insert table node
	MakeNode("bolInstalledLT", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ModulesInstalled.LotTracking.IsUsable), 2, XML_ATTRIBUTE);				//insert table node
	MakeNode("bolInstalledLA", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ModulesInstalled.Layaway.IsUsable), 2, XML_ATTRIBUTE);					//insert table node
	MakeNode("bolInstalledMLI", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ModulesInstalled.MultiLocationInventory.IsUsable), 2, XML_ATTRIBUTE);	//insert table node
	MakeNode("bolInstalledOE", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ModulesInstalled.OrderEntry.IsUsable), 2, XML_ATTRIBUTE);				//insert table node
	MakeNode("bolInstalledPR", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ModulesInstalled.Payroll.IsUsable), 2, XML_ATTRIBUTE);					//insert table node
	MakeNode("bolInstalledPS", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ModulesInstalled.SalesEntry.IsUsable), 2, XML_ATTRIBUTE);				//insert table node
	MakeNode("bolInstalledPL", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ModulesInstalled.Polling.IsUsable), 2, XML_ATTRIBUTE);					//insert table node
	MakeNode("bolInstalledPO", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ModulesInstalled.PurchaseOrder.IsUsable), 2, XML_ATTRIBUTE);			//insert table node
	MakeNode("bolInstalledSPS", CGBLForm::ConvertTrueFalseToString(g_pGBLSystemInformationCMSDll->ModulesInstalled.SpecialtyShop.IsUsable), 2, XML_ATTRIBUTE);			//insert table node
	if (g_pGBLSystemInformationCMSDll->ModuleSettings.CurrentSettings != NULL)
	{
		CModuleSettings* CurSet = g_pGBLSystemInformationCMSDll->ModuleSettings.CurrentSettings;
		MakeNode("bytGLInt", CGBLForm::ConvertIntToString(CurSet->GLIntegration), 2, XML_ATTRIBUTE);	//insert table node
		MakeNode("bytARInt", CGBLForm::ConvertIntToString(CurSet->ARIntegration), 2, XML_ATTRIBUTE);	//insert table node
		MakeNode("bytAPInt", CGBLForm::ConvertIntToString(CurSet->APIntegration), 2, XML_ATTRIBUTE);	//insert table node
		MakeNode("bytBRInt", CGBLForm::ConvertIntToString(CurSet->BRIntegration), 2, XML_ATTRIBUTE);	//insert table node
		MakeNode("bytINInt", CGBLForm::ConvertIntToString(CurSet->INIntegration), 2, XML_ATTRIBUTE);	//insert table node
		MakeNode("bytJCInt", CGBLForm::ConvertIntToString(CurSet->JCIntegration), 2, XML_ATTRIBUTE);	//insert table node
		MakeNode("bytPRInt", CGBLForm::ConvertIntToString(CurSet->PRIntegration), 2, XML_ATTRIBUTE);	//insert table node
		MakeNode("bytPOInt", CGBLForm::ConvertIntToString(CurSet->POIntegration), 2, XML_ATTRIBUTE);	//insert table node
		MakeNode("bytSAInt", CGBLForm::ConvertIntToString(CurSet->SAIntegration), 2, XML_ATTRIBUTE);	//insert table node
	}
	m_szXMLFormat += g_pGBLSystemInformationCMSDll->Localization.GetXML();
	m_szXMLFormat += g_pGBLSystemInformationCMSDll->CustomeModsInstalled();
	MakeNode("COMPANYINFO", "", 2, XML_ENDELEMENT);
};

// ****************************************************************
// * FillControls:	This function is used to get the data set the
// *        		data in the controls from XML
// * szControlID:	CString, Map ID for control to fill
// *				first char in id is the id for the control
// *				if there is a second char in the id it is use for multiple control with the same
// *				name and same dataset(table). Every thing start with 0
// * nDirection:	CString, The way the data is being returned
// ****************************************************************
bool CXML::FillControls(CString szControlID, int nDirection)
{
	CWnd* pFirstControlWnd = NULL; // TND 6/2/03 used to store pointer to first control so a message can be sent to parent when fill controls is done
	//set method variables AES
	COleDateTime dateTemp;
	COleDateTime* pdateTemp;


	BSTR data;
	CString strTemp, szMapID, szCounter;
	CString strName, strValue, szValue;
	COleDateTime dteValue;
	int nValue;
	int nType;
	//Create an pointer of the Node list, Element, and Node AES
	CCMSXMLDomNodeList::ComponentTypePtr pNodeList;								//	pointers used in XML  CCMSXMLDomElement
	CCMSXMLDomNode::ComponentTypePtr pNode;
	CCMSXMLDomNode::ComponentTypePtr pInnerNode;
	CCMSXMLDomNode::ComponentType* pXMLElement = NULL;;

	szMapID = szControlID;														//	Set the table number to map ID for map lookup AES
	szCounter = "";																//	counter is blank because of the first table
	if (szControlID != _T(""))
	{																			//	not doing account segment autofill
		szControlID = "";


		pNodeList = m_docPtr->getElementsByTagName(_bstr_t("Table" + szControlID));	//	get table element AES
		pNode = pNodeList->Getitem(0);											//	assign node list to node pointer AES
		int j = 0;																//	the next number to start the counter in the get item for nodes
		int nControlID = 1;

		//Check for any CMSDBOBJECT_TYPE which is a generic map from field names to values
		FillValuesToCMSDBObjects();

		while (pNode != NULL)
		{																		//	keep doing while there is data
			CString szGroupID = szMapID + szCounter;

			KEYTOELEMENTMAP* pKeyToElementMap = NULL;
			GetCtrlMapForGroup(szGroupID, pKeyToElementMap);
			if (pKeyToElementMap)
			{
				//	Get the Node List for all the returned values
				pNode->get_childNodes(&pNodeList);

				long nLen = 0;
				pNodeList->get_length(&nLen);									//	get the number of nodes in list AES

				if (nDirection != RECORD_AUTOFILL)
					MarkElementsBeforeFill(pKeyToElementMap);

				KEYTOELEMENTMAP::iterator itKey;
				MAPELEMENT* pElement = NULL;

				for (long i = 0; i < nLen; i++)
				{																//	go through the list to assign to the controls
					pNodeList->get_item(i, &pXMLElement);						//	get XML node element
					pXMLElement->get_nodeName(&data);
					strName = data;
					pXMLElement->get_text(&data);								//	get XML data value of element
					strValue = data;

					// begbert 10-31-2017 PBI 36450 : We now stow all the data we've gotten for future reference.
					m_GottenData["Table" + szControlID][strName] = strValue;

					//Condition szControlID.IsEmpty is added so that the FieldValueCallBack is
					//executed only for the first table. PGP(02/04/2004) (See Andy Swanson for details)
					if (m_IDataCallBackProvider && szControlID.IsEmpty())
						m_IDataCallBackProvider->FieldValueCallBack(strName, strValue);

					NotifyFieldValue(_T("Table") + szControlID, strName, strValue);

					itKey = pKeyToElementMap->find(strName);
					if (itKey == pKeyToElementMap->end())
						continue;

					pElement = &itKey->second;

					// Assign the value to the control
					// Different types of controls have different assigning mechanisms
					COleDateTime dateTime;										//	TND 5/30/03 variable for parsing a date time value
					if (nDirection != RECORD_AUTOFILL)
						pElement->bFillFlag = true;

					if (!pFirstControlWnd)										//	TND 6/2/03 grabbing pointer to first control so a message can be sent to parent when fill controls is done
					{
						//add check for grouper type because, if pObjVal is really of this
						//type, we cannot cast it to a CWnd since it only derives from CObject
						if (pElement->pObjVal->IsKindOf(RUNTIME_CLASS(CCMSGrouper)))
						{
							CCMSGrouper* const pclsGrouper = reinterpret_cast<CCMSGrouper*>(pElement->pObjVal);
							pFirstControlWnd = pclsGrouper->GetParentWindow();
						}
						else
							pFirstControlWnd = (CWnd*)pElement->pObjVal;
					}

					int nItems;													//	Used to see how many items in a combobox
					switch (pElement->nType)										//	get map control type
					{
					case FUNCCALLWRAPPERTYPE:									// begbert 10-27-2006 1-20344
						((CFuncCallWrapper*)pElement->pObjVal)->Set(strValue);
						break;
					case STRINGTYPE:											//	assign value for CEdit type AES
						((CEdit*)pElement->pObjVal)->SetWindowText(strValue);	//	set text to object in control map AES
						break;
					case ENCRYPTED_STRINGTYPE:
						// DTG, 4.2.2004 - decrypt data then set window.
						strValue = CGBLMath::DecryptData(strValue);
						((CEdit*)pElement->pObjVal)->SetWindowText(strValue);
						break;
					case STATICTYPE:											//	assign value for CStatic for char type AES
						// DTG, 12.15.2003, allow use padded static
						if (m_bUsePaddedCtrl)((CCMSPaddedStatic<CStatic>*)pElement->pObjVal)->SetWindowText(strValue);
						// DTG, 12.15.2003, else normal static
						else((CStatic*)pElement->pObjVal)->SetWindowText(strValue);		//	set text to object in control map AES
						break;
					case STATIC_NUMBERTYPE:										//	assign value for CStatic for number type AES
						if (!m_pNumEdit)											//	use cms number edit to format
							CreateNumberEdit(pElement->pObjVal);				//	call method to create object
						m_pNumEdit->SetValue(_tstof(strValue));
						m_pNumEdit->GetWindowText(strValue);
						// DTG, 12.15.2003, allow use padded static
						if (m_bUsePaddedCtrl)((CCMSPaddedStatic<CStatic>*)pElement->pObjVal)->SetWindowText(strValue);
						else((CStatic*)pElement->pObjVal)->SetWindowText(strValue);		//	set text to object in control map AES
						delete m_pNumEdit;
						m_pNumEdit = NULL;										//	numeric edit
						break;
					case STATIC_CURRENCYTYPE:									//	assign value for CStatic for currency type AES
						if (!m_pCurEdit)											//	use cms currency edit to format
							CreateCurrencyEdit(pElement->pObjVal);				//	call method to create object
						m_pCurEdit->SetValue(_tstof(strValue));
						m_pCurEdit->GetWindowText(strValue);
						// DTG, 12.15.2003, allow use padded static
						if (m_bUsePaddedCtrl)((CCMSPaddedStatic<CStatic>*)pElement->pObjVal)->SetWindowText(strValue);
						else((CStatic*)pElement->pObjVal)->SetWindowText(strValue);		//	set text to object in control map AES
						delete m_pCurEdit;
						m_pCurEdit = NULL;										//	currency edit
						break;
					case OXCURRENCYTYPE:
						//JKC This is for OXCurrencyEdit Controls ONLY!
						((COXCurrencyEdit*)pElement->pObjVal)->SetValue(_tstof(strValue));
						break;
					case CURRENCYTYPE:
						if (!m_pCurEdit)											//	use cms currency edit to format
							CreateCurrencyEdit(pElement->pObjVal);				//	call method to create object
						m_pCurEdit->SetValue(_tstof(strValue));
						m_pCurEdit->GetWindowText(strValue);
						// DTG, 12.15.2003, allow use padded static
						if (m_bUsePaddedCtrl)
							((CCMSPaddedStatic<CStatic>*)pElement->pObjVal)->SetWindowText(strValue);
						else
							((CEdit*)pElement->pObjVal)->SetWindowText(strValue);		//	set text to object in control map AES
						delete m_pCurEdit;
						m_pCurEdit = NULL;										//	currency edit
						break;
						// InTimeTec: 11/22/2013 : HS : 3667 D_SA_ Allow negative numbers in the Balance Registers window for all tender types. 1-37370
					case CURRENCYTYPE_NEGATIVE:
						if (m_bUsePaddedCtrl)((CCMSPaddedStatic<CStatic>*)pElement->pObjVal)->SetWindowText(strValue);
						else((CStatic*)pElement->pObjVal)->SetWindowText(strValue);		//	set text to object in control map
						break;
						// InTimeTec: End
					case STATIC_DATETYPE:										//	assign value for CStatic for Date type AES
						// DTG, 12.15.2003, allow use padded static
						if (m_bUsePaddedCtrl)
							((CCMSPaddedStatic<CStatic>*)pElement->pObjVal)->SetWindowText((LPCTSTR)FormatFromXMLDate(strValue));
						else
							((CStatic*)pElement->pObjVal)->SetWindowText((LPCTSTR)FormatFromXMLDate(strValue));	//set text to object in control map AES
						break;
					case CHECKTYPE:
						if ((strValue == "true") || (strValue == "false"))		//	true/false string need to convert to int
							nValue = CGBLForm::ConvertTrueFalseToInt(strValue);
						else
							nValue = _tstoi(strValue);							//	convert to int
						((CButton*)pElement->pObjVal)->SetCheck(nValue);		//	set check value to object in control map AES
						break;
					case COMBOTYPE:
					{
						CComboBox* const pCBO = DYNAMIC_DOWNCAST(CComboBox, pElement->pObjVal);
						ASSERT(pCBO);

						const int nSel = pCBO->FindStringExact(0, strValue);
						if (nSel == CB_ERR)
							if (strValue.IsEmpty())
								pCBO->SetCurSel(0); // If the value is empty, then set the index to 0.
							else
								pCBO->SetWindowText(strValue);					//	assign value for CCombType type AES
						else
							pCBO->SetCurSel(nSel);
					}
					break;
					case COMBOPARTTYPE:
					{
						CComboBox* const pCBO = DYNAMIC_DOWNCAST(CComboBox, pElement->pObjVal);
						ASSERT(pCBO);

						const int nSel = pCBO->FindString(0, strValue);
						if (nSel == CB_ERR)
							pCBO->SetWindowText(strValue);					//	assign value for CCombType type AES
						else
							pCBO->SetCurSel(nSel);
					}
					break;
					case COMBOITEMDATATYPE:
						// JKC If the item's data is equal then Select that entry.
						nItems = ((CComboBox*)pElement->pObjVal)->GetCount();
						for (int i = 0; i < ((CComboBox*)pElement->pObjVal)->GetCount(); i++)
						{
							if (((CComboBox*)pElement->pObjVal)->GetItemData(i) == (DWORD)_tstoi(strValue))
							{
								((CComboBox*)pElement->pObjVal)->SetCurSel(i);
								((CComboBox*)pElement->pObjVal)->GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(((CComboBox*)pElement->pObjVal)->GetDlgCtrlID(), CBN_SELCHANGE), (LPARAM)((CComboBox*)pElement->pObjVal)->GetSafeHwnd());
							}
						}
						break;
					case COMBOLIST_TYPE:
					{
						ComboList.szField = strName;
						ComboList.szValue = strValue;
						NMHDR wMessage {};											//	message
						wMessage.code = (UINT)(WPARAM)&ComboList;						//	set message with defined
						wMessage.hwndFrom = ((CComboBox*)pElement->pObjVal)->GetSafeHwnd();	//get safe handle
						//Pass a message to the Property sheet to get data AES
						NMHDR lMessage {};											//	message
						lMessage.code = WM_SET_COMBO_TEXT;						//	set message with defined
						lMessage.hwndFrom = ((CComboBox*)pElement->pObjVal)->GetSafeHwnd();	//get safe handle
						((CComboBox*)pElement->pObjVal)->GetParent()->SendMessage(WM_NOTIFY, (WPARAM)&wMessage, (LPARAM)&lMessage);
						((CComboBox*)pElement->pObjVal)->SendMessage(WM_NOTIFY, (WPARAM)&wMessage, (LPARAM)&lMessage);
					}
					break;
					case COMBOBOX_INDEX: // TND 5/29/03 sets the current selection of a combo box based on an index pulled from the XML data
						((CComboBox*)pElement->pObjVal)->SetCurSel(_tstoi(strValue));
						break;
					case RTXEDITTYPE:
						strValue.Replace(_T("\n"), _T("\r\n"));				//	add to make is use returns. aes
						((CRichEditCtrl*)pElement->pObjVal)->SetWindowText(strValue);	//	assign value for Rich Text Edit type AES
						break;
					case RADIOTYPE:												//	assign value for CButton type(radio) type AES
						if ((strValue == "true") || (strValue == "false"))		//	true/false string need to convert to int
							nValue = CGBLForm::ConvertTrueFalseToInt(strValue);
						else
							nValue = _tstoi(strValue);							//	convert to int
						for (int i = 0; i <= 5; i += 1)
						{														//	use up to 5 radio control in a grouping
							if (pElement != NULL && nValue == pElement->pnVal)
							{													//	get map id check for radio group from XML
								((CButton*)pElement->pObjVal)->SetState(true);	//	set state
								((CButton*)pElement->pObjVal)->SetFocus();		//	set focus
								((CButton*)pElement->pObjVal)->SetCheck(1);		//	set check
								break;
							}	//after data is fill, at form view level set focus back to where it was before data was retrieved AES
							//LookupElement((szMapID + szValue), strName, m_pMyElement);
							itKey = pKeyToElementMap->find((szMapID + szValue));
							if (itKey != pKeyToElementMap->end())
								pElement = &itKey->second;
							else
								pElement = NULL;
						}
						break;
					case GROUPTYPE:		//assign value for Radio Button CMSGrouper
						nType = ((CCMSGrouper*)pElement->pObjVal)->GetGroupType();

						//sb - 1-10904 - modified to allow for *_GROUP_ALLOW_NONE types
						switch (nType)
						{
						case CCMSGrouper::INT_GROUP:
						case CCMSGrouper::INT_GROUP_ALLOW_NONE:
							nValue = _tstoi(strValue);							//	convert to int
							((CCMSGrouper*)pElement->pObjVal)->SetIntegerValue(nValue);
							break;

						case CCMSGrouper::CSTR_GROUP:
						case CCMSGrouper::CSTR_GROUP_ALLOW_NONE:
							((CCMSGrouper*)pElement->pObjVal)->SetStringValue(strValue);
							break;
						};

						break;
					case DATETYPE:												//	assign value for date time  type AES
						((CDateTimeCtrl*)pElement->pObjVal)->EnableWindow(true);
						((CDateTimeCtrl*)pElement->pObjVal)->SetTime(FormatFromXMLToOLEDate(strValue));
						break;
					case DATETIMETYPE:											//	TND 5/30/03 converts to COLEDateTime using ParseDateTime - can be used for time only values
						if (!strValue.IsEmpty())									//	if date value is not empty, date control will be enabled and set
						{
							((CDateTimeCtrl*)pElement->pObjVal)->EnableWindow(true);
							dateTime.ParseDateTime(strValue);
							//Causes an assert if tried to set an invalid date time value - PGP(11/14/2003)
							if (dateTime.GetStatus() == COleDateTime::valid)
								((CDateTimeCtrl*)pElement->pObjVal)->SetTime(dateTime);
							else
							{
								dateTime.SetTime(0, 0, 0);						//	default to 12:00:00 AM
								((CDateTimeCtrl*)pElement->pObjVal)->SetTime(dateTime);
							}
						}
						else													//	date value is empty so date control will be disabled
							((CDateTimeCtrl*)pElement->pObjVal)->EnableWindow(false);
						break;
					case CMSPHONENUMBERTYPE:
						//As part of fixing Defect ID 1-13905, PGP(09/28/2004) - don't think
						//we need to enable window while setting window text.
						((CWnd*)pElement->pObjVal)->SendMessage(WM_CMS_SETPHONE, (WPARAM)&strValue, 0);
						break;
					case CMSDATETYPE://Assign value for CMSDateCtrl type
						//PGP(08/26/2004) - EnableDisable does the enabling/disabling later.
						//control does not need to be enabled to do a set text, commenting
						//as part of fixing 1-13480
						dateTemp = FormatFromXMLToOLEDate(strValue);
						pdateTemp = &dateTemp;
						((CWnd*)pElement->pObjVal)->SendMessage(WM_CMS_SETDATE, (WPARAM)(pdateTemp), 0);
						break;

					case ACCOUNTTYPE:											//	assign value for account control type AES
						((CWnd*)pElement->pObjVal)->SetWindowText(strValue);
						break;

					case LIST_TYPE:
						//code to fill list control goes here AES
						break;

					case CONTROLDATA:
						ASSERT(pElement->pObjVal->IsKindOf(RUNTIME_CLASS(CCMSControlData)));
						((CCMSControlData*)pElement->pObjVal)->SetCapturedData(strValue);
						((CCMSControlData*)pElement->pObjVal)->GetControl()->SetWindowText(strValue);
						break;
					case OX_NUMERIC_TYPE:										//	Defect ID 1-8389, PGP(11/24/2003)
					{
						COXNumericEdit* pEdit = (COXNumericEdit*)pElement->pObjVal;
						pEdit->SetValue(_tstof(strValue));
					}
					break;
					}

					//sb..17-Jul-03 - Send a message to the control to let it know that it has been populated.
					if (pElement->pObjVal->IsKindOf(RUNTIME_CLASS(CCMSGrouper)))
					{
						CCMSGrouper* const pclsGrouper = reinterpret_cast<CCMSGrouper*>(pElement->pObjVal);
						::SendMessage(pclsGrouper->GetParentWindow()->m_hWnd, WM_CTRLPOPULATED, (WPARAM)0, (LPARAM)0);
					}
					else if (pElement->pObjVal->IsKindOf(RUNTIME_CLASS(CCMSControlData)))
						((CCMSControlData*)pElement->pObjVal)->SendMessage(WM_CTRLPOPULATED, (WPARAM)0, (LPARAM)0);
					else if (pElement->pObjVal->IsKindOf(RUNTIME_CLASS(CWnd)))	//	begbert 10-27-2006 1-20344 : genericized to properly screen all invalid cases
						((CWnd*)pElement->pObjVal)->SendMessage(WM_CTRLPOPULATED, (WPARAM)0, (LPARAM)0);
				}
				//if(nDirection != RECORD_AUTOFILL) change because it was cause the not to clear out on the GL master form
				if (nDirection != RECORD_AUTOFILL && !m_bDontClearAllControl)
					ClearMissedElements(pKeyToElementMap);
			}

			//Entries remaining in temp map now are
			szControlID.Format(_T("%d"), nControlID);								//	control for the next table in the xml
			pNodeList = m_docPtr->getElementsByTagName(_bstr_t("Table" + szControlID));	//	get node list
			pNode = pNodeList->Getitem(j);											//	get next node List
			szCounter.Format(_T("%d"), j);											//	Item number and count should sink here
			if (pNode == NULL)															//	no more item, get the next table
			{
				j = 0;																//	add for next node item
				nControlID++;
				szControlID.Format(_T("%d"), nControlID);							//	control for the next table in the xml
				pNodeList = m_docPtr->getElementsByTagName(_bstr_t("Table" + szControlID));	//	get node list
				pNode = pNodeList->Getitem(j);										//	get next node
			}
			else
			{
				if (!m_bMultipleDetails)
				{
					pNode = pNodeList->Getitem(j + 1);									//	get next node
					if (pNode != NULL)
						pNode = NULL;
				}
				j++;
			}
			szMapID = szControlID;													//	table has changed, change map id
		}
	}// end if szControlID!=_T("")...account segment autofill
	//Assigning pointers to null without deleting
	//will cause memory leak PGP(03/18/2003)
	//Added code to delete
	//AES move up to were the control is called
	delete m_pCharEdit;
	//End added coded to delete.
	//pointer to the cms edit class
	m_pCharEdit = NULL;				// character edit

	// TND 6/2/03 grabbing pointer to first control so a message can be sent to parent when fill controls is done
	CGBLForm::SendMessageToParent(pFirstControlWnd, FILLCONTROLS);

	return true;
}

//****************************************************************
//FillGridWithData - This function is used to fill data set to the grid
//					from XML
//	*pObjVal - CObject,			Map ID for grid to fill
//	szTableNumber - CString,	The ID of the grid to table
//****************************************************************
bool CXML::FillGridWithData(CObject* pObjVal, CString szTableNumber)
{
	BSTR data;
	CString strTemp, szMapID, szID, szIDData;
	CString szName, szValue, szChar;
	CUGCell	cell;																//	grid cell class
	bool bValue = false;
	TRACE("fill grid start");
	DWORD dwTick = GetTickCount();												//	use as counter

	//Create an pointer of the Node list, Element, and Node AES
	CCMSXMLDomNodeList::ComponentTypePtr pNodeList;								//	pointers used in XML  CCMSXMLDomElement
	CCMSXMLDomNode::ComponentTypePtr pNode;
	CCMSXMLDomNode::ComponentTypePtr pInnerNode;
	CCMSXMLDomNode::ComponentType* pElement = NULL;;


	szMapID = szTableNumber;													//	Set the table number to map ID for map lookup AES
	if (szTableNumber == _T("0"))												//	If the table is 0 the node will be <Table> to set variable to blank
		szTableNumber = _T("");

	if (!ClearGridCells(pObjVal))												//PGP(06/30/2003)
		CGBLForm::CMSMessageBox(g_pGBLSystemInformationCMSDll->ApplicationMainFrame, IDS_ERR_CLEARING_GRID, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONERROR);

	CString szTableName = _T("Table") + szTableNumber;
	//If this is an object of type CCMSMTGrid, filling will be
	//handled by a secondary worker thread launched by the grid. PGP(06/21/2004)
	CWnd* pWnd = (CWnd*)pObjVal;
	if (pWnd->SendMessage(WM_CMS_MT_TYPECHECK) > 0)
	{
		pWnd->SendMessage(WM_CMS_MT_ASYNCFILL, (WPARAM)&m_szReturn, (LPARAM)&szTableName);
		return true;
	}
	ICMSGrid* pCMSGrid = GetCMSGrid(pObjVal);
	CUGCtrl& clsGrid = pCMSGrid->CMSGetBaseClass();

	long nLen = 0;
	pNodeList = m_docPtr->getElementsByTagName(szTableName.GetBuffer());		//	get table element AES
	pNode = pNodeList->Getitem(0);												//	assign node list to node pointer AES
	pNodeList->get_length(&nLen);												//	get the number of nodes in list AES
	int j = 1;																	//	the next number to start the counter in the get item for nodes
	int r = 0;																	//	row counter of the grid
	while (pNode != NULL)
	{																			//	If node list is null no need to do anything else
		// Get the Node List for all the returned values
		pNode->get_childNodes(&pNodeList);

		// DTG, 8.3.2004, moved this from the bottom to prevent extra blank row.
		//if we are going to exceed the number of rows, then create a new row on the grid
		if (pCMSGrid->CMSGetNumberRows() < j)
			clsGrid.SetNumberRows(j);

		pNodeList->get_length(&nLen);											//	get the number of nodes in list AES

		for (long i = 0; i < nLen; i++)
		{																		//	go through the list to assign to the controls
			pNodeList->get_item(i, &pElement);									//	get XML node element
			pElement->get_nodeName(&data);
			szName = data;

			pElement->get_text(&data);											//	get XML data value of element
			szValue = data;

			NotifyFieldValue(_T("Table") + szTableNumber, szName, szValue);

			CGrdMapColumnInfo& pMap = pCMSGrid->CMSGetMapColumnInfo();		//	type cast pMap to CMSGrid map info
			CGrdMapDataToText& pMapData = pCMSGrid->CMSGetMapDataToText();		//	type cast pMap to DataToText map info
			GRDCOLPROPS& pProps = pCMSGrid->CMSGetColProps();		//	type cast pMap to CMSGrid  struct
			GRDDATATOTEXT& pDataToText = pCMSGrid->CMSGetDataToText();			//	type cast pMap to DataToText  struct
			POSITION pos = pMap.GetStartPosition();

			//get start position in map Column
			while (pos != NULL)
			{
				pMap.GetNextAssoc(pos, szID, pProps);							//	get first column
				//AES 1-5784 7.22.2003 Column now have the bool for no negative numbers.
				m_bNoNegativeNumber = !pProps.bAllowNegNumbers;
				if (pProps.szField == szName)
				{																//	check is the column and node match
					//check for column data type
					if (pProps.nDataType == UGCELLDATA_NUMBER)
					{															//	Number type
						if (!m_pNumEdit)											//	use cms number edit to format
							CreateNumberEdit(pObjVal, true);					//	call method to create object
						m_pNumEdit->m_bNoNegativeNumber = m_bNoNegativeNumber;
						m_pNumEdit->SetDecimalDigitCount(pProps.nDecimalDigitCount);
						m_pNumEdit->SetFractionalDigitCount(pProps.nFractionalDigitCount);	//	use column properties
						m_pNumEdit->SetShowLeadingZero(true);
						if (szValue.Find(_T("E")) > 0)
						{
							double dValue = _tstof(szValue);
							szValue.Format(_T("%.10f"), dValue);
						}
						m_pNumEdit->SetWindowText((LPCTSTR)szValue);			//	set value to format
						m_pNumEdit->GetWindowText(szValue);						//	get format value

						if (pProps.nTranslateType == GRD_XLAT_PERCENT)
						{
							//PGP(03/04/2004), introduced temp variable so Format
							//does not use the same string as argument(causes pointer corruption)
							CString szTemp = szValue;
							szValue.Format(_T("%s%%"), (LPCTSTR)szTemp);
						}
					}
					else if (pProps.nDataType == UGCELLDATA_CURRENCY)
					{															//	currency type
						if (!m_pCurEdit)											//	use cms currency edit to format
							CreateCurrencyEdit(pObjVal, true);					//	call method to create object
						m_pCurEdit->m_bNoNegativeNumber = m_bNoNegativeNumber;
						m_pCurEdit->SetDecimalDigitCount(pProps.nDecimalDigitCount);
						m_pCurEdit->SetFractionalDigitCount(pProps.nFractionalDigitCount);
						m_pCurEdit->SetShowLeadingZero(true);
						if (szValue.Find(_T("E")) > 0)
						{
							double dValue = _tstof(szValue);
							szValue.Format(_T("%.10f"), dValue);
						}
						// DG 1-3603, 1-3604, 5/8/2003
						// check flag, see if negative numbers are allowed in grid.
						if (m_bNoNegativeNumber == false)
						{
							// DG 1-3603, 1-3604, 5/8/2003
							// if no minus sign, toggle edit box to be positive
							if (szValue.Find(_T("-")) < 0)
							{
								// DG 1-3603, 1-3604, 5/8/2003
								// set edit box to positive number.
								m_pCurEdit->ChangeSign(false);
							}
						}
						m_pCurEdit->SetValue(_tstof(szValue));
						//m_pCurEdit->SetWindowText((LPCTSTR)szValue);			//	set value to format
						m_pCurEdit->GetWindowText(szValue);						//	get format value

					}
					else if ((pProps.nDataType == UGCELLDATA_TIME) || (pProps.nDataType == GRD_GRIDDATEPICKER))
						szValue = FormatFromXMLDate(szValue.Left(10));			//	format to date string
					else if (pProps.nDataType == GRD_TXTDATETIME)
						szValue = FormatFromXMLDateTime(szValue);
					else if (pProps.nDataType == GRD_TXTSMALLDATETIME)
						szValue = FormatFromXMLDateTime(szValue, TRUE);
					else if (pProps.nDataType == UGCELLDATA_BOOL)
						bValue = (szValue == _T("true") || szValue == _T("1"));	//	Defect ID 1-14269, PGP(10/26/2004)
					else
					{
						// translate what is in the cell to something the database can understand
						// based on the Translate flag
						switch (pProps.nTranslateType)
						{
						case GRD_XLAT_NOTHING:
							break;
						case GRD_XLAT_DEBITCREDIT:								//	from true/false ->debit/credit
							CGBLForm::TranslateDebitCredit(szValue, true);		//	direction relative to the client (true=to the grid, false=from the grid)
							break;
						case GRD_XLAT_ACCTCLASSIF:								//	from number to account classification description (Asset,...)
							CGBLForm::TranslateAcctClassif(szValue, true);		//	direction relative to the client (true=to the grid, false=from the grid)
							break;
						case GRD_XLAT_ENCUMBCHECK:								//	nothing at this time
							CGBLForm::TranslateEncumber(szValue, true);			//	direction relative to the client (true=to the grid, false=from the grid)
							break;
						case GRD_XLAT_ACCOUNTNMBR:								//	adds segment separators to the account number
							CGBLForm::TranslateAcctNumber(szValue, true);		//	direction relative to the client (true=to the grid, false=from the grid)
							break;
						case GRD_XLAT_MORELESS:								//	from true/false to less/more
							CGBLForm::TranslateMoreLess(szValue, true);			//	direction relative to the client (true=to the grid, false=from the grid)
							break;
						case GRD_DATATOTEXT:
							if (pMapData.Lookup((_bstr_t)(pProps.szID + szValue), pDataToText) == TRUE) //	lookup control in the map by map ID and element name AES
								szValue = pDataToText.szText;
							break;
						case STATIC_DATETYPE:
							szValue = FormatFromXMLDate(szValue);
						}
					}
					if (pProps.nDataType == UGCELLDATA_BOOL)
						cell.SetBool(bValue);
					else
						cell.SetText(szValue);									//	set the header text
					cell.SetAlignment((short)pProps.nAlignment);				//	set cell alignment
					//need to set the cell type too, or it was cause an error by setting the wrong cell type to the cells. AES 2.3.04
					cell.SetCellType(pProps.nCellType);
					cell.SetDataType((short)pProps.nDataType);					//	set data type
					cell.SetCellType(pProps.nCellType);
					//If cell type is drop list, add extended property to
					//show button only when user clicks on cell. PGP(07/11/2003)
					if (pProps.nCellType == UGCT_DROPLIST)
						cell.SetCellTypeEx(UGCT_DROPLISTHIDEBUTTON);
					cell.SetFont(&g_pGBLSystemInformationCMSDll->Drawing.Fonts.Normal);	//	set cell font

					if (pCMSGrid->CMSGetNoHorizontalLines())
					{															//	check to see if grid has horizontal lines
						cell.SetBorder(UG_BDR_TTHIN);							//	get top line
						cell.SetBorder(UG_BDR_BTHIN);							//	get bottom line
						cell.SetBorderColor(&g_pGBLSystemInformationCMSDll->Drawing.Pens.White);	//	set line to white
					}
					//JKC Keep original back color
					cell.SetBackColor(pProps.nCellBackColor);
					clsGrid.SetCell(pProps.nOrder, r, &cell);					//	set cell
				}
				else
				{
					if (pProps.szField == "DIRTY")
					{
						cell.SetText(_T("0"));									//	set the header text
						cell.SetAlignment((short)pProps.nAlignment);			//	set cell alignment
						//need to set the cell type too, or it was cause an error by setting the wrong cell type to the cells. AES 2.3.04
						cell.SetCellType(pProps.nCellType);
						cell.SetDataType((short)pProps.nDataType);				//	set data type
						cell.SetFont(&g_pGBLSystemInformationCMSDll->Drawing.Fonts.Normal);	//	set cell font
						clsGrid.SetCell(pProps.nOrder, r, &cell);				//	set cell
					}
				}
			}
		}
		pNodeList = m_docPtr->getElementsByTagName(_bstr_t("Table" + szTableNumber));	//	get node list
		pNode = pNodeList->Getitem(j);											//	get next node
		j++;	//add for next node item
		r++;	//add for next row

	}

	//Send a message to the control to let it know that it has been populated.
	((CWnd*)pObjVal)->SendMessage(WM_CTRLPOPULATED, (WPARAM)0, (LPARAM)0);		//sb..17-Jul-03

	clsGrid.RedrawAll();														//	redraw grid

	//Assigning pointers to null without deleting
	//will cause memory leak PGP(03/18/2003)
	//Added code to delete the variables
	delete m_pNumEdit;
	delete m_pCharEdit;
	delete m_pCurEdit;
	//End added coded to delete.
	// pointer to the cms edit class
	m_pNumEdit = NULL;															//	numeric edit
	m_pCharEdit = NULL;															//	character edit
	m_pCurEdit = NULL;															//	currency edit

	TRACE("fill grid = %d\n", GetTickCount() - dwTick);

	return true;
}

//****************************************************************
//FillGridWithData - This function is used to clear grid of text
//
//	*pObjVal - CObject,			Map ID for grid to clear text
//
//****************************************************************
bool CXML::ClearGridCells(CObject* pObjVal)
{
	if (HasResetProvider())
	{
		GetResetProvider()->Reset();
		return true;
	}

	CUGCell	cell;				//grid cell class
	int		nRows;				//number of rows
	CString	szID;

	//Peter Ringering - 02/17/2004 - 1-10021
	if (pObjVal->IsKindOf(RUNTIME_CLASS(CWnd)) && !HasResetProvider())
	{
		CWnd* pControl = (CWnd*)pObjVal;
		if (pControl->SendMessage(WM_CMSGRID_CLEARGRID) > 0) return true;
	}

	// DTG, 5.25.2004, changed to use built in method.
	ICMSGrid* pCMSGrid = GetCMSGrid(pObjVal);
	CUGCtrl& clsGrid = pCMSGrid->CMSGetBaseClass();
	nRows = clsGrid.GetNumberRows();												//	get the number of row in grid

	CGrdMapColumnInfo& pMap = pCMSGrid->CMSGetMapColumnInfo();				//	type cast pMap to CMSGrid map info
	GRDCOLPROPS& pProps = pCMSGrid->CMSGetColProps();						//	type cast pMap to CMSGrid COLPROPS struct
	POSITION pos = pMap.GetStartPosition();												//	get start position in map Column
	while (pos != NULL)
	{
		pMap.GetNextAssoc(pos, szID, pProps);
		for (int j = 0; j <= nRows; j++)
		{
			if (pProps.szID > pCMSGrid->CMSGetStartClear())
			{																			//	set cell properties
				cell.SetAlignment((short)pProps.nAlignment);
				cell.SetDataType((short)pProps.nDataType);
				cell.SetFont(&g_pGBLSystemInformationCMSDll->Drawing.Fonts.Normal);
			}
			if (pCMSGrid->CMSGetNoHorizontalLines())
			{																			//	check to see if grid has horizontal lines
				cell.SetBorder(UG_BDR_TTHIN);											//	get top line
				cell.SetBorder(UG_BDR_BTHIN);											//	get bottom line
				cell.SetBorderColor(&g_pGBLSystemInformationCMSDll->Drawing.Pens.White);//	set line to white
			}
			cell.SetText(_T(""));															//	clear cell
			//JKC Keep original Back Color!
			cell.SetBackColor(pProps.nCellBackColor);

			clsGrid.SetCell(pProps.nOrder, (j), &cell);					//	set cell with new cleared text
		}
	}
	return true;
}

//****************************************************************
//ClearControls - This method is used to clear the control on a form, using the Map of controls AES
//
//	szControlID - CString,			Map ID for controls to clear text
//									If szControlID = "ALL" then clear all controls
//****************************************************************
bool CXML::ClearControls(CString szControlID)
{
	//This method is used to clear the control on a form, using the Map of controls AES
	//
	ClearCMSDBObjects();

	if (szControlID != "")
	{
		//CMapColumnInfo& map = m_mapColumnInfo;				//	Create a pointer to the control map AES
		GROUPTOCTRLSMAP::iterator it;
		for (it = m_clsMapCtrlGroups.begin(); it != m_clsMapCtrlGroups.end(); it++)
		{
			if (it->first == szControlID || szControlID == "ALL")
				ClearGroupElements(it->second);
		}
	}
	return false;
}

//****************************************************************
//EnableDisable - This method is used to enable and disable the control on a form,
//				using the Map of controls AES
//
//	szControlID - CString,			Map ID for controls to clear text
//	nType - int						Type of control, disable or enable
//
//****************************************************************
void CXML::EnableDisable(enuEnable nType, CString szControlID)
{
	if (szControlID != "")
	{
		bool bEnable = (nType == ENABLE);

		GROUPTOCTRLSMAP::iterator it;
		for (it = m_clsMapCtrlGroups.begin(); it != m_clsMapCtrlGroups.end(); it++)
		{
			if (it->first == szControlID)
				EnableGroupElements(it->second, bEnable);
		}
	}
}

//****************************************************************
//AutoFill - This method is used to for Autofill a control on a form AES
//
//	szControlID - CString,			Map ID for controls to clear text
//	nType - int						Type of control, disable or enable
//
//****************************************************************
bool CXML::AutoFill(
	CString strName,
	CString szModule,
	CString szForm,
	CStringArray* strCommandValues,
	CString szTableNumber)
{
	//Defect ID 1-8384, If the autofill flag is turned off for this company,
	//do not process autofill. PGP(10/28/2003)
	if (!g_pGBLSystemInformationCMSDll->ApplicationSettings.AutoFill) return false;
	//AutoFill AES if you have question ask me, it will be easier than trying to here. If I am no long here ask a senior to help. AES
	//This will be the same for most fills just copy and change your variables. AES
	//return false to reset the dirty flag
	//void *pOutVal = NULL;
	//int nTypeOfControl = 0;
	MAPELEMENT	pMyElement;
	int nTxtPos = 0;
	CString szValue;
	bool bReturn = true;
	bool bDelete = GetKeyState(VK_DELETE) < 0;
	bool bBackSpace = GetKeyState(VK_BACK) < 0;
	if (!bDelete)
	{
		if (LookupElement(szTableNumber, strName, pMyElement) == TRUE)
			// Assign the value to the control
			// Different types of controls have different assigning mechanisms
		{
			switch (pMyElement.nType)
			{
			case STRINGTYPE:
				((CEdit*)pMyElement.pObjVal)->GetWindowText(szValue);		//	get window text
				nTxtPos = szValue.GetLength();								//	get length
				szValue = szValue.Trim();	//Peter Ringering - 02/10/2004 - 1-9951
				if ((m_nStartChar >= nTxtPos) && (!bBackSpace))
					m_nStartChar = 0;
				if (m_nStartChar == nTxtPos && nTxtPos > 0)
				{															//	use the assigned position to start if this is the first time through
					nTxtPos = (nTxtPos - 1);
					szValue = szValue.Left(nTxtPos);						//	get string value to the left for XML get data
				}
				if (nTxtPos == 0)
				{															//	if length is 0 set value and window to blank
					szValue = "";
					((CEdit*)pMyElement.pObjVal)->SetWindowText(szValue);
					bReturn = false;	//return false
				}

				if (szValue != "")											//	if value is not blank get Auto fill data for control
					bReturn = GetXMLData(RECORD_AUTOFILL, szModule, szForm, strCommandValues, szTableNumber);
				((CEdit*)pMyElement.pObjVal)->SetSel(nTxtPos, -1);			//select current text in edit control
				((CEdit*)pMyElement.pObjVal)->GetSel(m_nStartChar, m_nEndChar);	//set the member for next autofill
				break;
			}
		}
	}
	return bReturn;
}

//****************************************************************
//ChangeXMLData - This function is used to change the data at the SQL server,
//			all data changes should got through this function. AES
//	nType - int,			The type of change insert, update or delete
//	szModule - CString,		The 2 letter module code
//	szForm - CString,		The name of the form the data is for
//	*strCommandValues -		CStringArray, XML command set Array
//	uinConnectionType
//	bNoInsert
//	bUpdateOnly             If the type is UPDATE and this is TRUE
//							then this function uses the UPDATEONLY command
//							to only update the fields in the XML data
//
//****************************************************************/
bool CXML::ChangeXMLData(
	enuRecordProc nType,
	CString szModule,
	CString szForm,
	CStringArray* strCommandValues,
	UINT uinConnectionType,
	bool bNoInsert,
	bool bUpdateOnly,
	bool bUseLowerCaseServerConnection)
{
	// 1-30178 BK 6/17/08 added bMakeServerCallLowerCaseToMatchNewCode variable to be able to set CONNECTION commands in regular case instead of all upper case to be consistent with new code
	// see CCFV_INRecalcInventoryFiles::OnFormOk()
	bMakeServerCallLowerCaseToMatchNewCode = bUseLowerCaseServerConnection;
	CoInitialize(NULL);
	//set variables for method here
	CString	szDirection = "";		//	direction string for the XML AES
	CString	szCommand = "";		//	Command string for the XML AES
	CString szTableCount = "";
	m_intTableCount = 0;

	try
	{						//	Check for the type of change data AES
		switch (nType)
		{
		case RECORD_INSERT:	//	insert records AES
			if (szForm.Mid(3) == "BATCHES")
				szCommand = "SPECIALPROCESS";
			else
				szCommand = "INSERT";
			break;
		case RECORD_UPDATE:				//update records AES
			// EK - 5/27/3 - Call UPDATEONLY command if requested
			if (bUpdateOnly)
				szCommand = "UPDATEONLY";
			else
				szCommand = "UPDATE";
			break;
		case RECORD_DELETE:				//delete records AES
			szCommand = "DELETE";
			break;
		case RECORD_PROCESS:
			szCommand = "SPECIALPROCESS";
			break;
		}

		//Make the Command string
		MakeXMLCommand(szCommand, szModule, szForm);						//	make Command XML
		//Make the Data string
		MakeXMLConnection("Connection", "10", "300", uinConnectionType, false, (nType == RECORD_INSERT) ? true : false);	//	make connection XML

		//company info node for Xml
		MakeCOMPANYINFO();

		if (bNoInsert)														//	ADD short node for no insert
			MakeNode("NOINSERT", "", 0, XML_SHORTNODE);

		if (!m_bGridOnly)
		{
			MakeNode("TABLE", "", 2, XML_STARTELEMENT);						//	insert table node

			//Use a CMSDBObject as part of xml construction
			GetValuesFromCMSDBObjects();

			for (int ii = 0; ii < strCommandValues->GetSize(); ii += 2)		//	get and insert inner node form string array AES
				MakeNode(strCommandValues->GetAt(ii), strCommandValues->GetAt(ii + 1), 3, XML_ATTRIBUTE);
			if (m_intTableCount > 0)											//	check if there was more then one table
				szTableCount.Format(_T("%d"), m_intTableCount);					//	set table number to string to pass
			if (!m_bTableEnd)
				MakeNode("TABLE" + szTableCount, "", 2, XML_ENDELEMENT);	//	insert close node for Table AES
			else
				m_bTableEnd = false;
		}
		GetGridData(nType);						//	get grid data for XML insert and updates AES
		GetListData(nType);

		if (HasDataHookProvider())
			GetDataHookProvider()->HookSaveData(*this);

		GetXMLFormat();							//	Finish XML format AES

		//m_docPtr.CreateInstance("msxml2.domdocument");
		CCMSXMLDomNodeList::ComponentTypePtr pNodeList;					//	pointers used in XML  CCMSXMLDomElement
		CCMSXMLDomNode::ComponentTypePtr pNode;
		CCMSXMLDomNode::ComponentTypePtr pInnerNode;
		//MSXML2::IXMLDOMNodeListPtr pNodeList;	//	node list pointer
		//MSXML2::IXMLDOMNodePtr pNode;			//	node pointer
		//MSXML2::IXMLDOMNodePtr pInnerNode;		//	inner node pointer

		// InTimeTec: 05/20/2013 : HS : Attach Files to Master Records
		// Modified the server call to save the Master forms with attachments
		if (m_bSaveSpecial)
		{
			m_docPtr->loadXML(SendXML(true, SXM_SPECIALPROCESS));
		}
		else
			m_docPtr->loadXML(SendXML());			//	call load xml method, which returns the _bstr_t to load in the doc pointer AES
		// InTimeTec: End
		pNodeList = m_docPtr->getElementsByTagName(_bstr_t("RECORDSAFFECTED")); //check list for no records AES
		pNode = pNodeList->Getitem(0);
		if (pNode == NULL)
		{
			//szCommand+=" HAS FAILED!";		//	if no record return the update, insert or delete has failed AES ERROR_RPT
			//Check for COMMITED because server may return that instead of "RECORDSAFFECTED"
			pNodeList = m_docPtr->getElementsByTagName(_bstr_t("COMMITED")); //check list for no records AES
			pNode = pNodeList->Getitem(0);
			if (pNode == NULL)
			{
				pNodeList = m_docPtr->getElementsByTagName(_bstr_t("ERROR_RPT")); //check for error report AES
				pNode = pNodeList->Getitem(0);
				if (pNode != NULL)
				{

					m_nMode = MODE_ERROR_RPT;
					return false; //change XML data has failed
				}
				pNodeList = m_docPtr->getElementsByTagName(_bstr_t("NewDataSet")); //check list for no records AES
				pNode = pNodeList->Getitem(0);
				if (pNode == NULL) return false; //change XML data has failed
			}
		}
		pNodeList = m_docPtr->getElementsByTagName(CMSStrings::XMLTags::Error); //check list for no records AES
		pNode = pNodeList->Getitem(0);
		if (pNode != NULL)
			return false; //change XML data has failed
	}
	catch (_com_error e)
	{											//	catch any com errors  AES
		CGBLForm::CMSMessageBox(g_pGBLSystemInformationCMSDll->ApplicationMainFrame, 0, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONERROR,
			e.ErrorMessage());
		return false;							//	change XML data has failed
	}
	return true;		//it worked AES
}

//****************************************************************
//FillGridWithData - This function is used to get data set from the grid
//					to the XML
//	nType - int		Update and Insert type used, for multiple types during update
//
// UPDATE CHANGES
// 5.26.04 AES	Made change to the way the xml nodes are made. Before we were using string and formatting
//				XML in it. This was to slow, so I changed the method to use XMLPramas which is a wrapper
//				around the MS XML 4.0. This change will affect all modules and is global.
//****************************************************************
void CXML::GetGridData(enuRecordProc nType)
{
	//add changes to use xmlparms to make nodes	AES 5.26.04
	CXMLParams clsFormatXMLParams;				//main params
	CXMLParams clsTableXMLParams;				//table params
	CXMLParams clsInsertXMLParams;				//Insert params
	CXMLParams clsUpdateXMLParams;				//Update params
	CXMLParams clsDeleteXMLParams;				//Update params
	DWORD dwTick = GetTickCount();				//use as counter

	int i = m_intTableCount;
	int nRows = 0;
	int nCol = 0;
	bool bStart, bEnd, bInstert, bUpdate, bDoAllRows = false;
	CString szID, szValue, szi, szc, szIDData;
	CUGCell	cell;

	if (nType == RECORD_PROCESS) {}

	//iterate to find all elements.
	GROUPTOCTRLSMAP& clsAllCtrlsMap = m_clsMapCtrlGroups;
	GROUPTOCTRLSMAP::iterator itGroup;
	KEYTOELEMENTMAP::iterator itKeys;
	for (itGroup = clsAllCtrlsMap.begin(); itGroup != clsAllCtrlsMap.end(); itGroup++)
	{													//	set the cells header text and size
		for (itKeys = itGroup->second.begin(); itKeys != itGroup->second.end(); itKeys++)
		{
			m_pMyElement = itKeys->second;
			if (m_pMyElement.nType == GRIDTYPE && m_pMyElement.pnVal == READWRITE)
			{
				ICMSGrid* pCMSGrid = GetCMSGrid(m_pMyElement.pObjVal);
				CUGCtrl& clsGrid = pCMSGrid->CMSGetBaseClass();
				//reset defaults variables
				if (m_bGridOnly)
					i = 0;
				else
					i++;
				bStart = false;	//s
				bEnd = false;
				bInstert = false;
				bUpdate = false;
				//bDoAllRows=false;
				CGrdMapColumnInfo& pMap = pCMSGrid->CMSGetMapColumnInfo();	//	type cast cms to cms grid map
				GRDCOLPROPS& pProps = pCMSGrid->CMSGetColProps();				//	type cast to cms grid struct
				CGrdMapDataToText& pMapData = pCMSGrid->CMSGetMapDataToText();//	type cast pMap to DataToText map info
				GRDDATATOTEXT& pDataToText = pCMSGrid->CMSGetDataToText();	//	type cast pMap to DataToText  struct
				if (pMap.Lookup(_T("0"), pProps) != TRUE)
				{																//	lookup control in the map by map ID and element name AES
				}
				nRows = clsGrid.GetNumberRows();		//	get number of row and columns
				nCol = clsGrid.GetNumberCols();
				for (int r = 0; r < nRows; r++)
				{
					//******* Get the cell
					clsGrid.GetCell(0, r, &cell);
					//****** Get the cell's text

					// DTG, 8/26/2003, 1-7471, flag not set in last iteration, apparently.
					bEnd = (r == nRows - 1);
					szValue = "";											//this value needed to be reset AES 11.10.03
					cell.GetText(&szValue);
					if ((szValue != "0") && (szValue != _T("")))
					{														//	check dirty column value to see if the row is dirty
						if (i == 0)
							szi = "";
						else
							szi.Format(_T("%d"), i);
						//added to remove all detail with out adding them back AES 1-20-04
						if ((nType == RECORD_UPDATE) && (szValue == "9"))
						{
							//Added using xmlparms AES 5.26.04
							MakeNode("DELETE", "", 1, XML_STARTELEMENT);	//	format nodes for XML delete detail
							MakeNode("TABLE" + szi, "", 2, XML_SHORTNODE);	//	short node for table
							MakeNode("DELETE", "", 1, XML_ENDELEMENT);
							//clsDeleteXMLParams.SetRoot("DELETE") ;				//	format nodes for XML delete detail
							//clsDeleteXMLParams.MakeParam("TABLE" + szi,"");		//make node for the table to delete on the insert
							//clsFormatXMLParams.AppendXMLParam(&clsDeleteXMLParams);
							break;
						}
						// 1-7844, DTG, Delete TABLE1 and no insert.
						if ((nType == RECORD_UPDATE) && (szValue == "3") && (!bStart) && (r == 0))
						{
							bInstert = true;
							//Added using xmlparms AES 5.26.04
							//MakeNode("DELETE", "", 1, XML_STARTELEMENT);	//	format nodes for XML delete detail
							//MakeNode("TABLE" + szi, "", 2, XML_SHORTNODE);	//	short node for table
							//MakeNode("DELETE", "", 1, XML_ENDELEMENT);
							clsDeleteXMLParams.SetRoot("DELETE");				//	format nodes for XML delete detail
							clsDeleteXMLParams.MakeParam("TABLE" + szi, "");		//make node for the table to delete on the insert
							//clsFormatXMLParams.AppendXMLParam(&clsDeleteXMLParams);

							bStart = true;									//	set start to true so this will not happen again until new grid
							bEnd = false;									//	not ready to end
							bDoAllRows = false;								//	now do all the rows
							bInstert = false;
						}
						if ((nType == RECORD_UPDATE) && (szValue == "1") && (!bStart) && (r == 0))
						{
							//type is update and value = insert detail and first row, delete all detail
							//and insert all detail items
							bInstert = true;
							//Added using xmlparms AES 5.26.04
							//MakeNode("DELETE", "", 1, XML_STARTELEMENT);	//	format nodes for XML delete detail
							//MakeNode("TABLE" + szi, "", 2, XML_SHORTNODE);	//	short node for table
							//MakeNode("DELETE", "", 1, XML_ENDELEMENT);
							clsDeleteXMLParams.SetRoot("DELETE");				//	format nodes for XML delete detail
							clsDeleteXMLParams.MakeParam("TABLE" + szi, "");		//make node for the table to delete on the insert
							//clsFormatXMLParams.AppendXMLParam(&clsDeleteXMLParams);
							//MakeNode("INSERT", "", 1, XML_STARTELEMENT);
							clsInsertXMLParams.SetRoot("INSERT");			//make insert param

							bStart = true;									//	set start to true so this will not happen again until new grid
							bEnd = false;									//	not ready to end
							bDoAllRows = true;								//	now do all the rows
						}
						else if ((nType == RECORD_UPDATE) && (szValue == "1") && (!bStart) && (r != 0) && bDoAllRows == false)
						{
							bInstert = true;								//	set to true this any insert only
							//Added using xmlparms AES 5.26.04
							//MakeNode("INSERT", "", 1, XML_STARTELEMENT);
							clsInsertXMLParams.SetRoot("INSERT");			//make insert param

							bStart = true;									//	same as above
							bEnd = false;
						}
						else if (((nType == RECORD_UPDATE) && (szValue == "2") && (!bStart)) && (bDoAllRows == false) && (!m_bGridOnly))
						{
							bUpdate = true;								//	set to true this is a update only
							//Added using xmlparms AES 5.26.04
							//MakeNode("UPDATE", "", 1, XML_STARTELEMENT);
							clsUpdateXMLParams.SetRoot("UPDATE");		//make update param

							bStart = true;								//	same as above
							bEnd = false;
						}
						else if ((nType == RECORD_INSERT) && (szValue == "1") && (!bStart) && (r == 0) && bDoAllRows == false)
						{
							bInstert = true;								//	set to true this any insert only
							//Added using xmlparms AES 5.26.04
							//MakeNode("INSERT", "", 1, XML_STARTELEMENT);
							clsInsertXMLParams.SetRoot("INSERT");			//make insert param

							bStart = true;									//	same as above
							bEnd = false;
						}
						//Added using xmlparms AES 5.26.04
						//MakeNode("TABLE" + szi, "", 2, XML_STARTELEMENT);	//	add detail table number
						clsTableXMLParams.SetRoot("TABLE" + szi);			//make table param

						for (int c = 1; c < nCol; c++)
						{												//	loop though the columns for data
							szc.Format(_T("%d"), c);
							pMap.Lookup((_bstr_t)szc, pProps);					//	do map lookup for properties
							//pProps.bSkipGetGridData = true;
							if (pProps.bSkipGetGridData <= 0)
							{
								clsGrid.GetCell(c, r, &cell);	//type cast to cms grid to get cell
								if (pProps.nDataType == UGCELLDATA_CURRENCY)
								{										//	check for data type
									szValue = "";
									cell.GetText(&szValue);				//	get value for cell, currency must remove $
									////////////////////////////////////////////////////////////////
									// DG 1-3603/04, 5/9/2003, This section handles negative numbers.
									if (!szValue.Find(_T("(")))
									{
										// for negative numbers.
										CString szNegSign = "-";
										szValue.Remove('(');
										szValue.Remove(')');
										szValue.Replace(g_pGBLSystemInformationCMSDll->Localization.CurrencySymbol, _T(""));
										szValue.Replace(g_pGBLSystemInformationCMSDll->Localization.CurrencyGroupSeparator, _T(""));
										szValue.Replace(g_pGBLSystemInformationCMSDll->Localization.CurrencyDecimalSeparator, _T("."));
										if (szValue != ".00")
										{
											szNegSign += szValue;
											szValue = szNegSign;
										}

									}///////////////////////////////////////////////////////////////
									else
									{
										// for positive numbers.
										szValue.Replace(g_pGBLSystemInformationCMSDll->Localization.CurrencySymbol, _T(""));
										szValue.Replace(g_pGBLSystemInformationCMSDll->Localization.CurrencyGroupSeparator, _T(""));
										szValue.Replace(g_pGBLSystemInformationCMSDll->Localization.CurrencyDecimalSeparator, _T("."));
									}
								}
								else if (pProps.nDataType == UGCELLDATA_NUMBER)
								{
									szValue = "";
									cell.GetText(&szValue);				//	get value for cell, currency must remove
									szValue.Replace(g_pGBLSystemInformationCMSDll->Localization.CurrencyGroupSeparator, _T(""));
									szValue.Replace(g_pGBLSystemInformationCMSDll->Localization.CurrencyDecimalSeparator, _T("."));
									szValue.Replace(g_pGBLSystemInformationCMSDll->Localization.PercentSymbol, _T(""));	// remove the percent sign no matter what
								}
								else if (pProps.nDataType == UGCELLDATA_BOOL)
								{
									int nV = cell.GetBool();
									szValue.Format(_T("%d"), nV);
								}
								else
								{
									szValue = "";
									cell.GetText(&szValue);				// just a char type get text
									// translate what is in the cell to something the database can understand
									// based on the Translate flag
									// from debit/credit -> true/false
									switch (pProps.nTranslateType)
									{
									case GRD_XLAT_NOTHING:
										break;
									case GRD_XLAT_DEBITCREDIT:					// from debit/credit -> true/false
										CGBLForm::TranslateDebitCredit(szValue, false);		// direction relative to the client (true=to the grid, false=from the grid)
										break;
									case GRD_XLAT_ACCTCLASSIF:					// nothing at this time
										//CGBLForm::TranslateAcctClassif(szValue, false);		// direction relative to the client (true=to the grid, false=from the grid)
										break;
									case GRD_XLAT_ENCUMBCHECK:					// nothing at this time
										//CGBLForm::TranslateEncumber(szValue, false);		// direction relative to the client (true=to the grid, false=from the grid)
										CGBLForm::TranslateDebitCredit(szValue, false);
										break;
									case GRD_XLAT_ACCOUNTNMBR:					// strip segment separators to the account number
										CGBLForm::TranslateAcctNumber(szValue, false);		// direction relative to the client (true=to the grid, false=from the grid)
										break;
									case GRD_XLAT_MORELESS:					// from less/more to true/false
										CGBLForm::TranslateMoreLess(szValue, false);		// direction relative to the client (true=to the grid, false=from the grid)
										break;
									case GRD_DATATOTEXT:
										POSITION posData = pMapData.GetStartPosition();       // from use data to text struct for values
										while (posData != NULL)
										{
											pMapData.GetNextAssoc(posData, szIDData, pDataToText);	// direction relative to the client (true=to the grid, false=from the grid)
											int nLength = pProps.szID.GetLength();					//get length to know want to look for
											if ((pDataToText.szText == szValue) && (pProps.szID == szIDData.Left(nLength)))
												szValue = pDataToText.szData;
										}
										break;
									}
								}

								// DTG, 5.3.2004, eat white.
								pProps.szField.Trim();

								// DTG, 5.3.2004, Changed to use IsEmpty() method.
								if (!pProps.szField.IsEmpty())
								{
									//MakeNode(pProps.szField, szValue, 3, XML_ATTRIBUTE);	//set node and inner
									clsTableXMLParams.MakeParam(pProps.szField, szValue);
								}

							}
						}
						//not needed now AES
						//MakeNode("TABLE" + szi, "", 2, XML_ENDELEMENT);		//end table and number
						if ((nType == RECORD_UPDATE) && bInstert)
							clsInsertXMLParams.AppendXMLParam(&clsTableXMLParams);		//add tables data to insert
						else if ((nType == RECORD_UPDATE) && bUpdate)
							clsUpdateXMLParams.AppendXMLParam(&clsTableXMLParams);		//add tables data to insert
						else if (nType == RECORD_INSERT)
							clsInsertXMLParams.AppendXMLParam(&clsTableXMLParams);		//add tables data to insert
						// EK - 07/16/04 - Append grid values to xml data for Special Process types (see AP_GenerateRecurring)
						else if (nType == RECORD_PROCESS)
							m_szXMLFormat.Append(clsTableXMLParams.GetXML());
					}
					else if (szValue == "")		//	if dirty column is blank no more data to get
						bEnd = true;			//	set end to finish XML format
					//PGP(12-02-2007) As part of fixing 1-28723. Most entry grids have an extra row at the end
					//which will have dirty col value as empty. So they function correctly. For a grid with fixed
					//rows (like the ap-1099 grid) and data in all rows, changing just the last row ends up not
					//creating the update xml. This is to fix that.
					bEnd = (r == nRows - 1);
					if (((nType == RECORD_UPDATE) && bInstert && bEnd) || (bDoAllRows == true && bEnd))
					{
						//MakeNode("INSERT", "", 1, XML_ENDELEMENT);				//finish insert and reset variables
						//clsFormatXMLParams.AppendXMLParam(&clsInsertXMLParams);		//add insert date to main
						//CString test = clsFormatXMLParams.GetXML();
						m_szXMLFormat.Append(clsDeleteXMLParams.GetXML());	//add date to format string
						m_szXMLFormat.Append(clsInsertXMLParams.GetXML());	//add date to format string
						bEnd = false;
						bStart = false;
						bInstert = false;
						break;
					}
					else if ((nType == RECORD_UPDATE) && bUpdate && bEnd && (!m_bGridOnly))
					{
						//MakeNode("UPDATE", "", 1, XML_ENDELEMENT);		//finish update and reset variables
						//clsFormatXMLParams.AppendXMLParam(&clsUpdateXMLParams);		//add Update date to main
						CString test = clsUpdateXMLParams.GetXML();
						m_szXMLFormat.Append(clsDeleteXMLParams.GetXML());	//add date to format string
						m_szXMLFormat.Append(clsUpdateXMLParams.GetXML());	//add date to format string
						bEnd = false;
						bStart = false;
						bUpdate = false;
						break;
					}
					else if ((nType == RECORD_INSERT) && bEnd && (!m_bGridOnly))
					{
						//CString test = clsInsertXMLParams.GetXML();
						m_szXMLFormat.Append(clsInsertXMLParams.GetXML());	//add date to format string
						bEnd = false;
						bStart = false;
						bUpdate = false;
						break;

					}
				}//end for r
			}
		}
	}
	TRACE("Tick = %d\n", GetTickCount() - dwTick);
}

//****************************************************************
//FillGridWithData - This function is used to get data set from the list
//					to the XML
//	nType - int		Update and Insert type used, for multiple types during update
//****************************************************************
void CXML::GetListData(enuRecordProc nType)
{
	int				i = m_intTableCount;
	CString			szTagName(_T(""));
	CString			szValue(_T(""));
	CString			szi(_T(""));
	CString			szID(_T(""));
	const CString	szNothing(_T(""));

	const CCMSListCtrl* pclsList = NULL;		//declared const to make sure nothing in the control is changed

	GROUPTOCTRLSMAP& clsAllCtrlsMap = m_clsMapCtrlGroups;
	GROUPTOCTRLSMAP::iterator itGroup;
	KEYTOELEMENTMAP::iterator itKeys;
	for (itGroup = clsAllCtrlsMap.begin(); itGroup != clsAllCtrlsMap.end(); itGroup++)
	{													//	set the cells header text and size
		for (itKeys = itGroup->second.begin(); itKeys != itGroup->second.end(); itKeys++)
		{
			m_pMyElement = itKeys->second;
			if (m_pMyElement.nType == LIST_TYPE										//IsA CListCtrl?
				&& m_pMyElement.pnVal == READWRITE									//Is editable?
				&& m_pMyElement.pObjVal->IsKindOf(RUNTIME_CLASS(CCMSListCtrl)))		//IsA CCMSListCtrl?
			{																		//okay, let's archive this baby!
				pclsList = reinterpret_cast<const CCMSListCtrl*>(m_pMyElement.pObjVal);

				CCMSListCtrl::DBColumnDataConstIterator	pDBColDataIter = pclsList->GetDBColumnDataEnd();
				int	nRows = pclsList->GetItemCount();
				//int	nCol	= pclsList->GetColumnCount();

				//reset default variables
				i = (m_bGridOnly ? 0 : i + 1);
				szi.Format(_T("%d"), i);	//table number

				if (nType == RECORD_UPDATE)
				{
					//type is update, delete all detail and insert all detail items
					MakeNode(_T("DELETE"), szNothing, 1, XML_STARTELEMENT);			//	format nodes for XML delete detail
					MakeNode(_T("TABLE") + szi, szNothing, 2, XML_SHORTNODE);			//	short node for table
					MakeNode(_T("DELETE"), szNothing, 1, XML_ENDELEMENT);
					MakeNode(_T("INSERT"), szNothing, 1, XML_STARTELEMENT);
				}

				for (int r = 0; r < nRows; r++)
				{
					MakeNode(_T("TABLE") + szi, szNothing, 2, XML_STARTELEMENT);		//	add detail table number

					//	build xml by looping though the columns using the class' data column map
					for (pDBColDataIter = const_cast<CCMSListCtrl*>(pclsList)->GetDBColumnDataBegin(); pDBColDataIter != pclsList->GetDBColumnDataEnd(); pDBColDataIter++)
					{
						// *pDBColDataIter is an instance of DBColumnData (or std::pair)
						szTagName = pDBColDataIter->first;														//first is the table column name

						//dirty column; skip it.
						if (szTagName == _T("DIRTY")) continue;

						szValue = const_cast<CCMSListCtrl*>(pclsList)->GetDataInDBFormat(r, pDBColDataIter->second);	//second is the column index
						MakeNode(szTagName, szValue, 3, XML_ATTRIBUTE);			//	set node and inner
					}

					MakeNode(_T("TABLE") + szi, szNothing, 2, XML_ENDELEMENT);			//	end table and number
				}	//end for r

				if ((nType == RECORD_UPDATE))
				{
					MakeNode(_T("INSERT"), szNothing, 1, XML_ENDELEMENT);			//	finish insert
				}
			}
		}
	}
}

//****************************************************************
//BtnSave - Routine to handle the coding behind the edit button
//	nMode - int,			The current form mode.  If it is
//							MODE_EDIT then the current record
//							will be deleted.
//	szModule - CString,		The 2 letter module code
//	szForm - CString,		The name of the form the data is for
//	*strCommandValues -		CStringArray, XML command set Array
//
//****************************************************************
bool CXML::BtnSave(
	enmFormModes nMode,
	CString szModule,
	CString szForm,
	CStringArray* strCommandValues,
	bool bUnlock,
	bool bUpdateOnly)
{
	if (nMode == MODE_ADD)			//	Insert record here AES
	{
		if (!ChangeXMLData(RECORD_INSERT, szModule, szForm, strCommandValues)) return false; //not OK
	}
	else if (nMode == MODE_EDIT)
	{								//	Update record here AES
		// EK - 5/27/3 - Pass the bUpdateOnly parameter
		if (!ChangeXMLData(RECORD_UPDATE, szModule, szForm, strCommandValues, CTY_COMPANYSERVER, false, bUpdateOnly)) return false;	//not OK

		if (bUnlock)
		{
			UINT uinConnectionType = CTY_COMPANYSERVER;
			if (m_bUseDefinedServer) uinConnectionType = m_uinDefinedServer;
			if (!LockRecord(RECORD_UNLOCK, szModule, szForm, strCommandValues, CXML::GetServerType(uinConnectionType)))
			{						//	unlock after update
				//	TODO: error message here
			}
		}
	}
	return true;					//	save data complete and OK
}

//****************************************************************
//BtnNextPrevious - Routine to handle the coding behind the edit button
//	nMode - int,			The current form mode.  If it is
//							MODE_EDIT then the current record
//							will be deleted.
//	bIsDirty - int,			dirty flag state
//	int nDirection			Direction to get the data
//	szModule - CString,		The 2 letter module code
//	szForm - CString,		The name of the form the data is for
//	*strCommandValues - CStringArray, XML command set Array
//	szControlID - CString,	Map ID for control to fill
//	szXMLTable - CString,	XML Table name
//****************************************************************
enmFormModes CXML::BtnNextPrevious(
	enmFormModes nMode,
	bool bIsDirty,
	int nDirection,
	CString szModule,
	CString szForm,
	CStringArray* strCommandValues,
	CString szControlID,
	CString szXMLTable)
{
	//CCMSWaitCursor c;
	if (nMode != MODE_VIEW)
	{										//	edit or add mode must check for dirty
											//	Set controls on the form
		if (!bIsDirty)
		{									//	check for dirty record and mode
			if (nMode == MODE_EDIT)
				if (m_szGUID != "")
				{
					UINT uinConnectionType = CTY_COMPANYSERVER;
					if (m_bUseDefinedServer) uinConnectionType = m_uinDefinedServer;
					if (!LockRecord(RECORD_UNLOCK, szModule, szForm, strCommandValues))
					{						//	current record is locked so unlock it.
						//	TODO: error message here
					}
				}
			CCMSWaitCursor clsWaitCursor(WM_RETRIEVING_RECORD);		//used to show status dialog with avi and label message AES 6.06.04
			ClearControls("ALL");			//JKC - 10/1/2003 Changed to ALL to clear all controls
			if (GetXMLData(nDirection, szModule, szForm, strCommandValues, szControlID, szXMLTable))
				return MODE_VIEW;			//	good get set to view mode
			else
			{
				//DefectID- 1-4126 PGP(04/23/2003)
				// DG 1-5022, 5/8/2003
				clsWaitCursor.CancelStatusDlg();						//used to cancel status dial0g AES 6.06.04
				CGBLForm::CMSMessageBox(g_pGBLSystemInformationCMSDll->ApplicationMainFrame, IDS_NORECORDSMSG, IDS_NORECORDS, MB_OK | MB_ICONEXCLAMATION);
				return MODE_ADD;			//	bad get or no data set to add mode
			}
		}
		else
		{									//	record was dirty so ask the ?
			// get the title of the active view.
			CView* pView = /*(CFormView*)*/CGBLForm::EkGetActiveView();
			CWnd* pWnd = pView->GetParentFrame();
			pWnd->GetWindowText(m_szFormTitle);

			m_nAnswer = CGBLForm::CMSMessageBox(g_pGBLSystemInformationCMSDll->ApplicationMainFrame, IDS_SAVECHANGES, 0, MB_YESNOCANCEL + MB_ICONQUESTION, "", m_szFormTitle, 0, false);
			if (m_nAnswer == IDYES)					//	Save changed data AES
			{
				if (nMode == MODE_ADD)
					return MODE_ADD_SAVING;			//	check the set array index in the current data set
				else if (nMode == MODE_EDIT)
					return MODE_EDIT_SAVING;		//	check the set array index in the current data set
			}
			else if (m_nAnswer == IDNO)				//	Do not Save changed data AES
			{
				if (nMode == MODE_EDIT)
					// EK - 5/29/3 - Defect ID 1-5475 - Only unlock if GUID is not empty
					// This gives us the possibility to skip the unlock function
					if (m_szGUID != "")
					{
						UINT uinConnectionType = CTY_COMPANYSERVER;
						if (m_bUseDefinedServer) uinConnectionType = m_uinDefinedServer;
						if (!LockRecord(RECORD_UNLOCK, szModule, szForm, strCommandValues, CXML::GetServerType(uinConnectionType)))
						{						//	unlock current record
							//TODO: error message here
						}
					}
				CCMSWaitCursor clsWaitCursor(WM_RETRIEVING_RECORD);		//used to show status dialog with avi and label message AES 6.06.04
				ClearControls("ALL");		//JKC - 10/1/2003 Changed to ALL to clear all controls
				if (GetXMLData(nDirection, szModule, szForm, strCommandValues, szControlID, szXMLTable))
				{							//	data by direction
					return MODE_VIEW;		//	good get set to view mode
				}
				else
				{
					//DefectID- 1-4126 PGP(04/23/2003)
					// DG 1-5022, 5/8/2003
					clsWaitCursor.CancelStatusDlg();		//used to cancel status dialog AES 6.06.04
					CGBLForm::CMSMessageBox(g_pGBLSystemInformationCMSDll->ApplicationMainFrame, IDS_NORECORDSMSG, IDS_NORECORDS, MB_OK | MB_ICONEXCLAMATION);
					return MODE_ADD;		//	bad get or no data set to add mode
				}
			}
			else if (m_nAnswer == IDCANCEL)	//	Cancel Previous changed data AES
				return MODE_NONE;			//	you has canceled do nothing just checking value AES
		}
	}
	else
	{										//	view mode
		//UpdateData();
		CCMSWaitCursor clsWaitCursor(WM_RETRIEVING_RECORD);		//used to show status dialog with avi and label message AES 6.06.04
		ClearControls("ALL");				//JKC - 10/1/2003 Changed to ALL to clear all controls
		if (!GetXMLData(nDirection, szModule, szForm, strCommandValues, szControlID, szXMLTable))
		{
			//DefectID- 1-4126 PGP(04/23/2003)
			// DG 1-5022, 5/8/2003
			clsWaitCursor.CancelStatusDlg();			//used to cancel status dialog AES 6.06.04
			CGBLForm::CMSMessageBox(g_pGBLSystemInformationCMSDll->ApplicationMainFrame, IDS_NORECORDSMSG, IDS_NORECORDS, MB_OK | MB_ICONEXCLAMATION);
			return MODE_ADD;
		}
		clsWaitCursor.CancelStatusDlg();			//used to cancel status dialog AES 6.06.04
	}
	//c.Restore();
	return nMode;							//	return the current state that the form need to be in.
}

//****************************************************************
//BtnEdit - Routine to handle the coding behind the edit button
//	nMode - int,			The current form mode.  If it is
//							MODE_EDIT then the current record
//							will be deleted.
//	szModule - CString,		The 2 letter module code
//	szForm - CString,		The name of the form the data is for
//	*strCommandValues - CStringArray, XML command set Array
//	szControlID - CString,	Map ID for control to fill
//	szXMLTable - CString,	XML Table name
//****************************************************************
enmFormModes CXML::BtnEdit(
	enmFormModes nMode,
	CString szModule,
	CString szForm,
	CStringArray* strCommandValues,
	CString szControlID,
	CString szXMLTable)
{
	CCMSWaitCursor clsWait;
	if (nMode == MODE_EDIT)
	{										//	mode is edit button is delete, verify the user want this function.
		// DG 1-5022, 5/8/2003
		m_nAnswer = CGBLForm::CMSMessageBox(g_pGBLSystemInformationCMSDll->ApplicationMainFrame, IDS_CONFIRM_DELETE_RECORD, IDS_DEL_SETUPFUNDCAPTION, MB_YESNO + MB_ICONQUESTION);

		if (m_nAnswer == IDYES)
		{
			// BK 5/13/03 delete record before unlocking it (moved this code below)
			// if(!LockRecord(RECORD_UNLOCK, szModule,  szForm, strCommandValues))
			// {
			//			//TODO: error message here
			// }
			if (ChangeXMLData(RECORD_DELETE, szModule, szForm, strCommandValues))
			{
				// BK 9/27/06 Version 2.5 Feature 8 Print Immediate from Enter Bills send message after a record is deleted
				NotifyRecordDeleted();

				// BK 5/13/03 delete record before unlocking it (moved this code from above)
				UINT uinConnectionType = CTY_COMPANYSERVER;
				if (m_bUseDefinedServer) uinConnectionType = m_uinDefinedServer;
				if (!LockRecord(RECORD_UNLOCK, szModule, szForm, strCommandValues, CXML::GetServerType(uinConnectionType)))
				{

				}
				// InTimeTec: 08/26/2013 : HS :Bug 8417 : D_AR - Credit Card Codes form does not clear to blank form when deleting an existing record - goes to the next record
				if (m_bClearAllControls)
					return MODE_ADD;
				// InTimeTec: End
				CCMSWaitCursor clsWaitCursor(WM_RETRIEVING_NEXT_RECORD);		//used to show status dialog with avi and label message AES 6.06.04
				ClearControls(szControlID); //HAE  Clear the controls before getting next record.
				if (GetXMLData(RECORD_NEXT, szModule, szForm, strCommandValues, szControlID, szXMLTable))
				{
					clsWait.Restore();
					return MODE_VIEW;
				}
				else
				{
					clsWaitCursor.CancelStatusDlg();				//used to cancel status dialog AES 6.06.04
					clsWait.Restore();
					return MODE_ADD;
				}
			}
			if (m_nMode == MODE_ERROR_RPT)
				nMode = MODE_ERROR_RPT;
		}
	}
	else
	{
		UINT uinConnectionType = CTY_COMPANYSERVER;
		if (m_bUseDefinedServer) uinConnectionType = m_uinDefinedServer;
		if (!LockRecord(RECORD_LOCK, szModule, szForm, strCommandValues, CXML::GetServerType(uinConnectionType)))
			return MODE_VIEW;		//record is lock stay in view mode
		else
		{
			//used to show status dialog with avi and label message AES 6.06.04
			//UT-2562, PGP(06/30/2004) - Moved here so that refreshing record avi
			//if kicks in, would not overlap with lock record status.
			CCMSWaitCursor clsWaitCursor(WM_REFRESHING_RECORD);
			GetXMLData(RECORD_CURRENT, szModule, szForm, strCommandValues, szControlID, szXMLTable);
			clsWaitCursor.CancelStatusDlg();				//used to cancel status dialog AES 6.06.04
			clsWait.Restore();
			return MODE_EDIT;		//record is lock by use set edit mode
		}
	}
	clsWait.Restore();
	return nMode;
}

//****************************************************************
//MySetElement - Set control map struct in the XML with controls
//CString szTable			Control id
//CString strKey			SQL field name to match with control
//CObject * pObjVal		pointer to object in map
//int pnVal				Use currently for grid read write properties
//int nType 				Type of control in map
//****************************************************************
void CXML::MySetElement(
	CString szTable,
	CString strKey,
	CObject* pObjVal,
	int pnVal,
	enuControlType nType,
	bool bValXML,
	bool bClearAfterSave)	// InTimeTec: 11/21/2013: HS : Defect 9074 : D_IN_Save IN Stock Information from the Stock Item Maintenance Screen When Creating New Item
{
	//set passed in to map AES
	//mapelement.szTable	= szTable;
	//mapelement.szKey	= strKey;
	mapElement.pnVal = pnVal;
	mapElement.pObjVal = pObjVal;
	mapElement.nType = nType;
	mapElement.bValXML = bValXML;
	// InTimeTec: 11/21/2013: HS : Defect 9074 : D_IN_Save IN Stock Information from the Stock Item Maintenance Screen When Creating New Item
	// Added the flag to prevent controls from clearing out
	mapElement.bClearOnSave = bClearAfterSave;

	KEYTOELEMENTMAP* pMapElements = NULL;
	GetCtrlMapForGroup(szTable, pMapElements);
	if (!pMapElements)
	{
		KEYTOELEMENTMAP clsKeyToElementMap;
		clsKeyToElementMap[strKey] = mapElement;
		m_clsMapCtrlGroups[szTable] = clsKeyToElementMap;
	}
	else
		pMapElements->operator [](strKey) = mapElement;
}

// BK 10/12/07 original LockBatchExclusive() was hard-coded for SA module - added this so can be used with PO also
CString CXML::GetBatchTable()
{
	return CMS::GetSI()->ApplicationSettings.ModuleStringID + _T("_BATCHES");
}

CString CXML::LockBatchExclusive(CString szBatchCode)
{
	// BK 10/12/07 originally hard-coded for SA module - added this so can be used with PO also
	CString szBatchType;
	szBatchType.Format(_T("%d"), CMS::GetSI()->CurrentBatch.BatchType);
	CString szModule = CMS::GetSI()->ApplicationSettings.ModuleStringID;

	CStringArray strCommandValues;
	strCommandValues.Add(_T("strLocation"));
	strCommandValues.Add(CMS::GetSI()->CompanySettings.DefaultLocation);
	strCommandValues.Add(_T("strBatchCode"));
	if (szBatchCode == _T(""))
		strCommandValues.Add(CMS::GetSI()->CurrentBatch.BatchCode);
	else
		strCommandValues.Add(szBatchCode);
	strCommandValues.Add(_T("bytBatchType"));
	strCommandValues.Add(szBatchType);
	CString szReturn;
	szReturn = _T("");
	LockRecord(RECORD_LOCK, szModule, GetBatchTable(), &strCommandValues, CXMLParams::CN_COMPANY, CXML::EXCLUSIVE, &szReturn, CMS::GetSI()->CurrentBatch.szGUID); // begbert 02-15-2006 VS8: warning C4482
	return szReturn;
}

CString CXML::UnlockBatchExclusive(CString szBatchCode)
{
	// BK 10/12/07 originally hard-coded for SA module - added this so can be used with PO also
	CString szBatchType;
	szBatchType.Format(_T("%d"), CMS::GetSI()->CurrentBatch.BatchType);
	CString szModule = CMS::GetSI()->ApplicationSettings.ModuleStringID;

	CStringArray strCommandValues;
	strCommandValues.Add(_T("strLocation"));
	strCommandValues.Add(g_pGBLSystemInformationCMSDll->CompanySettings.DefaultLocation);
	strCommandValues.Add(_T("strBatchCode"));
	if (szBatchCode == _T(""))
		strCommandValues.Add(g_pGBLSystemInformationCMSDll->CurrentBatch.BatchCode);
	else
		strCommandValues.Add(szBatchCode);
	strCommandValues.Add(_T("bytBatchType"));
	strCommandValues.Add(szBatchType);
	CString szReturn;
	szReturn = _T("");
	LockRecord(RECORD_LOCK, szModule, GetBatchTable(), &strCommandValues, CXMLParams::CN_COMPANY, CXML::EXCLUSIVEUNLOCK, &szReturn, L""); //g_pGBLSystemInformationCMSDll->CurrentBatch.szGUID); // begbert 02-15-2006 VS8: warning C4482
	return szReturn;
}

CString CXML::UnlockBatchPendingExclusive(CString szBatchCode)
{
	// BK 10/12/07 originally hard-coded for SA module - added this so can be used with PO also
	CString szBatchType;
	szBatchType.Format(_T("%d"), CMS::GetSI()->CurrentBatch.BatchType);
	CString szModule = CMS::GetSI()->ApplicationSettings.ModuleStringID;

	CStringArray strCommandValues;
	strCommandValues.Add(_T("strLocation"));
	strCommandValues.Add(CMS::GetSI()->CompanySettings.DefaultLocation);
	strCommandValues.Add(_T("strBatchCode"));
	if (szBatchCode == _T(""))
		strCommandValues.Add(CMS::GetSI()->CurrentBatch.BatchCode);
	else
		strCommandValues.Add(szBatchCode);
	strCommandValues.Add(_T("bytBatchType"));
	strCommandValues.Add(szBatchType);
	CString szReturn;
	szReturn = _T("");
	LockRecord(RECORD_LOCK, szModule, GetBatchTable(), &strCommandValues, CXMLParams::CN_COMPANY, CXML::EXCLUSIVEPENDINGUNLOCK, &szReturn, CMS::GetSI()->CurrentBatch.szGUID); // begbert 02-15-2006 VS8: warning C4482
	return szReturn;
}

//Returns true if the batch was locked, false if the batch was not locked
bool CXML::OELockBatchOnStartup(CString szBatchCode)
{
	//If a real batch is passed in, then just lock it
	if (szBatchCode != _T(""))
	{
		CMS::GetSI()->CurrentBatch.BatchCode = szBatchCode.MakeUpper();
		bool bLocked;
		bLocked = CMS::GetSI()->CurrentBatch.LockBatch();
		if (!bLocked)
		{
			CMS::GetSI()->CurrentBatch.BatchCode = _T("");
			return false;
		}
	}
	else
		return false;

	//KPM.2005.03.24 It was decided that the only way to select a batch automatically is by selecting
	//order entry from the menu in order maintenance.
	return true;
}

//****************************************************************
//LockRecord - Routine to handle the coding behind the edit button
//	nType - int,			Type of lock to do
//	szModule - CString,		The 2 letter module code
//	szForm - CString,		The name of the form the data is for
//	*strCommandValues - CStringArray, XML command set Array
//****************************************************************
bool CXML::LockRecord(
	enuRecordProc nType,
	CString szModule,
	CString szForm,
	CStringArray* strCommandValues,
	CXMLParams::enmConnectionType nServer,
	CXML::enuExclusive uinExclusive,
	CString* szReturnPointer,
	CString szGUIDOverride)
{
	//Added to check if we are supposed to lock the record.
	if (m_bNoRecordLock) return true;

	CoInitialize(NULL);
	//VARIABLES THAT ARE NEEDED
	BSTR data;
	//long nLen = 0;
	CString	szCommand = _T("LOCKRECORD");
	CString	szCommandType = _T("");
	try
	{
		switch (nType)
		{
		case RECORD_LOCK:						//record is a lock type
		{
			//KPM.2005.07.20 Adding user info to each lock for easy access
			strCommandValues->Add("strUserName");
			strCommandValues->Add(CMS::GetSI()->UserInformation.Name);

			strCommandValues->Add("strDepartment");
			strCommandValues->Add(CMS::GetSI()->UserInformation.Department);

			strCommandValues->Add("strPhoneExt");
			strCommandValues->Add(CMS::GetSI()->UserInformation.Phone);

			strCommandValues->Add("strTenantID");
			strCommandValues->Add(CMS::GetSI()->ApplicationSettings.TenantID);

			bool bUnLocked = true;
			//KPM.2005.03.24 Don't unlock sa messages
			if (szForm != szModule + _T("_BATCHES"))
			{
				if (m_szGUID != "") bUnLocked = LockRecord(RECORD_UNLOCK, szModule, szForm, strCommandValues, nServer);
			}

			if (bUnLocked)
			{
				szCommandType = "LOCK";					//set lock
				m_szGUID = CGBLMath::GenerateGUID();	//set guid
				//KPM, added code to handle special functionality for oe/sm record locking
				CXMLDocument doc;
				CXMLElement ele = doc.SetRoot(_T("LockInfo"));
				// BK 10/15/07 added PO_BATCHES to allow exclusive batch lock in PO also
				if (szForm == szModule + _T("_BATCHES"))
				{
					ele.AddNewChild(_T("MachineName"), CGBLEnvironment::GetMachineName());
					ele.AddNewChild(_T("UserName"), CMS::GetSI()->UserInformation.Name);
					ele.AddNewChild(_T("UserID"), CMS::GetSI()->UserInformation.ID);
					ele.AddNewChild(_T("UserPhone"), CMS::GetSI()->UserInformation.Phone);
					ele.AddNewChild(_T("UserDepartment"), CMS::GetSI()->UserInformation.Department);
					ele.AddNewChild(_T("Exclusive"), uinExclusive);
					ele.AddNewChild(_T("BatchMode"), CMS::GetSI()->ApplicationSettings.MultiBatchMode);
					if (_T("SA") == szModule || _T("SE") == szModule)
					{
						if (CMS::GetSI()->ModuleSettings.SASettings.Module == IDS_OE_MODULE && CMS::GetSI()->CommandLineSettings.SA_JobCostMode && CMS::GetSI()->ModulesInstalled.JobCost.IsUsable)
							ele.AddNewChild(_T("MultiUserBatch"), 0);	// begbert 08-18-2015 PBI 29350 : If in OE Job Cost Mode then we're never in multiple users per batch mode.
						else
							ele.AddNewChild(_T("MultiUserBatch"), CMS::GetSI()->ModuleSettings.OESettings.bMultipleUsers);
					}
					else
						ele.AddNewChild(_T("MultiUserBatch"), CMS::GetSI()->ModuleSettings.CurrentSettings->bMultipleUsers);
					ele.AddNewChild(_T("Location"), strCommandValues->GetAt(1));
					ele.AddNewChild(_T("Batch"), strCommandValues->GetAt(3));
					ele.AddNewChild(_T("Session"), CMS::GetSI()->ApplicationSettings.SessionName);
					// 1-32267 BK 3/6/09 needed in PO and Sales when one user is trying to lock a batch to post, but others are in the batch so the status
					// is PendingExclusiveLock if another user tries to print receiving/edit reports they need to be Allow to get a non-exclusive lock
					ele.AddNewChild(_T("AllowWhilePendingExclusiveLock"), CMS::GetSI()->CurrentBatch.GetAllowNonExclusiveLock());

					if (CMS::GetSI()->ApplicationSettings.ModuleID == IDS_SAE_MODULE)
						ele.AddNewChild(_T("Module"), _T("SE"));
					else if (CMS::GetSI()->ApplicationSettings.ModuleID == IDS_POE_MODULE)
						ele.AddNewChild(_T("Module"), _T("PE"));
					else
						ele.AddNewChild(_T("Module"), CMS::GetSI()->ApplicationSettings.ModuleStringID);

					if (szGUIDOverride == _T(""))
						ele.AddNewChild(_T("GUID"), m_szGUID);
					else
						ele.AddNewChild(_T("GUID"), szGUIDOverride);
					//Override used for modifying a current lock in some cases
					strCommandValues->Add(_T("strAdvancedXML"));
					strCommandValues->Add(ele.GetOuterXML());
				}
			}
			else
				return false;
		}
		break;
		case RECORD_UNLOCK:						//unlock type
			szCommandType = "UNLOCK";			//set unlock
			break;
		}

		//Make the Command string
		MakeXMLCommand(szCommand, szCommandType, szModule, szForm);

		//make the connection string
		MakeXMLConnection("Connection", "10", "300", CXML::GetServerType(nServer));

		if (szCommandType == "LOCK")
		{
			//SET THE XML THE THE LOCK OF RECORD
			MakeNode("TABLE", "", 1, XML_STARTELEMENT);
			MakeNode("RECORD", "", 2, XML_STARTELEMENT);
			for (int ii = 0; ii < strCommandValues->GetSize(); ii += 2)		//GET THE KEY VALUE
				MakeNode(strCommandValues->GetAt(ii), strCommandValues->GetAt(ii + 1), 3, XML_ATTRIBUTE);
			MakeNode("RECORD", "", 2, XML_ENDELEMENT);
			MakeNode("TABLE", "", 1, XML_ENDELEMENT);
			if (CMS::GetSI()->UserInformation.ID != _T(""))
				MakeNode("USER", CMS::GetSI()->UserInformation.ID, 1, XML_ATTRIBUTE);
			else
				MakeNode("USER", "SUP", 1, XML_ATTRIBUTE);
			MakeNode("GUID", m_szGUID, 1, XML_ATTRIBUTE);
			MakeNode("APPGUID", CMS::GetSI()->ApplicationSettings.AppGUID, 1, XML_ATTRIBUTE);
			MakeNode("MACGUID", CMS::GetSI()->ApplicationSettings.MachineGUID, 1, XML_ATTRIBUTE);
			MakeNode("strUserName", CMS::GetSI()->UserInformation.Name, 1, XML_ATTRIBUTE);
			MakeNode("strDepartment", CMS::GetSI()->UserInformation.Department, 1, XML_ATTRIBUTE);
			MakeNode("strPhoneExt", CMS::GetSI()->UserInformation.Phone, 1, XML_ATTRIBUTE);
			MakeNode("strCompanyID", CMS::GetSI()->CompanySettings.ID, 1, XML_ATTRIBUTE);

			MakeCOMPANYINFO();
			GetXMLFormat();

			// 1-32267 BK 3/7/09 if m_szXMLSendData variable is empty get formatted string being sent to server as xmlData to lock record
			// begbert 05-07-2009 1-32566 : This can be carried over from call to call on
			//   a single CXML object and needs to be reset each time, thus we remove the
			//   conditional that was preventing that.
			//if (m_szXMLSendData.IsEmpty() && m_szXMLData.Find(_T("<CONNECTION>")) > 0)
			m_szXMLSendData = m_szXMLData;
		}
		else
		{
			MakeNode("GUID", m_szGUID, 1, XML_ATTRIBUTE);						//SET THE GUID FOR UNLOCK
			if (m_uinDefinedServer != CTY_PRIMARYSERVER)
				MakeCOMPANYINFO();
			GetXMLFormat();
		}

		//Xml pointer to use
		CCMSXMLDomNodeList::ComponentTypePtr pNodeList;					//	pointers used in XML  CCMSXMLDomElement
		CCMSXMLDomNode::ComponentTypePtr pNode;
		CCMSXMLDomNode::ComponentTypePtr pInnerNode;
		CCMSXMLDomNode::ComponentType* pElement = NULL;;
		/*MSXML2::IXMLDOMNodeListPtr	pNodeList;
		MSXML2::IXMLDOMNodePtr		pNode;
		MSXML2::IXMLDOMNodePtr		pInnerNode;
		MSXML2::IXMLDOMNode*		pElement = NULL;*/
		//MSXML2::IXMLDOMNode*		pFirst = NULL;
		_bstr_t szReturn = SendXML();
		m_docPtr->loadXML(szReturn);									//SEND COMMAND TO SERVER

		switch (nType)
		{																//CHECK RETURN HERE
		case RECORD_LOCK:
			pNodeList = m_docPtr->getElementsByTagName(_bstr_t("NUMBER"));//NUMBER IS THE TAG
			pNode = pNodeList->Getitem(0);
			if (pNode == NULL)
			{											//IF NULL IS FAILED
				CGBLForm::CMSMessageBox(CMS::GetSI()->ApplicationMainFrame, IDS_RECORD_LOCK_FAILED, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONERROR);
				m_szGUID = _T("");
				return false;
			}
			else
			{
				pNode->get_childNodes(&pNodeList);		//USE POINTER FOR TO LOOK AT THE RETURN
				pNodeList->get_item(0, &pElement);
				pElement->get_nodeName(&data);
				pElement->get_text(&data);
				CString szNode = data;
				if (szNode == "LOCK_1")											//COULD NOT FOUND RECORD
				{
					CGBLForm::CMSMessageBox(CMS::GetSI()->ApplicationMainFrame, IDS_RECORD_NOT_FOUND, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONERROR);
					m_szGUID = _T("");
					return false;
				}
				else if (szNode == "LOCK_2")
				{							//RECORD ALREADY LOCKED
					//UINT uiReturn;
					CStringArray strValues;

					//TODO: BK remove this code and put in FileLock
																			//OPEN RECORD LOCKING FORM
										//CString szXMLUserInfo="<USER><strUserName>John Smith</strUserName><strDepartment>Sales</strDepartment><strTitle>Supervisor</strTitle><strPhoneExt>322-1903</strPhoneExt><strLocation>Basement</strLocation></USER>";
					strValues.Add(_T("strUserID"));
					strValues.Add(CMS::GetSI()->UserInformation.ID);
					CXML* objXML = new CXML;
					objXML->GetXMLData(RECORD_CURRENT, "GB", "CMSUSERS", &strValues, "", "", CTY_PRIMARYSERVER);
					CString szReturn;
					TSM_LockingStatus uiStatus {};
					// 1-32267 BK 3/7/09 can't resend m_szXMLData because it may contain data returned from server, set up a new variable (m_szXMLSendData) to hold data to lock record
					//CFileLock clsFileLock(m_szXMLCommand, m_szXMLData, "", &szReturn, &uiStatus); //objXML->m_szXMLUserInfo);
					CFileLock clsFileLock(m_szXMLCommand, m_szXMLSendData, "", &szReturn, &uiStatus); //objXML->m_szXMLUserInfo);
					delete objXML;
					clsFileLock.DoModal();
					//KPM.2005.07.19 This is sometimes not passed in which causes errors.
					if (szReturnPointer)
						*szReturnPointer = szReturn;
					//Handle error/information conditions, PGP
					switch (uiStatus)
					{
					case TSM_RECORDLOCKED:
					{
						CMS::GetSI()->ClientLockRefresh.AddLockID(m_szGUID, nServer);
						return true;
					}
					break;
					case TSM_RECORDDELETED:
						CGBLForm::CMSMessageBox(CMS::GetSI()->ApplicationMainFrame, IDS_RECORD_DELETED_BY_ANOTHER_USER, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONERROR);
						break;
					case TSM_ERROROCCURRED:
						CGBLForm::CMSMessageBox(CMS::GetSI()->ApplicationMainFrame, IDS_ERR_LOCKING_RECORD, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONERROR);
						break;
					case TSM_USERCANCELLED:
						m_szGUID = _T("");//Defect ID 1-21018, PGP(08/19/2005)
						return false;
					}
					m_szGUID = _T("");
					return false;
				}
				//KPM, 3/1/2005, Special locking for OE
				else if (szNode == "LOCK_NOMATCH")
				{
					if (szReturnPointer != NULL)
						*szReturnPointer = szNode;
					return true;
				}
				else if (szNode == "LOCK_MATCHFOUND")
				{
					if (szReturnPointer != NULL)
					{
						pNodeList = m_docPtr->getElementsByTagName(_bstr_t("BATCH"));
						pNode = pNodeList->Getitem(0);
						pNode->get_childNodes(&pNodeList);
						pNodeList->get_item(0, &pElement);
						pElement->get_text(&data);
						*szReturnPointer = data;
					}
					return true;
				}
				else if (szNode == "LOCK_CROSSMODULELOCK")
				{
					CString szBatchCode;
					pNodeList = m_docPtr->getElementsByTagName(_bstr_t("BATCH"));//NUMBER IS THE TAG
					pNode = pNodeList->Getitem(0);
					pNode->get_childNodes(&pNodeList);		//USE POINTER FOR TO LOOK AT THE RETURN
					pNodeList->get_item(0, &pElement);
					pElement->get_text(&data);
					szBatchCode = data;

					CString szMessage;
					szMessage = CGBLResources::GetResourceString(IDS_GBL_CROSSMODULELOCK);
					szMessage.Replace(_T("<BATCHCODE>"), szBatchCode);
					if (CMS::GetSI()->ApplicationSettings.ModuleStringID == _T("SA"))
						szMessage.Replace(_T("<OPPOSINGMODULE>"), CGBLResources::GetResourceString(IDS_GBL_ORDERENTRY));
					else if (CMS::GetSI()->ApplicationSettings.ModuleStringID == _T("PO"))
						szMessage.Replace(_T("<OPPOSINGMODULE>"), CGBLResources::GetResourceString(IDS_GBL_PURCHASEENTRY));
					else if (CMS::GetSI()->ApplicationSettings.ModuleStringID == _T("SE"))
						szMessage.Replace(_T("<OPPOSINGMODULE>"), CGBLResources::GetResourceString(IDS_GBL_ORDERMAINTENANCE));
					else if (CMS::GetSI()->ApplicationSettings.ModuleStringID == _T("PE"))
						szMessage.Replace(_T("<OPPOSINGMODULE>"), CGBLResources::GetResourceString(IDS_GBL_PURCHASEMAINTENANCE));
					CGBLForm::CMSMessageBox(NULL, 0, 0, MB_OK, szMessage);
					return false;
				}
				else if (szNode == "LOCK_FAIL_EXCEEDUSAGE")
				{
					CString szMessage;
					szMessage = _T("You cannot lock the same batch in the same module on the same machine more than once.");
					CGBLForm::CMSMessageBox(NULL, 0, 0, MB_OK, szMessage);
					return false;
				}
				else if (szNode == "LOCK_FAIL_PENDINGEXCLUSIVE")
				{
					CString szMessage;
					szMessage = _T("This batch is pending an exclusive lock and therefore cannot be used at this time");
					CGBLForm::CMSMessageBox(NULL, 0, 0, MB_OK, szMessage);
					return false;
				}
				else if (szNode == "LOCK_FAIL_EXCLUSIVELOCK")
				{
					CString szMessage;
					szMessage = _T("This batch is exclusively locked and therefore cannot be used at this time");
					CGBLForm::CMSMessageBox(NULL, 0, 0, MB_OK, szMessage);
					return false;
				}
				else if (szNode == "LOCK_PENDINGLOCKLIST")
				{
					//Return the user list, the lock exclusive form will sort out the data
					CString szTemp(szReturn.GetBSTR());
					*szReturnPointer = szTemp;
					return true;
				}
				else if (szNode == "LOCK_EXCLUSIVELYLOCKED")
				{
					*szReturnPointer = szNode;
					return true;
				}
				else if (szNode == "LOCK_EXCLUSIVEUNLOCKED")
				{
					*szReturnPointer = szNode;
					return true;
				}
				else if (szNode == "LOCK_PENDINGUNLOCKED")
				{
					*szReturnPointer = szNode;
					return true;
				}
			}
			break;
		case RECORD_UNLOCK:												//UNLOCK RETURNS
			pNodeList = m_docPtr->getElementsByTagName(_bstr_t("RECORDSAFFECTED"));	//GOOD UNLOCK
			pNode = pNodeList->Getitem(0);
			if (pNode == NULL)														//UNLOCK FAILED
			{
				CGBLForm::CMSMessageBox(CMS::GetSI()->ApplicationMainFrame, IDS_RECORD_WAS_NOT_UNLOCKED, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONERROR);
				m_szGUID = _T("");
				return false;
			}
			break;
		}
		//PGP 02/12/03
		//To add/remove lock guids in the refresh object

		switch (nType)
		{
		case RECORD_LOCK:
			CMS::GetSI()->ClientLockRefresh.AddLockID(m_szGUID, nServer);
			break;
		case RECORD_UNLOCK:
			CMS::GetSI()->ClientLockRefresh.RemoveLockID(m_szGUID);
			m_szGUID = _T("");
			break;
		}
		//End added code PGP 02/12/03
	}
	catch (_com_error e)														//COM ERRORS
	{
		CGBLForm::CMSMessageBox(g_pGBLSystemInformationCMSDll->ApplicationMainFrame, 0, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONERROR, e.ErrorMessage());
		m_szGUID = _T("");
		return false;
	}
	return true;
}

/****************************************************************
KillTheDoc - Used to release the XML doc outside of the XML class AES
****************************************************************/
void CXML::KillTheDoc()
{
	if (m_docPtr != NULL)
	{
		m_docPtr.Release();
		m_docPtr = NULL;
	}
}

//****************************************************************
//FormatFromXMLDateTime - Used to format XML date with time for now,
//will rewrite use OLE datetime AES
//****************************************************************
CString CXML::FormatFromXMLDateTime(CString XMLDateTime, BOOL bolExcludeSeconds)
{
	CString szY, szMth, szD, szH, szMin, szS;
	COleDateTime dtTemp;

	szY = XMLDateTime.Mid(0, 4);
	szMth = XMLDateTime.Mid(5, 2);
	szD = XMLDateTime.Mid(8, 2);
	szH = XMLDateTime.Mid(XMLDateTime.Find('T') + 1, 2);
	szMin = XMLDateTime.Mid(XMLDateTime.Find('T') + 4, 2);
	szS = XMLDateTime.Mid(XMLDateTime.Find('T') + 7, 2);

	dtTemp.SetDateTime(_tstoi(szY), _tstoi(szMth), _tstoi(szD), _tstoi(szH), _tstoi(szMin), _tstoi(szS));

	CString szDateTime(_T(""));
	if (bolExcludeSeconds)
		szDateTime = dtTemp.Format(VAR_DATEVALUEONLY) + _T(" ") + dtTemp.Format(_T("%I:%M %p"));
	else
		szDateTime = dtTemp.Format(VAR_DATEVALUEONLY) + _T(" ") + dtTemp.Format(VAR_TIMEVALUEONLY);

	return szDateTime;
}

//****************************************************************
// FormatFromXMLDate - Used to format XML date to a string using
// the system short date format
//****************************************************************
CString CXML::FormatFromXMLDate(CString XMLDate)
{
	COleDateTime dt = FormatFromXMLToOLEDate(XMLDate);
	CString strDate;

	SYSTEMTIME sysTime;
	dt.GetAsSystemTime(sysTime);

	if (GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &sysTime, NULL, strDate.GetBufferSetLength(80), 80) > 0)
	{
		strDate.ReleaseBuffer();
	}

	return strDate;

}

//****************************************************************
//FormatFromXMLToOLEDate - Used to format XML date for now,
//will rewrite use OLE datetime AES
//****************************************************************
COleDateTime CXML::FormatFromXMLToOLEDate(CString XMLDate)
{
	//This method is use the format a date string that comes from XML (20002-10-20T0:0000) to OLE datetime the CDateTimeCtrl control AES
	COleDateTime dteValue;
	CString szY, szM, szD;
	int		 nY, nM, nD;
	szY = XMLDate.Mid(0, 4);
	szM = XMLDate.Mid(5, 2);
	szD = XMLDate.Mid(8, 2);
	nY = _tstoi(szY);
	nM = _tstoi(szM);
	nD = _tstoi(szD);
	dteValue.SetDate(nY, nM, nD);

	return dteValue;
}

//****************************************************************
//FillGridFromCDataTable - Peter ??????????????????????
//
//****************************************************************
void CXML::FillGridFromCDataTable(
	BSTR szData,
	CObject* pObjVal,
	CString szTableNumber)
{
	m_docPtr->loadXML(szData);					//load data form b string to doc
	FillGridWithData(pObjVal, szTableNumber);	//call method, pass grid object, data to table number
}

// ********************************************************
// * Procedure:    FillDataSet
// *
// * Description:  Fills in the passed in dataset using the passed in XML Command
// * and XML Data objects.
// *
// * Parameters:   pDS - Pointer to DataSet to fill up
// *               pXMLCommand - XML Command
// *               pXMLData - XML Data
// *
// * Returns:      bool - true = success, false = errors.
// *
// * Date Created: 9/10/2003 by Peter Ringering
// ********************************************************
bool CXML::FillDataSet(
	CDataSet* pDS,
	CXMLParams* pXMLCommand,
	CXMLParams* pXMLData,
	stuErrorReturn* pErrorReturn/* = NULL*/)
{
	CXML clsServerCon;
	clsServerCon.m_szXMLCommand = pXMLCommand->GetXML();
	clsServerCon.m_szXMLData = pXMLData->GetXML();
	_bstr_t szXML = clsServerCon.SendXML(pErrorReturn);
	if (clsServerCon.GetErrorReturnID() != 0) return false;
	pDS->LoadFromXML(szXML);
	if (pDS->IsEmpty()) return false;

	return true;
}

// ********************************************************
// * Procedure:    FillXMLParams
// *
// * Description:  Fills in the passed in CXMLParams using the passed in XML Command
// * and XML Data objects.
// *
// * Parameters:   pDS - Pointer to DataSet to fill up
// *               pXMLCommand - XML Command
// *               pXMLData - XML Data
// *
// * Returns:      bool - true = success, false = errors.
// *
// * Date Created: 9/10/2003 by Peter Ringering
// ********************************************************
bool CXML::FillXMLParams(
	CXMLParams* pParams,
	CXMLParams* pXMLCommand,
	CXMLParams* pXMLData,
	stuErrorReturn* pErrorReturn/* = NULL*/)
{
	CXML clsServerCon;
	clsServerCon.m_szXMLCommand = pXMLCommand->GetXML();
	clsServerCon.m_szXMLData = pXMLData->GetXML();
	_bstr_t szXML = clsServerCon.SendXML(pErrorReturn);
	if (clsServerCon.GetErrorReturnID() != 0) return false;
	pParams->Dispose();
	pParams->SetXML(szXML);
	if (pParams->IsEmpty()) return false;
	return true;
}

//****************************************************************
//CreateCurrencyEdit - Create CMS edit control for formatting of data
//****************************************************************
bool CXML::CreateCurrencyEdit(CObject* pObjVal, bool bGrid)
{
	if (!m_pCurEdit)//Multiple calls cause memory leak otherwise(PGP-03/18/2003)
	{
		m_pCurEdit = new CCMSOXCurrencyEdit();		//new cms edit control
		const UINT nResourceID = (bGrid ? (UINT)127 : (UINT)125);

		if (!m_pCurEdit->Create(WS_CHILD | ES_AUTOHSCROLL, CRect(0, 0, 0, 0), ((CWnd*)pObjVal), nResourceID)) //create control
		{
			TRACE("CXML::CreateCurrencyEdit - Failed to create edit ");
			return false;//failed to create
		}
		m_pCurEdit->m_bNegativeValue = false;
		m_pCurEdit->SetHighlightOnSetFocus(TRUE);	//highlight on focus is true
	}
	return true;
}

//****************************************************************
//CreateNumberEdit - Create CMS edit control for formatting of data
//****************************************************************
bool CXML::CreateNumberEdit(CObject* pObjVal, bool bGrid)
{
	if (!m_pNumEdit)//Multiple calls cause memory leak otherwise(PGP-03/18/2003)
	{
		m_pNumEdit = new CCMSOXNumericEdit();		//new cms edit control
		const UINT nResourceID = (bGrid ? (UINT)127 : (UINT)126);

		if (!m_pNumEdit->Create(WS_CHILD | ES_AUTOHSCROLL, CRect(0, 0, 0, 0), ((CWnd*)pObjVal), nResourceID)) //create control
		{
			TRACE("CXML::CreateNumberEdit - Failed to create edit ");
			return false;//failed to create
		}

		m_pNumEdit->SetHighlightOnSetFocus(TRUE);	//highlight on focus is true
		//		if(!m_pNumEdit)								//check if control was created
		//			return false;
	}
	return true;
}

//****************************************************************
//CreateCharEdit - Create CMS edit control for formatting of data
//****************************************************************
bool CXML::CreateCharEdit(CObject* pObjVal, bool bGrid)
{
	if (!m_pCharEdit)//Multiple calls cause memory leak otherwise(PGP-03/18/2003)
	{
		// character edit text box
		m_pCharEdit = new CCMSOXEdit();			//new cms edit control

		if (!m_pCharEdit->Create(WS_CHILD | ES_AUTOHSCROLL, CRect(0, 0, 0, 0), ((CWnd*)pObjVal), 127)) //create control
		{
			TRACE("CXML::CreateCharEdit - Failed to create edit ");
			return false;//failed to create
		}

		m_pCharEdit->SetHighlightOnSetFocus(TRUE);	//no leading zeros
		if (!m_pCharEdit) return false;				//check if control was created
	}
	return true;
}

//---------------------------------------------------------------------------
bool CXML::FillListWithData(CObject* pObjVal, CString szTableNumber)
{
	BSTR data;
	CString strTemp, szMapID, szID, szIDData;
	CString szName, szValue, szChar;

	//Create an pointer of the Node list, Element, and Node AES
	CCMSXMLDomNodeList::ComponentTypePtr pNodeList;					//	pointers used in XML  CCMSXMLDomElement
	CCMSXMLDomNode::ComponentTypePtr pNode;
	CCMSXMLDomNode::ComponentTypePtr pInnerNode;
	CCMSXMLDomNode::ComponentType* pElement = NULL;;

	szMapID = szTableNumber;									//	Set the table number to map ID for map lookup AES
	if (szTableNumber == "0")									//	If the table is 0 the node will be <Table> to set variable to blank
		szTableNumber = "";

	CListCtrl* pList = (CListCtrl*)pObjVal;
	pObjVal->IsKindOf(RUNTIME_CLASS(CCMSListCtrl));
	CCMSListCtrl* const			pListEx = (pObjVal->IsKindOf(RUNTIME_CLASS(CCMSListCtrl)) ? reinterpret_cast<CCMSListCtrl*>(pObjVal) : NULL);
	const CCMSListCtrl::DBColumnDataMap* const pListMap = (pListEx != NULL ? &(pListEx->GetDBColumnDataMap()) : NULL);

	//Clear list
	if (pListEx)
		pListEx->DeleteAllItems();
	else
		pList->DeleteAllItems();

	NMLISTVIEW	tData {};
	tData.hdr.hwndFrom = pList->GetParent()->GetSafeHwnd();

	pNodeList = m_docPtr->getElementsByTagName(_bstr_t("Table" + szTableNumber));			//	get table element AES
	pNode = pNodeList->Getitem(0);							//	assign node list to node pointer AES
	int j = 1;												//	the next number to start the counter in the get item for nodes
	int nRow = 0;
	while (pNode != NULL)
	{														//	If node list is null no need to do anything else
		// Get the Node List for all the returned values
		pNode->get_childNodes(&pNodeList);

		long nLen = 0;
		pNodeList->get_length(&nLen);						//	get the number of nodes in list AES

		for (long i = 0; i < nLen; i++)
		{													//	go through the list to assign to the controls
			pNodeList->get_item(i, &pElement);				//	get XML node element

			pElement->get_nodeName(&data);					//	get the XML node name
			szName = data;

			pElement->get_text(&data);							//get XML data value of element
			szValue = data;

			if (i == 0)
				pList->InsertItem(nRow, szValue);
			else if (pListEx)
			{
				CCMSListCtrl::DBColumnDataConstIterator	pDBColDataIter = pListMap->find(szName.Trim());
				if (pDBColDataIter == pListMap->end()) continue;	//did not find where to put the data in the list, so skip it.
				const int nCol = pDBColDataIter->second;

				pListEx->SetItemText(nRow, nCol, szValue);
				//PGP(07/08/2003)
				tData.hdr.code = LVN_ITEMCHANGED;
				tData.iItem = nRow;
				tData.iSubItem = nCol;
				pListEx->SendMessage(WM_NOTIFY, (WPARAM)LVN_ITEMCHANGED, (LPARAM)&tData);
			}
			else
				pList->SetItemText(nRow, i, szValue);
		}
		nRow++;
		pNodeList = m_docPtr->getElementsByTagName(_bstr_t("Table" + szTableNumber));		//	get node list
		pNode = pNodeList->Getitem(j);													//	get next node
		j++;	//add for next node item
	}

	pList->SendMessage(WM_CTRLPOPULATED, (WPARAM)0, (LPARAM)0);

	return true;
}

//---------------------------------------------------------------------------
CString CXML::MakeRootElement(const CString& szXML)
{
	static const CString szRoot(_T("ROOT"));
	return (szLT + szRoot + szGT + szXML + szLT + szSlash + szRoot + szGT);
}

//---------------------------------------------------------------------------
CString CXML::MakeTableElement(const CString& szXMLRecord)
{
	static const CString szTable(_T("TABLE"));
	return (szLT + szTable + szGT + szXMLRecord + szLT + szSlash + szTable + szGT);
}

//---------------------------------------------------------------------------
CString CXML::MakeRecordElement(const CString& szXMLElements)
{
	static const CString szRecord(_T("RECORD"));
	return (szLT + szRecord + szGT + szXMLElements + szLT + szSlash + szRecord + szGT);
}

//---------------------------------------------------------------------------
CString CXML::MakeElement(const CString& szTag, const CString& szData)
{
	return (szLT + szTag + szGT + szData + szLT + szSlash + szTag + szGT);
}

//------------------------------------------------------------------------------
void CXML::Subscribe(CMS::ISubscriber* pSubscriber)
{
	m_objPublisher.Subscribe(pSubscriber);
}

//------------------------------------------------------------------------------
void CXML::Unsubscribe(CMS::ISubscriber* pSubscriber)
{
	m_objPublisher.Unsubscribe(pSubscriber);
}

//------------------------------------------------------------------------------
void CXML::Notify(CCMSMessage& objMessage)
{
	m_objPublisher.Notify(objMessage);
}

//------------------------------------------------------------------------------
void CXML::Notify(void)
{
	m_objPublisher.Notify();
}

//------------------------------------------------------------------------------
void CXML::NotifyClearField(const CString& szTable, const CString& szFieldName)
{
	CCMSXMLClearField objMsg(szTable, szFieldName);
	Notify(objMsg);
}

//------------------------------------------------------------------------------
void CXML::NotifyFieldValue(const CString& szTable, const CString& szFieldName, CString& szFieldValue)
{
	CCMSXMLFieldValue objFieldValue(szTable, szFieldName, szFieldValue);
	Notify(objFieldValue);
	szFieldValue = objFieldValue.GetFieldValue();
}

//------------------------------------------------------------------------------
// BK 9/28/06 Version 2.5 Feature 8 Print Immediate from Enter Bills make server call when record is deleted
void CXML::NotifyRecordDeleted(void)
{
	CCMSXMLRecordDeleted objMsg;
	Notify(objMsg);
}

//---------------------------------------------------------------------------

//Called as part of FillControls.
//Populates a pre-set CMSDBObject with values from XML.
void CXML::FillValuesToCMSDBObjects()
{
	GROUPTOCTRLSMAP& clsAllCtrlsMap = m_clsMapCtrlGroups;
	GROUPTOCTRLSMAP::iterator itGroup;
	KEYTOELEMENTMAP::iterator itKeys;
	for (itGroup = clsAllCtrlsMap.begin(); itGroup != clsAllCtrlsMap.end(); itGroup++)
	{													//	set the cells header text and size
		for (itKeys = itGroup->second.begin(); itKeys != itGroup->second.end(); itKeys++)
		{
			if (itKeys->second.nType == CMSDBOBJECT_TYPE)
			{
				CDataSet clsDataSet;
				clsDataSet.LoadFromXML(m_szReturn);
				((CCMSDBObject*)itKeys->second.pObjVal)->InitFrom(clsDataSet);
			}
		}
	}
}

//Called as part of ChangeXMLData which builds XML string from command values.
//
void CXML::GetValuesFromCMSDBObjects()
{
	GROUPTOCTRLSMAP& clsAllCtrlsMap = m_clsMapCtrlGroups;
	GROUPTOCTRLSMAP::iterator itGroup;
	KEYTOELEMENTMAP::iterator itKeys;
	for (itGroup = clsAllCtrlsMap.begin(); itGroup != clsAllCtrlsMap.end(); itGroup++)
	{
		for (itKeys = itGroup->second.begin(); itKeys != itGroup->second.end(); itKeys++)
		{
			if (itKeys->second.nType == CMSDBOBJECT_TYPE)
			{
				CCMSDBObject* pDBObj = (CCMSDBObject*)itKeys->second.pObjVal;

				CStringArray arszCommandValues;
				pDBObj->GetAllData(arszCommandValues);

				INT_PTR nSize = arszCommandValues.GetSize();
				for (INT_PTR nIndex = 0; nIndex < nSize; nIndex += 2)
					MakeNode(arszCommandValues.GetAt(nIndex), arszCommandValues.GetAt(nIndex + 1), 3, XML_ATTRIBUTE);
			}
		}
	}
}

//Clear values in CMSDBObject types.
void CXML::ClearCMSDBObjects()
{
	GROUPTOCTRLSMAP& clsAllCtrlsMap = m_clsMapCtrlGroups;
	GROUPTOCTRLSMAP::iterator itGroup;
	KEYTOELEMENTMAP::iterator itKeys;
	for (itGroup = clsAllCtrlsMap.begin(); itGroup != clsAllCtrlsMap.end(); itGroup++)
	{
		for (itKeys = itGroup->second.begin(); itKeys != itGroup->second.end(); itKeys++)
		{
			if (itKeys->second.nType == CMSDBOBJECT_TYPE)
				((CCMSDBObject*)itKeys->second.pObjVal)->ClearValues();
		}
	}
}

//Mark the elements in the specified control group. This is
//done to find out if any element was missed during a fill.
//Elements are missed if the database values where null. Error 1-13699.
void CXML::MarkElementsBeforeFill(KEYTOELEMENTMAP* pKeyToElementMap)
{
	if (pKeyToElementMap == NULL)
		return;

	KEYTOELEMENTMAP::iterator itKeys;
	for (itKeys = pKeyToElementMap->begin(); itKeys != pKeyToElementMap->end(); itKeys++)
		itKeys->second.bFillFlag = false;
}

//Clear contents of all elements in the specified control group
//which does not have the bFillFlag set. This is called after
//FillControls,
void CXML::ClearMissedElements(KEYTOELEMENTMAP* pKeyToElementMap)
{
	if (pKeyToElementMap == NULL)
		return;

	KEYTOELEMENTMAP::iterator itKeys;
	for (itKeys = pKeyToElementMap->begin(); itKeys != pKeyToElementMap->end(); itKeys++)
		if (itKeys->second.bFillFlag == false)
			ClearElement(itKeys->second);
}

//
void CXML::EnableGroupElements(KEYTOELEMENTMAP& clsKeyToElementMap, bool bEnable)
{
	KEYTOELEMENTMAP::iterator it;
	for (it = clsKeyToElementMap.begin(); it != clsKeyToElementMap.end(); it++)
		EnableElement(it->second, bEnable);
}

void CXML::EnableElement(MAPELEMENT& stElement, bool bEnable)
{
	switch (stElement.nType)
	{																		//	Go to element by type id AES
	case FUNCCALLWRAPPERTYPE:	// begbert 10-27-2006 1-20344
		((CFuncCallWrapper*)stElement.pObjVal)->Enable(bEnable);
		break;
	case OXCURRENCYTYPE:
	case CMSPHONENUMBERTYPE:
	case ENCRYPTED_STRINGTYPE:
	case STRINGTYPE:
		((CEdit*)stElement.pObjVal)->EnableWindow(bEnable);			//	Clear the control for CEdit AES
		break;
	case CURRENCYTYPE:
		((CEdit*)stElement.pObjVal)->EnableWindow(bEnable);			//	Clear the control for Currency CEdit JKC
		break;
		// InTimeTec: 11/22/2013 : HS : 3667 D_SA_ Allow negative numbers in the Balance Registers window for all tender types. 1-37370
	case CURRENCYTYPE_NEGATIVE:
		((CEdit*)stElement.pObjVal)->EnableWindow(bEnable);			//	Clear the control for Currency CEdit JKC
		break;
		// InTimeTec: End
	case CHECKTYPE:
		((CButton*)stElement.pObjVal)->EnableWindow(bEnable);		//	Clear the control for CButton AES
		break;
	case COMBOTYPE:
	case COMBOPARTTYPE:
	case COMBOLIST_TYPE:
		((CComboBox*)stElement.pObjVal)->EnableWindow(bEnable);
		break;
	case COMBOITEMDATATYPE:
	case COMBOBOX_INDEX: // TND 5/30/03
		((CComboBox*)stElement.pObjVal)->EnableWindow(bEnable);
		break;
	case RTXEDITTYPE:
		((CRichEditCtrl*)stElement.pObjVal)->EnableWindow(bEnable);	//	Clear the control for Rich Text Edit AES
		break;
	case DATETYPE:															//	assign value for date time  type AES
		((CDateTimeCtrl*)stElement.pObjVal)->EnableWindow(bEnable);
		break;
	case CMSDATETYPE://Assign value for CMSDateCtrl type
		((CWnd*)stElement.pObjVal)->EnableWindow(bEnable);
		break;
	case DATETIMETYPE: // TND 5/30/03
		((CWnd*)stElement.pObjVal)->EnableWindow(bEnable);
		break;
	case OX_NUMERIC_TYPE:
	case ACCOUNTTYPE:														//	assign value for account control type AES
		((CWnd*)stElement.pObjVal)->EnableWindow(bEnable);
		break;
	case GROUPTYPE:															//	assign value for account control type AES
		((CCMSGrouper*)stElement.pObjVal)->GrpEnableDisable(bEnable);
		break;
	case CONTROLDATA:
		ASSERT(stElement.pObjVal->IsKindOf(RUNTIME_CLASS(CCMSControlData)));
		((CCMSControlData*)stElement.pObjVal)->EnableWindow(bEnable);
		break;
	}
}

//Clear contents of this element.
void CXML::ClearElement(MAPELEMENT& stElement)
{
	// InTimeTec: 11/21/2013: HS : Defect 9074 : D_IN_Save IN Stock Information from the Stock Item Maintenance Screen When Creating New Item
	if (m_bKeepAfterSave && !stElement.bClearOnSave) return;

	CString szValue;
	COleDateTime dteValue;
	// DTG 1.26.2004, Added for locale
	CCMSDouble dValue;

	switch (stElement.nType)
	{											//	Go to element by type id AES
	case FUNCCALLWRAPPERTYPE:	// begbert 10-27-2006 1-20344
		((CFuncCallWrapper*)stElement.pObjVal)->Set(_T(""));
		break;
	case OXCURRENCYTYPE:
		if (((COXCurrencyEdit*)stElement.pObjVal)->IsNegativeValue())
			((COXCurrencyEdit*)stElement.pObjVal)->SetValue(-0);
		else
			((COXCurrencyEdit*)stElement.pObjVal)->SetValue(0);

		break;
	case CMSPHONENUMBERTYPE:
	case ENCRYPTED_STRINGTYPE:
	case STRINGTYPE:
		((CEdit*)stElement.pObjVal)->SetWindowText(_T(""));			//	Clear the control for CEdit AES
		break;
	case CURRENCYTYPE:
		((CEdit*)stElement.pObjVal)->SetWindowText(_T(""));			//	Clear the control for Currency CEdit JKC
		break;
		// InTimeTec: 11/22/2013 : HS : 3667 D_SA_ Allow negative numbers in the Balance Registers window for all tender types. 1-37370
	case CURRENCYTYPE_NEGATIVE:
		if (((COXCurrencyEdit*)stElement.pObjVal)->IsNegativeValue())
			((COXCurrencyEdit*)stElement.pObjVal)->SetValue(-0);
		else
			((COXCurrencyEdit*)stElement.pObjVal)->SetValue(0);
		break;
		// InTimeTec: End
	case CHECKTYPE:
		((CButton*)stElement.pObjVal)->SetCheck(0);				//	Clear the control for CButton AES
		break;
	case COMBOTYPE:
		((CComboBox*)stElement.pObjVal)->SetWindowText(_T(""));		//	Clear the control for CCombBox AES
		break;
	case COMBOITEMDATATYPE:
	case COMBOBOX_INDEX:											//  Defect ID 1-8222, Reset current selection to 0th item, PGP(10/07/2003)
		((CComboBox*)stElement.pObjVal)->SendMessage(CB_SETCURSEL, 0, 0);
		break;
	case RTXEDITTYPE:
		((CRichEditCtrl*)stElement.pObjVal)->SetWindowText(_T(""));	//	Clear the control for Rich Text Edit AES
		break;
	case STATICTYPE:
		((CStatic*)stElement.pObjVal)->SetWindowText(_T(""));		//	assign value for CEdit type AES
		break;
	case STATIC_NUMBERTYPE:
		((CStatic*)stElement.pObjVal)->SetWindowText(_T("0"));		//	assign value for CEdit type AES
		break;
	case STATIC_CURRENCYTYPE:
		((CStatic*)stElement.pObjVal)->SetWindowText(dValue.GetCurrencyValue(0));	//	assign value for CEdit type AES
		if (m_bUsePaddedCtrl)((CCMSPaddedStatic<CStatic>*)stElement.pObjVal)->SetWindowText(dValue.GetCurrencyValue(0));
		break;
	case STATIC_DATETYPE:											//	assign value for CStatic for Date type AES
		((CStatic*)stElement.pObjVal)->SetWindowText(_T(""));		//	set text to object in control map AES
		break;
	case DATETYPE:													//	assign value for date time  type AES
		((CDateTimeCtrl*)stElement.pObjVal)->SetTime(dteValue.GetCurrentTime());
		break;
	case CMSDATETYPE://Assign value for CMSDateCtrl type
		((CWnd*)stElement.pObjVal)->SendMessage(WM_CMS_CLEARDATE, 0, 0);
		break;
	case ACCOUNTTYPE:												//	assign value for account control type AES
		((CWnd*)stElement.pObjVal)->SetWindowText(_T(""));
		break;
	case GROUPTYPE:													//	assign default value for radio button group - tlg
		((CCMSGrouper*)stElement.pObjVal)->SetDefault();
		break;
	case LIST_TYPE:													//  clear list ctrl types PGP
	{
		if (stElement.pObjVal->IsKindOf(RUNTIME_CLASS(CCMSListCtrl)))
			((CCMSListCtrl*)stElement.pObjVal)->DeleteAllItems();
		else
			((CListCtrl*)stElement.pObjVal)->DeleteAllItems();
	}
	break;
	case CONTROLDATA:
		ASSERT(stElement.pObjVal->IsKindOf(RUNTIME_CLASS(CCMSControlData)));
		((CCMSControlData*)stElement.pObjVal)->ResetState();
		break;
	case OX_NUMERIC_TYPE://Defect ID 1-8389, PGP(11/24/2003)
	{
		COXNumericEdit* pEdit = (COXNumericEdit*)stElement.pObjVal;
		pEdit->SetValue(0.0);
	}
	break;
	}
}

//
void CXML::ClearGroupElements(KEYTOELEMENTMAP& clsKeyToElementMap)
{
	KEYTOELEMENTMAP::iterator it;
	for (it = clsKeyToElementMap.begin(); it != clsKeyToElementMap.end(); it++)
		ClearElement(it->second);
}

//
void CXML::RemoveAllElementsFromMap()
{
	m_clsMapCtrlGroups.clear();
}

bool CXML::RemoveElementFromMap(const CString& szGroupID, const CString& szKey)
{
	bool bResult = false;
	GROUPTOCTRLSMAP::iterator itGroups = m_clsMapCtrlGroups.find(szGroupID);
	if (itGroups != m_clsMapCtrlGroups.end())
	{
		KEYTOELEMENTMAP::iterator itKeys = itGroups->second.find(szKey);
		if (itKeys != itGroups->second.end())
		{
			itGroups->second.erase(itKeys);
			bResult = true;
		}
	}
	return bResult;
}

//
BOOL CXML::LookupElement(const CString& szGroup, const CString& szKey, MAPELEMENT& clsElement)
{
	BOOL bResult = FALSE;

	GROUPTOCTRLSMAP::iterator itGroups = m_clsMapCtrlGroups.find(szGroup);
	if (itGroups != m_clsMapCtrlGroups.end())
	{
		KEYTOELEMENTMAP::iterator itKeys = itGroups->second.find(szKey);
		if (itKeys != itGroups->second.end())
		{
			clsElement = itKeys->second;
			bResult = TRUE;
		}
	}
	return bResult;
}

//
void CXML::GetCtrlMapForGroup(const CString& szGroup, KEYTOELEMENTMAP*& pCtrlGroupMap)
{
	pCtrlGroupMap = NULL;
	GROUPTOCTRLSMAP::iterator it = m_clsMapCtrlGroups.find(szGroup);
	if (it != m_clsMapCtrlGroups.end())
		pCtrlGroupMap = &it->second;
}

void CXML::SendNotifyMsg()
{
	CCMSMessageXML	objMsg(*this);
	Notify(objMsg);
}

ICMSGrid* CXML::GetCMSGrid(CObject* pObjVal)
{
	CWnd* pWnd = (CWnd*)pObjVal;
	CGETICMSGRID clsInput;
	pWnd->SendMessage(WM_CMS_GETICMSGRID, 0, (LPARAM)&clsInput);
	return clsInput.m_pGrid;
}
