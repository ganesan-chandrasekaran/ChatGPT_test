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
#pragma once

#include <atlimage.h>
#include <set>

#include "..\DundasInclude\OXEdit.h"
#include "..\DundasInclude\OXMaskedEdit.h"
#include "..\CMSInclude\CMSOXEdit.h"

#include "..\CMSDll\StdIncludes.h"

#include "..\cmsdll\CTSystemInformation.h"
#include "..\cmsdll\GLSystemInformation.h"
#include "..\cmsdll\INSystemInformation.h"
#include "..\cmsdll\ARSystemInformation.h"
#include "..\cmsdll\APSystemInformation.h"
#include "..\cmsdll\SASystemInformation.h"
#include "..\cmsdll\BRSystemInformation.h"
#include "..\cmsdll\JCSystemInformation.h"
#include "..\cmsdll\OESystemInformation.h"
#include "..\cmsdll\PRSystemInformation.h"
#include "..\cmsdll\POSystemInformation.h"
#include "..\cmsdll\PSSystemInformation.h"
#include "..\cmsdll\DXSystemInformation.h"

#include "..\cmsdll\xinfotip.h"

 //Include the constants for the globals
#include "..\cmsdll\GBLErrorConstants.h"
#include "..\cmsdll\GBLUserInfo.h"
#include "..\cmsdll\AppSettings.h"
#include "..\cmsdll\ClientLockRefresh.h"
#include "..\cmsdll\CMSDynThemeManager.h"
#include "..\cmsdll\CMSDBMap.h"
#include "..\cmsdll\CMSDouble.h"
#include "..\cmsdll\CMSListCtrl.h"
//#include "..\cmsdll\ILookup.h"
#include "..\cmsdll\CMSMessage.h"
#include "..\cmsdll\CMSMessageTypes.h"
#include "..\cmsdll\CMSPublisherDelegate.h"
#include "..\cmsdll\CMSThreadController.h"
#include "..\cmsdll\CMSUtility.h"
#include "..\cmsdll\Dataset.h"
#include "..\cmsdll\XMLDoc.h"
#include "..\cmsdll\DoubleString.h"
#include "..\cmsdll\GBLBatchInfo.h"
#include "..\cmsdll\GBLCommandLineInfo.h"
#include "..\cmsdll\GBLConstants.h"
#include "..\cmsdll\GBLDebug.h"
#include "..\cmsdll\GBLDrawing.h"
#include "..\cmsdll\GBLEMail.h"
#include "..\cmsdll\GBLError.h"
#include "..\cmsdll\GBLFileSystem.h"
#include "..\cmsdll\GBLForm.h"
#include "..\cmsdll\ICMSParentBase.h"
#include "..\cmsdll\CMSDataMapTypes.h"
#include "..\cmsdll\ICMSControlEx.h"
#include "..\cmsdll\ICMSMultiRecord.h"
#include "..\cmsdll\GBLHelp.h"
#include "..\cmsdll\GBLLocalization.h"
#include "..\cmsdll\GBLMath.h"
#include "..\cmsdll\GBLModuleIntegrationHelper.h"
#include "..\cmsdll\GBLPrint.h"
#include "..\cmsdll\GBLRegistry.h"
#include "..\cmsdll\GBLResources.h"
#include "..\cmsdll\GBLSQLInfo.h"
#include "..\cmsdll\GBLSystemState.h"
#include "..\cmsdll\GBLUserRights.h"
#include "..\cmsdll\GBLUserRightsInformation.h"
#include "..\cmsdll\GBLWeb.h"
#include "..\cmsdll\IPublishSubscribe.h"
//KPM, 8/20/2003, This will track instances of forms being opened, needed
//more advanced mechanism for BR
#include "..\cmsdll\OpenFormTracker.h"

//Include the language ID's
#include "..\common\langids.h"
#include "..\cmsdll\GBLLookupConstants.h"
#include "..\cmsdll\GBLTranslate.h"

#include "..\cmsdll\CMSWaitCursor.h"
#include "..\CMSInclude\CMSCommonStrings.h"
#include "..\cmsdll\XML.h"
#include "..\cmsdll\cmsdatacontainer.h"
// #include "..\cmsdll\serverside.h"
// #include "..\cmsdll\cmsdataadapter.h"
// #include "..\cmsdll\cmsdatabridge.h"
// #include "..\cmsdll\cmsdatabridgetrigger.h"
#include "..\cmsdll\ICMSWinApp.h"
#include "..\cmsdll\CMSDataLock.h"
#include "..\cmsdll\CMSCurrency.h"
#include "..\cmsdll\CMSWinThread.h"
#include "..\cmsdll\gbleventlog.h"
#include "..\cmsdll\ICMSFrameWnd.h"
#include "..\cmsdll\Thinfinity.VirtualUI.h"
#include "..\CMSDotNet\ProductAuditAPI.h"

class CServerSideInput;
class CCMSDataAdapterOptions;
class CCMSDataAdapter;
class CCMSMessageDataAdapter;
class CCMSDataBridgeOptions;
class CCMSDataBridge;
class CCMSMessageDataBridge;
class CCMSMessageDataBridgeSave;
class CCMSDataBridgeTrigger;
class CCMSMessageBridgeTrigger;
class CCMSMessageBridgeXML;
class CCMSMessageTriggerCompl;

//class CDllFormManager ;//PGP(10/15/2003) - Moved to CMSWinApp in gblforms. PGP(12/08/2003)
class AFX_EXT_CLASS CGBLSystemInformation
{
	// Construction
public:
	CGBLSystemInformation();						// Standard Constructor
	virtual		~CGBLSystemInformation();			// Standard Destructor
	bool		ParseCommandLineParameters();		// This will take the command line settings and put them into the
	// structure to hold them.  It also sets flags as to what is specified
	bool		LoadFonts();						// This will preload a few fonts we use everywhere
	bool		LoadModuleValuesFromServer();		// This will load each modules specific settings
	//CString	GetServerDataVersion();				// This will retrieve the data version from the ATL Wrapper
	bool		LoadSystemGlobals(void);			// This initiates the routines which load system global objects
	bool		SetupToolTip(CWnd* pParent);		// This will set up the tool tip control
	bool		LoadPeriodDates(void);				// This will load the dates for the periods
	bool		LoadSegmentSetup(void);				// This will load the segment information for the segmented accounts
	CString		GetSegmentInfoString(void) const;			// This will build the segment info string from the segment information
	void		CheckForCommandLineOverrides(void);	// This will look at the command line settings and determine what globals to override
	bool		CanLoadLibrary(_TCHAR* szLibraryName);//This will determine if a library can be loaded or not
	bool		CanLoadFunction(_TCHAR* szLibraryName, _TCHAR* szFunctionName);	//This will load a function and call it using a function pointer
	bool		SetupHelpSystem();					// Used to setup the help system files
	UINT		GetCompaniesList(void);				// Fill the list of valid companies
	bool		LoadAPSystemSettings(bool keepSelectedBatch = false);
	bool		LoadARSystemSettings();
	bool		LoadBRSystemSettings();
	bool		LoadCTSystemSettings();
	bool		LoadGLSystemSettings(bool bSegmentInfoOnly = false);
	bool		LoadINSystemSettings();
	bool		LoadJCSystemSettings();
	bool		LoadPOSystemSettings(bool bFromModPrefs = false);
	bool		LoadPRSystemSettings();
	bool		LoadSASystemSettings();

	//Notification from cmsdll dllmain when a thread exits. PGP(02/13/2004)
	//void		NotifyThreadExit(DWORD dwThreadID) ;
private:
	void		InitializeGlobalVariables();
	bool		LoadPens();
	bool		LoadBrushes();
	bool		LoadCursors();
	bool		LoadBitmaps();
	bool		LoadIcons();
	bool		LoadColors();

	bool		InitializeLanguageDLL();

	//KPM, 4/4/2003, Added for tip of the day
	bool		LoadTipOfTheDay();

	bool		vuiLicenced();
public:
	//delete all objects used for early binding, uninit com libs
	//void	DeleteEarlyBindingObjects() ;
	bool		IsInstalled(UINT nModuleID) const;

	bool						SkipLogin;				// skip login when creating New, Modifying or Removing Company

	enum enuModuleSetting
	{
		InCompany,
		Licensed,
		TimeLocked,
		Expired,
		IsUsable
	};

	struct stuLicensed
	{
		stuLicensed(void)
			: bInCompany(false), bLicensed(false), bTimeLocked(false), bExpired(false), bIsUsable(false)
		{
		};

		stuLicensed(const stuLicensed& rhs)
			: bInCompany(rhs.bInCompany), bLicensed(rhs.bLicensed), bTimeLocked(rhs.bTimeLocked), bExpired(rhs.bExpired), bIsUsable(rhs.bIsUsable)
		{
		};

		const stuLicensed& operator= (const stuLicensed& rhs)
		{
			if (this != &rhs)
				bInCompany = rhs.bInCompany; bLicensed = rhs.bLicensed; bTimeLocked = rhs.bTimeLocked; bExpired = rhs.bExpired; bIsUsable = rhs.bIsUsable;
			return *this;
		};

		bool get_InCompany(void)  const { return bInCompany; }
		bool get_Licensed(void)   const { return bLicensed; }
		bool get_TimeLocked(void) const { return bTimeLocked; }
		bool get_Expired(void)    const { return bExpired; }
		bool get_IsUsable(void)   const { return bIsUsable; }

		void put_InCompany(bool b) { bInCompany = b; }
		void put_Licensed(bool b) { bLicensed = b; }
		void put_TimeLocked(bool b) { bTimeLocked = b; }
		void put_Expired(bool b) { bExpired = b; }
		void put_IsUsable(bool b) { bIsUsable = b; }

		__declspec(property(get = get_InCompany, put = put_InCompany))		bool InCompany;
		__declspec(property(get = get_Licensed, put = put_Licensed))		bool Licensed;
		__declspec(property(get = get_TimeLocked, put = put_TimeLocked))	bool TimeLocked;
		__declspec(property(get = get_Expired, put = put_Expired))			bool Expired;
		__declspec(property(get = get_IsUsable, put = put_IsUsable))		bool IsUsable;

		bool	bInCompany;
		bool	bLicensed;
		bool	bTimeLocked;
		bool	bExpired;
		bool	bIsUsable;
	};

private:

	//This stores information pertinent to the current computer system the application
	//is running on.
	struct stuSystemData
	{
		stuSystemData() : ComputerName(_T("")), ServerName(_T("")) {};
		//UINT					OperatingSystem;	// The operation system  Valid Values Are:
		//											//
		//bool					NTKernelBased;		// Whether or not this system is based on the NT kernel as
		//											// opposed to the 9x kernel
		CString					ComputerName;		// What this computer is named
		CString					ServerName;			// Server side's computer name.
	};

	//This structure stores all the settings that can be passed in on the command line
	struct stuCommandLineSettings
	{
		CString					PL_PrimaryServer;
		CString					PL_PrimaryDatabase;
		CString					PL_PrimaryDBType;
		CString					PL_PrimaryUserName;
		CString					PL_PrimaryPassword;
		CString					PL_PrimaryServerTimeout;

		CString					AL_ArchiveServer;
		CString					AL_ArchiveDatabase;
		CString					AL_PrimaryDBType;
		CString					AL_ArchiveUserName;
		CString					AL_ArchivePassword;
		CString					AL_ArchiveServerTimeout;

		CString					UL_CompanyUser;								// -CU
		CString					UL_CompanyPassword;							// -CP
		CString					DC_DefaultCompany;							// -DC
		bool					LO_BackgroundLogin;

		bool					LO_ForcePrimaryServerLogin;					// -FL
		bool					LO_ForceUserLogin;							// -FU
		bool					LO_ForceCompanySelection;					// -FC
		bool					RO_WritePrimaryDatabaseSettingToRegistry;	// -RS
		bool					RO_WriteUserSettingsToRegistry;				// -RU
		bool					RO_WriteDefaultCompanyToRegistry;			// -RC
		UINT					PO_Developer;								// -DE/-DV/-DZ
		bool					PO_XMLViewer;								// -XM
		bool					PO_XMLViewerOnError;						// -XE
		bool					PO_ShowStatusXML;							// -XN
		bool					PO_AssertOnError;							// -XA
		bool					PO_OrderEntry;								// -SO
		bool					PO_PointOfSale;								// -SP
		bool					PO_OpenSalesEntryFromSalesMaint;			// -OP
		bool					PO_MemoryCheckpoints;						// -MC PTR.07.27.2005
		bool					PO_POPurchasing;
		bool					PO_POPurchasingEdit;
		bool					PO_POReceiving;
		CString					PO_TransactionNumber;
		bool					PO_RequisitionMode;


		//KPM.2005.03.24 Add for batch to be passed.
		CString					BA_BatchCode;								// -ba
		CString					BA_Session;									// -sx
		//KPM.2005.04.11 Added for easier testing
		CString					BA_MachineName;								// -ma
		// DWP 2005.07.12 added code for doing silent automatic backups
		bool					CT_Backup;									// -BK
		bool					CT_Quiet;									// -QT
		CString					CT_BackUpPath;
		// RCG - 04/27/2011 - 9000006 - Automated Posting
		bool					Automated_Posting;							// -pt
		// DWP - 2008.05.06 Added for SQL 2 SQL all companies
		bool					CT_Sql2SqlAll;								// -SQL2SQLALL		Run for all companies listed in the Primary Database
		bool					CT_ATR;										// -ATR				Automatically run the SQL-2-SQL
		bool					CT_Force;									// -FORCE			Force a SQL-2-SQL on all modules (Deletes all the rows in the CMS_TablesXML table).

		//Peter Ringering - 10/06/2004 - Temp flag to use the new printing process.
		bool					PO_PrintDialogEx;							// -XX
		//KPM.2005.04.18 Made for logging, helps with some errors
		bool					DB_Logging;									// -lg
		CString					DB_LogName;									// <install dir>/Logs/<logname>.log
		bool					DB_BypassReportThread;						// -rt
		// 1-120031 BK 7/5/05 need to be able to tell if a module is opened from another module
		bool					PO_OpenedFromAnotherModule;
		//PGP(01/27/2006) - Diagnostic flag for gift transactions
		bool					GIVEX_ShowInfo;
		// begbert 08-30-2007 : added to enable the Native Currency combo for new companies (which is now otherwise disabled)
		bool					CT_EnableNativeCurrencyCombo;				// -NC
		//Defect ID 1-28295, PGP(03-02-2008) hide status bar on entry windows (PO and sales)
		bool					Hide_StatusBar;
		// begbert 06-19-2015 PBI 28383
		bool					SA_JobCostMode;								// -JC
	};

	//These are used for temporary global storage.  Do not use these unless you
	//have no other choice.  It's dangerous to store information globally like this
	//because other items may be accessing and changing the variables value.  Only
	//use if you are sure the value will not be changed from the time its set by you
	//until the time it is used by you.
#pragma warning( push )
#pragma warning( disable : 5220 )
	struct stuTemporaryStorage
	{
		volatile bool			tsbool;
		volatile bool			tsbool2;
		volatile bool			tsbool3;	//use the date control in the print forms //hae 12/4/2003
		volatile int			tsint;
		volatile unsigned int	tsunsignedint;
		volatile __int8			ts__int8;
		volatile __int16		ts__int16;
		volatile __int32		ts__int32;
		volatile __int64		ts__int64;
		volatile long			tslong;
		volatile byte			tsbyte;
		volatile char			tschar;
		volatile unsigned char	tsunsignedchar;
		volatile short			tsshort;
		volatile unsigned short	tsunsignedshort;
		volatile unsigned long	tsunsignedlong;
		volatile enum			tsenum;
		volatile float			tsfloat;
		volatile double			tsdouble;
		volatile wchar_t		tswchar_t;
		volatile BSTR			tsBSTR;
		_bstr_t					ts_bstr_t;
		volatile BOOL			tsBOOL;
		volatile DWORD			tsDWORD;
		volatile HDC			tsHDC;
		volatile HPEN			tsHPEN;
		volatile HBITMAP		tsHBITMAP;
		volatile LPARAM			tsLPARAM;
		volatile WPARAM			tsWPARAM;
		volatile UINT			tsUINT;
		volatile UINT			tsUINT2;
		CFont					tsCFont;
		CBrush					tsCBrush;
		CString					tsCString;
		CRect					tsCRect;
		CXMLParams				tsXMLParams;

		bool					bCircularError;  //PTR.07.12.2005 - Make sure creating errors in CGBLError doesn't create more errors in CGBLError.
		bool					bCircularEvent;

		//RS - 07/02/2024 - PBI 60840
		double					d_CalcCreditCardFee;
		double					d_InvoiceCreditCardFee;
	};
#pragma warning (pop)

	//This structure is actually created by the Fiscal Calendar structure
	//It is an used as an array to store all the period information
	struct stuFiscalPeriodInformation
	{
		COleDateTime			StartDate;  //The period start date
		COleDateTime			EndDate;	//The period end date
	};

	struct stuFiscalCalendar
	{
		stuFiscalPeriodInformation FiscalCalendar[66];  //An array of all the periods (0-65) start and end dates
		COleDateTime			StartDate;				//The start date of the fiscal calendar
		COleDateTime			EndDate;				//The end date of the fiscal calendar
		COleDateTime			BlockDate;				//The first day after taking into account the block period
		byte					BlockPeriod;			//The period in which to block transactions
		byte					NumberOfPeriods;		//The number of periods being used
		byte					CurrentPeriod;			//The current fiscal period
	};

	struct stuLocation
	{
		CString					Address1;				//The first address line for this location
		CString					Address2;				//The second address line for this location
		CString					City;					//The city for this location
		CString					State;					//The state for this location
		CString					PostalCode;				//The zip code for this location or the country's specific postal code
		CString					Province;				//The province for a foreign address
		CString					Country;				//The country for this location
		CString					Contact;				//The person to contact for this location
	};

	// PBI 17783 BK 3/17/14 system variables for Merchantware Genius
	struct stuMerchantWare
	{
		CString					AccountName;
		CString					SiteID;
		CString					Password;
		CString					DBA;
		CString					TerminalID;				// begbert 11-16-2015 PBI 31304 : added this
		CString					IP_Address;
		bool					UseGeniusForPreauths;	// begbert 05-07-2014 PBI 19565 : added this
		CString					FileName;
	};

	//This stores settings specific to the current selected company the user has
	//selected to use
	struct stuCompanySettings
	{
		bool					IsFund;					// is this a fund organization
		bool					NewCompanyFiscalCalendarNotCreated;		// used to see if new company's fiscal calendar is created yet
		CString					ID;						//The company ID, can be up to 6 letters
		CString					Name;					//The company Name
		stuLocation				BillingInformation;		//A location structure for the billing information
		stuLocation				ShippingInformation;	//A location structure for the shipping information
		CString					Phone;					//The primary phone number for the company
		CString					Fax;					//The primary fax for the company
		CString					Extension;				//The extension for the primary number
		CString					EMail;				    //The primary email for the company
		CString					Website;				//The company's website
		CString					DefaultLocation;		//The companies default location
		CString					DataVersion;			//The version of the data this client needs
		CDataTable				Preferences;			//An XML string (in a structured wrapper) which has all the
		stuMerchantWare			MerchantWare;			// system variables for Merchant Ware Genius
	};

	//This stores information about any additional items which the user has installed.  If this flag is
	//true then that means the following conditions exist:
	//1.  The user has installed this item from the install and put in the correct CD Key.
	//2.  The user has this module in their current application path
	//3.  The user has picked a company that has this add on installed to it

	//This stores information about any additional items which the user has installed.  These are items
	//which are specially made for a particular customer.  If this flag is
	//true then that means the following conditions exist:
	//1.  The user has installed this item from the install and put in the correct CD Key.
	//2.  The user has this module in their current application path
	//3.  The user has picked a company that has this add on installed to it

	struct stuTool
	{
		bool					Installed;
		CString					ExecutablePath;
	};

	struct stuECommerce
	{
		CString					ExecutablePath;
	};

	struct stuThirdPartyTools
	{
		stuTool					DonorTracking;
		stuTool					UtilityBilling;
		stuTool					CCAuthorization;
		stuTool					BarcodeGenerator;
		stuECommerce			ECommerce;
	};

public:
	struct stuPackagesInstalled
	{
		stuLicensed				ACT;
		stuLicensed				FUND;
	};

	struct stuModulesInstalled
	{
		//Modules Installed
		stuLicensed				AccountsPayable;
		stuLicensed				AccountsReceivable;
		stuLicensed				BankReconciliation;
		stuLicensed				BillOfMaterials;
		stuLicensed				CostCenter;
		stuLicensed				Commissions;
		stuLicensed				Controller;
		stuLicensed				DataExchange;
		stuLicensed				ECommerce;
		stuLicensed				FinancialAnalysis;
		stuLicensed				GeneralLedger;
		stuLicensed				Inventory;
		stuLicensed				JobCost;
		stuLicensed				LotTracking;
		stuLicensed				Layaway;
		stuLicensed				MultiLocationInventory;
		stuLicensed				OrderEntry;
		stuLicensed				Payroll;
		stuLicensed				PointOfSale;
		stuLicensed				Polling;
		stuLicensed				PurchaseOrder;
		stuLicensed				ReportGenerator;
		stuLicensed				SalesEntry;
		stuLicensed				SpecialtyShop;
		stuLicensed				API;
		stuLicensed				DenaliBI;
		std::set<int>			CustomModifications;
		stuPackagesInstalled	PackagesInstalled;
		stuThirdPartyTools		ThirdPartyTools;
	};

	struct stuPlatforms
	{
		stuModulesInstalled		ACT;
		stuModulesInstalled		FUND;
	};

private:
	struct stuMagneticMediaConstants
	{
		CString 				EmployerID;
		CString 				Transmittername;
		CString 				StreetAddress;
		CString 				TransCity;
		CString 				TransState;
		CString 				TransZip;
		CString 				TransZipExt;
		CString 				ReturnName;
		CString 				ReturnAddress;
		CString 				ReturnCity;
		CString 				ReturnState;
		CString 				ReturnZip;
		CString 				ReturnZipExt;
		CString 				ControlCode;
		CString 				ReturnPhone;
		CString 				Country;
		CString 				Pin;
		CString 				ReturnCountry;
		CString 				ProblemNotify;
		CString 				CountryCode;
		CString 				ForeignStateProvince;
		CString 				ForeignPostalCode;
		CString 				CountryCodeReturn;
		CString 				ForeignStateProvinceReturn;
		CString 				ForeignPostalCodeReturn;
		CString 				ContactName;
		CString 				ContactExt;
		CString 				ContactFax;
		CString					Email;
	};

	//struct stuLocalization
	//{
	//	//Language DLL Instance
	//	HINSTANCE				LanguageDLL;
	//	CString					CurrencyCode;
	//	CString					CurrencyName;
	//	CString					CurrencySymbol;
	//	double					ConversionRate;
	//	byte					NumberOfDecimals;
	//	CString					CurrencyCountry;
	//	CString					NetLang;
	//	CString					DecimalSeparator;
	//	CString					ThousandsSeparator;
	//	byte					CurrencySymbolPos;
	//	byte					NegativeFormat;
	//};

	struct stuSystemColors
	{
		COLORREF				_3DDarkShadow;
		COLORREF				ButtonFace;
		COLORREF				ButtonHighlight;
		COLORREF				_3DLight;
		COLORREF				ButtonShadow;
		COLORREF				ActiveBorder;
		COLORREF				ActiveCaption;
		COLORREF				ApplicationWorkspace;
		COLORREF				Background;
		COLORREF				Desktop;
		COLORREF				ButtonText;
		COLORREF				CaptionText;
		COLORREF				GrayText;
		COLORREF				Highlight;
		COLORREF				HighlightText;
		COLORREF				InactiveBorder;
		COLORREF				InactiveCaption;
		COLORREF				InactiveCaptionText;
		COLORREF				ToolTipBackground;
		COLORREF				ToolTipText;
		COLORREF				Menu;
		COLORREF				MenuBar;
		COLORREF				MenuText;
		COLORREF				Scrollbar;
		COLORREF				Window;
		COLORREF				WindowFrame;
		COLORREF				WindowText;
	};

	struct stuSystemBrushes
	{
		CBrush					_3DDarkShadow;
		CBrush					ButtonFace;
		CBrush					ButtonHighlight;
		CBrush					_3DLight;
		CBrush					ButtonShadow;
		CBrush					ActiveBorder;
		CBrush					ActiveCaption;
		CBrush					ApplicationWorkspace;
		CBrush					Background;
		CBrush					Desktop;
		CBrush					ButtonText;
		CBrush					CaptionText;
		CBrush					GrayText;
		CBrush					Highlight;
		CBrush					HighlightText;
		CBrush					InactiveBorder;
		CBrush					InactiveCaption;
		CBrush					InactiveCaptionText;
		CBrush					ToolTipBackground;
		CBrush					ToolTipText;
		CBrush					Menu;
		CBrush					MenuBar;
		CBrush					MenuText;
		CBrush					Scrollbar;
		CBrush					Window;
		CBrush					WindowFrame;
		CBrush					WindowText;
	};

	struct stuSystemPens
	{
		CPen					_3DDarkShadow;
		CPen					ButtonFace;
		CPen					ButtonHighlight;
		CPen					_3DLight;
		CPen					ButtonShadow;
		CPen					ActiveBorder;
		CPen					ActiveCaption;
		CPen					ApplicationWorkspace;
		CPen					Background;
		CPen					Desktop;
		CPen					ButtonText;
		CPen					CaptionText;
		CPen					GrayText;
		CPen					Highlight;
		CPen					HighlightText;
		CPen					InactiveBorder;
		CPen					InactiveCaption;
		CPen					InactiveCaptionText;
		CPen					ToolTipBackground;
		CPen					ToolTipText;
		CPen					Menu;
		CPen					MenuBar;
		CPen					MenuText;
		CPen					Scrollbar;
		CPen					Window;
		CPen					WindowFrame;
		CPen					WindowText;
	};

	struct stuColors
	{
		COLORREF				Black;
		COLORREF				Yellow;
		COLORREF				DkYellow;
		COLORREF				Red;
		COLORREF				DkRed;
		COLORREF				Magenta;
		COLORREF				DkMagenta;
		COLORREF				Blue;
		COLORREF				DkBlue;
		COLORREF				Cyan;
		COLORREF				DkCyan;
		COLORREF				Green;
		COLORREF				DkGreen;
		COLORREF				Gray;
		COLORREF				DkGray;
		COLORREF				White;
		COLORREF				Cream;
		COLORREF				Scarlet;

		// 32 bit colors
		COLORREF				LtYellow;
		COLORREF				GoldenRod;
		COLORREF				LtGreen;
		COLORREF				BlueGreen;
		COLORREF				LtBlue;
		COLORREF				MedGray;
		COLORREF				Purple;
		COLORREF               OrangeAccent;
		COLORREF               MdGreen;
		COLORREF               Teal;
		//RCG - 06/08/2017 - PBI 39109 - Added new colors
		COLORREF				TreePoppy;	//Orangish
		COLORREF				RadicalRed;	//Pinkish
		COLORREF				MineShaft;	//Dark Grayish
		stuSystemColors		System;
	};

	struct stuBrushes
	{
		CBrush					Black;
		CBrush					Yellow;
		CBrush					DkYellow;
		CBrush					Red;
		CBrush					DkRed;
		CBrush					Magenta;
		CBrush					DkMagenta;
		CBrush					Blue;
		CBrush					DkBlue;
		CBrush					Cyan;
		CBrush					DkCyan;
		CBrush					Green;
		CBrush					DkGreen;
		CBrush					Gray;
		CBrush					DkGray;
		CBrush					White;
		CBrush					Cream;
		CBrush					Scarlet;

		// 32 bit colors
		CBrush					LtYellow;
		CBrush					GoldenRod;
		CBrush					LtGreen;
		CBrush					LtBlue;
		CBrush					MedGray;
		CBrush					Purple;
		CBrush                 OrangeAccent;
		CBrush                 MdGreen;
		CBrush                 Teal;
		//RCG - 06/08/2017 - PBI 39109 - Added new brush colors
		CBrush					TreePoppy;	//Orangish
		CBrush					RadicalRed;	//Pinkish
		CBrush					MineShaft;	//Dark Grayish
		stuSystemBrushes		System;
	};

	struct stuPens
	{
		CPen					Black;
		CPen					Yellow;
		CPen					DkYellow;
		CPen					Red;
		CPen					DkRed;
		CPen					Magenta;
		CPen					DkMagenta;
		CPen					Blue;
		CPen					DkBlue;
		CPen					Cyan;
		CPen					DkCyan;
		CPen					Green;
		CPen					DkGreen;
		CPen					Gray;
		CPen					DkGray;
		CPen					White;
		CPen					Cream;
		CPen					Scarlet;

		// 32 bit colors
		CPen					LtYellow;
		CPen					GoldenRod;
		CPen					LtGreen;
		CPen					LtBlue;
		CPen					MedGray;
		CPen                   OrangeAccent;
		CPen                   MdGreen;
		CPen                   Teal;
		stuSystemPens			System;
	};

	struct stuSystemIcons
	{
		HICON					Application;
		HICON					Hand;
		HICON					Question;
		HICON					Exclamation;
		HICON					Asterisk;
		HICON					Warning;
		HICON					Error;
		HICON					Information;
	};

	struct AFX_EXT_CLASS stuIcons
	{
		HICON	get_ModuleIcon(void) const;
		__declspec(property(get = get_ModuleIcon))	HICON		ModuleIcon;

		// begbert 09-28-2018 PBI 47270 : Replaced virtually all of the icons
		HICON	Agreement() const;
		HICON	ArrowDown() const;
		HICON	ArrowLeft() const;
		HICON	BackUp() const;
		HICON	Bank() const;
		HICON	Bills() const;
		HICON	Blocks() const;
		HICON	Brush() const;
		HICON	Budget() const;
		HICON	Build() const;
		HICON	CalculatorCheck() const;
		HICON	CalculatorRefresh() const;
		HICON	Calculator() const;
		HICON	CalendarAdd() const;
		HICON	CalendarCheck() const;
		HICON	CalendarDelete() const;
		HICON	CalendarEdit() const;
		HICON	Calendar() const;
		HICON	CashRegisterCheck() const;
		HICON	CashRegisterConfigure() const;
		HICON	CashRegister() const;
		HICON	ChangeOrder() const;
		HICON	ChartBar() const;
		HICON	ChartLine() const;
		HICON	ChartPie() const;
		HICON	Check() const;
		HICON	CheckbookAdd() const;
		HICON	CheckbookDelete() const;
		HICON	Checkbook() const;
		HICON	ClipboardAdd() const;
		HICON	ClipboardEdit() const;
		HICON	ClipboardTool() const;
		HICON	ClockArrows() const;
		HICON	ConfigureData() const;
		HICON	Configure() const;
		HICON	Copy() const;
		HICON	CreditCardInformation() const;
		HICON	CreditCard() const;
		HICON	DatabaseAdd() const;
		HICON	DataHistorical() const;
		HICON	DebitCard() const;
		HICON	Delete() const;
		HICON	DiscountAdd() const;
		HICON	Discount() const;
		HICON	Drill() const;
		HICON	Euro() const;
		HICON	Expensify() const;
		HICON	FileAdd() const;
		HICON	FileCheck() const;
		HICON	FileConfigure() const;
		HICON	FinancialTransactionCheck() const;
		HICON	FundEdit() const;
		HICON	Fund() const;
		HICON	GiftCardAdd() const;
		HICON	GiftCardEdit() const;
		HICON	GiftCardInformation() const;
		HICON	GiftCardRefresh() const;
		HICON	GlobeDate() const;
		HICON	GlobeEdit() const;
		HICON	HourGlass() const;
		HICON	IdentificationWarning() const;
		HICON	Identification() const;
		HICON	Image() const;
		HICON	Inspect() const;
		HICON	InventoryCost() const;
		HICON	InventoryTransfer() const;
		HICON	LedgerAdd() const;
		HICON	LedgerConfigure() const;
		HICON	LedgerEdit() const;
		HICON	LedgerRefresh() const;
		HICON	Ledger() const;
		HICON	MagnifyingGlass() const;
		HICON	Mailbox() const;
		HICON	MoneyExchange() const;
		HICON	MoneyTransfer() const;
		HICON	MoneyWithdraw() const;
		HICON	Money() const;
		HICON	Options() const;
		HICON	PayCheck() const;
		HICON	PayMoney() const;
		HICON	PeopleConstructionGroup() const;
		HICON	PeopleConstructionMan() const;
		HICON	PeopleCopy() const;
		HICON	PeopleFinancialAdd() const;
		HICON	PeopleFinancial() const;
		HICON	PeopleManFront() const;
		HICON	PeoplePasswordSecurity() const;
		HICON	PeopleWomanFront() const;
		HICON	PrinterLabels() const;
		HICON	PrinterRestricted() const;
		HICON	Printer() const;
		HICON	PuzzleAdd() const;
		HICON	QuestionMark() const;
		HICON	RawMaterials() const;
		HICON	ReceiveGoods() const;
		HICON	Refresh() const;
		HICON	Renumber() const;
		HICON	Save() const;
		HICON	Scale() const;
		HICON	Scanner() const;
		HICON	SendMail() const;
		HICON	ShoppingCartAdd() const;
		HICON	SpreadsheetArrowRight() const;
		HICON	SpreadsheetEdit() const;
		HICON	StorefrontConfigure() const;
		HICON	StorefrontInformation() const;
		HICON	StorefrontLock() const;
		HICON	StorefrontRefresh() const;
		HICON	Tools() const;
		HICON	TransactionAdd() const;
		HICON	TransactionDelete() const;
		HICON	TransactionFinancialCheck() const;
		HICON	TransactionFinancial() const;
		HICON	TransactionSearch() const;
		HICON	Transaction() const;
		HICON	Trash() const;
		HICON	TruckAdd() const;
		HICON	TruckRefresh() const;
		HICON	Truck() const;
		HICON	W2Form() const;
		HICON	Wallet() const;


		// global/original icons
		HICON					Utility32()			const;	// no longer used except on some old forms that are probably not being used anymore
		HICON					PrefCode32()		const;
		HICON					FatalServer()		const;
		HICON					FatalClient()		const;
		HICON					XMLClient()			const;
		HICON					XMLServer()			const;
		HICON					DrillDown32()		const;
		HICON					DrillDown16()		const;

		// Version 3.0 BK Facelift 6/2010 updated icons (replaced 16x16 icons with new ones)
		// 16x16 icons - modules
		HICON					AP16()				const;
		HICON					AR16()				const;
		HICON					BR16()				const;
		HICON					CT16()				const;
		HICON					GL16()				const;
		HICON					IN16()				const;
		HICON					OE16()				const;			// oe sales entry
		HICON					PO16()				const;
		HICON					PR16()				const;
		HICON					PS16()				const;			// pos sales entry
		HICON					SE16()				const;			// sales entry
		HICON					SM16()				const;			// sales module
		HICON					Current16()			const;			// Icon for the current module
		HICON					Module16(UINT uinModuleID) const;	// Icon for a module
		HICON					Help16()			const;
		HICON					DTails()			const;
		HICON					SalesDTails()		const;
		//	PBI 24421	04/21/2015	mvt - Set up Job Cost integration
		HICON					JC16()				const;

		// 32x32 icons - modules
		HICON					AP32()				const;
		HICON					AR32()				const;
		HICON					BR32()				const;
		HICON					CT32()				const;
		HICON					GL32()				const;
		HICON					IN32()				const;
		HICON					PO32()				const;
		HICON					PR32()				const;
		HICON					SM32()				const;			// sales module
		//HICON					Current32()			const;			// Icon for the current module
		//HICON					Module32(UINT uinModuleID) const;	// Icon for a module
		HICON					Reports()			const;
		//	PBI 24421	04/21/2015	mvt - Set up Job Cost integration
		HICON					JC32()				const;
		HICON					CMS()				const;
		HICON					DenaliBI()			const;

		// global/general icons (version 3.0 BK 6/2010)
		//HICON					ArrowLeft()				const;
		//HICON					ArrowRight()			const;
		HICON					BillingAddress()		const;
		HICON					ChangeDateLocal()		const;
		HICON					Clear()					const;
		HICON					DisplayTableInfo()		const;
		HICON					Find()					const;
		HICON					GenerateFromExternal()	const;
		HICON					GenerateFromRecur()		const;
		HICON					GlobalChange()			const;
		HICON					ImportExport()			const;
		HICON					Lookups()				const;
		HICON					ModulePrefs()			const;
		HICON					Post()					const;
		HICON					Purge()					const;
		HICON					Recalculate()			const;
		//HICON					Recall()				const;
		HICON					ResetTotals()			const;
		//HICON					SaveWithoutPrinting()	const;
		HICON					SelectBatch()			const;
		HICON					SetupBatch()			const;
		HICON					SetupRecurring()		const;
		//HICON					SetupScreenLayout()		const;
		HICON					Stop()					const;
		HICON					UserLogin()				const;
		HICON					UsersLoggedIn()			const;
		//HICON					VoidDelete()			const;

		// CT - Controller icons
		HICON					SelectCompany()			const;
		HICON					AddChangeUser()			const;
		HICON					CompanyInfo()			const;
		HICON					InstallModules()		const;
		HICON					ChangeDateGlobal()		const;
		HICON					FiscalCalendar()		const;
		HICON					ChangePassword()		const;
		HICON					BackupCompany()			const;
		HICON					RestoreCompany()		const;
		HICON					CMSCurrencies()			const;
		HICON					ExchangeRates()			const;
		HICON					ProcessPayroll()		const;
		HICON					BackupLog()				const;

		// AP - Accounts Payable icons
		HICON					APVendor()				const;
		HICON					APEnterBills()			const;
		HICON					APAdjustPayBills()		const;
		HICON					APDiscountMessage()		const;
		HICON					APPrepareAutoPayments()	const;
		HICON					APProcessPrepaids()		const;
		HICON					APAdjustPosted()		const;
		HICON					GenerateElectronic()	const;		// used in AP and AR

		// AR - Accounts Receivable icons
		HICON					ARCustomers()			const;
		HICON					ARMultipleShipAdderss()	const;
		HICON					AREnterBillsPayments()	const;
		HICON					ARCalculateFinanceChg() const;

		// BR - Bank Reconciliation icons
		HICON					BRBankAccount()			const;
		HICON					BRBankRegister()		const;
		HICON					BREnterBankActivity()	const;
		HICON					BRVoidBankActivity()	const;
		HICON					BRReconcileBankAcct()	const;
		HICON					BRPayeeCardFile()		const;
		HICON					BRAddFromAP()			const;

		// GL - General Ledger icons
		HICON					GLAccounts()			const;
		HICON					GLJournalEntry()		const;
		HICON					GLAdjustAccounts()		const;
		HICON					GLProcessAllocations()	const;
		HICON					GLSpreadsheetExport()	const;
		HICON					GLCloseYear()			const;
		HICON					GLCloseFund()			const;
		HICON					GLSetupFund()			const;
		HICON					GLBudgetManagement()	const;	// begbert 02-04-2011 FUND : Added for Budget Management form.

		// IN - Inventory icons
		HICON					INAdjustStockQty()		const;
		HICON					INImportAdjustments()	const;
		HICON					INRecalculateDate()		const;
		HICON					INSelectSerialNos()		const;
		HICON					INSetupKits()			const;
		HICON					INStock()				const;
		HICON					INStockAlias()			const;
		HICON					INVarianceAmount()		const;

		// PO - Purchase Order
		HICON					POAddItems()			const;
		HICON					POEnterPurchaseOrders()	const;
		HICON					POReceiveItems()		const;

		// Sales icons
		HICON					ShipToButton()			const;
		HICON					SSAgeVerification()		const;
		//HICON					SAGiftCardOptions()		const;
		HICON					SALoadDevice()			const;
		HICON					SADeviceTest()			const;
		HICON					SAInventoryPricing()	const;
		HICON					TenderSale()			const;
		HICON					UnprintedItems()		const;
		HICON					PreviewPrintFormat()	const;
		// Sales tenders
		HICON					SAECash32()				const;
		HICON					SAECharge32()			const;
		HICON					SAECC32()				const;
		HICON					SAECheck32()			const;
		HICON					SAEDebit32()			const;
		HICON					SAECC_Swiper()			const;
		HICON					ForeignCur32()			const;
		//HICON					PreAuthorization()		const;
		HICON					SAEDeposit32()			const;
		HICON					SAEAltTender32()		const;
		HICON					GiftCert32()			const;
		//HICON					DebitCard32()			const;
		HICON					DepositStatus32()		const;
		HICON					RefreshRescan()			const;
		HICON					SAEBack32()				const;
		HICON					SAECancel32()			const;
		HICON					Done32()				const;
		HICON					SAEAuthorize32()		const;
		//HICON					AddTip()				const;		// not using until button is added to tender
		// Sales - OE icons
		HICON					OEEnterOrders()			const;
		HICON					OERepairBatchUtility()	const;
		// Sales - POS icons
		HICON					PSEnterSales()			const;
		HICON					PSBalanceRegister()		const;
		HICON					PSRepairRegister()		const;
		//HICON					PaidOut32()				const;
		HICON					OpenDrawer32()			const;
		// Sales - discount invoice icons
		HICON					DiscountInvoiceTotal()	const;
		HICON					DiscountSinceLast32()	const;
		HICON					DiscountLineItem32()	const;
		// Sales - layaway icons
		//HICON					NewLayaway32()			const;
		//HICON					PayLayaway32()			const;
		//HICON					PickupLayaway32()		const;
		//HICON					CancelLayaway32()		const;
		HICON					LayawaySchedule32()		const;
		// Sales - gift certificate icons
		HICON					GCPurchase32()			const;
		HICON					GCIncrement32()			const;
		HICON					GCBalanceInq32()		const;
		HICON					GCBalanceTransfer32()	const;
		HICON					GCAdjustment32()		const;

		stuSystemIcons			System;

		// Version 3.0 BK Facelift removed the following icons:
				//HICON					DX16()				const;
				//HICON					JC16()				const;
				//HICON					Save32()			const;
				//HICON					Blank16()			const;

				// InTimeTec: 07/08/2013 : HS : Copy Stock Item Alias
		HICON					Blank32()			const;
		// InTimeTec: End

		//HICON					TTCritical16()		const;
		//HICON					TTInformation16()	const;
		//HICON					TTFatal16()			const;
		//HICON					Servers()			const;
		//HICON					ValidClient()		const;
		//HICON					ValidServer()		const;
		//HICON					XML16()				const;
		//HICON					SetupWizard16()		const;
		//HICON					Zoom16()			const;
		//HICON					SAEOK32()			const;
		//HICON					SAEVoid16()			const;
		//HICON					SAEVoid32()			const;
		//HICON					Back32()			const;
		//HICON					Cancel32()			const;
		//HICON					PREmpData16()		const;
		//HICON					PREmpData32()		const;
		//HICON					BRStatementReconcile32() const;
		//HICON					INStock16()			const;
		//HICON					INStock32()			const;
		//HICON					POAdd16()			const;
		//HICON					POAdd32()			const;
		//HICON					POBuyer16()			const;
		//HICON					POBuyer32()			const;
		//HICON					POBuyer2_16()		const;
		//HICON					POBuyer2_32()		const;
		//HICON					POEdit16()			const;
		//HICON					POEdit32()			const;
		//HICON					POPrintEdit32()		const;
		//HICON					POPrintLabels32()	const;
		//HICON					POReceive16()		const;
		//HICON					POReceive32()		const;
		//HICON					POSer16()			const;
		//HICON					POSer32()			const;
		//HICON					Edit16()			const;
		//HICON					Edit32()			const;
		//HICON					Credit_Card_Gold()	const;
		//HICON					Accounts16()		const;
		//HICON					Adjustments16()		const;
		//HICON					Adjustments32()		const;
		//HICON					Padlock32()			const;
		//HICON					SalesHist32()		const;
		//HICON					ShipTo32()			const;
		//HICON					ViewPic32()			const;
		//HICON					ViewPic16()			const;
		//HICON					ViewPrinter32()		const;
		//HICON					ViewPrinter16()		const;
		//HICON					CustBilling32()		const;
		//HICON					CustCharge32()		const;
		//HICON					PayAdjust32()		const;
		//HICON					PayAdjust16()		const;
		//HICON					AddFrom32()			const;	// begbert 04-26-2007 1-27238 : added this
		//HICON					SAECash16()			const;	// begbert 05-10-2007 1-27260 : added this
		//HICON					BRStatementRec16()	const;	// begbert 05-10-2007 1-27260 : added this
		//HICON					BREnterBank16()		const;	// begbert 05-10-2007 1-27260 : added this
		//HICON					BRVoidBank16()		const;	// begbert 05-10-2007 1-27260 : added this
		//HICON					ImportTrans32()		const;	// begbert 06-29-2007 1-28018 : added this
		//HICON					Cal32()				const;	// begbert 08-28-2007 : added this
		//HICON					POMail16()			const;	// begbert 01-03-2008 : added this
		//HICON					Cancel16()			const;	// begbert 01-03-2008 : added this
		//HICON					POBack16()			const;	// begbert 01-03-2008 : added this
		//HICON					FileNew16()			const;	// begbert 01-03-2008 : added this
		//HICON					Forward16()			const;	// begbert 01-03-2008 : added this
		/////////////////////////////////
		//  Tooltip icons
		/////////////////////////////////
		//HICON					Critical16;
		//HICON					Information16;
		//HICON					ValidateServer;
		//HICON					ValidateClient;
		//HICON					Trans32;

	};

	struct stuSystemBitmaps
	{
		HBITMAP					ButtonCorners;
		HBITMAP					ButtonSize;
		HBITMAP					Check;
		HBITMAP					CheckBoxes;
		HBITMAP					Close;
		HBITMAP					Combo;
		HBITMAP					DownArrow;
		HBITMAP					DownArrowD;
		HBITMAP					DownArrowI;
		HBITMAP					LeftArrow;
		HBITMAP					LeftArrowD;
		HBITMAP					LeftArrowI;
		HBITMAP					MNArrow;
		HBITMAP					Reduce;
		HBITMAP					Reduced;
		HBITMAP					Restore;
		HBITMAP					Restored;
		HBITMAP					RightArrow;
		HBITMAP					RightArrowD;
		HBITMAP					RightArrowI;
		HBITMAP					Size;
		HBITMAP					UpArrow;
		HBITMAP					UpArrowD;
		HBITMAP					UpArrowI;
		HBITMAP					Zoom;
		HBITMAP					ZoomD;
	};

	struct stuBitmaps
	{
		HBITMAP					IRSEFile;
		HBITMAP					CMSLogo_Gray;
		HBITMAP					CMSBackDrop;
		HBITMAP					Clouds;
		HBITMAP					CougarSideBar;
		HBITMAP					CMSBackDropIntegration;
		HBITMAP					CougarHeadSplash;
		HBITMAP					CMSBackDropIntegrationDeveloper;
		HBITMAP					CMSInstallBanner;
		HBITMAP					Camera16;
		HBITMAP					Camera16Disabled;
		HBITMAP					Drilldown16;
		HBITMAP					Blank;
		stuSystemBitmaps		System;
		//	PBI 28141	06/12/2015	mvt - Changed to larger splash screen bitmap
		HBITMAP					CougarHeadSplashLg;
	};

	struct stuSystemCursors
	{
		HCURSOR					ApplicationStarting;
		HCURSOR					Arrow;
		HCURSOR					Crosshair;
		HCURSOR					IBeam;
		HCURSOR					No;
		HCURSOR					SizeAll;
		HCURSOR					SizeNESW;
		HCURSOR					SizeNS;
		HCURSOR					SizeNWSE;
		HCURSOR					SizeWE;
		HCURSOR					UpArrow;
		HCURSOR					Wait;
		HCURSOR					Sample;
		HCURSOR					Hand;
		HCURSOR					Question;
		HCURSOR					Note;
		HCURSOR					Warning;
		HCURSOR					Error;
		HCURSOR					Information;
	};

	struct stuCursors
	{
		HCURSOR					MagnifyingGlass;
		HCURSOR					DrillDown;
		stuSystemCursors		System;
	};

	struct stuFonts
	{
		CFont					Normal;
		CFont					Italic;
		CFont					Underline;
		CFont					Bold;
		CFont					BoldLarge;
		CFont					NormalUnderline;
		CFont					BoldUnderline;
		CFont					BoldUnderlineLarge;
		CFont					Small;
		CFont					SuperBig;
		CFont					NineItalic;
		CFont					CourierFont;		//Peter Ringering - 12/05/2003 - To display XML
		CFont					Giant;				// BK 8/31/04 AP Save Message
		// InTimeTec: 10/25/2013: HS: Epic 580 D_DEV_AR_Block Charges to Past Due Customers
		CFont					ArialBlack;
		CFont					ArialSeven;		//RCG - 06/08/2017 - PBI 39109 - Added new font for Help About
		CFont					ArialTen;		//RCG - 06/08/2017 - PBI 39109 - Added new font for Classic Navigation Splash Screens
		CFont					ArialTwelve;	//RCG - 06/08/2017 - PBI 39109 - Added new font for Help About

		//Symbol fonts loaded here for common windows standard symbols.
		//Load more here for special symbols.
		struct stuSymbolFonts
		{
			//Different points for Marlett go here.
			struct stuMarlett
			{
				CFont			Marlett10;
				//Initialize Marlett structure for different points.
				void InitMarlett(LOGFONT& lfMarlett, const short& nHeight)
				{
					ZeroMemory(&lfMarlett, sizeof(LOGFONT));
					lstrcpy(lfMarlett.lfFaceName, _T("Marlett"));
					lfMarlett.lfHeight = nHeight;
					lfMarlett.lfCharSet = DEFAULT_CHARSET;
				}
				bool CreateAll(LOGFONT& lf)
				{
					//Initialize 10 point Marlett.
					InitMarlett(lf, 10);
					if (Marlett10.CreateFontIndirect(&lf) == FALSE)
						return false;
					return true;
				}
			}Marlett;

			//Load all symbol fonts.
			bool Load()
			{
				LOGFONT lf;
				//Create all marletts
				if (!Marlett.CreateAll(lf))
					return false;
				//Other inits go here.
				return true;
			};
		}SymbolFonts;
	};

	struct __declspec(dllexport) stuModuleSettings
	{
		CModuleSettings* CurrentSettings;
		CAPSystemInformation	APSettings;
		CARSystemInformation	ARSettings;
		CBRSystemInformation	BRSettings;
		CCTSystemInformation	CTSettings;
		CDXSystemInformation	DXSettings;
		CGLSystemInformation	GLSettings;
		CINSystemInformation	INSettings;
		CJCSystemInformation	JCSettings;
		COESystemInformation	OESettings;
		CPOSystemInformation	POSettings;
		CPSSystemInformation	PSSettings;
		CPRSystemInformation	PRSettings;
		CSASystemInformation	SASettings;

		CMS::ISalesSystemInterfacePtr	GetSalesInterface(UINT nModuleID);
		CMS::ISalesSystemInterfacePtr	GetCurrentSalesInterface(void);
	};

	struct stuDrawing
	{
		stuIcons				Icons;
		stuBitmaps				Bitmaps;
		stuCursors				Cursors;
		stuBrushes				Brushes;
		stuPens					Pens;
		stuColors				Colors;
		stuFonts				Fonts;
		bool					FontsLoaded;
	};

	// DTG, 5.12.2004
public:

	struct __declspec(dllexport) stuServerInformation
	{
		CString					DBType;
		CString					Name;
		CString					Database;
		CString					User;
		CString					Password;
		int						Timeout;
		int						PoolTime;
		int						MinPoolSize;
		int						MaxPoolSize;
		CString					Application;
		bool					Reset;
		bool					Enlist;
		bool					Security;
		CString					NetworkType;
		int						PacketSize;
		bool					Persist;
		bool					Pooling;
		CString					Workstation;
		int						Port;
		bool					GetSchema;
		bool					MaintForm;
		bool					MARS;
		bool					Encrypt;
		bool					Async;
		bool					UseMaster;
		CString					MirrorServer;
		CString					Schema;
		CString					DataVersion;
		bool					ReadOnly;
		// Methods of structure
		void					Initialize();
		void 					AppendServerInfo(CXMLElement& eleServers, const CString szServer);
		void					Parse(CXMLElement& eleServers);
	};

	struct stuLookupSelected
	{
		UINT					FormID;
		UINT					ModuleID;
		CXMLParams				XMLParams;
		CXMLDocument			KeyInformation;	//PTR.06.15.2005.1-16432
	};

	struct __declspec(dllexport) stuServers
	{
		stuServerInformation	PrimaryServer;
		stuServerInformation	CompanyServer;
		stuServerInformation	ArchiveServer;
		//KPM, 4/26/2004, The server machine name.  Currently only
		//the IN Module retrieves this, modify gblstartup in gblforms
		//to make other modules set this value if you need it.
		CString					ServerComponentsMachineName;
		CString					AppendServerInfo(const CString& szData);
		void					Initialize();
		void					Parse(const CString& szData);
	};

	// DTG, 5.12.2004
private:

	struct stuSegmentInfo
	{
		__int8					Length;
		__int8					Type;
		CString					Description;
		bool					isFund;
	};

	struct stuSegmentSetup
	{
		stuSegmentInfo			Segments[6];
		__int8					NumberOfSegments;
		__int8					FundSegment;
		stuSegmentInfo* FundSegInfo;				// Pointer to the segment information for the fund segment.  Only set in fund.  Do not use in act as you will get an error.
		__int8					TotalAccountLength;
		CString					SeparationCharacter;
	};

	struct stuSecurity
	{
		CString					UserSettings;
		CString					CompanySettings;
		CStringArray			GroupSettings;
	};

	//This stores the path to certain application resources
	struct stuApplicationPaths
	{
		CString					szApplicationPath;
		CString					szReportPath;
		CString					szHelpPath;
	};

	//Added to handle the help file
	struct stuHelpFileSettings
	{
		CString					HelpFileName;	//The file name for the help file on the local system
		CString					HelpFilePath;	//The path to the help files on the local system
		CString					HelpRemotePath; //The path to the remote help file (future use, not used now)
		CString					HelpWebURL;		//The url to the web help files
		CString					HelpWebName;	//The help file name on the website (or directory to topic files)
		short					HelpType;		//The type of help being used.  0=local, 1=web, 2=remote (not on this computer)
	};

	struct stuTipOfTheDay
	{
		CString					FileName;
		BOOL					ShowTipsAtStartup;
		int						CurrentTipNumber;
	};

	// DTG, 5.12.2004
private:

	struct stuImageStorageData
	{
		stuImageStorageData() : Path_ImageCache(_T("")), Path_ImageNotFound(_T("")), Path_ImageLoading(_T("")) {};
		CString Path_ImageCache;
		CString Path_ImageNotFound;
		CString Path_ImageLoading;
	};

	// InTimeTec: 9.1.2012.KP - Custom Reports
	struct stuReportStorageData
	{
		stuReportStorageData() : Path_ReportCache(_T("")), Path_ReportNotFound(_T("")), Path_ReportLoading(_T("")) {};
		CString Path_ReportCache;
		CString Path_ReportNotFound;
		CString Path_ReportLoading;
	};
	// InTimeTec: End

	//PGP(05/17/2004) - Added structure to use with CMSWaitCursor.
	struct stuWaitMessage
	{
		//bValid is a validity indicator flag which is set to
		//true when the structure is ready to use.
		LONG lWaitCount;
		//string ID of the message to be shown in statusbar.
		UINT unMessageID;
		//ID of the thread which is using this structure.(reserved for future use)
		DWORD dwThreadID;
		//Message string to be shown in status bar. (Used when a resource string is
		//modified to indicate context information. unMessageID is set to 0 when this is used)
		CString szMessage;
	};

	VirtualUI* vui;
	// DTG, 5.12.2004
public:

	CAppSettings				ApplicationSettings;
	CGBLUserInfo				UserInformation;
	stuCompanySettings			CompanySettings;
	stuFiscalCalendar			FiscalCalendar;
	stuServers					Servers;
	stuModulesInstalled			ModulesInstalled;
	stuPlatforms				InstalledPlatforms;
	CGBLLocalization			Localization;
	stuModuleSettings			ModuleSettings;
	stuDrawing					Drawing;
	stuCommandLineSettings		CommandLineSettings;
	stuSegmentSetup				SegmentSetup;
	stuMagneticMediaConstants	MagneticMediaConstants;
	stuSecurity					Security;
	stuTemporaryStorage			TemporaryStorage;
	bool						SettingsLoadedSuccessfully;
	CXInfoTip					ToolTip;
	stuSystemData				SystemData;
	CRuntimeClass* pChildFrame;
	CRuntimeClass* pDocument;
	CWinApp* pTheApp;
	ICMSWinApp* pCMSApp;
	IGBLSIOutputModule* pOutputModule;	//Module implements this to add system information parameters specific to it (like in Sales Entry)
	CGBLRegistry				RegistryEditor;
	volatile bool				CancelApplicationLoad;
	CGBLBatchInfo				CurrentBatch;
	stuApplicationPaths			ApplicationPaths;
	stuHelpFileSettings			HelpSettings;
	stuTipOfTheDay				TipOfTheDay;
	CGBLUserRightsInformation	UserRight;

	//KPM, 8/20/2003, This will track instances of forms being opened, needed
	//more advanced mechanism for BR
	COpenFormTracker			OpenFormTracker;

	//Although this will actually be the CMainFrame, You cannot type cast it into that because this will be used
	//by every module.  Each modules CMainFrame needs to be imported with a #import so choosing the correct module
	//is impossible in this global.  If you need specialized methods of a specific main frames class, then use
	//AfxGetApp() and type cast it into the app type you want (or just type cast this variable like
	//(CMainFrame*)ApplicationMainFrame->xxx.
	ICMSMainFrameWnd* get_IApplicationMainFrame(void) const { return m_pIApplicationMainFrame; }
	void put_IApplicationMainFrame(ICMSMainFrameWnd* p_IFrameWnd) { m_pIApplicationMainFrame = p_IFrameWnd; }
	CFrameWnd* get_ApplicationMainFrame(void) const { return m_pIApplicationMainFrame ? m_pIApplicationMainFrame->Frame : NULL; }

	__declspec(property(get = get_IApplicationMainFrame, put = put_IApplicationMainFrame))	ICMSMainFrameWnd* IApplicationMainFrame;
	__declspec(property(get = get_ApplicationMainFrame))															CFrameWnd* ApplicationMainFrame;

	//PTR.07.26.2005 - Removed.  This is causing problems with threads stepping on each other where one thread
	//has an error and another doesn't.
	//stuErrorReturn			ErrorReturn;  //01/11/2003 - Peter Ringering - Error Return from dialog
	CCMSCurrencyCollection		Currencies;

	// Used to check and see if the modules the company says they have installed are actually installed legally on the users machine
	static void					CheckInstalledModulesAfterLogin(void);
	static CString				GetRegisteredOrganization(void);
	static CString				GetRegisteredOwner(void);
	void						SetModuleLicense(stuLicensed& stuMod, bool bExpired, bool bInCompany, bool bIsUsable, bool bLicensed, bool bTimeLocked);
	void						CheckModuleLicense(stuLicensed& stuMod);
	void						ResetModuleLicense(stuLicensed& stuMod, enuModuleSetting p_Part);
	void						CheckModuleLicenses();
	void						ResetModuleLicenses(enuModuleSetting p_Part);
	void						CheckModuleLicenses(stuModulesInstalled& ModInst);
	void						ResetModuleLicenses(stuModulesInstalled& ModInst, enuModuleSetting p_Part);
	void						ResetModulesInfo();
	void						ResetModulesInfo(stuModulesInstalled& ModInst);
	bool						GetPackageInstalled(CXMLDocument* objDoc = NULL);	// Used to determine the software suite that was installed
	bool						GetPackageInstalled(CXMLElement* objDoc, stuModulesInstalled& ModInst);
	bool						GetModuleInstalledSetting(stuModulesInstalled& ModInst, UINT nModuleID, enuModuleSetting p_Part);
	bool						GetModuleInstalledSetting(stuLicensed& stuMod, enuModuleSetting p_Part);
	void						SetModuleInstalledSetting(CXMLElement* objDoc, stuModulesInstalled& ModInst, enuModuleSetting p_Part);
	void						SetModuleInstalledSetting(CXMLElement* objDoc, stuModulesInstalled& ModInst, UINT nModuleID, enuModuleSetting p_Part);
	void						SetModuleInstalledSetting(stuModulesInstalled& ModInst, UINT nModuleID, enuModuleSetting p_Part, bool p_bValue);
	void						SetModuleInstalledSetting(stuLicensed& stuMod, enuModuleSetting p_Part, bool p_bValue);
	void						SetCustomModInstalled(CXMLElement* objDoc, stuModulesInstalled& ModInst);
	void						SetModuleInstalledSetting(stuLicensed& FromstuMod, stuLicensed& TostuMod);
	bool						SetCustomModInstalledValue(stuModulesInstalled& ModInst, int nCustomMod);
	bool						CustomModInstalled(stuModulesInstalled& ModInst, int nCustomMod);
	bool						CustomModInstalled(int nCustomMod);
	CString						CustomeModsInstalled();
	CString						AppendCustomeModsInstalled(const CString& szData);

	CString						DisplayableBatchCode(void) const;

	//Used to send refresh lock info to the server
	CClientLockRefresh			ClientLockRefresh;
	CCMSDynThemeManager			m_clsDynThemeManager;	//used to link to xp theme dlls.
	CMSProductAudit::CProductAuditAPI* ProductAuditAPI;

	//KPM, 4/29/2004, added for image caching
	stuImageStorageData			ImageStorageData;
	// InTimeTec: 10.9.2012.KP - Custom Reports
	stuReportStorageData        ReportStorageData;
	// InTimeTec: End
	//PGP(05/17/2004) Added for displaying status messages when processing - See error 1-10285 for details
	stuWaitMessage				WaitMessageDetails;

	CString GetAuthorizerDLLSettingsXML(void) const;
	void SetAuthorizerDLLSettingsXML(const CString& szAuthorizer);

	CString GetAuthorizerDLLFilepath(void) const;

	CString GetAuthorizerDLLFilename(void) const;
	void SetAuthorizerDLLFilename(const CString& szFilename);

	CMS::CCAuthorizationType GetCCAuthLevel(void) const;
	CMS::CheckAuthorizationType GetCheckAuthLevel(void) const;
	bool GetUseAddressVerification(void) const;
	bool GetAllowBlankAddress(void) const;

	bool GetAutoFillCreditCardInfo(void) const;
	void SetAutoFillCreditCardInfo(bool bolAutofill);

	CString GetPaymentProcessor(void) const;

	int GetVerificationSoftwareIDX(void) const;
	void SetVerificationSoftwareIDX(int nCCVerificationSoftware);

	bool GetUsingTPISmartPayments(void) const;
	void SetUsingTPISmartPayments(bool bolUsing);

	bool GetRunningInVirtualUI(void) const;

	VirtualUI* GetVirtualUI(void);
	void DownloadFile(std::wstring LocalFilename);
	void DownloadFile(std::wstring LocalFilename, std::wstring RemoteFilename);
	void DownloadFile(std::wstring LocalFilename, std::wstring RemoteFilename, std::wstring MimeType);
	void PreviewPdf(std::wstring FileName);

	__declspec(property(get = GetAuthorizerDLLSettingsXML, put = SetAuthorizerDLLSettingsXML))	CString	AuthorizerSettings;
	__declspec(property(get = GetAuthorizerDLLFilepath))										CString	AuthorizerFilepath;
	__declspec(property(get = GetAuthorizerDLLFilename, put = SetAuthorizerDLLFilename))		CString	AuthorizerFilename;
	__declspec(property(get = GetCCAuthLevel))													CCAuthorizationType	CCAuthLevel;
	__declspec(property(get = GetCheckAuthLevel))												CheckAuthorizationType CheckAuthLevel;
	__declspec(property(get = GetUseAddressVerification))										bool UseAddressVerification;
	__declspec(property(get = GetAllowBlankAddress))											bool AllowBlankAddress;
	__declspec(property(get = GetAutoFillCreditCardInfo, put = SetAutoFillCreditCardInfo))		bool AutoFillCreditCard;
	__declspec(property(get = GetPaymentProcessor))												CString PaymentProcessor;
	__declspec(property(get = GetUsingTPISmartPayments, put = SetUsingTPISmartPayments))		bool UsingTPI;
	__declspec(property(get = GetVerificationSoftwareIDX, put = SetVerificationSoftwareIDX))	int	 VerificationSoftwareIdx;
	__declspec(property(get = GetRunningInVirtualUI))											bool RunningInVirtualUI;
	__declspec(property(get = GetVirtualUI))													VirtualUI* VUI;

private:
	ICMSMainFrameWnd* m_pIApplicationMainFrame;
	void LoadPrintOptions(_bstr_t bstrXMLString);
};
