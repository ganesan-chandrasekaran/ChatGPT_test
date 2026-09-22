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
 // stdafx.h : include file for standard system include files,
 // or project specific include files that are used frequently,
 // but are changed infrequently

#pragma once

#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

//Peter Ringering - 12/29/2004 - Having to disable warning 4702 due to error in MFC.  Only happens in release mode.
/*From http://www.dotnet247.com/247reference/msgs/1/7775.aspx
  Posted October 2001 in microsoft.public.dotnet.languages.vc.
"What about the W4702 warnings I am getting in xtree when I disable
browsing information?"
--------------------------------------------------------------------------------------
"I'm afraid that one slipped through the cracks. It should have been fixed
in Beta2 but wasn't. It doesn't look like it will be fixed in the shipping
product either...You could just disable C4702 around the STL headers and wherever else you
see the problem." - Jason Shirk - VC++ Compiler Team (MPG.16193c33d28d720d9896f6@msnews.microsoft.com)
*/
#ifndef _DEBUG
#pragma warning(disable:4702)
#endif

#include "..\cmsinclude\winver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include "..\CMSDll\ModuleSettings.h"
#include "..\CMSDll\GBLSystemInformation.h"

#include "../GBLForms/GBLHeaders.h"