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
 * Initial code - sbarber
 ******************************************************************************/
#pragma once

#include <deque>
#include <map>
#include "..\dundasinclude\CMSDundasConst.h"
#include "..\dundasinclude\OXCoolToolBar.h"
#include "..\cmsinclude\CMSStatusBar.h"
#include "..\cmsdll\GBLConstants.h"
#include "..\cmsdll\CMSControlSubjectObserver.h"
#include "..\cmsdll\CMSModuleRefresh.h"
#include "..\cmsdll\GBLNetDll.h"
#include "..\cmsdll\ICMSFrameWnd.h"
#include "..\cmsdll\CMSStatusBarMsg.h"
#include "..\cmsdll\CMSGBLDataHolder.h"
#include "..\gblforms\CMSMenuBuilder.h"
#include "..\gblforms\GBL_DLG_WndSpy.h"
#include "..\gblforms\CMSWindowManager.h"
#include "..\gblforms\CMSMainFrameSupportServices.h"

#define BUTTON_STYLE(p1, p2)	static BYTE Get##p1##Style(void) { return p2; }

class CGBLSystemInformation;

template<typename T, typename TRAITS = CCMSMainFrameSupportServices>
class CCMSBaseMainFrameWnd
	: public T
	, public CCMSObserver
	, public CCMSSubject
	, public ICMSMainFrameWnd
	, public ICMSStatusBarHandler
{
	DECLARE_CLASS_NOCOPY(CCMSBaseMainFrameWnd)

	typedef typename T																						ConcreteBaseClass;
	typedef typename TRAITS																				traits;
	typedef typename CCMSBaseMainFrameWnd<ConcreteBaseClass, traits>	self;
	typedef std::map<UINT, CString>																	BatchCommandCollection;	//command id for a batch and the batch id
	typedef std::map<ICMSMainFrameWnd::enmStandardGUIItem, bool>				GUIAccessMap;

	friend typename traits;

public:
	//creates a static method called GetStdToolBtnStyle that returns TBSTYLE_BUTTON | TBSTYLE_DROPDOWN
	BUTTON_STYLE(StdToolBtn, TBSTYLE_BUTTON | TBSTYLE_DROPDOWN)

		CCMSBaseMainFrameWnd(void);
	virtual ~CCMSBaseMainFrameWnd(void) { CCMSGBLDataHolder::Instance().Dispose(); }

	virtual ICMSMainFrameWnd *	get_BaseFrameWnd(void);
	virtual CFrameWnd *				get_FrameWnd(void);
	virtual CMDIFrameWnd *			get_MDIFrameWnd(void);

	virtual void UpdateMenu(bool bStartUp = false);
	virtual void ShowWndInfo(CWnd * pWnd);

	__declspec(property(get = get_BaseFrameWnd))	ICMSMainFrameWnd *	BaseFrame;
	__declspec(property(get = get_FrameWnd))			CFrameWnd *				Frame;
	__declspec(property(get = get_MDIFrameWnd))		CMDIFrameWnd *			MDIFrame;

protected:
	virtual bool CreateMenu(void) = 0;

	virtual void Update(CCMSStatusBar& objStatusBar) { };
	virtual void MenuSecurityDisableItems(void);
	virtual void DisableToolbarButtons(bool bolDisable = true);
	virtual void UpdateEnableFlags(void) { };
	virtual void CreateModuleToolBar(void) = 0;
	virtual void EnablePrint(bool bolEnable);
	virtual void EnableSave(bool bolEnable);
	virtual void EnableClear(bool bolEnable);
	virtual void EnableHelp(bool bolEnable);
	virtual void AddStatusBarMessage(UINT nMenuItemID, const CString& szStatusString);
	virtual void AddStatusBarMessage(UINT nMenuItemID, UINT nResourceID);
	virtual void ToolBarAddToButtonArray(int nResourceID, int nButtonIdx, HICON hPic, BYTE fsStyle = TBSTYLE_BUTTON);
	virtual CMenu * MenuFindItemCMenu(CMenu * pParentMenu, UINT uMenuStringID) const;
	virtual bool AddMenu(CMenu * pParent, UINT nResourceID);
	virtual bool AddMenu(CMenu * pMenu, UINT nCommandID, UINT nResourceID, UINT nStatusID = 0);
	virtual bool AddMenu(CMenu * pMenu, UINT nCommandID, const CString& szItem, UINT nStatusID);
	virtual bool InsertMenu(CMenu * pParent, UINT nPos, UINT nResourceID, bool bByPosition = false);
	virtual bool InsertMenu(CMenu * pMenu, UINT nPos, UINT nCommandID, UINT nResourceID, UINT nStatusID = 0);
	virtual bool InsertMenu(CMenu * pMenu, UINT nPos, UINT nCommandID, const CString& szItem, const CString& szStatusString = _T(""));
	virtual bool DeleteMenuItem(CMenu * pMenu, UINT nResourceID, bool bolString = false);
	virtual bool DeleteMenu(CMenu * pMenu, UINT nResourceID, bool bolString = false);
	virtual void EnableMenu(UINT nCommandID, bool bolEnable = true);
	virtual void EnableMenu(const CString& szMenuItem, bool bolEnable = true);
	virtual bool AddSeparator(CMenu * pMenu);
	virtual bool InsertSeparator(CMenu * pMenu, UINT nBeforeCommandID);
	virtual void SetTitleText(void);
	virtual CString GetLast8Batches(stuErrorReturn& clsErrorReturn, int intBatchType);

	static CString UpdateMenuString(const CString& szMenuText, UINT nCommandID);

	virtual UINT		get_CurrentBatchCommandID(void) const;
	virtual UINT		put_CurrentBatchCommandID(UINT uiID);
	virtual CString		get_BatchCommand(UINT uiID) const;
	virtual void		put_BatchCommand(UINT uiID, const CString& szBatchCode);

	virtual BatchCommandCollection& get_BatchCollection(void) { return m_batchCommandArray; }

	virtual BYTE		get_ToolButtonStyle(void) const { return GetStdToolBtnStyle(); }
	virtual CGBLSystemInformation *	get_SI(void) const { return CMS::GetSI(); }

	virtual operator CFrameWnd& (void) { return *Frame; }
	virtual operator const CFrameWnd& (void) const { return *const_cast<self *>(this)->Frame; }

	virtual COXBitmapMenuOrganizer&		get_MenuOrganizer(void);
	virtual CMenu&						get_MainMenu(void);
	virtual void						put_MainMenu(CMenu& menu);
	virtual CMenu *						get_Menu(UINT nCommandID);
	virtual void						put_Menu(UINT nCommandID, CMenu * menu);
	virtual CCMSMenuInfo				get_MenuData(UINT nCommandID);
	virtual bool						get_MenuExists(UINT nCommandID);
	virtual COXCoolToolBar *			get_ToolBar(void);
	virtual CCMSStatusBar *				get_StatusBar(void);
	virtual CCMSModuleRefresh *			get_ModuleRefreshObject(void);
	virtual CImageList&					get_ToolBarImageList(void);
	virtual CCMSStatusBarMsgCollection&	get_StatusMessages(void);
	virtual CCMSTBButtonInfoCollection&	get_ToolButtonInfo(void);
	virtual TBBUTTON *&					get_ToolButtonInfoPointer(void);
	virtual ModuleTrackerInfo&			get_ModuleTracker(void);
	virtual IStatusBarHandler *			get_StatusBarHandler(void);
	virtual bool&						get_CanAccess(ICMSMainFrameWnd::enmStandardGUIItem nGUIItemID);
	virtual void						put_CanAccess(ICMSMainFrameWnd::enmStandardGUIItem nGUIItemID, bool bolAccess);
	virtual int							get_SelectedProcess(void) const;
	virtual void						put_SelectedProcess(int nSelectedProcess);
	virtual UINT_PTR					get_TimerID(void) const;
	virtual void						put_TimerID(UINT_PTR nTimerID);
	virtual CGBL_DLG_WndSpy *			get_WindowSpy(void) const;
	virtual void						put_WindowSpy(CGBL_DLG_WndSpy * pSpy);
	virtual UINT						get_ExitStatusMsgID(void) const;
	virtual void						put_ExitStatusMsgID(UINT uExitStatusMsgID);
	virtual UINT						get_TitleBarID(void) const;
	virtual void						put_TitleBarID(UINT TitleBarID);
	virtual CMS::ICMSObserver *			get_Observer(void);
	virtual UINT						get_WindowCount(void) const;
	virtual UINT						get_OpenWindowCount(void) const;
	virtual UINT						get_IconicWindowCount(void) const;

	bool	get_ShowXmlViewer(void) const;
	void	put_ShowXmlViewer(bool bShowXmlViewer);

	__declspec(property(get = get_CurrentBatchCommandID, put = put_CurrentBatchCommandID))	UINT			CurrentBatchCommandID;
	__declspec(property(get = get_BatchCommand, put = put_BatchCommand))									CString		BatchCommand[];
	__declspec(property(get = get_ToolButtonStyle))																				BYTE		ToolButtonStyle;
	__declspec(property(get = get_ShowXmlViewer, put = put_ShowXmlViewer))								bool			ShowXmlViewer;
	__declspec(property(get = get_ExitStatusMsgID, put = put_ExitStatusMsgID))								UINT			ExitStatusMsgID;
	__declspec(property(get = get_TitleBarID, put = put_TitleBarID))														UINT			TitleBarID;
	__declspec(property(get = get_BatchCollection))																				BatchCommandCollection& BatchCommands;
	__declspec(property(get = get_SI))																									CGBLSystemInformation	*	si;

	__declspec(property(get = get_MenuOrganizer))													COXBitmapMenuOrganizer&		MenuOrganizer;
	__declspec(property(get = get_MainMenu, put = put_MainMenu))						CMenu&										MainMenu;
	__declspec(property(get = get_Menu, put = put_Menu))										CMenu *										Menu[];
	__declspec(property(get = get_MenuData))															CCMSMenuInfo							MenuData[];
	__declspec(property(get = get_MenuExists))														bool												MenuExists[];
	__declspec(property(get = get_ToolBar))																COXCoolToolBar *						ToolBar;
	__declspec(property(get = get_StatusBar))															CCMSStatusBar *						StatusBar;
	__declspec(property(get = get_ModuleRefreshObject))										CCMSModuleRefresh *				CMSMod;
	__declspec(property(get = get_ToolBarImageList))												CImageList&									ToolBarImages;
	__declspec(property(get = get_StatusMessages))												CCMSStatusBarMsgCollection&	StatusMessages;
	__declspec(property(get = get_ToolButtonInfo))													CCMSTBButtonInfoCollection&	ToolButtonInfo;
	__declspec(property(get = get_ToolButtonInfoPointer))										TBBUTTON *&								ToolButtonInfoPointer;
	__declspec(property(get = get_ModuleTracker))													ModuleTrackerInfo&					ModuleTracker;
	__declspec(property(get = get_StatusBarHandler))												IStatusBarHandler *						StatusBarHandler;
	__declspec(property(get = get_CanAccess, put = put_CanAccess))					bool												CanAccess[];
	__declspec(property(get = get_SelectedProcess, put = put_SelectedProcess))	int													SelectedProcess;
	__declspec(property(get = get_TimerID, put = put_TimerID))								UINT_PTR											TimerID;
	__declspec(property(get = get_Observer))															ICMSObserver *							Observer;
	__declspec(property(get = get_WindowCount))													UINT												WindowCount;
	__declspec(property(get = get_OpenWindowCount))											UINT												OpenWindowCount;
	__declspec(property(get = get_IconicWindowCount))											UINT												IconicWindowCount;

	//common base services for MDI and SDI frames
	virtual BOOL	PreCreateWindow(CREATESTRUCT& cs);
	virtual int		OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL	OnNotify(WPARAM wParam, LPARAM lParam, LRESULT * pLResult);
	virtual void	GetMessageString(UINT nID, CString& rMessage) const;

	afx_msg void	OnDestroy(void);
	afx_msg BOOL	OnToolTipNotify(UINT id, NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void	OnInitMenuPopup(CMenu * pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void	OnCmdUIEnableMenu(CCmdUI *pCmdUI, UINT nBaseMenuID);
	afx_msg void	OnTimer(UINT_PTR unIDEvent);
	afx_msg void	OnWindowActivate(UINT nID);
	// 1-34642/1-34645 BK 8/19/10 added submenus to Cougar Dtails, financial & sales dashboards
	afx_msg void	OnEnableCougarDtails(CCmdUI *pCmdUI);	// begbert 02-08-2010 CDtails : Added this
	afx_msg void	OnEnableDenaliBI(CCmdUI *pCmdUI);
	afx_msg void	OnEnableFinancialDashboard(CCmdUI *pCmdUI);
	afx_msg void	OnEnableSalesDashboard(CCmdUI *pCmdUI);

	afx_msg void	OnEnableFile(CCmdUI *pCmdUI);
	afx_msg void	OnEnableAppExitButton(CCmdUI *pCmdUI);
	afx_msg void	OnEnableSaveButton(CCmdUI *pCmdUI);
	afx_msg void	OnEnablePrintButton(CCmdUI *pCmdUI);
	afx_msg void	OnEnableClearButton(CCmdUI *pCmdUI);
	afx_msg void	OnEnableHelpButton(CCmdUI *pCmdUI);
	afx_msg void	OnEnableArrangeOpenWindows(CCmdUI *pCmdUI);
	afx_msg void	OnEnableArrangeIconicWindows(CCmdUI *pCmdUI);
	afx_msg void	OnEnableCloseAll(CCmdUI *pCmdUI);
	afx_msg void	OnUpdateDisplayToolbarMenu(CCmdUI *pCmdUI);

	afx_msg void	OnFileAccountsPayable(UINT nID);
	afx_msg void	OnFileAccountsReceivable(UINT nID);
	afx_msg void	OnFileBankReconciliation(UINT nID);
	afx_msg void	OnFileGeneralLedger(UINT nID);
	afx_msg void	OnFileInventory(UINT nID);
	afx_msg void	OnFileJobCost(UINT nID);
	afx_msg void	OnFileOrderEntry(UINT nID);
	afx_msg void	OnFilePayroll(UINT nID);
	afx_msg void	OnFileSalesEntry(UINT nID);
	afx_msg void	OnFilePurchaseOrder(UINT nID);
	afx_msg void	OnFileSalesMaintenance(UINT nID);
	afx_msg void	OnFileController(void);
	afx_msg void	OnFileDenaliBI(void);
	// 1-34642/1-34645 BK 8/19/10 added submenus to Cougar Dtails, financial & sales dashboards
	afx_msg void	OnFileFinancialDashboard(void);
	afx_msg void	OnFileSalesDashboard(void);

	afx_msg void	OnAppExit(void);
	afx_msg void	OnFileSave(void);
	afx_msg void	OnFileClear(void);
	afx_msg void	OnFilePrint(void);
	afx_msg void	OnHelp(void);
	afx_msg void	OnWindowCloseAll(void);
	afx_msg void	OnHelpContents(void);
	afx_msg void	OnHelpAboutCougarMountainSoftware(void);
	afx_msg void	OnHelpTutorials(void);
	afx_msg void	OnHelpVideoTutorials(void);
	afx_msg void	OnHelpCMSWebsite(void);
	afx_msg void	OnHelpOnlineSupport(void);
	afx_msg void	OnHelpActivateSoftware(void);
	afx_msg void	OnHelpOrderCougarForms(void);
	afx_msg void	OnHelpMerchantTransactionServices(void);
	afx_msg void	OnDeveloperShowXMLViewer(void);
	afx_msg void	OnDeveloperShowXMLViewerOnError(void);
	afx_msg void	OnSelectDifferentCompany(void);
	afx_msg void	OnDeveloperShowWndSpy(void);
	afx_msg void	OnDeveloperAssertOnError(void);
	afx_msg void	OnOptionsStayInAddModeOnSave(void);
	afx_msg void	OnDisplayToolbar(void);
	afx_msg void	OnDisplayStatusbar(void);
	afx_msg void	OnDenaliClientUtilities(void);
	afx_msg void	OnDenaliServerUtilities(void);
	afx_msg void	OnDenaliLicenseUtilities(void);
	afx_msg void	OnSelectBatch(UINT uItemID);
	afx_msg LRESULT OnShowHelp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetupLocalization(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWaitStringChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWaitStringChange1(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnShowChangeDate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPingApp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEnsureAppVisible(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStartupCompleted(WPARAM wParam, LPARAM lParam);

private:
	COXCoolToolBar					m_wndToolBar;
	CCMSStatusBar					m_wndStatusBar;
	CCMSModuleRefresh				m_moduleRefresh;								// Module Information Thread
	COXBitmapMenuOrganizer			m_wndMenuOrganizer;						// used to organize menu images
	CImageList						m_toolbarImageList;

	UINT							m_uNextValidBatchCommandID;
	BatchCommandCollection			m_batchCommandArray;
	CCMSStatusBarMsgCollection		m_StatusBarMessages;
	CCMSTBButtonInfoCollection		m_ToolBarButtonInfo;
	TBBUTTON *						m_pToolBarButtonInfo;
	bool							m_bShowXmlViewer;
	UINT							m_uExitStatusMsgID;				// Exit Menu Status Message ID
	UINT							m_uTitleBarID;							// Resource ID for module name in title bar.  Make sure it includes the dash(-) at the end.
	int								m_nSelectedProcess;
	UINT_PTR						m_nTimer;
	ModuleTrackerInfo				m_pModuleTracker;
	GUIAccessMap					m_standardGUIAccess;
	std::auto_ptr<CGBL_DLG_WndSpy>	m_WndSpy;
};

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline CCMSBaseMainFrameWnd<T, TRAITS>::CCMSBaseMainFrameWnd(void)
	: CCMSBaseMainFrameWnd<T, TRAITS>::ConcreteBaseClass()
	, CCMSObserver()
	, CCMSSubject()
	, m_wndToolBar()
	, m_wndStatusBar()
	, m_moduleRefresh()
	, m_wndMenuOrganizer()
	, m_toolbarImageList()
	, m_uNextValidBatchCommandID(MENU_BATCH_IDRANGE_START)
	, m_batchCommandArray()
	, m_StatusBarMessages()
	, m_ToolBarButtonInfo()
	, m_pToolBarButtonInfo(NULL)
	, m_bShowXmlViewer(false)
	, m_uExitStatusMsgID(0)
	, m_uTitleBarID(0)
	, m_pModuleTracker(NULL)
	, m_standardGUIAccess()
	, m_nSelectedProcess(0)
	, m_nTimer(0)
	, m_WndSpy()
{
	m_standardGUIAccess[AppExitButton] = true;
	m_standardGUIAccess[SaveButton] = true;
	m_standardGUIAccess[PrintButton] = true;
	m_standardGUIAccess[ClearButton] = true;
	m_standardGUIAccess[HelpButton] = true;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline BOOL	CCMSBaseMainFrameWnd<T, TRAITS>::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style &= ~FWS_ADDTOTITLE;
	return ConcreteBaseClass::PreCreateWindow(cs);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline int CCMSBaseMainFrameWnd<T, TRAITS>::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (ConcreteBaseClass::OnCreate(lpCreateStruct) == -1)
		return -1;
	else if (traits::OnCreate(this, lpCreateStruct) == -1)
		return -1;

	SetTitleText();
	return 0;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline BOOL CCMSBaseMainFrameWnd<T, TRAITS>::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT * pLResult)
{
	if (!traits::OnNotify(this, wParam, lParam, pLResult)) return FALSE;

	//DG.03.21.2011 1-35466 Problems with toolbar resizing unexpectedly
	ToolBar->SetButtonsMinMaxWidth(32, 32);

	return __super::OnNotify(wParam, lParam, pLResult);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::GetMessageString(UINT nID, CString& rMessage) const
{
	traits::GetMessageString(const_cast<self *>(this), nID, rMessage);
	if (rMessage.IsEmpty()) __super::GetMessageString(nID, rMessage);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::OnInitMenuPopup(CMenu * pPopupMenu, UINT nIndex, BOOL bSystemMenu)
{
	__super::OnInitMenuPopup(pPopupMenu, nIndex, bSystemMenu);
	traits::OnInitMenuPopup(this, pPopupMenu, nIndex, bSystemMenu);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline ICMSMainFrameWnd * CCMSBaseMainFrameWnd<T, TRAITS>::get_BaseFrameWnd(void)
{
	return static_cast<ICMSMainFrameWnd *>(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline CFrameWnd * CCMSBaseMainFrameWnd<T, TRAITS>::get_FrameWnd(void)
{
	CFrameWnd * pFrame = NULL;
	if (ConcreteBaseClass::IsKindOf(RUNTIME_CLASS(CFrameWnd))) pFrame = DYNAMIC_DOWNCAST(CFrameWnd, this);
	return pFrame;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline CMDIFrameWnd * CCMSBaseMainFrameWnd<T, TRAITS>::get_MDIFrameWnd(void)
{
	CMDIFrameWnd * pFrame = NULL;
	if (ConcreteBaseClass::IsKindOf(RUNTIME_CLASS(CMDIFrameWnd))) pFrame = DYNAMIC_DOWNCAST(CMDIFrameWnd, this);
	return pFrame;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline COXCoolToolBar * CCMSBaseMainFrameWnd<T, TRAITS>::get_ToolBar(void)
{
	return &m_wndToolBar;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline CCMSStatusBar * CCMSBaseMainFrameWnd<T, TRAITS>::get_StatusBar(void)
{
	return &m_wndStatusBar;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline CCMSModuleRefresh * CCMSBaseMainFrameWnd<T, TRAITS>::get_ModuleRefreshObject(void)
{
	return &m_moduleRefresh;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline UINT CCMSBaseMainFrameWnd<T, TRAITS>::get_CurrentBatchCommandID(void) const
{
	return m_uNextValidBatchCommandID;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline UINT CCMSBaseMainFrameWnd<T, TRAITS>::put_CurrentBatchCommandID(UINT uiID)
{
	m_uNextValidBatchCommandID = uiID > MENU_BATCH_IDRANGE_END ? MENU_BATCH_IDRANGE_START : uiID;
	return m_uNextValidBatchCommandID;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline CString CCMSBaseMainFrameWnd<T, TRAITS>::get_BatchCommand(UINT uiID) const
{
	BatchCommandCollection::const_iterator	it = m_batchCommandArray.begin();
	for (; it != m_batchCommandArray.end(); it++) if (it->first == uiID) break;
	return it != m_batchCommandArray.end() ? it->second : _T("");
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::put_BatchCommand(UINT uiID, const CString& szBatchCode)
{
	if (m_batchCommandArray.size() > MENU_BATCH_IDRANGE_END - MENU_BATCH_IDRANGE_START) m_batchCommandArray.erase(m_batchCommandArray.begin());
	m_batchCommandArray[uiID] = szBatchCode;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline bool CCMSBaseMainFrameWnd<T, TRAITS>::get_ShowXmlViewer(void) const
{
	return m_bShowXmlViewer;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::put_ShowXmlViewer(bool bShowXmlViewer)
{
	m_bShowXmlViewer = bShowXmlViewer;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline COXBitmapMenuOrganizer& CCMSBaseMainFrameWnd<T, TRAITS>::get_MenuOrganizer(void)
{
	return m_wndMenuOrganizer;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline CMenu& CCMSBaseMainFrameWnd<T, TRAITS>::get_MainMenu(void)
{
	return *ConcreteBaseClass::GetMenu();
}
//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::put_MainMenu(CMenu& menu)
{
	menu.Attach(ConcreteBaseClass::GetMenu()->Detach());
	ConcreteBaseClass::SetMenu(&menu);
	ConcreteBaseClass::DrawMenuBar();
	menu.Detach();
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline CMenu * CCMSBaseMainFrameWnd<T, TRAITS>::get_Menu(UINT nCommandID)
{
	CCMSMenuBuilder	builder(this);
	CMenu * pMenu = builder.GetMenu(nCommandID);					//look for menu by command
	if (!pMenu) pMenu = builder.GetMenu(nCommandID, true);	//if not found, look for menu by string
	return pMenu;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::put_Menu(UINT nCommandID, CMenu * pNewMenu)
{
	CMenu * const pMenu = get_Menu(nCommandID);
	ASSERT_VALID(pMenu);

	pNewMenu->Attach(pMenu->Detach());
	ConcreteBaseClass::DrawMenuBar();
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline CCMSMenuInfo CCMSBaseMainFrameWnd<T, TRAITS>::get_MenuData(UINT nCommandID)
{
	CCMSMenuInfo		objMenuInfo;

	CCMSMenuBuilder	builder(this);
	CMenu * pMenu = NULL;
	int nPos = builder.GetMenuItem(nCommandID, pMenu);							//look for menu by command
	if (!pMenu) nPos = builder.GetMenuItem(nCommandID, pMenu, true);	//if not found, look for menu by string

	objMenuInfo.ParentMenu = pMenu;
	objMenuInfo.MenuIndex = nPos;
	objMenuInfo.Menu = Menu[nCommandID];

	return objMenuInfo;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline bool CCMSBaseMainFrameWnd<T, TRAITS>::get_MenuExists(UINT nCommandID)
{
	return Menu[nCommandID] != NULL;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline CImageList& CCMSBaseMainFrameWnd<T, TRAITS>::get_ToolBarImageList(void)
{
	return m_toolbarImageList;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline CCMSStatusBarMsgCollection&	CCMSBaseMainFrameWnd<T, TRAITS>::get_StatusMessages(void)
{
	return m_StatusBarMessages;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline CCMSTBButtonInfoCollection& CCMSBaseMainFrameWnd<T, TRAITS>::get_ToolButtonInfo(void)
{
	return 	m_ToolBarButtonInfo;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline TBBUTTON *& CCMSBaseMainFrameWnd<T, TRAITS>::get_ToolButtonInfoPointer(void)
{
	return m_pToolBarButtonInfo;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline bool& CCMSBaseMainFrameWnd<T, TRAITS>::get_CanAccess(ICMSMainFrameWnd::enmStandardGUIItem nGUIItemID)
{
	GUIAccessMap::iterator	it = m_standardGUIAccess.find(nGUIItemID);
	return it->second;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::put_CanAccess(ICMSMainFrameWnd::enmStandardGUIItem nGUIItemID, bool bolAccess)
{
	GUIAccessMap::iterator	it = m_standardGUIAccess.find(nGUIItemID);
	it->second = bolAccess;
}
//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline CGBL_DLG_WndSpy * CCMSBaseMainFrameWnd<T, TRAITS>::get_WindowSpy(void) const
{
	return m_WndSpy.get();
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::put_WindowSpy(CGBL_DLG_WndSpy * pSpy)
{
	m_WndSpy.reset(pSpy);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline CMS::ICMSObserver * CCMSBaseMainFrameWnd<T, TRAITS>::get_Observer(void)
{
	return static_cast<CMS::ICMSObserver *>(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline UINT CCMSBaseMainFrameWnd<T, TRAITS>::get_WindowCount(void) const
{
	return traits::get_WindowCount(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline UINT CCMSBaseMainFrameWnd<T, TRAITS>::get_OpenWindowCount(void) const
{
	return traits::get_OpenWindowCount(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline UINT CCMSBaseMainFrameWnd<T, TRAITS>::get_IconicWindowCount(void) const
{
	return traits::get_IconicWindowCount(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::MenuSecurityDisableItems(void)
{
	traits::MenuSecurityDisableItems(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::UpdateMenu(bool bStartUp)
{
	traits::UpdateMenu(this, bStartUp);
}
//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::ShowWndInfo(CWnd * pWnd)
{
	if (WindowSpy) WindowSpy->ShowInfo(pWnd);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline CString CCMSBaseMainFrameWnd<T, TRAITS>::UpdateMenuString(const CString& szMenuText, UINT nCommandID)
{
	return traits::UpdateMenuString(szMenuText, nCommandID);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::EnablePrint(bool bolEnable)
{
	CanAccess[PrintButton] = bolEnable;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::EnableSave(bool bolEnable)
{
	CanAccess[SaveButton] = bolEnable;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::EnableClear(bool bolEnable)
{
	CanAccess[ClearButton] = bolEnable;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::EnableHelp(bool bolEnable)
{
	CanAccess[HelpButton] = bolEnable;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::DisableToolbarButtons(bool bolDisable)
{
	traits::DisableToolbarButtons(this, bolDisable);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::AddStatusBarMessage(UINT nMenuItemID, const CString& szStatusString)
{
	StatusMessages.AddMessage(nMenuItemID, szStatusString);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::AddStatusBarMessage(UINT nMenuItemID, UINT nResourceID)
{
	AddStatusBarMessage(nMenuItemID, CGBLResources::GetResourceString(nResourceID));
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::ToolBarAddToButtonArray(int nResourceID, int nButtonIdx, HICON hPic, BYTE fsStyle)
{
	traits::ToolBarAddToButtonArray(this, nResourceID, nButtonIdx, hPic, fsStyle);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline CMenu * CCMSBaseMainFrameWnd<T, TRAITS>::MenuFindItemCMenu(CMenu * pParentMenu, UINT uMenuStringID) const
{
	return traits::MenuFindItemCMenu(const_cast<self *>(this), pParentMenu, uMenuStringID);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::EnableMenu(UINT nCommandID, bool bolEnable)
{
	traits::EnableMenu(this, nCommandID, bolEnable);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::EnableMenu(const CString& szMenuItem, bool bolEnable)
{
	traits::EnableMenu(this, szMenuItem, bolEnable);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline bool CCMSBaseMainFrameWnd<T, TRAITS>::AddMenu(CMenu * pParent, UINT nResourceID)
{
	return traits::AddMenu(this, pParent, nResourceID);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline bool CCMSBaseMainFrameWnd<T, TRAITS>::AddMenu(CMenu * pMenu, UINT nCommandID, UINT nResourceID, UINT nStatusID)
{
	return traits::AddMenu(this, pMenu, nCommandID, nResourceID, nStatusID);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline bool CCMSBaseMainFrameWnd<T, TRAITS>::AddMenu(CMenu * pMenu, UINT nCommandID, const CString& szItem, UINT nStatusID)
{
	return traits::AddMenu(this, pMenu, nCommandID, szItem, nStatusID);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline bool CCMSBaseMainFrameWnd<T, TRAITS>::AddSeparator(CMenu * pMenu)
{
	return traits::AddSeparator(this, pMenu);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline bool CCMSBaseMainFrameWnd<T, TRAITS>::InsertSeparator(CMenu * pMenu, UINT nBeforeCommandID)
{
	return traits::InsertSeparator(this, pMenu, nBeforeCommandID);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::SetTitleText(void)
{
	traits::SetTitleText(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline CString CCMSBaseMainFrameWnd<T, TRAITS>::GetLast8Batches(stuErrorReturn& clsErrorReturn, int intBatchType)
{
	return traits::GetLast8Batches(this, clsErrorReturn, intBatchType);
}
//------------------------------------------------------------------------------
// used to create a menu subgroup (multi-batch mode batches)
// BK 1/7/2011 added bByPosition parameter used to create a menu subgroup (used with FASB and all menu subgroups that are set by position)
template<typename T, typename TRAITS>
inline bool CCMSBaseMainFrameWnd<T, TRAITS>::InsertMenu(CMenu * pParent, UINT nPos, UINT nResourceID, bool bByPosition)
{
	return traits::InsertMenu(this, pParent, nPos, nResourceID, bByPosition);
}
//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline bool CCMSBaseMainFrameWnd<T, TRAITS>::InsertMenu(CMenu * pMenu, UINT nPos, UINT nCommandID, UINT nResourceID, UINT nStatusID)
{
	return traits::InsertMenu(this, pMenu, nPos, nCommandID, nResourceID, nStatusID);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline bool CCMSBaseMainFrameWnd<T, TRAITS>::InsertMenu(CMenu * pMenu, UINT nPos, UINT nCommandID, const CString& szItem, const CString& szStatusString)
{
	return traits::InsertMenu(this, pMenu, nPos, nCommandID, szItem, szStatusString);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline bool CCMSBaseMainFrameWnd<T, TRAITS>::DeleteMenu(CMenu * pMenu, UINT nResourceID, bool bolString)
{
	return traits::DeleteMenu(this, pMenu, nResourceID, bolString);
}
//------------------------------------------------------------------------------
// delete only the menu items no separators
template<typename T, typename TRAITS>
inline bool CCMSBaseMainFrameWnd<T, TRAITS>::DeleteMenuItem(CMenu * pMenu, UINT nResourceID, bool bolString)
{
	return traits::DeleteMenu(this, pMenu, nResourceID, bolString, false);
}
//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline UINT CCMSBaseMainFrameWnd<T, TRAITS>::get_ExitStatusMsgID(void) const
{
	return m_uExitStatusMsgID;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::put_ExitStatusMsgID(UINT uExitStatusMsgID)
{
	m_uExitStatusMsgID = uExitStatusMsgID;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline UINT CCMSBaseMainFrameWnd<T, TRAITS>::get_TitleBarID(void) const
{
	return m_uTitleBarID;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::put_TitleBarID(UINT uTitleBarID)
{
	m_uTitleBarID = uTitleBarID;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline ICMSMainFrameWnd::ModuleTrackerInfo& CCMSBaseMainFrameWnd<T, TRAITS>::get_ModuleTracker(void)
{
	return m_pModuleTracker;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline IStatusBarHandler * CCMSBaseMainFrameWnd<T, TRAITS>::get_StatusBarHandler(void)
{
	return static_cast<IStatusBarHandler *>(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline int CCMSBaseMainFrameWnd<T, TRAITS>::get_SelectedProcess(void) const
{
	return m_nSelectedProcess;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::put_SelectedProcess(int nSelectedProcess)
{
	m_nSelectedProcess = nSelectedProcess;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline UINT_PTR CCMSBaseMainFrameWnd<T, TRAITS>::get_TimerID(void) const
{
	return m_nTimer;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::put_TimerID(UINT_PTR nTimerID)
{
	m_nTimer = nTimerID;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::OnDestroy(void)
{
	traits::OnDestroy(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline BOOL CCMSBaseMainFrameWnd<T, TRAITS>::OnToolTipNotify(UINT id, NMHDR * pNMHDR, LRESULT * pResult)
{
	return traits::OnToolTipNotify(this, id, pNMHDR, pResult);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::OnWindowActivate(UINT nID)
{
	CCMSWindowManager::Instance().ActivateWindow(nID);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::OnCmdUIEnableMenu(CCmdUI *pCmdUI, UINT nBaseMenuID)
{
	traits::OnCmdUIEnableMenu(this, pCmdUI, nBaseMenuID);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::OnTimer(UINT_PTR unIDEvent)
{
	traits::OnTimer(this, unIDEvent);
	__super::OnTimer(unIDEvent);
	TimerID = 0;
}

//------------------------------------------------------------------------------
// 1-34642/1-34645 BK 8/19/10 added submenus to Cougar Dtails, financial & sales dashboards
// begbert 02-08-2010 CDtails : Added this
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::OnEnableCougarDtails(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(si->UserRight.m_stuModulesNode.CougarDtails.bTotal);
}
//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::OnEnableDenaliBI(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(/*si->UserRight.m_stuModulesNode.bDenaliBI &&*/ si->ModulesInstalled.DenaliBI.Licensed);
	//pCmdUI->Enable(true);
}
//------------------------------------------------------------------------------
// 1-34642/1-34645 BK 8/19/10 added submenus to Cougar Dtails, financial & sales dashboards
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::OnEnableFinancialDashboard(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(si->UserRight.m_stuModulesNode.CougarDtails.bFinancialDashboard && si->ModulesInstalled.GeneralLedger.IsUsable);
}
//------------------------------------------------------------------------------
// 1-34642/1-34645 BK 8/19/10 added submenus to Cougar Dtails, financial & sales dashboards
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::OnEnableSalesDashboard(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(si->UserRight.m_stuModulesNode.CougarDtails.bSalesDashboard && si->ModulesInstalled.SalesEntry.IsUsable);
	//pCmdUI->Enable(true);
}
//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void CCMSBaseMainFrameWnd<T, TRAITS>::OnEnableFile(CCmdUI *pCmdUI)
{
	traits::OnEnableFile(this, pCmdUI);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnEnableAppExitButton(CCmdUI * pCmdUI)
{
	pCmdUI->Enable(CanAccess[AppExitButton]);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnEnableSaveButton(CCmdUI *pCmdUI)
{
	traits::OnEnableSaveButton(this, pCmdUI, CanAccess[SaveButton]);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnEnablePrintButton(CCmdUI *pCmdUI)
{
	traits::OnEnablePrintButton(this, pCmdUI, CanAccess[PrintButton]);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnEnableClearButton(CCmdUI *pCmdUI)
{
	traits::OnEnableClearButton(this, pCmdUI, CanAccess[ClearButton]);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnEnableArrangeOpenWindows(CCmdUI *pCmdUI)
{
	traits::OnEnableArrangeOpenWindows(this, pCmdUI);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnEnableArrangeIconicWindows(CCmdUI *pCmdUI)
{
	traits::OnEnableArrangeIconicWindows(this, pCmdUI);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnEnableCloseAll(CCmdUI *pCmdUI)
{
	traits::OnEnableCloseAll(this, pCmdUI);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnEnableHelpButton(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(CanAccess[HelpButton] ? TRUE : FALSE);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnUpdateDisplayToolbarMenu(CCmdUI *pCmdUI)
{
	traits::OnUpdateDisplayToolbarMenu(this, pCmdUI);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnFileAccountsPayable(UINT nID)
{
	ModuleTracker->LaunchModule(IDS_AP_MODULE, nID);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnFileAccountsReceivable(UINT nID)
{
	ModuleTracker->LaunchModule(IDS_AR_MODULE, nID);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnFileBankReconciliation(UINT nID)
{
	ModuleTracker->LaunchModule(IDS_BR_MODULE, nID);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnFileGeneralLedger(UINT nID)
{
	ModuleTracker->LaunchModule(IDS_GL_MODULE, nID);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnFileInventory(UINT nID)
{
	ModuleTracker->LaunchModule(IDS_IN_MODULE, nID);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnFileJobCost(UINT nID)
{
	ModuleTracker->LaunchModule(IDS_JC_MODULE, nID);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnFileOrderEntry(UINT nID)
{
	ModuleTracker->LaunchModule(IDS_OE_MODULE, nID);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnFilePayroll(UINT nID)
{
	ModuleTracker->LaunchModule(IDS_PR_MODULE, nID);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnFileSalesEntry(UINT nID)
{
	ModuleTracker->LaunchModule(IDS_PS_MODULE, nID);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnFilePurchaseOrder(UINT nID)
{
	ModuleTracker->LaunchModule(IDS_PO_MODULE, nID);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnFileSalesMaintenance(UINT nID)
{
	ModuleTracker->LaunchModule(IDS_SA_MODULE, nID);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnFileController(void)
{
	ModuleTracker->LaunchModule(IDS_CT_MODULE, IDS_CONTROLLER);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnFileDenaliBI(void)
{
	if (!si->ModuleSettings.CTSettings.SisenseURL.IsEmpty())
	{
		// Sisense lives in the same place as the license server, which is fine.  However sisense can't handle it if you refer to
		//   the license server by name while on it, which is dumb, and also a problem.  So, when the local machine is the same
		//   as the server, call it 'localhost' instead.

		const long INFO_BUFFER_SIZE = 32767;
		TCHAR  infoBuf[INFO_BUFFER_SIZE];
		DWORD  bufCharCount = INFO_BUFFER_SIZE;
		VERIFY(GetComputerName(infoBuf, &bufCharCount));

		CString szLocalName = infoBuf;

		CString szServerName = si->Servers.PrimaryServer.Name;
		int pos = szServerName.Find('\\');
		if (pos > -1) szServerName = szServerName.Left(pos);

		if (szLocalName == szServerName)
			szServerName = L"localhost";

		CGBLWeb::OpenWebsite(L"http://" + szServerName + L":8081" + si->ModuleSettings.CTSettings.SisenseURL);
	} else
		CGBLForm::CMSMessageBox(si->ApplicationMainFrame, 0, IDS_LBL_ERROR, MB_OK | MB_ICONERROR, L"To access Denali BI, you must enter the required information in the Denali Business Intelligence window in the Controller module.", L"", 0, false);

	//ModuleTracker->LaunchModule(IDS_BI_MODULE, IDS_MODULE_BI);
}

//------------------------------------------------------------------------------
// 1-34642/1-34645 BK 8/19/10 added submenus to Cougar Dtails, financial & sales dashboards
// begbert 02-08-2010 CDtails : Added this
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnFileFinancialDashboard(void)
{
	CString strPath = CGBLForm::GetPath();
	// checking for back slash on end of path and adding it if necessary
	if (strPath.Right(1) != _T("\\")) strPath += _T("\\");

	CString szFilename = strPath + _T("CMSDtails.exe");
	CString szParams = _T("-dc ") + si->CompanySettings.ID + _T(" -us ") + si->UserInformation.ID;
	if (!si->UserInformation.Password.IsEmpty())
		szParams += _T(" -ps ") + CGBLMath::EncryptData(si->UserInformation.Password);
	if (si->CompanySettings.IsFund) szParams += _T(" -fn");
	::ShellExecute(GetSafeHwnd(), _T("Open"), (LPCTSTR)szFilename, szParams, NULL, SW_SHOWNORMAL);
}
//------------------------------------------------------------------------------
// 1-34642/1-34645 BK 8/19/10 added submenus to Cougar Dtails, financial & sales dashboards
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnFileSalesDashboard(void)
{
	CString strPath = CGBLForm::GetPath();
	// checking for back slash on end of path and adding it if necessary
	if (strPath.Right(1) != _T("\\")) strPath += _T("\\");

	CString szFilename = strPath + _T("CMSSDtails.exe");
	CString szParams = _T("-dc ") + si->CompanySettings.ID + _T(" -us ") + si->UserInformation.ID;
	if (!si->UserInformation.Password.IsEmpty())
		szParams += _T(" -ps ") + CGBLMath::EncryptData(si->UserInformation.Password);
	if (si->CompanySettings.IsFund) szParams += _T(" -fn");
	::ShellExecute(GetSafeHwnd(), _T("Open"), (LPCTSTR)szFilename, szParams, NULL, SW_SHOWNORMAL);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnAppExit(void)
{
	traits::OnAppExit(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnFileSave(void)
{
	traits::OnFileSave(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnFileClear(void)
{
	traits::OnFileClear(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnFilePrint(void)
{
	traits::OnFilePrint(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnHelp(void)
{
	traits::OnHelp(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnWindowCloseAll(void)
{
	traits::OnWindowCloseAll(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnHelpContents(void)
{
	traits::OnHelpContents(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnHelpAboutCougarMountainSoftware(void)
{
	CAbout clsHelp;
	clsHelp.DoModal();
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnHelpTutorials(void)
{
	CGBLForm::CMSMessageBox(si->ApplicationMainFrame, 0, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONINFORMATION, _T("OnHelpTutorials"));
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnHelpVideoTutorials(void)
{
	CGBLForm::CMSMessageBox(si->ApplicationMainFrame, 0, IDS_COUGAR_MOUNTAIN, MB_OK | MB_ICONINFORMATION, _T("OnHelpVideoTutorials"));
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnHelpCMSWebsite(void)
{
	CGBLWeb::OpenWebsite(CGBLResources::GetResourceString(IDS_CMS_WEBSITE));
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnHelpOnlineSupport(void)
{
	CGBLWeb::OpenWebsite(CGBLResources::GetResourceString(IDS_CMS_WEBSITE_TECHSUPPORT));
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnHelpActivateSoftware(void)
{
	CGBLWeb::OpenWebsite(CGBLResources::GetResourceString(IDS_CMS_WEBSITE_ACTIVATION));
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnHelpOrderCougarForms(void)
{
	CGBLWeb::OpenWebsite(CGBLResources::GetResourceString(IDS_CMS_WEBSITE_ORDER_CMS_FORMS));
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnHelpMerchantTransactionServices(void)
{
	CGBLWeb::OpenWebsite(CGBLResources::GetResourceString(IDS_CMS_WEBSITE_MERCHANT_SERVICES));
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnDeveloperShowXMLViewer(void)
{
	traits::OnDeveloperShowXMLViewer(this);
}

template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnDeveloperShowXMLViewerOnError(void)
{
	traits::OnDeveloperShowXMLViewerOnError(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnSelectDifferentCompany(void)
{
	traits::OnSelectDifferentCompany(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnDeveloperShowWndSpy(void)
{
	traits::OnDeveloperShowWndSpy(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnDeveloperAssertOnError(void)
{
	traits::OnDeveloperAssertOnError(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnOptionsStayInAddModeOnSave(void)
{
	traits::OnOptionsStayInAddModeOnSave(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnDisplayToolbar(void)
{
	traits::OnDisplayToolbar(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnDisplayStatusbar(void)
{
	traits::OnDisplayStatusbar(this);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnDenaliClientUtilities(void)
{
	CGBLForm::LaunchModule(_T("ClientUtil.exe"));
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnDenaliServerUtilities(void)
{
	CGBLForm::LaunchModule(_T("ServerUtil.exe"));
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	CCMSBaseMainFrameWnd<T, TRAITS>::OnDenaliLicenseUtilities(void)
{
	CGBLForm::LaunchModule(_T("LicSvrUtil.exe"));
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline void	 CCMSBaseMainFrameWnd<T, TRAITS>::OnSelectBatch(UINT uItemID)
{
	si->CurrentBatch.SelectBatch(BatchCommand[uItemID]);
	StatusBar->Update();
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline LRESULT CCMSBaseMainFrameWnd<T, TRAITS>::OnShowHelp(WPARAM wParam, LPARAM lParam)
{
	return traits::OnShowHelp(this, wParam, lParam);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline LRESULT CCMSBaseMainFrameWnd<T, TRAITS>::OnSetupLocalization(WPARAM wParam, LPARAM lParam)
{
	return traits::OnSetupLocalization(this, wParam, lParam);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline LRESULT CCMSBaseMainFrameWnd<T, TRAITS>::OnWaitStringChange(WPARAM wParam, LPARAM lParam)
{
	return traits::OnWaitStringChange(this, wParam, lParam);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline LRESULT CCMSBaseMainFrameWnd<T, TRAITS>::OnWaitStringChange1(WPARAM wParam, LPARAM lParam)
{
	return traits::OnWaitStringChange1(this, wParam, lParam);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline LRESULT CCMSBaseMainFrameWnd<T, TRAITS>::OnShowChangeDate(WPARAM wParam, LPARAM lParam)
{
	CGBLFormWrap::ShowChangeDate();
	return 0;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline LRESULT CCMSBaseMainFrameWnd<T, TRAITS>::OnPingApp(WPARAM wParam, LPARAM lParam)
{
	lParam = (LPARAM)GetSafeHwnd();
	return (LRESULT)WM_CMS_FIND_APP_SUCCESS;
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline LRESULT CCMSBaseMainFrameWnd<T, TRAITS>::OnEnsureAppVisible(WPARAM wParam, LPARAM lParam)
{
	return traits::OnEnsureAppVisible(this, wParam, lParam);
}

//------------------------------------------------------------------------------
template<typename T, typename TRAITS>
inline LRESULT CCMSBaseMainFrameWnd<T, TRAITS>::OnStartupCompleted(WPARAM wParam, LPARAM lParam)
{
	return traits::OnStartupCompleted(this, wParam, lParam);
}
