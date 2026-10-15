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
 *
 *	Created:	11/11/2004
 *	Author:		Sonny Barber and Peter Ringering
 *
 *	Purpose:	The main interface between a control and the server side.

 ******************************************************************************/
#pragma once
namespace CMSGBL
{
	interface ICMSControlEx : public ICMSDataMapEx
	{
	public:
		virtual void SetText(const CString& szData) = 0;
		virtual CString GetText(void) const = 0;

		virtual BOOL EnableWindow(BOOL bEnable = TRUE) = 0;
		virtual BOOL ShowWindow(int nCmdShow) = 0;

		virtual BOOL IsKindOf(const CRuntimeClass* pClass) const = 0;

		//to allow for safe casting. always call this first, then cast the return value to the proper type.
		//DO NOT directly try to cast a ICMSControlEx pointer.
		//always use IsKindOf before cast to determine if cast is safe.
		virtual void* GetNativePtr(void) = 0;

		//GetResTypeNot in template.
		virtual enuResourceType GetResType() const = 0;	//Read only.

		virtual CWnd* GetWnd() = 0;
		virtual bool IsValidClientData() = 0;

		virtual bool OnServerValFail(stuErrorReturn* pErrorReturn) = 0;
		virtual int GetMaxLen() const = 0;

		virtual UINT GetWndLabel() const = 0;
		virtual CWnd* GetWndLabelPtr() = 0;
		virtual void SetWndLabel(UINT uiLabel, UINT uiLabel2 = 0) = 0;
		virtual void SetWndLabel(CWnd* pLabel, CWnd* pLabel2 = NULL) = 0;
		virtual CString GetCaption() const = 0;
		virtual void SetCaption(UINT uiString) = 0;
		virtual void SetCaption(const CString& szString) = 0;
		virtual int GetLayoutOffset(int nSide) const = 0;
		virtual bool GetAnchorLabel() const = 0;
		virtual void SetAnchorLabel(bool bValue) = 0;

		//sb - 18-Nov-05 - these operators allow us to just move the contents of one side to the other side.
		//purposely did NOT overload operator= because we are NOT setting one side equal to the other,
		//only moving the contents. as such, overloading the right and left shift operators is syntactically
		//more appropriate as it is in the spirit of what the native operators do.

		//<< pushes the rhs contents into the lhs object. << may be chained.
		//>> pushes the lhs contents into the rhs object. >> may be chained.
		__declspec(dllexport) friend const CString& operator<< (ICMSControlEx& objLHS, const CString& szRHS) { objLHS.SetText(szRHS); return szRHS; }
		__declspec(dllexport) friend const ICMSControlEx& operator >> (const ICMSControlEx& objLHS, CString& szRHS) { szRHS = objLHS.GetText(); return objLHS; }
		__declspec(dllexport) friend const ICMSControlEx& operator<< (ICMSControlEx& objLHS, const ICMSControlEx& objRHS) { objLHS.SetText(objRHS.GetText()); return objRHS; }
		__declspec(dllexport) friend const ICMSControlEx& operator >> (const ICMSControlEx& objLHS, ICMSControlEx& objRHS) { objRHS.SetText(objLHS.GetText()); return objLHS; }

	protected:
		//InitStyle Not in template.
		virtual void InitStyle() = 0;
	};

	template <typename T>
	class CCMSControlEx : public T, virtual public CMSGBL::ICMSControlEx
	{
	public:
		CCMSControlEx();
		~CCMSControlEx();

	protected:
		CString m_szDefaultValue;
		bool m_bStyleInit;
		DWORD m_dwCMSStyle;
		ICMSParentBase* m_pParent;
		bool m_bTrimDBValue;
		UINT m_uiLabel;
		bool m_bAnchorLabel;

	public: //Methods
		virtual BOOL IsKindOf(const CRuntimeClass* pClass) const;

		//to allow for safe casting. always call this first, then cast the return value to the proper type.
		//DO NOT directly try to cast a CCMSControlEx pointer.
		//always use IsKindOf before cast to determine if cast is safe.
		virtual void* GetNativePtr(void);
		virtual void SetDBValue(const CString& szValue);
		virtual CString GetDBValue(void) const;
		virtual void ClearValue(const CString& szDefaultValue = _T(""));
		virtual bool Changed(void) const { return true; }

		virtual const CString& GetDefaultValue() const;
		virtual void SetDefaultValue(const CString& szValue);

		virtual void SetText(const CString& szData);
		virtual void SetText(UINT uiString);
		virtual CString GetText(void) const;
		virtual BOOL EnableWindow(BOOL bEnable = TRUE);
		virtual BOOL ShowWindow(int nCmdShow);
		virtual CWnd* GetWnd();
		virtual enuICMSDataMapTypes GetDMType() const;
		virtual DWORD GetCMSStyle() const;
		virtual void ModifyCMSStyle(DWORD dwAdd = 0, DWORD dwRemove = 0);
		virtual bool IsValidClientData();
		virtual ICMSParentBase* GetParentBase();
		virtual void SetParentBase(ICMSParentBase* pParent);
		virtual bool OnServerValFail(stuErrorReturn* pErrorReturn);

		virtual bool GetTrimDBValue() const;
		virtual void SetTrimDBValue(bool bValue);
		virtual int GetMaxLen() const;

		virtual CWnd* GetWndLabelPtr();
		virtual UINT GetWndLabel() const;
		virtual void SetWndLabel(UINT uiLabel, UINT uiLabel2 = 0);
		virtual void SetWndLabel(CWnd* pLabel, CWnd* pLabel2 = NULL);
		virtual CString GetCaption() const;
		virtual void SetCaption(UINT uiString);
		virtual void SetCaption(const CString& szString);
		virtual int GetLayoutOffset(int nSide) const;
		virtual bool GetAnchorLabel() const;
		virtual void SetAnchorLabel(bool bValue);

		virtual void ForceFocus(void) const;

	protected:
		virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	};

	//------------------------------------------------------------------------------
	template <typename T>
	CCMSControlEx<T>::CCMSControlEx()
		: m_szDefaultValue(_T(""))
		, m_bStyleInit(false)
		, m_dwCMSStyle(0)
		, m_pParent(NULL)
		, m_bTrimDBValue(true)
		, m_uiLabel(0)
		, m_bAnchorLabel(true)
	{}

	//------------------------------------------------------------------------------
	template <typename T>
	inline CCMSControlEx<T>::~CCMSControlEx()
	{
		//Nothing right now.
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline BOOL CCMSControlEx<T>::IsKindOf(const CRuntimeClass* pClass) const
	{
		return T::IsKindOf(pClass);
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline void* CCMSControlEx<T>::GetNativePtr(void)
	{
		return reinterpret_cast<void*>(this);
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSControlEx<T>::SetDBValue(const CString& szData)
	{
		CString szDBValue = szData;
		if (m_bTrimDBValue)
			szDBValue = szDBValue.Trim();

		bool bChanged = (szDBValue != GetDBValue());

		SetText(szDBValue);

		if (bChanged && m_pParent)
			m_pParent->DBValueChanged(*this);
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline CString CCMSControlEx<T>::GetDBValue(void) const
	{
		CString szDBValue = GetText();
		if (m_bTrimDBValue)
			szDBValue = szDBValue.Trim();

		return szDBValue;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline BOOL CCMSControlEx<T>::EnableWindow(BOOL bEnable)
	{
		return T::EnableWindow(bEnable);
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline BOOL CCMSControlEx<T>::ShowWindow(int nCmdShow)
	{
		return T::ShowWindow(nCmdShow);
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSControlEx<T>::SetText(const CString& szData)
	{
		T::SetWindowText(szData);
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSControlEx<T>::SetText(UINT uiString)
	{
		T::SetWindowText(CGBLResources::GetResourceString(uiString));
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline CString CCMSControlEx<T>::GetText(void) const
	{
		CString szReturn = _T("");
		T::GetWindowText(szReturn);
		return szReturn;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSControlEx<T>::ClearValue(const CString& szDefaultValue/* = _T("")*/)
	{
		if (szDefaultValue == _T(""))
			SetDBValue(m_szDefaultValue);
		else
			SetDBValue(szDefaultValue);
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline const CString& CCMSControlEx<T>::GetDefaultValue() const
	{
		return m_szDefaultValue;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSControlEx<T>::SetDefaultValue(const CString& szValue)
	{
		m_szDefaultValue = szValue;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline LRESULT CCMSControlEx<T>::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (!m_bStyleInit && GetSafeHwnd())
		{
			m_bStyleInit = true;
			InitStyle();
		}
		return T::WindowProc(message, wParam, lParam);
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline CWnd* CCMSControlEx<T>::GetWnd()
	{
		return this;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline enuICMSDataMapTypes CCMSControlEx<T>::GetDMType() const
	{
		return IDM_ControlEx;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline DWORD CCMSControlEx<T>::GetCMSStyle() const
	{
		return m_dwCMSStyle;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSControlEx<T>::ModifyCMSStyle(DWORD dwAdd/* = 0*/, DWORD dwRemove/* = 0*/)
	{
		m_dwCMSStyle = (m_dwCMSStyle & ~dwRemove) | dwAdd;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline bool CCMSControlEx<T>::IsValidClientData()
	{
		//Assume true.  CCMSEditBase can override this.
		return true;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline ICMSParentBase* CCMSControlEx<T>::GetParentBase()
	{
		return m_pParent;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSControlEx<T>::SetParentBase(ICMSParentBase* pParent)
	{
		m_pParent = pParent;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline bool CCMSControlEx<T>::OnServerValFail(stuErrorReturn* pErrorReturn)
	{
		m_pParent->ValFailSetFocus();
		T::SetFocus();
		return true;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline bool CCMSControlEx<T>::GetTrimDBValue() const
	{
		return m_bTrimDBValue;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSControlEx<T>::SetTrimDBValue(bool bValue)
	{
		m_bTrimDBValue = bValue;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline int CCMSControlEx<T>::GetMaxLen() const
	{
		//Default is 0 (whatever is in the control)
		return 0;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline UINT CCMSControlEx<T>::GetWndLabel() const
	{
		return m_uiLabel;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline CWnd* CCMSControlEx<T>::GetWndLabelPtr()
	{
		//There has to be a valid label control here.
		//Make sure you run DDX_CMSControl or SetWndLabel before calling this method.
		ASSERT(m_uiLabel);

		CWnd* pWnd = GetWnd();
		//This has to be a valid window.
		ASSERT_VALID(pWnd);
		return pWnd->GetParent()->GetDlgItem((int)m_uiLabel);
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSControlEx<T>::SetWndLabel(UINT uiLabel, UINT uiLabel2/* = 0*/)
	{
		UNREFERENCED_PARAMETER(uiLabel2);
		m_uiLabel = uiLabel;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSControlEx<T>::SetWndLabel(CWnd* pLabel, CWnd* pLabel2/* = NULL*/)
	{
		UNREFERENCED_PARAMETER(pLabel2);
		ASSERT_VALID(pLabel);
		m_uiLabel = (UINT)pLabel->GetDlgCtrlID();
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline CString CCMSControlEx<T>::GetCaption() const
	{
		const CWnd* const pLabel = const_cast<CCMSControlEx<T> *>(this)->GetWndLabelPtr();
		//This has to be valid.
		ASSERT_VALID(pLabel);
		CString szReturn = _T("");
		pLabel->GetWindowText(szReturn);
		return szReturn;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSControlEx<T>::SetCaption(UINT uiString)
	{
		SetCaption(CGBLResources::GetResourceString(uiString));
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSControlEx<T>::SetCaption(const CString& szString)
	{
		CWnd* pLabel = GetWndLabelPtr();
		//This has to be valid.
		ASSERT_VALID(pLabel);
		pLabel->SetWindowText(szString);
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline int CCMSControlEx<T>::GetLayoutOffset(int nSide) const
	{
		return 0;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline bool CCMSControlEx<T>::GetAnchorLabel() const
	{
		return m_bAnchorLabel;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSControlEx<T>::SetAnchorLabel(bool bValue)
	{
		m_bAnchorLabel = bValue;
	}

	//------------------------------------------------------------------------------
	template <typename T>
	inline void CCMSControlEx<T>::ForceFocus(void) const
	{
		CCMSControlEx<T>* const pSelf = const_cast<CCMSControlEx<T> *>(this);
		CWnd* const pParent = pSelf->GetParent();
		if (pParent) pParent->PostMessage(WM_NEXTDLGCTL, (WPARAM)GetSafeHwnd(), (LPARAM)TRUE);
	}
}
