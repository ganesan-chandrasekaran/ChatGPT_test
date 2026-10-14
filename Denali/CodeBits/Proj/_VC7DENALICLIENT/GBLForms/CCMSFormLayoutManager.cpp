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
#include "..\cmsdll\CMSMessageTypes.h"
#include "..\gblforms\gblsi.h"
#include "..\gblforms\cmsformlayoutmanager.h"

std::auto_ptr<CCMSFormLayoutManager::CCMSFormLayout>	CCMSFormLayoutManager::m_pInstance;

//------------------------------------------------------------------------------
CCMSFormLayoutManager::CCMSFormLayout * CCMSFormLayoutManager::Instance(void)
{
	if (!m_pInstance.get()) m_pInstance.reset(CCMSFormLayout::New());
	return m_pInstance.get();
}

//------------------------------------------------------------------------------
CCMSFormLayoutManager::CCMSFormLayout::CCMSFormLayout(void)
{
}

//------------------------------------------------------------------------------
CCMSFormLayoutManager::CCMSFormLayout::~CCMSFormLayout(void)
{
}

//------------------------------------------------------------------------------
CCMSFormLayoutManager::CCMSFormKey::CCMSFormKey(void)
	: m_dt(COleDateTime::GetCurrentTime())
	, m_dwTicks(::GetTickCount())
{
}

//------------------------------------------------------------------------------
CCMSFormLayoutManager::CCMSFormKey::CCMSFormKey(const CCMSFormKey& rhs)
	: m_dt(rhs.m_dt)
	, m_dwTicks(rhs.m_dwTicks)
{
}

//------------------------------------------------------------------------------
CCMSFormLayoutManager::CCMSFormKey::~CCMSFormKey(void)
{
}

//------------------------------------------------------------------------------
CCMSFormLayoutManager::CCMSFormKey& CCMSFormLayoutManager::CCMSFormKey::operator= (const CCMSFormLayoutManager::CCMSFormKey& rhs)
{
	if (this != &rhs)
	{
		m_dt = rhs.m_dt;
		m_dwTicks = rhs.m_dwTicks;
	}

	return *this;
}

//------------------------------------------------------------------------------
bool CCMSFormLayoutManager::CCMSFormKey::operator== (const CCMSFormLayoutManager::CCMSFormKey& rhs) const
{
	return m_dt == rhs.m_dt && m_dwTicks == rhs.m_dwTicks;
}

//------------------------------------------------------------------------------
bool CCMSFormLayoutManager::CCMSFormKey::operator< (const CCMSFormLayoutManager::CCMSFormKey& rhs) const
{
	return (m_dt < rhs.m_dt || (m_dt == rhs.m_dt && m_dwTicks < rhs.m_dwTicks)) ? true : false;
}

//------------------------------------------------------------------------------
bool CCMSFormLayoutManager::CCMSFormKey::operator!= (const CCMSFormLayoutManager::CCMSFormKey& rhs) const
{
	return !operator== (rhs);
}

//------------------------------------------------------------------------------
CCMSFormLayoutManager::CCMSFormLayout * CCMSFormLayoutManager::CCMSFormLayout::New(void)
{
	return new CCMSFormLayout();
}

//------------------------------------------------------------------------------
void CCMSFormLayoutManager::CCMSFormLayout::Notify(CCMSMessage& objMsg)
{
	CCMSFocusMessage * const pMessage = DYNAMIC_DOWNCAST(CCMSFocusMessage, &objMsg);
	if (!pMessage || !pMessage->GetForm()) return;

	CWnd * const	pWnd = pMessage->GetForm();

	switch (pMessage->GetStatus())
	{
	case CCMSFocusMessage::Dying:
	{
		iterator	it = std::find_if(begin(), end(), FindWindow(pWnd));
		if (it != end()) erase(it);
	}
	break;

	case CCMSFocusMessage::GotFocus:
		MoveWindow(pWnd);
		break;
	};
}

//------------------------------------------------------------------------------
void CCMSFormLayoutManager::CCMSFormLayout::MoveWindow(CWnd * pWnd)
{
	static const int nOffset = 40;
	iterator				it = std::find_if(begin(), end(), FindWindow(pWnd));
	if (it == end() || it->second.second) return;		//already been moved

	it->second.second = true;
	CWnd * const pPreviousWnd = (--it)->second.first;

	//only cascade the windows when the new window is the same as the previous window
	const int nCurrentID = pWnd->GetDlgCtrlID();
	const CString szCurrentID((LPCTSTR)pWnd->SendMessage(WM_GETSUBFORM));

	const int nPreviousID = pPreviousWnd->GetDlgCtrlID();
	const CString szPreviousID((LPCTSTR)pPreviousWnd->SendMessage(WM_GETSUBFORM));

	if (nCurrentID != nPreviousID || szCurrentID != szPreviousID) return;

	CRect	rectPrev;
	pPreviousWnd->GetParent()->GetWindowRect(&rectPrev);
	pPreviousWnd->GetParent()->ScreenToClient(&rectPrev);

	rectPrev.left += (nOffset / 2);
	rectPrev.right += (nOffset / 2);
	rectPrev.top -= nOffset;
	rectPrev.bottom -= nOffset;

	pPreviousWnd->GetParent()->ClientToScreen(&rectPrev);

	CRect	rectCurr;
	pWnd->GetParent()->GetWindowRect(&rectCurr);
	pWnd->GetParent()->ScreenToClient(&rectCurr);

	const long nCurrRight = rectPrev.left + rectCurr.Width();
	const long nCurrBottom = rectPrev.top + rectCurr.Height();

	CRect	rectApp;
	CMSGBL::GetSI()->ApplicationMainFrame->GetWindowRect(&rectApp);
	CMSGBL::GetSI()->ApplicationMainFrame->ScreenToClient(&rectApp);

	if (nCurrRight > rectApp.right - nOffset || nCurrBottom > rectApp.bottom - nOffset)
	{
		rectPrev = rectApp;
		rectPrev.top += nOffset / 2;
		rectPrev.left += nOffset / 2;
		CMSGBL::GetSI()->ApplicationMainFrame->ClientToScreen(&rectPrev);
	}

	pWnd->GetParent()->SetWindowPos(NULL, rectPrev.left, rectPrev.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);
}
