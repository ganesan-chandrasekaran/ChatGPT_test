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
#include "Splash.h"

 //#ifdef _DEBUG
 //#define new DEBUG_NEW
 //#undef THIS_FILE
 //static char THIS_FILE[] = __FILE__;
 //#endif

IMPLEMENT_DYNAMIC(CSplash, CCMSDialog)
/////////////////////////////////////////////////////////////////////////////
// CSplash dialog constructor
//---------------------------------------------------------------------------
CSplash::CSplash()
	: CCMSDialog(CSplash::IDD)
{
	Create(IDD_GBL_DLG_SPLASH);
}

//---------------------------------------------------------------------------
void CSplash::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GBL_LBL_CMSLOGO, m_lblCMSLogo);
	DDX_Control(pDX, IDC_GBL_BTN_CANCEL, m_cmdCancel);

	//	PBI 28141	06/11/2015	mvt - Changed splash logo to static image
	DDX_Control(pDX, IDC_GBL_PIC_DENALI, m_picCougarLogo);
}

//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CSplash, CCMSDialog)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_GBL_BTN_CANCEL, &CSplash::OnBtnCancel)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSplash message handlers
//---------------------------------------------------------------------------
BOOL CSplash::OnInitDialog()
{
	__super::OnInitDialog();

	return TRUE;
}

void CSplash::SetupDisplay()
{
	// set text in command buttons
	m_cmdCancel.SetWindowText(CGBLResources::GetResourceString(IDS_CANCEL));
	// change font to boldLarge on CMS Logo
	//RCG - 06/08/2017 - PBI 39109 - Changed font from BoldLarge
	m_lblCMSLogo.SetFont(&si->Drawing.Fonts.ArialTwelve);
	// set text in labels
	this->SetIcon(CGBLForm::GetSmallModuleIcon(), TRUE);								//set icon from the global resouce AES
	SetDlgItemText(IDC_GBL_LBL_CMSLOGO, _T(" ") + CGBLResources::GetResourceString(IDS_COUGAR_MOUNTAIN));
	SetDlgItemText(IDC_GBL_LBL_COPYRIGHT, CGBLForm::GetCopyright());	//PTR.06.07.2005.1-19678
	SetDlgItemText(IDC_GBL_LBL_RIGHTSRESERVED, CGBLResources::GetResourceString(IDS_ALL_RIGHTS_RESERVED));
	SetDlgItemText(IDC_GBL_LBL_INITIALIZING, CGBLResources::GetResourceString(IDS_SPLASH_INITIALIZING));
	SetDlgItemText(IDS_LBL_COPYRIGHTWARNING, CGBLResources::GetResourceString(IDS_COPYRIGHT_WARNING));
	SetDlgItemText(IDC_GBL_LBL_INITIALIZINGINFO, m_szInitializingInfo);
	m_szExitingMessage = CGBLResources::GetResourceString(IDS_EXIT_APP);
	switch (si->ApplicationSettings.ModuleID)
	{
	case IDS_AP_MODULE:
		this->SetWindowText(CGBLResources::GetResourceString(IDS_COUGAR_MOUNTAIN_AP));
		break;

	case IDS_AR_MODULE:
		this->SetWindowText(CGBLResources::GetResourceString(IDS_COUGAR_MOUNTAIN_AR));
		break;

	case IDS_BR_MODULE:
		this->SetWindowText(CGBLResources::GetResourceString(IDS_COUGAR_MOUNTAIN_BR));
		break;

	case IDS_CT_MODULE:
	case IDS_GB_MODULE:
	{
		CString szTitle = CGBLResources::GetResourceString(IDS_MAIN_MENU);
		this->SetWindowText(szTitle.Left(szTitle.GetLength() - 2));
	}
	break;

	case IDS_GL_MODULE:
		this->SetWindowText(CGBLResources::GetResourceString(IDS_COUGAR_MOUNTAIN_GL));
		break;

	case IDS_IN_MODULE:
		this->SetWindowText(CGBLResources::GetResourceString(IDS_COUGAR_MOUNTAIN_IN));
		break;

	case IDS_JC_MODULE:
		this->SetWindowText(CGBLResources::GetResourceString(IDS_COUGAR_MOUNTAIN_JC));
		break;

	case IDS_PO_MODULE:
		this->SetWindowText(CGBLResources::GetResourceString(IDS_COUGAR_MOUNTAIN_PO));
		break;

	case IDS_PS_MODULE:
		this->SetWindowText(CGBLResources::GetResourceString(IDS_COUGAR_MOUNTAIN_PS));
		break;

	case IDS_PR_MODULE:
		this->SetWindowText(CGBLResources::GetResourceString(IDS_COUGAR_MOUNTAIN_PR));
		break;

	case IDS_OE_MODULE:
		this->SetWindowText(CGBLResources::GetResourceString(IDS_COUGAR_MOUNTAIN_OE));
		break;

	case IDS_SA_MODULE:
	case IDS_SAE_MODULE:
		this->SetWindowText(CGBLResources::GetResourceString(IDS_COUGAR_MOUNTAIN_SA));
		break;
	}
}

//---------------------------------------------------------------------------
void CSplash::OnBtnCancel()
{
	si->CancelApplicationLoad = true;
	m_cmdCancel.EnableWindow(false);
	SetDlgItemText(IDC_GBL_LBL_INITIALIZINGINFO, m_szExitingMessage);
	this->Invalidate();
	//PostQuitMessage(0);
	DoEvents();
	//EndDialog need not be called from here,
	//thread calls hideform which will call enddialog PGP(05/29/2003)
//	__super::EndDialog(0);
}

void CSplash::DoEvents()
{
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, NULL, NULL, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

//---------------------------------------------------------------------------
bool CSplash::HideForm()
{
	EndDialog(0);
	//__super::OnCancel();
	//return m_bLoaded;
	return true;
}

//---------------------------------------------------------------------------
// call to update message label as status progresses
void CSplash::UpdateStatus()
{
	SetDlgItemText(IDC_GBL_LBL_INITIALIZINGINFO, m_szInitializingInfo); //_T("  " + CGBLResources::GetResourceString(uinID)));
	DoEvents();
}

//---------------------------------------------------------------------------
// set color on CMS Logo
HBRUSH CSplash::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	if (nCtlColor == CTLCOLOR_STATIC
		&& pWnd->GetDlgCtrlID() == IDC_GBL_LBL_CMSLOGO)
	{
		pDC->SetTextColor(si->Drawing.Colors.White);
		//RCG - 06/08/2017 - PBI 39109 - Changed background color from Black but improvised since Colors and Brushes for MineShaft are not available here. 
		pDC->SetBkColor(RGB(45, 45, 45));	//MineShaft
		return CreateSolidBrush(RGB(45, 45, 45));	//MineShaft
	}
	return hbr;
}

//---------------------------------------------------------------------------
// create rectangle to place bitmap in so that bitmap can be stretched to
// correct size
void CSplash::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	//	PBI 28141	06/11/2015	mvt - Changed splash logo to static image
	CRect rect;
	this->GetClientRect(rect);
	//rect.top  = rect.top + 3;
	//rect.left = rect.left + 6;
	//rect.bottom = rect.bottom - 6;
	//rect.right = rect.left + 135;

	//CGBLDrawing::DrawStretchedBitmap(si->Drawing.Bitmaps.CougarHeadSplashLg, dc, rect);
	// Do not call CDialog::OnPaint() for painting messages

	m_picCougarLogo.SetBitmap(CMS::GetSI()->Drawing.Bitmaps.CougarHeadSplashLg);
}

//---------------------------------------------------------------------------
