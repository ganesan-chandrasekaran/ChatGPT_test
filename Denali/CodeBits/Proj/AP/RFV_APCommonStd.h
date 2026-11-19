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

#include "../GBLForms/ReportFormDrillDownView.h"
 //#include "../GBLForms/gblreportuserdefinesstd.h"

class AFX_EXT_CLASS CRFV_APCommonStd : public CReportFormDrillDownView//, public CGBLReportUserDefinesStd
{
public:
	// convenience helper functions, mostly to shorten/clean up code:
	static inline CString GetResStr(int id) { return CGBLResources::GetResourceString((UINT)id); }

	static inline int AddIDSToCombo(CComboBox& combo, int id, int index = -1)
	{	// Adds the give IDS string to the combo, with the IDS id as its itemData.
		// returns the position of the added string.
		index = combo.InsertString(index, GetResStr(id));
		combo.SetItemData(index, (DWORD_PTR)id);
		return index;
	}

	static inline int AddIDSToComboN(CComboBox& combo, int id, int index = -1)
	{	// Adds the give IDS string to the combo, with the its row number as its itemData.
		// returns the position of the added string.
		index = combo.InsertString(index, GetResStr(id));
		combo.SetItemData(index, (DWORD_PTR)index);
		return index;
	}

	//	static inline int AddIDSToComboAs(CComboBox& combo, int id, int as, int index = -1)
	//	{	// Adds the give IDS string to the combo, with 'as' as its itemData.
	//		// returns the position of the added string.
	//		index = combo.InsertString(index, GetResStr(id));
	//		combo.SetItemData(index, as);
	//		return index;
	//	}

	enum ESizeModes { eSZ_WIDTH = 1, eSZ_HEIGHT = 2, eSZ_BOTH = 3 };
	void ConformSize(ESizeModes mode, CWnd* w1, CWnd* w2);
	void ConformSize(ESizeModes mode, size_t count, ...);
	//Defect ID 1-31225, PGP(10-02-2009) This function is here so that the base class' OnBnClickedOK
	//functionality can be invoked without relying on m_btnOK. Always operates on IDOK.
	//My advice is to seriously consider avoiding the use of hidden buttons like this to handle base functionality.
	//I have to do this as the m_btnOK IDs keep changing from revision to revision (might be to fix other errors)
	void InvokeIDOK();


	DECLARE_DYNCREATE(CRFV_APCommonStd)

public:
	CRFV_APCommonStd(UINT IDD = NULL);           // protected constructor used by dynamic creation
	virtual ~CRFV_APCommonStd();


#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void OnInitialUpdate();


	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// contains the code to set the control captions
	virtual void 										LoadResources();
	// populates combo boxes and other controls
	virtual void 										InitControls();
	// sets control sizes on controls that resize only at startup. If the user could
	// resize the form these controls would NOT resize
	virtual void 										SetControlSizes();
	// position the controls and sets up resizing rules.
	//	virtual void 										SetupLayoutManager();
	virtual void										LayOutUDFs(CWnd* pAboveStart, CWnd* pAboveEnd);
	// sets the form bounds
	virtual void 										GetFormBounds(CWnd*& pRightBound, CWnd*& pBottomBound) {}
	// this is called after the form has been wrapped. Allows you to fix any
	// minor sizing issues that sometimes can arise.
	virtual void										PostWrapForm() { __super::PostWrapForm(); }
	virtual void 										SetReportValues(CMSGBL::IReportInstantiator * pReportInstantiator);
	//Validates the information of the client form before the parameters are setup and set to the server components to get the data to give to the report.
	virtual bool 										Validate();
	virtual void										InitAutofillControls();
	virtual int											LoadSortByCombo();
	virtual int											LoadVendorStatusCombo();
	virtual void										SetupUDFControls();
	virtual void										PlaceUDFControls();

	DECLARE_MESSAGE_MAP()

public:

	bool												GetUsePageBreak() { return m_bUsePageBreak; }
	void												SetUsePageBreak(bool bUsePageBreak) { m_bUsePageBreak = bUsePageBreak; }
	bool												m_bUsePageBreak;

	// These pointers are useful if you need to place things beneath the UDFs.
	CWnd*												m_pUDFStartPos;
	CWnd*												m_pUDFEndPos;

	CCMSComboBox										m_cboSortBy;
	CCMSComboBox										m_cboVendorStatus;
	CCMSComboBox										m_cboFilter;

	CStatic												m_lblStart;
	CStatic												m_lblEnd;

	CCMSAutoFillEditEx									m_txtARCode;
	CCMSAutoFillEditEx									m_txtVendNumStart;
	CCMSAutoFillEditEx									m_txtVendNumEnd;
	CCMSAutoFillEditEx									m_txtVendNameStart;
	CCMSAutoFillEditEx									m_txtVendNameEnd;
	CCMSEdit											m_txtProdTypeStart;
	CCMSEdit											m_txtProdTypeEnd;
	CCMSAutoFillEditEx									m_txtAPCodeStart;
	CCMSAutoFillEditEx									m_txtAPCodeEnd;

	CCMSCoolButton										m_chkGroupByAPCode;

	CCMSOXCoolButton									m_btnOK;
	CCMSOXCoolButton									m_btnCancel;
	CCMSHelpButton										m_btnHelp;

	CCMSDateCtrlEx										m_txtDueDateStart;
	CCMSDateCtrlEx										m_txtDueDateEnd;

	afx_msg void										OnEnSetFocusNumSortByUDFStart();
	afx_msg void										OnEnSetFocusNumSortByUDFEnd();

	afx_msg void										OnEnSetFocusNumFilterUDFStart();
	afx_msg void										OnEnSetFocusNumFilterUDFEnd();

	afx_msg void										OnCbnSelchangeCboSortby();
	afx_msg void										OnCbnSelchangeCboAddfilter();
};
