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
 ******************************************************************************
 *  Author      Barbara Knuff
 *  Date        01/17/2003
 ******************************************************************************/
#pragma once

#include "resource.h"
#include "afxwin.h"

 /////////////////////////////////////////////////////////////////////////////
 // CSplash dialog
class __declspec(dllexport) CSplash : public CCMSDialog
{
	DECLARE_DYNAMIC(CSplash)
	// Construction
public:
	CSplash();   // standard constructor
	void UpdateStatus();
	bool HideForm();
	void SetupDisplay();
	void DoEvents();
	CString m_szInitializingInfo;
	CString m_szExitingMessage;
	void ShowMessage(LPCTSTR szMsg);

	//	PBI 28141	06/11/2015	mvt - Changed splash logo to static image
	CStatic m_picCougarLogo;
protected:
	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CSplash)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

private:
	// Dialog Data
	enum { IDD = IDD_GBL_DLG_SPLASH };
	CStatic			m_lblCMSLogo;
	COXCoolButton	m_cmdCancel;

	//void LoadResources();
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBtnCancel();
public:
	afx_msg void OnPaint();
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

