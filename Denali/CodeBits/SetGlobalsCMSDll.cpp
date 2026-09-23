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
#include "SetGlobalsCMSDll.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//Global Variables KPM, 10/23/2002
CGBLSystemInformation* g_pGBLSystemInformationCMSDll = NULL;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSetGlobalsCMSDll::CSetGlobalsCMSDll()
{

}

CSetGlobalsCMSDll::CSetGlobalsCMSDll(CGBLSystemInformation* pGBLSystemInformation)
{
	this->SetSystemInformation(pGBLSystemInformation);
}

CSetGlobalsCMSDll::~CSetGlobalsCMSDll()
{

}

void CSetGlobalsCMSDll::SetSystemInformation(CGBLSystemInformation* pGBLSystemInformation)
{
	g_pGBLSystemInformationCMSDll = pGBLSystemInformation;
}
