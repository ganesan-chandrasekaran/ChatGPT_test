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
// CFV_CTMagneticMediaConstants.cpp : implementation file
//

#include "stdafx.h"
#include "..\cmsdll\cmsutility.h"
#include "CFV_CTMagneticMediaConstants.h"
#include ".\cfv_ctmagneticmediaconstants.h"

// Initializing static member
bool CCFV_CTMagneticMediaConstants::m_bFirstNotification = true;      // bool used for validation of PIN number
// CCFV_CTMagneticMediaConstants

IMPLEMENT_DYNCREATE(CCFV_CTMagneticMediaConstants, CCMSFormView)

CCFV_CTMagneticMediaConstants::CCFV_CTMagneticMediaConstants()
	: CCMSFormView(CCFV_CTMagneticMediaConstants::IDD)
	, m_szEmployerID(_T(""))
	, m_szTransmitterName(_T(""))
	, m_szTransmitterAddress(_T(""))
	, m_szTransmitterCity(_T(""))
	, m_szTransmitterState(_T(""))
	, m_szTransmitterCountry(_T(""))
	, m_szTransmitterPostalCode(_T(""))
	, m_szTransmitterPostalExt(_T(""))
	, m_szReturnName(_T(""))
	, m_szReturnAddress(_T(""))
	, m_szReturnCity(_T(""))
	, m_szReturnState(_T(""))
	, m_szReturnPostalCode(_T(""))
	, m_szReturnCountry(_T(""))
	, m_szControlCode(_T(""))
	, m_szPhoneNumber(_T(""))
	, m_szReturnPostalExtension(_T(""))
	, m_szPin(_T(""))
	, m_szTransStateProvince(_T(""))
	, m_szTransForeignPostal(_T(""))
	, m_szRetStateProvince(_T(""))
	, m_szRetForeignPostal(_T(""))
	, m_szContactName(_T(""))
	, m_szContactFax(_T(""))
	, m_szContantEMail(_T(""))
	, m_szPhoneExt(_T(""))
	, m_bDirty(false)
	, m_nEmail(BST_CHECKED)
	, m_nPostal(BST_UNCHECKED)
{
	m_bDontOpen = false;
	m_bGoodLock = true;
	m_RecordSaved = false;
	m_szSubForm = _T("EAC99ED1-C3D4-4ad8-BD11-3FDA4D440EBE");
}

CCFV_CTMagneticMediaConstants::~CCFV_CTMagneticMediaConstants()
{
	CGBLForm::SendMessageToMainFrame(WM_TB_SAVE_ENABLE, false);					//enable the toolbar buttons
}

void CCFV_CTMagneticMediaConstants::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CTF_ICO_TOP, m_icoTop);
	DDX_Control(pDX, IDC_CTF_SEP_TOP, m_sepTop);
	DDX_Control(pDX, IDC_CVT_SEP2, m_sepTop2);
	DDX_Control(pDX, IDC_CVT_SEP3, m_sepTop3);
	DDX_Control(pDX, IDC_CVT_SEP4, m_sepBottom);
	DDX_Control(pDX, IDC_EDIT_EMPLOYER_ID, m_txtEmployerID);
	DDX_Text(pDX, IDC_EDIT_EMPLOYER_ID, m_szEmployerID);
	DDX_Control(pDX, IDC_EDIT_TRANSMITTER_NAME, m_txtTransmitterName);
	DDX_Text(pDX, IDC_EDIT_TRANSMITTER_NAME, m_szTransmitterName);
	// Test for adding new controls
	DDX_Control(pDX, IDC_EDIT_FIRST_NAME, m_txtFirstName);
	DDX_Text(pDX, IDC_EDIT_FIRST_NAME, m_szFirstName);
	// Remove upto this after test
	DDX_Control(pDX, IDC_EDIT_STREET_ADDR, m_txtTransmitterAddress);
	DDX_Text(pDX, IDC_EDIT_STREET_ADDR, m_szTransmitterAddress);
	DDX_Control(pDX, IDC_EDIT_CITY, m_txtTransmitterCity);
	DDX_Text(pDX, IDC_EDIT_CITY, m_szTransmitterCity);
	DDX_Control(pDX, IDC_EDIT_STATE, m_txtTransmitterState);
	DDX_Text(pDX, IDC_EDIT_STATE, m_szTransmitterState);
	DDX_Control(pDX, IDC_EDIT_COUNTRY, m_txtTransmitterCountry);
	DDX_Text(pDX, IDC_EDIT_COUNTRY, m_szTransmitterCountry);
	DDX_Control(pDX, IDC_EDIT_ZIP_CODE, m_txtTransmitterPostalCode);
	DDX_Text(pDX, IDC_EDIT_ZIP_CODE, m_szTransmitterPostalCode);
	DDX_Control(pDX, IDC_EDIT_ZIP_CODE_EXT, m_txtTransmitterPostalExt);
	DDX_Text(pDX, IDC_EDIT_ZIP_CODE_EXT, m_szTransmitterPostalExt);
	DDX_Control(pDX, IDC_EDIT_FILE_RETURN_NAME, m_txtReturnName);
	DDX_Text(pDX, IDC_EDIT_FILE_RETURN_NAME, m_szReturnName);
	DDX_Control(pDX, IDC_EDIT_FILE_RETURN_ADDR, m_txtReturnAddress);
	DDX_Text(pDX, IDC_EDIT_FILE_RETURN_ADDR, m_szReturnAddress);
	DDX_Control(pDX, IDC_EDIT_CITY2, m_txtReturnCity);
	DDX_Text(pDX, IDC_EDIT_CITY2, m_szReturnCity);
	DDX_Control(pDX, IDC_EDIT_STATE2, m_txtReturnState);
	DDX_Text(pDX, IDC_EDIT_STATE2, m_szReturnState);
	DDX_Control(pDX, IDC_EDIT_ZIP_CODE2, m_txtReturnPostalCode);
	DDX_Text(pDX, IDC_EDIT_ZIP_CODE2, m_szReturnPostalCode);
	DDX_Control(pDX, IDC_EDIT_COUNTRY_RETURN, m_txtReturnCountry);
	DDX_Text(pDX, IDC_EDIT_COUNTRY_RETURN, m_szReturnCountry);
	DDX_Control(pDX, IDC_EDIT_TCC, m_txtControlCode);
	DDX_Text(pDX, IDC_EDIT_TCC, m_szControlCode);
	DDX_Control(pDX, IDC_EDIT_PHONE_NUMBER, m_txtPhoneNumber);
	DDX_Text(pDX, IDC_EDIT_PHONE_NUMBER, m_szPhoneNumber);
	DDX_Control(pDX, IDC_EDIT_ZIP_CODE_EXT2, m_txtReturnPostalExtension);
	DDX_Text(pDX, IDC_EDIT_ZIP_CODE_EXT2, m_szReturnPostalExtension);
	DDX_Control(pDX, IDC_CTF_RDO_EMAIL, m_rdoEMail);
	DDX_Check(pDX, IDC_CTF_RDO_EMAIL, m_nEmail);
	DDX_Control(pDX, IDC_CTF_RDO_POSTAL, m_rdoPostal);
	DDX_Check(pDX, IDC_CTF_RDO_POSTAL, m_nPostal);
	DDX_Control(pDX, IDC_CFT_TXT_PIN, m_txtPin);
	DDX_Text(pDX, IDC_CFT_TXT_PIN, m_szPin);
	DDX_Control(pDX, IDC_EDIT_STATEPROVINCE, m_txtTransStateProvince);
	DDX_Text(pDX, IDC_EDIT_STATEPROVINCE, m_szTransStateProvince);
	DDX_Control(pDX, IDC_EDIT_FOREIGNPOSTAL, m_txtTransForeignPostal);
	DDX_Text(pDX, IDC_EDIT_FOREIGNPOSTAL, m_szTransForeignPostal);
	DDX_Control(pDX, IDC_EDIT_STATEPROVINCE1, m_txtRetStateProvince);
	DDX_Text(pDX, IDC_EDIT_STATEPROVINCE1, m_szRetStateProvince);
	DDX_Control(pDX, IDC_EDIT_FOREIGNPOSTAL1, m_txtRetForeignPostal);
	DDX_Text(pDX, IDC_EDIT_FOREIGNPOSTAL1, m_szRetForeignPostal);
	DDX_Control(pDX, IDC_EDIT_CONTACTNAME, m_txtContactName);
	DDX_Text(pDX, IDC_EDIT_CONTACTNAME, m_szContactName);
	DDX_Control(pDX, IDC_EDIT_FAX, m_txtContactFax);
	DDX_Text(pDX, IDC_EDIT_FAX, m_szContactFax);
	DDX_Control(pDX, IDC_EDIT_EMAIL1, m_txtContantEMail);
	DDX_Text(pDX, IDC_EDIT_EMAIL1, m_szContantEMail);
	DDX_Control(pDX, IDC_CFT_TXT_EXTENSION, m_txtPhoneExt);
	DDX_Text(pDX, IDC_CFT_TXT_EXTENSION, m_szPhoneExt);
}

BEGIN_MESSAGE_MAP(CCFV_CTMagneticMediaConstants, CCMSFormView)
	ON_EN_SETFOCUS(IDC_EDIT_EMPLOYER_ID, OnEnSetfocusEditEmployerId)
	ON_BN_CLICKED(IDC_CTF_RDO_POSTAL, OnBnClickedCtfRdoPostal)
	ON_BN_CLICKED(IDC_CTF_RDO_EMAIL, OnBnClickedCtfRdoEmail)
	ON_EN_CHANGE(IDC_CFT_TXT_EXTENSION, OnEnChangeCftTxtExtension)
	ON_EN_CHANGE(IDC_EDIT_EMPLOYER_ID, OnEnChangeEditEmployerId)
	ON_EN_CHANGE(IDC_EDIT_TRANSMITTER_NAME, OnEnChangeEditTransmitterName)
	// Test for adding new controls
	ON_EN_CHANGE(IDC_EDIT_FIRST_NAME, OnEnChangeEditFirstName)
	// Remove upto this after test
	ON_EN_CHANGE(IDC_EDIT_STREET_ADDR, OnEnChangeEditStreetAddr)
	ON_EN_CHANGE(IDC_EDIT_CITY, OnEnChangeEditCity)
	ON_EN_CHANGE(IDC_EDIT_STATE, OnEnChangeEditState)
	ON_EN_CHANGE(IDC_EDIT_COUNTRY, OnEnChangeEditCountry)
	ON_EN_CHANGE(IDC_EDIT_ZIP_CODE, OnEnChangeEditZipCode)
	ON_EN_CHANGE(IDC_EDIT_ZIP_CODE_EXT, OnEnChangeEditZipCodeExt)
	ON_EN_CHANGE(IDC_EDIT_FILE_RETURN_NAME, OnEnChangeEditFileReturnName)
	ON_EN_CHANGE(IDC_EDIT_FILE_RETURN_ADDR, OnEnChangeEditFileReturnAddr)
	ON_EN_CHANGE(IDC_EDIT_CITY2, OnEnChangeEditCity2)
	ON_EN_CHANGE(IDC_EDIT_STATE2, OnEnChangeEditState2)
	ON_EN_CHANGE(IDC_EDIT_ZIP_CODE2, OnEnChangeEditZipCode2)
	ON_EN_CHANGE(IDC_EDIT_COUNTRY_RETURN, OnEnChangeEditCountryReturn)
	ON_EN_CHANGE(IDC_EDIT_TCC, OnEnChangeEditTcc)
	ON_EN_CHANGE(IDC_EDIT_PHONE_NUMBER, OnEnChangeEditPhoneNumber)
	ON_EN_CHANGE(IDC_EDIT_ZIP_CODE_EXT2, OnEnChangeEditZipCodeExt2)
	ON_EN_CHANGE(IDC_CFT_TXT_PIN, OnEnChangeCftTxtPin)
	ON_EN_CHANGE(IDC_EDIT_STATEPROVINCE, OnEnChangeEditStateprovince)
	ON_EN_CHANGE(IDC_EDIT_FOREIGNPOSTAL, OnEnChangeEditForeignpostal)
	ON_EN_CHANGE(IDC_EDIT_STATEPROVINCE1, OnEnChangeEditStateprovince1)
	ON_EN_CHANGE(IDC_EDIT_FOREIGNPOSTAL1, OnEnChangeEditForeignpostal1)
	ON_EN_CHANGE(IDC_EDIT_CONTACTNAME, OnEnChangeEditContactname)
	ON_EN_CHANGE(IDC_EDIT_FAX, OnEnChangeEditFax)
	ON_EN_CHANGE(IDC_EDIT_EMAIL1, OnEnChangeEditEmail1)
END_MESSAGE_MAP()

// CCFV_CTMagneticMediaConstants message handlers

void CCFV_CTMagneticMediaConstants::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	m_rdoEMail.SetRadioValue(0);								//set rdo values
	m_rdoPostal.SetRadioValue(1);
	m_GrouperPreferred.SetGroupType(CCMSGrouper::INT_GROUP);	// set the internal representation type of the group as a integer type
	m_GrouperPreferred.SetParentWindow(this);					// identify the hWnd of the current form - we insure that form exists before acting on any of it's controls
	m_GrouperPreferred.AddControl(IDC_CTF_RDO_EMAIL);			// add the control id to the CMSGrouper's CArray, so that we can access the dialog item on the form
	m_GrouperPreferred.AddControl(IDC_CTF_RDO_POSTAL);			// add the control id to the CMSGrouper's CArray, so that we can access the dialog item on the form
	m_GrouperPreferred.InitDefault(1, _T(""));					// set the 'Normal' or default value for the group
	m_GrouperPreferred.SetDefault();

	SetButtonControls();										//Set the control buttons on form AES
	LoadResources();											//load string from lang dll AES
	SetTextLimits();											//set text limits AES
	FillControlMap();											//fill control in xml
	LoadMagData();												//load data form server
	m_txtEmployerID.SetFocus();
	m_txtEmployerID.SetSel(0, -1);
	CGBLForm::SendMessageToMainFrame(WM_TB_SAVE_ENABLE, true);					//enable the toolbar buttons
	CGBLForm::SendMessageToMainFrame(WM_TB_CLEAR_ENABLE, false);					//enable the toolbar buttons
	CGBLForm::SendMessageToMainFrame(WM_TB_PRINT_ENABLE, false);

	m_bFirstNotification = true;

	//SET CHILDFRAME TO SIZE OF THE DIALOG AES
	CGBLForm::ResizeToFit(this);
}

/****************************************************************
	LoadMagData - Loads data for the sever

	Called -			OnInitialUpdate
//****************************************************************/
void CCFV_CTMagneticMediaConstants::LoadMagData()
{
	// begbert 10-19-2009 1-33583 : Switched to use the CMS_ElectronicConstants company table
	if (!m_clsXML.GetXMLData(RECORD_CURRENT, _T("CT"), _T("CMS_ElectronicConstants"), &strCommandValues, _T("0"), _T(""))) {
		if (m_clsXML.m_bServerError) {						//if there is a server error do not open
			m_bDontOpen = true;								// use this member for that
		}
		// EK - 6/10/3 - 1-5761 - We do not lock record because of error: set GoodLock to false
		m_bGoodLock = false;								    //was not lock, used for unlock on PostDestory
	}
	else {													//good load so lock record
	   // begbert 10-19-2009 1-33583 : Switched to use the CMS_ElectronicConstants company table
		if (!m_clsXML.LockRecord(RECORD_LOCK, _T("CT"), _T("CMS_ElectronicConstants"), &strCommandValues)) {
			m_dlgCMSReportButtons.m_btnOk.EnableWindow(false);//record would not lock so disable control, be still show form
			m_clsXML.EnableDisable(DISABLE, _T("0"));
			m_bGoodLock = false;								//was not lock, used for unlock on PostDestory

		}
	}

	UpdateData(TRUE);
	m_bDirty = false;
}

/****************************************************************
	SetTextLimits - used to set the text limits for the cedit control

	Called -			OnInitialUpdate
//****************************************************************/
void CCFV_CTMagneticMediaConstants::SetTextLimits()
{
	m_txtEmployerID.SetLimitText(10);

	m_txtTransmitterName.SetLimitText(50);
	// Test for adding new controls
	m_txtFirstName.SetLimitText(50);  // Test for adding new controls
	// Remove upto this after test	
	m_txtTransmitterAddress.SetLimitText(50);
	m_txtTransmitterCity.SetLimitText(25);
	m_txtTransmitterState.SetLimitText(2);
	m_txtTransmitterPostalCode.SetLimitText(5);
	m_txtTransmitterPostalExt.SetLimitText(4);

	m_txtReturnName.SetLimitText(50);
	m_txtReturnAddress.SetLimitText(50);
	m_txtReturnCity.SetLimitText(25);
	m_txtReturnState.SetLimitText(2);
	m_txtReturnPostalCode.SetLimitText(5);
	m_txtReturnPostalExtension.SetLimitText(4);

	m_txtControlCode.SetLimitText(5);	// begbert 03-30-2007 1-27047 : Corrected from 6 to 5
	m_txtPhoneNumber.SetLimitText(30);	//sb.1-15437
	m_txtPin.SetLimitText(50);
	m_txtTransmitterCountry.SetLimitText(2);
	m_txtTransStateProvince.SetLimitText(23);
	m_txtTransForeignPostal.SetLimitText(15);
	m_txtReturnCountry.SetLimitText(2);
	m_txtRetStateProvince.SetLimitText(23);
	m_txtRetForeignPostal.SetLimitText(15);
	m_txtContactName.SetLimitText(27);
	m_txtPhoneExt.SetLimitText(5);
	m_txtContactFax.SetLimitText(30);		//sb.1-15437
	m_txtContantEMail.SetLimitText(80);		//01/10/2007 CJB - 1-25109  //Updated 1099 Magnetic Media for Tax Year 2006.
}

/****************************************************************
	SetButtonControls - Loads the control button variable for the dlg,
						to use the XML class in the CMSDLL. AES

	Called -			OnInitialUpdate
//****************************************************************/
bool CCFV_CTMagneticMediaConstants::SetButtonControls(void)
{
	bool bReturn = false; //default return AES
	try
	{
		if (m_dlgButtons.BindControl(GetDlgItem(IDC_CTF_BUTTONHOLDER), true))			//bind button to dialog AES
		{
			//changes to the button for record movement in the buttons controls AES
			// m_dlgCMSReportButtons.m_szModule=_T("GL");						//Set Module AES
			//m_dlgCMSReportButtons.m_clsXML=&m_clsXML;					//pass XML class to pointer
			 //m_dlgCMSReportButtons.m_lContextHelpID=0x001;				//set context help id AES
			bReturn = true;
		}
	}
	catch (_com_error& e)
	{
		// if problems, handle and display errors. AES
		CGBLForm::HandleError(e);
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
// Validate [virtual] -
bool CCFV_CTMagneticMediaConstants::Validate(void)
{
	return false;							//send a false so use button at form level, the button is a report button and I don't want to run a report. AES
}

/////////////////////////////////////////////////////////////////////////////
// LoadResources - uses the Language DLL to load the displayed strings for whatever
//                  language the client is using
//
void CCFV_CTMagneticMediaConstants::LoadResources()
{
	m_Wnd = this;
	//Set Text for all the Labels on the Form jkl
	CGBLForm::SetFormTitle(this, CGBLResources::GetResourceString(IDS_CMS_MAGNETIC_MEDIA_CONSTANTS));
	m_icoTop.SetIcon(si->Drawing.Icons.GenerateElectronic());
	SetResourceText(IDC_CTF_TOPHEADER, IDS_MAGNETIC_MEDIA_HEADER);
	SetResourceText(IDC_STATIC_EMPLOYER_ID, IDS_EMPLOYER_ID);
	SetResourceText(IDC_CFT_PIN, IDS_PIN);
	SetResourceText(IDC_STATIC_TRANS_NAME, IDS_TRANSMITTER_NAME);
	SetResourceText(IDC_STATIC_STREET_ADDR, IDS_STREET_ADDR);
	SetResourceText(IDC_STATIC_CITY, IDS_CITY);
	SetResourceText(IDC_STATIC_STATE, IDS_STATE);
	SetResourceText(IDC_STATIC_COUNTRY, IDS_COUNTRY_CODE);
	SetResourceText(IDC_STATIC_ZIP_CODE, IDS_ZIP_CODE);
	SetResourceText(IDC_STATIC_ZIP_CODE_EXT, IDS_ZIP_CODE_EXT);
	SetResourceText(IDC_CVT_STATEPROVINCE, IDS_STATEPROVINCE);
	SetResourceText(IDC_CVT_FOREIGNPOSTAL, IDS_FOREIGNPOSTAL_CODE);
	SetResourceText(IDC_CVT_STATEPROVINCE1, IDS_STATEPROVINCE);
	SetResourceText(IDC_CVT_FOREIGNPOSTAL1, IDS_FOREIGNPOSTAL_CODE);
	SetResourceText(IDC_STATIC_ZIP_CODE_EXT1, IDS_ZIP_CODE_EXT);
	//SetResourceText(IDC_CVT_CONTACTINFO,IDS_CONTACTINFO);
	SetResourceText(IDC_GRP_CONTACTINFO, IDS_CONTACT_INFORMATION);
	SetResourceText(IDC_CVT_CONTACTNAME, IDS_NAME);
	SetResourceText(IDC_CVT_FAX, IDS_FAX);
	SetResourceText(IDC_CVT_EMAIL, IDS_EMAIL1);
	SetResourceText(IDC_STATIC_FILE_RETURN_NAME, IDS_FILE_RETURN_NAME);
	SetResourceText(IDC_STATIC_FILE_RETURN_ADDR, IDS_FILE_RETURN_ADDR);
	SetResourceText(IDC_STATIC_CITY2, IDS_CITY);
	SetResourceText(IDC_STATIC_STATE2, IDS_STATE);
	SetResourceText(IDC_STATIC_ZIP_CODE2, IDS_ZIP_CODE);
	SetResourceText(IDC_STATIC_COUNTRY_RETURN, IDS_COUNTRY_CODE);
	SetResourceText(IDC_STATIC_TCC, IDS_TCC);
	SetResourceText(IDC_STATIC_PHONE_NUM, IDS_PHONE_NUMBER);
	SetResourceText(IDC_CTF_PREFERRED_METHOD, IDS_CFT_PREFERED_METHOD);
	SetResourceText(IDC_CTF_RDO_EMAIL, IDS_CFT_EMAIL_INTERNET);
	SetResourceText(IDC_CTF_RDO_POSTAL, IDS_CFT_POSTALSERVICE);
	SetResourceText(IDC_CTF_LBL_EXTENSION, IDS_EXTENSION);

	// Test for adding new controls
	//SetResourceText(IDC_STATIC_FIRST_NAME_LABEL, IDS_EMPLOYER_ID);
	//SetResourceText(IDC_STATIC_FIRST_NAME_LABEL, IDS_LAST_NAME);
	SetResourceText(IDC_STATIC_FIRST_NAME_LABEL, 17849);
	//CGBLResources::GetResourceString(IDS_FIRST_NAME)

	// Remove upto this after test
	//JKC 8/28/03 DefectID: 1-7595 Set font to nine point italic!
	GetDlgItem(IDC_GRP_CONTACTINFO)->SetFont(&si->Drawing.Fonts.NineItalic);
	GetDlgItem(IDC_CTF_PREFERRED_METHOD)->SetFont(&si->Drawing.Fonts.NineItalic);
}

/****************************************************************
FillControlMap - Set the map in a struct for the form controls,
				****IMPORTANT(THE MAP NAME AND THE ARRAY VALUE MUST BE	// THE SAME) *****AES
				Call MySetElement method in the XML class to set the struct and map.
				/****************************************************************
				MySetElement - Set control map struct in the XML with controls
				CString szTable			Control id
				CString strKey			SQL field name to match with control
				CObject * pObjVal		pointer to object in map
				int pnVal				Use currently for grid read write properties
				int nType 				Type of control in map
				****************************************************************/
				//****************************************************************/
void CCFV_CTMagneticMediaConstants::FillControlMap()
{
	m_clsXML.MySetElement(_T("0"), _T("strEmployerID"), &m_txtEmployerID, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strPin"), &m_txtPin, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strTransmittername"), &m_txtTransmitterName, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strStreetAddress"), &m_txtTransmitterAddress, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strTransCity"), &m_txtTransmitterCity, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strTransState"), &m_txtTransmitterState, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strTransZip"), &m_txtTransmitterPostalCode, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strTransZipExt"), &m_txtTransmitterPostalExt, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strForeignStateProvince"), &m_txtTransStateProvince, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strForeignPostalCode"), &m_txtTransForeignPostal, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strForeignStateProvinceReturn"), &m_txtRetStateProvince, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strForeignPostalCodeReturn"), &m_txtRetForeignPostal, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strContactName"), &m_txtContactName, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strEmail"), &m_txtContantEMail, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strCountryCodeReturn"), &m_txtReturnCountry, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strCountryCode"), &m_txtTransmitterCountry, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strContactExt"), &m_txtPhoneExt, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strReturnName"), &m_txtReturnName, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strReturnAddress"), &m_txtReturnAddress, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strReturnCity"), &m_txtReturnCity, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strReturnState"), &m_txtReturnState, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strReturnZip"), &m_txtReturnPostalCode, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strReturnZipExt"), &m_txtReturnPostalExtension, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strControlCode"), &m_txtControlCode, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strReturnPhone"), &m_txtPhoneNumber, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("strContactFax"), &m_txtContactFax, NULL, STRINGTYPE);
	m_clsXML.MySetElement(_T("0"), _T("bytProblemNotify"), &m_GrouperPreferred, 0, GROUPTYPE);
}

/****************************************************************
	SetArrayValues - Set a CString Array with the ID of the control for SQL and
					value from the control
	int nType	-	Use for ID of which Array to set. Get next record only needs
					the Key value in the SQL table. Update and Insert need all control
					data. AutoFill will only need the key value of that autofill.
//****************************************************************/
void CCFV_CTMagneticMediaConstants::SetArrayValues(int nType)
{
	//Set the Array values for passing to the XML Data. This is the key index for the SPs AES
	strCommandValues.RemoveAll();	//clear array for now values AES
	UpdateData();

	strCommandValues.Add(_T("strEmployerID"));
	strCommandValues.Add((m_szEmployerID));
	strCommandValues.Add(_T("strPin"));
	strCommandValues.Add((m_szPin));
	strCommandValues.Add(_T("strTransmittername"));
	strCommandValues.Add((m_szTransmitterName));
	strCommandValues.Add(_T("strStreetAddress"));
	strCommandValues.Add((m_szTransmitterAddress));
	strCommandValues.Add(_T("strTransCity"));
	strCommandValues.Add((m_szTransmitterCity));
	strCommandValues.Add(_T("strTransState"));
	strCommandValues.Add((m_szTransmitterState));
	strCommandValues.Add(_T("strTransZip"));
	strCommandValues.Add((m_szTransmitterPostalCode));
	strCommandValues.Add(_T("strTransZipExt"));
	strCommandValues.Add((m_szTransmitterPostalExt));
	strCommandValues.Add(_T("strReturnName"));
	strCommandValues.Add((m_szReturnName));
	strCommandValues.Add(_T("strReturnAddress"));
	strCommandValues.Add((m_szReturnAddress));
	strCommandValues.Add(_T("strReturnCity"));
	strCommandValues.Add((m_szReturnCity));
	strCommandValues.Add(_T("strReturnState"));
	strCommandValues.Add((m_szReturnState));
	strCommandValues.Add(_T("strReturnZip"));
	strCommandValues.Add((m_szReturnPostalCode));
	strCommandValues.Add(_T("strReturnZipExt"));
	strCommandValues.Add((m_szReturnPostalExtension));
	strCommandValues.Add(_T("strControlCode"));
	strCommandValues.Add((m_szControlCode));
	strCommandValues.Add(_T("bytProblemNotify"));
	strCommandValues.Add((CGBLForm::ConvertIntToString(m_GrouperPreferred.GetIntegerValue())));
	strCommandValues.Add(_T("strCountryCodeReturn"));
	strCommandValues.Add((m_szReturnCountry));
	strCommandValues.Add(_T("strCountryCode"));
	strCommandValues.Add((m_szTransmitterCountry));
	strCommandValues.Add(_T("strForeignStateProvince"));
	strCommandValues.Add((m_szTransStateProvince));
	strCommandValues.Add(_T("strForeignPostalCode"));
	strCommandValues.Add((m_szTransForeignPostal));
	strCommandValues.Add(_T("strForeignStateProvinceReturn"));
	strCommandValues.Add((m_szRetStateProvince));
	strCommandValues.Add(_T("strForeignPostalCodeReturn"));
	strCommandValues.Add((m_szRetForeignPostal));
	strCommandValues.Add(_T("strContactName"));
	strCommandValues.Add((m_szContactName));
	strCommandValues.Add(_T("strReturnPhone"));
	strCommandValues.Add((m_szPhoneNumber));
	strCommandValues.Add(_T("strContactFax"));
	strCommandValues.Add(m_szContactFax);		//sb - 1-15437 - was strCommandValues.Add(FilterForNumerics(m_szContactFax) );
	strCommandValues.Add(_T("strEmail"));
	strCommandValues.Add((m_szContantEMail));
	strCommandValues.Add(_T("strContactExt"));
	strCommandValues.Add((m_szPhoneExt));
}

/****************************************************************
	SaveChanges - Override to save change when the dialog is closed.
//****************************************************************/
void CCFV_CTMagneticMediaConstants::SaveChanges()
{
	CallXMLChangeData();
}

/****************************************************************
	CallXMLChangeData - Used to call the change data function in the xml
					class.
//****************************************************************/
void CCFV_CTMagneticMediaConstants::CallXMLChangeData()
{
	if (ValidateRequired()) {
		SetArrayValues();
		// begbert 10-19-2009 1-33583 : Switched to use the CMS_ElectronicConstants company table
		if (m_clsXML.ChangeXMLData(RECORD_UPDATE, _T("CT"), _T("CMS_ElectronicConstants"), &strCommandValues)) {
			m_RecordSaved = true;
			m_bDirty = false;
		}
	}
}

BOOL CCFV_CTMagneticMediaConstants::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	CString szValue(_T(""));
	NMHDR * const pMessage = (NMHDR*)lParam;

	switch (pMessage->code)
	{
	case WM_BTN_OK:			//Message received from Ok Button
		//1-28465 BK 2/21/08 reset dirty flag
		m_bDirty = false;

		CallXMLChangeData();
		PostMessage(WM_CLOSE);
		break;
	case WM_BTN_CANCEL:
		PostMessage(WM_CLOSE);
		break;
	case WM_OK_DONE:
		if (m_RecordSaved) m_dlgCMSReportButtons.OnBnClickedGblBtnCancel();
		break;
	}

	return __super::OnNotify(wParam, lParam, pResult);
}

void CCFV_CTMagneticMediaConstants::PostNcDestroy()
{	//unlock the record on close
	if (m_bGoodLock) {
		// begbert 10-19-2009 1-33583 : Switched to use the CMS_ElectronicConstants company table
		m_clsXML.LockRecord(RECORD_UNLOCK, _T("CT"), _T("CMS_ElectronicConstants"), &strCommandValues);
	}
	CGBLForm::SendMessageToMainFrame(MAGMEDIACONSTANTS, 0);				//SEND message to the main that this form is closed AES

	__super::PostNcDestroy();
}

bool CCFV_CTMagneticMediaConstants::ValidateRequired(void)
{
	// 1-29161 BK 3/6/08 commented out this validation because this message has to do with payroll, which is not available yet
	//  UpdateData();
	//  // Put the form validation here
	//  if (m_szPin.IsEmpty() && m_bFirstNotification)
	//  {
		  //CGBLForm::CMSMessageBox(this,IDS_MSG_MAGNETIC_MEDIA_NO_PIN, IDS_CAPTION_MAGNETIC_MEDIA_NO_PIN, MB_OK | MB_ICONEXCLAMATION);
	//      m_bFirstNotification = false;
	//      return false;
	//  }
	return true;
}

void CCFV_CTMagneticMediaConstants::OnEnSetfocusEditEmployerId()
{
	//There was an error in the opening of the form so close it.
	if (m_bDontOpen)
	{
		GetParent()->PostMessage(WM_CLOSE, 0, 0);
	}
}

void CCFV_CTMagneticMediaConstants::OnBnClickedCtfRdoPostal()
{
	const int nPostal = m_rdoPostal.GetCheck();
	SetDirty(m_nPostal != nPostal);
	m_nPostal = nPostal;
}

void CCFV_CTMagneticMediaConstants::OnBnClickedCtfRdoEmail()
{
	const int nEmail = m_rdoEMail.GetCheck();
	SetDirty(m_nEmail != nEmail);
	m_nEmail = nEmail;
}

CString CCFV_CTMagneticMediaConstants::FilterForNumerics(const CString& szPhoneNo) const
{
	CString szReturnVal(_T(""));
	for (int idx = 0; idx < szPhoneNo.GetLength(); idx++)
	{
		if (CMS::IsNumeric(szPhoneNo[idx])) szReturnVal += szPhoneNo[idx];
	}

	return szReturnVal;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeCftTxtExtension()
{
	const CString szCurrValue(m_txtPhoneExt.GetText());
	SetDirty(m_szPhoneExt != szCurrValue);
	m_szPhoneExt = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditEmployerId()
{
	const CString szCurrValue(m_txtEmployerID.GetText());
	SetDirty(m_szEmployerID != szCurrValue);
	m_szEmployerID = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditTransmitterName()
{
	const CString szCurrValue(m_txtTransmitterName.GetText());
	SetDirty(m_szTransmitterName != szCurrValue);
	m_szTransmitterName = szCurrValue;
}

// Test for adding new controls
void CCFV_CTMagneticMediaConstants::OnEnChangeEditFirstName()
{
	static int s_nFirstNameChangeCount = 0;
	++s_nFirstNameChangeCount;

	const CString szCurrValue(m_txtFirstName.GetText());
	SetDirty(m_szFirstName != szCurrValue);
	m_szFirstName = szCurrValue;

	/*CGBLForm::CMSMessageBox(
		this,
		0,
		IDS_COUGAR_MOUNTAIN,
		MB_OK | MB_ICONINFORMATION,
		m_szFirstName,
		_T(""),
		0,
		false);*/
}
// Remove upto this after test

void CCFV_CTMagneticMediaConstants::OnEnChangeEditStreetAddr()
{
	const CString szCurrValue(m_txtTransmitterAddress.GetText());
	SetDirty(m_szTransmitterAddress != szCurrValue);
	m_szTransmitterAddress = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditCity()
{
	const CString szCurrValue(m_txtTransmitterCity.GetText());
	SetDirty(m_szTransmitterCity != szCurrValue);
	m_szTransmitterCity = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditState()
{
	const CString szCurrValue(m_txtTransmitterState.GetText());
	SetDirty(m_szTransmitterState != szCurrValue);
	m_szTransmitterState = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditCountry()
{
	const CString szCurrValue(m_txtTransmitterCountry.GetText());
	SetDirty(m_szTransmitterCountry != szCurrValue);
	m_szTransmitterCountry = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditZipCode()
{
	const CString szCurrValue(m_txtTransmitterPostalCode.GetText());
	SetDirty(m_szTransmitterPostalCode != szCurrValue);
	m_szTransmitterPostalCode = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditZipCodeExt()
{
	const CString szCurrValue(m_txtTransmitterPostalExt.GetText());
	SetDirty(m_szTransmitterPostalExt != szCurrValue);
	m_szTransmitterPostalExt = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditFileReturnName()
{
	const CString szCurrValue(m_txtReturnName.GetText());
	SetDirty(m_szReturnName != szCurrValue);
	m_szReturnName = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditFileReturnAddr()
{
	const CString szCurrValue(m_txtReturnAddress.GetText());
	SetDirty(m_szReturnAddress != szCurrValue);
	m_szReturnAddress = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditCity2()
{
	const CString szCurrValue(m_txtReturnCity.GetText());
	SetDirty(m_szReturnCity != szCurrValue);
	m_szReturnCity = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditState2()
{
	const CString szCurrValue(m_txtReturnState.GetText());
	SetDirty(m_szReturnState != szCurrValue);
	m_szReturnState = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditZipCode2()
{
	const CString szCurrValue(m_txtReturnPostalCode.GetText());
	SetDirty(m_szReturnPostalCode != szCurrValue);
	m_szReturnPostalCode = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditCountryReturn()
{
	const CString szCurrValue(m_txtReturnCountry.GetText());
	SetDirty(m_szReturnCountry != szCurrValue);
	m_szReturnCountry = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditTcc()
{
	const CString szCurrValue(m_txtControlCode.GetText());
	SetDirty(m_szControlCode != szCurrValue);
	m_szControlCode = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditPhoneNumber()
{
	CString szCurrValue(_T(""));
	m_txtPhoneNumber.GetWindowText(szCurrValue);
	SetDirty(m_szPhoneNumber != szCurrValue);
	m_szPhoneNumber = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditZipCodeExt2()
{
	const CString szCurrValue(m_txtReturnPostalExtension.GetText());
	SetDirty(m_szReturnPostalExtension != szCurrValue);
	m_szReturnPostalExtension = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeCftTxtPin()
{
	const CString szCurrValue(m_txtPin.GetText());
	SetDirty(m_szPin != szCurrValue);
	m_szPin = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditStateprovince()
{
	const CString szCurrValue(m_txtTransStateProvince.GetText());
	SetDirty(m_szTransStateProvince != szCurrValue);
	m_szTransStateProvince = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditForeignpostal()
{
	const CString szCurrValue(m_txtTransForeignPostal.GetText());
	SetDirty(m_szTransForeignPostal != szCurrValue);
	m_szTransForeignPostal = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditStateprovince1()
{
	const CString szCurrValue(m_txtRetStateProvince.GetText());
	SetDirty(m_szRetStateProvince != szCurrValue);
	m_szRetStateProvince = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditForeignpostal1()
{
	const CString szCurrValue(m_txtRetForeignPostal.GetText());
	SetDirty(m_szRetForeignPostal != szCurrValue);
	m_szRetForeignPostal = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditContactname()
{
	const CString szCurrValue(m_txtContactName.GetText());
	SetDirty(m_szContactName != szCurrValue);
	m_szContactName = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditFax()
{
	CString szCurrValue(_T(""));
	m_txtContactFax.GetWindowText(szCurrValue);
	SetDirty(m_szContactFax != szCurrValue);
	m_szContactFax = szCurrValue;
}

void CCFV_CTMagneticMediaConstants::OnEnChangeEditEmail1()
{
	const CString szCurrValue(m_txtContantEMail.GetText());
	SetDirty(m_szContantEMail != szCurrValue);
	m_szContantEMail = szCurrValue;
}
