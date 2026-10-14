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
#include "../APForms/resource.h"
#include "../ARForms/resource.h"
#include "../CTForms/resource.h"
#include "../GLForms/resource.h"
#include "../INForms/resource.h"
#include "../SAForms/resource.h"
#include "../cmshwwiz/resource.h"
#include "../BRForms/resource.h"
#include "../POForms/resource.h"
#include "../SEForms/resource.h"
#include "../GBLForms/resource.h"
#include "../GBLForms/CMSFormView.h"
#include "../GBLForms/cmsformlayoutmanager.h"
#include "../GBLForms/cmswindowmanager.h"

 //Code for New Edit Control Class that Highlights contents of edit control on First Click
BEGIN_MESSAGE_MAP(CEditEx, CEdit)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

void CEditEx::OnLButtonDown(UINT nFlags, CPoint pt)
{
	CEdit::OnLButtonDown(nFlags, pt);
	SetSel(0, -1);
}

IMPLEMENT_DYNCREATE(CCMSFormView, CFormView)

CCMSFormView::CCMSFormView(UINT IDD)
	: CFormView(IDD)
	, m_szSubForm("")
	, m_pMasterSheet(NULL)
	, m_pEXMaintDoc(NULL)
	, m_pPropertySheetInterface(NULL)
	, m_hwLookFocus(NULL)
	, m_bEXLayoutManager(true)
	, m_dteOpen(COleDateTime::GetCurrentTime())
	, m_uiCommandID(CCMSWindowManager::Instance().GenerateCommandID())
	, m_bolTopmostWindow(false)
	, m_objPrinterSetup(NULL)
	, m_dpi(-1)
{
	m_eModuleLib = appmodule;
	m_bClosing = false;
	si = g_pGBLSystemInformationGBLForms;
	m_pLookup = NULL;							// lookup class pointer
	m_nLookupType = 0;							// flag to indicate witch lookup was invoked
	m_clsXMLParams = NULL;							// param struct to receive data from the lookups
	m_Wnd = (CWnd*)this;
	m_IsThisFund = CMS::GetSI()->CompanySettings.IsFund;
	//pNETDLL.CreateInstance(__uuidof(LoadNETDLL));	//create netdll AES
	m_nFormID = -1;//init form id, individual forms will set this to corresponding id.
	//KPM, 8/20/2003, Setup the form tracker information
	si->OpenFormTracker.CreateFormOpenInformation(&m_FormInfo, 0, COpenFormTracker::CMSDialog, CGBLMath::GenerateGUID(), NULL);	//sb - vs8
	m_lpInput = 0;
	m_nReportType = 0;

	m_bCanceledDirty = false;
	m_bUserInteracted = false;						// DG - modified if user clicks on form or used key control

	//Peter Ringering - 10/08/2004 - New and improved EXMode additions.
	m_bEXMode = false;
	m_pEXTopHeader = NULL;
	m_pEXButtonPanel = NULL;
	m_pEXLayoutManager = NULL;
	m_pPrintCodes = NULL;
	m_bEXShowTopHeader = true;							//This is only valid if m_bEXMode = true.
	m_bEXInitDone = false;

	// PBI 22186 BK 8/21/14 Added red flag reports, added this variable to show correct module in print options box and on the report
	m_szModuleAbbrev = _T("");

	nUseReportForModule = si->ApplicationSettings.UseReportForModule;

	CCMSWindowManager::Instance().Add(this);
}

CCMSFormView::~CCMSFormView(void)
{
	CCMSWindowManager::Instance().Remove(this);

	//KPM, 8/20/2003, UnRegister this class with the form tracker
	si->OpenFormTracker.CMSRemoveForm(&m_FormInfo);
	zap(m_pEXTopHeader);
	zap(m_pEXLayoutManager);
	zap(m_pEXButtonPanel);
}

BEGIN_MESSAGE_MAP(CCMSFormView, CFormView)
	ON_MESSAGE(WM_USER_ONFOCUS, &CCMSFormView::OnGetFocusAcctCtrl)
	ON_MESSAGE(WM_USER_KILLFOCUS, &CCMSFormView::OnKillFocusAcctCtrl)
	ON_MESSAGE(WM_GETSUBFORM, &CCMSFormView::GetSubForm)
	//ON_MESSAGE(WM_GET_ACTIVE_PAGE_INDEX, GetActivePageIndex)
	//ON_MESSAGE(WM_GET_ACTIVE_PAGE_ID, GetActivePropPage)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, &CCMSFormView::OnTTNNeedText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, &CCMSFormView::OnTTNNeedText)
	ON_WM_CREATE()
	ON_MESSAGE(WM_CMS_QUERY_GRID_NOTIFYSINK, &CCMSFormView::OnQueryGridNotifySink)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_CMS_INITSIZEWND, &CCMSFormView::OnCMS_InitSizeWnd)
	ON_MESSAGE(CMS_MSG_PRE_CLOSE_FORM, &CCMSFormView::PreCloseForm)
	ON_WM_SIZE()
	ON_WM_WINDOWPOSCHANGING()
	ON_BN_CLICKED(IDCANCEL, &CCMSFormView::OnCancel)
	ON_BN_CLICKED(IDHELP, &CCMSFormView::OnHelp)
	ON_MESSAGE(WM_LOOKRETURNFOCUS, &CCMSFormView::OnLookReturnFocus)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

//Lookup maintenance. Send message to lookup to init a close on the forms
//Reset window links in frame.
LRESULT CCMSFormView::PreCloseForm(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = 1;
	HWND hWndLookupNext = (HWND)GetParent()->SendMessage(CMS_MSG_GET_LKP_LINK, LinkLookupNext);
	if (::IsWindow(hWndLookupNext))
		lResult = ::SendMessage(hWndLookupNext, CMS_MSG_PRE_CLOSE_LKP, 0, 0);
	if (lResult)
	{
		m_pLookup = NULL;
		GetParent()->SendMessage(CMS_MSG_SET_LKP_LINK, LinkLookupNext);
		GetParent()->SendMessage(CMS_MSG_SET_LKP_LINK, LinkFormNext);
	}
	return lResult;
}

//Return the pointer to a notifysink object to the caller.
//This is being called from an IAsyncFillNotifyBridge type. - See CMSMTGrid.
LRESULT CCMSFormView::OnQueryGridNotifySink(WPARAM wParam, LPARAM lParam)
{
	return (LRESULT)&m_clsGridNotifySink;
}

void CCMSFormView::OnInitialUpdate()
{
	//KPM, 8/20/2003, Register this class with the form tracker
	m_FormInfo.WindowHandle = this;
	si->OpenFormTracker.CMSAddForm(&m_FormInfo);
	__super::OnInitialUpdate();

	//Set pointer of this form to the default XML class
	m_clsXML.m_hForm = this;

	// DTG, 5.17.2004, prevent scroll bars on the child form when parent
	// frame is smaller than the child form view.
	ResizeParentToFit(false);

	//Peter Ringering - 11/12/2003
	CDocument* pDoc = GetDocument();
	if (pDoc->IsKindOf(RUNTIME_CLASS(CCMSDoc)))
	{
		CCMSDoc* pCMSDoc = (CCMSDoc*)pDoc;
		m_lpInput = pCMSDoc->GetLPInput();
	}

	//Peter Ringering - 10/13/2004 - EX Mode Additions
	if (m_bEXMode)
	{
		m_pEXTopHeader = new CCMSTopHeader();
		if (m_bEXLayoutManager)
			m_pEXLayoutManager = new CCMSLayoutManager();
		CGBLFormWrap::InitCMSFormEx(this);
		m_bEXInitDone = true;
	}
	else
	{
		PlaceCMSHeader();
		InitControls();		// added for Version 3.0 has no effect on earlier versions
	}

	EnableToolTips();

	//take string from locales
	//JKC Changed variable name to match Microsoft
	//if(si->Localization.CurrencySymbolPos == 0)
	//	m_szLocaleCurrencyMask.Format("%s#", si->Localization.CurrencySymbol) ;
	if (si->Localization.CurrencyPositivePattern == 0)
		m_szLocaleCurrencyMask.Format(_T("%s#"), si->Localization.ISOCurrencySymbol);
	else
		//JKC Changed variable name to match Microsoft
		//m_szLocaleCurrencyMask.Format("#%s", si->Localization.CurrencySymbol) ;
		m_szLocaleCurrencyMask.Format(_T("#%s"), si->Localization.ISOCurrencySymbol);

	m_clsXML.m_hForm = this;

	m_clsGridNotifySink.Subscribe(this);
	if (m_pEXMaintDoc)
		OnMaintStartup();

	//PTR.07.13.2005 - The problem is that this is dependent upon all CCMSFormView-derived classes to
	//utilize CCMSFormView's publish/subscribe services and not have their own publish/subscribe setup.
	//We have a few form view's like Print Codes that have their own publish/subscribe service.  This
	//fix will then cause these to break and with it being so close to release date, it is not wise
	//to go through everything and break it.  We'll have to fix this error later.
	//Defect ID 1-14412, PGP(06/20/2006) - Uncommented code below - see error description for details.
	CCMSFormLayoutManager::Instance()->Add(this);
	CCMSFocusMessage	objMessage(CCMSFocusMessage::New, this);
	Publish(objMessage);
}

CString	 CCMSFormView::GetWindowTitle(void) const
{
	CDocument* const pDoc = GetDocument();
	ASSERT(pDoc);
	const CString szTitle(pDoc->GetTitle());
	return szTitle;
}

BOOL CCMSFormView::IsWindowIconic(void) const
{
	return (::IsWindow(GetSafeHwnd()) ? const_cast<CCMSFormView*>(this)->GetFrame()->IsIconic() : FALSE);
}

BOOL CCMSFormView::Show(int nStyle) const
{
	return (::IsWindow(GetSafeHwnd()) ? const_cast<CCMSFormView*>(this)->GetFrame()->ShowWindow(nStyle) : FALSE);
}

void	CCMSFormView::BringToTop(void) const
{
	if (::IsWindow(GetSafeHwnd())) const_cast<CCMSFormView*>(this)->GetFrame()->BringWindowToTop();
}

//
BOOL CCMSFormView::OnTTNNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	//Not moving to CGBLForm because of reference to CAccountControl. PGP(06/24/2004)
	return CGBLFormWrap::OnTTNNeedText(id, pNMHDR, pResult);
}

/*********************************************************
Function    PlaceCMSHeader
Purpose     To Adjust Icon, Description, and Top Separator to be standard on all forms
Parameters  none
Returns     none
***********************************************************/
void CCMSFormView::PlaceCMSHeader()
{
	//This should not be run in the new and improved (EX Type) form views.
	ASSERT(!m_bEXMode);

	UINT nIconIDC = IDC_GBL_ICO_TOP;
	UINT nLabelIDC = IDC_GBL_TOPHEADER;
	UINT nSepIDC = IDC_GBL_SEP_TOP;

	CWnd* pWnd;
	//Could be on a GBLForm
	pWnd = GetDlgItem(IDC_GBL_SEP_TOP);
	if (pWnd != NULL)
	{
		nIconIDC = IDC_GBL_ICO_TOP;
		nLabelIDC = IDC_GBL_TOPHEADER;
		nSepIDC = IDC_GBL_SEP_TOP;
	}
	else
	{
		GetFormModule();
		if (m_nModuleID == 0)
		{
			m_nModuleID = si->ApplicationSettings.ModuleID;
		}
		bool bGoodModule = false;
		switch (m_nModuleID)
		{
		case IDS_AP_MODULE:
			nIconIDC = IDC_APF_ICO_TOP;
			nLabelIDC = IDC_APF_TOPHEADER;
			nSepIDC = IDC_APF_SEP_TOP;
			bGoodModule = true;
			break;
		case IDS_AR_MODULE:
			nIconIDC = IDC_ARF_ICO_TOP;
			nLabelIDC = IDC_ARF_TOPHEADER;
			nSepIDC = IDC_ARF_SEP_TOP;
			bGoodModule = true;
			break;
		case IDS_GB_MODULE:
			nIconIDC = IDC_GBL_ICO_TOP;
			nLabelIDC = IDC_GBL_TOPHEADER;
			nSepIDC = IDC_GBL_SEP_TOP;
			bGoodModule = true;
			break;
		case IDS_CT_MODULE:
			nIconIDC = IDC_CTF_ICO_TOP;
			nLabelIDC = IDC_CTF_TOPHEADER;
			nSepIDC = IDC_CTF_SEP_TOP;
			bGoodModule = true;
			break;
		case IDS_GL_MODULE:
			nIconIDC = IDC_GLF_ICO_TOP;
			nLabelIDC = IDC_GLF_TOPHEADER;
			nSepIDC = IDC_GLF_SEP_TOP;
			bGoodModule = true;
			break;
		case IDS_IN_MODULE:
			nIconIDC = IDC_INF_ICO_TOP;
			nLabelIDC = IDC_INF_TOPHEADER;
			nSepIDC = IDC_INF_SEP_TOP;
			bGoodModule = true;
			break;
		case IDS_SAE_MODULE:
		case IDS_SA_MODULE:
		{
			switch (m_eModuleLib)
			{
			case appmodule:  //saforms
				nIconIDC = IDC_SAF_ICO_TOP;
				nLabelIDC = IDC_SAF_TOPHEADER;
				nSepIDC = IDC_SAF_SEP_TOP;
				bGoodModule = true;
				break;
			case cmssales:
				nIconIDC = IDC_SAE_ICO_TOP;
				nLabelIDC = IDC_SAE_TOPHEADER;
				nSepIDC = IDC_SAE_SEP_TOP;
				bGoodModule = true;
				break;
			case cmshwwiz:
				nIconIDC = IDC_HWW_ICO_TOP;
				nLabelIDC = IDC_HWW_TOPHEADER;
				nSepIDC = IDC_HWW_SEP_TOP;
				bGoodModule = true;
				break;
			}
		}
		break;
		//KPM, 10/7/2003, add for BR
		case IDS_BR_MODULE:
			nIconIDC = IDC_BRF_ICO_TOP;
			nLabelIDC = IDC_BRF_TOPHEADER;
			nSepIDC = IDC_BRF_SEP_TOP;
			bGoodModule = true;
			break;
			//begbert 08-08-2007, add for PO/POE
		case IDS_PO_MODULE:
		case IDS_POE_MODULE:
			nIconIDC = IDC_PO_ICO_TOP;
			nLabelIDC = IDC_PO_TOPHEADER;
			nSepIDC = ID_PO_SEP_TOP;
			bGoodModule = true;
			break;
		default:
			bGoodModule = false;
			break;
		}
		if (!bGoodModule)
		{
			//If you get here you need to add your module and module IDCs to the switch statement
			CGBLForm::CMSMessageBox(
				this,
				IDS_ERR_GB_BADMODULE,
				IDS_ERR_GB_BADMODULE,
				MB_OK | MB_ICONEXCLAMATION);
			return;
		}
		pWnd = GetDlgItem((int)nSepIDC);
	}

	if (pWnd == NULL || GetDlgItem((int)nLabelIDC) == NULL || GetDlgItem((int)nIconIDC) == NULL)
	{
		//Could not find IDCs so just return
		return;
	}

	//*********Spacing Variables**********************
	CRect rcLine;			//Rectangle for the static control for the lines.
	CRect rcButtons;		//Rectangle for the static control for the buttons.
	CRect rcDialog;			//Rectangle for the static control for the Dialog.

	if (m_LayoutManager.IsAttached())
	{
		m_LayoutManager.Detach();
	}
	m_LayoutManager.Attach(this);		//attach the dialog
	m_LayoutManager.AddAllChildren();	//Add all items to the layout manager

	//**************GLOBAL RESIZING ITEMS*************************************************************
	// Build Top Down, Left to Right
	//Constrain the top icon to the right place on the form
	m_LayoutManager.SetConstraint(nIconIDC, OX_LMS_LEFT, OX_LMT_SAME, nBorder);
	m_LayoutManager.SetConstraint(nIconIDC, OX_LMS_TOP, OX_LMT_SAME, nBorder);
	//Constrain the top label to the right of the icon
	m_LayoutManager.SetConstraint(nLabelIDC, OX_LMS_LEFT, OX_LMT_OPPOSITE, nBorder, nIconIDC);
	m_LayoutManager.SetConstraint(nLabelIDC, OX_LMS_TOP, OX_LMT_SAME, nBorder);
	//Make the top label resize to the right
	m_LayoutManager.SetConstraint(nLabelIDC, OX_LMS_RIGHT, OX_LMT_SAME, -nBorder);

	pWnd->GetWindowRect(rcLine);
	rcLine.bottom = rcLine.top + nVerticalSpace;
	ScreenToClient(rcLine); // convert the screen coords
	pWnd->MoveWindow(rcLine);

	//Make the top line resize to the right (Offset of the margin)
	m_LayoutManager.SetConstraint(nSepIDC, OX_LMS_LEFT, OX_LMT_SAME, nBorder);
	m_LayoutManager.SetConstraint(nSepIDC, OX_LMS_RIGHT, OX_LMT_SAME, -nBorder);
	m_LayoutManager.SetConstraint(nSepIDC, OX_LMS_TOP, OX_LMT_OPPOSITE, nVerticalSpace, nLabelIDC);

	m_LayoutManager.RedrawLayout();
	// *****************End of top part of form****************
}

//*********************************************************
//* Function	GetFormModule
//*
//* Purpose 	Retrieve which module the form is associated
//*     		with based on the septop IDC value
//*
//* Parameters	none
//*
//* Returns 	none
//*********************************************************
void CCMSFormView::GetFormModule()
{
	if (SetFormModule(IDC_APF_SEP_TOP, IDS_AP_MODULE))
	{
	}
	else if (SetFormModule(IDC_ARF_SEP_TOP, IDS_AR_MODULE))
	{
	}
	else if (SetFormModule(IDC_CTF_SEP_TOP, IDS_CT_MODULE))
	{
	}
	else if (SetFormModule(IDC_GLF_SEP_TOP, IDS_GL_MODULE))
	{
	}
	else if (SetFormModule(IDC_INF_SEP_TOP, IDS_IN_MODULE))
	{
	}
	else if (SetFormModule(IDC_HWW_SEP_TOP, IDS_SA_MODULE))
	{
	}
	else if (SetFormModule(IDC_SAE_SEP_TOP, IDS_SA_MODULE))
	{
	}
	else if (SetFormModule(IDC_SAF_SEP_TOP, IDS_SA_MODULE))
	{
	}
	else if (SetFormModule(IDC_BRF_SEP_TOP, IDS_BR_MODULE))
	{
	}
	else
	{
		m_nModuleID = 0;
	}
	return;
}

/*********************************************************
Function    SetFormModule
Purpose     Retrieve which module the form is associated with based on the septop IDC value
Parameters  unItem - DlgItem to look for on form
unModule - Module ID to set
Returns     true - m_szModule and m_nModule set
false - m_szModule and m_nModule not set
***********************************************************/
bool CCMSFormView::SetFormModule(UINT unItem, UINT unModule)
{
	CWnd* pWnd;
	pWnd = GetDlgItem((int)unItem);
	if (pWnd != NULL)
	{
		m_szModule = RESSTRING(unModule);
		m_nModuleID = unModule;
		return true;
	}
	return false;
}

/*********************************************************
Function    GetPrintParams
Purpose     To return Print Params in XML
Parameters  none
Returns     CString
***********************************************************/
//PGP 02/10/03
// BK 05/03/03 - added reportType parameter
CString CCMSFormView::GetPrintParams(ReportTypes enuReportType, bool bolEnableReportParams)
{
	return CGBLPrint::GetPrintParams(enuReportType, bolEnableReportParams, nUseReportForModule);
}

/*********************************************************
Function    SetResourceText
Purpose     Used to get the text string from the resource file for labels
Parameters  long lResourceID		ID of the resource label to set the text to.
long lResourceText		ID of the string text form lang file
Returns     true if no error
false if error

***********************************************************/
void CCMSFormView::SetResourceText(long lResourceID, long lResourceText)
{
	ASSERT_VALID(this->GetDlgItem(lResourceID));
	this->GetDlgItem(lResourceID)->SetWindowText(RESSTRING((UINT)lResourceText));
}

void CCMSFormView::SetResourceText(long lResourceID, const CString& szString)
{
	ASSERT_VALID(this->GetDlgItem(lResourceID));
	this->GetDlgItem(lResourceID)->SetWindowText(szString);
}

void CCMSFormView::SetResourceText(CWnd* pResourceWnd, long lResourceText)
{
	ASSERT_VALID(pResourceWnd);
	pResourceWnd->SetWindowText(RESSTRING((UINT)lResourceText));
}

void CCMSFormView::SetResourceText(CWnd* pResourceWnd, const CString& szString)
{
	ASSERT_VALID(pResourceWnd);
	pResourceWnd->SetWindowText(szString);
}

//*********************************************************
// Function		Validate
// Purpose		Used to return the dirty flag
// Parameters	none
// Returns		true if validation passes
//				false if validation does not pass
//***********************************************************/
bool CCMSFormView::Validate(void)
{
	bool bReturn = false;
	bReturn = true; //for now

	return bReturn;
}

//*********************************************************
// Function		OnNotify
// Purpose		Used to send messages to form
// Parameters		WPARAM wParam - Message Parameters
//				LPARAM lParam - Main Message part
//				LRESULT* pResult - Return Result
// Returns		true if validation passes
//				false if validation does not pass
//*********************************************************
BOOL CCMSFormView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* pMessage = (NMHDR*)lParam;
	try
	{
		//03/26/2019 CJB - PBI 49101 - First check for a valid pointer
		bool bValidPointer = false;
		if (::IsWindow(this->m_hWnd))
		{
			if (this != NULL && AfxIsValidAddress(this, sizeof(CCMSFormView)))
				bValidPointer = true;
		}

		switch (pMessage->code)
		{
			///use for the message sent from the buttons dlg to the form. AES
		case WM_BTN_OK:				//Message received from Save Button
			if (bValidPointer)
				this->OnOk();		//Call OnOk method
			break;
		case WM_OK_DONE:			//Message received from Save Button
			if (bValidPointer)
				this->OnOkAfter();	//Call OnOkAfter method
			break;
		case WM_BTN_CANCEL:			//Message received from Edit/Delete Button
			if (bValidPointer)
				this->OnFormCancel();	//Call OnCancel method  //11/18/2003 Heather Elston
			break;
		case WM_CANCEL_DONE:		//Message received from Edit/Delete Button
			if (bValidPointer)
				this->OnCancelAfter();	//Call OnCancelAfter method
			break;
		case WM_BTN_HELP:			//Message received from Clear/New Button
			if (bValidPointer)
				this->OnHelp();			//Call OnHelp method
			break;
		case WM_HELP_DONE:			//Message received from Clear/New Button
			if (bValidPointer)
				this->OnHelpAfter();	//Call OnHelpAfter method
			break;
		case WM_UPDATEDATA:
			UpdateData();
			break;
		case WM_CMS_HAS_PROPERTY_SHEET:
			m_pMasterSheet = DYNAMIC_DOWNCAST(CCMSPropertySheet, CWnd::FromHandlePermanent(pMessage->hwndFrom));
			break;
		case WM_DIRTY:
			if (m_pEXMaintDoc)
				m_pEXMaintDoc->SetModifiedFlag(TRUE);
			break;
		}
	}
	catch (_com_error& e)
	{
		// if problems, handle and display errors. AES
		CGBLForm::HandleError(e);
	}
	if (::IsWindow(pMessage->hwndFrom))
		return __super::OnNotify(wParam, lParam, pResult);
	else
		return TRUE;
}

//*********************************************************
// Function		PreTranslateMessage
// Purpose		Pre-translate messages sent to form
// Parameters	*pMsg - Message
// Returns		true if Message is to continue on
//				false if Message is not to continue on
//*********************************************************
BOOL CCMSFormView::PreTranslateMessage(MSG* pMsg)
{
	//Do we have a PropertySheet?
	if (m_pMasterSheet != NULL)
	{
		//Let's see if it's pre-translate handles this message (CTRL+TAB is one msg it will handle)
		if (m_pMasterSheet->PreTranslateMessage(pMsg))
		{
			return TRUE;
		}
	}

	bool bKeydownDetected = false;
	if (pMsg->message == WM_KEYDOWN)
	{
		bKeydownDetected = true;
	}

	// has the user already interacted with the form
	if (!m_bUserInteracted)
	{
		// check for keyboard activity from the user
		if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
			m_bUserInteracted = true;
		else
		{
			// check for mouse activity from the user
			if (pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST)
			{
				if (pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_LBUTTONUP)

					m_bUserInteracted = true;
			}
		}
	}

	if (CGBLForm::GBLPreTranslateMessage(pMsg, this) == TRUE) return TRUE;
	return __super::PreTranslateMessage(pMsg);
}
//PTR.05.18.2005.1-18865
BOOL CCMSFormView::OnCommand(WPARAM wParam, LPARAM lParam)
{
	CGBLForm::GBLOnCommand(this, wParam, lParam);
	return __super::OnCommand(wParam, lParam);
}
BOOL CCMSFormView::IsPrintEnabled()
{
	return FALSE;
}

BOOL CCMSFormView::IsSaveEnabled()
{
	return FALSE;
}

BOOL CCMSFormView::IsClearEnabled()
{
	return FALSE;
}

void	CCMSFormView::OnKillFocus(CWnd* pNewWnd)
{
	__super::OnKillFocus(pNewWnd);
	CCMSFocusMessage	objMessage(CCMSFocusMessage::LostFocus, this);
	Publish(objMessage);
}

//*********************************************************
// Function		OnSetFocus
// Purpose		Routine called when doing add on the fly
// Parameters	*pOldWnd - hWnd to object losing focus
// Returns		None
//*********************************************************
void CCMSFormView::OnSetFocus(CWnd* pOldWnd)
{
	__super::OnSetFocus(pOldWnd);
	//1-24783 AES causing a assert error on the enter bills form when it opened.
	//CCMSWindowManager::Instance().ResetWindowStates();
	//SetWindowTopmost(true);

	//Defect ID 1-27843, PGP(06-16-2007) and other related Focus Advance After Lookup Selection (FAALS)..:) issues.
	//This code was commented before as part of the issue above. The RestoreFocus call here
	//is necessary for look up selections to advance focus.
	const bool bolStatus = RestoreFocus();
	if (!bolStatus) __super::OnSetFocus(pOldWnd);
	CCMSFocusMessage	objMessage(CCMSFocusMessage::GotFocus, this);
	Publish(objMessage);
	////if (::IsWindow(m_hwLookFocus) && ::IsChild(GetSafeHwnd(), m_hwLookFocus))
	//{
	//	//Set CFormView's focus to NULL.  If Microsoft ever changes this in future releases of
	//	//MFC, Then we'll just need to fix this method.
	//	m_hWndFocus = NULL;
	//
	//	// otherwise, set focus to the appropriate lookup control window.
	//	//::SetFocus(m_hwLookFocus);
	//	::PostMessage(GetSafeHwnd(), WM_NEXTDLGCTL, (WPARAM)m_hwLookFocus, TRUE);
	//	m_hwLookFocus = NULL;
	//
	//	//We will not call base class.  This way we don't fight CFormView's OnSetFocus handling.
	//}
	//else
	//{
	//	if (::IsWindow(m_hWndFocus) && ::IsChild(GetSafeHwnd(), m_hWndFocus))
	//	{
	//		CWnd* pFocus;
	//		pFocus = CWnd::FromHandle(m_hWndFocus);
	//		int nStop;
	//		nStop = 1;
	//	}
	//	m_hwLookFocus = NULL;
	//	__super::OnSetFocus(pOldWnd);
	//}

	////Peter Ringering - 02/21/2005 - This was never running because the message
	////was never mapped in the message map at the top of this file.  I see no
	////reason why setting focus to this form should require a change mode message
	////to be sent to the mainframe.  The maintenance form can do it though.
	////JKC DefectID:1-8773 Need to update StatusBar
	//CGBLForm::SendMessageToMainFrame(WM_CHANGE_MODE, 0);
}
//Peter Ringering - 02/21/2005 - This is sent from the CCMSAutoFillEdit and CAccountControl to
//this form telling it to set focus to it or the next item in the tab order.
//wParam = Set focus to the next control?
//lParam - HWND of the CCMSAutoFillEdit or CAccountControl.
LRESULT CCMSFormView::OnLookReturnFocus(WPARAM wParam, LPARAM lParam)
{
	HWND hwFocus = (HWND)lParam;
	//if (wParam)
	//{
	//	CWnd* pFocus = CWnd::FromHandle(hwFocus);
	//	//We use pFocus's parent.  We can't assume the parent is this formview.  It
	//	//could be that pFocus's parent is a property page or an embedded dialog.
	//	hwFocus = pFocus->GetParent()->GetNextDlgTabItem(pFocus, FALSE)->GetSafeHwnd();
	//}
	//Defect ID 1-26370, PGP(02-18-2008) - I am not sure why the code above is commented, but
	//it does effectively break the tabout for account controls. Now account controls and autofilledits
	//send the same message. so in order to not break autofills, i am turning this on only for account controls.
	CWnd* pWndSender = CWnd::FromHandle(hwFocus);
	if (pWndSender->GetSafeHwnd() && pWndSender->IsKindOf(RUNTIME_CLASS(CAccountControl)))
		pWndSender->GetParent()->PostMessage(WM_NEXTDLGCTL);

	m_hwLookFocus = hwFocus;
	return 0;
}
//*********************************************************
// Function		OnSetFocusLookup
// Purpose		Routine called when lookup is returning a value
// Parameters	*pLookup - hWnd to object lookup
// Returns		None
//*********************************************************
bool CCMSFormView::OnSetFocusFromLookup(CWnd* pLookup)
{
	bool bReturn = false;
	CString szValue = m_clsXMLParams.GetXML();

	if (szValue.Find(_T("PRIMARYKEY"), 0) > 0)
	{
		m_clsXML.ClearControls("ALL");		// 1-13494 BK 8/30/04 when returning from lookup clear all controls before loading data
		m_clsDataSet.LoadFromXML(m_clsXMLParams.GetXMLBSTR());					//load cdataset
		m_clsDataSet.GetTable("PRIMARYKEY", &m_clsTable);						//get key value set
		m_clsTable.GetRow(0, &m_clsDataRow);									//set if the is data in the prams
		if (!m_clsDataRow.IsEmpty())
		{																		//there is something to get
			bReturn = true;														//passed
		}
	}
	m_clsDataSet.Dispose();														//reset dataset
	m_clsXMLParams.Clear();														//reset parms

	return bReturn;
}

//*********************************************************
// Function		OnHelp
// Purpose		Routine called when the Help Button is
//				pressed
// Parameters	None
// Returns		None
//*********************************************************
void CCMSFormView::OnHelp(void)
{
	CWnd* const pBtn = GetDlgItem(IDHELP);
	//sb - if the help button is one of the following types, then it has already launched the help
	if (pBtn && (pBtn->IsKindOf(RUNTIME_CLASS(CCMSHelpButton)) || pBtn->IsKindOf(RUNTIME_CLASS(CCMSHelpButtonEx)))) return;
	CGBLHelp::ShowContextHelp(si);
}

//*********************************************************
// Function		SetAccountSeg
// Purpose		Used to set the account seg and property to
//				send to Crystal
// Parameters	None
// Returns		None
//*********************************************************
void CCMSFormView::SetAccountSegArray(BOOL bPrintCodes)
{
	CString szValue = "";							//used as a variable in this method
	//values for the string array are starting to be set
	szValue.Format(_T("%d"), si->SegmentSetup.Segments[0].Length);
	MakeCommandValue("@nSeg1", szValue, bPrintCodes);

	szValue.Format(_T("%d"), si->SegmentSetup.Segments[1].Length);
	MakeCommandValue("@nSeg2", szValue, bPrintCodes);

	szValue.Format(_T("%d"), si->SegmentSetup.Segments[2].Length);
	MakeCommandValue("@nSeg3", szValue, bPrintCodes);

	szValue.Format(_T("%d"), si->SegmentSetup.Segments[3].Length);
	MakeCommandValue("@nSeg4", szValue, bPrintCodes);

	szValue.Format(_T("%d"), si->SegmentSetup.Segments[4].Length);
	MakeCommandValue("@nSeg5", szValue, bPrintCodes);

	szValue.Format(_T("%d"), si->SegmentSetup.Segments[5].Length);
	MakeCommandValue("@nSeg6", szValue, bPrintCodes);

	szValue.Format(_T("%d"), si->SegmentSetup.TotalAccountLength);
	MakeCommandValue("@nAccountLength", szValue, bPrintCodes);

	MakeCommandValue(_T("@szSegmentChar"), si->SegmentSetup.SeparationCharacter, bPrintCodes);

	szValue.Format(_T("%d"), si->SegmentSetup.NumberOfSegments, bPrintCodes);
	MakeCommandValue(_T("@nSegmentCount"), szValue, bPrintCodes);
}

//*********************************************************
// Function		SetCompanyArray
// Purpose		Used to set the company info to
//				send to Crystal
// Parameters	None
// Returns		None
//*********************************************************
void CCMSFormView::SetCompanyArray(BOOL bPrintCodes)
{
	//start fill the string array with values
	MakeCommandValue("szCompanyName", si->CompanySettings.Name, bPrintCodes);
	MakeCommandValue("szCompanyCode", si->CompanySettings.ID, bPrintCodes);
}

void CCMSFormView::ClearCommandValues(BOOL bPrintCodes)
{
	if (bPrintCodes)
	{
		ASSERT(m_dlgCMSControlButtons.m_pPrintCodes);
		m_dlgCMSControlButtons.m_pPrintCodes->strCommandValues.RemoveAll();
	}
	else
	{
		strCommandValues.RemoveAll();
	}
}

// ********************************************************
// * Procedure:    MakeCommandValue (Overload #1)
// *
// * Description:  Adds the passed in command and Resource ID to strCommands
// *
// * Parameters:   szParamName - Parameter to Name
// *               uinResourceID - Resource ID containing parameter value
// *
// * Date Created: 02/27/2003 by Peter Ringering
// ********************************************************
void CCMSFormView::MakeCommandValue(CString szParamName, UINT uinResourceID, BOOL bPrintCodes)
{
	MakeCommandValue(szParamName, RESSTRING(uinResourceID), bPrintCodes);
}

// ********************************************************
// * Procedure:    MakeCommandValue (Overload #2)
// *
// * Description:  Adds the passed in command and value to strCommands
// *
// * Parameters:   szParamName - Parameter to Name
// *               szParamValue - Parameter Value
// *
// * Date Created: 02/27/2003 by Peter Ringering
// ********************************************************
void CCMSFormView::MakeCommandValue(CString szParamName, CString szParamValue, BOOL bPrintCodes)
{
	if (bPrintCodes && m_dlgCMSControlButtons.m_pPrintCodes)	//sb - 1-22151
	{
		//ASSERT(m_dlgCMSControlButtons.m_pPrintCodes) ;
		m_dlgCMSControlButtons.m_pPrintCodes->MakeCommandValue(szParamName, szParamValue);
	}
	else
	{
		strCommandValues.Add(szParamName);
		strCommandValues.Add(szParamValue);
	}
}

void CCMSFormView::MakeReportCommandValue(CString szParamName, CString szParamValue)
{
	MakeCommandValue(szParamName, szParamValue, FALSE);
}

void CCMSFormView::MakeReportCommandValue(CString szParamName, UINT uiResourceID)
{
	MakeCommandValue(szParamName, RESSTRING(uiResourceID), FALSE);
}

// ********************************************************
// * Procedure:    SetReportTitle
// *
// * Description:  Creates a report title command value for use in setting the print dialog
// *			   caption and the report header.
// *
// * Parameters:   unResource - Resource containing the string.
// *
// * Date Created: 03/27/2003 by Peter Ringering
// ********************************************************
void CCMSFormView::SetReportTitle(UINT unResource, BOOL bPrintCodes)
{
	SetReportTitle(RESSTRING(unResource), bPrintCodes);
}
//------------------------------------------------------------------------------
void CCMSFormView::SetReportTitle(CString szFormTitle, BOOL bPrintCodes)
{
	//This is a repeated function from the CMSFormView Class.  If this form was a CMSFormView it would cause a
	//circular reference.
	CXMLParams clsParams;
	clsParams.MakeParam("REPORTTITLE", szFormTitle);

	CString szModule = si->ApplicationSettings.ModuleStringID;
	// PBI 22186 BK 8/21/14 Added red flag reports, added parameter to show correct module in print options box and on report
	if (!m_szModuleAbbrev.IsEmpty())
		szModule = m_szModuleAbbrev;

	if (szModule == RESSTRING(IDS_CT_MODULE))
		szModule = RESSTRING(IDS_CMS);

	if (szModule == RESSTRING(IDS_SA_MODULE))
		szModule = RESSTRING(IDS_SALES);

	clsParams.MakeParam("MODULE", szModule);
	CString szTitle = clsParams.StringMerge(RESSTRING(IDS_GBL_REPORTTITLE));
	MakeCommandValue("@REPORTTITLE", szTitle, bPrintCodes);
}

// ********************************************************
// * Procedure:    MakeLableValue
// *
// * Description:  Adds the passed in command and value to strCommands
// *
// * Parameters:   szParamName - Parameter to Name
// *               szParamValue - string form resouce
// ********************************************************
void CCMSFormView::MakePramValue(CString szParamName, CString szParamValue)
{
	ASSERT(m_dlgCMSControlButtons.m_pPrintCodes);

	m_dlgCMSControlButtons.m_pPrintCodes->MakeCommandValue(szParamName, szParamValue);
}

//KPM.2005.05.06 This method is never hit. WM_CLOSE is not passed to anything using
//the microsoft document/view architecture.  Use OnFormClose instead.
void CCMSFormView::OnClose()
{
	m_bClosing = true;
	//03/27/2019 CJB - PBI 49101 - First check for a valid pointer
	if (::IsWindow(this->m_hWnd))
		GetParent()->SendMessage(WM_CLOSE, 0, 0);
}

//Account control messages for getfocus and killfocus. PGP(03/24/2003)
LRESULT CCMSFormView::OnGetFocusAcctCtrl(WPARAM wPar, LPARAM lPar)
{
	UINT unControlID = (UINT)wPar;
	UINT unLabelID = 0;
	if (m_clsControlToLabelMap.Lookup(unControlID, unLabelID) != 0)
		GetDlgItem((int)unLabelID)->SetFont(&si->Drawing.Fonts.NormalUnderline);

	return 0;
}

LRESULT CCMSFormView::OnKillFocusAcctCtrl(WPARAM wPar, LPARAM lPar)
{
	UINT unControlID = (UINT)wPar;
	UINT unLabelID = 0;
	if (m_clsControlToLabelMap.Lookup(unControlID, unLabelID) != 0)
		GetDlgItem((int)unLabelID)->SetFont(&si->Drawing.Fonts.Normal);

	return 0;
}

//Adds entries in internal map for finding the label of a control. PGP(03/24/2003)
void CCMSFormView::AddControlToLabelMapEntry(UINT unControlID, UINT unLabelID)
{
	m_clsControlToLabelMap.SetAt(unControlID, unLabelID);
}

int CCMSFormView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

// ********************************************************
// * Procedure:    SearchFor
// *
// * Description:  This funtion is called when the lookup is invoked
// * and what it basically does is if you have selected
// * an item from a lookup then the function will search
// * for the item you had selected and highlight that item
// * in the lookup form when it is called again.
// *
// * Parameters:   CXMLParams* pParams
// *               CString szSQLFieldName - this parameter is the name of the SQL field
// *										from the table where the lookup is made from
// *               CString szControlName - this parameter is the string variable name that is declared
// *									   for the control of the where the lookup is invoked.
// *
// * Date Created: 05/09/2003 by Mike Gregg
// ********************************************************
void CCMSFormView::SearchFor(CXMLParams* pParams, CString szSQLFieldName, CString szControlName)
{
	CXMLParams   clsSearchFor(true);

	//Check to see if starting reference number is blank
	if (szControlName.GetLength() > 0)
	{
		//Fill the search for param and append it to the param that will be sent to the lookup
		clsSearchFor.SetRoot("SEARCHFOR");
		clsSearchFor.MakeParam(szSQLFieldName, szControlName);
		pParams->AppendXMLParam(&clsSearchFor);
	}
}

//Called from CMSDoc when the frame is about to close.
//individual forms should set the m_nFormID variable to
//the desired value as per GLConstants.h. PGP(05/13/2003)
void CCMSFormView::OnFormClose()
{
	if (m_nFormID != -1)
		CGBLForm::SendMessageToMainFrame((UINT)m_nFormID, 0);
}

// Function name   : CCMSFormView::SetAutoFillData
// Description     :
// Return type     : void
// Argument        : CCMSAutoFillEdit* p_obj
// Argument        : UINT uinLabel
// Argument        : UINT uinLookupID
// Argument        : char* pszFormName
// Argument        : char* pszFieldName
// Argument        : CString szDefaultString
// Argument        : char* pszControlID
// Argument        : UINT uinModule
// Argument        : char* pszModuleName
// Argument        : bool bAddOnFly
// Argument        : bool bViewForm
// Argument        : UINT nMode
// Argument        : CCMSAutoFillEdit* pCtrl
void CCMSFormView::SetAutoFillData(CCMSAutoFillEdit* p_obj, UINT uinLookupID, _TCHAR* pszFormName, _TCHAR* pszFieldName, CString szDefaultString, _TCHAR* pszControlID, UINT uinModule, _TCHAR* pszModuleName, bool bAddOnFly, bool bViewForm, int nMode, CWnd* pLabel1, CWnd* pLabel2, CCMSAutoFillEdit* pCtrl)
{
	ASSERT(p_obj);


	if (pCtrl != NULL) // start
	{
		p_obj->SetWndLabel(pLabel1, pLabel2);

	}
	else if (!p_obj->IsStartCrtl() && pCtrl != NULL) // end
	{
		p_obj->SetWndLabel(pLabel1, pLabel2);
	}
	else // normal
	{
		p_obj->SetWndLabel(pLabel1);
	}

	p_obj->SetDataLookupID(uinLookupID);
	p_obj->SetDataFormName(pszFormName);
	p_obj->SetDataFieldName(pszFieldName);
	p_obj->SetDataDefaultString(szDefaultString);
	p_obj->SetDataXMLControlID(pszControlID);
	// DTG, 4.20.2004
	if (pCtrl != NULL)
	{
		p_obj->SetAsStartCtrl();
		p_obj->SetEndCtrlPtr(pCtrl);
	}

	// common data.
	InitCommonAutoFillFields(p_obj
		, uinModule
		, pszModuleName
		, &m_bIsAutofill
		, &m_bIsLookup
		, &nMode
		, &m_clsXML
		, &m_clsXMLParams
		, &m_pLookup
		, bAddOnFly
		, bViewForm);

}

// Function name   : CCMSFormView::InitCommonAutoFillFields
// Description     : Set common auto fill data
// Return type     : void
// Argument        : CCMSAutoFillEdit* pAFEdit
// Argument        : UINT uinModule
// Argument        : _TCHAR* pszModuleName
// Argument        : bool* p_bIsAutoFill
// Argument        : bool* p_bIsLookup
// Argument        : int* p_nMode
// Argument        : CXML* pXml
// Argument        : CXMLParams* pXmlParams
// Argument        : CLookup** pLookup
// Argument        : bool bAddOnFly
// Argument        : bool bViewForm
void CCMSFormView::InitCommonAutoFillFields(CCMSAutoFillEdit* pAFEdit, UINT uinModule, _TCHAR* pszModuleName, bool* p_bIsAutoFill, bool* p_bIsLookup, int* p_nMode, CXML* pXml, CXMLParams* pXmlParams, CLookup** pLookup, bool bAddOnFly, bool bViewForm)
{
	ASSERT(pAFEdit);

	pAFEdit->SetDataModuleID(uinModule);
	pAFEdit->SetDataModuleName(pszModuleName);

	pAFEdit->SetFlagAutoFill(p_bIsAutoFill);
	pAFEdit->SetFlagLookup(p_bIsLookup);

	pAFEdit->SetFlagMode(p_nMode);

	pAFEdit->SetDataPtrXML(pXml);
	pAFEdit->SetDataPtrXMLParams(pXmlParams);
	pAFEdit->SetDataPtrLookup(pLookup);
	pAFEdit->m_bAddOnFly = bAddOnFly;
	pAFEdit->m_bViewForm = bViewForm;
}
//
//inline LRESULT CCMSFormView::GetActivePageIndex(WPARAM wParam, LPARAM lParam)
//{
//	UNREFERENCED_PARAMETER(wParam);
//	UNREFERENCED_PARAMETER(lParam);
//
//	if (!m_pPropertySheetInterface) return 0;
//
//	return (LRESULT) m_pPropertySheetInterface->GetActivePageIndex();
//}
//
//inline LRESULT CCMSFormView::GetActivePropPage(WPARAM wParam, LPARAM lParam)
//{
//	UNREFERENCED_PARAMETER(wParam);
//	UNREFERENCED_PARAMETER(lParam);
//
//	if (!m_pPropertySheetInterface) return 0;
//
//	CPropertyPage* pPage = m_pPropertySheetInterface->GetActivePagePtr();
//
//	return (LRESULT)(UINT) pPage->SendMessage(WM_GETHELPID);
//}

LRESULT CCMSFormView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	CGBLForm::GBLWindowProc(message, wParam, lParam);

	return __super::WindowProc(message, wParam, lParam);
}

//**************************************************************************
//* Procedure:     SetInitSize
//*
//* Description:   Sets the initial size of this form view based on the right
//*                bound and bottom bound parameters.
//*
//* Parameters:    pRightBound - Control on the right of this form to size
//*                  the form to.
//*                pBottomBound - Control on the bottom of this form to size
//*                  the form to.
//*
//* Date Created:  08/18/2004 by Peter Ringering (1-11049)
//**************************************************************************
void CCMSFormView::SetInitSize(CWnd* pRightBound, CWnd* pBottomBound)
{
	if (m_bEXMode)
		return;

	ASSERT(pRightBound);
	ASSERT(pBottomBound);

	CRect rcThis, rcRight, rcBottom, rcFrame;
	GetClientRect(rcThis);
	CGBLForm::GetTrueWindowRect(this, pRightBound, rcRight);
	CGBLForm::GetTrueWindowRect(this, pBottomBound, rcBottom);

	int nDeltaX = 0;
	int nDeltaY = 0;
	int nThisHeight = rcThis.Height();
	if (rcRight.right > rcThis.Width())
		nDeltaX = rcRight.right - rcThis.Width();

	if (rcBottom.bottom > nThisHeight)
		nDeltaY = rcBottom.bottom - nThisHeight;

	if (!(nDeltaX + nDeltaY))
		return;

	CWnd* pFrame = GetParentFrame();

	CGBLForm::GetTrueWindowRect(pFrame->GetParent(), pFrame, rcFrame);

	if (nDeltaX)
		rcFrame.right += nDeltaX;
	if (nDeltaY)
		rcFrame.bottom += nDeltaY;

	pFrame->MoveWindow(rcFrame);
}

void CCMSFormView::SetInitSize(int nRightBound, int nBottomBound)
{
	CWnd* pRightBound = GetDlgItem(nRightBound);
	CWnd* pBottomBound = GetDlgItem(nBottomBound);
	SetInitSize(pRightBound, pBottomBound);
}

LRESULT	CCMSFormView::OnCMS_InitSizeWnd(WPARAM wParam, LPARAM lParam)
{
	//The base class doesn't need these but derived classes may.
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	CSize sizeInitSize;
	sizeInitSize.cx = 0;
	sizeInitSize.cy = 0;
	SendMessageToDescendants(WM_CMS_INITSIZEWND, 0, (LPARAM)&sizeInitSize, FALSE);
	return 0;
}

//**************************************************************************
//* Procedure:     SetFormDescription
//*
//* Description:   Sets the caption of the top header.  Only works when
//*                m_bEXMode is true.
//*
//* Parameters:    szText - Text to place into the top header's caption.
//* Parameters:    uiStringID - String resource used to set the top header
//*                  control's caption.
//*
//* Date Created:  10/13/2004 by Peter Ringering
//**************************************************************************
void CCMSFormView::SetFormDescription(UINT uiStringID)
{
	ASSERT(m_pEXTopHeader);
	m_pEXTopHeader->SetHeadText(uiStringID);
}

void CCMSFormView::SetFormDescription(const CString& szText)
{
	ASSERT(m_pEXTopHeader);
	m_pEXTopHeader->SetHeadText(szText);
}

//	PBI 36910	10/12/2016	mvt - Added version of SetHeadText to allow control characters
void CCMSFormView::SetFormDescription(const CString& szText, bool bTrim)
{
	ASSERT(m_pEXTopHeader);
	m_pEXTopHeader->SetHeadTextUntrimmed(szText);
}

void CCMSFormView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	if (m_bEXMode)
		CGBLFormWrap::OnSizeCMSFormEx(this);
}

void CCMSFormView::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	__super::OnWindowPosChanging(lpwndpos);

	if (m_bEXMode)
	{
		ModifyStyle(WS_VSCROLL, 0);
		ModifyStyle(WS_HSCROLL, 0);
	}
}

void CCMSFormView::OnCancel()
{
	GetParent()->PostMessage(WM_CLOSE, 0, 0);
}

void CCMSFormView::Notify(CCMSMessage& objMessage)
{
	switch (objMessage.GetClassID())
	{
	case CCMSFVStartupMessageID:
	{
		CCMSDoc* pDoc = (CCMSDoc*)GetDocument();
		if (pDoc->IsMaintDoc())
		{
			m_pEXMaintDoc = (CCMSMaintDoc*)pDoc;
			m_bEXMode = true;
		}

		CCMSFVOnStartupMsg* const pStartMsg = DYNAMIC_DOWNCAST(CCMSFVOnStartupMsg, &objMessage);
		pStartMsg->SetAbort(!OnStartup(pStartMsg->GetLPInput()));
		//PTR.06.15.2005.1-16432
		if (!pStartMsg->GetAbort() && pStartMsg->GetLookupInfo())
			ProcessLookupInfo(pStartMsg->GetLookupInfo());
	}
	break;
	case CCMSMaintSetupMessageID:
	{
		CCMSMaintSetupMsg* const pMsg = DYNAMIC_DOWNCAST(CCMSMaintSetupMsg, &objMessage);
		if (pMsg->GetMsgType() == MaintMsg_OnDBSetup)
			OnDBSetup(pMsg->GetMaintDoc());
		else
			OnAddFieldMaps(pMsg->GetMaintDoc());
	}
	break;
	}
}

void CCMSFormView::OnMaintStartup()
{
	m_pEXMaintDoc->Startup();
}

void CCMSFormView::OnDestroy()
{
	CCMSFocusMessage	objMessage(CCMSFocusMessage::Dying, this);
	Publish(objMessage);
	__super::OnDestroy();
}
//PTR.07.05.2005.1-16330 - Apparently, below code had no effect on the error.  But since it's possible
//we may have future errors.  I'm going to comment out the top code in OnActivateFrame and OnActivateView.
//So, if we get problems in the future, we can see if this will fix it.

////PTR.07.05.2005.1-16330 - Turning off MFC's CFormView focus save process.  CMS has it's own now.
//
////We seem to be fighting Microsoft with CCMSFocusManager.  So, I'm going to completely turn off CFormView's
////focus manager.  All of CFormView's focus management is hinged on SaveFocusControl.  It is the only
////method that sets m_hWndFocus.  When CFormView's OnSetFocus runs, it checks to see if m_hWndFocus is
////valid and if it is, then it sets focus to it.  So, I'm going to fix the code so m_hWndFocus (which
////is a protected member of CFormView) is always set to NULL.  SaveFocusControl is only called by
////CFormView's OnActivateView and OnActivateFrame virtuals.
void CCMSFormView::OnActivateFrame(UINT nState, CFrameWnd* pFrameWnd)
{
	//m_hWndFocus = NULL;

	__super::OnActivateFrame(nState, pFrameWnd);
}
void CCMSFormView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	//Completely overriding CFormView's OnActivateView so it doesn't call SaveFocusControl yet
	//calls it's parent's (CView) virtual
	//m_hWndFocus = NULL;
	//CView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	__super::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

double CCMSFormView::GetDPI()
{
	if (m_dpi < 0)
	{
		CDC* screen = GetDC();
		m_dpi = screen->GetDeviceCaps(LOGPIXELSX);
		ReleaseDC(screen);
	}
	return m_dpi;
}


// Helper function for dealing with hi-res monitor scaling
int CCMSFormView::Scale(int length)
{
	return (int)(length * GetDPI() / 96 + 0.5);
}
