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

 //Modification History:
 //Modified By: PGP(08/13/2003)
 //Changed StartupBeforeMainFrame to alter Splash screen behavior.
 //Splash screen is created as a local window object and DoEvents used to pump the events
 //Removed code initiating Splashthread.
 //Moved Splash init, update and destroy to CCMSWinApp so that the splash screen
 //can be controlled using the app - as part of fixing UT-2837.

#include "stdafx.h"

#include "..\GBLForms\MenuItemInfoList.h"
#include "..\GBLForms\GBLStartup.h"
#include "..\GBLForms\TipOfTheDay.h"
#include "..\GBLForms\CMSHelpFilesMapLoader.h"
#include "..\GBLForms\SplashThread.h"
#include "..\GBLForms\Splash.h"

#include "..\cmsdll\GBLCommandLineInfo.h"
#include "..\cmsdll\SetGlobalscmsdll.h"				//CMSDLL Global setting routine
#include "..\cmsdll\gblserver.h"
#include "..\cmsdll\gblimage.h"
#include "..\cmsdll\gblnetdll.h"
#include "..\Common\LangIDs.h"
#include "..\cmsdll\GBLApplication.h"
#include "..\GBLForms\CMSMessageBox.h"
#include "..\CMSDotNet\DenaliOfficeView.h"
#include "..\cmsdll\CMSException.h"
#include "..\CMSDotNet\ProductAuditAPI.h"

CGBLStartup::CGBLStartup(UINT uinModule, CGBLSystemInformation* pSystemInformation, CCMSWinApp* pTheApp)
	: m_pTheApp(pTheApp)
	, m_pMainFrame(NULL)
	, m_pCMSStatusBar(NULL)
	, si(pSystemInformation)
	, m_bShowSplashWindow(true)
{
	CMS_VALID_POINTER(pSystemInformation, CGBLSystemInformation);
	CMS_VALID_POINTER(pTheApp, CCMSWinApp);

	si->ApplicationSettings.SetModule(uinModule);
	si->pTheApp = m_pTheApp;
	si->pCMSApp = pTheApp;
}

CGBLStartup::~CGBLStartup(void)
{}

bool CGBLStartup::SetFrameWnd(ICMSMainFrameWnd* pFrameWnd)
{
	CMS_VALID_POINTER(pFrameWnd, ICMSMainFrameWnd);
	m_pMainFrame = pFrameWnd;
	si->IApplicationMainFrame = m_pMainFrame;
	return true;
}

bool CGBLStartup::ValidateDates()
{
	// 1-35852 BK 5/11/11 this is very, very old code (hard-coded error message) and calendar is misspelled.
	// Removed this code to further evaluate if a person should be locked out of a module if their date
	// is outside the fiscal calendar. Lori found this by having the COMPUTER date outside the fiscal calendar then opening a
	// module other than controller.
/*
	if (si->ApplicationSettings.SystemDate < si->FiscalCalendar.StartDate ||
		si->ApplicationSettings.SystemDate > si->FiscalCalendar.EndDate)
	{
		CGBLForm::CMSMessageBox(si->ApplicationMainFrame, 0, 0, MB_OK | MB_ICONERROR, _T("System Date Must Be Within Fiscal Calender."), _T("Module Load") ) ;
		return false ;
	}
	else
*/
	return true;
}

bool CGBLStartup::StartupBeforeMainFrame()
{
	/*
	******************* WARNING! WARNING! WARNING! ******************************

	This is the startup process for all applications.  It is VITAL that this not be
	modified without express authorization from a Senior Programmer or the Director
	of Product Development.  Errors here can cause the application to suddenly not work.

	Failure in any of the below routines will cause the application to exit.

	The following routines are run at startup in the following order:
	1.  Show the splash screen.
	2.  CGBLSystemInformation load.
	3.  CheckCOMInterfaces - This is to ensure we can successfully connect to NETDLL and SXMLPARS.
	4.  Registry.SetupAllValues - This is to get all the machine information and other
		info from the registry.  This has to be after NETDLL since we use NETDLL to
		process registry information.
	5.  GetVersionInfo - This is to get all the version information.
	6.  Send the application startup event to the event log.
	7.  CheckClientServerVersion - This is to ensure the version number is the same
		between the client and server.
	8.  Login to the Primary Database.
	9.  Check the Primary Database Version.
	10. Login to the License Server.

	//12/16/2008 CJB - 1-31566  Per Training & Design, no longer give an error message for screen resolution.
	//We have a design minimum, but will not disallow users to run it at a lower resolution.
	11. Ensure screen size meets CMS minimum standards.

	******************* WARNING! WARNING! WARNING! ******************************
	*/

	////////////////////////////////////////////////////////////////////////////
	// START OF CMS SPECIFIC PREFIX
	////////////////////////////////////////////////////////////////////////////

	AfxInitRichEdit2();

	// Give the CMSDLL information about its globals
	CSetGlobalsCMSDll clsSetGlobalsCMSDll(si);

	if (!si->ParseCommandLineParameters()) // Read the command line parameters before we do anything else
		return false;

	si->CheckForCommandLineOverrides();	//PTR.07.29.2005

	//1. Show the splash screen.
	//RCG - 05/27/2011 – 9000006 – Automated Posting
	// If this is CT and we have a -qt command line option then don't show the splash screen
	if (m_bShowSplashWindow && si->ApplicationSettings.IsCT())
		m_bShowSplashWindow = !si->CommandLineSettings.CT_Quiet;
	else if (m_bShowSplashWindow)	// If this is not CT and we have a -pt command line option then don't show the splash screen
		m_bShowSplashWindow = !si->CommandLineSettings.Automated_Posting;

	if (m_bShowSplashWindow)
		m_pTheApp->ShowSplashWnd();

	if (!UpdateSplashScreen(IDS_READING_ENVIRONMENT))
		return HandleStartupFailure();

	//2. CGBLSystemInformation load.
	//Now do the process(es)  This needs to be before the CheckCOMInterfaces as it resets variables sent in CheckCOMInterfaces.
	if (!si->LoadSystemGlobals())
		return HandleStartupFailure();

	//3. CheckCOMInterfaces - This is to ensure we can successfully connect to NETDLL and SXMLPARS.
	// KPM, 9/25/2003, Check the interfaces first, no need to show anything if
	// these are broken
	if (!CheckCOMInterfaces())
	{
		m_pTheApp->HideSplashWnd();

#ifdef _DEBUG
		// This code is to try to let the threads finish on their own before exiting.
		// This is only a problem in Debug as you get an assertion error if things don't
		// close right.
		COleMessageFilter* pFilter = AfxOleGetMessageFilter();
		int nCount = 0;

		//PTR.07.26.2005 - Reducing max count from 600 to 100 - which reduces count time
		//from 60 seconds to 10 seconds.
		while (pFilter->m_dwRef > 1 && nCount < 100)
		{
			nCount++;
			Sleep(100);
		}

		pFilter->m_dwRef = 0;
#endif

		return HandleStartupFailure();
	}

	//KPM.2005.08.10 Load the help file info
	si->SetupHelpSystem();
	CCMSHelpFilesMapLoader::LoadHelpFilesMap(si->ApplicationSettings.ModuleID);

	//4.  Registry.SetupAllValues - This is to get all the machine information and other
	//    info from the registry.  This has to be after NETDLL since we use NETDLL to
	//    process registry information.
	if (!UpdateSplashScreen(IDS_READING_ENVIRONMENT))
		return HandleStartupFailure();

	// MG 4/08 Opening Denali Modules Routine spec
	si->RegistryEditor.SetupAllValues();

	// 1-31094 BK 10/23/08 added this line because the line above reads from register primary info from registry after global variables were set from commandline
	CMS::GetSI()->CheckForCommandLineOverrides();

	//5.  GetVersionInfo - This is to get all the version information.
	if (!UpdateSplashScreen(IDS_LOGIN_GETTINGVERSIONINFO))
		return HandleStartupFailure();

	if (!CGBLApplication::GetAllVersionData())
		return HandleStartupFailure();

	//6. Send the application startup event to the event log.
	CGBLDebug::MemoryCheckpoint(_T("Send Application Startup Event"));

	CString szCommandLine = GetCommandLine();
	HidePassword(szCommandLine, _T("cp"));	//Hide the user password.
	HidePassword(szCommandLine, _T("xp"));	//Hide the primary database password.
	CXMLDocument docXMLData(CMSStrings::XMLTags::Root);
	CXMLElement eleRoot = docXMLData.GetDocumentElement();
	eleRoot.AddNewChild(_T("COMMAND_LINE"), szCommandLine);
	eleRoot.AddNewChild(_T("CLIENT_VERSION"), si->ApplicationSettings.ClientBuildNumber);
	eleRoot.AddNewChild(_T("SERVER_VERSION"), si->ApplicationSettings.ServerBuildNumber);
	CString szLogMsg(_T(""));
	szLogMsg.Format(RESSTRING(IDS_LOGMSG_APPSTARTUP), AfxGetAppName());
	CGBLEventLogOptions clsOptions;
	clsOptions.m_dwEventFlags = EVLOG_XMLCOMMAND;
	clsOptions.m_eCategory = EventLogCategory_Application;
	CGBLEventLog::WriteLogXML(EventLog_Startup, szLogMsg, &clsOptions, &docXMLData);

	//7.  CheckClientServerVersion - This is to ensure the version number is the same
	//    between the client and server.  If this fails, then the application exits.
	if (!si->ApplicationSettings.DeveloperMode)	//PTR.08.01.2005 - We'll let developers have mismatches.
		if (!CGBLApplication::DoServerAndClientVersionsMatch())
			return HandleStartupFailure();

	//8.  Login to the Primary Database.
	if (!UpdateSplashScreen(IDS_LOGIN_CONNECTINGTOPRIMARYDB))
		return HandleStartupFailure();

	if (!LoginPrimaryDB())
		return HandleStartupFailure();

	//9.  Check the Primary Database Version.
	if (!si->ApplicationSettings.DeveloperMode)	//PTR.08.01.2005 - We'll let developers have mismatches.
	{
		switch (CheckPrimaryDBVersion())
		{
		case GBLStart_Fail:
			return HandleStartupFailure();
			break;
		case GBLStart_ShutDown:
			return false;
			break;
		}
	}

	// begbert 03-07-2012 1-36721 : Test security and close if failed.
	{
		bool bFailedSecurity = false;

		HMODULE hDotNetDLL = AfxLoadLibrary(_T("CMSDotNet.dll"));
		CString szUIVersion;
		if (hDotNetDLL != NULL) // LoadLibrary succeeded
		{
			try
			{
				GETDLLFUNC(DUI_TestConnectionRights, hDotNetDLL);
				if (DUI_TestConnectionRights != NULL)
					DUI_TestConnectionRights(bFailedSecurity);
				AfxFreeLibrary(hDotNetDLL);
			}
			catch (...)
			{
			}
		}

		if (bFailedSecurity)
		{
			if (si->CancelApplicationLoad) return false;
			CGBLForm::ShowCriticalMsg(IDS_INSUFFICIENT_USER_RIGHTS_MSG, IDS_INSUFFICIENT_USER_RIGHTS);
			return false;
		}
	}

	// Made it so this is not run for SQL-2-SQL.
	if (si->ApplicationSettings.StartupModule != CAppSettings::SQLTOSQL)
	{
		// 1-35164 BK 11/22/10 get data version from UI database.  blank result if db does not exist, info table does not exist, data in table is blank or does not exist.
		HMODULE hDotNetDLL = AfxLoadLibrary(_T("CMSDotNet.dll"));
		CString szUIVersion;
		if (hDotNetDLL != NULL) // LoadLibrary succeeded
		{
			try
			{
				GETDLLFUNC(DUI_GetVersion, hDotNetDLL);
				if (DUI_GetVersion != NULL)
					szUIVersion = DUI_GetVersion();
				AfxFreeLibrary(hDotNetDLL);
			}
			catch (...)
			{
			}
		}

		// 1-35164 BK 11/22/10 if CMSDenaliUI database doesn't exist call CMSUIDatabase.exe to create it
	// LKK 2/8/11 do not run in developer mode. do not run if called from Install Primary Db
		if (!CMS::GetSI()->ApplicationSettings.DeveloperMode && si->ApplicationSettings.StartupModule != CAppSettings::INSTALLPRIMARYDB)
		{
			//RCG - 03/14/2019 - PBI 3586 - Changed comparison of version numbers from string compare
			auto uiDataVersion = szUIVersion;
			auto uiDataVersionFromConstants = si->ApplicationSettings.UIDataVersion;
			// compare versions
			auto uiVersion = CGBLForm::CompareVersions(uiDataVersion, uiDataVersionFromConstants);

			switch (uiVersion)
			{
			case enuVersionTime::SAME:
				break;
			case enuVersionTime::NEWER:
			case enuVersionTime::OLDER:
				//RCG - 03/14/2019 - Bug 49320 - Run Update Navigation Database when Data Version in CMSDenaliUI database is newer
				if (!CGBLForm::LaunchExecutable(_T("CMSUIDatabase.exe"), true, _T("-si")))
				{
					//error message if file not found //LKK 12/20/10 changed to use string resources
					CGBLForm::CMSMessageBox(si->ApplicationMainFrame, IDS_ERR_UPDATE_UIDB, IDS_ERR_UIDB_UTILITY_NOT_FOUND, MB_OK | MB_ICONEXCLAMATION);
				}
				break;
			default:
				throw new CCMSException(_T("Value of enum enuVersionTime is not supported"));
			}
		}
	}

	// begbert 01-29-2009 1-34136 : Moved this prior to logging into the license server
	//   so that the sales entry module parameter will have been read.
	CCommandLineInfo cmdInfo;
	m_pTheApp->ParseCommandLine(cmdInfo);

	bool ShowMessage = !(CMS::GetSI()->CommandLineSettings.CT_Quiet || CMS::GetSI()->CommandLineSettings.Automated_Posting);

	//10. Login to the License Server.
	if (!UpdateSplashScreen(IDS_LOGIN_CONNECTINGTOLICSERVER))
		return HandleStartupFailure();

	// begbert 01-29-2009 1-34136 : Is were on Sales Entry, temporary set the module ID to the
	//   specific module mode we're in (OE/POS) so that the license server will screen by it.
	UINT uinModule = si->ApplicationSettings.ModuleID;
	if (si->ApplicationSettings.IsSA() &&
		si->ModuleSettings.SASettings.Module != 0)
	{
		if (si->ModuleSettings.SASettings.Module == IDS_OE_MODULE)
			si->ApplicationSettings.ModuleID = IDS_OE_MODULE;
		else if (si->ModuleSettings.SASettings.Module == IDS_PS_MODULE)
			si->ApplicationSettings.ModuleID = IDS_PS_MODULE;
	}

	if (!GetTenantID(ShowMessage))
	{
		m_pTheApp->HideSplashWnd();
#ifdef _DEBUG
		// This code is to try to let the threads finish on their own before exiting.
		// This is only a problem in Debug as you get an assertion error if things don't
		// close right.
		COleMessageFilter* pFilter = AfxOleGetMessageFilter();
		int nCount = 0;
		//PTR.07.26.2005 - Reducing max count from 600 to 100 - which reduces count time
		//from 60 seconds to 10 seconds.
		while (pFilter->m_dwRef > 1 && nCount < 100)
		{
			nCount++;
			Sleep(100);
		}
		pFilter->m_dwRef = 0;
#endif
		return HandleStartupFailure();
	}

	if (!LoginLicenseServer(ShowMessage))
	{
		m_pTheApp->HideSplashWnd();
#ifdef _DEBUG
		// This code is to try to let the threads finish on their own before exiting.
		// This is only a problem in Debug as you get an assertion error if things don't
		// close right.
		COleMessageFilter* pFilter = AfxOleGetMessageFilter();
		int nCount = 0;
		//PTR.07.26.2005 - Reducing max count from 600 to 100 - which reduces count time
		//from 60 seconds to 10 seconds.
		while (pFilter->m_dwRef > 1 && nCount < 100)
		{
			nCount++;
			Sleep(100);
		}
		pFilter->m_dwRef = 0;
#endif
		return HandleStartupFailure();
	}

	// begbert 01-29-2009 1-34136 : Restore the Module ID.
	si->ApplicationSettings.ModuleID = uinModule;

	//11. Ensure screen size meets CMS minimum standards.
	if (!UpdateSplashScreen(IDS_LOGIN_STARTINGAPPLICATION))
		return HandleStartupFailure();

	//12/16/2008 CJB - 1-31566  Per Training & Design, no longer give an error message for screen resolution.
	//We have a design minimum, but will not disallow users to run it at a lower resolution.
	//// Check for screen size
	//// handle the environment setting
	//int cx = GetSystemMetrics(SM_CXSCREEN) ;		// width
	//int cy = GetSystemMetrics(SM_CYSCREEN) ;		// height
	//if((cx < 1024) || (cy < 768))
	//{
	//	CString sMsg ;
	//	sMsg.Format(RESSTRING(IDS_RES_ERROR), cx, cy) ;
	//	CGBLForm::CMSMessageBox(NULL, 0, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONERROR,	sMsg) ;
	//	return HandleStartupFailure() ;
	//}

	//////////////////////////////////////////////////////////////////////////////////
	// END OF CMS SPECIFIC PREFIX
	//////////////////////////////////////////////////////////////////////////////////
	// Initialize OLE libraries, moved up AfxOleInit as part of fixing stack corruption.
	//if (!AfxOleInit())
	//	return FALSE ;

	AfxEnableControlContainer();

	//KPM, 4/26/2004, Get the server name for IN
	if (si->ApplicationSettings.ModuleID == IDS_IN_MODULE)
		si->Servers.ServerComponentsMachineName = CGBLServer::GetServerMachineName();

	return true;
}

//KPM, 9/25/2003, Moving all interface checking to one spot to be
//done right after the splash screen shows
bool CGBLStartup::CheckCOMInterfaces()
{
	HRESULT ret;

	//KPM.2005.05.03 Fix given by Microsoft to hide the server busy dialog
	//Init COM library
	AfxOleInit();

	//Disable the server not responding.
	AfxOleGetMessageFilter()->EnableNotRespondingDialog(FALSE);

	//Disable the server busy
	AfxOleGetMessageFilter()->EnableBusyDialog(FALSE);

	//Loading NETDLL
	if (!UpdateSplashScreen(IDS_LOADING_CLIENT))
		return false;

	if (CGBLNetDll::InitializeLibrary())
		si->ApplicationSettings.NETDLLInitialized = true;	//PTR.07.26.2005
	else
	{
		//TODO MG 4/08 Opening Denali Modules Routine spec:
		CGBLForm::ShowCriticalMsg(IDS_NETDLL_ERROR, IDS_NETDLL_ERROR_CAP);
		return false;
	}

	// MG 4/08 Opening Denali Modules Routine spec
	if (si->ApplicationSettings.NETDLLInitialized)
		CGBLNetDll::RegisterHelpUser();

	return true;
}

bool CGBLStartup::LoginPrimaryDB()
{
	// If the primary server info is stored in the local machine hive then the user can't change it
	//RCG - 05/27/2011 – 9000006 – Automated Posting
	//Attempt background server login first
	if (!CGBLFormWrap::TryLoginServer())
	{
		if (!(si->CommandLineSettings.CT_Quiet ||
			  si->CommandLineSettings.Automated_Posting))
			CGBLForm::CMSMessageBox(
				si->ApplicationMainFrame,
				IDS_ERR_SERVERLOGIN2,
				IDS_SEVER_LOGIN_ERROR,
				MB_OK | MB_ICONSTOP);

		return false;
	}

	return true;
}

bool CGBLStartup::GetTenantID(bool showMessage)
{
	if (!CGBLForm::IsLicenseServerRequiredForModule(true))
		return true;
	CGBLNetDll::TenantReturn tr = CGBLNetDll::GetTenantID((LPCTSTR)si->ApplicationSettings.UserName, false);
	si->ApplicationSettings.UseTenants = tr.UseTenants;
	if (si->ApplicationSettings.UseTenants)
	{
		si->ApplicationSettings.TenantID = tr.TenantID;
		if (si->ApplicationSettings.TenantID == _T("No License Service"))
		{
			if (CGBLApplication::GetPrimaryDBInfo())
			{
				if (TryStartLicenseService(showMessage))
				{
					CGBLNetDll::TenantReturn tr = CGBLNetDll::GetTenantID((LPCTSTR)si->ApplicationSettings.UserName);
					si->ApplicationSettings.UseTenants = tr.UseTenants;
					if (si->ApplicationSettings.UseTenants)
						si->ApplicationSettings.TenantID = tr.TenantID;
					else
						si->ApplicationSettings.TenantID = _T("NORMAL DENALI");
					if (si->ApplicationSettings.TenantID == EMPTY_STRING)
					{
						CGBLNetDll::ShowNotTenantMemberMessage(si->ApplicationSettings.UserName, showMessage);
						return false;
					}
					else
						return true;
				}
				else
					return false;
			}
			else
			{
				// if CMSLicenseService.exe is not in working directory show message and exit
				if (!CGBLForm::DoesFileExistInCMSPath(_T("CMSLicenseService.exe")))
					return CGBLNetDll::ShowServerCannotStartMessage(showMessage);

				// if user isn't allowed to write to registry show message and exit
				if (!CMS::GetSI()->RegistryEditor.CanWriteToRegistery())
					return CGBLNetDll::ShowServerCannotStartMessage(showMessage);

				// RCG - 08/05/2013 - Bug 8016 - Changed title, message, and OK and Cancel buttons to better reflect question being asked
				// msg 4: Connection to License Manager
				// Do you want to run the license server on this machine?
				if (CGBLForm::CMSMessageBox(
					CMS::GetSI()->ApplicationMainFrame,
					IDS_CONNECT_LICMGR_MSG,
					IDS_CONNECT_LICMGR,
					MB_YESNO | MB_ICONQUESTION) == IDNO)
					return CGBLNetDll::ShowServerCannotStartMessage(showMessage);
				else
				{
					// open license manager address
					if (!CGBLForm::LaunchExecutable(_T("LicSvrUtil.exe"), true, _T("-installconfig")))
						return CGBLNetDll::ShowServerCannotStartMessage(showMessage);

					// server call to CMSPrimaryInfo to check if license fields in contain data
					if (CGBLApplication::GetPrimaryDBInfo())
					{
						CGBLNetDll::TenantReturn tr = CGBLNetDll::GetTenantID((LPCTSTR)si->ApplicationSettings.UserName);
						si->ApplicationSettings.UseTenants = tr.UseTenants;
						if (si->ApplicationSettings.UseTenants)
							si->ApplicationSettings.TenantID = tr.TenantID;
						else
							si->ApplicationSettings.TenantID = _T("NORMAL DENALI");
						if (si->ApplicationSettings.TenantID == EMPTY_STRING)
							return CGBLNetDll::ShowNotTenantMemberMessage(si->ApplicationSettings.UserName, showMessage);
						else
							return true;
					}
					else
						return CGBLNetDll::ShowServerCannotStartMessage(showMessage);
				}
			}
		}
		else if (si->ApplicationSettings.TenantID == EMPTY_STRING)
		{
			return CGBLNetDll::ShowNotTenantMemberMessage(si->ApplicationSettings.UserName, showMessage);
		}
	}
	return true;
}

//---------------------------------------------------------------------------
// BK 5/07/08 implemented Opening Denali Modules Routine design change spec
bool CGBLStartup::LoginLicenseServer(bool showMessage)
{
	if (!CGBLForm::IsLicenseServerRequiredForModule())
		return true;

	CGBLDebug::MemoryCheckpoint(_T("Query License Server"));
	try
	{
		_bstr_t bszLicenseInfo = EMPTY_STRING;
		// is license service running
		if (!CGBLNetDll::IsLicenseServerRunning(bszLicenseInfo))
		{
			// server call to CMSPrimaryInfo to check if license fields in contain data
			if (CGBLApplication::GetPrimaryDBInfo())
			{
				if (TryStartLicenseService(true))
					return CGBLNetDll::SetValidLicenseModuleKeys(bszLicenseInfo);
				// 1-30751 BK 8/27/08 exit if the license server doesn't start
				else
					return false;
			}
			else
			{
				// if CMSLicenseService.exe is not in working directory show message and exit
				if (!CGBLForm::DoesFileExistInCMSPath(_T("CMSLicenseService.exe")))
					return CGBLNetDll::ShowServerCannotStartMessage(showMessage);

				// if user isn't allowed to write to registry show message and exit
				if (!CMS::GetSI()->RegistryEditor.CanWriteToRegistery())
					return CGBLNetDll::ShowServerCannotStartMessage(showMessage);

				// RCG - 08/05/2013 - Bug 8016 - Changed title, message, and OK and Cancel buttons to better reflect question being asked
				// msg 4: Connection to License Manager
				// Do you want to run the license server on this machine?
				if (CGBLForm::CMSMessageBox(
					CMS::GetSI()->ApplicationMainFrame,
					IDS_CONNECT_LICMGR_MSG,
					IDS_CONNECT_LICMGR,
					MB_YESNO | MB_ICONQUESTION) == IDNO)
					return CGBLNetDll::ShowServerCannotStartMessage(showMessage);
				else
				{
					// open license manager address
					if (!CGBLForm::LaunchExecutable(_T("LicSvrUtil.exe"), true, _T("-installconfig")))
						return CGBLNetDll::ShowServerCannotStartMessage(showMessage);

					// server call to CMSPrimaryInfo to check if license fields in contain data
					if (CGBLApplication::GetPrimaryDBInfo())
					{
						if (TryStartLicenseService(false))
							return CGBLNetDll::SetValidLicenseModuleKeys(bszLicenseInfo);
						// 1-30751 BK 8/27/08 exit if the license server doesn't start
						else
							return false;
					}
					else
						return CGBLNetDll::ShowServerCannotStartMessage(showMessage);
				}
			}
		}
		// license server is running - check for valid module key
		else
			return CGBLNetDll::SetValidLicenseModuleKeys(bszLicenseInfo);
	}
	catch (std::runtime_error& err)
	{
		CString szError(err.what());
		CString szCaption = RESSTRING(IDS_COUGAR_MOUNTAIN);
		CGBLForm::ShowCriticalMsg(szError, szCaption);
		return false;
	}
	//return true;
}

//---------------------------------------------------------------------------
// BK 5/07/08 implemented Opening Denali Modules Routine design change spec
// license server is running - if valid module key not found launch program so user can enter keys
bool CGBLStartup::TryStartLicenseService(bool bDisplayMessage)
{
	INT_PTR nReply = IDOK;
	bool ShowMessage = !(si->CommandLineSettings.CT_Quiet || si->CommandLineSettings.Automated_Posting);

	if (bDisplayMessage && ShowMessage)
	{
		if (CGBLNetDll::CanAccessLicenseServer(CMS::GetSI()->ApplicationSettings.LicenseServerMachineName))
		{
			// msg 5: The License Server is not currently running.  Please make sure the license server is available in the network, and then select the Start the Service button or Cancel to exit the application.
			CCMSMessageBox dlgMsg(
				RESSTRING(IDS_NO_LICENSE_SERVER_MSG),
				IDS_NO_LICENSE_SERVER,
				IDS_START_LICENSE_SERVICE);
			nReply = dlgMsg.DoModal();
		}
		else
		{
			nReply = IDCANCEL;
		}
	}
	// try to start license server
	if (nReply == IDOK)
	{
		if (!CGBLNetDll::StartLicenseServer(CMS::GetSI()->ApplicationSettings.LicenseServerMachineName))
		{
			// msg 6: The system was unable to start the license service. Please contact your system administrator or Cougar Mountain Customer Support at (800) 390-7053 for assistance.
			return CGBLNetDll::ShowServerCannotStartMessage(ShowMessage);
		}
	}
	else
	{
		// msg 6: The system was unable to start the license service. Please contact your system administrator or Cougar Mountain Customer Support at (800) 390-7053 for assistance.
		return CGBLNetDll::ShowServerCannotStartMessage(ShowMessage);
	}
	return true;
}

//---------------------------------------------------------------------------
bool CGBLStartup::StartupAfterMainFrame(int nCmdShow)
{

	// 1-5920, DTG, Set application icons to globals, big and small both.
	if (m_pMainFrame && m_pMainFrame->Frame->GetSafeHwnd())
	{
		//m_pMainFrame->Frame->SetIcon(CGBLForm::GetSmallModuleIcon(), false) ;	// small icon
		m_pMainFrame->Frame->SetIcon(CGBLForm::GetSmallModuleIcon(), TRUE);	// big icon
		// BK 6/25/05 moved from below so Sql-to-Sql will run
		m_pMainFrame->CMSMod->StartRefreshThread();
	}

	// begbert 01-29-2009 1-34136 : Moved this prior to logging into the license server
	//// Parse command line for standard shell commands, DDE, file open
	//CCommandLineInfo cmdInfo ;
	//m_pTheApp->ParseCommandLine(cmdInfo) ;

	//////////////////////////////////////////////////////////////////////////
	//  START OF CUSTOM FINALIZATION FOR CMS COMPONENTS
	//////////////////////////////////////////////////////////////////////////
	//Tell the splash thread to shut down the splash screen
	//Use CMSWinApp function to initialize and show mainwnd for the first time
	//Hides splashwnd

	// If we are not in quite mode then show main frame
	//RCG - 05/27/2011 – 9000006 – Automated Posting
	if (!(si->CommandLineSettings.CT_Quiet || si->CommandLineSettings.Automated_Posting) && m_pMainFrame)
		m_pTheApp->InitAndShowMainWnd(m_pMainFrame->Frame, nCmdShow);
	else if (!(si->CommandLineSettings.CT_Quiet || si->CommandLineSettings.Automated_Posting))
	{
		// begbert 01-23-2007 : altered to recover the functionality of hiding the
		//   splash window even if no main frame has been set (as in the case when
		//   the main window is a dialog).
		m_pTheApp->HideSplashWnd();
		CGBLForm::DoEvents();
	}

	// Login to Server, Company and User
	if (!CGBLFormWrap::PerformLoginRoutineAtStartup())
	{
		//KPM.2005.06.29 Don't show the message if its from the command line.
		bool bFromCommandLine = si->CommandLineSettings.DC_DefaultCompany.GetLength() > 0 ? true : false;
		if (si->ApplicationSettings.IsOE() && bFromCommandLine)
			return false;

		//1-5782 Heather Elston 8/12/2003
		//RCG - 05/27/2011 – 9000006 – Automated Posting
		if (CMS::GetSI()->CustomModInstalled(AUTOMATED_POSTING))
		{
			if (si->CommandLineSettings.CT_Quiet || si->CommandLineSettings.Automated_Posting)
			{// TODO: Put in Event Log Code here
			}
			else
			{
				if (!CGBLFormWrap::GetExceededNumberLogins())
					CGBLForm::CMSMessageBox(si->ApplicationMainFrame, IDS_FAILED_LOGIN, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONERROR);
			}
		}
		else
		{
			if (si->CommandLineSettings.CT_Quiet)
			{// TODO: Put in Event Log Code here
			}
			else
			{
				if (!CGBLFormWrap::GetExceededNumberLogins())
					CGBLForm::CMSMessageBox(si->ApplicationMainFrame, IDS_FAILED_LOGIN, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONERROR);
			}
		}
		return false;
	}

	//KPM, 4/29/2004, Load the image information for sales entry.
	if (si->ApplicationSettings.IsSAE() || si->ApplicationSettings.IsIN())
		CGBLImage::LoadImageSettings(si);

	//RCG - 05/27/2011 – 9000006 – Automated Posting
	if (CMS::GetSI()->CustomModInstalled(AUTOMATED_POSTING))
	{
		if (m_pMainFrame && m_pMainFrame->Frame->GetSafeHwnd() && !(si->CommandLineSettings.CT_Quiet || si->CommandLineSettings.Automated_Posting))
			CGBLForm::SendMessageToMainFrame(WM_SETTITLE, 0);
	}
	else
	{
		if (m_pMainFrame && m_pMainFrame->Frame->GetSafeHwnd() && !si->CommandLineSettings.CT_Quiet)
			CGBLForm::SendMessageToMainFrame(WM_SETTITLE, 0);
	}

	//////////////////////////////////////////////////////////////////////////
	//  END OF CUSTOM FINALIZATION FOR CMS COMPONENTS
	//////////////////////////////////////////////////////////////////////////

	//Okay, last chance to exit, now that the thread is done we'll check one more time
	if (si->CancelApplicationLoad) return false;

	//Start the client lock refresh thread
	si->ClientLockRefresh.StartRefreshThread();

	si->ProductAuditAPI = CMSProductAudit::CreateProductAuditObject(si->ApplicationSettings.GetModuleString(si->ApplicationSettings.ModuleID), si->CompanySettings.ID, si->CompanySettings.IsFund);
	if (si->ProductAuditAPI != NULL)
		si->ProductAuditAPI->SendPostEvent();

	// 1-35852 BK 5/11/11 removed this code to further evaluate if a person should be locked out of a module if their date
	// is outside the fiscal calendar. Lori found this by having the COMPUTER date outside the fiscal calendar then opening a
	// module other than controller.
	//if(!si->ApplicationSettings.IsCT())
	//	if(!ValidateDates())
	//		return false ;

	//KPM, 9/23/2003, there is an error that the splash screens are wrong on startup.
	//It's because the globals are not updated at the time the last draw is done.
	//Forcing a redraw at this point will fix the problem.

	//Peter Ringering - 9/24/2003 - 1-8006 - Don't process if null pointer (SQL-2-SQL).
	//RCG - 05/27/2011 – 9000006 – Automated Posting
	if (CMS::GetSI()->CustomModInstalled(AUTOMATED_POSTING))
	{
		if (m_pMainFrame && !(si->CommandLineSettings.CT_Quiet || si->CommandLineSettings.Automated_Posting))
		{
			m_pMainFrame->Frame->Invalidate();
			m_pMainFrame->StatusBar->Update();

			// this is so it will bring itself to the top automatically when
			// launched by the controller. also notify the module tracker
			//that it is okay to initialize itself.
			CWnd* const pMainWnd = AfxGetMainWnd();
			ASSERT_VALID(pMainWnd);
			pMainWnd->SendMessage(WM_CMS_STARTUP_COMPLETED, (WPARAM)0, (LPARAM)0);

			::PostMessage(HWND_BROADCAST, m_pMainFrame->ModuleActivation, (WPARAM)m_pMainFrame->Frame->GetSafeHwnd(), 0);

			if (m_pMainFrame->Frame->GetSafeHwnd())
			{
				if (si->UserInformation.ID == RESSTRING(IDS_SUP)) // TND 6/12/03 manually override all security settings if user is SUP
					si->UserRight.SetDefaultValuesForUserSUP();
				m_pMainFrame->MenuSecurityDisableItems();
			}

			//RCG - 04/25/2013 - PBI 4704 - Modified Custom Enhancement for addition of Custom Menus (PBI 3706)
			// InTimeTec: 11.20.2012.HS - Custom Menus
			CGBLForm::SendMessageToMainFrame(WM_ADD_CUSTOM_MENU, 0);
			// InTimeTec: End
			m_pMainFrame->UpdateMenu();

			CGBLForm::SendMessageToMainFrame(WM_UPDATE_MODULE, 0);
		}
	}
	else
	{
		if (m_pMainFrame && !si->CommandLineSettings.CT_Quiet)
		{
			m_pMainFrame->Frame->Invalidate();
			m_pMainFrame->StatusBar->Update();

			// this is so it will bring itself to the top automatically when
			// launched by the controller. also notify the module tracker
			//that it is okay to initialize itself.
			CWnd* const pMainWnd = AfxGetMainWnd();
			ASSERT_VALID(pMainWnd);
			pMainWnd->SendMessage(WM_CMS_STARTUP_COMPLETED, (WPARAM)0, (LPARAM)0);

			::PostMessage(HWND_BROADCAST, m_pMainFrame->ModuleActivation, (WPARAM)m_pMainFrame->Frame->GetSafeHwnd(), 0);

			if (m_pMainFrame->Frame->GetSafeHwnd())
			{
				if (si->UserInformation.ID == RESSTRING(IDS_SUP)) // TND 6/12/03 manually override all security settings if user is SUP
					si->UserRight.SetDefaultValuesForUserSUP();
				m_pMainFrame->MenuSecurityDisableItems();
			}

			// InTimeTec: 11.20.2012.HS - Custom Menus
			CGBLForm::SendMessageToMainFrame(WM_ADD_CUSTOM_MENU, 0);
			// InTimeTec: End
			m_pMainFrame->UpdateMenu();

			CGBLForm::SendMessageToMainFrame(WM_UPDATE_MODULE, 0);
		}
	}

	//Load countries one time. PGP(07/29/2005) - related to delay in loading customer section in sales etc.
	CCMSCountryComboBox::InitCountries();

	//PTR.07.15.2005 - Event Logging
	bool bSendReady(true);
	if (si->pOutputModule)
		bSendReady = si->pOutputModule->CanGBLDoModuleDone();
	if (bSendReady)
		CGBLEventLog::WriteModuleReady();

	// begbert 07-28-2010 Version 3.0 : Set the title text after the Module Tracker
	//   has been created, so that an instance number can be appended to it.
	if (m_pMainFrame)
		m_pMainFrame->SetTitleText();

	return TRUE;
}

void CGBLStartup::Cleanup(UINT uinMessage)
{
	//Show the error message
	CGBLForm::CMSMessageBox(si->ApplicationMainFrame, uinMessage, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONERROR);
}

CString CGBLStartup::GetNetDLLlang(void)
{
	CXML clsXml;
	clsXml.MakeNode("IDS_PRINTERINVALID", RESSTRING(IDS_PRINTERINVALID), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_PRINTOPTIONDESC", RESSTRING(IDS_PRINTOPTIONDESC), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_PRINTER", RESSTRING(IDS_PRINTER), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_SCREEN", RESSTRING(IDS_SCREEN), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_FILE", RESSTRING(IDS_DOTNET_FILE_LABLE), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_LASERPRINTER", RESSTRING(IDS_LASERPRINTER), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_DOTMATRIX", RESSTRING(IDS_DOTMATRIX), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_NOOFCOPIES", RESSTRING(IDS_NOOFCOPIES), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_OK", RESSTRING(IDS_OK), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_CANCEL", RESSTRING(IDS_CANCEL), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_SETUPPRINTER", RESSTRING(IDS_SETUPPRINTER), 2, XML_ATTRIBUTE);
	//Error:1-4363 4/8/03 JKL -- Changed Node and StringFile from IDS_COUGARVIEWER to IDS_CRYSTALVIEWER
	clsXml.MakeNode("IDS_CRYSTALVIEWER", RESSTRING(IDS_CRYSTALVIEWER), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_CURPRINTER", RESSTRING(IDS_CURPRINTER), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_EXPORT_OPTIONS", RESSTRING(IDS_EXPORT_OPTIONS), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_EXPORTFORMAT", RESSTRING(IDS_EXPORTFORMAT), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_WORDFORMAT", RESSTRING(IDS_WORDFORMAT), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_PDFFORMAT", RESSTRING(IDS_PDFFORMAT), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_EXCELFORMAT", RESSTRING(IDS_EXCELFORMAT), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_HTML40FORMAT", RESSTRING(IDS_HTML40FORMAT), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_XMLFORMAT", RESSTRING(IDS_XMLFORMAT), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_EXPORT_FILENAME", RESSTRING(IDS_REPORT), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_REPORTPRINTOPTIONS", RESSTRING(IDS_REPORTPRINTOPTIONS), 2, XML_ATTRIBUTE);
	clsXml.MakeNode("IDS_DEFAULT", RESSTRING(IDS_DEFAULT), 2, XML_ATTRIBUTE);
	//add for the print report summary AES 6.15.04
	clsXml.MakeNode("IDS_GBL_PRINT_REPORT_PARAMS", RESSTRING(IDS_GBL_PRINT_REPORT_PARAMS), 2, XML_ATTRIBUTE);

	clsXml.m_szXMLFormat = "<LANG>" + clsXml.m_szXMLFormat + "</LANG>";
	clsXml.GetXMLFormat();
	return clsXml.m_szXMLFormat;
}

bool CGBLStartup::SetStatusBar(CCMSStatusBar* pStatusBar)
{
	m_pCMSStatusBar = pStatusBar;
	return true;
}

void CGBLStartup::NotifyThatLoadDocumentTemplatesFailed()
{
	CGBLForm::CMSMessageBox(si->ApplicationMainFrame, IDS_FAILED_LOAD_DOCUMENT_TEMPLATES, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONERROR);
}

void CGBLStartup::HidePassword(CString& szCommandLine, const CString& szPassCmd)
{
	CString szAppPath = _T("\"") + si->ApplicationPaths.szApplicationPath;
	int nStart = szAppPath.GetLength();
	CString szPassParam(_T(""));
	szPassParam.Format(_T("-%s"), szPassCmd);
	int nPassPos = szCommandLine.Find(szPassParam, nStart);
	if (nPassPos >= 0)
	{
		//Can't log password - violates security.
		int nEndPassword = szCommandLine.Find(_T("-"), nPassPos + 3);
		CString szPassword = _T("");
		if (nEndPassword >= 0)
		{
			nEndPassword--;
			szPassword = szCommandLine.Mid(nPassPos, nEndPassword - nPassPos);
		}
		else
		{
			//Password is at the end of the commandline.
			szPassword = szCommandLine.Mid(nPassPos);
		}

		CString szReplace(_T(""));
		szReplace.Format(_T("%s <PASS_HIDE>"), szPassParam);
		szCommandLine.Replace(szPassword, szReplace);
	}
}

//------------------------------------------------------------------------------
// 1-18606 BK 6/27/05 Check Versions see specs DesignChanges.doc
// Compare Client to Primary DB version
CGBLStartup::enuGBLStartReturns CGBLStartup::CheckPrimaryDBVersion()
{
	//We don't need to run this for InstallPrimaryDB.  However, we do need to run this for SQL-2-SQL and CreateDB.
	if (si->ApplicationSettings.StartupModule == CAppSettings::INSTALLPRIMARYDB)
		return GBLStart_Pass;

	//RCG - 03/14/2019 - PBI 3586 - Changed comparison of version numbers from string compare
	auto primaryDataVersion = CGBLApplication::GetPrimaryDataVersion();
	auto primaryDataVersionFromConstants = CMS::GetSI()->ApplicationSettings.PrimaryDataVersion;
	// compare versions
	auto primaryVersion = CGBLForm::CompareVersions(primaryDataVersion, primaryDataVersionFromConstants);

	switch (primaryVersion)
	{
	case enuVersionTime::SAME:
		return GBLStart_Pass;
	case enuVersionTime::NEWER:
		// client is older than Primary data ( 1-32510 BK 4/21/09 include versions in error message )
		CGBLForm::CMSMessageBox(si->ApplicationMainFrame, IDS_VERSIONCHK2_OLDMSG, IDS_VERSIONCHK2, MB_OK | MB_ICONSTOP, primaryDataVersionFromConstants, primaryDataVersion);
		return GBLStart_Fail;
	case enuVersionTime::OLDER:
	{
		// client is newer than Primary data
		// 1-31222 BK 10/29/08 changed text on button from Update Company to Update Primary DB
		CCMSMessageBox dlgMsg(RESSTRING(IDS_VERSIONCHK2_NEWMSG), IDS_VERSIONCHK2, IDS_UPDATE_PRIMARYDB);
		if (dlgMsg.DoModal() == IDOK)
		{
			if (CGBLForm::LaunchExecutable(_T("InstallPrimaryDB.exe"), true, _T("-ATR")))
			{
				if (CGBLFormWrap::ConnectToPrimaryDB())
					return GBLStart_Pass;
				else
					return GBLStart_ShutDown;
			}
		}
	}
	return GBLStart_Fail;
	default:
		throw new CCMSException(_T("Value of enum enuVersionTime is not supported"));
	}
}

//------------------------------------------------------------------------------
bool CGBLStartup::UpdateSplashScreen(UINT uiRes)
{
	CGBLForm::DoEvents();

	//See if the user pressed cancel on the splash screen
	if (si->CancelApplicationLoad)
		return false;

	if (m_bShowSplashWindow)
		m_pTheApp->UpdateSplashMessage(uiRes);
	CGBLDebug::MemoryCheckpoint(uiRes);

	return true;
}

bool CGBLStartup::HandleStartupFailure()
{
	if (CMS::GetSI()->CancelApplicationLoad) return false;
	CGBLForm::ShowCriticalMsg(IDS_FAILED_LOGIN, IDS_COUGAR_MOUNTAIN);
	return false;
}
