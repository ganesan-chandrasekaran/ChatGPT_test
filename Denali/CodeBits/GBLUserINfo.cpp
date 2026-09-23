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
 //#include "GBLUserInfo.h"

 // CGBLUserINfo

CGBLUserInfo::CGBLUserInfo()
{
	m_bLoginSuccess = false;
	m_bTryLogin = false;
	m_szPrivatePassword = _T("");
	ArcTimeURL = 0;
	ArcBillURL = 0;
}

CGBLUserInfo::~CGBLUserInfo()
{
}

extern CGBLSystemInformation* g_pGBLSystemInformationCMSDll;

/// <summary>
/// Determines whether company a fund company by looking at the data in the company list.
/// </summary>
/// <param name="szCompanyCode">The company code.</param>
/// <returns>
///   <c>true</c> if company is fund; otherwise, <c>false</c>.
/// </returns>
bool    CGBLUserInfo::IsCompanyFund(CString szCompanyCode)
{
	bool bIsFundSelected = false;  // Assume Act

	if (g_pGBLSystemInformationCMSDll->ModulesInstalled.PackagesInstalled.FUND.Licensed &&
		g_pGBLSystemInformationCMSDll->ModulesInstalled.PackagesInstalled.ACT.Licensed)
	{
		CDataRow  objDR;
		for (long nRow = 0; nRow < ValidCompanies.getRowCount(); nRow++)
		{
			ValidCompanies.GetRow(nRow, &objDR);
			if (szCompanyCode == objDR.XMLStr(_T("strCompanyID")).MakeUpper())
				bIsFundSelected = objDR.XMLBOOL(_T("bolFundCompany"), false);
			objDR.Dispose();
		}
	}
	else if (g_pGBLSystemInformationCMSDll->ModulesInstalled.PackagesInstalled.FUND.Licensed)
		bIsFundSelected = true;  // Fund Only so just go with it

	return bIsFundSelected;
}

void	CGBLUserInfo::Empty()
{
	ID.Empty();
	Name.Empty();
	Password.Empty();
	Reset = false;
	FirstTime = false;
	IgnoreCase = false;
	Changed.SetDate(1900, 1, 1);
	DateLocked.SetDate(1900, 1, 1);
	Locked = false;
	Access = 0;
	LastLogin.SetDate(1900, 1, 1);
	Department.Empty();
	Title.Empty();
	Phone.Empty();
	Location.Empty();
	EmployeeNumber.Empty();
	DefaultCompany.Empty();
	bstrValidCompanies = _T("");
}
