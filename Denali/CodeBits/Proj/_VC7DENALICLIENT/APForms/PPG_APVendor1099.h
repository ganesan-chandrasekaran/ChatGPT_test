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
#pragma once
#include "./resource.h"
#include "./GRD_APVendor1099.h"

 // CPPG_APVendor1099 dialog

class CPPG_APVendor1099 : public CCMSPropertyPage
{
	DECLARE_DYNAMIC(CPPG_APVendor1099)

public:
	CPPG_APVendor1099();
	virtual ~CPPG_APVendor1099();

	// Dialog Data
	enum { IDD = IDD_APF_PP_VENDOR_1099 };

	bool Validate();

private:
	void InitGrouperControls();
	void FormatFedID();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void InitGrid(void);
	void LoadResources();
	void SetControlProperties();

	//Standard Edit Ctrls
	COXCoolCtrl<COXEdit>	m_txtFederalID;

	//Combobox Ctrl
	COXCoolCtrl<CCMSComboBox>	m_cboDefaultType;

	//Check box ctrl
	COXCoolCtrl<CCMSCoolButton>	m_chk2TIN;
	COXCoolCtrl<CCMSCoolButton>	m_chkOver5000;
	//	PBI 30667	12/1/2015	mvt - Added FATCA Filing checkbox
	COXCoolCtrl<CCMSCoolButton> m_chkFATCAFiling;

	//Radio Buttons
	COXCoolCtrl<CCMSCoolButton>	m_rdoBusiness;
	COXCoolCtrl<CCMSCoolButton>	m_rdoIndividual;

	CCMSGrouper			m_clsGrouperType;

	//CString vars
	CString		m_szFederalID;
	CString		m_szDefaultType;

	//Integer vars
	INT32		m_n2TIN;
	INT32		m_nOver5000;

	//	PBI 30667	12/1/2015	mvt - Added FATCA Filing checkbox
	INT32		m_nFATCAFiling;

	//Radio button integer variable
	INT32		m_nrdoAcctType;

	DECLARE_MESSAGE_MAP()

	BOOL	m_bGridAttached;

public:
	virtual BOOL OnInitDialog();
	void FillControlMap(CXML* pXML);
	void SetArrayValues(int nType, CStringArray& arszCommandValues);
	void ChangeMode(enmFormModes nMode);

	CString  GetCurrentYearSum();

	afx_msg void OnEnSetfocusApfTxtFedId();

	CGRD_APVendor1099		m_cls1099Grid;
};
