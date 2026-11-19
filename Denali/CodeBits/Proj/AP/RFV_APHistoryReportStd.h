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
 * Initial code - mgregg - adapted by begbert 12-02-2005
 ******************************************************************************/
#pragma once

#include "RFV_APCommonStd.h"

class CRFV_APHistoryReportStd : public CRFV_APCommonStd
{
	DECLARE_DYNCREATE(CRFV_APHistoryReportStd)

protected:
	CRFV_APHistoryReportStd();           // protected constructor used by dynamic creation
	virtual ~CRFV_APHistoryReportStd();

public:
	enum { IDD = IDD_APF_RFV_HISTORY_STD };
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// contains the code to set the control captions
	virtual bool										Validate(void);
	virtual void 										LoadResources();
	virtual void										InitControls();
	virtual void 										SetupLayoutManager();
	virtual void										SetControlSizes();
	virtual void 										SetReportValues(CMSGBL::IReportInstantiator * pReportInstantiator);

	virtual void OnClickedHistoryRadio();

	virtual int											LoadSortByCombo(void);

	virtual int  LoadReportTypeCombo();
	virtual int  LoadTransactionTypeCombo();
	virtual void InitCurrencyEdit(CCMSCurrencyEdit &edit, CString value);
	virtual void 										GetFormBounds(CWnd*& pRightBound, CWnd*& pBottomBound);

	DECLARE_MESSAGE_MAP()

public:

	CCMSCoolButton					m_optHistory;
	CCMSCoolButton					m_optInvoiceToPayment;
	CCMSCoolButton					m_optPaymentToInvoice;
	// for drilldown reporting
	CCMSCoolButton					m_chkIsDrillDown;
	CCMSEdit						m_txtReference;
	void OnEnKillfocusTxtPeriodStart();
	void OnEnKillfocusTxtPeriodEnd();
	void OnEnKillfocusTxtPeriod(COXCoolCtrl<CCMSEdit>& edit);

	void OnEnSetfocusTxtPeriodStart();
	void OnEnSetfocusTxtPeriodEnd();
	void OnEnSetfocusTxtPeriod(COXCoolCtrl<CCMSEdit>& edit);

	void OnEnChangeTxtPeriodStart();
	void OnEnChangeTxtPeriodEnd();
	void OnEnChangeTxtPeriod(COXCoolCtrl<CCMSEdit>& edit, CString& was);

protected:

	// the aging report needs to provide these differently.
	//virtual bool GetAgeByDueDate();
	//virtual CString GetReportName();
	//virtual UINT GetReportTitleID();

public:
	CString							m_PrevPeriodStart;
	CString							m_PrevPeriodEnd;



	CStatic							m_lblDueDate;
	CStatic							m_lblPeriod;
	CStatic							m_lblVendTotDue;
	CStatic							m_lblItemAmtRem;

	CCMSCoolButton					m_chkPageBreakOnVendor;

	CCMSComboBox					m_cboReportType;
	CCMSComboBox					m_cboTransactionType;

	/*COXCoolCtrl<CCMSDateCtrl>		m_txtDueDateStart;
	COXCoolCtrl<CCMSDateCtrl>		m_txtDueDateEnd;*/

	COXCoolCtrl<CCMSEdit>			m_txtPeriodStart;
	COXCoolCtrl<CCMSEdit>			m_txtPeriodEnd;

	COXCoolCtrl<CCMSCurrencyEdit>	m_txtVendTotDueStart;
	COXCoolCtrl<CCMSCurrencyEdit>	m_txtVendTotDueEnd;
	COXCoolCtrl<CCMSCurrencyEdit>	m_txtItemAmtRemStart;
	COXCoolCtrl<CCMSCurrencyEdit>	m_txtItemAmtRemEnd;

	CCMSCoolButton					m_chkVendTotDueExclZero;
	CCMSCoolButton					m_chkItemAmtRemExclZero;
};


