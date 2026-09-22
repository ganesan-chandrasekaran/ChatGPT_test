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
 *******************************************************************************
 * Initial code - sbarber, shamelessly swiped a good portion of this code from the original CCMSMainFrame class
 ******************************************************************************/
#include "stdafx.h"
#include <deque>
#include <stack>
#include <utility>
#include "..\cmsinclude\HelpTopicIDS.h"
#include "..\cmsdll\ICMSFrameWnd.h"
#include "..\cmsdll\CMSTBButtonInfo.h"
#include "..\cmsdll\CMSModuleDataRepository.h"
#include "..\cmsdll\ICMSWinApp.h"
#include "..\cmsdll\GBLNetDll.h"
#include "..\gblforms\CMSWinApp.h"
#include "..\gblforms\CMSAccelMgr.h"
#include "..\gblforms\CMSMenuBuilder.h"
#include "..\gblforms\CMSModuleTracker.h"
#include "..\gblforms\CMSWindowManager.h"
#include "..\gblforms\GBL_DLG_WndSpy.h"
#include "..\gblforms\CMSMainFrameSupportServices.h"

typedef std::pair<int, int>		Point;

//------------------------------------------------------------------------------
class CCMSPoint : Point
{
public:
	CCMSPoint(void) : Point(0, 0) {};
	CCMSPoint(int x, int y) : Point(x, y) {};
	CCMSPoint(const CCMSPoint& rhs) : Point(rhs.x, rhs.y) {};
	virtual ~CCMSPoint(void) {};

	const CCMSPoint& operator= (const CCMSPoint& rhs) { if (this != &rhs) { first = rhs.first; second = rhs.second; } return *this; }

	int get_X(void) const { return first; }
	int get_Y(void) const { return second; }

	void put_X(int x) { first = x; }
	void put_Y(int y) { second = y; }

	__declspec(property(get = get_X, put = put_X))	int		x;
	__declspec(property(get = get_Y, put = put_Y))	int		y;
};

//------------------------------------------------------------------------------
struct InitModuleStatusMessageFtor
{
	InitModuleStatusMessageFtor(ICMSMainFrameWnd* pFrame) : m_pFrame(pFrame) {};

	void operator() (const CCMSModuleInfo::CCMSModule& vt)
	{
		m_pFrame->AddStatusBarMessage(vt.GetModuleLaunchID(), vt.GetModuleDesc());
	}

	ICMSMainFrameWnd* const m_pFrame;
};

//these free functions are not meant for consumption outside this file
//------------------------------------------------------------------------------
void UpdateMenuString(CString& szMenuItem, UINT nCommandID)
{
	//Show accelerator key names in menu. PGP(10/27/2004)
	//In case mainwindow is not assigned to CWinApp yet, don't access.
	//AfxGetMainWnd call will return the current active window, we need application main.
	CWnd* const	pMainWnd = ::AfxGetApp()->m_pMainWnd;
	if (pMainWnd)
	{
		static CCMSWinApp* pApp = DYNAMIC_DOWNCAST(CCMSWinApp, ::AfxGetApp());
		ASSERT(pApp);

		//Query AccelMgr for an entry, if found, get the key name, add to menu string.
		static CCMSAccelMgr& clsAccelMgr = pApp->GetAccelMgr();

		if (clsAccelMgr.HasEntry((WORD)nCommandID))
		{
			CString szKeyString(_T(""));
			clsAccelMgr.GetKeyString((WORD)nCommandID, szKeyString);
			szMenuItem += _T("\t") + szKeyString;
		}
	}
}

//------------------------------------------------------------------------------
//Defect ID 1-28253, 1-28493, PGP(10-27-2007)
void RefreshFileMenuItems(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	const bool bSalesEntry = (pFrame->si->ModulesInstalled.OrderEntry.IsUsable && pFrame->si->ModulesInstalled.PointOfSale.IsUsable);

	COXBitmapMenu menu;
	CMenu* const		pMenu = pFrame->Menu[IDS_FILE];

	if (!pMenu) return;

	//Reset all menu items, as menu does not have hide functionality like toolbar does.
	int nCount = pMenu->GetMenuItemCount();
	for (int i = 0; i < nCount; ++i)
		pMenu->DeleteMenu(0, MF_BYPOSITION);

	//now add all.
	if (pFrame->si->ModulesInstalled.AccountsPayable.IsUsable)
	{
		menu.CreatePopupMenu();
		pMenu->AppendMenu(MF_POPUP, (UINT_PTR)menu.Detach(), CGBLResources::GetResourceString(IDS_MENU_ACCOUNTS_PAYABLE));
	}

	if (pFrame->si->ModulesInstalled.AccountsReceivable.IsUsable)
	{
		menu.CreatePopupMenu();
		pMenu->AppendMenu(MF_POPUP, (UINT_PTR)menu.Detach(), CGBLResources::GetResourceString(IDS_ACCOUNTS_RECEIVABLE));
	}

	if (pFrame->si->ModulesInstalled.BankReconciliation.IsUsable)
	{
		menu.CreatePopupMenu();
		pMenu->AppendMenu(MF_POPUP, (UINT_PTR)menu.Detach(), CGBLResources::GetResourceString(IDS_BANK_RECONCILIATION));
	}

	if (pFrame->si->ModulesInstalled.GeneralLedger.IsUsable)
	{
		menu.CreatePopupMenu();
		pMenu->AppendMenu(MF_POPUP, (UINT_PTR)menu.Detach(), CGBLResources::GetResourceString(IDS_MENU_GENERAL_LEDGER));
	}

	if (pFrame->si->ModulesInstalled.Inventory.IsUsable)
	{
		menu.CreatePopupMenu();
		pMenu->AppendMenu(MF_POPUP, (UINT_PTR)menu.Detach(), CGBLResources::GetResourceString(IDS_INVENTORY));
	}

	if (pFrame->si->ModulesInstalled.JobCost.IsUsable)
	{
		menu.CreatePopupMenu();
		pMenu->AppendMenu(MF_POPUP, (UINT_PTR)menu.Detach(), CGBLResources::GetResourceString(IDS_MENU_JOBCOST));
	}

	if (pFrame->si->ModulesInstalled.PurchaseOrder.IsUsable)
	{
		menu.CreatePopupMenu();
		pMenu->AppendMenu(MF_POPUP, (UINT_PTR)menu.Detach(), CGBLResources::GetResourceString(IDS_PURCHASE_ORDER));
	}

	if (bSalesEntry)
	{
		menu.CreatePopupMenu();
		pMenu->AppendMenu(MF_POPUP, (UINT_PTR)menu.Detach(), CGBLResources::GetResourceString(IDS_SALES_MAINTENANCE));
	}
	else if (pFrame->si->ModulesInstalled.OrderEntry.IsUsable)
	{
		menu.CreatePopupMenu();
		// begbert 01-29-2010 1-34155 : Corrected from IDS_ORDER_MAINTENANCE to IDS_SALES_MAINTENANCE
		pMenu->AppendMenu(MF_POPUP, (UINT_PTR)menu.Detach(), CGBLResources::GetResourceString(IDS_SALES_MAINTENANCE));
	}
	else if (pFrame->si->ModulesInstalled.PointOfSale.IsUsable)
	{
		menu.CreatePopupMenu();
		// begbert 01-29-2010 1-34155 : Corrected from IDS_POINT_OF_SALE_MAINTENANCE to IDS_SALES_MAINTENANCE
		pMenu->AppendMenu(MF_POPUP, (UINT_PTR)menu.Detach(), CGBLResources::GetResourceString(IDS_SALES_MAINTENANCE));
	}
	// begbert 01-17-2012 Payroll: Added menu item
	if (pFrame->si->ModulesInstalled.Payroll.IsUsable)
	{
		menu.CreatePopupMenu();
		pMenu->AppendMenu(MF_POPUP, (UINT_PTR)menu.Detach(), CGBLResources::GetResourceString(IDS_PAYROLL));
	}

	if (!pFrame->si->ApplicationSettings.IsCT())
		CCMSMainFrameSupportServices::AddMenu(pFrame, pMenu, IDS_CONTROLLER, IDS_CONTROLLER, CGBLResources::GetResourceString(IDS_CONTROLLER_STATUS));

	CCMSMainFrameSupportServices::AddSeparator(pFrame, pMenu);

	if (pFrame->si->ModulesInstalled.DenaliBI.Licensed &&
		pFrame->si->ApplicationSettings.StartupModule != CAppSettings::DENALIBI)
	{
		CCMSMainFrameSupportServices::AddMenu(pFrame, pMenu, IDS_MENU_DENALIBI, IDS_MENU_DENALIBI, RESSTRING(IDS_MODULE_BI));
	}

	// 1-34642/1-34645 BK 8/19/10 added submenus to Cougar Dtails, financial & sales dashboards
	CCMSMainFrameSupportServices::AddMenu(pFrame, pMenu, IDS_FILE_COUGAR_DTAILS);
	int nID = pMenu->GetMenuItemCount();
	CMenu* pDtailMenu = pMenu->GetSubMenu(nID - 1);
	CCMSMainFrameSupportServices::AddMenu(pFrame, pDtailMenu, IDS_MENU_FINANCIAL_DASHBOARD, IDS_MENU_FINANCIAL_DASHBOARD, RESSTRING(IDS_STATUS_FINANCIAL_DASHBOARD));
	CCMSMainFrameSupportServices::AddMenu(pFrame, pDtailMenu, IDS_MENU_SALES_DASHBOARD, IDS_MENU_SALES_DASHBOARD, RESSTRING(IDS_STATUS_SALES_DASHBOARD));

	CCMSMainFrameSupportServices::AddSeparator(pFrame, pMenu);

	const CString szStatusMsg = CGBLResources::GetResourceString(IDS_STATUS_EXIT) + _T(" ") + pFrame->si->ApplicationSettings.ModuleName;
	CCMSMainFrameSupportServices::AddMenu(pFrame, pMenu, IDS_EXIT, IDS_EXIT, szStatusMsg);

	pFrame->Frame->DrawMenuBar();
}

//------------------------------------------------------------------------------
void AddModuleGUIItems(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	CCMSMainFrameSupportServices::ToolBarAddSeparatorButton(pFrame);
	CCMSMainFrameSupportServices::ToolBarAddModuleButtonCluster(pFrame);					// program module button cluster GL,BR...

	RefreshFileMenuItems(pFrame);
}

//------------------------------------------------------------------------------
void	EnableModuleMenuItem(ICMSMainFrameWnd* pFrame, CCmdUI* pCmdUI, UINT nMenuID)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);
	ASSERT(pCmdUI);

	const UINT		nState = pCmdUI->m_pParentMenu->GetMenuState(pCmdUI->m_nID, MF_BYCOMMAND);
	const bool		bolMe = pFrame->ModuleTracker->IsMe(pFrame->si->ApplicationSettings.ModuleID, pCmdUI->m_nID);
	const BOOL		bolEnabled = ((nState & MF_ENABLED) == MF_ENABLED) && !bolMe;

	pCmdUI->Enable(CCMSModuleInfo::Instance().CanLaunch(nMenuID) && bolEnabled);
}

//------------------------------------------------------------------------------
UINT GetDocumentViewCount(const CDocument* pDocument, bool bolIconic)
{
	ASSERT(pDocument);

	UINT			nCount = 0;
	CView* pView = NULL;
	CFrameWnd* pFrame = NULL;
	POSITION		pos = pDocument->GetFirstViewPosition();

	while (pos)
	{
		pView = pDocument->GetNextView(pos);
		if (pView)
		{
			pFrame = pView->GetParentFrame();
			if (pFrame)
			{
				if (bolIconic)		//counting windows which are iconic
					nCount += pFrame->IsIconic() ? 1 : 0;
				else
					nCount += pFrame->IsIconic() ? 0 : 1;
			}
		}
	}

	return nCount;
}

//------------------------------------------------------------------------------
UINT GetTemplateViewCount(CDocTemplate* pTemplate, bool bolIconic)
{
	ASSERT(pTemplate);

	UINT			nCount = 0;
	CDocument* pDocument = NULL;
	POSITION		pos = pTemplate->GetFirstDocPosition();

	while (pos)
	{
		pDocument = pTemplate->GetNextDoc(pos);
		if (pDocument) nCount += GetDocumentViewCount(pDocument, bolIconic);
	}

	return nCount;
}

//------------------------------------------------------------------------------
UINT GetWindowCount(bool bolIconic = false)
{
	CCMSWinApp* const	pWinApp = DYNAMIC_DOWNCAST(CCMSWinApp, ::AfxGetApp());
	ASSERT_VALID(pWinApp);

	if (!pWinApp) return 0;

	UINT			nCount = 0;
	CDocTemplate* pTemplate = NULL;
	POSITION		pos = pWinApp->GetFirstDocTemplatePosition();

	while (pos)
	{
		pTemplate = pWinApp->GetNextDocTemplate(pos);
		if (pTemplate) nCount += GetTemplateViewCount(pTemplate, bolIconic);
	}

	return nCount;
}

//------------------------------------------------------------------------------
bool CloseDocumentViews(const CDocument* pDocument)
{
	ASSERT(pDocument);

	bool			bolCanClose = true;
	CFrameWnd* pCurrentFrame = NULL;
	CFrameWnd* pNewFrame = NULL;
	CView* pView = NULL;
	POSITION		pos = pDocument->GetFirstViewPosition();

	while (pos && bolCanClose)
	{
		pView = pDocument->GetNextView(pos);
		if (pView)
		{
			CCMSFormView* const pForm = DYNAMIC_DOWNCAST(CCMSFormView, pView);
			if (pForm)
			{
				pCurrentFrame = pForm->GetParentFrame();
				pCurrentFrame->SendMessage(WM_CLOSE);
				CGBLForm::DoEvents();

				pNewFrame = CGBLForm::EkGetActiveFrame();
				bolCanClose = pCurrentFrame != pNewFrame;
			}
		}
	}

	return bolCanClose;
}

//------------------------------------------------------------------------------
bool CloseTemplateDocuments(CDocTemplate* pTemplate)
{
	ASSERT(pTemplate);

	bool			bolCanClose = true;
	CDocument* pDocument = NULL;
	POSITION		pos = pTemplate->GetFirstDocPosition();

	while (pos && bolCanClose)
	{
		pDocument = pTemplate->GetNextDoc(pos);
		if (pDocument) bolCanClose = CloseDocumentViews(pDocument);
	}

	return bolCanClose;
}

//------------------------------------------------------------------------------
bool CloseWindows(void)
{
	CCMSWinApp* const pWinApp = DYNAMIC_DOWNCAST(CCMSWinApp, ::AfxGetApp());
	ASSERT_VALID(pWinApp);

	if (!pWinApp) return true;

	bool			bolCanClose = true;
	CDocTemplate* pTemplate = NULL;
	POSITION		pos = pWinApp->GetFirstDocTemplatePosition();

	while (pos && bolCanClose)
	{
		pTemplate = pWinApp->GetNextDocTemplate(pos);
		if (pTemplate) bolCanClose = CloseTemplateDocuments(pTemplate);
	}

	return bolCanClose;
}

//------------------------------------------------------------------------------
CCMSPoint CalculateToolbarMenuPos(ICMSMainFrameWnd* pFrame, int nItem)
{
	//JKC DefectID: 1-7756 Get Toolbar window rect as a baseline for where to put popup window
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	CRect			rectBtn(0, 0, 0, 0);
	CRect			rectToolbar(0, 0, 0, 0);

	pFrame->ToolBar->GetWindowRect(&rectToolbar);
	pFrame->ToolBar->GetToolBarCtrl().GetRect(nItem, &rectBtn);

	const BOOL bFloating = pFrame->ToolBar->IsFloating();

	//If docked and single row, we must be on the top or bottom and need a horizontal adjustment
	const int		nDockedTopAdjustment = ((!bFloating) && (pFrame->ToolBar->GetToolBarCtrl().GetRows() == 1)) ? 15 : 0;
	//If docked and multi row, we must be on one of the sides and need a vertical adjustment
	const int		nDockedRowAdjustment = ((!bFloating) && (pFrame->ToolBar->GetToolBarCtrl().GetRows() > 1)) ? 15 : 0;

	CCMSPoint	pos;
	//Left position of window = Button left + Toolbar left + Horizontal Adjustment
	pos.x = rectBtn.left + 2 + rectToolbar.left + nDockedTopAdjustment; //JKC DefectID: 1-7756 Adjust starting x pos
	//Top position of window = Button bottom + Toolbar top + Vertical Adjustment
	pos.y = rectBtn.bottom + rectToolbar.top + nDockedRowAdjustment; //JKC DefectID: 1-7756 Adjust starting y pos

	return pos;
}

//------------------------------------------------------------------------------
bool InitializeToolBarMenu(ICMSMainFrameWnd* pFrame, const CCMSPoint& pos, int nItem)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	bool			bolMenuHandled = false;

	switch (nItem)
	{
	case IDS_LAUNCH_AP:
		ASSERT_VALID(pFrame->Menu[IDS_MENU_ACCOUNTS_PAYABLE]);
		pFrame->ModuleTracker->UpdateMenu(IDS_AP_MODULE, pFrame->Menu[IDS_MENU_ACCOUNTS_PAYABLE]);
		pFrame->Menu[IDS_MENU_ACCOUNTS_PAYABLE]->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pos.x, pos.y, pFrame->Frame);
		bolMenuHandled = true;
		break;
	case IDS_LAUNCH_AR:
		ASSERT_VALID(pFrame->Menu[IDS_ACCOUNTS_RECEIVABLE]);
		pFrame->ModuleTracker->UpdateMenu(IDS_AR_MODULE, pFrame->Menu[IDS_ACCOUNTS_RECEIVABLE]);
		pFrame->Menu[IDS_ACCOUNTS_RECEIVABLE]->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pos.x, pos.y, pFrame->Frame);
		bolMenuHandled = true;
		break;
	case IDS_LAUNCH_BR:
		ASSERT_VALID(pFrame->Menu[IDS_BANK_RECONCILIATION]);
		pFrame->ModuleTracker->UpdateMenu(IDS_BR_MODULE, pFrame->Menu[IDS_BANK_RECONCILIATION]);
		pFrame->Menu[IDS_BANK_RECONCILIATION]->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pos.x, pos.y, pFrame->Frame);
		bolMenuHandled = true;
		break;
	case IDS_LAUNCH_GL:
		ASSERT_VALID(pFrame->Menu[IDS_MENU_GENERAL_LEDGER]);
		pFrame->ModuleTracker->UpdateMenu(IDS_GL_MODULE, pFrame->Menu[IDS_MENU_GENERAL_LEDGER]);
		pFrame->Menu[IDS_MENU_GENERAL_LEDGER]->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pos.x, pos.y, pFrame->Frame);
		bolMenuHandled = true;
		break;
	case IDS_LAUNCH_IN:
		ASSERT_VALID(pFrame->Menu[IDS_INVENTORY]);
		pFrame->ModuleTracker->UpdateMenu(IDS_IN_MODULE, pFrame->Menu[IDS_INVENTORY]);
		pFrame->Menu[IDS_INVENTORY]->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pos.x, pos.y, pFrame->Frame);
		bolMenuHandled = true;
		break;
	case IDS_LAUNCH_JC:
		ASSERT_VALID(pFrame->Menu[IDS_MENU_JOBCOST]);
		pFrame->ModuleTracker->UpdateMenu(IDS_JC_MODULE, pFrame->Menu[IDS_MENU_JOBCOST]);
		pFrame->Menu[IDS_MENU_JOBCOST]->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pos.x, pos.y, pFrame->Frame);
		bolMenuHandled = true;
		break;
	case IDS_LAUNCH_PR:
		ASSERT_VALID(pFrame->Menu[IDS_PAYROLL]);
		pFrame->ModuleTracker->UpdateMenu(IDS_PR_MODULE, pFrame->Menu[IDS_PAYROLL]);
		pFrame->Menu[IDS_PAYROLL]->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pos.x, pos.y, pFrame->Frame);
		bolMenuHandled = true;
		break;
	case IDS_LAUNCH_PO:
		ASSERT_VALID(pFrame->Menu[IDS_PURCHASE_ORDER]);
		pFrame->ModuleTracker->UpdateMenu(IDS_PO_MODULE, pFrame->Menu[IDS_PURCHASE_ORDER]);
		pFrame->Menu[IDS_PURCHASE_ORDER]->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pos.x, pos.y, pFrame->Frame);
		bolMenuHandled = true;
		break;
	case IDS_LAUNCH_SA:
		ASSERT_VALID(pFrame->Menu[IDS_SALES_MAINTENANCE]);
		pFrame->ModuleTracker->UpdateMenu(IDS_SA_MODULE, pFrame->Menu[IDS_SALES_MAINTENANCE]);
		pFrame->Menu[IDS_SALES_MAINTENANCE]->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pos.x, pos.y, pFrame->Frame);
		bolMenuHandled = true;
		break;
	case IDS_LAUNCH_OE:
		ASSERT_VALID(pFrame->Menu[IDS_ORDER_ENTRY]);
		pFrame->ModuleTracker->UpdateMenu(IDS_OE_MODULE, pFrame->Menu[IDS_ORDER_ENTRY]);
		pFrame->Menu[IDS_ORDER_ENTRY]->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pos.x, pos.y, pFrame->Frame);
		bolMenuHandled = true;
		break;
	case IDS_LAUNCH_PS:
		ASSERT_VALID(pFrame->Menu[IDS_POINT_OF_SALE_ENTRY]);
		pFrame->ModuleTracker->UpdateMenu(IDS_PS_MODULE, pFrame->Menu[IDS_POINT_OF_SALE_ENTRY]);
		pFrame->Menu[IDS_POINT_OF_SALE_ENTRY]->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pos.x, pos.y, pFrame->Frame);
		bolMenuHandled = true;
		break;
	};

	return bolMenuHandled;
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::SetTitleText(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	if (!pFrame->TitleBarID) return;

	CString strTitle(_T(""));

	if (!pFrame->si->CompanySettings.Name.IsEmpty() && !pFrame->si->CompanySettings.ID.IsEmpty())
	{
		switch (pFrame->si->ApplicationSettings.ModuleID)
		{
		case IDS_CT_MODULE:
			strTitle = CGBLResources::GetResourceString(pFrame->TitleBarID) + _T(" (") +
				pFrame->si->CompanySettings.ID + _T(")");
			break;

		default:
			strTitle = CGBLResources::GetResourceString(pFrame->TitleBarID) + _T(" - ") +
				pFrame->si->CompanySettings.Name + _T(" (") +
				pFrame->si->CompanySettings.ID + _T(")");
			break;
		};
	}
	else
	{
		strTitle = CGBLResources::GetResourceString(pFrame->TitleBarID);
		strTitle.Trim();
		if (strTitle[strTitle.GetLength() - 1] == _T('-')) strTitle = strTitle.Left(strTitle.GetLength() - 1); // stripping off trailing dash
	}

	// begbert 07-28-2010 Version 3.0 : Append the Instance number.
	CCMSModuleTracker* pModuleTracker = pFrame->ModuleTracker.get();
	if (pModuleTracker && pFrame->si->ApplicationSettings.ModuleID != IDS_CT_MODULE)
	{
		strTitle += " - " + pModuleTracker->GetMyInstance(pFrame->si->ApplicationSettings.ModuleID);
	}

	strTitle.Replace(_T("- -"), _T("-"));
	pFrame->Frame->SetWindowText(strTitle);
}

//------------------------------------------------------------------------------
bool CCMSMainFrameSupportServices::AttachOrganizer(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	COXBitmapMenuOrganizer& menuOrganizer = pFrame->MenuOrganizer;
	if (!menuOrganizer.IsAttached())
	{
		if (!menuOrganizer.AttachFrameWnd(pFrame->Frame)) // attaching frame window to menu organizer
			return false;
	}

	CMenu& menu = pFrame->MainMenu;
	ASSERT_VALID(&menu);
	TRACE(_T("MENU ITEM COUNT - %d\n"), menu.GetMenuItemCount());

	for (int i = menu.GetMenuItemCount() - 1; i > -1; i--)
		menu.DeleteMenu((UINT)i, MF_BYPOSITION);

	COXBitmapMenu oxMenu;
	pFrame->MainMenu = oxMenu;

	return true;
}

//------------------------------------------------------------------------------
bool CCMSMainFrameSupportServices::CreateMenu(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	CMenu& mainMenu = pFrame->MainMenu;

	COXBitmapMenu menu;
	menu.CreatePopupMenu();
	mainMenu.AppendMenu(MF_POPUP, (UINT_PTR)menu.Detach(), CGBLResources::GetResourceString(IDS_FILE));

	pFrame->CreateMenu();		//allow frame to add custom items

	//Create Window menu
	menu.CreatePopupMenu();
	mainMenu.AppendMenu(MF_POPUP, (UINT_PTR)menu.m_hMenu, CGBLResources::GetResourceString(IDS_MENU_WINDOW));
	if (pFrame->MDIFrame)
	{
		menu.AppendMenu(MF_STRING, IDS_TILE_HORIZONTALLY, CGBLResources::GetResourceString(IDS_TILE_HORIZONTALLY));
		menu.AppendMenu(MF_STRING, IDS_TILE_VERTICALLY, CGBLResources::GetResourceString(IDS_TILE_VERTICALLY));
		menu.AppendMenu(MF_STRING, IDS_CASCADE, CGBLResources::GetResourceString(IDS_CASCADE));
		menu.AppendMenu(MF_STRING, IDS_ARRANGE_ICONS, CGBLResources::GetResourceString(IDS_ARRANGE_ICONS));
		menu.AppendMenu(MF_SEPARATOR);
	}
	menu.AppendMenu(MF_STRING, IDS_CLOSE_ALL, CGBLResources::GetResourceString(IDS_CLOSE_ALL));
	menu.Detach();

	//Create Help menu
	menu.CreatePopupMenu();
	mainMenu.AppendMenu(MF_POPUP, (UINT_PTR)menu.m_hMenu, CGBLResources::GetResourceString(IDS_MENU_HELP));
	menu.AppendMenu(MF_STRING, IDS_MENU_CONTENTS, CGBLResources::GetResourceString(IDS_MENU_CONTENTS));
	pFrame->StatusMessages.AddMessage(IDS_MENU_CONTENTS, IDS_STATUS_MENU_CONTENTS);
	menu.AppendMenu(MF_SEPARATOR);

	menu.AppendMenu(MF_STRING, IDS_HELPMENU_CMS_WEBSITE, CGBLResources::GetResourceString(IDS_HELPMENU_CMS_WEBSITE));
	pFrame->StatusMessages.AddMessage(IDS_HELPMENU_CMS_WEBSITE, IDS_STATUS_HELPMENU_CMS_WEBSITE);

	menu.AppendMenu(MF_STRING, IDS_HELPMENU_ONLINE_SUPPORT, CGBLResources::GetResourceString(IDS_HELPMENU_ONLINE_SUPPORT));
	pFrame->StatusMessages.AddMessage(IDS_HELPMENU_ONLINE_SUPPORT, IDS_STATUS_HELPMENU_ONLINE_SUPPORT);
	menu.AppendMenu(MF_SEPARATOR);

	menu.AppendMenu(MF_STRING, IDS_HELPMENU_ORDER_CMS_FORMS, CGBLResources::GetResourceString(IDS_HELPMENU_ORDER_CMS_FORMS));
	pFrame->StatusMessages.AddMessage(IDS_HELPMENU_ORDER_CMS_FORMS, IDS_STATUS_HELPMENU_ORDER_CMS_FORMS);
	menu.AppendMenu(MF_STRING, IDS_HELPMENU_MERCHANT_SERVICES, CGBLResources::GetResourceString(IDS_HELPMENU_MERCHANT_SERVICES));
	pFrame->StatusMessages.AddMessage(IDS_HELPMENU_MERCHANT_SERVICES, IDS_STATUS_HELPMENU_MERCHANT_SERVICES);
	menu.AppendMenu(MF_SEPARATOR);

	menu.AppendMenu(MF_STRING, IDS_ABOUT_TITLE, CGBLResources::GetResourceString(IDS_ABOUT_COUGAR_MOUNTAIN_SOFTWARE));
	pFrame->StatusMessages.AddMessage(IDS_ABOUT_TITLE, IDS_STATUS_ABOUT_TITLE);
	menu.Detach();

	pFrame->Frame->DrawMenuBar();

	return true;
}

//------------------------------------------------------------------------------
bool CCMSMainFrameSupportServices::CreateToolBar(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	CGBLSystemInformation* const	si = pFrame->si;

	if (si->ApplicationSettings.ModuleID != IDS_SAE_MODULE &&
		si->ApplicationSettings.ModuleID != IDS_POE_MODULE)
	{
		COXCoolToolBar* const pToolbar = pFrame->ToolBar;

		if (!pToolbar->CreateEx(pFrame->Frame, TBSTYLE_FLAT,
								WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER |
								CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC |
								TBSTYLE_DROPDOWN))
		{
			TRACE0("Failed to create toolbar\n");
			return false;      // fail to create
		}
		else
			pToolbar->GetToolBarCtrl().SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);

		//DG.03.21.2011 1-35466 Problems with toolbar resizing unexpectedly
		//pToolbar->SetButtonsMinMaxWidth(32,32);


		// begbert 07-21-2010 Version 3.0 : The icons should be 24x24 in size now.
//		const int intIConXDim = ::GetSystemMetrics(SM_CXSMICON);
//		const int intIConYDim = ::GetSystemMetrics(SM_CYSMICON);
		const int intIConXDim = 24;
		const int intIConYDim = 24;

		CImageList& images = pFrame->ToolBarImages;
		images.Create(intIConXDim, intIConYDim, ILC_COLOR24 | ILC_MASK, 20, 3);

		// tool bar icons
		// Put all of the global ones in here first and then
		// call the child class's routine to add it's buttons.
		images.Add(si->Drawing.Icons.Save());			//enuTBSaveIdx
		images.Add(si->Drawing.Icons.Clear());				//enuTBClearEntryIdx
		images.Add(si->Drawing.Icons.Printer());			//enuTBPrintIdx
		images.Add(si->Drawing.Icons.AP16());				//enuTBAcountsPayIdx
		images.Add(si->Drawing.Icons.AR16());				//enuTBAcountsRecvIdx
		images.Add(si->Drawing.Icons.BR16());				//enuTBBankRecIdx
		//images.Add(si->Drawing.Icons.DX16() );			//enuTBDataExchangeIdx
		images.Add(si->Drawing.Icons.GL16());				//enuTBGenLedgerIdx
		images.Add(si->Drawing.Icons.IN16());				//enuTBInventoryIdx
		//	PBI 24421	04/21/2015	mvt - Set up Job Cost integration
		images.Add(si->Drawing.Icons.JC16());			//enuTBJobCostIdx
		images.Add(si->Drawing.Icons.OE16());				//enuTBOrderEntryIdx
		images.Add(si->Drawing.Icons.PO16());				//enuTBPurchaseOrdIdx
		images.Add(si->Drawing.Icons.PS16());				//enuTBPOSEntryIdx
		// BK 1/16/12 added payroll
		images.Add(si->Drawing.Icons.PR16());				//enuTBPayrollIdx
		images.Add(si->Drawing.Icons.SM16());				//enuTBSalesMaintIdx
		images.Add(si->Drawing.Icons.CT16());				//enuTBControllerIdx
		images.Add(si->Drawing.Icons.Help16());			//enuTBHelpIdx
		images.Add(si->Drawing.Icons.DTails());			//enuTBCougarDtailsIdx	// begbert 02-08-2010 CDtails : Added this
		images.Add(si->Drawing.Icons.SalesDTails());		//enuTBSalesDtailsIdx
		images.Add(si->Drawing.Icons.DenaliBI());			//enuTBDenaliBI

		pToolbar->GetToolBarCtrl().SetImageList(&images);
		pToolbar->EnableDocking(CBRS_ALIGN_TOP | CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);
		pToolbar->SetBarStyle(pToolbar->GetBarStyle() | CBRS_SIZE_DYNAMIC);

		ToolBarAddPrintSaveClearButtonCluster(pFrame);	// save button
		ToolBarAddSeparatorButton(pFrame);						// separator
		pFrame->CreateModuleToolBar();								//add module specific buttons

	}

	return true;
}

//------------------------------------------------------------------------------
bool CCMSMainFrameSupportServices::CreateStatusBar(ICMSMainFrameWnd* pFrame)
{
	if (!pFrame->StatusBar->Create(pFrame->Frame, pFrame->si))	return false;
	pFrame->StatusBar->RegisterUpdateHandler(pFrame->StatusBarHandler);
	//Defect ID 1-28295, PGP(03-02-2008)
	if (pFrame->si->CommandLineSettings.Hide_StatusBar)
		pFrame->Frame->ShowControlBar(pFrame->StatusBar, FALSE, FALSE);
	return true;
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::InitModuleStatusMessages(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);

	const CCMSModuleInfo::CCMSModuleData& objModuleData = CCMSModuleInfo::Instance();
	std::for_each(objModuleData.begin(), objModuleData.end(), InitModuleStatusMessageFtor(pFrame));
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::MenuSecurityDisableItems(ICMSMainFrameWnd* pFrame)
{
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::UpdateMenu(ICMSMainFrameWnd* pFrame, bool bStartUp)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	if (bStartUp)
	{
#ifdef _DEBUG
		CGBLSystemInformation* const	si = pFrame->si;
		ASSERT(si);

		if (si->ApplicationSettings.DeveloperMode) // only add if in developer mode
		{
			if (!AddMenu(pFrame, &pFrame->MainMenu, IDS_DEVELOPER_MENU)) return;

			if (!AddMenu(pFrame, pFrame->Menu[IDS_DEVELOPER_MENU], IDS_GBL_SELECT_DIFFERENT_COMPANY, IDS_GBL_SELECT_DIFFERENT_COMPANY, IDS_GBL_SELECT_DIFFERENT_COMPANY)) return;
			if (!AddMenu(pFrame, pFrame->Menu[IDS_DEVELOPER_MENU], IDS_GBL_MNU_DENALI_ADMIN_UTILITIES, IDS_GBL_MNU_DENALI_ADMIN_UTILITIES, IDS_GBL_MNU_DENALI_ADMIN_UTILITIES)) return;
			if (!AddMenu(pFrame, pFrame->Menu[IDS_DEVELOPER_MENU], IDS_GBL_MNU_DENALI_CLIENT_UTILITIES, IDS_GBL_MNU_DENALI_CLIENT_UTILITIES, IDS_GBL_MNU_DENALI_CLIENT_UTILITIES)) return;
			if (!AddMenu(pFrame, pFrame->Menu[IDS_DEVELOPER_MENU], IDS_GBL_MNU_DENALI_SERVER_UTILITIES, IDS_GBL_MNU_DENALI_SERVER_UTILITIES, IDS_GBL_MNU_DENALI_SERVER_UTILITIES)) return;
			if (!AddMenu(pFrame, pFrame->Menu[IDS_DEVELOPER_MENU], IDS_GBL_MNU_DENALI_LICENSE_UTILITIES, IDS_GBL_MNU_DENALI_LICENSE_UTILITIES, IDS_GBL_MNU_DENALI_LICENSE_UTILITIES)) return;

			if (!AddSeparator(pFrame, pFrame->Menu[IDS_DEVELOPER_MENU])) return;

			if (!AddMenu(pFrame, pFrame->Menu[IDS_DEVELOPER_MENU], IDS_SHOW_XML_VIEWER, IDS_SHOW_XML_VIEWER, IDS_SHOW_XML_VIEWER)) return;
			if (!AddMenu(pFrame, pFrame->Menu[IDS_DEVELOPER_MENU], IDS_SHOW_XML_VIEWER_ON_ERROR, IDS_SHOW_XML_VIEWER_ON_ERROR, IDS_SHOW_XML_VIEWER_ON_ERROR)) return;
			if (!AddMenu(pFrame, pFrame->Menu[IDS_DEVELOPER_MENU], IDS_ASSERTONERROR, IDS_ASSERTONERROR, IDS_ASSERTONERROR)) return;
			if (!AddMenu(pFrame, pFrame->Menu[IDS_DEVELOPER_MENU], IDS_WNDSPY, IDS_WNDSPY, IDS_WNDSPY)) return;
			UINT Check;
			if (pFrame->si->ApplicationSettings.ShowXMLViewer) // change it to not show
				Check = MF_CHECKED;
			else // change it to show
				Check = MF_UNCHECKED;
			pFrame->Menu[IDS_DEVELOPER_MENU]->CheckMenuItem(IDS_SHOW_XML_VIEWER, Check | MF_BYCOMMAND);
			if (pFrame->si->ApplicationSettings.ShowXMLViewerOnError) // Show Check box
				Check = MF_CHECKED;
			else // change it to show
				Check = MF_UNCHECKED;
			pFrame->Menu[IDS_DEVELOPER_MENU]->CheckMenuItem(IDS_SHOW_XML_VIEWER_ON_ERROR, Check | MF_BYCOMMAND);
		}
#endif
	}
}

//------------------------------------------------------------------------------
CMenu* CCMSMainFrameSupportServices::MenuFindItemCMenu(ICMSMainFrameWnd* pFrame, CMenu* pParentMenu, UINT uMenuStringID)
{
	ASSERT_VALID(pParentMenu);
	CCMSMenuBuilder	objBuilder(pFrame->Frame);
	return objBuilder.GetMenu(pParentMenu, uMenuStringID, true);
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::EnableMenu(ICMSMainFrameWnd* pFrame, UINT nCommandID, bool bolEnable)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	CCMSMenuBuilder	objBuilder(pFrame->Frame);
	CMenu* pMenu = NULL;
	int								nPos = objBuilder.GetMenuItem(nCommandID, pMenu);

	if (!pMenu) nPos = objBuilder.GetMenuItem(nCommandID, pMenu, true);

	if (pMenu)
	{
		pMenu->EnableMenuItem((UINT)nPos, (UINT)(MF_BYPOSITION | (bolEnable ? MF_ENABLED : MF_GRAYED | MF_DISABLED)));
		pFrame->Frame->DrawMenuBar();
	}
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::EnableMenu(ICMSMainFrameWnd* pFrame, const CString& szMenuItem, bool bolEnable)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	typedef std::pair<int, CMenu*>		MenuPosition;
	typedef std::stack<MenuPosition>	MenuQueue;

	MenuQueue				queue;
	CCMSMenuBuilder	objBuilder(pFrame->Frame);
	CMenu* pMenu = &pFrame->MainMenu;
	CMenu* pSubMenu = NULL;
	if (!pMenu) return;

	CString		szItem(_T(""));
	for (int i = 0; i < pMenu->GetMenuItemCount(); i++)
	{
		pMenu->GetMenuString((UINT)i, szItem, MF_BYPOSITION);
		if (szItem == szMenuItem)
		{
			pMenu->EnableMenuItem((UINT)i, (bolEnable ? (UINT)(MF_BYPOSITION | MF_ENABLED) : (UINT)(MF_BYPOSITION | MF_GRAYED | MF_DISABLED)));
			pFrame->Frame->DrawMenuBar();
			return;
		}

		pSubMenu = pMenu->GetSubMenu(i);
		if (pSubMenu)		//this menu has a submenu; save where we left off at and search the submenu
		{
			MenuPosition	pos(i, pMenu);
			queue.push(pos);

			i = 0;
			pMenu = pSubMenu;
		}
		else if (!queue.empty() && i == pMenu->GetMenuItemCount() - 1)	//out of items on this menu; see if we left off searching a parent menu
		{
			MenuPosition	pos = queue.top();	//pop off the previous search position and continue searching
			queue.pop();

			i = pos.first;
			pMenu = pos.second;
		}
	}
}

//------------------------------------------------------------------------------
bool CCMSMainFrameSupportServices::AddMenu(ICMSMainFrameWnd* pFrame, CMenu* pParent, UINT nResourceID)
{
	ASSERT_VALID(pParent);

	COXBitmapMenu		menu;
	menu.CreatePopupMenu();

	const BOOL bolStatus = pParent->AppendMenu(MF_POPUP, (UINT_PTR)menu.Detach(), CGBLResources::GetResourceString(nResourceID));
	pFrame->Frame->DrawMenuBar();

	return bolStatus ? true : false;
}

//------------------------------------------------------------------------------
bool CCMSMainFrameSupportServices::AddMenu(ICMSMainFrameWnd* pFrame, CMenu* pMenu, UINT nCommandID, UINT nResourceID, UINT nStatusID)
{
	ASSERT(pFrame);
	ASSERT_VALID(pMenu);

	if (nStatusID) pFrame->StatusMessages.AddMessage(nCommandID, nStatusID);

	CString		szItem(CGBLResources::GetResourceString(nResourceID));
	::UpdateMenuString(szItem, nCommandID);

	const BOOL bolStatus = pMenu->AppendMenu(MF_STRING, nCommandID, szItem);
	pFrame->Frame->DrawMenuBar();

	return bolStatus ? true : false;
}

//------------------------------------------------------------------------------
bool CCMSMainFrameSupportServices::AddMenu(ICMSMainFrameWnd* pFrame, CMenu* pMenu, UINT nCommandID, UINT nResourceID, const CString& szStatus)
{
	ASSERT(pFrame);
	ASSERT_VALID(pMenu);

	pFrame->StatusMessages.AddMessage(nCommandID, szStatus);

	CString		szItem(CGBLResources::GetResourceString(nResourceID));
	::UpdateMenuString(szItem, nCommandID);

	const BOOL bolStatus = pMenu->AppendMenu(MF_STRING, nCommandID, szItem);
	pFrame->Frame->DrawMenuBar();

	return bolStatus ? true : false;
}

//------------------------------------------------------------------------------
bool CCMSMainFrameSupportServices::AddMenu(ICMSMainFrameWnd* pFrame, CMenu* pMenu, UINT nCommandID, const CString& szItem, UINT nStatusID)
{
	ASSERT(pFrame);
	ASSERT_VALID(pMenu);

	if (nStatusID) pFrame->StatusMessages.AddMessage(nCommandID, nStatusID);

	CString		szMenu(szItem);
	::UpdateMenuString(szMenu, nCommandID);

	const BOOL bolStatus = pMenu->AppendMenu(MF_STRING, nCommandID, szMenu);
	pFrame->Frame->DrawMenuBar();

	return bolStatus ? true : false;
}

//------------------------------------------------------------------------------
CString CCMSMainFrameSupportServices::UpdateMenuString(const CString& szMenuText, UINT nCommandID)
{
	CString szReturnText(szMenuText);
	::UpdateMenuString(szReturnText, nCommandID);
	return szReturnText;
}

//------------------------------------------------------------------------------
bool CCMSMainFrameSupportServices::AddSeparator(ICMSMainFrameWnd* pFrame, CMenu* pMenu)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);
	ASSERT_VALID(pMenu);

	const BOOL bolStatus = pMenu->AppendMenu(MF_SEPARATOR);
	pFrame->Frame->DrawMenuBar();

	return bolStatus ? true : false;
}

//------------------------------------------------------------------------------
bool CCMSMainFrameSupportServices::InsertSeparator(ICMSMainFrameWnd* pFrame, CMenu* pMenu, UINT nBeforeCommandID)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);
	ASSERT_VALID(pMenu);

	const BOOL bolStatus = pMenu->InsertMenu(nBeforeCommandID, MF_SEPARATOR | MF_BYCOMMAND);
	pFrame->Frame->DrawMenuBar();

	return bolStatus ? true : false;
}
//------------------------------------------------------------------------------
// used to create a menu subgroup (multi-batch mode batches)
bool CCMSMainFrameSupportServices::InsertMenu(ICMSMainFrameWnd* pFrame, CMenu* pParent, UINT nPos, UINT nResourceID, bool bByPosition)
{
	ASSERT_VALID(pParent);

	COXBitmapMenu		menu;
	menu.CreatePopupMenu();

	BOOL bolStatus = false;
	// BK 1/7/11 needed subgroup menu that uses the MF_BYPOSITION flag to use with fund FASB reports menu subgroup
	if (bByPosition)
		bolStatus = pParent->InsertMenu(nPos, MF_POPUP | MF_BYPOSITION, (UINT_PTR)menu.Detach(), CGBLResources::GetResourceString(nResourceID));
	else
		bolStatus = pParent->InsertMenu(nPos, MF_POPUP, (UINT_PTR)menu.Detach(), CGBLResources::GetResourceString(nResourceID));
	pFrame->Frame->DrawMenuBar();

	return bolStatus ? true : false;
}
//------------------------------------------------------------------------------
bool CCMSMainFrameSupportServices::InsertMenu(ICMSMainFrameWnd* pFrame, CMenu* pMenu, UINT nPos, UINT nCommandID, UINT nResourceID, UINT nStatusID)
{
	ASSERT(pFrame);
	ASSERT_VALID(pMenu);

	if (nStatusID) pFrame->StatusMessages.AddMessage(nCommandID, nStatusID);

	CString		szItem(CGBLResources::GetResourceString(nResourceID));
	::UpdateMenuString(szItem, nCommandID);

	const BOOL bolStatus = pMenu->InsertMenu(nPos, MF_STRING | MF_BYCOMMAND, nCommandID, szItem);
	pFrame->Frame->DrawMenuBar();

	return bolStatus ? true : false;
}

//------------------------------------------------------------------------------
bool CCMSMainFrameSupportServices::InsertMenu(ICMSMainFrameWnd* pFrame, CMenu* pMenu, UINT nPos, UINT nCommandID, const CString& szItem, const CString& szStatusString)
{
	ASSERT(pFrame);
	ASSERT_VALID(pMenu);

	if (!szStatusString.IsEmpty()) pFrame->StatusMessages.AddMessage(nCommandID, szStatusString);

	CString		szMenu(szItem);
	::UpdateMenuString(szMenu, nCommandID);

	const BOOL bolStatus = pMenu->InsertMenu(nPos, MF_STRING | MF_BYPOSITION, nCommandID, szMenu);
	pFrame->Frame->DrawMenuBar();

	return bolStatus ? true : false;
}

//------------------------------------------------------------------------------
bool CCMSMainFrameSupportServices::DeleteMenu(ICMSMainFrameWnd* pFrame, CMenu* pMenu, UINT nResourceID, bool bolString, bool bRemoveDuplicateSeparators)
{
	UNREFERENCED_PARAMETER(pFrame);
	ASSERT_VALID(pMenu);

	CCMSMenuBuilder	objBuilder(pFrame->Frame);
	CMenu* pTemp = NULL;
	const int nPos = objBuilder.GetMenuItem(pMenu, nResourceID, pTemp, bolString);
	bool	bolStatus = true;

	if (pTemp)
	{
		const UINT nCmdID = pMenu->GetMenuItemID(nPos);
		pFrame->StatusMessages.DeleteMessage((int)nCmdID);
		bolStatus = pMenu->DeleteMenu((UINT)nPos, MF_BYPOSITION) ? true : false;


		//if the delete was successful, check for consecutive separator lines and correct if found
		if (bolStatus && bRemoveDuplicateSeparators)
		{
			UINT nIdx = __max(0, (UINT)nPos - 1);
			UINT	nState = 0;
			bool	bolSeparator = false;

			// begbert 07-19-2010 VS2010 : Cast the return from GetMenuItemCount() to the expected UINT
			for (; nIdx < __min((UINT)pMenu->GetMenuItemCount(), (UINT)nPos + 2); )
			{
				nState = pMenu->GetMenuState(nIdx, MF_BYPOSITION);

				if (bolSeparator && (nState & MF_SEPARATOR) == MF_SEPARATOR)		//consecutive separators
				{
					pMenu->DeleteMenu(nIdx, MF_BYPOSITION);
					bolSeparator = false;
					continue;
				}
				else if ((nState & MF_SEPARATOR) == MF_SEPARATOR)						//possible consecutive separators
					bolSeparator = true;
				else																												//no consecutive separators found
					bolSeparator = false;

				nIdx++;
			}
		}

		pFrame->Frame->DrawMenuBar();
	}

	return bolStatus;
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::DisableToolbarButtons(ICMSMainFrameWnd* pFrame, bool bolDisable)
{
	if (!pFrame->ModuleTracker.get()) return;

	CToolBarCtrl& clsCtrl = pFrame->ToolBar->GetToolBarCtrl();
	CCMSModuleInfo::CCMSModuleData::const_iterator	it = CCMSModuleInfo::Instance().begin();

	for (; it != CCMSModuleInfo::Instance().end(); it++)
	{
		if (it->GetModuleName().IsEmpty()) continue;
		clsCtrl.EnableButton((int)it->GetModuleLaunchID(), FALSE);
	}
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::ToolBarAddPrintSaveClearButtonCluster(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	ToolBarAddToButtonArray(pFrame, IDS_PRINT, ICMSMainFrameWnd::enuTBPrintIdx);				// print button
	ToolBarAddToButtonArray(pFrame, IDS_SAVE, ICMSMainFrameWnd::enuTBSaveIdx);				// save button
	ToolBarAddToButtonArray(pFrame, IDS_CLEAR, ICMSMainFrameWnd::enuTBClearEntryIdx);		// clear button

	CCMSStatusBarMsgCollection& StatusMessages = pFrame->StatusMessages;
	StatusMessages.AddMessage(IDS_PRINT, IDS_STATUS_PRINT);	// adding status bar message for this button
	StatusMessages.AddMessage(IDS_SAVE, IDS_STATUS_SAVE);
	StatusMessages.AddMessage(IDS_CLEAR, IDS_STATUS_CLEAR);
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::ToolBarAddSeparatorButton(ICMSMainFrameWnd* pFrame)
{
	ToolBarAddToButtonArray(pFrame, 0, 0, NULL, TBSTYLE_SEP);
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::ToolBarAddModuleButtonCluster(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	CGBLSystemInformation* const	si = pFrame->si;

	// begbert 12-07-2006 1-26059 : Removed the conditionals around button addition.
	ToolBarAddToButtonArray(pFrame, IDS_LAUNCH_AP, ICMSMainFrameWnd::enuTBAcountsPayIdx, NULL, pFrame->ToolButtonStyle);	// AP accounts payable button
	ToolBarAddToButtonArray(pFrame, IDS_LAUNCH_AR, ICMSMainFrameWnd::enuTBAcountsRecvIdx, NULL, pFrame->ToolButtonStyle);	// AR accounts receivable button
	ToolBarAddToButtonArray(pFrame, IDS_LAUNCH_BR, ICMSMainFrameWnd::enuTBBankRecIdx, NULL, pFrame->ToolButtonStyle);	// BR bank reconciliation button
	ToolBarAddToButtonArray(pFrame, IDS_LAUNCH_GL, ICMSMainFrameWnd::enuTBGenLedgerIdx, NULL, pFrame->ToolButtonStyle);	// GL general ledger button
	ToolBarAddToButtonArray(pFrame, IDS_LAUNCH_IN, ICMSMainFrameWnd::enuTBInventoryIdx, NULL, pFrame->ToolButtonStyle);	// INV inventory button
	//	PBI 24421	04/21/2015	mvt - Set up Job Cost integration
	ToolBarAddToButtonArray(pFrame, IDS_LAUNCH_JC, ICMSMainFrameWnd::enuTBJobCostIdx, NULL, pFrame->ToolButtonStyle);	// JC job cost button
	ToolBarAddToButtonArray(pFrame, IDS_LAUNCH_PO, ICMSMainFrameWnd::enuTBPurchaseOrdIdx, NULL, pFrame->ToolButtonStyle);	// PO purchase order button
	// BK 1/16/12 added payroll
	ToolBarAddToButtonArray(pFrame, IDS_LAUNCH_PR, ICMSMainFrameWnd::enuTBPayrollIdx, NULL, pFrame->ToolButtonStyle);	// PR payroll button
	ToolBarAddToButtonArray(pFrame, IDS_LAUNCH_SA, ICMSMainFrameWnd::enuTBSalesMaintIdx, NULL, pFrame->ToolButtonStyle);	// SA sales maintenance button

	if (si->ApplicationSettings.StartupModule != CAppSettings::CMSCT)
		ToolBarAddToButtonArray(pFrame, IDS_CONTROLLER, ICMSMainFrameWnd::enuTBControllerIdx);	// controller button

	ToolBarAddSeparatorButton(pFrame);

	if (pFrame->si->ModulesInstalled.DenaliBI.Licensed &&
		si->ApplicationSettings.StartupModule != CAppSettings::DENALIBI)
	{
		ToolBarAddToButtonArray(pFrame, IDS_MENU_DENALIBI, ICMSMainFrameWnd::enuTBDenaliBI);
		pFrame->StatusMessages.AddMessage(IDS_MENU_DENALIBI, IDS_MODULE_BI);			// adding status bar message for this button
	}
}

//------------------------------------------------------------------------------
// 1-34642/1-34645 BK 8/19/10 added submenus to Cougar Dtails, financial & sales dashboards
void CCMSMainFrameSupportServices::ToolBarAddFinancialDashboardButton(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	ToolBarAddToButtonArray(pFrame, IDS_MENU_FINANCIAL_DASHBOARD, ICMSMainFrameWnd::enuTBCougarDtailsIdx);	// Cougar Dtails Financial button
	pFrame->StatusMessages.AddMessage(IDS_MENU_FINANCIAL_DASHBOARD, IDS_STATUS_FINANCIAL_DASHBOARD);		// adding status bar message for this button
}
//------------------------------------------------------------------------------
// 1-34642/1-34645 BK 8/19/10 added submenus to Cougar Dtails, financial & sales dashboards
void CCMSMainFrameSupportServices::ToolBarAddSalesDashboardButton(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	ToolBarAddToButtonArray(pFrame, IDS_MENU_SALES_DASHBOARD, ICMSMainFrameWnd::enuTBSalesDtailsIdx);	// Sales Dashboard button
	pFrame->StatusMessages.AddMessage(IDS_MENU_SALES_DASHBOARD, IDS_STATUS_SALES_DASHBOARD);			// adding status bar message for this button
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::ToolBarAddHelpButton(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	ToolBarAddToButtonArray(pFrame, IDS_HELP, ICMSMainFrameWnd::enuTBHelpIdx);	// help button
	pFrame->StatusMessages.AddMessage(IDS_HELP, IDS_STATUS_HELP);	// adding status bar message for this button
}

//------------------------------------------------------------------------------
void	CCMSMainFrameSupportServices::ToolBarAddToButtonArray(ICMSMainFrameWnd* pFrame, const int idCommand, const int nBitMapIndex, HICON hPic, BYTE fsStyle)
{
	const BYTE							bytState = ((fsStyle & TBSTYLE_BUTTON) == TBSTYLE_BUTTON ? (BYTE)TBSTATE_ENABLED : (BYTE)0);
	const CCMSTBButtonInfo	objBtnInfo(idCommand, nBitMapIndex, fsStyle, bytState);

	pFrame->ToolButtonInfo.AddButtonInfo(objBtnInfo);
	if (hPic) pFrame->ToolBarImages.Add(hPic);
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::ToolBarFinishSetup(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	static UINT		nButtonCnt = 0;
	TBBUTTON*& pButtonInfo = pFrame->ToolButtonInfoPointer;

	if (!pButtonInfo)
	{
		CCMSTBButtonInfoCollection& buttonInfo = pFrame->ToolButtonInfo;
		buttonInfo.Copy(pButtonInfo, nButtonCnt);
		buttonInfo.Empty();		//we don't need this anymore (I hope!), so free up the memory
	}

	if (::IsWindow(pFrame->ToolBar->GetSafeHwnd()))
	{
		pFrame->ToolBar->GetToolBarCtrl().AddButtons((int)nButtonCnt, pButtonInfo); // sending buttons to tool bar
		;
	}
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::ToolBarSetTitle(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	CGBLSystemInformation* const	si = pFrame->si;
	UINT unID = IDS_TOOLBAR_TITLE;					//generic toolbar title

	switch (si->ApplicationSettings.ModuleID)
	{
	case IDS_AP_MODULE:
		unID = IDS_ACCOUNTS_PAYABLE_TOOLBAR;
		break;
	case IDS_AR_MODULE:
		unID = IDS_ACCOUNTS_RECEIVABLE_TOOLBAR;
		break;
	case IDS_BR_MODULE:
		unID = IDS_BR_TOOLBAR;
		break;
	case IDS_CT_MODULE:
		unID = IDS_CT_TOOLBAR;
		break;
	case IDS_GL_MODULE:
		unID = IDS_GL_TOOLBAR;
		break;
	case IDS_IN_MODULE:
		unID = IDS_IN_TOOLBAR;
		break;
	case IDS_JC_MODULE:
		unID = IDS_JC_TOOLBAR;
		break;
	case IDS_PO_MODULE:
		unID = IDS_PO_TOOLBAR;
		break;
	case IDS_PR_MODULE:
		unID = IDS_PR_TOOLBAR;
		break;
	case IDS_SA_MODULE:
		unID = IDS_SA_TOOLBAR;
		break;
	default:
		TRACE("Resource string needed for toolbar - module id = %d\n", si->ApplicationSettings.ModuleID);
	}

	//set the toolbar's title.
	if (::IsWindow(pFrame->ToolBar->GetSafeHwnd())) pFrame->ToolBar->SetWindowText(CGBLResources::GetResourceString(unID));
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::ToolBarAdjustAppearance(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	CGBLSystemInformation* const	si = pFrame->si;

	if (!si->m_clsDynThemeManager.IsThemeActive())
	{
		if (::IsWindow(pFrame->ToolBar->GetSafeHwnd()))
		{
			SIZE sizeButton, sizeImage;
			sizeButton.cx = ::GetSystemMetrics(SM_CXSMICON);
			sizeButton.cy = ::GetSystemMetrics(SM_CYSMICON);

			//Defect ID 1-9250, adjusted button width for 800X600. PGP(01/13/2004)
			if (::GetSystemMetrics(SM_CXFULLSCREEN) <= 800) sizeButton.cx -= 2;

			sizeImage = sizeButton;
			sizeButton.cx = sizeImage.cx + 7;
			sizeButton.cy = sizeImage.cy + 6;

			pFrame->ToolBar->SetSizes(sizeButton, sizeImage);
			pFrame->ToolBar->GetToolBarCtrl().AutoSize();
			pFrame->ToolBar->UpdateSizes();
			pFrame->ToolBar->SetCustomizable(TRUE);
		}
	}

	if (::IsWindow(pFrame->ToolBar->GetSafeHwnd()))
	{
		// Version 3.0
		pFrame->ToolBar->IniSizes(CSize(25, 25));
		pFrame->Frame->ShowControlBar(pFrame->ToolBar, FALSE, FALSE);	//force the toolbar to resize and display all the added buttons
		pFrame->Frame->ShowControlBar(pFrame->ToolBar, TRUE, FALSE);
	}

	pFrame->Frame->RecalcLayout(FALSE);
}

//------------------------------------------------------------------------------
UINT CCMSMainFrameSupportServices::get_WindowCount(const ICMSMainFrameWnd* pFrame)
{
	UNREFERENCED_PARAMETER(pFrame);
	return GetWindowCount(false) + GetWindowCount(true);
}

//------------------------------------------------------------------------------
UINT CCMSMainFrameSupportServices::get_OpenWindowCount(const ICMSMainFrameWnd* pFrame)
{
	UNREFERENCED_PARAMETER(pFrame);
	return GetWindowCount(false);
}

//------------------------------------------------------------------------------
UINT CCMSMainFrameSupportServices::get_IconicWindowCount(const ICMSMainFrameWnd* pFrame)
{
	UNREFERENCED_PARAMETER(pFrame);
	return GetWindowCount(true);
}

//------------------------------------------------------------------------------
CString CCMSMainFrameSupportServices::GetLast8Batches(ICMSMainFrameWnd* pFrame, stuErrorReturn& clsErrorReturn, int intBatchType)
{
	UNREFERENCED_PARAMETER(pFrame);

	CXMLDocument xmlDoc(CMSStrings::XMLTags::Root);
	CXMLElement eleRoot = xmlDoc.GetDocumentElement();
	CXMLElement eleTable = eleRoot.AddNewChild(CMSStrings::XMLTags::TABLE);

	eleTable.AddNewChild(_T("strCompany"), CMS::GetSI()->CompanySettings.ID);
	eleTable.AddNewChild(_T("strLocation"), CMS::GetSI()->CompanySettings.DefaultLocation);
	eleTable.AddNewChild(_T("strModule"), CMS::GetSI()->ApplicationSettings.ModuleStringID);
	eleTable.AddNewChild(_T("strUser"), CMS::GetSI()->UserInformation.ID);
	eleTable.AddNewChild(_T("shtBatchType"), intBatchType);

	CCMSDataAdapter clsAdapter(CMSStrings::XMLTags::SPECIALPROCESS, IDS_GB_MODULE, _T("RecentBatches"));
	CCMSDataAdapterOptions clsOptions;

	clsOptions.m_szPrimaryCon = CMSStrings::XMLTags::Connection;
	clsOptions.m_szCompanyCon.Empty();
	clsOptions.m_szArchiveCon.Empty();

	// turn off the XML viewer if it has been turned on by the user.
	const bool b = clsOptions.m_clsServerInput.m_bShowViewer;
	clsOptions.m_clsServerInput.m_bShowViewer = false;

	CDataSet		ds;
	clsAdapter.GetData(xmlDoc, ds, clsOptions);
	clsOptions.m_clsServerInput.m_bShowViewer = b;	//restore the XML viewer

	return ds.GetXML();
}

//------------------------------------------------------------------------------
int CCMSMainFrameSupportServices::OnCreate(ICMSMainFrameWnd* pFrame, LPCREATESTRUCT lpCreateStruct)
{
	ASSERT(pFrame);

	if (!AttachOrganizer(pFrame))
		return -1;

	else if (!CreateMenu(pFrame))
		return -1;

	else if (!CreateToolBar(pFrame))
		return -1;

	else if (!CreateStatusBar(pFrame))
		return -1;

	// BK 11/1/10 changed dundas code to display 16x16 icons in menu - COXBitmapMenuOrganizer::AutoSetMenuImage()
	else if (!pFrame->MenuOrganizer.AutoSetMenuImage())
		return -1;

	InitModuleStatusMessages(pFrame);

	pFrame->Frame->EnableDocking(CBRS_ALIGN_ANY);
	if (::IsWindow(pFrame->ToolBar->GetSafeHwnd())) pFrame->Frame->DockControlBar(pFrame->ToolBar, AFX_IDW_DOCKBAR_TOP);

	return 0;
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::OnDestroy(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);

	pFrame->CMSMod->TerminateRefreshThread();
	pFrame->si->ClientLockRefresh.TerminateRefreshThread();
	CCMSWaitCursor::ExitStatusThread();

	pFrame->MenuOrganizer.Empty();
	pFrame->MenuOrganizer.DetachFrameWnd();

	PostQuitMessage(1);
}

//------------------------------------------------------------------------------
BOOL CCMSMainFrameSupportServices::OnNotify(ICMSMainFrameWnd* pFrame, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	LPNMHDR const		lpnm = (LPNMHDR)lParam;
	LPNMTOOLBAR const	lpnmTB = (LPNMTOOLBAR)lParam;
	LPNMHDR const		pwMessage = (NMHDR*)wParam;

	switch (lpnm->code)
	{
	case WM_SETTITLE:
		SetTitleText(pFrame);
		break;
	case WM_STATUSUPDATE:
		pFrame->StatusBar->Update();
		break;
	case WM_CHANGE_MODE:
		pFrame->StatusBar->SetMode((int)pwMessage->code);
		break;
	case WM_UPDATE_MODULE:
	{
		//Defect ID 1-28253, 1-28493, PGP(10-27-2007)
		RefreshFileMenuItems(pFrame);
		// begbert 05-04-2006 1-24248 : ensure that the toolbar is kept up-to-date with the modules.
		CCMSModuleInfo::Instance().SetMenuAccess(pFrame->ToolBar->GetToolBarCtrl());
		pFrame->CMSMod->RebuildRefreshXML();
	}
	break;
	case WM_STARTPROCESS:
		pFrame->SelectedProcess = (int)pwMessage->code;
		pFrame->TimerID = pFrame->Frame->SetTimer(1, 50, NULL);
		if (!pFrame->TimerID)
		{
			TRACE(_T("*** ERROR not able to set Timer in OnCreate\n"));
			return false;
		}
		break;
	case WM_TB_PRINT_ENABLE:
		pFrame->EnablePrint(pwMessage->code == 1);
		break;
	case WM_TB_SAVE_ENABLE:
		pFrame->EnableSave(pwMessage->code == 1);
		break;
	case WM_TB_CLEAR_ENABLE:
		pFrame->EnableClear(pwMessage->code == 1);
		break;
	case WM_UPDATE_ENABLE_FLAGS:
		pFrame->MenuSecurityDisableItems();
		break;
		//7796 9.10.2003 AES added so you can send a message from mod pref to update the menu when UDF are add or changed.
	case WM_UPDATE_MENU_UDFS:
		pFrame->UpdateMenu(false);
		break;
	case WM_NOTIFY_NAVIGATOR_INTERFACE:	// begbert 11-12-2010 1-35115 : Pass along the notification to the frame for handling.
		pFrame->NotifyNavigatorInterface((enuNavigatorNotifications)pwMessage->code);
		break;
		// 2/11/09 BK added button to turn of xml viewer and uncheck menu item
	case WM_TURN_OFF_XML_VIEWER:
		OnDeveloperShowXMLViewer(pFrame);
		break;

		// InTimeTec: 11.20.2012.HS - Custom Menus
	case WM_ADD_CUSTOM_MENU:
		//RCG - 09/12/2014 - Bug 8130/18883 - Do not create custom menu if no companies are installed
		if (!pFrame->GetNoCompaniesInstalled())
		{
			pFrame->CreateCustomMenu();
			pFrame->CustomMenuSecurityDisableItem();
			pFrame->Frame->DrawMenuBar();
		}
		break;
		// InTimeTec: End

	case TBN_DROPDOWN:	//drop down button was hit
		CCMSPoint	pos = CalculateToolbarMenuPos(pFrame, lpnmTB->iItem);
		return InitializeToolBarMenu(pFrame, pos, lpnmTB->iItem) ? TRUE : FALSE;	//false indicates the TBN_DROPDOWN notification was handled.
	}

	return TRUE;
}

//------------------------------------------------------------------------------
BOOL CCMSMainFrameSupportServices::OnToolTipNotify(ICMSMainFrameWnd* pFrame, UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREFERENCED_PARAMETER(pFrame);

	NMTTDISPINFO* const pToolTipText = (NMTTDISPINFO*)pNMHDR;

	const UINT nID = (UINT)pNMHDR->idFrom;
	CString szToolTipText(_T(""));

	if (nID >= MENU_IDRANGE_START && nID < IDS_LAUNCH_AR)
		szToolTipText = CGBLResources::GetResourceString(IDS_MODULE_AP);
	else if (nID >= IDS_LAUNCH_AR && nID < IDS_LAUNCH_BR)
		szToolTipText = CGBLResources::GetResourceString(IDS_MODULE_AR);
	else if (nID >= IDS_LAUNCH_BR && nID < IDS_LAUNCH_DX)
		szToolTipText = CGBLResources::GetResourceString(IDS_MODULE_BR);
	else if (nID >= IDS_LAUNCH_DX && nID < IDS_LAUNCH_GL)
		szToolTipText = CGBLResources::GetResourceString(IDS_MODULE_DX);
	else if (nID >= IDS_LAUNCH_GL && nID < IDS_LAUNCH_IN)
		szToolTipText = CGBLResources::GetResourceString(IDS_MODULE_GL);
	else if (nID >= IDS_LAUNCH_IN && nID < IDS_LAUNCH_JC)
		szToolTipText = CGBLResources::GetResourceString(IDS_MODULE_IN);
	else if (nID >= IDS_LAUNCH_JC && nID < IDS_LAUNCH_PR)
		szToolTipText = CGBLResources::GetResourceString(IDS_MODULE_JC);
	else if (nID >= IDS_LAUNCH_PR && nID < IDS_LAUNCH_PO)
		szToolTipText = CGBLResources::GetResourceString(IDS_MODULE_PR);
	else if (nID >= IDS_LAUNCH_PO && nID < IDS_LAUNCH_SA)
		szToolTipText = CGBLResources::GetResourceString(IDS_MODULE_PO);
	else if (nID >= IDS_LAUNCH_SA && nID < MENU_IDRANGE_END)
		szToolTipText = CGBLResources::GetResourceString(IDS_MODULE_SA);
	else
		szToolTipText = CGBLResources::GetResourceString(nID);

	_tcsncpy_s(pToolTipText->szText, 78, szToolTipText.GetString(), (size_t)szToolTipText.GetLength());

	return (FALSE);
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::OnInitMenuPopup(ICMSMainFrameWnd* pFrame, CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	if (bSysMenu) return;

	CMenu* pMenu = pFrame->Menu[IDS_FILE];
	if (pPopupMenu == pMenu) *pFrame->ModuleTracker >> pMenu;	//insert menu items for all the open modules into the file menu

	pMenu = pFrame->Menu[IDS_MENU_WINDOW];
	if (pMenu == pPopupMenu) CCMSWindowManager::Instance() >> pMenu;	//insert menu items for all open windows into the window menu

	pFrame->MenuOrganizer.OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::OnCmdUIEnableMenu(ICMSMainFrameWnd* pFrame, CCmdUI* pCmdUI, UINT nBaseMenuID)
{
	ASSERT(pFrame);
	ASSERT(pCmdUI);

	if (!pFrame->ModuleTracker.get() || !pCmdUI->m_pParentMenu->GetSafeHmenu())
		return;
	else if (pCmdUI->m_nID == nBaseMenuID)
		pCmdUI->Enable(CCMSModuleInfo::Instance().CanLaunch(nBaseMenuID));
	else
		EnableModuleMenuItem(pFrame, pCmdUI, nBaseMenuID);
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::OnTimer(ICMSMainFrameWnd* pFrame, UINT_PTR unIDEvent)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	if (unIDEvent == 1)
	{
		const int nFormProcess = pFrame->SelectedProcess;
		pFrame->SelectedProcess = 0;

		pFrame->Frame->KillTimer(pFrame->TimerID);

		if (nFormProcess - GOTO_SETUPBATCH > 0)
		{
			pFrame->SelectedProcess = nFormProcess - GOTO_SETUPBATCH;
			pFrame->StartProcess(GOTO_SETUPBATCH);
		}
		else
			pFrame->StartProcess(nFormProcess);
	}
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::OnEnableFile(ICMSMainFrameWnd* pFrame, CCmdUI* pCmdUI)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);
	ASSERT(pCmdUI);

	if (!pCmdUI->m_pMenu) return;
	*pFrame->ModuleTracker >> pCmdUI->m_pMenu;
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::OnEnableSaveButton(ICMSMainFrameWnd* pFrame, CCmdUI* pCmdUI, bool& bolCanAccess)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);
	ASSERT(pCmdUI);

	//JKC DefectID: 1-8870, 1-8865 Update ToolBar based on formview buttons
	if (::IsWindow(::AfxGetMainWnd()->GetSafeHwnd()))
	{
		CView* const pView = CGBLForm::EkGetActiveView();
		if (pView && ::IsWindow(pView->GetSafeHwnd()))
		{
			CCMSFormView* const pCMSWnd = DYNAMIC_DOWNCAST(CCMSFormView, pView);
			CPrintCodes* const pPCWnd = DYNAMIC_DOWNCAST(CPrintCodes, pView);

			if (pCMSWnd && ::IsWindow(pCMSWnd->GetSafeHwnd()))
				bolCanAccess = (pCMSWnd->IsSaveEnabled() ? true : false);

			else if (pPCWnd && ::IsWindow(pPCWnd->GetSafeHwnd()))
				bolCanAccess = (pPCWnd->IsSaveEnabled() ? true : false);

			else
				bolCanAccess = false;
		}
		else
			bolCanAccess = false;
	}

	pCmdUI->Enable(bolCanAccess);
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::OnEnablePrintButton(ICMSMainFrameWnd* pFrame, CCmdUI* pCmdUI, bool& bolCanAccess)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);
	ASSERT(pCmdUI);

	//JKC DefectID: 1-8870, 1-8865 Update ToolBar based on formview buttons
	if (::IsWindow(::AfxGetMainWnd()->GetSafeHwnd()))
	{
		CView* const pView = CGBLForm::EkGetActiveView();
		if (pView && ::IsWindow(pView->GetSafeHwnd()))
		{
			CCMSFormView* const pCMSWnd = DYNAMIC_DOWNCAST(CCMSFormView, pView);
			CPrintCodes* const pPCWnd = DYNAMIC_DOWNCAST(CPrintCodes, pView);

			if (pCMSWnd && ::IsWindow(pCMSWnd->GetSafeHwnd()))
				bolCanAccess = (pCMSWnd->IsPrintEnabled() ? true : false);

			else if (pPCWnd && ::IsWindow(pPCWnd->GetSafeHwnd()))
				bolCanAccess = (pPCWnd->IsPrintEnabled() ? true : false);

			else
				bolCanAccess = false;
		}
		else
			bolCanAccess = false;
	}

	pCmdUI->Enable(bolCanAccess);
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::OnEnableClearButton(ICMSMainFrameWnd* pFrame, CCmdUI* pCmdUI, bool& bolCanAccess)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);
	ASSERT(pCmdUI);

	//JKC DefectID: 1-8870, 1-8865 Update ToolBar based on formview buttons
	if (::IsWindow(::AfxGetMainWnd()->GetSafeHwnd()))
	{
		CView* const pView = CGBLForm::EkGetActiveView();
		if (pView && ::IsWindow(pView->GetSafeHwnd()))
		{
			CCMSFormView* const pCMSWnd = DYNAMIC_DOWNCAST(CCMSFormView, pView);
			CPrintCodes* const pPCWnd = DYNAMIC_DOWNCAST(CPrintCodes, pView);

			if (pCMSWnd && ::IsWindow(pCMSWnd->GetSafeHwnd()))
				bolCanAccess = (pCMSWnd->IsClearEnabled() ? true : false);

			else if (pPCWnd && ::IsWindow(pPCWnd->GetSafeHwnd()))
				bolCanAccess = (pPCWnd->IsClearEnabled() ? true : false);

			else
				bolCanAccess = false;
		}
		else
			bolCanAccess = false;
	}

	pCmdUI->Enable(bolCanAccess);
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::OnEnableArrangeOpenWindows(ICMSMainFrameWnd* pFrame, CCmdUI* pCmdUI)
{
	UNREFERENCED_PARAMETER(pFrame);
	ASSERT(pCmdUI);

	const UINT nWindowCnt = GetWindowCount();
	pCmdUI->Enable(nWindowCnt > 0);
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::OnEnableArrangeIconicWindows(ICMSMainFrameWnd* pFrame, CCmdUI* pCmdUI)
{
	UNREFERENCED_PARAMETER(pFrame);
	ASSERT(pCmdUI);

	const UINT nIconicWindowCnt = GetWindowCount(true);
	pCmdUI->Enable(nIconicWindowCnt > 0);
}

//------------------------------------------------------------------------------
void CCMSMainFrameSupportServices::OnEnableCloseAll(ICMSMainFrameWnd* pFrame, CCmdUI* pCmdUI)
{
	UNREFERENCED_PARAMETER(pFrame);
	ASSERT(pCmdUI);

	const UINT nWindowCnt = GetWindowCount();
	const UINT nIconicWindowCnt = GetWindowCount(true);
	pCmdUI->Enable((nWindowCnt + nIconicWindowCnt) > 0);
}

//------------------------------------------------------------------------------
void	CCMSMainFrameSupportServices::OnUpdateDisplayToolbarMenu(ICMSMainFrameWnd* pFrame, CCmdUI* pCmdUI)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);
	ASSERT(pCmdUI);

	if (::IsWindowVisible(pFrame->ToolBar->GetSafeHwnd())) // toolbar is visible
		pFrame->MainMenu.CheckMenuItem(IDS_DISPLAY_TOOLBAR, MF_CHECKED | MF_BYCOMMAND);
	else
		pFrame->MainMenu.CheckMenuItem(IDS_DISPLAY_TOOLBAR, MF_UNCHECKED | MF_BYCOMMAND);
}

//------------------------------------------------------------------------------
void	CCMSMainFrameSupportServices::OnAppExit(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	if (CloseWindows())
	{
		pFrame->ModuleTracker->Close();
		pFrame->Frame->PostMessage(WM_CLOSE);
	}
}

//------------------------------------------------------------------------------
void	CCMSMainFrameSupportServices::OnFileSave(ICMSMainFrameWnd* pFrame)
{
	UNREFERENCED_PARAMETER(pFrame);

	CMaintFormView* const pWnd = DYNAMIC_DOWNCAST(CMaintFormView, CGBLForm::EkGetActiveView());
	CCMSFormView* const pCMSWnd = DYNAMIC_DOWNCAST(CCMSFormView, CGBLForm::EkGetActiveView());

	if (pWnd)
	{
		if (pWnd->GetControlButtons())													// check if the cms control btn are being used
			pWnd->GetControlButtons()->OnBnClickedBtnSave();			// do to btn clk for the button

		else if (pWnd->m_dlgCMSReportButtons)									// check if the cms control btn are being used
			pWnd->m_dlgCMSReportButtons.OnBnClickedGblBtnOk();	// do to btn clk for the button
	}
	else if (pCMSWnd && pCMSWnd->m_dlgMiscButtons)
		pCMSWnd->m_dlgMiscButtons.OnBnClickedButton1();
}

//------------------------------------------------------------------------------
void	CCMSMainFrameSupportServices::OnFileClear(ICMSMainFrameWnd* pFrame)
{
	UNREFERENCED_PARAMETER(pFrame);

	// TODO: Add your command handler code here
	CMaintFormView* const pWnd = DYNAMIC_DOWNCAST(CMaintFormView, CGBLForm::EkGetActiveView());

	if (pWnd && pWnd->GetControlButtons())										// check if the cms control btn are being used
		pWnd->GetControlButtons()->OnBnClickedBtnClear();				// do to btn clk for the button
}

//------------------------------------------------------------------------------
void	CCMSMainFrameSupportServices::OnFilePrint(ICMSMainFrameWnd* pFrame)
{
	UNREFERENCED_PARAMETER(pFrame);

	CView* const pActiveView = CGBLForm::EkGetActiveView();		//GET handle to veiw

	CMaintFormView* const pMainWnd = DYNAMIC_DOWNCAST(CMaintFormView, pActiveView);
	CReportFormView* const pReportWnd = DYNAMIC_DOWNCAST(CReportFormView, pActiveView);
	CCMSFormView* const pCMSWnd = DYNAMIC_DOWNCAST(CCMSFormView, pActiveView);
	CPrintCodes* const pPCWnd = DYNAMIC_DOWNCAST(CPrintCodes, pActiveView);

	//check if the cms control btn are being used
	if (pMainWnd && ::IsWindow(pMainWnd->GetControlButtons()->GetSafeHwnd()))
		pMainWnd->GetControlButtons()->OnBnClickedBtnPrint();

	else if (pReportWnd && ::IsWindow(pReportWnd->m_dlgCMSReportButtons.GetSafeHwnd()))
		pReportWnd->m_dlgCMSReportButtons.OnBnClickedGblBtnOk();

	else if (pCMSWnd && ::IsWindow(pCMSWnd->m_dlgMiscButtons.GetSafeHwnd()))
		pCMSWnd->m_dlgMiscButtons.PrintFromToolbar();

	else if (pPCWnd && ::IsWindow(pPCWnd->m_DialogActionButtons.GetSafeHwnd()))
		pPCWnd->m_DialogActionButtons.OnBnClickedGblBtnOk();
}

//------------------------------------------------------------------------------
void	CCMSMainFrameSupportServices::OnHelp(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	// begbert 01-11-2011 FUND : Use the fund core help file if necessary.
	pFrame->si->HelpSettings.HelpFileName = pFrame->si->CompanySettings.IsFund ? HF_FUNDCORE : HF_ACTCORE;
	CGBLHelp::ShowHelpTableOfContents(pFrame->si);
}

//------------------------------------------------------------------------------
void	CCMSMainFrameSupportServices::OnWindowCloseAll(ICMSMainFrameWnd* pFrame)
{
	UNREFERENCED_PARAMETER(pFrame);

	CloseWindows();
}

//------------------------------------------------------------------------------
void	CCMSMainFrameSupportServices::OnHelpContents(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	// begbert 01-11-2011 FUND : Use the fund core help file if necessary.
	pFrame->si->HelpSettings.HelpFileName = pFrame->si->CompanySettings.IsFund ? HF_FUNDCORE : HF_ACTCORE;
	CGBLHelp::ShowHelpTableOfContents(pFrame->si);
}

//------------------------------------------------------------------------------
void	CCMSMainFrameSupportServices::OnDeveloperShowXMLViewer(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	if (pFrame->si->ApplicationSettings.ShowXMLViewer) // change it to not show
	{
		pFrame->si->ApplicationSettings.ShowXMLViewer = false;
		pFrame->Menu[IDS_DEVELOPER_MENU]->CheckMenuItem(IDS_SHOW_XML_VIEWER, MF_UNCHECKED | MF_BYCOMMAND);
	}
	else // change it to show
	{
		pFrame->si->ApplicationSettings.ShowXMLViewer = true;
		pFrame->Menu[IDS_DEVELOPER_MENU]->CheckMenuItem(IDS_SHOW_XML_VIEWER, MF_CHECKED | MF_BYCOMMAND);
	}
}

//------------------------------------------------------------------------------
void	CCMSMainFrameSupportServices::OnDeveloperShowXMLViewerOnError(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	if (pFrame->si->ApplicationSettings.ShowXMLViewerOnError) // change it to not show
	{
		pFrame->si->ApplicationSettings.ShowXMLViewerOnError = false;
		pFrame->Menu[IDS_DEVELOPER_MENU]->CheckMenuItem(IDS_SHOW_XML_VIEWER_ON_ERROR, MF_UNCHECKED | MF_BYCOMMAND);
	}
	else // change it to show
	{
		pFrame->si->ApplicationSettings.ShowXMLViewerOnError = true;
		pFrame->Menu[IDS_DEVELOPER_MENU]->CheckMenuItem(IDS_SHOW_XML_VIEWER_ON_ERROR, MF_CHECKED | MF_BYCOMMAND);
	}
}

//------------------------------------------------------------------------------
void	CCMSMainFrameSupportServices::OnSelectDifferentCompany(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	const bool bResult = CGBLFormWrap::ShowCompanyLogin();

	if (!bResult && !pFrame->si->SkipLogin && pFrame->si->CompanySettings.ID.IsEmpty())
		CGBLForm::CMSMessageBox(pFrame->Frame, IDS_FAILED_LOGIN, IDS_INVALID_LOGIN, MB_OK | MB_ICONERROR);
}

//------------------------------------------------------------------------------
void	CCMSMainFrameSupportServices::OnDeveloperShowWndSpy(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	if (pFrame->WindowSpy && pFrame->si->ApplicationSettings.ShowWndSpy)
		pFrame->WindowSpy->SetFocus();
	else
	{
		pFrame->si->ApplicationSettings.ShowWndSpy = true;
		CGBL_DLG_WndSpy* const pWndSpy = new CGBL_DLG_WndSpy();
		pWndSpy->Create(CGBL_DLG_WndSpy::IDD, pFrame->Frame);
		pWndSpy->ShowWindow(SW_SHOW);

		pFrame->WindowSpy = pWndSpy;
	}
}

//------------------------------------------------------------------------------
void	CCMSMainFrameSupportServices::OnDeveloperAssertOnError(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	// TODO: Add your command handler code here
	if (pFrame->si->CommandLineSettings.PO_AssertOnError) // change it to not show
	{
		pFrame->si->CommandLineSettings.PO_AssertOnError = false;
		pFrame->Menu[IDS_DEVELOPER_MENU]->CheckMenuItem(IDS_ASSERTONERROR, MF_UNCHECKED | MF_BYCOMMAND);
	}
	else // change it to show
	{
		pFrame->si->CommandLineSettings.PO_AssertOnError = true;
		pFrame->Menu[IDS_DEVELOPER_MENU]->CheckMenuItem(IDS_ASSERTONERROR, MF_CHECKED | MF_BYCOMMAND);
	}
}

//------------------------------------------------------------------------------
void	CCMSMainFrameSupportServices::OnOptionsStayInAddModeOnSave(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	//	PBI 38564	02/08/2017	mvt - Require Stay In Add Mode On Save to be checked for Generate Customer Number On Save in AR.
	if (pFrame->si->ApplicationSettings.IsAR() && pFrame->si->ModuleSettings.ARSettings.GenerateCustomerNumberOnSave)
	{
		pFrame->si->ApplicationSettings.StayInAddModeOnSave = true;
		pFrame->MainMenu.CheckMenuItem(IDS_STAY_IN_ADD_MODE_ON_SAVE, MF_CHECKED | MF_BYCOMMAND);
		return;
	}

	// checking / unchecking menu item and setting global TND
	if (pFrame->si->ApplicationSettings.StayInAddModeOnSave)
	{
		pFrame->si->ApplicationSettings.StayInAddModeOnSave = false; // uncheck
		pFrame->MainMenu.CheckMenuItem(IDS_STAY_IN_ADD_MODE_ON_SAVE, MF_UNCHECKED | MF_BYCOMMAND);
	}
	else
	{
		pFrame->si->ApplicationSettings.StayInAddModeOnSave = true; // check
		pFrame->MainMenu.CheckMenuItem(IDS_STAY_IN_ADD_MODE_ON_SAVE, MF_CHECKED | MF_BYCOMMAND);
	}
}

//------------------------------------------------------------------------------
void	CCMSMainFrameSupportServices::OnDisplayToolbar(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	if (::IsWindowVisible(pFrame->ToolBar->GetSafeHwnd())) // toolbar is visible
	{
		pFrame->MainMenu.CheckMenuItem(IDS_DISPLAY_TOOLBAR, MF_UNCHECKED | MF_BYCOMMAND);
		pFrame->Frame->ShowControlBar(pFrame->ToolBar, FALSE, FALSE);
	}
	else
	{
		pFrame->MainMenu.CheckMenuItem(IDS_DISPLAY_TOOLBAR, MF_CHECKED | MF_BYCOMMAND);
		pFrame->Frame->ShowControlBar(pFrame->ToolBar, TRUE, FALSE);
	}

	pFrame->Frame->RecalcLayout(FALSE);
}

//------------------------------------------------------------------------------
void	CCMSMainFrameSupportServices::OnDisplayStatusbar(ICMSMainFrameWnd* pFrame)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	if (::IsWindowVisible(pFrame->StatusBar->GetSafeHwnd())) // Statusbar is visible
	{
		pFrame->MainMenu.CheckMenuItem(IDS_MENU_GL_DISPLAY_STATUSBAR, MF_UNCHECKED | MF_BYCOMMAND);
		pFrame->Frame->ShowControlBar(pFrame->StatusBar, FALSE, FALSE);
	}
	else
	{
		pFrame->MainMenu.CheckMenuItem(IDS_MENU_GL_DISPLAY_STATUSBAR, MF_CHECKED | MF_BYCOMMAND);
		pFrame->Frame->ShowControlBar(pFrame->StatusBar, TRUE, FALSE);
	}

	pFrame->Frame->RecalcLayout(FALSE);
}

//------------------------------------------------------------------------------
LRESULT CCMSMainFrameSupportServices::OnShowHelp(ICMSMainFrameWnd* pFrame, WPARAM wParam, LPARAM lParam)
{
	ASSERT(pFrame);

	if ((::GetKeyState(VK_CONTROL) & 0x80) || (::GetAsyncKeyState(VK_SHIFT) & 0x80)) return 0;	//prevent help from being launched if the user presses ctrl-F1
	CGBLHelp::ShowContextHelp(pFrame->si);
	return 0;
}

//------------------------------------------------------------------------------
LRESULT CCMSMainFrameSupportServices::OnSetupLocalization(ICMSMainFrameWnd* pFrame, WPARAM wParam, LPARAM lParam)
{
	ASSERT(pFrame);

	COXCurrencyEdit::stuLocalization* const pObj = ((COXCurrencyEdit::stuLocalization*)wParam);

	pObj->szDecimalSeparator = pFrame->si->Localization.CurrencyDecimalSeparator;
	pObj->szGroupSeparator = pFrame->si->Localization.CurrencyGroupSeparator;

	CString szBuffer(_T(""));
	const int nLength = ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ILZERO, NULL, 0);

	if (nLength)
	{
		::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ILZERO, szBuffer.GetBufferSetLength(nLength + 1), nLength);
		szBuffer.ReleaseBuffer();
		pObj->bShowLeadingZero = (_ttoi(szBuffer) > 1 ? true : false);
	}

	//PGP(08/25/2004) - Check if we have an entry before accessing
	pObj->nGroupLength = 3;//fallback value if something fails.
	if (pFrame->si->Localization.CurrencyGroupSizes.GetCount() > 0)
		pObj->nGroupLength = _tstoi(pFrame->si->Localization.CurrencyGroupSizes.GetAt(0));

	pObj->nFractionalDigitCount = pFrame->si->Localization.CurrencyDecimalDigits;
	pObj->sCurrency = pFrame->si->Localization.CurrencySymbol;
	pObj->sFormatPositive = pFrame->si->Localization.CurrencyPositiveTemplate;
	pObj->sFormatNegative = pFrame->si->Localization.CurrencyNegativeTemplate;

	return	TRUE;
}

//------------------------------------------------------------------------------
LRESULT CCMSMainFrameSupportServices::OnWaitStringChange(ICMSMainFrameWnd* pFrame, WPARAM wParam, LPARAM lParam)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	try
	{
		if (pFrame->si->WaitMessageDetails.lWaitCount > 0)
		{

			if (pFrame->si->WaitMessageDetails.unMessageID > 0)
			{
				CString	 szMessage = CGBLResources::GetResourceString(pFrame->si->WaitMessageDetails.unMessageID);
				pFrame->Frame->SetMessageText(szMessage);
			}
			else
				pFrame->Frame->SetMessageText(pFrame->si->WaitMessageDetails.szMessage);
		}
		else
			pFrame->Frame->SetMessageText(AFX_IDS_IDLEMESSAGE);	//Show the ready message
	}
	catch (...) {};

	return 0;
}

//------------------------------------------------------------------------------
LRESULT CCMSMainFrameSupportServices::OnWaitStringChange1(ICMSMainFrameWnd* pFrame, WPARAM wParam, LPARAM lParam)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	if (lParam <= 0) return 0;

	const CString	szText((TCHAR*)wParam, (int)lParam);
	pFrame->Frame->SetMessageText(szText);

	return 0;
}

//------------------------------------------------------------------------------
// begbert 12-12-2014 PBI 7976 : This function only restores the window from the taskbar, because an app can't give itself focus.
LRESULT	CCMSMainFrameSupportServices::OnEnsureAppVisible(ICMSMainFrameWnd* pFrame, WPARAM wParam, LPARAM lParam)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	if (::IsIconic(pFrame->Frame->GetSafeHwnd())) pFrame->Frame->ShowWindow(SW_RESTORE);

	return (LRESULT)WM_CMS_FIND_APP_SUCCESS;
}

//------------------------------------------------------------------------------
LRESULT	CCMSMainFrameSupportServices::OnStartupCompleted(ICMSMainFrameWnd* pFrame, WPARAM wParam, LPARAM lParam)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	AddModuleGUIItems(pFrame);

	ToolBarAddFinancialDashboardButton(pFrame);								// Cougar Dtails button		begbert 02-08-2010 CDtails : Added this
	ToolBarAddSalesDashboardButton(pFrame);
	CCMSMainFrameSupportServices::ToolBarAddSeparatorButton(pFrame);
	ToolBarAddHelpButton(pFrame);								// help button
	ToolBarFinishSetup(pFrame);									// commits changes to toolbar
	ToolBarSetTitle(pFrame);									// set the module specific toolbar title
	ToolBarAdjustAppearance(pFrame);							//Defect ID 1-9250, PGP(12/09/2003)

	// BK 11/1/10 changed dundas code to display 16x16 icons in menu - COXBitmapMenuOrganizer::AutoSetMenuImage()
	pFrame->MenuOrganizer.AutoSetMenuImage();

	// begbert 11-17-2010 1-35114 : To correct a hard-to-reproduce problem with the
	//   toolbar sizing, force a last-minute AutoSize() in case it happens to need it.
	if (::IsWindow(pFrame->ToolBar->m_hWnd))
		pFrame->ToolBar->GetToolBarCtrl().AutoSize();

	// begbert 11-08-2010 1-34988 : redraw the frame window to clear some erroneous
	//    image data left in the non-client area around the toolbar.
	pFrame->MDIFrame->RedrawWindow();

	CCMSModuleInfo::Instance().SetMenuAccess(pFrame->ToolBar->GetToolBarCtrl());

	//auto_ptr::reset will destroy the existing object (if it has one) and then set it's pointer to the new instance
	if (!pFrame->ModuleTracker.get() || *pFrame->ModuleTracker != pFrame->si->CompanySettings.ID)
		pFrame->ModuleTracker.reset(CCMSModuleTracker::New(pFrame->Frame->GetSafeHwnd()));

	return (LRESULT)1;
}

//------------------------------------------------------------------------------
void	CCMSMainFrameSupportServices::GetMessageString(ICMSMainFrameWnd* pFrame, UINT nID, CString& rMessage)
{
	ASSERT(pFrame);
	ASSERT_VALID(pFrame->Frame);

	if (pFrame->StatusMessages.MessageExists((int)nID))
	{
		const CCMSStatusBarMsg& objMsg = pFrame->StatusMessages.GetMessage((int)nID);

		if (objMsg.Resource)
			rMessage = CGBLResources::GetResourceString(objMsg.Resource);
		else
			rMessage = objMsg.Message;

		rMessage.Replace(_T("&"), _T(""));
	}
}
