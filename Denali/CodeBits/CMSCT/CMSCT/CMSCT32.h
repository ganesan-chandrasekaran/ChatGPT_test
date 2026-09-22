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
#if !defined(AFX_CMSCT32_H__18B18F96_0BFC_4FD3_9544_14E82A448B05__INCLUDED_)
#define AFX_CMSCT32_H__18B18F96_0BFC_4FD3_9544_14E82A448B05__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "../gblforms/SplashThread.h"		//Splash Screen for startup
#include "../gblforms/cmswinapp.h"

 /////////////////////////////////////////////////////////////////////////////
 // CCMSCT32App:
class CCMSCT32App : public CCMSWinApp
{
public:
	CCMSCT32App();
	virtual BOOL			InitInstance();
	virtual int				ExitInstance();

protected:
	void	AddSharedDocTemplates(void);


private:
	// Implementation
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CMSCT32_H__18B18F96_0BFC_4FD3_9544_14E82A448B05__INCLUDED_)
