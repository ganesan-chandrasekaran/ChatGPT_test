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
#include "../CTForms/resource.h"

 // CCFV_CTMagneticMediaConstants form view

class __declspec(dllexport)CCFV_CTMagneticMediaConstants : public CCMSFormView
{
	DECLARE_DYNCREATE(CCFV_CTMagneticMediaConstants)

protected:
	CCFV_CTMagneticMediaConstants();           // protected constructor used by dynamic creation
	virtual ~CCFV_CTMagneticMediaConstants();
	bool SetButtonControls(void);
	virtual bool Validate(void);					//Validates data entered on form when user clicks ok button
	void LoadResources();					//Use to load string resource form language
	bool m_bGoodLock;
	bool m_RecordSaved;
	bool m_bDontOpen;
	bool ValidateRequired(void);
	void SetTextLimits();
	void LoadMagData();

public:
	enum { IDD = IDD_CMS_MAGNETIC_MEDIA_CONSTANTS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate();
	virtual void PostNcDestroy();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg void OnEnSetfocusEditEmployerId();
	afx_msg void OnBnClickedCtfRdoPostal();
	afx_msg void OnBnClickedCtfRdoEmail();
	afx_msg void OnEnChangeCftTxtExtension();
	afx_msg void OnEnChangeEditEmployerId();
	afx_msg void OnEnChangeEditTransmitterName();
	// Test for adding new controls
	afx_msg void OnEnChangeEditFirstName();
	// Remove upto this after test
	afx_msg void OnEnChangeEditStreetAddr();
	afx_msg void OnEnChangeEditCity();
	afx_msg void OnEnChangeEditState();
	afx_msg void OnEnChangeEditCountry();
	afx_msg void OnEnChangeEditZipCode();
	afx_msg void OnEnChangeEditZipCodeExt();
	afx_msg void OnEnChangeEditFileReturnName();
	afx_msg void OnEnChangeEditFileReturnAddr();
	afx_msg void OnEnChangeEditCity2();
	afx_msg void OnEnChangeEditState2();
	afx_msg void OnEnChangeEditZipCode2();
	afx_msg void OnEnChangeEditCountryReturn();
	afx_msg void OnEnChangeEditTcc();
	afx_msg void OnEnChangeEditPhoneNumber();
	afx_msg void OnEnChangeEditZipCodeExt2();
	afx_msg void OnEnChangeCftTxtPin();
	afx_msg void OnEnChangeEditStateprovince();
	afx_msg void OnEnChangeEditForeignpostal();
	afx_msg void OnEnChangeEditStateprovince1();
	afx_msg void OnEnChangeEditForeignpostal1();
	afx_msg void OnEnChangeEditContactname();
	afx_msg void OnEnChangeEditFax();
	afx_msg void OnEnChangeEditEmail1();

	DECLARE_MESSAGE_MAP()

	void	FillControlMap();						//method used to fill the control map(data binding) aES
	void	SetArrayValues(int nType = NULL);		//method used to fill a array of value from the controls to send to the XML class for data Manipulation AES
	virtual void SetDirty(bool bolDirty = true) { if (!m_bDirty) m_bDirty = bolDirty; }
	virtual bool IsDirty() { return m_bDirty; }    //override to provide dirty state
	virtual void SaveChanges();//Override to call save.
	bool m_bDirty;
	void	CallXMLChangeData();

public:
	COXSeparator m_sepTop2;
	COXSeparator m_sepTop3;
	CCMSEdit m_txtEmployerID;
	CString m_szEmployerID;
	CCMSEdit m_txtTransmitterName;
	CString m_szTransmitterName;
	// Test for adding new controls
	CCMSEdit m_txtFirstName;
	CString  m_szFirstName;
	// Remove upto this after test
	CCMSEdit m_txtTransmitterAddress;
	CString m_szTransmitterAddress;
	CCMSEdit m_txtTransmitterCity;
	CString m_szTransmitterCity;
	CCMSEdit m_txtTransmitterState;
	CString m_szTransmitterState;
	CCMSEdit m_txtTransmitterCountry;
	CString m_szTransmitterCountry;
	CCMSEdit m_txtTransmitterPostalCode;
	CString m_szTransmitterPostalCode;
	CCMSEdit m_txtTransmitterPostalExt;
	CString m_szTransmitterPostalExt;
	CCMSEdit m_txtReturnName;
	CString m_szReturnName;
	CCMSEdit m_txtReturnAddress;
	CString m_szReturnAddress;
	CCMSEdit m_txtReturnCity;
	CString m_szReturnCity;
	CCMSEdit m_txtReturnState;
	CString m_szReturnState;
	CCMSEdit m_txtReturnPostalCode;
	CString m_szReturnPostalCode;
	CCMSEdit m_txtReturnCountry;
	CString m_szReturnCountry;
	CCMSEdit m_txtControlCode;
	CString m_szControlCode;
	//JKC DefectID: 1-8514 Change to COXMaskedEdit Ctrl
	// 1-13591, DTG, 9.7.2004, changed from COXMaskedEdit.
	COXCoolCtrl<CCMSPhoneNumberEdit> m_txtPhoneNumber;
	CString m_szPhoneNumber;
	CCMSEdit m_txtReturnPostalExtension;
	CString m_szReturnPostalExtension;
	CCMSCoolButton	m_rdoEMail;
	int m_nEmail;
	CCMSCoolButton	m_rdoPostal;
	int m_nPostal;
	CCMSGrouper			m_GrouperPreferred;
	CCMSEdit m_txtPin;
	CString m_szPin;
	CCMSEdit m_txtTransStateProvince;
	CString m_szTransStateProvince;
	CCMSEdit m_txtTransForeignPostal;
	CString m_szTransForeignPostal;
	CCMSEdit m_txtRetStateProvince;
	CString m_szRetStateProvince;
	CCMSEdit m_txtRetForeignPostal;
	CString m_szRetForeignPostal;
	CCMSEdit m_txtContactName;
	CString m_szContactName;
	//JKC DefectID: 1-8514 Change to COXMaskedEdit Ctrl
	// 1-13591, DTG, 9.7.2004, changed from COXMaskedEdit.
	COXCoolCtrl<CCMSPhoneNumberEdit> m_txtContactFax;
	CString m_szContactFax;
	CCMSEdit m_txtContantEMail;
	CString m_szContantEMail;
	CCMSEdit m_txtPhoneExt;
	CString m_szPhoneExt;

private:
	static bool m_bFirstNotification;
	CDialogActionButtons	m_dlgButtons;
	CString FilterForNumerics(const CString& szPhoneNo) const;
};
