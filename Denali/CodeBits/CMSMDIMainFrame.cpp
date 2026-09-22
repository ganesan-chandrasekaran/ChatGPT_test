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
#include "..\cmsdll\CMSModuleDataRepository.h"
#include "CMSMDIMainFrame.h"
#include "..\cmsinclude\HelpTopicIDS.h"

 // begbert 07-20-2010 3.0 Interface : Headers to provide the necessary objects so that
 //   the CommandGateway function can call the various functionality it needs to.
#include "../INForms/resource.h"

// CCMSMDIMainFrame

// begbert 12-02-2010 1-35197 : Make note of whether the window was destroyed
//   in a static variable that will survive the window object, so that we can
//   check whether the DUI creation function has had the its window objects
//   deleted out from under it.
static bool s_bIsDestroyed = false;

//------------------------------------------------------------------------------
CCMSMDIMainFrame::CCMSMDIMainFrame(void)
// begbert 07-20-2010 3.0 Interface : initializer for the office view
// begbert 01-11-2011 1-35253 : initializer for m_bDisableExit
	: m_hDotNetDLL(NULL), m_pDenaliOfficeView(NULL), m_pWebBrowserView(NULL), m_bDisableExit(false)
{
}

//------------------------------------------------------------------------------
CCMSMDIMainFrame::~CCMSMDIMainFrame(void)
{
	if (m_hDotNetDLL != NULL)
		AfxFreeLibrary(m_hDotNetDLL);
}

//------------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CCMSMDIMainFrame, CCMSBaseMainFrameWnd<CMDIFrameWnd>)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_INITMENUPOPUP()
	ON_WM_TIMER()

	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, &CCMSMDIMainFrame::OnToolTipNotify)
	ON_COMMAND_RANGE(WINDOW_ACTIVATE_COMMAND_ID_START, WINDOW_ACTIVATE_COMMAND_ID_END, &CCMSMDIMainFrame::OnWindowActivate)

	// module toolbar buttons
	ON_UPDATE_COMMAND_UI_RANGE(IDS_LAUNCH_AP, IDS_LAUNCH_AP + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnEnableAccountsPayable)
	ON_UPDATE_COMMAND_UI_RANGE(IDS_LAUNCH_AR, IDS_LAUNCH_AR + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnEnableAccountsReceivable)
	ON_UPDATE_COMMAND_UI_RANGE(IDS_LAUNCH_BR, IDS_LAUNCH_BR + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnEnableBankReconciliation)
	ON_UPDATE_COMMAND_UI_RANGE(IDS_LAUNCH_GL, IDS_LAUNCH_GL + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnEnableGeneralLedger)
	ON_UPDATE_COMMAND_UI_RANGE(IDS_LAUNCH_IN, IDS_LAUNCH_IN + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnEnableInventory)
	ON_UPDATE_COMMAND_UI_RANGE(IDS_LAUNCH_JC, IDS_LAUNCH_JC + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnEnableJobCost)
	ON_UPDATE_COMMAND_UI_RANGE(IDS_LAUNCH_OE, IDS_LAUNCH_OE + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnEnableOrderEntry)
	ON_UPDATE_COMMAND_UI_RANGE(IDS_LAUNCH_PR, IDS_LAUNCH_PR + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnEnablePayroll)
	ON_UPDATE_COMMAND_UI_RANGE(IDS_LAUNCH_PS, IDS_LAUNCH_PS + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnEnableSalesEntry)
	ON_UPDATE_COMMAND_UI_RANGE(IDS_LAUNCH_PO, IDS_LAUNCH_PO + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnEnablePurchaseOrder)
	ON_UPDATE_COMMAND_UI_RANGE(IDS_LAUNCH_SA, IDS_LAUNCH_SA + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnEnableSalesMaintenance)
	// 1-34692 BK 8/19/10 added mod prefs to controller
	ON_UPDATE_COMMAND_UI(IDS_CONTROLLER, &CCMSMDIMainFrame::OnEnableController)
	// 1-34642/1-34645 BK 8/19/10 added submenus to Cougar Dtails, financial & sales dashboards
	ON_UPDATE_COMMAND_UI(IDS_FILE_COUGAR_DTAILS, &CCMSMDIMainFrame::OnEnableCougarDtails)
	ON_UPDATE_COMMAND_UI(IDS_MENU_DENALIBI, &CCMSMDIMainFrame::OnEnableDenaliBI)
	ON_UPDATE_COMMAND_UI(IDS_MENU_FINANCIAL_DASHBOARD, &CCMSMDIMainFrame::OnEnableFinancialDashboard)
	ON_UPDATE_COMMAND_UI(IDS_MENU_SALES_DASHBOARD, &CCMSMDIMainFrame::OnEnableSalesDashboard)

	// enabling disabling toolbar buttons
	ON_UPDATE_COMMAND_UI(IDS_FILE, &CCMSMDIMainFrame::OnEnableFile)
	ON_UPDATE_COMMAND_UI(IDS_EXIT, &CCMSMDIMainFrame::OnEnableAppExitButton)
	ON_UPDATE_COMMAND_UI(IDS_SAVE, &CCMSMDIMainFrame::OnEnableSaveButton)
	ON_UPDATE_COMMAND_UI(IDS_PRINT, &CCMSMDIMainFrame::OnEnablePrintButton)
	ON_UPDATE_COMMAND_UI(IDS_CLEAR, &CCMSMDIMainFrame::OnEnableClearButton)
	ON_UPDATE_COMMAND_UI(IDS_CASCADE, &CCMSMDIMainFrame::OnEnableCascade)
	ON_UPDATE_COMMAND_UI(IDS_ARRANGE_ICONS, &CCMSMDIMainFrame::OnEnableArrangeIcons)
	ON_UPDATE_COMMAND_UI(IDS_TILE_HORIZONTALLY, &CCMSMDIMainFrame::OnEnableTileHorizontally)
	ON_UPDATE_COMMAND_UI(IDS_TILE_VERTICALLY, &CCMSMDIMainFrame::OnEnableTileVertically)
	ON_UPDATE_COMMAND_UI(IDS_CLOSE_ALL, &CCMSMDIMainFrame::OnEnableCloseAll)
	ON_UPDATE_COMMAND_UI(IDS_HELP, &CCMSMDIMainFrame::OnEnableHelpButton)
	ON_UPDATE_COMMAND_UI(IDS_DISPLAY_TOOLBAR, &CCMSMDIMainFrame::OnUpdateDisplayToolbarMenu)

	ON_COMMAND_RANGE(IDS_LAUNCH_AP, IDS_LAUNCH_AP + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnFileAccountsPayable)
	ON_COMMAND_RANGE(IDS_LAUNCH_AR, IDS_LAUNCH_AR + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnFileAccountsReceivable)
	ON_COMMAND_RANGE(IDS_LAUNCH_BR, IDS_LAUNCH_BR + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnFileBankReconciliation)
	ON_COMMAND_RANGE(IDS_LAUNCH_GL, IDS_LAUNCH_GL + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnFileGeneralLedger)
	ON_COMMAND_RANGE(IDS_LAUNCH_IN, IDS_LAUNCH_IN + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnFileInventory)
	ON_COMMAND_RANGE(IDS_LAUNCH_JC, IDS_LAUNCH_JC + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnFileJobCost)
	ON_COMMAND_RANGE(IDS_LAUNCH_OE, IDS_LAUNCH_OE + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnFileOrderEntry)
	ON_COMMAND_RANGE(IDS_LAUNCH_PR, IDS_LAUNCH_PR + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnFilePayroll)
	ON_COMMAND_RANGE(IDS_LAUNCH_PS, IDS_LAUNCH_PS + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnFileSalesEntry)
	ON_COMMAND_RANGE(IDS_LAUNCH_PO, IDS_LAUNCH_PO + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnFilePurchaseOrder)
	ON_COMMAND_RANGE(IDS_LAUNCH_SA, IDS_LAUNCH_SA + MAX_MODULE_INSTANCE_CNT, &CCMSMDIMainFrame::OnFileSalesMaintenance)
	// 1-34692 BK 8/19/10 added mod prefs to controller
	ON_COMMAND(IDS_CONTROLLER, &CCMSMDIMainFrame::OnFileController)
	ON_COMMAND(IDS_MENU_DENALIBI, &CCMSMDIMainFrame::OnFileDenaliBI)
	// 1-34642/1-34645 BK 8/19/10 added submenus to Cougar Dtails, financial & sales dashboards
	ON_COMMAND(IDS_MENU_FINANCIAL_DASHBOARD, &CCMSMDIMainFrame::OnFileFinancialDashboard)
	ON_COMMAND(IDS_MENU_SALES_DASHBOARD, &CCMSMDIMainFrame::OnFileSalesDashboard)

	//ON_COMMAND(IDS_EXIT,									&CCMSMDIMainFrame::OnAppExit)
	ON_COMMAND(IDS_SAVE, &CCMSMDIMainFrame::OnFileSave)
	ON_COMMAND(IDS_CLEAR, &CCMSMDIMainFrame::OnFileClear)
	ON_COMMAND(IDS_PRINT, &CCMSMDIMainFrame::OnFilePrint)
	ON_COMMAND(IDS_HELP, &CCMSMDIMainFrame::OnHelp)
	ON_COMMAND(IDS_TILE_VERTICALLY, &CCMSMDIMainFrame::OnWindowTileVertical)
	ON_COMMAND(IDS_TILE_HORIZONTALLY, &CCMSMDIMainFrame::OnWindowTileHorizontal)
	ON_COMMAND(IDS_CASCADE, &CCMSMDIMainFrame::OnWindowCascade)
	ON_COMMAND(IDS_ARRANGE_ICONS, &CCMSMDIMainFrame::OnWindowArrangeIcons)
	ON_COMMAND(IDS_CLOSE_ALL, &CCMSMDIMainFrame::OnWindowCloseAll)
	ON_COMMAND(IDS_MENU_CONTENTS, &CCMSMDIMainFrame::OnHelpContents)
	ON_COMMAND(IDS_ABOUT_TITLE, &CCMSMDIMainFrame::OnHelpAboutCougarMountainSoftware)
	ON_COMMAND(IDS_HELPMENU_TUTORIALS, &CCMSMDIMainFrame::OnHelpTutorials)
	ON_COMMAND(IDS_HELPMENU_VIDEOTUTORIALS, &CCMSMDIMainFrame::OnHelpVideoTutorials)
	ON_COMMAND(IDS_HELPMENU_CMS_WEBSITE, &CCMSMDIMainFrame::OnHelpCMSWebsite)
	ON_COMMAND(IDS_HELPMENU_ONLINE_SUPPORT, &CCMSMDIMainFrame::OnHelpOnlineSupport)
	ON_COMMAND(IDS_HELPMENU_ACTIVATE_SOFTWARE, &CCMSMDIMainFrame::OnHelpActivateSoftware)
	ON_COMMAND(IDS_HELPMENU_ORDER_CMS_FORMS, &CCMSMDIMainFrame::OnHelpOrderCougarForms)
	ON_COMMAND(IDS_HELPMENU_MERCHANT_SERVICES, &CCMSMDIMainFrame::OnHelpMerchantTransactionServices)
	ON_COMMAND(IDS_SHOW_XML_VIEWER_ON_ERROR, &CCMSMDIMainFrame::OnDeveloperShowXMLViewerOnError)
	ON_COMMAND(IDS_SHOW_XML_VIEWER, &CCMSMDIMainFrame::OnDeveloperShowXMLViewer)
	ON_COMMAND(IDS_GBL_SELECT_DIFFERENT_COMPANY, &CCMSMDIMainFrame::OnSelectDifferentCompany)
	ON_COMMAND(IDS_WNDSPY, &CCMSMDIMainFrame::OnDeveloperShowWndSpy)
	ON_COMMAND(IDS_ASSERTONERROR, &CCMSMDIMainFrame::OnDeveloperAssertOnError)
	ON_COMMAND(IDS_STAY_IN_ADD_MODE_ON_SAVE, &CCMSMDIMainFrame::OnOptionsStayInAddModeOnSave)
	ON_COMMAND(IDS_DISPLAY_TOOLBAR, &CCMSMDIMainFrame::OnDisplayToolbar)
	ON_COMMAND(IDS_MENU_GL_DISPLAY_STATUSBAR, &CCMSMDIMainFrame::OnDisplayStatusbar)

	ON_COMMAND(IDS_GBL_MNU_DENALI_CLIENT_UTILITIES, &CCMSMDIMainFrame::OnDenaliClientUtilities)
	ON_COMMAND(IDS_GBL_MNU_DENALI_SERVER_UTILITIES, &CCMSMDIMainFrame::OnDenaliServerUtilities)
	ON_COMMAND(IDS_GBL_MNU_DENALI_LICENSE_UTILITIES, &CCMSMDIMainFrame::OnDenaliLicenseUtilities)

	ON_COMMAND_RANGE(MENU_BATCH_IDRANGE_START, MENU_BATCH_IDRANGE_END, &CCMSMDIMainFrame::OnSelectBatch)

	ON_MESSAGE(WM_HELP, &CCMSMDIMainFrame::OnShowHelp)			// BK 11/03/04 get message from CMSMessageBox Help button
	ON_MESSAGE(WM_SHOWHELP, &CCMSMDIMainFrame::OnShowHelp)
	ON_MESSAGE(WM_CMS_SETUPLOCALIZATION, &CCMSMDIMainFrame::OnSetupLocalization)
	ON_MESSAGE(WM_CMS_WAIT_STRING_CHANGE, &CCMSMDIMainFrame::OnWaitStringChange)
	ON_MESSAGE(WM_CMS_WAIT_STRING_CHANGE1, &CCMSMDIMainFrame::OnWaitStringChange1)
	ON_MESSAGE(WM_UPDATE_CHANGE_DATE, &CCMSMDIMainFrame::OnShowChangeDate)

	ON_REGISTERED_MESSAGE(WM_CMS_PING_APP, &CCMSMDIMainFrame::OnPingApp)
	ON_REGISTERED_MESSAGE(WM_CMS_ENSURE_APP_VISIBLE, &CCMSMDIMainFrame::OnEnsureAppVisible)
	ON_REGISTERED_MESSAGE(WM_CMS_STARTUP_COMPLETED, &CCMSMDIMainFrame::OnStartupCompleted)
	ON_MESSAGE(WM_USER_MESSAGE_1, &CCMSMDIMainFrame::OnNavigatorDataRequestReturn)
END_MESSAGE_MAP()

// CCMSMDIMainFrame message handlers

//------------------------------------------------------------------------------
int CCMSMDIMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	else if (!m_wndMdiBackground.SubclassWindow(m_hWndMDIClient))
		return -1;

	return 0;
}

//------------------------------------------------------------------------------
void CCMSMDIMainFrame::OnDestroy(void)
{
	s_bIsDestroyed = true; // begbert 12-02-2010 1-35197 : Note that the window has been destroyed.
	__super::OnDestroy();
}

//------------------------------------------------------------------------------
BOOL CCMSMDIMainFrame::OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	return __super::OnToolTipNotify(id, pNMHDR, pResult);
}

//------------------------------------------------------------------------------
void CCMSMDIMainFrame::OnTimer(UINT_PTR unIDEvent)
{
	__super::OnTimer(unIDEvent);
}

//------------------------------------------------------------------------------
void CCMSMDIMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSystemMenu)
{
	__super::OnInitMenuPopup(pPopupMenu, nIndex, bSystemMenu);
}

//------------------------------------------------------------------------------
void CCMSMDIMainFrame::OnWindowActivate(UINT nID)
{
	__super::OnWindowActivate(nID);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableAccountsPayable(CCmdUI* pCmdUI)
{
	__super::OnCmdUIEnableMenu(pCmdUI, IDS_LAUNCH_AP);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableAccountsReceivable(CCmdUI* pCmdUI)
{
	__super::OnCmdUIEnableMenu(pCmdUI, IDS_LAUNCH_AR);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableBankReconciliation(CCmdUI* pCmdUI)
{
	__super::OnCmdUIEnableMenu(pCmdUI, IDS_LAUNCH_BR);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableController(CCmdUI* pCmdUI)
{
	if (ModuleTracker.get())
		ModuleTracker->UpdateInstance(IDS_CT_MODULE, IDS_CONTROLLER);
	pCmdUI->Enable(CCMSModuleInfo::Instance().CanLaunch(IDS_CONTROLLER));
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableGeneralLedger(CCmdUI* pCmdUI)
{
	__super::OnCmdUIEnableMenu(pCmdUI, IDS_LAUNCH_GL);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableInventory(CCmdUI* pCmdUI)
{
	__super::OnCmdUIEnableMenu(pCmdUI, IDS_LAUNCH_IN);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableJobCost(CCmdUI* pCmdUI)
{
	__super::OnCmdUIEnableMenu(pCmdUI, IDS_LAUNCH_JC);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableOrderEntry(CCmdUI* pCmdUI)
{
	__super::OnCmdUIEnableMenu(pCmdUI, IDS_LAUNCH_OE);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableOrderMaintenance(CCmdUI* pCmdUI)
{
	__super::OnCmdUIEnableMenu(pCmdUI, IDS_ORDER_MAINTENANCE);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnablePurchaseOrder(CCmdUI* pCmdUI)
{
	__super::OnCmdUIEnableMenu(pCmdUI, IDS_LAUNCH_PO);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnablePointOfSaleEntry(CCmdUI* pCmdUI)
{
	__super::OnCmdUIEnableMenu(pCmdUI, IDS_LAUNCH_PS);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnablePointOfSaleMaintenance(CCmdUI* pCmdUI)
{
	__super::OnCmdUIEnableMenu(pCmdUI, IDS_POINT_OF_SALE_MAINTENANCE);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnablePayroll(CCmdUI* pCmdUI)
{
	__super::OnCmdUIEnableMenu(pCmdUI, IDS_LAUNCH_PR);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableSalesEntry(CCmdUI* pCmdUI)
{
	__super::OnCmdUIEnableMenu(pCmdUI, IDS_LAUNCH_PS);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableSalesMaintenance(CCmdUI* pCmdUI)
{
	__super::OnCmdUIEnableMenu(pCmdUI, IDS_LAUNCH_SA);
}

//------------------------------------------------------------------------------
// 1-34642/1-34645 BK 8/19/10 added submenus to Cougar Dtails, financial & sales dashboards
// begbert 02-08-2010 CDtails : Added this Financial Dashboard
void	CCMSMDIMainFrame::OnEnableCougarDtails(CCmdUI* pCmdUI)
{
	__super::OnEnableCougarDtails(pCmdUI);
}
//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableDenaliBI(CCmdUI* pCmdUI)
{
	__super::OnEnableDenaliBI(pCmdUI);
}
//------------------------------------------------------------------------------
// 1-34642/1-34645 BK 8/19/10 added submenus to Cougar Dtails, financial & sales dashboards
void	CCMSMDIMainFrame::OnEnableFinancialDashboard(CCmdUI* pCmdUI)
{
	__super::OnEnableFinancialDashboard(pCmdUI);
}
//------------------------------------------------------------------------------
// 1-34642/1-34645 BK 8/19/10 added submenus to Cougar Dtails, financial & sales dashboards
void	CCMSMDIMainFrame::OnEnableSalesDashboard(CCmdUI* pCmdUI)
{
	__super::OnEnableSalesDashboard(pCmdUI);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableFile(CCmdUI* pCmdUI)
{
	__super::OnEnableFile(pCmdUI);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableAppExitButton(CCmdUI* pCmdUI)
{
	if (m_bDisableExit)
		pCmdUI->Enable(false); // begbert 01-11-2011 1-35253 : Disable the button if necessary
	else
		__super::OnEnableAppExitButton(pCmdUI);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableSaveButton(CCmdUI* pCmdUI)
{
	__super::OnEnableSaveButton(pCmdUI);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnablePrintButton(CCmdUI* pCmdUI)
{
	__super::OnEnablePrintButton(pCmdUI);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableClearButton(CCmdUI* pCmdUI)
{
	__super::OnEnableClearButton(pCmdUI);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableCascade(CCmdUI* pCmdUI)
{
	__super::OnEnableArrangeOpenWindows(pCmdUI);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableArrangeIcons(CCmdUI* pCmdUI)
{
	__super::OnEnableArrangeIconicWindows(pCmdUI);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableTileHorizontally(CCmdUI* pCmdUI)
{
	__super::OnEnableArrangeOpenWindows(pCmdUI);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableTileVertically(CCmdUI* pCmdUI)
{
	__super::OnEnableArrangeOpenWindows(pCmdUI);
}
//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableCloseAll(CCmdUI* pCmdUI)
{
	__super::OnEnableCloseAll(pCmdUI);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnEnableHelpButton(CCmdUI* pCmdUI)
{
	__super::OnEnableHelpButton(pCmdUI);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnUpdateDisplayToolbarMenu(CCmdUI* pCmdUI)
{
	__super::OnUpdateDisplayToolbarMenu(pCmdUI);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnFileAccountsPayable(UINT nID)
{
	__super::OnFileAccountsPayable(nID);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnFileAccountsReceivable(UINT nID)
{
	__super::OnFileAccountsReceivable(nID);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnFileBankReconciliation(UINT nID)
{
	__super::OnFileBankReconciliation(nID);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnFileGeneralLedger(UINT nID)
{
	__super::OnFileGeneralLedger(nID);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnFileInventory(UINT nID)
{
	__super::OnFileInventory(nID);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnFileJobCost(UINT nID)
{
	__super::OnFileJobCost(nID);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnFileOrderEntry(UINT nID)
{
	__super::OnFileOrderEntry(nID);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnFilePayroll(UINT nID)
{
	__super::OnFilePayroll(nID);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnFileSalesEntry(UINT nID)
{
	__super::OnFileSalesEntry(nID);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnFilePurchaseOrder(UINT nID)
{
	__super::OnFilePurchaseOrder(nID);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnFileSalesMaintenance(UINT nID)
{
	__super::OnFileSalesMaintenance(nID);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnFileController(void)
{
	__super::OnFileController();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnFileDenaliBI(void)
{
	__super::OnFileDenaliBI();
}

//------------------------------------------------------------------------------
// 1-34642/1-34645 BK 8/19/10 added submenus to Cougar Dtails, financial & sales dashboards
void	CCMSMDIMainFrame::OnFileFinancialDashboard(void)
{
	__super::OnFileFinancialDashboard();
}
//------------------------------------------------------------------------------
// 1-34642/1-34645 BK 8/19/10 added submenus to Cougar Dtails, financial & sales dashboards
void	CCMSMDIMainFrame::OnFileSalesDashboard(void)
{
	__super::OnFileSalesDashboard();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnAppExit(void)
{
	__super::OnAppExit();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnFileSave(void)
{
	__super::OnFileSave();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnFileClear(void)
{
	//Defect ID 1-28460, PGP(10-20-2007)
	__super::OnFileClear();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnFilePrint(void)
{
	__super::OnFilePrint();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnHelp(void)
{
	__super::OnHelp();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnWindowTileHorizontal(void)
{
	BOOL OfficeEnable = TRUE;
	if (m_pDenaliOfficeView != NULL)
	{
		OfficeEnable = m_pDenaliOfficeView->GetParentFrame()->IsWindowEnabled();
		m_pDenaliOfficeView->GetParentFrame()->EnableWindow(FALSE);
	}

	MDITile(MDITILE_HORIZONTAL + MDITILE_SKIPDISABLED);   // Tile MDI child windows horizontal

	if (m_pDenaliOfficeView != NULL)
		m_pDenaliOfficeView->GetParentFrame()->EnableWindow(OfficeEnable);
	else
	{
		// Erick Korsten - 04/24/03 - Defect ID 1-4552 - Move active frame a bit to force the scrollbar to show
		CFrameWnd* pFrameWnd = CGBLForm::EkGetActiveFrame();
		CRect rc;
		pFrameWnd->GetWindowRect(&rc);
		pFrameWnd->SetWindowPos(NULL, 1, 1, rc.Width(), rc.Height(), SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER);
	}
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnWindowTileVertical(void)
{
	BOOL OfficeEnable = TRUE;
	if (m_pDenaliOfficeView != NULL)
	{
		OfficeEnable = m_pDenaliOfficeView->GetParentFrame()->IsWindowEnabled();
		m_pDenaliOfficeView->GetParentFrame()->EnableWindow(FALSE);
	}

	MDITile(MDITILE_VERTICAL + MDITILE_SKIPDISABLED);   // Tile MDI child windows vertical

	if (m_pDenaliOfficeView != NULL)
		m_pDenaliOfficeView->GetParentFrame()->EnableWindow(OfficeEnable);
	else
	{
		// Erick Korsten - 04/24/03 - Defect ID 1-4552 - Move active frame a bit to force the scrollbar to show
		CFrameWnd* pFrameWnd = CGBLForm::EkGetActiveFrame();
		CRect rc;
		pFrameWnd->GetWindowRect(&rc);
		pFrameWnd->SetWindowPos(NULL, 1, 1, rc.Width(), rc.Height(), SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER);
	}
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnWindowCascade(void)
{
	int CascadeSettings = MDITILE_SKIPDISABLED;
#if(_WIN32_WINNT >= 0x0500)
	CascadeSettings += MDITILE_ZORDER;
#endif /* _WIN32_WINNT >= 0x0500 */

	BOOL OfficeEnable = TRUE;
	if (m_pDenaliOfficeView != NULL)
	{
		OfficeEnable = m_pDenaliOfficeView->GetParentFrame()->IsWindowEnabled();
		m_pDenaliOfficeView->GetParentFrame()->EnableWindow(FALSE);
	}

	MDICascade(CascadeSettings);                // Arrange all the MDI child windows in a cascade format

	if (m_pDenaliOfficeView != NULL)
		m_pDenaliOfficeView->GetParentFrame()->EnableWindow(OfficeEnable);
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnWindowArrangeIcons(void)
{
	MDIIconArrange();               // Arrange all minimized document child windows
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnWindowCloseAll(void)
{
	__super::OnWindowCloseAll();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnHelpContents(void)
{
	__super::OnHelpContents();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnHelpAboutCougarMountainSoftware(void)
{
	__super::OnHelpAboutCougarMountainSoftware();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnHelpTutorials(void)
{
	__super::OnHelpTutorials();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnHelpVideoTutorials(void)
{
	__super::OnHelpVideoTutorials();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnHelpCMSWebsite(void)
{
	__super::OnHelpCMSWebsite();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnHelpOnlineSupport(void)
{
	__super::OnHelpOnlineSupport();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnHelpActivateSoftware(void)
{
	__super::OnHelpActivateSoftware();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnHelpOrderCougarForms(void)
{
	__super::OnHelpOrderCougarForms();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnHelpMerchantTransactionServices(void)
{
	__super::OnHelpMerchantTransactionServices();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnDeveloperShowXMLViewer(void)
{
	__super::OnDeveloperShowXMLViewer();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnDeveloperShowXMLViewerOnError(void)
{
	__super::OnDeveloperShowXMLViewerOnError();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnSelectDifferentCompany(void)
{
	__super::OnSelectDifferentCompany();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnDeveloperShowWndSpy(void)
{
	__super::OnDeveloperShowWndSpy();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnDeveloperAssertOnError(void)
{
	__super::OnDeveloperAssertOnError();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnOptionsStayInAddModeOnSave(void)
{
	__super::OnOptionsStayInAddModeOnSave();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnDisplayToolbar(void)
{
	__super::OnDisplayToolbar();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnDisplayStatusbar(void)
{
	__super::OnDisplayStatusbar();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnDenaliClientUtilities(void)
{
	__super::OnDenaliClientUtilities();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnDenaliServerUtilities(void)
{
	__super::OnDenaliServerUtilities();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnDenaliLicenseUtilities(void)
{
	__super::OnDenaliLicenseUtilities();
}

//------------------------------------------------------------------------------
void	CCMSMDIMainFrame::OnSelectBatch(UINT uItemID)
{
	__super::OnSelectBatch(uItemID);
}

//------------------------------------------------------------------------------
LRESULT	CCMSMDIMainFrame::OnShowHelp(WPARAM wParam, LPARAM lParam)
{
	return __super::OnShowHelp(wParam, lParam);
}

//------------------------------------------------------------------------------
LRESULT	CCMSMDIMainFrame::OnSetupLocalization(WPARAM wParam, LPARAM lParam)
{
	return __super::OnSetupLocalization(wParam, lParam);
}

//------------------------------------------------------------------------------
LRESULT	CCMSMDIMainFrame::OnWaitStringChange(WPARAM wParam, LPARAM lParam)
{
	return __super::OnWaitStringChange(wParam, lParam);
}

//------------------------------------------------------------------------------
LRESULT	CCMSMDIMainFrame::OnWaitStringChange1(WPARAM wParam, LPARAM lParam)
{
	return __super::OnWaitStringChange1(wParam, lParam);
}

//------------------------------------------------------------------------------
LRESULT	CCMSMDIMainFrame::OnShowChangeDate(WPARAM wParam, LPARAM lParam)
{
	return __super::OnShowChangeDate(wParam, lParam);
}

//------------------------------------------------------------------------------
LRESULT	CCMSMDIMainFrame::OnPingApp(WPARAM wParam, LPARAM lParam)
{
	return __super::OnPingApp(wParam, lParam);
}

//------------------------------------------------------------------------------
LRESULT	CCMSMDIMainFrame::OnEnsureAppVisible(WPARAM wParam, LPARAM lParam)
{
	return __super::OnEnsureAppVisible(wParam, lParam);
}

//------------------------------------------------------------------------------
LRESULT	CCMSMDIMainFrame::OnStartupCompleted(WPARAM wParam, LPARAM lParam)
{
	return __super::OnStartupCompleted(wParam, lParam);
}

//------------------------------------------------------------------------------
// begbert 07-20-2010 3.0 Interface : Notify the child windows when the parent
//   changes size, so that the office view can resize itself to fit.
void CCMSMDIMainFrame::RecalcLayout(BOOL bNotify/*=TRUE*/)
{
	__super::RecalcLayout(bNotify);

	if (::IsWindow(m_hWndMDIClient))
	{
		CWnd* pClientWnd = CWnd::FromHandle(m_hWndMDIClient);

		pClientWnd->SendMessageToDescendants(WM_SIZEPARENT,
			0, 0, FALSE, FALSE);
	}
}

//------------------------------------------------------------------------------
// begbert 07-20-2010 3.0 Interface : Call RecalcLayout to ensure the office
//   view's position is kept up to date.
void CCMSMDIMainFrame::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	RecalcLayout();
}

// begbert 07-20-2010 3.0 Interface : Load the CMSDotNet dll and create the
//   Denali office view with its members, setting this as its command receiver.
// begbert 11-09-2010 1-34979 : Altered to use combo files.
bool CCMSMDIMainFrame::CreateDenaliOfficeView(const CString& szCombo, const CString& szComboFund /*=T("")*/)
{
	m_szDenaliOfficeViewXMLFile = szCombo;
	m_szDenaliOfficeViewXMLFileFund = szComboFund.IsEmpty() ? szCombo : szComboFund;
	return ShowDenaliOfficeView();
}

// Sets it to show/hide based on the current settings in controller module preferences.
bool CCMSMDIMainFrame::ShowDenaliOfficeView()
{
	bool bShow = false;
	switch (si->ApplicationSettings.ModuleID)
	{
	case IDS_AP_MODULE: bShow = !si->ModuleSettings.CTSettings.UseAPClassic; break;
	case IDS_AR_MODULE: bShow = !si->ModuleSettings.CTSettings.UseARClassic; break;
	case IDS_BR_MODULE: bShow = !si->ModuleSettings.CTSettings.UseBRClassic; break;
	case IDS_CT_MODULE: bShow = !si->ModuleSettings.CTSettings.UseCTClassic; break;
	case IDS_GL_MODULE: bShow = !si->ModuleSettings.CTSettings.UseGLClassic; break;
	case IDS_IN_MODULE: bShow = !si->ModuleSettings.CTSettings.UseINClassic; break;
		//	PBI 29370	04/17/2015	mvt - Set up Job Cost integration
	case IDS_JC_MODULE:
		if (!CMS::GetSI()->CompanySettings.IsFund)
			bShow = !si->ModuleSettings.CTSettings.UseJCClassic;
		break;
	case IDS_PO_MODULE: bShow = !si->ModuleSettings.CTSettings.UsePOClassic; break;
	case IDS_PR_MODULE: bShow = !si->ModuleSettings.CTSettings.UsePRClassic; break;
	case IDS_SA_MODULE: bShow = !si->ModuleSettings.CTSettings.UseSAClassic; break;
		//	case IDS_SAE_MODULE:
	}

	return ShowDenaliOfficeView(bShow);
}

HMODULE CCMSMDIMainFrame::GetDotNetDLL()
{
	if (m_hDotNetDLL == NULL)
		m_hDotNetDLL = AfxLoadLibrary(_T("CMSDotNet.dll"));
	return m_hDotNetDLL;
}

bool CCMSMDIMainFrame::ShowDenaliOfficeView(bool bShow)
{
	HMODULE hDotNetDLL = GetDotNetDLL();
	if (hDotNetDLL == NULL) return false;

	GETDLLFUNC(GetDenaliOfficeViewTemplate, hDotNetDLL);
	if (GetDenaliOfficeViewTemplate == NULL) return false;

	// begbert 06-02-2011 1-35942 : in PO, only use the fund file if you're in fund AND the encumbrance is on.
	bool bUseFundFile = CMS::GetSI()->CompanySettings.IsFund;
	if (CMS::GetSI()->ApplicationSettings.ModuleStringID == _T("PO") &&
		!CMS::GetSI()->ModuleSettings.POSettings.bPostEncumber)
		bUseFundFile = false;
	CString szDUIXMLFile = bUseFundFile ? m_szDenaliOfficeViewXMLFileFund : m_szDenaliOfficeViewXMLFile;

	bool bExists = m_pDenaliOfficeView != NULL;
	bool bDoLoad = !(bExists && szDUIXMLFile == m_szDenaliOfficeViewXMLLoadedFile);
	bool bSuccess = true;

	if (szDUIXMLFile.IsEmpty()) bShow = false; // Only show it if we have a valid config file for it.

	if (bShow == false)
	{	// If it exists, hide it.
		if (bExists)
			m_pDenaliOfficeView->GetParentFrame()->ShowWindow(SW_HIDE);
		return true;
	}

	// It already exists and has the right config file loaded: simply show it.
	if (bExists && !bDoLoad)
	{
		m_pDenaliOfficeView->GetParentFrame()->ShowWindow(SW_SHOW);
		return true;
	}

	if (!bExists)
	{
		// Create the navigator pane
		Invalidate();	// repaint the background before starting, as this seizes control of the app for a moment.

		CCMSWaitCursor clsCursor;

		CDocTemplate* pDocTemplate = GetDenaliOfficeViewTemplate();
		CDocument* pDoc = pDocTemplate->OpenDocumentFile(NULL, false);
		if (pDoc == NULL) return false;	// doc creation failure

		pDoc->UpdateAllViews(NULL);
		POSITION posView = pDoc->GetFirstViewPosition();
		if (posView == NULL) return false;	// view acquisition failure

		m_pDenaliOfficeView = pDoc->GetNextView(posView);

		// move the office view to the back
		m_pDenaliOfficeView->GetParentFrame()->SetWindowPos(&CWnd::wndBottom, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

		m_pDenaliOfficeView->SendMessage(WM_DUI_SETCOMMANDRECEIVER, (WPARAM)(CCommandReceiver*)this, true);

		// Size the control to fill window
		RecalcLayout();

		if (s_bIsDestroyed) return false;	// begbert 12-02-2010 1-35197 : Check whether the window has been destroyed.
	}

	if (bDoLoad)
	{
		m_pDenaliOfficeView->SendMessage(WM_DUI_SETMODULENAME, (WPARAM)(LPCTSTR)si->ApplicationSettings.ModuleName);
		bSuccess = !!m_pDenaliOfficeView->SendMessage(WM_DUI_LOADCONTROLFILEFORWAITSTATE, (WPARAM)(LPCTSTR)szDUIXMLFile);

		if (s_bIsDestroyed) return false;	// begbert 12-02-2010 1-35197 : Check whether the window has been destroyed.

		if (bSuccess)
		{
			m_pDenaliOfficeView->GetParentFrame()->ShowWindow(SW_SHOW);

			m_pDenaliOfficeView->SendMessage(WM_DUI_SUSPENDSECURITYUPDATE);
			UpdateNavigatorIntegrationInformation();
			UpdateNavigatorCompanyUserIDInformation();
			UpdateNavigatorSecurityXML();
			UpdateModulePreferenceFlags();

			// begbert 01-11-2011 1-35253 : disable the X button during the load, to prevent the program from closing out from under it.
			CMenu* pSM = GetSystemMenu(FALSE);
			if (pSM) pSM->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			m_bDisableExit = true;

			bSuccess = !!m_pDenaliOfficeView->SendMessage(WM_DUI_LOADCONTROLFILE, (WPARAM)(LPCTSTR)szDUIXMLFile);

			// begbert 01-11-2011 1-35253 : re-enable the X button.
			if (pSM) pSM->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND);
			m_bDisableExit = false;

			if (s_bIsDestroyed) return false;	// begbert 12-02-2010 1-35197 : Check whether the window has been destroyed.

			m_pDenaliOfficeView->SendMessage(WM_DUI_RESUMESECURITYUPDATE);

			m_szDenaliOfficeViewXMLLoadedFile = szDUIXMLFile;
		}
	}

	// There was a problem loading the DUI; we won't be showing it.
	if (!bSuccess)
	{
		if (s_bIsDestroyed) return false;	// begbert 12-02-2010 1-35197 : Check whether the window has been destroyed.

		m_pDenaliOfficeView->GetParentFrame()->ShowWindow(SW_HIDE);
		m_pDenaliOfficeView->GetParentFrame()->SendMessage(WM_CLOSE);
		m_pDenaliOfficeView = NULL;
	}

	return true;
}

// begbert 11-12-2010 1-35115 : Handle Navigator notifications from elsewhere in the program.
void CCMSMDIMainFrame::NotifyNavigatorInterface(enuNavigatorNotifications eNotification)
{
	if (m_pDenaliOfficeView == NULL) return;

	switch (eNotification)
	{
	case eIntegrationChanged:		UpdateNavigatorIntegrationInformation(); break;
	case eCompanyUserIDChanged:		UpdateNavigatorCompanyUserIDInformation(); break;
	case eShowHideNavigator:		ShowDenaliOfficeView(); break;
	case eSecurityXMLChanged:		UpdateNavigatorSecurityXML(); break;
	case eModulePreferenceFlags:	UpdateModulePreferenceFlags(); break;
	case eSuspendSecurityUpdate:	m_pDenaliOfficeView->SendMessage(WM_DUI_SUSPENDSECURITYUPDATE); break;
	case eResumeSecurityUpdate:		m_pDenaliOfficeView->SendMessage(WM_DUI_RESUMESECURITYUPDATE); break;
	}
}

// begbert 11-11-2010 1-35037 : added this function
void CCMSMDIMainFrame::UpdateNavigatorCompanyUserIDInformation()
{
	if (m_pDenaliOfficeView == NULL) return;

	m_pDenaliOfficeView->SendMessage(WM_DUI_SUSPENDSECURITYUPDATE);
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETCOMPANYID, (WPARAM)(LPCTSTR)si->CompanySettings.ID);
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETISSUPERVISOR, (WPARAM)(si->UserInformation.ID == CGBLResources::GetResourceString(IDS_SUP)));
	m_pDenaliOfficeView->SendMessage(WM_DUI_RESUMESECURITYUPDATE);
}

void CCMSMDIMainFrame::UpdateNavigatorSecurityXML()
{
	if (m_pDenaliOfficeView == NULL) return;

	m_pDenaliOfficeView->SendMessage(WM_DUI_SETSECURITY, (WPARAM)(LPCTSTR)si->UserRight.GetSecurityXML());
}

void CCMSMDIMainFrame::UpdateModulePreferenceFlags()
{
	if (m_pDenaliOfficeView == NULL) return;

	bool bLayawayAvailable = si->ModulesInstalled.Layaway.IsUsable && (si->ModuleSettings.PSSettings.ActivateLayaway || si->ModuleSettings.OESettings.ActivateLayaway);
	bool bGiftCertificateAvailable = si->ModuleSettings.OESettings.ActivateGiftCertificate || si->ModuleSettings.PSSettings.ActivateGiftCertificate;

	m_pDenaliOfficeView->SendMessage(WM_DUI_SUSPENDSECURITYUPDATE);
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETISINTERFACECODESACTIVATED, (WPARAM)si->ApplicationSettings.InterfaceKeys);
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETISLAYAWAYACTIVATED, (WPARAM)bLayawayAvailable);
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETISGIFTCARDSACTIVATED, (WPARAM)bGiftCertificateAvailable);
	m_pDenaliOfficeView->SendMessage(WM_DUI_RESUMESECURITYUPDATE);
}

void CCMSMDIMainFrame::AddIntegrationTag(CXMLElement& eleRoot, CModuleSettings* pSettings, EIntegrationTypes eIntType, const CString& szInsert /*=EMPTY_STRING*/)
{
	CGBLSystemInformation* si = CMS::GetSI();
	int i;

	//	e_GL, e_GLSA, e_IN, e_INJC, e_INSA, e_AP, e_APLookup, e_APSA, e_AR, e_ARSA, e_BR, e_BRAR, e_JC, e_JCSA, e_SS, e_MLI

	CString sztag[] = {
		_T("strGLIntegration"), _T("strGLIntegration"),
		_T("strINIntegration"), _T("strINIntegration"), _T("strINIntegration"),
		_T("strAPIntegration"), _T("strAPIntegration"), _T("strAPIntegration"),
		_T("strARIntegration"), _T("strARIntegration"),
		_T("strBRIntegration"), _T("strBRIntegration"),
		_T("strJCIntegration"), _T("strJCIntegration"),
		_T("strSSIntegration"),
		_T("strMLIIntegration")
	};

	bool bUsable[] = {
		true, true,	// General Ledger doesn't mind if the module isn't usable; it could be in report only mode.
		si->ModulesInstalled.Inventory.IsUsable, si->ModulesInstalled.Inventory.IsUsable, si->ModulesInstalled.Inventory.IsUsable,
		si->ModulesInstalled.AccountsPayable.IsUsable, si->ModulesInstalled.AccountsPayable.IsUsable, si->ModulesInstalled.AccountsPayable.IsUsable,
		si->ModulesInstalled.AccountsReceivable.IsUsable, si->ModulesInstalled.AccountsReceivable.IsUsable,
		si->ModulesInstalled.BankReconciliation.IsUsable, si->ModulesInstalled.BankReconciliation.IsUsable,
		si->ModulesInstalled.JobCost.IsUsable, si->ModulesInstalled.JobCost.IsUsable,
		si->ModulesInstalled.SpecialtyShop.IsUsable,
		si->ModulesInstalled.MultiLocationInventory.IsUsable
	};

	short intLevel[] = {
		pSettings->GLIntegration, pSettings->GLIntegration,
		pSettings->INIntegration, pSettings->INIntegration, pSettings->INIntegration,
		pSettings->APIntegration, pSettings->APIntegration, pSettings->APIntegration,
		pSettings->ARIntegration, pSettings->ARIntegration,
		pSettings->BRIntegration, pSettings->BRIntegration,
		pSettings->JCIntegration, pSettings->JCIntegration,
		1,
		1	// SS and MLI are level 1 if they're Usable
	};

	short intNumCodes[17] =
	{
		12, 12,		// GL
		5, 5, 5,	// IN
		3, 3, 3,	// AP
		3, 3,		// AR
		3, 5,		// BR
		3, 3,		// JC
		2,			// SS
		2			// MLI
	};

	short intCodes[17][12] = {
		{ 0, 1, 2, 3, 4, 10, 11, 12, 101, 102, 103, 104 },	// e_GL
	{ 0, 1, 2, 3, 4, 10, 11, 12, 101, 102, 103, 104 },	// e_GLSA
	{ 0, 1, 2, 101, 102 },								// e_IN
	{ 0, 1, 2, 101, 102 },								// e_INJC
	{ 0, 1, 2, 101, 102 },								// e_INSA
	{ 0, 1, 101 },										// e_AP
	{ 0, 1, 101 },										// e_APLookup
	{ 0, 1, 101 },										// e_APSA
	{ 0, 1, 101 },										// e_AR
	{ 0, 1, 101 },										// e_ARSA
	{ 0, 1, 101 },										// e_BR
	{ 0, 1, 2, 101, 102 },								// e_BRAR
	{ 0, 1, 101 },										// e_JC
	{ 0, 1, 101 },										// e_JCSA
	{ 0, 1 },											// e_SS
	{ 0, 1 }											// e_MLI
	};

	CString szDecode[17][12] = {
		{ // e_GL
			RESSTRING(IDS_INTEGRATION_OFF),						// "Integration OFF"
			RESSTRING(IDS_INTEGRATION_SUMMARY),					// "Integration Summary"
			RESSTRING(IDS_INTEGRATION_DETAIL),					// "Integration Detail"
			RESSTRING(IDS_INTEGRATION_CONDENSED),				// "Integration Condensed"
			RESSTRING(IDS_INTEGRATION_FULL_DETAIL),				// "Integration Full Detail"
			RESSTRING(IDS_INTEGRATED_REPORTONLY_SUM),			// "Report Only Summary"
			RESSTRING(IDS_INTEGRATED_REPORTONLY_DET),			// "Report Only Detail"
			RESSTRING(IDS_GL_ROFULLDTL),						// "Report Only - Full Detail"
			RESSTRING(IDS_INTEGRATED_SUMMARY_FAILED),			// "Integrated Summary Failed"
			RESSTRING(IDS_INTEGRATED_DETAIL_FAILED),			// "Integrated Detail Failed"
			RESSTRING(IDS_INTEGRATED_CONDENSED_FAILED),			// "Integrated Condensed Failed"
			RESSTRING(IDS_INTEGRATED_FULL_DETAIL_FAILED) },		// "Integrated Full Detail Failed"
		{ // e_GLSA
			RESSTRING(IDS_OFF_CAPS),							// "OFF"
			RESSTRING(IDS_ON_SUMMARY),							// "ON-Summary"
			RESSTRING(IDS_ON_DETAIL),							// "ON-Detail"
			RESSTRING(IDS_ON_CONDENSED),						// "ON-Condensed"
			RESSTRING(IDS_ON_DETAIL),							// "ON-Detail"
			RESSTRING(IDS_REPORT_ONLY_S),						// "Report Only-S"
			RESSTRING(IDS_REPORT_ONLY_D),						// "Report Only-D"
			RESSTRING(IDS_REPORT_ONLY_D),						// "Report Only-D"
			RESSTRING(IDS_FAILED),								// "Failed"
			RESSTRING(IDS_FAILED),								// "Failed"
			RESSTRING(IDS_FAILED),								// "Failed"
			RESSTRING(IDS_FAILED) },							// "Failed"

		{ // e_IN
			RESSTRING(IDS_INTEGRATION_OFF),						// "Integration OFF"
			RESSTRING(IDS_INTEGRATED),							// "Integrated"
			RESSTRING(IDS_INTEGRATED),							// "Integrated"
			RESSTRING(IDS_INTEGRATION_FAILED),					// "Integration Failed"
			RESSTRING(IDS_INTEGRATION_FAILED) },				// "Integration Failed"
		{ // e_INJC
			RESSTRING(IDS_INTEGRATION_OFF),						// "Integration OFF"
			RESSTRING(IDS_BATCHMODE),							// "Batch Mode"
			RESSTRING(IDS_INV_IMMEDIATE_COMMIT),				// "Immediate - Commit Inventory"
			RESSTRING(IDS_BATCHMODE_INT_FAILED),				// "Batch Mode - Integration Failed"
			RESSTRING(IDS_INV_IMMEDIATE_COMMIT_INT_FAILED) },	// "Immediate - Commit Inventory - Integration Failed"
		{ // e_INSA
			RESSTRING(IDS_OFF_CAPS),							// "OFF"
			RESSTRING(IDS_ON_BATCH),							// "ON-Batch"
			RESSTRING(IDS_ON_IMMEDIATE),						// "ON-Immediate"
			RESSTRING(IDS_FAILED),								// "Failed"
			RESSTRING(IDS_FAILED) },							// "Failed"

		{ // e_AP
			RESSTRING(IDS_INTEGRATION_OFF),						// "Integration OFF"
			RESSTRING(IDS_INTEGRATED),							// "Integrated"
			RESSTRING(IDS_INTEGRATION_FAILED) },				// "Integration Failed"
		{ // e_APLookup
			RESSTRING(IDS_LOOKUP_DISABLED),						// "Lookup Disabled"
			RESSTRING(IDS_LOOKUP_ENABLED),						// "Lookup Enabled"
			RESSTRING(IDS_LOOKUP_ENABLED_INTEGRATION_FAIL) },	// "Lookup Enabled - Integration Fail"
		{ // e_APSA
			RESSTRING(IDS_OFF_CAPS),							// "OFF"
			RESSTRING(IDS_ON_CAPS),								// "ON"
			RESSTRING(IDS_FAILED) },							// "Failed"

		{ // e_AR
			RESSTRING(IDS_INTEGRATION_OFF),						// "Integration OFF"
			RESSTRING(IDS_INTEGRATED),							// "Integrated"
			RESSTRING(IDS_INTEGRATION_FAILED) },				// "Integration Failed"
		{ // e_ARSA
			RESSTRING(IDS_OFF_CAPS),							// "OFF"
			RESSTRING(IDS_ON_CAPS),								// "ON"
			RESSTRING(IDS_FAILED) },							// "Failed"

		{ // e_BR
			RESSTRING(IDS_INTEGRATION_OFF),						// "Integration OFF"
			RESSTRING(IDS_INTEGRATED),							// "Integrated"
			RESSTRING(IDS_INTEGRATION_FAILED) },				// "Integration Failed"
		{ // e_BRAR
			RESSTRING(IDS_INTEGRATION_OFF),						// "Integration OFF"
			RESSTRING(IDS_INTEGRATED_SUMMARY),					// "Integrated Summary"
			RESSTRING(IDS_INTEGRATED_DEATIL),					// "Integrated Detail"
			RESSTRING(IDS_LBL_INTEGRATED_SUMMARY_INT_FAILED),	// "Integrated Summary - Integration Failed"
			RESSTRING(IDS_LBL_INTEGRATED_DETAIL_INT_FAILED) },	// "Integrated Detail - Integration Failed"

		{ // e_JC
			RESSTRING(IDS_INTEGRATION_OFF),						// "Integration OFF"
			RESSTRING(IDS_INTEGRATED),							// "Integrated"
			RESSTRING(IDS_INTEGRATION_FAILED) },				// "Integration Failed"
		{ // e_JCSA
			RESSTRING(IDS_OFF_CAPS),							// "OFF"
			RESSTRING(IDS_ON_CAPS),								// "ON"
			RESSTRING(IDS_FAILED) },							// "Failed"

		{ // e_SS
			RESSTRING(IDS_NOT_INSTALLED),						// "Not Installed"
			RESSTRING(IDS_INSTALLED) },							// "Installed"

		{ // e_MLI
			RESSTRING(IDS_NOT_INSTALLED),						// "Not Installed"
			RESSTRING(IDS_INSTALLED) }							// "Installed"
	};

	// not usable == not integrated
	if (!bUsable[eIntType])
		intLevel[eIntType] = 0;

	// find the matching code, decode it into the xml, and return.
	for (i = 0; i < intNumCodes[eIntType]; i++)
		if (intCodes[eIntType][i] == intLevel[eIntType])
		{
			CString szFixedTag = sztag[eIntType];
			szFixedTag.Insert(5, szInsert);
			eleRoot.AddNewChild(szFixedTag, szDecode[eIntType][i]);
			return;
		}

	// It shouldn't get here; the integration code was unknown; send nothing.
	ASSERT(FALSE);
	return;
};

void CCMSMDIMainFrame::UpdateNavigatorIntegrationInformation()
{
	if (m_pDenaliOfficeView == NULL) return;

	CXMLDocument xmlDoc(_T("Integration"));
	CXMLElement	 eleRoot = xmlDoc.GetDocumentElement();
	CModuleSettings* pSettings = g_pGBLSystemInformationGBLForms->ModuleSettings.CurrentSettings;

	//	e_GL, e_IN, e_INSAJC, e_AP, e_APPO, e_APLookup, e_AR, e_BR, e_BRAR, e_JC, e_SS, e_MLI

	switch (g_pGBLSystemInformationGBLForms->ApplicationSettings.ModuleID)
	{
	case IDS_CT_MODULE:
		eleRoot.AddAttribute(_T("CurrentModule"), RESSTRING(IDS_MODULE_CT));
		break;
	case IDS_GL_MODULE:
		eleRoot.AddAttribute(_T("CurrentModule"), RESSTRING(IDS_MODULE_GL));
		break;
	case IDS_IN_MODULE:
		eleRoot.AddAttribute(_T("CurrentModule"), RESSTRING(IDS_MODULE_IN));
		AddIntegrationTag(eleRoot, pSettings, e_MLI);
		AddIntegrationTag(eleRoot, pSettings, e_SS);
		AddIntegrationTag(eleRoot, pSettings, e_GL);
		AddIntegrationTag(eleRoot, pSettings, e_APLookup);
		//		AddIntegrationTag(eleRoot, pSettings, e_ECommerce);	// Hide until e-commerce is ready
		break;
	case IDS_AR_MODULE:
		eleRoot.AddAttribute(_T("CurrentModule"), RESSTRING(IDS_MODULE_AR));
		AddIntegrationTag(eleRoot, pSettings, e_GL);
		AddIntegrationTag(eleRoot, pSettings, e_BRAR);
		//		AddIntegrationTag(eleRoot, pSettings, e_ECommerce);	// Hide until e-commerce is ready
		break;
	case IDS_AP_MODULE:
		eleRoot.AddAttribute(_T("CurrentModule"), RESSTRING(IDS_MODULE_AP));
		AddIntegrationTag(eleRoot, pSettings, e_GL);
		AddIntegrationTag(eleRoot, pSettings, e_BR);
		AddIntegrationTag(eleRoot, pSettings, e_IN);
		//AddIntegrationTag(eleRoot, pSettings, e_JC);	// Hide mention of JC prior to its release
		break;
	case IDS_SA_MODULE:
	case IDS_SAE_MODULE:
		eleRoot.AddAttribute(_T("CurrentModule"), RESSTRING(IDS_MODULE_SA));
		AddIntegrationTag(eleRoot, pSettings, e_SS);

		pSettings = g_pGBLSystemInformationGBLForms->ModuleSettings.CurrentSettings;

		if (g_pGBLSystemInformationGBLForms->ModulesInstalled.PointOfSale.IsUsable)
		{
			pSettings = &g_pGBLSystemInformationGBLForms->ModuleSettings.PSSettings;

			AddIntegrationTag(eleRoot, pSettings, e_GLSA, RESSTRING(IDS_PS_MODULE));
			AddIntegrationTag(eleRoot, pSettings, e_ARSA, RESSTRING(IDS_PS_MODULE));
			AddIntegrationTag(eleRoot, pSettings, e_INSA, RESSTRING(IDS_PS_MODULE));
			AddIntegrationTag(eleRoot, pSettings, e_APSA, RESSTRING(IDS_PS_MODULE));
			//AddIntegrationTag(eleRoot, pSettings, e_JCSA, RESSTRING(IDS_PS_MODULE) );
		}

		if (g_pGBLSystemInformationGBLForms->ModulesInstalled.OrderEntry.IsUsable)
		{
			pSettings = &g_pGBLSystemInformationGBLForms->ModuleSettings.OESettings;

			AddIntegrationTag(eleRoot, pSettings, e_GLSA, RESSTRING(IDS_OE_MODULE));
			AddIntegrationTag(eleRoot, pSettings, e_ARSA, RESSTRING(IDS_OE_MODULE));
			AddIntegrationTag(eleRoot, pSettings, e_INSA, RESSTRING(IDS_OE_MODULE));
			AddIntegrationTag(eleRoot, pSettings, e_APSA, RESSTRING(IDS_OE_MODULE));
			//AddIntegrationTag(eleRoot, pSettings, e_JCSA, RESSTRING(IDS_OE_MODULE) );
		}
		break;
	case IDS_PR_MODULE:
		eleRoot.AddAttribute(_T("CurrentModule"), RESSTRING(IDS_MODULE_PR));
		break;
	case IDS_POE_MODULE:	// begbert 07-17-2007 : added POE as a PO module
	case IDS_PO_MODULE:
		eleRoot.AddAttribute(_T("CurrentModule"), RESSTRING(IDS_MODULE_PO));
		AddIntegrationTag(eleRoot, pSettings, e_GL);
		AddIntegrationTag(eleRoot, pSettings, e_IN);
		AddIntegrationTag(eleRoot, pSettings, e_AP);
		//	PBI 29370	04/17/2015	mvt - Set up Job Cost integration
		if (!CMS::GetSI()->CompanySettings.IsFund)
			AddIntegrationTag(eleRoot, pSettings, e_JC);
		break;
	case IDS_JC_MODULE:
		eleRoot.AddAttribute(_T("CurrentModule"), RESSTRING(IDS_MODULE_JC));
		break;
	case IDS_BR_MODULE:
		eleRoot.AddAttribute(_T("CurrentModule"), RESSTRING(IDS_MODULE_BR));
		AddIntegrationTag(eleRoot, pSettings, e_SS);
		AddIntegrationTag(eleRoot, pSettings, e_GL);
		break;
	}

	CString szModuleIntegration = xmlDoc.GetOuterXML();
	xmlDoc.Dispose();

	m_pDenaliOfficeView->SendMessage(WM_DUI_SUSPENDSECURITYUPDATE);
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETISMLIACTIVATED, (WPARAM)si->ModulesInstalled.MultiLocationInventory.IsUsable);
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETISSSACTIVATED, (WPARAM)si->ModulesInstalled.SpecialtyShop.IsUsable);
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETYODLEEUSABLE, (WPARAM)si->ApplicationSettings.YodleeUsable);
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETARCBILLUSABLE, (WPARAM)si->ApplicationSettings.ArcBillUsable);
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETISAPINTEGRATED, (WPARAM)(si->ModuleSettings.CurrentSettings->APIntegration > 0));
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETISARINTEGRATED, (WPARAM)(si->ModuleSettings.CurrentSettings->ARIntegration > 0));
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETISBRINTEGRATED, (WPARAM)(si->ModuleSettings.CurrentSettings->BRIntegration > 0));
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETISGLINTEGRATED, (WPARAM)(si->ModuleSettings.CurrentSettings->GLIntegration > 0));
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETISININTEGRATED, (WPARAM)(si->ModuleSettings.CurrentSettings->INIntegration > 0));
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETISJCINTEGRATED, (WPARAM)(si->ModuleSettings.CurrentSettings->JCIntegration > 0));
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETISOEINTEGRATED, (WPARAM)(si->ModuleSettings.CurrentSettings->SAIntegration > 0 && si->ModulesInstalled.OrderEntry.IsUsable));
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETISPOINTEGRATED, (WPARAM)(si->ModuleSettings.CurrentSettings->POIntegration > 0));
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETISPRINTEGRATED, (WPARAM)(si->ModuleSettings.CurrentSettings->PRIntegration > 0));
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETISPSINTEGRATED, (WPARAM)(si->ModuleSettings.CurrentSettings->SAIntegration > 0 && si->ModulesInstalled.PointOfSale.IsUsable));
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETISSAINTEGRATED, (WPARAM)(si->ModuleSettings.CurrentSettings->SAIntegration > 0));
	m_pDenaliOfficeView->SendMessage(WM_DUI_SETINTEGRATION, (WPARAM)(LPCTSTR)szModuleIntegration);
	m_pDenaliOfficeView->SendMessage(WM_DUI_RESUMESECURITYUPDATE);
}

// begbert 07-20-2010 3.0 Interface : This is the command gateway, which handles all the
//   messages and commands sent back by the Denali Interface and opens the desired windows
//   or does whatever other functions may be necessary.
//
// Returns true if the command was handled, in which case no further processing is needed.
bool CCMSMDIMainFrame::CommandGateway(const CCommand& command)
{
	if (!m_pDenaliOfficeView) return false;

	bool bToRet = true;

	if (command.m_eType == command.eHELP)
	{
		// begbert 11-08-2010 1-35035 : massage the help file information to
		//   meet CGBLHelp::ShowHelpTopic()'s requirements.
		CString szHelpFile = command.m_szHelpFile;
		CString szHelpTopic = command.m_szHelpTopic;
		CString szHelpTopic_lower = szHelpTopic;
		szHelpTopic_lower.MakeLower();

		CString szhtm = _T(".htm");

		//02/21/2019 CJB - PBI 47292 - Changed Help to access Web Help.
		if (szHelpTopic != _T("") && szHelpTopic_lower.Right(szhtm.GetLength()) != szhtm)
			szHelpTopic = szHelpTopic + szhtm;

		CGBLHelp::ShowHelpByTopic(CMS::GetSI(), szHelpTopic, szHelpFile);
		return true;
	}

	if (command.m_eType == command.eREQUESTDATA)
	{
		/*
				// This code does it all in the single thread - sometimes useful when debugging and stepping through.
				CCommand nonconstcommand = command;
				HandleNavigatorDataRequest(nonconstcommand);
				m_pDenaliOfficeView->SendMessage(WM_DUI_UPDATESERVERDATA, (WPARAM) &nonconstcommand);
				return true;
		/*/
		// the new CNavigatorDataRequest object is deleted in OnNavigatorDataRequestReturn().
		return AfxBeginThread(::HandleNavigatorDataRequest, new CNavigatorDataRequest(this, command)) != NULL;
		//*/
	}

	// By this point, all types besides eCOMMAND should have been handled.
	if (command.m_eType != command.eCOMMAND) { ASSERT(FALSE); return false; }

	if (!HasGatewayRights(command)) return true;

	CString szCommand = command.m_szCommand;
	int nSpace = szCommand.Find(' ');
	if (nSpace != -1) szCommand = szCommand.Left(nSpace);

	if (szCommand.Left(2) == "AP")
	{
		if (szCommand == "AP00000") OnNav_OpenAccountsPayableModule();
		else if (szCommand == "AP00001") OnNav_APVendorMaintenance();
		else if (szCommand == "AP00002") OnNav_APCopyVendors();
		else if (szCommand == "AP00003") OnNav_APRenumberVendors();
		else if (szCommand == "AP00004") OnNav_APDefaultVendorTemplate();
		else if (szCommand == "AP00005") OnNav_APImportExport();
		else if (szCommand == "AP00006") OnNav_APEnterBills();
		else if (szCommand == "AP00007") OnNav_APGenerateBillsfromRecurring();
		else if (szCommand == "AP00008") OnNav_APGenerateBillsfromExternal();
		else if (szCommand == "AP00009") OnNav_APEnterBillsElectronicsFundsTransfer();
		else if (szCommand == "AP00010") OnNav_APImmediateCheckRegister();
		else if (szCommand == "AP00011") OnNav_APPostBills();
		else if (szCommand == "AP00012") OnNav_APClearBills();
		else if (szCommand == "AP00013") OnNav_APEnterBillsChangeLocalTransactionDate();
		else if (szCommand == "AP00014") OnNav_APPreparePaymentsAutomatically();
		else if (szCommand == "AP00015") OnNav_APAdjustPayBills();
		else if (szCommand == "AP00016") OnNav_APPayBillsElectronicFundsTransfer();
		else if (szCommand == "AP00017") OnNav_APPostPayments();
		else if (szCommand == "AP00018") OnNav_APClearPaymentsandAdjustments();
		else if (szCommand == "AP00019") OnNav_APPayBillsChangeLocalTransactionDate();
		else if (szCommand == "AP00020") OnNav_APProcessPrepaidsandFutures();
		else if (szCommand == "AP00021") OnNav_APPurgeOpenItems();
		else if (szCommand == "AP00022") OnNav_APPurgeHistory();
		else if (szCommand == "AP00023") OnNav_APResetYTDTotals();
		else if (szCommand == "AP00024") OnNav_APRemoveTemporaryVendors();
		else if (szCommand == "AP00025") OnNav_APModulePreferences();
		else if (szCommand == "AP00026") OnNav_APEditCodes();
		else if (szCommand == "AP00027") OnNav_APAllocationCodes();
		else if (szCommand == "AP00028") OnNav_APFrequencyCodes();
		else if (szCommand == "AP00029") OnNav_APSetUpRecurringBills();
		else if (szCommand == "AP00030") OnNav_APDisplayTableInformation();
		else if (szCommand == "AP00031") OnNav_APRecalculateVendorBalances();
		else if (szCommand == "AP00032") OnNav_APAdjustPostedItemDueDate();
		else if (szCommand == "AP00033") OnNav_APEnterBillsSelectBatch();
		else if (szCommand == "AP00034") OnNav_APEnterBillsSetUpBatch();
		else if (szCommand == "AP00035") OnNav_APPayBillsSelectBatch();
		else if (szCommand == "AP00036") OnNav_APPayBillsSetUpBatch();
		else if (szCommand == "AP00037") OnNav_APEnterBillsPrintEditReport();
		else if (szCommand == "AP00038") OnNav_APEnterBillsPrintInventoryLabels();
		else if (szCommand == "AP00039") OnNav_APEnterBillsPrintImmediateChecks();
		else if (szCommand == "AP00040") OnNav_APPayBillsPrintEditReport();
		else if (szCommand == "AP00041") OnNav_APPayBillsPrintChecks();
		else if (szCommand == "AP00042") OnNav_APPayBillsCheckRegister();
		else if (szCommand == "AP00043") OnNav_APReportsQuickVendorList();
		else if (szCommand == "AP00044") OnNav_APReportsVendorLabels();
		else if (szCommand == "AP00045") OnNav_APReportsMasterVendor();
		else if (szCommand == "AP00046") OnNav_APReportsAging();
		//		else if (szCommand == "AP00047") OnNav_APReportsTotalsDebt();
		else if (szCommand == "AP00060") OnNav_APReportsDebt();		//	PBI 41382	08/31/2017	mvt - Added debt report
		else if (szCommand == "AP00048") OnNav_APReportsCashRequirements();
		else if (szCommand == "AP00049") OnNav_APReportsOverdue();
		else if (szCommand == "AP00050") OnNav_APReportsPrepaid();
		else if (szCommand == "AP00051") OnNav_APReportsFutureLiability();
		else if (szCommand == "AP00052") OnNav_APReportsDatedInvoicePayment();
		else if (szCommand == "AP00053") OnNav_APReportsGLExpenseDistribution();
		else if (szCommand == "AP00054") OnNav_APReportsHistory();
		else if (szCommand == "AP00055") OnNav_APReports1099();
		else if (szCommand == "AP00056") OnNav_APReportsElectronicFile1099();
		else if (szCommand == "AP00057") OnNav_APReportsControl();
		else if (szCommand == "AP00058") OnNav_APPayBillsPositivePay();
		else if (szCommand == "AP00059") OnNav_APEnterBillsPositivePay();
		else if (szCommand == "AP00061") OnNav_APEnterBillsSendInternalControlAlerts(); // PBI 46406	09/26/2018	mvt - Added security for Internal Control Alerts in AP
		else if (szCommand == "AP00062") OnNav_APPayBillsSendInternalControlAlerts(); // PBI 46406	09/26/2018	mvt - Added security for Internal Control Alerts in AP
		else if (szCommand == "AP00063") OnNav_APManageInternalControlAlerts(); // PBI 46406	09/26/2018	mvt - Added security for Internal Control Alerts in AP
		else if (szCommand == "AP00064") OnNav_APManageInternalControlAlertsEnterBills();	// PBI 46406	09/26/2018	mvt - Added security for Internal Control Alerts in AP
		else if (szCommand == "AP00065") OnNav_APManageInternalControlAlertsPayBills();		// PBI 46406	09/26/2018	mvt - Added security for Internal Control Alerts in AP
		else if (szCommand == "AP00066") OnNav_APGenerateBillsfromExpensify();		//	PBI 49849	06/14/2019	mvt - Added Expensify to navigation
		else if (szCommand == "AP00067") OnNav_APReports1099();	//RCG - 11/25/2020 - PBI 52809 - Added 1099-NEC to navigation
		else if (szCommand == "AP00068") OnNav_APImportExportArcBill();	//RCG - 06/11/2021 - PBI 57746 - Added Import/Export ArcBill

		else
			bToRet = false;
	}

	if (szCommand.Left(2) == "AR")
	{
		if (szCommand == "AR00000") OnNav_OpenAccountsReceivablesModule();
		else if (szCommand == "AR00001") OnNav_ARCustomerMaintenance();
		else if (szCommand == "AR00002") OnNav_ARCopyCustomers();
		else if (szCommand == "AR00003") OnNav_ARRenumberCustomers();
		else if (szCommand == "AR00004") OnNav_ARGenerateTransactionsfromExternal();
		else if (szCommand == "AR00005") OnNav_ARGenerateTransactionsfromRecurring();
		else if (szCommand == "AR00006") OnNav_ARCalculateFinanceCharges();
		else if (szCommand == "AR00007") OnNav_ARChangeLocalTransactionDate();
		else if (szCommand == "AR00008") OnNav_ARPurgeOpenItems();
		else if (szCommand == "AR00009") OnNav_ARResetYTDCustomerTotals();
		else if (szCommand == "AR00010") OnNav_ARResetCreditCardTotals();
		else if (szCommand == "AR00011") OnNav_ARPurgeHistory();
		else if (szCommand == "AR00012") OnNav_ARModulePreferences();
		else if (szCommand == "AR00013") OnNav_AREnterPaymentsandAdjustments();
		else if (szCommand == "AR00014") OnNav_ARPostPaymentsandAdjustments();
		else if (szCommand == "AR00015") OnNav_ARClearTransactions();
		else if (szCommand == "AR00016") OnNav_ARSelectBatch();
		else if (szCommand == "AR00017") OnNav_ARSetUpBatch();
		else if (szCommand == "AR00018") OnNav_AREditCodes();
		else if (szCommand == "AR00019") OnNav_ARTermsCodes();
		else if (szCommand == "AR00020") OnNav_ARTypeCodes();
		else if (szCommand == "AR00021") OnNav_ARUDF1Codes();
		else if (szCommand == "AR00022") OnNav_ARUDF2Codes();
		else if (szCommand == "AR00023") OnNav_ARTransactionCodes();
		else if (szCommand == "AR00024") OnNav_ARClerkCodes();
		else if (szCommand == "AR00025") OnNav_ARFrequencyCodes();
		else if (szCommand == "AR00026") OnNav_ARSetUpRecurringTransactions();
		else if (szCommand == "AR00027") OnNav_ARSalespersonCodes();
		else if (szCommand == "AR00028") OnNav_ARTaxCodes();
		else if (szCommand == "AR00029") OnNav_ARCreditCardCodes();
		else if (szCommand == "AR00030") OnNav_ARDisplayTableInformation();
		else if (szCommand == "AR00031") OnNav_ARRecalculateMasterTable();
		else if (szCommand == "AR00032") OnNav_ARImportExport();
		else if (szCommand == "AR00033") OnNav_ARTransactionPrintEditReport();
		else if (szCommand == "AR00034") OnNav_ARReportsCustomerQuickList();
		else if (szCommand == "AR00035") OnNav_ARReportsCustomerLabels();
		else if (szCommand == "AR00036") OnNav_ARReportsCustomerMaster();
		else if (szCommand == "AR00037") OnNav_ARReportsCustomerStatements();
		else if (szCommand == "AR00038") OnNav_ARReportsCollection();
		else if (szCommand == "AR00039") OnNav_ARReportsAgedAnalysis();
		else if (szCommand == "AR00040") OnNav_ARReportsDatedInvoicePayment();
		else if (szCommand == "AR00041") OnNav_ARReportsHistory();
		else if (szCommand == "AR00042") OnNav_ARReportsControl();
		else if (szCommand == "AR00043") OnNav_ARGenerateElectronicFundsTransferFile();
		else if (szCommand == "AR00044") OnNav_ARCollectionStatusCodes();
		else if (szCommand == "AR00045") OnNav_ARCustomerContactsDepartment();
		else if (szCommand == "AR00046") OnNav_ARCustomerContacts();
		else if (szCommand == "AR00047") OnNav_ARReportsPastDue();	//	PBI 32828	01/25/2016	mvt - Added Past Due report
		else if (szCommand == "AR00048") OnNav_ARCustomerLeasedItem();	//	PBI 33653	06/08/2016	mvt - Added Customer Leased Item report
		else if (szCommand == "AR00049") OnNav_ARPrepaidServices();	//	PBI 34414	06/13/2016	mvt - Added Prepaid Services report
		else if (szCommand == "AR00050") OnNav_ARPrepaidServicesHistory();	//	PBI 35076	06/24/2016	mvt - Added Prepaid Services History report
		else if (szCommand == "AR00051") OnNav_ARImportAccountsFromNeonCRM();	//RCG - 02/28/2018 - PBI 44247 - Added Import Accounts From NeonCRM
		else if (szCommand == "AR00052") OnNav_ARAPIPosting();		//	PBI 49433	06/03/2019	mvt - Added API Posting Internal Control Alert
		else if (szCommand == "AR00053") OnNav_ARReprintPostingAudit(); 	//	PBI 49433	06/04/2019	mvt - Added reprint posting audit report
		else if (szCommand == "AR00054") OnNav_ARGeneratePaymentsFromPayPal(); //RCG - 09/05/2019 - PBI 50961 - Added Generate Payments from PayPal
		else if (szCommand == "AR00055") OnNav_ARAutomatedPaymentCollection(); //	PBI 50961	09/16/2019	mvt - Added AR Automated Payment Collection
		else if (szCommand == "AR00056") OnNav_ARImportExportArcBill();	//RCG - 06/06/2022 - PBI 58628 - Added Import/Export ArcBill
		else if (szCommand == "AR00057") OnNav_ARGenerateTransactionsFromArcBill();	//RS - 06/07/2022 - PBI 59620 - Added Generate Transactions From ArcBill
		else
			bToRet = false;
	}

	if (szCommand.Left(2) == "BR")
	{
		if (szCommand == "BR00000") OnNav_OpenBankReconciliationModule();
		else if (szCommand == "BR00001") OnNav_BRAccountMaintenance();
		else if (szCommand == "BR00002") OnNav_BRBankRegister();
		else if (szCommand == "BR00003") OnNav_BRRebuildBankAccountTotals();
		else if (szCommand == "BR00004") OnNav_BREnterBankActivity();
		else if (szCommand == "BR00005") OnNav_BRVoidBankActivity();
		else if (szCommand == "BR00006") OnNav_BRGenerateAcitivityfromExternal();
		else if (szCommand == "BR00007") OnNav_BRGenerateActivityfromRecurring();
		else if (szCommand == "BR00008") OnNav_BREnterActivityChangeLocalTransactionDate();
		else if (szCommand == "BR00009") OnNav_BRReconcileBankStatement();
		else if (szCommand == "BR00010") OnNav_BRPurgeProcessing();
		else if (szCommand == "BR00011") OnNav_BRModulePreferences();
		else if (szCommand == "BR00012") OnNav_BRPayeeCardFile();
		else if (szCommand == "BR00013") OnNav_BRSetUpRecurringActivity();
		else if (szCommand == "BR00014") OnNav_BRDisplayTableInformation();
		else if (szCommand == "BR00015") OnNav_BRPostActivity();
		else if (szCommand == "BR00016") OnNav_BRSelectActivityBatch();
		else if (szCommand == "BR00017") OnNav_BRSetUpActivityBatch();
		else if (szCommand == "BR00018") OnNav_BRClearActivityBatch();
		else if (szCommand == "BR00019") OnNav_BRPostReconciliatonBatch();
		else if (szCommand == "BR00020") OnNav_BRSelectReconciliationBatch();
		else if (szCommand == "BR00021") OnNav_BRSetUpReconciliationBatch();
		else if (szCommand == "BR00022") OnNav_BRClearReconciliationBatch();
		else if (szCommand == "BR00023") OnNav_BRCategoryCodes();
		else if (szCommand == "BR00024") OnNav_BRFrequencyCodes();
		else if (szCommand == "BR00025") OnNav_BRReconciliationChangeLocalTransactionDate();
		else if (szCommand == "BR00043") OnNav_BREnterActivityPrintEditReport();
		else if (szCommand == "BR00044") OnNav_BREnterActivityPrintChecks();
		else if (szCommand == "BR00045") OnNav_BREnterActivityPrintDepositSlips();
		else if (szCommand == "BR00046") OnNav_BRReconciliationPrintEditReport();
		else if (szCommand == "BR00047") OnNav_BRReportsBankRegister();
		else if (szCommand == "BR00048") OnNav_BRReportsMissingCheck();
		else if (szCommand == "BR00049") OnNav_BRReportsPayeeCard();
		else if (szCommand == "BR00050") OnNav_BRReconciliationReport();
		else if (szCommand == "BR00051") OnNav_BRManageInternalControlAlerts(); // PBI 46406	09/26/2018	mvt - Added security for Internal Control Alerts in BR
		else if (szCommand == "BR00052") OnNav_BRSendInternalControlAlerts(); // PBI 46406	09/27/2018	mvt - Added security for Internal Control Alerts in BR
		else if (szCommand == "BR00053") OnNav_BREnterActivityPositivePay(); //RS - 01/30/2025 - PBI 64592 - Added Positive Pay
		else
			bToRet = false;
	}

	if (szCommand.Left(2) == "CT")
	{
		if (szCommand == "CT00000") OnNav_OpenController();
		else if (szCommand == "CT00001") OnNav_CTSetupSelectaNewCompany();
		else if (szCommand == "CT00002") OnNav_CTInstallModules();
		else if (szCommand == "CT00003") OnNav_CTChangeDate();
		else if (szCommand == "CT00004") OnNav_CTFiscalCalendar();
		else if (szCommand == "CT00005") OnNav_CTElectronicConstants();
		else if (szCommand == "CT00006") OnNav_CTAddChangeUsers();
		else if (szCommand == "CT00007") OnNav_CTAddChangeGroups();
		else if (szCommand == "CT00008") OnNav_CTChangePassword();
		else if (szCommand == "CT00009") OnNav_CTResetPassword();
		else if (szCommand == "CT00010") OnNav_CTCopyUsersFromCompany();
		else if (szCommand == "CT00011") OnNav_CTCopyGroupsFromCompany();
		else if (szCommand == "CT00012") OnNav_CTCurrencyValues();
		else if (szCommand == "CT00013") OnNav_CTExchangeRates();
		else if (szCommand == "CT00014") OnNav_CTDisplayControllerTableInformation();
		else if (szCommand == "CT00015") OnNav_CTProcessPayrollTransactions();
		else if (szCommand == "CT00016") OnNav_CTBackupCompany();
		else if (szCommand == "CT00017") OnNav_CTRestoreCompany();
		else if (szCommand == "CT00018") OnNav_ControllerModulePreferences();
		else if (szCommand == "CT00019") OnNav_CTCopyOrganizationUtility();		// begbert 04-16-2013 PBI 4036 : Added this
		else if (szCommand == "CT00020") OnNav_CTMerchantWareSetup();			// PBI 19692 jhicks 05/12/2014 : Added this
		else if (szCommand == "CT00021") OnNav_CTNeonCRMSetup();
		else if (szCommand == "CT00022") OnNav_CTDenaliBusinessIntelligence();	// PBI 45155 06/13/2018 mvt - Added command
		else if (szCommand == "CT00023") OnNav_CTInternalControlAlertsSetup();	// PBI 46406 09/26/2018 mvt - Added security for Internal Control Alerts in Controller
		else if (szCommand == "CT00024") OnNav_CTYodleeIntegrationSetup();		// PBI 47520 12/04/2018 mvt - Added security for Yodlee integration setup
		else if (szCommand == "CT00025") OnNav_CTPayPalIntegrationSetup();		// PBI 50961 09/04/2019 mvt - Added security for PayPal integration setup
		else if (szCommand == "CT00026") OnNav_CTDonatelyIntegrationSetup();	// PBI 52317 11/19/2019 mvt - Added security for Donately integration
		else if (szCommand == "CT00027") OnNav_CTCloverConnectSetup();			// RS - 04/26/2024 - PBI 63797 - Added user security and menu item to Organization menu
		else
			bToRet = false;
	}

	if (szCommand.Left(2) == "GL")
	{
		if (szCommand == "GL00000") OnNav_OpenGeneralLedgerModule();
		else if (szCommand == "GL00001") OnNav_GLOpenGeneralLedgerMasterForm();
		else if (szCommand == "GL00002") OnNav_GLSetupGLAccountGroups();
		else if (szCommand == "GL00003") OnNav_GLGlobalBudgetUpdate();
		else if (szCommand == "GL00004") OnNav_GLCopyChartofAccounts();
		else if (szCommand == "GL00005") OnNav_GLRenumberAccounts();
		else if (szCommand == "GL00006") OnNav_GLJournalEntry();
		else if (szCommand == "GL00007") OnNav_GLProcessAllocations();
		else if (szCommand == "GL00008") OnNav_GLGenerateJournalEntriesfromRecurring();
		else if (szCommand == "GL00009") OnNav_GLGenerateJournalEntriesfromExternalFile();
		else if (szCommand == "GL00010") OnNav_GLPrintEditReport();
		else if (szCommand == "GL00011") OnNav_GLPostJournalEntries();
		else if (szCommand == "GL00012") OnNav_GLClearJournalEntries();
		else if (szCommand == "GL00013") OnNav_GLChangeLocalTransactionDate();
		else if (szCommand == "GL00014") OnNav_GLPurgeHistory();
		else if (szCommand == "GL00015") OnNav_GLYearEndClose();
		else if (szCommand == "GL00016") OnNav_GLAdjustAccountSegments();
		else if (szCommand == "GL00017") OnNav_GLModulePreferences();
		else if (szCommand == "GL00018") OnNav_GLInterfaceTable();
		else if (szCommand == "GL00019") OnNav_GLFrequencyCodes();
		else if (szCommand == "GL00020") OnNav_GLAllocationCodes();
		else if (szCommand == "GL00021") OnNav_GLAdvancedFilterCodes();
		else if (szCommand == "GL00022") OnNav_GLReprintPosting();
		else if (szCommand == "GL00023") OnNav_GLConsolidateLedger();
		else if (szCommand == "GL00024") OnNav_GLDisplayTableInformation();
		else if (szCommand == "GL00025") OnNav_GLRecalculatePeriodTable();
		else if (szCommand == "GL00026") OnNav_GLImportExport();
		else if (szCommand == "GL00027") OnNav_GLSelectJournalEntryBatch();
		else if (szCommand == "GL00028") OnNav_GLSetUpJournalEntryBatch();
		else if (szCommand == "GL00029") OnNav_GLReportsChartOfAccounts();
		else if (szCommand == "GL00030") OnNav_GLReportsAuditTrail();
		else if (szCommand == "GL00031") OnNav_GLReportsTrialBalance();
		else if (szCommand == "GL00032") OnNav_GLReportsBalanceSheet();
		else if (szCommand == "GL00033") OnNav_GLReportsIncomeStatement();
		else if (szCommand == "GL00034") OnNav_GLReportsCashFlow();
		else if (szCommand == "GL00035") OnNav_GLReportsBudgetPerformance();
		else if (szCommand == "GL00036") OnNav_GLReportsBudgetReports();
		else if (szCommand == "GL00037") OnNav_GLReportsSpreadsheetExport();
		else if (szCommand == "GL00038") OnNav_GLSetUpFunds();					// begbert 03-31-2011 FUND : Added this
		else if (szCommand == "GL00039") OnNav_GLUserDefinedClassifications();	// begbert 03-31-2011 FUND : Added this
		else if (szCommand == "GL00040") OnNav_GLGenerateDueToDueFrom();		// begbert 03-31-2011 FUND : Added this
		else if (szCommand == "GL00041") OnNav_GLRevenueExpenseReport();		// begbert 03-31-2011 FUND : Added this
		else if (szCommand == "GL00042") OnNav_GLStatementofActivities();		// begbert 03-31-2011 FUND : Added this
		else if (szCommand == "GL00043") OnNav_GLStatementofCashFlows();		// begbert 03-31-2011 FUND : Added this
		else if (szCommand == "GL00044") OnNav_GLStatementofFinancialPosition();	// begbert 03-31-2011 FUND : Added this
		else if (szCommand == "GL00045") OnNav_GLCloseFundYear();				// begbert 03-31-2011 FUND : Added this
		else if (szCommand == "GL00046") OnNav_GLAdvancedReportGroupCodes();	// begbert 06/04/2014 PBI 19171 : Added this
		else if (szCommand == "GL00047") OnNav_GLImportDonationsFromNeonCRM();	//RCG - 02/28/2018 - PBI 44247 - Added Import Donations From NeonCRM
		else if (szCommand == "GL00048") OnNav_GLReportsBudgetTracking();		// PBI 47610 03/05/2019 mvt - Added budget tracking report.
		else if (szCommand == "GL00049") OnNav_GLAPIPosting();					// PBI 49433 06/04/2019 mvt - Added API Posting
		else if (szCommand == "GL00050") OnNav_GLReportsStatementOfFunctionalExpenses();	// PBI 51151 08/27/2019 mvt - Added Statement of Functional Expenses
		else if (szCommand == "GL00051") OnNav_GLGenerateJournalEntriesfromFacebook();		// PBI 52439 11/14/2019 mvt - Added import donations from Facebook
		else if (szCommand == "GL00052") OnNav_GLGenerateJournalEntriesfromDonately();		// PBI 52317 11/19/2019 mvt - Added import donations from Donately
		else if (szCommand == "GL00053") OnNav_GLReportsMissingAccounts();		// PBI 51105 01/23/2020 mvt - Added Missing Accounts report.
		else if (szCommand == "GL00054") OnNav_GLBudgetWhatifAnalysis();		// PBI 53765 02/12/2020 mvt - Added WhatIf to navigation
		else if (szCommand == "GL00055") OnNav_GLCostCenters();					// RCG - 01/26/2021 - PBI 56230 - Added Cost Centers
		else if (szCommand == "GL00056") OnNav_GLReportsCostCenterIncomeStatement();	// RCG - 02/02/2021 - PBI 56230 - Added Cost Center
		else if (szCommand == "GL00057") OnNav_GLReportsCostCenterRevenueAndExpense();	// RCG - 03/01/2021 - PBI 56230 - Added Cost Center
		else
			bToRet = false;
	}

	if (szCommand.Left(2) == "IN")
	{
		if (szCommand == "IN00000") OnNav_OpenInventoryModule();
		else if (szCommand == "IN00001") OnNav_INStockItemMaintenance();
		else if (szCommand == "IN00002") OnNav_INSetUpStockAlias();
		else if (szCommand == "IN00003") OnNav_INCopyStockItems();
		else if (szCommand == "IN00004") OnNav_INRenumberStockItems();
		else if (szCommand == "IN00005") OnNav_INGlobalPriceChange();
		else if (szCommand == "IN00006") OnNav_INAdjustStockQuantities();
		else if (szCommand == "IN00007") OnNav_INAdjustmentTransaction();
		else if (szCommand == "IN00008") OnNav_INGenerateAdjustmentsfromScanner();
		else if (szCommand == "IN00009") OnNav_INGenerateAdjustmentsfromExternal();
		else if (szCommand == "IN00010") OnNav_INChangeLocalTransactionDate();
		else if (szCommand == "IN00011") OnNav_INStandardCostVarianceAmounts();
		else if (szCommand == "IN00012") OnNav_INPurgeHistory();
		else if (szCommand == "IN00013") OnNav_INSetUpKits();
		else if (szCommand == "IN00014") OnNav_INModulePreferences();
		else if (szCommand == "IN00015") OnNav_INDisplayTableInformation();
		else if (szCommand == "IN00016") OnNav_INRecalculateInventoryTables();
		else if (szCommand == "IN00017") OnNav_INImportExport();
		else if (szCommand == "IN00018") OnNav_INPostAdjustmentBatch();
		else if (szCommand == "IN00019") OnNav_INSelectAdjustmentBatch();
		else if (szCommand == "IN00020") OnNav_INSetUpAdjustmentBatch();
		else if (szCommand == "IN00021") OnNav_INClearAdjustmentBatch();
		else if (szCommand == "IN00022") OnNav_INInventoryCodes();
		else if (szCommand == "IN00023") OnNav_INTransactionCodes();
		else if (szCommand == "IN00024") OnNav_INMultipackCodes();
		else if (szCommand == "IN00025") OnNav_INPromotionalPricingCodes();
		else if (szCommand == "IN00026") OnNav_INUDF1Codes();
		else if (szCommand == "IN00027") OnNav_INUDF2Codes();
		else if (szCommand == "IN00028") OnNav_INAdjustmentsPrintEditReport();
		else if (szCommand == "IN00029") OnNav_INReportsQuickStockList();
		else if (szCommand == "IN00030") OnNav_INReportsStockLabels();
		else if (szCommand == "IN00031") OnNav_INReportsMasterStock();
		else if (szCommand == "IN00032") OnNav_INReportsPhysicalCountWorksheet();
		else if (szCommand == "IN00033") OnNav_INReportsPriceBook();
		else if (szCommand == "IN00034") OnNav_INReportsOnHand();
		else if (szCommand == "IN00035") OnNav_INReportsStockValue();
		else if (szCommand == "IN00036") OnNav_INReportsUnderstock();
		else if (szCommand == "IN00037") OnNav_INReportsOverstock();
		else if (szCommand == "IN00038") OnNav_INReportsOnOrder();
		else if (szCommand == "IN00039") OnNav_INReportsBackOrder();
		else if (szCommand == "IN00040") OnNav_INReportsHistory();
		else if (szCommand == "IN00041") OnNav_INReportsMovement();
		else if (szCommand == "IN00042") OnNav_INReportsMonthlyMovement();
		else if (szCommand == "IN00043") OnNav_INReportsAdvancedSalesInfo();
		else if (szCommand == "IN00044") OnNav_INReportsPromotionalSales();
		else if (szCommand == "IN00045") OnNav_INReportsControl();
		else if (szCommand == "IN00046") OnNav_INKitAssemblyReport();
		else if (szCommand == "IN00047") OnNav_INMasterKitReport();
		else if (szCommand == "IN00048") OnNav_INKitComponentRequirementReport();
		else if (szCommand == "IN00049") OnNav_INLocationCodes();
		else if (szCommand == "IN00050") OnNav_INUpdateMinMaxQuantities();
		else if (szCommand == "IN00051") OnNav_INBelowMinimumQuantity();
		else if (szCommand == "IN00052") OnNav_INLotExpiration();
		else if (szCommand == "IN00053") OnNav_INAdjustmentsPrintShrinkageReport();
		else if (szCommand == "IN00054") OnNav_INImportExportArcBill();			// RS 06/08/2022 - PBI 59622 - Added Import/Export ArcBill
		else
			bToRet = false;
	}

	if (szCommand.Left(2) == "PO")
	{
		if (szCommand == "PO00000") OnNav_OpenPurchaseOrderModule();
		else if (szCommand == "PO00001") OnNav_POEnterPurchaseOrders();
		else if (szCommand == "PO00002") OnNav_POEditPurchaseOrders();
		else if (szCommand == "PO00003") OnNav_POGeneratePurchaseOrdersfromInventory();
		else if (szCommand == "PO00004") OnNav_POGeneratePurchaseOrdersfromRecurring();
		else if (szCommand == "PO00005") OnNav_POPurchasingChangeLocalTransactionDate();
		else if (szCommand == "PO00006") OnNav_POReceiveItems();
		else if (szCommand == "PO00007") OnNav_POPurgeHistory();
		else if (szCommand == "PO00008") OnNav_POModulePreferences();
		else if (szCommand == "PO00009") OnNav_POSetUpPurchasingReceivingLayout();
		else if (szCommand == "PO00010") OnNav_PODisplayTableInformation();
		else if (szCommand == "PO00011") OnNav_POSetActiveBatchtoOpen();
		else if (szCommand == "PO00012") OnNav_POSetActiveBatchtoRecurring();
		else if (szCommand == "PO00013") OnNav_POPostReceivingBatch();
		else if (szCommand == "PO00014") OnNav_POSelectReceivingBatch();
		else if (szCommand == "PO00015") OnNav_POSetUpReceivingBatch();
		else if (szCommand == "PO00016") OnNav_POClearReceivingTransactions();
		else if (szCommand == "PO00017") OnNav_POBuyerCodes();
		else if (szCommand == "PO00018") OnNav_POCommentCodes();
		else if (szCommand == "PO00019") OnNav_PODepartmentCodes();
		else if (szCommand == "PO00020") OnNav_POFrequencyCodes();
		else if (szCommand == "PO00021") OnNav_POShipViaCodes();
		else if (szCommand == "PO00022") OnNav_POReceivingChangeLocalTransactionDate();
		else if (szCommand == "PO00023") OnNav_POPurchasingPrintPO();
		else if (szCommand == "PO00024") OnNav_POPurchasingPrintRecurringPO();
		else if (szCommand == "PO00025") OnNav_POPrintReceivingTransaction();
		else if (szCommand == "PO00026") OnNav_POReceivingPrintEditReport();
		else if (szCommand == "PO00027") OnNav_POReportsOpenPO();
		else if (szCommand == "PO00028") OnNav_POReportsExpectedItems();
		else if (szCommand == "PO00029") OnNav_POReportsVendorPerformance();
		else if (szCommand == "PO00030") OnNav_POReportsHistory();
		else if (szCommand == "PO00031") OnNav_POReportsReprintPO();
		else if (szCommand == "PO00032") OnNav_POSetUpRecurring();
		else if (szCommand == "PO00033") OnNav_POPreIssuePurchaseOrders();
		else if (szCommand == "PO00034") OnNav_POClearRecurringBatch();
		else if (szCommand == "PO00035") OnNav_POPrintReceivingLabels();		// begbert 11-09-2010 1-35070 : Added this
		else if (szCommand == "PO00036") OnNav_POPurchasingEditReport();		// begbert 03-31-2011 FUND : Added this
		else if (szCommand == "PO00037") OnNav_POPostPurchasingTransactions();	// begbert 03-31-2011 FUND : Added this
		else if (szCommand == "PO00038") OnNav_POSetUpPurchasingBatch();		// begbert 03-31-2011 FUND : Added this
		else if (szCommand == "PO00039") OnNav_POClearPurchasingBatch();		// begbert 03-31-2011 FUND : Added this
		else if (szCommand == "PO00040") OnNav_POEnterRequisitions();
		else if (szCommand == "PO00041") OnNav_POEditRequisitions();
		else if (szCommand == "PO00042") OnNav_POReportsOpenRequisition();		// PBI 40936 06/01/2017 mvt - Added Open Requisition report
		else if (szCommand == "PO00043") OnNav_POSendInternalControlAlertsPurchasing();		// PBI 46406 09/28/2018 mvt - Added security for Internal Control Alerts in PO
		else if (szCommand == "PO00044") OnNav_POSendInternalControlAlertsReceiving();		// PBI 46406 09/28/2018 mvt - Added security for Internal Control Alerts in PO
		else if (szCommand == "PO00045") OnNav_POManageInternalControlAlerts();				// PBI 46406 09/28/2018 mvt - Added security for Internal Control Alerts in PO
		else if (szCommand == "PO00046") OnNav_POManageInternalControlAlertsReceiving();	// PBI 46406 09/28/2018 mvt - Added security for Internal Control Alerts in PO
		else if (szCommand == "PO00047") OnNav_POManageInternalControlAlertsPurchasing();	// PBI 46406 11/21/2018 mvt - Added security for Internal Control Alerts in PO
		else if (szCommand == "PO00048") OnNav_POSendInternalControlAlertsPurchasingNav();	// PBI 46406 11/21/2018 mvt - Added security for Internal Control Alerts in PO
		else
			bToRet = false;
	}

	if (szCommand.Left(2) == "SA")
	{
		if (szCommand == "SA00000") OnNav_SAOpenSalesModule();
		else if (szCommand == "SA00001") OnNav_SAOEEnterOrders();
		else if (szCommand == "SA00002") OnNav_SAOEGenerateOrdersfromExternal();
		else if (szCommand == "SA00003") OnNav_SAOEGenerateOrdersfromRecurring();
		else if (szCommand == "SA00004") OnNav_SAOEChangeLocalTransactionDate();
		else if (szCommand == "SA00005") OnNav_SAOERepairBatchUtility();
		else if (szCommand == "SA00006") OnNav_SAPSEnterSales();
		else if (szCommand == "SA00007") OnNav_SAPSBalanceRegister();
		else if (szCommand == "SA00008") OnNav_SAPSRepairRegisterUtility();
		else if (szCommand == "SA00009") OnNav_SAResetCreditCardTotals();
		else if (szCommand == "SA00010") OnNav_SAPurgeDepositJournal();
		else if (szCommand == "SA00011") OnNav_SAPurgeHistory();
		else if (szCommand == "SA00012") OnNav_SAPurgeSettledChecks();
		else if (szCommand == "SA00013") OnNav_SAPurgePOSHolds();
		else if (szCommand == "SA00014") OnNav_SAPurgeClosedLayaways();
		else if (szCommand == "SA00015") OnNav_SAOEModulePreferences();
		else if (szCommand == "SA00016") OnNav_SAPSModulePreferences();
		else if (szCommand == "SA00017") OnNav_SASetUpSalesEntryLayout();
		//		else if (szCommnad == "SA?????") OnNav_SASetUpItemButtons					();
		else if (szCommand == "SA00018") OnNav_SASetUpLayaways();
		else if (szCommand == "SA00019") OnNav_SASetUpGiftCardOptions();
		else if (szCommand == "SA00020") OnNav_SADisplayTableInformation();
		else if (szCommand == "SA00021") OnNav_SAOEPostInvoices();
		else if (szCommand == "SA00022") OnNav_SAOESetActiveBatchtoBackOrder();
		else if (szCommand == "SA00023") OnNav_SAOESetActiveBatchtoQuote();
		else if (szCommand == "SA00024") OnNav_SAOESetActiveBatchtoRecurring();
		else if (szCommand == "SA00025") OnNav_SAOESelectBatch();
		else if (szCommand == "SA00026") OnNav_SAOESetUpBatch();
		else if (szCommand == "SA00027") OnNav_SAOEClearOrdersBatch();
		else if (szCommand == "SA00028") OnNav_SAPSPostRegisterTransactions();
		else if (szCommand == "SA00029") OnNav_SAPSClearRegisterSales();
		else if (szCommand == "SA00030") OnNav_SASalesDepartmentCodes();
		else if (szCommand == "SA00031") OnNav_SASalespersonCodes();
		else if (szCommand == "SA00032") OnNav_SASalesTaxCodes();
		else if (szCommand == "SA00033") OnNav_SAAlternateTenderCodes();
		else if (szCommand == "SA00034") OnNav_SANormalNonInventoryCodes();
		else if (szCommand == "SA00035") OnNav_SAPaidOutCodes();
		else if (szCommand == "SA00036") OnNav_SACouponBuydownCodes();
		else if (szCommand == "SA00037") OnNav_SACreditCardCodes();
		else if (szCommand == "SA00038") OnNav_SADiscountCodes();
		else if (szCommand == "SA00039") OnNav_SAPriceLevelCodes();
		else if (szCommand == "SA00040") OnNav_SAMarkupMarginCodes();
		else if (szCommand == "SA00041") OnNav_SACustomerSpecialPricing();
		else if (szCommand == "SA00042") OnNav_SARegisterCodes();
		else if (szCommand == "SA00043") OnNav_SARegisterPrintingFormatCodes();
		else if (szCommand == "SA00044") OnNav_SADeviceCodes();
		else if (szCommand == "SA00045") OnNav_SAPrinterCodes();
		else if (szCommand == "SA00046") OnNav_SAFrequencyCodes();
		else if (szCommand == "SA00047") OnNav_SACommentCodes();
		else if (szCommand == "SA00048") OnNav_SACustomTrackingCodes();
		else if (szCommand == "SA00049") OnNav_SAMaintainFlaggedChecks();
		else if (szCommand == "SA00050") OnNav_SAShipViaCodes();
		else if (szCommand == "SA00051") OnNav_SAOrderEntryPrintEditReport();
		else if (szCommand == "SA00052") OnNav_SAOrderEntryPrintPickingReport();
		else if (szCommand == "SA00053") OnNav_SAOrderEntryPrintInvoices();
		else if (szCommand == "SA00054") OnNav_SAOrderEntryPrintShippingLabels();
		else if (szCommand == "SA00055") OnNav_SAOrderEntryPrintPackingSlips();
		else if (szCommand == "SA00056") OnNav_SAPointOfSalePrintEditReport();
		else if (szCommand == "SA00057") OnNav_SAReportsSalesBySalesperson();
		else if (szCommand == "SA00058") OnNav_SAReportsSalesByDepartment();
		else if (szCommand == "SA00059") OnNav_SAReportsSalesByItem();
		else if (szCommand == "SA00060") OnNav_SAReportsSalesByRegister();
		else if (szCommand == "SA00061") OnNav_SAReportsCashReceipts();
		else if (szCommand == "SA00062") OnNav_SAReportsCreditCardReceipts();
		else if (szCommand == "SA00063") OnNav_SAReportsSalesTax();
		else if (szCommand == "SA00064") OnNav_SAReportsProfitMargin();
		else if (szCommand == "SA00065") OnNav_SAReportsCouponBuydown();
		else if (szCommand == "SA00066") OnNav_SAReportsPromotionalSales();
		else if (szCommand == "SA00067") OnNav_SAReportsSalesHistory();
		else if (szCommand == "SA00068") OnNav_SAReportsSalesLabels();
		else if (szCommand == "SA00069") OnNav_SAReportsInvoiceTracking();
		else if (szCommand == "SA00070") OnNav_SAReportsReprintInvoicesFromHistory();
		else if (szCommand == "SA00071") OnNav_SAReportsBackorderTracking();
		else if (szCommand == "SA00072") OnNav_SaReportsBackorderFulfillment();
		else if (szCommand == "SA00073") OnNav_SAReportsDepositJournal();
		else if (szCommand == "SA00074") OnNav_SAReportsGiftCertificate();
		else if (szCommand == "SA00075") OnNav_SAReportsLayawayHistory();
		else if (szCommand == "SA00076") OnNav_SAReportsLayawayByStockItem();
		else if (szCommand == "SA00077") OnNav_SAReportsLayawayBySalesperson();
		else if (szCommand == "SA00078") OnNav_SAReportsLayawayByRegister();
		else if (szCommand == "SA00079") OnNav_SAReportsLayawayDelinquencies();
		else if (szCommand == "SA00080") OnNav_SAReportsLayawaySchedule();
		else if (szCommand == "SA00081") OnNav_SAReportsDailyRegisterTotals();	// begbert 11-10-2010 1-35077 : Added this
		else if (szCommand == "SA00082") OnNav_SAReportsPriceVariance();
		else if (szCommand == "SA00083") OnNav_PSReportsRegisterStatus();		// begbert 11-18-2010 1-35143 : Added this
		else if (szCommand == "SA00084") OnNav_SAReportsMerchantWareReport();	// PBI 19188 jhicks 05/12/2014: Added this
		else if (szCommand == "SA00086") OnNav_SAReportsPureProductSales();		// PBI 24832 jepk 11/25/2014 Added this
		else if (szCommand == "SA00087") OnNav_SASetUp3dCartIntegration();		// PBI 27598 jepk 2/19/2015 Added this
		else if (szCommand == "SA00088") OnNav_SAGenerateOrdersFrom3dCart();	// PBI 27598 jepk 2/19/2015 Added this
		else if (szCommand == "SA00089") OnNav_SASetUpAvaTaxIntegration();		// PBI 27453 jepk 6/30/2015 AvataxIntegration
		else if (szCommand == "SA00090") OnNav_SAReportsRecurringInvoice();		// PBI 36910 10/05/2016 mvt - Added Recurring Invoice report
		else if (szCommand == "SA00091") OnNav_SAReportsSalesByStateRegion();	// PBI 50684 06/26/2019 mvt - Added Sales by State/Region report
		else
			bToRet = false;
	}


	if (szCommand.Left(3) == "GBL")
	{
		if (szCommand == "GBL00001") OnNav_GBLAboutCougarMountainSoftware();	// begbert 11-04-2010 1-34991 : Added this
		else if (szCommand == "GBL00002") OnNav_GBLFinancialDashboard();		// begbert 11-08-2010 1-35036 : Added this
		else if (szCommand == "GBL00003") OnNav_GBLSalesDashboard();			// begbert 11-08-2010 1-35036 : Added this
		else if (szCommand == "GBL00004") OnNav_GBLDenaliBusinessIntelligence();

		else if (szCommand == "GBLB00001") OnNav_GBLSelectBatch(command.m_szDataType, command.m_nValue1, command.m_szValue2, command.m_szKeyFields);
		else if (szCommand == "GBLB00002") OnNav_GBLEditBatch(command.m_szDataType, command.m_nValue1, command.m_szValue2, command.m_szKeyFields);
		else if (szCommand == "GBLB00003") OnNav_GBLClearBatch(command.m_szDataType, command.m_nValue1, command.m_szValue2, command.m_szKeyFields);
		else if (szCommand == "GBLB00004") OnNav_GBLDeleteBatch(command.m_szDataType, command.m_nValue1, command.m_szValue2, command.m_szKeyFields);
		else if (szCommand == "GBLB00005") OnNav_GBLPostBatch(command.m_szDataType, command.m_nValue1, command.m_szValue2, command.m_szKeyFields);
		else if (szCommand == "GBLB00006") OnNav_GBLCreateBatch(command.m_szDataType, command.m_nValue1, command.m_szValue2, command.m_szKeyFields);
		else if (szCommand == "GBLC00001") OnNav_GBLEditCode(command.m_szDataType, command.m_nValue1, command.m_szValue2, command.m_szKeyFields);
		else if (szCommand == "GBLC00002") OnNav_GBLCreateCode(command.m_szDataType, command.m_nValue1, command.m_szValue2, command.m_szKeyFields);
		else if (szCommand == "GBLC00003") OnNav_GBLDeleteCode(command.m_szDataType, command.m_nValue1, command.m_szValue2, command.m_szKeyFields);
		else
			bToRet = false;
	}

	// move the office view to the back
	m_pDenaliOfficeView->GetParentFrame()->SetWindowPos(&CWnd::wndBottom, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

	return bToRet;
}

bool CCMSMDIMainFrame::HasGatewayRights(const CCommand& command)
{
	//si->UserRight.m_stuInventoryNode
	return true;
}

// begbert 08-09-2010 3.0 Interface : command handlers -
//   put global code here, or override in derived classes.
void CCMSMDIMainFrame::OnNav_OpenAccountsPayableModule() { SimulateClickOn(IDS_LAUNCH_AP); }
void CCMSMDIMainFrame::OnNav_OpenAccountsReceivablesModule() { SimulateClickOn(IDS_LAUNCH_AR); }
void CCMSMDIMainFrame::OnNav_OpenBankReconciliationModule() { SimulateClickOn(IDS_LAUNCH_BR); }
void CCMSMDIMainFrame::OnNav_OpenController() { SimulateClickOn(IDS_LAUNCH_CT); }
void CCMSMDIMainFrame::OnNav_OpenGeneralLedgerModule() { SimulateClickOn(IDS_LAUNCH_GL); }
void CCMSMDIMainFrame::OnNav_OpenInventoryModule() { SimulateClickOn(IDS_LAUNCH_IN); }
void CCMSMDIMainFrame::OnNav_OpenPurchaseOrderModule() { SimulateClickOn(IDS_LAUNCH_PO); }
void CCMSMDIMainFrame::OnNav_SAOpenSalesModule() { SimulateClickOn(IDS_LAUNCH_SA); }
void CCMSMDIMainFrame::OnNav_GBLAboutCougarMountainSoftware() { SimulateClickOn(IDS_ABOUT_TITLE); }	// begbert 11-04-2010 1-34991 : Added this
void CCMSMDIMainFrame::OnNav_GBLFinancialDashboard() { SimulateClickOn(IDS_MENU_FINANCIAL_DASHBOARD); }	// begbert 11-08-2010 1-35036 : Added this
void CCMSMDIMainFrame::OnNav_GBLSalesDashboard() { SimulateClickOn(IDS_MENU_SALES_DASHBOARD); }	// begbert 11-08-2010 1-35036 : Added this
void CCMSMDIMainFrame::OnNav_GBLDenaliBusinessIntelligence() { SimulateClickOn(IDS_MENU_DENALIBI); }

// begbert 08-09-2010 3.0 Interface : helper functions
void CCMSMDIMainFrame::SimulateClickOn(UINT nCommandID)
{
	si->ApplicationSettings.UseReportForModule = PRINT_COMBINED;
	if (((ToolBar->GetToolBarCtrl().GetState((int)nCommandID) & TBSTATE_ENABLED) != 0) ||
		(GetMenu()->GetMenuState(nCommandID, MF_BYCOMMAND) != MF_ENABLED))
		CWnd::PostMessage(WM_COMMAND, nCommandID);
}


void CCMSMDIMainFrame::WebBrowserViewNavigate(const CString& szUrl)
{
	if (this == NULL || m_pWebBrowserView == NULL) return;

	m_pWebBrowserView->SendMessage(WM_WBV_NAVIGATE, (WPARAM)(LPCTSTR)szUrl);
}

CView* CCMSMDIMainFrame::ShowForm(const UINT unFormID, LPARAM pParam /*=NULL*/, UINT unSubFormID /*=0*/, CGBLSystemInformation::stuLookupSelected* pLookupInfo /*=NULL*/, const BOOL bStartVisible /*=TRUE*/, bool bCentered /*=true*/)
{
	return ((CCMSWinApp*)AfxGetApp())->m_pFormManager->ShowForm(unFormID, pParam, unSubFormID, pLookupInfo, bStartVisible, bCentered);
}

CView* CCMSMDIMainFrame::ShowFormExclusive(const UINT unFormID, LPARAM pParam /*=NULL*/, UINT unSubFormID /*=0*/, CGBLSystemInformation::stuLookupSelected* pLookupInfo /*=NULL*/, bool bCentered /*=false*/)
{
	return ((CCMSWinApp*)AfxGetApp())->m_pFormManager->ShowFormExclusive(unFormID, pParam, unSubFormID, pLookupInfo, bCentered);
}

// This is the worker thread's function, called by CommandGateway().
// This sends vpParam back to the calling window with the result, and so doesn't delete it.
UINT HandleNavigatorDataRequest(LPVOID vpParam)
{
	CCMSMDIMainFrame::CNavigatorDataRequest* pRequest = (CCMSMDIMainFrame::CNavigatorDataRequest*)vpParam;
	CCommandReceiver::CCommand command = pRequest->GetCommand();
	UINT retval = pRequest->m_pMainFrame->HandleNavigatorDataRequest(command);
	pRequest->SetCommand(command);
	pRequest->m_pMainFrame->PostMessage(WM_USER_MESSAGE_1, (WPARAM)pRequest, (LPARAM)retval);
	return retval;
}

// This is called on the interface thread from the worker thread, and sends a message to the DUI.
// This needs to delete wParam.
LRESULT CCMSMDIMainFrame::OnNavigatorDataRequestReturn(WPARAM wParam, LPARAM lParam)
{
	if (m_pDenaliOfficeView == NULL) return -1;

	CCMSMDIMainFrame::CNavigatorDataRequest* pRequest = (CCMSMDIMainFrame::CNavigatorDataRequest*)wParam;
	UINT retval = (UINT)lParam;

	if (retval != -1)
	{
		CCommand command = pRequest->GetCommand();
		m_pDenaliOfficeView->SendMessage(WM_DUI_UPDATESERVERDATA, (WPARAM)&command);
	}
	delete pRequest;	// Allocated in CommandGateway(); deleted here.

	return 0;
}

static void AppendCodeField(CDataRow& dataRow, const CString& szField, CString& szCodeRet, CString& szKeysRet)
{
	if (szField.IsEmpty()) return;
	CString szCode = dataRow.XMLStr(szField);
	//RCG - 08/09/2016 - Bug 34107 - Added code to escape the five XML special characters.
	szCode = CXMLDocument::FormatStringForXML(szCode);
	szKeysRet += _T("<") + szField + _T(">") + szCode + _T("</") + szField + _T(">");
	if (szCode.IsEmpty()) return;
	if (!szCodeRet.IsEmpty()) szCodeRet += _T("-");
	szCodeRet += szCode;
}

//---------------------------------------------------------------------------
CString CCMSMDIMainFrame::GetCodesBatchesForNavigator(CNavigatorCodeBatchInfo& cbinfo, const CString szToMatch /*=_T("")*/)
{
	if (cbinfo.IsUDF2())
		return GetUDF2CodesForNavigator(cbinfo, szToMatch);

	if (cbinfo.IsUDFn())
		return GetUDFnCodesForNavigator(cbinfo, szToMatch);

	// Should be one more than the max the DUI will display; the DUI will use the extra one
	//   to indicate that it should show a 'display lookup' option after the last.
	const int MAXNUMTOSEND = 51;

	// begbert 11-16-2010 1-35103 : in single-batch mode, return the single batch as 'Single'.
	if (cbinfo.IsBatch() && CMS::GetSI()->ApplicationSettings.MultiBatchMode == MBM_Single)
	{
		return _T("<CodesBatches><CodeBatch><Code>Single</Code><Description>Single Batch</Description><ExtraData/><KeyFields>Single Batch</KeyFields></CodeBatch></CodesBatches>");
	}

	CString szResult = _T("<CodesBatches></CodesBatches>");
	if (cbinfo.IsNone()) return szResult;

	CCMSDataAdapter dataAdapter;
	if (cbinfo.IsBatch())
		dataAdapter = CCMSDataAdapter(CMSStrings::XMLTags::GET, cbinfo.m_szModule, cbinfo.m_szObject);
	else
		dataAdapter = CCMSDataAdapter(CMSStrings::XMLTags::Lookup, cbinfo.m_szModule, cbinfo.m_szObject);
	CDataSet returnDS;

	CString szXML;
	if (cbinfo.IsBatch())
	{
		szXML.Format(
			_T("<ROOT><TABLE><RECORD><strLocation>%s</strLocation><bytBatchType>%d</bytBatchType></RECORD></TABLE></ROOT>"),
			(LPCTSTR)CMS::GetSI()->CompanySettings.DefaultLocation, cbinfo.m_nBatchType);
	}
	else
	{
		szXML.Format(
			_T("<ROOT><SORTBY><%s>0</%s></SORTBY><ROWCOUNT>9999999</ROWCOUNT>%s</ROOT>"),
			(LPCTSTR)cbinfo.m_szCodeTag, (LPCTSTR)cbinfo.m_szCodeTag, (LPCTSTR)cbinfo.m_szKeyInfo);
	}
	CXMLDocument xmlDoc;
	xmlDoc.LoadXML(szXML);

	if (dataAdapter.GetData(xmlDoc, returnDS) == rcSuccess)
	{
		CDataTable dataTable;

		CXMLDocument xmlOutDoc(szToMatch.IsEmpty() ? _T("CodesBatches") : CMSStrings::XMLTags::Root);
		CXMLElement	 eleOutRoot = xmlOutDoc.GetDocumentElement();

		CString szTable = cbinfo.IsBatch() ? CMSStrings::XMLTags::Table : cbinfo.m_szObject;
		if (returnDS.TableExists(szTable))
			returnDS.GetTable(szTable, &dataTable);

		if (!dataTable.IsEmpty())
		{
			for (long lRow = 0; lRow < MAXNUMTOSEND && lRow < dataTable.getRowCount(); lRow++)
			{
				CDataRow dataRow;
				dataTable.GetRow(lRow, &dataRow);

				//				CString szCode   = cbinfo.m_szCodeTag .IsEmpty() ? EMPTY_STRING : dataRow.XMLStr(cbinfo.m_szCodeTag );
				//				CString szDesc   = cbinfo.m_szDescTag .IsEmpty() ? EMPTY_STRING : dataRow.XMLStr(cbinfo.m_szDescTag );
				//				CString szExtra  = EMPTY_STRING;	// As yet no ExtraData has been required.

				CString szCode, szDesc, szExtra, szKeyFields;
				AppendCodeField(dataRow, cbinfo.m_szCodeTag, szCode, szKeyFields);
				AppendCodeField(dataRow, cbinfo.m_szCodeTag2, szCode, szKeyFields);
				AppendCodeField(dataRow, cbinfo.m_szCodeTag3, szCode, szKeyFields);
				if (!cbinfo.m_szDescTag.IsEmpty()) szDesc = dataRow.XMLStr(cbinfo.m_szDescTag);
				szExtra = EMPTY_STRING;	// As yet nothing has been specified for this field.

				if (szToMatch.IsEmpty())
				{
					CXMLElement eleOutRecord = eleOutRoot.AddNewChild(_T("CodeBatch"));

					eleOutRecord.AddNewChild(_T("Code"), szCode);
					eleOutRecord.AddNewChild(_T("Description"), szDesc);
					eleOutRecord.AddNewChild(_T("ExtraData"), szExtra);
					eleOutRecord.AddNewChild(_T("KeyFields"), szKeyFields);

					eleOutRecord.Dispose();
				}
				else
				{
					// If there's a string to match, return the all the fields for the row that matches.
					if (szToMatch != szKeyFields) continue;

					CXMLElement eleOutRecord = eleOutRoot.AddNewChild(CMSStrings::XMLTags::PrimaryKey);

					for (int nCol = 0; nCol < dataRow.getColumnCount(); nCol++)
					{
						CDataCell dcCell;
						dataRow.GetDataCell(nCol, &dcCell);
						eleOutRecord.AddNewChild(dcCell.getColumnName(), dcCell.getColumnText());
					}

					eleOutRecord.Dispose();
					break;	// after we've found/transcripted the matching record, we're done.
				}
			}
		}

		szResult = xmlOutDoc.GetOuterXML();

		xmlOutDoc.Dispose();
		dataTable.Dispose();
	}

	xmlDoc.Dispose();
	returnDS.Dispose();

	return szResult;
}

UINT CCMSMDIMainFrame::HandleNavigatorDataRequest(CCommand& command)
{
	if (command.m_eType != command.eREQUESTDATA) { ASSERT(FALSE); return (UINT)-1; }

	//	si->ApplicationSettings.ShowXMLViewer = true;

	CNavigatorCodeBatchInfo cbinfo = GetCodeBatchInfo(command.m_szDataType);
	if (cbinfo.IsNone())
	{
		//	CGBLForm::CMSMessageBox(this, 0, 0, MB_OK|MB_ICONSTOP, command.m_szDataType, _T("HandleNavigatorDataRequest"), 0, false);
	//		ASSERT(FALSE);
		return (UINT)-1;
	}

	command.m_szResultXML = GetCodesBatchesForNavigator(cbinfo);
	return 0;
}

void CCMSMDIMainFrame::NavCreateEditDel(const CString& szDataType, const CString& szKeyFields, bool bEdit, bool bDel)
{
	CNavigatorCodeBatchInfo cbinfo = GetCodeBatchInfo(szDataType);

	CMaintFormView* pMaintForm;

	if (cbinfo.IsCode())
	{
		// The form information for this code has not been set up - abort.
		if (cbinfo.m_unFormID == 0)
		{
			ASSERT(FALSE);
			return;
		}

		pMaintForm = (CMaintFormView*)ShowForm(cbinfo.m_unFormID, cbinfo.m_pParam, cbinfo.m_unSubFormID, cbinfo.m_pLookupInfo, cbinfo.m_bStartVisible, cbinfo.m_bCentered);
	}
	else if (cbinfo.IsUDF2())
	{
		int nUDF = (szKeyFields.Find(_T("<nCurUDF>1</nCurUDF>")) != -1) ? 1 : 0;

		pMaintForm = (CMaintFormView*)ShowForm(IDD_GBL_DLG_UDF, (LPARAM)&cbinfo.m_UDFInput[nUDF], (UINT)cbinfo.m_UDFInput[nUDF].m_nUDFType);
	}
	else if (cbinfo.IsUDFn())
	{
		pMaintForm = NavCreateEditDelUDFn(cbinfo, szKeyFields);
	}
	else
		return; // Invalid DataType

	// We really ought to have one of these by now.
	ASSERT(pMaintForm);
	if (!pMaintForm) return;

	if (!bEdit) return;

	CString szPrimaryKey = GetCodesBatchesForNavigator(cbinfo, szKeyFields);

	//	NMHDR message;
	//	message.code = WM_BTN_FIND ;
	//	message.hwndFrom = this->GetSafeHwnd();
	//	pMaintForm->SendMessage(WM_NOTIFY, message.idFrom, (LPARAM) &message) ;

	CCMSDoc* pDoc = (CCMSDoc*)pMaintForm->GetDocument();
	bool bMaintDoc = pDoc->IsMaintDoc();
	if (bMaintDoc)
	{
		CCMSMaintDoc* pMaintDoc = (CCMSMaintDoc*)pDoc;
		CString szXML = szPrimaryKey;
		pMaintDoc->OnLookView(szXML);
	}
	else
	{
		CXMLParams clsXMLParams;
		clsXMLParams.SetXML(szPrimaryKey);
		pMaintForm->m_clsXMLParams = clsXMLParams;						//set prams for the get recorded
		pMaintForm->m_bAddOnTheFly = true;
		pMaintForm->OnSetFocusFromLookup(NULL);										//use method in cmsform view to set data tables
		pMaintForm->LookupGetRecord(NULL);												//call method from MaintFormView to get the record
		if (::IsWindow(pMaintForm->m_dlgCMSControlButtons.GetSafeHwnd())) pMaintForm->m_dlgCMSControlButtons.SetFocus();
	}

	// Fake a single click on 'Edit/Del' to edit; two clicks to delete.
	CCMSControlButtons* pButtons = pMaintForm->GetControlButtons();
	ASSERT(pButtons);
	pButtons->PostMessage(WM_COMMAND, IDC_GBL_BTN_EDIT);
	if (bDel)
		pButtons->PostMessage(WM_COMMAND, IDC_GBL_BTN_EDIT);
}

void CCMSMDIMainFrame::OnNav_GBLCreateCode(const CString& szDataType, int nValue1, const CString& szValue2, const CString& szKeyFields)
{
	NavCreateEditDel(szDataType, szKeyFields);
}

void CCMSMDIMainFrame::OnNav_GBLEditCode(const CString& szDataType, int nValue1, const CString& szValue2, const CString& szKeyFields)
{
	NavCreateEditDel(szDataType, szKeyFields, true);
}

void CCMSMDIMainFrame::OnNav_GBLDeleteCode(const CString& szDataType, int nValue1, const CString& szValue2, const CString& szKeyFields)
{
	NavCreateEditDel(szDataType, szKeyFields, true, true);
}


CString CCMSMDIMainFrame::GetUDF2CodesForNavigator(CNavigatorCodeBatchInfo& cbinfo, const CString szToMatch /*=_T("")*/)
{
	CString szResult;
	ASSERT(cbinfo.IsUDF2());
	if (!cbinfo.IsUDF2()) return szResult;

	// Should be one more than the max the DUI will display; the DUI will use the extra one
	//   to indicate that it should show a 'display lookup' option after the last.
	const int MAXNUMTOSEND = 51;

	for (int nUDF = 0; nUDF < 2; nUDF++)
	{
		// If we're looking for a match, see if we found it on a previous pass.
		//   If we did, return it, otherwise clear szReturn.
		if (!szToMatch.IsEmpty())
		{
			if (szResult == _T("<ROOT/>"))
				szResult = EMPTY_STRING;
			if (!szResult.IsEmpty())
				break;
		}

		CNavigatorCodeBatchInfo::CCBInfo pcbinfo = cbinfo.m_pUDFCBInfo[nUDF];

		if (pcbinfo->m_szUDFName.IsEmpty()) continue;

		//		if (nUDF == 0 && !si->UserRight.m_stuAccountsPayableNode.Options.bUserDefinedField1) continue;
		//		if (nUDF == 1 && !si->UserRight.m_stuAccountsPayableNode.Options.bUserDefinedField2) continue;

		CCMSDataAdapter dataAdapter = CCMSDataAdapter(CMSStrings::XMLTags::Lookup, pcbinfo->m_szModule, pcbinfo->m_szObject);
		CDataSet returnDS;

		CString szCodeTag = _T("strUDFCode"); //pcbinfo->m_szCodeTag

		CString szXML;
		szXML.Format(
			_T("<ROOT><SORTBY><%s>0</%s></SORTBY><ROWCOUNT>9999999</ROWCOUNT>%s</ROOT>"),
			(LPCTSTR)szCodeTag, (LPCTSTR)szCodeTag, (LPCTSTR)pcbinfo->m_szKeyInfo);

		CXMLDocument xmlDoc;
		xmlDoc.LoadXML(szXML);

		if (dataAdapter.GetData(xmlDoc, returnDS) == rcSuccess)
		{
			CDataTable dataTable;

			CXMLDocument xmlOutDoc(szToMatch.IsEmpty() ? _T("UDFItem") : CMSStrings::XMLTags::Root);
			CXMLElement	 eleOutRoot = xmlOutDoc.GetDocumentElement();

			if (szToMatch.IsEmpty())
			{
				eleOutRoot.AddAttribute(_T("id"), pcbinfo->m_szUDFName);
				eleOutRoot.AddAttribute(_T("ExtraData"), EMPTY_STRING);
			}

			CString szTable = pcbinfo->m_szObject;
			if (returnDS.TableExists(szTable))
				returnDS.GetTable(szTable, &dataTable);

			if (!dataTable.IsEmpty())
			{
				for (long lRow = 0; lRow < MAXNUMTOSEND && lRow < dataTable.getRowCount(); lRow++)
				{
					CDataRow dataRow;
					dataTable.GetRow(lRow, &dataRow);

					CString szCode, szDesc, szJunk, szKeyFields;
					szKeyFields.Format(_T("<nCurUDF>%d</nCurUDF>"), nUDF);
					AppendCodeField(dataRow, _T("strUDFCode"), szCode, szKeyFields);
					//AppendCodeField(dataRow, _T("bytUDFType"), szJunk, szKeyFields);
					szDesc = dataRow.XMLStr(_T("strDescription"));

					if (szToMatch.IsEmpty())
					{
						CXMLElement eleOutRecord = eleOutRoot.AddNewChild(_T("Code"));

						eleOutRecord.AddNewChild(_T("Id"), szCode);
						eleOutRecord.AddNewChild(_T("Description"), szDesc);
						eleOutRecord.AddNewChild(_T("KeyFields"), szKeyFields);

						eleOutRecord.Dispose();
					}
					else
					{
						// If there's a string to match, return the all the fields for the row that matches.
						if (szToMatch != szKeyFields) continue;

						CXMLElement eleOutRecord = eleOutRoot.AddNewChild(CMSStrings::XMLTags::PrimaryKey);

						for (int nCol = 0; nCol < dataRow.getColumnCount(); nCol++)
						{
							CDataCell dcCell;
							dataRow.GetDataCell(nCol, &dcCell);
							eleOutRecord.AddNewChild(dcCell.getColumnName(), dcCell.getColumnText());
						}

						eleOutRecord.Dispose();
						break;	// after we've found/transcripted the matching record, we're done.
					}
				}
			}

			szResult += xmlOutDoc.GetOuterXML();

			xmlOutDoc.Dispose();
			dataTable.Dispose();
		}

		xmlDoc.Dispose();
		returnDS.Dispose();
	}

	if (szToMatch.IsEmpty())
	{
		szResult = _T("<UDFItems>") + szResult + _T("</UDFItems>");
	}

	return szResult;
}

CString CCMSMDIMainFrame::GetUDFnCodesForNavigator(CNavigatorCodeBatchInfo& cbinfo, const CString szToMatch /*=_T("")*/)
{
	CString szResult;
	ASSERT(cbinfo.IsUDFn());
	if (!cbinfo.IsUDFn()) return szResult;

	// Should be one more than the max the DUI will display; the DUI will use the extra one
	//   to indicate that it should show a 'display lookup' option after the last.
	const int MAXNUMTOSEND = 51;

	UDFCollection& vec = cbinfo.m_UDFCollection;

	CString szObject = _T("LKP_UDF_CODES");

	CCMSDataAdapter dataAdapter = CCMSDataAdapter(CMSStrings::XMLTags::Lookup, cbinfo.m_szModule, szObject);
	CDataSet returnDS;
	CDataTable dataTable;

	CString szXML = _T("<ROOT><SORTBY><strCode>0</strCode></SORTBY><ROWCOUNT>9999999</ROWCOUNT></ROOT>");

	CXMLDocument xmlDoc;
	xmlDoc.LoadXML(szXML);

	if (dataAdapter.GetData(xmlDoc, returnDS) == rcSuccess)
	{
		if (returnDS.TableExists(szObject))
			returnDS.GetTable(szObject, &dataTable);

		// begbert 10-17-2011 1-38353 : even if there are no lookup codes created for the UDFs,
		//   we still need to go through the for loop and create the UDF entries, so that the
		//   UDFs that currently lack codes are still represented.
//		if(!dataTable.IsEmpty())
		{
			for (size_t nUDF = 0; nUDF < vec.size(); nUDF++)
			{
				if (!vec[nUDF].IsUDFLookup) continue;

				CXMLDocument xmlOutDoc(szToMatch.IsEmpty() ? _T("UDFItem") : _T("ROOT"));
				CXMLElement	 eleOutRoot = xmlOutDoc.GetDocumentElement();

				if (szToMatch.IsEmpty())
				{
					eleOutRoot.AddAttribute(_T("id"), vec[nUDF].Caption);
					eleOutRoot.AddAttribute(_T("ExtraData"), EMPTY_STRING);
				}

				for (long lRow = 0; lRow < MAXNUMTOSEND && lRow < dataTable.getRowCount(); lRow++)
				{
					CDataRow dataRow;
					dataTable.GetRow(lRow, &dataRow);

					CString szCode, szDesc, szJunk, szKeyFields;
					AppendCodeField(dataRow, _T("UDF_ID"), szJunk, szKeyFields);
					if (szJunk != vec[nUDF].UDF_ID) continue;	// Only retain lines that match this UDF

					AppendCodeField(dataRow, _T("strModule"), szJunk, szKeyFields);
					AppendCodeField(dataRow, _T("strCode"), szCode, szKeyFields);
					szDesc = dataRow.XMLStr(_T("strDescription"));

					if (szToMatch.IsEmpty())
					{
						CXMLElement eleOutRecord = eleOutRoot.AddNewChild(_T("Code"));

						eleOutRecord.AddNewChild(_T("Id"), szCode);
						eleOutRecord.AddNewChild(_T("Description"), szDesc);
						eleOutRecord.AddNewChild(_T("KeyFields"), szKeyFields);

						eleOutRecord.Dispose();
					}
					else
					{
						// If there's a string to match, return the all the fields for the row that matches.
						if (szToMatch != szKeyFields) continue;

						CXMLElement eleOutRecord = eleOutRoot.AddNewChild(CMSStrings::XMLTags::PrimaryKey);

						for (int nCol = 0; nCol < dataRow.getColumnCount(); nCol++)
						{
							CDataCell dcCell;
							dataRow.GetDataCell(nCol, &dcCell);
							eleOutRecord.AddNewChild(dcCell.getColumnName(), dcCell.getColumnText());
						}

						eleOutRecord.Dispose();
						break;	// after we've found/transcripted the matching record, we're done.
					}
				}

				if (szToMatch.IsEmpty())
				{
					szResult += xmlOutDoc.GetOuterXML();
				}
				else
				{
					// We're only interested in the single non-empty result.
					szResult = xmlOutDoc.GetOuterXML();
					if (szResult != _T("<ROOT/>")) break;	// We found it; we're done
				}

				xmlOutDoc.Dispose();
			}
		}
	}

	dataTable.Dispose();
	xmlDoc.Dispose();
	returnDS.Dispose();

	if (szToMatch.IsEmpty())
	{
		szResult = _T("<UDFItems>") + szResult + _T("</UDFItems>");
	}

	return szResult;
}

CMaintFormView* CCMSMDIMainFrame::NavCreateEditDelUDFn(CNavigatorCodeBatchInfo& cbinfo, const CString& szKeyFields)
{
	UDFCollection& vec = cbinfo.m_UDFCollection;

	CXMLDocument xmlDoc;
	xmlDoc.LoadXML(_T("<ROOT>") + szKeyFields + _T("</ROOT>"));
	CXMLElement xmlRoot = xmlDoc.GetDocumentElement();
	CString szUDFID;
	if (xmlRoot.ElementExists(_T("UDF_ID")))
		szUDFID = xmlRoot.GetItem(_T("UDF_ID")).GetInnerText();
	else
		return NULL;

	for (size_t i = 0; i < vec.size(); i++)
		if (vec[i].UDF_ID == szUDFID)
		{
			cbinfo.m_UDFInput[0].m_szUDFName = vec[i].Caption;
			return (CMaintFormView*)ShowForm(IDD_GBL_DLG_UDF, (LPARAM)&cbinfo.m_UDFInput[0], (UINT)cbinfo.m_UDFInput[0].m_nUDFType);
		}

	return NULL;
}


bool CCMSMDIMainFrame::CreateWebBrowserView(const CString& szUrl, bool bShow)
{
	HMODULE hDotNetDLL = GetDotNetDLL();
	if (hDotNetDLL == NULL) return false;

	GETDLLFUNC(GetWebBrowserViewTemplate, hDotNetDLL);
	if (GetWebBrowserViewTemplate == NULL) return false;

	bool bExists = m_pWebBrowserView != NULL;

	if (bShow == false)
	{	// If it exists, hide it.
		if (bExists)
			m_pWebBrowserView->GetParentFrame()->ShowWindow(SW_HIDE);
		return true;
	}

	// It already exists: simply show it.
	if (bExists)
	{
		WebBrowserViewNavigate(szUrl);
		m_pWebBrowserView->GetParentFrame()->ShowWindow(SW_SHOW);
		return true;
	}

	if (!bExists)
	{
		// Create the navigator pane
		Invalidate();	// repaint the background before starting, as this seizes control of the app for a moment.

		CCMSWaitCursor clsCursor;

		CDocTemplate* pDocTemplate = GetWebBrowserViewTemplate();
		CDocument* pDoc = pDocTemplate->OpenDocumentFile(NULL, false);
		if (pDoc == NULL) return false;	// doc creation failure

		pDoc->UpdateAllViews(NULL);
		POSITION posView = pDoc->GetFirstViewPosition();
		if (posView == NULL) return false;	// view acquisition failure

		m_pWebBrowserView = pDoc->GetNextView(posView);

		// move the office view to the back
		m_pWebBrowserView->GetParentFrame()->SetWindowPos(&CWnd::wndBottom, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

		m_pWebBrowserView->SendMessage(WM_DUI_SETCOMMANDRECEIVER, (WPARAM)(CCommandReceiver*)this, true);

		// Size the control to fill window
		RecalcLayout();

		WebBrowserViewNavigate(szUrl);

		m_pWebBrowserView->GetParentFrame()->ShowWindow(SW_SHOW);

		if (s_bIsDestroyed) return false;	// begbert 12-02-2010 1-35197 : Check whether the window has been destroyed.
	}

	return true;
}
