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
#include <atlbase.h>
#include <stdexcept>
#include "..\cmsdll\GBLEnvironment.h"
#include "..\cmsdll\Thinfinity.VirtualUI.h"
#include <functional>
#include <string>
#include "DotNetMethodsDLLImport.h"
#include "../cmsinclude/GBLDataVersion.h"
#include "../CMSDotNet/ArcBill.h"
#include "../CMSDotNet/Yodlee.h"

constexpr auto MAX_BUFLEN = 4000;

/////////////////////////////////////////////////////////////////////////////
// CGBLSystemInformation

/////////////////////////////////////////////////////////////////////////////
// Public Methods for class
CGBLSystemInformation::CGBLSystemInformation()
	: pOutputModule(NULL)
	, m_pIApplicationMainFrame(NULL)
{
	SkipLogin = false;	// skip login when creating New, Modifying or Removing Company

	if (!InitializeLanguageDLL())
	{
		//28 August 2003 MLG - Changed the code to get rid of the AfxMessageBox and use the CMSMessagebox.
		CGBLForm::CMSMessageBox(ApplicationMainFrame, IDS_FAILED_TO_LOAD_RESOURCE_DLL, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONERROR);
		this->SettingsLoadedSuccessfully = false;
		return;
	}

	Drawing.Colors.Black = RGB(0, 0, 0);
	Drawing.Colors.White = RGB(255, 255, 255);
	Drawing.Brushes.Black.CreateSolidBrush(RGB(0, 0, 0));
	Drawing.Bitmaps.CougarHeadSplash = ::LoadBitmap(this->Localization.LanguageDLL, MAKEINTRESOURCE(IDB_COUGARHEADSPLASH));

	//	PBI 28141	06/12/2015	mvt - Changed to larger splash screen bitmap
	Drawing.Bitmaps.CougarHeadSplashLg = ::LoadBitmap(this->Localization.LanguageDLL, MAKEINTRESOURCE(IDB_COUGARHEADSPLASHLG));

	if (!CGBLSystemInformation::LoadFonts())
	{
		this->SettingsLoadedSuccessfully = false;
		return;
	}
	this->SettingsLoadedSuccessfully = true;

	//Create instances of early binding helper classes for com interop.
	//	CreateEarlyBindingObjects();

	//Initialize wait message structure
	WaitMessageDetails.lWaitCount = 0;
	WaitMessageDetails.dwThreadID = 0;
	WaitMessageDetails.szMessage = _T("");
	WaitMessageDetails.unMessageID = 0;

	TemporaryStorage.bCircularError = false;
	TemporaryStorage.bCircularEvent = false;

	//RS - 12/3/2024 - PBI 60840
	TemporaryStorage.d_CalcCreditCardFee = 0.0;
	TemporaryStorage.d_InvoiceCreditCardFee = 0.0;

	if (vuiLicenced())
		VUI->Start();
}

//Added cleanup code to unlock batch, free language lib,
//uninit comLibs and delete com interop objects PGP(10/21/2003)
CGBLSystemInformation::~CGBLSystemInformation()
{
	// begbert 05-22-2006 : added to prevent the post-closure assertions in developer mode, caused by showing the XML viewer when the program is half-closed.
	g_pGBLSystemInformationCMSDll->ApplicationSettings.ShowXMLViewer = false;
	g_pGBLSystemInformationCMSDll->ApplicationSettings.ShowXMLViewerOnError = false;

	CCMSWaitCursor::ExitStatusThread();

	//Unlock batch if locked.
	CurrentBatch.UnlockBatch();

	//Free language library, PGP(10/21/2003)
	if (Localization.LanguageDLL != NULL)
		FreeLibrary(Localization.LanguageDLL);
	//Application not cleaning up - PGP(07/07/2005)
	ClientLockRefresh.TerminateRefreshThread();

	if (ProductAuditAPI != NULL)
		delete ProductAuditAPI;

	//	DeleteEarlyBindingObjects();

	g_pGBLSystemInformationCMSDll = NULL;
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::ParseCommandLineParameters()
{
	try
	{
		CGBLCommandLineInfo clsGBLCommandLineInfo;
		if (!clsGBLCommandLineInfo.CMSParseCommandLine())	return false;
	}
	catch (...)
	{
		return false;
	}

	return true;
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadFonts()
{
	if (this->Drawing.FontsLoaded)
	{
		AfxMessageBox(
			_T("Developer Note from Ken:  I loaded the fonts up on the global creation so take out the call you are making to load the fonts up.  I found a way to get the DC before the application starts."));
		return true;
	}

	try
	{
		//Added DeleteDC calls, PGP(10/15/2003)
		HDC TempDC;
		TempDC = GetDC(NULL);

		LOGFONT lf;						// logical font setting
		ZeroMemory(&lf, sizeof(lf));

		lf.lfHeight = -MulDiv(8, GetDeviceCaps(TempDC, LOGPIXELSY), 72);
		lf.lfWidth = 0;
		lf.lfEscapement = 0;
		lf.lfOrientation = 0;
		lf.lfWeight = FW_NORMAL;
		lf.lfItalic = FALSE;
		lf.lfUnderline = FALSE;
		lf.lfStrikeOut = FALSE;
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf.lfQuality = PROOF_QUALITY;
		lf.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
		_tcscpy_s(lf.lfFaceName, LF_FACESIZE, _T("Arial")); // begbert 02-15-2006 VS8: added _s

		// create the font
		if (!Drawing.Fonts.Normal.CreateFontIndirect(&lf))
		{
			ReleaseDC(NULL, TempDC);
			return false;
		}

		// create the normal underline font
		lf.lfUnderline = TRUE;
		if (!Drawing.Fonts.NormalUnderline.CreateFontIndirect(&lf))
		{
			ReleaseDC(NULL, TempDC);
			return false;
		}

		// create italic font
		lf.lfUnderline = FALSE;
		lf.lfItalic = TRUE;
		if (!Drawing.Fonts.Italic.CreateFontIndirect(&lf))
		{
			ReleaseDC(NULL, TempDC);
			return false;
		}

		lf.lfHeight = -MulDiv(9, GetDeviceCaps(TempDC, LOGPIXELSY), 72);
		if (!Drawing.Fonts.NineItalic.CreateFontIndirect(&lf))
		{
			ReleaseDC(NULL, TempDC);
			return false;
		}

		// create bold
		lf.lfItalic = FALSE;
		lf.lfWeight = FW_BOLD;
		if (!Drawing.Fonts.Bold.CreateFontIndirect(&lf))
		{
			ReleaseDC(NULL, TempDC);
			return false;
		}

		// create the bold underline
		lf.lfUnderline = TRUE;
		if (!Drawing.Fonts.BoldUnderline.CreateFontIndirect(&lf))
		{
			ReleaseDC(NULL, TempDC);
			return false;
		}

		lf.lfHeight = -MulDiv(10, GetDeviceCaps(TempDC, LOGPIXELSY), 72);
		lf.lfUnderline = FALSE;
		if (!Drawing.Fonts.BoldLarge.CreateFontIndirect(&lf))
		{
			ReleaseDC(NULL, TempDC);
			return false;
		}

		//KPM, 1/17/2003 Added a super big font for the module name
		lf.lfHeight = -MulDiv(10, GetDeviceCaps(TempDC, LOGPIXELSY), 60);
		lf.lfUnderline = FALSE;
		if (!Drawing.Fonts.SuperBig.CreateFontIndirect(&lf))
		{
			ReleaseDC(NULL, TempDC);
			return false;
		}

		// larger font for highlighting information
		lf.lfUnderline = TRUE;
		lf.lfHeight = -MulDiv(10, GetDeviceCaps(TempDC, LOGPIXELSY), 72);
		lf.lfPitchAndFamily = VARIABLE_PITCH | FF_ROMAN;
		if (!Drawing.Fonts.BoldUnderlineLarge.CreateFontIndirect(&lf))
		{
			ReleaseDC(NULL, TempDC);
			return false;
		}

		// small font used for about dialog
		lf.lfUnderline = FALSE;
		lf.lfHeight = -MulDiv(7, GetDeviceCaps(TempDC, LOGPIXELSY), 72);
		lf.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
		if (!Drawing.Fonts.Small.CreateFontIndirect(&lf))
		{
			ReleaseDC(NULL, TempDC);
			return false;
		}
		// BK 8/31/04 AP Save Message
		lf.lfHeight = -MulDiv(10, GetDeviceCaps(TempDC, LOGPIXELSY), 40);
		lf.lfUnderline = FALSE;
		if (!Drawing.Fonts.Giant.CreateFontIndirect(&lf))
		{
			ReleaseDC(NULL, TempDC);
			return false;
		}

		//RCG - 06/08/2017 - PBI 39109 - Added new font for Help About
		lf.lfHeight = -MulDiv(7, GetDeviceCaps(TempDC, LOGPIXELSY), 72);
		lf.lfWeight = FW_NORMAL;
		lf.lfItalic = FALSE;
		lf.lfUnderline = FALSE;
		if (!Drawing.Fonts.ArialSeven.CreateFontIndirect(&lf))
		{
			ReleaseDC(NULL, TempDC);
			return false;
		}

		//RCG - 06/08/2017 - PBI 39109 - Added new font for Classic Navigation Splash Screens
		lf.lfHeight = -MulDiv(10, GetDeviceCaps(TempDC, LOGPIXELSY), 72);
		if (!Drawing.Fonts.ArialTen.CreateFontIndirect(&lf))
		{
			ReleaseDC(NULL, TempDC);
			return false;
		}

		//RCG - 06/08/2017 - PBI 39109 - Added new font for Help About
		lf.lfHeight = -MulDiv(12, GetDeviceCaps(TempDC, LOGPIXELSY), 72);
		if (!Drawing.Fonts.ArialTwelve.CreateFontIndirect(&lf))
		{
			ReleaseDC(NULL, TempDC);
			return false;
		}

		//Peter Ringering - 12/05/2003 - To display XML
		LOGFONT lfCourier;
		::ZeroMemory(&lfCourier, sizeof(lfCourier));
		lfCourier.lfHeight = 14; //12-Point
		lfCourier.lfWeight = FW_NORMAL;
		lfCourier.lfUnderline = FALSE;
		lfCourier.lfItalic = FALSE;
		::lstrcpy(lfCourier.lfFaceName, _T("Courier New"));
		Drawing.Fonts.CourierFont.CreateFontIndirect(&lfCourier);

		// InTimeTec: 10/25/2013: HS: Epic 580 D_DEV_AR_Block Charges to Past Due Customers
		// Added a much bold font to display Customers Past Due
		LOGFONT lfArialBlack;
		::ZeroMemory(&lfArialBlack, sizeof(lfArialBlack));
		lfCourier.lfHeight = -MulDiv(20, GetDeviceCaps(TempDC, LOGPIXELSY), 72);
		lfCourier.lfWeight = FW_BOLD;
		lfCourier.lfUnderline = FALSE;
		lfCourier.lfItalic = FALSE;
		::lstrcpy(lfCourier.lfFaceName, _T("Arial Black"));
		Drawing.Fonts.ArialBlack.CreateFontIndirect(&lfCourier);
		// InTimeTec: End

		ReleaseDC(NULL, TempDC);

		//Marlett 10 Symbol font - PGP(12/02/2004)
		if (!Drawing.Fonts.SymbolFonts.Load())
			return false;

		this->Drawing.FontsLoaded = true;
	}
	catch (...)
	{
		return false;
	}
	return true;
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadModuleValuesFromServer()
{
	bool bLoadSA = false;
	CAppSettings* objAppSet = &this->ApplicationSettings;

	// PBI 22186 BK 8/25/14 Added red flag reports, added module in order to show a form in another module
	if (objAppSet->IsCT())
	{
		if (ModulesInstalled.AccountsPayable.IsUsable)
			if (!LoadAPSystemSettings())
				return false;
		if (ModulesInstalled.AccountsReceivable.IsUsable)
			if (!LoadARSystemSettings())
				return false;
		if (ModulesInstalled.BankReconciliation.IsUsable)
			if (!LoadBRSystemSettings())
				return false;
		if (ModulesInstalled.GeneralLedger.IsUsable)
			if (!LoadGLSystemSettings())
				return false;
		if (ModulesInstalled.Inventory.IsUsable)
			if (!LoadINSystemSettings())
				return false;
		if (ModulesInstalled.PurchaseOrder.IsUsable)
			if (!LoadPOSystemSettings())
				return false;
		if (ModulesInstalled.SalesEntry.IsUsable)
			if (!LoadSASystemSettings())
				return false;
		//	PBI 29370	04/17/2015	mvt - Activated Job Cost integration
		if (ModulesInstalled.JobCost.IsUsable && !CompanySettings.IsFund)
			if (!LoadJCSystemSettings())
				return false;
	}
	if (!LoadCTSystemSettings())
		return false;

	if (objAppSet->IsAP())
	{
		if (!LoadAPSystemSettings())
			return false;

		//1-8584 Heather Elston 11/5/2003
		if ((ModuleSettings.APSettings.GLIntegration > 0 &&
			 ModuleSettings.APSettings.GLIntegration < 10) ||
			ModuleSettings.APSettings.GLIntegration > 99)
			if (!LoadGLSystemSettings())
				return false;

		// begbert 05-12-2008 1-29704 : added this, because AP can show the IN Stock form.
		if (ModulesInstalled.Inventory.IsUsable)
			if (!LoadINSystemSettings())
				return false;

		if (ModulesInstalled.BankReconciliation.IsUsable)
			if (!LoadBRSystemSettings())
				return false;
	}

	if (objAppSet->IsAR())
	{
		if (!LoadARSystemSettings())
			return false;

		// 1-23927 BK 5/11/06 if a sales module is installed load sa settings
		if (ModulesInstalled.OrderEntry.IsUsable ||
			ModulesInstalled.PointOfSale.IsUsable)
		{
			if (!LoadSASystemSettings())
				return false;

			bLoadSA = true;
		}

		if (ModulesInstalled.GeneralLedger.IsUsable)
			if ((ModuleSettings.ARSettings.GLIntegration > 0 &&
				 ModuleSettings.ARSettings.GLIntegration < 10) ||
				ModuleSettings.ARSettings.GLIntegration > 99)
				if (!LoadGLSystemSettings())
					return false;

		if (ModulesInstalled.AccountsPayable.IsUsable)
			if (!LoadAPSystemSettings())
				return false;

		if (ModulesInstalled.BankReconciliation.IsUsable)
			if (!LoadBRSystemSettings())
				return false;
	}

	if (objAppSet->IsBR())
	{
		if (!LoadBRSystemSettings())
			return false;

		// begbert 06-12-2007 1-27896 : this is needed for the CAccountControls to autofill in BR.
		if (ModulesInstalled.GeneralLedger.IsUsable)
			if ((ModuleSettings.BRSettings.GLIntegration > 0 &&
				 ModuleSettings.BRSettings.GLIntegration < 10) ||
				ModuleSettings.BRSettings.GLIntegration > 99)
				if (!LoadGLSystemSettings())
					return false;

		if (ModulesInstalled.AccountsPayable.IsUsable)
			if (!LoadAPSystemSettings())
				return false;
	}

	if (objAppSet->IsGL())
		if (!LoadGLSystemSettings())
			return false;

	if (objAppSet->IsIN())
	{
		if (!LoadINSystemSettings())
			return false;

		// BK 5/11/06 if a sales module is installed load sa settings
		if (ModulesInstalled.OrderEntry.IsUsable ||
			ModulesInstalled.PointOfSale.IsUsable)
		{
			if (!LoadSASystemSettings())
				return false;

			bLoadSA = true;
		}

		//Peter Ringering - 10/29/2003 - 1-8437
		if (bLoadSA ||
			(ModuleSettings.INSettings.GLIntegration > 0 &&
			 ModuleSettings.INSettings.GLIntegration < 10) ||
			ModuleSettings.INSettings.GLIntegration > 99)
		{
			if (!LoadGLSystemSettings())
				return false;
		}

		// 1-13535/1-13536 BK 9/2/04 added AP system settings for Add-the-the-Fly AP Vendor
		if (ModulesInstalled.AccountsPayable.IsUsable)
		{
			if (!LoadAPSystemSettings())
				return false;
		}
	}

	if (objAppSet->IsJC())
	{
		if (!LoadJCSystemSettings())
			return false;
	}

	if (objAppSet->IsPO())
	{
		if (!LoadPOSystemSettings())
			return false;

		if (ModulesInstalled.AccountsPayable.IsUsable)
			if (!LoadAPSystemSettings())
				return false;
		// 1-31949 BK 2/13/09 load AR system information for Drop Ship Customer
		if (ModulesInstalled.AccountsReceivable.IsUsable)
			if (!LoadARSystemSettings())
				return false;

		if (ModulesInstalled.Inventory.IsUsable)
			if (!LoadINSystemSettings())
				return false;

		if (ModulesInstalled.GeneralLedger.IsUsable)
			if (!LoadGLSystemSettings())
				return false;

		//	PBI 29370	04/17/2015	mvt - Activated Job Cost integration
		if (ModulesInstalled.JobCost.IsUsable &&
			!CompanySettings.IsFund)
			if (!LoadJCSystemSettings())
				return false;
	}

	if (objAppSet->IsPR())
		if (!LoadPRSystemSettings())
			return false;

	if (objAppSet->IsSA())
	{
		if (!LoadSASystemSettings()) return false;
		if (ModulesInstalled.GeneralLedger.IsUsable)
			if (!LoadGLSystemSettings()) return false;
		if ((ModuleSettings.SASettings.ARIntegration > 0 && ModuleSettings.SASettings.ARIntegration < 10) || ModuleSettings.SASettings.ARIntegration > 99)
			if (!LoadARSystemSettings()) return false;
		if ((ModuleSettings.SASettings.INIntegration > 0 && ModuleSettings.SASettings.INIntegration < 10) || ModuleSettings.SASettings.INIntegration > 99)
			if (!LoadINSystemSettings()) return false;
		// BK 5/11/06 if a sales module is installed load sa settings
		if ((ModuleSettings.SASettings.APIntegration > 0 && ModuleSettings.SASettings.APIntegration < 10) || ModuleSettings.SASettings.APIntegration > 99)
			if (!LoadAPSystemSettings()) return false;

		// Sales doesn't "officially" integrate to PO, but we need to have the PO system settings for it anyway.
		if (!LoadPOSystemSettings()) return false;
	}

	objAppSet = NULL;
	delete objAppSet;

	return true;
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadSystemGlobals(void)
{
	//Set the global variable
	g_pGBLSystemInformationCMSDll = this;

	//Setup all the default values
	InitializeGlobalVariables();

	if (!ParseCommandLineParameters())
		return false;

	this->ApplicationSettings.ShowXMLViewer = this->CommandLineSettings.PO_XMLViewer;
	this->ApplicationSettings.ShowXMLViewerOnError = this->CommandLineSettings.PO_XMLViewerOnError;

	if (!LoadColors())
	{
		CGBLForm::CMSMessageBox(ApplicationMainFrame,
								IDS_FAILED_TO_LOAD_COLORS,
								IDS_COUGAR_MOUNTAIN,
								MB_OK | MB_ICONERROR);
		return false;
	}

	if (!LoadIcons())
	{
		CGBLForm::CMSMessageBox(ApplicationMainFrame,
								IDS_FAILED_TO_LOAD_ICONS,
								IDS_COUGAR_MOUNTAIN,
								MB_OK | MB_ICONERROR);
		return false;
	}

	if (!LoadBitmaps())
	{
		CGBLForm::CMSMessageBox(ApplicationMainFrame,
								IDS_FAILED_TO_LOAD_BITMAPS,
								IDS_COUGAR_MOUNTAIN,
								MB_OK | MB_ICONERROR);
		return false;
	}

	if (!LoadCursors())
	{
		CGBLForm::CMSMessageBox(ApplicationMainFrame,
								IDS_FAILED_TO_LOAD_CURSORS,
								IDS_COUGAR_MOUNTAIN,
								MB_OK | MB_ICONERROR);
		return false;
	}

	if (!LoadBrushes())
	{
		CGBLForm::CMSMessageBox(ApplicationMainFrame,
								IDS_FAILED_TO_LOAD_BRUSHES,
								IDS_COUGAR_MOUNTAIN,
								MB_OK | MB_ICONERROR);
		return false;
	}

	if (!LoadPens())
	{
		CGBLForm::CMSMessageBox(ApplicationMainFrame,
								IDS_FAILED_TO_LOAD_PENS,
								IDS_COUGAR_MOUNTAIN,
								MB_OK | MB_ICONERROR);
		return false;
	}

	if (!LoadPeriodDates())
	{
		CGBLForm::CMSMessageBox(ApplicationMainFrame,
								IDS_FAILED_TO_LOAD_PERIOD_DATE,
								IDS_COUGAR_MOUNTAIN,
								MB_OK | MB_ICONERROR);
		return false;
	}

	if (!LoadSegmentSetup())
	{
		CGBLForm::CMSMessageBox(ApplicationMainFrame,
								IDS_FAILED_TO_LOAD_SEGMENT_SETUP,
								IDS_COUGAR_MOUNTAIN,
								MB_OK | MB_ICONERROR);
		return false;
	}

	if (!LoadTipOfTheDay())
	{
		CGBLForm::CMSMessageBox(ApplicationMainFrame,
								IDS_FAILED_TO_LOAD_TIP_OF_THE_DAY,
								IDS_COUGAR_MOUNTAIN,
								MB_OK | MB_ICONERROR);
		return false;
	}
	//Get the operating system information
	//SystemData.NTKernelBased = CGBLSystemState::IsNTKernelOS();

	//OS Values
	//	OPS_WIN95
	//	OPS_WIN95SP1
	//	OPS_WIN95REL2
	//	OPS_WIN98
	//	OPS_WIN98SP1
	//	OPS_WIN98SE
	//	OPS_WINME
	//	OPS_NT351
	//	OPS_NT4
	//	OPS_WIN2000
	//	OPS_WINXP

	CString cBuffer(_T(""));
	DWORD	dwLength = MAX_COMPUTERNAME_LENGTH + sizeof(wchar_t);
	LPDWORD	lpdwLength = &dwLength;

	//SystemData.OperatingSystem = CGBLSystemState::GetOperatingSystem();
	BOOL bComputerNameError = GetComputerName(cBuffer.GetBufferSetLength((int)dwLength), lpdwLength);
	cBuffer.ReleaseBuffer();

	if (!bComputerNameError)
	{
		//TODO: Add computer name error handling
			//AfxMessageBox(_T("Failed to retrieve Computer Name")) ;
	}

	SystemData.ComputerName = cBuffer;
	SystemData.ServerName.Empty();

	//Load the system paths
	ApplicationPaths.szApplicationPath = CGBLForm::GetPath();
	ApplicationPaths.szHelpPath = ApplicationPaths.szApplicationPath + _T("Help\\");
	ApplicationPaths.szReportPath = ApplicationPaths.szApplicationPath + _T("Reports\\");

	CheckForCommandLineOverrides();	//PTR.07.29.2005

	//RCG - 09/28/2021 - PBI 57710 - Get ArcTime URL selection from the registry
	CXMLParams ArcTimeParam, ArcBillParam;
	if (CGBLRegistry::GetKey(&ArcTimeParam, _T("ArcTimeURL"), false))
		//Set system variable
		UserInformation.ArcTimeURL = ArcTimeParam.GetParamInt("ArcTimeURL", false);

	//RCG - 09/28/2021 - PBI 57710 - Get ArcBill URL selection from the registry
	ArcTimeParam.Clear();
	if (CGBLRegistry::GetKey(&ArcBillParam, _T("ArcBillURL"), false))
		//Set system variable
		UserInformation.ArcBillURL = ArcBillParam.GetParamInt("ArcBillURL", false);
	ArcBillParam.Clear();

	ApplicationSettings.ArcBillUsable = CMSArcBill::SiteAvailable(UserInformation.ArcBillURL);
	ApplicationSettings.YodleeUsable = CMSYodlee::YodleeAPIUsable();

	return true;
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::SetupToolTip(CWnd* pParent)
{
	this->ToolTip.InitializeToolTip();

	return true;
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadPeriodDates(void)
{
	COleDateTime time(2005, 01, 01, 17, 30, 0);
	int			 Year;
	time.Format(_T("mm/dd/yyyy"));
	time = COleDateTime::GetCurrentTime();
	Year = time.GetYear();
	time.SetDate(Year - 1, 12, 31);
	FiscalCalendar.FiscalCalendar[0].StartDate = time;
	time.SetDate(Year - 1, 12, 31);
	FiscalCalendar.FiscalCalendar[0].EndDate = time;
	time.SetDate(Year, 01, 01);
	for (int nCntr = 0; nCntr < 5; nCntr++)
	{
		FiscalCalendar.FiscalCalendar[1 + (nCntr * 13)].StartDate = time;
		time.SetDate(Year + nCntr, 01, 31);
		FiscalCalendar.FiscalCalendar[1 + (nCntr * 13)].EndDate = time;
		time.SetDate(Year + nCntr, 02, 01);
		FiscalCalendar.FiscalCalendar[2 + (nCntr * 13)].StartDate = time;
		if ((Year + nCntr) % 4 == 0)
		{
			if ((Year + nCntr) % 100 == 0)
			{
				if ((Year + nCntr) % 400 == 0)
					time.SetDate(Year + nCntr, 02, 29);
				else
					time.SetDate(Year + nCntr, 02, 28);
			}
			else
				time.SetDate(Year + nCntr, 02, 29);
		}
		else
			time.SetDate(Year + nCntr, 02, 28);

		FiscalCalendar.FiscalCalendar[2 + (nCntr * 13)].EndDate = time;
		time.SetDate(Year + nCntr, 03, 01);
		FiscalCalendar.FiscalCalendar[3 + (nCntr * 13)].StartDate = time;
		time.SetDate(Year + nCntr, 03, 31);
		FiscalCalendar.FiscalCalendar[3 + (nCntr * 13)].EndDate = time;
		time.SetDate(Year + nCntr, 04, 01);
		FiscalCalendar.FiscalCalendar[4 + (nCntr * 13)].StartDate = time;
		time.SetDate(Year + nCntr, 04, 30);
		FiscalCalendar.FiscalCalendar[4 + (nCntr * 13)].EndDate = time;
		time.SetDate(Year + nCntr, 05, 01);
		FiscalCalendar.FiscalCalendar[5 + (nCntr * 13)].StartDate = time;
		time.SetDate(Year + nCntr, 05, 31);
		FiscalCalendar.FiscalCalendar[5 + (nCntr * 13)].EndDate = time;
		time.SetDate(Year + nCntr, 06, 01);
		FiscalCalendar.FiscalCalendar[6 + (nCntr * 13)].StartDate = time;
		time.SetDate(Year + nCntr, 06, 30);
		FiscalCalendar.FiscalCalendar[6 + (nCntr * 13)].EndDate = time;
		time.SetDate(Year + nCntr, 07, 01);
		FiscalCalendar.FiscalCalendar[7 + (nCntr * 13)].StartDate = time;
		time.SetDate(Year + nCntr, 07, 31);
		FiscalCalendar.FiscalCalendar[7 + (nCntr * 13)].EndDate = time;
		time.SetDate(Year + nCntr, 8, 01);
		FiscalCalendar.FiscalCalendar[8 + (nCntr * 13)].StartDate = time;
		time.SetDate(Year + nCntr, 8, 31);
		FiscalCalendar.FiscalCalendar[8 + (nCntr * 13)].EndDate = time;
		time.SetDate(Year + nCntr, 9, 01);
		FiscalCalendar.FiscalCalendar[9 + (nCntr * 13)].StartDate = time;
		time.SetDate(Year + nCntr, 9, 30);
		FiscalCalendar.FiscalCalendar[9 + (nCntr * 13)].EndDate = time;
		time.SetDate(Year + nCntr, 10, 01);
		FiscalCalendar.FiscalCalendar[10 + (nCntr * 13)].StartDate = time;
		time.SetDate(Year + nCntr, 10, 31);
		FiscalCalendar.FiscalCalendar[10 + (nCntr * 13)].EndDate = time;
		time.SetDate(Year + nCntr, 11, 01);
		FiscalCalendar.FiscalCalendar[11 + (nCntr * 13)].StartDate = time;
		time.SetDate(Year + nCntr, 11, 30);
		FiscalCalendar.FiscalCalendar[11 + (nCntr * 13)].EndDate = time;
		time.SetDate(Year + nCntr, 12, 01);
		FiscalCalendar.FiscalCalendar[12 + (nCntr * 13)].StartDate = time;
		time.SetDate(Year + nCntr, 12, 31);
		FiscalCalendar.FiscalCalendar[12 + (nCntr * 13)].EndDate = time;
	}

	FiscalCalendar.NumberOfPeriods = 12;
	FiscalCalendar.BlockPeriod = 0;
	FiscalCalendar.BlockDate = FiscalCalendar.FiscalCalendar[FiscalCalendar.BlockPeriod].EndDate;
	FiscalCalendar.CurrentPeriod = 1;
	FiscalCalendar.StartDate = FiscalCalendar.FiscalCalendar[1].StartDate;
	FiscalCalendar.EndDate = FiscalCalendar.FiscalCalendar[52 + FiscalCalendar.NumberOfPeriods].EndDate;

	return true;
}

//-------------------------------------------------------------------
// set account segment defaults
bool CGBLSystemInformation::LoadSegmentSetup(void)
{
	SegmentSetup.FundSegment = 1;
	SegmentSetup.NumberOfSegments = 1;
	SegmentSetup.Segments[0].Length = 50;
	SegmentSetup.Segments[1].Length = 0;
	SegmentSetup.Segments[2].Length = 0;
	SegmentSetup.Segments[3].Length = 0;
	SegmentSetup.Segments[4].Length = 0;
	SegmentSetup.Segments[5].Length = 0;
	SegmentSetup.TotalAccountLength = 50;
	SegmentSetup.SeparationCharacter = _T(".");

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Private Methods for class
void CGBLSystemInformation::InitializeGlobalVariables()
{
	ResetModulesInfo();

	//Command Line Parameter Values
	CommandLineSettings.PL_PrimaryServer.Empty();
	CommandLineSettings.PL_PrimaryDatabase.Empty();
	CommandLineSettings.PL_PrimaryDBType.Empty();
	CommandLineSettings.PL_PrimaryUserName.Empty();
	CommandLineSettings.PL_PrimaryPassword.Empty();
	CommandLineSettings.PL_PrimaryServerTimeout.Empty();

	CommandLineSettings.AL_ArchiveServer.Empty();
	CommandLineSettings.AL_ArchiveDatabase.Empty();
	CommandLineSettings.AL_PrimaryDBType.Empty();
	CommandLineSettings.AL_ArchiveUserName.Empty();
	CommandLineSettings.AL_ArchivePassword.Empty();
	CommandLineSettings.AL_ArchiveServerTimeout.Empty();

	CommandLineSettings.UL_CompanyUser.Empty();
	CommandLineSettings.UL_CompanyPassword.Empty();
	CommandLineSettings.DC_DefaultCompany.Empty();
	CommandLineSettings.LO_BackgroundLogin = false;

	CommandLineSettings.LO_ForcePrimaryServerLogin = false;
	CommandLineSettings.LO_ForceUserLogin = false;
	CommandLineSettings.LO_ForceCompanySelection = false;
	CommandLineSettings.RO_WritePrimaryDatabaseSettingToRegistry = false;
	CommandLineSettings.RO_WriteUserSettingsToRegistry = false;
	CommandLineSettings.RO_WriteDefaultCompanyToRegistry = false;
	CommandLineSettings.PO_Developer = 0;
	CommandLineSettings.PO_XMLViewer = false;
	CommandLineSettings.PO_XMLViewerOnError = false;
	CommandLineSettings.PO_AssertOnError = false;
	CommandLineSettings.PO_OrderEntry = false;
	CommandLineSettings.PO_PointOfSale = false;
	CommandLineSettings.PO_OpenedFromAnotherModule = false;
	CommandLineSettings.PO_MemoryCheckpoints = false;
	CommandLineSettings.GIVEX_ShowInfo = false;
	CommandLineSettings.CT_EnableNativeCurrencyCombo = false;	// begbert 08-30-2007

	//Company Server Information
	Servers.CompanyServer.Initialize();

	//Archive Server Information
	Servers.ArchiveServer.Initialize();

	//Company Information
	CompanySettings.ID.Empty();
	CompanySettings.Name.Empty();
	CompanySettings.BillingInformation.Address1.Empty();
	CompanySettings.BillingInformation.Address2.Empty();
	CompanySettings.BillingInformation.City.Empty();
	CompanySettings.BillingInformation.State.Empty();
	CompanySettings.BillingInformation.PostalCode.Empty();
	CompanySettings.BillingInformation.Province.Empty();
	CompanySettings.BillingInformation.Country.Empty();
	CompanySettings.BillingInformation.Contact.Empty();
	CompanySettings.ShippingInformation.Address1.Empty();
	CompanySettings.ShippingInformation.Address2.Empty();
	CompanySettings.ShippingInformation.City.Empty();
	CompanySettings.ShippingInformation.State.Empty();
	CompanySettings.ShippingInformation.PostalCode.Empty();
	CompanySettings.ShippingInformation.Province.Empty();
	CompanySettings.ShippingInformation.Country.Empty();
	CompanySettings.ShippingInformation.Contact.Empty();
	CompanySettings.Phone.Empty();
	CompanySettings.Fax.Empty();
	CompanySettings.Extension.Empty();
	CompanySettings.EMail.Empty();
	CompanySettings.Website.Empty();
	CompanySettings.DefaultLocation.Empty();
	CompanySettings.DataVersion.Empty();
	CompanySettings.NewCompanyFiscalCalendarNotCreated = false;
	CompanySettings.IsFund = false;
	// PBI 17783 BK 3/17/14 system variables for Merchantware Genius
	CompanySettings.MerchantWare.AccountName.Empty();
	CompanySettings.MerchantWare.SiteID.Empty();
	CompanySettings.MerchantWare.Password.Empty();
	CompanySettings.MerchantWare.DBA.Empty();
	CompanySettings.MerchantWare.TerminalID.Empty();			// begbert 11-16-2015 PBI 31304 : added this
	CompanySettings.MerchantWare.IP_Address.Empty();
	CompanySettings.MerchantWare.UseGeniusForPreauths = false;	// begbert 05-07-2014 PBI 19565 : added this

	UserInformation.Empty();

	CAppSettings* objAppSet = &this->ApplicationSettings;
	if (objAppSet->IsAP())
		ModuleSettings.CurrentSettings = &ModuleSettings.APSettings;
	if (objAppSet->IsAR())
		ModuleSettings.CurrentSettings = &ModuleSettings.ARSettings;
	if (objAppSet->IsBR())
		ModuleSettings.CurrentSettings = &ModuleSettings.BRSettings;
	if (objAppSet->IsGL())
		ModuleSettings.CurrentSettings = &ModuleSettings.GLSettings;
	if (objAppSet->IsIN())
		ModuleSettings.CurrentSettings = &ModuleSettings.INSettings;
	if (objAppSet->IsJC())
		ModuleSettings.CurrentSettings = &ModuleSettings.JCSettings;
	if (objAppSet->IsPO())
		ModuleSettings.CurrentSettings = &ModuleSettings.POSettings;
	if (objAppSet->IsPR())
		ModuleSettings.CurrentSettings = &ModuleSettings.PRSettings;
	if (objAppSet->IsSA())
		// JKC Test
		ModuleSettings.CurrentSettings = &ModuleSettings.SASettings;
	// BK 10/10/03 returning 0 until we decide how to make this work with OE and POS
	//ModuleSettings.CurrentSettings = 0; //&ModuleSettings.SASettings;
	if (objAppSet->IsCT())
		ModuleSettings.CurrentSettings = &ModuleSettings.CTSettings;
	if (objAppSet->IsBI())
		ModuleSettings.CurrentSettings = &ModuleSettings.CTSettings;	// the Business Intelligence module piggybacks the Controller settings.

	//KPM.2005.03.25 Load the session info
	objAppSet->SessionName = CGBLEnvironment::GetSessionName();
	objAppSet->UserName = CGBLEnvironment::GetUserName();

	objAppSet = NULL;

	//Don't tell it to cancel, the splash screen will set this to true if the user presses
	//the cancel button on the splash screen
	CancelApplicationLoad = false;

	////Added for a listing of what the registry said the user installed
	//InitializeInstallInformation(InstallInformation);
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadPens()
{
	try
	{
		Drawing.Pens.Black.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
		Drawing.Pens.Yellow.CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
		Drawing.Pens.DkYellow.CreatePen(PS_SOLID, 1, RGB(128, 128, 0));
		Drawing.Pens.Red.CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		Drawing.Pens.DkRed.CreatePen(PS_SOLID, 1, RGB(128, 0, 0));
		Drawing.Pens.Magenta.CreatePen(PS_SOLID, 1, RGB(255, 0, 255));
		Drawing.Pens.DkMagenta.CreatePen(PS_SOLID, 1, RGB(128, 0, 128));
		Drawing.Pens.Blue.CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
		Drawing.Pens.DkBlue.CreatePen(PS_SOLID, 1, RGB(0, 0, 128));
		Drawing.Pens.Cyan.CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
		Drawing.Pens.DkCyan.CreatePen(PS_SOLID, 1, RGB(0, 128, 128));
		Drawing.Pens.Green.CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
		Drawing.Pens.DkGreen.CreatePen(PS_SOLID, 1, RGB(0, 128, 0));
		Drawing.Pens.Gray.CreatePen(PS_SOLID, 1, RGB(192, 192, 192));
		Drawing.Pens.DkGray.CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
		Drawing.Pens.White.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
		Drawing.Pens.Cream.CreatePen(PS_SOLID, 1, RGB(255, 200, 150));
		Drawing.Pens.Scarlet.CreatePen(PS_SOLID, 1, RGB(200, 100, 100));

		// 32 bit Colors
		Drawing.Pens.LtYellow.CreatePen(PS_SOLID, 1, RGB(255, 245, 226));
		Drawing.Pens.GoldenRod.CreatePen(PS_SOLID, 1, RGB(201, 200, 114));
		Drawing.Pens.LtGreen.CreatePen(PS_SOLID, 1, RGB(194, 226, 217));
		Drawing.Pens.LtBlue.CreatePen(PS_SOLID, 1, RGB(166, 202, 240));
		Drawing.Pens.MedGray.CreatePen(PS_SOLID, 1, RGB(160, 160, 164));
		Drawing.Pens.MdGreen.CreatePen(PS_SOLID, 1, RGB(108, 160, 141));
		Drawing.Pens.Teal.CreatePen(PS_SOLID, 1, RGB(0, 92, 126));
		Drawing.Pens.OrangeAccent.CreatePen(PS_SOLID, 1, RGB(249, 172, 70));

		// system Colors
		//COLOR_3DDKSHADOW Dark shadow for three-dimensional display elements.
		Drawing.Pens.System._3DDarkShadow.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DDKSHADOW));

		//COLOR_3DFACE, COLOR_BTNFACE Face color for three-dimensional display elements and for dialog box backgrounds.
		Drawing.Pens.System.ButtonFace.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DFACE));

		//COLOR_3DHILIGHT, COLOR_3DHIGHLIGHT, COLOR_BTNHILIGHT, COLOR_BTNHIGHLIGHT Highlight color for three-dimensional display elements (for edges facing the light source.)
		Drawing.Pens.System.ButtonHighlight.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_BTNHIGHLIGHT));

		//COLOR_3DLIGHT Light color for three-dimensional display elements (for edges facing the light source.)
		Drawing.Pens.System._3DLight.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DLIGHT));

		//COLOR_3DSHADOW, COLOR_BTNSHADOW Shadow color for three-dimensional display elements (for edges facing away from the light source).
		Drawing.Pens.System.ButtonShadow.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DSHADOW));

		//COLOR_ACTIVEBORDER Active window border.
		Drawing.Pens.System.ActiveBorder.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_ACTIVEBORDER));

		//COLOR_ACTIVECAPTION Active window title bar. Windows 98/Me, Windows 2000 or later: Specifies the left side color in the color gradient of an active window's title bar if the gradient effect is enabled.
		Drawing.Pens.System.ActiveCaption.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_ACTIVECAPTION));

		//COLOR_APPWORKSPACE Background color of multiple document interface (MDI) applications.
		Drawing.Pens.System.ApplicationWorkspace.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_APPWORKSPACE));

		//COLOR_BACKGROUND, COLOR_DESKTOP Desktop.
		Drawing.Pens.System.Background.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_BACKGROUND));
		Drawing.Pens.System.Desktop.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_DESKTOP));

		//COLOR_BTNTEXT Text on push buttons.
		Drawing.Pens.System.ButtonText.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_BTNTEXT));

		//COLOR_CAPTIONTEXT Text in caption, size box, and scroll bar arrow box.
		Drawing.Pens.System.CaptionText.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_CAPTIONTEXT));

		//COLOR_GRAYTEXT Grayed (disabled) text. This color is set to 0 if the current display driver does not support a solid gray color.
		Drawing.Pens.System.GrayText.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_GRAYTEXT));

		//COLOR_HIGHLIGHT Item(s) selected in a control.
		Drawing.Pens.System.Highlight.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_HIGHLIGHT));

		//COLOR_HIGHLIGHTTEXT Text of item(s) selected in a control.
		Drawing.Pens.System.HighlightText.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_HIGHLIGHTTEXT));

		//COLOR_INACTIVEBORDER Inactive window border.
		Drawing.Pens.System.InactiveBorder.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_INACTIVEBORDER));

		//COLOR_INACTIVECAPTION Inactive window caption. Windows 98/Me, Windows 2000 or later: Specifies the left side color in the color gradient of an inactive window's title bar if the gradient effect is enabled.
		Drawing.Pens.System.InactiveCaption.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_INACTIVECAPTION));

		//COLOR_INACTIVECAPTIONTEXT Color of text in an inactive caption.
		Drawing.Pens.System.InactiveCaptionText.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_INACTIVECAPTIONTEXT));

		//COLOR_INFOBK Background color for tooltip controls.
		Drawing.Pens.System.ToolTipBackground.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_INFOBK));

		//COLOR_INFOTEXT Text color for tooltip controls.
		Drawing.Pens.System.ToolTipText.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_INFOTEXT));

		//COLOR_MENU Menu background.
		Drawing.Pens.System.Menu.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_MENU));

		//COLOR_MENUTEXT Text in menus.
		Drawing.Pens.System.MenuText.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_MENUTEXT));

		//COLOR_SCROLLBAR Scroll bar gray area.
		Drawing.Pens.System.Scrollbar.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_SCROLLBAR));

		//COLOR_WINDOW Window background.
		Drawing.Pens.System.Window.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_WINDOW));

		//COLOR_WINDOWFRAME Window frame.
		Drawing.Pens.System.WindowFrame.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_WINDOWFRAME));

		//COLOR_WINDOWTEXT
		Drawing.Pens.System.WindowText.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_WINDOWTEXT));
	}
	catch (...)
	{
		return false;
	}

	return true;
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadBrushes()
{
	try
	{
		Drawing.Brushes.Yellow.CreateSolidBrush(RGB(255, 255, 0));
		Drawing.Brushes.DkYellow.CreateSolidBrush(RGB(128, 128, 0));
		Drawing.Brushes.Red.CreateSolidBrush(RGB(255, 0, 0));
		Drawing.Brushes.DkRed.CreateSolidBrush(RGB(128, 0, 0));
		Drawing.Brushes.Magenta.CreateSolidBrush(RGB(255, 0, 255));
		Drawing.Brushes.DkMagenta.CreateSolidBrush(RGB(128, 0, 128));
		Drawing.Brushes.Blue.CreateSolidBrush(RGB(0, 0, 255));
		Drawing.Brushes.DkBlue.CreateSolidBrush(RGB(0, 0, 128));
		Drawing.Brushes.Cyan.CreateSolidBrush(RGB(0, 255, 255));
		Drawing.Brushes.DkCyan.CreateSolidBrush(RGB(0, 128, 128));
		Drawing.Brushes.Green.CreateSolidBrush(RGB(0, 255, 0));
		Drawing.Brushes.DkGreen.CreateSolidBrush(RGB(0, 128, 0));
		Drawing.Brushes.Gray.CreateSolidBrush(RGB(192, 192, 192));
		Drawing.Brushes.DkGray.CreateSolidBrush(RGB(128, 128, 128));
		Drawing.Brushes.White.CreateSolidBrush(RGB(255, 255, 255));
		Drawing.Brushes.Cream.CreateSolidBrush(RGB(255, 200, 150));
		Drawing.Brushes.Scarlet.CreateSolidBrush(RGB(200, 100, 100));
		//KPM, changed purple, found the right color numbers from print key
		Drawing.Brushes.Purple.CreateSolidBrush(RGB(128, 0, 128));

		// 32 bit Colors
		Drawing.Brushes.LtYellow.CreateSolidBrush(RGB(255, 245, 226));
		Drawing.Brushes.GoldenRod.CreateSolidBrush(RGB(201, 200, 114));
		Drawing.Brushes.LtGreen.CreateSolidBrush(RGB(194, 226, 217));
		Drawing.Brushes.LtBlue.CreateSolidBrush(RGB(166, 202, 240));
		Drawing.Brushes.MedGray.CreateSolidBrush(RGB(160, 160, 164));
		Drawing.Brushes.MdGreen.CreateSolidBrush(RGB(108, 160, 141));
		Drawing.Brushes.Teal.CreateSolidBrush(RGB(0, 92, 126));
		Drawing.Brushes.OrangeAccent.CreateSolidBrush(RGB(249, 172, 70));
		//RCG - 06/08/2017 - PBI 39109 - Added new brush colors
		Drawing.Brushes.TreePoppy.CreateSolidBrush(RGB(247, 148, 31));	//Orangish
		Drawing.Brushes.RadicalRed.CreateSolidBrush(RGB(255, 64, 89));	//Pinkish
		Drawing.Brushes.MineShaft.CreateSolidBrush(RGB(45, 45, 45));	//Dark Grayish

		// system Colors
		//COLOR_3DDKSHADOW Dark shadow for three-dimensional display elements.
		Drawing.Brushes.System._3DDarkShadow.CreateSolidBrush(::GetSysColor(COLOR_3DDKSHADOW));

		//COLOR_3DFACE, COLOR_BTNFACE Face color for three-dimensional display elements and for dialog box backgrounds.
		Drawing.Brushes.System.ButtonFace.CreateSolidBrush(::GetSysColor(COLOR_3DFACE));

		//COLOR_3DHILIGHT, COLOR_3DHIGHLIGHT, COLOR_BTNHILIGHT, COLOR_BTNHIGHLIGHT Highlight color for three-dimensional display elements (for edges facing the light source.)
		Drawing.Brushes.System.ButtonHighlight.CreateSolidBrush(::GetSysColor(COLOR_BTNHIGHLIGHT));

		//COLOR_3DLIGHT Light color for three-dimensional display elements (for edges facing the light source.)
		Drawing.Brushes.System._3DLight.CreateSolidBrush(::GetSysColor(COLOR_3DLIGHT));

		//COLOR_3DSHADOW, COLOR_BTNSHADOW Shadow color for three-dimensional display elements (for edges facing away from the light source).
		Drawing.Brushes.System.ButtonShadow.CreateSolidBrush(::GetSysColor(COLOR_3DSHADOW));

		//COLOR_ACTIVEBORDER Active window border.
		Drawing.Brushes.System.ActiveBorder.CreateSolidBrush(::GetSysColor(COLOR_ACTIVEBORDER));

		//COLOR_ACTIVECAPTION Active window title bar. Windows 98/Me, Windows 2000 or later: Specifies the left side color in the color gradient of an active window's title bar if the gradient effect is enabled.
		Drawing.Brushes.System.ActiveCaption.CreateSolidBrush(::GetSysColor(COLOR_ACTIVECAPTION));

		//COLOR_APPWORKSPACE Background color of multiple document interface (MDI) applications.
		Drawing.Brushes.System.ApplicationWorkspace.CreateSolidBrush(::GetSysColor(COLOR_APPWORKSPACE));

		//COLOR_BACKGROUND, COLOR_DESKTOP Desktop.
		Drawing.Brushes.System.Background.CreateSolidBrush(::GetSysColor(COLOR_BACKGROUND));
		Drawing.Brushes.System.Desktop.CreateSolidBrush(::GetSysColor(COLOR_DESKTOP));

		//COLOR_BTNTEXT Text on push buttons.
		Drawing.Brushes.System.ButtonText.CreateSolidBrush(::GetSysColor(COLOR_BTNTEXT));

		//COLOR_CAPTIONTEXT Text in caption, size box, and scroll bar arrow box.
		Drawing.Brushes.System.CaptionText.CreateSolidBrush(::GetSysColor(COLOR_CAPTIONTEXT));

		//COLOR_GRAYTEXT Grayed (disabled) text. This color is set to 0 if the current display driver does not support a solid gray color.
		Drawing.Brushes.System.GrayText.CreateSolidBrush(::GetSysColor(COLOR_GRAYTEXT));

		//COLOR_HIGHLIGHT Item(s) selected in a control.
		Drawing.Brushes.System.Highlight.CreateSolidBrush(::GetSysColor(COLOR_HIGHLIGHT));

		//COLOR_HIGHLIGHTTEXT Text of item(s) selected in a control.
		Drawing.Brushes.System.HighlightText.CreateSolidBrush(::GetSysColor(COLOR_HIGHLIGHTTEXT));

		//COLOR_INACTIVEBORDER Inactive window border.
		Drawing.Brushes.System.InactiveBorder.CreateSolidBrush(::GetSysColor(COLOR_INACTIVEBORDER));

		//COLOR_INACTIVECAPTION Inactive window caption. Windows 98/Me, Windows 2000 or later: Specifies the left side color in the color gradient of an inactive window's title bar if the gradient effect is enabled.
		Drawing.Brushes.System.InactiveCaption.CreateSolidBrush(::GetSysColor(COLOR_INACTIVECAPTION));

		//COLOR_INACTIVECAPTIONTEXT Color of text in an inactive caption.
		Drawing.Brushes.System.InactiveCaptionText.CreateSolidBrush(::GetSysColor(COLOR_INACTIVECAPTIONTEXT));

		//COLOR_INFOBK Background color for tooltip controls.
		Drawing.Brushes.System.ToolTipBackground.CreateSolidBrush(::GetSysColor(COLOR_INFOBK));

		//COLOR_INFOTEXT Text color for tooltip controls.
		Drawing.Brushes.System.ToolTipText.CreateSolidBrush(::GetSysColor(COLOR_INFOTEXT));

		//COLOR_MENU Menu background.
		Drawing.Brushes.System.Menu.CreateSolidBrush(::GetSysColor(COLOR_MENU));

		//COLOR_MENUTEXT Text in menus.
		Drawing.Brushes.System.MenuText.CreateSolidBrush(::GetSysColor(COLOR_MENUTEXT));

		//COLOR_SCROLLBAR Scroll bar gray area.
		Drawing.Brushes.System.Scrollbar.CreateSolidBrush(::GetSysColor(COLOR_SCROLLBAR));

		//COLOR_WINDOW Window background.
		Drawing.Brushes.System.Window.CreateSolidBrush(::GetSysColor(COLOR_WINDOW));

		//COLOR_WINDOWFRAME Window frame.
		Drawing.Brushes.System.WindowFrame.CreateSolidBrush(::GetSysColor(COLOR_WINDOWFRAME));

		//COLOR_WINDOWTEXT
		Drawing.Brushes.System.WindowText.CreateSolidBrush(::GetSysColor(COLOR_WINDOWTEXT));
	}
	catch (...)
	{
		return false;
	}
	return true;
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadCursors()
{
	try
	{
		//CMS Defined cursors
		if ((CGBLResources::GetResourceCursor(IDC_CUR_MAGNIFYINGGLASS, &Drawing.Cursors.MagnifyingGlass)) == NULL) { return false; }
		if ((CGBLResources::GetResourceCursor(IDC_CUR_DRILLDOWN, &Drawing.Cursors.DrillDown)) == NULL) { return false; }

		//System Cursors
		if ((Drawing.Cursors.System.ApplicationStarting = LoadCursor(NULL, MAKEINTRESOURCE(OCR_APPSTARTING))) == NULL) { return false; }
		if ((Drawing.Cursors.System.Arrow = LoadCursor(NULL, MAKEINTRESOURCE(OCR_NORMAL))) == NULL) { return false; }
		if ((Drawing.Cursors.System.Crosshair = LoadCursor(NULL, MAKEINTRESOURCE(OCR_CROSS))) == NULL) { return false; }
		if ((Drawing.Cursors.System.IBeam = LoadCursor(NULL, MAKEINTRESOURCE(OCR_IBEAM))) == NULL) { return false; }
		if ((Drawing.Cursors.System.No = LoadCursor(NULL, MAKEINTRESOURCE(OCR_NO))) == NULL) { return false; }
		if ((Drawing.Cursors.System.SizeAll = LoadCursor(NULL, MAKEINTRESOURCE(OCR_SIZEALL))) == NULL) { return false; }
		if ((Drawing.Cursors.System.SizeNESW = LoadCursor(NULL, MAKEINTRESOURCE(OCR_SIZENESW))) == NULL) { return false; }
		if ((Drawing.Cursors.System.SizeNS = LoadCursor(NULL, MAKEINTRESOURCE(OCR_SIZENS))) == NULL) { return false; }
		if ((Drawing.Cursors.System.SizeNWSE = LoadCursor(NULL, MAKEINTRESOURCE(OCR_SIZENWSE))) == NULL) { return false; }
		if ((Drawing.Cursors.System.SizeWE = LoadCursor(NULL, MAKEINTRESOURCE(OCR_SIZEWE))) == NULL) { return false; }
		if ((Drawing.Cursors.System.UpArrow = LoadCursor(NULL, MAKEINTRESOURCE(OCR_UP))) == NULL) { return false; }
		if ((Drawing.Cursors.System.Wait = LoadCursor(NULL, MAKEINTRESOURCE(OCR_WAIT))) == NULL) { return false; }
		if ((Drawing.Cursors.System.Sample = LoadCursor(NULL, MAKEINTRESOURCE(OIC_SAMPLE))) == NULL) { return false; }
		if ((Drawing.Cursors.System.Hand = LoadCursor(NULL, MAKEINTRESOURCE(OIC_HAND))) == NULL) { return false; }
		if ((Drawing.Cursors.System.Question = LoadCursor(NULL, MAKEINTRESOURCE(OIC_QUES))) == NULL) { return false; }
		if ((Drawing.Cursors.System.Note = LoadCursor(NULL, MAKEINTRESOURCE(OIC_NOTE))) == NULL) { return false; }
		if ((Drawing.Cursors.System.Warning = LoadCursor(NULL, MAKEINTRESOURCE(OIC_WARNING))) == NULL) { return false; }
		if ((Drawing.Cursors.System.Error = LoadCursor(NULL, MAKEINTRESOURCE(OIC_ERROR))) == NULL) { return false; }
		if ((Drawing.Cursors.System.Information = LoadCursor(NULL, MAKEINTRESOURCE(OIC_INFORMATION))) == NULL) { return false; }
	}
	catch (...)
	{
		return false;
	}
	return true;
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadBitmaps()
{
	try
	{
		if (!CGBLResources::GetResourceBitmap(IDB_CMSBACKDROP, &Drawing.Bitmaps.CMSBackDrop)) { return false; }
		if (!CGBLResources::GetResourceBitmap(IDB_IRSEFILE, &Drawing.Bitmaps.IRSEFile)) { return false; }
		if (!CGBLResources::GetResourceBitmap(IDB_CMSLOGO_GRAY, &Drawing.Bitmaps.CMSLogo_Gray)) { return false; }
		if (!CGBLResources::GetResourceBitmap(IDB_CLOUDS, &Drawing.Bitmaps.Clouds)) { return false; }
		if (!CGBLResources::GetResourceBitmap(IDB_COUGARSIDEBAR, &Drawing.Bitmaps.CougarSideBar)) { return false; }
		if (!CGBLResources::GetResourceBitmap(IDB_BACKDROPTEMPLATE, &Drawing.Bitmaps.CMSBackDropIntegration)) { return false; }
		if (!CGBLResources::GetResourceBitmap(IDB_CMSINSTALLBANNER, &Drawing.Bitmaps.CMSInstallBanner)) { return false; }
		if (!CGBLResources::GetResourceBitmap(IDB_CAMERA_16X16, &Drawing.Bitmaps.Camera16)) { return false; }
		if (!CGBLResources::GetResourceBitmap(IDB_CAMERA_16X16_DISABLED, &Drawing.Bitmaps.Camera16Disabled)) { return false; }
		if (!CGBLResources::GetResourceBitmap(IDB_DRILLDOWN_16X16, &Drawing.Bitmaps.Drilldown16)) { return false; }

		if (!CGBLResources::GetResourceBitmap(IDB_BACKDROPTEMPLATEDEVELOPER, &Drawing.Bitmaps.CMSBackDropIntegrationDeveloper)) { return false; }
		if (!CGBLResources::GetResourceBitmap(IDB_BLANK, &Drawing.Bitmaps.Blank)) { return false; }

		//Load the system bitmaps
		if ((Drawing.Bitmaps.System.ButtonCorners = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_BTNCORNERS))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.ButtonSize = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_BTSIZE))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.Check = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_CHECK))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.CheckBoxes = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_CHECKBOXES))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.Close = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_CLOSE))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.Combo = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_COMBO))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.DownArrow = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_DNARROW))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.DownArrowD = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_DNARROWD))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.DownArrowI = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_DNARROWI))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.LeftArrow = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_LFARROW))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.LeftArrowD = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_LFARROWD))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.LeftArrowI = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_LFARROWI))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.MNArrow = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_MNARROW))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.Reduce = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_REDUCE))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.Reduced = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_REDUCED))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.Restore = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_RESTORE))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.Restored = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_RESTORED))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.RightArrow = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_RGARROW))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.RightArrowD = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_RGARROWD))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.RightArrowI = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_RGARROWI))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.Size = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_SIZE))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.UpArrow = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_UPARROW))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.UpArrowD = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_UPARROWD))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.UpArrowI = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_UPARROWI))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.Zoom = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_ZOOM))) == NULL) { return false; }
		if ((Drawing.Bitmaps.System.ZoomD = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_ZOOMD))) == NULL) { return false; }
	}
	catch (...)
	{
		return false;
	}
	return true;
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadIcons()
{
	if ((Drawing.Icons.System.Application = LoadIcon(NULL, IDI_APPLICATION)) == NULL) { return false; }
	if ((Drawing.Icons.System.Hand = LoadIcon(NULL, IDI_HAND)) == NULL) { return false; }
	if ((Drawing.Icons.System.Question = LoadIcon(NULL, IDI_QUESTION)) == NULL) { return false; }
	if ((Drawing.Icons.System.Exclamation = LoadIcon(NULL, IDI_EXCLAMATION)) == NULL) { return false; }
	if ((Drawing.Icons.System.Asterisk = LoadIcon(NULL, IDI_ASTERISK)) == NULL) { return false; }
	if ((Drawing.Icons.System.Warning = LoadIcon(NULL, IDI_WARNING)) == NULL) { return false; }
	if ((Drawing.Icons.System.Error = LoadIcon(NULL, IDI_ERROR)) == NULL) { return false; }
	if ((Drawing.Icons.System.Information = LoadIcon(NULL, IDI_INFORMATION)) == NULL) { return false; }

	return true;
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadColors()
{
	try
	{
		Drawing.Colors.Yellow = RGB(255, 255, 0);
		Drawing.Colors.DkYellow = RGB(128, 128, 0);
		Drawing.Colors.Red = RGB(255, 0, 0);
		Drawing.Colors.DkRed = RGB(128, 0, 0);
		Drawing.Colors.Magenta = RGB(255, 0, 255);
		Drawing.Colors.DkMagenta = RGB(128, 0, 128);
		Drawing.Colors.Blue = RGB(0, 0, 255);
		Drawing.Colors.DkBlue = RGB(0, 0, 128);
		Drawing.Colors.Cyan = RGB(0, 255, 255);
		Drawing.Colors.DkCyan = RGB(0, 128, 128);
		Drawing.Colors.Green = RGB(0, 255, 0);
		Drawing.Colors.DkGreen = RGB(0, 128, 0);
		Drawing.Colors.Gray = RGB(192, 192, 192);
		Drawing.Colors.DkGray = RGB(128, 128, 128);

		Drawing.Colors.Cream = RGB(255, 200, 150);
		Drawing.Colors.Scarlet = RGB(200, 100, 100);

		// 32 bit Colors
		Drawing.Colors.LtYellow = RGB(255, 245, 226);
		Drawing.Colors.GoldenRod = RGB(201, 200, 114);
		Drawing.Colors.LtGreen = RGB(194, 226, 217); //DAJ 9/13/5 err: 1-21135
		Drawing.Colors.LtBlue = RGB(166, 202, 240);
		Drawing.Colors.BlueGreen = RGB(60, 230, 223);
		Drawing.Colors.MedGray = RGB(160, 160, 164);
		//KPM, changed purple, found the right color numbers from print key
		Drawing.Colors.Purple = RGB(128, 0, 128);
		Drawing.Colors.MdGreen = RGB(108, 160, 141);
		Drawing.Colors.Teal = RGB(0, 92, 126);
		Drawing.Colors.OrangeAccent = RGB(249, 172, 70);
		//RCG - 06/08/2017 - PBI 39109 - Added new colors
		Drawing.Colors.TreePoppy = RGB(247, 148, 31);	//Orangish
		Drawing.Colors.RadicalRed = RGB(255, 64, 89);	//Pinkish
		Drawing.Colors.MineShaft = RGB(45, 45, 45);		//Dark Grayish

		// system Colors
		//COLOR_3DDKSHADOW Dark shadow for three-dimensional display elements.
		Drawing.Colors.System._3DDarkShadow = ::GetSysColor(COLOR_3DDKSHADOW);

		//COLOR_3DFACE, COLOR_BTNFACE Face color for three-dimensional display elements and for dialog box backgrounds.
		Drawing.Colors.System.ButtonFace = ::GetSysColor(COLOR_3DFACE);

		//COLOR_3DHILIGHT, COLOR_3DHIGHLIGHT, COLOR_BTNHILIGHT, COLOR_BTNHIGHLIGHT Highlight color for three-dimensional display elements (for edges facing the light source.)
		Drawing.Colors.System.ButtonHighlight = ::GetSysColor(COLOR_BTNHIGHLIGHT);

		//COLOR_3DLIGHT Light color for three-dimensional display elements (for edges facing the light source.)
		Drawing.Colors.System._3DLight = ::GetSysColor(COLOR_3DLIGHT);

		//COLOR_3DSHADOW, COLOR_BTNSHADOW Shadow color for three-dimensional display elements (for edges facing away from the light source).
		Drawing.Colors.System.ButtonShadow = ::GetSysColor(COLOR_3DSHADOW);

		//COLOR_ACTIVEBORDER Active window border.
		Drawing.Colors.System.ActiveBorder = ::GetSysColor(COLOR_ACTIVEBORDER);

		//COLOR_ACTIVECAPTION Active window title bar. Windows 98/Me, Windows 2000 or later: Specifies the left side color in the color gradient of an active window's title bar if the gradient effect is enabled.
		Drawing.Colors.System.ActiveCaption = ::GetSysColor(COLOR_ACTIVECAPTION);

		//COLOR_APPWORKSPACE Background color of multiple document interface (MDI) applications.
		Drawing.Colors.System.ApplicationWorkspace = ::GetSysColor(COLOR_APPWORKSPACE);

		//COLOR_BACKGROUND, COLOR_DESKTOP Desktop.
		Drawing.Colors.System.Background = ::GetSysColor(COLOR_BACKGROUND);
		Drawing.Colors.System.Desktop = ::GetSysColor(COLOR_DESKTOP);

		//COLOR_BTNTEXT Text on push buttons.
		Drawing.Colors.System.ButtonText = ::GetSysColor(COLOR_BTNTEXT);

		//COLOR_CAPTIONTEXT Text in caption, size box, and scroll bar arrow box.
		Drawing.Colors.System.CaptionText = ::GetSysColor(COLOR_CAPTIONTEXT);

		//COLOR_GRAYTEXT Grayed (disabled) text. This color is set to 0 if the current display driver does not support a solid gray color.
		Drawing.Colors.System.GrayText = ::GetSysColor(COLOR_GRAYTEXT);

		//COLOR_HIGHLIGHT Item(s) selected in a control.
		Drawing.Colors.System.Highlight = ::GetSysColor(COLOR_HIGHLIGHT);

		//COLOR_HIGHLIGHTTEXT Text of item(s) selected in a control.
		Drawing.Colors.System.HighlightText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);

		//COLOR_INACTIVEBORDER Inactive window border.
		Drawing.Colors.System.InactiveBorder = ::GetSysColor(COLOR_INACTIVEBORDER);

		//COLOR_INACTIVECAPTION Inactive window caption. Windows 98/Me, Windows 2000 or later: Specifies the left side color in the color gradient of an inactive window's title bar if the gradient effect is enabled.
		Drawing.Colors.System.InactiveCaption = ::GetSysColor(COLOR_INACTIVECAPTION);

		//COLOR_INACTIVECAPTIONTEXT Color of text in an inactive caption.
		Drawing.Colors.System.InactiveCaptionText = ::GetSysColor(COLOR_INACTIVECAPTIONTEXT);

		//COLOR_INFOBK Background color for tooltip controls.
		Drawing.Colors.System.ToolTipBackground = ::GetSysColor(COLOR_INFOBK);

		//COLOR_INFOTEXT Text color for tooltip controls.
		Drawing.Colors.System.ToolTipText = ::GetSysColor(COLOR_INFOTEXT);

		//COLOR_MENU Menu background.
		Drawing.Colors.System.Menu = ::GetSysColor(COLOR_MENU);

		//COLOR_MENUTEXT Text in menus.
		Drawing.Colors.System.MenuText = ::GetSysColor(COLOR_MENUTEXT);

		//COLOR_SCROLLBAR Scroll bar gray area.
		Drawing.Colors.System.Scrollbar = ::GetSysColor(COLOR_SCROLLBAR);

		//COLOR_WINDOW Window background.
		Drawing.Colors.System.Window = ::GetSysColor(COLOR_WINDOW);

		//COLOR_WINDOWFRAME Window frame.
		Drawing.Colors.System.WindowFrame = ::GetSysColor(COLOR_WINDOWFRAME);

		//COLOR_WINDOWTEXT
		Drawing.Colors.System.WindowText = ::GetSysColor(COLOR_WINDOWTEXT);
	}
	catch (...)
	{
		//return false;
	}
	return true;
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::InitializeLanguageDLL()
{
	try
	{
		this->Localization.LanguageDLL = CGBLResources::GetResourceHandle();
		this->Localization.NetLang = CGBLResources::GetUserLangID();
	}
	catch (...)
	{
		return false;
	}
	return true;
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadAPSystemSettings(bool keepSelectedBatch /*= false*/)
{
	// 1-24790 BK 6/28/06 exit if module isn't installed - safeguard to prevent server calls to tables that don't exist
	if (!ModulesInstalled.AccountsPayable.IsUsable) return true;

	//Get the AP System Information
	CXMLParams clsXMLCommand, clsXMLData;
	CXMLParams clsXMLItem(true);
	CXMLParams clsXMLReturn(true);
	CXMLParams clsXMLTableData(true);
	CXMLParams::MakeXMLCommand(&clsXMLCommand, _T("GET"), _T("AP"), _T("AP_SYSTEM"));
	CXMLParams::AppendCompanyInfo(&clsXMLData);
	CXMLParams::AppendXMLConnection(&clsXMLData, CXMLParams::CN_COMPANY); // begbert 02-15-2006 VS8: warning C4482

	clsXMLItem.SetRoot(_T("RECORD"));
	clsXMLItem.MakeParam(_T("strLocation"), this->CompanySettings.DefaultLocation);

	//Store the data in here
	clsXMLTableData.SetRoot(_T("TABLE"));
	clsXMLTableData.AppendXMLParam(&clsXMLItem);
	clsXMLData.AppendXMLParam(&clsXMLTableData);

	//Create the xml sending class which communicates with the server
	CXML clsXML;

	clsXMLReturn.SetXML(clsXML.SendXML(clsXMLCommand.GetXML(), clsXMLData.GetXML()));
	if (clsXMLReturn.ParamExists(CMSStrings::XMLTags::Error)) return false;

	//Retrieve the Child Node (Table)
	CXMLParams clsXMLTable;
	clsXMLTable = clsXMLReturn.GetXMLParams(CMSStrings::XMLTags::Table);

	return ModuleSettings.APSettings.LoadData(clsXMLTable, keepSelectedBatch);
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadARSystemSettings()
{
	// 1-24790 BK 6/28/06 exit if module isn't installed - safeguard to prevent server calls to tables that don't exist
	if (!ModulesInstalled.AccountsReceivable.IsUsable) return true;

	//Get the AR System Information
	CXMLParams clsXMLCommand, clsXMLData;
	CXMLParams clsXMLItem(true);
	CXMLParams clsXMLReturn(true);
	CXMLParams clsXMLTableData(true);
	CXMLParams::MakeXMLCommand(&clsXMLCommand, _T("GET"), _T("AR"), _T("AR_SYSTEM"));
	CXMLParams::AppendCompanyInfo(&clsXMLData);
	CXMLParams::AppendXMLConnection(&clsXMLData, CXMLParams::CN_COMPANY); // begbert 02-15-2006 VS8: warning C4482

	clsXMLItem.SetRoot(_T("RECORD"));
	clsXMLItem.MakeParam(_T("strLocation"), this->CompanySettings.DefaultLocation);

	//Store the data in here
	clsXMLTableData.SetRoot(_T("TABLE"));
	clsXMLTableData.AppendXMLParam(&clsXMLItem);
	clsXMLData.AppendXMLParam(&clsXMLTableData);

	//Create the xml sending class which communicates with the server
	CXML clsXML;

	clsXMLReturn.SetXML(clsXML.SendXML(clsXMLCommand.GetXML(), clsXMLData.GetXML()));

	if (clsXMLReturn.ParamExists(CMSStrings::XMLTags::Error)) return false;

	//Retrieve the Child Node (Table)
	CXMLParams clsXMLTable;
	clsXMLTable = clsXMLReturn.GetXMLParams(CMSStrings::XMLTags::Table);

	return ModuleSettings.ARSettings.LoadData(clsXMLTable);
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadBRSystemSettings()
{
	// 1-24790 BK 6/28/06 exit if module isn't installed - safeguard to prevent server calls to tables that don't exist
	if (!ModulesInstalled.BankReconciliation.IsUsable) return true;

	//Get the BR System Information
	CXMLParams clsXMLCommand, clsXMLData;
	CXMLParams clsXMLItem(true);
	CXMLParams clsXMLReturn(true);
	CXMLParams clsXMLTableData(true);
	CXMLParams::MakeXMLCommand(&clsXMLCommand, _T("GET"), _T("BR"), _T("BR_SYSTEM"));
	CXMLParams::AppendCompanyInfo(&clsXMLData);
	CXMLParams::AppendXMLConnection(&clsXMLData, CXMLParams::CN_COMPANY); // begbert 02-15-2006 VS8: warning C4482

	clsXMLItem.SetRoot(_T("RECORD"));
	clsXMLItem.MakeParam(_T("strLocation"), this->CompanySettings.DefaultLocation);

	//Store the data in here
	clsXMLTableData.SetRoot(_T("TABLE"));
	clsXMLTableData.AppendXMLParam(&clsXMLItem);
	clsXMLData.AppendXMLParam(&clsXMLTableData);

	//Create the xml sending class which communicates with the server
	CXML clsXML;

	clsXMLReturn.SetXML(clsXML.SendXML(clsXMLCommand.GetXML(), clsXMLData.GetXML()));

	if (clsXMLReturn.ParamExists(CMSStrings::XMLTags::Error))	return false;

	//Retrieve the Child Node (Table)
	CXMLParams clsXMLTable;
	clsXMLTable = clsXMLReturn.GetXMLParams(CMSStrings::XMLTags::Table);

	return ModuleSettings.BRSettings.LoadData(clsXMLTable);
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadCTSystemSettings()
{
	//Get CT System Information
	CXMLParams clsXMLCommand, clsXMLData;
	CXMLParams clsXMLItem(true);
	CXMLParams clsXMLReturn(true);
	CXMLParams clsXMLTableData(true);
	CXMLParams::MakeXMLCommand(&clsXMLCommand, _T("GET"), _T("CT"), _T("CMS_SYSTEM"));
	CXMLParams::AppendCompanyInfo(&clsXMLData);
	CXMLParams::AppendXMLConnection(&clsXMLData, CXMLParams::CN_COMPANY);

	clsXMLItem.SetRoot(_T("RECORD"));
	clsXMLItem.MakeParam(_T("strLocation"), this->CompanySettings.DefaultLocation);

	//Store the data in here
	clsXMLTableData.SetRoot(_T("TABLE"));
	clsXMLTableData.AppendXMLParam(&clsXMLItem);
	clsXMLData.AppendXMLParam(&clsXMLTableData);

	//Create the xml sending class which communicates with the server
	CXML clsXML;
	clsXMLReturn.SetXML(clsXML.SendXML(clsXMLCommand.GetXML(), clsXMLData.GetXML()));
	if (clsXMLReturn.ParamExists(CMSStrings::XMLTags::Error))
		return false;

	//Retrieve the Child Node (Table)
	CXMLParams clsXMLTable;

	// 1-34934 BK 10/20/10 skip trying to get data if CMS_System table doesn't exist
	if (clsXMLReturn.ParamExists(CMSStrings::XMLTags::Table))
		clsXMLTable = clsXMLReturn.GetXMLParams(CMSStrings::XMLTags::Table);

	return ModuleSettings.CTSettings.LoadData(clsXMLTable);
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadGLSystemSettings(bool bSegmentInfoOnly)
{
	// 1-24788 BK 6/27/06 exit if module isn't installed - safeguard to prevent server calls to tables that don't exist
	if (!ModulesInstalled.GeneralLedger.IsUsable) return true;

	//Get the GL System Information
	CXMLParams clsXMLCommand, clsXMLData;
	CXMLParams clsXMLItem(true);
	CXMLParams clsXMLReturn(true);
	CXMLParams clsXMLTableData(true);
	CXMLParams::MakeXMLCommand(&clsXMLCommand, _T("GET"), _T("GL"), _T("GL_SYSTEM"));
	CXMLParams::AppendCompanyInfo(&clsXMLData);
	CXMLParams::AppendXMLConnection(&clsXMLData, CXMLParams::CN_COMPANY); // begbert 02-15-2006 VS8: warning C4482

	clsXMLItem.SetRoot(_T("RECORD"));
	clsXMLItem.MakeParam(_T("strLocation"), this->CompanySettings.DefaultLocation);

	//Store the data in here
	clsXMLTableData.SetRoot(_T("TABLE"));
	clsXMLTableData.AppendXMLParam(&clsXMLItem);
	clsXMLData.AppendXMLParam(&clsXMLTableData);

	//Create the xml sending class which communicates with the server
	CXML clsXML;

	clsXMLReturn.SetXML(clsXML.SendXML(clsXMLCommand.GetXML(), clsXMLData.GetXML()));

	if (clsXMLReturn.ParamExists(CMSStrings::XMLTags::Error))	return false;

	//Retrieve the Child Node (Table)
	CXMLParams clsXMLTable;
	clsXMLTable = clsXMLReturn.GetXMLParams(CMSStrings::XMLTags::Table);

	// BK 12/30/2010 when AdjustAcctSegments dialog is saved need to reload account segment info so ModPrefs grid can be updated
	// needed if segment positions are changed
	if (!bSegmentInfoOnly)
	{
		if (!ModuleSettings.GLSettings.LoadData(clsXMLTable))
			return false;
	}

	try
	{
		//////////////////////////////////////////////////////
		//  LOAD SEGMENT SETUP
		//////////////////////////////////////////////////////
		SegmentSetup.NumberOfSegments = clsXMLTable.GetParam__int8(_T("bytSegmentCount"));
		SegmentSetup.Segments[0].Length = clsXMLTable.GetParam__int8(_T("bytSegmentLength1"));
		SegmentSetup.Segments[1].Length = clsXMLTable.GetParam__int8(_T("bytSegmentLength2"));
		SegmentSetup.Segments[2].Length = clsXMLTable.GetParam__int8(_T("bytSegmentLength3"));
		SegmentSetup.Segments[3].Length = clsXMLTable.GetParam__int8(_T("bytSegmentLength4"));
		SegmentSetup.Segments[4].Length = clsXMLTable.GetParam__int8(_T("bytSegmentLength5"));
		SegmentSetup.Segments[5].Length = clsXMLTable.GetParam__int8(_T("bytSegmentLength6"));
		SegmentSetup.Segments[0].Type = clsXMLTable.GetParam__int8(_T("bytSegmentType1"));
		SegmentSetup.Segments[1].Type = clsXMLTable.GetParam__int8(_T("bytSegmentType2"));
		SegmentSetup.Segments[2].Type = clsXMLTable.GetParam__int8(_T("bytSegmentType3"));
		SegmentSetup.Segments[3].Type = clsXMLTable.GetParam__int8(_T("bytSegmentType4"));
		SegmentSetup.Segments[4].Type = clsXMLTable.GetParam__int8(_T("bytSegmentType5"));
		SegmentSetup.Segments[5].Type = clsXMLTable.GetParam__int8(_T("bytSegmentType6"));
		SegmentSetup.SeparationCharacter = clsXMLTable.GetParamStr(_T("strSeparator"));

		//AES this is were the code needed to be to fix the below error 1-3461 4.23.2003
		/*Peter Ringering - 03/04/2003 - 1-3452 - Already done by the server component*/
		SegmentSetup.TotalAccountLength = clsXMLTable.GetParam__int8(_T("bytSegmentLength"));

		// BK 12/17/2010 clear values for non-fund companies
		SegmentSetup.FundSegment = 0;
		SegmentSetup.FundSegInfo = &SegmentSetup.Segments[0];
		SegmentSetup.FundSegInfo->isFund = false;

		for (int nCntr = 0; nCntr < 6; nCntr++)
		{
			// fund segment is always type = 1
			if (SegmentSetup.Segments[nCntr].Type == 1)
			{
				// BK 12/17/2010 corrected by adding +1 since fund segments are 1-6 not 0-5
				SegmentSetup.FundSegment = (__int8)(nCntr + 1);
				SegmentSetup.FundSegInfo = &SegmentSetup.Segments[nCntr];
				SegmentSetup.FundSegInfo->isFund = True;
				// 1-35647 BK 4/14/11 set fund segment length
				SegmentSetup.FundSegInfo->Length = SegmentSetup.Segments[nCntr].Length;
			}
		}
		// BK 1/17/2011 if this is a fund company and there's no Fund Segment then it's a new company, set the default the fund segment as first segment (default)
		if (CompanySettings.IsFund &&
			(SegmentSetup.FundSegment == 0 ||
			 SegmentSetup.NumberOfSegments == 1))
		{
			SegmentSetup.NumberOfSegments = 2;
			SegmentSetup.FundSegment = 1;
			SegmentSetup.FundSegInfo->isFund = True;
			SegmentSetup.FundSegInfo->Length = 0;
			// 1-35398 BK 4/25/11 since this is a new fund company and TotalAccountLength=50 (default) set the total account length to 0
			if (SegmentSetup.TotalAccountLength == 50)
				SegmentSetup.TotalAccountLength = 0;
		}
		// When we reload the segments, we need to reload these accounts too as they might have changed.
		ModuleSettings.GLSettings.NeonCRMDefaultCashAccount = clsXMLTable.GetParamStr(L"strNeonCRMDefaultCashAccount", false);
		ModuleSettings.GLSettings.NeonCRMDefaultRevenueAccount = clsXMLTable.GetParamStr(L"strNeonCRMDefaultRevenueAccount", false);
		ModuleSettings.GLSettings.NeonCRMDefaultDefaultFund = clsXMLTable.GetParamStr(L"strNeonCRMDefaultDefaultFund", false);

		ModuleSettings.GLSettings.BalanceForward = clsXMLTable.GetParamBool(_T("bolBalanceForward"));
	}
	catch (...)
	{
		return false;
	}
	return true;
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadINSystemSettings()
{
	// 1-24790 BK 6/28/06 exit if module isn't installed - safeguard to prevent server calls to tables that don't exist
	if (!ModulesInstalled.Inventory.IsUsable) return true;

	//Get the IN System Information
	CXMLParams clsXMLCommand, clsXMLData;
	CXMLParams clsXMLItem(true);
	CXMLParams clsXMLReturn(true);
	CXMLParams clsXMLTableData(true);
	CXMLParams::MakeXMLCommand(&clsXMLCommand, _T("GET"), _T("IN"), _T("IN_SYSTEM"));
	CXMLParams::AppendCompanyInfo(&clsXMLData);
	CXMLParams::AppendXMLConnection(&clsXMLData, CXMLParams::CN_COMPANY); // begbert 02-15-2006 VS8: warning C4482

	clsXMLItem.SetRoot(_T("RECORD"));
	clsXMLItem.MakeParam(_T("strLocation"), this->CompanySettings.DefaultLocation);

	//Store the data in here
	clsXMLTableData.SetRoot(_T("TABLE"));
	clsXMLTableData.AppendXMLParam(&clsXMLItem);
	clsXMLData.AppendXMLParam(&clsXMLTableData);

	//Create the xml sending class which communicates with the server
	CXML clsXML;

	clsXMLReturn.SetXML(clsXML.SendXML(clsXMLCommand.GetXML(), clsXMLData.GetXML()));

	if (clsXMLReturn.ParamExists(CMSStrings::XMLTags::Error))	return false;

	//Retrieve the Child Node (Table)
	CXMLParams clsXMLTable;
	clsXMLTable = clsXMLReturn.GetXMLParams(CMSStrings::XMLTags::Table);

	if (!ModuleSettings.INSettings.LoadData(clsXMLTable))		return false;

	return true;
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadJCSystemSettings()
{
	// BK 7/22/04 Cannot load JC_System if there are no JobCost tables
	if (!g_pGBLSystemInformationCMSDll->ModulesInstalled.JobCost.IsUsable)	return true;

	//Get the JC System Information
	CXMLParams clsXMLCommand, clsXMLData;
	CXMLParams clsXMLReturn(true);
	CXMLParams clsXMLTableData(true);
	CXMLParams::MakeXMLCommand(&clsXMLCommand, _T("GET"), _T("JC"), _T("JC_SYSTEM"));
	CXMLParams::AppendCompanyInfo(&clsXMLData);
	CXMLParams::AppendXMLConnection(&clsXMLData, CXMLParams::CN_COMPANY); // begbert 02-15-2006 VS8: warning C4482

	//Store the data in here
	clsXMLTableData.SetRoot(_T("TABLE"));
	clsXMLTableData.MakeParam(_T("RECORD"), _T(""));
	clsXMLData.AppendXMLParam(&clsXMLTableData);

	//Create the xml sending class which communicates with the server
	CXML clsXML;

	clsXMLReturn.SetXML(clsXML.SendXML(clsXMLCommand.GetXML(), clsXMLData.GetXML()));

	if (clsXMLReturn.ParamExists(CMSStrings::XMLTags::Error))	return false;

	//Retrieve the Child Node (Table)
	CXMLParams clsXMLTable;
	//	PBI 29370	04/17/2015	mvt - Activated Job Cost integration
	if (!CompanySettings.IsFund)
		clsXMLTable = clsXMLReturn.GetXMLParams(CMSStrings::XMLTags::TABLE);

	if (!ModuleSettings.JCSettings.LoadData(clsXMLTable))		return false;

	return true;
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadPRSystemSettings()
{
	// 1-24790 BK 6/28/06 exit if module isn't installed - safeguard to prevent server calls to tables that don't exist
	if (!ModulesInstalled.Payroll.IsUsable) return true;

	//Get the PR System Information
	CXMLParams clsXMLCommand, clsXMLData;
	CXMLParams clsXMLItem(true);
	CXMLParams clsXMLReturn(true);
	CXMLParams clsXMLTableData(true);
	CXMLParams::MakeXMLCommand(&clsXMLCommand, _T("GET"), _T("PR"), _T("PR_SYSTEM"));
	CXMLParams::AppendCompanyInfo(&clsXMLData);
	CXMLParams::AppendXMLConnection(&clsXMLData, CXMLParams::CN_COMPANY); // begbert 02-15-2006 VS8: warning C4482

	clsXMLItem.SetRoot(_T("RECORD"));
	clsXMLItem.MakeParam(_T("strLocation"), this->CompanySettings.DefaultLocation);

	//Store the data in here
	clsXMLTableData.SetRoot(_T("TABLE"));
	clsXMLTableData.AppendXMLParam(&clsXMLItem);
	clsXMLTableData.MakeParam(_T("RECORD"), _T(""));
	clsXMLData.AppendXMLParam(&clsXMLTableData);

	//Create the xml sending class which communicates with the server
	CXML clsXML;

	clsXMLReturn.SetXML(clsXML.SendXML(clsXMLCommand.GetXML(), clsXMLData.GetXML()));
	if (clsXMLReturn.ParamExists(CMSStrings::XMLTags::Error))	return false;

	//Retrieve the Child Node (Table)
	CXMLParams clsXMLTable;
	clsXMLTable = clsXMLReturn.GetXMLParams(CMSStrings::XMLTags::Table);

	if (!ModuleSettings.PRSettings.LoadData(clsXMLTable))		return false;
	return true;
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadPOSystemSettings(bool bFromModPrefs)
{
	// 1-24790 BK 6/28/06 exit if module isn't installed - safeguard to prevent server calls to tables that don't exist
	if (!ModulesInstalled.PurchaseOrder.IsUsable)
		return true;

	//Get the PO System Information
	CXMLParams clsXMLCommand, clsXMLData;
	CXMLParams clsXMLReturn(true);
	CXMLParams clsXMLItem(true);
	CXMLParams clsXMLTableData(true);
	CXMLParams::MakeXMLCommand(&clsXMLCommand, _T("GET"), _T("PO"), _T("PO_SYSTEM"));
	CXMLParams::AppendCompanyInfo(&clsXMLData);
	CXMLParams::AppendXMLConnection(&clsXMLData, CXMLParams::CN_COMPANY); // begbert 02-15-2006 VS8: warning C4482

	clsXMLItem.SetRoot(_T("RECORD"));
	clsXMLItem.MakeParam(_T("strLocation"), this->CompanySettings.DefaultLocation);

	//Store the data in here
	clsXMLTableData.SetRoot(_T("TABLE"));
	clsXMLTableData.AppendXMLParam(&clsXMLItem);
	clsXMLTableData.MakeParam(_T("RECORD"), _T(""));
	clsXMLData.AppendXMLParam(&clsXMLTableData);

	//Create the xml sending class which communicates with the server
	CXML clsXML;

	clsXMLReturn.SetXML(clsXML.SendXML(clsXMLCommand.GetXML(), clsXMLData.GetXML()));
	if (clsXMLReturn.ParamExists(CMSStrings::XMLTags::Error))
		return false;

	//Retrieve the Child Node (Table)
	CXMLParams clsXMLTable;
	clsXMLTable = clsXMLReturn.GetXMLParams(CMSStrings::XMLTags::Table);
	if (!ModuleSettings.POSettings.LoadData(clsXMLTable))
		return false;

	//load the global list of udfs
	ApplicationSettings.LoadGlobalUDFs();

	//load the selected module list of udfs
	ModuleSettings.POSettings.SelectedUDFs.LoadSelectedUDFs();

	bstr_t bstrReturn = clsXMLReturn.GetXMLBSTR();

	// PBI 22186 BK 8/25/14 Added red flag reports, added this to avoid using PO's print options while in CT
	if (ApplicationSettings.IsPO())
		LoadPrintOptions(bstrReturn);

	return true;
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadSASystemSettings()
{
	// 1-24790 BK 6/28/06 exit if module isn't installed - safeguard to prevent server calls to tables that don't exist
	if (!ModulesInstalled.OrderEntry.IsUsable && !ModulesInstalled.PointOfSale.IsUsable && !ModulesInstalled.SalesEntry.IsUsable)
		return true;

	//Get the SA System Information
	CXMLParams clsXMLCommand, clsXMLData;
	CXMLParams clsXMLItem(true);
	CXMLParams clsXMLReturn(true);
	CXMLParams clsXMLTableData(true);
	CXMLParams::MakeXMLCommand(&clsXMLCommand, _T("GET"), _T("SA"), _T("SA_SYSTEM"));
	CXMLParams::AppendCompanyInfo(&clsXMLData);
	CXMLParams::AppendXMLConnection(&clsXMLData, CXMLParams::CN_COMPANY); // begbert 02-15-2006 VS8: warning C4482

	clsXMLItem.SetRoot(_T("RECORD"));
	clsXMLItem.MakeParam(_T("strLocation"), this->CompanySettings.DefaultLocation);
	clsXMLItem.MakeParam(_T("BYTMODULE"), "");

	//Store the data in here
	clsXMLTableData.SetRoot(_T("TABLE"));
	clsXMLTableData.AppendXMLParam(&clsXMLItem);
	clsXMLTableData.MakeParam(_T("RECORD"), _T(""));
	clsXMLData.AppendXMLParam(&clsXMLTableData);

	//Create the xml sending class which communicates with the server
	CXML clsXML;

	_bstr_t bstrReturn = clsXML.SendXML(clsXMLCommand.GetXML(), clsXMLData.GetXML());
	clsXMLReturn.SetXML(bstrReturn);
	if (clsXMLReturn.ParamExists(CMSStrings::XMLTags::Error))	return false;

	try
	{
		if (!ModuleSettings.SASettings.LoadData(bstrReturn))
			return false;
	}
	catch (...)
	{
		return false;
	}
	return true;
}

//-------------------------------------------------------------------
void CGBLSystemInformation::LoadPrintOptions(_bstr_t bstrXMLString)
{
	CDataTable	clsTable;
	CDataSet	clsData;

	clsData.LoadFromXML(bstrXMLString);

	// Printer Options table - Table7
	if (clsData.TableExists(_T("Table7")))
	{
		clsData.GetTable(_T("Table7"), &clsTable);
		if (!ModuleSettings.CurrentSettings->LoadCurrentPrinterSettings(&clsTable))
			ModuleSettings.CurrentSettings->LoadDefaultPrinterSettings();

	}
}

//-------------------------------------------------------------------
// Used to check and see if the modules the company says they have
// installed are actually installed legally on the users machine
void CGBLSystemInformation::CheckInstalledModulesAfterLogin(void)
{
	//Controller
	if (g_pGBLSystemInformationCMSDll->ModulesInstalled.Controller.IsUsable)
	{

	}

	//General Ledger

	//Inventory

	//Accounts Receivable

	//Accounts Payable

	//
}

//-------------------------------------------------------------------
void CGBLSystemInformation::CheckForCommandLineOverrides(void)
{
	//Check for developer mode, always set because it's false by default, if the user decides to
	//set it then it turns it to true
	if (CommandLineSettings.PO_Developer > 0)
	{
		ApplicationSettings.DeveloperMode = true;
		if (CommandLineSettings.PO_Developer == 2)
			ApplicationSettings.ShowXMLViewer = true;
	}

	//Override Primary Server Settings but only if they have specified valid entries for all mandatory items
	if ((!CommandLineSettings.PL_PrimaryServer.IsEmpty()) &&
		(!CommandLineSettings.PL_PrimaryDatabase.IsEmpty()) &&
		(!CommandLineSettings.PL_PrimaryUserName.IsEmpty()))
	{
		Servers.PrimaryServer.Name = CommandLineSettings.PL_PrimaryServer;
		Servers.PrimaryServer.Database = CommandLineSettings.PL_PrimaryDatabase;
		Servers.PrimaryServer.User = CommandLineSettings.PL_PrimaryUserName;
		if (!Servers.PrimaryServer.Password.IsEmpty())	Servers.PrimaryServer.Password = CommandLineSettings.PL_PrimaryPassword;
		// 1-31094 BK 10/23/08 changed so DBType will be filled correctly
		//if (! Servers.PrimaryServer.DBType.IsEmpty() )
		if (Servers.PrimaryServer.DBType.IsEmpty())
			Servers.PrimaryServer.DBType = _T("SQLSERVER");
		if (Servers.PrimaryServer.Timeout != 0)		Servers.PrimaryServer.Timeout = _tstoi(CommandLineSettings.PL_PrimaryServerTimeout);
	}

	//Override Archive Server Settings but only if they have specified valid entries for all mandatory items
	if ((!CommandLineSettings.AL_ArchiveServer.IsEmpty()) &&
		(!CommandLineSettings.AL_ArchiveDatabase.IsEmpty()) &&
		(!CommandLineSettings.AL_ArchiveUserName.IsEmpty()))
	{
		Servers.ArchiveServer.Name = CommandLineSettings.AL_ArchiveServer;
		Servers.ArchiveServer.Database = CommandLineSettings.AL_ArchiveDatabase;
		Servers.ArchiveServer.User = CommandLineSettings.AL_ArchiveUserName;
		if (!Servers.ArchiveServer.Password.IsEmpty())	Servers.ArchiveServer.Password = CommandLineSettings.AL_ArchivePassword;
		if (!Servers.ArchiveServer.DBType.IsEmpty())		Servers.ArchiveServer.DBType = _T("SQLSERVER");
		if (Servers.ArchiveServer.Timeout != 0)		Servers.ArchiveServer.Timeout = _tstoi(CommandLineSettings.AL_ArchiveServerTimeout);
	}

	//Override User Login and Company Selection Settings but only if they have specified valid entries for all mandatory items
	CommandLineSettings.LO_BackgroundLogin = false;
	if ((!CommandLineSettings.UL_CompanyUser.IsEmpty()) &&
		(!CommandLineSettings.DC_DefaultCompany.IsEmpty()))
		CommandLineSettings.LO_BackgroundLogin = true;
}

//Check to see if this library can be loaded. PGP(03/07/2003)
bool CGBLSystemInformation::CanLoadLibrary(_TCHAR* szLibraryName)
{
	ASSERT(szLibraryName);

	//Try to load the library
	HMODULE hLib = ::LoadLibrary(szLibraryName);
	if (hLib != NULL)
	{
		::FreeLibrary(hLib);
		return true;
	}
	else
		return false;
}

//Check to see if a function exists in the library PGP(03/07/2003)
bool CGBLSystemInformation::CanLoadFunction(_TCHAR* szLibraryName, _TCHAR* szFunctionName)
{
	ASSERT(szLibraryName);
	ASSERT(szFunctionName);

	bool bCanLoadFunction = false;
	//Try to load the library
	HMODULE hLib = ::LoadLibrary(szLibraryName);
	if (hLib != NULL)
	{
		//Try to get the address of the function in the lib
		FARPROC pFn = ::GetProcAddress(hLib, (_bstr_t)szFunctionName);
		if (pFn != NULL)
		{
			bCanLoadFunction = true;
			::FreeLibrary(hLib);
		}
	}
	return bCanLoadFunction;
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::SetupHelpSystem()
{
	//02/21/2019 CJB - PBI 47292 - Changed Help to access Web Help.
	HelpSettings.HelpFileName = _T("");
	HelpSettings.HelpFilePath = CGBLForm::GetHelpPath();
	HelpSettings.HelpRemotePath = HelpSettings.HelpFilePath;
	HelpSettings.HelpType = 1;
	HelpSettings.HelpWebName = _T("#page/");
	HelpSettings.HelpWebURL = _T("https://cougarmtnservice.com/help/");

	return true;
}

//-------------------------------------------------------------------
void CGBLSystemInformation::SetModuleLicense(stuLicensed& stuMod, bool bExpired, bool bInCompany, bool bIsUsable, bool bLicensed, bool bTimeLocked)
{
	stuMod.Expired = bExpired;
	stuMod.InCompany = bInCompany;
	stuMod.IsUsable = bIsUsable;
	stuMod.Licensed = bLicensed;
	stuMod.TimeLocked = bTimeLocked;
}

//-------------------------------------------------------------------
void CGBLSystemInformation::CheckModuleLicense(stuLicensed& stuMod)
{
	if (stuMod.InCompany && !stuMod.Expired && (stuMod.Licensed || stuMod.TimeLocked))
	{
		stuMod.IsUsable = true;
	}
}

//-------------------------------------------------------------------
void CGBLSystemInformation::ResetModuleLicense(stuLicensed& stuMod, enuModuleSetting p_Part)
{
	switch (p_Part)
	{
	case InCompany:
		stuMod.InCompany = false;
		break;

	case Licensed:
		stuMod.Licensed = false;
		break;

	case TimeLocked:
		stuMod.TimeLocked = false;
		break;

	case Expired:
		stuMod.Expired = false;
		break;

	case IsUsable:
		stuMod.IsUsable = false;
		break;
	}
}

//-------------------------------------------------------------------
void CGBLSystemInformation::CheckModuleLicenses()
{
	CheckModuleLicenses(ModulesInstalled);
}

//-------------------------------------------------------------------
void CGBLSystemInformation::ResetModuleLicenses(enuModuleSetting p_Part)
{
	ResetModuleLicenses(ModulesInstalled, p_Part);
}

//-------------------------------------------------------------------
void CGBLSystemInformation::CheckModuleLicenses(stuModulesInstalled& ModInst)
{
	CheckModuleLicense(ModInst.PackagesInstalled.FUND);
	CheckModuleLicense(ModInst.PackagesInstalled.ACT);

	//Modules Installed
	CheckModuleLicense(ModInst.AccountsPayable);
	CheckModuleLicense(ModInst.AccountsReceivable);
	CheckModuleLicense(ModInst.BankReconciliation);
	CheckModuleLicense(ModInst.BillOfMaterials);
	CheckModuleLicense(ModInst.CostCenter);
	CheckModuleLicense(ModInst.Commissions);
	CheckModuleLicense(ModInst.Controller);
	CheckModuleLicense(ModInst.DataExchange);
	CheckModuleLicense(ModInst.ECommerce);
	CheckModuleLicense(ModInst.FinancialAnalysis);
	CheckModuleLicense(ModInst.GeneralLedger);
	CheckModuleLicense(ModInst.Inventory);
	CheckModuleLicense(ModInst.JobCost);
	CheckModuleLicense(ModInst.Layaway);
	CheckModuleLicense(ModInst.LotTracking);
	CheckModuleLicense(ModInst.MultiLocationInventory);
	CheckModuleLicense(ModInst.OrderEntry);
	CheckModuleLicense(ModInst.Payroll);
	CheckModuleLicense(ModInst.PointOfSale);
	CheckModuleLicense(ModInst.Polling);
	CheckModuleLicense(ModInst.PurchaseOrder);
	CheckModuleLicense(ModInst.ReportGenerator);
	CheckModuleLicense(ModInst.SalesEntry);
	CheckModuleLicense(ModInst.SpecialtyShop);
	CheckModuleLicense(ModInst.API);
	CheckModuleLicense(ModInst.DenaliBI);
}

//-------------------------------------------------------------------
void CGBLSystemInformation::ResetModuleLicenses(stuModulesInstalled& ModInst, enuModuleSetting p_Part)
{
	// BK 10/14/04 set to true because layaway will always be installed
	ResetModuleLicense(ModInst.PackagesInstalled.FUND, p_Part);
	ResetModuleLicense(ModInst.PackagesInstalled.ACT, p_Part);

	//Modules Installed
	ResetModuleLicense(ModInst.AccountsPayable, p_Part);
	ResetModuleLicense(ModInst.AccountsReceivable, p_Part);
	ResetModuleLicense(ModInst.BankReconciliation, p_Part);
	ResetModuleLicense(ModInst.BillOfMaterials, p_Part);
	ResetModuleLicense(ModInst.CostCenter, p_Part);
	ResetModuleLicense(ModInst.Commissions, p_Part);
	ResetModuleLicense(ModInst.Controller, p_Part);
	ResetModuleLicense(ModInst.DataExchange, p_Part);
	ResetModuleLicense(ModInst.ECommerce, p_Part);
	ResetModuleLicense(ModInst.FinancialAnalysis, p_Part);
	ResetModuleLicense(ModInst.GeneralLedger, p_Part);
	ResetModuleLicense(ModInst.Inventory, p_Part);
	ResetModuleLicense(ModInst.JobCost, p_Part);
	ResetModuleLicense(ModInst.Layaway, p_Part);
	ResetModuleLicense(ModInst.LotTracking, p_Part);
	ResetModuleLicense(ModInst.MultiLocationInventory, p_Part);
	ResetModuleLicense(ModInst.OrderEntry, p_Part);
	ResetModuleLicense(ModInst.Payroll, p_Part);
	ResetModuleLicense(ModInst.PointOfSale, p_Part);
	ResetModuleLicense(ModInst.Polling, p_Part);
	ResetModuleLicense(ModInst.PurchaseOrder, p_Part);
	ResetModuleLicense(ModInst.ReportGenerator, p_Part);
	ResetModuleLicense(ModInst.SalesEntry, p_Part);
	ResetModuleLicense(ModInst.SpecialtyShop, p_Part);
	ResetModuleLicense(ModInst.API, p_Part);
	ResetModuleLicense(ModInst.DenaliBI, p_Part);
}

//-------------------------------------------------------------------
void CGBLSystemInformation::ResetModulesInfo()
{
	ResetModulesInfo(ModulesInstalled);
	ResetModulesInfo(InstalledPlatforms.ACT);
	ResetModulesInfo(InstalledPlatforms.FUND);
}

//-------------------------------------------------------------------
void CGBLSystemInformation::ResetModulesInfo(stuModulesInstalled& ModInst)
{
	ResetModuleLicenses(ModInst, InCompany);
	ResetModuleLicenses(ModInst, Licensed);
	ResetModuleLicenses(ModInst, TimeLocked);
	ResetModuleLicenses(ModInst, Expired);
	ResetModuleLicenses(ModInst, IsUsable);
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::GetPackageInstalled(CXMLDocument* objDoc)
{
	ASSERT(objDoc);
	CXMLElement eleRoot = objDoc->GetDocumentElement();
	if (eleRoot.ElementExists(_T("modules")))
	{
		ApplicationSettings.nMaxUserCount = eleRoot.GetItem(_T("usercount")).GetInnerTextInt();
		ApplicationSettings.bMaxReached = eleRoot.ElementExists(_T("maxusers"));
		if (eleRoot.ElementExists(_T("license")))
			ApplicationSettings.LicenseGUID = eleRoot.GetItem(_T("license")).GetInnerText();
		else
			ApplicationSettings.LicenseGUID.Empty();
		return GetPackageInstalled(&eleRoot, ModulesInstalled);
	}
	else
	{
		if (eleRoot.ElementExists(_T("Act")))
		{
			CXMLElement eleAct = eleRoot.GetItem(_T("Act"));
			ModulesInstalled.PackagesInstalled.ACT.Licensed = GetPackageInstalled(&eleAct, InstalledPlatforms.ACT);
		}

		if (eleRoot.ElementExists(_T("Fund")))
		{
			CXMLElement eleFund = eleRoot.GetItem(_T("Fund"));
			ModulesInstalled.PackagesInstalled.FUND.Licensed = GetPackageInstalled(&eleFund, InstalledPlatforms.FUND);
		}

		return ModulesInstalled.PackagesInstalled.ACT.Licensed || ModulesInstalled.PackagesInstalled.FUND.Licensed;
	}
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::GetPackageInstalled(CXMLElement* objDoc, stuModulesInstalled& ModInst)
{
	ASSERT(objDoc);
	if (objDoc->ElementExists(_T("modules")))
	{
		CXMLElement eleModules = objDoc->GetItem(_T("modules"));

		// These are the modules that have expired sometime in the past
		if (eleModules.ElementExists(_T("invalid")))
		{
			CXMLElement eleExpired = eleModules.GetItem(_T("invalid"));
			SetModuleInstalledSetting(&eleExpired, ModInst, CGBLSystemInformation::Expired); // begbert 02-15-2006 VS8: warning C4482
		}

		// These are the modules that are not activated or are demo and have not expired
		if (eleModules.ElementExists(_T("timelock")))
		{
			CXMLElement eleTimelocked = eleModules.GetItem(_T("timelock"));
			SetModuleInstalledSetting(&eleTimelocked, ModInst, CGBLSystemInformation::TimeLocked); // begbert 02-15-2006 VS8: warning C4482
			SetModuleInstalledSetting(&eleTimelocked, ModInst, CGBLSystemInformation::Licensed);
		}

		// These are modules that have been activated and are not demo and have not expired
		if (eleModules.ElementExists(_T("activated")))
		{
			CXMLElement eleActivated = eleModules.GetItem(_T("activated"));
			SetModuleInstalledSetting(&eleActivated, ModInst, CGBLSystemInformation::Licensed); // begbert 02-15-2006 VS8: warning C4482
		}
	}

	// These are custom modifications that are to be activated in this module
	if (objDoc->ElementExists(_T("custom")))
	{
		CXMLElement eleCustom = objDoc->GetItem(_T("custom"));
		SetCustomModInstalled(&eleCustom, ModInst);
	}

	SetModuleInstalledSetting(ModInst.OrderEntry, ModInst.SalesEntry);
	SetModuleInstalledSetting(ModInst.PointOfSale, ModInst.SalesEntry);
	//SetModuleInstalledSetting(ModInst.PointOfSale, ModInst.SalesEntry);
	//PTR.08.22.2005 - Layaway is always licensed if OE or POS is licensed.
	//if(ModInst.Layaway.Licensed)
	if (ModInst.SalesEntry.Licensed)
	{
		SetModuleInstalledSetting(ModInst.Layaway, CGBLSystemInformation::Licensed, true); // begbert 02-15-2006 VS8: warning C4482
		SetModuleInstalledSetting(ModInst.Layaway, CGBLSystemInformation::InCompany, true);
	}
	if (ModInst.LotTracking.Licensed)
		SetModuleInstalledSetting(ModInst.LotTracking, CGBLSystemInformation::InCompany, true);
	if (ModInst.ReportGenerator.Licensed && ModInst.GeneralLedger.Licensed)
		SetModuleInstalledSetting(ModInst.ReportGenerator, CGBLSystemInformation::InCompany, true);
	if (ModInst.SpecialtyShop.Licensed)
		SetModuleInstalledSetting(ModInst.SpecialtyShop, CGBLSystemInformation::InCompany, true);
	if (ModInst.CostCenter.Licensed && ModInst.GeneralLedger.Licensed)
		SetModuleInstalledSetting(ModInst.CostCenter, CGBLSystemInformation::InCompany, true);

	return objDoc->ElementExists(_T("validmodule"));
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::IsInstalled(UINT nModuleID) const
{
	switch (nModuleID)
	{
	case IDS_AP_MODULE:
		return ModulesInstalled.AccountsPayable.IsUsable;
	case IDS_AR_MODULE:
		return ModulesInstalled.AccountsReceivable.IsUsable;
	case IDS_BI_MODULE:
		return ModulesInstalled.DenaliBI.Licensed;	// For DenaliBI, being licensed is all you need.
	case IDS_BR_MODULE:
		return ModulesInstalled.BankReconciliation.IsUsable;
	case IDS_BM_MODULE:
		return ModulesInstalled.BillOfMaterials.IsUsable;
	case IDS_CC_MODULE:
		return ModulesInstalled.GeneralLedger.IsUsable && ModulesInstalled.CostCenter.IsUsable;
	case IDS_CM_MODULE:
		return ModulesInstalled.Commissions.IsUsable;
	case IDS_CT_MODULE:
		return ModulesInstalled.Controller.IsUsable;
	case IDS_EC_MODULE:
		return ModulesInstalled.ECommerce.IsUsable;
	case IDS_FA_MODULE:
		return ModulesInstalled.FinancialAnalysis.IsUsable;
	case IDS_GL_MODULE:
		return ModulesInstalled.GeneralLedger.IsUsable;
	case IDS_IN_MODULE:
		return ModulesInstalled.Inventory.IsUsable;
	case IDS_JC_MODULE:
		return ModulesInstalled.JobCost.IsUsable;
	case IDS_LY_MODULE:
		return ModulesInstalled.Layaway.IsUsable;
	case IDS_LT_MODULE:
		return ModulesInstalled.LotTracking.IsUsable;
	case IDS_MLI_MODULE:
		return ModulesInstalled.MultiLocationInventory.IsUsable;
	case IDS_OE_MODULE:
		return ModulesInstalled.OrderEntry.IsUsable;
	case IDS_PR_MODULE:
		return ModulesInstalled.Payroll.IsUsable;
	case IDS_PS_MODULE:
		return ModulesInstalled.PointOfSale.IsUsable;
	case IDS_PL_MODULE:
		return ModulesInstalled.Polling.IsUsable;
	case IDS_PO_MODULE:
		return ModulesInstalled.PurchaseOrder.IsUsable;
	case IDS_RG_MODULE:
		return ModulesInstalled.ReportGenerator.IsUsable;
	case IDS_SA_MODULE:
		return ModulesInstalled.SalesEntry.IsUsable;
	case IDS_SS_MODULE:
		return ModulesInstalled.SpecialtyShop.IsUsable;
	default:
		return false;
	}
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::GetModuleInstalledSetting(stuModulesInstalled& ModInst, UINT nModuleID, enuModuleSetting p_Part)
{
	switch (nModuleID)
	{
	case IDS_AP_MODULE:
		return GetModuleInstalledSetting(ModInst.AccountsPayable, p_Part);
	case IDS_AR_MODULE:
		return GetModuleInstalledSetting(ModInst.AccountsReceivable, p_Part);
	case IDS_BI_MODULE:
		return GetModuleInstalledSetting(ModInst.DenaliBI, p_Part);
	case IDS_BR_MODULE:
		return GetModuleInstalledSetting(ModInst.BankReconciliation, p_Part);
	case IDS_BM_MODULE:
		return GetModuleInstalledSetting(ModInst.BillOfMaterials, p_Part);
	case IDS_CM_MODULE:
		return GetModuleInstalledSetting(ModInst.Commissions, p_Part);
	case IDS_CC_MODULE:
		return GetModuleInstalledSetting(ModInst.CostCenter, p_Part);
	case IDS_CT_MODULE:
		return GetModuleInstalledSetting(ModInst.Controller, p_Part);
	case IDS_EC_MODULE:
		return GetModuleInstalledSetting(ModInst.ECommerce, p_Part);
	case IDS_FA_MODULE:
		return GetModuleInstalledSetting(ModInst.FinancialAnalysis, p_Part);
	case IDS_GL_MODULE:
		return GetModuleInstalledSetting(ModInst.GeneralLedger, p_Part);
	case IDS_IN_MODULE:
		return GetModuleInstalledSetting(ModInst.Inventory, p_Part);
	case IDS_JC_MODULE:
		return GetModuleInstalledSetting(ModInst.JobCost, p_Part);
	case IDS_LY_MODULE:
		return GetModuleInstalledSetting(ModInst.Layaway, p_Part);
	case IDS_LT_MODULE:
		return GetModuleInstalledSetting(ModInst.LotTracking, p_Part);
	case IDS_MLI_MODULE:
		return GetModuleInstalledSetting(ModInst.MultiLocationInventory, p_Part);
	case IDS_OE_MODULE:
		return GetModuleInstalledSetting(ModInst.OrderEntry, p_Part);
	case IDS_PR_MODULE:
		return GetModuleInstalledSetting(ModInst.Payroll, p_Part);
	case IDS_PS_MODULE:
		return GetModuleInstalledSetting(ModInst.PointOfSale, p_Part);
	case IDS_PL_MODULE:
		return GetModuleInstalledSetting(ModInst.Polling, p_Part);
	case IDS_PO_MODULE:
		return GetModuleInstalledSetting(ModInst.PurchaseOrder, p_Part);
	case IDS_RG_MODULE:
		return GetModuleInstalledSetting(ModInst.ReportGenerator, p_Part);
	case IDS_SA_MODULE:
		return GetModuleInstalledSetting(ModInst.SalesEntry, p_Part);
	case IDS_SS_MODULE:
		return GetModuleInstalledSetting(ModInst.SpecialtyShop, p_Part);
	default:
		return false;
	}
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::GetModuleInstalledSetting(stuLicensed& stuMod, enuModuleSetting p_Part)
{
	switch (p_Part)
	{ // begbert 02-15-2006 VS8: warning C4482
	case CGBLSystemInformation::Expired:
		return stuMod.Expired;
	case CGBLSystemInformation::InCompany:
		return stuMod.InCompany;
	case CGBLSystemInformation::IsUsable:
		return stuMod.IsUsable;
	case CGBLSystemInformation::Licensed:
		return stuMod.Licensed;
	case CGBLSystemInformation::TimeLocked:
		return stuMod.TimeLocked;
	default:
		return false;
	}
}

//-------------------------------------------------------------------
void CGBLSystemInformation::SetModuleInstalledSetting(CXMLElement* objDoc, stuModulesInstalled& ModInst, enuModuleSetting p_Part)
{
	ASSERT(objDoc);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_AP_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_AR_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_BR_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_BM_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_CM_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_CC_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_CT_MODULE, p_Part);
	//	SetModuleInstalledSetting(objDoc, ModInst, IDS_DX_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_EC_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_FA_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_GL_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_IN_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_JC_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_LY_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_LT_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_MLI_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_OE_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_PR_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_PS_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_PL_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_PO_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_RG_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_SA_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_SS_MODULE, p_Part);
	SetModuleInstalledSetting(objDoc, ModInst, IDS_BI_MODULE, p_Part);
}

//-------------------------------------------------------------------
void CGBLSystemInformation::SetModuleInstalledSetting(CXMLElement* objDoc, stuModulesInstalled& ModInst, UINT nModuleID, enuModuleSetting p_Part)
{
	ASSERT(objDoc);
	// assume that the item was set to false at some point and we are only changing to true.
	if (objDoc->ElementExists(ApplicationSettings.GetLicenseModule(nModuleID)))
		SetModuleInstalledSetting(ModInst, nModuleID, p_Part, true);
}

//-------------------------------------------------------------------
void CGBLSystemInformation::SetModuleInstalledSetting(stuLicensed& FromstuMod, stuLicensed& TostuMod)
{
	if (FromstuMod.Expired)
		SetModuleInstalledSetting(TostuMod, CGBLSystemInformation::Expired, true); // begbert 02-15-2006 VS8: warning C4482
	if (FromstuMod.InCompany)
		SetModuleInstalledSetting(TostuMod, CGBLSystemInformation::InCompany, true);
	if (FromstuMod.IsUsable)
		SetModuleInstalledSetting(TostuMod, CGBLSystemInformation::IsUsable, true);
	if (FromstuMod.Licensed)
		SetModuleInstalledSetting(TostuMod, CGBLSystemInformation::Licensed, true);
	if (FromstuMod.TimeLocked)
		SetModuleInstalledSetting(TostuMod, CGBLSystemInformation::TimeLocked, true);
}

//-------------------------------------------------------------------
void CGBLSystemInformation::SetModuleInstalledSetting(stuModulesInstalled& ModInst, UINT nModuleID, enuModuleSetting p_Part, bool p_bValue)
{
	switch (nModuleID)
	{
	case IDS_AP_MODULE:
		SetModuleInstalledSetting(ModInst.AccountsPayable, p_Part, p_bValue);
		break;
	case IDS_AR_MODULE:
		SetModuleInstalledSetting(ModInst.AccountsReceivable, p_Part, p_bValue);
		break;
	case IDS_BI_MODULE:
		SetModuleInstalledSetting(ModInst.DenaliBI, p_Part, p_bValue);
		break;
	case IDS_BR_MODULE:
		SetModuleInstalledSetting(ModInst.BankReconciliation, p_Part, p_bValue);
		break;
	case IDS_BM_MODULE:
		SetModuleInstalledSetting(ModInst.BillOfMaterials, p_Part, p_bValue);
		break;
	case IDS_CM_MODULE:
		SetModuleInstalledSetting(ModInst.Commissions, p_Part, p_bValue);
		break;
	case IDS_CC_MODULE:
		SetModuleInstalledSetting(ModInst.CostCenter, p_Part, p_bValue);
		break;
	case IDS_CT_MODULE:
		SetModuleInstalledSetting(ModInst.Controller, p_Part, p_bValue);
		break;
	case IDS_EC_MODULE:
		SetModuleInstalledSetting(ModInst.ECommerce, p_Part, p_bValue);
		break;
	case IDS_FA_MODULE:
		SetModuleInstalledSetting(ModInst.FinancialAnalysis, p_Part, p_bValue);
		break;
	case IDS_GL_MODULE:
		SetModuleInstalledSetting(ModInst.GeneralLedger, p_Part, p_bValue);
		break;
	case IDS_IN_MODULE:
		SetModuleInstalledSetting(ModInst.Inventory, p_Part, p_bValue);
		break;
	case IDS_JC_MODULE:
		SetModuleInstalledSetting(ModInst.JobCost, p_Part, p_bValue);
		break;
	case IDS_LY_MODULE:
		SetModuleInstalledSetting(ModInst.Layaway, p_Part, p_bValue);
		break;
	case IDS_LT_MODULE:
		SetModuleInstalledSetting(ModInst.LotTracking, p_Part, p_bValue);
		break;
	case IDS_MLI_MODULE:
		SetModuleInstalledSetting(ModInst.MultiLocationInventory, p_Part, p_bValue);
		break;
	case IDS_OE_MODULE:
		SetModuleInstalledSetting(ModInst.OrderEntry, p_Part, p_bValue);
		break;
	case IDS_PR_MODULE:
		SetModuleInstalledSetting(ModInst.Payroll, p_Part, p_bValue);
		break;
	case IDS_PS_MODULE:
		SetModuleInstalledSetting(ModInst.PointOfSale, p_Part, p_bValue);
		break;
	case IDS_PL_MODULE:
		SetModuleInstalledSetting(ModInst.Polling, p_Part, p_bValue);
		break;
	case IDS_PO_MODULE:
		SetModuleInstalledSetting(ModInst.PurchaseOrder, p_Part, p_bValue);
		break;
	case IDS_RG_MODULE:
		SetModuleInstalledSetting(ModInst.ReportGenerator, p_Part, p_bValue);
		break;
	case IDS_SA_MODULE:
		SetModuleInstalledSetting(ModInst.SalesEntry, p_Part, p_bValue);
		break;
	case IDS_SS_MODULE:
		SetModuleInstalledSetting(ModInst.SpecialtyShop, p_Part, p_bValue);
		break;
	}
}

//-------------------------------------------------------------------
void CGBLSystemInformation::SetModuleInstalledSetting(stuLicensed& stuMod, enuModuleSetting p_Part, bool p_bValue)
{
	switch (p_Part)
	{ // begbert 02-15-2006 VS8: warning C4482
	case CGBLSystemInformation::Expired:
		stuMod.Expired = p_bValue;
		break;
	case CGBLSystemInformation::InCompany:
		stuMod.InCompany = p_bValue;
		break;
	case CGBLSystemInformation::IsUsable:
		stuMod.IsUsable = p_bValue;
		break;
	case CGBLSystemInformation::Licensed:
		stuMod.Licensed = p_bValue;
		break;
	case CGBLSystemInformation::TimeLocked:
		stuMod.TimeLocked = p_bValue;
		break;
	}
}

//-------------------------------------------------------------------
void CGBLSystemInformation::SetCustomModInstalled(CXMLElement* objDoc, stuModulesInstalled& ModInst)
{
	ASSERT(objDoc);
	CXMLNodeList XMLNodes = objDoc->GetChildNodes();
	for (ULONG lIndex = 0; lIndex < XMLNodes.GetCount(); lIndex++)
		SetCustomModInstalledValue(ModInst, XMLNodes.GetItem(lIndex).GetInnerTextInt());
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::SetCustomModInstalledValue(stuModulesInstalled& ModInst, int nCustomMod)
{
	// Lets see if we have added this one before
	std::set<int>::iterator it = ModInst.CustomModifications.find(nCustomMod);
	// Add it if we are at the end
	if (it == ModInst.CustomModifications.end()) ModInst.CustomModifications.insert(nCustomMod);
	// Return if we added the item or not
	return (it == ModInst.CustomModifications.end());
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::CustomModInstalled(stuModulesInstalled& ModInst, int nCustomMod)
{
	std::set<int>::iterator it = ModInst.CustomModifications.find(nCustomMod);
	return it != ModInst.CustomModifications.end();
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::CustomModInstalled(int nCustomMod)
{
	return CustomModInstalled(this->ModulesInstalled, nCustomMod);
}

//-------------------------------------------------------------------
CString	CGBLSystemInformation::CustomeModsInstalled()
{
	CString szXML;
	szXML = _T("");
	std::set<int>::iterator it = this->ModulesInstalled.CustomModifications.begin();
	if (it == this->ModulesInstalled.CustomModifications.end()) return szXML; // There are no custom mod numbers
	szXML = _T("<CUSTOM>");
	for (; it != this->ModulesInstalled.CustomModifications.end(); it++)
	{
		szXML = szXML + _T("<MOD>");
		CString s;
		s.Format(_T("%i"), *it);
		szXML = szXML + s;
		szXML = szXML + _T("</MOD>");
	}
	szXML = szXML + _T("</CUSTOM>");
	return szXML;
}

//-------------------------------------------------------------------
CString	CGBLSystemInformation::AppendCustomeModsInstalled(const CString& szData)
{
	if (szData.GetLength() == 0)
		return szData;

	CXMLDocument docXMLData;
	docXMLData.LoadXML(szData);
	//KPM.2005.08.03 Adding in server info for every call
	CXMLElement ele = docXMLData.GetDocumentElement();
	std::set<int>::iterator it = this->ModulesInstalled.CustomModifications.begin();
	if (it == this->ModulesInstalled.CustomModifications.end()) return szData; // There are no custom mod numbers
	CXMLElement eleCustom = ele.AddNewChild(_T("CUSTOM"));
	for (; it != this->ModulesInstalled.CustomModifications.end(); it++)
	{
		CString s;
		s.Format(_T("%i"), *it);
		CXMLElement eleMod = eleCustom.AddNewChild("MOD", s);
	}
	return docXMLData.GetOuterXML();
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::LoadTipOfTheDay()
{
	this->TipOfTheDay.CurrentTipNumber = 0;
	this->TipOfTheDay.FileName = CGBLForm::GetPath() + _T("CMSTips.txt");
	this->TipOfTheDay.ShowTipsAtStartup = false;

	return true;
}

//-------------------------------------------------------------------
bool CGBLSystemInformation::vuiLicenced()
{
	FILE* pStream;
	CString szData = _T("");
	_wfopen_s(&pStream, CGBLForm::GetPath() + _T("DenaliOnTheWeb.License"), _T("r"));

	if (pStream)
	{
		if (fseek(pStream, 0, SEEK_SET) == 0)
		{
			CString szDataLine = _T("");
			bool bStop = false;

			while (!bStop) // Loop through all the lines in the file and load them in the szData variable
			{
				LPTSTR lpsz = szDataLine.GetBuffer(MAX_BUFLEN);
				// read line of data into lpsz string (if eof then stop otherwise add line to string)
				if (_fgetts(lpsz, MAX_BUFLEN, pStream) == NULL)
					bStop = true;
				else
					szData = szData + lpsz;

				// clear string buffer
				szDataLine.ReleaseBuffer();
			}
			fclose(pStream);  // Close the file
			if (!szData.IsEmpty())
			{
				CString decryptedData = CGBLMath::DecryptData(szData);  // Decrypt the data in the file
				CXMLParams Params;
				Params.SetXML(decryptedData); // Load the decrypted text into a XMLParams object
				CString Hash = Params.GetParamStr(_T("HASH"));  // Get the hash value from the data
				CString LicenseKey = Params.GetParamStr(_T("License"));
				Params.SetParamStr(_T("HASH"), EMPTY_STRING);   // Empty the hash value field to bring it back to what it was before the prior hash value was calculated
				CString DataWithoutHash = Params.GetXML();
				DataWithoutHash.Replace(_T("\t"), _T("  "));	// CXMLParams replaces the leasing spaces with tabs
				DataWithoutHash.TrimRight();					// CXMLParams adds a carriage return/line feed at the end
				CString localHash = CDotNetMethods::GetHash(DataWithoutHash);  // Recalculate the hash code, it should be the same as the one that was stored in the file.
				int version = CDotNetMethods::GetLicenseVersion(LicenseKey);   // Get the version number from the license key
				return Hash == localHash && version == CAV_LICENSEVERSION; // Check that the two hash values are the same
			}
		}
	}

	return false;
}

//-------------------------------------------------------------------
CString CGBLSystemInformation::GetSegmentInfoString(void) const
{
	// This will build the segment info string from the segment information
	CString szSegmentInfo(_T(""));
	CString szTmp(_T(""));
	for (int i = 0; i < 6; i++)
	{
		szTmp.Format(_T("%02d"), this->SegmentSetup.Segments[i].Length);
		szSegmentInfo.Append(szTmp);
	}

	return szSegmentInfo;
}

//-------------------------------------------------------------------
// 1-20038 BK 7/11/05 if no companies in CMSCompanies return error number
UINT CGBLSystemInformation::GetCompaniesList()
{
	CXML		clsXMLSender;
	CXMLParams	clsXMLCommand;
	CXMLParams	clsXMLData;

	CXMLParams::MakeXMLCommand(&clsXMLCommand, _T("GETALLCOMPANIES"), this->ApplicationSettings.ModuleStringID, _T(""));

	bool bNonModule = !CGBLForm::IsLicenseServerRequiredForModule();	//PTR.07.26.2005

	if (ModulesInstalled.PackagesInstalled.FUND.Licensed || bNonModule)
		clsXMLData.MakeParam(_T("FundInstalled"), _T("1"));
	else
		clsXMLData.MakeParam(_T("FundInstalled"), _T("0"));

	clsXMLData.MakeParam(_T("TenantID"), ApplicationSettings.TenantID);
	// 1-36004 BK 6/10/11 added params for mli in fund and act
	clsXMLData.MakeParam(_T("ActInstalled"), ModulesInstalled.PackagesInstalled.ACT.Licensed || bNonModule);
	//clsXMLData.MakeParam(_T("MLIInstalled"), InstalledPlatforms.ACT.MultiLocationInventory.Licensed || bNonModule);
	clsXMLData.MakeParam(_T("ActMLIInstalled"), InstalledPlatforms.ACT.MultiLocationInventory.Licensed || bNonModule);
	clsXMLData.MakeParam(_T("FundMLIInstalled"), InstalledPlatforms.FUND.MultiLocationInventory.Licensed || bNonModule);

	CXMLParams::AppendXMLConnection(&clsXMLData);
	stuErrorReturn clsErrorReturn;
	CString bstrReturn = clsXMLSender.SendXML(clsXMLCommand.GetXML(), clsXMLData.GetXML(), clsErrorReturn).copy();

	CDataSet clsDataset;
	clsDataset.LoadFromXML(bstrReturn);

	//If there was not an error
	if (!clsDataset.TableExists(CMSStrings::XMLTags::Error))
	{
		if (clsDataset.TableExists(CMSStrings::XMLTags::Table))
			clsDataset.GetTable(CMSStrings::XMLTags::Table, &this->UserInformation.ValidCompanies);

		// 1-35714 BK 5/13/11 if this is a fund company and strSegmentInfo = "500000000000" change to "000000000000"
		// because this means segments are not setup
		if (ModulesInstalled.PackagesInstalled.FUND.Licensed)
		{
			if (UserInformation.ValidCompanies.getRowCount() && UserInformation.ValidCompanies.getColumnCount())
			{
				CDataRow  objDR;
				for (long nRow = 0; nRow < UserInformation.ValidCompanies.getRowCount(); nRow++)
				{
					UserInformation.ValidCompanies.GetRow(nRow, &objDR);
					bool    bFundCompany = objDR.XMLBOOL(_T("bolFundCompany"), false);
					CString szSegmentInfo = objDR.XMLStr(_T("strSegmentInfo"), false);
					if (bFundCompany && szSegmentInfo == _T("500000000000"))
					{
						szSegmentInfo = _T("000000000000");
						objDR.SetRowValue(_T("strSegmentInfo"), szSegmentInfo);
					}
					objDR.Dispose();
				}
			}
		}
	}
	else
		return clsErrorReturn.uiErrorID;

	return 0;
}

//-------------------------------------------------------------------
CString	CGBLSystemInformation::GetRegisteredOrganization(void)
{
	CRegKey  clsReader;
	CString	  szValue(_T(""));
	ULONG  lLen = 1024;

#if defined _M_IX86
	const CString	szRegPath(_T("SOFTWARE\\Cougar Mountain Software\\Denali\\CurrentVersion"));
#elif defined _M_X64
	const CString	szRegPath(_T("SOFTWARE\\WOW6432Node\\Cougar Mountain Software\\Denali\\CurrentVersion"));
#else
#error "Neither _M_X86 or _M_X64 are defined"
#endif

	if (clsReader.Open(HKEY_LOCAL_MACHINE, szRegPath, KEY_READ) == ERROR_SUCCESS)
	{
		/*LONG  lStatus =*/
		clsReader.QueryStringValue(_T("RegisteredOrganization"), szValue.GetBufferSetLength((int)(lLen + 1)), &lLen);
		clsReader.Close();
	}

	szValue.ReleaseBuffer();
	szValue.Trim();
	szValue.FreeExtra();

	if (szValue.IsEmpty())
		szValue = CGBLResources::GetResourceString(IDS_GBL_DEVL_QA);

	return szValue;
}

//-------------------------------------------------------------------
CString CGBLSystemInformation::GetRegisteredOwner(void)
{
	CRegKey	clsReader;
	CString	 szValue(_T(""));
	ULONG  lLen = 1024;

#if defined _M_IX86
	const CString	szRegPath(_T("SOFTWARE\\Cougar Mountain Software\\Denali\\CurrentVersion"));
#elif defined _M_X64
	const CString	szRegPath(_T("SOFTWARE\\WOW6432Node\\Cougar Mountain Software\\Denali\\CurrentVersion"));
#else
#error "Neither _M_X86 or _M_X64 are defined"
#endif

	if (clsReader.Open(HKEY_LOCAL_MACHINE, szRegPath, KEY_READ) == ERROR_SUCCESS)
	{
		/*LONG  lStatus = */
		clsReader.QueryStringValue(_T("RegisteredOwner"), szValue.GetBufferSetLength((int)(lLen + 1)), &lLen);
		clsReader.Close();
	}

	szValue.ReleaseBuffer();
	szValue.Trim();
	szValue.FreeExtra();

	if (szValue.IsEmpty())
		szValue = CGBLResources::GetResourceString(IDS_GBL_DEVL_QA_USERNAME);

	return szValue;
}

//-------------------------------------------------------------------
CString	CGBLSystemInformation::DisplayableBatchCode(void) const
{
	CString szBatchCode(_T(""));

	// begbert 08-07-2015 PBI 28383 : JC___ is always displayed as single batch.
	if (CurrentBatch.BatchCode == L"JC___") return RESSTRING(IDS_SINGLE);

	//Multi Batch mode and the batch isn't empty
	if (ApplicationSettings.IsMultiBatch() && !CurrentBatch.BatchCode.IsEmpty())
		szBatchCode = CurrentBatch.BatchCode;

	//Single batch mode
	else if (ApplicationSettings.IsSingleBatch())
	{
		//KPM.2005.04.06 Show the other types of batches (OEBOT, OERET, OEQUT) if they are selected
		//in single batch mode.
		CurrentBatch.BatchCode;
		if (CurrentBatch.BatchCode == SINGLE_BATCH || CurrentBatch.BatchCode == EMPTY_STRING)
			szBatchCode = CGBLResources::GetResourceString(IDS_SINGLE);
		else
			szBatchCode = CurrentBatch.BatchCode;
	}
	else
		szBatchCode = CGBLResources::GetResourceString(IDS_NONE);

	return szBatchCode;
}

//-------------------------------------------------------------------
// begbert 09-28-2018 PBI 47270 : Replaced virtually all of the icons
//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Agreement() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_AGREEMENT, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ArrowDown() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_ARROW_DOWN, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ArrowLeft() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_ARROW_LEFT, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::BackUp() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BACK_UP, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Bank() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BANK, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Bills() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BILLS, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Blocks() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BLOCKS, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Brush() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BRUSH, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Budget() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BUDGET, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Build() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BUILD, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CalculatorCheck() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CALCULATOR_CHECK, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CalculatorRefresh() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CALCULATOR_REFRESH, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Calculator() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CALCULATOR, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CalendarAdd() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CALENDAR_ADD, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CalendarCheck() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CALENDAR_CHECK, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CalendarDelete() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CALENDAR_DELETE, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CalendarEdit() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CALENDAR_EDIT, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Calendar() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CALENDAR, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CashRegisterCheck() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CASH_REGISTER_CHECK, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CashRegisterConfigure() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CASH_REGISTER_CONFIGURE, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CashRegister() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CASH_REGISTER, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ChangeOrder() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CHANGE_ORDER, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ChartBar() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CHART_BAR, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ChartLine() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CHART_LINE, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ChartPie() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CHART_PIE, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Check() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CHECK, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CheckbookAdd() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CHECKBOOK_ADD, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CheckbookDelete() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CHECKBOOK_DELETE, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Checkbook() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CHECKBOOK, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ClipboardAdd() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CLIPBOARD_ADD, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ClipboardEdit() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CLIPBOARD_EDIT, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ClipboardTool() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CLIPBOARD_TOOL, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ClockArrows() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CLOCK_ARROWS, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ConfigureData() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CONFIGURE_DATA, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Configure() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CONFIGURE, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Copy() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_COPY, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CreditCardInformation() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CREDIT_CARD_INFORMATION, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CreditCard() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CREDIT_CARD, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::DatabaseAdd() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DATABASE_ADD, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::DataHistorical() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DATA_HISTORICAL, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::DebitCard() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DEBIT_CARD, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Delete() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DELETE, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::DiscountAdd() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DISCOUNT_ADD, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Discount() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DISCOUNT, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Drill() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DRILL, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Euro() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_EURO, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Expensify() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_EXPENSIFY, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::FileAdd() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_FILE_ADD, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::FileCheck() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_FILE_CHECK, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::FileConfigure() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_FILE_CONFIGURE, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::FinancialTransactionCheck() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_FINANCIAL_TRANSACTION_CHECK, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::FundEdit() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_FUND_EDIT, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Fund() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_FUND, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GiftCardAdd() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_GIFT_CARD_ADD, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GiftCardEdit() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_GIFT_CARD_EDIT, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GiftCardInformation() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_GIFT_CARD_INFORMATION, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GiftCardRefresh() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_GIFT_CARD_REFRESH, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GlobeDate() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_GLOBE_DATE, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GlobeEdit() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_GLOBE_EDIT, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::HourGlass() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_HOUR_GLASS, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::IdentificationWarning() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_IDENTIFICATION_WARNING, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Identification() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_IDENTIFICATION, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Image() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_IMAGE, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Inspect() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_INSPECT, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::InventoryCost() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_INVENTORY_COST, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::InventoryTransfer() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_INVENTORY_TRANSFER, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::LedgerAdd() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_LEDGER_ADD, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::LedgerConfigure() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_LEDGER_CONFIGURE, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::LedgerEdit() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_LEDGER_EDIT, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::LedgerRefresh() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_LEDGER_REFRESH, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Ledger() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_LEDGER, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::MagnifyingGlass() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_MAGNIFYING_GLASS, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Mailbox() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_MAILBOX, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::MoneyExchange() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_MONEY_EXCHANGE, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::MoneyTransfer() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_MONEY_TRANSFER, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::MoneyWithdraw() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_MONEY_WITHDRAW, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Money() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_MONEY, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Options() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_OPTIONS, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PayCheck() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PAY_CHECK, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PayMoney() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PAY_MONEY, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PeopleConstructionGroup() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PEOPLE_CONSTRUCTION_GROUP, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PeopleConstructionMan() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PEOPLE_CONSTRUCTION_MAN, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PeopleCopy() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PEOPLE_COPY, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PeopleFinancialAdd() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PEOPLE_FINANCIAL_ADD, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PeopleFinancial() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PEOPLE_FINANCIAL, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PeopleManFront() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PEOPLE_MAN_FRONT, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PeoplePasswordSecurity() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PEOPLE_PASSWORD_SECURITY, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PeopleWomanFront() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PEOPLE_WOMAN_FRONT, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PrinterLabels() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PRINTER_LABELS, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PrinterRestricted() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PRINTER_RESTRICTED, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Printer() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PRINTER, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PuzzleAdd() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PUZZLE_ADD, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::QuestionMark() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_QUESTION_MARK, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::RawMaterials() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_RAW_MATERIALS, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ReceiveGoods() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_RECEIVE_GOODS, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Refresh() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_REFRESH, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Renumber() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_RENUMBER, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Save() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SAVE, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Scale() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SCALE, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Scanner() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SCANNER, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SendMail() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SEND_MAIL, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ShoppingCartAdd() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SHOPPING_CART_ADD, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SpreadsheetArrowRight() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SPREADSHEET_ARROW_RIGHT, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SpreadsheetEdit() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SPREADSHEET_EDIT, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::StorefrontConfigure() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_STOREFRONT_CONFIGURE, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::StorefrontInformation() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_STOREFRONT_INFOMATION, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::StorefrontLock() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_STOREFRONT_LOCK, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::StorefrontRefresh() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_STOREFRONT_REFRESH, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Tools() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TOOLS, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::TransactionAdd() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRANSACTION_ADD, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::TransactionDelete() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRANSACTION_DELETE, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::TransactionFinancialCheck() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRANSACTION_FINANCIAL_CHECK, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::TransactionFinancial() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRANSACTION_FINANCIAL, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::TransactionSearch() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRANSACTION_SEARCH, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Transaction() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRANSACTION, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Trash() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRASH, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::TruckAdd() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRUCK_ADD, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::TruckRefresh() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRUCK_REFRESH, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Truck() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRUCK, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::W2Form() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_W2_FORM, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Wallet() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_WALLET, &hic); ASSERT(hic);

	return hic;
}

//-------------------------------------------------------------------
// 16x16 Icons - Modules
HICON	CGBLSystemInformation::stuIcons::get_ModuleIcon(void) const
{
	return Current16();
}

//-------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::AP16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_AP, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::AR16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_AR, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::BR16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BR, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CT16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CONTROLLER, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GL16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_GL, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::IN16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_INV, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::OE16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_OE, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PO16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PO, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PS16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_POS, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PR16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PR, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SE16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SLS, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SM16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SLS, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Current16() const
{
	static HICON	hic = NULL;
	if (!hic)
		hic = this->Module16(g_pGBLSystemInformationCMSDll->ApplicationSettings.ModuleID);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Module16(UINT uinModuleID) const
{
	HICON	hic = NULL;

	switch (uinModuleID)
	{
	case IDS_AP_MODULE:
	case IDS_AP_POST_BILLS:
	case IDS_AP_POST_PAYMENTS:
		hic = this->AP16();
		break;
	case IDS_AR_MODULE:
		hic = this->AR16();
		break;
	case IDS_BR_MODULE:
		hic = this->BR16();
		break;
	case IDS_CT_MODULE:
	case IDS_GB_MODULE:
		hic = this->CT16();
		break;
		//case IDS_DX_MODULE:
		//	hic = this->DX16();
		//	break;
	case IDS_GL_MODULE:
		hic = this->GL16();
		break;
	case IDS_IN_MODULE:
		hic = this->IN16();
		break;
		//	PBI 24421	04/21/2015	mvt - Set up Job Cost integration
	case IDS_JC_MODULE:
		hic = this->JC16();
		break;
	case IDS_PO_MODULE:
		hic = this->PO16();
		break;
	case IDS_PR_MODULE:
		hic = this->PR16();
		break;
	case IDS_SA_MODULE:
	case IDS_SAE_MODULE:
		hic = this->SM16();
		break;
	case IDS_OE_MODULE:
		hic = this->OE16();
		break;
	case IDS_PS_MODULE:
		hic = this->PS16();
		break;
	}
	return hic;
}
// end 16x16 Icons - Modules

//---------------------------------------------------------------------------
// 32x32 Icons - Modules
HICON	CGBLSystemInformation::stuIcons::AP32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_AP, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::AR32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_AR, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::BR32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BR, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CT32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CONTROLLER, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GL32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_GL, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::IN32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_INV, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PO32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PO, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PR32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PR, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SM32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SLS, &hic); ASSERT(hic);

	return hic;
}

////---------------------------------------------------------------------------
//HICON	CGBLSystemInformation::stuIcons::Current32() const
//{
//	static HICON	hic = NULL;
//	if (!hic)
//		hic = this->Module32(g_pGBLSystemInformationCMSDll->ApplicationSettings.ModuleID);
//
//	return hic;
//}

////---------------------------------------------------------------------------
//HICON	CGBLSystemInformation::stuIcons::Module32(UINT uinModuleID) const
//{
//	HICON	hic = NULL;
//
//	switch (uinModuleID)
//	{
//	case IDS_AP_MODULE:
//	case IDS_AP_POST_BILLS:
//	case IDS_AP_POST_PAYMENTS:
//		hic = this->AP32();
//		break;
//	case IDS_AR_MODULE:
//		hic = this->AR32();
//		break;
//	case IDS_BR_MODULE:
//		hic = this->BR32();
//		break;
//	case IDS_CT_MODULE:
//	case IDS_GB_MODULE:
//		hic = this->CT32();
//		break;
//		//case IDS_DX_MODULE:
//		//	hic = this->DX32();
//		//	break;
//	case IDS_GL_MODULE:
//		hic = this->GL32();
//		break;
//	case IDS_IN_MODULE:
//		hic = this->IN32();
//		break;
//		//case IDS_JC_MODULE:
//		//	hic = this->JC32();
//		//	break;
//	case IDS_PO_MODULE:
//		hic = this->PO32();
//		break;
//	case IDS_PR_MODULE:
//		hic = this->PR32();
//		break;
//	case IDS_OE_MODULE:
//	case IDS_PS_MODULE:
//	case IDS_SA_MODULE:
//	case IDS_SAE_MODULE:
//		hic = this->SM32();
//		break;
//	}
//
//	return hic;
//}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Reports() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRANSACTION, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
// global/general icons (Version 3.0 Facelift BK 6/2010)
HICON	CGBLSystemInformation::stuIcons::ChangeDateLocal() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CALENDAR_EDIT, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Lookups() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_MAGNIFYING_GLASS, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ModulePrefs() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CONFIGURE, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Post() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_LEDGER_ADD, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::DisplayTableInfo() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_INSPECT, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GenerateFromRecur() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CLOCK_ARROWS, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Find() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_MAGNIFYING_GLASS, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GenerateFromExternal() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_ARROW_LEFT, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SelectBatch() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_FILE_CHECK, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SetupBatch() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_FILE_ADD, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Clear() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BRUSH, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Purge() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRASH, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ResetTotals() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_REFRESH, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Recalculate() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CALCULATOR_REFRESH, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GlobalChange() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_GLOBE_EDIT, &hic); ASSERT(hic);

	return hic;
}

////---------------------------------------------------------------------------
//HICON	CGBLSystemInformation::stuIcons::Recall() const
//{
//	static HICON	hic = NULL;
//	if (!hic)
//		CGBLResources::GetResourceIcon(IDI_TRANSACTION_SEARCH, &hic); ASSERT(hic);
//
//	return hic;
//}

////---------------------------------------------------------------------------
//HICON	CGBLSystemInformation::stuIcons::VoidDelete() const
//{
//	static HICON	hic = NULL;
//	if (!hic)
//		CGBLResources::GetResourceIcon(IDI_DELETE, &hic); ASSERT(hic);
//
//	return hic;
//}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SetupRecurring() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CLOCK_ARROWS, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ImportExport() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_LEDGER_REFRESH, &hic); ASSERT(hic);

	return hic;
}

////---------------------------------------------------------------------------
//HICON	CGBLSystemInformation::stuIcons::SetupScreenLayout() const
//{
//	static HICON	hic = NULL;
//	if (!hic)
//		CGBLResources::GetResourceIcon(IDI_WINDOW_CONFIGURATION, &hic); ASSERT(hic);
//
//	return hic;
//}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Stop() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DELETE, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::UserLogin() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_IDENTIFICATION, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::UsersLoggedIn() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PEOPLE_MAN_FRONT, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SelectCompany() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_STOREFRONT_CONFIGURE, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CompanyInfo() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_STOREFRONT_INFOMATION, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::InstallModules() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PUZZLE_ADD, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ChangeDateGlobal() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_GLOBE_DATE, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::FiscalCalendar() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CALENDAR, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ChangePassword() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PEOPLE_PASSWORD_SECURITY, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::AddChangeUser() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PEOPLE_COPY, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::BackupCompany() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_STOREFRONT_LOCK, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::RestoreCompany() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_STOREFRONT_REFRESH, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CMSCurrencies() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_MONEY, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ExchangeRates() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_MONEY_EXCHANGE, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ProcessPayroll() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PAY_CHECK, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::BackupLog() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BACK_UP, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::APVendor() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRUCK, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::APEnterBills() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BILLS, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::APAdjustPayBills() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRANSACTION_FINANCIAL_CHECK, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::APDiscountMessage() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DISCOUNT, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::APPrepareAutoPayments() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_FINANCIAL_TRANSACTION_CHECK, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::APProcessPrepaids() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CALENDAR_CHECK, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::APAdjustPosted() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CALENDAR_EDIT, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GenerateElectronic() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_MONEY_TRANSFER, &hic); ASSERT(hic);

	return hic;
}

////---------------------------------------------------------------------------
//HICON	CGBLSystemInformation::stuIcons::SaveWithoutPrinting() const
//{
//	static HICON	hic = NULL;
//	if (!hic)
//		CGBLResources::GetResourceIcon(IDI_PRINTER_RESTRICTED, &hic); ASSERT(hic);
//
//	return hic;
//}

////---------------------------------------------------------------------------
//HICON	CGBLSystemInformation::stuIcons::ArrowLeft() const
//{
//	static HICON	hic = NULL;
//	if (!hic)
//		CGBLResources::GetResourceIcon(IDI_ARROW_LEFT, &hic); ASSERT(hic);
//
//	return hic;
//}

////---------------------------------------------------------------------------
//HICON	CGBLSystemInformation::stuIcons::ArrowRight() const
//{
//	static HICON	hic = NULL;
//	if (!hic)
//		CGBLResources::GetResourceIcon(IDI_ARROW_RIGHT, &hic); ASSERT(hic);
//
//	return hic;
//}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ARMultipleShipAdderss() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRUCK_ADD, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ARCustomers() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PEOPLE_WOMAN_FRONT, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::AREnterBillsPayments() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRANSACTION_FINANCIAL, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ARCalculateFinanceChg() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CALCULATOR, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::BRBankAccount() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BANK, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::BRBankRegister() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CHECKBOOK, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::BREnterBankActivity() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CHECKBOOK_ADD, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::BRVoidBankActivity() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CHECKBOOK_DELETE, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::BRReconcileBankAcct() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SCALE, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::BRPayeeCardFile() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PEOPLE_FINANCIAL, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::BRAddFromAP() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PEOPLE_FINANCIAL_ADD, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GLAccounts() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_LEDGER, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GLJournalEntry() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_LEDGER_EDIT, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GLAdjustAccounts() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_LEDGER_CONFIGURE, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GLProcessAllocations() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SPREADSHEET_EDIT, &hic); ASSERT(hic);
	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GLSpreadsheetExport() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SPREADSHEET_ARROW_RIGHT, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GLCloseYear() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CALENDAR_CHECK, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GLSetupFund() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_FUND_EDIT, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GLCloseFund() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_FUND, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
// begbert 02-04-2011 FUND : Added for Budget Management form.
HICON	CGBLSystemInformation::stuIcons::GLBudgetManagement() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BUDGET, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::INRecalculateDate() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CALENDAR, &hic); ASSERT(hic);
	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::INSelectSerialNos() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SCANNER, &hic); ASSERT(hic);
	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::INStock() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_INVENTORY_TRANSFER, &hic); ASSERT(hic);
	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::INStockAlias() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CONFIGURE, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::INAdjustStockQty() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CLIPBOARD_EDIT, &hic); ASSERT(hic);
	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::INImportAdjustments() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CLIPBOARD_ADD, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::INVarianceAmount() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CALCULATOR_CHECK, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::INSetupKits() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BLOCKS, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ShipToButton() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRUCK_REFRESH, &hic); ASSERT(hic);

	return hic;
}

////---------------------------------------------------------------------------
//HICON	CGBLSystemInformation::stuIcons::SAGiftCardOptions() const
//{
//	static HICON	hic = NULL;
//	if (!hic)
//		CGBLResources::GetResourceIcon(IDI_GIFT_CARD_CONFIGURATION, &hic); ASSERT(hic);
//
//	return hic;
//}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::BillingAddress() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_MAILBOX, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::POAddItems() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SHOPPING_CART_ADD, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::POReceiveItems() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_RECEIVE_GOODS, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::POEnterPurchaseOrders() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRANSACTION_ADD, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::OEEnterOrders() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRANSACTION_ADD, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::OERepairBatchUtility() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_FILE_CONFIGURE, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PSEnterSales() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CASH_REGISTER, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PSBalanceRegister() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CASH_REGISTER_CHECK, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PSRepairRegister() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CASH_REGISTER_CONFIGURE, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
// begbert 02-08-2010 CDtails : Added this
HICON	CGBLSystemInformation::stuIcons::DTails() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CHART_BAR, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SalesDTails() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CHART_LINE, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Utility32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_ATOM, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
// code windows - Options>Codes
HICON	CGBLSystemInformation::stuIcons::PrefCode32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CONFIGURE_DATA, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::FatalServer() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DELETE, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::FatalClient() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DELETE, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::XMLClient() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DELETE, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::XMLServer() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DELETE, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Help16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_QUESTION_MARK, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SAECash32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_MONEY, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SAECC32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CREDIT_CARD, &hic); ASSERT(hic);
	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SAECharge32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_AGREEMENT, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SAECheck32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CHECKBOOK, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SAEDebit32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DEBIT_CARD, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
// PBI 17783 BK 3/12/14 Added Credit Card Swiper icon
HICON	CGBLSystemInformation::stuIcons::SAECC_Swiper() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CREDIT_CARD_SWIPER, &hic); ASSERT(hic);
	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ForeignCur32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_EURO, &hic); ASSERT(hic);

	return hic;
}

////---------------------------------------------------------------------------
//HICON	CGBLSystemInformation::stuIcons::PreAuthorization() const
//{
//	static HICON	hic = NULL;
//	if (!hic)
//		CGBLResources::GetResourceIcon(IDI_CREDIT_CARD_INFORMATION, &hic); ASSERT(hic);
//
//	return hic;
//}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SAEDeposit32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_MONEY, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SAEAltTender32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_WALLET, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GiftCert32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_GIFT_CARD, &hic); ASSERT(hic);

	return hic;
}

////---------------------------------------------------------------------------
//HICON	CGBLSystemInformation::stuIcons::DebitCard32() const
//{
//	static HICON	hic = NULL;
//	if (!hic)
//		CGBLResources::GetResourceIcon(IDI_DEBIT_CARD, &hic); ASSERT(hic);
//
//	return hic;
//}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::DepositStatus32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_MONEY, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::RefreshRescan() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_REFRESH, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SAEBack32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_ARROW_LEFT, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SAECancel32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DELETE, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Done32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CHECK, &hic); ASSERT(hic);

	return hic;
}

////---------------------------------------------------------------------------
//HICON	CGBLSystemInformation::stuIcons::AddTip() const
//{
//	static HICON	hic = NULL;
//	if (!hic)
//		CGBLResources::GetResourceIcon(IDI_CREDIT_CARD_GOLD_ADD, &hic); ASSERT(hic);
//
//	return hic;
//}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SSAgeVerification() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_IDENTIFICATION_WARNING, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SAEAuthorize32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRANSACTION_FINANCIAL_CHECK, &hic); ASSERT(hic);

	return hic;
}

////---------------------------------------------------------------------------
//HICON	CGBLSystemInformation::stuIcons::PaidOut32() const
//{
//	static HICON	hic = NULL;
//	if (!hic)
//		CGBLResources::GetResourceIcon(IDI_PAY_MONEY, &hic); ASSERT(hic);
//
//	return hic;
//}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::OpenDrawer32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CASH_REGISTER, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::DiscountInvoiceTotal() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DISCOUNT, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::DiscountSinceLast32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DISCOUNT_ADD, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::DiscountLineItem32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DISCOUNT, &hic); ASSERT(hic);

	return hic;
}

////---------------------------------------------------------------------------
//HICON	CGBLSystemInformation::stuIcons::NewLayaway32() const
//{
//	static HICON	hic = NULL;
//	if (!hic)
//		CGBLResources::GetResourceIcon(IDI_CALENDAR_ADD, &hic); ASSERT(hic);
//
//	return hic;
//}

////---------------------------------------------------------------------------
//HICON	CGBLSystemInformation::stuIcons::PayLayaway32() const
//{
//	static HICON	hic = NULL;
//	if (!hic)
//		CGBLResources::GetResourceIcon(IDI_CALENDAR, &hic); ASSERT(hic);
//
//	return hic;
//}

////---------------------------------------------------------------------------
//HICON	CGBLSystemInformation::stuIcons::PickupLayaway32() const
//{
//	static HICON	hic = NULL;
//	if (!hic)
//		CGBLResources::GetResourceIcon(IDI_CALENDAR_CHECK, &hic); ASSERT(hic);
//
//	return hic;
//}

////---------------------------------------------------------------------------
//HICON	CGBLSystemInformation::stuIcons::CancelLayaway32() const
//{
//	static HICON	hic = NULL;
//	if (!hic)
//		CGBLResources::GetResourceIcon(IDI_CALENDAR_DELETE, &hic); ASSERT(hic);
//
//	return hic;
//}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::LayawaySchedule32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CALENDAR, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GCPurchase32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_GIFT_CARD, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GCIncrement32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_GIFT_CARD_ADD, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GCBalanceInq32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_GIFT_CARD_INFORMATION, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GCBalanceTransfer32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_GIFT_CARD_REFRESH, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::GCAdjustment32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_GIFT_CARD_EDIT, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SADeviceTest() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CHECK, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SALoadDevice() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_ARROW_DOWN, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SAInventoryPricing() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_INVENTORY_COST, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::TenderSale() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_MONEY, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::UnprintedItems() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TRANSACTION_DELETE, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PreviewPrintFormat() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_MAGNIFYING_GLASS, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::DrillDown32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DRILLDOWN_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::DrillDown16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DRILLDOWN_16X16, &hic); ASSERT(hic);

	return hic;
}

/*
Version 3.0 Facelift - removed the following icons

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::DX16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DX_16X16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::DX32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DX_32X32, &hic); ASSERT(hic);

	return hic;
}
*/
//	PBI 24421	04/21/2015	mvt - Set up Job Cost integration

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::JC16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_JC, &hic); ASSERT(hic);
	//        CGBLResources::GetResourceIcon(IDI_JC_16X16, &hic); ASSERT(hic);

	return hic;
}

////---------------------------------------------------------------------------
//HICON	CGBLSystemInformation::stuIcons::JC32() const
//{
//	static HICON	hic = NULL;
//	if (!hic)
//		CGBLResources::GetResourceIcon(IDI_JC, &hic); ASSERT(hic);
//	//        CGBLResources::GetResourceIcon(IDI_JC_32X32, &hic); ASSERT(hic);
//
//	return hic;
//}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CMS() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CMS, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::DenaliBI() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_DENALIBI, &hic); ASSERT(hic);

	return hic;
}

////---------------------------------------------------------------------------
//HICON	CGBLSystemInformation::stuIcons::SendMail() const
//{
//	static HICON	hic = NULL;
//	if (!hic)
//		CGBLResources::GetResourceIcon(IDI_SEND_MAIL, &hic); ASSERT(hic);
//
//	return hic;
//}

/*
---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::OE32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_OE, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PS32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_POS, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SE16(bool bIsOE) const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SA, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SE32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SA, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
// begbert 05-03-2010 PA_DSS : Added this
HICON	CGBLSystemInformation::stuIcons::Credit_Card_Gold() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CREDIT_CARD, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Adjustments32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_ADJUSTMENTS_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Adjustments16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_ADJUSTMENTS_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Zoom16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_ZOOM_16X16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::INStock16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_STOCK_ITEMS, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::INStock32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_INSTOCK_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::POAdd16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_POADD_16X16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::POAdd32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_POADD_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::POBuyer16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_POBUYER_16X16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::POBuyer2_16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_POBUYER2_16X16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::POBuyer2_32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_POBUYER2_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::POEdit16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_POEDIT_16X16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::POEdit32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_POEDIT_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::POPrintEdit32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_POPREDIT_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::POPrintLabels32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_POPRLABELS_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::POReceive16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PORECEIVE_16X16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::POReceive32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PORECEIVE_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::POSer16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_POSER_16X16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::POSer32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_POSER_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PREmpData16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PREMPDATA_16X16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PREmpData32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PREMPDATA_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Edit16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BRUSH, &hic); ASSERT(hic);
	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Edit32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BRUSH, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Blank16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BLANK16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Blank32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BLANK32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::TTCritical6() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TTCRITICAL16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::TTInformation16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TTINFORMATION16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::TTFatal16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_TTFATAL16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Servers() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SERVERS, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ValidClient() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_VALIDCLIENT, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ValidServer() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_VALIDSERVER, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::XML16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_XML16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SetupWizard16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SETUPWIZARD, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Camera() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CAMERA, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SAEOK32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SAE_OK, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SAEVoid16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SAEVOID16X16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SAEVoid32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SAEVOID32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Back32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BACK_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Cancel32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CANCEL_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CashReg16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CASHREG_16X16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CashReg48() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CASHREG_48X48, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CreditCard32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CREDIT_CARD, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CustHist32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CUSTHIST_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::FinHist32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_FINHIST_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Padlock32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PADLOCK, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::SalesHist32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SALESHIST_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ShipTo32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SHIPTO_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ViewPic16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_VIEWPIC_16X16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::ViewPic32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_VIEWPIC_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
// 2/19/07 BK new icons bank register
HICON	CGBLSystemInformation::stuIcons::ViewPrinter16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_VIEW_PR16X16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
// 2/19/07 BK new icons bank register
HICON	CGBLSystemInformation::stuIcons::ViewPrinter32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_VIEW_PR32X32, &hic); ASSERT(hic);
	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CustBilling32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CUSTBILLING_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::CustCharge32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CUSTCHARGE_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::PayAdjust16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_PAYADJUST_16X16, &hic); ASSERT(hic);

	return hic;
}

 begbert 04-26-2007 1-27238 : added this
HICON	CGBLSystemInformation::stuIcons::AddFrom32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_ADDFROM_32X32, &hic); ASSERT(hic);

	return hic;
}

// begbert 05-10-2007 1-27260 : added this
HICON	CGBLSystemInformation::stuIcons::SAECash16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_SAECASH16X16, &hic); ASSERT(hic);

	return hic;
}

// begbert 05-10-2007 1-27260 : added this
HICON	CGBLSystemInformation::stuIcons::BRStatementRec16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BRSTATEMENTREC16X16, &hic); ASSERT(hic);

	return hic;
}

// begbert 05-10-2007 1-27260 : added this
HICON	CGBLSystemInformation::stuIcons::BREnterBank16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BRENTERBANK16X16, &hic); ASSERT(hic);

	return hic;
}

HICON	CGBLSystemInformation::stuIcons::BRVoidBank16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BRVOIDBANK16X16, &hic); ASSERT(hic);

	return hic;
}

HICON	CGBLSystemInformation::stuIcons::ImportTrans32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_IMPORTTRANS32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Cal32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CAL_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::POMail16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_POMAIL_32X32, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Cancel16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_CANCEL_16X16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::POBack16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BACK_16X16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::FileNew16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_FILENEW_16X16, &hic); ASSERT(hic);

	return hic;
}

//---------------------------------------------------------------------------
HICON	CGBLSystemInformation::stuIcons::Forward16() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_FORWARD_16X16, &hic); ASSERT(hic);

	return hic;
}
*/

// InTimeTec: 07/08/2013 : HS : Copy Stock Item Alias
// resource icon used for duplicate aliases message
HICON	CGBLSystemInformation::stuIcons::Blank32() const
{
	static HICON	hic = NULL;
	if (!hic)
		CGBLResources::GetResourceIcon(IDI_BLANK32, &hic); ASSERT(hic);
	return hic;
}

// InTimeTec: End
//---------------------------------------------------------------------------
CMS::ISalesSystemInterfacePtr	CGBLSystemInformation::stuModuleSettings::GetSalesInterface(UINT nModuleID)
{
	CMS::ISalesSystemInterfacePtr	pIFace = NULL;
	switch (nModuleID)
	{
	case IDS_OE_MODULE:
		pIFace = &OESettings;
		break;
	case IDS_PS_MODULE:
		pIFace = &PSSettings;
		break;
	default:
		throw std::domain_error("nModuleID is not equal to IDS_OE_MODULE or IDS_PS_MODULE");
		break;
	};

	return pIFace;
}

//---------------------------------------------------------------------------
CMS::ISalesSystemInterfacePtr	CGBLSystemInformation::stuModuleSettings::GetCurrentSalesInterface(void)
{
	//PTR.08.19.2005.1-21083
	UINT uiModule = SASettings.Module;
	if (!uiModule)
	{
		CView* const pView = CGBLForm::EkGetActiveView();
		ASSERT(pView);
		uiModule = ::IsWindow(pView->GetSafeHwnd()) ? (UINT)pView->SendMessage(WM_CMSGETCURRENTMODULE) : uiModule;
	}
	CMS::ISalesSystemInterfacePtr	pIFace = GetSalesInterface(uiModule);

	return pIFace;
}

//---------------------------------------------------------------------------
CString	CGBLSystemInformation::stuServers::AppendServerInfo(const CString& szData)
{
	if (szData.GetLength() == 0)
		return szData;

	CXMLDocument docXMLData;
	docXMLData.LoadXML(szData);
	//KPM.2005.08.03 Adding in server info for every call
	CXMLElement ele = docXMLData.GetDocumentElement();
	CXMLElement eleServers = ele.AddNewChild(_T("ServersListing"), _T(""));

	this->CompanyServer.AppendServerInfo(eleServers, _T("CompanyServer"));
	this->PrimaryServer.AppendServerInfo(eleServers, _T("PrimaryServer"));
	this->ArchiveServer.AppendServerInfo(eleServers, _T("ArchiveServer"));

	return docXMLData.GetOuterXML();
}

//---------------------------------------------------------------------------
void	CGBLSystemInformation::stuServerInformation::AppendServerInfo(CXMLElement& eleServers, const CString szServer)
{
	CXMLElement eleServer = eleServers.AddNewChild(szServer, _T(""));
	eleServer.AddNewChild(_T("DBType"), this->DBType);
	eleServer.AddNewChild(_T("Server"), this->Name);
	eleServer.AddNewChild(_T("Database"), this->Database);
	eleServer.AddNewChild(_T("User"), this->User);
	eleServer.AddNewChild(_T("Password"), this->Password);

	if (this->Timeout != 0)				eleServer.AddNewChild(_T("Timeout"), this->Timeout);
	if (this->PoolTime != 0)			eleServer.AddNewChild(_T("PoolTime"), this->PoolTime);
	if (this->MinPoolSize != 0)			eleServer.AddNewChild(_T("MinPoolSize"), this->MinPoolSize);
	if (this->MaxPoolSize != 0)			eleServer.AddNewChild(_T("MaxPoolSize"), this->MaxPoolSize);
	if (!this->Application.IsEmpty())	eleServer.AddNewChild(_T("Application"), this->Application);
	if (this->Reset)					eleServer.AddNewChild(_T("Reset"));
	if (this->Enlist)					eleServer.AddNewChild(_T("Enlist"));
	if (this->Security)					eleServer.AddNewChild(_T("Security"));
	if (!this->NetworkType.IsEmpty())	eleServer.AddNewChild(_T("NetworkType"), this->NetworkType);
	if (this->PacketSize != 0)			eleServer.AddNewChild(_T("PacketSize"), this->PacketSize);
	if (this->Persist)					eleServer.AddNewChild(_T("Persist"));
	if (this->Pooling)					eleServer.AddNewChild(_T("Pooling"));
	if (!this->Workstation.IsEmpty())	eleServer.AddNewChild(_T("Workstation"), this->Workstation);
	if (this->Port != 0)				eleServer.AddNewChild(_T("Port"), this->Port);
	if (this->GetSchema)				eleServer.AddNewChild(_T("GetSchema"));
	if (this->MaintForm)				eleServer.AddNewChild(_T("MaintForm"));
	if (this->MARS)						eleServer.AddNewChild(_T("MARS"));
	if (this->Encrypt)					eleServer.AddNewChild(_T("Encrypt"));
	if (this->Async)					eleServer.AddNewChild(_T("Async"));
	if (this->UseMaster)				eleServer.AddNewChild(_T("UseMaster"));
	if (!this->MirrorServer.IsEmpty())	eleServer.AddNewChild(_T("MirrorServer"), this->MirrorServer);
	if (!this->Schema.IsEmpty())		eleServer.AddNewChild(_T("Schema"), this->Schema);
	if (!this->DataVersion.IsEmpty())	eleServer.AddNewChild(_T("DataVersion"), this->DataVersion);
}

//---------------------------------------------------------------------------
void	CGBLSystemInformation::stuServers::Parse(const CString& szData)
{
	if (szData.GetLength() == 0)
		return;

	CXMLDocument docXMLData;
	docXMLData.LoadXML(szData);
	CXMLElement ele = docXMLData.GetDocumentElement();
	if (ele.ElementExists("ServersListing"))
	{
		CXMLElement eleServers = ele.GetItem("ServersListing");
		if (eleServers.ElementExists("CompanyServer"))
		{
			CXMLElement eleServer = eleServers.GetItem("CompanyServer");
			this->CompanyServer.Parse(eleServer);
		}
		if (eleServers.ElementExists("PrimaryServer"))
		{
			CXMLElement eleServer = eleServers.GetItem("PrimaryServer");
			this->PrimaryServer.Parse(eleServer);
		}
		if (eleServers.ElementExists("ArchiveServer"))
		{
			CXMLElement eleServer = eleServers.GetItem("ArchiveServer");
			this->ArchiveServer.Parse(eleServer);
		}
	}
}

//---------------------------------------------------------------------------
void	CGBLSystemInformation::stuServerInformation::Parse(CXMLElement& eleServer)
{
	this->Initialize();
	if (eleServer.ElementExists("DBType"))			this->DBType = eleServer.GetItem("DBType").GetInnerText();
	if (eleServer.ElementExists("Server"))			this->Name = eleServer.GetItem("Server").GetInnerText();
	if (eleServer.ElementExists("Catalog"))			this->Database = eleServer.GetItem("Catalog").GetInnerText();
	if (eleServer.ElementExists("Database"))		this->Database = eleServer.GetItem("Database").GetInnerText();
	if (eleServer.ElementExists("User"))			this->User = eleServer.GetItem("User").GetInnerText();
	if (eleServer.ElementExists("Password"))		this->Password = eleServer.GetItem("Password").GetInnerText();
	if (eleServer.ElementExists("Timeout"))			this->Timeout = eleServer.GetItem("Timeout").GetInnerTextInt();
	if (eleServer.ElementExists("PoolTime"))		this->PoolTime = eleServer.GetItem("PoolTime").GetInnerTextInt();
	if (eleServer.ElementExists("MinPoolSize"))		this->MinPoolSize = eleServer.GetItem("MinPoolSize").GetInnerTextInt();
	if (eleServer.ElementExists("MaxPoolSize"))		this->MaxPoolSize = eleServer.GetItem("MaxPoolSize").GetInnerTextInt();
	if (eleServer.ElementExists("Application"))		this->Application = eleServer.GetItem("Application").GetInnerText();
	if (eleServer.ElementExists("Reset"))			this->Reset = true;
	if (eleServer.ElementExists("Enlist"))			this->Enlist = true;
	if (eleServer.ElementExists("Security"))		this->Security = true;
	if (eleServer.ElementExists("NetworkType"))		this->NetworkType = eleServer.GetItem("NetworkType").GetInnerText();
	if (eleServer.ElementExists("PacketSize"))		this->PacketSize = eleServer.GetItem("PacketSize").GetInnerTextInt();
	if (eleServer.ElementExists("Persist"))			this->Persist = true;
	if (eleServer.ElementExists("Pooling"))			this->Pooling = true;
	if (eleServer.ElementExists("Workstation"))		this->Workstation = eleServer.GetItem("Workstation").GetInnerText();
	if (eleServer.ElementExists("Port"))			this->Port = eleServer.GetItem("Port").GetInnerTextInt();
	if (eleServer.ElementExists("GetSchema"))		this->GetSchema = true;
	if (eleServer.ElementExists("MaintForm"))		this->MaintForm = true;
	if (eleServer.ElementExists("MARS"))			this->MARS = true;
	if (eleServer.ElementExists("Encrypt"))			this->Encrypt = true;
	if (eleServer.ElementExists("Async"))			this->Async = true;
	if (eleServer.ElementExists("UseMaster"))		this->UseMaster = true;
	if (eleServer.ElementExists("MirrorServer"))	this->MirrorServer = eleServer.GetItem("MirrorServer").GetInnerText();
	if (eleServer.ElementExists("Schema"))			this->Schema = eleServer.GetItem("Schema").GetInnerText();
	if (eleServer.ElementExists("DataVersion"))		this->DataVersion = eleServer.GetItem("DataVersion").GetInnerText();
}

//---------------------------------------------------------------------------
void	CGBLSystemInformation::stuServers::Initialize()
{
	this->CompanyServer.Initialize();
	this->PrimaryServer.Initialize();
	this->ArchiveServer.Initialize();
}

//---------------------------------------------------------------------------
void	CGBLSystemInformation::stuServerInformation::Initialize()
{
	this->Name.Empty();
	this->Database.Empty();
	this->User.Empty();
	this->Password.Empty();
	this->Timeout = 0;
	this->DBType = _T("SQLServer");
	this->PoolTime = 0;
	this->MinPoolSize = 0;
	this->MaxPoolSize = 0;
	this->Application.Empty();
	this->Reset = false;
	this->Enlist = false;
	this->Security = false;
	this->NetworkType.Empty();
	this->PacketSize = 0;
	this->Persist = false;
	this->Pooling = false;
	this->Workstation.Empty();
	this->Port = 0;
	this->GetSchema = false;
	this->MaintForm = false;
	this->MARS = false;
	this->Encrypt = false;
	this->Async = false;
	this->UseMaster = false;
	this->ReadOnly = false;
	this->MirrorServer.Empty();
	this->Schema.Empty();
	this->DataVersion.Empty();
}

//---------------------------------------------------------------------------
CString CGBLSystemInformation::GetAuthorizerDLLSettingsXML(void) const
{
	CGBLSystemInformation* const self = const_cast<CGBLSystemInformation*>(this);

	if (self->ApplicationSettings.IsAR())
		return ModuleSettings.ARSettings.GetAuthorizerDLLSettingsXML();
	else if (self->ModuleSettings.GetCurrentSalesInterface())
		return self->ModuleSettings.GetCurrentSalesInterface()->GetAuthorizerDLLSettingsXML();

	return _T("");
}

//---------------------------------------------------------------------------
void CGBLSystemInformation::SetAuthorizerDLLSettingsXML(const CString& szAuthorizerSettings)
{
	if (ApplicationSettings.IsAR())
		ModuleSettings.ARSettings.SetAuthorizerDLLSettingsXML(szAuthorizerSettings);
	else if (ModuleSettings.GetCurrentSalesInterface())
		ModuleSettings.GetCurrentSalesInterface()->SetAuthorizerDLLSettingsXML(szAuthorizerSettings);
}

//---------------------------------------------------------------------------
CString CGBLSystemInformation::GetAuthorizerDLLFilepath(void) const
{
	CString		szFilepath(_T(""));
	CXMLParams	clsXMLReturn(true);
	CXMLParams	clsXMLTable;
	CXMLParams	clsXMLCards;
	CXMLParams	clsXMLCard;

	const CString szAuthFile(GetAuthorizerDLLFilename());
	const CString szAuthXML(GetAuthorizerDLLSettingsXML());

	if (szAuthFile.IsEmpty() || szAuthXML.IsEmpty()) goto XIT;

	clsXMLReturn.SetXML(szAuthXML);

	if (!clsXMLReturn.ParamExists(szAuthFile)) goto XIT;
	clsXMLTable = clsXMLReturn.GetXMLParams(szAuthFile);

	if (!clsXMLTable.ParamExists(CMSStrings::Authorization::Cards)) goto XIT;
	clsXMLCards = clsXMLTable.GetXMLParams(CMSStrings::Authorization::Cards);

	if (!clsXMLCards.ParamExists(CMSStrings::Authorization::Card)) goto XIT;
	clsXMLCard = clsXMLCards.GetXMLParams(CMSStrings::Authorization::Card);

	szFilepath = clsXMLCard.GetParamStr(CMSStrings::Authorization::Filepath, false);

XIT:
	clsXMLCard.Dispose();
	clsXMLCards.Dispose();
	clsXMLTable.Dispose();
	clsXMLReturn.Dispose();

	return szFilepath;
}

//---------------------------------------------------------------------------
CString CGBLSystemInformation::GetAuthorizerDLLFilename(void) const
{
	CGBLSystemInformation* const self = const_cast<CGBLSystemInformation*>(this);

	if (self->ApplicationSettings.IsAR())
		return self->ModuleSettings.ARSettings.GetAuthorizerDLLFilename();
	else if (self->ModuleSettings.GetCurrentSalesInterface())
		return self->ModuleSettings.GetCurrentSalesInterface()->GetAuthorizerDLLFilename();

	return (_T(""));
}

//---------------------------------------------------------------------------
void CGBLSystemInformation::SetAuthorizerDLLFilename(const CString& szFilename)
{
	if (ApplicationSettings.IsAR())
		ModuleSettings.ARSettings.SetAuthorizerDLLFilename(szFilename);
	else if (ModuleSettings.GetCurrentSalesInterface())
		ModuleSettings.GetCurrentSalesInterface()->SetAuthorizerDLLFilename(szFilename);
}

//---------------------------------------------------------------------------
CMS::CCAuthorizationType CGBLSystemInformation::GetCCAuthLevel(void) const
{
	CMS::CCAuthorizationType eLevel = CMS::AUTH_TYPE_INACTIVE;
	CXMLParams		clsXMLReturn(true);
	CXMLParams		clsXMLTable;
	CXMLParams		clsXMLCards;
	CXMLParams		clsXMLCard;

	const CString	szAuthFile(GetAuthorizerDLLFilename());
	const CString	szAuthXML(GetAuthorizerDLLSettingsXML());

	if (szAuthFile.IsEmpty() || szAuthXML.IsEmpty()) goto XIT;
	clsXMLReturn.SetXML(szAuthXML);

	if (!clsXMLReturn.ParamExists(szAuthFile)) goto XIT;
	clsXMLTable = clsXMLReturn.GetXMLParams(szAuthFile);

	if (!clsXMLTable.ParamExists(CMSStrings::Authorization::Cards)) goto XIT;
	clsXMLCards = clsXMLTable.GetXMLParams(CMSStrings::Authorization::Cards);

	if (!clsXMLCards.ParamExists(CMSStrings::Authorization::Card)) goto XIT;
	clsXMLCard = clsXMLCards.GetXMLParams(CMSStrings::Authorization::Card);

	eLevel = (CMS::CCAuthorizationType)clsXMLCard.GetParamInt(CMSStrings::Authorization::CCAuthLevel, false);

XIT:
	clsXMLCard.Dispose();
	clsXMLCards.Dispose();
	clsXMLTable.Dispose();
	clsXMLReturn.Dispose();

	return eLevel;
}

//---------------------------------------------------------------------------
CMS::CheckAuthorizationType CGBLSystemInformation::GetCheckAuthLevel(void) const
{
	CMS::CheckAuthorizationType eLevel = CMS::AUTH_TYPE_INACTIVE;
	CXMLParams		clsXMLReturn(true);
	CXMLParams		clsXMLTable;
	CXMLParams		clsXMLCards;
	CXMLParams		clsXMLCard;

	const CString	szAuthFile(GetAuthorizerDLLFilename());
	const CString	szAuthXML(GetAuthorizerDLLSettingsXML());

	if (szAuthFile.IsEmpty() || szAuthXML.IsEmpty()) goto XIT;

	clsXMLReturn.SetXML(szAuthXML);

	if (!clsXMLReturn.ParamExists(szAuthFile)) goto XIT;
	clsXMLTable = clsXMLReturn.GetXMLParams(szAuthFile);

	if (!clsXMLTable.ParamExists(CMSStrings::Authorization::Cards)) goto XIT;
	clsXMLCards = clsXMLTable.GetXMLParams(CMSStrings::Authorization::Cards);

	if (!clsXMLCards.ParamExists(CMSStrings::Authorization::Card)) goto XIT;
	clsXMLCard = clsXMLCards.GetXMLParams(CMSStrings::Authorization::Card);

	eLevel = (CMS::CheckAuthorizationType)clsXMLCard.GetParamInt(CMSStrings::Authorization::CheckAuthLevel, false);

XIT:
	clsXMLCard.Dispose();
	clsXMLCards.Dispose();
	clsXMLTable.Dispose();
	clsXMLReturn.Dispose();

	return eLevel;
}

//---------------------------------------------------------------------------
bool CGBLSystemInformation::GetUseAddressVerification(void) const
{
	bool			bolAllowAddressVerify = false;
	CXMLParams		clsXMLReturn(true);
	CXMLParams		clsXMLTable;
	CXMLParams		clsXMLCards;
	CXMLParams		clsXMLCard;

	const CString	szAuthFile(GetAuthorizerDLLFilename());
	const CString	szAuthXML(GetAuthorizerDLLSettingsXML());

	if (szAuthFile.IsEmpty() || szAuthXML.IsEmpty()) goto XIT;

	clsXMLReturn.SetXML(szAuthXML);

	if (!clsXMLReturn.ParamExists(szAuthFile)) goto XIT;
	clsXMLTable = clsXMLReturn.GetXMLParams(szAuthFile);

	if (!clsXMLTable.ParamExists(CMSStrings::Authorization::Cards)) goto XIT;
	clsXMLCards = clsXMLTable.GetXMLParams(CMSStrings::Authorization::Cards);

	if (!clsXMLCards.ParamExists(CMSStrings::Authorization::Card)) goto XIT;
	clsXMLCard = clsXMLCards.GetXMLParams(CMSStrings::Authorization::Card);

	bolAllowAddressVerify = clsXMLCard.GetParamBool(CMSStrings::Authorization::AddressVerification, false);

XIT:
	clsXMLCard.Dispose();
	clsXMLCards.Dispose();
	clsXMLTable.Dispose();
	clsXMLReturn.Dispose();

	return bolAllowAddressVerify;
}

//---------------------------------------------------------------------------
bool CGBLSystemInformation::GetAllowBlankAddress(void) const
{
	bool			bolAllowBlankAddress = false;
	CXMLParams		clsXMLReturn(true);
	CXMLParams		clsXMLTable;
	CXMLParams		clsXMLCards;
	CXMLParams		clsXMLCard;

	const CString	szAuthFile(GetAuthorizerDLLFilename());
	const CString	szAuthXML(GetAuthorizerDLLSettingsXML());

	if (szAuthFile.IsEmpty() || szAuthXML.IsEmpty()) goto XIT;

	clsXMLReturn.SetXML(szAuthXML);

	if (!clsXMLReturn.ParamExists(szAuthFile)) goto XIT;
	clsXMLTable = clsXMLReturn.GetXMLParams(szAuthFile);

	if (!clsXMLTable.ParamExists(CMSStrings::Authorization::Cards)) goto XIT;
	clsXMLCards = clsXMLTable.GetXMLParams(CMSStrings::Authorization::Cards);

	if (!clsXMLCards.ParamExists(CMSStrings::Authorization::Card)) goto XIT;
	clsXMLCard = clsXMLCards.GetXMLParams(CMSStrings::Authorization::Card);

	bolAllowBlankAddress = clsXMLCard.GetParamBool(CMSStrings::Authorization::AllowBlankAddress, false);

XIT:
	clsXMLCard.Dispose();
	clsXMLCards.Dispose();
	clsXMLTable.Dispose();
	clsXMLReturn.Dispose();

	return bolAllowBlankAddress;
}

//---------------------------------------------------------------------------
bool CGBLSystemInformation::GetAutoFillCreditCardInfo(void) const
{
	CGBLSystemInformation* const self = const_cast<CGBLSystemInformation*>(this);
	bool		bolStatus = false;

	if (self->ApplicationSettings.IsAR())
		bolStatus = self->ModuleSettings.ARSettings.GetAutoFillCreditCardInfo();
	else if (self->ModuleSettings.GetCurrentSalesInterface())
		bolStatus = self->ModuleSettings.GetCurrentSalesInterface()->GetAutoFillCreditCardInfo();

	return bolStatus;
}

//---------------------------------------------------------------------------
void CGBLSystemInformation::SetAutoFillCreditCardInfo(bool bolAutofill)
{
	if (ApplicationSettings.IsAR())
		ModuleSettings.ARSettings.SetAutoFillCreditCardInfo(bolAutofill);
}

//---------------------------------------------------------------------------
CString CGBLSystemInformation::GetPaymentProcessor(void) const
{
	CXMLParams		clsXMLReturn(true);
	CXMLParams		clsXMLTable;
	CXMLParams		clsXMLCards;
	CXMLParams		clsXMLCard;

	CString			szProcessor(_T(""));
	const CString	szAuthFile(GetAuthorizerDLLFilename());
	const CString	szAuthXML(GetAuthorizerDLLSettingsXML());

	if (szAuthFile.IsEmpty() || szAuthXML.IsEmpty()) goto XIT;

	clsXMLReturn.SetXML(szAuthXML);

	if (!clsXMLReturn.ParamExists(szAuthFile)) goto XIT;
	clsXMLTable = clsXMLReturn.GetXMLParams(szAuthFile);

	if (!clsXMLTable.ParamExists(CMSStrings::Authorization::Cards)) goto XIT;
	clsXMLCards = clsXMLTable.GetXMLParams(CMSStrings::Authorization::Cards);

	if (!clsXMLCards.ParamExists(CMSStrings::Authorization::Card)) goto XIT;
	clsXMLCard = clsXMLCards.GetXMLParams(CMSStrings::Authorization::Card);

	szProcessor = clsXMLCard.GetParamStr(CMSStrings::Authorization::Processor, false);

XIT:
	clsXMLCard.Dispose();
	clsXMLCards.Dispose();
	clsXMLTable.Dispose();
	clsXMLReturn.Dispose();

	return szProcessor;
}

//---------------------------------------------------------------------------
int CGBLSystemInformation::GetVerificationSoftwareIDX(void) const
{
	CGBLSystemInformation* const self = const_cast<CGBLSystemInformation*>(this);

	if (self->ApplicationSettings.IsAR())
		return self->ModuleSettings.ARSettings.GetVerificationSoftwareIDX();
	else if (self->ModuleSettings.GetCurrentSalesInterface())
		return self->ModuleSettings.GetCurrentSalesInterface()->GetVerificationSoftware();

	return -1;
}

//---------------------------------------------------------------------------
void CGBLSystemInformation::SetVerificationSoftwareIDX(int nCCVerificationSoftware)
{
	if (ApplicationSettings.IsAR())
		ModuleSettings.ARSettings.SetVerificationSoftwareIDX(nCCVerificationSoftware);
	else if (ModuleSettings.GetCurrentSalesInterface())
		ModuleSettings.GetCurrentSalesInterface()->SetVerificationSoftware(nCCVerificationSoftware);
}

//---------------------------------------------------------------------------
bool CGBLSystemInformation::GetUsingTPISmartPayments(void) const
{
	CGBLSystemInformation* const self = const_cast<CGBLSystemInformation*>(this);

	if (self->ApplicationSettings.IsAR())
		return self->ModuleSettings.ARSettings.GetUsingTPISmartPayments();
	else if (self->ModuleSettings.GetCurrentSalesInterface())
		return self->ModuleSettings.GetCurrentSalesInterface()->GetUsingTPISmartPayments();

	return false;
}

//---------------------------------------------------------------------------
void CGBLSystemInformation::SetUsingTPISmartPayments(bool bolUsing)
{
	if (ApplicationSettings.IsAR())
		ModuleSettings.ARSettings.SetUsingTPISmartPayments(bolUsing);
	else if (ModuleSettings.GetCurrentSalesInterface())
		ModuleSettings.GetCurrentSalesInterface()->SetUsingTPISmartPayments(bolUsing);
}

//---------------------------------------------------------------------------
bool CGBLSystemInformation::GetRunningInVirtualUI(void) const
{
	return vui == nullptr ? false : vui->Active();
}

//---------------------------------------------------------------------------
VirtualUI* CGBLSystemInformation::GetVirtualUI(void)
{
	if (vui == nullptr)
		vui = new VirtualUI();
	return vui;
}

//---------------------------------------------------------------------------
void CGBLSystemInformation::DownloadFile(std::wstring LocalFilename)
{
	if (vui != nullptr)
		VUI->DownloadFile(LocalFilename);
}

//---------------------------------------------------------------------------
void CGBLSystemInformation::DownloadFile(std::wstring LocalFilename, std::wstring RemoteFilename)
{
	if (vui != nullptr)
		VUI->DownloadFile(LocalFilename, RemoteFilename);
}

//---------------------------------------------------------------------------
void CGBLSystemInformation::DownloadFile(std::wstring LocalFilename, std::wstring RemoteFilename, std::wstring MimeType)
{
	if (vui != nullptr)
		VUI->DownloadFile(LocalFilename, RemoteFilename, MimeType);
}

//---------------------------------------------------------------------------
void CGBLSystemInformation::PreviewPdf(std::wstring FileName)
{
	if (vui != nullptr)
		VUI->PreviewPdf(FileName);
}
