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

#include "..\cmsdll\CMSMessageTypes.h"
#include "..\cmsdll\CMSPublisherDelegate.h"

class __declspec(dllexport) CCMSPublisher : public CMS::IPublisher
{
public:
	CCMSPublisher(void) : m_objPublisher() {};
	virtual ~CCMSPublisher(void) {};

	virtual void Subscribe(CMS::ISubscriber* pSubscriber) { m_objPublisher.Subscribe(pSubscriber); }
	virtual void Unsubscribe(CMS::ISubscriber* pSubscriber) { m_objPublisher.Unsubscribe(pSubscriber); }
	virtual void Publish(CCMSMessage& objMessage) { m_objPublisher.Notify(objMessage); }

protected:
	mutable CCMSPublisherDelegate m_objPublisher;
};

template <typename T>
struct tagPublisherT : public T
{
	typedef T		traits_type;

	tagPublisherT() : m_objPublisher() {};

	virtual void Publish(CCMSMessage& objMessage) { m_objPublisher.Notify(objMessage); }

	mutable CCMSPublisherDelegate m_objPublisher;
};

template <typename T>
struct tagPublisherT2 : public tagPublisherT<T>, public CMS::IPublisher
{
	typedef T		traits_type;

	tagPublisherT2() : tagPublisherT<T>() {};

	virtual void Subscribe(CMS::ISubscriber* pSubscriber) { m_objPublisher.Subscribe(pSubscriber); }
	virtual void Unsubscribe(CMS::ISubscriber* pSubscriber) { m_objPublisher.Unsubscribe(pSubscriber); }
};

template <typename T, typename TRAITS = tagPublisherT2<T> >
struct tagWndProc
{
	typedef typename TRAITS		  traits_type;
	typedef typename traits_type* traits_type_ptr;

	LRESULT WindowProc(typename traits_type_ptr pWnd, UINT message, WPARAM wParam, LPARAM lParam);

	CCMSWindowMessage	m_objMsg;
};

template <typename T, typename TRAITS = tagWndProc<T> >
class CCMSWndProcPublisher : public tagPublisherT<T>
{
public:
	typedef T		traits_type;
	typedef TRAITS	traits;

	CCMSWndProcPublisher(void) : m_objTraits() {};
	virtual ~CCMSWndProcPublisher(void) {};

	virtual void Subscribe(CMS::ISubscriber* pSubscriber) { m_objPublisher.Subscribe(pSubscriber); }
	virtual void Unsubscribe(CMS::ISubscriber* pSubscriber) { m_objPublisher.Unsubscribe(pSubscriber); }

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

private:
	traits	m_objTraits;
};

template <typename T, typename TRAITS = tagWndProc<T> >
class CCMSWndProcPublisher2 : public tagPublisherT2<T>
{
public:
	typedef T		traits_type;
	typedef TRAITS	traits;

	CCMSWndProcPublisher2(void) : m_objTraits() {};
	virtual ~CCMSWndProcPublisher2(void) {};

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

private:
	traits	m_objTraits;
};

//------------------------------------------------------------------------------
template <typename T, typename TRAITS>
inline LRESULT tagWndProc<T, TRAITS>::WindowProc(typename tagWndProc<T, TRAITS>::traits_type_ptr pWnd
	, UINT message
	, WPARAM wParam
	, LPARAM lParam)
{
	m_objMsg.SetOuter(pWnd);
	m_objMsg.Init(message, wParam, lParam);

	pWnd->Publish(m_objMsg);

	if (m_objMsg.GetCancel())
		return (LRESULT)1;
	else
		return (LRESULT)0;
}

//------------------------------------------------------------------------------
template <typename T, typename TRAITS>
inline LRESULT CCMSWndProcPublisher<T, TRAITS>::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT	lr = m_objTraits.WindowProc(this, message, wParam, lParam);
	if (lr == 0) lr = traits_type::WindowProc(message, wParam, lParam);
	return lr;
}

//------------------------------------------------------------------------------
template <typename T, typename TRAITS>
inline LRESULT CCMSWndProcPublisher2<T, TRAITS>::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT	lr = m_objTraits.WindowProc(this, message, wParam, lParam);
	if (lr == 0) lr = traits_type::WindowProc(message, wParam, lParam);
	return lr;
}
