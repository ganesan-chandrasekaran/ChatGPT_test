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
 *	Date:		9/10/2012
 *	Author:		InTimeTec: KP/HS
 ******************************************************************************/
#pragma once
#include "../cmsdll/GBLUserInfo.h"

#define RMV_SRV     4000
#define RMV_GRD		4001
#define RMV_GRDSRV	4002
#define	DWN_EDIT	4003
class CMP_PPG_CustomOptions;
//---------------------------------------------------------------------------

// subclass grid
class CGRD_CustomOptionsEx : public CCMSGridEntry
{
	CBitmap m_bmpDrillDown;
	//Constructor, Startup
public:
	CGRD_CustomOptionsEx(ICMSGridEntryBL& clsBL);
	virtual ~CGRD_CustomOptionsEx();
	CBitmap* GetBitmapDrillDown() { return &m_bmpDrillDown; }
	bool RemoveFromServer(int intCurrentRow, bool bRemoveBoth, bool bMsgBox = true);
	// Validate line type before saving.
	bool IsValid(ULONGLONG& ulPublishedFileSize);
	void ChangeMode(enmFormModes eMode);

	ULONGLONG	m_ulPublishedReportSize;

	//CMS overrides
protected:
	virtual void OnSetup();
	virtual void GetContextMenu(CMenu* pPopup, long lRow, int nCol);
	virtual void OnMenuCommand(int col, long row, int section, int item) { OnCMSMenuCommand(col, row, item); }
	virtual int OnCMSMenuCommand(int col, long row, int item);
	// File already published will be checked on EditFinish
	virtual int OnEditFinish(int col, long row, CWnd *edit, LPCTSTR string, BOOL cancelFlag);
	virtual int	OnEditStart(int nCol, long row, CWnd **edit);
	virtual BOOL OnCtrlCharDown(UINT wParam, long row, int col);
	void PopulateMenuItems(long lRow, CMenu* pMenu = NULL);
	virtual void OnRowChange(long oldrow, long newrow);
	// InTimeTec: 10/31/2013 : HS : Bug 9608: DEV_CT_Custom Options tab, Module Preferences form, Location/File Name field focus not moving using mouse click on next detail line
	virtual bool CanLeaveRow(long lRow, int& nNewCol, bool* pbRowWasDeleted = NULL);

private:
	void AddMenuItem(const UINT unID, const CString& szString, CMenu* pMenu, bool bEnable);

};
inline void CGRD_CustomOptionsEx::ChangeMode(enmFormModes eMode)
{
	m_bEditMode = (eMode == MODE_ADD || eMode == MODE_EDIT ? true : false);
}
//---------------------------------------------------------------------------

// line type class
class CLTCustomOptions : public CCMSLTBase
{
	DECLARE_DYNAMIC(CLTCustomOptions)

	//Constructor, Startup
public:
	CLTCustomOptions(CMP_PPG_CustomOptions& clsPage, const CString& szLineType, CCMSGridEntry& clsGrid);
	virtual ~CLTCustomOptions(void);

	enum enuColumnIDs
	{
		COL_REPORTTYPE,
		COL_REPORTLOC,
		COL_REPORTLOC_DRILLDOWN,
		COL_REPORTNAME,
		COL_PUBLISHTOSRV
	};


	enum enuModuleType
	{
		MOD_NONE = 0,
		MOD_AP,
		MOD_AR,
		MOD_BR,
		MOD_GL,
		MOD_PO,
		MOD_SA
	};

	enum enuReportType
	{
		RPT_NONE = 0,
		RPT_APAGING,
		RPT_APENTERBILLS,
		RPT_APIMMEDIATECHECKS,
		RPT_APPAYBILLS,
		RPT_APPRINTCHECKS,
		RPT_ARCUSTSTATEMENTS,
		RPT_ARAGEDANALYSIS,
		RPT_ARDATEDINVOICETOPAYMENT,
		RPT_BRPRINTCHECKS,
		RPT_GLAUDITTRIAL,
		RPT_GLBALANCESHEET,
		RPT_GLBUDGETPERFORMANCE,
		RPT_GLREVENUEEXPENSEINCOMESTATEMENT,
		RPT_GLBUDGETTRACKING,
		RPT_INHISTORY,
		RPT_INONHAND,
		RPT_INONORDER,
		RPT_POPRINTPURCHASE,
		RPT_POREPRINTPURCHASE,
		RPT_PRCHECK,
		RPT_PREDITREPORT,
		RPT_PREMPLOYEEHISTORY,
		RPT_PREMPLOYEELEAVE,
		RPT_OEEDITREPORT,
		RPT_OEPRINTINVOICES,
		RPT_OEPRINTINVOICESFRMHISTORY,
		RPT_OEPACKINGSLIPS,
		RPT_OEPICKLIST,
		RPT_PSEDITREPORT,
		RPT_SAPROFITMARGIN,
		RPT_SASALESBYSALESPERSON
	};

	//CMS overrides
public:
	virtual bool		CanAddNewRowAtEnd();
	virtual CString		GetCellText(int nColumnID, int nLTMapType);
	virtual bool		SetCellText(int nColumnID, CString& szCellText, CString& szNewLineType, int nLTMapType);
	virtual bool		GetCellBool(int nColumnID, int nLTMapType);
	virtual bool		SetCellBool(int nColumnID, bool& bValue, CString& szNewLineType, int nLTMapType);
	virtual CString		GetCellComboValue(int nColumnID, int nLTMapType);
	virtual bool		SetCellComboValue(int nColumnID, CString& szValue, CString& szNewLineType, int nLTMapType);
	virtual void		SaveToDB(CXMLElement& eleRow, long lDetailNo, int nLTMapType, bool& bProcessed);
	virtual void		LoadFromDB(CDataRow& drRow, int nLTMapType, bool& bProcessed);
	virtual bool		IsRowDirty() const { return true; }
	virtual bool		IsLineEmpty(void) const;
	// InTimeTec: 10/25/2013 : TKG : Bug 9608: DEV_CT_Custom Options tab, Module Preferences form, Location/File Name field focus not moving using mouse click on next detail line
	// Method for validation of Report Name on the grid
	bool ValidateRowBeforeLeave(long lRow);
	// InTimeTec: End
	virtual bool		CanDeleteRow(long lRow, CString& szNewLineType);
	virtual bool		IsValidData(int nLTMapType, long lRow, int& nFailColumnID);

	//Command button support
	virtual void OnCommandBtn(int nColumnID);
	void ShowFileBrowse(int nColumnID);

	void SetPublished(bool bPublished) { m_bPublishToServer = bPublished;  m_szHashData = _T(""); }
	void SetUpdate(bool bIsUpdated) { m_bIsUpdated = bIsUpdated; }

	// Flag to ensure the message box appears once.
	bool IsVerified() { return m_bVerified; }
	void SetVerified(bool bVerify) { m_bVerified = bVerify; }

	// Validate line type before saving.
	virtual bool		IsValid(long lRow, int& nFailColumnID, CWnd* Parent = NULL);
	bool				IsAccessAllowed(long lRow, int& nFailColumnID, ULONGLONG& ulPublishedFileSize, CWnd* Parent = NULL);

	// Modified bit in report location field
	bool IsModified() { return m_bModified; }
	void SetModified(bool bModified) { m_bModified = bModified; }

public:
	CString&	GetReportName() { return m_szReportName; }
	CString&	GetFileName() { return m_szFileName; }
	int			GetRowID() { return m_intRowID; }
	CString&	GetHashValue() { return m_szHashData; }
	CString&	GetReportLoc() { return m_szReportLoc; }
	bool		IsPublished() { return m_bPublishToServer; }
	COleDateTime GetModifiedDate() { return m_dteLastModified; }
	void		SetHashValue(CString szHash) { m_szHashData = szHash; }
	//Form Controls

protected:
	CMP_PPG_CustomOptions&	m_clsPage;
	enuModuleType           m_eModuleType;
	enuReportType           m_eReportType;
	int						m_intRowID;
	CString                 m_szReportLoc;
	CString					m_szReportName;
	CString					m_szFileName;
	COleDateTime			m_dteLastModified;
	bool                    m_bPublishToServer;
	CString					m_szHashData;
	int						m_intOrder;
	bool					m_bVerified;
	bool					m_bIsUpdated;
	bool					m_bModified;

	CGRD_CustomOptionsEx& GetTypedGrid()const { return (CGRD_CustomOptionsEx&)m_clsGrid; }

	bool					m_bRowDirty;

public:
	CString GetModuleName(int enuModuleValue);
	int GetReportValue(CString szModule, CString szReport);
	CString GetReportName(int enuReportValue);
};
