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
#include "CMSPOEntryLayout.h"
#include "POShrdConstants.h"
#include "../cmsdll/cmsuserdefinedfields.h"

using namespace POEntryLayout;


//Database mapping object for a SA_ScreenDetail database table row
//Define the array for data - minimal mapping for test now. Detail items need more data.
LPCTSTR CCMSPOEntryLayout::CPOScreenDetailRecord::m_arszFieldNames[] =
{
	_T("strScreenCode"), _T("bytEntryScreen"), _T("bytTab"), _T("bytRow"),
	_T("bytLineType"), _T("UDF_ID"), _T("bolDisplay"), _T("bolAllowEdit"),
	_T("bolSkipColumn"), _T("strCaption"), _T("intColumnWidth"), _T("intOrder"),
};


//data types coming in - sync with field names above.
INT32 CCMSPOEntryLayout::CPOScreenDetailRecord::m_arnFieldTypes[] =
{
	CCMSVariant::T_STRING, CCMSVariant::T_STRING, CCMSVariant::T_BYTE, CCMSVariant::T_BYTE,
	CCMSVariant::T_BYTE, CCMSVariant::T_STRING, CCMSVariant::T_BOOL, CCMSVariant::T_BOOL,
	CCMSVariant::T_BOOL, CCMSVariant::T_STRING, CCMSVariant::T_LONG, CCMSVariant::T_LONG,
};


CCMSPOEntryLayout::CPOScreenDetailRecord::CPOScreenDetailRecord()
	:CCMSDBObject(sizeof(m_arszFieldNames) / sizeof(LPCTSTR), m_arnFieldTypes, m_arszFieldNames, NULL, NULL)
{
	Initialize();
}

//Database mapping object for a PO_ScreenHeader database table row
LPCTSTR CCMSPOEntryLayout::CPOScreenHeaderRecord::m_arszFieldNames[] =
{
	_T("bytModule"), _T("bolCustPrompt"), _T("bolFinHistInfo"), _T("bolCustNotes"),
	_T("bolCustBilling"), _T("bolCustShipping"), _T("bolTranHistInfo"), _T("bolLayaway"),
	_T("bolHideHeader")
};
//data types coming in - sync with field names above.
INT32 CCMSPOEntryLayout::CPOScreenHeaderRecord::m_arnFieldTypes[] =
{
	CCMSVariant::T_BYTE, CCMSVariant::T_BOOL, CCMSVariant::T_BOOL, CCMSVariant::T_BOOL,
	CCMSVariant::T_BOOL, CCMSVariant::T_BOOL, CCMSVariant::T_BOOL, CCMSVariant::T_BOOL,
	CCMSVariant::T_BOOL
};

CCMSPOEntryLayout::CPOScreenHeaderRecord::CPOScreenHeaderRecord()
	:CCMSDBObject(sizeof(m_arszFieldNames) / sizeof(LPCTSTR), m_arnFieldTypes, m_arszFieldNames, NULL, NULL)
{
	Initialize();
}
/*
//Database mapping object for a SA_PrinterFormat database table row
LPCTSTR CCMSPOEntryLayout::CSAPrinterFormatRecord::m_arszFieldNames[] =
{
	//KPM.2005.03.29 Changed strInvoiceFormat to strFormatCode
	_T("bytModule"), _T("bytTenderNo"), _T("bytTranTypeFlag"), _T("intNoOfCopies"),
	_T("intPrinterNumber"), _T("strFormatCode"), _T("strScreenCode"), _T("strPrinterName"), _T("bolOE"), _T("bolCustom"), _T("strReportName")
};
//data types coming in - sync with field names above.
INT32 CCMSPOEntryLayout::CSAPrinterFormatRecord::m_arnFieldTypes[] =
{
	CCMSVariant::T_BYTE, CCMSVariant::T_BYTE, CCMSVariant::T_BYTE, CCMSVariant::T_LONG,
	CCMSVariant::T_LONG, CCMSVariant::T_STRING, CCMSVariant::T_STRING, CCMSVariant::T_STRING, CCMSVariant::T_BOOL, CCMSVariant::T_BOOL, CCMSVariant::T_STRING
};

CCMSPOEntryLayout::CSAPrinterFormatRecord::CSAPrinterFormatRecord()
:CCMSDBObject(sizeof(m_arszFieldNames)/sizeof(LPCTSTR), m_arnFieldTypes, m_arszFieldNames, NULL, NULL)
{
	Initialize() ;
}
*/

//*************************************************************************************
//CCMSPOEntryLayout::CColumnItem
//*************************************************************************************

// begbert 04-30-2009 : Altered to not load over the values of disabled/hidden fields;
//   such fields should always retain their hardcoded defaults, recartless of loaded data.
// I also changed the name to more clearly state that this function is for use in loading only;
//   because it won't copy over those noneditable fields, it's not a true assignment.
void CCMSPOEntryLayout::CColumnItem::LoadFromRecord(CCMSPOEntryLayout::CPOScreenDetailRecord& clsRecord)
{
	CCMSPOEntryLayout::CPOScreenDetailRecord* p; p = NULL;

	unsigned int i;
	for (i = 0; i < m_aryLineData.size(); i++)
	{
		CColLineData& data(m_aryLineData[i]);

		if (data.m_nFieldID == clsRecord.GetByteValue(p->DBF_BYT_LINETYPE))
		{
			m_bVisible = clsRecord.GetBoolValue(p->DBF_BOL_DISPLAY);
			m_bytRow = clsRecord.GetByteValue(p->DBF_BYT_ROW);
			m_intColumnNo = clsRecord.GetLongValue(p->DBF_INT_ORDER);
			if ((data.GetFlagsHeading() & eDISABLED) != eDISABLED &&
				(data.GetFlagsHeading() & eHIDDEN) != eHIDDEN)
				data.m_szHeader = clsRecord.GetStringValue(p->DBF_STR_CAPTION);
			if ((data.GetFlagsSkipColumn() & eDISABLED) != eDISABLED &&
				(data.GetFlagsSkipColumn() & eHIDDEN) != eHIDDEN)
				data.m_bSkipColumn = clsRecord.GetBoolValue(p->DBF_BOL_SKIPCOLUMN);
			if ((data.GetFlagsAllowInput() & eDISABLED) != eDISABLED &&
				(data.GetFlagsAllowInput() & eHIDDEN) != eHIDDEN)
				data.m_bAllowEdit = clsRecord.GetBoolValue(p->DBF_BOL_ALLOWEDIT);
			if ((data.GetFlagsColWidth() & eDISABLED) != eDISABLED &&
				(data.GetFlagsColWidth() & eHIDDEN) != eHIDDEN)
				m_intColumnWidth = clsRecord.GetLongValue(p->DBF_INT_COLUMNWIDTH);
			m_szUDF_ID = clsRecord.GetStringValue(p->DBF_STR_UDF_ID);
			if (m_szUDF_ID == _T("N/A") || m_szUDF_ID == _T("'N/A'")) m_szUDF_ID = _T("");
		}
	}
}


//Helper for copy constructor.
void CCMSPOEntryLayout::CColumnItem::CopyFrom(const CColumnItem& rhs)
{
	m_szColumnName = rhs.m_szColumnName;
	m_bytRow = rhs.m_bytRow;
	m_bVisible = rhs.m_bVisible;
	m_intDefColumnNo = rhs.m_intDefColumnNo;
	m_intColumnNo = rhs.m_intColumnNo;
	m_intDefColumnWidth = rhs.m_intDefColumnWidth;
	m_intColumnWidth = rhs.m_intColumnWidth;
	m_szUDF_ID = rhs.m_szUDF_ID;
	m_aryLineData = rhs.m_aryLineData;
}

const CCMSPOEntryLayout::CColLineData& CCMSPOEntryLayout::CColumnItem::GetInfo(UINT i) const
{
	if (i >= 0 && i < m_aryLineData.size()) return m_aryLineData[i];
	static const CCMSPOEntryLayout::CColLineData objBadData;
	return objBadData;
}
CCMSPOEntryLayout::CColLineData& CCMSPOEntryLayout::CColumnItem::GetInfo(UINT i)
{
	if (i >= 0 && i < m_aryLineData.size()) return m_aryLineData[i];
	static CCMSPOEntryLayout::CColLineData objBadData;
	return objBadData = CCMSPOEntryLayout::CColLineData();
}

UINT  CCMSPOEntryLayout::CColumnItem::GetIndexMatchingLineType(BYTE bytLineTypes) const
{
	for (unsigned int i = 0; i < m_aryLineData.size(); i++)
		if (m_aryLineData[i].m_bytLineTypes & bytLineTypes)
			return i;
	return 0;
}


//
CCMSPOEntryLayout::CColumnItem::CColumnItem()
{
	m_intDefColumnNo = m_intColumnWidth = m_intDefColumnWidth = m_intColumnNo = 0;
	m_bytRow = 0;
	m_bVisible = false;
}

void CCMSPOEntryLayout::CColumnItem::RestoreDefaults()
{
	m_bVisible = IsMandatory();
	m_bytRow = 0;
	m_intColumnNo = m_intDefColumnNo;
	m_intColumnWidth = m_intDefColumnWidth;
	for (unsigned int i = 0; i < m_aryLineData.size(); i++)
	{
		m_aryLineData[i].m_szHeader = m_aryLineData[i].m_szDefHeader;
		m_aryLineData[i].m_bSkipColumn = (m_aryLineData[i].GetFlagsSkipColumn() & eCHECKED) == eCHECKED;
		m_aryLineData[i].m_bAllowEdit = (m_aryLineData[i].GetFlagsAllowInput() & eCHECKED) == eCHECKED;
	}
}

//Init for a detail line type.
void CCMSPOEntryLayout::CColumnItem::AddInfo(const INT32& nID, const CString& szHeader, const CString& szDesc, const int& intColumnNo, const int& intColumnWidth,
	BYTE bytFlagsAllowInput /* =eCHECKED   */,
	BYTE bytFlagsSkipColumn /* =eUNCHECKED */,
	BYTE bytFlagsHeading    /* =eVISIBLE   */,
	BYTE bytFlagsColWidth   /* =eVISIBLE   */,
	BYTE bytFlagsMobility   /* =eOPTIONAL  */,
	CString UDF_ID          /* =_T("")     */,
	BYTE bytLineTypes       /* =POEntryLayout::LineTypes::AllTypes*/)
{
	// If this isn't a new column, use the existing mobility and column width, otherwise don't.
	if (IsValid())
	{
		bytFlagsMobility = GetFlagsMobility(0);
	}
	else
	{
		m_intDefColumnNo = intColumnNo;
		m_intDefColumnWidth = intColumnWidth;
		m_szColumnName = ((bytFlagsMobility & eMANDATORY) == eMANDATORY) ? RESSTRING(IDS_NOTES_STAR) + szHeader : szHeader;
	}
	m_szUDF_ID = UDF_ID;


	CColLineData ld;

	ld.m_nFieldID = nID;
	ld.m_bytLineTypes = bytLineTypes;
	ld.m_szFieldName = ld.m_szDefHeader = szHeader;
	ld.m_szFieldDesc = szDesc;

	// interpret the shortcut flag notations and deal with illegal flag combinations

	//    Interpret N/A
	if (bytFlagsAllowInput == eNA) bytFlagsAllowInput = eHIDDEN | eUNCHECKED;
	if (bytFlagsSkipColumn == eNA) bytFlagsSkipColumn = eHIDDEN | eCHECKED;

	//    SkipColumn inherits any of AllowInput's Hidden and Disabled flags, in additions to its own.
	bytFlagsSkipColumn |= bytFlagsSkipColumn & (eHIDDEN | eDISABLED);

	//    Any field that is hidden is also disabled.
	if ((bytFlagsAllowInput & eHIDDEN) == eHIDDEN) bytFlagsAllowInput |= eDISABLED;
	if ((bytFlagsSkipColumn & eHIDDEN) == eHIDDEN) bytFlagsSkipColumn |= eDISABLED;
	if ((bytFlagsHeading & eHIDDEN) == eHIDDEN) bytFlagsHeading |= eDISABLED;
	if ((bytFlagsColWidth & eHIDDEN) == eHIDDEN) bytFlagsColWidth |= eDISABLED;


	// pack the flags

	ASSERT((bytFlagsMobility & ~0xFF) == 0);
	ASSERT((bytFlagsAllowInput & ~0xFF) == 0);
	ASSERT((bytFlagsSkipColumn & ~0xFF) == 0);
	ASSERT((bytFlagsHeading & ~0xFF) == 0);
	ASSERT((bytFlagsColWidth & ~0xFF) == 0);

	ld.m_nFlags = (bytFlagsMobility << 0) |
		(bytFlagsAllowInput << 4) |
		(bytFlagsSkipColumn << 8) |
		(bytFlagsHeading << 12) |
		(bytFlagsColWidth << 16);

	//		BYTE  GetFlagsMobility  () const { return (m_nFlags >>  0) & 0xFF; }
	//		BYTE  GetFlagsAllowInput() const { return (m_nFlags >>  4) & 0xFF; }
	//		BYTE  GetFlagsSkipColumn() const { return (m_nFlags >>  8) & 0xFF; }
	//		BYTE  GetFlagsHeading   () const { return (m_nFlags >> 12) & 0xFF; }
	//		BYTE  GetFlagsColWidth  () const { return (m_nFlags >> 16) & 0xFF; }

	m_aryLineData.push_back(ld);

	RestoreDefaults();
}

//*************************************************************************************
//CCMSPOEntryLayout::CColumnItems
//*************************************************************************************
/*
CCMSPOEntryLayout::CColumnItems::CColumnItems()
{
}

//Make entries in ordered columns vector
void CCMSPOEntryLayout::CColumnItems::PopulateOrderedCols()
{
	m_vOrderedCols.clear() ;
	ORDEREDCOLUMNS::iterator it ;
	for(it = m_mapItems.begin(); it != m_mapItems.end(); it++)
		m_vOrderedCols.push_back(&it->second) ;
}
*/

//

void CCMSPOEntryLayout::CColumnItems::AddColumn(const INT32& nFieldID, const UINT& uiHeaderID, const UINT& uiDescID, const BYTE& bytColumnWidth,
	BYTE bytFlagsAllowInput /* =eCHECKED   */,
	BYTE bytFlagsSkipColumn /* =eUNCHECKED */,
	BYTE bytFlagsHeading    /* =eVISIBLE   */,
	BYTE bytFlagsColWidth   /* =eVISIBLE   */,
	BYTE bytFlagsMobility   /* =eOPTIONAL  */,
	BYTE bytLineTypes       /* =AllTypes   */,
	INT32 nColID            /* =0          */,
	CString szColumnName    /* =_T("")     */)
{
	CColumnItem clsItem;

	if (nColID == 0) nColID = nFieldID;

	if (GetItem(nColID, _T(""), clsItem))
	{
		// we found it!  Tack on the extra info and set it back into the list.
		clsItem.AddInfo(nFieldID, RESSTRING(uiHeaderID), RESSTRING(uiDescID), (int)m_vOrderedCols.size(), bytColumnWidth, bytFlagsAllowInput, bytFlagsSkipColumn, bytFlagsHeading, bytFlagsColWidth, bytFlagsMobility, _T(""), bytLineTypes);
		if (!szColumnName.IsEmpty())
		{
			clsItem.SetColumnName(clsItem.IsMandatory() ? RESSTRING(IDS_NOTES_STAR) + szColumnName : szColumnName);
		}
		SetItem(nColID, _T(""), clsItem);
	}
	else
	{
		// we didn't find it; add a new one.
		clsItem.AddInfo(nFieldID, RESSTRING(uiHeaderID), RESSTRING(uiDescID), (int)m_vOrderedCols.size(), bytColumnWidth, bytFlagsAllowInput, bytFlagsSkipColumn, bytFlagsHeading, bytFlagsColWidth, bytFlagsMobility, _T(""), bytLineTypes);
		if (!szColumnName.IsEmpty())
		{
			clsItem.SetColumnName(clsItem.IsMandatory() ? RESSTRING(IDS_NOTES_STAR) + szColumnName : szColumnName);
		}
		m_vOrderedCols.push_back(clsItem);
	}
}

void CCMSPOEntryLayout::CColumnItems::AddUDFColumns(const INT32& nFieldID, bool bIsHeader, bool bIsDescription, const UINT& uiDescID, const BYTE& bytColumnWidth,
	BYTE bytFlagsAllowInput /* =eCHECKED   */,
	BYTE bytFlagsSkipColumn /* =eUNCHECKED */,
	BYTE bytFlagsHeading    /* =eVISIBLE   */,
	BYTE bytFlagsColWidth   /* =eVISIBLE   */,
	BYTE bytFlagsMobility   /* =eOPTIONAL  */)
{
	UDFTableType::enuUDFTableType eTableType = bIsHeader ? UDFTableType::TableType_TranHeader : UDFTableType::TableType_TranDetail;
	CString strUDFCaption;

	CCMSUserDefinedFields& vecUDFs = CMS::GetSI()->ModuleSettings.POSettings.SelectedUDFs;
	CCMSUserDefinedFields::const_iterator it, end = vecUDFs.end();

	for (it = vecUDFs.begin(); it != end; it++)
	{
		if (it->TableType != eTableType) continue;
		if (bIsDescription && !it->IsUDFLookup) continue;
		CColumnItem clsItem;
		strUDFCaption.Format(_T("UDF%s%02d-%s"), bIsDescription ? _T("Desc") : _T(""), it->UDFNumber, it->Caption);
		clsItem.AddInfo(nFieldID, strUDFCaption, RESSTRING(uiDescID), (int)m_vOrderedCols.size(), bytColumnWidth, bytFlagsAllowInput, bytFlagsSkipColumn, bytFlagsHeading, bytFlagsColWidth, bytFlagsMobility, it->UDF_ID);
		m_vOrderedCols.push_back(clsItem);
	}
}

CCMSPOEntryLayout::CColumnItems& CCMSPOEntryLayout::CColumnItems::operator = (const CCMSPOEntryLayout::CColumnItems& rhs)
{
	m_vOrderedCols = rhs.m_vOrderedCols;
	return *this;
}

// returns -1 if not found.
int CCMSPOEntryLayout::CColumnItems::Find(const CColumnItem::TKey& key) const
{
	for (size_t i = 0; i < m_vOrderedCols.size(); i++)
		if (m_vOrderedCols[i] == key)
			return (int)i;
	return -1;
}
//Defect ID 1-30367, PGP(08-10-2009)
//returns true if there is atleast one visible column. false otherwise
bool CCMSPOEntryLayout::CColumnItems::IsEmpty(const int nRow) const
{
	return std::find_if(m_vOrderedCols.begin(), m_vOrderedCols.end(), FindVisible(nRow)) == m_vOrderedCols.end();
}


bool CCMSPOEntryLayout::CColumnItem::operator== (const CColumnItem::TKey& key) const
{
	// the UDF_ID and ANY of the infos' fieldIDs have to match.
	if (m_szUDF_ID != key.second) return false;
	for (unsigned int i = 0; i < m_aryLineData.size(); i++)
	{
		if (m_aryLineData[i].m_nFieldID == key.first)
			return true;
	}
	return false;
}


//
bool CCMSPOEntryLayout::CColumnItems::GetItem(const INT32& nID, const CString& szUDF_ID, CCMSPOEntryLayout::CColumnItem& clsItem) const
{
	int nIndex = Find(CColumnItem::TKey(nID, szUDF_ID));
	if (nIndex != -1)
	{
		clsItem = m_vOrderedCols[(size_t)nIndex];
		return true;
	}
	return false;
}

//
bool CCMSPOEntryLayout::CColumnItems::SetItem(const INT32& nID, const CString& szUDF_ID, CCMSPOEntryLayout::CColumnItem& clsItem)
{
	int nIndex = Find(CColumnItem::TKey(nID, szUDF_ID));
	if (nIndex != -1)
	{
		m_vOrderedCols[(size_t)nIndex] = clsItem;
		return true;
	}
	return false;
}

// begbert 04-30-2009 : changed the behavior slightly, to not load over the values of non-user-editable fields.
bool CCMSPOEntryLayout::CColumnItems::LoadItem(CCMSPOEntryLayout::CPOScreenDetailRecord& clsRecord)
{
	CColumnItem clsItem;
	INT32   nType = clsRecord.GetByteValue(CCMSPOEntryLayout::CPOScreenDetailRecord::DBF_BYT_LINETYPE);
	CString szUDF_ID = clsRecord.GetStringValue(CCMSPOEntryLayout::CPOScreenDetailRecord::DBF_STR_UDF_ID);
	if (szUDF_ID == _T("N/A") || szUDF_ID == _T("'N/A'")) szUDF_ID = _T("");
	if (GetItem(nType, szUDF_ID, clsItem))
	{
		clsItem.LoadFromRecord(clsRecord);
		return SetItem(nType, szUDF_ID, clsItem);
	}
	return false;
}

CCMSPOEntryLayout::CColumnItem* CCMSPOEntryLayout::CColumnItems::GetItem(const INT32& nID, CString szUDF_ID /*=_T("")*/)
{
	int nIndex = Find(CColumnItem::TKey(nID, szUDF_ID));
	return nIndex != -1 ? &m_vOrderedCols[(size_t)nIndex] : NULL;
}

const CCMSPOEntryLayout::CColumnItem* CCMSPOEntryLayout::CColumnItems::GetItem(const INT32& nID, CString szUDF_ID /*=_T("")*/) const
{
	int nIndex = Find(CColumnItem::TKey(nID, szUDF_ID));
	return nIndex != -1 ? &m_vOrderedCols[(size_t)nIndex] : NULL;
}


bool CCMSPOEntryLayout::CColumnItem::OrderColPtrsPred(const CCMSPOEntryLayout::CColumnItem* lhs, const CCMSPOEntryLayout::CColumnItem* rhs)
{
	return	lhs->m_bVisible != rhs->m_bVisible ? lhs->m_bVisible :
		lhs->m_bytRow != rhs->m_bytRow ? lhs->m_bytRow < rhs->m_bytRow :
		lhs->m_intColumnNo < rhs->m_intColumnNo;
}

// begbert 04-30-2009 : changed the behavior slightly, to not load over the values of non-user-editable fields.
//   This make this function specific to loading, rather than a generic 'set', so it was renamed accordingly.
bool CCMSPOEntryLayout::CColumnItems::LoadFrom(CDataSet& clsDataSet)
{
	//Pre-Condition: Load the default values so we match the incoming
	//field types with them. Also, lets the collection fall back to
	//defaults if a database entry does not exist.
	RestoreDefaults();

	if (clsDataSet.TableExists(CMSStrings::XMLTags::Table1))
	{
		CDataTable clsDataTable;
		clsDataSet.GetTable(CMSStrings::XMLTags::Table1, &clsDataTable);
		long lRowCount = clsDataTable.getRowCount();

		CCMSPOEntryLayout::CPOScreenDetailRecord clsRecord;
		for (long lRowIndex = 0; lRowIndex < lRowCount; lRowIndex++)
		{
			CDataRow clsDataRow;
			clsDataTable.GetRow(lRowIndex, &clsDataRow);
			clsRecord.InitFrom(clsDataRow);
			if (clsRecord.GetByteValue(clsRecord.DBF_BYT_TAB) == m_bytTab)
				LoadItem(clsRecord);
			clsDataRow.Dispose();
		}
	}
	AutoFixData();

	return true;
}

void CCMSPOEntryLayout::CColumnItems::AutoFixData()
{
	std::stable_sort(m_vOrderedCols.begin(), m_vOrderedCols.end(), CColumnItem::OrderColsPred);
	for (size_t i = 0, col = 0; i < m_vOrderedCols.size(); i++, col++)
	{
		if (i && m_vOrderedCols[i - 1].GetRow() != m_vOrderedCols[i].GetRow()) col = 0;
		if (m_vOrderedCols[i].GetColumnNo() != (int)col)
			m_vOrderedCols[i].SetColumnNo((int)col);
	}
}

void CCMSPOEntryLayout::CColumnItems::RestoreDefaults()
{
	for (UINT i = 0; i < m_vOrderedCols.size(); i++)
		m_vOrderedCols[i].RestoreDefaults();
}


//*************************************************************************************
//CCMSPOEntryLayout::CPURVendorItems
//*************************************************************************************
//Load the default values for all vendor items
CCMSPOEntryLayout::CPURVendorItems::CPURVendorItems()
	: CColumnItems(0)
{
	using namespace PUR_VendorItems;
	m_vOrderedCols.clear();

	AddColumn(PaidOutCheck, IDS_PAIDOUT_CHK, 0, 0, eCHECKED | eDISABLED, eUNCHECKED, eHIDDEN, eHIDDEN, eMANDATORY);
	AddColumn(_1099ApplicableCheck, IDS_1099APP_CHK, 0, 0, eCHECKED | eDISABLED, eUNCHECKED, eHIDDEN, eHIDDEN, eMANDATORY);
	AddColumn(DropShipCheck, IDS_DROPSHIP_CHK, 0, 0, eCHECKED | eDISABLED, eUNCHECKED, eHIDDEN, eHIDDEN, eMANDATORY);
	AddColumn(RecurringCheck, IDS_RECURR_CHK, 0, 0, eCHECKED | eDISABLED, eUNCHECKED, eHIDDEN, eHIDDEN, eMANDATORY);
	AddColumn(FinancialHistoryButton, IDS_FINANCIAL_BTN, 0, 0, eCHECKED | eDISABLED, eUNCHECKED, eHIDDEN, eHIDDEN, eMANDATORY);
	AddColumn(TransactionHistoryButton, IDS_HISTORY_BTN, 0, 0, eCHECKED | eDISABLED, eUNCHECKED, eHIDDEN, eHIDDEN, eMANDATORY);
	AddColumn(UseAPVendorAddressCheck, IDS_PO_USEAPVENDORADDRESSCHECKBOX, 0, 0, eCHECKED | eDISABLED, eUNCHECKED, eHIDDEN, eHIDDEN, eMANDATORY); // begbert 04-02-2009 1-29382 : Added this
	// CR 1-30996 BK 9/22/08 check and disable TabStop on Vendor Address line
	AddColumn(VendorAddress, IDS_AP_VENDOR_ADDRESS_LOOKUP, 0, 0, eCHECKED, eCHECKED | eDISABLED, eHIDDEN, eHIDDEN, eMANDATORY);
	AddColumn(ShippingAddress, IDS_SE_CUSTOMER_SHIPPING_ADDRESS, 0, 0, eCHECKED, eUNCHECKED, eHIDDEN, eHIDDEN, eMANDATORY);
	AddColumn(CustomerNumForTheVendor, IDS_CUSTOMER_VENDOR, 0, 0, eCHECKED, eUNCHECKED, eHIDDEN, eHIDDEN, eMANDATORY);
	if (CMS::GetSI()->CompanySettings.IsFund)
		AddColumn(OnHoldCheck, IDS_ONHOLD_CHK, 0, 0, eCHECKED, eUNCHECKED, eHIDDEN, eHIDDEN, eMANDATORY); // begbert 03-15-2011 1-35268 : Added this
}

void CCMSPOEntryLayout::CPURVendorItems::AutoFixData()
{
	// Do nothing - we don't want to accidentally rearrange the order of these.
}



//*************************************************************************************
//CCMSPOEntryLayout::CPURHeaderItems
//*************************************************************************************
//Load the default values for all header items
CCMSPOEntryLayout::CPURHeaderItems::CPURHeaderItems()
	: CColumnItems(1)
{
	using namespace PUR_HeaderItems;
	m_vOrderedCols.clear();

	AddColumn(PODate, IDS_PO_DATE, IDS_PO_FIELDDESC_PODATE, 10, eCHECKED, eUNCHECKED);
	AddColumn(ExpectedDate, IDS_PO_EXPECTED_DATE, IDS_PO_FIELDDESC_POEXPECTEDDATE, 10, eCHECKED, eUNCHECKED);
	AddColumn(FreightType, IDS_FREIGHT_TYPE, IDS_PO_FIELDDESC_FREIGHTTYPE, 7, eCHECKED, eUNCHECKED);
	AddColumn(FreightAmount, IDS_PO_FREIGHT_AMOUNT, IDS_PO_FIELDDESC_FREIGHTAMT, 10, eCHECKED, eUNCHECKED);
	AddColumn(FOB, IDS_PO_FOB, IDS_PO_FIELDDESC_FOB, 20, eCHECKED, eUNCHECKED);
	AddColumn(ShipVia, IDS_SHIP_VIA, IDS_PO_FIELDDESC_SHIPVIA, 6, eCHECKED, eUNCHECKED);
	AddColumn(ShipViaDescription, IDS_PO_SHIP_VIA_DESCRIPTION, IDS_PO_FIELDDESC_SHIPVIADESC, 20, eNA, eNA);
	AddColumn(Buyer, IDS_PO_BUYER, IDS_PO_FIELDDESC_BUYER, 6, eCHECKED, eUNCHECKED);
	AddColumn(BuyerName, IDS_PO_BUYER_NAME, IDS_PO_FIELDDESC_BUYERNAME, 20, eNA, eNA);
	AddColumn(Department, IDS_DEPARTMENT, IDS_PO_FIELDDESC_DEPARTMENT, 6, eCHECKED, eUNCHECKED);
	AddColumn(DepartmentDescription, IDS_PO_DEPARTMENT_DESCRIPTION, IDS_PO_FIELDDESC_DEPARTMENTDESC, 30, eNA, eNA);
	AddColumn(VendorContact, IDS_PO_VENDOR_CONTACT, IDS_PO_FIELDDESC_VENDORCONTACT, 20, eCHECKED, eUNCHECKED);
	AddColumn(Terms, IDS_TERMS2, IDS_PO_FIELDDESC_TERMS, 30, eCHECKED, eUNCHECKED);
	AddColumn(SalesTaxPct, IDS_SALES_TAX_PCT, IDS_PO_FIELDDESC_SALESTAXPCT, 8, eCHECKED, eUNCHECKED);
	AddColumn(DiscountType, IDS_PO_DISCOUNT_TYPE, IDS_PO_FIELDDESC_DISCOUNTTYPE, 8, eCHECKED, eUNCHECKED);
	AddColumn(DiscountAmt, IDS_DISCOUNT_AMT, IDS_PO_FIELDDESC_DISCOUNTAMT, 8, eCHECKED, eUNCHECKED);
	AddColumn(PaymentDiscount, IDS_AR_PAYMENT_DISCOUNT, IDS_PO_FIELDDESC_PAYMENTDISCOUNT, 8, eUNCHECKED | eDISABLED, eCHECKED | eDISABLED);	// begbert 04-30-2009 : per the PO Purchasing Entry spec, this field is not editable.
	AddColumn(OpenPO, IDS_PO_OPEN_PO, IDS_PO_FIELDDESC_OPENPO, 8, eCHECKED, eUNCHECKED);
	AddColumn(CutoffDate, IDS_CUTOFF_DATE, IDS_PO_FIELDDESC_CUTOFFDATE, 8, eCHECKED, eUNCHECKED);
	AddColumn(Remarks, IDS_REMARKS, IDS_PO_FIELDDESC_REMARKS, 15, eCHECKED, eUNCHECKED);
	AddUDFColumns(UDF, true, false, IDS_PO_FIELDDESC_HEADERUDFS, 8, eCHECKED, eUNCHECKED);
	AddUDFColumns(UDFDescription, true, true, IDS_PO_FIELDDESC_UDFDESCS, 30, eUNCHECKED | eDISABLED, eCHECKED | eDISABLED);
}

//*************************************************************************************
//CCMSPOEntryLayout::CPURDetailItems
//*************************************************************************************
//Load the default values.
CCMSPOEntryLayout::CPURDetailItems::CPURDetailItems()
	: CColumnItems(2)
{
	using namespace DetailItems;
	m_vOrderedCols.clear();

	AddColumn(StockNumber, IDS_LBL_STOCKNO, IDS_PO_FIELDDESC_STOCKNUMBER, 12, eCHECKED, eUNCHECKED, eVISIBLE, eVISIBLE, eMANDATORY | eFIXEDCOL, LineTypes::StockItem, StockNumber, RESSTRING(IDS_PO_STOCKNUM_GLACCT_COMMENT));
	AddColumn(GLAccount, IDS_GLACCOUNT, IDS_PO_FIELDDESC_GLACCOUNT, 12, eCHECKED, eUNCHECKED, eVISIBLE, eVISIBLE, eMANDATORY | eFIXEDCOL, LineTypes::DirectExpense, StockNumber);
	AddColumn(Comment, IDS_COMMENT, IDS_PO_FIELDDESC_COMMENTCODE, 12, eCHECKED, eUNCHECKED, eVISIBLE, eVISIBLE, eMANDATORY | eFIXEDCOL, LineTypes::Comment, StockNumber);	// begbert 09-04-2009 1-30891 : changed from IDS_COMMENTCODE.
	if (CMS::GetSI()->ModulesInstalled.MultiLocationInventory.IsUsable)
		AddColumn(MLILocation, IDS_PROMO_LOCATION, IDS_PO_FIELDDESC_LOCATION, 12, eCHECKED, eUNCHECKED);
	if (CMS::GetSI()->ModulesInstalled.CostCenter.IsUsable)
		AddColumn(CostCenter, IDS_COST_CENTER, IDS_PO_FIELDDESC_COSTCENTER, 12, eCHECKED, eUNCHECKED);
	AddColumn(QuantityOrdered, IDS_SA_PROMPT_QTY, IDS_PO_FIELDDESC_QUANTITY, 6, eCHECKED, eUNCHECKED, eVISIBLE, eVISIBLE, eMANDATORY);
	AddColumn(VendorStockNum, IDS_PO_VENDOR_STOCK_NUM, IDS_PO_FIELDDESC_VENDORSTOCKNUM, 12, eCHECKED, eUNCHECKED);
	AddColumn(Description, IDS_DESCRIPTION, IDS_PO_FIELDDESC_DESCRIPTION, 20, eCHECKED, eUNCHECKED);
	AddColumn(OrderedDate, IDS_ORDERED_DATE, IDS_PO_FIELDDESC_ORDEREDDATE, 12, eCHECKED, eUNCHECKED);
	AddColumn(ExpectedDate, IDS_PO_EXPECTED_DATE, IDS_PO_FIELDDESC_EXPECTEDDATE, 12, eCHECKED, eUNCHECKED);
	AddColumn(UnitCost, IDS_UNIT_COST, IDS_PO_FIELDDESC_UNITCOST, 12, eCHECKED, eUNCHECKED);
	AddColumn(ExtendedCost, IDS_LBL_EXTENDED, IDS_PO_FIELDDESC_EXTENDEDCOST, 12, eCHECKED, eUNCHECKED);
	AddColumn(IsLandItem, IDS_LAND_ITEM, IDS_PO_FIELDDESC_LANDITEM, 6, eCHECKED, eUNCHECKED);
	AddColumn(IsTaxable, IDS_CHK_TAXABLE, IDS_PO_FIELDDESC_TAXABLE, 6, eCHECKED, eUNCHECKED);
	AddColumn(IsDiscount, IDS_DISCOUNT, IDS_PO_FIELDDESC_DISCOUNT, 6, eCHECKED, eUNCHECKED);
	AddColumn(IsApplyFreight, IDS_APPLY_FREIGHT, IDS_PO_FIELDDESC_APPLYFREIGHT, 6, eCHECKED, eUNCHECKED);
	AddColumn(LastCost, IDS_LBL_LASTCOST, IDS_PO_FIELDDESC_LASTCOST, 12, eNA, eNA);
	AddColumn(References, IDS_REFERENCES, IDS_PO_FIELDDESC_REFERENCES, 20, eCHECKED, eUNCHECKED);
	AddColumn(SourceInvoice, IDS_SALES_INVOICE_NUMBER, IDS_PO_FIELDDESC_SALESINVOICENUMBER, 12, eUNCHECKED | eDISABLED, eCHECKED | eDISABLED);
	AddUDFColumns(UDF, false, false, IDS_PO_FIELDDESC_DETAILUDFS, 8, eCHECKED, eUNCHECKED);
	AddUDFColumns(UDFDescription, false, true, IDS_PO_FIELDDESC_UDFDESCS2, 30, eNA, eNA);

	if (CMS::GetSI()->CompanySettings.IsFund)
		AddColumn(Encumbrance, IDS_ENCUMBRANCE, IDS_PO_FIELDDESC_ENCUMBRANCE, 50, eCHECKED, eUNCHECKED);	// begbert 03-15-2011 1-35268 : Added this

	//	PBI 27164	2/5/2015	mvt - Added Job, Phase, and Cost Code columns for Job Cost
	if (CMS::GetSI()->ModulesInstalled.JobCost.IsUsable)
	{
		AddColumn(JobCode, IDS_JOB, IDS_PO_FIELDDESC_JOB, 10, eNA, eNA);
		AddColumn(PhaseCode, IDS_PHASE, IDS_PO_FIELDDESC_PHASE, 10, eNA, eNA);
		AddColumn(CostCode, IDS_COST_CODE, IDS_PO_FIELDDESC_COST_CODE, 10, eNA, eNA);
	}
}

//*************************************************************************************
//CCMSPOEntryLayout::CRCVHeaderItems
//*************************************************************************************
//Load the default values.
CCMSPOEntryLayout::CRCVHeaderItems::CRCVHeaderItems()
	: CColumnItems(1)
{
	using namespace RCV_HeaderItems;
	m_vOrderedCols.clear();

	AddColumn(PODate, IDS_PO_DATE, IDS_PO_FIELDDESC_PODATE2, 10, eNA, eNA);
	AddColumn(Receive_GLDate, IDS_PO_RECEIVE_GL_DATE, IDS_PO_FIELDDESC_RCVGLDATE, 10, eCHECKED, eUNCHECKED);
	AddColumn(VendorNumber, IDS_VENDOR_NUMBER, IDS_PO_FIELDDESC_VENDORNUMBER, 20, eNA, eNA);
	AddColumn(VendorName, IDS_VENDOR_NAME, IDS_PO_FIELDDESC_VENDORNAME, 20, eNA, eNA);
	AddColumn(TransactionDescription, IDS_LBL_TRANSACTIONDESC, IDS_PO_FIELDDESC_TRANSACTIONDESC, 30, eCHECKED, eUNCHECKED);
	AddColumn(Buyer, IDS_PO_BUYER, IDS_PO_FIELDDESC_BUYER2, 6, eNA, eNA);	// begbert 01-10-2009 1-28571 : fixed def colum width
	AddColumn(BuyerName, IDS_PO_BUYER_NAME, IDS_PO_FIELDDESC_BUYERNAME, 20, eNA, eNA);
	AddColumn(ReceivedBy, IDS_RECEIVED_BY, IDS_PO_FIELDDESC_RECEIVEDBY, 20, eNA, eNA);
	AddColumn(Department, IDS_DEPARTMENT, IDS_PO_FIELDDESC_DEPARTMENT2, 10, eNA, eNA);
	AddColumn(Complete, IDS_COMPLETE, IDS_PO_FIELDDESC_COMPLETE, 6, eCHECKED, eUNCHECKED);
	AddColumn(Remarks, IDS_REMARKS, IDS_PO_FIELDDESC_REMARKS, 15, eCHECKED, eUNCHECKED);
	AddUDFColumns(UDF, true, false, IDS_PO_FIELDDESC_HEADERUDFS2, 8, eCHECKED, eUNCHECKED);
	AddUDFColumns(UDFDescription, true, true, IDS_PO_FIELDDESC_UDFDESCS2, 30, eUNCHECKED | eDISABLED, eUNCHECKED | eDISABLED);
}


//*************************************************************************************
//CCMSPOEntryLayout::CRCVDetailItems
//*************************************************************************************
//Load the default values.
CCMSPOEntryLayout::CRCVDetailItems::CRCVDetailItems()
	: CColumnItems(2)
{
	using namespace DetailItems;
	m_vOrderedCols.clear();

	AddColumn(StockNumber, IDS_PO_STOCK_NUM__ACCT, IDS_PO_FIELDDESC_STOCKNUMACCT, 15, eNA, eNA, eVISIBLE, eVISIBLE, eMANDATORY | eFIXEDCOL);
	if (CMS::GetSI()->ModulesInstalled.MultiLocationInventory.IsUsable)	// begbert 03-27-2009 1-29048 : Corrected to match updated spec.
		AddColumn(MLILocation, IDS_LBL_LOCATION, IDS_PO_FIELDDESC_LOCATION2, 12, eNA, eNA, eVISIBLE, eVISIBLE, eFIXEDCOL);
	AddColumn(Description, IDS_AR_DESCRIPTION, IDS_PO_FIELDDESC_DESCRIPTION2, 20, eCHECKED, eUNCHECKED);	// begbert 03-18-2009 1-29268 : Enabled editing to comply with spec change.
	AddColumn(VendorStockNum, IDS_PO_VENDOR_STOCK_NUM, IDS_PO_FIELDDESC_VENDORSTOCKNUM, 12, eCHECKED, eUNCHECKED);
	if (CMS::GetSI()->ModulesInstalled.CostCenter.IsUsable)
		AddColumn(CostCenter, IDS_COST_CENTER, IDS_PO_FIELDDESC_COSTCENTER, 12, eUNCHECKED | eDISABLED, eCHECKED | eDISABLED);
	AddColumn(QuantityReceived, IDS_PO_QTY_RCVD_RETURNED, IDS_PO_FIELDDESC_QUANTITYRCVDRTRND, 11, eCHECKED, eUNCHECKED, eVISIBLE, eVISIBLE, eMANDATORY); // begbert 03-24-2009 1-29201 : Increased the default widths of these four quantity fields to be able to show their full widths, per changed spec.
	AddColumn(QuantityRemaining, IDS_QUANTITY_REMAINING, IDS_PO_FIELDDESC_QUANTITYREMAINING, 8, eCHECKED, eUNCHECKED);
	AddColumn(QuantityToInvoice, IDS_QUANTITY_TO_INVOICE, IDS_PO_FIELDDESC_QUANTITYTOINVOICE, 8, eCHECKED, eUNCHECKED, eVISIBLE, eVISIBLE, eMANDATORY);
	AddColumn(ReceivedAndNotInvoiced, IDS_RECEIVED_AND_NOT_INVOICED, IDS_PO_FIELDDESC_QTYRCVDNOTINVOICED, 11, eNA, eNA);
	AddColumn(ActualCost, IDS_ACTUAL_COST, IDS_PO_FIELDDESC_ACTUALCOST, 12, eCHECKED, eUNCHECKED);
	AddColumn(EstimatedActualCost, IDS_ESTIMATED_ACTUAL_COST, IDS_PO_FIELDDESC_ESTACTUALCOST, 12, eCHECKED, eUNCHECKED);
	AddColumn(ExtendedInvoice, IDS_EXTENDED_INVOICE, IDS_PO_FIELDDESC_EXTENDEDINVOICE, 12, eCHECKED, eUNCHECKED);
	AddColumn(InvUnitDiscount, IDS_PO_INV_UNIT_DISCOUNT, IDS_PO_FIELDDESC_INVUNITDISCOUNT, 12, eNA, eNA);
	AddColumn(InvExtDiscount, IDS_PO_INV_EXT_DISCOUNT, IDS_PO_FIELDDESC_INVEXTDISCOUNT, 12, eNA, eNA);
	AddColumn(InvUnitTax, IDS_PO_INV_UNIT_TAX, IDS_PO_FIELDDESC_INVUNITTAX, 12, eNA, eNA);
	AddColumn(InvExtTax, IDS_PO_INV_EXT_TAX, IDS_PO_FIELDDESC_INVEXTTAX, 12, eNA, eNA);
	AddColumn(InvUnitFreight, IDS_PO_INV_UNIT_FREIGHT, IDS_PO_FIELDDESC_INVUNITFREIGHT, 12, eNA, eNA);
	AddColumn(InvExtFreight, IDS_PO_INV_EXT_FREIGHT, IDS_PO_FIELDDESC_INVEXTFREIGHT, 12, eNA, eNA);
	AddColumn(ExtendedReceive, IDS_EXTENDED_RECEIVE, IDS_PO_FIELDDESC_EXTENDEDRECIEVE, 12, eCHECKED, eUNCHECKED);
	AddColumn(IsLandItem, IDS_LAND_ITEM, IDS_PO_FIELDDESC_LANDITEM2, 6, eCHECKED, eUNCHECKED);
	AddColumn(LandUnitTax, IDS_PO_LAND_UNIT_TAX, IDS_PO_FIELDDESC_LANDUNITTAX, 12, eNA, eNA);
	AddColumn(LandExtTax, IDS_PO_LAND_EXT_TAX, IDS_PO_FIELDDESC_LANDEXTTAX, 12, eNA, eNA);
	AddColumn(LandUnitDiscount, IDS_PO_LAND_UNIT_DISCOUNT, IDS_PO_FIELDDESC_LANDUNITDISCOUNT, 12, eNA, eNA);
	AddColumn(LandExtDiscount, IDS_PO_LAND_EXT_DISCOUNT, IDS_PO_FIELDDESC_LANDEXTDISCOUNT, 12, eNA, eNA);
	AddColumn(LandUnitFreight, IDS_PO_LAND_UNIT_FREIGHT, IDS_PO_FIELDDESC_LANDUNITFREIGHT, 12, eNA, eNA);
	AddColumn(LandExtFreight, IDS_PO_LAND_EXT_FREIGHT, IDS_PO_FIELDDESC_LANDEXTFREIGHT, 12, eNA, eNA);
	AddColumn(LandedUnitCost, IDS_PO_LANDED_UNIT_COST, IDS_PO_FIELDDESC_LANDEDUNITCOST, 12, eNA, eNA);
	AddColumn(LandedExtCost, IDS_PO_LANDED_EXT_COST, IDS_PO_FIELDDESC_LANDEDEXTCOST, 12, eNA, eNA);
	AddColumn(_1099Type, IDS_AP_1099_TYPE, IDS_PO_FIELDDESC_1099TYPE, 15, eCHECKED, eUNCHECKED);
	AddColumn(IsCompleted, IDS_COMPLETED, IDS_PO_FIELDDESC_COMPLETED, 6, eCHECKED, eUNCHECKED);
	AddColumn(SourceInvoice, IDS_SALES_INVOICE_NUMBER, IDS_PO_FIELDDESC_SALESINVOICENUMBER, 12, eUNCHECKED | eDISABLED, eCHECKED | eDISABLED);
	AddUDFColumns(UDF, false, false, IDS_PO_FIELDDESC_DETAILUDFS, 8, eCHECKED, eUNCHECKED);
	AddUDFColumns(UDFDescription, false, true, IDS_PO_FIELDDESC_UDFDESCS2, 30, eNA, eNA);

	//	PBI 27164	2/6/2015	mvt - Added Job, Phase, and Cost Code columns for Job Cost
	if (CMS::GetSI()->ModulesInstalled.JobCost.IsUsable)
	{
		AddColumn(JobCode, IDS_JOB, IDS_PO_FIELDDESC_JOB, 10, eNA, eNA);
		AddColumn(PhaseCode, IDS_PHASE, IDS_PO_FIELDDESC_PHASE, 10, eNA, eNA);
		AddColumn(CostCode, IDS_COST_CODE, IDS_PO_FIELDDESC_COST_CODE, 10, eNA, eNA);
	}
}


//*************************************************************************************
//CCMSPOEntryLayout::CRCVLandingItems
//*************************************************************************************
//Load the default values.
CCMSPOEntryLayout::CRCVLandingItems::CRCVLandingItems()
	: CColumnItems(3)
{
	using namespace RCV_LandingItems;
	m_vOrderedCols.clear();

	AddColumn(LandedPurchaseDiscount, IDS_PO_LANDED_PURCHASE_DISCOUNT, IDS_PO_FIELDDESC_LANDEDPURDISCOUNT, 15, eCHECKED, eCHECKED);
	AddColumn(LandedTax, IDS_PO_LANDED_TAX, IDS_PO_FIELDDESC_LANDEDTAX, 15, eCHECKED, eCHECKED);
	AddColumn(LandedFreight, IDS_PO_LANDED_FREIGHT, IDS_PO_FIELDDESC_LANDEDFREIGHT, 15, eCHECKED, eCHECKED);
}



//*************************************************************************************
//CCMSPOEntryLayout::CRCVInvoicingItems
//*************************************************************************************
//Load the default values.
CCMSPOEntryLayout::CRCVInvoicingItems::CRCVInvoicingItems()
	: CColumnItems(4)
{
	using namespace RCV_InvoicingItems;
	m_vOrderedCols.clear();

	AddColumn(InvoiceNum, IDS_AP_INVNO, IDS_PO_FIELDDESC_INVOICENUM, 20, eCHECKED, eUNCHECKED, eVISIBLE, eVISIBLE, eMANDATORY);
	AddColumn(InvoiceDate, IDS_SA_INVOICEDATE, IDS_PO_FIELDDESC_INVOICEDATE, 12, eCHECKED, eUNCHECKED);
	AddColumn(DueDate, IDS_AP_LBL_DUE_DATE, IDS_PO_FIELDDESC_DUEDATE, 12, eCHECKED, eUNCHECKED);
	AddColumn(DiscDate, IDS_AP_DISCDATE, IDS_PO_FIELDDESC_DISCDATE, 12, eCHECKED, eUNCHECKED);
	AddColumn(EarlyPaymentDisc, IDS_PO_EARLY_PAYMENT_DISC, IDS_PO_FIELDDESC_EARLYPAYMENTDISC, 15, eCHECKED, eUNCHECKED);
	AddColumn(PurchaseDisc, IDS_PO_PURCHASE_DISC, IDS_PO_FIELDDESC_PURCHASEDISC, 15, eCHECKED, eUNCHECKED);
	AddColumn(Tax, IDS_CTRL_TAX, IDS_PO_FIELDDESC_TAX, 15, eCHECKED, eUNCHECKED);
	AddColumn(Freight, IDS_SA_FREIGHT, IDS_PO_FIELDDESC_FREIGHT, 15, eCHECKED, eUNCHECKED);
	AddColumn(FreightVendor, IDS_FREIGHT_VENDOR, IDS_PO_FIELDDESC_FREIGHTVENDOR, 20, eCHECKED, eUNCHECKED, eVISIBLE, eVISIBLE, eMANDATORY);
	AddColumn(FreightInvoiceNum, IDS_PO_FREIGHT_INVOICE_NUM, IDS_PO_FIELDDESC_FREIGHTINVOICENUM, 12, eCHECKED, eUNCHECKED, eVISIBLE, eVISIBLE, eMANDATORY);
	AddColumn(FreightExpenseAcct, IDS_PO_FREIGHT_EXPENSE_ACCT, IDS_PO_FIELDDESC_FREIGHTEXPENSEACCT, 20, eCHECKED, eUNCHECKED);
	AddColumn(AcctDescription, IDS_PO_ACCT_DESCRIPTION, IDS_PO_FIELDDESC_ACCTDESC, 20, eNA, eNA);
	if (CMS::GetSI()->ModulesInstalled.CostCenter.IsUsable)
		AddColumn(FreightCostCenter, IDS_FREIGHT_COST_CENTER, IDS_PO_FIELDDESC_FREIGHTCOSTCENTER, 12, eCHECKED, eUNCHECKED);
}


/*
//*************************************************************************************
//CCMSPOEntryLayout::CVendor
//*************************************************************************************
//Load the default values.
void CCMSPOEntryLayout::CVendor::LoadDefaults()
{
	m_bHideCustomerInfo		= false ;
	m_bFinancialHistory		= true ;
	m_bTransactionHistory	= true ;
	m_bNotes				= true ;
	m_bBillingAddress		= true ;
	m_bShippingAddress		= true ;
	m_bHideHeader			= false;
}

//= operator
CCMSPOEntryLayout::CVendor& CCMSPOEntryLayout::CVendor::operator = (const CCMSPOEntryLayout::CVendor& rhs)
{
	m_bHideCustomerInfo		= rhs.m_bHideCustomerInfo ;
	m_bFinancialHistory		= rhs.m_bFinancialHistory ;
	m_bTransactionHistory	= rhs.m_bTransactionHistory ;
	m_bNotes				= rhs.m_bNotes ;
	m_bBillingAddress		= rhs.m_bBillingAddress ;
	m_bShippingAddress		= rhs.m_bShippingAddress ;
	m_bHideHeader			= rhs.m_bHideHeader;

	return *this ;
}

//overloaded = operator for CSAScreenHeaderRecord
CCMSPOEntryLayout::CVendor& CCMSPOEntryLayout::CVendor::operator = (CCMSPOEntryLayout::CPOScreenHeaderRecord& rhs)
{
	m_bHideCustomerInfo		= rhs.GetBoolValue(CCMSPOEntryLayout::CPOScreenHeaderRecord::DBF_BOL_CUSTPROMPT) ;
	m_bFinancialHistory		= rhs.GetBoolValue(CCMSPOEntryLayout::CPOScreenHeaderRecord::DBF_BOL_FINHISTINFO) ;
	m_bTransactionHistory	= rhs.GetBoolValue(CCMSPOEntryLayout::CPOScreenHeaderRecord::DBF_BOL_TRANHISTINFO) ;
	m_bNotes				= rhs.GetBoolValue(CCMSPOEntryLayout::CPOScreenHeaderRecord::DBF_BOL_CUSTNOTES) ;
	m_bBillingAddress		= rhs.GetBoolValue(CCMSPOEntryLayout::CPOScreenHeaderRecord::DBF_BOL_CUSTBILLING) ;
	m_bShippingAddress		= rhs.GetBoolValue(CCMSPOEntryLayout::CPOScreenHeaderRecord::DBF_BOL_CUSTSHIPPING) ;
	m_bHideHeader			= rhs.GetBoolValue(CCMSPOEntryLayout::CPOScreenHeaderRecord::DBF_BOL_HIDEHEADER) ;

	return *this ;
}

//Load from dataset
bool CCMSPOEntryLayout::CVendor::LoadFrom(CDataSet& clsDataSet)
{
	CCMSPOEntryLayout::CPOScreenHeaderRecord clsRecord ;
	if(clsRecord.InitFrom(clsDataSet))
	{
		operator = (clsRecord) ;
		return true ;
	}
	return false ;
}
*/



//*************************************************************************************
//*************************************************************************************
//CCMSPOEntryLayout class.
//*************************************************************************************
//*************************************************************************************

CCMSPOEntryLayout::CCMSPOEntryLayout(BYTE bytModule /*=SCREEN_MODULE_PUR*/)
{
	si = CGBLForm::GetSystemInformationPointer();

	//member variables used while passing information for preview.
	m_szScreenCode = _T("");
	m_bytModule = bytModule;

	m_bHideVendorPane = m_bHideHeaderPane = false;
	m_bHideLandingPane = m_bHideInvoicingPane = false;
}

CCMSPOEntryLayout::~CCMSPOEntryLayout(void)
{
}

//Split to table, pass to all member objects to parse and fill.
// This function automatically autofixes the data, as the column numbers are not user-editable anyway.
bool CCMSPOEntryLayout::LoadFrom(CDataSet& clsDataSet)
{
	CPOScreenHeaderRecord clsRecord;
	clsRecord.InitFrom(clsDataSet);
	m_bytModule = clsRecord.GetByteValue(CPOScreenHeaderRecord::DBF_BYT_MODULE);

	if (clsDataSet.TableExists(CMSStrings::XMLTags::Table))
	{
		CDataTable clsDataTable;
		clsDataSet.GetTable(CMSStrings::XMLTags::Table, &clsDataTable);

		m_bytModule = clsDataTable.XMLByte(0, _T("bytEntryScreen"));
		m_bHideVendorPane = clsDataTable.XMLBOOL(0, _T("bolMinimizeVendor"));
		m_bHideHeaderPane = clsDataTable.XMLBOOL(0, _T("bolMinimizeHeader"));
		m_bHideLandingPane = clsDataTable.XMLBOOL(0, _T("bolMinimizeLanding"));
		m_bHideInvoicingPane = clsDataTable.XMLBOOL(0, _T("bolMinimizeInvoice"));
	}

	m_clsPURVendorItems.LoadFrom(clsDataSet);
	m_clsPURHeaderItems.LoadFrom(clsDataSet);
	m_clsPURDetailItems.LoadFrom(clsDataSet);
	m_clsRCVHeaderItems.LoadFrom(clsDataSet);
	m_clsRCVDetailItems.LoadFrom(clsDataSet);
	m_clsRCVLandingItems.LoadFrom(clsDataSet);
	m_clsRCVInvoicingItems.LoadFrom(clsDataSet);

	return true;
}

//Load values from database.
//Test code to load:
/*
	CCMSPOEntryLayout clsLayout ;
	clsLayout.m_clsTenderItems.LoadDefaults() ;//so that we have actual map entries.
	clsLayout.LoadFromDB(_T("01")) ;
*/
bool CCMSPOEntryLayout::LoadFromDB(const CString& szScreenCode, BYTE bytModule, const CString& szRegisterCode)
{
	CXML clsXML;
	CXMLParams clsXMLCommand;
	CXMLParams clsXMLData;
	CXMLParams::MakeXMLCommand(&clsXMLCommand, _T("GET"), _T("PO"), _T("PO_SCREENHEADER"));
	CXMLParams::AppendXMLConnection(&clsXMLData, CXMLParams::CN_COMPANY);
	CXMLParams::AppendCompanyInfo(&clsXMLData);
	clsXMLData.MakeParam(_T("DIRECTION"), _T("CURRENT"));
	CXMLParams tableXMLParam;
	tableXMLParam.SetRoot(_T("TABLE"));
	CXMLParams recordXMLParam(true);
	recordXMLParam.SetRoot(_T("RECORD"));
	recordXMLParam.MakeParam(_T("strScreenCode"), szScreenCode);
	//	recordXMLParam.MakeParam(_T("bytModule"), bytModule) ;
	//	if(szRegisterCode != "")
	//		recordXMLParam.MakeParam(_T("strRegisterCode"), szRegisterCode) ;
	tableXMLParam.AppendXMLParam(&recordXMLParam);
	clsXMLData.AppendXMLParam(&tableXMLParam);

	clsXML.m_szXMLCommand = clsXMLCommand.GetXML();
	clsXML.m_szXMLData = clsXMLData.GetXML();

	CDataSet clsDataSet;
	clsDataSet.LoadFromXML(clsXML.SendXML());
	CString text = clsDataSet.GetXML();
	//CGBLForm::DumpStringToFile(_T("c:\\xml\\myxml.xml"), clsDataSet.GetXML()) ;
	LoadFrom(clsDataSet);
	return true;
}
