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
 /*
	 Collection of classes and structures which fall under po entry layout.
	 For ease of use and maintainability, all of the related classes are declared in this file.
	 (Downside would be difficulty in finding classes within CMSPOEntryLayout,
	 but can be traded for compile time and having to open that many files)
	 POForms and POEForms commonly uses CMSPOEntryLayout. SetupPOEntry Layout form
	 in POForms defines its own additional structures to maintain control related information
	 which was not put into these classes because they dont make sense in the generic purpose
	 of these classes which is to hold config information.
 */

#pragma once

 //Why do we need this #pragma disable?
 //http://www.dotnet247.com/247reference/msgs/1/7775.aspx
 //(or search for "C4702 xtree release" )
#pragma warning(disable:4702)

#include "POShrdConstants.h"
#include "../CMSDll/CMSDBObject.h"

namespace CCMSPOEntryLayout_flags
{
	enum enuFlags
	{

		// control enabling/defaults
		eVISIBLE = 0, // 0x oo00
		eDISABLED = 1, // 0x oo01
		eHIDDEN = 2, // 0x oo10

		eCHECKED = 4, // 0x o1oo
		eUNCHECKED = 0, // 0x o0oo

		eNA = 128, // only valid for the checkboxes; makes the field readonly and hides the checkboxes.


		// field properties
		eOPTIONAL = 0, // 0x ooo0
		eMANDATORY = 1, // 0x ooo1

		eMOBILE = 0, // 0x oo0o
		eFIXEDCOL = 2, // 0x oo1o
	};
}

using namespace CCMSPOEntryLayout_flags;

class CCMSPOEntryLayout
{

public:
	//Data structures for db tables.
	//Encapsulate a database table row here (SA_ScreenDetail)
	class CPOScreenDetailRecord : public CCMSDBObject
	{
	public:
		static LPCTSTR	m_arszFieldNames[];
		static INT32	m_arnFieldTypes[];
		enum
		{
			DBF_STR_SCREENCODE = 0,
			DBF_BYT_ENTRYSCREEN,
			DBF_BYT_TAB,
			DBF_BYT_ROW,
			DBF_BYT_LINETYPE,
			DBF_STR_UDF_ID,
			DBF_BOL_DISPLAY,
			DBF_BOL_ALLOWEDIT,
			DBF_BOL_SKIPCOLUMN,
			DBF_STR_CAPTION,
			DBF_INT_COLUMNWIDTH,
			DBF_INT_ORDER,
			DBF_MAX_FIELD_COUNT
		};
		CPOScreenDetailRecord();
	};
	//Encapsulate a database table row here (PO_ScreenHeader)
	class CPOScreenHeaderRecord : public CCMSDBObject
	{
		static LPCTSTR	m_arszFieldNames[];
		static INT32	m_arnFieldTypes[];
	public:
		enum
		{
			DBF_BYT_MODULE,
			DBF_BOL_CUSTPROMPT,
			DBF_BOL_FINHISTINFO,
			DBF_BOL_CUSTNOTES,
			DBF_BOL_CUSTBILLING,
			DBF_BOL_CUSTSHIPPING,
			DBF_BOL_TRANHISTINFO,
			DBF_BOL_LAYAWAY,
			DBF_BOL_HIDEVENDOR,
			DBF_BOL_HIDEHEADER,
			DBF_BOL_HIDELANDING,
			DBF_BOL_HIDEINVOICING,
			DBF_MAX_FIELD_COUNT
		};
		CPOScreenHeaderRecord();
	};

public:


	class CColumnItem;
	class CColLineData
	{
		friend class CCMSPOEntryLayout::CColumnItem;
	private:
		CColLineData()
		{
			m_nFieldID = m_nFlags = 0;
			m_bytLineTypes = 0;
			m_szFieldName = m_szFieldDesc = m_szDefHeader = m_szHeader = _T("");
			m_bSkipColumn = m_bAllowEdit = false;
		}


		INT32	m_nFieldID;
		BYTE	m_bytLineTypes;
		CString m_szFieldName;
		CString m_szFieldDesc;

		INT32	m_nFlags;
		CString m_szDefHeader;

		CString m_szHeader;
		bool	m_bSkipColumn;
		bool	m_bAllowEdit;

	protected:
		BYTE	GetFlagsMobility() const { return (BYTE)((m_nFlags >> 0) & 0xFF); }
		BYTE	GetFlagsAllowInput() const { return (BYTE)((m_nFlags >> 4) & 0xFF); }
		BYTE	GetFlagsSkipColumn() const { return (BYTE)((m_nFlags >> 8) & 0xFF); }
		BYTE	GetFlagsHeading() const { return (BYTE)((m_nFlags >> 12) & 0xFF); }
		BYTE	GetFlagsColWidth() const { return (BYTE)((m_nFlags >> 16) & 0xFF); }
	};


	// this object is used in the std:vector object ORDEREDDETAIL
	class AFX_EXT_CLASS CColumnItem
	{
	protected:
		CString m_szColumnName;
		BYTE	m_bytRow;
		bool	m_bVisible;
		int		m_intColumnNo;
		int		m_intDefColumnNo;
		int		m_intDefColumnWidth;

		int		m_intColumnWidth;
		CString m_szUDF_ID;

		std::vector<CColLineData> m_aryLineData;

		void CopyFrom(const CColumnItem& rhs);

		const CColLineData& GetInfo(UINT i) const;
		CColLineData& GetInfo(UINT i);
	public:

		typedef std::pair<INT32, CString> TKey;	// ColID and UDF_ID

		bool operator== (const TKey& key) const;
		//		static bool OrderFieldNamePtrsPred	(const CColumnItem* lhs, const CColumnItem* rhs);
		static bool OrderColPtrsPred(const CColumnItem* lhs, const CColumnItem* rhs);
		static bool OrderColsPred(const CColumnItem& lhs, const CColumnItem& rhs) { return OrderColPtrsPred(&lhs, &rhs); }


		CColumnItem();
		CColumnItem(const CColumnItem& rhs) { CopyFrom(rhs); }
		CColumnItem& operator = (const CColumnItem& rhs) { CopyFrom(rhs); return *this; }
		void LoadFromRecord(CCMSPOEntryLayout::CPOScreenDetailRecord& clsRecord);	// begbert 04-30-2009 : Changed functionality from being a true assignment, and so changed the function name accordingly.
		void RestoreDefaults();

		void AddInfo(const INT32& nID, const CString& szHeader, const CString& szDesc, const int& intColumnNo, const int& intColumnWidth,
			BYTE bytFlagsAllowInput = eCHECKED,
			BYTE bytFlagsSkipColumn = eUNCHECKED,
			BYTE bytFlagsHeading = eVISIBLE,
			BYTE bytFlagsColWidth = eVISIBLE,
			BYTE bytFlagsMobility = eOPTIONAL,
			CString UDF_ID = _T(""),
			BYTE bytLineTypes = POEntryLayout::LineTypes::AllTypes);

		void	SetColumnName(CString szColumnName) { m_szColumnName = szColumnName; }


		TKey	GetKey(BYTE bytLineTypes = POEntryLayout::LineTypes::AllTypes) const { return TKey(GetInfo(GetIndexMatchingLineType(bytLineTypes)).m_nFieldID, m_szUDF_ID); }

		UINT	GetNumSubFields() const { return (UINT)m_aryLineData.size(); }
		UINT	GetIndexMatchingLineType(BYTE bytLineTypes) const;

		const CString& GetColumnName() const { return m_szColumnName; }
		INT32	GetID(UINT i) const { return GetInfo(i).m_nFieldID; }
		CString	GetFieldID(UINT i) const { return GetInfo(i).m_szFieldName; }
		CString	GetFieldName(UINT i) const { return GetInfo(i).m_szFieldName; }
		CString	GetFieldDesc(UINT i) const { return GetInfo(i).m_szFieldDesc; }
		BYTE	GetFlagsMobility(UINT i) const { return GetInfo(i).GetFlagsMobility(); }
		BYTE	GetFlagsAllowInput(UINT i) const { return GetInfo(i).GetFlagsAllowInput(); }
		BYTE	GetFlagsSkipColumn(UINT i) const { return GetInfo(i).GetFlagsSkipColumn(); }
		BYTE	GetFlagsHeading(UINT i) const { return GetInfo(i).GetFlagsHeading(); }
		BYTE	GetFlagsColWidth(UINT i) const { return GetInfo(i).GetFlagsColWidth(); }
		CString GetUDF_ID() const { return m_szUDF_ID; }

		bool    IsVisible() const { return m_bVisible; }
		BYTE	GetRow() const { return m_bytRow; }
		int		GetColumnNo() const { return m_intColumnNo; }
		CString GetHeader(UINT i) const { return GetInfo(i).m_szHeader; }
		bool	GetSkipColumn(UINT i) const { return GetInfo(i).m_bSkipColumn; }
		bool	GetAllowEdit(UINT i) const { return GetInfo(i).m_bAllowEdit; }
		int		GetColumnWidth() const { return m_intColumnWidth; }

		bool	IsValid() const { return m_aryLineData.size() != 0; }
		bool    IsMandatory() const { return (GetFlagsMobility(0) & eMANDATORY) == eMANDATORY; }
		bool    IsMobile() const { return (GetFlagsMobility(0) & eFIXEDCOL) != eFIXEDCOL; }

		void    SetIsVisible(bool bVisible) { m_bVisible = bVisible; }
		void	SetRow(BYTE	bytRow) { m_bytRow = bytRow; }
		void	SetColumnNo(int	intColumnNo) { m_intColumnNo = intColumnNo; }
		void	SetColumnWidth(int	intColumnWidth) { m_intColumnWidth = intColumnWidth; }
		void	SetHeader(const CString& szHeader, UINT i) { GetInfo(i).m_szHeader = szHeader; }
		void	SetSkipColumn(bool	bSkipColumn, UINT i) { GetInfo(i).m_bSkipColumn = bSkipColumn; }
		void	SetAllowEdit(bool	bAllowEdit, UINT i) { GetInfo(i).m_bAllowEdit = bAllowEdit; }
	};

	typedef std::vector<CColumnItem> ORDEREDCOLUMNS;

	/*
		class CVendor
		{
		public:
			bool	m_bHideCustomerInfo ;
			bool	m_bFinancialHistory ;
			bool	m_bTransactionHistory ;
			bool	m_bNotes ;
			bool	m_bBillingAddress ;
			bool	m_bShippingAddress ;
			bool	m_bHideHeader;
		public:
			void				LoadDefaults() ;
			CVendor&			operator = (const CVendor& rhs) ;
			CVendor&			operator = (CPOScreenHeaderRecord& rhs) ;
			bool				LoadFrom(CDataSet& clsDataSet) ;
		} m_clsVendor ;
	*/


	// class used to handle detail information
public:

	class CColumnItems
	{
	protected:
		struct FindVisible
		{
			const int m_nRow;
			FindVisible(const int nRow) :m_nRow(nRow) {}
			bool operator()(const ORDEREDCOLUMNS::value_type& vt)
			{
				return vt.GetRow() == m_nRow && vt.IsVisible();
			}
		};
	protected:
		ORDEREDCOLUMNS m_vOrderedCols;

		bool LoadItem(CCMSPOEntryLayout::CPOScreenDetailRecord& clsRecord);	// begbert 04-30-2009 : The functionality has changed from being a normal set to being more load-specific, and so I changed the function name accordingly.

		// helper functions
		int Find(const CColumnItem::TKey& key) const;

	public:
		CColumnItems(BYTE bytTab) : m_bytTab(bytTab) { }
		void AddColumn(const INT32& nFieldID, const UINT& uiHeaderID, const UINT& uiDescID, const BYTE& bytColumnWidth,
			BYTE bytFlagsAllowInput = eCHECKED,
			BYTE bytFlagsSkipColumn = eUNCHECKED,
			BYTE bytFlagsHeading = eVISIBLE,
			BYTE bytFlagsColWidth = eVISIBLE,
			BYTE bytFlagsMobility = eOPTIONAL,
			BYTE bytLineTypes = POEntryLayout::LineTypes::AllTypes,
			INT32 nColID = 0,
			CString szColumnName = _T(""));

		void AddUDFColumns(const INT32& nFieldID, bool bIsHeader, bool bIsDescription, const UINT& uiDescID, const BYTE& bytColumnWidth,
			BYTE bytFlagsAllowInput = eCHECKED,
			BYTE bytFlagsSkipColumn = eUNCHECKED,
			BYTE bytFlagsHeading = eVISIBLE,
			BYTE bytFlagsColWidth = eVISIBLE,
			BYTE bytFlagsMobility = eOPTIONAL);

		CColumnItems&		operator = (const CColumnItems& rhs);

		bool GetItem(const INT32& nID, const CString& szUDF_ID, CColumnItem& clsItem) const;
		bool SetItem(const INT32& nID, const CString& szUDF_ID, CColumnItem& clsItem);
		CColumnItem* GetItem(const INT32& nID, CString szUDF_ID = _T(""));
		const CColumnItem* GetItem(const INT32& nID, CString szUDF_ID = _T("")) const;

		bool LoadFrom(CDataSet& clsDataSet);
		void RestoreDefaults();
		virtual void AutoFixData();
		const size_t GetCount() const { return m_vOrderedCols.size(); }
		CColumnItem* operator[](int i) { return &m_vOrderedCols[(size_t)i]; }
		const CColumnItem* operator[](int i) const { return i >= 0 && i < (int)m_vOrderedCols.size() ? &m_vOrderedCols[(size_t)i] : NULL; }
		bool operator<(const CColumnItems& rhs);
		bool IsEmpty(const int nRow) const;
	private:
		BYTE m_bytTab;
	};


	// These classes define which column items are part of each section.
	class CPURVendorItems : public CColumnItems { public: CPURVendorItems(); virtual void AutoFixData(); } m_clsPURVendorItems;
	class CPURHeaderItems : public CColumnItems { public: CPURHeaderItems(); } m_clsPURHeaderItems;
	class CPURDetailItems : public CColumnItems { public: CPURDetailItems(); } m_clsPURDetailItems;
	class CRCVHeaderItems : public CColumnItems { public: CRCVHeaderItems(); } m_clsRCVHeaderItems;
	class CRCVDetailItems : public CColumnItems { public: CRCVDetailItems(); } m_clsRCVDetailItems;
	class CRCVLandingItems : public CColumnItems { public: CRCVLandingItems(); } m_clsRCVLandingItems;
	class CRCVInvoicingItems : public CColumnItems { public: CRCVInvoicingItems(); } m_clsRCVInvoicingItems;


	//CCMSSalesEntryLayout member functions.
private:
	CGBLSystemInformation* si;
	BYTE		m_bytModule;
	CString		m_szScreenCode;

	bool		m_bHideVendorPane;
	bool		m_bHideHeaderPane;
	bool		m_bHideLandingPane;
	bool		m_bHideInvoicingPane;

public:
	CCMSPOEntryLayout(BYTE bytModule = (BYTE)SCREEN_MODULE_PUR);
	~CCMSPOEntryLayout(void);

	bool LoadFromDB(const CString& szScreenCode, BYTE bytModule = 1, const CString& szRegisterCode = _T(""));
	bool LoadFrom(CDataSet& clsDataSet);
	//
	void SetScreenCode(const CString& szCode) { m_szScreenCode = szCode; }
	void SetModule(const BYTE& bytModule) { m_bytModule = bytModule; }
	// begbert 01-12-2009 1-31687 : Fixed the 'SetHidePane' functions, which were effectless.
	void SetHideVendorPane(const bool bValue) { m_bHideVendorPane = bValue; }
	void SetHideHeaderPane(const bool bValue) { m_bHideHeaderPane = bValue; }
	void SetHideLandingPane(const bool bValue) { m_bHideLandingPane = bValue; }
	void SetHideInvoicingPane(const bool bValue) { m_bHideInvoicingPane = bValue; }
	//
	const CString&	GetScreenCode() const { return m_szScreenCode; }
	const BYTE&		GetModule() const { return m_bytModule; }
	const bool		GetHideVendorPane() const { return m_bHideVendorPane; }
	const bool		GetHideHeaderPane() const { return m_bHideHeaderPane; }
	const bool		GetHideLandingPane() const { return m_bHideLandingPane; }
	const bool		GetHideInvoicingPane() const { return m_bHideInvoicingPane; }

	CColumnItems& GetVendorItems() { return m_clsPURVendorItems; }
	CColumnItems& GetHeaderItems() { if (IsPUR()) return m_clsPURHeaderItems; else return m_clsRCVHeaderItems; }
	CColumnItems& GetDetailItems() { if (IsPUR()) return m_clsPURDetailItems; else return m_clsRCVDetailItems; }
	CColumnItems& GetLandingItems() { return m_clsRCVLandingItems; }
	CColumnItems& GetInvoiceItems() { return m_clsRCVInvoicingItems; }

	const bool IsPUR() const { return m_bytModule == SCREEN_MODULE_PUR; }
	const bool IsRCV() const { return m_bytModule == SCREEN_MODULE_RCV; }

	void ResetToDefaults()
	{
		m_clsPURVendorItems.RestoreDefaults();
		m_clsPURHeaderItems.RestoreDefaults();
		m_clsPURDetailItems.RestoreDefaults();
		m_clsRCVHeaderItems.RestoreDefaults();
		m_clsRCVDetailItems.RestoreDefaults();
		m_clsRCVLandingItems.RestoreDefaults();
		m_clsRCVInvoicingItems.RestoreDefaults();
	}
};

template<> AFX_INLINE UINT AFXAPI HashKey(CCMSPOEntryLayout::CColumnItem::TKey key)
{
	CString text;
	text.Format(_T("%i, %s"), key.first, key.second);
	// default identity hash - works for most primitive values
	return HashKey((LPCWSTR)text);
}



/*
//Forms implement this for configuration.
interface ITenderConfigLayout
{
	virtual void ConfigDialog(const INT32& nTenderType, const INT32& nPropType, CCMSPOEntryLayout::CTenderItem& clsItem) = 0 ;
};
*/
