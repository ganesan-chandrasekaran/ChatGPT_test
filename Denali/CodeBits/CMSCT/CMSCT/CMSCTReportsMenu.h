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
#pragma once

 //forward declaration
class CMainFrame;

class _declspec(dllexport) CCMSCTReportsMenu
{
public:
	enum enmINTEGRATIONREPORTS
	{
		eINTEGRATION_CHKLST_REPORT_1 = 0,
		eINTEGRATION_CHKLST_REPORT_2,
		eINTEGRATION_CHKLST_REPORT_3,
		eINTEGRATION_CHKLST_REPORT_4,
		eINTEGRATION_CHKLST_REPORT_5,
		eINTEGRATION_CHKLST_END
	};


	static void SetMainFrame(CMainFrame& clsFrame);					//must be called by the client so this class can access the menu
	static void RefreshMenu(void);									//can be called by the client if the menu needs to be refreshed

	// initialize items in report menu by making a server call to see if integration and/or conversion reports exist
	static bool InitializeReportMenu();
	static BOOL EnableIntegrationMenu() { return m_bEnableIntegrationMenu ? TRUE : FALSE; }
	static BOOL EnableConversionMenu() { return m_bEnableConversionMenu ? TRUE : FALSE; }
	// used to enable/disable menu items based on if any reports exist and user rights
	static BOOL GetDoesIntegrationErrorReportExist() { return m_bIntegrationErrorReportExists ? TRUE : FALSE; }
	static BOOL GetDoesPreConversionChecklistExist() { return m_bPreConversionReportExists ? TRUE : FALSE; }
	static BOOL GetDoesCodesValuesAddedReportExist() { return m_bCodeValuesAddedReportExists ? TRUE : FALSE; }
	static BOOL GetDoesIntegrationStatusReportExist() { return m_bConversionIntegReportExists ? TRUE : FALSE; }

	// integration reports
	static void RunReportsIntegrationChecklist(CCMSCTReportsMenu::enmINTEGRATIONREPORTS eReport);
	static void RemoveReportsIntegrationChecklist(void);

	// conversion reports
	static void RunReportsConversionChecklist();
	static void RunReportsConversionCodesAdded();
	static void RunReportsConversionIntegrationStatus();
	static void RemoveConversionReports(void);

	// purge all reports older than 30 days
	static void RemoveAllReportsOlderThan30Days(void);


private:
	struct CMenuItemData
	{
		CMenuItemData(void)
			: m_eReportType(CGBLPrint::AccountConversionChecklist)
			, m_szCompanyCode16(_T(""))
			, m_szCompanyName16(_T(""))
			, m_szReportXML(_T(""))
			, m_szDateTime(_T(""))
		{};

		CMenuItemData(const CMenuItemData& inRHS)
			: m_eReportType(inRHS.m_eReportType)
			, m_szCompanyCode16(inRHS.m_szCompanyCode16)
			, m_szCompanyName16(inRHS.m_szCompanyName16)
			, m_szReportXML(inRHS.m_szReportXML)
			, m_szDateTime(inRHS.m_szDateTime)
		{};

		virtual ~CMenuItemData(void) {};

		const CMenuItemData& operator= (const CMenuItemData& inRHS);
		const CMenuItemData& operator= (CDataRow& clsDataRow);

		BOOL IsEmpty(void) const;

		CGBLPrint::enmConvIntReportTypes m_eReportType;
		CString m_szCompanyCode16;
		CString m_szCompanyName16;
		CString m_szReportXML;
		CString m_szDateTime;
	};

	typedef std::vector<int> MNUIDS;
	//the int parameter in this map should be a member of one of the enm*REPORTS enumerations
	typedef std::map<int, CMenuItemData> MNUDATA;

	static MNUIDS m_vecReportsIntegrationChecklistIDs;
	static MNUIDS m_vecReportsConversionChecklistIDs;
	static MNUIDS m_vecReportsConversionCodesAddedIDs;
	static MNUIDS m_vecReportsConversionIntegrationIDs;

	static MNUDATA m_mapIntegrationErrorReportXML;
	static MNUDATA m_mapConversionCodesAddedXML;
	static MNUDATA m_mapConversionChecklistXML;
	static MNUDATA m_mapConversionIntegrationXML;

	static void MakeQueryCommand(CXML& clsXML);
	static void MakeDeleteCommand(CXML& clsXML, int nTypeReportBeingDeleted);

	static CMenu * GetReportsIntegrationChecklistMenu(void);
	static CMenu * GetReportsConversionCodesAddedMenu(void);
	static CMenu * GetIntegrationMainMenu(void);

	static const MNUIDS& GetReportsIntegrationChecklistMenuIDs(void);
	static const MNUIDS& GetReportsConversionChecklistMenuIDs(void);
	static const MNUIDS& GetReportsConversionCodesAddedMenuIDs(void);
	static const MNUIDS& GetReportsConversionIntegrationMenuIDs(void);

	static void RemoveReportsIntegrationChecklistMenu(void);
	static CMainFrame * m_pclsFrame;

	static CMenuItemData	m_stuItem;

	static bool m_bEnableIntegrationMenu;
	static bool m_bEnableConversionMenu;
	static bool m_bIntegrationErrorReportExists;
	static bool m_bPreConversionReportExists;
	static bool m_bCodeValuesAddedReportExists;
	static bool m_bConversionIntegReportExists;

	//make these private since this is a static class intended to be accessed globally
	//they should never be called
	CCMSCTReportsMenu(void);
	CCMSCTReportsMenu(const CCMSCTReportsMenu&);
	virtual ~CCMSCTReportsMenu(void);
	const CCMSCTReportsMenu& operator= (const CCMSCTReportsMenu&);
};
