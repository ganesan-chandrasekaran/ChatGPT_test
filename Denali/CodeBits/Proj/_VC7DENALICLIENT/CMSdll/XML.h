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
#pragma once
 // XML.h : header file
 //
#include "GBLConstants.h"
#include "GBLErrorConstants.h"
#include "IPublishSubscribe.h"
#include "CMSPublisherDelegate.h"

#include "..\cmsdll\CMSMSXML.h"

class CServerSideInput;
class CCMSDataAdapterOptions;
class CCMSDataAdapter;
class CCMSMessageDataAdapter;
class CCMSDataBridgeOptions;
class CCMSDataBridge;
class CCMSMessageDataBridge;
class CCMSMessageDataBridgeSave;
class CCMSDataBridgeTrigger;
class CCMSMessageBridgeTrigger;
class CCMSMessageBridgeXML;
class CCMSMessageTriggerCompl;

//Interface to hook into xml parsing done by CXML class,
//provides notification before clearFields.
//provides notifications when a field is parsed.
//See CXML::RegisterDataCallBackProvider to attach.

interface IXMLDataCallBackProvider
{
	virtual void		ClearFields() = 0;
	virtual void		FieldValueCallBack(LPCTSTR lpszField, LPCTSTR lpszValue) = 0;
};

// begbert 10-25-2006 1-20344 : virtual base for template class below
class CFuncCallWrapper : public CObject
{
public:
	virtual void Set(CString str) = 0;
	virtual BOOL Enable(BOOL bEnable) = 0;
};

// begbert 10-25-2006 1-20344 : This class may be used to allow any objects that have accessors on their variables to transfer data
// begbert 06-08-2007 1-27829 : Added a second allowable function-prototype for the 'Set' function, and made the Enable function optional.

template <class T>
class CFuncCallWrapperT : public CFuncCallWrapper
{
	typedef T	object;

	typedef void (object::*PSETFUNC1) (CString);
	typedef void (object::*PSETFUNC2) (const CString&);
	typedef void (object::*PENABLEFUNC1) (bool);
	typedef BOOL(object::*PENABLEFUNC2) (BOOL);

public:


	CFuncCallWrapperT<T>(T& pObj, PSETFUNC1 fpSet) : m_pObj(&pObj), m_fpSet1(fpSet), m_fpSet2(NULL), m_fpEnable1(NULL), m_fpEnable2(NULL) { }
	CFuncCallWrapperT<T>(T& pObj, PSETFUNC2 fpSet) : m_pObj(&pObj), m_fpSet1(NULL), m_fpSet2(fpSet), m_fpEnable1(NULL), m_fpEnable2(NULL) { }
	CFuncCallWrapperT<T>(T& pObj, PSETFUNC1 fpSet, PENABLEFUNC1 fpEnable) : m_pObj(&pObj), m_fpSet1(fpSet), m_fpSet2(NULL), m_fpEnable1(fpEnable), m_fpEnable2(NULL) { }
	CFuncCallWrapperT<T>(T& pObj, PSETFUNC2 fpSet, PENABLEFUNC1 fpEnable) : m_pObj(&pObj), m_fpSet1(NULL), m_fpSet2(fpSet), m_fpEnable1(fpEnable), m_fpEnable2(NULL) { }
	CFuncCallWrapperT<T>(T& pObj, PSETFUNC1 fpSet, PENABLEFUNC2 fpEnable) : m_pObj(&pObj), m_fpSet1(fpSet), m_fpSet2(NULL), m_fpEnable1(NULL), m_fpEnable2(fpEnable) { }
	CFuncCallWrapperT<T>(T& pObj, PSETFUNC2 fpSet, PENABLEFUNC2 fpEnable) : m_pObj(&pObj), m_fpSet1(NULL), m_fpSet2(fpSet), m_fpEnable1(NULL), m_fpEnable2(fpEnable) { }

	void Set(CString szValue)
	{
		ASSERT(m_pObj && (m_fpSet1 || m_fpSet2));
		if (!m_pObj) return;
		if (m_fpSet1) (m_pObj->*m_fpSet1)(szValue);
		if (m_fpSet2) (m_pObj->*m_fpSet2)(szValue);
	}

	BOOL Enable(BOOL bEnable)
	{
		ASSERT(m_pObj);
		if (!m_pObj) return FALSE;
		if (m_fpEnable1) { (m_pObj->*m_fpEnable1)(!!bEnable); return TRUE; }
		if (m_fpEnable2) return (m_pObj->*m_fpEnable2)(bEnable);
		return FALSE;
	}

private:
	// for reasons beyond my ken, the i, j, and k objects or something like them is REQUIRED for the objects to pack correctly.
	T*				m_pObj;
	PSETFUNC1		m_fpSet1;
	int i;
	PSETFUNC2		m_fpSet2;
	int j;
	PENABLEFUNC1	m_fpEnable1;
	int k;
	PENABLEFUNC2	m_fpEnable2;
};

class CXML;

//Interface to hook into CXML to do the following
//1. Get a callback with a reference to internal m_docPtr when new data is fetched
//as a result of a GetXMLData call. (Can be used to fill a memory data structure)
//2. Get a callback before data is saved with a reference to the XML class,
//so the internal data string can be accessed and modified.
//See CXML::RegisterDataHook to attach
interface IXMLDataHookProvider
{
	virtual void		HookGetData(CXML& clsXML, const CString& szControlID) = 0;
	virtual void		HookSaveData(CXML& clsXML) = 0;
};

interface ICMSResetProvider
{
	virtual void Reset(void) = 0;
};

/////////////////////////////////////////////////////////////////////////////
// CXML window
class CCMSOXNumericEdit;
class CCMSOXCurrencyEdit;
class CCMSOXEdit;
interface ICMSGrid;

class __declspec(dllexport) CXML : public CWnd, public CMS::IPublisher, public CCMSSubscriber<CXML>
{
	friend class CServerSide;
	DECLARE_DYNCREATE(CXML)
public:
	CXML();
	virtual				~CXML();
	//using CMS edit class

	//Developer Level variables go here AES
	enum enuExclusive
	{
		NONEXCLUSIVE = 0,
		EXCLUSIVE = 1,
		EXCLUSIVE_PENDING = 2,
		STARTUPLOCK = 3,
		EXCLUSIVEUNLOCK = 4,
		EXCLUSIVEPENDINGUNLOCK = 5,
		OEMULIPTLEUSERSPERBATCHON = 6,
		OEMULIPTLEUSERSPERBATCHOFF = 7
	};

	//add so that you can change the connection at the form level to you a different server for maint forms.
	bool				m_bUseDefinedServer;
	UINT				m_uinDefinedServer;
	bool				m_bServerError;
	bool				m_bDeveloperMode;
	bool				m_bShowXmlViewer;
	bool				m_bStart;
	bool				m_bGridOnly;
	bool				m_bTableEnd;
	int 				m_intTableCount;
	CString				m_szXMLFormat;
	CString				m_szXMLReport;
	CString				m_szXMLCommand;
	CString				m_szXMLData;
	CString				m_szXMLSendData;
	CString				m_szFormTitle;
	CWnd*				m_hForm;
	int					m_nAnswer;
	int					m_nStartChar;	//Use for the Autofill AES
	int					m_nEndChar;		//Use for the Autofill AES
	CString				m_szUser,
		m_szNumber,
		m_szText,
		m_szGUID;

	bool				m_bNoNegativeNumber;			//Used to not use negative number in the grids

	bool				m_bNoRecordLock;
	bool				m_bUsePaddedCtrl;
	bool				m_bMultipleDetails;			//use so you can have multiple controls to fill with from detail lines. AES
	bool				m_bDontClearAllControl; //to keep the element from clearing out all the controls AES
	int					m_nMode;
	// InTimeTec: 05/20/2013 : HS : Attach Files to Master Records
	bool				m_bSaveSpecial;
	// InTimeTec: 08/26/2013 : HS :Bug 8417 : D_AR - Credit Card Codes form does not clear to blank form when deleting an existing record - goes to the next record
	bool				m_bClearAllControls;
	// InTimeTec: 11/21/2013: HS : Defect 9074 : D_IN_Save IN Stock Information from the Stock Item Maintenance Screen When Creating New Item
	bool				m_bKeepAfterSave;
	// InTimeTec: End
// Attributes
public:
	//KPM, 3/1/2005, This will lock the batch when the module starts up
	bool				OELockBatchOnStartup(CString szBatchCode);
	bool				FillListWithData(CObject *pObjVal, CString szTableNumber);
	bool				LockRecord(enuRecordProc nType, CString szModule, CString szForm, CStringArray *strCommandValues, CXMLParams::enmConnectionType nServer = CXMLParams::CN_COMPANY, CXML::enuExclusive uinExclusive = CXML::NONEXCLUSIVE, CString* szReturnPointer = NULL, CString szGUIDOverride = _T("")); // begbert 02-15-2006 VS8: warning C4482
	CString				LockBatchExclusive(CString szBatchCode = _T(""));
	CString				UnlockBatchExclusive(CString szBatchCode = _T(""));
	CString				UnlockBatchPendingExclusive(CString szBatchCode = _T(""));
private:
	CString				GetBatchTable();
public:
	//Be careful when changing optional parameter default values.  Otherwise code could break.
	//Instead, add another overload.
	_bstr_t				SendXML(bool bShowErrorDialog = true, UINT uinSendXMLType = SXM_SAMEPROCESS, bool bShowViewer = true, stuErrorReturn* pErrorReturn = NULL, CString szServerCallDisplay = _T(""));
	_bstr_t				SendXML(stuErrorReturn* pErrorReturn, bool bShowErrorDialog = true, UINT uinSendXMLType = SXM_SAMEPROCESS, bool bShowViewer = true, CString szServerCallDisplay = _T(""));
	//Ensure changes in below method default values, be reflected in 4th overload.
	_bstr_t				SendXML(CString szCommand, CString szData = _T(""), bool bShowErrorDialog = true, UINT uinSendXMLType = SXM_SAMEPROCESS, bool bShowViewer = true, stuErrorReturn* pErrorReturn = NULL, CString szServerCallDisplay = _T(""));
	_bstr_t				SendXML(const CString& szCommand, const CString& szData, stuErrorReturn& clsErrorReturn);

	void				MakeXMLConnection(CString szType, CString szTimeout, CString szPoolTime, UINT uinConnectionType = CTY_COMPANYSERVER, bool bSchema = false, bool bMaintForm = false);
	UINT				GetServerType(CXMLParams::enmConnectionType p_nServer = CXMLParams::CN_COMPANY); // begbert 02-15-2006 VS8: warning C4482
	CXMLParams::enmConnectionType GetServerType(UINT p_uinConnectionType = CTY_COMPANYSERVER);
	//KPM, some params not used now, made shortcut method but left original for backwards compatibility
	void				MakeXMLConnection(UINT uinConnectionType, bool bSchema = false);

	void				MakeXMLCommand(CString szCommand, CString szCommandType, CString szModule, CString szObjTable);
	void				MakeXMLCommand(CString szCommand, CString szModule, CString szObjTable);
	void				GetXMLFormat();
	void				MakeNode(CString szSingle, CString szInner = _T(""), int nSpace = 0, int nType = 0, bool bPreserveSpace = false);
	// begbert 03-14-2007 : added strExtraReturnFields to GetXMLData to allow extra fields to be returned by autofills.
	bool				GetXMLData(int nDirection, CString szModule, CString szForm, CStringArray *strCommandValues, CString szControlID, CString szXMLTable = _T(""), UINT uinConnectionType = CTY_COMPANYSERVER, bool bHeaderOnly = false, CStringArray* strExtraReturnFields = NULL);
	void				MakeCOMPANYINFO(void);
	bool				ChangeXMLData(enuRecordProc nType, CString szModule, CString szForm, CStringArray *strCommandValues, UINT uinConnectionType = CTY_COMPANYSERVER, bool bNoInsert = false, bool bUpdateOnly = false, bool bUseLowerCaseServerConnection = false);
	bool				BtnSave(enmFormModes nMode, CString szModule, CString szForm, CStringArray *strCommandValues, bool bUnlock = true, bool bUpdateOnly = false);
	enmFormModes		BtnNextPrevious(enmFormModes nMode, bool bIsDirty, int nDirection, CString szModule, CString szForm, CStringArray* strCommandValues, CString szControlID, CString szXMLTable = _T(""));
	enmFormModes		BtnEdit(enmFormModes nMode, CString szModule, CString szForm, CStringArray *strCommandValues, CString szControlID, CString szXMLTable = _T(""));
	bool				AutoFill(CString strName, CString szModule, CString szForm, CStringArray *strCommandValues, CString szControlID = _T("0"));
	bool				FillControls(CString szControlID, int nDirection);
	bool				FillGridWithData(CObject *pObjVal, CString szTableNumber);	//Use to fill a grid with data AES
	bool				ClearControls(CString szControlID);				//Used to clear the controls to blank before receiving new data, onBtnClear, and on Addmode AES
	bool				ClearGridCells(CObject *pObjVal);
	bool				SetReportXMLParameters(CString szModule, CString szReportName, CStringArray *strCommandValues, UINT uinConnectionType = CTY_COMPANYSERVER);
	bool				SetReportXMLParameters(const CString& szModule, const CString& szReportName, const CString& szDMReportName, const CStringArray& strCommandValues, UINT uinConnectionType = CTY_COMPANYSERVER);
	static CString		FormatFromXMLDate(CString XMLDate);
	static CString		FormatFromXMLDateTime(CString XMLDateTime, BOOL bolExcludeSeconds = FALSE);
	static COleDateTime FormatFromXMLToOLEDate(CString XMLDate);
	bool				CreateCurrencyEdit(CObject* pObjVal, bool bGrid = false);
	bool				CreateNumberEdit(CObject* pObjVal, bool bGrid = false);
	bool				CreateCharEdit(CObject* pObjVal, bool bGrid = false);
	void				GetGridData(enuRecordProc nType);
	void				GetListData(enuRecordProc nType);
	void				EnableDisable(enuEnable nType, CString szControlID);
	void				FillGridFromCDataTable(BSTR szData, CObject* pObjVal, CString szTableNumber);
	static bool			FillDataSet(CDataSet* pDS, CXMLParams* pXMLCommand, CXMLParams* pXMLData, stuErrorReturn* pErrorReturn = NULL);
	static bool			FillXMLParams(CXMLParams* pParams, CXMLParams* pXMLCommand, CXMLParams* pXMLData, stuErrorReturn* pErrorReturn = NULL);
	// InTimeTec: 11/21/2013: HS : Defect 9074 : D_IN_Save IN Stock Information from the Stock Item Maintenance Screen When Creating New Item
	// Added a flag to clear the controls after save. defaults to true.
	void				MySetElement(CString szTable, CString strKey, CObject* pObjVal, int pnVal, enuControlType nType, bool bValXML = true, bool bClearAfterSave = true);

	static CString		MakeRootElement(const CString& szXML);
	static CString		MakeTableElement(const CString& szXMLRecord);
	static CString		MakeRecordElement(const CString& szXMLElements);
	static CString		MakeElement(const CString& szTag, const CString& szData);

	_bstr_t				GetXMLNodeFormat(CString szSingle, CString szInner, bool bPreserveSpace = false);
	struct COMBOLIST
	{
		CString			szField;
		CString			szValue;

	} ComboList;

	//MSXML2::IXMLDOMDocumentPtr	m_docPtr;

	CCMSXMLDomDoc::ComponentTypePtr  m_docPtr;

	//ATLWRAPPERLib::IATLWRAPPtr	m_pATLWrapper;
	/*SXMLPARS::_CXMLCommandParserPtr m_pSXMLPARSWrapper;*/

	CCMSOXNumericEdit*	m_pNumEdit;					// numeric edit
	CCMSOXEdit*			m_pCharEdit;				// character edit
	CCMSOXCurrencyEdit*	m_pCurEdit;					// currency edit
	COleDateTime		m_DateTime;					// date time control
	CString				m_szXMLUserInfo;			// use date for locking

public:

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CXML)
		//}}AFX_VIRTUAL

	// Implementation
public:
	void				KillTheDoc();
	_bstr_t				GetReturnValue(void) const { return m_szReturn; }
	void				ClearReturnValue(void);

	//Register an implementation of IXMLDataCallBackProvider interface.
	void				RegisterDataCallBackProvider(IXMLDataCallBackProvider* pIProvider) { m_IDataCallBackProvider = pIProvider; }
	IXMLDataCallBackProvider*	GetDataCallBackProvider() { return m_IDataCallBackProvider; }
	bool				HasDataCallBackProvider() { return m_IDataCallBackProvider != NULL; }
	//Register an implementation of IXMLDataHookProvider interface.
	void				RegisterDataHookProvider(IXMLDataHookProvider* pIProvider) { m_IDataHookProvider = pIProvider; }
	IXMLDataHookProvider*		GetDataHookProvider() { return m_IDataHookProvider; }
	bool				HasDataHookProvider() { return m_IDataHookProvider != NULL; }
	//Register an implementation of ICMSResetProvider interface.
	void				RegisterResetProvider(ICMSResetProvider* pIProvider) { m_IContainerReset = pIProvider; }
	ICMSResetProvider *		GetResetProvider() { return m_IContainerReset; }
	bool				HasResetProvider() { return m_IContainerReset != NULL; }

	//sb - 21-jun-2004 - implement IPublisher interface
	virtual void Subscribe(CMS::ISubscriber * pSubscriber);
	virtual void Unsubscribe(CMS::ISubscriber * pSubscriber);

	//friend __declspec(dllexport) CXML& operator+= (CXML& objPublisher, CMS::ISubscriber * pSubscriber);		//equivalent to Subscribe
	//friend __declspec(dllexport) CXML& operator-= (CXML& objPublisher, CMS::ISubscriber * pSubscriber);		//equivalent to Unsubscribe


	// Generated message map functions
protected:
	struct MAPELEMENT
	{
		CObject*		pObjVal;
		int				pnVal;
		int				nType;
		bool			bValXML;
		bool			bFillFlag;
		bool			bClearOnSave;	// InTimeTec: 11/21/2013: HS : Defect 9074 : D_IN_Save IN Stock Information from the Stock Item Maintenance Screen When Creating New Item
	} mapElement;

	MAPELEMENT			m_pMyElement;

	typedef std::map<CString, MAPELEMENT>  KEYTOELEMENTMAP;
	typedef std::map<CString, KEYTOELEMENTMAP> GROUPTOCTRLSMAP;

	GROUPTOCTRLSMAP m_clsMapCtrlGroups;

	void GetCtrlMapForGroup(const CString& szGroup, KEYTOELEMENTMAP*& pCtrlGroupMap);
	void ClearGroupElements(KEYTOELEMENTMAP& clsKeyToElementMap);
	void EnableElement(MAPELEMENT& stElement, bool bEnable);
	void EnableGroupElements(KEYTOELEMENTMAP& clsKeyToElementMap, bool bEnable);
	BOOL LookupElement(const CString& szGroup, const CString& szKey, MAPELEMENT& clsElement);

	IXMLDataCallBackProvider*	m_IDataCallBackProvider;
	IXMLDataHookProvider*		m_IDataHookProvider;
	ICMSResetProvider *			m_IContainerReset;

	mutable CCMSPublisherDelegate	 m_objPublisher;

	virtual void Notify(CCMSMessage& objMessage);
	virtual void Notify(void);
	void NotifyClearField(const CString& szTable = _T(""), const CString& szFieldName = _T(""));
	void NotifyFieldValue(const CString& szTable, const CString& szFieldName, CString& szFieldValue);
	void NotifyRecordDeleted();

	//This variable holds the return from the server component
	_bstr_t				m_szReturn;

	//PGP(08/04/2004) Handles Fetch / Save from a CCMSDBObject type
	void FillValuesToCMSDBObjects();
	void GetValuesFromCMSDBObjects();
	void ClearCMSDBObjects();

	void MarkElementsBeforeFill(KEYTOELEMENTMAP* pKeyToElementMap);
	//Clear contents of all elements in the specified control group
	//which does not have the bFillFlag set.
	void ClearMissedElements(KEYTOELEMENTMAP* pKeyToElementMap);
	//Clear contents of this element.
	void ClearElement(MAPELEMENT& stElement);
	//{{AFX_MSG(CXML)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
public:
	void RemoveAllElementsFromMap();
	bool RemoveElementFromMap(const CString& szGroupID, const CString& szKey);

	//Purposely made this return a copy.  That way no other code steps on this.
	stuErrorReturn GetErrorReturn() { return m_clsErrorReturn; }
	UINT GetErrorReturnID() { return m_clsErrorReturn.uiErrorID; }

	// begbert 10-31-2017 PBI 36450 : These return the data fields returned by the last XMLGetData call.
	const std::map<CString, std::map<CString, CString>>& GetGottenData() { return m_GottenData; }
	const std::map<CString, CString>& GetGottenData(const CString& table) { return m_GottenData[table]; }

private:
	void SendNotifyMsg();
	ICMSGrid* GetCMSGrid(CObject* pObjVal);
	stuErrorReturn m_clsErrorReturn;
	bool bMakeServerCallLowerCaseToMatchNewCode;
	std::map<CString, std::map<CString, CString>> m_GottenData;	// begbert 10-31-2017 PBI 36450 : This stores the data fields returned by the last XMLGetData call.
};
