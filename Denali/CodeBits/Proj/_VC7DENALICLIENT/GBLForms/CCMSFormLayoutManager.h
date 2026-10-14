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
 *******************************************************************************
 * Initial code - sbarber
 ******************************************************************************/
#pragma once

#include <memory>
#include <map>
#include <utility>

class CCMSFormView;
class CCMSMessage;

class __declspec(dllexport) CCMSFormLayoutManager
{
	class CCMSFormLayout;

public:
	static CCMSFormLayout * Instance(void);

protected:

private:
	class CCMSFormKey
	{
	public:
		CCMSFormKey(void);
		CCMSFormKey(const CCMSFormKey& rhs);
		~CCMSFormKey(void);

		CCMSFormKey& operator= (const CCMSFormKey& rhs);

		bool operator== (const CCMSFormKey& rhs) const;
		bool operator< (const CCMSFormKey& rhs) const;
		bool operator!= (const CCMSFormKey& rhs) const;

	protected:
	private:
		COleDateTime	m_dt;
		DWORD			m_dwTicks;
	};

	typedef	std::pair<CWnd *, bool>								ManagedForm;
	typedef	std::map<CCMSFormKey, ManagedForm>	FormMap;

	class CCMSFormLayout : protected FormMap, protected CCMSSubscriber<CCMSFormLayout>
	{
		friend class CMS::IPublisher;
		friend class CCMSFormLayoutManager;

	public:
		CCMSFormLayout(void);
		virtual ~CCMSFormLayout(void);

		template<typename T>
		void Add(T * pWnd);

	protected:
		static CCMSFormLayout * New(void);
		virtual void Notify(CCMSMessage& objMsg);
		virtual void Notify(void) { ASSERT(FALSE); }

		template<typename T>
		void Subscribe(T * pWnd) { pWnd->Subscribe(this); }

		void MoveWindow(CWnd * pWnd);

	private:
		struct FindWindow
		{
			FindWindow(CWnd * pWnd) : m_pWnd(pWnd) {};
			bool operator() (const FormMap::value_type& value) { return value.second.first == m_pWnd; }
			const CWnd * const m_pWnd;
		};

	private:
		CCMSFormLayout& operator= (const CCMSFormLayout&) { return *this; }
	};


	static std::auto_ptr<CCMSFormLayout>	m_pInstance;

private:
	CCMSFormLayoutManager(void) {};
	~CCMSFormLayoutManager(void) {};
	CCMSFormLayoutManager& operator= (const CCMSFormLayoutManager&) { return *this; }
};

//------------------------------------------------------------------------------
template<typename T>
void CCMSFormLayoutManager::CCMSFormLayout::Add(T * pWnd)
{
	const size_type	nSize = size();
	const_iterator		it = std::find_if(begin(), end(), FindWindow(pWnd));

	if (it == end())
	{
		(*this)[CCMSFormKey()] = std::make_pair(pWnd, nSize == 0 ? true : false);
		Subscribe(pWnd);
	}
}
