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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//Global Variables KPM, 10/23/2002

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CString CGBLResources::GetResourceString(UINT uinID, HMODULE hmDLL)
{
	CString szStringValue(_T(""));
	::LoadString(hmDLL, uinID, szStringValue.GetBufferSetLength(5001), 5000);
	szStringValue.ReleaseBuffer();
	return szStringValue;
}

CString CGBLResources::GetResourceString(UINT uinID)
{
	CString szStringValue(_T(""));
	::LoadString(g_pGBLSystemInformationCMSDll->Localization.LanguageDLL, uinID, szStringValue.GetBufferSetLength(5001), 5000);
	szStringValue.ReleaseBuffer();
	return szStringValue;
}

bool CGBLResources::GetResourceBitmap(UINT uinBitmapID, HBITMAP* hbm)
{
	if (g_pGBLSystemInformationCMSDll == NULL)
	{
		//28 August 2003 MLG - Changed the code to get rid of the AfxMessageBox and use the CMSMessagebox.
		CGBLForm::CMSMessageBox(
			g_pGBLSystemInformationCMSDll->ApplicationMainFrame,
			0,
			IDS_COUGAR_MOUNTAIN,
			MB_OK | MB_ICONERROR,
			"Global Variable Pointer has a 0 address");
		return false;
	}

	*hbm = ::LoadBitmap(g_pGBLSystemInformationCMSDll->Localization.LanguageDLL, MAKEINTRESOURCE(uinBitmapID));

	if (hbm == NULL)
	{
		//28 August 2003 MLG - Changed the code to get rid of the AfxMessageBox and use the CMSMessagebox.
		CGBLForm::CMSMessageBox(
			g_pGBLSystemInformationCMSDll->ApplicationMainFrame,
			IDS_FAILED_TO_LOAD_BITMAB,
			IDS_COUGAR_MOUNTAIN,
			MB_OK | MB_ICONERROR);
		return false;
	}
	return true;
}

bool CGBLResources::GetResourceIcon(UINT uinIconID, HICON* hic)
{
	//did the client pass the resource id of the icon to load? if not, suppress the message and return false.
	if (!uinIconID) return false;

	*hic = ::LoadIcon(g_pGBLSystemInformationCMSDll->Localization.LanguageDLL, MAKEINTRESOURCE(uinIconID));

	//sb - 20-dec-2004 - fixed bug here. needed to do *hic instead of hic in the if statement. before it was incorrectly
	//evaluating if hic was passed. should have been checking if something was loaded into the handle hic was pointing at.
	if (*hic == NULL)
	{
		//28 August 2003 MLG - Changed the code to get rid of the AfxMessageBox and use the CMSMessagebox.
		CGBLForm::CMSMessageBox(
			g_pGBLSystemInformationCMSDll->ApplicationMainFrame,
			IDS_FAILED_TO_LOAD_ICON,
			IDS_COUGAR_MOUNTAIN,
			MB_OK | MB_ICONERROR);
		return false;
	}
	return true;
}

bool CGBLResources::GetResourceCursor(UINT uinCursorID, HCURSOR *hcur)
{
	*hcur = ::LoadCursor(g_pGBLSystemInformationCMSDll->Localization.LanguageDLL, MAKEINTRESOURCE(uinCursorID));
	if (hcur == NULL)
	{
		//28 August 2003 MLG - Changed the code to get rid of the AfxMessageBox and use the CMSMessagebox.
		CGBLForm::CMSMessageBox(
			g_pGBLSystemInformationCMSDll->ApplicationMainFrame,
			IDS_FAILED_TO_LOAD_CURSOR,
			IDS_COUGAR_MOUNTAIN,
			MB_OK | MB_ICONERROR);
		return false;
	}
	return true;
}
CString CGBLResources::GetUserLangID()
{
	DWORD dwLocaleLangID = 0;
	dwLocaleLangID = GetUserDefaultLCID();
	TCHAR szLocaleLangID[255] = _T("");
	_itot_s((int)dwLocaleLangID, szLocaleLangID, 255, 16);
	return szLocaleLangID;
}
HINSTANCE CGBLResources::GetResourceHandle()
{
	//#define MAXLENGTH 255
	DWORD dwLocaleLangID = 0;
	dwLocaleLangID = GetUserDefaultLCID();
	TCHAR szLocaleLangID[255] = _T("");
	TCHAR szDllName[500] = _T("");

	//--- converting ID to string
	_itot_s((int)dwLocaleLangID, szLocaleLangID, 255, 16);

	GetModuleFileName(GetModuleHandle(_T("ADOWrapperClasses.dll")), szDllName, sizeof(szDllName) / sizeof(TCHAR));
	CString csDllName = szDllName;

	csDllName = csDllName.Left(csDllName.GetLength() - lstrlen(_T("ADOWrapperClasses.dll")));

	//--- appending strings to get the dll name.
	::lstrcpy(szDllName, _T("CMSLANG"));
	::lstrcat(szDllName, szLocaleLangID);
	::lstrcat(szDllName, _T(".dll"));

	csDllName = csDllName + szDllName;

	HINSTANCE hLangInstance = ::LoadLibrary(csDllName);

	// If the locale specific Language DLL is not found. Then we try to load the English Language DLL
	if (NULL == hLangInstance && NULL == (hLangInstance = ::LoadLibrary(_T("CMSLANG409.dll"))))
	{
		//28 August 2003 MLG - Changed the code to get rid of the AfxMessageBox and use the CMSMessagebox.
		CGBLForm::CMSMessageBox(CMS::GetSI()->ApplicationMainFrame, IDS_LANGUAGE_DLL_NOT_FOUND, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONSTOP);
		// If the ENGLISH Language is also not found then we exit the application
		ExitProcess(0);
	}

	return hLangInstance;
}
