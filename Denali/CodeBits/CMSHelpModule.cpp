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
#include <stdexcept>
#include <htmlhelp.h>
#include "..\cmsdll\CMSHelpModule.h"
#include "..\cmsdll\GBLNetDll.h"

extern CGBLSystemInformation * g_pGBLSystemInformationCMSDll;

std::auto_ptr<CCMSHelpLauncher>	CCMSHelpModule::m_pInstance;

//------------------------------------------------------------------------------
CCMSHelpLauncher::~CCMSHelpLauncher(void)
{
}

//------------------------------------------------------------------------------
CCMSHelpLauncher * CCMSHelpLauncher::New(void)
{
	CCMSHelpLauncher * const pLauncher = new CCMSHelpLauncher();
	ASSERT(pLauncher);
	return pLauncher;
}

//------------------------------------------------------------------------------
CCMSHelpLauncher::CCMSHelpLauncher(void)
{

}

//------------------------------------------------------------------------------
bool CCMSHelpLauncher::Display(const CString& szHelpFile, UINT nCmd, DWORD_PTR dwData)
{
	Log(szHelpFile, nCmd, dwData);
	_bstr_t szHelp(szHelpFile);
	CGBLNetDll::ShowHelp(L"", (int)nCmd, szHelp, L"");
	return true;
}

//---------------------------------------------------------------------------
bool CCMSHelpLauncher::ProcessMsg(LPMSG lpMsg)
{
	Load();
	//	const bool bolStatus =   // begbert 02-15-2006 VS8: Not referenced.
	::HtmlHelp(NULL, NULL, HH_PRETRANSLATEMESSAGE, (DWORD_PTR)lpMsg) ? true : false;
	return false;
}

//------------------------------------------------------------------------------
void CCMSHelpLauncher::Load(void)
{
	if (m_dwLoadingThreadID) return;
	::HtmlHelp(NULL, NULL, HH_INITIALIZE, (DWORD_PTR)&m_dwCookie);
	m_dwLoadingThreadID = ::GetCurrentThreadId();
}

//------------------------------------------------------------------------------
void CCMSHelpLauncher::Unload(void)
{
	::HtmlHelp(NULL, NULL, HH_CLOSE_ALL, NULL);
	::HtmlHelp(NULL, NULL, HH_UNINITIALIZE, m_dwCookie);
}

//------------------------------------------------------------------------------
void CCMSHelpLauncher::Log(const CString& szHelpFile, UINT nCmd, DWORD_PTR dwData) const
{
	//PTR.07.25.2005 - Event Log
	CXMLDocument docXMLCommand(CMSStrings::XMLTags::Root);
	CXMLElement eleRoot = docXMLCommand.GetDocumentElement();
	eleRoot.AddNewChild(_T("HELPFILE"), szHelpFile);
	eleRoot.AddNewChild(_T("HELPURL"), _T(" "));
	CGBLEventLogOptions clsOptions;
	clsOptions.m_eCategory = EventLogCategory_Help;
	clsOptions.m_eIconType = EventICO_Info;
	CGBLEventLog::WriteLogXML(EventLog_HelpLaunching, _T("Help Launch"), &clsOptions, &docXMLCommand);
}

//------------------------------------------------------------------------------
CCMSHelpLauncher * CCMSHelpModule::Instance(void)
{
	if (!m_pInstance.get()) m_pInstance.reset(CCMSHelpLauncher::New());
	return m_pInstance.get();
}
