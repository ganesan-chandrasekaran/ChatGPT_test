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

#include "StdAfx.h"
#include "Winspool.h"
#include <memory>

extern CGBLSystemInformation * g_pGBLSystemInformationCMSDll;

//------------------------------------------------------------------------------
__declspec(dllexport) bool CMS::IsAlpha(const CString& szValue, const CString& szExtraAllowedChrs)
{
	CString szValidChars(CGBLResources::GetResourceString(IDS_GBL_ALPHABET));
	CString szExtraChrs(szExtraAllowedChrs);

	szValidChars = szValidChars.Trim() + szExtraChrs.Trim();

	const CString szExtracted(szValue.SpanIncluding(szValidChars));

	//if the length of the incoming string is not equal to the extracted
	//string, then the incoming contained characters not found in the
	//resource string or in the passed extra valid characters string
	return (szValue.GetLength() == szExtracted.GetLength());
}

//------------------------------------------------------------------------------
__declspec(dllexport) bool CMS::IsNumeric(const CString& szValue, const CString& szExtraAllowedChrs)
{
	CString szValidChars(CGBLResources::GetResourceString(IDS_GBL_NUMBERS));
	CString szExtraChrs(szExtraAllowedChrs);

	szValidChars = szValidChars.Trim() + szExtraChrs.Trim();

	const CString szExtracted(szValue.SpanIncluding(szValidChars));

	//if the length of the incoming string is not equal to the extracted
	//string, then the incoming contained characters not found in the
	//resource string or in the passed extra valid characters string
	return (szValue.GetLength() == szExtracted.GetLength());
}

//------------------------------------------------------------------------------
__declspec(dllexport) bool CMS::IsNumeric(TCHAR szValue, const CString& szExtraAllowedChrs)
{
	CString szValidChars(CGBLResources::GetResourceString(IDS_GBL_NUMBERS));
	CString szExtraChrs(szExtraAllowedChrs);

	szValidChars = szValidChars.Trim() + szExtraChrs.Trim();
	return (szValidChars.Find(szValue) > -1);
}

//------------------------------------------------------------------------------
__declspec(dllexport) bool CMS::IsPunct(const CString& szValue, const CString& szExtraAllowedChrs)
{
	CString szValidChars(CGBLResources::GetResourceString(IDS_GBL_PUNCTUATION));
	CString szExtraChrs(szExtraAllowedChrs);

	szValidChars = szValidChars.Trim() + szExtraChrs.Trim();

	const CString szExtracted(szValue.SpanIncluding(szValidChars));

	//if the length of the incoming string is not equal to the extracted
	//string, then the incoming contained characters not found in the
	//resource string or in the passed extra valid characters string
	return (szValue.GetLength() == szExtracted.GetLength());
}

//------------------------------------------------------------------------------
__declspec(dllexport) BOOL CMS::IsAlphaNumeric(const CString& szValue, const CString& szExtraAllowedChrs)
{
	CString szValidAlphaChars(CGBLResources::GetResourceString(IDS_GBL_ALPHABET));
	CString szValidNumericChars(CGBLResources::GetResourceString(IDS_GBL_NUMBERS));
	CString szExtraChrs(szExtraAllowedChrs);

	const CString szValidChars(szValidAlphaChars.Trim() + szValidNumericChars.Trim() + szExtraChrs.Trim());
	const CString szExtracted(szValue.SpanIncluding(szValidChars));

	//if the length of the incoming string is not equal to the extracted
	//string, then the incoming contained characters not found in the
	//resource strings or in the passed extra valid characters string
	return (szValue.GetLength() == szExtracted.GetLength());
}

//------------------------------------------------------------------------------
__declspec(dllexport) void CMS::MakeXMLCommand(CXML& clsXML, const CString& szModule, const CString& szForm, int nCommand, const CStringArray& szarrValues)
{
	//we must receive an even number of parameters (column names
	//are odd, parameter values are even) otherwise we're screwed.
	ASSERT((szarrValues.GetCount() % 2) == 0);

	CString szCommand(_T("GET"));
	CString szDirection(_T(""));

	switch (nCommand)
	{		//set the direction for XML
	case RECORD_FIRST:
		break;
	case RECORD_NEXT:
		szDirection = _T("NEXT");			//get next record
		break;
	case RECORD_PREVIOUS:
		szDirection = _T("PREVIOUS");	//get previous record
		break;
	case RECORD_CURRENT:
		szDirection = _T("CURRENT");		//get current record
		break;
	case RECORD_AUTOFILL:
		szCommand = _T("AUTOFILL");	//get a single field for Autofill
		szDirection = _T("FIRST");			//get first matching record
		break;
	case RECORD_UPDATE:
		szCommand = _T("UPDATE");
		break;
	default:
		ASSERT(1 == 0);						//break because we got an invalid command
	}

	CString	 szXML(_T(""));

	//format column names (odd indexes) and column values (even indexes)
	for (long nIdx = 0; nIdx < szarrValues.GetCount(); nIdx += 2)
		szXML += CXML::MakeElement(szarrValues[nIdx], szarrValues[nIdx + 1]);

	//wrap the column information with the required <RECORD>, <TABLE> and <ROOT> tags
	szXML = CXML::MakeRecordElement(szXML);
	szXML = CXML::MakeTableElement(szXML);
	if (!szDirection.IsEmpty()) szXML += _T("<DIRECTION>") + szDirection + _T("</DIRECTION>");
	szXML = CXML::MakeRootElement(szXML);

	//push the column information into a CXMLParams instance and attach
	//the required company information and server login information to it
	CXMLParams clsRoot(true);
	clsRoot.SetXML(szXML);
	CXMLParams::AppendCompanyInfo(&clsRoot);
	CXMLParams::AppendXMLConnection(&clsRoot, CXMLParams::CN_COMPANY);

	// Make command XML.
	CXMLParams clsXMLCommand;
	CXMLParams::MakeXMLCommand(&clsXMLCommand, szCommand, szModule, szForm);

	clsXML.m_szXMLCommand = clsXMLCommand.GetXML();
	clsXML.m_szXMLData = clsRoot.GetXML();
}


//------------------------------------------------------------------------------
__declspec(dllexport) CString CMS::Replicate(TCHAR ch, int nCount)
{
	CString szValue(ch, nCount);
	return szValue;
}

//------------------------------------------------------------------------------
__declspec(dllexport) CGBLSystemInformation * CMS::GetSI(void)
{
	return g_pGBLSystemInformationCMSDll;
}

//------------------------------------------------------------------------------
__declspec(dllexport) bool CMS::ReturnAvailablePrinters(CMS::PRINTERLIST& clsPrinterVec, CString& szErrorMsg)
{
	static const long		PRINTER_STRUCT_TYPE = 5L;		//type of printer info structure we will request
	DWORD					dwBytesRequired = 0;
	DWORD					dwArrSize = 0;
	std::auto_ptr<TCHAR>	pszPrinterName;
	std::auto_ptr<BYTE>		pbPrinterInfoBuffer;
	DWORD					dwPrinterInfoBufferSize = 0;

	szErrorMsg = _T("");

	// Inquire for the number of bytes to allocate in order to get the printer info
	if (!::EnumPrinters(
		PRINTER_ENUM_LOCAL |
		PRINTER_ENUM_CONNECTIONS,
		pszPrinterName.get(),
		PRINTER_STRUCT_TYPE,
		pbPrinterInfoBuffer.get(),
		dwPrinterInfoBufferSize,
		&dwBytesRequired,
		&dwArrSize))
	{
		//check for ERROR_INSUFFICIENT_BUFFER. we know that we should get this error
		//as we have not allocated a buffer yet; we just wanted to know how big a buffer
		//we needed to allocate. If we get an error other than ERROR_INSUFFICIENT_BUFFER,
		//capture the error message and return a failure status.
		if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		{
			LPVOID lpMsgBuf = NULL;
			if (!::FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				::GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// Default language
				(LPTSTR)&lpMsgBuf,
				0,
				NULL)) return false;

			szErrorMsg = (LPCTSTR)lpMsgBuf;
			::LocalFree(lpMsgBuf);

			return false;
		}
	}

	//allocate the needed amount of memory to get the printer info.
	//we will hold this memory in an auto_ptr so we won't have to worry
	//about deallocating it on exit or if anything goes wrong.
	//the allocated memory will be treated as an array of PRINTER_INFO_5 structures.
	//if the memory cannot be allocated, then quit.
	dwPrinterInfoBufferSize = dwBytesRequired;
	pbPrinterInfoBuffer.reset(new BYTE[dwBytesRequired]);
	if (pbPrinterInfoBuffer.get() == NULL) return false;				//did we get the memory?
	memset(pbPrinterInfoBuffer.get(), 0, dwPrinterInfoBufferSize);		//initialize the memory

	// now get the printer information.
	// also check for error - unlikely though as first call to EnumPrinters succeeded!
	if (!::EnumPrinters(
		PRINTER_ENUM_LOCAL |
		PRINTER_ENUM_CONNECTIONS,
		pszPrinterName.get(),
		PRINTER_STRUCT_TYPE,
		pbPrinterInfoBuffer.get(),
		dwPrinterInfoBufferSize,
		&dwBytesRequired,
		&dwArrSize)) return false;

	// do we have any printers?
	if (dwArrSize == 0) return false;

	// Cast our printer buffer as a pointer to PRINTER_INFO_5 structures
	PRINTER_INFO_5 * pbtPrinterInfo = (PRINTER_INFO_5 *)pbPrinterInfoBuffer.get();

	// Loop adding the printers to the list
	for (DWORD i = 0; i < dwArrSize; i++, pbtPrinterInfo++)
		clsPrinterVec.push_back(pbtPrinterInfo->pPrinterName);

	return true;
}

//------------------------------------------------------------------------------
__declspec(dllexport) bool CMS::ReturnDefaultPrinter(CString& szPrinterName, CString& szErrorMsg)
{
	std::auto_ptr<TCHAR>	pszBuffer;
	DWORD							dwBytesRequired = 0;

	szPrinterName.Empty();
	szErrorMsg.Empty();

	//get the current default printer. first request how much memory is required to acquire the data
	if (!::GetDefaultPrinter(pszBuffer.get(), &dwBytesRequired))
	{
		//check for ERROR_INSUFFICIENT_BUFFER. we know that we should get this error
		//as we have not allocated a buffer yet; we just wanted to know how big a buffer
		//we needed to allocate. if we get an error other than ERROR_INSUFFICIENT_BUFFER,
		//capture the error message and return a failure status.
		if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		{
			LPVOID lpMsgBuf = NULL;
			if (!::FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				::GetLastError(),
				MAKELANGID(LANG_NEUTRAL,
					SUBLANG_DEFAULT),	// Default language
					(LPTSTR)&lpMsgBuf,
				0,
				NULL)) return false;

			szErrorMsg = (LPCTSTR)lpMsgBuf;
			::LocalFree(lpMsgBuf);

			return false;
		}

		//allocate the needed amount of memory to get the printer info.
		//we will hold this memory in an auto_ptr so we won't have to worry
		//about deallocating it on exit or if anything goes wrong.
		pszBuffer.reset(new TCHAR[dwBytesRequired + 1]);
		if (pszBuffer.get() == NULL) return false;				// did we get the memory?
		_tcsnset(pszBuffer.get(), _T('\0'), dwBytesRequired);

		if (!::GetDefaultPrinter(pszBuffer.get(), &dwBytesRequired)) return false;
		szPrinterName = pszBuffer.get();

		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
__declspec(dllexport) bool CMS::IsValidBatchCode(enmMultiBatch eBatchMode, const CString& szBatchCode, const int intBatchType)
{
	bool bRet = false;

	if ((eBatchMode == MBM_Multi) &&
		(!szBatchCode.IsEmpty()) &&
		(szBatchCode != _T("___")) &&
		(intBatchType == GetSI()->CurrentBatch.BatchType))
		bRet = true;
	//KPM.2005.05.12 No need to check for the batch id equal to ___ because if its single, then
	//return true
	else if (eBatchMode == MBM_Single) // && (szBatchCode == _T("___")))
		bRet = true;

	return bRet;
}

//------------------------------------------------------------------------------
__declspec(dllexport) bool CMS::UnpostedTransactionsExist(const CString& szModuleID, const CString& szLocation, const CString& szBatchCode, int intBatchType)
{
	//swiped from CPost. check if there is data to be posted.
	CXMLParams clsXMLData;
	CXMLParams clsXMLCommand;
	CXML			clsXML;

	CXMLParams::MakeXMLCommand(&clsXMLCommand, _T("SPECIALPROCESS"), szModuleID, _T("ISBATCHFILEEMPTY"));

	CXMLParams::AppendCompanyInfo(&clsXMLData);
	CXMLParams::AppendXMLConnection(&clsXMLData, CXMLParams::CN_COMPANY);

	clsXMLData.MakeParam(_T("Batch"), szBatchCode);
	clsXMLData.MakeParam(_T("BatchType"), intBatchType);
	clsXMLData.MakeParam(_T("Location"), szLocation);

	clsXML.m_szXMLCommand = clsXMLCommand.GetXML();
	clsXML.m_szXMLData = clsXMLData.GetXML();

	const CString szReturn(clsXML.SendXML().copy());

	return (szReturn.Find(CMSStrings::XMLTags::NoData) == -1 && szReturn.Find(CMSStrings::XMLTags::Error) == -1);
}


// Function name   : FormatDateFromServer
// Description     : This method is use the format a date string that comes from XML (20002-10-20T0:0000) to 10/20/2002. DTG, 2.16.2004
// Return type     : CString
// Argument        : CString
__declspec(dllexport) CString CMS::FormatDateFromServer(CString szDate)
{
	CString szY, szM, szD;
	//
	szY = szDate.Mid(0, 4);
	szM = szDate.Mid(5, 2);
	szD = szDate.Mid(8, 2);
	return szM + "/" + szD + "/" + szY;
}

__declspec(dllexport) CString CMS::GetClipboardText(CWnd * pWnd)
{
	CString				szText(_T(""));
	const HWND	hWnd = (pWnd && ::IsWindow(pWnd->GetSafeHwnd())) ? pWnd->GetSafeHwnd() : NULL;

	if (::OpenClipboard(hWnd))
	{
		const HGLOBAL	hMem = ::GetClipboardData(CF_UNICODETEXT);
		if (hMem)
		{
			const wchar_t *	pszCode = (const wchar_t *)::GlobalLock(hMem);
			const size_t			nLen = ::wcslen(pszCode) + 1;

			::wcsncpy_s(szText.GetBufferSetLength((int)nLen), nLen, pszCode, nLen - 1);

			::GlobalUnlock(hMem);
			szText.ReleaseBufferSetLength((int)nLen);
		}

		::CloseClipboard();
	}

	return szText;
}

__declspec(dllexport) bool CMS::SetClipboardText(const CString& szText, CWnd * pWnd)
{
	if (szText.IsEmpty()) return true;	//is there something to copy? if not return success.

	bool					bolStatus = false;
	const HWND	hWnd = (pWnd && ::IsWindow(pWnd->GetSafeHwnd())) ? pWnd->GetSafeHwnd() : NULL;
	if (!hWnd) return bolStatus;

	if (::OpenClipboard(hWnd))
	{
		if (!::EmptyClipboard())
		{
			::CloseClipboard();
			return false;
		}

		const ULONG		nSize = szText.GetLength() + sizeof(TCHAR);
		const HGLOBAL	hMem = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, nSize * sizeof(TCHAR));

		if (!hMem)
		{
			::CloseClipboard();
			return false;
		}

		LPTSTR	pszText = (LPTSTR)::GlobalLock(hMem);
		::_tcsncpy_s(pszText, nSize * sizeof(TCHAR), szText.GetString(), (size_t)szText.GetLength());
		::GlobalUnlock(hMem);

		::SetClipboardData(CF_UNICODETEXT, hMem);
		::CloseClipboard();

		bolStatus = true;
	}

	return bolStatus;
}

__declspec(dllexport) CString CMS::GetWindowClassName(CWnd * pWnd)
{
	ASSERT(pWnd);
	return CMS::GetWindowClassName(pWnd->GetSafeHwnd());
}

__declspec(dllexport) CString CMS::GetWindowClassName(HWND hWnd)
{
	ASSERT(::IsWindow(hWnd));

	const int nMax = 256;
	CString szWndClassName(L"");
	const int nLen = ::GetClassName(hWnd, szWndClassName.GetBufferSetLength(nMax), nMax - 1);
	szWndClassName.ReleaseBufferSetLength(nLen);
	return szWndClassName;
}

// Function name   : CCMSPaddedStatic2::WindowProc
// Description     : This class pads the text with two space in front and one space behind to
//                 : make it look like the text padding in the CEdit control.
// Return type     : inline LRESULT
// Argument        : UINT message
// Argument        : WPARAM wParam
// Argument        : LPARAM lParam
LRESULT CCMSPaddedStatic2::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_SETTEXT:

		m_szText = (LPCTSTR)lParam;

		m_szText.Trim(); // eat white.

		m_szText = m_szText + "  "; // end pad. 2 space
		m_szText = "  " + m_szText; // front pad. 2 spaces

		lParam = (LPARAM)(LPCTSTR)m_szText;

		break;
	}

	return CStatic::WindowProc(message, wParam, lParam);
}


LRESULT CCMSCurrencyEdit2::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case EM_SETSEL:

		break;
	case WM_SETFOCUS:

		break;
	}

	return COXCurrencyEdit::WindowProc(message, wParam, lParam);
}
