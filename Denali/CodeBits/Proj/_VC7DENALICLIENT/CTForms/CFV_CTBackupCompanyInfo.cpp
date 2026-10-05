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
#include <stdlib.h>
#include <ShlObj.h>
#include ".\CFV_CTBackupRestore.h"
#include ".\CFV_CTBackupCompanyInfo.h"
#include "..\cmsdll\GBLNetDll.h"

 //helper function to return the day of week string
 //in a language aware way.
CString GetLocaleDayOfWeekString()
{
	// Print out the day of the week using localized day name
	UINT DayOfWeek[] = {
	LOCALE_SDAYNAME7,   // Sunday
	LOCALE_SDAYNAME1,
	LOCALE_SDAYNAME2,
	LOCALE_SDAYNAME3,
	LOCALE_SDAYNAME4,
	LOCALE_SDAYNAME5,
	LOCALE_SDAYNAME6   // Saturday
	};
	TCHAR szWeekday[256];
	CTime time(CTime::GetCurrentTime());   // Initialize CTime with current time
	::GetLocaleInfo(LOCALE_USER_DEFAULT,   // Get string for day of the week from system
		DayOfWeek[time.GetDayOfWeek() - 1],   // Get day of week from CTime
		szWeekday, sizeof(szWeekday));
	return szWeekday;
}

// CCFV_CTBackupCompanyInfo

IMPLEMENT_DYNCREATE(CCFV_CTBackupCompanyInfo, CCMSFormView)

CCFV_CTBackupCompanyInfo::CCFV_CTBackupCompanyInfo()
	: CCMSFormView(CCFV_CTBackupCompanyInfo::IDD)
{
	m_txtDestPath.SetHighlightOnSetFocus(true);
	m_bMultiDisk = false;
	m_szSubForm = _T("DA638ECED67D497f80A75D96C4273977");
}

CCFV_CTBackupCompanyInfo::~CCFV_CTBackupCompanyInfo()
{

}

void CCFV_CTBackupCompanyInfo::DoDataExchange(CDataExchange* pDX)
{
	CCMSFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CTF_ICO_TOP, m_mfcTopIcon);
	DDX_Control(pDX, IDC_CTF_SEP_TOP, m_clsTopseparator);
	DDX_Control(pDX, IDC_CTF_LBL_SEPBOTTOM, m_clsBottomseparator);
	DDX_Control(pDX, IDC_CTF_TXT_DESTPATH, m_txtDestPath);
}

BEGIN_MESSAGE_MAP(CCFV_CTBackupCompanyInfo, CCMSFormView)
	ON_COMMAND(IDC_CTF_BTN_DIR, SelectDestDir)
	ON_EN_KILLFOCUS(IDC_CTF_TXT_DESTPATH, OnEnKillfocusCtfTxtDestpath)
	ON_WM_DESTROY()
	ON_EN_UPDATE(IDC_CTF_TXT_DESTPATH, OnChangeDestPath)
END_MESSAGE_MAP()

// CCFV_CTBackupCompanyInfo message handlers
void CCFV_CTBackupCompanyInfo::OnInitialUpdate()
{
	CCMSFormView::OnInitialUpdate();

	//create action buttons form
	m_clsActionButtonsForm.BindControl(GetDlgItem(IDC_CTF_BTN_PLACEHOLDER), true);

	//Make the default file name to backup to.
	CString szCompany = si->CompanySettings.ID;
	CString szFileName = szCompany + _T("DATA.CAB");
	SetDlgItemText(IDC_CTF_TXT_FILE_NAME, szFileName);

	//Make the default dir to backup to.
	//default directory is in the format: APPDIR\COMPANYNAME\BACKUP\yyyymmddhhmm"(ref:Dave Parvin)
	CString szAppDir;
	szAppDir.Empty();

	TCHAR   path[MAX_PATH];
	if (si->RunningInVirtualUI)
	{
		stuErrorReturn clsErrorReturn;
		szAppDir = CGBLNetDll::DoSpecialCommand(_T("<ROOT><COMMAND>GetVirtualUITempPath</COMMAND></ROOT>"), false, &clsErrorReturn);
		if (szAppDir.Right(1) != _T("\\"))
			szAppDir += _T("\\");
		GetDlgItem(IDC_CTF_TXT_DESTPATH)->EnableWindow(FALSE);
		GetDlgItem(IDC_CTF_BTN_DIR)->EnableWindow(FALSE);
	}
	else
	{
		// AR 04/30/2024 PBI 62690 - If Multi Tenant, the destination path pointing to a folder in user's profile
		if (si->ApplicationSettings.TenantID == _T("NORMAL DENALI"))
			szAppDir = si->ApplicationPaths.szApplicationPath;
		else
		{
			if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, path)))
			{
				szAppDir = path;
				szAppDir += _T("\\DENALI BACKUPS\\");
			}
		}
		szAppDir += szCompany;
		szAppDir += _T("\\BACKUP\\");

		//get date
		CTime clsCurrentTime = CTime::GetCurrentTime();
		CString szBackupDirName = clsCurrentTime.Format(_T("%Y%m%d%H%M"));//original
		szAppDir += szBackupDirName + _T("\\");
	}

	SetDlgItemText(IDC_CTF_TXT_DESTPATH, szAppDir);//set path to backup to
	m_txtDestPath.SetSel(0, -1, FALSE);
	SetDlgItemText(IDC_CTF_TXT_SOURCE_SERVER, si->Servers.PrimaryServer.Name);//set current database server
	SetDlgItemText(IDC_CTF_TXT_SOURCE_COMPANY, szCompany);//set current database

	LoadResources();
	GetBackupFlags();

	ResizeParentToFit(FALSE);

	//Defect ID 1-13582, PGP(09/03/2004)
	PostMessage(WM_NEXTDLGCTL, (WPARAM)m_txtDestPath.GetSafeHwnd(), TRUE);
}

void CCFV_CTBackupCompanyInfo::LoadResources()
{
	//Set title of form
	CGBLForm::SetFormTitle(this, CGBLResources::GetResourceString(IDS_CAPTION_BACKUP_COMPANY_INFO));
	//picture for the icon
	m_mfcTopIcon.SetIcon(si->Drawing.Icons.BackupCompany());
	//header title.
	SetResourceText(IDC_CTF_TOPHEADER, IDS_HEADER_BACKUP_COMPANY_INFO);
	//labels for edits
	// 1-7526, DTG, 8/28/2003, changed string table for path and resource ID for file.
	SetResourceText(IDC_LBL_DEST_PATH, IDS_LBL_DEST_PATH);
	SetResourceText(IDC_LBL_FILE_NAME, IDS_FILE_NAME);
	SetDlgItemText(IDC_LBL_SOURCE_SERVER, CGBLResources::GetResourceString(IDS_LBL_SOURCE_SERVER));
	SetDlgItemText(IDC_LBL_SOURCE_COMPANY, CGBLResources::GetResourceString(IDS_LBL_SOURCE_COMPANY));
	SetResourceText(IDC_CTF_BTN_DIR, IDS_ECLIPSE);
}

//Handle Notifications from dlg action buttons
BOOL CCFV_CTBackupCompanyInfo::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	try
	{
		NMHDR* pMessage = (NMHDR*)lParam;

		switch (pMessage->code)
		{
		case WM_BTN_OK:
			StartBackup();
			break;
		case WM_BTN_CANCEL:
			GetParent()->PostMessage(WM_CLOSE);
			break;
		}
	}
	catch (_com_error& e)
	{
		CGBLForm::HandleError(e);
	}
	return __super::OnNotify(wParam, lParam, pResult);
}

void CCFV_CTBackupCompanyInfo::OnEnKillfocusCtfTxtDestpath()
{
	m_txtDestPath.SetSel(m_txtDestPath.LineLength(), m_txtDestPath.LineLength());
}

//send xml to get backup info
void CCFV_CTBackupCompanyInfo::StartBackup()
{
	//get dir
	CString szDir;
	GetDlgItemText(IDC_CTF_TXT_DESTPATH, szDir);
	//get filename
	CString szFileName;
	GetDlgItemText(IDC_CTF_TXT_FILE_NAME, szFileName);
	//pre-condition: search for empty string
	szDir.Trim();
	if (szDir.IsEmpty())
	{
		CGBLForm::CMSMessageBox(this, IDS_SPECIFY_DIRECTORY, 0, MB_OK | MB_ICONEXCLAMATION);
		GetDlgItem(IDC_CTF_TXT_DESTPATH)->SetFocus();
		return;
	}
	//make full path
	CString szFullPath = MakeFullPathName(szDir, szFileName);

	//Check if this path exists and file can be created.
	if (VerifyPathAndFile(szDir, szFullPath))
	{
		SaveBackupFlags();
		//Ready to start backup
		//setup backup/restore common dialog for backup
		CCFV_CTBackupRestore oDlg;
		CString szServerType, szServer, szDBName;
		CString       m_szcabarry;	//AR - 07/21/2023 - Bug 25977 - Added
		//defect 1-15592, KPM, 2/15/2005.  Removed check boxes so always pass in true for data and images.
		//AR - 07/21/2023 - Bug 25977
		oDlg.SetParams(szFullPath, szServerType, szServer, szDBName, CCFV_CTBackupRestore::BACKUP, m_szcabarry, _T(""),
			true, true, m_bMultiDisk);

		/*oDlg.SetParams(szFullPath, szServerType, szServer, szDBName, CCFV_CTBackupRestore::BACKUP, _T(""),
			m_bolBackupData, si->ModulesInstalled.Inventory ? m_bolBackupImages:FALSE);*/

		GetParentFrame()->ShowWindow(SW_HIDE);	// Hide this window as backup status shows up now
		oDlg.DoModal();							// backup operation is started here.

		GetParentFrame()->PostMessage(WM_CLOSE, 0, 0);
	}
	else
	{
		GetDlgItem(IDC_CTF_TXT_DESTPATH)->SetFocus();
		if (GetDlgItem(IDC_CTF_TXT_DESTPATH)->IsKindOf(RUNTIME_CLASS(CEdit))) ((CEdit*)GetDlgItem(IDC_CTF_TXT_DESTPATH))->SetSel(0, -1);
	}
}

//send xml to get backup info
// DWP - 4866 - 7/29/13 - If the backup does not start then we want to return FALSE here
BOOL CCFV_CTBackupCompanyInfo::StartSilentBackup(CString szDir, CString szFileName)
{
	//pre-condition: search for empty string
	szDir.Trim();
	if (szDir.IsEmpty())
	{
		// TODO: Add Event Log information here
		return FALSE;
	}
	//make full path
	CString szFullPath = MakeFullPathName(szDir, szFileName);

	//Check if this path exists and file can be created.
	if (VerifyPathAndFile(szDir, szFullPath))
	{
		SaveBackupFlags();
		//Ready to start backup
		//setup backup/restore common dialog for backup
		CCFV_CTBackupRestore oDlg;
		CString szServerType, szServer, szDBName;
		CString m_szcabarry;	//AR - 07/21/2023 - Bug 25977 - Added
		//defect 1-15592, KPM, 2/15/2005.  Removed check boxes so always pass in true for data and images.
		oDlg.SetMessageMode(false);
		oDlg.SetParams(
			szFullPath,
			szServerType,
			szServer,
			szDBName,
			CCFV_CTBackupRestore::BACKUP,
			m_szcabarry,	//AR - 07/21/2023 - Bug 25977
			_T(""),
			true,
			true,
			m_bMultiDisk);

		oDlg.InitProcessThread();							// backup operation is started here.
	}
	else
	{
		// TODO: Add Event Logging here if path creation fails
		return FALSE;
	}
	return TRUE;
}

//Brings up a folder selection dialog using SHBrowseForFolder.
void CCFV_CTBackupCompanyInfo::SelectDestDir()
{
	//RCG - 4/3/2007 - 1-26183/1-26213 - Changed Browse for Folder description
	//CString szTitle = CGBLResources::GetResourceString(IDS_SELECT_FOLDER_FOR_BACKUP) ;
	CString szTitle = CGBLResources::GetResourceString(IDS_BROWSE_FOR_FOLDER_MESSAGE);

	CString szDir;
	GetDlgItemText(IDC_CTF_TXT_DESTPATH, szDir);
	szDir = CGBLForm::BrowseForDir(szTitle.GetBuffer(), szDir.Trim(), GetSafeHwnd());
	if (!szDir.IsEmpty())
	{
		SetDlgItemText(IDC_CTF_TXT_DESTPATH, szDir);
		m_txtDestPath.SetSel(m_txtDestPath.LineLength(), m_txtDestPath.LineLength());
	}
}

CString CCFV_CTBackupCompanyInfo::MakeFullPathName(const CString& szDir, const CString& szFileName)
{
	CString szFullPath;
	//Make fully qualified path from the selected directory and filename,

	//Combine path with file name
	//pre-condition: add "\\" if not present
	if (szDir.ReverseFind(_T('\\')) != szDir.GetLength() - 1)
		szFullPath = szDir + _T("\\") + szFileName;
	else
		szFullPath = szDir + szFileName;

	return szFullPath;
}

//This function attempts to create the destination directory first
//to make sure that the directory can be created and issues
//appropriate error messages if any. If the directory exists and
//file can be created, delete the created file and return.
//There can be removable devices, cd r/w etc used for backup.
//Arguments: directory name in the form "c:\\backup\\yyyymmddHHMM"
//Arguments: File name in the form "file.dat"
//Return Values: true if can continue, false if not.
//Added: Checking for dest dir in removable devices - Floppy Drives.
bool CCFV_CTBackupCompanyInfo::VerifyPathAndFile(const CString& szDir, const CString& szFileName)
{
	//Split destination to drive and path.
	_TCHAR szDrive[_MAX_DRIVE];
	_TCHAR szDirectory[_MAX_DIR];
	_TCHAR szFName[_MAX_FNAME];
	_TCHAR szExt[_MAX_EXT];

	_tsplitpath_s(szDir, szDrive, _MAX_DRIVE, szDirectory, _MAX_DIR, szFName, _MAX_FNAME, szExt, _MAX_EXT); // begbert 02-16-2006 VS8: added _s

	//Pre-Condition: If we are dealing with removable devices,
	//Make sure they are ready and warn about any existing data being erased etc.
	m_bMultiDisk = (CGBLForm::GetDriveType(szDrive) == CGBLForm::CMSDRIVE_FLOPPY);
	if (m_bMultiDisk && !PrepareDevice(szDrive))
		return false;

	int nReturn = ERROR_SUCCESS;
	CString szTempRoot = szDir;
	szTempRoot.TrimRight('\\');
	szTempRoot.TrimRight('//');
	szTempRoot.Trim();
	//If we have a valid directory name, create the directory.
	if (szTempRoot != szDrive)
	{
		//Defect ID 1-14566, PGP(11/12/2004) - Confirm creation for
		if (m_bMultiDisk && CGBLForm::CMSMessageBox(this, IDS_CT_DIR_DOES_NOT_EXIST, IDS_CMS_BACKUP_COMPANY_INFO,
			MB_YESNO, szDir, _T(""), 0, false) == IDNO)
			return false;
		nReturn = SHCreateDirectoryEx(NULL, szDir, NULL);
	}

	// begbert 11-06-2008 1-31241 : On mapped network drives, sometimes these file creations work; sometimes they don't.
	if (nReturn == ERROR_SUCCESS || nReturn == ERROR_FILE_EXISTS || nReturn == ERROR_ALREADY_EXISTS)
	{
		//Directory created or already exists, check if a file
		//with the selected backup file name already exists.
		//Defect ID 1-14890, PGP(12/07/2004) - Checking for existing files.
		// begbert 10-30-2008 1-31241 : Specified rights, security, and attribute parameters to get it to work with network drives.
		HANDLE hFile = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE | STANDARD_RIGHTS_ALL,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile != INVALID_HANDLE_VALUE)	// file opened, close handle and return success.
		{
			CloseHandle(hFile);
			// DWP - 4866 - 7/29/13 - If we are doing a silent backup then assume we are overwriting.
			if (!si->CommandLineSettings.CT_Quiet &&
				CGBLForm::ShowYNMsg(this, IDS_CT_BACKUP_FILE_EXISTS_MSG, IDS_CT_BACKUP_FILE_EXISTS_TITLE) == IDNO)
				return false;

			//delete test file before start.
			DeleteFile(szFileName);

			return true;
		}
		else
		{

			// begbert 11-06-2008 1-31241 : On mapped network drives, sometimes these file creations work;
			//   sometimes they don't.  As yet I have no explanation why.  So, as a stopgap, we try to open
			//   the file several times before accepting that it really isn't going to open.
			int nTries = 0;
			while (nTries < 5)
			{
				//The selected filename does not exist in destination directory, as a pre-condition to
				//starting backup, try creating the dest file here.
				// begbert 10-30-2008 1-31241 : Specified rights, security, and attribute parameters to get it to work with network drives.
				HANDLE hFile = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE | STANDARD_RIGHTS_ALL,
					FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_NORMAL, NULL);

				if (hFile != INVALID_HANDLE_VALUE)
				{
					CloseHandle(hFile);
					//delete test file before start.
					DeleteFile(szFileName);

					//Good to go!!
					return true;
				}

				nTries++;
			}

			//If we fail to create a file, the code below would show an OS error
			//message to report what specific error happened.
		}
	}

	//error, Directory could not be created,
	//or file could not be read.
	//PGP(09/21/2004) - I had a debug error here, so removed code and used szMsg.GetBuffer()
	//Will revisit when GBLForm::ShowLastOSError() is changed to take const CString& as param.
	CString szMsg(CGBLResources::GetResourceString(IDS_LBL_ERROR));
	CGBLForm::ShowLastOSError(szMsg.GetBuffer());		// last error will be reported in OS default language.

	return false;
}

//sb - 18-may-2004 - 1-11244
//---------------------------------------------------------------------------
BOOL CCFV_CTBackupCompanyInfo::GetBackupFlags(void)
{
	CCMSDataAdapter	objDataSource(CMSStrings::XMLTags::SPECIALPROCESS, CMS::GetSI()->ApplicationSettings.ModuleID, _T("GETBACKUPFLAGS"));

	CCMSDataAdapterOptions	objDataSourceOptions;
	objDataSourceOptions.m_szArchiveCon.Empty();
	objDataSourceOptions.m_szPrimaryCon.Empty();
	objDataSourceOptions.m_szCompanyCon = CMSStrings::XMLTags::Connection;

	CDataSet clsDS;
	CXMLDocument docXMLData(CMSStrings::XMLTags::Root);
	const BOOL bolStatus = (objDataSource.GetData(docXMLData, clsDS, objDataSourceOptions) == rcSuccess);
	clsDS.Dispose();

	return bolStatus;
}

//sb - 18-may-2004 - 1-11244
//---------------------------------------------------------------------------
BOOL CCFV_CTBackupCompanyInfo::SaveBackupFlags(void)
{
	if (!si->CommandLineSettings.CT_Quiet)
		UpdateData(TRUE);

	CXML clsXML;
	CXMLParams clsRoot;						// create root data parameter
	CXMLParams clsCommand;				// create xml Command parameter

	// xml Command
	clsCommand.MakeXMLCommand(&clsCommand, _T("SPECIALPROCESS"), si->ApplicationSettings.ModuleStringID, _T("SAVEBACKUPFLAGS"));
	clsXML.m_szXMLCommand = clsCommand.GetXML();
	clsRoot.AppendXMLConnection(&clsRoot, CXMLParams::CN_COMPANY);

	CXMLParams clsCompany(true);		// create xml Company parameter
	clsXML.MakeCOMPANYINFO();
	clsCompany.SetXML(clsXML.m_szXMLFormat);
	clsRoot.AppendXMLParam(&clsCompany);
	clsXML.m_szXMLData = clsRoot.GetXML();

	const CString szResult(clsXML.SendXML().copy());

	return  (szResult.Find(_T("<ERROR>")) < 0);
}

void CCFV_CTBackupCompanyInfo::OnDestroy()
{
	SaveBackupFlags();
	__super::OnDestroy();
}

void CCFV_CTBackupCompanyInfo::OnChangeDestPath()
{
	if (m_clsActionButtonsForm.m_btnOk.GetSafeHwnd())
		m_clsActionButtonsForm.m_btnOk.EnableWindow(m_txtDestPath.GetWindowTextLength() > 0);
}

//For floppy drives, clear out existing data before proceeding.
//Defect ID 1-14542, PGP(11/12/2004)
bool CCFV_CTBackupCompanyInfo::PrepareDevice(const CString& szDrive)
{
	if (CGBLForm::GetDriveType(szDrive) == CGBLForm::CMSDRIVE_FLOPPY)
	{
		if (CGBLForm::CMSMessageBox(this, IDS_CT_MULTI_DISK_BKP_WARNING, IDS_CT_MULTI_DISK_BKP_TITLE, MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
			return false;

		//Bring up windows cancelable Delete file operation with a status.
		SHFILEOPSTRUCT shFileST;
		memset(&shFileST, 0, sizeof(shFileST));

		_TCHAR szDeleteFrom[_MAX_PATH];
		memset(szDeleteFrom, 0, sizeof(szDeleteFrom));
		_tcscat_s(szDeleteFrom, _MAX_PATH, szDrive); // begbert 02-16-2006 VS8: added _s
		_tcscat_s(szDeleteFrom, _MAX_PATH, _T("\\*.*")); // begbert 02-16-2006 VS8: added _s

		shFileST.hwnd = GetParent()->GetSafeHwnd();
		shFileST.pFrom = szDeleteFrom;
		shFileST.wFunc = FO_DELETE;
		shFileST.fFlags = FOF_NOCONFIRMATION | FOF_SIMPLEPROGRESS;
		//If user decided to cancel during delete, return false.
		if (SHFileOperation(&shFileST) != 0 || shFileST.fAnyOperationsAborted)
			return false;
	}
	return true;
}
