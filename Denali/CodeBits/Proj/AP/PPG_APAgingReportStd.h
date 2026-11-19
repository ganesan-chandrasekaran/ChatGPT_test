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
 *******************************************************************************
 *	Created:	10/13/2006
 *	Author:		Andy Swanson
 *  Version 2.5 Feature 12
 ******************************************************************************/
#pragma once

#include "PPG_APTotalDebtReportStd.h"
 // InTimeTec: 12.15.2012.HS - Custom Reports
#include "..\gblforms\CMSCustomOptions.h"
// InTimeTec: End

class CPPG_APAgingReportStd : public CPPG_APTotalDebtReportStd
{
	DECLARE_DYNCREATE(CPPG_APAgingReportStd)

public:
	CPPG_APAgingReportStd();           // protected constructor used by dynamic creation
	virtual ~CPPG_APAgingReportStd();


	enum { IDD = IDD_AP_PP_AGED_TOTAL_DEBT };
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//virtual void AdjustDialog();
	//virtual BOOL										OnInitDialog();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// contains the code to set the control captions
	virtual void 										LoadResources();
	virtual void										InitControls();
	virtual void 										SetupLayoutManager();
	virtual void										SetControlSizes();
	virtual void 										SetReportValues(CMSGBL::IReportInstantiator * pReportInstantiator);

	virtual int  LoadReportTypeCombo();

	virtual void OnClickedAgeByDate();

	// provided for the Total Debt report.
	virtual bool GetAgeByDueDate();
	virtual CString GetReportName();
	virtual UINT GetReportTitleID();


	DECLARE_MESSAGE_MAP()

public:

	CCMSCoolButton					m_optAgeByDueDate;
	CCMSCoolButton					m_optAgeByInvoiceDate;

	// InTimeTec: 10.23.2012.HS - Custom Reports
	CButton							m_chkUseCustomReport;
	CCMSComboBox					m_cboCustomReports;
	virtual void 					GetFormBounds(CWnd*& pRightBound, CWnd*& pBottomBound);
	virtual void					AdjustDialog(CWnd* pBottomControlWnd = NULL);
	void							EnableCustomReportCombo();
	CMSCustomOptions				m_clsCustomOptions;
	// InTimeTec: End
};
