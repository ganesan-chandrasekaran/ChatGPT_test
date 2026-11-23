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
 *  WAPNING:  Unauthorized reproduction of this program as well as
 *  unauthorized preparation of derivative works based upon the program or
 *  distribution of copies by sale, rental, lease or lending are violations
 *  of federal copyright laws and state trade secret laws, punishable by
 *  civil and criminal penalties.
 *
 ******************************************************************************
 *	Date:		9/10/2012
 *	Author:		InTimeTec: Kapil Paliwal/Hitesh Sharma
 ******************************************************************************/
#pragma once
#include "MP_GRD_CustomOptions.h"
 // InTimeTec: 12.20.2012.HS - Custom Menus
#include "MP_GRD_CustomMenu.h"
// InTimeTec: End


class CMP_CFV_ModPrefs;
class AFX_EXT_CLASS CMP_PPG_CustomOptions : public CCMSPropertyPageEx, public ICMSGridEntryBL
{
	DECLARE_DYNAMIC(CMP_PPG_CustomOptions)

	//Constructor, Startup
public:
	CMP_PPG_CustomOptions(CMP_CFV_ModPrefs& clsForm);
	bool m_bInitCustomReports;
	int newVar;
	virtual ~CMP_PPG_CustomOptions();

	//Form Resource
	enum { IDD = IDD_GBL_MODPREFS_CUSTOMOPTIONS_STD };


	//MFC Overrides, Messages
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()

	//CMS overrides
protected:
	virtual void LoadResources();
	virtual void SetupLayoutManager();
	virtual void GetFormBounds(CWnd*& pRightBound, CWnd*& pBottomBound);
	virtual void OnAddFieldMaps(CCMSMaintDoc& clsDoc);
	virtual void SetupGridProps(CCMSGridEntry& clsGrid, CString& szLineTypeField/* = ""*/);
	virtual void AddGridColumns(CCMSGridEntry& clsGrid);
	virtual void SetupColumn(int nColumnID, CCMSGridControlInfo& clsControlInfo, CCMSGridEntry& clsGrid);
	virtual void PostSetupGridProps(CCMSGridEntry& clsGrid);
	virtual void Notify(CCMSMessage& objMessage);
	virtual CCMSLTBase* CreateNewRow(long lRow, CString& szLineType, CCMSGridEntry& clsGrid);
	virtual ICMSGridEntryBLSorting* GetSorting() { return NULL; }
	virtual BOOL OnSetActive(void);
	void 	LoadCustomReportsGrid(void);
	// InTimeTec: 12.20.2012.HS - Custom Menus
	void 	LoadCustomMenusGrid(void);
	// InTimeTec: End
	virtual void OnEvent(CCMSSubject* pSubject, UINT uiEvent);
	// Changing the way grid is attached to the form. This is in order to fix scroll bar issues
	virtual void InitControls();
	void SetCustomReportsGrid();
	void SetCustomMenusGrid();

	//Business Logic
public:
	CString GetModuleStringID();
	void	SetModifiedFlagFromGrid(BOOL bModified);
	//Added for handling invalid file path
	bool IsValid();

	//Form Controls
protected:
	CMP_CFV_ModPrefs&	m_clsForm;
	// Changing the way grid is attached to the form. This is in order to fix scroll bar issues
	std::auto_ptr<CGRD_CustomOptionsEx>	m_grdGrid;
	std::auto_ptr<CGRD_CustomMenuEx>	m_grdGridMenu;
	std::auto_ptr<CCMSGridEntry>	m_pGrid;

	//Local Member Variables
private:
	bool	m_bInit;
};
