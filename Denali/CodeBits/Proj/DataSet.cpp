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
#include "..\Common\LangIDs.h"

#define ERRORCAPTION "Data Retrieval Error"
#define ERRORSTYLE		MB_OK | MB_ICONERROR

 /***************************************************************
 Function       CDataSet (Constructor)
 Type					 void
 Purpose        Constructs a CDataSet Object
 Parameters     None
 Returns        void
 Author         Peter Ringering
 Date           11/01/2002
 ****************************************************************/
CDataSet::CDataSet()
{
	CoInitialize(NULL);
	Dispose();
	bOutofMemory = false;
	//m_pDOMDocument = MakeXMLDocument();
}

/***************************************************************
Function       CDataSet (Destructor)
Type					 void
Purpose        Destroys m_pDOMDocument then finally the CDataSet Object
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
CDataSet::~CDataSet()
{
	Dispose();
}

/***************************************************************
Function       Dispose
Type           void
Purpose        Releases and sets to NULL the XML Document.
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
void	CDataSet::Dispose()
{
	if (m_pDOMDocument != NULL)
	{
		m_pDOMDocument.Release();
		m_pDOMDocument = NULL;
	}
	m_nTableCount = 0;
}

/***************************************************************
Function       IsEmpty
Type           bool
Purpose        Checks to see if the XML Document is NULL.
Parameters     None
Returns        bool
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
bool	CDataSet::IsEmpty()
{
	CString szXML = _T("");
	//if (m_nTableCount == 0) return true;
	bOutofMemory = false;
	if (m_pDOMDocument == NULL) return true;

	//AR 10/13/2025 PBI 65666 - Out of memory is handled for large data in lookups.
	try
	{
		szXML = m_pDOMDocument->xml.GetBSTR();
	}
	catch (_com_error e)
	{
		HRESULT hr = e.Error();
		if (hr == E_OUTOFMEMORY)
			CGBLForm::CMSMessageBox(AfxGetMainWnd(), IDS_INSUFFICIENT_MEMORY_MSG, IDS_INSUFFICIENT_MEMORY_TITLE, MB_ICONERROR | MB_OK);
		return true;
	}
	catch (CException* pe)
	{
		CGBLForm::CMSMessageBox(AfxGetMainWnd(), IDS_INSUFFICIENT_MEMORY_MSG, IDS_INSUFFICIENT_MEMORY_TITLE, MB_ICONERROR | MB_OK);
		pe->Delete();
		bOutofMemory = true;
		Dispose();
		return true;
	}
	return (szXML == "");
}

// ********************************************************
// * Procedure:    HasData
// *
// * Description:  Checks to see if this object has at least one table and that table
// * has at least one row and that row has at least one column.
// *
// * Parameters:   None
// *
// * Returns:      bool - true = Has Data, false = Has no data.
// *
// * Date Created: 8/20/2003 by Peter Ringering
// ********************************************************
bool	CDataSet::HasData()
{
	if (IsEmpty()) return false;
	if (this->getTableCount() == 0) return false;
	CDataTable clsDT;
	GetTable(0, &clsDT, true);	//sb - added flag to tell method to ignore schema
	if (clsDT.getRowCount() == 0) return false;
	if (clsDT.getColumnCount() == 0) return false;
	return true;
}

/***************************************************************
Function       LoadFromXML (Overload #1)
Type           void
Purpose        Loads up the XML Document with XML from the
parameter.  If the XML is invalid, then a
Denali pop-up error dialog will show and the
XML document will remain NULL.
Parameters     szXML – The BSTR variable containing the XML text.
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
void	CDataSet::LoadFromXML(const _bstr_t& bstXML)
{
	if (!IsEmpty()) Dispose();

	CCMSXMLDomDoc::GetPolicy().CreateObject(m_pDOMDocument, CCMSXMLDomDoc::GetProgID());
	try
	{
		m_pDOMDocument->loadXML(bstXML);
	}
	catch (CException* pe)
	{
		HandleError(0, pe);
		return;
	}
	catch (...)
	{
		HandleError(ERR_GB_DATASET_INVALIDXML);
		return;
	}
	m_nTableCount = GetTableCount(this);		
	//CheckDSError(this);		//01/10/2003 - Peter Ringering - Not needed anymore.
}

/***************************************************************
Function       LoadFromXML (Overload #2)
Type           void
Purpose        Loads up the XML Document with XML from the
parameter.  If the XML is invalid, then a
Denali pop-up error dialog will show and the XML
document will remain NULL.
Parameters     szXML – The CString variable containing the XML text.
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
void	CDataSet::LoadFromXML(const CString& szXML)
{
	_bstr_t bstXML = szXML;
	LoadFromXML(bstXML);
}

CString CDataSet::GetXML(void)
{
	return LPCTSTR(m_pDOMDocument->Getxml());
}

/***************************************************************
Function       TableExists
Type           bool
Purpose        Checks to see if a table with the table name of szTableName
exists in the CDataSet object.  Returns true if the table exists.
Parameters     szTableName – The name of the table in the CDataSet object.
Returns        bool
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
bool	CDataSet::TableExists(const CString& szTableName)
{
	if (IsEmpty()) return false;
	CDataTable DT;
	LoadDataTable(this, &DT, szTableName, false);
	if (DT.IsEmpty()) return false;
	if (DT.m_lRowCount == 0) return false;
	DT.Dispose();
	return true;
}

/***************************************************************
Function       GetTable (Overload #1)
Type           void
Purpose        Loads up the CDataTable pointer with the szTableName table.
If the table doesn’t exist, then a Denali pop-up error dialog
will show and the CDataTable’s node list variable will remain NULL.
Parameters     szTableName – The name of the table in the CDataSet object.
pDT – A pointer to an empty CDataTable object to be filled.
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
void	CDataSet::GetTable(const CString& szTableName, CDataTable* pDT)
{
	ASSERT(pDT);
	LoadDataTable(this, pDT, szTableName, true);
	pDT->m_pDataSet = this;		//Peter Ringering - 02/08/2005 - 1-16669
	return;
}

/***************************************************************
Function       GetTable (Overload #2)
Type           void
Purpose        Loads up the CDataTable pointer with the table at the given index.
If the table doesn’t exist, then a Denali pop-up error dialog will
show and the CDataTable’s node list variable will remain NULL.
Parameters     nIndex – The zero-based index of the table in the CDataSet object.
pDT – A pointer to an empty CDataTable object to be filled.
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
void	CDataSet::GetTable(int nIndex, CDataTable* pDT, bool bIgnoreSchema/* = false*/)
{
	ASSERT(pDT);

	if (bIgnoreSchema && SchemaExists(m_pDOMDocument))
		nIndex++;

	//Do we have any tables at all?
	if (nIndex > m_nTableCount - 1)
	{
		CString szMsg;
		CXMLParams XMLParams;
		XMLParams.MakeParam("index", nIndex);
		XMLParams.MakeParam("maxindex", m_nTableCount);
		if (m_nTableCount == 0)
			HandleError(ERR_GB_DATASET_INVALIDTB0TB, NULL, &XMLParams);
		else
			HandleError(ERR_GB_DATASET_INVALIDTBINDEX, NULL, &XMLParams);
		XMLParams.Dispose();
		return;
	}
	//Double check for the existence of any tables.
	CCMSXMLDomElement::ComponentTypePtr       pXMLElement;
	HRESULT hr = m_pDOMDocument->get_documentElement(&pXMLElement);
	if (hr != S_OK) return;
	VARIANT_BOOL bHasKids = pXMLElement->hasChildNodes();
	if (!bHasKids)
	{
		//		DestroyElement(pXMLElement);
		return;
	}

	//Triple check for the existence of any tables.
	CCMSXMLDomNodeList::ComponentTypePtr pXMLNodeList;
	hr = pXMLElement->get_childNodes(&pXMLNodeList);
	if (hr != S_OK) return;
	//	DestroyElement(pXMLElement);

	long lLength;
	hr = pXMLNodeList->get_length(&lLength);
	if (hr != S_OK || lLength == 0)
	{
		//		DestroyNodeList(pXMLNodeList);
		return;
	}

	//Search for the table.
	int nTableIndex = 0;
	for (long lCounter = 0; lCounter < lLength; lCounter++)
	{
		CCMSXMLDomNode::ComponentTypePtr pXMLNode;
		hr = pXMLNodeList->get_item(lCounter, &pXMLNode);
		if (hr != S_OK)
		{
			pDT->Dispose();
			return;
		}
		BSTR bstCurNodeName;
		pXMLNode->get_nodeName(&bstCurNodeName);
		if (hr != S_OK)
		{
			pDT->Dispose();
			return;
		}
		CString szTableName = bstCurNodeName;
		if (szTableName == _T("xs:schema"))
		{
			nTableIndex++;
			continue;
		}
		LoadDataTable(this, pDT, szTableName, true);
		pDT->m_pDataSet = this;		//Peter Ringering - 02/08/2005 - 1-16669
		if (nTableIndex == nIndex)
		{
			pDT->m_szTableName = szTableName;
			break;
		}
		if (pDT->m_lRowCount > 0) lCounter += (pDT->m_lRowCount - 1);
		nTableIndex++;
		pDT->Dispose();
	}
	return;
}

CDataTable CDataSet::Table(int nIndex, bool bIgnoreSchema/* = false*/)
{
	CDataTable DT;
	this->GetTable(nIndex, &DT, bIgnoreSchema);

	return DT;
}

CDataTable CDataSet::Table(const CString& szTableName)
{
	CDataTable DT;
	this->GetTable(szTableName, &DT);

	return DT;
}

// ********************************************************
// * Procedure:    AddTable
// *
// * Description:  Adds a table to the CDataSet.
// *
// * Parameters:   szTableName - Table name to add.
// *               pDT - New Table.
// *
// * Date Created: 03/12/2003 by Peter Ringering
// ********************************************************
void	CDataSet::AddTable(const CString& szTableName, CDataTable* pDT)
{
	ASSERT(pDT);
	if (m_nTableCount == 0)
	{
		Dispose();
		m_pDOMDocument = MakeXMLDocument("ROOT");
	}
	pDT->Dispose();
	if (TableExists(szTableName)) return;
	AddNodeVal(m_pDOMDocument, szTableName);
	GetTable(szTableName, pDT);
	SetNewTableFlag(pDT);
	//NewRowCount(pDT);
	m_nTableCount++;
}

// ********************************************************
// * Procedure:    RemoveTable
// *
// * Description:  Removes all rows and columns from the passed in table and disposes the table.
// *
// * Parameters:   szTableName - Table name to remove.
// *
// * Date Created: 03/12/2003 by Peter Ringering
// ********************************************************
void	CDataSet::RemoveTable(const CString& szTableName)
{
	CDataTable clsDT;
	GetTable(szTableName, &clsDT);
	RemoveTable(&clsDT);
}

// ********************************************************
// * Procedure:    RemoveTable
// *
// * Description:  Removes all rows and columns from the passed in table and disposes the table.
// *
// * Parameters:   nTableIndex - Table index to remove.
// *
// * Date Created: 03/12/2003 by Peter Ringering
// ********************************************************
void	CDataSet::RemoveTable(int nTableIndex)
{
	CDataTable clsDT;
	GetTable(nTableIndex, &clsDT);
	RemoveTable(&clsDT);
}

// ********************************************************
// * Procedure:    RemoveTable
// *
// * Description:  Removes all rows and columns from the passed in table and disposes the table.
// *
// * Parameters:   pDT - Data Table to remove.
// *
// * Date Created: 03/12/2003 by Peter Ringering
// ********************************************************
void	CDataSet::RemoveTable(CDataTable* pDT)
{
	ASSERT(pDT);
	if (pDT->IsEmpty()) return;
	for (long lRow = 0; lRow < pDT->getRowCount(); lRow++)
	{
		pDT->RemoveRow(lRow);
	}
	pDT->Dispose();
	m_nTableCount--;
}

// ********************************************************
// * Procedure:    SchemaFieldExists
// *
// * Description:  Checks the passed-in XML document to see if schema
// *			   exists.  If it does, then it checks to see if the
// *			   passed-in field name exists for the passed-in
// *			   table name in the schema.
// *
// * Parameters:   pDOMDocument - Document to check
// *			   szTable - Table to look for
// *			   szField - Field to look for.
// *
// * Date Created: 12/05/2004 by Peter Ringering
// ********************************************************
bool CDataSet::SchemaFieldExists(CCMSXMLDomDoc::ComponentTypePtr& pDOMDocument, const CString& szTable, const CString& szField)
{
	if (!SchemaExists(pDOMDocument))
		return false;

	CXMLDocument docSchema(pDOMDocument);
	CXMLElement eleRoot = docSchema.GetDocumentElement();
	CXMLNodeList nlChildren = eleRoot.GetChildNodes();
	CXMLElement eleFirst = nlChildren.GetItem(0);

	CString szXPath = _T("");
	szXPath.Format(_T("xs:element/xs:complexType/xs:choice/xs:element[@name = \"%s\"]/xs:complexType/xs:sequence/xs:element[@name = \"%s\"]"), szTable, szField);
	CXMLNodeList nlSchemaNodes = eleFirst.SelectNodes(szXPath);
	ULONG ulCount = nlSchemaNodes.GetCount();
	return (ulCount > 0);
}

bool CDataSet::SchemaExists(CCMSXMLDomDoc::ComponentTypePtr& pDOMDocument)
{
	if (pDOMDocument == NULL)
		return false;

	CXMLDocument docSchema(pDOMDocument);
	if (!docSchema.RootExists())
		return false;

	CXMLElement eleRoot = docSchema.GetDocumentElement();
	CXMLNodeList nlChildren = eleRoot.GetChildNodes();
	if (nlChildren.GetCount() <= 0)
		return false;

	CXMLElement eleFirst = nlChildren.GetItem(0);
	if (eleFirst.GetName() != "xs:schema")
		return false;

	return true;
}

/***************************************************************
Function       CDataTable (Constructor)
Type					 void
Purpose        Constructs a CDataTable Object
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
CDataTable::CDataTable()
	: m_pDataSet(NULL)
{
	CoInitialize(NULL);
	Dispose();
	m_bNewTable = false;
	m_szXMLSchema = "";
}

/***************************************************************
Function       CDataTable (Constructor)
Type					 void
Purpose        Constructs a CDataTable Object
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
CDataTable::CDataTable(const CDataTable& inRHS)
	: m_pNodeList(inRHS.m_pNodeList)
	, m_pDocElement(inRHS.m_pDocElement)
	, m_szTableName(inRHS.m_szTableName)
	, m_nColumnCount(inRHS.m_nColumnCount)
	, m_lRowCount(inRHS.m_lRowCount)
	, m_bNewRow(inRHS.m_bNewRow)
	, m_bNewTable(inRHS.m_bNewTable)
	, m_szXMLSchema(inRHS.m_szXMLSchema)
	, m_pDataSet(inRHS.m_pDataSet)
{
	CoInitialize(NULL);
}

/***************************************************************
Function       CDataTable (Constructor)
Type					 void
Purpose        Constructs a CDataTable Object
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
CDataTable::CDataTable(CDataSet& objDataSet)
	: m_pDataSet(&objDataSet)
{
	CoInitialize(NULL);

	//Peter Ringering - 02/08/2005 - 1-16669
	m_pDataSet->m_pDOMDocument.AddRef();
}

CDataTable::~CDataTable()
{
	Dispose();
	//	DestroyElement(m_pDocElement);
		//Peter Ringering - 02/08/2005 - 1-16669
		//if (m_pDataSet) m_pDataSet->m_pDOMDocument.Release();
}

//------------------------------------------------------------------------------
const CDataTable& CDataTable::operator= (const CDataTable& inRHS)
{
	if (this != &inRHS)
	{
		m_pNodeList = inRHS.m_pNodeList;
		m_pDocElement = inRHS.m_pDocElement;
		m_szTableName = inRHS.m_szTableName;
		m_nColumnCount = inRHS.m_nColumnCount;
		m_lRowCount = inRHS.m_lRowCount;
		m_bNewRow = inRHS.m_bNewRow;
		m_bNewTable = inRHS.m_bNewTable;
		m_szXMLSchema = inRHS.m_szXMLSchema;

		//Peter Ringering - 02/08/2005 - 1-16669
		//if (m_pDataSet)
		//	m_pDataSet->m_pDOMDocument.Release();

		m_pDataSet = inRHS.m_pDataSet;
		//if (m_pDataSet)
		//	m_pDataSet->m_pDOMDocument.AddRef();
	}

	return *this;
}

/***************************************************************
Function       Dispose
Type           void
Purpose        Releases and sets to NULL the XML Node List.
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
void	CDataTable::Dispose()
{
	if (m_pNodeList != NULL)
	{
		m_pNodeList.Release();
		m_pNodeList = NULL;
	}
	m_szTableName = "";
	m_lRowCount = 0;
	m_nColumnCount = 0;
}

/***************************************************************
Function       IsEmpty
Type           bool
Purpose        Checks to see if the XML Node List is NULL.
Parameters     None
Returns        bool
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
bool	CDataTable::IsEmpty()
{
	return (m_pNodeList == NULL);
}

/***************************************************************
Function       GetRow
Type           void
Purpose        Loads up the CDataRow pointer with the row at the given index.
If the row doesn’t exist, then a Denali pop-up error dialog will
show and the CDataRow’s node variable will remain NULL.
Parameters     lIndex – The zero-based index of the row in the CDataTable object.
pDR – A pointer to an empty CDataRow object to be filled.
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
void	CDataTable::GetRow(long lRowIndex, CDataRow* pDR)
{
	ASSERT(pDR);
	if (IsEmpty()) return;
	LoadDataRow(this, pDR, lRowIndex);
	if (!pDR->m_pDataTable)
	{
		pDR->m_pDataTable = this;
		//Peter Ringering - 02/08/2005 - 1-16669
		//pDR->m_pDataTable->m_pNodeList.AddRef();
		//pDR->m_pDataTable->m_pDocElement.AddRef();
	}
	pDR->m_szDataTableName = m_szTableName;

	return;
}

CDataRow CDataTable::Row(long lRowIndex)
{
	CDataRow DR;
	this->GetRow(lRowIndex, &DR);

	return DR;
}
/***************************************************************
Function		Select (Overload #1)
Type			void
Purpose			Loads up the CDataRow pointer with the first row
matching a given criteria.  If the XPath syntax
is wrong, then a Denali pop-up error dialog will
show and the CDataRow’s node variable will remain
NULL.
Parameters		szCondition – The condition formatted in XPath
syntax.  You do not need to worry about putting in
the table name though.  Just use something like:
“Field = ‘Value’”.  See the Microsoft XML 4.0
Parser SDK documentation on how to use XPath.
pDR – A pointer to an empty CDataRow object to
be filled.
Returns			void
Author			Peter Ringering
Date			11/15/2002
****************************************************************/
void	CDataTable::Select(const CString& szCondition, CDataRow* pDR)
{
	ASSERT(pDR);

	m_bNewRow = false;

	CString szXPath = MakeXPath(szCondition, m_szTableName);
	if (m_lRowCount == 0) return;
	HRESULT hr = m_pDocElement->raw_selectSingleNode(_bstr_t(LPCTSTR(szXPath)), &pDR->m_pNode);
	if (FAILED(hr))
	{
		pDR->Dispose();
		CXMLParams XMLParams;
		XMLParams.MakeParam("query", szCondition);
		XMLParams.MakeParam("table", m_szTableName);
		HandleError(ERR_GB_DATASET_INVALIDQUERY, NULL, &XMLParams);
		return;
	}
	if (pDR->IsEmpty())
	{
		pDR->Dispose();
		return;
	}
	//Load up the data row with the results.
	LoadDataRow(pDR);
	pDR->m_szDataTableName = m_szTableName; //PTR.04.28.2005.1-16627
	long lCounter;
	//Figure out the row index for the given row within the original data table.
	for (lCounter = 0; lCounter < m_lRowCount; lCounter++)
	{
		CCMSXMLDomNodeList::ComponentTypePtr pXMLNodeList = m_pDocElement->GetchildNodes();
		CCMSXMLDomNode::ComponentTypePtr pXMLNodeCur = pXMLNodeList->Getitem(lCounter);
		if (pDR->m_pNode == pXMLNodeCur)
		{
			//Found the row index.
			pDR->m_lRowIndex = lCounter;
			return;
		}
		//		DestroyNode(pXMLNodeCur);
		//		DestroyNodeList(pXMLNodeList);
	}
	//We didn't find anything.
	pDR->Dispose();
	return;
}

/***************************************************************
Function		Select (Overload #1)
Type			void
Purpose			Loads up the CDataTable pointer with all the rows
matching a given criteria.  If the XPath syntax
is wrong, then a Denali pop-up error dialog will
show and the CDataTable’s node list variable will
remain NULL.
Parameters		szCondition – The condition formatted in XPath
syntax.  You do not need to worry about putting
in the table name though.  Just use something
like: “Field = ‘Value’”.  See the Microsoft XML
4.0 Parser SDK documentation on how to use XPath.
pDT – A pointer to an empty CDataTable object to
be filled.
Returns			void
Author			Peter Ringering
Date			11/15/2002
****************************************************************/
void	CDataTable::Select(const CString& szCondition, CDataTable* pDT)
{
	ASSERT(pDT);
	CString szXPath = MakeXPath(szCondition, m_szTableName);
	//CDataSet DS;
	//Make the XML Document that we will select from.
	//MakeSelectDS(&DS, this);
	HRESULT hr = m_pDocElement->raw_selectNodes(_bstr_t(LPCTSTR(szXPath)), &pDT->m_pNodeList);
	if (hr != S_OK)
	{
		CXMLParams XMLParams;
		XMLParams.MakeParam("query", szCondition);
		XMLParams.MakeParam("table", m_szTableName);
		HandleError(ERR_GB_DATASET_INVALIDQUERY, NULL, &XMLParams);
		return;
	}
	//Load the data table with the results.
	LoadDataTable(pDT, "", false);
	pDT->m_szTableName = m_szTableName; //PTR.04.28.2005.1-16627
}

/***************************************************************
Function       GetColumnName
Type           CString
Purpose        Returns the column name for the inputted column index.
If the column index is wrong, then a Denali pop-up error dialog will show.
Parameters     nColumnIndex – The index of the column you want to get the name of.
Returns        CString - Name of the column.
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
CString CDataTable::GetColumnName(int nColumnIndex)
{
	if (IsEmpty()) return "";
	//Make sure we have at least 1 row.
	if (m_lRowCount == 0 || m_nColumnCount == 0)
	{
		CXMLParams XMLParams;
		XMLParams.MakeParam("columnindex", nColumnIndex);
		XMLParams.MakeParam("table", m_szTableName);
		HandleError(ERR_GB_DATASET_INVALIDCOL0REC, NULL, &XMLParams);
		return "";
	}
	CString szColumnName = "";
	CDataRow DR;
	//Get the first row
	GetRow(0, &DR);
	if (!DR.IsEmpty())
	{
		CDataCell DC;
		//Get the cell at the specified index.
		DR.GetDataCell(nColumnIndex, &DC);
		//Get the column name for the cell.
		szColumnName = DC.m_szColumnName;
		DC.Dispose();
	}
	DR.Dispose();

	return szColumnName;
}

/***************************************************************
Function       XMLStr (Overload #1)
Type           CString
Purpose        Returns a CString variable containing the data located at the
given row index and column index.  If the row index or the column
index is invalid and bValidate = true, then a Denali pop-up error
dialog will show and the return value will be empty.
Parameters     lRowIndex – The row index
nColumnIndex – The column index
bValidate (Optional--default=true) - If the column index is invalid do we want to show an error message?
Returns        CString
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
CString CDataTable::XMLStr(long lRowIndex, int nColumnIndex, bool bValidate, bool bTrim/* = true*/)
{
	CDataRow DR;
	GetRow(lRowIndex, &DR);
	if (DR.IsEmpty()) return "";
	CString szValue = DR.XMLStr(nColumnIndex, bValidate, bTrim);
	DR.Dispose();

	return szValue;
}

/***************************************************************
Function       XMLStr (Overload #2)
Type           CString
Purpose        Returns a CString variable containing the data located at the
given row index and column name.  If the row index or the column
name is invalid and bValidate = true, then a Denali pop-up error
dialog will show and the return value will be empty.
Parameters     lRowIndex – The row index
szColumnName – The name of the column
bValidate (Optional--default=true) - If the column name is invalid do we want to show an error message?
Returns        CString
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
CString CDataTable::XMLStr(long lRowIndex, const CString& szColumnName, bool bValidate, bool bTrim/* = true*/)
{
	CDataRow DR;
	GetRow(lRowIndex, &DR);
	if (DR.IsEmpty()) return "";
	CString szValue = DR.XMLStr(szColumnName, bValidate, bTrim);
	DR.Dispose();

	return szValue;
}

/***************************************************************
Function       XMLLng (Overload #1)
Type           long
Purpose        Returns a long variable containing the data located at the
given row index and column index.  If the row index or the column
index is invalid and bValidate = true, then a Denali pop-up error
dialog will show and the return value will be empty.
Parameters     lRowIndex – The row index
nColumnIndex – The column index
bValidate (Optional--default=true) - If the column index is invalid do we want to show an error message?
Returns        long
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
int		CDataTable::XMLInt(long lRowIndex, int nColumnIndex, bool bValidate)
{
	CDataRow DR;
	GetRow(lRowIndex, &DR);
	if (DR.IsEmpty()) return 0;
	int nValue = DR.XMLInt(nColumnIndex, bValidate);
	DR.Dispose();

	return nValue;
}

/***************************************************************
Function       XMLLng (Overload #2)
Type           long
Purpose        Returns a long variable containing the data located at the
given row index and column name.  If the row index or the column
name is invalid and bValidate = true, then a Denali pop-up error
dialog will show and the return value will be empty.
Parameters     lRowIndex – The row index
szColumnName – The name of the column
bValidate (Optional--default=true) - If the column name is invalid do we want to show an error message?
Returns        long
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
int		CDataTable::XMLInt(long lRowIndex, const CString& szColumnName, bool bValidate)
{
	CDataRow DR;
	GetRow(lRowIndex, &DR);
	if (DR.IsEmpty()) return 0;
	int nValue = DR.XMLInt(szColumnName, bValidate);
	DR.Dispose();

	return nValue;
}

/***************************************************************
Function       XMLLng (Overload #1)
Type           long
Purpose        Returns a long variable containing the data located at the
given row index and column index.  If the row index or the column
index is invalid and bValidate = true, then a Denali pop-up error
dialog will show and the return value will be empty.
Parameters     lRowIndex – The row index
nColumnIndex – The column index
bValidate (Optional--default=true) - If the column index is invalid do we want to show an error message?
Returns        long
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
long	CDataTable::XMLLng(long lRowIndex, int nColumnIndex, bool bValidate)
{
	CDataRow DR;
	GetRow(lRowIndex, &DR);
	if (DR.IsEmpty()) return 0;
	long lValue = DR.XMLLng(nColumnIndex, bValidate);
	DR.Dispose();

	return lValue;
}

/***************************************************************
Function       XMLLng (Overload #2)
Type           long
Purpose        Returns a long variable containing the data located at the
given row index and column name.  If the row index or the column
name is invalid and bValidate = true, then a Denali pop-up error
dialog will show and the return value will be empty.
Parameters     lRowIndex – The row index
szColumnName – The name of the column
bValidate (Optional--default=true) - If the column name is invalid do we want to show an error message?
Returns        long
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
long	CDataTable::XMLLng(long lRowIndex, const CString& szColumnName, bool bValidate)
{
	CDataRow DR;
	GetRow(lRowIndex, &DR);
	if (DR.IsEmpty()) return 0;
	long lValue = DR.XMLLng(szColumnName, bValidate);
	DR.Dispose();

	return lValue;
}

/***************************************************************
Function       XMLDbl (Overload #1)
Type           double
Purpose        Returns a double variable containing the data located at the
given row index and column index.  If the row index or the column
index is invalid and bValidate = true, then a Denali pop-up error
dialog will show and the return value will be empty.
Parameters     lRowIndex – The row index
nColumnIndex – The column index
bValidate (Optional--default=true) - If the column index is invalid do we want to show an error message?
Returns        double
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
double	CDataTable::XMLDbl(long lRowIndex, int nColumnIndex, bool bValidate)
{
	CDataRow DR;
	GetRow(lRowIndex, &DR);
	if (DR.IsEmpty()) return 0;
	double dValue = DR.XMLDbl(nColumnIndex, bValidate);
	DR.Dispose();

	return dValue;
}

/***************************************************************
Function       XMLDbl (Overload #2)
Type           double
Purpose        Returns a double variable containing the data located at the
given row index and column name.  If the row index or the column
name is invalid and bValidate = true, then a Denali pop-up error
dialog will show and the return value will be empty.
Parameters     lRowIndex – The row index
szColumnName – The name of the column
bValidate (Optional--default=true) - If the column name is invalid do we want to show an error message?
Returns        double
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
double	CDataTable::XMLDbl(long lRowIndex, const CString& szColumnName, bool bValidate)
{
	CDataRow DR;
	GetRow(lRowIndex, &DR);
	if (DR.IsEmpty()) return 0;
	double dValue = DR.XMLDbl(szColumnName, bValidate);
	DR.Dispose();

	return dValue;
}

/***************************************************************
Function       XMLBOOL (Overload #1)
Type           bool
Purpose        Returns a bool variable containing the data located at the
given row index and column index.  If the row index or the column
index is invalid and bValidate = true, then a Denali pop-up error
dialog will show and the return value will be empty.
Parameters     lRowIndex – The row index
nColumnIndex – The column index
bValidate (Optional--default=true) - If the column index is invalid do we want to show an error message?
Returns        bool
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
bool	CDataTable::XMLBOOL(long lRowIndex, int nColumnIndex, bool bValidate)
{
	CDataRow DR;
	GetRow(lRowIndex, &DR);
	if (DR.IsEmpty()) return FALSE;
	bool bValue = DR.XMLBOOL(nColumnIndex, bValidate);
	DR.Dispose();

	return bValue;
}

/***************************************************************
Function       XMLBOOL (Overload #2)
Type           bool
Purpose        Returns a bool variable containing the data located at the
given row index and column name.  If the row index or the column
name is invalid and bValidate = true, then a Denali pop-up error
dialog will show and the return value will be empty.
Parameters     lRowIndex – The row index
szColumnName – The name of the column
bValidate (Optional--default=true) - If the column name is invalid do we want to show an error message?
Returns        bool
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
bool	CDataTable::XMLBOOL(long lRowIndex, const CString& szColumnName, bool bValidate)
{
	CDataRow DR;
	GetRow(lRowIndex, &DR);
	if (DR.IsEmpty()) return false;
	bool bValue = DR.XMLBOOL(szColumnName, bValidate);
	DR.Dispose();

	return bValue;
}

/***************************************************************
Function       XMLByte (Overload #1)
Type           byte
Purpose        Returns a CString variable containing the data located at the
given row index and column index.  If the row index or the column
index is invalid and bValidate = true, then a Denali pop-up error
dialog will show and the return value will be empty.
Parameters     lRowIndex – The row index
nColumnIndex – The column index
bValidate (Optional--default=true) - If the column index is invalid do we want to show an error message?
Returns        byte
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
byte	CDataTable::XMLByte(long lRowIndex, int nColumnIndex, bool bValidate)
{
	CDataRow DR;
	GetRow(lRowIndex, &DR);
	byte bytValue = 0;
	if (DR.IsEmpty()) return bytValue;
	bytValue = DR.XMLByte(nColumnIndex, bValidate);
	DR.Dispose();

	return bytValue;
}

/***************************************************************
Function       XMLByte (Overload #2)
Type           byte
Purpose        Returns a byte variable containing the data located at the
given row index and column name.  If the row index or the column
name is invalid and bValidate = true, then a Denali pop-up error
dialog will show and the return value will be empty.
Parameters     lRowIndex – The row index
szColumnName – The name of the column
bValidate (Optional--default=true) - If the column name is invalid do we want to show an error message?
Returns        byte
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
byte	CDataTable::XMLByte(long lRowIndex, const CString& szColumnName, bool bValidate)
{
	CDataRow DR;
	GetRow(lRowIndex, &DR);
	byte bytValue = 0;
	if (DR.IsEmpty()) return bytValue;
	bytValue = DR.XMLByte(szColumnName, bValidate);
	DR.Dispose();

	return bytValue;
}

/***************************************************************
Function       XMLDate (Overload #1)
Type           COleDateTime
Purpose        Returns a COleDateTime variable containing the data located at the
given row index and column index.  If the row index or the column
index is invalid and bValidate = true, then a Denali pop-up error
dialog will show and the return value will be empty.
Parameters     lRowIndex – The row index
nColumnIndex – The column index
bValidate (Optional--default=true) - If the column index is invalid do we want to show an error message?
Returns        COleDateTime
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
COleDateTime CDataTable::XMLDate(long lRowIndex, int nColumnIndex, bool bValidate)
{
	CDataRow DR;
	GetRow(lRowIndex, &DR);
	COleDateTime dteValue;
	if (DR.IsEmpty()) return dteValue;
	dteValue = DR.XMLDate(nColumnIndex, bValidate);
	DR.Dispose();

	return dteValue;
}

/***************************************************************
Function       XMLDate (Overload #2)
Type           COleDateTime
Purpose        Returns a COleDateTime variable containing the data located at the
given row index and column name.  If the row index or the column
name is invalid and bValidate = true, then a Denali pop-up error
dialog will show and the return value will be empty.
Parameters     lRowIndex – The row index
szColumnName – The name of the column
bValidate (Optional--default=true) - If the column name is invalid do we want to show an error message?
Returns        COleDateTime
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
COleDateTime CDataTable::XMLDate(long lRowIndex, const CString& szColumnName, bool bValidate)
{
	CDataRow DR;
	GetRow(lRowIndex, &DR);
	COleDateTime dteValue;
	if (DR.IsEmpty()) return dteValue;
	dteValue = DR.XMLDate(szColumnName, bValidate);
	DR.Dispose();

	return dteValue;
}

/***************************************************************
Function       ColumnExists
Type           bool
Purpose        Returns true if the column name exists in the CDataTable.
Returns false if it doesn’t exist.  No validation occurs here.
Parameters     szColumnName – The name of the column to check.
Returns        bool
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
bool	CDataTable::ColumnExists(const CString& szColumnName)
{
	if (m_pDataSet)
		if (CDataSet::SchemaFieldExists(m_pDataSet->m_pDOMDocument, m_szTableName, szColumnName))
			return true;

	if (m_lRowCount == 0 || m_nColumnCount == 0 || IsEmpty()) return false;
	CDataRow DR;
	GetRow(0, &DR);
	if (DR.IsEmpty()) return false;
	bool bReturn = DR.ColumnExists(szColumnName);
	DR.Dispose();

	return bReturn;
}

// ********************************************************
// * Procedure:    AddColumn
// *
// * Description:  Adds a column to the CDataTable.
// *
// * Parameters:   szColumnName - New column name to add.
// *               nIndex - Index to position the column at.  -1 means at the end.
// *
// * Returns:      int - Index of the column added.  -1 if the column already exists.
// *
// * Date Created: 03/12/2003 by Peter Ringering
// ********************************************************
int		CDataTable::AddColumn(const CString& szColumnName, int nIndex)
{
	int nNewIndex = m_nColumnCount;
	if (ColumnExists(szColumnName)) return -1;
	for (long lRow = 0; lRow < m_lRowCount; lRow++)
	{
		CDataRow clsDR;
		GetRow(lRow, &clsDR);
		nNewIndex = ::AddColumn(&clsDR, szColumnName, nIndex);
		clsDR.Dispose();
	}

	return nNewIndex;
}

// ********************************************************
// * Procedure:    RemoveColumn
// *
// * Description:  Removes a column from the CDataTable.
// *
// * Parameters:   szColumnName - column name to remove.
// *
// * Date Created: 03/12/2003 by Peter Ringering
// ********************************************************
void	CDataTable::RemoveColumn(const CString& szColumnName)
{
	if (!ColumnExists(szColumnName)) return;
	for (long lRow = 0; lRow < m_lRowCount; lRow++)
	{
		CDataRow clsDR;
		GetRow(lRow, &clsDR);
		::RemoveColumn(&clsDR, szColumnName);
		m_nColumnCount = clsDR.getColumnCount();
		clsDR.Dispose();
	}

	return;
}

// ********************************************************
// * Procedure:    RemoveColumn
// *
// * Description:  Removes a column from the CDataTable.
// *
// * Parameters:   nColumnIndex - column index to remove.
// *
// * Date Created: 03/12/2003 by Peter Ringering
// ********************************************************
void	CDataTable::RemoveColumn(int nColumnIndex)
{
	if (m_nColumnCount == 0) return;
	for (long lRow = 0; lRow < m_lRowCount; lRow++)
	{
		CDataRow clsDR;
		GetRow(lRow, &clsDR);
		::RemoveColumn(&clsDR, nColumnIndex);
		m_nColumnCount = clsDR.getColumnCount();
		clsDR.Dispose();
	}
	return;
}

/***************************************************************
Function       NewRow
Type           CDataRow
Purpose        Creates a new row in the table and returns it.  Note
the row doesn't exist in the table until it is added with AddRow.
Parameters     None
Returns        CDataRow
Author         Peter Ringering
Date           01/10/2003
****************************************************************/
void	CDataTable::NewRow(CDataRow* pDR)
{
	if (m_bNewRow == true)
	{
		HandleError(ERR_GB_DATATABLE_NEWROWADD, NULL, NULL);
		return;
	}
	if (m_bNewTable)
	{
		//pDR->m_pNode = m_pDocElement->selectSingleNode(m_szTableName.AllocSysString());
		pDR->m_pNode = m_pDocElement->selectSingleNode(_bstr_t(m_szTableName));
	}
	else
	{
		CDataRow clsRow0DR;
		GetRow(0, &clsRow0DR);
		CCMSXMLDomDoc::ComponentTypePtr pDocument = clsRow0DR.m_pNode->ownerDocument;
		//pDR->m_pNode = pDocument->createElement(m_szTableName.AllocSysString());
		pDR->m_pNode = pDocument->createElement(_bstr_t(m_szTableName));
		//clsNewRow.m_pNode = clsRow0DR.m_pNode->cloneNode(true);
		for (int nColumn = 0; nColumn < m_nColumnCount; nColumn++)
		{
			CDataCell clsDC;
			clsRow0DR.GetDataCell(nColumn, &clsDC);
			AddNodeVal(pDocument, clsDC.getColumnName(), "", pDR->m_pNode, false);
			//clsNewRow.m_pNode->appendChild(clsDC.m_pNode->cloneNode(false));
		}
	}
	LoadDataRow(pDR);
	pDR->m_szDataTableName = m_szTableName;

	UpdateNewFlag(pDR, true);
	m_bNewRow = true;
	return;
}

/***************************************************************
Function       AddRow
Type           void
Purpose        Adds the passed in row to the table.  Note,
the row needs to have been created with new row prior to adding it.
Parameters     pDR - Row to Add
lRowIndex - Index to set the row to.
Returns        void
Author         Peter Ringering
Date           01/10/2003
****************************************************************/
void	CDataTable::AddRow(CDataRow* pDR, long lRowIndex)
{
	if (m_bNewRow != true)
	{
		HandleError(ERR_GB_DATATABLE_ZERONEWROW, NULL, NULL);
		return;
	}
	if (IsRowNew(pDR) != true)
	{
		HandleError(ERR_GB_DATATABLE_NOTNEWROW, NULL, NULL);
		return;
	}
	if (m_bNewTable)
	{
		m_bNewTable = false;
		m_lRowCount = 1;
	}
	else if (lRowIndex == -1 || lRowIndex >= m_lRowCount)
	{
		m_pDocElement->appendChild(pDR->m_pNode);
	}
	else
	{
		CCMSXMLDomNode::ComponentType* pRefNode;
		m_pNodeList->get_item(lRowIndex, &pRefNode);
		m_pDocElement->insertBefore(pDR->m_pNode, pRefNode);
		pRefNode->Release();
	}
	UpdateNewFlag(pDR, false);
	pDR->Dispose();
	m_bNewRow = false;
	CString szTableName = m_szTableName;
	Dispose();
	m_pNodeList = m_pDocElement->selectNodes(_bstr_t(szTableName));
	LoadDataTable(this, szTableName, false);
	if (lRowIndex == -1) lRowIndex = this->m_lRowCount - 1;
	GetRow(lRowIndex, pDR);
}

/***************************************************************
Function       RemoveRow
Type           void
Purpose        Removes the row at lRowIndex
Parameters     lRowIndex - Row index to remove.
Returns        void
Author         Peter Ringering
Date           01/10/2003
****************************************************************/
void	CDataTable::RemoveRow(long lRowIndex)
{
	CDataRow clsDR;
	GetRow(lRowIndex, &clsDR);
	if (clsDR.IsEmpty()) return;
	m_pDocElement->removeChild(clsDR.m_pNode);
	CString szTableName = m_szTableName;
	Dispose();
	m_pNodeList = m_pDocElement->selectNodes(_bstr_t(szTableName));
	LoadDataTable(this, szTableName, false);
}

// ********************************************************
// * Procedure:    GetXML
// *
// * Description:  Retrieves the XML from this CDataTable in CString Format.
// *
// * Parameters:   szNewTableName (IN, Optional) : new table name
// *               bIncludeSchema (IN, Optional) : include schema info?
// *
// * Returns:      CString - XML to return
// *
// * Date Created: 03/18/2003 by Peter Ringering
// ********************************************************
CString CDataTable::GetXML(const CString& szNewTableName, bool bIncludeSchema)
{
	CXMLParams clsXML;

	// EK - 7/18/3 - Adding schema information on request
	if (bIncludeSchema)
	{
		CXMLParams clsSchemaXML(true);
		clsSchemaXML.SetXML(this->GetSchema(szNewTableName));
		clsXML.AppendXMLParam(&clsSchemaXML);
		clsSchemaXML.Dispose();
	}

	for (long lRow = 0; lRow < m_lRowCount; lRow++)
	{
		CXMLParams clsRowXML(true);
		CDataRow clsDR;
		GetRow(lRow, &clsDR);
		clsRowXML.SetXML(clsDR.GetXML());
		if (szNewTableName != _T(""))
		{
			CXMLParams clsNewRowXML(false);
			// Create function to create a new CXMLParams with the parameters from an existing param object
			clsRowXML.CopyTo(&clsNewRowXML, szNewTableName);
			clsXML.AppendXMLParam(&clsNewRowXML);
			clsNewRowXML.Dispose();
		}
		else
			clsXML.AppendXMLParam(&clsRowXML);

		clsRowXML.Dispose();
		clsDR.Dispose();
	}
	// EK - 01/08/04 - Return a CString in stead of a BSTR to prevent returning an invalid pointer
	//return clsXML.GetXMLBSTR();
	const CString szXML(clsXML.GetXML());
	clsXML.Dispose();

	return szXML;
}

// ********************************************************
// * Procedure:    GetSchema
// *
// * Description:  Retrieves the Schema XML for this table.
// *               Gets the schema information from the
// *               dataset the table was created from.
// *               Returns empty string if there is no information
// *
// * Parameters:   szNewTableName (IN, Optional) : new table name
// *
// * Returns:      BSTR - XML to return
// *
// * Date Created: 07/18/2003 by Erick Korsten
// ********************************************************
CString CDataTable::GetSchema(const CString& szNewTableName)
{
	// EK - 7/18/3 - Load schema
	CCMSXMLDomNode::ComponentTypePtr pParentNode = NULL, pChildNode = NULL, pTableAttrNode = NULL, pAttrNode = NULL;
	CCMSXMLDomNodeList::ComponentTypePtr pNodeList = NULL;
	CCMSXMLDomAttribute::ComponentTypePtr pIXMLDOMAttribute = NULL;
	CCMSXMLDomNodeMap::ComponentType* p_attributeMap;
	long lLength = 0;
	HRESULT hr;
	_variant_t varValue;

	// EK - 12/30/03 - 1-9423 - Load the schema information into a new XML Document
	//pParentNode = m_pDocElement->selectSingleNode("//xs:schema/xs:element/xs:complexType/xs:choice");
	CCMSXMLDomDoc::ComponentTypePtr pNewDOMDocument;
	pNewDOMDocument = MakeXMLDocument("Schema");
	pNewDOMDocument->loadXML(m_pDocElement->selectSingleNode("//xs:schema")->xml);
	pParentNode = pNewDOMDocument->selectSingleNode("//xs:schema/xs:element/xs:complexType/xs:choice");

	if (NULL != pParentNode)    // Node found
	{
		// get a list of all the table definitions
		hr = pParentNode->get_childNodes(&pNodeList);
		if (S_OK == hr)
			pNodeList->get_length(&lLength);

		// Go through all the child nodes, remove the child if it is not the requested table
		long l = 0;
		while (l < lLength)
		{
			hr = pNodeList->get_item(l, &pChildNode);
			if (NULL == pChildNode) break;      // we reached the end of the nodelist

			hr = pChildNode->get_attributes(&p_attributeMap);
			if (S_OK == hr)
			{
				// Determine table name
				pAttrNode = p_attributeMap->getNamedItem(_bstr_t("name"));
				pAttrNode->get_nodeValue(&varValue);
				CString szParam = (_bstr_t(varValue)).GetBSTR();

				// If table name is not equal to requested table: remove node
				if (szParam != this->getTableName())
				{
					pParentNode->removeChild(pChildNode);
				}   // it is the table: do not remove
				else
				{  // requested table
					pTableAttrNode = pAttrNode;
					l++;    // skip this table and go on to the next
				}
			}
		}

		// Change table name attribute
		if (NULL != pTableAttrNode)
		{									// The table was found: adjust the table name to the new name
			if ("" != szNewTableName)
				pTableAttrNode->put_nodeValue(_variant_t(szNewTableName));
			//			DestroyNode(pTableAttrNode);    // Release node pointer
		}
	}
	//	DestroyNode(pParentNode);

		// Get new schema
		// EK - 12/30/03 - 1-9423 - Load the schema information into a new XML Document
		//pParentNode = m_pDocElement->selectSingleNode("//xs:schema");
	pParentNode = pNewDOMDocument->selectSingleNode("//xs:schema");
	if (NULL == pParentNode)
		return "";
	else
	{
		CString szXML = LPCTSTR(pParentNode->Getxml());
		pNewDOMDocument = NULL;
		return szXML;
	}
}

//**************************************************************************
//* Procedure:     SelectDistinct
//*
//* Description:   Parses through all distinct rows for the passed in column
//*                name(s).  It sends each distinct row to clsCallBack.
//*                ***WARNING***  This only works properly if the table is
//*                ordered per the passed in column name(s) order.
//*
//* Parameters:    szColumnName - Column name to search distinct items on.
//*                  This parameter is useful when searching just one column.
//*                clsCallBack - Interface that is called back for every
//*                  distinct row.
//*                lpData - Data used by client to coordinate between this
//*                  method and the IDTDistinctCallBack::ProcessDistinctRow
//*                  call back.
//**************************************************************************
void CDataTable::SelectDistinct(const CString& szColumnName, IDTDistinctCallBack& clsCallBack, LPARAM lpData /*=0*/)
{
	CStringArray aColumnNames;
	aColumnNames.Add(szColumnName);
	SelectDistinct(aColumnNames, clsCallBack, lpData);
}

//**************************************************************************
//* Procedure:     SelectDistinct
//*
//* Description:   Parses through all distinct rows for the passed in column
//*                name(s).  It sends each distinct row to clsCallBack.
//*                ***WARNING***  This only works properly if the table is
//*                ordered per the passed in column name(s) order.
//*
//* Parameters:    aColumnNames - An array of column names to get distinct
//*                  items for.
//*                clsCallBack - Interface that is called back for every
//*                  distinct row.
//*                lpData - Data used by client to coordinate between this
//*                  method and the IDTDistinctCallBack::ProcessDistinctRow
//*                  call back.
//**************************************************************************
void CDataTable::SelectDistinct(CStringArray& aColumnNames, IDTDistinctCallBack& clsCallBack, LPARAM lpData /*=0*/)
{
	if (!getRowCount())
		return;

	CString szName = getTableName();
	for (long lIndex = 0; lIndex < getRowCount(); lIndex++)
	{
		//First make an XPath statement to get all rows matching the distinct criteria.
		CDataRow drRow;
		GetRow(lIndex, &drRow);
		CString szXPSearch(_T(""));
		for (int nColumn = 0; nColumn < aColumnNames.GetSize(); nColumn++)
		{
			//Go through each column name and use its name and value to create the query.
			CString szColumn = aColumnNames.GetAt(nColumn);
			//Make sure that the passed-in column name actually exists in this table.
			ASSERT(ColumnExists(szColumn));
			CString szValue = drRow.XMLStr(szColumn);
			CString szSearch(_T(""));
			szSearch.Format(_T("%s%s = %s"), (nColumn ? _T(" and ") : _T("")), szColumn, CXMLDocument::FormatXPathValue(szValue));
			szXPSearch += szSearch;
		}
		//Get all child rows matching this criteria.
		CDataTable dtSelect;
		Select(szXPSearch, &dtSelect);
		if (dtSelect.getRowCount())
		{
			//Call back to process the found row.
			CDataRow drFound;
			dtSelect.GetRow(0, &drFound);
			clsCallBack.ProcessDistinctRow(drFound, lpData);
			//Increment lIndex by Number of found elements - 1;
			lIndex += (dtSelect.getRowCount() - 1);
		}
	}
}

/***************************************************************
Function       CDataRow (Constructor)
Type					 void
Purpose        Constructs a CDataRow Object
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
CDataRow::CDataRow()
	: m_lRowIndex(0)
	, m_nColumnCount(0)
	, m_pNode(NULL)
	, m_szDataTableName(_T(""))
	, m_bNewRow(false)
	, m_pDataTable(NULL)
{
	CoInitialize(NULL);
	Dispose();
}

/***************************************************************
Function       CDataRow (Constructor)
Type					 void
Purpose        Constructs a CDataRow Object
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
CDataRow::CDataRow(const CDataRow& inRHS)
	: m_lRowIndex(inRHS.m_lRowIndex)
	, m_nColumnCount(inRHS.m_nColumnCount)
	, m_pNode(inRHS.m_pNode)
	, m_szDataTableName(inRHS.m_szDataTableName)
	, m_bNewRow(inRHS.m_bNewRow)
	, m_pDataTable(inRHS.m_pDataTable)
{
	CoInitialize(NULL);
	//Dispose();		//sb - this line throws away everything that was copied
}

/***************************************************************
Function       CDataRow (Constructor)
Type					 void
Purpose        Constructs a CDataRow Object
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
CDataRow::CDataRow(CDataTable& objDataTable)
	: m_pDataTable(&objDataTable)
{
	CoInitialize(NULL);
	//Dispose();

	//Peter Ringering - 02/08/2005 - 1-16669
	//m_pDataTable->m_pNodeList.AddRef();
	//m_pDataTable->m_pDocElement.AddRef();
}

//------------------------------------------------------------------------------
const CDataRow& CDataRow::operator= (const CDataRow& inRHS)
{
	if (this != &inRHS)
	{
		m_lRowIndex = inRHS.m_lRowIndex;
		m_nColumnCount = inRHS.m_nColumnCount;
		m_pNode = inRHS.m_pNode;
		m_szDataTableName = inRHS.m_szDataTableName;
		m_bNewRow = inRHS.m_bNewRow;

		//Peter Ringering - 02/08/2005 - 1-16669
		//if (m_pDataTable)
		//{
		//	m_pDataTable->m_pNodeList.Release();
		//	m_pDataTable->m_pDocElement.Release();
		//}

		m_pDataTable = inRHS.m_pDataTable;

		//Peter Ringering - 02/08/2005 - 1-16669
		//if (m_pDataTable)
		//{
		//	m_pDataTable->m_pNodeList.AddRef();
		//	m_pDataTable->m_pDocElement.AddRef();
		//}
	}

	return *this;
}

/***************************************************************
Function       CDataRow (Destructor)
Type					 void
Purpose        Destroys m_pNode then finally the CDataRow Object
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
CDataRow::~CDataRow()
{
	Dispose();

	//Peter Ringering - 02/08/2005 - 1-16669
	//if (m_pDataTable)
	//{
	//	m_pDataTable->m_pNodeList.Release();
	//	m_pDataTable->m_pDocElement.Release();
	//}
}

/***************************************************************
Function       Dispose
Type           void
Purpose        Releases and sets to NULL the XML Node.
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
void	CDataRow::Dispose()
{
	if (!IsEmpty())
	{
		m_pNode.Release();
		m_pNode = NULL;
	}
	m_nColumnCount = 0;
	m_lRowIndex = 0;
	m_szDataTableName = "";
}

/***************************************************************
Function       IsEmpty
Type           bool
Purpose        Checks to see if the XML Node is NULL.
Parameters     None
Returns        bool
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
bool	CDataRow::IsEmpty()
{
	return (m_pNode == NULL);
}

/***************************************************************
Function       GetDataCell (Overload #1)
Type           void
Purpose        Loads up the CDataCell pointer with the cell at the given column index.
If the column doesn’t exist and bValidate = true, then a Denali pop-up
error dialog will show and the CDataCell’s node variable will remain NULL.
Parameters     nColumnIndex – The zero-based index of the column in the CDataRow object.
pDC – A pointer to an empty CDataCell object to be filled.
bValidate (Optional--default=true) - Do we want an error to show if the column doesn't exist?
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
void	CDataRow::GetDataCell(int nColumnIndex, CDataCell* pDC, bool bValidate, bool bTrim/* = true*/)
{
	ASSERT(pDC);
	pDC->m_bTrimValue = bTrim;
	if (!ValidateRow(this, true, nColumnIndex, "")) return;
	LoadDataCell(this, pDC, nColumnIndex, bValidate);
}

/***************************************************************
Function       GetDataCell (Overload #2)
Type           void
Purpose        Loads up the CDataCell pointer with the cell for the given
column name.  If the column doesn’t exist and bValidate = true,
then a Denali pop-up error dialog will show and the CDataCell’s node
variable will remain NULL.
Parameters     szColumnName – The name of the column.
pDC – A pointer to an empty CDataCell object to be filled.
bValidate (Optional--default=true) - Do we want an error to show if the column doesn't exist?
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
void	CDataRow::GetDataCell(const CString& szColumnName, CDataCell* pDC, bool bValidate, bool bTrim/* = true*/)
{
	ASSERT(pDC);
	pDC->m_bTrimValue = bTrim;
	if (!ValidateRow(this, false, 0, szColumnName)) return;
	LoadDataCell(this, pDC, szColumnName, bValidate);
}

CDataCell CDataRow::Cell(const CString& szColumnName, bool bValidate)
{
	CDataCell DCell;
	this->GetDataCell(szColumnName, &DCell, bValidate);

	return DCell;
}

CDataCell CDataRow::Cell(int nColumnIndex, bool bValidate)
{
	CDataCell DCell;
	this->GetDataCell(nColumnIndex, &DCell, bValidate);

	return DCell;
}

/***************************************************************
Function       ColumnExists
Type           bool
Purpose        Returns true if the column name exists in the CDataRow.
Returns false if it doesn’t exist.  No validation occurs here.
Parameters     szColumnName – The name of the column to check.
Returns        bool
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
bool	CDataRow::ColumnExists(const CString& szColumnName)
{
	//Peter Ringering - 02/08/2005 - 1-16669
	CCMSXMLDomDoc::ComponentTypePtr docOwner(m_pNode->ownerDocument);
	//if (m_pDataTable)
	//	if (m_pDataTable->m_pDataSet)
	if (CDataSet::SchemaFieldExists(docOwner, m_szDataTableName, szColumnName))
		return true;

	if (m_nColumnCount == 0 || IsEmpty()) return false;
	CDataCell DC;
	LoadDataCell(this, &DC, szColumnName, false);
	if (DC.IsEmpty()) return false;
	DC.Dispose();
	return true;
}

//PTR.06.08.2005
bool CDataRow::IsNull(const CString& szColumnName)
{
	if (m_nColumnCount == 0 || IsEmpty()) return true;

	CCMSXMLDomDoc::ComponentTypePtr docOwner(m_pNode->ownerDocument);
	if (!CDataSet::SchemaFieldExists(docOwner, m_szDataTableName, szColumnName))
		return true;

	CDataCell DC;
	LoadDataCell(this, &DC, szColumnName, false);
	if (DC.IsEmpty()) return true;

	return false;
}

//PGP(06/03/2004) - a less overhead access method to get string values for
//multiple operations on dataRow.
void CDataRow::XMLStr(int nColumnIndex, CString& szValue, bool bValidate, bool bTrim/* = true*/)
{
	szValue.Empty();
	if (IsEmpty())
		return;
	if (m_nColumnCount == 0)
		return;

	CCMSXMLDomNode::ComponentTypePtr pNode = NULL;
	CCMSXMLDomNodeList::ComponentTypePtr pNodeList = NULL;
	HRESULT hr = m_pNode->get_childNodes(&pNodeList);
	if (hr == S_OK)
		hr = pNodeList->get_item(nColumnIndex, &pNode);
	if (hr != S_OK)
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("columnindex", nColumnIndex);
			XMLParams.MakeParam("maxindex", m_nColumnCount - 1);
			XMLParams.MakeParam("table", m_szDataTableName);
			HandleError(ERR_GB_DATASET_INVALIDCOLINDEX, NULL, &XMLParams);
			XMLParams.Dispose();
		}
		return;
	}
	else
	{
		//PTR.08.10.2005.1-16244
		if (!bTrim)
			CXMLElement::PreserveSpaces(pNode, pNode->ownerDocument);

		BSTR bstdata;
		if (pNode->get_text(&bstdata) == S_OK)
			szValue = bstdata;
	}
	//	if(pNodeList != NULL)
	//		DestroyNodeList(pNodeList);
}

/***************************************************************
Function       XMLStr (Overload #1)
Type           CString
Purpose        Returns a CString variable containing the data located
at the given column index.  If the column index is invalid and bValidate = true,
then a Denali pop-up error dialog will show and the return value will be empty.
Parameters     nColumnIndex – The column index
bValidate (Optional--default=true) - If the column index is invalid do we want to show an error message?
Returns        CString
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
CString CDataRow::XMLStr(int nColumnIndex, bool bValidate, bool bTrim/* = true*/)
{
	CString szValue = "";
	if (IsEmpty()) return szValue;
	CDataCell DC;
	GetDataCell(nColumnIndex, &DC, bValidate, bTrim);
	if (!DC.IsEmpty()) szValue = DC.m_szColumnText;
	DC.Dispose();
	return szValue;
}

/***************************************************************
Function       XMLStr (Overload #2)
Type           CString
Purpose        Returns a CString variable containing the data located
at the given column name.  If the column name is invalid and bValidate = true,
then a Denali pop-up error dialog will show and the return value will be empty.
Parameters     szColumnName – The name of the column
bValidate (Optional--default=true) - If the column name is invalid do we want to show an error message?
Returns        CString
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
CString CDataRow::XMLStr(const CString& szColumnName, bool bValidate, bool bTrim/* = true*/)
{
	CString szValue = "";
	if (IsEmpty()) return szValue;
	CDataCell DC;
	GetDataCell(szColumnName, &DC, bValidate, bTrim);
	if (!DC.IsEmpty()) szValue = DC.m_szColumnText;
	DC.Dispose();
	return szValue;
}

/***************************************************************
Function       XMLLng (Overload #1)
Type           long
Purpose        Returns a long variable containing the data located
at the given column index.  If the column index is invalid and bValidate = true,
then a Denali pop-up error dialog will show and the return value will be empty.
Parameters     nColumnIndex – The column index
bValidate (Optional--default=true) - If the column index is invalid do we want to show an error message?
Returns        long
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
int		CDataRow::XMLInt(int nColumnIndex, bool bValidate)
{
	CString szValue = "";
	if (IsEmpty()) return 0;
	CDataCell DC;
	GetDataCell(nColumnIndex, &DC, bValidate);
	long lValue = GetDataInt(this, &DC);
	DC.Dispose();
	return lValue;
}

/***************************************************************
Function       XMLLng (Overload #2)
Type           long
Purpose        Returns a long variable containing the data located
at the given column name.  If the column name is invalid and bValidate = true,
then a Denali pop-up error dialog will show and the return value will be empty.
Parameters     szColumnName – The name of the column
bValidate (Optional--default=true) - If the column name is invalid do we want to show an error message?
Returns        long
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
int		CDataRow::XMLInt(const CString& szColumnName, bool bValidate)
{
	CString szValue = "";
	if (IsEmpty()) return 0;
	CDataCell DC;
	GetDataCell(szColumnName, &DC, bValidate);
	long lValue = GetDataInt(this, &DC);
	DC.Dispose();
	return lValue;
}

/***************************************************************
Function       XMLLng (Overload #1)
Type           long
Purpose        Returns a long variable containing the data located
at the given column index.  If the column index is invalid and bValidate = true,
then a Denali pop-up error dialog will show and the return value will be empty.
Parameters     nColumnIndex – The column index
bValidate (Optional--default=true) - If the column index is invalid do we want to show an error message?
Returns        long
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
long	CDataRow::XMLLng(int nColumnIndex, bool bValidate)
{
	CString szValue = "";
	if (IsEmpty()) return 0;
	CDataCell DC;
	GetDataCell(nColumnIndex, &DC, bValidate);
	long lValue = GetDataLong(this, &DC);
	DC.Dispose();
	return lValue;
}

/***************************************************************
Function       XMLLng (Overload #2)
Type           long
Purpose        Returns a long variable containing the data located
at the given column name.  If the column name is invalid and bValidate = true,
then a Denali pop-up error dialog will show and the return value will be empty.
Parameters     szColumnName – The name of the column
bValidate (Optional--default=true) - If the column name is invalid do we want to show an error message?
Returns        long
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
long	CDataRow::XMLLng(const CString& szColumnName, bool bValidate)
{
	CString szValue = "";
	if (IsEmpty()) return 0;
	CDataCell DC;
	GetDataCell(szColumnName, &DC, bValidate);
	long lValue = GetDataLong(this, &DC);
	DC.Dispose();
	return lValue;
}

/***************************************************************
Function       XMLBOOL (Overload #1)
Type           bool
Purpose        Returns a bool variable containing the data located
at the given column index.  If the column index is invalid and bValidate = true,
then a Denali pop-up error dialog will show and the return value will be empty.
Parameters     nColumnIndex – The column index
bValidate (Optional--default=true) - If the column index is invalid do we want to show an error message?
Returns        bool
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
bool	CDataRow::XMLBOOL(int nColumnIndex, bool bValidate)
{
	CString szValue = "";
	if (IsEmpty()) return false;
	CDataCell DC;
	GetDataCell(nColumnIndex, &DC, bValidate);
	bool bValue = GetDatabool(this, &DC);
	DC.Dispose();
	return bValue;
}

/***************************************************************
Function       XMLBOOL (Overload #2)
Type           bool
Purpose        Returns a CString variable containing the data located
at the given column name.  If the column name is invalid and bValidate = true,
then a Denali pop-up error dialog will show and the return value will be empty.
Parameters     szColumnName – The name of the column
bValidate (Optional--default=true) - If the column name is invalid do we want to show an error message?
Returns        bool
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
bool	CDataRow::XMLBOOL(const CString& szColumnName, bool bValidate)
{
	CString szValue = "";
	if (IsEmpty()) return false;
	CDataCell DC;
	GetDataCell(szColumnName, &DC, bValidate);
	bool bValue = GetDatabool(this, &DC);
	DC.Dispose();
	return bValue;
}

/***************************************************************
Function       XMLDbl (Overload #1)
Type           double
Purpose        Returns a double variable containing the data located
at the given column index.  If the column index is invalid and bValidate = true,
then a Denali pop-up error dialog will show and the return value will be empty.
Parameters     nColumnIndex – The column index
bValidate (Optional--default=true) - If the column index is invalid do we want to show an error message?
Returns        double
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
double	CDataRow::XMLDbl(int nColumnIndex, bool bValidate)
{
	CString szValue = "";
	if (IsEmpty()) return 0;
	CDataCell DC;
	GetDataCell(nColumnIndex, &DC, bValidate);
	double dValue = GetDataDouble(this, &DC);
	DC.Dispose();
	return dValue;
}

/***************************************************************
Function       XMLDbl (Overload #2)
Type           double
Purpose        Returns a double variable containing the data located
at the given column name.  If the column name is invalid and bValidate = true,
then a Denali pop-up error dialog will show and the return value will be empty.
Parameters     szColumnName – The name of the column
bValidate (Optional--default=true) - If the column name is invalid do we want to show an error message?
Returns        double
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
double	CDataRow::XMLDbl(const CString& szColumnName, bool bValidate)
{
	CString szValue = "";
	if (IsEmpty()) return 0;
	CDataCell DC;
	GetDataCell(szColumnName, &DC, bValidate);
	double dValue = GetDataDouble(this, &DC);
	DC.Dispose();
	return dValue;
}

/***************************************************************
Function       XMLDate (Overload #1)
Type           COleDateTime
Purpose        Returns a COleDateTime variable containing the data located
at the given column index.  If the column index is invalid and bValidate = true,
then a Denali pop-up error dialog will show and the return value will be empty.
Parameters     nColumnIndex – The column index
bValidate (Optional--default=true) - If the column index is invalid do we want to show an error message?
Returns        COleDateTime
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
COleDateTime CDataRow::XMLDate(int nColumnIndex, bool bValidate)
{
	COleDateTime dteDate;
	CString szValue = "";
	if (IsEmpty()) return dteDate;
	CDataCell DC;
	GetDataCell(nColumnIndex, &DC, bValidate);
	dteDate = GetDataDate(this, &DC);
	DC.Dispose();
	return dteDate;
}

/***************************************************************
Function       XMLDate (Overload #2)
Type           COleDateTime
Purpose        Returns a COleDateTime variable containing the data located
at the given column name.  If the column name is invalid and bValidate = true,
then a Denali pop-up error dialog will show and the return value will be empty.
Parameters     szColumnName – The name of the column
bValidate (Optional--default=true) - If the column name is invalid do we want to show an error message?
Returns        COleDateTime
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
COleDateTime CDataRow::XMLDate(const CString& szColumnName, bool bValidate)
{
	CString szValue = "";
	COleDateTime dteDate;
	if (IsEmpty()) return dteDate;
	CDataCell DC;
	GetDataCell(szColumnName, &DC, bValidate);
	dteDate = GetDataDate(this, &DC);
	DC.Dispose();
	return dteDate;
}

/***************************************************************
Function       XMLByte (Overload #1)
Type           byte
Purpose        Returns a byte variable containing the data located
at the given column index.  If the column index is invalid and bValidate = true,
then a Denali pop-up error dialog will show and the return value will be empty.
Parameters     nColumnIndex – The column index
bValidate (Optional--default=true) - If the column index is invalid do we want to show an error message?
Returns        byte
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
byte	CDataRow::XMLByte(int nColumnIndex, bool bValidate)
{
	byte bytValue = 0;
	CString szValue = "";
	if (IsEmpty()) return bytValue;
	CDataCell DC;
	GetDataCell(nColumnIndex, &DC, bValidate);
	bytValue = GetDataByte(this, &DC);
	DC.Dispose();
	return bytValue;
}

/***************************************************************
Function       XMLByte (Overload #2)
Type           byte
Purpose        Returns a byte variable containing the data located
at the given column name.  If the column name is invalid and bValidate = true,
then a Denali pop-up error dialog will show and the return value will be empty.
Parameters     szColumnName – The name of the column
bValidate (Optional--default=true) - If the column name is invalid do we want to show an error message?
Returns        byte
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
byte	CDataRow::XMLByte(const CString& szColumnName, bool bValidate)
{
	byte bytValue = 0;
	CString szValue = "";
	if (IsEmpty()) return bytValue;
	CDataCell DC;
	GetDataCell(szColumnName, &DC, bValidate);
	bytValue = GetDataByte(this, &DC);
	DC.Dispose();
	return bytValue;
}

// *********************************************************
// * Function	SetRowValue
// * Type		void
// * Purpose	Changes the value for the column in the table.
// * Parameters	nColumnIndex (Overloads 1, 3, 5, 7) – The column index
// *    		szColumnName (Overloads 2, 4, 6, 8) – The name of the column
// *    		szValue (Overloads 1, 2) - Value to set.
// *			nValue (int) (Overloads 3, 4) - Value to set.
// *    		dValue (double) (Overloads 5, 6) - Value to set.
// *    		dteValue (COleDateTime) (Overloads 7, 8) - Value to set.
// * Returns	void
// *********************************************************
void	CDataRow::SetRowValue(int nColumnIndex, const CString& szValue)
{
	CDataCell clsDC;
	GetDataCell(nColumnIndex, &clsDC);
	if (clsDC.IsEmpty()) return;
	clsDC.SetValue(szValue);
	clsDC.Dispose();
}

// *********************************************************
// * Function	SetRowValue
// * Type		void
// * Purpose	Changes the value for the column in the table.
// * Parameters	nColumnIndex (Overloads 1, 3, 5, 7) – The column index
// *    		szColumnName (Overloads 2, 4, 6, 8) – The name of the column
// *    		szValue (Overloads 1, 2) - Value to set.
// *			nValue (int) (Overloads 3, 4) - Value to set.
// *    		dValue (double) (Overloads 5, 6) - Value to set.
// *    		dteValue (COleDateTime) (Overloads 7, 8) - Value to set.
// * Returns	void
// *********************************************************
void	CDataRow::SetRowValue(const CString& szColumnName, const CString& szValue)
{
	CDataCell clsDC;
	GetDataCell(szColumnName, &clsDC, false);
	if (clsDC.IsEmpty()) return;
	clsDC.SetValue(szValue);
	clsDC.Dispose();
}

// *********************************************************
// * Function	SetRowValue
// * Type		void
// * Purpose	Changes the value for the column in the table.
// * Parameters	nColumnIndex (Overloads 1, 3, 5, 7) – The column index
// *    		szColumnName (Overloads 2, 4, 6, 8) – The name of the column
// *    		szValue (Overloads 1, 2) - Value to set.
// *			nValue (int) (Overloads 3, 4) - Value to set.
// *    		dValue (double) (Overloads 5, 6) - Value to set.
// *    		dteValue (COleDateTime) (Overloads 7, 8) - Value to set.
// * Returns	void
// *********************************************************
void	CDataRow::SetRowValue(int nColumnIndex, int nValue)
{
	CDataCell clsDC;
	GetDataCell(nColumnIndex, &clsDC);
	if (clsDC.IsEmpty()) return;
	clsDC.SetValue(nValue);
	clsDC.Dispose();
}

// *********************************************************
// * Function	SetRowValue
// * Type		void
// * Purpose	Changes the value for the column in the table.
// * Parameters	nColumnIndex (Overloads 1, 3, 5, 7) – The column index
// *    		szColumnName (Overloads 2, 4, 6, 8) – The name of the column
// *    		szValue (Overloads 1, 2) - Value to set.
// *			nValue (int) (Overloads 3, 4) - Value to set.
// *    		dValue (double) (Overloads 5, 6) - Value to set.
// *    		dteValue (COleDateTime) (Overloads 7, 8) - Value to set.
// * Returns	void
// *********************************************************
void	CDataRow::SetRowValue(const CString& szColumnName, int nValue)
{
	CDataCell clsDC;
	GetDataCell(szColumnName, &clsDC, false);
	if (clsDC.IsEmpty()) return;
	clsDC.SetValue(nValue);
	clsDC.Dispose();
}

// *********************************************************
// * Function	SetRowValue
// * Type		void
// * Purpose	Changes the value for the column in the table.
// * Parameters	nColumnIndex (Overloads 1, 3, 5, 7) – The column index
// *    		szColumnName (Overloads 2, 4, 6, 8) – The name of the column
// *    		szValue (Overloads 1, 2) - Value to set.
// *			nValue (int) (Overloads 3, 4) - Value to set.
// *    		dValue (double) (Overloads 5, 6) - Value to set.
// *    		dteValue (COleDateTime) (Overloads 7, 8) - Value to set.
// * Returns	void
// *********************************************************
void	CDataRow::SetRowValue(int nColumnIndex, double dValue)
{
	CDataCell clsDC;
	GetDataCell(nColumnIndex, &clsDC);
	if (clsDC.IsEmpty()) return;
	clsDC.SetValue(dValue);
	clsDC.Dispose();
}

// *********************************************************
// * Function	SetRowValue
// * Type		void
// * Purpose	Changes the value for the column in the table.
// * Parameters	nColumnIndex (Overloads 1, 3, 5, 7) – The column index
// *    		szColumnName (Overloads 2, 4, 6, 8) – The name of the column
// *    		szValue (Overloads 1, 2) - Value to set.
// *			nValue (int) (Overloads 3, 4) - Value to set.
// *    		dValue (double) (Overloads 5, 6) - Value to set.
// *    		dteValue (COleDateTime) (Overloads 7, 8) - Value to set.
// * Returns	void
// *********************************************************
void	CDataRow::SetRowValue(const CString& szColumnName, double dValue)
{
	CDataCell clsDC;
	GetDataCell(szColumnName, &clsDC, false);
	if (clsDC.IsEmpty()) return;
	clsDC.SetValue(dValue);
	clsDC.Dispose();
}

// *********************************************************
// * Function	SetRowValue
// * Type		void
// * Purpose	Changes the value for the column in the table.
// * Parameters	nColumnIndex (Overloads 1, 3, 5, 7) – The column index
// *    		szColumnName (Overloads 2, 4, 6, 8) – The name of the column
// *    		szValue (Overloads 1, 2) - Value to set.
// *			nValue (int) (Overloads 3, 4) - Value to set.
// *    		dValue (double) (Overloads 5, 6) - Value to set.
// *    		dteValue (COleDateTime) (Overloads 7, 8) - Value to set.
// * Returns	void
// *********************************************************
void	CDataRow::SetRowValue(int nColumnIndex, const COleDateTime& dteValue)
{
	CDataCell clsDC;
	GetDataCell(nColumnIndex, &clsDC);
	if (clsDC.IsEmpty()) return;
	clsDC.SetValue(dteValue);
	clsDC.Dispose();
}

// *********************************************************
// * Function	SetRowValue
// * Type		void
// * Purpose	Changes the value for the column in the table.
// * Parameters	nColumnIndex (Overloads 1, 3, 5, 7) – The column index
// *    		szColumnName (Overloads 2, 4, 6, 8) – The name of the column
// *    		szValue (Overloads 1, 2) - Value to set.
// *    		nValue (int) (Overloads 3, 4) - Value to set.
// *    		dValue (double) (Overloads 5, 6) - Value to set.
// *    		dteValue (COleDateTime) (Overloads 7, 8) - Value to set.
// * Returns	void
// *********************************************************
void	CDataRow::SetRowValue(const CString& szColumnName, const COleDateTime& dteValue)
{
	CDataCell clsDC;
	GetDataCell(szColumnName, &clsDC, false);
	if (clsDC.IsEmpty()) return;
	clsDC.SetValue(dteValue);
	clsDC.Dispose();
}

/***************************************************************
Function       SetRowValues
Type           void
Purpose        Sets row values using passed in XML.
NOTE:  The passed in XML needs to have the same "column"
structure as the row.
Parameters     szXML - XML string to use.
Returns        void
Author         Peter Ringering
Date           01/12/2003
****************************************************************/
void	CDataRow::SetRowValues(const CString& szXML)
{
	CXMLParams clsXML(true);
	clsXML.SetXML(szXML);
	if (clsXML.IsEmpty()) return;
	for (long lCounter = 0; lCounter < clsXML.GetParamCount(); lCounter++)
	{
		CCMSXMLDomNode::ComponentTypePtr pNode = clsXML.m_pRoot->childNodes->Getitem(lCounter);
		CDataCell clsDC;
		GetDataCell((BSTR)pNode->nodeName, &clsDC, false);
		if (!clsDC.IsEmpty())
		{
			clsDC.SetValue((BSTR)pNode->text);
		}
		clsDC.Dispose();
		//		DestroyNode(pNode);
	}
	clsXML.Dispose();
}

/***************************************************************
Function       GetXMLBSTR
Type           BSTR
Purpose        Returns the XML for the passed in column.  This is very useful
if the cell has child nodes that you want to parse using a CXMLParams object.
Parameters     nColumnIndex – The column index
Returns        BSTR
Author         Peter Ringering
Date           01/12/2003
****************************************************************/
_bstr_t CDataRow::GetXMLBSTR(int nColumnIndex, bool bValidate)
{
	_bstr_t bstReturn;
	CDataCell clsDC;
	GetDataCell(nColumnIndex, &clsDC, bValidate);
	if (!clsDC.IsEmpty()) bstReturn = clsDC.GetXMLBSTR();
	clsDC.Dispose();
	return bstReturn;
}

/***************************************************************
Function       GetXMLBSTR
Type           BSTR
Purpose        Returns the XML for the passed in column.  This is very useful
if the cell has child nodes that you want to parse using a CXMLParams object.
Parameters     szColumnName – The name of the column
Returns        BSTR
Author         Peter Ringering
Date           01/12/2003
****************************************************************/
_bstr_t CDataRow::GetXMLBSTR(const CString& szColumnName, bool bValidate)
{
	CDataCell clsDC;
	GetDataCell(szColumnName, &clsDC, bValidate);
	_bstr_t bstReturn;
	if (clsDC.IsEmpty()) return bstReturn;
	bstReturn = clsDC.GetXMLBSTR();
	clsDC.Dispose();
	return bstReturn;
}

/***************************************************************
Function       GetXMLCString
Type           CString
Purpose        Returns the XML for the passed in column.  This is very useful
if the cell has child nodes that you want to parse using a CXMLParams object.
Parameters     nColumnIndex – The column index
Returns        BSTR
Author         Peter Ringering
Date           01/12/2003
****************************************************************/
CString CDataRow::GetXMLCString(int nColumnIndex, bool bValidate)
{
	return LPCTSTR(GetXMLBSTR(nColumnIndex, bValidate));
}

/***************************************************************
Function       GetXMLCString
Type           CString
Purpose        Returns the XML for the passed in column.  This is very useful
if the cell has child nodes that you want to parse using a CXMLParams object.
Parameters     szColumnName – The name of the column
Returns        BSTR
Author         Peter Ringering
Date           01/12/2003
****************************************************************/
CString CDataRow::GetXMLCString(const CString& szColumnName, bool bValidate)
{
	return CString(GetXMLBSTR(szColumnName, bValidate).GetBSTR());
}

/***************************************************************
Function       GetXML
Type           CString
Purpose        Returns the XML inside the DataRow.
Parameters     None
Returns        CString
Author         Peter Ringering
Date           01/12/2003
****************************************************************/
CString CDataRow::GetXML()
{
	return CString(m_pNode->Getxml().GetBSTR());
}

/***************************************************************
Function       CDataCell (Constructor)
Type					 void
Purpose        Constructs a CDataCell Object
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
CDataCell::CDataCell()
	: m_pDataRow(NULL)
	, m_bTrimValue(true)
{
	CoInitialize(NULL);
	Dispose();
}

/***************************************************************
Function       CDataCell (Constructor)
Type					 void
Purpose        Constructs a CDataCell Object
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
CDataCell::CDataCell(const CDataCell& inRHS)
	: m_szDataTableName(inRHS.m_szDataTableName)
	, m_szColumnName(inRHS.m_szColumnName)
	, m_szColumnText(inRHS.m_szColumnText)
	, m_lRowIndex(inRHS.m_lRowIndex)
	, m_pNode(inRHS.m_pNode)
	, m_pDataRow(inRHS.m_pDataRow)
	, m_bTrimValue(inRHS.m_bTrimValue)
{
	CoInitialize(NULL);
	Dispose();
}

/***************************************************************
Function       CDataCell (Constructor)
Type					 void
Purpose        Constructs a CDataCell Object
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
CDataCell::CDataCell(CDataRow& objDataRow)
	: m_pDataRow(&objDataRow)
	, m_bTrimValue(true)
{
	CoInitialize(NULL);
	Dispose();

	//Peter Ringering - 02/08/2005 - 1-16669
	//m_pDataRow->m_pNode.AddRef();
}

//------------------------------------------------------------------------------
const CDataCell& CDataCell::operator= (const CDataCell& inRHS)
{
	if (this != &inRHS)
	{
		m_szDataTableName = inRHS.m_szDataTableName;
		m_szColumnName = inRHS.m_szColumnName;
		m_szColumnText = inRHS.m_szColumnText;
		m_lRowIndex = inRHS.m_lRowIndex;
		m_pNode = inRHS.m_pNode;

		//Peter Ringering - 02/08/2005 - 1-16669
		//if (m_pDataRow)
		//	m_pDataRow->m_pNode.Release();

		m_pDataRow = inRHS.m_pDataRow;
		//if (m_pDataRow)
		//	m_pDataRow->m_pNode.AddRef();
		m_bTrimValue = inRHS.m_bTrimValue;
	}

	return *this;
}

/***************************************************************
Function       CDataCell (Destructor)
Type					 void
Purpose        Destroys m_pNode then finally the CDataCell Object
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
CDataCell::~CDataCell()
{
	Dispose();

	//Peter Ringering - 02/08/2005 - 1-16669
	//if (m_pDataRow) m_pDataRow->m_pNode.Release();
}

/***************************************************************
Function       Dispose
Type           void
Purpose        Releases and sets to NULL the XML Node.
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
void	CDataCell::Dispose()
{
	if (!IsEmpty())
	{
		m_pNode.Release();
		m_pNode = NULL;
	}
	m_szColumnName = "";
	m_szColumnText = "";
	m_bTrimValue = true;
}

/***************************************************************
Function       IsEmpty
Type           bool
Purpose        Checks to see if the XML Node is NULL.
Parameters     None
Returns        bool
Author         Peter Ringering
Date           11/01/2002
****************************************************************/
bool	CDataCell::IsEmpty()
{
	return (m_pNode == NULL);
}

// *********************************************************
// * Function	SetValue
// * Type		void
// * Purpose	Changes the cell value to what's passed in.
// * Parameters	szValue (Overload 1) - Value to set the cell to.
// *    		intValue (Overload 2) - Value to set the cell to.
// *    		dValue (Overload 3) - Value to set the cell to.
// *    		dteValue (Overload 4) - Value to set the cell to.
// * Returns	void
// *********************************************************
void	CDataCell::SetValue(const CString& szValue)
{
	m_pNode->text = _bstr_t(szValue);
	m_szColumnText = szValue;
}

// *********************************************************
// * Function	SetValue
// * Type		void
// * Purpose	Changes the cell value to what's passed in.
// * Parameters	szValue (Overload 1) - Value to set the cell to.
// *    		intValue (Overload 2) - Value to set the cell to.
// *    		dValue (Overload 3) - Value to set the cell to.
// *    		dteValue (Overload 4) - Value to set the cell to.
// * Returns	void
// *********************************************************
void	CDataCell::SetValue(int nValue)
{
	CString szTemp;
	szTemp.Format(_T("%d"), nValue);
	m_pNode->text = _bstr_t(szTemp);
	m_szColumnText = szTemp;
}

// *********************************************************
// * Function	SetValue
// * Type		void
// * Purpose	Changes the cell value to what's passed in.
// * Parameters	szValue (Overload 1) - Value to set the cell to.
// *    		intValue (Overload 2) - Value to set the cell to.
// *    		dValue (Overload 3) - Value to set the cell to.
// *    		dteValue (Overload 4) - Value to set the cell to.
// * Returns	void
// *********************************************************
void	CDataCell::SetValue(double dValue)
{
	CString szTemp;
	szTemp.Format(_T("%f"), dValue);
	m_pNode->text = _bstr_t(szTemp);
	m_szColumnText = szTemp;
}

// *********************************************************
// * Function	SetValue
// * Type		void
// * Purpose	Changes the cell value to what's passed in.
// * Parameters	szValue (Overload 1) - Value to set the cell to.
// *    		intValue (Overload 2) - Value to set the cell to.
// *    		dValue (Overload 3) - Value to set the cell to.
// *    		dteValue (Overload 4) - Value to set the cell to.
// * Returns	void
// *********************************************************
void	CDataCell::SetValue(const COleDateTime& dteValue)
{
	CString szTemp;
	szTemp = dteValue.Format(VAR_DATEVALUEONLY);
	m_pNode->text = _bstr_t(szTemp);
	m_szColumnText = szTemp;
}

/*********************************************************
Function		GetDataDouble
Type			double
Purpose			Formats the pDC's m_szColumnText value into
a double variable.  If the value cannot be
formatted, then a validation message is
displayed to the user.
Parameters		None
Returns			double
Author			Peter Ringering
Date			11/01/2002
***********************************************************/
double	CDataCell::GetDataDouble()
{

	double dValue = 0;
	CString szValue = "";
	if (!this->IsEmpty()) szValue = this->m_szColumnText;
	if (szValue != "")
	{
		try
		{
			dValue = _tstof(szValue);
			if (!CGBLForm::ValidNumber(CGBLForm::TYP_DOUBLE, szValue)) // begbert 02-15-2006 VS8: warning C4482
				throw(szValue);
		}
		catch (CXMLParams XMLParams)
		{
			//CXMLParams XMLParams;
			XMLParams.MakeParam("value", szValue);
			XMLParams.MakeParam("vartype", "double");
			XMLParams.MakeParam("table", this->m_szDataTableName);
			XMLParams.MakeParam("row", this->m_lRowIndex);
			XMLParams.MakeParam("column", this->m_szColumnName);
			HandleError(ERR_GB_DATASET_INVALIDVALUE, NULL, &XMLParams);
			XMLParams.Dispose();
		}
	}
	return dValue;
}

/*********************************************************
Function		GetDataInt
Type			int
Purpose			Formats the pDC's m_szColumnText value into
a int variable.  If the value cannot be
formatted, then a validation message is
displayed to the user.
Parameters		None
Returns			int
Author			David Parvin
Date			05/05/2004
***********************************************************/
int		CDataCell::GetDataInt()
{
	int nValue = 0;
	CString szValue = "";
	if (!this->IsEmpty()) szValue = this->m_szColumnText;
	if (szValue != "")
	{
		try
		{
			nValue = _tstoi(szValue);
			if (!CGBLForm::ValidNumber(CGBLForm::TYP_INT, szValue)) // begbert 02-15-2006 VS8: warning C4482
				throw(szValue);
		}
		catch (...)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("value", szValue);
			XMLParams.MakeParam("vartype", "long");
			XMLParams.MakeParam("table", this->m_szDataTableName);
			XMLParams.MakeParam("row", this->m_lRowIndex);
			XMLParams.MakeParam("column", this->m_szColumnName);
			HandleError(ERR_GB_DATASET_INVALIDVALUE, NULL, &XMLParams);
			XMLParams.Dispose();
		}
	}
	return nValue;
}

/*********************************************************
Function       GetDataLong
Type           long
Purpose        Formats the pDC's m_szColumnText value into a long variable.
If the value cannot be formatted, then a validation message is
displayed to the user.
Parameters     pDR - Pointer to a CDataRow object.
pDC - Pointer to a CDataCell object.
Returns        long
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
long	CDataCell::GetDataLong()
{

	long lValue = 0L;
	CString szValue = "";
	if (!this->IsEmpty()) szValue = this->m_szColumnText;
	if (szValue != "")
	{
		try
		{
			lValue = _tstol(szValue);
			if (!CGBLForm::ValidNumber(CGBLForm::TYP_LONG, szValue)) // begbert 02-15-2006 VS8: warning C4482
				throw(szValue);
		}
		catch (...)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("value", szValue);
			XMLParams.MakeParam("vartype", "long");
			XMLParams.MakeParam("table", this->m_szDataTableName);
			XMLParams.MakeParam("row", this->m_lRowIndex);
			XMLParams.MakeParam("column", this->m_szColumnName);
			HandleError(ERR_GB_DATASET_INVALIDVALUE, NULL, &XMLParams);
			XMLParams.Dispose();
		}
	}
	return lValue;
}

/*********************************************************
Function       GetDatabool
Type           bool
Purpose        Formats the pDC's m_szColumnText value into a bool variable.
0=false or "false" = false, <>0=true, or "true" = true.
If the value cannot be formatted, then a validation message is
displayed to the user.
Parameters     pDR - Pointer to a CDataRow object.
pDC - Pointer to a CDataCell object.
Returns        bool
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
bool	CDataCell::GetDatabool()
{
	bool bValue = false;
	CString szValue = "";
	if (!this->IsEmpty()) szValue = this->m_szColumnText;
	if (szValue != "")
	{
		if (szValue.CompareNoCase(_T("true")) != 0)
		{
			if (szValue.CompareNoCase(_T("false")) != 0)
			{
				int nValue = _tstoi(szValue);
				switch (nValue)
				{
				case 0:
					break;
				case 1:
				case -1:
					bValue = true;
					break;
				default:
					CXMLParams XMLParams;
					XMLParams.MakeParam("value", szValue);
					XMLParams.MakeParam("vartype", "boolean");
					XMLParams.MakeParam("table", this->m_szDataTableName);
					XMLParams.MakeParam("row", this->m_lRowIndex);
					XMLParams.MakeParam("column", this->m_szColumnName);
					HandleError(ERR_GB_DATASET_INVALIDVALUE, NULL, &XMLParams);
					XMLParams.Dispose();
				}
			}
		}
		else
		{
			bValue = true;
		}
	}
	return bValue;
}

/*********************************************************
Function		GetDataDate
Type			COleDateTime
Purpose			Formats the m_szColumnText value into a
COleDateTime variable.  If the value
cannot be formatted, then a validation
message is displayed to the user.
Parameters		None
Returns			COleDateTime
Author			Peter Ringering
Date			11/01/2002
***********************************************************/
COleDateTime CDataCell::GetDataDate()
{
	COleDateTime dteDate;
	CString szValue = "";
	if (!this->IsEmpty()) szValue = this->m_szColumnText;
	if (szValue != "")
	{
		try
		{
			dteDate.ParseDateTime(szValue.Left(10) + " " + szValue.Mid(11, 8));
		}
		catch (...)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("value", szValue);
			XMLParams.MakeParam("vartype", "date");
			XMLParams.MakeParam("table", this->m_szDataTableName);
			XMLParams.MakeParam("row", this->m_lRowIndex);
			XMLParams.MakeParam("column", this->m_szColumnName);
			HandleError(ERR_GB_DATASET_INVALIDVALUE, NULL, &XMLParams);
			XMLParams.Dispose();
		}
	}
	return dteDate;
}

/*********************************************************
Function		GetDataByte
Type			byte
Purpose			Formats the m_szColumnText value into a
byte variable.  If the value cannot be
formatted, then a validation message is
displayed to the user.
Parameters		None
Returns			byte
Author			Peter Ringering
Date			11/01/2002
***********************************************************/
byte	CDataCell::GetDataByte()
{
	byte bytValue = 0;
	CString szValue = "";
	if (!this->IsEmpty()) szValue = this->m_szColumnText;
	if (szValue != "")
	{
		try
		{
			bytValue = (byte)_tstoi(szValue);
			if (!CGBLForm::ValidNumber(CGBLForm::TYP_BYTE, szValue)) // begbert 02-15-2006 VS8: warning C4482
				throw(szValue);
		}
		catch (...)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("value", szValue);
			XMLParams.MakeParam("vartype", "byte");
			XMLParams.MakeParam("table", this->m_szDataTableName);
			XMLParams.MakeParam("row", this->m_lRowIndex);
			XMLParams.MakeParam("column", this->m_szColumnName);
			HandleError(ERR_GB_DATASET_INVALIDVALUE, NULL, &XMLParams);
			XMLParams.Dispose();
		}
	}
	return bytValue;
}

/*********************************************************
Function       GetXMLBSTR
Type           BSTR
Purpose        Returns the XML inside the DataCell.  This is very useful
if the cell has child nodes that you want to parse using a CXMLParams object.
Parameters     None
Returns        BSTR
Author         Peter Ringering
Date           01/12/2003
***********************************************************/
_bstr_t CDataCell::GetXMLBSTR()
{
	return m_pNode->xml;
}

/*********************************************************
Function       HasChildren
Type           bool
Purpose        Checks to see if this cell has any "child nodes".
(It shouldn't but it could.)
Parameters     None
Returns        bool
Author         Peter Ringering
Date           01/12/2003
***********************************************************/
bool	CDataCell::HasChildren()
{
	//For some reason, Microsoft's hasChildNodes method always returns true and the chidNodes->length
	//property is 1 1f there are no child nodes or if there is only 1 child node.
	long lLength = m_pNode->childNodes->length;
	return (lLength > 1);
}

/*********************************************************
Function       CXMLParams (Constructor)
Type					 void
Purpose        Constructs a CXMLParams object.  If bManual is false, then it executes
SetRoot with the root node name being “ROOT”.
Parameters     bManual (Optional--default=false) - Do we want the root node set manually or not?
Returns        void
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
CXMLParams::CXMLParams(bool bManual)
{
	m_szRootName = "";
	CoInitialize(NULL);
	try
	{
		if (!bManual) SetRoot("ROOT");
	}
	catch (CException* pe)
	{
		HandleError(0, pe);
	}
}

/*********************************************************
Function			CXMLParams (Copy Constructor)
Type				CXMLParams&
Purpose			Constructs a copy of an existing CXMLParams object.
Parameters	The object to be copied
Returns			void
Author			Sonny Barber
Date				21-oct-2003
***********************************************************/
CXMLParams::CXMLParams(const CXMLParams& inRHS)
	: m_pDOMDocument(inRHS.m_pDOMDocument)	//m_pDOMDocument is a _com_ptr. this means that we will get a call to AddRef on the RHS.
	, m_pRoot(inRHS.m_pRoot)									//ditto
	, m_szRootName(inRHS.m_szRootName)
	, m_lParamCount(inRHS.m_lParamCount)
{
	//intentionally left blank
}

/*********************************************************
Function			operator=
Type				CXMLParams&
Purpose			Constructs a copy of an existing CXMLParams object.
Parameters	The object to be assigned
Returns			CXMLParams&
Author			Sonny Barber
Date				21-oct-2003
***********************************************************/
const CXMLParams& CXMLParams::operator= (const CXMLParams& inRHS)
{
	if (this != &inRHS)
	{
		m_pDOMDocument = inRHS.m_pDOMDocument;	//m_pDOMDocument is a _com_ptr. this means that we will get a call to Release on the LHS and an AddRef on the RHS.
		m_pRoot = inRHS.m_pRoot;								//ditto
		m_szRootName = inRHS.m_szRootName;
		m_lParamCount = inRHS.m_lParamCount;
	}
	return (*this);
}

/*********************************************************
Function       CXMLParams (Destructor)
Type					 void
Purpose        Destroys m_pDOMDocument then finally the CXMLParams Object
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
CXMLParams::~CXMLParams()
{
	if (m_pDOMDocument != NULL)
	{
		//		DestroyNode(m_pRoot);

		try
		{
			m_pDOMDocument.Release();
		}
		catch (...) {};

		m_pDOMDocument = NULL;
	}
}

/*********************************************************
Function       SetRoot
Type           void
Purpose        Initializes the m_pRoot node with the name of the passed in root name.
Parameters     szRootNode – Name of the root node.
Returns        void
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
void	CXMLParams::SetRoot(const CString& szRootName)
{
	if (m_szRootName != "")
	{
		CXMLParams clsParams;
		clsParams.MakeParam("ROOTNODE", m_szRootName);
		HandleError(ERR_GB_CXMLPARAMS_ROOTEXISTS, NULL, &clsParams);
		clsParams.Dispose();
		return;
	}
	m_pDOMDocument = MakeXMLDocument(szRootName);
	if (m_pDOMDocument != NULL)
		m_pRoot = m_pDOMDocument->documentElement;
}

CString	CXMLParams::GetXMLRootNodeName()
{
	if (m_pRoot == NULL)
	{
		CXMLParams clsParams;
		clsParams.MakeParam("XML", _T(""));
		HandleError(ERR_GB_CXMLPARAMS_BADXML, NULL, &clsParams);
		clsParams.Dispose();
		Dispose();
		return _T("");
	}
	CString szReturn = LPCTSTR(m_pRoot->nodeName);
	return szReturn;
}

/*********************************************************
Function       GetXML
Type           CString
Purpose        Returns the XML string from the document.
Parameters     None
Returns        CString
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
CString CXMLParams::GetXML()
{
	return CString(m_pDOMDocument->Getxml().GetBSTR());
}

/*********************************************************
Function       GetXML
Type           CString
Purpose        Returns the XML string for the passed in node.
Parameters     szTag - Node tag.
Returns        CString
Author         Peter Ringering
Date           01/12/2003
***********************************************************/
CString CXMLParams::GetXML(const CString& szTag, bool bValidate)
{
	try
	{
		CString szReturn = (BSTR)m_pRoot->selectSingleNode(_bstr_t(szTag))->xml;
		return szReturn;
	}
	catch (...)
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("tagname", szTag);
			XMLParams.MakeParam("nodename", m_szRootName);
			HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
			XMLParams.Dispose();
		}
		return "";
	}
}

/*********************************************************
Function       GetXMLBSTR
Type           BSTR
Purpose        Returns the XML string from the document in BSTR format.
Parameters     None
Returns        BSTR
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
_bstr_t	CXMLParams::GetXMLBSTR()
{
	return m_pDOMDocument->Getxml();
}

/*********************************************************
Function       SetXML (Overload #1)
Type           void
Purpose        Loads up the XML Document with XML from the
parameter then it initializes the root node.
It is best to use this method only if you construct
the object using the bManual = true overload.  If
the XML is invalid, then a Denali pop-up error dialog
will show and the XML document and the root node will remain NULL.
Parameters     szXML – The CString variable containing the XML text.
Returns        void
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
void	CXMLParams::SetXML(const CString& szXML)
{
	SetXML(_bstr_t(szXML));
}

/*********************************************************
Function       SetXML (Overload #2)
Type           void
Purpose        Loads up the XML Document with XML from the
parameter then it initializes the root node.
It is best to use this method only if you construct
the object using the bManual = true overload.  If
the XML is invalid, then a Denali pop-up error dialog
will show and the XML document and the root node will remain NULL.
Parameters     szXML – The BSTR variable containing the XML text.
Returns        void
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
void	CXMLParams::SetXML(_bstr_t bstXML)
{
	if (!m_szRootName.IsEmpty()) return;
	try
	{
		CCMSXMLDomDoc::GetPolicy().CreateObject(m_pDOMDocument, CCMSXMLDomDoc::GetProgID());
		m_pDOMDocument->loadXML(bstXML);
		m_pRoot = m_pDOMDocument->documentElement;
		if (m_pRoot == NULL)
		{
			CXMLParams clsParams;
			clsParams.MakeParam("XML", bstXML.copy());
			HandleError(ERR_GB_CXMLPARAMS_BADXML, NULL, &clsParams);
			clsParams.Dispose();
			Dispose();
			return;
		}
		m_szRootName = LPCTSTR(m_pRoot->baseName);
		m_lParamCount = m_pRoot->childNodes->length;
	}
	catch (CException* pe)
	{
		HandleError(0, pe);
	}
}

// *********************************************************
// * Function 	SetRootNode
// * Type 		void
// * Purpose  	Initializes m_pRoot using the parameter.
// *      		It is best to use this method only if you
// *      		construct the object using the bManual =
// *      		true overload.  If the node is invalid or
// *      		NULL, then a pop-up error dialog will show
// *      		and the XML document and root node will
// *      		remain NULL.
// * Parameters	pRoot - MSXML2 Node to initialize this with.
// * Returns		void
// *********************************************************
void	CXMLParams::SetRootNode(CCMSXMLDomNode::ComponentTypePtr pRoot)
{
	try
	{
		SetXML(pRoot->Getxml());
	}
	catch (CException* pe)
	{
		HandleError(0, pe);
	}
}

// *********************************************************
// * Function 	MakeParam
// *
// * Purpose 	Adds a node to m_pRoot and with the name
// *      		and value equal to the input parameters.
// *
// * Parameters	szTagName – Name of the node’s tag.
// *      		szValue – The node’s value (Overload #1)
// *      		uiValue – The node’s value (Overload #2)
// *      		iValue – The node’s value (Overload #3)
// *      		lValue – The node’s value (Overload #4)
// *      		dValue – The node’s value (Overload #5)
// *
// * Returns  	void
// *********************************************************
void	CXMLParams::MakeParam(const CString& szTagName)
{
	if (m_pRoot == NULL)
	{
		HandleError(ERR_GB_CXMLPARAMS_NOXMLROOT);
		return;
	}
	CCMSXMLDomNode::ComponentTypePtr pNode;
	try
	{
		pNode = m_pDOMDocument->createNode("element", _bstr_t(szTagName), "");
		m_pRoot->appendChild(pNode);
		m_lParamCount++;
	}
	catch (CException* pe)
	{
		HandleError(0, pe);
	}
	catch (_com_error e)
	{
		stuErrorReturn clsErrorReturn;
		CGBLError clsError(clsErrorReturn);
		clsError.HandleException(&e);
	}
	//	DestroyNode(pNode);
}

// *********************************************************
// * Function 	MakeParam
// *
// * Purpose 	Adds a node to m_pRoot and with the name
// *      		and value equal to the input parameters.
// *
// * Parameters	szTagName – Name of the node’s tag.
// *      		szValue – The node’s value (Overload #1)
// *      		uiValue – The node’s value (Overload #2)
// *      		iValue – The node’s value (Overload #3)
// *      		lValue – The node’s value (Overload #4)
// *      		dValue – The node’s value (Overload #5)
// *
// * Returns  	void
// *********************************************************
void	CXMLParams::MakeParam(const CString& szTagName, const CString& szValue)
{
	if (m_pRoot == NULL)
	{
		HandleError(ERR_GB_CXMLPARAMS_NOXMLROOT);
		return;
	}
	CCMSXMLDomNode::ComponentTypePtr pNode;
	try
	{
		pNode = m_pDOMDocument->createNode("element", _bstr_t(szTagName), "");
		pNode->text = _bstr_t(szValue);
		m_pRoot->appendChild(pNode);
		m_lParamCount++;
	}
	catch (CException* pe)
	{
		HandleError(0, pe);
	}
	catch (_com_error e)
	{
		stuErrorReturn clsErrorReturn;
		CGBLError clsError(clsErrorReturn);
		clsError.HandleException(&e);
	}
	//	DestroyNode(pNode);
}

// *********************************************************
// * Function 	MakeParam
// *
// * Purpose 	Adds a node to m_pRoot and with the name
// *      		and value equal to the input parameters.
// *
// * Parameters	szTagName – Name of the node’s tag.
// *      		szValue – The node’s value (Overload #1)
// *      		uiValue – The node’s value (Overload #2)
// *      		iValue – The node’s value (Overload #3)
// *      		lValue – The node’s value (Overload #4)
// *      		dValue – The node’s value (Overload #5)
// *
// * Returns  	void
// *********************************************************
void	CXMLParams::MakeParam(const CString& szTagName, UINT uiValue)
{
	MakeParam(szTagName, (long)uiValue);
}

// *********************************************************
// * Function 	MakeParam
// *
// * Purpose 	Adds a node to m_pRoot and with the name
// *      		and value equal to the input parameters.
// *
// * Parameters	szTagName – Name of the node’s tag.
// *      		szValue – The node’s value (Overload #1)
// *      		uiValue – The node’s value (Overload #2)
// *      		iValue – The node’s value (Overload #3)
// *      		lValue – The node’s value (Overload #4)
// *      		dValue – The node’s value (Overload #5)
// *
// * Returns  	void
// *********************************************************
void	CXMLParams::MakeParam(const CString& szTagName, int iValue)
{
	MakeParam(szTagName, (long)iValue);
}

// *********************************************************
// * Function 	MakeParam
// *
// * Purpose 	Adds a node to m_pRoot and with the name
// *      		and value equal to the input parameters.
// *
// * Parameters	szTagName – Name of the node’s tag.
// *      		szValue – The node’s value (Overload #1)
// *      		uiValue – The node’s value (Overload #2)
// *      		iValue – The node’s value (Overload #3)
// *      		lValue – The node’s value (Overload #4)
// *      		dValue – The node’s value (Overload #5)
// *
// * Returns  	void
// *********************************************************
void	CXMLParams::MakeParam(const CString& szTagName, long lValue)
{
	CString szBuffer;
	szBuffer.Format(_T("%d"), lValue);
	MakeParam(szTagName, szBuffer);
}

// *********************************************************
// * Function 	MakeParam
// *
// * Purpose 	Adds a node to m_pRoot and with the name
// *      		and value equal to the input parameters.
// *
// * Parameters	szTagName – Name of the node’s tag.
// *      		szValue – The node’s value (Overload #1)
// *      		uiValue – The node’s value (Overload #2)
// *      		iValue – The node’s value (Overload #3)
// *      		lValue – The node’s value (Overload #4)
// *      		dValue – The node’s value (Overload #5)
// *
// * Returns  	void
// *********************************************************
void	CXMLParams::MakeParam(const CString& szTagName, double dValue)
{
	CString szBuffer;
	szBuffer.Format(_T("%f"), dValue);
	MakeParam(szTagName, szBuffer);
}

// *********************************************************
// * Function 	UpdateParam
// *
// * Purpose 	Update a m_pRoot node with the name
// *      		and value equal to the input parameters.
// *
// * Parameters	szTagName – Name of the node’s tag.
// *      		szValue – The node’s value (Overload #1)
// *      		uiValue – The node’s value (Overload #2)
// *      		iValue – The node’s value (Overload #3)
// *      		lValue – The node’s value (Overload #4)
// *      		dValue – The node’s value (Overload #5)
// *
// * Returns  	void
// *********************************************************
void	CXMLParams::UpdateParam(const CString& szTagName, const CString& szValue)
{
	CCMSXMLDomNode::ComponentTypePtr pNode;
	try
	{
		pNode = m_pRoot->selectSingleNode(_bstr_t(szTagName));
		pNode->text = _bstr_t(szValue);
	}
	catch (...)
	{
		CXMLParams XMLParams;
		XMLParams.MakeParam("tagname", szTagName);
		XMLParams.MakeParam("nodename", m_szRootName);
		HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
		XMLParams.Dispose();
	}
	//	DestroyNode(pNode);
}

void	CXMLParams::UpdateParam(const CString& szTagName, UINT uiValue)
{
	UpdateParam(szTagName, (long)uiValue);
}

void	CXMLParams::UpdateParam(const CString& szTagName, int iValue)
{
	UpdateParam(szTagName, (long)iValue);
}

void	CXMLParams::UpdateParam(const CString& szTagName, long lValue)
{
	CString szBuffer;
	szBuffer.Format(_T("%d"), lValue);
	UpdateParam(szTagName, szBuffer);
}

void	CXMLParams::UpdateParam(const CString& szTagName, double dValue)
{
	CString szBuffer;
	szBuffer.Format(_T("%f"), dValue);
	UpdateParam(szTagName, szBuffer);
}

// *********************************************************
// * Function 	RemoveParam
// *
// * Purpose 	Removes a m_pRoot node with the name
// *      		equal to the input parameter.
// *
// * Parameters	szTagName – Name of the node’s tag.
// *
// * Returns  	void
// *********************************************************
void	CXMLParams::RemoveParam(const CString& szTagName)
{
	CCMSXMLDomNode::ComponentTypePtr pNode;
	try
	{
		pNode = m_pRoot->selectSingleNode(_bstr_t(szTagName));
		m_pRoot->removeChild(pNode);
		m_lParamCount--;
	}
	catch (...)
	{
		CXMLParams XMLParams;
		XMLParams.MakeParam("tagname", szTagName);
		XMLParams.MakeParam("nodename", m_szRootName);
		HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
		XMLParams.Dispose();
	}
	//	DestroyNode(pNode);
}

/*********************************************************
Function       GetParamTag
Type           CString
Purpose        Returns the tag name for the index of the node.
Parameters     lIndex - Node index..
Returns        CString
Author         Peter Ringering
Date           01/12/2003
***********************************************************/
CString CXMLParams::GetParamTag(long lIndex) const
{
	try
	{
		if (lIndex > (m_lParamCount - 1)) return "";
		CCMSXMLDomNode::ComponentTypePtr pNode = m_pRoot->childNodes->Getitem(lIndex);
		CString szReturn = LPCTSTR(pNode->nodeName);
		//		DestroyNode(pNode);
		return szReturn;
	}
	catch (_com_error e)
	{
		stuErrorReturn clsErrorReturn;
		CGBLError clsError(clsErrorReturn);
		clsError.HandleException(&e);
		return "";
	}
}

/*********************************************************
Function       HasChildren
Type           bool
Purpose        Returns if the passed in node tag has any child nodes.
Parameters     szTagName – Name of the node’s tag.
Returns        bool
Author         Peter Ringering
Date           01/12/2003
***********************************************************/
bool	CXMLParams::HasChildren(const CString& szTagName) const
{
	CCMSXMLDomNode::ComponentTypePtr pNode;
	bool bReturn = false;
	try
	{
		pNode = m_pRoot->selectSingleNode(_bstr_t(szTagName));
		bReturn = (pNode->childNodes->length > 1);
		//		DestroyNode(pNode);
		return bReturn;
	}
	catch (...)
	{
		CXMLParams XMLParams;
		XMLParams.MakeParam("tagname", szTagName);
		XMLParams.MakeParam("nodename", m_szRootName);
		HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
		//		DestroyNode(pNode);
		XMLParams.Dispose();
		return false;
	}
}

/*********************************************************
Function	GetParamChar
Type		CString
Purpose		Returns a Char variable containing the data
located for the node containing the inputted
tag name.  If the node is invalid and
bValidate = true, then a Denali pop-up error
dialog will show and the return value will be
empty.
Parameters  szTagName – Name of the node’s tag.
bValidate (Optional--default=true) - Do we
want a validation message to show if the
node is invalid?
Returns		char
Author		David Parvin
Date		04/03/2003
***********************************************************/
byte	CXMLParams::GetParamByte(const CString& szTagName, bool bValidate) const
{
	try
	{
		return (byte)GetParamInt(szTagName, bValidate);
	}
	catch (...)
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("tagname", szTagName);
			XMLParams.MakeParam("nodename", m_szRootName);
			HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
			XMLParams.Dispose();
		}
		return (char)0;
	}
}

/*********************************************************
Function       GetParamStr
Type           CString
Purpose        Returns a CString variable containing the data
located for the node containing the inputted tag name.
If the node is invalid and bValidate = true, then a
Denali pop-up error dialog will show and the return
value will be empty.
Parameters     szTagName – Name of the node’s tag.
bValidate (Optional--default=true) - Do we want a validation message to show if the node is invalid?
Returns        CString
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
CString CXMLParams::GetParamStr(const CString& szTagName, bool bValidate) const
{
	try
	{
		// JEPK 12/16/2014 PBI 25674:ALL_Review first chance exceptions. Prevent Null value exception.
		CCMSXMLDomNode::ComponentTypePtr _node = m_pRoot->selectSingleNode(_bstr_t(szTagName));
		_bstr_t bstReturn;
		if (_node == NULL)
			bstReturn = "";
		else
			bstReturn = _node->Gettext();
		CString szReturn(bstReturn.GetBSTR());
		return szReturn;
	}
	catch (...)
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("tagname", szTagName);
			XMLParams.MakeParam("nodename", m_szRootName);
			HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
			XMLParams.Dispose();
		}
		return "";
	}
}

/*********************************************************
Function	GetParamChar
Type		CString
Purpose		Returns a Char variable containing the data
located for the node containing the inputted
tag name.  If the node is invalid and
bValidate = true, then a Denali pop-up error
dialog will show and the return value will be
empty.
Parameters  szTagName – Name of the node’s tag.
bValidate (Optional--default=true) - Do we
want a validation message to show if the
node is invalid?
Returns		char
Author		David Parvin
Date		04/03/2003
***********************************************************/
char	CXMLParams::GetParamChar(const CString& szTagName, bool bValidate) const
{
	try
	{
		_bstr_t bstReturn = m_pRoot->selectSingleNode(_bstr_t(szTagName))->Gettext();
		const CString szData(bstReturn.GetBSTR());
		char szReturn = szData.GetLength() > 0 ? (char)szData.GetAt(0) : (char)NULL;
		return szReturn;
	}
	catch (...)
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("tagname", szTagName);
			XMLParams.MakeParam("nodename", m_szRootName);
			HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
			XMLParams.Dispose();
		}
		return (char)0;
	}
}

/*********************************************************
Function	GetParamUChar
Type		CString
Purpose		Returns an Unsigned Char variable containing
the data located for the node containing the
inputted tag name.  If the node is invalid and
bValidate = true, then a Denali pop-up error
dialog will show and the return value will be
empty.
Parameters  szTagName – Name of the node’s tag.
bValidate (Optional--default=true) - Do we
want a validation message to show if the
node is invalid?
Returns		char
Author		David Parvin
Date		04/03/2003
***********************************************************/
unsigned char CXMLParams::GetParamUChar(const CString& szTagName, bool bValidate) const
{
	try
	{
		_bstr_t bstReturn = m_pRoot->selectSingleNode(_bstr_t(szTagName))->Gettext();
		const CString szData(bstReturn.GetBSTR());
		unsigned char szReturn = (unsigned char)(szData.GetLength() > 0 ? szData.GetAt(0) : NULL);
		return szReturn;
	}
	catch (...)
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("tagname", szTagName);
			XMLParams.MakeParam("nodename", m_szRootName);
			HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
			XMLParams.Dispose();
		}
		return (unsigned char)0;
	}
}

/*********************************************************
Function	GetParamUInt
Type		unsigned int
Purpose		Returns a unsigned int variable containing
the data located for the node containing the
inputted tag name.  If the node is invalid
and bValidate = true, then a Denali pop-up
error dialog will show and the return
value will be empty.
Parameters	szTagName – Name of the node’s tag.
bValidate (Optional--default=true) - Do we
want a validation message to show if the
node is invalid?
Returns		unsigned int
Author		David Parvin
Date		04/03/2003
***********************************************************/
int		CXMLParams::GetParamInt(const CString& szTagName, bool bValidate) const
{
	try
	{
		return _tstoi(GetParamStr(szTagName, bValidate));
	}
	catch (...)
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("tagname", szTagName);
			XMLParams.MakeParam("nodename", m_szRootName);
			HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
			XMLParams.Dispose();
		}
		return 0;
	}
}

/*********************************************************
Function	GetParamUInt
Type		unsigned int
Purpose		Returns a unsigned int variable containing
the data located for the node containing the
inputted tag name.  If the node is invalid
and bValidate = true, then a Denali pop-up
error dialog will show and the return
value will be empty.
Parameters	szTagName – Name of the node’s tag.
bValidate (Optional--default=true) - Do we
want a validation message to show if the
node is invalid?
Returns		unsigned int
Author		David Parvin
Date		04/03/2003
***********************************************************/
unsigned int CXMLParams::GetParamUInt(const CString& szTagName, bool bValidate) const
{
	try
	{
		return (unsigned int)_tstoi(GetParamStr(szTagName, bValidate));
	}
	catch (...)
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("tagname", szTagName);
			XMLParams.MakeParam("nodename", m_szRootName);
			HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
			XMLParams.Dispose();
		}
		return 0;
	}
}

/*********************************************************
Function       GetParam__int8
Type           __int8
Purpose        Returns a __int8 variable containing the data
located for the node containing the inputted tag name.
If the node is invalid and bValidate = true, then a
Denali pop-up error dialog will show and the return
value will be empty.
Parameters     szTagName – Name of the node’s tag.
bValidate (Optional--default=true) - Do we want a validation message to show if the node is invalid?
Returns        __int8
Author         Kenneth P. Moss
Date           12/01/2002
***********************************************************/
__int8	CXMLParams::GetParam__int8(const CString& szTagName, bool bValidate) const
{
	try
	{
		return __int8(_tstoi(GetParamStr(szTagName, bValidate)));
	}
	catch (...)
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("tagname", szTagName);
			XMLParams.MakeParam("nodename", m_szRootName);
			HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
			XMLParams.Dispose();
		}
		return 0;
	}
}

/*********************************************************
Function       GetParam__int16
Type           __int16
Purpose        Returns a __int16 variable containing the data
located for the node containing the inputted tag name.
If the node is invalid and bValidate = true, then a
Denali pop-up error dialog will show and the return
value will be empty.
Parameters     szTagName – Name of the node’s tag.
bValidate (Optional--default=true) - Do we want a validation message to show if the node is invalid?
Returns        __int16
Author         Kenneth P. Moss
Date           12/01/2002
***********************************************************/
__int16 CXMLParams::GetParam__int16(const CString& szTagName, bool bValidate) const
{
	try
	{
		return __int16(_tstoi(GetParamStr(szTagName, bValidate)));
	}
	catch (...)
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("tagname", szTagName);
			XMLParams.MakeParam("nodename", m_szRootName);
			HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
			XMLParams.Dispose();
		}
		return 0;
	}
}

/*********************************************************
Function       GetParam__int32
Type           __int32
Purpose        Returns a __int32 variable containing the data
located for the node containing the inputted tag name.
If the node is invalid and bValidate = true, then a
Denali pop-up error dialog will show and the return
value will be empty.
Parameters     szTagName – Name of the node’s tag.
bValidate (Optional--default=true) - Do we want a validation message to show if the node is invalid?
Returns        __int32
Author         Kenneth P. Moss
Date           12/01/2002
***********************************************************/
__int32 CXMLParams::GetParam__int32(const CString& szTagName, bool bValidate) const
{
	try
	{
		return __int32(_tstoi(GetParamStr(szTagName, bValidate)));
	}
	catch (...)
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("tagname", szTagName);
			XMLParams.MakeParam("nodename", m_szRootName);
			HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
			XMLParams.Dispose();
		}
		return 0;
	}
}

/*********************************************************
Function	GetParamFloat
Type		Float
Purpose		Returns a long variable containing the data
located for the node containing the inputted
tag name.  If the node is invalid and
bValidate = true, then a Denali pop-up error
dialog will show and the return value will
be empty.
Parameters	szTagName – Name of the node’s tag.
bValidate (Optional--default=true) - Do we
want a validation message to show if the
node is invalid?
Returns		float
Author		David Parvin
Date		04/03/2003
***********************************************************/
float	CXMLParams::GetParamFloat(const CString& szTagName, bool bValidate) const
{
	try
	{
		return (float)_tstof(GetParamStr(szTagName, bValidate));
	}
	catch (...)
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("tagname", szTagName);
			XMLParams.MakeParam("nodename", m_szRootName);
			HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
			XMLParams.Dispose();
		}
		return (float)0;
	}
}

/*********************************************************
Function	GetParamDouble
Type		double
Purpose		Returns a long variable containing the data
located for the node containing the inputted
tag name.  If the node is invalid and
bValidate = true, then a Denali pop-up error
dialog will show and the return value will
be empty.
Parameters	szTagName – Name of the node’s tag.
bValidate (Optional--default=true) - Do we
want a validation message to show if the
node is invalid?
Returns		double
Author		David Parvin
Date		04/03/2003
***********************************************************/
double	CXMLParams::GetParamDouble(const CString& szTagName, bool bValidate) const
{
	try
	{
		return _tstof(GetParamStr(szTagName, bValidate));
	}
	catch (...)
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("tagname", szTagName);
			XMLParams.MakeParam("nodename", m_szRootName);
			HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
			XMLParams.Dispose();
		}
		return 0;
	}
}

short CXMLParams::GetParamShort(const CString& szTagName, bool bValidate) const
{
	try
	{
		return static_cast<short>(_ttoi(GetParamStr(szTagName, bValidate)));
	}
	catch (...)
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("tagname", szTagName);
			XMLParams.MakeParam("nodename", m_szRootName);
			HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
			XMLParams.Dispose();
		}
		return 0;
	}
}

/// <summary>
/// Gets the parameter unsigned short.
/// </summary>
/// <param name="szTagName">Name of the tag.</param>
/// <param name="bValidate">if set to <c>true</c> validate.</param>
/// <returns></returns>
unsigned short CXMLParams::GetParamUShort(const CString& szTagName, bool bValidate) const
{
	try
	{
		unsigned long temp = std::stoul(std::wstring(GetParamStr(szTagName, bValidate)));
		if (temp <= USHRT_MAX)
			return static_cast<unsigned short>(temp);
	}
	catch (...)
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("tagname", szTagName);
			XMLParams.MakeParam("nodename", m_szRootName);
			HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
			XMLParams.Dispose();
		}
	}
	return 0;
}

/*********************************************************
Function       GetParamBool
Type           bool
Purpose        Returns a bool variable containing the data
located for the node containing the inputted tag name.
If the node is invalid and bValidate = true, then a
Denali pop-up error dialog will show and the return
value will be empty.
Parameters     szTagName – Name of the node’s tag.
bValidate (Optional--default=true) - Do we want a validation message to show if the node is invalid?
Returns        bool
Author         Kenneth P. Moss
Date           12/01/2002
***********************************************************/
bool	CXMLParams::GetParamBool(const CString& szTagName, bool bValidate) const
{
	try
	{
		CString szReturn = GetParamStr(szTagName, bValidate);
		if (szReturn == "true") return true;
		if (szReturn == "false") return false;
		return !((szReturn == "0") || (szReturn == ""));
	}
	catch (...)
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("tagname", szTagName);
			XMLParams.MakeParam("nodename", m_szRootName);
			HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
			XMLParams.Dispose();
		}
		return 0;
	}
}

/*********************************************************
Function       GetParamLong
Type           long
Purpose        Returns a long variable containing the data
located for the node containing the inputted tag name.
If the node is invalid and bValidate = true, then a
Denali pop-up error dialog will show and the return
value will be empty.
Parameters     szTagName – Name of the node’s tag.
bValidate (Optional--default=true) - Do we want a validation message to show if the node is invalid?
Returns        long
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
long	CXMLParams::GetParamLong(const CString& szTagName, bool bValidate) const
{
	try
	{
		return _tstol(GetParamStr(szTagName, bValidate));
	}
	catch (...)
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("tagname", szTagName);
			XMLParams.MakeParam("nodename", m_szRootName);
			HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
			XMLParams.Dispose();
		}
		return 0;
	}
}

/*********************************************************
Function       GetParamULong
Type           long
Purpose        Returns a long variable containing the data
located for the node containing the inputted tag name.
If the node is invalid and bValidate = true, then a
Denali pop-up error dialog will show and the return
value will be empty.
Parameters     szTagName – Name of the node’s tag.
bValidate (Optional--default=true) - Do we want a validation message to show if the node is invalid?
Returns        long
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
unsigned long CXMLParams::GetParamULong(const CString& szTagName, bool bValidate) const
{
	try
	{
		return (unsigned long)_tstol(GetParamStr(szTagName, bValidate));
	}
	catch (...)
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("tagname", szTagName);
			XMLParams.MakeParam("nodename", m_szRootName);
			HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
			XMLParams.Dispose();
		}
		return 0;
	}
}

/*********************************************************
Function       GetParamDate
Type           COleDateTime
Purpose        Returns a COleDateTime variable containing
the data located for the node containing
the inputted tag name.  If the node is
invalid and bValidate = true, then a Denali
pop-up error dialog will show and the return
value will be empty.
Parameters     szTagName – Name of the node’s tag.
bValidate (Optional--default=true) - Do we want a validation message to show if the node is invalid?
Returns        COleDateTime
Author         David W. Parvin
Date           01/29/2003
***********************************************************/
COleDateTime CXMLParams::GetParamDate(const CString& szTagName, bool bValidate) const
{
	COleDateTime dteDate;
	try
	{
		CString szValue = GetParamStr(szTagName, bValidate);
		dteDate.ParseDateTime(szValue.Left(10) + " " + szValue.Mid(11, 8));
		return dteDate;
	}
	catch (...)
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("tagname", szTagName);
			XMLParams.MakeParam("nodename", m_szRootName);
			HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
			XMLParams.Dispose();
		}
		dteDate.ParseDateTime(_T(""));
		return dteDate;
	}
}

/*********************************************************
Function       AppendXMLParams
Type           void
Purpose        Adds the pParam’s m_pRoot document element node to this object’s m_pRoot.
Parameters     pParam – A pointer to another CXMLParams object.
Returns        void
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
void	CXMLParams::AppendXMLParam(CXMLParams* pParam) const
{
	//	CXMLParams TempParam(true);
	//	TempParam.SetXML(pParam->GetXML());
	//	m_pRoot->appendChild(TempParam.m_pRoot);
	m_pRoot->appendChild(pParam->m_pRoot);
}

CXMLParams& CXMLParams::operator+=(CXMLParams& pRightParam)
{
	this->AppendXMLParam(&pRightParam);
	return *this;
}

/*********************************************************
Function       GetXMLParams
Type           CXMLParams
Purpose        Returns a CXMLParams object whose m_pRoot contains the
node with the given tag name.  If the node is invalid,
then a Denali pop-up error dialog will show and the
return object will be “empty”.
Parameters     szTagName – Name of the node’s tag.
Returns        CXMLParams
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
CXMLParams CXMLParams::GetXMLParams(const CString& szTagName) const
{
	CXMLParams XMLParam(true);
	CCMSXMLDomNode::ComponentTypePtr pNode;
	try
	{
		pNode = m_pRoot->selectSingleNode(_bstr_t(szTagName));
		XMLParam.SetRootNode(pNode);
	}
	catch (...)
	{
		CXMLParams XMLParams;
		XMLParams.MakeParam("tagname", szTagName);
		XMLParams.MakeParam("nodename", m_szRootName);
		HandleError(ERR_GB_CXMLPARAMS_BADTAG, NULL, &XMLParams);
		XMLParams.Dispose();
	}
	//	DestroyNode(pNode);
	return XMLParam;
}

/*********************************************************
Function       StringMerge
Type           CString
Purpose        Loops through all the nodes in the current object.
For each node, it replaces all occurrences of the
tag in the inputted string equal to the node’s tag
name with the text that’s in the node.
Parameters     szResourceStr – The resource string containing XML tags to be filled in.
Returns        CString
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
CString CXMLParams::StringMerge(CString szResourceStr) const
{
	if (m_pRoot == NULL) return szResourceStr;
	CCMSXMLDomNodeList::ComponentTypePtr pNodeList;
	try
	{
		m_pRoot->get_childNodes(&pNodeList);
		for (long lCounter = 0; lCounter < pNodeList->Getlength(); lCounter++)
		{
			CCMSXMLDomNode::ComponentTypePtr pNode;
			pNodeList->get_item(lCounter, &pNode);
			CString szParam = LPCTSTR(pNode->GetnodeName());
			szParam.MakeUpper();
			szParam = "<" + szParam + ">";
			CString szValue = LPCTSTR(pNode->Gettext());
			szResourceStr.Replace(szParam, szValue);
			//			DestroyNode(pNode);
		}
	}
	catch (CException* pe)
	{
		HandleError(0, pe);
	}
	//	DestroyNodeList(pNodeList);
	return szResourceStr;
}

/*********************************************************
Function       Clear
Type           void
Purpose        Removes all nodes from m_pRoot.
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
void	CXMLParams::Clear() const
{
	/*Peter Ringering - 02/11/2003 - Need to check if root is null first*/
	if (m_pRoot == NULL) return;
	CCMSXMLDomNodeList::ComponentTypePtr pNodeList;
	try
	{
		this->m_pRoot->get_childNodes(&pNodeList);
	}
	catch (...)
	{
		return;
	}
	long lLength = pNodeList->Getlength();
	for (long lCounter = 0; lCounter < lLength; lCounter++)
	{
		CCMSXMLDomNode::ComponentTypePtr pNode;
		pNodeList->get_item(0, &pNode);
		m_pRoot->removeChild(pNode);
		//		DestroyNode(pNode);
	}
	//	DestroyNodeList(pNodeList);
}

/*********************************************************
Function       Dispose
Type           void
Purpose        Releases and sets to NULL the XML Document.
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
void	CXMLParams::Dispose()
{
	//	DestroyNode(m_pRoot);
	if (m_pDOMDocument != NULL)
	{
		m_pDOMDocument.Release();
		m_pDOMDocument = NULL;
	}
	m_szRootName.Empty();
}

/*********************************************************
Function       IsEmpty
Type           bool
Purpose        Checks to see if the XML Document is NULL.
Parameters     None
Returns        bool
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
bool	CXMLParams::IsEmpty() const
{
	if (m_pRoot == NULL || m_pDOMDocument == NULL) return true;
	return false;
}

/*********************************************************
Function       TestParams
Type           void
Purpose        A developer-only method.  It’s used to test all the
methods in the CXMLParams object.
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
void	CXMLParams::TestParams()
{
	CXMLParams XMLTest(true);
	XMLTest.SetRoot("TESTROOT");
	CXMLParams XMLParam;
	XMLParam.MakeParam("XML1", "VAL1");
	XMLParam.MakeParam("XML2", "VAL2");
	XMLTest.AppendXMLParam(&XMLParam);
	XMLTest.MakeParam("TEST1", "1VALUE");
	XMLTest.MakeParam("TEST2", "2VALUE");
	AfxMessageBox(XMLTest.GetXML());
	XMLTest.Clear();
	XMLTest.MakeParam("TEST3", "3VALUE");
	AfxMessageBox(XMLTest.GetXML());
}

/*********************************************************
Function       ParamExists
Type           bool
Purpose        Checks to see if the node exists in m_pDOMDocument
Parameters     szTagName - Name of the node
Returns        bool
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
bool	CXMLParams::ParamExists(const CString& szTagName) const
{
	if (IsEmpty()) return false;
	CCMSXMLDomNode::ComponentTypePtr pNode;
	try
	{
		pNode = m_pRoot->selectSingleNode(_bstr_t(szTagName));
	}
	catch (...)
	{
		return false;
	}
	if (pNode == NULL) return false;
	return true;
}

/*********************************************************
Function       GetParamCount
Type           long
Purpose        Returns the number of nodes inside the parent node.
Parameters     None
Returns        long
Author         Peter Ringering
Date           01/11/2003
***********************************************************/
long	CXMLParams::GetParamCount() const
{
	if (m_pDOMDocument == NULL || m_pRoot == NULL) return 0;
	return m_pRoot->childNodes->Getlength();
}

/*********************************************************
Function       GetTableCount
Type           int
Purpose        Returns the number of CDataTable objects inside the pDS object.
Parameters     pDS - Pointer to a DataSet object.
Returns        int
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
int		GetTableCount(CDataSet* pDS)
{
	if (pDS->IsEmpty()) return 0;
	long lTableCount = 0;
	long lLength = 0;
	VARIANT_BOOL bHasKids;
	HRESULT hr;
	CCMSXMLDomElement::ComponentTypePtr       pXMLElement;
	CCMSXMLDomNodeList::ComponentTypePtr      pXMLNodeList;
	CCMSXMLDomNodeList::ComponentTypePtr      pRowList;
	CCMSXMLDomNode::ComponentTypePtr			pXMLNode;

	//Is pDS not empty?
	hr = pDS->m_pDOMDocument->get_documentElement(&pXMLElement);
	if (FAILED(hr)) return 0;
	bHasKids = pXMLElement->hasChildNodes();
	if (!bHasKids) return 0;

	//Are there any nodes inside pDS?
	hr = pXMLElement->get_childNodes(&pXMLNodeList);
	if (FAILED(hr)) return 0;
	//	DestroyElement(pXMLElement);

		//Get the number of nodes inside pDS.
	hr = pXMLNodeList->get_length(&lLength);
	if (FAILED(hr)) return 0;
	if (lLength == 0) return 0;

	//We're Married with Children.  Let's see how many we have.
	CString szTableName = "";
	for (int nCounter = 0; nCounter < lLength; nCounter++)
	{
		//Increment Table Count.
		lTableCount++;
		hr = pXMLNodeList->get_item(nCounter, &pXMLNode);
		if (FAILED(hr)) return 0;
		BSTR bstCurNodeName;
		pXMLNode->get_nodeName(&bstCurNodeName);

		if (FAILED(hr)) return 0;
		else if (_bstr_t(bstCurNodeName) == _bstr_t(_T("xs:schema"))) continue;

		CString szTableName = bstCurNodeName;
		//Load up a data table with the name of the node.
		CDataTable DT;
		LoadDataTable(pDS, &DT, szTableName, true);
		//Increment "for" counter by the number of rows inside the data table.
		if (DT.m_lRowCount > 0) nCounter += (DT.m_lRowCount - 1);
		//		DestroyNode(pXMLNode);
		DT.Dispose();
	}
	//	DestroyNodeList(pXMLNodeList);
	return lTableCount;
}

/*********************************************************
Function       GetRowCount
Type           long
Purpose        Returns the number of CDataRow objects inside the pDT object.
Parameters     pDT - Pointer to a DataTable object.
Returns        long
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
long	GetRowCount(CDataTable* pDT)
{
	ASSERT(pDT);
	long lRowCount;
	HRESULT hr;
	hr = pDT->m_pNodeList->get_length(&lRowCount);
	return lRowCount;
}

/*********************************************************
Function       GetColumnCount
Type           int
Purpose        Returns the number of columns inside the pDT object.
Parameters     pDT - Pointer to a DataTable object.
Returns        long
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
int		GetColumnCount(CDataTable* pDT, long lRowCount)
{
	ASSERT(pDT);
	if (lRowCount == 0) return 0;
	int nColCount;
	CDataRow DR;
	pDT->GetRow(0, &DR);
	nColCount = DR.m_nColumnCount;
	DR.Dispose();

	return nColCount;
}

/*********************************************************
Function       ValidateRow
Type           bool
Purpose        Checks to see if the row has columns.  Displays message to user if not.
Parameters     pDR - Pointer to a CDataRow object.
bUseIndex - true means to use the nColIndex parameter,
false means to use the szColName parameter.
nColIndex - Column Index
szColName - Column Name
Returns        bool
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
bool	ValidateRow(CDataRow* pDR, bool bUseIndex, int nColIndex, const CString& szColName)
{
	ASSERT(pDR);
	if (pDR->m_nColumnCount == 0)
	{
		CXMLParams XMLParams;
		XMLParams.MakeParam("table", pDR->m_szDataTableName);
		CString szMsg;
		if (bUseIndex)
			XMLParams.MakeParam("column", nColIndex);
		else
			XMLParams.MakeParam("column", szColName);
		HandleError(ERR_GB_DATASET_INVALIDCOL0COL, NULL, &XMLParams, &pDR->GetDataTable().GetDataSet());
		XMLParams.Dispose();
		return false;
	}
	return true;
}

// ********************************************************
// * Procedure:    AddColumn
// *
// * Description:  Adds a column to the CDataRow.
// *
// * Parameters:   szColumnName - New column name to add.
// *               nIndex - Index to position the column at.  -1 means at the end.
// *
// * Returns:      int - Index of the column added.  -1 if the column already exists.
// *
// * Date Created: 03/12/2003 by Peter Ringering
// ********************************************************
int		AddColumn(CDataRow* pDR, const CString& szColumnName, int nIndex)
{
	ASSERT(pDR);
	if (pDR->ColumnExists(szColumnName)) return -1;
	AddNodeVal(pDR->m_pNode->ownerDocument, szColumnName, "", pDR->m_pNode, true);
	pDR->m_nColumnCount++;

	return pDR->m_nColumnCount - 1;
}

// ********************************************************
// * Procedure:    RemoveColumn
// *
// * Description:  Removes a column from the CDataRow.
// *
// * Parameters:   szColumnName - column name to remove.
// *
// * Date Created: 03/12/2003 by Peter Ringering
// ********************************************************
void	RemoveColumn(CDataRow* pDR, const CString& szColumnName)
{
	ASSERT(pDR);
	if (!pDR->ColumnExists(szColumnName)) return;
	CDataCell clsDC;
	pDR->GetDataCell(szColumnName, &clsDC);
	if (clsDC.IsEmpty()) return;
	pDR->m_pNode->removeChild(clsDC.m_pNode);
	pDR->m_nColumnCount--;
}

// ********************************************************
// * Procedure:    RemoveColumn
// *
// * Description:  Removes a column from the CDataRow.
// *
// * Parameters:   nColumnIndex - column index to remove.
// *
// * Date Created: 03/12/2003 by Peter Ringering
// ********************************************************
void	RemoveColumn(CDataRow* pDR, int nColumnIndex)
{
	ASSERT(pDR);
	if (nColumnIndex > pDR->m_nColumnCount - 1) return;
	CDataCell clsDC;
	pDR->GetDataCell(nColumnIndex, &clsDC);
	if (clsDC.IsEmpty()) return;
	pDR->m_pNode->removeChild(clsDC.m_pNode);
	pDR->m_nColumnCount--;
}

// ********************************************************
// * Procedure:    MakeXMLDocument
// *
// * Description:  Creates a document with a root node name of szNodeName
// *
// * Parameters:   szRootName - Name to set the root to.
// *
// * Date Created: 03/12/2003 by Peter Ringering
// ********************************************************
CCMSXMLDomDoc::ComponentTypePtr MakeXMLDocument(const CString& szRootName)
{
	CCMSXMLDomDoc::ComponentTypePtr pDocument;
	try
	{
		CCMSXMLDomDoc::GetPolicy().CreateObject(pDocument, CCMSXMLDomDoc::GetProgID());
		//pDocument.CreateInstance("msxml2.domdocument");
		CCMSXMLDomNode::ComponentTypePtr pNode = pDocument->createNode("element", _bstr_t(szRootName), "");
		pDocument->appendChild(pNode);
	}
	catch (CException* pe)
	{
		HandleError(0, pe);
	}
	return pDocument;
}

// ********************************************************
// * Procedure:    AddNodeVal
// *
// * Description:  Creates a node with a node name of szNodeName, Sets it value
// * to szNodeValue, and appends it to pParentNode.
// *
// * Parameters:   pDocument - XML Document to create the node.
// *               szNodeName - Name of the new node.
// *               szNodeValue - Value of the new node.
// *               pParentNode - Parent Node to append to.
// *               bUpdate - Just update the node or add a new one?
// *
// * Date Created: 03/12/2003 by Peter Ringering
// ********************************************************
void AddNodeVal(
	CCMSXMLDomDoc::ComponentTypePtr pDocument,
	const CString& szNodeName,
	const CString& szNodeValue,
	CCMSXMLDomNode::ComponentTypePtr pParentNode,
	bool bUpdate)
{
	CCMSXMLDomNode::ComponentTypePtr pNode;
	if (pParentNode == NULL) pParentNode = pDocument->documentElement;
	try
	{
		_bstr_t bstNodeNameBSTR(szNodeName);
		pNode = pParentNode->selectSingleNode(bstNodeNameBSTR);
		if (pNode != NULL && bUpdate)
		{
			pNode->text = _bstr_t(szNodeValue);
			return;
		}
		pNode = pDocument->createNode("element", _bstr_t(szNodeName), "");
		pNode->text = _bstr_t(szNodeValue);
		pParentNode->appendChild(pNode);
	}
	catch (CException* pe)
	{
		HandleError(0, pe);
	}
	catch (_com_error e)
	{
		stuErrorReturn clsErrorReturn;
		CGBLError clsError(clsErrorReturn);
		clsError.HandleException(&e);
	}
	//	DestroyNode(pNode);
}

/*********************************************************
Function       LoadDataTable (Overload #1)
Type           void
Purpose        Loads up the pDT's XML node list from pDS.
Parameters     pDS - Pointer to a CDataSet object.
pDT - Pointer to a CDataTable object.
szTableName - Name of the table to get.
bValidate (Optional--default=true) - Do we want a validation message to show on error?
Returns        void
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
void	LoadDataTable(CDataSet* pDS, CDataTable* pDT, const CString& szTableName, bool bValidate)
{
	pDT->m_pDocElement = pDS->m_pDOMDocument->documentElement;
	pDT->m_pNodeList = pDT->m_pDocElement->selectNodes(_bstr_t(szTableName));
	LoadDataTable(pDT, szTableName, bValidate);
}

/*********************************************************
Function       LoadDataTable (Overload #2)
Type           void
Purpose		   Validates pDT to ensure it has data.  Set m_lRowCount and m_nColumnCount
properties.
Parameters     pDT - Pointer to a CDataTable object.
szTableName - Name of the table to get.
bValidate (Optional--default=true) - Do we want a validation message to show on error?
Returns        void
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
void	LoadDataTable(CDataTable* pDT, const CString& szTableName, bool bValidate)
{
	ASSERT(pDT);
	long lCount = 0;
	if (!pDT->IsEmpty()) pDT->m_pNodeList->get_length(&lCount);
	if (lCount == 0)
	{
		pDT->Dispose();
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("table", szTableName);
			HandleError(ERR_GB_DATASET_INVALIDTB0TB, NULL, &XMLParams);
		}
		return;
	}
	pDT->m_szTableName = szTableName;
	if (!pDT->IsEmpty())
	{
		pDT->m_lRowCount = GetRowCount(pDT);
		pDT->m_nColumnCount = GetColumnCount(pDT, pDT->m_lRowCount);
	}
}

/*********************************************************
Function       LoadDataRow (Overload #1)
Type           void
Purpose        Loads up the pDR's XML node from pDT.
Parameters     pDT - Pointer to a CDataTable object.
pDR - Pointer to a CDataRow object.
lRowIndex - Row number to get.
Returns        void
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
void	LoadDataRow(CDataTable* pDT, CDataRow* pDR, long lRowIndex)
{
	ASSERT(pDT);
	ASSERT(pDR);
	pDR->m_pNode = pDT->m_pNodeList->Getitem(lRowIndex);
	if (pDR->IsEmpty() || pDT->m_lRowCount == 0)
	{
		CXMLParams XMLParams;
		XMLParams.MakeParam("rowindex", lRowIndex);
		XMLParams.MakeParam("rowcount", pDT->m_lRowCount);
		XMLParams.MakeParam("maxindex", pDT->m_lRowCount - 1);
		XMLParams.MakeParam("table", pDT->m_szTableName);
		HandleError(ERR_GB_DATASET_INVALIDROWINDEX, NULL, &XMLParams, &pDT->GetDataSet());
		pDR->Dispose();
		return;
	}
	LoadDataRow(pDR);
	pDR->m_lRowIndex = lRowIndex;
}

/*********************************************************
Function       LoadDataTable (Overload #2)
Type           void
Purpose        Validates pDR to ensure it has data.  Set m_lRowIndex and m_nColumnCount
properties.
Parameters     pDR - Pointer to a CDataRow object.
Returns        void
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
void	LoadDataRow(CDataRow* pDR)
{
	ASSERT(pDR);
	if (!pDR->IsEmpty())
	{
		pDR->m_nColumnCount = pDR->m_pNode->childNodes->Getlength();
		if (pDR->m_nColumnCount == 1)
		{
			CCMSXMLDomNode::ComponentTypePtr pNode = pDR->m_pNode->childNodes->Getitem(0);
			CString szXML = (BSTR)pNode->xml;
			if (szXML == "") pDR->m_nColumnCount = 0;
		}
	}
}

/*********************************************************
Function       LoadDataCell (Overload #1)
Type           void
Purpose        Loads up the pDC's XML node from pDR.
Parameters     pDR - Pointer to a CDataRow object.
pDC - Pointer to a CDataCell object.
nColumnIndex - Column # to get.
bValidate (Optional--default=true) - Do we want a validation message to show on error?
Returns        void
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
void	LoadDataCell(CDataRow* pDR, CDataCell* pDC, int nColumnIndex, bool bValidate)
{
	ASSERT(pDR);
	ASSERT(pDC);
	CCMSXMLDomNodeList::ComponentTypePtr pNodeList;
	HRESULT hr = pDR->m_pNode->get_childNodes(&pNodeList);
	if (hr == S_OK) hr = pNodeList->get_item(nColumnIndex, &pDC->m_pNode);
	if (hr != S_OK)
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("columnindex", nColumnIndex);
			XMLParams.MakeParam("maxindex", pDR->m_nColumnCount - 1);
			XMLParams.MakeParam("table", pDR->m_szDataTableName);
			HandleError(ERR_GB_DATASET_INVALIDCOLINDEX, NULL, &XMLParams, &pDR->GetDataTable().GetDataSet());
			XMLParams.Dispose();
		}
		//		if (pNodeList != NULL) DestroyNodeList(pNodeList);
		pDC->Dispose();
		return;
	}
	//	DestroyNodeList(pNodeList);
	LoadDataCell(pDR, pDC);
}

/*********************************************************
Function       LoadDataCell (Overload #2)
Type           void
Purpose        Loads up the pDC's XML node from pDR.
Parameters     pDR - Pointer to a CDataRow object.
pDC - Pointer to a CDataCell object.
szColumnName - Name of the column to get.
bValidate (Optional--default=true) - Do we want a validation message to show on error?
Returns        void
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
void	LoadDataCell(CDataRow* pDR, CDataCell* pDC, const CString& szColumnName, bool bValidate)
{
	ASSERT(pDR);
	ASSERT(pDC);
	HRESULT hr = pDR->m_pNode->raw_selectSingleNode(_bstr_t(LPCTSTR(szColumnName)), &pDC->m_pNode);

	//Peter Ringering - 12/05/2004 - Check to see if the child node exists in the
	//schema if it can't be found in the current node.
	if (hr != S_OK && bValidate)
	{
		CCMSXMLDomDoc::ComponentTypePtr pDoc = pDR->m_pNode->GetownerDocument();
		if (CDataSet::SchemaFieldExists(pDoc, pDR->getDataTableName(), szColumnName))
			hr = S_OK;
	}

	if (hr == S_OK)
		LoadDataCell(pDR, pDC);
	else
	{
		if (bValidate)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("column", szColumnName);
			XMLParams.MakeParam("table", pDR->m_szDataTableName);
			HandleError(ERR_GB_DATASET_INVALIDCOLNAME, NULL, &XMLParams, &pDR->GetDataTable().GetDataSet());
			XMLParams.Dispose();
		}
		pDC->Dispose();
	}
}

/*********************************************************
Function       LoadDataCell (Overload #3)
Type           void
Purpose        Validates pDC to ensure it has data.
Parameters     pDR - Pointer to a CDataRow object.
pDC - Pointer to a CDataCell object.
Returns        void
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
void	LoadDataCell(CDataRow* pDR, CDataCell* pDC)
{
	ASSERT(pDR);
	ASSERT(pDC);
	if (!pDC->IsEmpty())
	{
		//PTR.08.10.2005.1-16244
		if (!pDC->m_bTrimValue)
			CXMLElement::PreserveSpaces(pDC->m_pNode, pDC->m_pNode->ownerDocument);

		BSTR bstdata;
		HRESULT hr = pDC->m_pNode->get_nodeName(&bstdata);
		if (hr != S_OK)
		{
			pDC->Dispose();
			return;
		}
		pDC->m_szColumnName = bstdata;

		hr = pDC->m_pNode->get_text(&bstdata);
		if (hr != S_OK)
		{
			pDC->Dispose();
			return;
		}
		pDC->m_szColumnText = bstdata;
		if (!pDR->IsEmpty())
		{
			pDC->m_lRowIndex = pDR->m_lRowIndex;
			pDC->m_szDataTableName = pDR->m_szDataTableName;
		}
	}
}

/*********************************************************
Function       GetDataInt
Type           int
Purpose        Formats the pDC's m_szColumnText value into a long variable.
If the value cannot be formatted, then a validation message is
displayed to the user.
Parameters     pDR - Pointer to a CDataRow object.
pDC - Pointer to a CDataCell object.
Returns        int
Author         David Parvin
Date           05/05/2004
***********************************************************/
int		GetDataInt(CDataRow* pDR, CDataCell* pDC)
{
	ASSERT(pDR);
	ASSERT(pDC);
	int nValue = 0;
	CString szValue = "";
	if (!pDC->IsEmpty()) szValue = pDC->m_szColumnText;
	if (szValue != "")
	{
		try
		{
			nValue = _tstoi(szValue);
			if (!CGBLForm::ValidNumber(CGBLForm::TYP_INT, szValue)) // begbert 02-15-2006 VS8: warning C4482
				throw(szValue);
		}
		catch (...)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("value", szValue);
			XMLParams.MakeParam("vartype", "long");
			XMLParams.MakeParam("table", pDR->m_szDataTableName);
			XMLParams.MakeParam("row", pDR->m_lRowIndex);
			XMLParams.MakeParam("column", pDC->m_szColumnName);
			HandleError(ERR_GB_DATASET_INVALIDVALUE, NULL, &XMLParams, &pDR->GetDataTable().GetDataSet());
			XMLParams.Dispose();
		}
	}
	return nValue;
}

/*********************************************************
Function       GetDataLong
Type           long
Purpose        Formats the pDC's m_szColumnText value into a long variable.
If the value cannot be formatted, then a validation message is
displayed to the user.
Parameters     pDR - Pointer to a CDataRow object.
pDC - Pointer to a CDataCell object.
Returns        long
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
long	GetDataLong(CDataRow* pDR, CDataCell* pDC)
{
	ASSERT(pDR);
	ASSERT(pDC);
	long lValue = 0;
	CString szValue = "";
	if (!pDC->IsEmpty()) szValue = pDC->m_szColumnText;
	if (szValue != "")
	{
		try
		{
			lValue = _tstol(szValue);
			if (!CGBLForm::ValidNumber(CGBLForm::TYP_LONG, szValue)) // begbert 02-15-2006 VS8: warning C4482
				throw(szValue);
		}
		catch (...)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("value", szValue);
			XMLParams.MakeParam("vartype", "long");
			XMLParams.MakeParam("table", pDR->m_szDataTableName);
			XMLParams.MakeParam("row", pDR->m_lRowIndex);
			XMLParams.MakeParam("column", pDC->m_szColumnName);
			HandleError(ERR_GB_DATASET_INVALIDVALUE, NULL, &XMLParams, &pDR->GetDataTable().GetDataSet());
			XMLParams.Dispose();
		}
	}
	return lValue;
}

/*********************************************************
Function       GetDatabool
Type           bool
Purpose        Formats the pDC's m_szColumnText value into a bool variable.
0=false or "false" = false, <>0=true, or "true" = true.
If the value cannot be formatted, then a validation message is
displayed to the user.
Parameters     pDR - Pointer to a CDataRow object.
pDC - Pointer to a CDataCell object.
Returns        bool
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
bool	GetDatabool(CDataRow* pDR, CDataCell* pDC)
{
	ASSERT(pDR);
	ASSERT(pDC);
	bool bValue = false;
	CString szValue = "";
	if (!pDC->IsEmpty()) szValue = pDC->m_szColumnText;
	if (szValue != "")
	{
		if (szValue.CompareNoCase(_T("true")) != 0)
		{
			if (szValue.CompareNoCase(_T("false")) != 0)
			{
				int nValue = _tstoi(szValue);
				switch (nValue)
				{
				case 0:
					break;
				case 1:
				case -1:
					bValue = true;
					break;
				default:
					CXMLParams XMLParams;
					XMLParams.MakeParam("value", szValue);
					XMLParams.MakeParam("vartype", "boolean");
					XMLParams.MakeParam("table", pDR->m_szDataTableName);
					XMLParams.MakeParam("row", pDR->m_lRowIndex);
					XMLParams.MakeParam("column", pDC->m_szColumnName);
					HandleError(ERR_GB_DATASET_INVALIDVALUE, NULL, &XMLParams, &pDR->GetDataTable().GetDataSet());
					XMLParams.Dispose();
				}
			}
		}
		else
		{
			bValue = true;
			return bValue;
		}
	}
	return bValue;
}

/*********************************************************
Function       GetDataDouble
Type           double
Purpose        Formats the pDC's m_szColumnText value into a double variable.
If the value cannot be formatted, then a validation message is
displayed to the user.
Parameters     pDR - Pointer to a CDataRow object.
pDC - Pointer to a CDataCell object.
Returns        double
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
double	GetDataDouble(CDataRow* pDR, CDataCell* pDC)
{
	ASSERT(pDR);
	ASSERT(pDC);
	double dValue = 0;
	CString szValue = "";
	if (!pDC->IsEmpty()) szValue = pDC->m_szColumnText;
	if (szValue != "")
	{
		try
		{
			dValue = _tstof(szValue);
			if (!CGBLForm::ValidNumber(CGBLForm::TYP_DOUBLE, szValue)) // begbert 02-15-2006 VS8: warning C4482
				throw(szValue);
		}
		catch (...)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("value", szValue);
			XMLParams.MakeParam("vartype", "double");
			XMLParams.MakeParam("table", pDR->m_szDataTableName);
			XMLParams.MakeParam("row", pDR->m_lRowIndex);
			XMLParams.MakeParam("column", pDC->m_szColumnName);
			HandleError(ERR_GB_DATASET_INVALIDVALUE, NULL, &XMLParams, &pDR->GetDataTable().GetDataSet());
			XMLParams.Dispose();
		}
	}
	return dValue;
}

/*********************************************************
Function       GetDataDate
Type           COleDateTime
Purpose        Formats the pDC's m_szColumnText value into a COleDateTime variable.
If the value cannot be formatted, then a validation message is
displayed to the user.
Parameters     pDR - Pointer to a CDataRow object.
pDC - Pointer to a CDataCell object.
Returns        COleDateTime
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
COleDateTime GetDataDate(CDataRow* pDR, CDataCell* pDC)
{
	ASSERT(pDR);
	ASSERT(pDC);
	COleDateTime dteDate;
	CString szValue = "";
	if (!pDC->IsEmpty()) szValue = pDC->m_szColumnText;
	if (szValue != "")
	{
		try
		{
			dteDate.ParseDateTime(szValue.Left(10) + " " + szValue.Mid(11, 8));
		}
		catch (...)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("value", szValue);
			XMLParams.MakeParam("vartype", "date");
			XMLParams.MakeParam("table", pDR->m_szDataTableName);
			XMLParams.MakeParam("row", pDR->m_lRowIndex);
			XMLParams.MakeParam("column", pDC->m_szColumnName);
			HandleError(ERR_GB_DATASET_INVALIDVALUE, NULL, &XMLParams, &pDR->GetDataTable().GetDataSet());
			XMLParams.Dispose();
		}
	}
	return dteDate;
}

/*********************************************************
Function       GetDataByte
Type           byte
Purpose        Formats the pDC's m_szColumnText value into a byte variable.
If the value cannot be formatted, then a validation message is
displayed to the user.
Parameters     pDR - Pointer to a CDataRow object.
pDC - Pointer to a CDataCell object.
Returns        byte
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
byte	GetDataByte(CDataRow* pDR, CDataCell* pDC)
{
	ASSERT(pDR);
	ASSERT(pDC);
	byte bytValue = 0;
	CString szValue = "";
	if (!pDC->IsEmpty()) szValue = pDC->m_szColumnText;
	if (szValue != "")
	{
		try
		{
			bytValue = (byte)_tstoi(szValue);
			if (!CGBLForm::ValidNumber(CGBLForm::TYP_BYTE, szValue)) // begbert 02-15-2006 VS8: warning C4482
				throw(szValue);
		}
		catch (...)
		{
			CXMLParams XMLParams;
			XMLParams.MakeParam("value", szValue);
			XMLParams.MakeParam("vartype", "byte");
			XMLParams.MakeParam("table", pDR->m_szDataTableName);
			XMLParams.MakeParam("row", pDR->m_lRowIndex);
			XMLParams.MakeParam("column", pDC->m_szColumnName);
			HandleError(ERR_GB_DATASET_INVALIDVALUE, NULL, &XMLParams, &pDR->GetDataTable().GetDataSet());
			XMLParams.Dispose();
		}
	}
	return bytValue;
}

/*********************************************************
Function       TestDataSet
Type           void
Purpose        A developer-only method.  It’s used to test all the methods in the
CDataSet, CDataTable, CDataRow, and CDataCell objects.
Parameters     None
Returns        void
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
void	CDataSet::TestDataSet()
{
	CString szXML = TestDS_GetXML();
	LoadFromXML(szXML);
	CDataTable DT;
	GetTable(0, &DT);
	CString szTableName = DT.m_szTableName;
	CString szTValue = DT.XMLStr(0, "COMPANY_CODE");

	CDataRow DR;
	DT.GetRow(0, &DR);
	CString szColumnName = DT.GetColumnName(2);
	CString szValue = DR.XMLStr("COMPANY_CODE");
	DR.Dispose();
	DT.Dispose();
	Dispose();
	CWnd MsgWnd;
	MsgWnd.MessageBox("Column = " + szColumnName + " Value = " + szValue);

	return;
}

/*********************************************************
Function       TestDS_GetXML
Type           CString
Purpose        Returns an XML string to TestDS which is used for testing purposes only.
Parameters     None
Returns        CString
Author         Peter Ringering
Date           11/01/2002
***********************************************************/
CString TestDS_GetXML()
{
	CString szXML;
	szXML = "<ROOT>";
	szXML += "  <CMSRUNIN>";
	szXML += "    <REPORT_TITLE>SQL-2-SQL Conversion Error Report</REPORT_TITLE>";
	szXML += "    <COMPANY_NAME>Peter Fund Conversion Test - 2</COMPANY_NAME>";
	szXML += "    <COMPANY_CODE>P03</COMPANY_CODE>";
	szXML += "    <DEVELOPER>true</DEVELOPER>";
	szXML += "    <ABORT>false</ABORT>";
	szXML += "    <LONG>10</LONG>";
	szXML += "    <DOUBLE>987.1234</DOUBLE>";
	szXML += "  </CMSRUNIN>";
	szXML += "  <WINCONVERT_ERROR_TABLE>";
	szXML += "    <PASS>false</PASS>";
	szXML += "    <TASK>Processing Module: General Ledger</TASK>";
	szXML += "    <DESCRIPTION>Error Creating Index …</DESCRIPTION>";
	szXML += "    <ERR_DESC>Cannot define PRIMARY KEY constraint ...</ERR_DESC>";
	szXML += "    <SQL_STATEMENT>ALTER TABLE GL_MasterTable...</SQL_STATEMENT>";
	szXML += "    <LONG>5</LONG>";
	szXML += "    <name>PRINGERING\\PETERSQL</name>";
	szXML += "  </WINCONVERT_ERROR_TABLE>";
	szXML += "  <WINCONVERT_ERROR_TABLE>";
	szXML += "    <PASS>true</PASS>";
	szXML += "    <TASK>Processing Module: General Ledger</TASK>";
	szXML += "    <DESCRIPTION>Errors were encountered while...</DESCRIPTION>";
	szXML += "    <ERR_DESC />";
	szXML += "    <SQL_STATEMENT />";
	szXML += "    <LONG>5</LONG>";
	szXML += "    <name>PRINGERING\\PETERSQL</name>";
	szXML += "  </WINCONVERT_ERROR_TABLE>";
	szXML += "  <WINCONVERT_ERROR_TABLE>";
	szXML += "    <PASS>false</PASS>";
	szXML += "    <TASK>Processing Module: General Ledger</TASK>";
	szXML += "    <DESCRIPTION>Errors were encountered while...</DESCRIPTION>";
	szXML += "    <ERR_DESC />";
	szXML += "    <SQL_STATEMENT />";
	szXML += "    <LONG>10</LONG>";
	szXML += "    <name>PRINGERING\\PETERSQL</name>";
	szXML += "  </WINCONVERT_ERROR_TABLE>";
	szXML += "  <ERROR_TABLE2>";
	szXML += "    <PASS>false</PASS>";
	szXML += "    <TASK>Processing Module: General Ledger</TASK>";
	szXML += "    <DESCRIPTION>Error Creating Index...</DESCRIPTION>";
	szXML += "    <ERR_DESC>Cannot define PRIMARY KEY constraint ...</ERR_DESC>";
	szXML += "    <SQL_STATEMENT>ALTER TABLE GL_MasterTable...</SQL_STATEMENT>";
	szXML += "  </ERROR_TABLE2>";
	szXML += "  <ERROR_TABLE2>";
	szXML += "    <PASS>false</PASS>";
	szXML += "    <TASK>Processing Module: General Ledger</TASK>";
	szXML += "    <DESCRIPTION>Errors were encountered while...</DESCRIPTION>";
	szXML += "    <ERR_DESC />";
	szXML += "    <SQL_STATEMENT />";
	szXML += "  </ERROR_TABLE2>";
	szXML += "</ROOT>";
	return szXML;
}

/*********************************************************
Function       MakeXPath
Type           CString
Purpose        Takes a SQL-style WHERE clause and formats it into XML XPath syntax.
Parameters     szWHERE - Where clause
szTable - Table name.
Returns        CString
Author         Peter Ringering
Date           11/15/2002
***********************************************************/
CString MakeXPath(const CString& szWHERE, const CString& szTable)
{
	CString szReturn = szWHERE;
	szReturn.Replace(_T("<>"), _T("!="));
	szReturn = szTable + "[" + szReturn + "]";
	return szReturn;
}

/*********************************************************
Function       MakeXMLCommand
Type           void
Purpose        Fills the pXMLCommand object with command values used in creating the
"COMMAND" XML string to be sent to the server component.
Parameters     pXMLCommand - CXMLParams object to be filled up.
szCommand - Command name.
szModule - 2-Character module code.
szObject - Object name.
Returns        void
Author         Kenneth P. Moss
Date           12/17/2002
***********************************************************/
void	CXMLParams::MakeXMLCommand(CXMLParams* pXMLCommand, const CString& szCommand, const CString& szModule, const CString& szObject)
{
	ASSERT(pXMLCommand);
	pXMLCommand->MakeParam("COMMAND", szCommand);
	pXMLCommand->MakeParam("MODULE", szModule);
	pXMLCommand->MakeParam("OBJECT", szObject);
}

void	CXMLParams::MakeXMLCommand(CXMLElement* pXMLCommand, const CString& szCommand, const CString& szModule, const CString& szObject)
{
	ASSERT(pXMLCommand);
	pXMLCommand->AddNewChild(_T("COMMAND"), szCommand);
	pXMLCommand->AddNewChild(_T("MODULE"), szModule);
	pXMLCommand->AddNewChild(_T("OBJECT"), szObject);
}

/*********************************************************
Function       MakeXMLCommandData
Type           void
Purpose        Fills the pXMLCommand object with command values used in creating the
"COMMAND" XML string to be sent to the server component.
Then it appends connection and company info information to the passed in XML data object.
Parameters     pXMLCommand - Command CXMLParams object to be filled up.
pXMLData - Data CXMLParams object to be initialized.
szCommand - Command name.
szModule - 2-Character module code.
szObject - Object name.
bGetSchema - Get Schema (false);
Returns        void
Author         Peter Ringering
Date           09/10/2003
***********************************************************/
void	CXMLParams::MakeXMLCommandData(
	CXMLParams* pXMLCommand,
	CXMLParams* pXMLData,
	const CString& szCommand,
	const CString& szModule,
	const CString& szObject,
	bool bGetSchema)
{
	ASSERT(pXMLCommand);
	MakeXMLCommand(pXMLCommand, szCommand, szModule, szObject);
	AppendCompanyInfo(pXMLData);
	AppendXMLConnection(pXMLData, CXMLParams::CN_COMPANY, "CONNECTION", bGetSchema);
}
/*********************************************************
Function		AppendXMLConnection
Type			void
Purpose			Appends a CONNECTION node with values from the globals to the pXMLCommand object.
The pXMLCommand object's XML string is the "DATA" XML string which is sent to the
server component.
Parameters		pXMLCommand - CXMLParams object to be filled up.
nLoginType (Optional--default=1) - 1=Primary Server, 2=Company Server, 3=Archive Server.
CString szTag (Optional--default="CONNECTION") Connection node tag
bool bGetSchema (Optional, IN) : add getSchema parameter
CString szUserID (Optional--default=""): override user ID
CString szPassword (Optional--default=""): override password
Author			Kenneth P. Moss
Date			12/17/2002
***********************************************************/
void	CXMLParams::AppendXMLConnection(
	CXMLParams* pXMLCommand,
	const enmConnectionType nLoginType,
	const CString& szTag,
	bool bGetSchema,
	const CString& szUserID,
	const CString& szPassword,
	bool bMaintForm/* = false*/,
	const bool bUseMaster/* = false */)
{
	ASSERT(pXMLCommand);
	CXMLParams clsXMLConnection(true);
	clsXMLConnection.SetXML(GetXMLConnection(nLoginType, szTag, bGetSchema, szUserID, szPassword, bMaintForm, bUseMaster));
	pXMLCommand->AppendXMLParam(&clsXMLConnection);
}

void	CXMLParams::AppendXMLConnection(
	CXMLElement* pXMLCommand,
	const enmConnectionType nLoginType/* = CN_PRIMARY*/,
	const CString& szTag/*=_T("CONNECTION")*/,
	bool bGetSchema/*=false*/,
	const CString& szUserID/* = _T("")*/,
	const CString& szPassword/* = _T("")*/,
	bool bMaintForm/* = false*/,
	const bool bUseMaster/* = false */)
{
	ASSERT(pXMLCommand);

	CGBLSystemInformation::stuServerInformation* stuServer = NULL;
	switch (nLoginType)
	{
	case CN_PRIMARY:
		stuServer = &g_pGBLSystemInformationCMSDll->Servers.PrimaryServer;
		break;
	case CN_COMPANY:
		stuServer = &g_pGBLSystemInformationCMSDll->Servers.CompanyServer;
		break;
	case CN_ARCHIVE:
		stuServer = &g_pGBLSystemInformationCMSDll->Servers.ArchiveServer;
		break;
	default:
		stuServer = &g_pGBLSystemInformationCMSDll->Servers.PrimaryServer;
		ASSERT(stuServer);
		break;
	}

	//Update Erick Korsten 03/11/03: option to rename the connection tag
	bool lbUseMaster = stuServer->UseMaster;
	bool lbGetSchema = stuServer->GetSchema;
	bool lbMaintForm = stuServer->MaintForm;
	CString lszUser = stuServer->User;
	CString lszPassword = stuServer->Password;

	if (bUseMaster) stuServer->UseMaster = true;
	if (bGetSchema) stuServer->GetSchema = true;
	if (bMaintForm) stuServer->MaintForm = true;

	if (szUserID != "")
	{
		stuServer->User = szUserID;
		stuServer->Password = szPassword;
	}
	CXMLElement lpXMLCommand = *pXMLCommand;
	stuServer->AppendServerInfo(lpXMLCommand, szTag);

	stuServer->UseMaster = lbUseMaster;
	stuServer->GetSchema = lbGetSchema;
	stuServer->MaintForm = lbMaintForm;
	stuServer->User = lszUser;
	stuServer->Password = lszPassword;
}

CString	CXMLParams::GetXMLConnection(
	const enmConnectionType nLoginType,
	const CString& szTag,
	bool bGetSchema,
	const CString& szUserID,
	const CString& szPassword,
	bool bMaintForm/* = false*/,
	const bool bUseMaster/* = false */)
{
	CXMLDocument docConnection(CMSStrings::XMLTags::Root);
	CXMLElement eleRoot = docConnection.GetDocumentElement();
	AppendXMLConnection(&eleRoot, nLoginType, szTag, bGetSchema, szUserID, szPassword, bMaintForm, bUseMaster);
	CXMLElement eleConnection = eleRoot.GetItem(CMSStrings::XMLTags::Connection);
	return eleConnection.GetOuterXML();
}

//---------------------------------------------------------------------------
CString	CXMLParams::AppendServerInfo(const CString& szData)
{
	return g_pGBLSystemInformationCMSDll->Servers.AppendServerInfo(szData);
}

//---------------------------------------------------------------------------
void	CXMLParams::SetupCheckStatusXML(
	CXML& clsStatusXML,
	const CString szGUID,
	const CXMLParams::enmConnectionType nLoginType,
	const bool bUseMaster)
{
	// make the xml to query status
	CXMLParams clsXMLCommand;
	clsXMLCommand.MakeParam("COMMAND", "STATUSTABLESTATUS");
	clsXMLCommand.MakeParam("MODULE", "");
	clsXMLCommand.MakeParam("OBJECT", "");

	CXMLParams clsData;
	clsData.MakeParam("GUID", szGUID);
	CXMLParams::AppendXMLConnection(&clsData, nLoginType, "CONNECTION", false, L"", L"", false, bUseMaster);
	CXMLParams::AppendCompanyInfo(&clsData);
	clsStatusXML.m_szXMLCommand = clsXMLCommand.GetXML();
	clsStatusXML.m_szXMLData = clsData.GetXML();
	clsStatusXML.m_szXMLData = CXMLParams::AppendServerInfo(clsStatusXML.m_szXMLData);

	clsData.Dispose();
	clsXMLCommand.Dispose();
}

//---------------------------------------------------------------------------
void	CXMLParams::CancelStatusXML(
	CXML& clsStatusXML,
	const CString szGUID,
	const CXMLParams::enmConnectionType nLoginType,
	const bool bUseMaster)
{
	// make the xml to query status
	CXMLParams clsXMLCommand;
	clsXMLCommand.MakeParam("COMMAND", "CANCELPROCESS");
	clsXMLCommand.MakeParam("MODULE", "");
	clsXMLCommand.MakeParam("OBJECT", "");
	clsXMLCommand.MakeParam("GUID", szGUID);
	CXMLParams clsData;
	CXMLParams::AppendXMLConnection(&clsData, nLoginType, "CONNECTION");
	CXMLParams::AppendCompanyInfo(&clsData);
	clsStatusXML.m_szXMLCommand = clsXMLCommand.GetXML();
	clsStatusXML.m_szXMLData = clsData.GetXML();
	clsStatusXML.m_szXMLData = CXMLParams::AppendServerInfo(clsStatusXML.m_szXMLData);

	clsData.Dispose();
	clsXMLCommand.Dispose();
}

/*********************************************************
Function       UpdateNewFlag
Type           void
Purpose        Sets the new flag on the row equal to what's passed in.
Since this is a friend function, it has access to the private
variables inside pDR.
Parameters     pDR - Data Row to set the flag on.
bNewRow - Value to set the flag to.
Returns        void
Author         Peter Ringering
Date           01/12/2003
***********************************************************/
void	UpdateNewFlag(CDataRow* pDR, bool bNewRow)
{
	ASSERT(pDR);
	pDR->m_bNewRow = bNewRow;
	return;
}

/*********************************************************
Function       IsRowNew
Type           bool
Purpose        Checks the new row flag on the passed in DataRow.
Since this is a friend function, it has access to the private
variables inside pDR.
Parameters     pDR - Data Row to set the flag on.
Returns        true=Row is a new row, false=Row is not a new row
Author         Peter Ringering
Date           01/12/2003
***********************************************************/
bool	IsRowNew(CDataRow* pDR)
{
	ASSERT(pDR);
	return pDR->m_bNewRow;
}

/*********************************************************
Function       CopyTo
Type           void
Purpose        Copies all the XML from this object to pXMLParamTo.
pXMLParamTo is disposed so whatever was in it prior
to copying will be deleted.
Parameters     pXMLParamTo - CXMLParams Object to copy to.
szRootName - new root name
Returns        void
Author         Peter Ringering
Date           01/12/2003
***********************************************************/
void	CXMLParams::CopyTo(CXMLParams* pXMLParamTo, const CString& szRootName)
{
	ASSERT(pXMLParamTo);
	pXMLParamTo->Dispose();

	if ("" == szRootName)
	{
		pXMLParamTo->SetXML(GetXMLBSTR());
	}
	else
	{   // EK - 07/21/03 - Add functionality to set the root name
		try
		{
			pXMLParamTo->SetRoot(szRootName);       // set the new root name
			CCMSXMLDomNodeList::ComponentTypePtr pNodeList = NULL;
			HRESULT hr = m_pRoot->get_childNodes(&pNodeList);
			if (hr != S_OK) return;

			// Copy all the children from the old param to the new param
			for (long lCounter = 0; lCounter < pNodeList->Getlength(); lCounter++)
			{
				CCMSXMLDomNode::ComponentTypePtr pNode, pNewNode;
				pNodeList->get_item(lCounter, &pNode);
				pNewNode = pNode->cloneNode(false);            // Create a new node
				pNewNode->text = pNode->text;                  // Set the value for the new node
				pXMLParamTo->m_pRoot->appendChild(pNewNode);   // Append the new node to the new CXMLParams class
				// Release the pointers to the nodes
//				DestroyNode(pNode);
//				DestroyNode(pNewNode);
			}
		}
		catch (CException* pe)
		{
			HandleError(0, pe);
		}
	}
}

/*********************************************************
Function	HandleError
Type		void
Purpose		Using the parameters, it displays a validation error message to the user.
Parameters	uinErrorID - Error ID to display.
pe - Pointer to a CExeption object.  If this is not null, then whatever is in
this variable is displayed.
pXMLParams - Pointer to a CXMLParams object.  Used to String merge the string found
in uinErrorID with parameters found in this object.
Returns		void
Author		Peter Ringering
Date		11/15/2002
***********************************************************/
static void HandleError(UINT uinErrorID, CException* pe, CXMLParams* pXMLParams, CDataSet* pDataSet)
{
	stuErrorReturn clsErrorReturn;
	CGBLError clsError(clsErrorReturn);
	if (pe != NULL)
	{
		clsError.HandleException(pe);
		return;
	}

	stuErrorInput stuError;
	stuError.n8Module = MODULE_GLOBAL;
	stuError.uiErrorID = uinErrorID;
	if (pXMLParams != NULL) pXMLParams->CopyTo(&stuError.clsErrorParams);
	if (pDataSet != NULL) stuError.szDetail = pDataSet->GetXML();
	clsError.DisplayError(&stuError);
	clsError.ResetErrorReturn();
}

/*---------------------------------------------------------------------------------------------------------
Append FiscalCalendar to xml parameters

Note:		if range of periods doesn't include current period you can designate bIncludeCurrentPeriod=true to include it in params
Parameters:	pXMLCommand   - address of your xml params
n8StartPeriod - first period in range to include
n8EndPeriod	  - last period in range to include
bIncludeCurrentPeriod - do you want to include current period?

Rewritten and commented by BK 2/25/2003
*/
void	CXMLParams::AppendFiscalCalendar(CXMLParams* pXMLCommand, __int8 n8StartPeriod, __int8 n8EndPeriod, bool bIncludeCurrentPeriod)
{
	ASSERT(pXMLCommand);
	int		nPeriod = 0;			// fiscal calendar period
	bool	bIncludesCurrent = false;		// when going thru periods see if this is current period
	CString	szText = _T("");		// string to hold formatted period

	// set Fiscal Calendar root
	CXMLParams clsFiscalCalendar(true);
	clsFiscalCalendar.SetRoot("FiscalCalendar");

	if (n8EndPeriod >= n8StartPeriod)
	{
		// Make Param of Each Period selected by user (set nPeriod from Start Period to End Period textboxes)
		for (nPeriod = n8StartPeriod; nPeriod <= n8EndPeriod; nPeriod++)
		{
			CXMLParams clsPeriodRange(true);						// create xml Range of Periods parameter
			szText.Format(_T("%d"), nPeriod);							// format nPeriod to a string
			clsPeriodRange.SetRoot(_T("Period" + szText));			// set root (example: Period2)
			clsPeriodRange.MakeParam(_T("StartDate"), (g_pGBLSystemInformationCMSDll->FiscalCalendar.FiscalCalendar[nPeriod].StartDate.Format(VAR_DATEVALUEONLY)));
			clsPeriodRange.MakeParam(_T("EndDate"), (g_pGBLSystemInformationCMSDll->FiscalCalendar.FiscalCalendar[nPeriod].EndDate.Format(VAR_DATEVALUEONLY)));
			clsFiscalCalendar.AppendXMLParam(&clsPeriodRange);		// append PeriodRange to FiscalCalendar
			clsPeriodRange.Dispose();								// release memory clsPeriodRange param

			// check if this is the current period
			if (nPeriod == g_pGBLSystemInformationCMSDll->FiscalCalendar.CurrentPeriod)
				bIncludesCurrent = true;
		}
	}
	// if user wants to include current period and it was not included in the range of periods above then append current period now
	if (bIncludeCurrentPeriod && !bIncludesCurrent)
	{
		CXMLParams	 clsCurrentPeriod(true);										// create xml CurrentPeriod parameter
		nPeriod = g_pGBLSystemInformationCMSDll->FiscalCalendar.CurrentPeriod;		// save current period as nPeriod
		szText.Format(_T("%d"), nPeriod);												// format period as text

		// Current Period root
		clsCurrentPeriod.SetRoot(_T("Period" + szText));							// append period number to "Period"
		clsCurrentPeriod.MakeParam(_T("StartDate"), g_pGBLSystemInformationCMSDll->FiscalCalendar.FiscalCalendar[nPeriod].StartDate.Format(VAR_DATEVALUEONLY));
		clsCurrentPeriod.MakeParam(_T("EndDate"), g_pGBLSystemInformationCMSDll->FiscalCalendar.FiscalCalendar[nPeriod].EndDate.Format(VAR_DATEVALUEONLY));
		clsFiscalCalendar.AppendXMLParam(&clsCurrentPeriod);		// append clsCurrentPeriod param to FiscalCalendar
		clsCurrentPeriod.Dispose();									// release memory clsCurrentPeriod param
	}
	// Now append the newly created fiscal calendar xml to the passed in node
	pXMLCommand->MakeParam(_T("intNumberOfPeriods"), (g_pGBLSystemInformationCMSDll->FiscalCalendar.NumberOfPeriods));
	pXMLCommand->MakeParam(_T("intBlockPeriod"), (g_pGBLSystemInformationCMSDll->FiscalCalendar.BlockPeriod));
	pXMLCommand->AppendXMLParam(&clsFiscalCalendar);
}

//---------------------------------------------------------------------------------------------------------
// * Procedure:    AppendCompanyInfo
// *
// * Description:  Appends the COMPANYINFO node to the current XML Params.
// *
// * Parameters:   pXMLCommand - Pointer to a CXMLParams object.
// *
// * Date Created: 03/15/2003 by Peter Ringering
// ********************************************************
void	CXMLParams::AppendCompanyInfo(CXMLParams* pXMLCommand)
{
	ASSERT(pXMLCommand);
	CXML clsXML;
	clsXML.MakeCOMPANYINFO();
	CXMLParams clsCompanyInfo(true);
	clsCompanyInfo.SetXML(clsXML.m_szXMLFormat);
	pXMLCommand->AppendXMLParam(&clsCompanyInfo);
	clsCompanyInfo.Dispose();
}

//-------------------------------------------------------------------------
void	CXMLParams::AppendCompanyInfo(CXMLElement* pXMLCommand)
{
	CXML clsXML;
	clsXML.MakeCOMPANYINFO();
	CXMLParams clsCompanyInfo(false);
	AppendCompanyInfo(&clsCompanyInfo);
	pXMLCommand->AppendXMLParams(&clsCompanyInfo, true);
}

//-------------------------------------------------------------------------
// append params to pass GL Classification Code translation
void	CXMLParams::AppendGLClassificationTranslation(CXMLParams* pXML)
{
	ASSERT(pXML);
	CXMLParams   clsTranslate(true);
	CXMLParams* pField;

	clsTranslate.SetRoot("TRANSLATE");

	// Non Ledger
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "0");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(0));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Asset
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "10");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(10));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Current Asset
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "20");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(20));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Fixed Asset
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "30");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(30));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Long Term Investments (Asset)
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "40");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(40));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Contra-Asset
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "100");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(100));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Liability
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "200");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(200));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Current Liability
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "210");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(210));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Long Term Liability
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "220");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(220));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Contra-Liability
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "300");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(300));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Equity
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "400");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(400));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Contra-Equity
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "500");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(500));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Income (Operating Credit)
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "600");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(600));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Other Revenue/Gains (Income)
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "610");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(610));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Income Adjustment
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "700");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(700));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Expense (Operating Debit)
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "800");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(800));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Cost of Good Sold (Expense)
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "810");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(810));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Selling Expenses (Expense)
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "820");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(820));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Administrative Expenses (Expense)
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "830");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(830));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Other Expense (Expense)
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "840");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(840));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Tax Expense (Expense)
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "850");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(850));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Expense Adjustment
	pField = new CXMLParams(true);
	pField->SetRoot("bytClassification");
	pField->MakeParam("bytValue", "900");
	pField->MakeParam("strString", CGBLTranslate::TranslateGLClassification(900));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// append params to param passed in
	pXML->AppendXMLParam(&clsTranslate);
}

//-------------------------------------------------------------------------
// append params to pass GL Credit/Debit Code (from curAmount field) translation
void	CXMLParams::AppendGLDebitCreditTranslation(CXMLParams* pXML)
{
	ASSERT(pXML);
	CXMLParams   clsTranslate(true);
	CXMLParams* pField;

	clsTranslate.SetRoot("TRANSLATE");

	// Credit
	pField = new CXMLParams(true);
	pField->SetRoot("curAmount");
	pField->MakeParam("bytValue", "0");
	pField->MakeParam("operator", "<");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_MASTER_CR)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Debit
	pField = new CXMLParams(true);
	pField->SetRoot("curAmount");
	pField->MakeParam("bytValue", "0");
	pField->MakeParam("operator", ">=");
	pField->MakeParam("strString", "");
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// append params to param passed in
	pXML->AppendXMLParam(&clsTranslate);
}

//-------------------------------------------------------------------------
// append params to pass GL Fund Type
void CXMLParams::AppendGLFundTypeTranslation(CXMLParams* pXML)
{
	ASSERT(pXML);
	CXMLParams   clsTranslate(true);
	CXMLParams* pField;

	clsTranslate.SetRoot("TRANSLATE");

	// Restricted
	pField = new CXMLParams(true);
	pField->SetRoot("bytFundType");
	pField->MakeParam("bytValue", "0");
	pField->MakeParam("strString", RESSTRING(IDS_FUNDTRANSFEROPTION1));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Due To/Due From
	pField = new CXMLParams(true);
	pField->SetRoot("bytFundType");
	pField->MakeParam("bytValue", "1");
	pField->MakeParam("strString", RESSTRING(IDS_FUNDTRANSFEROPTION2));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Unrestricted
	pField = new CXMLParams(true);
	pField->SetRoot("bytFundType");
	pField->MakeParam("bytValue", "2");
	pField->MakeParam("strString", RESSTRING(IDS_FUNDTRANSFEROPTION3));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// append params to param passed in
	pXML->AppendXMLParam(&clsTranslate);
}

//-------------------------------------------------------------------------
// append params to pass AR Yes/No Code (from CASH field) translation
void	CXMLParams::AppendARCashTranslation(CXMLParams* pXML)
{
	ASSERT(pXML);
	CXMLParams   clsTranslate(true);
	CXMLParams* pField;

	clsTranslate.SetRoot("TRANSLATE");

	// Credit
	pField = new CXMLParams(true);
	pField->SetRoot("bolIsCash");
	pField->MakeParam("bytValue", "0");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_NO)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Debit
	pField = new CXMLParams(true);
	pField->SetRoot("bolIsCash");
	pField->MakeParam("bytValue", "1");
	pField->MakeParam("operator", ">=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_YES)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// append params to param passed in
	pXML->AppendXMLParam(&clsTranslate);
}

//-------------------------------------------------------------------------
// append params to pass AP Line Type (from bytLineType field) translation
//0 = Payment
//1 = Direct Expense
//2 = Prepaid Expense
//3 = Future Liability
//4 = Job Cost
//5 = Inventory
//6 = Landing
//7 = Debit Memo
void	CXMLParams::AppendAPLineTypeTranslation(CXMLParams* pXML)
{
	ASSERT(pXML);
	CXMLParams   clsTranslate(true);
	CXMLParams* pField;

	clsTranslate.SetRoot("TRANSLATE");

	//1-25128 AES Most are not correct values
	//0 = Deleted
	pField = new CXMLParams(true);
	pField->SetRoot("bytLineType");
	pField->MakeParam("bytValue", "0");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_DELETED)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// 1 = Direct Expense
	pField = new CXMLParams(true);
	pField->SetRoot("bytLineType");
	pField->MakeParam("bytValue", "1");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_DIRECT_EXPENSE)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//2 = Prepaid Expense
	pField = new CXMLParams(true);
	pField->SetRoot("bytLineType");
	pField->MakeParam("bytValue", "2");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_PREPAID_EXPENSE)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//3 = Future Liability
	pField = new CXMLParams(true);
	pField->SetRoot("bytLineType");
	pField->MakeParam("bytValue", "3");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_FUTURE_LIABILITY)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//4 = Inventory
	pField = new CXMLParams(true);
	pField->SetRoot("bytLineType");
	pField->MakeParam("bytValue", "4");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_MODULE_IN)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//5 = Landing
	pField = new CXMLParams(true);
	pField->SetRoot("bytLineType");
	pField->MakeParam("bytValue", "5");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AP_ENTERBILLS_LANDING)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//6 = Purchase Discount
	pField = new CXMLParams(true);
	pField->SetRoot("bytLineType");
	pField->MakeParam("bytValue", "6");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AP_PURCHASE_DISCOUNT)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//7 = Allocation
	pField = new CXMLParams(true);
	pField->SetRoot("bytLineType");
	pField->MakeParam("bytValue", "7");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AP_ENTERBILLS_ALLOCATION)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//8 = Allocation Adj
	pField = new CXMLParams(true);
	pField->SetRoot("bytLineType");
	pField->MakeParam("bytValue", "7");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_ALLOC_ADJ)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;
	//TODO Add Job Cost for AP
	// append params to param passed in
	pXML->AppendXMLParam(&clsTranslate);
}

//-------------------------------------------------------------------------
//1-26392/1-26396 BK 12/05/06 combined 2 fields into a calculated field to display correctly
// (bytOpenTranType * 100) + IsNull(bytLineType, 0) as LKP_OpenTranLineType
void	CXMLParams::AppendAPPaymentTranslation(CXMLParams* pXML)
{
	ASSERT(pXML);
	CXMLParams   clsTranslate(true);
	CXMLParams* pField;

	clsTranslate.SetRoot("TRANSLATE");
	// Debit Memo
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_OpenTranLineType");
	pField->MakeParam("bytValue", "700");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AP_DEBIT_MEMO)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Payment - Credit Adjustment
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_OpenTranLineType");
	pField->MakeParam("bytValue", "200");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AR_PAYMENT)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Payment - Payment Correction
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_OpenTranLineType");
	pField->MakeParam("bytValue", "300");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AR_PAYMENT)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Payment - Payment
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_OpenTranLineType");
	pField->MakeParam("bytValue", "400");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AR_PAYMENT)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Payment - Debit Adjustment
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_OpenTranLineType");
	pField->MakeParam("bytValue", "500");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AR_PAYMENT)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Payment - Unapplied Debit
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_OpenTranLineType");
	pField->MakeParam("bytValue", "700");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AR_PAYMENT)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Payment - Apply Unapplieds Directly
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_OpenTranLineType");
	pField->MakeParam("bytValue", "800");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AR_PAYMENT)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Invoice Types
	//0 = Deleted
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_OpenTranLineType");
	pField->MakeParam("bytValue", "0");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_DELETED)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// 1 = Direct Expense
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_OpenTranLineType");
	pField->MakeParam("bytValue", "101");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_DIRECT_EXPENSE)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//2 = Prepaid Expense
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_OpenTranLineType");
	pField->MakeParam("bytValue", "102");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_PREPAID_EXPENSE)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//3 = Future Liability
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_OpenTranLineType");
	pField->MakeParam("bytValue", "103");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_FUTURE_LIABILITY)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//4 = Inventory
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_OpenTranLineType");
	pField->MakeParam("bytValue", "104");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_MODULE_IN)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//5 = Landing
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_OpenTranLineType");
	pField->MakeParam("bytValue", "105");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AP_ENTERBILLS_LANDING)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//6 = Purchase Discount
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_OpenTranLineType");
	pField->MakeParam("bytValue", "106");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AP_PURCHASE_DISCOUNT)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// 1 = Direct Expense
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_OpenTranLineType");
	pField->MakeParam("bytValue", "601");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_DIRECT_EXPENSE)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//2 = Prepaid Expense
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_OpenTranLineType");
	pField->MakeParam("bytValue", "602");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_PREPAID_EXPENSE)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//3 = Future Liability
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_OpenTranLineType");
	pField->MakeParam("bytValue", "603");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_FUTURE_LIABILITY)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//4 = Inventory
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_OpenTranLineType");
	pField->MakeParam("bytValue", "604");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_MODULE_IN)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//5 = Landing
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_OpenTranLineType");
	pField->MakeParam("bytValue", "605");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AP_ENTERBILLS_LANDING)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//6 = Purchase Discount
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_OpenTranLineType");
	pField->MakeParam("bytValue", "606");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AP_PURCHASE_DISCOUNT)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// BK 12/06/06 Following type never used
	////7 = Allocation
	//pField = new CXMLParams(true);
	//pField->SetRoot("LKP_OpenTranLineType");
	//pField->MakeParam("bytValue",  "7");
	//   pField->MakeParam("operator", "=");
	//pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AP_ENTERBILLS_ALLOCATION)));
	//clsTranslate.AppendXMLParam(pField);
	//delete pField;

	////17 = Allocation Adj
	//pField = new CXMLParams(true);
	//pField->SetRoot("LKP_OpenTranLineType");
	//pField->MakeParam("bytValue",  "7");
	//   pField->MakeParam("operator", "=");
	//pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_ALLOC_ADJ)));
	//clsTranslate.AppendXMLParam(pField);
	//delete pField;
	//TODO Add Job Cost for AP
	// append params to param passed in
	pXML->AppendXMLParam(&clsTranslate);
}

//-------------------------------------------------------------------------
// append params to pass AP Transaction Type (from bytTranType field) translation
//1 = Invoice
//2 = CR Adjustment
//3 = Payment Correction
//4 = Payment
//5 = DR Adjustment
//6 = DR Invoice
//7 = Unapplied DR
//8 = Apply DR
//9 = Unapplied Payment
void	CXMLParams::AppendAPTranTypeTranslation(CXMLParams* pXML)
{
	ASSERT(pXML);
	CXMLParams   clsTranslate(true);
	CXMLParams* pField;

	clsTranslate.SetRoot("TRANSLATE");

	// 1 = Invoice
	pField = new CXMLParams(true);
	pField->SetRoot("bytTranType");
	pField->MakeParam("bytValue", "1");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_INVOICE)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//2 = CR Adjustment
	pField = new CXMLParams(true);
	pField->SetRoot("bytTranType");
	pField->MakeParam("bytValue", "2");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AP_CR_ADJ)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//3 = Payment Correction
	pField = new CXMLParams(true);
	pField->SetRoot("bytTranType");
	pField->MakeParam("bytValue", "3");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AP_PAY_COR)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//4 = Payment
	pField = new CXMLParams(true);
	pField->SetRoot("bytTranType");
	pField->MakeParam("bytValue", "4");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AP_PAYMENT)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//5 = DR Adjustment
	pField = new CXMLParams(true);
	pField->SetRoot("bytTranType");
	pField->MakeParam("bytValue", "5");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AP_DR_ADJ)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//6 = DR Invoice
	pField = new CXMLParams(true);
	pField->SetRoot("bytTranType");
	pField->MakeParam("bytValue", "6");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AP_DR_INV)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//7 = Unapplied DR
	pField = new CXMLParams(true);
	pField->SetRoot("bytTranType");
	pField->MakeParam("bytValue", "7");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AP_UN_DR)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//8 = Apply DR
	pField = new CXMLParams(true);
	pField->SetRoot("bytTranType");
	pField->MakeParam("bytValue", "8");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_AP_APPLY_DR)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//9 = Unapplied Payment
	pField = new CXMLParams(true);
	pField->SetRoot("bytTranType");
	pField->MakeParam("bytValue", "9");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_UN_PAYMENT)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// append params to param passed in
	pXML->AppendXMLParam(&clsTranslate);
}

// ********************************************************
// * Procedure:    SetParamStr
// *
// * Description:  Changes the node string value for the node specified
// *               by the input tag name.
// *
// * Parameters:   szTagName - Name of node to change
// *               szValue   - New string value
// *
// * Date Created: 04/16/2003 by Erick Korsten
// ********************************************************
bool	CXMLParams::SetParamStr(const CString& szTagName, const CString& szValue) const
{
	bool bRc = false;
	try
	{
		CCMSXMLDomNode::ComponentTypePtr pNode = m_pRoot->selectSingleNode(_bstr_t(szTagName));
		if (pNode != NULL)
		{
			pNode->text = _bstr_t(szValue);
			bRc = true;
		}
	}
	catch (...)
	{

	}
	return bRc;
}

// ********************************************************
// * Procedure:    SetParamLng
// *
// * Description:  Changes the node string value for the node specified
// *               by the input tag name.
// *
// * Parameters:   szTagName - Name of node to change
// *               lValue    - New long value
// *
// * Date Created: 05/07/2007 by David W. Parvin
// ********************************************************
bool	CXMLParams::SetParamLng(const CString& szTagName, const long& lValue) const
{
	bool bRc = false;
	try
	{
		CCMSXMLDomNode::ComponentTypePtr pNode = m_pRoot->selectSingleNode(_bstr_t(szTagName));
		if (pNode == NULL)
		{
			CCMSXMLDomNode::ComponentTypePtr pNewNode;
			CCMSXMLDomDoc::ComponentTypePtr ownerDocument;
			CString szValue;
			szValue.Format(_T("%i"), lValue);

			ownerDocument = m_pRoot->GetownerDocument();
			pNewNode = ownerDocument->createElement(_bstr_t(szTagName));    // Create a new node
			pNewNode->text = _bstr_t(szValue);								// Set the value for the new node
			m_pRoot->appendChild(pNewNode);									// Append the new node to the new CXMLParams class
			// Release the pointers to the nodes
//			DestroyNode(pNewNode);

			bRc = true;
		}
		else
		{
			CString szValue;
			szValue.Format(_T("%i"), lValue);
			pNode->text = _bstr_t(szValue);
			bRc = true;
		}
	}
	catch (...)
	{

	}
	return bRc;
}

void	CXMLParams::AppendBRTransactionTypeTranslation(CXMLParams* pXML)
{
	ASSERT(pXML);
	CXMLParams   clsTranslate(true);
	CXMLParams* pField;

	clsTranslate.SetRoot("TRANSLATE");

	// 0 = Check
	pField = new CXMLParams(true);
	pField->SetRoot("bytTransactionType");
	pField->MakeParam("bytValue", "0");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_CHECK)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//1 = Deposit
	pField = new CXMLParams(true);
	pField->SetRoot("bytTransactionType");
	pField->MakeParam("bytValue", "1");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_BR_DEPOSIT)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//2 = Deduction
	pField = new CXMLParams(true);
	pField->SetRoot("bytTransactionType");
	pField->MakeParam("bytValue", "2");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_BR_DEDUCTION)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//3 = Add
	pField = new CXMLParams(true);
	pField->SetRoot("bytTransactionType");
	pField->MakeParam("bytValue", "3");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_BR_ADDITION)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//4 = BC
	pField = new CXMLParams(true);
	pField->SetRoot("bytTransactionType");
	pField->MakeParam("bytValue", "4");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_BR_BANK_CHARGE)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//5 = Transfer
	pField = new CXMLParams(true);
	pField->SetRoot("bytTransactionType");
	pField->MakeParam("bytValue", "5");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_IN_TRANSFER)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// append params to param passed in
	pXML->AppendXMLParam(&clsTranslate);
}

void	CXMLParams::AppendBRStatusTranslation(CXMLParams* pXML)
{
	ASSERT(pXML);
	CXMLParams   clsTranslate(true);
	CXMLParams* pField;

	clsTranslate.SetRoot("TRANSLATE");

	// 0 = Outstanding
	pField = new CXMLParams(true);
	pField->SetRoot("bytCheckStatus");
	pField->MakeParam("bytValue", "0");
	pField->MakeParam("operator", "=");
	//pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_BR_OS)));
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_BR_OUTSTANDING)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//1 = Cleared
	pField = new CXMLParams(true);
	pField->SetRoot("bytCheckStatus");
	pField->MakeParam("bytValue", "1");
	pField->MakeParam("operator", "=");
	//pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_BR_CLRD)));
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_BR_CLEARED)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//2 = Voided
	pField = new CXMLParams(true);
	pField->SetRoot("bytCheckStatus");
	pField->MakeParam("bytValue", "2");
	pField->MakeParam("operator", "=");
	//pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_BR_VOID)));
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_BR_VOIDED)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// append params to param passed in
	pXML->AppendXMLParam(&clsTranslate);
}

// 1-27851 BK 6/15/07 translate creation module column to: BR Payee, Vendor, or Employee
void	CXMLParams::AppendBRPayeeTypeTranslation(CXMLParams* pXML)
{
	ASSERT(pXML);
	CXMLParams   clsTranslate(true);
	CXMLParams* pField;

	clsTranslate.SetRoot("TRANSLATE");

	// 0 (BR) = BR Payee
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_CreationModule");
	pField->MakeParam("bytValue", "0");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_BR_BRPAYEE)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// 1 (AP) = Vendor
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_CreationModule");
	pField->MakeParam("bytValue", "1");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_LBL_VENDOR)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// 2 (PR) = Employee
	pField = new CXMLParams(true);
	pField->SetRoot("LKP_CreationModule");
	pField->MakeParam("bytValue", "2");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_BR_EMPLOYEE)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// append params to param passed in
	pXML->AppendXMLParam(&clsTranslate);
}

void	CXMLParams::AppendBRTransactionTypeAndStatusTranslation(CXMLParams* pXML)
{
	ASSERT(pXML);
	CXMLParams   clsTranslate(true);
	CXMLParams* pField;

	clsTranslate.SetRoot("TRANSLATE");

	// 0 = Check
	pField = new CXMLParams(true);
	pField->SetRoot("bytTransactionType");
	pField->MakeParam("bytValue", "0");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_BR_CHK)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//1 = Deposit
	pField = new CXMLParams(true);
	pField->SetRoot("bytTransactionType");
	pField->MakeParam("bytValue", "1");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_BR_DEP)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//2 = Deduction
	pField = new CXMLParams(true);
	pField->SetRoot("bytTransactionType");
	pField->MakeParam("bytValue", "2");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_BR_DED)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//3 = Add
	pField = new CXMLParams(true);
	pField->SetRoot("bytTransactionType");
	pField->MakeParam("bytValue", "3");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_BR_ADD)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//4 = BC
	pField = new CXMLParams(true);
	pField->SetRoot("bytTransactionType");
	pField->MakeParam("bytValue", "4");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_BR_BC)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//0 = O/S
	pField = new CXMLParams(true);
	pField->SetRoot("bytCheckStatus");
	pField->MakeParam("bytValue", "0");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_BR_OS)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//1 = Cleared
	pField = new CXMLParams(true);
	pField->SetRoot("bytCheckStatus");
	pField->MakeParam("bytValue", "1");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_BR_CLRD)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//2 = Void
	pField = new CXMLParams(true);
	pField->SetRoot("bytCheckStatus");
	pField->MakeParam("bytValue", "2");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_BR_VOID)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// append params to param passed in
	pXML->AppendXMLParam(&clsTranslate);
}

void	CXMLParams::AppendINSerialNumberTranslation(CXMLParams* pXML)
{
	ASSERT(pXML);
	// EK - 1-9733 - 02/17/2004 - Adding code to translate the serial number field for Standard Cost and Weighted Average types
	CXMLParams   clsTranslate(true);
	CXMLParams* pField;

	clsTranslate.SetRoot("TRANSLATE");

	// NULL = <<SUMMARY>>
	pField = new CXMLParams(true);
	pField->SetRoot("strSerialNumber");
	pField->MakeParam("strValue", "NULL");
	pField->MakeParam("operator", "IS");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_SUMMARY_CQ)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// "" = <<SUMMARY>>
	pField = new CXMLParams(true);
	pField->SetRoot("strSerialNumber");
	pField->MakeParam("strValue", "");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_SUMMARY_CQ)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// ELSE => use serial number field
	pField = new CXMLParams(true);
	pField->SetRoot("strSerialNumber");
	pField->MakeParam("strField", "strSerialNumber");
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// append params to param passed in
	pXML->AppendXMLParam(&clsTranslate);
}

//Translation strings for bytModule field
void	CXMLParams::AppendSAEntryWindowTranslation(CXMLParams* pXML)
{
	ASSERT(pXML);
	CXMLParams   clsTranslate(true);
	CXMLParams* pField;

	clsTranslate.SetRoot("TRANSLATE");

	//1 = Order Entry
	pField = new CXMLParams(true);
	pField->SetRoot("bytModule");
	pField->MakeParam("bytValue", "1");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_MODULE_OE)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//2 = POS Register
	pField = new CXMLParams(true);
	pField->SetRoot("bytModule");
	pField->MakeParam("bytValue", "2");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_POS_REGISTER)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//3 = Job Cost
	pField = new CXMLParams(true);
	pField->SetRoot("bytModule");
	pField->MakeParam("bytValue", "3");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_MODULE_JC)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// append params to param passed in
	pXML->AppendXMLParam(&clsTranslate);
}

//-------------------------------------------------------------------------
// append params to display ___ as Single batch
void	CXMLParams::AppendSABatchTranslation(CXMLParams* pXML)
{
	ASSERT(pXML);
	CXMLParams   clsTranslate(true);
	CXMLParams* pField;

	clsTranslate.SetRoot("TRANSLATE");

	// Single batch
	pField = new CXMLParams(true);
	pField->SetRoot("strBatchRegNo");
	pField->MakeParam("strValue", "___");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_SINGLE)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Else
	pField = new CXMLParams(true);
	pField->SetRoot("strBatchRegNo");
	pField->MakeParam("strField", "strBatchRegNo");
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// append params to param passed in
	pXML->AppendXMLParam(&clsTranslate);
}

void	CXMLParams::AppendSADepositStatusTranslation(CXMLParams* pXML)
{
	ASSERT(pXML);
	CXMLParams   clsTranslate(true);
	CXMLParams* pField;

	clsTranslate.SetRoot("TRANSLATE");

	// 50 = Deposit Cash
	pField = new CXMLParams(true);
	pField->SetRoot("bytPaymentType");
	pField->MakeParam("bytValue", "50");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_DEPOSITCASH)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//51 = Deposit Check
	pField = new CXMLParams(true);
	pField->SetRoot("bytPaymentType");
	pField->MakeParam("bytValue", "51");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_DEPOSITCHECK)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//52 = Deposit Credit Card
	pField = new CXMLParams(true);
	pField->SetRoot("bytPaymentType");
	pField->MakeParam("bytValue", "52");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_DEPOSITCC)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	//53 = Deposit Alternate Tender
	pField = new CXMLParams(true);
	pField->SetRoot("bytPaymentType");
	pField->MakeParam("bytValue", "53");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_DEPOSITALTTENDER)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// 1-22818 BK 1/13/06 corrected code - Gift Card code is 56
	//54 = Deposit Gift Card
	pField = new CXMLParams(true);
	pField->SetRoot("bytPaymentType");
	pField->MakeParam("bytValue", "56");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_DEPOSITGIFTCARD)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// 1-22818 BK 1/13/06 added Debit Card
	//57 = Deposit Debit Card
	pField = new CXMLParams(true);
	pField->SetRoot("bytPaymentType");
	pField->MakeParam("bytValue", "57");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_DEPOSIT_DEBIT)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// append params to param passed in
	pXML->AppendXMLParam(&clsTranslate);
}

// BK 1/3/06 added posted/unposted column - not using added to code in CDetailGrid under TYP_BATCH
void	CXMLParams::AppendSAWorkOrderTranslation(CXMLParams* pXML)
{
	ASSERT(pXML);
	CXMLParams   clsTranslate(true);
	CXMLParams* pField;

	clsTranslate.SetRoot("TRANSLATE");

	// Posted batch
	pField = new CXMLParams(true);
	pField->SetRoot("strBatchRegNo");
	pField->MakeParam("strValue", "OEQUT");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_SA_POSTED)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Else
	pField = new CXMLParams(true);
	pField->SetRoot("strBatchRegNo");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_SA_UNPOSTED)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// append params to param passed in
	pXML->AppendXMLParam(&clsTranslate);
}

void	CXMLParams::AppendPOLayoutCodeTranslation(CXMLParams* pXML)
{
	ASSERT(pXML);
	CXMLParams   clsTranslate(true);
	CXMLParams* pField;

	clsTranslate.SetRoot("TRANSLATE");

	// Purchasing
	pField = new CXMLParams(true);
	pField->SetRoot("bytEntryScreen");
	pField->MakeParam("bytValue", "0");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_PURCHASING)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Receiving
	pField = new CXMLParams(true);
	pField->SetRoot("bytEntryScreen");
	pField->MakeParam("bytValue", "1");
	pField->MakeParam(_T("strString"), (CGBLResources::GetResourceString(IDS_RECEIVING)));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// append params to param passed in
	pXML->AppendXMLParam(&clsTranslate);
}

void	CXMLParams::AppendUdfModPrefsTranslation(CXMLParams* pXML)
{
	ASSERT(pXML);
	CXMLParams   clsTranslate(true);
	CXMLParams* pField;

	clsTranslate.SetRoot("TRANSLATE");

	// Field Type 0 - Text
	pField = new CXMLParams(true);
	pField->SetRoot("bytFieldType");
	pField->MakeParam("bytValue", "0");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), RESSTRING(IDS_TEXT));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Field Type 1 - Number
	pField = new CXMLParams(true);
	pField->SetRoot("bytFieldType");
	pField->MakeParam("bytValue", "1");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), RESSTRING(IDS_NUMBER));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Field Type 2 - Date
	pField = new CXMLParams(true);
	pField->SetRoot("bytFieldType");
	pField->MakeParam("bytValue", "2");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), RESSTRING(IDS_DATE));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Field Type 3 - Checkbox
	pField = new CXMLParams(true);
	pField->SetRoot("bytFieldType");
	pField->MakeParam("bytValue", "3");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), RESSTRING(IDS_CHECKBOX));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Field Type 4 - Multi-line Text
	pField = new CXMLParams(true);
	pField->SetRoot("bytFieldType");
	pField->MakeParam("bytValue", "4");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), RESSTRING(IDS_MULTILINE_TEXT));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Lookup 0 - No
	pField = new CXMLParams(true);
	pField->SetRoot("bolLookup");
	pField->MakeParam("bytValue", "0");
	pField->MakeParam("operator", "=");
	pField->MakeParam(_T("strString"), RESSTRING(IDS_NO));
	clsTranslate.AppendXMLParam(pField);
	delete pField;

	// Lookup 1 - Yes
	pField = new CXMLParams(true);
	pField->SetRoot("bolLookup");
	pField->MakeParam("bytValue", "1");
	pField->MakeParam("operator", ">=");
	pField->MakeParam(_T("strString"), RESSTRING(IDS_YES));
	clsTranslate.AppendXMLParam(pField);
	delete pField;


	//// Size 0 - Empty String
	//pField = new CXMLParams(true);
	//pField->SetRoot("bytLength");
	//pField->MakeParam("bytValue",  "0");
	//pField->MakeParam("operator", "=");
	//pField->MakeParam(_T("strString"), EMPTY_STRING);
	//clsTranslate.AppendXMLParam(pField);
	//delete pField;

	//// Size Else display value
	//pField = new CXMLParams(true);
	//pField->SetRoot("bytLength");
	//pField->MakeParam("bytValue",  "0");
	//pField->MakeParam("operator", ">");
	//pField->MakeParam("strField", "bytLength");
	//clsTranslate.AppendXMLParam(pField);
	//delete pField;

	// append params to param passed in
	pXML->AppendXMLParam(&clsTranslate);
}
