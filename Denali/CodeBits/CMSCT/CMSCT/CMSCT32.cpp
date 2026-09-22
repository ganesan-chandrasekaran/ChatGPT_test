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
#include "CMSCT32.h"
#include "MainFrm.h"
#include "../cmsinclude/GBLDataVersion.h"
#include "../CTForms/SetGlobalsCTForms.h"
#include "../GBLForms/SetGlobalsGBLForms.h"
#include "../CTForms/CTDllFormManager.h"
#include "../gblforms/GBLStartup.h"
 //  PBI 22186 BK 8/19/14 Added red flag reports
#include "..\APForms\APDllFormManager.h"
#include "..\ARForms\ARDllFormManager.h"
#include "..\BRForms\BRDllFormManager.h"
#include "..\GLForms\GLDllFormManager.h"
#include "..\INForms\INDllFormManager.h"
#include "..\POForms\PODllFormManager.h"
#include "..\SAForms\SADllFormManager.h"

/////////////////////////////////////////////////////////////////////////////
// The one and only CCMSCT32App object
CCMSCT32App theApp;
CGBLSystemInformation g_GBLSystemInformation;

// -------------------------------------------------------------
BEGIN_MESSAGE_MAP(CCMSCT32App, CCMSWinApp)
END_MESSAGE_MAP()

// -------------------------------------------------------------
CCMSCT32App::CCMSCT32App()
{
}

// -------------------------------------------------------------
BOOL CCMSCT32App::InitInstance()
{
	__super::InitInstance();

	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	// Give GBLForms information about its globals
	CSetGlobalsGBLForms	clsSetGlobalsGBLForms(&g_GBLSystemInformation);

	// Give CTForms information about its globals
	CSetGlobalsCTForms clsSetGlobalsCTForms(&g_GBLSystemInformation);

	//KPM.2005.05.24 Uniquely identify this executable
	g_GBLSystemInformation.ApplicationSettings.StartupModule = CAppSettings::CMSCT; // begbert 02-17-2006 VS8: warning C4482
	g_GBLSystemInformation.pChildFrame = RUNTIME_CLASS(CCMSMDIChildWnd);
	g_GBLSystemInformation.pDocument = RUNTIME_CLASS(CCMSMaintDoc);

	CGBLStartup clsStartup(IDS_CT_MODULE, &g_GBLSystemInformation, &theApp);
	if (!clsStartup.StartupBeforeMainFrame())
		return FALSE;

	// create main MDI Frame window
	CMainFrame* const pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		if (!g_GBLSystemInformation.CommandLineSettings.CT_Quiet)
		{
			//sb - changed to use AfxMessageBox since, if LoadFrame fails, g_GBLSystemInformation.ApplicationMainFrame
			//will not be valid for CMSMessageBox to use.
			::AfxMessageBox(CGBLResources::GetResourceString(IDS_GL_FAILED_TO_CREATE_MAINFRM), MB_OK | MB_ICONERROR);
			PostQuitMessage(1);
		}
		else
		{
			// TODO:Need event log info added here
		}
		PostQuitMessage(1);
	}

	// Set the startup class values
	m_pMainWnd = pMainFrame;
	clsStartup.SetFrameWnd(pMainFrame);

	m_pFormManager.reset(new CCTDllFormManager(this));
	if (!m_pFormManager->AddDocTemplates()) return FALSE;

	// PBI 22186 BK 8/19/14 Added red flag reports
	AddSharedDocTemplates();

	// begbert 11-08-2010 1-35067 : Added the bClosing flag to let us know not to spend time starting the DUI.
	bool bClosing = false;
	// Finish the startup routine
	if (!clsStartup.StartupAfterMainFrame())
	{
		PostQuitMessage(1);
		bClosing = true;
	}

	if (g_GBLSystemInformation.CommandLineSettings.CT_Backup)
	{
		// DWP - 4866 - 7/29/13 - If the backup does not start then we want to return FALSE here
		return pMainFrame->BackupcompanyBackup();
	}

	// begbert 08-19-2010 1-34292 : show the Denali Office Interface pane if the CT Module Preferences don't say not to.
	// begbert 11-09-2010 1-34979 : Altered to use combo files.
	// begbert 11-12-2010 1-35115 : Moved the check for whether we're in classic view inside CreateDenaliOfficeView.
	// 1-35360 BK 4/27/11 don't try to load OfficeView when doing a backup from commandline
	if (!bClosing && !g_GBLSystemInformation.CommandLineSettings.CT_Backup)
		((CCMSMDIMainFrame*)m_pMainWnd)->CreateDenaliOfficeView(_T("ControllerCOMBO.XML"), _T("ControllerFNDCOMBO.XML"));

	return TRUE;
}

// -------------------------------------------------------------
// PBI 22186 BK 8/19/14 Added red flag reports
void CCMSCT32App::AddSharedDocTemplates(void)
{
	// AP module
	CAPDllFormManager clsAPFormManager(this);
	m_pFormManager->RegisterSharedFormsFrom(clsAPFormManager);

	// AR module
	CARDllFormManager clsARFormManager(this);
	m_pFormManager->RegisterSharedFormsFrom(clsARFormManager);

	// BR module
	CBRDllFormManager clsBRFormManager(this);
	m_pFormManager->RegisterSharedFormsFrom(clsBRFormManager);

	// GL module
	CGLDllFormManager clsGLFormManager(this);
	m_pFormManager->RegisterSharedFormsFrom(clsGLFormManager);

	// IN module
	CINDllFormManager clsINFormManager(this);
	m_pFormManager->RegisterSharedFormsFrom(clsINFormManager);

	// PO module
	CPODllFormManager clsPOFormManager(this);
	m_pFormManager->RegisterSharedFormsFrom(clsPOFormManager);

	// Sales
	CSADllFormManager clsSAFormManager(this);
	m_pFormManager->RegisterSharedFormsFrom(clsSAFormManager);
}

// -------------------------------------------------------------
int CCMSCT32App::ExitInstance()
{
	if (g_GBLSystemInformation.RunningInVirtualUI)
	{
		g_GBLSystemInformation.VUI->Stop();
	}
	if (m_pMainWnd && ::IsWindow(m_pMainWnd->GetSafeHwnd())) m_pMainWnd->SendMessage(WM_DESTROY);
	Sleep(100);
	return __super::ExitInstance();
}