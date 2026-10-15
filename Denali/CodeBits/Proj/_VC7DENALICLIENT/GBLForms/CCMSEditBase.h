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
namespace CMSGBL
{
	template <typename T>
	class CCMSEditBase : public T
	{
	public:
		CCMSEditBase();
		~CCMSEditBase();

	protected:
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		virtual void InitStyle();
		virtual enuResourceType GetResType() const;

	public:
		CString GetSelectedText();
		void SetEnterToTab(bool bValue);
		bool GetEnterToTab();

	protected:
		bool m_bEnterToTab;
	};
	//------------------------------------------------------------------------------
	template <typename T>
	CCMSEditBase<T>::CCMSEditBase()
		: m_bEnterToTab(true)
	{
		//Nothing right now.
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline CCMSEditBase<T>::~CCMSEditBase()
	{
		//Nothing right now.
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline BOOL CCMSEditBase<T>::PreTranslateMessage(MSG* pMsg)
	{
		//Code Here to translate ENTER to TAB.
		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
		{
			LONG_PTR dwStyle = GetWindowLongPtr(pMsg->hwnd, GWL_STYLE);
			if (dwStyle & ES_WANTRETURN)
				return FALSE;

			SendMessage(WM_CMSCRLF);
			if (m_bEnterToTab)
				pMsg->wParam = VK_TAB;
		}
		return T::PreTranslateMessage(pMsg);
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSEditBase<T>::InitStyle()
	{
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline enuResourceType CCMSEditBase<T>::GetResType() const
	{
		return Res_Edit;
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline CString CCMSEditBase<T>::GetSelectedText()
	{
		CString szReturn = CGBLForm::GetWindowText(this);
		int nStart(0), nEnd(0);
		CEdit::GetSel(nStart, nEnd);
		if (nStart >= 0)
		{
			if (nEnd >= 0)
				szReturn = szReturn.Mid(nStart, nEnd - nStart);
			else
				szReturn = szReturn.Mid(nStart);
		}
		return szReturn;
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSEditBase<T>::SetEnterToTab(bool bValue)
	{
		m_bEnterToTab = bValue;
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline bool CCMSEditBase<T>::GetEnterToTab()
	{
		return m_bEnterToTab;
	}

	template <typename T>
	class CCMSStringEditBase : public CCMSEditBase<CCMSControlEx<T> >
	{
	public:
		CCMSStringEditBase();
		~CCMSStringEditBase();

	protected:
		CString m_szNullValMsg;
		CString m_szNullValCaption;
		int m_nDisplayLen;
		bool m_bValMsgAddModule;

	public:
		virtual bool IsValidClientData();

		bool GetAllowNulls() const;
		const CString& GetNullValMsg() const;
		void SetNullValMsg(const CString& szValue);
		void SetNullValMsg(UINT uiStringRes);
		const CString& GetNullValCaption() const;
		void SetNullValCaption(const CString& szValue);
		void SetNullValCaption(UINT uiStringRes);
		void SetNullValAddModule(bool bValue);
		bool GetNullValAddModule();

		void SetControlProperties(int nMaxLen, int nDisplayLen);
		void SetValidationProperties(UINT uiNullValMsg, UINT uiNullValCaption, bool bAddModule = true);
		void SetValidationProperties(const CString& szNullValMsg, const CString& szNullValCaption, bool bAddModule = true);
		virtual int GetMaxLen() const;
	};

	//------------------------------------------------------------------------------
	template <typename T>
	inline CCMSStringEditBase<T>::CCMSStringEditBase()
		: m_szNullValMsg(_T(""))
		, m_szNullValCaption(_T(""))
		, m_nDisplayLen(0)
		, m_bValMsgAddModule(true)
	{
		//Nothing right now.
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline CCMSStringEditBase<T>::~CCMSStringEditBase()
	{
		//Nothing right now.
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline bool CCMSStringEditBase<T>::GetAllowNulls() const
	{
		return (m_szNullValMsg == _T(""));
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline const CString& CCMSStringEditBase<T>::GetNullValMsg() const
	{
		return m_szNullValMsg;
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSStringEditBase<T>::SetNullValMsg(const CString& szValue)
	{
		m_szNullValMsg = szValue;
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSStringEditBase<T>::SetNullValMsg(UINT uiStringRes)
	{
		SetNullValMsg(CGBLResources::GetResourceString(uiStringRes));
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline const CString& CCMSStringEditBase<T>::GetNullValCaption() const
	{
		return m_szNullValCaption;
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSStringEditBase<T>::SetNullValCaption(const CString& szValue)
	{
		m_szNullValCaption = szValue;
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSStringEditBase<T>::SetNullValCaption(UINT uiStringRes)
	{
		SetNullValCaption(CGBLResources::GetResourceString(uiStringRes));
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSStringEditBase<T>::SetNullValAddModule(bool bValue)
	{
		m_bValMsgAddModule = bValue;
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline bool CCMSStringEditBase<T>::GetNullValAddModule()
	{
		return m_bValMsgAddModule;
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline bool CCMSStringEditBase<T>::IsValidClientData()
	{
		if (!GetAllowNulls())
		{
			CString szText = GetText().Trim();
			if (szText == _T(""))
			{
				if (m_pParent)
					m_pParent->ValFailSetFocus();
				if (m_bValMsgAddModule)
					CGBLForm::ShowValidationError(GetParent(), m_szNullValMsg, m_szNullValCaption);
				else
					CGBLForm::ShowValidationErrorNM(GetParent(), m_szNullValMsg, m_szNullValCaption);
				SetFocus();
				return false;
			}
		}
		return true;
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSStringEditBase<T>::SetValidationProperties(UINT uiNullValMsg, const UINT uiNullValCaption, bool bAddModule/* = true*/)
	{
		SetValidationProperties(CGBLResources::GetResourceString(uiNullValMsg), CGBLResources::GetResourceString(uiNullValCaption), bAddModule);
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSStringEditBase<T>::SetValidationProperties(const CString& szNullValMsg, const CString& szNullValCaption, bool bAddModule/* = true*/)
	{
		m_szNullValMsg = szNullValMsg;
		m_szNullValCaption = szNullValCaption;
		m_bValMsgAddModule = bAddModule;
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSStringEditBase<T>::SetControlProperties(int nMaxLen, int nDisplayLen)
	{
		T::SetLimitText((UINT)nMaxLen);
		m_nDisplayLen = nDisplayLen;
	}
	//------------------------------------------------------------------------------
	template <typename T>
	inline int CCMSStringEditBase<T>::GetMaxLen() const
	{
		return m_nDisplayLen;
	}
}
