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
#include <algorithm>
#include "..\cmsdll\CMSReportInstantiator.h"
#include "..\cmsdll\gblnetdll.h"

const CString	CCMSReportInstantiator::REPORT = _T("REPORT");

//------------------------------------------------------------------------------
CCMSReportInstantiator::CCMSReportInstantiator(
	CMSGBL::IReportView* pReportView
	, const CString& szReportName
	, CGBLSystemInformation* si)
	: m_pReportView(pReportView)
	, m_si(si)
	, m_enuReportType(eREPORTS)
	, m_bolEnableReportParamsCheckbox(true)
	, m_bolEnablePrinterType(false)
	, m_bolSuppressPrintDialog(false)
	, m_bolDefaultPrintToScreen(false)
	, m_bolEnableOutputTypeRadios(true)
	, m_szReportName(szReportName)
	, m_szDMReportName(_T(""))
	, m_szReportObject(_T(""))
	, m_szDMReportObject(_T(""))
	, m_szPrintParameters(_T(""))
	, m_szarrReportParams()
	, m_uintConnectionType(CTY_COMPANYSERVER)
	, m_bolCancelReport(false)
	, m_clsReportParams()
	, m_clsDataParams()
	, m_clsSubreports()
	, m_clsXMLErrorReportParams(true)
	, m_clsXMLErrorDataParams(true)
	, m_szPrinterName(_T(""))
	, m_copies(1)
	, m_szNETCommand(_T("PrintReport"))
	, m_bPrintSummary(false)
	, m_bolUseReportDataSet(false)
	// InTimeTec: 08/01/2013 : HS : GL Export Audit Trail and Trail Balance To CSV
	, m_bCSVDefined(false)
	, m_bCSVEnabled(false)
	, m_bWarnBeforeOverwriting(false)
	, m_szCSVOuputPath(EMPTY_STRING)
	, m_szCSVDefaultName(EMPTY_STRING)
	, m_objPrinterSetup(NULL)
	, m_nUseReportForModule(PRINT_COMBINED)
	// InTimeTec: End
{
	CMS_VALID_POINTER(m_pReportView, CMSGBL::IReportView);
	m_szPrintParameters = CGBLPrint::GetPrintParams(m_enuReportType, m_bolEnableReportParamsCheckbox, m_bolEnablePrinterType, m_bolSuppressPrintDialog, m_bolDefaultPrintToScreen, m_bolEnableOutputTypeRadios, m_szPrinterName, m_nUseReportForModule);
	//	PBI 20585	07/17/2014	mvt - Advanced GL preview reports
	m_bolIsPreview = false;
}

//------------------------------------------------------------------------------
CCMSReportInstantiator::~CCMSReportInstantiator(void)
{
	if (m_objPrinterSetup != NULL)
		m_objPrinterSetup = NULL;
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::MakeReportTitle(UINT uintResourceID /*= 0*/, bool bolIncludeModuleTag /*= true*/, const CString& szReportTableName /*= _T("REPORT")*/)
{
	MakeReportTitle(uintResourceID == 0 ? m_pReportView->GetFormTitle() : CGBLResources::GetResourceString(uintResourceID), bolIncludeModuleTag, szReportTableName);
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::MakeReportTitle(const CString& szTitle, bool bolIncludeModuleTag /*= true*/, const CString& szReportTableName)
{
	if (!bolIncludeModuleTag)
		MakeParam(_T("@REPORTTITLE"), szTitle, szReportTableName);
	else
	{
		CXMLParams clsParams;
		clsParams.MakeParam("REPORTTITLE", szTitle);

		// PBI 22186 BK 8/21/14 Added red flag reports, show correct module in print options box
		if (m_si->ApplicationSettings.ModuleID == IDS_CT_MODULE && GetModuleName() != "")
			clsParams.MakeParam(_T("MODULE"), GetModuleName());

		//GC 1/18/2005 1-15791
		else if (m_si->ApplicationSettings.ModuleID == IDS_SA_MODULE)
			clsParams.MakeParam(_T("MODULE"), CGBLResources::GetResourceString(IDS_SALES));
		else
			clsParams.MakeParam(_T("MODULE"), m_si->ApplicationSettings.ModuleStringID);


		CString szRptTitle = clsParams.StringMerge(CGBLResources::GetResourceString(IDS_GBL_REPORTTITLE));
		MakeParam(_T("@REPORTTITLE"), szRptTitle, szReportTableName);
	}
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::MakeAccountSegmentInformation(const CString& szReportTableName)
{
	MakeParam(_T("@nSeg1"), CGBLForm::ToString(m_si->SegmentSetup.Segments[0].Length), szReportTableName);
	MakeParam(_T("@nSeg2"), CGBLForm::ToString(m_si->SegmentSetup.Segments[1].Length), szReportTableName);
	MakeParam(_T("@nSeg3"), CGBLForm::ToString(m_si->SegmentSetup.Segments[2].Length), szReportTableName);
	MakeParam(_T("@nSeg4"), CGBLForm::ToString(m_si->SegmentSetup.Segments[3].Length), szReportTableName);
	MakeParam(_T("@nSeg5"), CGBLForm::ToString(m_si->SegmentSetup.Segments[4].Length), szReportTableName);
	MakeParam(_T("@nSeg5"), CGBLForm::ToString(m_si->SegmentSetup.Segments[5].Length), szReportTableName);
	MakeParam(_T("@nSeg6"), CGBLForm::ToString(m_si->SegmentSetup.Segments[6].Length), szReportTableName);
	MakeParam(_T("@nAccountLength"), CGBLForm::ToString(m_si->SegmentSetup.TotalAccountLength), szReportTableName);
	MakeParam(_T("@szSegmentChar"), m_si->SegmentSetup.SeparationCharacter, szReportTableName);
	MakeParam(_T("@nSegmentCount"), CGBLForm::ToString(m_si->SegmentSetup.NumberOfSegments), szReportTableName);
}

//------------------------------------------------------------------------------
CString CCMSReportInstantiator::GetReportObject(void) const
{
	CString szReport(_T(""));

	if (m_szReportObject.IsEmpty())
	{
		szReport = m_szReportName;
		szReport.MakeUpper();
		szReport.Replace(_T(".RPT"), _T(""));
	}
	else
		return m_szReportObject;

	return szReport;
}

//------------------------------------------------------------------------------
CString	CCMSReportInstantiator::GetDMReportObject(void) const
{
	CString szReport(_T(""));

	if (m_szDMReportObject.IsEmpty())
	{
		szReport = m_szDMReportName;
		szReport.MakeUpper();
		szReport.Replace(_T(".RPT"), _T(""));
	}
	else
		return m_szDMReportObject;

	return szReport;
}

//------------------------------------------------------------------------------
CString	CCMSReportInstantiator::GetReportName(void) const
{
	return m_szReportName;
}

//------------------------------------------------------------------------------
CString CCMSReportInstantiator::GetDotMatrixReportName(void) const
{
	return m_szDMReportName;
}

//------------------------------------------------------------------------------
ReportTypes CCMSReportInstantiator::GetReportType(void) const
{
	return m_enuReportType;
}

//------------------------------------------------------------------------------
bool	CCMSReportInstantiator::GetEnableReportParamsCheckbox(void) const
{
	return m_bolEnableReportParamsCheckbox;
}

//------------------------------------------------------------------------------
UINT	CCMSReportInstantiator::GetConnectionType(void) const
{
	return m_uintConnectionType;
}

//------------------------------------------------------------------------------
CString CCMSReportInstantiator::GetPrintParameters(void) const
{
	return m_szPrintParameters;
}

//------------------------------------------------------------------------------
CString CCMSReportInstantiator::GetModuleName(void) const
{
	return m_szModuleName;
}

//------------------------------------------------------------------------------
CXMLParams& CCMSReportInstantiator::GetErrorXMLReportParams(void)
{
	return m_clsXMLErrorReportParams;
}

//------------------------------------------------------------------------------
CXMLParams& CCMSReportInstantiator::GetErrorXMLDataParams(void)
{
	return m_clsXMLErrorDataParams;
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::SetReportName(const CString& szReportName, const CString& szDMReportName)
{
	m_szReportName = szReportName;
	m_szDMReportName = szDMReportName;
}

//------------------------------------------------------------------------------

// InTimeTec: 10.1.2012.HS - Custom Reports
void	CCMSReportInstantiator::SetCustomReportPath(const CString& szReportPath)
{
	m_szCustomReportPath = szReportPath;
}
// InTimeTec: End

// InTimeTec: 08/01/2013 : HS : GL Export Audit Trail and Trail Balance To CSV
// Set the member variables for CSV details
void	CCMSReportInstantiator::SetCSVDefined(bool bCSVDefined, bool bCSVEnabled, bool bWarnBeforeOverwriting, CString& szCSVOutPutPath, CString& szCSVDefaultName)
{
	m_bCSVDefined = bCSVDefined;
	m_bCSVEnabled = bCSVEnabled;
	m_bWarnBeforeOverwriting = bWarnBeforeOverwriting;
	m_szCSVOuputPath = szCSVOutPutPath;
	m_szCSVDefaultName = szCSVDefaultName;
}
// InTimeTec: End

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::SetDotMatrixReportName(const CString& szDMReportName)
{
	m_szDMReportName = szDMReportName;
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::SetReportObject(const CString& szReportObject)
{
	m_szReportObject = szReportObject;
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::SetReportType(ReportTypes enuReportType)
{
	m_enuReportType = enuReportType;
	m_szPrintParameters = CGBLPrint::GetPrintParams(m_enuReportType, m_bolEnableReportParamsCheckbox, m_bolEnablePrinterType, m_bolSuppressPrintDialog, m_bolDefaultPrintToScreen, m_bolEnableOutputTypeRadios, m_szPrinterName, m_nUseReportForModule);
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::SetEnableReportParamsCheckbox(bool bolEnableReportParamsCheckbox)
{
	m_bolEnableReportParamsCheckbox = bolEnableReportParamsCheckbox;
	m_szPrintParameters = CGBLPrint::GetPrintParams(m_enuReportType, m_bolEnableReportParamsCheckbox, m_bolEnablePrinterType, m_bolSuppressPrintDialog, m_bolDefaultPrintToScreen, m_bolEnableOutputTypeRadios, m_szPrinterName, m_nUseReportForModule);
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::SetEnablePrinterTypeSelection(bool bolEnablePrinterType)
{
	m_bolEnablePrinterType = bolEnablePrinterType;
	m_szPrintParameters = CGBLPrint::GetPrintParams(m_enuReportType, m_bolEnableReportParamsCheckbox, m_bolEnablePrinterType, m_bolSuppressPrintDialog, m_bolDefaultPrintToScreen, m_bolEnableOutputTypeRadios, m_szPrinterName, m_nUseReportForModule);
}

// begbert 09-07-2006 drilldown : added this function; also added m_bolSuppressPrintDialog to header and GetPrintParams calls.
void	CCMSReportInstantiator::SetSuppressPrintDialog(bool bolSuppressPrintDialog)
{
	m_bolSuppressPrintDialog = bolSuppressPrintDialog;
	m_szPrintParameters = CGBLPrint::GetPrintParams(m_enuReportType, m_bolEnableReportParamsCheckbox, m_bolEnablePrinterType, m_bolSuppressPrintDialog, m_bolDefaultPrintToScreen, m_bolEnableOutputTypeRadios, m_szPrinterName, m_nUseReportForModule);
}

// begbert 09-13-2006 drilldown : added this function; also added m_bolForcePrintToScreen to header and GetPrintParams calls.
void	CCMSReportInstantiator::SetDefaultPrintToScreen(bool bolDefaultPrintToScreen)
{
	m_bolDefaultPrintToScreen = bolDefaultPrintToScreen;
	m_szPrintParameters = CGBLPrint::GetPrintParams(m_enuReportType, m_bolEnableReportParamsCheckbox, m_bolEnablePrinterType, m_bolSuppressPrintDialog, m_bolDefaultPrintToScreen, m_bolEnableOutputTypeRadios, m_szPrinterName, m_nUseReportForModule);
}

// begbert 06-01-2007 1-27753 : added this function, also added m_bolEnableOutputTypeRadios to header and GetPrintParams calls.
// begbert 05-29-2008 1-29491 : Added the missing explicit 'false' for the bForcePrintToScreen to header and GetPrintParams calls;
//									this was missing (and thus being filled in erroneously by the m_bolEnableOutputTypeRadios param).
void	CCMSReportInstantiator::SetEnableOutputTypeRadios(bool bolEnableOutputTypeRadios)
{
	m_bolEnableOutputTypeRadios = bolEnableOutputTypeRadios;
	m_szPrintParameters = CGBLPrint::GetPrintParams(m_enuReportType, m_bolEnableReportParamsCheckbox, m_bolEnablePrinterType, m_bolSuppressPrintDialog, m_bolDefaultPrintToScreen, m_bolEnableOutputTypeRadios, m_szPrinterName, m_nUseReportForModule);
}

void	CCMSReportInstantiator::SetPrinterName(const CString& szPrinterName)
{
	m_szPrinterName = szPrinterName;
	m_szPrintParameters = CGBLPrint::GetPrintParams(m_enuReportType, m_bolEnableReportParamsCheckbox, m_bolEnablePrinterType, m_bolSuppressPrintDialog, m_bolDefaultPrintToScreen, m_bolEnableOutputTypeRadios, m_szPrinterName, m_nUseReportForModule);
}

void	CCMSReportInstantiator::SetUseReportForModule(const __int8 nUseReportForModule/* = PRINT_COMBINED*/)
{
	m_nUseReportForModule = nUseReportForModule;
	m_szPrintParameters = CGBLPrint::GetPrintParams(m_enuReportType, m_bolEnableReportParamsCheckbox, m_bolEnablePrinterType, m_bolSuppressPrintDialog, m_bolDefaultPrintToScreen, m_bolEnableOutputTypeRadios, m_szPrinterName, m_nUseReportForModule);
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::SetConnectionType(UINT uintConnectionType)
{
	m_uintConnectionType = uintConnectionType;
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::SetPrintParameters(const CString& szPrintParameters)
{
	m_szPrintParameters = szPrintParameters;
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::SetModuleName(const CString& szModuleName)
{
	m_szModuleName = szModuleName;
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::CancelReport(bool bolCancel)
{
	m_bolCancelReport = bolCancel;
}

//------------------------------------------------------------------------------
bool	CCMSReportInstantiator::CancelReport(void) const
{
	return m_bolCancelReport;
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::MakeReportValue(const CString& szTag, const CString& szValue, const CString& szReportTableName)
{
	MakeParam(szTag, szValue, szReportTableName);
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::MakeReportValue(const CString& szTag, long lngValue, const CString& szReportTableName)
{
	MakeParam(szTag, CGBLForm::ToString(lngValue), szReportTableName);
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::MakeReportValue(const CString& szTag, bool bolValue, const CString& szReportTableName)
{
	MakeParam(szTag, bolValue ? _T("true") : _T("false"), szReportTableName);
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::MakeReportValue(const CString& szTag, int intValue, const CString& szReportTableName)
{
	MakeParam(szTag, CGBLForm::ToString(intValue), szReportTableName);
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::MakeReportValue(const CString& szTag, double dblValue, const CString& szReportTableName)
{
	MakeParam(szTag, CCMSDouble(dblValue), szReportTableName);
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::MakeReportValue(const CString& szTag, const COleDateTime& dtValue, UINT uintFormat, const CString& szReportTableName)
{
	MakeParam(szTag, dtValue.Format(uintFormat), szReportTableName);
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::MakeReportValue(const CString& szTag, const COleDateTime& dtValue, const CString& szFormatMask, const CString& szReportTableName)
{
	MakeParam(szTag, dtValue.Format(szFormatMask), szReportTableName);
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::MakeReportValue(const CString& szTag, CMSGBL::ICMSDataMapEx& objDataMap, const CString& szReportTableName)
{
	MakeParam(szTag, objDataMap.GetDBValue(), szReportTableName);
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::MakeReportResource(const CString& szTag, UINT uintResourceID, const CString& szReportTableName)
{
	MakeParam(szTag, CGBLResources::GetResourceString(uintResourceID), szReportTableName);
}

//begbert 01-24-2007: Exactly like MakeReportResource(), except it tacks a colon onto the end of the string before sending it on.
void	CCMSReportInstantiator::MakeReportResource_C(const CString& szTag, UINT uintResourceID, const CString& szReportTableName)
{
	MakeParam(szTag, CGBLResources::GetResourceString(uintResourceID) + _T(":"), szReportTableName);
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::MakeSubReport(const CString& szReportTableName)
{
	m_clsSubreports.push_back(szReportTableName);
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::MakeParam(const CString& szTag, const CString& szValue, const CString& szReportTableName)
{
	CString szTagName(szTag);
	CString szData(szValue);

	szTagName.Trim();
	szData.Trim();

	ASSERT(szTagName.GetLength() > 1);

	const CString	szReportLevel(szReportTableName == REPORT ? szReportTableName : REPORT + _T("/") + szReportTableName);

	//if this is a report parameter, remove the @ indicating that it is. this is because the xmlSoc
	//the report uses won't want it and, if it is a server param, the server won't want it either
	const CString tag1(szTagName.Find(_T("@")) == 0 ? szTagName.Mid(1) : szTagName);
	//if this is a server parameter, remove the # indicating that it is. reasons are ditto the above except this is just for the report
	const CString tag2(szTagName.Find(_T("#")) == szTagName.GetLength() - 1 ? tag1.Left(tag1.GetLength() - 1) : tag1);

	if (szTagName.Find(_T("@")) == 0)
		m_clsReportParams.insert(std::make_pair(szReportLevel, std::make_pair(tag2, szValue)));

	if (szTagName.Find(_T("#")) == szTagName.GetLength() - 1)
		m_clsDataParams.push_back(std::make_pair(tag1, szValue));
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::MakeReportDocument(CXMLDocument& clsXMLDoc)
{
	ASSERT(m_clsReportParams.size() > 0);
	std::for_each(m_clsReportParams.begin(), m_clsReportParams.end(), CCMSBuildReportParamDoc(clsXMLDoc));
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::MakeReportDocument(CXMLParams& objReportParams)
{
	ASSERT(m_clsReportParams.size() > 0);
	std::for_each(m_clsReportParams.begin(), m_clsReportParams.end(), CCMSBuildReportParams(objReportParams));
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::MakeSubReports(CXMLDocument& clsXMLDoc)
{
	std::for_each(m_clsSubreports.begin(), m_clsSubreports.end(), CCMSBuildSubreports(clsXMLDoc));
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::MakeDataParams(CStringArray& szarrData)
{
	std::for_each(m_clsDataParams.begin(), m_clsDataParams.end(), CCMSBuildDataParam(szarrData));
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::ExecuteReport(void)
{
	switch (m_enuReportType)
	{
	case eERRORREPORT:
		ExecuteErrorReport();
		break;
	default:
		ExecuteNormalReport();
		break;
	};
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::ExecuteNormalReport(void)
{
	CCMSWaitCursor c;

	//	PBI 20585	07/17/2014	mvt - Advanced GL preview reports
	if (!m_bolIsPreview)
		m_pReportView->SetReportValues(this);
	if (CancelReport()) return;
	// InTimeTec: 10.3.2012.HS - Custom Reports
	CString szPath(_T(""));
	if (!m_szCustomReportPath.IsEmpty())
	{
		if (m_szCustomReportPath == RESSTRING(IDS_LBL_ERROR))
		{
			CString szMessage;
			szMessage = RESSTRING(IDS_SERVER_ACCESS);
			CGBLForm::CMSMessageBox(AfxGetMainWnd(), 0, IDS_SERVER_INACCESSIBLE, MB_OK | MB_ICONSTOP, szMessage);
			CancelReport(true);
			return;
		}
		else if (m_szCustomReportPath == RESSTRING(IDS_TITLE_ERROR_REPORT))
		{
			CGBLForm::CMSMessageBox(AfxGetMainWnd(), IDS_ERR_MSG_REPORT_NOT_FOUND, IDS_TITLE_ERROR_REPORT, MB_OK | MB_ICONSTOP);
			CancelReport(true);
			return;
		}
		szPath = m_szCustomReportPath;
	}
	else
	{
		//if (m_szModuleName.IsEmpty())
		//	m_szModuleName = m_si->ApplicationSettings.ModuleStringID;
		szPath = CGBLForm::GetReportPath(m_szReportName, false, m_szModuleName.IsEmpty() ? m_si->ApplicationSettings.ModuleStringID : m_szModuleName);
	}
	// InTimeTec: End
	CancelReport(szPath.IsEmpty());
	if (CancelReport()) return;		//cancel the report

	CString szDMPath(_T(""));
	if (m_bolEnablePrinterType)	//if the user can select dot matrix and we don't have a report file for it, cancel the report
	{
		szDMPath = CGBLForm::GetReportPath(m_szDMReportName);
		//	PBI 35886	09/30/2016	mvt - Override dot matrix report path with custom path
		if (!m_szCustomReportPath.IsEmpty())
			szDMPath = m_szCustomReportPath;
		CancelReport(szDMPath.IsEmpty());
		if (CancelReport()) return;		//cancel the report
	}

	// InTimeTec: 08/01/2013 : HS : GL Export Audit Trail and Trail Balance To CSV
	if (m_bCSVDefined)
		SetCSVDetails();
	// InTimeTec: End

	m_pReportView->EnableParent(FALSE);

	MakeParam(_T("@szReport"), szPath, CMSStrings::XMLTags::REPORT);
	MakeParam(_T("@szDMReport"), szDMPath, CMSStrings::XMLTags::REPORT);
	MakeParam(_T("@szCompanyName"), m_si->CompanySettings.Name, CMSStrings::XMLTags::REPORT);
	MakeParam(_T("@szCompanyCode"), m_si->CompanySettings.ID, CMSStrings::XMLTags::REPORT);

	CXMLDocument	clsXMLDoc;
	MakeReportDocument(clsXMLDoc);
	MakeSubReports(clsXMLDoc);
	CString szReportParams(clsXMLDoc.GetOuterXML(true));

	CStringArray szarrData;
	MakeDataParams(szarrData);

	CXML	objXML;
	const CString szModule(m_szModuleName.IsEmpty() ? m_si->ApplicationSettings.ModuleStringID : m_szModuleName);
	objXML.SetReportXMLParameters(szModule, GetReportObject(), GetDMReportObject(), szarrData, m_uintConnectionType);

	c.Restore();

	//12/18/2005 - RCG - Removed
	//::AfxMessageBox(szReportParams, MB_OK | MB_ICONINFORMATION, 0);
	//::AfxMessageBox(objXML.m_szXMLCommand, MB_OK | MB_ICONINFORMATION, 0);
	//::AfxMessageBox(objXML.m_szXMLFormat, MB_OK | MB_ICONINFORMATION, 0);
	//::AfxMessageBox(m_szPrintParameters, MB_OK | MB_ICONINFORMATION, 0);

	if ((m_bolUseReportDataSet) && (m_objReportDataSet.getTableCount() > 0))
	{
		CString szXML = m_objReportDataSet.GetXML();

		szXML.Replace(_T("<ROOT>"), _T(""));
		szXML.Replace(_T("</ROOT>"), _T(""));
		szXML.Replace(_T("<ReportData>"), _T(""));
		szXML.Replace(_T("</ReportData>"), _T(""));

		CGBLPrint::PrintReport(&szXML, &szReportParams, ePOSTING, false);

		m_objReportDataSet.Dispose();
	}
	else
	{
		// begbert 06-19-2007 1-27975 : capture the result in m_bolCancelReport
		m_bolCancelReport = CGBLPrint::LoadPrintBox(
			m_objPrinterSetup,
			&szReportParams,
			&objXML.m_szXMLCommand,
			&objXML.m_szXMLFormat,
			m_szPrintParameters,
			(m_si->ApplicationSettings.DeveloperMode && m_si->ApplicationSettings.ShowXMLViewer),
			true,
			&m_clsErrorReturn);
	}

	m_bolUseReportDataSet = false;
	m_pReportView->EnableParent(TRUE);
	m_pReportView->OnReportEnd(m_clsErrorReturn);
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::ExecuteErrorReport(void)
{
	CCMSWaitCursor c;

	//	PBI 20585	07/17/2014	mvt - Advanced GL preview reports
	if (!m_bolIsPreview)
		m_pReportView->SetReportValues(this);
	if (CancelReport()) return;

	CString szXMLReport(m_clsXMLErrorReportParams.GetXML());
	CString szXMLData(m_clsXMLErrorDataParams.GetXML());

	c.Restore();

	CGBLPrint::PrintReport(&szXMLData, &szXMLReport, m_enuReportType, false);
}

//----------------------------------------------------------------------------------
void	CCMSReportInstantiator::StraightToPrinter()
{
	//DAJ 5/11/5 err: 1-19253 wrote this function to use the new NETDLL methods
	CCMSWaitCursor c;
	CXMLDocument clsXMLRoot(CMSStrings::XMLTags::Root);
	CXMLElement ele = clsXMLRoot.GetDocumentElement();

	ele.AddNewChild(L"COMMAND", m_szNETCommand);
	ele.AddNewChild(L"PRINTSUMMARY", m_bPrintSummary);
	ele.AddNewChild(L"SHOWXMLVIEWER", m_si->ApplicationSettings.ShowXMLViewer);

	CXMLElement clsPrint = ele.AddNewChild(L"PRINTTYPE");
	clsPrint.AddNewChild(L"TYPE", L"1");
	CString szPrinterName = m_szPrinterName;
	if (szPrinterName.IsEmpty())
		szPrinterName = CGBLPrint::GetDefaultPrinterName();

	clsPrint.AddNewChild(L"PRINTERNAME", szPrinterName);
	clsPrint.AddNewChild(L"COPIES", CGBLForm::ToString(m_copies));

	m_pReportView->SetReportValues(this);
	ele.AddNewChild(L"XMLCOMMAND", L"");
	CXMLElement eleXmlData = ele.AddNewChild(L"XMLDATA", m_clsXMLErrorDataParams.GetXML());
	CXMLElement eleXMLReport = ele.AddNewChild(L"XMLREPORTPARAMS", m_clsXMLErrorReportParams.GetXML());

	c.Restore();

	CString szError(CGBLNetDll::DoSpecialCommand(clsXMLRoot.GetOuterXML()));
}

//------------------------------------------------------------------------------
CCMSReportInstantiator::CCMSBuildReportParamDoc::CCMSBuildReportParamDoc(CXMLDocument& clsXMLDoc)
	: m_clsXMLDoc(clsXMLDoc)
	, m_clsXMLReport()
	, m_clsXMLElement()
	, m_szTableName(_T("ROOT"))
{
	m_clsXMLReport = m_clsXMLDoc.SetRoot(m_szTableName);
	m_clsXMLReport = m_clsXMLReport.AddNewChild(CCMSReportInstantiator::REPORT);
	m_szTableName = CCMSReportInstantiator::REPORT;
}

//------------------------------------------------------------------------------
CCMSReportInstantiator::CCMSBuildReportParams::CCMSBuildReportParams(CXMLParams& objReportParams)
	: m_objReportParams(objReportParams)
	, m_szTableName(_T("ROOT"))
{
	m_szTableName = CCMSReportInstantiator::REPORT;
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::CCMSBuildReportParamDoc::operator() (const CCMSReportInstantiator::CCMSReportParams::value_type& clsReportParam)
{
	if (clsReportParam.first == CCMSReportInstantiator::REPORT)
		m_clsXMLReport.AddNewChild(clsReportParam.second.first, clsReportParam.second.second);

	else
	{
		const CString	szReportTag(CCMSReportInstantiator::REPORT + _T("/"));
		const CString	szReportLevel(clsReportParam.first.Mid(szReportTag.GetLength()));

		if (m_szTableName != szReportLevel)
		{
			m_szTableName = szReportLevel;
			m_clsXMLElement = m_clsXMLReport.AddNewChild(m_szTableName);
		}

		m_clsXMLElement.AddNewChild(clsReportParam.second.first, clsReportParam.second.second);
	}
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::CCMSBuildReportParams::operator() (const CCMSReportInstantiator::CCMSReportParams::value_type& clsReportParam)
{
	m_objReportParams.MakeParam(clsReportParam.second.first, clsReportParam.second.second);
}

//------------------------------------------------------------------------------
CCMSReportInstantiator::CCMSBuildSubreports::CCMSBuildSubreports(CXMLDocument& clsXMLDoc)
	: m_clsXMLDoc(clsXMLDoc)
	, m_clsXMLElement()
{
	m_clsXMLElement = m_clsXMLDoc.GetDocumentElement();
	m_clsXMLElement = m_clsXMLElement.AddNewChild(_T("Subreports"));
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::CCMSBuildSubreports::operator() (const CCMSReportInstantiator::CCMSSubreports::value_type& clsReportname)
{
	m_clsXMLElement.AddNewChild(clsReportname);
}

//------------------------------------------------------------------------------
CCMSReportInstantiator::CCMSBuildDataParam::CCMSBuildDataParam(CStringArray& szarrData)
	: m_szarrData(szarrData)
{
	m_szarrData.RemoveAll();
}

//------------------------------------------------------------------------------
void	CCMSReportInstantiator::CCMSBuildDataParam::operator() (const CCMSReportInstantiator::CCMSDataParams::value_type& clsDataParam)
{
	m_szarrData.Add(clsDataParam.first);
	m_szarrData.Add(clsDataParam.second);
}

// InTimeTec: 06.28.2012.KP - GL Impact Summary Report
// this function creates the Command, Data and Report XML and send it to server
// for printing the GL Impact summary Report
void	CCMSReportInstantiator::GLImpact(CString szReportParam, CString szBatchCode, int nBatchType, int nPostingNumber)
{
	//Command XML [start]
	CXMLParams  clsXMLCommand;
	clsXMLCommand.MakeParam(_T("COMMAND"), _T("POST"));
	clsXMLCommand.MakeParam(_T("OBJECT"), _T(""));
	clsXMLCommand.MakeParam(_T("MODULE"), m_si->ApplicationSettings.ModuleStringID);
	CXML clsXML;
	clsXML.m_szXMLCommand = clsXMLCommand.GetXML();
	clsXMLCommand.Dispose();
	//Command XML [end]

	CString bc = szBatchCode.IsEmpty() ? m_si->CurrentBatch.BatchCode : szBatchCode;
	int bt = nBatchType == -1 ? m_si->CurrentBatch.BatchType : nBatchType;
	int pn = nPostingNumber == -1 ? m_si->CurrentBatch.PostingNumber : nPostingNumber;

	// Data XML[Start]
	CXMLParams clsXMLData;
	clsXMLData.MakeParam("GUID", CGBLMath::GenerateGUID());
	CXMLParams::AppendCompanyInfo(&clsXMLData);
	CXMLParams::AppendXMLConnection(&clsXMLData, CXMLParams::CN_COMPANY, "CONNECTION", true);
	//clsXMLData.MakeParam("bolUserPermission",  pData->m_bUserPermission);
	clsXMLData.MakeParam("strBatchCode", bc);
	if (bc.IsEmpty())
	{
		AfxMessageBox(_T("No batch is selected. Please Select a batch"));
		return;
	}
	clsXMLData.MakeParam("bytBatchType", bt);
	clsXMLData.MakeParam("strLocation", m_si->CompanySettings.DefaultLocation);
	clsXMLData.MakeParam("intPostingNumber", pn);
	clsXMLData.MakeParam("FundInstalled", m_si->CompanySettings.IsFund ? 1 : 0);

	if (m_si->CompanySettings.IsFund)
	{
		int  bGLIntegLevel = m_si->ModuleSettings.CurrentSettings->GLIntegration;
		bool bGLIntegration = ((bGLIntegLevel > 0 && bGLIntegLevel < 10) || bGLIntegLevel > 99) ? true : false;

		if (bGLIntegration || m_si->ApplicationSettings.ModuleID == IDS_GL_MODULE)
		{
			if (m_si->SegmentSetup.FundSegment == 1)
				clsXMLData.MakeParam("OneBasedFundSegmentStart", 1);
			else
			{
				long lAccountLength = (long)m_si->SegmentSetup.TotalAccountLength;
				long lFundSegmentLength = (long)m_si->SegmentSetup.FundSegInfo->Length;
				clsXMLData.MakeParam("OneBasedFundSegmentStart", lAccountLength - lFundSegmentLength + 1);
			}

			clsXMLData.MakeParam("FundSegmentLength", m_si->SegmentSetup.FundSegInfo->Length);
		}
	}

	clsXMLData.MakeParam("FCStartDate", m_si->FiscalCalendar.StartDate.Format(VAR_DATEVALUEONLY));
	clsXMLData.MakeParam("FCEndDate", m_si->FiscalCalendar.EndDate.Format(VAR_DATEVALUEONLY));
	clsXMLData.MakeParam("FCBlockDate", m_si->FiscalCalendar.BlockDate.Format(VAR_DATEVALUEONLY));
	clsXMLData.MakeParam("GLInterfaceKeys", m_si->ApplicationSettings.InterfaceKeys);

	if (m_si->ModuleSettings.CurrentSettings->IsForModule() == IDS_OE_MODULE)
	{
		clsXMLData.MakeParam("bytSAModule", "1");
		clsXMLData.UpdateParam("COMPANYINFO/bytGLInt", m_si->ModuleSettings.OESettings.GetGLIntegration());
		clsXMLData.UpdateParam("COMPANYINFO/bytAPInt", m_si->ModuleSettings.OESettings.GetAPIntegration());
		clsXMLData.UpdateParam("COMPANYINFO/bytARInt", m_si->ModuleSettings.OESettings.GetARIntegration());
		clsXMLData.UpdateParam("COMPANYINFO/bytINInt", m_si->ModuleSettings.OESettings.GetINIntegration());
		clsXMLData.UpdateParam("COMPANYINFO/bytJCInt", m_si->ModuleSettings.OESettings.GetJCIntegration());
	}

	if (m_si->ModuleSettings.CurrentSettings->IsForModule() == IDS_PS_MODULE)
	{
		clsXMLData.MakeParam("bytSAModule", "2");
		clsXMLData.UpdateParam("COMPANYINFO/bytGLInt", m_si->ModuleSettings.PSSettings.GetGLIntegration());
		clsXMLData.UpdateParam("COMPANYINFO/bytAPInt", m_si->ModuleSettings.PSSettings.GetAPIntegration());
		clsXMLData.UpdateParam("COMPANYINFO/bytARInt", m_si->ModuleSettings.PSSettings.GetARIntegration());
		clsXMLData.UpdateParam("COMPANYINFO/bytINInt", m_si->ModuleSettings.PSSettings.GetINIntegration());
		clsXMLData.UpdateParam("COMPANYINFO/bytJCInt", m_si->ModuleSettings.PSSettings.GetJCIntegration());
	}

	if (m_si->ModuleSettings.CurrentSettings->IsForModule() == IDS_OE_MODULE || m_si->ModuleSettings.CurrentSettings->IsForModule() == IDS_PS_MODULE)
	{
		clsXMLData.MakeParam("bytCCAuthLevel", m_si->ModuleSettings.GetCurrentSalesInterface()->GetCCAuthLevel());
		clsXMLData.MakeParam("bytCKAuthLevel", m_si->ModuleSettings.GetCurrentSalesInterface()->GetCheckAuthLevel());
	}

	if (m_si->ModuleSettings.CurrentSettings->IsForModule() == IDS_GL_MODULE)
	{
		clsXMLData.MakeParam("bolUnbalancedTran", m_si->ModuleSettings.GLSettings.UnbalancedTran);
		clsXMLData.MakeParam("bolUnbalancedFund", m_si->ModuleSettings.GLSettings.UnbalancedFund);
	}

	//New Node added to make server code identify that this request is for impact report
	clsXMLData.MakeParam("bytGLImpact", true);

	//Make server to ignore locking records. We don't need locking for Impact Summary Report
	clsXMLData.MakeParam("IGNORELOCKING", "");
	// InTimeTec: 09/20/2013 : TKG : Bug 9255: D_AP_Pay Bills - GL Impact Summary failing to print correctly when you have unprinted check in batch
	// Added a new flag to avoid the unprinted check validation
	clsXMLData.MakeParam("OVERIDEUNPRINTED", "");
	// InTimeTec: End
	//clsXMLData.MakeParam("USEROVERRIDE", ""); // 6489 - DWP - 06/27/13 - Removed this so we will get the GL Validation errors having to do with the batch being unbalanced when printing this report.
	//	PBI 34953	09/29/2016	mvt - Replaced the XML parameter so the report will print when out of balance posting is allowed.
	if (m_si->ModuleSettings.CurrentSettings->IsForModule() == IDS_GL_MODULE &&
		m_si->ModuleSettings.GLSettings.UnbalancedTran &&
		m_si->UserRight.m_stuGeneralLedgerNode.Journal.bAllowOutOfBalancePosting)
		clsXMLData.MakeParam("USEROVERRIDE", "");

	clsXMLData.MakeParam("bolUserPermission", true);
	clsXML.m_szXMLData = clsXMLData.GetXML();
	clsXML.m_szXMLData = m_si->Servers.AppendServerInfo(clsXML.m_szXMLData);
	clsXMLData.Dispose();
	//Data XML [end]

	//Report XML [start]
	GLImpactReportValues(bc);
	CCMSWaitCursor c;

	if (CancelReport()) return;

	//This function Returns the path which takes module name (In this its taking Respective module ) What we have to do??
	//m_szModuleName = _T("GL");
	const CString szPath(CGBLForm::GetReportPath(m_szReportName, false, _T("GL")));
	CancelReport(szPath.IsEmpty());
	if (CancelReport()) return;

	CString szDMPath(_T(""));
	if (m_bolEnablePrinterType)
	{
		szDMPath = CGBLForm::GetReportPath(m_szDMReportName);
		CancelReport(szDMPath.IsEmpty());
		if (CancelReport()) return;
	}

	m_pReportView->EnableParent(FALSE);

	MakeParam(_T("@szReport"), szPath, CMSStrings::XMLTags::REPORT);
	MakeParam(_T("@szDMReport"), szDMPath, CMSStrings::XMLTags::REPORT);
	MakeParam(_T("@szCompanyName"), m_si->CompanySettings.Name, CMSStrings::XMLTags::REPORT);
	MakeParam(_T("@szCompanyCode"), m_si->CompanySettings.ID, CMSStrings::XMLTags::REPORT);

	CXMLDocument clsXMLDoc;
	MakeReportDocument(clsXMLDoc);
	MakeSubReports(clsXMLDoc);
	CString szReportParams(clsXMLDoc.GetOuterXML(true));

	CStringArray szarrData;
	MakeDataParams(szarrData);

	CXML objXML;
	const CString szModule(m_szModuleName.IsEmpty() ? m_si->ApplicationSettings.ModuleStringID : m_szModuleName);
	objXML.SetReportXMLParameters(szModule, GetReportObject(), GetDMReportObject(), szarrData, m_uintConnectionType);

	c.Restore();
	//Report XML [end]

	//Printing Report
	if ((m_bolUseReportDataSet) && (m_objReportDataSet.getTableCount() > 0))
	{
		CString szXML = m_objReportDataSet.GetXML();

		szXML.Replace(_T("<ROOT>"), _T(""));
		szXML.Replace(_T("</ROOT>"), _T(""));
		szXML.Replace(_T("<ReportData>"), _T(""));
		szXML.Replace(_T("</ReportData>"), _T(""));

		CGBLPrint::PrintReport(&szXML, &szReportParams, ePOSTING, false);

		m_objReportDataSet.Dispose();
	}
	else
	{

		m_bolCancelReport = CGBLPrint::LoadPrintBox(
			m_objPrinterSetup,
			&szReportParams,
			&clsXML.m_szXMLCommand,
			&clsXML.m_szXMLData,
			m_szPrintParameters,
			(m_si->ApplicationSettings.DeveloperMode && m_si->ApplicationSettings.ShowXMLViewer),
			true,
			&m_clsErrorReturn);
	}

	m_bolUseReportDataSet = false;
	m_pReportView->EnableParent(TRUE);
	m_pReportView->OnReportEnd(m_clsErrorReturn);
}
// InTimeTec: End

// InTimeTec: 07.2.2012.KP - GL Impact Summary Report
// Create the Report XML
void	CCMSReportInstantiator::GLImpactReportValues(CString szBatchCode)
{
	MakeReportTitle(" General Ledger Impact Summary Report");
	MakeReportValue(_T("@szBatchCode#"), szBatchCode.IsEmpty() ? m_si->CurrentBatch.BatchCode : szBatchCode);
	MakeReportValue(_T("@bDetailed"), 1);
	MakeReportValue(_T("@szReportType"), _T("Summary"));
	MakeReportResource(_T("@IDS_MODULE"), IDS_MODULE);

	if (m_si->ApplicationSettings.ModuleStringID == _T("GL"))
		MakeReportResource(_T("@szModuleString"), IDS_CMS_GENERALLEDGER);
	else if (m_si->ApplicationSettings.ModuleStringID == _T("AR"))
		MakeReportResource(_T("@szModuleString"), IDS_MODULE_AR);
	else if (m_si->ApplicationSettings.ModuleStringID == _T("AP"))
		MakeReportResource(_T("@szModuleString"), IDS_MODULE_AP);
	else if (m_si->ApplicationSettings.ModuleStringID == _T("PO"))
		MakeReportResource(_T("@szModuleString"), IDS_MODULE_PO);
	else if (m_si->ApplicationSettings.ModuleStringID == _T("SA"))
		MakeReportResource(_T("@szModuleString"), IDS_SALES);
	else if (m_si->ApplicationSettings.ModuleStringID == _T("BR"))
		MakeReportResource(_T("@szModuleString"), IDS_MODULE_BR);
	else if (m_si->ApplicationSettings.ModuleStringID == _T("IN"))
		MakeReportResource(_T("@szModuleString"), IDS_INVENTORY1);

	MakeReportResource(_T("@IDS_BATCHWITHCOLON"), IDS_BATCHWITHCOLON);
	MakeReportResource(_T("@IDS_MODULE"), IDS_MODULE);
	MakeReportResource(_T("@IDS_ACCOUNT_NUMBER_S_DESCRIPTION"), IDS_ACCOUNT_NUMBER_S_DESCRIPTION);
	MakeReportResource(_T("@IDS_ACCOUNTCLASS"), IDS_ACCOUNTCLASS);
	MakeReportResource(_T("@IDS_DEBIT"), IDS_DEBIT);
	MakeReportResource(_T("@IDS_CREDIT"), IDS_CREDIT);
	MakeReportResource(_T("@IDS_TOTAL_DEBITS"), IDS_TOTAL_DEBITS);
	MakeReportResource(_T("@IDS_CREDITS"), IDS_CREDITS);
	MakeReportResource(_T("@IDS_ACCOUNT_BALANCE_AFTER_POSTING"), IDS_ACCOUNT_BALANCE_AFTER_POSTING);
	MakeReportResource(_T("@IDS_IMPACT_SUMMARY_NOTE"), IDS_IMPACT_SUMMARY_NOTE);
	MakeReportResource(_T("@IDS_SINGLE"), IDS_SINGLE);

	//Xml for Error report, if validation fails.
	//m_szModuleName = _T("GB");
	const CString szPath(CGBLForm::GetReportPath(_T("GBLErrorReport.rpt"), false, _T("GB")));
	MakeParam(_T("@szErrorReport"), szPath, CMSStrings::XMLTags::REPORT);
	CString szErrorReportTitle = _T(" Posting Error Report");
	CString szModuleID(m_si->ApplicationSettings.ModuleStringID);
	szModuleID += szErrorReportTitle;
	MakeParam(_T("@ERRORREPORTTITLE"), szModuleID, CMSStrings::XMLTags::REPORT);
	MakeReportValue(_T("@bShowLineOnly"), false);
	MakeReportResource(_T("@IDS_DESCRIPTION"), IDS_DESCRIPTION);
	MakeReportResource(_T("@IDS_TRAN_NO"), IDS_TRAN_NO);
	MakeReportResource(_T("@IDS_LINE_NUMBER"), IDS_LINE_NUMBER);
	MakeReportResource(_T("@IDS_ITEMNUMBER"), IDS_ITEMNUMBER);
}
// InTimeTec: End

//	PBI 36883	10/17/2016	mvt - Added PO Edit Report
void	CCMSReportInstantiator::POEditReport(
	int nReportType, CString szStartDate,
	CString szEndDate, CString szStartTime,
	CString szEndTime, CString szBatchStart,
	CString szBatchEnd, CString szTransStart,
	CString szTransEnd, CString szInvoiceNumberStart,
	CString szInvoiceNumberEnd, CString szCustomerNumberStart,
	CString szCustomerNumberEnd, byte bytModule)
{
	//Command XML [start]
	CXMLParams  clsXMLCommand;
	clsXMLCommand.MakeParam(_T("COMMAND"), _T("REPORT"));
	clsXMLCommand.MakeParam(_T("OBJECT"), _T("SAPRINTPOEDITREPORT"));
	clsXMLCommand.MakeParam(_T("MODULE"), _T("SA"));
	CXML clsXML;
	clsXML.m_szXMLCommand = clsXMLCommand.GetXML();
	clsXMLCommand.Dispose();
	//Command XML [end]

	// Data XML[Start]
	CXMLParams clsXMLData;
	clsXMLData.MakeParam("GUID", CGBLMath::GenerateGUID());
	CXMLParams::AppendCompanyInfo(&clsXMLData);
	CXMLParams::AppendXMLConnection(&clsXMLData, CXMLParams::CN_COMPANY, "CONNECTION", true);

	clsXMLData.MakeParam("bytReportType", CGBLForm::ToString(nReportType));
	clsXMLData.MakeParam("bolMultiBatchMode", CMS::GetSI()->ApplicationSettings.MultiBatchMode == MBM_Multi ? true : false);
	clsXMLData.MakeParam("bolIsPurchasing", true);

	CString szStartDateTime = szStartDate + " " + szStartTime;
	CString szEndDateTime = szEndDate + " " + szEndTime;
	clsXMLData.MakeParam("szStartDate", szStartDateTime);
	clsXMLData.MakeParam("szEndDate", szEndDateTime);

	clsXMLData.MakeParam("szBatchStart", szBatchStart);
	clsXMLData.MakeParam("szBatchEnd", szBatchEnd);

	clsXMLData.MakeParam("szTransStart", szTransStart);
	clsXMLData.MakeParam("szTransEnd", szTransEnd);

	clsXMLData.MakeParam("szInvoiceNumberStart", szInvoiceNumberStart);
	clsXMLData.MakeParam("szInvoiceNumberEnd", szInvoiceNumberEnd);

	clsXMLData.MakeParam("szCustomerNumberStart", szCustomerNumberStart);
	clsXMLData.MakeParam("szCustomerNumberEnd", szCustomerNumberEnd);

	CString szModuleNumber;
	szModuleNumber.Format(_T("%d"), bytModule);
	clsXMLData.MakeParam("szModule", szModuleNumber);

	clsXML.m_szXMLData = clsXMLData.GetXML();
	clsXML.m_szXMLData = m_si->Servers.AppendServerInfo(clsXML.m_szXMLData);

	clsXMLData.Dispose();
	//Data XML [end]

	//Report XML [start]
	POEditReportValues(nReportType,
		szStartDate, szEndDate,
		szStartTime, szEndTime,
		szBatchStart, szBatchEnd,
		szTransStart, szTransEnd,
		szInvoiceNumberStart, szInvoiceNumberEnd,
		szCustomerNumberStart, szCustomerNumberEnd, bytModule);
	CCMSWaitCursor c;

	if (CancelReport()) return;

	//This function Returns the path which takes module name (In this its taking Respective module ) What we have to do??
	//m_szModuleName = _T("GL");
	const CString szPath(CGBLForm::GetReportPath(m_szReportName, false, _T("SA")));
	CancelReport(szPath.IsEmpty());
	if (CancelReport()) return;

	CString szDMPath(_T(""));
	if (m_bolEnablePrinterType)
	{
		szDMPath = CGBLForm::GetReportPath(m_szDMReportName);
		CancelReport(szDMPath.IsEmpty());
		if (CancelReport()) return;
	}

	m_pReportView->EnableParent(FALSE);

	MakeParam(_T("@szReport"), szPath, CMSStrings::XMLTags::REPORT);
	MakeParam(_T("@szDMReport"), szDMPath, CMSStrings::XMLTags::REPORT);
	MakeParam(_T("@szCompanyName"), m_si->CompanySettings.Name, CMSStrings::XMLTags::REPORT);
	MakeParam(_T("@szCompanyCode"), m_si->CompanySettings.ID, CMSStrings::XMLTags::REPORT);

	CXMLDocument clsXMLDoc;
	MakeReportDocument(clsXMLDoc);
	MakeSubReports(clsXMLDoc);
	CString szReportParams(clsXMLDoc.GetOuterXML(true));

	CStringArray szarrData;
	MakeDataParams(szarrData);

	CXML objXML;
	const CString szModule(m_szModuleName.IsEmpty() ? m_si->ApplicationSettings.ModuleStringID : m_szModuleName);
	objXML.SetReportXMLParameters(szModule, GetReportObject(), GetDMReportObject(), szarrData, m_uintConnectionType);

	c.Restore();
	//Report XML [end]

	//Printing Report
	if ((m_bolUseReportDataSet) && (m_objReportDataSet.getTableCount() > 0))
	{
		CString szXML = m_objReportDataSet.GetXML();

		szXML.Replace(_T("<ROOT>"), _T(""));
		szXML.Replace(_T("</ROOT>"), _T(""));
		szXML.Replace(_T("<ReportData>"), _T(""));
		szXML.Replace(_T("</ReportData>"), _T(""));

		CGBLPrint::PrintReport(&szXML, &szReportParams, ePOSTING, false);

		m_objReportDataSet.Dispose();
	}
	else
	{

		m_bolCancelReport = CGBLPrint::LoadPrintBox(
			m_objPrinterSetup,
			&szReportParams,
			&clsXML.m_szXMLCommand,
			&clsXML.m_szXMLData,
			m_szPrintParameters,
			(m_si->ApplicationSettings.DeveloperMode && m_si->ApplicationSettings.ShowXMLViewer),
			true,
			&m_clsErrorReturn);
	}

	m_bolUseReportDataSet = false;
	m_pReportView->EnableParent(TRUE);
	m_pReportView->OnReportEnd(m_clsErrorReturn);
}

//	PBI 36883	10/17/2016	mvt - Added PO Edit Report
void	CCMSReportInstantiator::POEditReportValues(
	int nReportType, CString szStartDate,
	CString szEndDate, CString szStartTime,
	CString szEndTime, CString szBatchStart,
	CString szBatchEnd, CString szTransStart,
	CString szTransEnd, CString szInvoiceNumberStart,
	CString szInvoiceNumberEnd, CString szCustomerNumberStart,
	CString szCustomerNumberEnd, byte bytModule)
{
	bool bIsMultiBatchMode = CMS::GetSI()->ApplicationSettings.MultiBatchMode == MBM_Multi ? true : false;
	MakeReportTitle(IDS_PURCHASE_EDIT_REPORT);
	m_szReportName = _T("SA_POPurchasingEdit.rpt");
	SetReportName(m_szReportName);
	MakeReportValue(_T("@bytReportType#"), CGBLForm::ToString(nReportType));
	MakeReportValue(_T("@bolMultiBatchMode#"), bIsMultiBatchMode);

	MakeReportValue(_T("szStartDate#"), szStartDate + _T(" ") + szStartTime);
	MakeReportValue(_T("szEndDate#"), szEndDate + _T(" ") + szEndTime);

	MakeReportValue(_T("@szStartDate"), szStartDate);
	MakeReportValue(_T("@szEndDate"), szEndDate);

	MakeReportValue(_T("@szStartTime"), szStartTime);
	MakeReportValue(_T("@szEndTime"), szEndTime);

	MakeReportValue(_T("@szBatchStart"), szBatchStart);
	MakeReportValue(_T("@szBatchEnd"), szBatchEnd);

	MakeReportValue(_T("@szTransStart"), szTransStart);
	MakeReportValue(_T("@szTransEnd"), szTransEnd);

	MakeReportValue(_T("@szInvoiceNumberStart#"), szInvoiceNumberStart);
	MakeReportValue(_T("@szInvoiceNumberEnd#"), szInvoiceNumberEnd);

	MakeReportValue(_T("@szCustomerNumberStart"), szCustomerNumberStart);
	MakeReportValue(_T("@szCustomerNumberEnd"), szCustomerNumberEnd);

	CString szModuleNumber;
	szModuleNumber.Format(_T("%d"), bytModule);
	MakeReportValue(_T("@szModule"), szModuleNumber);

	MakeReportValue(_T("@bolIsOE#"), bytModule = 1 ? true : false);

	MakeReportValue(_T("bolIsPurchasing#"), true);
	MakeReportResource(_T("@IDS_COMMENT"), IDS_COMMENT);
	MakeReportResource(_T("@IDS_COMPLETE"), IDS_COMPLETE);
	MakeReportResource(_T("@IDS_DESCRIPTION"), CMS::GetSI()->ModuleSettings.POSettings.JCIntegration == 0 ? (UINT)IDS_DESCRIPTION : (UINT)IDS_DESCRIPTION_JOB_PHASE);
	MakeReportResource(_T("@IDS_STOCK_NUMBER_GL_ACCT"), CMS::GetSI()->ModuleSettings.POSettings.JCIntegration == 0 ? (UINT)IDS_PO_STOCK_NUMBER_GL_ACCT : (UINT)IDS_PO_STOCK_NUMBER_GL_ACCT_COST);
	MakeReportResource(_T("@IDS_INVOICED_EXT"), IDS_INVOICED_EXT);
	MakeReportResource(_T("@IDS_INVOICEDCOST"), IDS_INVOICEDCOST);
	MakeReportResource(_T("@IDS_LANDED_AMT"), IDS_LANDED_AMT);
	MakeReportResource(_T("@IDS_LOCATION"), IDS_LOCATION);
	MakeReportResource(_T("@IDS_ORDERED"), IDS_ORDERED);
	MakeReportResource(_T("@IDS_QTY_INVOICED"), IDS_QTY_INVOICED);
	MakeReportResource(_T("@IDS_QTY_RECEIVED"), IDS_QTY_RECEIVED);
	MakeReportResource(_T("@IDS_QTY_REMAINING"), IDS_QTY_REMAINING);
	MakeReportResource(_T("@IDS_RECEIVEDCOST"), IDS_RECEIVEDCOST);
	MakeReportResource(_T("@IDS_DETAILED"), IDS_DETAILED);
	// Group Header 2b string values
	MakeReportResource(_T("@IDS_PO_NUMBER_C"), IDS_PO_NUMBER_C);
	MakeReportResource(_T("@IDS_AP_VENDOR_NUMBER_C"), IDS_AP_VENDOR_NUMBER_C);
	MakeReportResource(_T("@IDS_VENDOR_CONTACT_C"), IDS_VENDOR_CONTACT_C);
	MakeReportResource(_T("@IDS_PAID_OUT_C"), IDS_PAID_OUT_C);
	MakeReportResource(_T("@IDS_PAIDOUT_VENDOR_C"), IDS_PAIDOUT_VENDOR_C);
	MakeReportResource(_T("@IDS_ORDER_DATE_C"), IDS_ORDER_DATE_C);
	MakeReportResource(_T("@IDS_BUYER_C"), IDS_BUYER_C);
	MakeReportResource(_T("@IDS_HOLD_C"), IDS_HOLD_C);
	MakeReportResource(_T("@IDS_DROP_SHIP_C"), IDS_DROP_SHIP_C);
	MakeReportResource(_T("@IDS_SHIPPING_METHOD_C"), IDS_SHIPPING_METHOD_C);
	MakeReportResource(_T("@IDS_FREIGHT_C"), IDS_FREIGHT_C);			//
	MakeReportResource(_T("@IDS_TOTAL_AMOUNT_C"), IDS_TOTAL_AMOUNT_C);
	MakeReportResource(_T("@IDS_SALES_TAX_C"), IDS_SALES_TAX_C);
	MakeReportResource(_T("@IDS_AP_DISCOUNT_C"), IDS_AP_DISCOUNT_C);
	MakeReportResource(_T("@IDS_DISCOUNT_AMOUNT_C"), IDS_DISCOUNT_AMOUNT_C);
	// Group Header 2d string values
	MakeReportResource(_T("@IDS_SA_PP_LINEITEM_LINE_NO"), IDS_SA_PP_LINEITEM_LINE_NO);
	MakeReportResource(_T("@IDS_STOCKNUMBER_S_GLACCOUNT"), IDS_STOCKNUMBER_S_GLACCOUNT);
	MakeReportResource(_T("@IDS_GBL_QTY_ORDERED"), IDS_GBL_QTY_ORDERED);
	MakeReportResource(_T("@IDS_UNIT_COST"), IDS_UNIT_COST);
	//MakeReportResource(_T("@IDS_EXTENSION"), IDS_EXTENSION);//1-35266 DRR 05-18-2011
	MakeReportResource(_T("@IDS_LBL_EXTENDED"), IDS_LBL_EXTENDED);
	MakeReportResource(_T("@IDS_CTRL_TAX"), IDS_CTRL_TAX);
	MakeReportResource(_T("@IDS_LBL_VENDORSTOCKNO"), IDS_LBL_VENDORSTOCKNO);
	MakeReportResource(_T("@IDS_TYPE"), IDS_TYPE);
	MakeReportResource(_T("@IDS_COMMITED_ACCOUNT"), IDS_COMMITED_ACCOUNT);
	MakeReportResource(_T("@IDS_DISCOUNT"), IDS_DISCOUNT);
	// Group Footer 2b string values
	MakeReportResource(_T("@IDS_SUBTOTAL_C"), IDS_SUBTOTAL_C);
	// Group Footer 1c and b string values
	MakeReportResource(_T("@IDS_TOTAL_FOR_BATCH_C"), IDS_TOTAL_FOR_BATCH_C);
	MakeReportResource(_T("@IDS_AP_TOTAL_INVENTORY_C"), IDS_AP_TOTAL_INVENTORY_C);
	MakeReportResource(_T("@IDS_AP_TOTAL_DIRECT_EXPENSE_C"), IDS_AP_TOTAL_DIRECT_EXPENSE_C);
	MakeReportResource(_T("@IDS_TOTAL_SALES_TAX_C"), IDS_TOTAL_SALES_TAX_C);
	MakeReportResource(_T("@IDS_LESS_TOTAL_DISCOUNTS_C"), IDS_LESS_TOTAL_DISCOUNTS_C);
	MakeReportResource(_T("@IDS_TOTAL_FREIGHT_C"), IDS_TOTAL_FREIGHT_C);
	MakeReportResource(_T("@IDS_PERCENT"), IDS_PERCENT);
	MakeReportResource(_T("@IDS_DOLLAR"), IDS_DOLLAR);
	MakeReportResource(_T("@IDS_AP_DISCDATE"), IDS_AP_DISCDATE);
	MakeReportResource(_T("@IDS_CT_PAID_OUT"), IDS_CT_PAID_OUT);
	MakeReportResource(_T("@IDS_DEPARTMENT"), IDS_DEPARTMENT);
	MakeReportResource_C(_T("@IDS_DESCRIPTION_C"), IDS_DESCRIPTION);
	MakeReportResource(_T("@IDS_INVOICEDUE"), IDS_INVOICEDUE);
	MakeReportResource(_T("@IDS_PO_NUMBER"), IDS_PO_NUMBER);
	MakeReportResource(_T("@IDS_RECEIVED"), IDS_RECEIVED);
	MakeReportResource(_T("@IDS_RECEIVED_BY"), IDS_RECEIVED_BY);
	MakeReportResource(_T("@IDS_SA_TRANSACTION_TOTALS"), IDS_SA_TRANSACTION_TOTALS);
	MakeReportResource(_T("@IDS_NO"), IDS_NO);
	MakeReportResource(_T("@IDS_YES"), IDS_YES);
	MakeReportResource(_T("@IDS_CONDENSED"), IDS_CONDENSED);
	MakeReportResource_C(_T("@IDS_AP_PURCHASE_DISCOUNT_C"), IDS_AP_PURCHASE_DISCOUNT);
	MakeReportResource(_T("@IDS_BATCH"), IDS_BATCH);
	MakeReportResource_C(_T("@IDS_BATCHNUMBER_C"), IDS_BATCHNUMBER);
	MakeReportResource(_T("@IDS_EXPECTED"), IDS_EXPECTED);
	MakeReportResource_C(_T("@IDS_FREIGHT_ADDED_C"), IDS_FREIGHT_ADDED);
	MakeReportResource_C(_T("@IDS_FREIGHT_COLLECT_C"), IDS_FREIGHT_COLLECT);
	MakeReportResource(_T("@IDS_INVOICE_NUMBER"), IDS_INVOICE_NUMBER);
	MakeReportResource(_T("@IDS_INVOICED"), IDS_INVOICED);
	MakeReportResource_C(_T("@IDS_LBL_TRANSACTION_NUMBER_C"), IDS_LBL_TRANSACTION_NUMBER);
	MakeReportResource_C(_T("@IDS_LINE_ITEMS_C"), IDS_LINE_ITEMS);
	MakeReportResource(_T("@IDS_LINE_NUMBER"), IDS_LINE_NUMBER);
	MakeReportResource(_T("@IDS_PAGE"), IDS_PAGE);
	MakeReportResource(_T("@IDS_RECEIVED_LANDED"), IDS_RECEIVED_LANDED);
	MakeReportResource(_T("@IDS_REPORT_SELECTION_CRITERIA"), IDS_REPORT_SELECTION_CRITERIA);
	MakeReportResource(_T("@IDS_REPORT_TOTALS"), IDS_REPORT_TOTALS);
	MakeReportResource(_T("@IDS_REPORT_TYPE"), IDS_REPORT_TYPE);
	MakeReportResource_C(_T("@IDS_SALES_TAX_C"), IDS_SALES_TAX);
	MakeReportResource_C(_T("@IDS_TOTAL_C"), IDS_TOTAL);
	MakeReportResource(_T("@IDS_TOTALS"), IDS_TOTALS);
	MakeReportResource_C(_T("@IDS_VENDOR_C"), IDS_VENDOR);
	MakeReportResource(_T("@IDS_START"), IDS_START);
	MakeReportResource(_T("@IDS_END"), IDS_END);
	MakeReportResource(_T("@IDS_SUMMARY"), IDS_SUMMARY);
	MakeReportResource(_T("@IDS_NO"), IDS_NO);
	MakeReportResource(_T("@IDS_YES"), IDS_YES);
	MakeReportResource(_T("@IDS_AP_VENDOR_NUMBER_C"), IDS_AP_VENDOR_NUMBER_C);
	MakeReportResource(_T("@IDS_VENDOR_CONTACT_C"), IDS_VENDOR_CONTACT_C);
	MakeReportResource(_T("@IDS_PAID_OUT_C"), IDS_PAID_OUT_C);
	MakeReportResource(_T("@IDS_PAIDOUT_VENDOR_C"), IDS_PAIDOUT_VENDOR_C);
	MakeReportResource(_T("@IDS_ORDER_DATE_C"), IDS_ORDER_DATE_C);
	MakeReportResource(_T("@IDS_BUYER_C"), IDS_BUYER_C);
	MakeReportResource(_T("@IDS_HOLD_C"), IDS_HOLD_C);
	MakeReportResource(_T("@IDS_DROP_SHIP_C"), IDS_DROP_SHIP_C);
	MakeReportResource(_T("@IDS_SHIPPING_METHOD_C"), IDS_SHIPPING_METHOD_C);
	MakeReportResource(_T("@IDS_TOTAL_AMOUNT_C"), IDS_TOTAL_AMOUNT_C);
	MakeReportResource(_T("@IDS_SALES_TAX_C"), IDS_SALES_TAX_C);
	MakeReportResource(_T("@IDS_AP_DISCOUNT_C"), IDS_AP_DISCOUNT_C);
	MakeReportResource(_T("@IDS_DISCOUNT_AMOUNT_C"), IDS_DISCOUNT_AMOUNT_C);
	MakeReportResource(_T("@IDS_SUBTOTAL_C"), IDS_SUBTOTAL_C);
	MakeReportResource(_T("@IDS_FREIGHT_C"), IDS_FREIGHT_C);
	MakeReportResource(_T("@IDS_TOTAL_FOR_BATCH_C"), IDS_TOTAL_FOR_BATCH_C);
	MakeReportResource(_T("@IDS_AP_TOTAL_INVENTORY_C"), IDS_AP_TOTAL_INVENTORY_C);
	MakeReportResource(_T("@IDS_AP_TOTAL_DIRECT_EXPENSE_C"), IDS_AP_TOTAL_DIRECT_EXPENSE_C);
	MakeReportResource(_T("@IDS_TOTAL_SALES_TAX_C"), IDS_TOTAL_SALES_TAX_C);
	MakeReportResource(_T("@IDS_LESS_TOTAL_DISCOUNTS_C"), IDS_LESS_TOTAL_DISCOUNTS_C);
	MakeReportResource(_T("@IDS_TOTAL_FREIGHT_C"), IDS_TOTAL_FREIGHT_C);
	MakeReportResource(_T("@IDS_AR_CONDENSED"), IDS_AR_CONDENSED);
	MakeReportResource(_T("@IDS_PERCENT"), IDS_PERCENT);
	MakeReportResource(_T("@IDS_DOLLAR"), IDS_DOLLAR);
	MakeReportResource(_T("@IDS_ASSIGNED_AT_POSTING"), IDS_ASSIGNED_AT_POSTING);
	MakeReportResource(_T("@IDS_AR_DATE"), IDS_AR_DATE);
	MakeReportResource(_T("@IDS_SA_TIME_C"), IDS_SA_TIME_C);
	MakeReportResource(_T("@IDS_CUSTOMER_NUBMER"), IDS_CUSTOMER_NUBMER);
}

// InTimeTec: 08/01/2013 : HS : GL Export Audit Trail and Trail Balance To CSV
// Add the CSV information to the Print XML
void	CCMSReportInstantiator::SetCSVDetails()
{
	if (m_szPrintParameters.IsEmpty())
		m_szPrintParameters = CGBLPrint::GetPrintParams(m_enuReportType, m_bolEnableReportParamsCheckbox, m_bolEnablePrinterType, m_bolSuppressPrintDialog, m_bolDefaultPrintToScreen, m_bolEnableOutputTypeRadios, m_szPrinterName, m_nUseReportForModule);

	CXMLDocument	xmlDoc;
	xmlDoc.LoadXML(m_szPrintParameters);
	CXMLElement	eleRoot = xmlDoc.GetDocumentElement();
	CXMLElement elePrintParams = eleRoot.GetItem(_T("REPORTPARAMS"));
	elePrintParams.AddNewChild(_T("CSV_OPTION_VISIBLE"), m_bCSVDefined);
	elePrintParams.AddNewChild(_T("CSV_OPTION_ENABLED"), m_bCSVEnabled);
	elePrintParams.AddNewChild(_T("CSV_WARN"), m_bWarnBeforeOverwriting);
	elePrintParams.AddNewChild(_T("CSV_OUTPUT_PATH"), m_szCSVOuputPath);
	elePrintParams.AddNewChild(_T("CSV_DEFAULT_NAME"), m_szCSVDefaultName);
	m_szPrintParameters = xmlDoc.GetOuterXML();
}
// InTimeTec: End

void	CCMSReportInstantiator::SetPrinterSetup(
	PrinterSetupProxy* PrinterSetup)
{
	m_objPrinterSetup = PrinterSetup;
}

void	CCMSReportInstantiator::SetIsLandscape(bool value)
{
	// for this to work properly, set the m_objPrinterSetup variable
	// equal to the return from calling GetPrinterProxy, and do it in
	// the constructor of your report form.
	//		m_objPrinterSetup = GetPrinterProxy();

	// In the destructor of your report form, delete the variable
	//		delete m_objPrinterSetup;

	// Look at the code which is run when the OK button is pressed and
	// the report instantiator is created and make sure the SetPrinterSetup
	// routine is called.
	//		objReportInstantiator.SetPrinterSetup(m_objPrinterSetup);

	// This routine is normally called in the SetReportValues routine
	// on your report form.

	// By default IsLandscape is false so this normally only needs to
	// be set to true but if you can change from one report to another
	// and some are landscape and some are not, it is sometimes good to
	// set this to false.

	if (m_objPrinterSetup == NULL)
		ASSERT(false);  // Look at the comments above to fix issue
	else
		m_objPrinterSetup->SetIsLandscape(value);
}
