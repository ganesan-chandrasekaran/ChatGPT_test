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
#include "StdAfx.h"
#include "GrdPurDetail.h"
#include "GrdRcvDetail.h"
#include "cmspoentry32Doc.h"
#include "../CMSPOEntry/mainfrm.h"
#include "CMSTransaction.h"
#include "LTPurDetailUnknown.h"
#include "LTPurDetailInventory.h"
#include "LTPurDetailDirExp.h"
#include "LTPurDetailComment.h"
#include "DlgCommentEditor.h"
#include "../POForms/POConstants.h"
#include "DlgVendorPurchaseHistory.h"
#include "DlgNoInfo.h"

#include "..\INForms\resource.h"
#include "..\POForms\resource.h"
#include "..\Common\LangIDs.h"


using namespace POEntryLayout::DetailItems;

//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CGrdPurDetail, CCMSGrid)
    ON_WM_SIZE()
    //	ON_WM_KEYDOWN()
    //	ON_MESSAGE(POE_WM_PAYMENT_METHOD_CHANGE, &CGrdPurDetail::OnPaymentMethodChange)
    //	ON_MESSAGE(POE_WM_REFRESH_DETAIL_GRID, &CGrdPurDetail::Refresh)
    ON_MESSAGE(POE_WM_DETAIL_LIMIT_REACHED, &CGrdPurDetail::OnDetailLimitReached)	
	ON_MESSAGE(WM_CMSGRID_LOOKUPEND, &CGrdPurDetail::OnGridLookupEnd)
END_MESSAGE_MAP()
//---------------------------------------------------------------------------
CGrdPurDetail::CGrdPurDetail(int nRow /*=0*/)
    : CPOEntryGridBase(nRow)
    , m_clsStockLookInfo(POEntryLayout::DetailItems::StockNumber)
    , m_clsGLAcctLookInfo(POEntryLayout::DetailItems::StockNumber)
	, m_clsCostCenterLookInfo(POEntryLayout::DetailItems::CostCenter)
	, m_clsINDescLookInfo(POEntryLayout::DetailItems::Description)
    //, m_clsCommentLookInfo(POEntryLayout::PUR_DetailItems::Comment)
    , m_clsMLILocLookInfo(POEntryLayout::DetailItems::MLILocation)
    , m_clsParser(si->ModuleSettings.POSettings.bAutoFormatComment, 32)
{
    m_bLaunchedMaintForm = false;	// begbert 04-09-2008 1-28813 : added this
    m_bDisableOnEditFinishHandling = false; // begbert 02-04-2009 1-31940 : added this
    m_pLookup = NULL;
    m_pTransaction = NULL;
    m_pGridAutoFill = NULL;

    m_nLookupCol = -1;
    m_lLookupRow = -1;

    m_bIsAutoFill = false;
    m_nFirstViewableCol = 1;
    m_pEntry = NULL;
    m_bDeleteRow = true;
}

//---------------------------------------------------------------------------
CGrdPurDetail::~CGrdPurDetail()
{
    delete m_pGridAutoFill;
    m_pGridAutoFill = NULL;
}

void CGrdPurDetail::SetLayoutObject(const CCMSPOEntryLayout* pLayout)
{
    if (!pLayout) return;
    //	ASSERT(si->ModuleSettings.POSettings.Module == IDS_PUR_MODULE);
    ASSERT(pLayout->IsPUR());
    m_pColLayout = &pLayout->m_clsPURDetailItems;
}

/*
//---------------------------------------------------------------------------
// get the column definitions from the array
int CGrdPurDetail::PopulateColumnArray(CString szDescription)
{
    ASSERT(m_pColLayout);
    if (!m_pColLayout) return 1;

    InitAllColumnsMap() ;
    //set prompt ids for column types.
    SetupColumnPrompts() ;

    //This function looks into column information array and adds required columns
    //into the grid's internal column map.
    DoColumnMapping() ;
    InitMappedColumns() ;

    return 0;
}
*/

//---------------------------------------------------------------------------
//Look in m_mapcolumninfo and change datatype and
//celltype properties before we call InitEmbeddedControls.
void CGrdPurDetail::InitMappedColumns()
{
    //	if (!registerprops || !si)	return;

    CString szKey;
    COLPROPS colprops;
    POSITION pos = m_mapcolumninfo.GetStartPosition();
    while (pos)
    {
        m_mapcolumninfo.GetNextAssoc(pos, szKey, colprops);
        colprops.nAlignment = UG_ALIGNLEFT;
        colprops.nCellType = UGCT_NORMAL;
        colprops.nDataType = UGCELLDATA_STRING;
        colprops.nTranslateType = GRD_XLAT_NOTHING;
        colprops.nMaxDataWidth = 50;
        colprops.szDropList = EMPTY_STRING;

        switch (FilterUDFids(colprops.nDBColType))
        {
            // dates
        case OrderedDate:
        case ExpectedDate:
            colprops.nDataType = GRD_GRIDDATEPICKER;
            colprops.nAlignment = UG_ALIGNCENTER;
            break;

            // autofills
        case StockNumber:
            colprops.nDataType = GRD_GRIDAUTOFILL;
            break;

        case MLILocation:
            if (si->ModulesInstalled.Inventory.IsUsable && si->ModuleSettings.CurrentSettings->GetINIntegration() > 0
                && si->ModulesInstalled.MultiLocationInventory.IsUsable)
            {
                colprops.nDataType = GRD_GRIDAUTOFILL;
                colprops.nMaxDataWidth = 50;
            }
            else {
                colprops.bAllowFocus = false;
                colprops.bVisible = false;
            }
            break;

		case CostCenter:
			colprops.nDataType = GRD_GRIDAUTOFILL;
            break;

        case VendorStockNum:
            //if (si->ModulesInstalled.GeneralLedger.IsUsable && si->ModuleSettings.CurrentSettings->GetGLIntegration() > 0)
            //	colprops.nDataType  = GRD_GRIDAUTOFILL ;
            break;

            // Numeric
        case QuantityOrdered:
            colprops.nDataType = UGCELLDATA_NUMBER;
            colprops.nAlignment = UG_ALIGNRIGHT;
            colprops.nDecimalDigitCount = 8;	//begbert 09-23-3008 1-31001 : reduced from 9 to 8 digits, per the spec
            colprops.nFractionalDigitCount = 4;
            colprops.bAllowNegNumbers = true;
            break;

            // Currency
        case ExtendedCost:
            colprops.nDataType = UGCELLDATA_CURRENCY;
            colprops.nAlignment = UG_ALIGNRIGHT;
            colprops.nDecimalDigitCount = 9;
            colprops.nFractionalDigitCount = CMS::GetSI()->Localization.CurrencyDecimalDigits;
            colprops.bAllowNegNumbers = true;
            break;

            // begbert 05-09-2008 1-29746 : Unit and last cost set to have 4 decimal places.
        case UnitCost:
        case LastCost:
            colprops.nDataType = UGCELLDATA_CURRENCY;
            colprops.nAlignment = UG_ALIGNRIGHT;
            colprops.nDecimalDigitCount = 9;
            colprops.nFractionalDigitCount = 4;
            colprops.bAllowNegNumbers = false;
            break;

            // Checkbox
        case IsLandItem:
            // begbert 04-18-2008 1-28906 : Don't show if Landing is disabled or set to AUTO_DISTRIBUTION.
            if (CMS::GetSI()->ModuleSettings.POSettings.nLandingMethod == POConstants::LandingMethod::AUTO_DISTRIBUTION ||
                CMS::GetSI()->ModuleSettings.POSettings.nItemsToLand == POConstants::ItemsToLand::NONE)
            {
                colprops.bAllowFocus = false;
                colprops.bVisible = false;
            }
            // otherwise fall through
        case IsApplyFreight:	// begbert 03-05-2009 1-32263 : ApplyFreight no longer needs special handling here.
        case IsTaxable:
        case IsDiscount:
            colprops.nDataType = UGCELLDATA_BOOL;
            colprops.nCellType = UGCT_CHECKBOX;
            colprops.nAlignment = UG_ALIGNCENTER;
            break;

        case Description:
            //InTimeTec: 11.14.2013.SP- In order to fix Defect # 12150 :D_PO_Server exception error when description is longer than 50 characters
			if (si->ModulesInstalled.Inventory.IsUsable && si->ModuleSettings.CurrentSettings->GetINIntegration() > 0)
            {
                colprops.nDataType = GRD_GRIDAUTOFILL;
                colprops.nMaxDataWidth = 50;
            } else
				colprops.nMaxDataWidth = 120;
            break;
            //InTimeTec: End
        case References:
            break;

            // begbert 03-22-2011 1-35267 : added Encumbrance column
        case Encumbrance:
            // Show only in Fund when encumbrance posting is turned on
            if (CMS::GetSI()->CompanySettings.IsFund &&
                CMS::GetSI()->ModuleSettings.POSettings.bPostEncumber)
                // && CGBLModuleIntegrationHelper::IsGLIntegrated() )	// begbert 05-19-2011 1-35903 : Removed this
            {
                colprops.nDataType = GRD_GRIDACCTCTRLNOFUND;
                colprops.nTranslateType = GRD_XLAT_ACCOUNTNMBR;
                InitEmbeddedControls(true);

                colprops.nMaxDataWidth = 50;
            }
            else {
                colprops.bAllowFocus = false;
                colprops.bVisible = false;
            }
            break;

        case UDF:
        case UDFDescription:
            InitMappedUDFColumn(colprops);
            break;

            //	PBI 27164	2/5/2015	mvt - Added Job, Phase, and Cost Code columns for Job Cost
        case JobCode:
        case PhaseCode:
        case CostCode:
            if (!CMS::GetSI()->ModulesInstalled.JobCost.bIsUsable)
                colprops.bVisible = false;
            colprops.nMaxDataWidth = 20;
            break;

        case SourceInvoice:
            colprops.nMaxDataWidth = 20;
            break;

        default:
            colprops.bAllowFocus = false;
            colprops.bVisible = false;

        }
        m_mapcolumninfo[szKey] = colprops;
    }
}

void CGrdPurDetail::OnSetup()
{
    __super::OnSetup();
    LockColumns(2);
    UpdateColumnVisibility();
}

//---------------------------------------------------------------------------
// initialize auto fill fields
void CGrdPurDetail::InitAutoFillEdits()
{
    if (si->ModulesInstalled.Inventory.IsUsable && si->ModuleSettings.CurrentSettings->GetINIntegration() > 0)
    {
        //PTR.06.02.2005.1-17846
        CString szForm = _T("IN_MASTER.STRSTOCK.ALLSTOCK");
        // 1-29210 BK 4/11/08 when MLI lookup was not being reset to user MLI stock lookup and autofill wasn't working correctly
        //bool bFilterMLI = si->ModulesInstalled.MultiLocationInventory.IsUsable;// && si->registerprops->eUseDefaultMLILocation == USEMLI_UseDefaultNoLook && !registerprops->szDefaultMLILocation.IsEmpty();
        //if (bFilterMLI)
        //	szForm = _T("IN_MASTER.STRSTOCK.MLILOCATION");
        //m_clsStockLookInfo.SetLookupInfo(_T("strStock"), szForm, _T("IN"), IDS_IN_MODULE, LKP_IN_STOCKNUMBER);
        //if (si->ModulesInstalled.MultiLocationInventory.IsUsable)
        //	m_clsStockLookInfo.m_uiLookupID = FIL_POLLING_LOCATION_AND_MLILOCATION;

        //LKK 6/2/08 1-29919 new lookups
        // begbert 01-28-2009 1-31873 : Updated to the new _POE variant lookups
        if (CMS::GetSI()->ModulesInstalled.MultiLocationInventory.IsUsable)
            m_clsStockLookInfo.SetLookupInfo(_T("strStock"), szForm, _T("IN"), IDS_IN_MODULE, LKP_IN_STOCKNUMBER_MLI_PURCH_POE);
        else
            m_clsStockLookInfo.SetLookupInfo(_T("strStock"), szForm, _T("IN"), IDS_IN_MODULE, LKP_IN_STOCKNUMBER_PURCH_POE);

        m_clsStockLookInfo.m_aExtraParams.Add(_T("strLocation"));
        m_clsStockLookInfo.m_aExtraParams.Add(si->CompanySettings.DefaultLocation);
        // 1-29210 BK 4/11/08 removed because it's not needed
        //if (bFilterMLI)
        //{
        //	m_clsStockLookInfo.m_aExtraParams.Add(_T("strMLILocation"));
        //	m_clsStockLookInfo.m_aExtraParams.Add(CMS::GetSI()->CompanySettings.DefaultLocation);
        //	m_clsStockLookInfo.m_szLookFilter = _T("LKP_IN_FILTER_MLI_POLLING_LOCATION");
        //}

        // begbert 04-09-2008 1-29227 : Enable add/view buttons if appropriate.
        // begbert 01-27-2009 1-31873 : Per spec, these settings override the user rights settings.
        m_clsStockLookInfo.m_bAddMode = m_pTransaction->GetBuyerSettings()->nInventoryQuery == POConstants::QueryType::INQUIRY_EDIT;
        m_clsStockLookInfo.m_bViewMode = m_pTransaction->GetBuyerSettings()->nInventoryQuery != POConstants::QueryType::NOT_ALLOWED;

        UpdateAutoFillParams(m_clsStockLookInfo);

        CCMSPOEntryLayout::CColumnItem clsItem;
        m_pColLayout->GetItem(POEntryLayout::DetailItems::MLILocation, _T(""), clsItem);

        if (si->ModulesInstalled.MultiLocationInventory.IsUsable &&
            clsItem.IsVisible())
        {
            m_clsMLILocLookInfo.SetLookupInfo(_T("strMLILocation"), _T("IN_MASTER.STRMLILOCATION.STOCKLOC_AUTO"), _T("IN"), IDS_IN_MODULE, LKP_IN_MASTERLOCATION);
            m_clsMLILocLookInfo.m_szLookFilter = _T("IN_STOCK");
            UpdateAutoFillParams(m_clsMLILocLookInfo);
        }

		if (IsColumnVisible(POEntryLayout::DetailItems::Description))
		{
			m_clsINDescLookInfo.SetLookupInfo(_T("strDescription"), _T("IN_MASTER.STRDESCRIPTION.ID_AF_DESCRIPTION"), _T("IN"), IDS_IN_MODULE, LKP_IN_STOCK_DESCR);
			if (si->ModulesInstalled.MultiLocationInventory.IsUsable)
				m_clsINDescLookInfo.m_uiLookupID = LKP_IN_STOCK_DESCR_MLI;
			//// begbert 08-28-2012 1-37356 : Added the location filter.
			//if (bFilterMLI)
			//{
			//	m_clsINDescLookInfo.m_aExtraParams.Add(_T("strLocation"));
			//	m_clsINDescLookInfo.m_aExtraParams.Add(si->CompanySettings.DefaultLocation);
			//	m_clsINDescLookInfo.m_aExtraParams.Add(_T("strMLILocation"));
			//	m_clsINDescLookInfo.m_aExtraParams.Add(registerprops->szDefaultMLILocation);
			//	m_clsINDescLookInfo.m_szLookFilter = _T("FIL_POLLING_LOCATION_AND_MLILOCATION");
			//}
			m_clsINDescLookInfo.m_bCodeLookup = false;
			m_clsINDescLookInfo.m_bScannerMode = true;	// begbert 03-05-2008 1-28932 : Disable Autofill on this column
			m_clsINDescLookInfo.m_bAddMode = true;
			m_clsINDescLookInfo.m_bViewMode = true;
			UpdateAutoFillParams(m_clsINDescLookInfo);
		}
    }
    // 1-31795 BK 1/20/09 changed to IsGLIntegrated() function returns true if  GeneralLedger.IsUsable() and integration level 1 - 9 or 101 - 104 (all but report only settings)
    if (CGBLModuleIntegrationHelper::IsGLIntegrated())
        //if (si->ModulesInstalled.GeneralLedger.IsUsable && si->ModuleSettings.CurrentSettings->GetGLIntegration() > 0)
    {
        // begbert 03-27-2008 1-29035 : corrected the field name from "strGLAccount"
        // begbert 04-22-2008 : corrected the form name from "GL_MASTER"
        m_clsGLAcctLookInfo.SetLookupInfo(_T("strAccount"), _T("GL_MasterTable"), _T("GL"), IDS_GL_MODULE, LKP_GL_ACCOUNT);
        m_clsGLAcctLookInfo.m_bAddMode = true;
        m_clsGLAcctLookInfo.m_bViewMode = true;
        m_clsGLAcctLookInfo.m_szLookFilter = _T("LKP_GL_ACCOUNT_DEFAULT");

		m_clsCostCenterLookInfo.SetLookupInfo(_T("strCostCenter"), _T("GL_COSTCENTERS"), _T("GL"), IDS_GL_MODULE, LKP_GL_COSTCENTERS);
        m_clsCostCenterLookInfo.m_bAddMode = true;
        m_clsCostCenterLookInfo.m_bViewMode = true;
    }


    //	UpdateAutoFillParams(StockNumber, _T("strStockNumber"), _T("IN_MASTER"), _T("IN"), IDS_IN_MODULE, LKP_IN_STOCKNUMBER) ;
    //	UpdateAutoFillParams(GLAccount, _T("strGLAccount"), _T("GL_MASTER"), _T("GL"), IDS_GL_MODULE, LKP_GL_ACCOUNT) ;
    //	UpdateAutoFillParams(CommentCode, _T("strCommentCode"), _T("PO_COMMENTCODE"), _T("PO"), IDS_PO_MODULE, LKP_PO_COMMENTCODES) ;
    //	UpdateAutoFillParams(MLILocation, _T("strMLILocation"), _T("PO_MLILOCATION"), _T("PO"), IDS_PO_MODULE, LKP_LOCATION_STOCK) ;
    UpdateAutoFillParams(VendorStockNum, _T("strVendorStockNum"), _T("PO_VENDORSTOCKNUM"), _T("PO"), IDS_PO_MODULE, LKP_IN_TEMP_ALTERNATE_VENDOR1);
    UpdateUDFAutoFillParams();
}


void CGrdPurDetail::OnCellChange(int oldcol, int newcol, long oldrow, long newrow)
{
    BASETYPE nOldType = UNKNOWN, nNewType = UNKNOWN;
    //	POSITION pos = m_mapcolumninfo.GetStartPosition();
    //	CString szKey;
    //	COLPROPS colprops;

    if (m_pTransaction != NULL)
    {
        if (oldrow >= 0 && oldrow < m_pTransaction->m_arOrderedDetail.GetCount())
            nOldType = m_pTransaction->m_arOrderedDetail.GetAt(oldrow)->WhatAmI();

        if (newrow >= 0 && newrow < m_pTransaction->m_arOrderedDetail.GetCount())
            nNewType = m_pTransaction->m_arOrderedDetail.GetAt(newrow)->WhatAmI();

        if (nOldType != nNewType)
            UpdateStockNumberColumnProperties(newrow);

        // begbert 04-14-2008 1-29391 : set up the ExtraParams that apply to this MLILocation cell at this time.
        // begbert 06-17-2008 1-30160 : but only do so if the line we're moving to is a valid one.
        if (newrow >= 0 && newrow < m_pTransaction->m_arOrderedDetail.GetCount() &&
            FilterUDFids(GetColumnID(newcol)) == MLILocation &&
            si->ModulesInstalled.MultiLocationInventory.IsUsable)
        {
            CStringArray* pArray = &m_clsMLILocLookInfo.m_aExtraParams;
            pArray->RemoveAll();
            pArray->Add(_T("strStock"));
            pArray->Add(m_pTransaction->GetDetailLine(newrow)->GetColumnValueString(StockNumber));
            pArray->Add(_T("strLocation"));
            pArray->Add(si->CompanySettings.DefaultLocation);
            UpdateAutoFillParams(m_clsMLILocLookInfo);
        }

        // begbert 04-14-2008 1-29391 : set up the ExtraParams that apply to this Variant Description cell at this time.
        // begbert 06-17-2008 1-30160 : but only do so if the line we're moving to is a valid one.
        if (newrow >= 0 && newrow < m_pTransaction->m_arOrderedDetail.GetCount() &&
            FilterUDFids(GetColumnID(newcol)) == Description)
        {
			auto pLine = m_pTransaction->GetDetailLine(newrow);
			if (pLine->IsUsingVariants())
			{
				m_clsINDescLookInfo.m_szField = _T("strVariantCode");
				m_clsINDescLookInfo.m_szForm = _T("IN_StockVariant.strVariantCode.StockFilter");
				m_clsINDescLookInfo.m_szModule = _T("IN");
				m_clsINDescLookInfo.m_uiModule = IDS_IN_MODULE;
				m_clsINDescLookInfo.m_uiLookupID = LKP_IN_STOCKVARIANT;
				m_clsINDescLookInfo.m_aExtraParams.RemoveAll();
				m_clsINDescLookInfo.m_aExtraParams.Add(_T("strStock"));
				m_clsINDescLookInfo.m_aExtraParams.Add(pLine->GetColumnValueString(StockNumber));
				m_clsINDescLookInfo.m_aExtraParams.Add(_T("strMLILocation"));
				m_clsINDescLookInfo.m_aExtraParams.Add(pLine->GetColumnValueString(MLILocation));
				m_clsINDescLookInfo.m_szLookFilter = si->ModulesInstalled.MultiLocationInventory.IsUsable ? L"IN_STOCK_MLI_FILTER" : L"IN_STOCK_FILTER";
				m_clsINDescLookInfo.m_bCodeLookup = false;
				m_clsINDescLookInfo.m_bScannerMode = false;	// since this is acting as a code field now, enable the autofill.
				m_clsINDescLookInfo.m_bAddMode = false;
				m_clsINDescLookInfo.m_bViewMode = false;
			} else {
				//bool bFilterMLI = si->ModulesInstalled.MultiLocationInventory.IsUsable && registerprops->eUseDefaultMLILocation == USEMLI_UseDefaultNoLook && !registerprops->szDefaultMLILocation.IsEmpty();

				m_clsINDescLookInfo.m_szField = _T("strDescription");
				m_clsINDescLookInfo.m_szForm = _T("IN_MASTER.STRDESCRIPTION.ID_AF_DESCRIPTION");
				m_clsINDescLookInfo.m_szModule = _T("IN");
				m_clsINDescLookInfo.m_uiModule = IDS_IN_MODULE;
				m_clsINDescLookInfo.m_uiLookupID = si->ModulesInstalled.MultiLocationInventory.IsUsable ? LKP_IN_STOCK_DESCR_MLI : LKP_IN_STOCK_DESCR;
				m_clsINDescLookInfo.m_aExtraParams.RemoveAll();
				//if (bFilterMLI)
				//{
				//	m_clsINDescLookInfo.m_aExtraParams.Add(_T("strLocation"));
				//	m_clsINDescLookInfo.m_aExtraParams.Add(si->CompanySettings.DefaultLocation);
				//	m_clsINDescLookInfo.m_aExtraParams.Add(_T("strMLILocation"));
				//	m_clsINDescLookInfo.m_aExtraParams.Add(registerprops->szDefaultMLILocation);
				//	m_clsINDescLookInfo.m_szLookFilter = _T("FIL_POLLING_LOCATION_AND_MLILOCATION");
				//}
				m_clsINDescLookInfo.m_bCodeLookup = false;
				m_clsINDescLookInfo.m_bScannerMode = true;	// begbert 03-05-2008 1-28932 : Disable Autofill on this column
				m_clsINDescLookInfo.m_bAddMode = true;
				m_clsINDescLookInfo.m_bViewMode = true;
			}
			UpdateAutoFillParams(m_clsINDescLookInfo);
        }

		if (newrow >= 0 && newrow < m_pTransaction->m_arOrderedDetail.GetCount() &&
            FilterUDFids(GetColumnID(newcol)) == CostCenter &&
            CGBLModuleIntegrationHelper::IsGLIntegrated())
        {
            UpdateAutoFillParams(m_clsCostCenterLookInfo);
        }

    }

    __super::OnCellChange(oldcol, newcol, oldrow, newrow);
}


void CGrdPurDetail::UpdateStockNumberColumnProperties(long newrow)
{
    if (m_pTransaction == NULL ||
        newrow < 0 ||
        newrow >= m_pTransaction->m_arOrderedDetail.GetCount())
        return;

    BASETYPE nNewType = m_pTransaction->m_arOrderedDetail.GetAt(newrow)->WhatAmI();
    POSITION pos = m_mapcolumninfo.GetStartPosition();
    CString szKey;
    COLPROPS colprops;

    while (true)
    {
        if (!pos) return;
        m_mapcolumninfo.GetNextAssoc(pos, szKey, colprops);
        if (FilterUDFids(colprops.nDBColType) == StockNumber)
            break;
    }

    AUTOFILLINFO* pAutoFillInfo = NULL;
    // begbert 03-28-2008 1-29353 : UNKNOWNs get inventorylike lookups.
    // begbert 10-14-2008 1-31110 : UNKNOWNs now fall back on direxpense lookups in the case of no IN integration.
    if ((nNewType == UNKNOWN || nNewType == INVENTORY) && si->ModulesInstalled.Inventory.IsUsable && si->ModuleSettings.CurrentSettings->GetINIntegration() > 0)
        pAutoFillInfo = &m_clsStockLookInfo;
    // 1-32088 BK 2/27/09 GL Account isn't an autofill it is a separate control, changed to use GL Account control and exit function
    // 1-31795 BK 1/20/09 changed to IsGLIntegrated() function returns true if  GeneralLedger.IsUsable() and integration level 1 - 9 or 101 - 104 (all but report only settings)
    //else if ((nNewType == UNKNOWN || nNewType == DIRECTEXPENSE) && si->ModulesInstalled.GeneralLedger.IsUsable && si->ModuleSettings.CurrentSettings->GetGLIntegration() > 0)
    else if ((nNewType == UNKNOWN || nNewType == DIRECTEXPENSE) && CGBLModuleIntegrationHelper::IsGLIntegrated())
    {
        //		pAutoFillInfo = &m_clsGLAcctLookInfo;
        m_mapcolumninfo[szKey].nDataType = GRD_GRIDACCTCTRL;
        m_mapcolumninfo[szKey].nTranslateType = GRD_XLAT_ACCOUNTNMBR;
        InitEmbeddedControls(true);
        return;
    }

    if (pAutoFillInfo != NULL)
    {
        UpdateAutoFillParams(*pAutoFillInfo);
        m_mapcolumninfo[szKey].nDataType = GRD_GRIDAUTOFILL;
    }
    else
        m_mapcolumninfo[szKey].nDataType = UGCELLDATA_STRING;

    InitEmbeddedControls(true);
}

//---------------------------------------------------------------------------
//
/*
bool CGrdPurDetail::IsColumnSupported(const INT32& nDBColType)
{
    COLPROPS colprops ;
    return m_clsAllColumns.Lookup(nDBColType, colprops) == TRUE ;
}
*/

//---------------------------------------------------------------------------
//Column prompts are message texts displayed on status area
//when a column gets focus. Setup prompt ids for all
//supported column types
void CGrdPurDetail::SetupColumnPrompts()
{
    SetColPrompt(StockNumber, IDS_POE_STOCK_STATUS_MSG);//Defect ID 1-30268, PGP(07-26-2008)
    SetColPrompt(GLAccount, IDS_SE_STATUS_GRD_HDR_CHAR_ALLOWED);
    SetColPrompt(Comment, IDS_SE_STATUS_GRD_HDR_CHAR_ALLOWED);
    SetColPrompt(MLILocation, IDS_SE_STATUS_GRD_HDR_CHAR_ALLOWED);
    SetColPrompt(QuantityOrdered, IDS_SE_STATUS_GRD_HDR_NUM_ONLY);
	SetColPrompt(CostCenter, IDS_SE_STATUS_GRD_HDR_CHAR_ALLOWED);
    SetColPrompt(VendorStockNum, IDS_SE_STATUS_GRD_HDR_CHAR_ALLOWED);
    SetColPrompt(Description, IDS_SE_STATUS_GRD_HDR_CHAR_ALLOWED);
    SetColPrompt(OrderedDate, IDS_SE_STATUS_GRD_HDR_DATE);
    SetColPrompt(ExpectedDate, IDS_SE_STATUS_GRD_HDR_DATE);
    SetColPrompt(UnitCost, IDS_SE_STATUS_GRD_HDR_NUM_ONLY);
    SetColPrompt(ExtendedCost, IDS_SE_STATUS_GRD_HDR_NUM_ONLY);
    SetColPrompt(IsLandItem, IDS_SE_STATUS_GRD_HDR_NUM_ONLY);
    SetColPrompt(IsTaxable, IDS_BLANK);
    SetColPrompt(IsDiscount, IDS_BLANK);
    SetColPrompt(IsApplyFreight, IDS_BLANK);
    SetColPrompt(LastCost, IDS_BLANK);
    SetColPrompt(References, IDS_SE_STATUS_GRD_HDR_CHAR_ALLOWED);
    SetColPrompt(Encumbrance, IDS_SE_STATUS_GRD_HDR_CHAR_ALLOWED);	// begbert 03-22-2011 1-35267 : added Encumbrance column

//	UDF
//	UDFDescription
}

//---------------------------------------------------------------------------
// 1-18734 BK 4/5/05 update trans number and invoice number when a new transaction is being created
void CGrdPurDetail::OnSetTransaction()
{
    /*
        // 1-18734 BK 04/21/05 commented out these lines because of design change:
        // tran number will be displayed as Gen.OnSave on new transactions will only display tran number on recalled transactions
        // set next transaction number in Detail grid
        //CString szText;
        //int nTranCol = GetDisplayedColumnIndex(SalesEntryLayout::DetailItems::TransactionNumber) ;
        //if(nTranCol >= 0)
        //{
        //	szText.Format(_T("%d"), m_pTransaction->GetTranNo());
        //	QuickSetText(nTranCol, 0, szText);
        //	RedrawCell(nTranCol, 0);
        //}

        // if not using OnLine Invoice numbering display next Invoice number or next WorkOrder/Quote number in grid
        CString szText;
        if (!registerprops)  return;

        // 1-20726 BK 8/3/05 added m_pTransaction->GetRecalled()
        if(!registerprops->bOnlineInvoiceNumber || m_pTransaction->GetRecalled())
        {
            int nInvoiceNoCol = GetDisplayedColumnIndex(SalesEntryLayout::DetailItems::InvoiceNumber);
            // 1-20726 BK 8/3/05 when Quote is recalled and changed into Invoice & w/OnlineInvoiceNo display GenOnSave because
            // new invoice number will be generated when it is saved
            if (m_pTransaction->GetCreateNewInvoiceNo() && registerprops->bOnlineInvoiceNumber)
                szText = CGBLResources::GetResourceString(IDS_GEN_ON_SAVE);
            else
                // BK 12/13/05 used with invoice number, quote and work order number
                szText = m_pTransaction->GetInvoiceNo();

            if(nInvoiceNoCol >= 0)
            {
                QuickSetText(nInvoiceNoCol, 0, szText);
                RedrawCell(nInvoiceNoCol, 0);
            }
        }
    */
}

/***************************************************
OnGetCell
    This message is sent everytime the grid needs to
    draw a cell in the grid. At this point the cell
    class has been filled with the information to be
    used to draw the cell. The information can now be
    changed before it is used for drawing
****************************************************/
void CGrdPurDetail::OnGetCell(int col, long row, CUGCell *cell) {
    UNREFERENCED_PARAMETER(cell);
    UNREFERENCED_PARAMETER(col);
    UNREFERENCED_PARAMETER(row);
}

//---------------------------------------------------------------------------
//Change column widths on size. this avoids a horizontal scrollbar
//PGP(10/13/2003)
void CGrdPurDetail::OnSize(UINT nType, int cx, int cy)
{
    //invoice number, invoice date, payment method, salesperson, shipvia,
    //shipdate,po number, tax code, discount code.
    //125, 125, 150, 50, 50, 50, 50, 50, 50
    //const INT32 MAX_COLUMNS = 9 ;
    //INT32 nWidthArray[MAX_COLUMNS] = {125, 85, 120, 60, 60, 85, 60, 60, 0} ;
    //INT32 nSumWidth = 0 ;
    //INT32 nIndex = 0 ;
    //for(nIndex = 0; nIndex < MAX_COLUMNS; nIndex++)
    //	nSumWidth += nWidthArray[nIndex] ;

    //nWidthArray[MAX_COLUMNS-1] = cx - nSumWidth ;
    //for(nIndex = 0; nIndex < MAX_COLUMNS; nIndex++)
    //	SetColWidth(nIndex, nWidthArray[nIndex]) ;

    //RedrawAll() ;

    CUGCtrl::OnSize(nType, cx, cy);
}

//---------------------------------------------------------------------------
int CGrdPurDetail::OnCanMove(int oldcol, long oldrow, int newcol, long newrow)
{
    return CUGCtrl::OnCanMove(oldcol, oldrow, newcol, newrow);
}

void CGrdPurDetail::OnEvent(CCMSSubject * pSubject, UINT uiEvent)
{
    if (uiEvent == POE_WM_CHANGED_AMOUNTS)
    {
        UpdateColumnVisibility();
        UpdateFreightTaxDiscountLanding();	// begbert 04-30-2008 1-29507 : update the totals.
    }
    __super::OnEvent(pSubject, uiEvent);
}

void CGrdPurDetail::UpdateColumnVisibility()
{
    // begbert 04-16-2008 : Largely rewrote to avoid problems if the columns weren't
    //   in the grid at all, due to being set to invisible in the layout.

    // begbert 03-11-2009 1-32323 : Added the 'Taxable' column visibility adjustment, and made all
    //   three landing checkbox columns dependent on the Landing Method not being Auto Distribute.
    // begbert 03-17-2009 1-32389 : Items To Land must also be set to None for Auto Distribute.

	// begbert 05-23-2019 PBI 30685 : Now, a decade later, the winds have shifted and we
	//   don't want the columns to be hidden based on the landing method anymore.

	// begbert 11-27-2019 PBI 52533 : And now the last vestiges of behavior of this function are gone,
	//   but I'll leave the empty shell here as a placeholder and an example for if we want to make
	//   columns to disappear at some point in the future.

    //int nCol = GetDisplayedColumnIndex(IsTaxable);
    //if (nCol >= 0)
    //{
    //    if (CMS::GetSI()->ModuleSettings.POSettings.nItemsToLand == POConstants::ItemsToLand::NONE ||
    //        CMS::GetSI()->ModuleSettings.POSettings.nLandingMethod != POConstants::LandingMethod::AUTO_DISTRIBUTION)
    //    {
    //        ShowColumn(nCol, SW_NORMAL);
    //    }
    //    else {
    //        ShowColumn(nCol, SW_HIDE);
    //    }
    //}

    //nCol = GetDisplayedColumnIndex(IsDiscount);
    //if (nCol >= 0)
    //{
    //    if (m_pTransaction->GetGridDiscountPercentAmt() != 0 &&
    //        (CMS::GetSI()->ModuleSettings.POSettings.nItemsToLand == POConstants::ItemsToLand::NONE ||
    //            CMS::GetSI()->ModuleSettings.POSettings.nLandingMethod != POConstants::LandingMethod::AUTO_DISTRIBUTION))
    //    {
    //        ShowColumn(nCol, SW_NORMAL);
    //    }
    //    else {
    //        ShowColumn(nCol, SW_HIDE);
    //    }
    //}

    //int nCol = GetDisplayedColumnIndex(IsApplyFreight);
    //if (nCol >= 0)
    //{
    //    if (m_pTransaction->GetGridFreightAmount() != 0 &&
    //        //CMS::GetSI()->ModuleSettings.POSettings.nLandingMethod != POConstants::LandingMethod::AUTO_DISTRIBUTION &&
    //        (si->ModuleSettings.POSettings.nItemsToLand == POConstants::ItemsToLand::INVENTORY || si->ModuleSettings.POSettings.nItemsToLand == POConstants::ItemsToLand::ALL_LINES))
    //    {
    //        ShowColumn(nCol, SW_NORMAL);
    //    }
    //    else {
    //        ShowColumn(nCol, SW_HIDE);
    //    }
    //}
}

//---------------------------------------------------------------------------
//Called on first time initialization, and then each time grid is
//refreshed when a customer change occurs.
void CGrdPurDetail::ClearGrid()
{
    // begbert 01-15-2008 1-31713 : zero out the active cell so that no partial
    //   lines or anything get added back in upon some cell losing focus later on.
    GotoCell(0, 0);

    // begbert 04-29-2008 : There's a chance the transaction's lines will already have been cleared,
    //  so synchronize the grid's line count with it before trying to delete anything.
    SetNumberRows((int)m_pTransaction->m_arOrderedDetail.GetCount());

    // begbert 04-16-2008 : Prevent a leftover visible control from re-adding data to a row once they're all gone
    m_pCtrlHelper->HideVisibleCtrl();
    int nRow = GetNumberRows();
    while (nRow > 0) DeleteObject(--nRow);

    SetNumberRows(1);

    //	// clear all columns
    //	m_nCols = GetNumberCols();
    //	for (int n=0; n< m_nCols; n++)
    //		QuickSetText(n, 0, _T(""));


        // 1-19409 BK 5/18/05 if the transaction is being recalled exit here so CMSTransaction variables won't be changed after data is retrieved
        // begbert 04-25-2008 1-29463 : this causes problems and is no longer necessary; ClearGrid is no longer called on Refresh().
    //	if (m_pTransaction && m_pTransaction->GetRecalled())
    //		return;

    if (m_pTransaction)
    {
        // begbert 03-30-2009 1-32431 : Use the delete detail lines function - its safer.
        m_pTransaction->DeleteDetailLines(false);

        CLTDetailBase* pLine = CLTDetailBase::GetNew(UNKNOWN, this, m_pTransaction);
        m_pTransaction->AddDetailLine(pLine);

        UpdateFreightTaxDiscountLanding();	// begbert 04-18-2008 1-29463 : update the totals.

    }

    UpdateHeadings(0);	// begbert 07-18-2008 1-30215 : Reset the key-column's heading to stock number.

    // resets grid so combo will not keep focus and data can be set into grid when customer is changed
    OnKillFocus(0);
}

//---------------------------------------------------------------------------
//Called from FVEntry when the customer selection changes. PGP(10/23/2003)
//Changed to user defined message handler for CMS_MSG_REFRESH_Detail_GRID
//Sent when customer info is refreshed in embedded customer dialog. PGP(12/22/2003)
//Added update for additional columns PGP(02/09/2004)
// BK 1/14/05 called from dlgEmbeddedCustomer::RefreshFromDlgEntry (on customer kill focus) SE_WM_REFRESH_Detail_GRID
// BK 1/18/05 also called from CFVEntry::RefreshForm (on recall transaction)
LRESULT CGrdPurDetail::Refresh(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    //Pre-Condition: Check for Valid transaction
    if (!m_pTransaction)	return 0;

    // begbert 03-05-2009 1-32147 : Hide the control so it doesn't carry an old value into the newly-refreshed grid.
    m_pCtrlHelper->HideVisibleCtrl(false);

    INT_PTR nCount = m_pTransaction->m_arOrderedDetail.GetCount();
    if (nCount == 0 || m_pTransaction->m_arOrderedDetail[nCount - 1]->WhatAmI() != UNKNOWN)
    {
        CLTDetailBase* pLine = CLTDetailBase::GetNew(UNKNOWN, this, m_pTransaction);
        m_pTransaction->AddDetailLine(pLine);
        nCount++;
    }
    SetNumberRows((long)nCount);
    if (nCount == 1)
        UpdateStockNumberColumnProperties(0);


    // recalled transaction
    if (m_pTransaction->GetRecalled())
    {
        // begbert 05-23-2008 : clear control, lest it interfere with this
        m_pCtrlHelper->HideVisibleCtrl();

        for (int nRow = 0; nRow < m_pTransaction->m_arOrderedDetail.GetCount(); nRow++)
            for (int nCol = 0; nCol < m_arOrderedColumns.GetCount(); nCol++)
            {
                INT32 nColID = GetColumnID(nCol);
                if (nColID == -1) continue;
                CString szValue = GetColumnValueString(nColID, nRow);
                UpdateColValue(nRow, nColID, szValue);	// begbert 04-17-2009  : nRow and nCcolID were switched
            }
        UpdateColumnVisibility(); // begbert 04-17-2008 : added this

        // begbert 05-23-2008 : Update the grid state.
        UpdateHeadings(0);
        Select(0, 0);
    }

    UpdateFreightTaxDiscountLanding();	// begbert 05-19-2008 : This was missing

    RedrawAll();
    return 0;
}

// begbert 05-23-2008 1-29934 : This function ensures that the Stock Column Properties stay in sync with the headings.
void CGrdPurDetail::UpdateHeadings(long nRow)
{
    __super::UpdateHeadings(nRow);
    UpdateStockNumberColumnProperties(nRow);
}

// ********************************************************
// * Procedure:    UpdateColValue
// *
// * Description:  Updates the value for the passed in column ID
// *
// * Parameters:   szColID - Column to update
// *
// * Date Created: 12/31/2003 by Peter Ringering
// ********************************************************
//PGP(02/09/2004)-Altered to add default parameter to update heading.
//bHeading defaults to false, so that the function updates row 0.
//if set to true, it updates the column heading instead. Used to rename udf column headings.
void CGrdPurDetail::UpdateColValue(long lRow, INT32 nColID, LPCTSTR szValue, bool bUpdateTrans, bool bHeading/*=false*/)
{
    //PTR.04.13.2005.1-16954 - Make sure we set the transaction variable when column is not showing.
    int nCol = GetDisplayedColumnIndex(nColID);
    if (nCol >= 0)
    {
        QuickSetText(nCol, bHeading ? -1 : lRow, szValue);
        if (bUpdateTrans)
            UpdateTransaction(lRow, nColID, szValue);
    }
}

// ********************************************************
// * Procedure:    UpdateTransaction
// *
// * Description:  Updates the appropriate m_pTransaction value for the passed in column number.
// *
// * Parameters:   nColID - Column ID to update.
// *			   szValue - Value to update with.
// *
// * Date Created: 02/14/2004 by Peter Ringering
// ********************************************************
void CGrdPurDetail::UpdateTransaction(long lRow, INT32 nColID, LPCTSTR szValue, bool bOverwriteSame)
{
    //Check for Preview mode.
    if (m_pTransaction == NULL ||
        lRow >= m_pTransaction->m_arOrderedDetail.GetCount() ||
        m_pTransaction->m_arOrderedDetail[lRow] == NULL)
        return;

    CLTDetailBase* pLine = (CLTDetailBase*)m_pTransaction->m_arOrderedDetail[lRow];
    CString szOldValue = pLine->GetColumnValueString(nColID);

    // begbert 03-11-2009 1-30131 : rather than bailing out of the function to avoid values altered
    //   in the select from being altered at the end of the function, we instead set a flag saying
    //   whether or not to do so.  This allows the other processing that happens afterwards to happen.
    bool bNeedToSetValue = true;

    // begbert 05-20-2008 : Numbers should be compared as numbers.
    COLPROPS objColProps;
    GetColumnProps(nColID, objColProps);
    // begbert 11-24-2008 1-31469 : Added a parameter to allow the value to be set even if that
    //   would not directly change it - useful to trigger a re-run of the validation.
    if (!bOverwriteSame)
        if (objColProps.nDataType == UGCELLDATA_NUMBER ||
            objColProps.nDataType == UGCELLDATA_CURRENCY)
        {
            if (SetDouble(szValue) == SetDouble(szOldValue)) return;
        }
        else {
            if (szValue == szOldValue) return;
        }

        // begbert 08-12-2008 1-30244 : split out the MLILocation case to correctly handle the different
        //   validation; also moved the actual setting of the field until after the validation is complete.
        switch (FilterUDFids(nColID))
        {
        case StockNumber: case GLAccount: case Comment:
            // begbert 04-09-2008 1-28813 : added line type cases and ShowNoInfoDetectedDlg call
            if (pLine->WhatAmI() == INVENTORY)
            {
                if (si->ModulesInstalled.Inventory.IsUsable && si->ModuleSettings.CurrentSettings->GetINIntegration() > 0)
                {
                    if (ValidateInventory(lRow, szValue))
                    {
                        // begbert 11-11-2008 1-31343 : If it validated, Do the Calculations and we're done.
                        // begbert 03-11-2009 1-30131 : Not quite done; and the calulations are now done at function end.
                        bNeedToSetValue = false;
                    }
                    else if (!ShowNoInfoDetectedDlg(pLine, szValue))
                        return;
                    else if (m_pTransaction->m_arOrderedDetail[lRow]->WhatAmI() == COMMENT)
                        bNeedToSetValue = false;	// begbert 12-13-2013 PBI 13166 : If we showed the 'no info detected' form and a comment line was added, don't try to set the comment text into the key field later.

                }
                else {
                    // begbert 08-27-2008 1-30131 : Given a new stock number the line should reset.
                    // begbert 11-24-2008 1-31725 : Now take care to delete the old line, making a new one in its place.
                    m_pTransaction->m_arOrderedDetail[lRow] = CLTDetailBase::GetNew(pLine->WhatAmI(), this, m_pTransaction);
                    pLine->Delete();
                }
            }
            else if (pLine->WhatAmI() == DIRECTEXPENSE)
            {
                // 1-31795 BK 1/20/09 changed to IsGLIntegrated() function returns true if  GeneralLedger.IsUsable() and integration level 1 - 9 or 101 - 104 (all but report only settings)
                if (CGBLModuleIntegrationHelper::IsGLIntegrated())
                    // begbert 10-14-2008 1-31110 : Enabled for autofill on direct expense lines.
                    //if (si->ModulesInstalled.GeneralLedger.IsUsable && si->ModuleSettings.CurrentSettings->GetGLIntegration() > 0)
                {
                    if (ValidateGLAccount(lRow, szValue))
                    {
                        // begbert 11-11-2008 1-31343 : If it validated, Do the Calculations and we're done.
                        // begbert 03-11-2009 1-30131 : Not quite done; and the calulations are now done at function end.
                        bNeedToSetValue = false;
                    }
                    else if (!ShowNoInfoDetectedDlg(pLine, szValue))
                        return;
                    else if (m_pTransaction->m_arOrderedDetail[lRow]->WhatAmI() == COMMENT)
                        bNeedToSetValue = false;	// begbert 12-13-2013 PBI 13166 : If we showed the 'no info detected' form and a comment line was added, don't try to set the comment text into the key field later.
                }
            }

            // begbert 02-04-2009 1-31940 : Moved this out here so that, regardless of what the line *was*,
            //   if it's now an inventory line, this gets done.
            pLine = m_pTransaction->m_arOrderedDetail[lRow];
            if (pLine->WhatAmI() == INVENTORY)
            {
                // begbert 08-27-2008 1-30131 : Given a new stock number, the Taxable and Discount
                //   checkboxes' default values depend on whether amounts have been set for them in the header
                //   at the time the stock number is set.
                pLine->SetColumnValueBool(IsTaxable, m_pTransaction->GetSalesTaxPercent() > 0.0);
                pLine->SetColumnValueBool(IsDiscount, m_pTransaction->GetGridDiscountPercentAmt() > 0.0);

				// begbert 12-04-2019 PBI 52533 : The 'Apply Freight' checkbox no longer cares about the amount in the header.
                pLine->SetColumnValueBool(IsApplyFreight, true);
            }

            // begbert 03-12-2009 1-32346 : if the landing method is autodistribute, turn it all on regardless.
            pLine->ApplyAutodistributionToLanding();

            break;

        case MLILocation:
            // begbert 04-09-2008 1-28813 : added line type cases and ShowNoInfoDetectedDlg call
            if (pLine->WhatAmI() == INVENTORY)
            {
                if (si->ModulesInstalled.Inventory.IsUsable && si->ModuleSettings.CurrentSettings->GetINIntegration() > 0)
                {
                    // begbert 08-27-2008 1-30131 : added a parameter to indicate *not* to reset the line,
                    //   even if a new stock record is found for the new location.
                    // begbert 11-12-2008 : the 'pLine->' on the GetColumnValueString() call was missing.
                    if (ValidateInventory(lRow, pLine->GetColumnValueString(StockNumber), szValue, false))
                    {
                        // begbert 11-11-2008 1-31343 : If it validated, Do the Calculations and we're done.
                        // begbert 03-11-2009 1-30131 : Not quite done; and the calulations are now done at function end.
                        bNeedToSetValue = false;
                    }
                    else
                        return;
                }
            }
            break;
        }

        if (bNeedToSetValue)
            pLine->SetColumnValueString(nColID, szValue);
        DoCalculations(nColID, lRow);
}


// this is called after the transaction is updared
void CGrdPurDetail::DoCalculations(int nColID, long nRow)
{
    switch (FilterUDFids(nColID))
    {
    case StockNumber:
    case MLILocation:

        if (si->ModulesInstalled.Inventory.IsUsable && si->ModuleSettings.CurrentSettings->GetINIntegration() > 0)
        {
            UpdateExtendedCost(nRow, false);
            UpdateFreightTaxDiscountLanding();
        }
        break;

    case QuantityOrdered:
        UpdateQuantityRemaining(nRow);
        // ..falls through...
    case UnitCost:
    case ExtendedCost:
        UpdateExtendedCost(nRow, nColID == ExtendedCost);
        UpdateFreightTaxDiscountLanding();
        break;

    case IsLandItem:
    case IsTaxable:
    case IsDiscount:
    case IsApplyFreight:
        UpdateFreightTaxDiscountLanding();
        break;
        /*
            case GLAccount:
            case Comment:
            case VendorStockNum:
            case Description:
            case OrderedDate:
            case ExpectedDate:
            case LastCost:
            case References:
            case Encumbrance:	// begbert 03-22-2011 1-35267 : added Encumbrance column
        */
    }
}


// begbert 06-12-2008 1-30118 : Added the optional parameter
void CGrdPurDetail::UpdateFreightTaxDiscountLanding(bool bAsIfAllWerePositive)
{
    using namespace POEntryLayout::DetailItems;
    double dLineQty, dLineExt, dLineDisc, dLineTax, dLineFrt, dLineLand;
    double dPOSubtotal, dTotalDiscount, dTotalTax, dTotalFreight;
    double dTotalDiscountable, dTotalApplyFreight;

    bool bIsDollar = m_pTransaction->GetDiscountType() == Dollar;

    dPOSubtotal = 0.0;
    dTotalDiscount = bIsDollar ? m_pTransaction->GetGridDiscountPercentAmt() : 0.0;
    dTotalTax = 0.0;
    dTotalFreight = m_pTransaction->GetGridFreightAmount();
    dTotalDiscountable = 0.0;
    dTotalApplyFreight = 0.0;

    // Sum up subtotal, and (for dollar discounts) count and total the discountable lines.
    for (int i = 0; i < m_pTransaction->m_arOrderedDetail.GetCount(); i++)
    {
        CLTDetailBase* pLine = m_pTransaction->m_arOrderedDetail[i];

        if (pLine->WhatAmI() != INVENTORY && pLine->WhatAmI() != DIRECTEXPENSE)
            continue;	// begbert 10-24-2008 : Don't waste time processing dataless lines.

        dLineExt = pLine->GetColumnValueDouble(ExtendedCost);

        // begbert 06-12-2008 1-30118 : make it positive if necessary
        if (bAsIfAllWerePositive) dLineExt = abs(dLineExt);

        dPOSubtotal += dLineExt;

        if (bIsDollar && pLine->GetColumnValueBool(IsDiscount))
            dTotalDiscountable += abs(dLineExt);

        if (pLine->GetColumnValueBool(IsApplyFreight))
            dTotalApplyFreight += abs(dLineExt);
    }

    // Calculate the per-line tax
    for (int i = 0; i < m_pTransaction->m_arOrderedDetail.GetCount(); i++)
    {
        CLTDetailBase* pLine = m_pTransaction->m_arOrderedDetail[i];

        if (pLine->WhatAmI() != INVENTORY && pLine->WhatAmI() != DIRECTEXPENSE)
            continue;	// begbert 10-24-2008 : Don't waste time processing dataless lines.

        dLineExt = pLine->GetColumnValueDouble(ExtendedCost);
        dLineQty = pLine->GetColumnValueDouble(QuantityOrdered);

        // begbert 06-12-2008 1-30118 : make them positive if necessary
        if (bAsIfAllWerePositive) dLineExt = abs(dLineExt);
        if (bAsIfAllWerePositive) dLineQty = abs(dLineQty);

        if (dLineQty == 0.0)
        {
            pLine->SetColumnValueDouble(PurUnitDiscount, 0.0);
            pLine->SetColumnValueDouble(PurExtDiscount, 0.0);
            pLine->SetColumnValueDouble(PurUnitTax, 0.0);
            pLine->SetColumnValueDouble(PurExtTax, 0.0);
            pLine->SetColumnValueDouble(PurUnitFreight, 0.0);
            pLine->SetColumnValueDouble(PurExtFreight, 0.0);
            pLine->SetColumnValueDouble(PurUnitLanding, 0.0);
            pLine->SetColumnValueDouble(PurExtLanding, 0.0);
        }

        dLineDisc = 0.0;
        if (pLine->GetColumnValueBool(IsDiscount))
        {
            if (bIsDollar)
            {
                dLineDisc = abs(dTotalDiscount * dLineExt / dTotalDiscountable);
            }
            else if (dLineExt > 0.0)
            {
                // begbert 03-17-2009 1-32380 : round off the detail line discount before summing it
                dLineDisc = SetDouble(GetCurrency(dLineExt * m_pTransaction->GetGridDiscountPercentAmt() / 100.0));
                dTotalDiscount += dLineDisc;
            }
        }

        dLineTax = 0.0;
        if (pLine->GetColumnValueBool(IsTaxable))
        {
            // begbert 03-17-2009 1-32380 : round off the detail line tax before summing it
            dLineTax = SetDouble(GetCurrency((dLineExt - dLineDisc) * m_pTransaction->GetSalesTaxPercent() / 100.0));
            dTotalTax += dLineTax;
        }

        dLineFrt = 0.0;
        if (pLine->GetColumnValueBool(IsApplyFreight))
            dLineFrt = abs(dTotalFreight * dLineExt / dTotalApplyFreight);

        dLineLand = dLineExt - dLineDisc + dLineTax + dLineFrt;
        pLine->SetColumnValueDouble(PurUnitDiscount, dLineDisc / dLineQty);
        pLine->SetColumnValueDouble(PurExtDiscount, dLineDisc);
        pLine->SetColumnValueDouble(PurUnitTax, dLineTax / dLineQty);
        pLine->SetColumnValueDouble(PurExtTax, dLineTax);
        pLine->SetColumnValueDouble(PurUnitFreight, dLineFrt / dLineQty);
        pLine->SetColumnValueDouble(PurExtFreight, dLineFrt);
        pLine->SetColumnValueDouble(PurUnitLanding, dLineLand / dLineQty);
        pLine->SetColumnValueDouble(PurExtLanding, dLineLand);
    }

    // begbert 11-12-2008 1-31395 : Round these off now, as they may contain fractional cents.
    dTotalDiscount = SetDouble(GetCurrency(dTotalDiscount));
    dTotalTax = SetDouble(GetCurrency(dTotalTax));

    m_pTransaction->SetPOSubtotal(dPOSubtotal);
    m_pTransaction->SetTotalExpectedDollars(dPOSubtotal);	// begbert 11-18-2008 1-31288 : TotalExpectedDollars should be the subtotal, not the total.
    m_pTransaction->SetTotalEstimatedDiscount(dTotalDiscount);
    m_pTransaction->SetTotalEstimatedTax(dTotalTax);
    m_pTransaction->SetTotalEstimatedFreight(dTotalFreight);
    m_pTransaction->SetPOAmount(dPOSubtotal - dTotalDiscount + dTotalTax + dTotalFreight);

    // begbert 06-12-2008 1-30118 : Not only is updating the totals section probably undesirable
    //   if we're playing what-if games with the numbers, but this actually re-calculates the
    //   numbers itself and therefore undoes the very differences we wanted.
    if (!bAsIfAllWerePositive)
        m_pTransaction->UpdateTotals();
}

// begbert 08-12-2008 1-30244 : You can now pass in the values to be validated rather than having to assign them to the line.
// begbert 08-27-2008 1-30131 : And you can now pass in whether or not to reset the other fields of the line if the validation succeeds.  (Defaults to true.)
bool CGrdPurDetail::ValidateInventory(long nRow, CString p_szStock, CString p_szMLILocation /*=EMPTY_STRING*/, bool bResetOnSuccess /*=true*/)
{
    CDataRow objDR, objDR2;

    CLTDetailBase* pLine = m_pTransaction->m_arOrderedDetail[nRow];

    if (!pLine->GetInventoryItem(objDR, p_szStock, p_szMLILocation)) return false;

    CString szStock = objDR.XMLStr(_T("strStock"));
    CString szSuperseded = objDR.XMLStr(_T("strSupersededStock"));

    if (!szSuperseded.IsEmpty())
    {
        objDR.Dispose();

        UINT    uiTitleID = IDS_SAE_CIRCSUPERSEDED_CAPTION;
        CString szMessage;
        if (ValidateSuperceded(pLine, szSuperseded, szStock))
        {
            uiTitleID = IDS_SAE_SUPERSEDEDUSED_CAPTION;
            szMessage = RESSTRING(IDS_SAE_SUPERSEDEDUSED_MSG);
        }
        else
            szMessage = RESSTRING(szSuperseded == szStock ? (UINT)IDS_SAE_CIRCSUPERSEDED_MSG : (UINT)IDS_SAE_SUPERSEDEDNF_MSG);

        szMessage.Replace(_T("<STRSUPERSEDEDSTOCK>"), szSuperseded);
        szMessage.Replace(_T("<STRORIGSTOCK>"), szStock);

        if (uiTitleID == IDS_SAE_SUPERSEDEDUSED_CAPTION)
        {
            szStock = szSuperseded;

            if (pLine->GetInventoryItem(objDR, szStock))
                szStock = objDR.XMLStr(_T("strStock"));
        }

        // begbert 11-19-2008 1-31343 : Set the stock number to the value the user typed in, so that the inevitable
        //   spate of focus messages caused when a message box is shown from within the lostfocus handler will be
        //   ignored by the part that ignores lostfocus messages when the value wasn't change.  This is an acceptable
        //   approach because the value will be soon replaced by the actual superceding stock item anyway.
        // begbert 11-21-2008 1-31343 : Set the MLI Location similarly.
        pLine->SetColumnValueString(StockNumber, p_szStock);
        pLine->SetColumnValueString(MLILocation, p_szMLILocation);

        CGBLForm::CMSMessageBox(this, 0, uiTitleID, MB_OK | MB_ICONEXCLAMATION, szMessage, EMPTY_STRING, 0, false);
    }

    // begbert 08-27-2008 1-30131 : clear out the old to make way for the new, if they didn't say not to
    // begbert 11-24-2008 1-31469 : Now take care to delete the old line, making a new one in its place
    //   which gets a copy of the old line's values, if they didn't say not to.
    DeleteComment(nRow, false);	// begbert 02-27-2009 1-32159 : Clear out comments attached to the line
    m_pTransaction->m_arOrderedDetail[nRow] = CLTDetailBase::GetNew(INVENTORY, this, m_pTransaction);
    if (!bResetOnSuccess) *m_pTransaction->m_arOrderedDetail[nRow] = *pLine;
    pLine->Delete();
    pLine = m_pTransaction->m_arOrderedDetail[nRow];

    CString szLocation = objDR.XMLStr(_T("strLocation"));
    CString szMLILocation = objDR.XMLStr(_T("strMLILocation"));
    CString szDescription = objDR.XMLStr(_T("strDescription"));
    double  dBuyConversion = objDR.XMLDbl(_T("decBuyConversion"));
    double  dLastCost = 0.0;
    CString szVendorStockNum = EMPTY_STRING; // begbert 06-17-2008 1-30104 : if there's no alternate vendor, clear that field.
    bool	bSerialized = SetBool(objDR.XMLStr(_T("bolSerializedFlag")));	// begbert 10-14-2008 1-31103 : Note whether the item is serialized.
    // InTimeTec: 2/28/2014 : HS : IN Lot Number Tracking
    bool	bLotNumbered = SetBool(objDR.XMLStr(_T("bolLotInventoryFlag")));
	bool	bUsingVariants = SetBool(objDR.XMLStr(_T("bolUseVariants")));
	CString szVariantCode = objDR.XMLStr(_T("strVariantCode"), false);

	if (bUsingVariants)
		szDescription = szVariantCode;

    // begbert 04-30-2008 1-29509 : GetINAltVendorInfo was moved and changed slightly, so this changes accordingly.
    // begbert 09-23-2008 1-29613 : Also, if the AltVendorInfo doesn't give a Last Cost default to the stock
    //   item's primary last cost; the code was rearranged to do this and be more efficient about it.
    if (pLine->GetINAltVendorInfo(objDR2, szStock, szLocation, szMLILocation))
    {
        // begbert 04-01-2009 1-32444 : We want the _Unlanded_ Last Cost, which
        //    is in the _curUnitCost_ field of the Alternate Vendor Record.
        dLastCost = objDR2.XMLDbl(_T("curUnitCost"));
        szVendorStockNum = objDR2.XMLStr(_T("strVendorStock"));
        objDR2.Dispose();
    }
    if (dLastCost == 0.0) dLastCost = objDR.XMLDbl(_T("curLastCost"));
    if (dBuyConversion == 0.0) dBuyConversion = 1.0;	// begbert 09-23-2008 : paranoia check
    dLastCost /= dBuyConversion;

    pLine->SetColumnValueString(StockNumber, szStock);
    pLine->SetColumnValueString(Location, szLocation);
    pLine->SetColumnValueString(MLILocation, szMLILocation);
    pLine->SetColumnValueString(Description, szDescription);
    pLine->SetColumnValueDouble(BuyConversion, dBuyConversion);	// begbert 05-21-2008 1-29925 : Added Buy Conversion scaling for costs
    pLine->SetColumnValueDouble(LastCost, dLastCost);
    pLine->SetColumnValueDouble(UnitCost, dLastCost);
    pLine->SetColumnValueString(VendorStockNum, szVendorStockNum);
    pLine->SetSerialized(bSerialized);	// begbert 10-14-2008 1-31103 : Note whether the item is serialized.
    // InTimeTec: 2/28/2014 : HS : IN Lot Number Tracking
    pLine->SetLotNumbered(bLotNumbered);
	pLine->SetUsingVariants(bUsingVariants);

	if (pLine->IsUsingVariants() && szVariantCode.IsEmpty())
	{
		// Go fetch the first listed variant to set as the description.
		CDataSet     objDS;
		CXMLDocument xmlDoc(L"ROOT");
		CXMLElement	 xmlRoot = xmlDoc.GetDocumentElement();
		
		xmlRoot.AddNewChild(L"strStock", szStock);
		xmlRoot.AddNewChild(L"strMLILocation", szMLILocation);
		xmlRoot.AddNewChild(L"strLocation", szLocation);
	
		CCMSDataAdapter	objDA(L"SPECIALPROCESS", L"IN", L"GETSTOCKVARIANTS");
		if (objDA.GetData(xmlDoc, objDS) != rcSuccess)
			return false;

		// We only need the first one.
		if (objDS.TableExists(L"Table"))
		{
			CDataTable	objDT;
			objDS.GetTable(L"Table", &objDT);
			if (objDT.getRowCount() > 0)
			{
				CDataRow	objDR;
				objDT.GetRow(0, &objDR);
				pLine->SetColumnValueString(Description, objDR.XMLStr(L"strVariantCode"));
			}
		}
	}

    m_pCtrlHelper->UpdateVisibleCtrlText();

    ManageExtDescriptions(nRow, pLine, objDR);

    pLine->SetPictureInfo(objDR);

    objDR.Dispose();

    return true;
}

// begbert 10-14-2008 1-31110 : Validate the data and get descriptions for autofilled direct expense lines.
bool CGrdPurDetail::ValidateGLAccount(long nRow, CString p_szGLAccount, bool bResetOnSuccess /*=true*/)
{
    CDataRow objDR, objDR2;

    // 1-31795 BK 1/20/09 changed to IsGLIntegrated() function returns true if  GeneralLedger.IsUsable() and integration level 1 - 9 or 101 - 104 (all but report only settings)
    if (!CGBLModuleIntegrationHelper::IsGLIntegrated())
        //if (!si->ModulesInstalled.GeneralLedger.IsUsable || si->ModuleSettings.CurrentSettings->GetGLIntegration() <= 0)
        return true; // If we're not integrated, then everything passes.

    CLTDetailBase* pLine = m_pTransaction->m_arOrderedDetail[nRow];

    if (!pLine->GetGLAccount(objDR, p_szGLAccount)) return false;

    CString szAccount = objDR.XMLStr(_T("strAccount"));

    // 1-32088 BK 3/3/09 when more than 1 segment display segment separator
    if (CMS::GetSI()->SegmentSetup.NumberOfSegments > 1)
        CGBLForm::AddSegSeparators(szAccount);

    // begbert 08-27-2008 1-30131 : clear out the old to make way for the new, if they didn't say not to
    // begbert 11-24-2008 1-31725 : Now take care to delete the old line, making a new one in its place
    //   which gets a copy of the old line's values, if they didn't say not to.
    m_pTransaction->m_arOrderedDetail[nRow] = CLTDetailBase::GetNew(pLine->WhatAmI(), this, m_pTransaction);
    if (!bResetOnSuccess) *m_pTransaction->m_arOrderedDetail[nRow] = *pLine;
    pLine->Delete();
    pLine = m_pTransaction->m_arOrderedDetail[nRow];

    CString szDescription = objDR.XMLStr(_T("strDescription"));

    pLine->SetColumnValueString(GLAccount, szAccount);
    pLine->SetColumnValueString(Description, szDescription);

    m_pCtrlHelper->UpdateVisibleCtrlText();

    objDR.Dispose();

    return true;
}

bool CGrdPurDetail::ValidateSuperceded(CLTDetailBase* pLine, CString& szSuperseded, const CString& szOriginalStock)
{
    ASSERT(!szSuperseded.IsEmpty());	// This shouldn't happen; just checking.

    CDataRow objDR;

    if (szSuperseded == szOriginalStock || !pLine->GetInventoryItem(objDR, szSuperseded))
        return false;

    CString szTemp = objDR.XMLStr(_T("strSupersededStock"));

    objDR.Dispose();

    if (szTemp.IsEmpty())
        return true;

    szSuperseded = szTemp;
    // 1-35723 BK 5/3/11 function was calling itself when a selected stock contained a superseded stock
    //return ValidateSuperceded(pLine, szSuperseded, szOriginalStock);
    return true;
}

// begbert 01-15-2007 1-28835 : This function manages the Ext. Description comment lines.
void CGrdPurDetail::ManageExtDescriptions(long nRow, CLTDetailBase* pLine, CDataRow& objDR)
{
    int nUseInPO = objDR.XMLByte(_T("bytUseInPO")); // 0=Don't use, 1=Use, 2=Prompt

    // begbert 10-14-2008 : added this and subsequent Refresh()es to clear out any old
    //   extended description prior to adding the new extended description (if any).
    DeleteComment(nRow, false);

    if (nUseInPO == 0) { Refresh(); return; }	// Don't use

    const CString szUse = _T("1"), szNl = _T("\n");
    CString szExtDesc = EMPTY_STRING;
    CString szLinesInPO = objDR.XMLStr(_T("strLinesInPO"));
    if (szLinesInPO.Mid(0, 1) == szUse) szExtDesc += szNl + objDR.XMLStr(_T("strDescription1")).Trim();
    if (szLinesInPO.Mid(1, 1) == szUse) szExtDesc += szNl + objDR.XMLStr(_T("strDescription2")).Trim();
    if (szLinesInPO.Mid(2, 1) == szUse) szExtDesc += szNl + objDR.XMLStr(_T("strDescription3")).Trim();
    if (szLinesInPO.Mid(3, 1) == szUse) szExtDesc += szNl + objDR.XMLStr(_T("strDescription4")).Trim();
    szExtDesc = szExtDesc.Mid(1);	// clip off initial \n

    if (nUseInPO == 2)	// Prompt
    {
        CString szMsg = RESSTRING(IDS_SAE_USEEXTDESC_MSG);
        CString szCaption = RESSTRING(IDS_SAE_USEEXTDESC_CAPTION);
        szMsg.Replace(_T("<STOCK>"), objDR.XMLStr(_T("strStock")));
        szMsg.Replace(_T("<EXTDESC>"), szExtDesc);
        if (CGBLForm::CMSMessageBox(AfxGetMainWnd(), 0, 0, MB_YESNO | MB_ICONQUESTION, szMsg, szCaption, 0, false) != IDYES)
        {
            Refresh(); return;
        }
    }

    InsertComment(nRow + 1, szExtDesc, pLine);
    Refresh();
}

//---------------------------------------------------------------------------
void CGrdPurDetail::UpdateExtendedCost(long nRow, bool bFromExtended)
{
    if (m_pTransaction->m_arOrderedDetail[nRow]->WhatAmI() == COMMENT) return;
    CLTDetailBase* pLine = m_pTransaction->m_arOrderedDetail[nRow];

    COLPROPS colprops;
    double dQuantity = pLine->GetColumnValueDouble(QuantityOrdered);

    if (bFromExtended && dQuantity != 0.0)	// calculate unit cost from extended
    {
        // begbert 05-09-2008 : don't let this back-calculate a negative into cost; instead negate Quantity if necessary
        // begbert 09-29-2008 1-31002 : Corrected the negative check to hinge on 0 not 1, and
        //   adjusted the back-calculation of Unit Cost to floor off any undisplayed fractional
        //   part, lest it round up and raise the ExtendedCost above what was entered.
        double fracfactor = pow(10.0, GetColProps(UnitCost, colprops) ? colprops.nFractionalDigitCount : 4);
        double dExtended = pLine->GetColumnValueDouble(ExtendedCost);
        pLine->SetColumnValueDouble(UnitCost, abs(floor(dExtended / dQuantity * fracfactor) / fracfactor));
        if (dQuantity * dExtended < 0)
            pLine->SetColumnValueDouble(QuantityOrdered, dQuantity = -dQuantity);
    }

    // begbert 06-12-2008 : when a line is made negative, the posting-related checkboxes clear.
    if (dQuantity < 0)
    {
        pLine->SetColumnValueBool(IsLandItem, false);
        pLine->SetColumnValueBool(IsDiscount, false);
        pLine->SetColumnValueBool(IsApplyFreight, false);
    }
    else {
        // begbert 03-12-2009 1-32346 : if the landing method is autodistribute, turn it all on.
        pLine->ApplyAutodistributionToLanding();
    }

    // calculate extended from unit cost and quantity
    double dUnitCost = pLine->GetColumnValueDouble(UnitCost);

    // begbert 10-02-2008 1-31002 : Check whether the Extended Cost is larger than
    //   the cell's edit control is configured to hold - if so, reduce the Quantity
    //   to something that does not result in overflowing the control.
    int digitplaces = 9;
    int fracplaces = 2;
    if (GetColProps(ExtendedCost, colprops))
    {
        digitplaces = colprops.nDecimalDigitCount;
        fracplaces = colprops.nFractionalDigitCount;
    }
    CString szMaxVal = (digitplaces > 0 ? CString(_T('9'), digitplaces) : _T("0")) +
        (fracplaces > 0 ? "." + CString(_T('9'), fracplaces) : _T(""));
    CCMSDouble dMaxVal(szMaxVal);
    if (abs(dUnitCost * dQuantity) > dMaxVal)
    {
        double fracfactor = pow(10.0, GetColProps(QuantityOrdered, colprops) ? colprops.nFractionalDigitCount : 4);
        dQuantity = floor(dMaxVal / dUnitCost * fracfactor) / fracfactor * ((dQuantity < 0) ? -1.0 : 1.0); // begbert 10-22-2008 : retained negative
        pLine->SetColumnValueDouble(QuantityOrdered, dQuantity);
    }

    pLine->SetColumnValueDouble(ExtendedCost, dUnitCost * dQuantity);
}

void CGrdPurDetail::UpdateQuantityRemaining(long nRow)
{
    if (m_pTransaction->m_arOrderedDetail[nRow]->WhatAmI() == COMMENT) return;
    CLTDetailBase* pLine = m_pTransaction->m_arOrderedDetail[nRow];

    double dOrdered = pLine->GetColumnValueDouble(QuantityOrdered);
    double dPosted = pLine->GetColumnValueDouble(QuantityReceivedPosted);

    // begbert 06-06-2008 : While we're at it, verify that dOrdered hasn't been given an invalid value.
    if (dPosted != 0.0 &&			// if nothing has been posted we don't care what they did to dOrdered.
        dOrdered / dPosted < 1.0)	// checks whether dOrdered is closer to zero or has different negativity.
    {
        dOrdered = dPosted;
        pLine->SetColumnValueDouble(QuantityOrdered, dOrdered);
    }

    pLine->SetColumnValueDouble(QuantityRemaining, dOrdered - dPosted);
}

//---------------------------------------------------------------------------
//Validation for grid autofill controls
int CGrdPurDetail::OnEditFinish(int col, long row, CWnd* edit, LPCTSTR string, BOOL cancelFlag)
{
    // begbert 02-04-2009 1-31940 : Added this flag check to enable other
    //   functions to (temporarily) disable the activities of this function,
    //   presumably for the duration of some other operation.
    if (m_bDisableOnEditFinishHandling) return FALSE;

    UNREFERENCED_PARAMETER(cancelFlag);
    UNREFERENCED_PARAMETER(edit);

    // begbert 04-30-2008 : avoid crashes if this has been cleared out.
    if (row < 0 || row >= m_pTransaction->m_arOrderedDetail.GetCount())
        return FALSE;	// Happens on closing.

    BOOL bValidate = FALSE;
    BOOL bValidateUsingNoInfoForm = FALSE;
    INT32 nColumnID = GetDisplayedColumnType(col, row);

    UINT    nLookupID = 0;
    UINT	nModuleID = IDS_PO_MODULE;
    CString szFilter = EMPTY_STRING;
    CString szCaption = QuickGetText(col, -1);

    CStringArray szErrorMsg;
    szErrorMsg.Add(szCaption);
    szErrorMsg.Add(string);
    szErrorMsg.Add(szCaption);
    szErrorMsg.Add(string);

    // begbert 08-19-2008 1-30763 : if the value was not changed, don't do anything further,
    //   to save on server calls and wasted work.
    // begbert 01-19-2009 1-31775 : If the column is the StockNumber or GLAccount, and is blank, make
    //   sure not to exit here - we may need to deal with the line even if it's unchanged, to delete it.
    CLTDetailBase* pLine = m_pTransaction->m_arOrderedDetail[row];
    if (pLine && string == pLine->GetColumnValueString(nColumnID) &&	// begbert 10-27-2008 : removed erroneous FilterUDFids()
        ((nColumnID != StockNumber && nColumnID != GLAccount) || lstrlen(string) != 0))
        return TRUE;

    switch (FilterUDFids(nColumnID))
    {
    case StockNumber:
    case GLAccount:

        // begbert 08-19-2008 1-30763 : If these columns are set to blank, delete the line,
        //   unless it's the extra blank one at the end.
        // begbert 02-24-2009 1-32092 : a completely whitespace string counts as blank.
        if (lstrlen(CString(string).Trim()) == 0)
        {
            if (pLine && pLine->WhatAmI() != UNKNOWN)
            {
                m_pCtrlHelper->HideVisibleCtrl(false);
                DeleteRow(row);
                return FALSE;
            }
            return true;
        }
        else
            bValidateUsingNoInfoForm = true;
        break;

	case Description:
		if (pLine->IsUsingVariants() && string != pLine->GetColumnValueString(Description))
		{
			if (lstrlen(string) == 0)
			{
				edit->SetWindowText(pLine->GetColumnValueString(Description));
				CGBLForm::ShowCriticalMsg(L"Description cannot be blank.", L"Error");
				return FALSE;
			}
		}
		break;

    case QuantityOrdered:

        // begbert 10-14-2008 1-31103 : Quantities above 100,000 are not valid on serialized items.
        // begbert 12-04-2008 1-31420 : The limit is now 999,999.
        if (pLine->IsSerialized())
        {
            CCMSDouble dValue(string);
            if (dValue > 999999.0 || dValue < -999999.0)
            {
                edit->SetWindowTextW(dValue > 0.0 ? _T("999999.0") : _T("-999999.0"));
                CGBLForm::CMSMessageBox(this, IDS_PO_MAX_SERIALIZED_QTY_EXCEEDED_MSG, IDS_PO_MAX_SERIALIZED_QTY_EXCEEDED, MB_OK | MB_ICONEXCLAMATION);
                return FALSE;
            }
        }
        break;

    case UDF:
        OnEditFinishUDF(nColumnID, row, string, nLookupID, nModuleID, szFilter);
        bValidate = nLookupID != 0;
        break;
    }

    // ensure valid data is entered
    if (bValidate && lstrlen(string) > 0)
    {
        if (!m_pAutoFillEditControl->IsDataValid(string))
        {
            m_pAutoFillEditControl->SetWindowText(EMPTY_STRING);
            if (CGBLForm::CMSMessageBox(this, IDS_PO_INVALID_FIELD_VALUE_MSG, IDS_INVALID_FIELD_VALUE, MB_OKCANCEL | MB_ICONEXCLAMATION, &szErrorMsg) == IDOK)
                ShowLookup(nLookupID, nModuleID, szFilter);
            return FALSE;
        }
    }

    /*	// begbert 08-27-2008 : removed this because it is done better elsewhere and was incorrectly handling non-inventory lines.

        // begbert 04-09-2008 1-28813 : Pre-emptively handle invalid data in the kjey field,
        if (bValidateUsingNoInfoForm)
        {
            CDataRow objDR;	// begbert 07-24-2008 1-30267 : switched the validation function to one that accounts for aliases.
            // begbert 08-19-2008 1-30763 : Moved the declaration of pLine earlier, and removed an erroneous DeleteRow().
            if (pLine &&
                !pLine->GetInventoryItem(objDR, string) )
    //			!m_pAutoFillEditControl->IsDataValid(string) )
            {
                ShowNoInfoDetectedDlg(pLine, string);
                return FALSE;
            }
        }
    */
    if (lstrlen(string) > 0 && row == GetNumberRows() - 1)
    {
        CLTDetailBase* pLine = m_pTransaction->m_arOrderedDetail[row];
        if (pLine && pLine->WhatAmI() == UNKNOWN)
        {
            // begbert 10-23-2008 1-31104 : Make sure there grid isn't already at its limit before adding.
            if (!ValidateDetailCount()) return FALSE;

            BASETYPE nLineType;
            // begbert 10-14-2008 1-31110 : if IN is not integrated then entering
            //   text in an UNKNOWN row creates a Direct Expense row.
            if (si->ModulesInstalled.Inventory.IsUsable && si->ModuleSettings.CurrentSettings->GetINIntegration() > 0)
                nLineType = INVENTORY;
            else
                nLineType = DIRECTEXPENSE;

            m_pTransaction->m_arOrderedDetail[row] = CLTDetailBase::GetNew(nLineType, this, m_pTransaction);
            m_pTransaction->AddDetailLine(pLine);
            SetNumberRows(row + 2, TRUE, row);	// begbert 11-18-2008 1-31421 : Pass in the row at which to insert elements
            RedrawAll();
        }
    }

    // begbert 04-06-2009 1-32338 : Update pLine; the above if statement may have created a new one.
    pLine = m_pTransaction->m_arOrderedDetail[row];
    REFERENCE_KEY key = pLine->GetReferenceKey();

    UpdateTransaction(row, nColumnID, string);

    // begbert 04-06-2009 1-32338 : UpdateTransaction can fail to set the value, or set it to
    //   something else; if it did, call OnEditFinish again.  (Assuming the line still exists.)
    pLine = m_pTransaction->GetDetailLine(key.szID);
    if (pLine && pLine->GetRow() == row)
    {
        CString szString2 = pLine->GetColumnValueString(nColumnID);
        if (szString2 != string)
            OnEditFinish(col, row, edit, szString2, cancelFlag);
    }

    return __super::OnEditFinish(col, row, edit, string, cancelFlag);
}

LRESULT CGrdPurDetail::OnGridLookupEnd(WPARAM wparam, LPARAM lparam)
{
	//PTR.06.02.2005.1-17846
	switch (GetColumnID(GetCurrentCol()))
	{
		// begbert 02-22-2008 1-28932 : Populate the stock column from the description
		//   lookup info, so the usual stock fetch will do what we need to get done.
	case Description:
	{
		CXMLElement* eleRoot = (CXMLElement*)lparam;
		if (eleRoot->ElementExists(_T("strStock")))
		{
			const REFERENCE_KEY rk = m_pTransaction->m_arOrderedDetail.GetAt(GetCurrentRow());
			CLTPurDetailInventory * const pLine = DYNAMIC_DOWNCAST(CLTPurDetailInventory, m_pTransaction->GetDetailLine(rk.szID));
			if (!pLine) break;

			// begbert 04/03/2019 Bug 45242 : If changing the description returned a different stock number or location, update the row based on the new value.
			CString szStock = eleRoot->GetItem(_T("strStock")).GetInnerText();
			CString szLocation = eleRoot->GetItem(_T("strMLILocation")).GetInnerText();

			if (szStock != pLine->GetColumnValueString(StockNumber))
			{
				pLine->SetColumnValueString(Location, szLocation);
				if (!SetCellValueFromEntry(GetCurrentRow(), GetDisplayedColumnIndex(StockNumber), szStock, INVENTORY))
					return -1;
			}
			else if (szLocation != pLine->GetColumnValueString(MLILocation))
			{
				if (!SetCellValueFromEntry(GetCurrentRow(), GetDisplayedColumnIndex(Location), szLocation, INVENTORY))
					return -1;
			}
		}
	}
	break;
	}
	return 0;
}

// begbert 10-23-2008 1-31104 : This returns true if the number of limit-applicable lines
//   is *less* than the limit - that is, if there is room for more such lines to be added.
bool CGrdPurDetail::ValidateDetailCount()
{
    if (!m_pTransaction->ValidateInvDxpDetailCount())
    {
        PostMessage(POE_WM_DETAIL_LIMIT_REACHED, (WPARAM)m_pTransaction->GetDetailLineLimit(), 0);
        return false;
    }
    return true;
}

// begbert 10-24-2008 1-31104 : This message handler shows the 'detail maximum reached' warning box to the user.
LRESULT CGrdPurDetail::OnDetailLimitReached(WPARAM nLimit, LPARAM)
{
    CGBLForm::CMSMessageBox(this, IDS_PO_DETAIL_MAXIMUM_REACHED_MSG, IDS_PO_DETAIL_MAXIMUM_REACHED, MB_ICONEXCLAMATION | MB_OK);
    return 0;
}

void CGrdPurDetail::OnCheckChanged(INT32 nColID, long row, const CString& szValue)
{
    switch (nColID)
    {
    case IsDiscount: case IsTaxable:
        DoCalculations(nColID, row);
        break;
    }
}

void CGrdPurDetail::OnMenuCommand(int col, long row, int section, int item)
{
    if (item > POE_WM_OFFSET)//PGP(11-18-2007)
        OnCMSMenuCommand(col, row, item);
}

int CGrdPurDetail::OnCMSMenuCommand(int nCol, long lRow, int nItem)
{
    //	if (!m_pEntry || m_pEntry->GetIsInitializing())
    //		return 0;

        // begbert 02-26-2009 1-32136 : Hide the control, so that any changes made to it are
        //   applied to the row, and if that removes the row, it happens before we try to get it.
    m_pCtrlHelper->HideVisibleCtrl();

    CLTDetailBase* pLTBase = (CLTDetailBase*)GetRow(lRow);
    //	CLTBase* pLTCurrent = pLTBase;
    if (pLTBase)
    {
        while (pLTBase->HasParent())
        {
            REFERENCE_KEY rkP = pLTBase->GetParentObject();
            pLTBase = m_pTransaction->GetDetailLine(rkP.szID);
            lRow = m_pTransaction->GetPosition(rkP);
        }

        int nColumnID = GetColumnID(nCol);
        enuOnMenuCmdResults eResult = pLTBase ? pLTBase->OnMenuCommand(nColumnID, nItem) : OnMenu_OK;	//sb - 23-dec-05 - pLTBase can be null.
        if (eResult == OnMenu_OK)
            return TRUE;
    }
    bool bProcessHandle = true;

    NMHDR nmHdr;
    nmHdr.hwndFrom = this->m_hWnd;
    nmHdr.code = 0;
    nmHdr.idFrom = (UINT_PTR)lRow;
    //	CWnd *pOwner = m_pEntry;

    m_pCtrlHelper->HideVisibleCtrl();

    // begbert 10-06-2008 : prevent most rght-click operations if the PO is read-only or the line is locked.
    switch (nItem)
    {
    case POE_DBG_VIEWOBJECTINFO: break;
    case POE_COPY_ROW: case POE_SAVE_AND_PRINT_PO: case POE_SAVE_PO: if (pLTBase && pLTBase->IsLineLocked()) return FALSE;
    default: if (!m_pTransaction || m_pTransaction->IsReadOnly() || (pLTBase && pLTBase->IsLineLocked())) return FALSE;
    }

    switch (nItem)
    {
    case POE_DBG_VIEWOBJECTINFO:
        // runtime debugging information for object
        bProcessHandle = false;
        {
            //			CPSTransInfo psdlg(_T("Object Information"));
            //			psdlg.m_pTransaction = m_pTransaction;
            //			REFERENCE_KEY rkO = m_pTransaction->m_arOrderedDetail.GetAt(lRow);
            //			psdlg.m_ppObjectSettings.m_pTransaction = m_pTransaction;
            //			psdlg.m_ppObjectSettings.rkO = rkO;
            //			psdlg.DoModal();
        }
        break;

    case POE_INS_INVENTORY_B4:
    {
        //PTR.04.14.2005.1-17816
        if (!CanLeaveRow(lRow, nCol))
            return FALSE;
        // begbert 10-23-2008 1-31104 : Make sure there grid isn't already at its limit before adding.
        if (!ValidateDetailCount()) return FALSE;
        InsertRow(lRow, INVENTORY, true);
        nCol = GetDisplayedColumnIndex(POEntryLayout::DetailItems::StockNumber);		//PTR.04.14.2005.1-17875
        if (nCol <= -1) break;
        GotoCell(nCol, lRow);
        UpdateCellControl(lRow, nCol);
        // begbert 04-22-2008 1-29496 : Switched to use the same lookup code as the regular lookups.
        CCMSGridAutoFillEdit* pCtrl = DYNAMIC_DOWNCAST(CCMSGridAutoFillEdit, m_pCtrlHelper->GetCtrl(nCol));
        if (pCtrl) pCtrl->ShowLookup();
    }
    break;
    case POE_INS_DIRECTEXPENSE_B4:
    {
        //PTR.04.14.2005.1-17816
        if (!CanLeaveRow(lRow, nCol))
            return FALSE;
        // begbert 10-23-2008 1-31104 : Make sure there grid isn't already at its limit before adding.
        if (!ValidateDetailCount()) return FALSE;

        InsertRow(lRow, DIRECTEXPENSE, true);
        nCol = GetDisplayedColumnIndex(POEntryLayout::DetailItems::GLAccount);		//PTR.04.14.2005.1-17875
        //PGP(11-18-2007) Check for valid col.
        if (nCol <= -1) break;
        GotoCell(nCol, lRow);

        // 1-32088 BK 3/3/09 changed to use the account control lookup
        CAccountControl* pCtrl = DYNAMIC_DOWNCAST(CAccountControl, m_pCtrlHelper->GetCtrl(nCol));
        if (pCtrl) pCtrl->LookupAccountMain();	// begbert 04-13-2011 1-35598 : Removed the 'show the fund dialog' parameter
    }
    break;
    case POE_INS_BLANKLINE_B4:
        //PTR.04.14.2005.1-17816
        if (!CanLeaveRow(lRow, nCol))
            return FALSE;
        InsertRow(lRow, BLANK, true);
        GotoCell(nCol, lRow);
        UpdateCellControl(lRow, nCol);
        break;

    case POE_EDIT_COMMENTS:
        EditComments(lRow);
        //At this point pLTBase is no longer valid.
        pLTBase = NULL;
        break;
    case POE_DELETE_COMMENTS:
        //OnDeleteComment();
        DeleteComment(lRow);
        UpdateFreightTaxDiscountLanding();	// begbert 04-10-2008 1-29436 : a line is gone; recalculate
        return TRUE;
        break;
    case POE_ADD_COMMENTCODE:
        ShowCommentCodeLookup(pLTBase);
        break;
    case POE_ADD_COMMENTS_BASE_B4:
        // added to handle transaction level comments
    {
        CLTDetailBase* pLTCurrent = pLTBase;
        ShowCommentDialog(pLTBase);
        lRow = m_pTransaction->GetDetailPosition(pLTCurrent->GetLineID());
        GotoCell(GetCurrentCol(), lRow);
    }
    break;

    case POE_CUT_ROW:
        // begbert 06-06-2008 : if it can't be deleted, it can't be cut.
        if (pLTBase->CanDeleteObject())
        {
            CopyToClipboard(lRow);
            DeleteRow(lRow);
            UpdateCellControl(lRow);
            UpdateFreightTaxDiscountLanding();	// begbert 04-29-2008 : a line is gone; recalculate
            // begbert 04-29-2008 1-29590 : set dirty flag when line is deleted
            CGBLForm::SendMessageToParent(this, WM_DIRTY);
        }
        break;
    case POE_COPY_ROW:
        CopyToClipboard(lRow);
        break;
    case POE_PASTE_ROW:
        if (m_pClipboardedLine != NULL)
        {
            // begbert 10-23-2008 1-31104 : Make sure there grid isn't already at its limit before adding non-comment lines.
            if (m_pClipboardedLine->WhatAmI() != COMMENT &&
                m_pClipboardedLine->WhatAmI() != BLANK &&
                !ValidateDetailCount()) return FALSE;
            if (!pLTBase->EnableMenuItem(POE_COPY_ROW) || pLTBase->WhatAmI() == UNKNOWN)
            {
                InsertRow(lRow, m_pClipboardedLine->WhatAmI(), true);
                //				UpdateCellControl(lRow, nCol);
            }
            PasteFromClipboard(lRow);
            UpdateFreightTaxDiscountLanding();	// begbert 04-10-2008 : a line's been added; recalculate
            // begbert 04-29-2008 1-29590 : set dirty flag when line is added
            CGBLForm::SendMessageToParent(this, WM_DIRTY);
        }
        break;
    case POE_DELETE_ROW:
        // begbert 06-06-2008 : Preemptively check whether the line is deleteable
        if (pLTBase->CanDeleteObject())
        {
            DeleteRow(lRow);
            UpdateCellControl(lRow);
            UpdateFreightTaxDiscountLanding();	// begbert 04-10-2008 1-29436 : a line is gone; recalculate
            // begbert 04-29-2008 1-29590 : set dirty flag when line is deleted
            CGBLForm::SendMessageToParent(this, WM_DIRTY);
            return TRUE;
        }
        break;

    case POE_SAVE_AND_PRINT_PO:
        //begbert 01-29-2009 1-31906 : Inform the FV that this item was clicked.
        ::SendMessage(GetParent()->GetParent()->m_hWnd, WM_COMMAND, MAKEWPARAM(IDC_POE_BTN_SAVE_AND_PRINT, BN_CLICKED), 0);//, (LPARAM)hApplyBtn);
        break;

    case POE_SAVE_PO:
        //begbert 01-29-2009 1-31906 : Inform the FV that this item was clicked.
        ::SendMessage(GetParent()->GetParent()->m_hWnd, WM_COMMAND, MAKEWPARAM(IDC_POE_BTN_SAVE, BN_CLICKED), 0);//, (LPARAM)hApplyBtn);
        break;

    }

    UpdateFreightTaxDiscountLanding();	// begbert 04-10-2008 1-29436 : several of the above actions can change the totals; recalculate

    //PGP(11/02/2005) - Check code for repaint issue.
    Invalidate();

    if (nmHdr.code)
        return TRUE;

    return FALSE;
}
/***************************************************
return UG_SUCCESS to allow the menu to appear
return 1 to not allow the menu to appear, this get's complicated
here because menu changes based on the line type
****************************************************/
int CGrdPurDetail::OnMenuStart(int col, long row, int section)
{
    //	if (m_pTransaction->GetRecalled() && !m_pTransaction->GetIsSalespersonAllowedEditRecalled())
    //		return FALSE;

        //PTR.07.05.2005.1-19912
        //CreateGridMenu(row, col);
        //return TRUE;
    return CreateGridMenu(row, col);
}
void CGrdPurDetail::GetContextMenu(CMenu* pPopup, long lRow, int nCol)
{
    AddMenuItem(10000, _T(""), pPopup);
    CreateGridMenu(lRow, nCol, pPopup);
}
int CGrdPurDetail::CreateGridMenu(long lRow, int nCol, CMenu* pPopup/* = NULL*/)
{
    //	BOOL	bAllowDLI = true;

    //	int		nDLIExisting = 0;
    //	BOOL	bAllowDelete = true;		// delete information
    //	BOOL	bIsParent = false;			// is this a parent object
    //	BOOL	bAllowComments = false;		// allow a comment

    //	BOOL	bAllowEditSerial = false;	// allow edit of serial numbers attached
    //	int		nChildren = 0;				// number of children of a specific type

    if (lRow < 0)
        return 0;


    // key state for debugging below..
    //bool bShift = GetKeyState(VK_SHIFT) < 0 ? true : false;
    //bool bControl = GetKeyState(VK_CONTROL) < 0 ? true : false;


    // get the object in question first
    REFERENCE_KEY refkey = m_pTransaction->m_arOrderedDetail.GetAt(lRow);
    CObject* pObj = m_pTransaction->GetDetailLine(refkey.szID);
    CLTBase* pLTBase = static_cast<CLTBase*>(pObj);
    CLTBase* pLTChild = pLTBase;	//PTR.06.03.2005.1-17894

    if (pLTBase->HasParent())
    {
        REFERENCE_KEY rkP = pLTBase->GetParentObject();
        pLTBase = (CLTBase*)m_pTransaction->GetDetailLine(rkP.szID);
    }

    //PGP(12-16-2007) Clear menu items in grid's cache and the object handed over here.
    if (pPopup)
        while (pPopup->DeleteMenu(0, MF_BYPOSITION) != FALSE);
    EmptyMenu();

    if (si->ApplicationSettings.DeveloperMode)
    {
        AddMenuItem(POE_DBG_VIEWOBJECTINFO, IDS_VIEWOBJECTINFO, pPopup, pLTBase);
        AddMenuItem(10000, _T(""), pPopup);
    }
    // begbert 10-14-2008 1-31110 : if IN isn't integrated, remove the option to add inventory lines.
    if (si->ModulesInstalled.Inventory.IsUsable && si->ModuleSettings.CurrentSettings->GetINIntegration() > 0)
        AddMenuItem(POE_INS_INVENTORY_B4, IDS_INS_INVENTORYITEM, pPopup, pLTBase);
    AddMenuItem(POE_INS_DIRECTEXPENSE_B4, IDS_INS_DIRECTEXPENSEITEM, pPopup, pLTBase);
    AddMenuItem(POE_INS_BLANKLINE_B4, IDS_INS_BLANKLINE, pPopup, pLTBase);
    AddMenuItem(10000, _T(""), pPopup);

    AddMenuItem(POE_EDIT_COMMENTS, IDS_EDIT_COMMENT, pPopup, pLTBase);
    AddMenuItem(POE_DELETE_COMMENTS, IDS_DELETE_COMMENT, pPopup, pLTBase);
    AddMenuItem(POE_ADD_COMMENTS_BASE_B4, IDS_INSERT_COMMENT, pPopup, pLTBase);
    AddMenuItem(POE_ADD_COMMENTCODE, IDS_INSERTCOMMENTCODE, pPopup, pLTBase);
    AddMenuItem(10000, _T(""), pPopup);

    AddMenuItem(POE_CUT_ROW, IDS_CUTROW, pPopup, pLTChild);
    AddMenuItem(POE_COPY_ROW, IDS_COPYROW, pPopup, pLTChild);
    AddMenuItem(POE_PASTE_ROW, IDS_PASTEROW, pPopup, pLTChild);
    AddMenuItem(POE_DELETE_ROW, IDS_MENU_GRID_DELETE_ROW, pPopup, pLTBase);

    // begbert 10-30-2008 1-31263 : check against the module settings, to see if this
    //   purchasing grid is being shown from the Receiving modules's add items form.
    if (CMS::GetSI()->ModuleSettings.POSettings.Module == IDS_PUR_MODULE)
    {
        AddMenuItem(10000, _T(""), pPopup);

        AddMenuItem(POE_SAVE_AND_PRINT_PO, IDS_PO_SAVEANDPRINTPO, pPopup, pLTBase);
        AddMenuItem(POE_SAVE_PO, IDS_SAVE_PURCHASE_ORDER, pPopup, pLTBase);
    }
    return TRUE;
}

//---------------------------------------------------------------------------
// 1-19274 BK 5/16/05 don't allow user to resize grid title
int CGrdPurDetail::OnCanSizeTopHdg()
{
    return FALSE;
}

bool CGrdPurDetail::OnGridKeyDown(UINT *vcKey, BOOL processed)
{
    if (m_pTransaction->IsReadOnly())
        return __super::OnGridKeyDown(vcKey, processed);

    int nColumnID = GetColumnID(GetCurrentCol());

    if (nColumnID == POEntryLayout::DetailItems::StockNumber &&
        GetKeyState(VK_MENU) < 0)
    {
        CLTDetailBase* pLTBase = m_pTransaction->m_arOrderedDetail[GetCurrentRow()];
        if (GetCellStyle(GetCurrentCol(), GetCurrentRow()) & CMSCS_EDITABLE)
        {
            switch (*vcKey)
            {
            case VK_F5:
                //Comment Code Lookup
                m_pCtrlHelper->HideVisibleCtrl();
                ShowCommentCodeLookup(pLTBase);
                return true;
            case VK_F6:
                //Comment
                m_pCtrlHelper->HideVisibleCtrl();
                m_lLookupRow = GetCurrentRow();
                m_nLookupCol = GetCurrentCol();
                ShowCommentDialog(pLTBase);
                return true;
            case VK_F7:
                //Direct Expense Lookup
                m_pCtrlHelper->HideVisibleCtrl();
                Invalidate();
                // 1-33281 BK 8/25/09 only show lookup if GL is integrated
                if (CGBLModuleIntegrationHelper::IsGLIntegrated())
                    ShowDirectExpenseLookup(pLTBase);
                else
                    // 1-33281 BK 8/25/09 when not integrated insert direct expense line
                    OnCMSMenuCommand(GetCurrentCol(), GetCurrentRow(), POE_INS_DIRECTEXPENSE_B4);
                return true;
            }
        }
    }
    return __super::OnGridKeyDown(vcKey, processed);
}


void CGrdPurDetail::ShowCommentCodeLookup(CLTDetailBase* pLTBase)
{
    CGBLFormWrap clsWrapper;
    //PTR.04.12.2005.1-16696
    m_uiActiveLookup = LKP_PO_COMMENTCODES;
    m_lLookupRow = GetCurrentRow();
    m_nLookupCol = GetCurrentCol();
    m_pLookup = clsWrapper.ShowLookup(
        LKP_PO_COMMENTCODES,
        IDS_PO_MODULE,
        true,
        true, &m_clsXMLParams,
        _T(""),
        IDS_PO_MODULE,
        false);
    m_pLookup->Subscribe(this); //subcribe the lookup so that when the lookup goes way, the Notify function
                                // will invoke
}


void CGrdPurDetail::AddCommentsFromParser(long lRow, CLTBase* pParent/* = NULL*/)
{
    if (lRow < 0)
        lRow = 0;
    int lOffset = 0;

    // begbert 03-02-2009 1-32202 : Do not allow comments with overlong strings to be added.
    if (!si->ModuleSettings.POSettings.bAutoFormatComment)
    {
        for (int nIndex = 0; nIndex < m_clsParser.GetCommentLineCount(); nIndex++)
        {
            CString szComment(_T("")), szSeparator(_T(""));
            m_clsParser.GetCommentLineAt(nIndex, szComment, szSeparator);
            if (szComment.GetLength() > 50)
            {
                CGBLForm::ShowValidationErrorNM(this, IDS_COMMENTTOOLONGMSG, IDS_COMMENTTOOLONGCAPTION);
                return;
            }
        }
    }

    for (int nLoop = 0; nLoop < m_clsParser.GetCommentLineCount(); nLoop++, lRow++)
    {
        //1-21804 BK 9/22/06 set dirty flag when comment line is added
        CGBLForm::SendMessageToParent(this, WM_DIRTY);
        CString szComment(_T("")), szSeparator(_T(""));
        m_clsParser.GetCommentLineAt(nLoop, szComment, szSeparator);
        if (szSeparator.IsEmpty())	// begbert 10-09-2008 : prevent lines from running into one another.
            szSeparator = "\r\n";

        // begbert 08-14-2008 : Unknown-type lines can't be parents.
        if (pParent && pParent->WhatAmI() != UNKNOWN)
        {
            lOffset = pParent->CalcOffsetInsert(COMMENT, BOTTOM);
            lRow = pParent->GetRow() + lOffset + 1;
        }
        // begbert 10-14-2008 1-31110 : added parameter to prevent
        //   the added row changing the key column header.
        CLTPurDetailComment* pLTComment = (CLTPurDetailComment*)InsertRow(lRow, COMMENT, false);
        pLTComment->SetComment(szComment);
        pLTComment->SetSeparator(szSeparator);
        if (pParent)
        {
            pLTComment->AttachToParent(pParent, lOffset);
        }
    }
    // begbert 07-24-2008 1-30405 : Removed an old erroneous Entry_GoRight() call.
}

void CGrdPurDetail::InsertComment(long lRow, CString szComment, CLTDetailBase* pParent/* = NULL*/)
{
    m_clsParser.ParseComment(szComment);
    AddCommentsFromParser(lRow, pParent);
}

void CGrdPurDetail::InsertCommentCode(long lRow, CString szCommentCode)
{
    InsertComment(lRow, CLTPurDetailComment::QueryCommentCode(szCommentCode));	// begbert 10-09-2008 1-30080 : don't add comment codes as children
}

void CGrdPurDetail::ShowCommentDialog(CLTDetailBase* pLTBase)
{
    // begbert 08-14-2008 1-30688 : Replaced the call to EditComments with a
    //   truncated version thereof that does not include the existing lines.
    // begbert 10-09-2008 : further altered the code such that the lines that
    //   this adds are not added as children of the line, but in fact are
    //   inserted above the line that was clicked on.
    int lRow = m_pTransaction->GetPosition(pLTBase->GetReferenceKey());

    m_clsParser.Reset();

    CDlgCommentEditor	dlg(m_clsParser);
    if (dlg.DoModal() == IDOK)
    {
        AddCommentsFromParser(lRow);
    }
    else {
        // begbert 10-07-2008 1-31069 : If the dialog is canceled, restore the control that had focus before.
        m_pCtrlHelper->ShowCtrl(GetCurrentCol(), GetCurrentRow(), true);
    }
    // update the grid
    Refresh();	// begbert 10-09-2008 : Was RedrawAll(), which was incorrect.
}

// Finds the consecutive range of comments that includes the given line,
//   or if the line isn't the comment but is part of a group with comments,
//   it returns the first range of comments that are in that group.
std::vector<CLTDetailBase*> CGrdPurDetail::FindCommentsFor(long lRow)
{
    std::vector<CLTDetailBase*> arCommentLines;

    CLTDetailBase* pLTLine = m_pTransaction->m_arOrderedDetail.GetAt(lRow);

    if (pLTLine->WhatAmI() != COMMENT)
    {
        if (pLTLine->HasChildren(COMMENT))	// if this has children comments, return those.
            return FindCommentsFor(m_pTransaction->GetDetailPosition(pLTLine->GetChildren(COMMENT)[0]));

        if (pLTLine->HasParent())	// If this has a parent, return its comments, if any.
            return FindCommentsFor(pLTLine->GetParentLine()->GetRow());

        return arCommentLines;	// If there is no comment in sight, return the empty array.
    }

    CCMSTransaction::TRange	oRange = m_pTransaction->GetParentContiguous(pLTLine->GetLineID());
    for (lRow = oRange.first; lRow <= oRange.second; lRow++)
    {
        arCommentLines.push_back(m_pTransaction->m_arOrderedDetail.GetAt(lRow));
    }
    return arCommentLines;
}

void CGrdPurDetail::DeleteComment(long lRow, bool bRefresh/* = true*/)
{
    std::vector<CLTDetailBase*> arCommentLines = FindCommentsFor(lRow);

    for (lRow = 0; lRow < (int)arCommentLines.size(); lRow++)
    {
        m_pTransaction->DeleteDetailLine(arCommentLines[(size_t)lRow]);
        // begbert 04-29-2008 1-29590 : set dirty flag when line is deleted
        CGBLForm::SendMessageToParent(this, WM_DIRTY);
    }

    if (bRefresh)
        Refresh(); // begbert 08-12-2008 : Was RedrawAll(), which was incorrect.
}
//Edit child comments.
void CGrdPurDetail::EditComments(long lRow)
{
    CLTBase* pLTRow = m_pTransaction->GetDetailLine(lRow);
    if (!pLTRow) return;	// not a valid line

    std::vector<CLTDetailBase*> arCommentLines = FindCommentsFor(lRow);

    if (!arCommentLines.empty())
    {
        pLTRow = arCommentLines[0]->GetParentLine();
        lRow = arCommentLines[0]->GetRow();	// If comments were found, point lRow (which will be the insertion point) to the top row of the comments we're editing to avoid the lines being added back below where they should be.
    }

    if (pLTRow && pLTRow->WhatAmI() == UNKNOWN)
        pLTRow = NULL;

    m_clsParser.Reset();
    for (size_t i = 0; i < arCommentLines.size(); i++)
    {
        CLTPurDetailComment* pLTComment = DYNAMIC_DOWNCAST(CLTPurDetailComment, arCommentLines[i]);
        m_clsParser.AddCommentLine(pLTComment->GetColumnValueString(Description), pLTComment->GetSeparator());
    }

    CDlgCommentEditor	dlg(m_clsParser);
    if (dlg.DoModal() == IDOK)
    {
        DeleteComment(lRow, false);

        AddCommentsFromParser(lRow, pLTRow);
    }
    // update the grid
    Refresh();	// begbert 10-09-2008 : Was RedrawAll(), which was incorrect.
}

// begbert 04-09-2008 1-29496 : removed ShowDirectExpenseLookup; added parameter to this.
void CGrdPurDetail::ShowDirectExpenseLookup(CLTDetailBase* pLTBase, const CString& szValue /*=EMPTY_STRING*/)
{
    CXMLDocument docInput(CMSStrings::XMLTags::Root);
    CXMLElement eleRoot = docInput.GetDocumentElement();
    CXMLElement eleKeyInfo = eleRoot.AddNewChild(CMSStrings::XMLTags::KeyInformation);
    eleKeyInfo.AddNewChild(_T("bytCoupon"), 0);
    eleKeyInfo.Dispose();

    if (pLTBase->WhatAmI() != UNKNOWN || !szValue.IsEmpty())
    {
        CXMLElement eleSearchFor = eleRoot.AddNewChild(CMSStrings::XMLTags::SearchFor);
        eleSearchFor.AddNewChild(_T("strAccount"), !szValue.IsEmpty() ? szValue : pLTBase->GetColumnValueString(GLAccount));
        eleSearchFor.Dispose();
    }
    eleRoot.Dispose();

    m_clsXMLParams.Dispose();
    m_clsXMLParams.SetXML(docInput.GetOuterXML());
    CGBLFormWrap clsFormWrap;
    m_uiActiveLookup = LKP_GL_ACCOUNT;
    m_lLookupRow = GetCurrentRow();
    m_nLookupCol = GetCurrentCol();
    CLookup* pLookup = clsFormWrap.ShowLookup(
        LKP_GL_ACCOUNT,
        IDS_GL_MODULE,
        true,
        true, &m_clsXMLParams,
        _T("LKP_GL_ACCOUNT_DEFAULT"), //_T("GL_Master"),
        IDS_GL_MODULE,
        false);
    pLookup->Subscribe(this);
    m_pLookup = pLookup;
}

// begbert 04-09-2008 1-28813 : This function shows the 'No Info Detected' dialog, and carries out the selected option.
// begbert 04-22-2008 1-29496 : Added the 'szValue' parameter, and cleaned up the code slightly.
// begbert 08-12-2008 1-30244 : It now returns whether the dialog was canceled or not.  (false = canceled; true = not)
bool CGrdPurDetail::ShowNoInfoDetectedDlg(CLTDetailBase* pLTBase, const CString& szValue)
{
    m_pCtrlHelper->UpdateVisibleCtrlText();

    // begbert 02-04-2009 1-31940 : Disable OnEditFinish while showing the modal
    //   dialog, so that it doesn't trash pLTBase when the dialog takes focus.
    m_bDisableOnEditFinishHandling = true;
    CDlgNoInfo dlg(szValue, m_pTransaction->GetBuyerSettings());
    if (dlg.DoModal() == IDCANCEL)
    {
        m_bDisableOnEditFinishHandling = false;
        return false;
    }
    m_bDisableOnEditFinishHandling = false;

    CXMLParams  clsRoot(true), clsPK(true);
    CGBLSystemInformation::stuLookupSelected LookupSelected;
    LookupSelected.FormID = 0;

    switch (dlg.GetReturn())
    {
    case ADDINV:
    {
        LookupSelected.FormID = IDD_STOCK;
        m_uiActiveLookup = LKP_IN_STOCKNUMBER_PURCH_POE; //LKK 6/2/08 1-29919 new lookup   // begbert 01-28-2009 1-31873 : Updated to _POE variant lookup

        clsPK.SetRoot(_T("PRIMARYKEY"));
        clsPK.MakeParam(_T("strStock"), szValue);
        clsPK.MakeParam(_T("strMLILocation"), _T(""));
    }
    break;
    case ADDDIREXP:
        // 1-31795 BK 1/20/09 changed to IsGLIntegrated() function returns true if  GeneralLedger.IsUsable() and integration level 1 - 9 or 101 - 104 (all but report only settings)
        if (CGBLModuleIntegrationHelper::IsGLIntegrated())
            // begbert 10-23-2008 1-31181 : If it's not integrated, insert the given string as a direct expense line.
            //if (si->ModulesInstalled.GeneralLedger.IsUsable && si->ModuleSettings.CurrentSettings->GetGLIntegration() > 0)
            ShowDirectExpenseLookup(pLTBase, szValue);
        else {
            int lRow = pLTBase->GetRow();
            DeleteRow(lRow);
            // begbert 08-25-2009 1-33285 : When InsertRow was selecting the row prior to
            //   the szValue being set into it, it was making the line vanish.  Moved the
            //   select to be done after the value has been set to avoid that problem.
            pLTBase = (CLTDetailBase*)InsertRow(lRow, DIRECTEXPENSE, false);
            m_pCtrlHelper->GetCtrl(GetDisplayedColumnIndex(GLAccount))->SetWindowText(szValue);
            Select(0, lRow);
            UpdateHeadings(lRow);
        }
        break;
    case ADDCCODE:
    {
        LookupSelected.FormID = IDD_POF_MFV_COMMENTCODES;
        m_uiActiveLookup = LKP_PO_COMMENTCODES;

        clsPK.SetRoot(_T("PRIMARYKEY"));
        clsPK.MakeParam(_T("strCommentCode"), szValue);
    }
    break;
    case ADDCOMMENT:
    {
        int lRow = pLTBase->GetRow();
        m_clsParser.Reset();
        m_clsParser.ParseComment(szValue);
        CDlgCommentEditor dlg(m_clsParser);
        if (dlg.DoModal() != IDOK)
            return false;
        MoveToNextCell();
        DeleteRow(lRow);
        AddCommentsFromParser(lRow);
        GotoRow(lRow + 1);
        // update the grid
        RedrawAll();
    }
    break;
    }

    if (!LookupSelected.FormID)
        return true;

    UINT uiMode = WM_ADDONFLY;

    clsRoot.SetRoot(_T("ROOT"));
    clsRoot.AppendXMLParam(&clsPK);
    m_lLookupRow = pLTBase->GetRow();
    m_nLookupCol = GetDisplayedColumnIndex(StockNumber);

    CGBLForm::SendMessageToMainFrame(WM_ADDONFLY, (WPARAM)&LookupSelected);

    CMaintFormView *pCurrentView = (CMaintFormView*)CGBLForm::EkGetActiveView();	//get view
//		*pCurrentView += this;		//sb - 1-14057 - subscribe to notifications of when the maint view is canceled so we can refresh
    pCurrentView->Subscribe(this);
    CCMSDoc* pDoc = (CCMSDoc*)pCurrentView->GetDocument();
    bool bMaintDoc = pDoc->IsMaintDoc();
    if (bMaintDoc)
        pDoc->Subscribe(this);

    if (uiMode == WM_VIEWMODIFY)	//check button mode
    {
        if (bMaintDoc)
        {
            CCMSMaintDoc* pMaintDoc = (CCMSMaintDoc*)pDoc;
            //				CString szXML = LookupSelected.XMLParams.GetXML();
            pMaintDoc->OnLookView(clsRoot.GetXML());
        }
        else
        {
            //				pCurrentView->m_clsXMLParams = LookupSelected.XMLParams;						//set prams for the get recored
            pCurrentView->m_clsXMLParams = clsRoot;
            pCurrentView->m_bAddOnTheFly = true;
            pCurrentView->OnSetFocusFromLookup(this);										//use method in cmsform view to set data tables
            pCurrentView->LookupGetRecord(NULL);												//call method from MaintFormView to get the record
            if (::IsWindow(pCurrentView->m_dlgCMSControlButtons.GetSafeHwnd())) pCurrentView->m_dlgCMSControlButtons.SetFocus();
        }
    }
    // Add mode
    else
    {
        if (bMaintDoc)
        {
            CCMSMaintDoc* pMaintDoc = (CCMSMaintDoc*)pDoc;
            pMaintDoc->OnLookAdd(szValue);
        }
        else
        {
            if (szValue.IsEmpty())
                pCurrentView->LookupGetRecord(NULL, true);
            else
            {
                CDataSet	clsDataSet;							//used for lookup
                CDataTable	clsTable;
                CDataRow	clsDataRow;
                CXMLParams	clsParams;

                clsDataSet.LoadFromXML(clsRoot.GetXMLBSTR());					//load cdataset
                clsDataSet.GetTable("PRIMARYKEY", &clsTable);						//get key value set
                clsTable.GetRow(0, &clsDataRow);
                clsDataRow.SetRowValue(0, szValue);
                pCurrentView->m_clsDataRow = clsDataRow;
                pCurrentView->LookupGetRecord(NULL);
            }
        }
    }
    GetParent()->SendMessage(CMS_MSG_SET_LKP_LINK, LinkFormNext, (LPARAM)pCurrentView->GetSafeHwnd());
    pCurrentView->GetParent()->SendMessage(CMS_MSG_SET_LKP_LINK, LinkLookupPrev, (LPARAM)m_hWnd);
    m_bLaunchedMaintForm = true;
    return true;
}


void CGrdPurDetail::Notify(CCMSMessage& objMessage)
{
    switch (objMessage.GetClassID())
    {
    case CCMSMaintSelectRecordID:
    {	// begbert 04-09-2008 1-28813 : added this case to handle direct-add-viewing of maintenence forms.
        CCMSMaintSelectRecord* pMsg = DYNAMIC_DOWNCAST(CCMSMaintSelectRecord, &objMessage);
        CXMLDocument objXMLDoc;
        pMsg->GetFormView().GetPKValues(objXMLDoc);
        pMsg->SetDoSelect();
        m_clsXMLParams.Dispose();
        m_clsXMLParams.SetXML(objXMLDoc.GetOuterXML());
        FillValuesFromLookup();
    }
    break;
    case CCMSMaintMessageID:
    {	// begbert 04-09-2008 1-28813 : added this case to handle direct-add-viewing of maintenence forms.
        CCMSMaintMsg* pMaintMsg = DYNAMIC_DOWNCAST(CCMSMaintMsg, &objMessage);
        if (pMaintMsg->GetMsgType() == MaintMsg_OnRecordToLookup)
        {
            CCMSMaintMsg_RecordToLookup* pMsg = DYNAMIC_DOWNCAST(CCMSMaintMsg_RecordToLookup, pMaintMsg);
            if (pMsg)
            {
                m_clsXMLParams.Dispose();
                m_clsXMLParams.SetXML(pMsg->GetSelectedRecord().GetOuterXML());
                FillValuesFromLookup();
                //					GetParent()->PostMessage(WM_CLOSE);
            }
        }
    }
    break;
    case CCMSLookupSelectID:
        FillValuesFromLookup();
    case CCMSLookupShutdownID:
        // begbert 10-07-2008 1-31069 : When a lookup is canceled, restore the control that had focus before.
        m_pCtrlHelper->ShowCtrl(GetCurrentCol(), GetCurrentRow(), true);
        PostMessage(WM_SETFOCUS);
        break;
    case CCMSPOEVendorPurchaseHistoryOKID:
    {
        CCMSPOEVendorPurchaseHistoryOKMsg* pVFHMsg = DYNAMIC_DOWNCAST(CCMSPOEVendorPurchaseHistoryOKMsg, &objMessage);
        ASSERT(pVFHMsg);
        OnVendorPurchaseHistoryOKMsg(pVFHMsg->pVPHDlg);
    }
    break;
    }
    __super::Notify(objMessage);
}

void CGrdPurDetail::FillValuesFromLookup()
{
    // begbert 04-09-2008 1-28813 : restored m_bLaunchedMaintForm and added handling for the 'BASETABLE' case
    if (m_pLookup == NULL && m_bLaunchedMaintForm == false)
        return;

    if (!m_clsXMLParams.ParamExists(_T("CANCELLED")) && !m_clsXMLParams.ParamExists(_T("PRIMARYKEY")))
        if (m_pLookup && IsWindow(m_pLookup->m_hWnd)) m_pLookup->GetParent()->PostMessage(WM_CLOSE, 0, 0);
    m_pLookup = NULL;

    if (m_clsXMLParams.ParamExists(_T("PRIMARYKEY")))
    {
        CXMLParams clsXMLData = m_clsXMLParams.GetXMLParams(_T("PRIMARYKEY"));
        UpdateObjectFromLookup(clsXMLData);
    }
    else if (m_clsXMLParams.ParamExists(_T("BASETABLE")))
    {
        UpdateObjectFromLookup(m_clsXMLParams);
    }
    m_bLaunchedMaintForm = false;
    m_clsXMLParams.Clear();
}

//Function to update the corresponding grid object properties from
//the xml returned from lookup.
//<TESTINGNOTES>: object properties might not be set correct.
//may need additional query or separate lookup defenitions to get all
//relevant data.
//TODO: Get information about mapping object properties to database fields.
//Ask Dave.G
//PGP(09/25/2003): Changed argument to CXMLParams, changed function calls within.
void CGrdPurDetail::UpdateObjectFromLookup(CXMLParams& clsXMLParams)
{
    if (m_lLookupRow >= 0 && m_lLookupRow <= m_pTransaction->m_arOrderedDetail.GetUpperBound())
    {
        //		CString szColumnID = m_arOrderedColumns.GetAt(m_nLookupCol);

        CString szText = _T("");
        BASETYPE eLineType = UNKNOWN;
        switch (m_uiActiveLookup)
        {
            //LKK 6/2/08 1-29919 new lookups
            // begbert 01-28-2009 1-31873 : Updated to _POE variant lookups
        case LKP_IN_STOCKNUMBER_PURCH_POE:
        case LKP_IN_STOCKNUMBER_MLI_PURCH_POE:
            eLineType = INVENTORY;
            szText = clsXMLParams.GetParamStr(_T("strStock"));
            break;
        case LKP_GL_ACCOUNT:
            szText = clsXMLParams.GetParamStr(_T("strAccount"));
            eLineType = DIRECTEXPENSE;
            break;
        case LKP_PO_COMMENTCODES:
            // begbert 04-09-2008 1-28813 : Added 'm_bLaunchedMaintForm' handling.
            if (m_bLaunchedMaintForm) DeleteRow(m_lLookupRow);
            InsertComment(m_lLookupRow, CLTPurDetailComment::QueryCommentCode(clsXMLParams.GetParamStr(_T("strCommentCode"))));	// begbert 10-09-2008 1-30080 : don't insert the comment codes as children.
            eLineType = UNKNOWN;	// don't do the generic handling
            break;
        default:
            // begbert 04-17-2008 : safe handler for invalid data
            eLineType = UNKNOWN;	// don't do the generic handling
            break;
        }
        if (eLineType != UNKNOWN)
        {
            // begbert 07-21-2008 1-30251 : Clear the cell to be set, to force a fetch of the looked-up record (in case it was altered).
            // begbert 08-12-2008 1-30244 : But back up the prior value and restore it (and do nothing else) if the value is not updated for some reason.
            // begbert 11-24-2008 1-31469 : Added a parameter to force a fetch without having to trick it, so we don't have to clear the cell.
            CString szOldValue = m_pTransaction->m_arOrderedDetail[m_lLookupRow]->GetColumnValueString(GetColumnID(m_nLookupCol));
            SetCellValueFromEntry(m_lLookupRow, m_nLookupCol, szText, eLineType, true);
            CString szNewValue = m_pTransaction->m_arOrderedDetail[m_lLookupRow]->GetColumnValueString(GetColumnID(m_nLookupCol));
            if (szNewValue != szOldValue)
            {
                // begbert 05-06-2008 1-29715 : Load in the description, from the Stock Number/ GL Account that got us in here.
                // begbert 12-01-2008 1-31498 : If selected on the maintenence form rather than the lookup, no description is
                //   returned; handle that cleanly.  Also added similar error protection for strMLILocation, and also added a
                //   test for LKP_IN_STOCKNUMBER_PURCH, which is what's returned when the value is selected on the maintenence form.
                // begbert 01-28-2009 1-31873 : Updated to _POE variant lookups
                if (clsXMLParams.ParamExists(_T("strDescription")) && !m_pTransaction->m_arOrderedDetail[m_lLookupRow]->IsUsingVariants())
                    SetCellValueFromEntry(m_lLookupRow, GetDisplayedColumnIndex(Description), szText = clsXMLParams.GetParamStr(_T("strDescription")), eLineType);
                if ((m_uiActiveLookup == LKP_IN_STOCKNUMBER_PURCH_POE ||
                    m_uiActiveLookup == LKP_IN_STOCKNUMBER_MLI_PURCH_POE) && //LKK 6/2/08 1-29919 new lookup
                    clsXMLParams.ParamExists(_T("strMLILocation")))
                    SetCellValueFromEntry(m_lLookupRow, GetDisplayedColumnIndex(MLILocation), szText = clsXMLParams.GetParamStr(_T("strMLILocation")), eLineType);
                //			UpdateGrid();
                            // begbert 11-25-2008 1-31481 : Removed this; it was redundant with the tabout in the lookup itself and apparently unnecessary.
            //				Entry_GoRight(m_nLookupCol, m_lLookupRow);	//PTR.03.22.2005.1-16285
            }
        }
    }
    m_uiActiveLookup = 0;
    m_lLookupRow = 0;
    m_nLookupCol = 0;
}

BOOL CGrdPurDetail::SetCellValueFromEntry(long lRow, int nCol, CString& szText, BASETYPE eBaseType, bool bOverwriteSame)
{
    if (lRow < 0 || lRow >= m_pTransaction->m_arOrderedDetail.GetCount() ||
        nCol < 0 || nCol >= m_arOrderedColumns.GetCount())
        return FALSE;	// out of range.

    // begbert 10-23-2008 1-31104 : Do not allow the change if it would attempt to add a line beyond the limit.
    if (lRow == m_pTransaction->m_arOrderedDetail.GetCount() - 1 &&
        !ValidateDetailCount())
        return FALSE;

    CLTDetailBase *pOldLine, *pNewLine;

    pOldLine = m_pTransaction->m_arOrderedDetail[lRow];
    if (pOldLine->WhatAmI() == eBaseType)
    {	// No need to change line types; carry on.
        // begbert 08-12-2008 1-30244 : (Copied from below) Hide the control early without saving its contents, or it will be hid *with* saving in UpdateTransaction, which causes problems.
        m_pCtrlHelper->HideVisibleCtrl(false);
        // begbert 11-24-2008 1-31469 : Pass along the bOverwriteSame parameter
        UpdateTransaction(lRow, GetColumnID(nCol), szText, bOverwriteSame);
        return TRUE;
    }

    pNewLine = CLTDetailBase::GetNew(eBaseType, this, m_pTransaction);

    *pNewLine = *pOldLine;
    m_pTransaction->m_arOrderedDetail[lRow] = pNewLine;

    if (lRow == m_pTransaction->m_arOrderedDetail.GetCount() - 1)
    {
        ASSERT(pOldLine->WhatAmI() == UNKNOWN);	// just checking

        m_pTransaction->AddDetailLine(pOldLine);
        SetNumberRows(lRow + 2, TRUE, lRow);	// begbert 11-18-2008 1-31421 : Pass in the row at which to insert elements
    }
    else {
        pOldLine->Delete();
    }

    // begbert 05-23-2008 1-29934 : Hide the control early without saving its contents, or it will be hid *with* saving in UpdateTransaction, which causes problems.
    m_pCtrlHelper->HideVisibleCtrl(false);
    // begbert 11-24-2008 1-31469 : Pass along the bOverwriteSame parameter
    UpdateTransaction(lRow, GetColumnID(nCol), szText, bOverwriteSame);
    UpdateHeadings(lRow);

    RedrawAll();
    return TRUE;
}

// begbert 10-14-2008 1-31110 : added a parameter saying whether the focus will move to the new row or not.
CLTBase* CGrdPurDetail::InsertRow(long lRow, BASETYPE eBaseType, bool bSelectRow)
{
    if (lRow < 0)
    {
        ASSERT(FALSE);
        lRow = 0;
    }

    if (lRow >= (long)m_pTransaction->m_arOrderedDetail.GetCount())
    {
        ASSERT(FALSE);
        lRow = (long)(m_pTransaction->m_arOrderedDetail.GetCount() - 1);
    }

    CLTDetailBase *pNewLine = CLTDetailBase::GetNew(eBaseType, this, m_pTransaction);

    m_pTransaction->m_arOrderedDetail.InsertAt(lRow, pNewLine);

    //	__super::InsertRow(lRow);
    m_pTransaction->AddDetailLine(pNewLine, lRow);
    SetNumberRows(GetNumberRows() + 1, TRUE, lRow);// begbert 11-18-2008 1-31421 : Pass in the row at which to insert elements

    if (bSelectRow)
    {
        Select(0, lRow);
        UpdateHeadings(lRow);
    }

    RedrawAll();
    return pNewLine;
}

ILineData* CGrdPurDetail::GetRow(int nIndex)
{
    if (nIndex < 0 ||
        m_pTransaction == NULL ||
        nIndex >= m_pTransaction->m_arOrderedDetail.GetCount())
        return NULL;
    return m_pTransaction->m_arOrderedDetail[nIndex];
}
const ILineData* CGrdPurDetail::GetRow(int nIndex) const
{
    if (nIndex < 0 ||
        m_pTransaction == NULL ||
        nIndex >= m_pTransaction->m_arOrderedDetail.GetCount())
        return NULL;
    return m_pTransaction->m_arOrderedDetail[nIndex];
}

CString CGrdPurDetail::GetColumnValueString(INT32 nColID, long row) const
{
    if (row < 0 || row >= m_pTransaction->m_arOrderedDetail.GetSize()) return _T("");
    CLTDetailBase* pLine = m_pTransaction->m_arOrderedDetail[row];
    ASSERT(pLine != NULL);
    return pLine ? pLine->GetColumnValueString(nColID) : _T("");
}

void CGrdPurDetail::SetColumnValueString(INT32 nColID, long row, const CString& szValue)
{
    if (row < 0 || row >= m_pTransaction->m_arOrderedDetail.GetSize()) return;
    CLTDetailBase* pLine = m_pTransaction->m_arOrderedDetail[row];
    ASSERT(pLine != NULL);
    if (pLine) pLine->SetColumnValueString(nColID, szValue);
}

/*

// If the given row is a comment row, this returns range of all the
//   consecutive contiguous comment rows before and after the given row.
bool CGrdPurDetail::GetCommentRange(long lRow, long& lFirst, long& lLast) const
{
    int lMaxRow = lRow >= m_pTransaction->m_arOrderedDetail.GetCount() - 1;

    if (lRow < 0 || lRow > lMaxRow ||
        m_pTransaction->m_arOrderedDetail[lRow]->WhatAmI() != COMMENT)
        return false;

    lFirst = lLast = lRow;
    while (lFirst > 0 && m_pTransaction->m_arOrderedDetail[lFirst-1]->WhatAmI() == COMMENT)
        lFirst--;
    while (lLast < lMaxRow && m_pTransaction->m_arOrderedDetail[lLast+1]->WhatAmI() == COMMENT)
        lLast++;
    return true;
}

// If the given row is a comment row, this returns the concatenation of all
//   the consecutive contiguous comment rows before and after the given row.
CString CGrdPurDetail::GetCommentAround(long lRow) const
{
    CString szComment;
    long lStart, lEnd;
    if (!GetCommentRange(lRow, lStart, lEnd)) return szComment;

    for (lRow = lStart; lRow <= lEnd; lRow++)
    {
        szComment += ((CLTPurDetailComment*) m_pTransaction->m_arOrderedDetail[lRow])->GetComment();
    }
    return szComment;
}
*/


//PGP(04/05/2004) - moved the launch of saleshistory form
//to here, as selected items are directly accessed from the dialog's
//lower grid.
void CGrdPurDetail::ShowPurchaseHistoryDlg()
{
    CDlgVendorPurchaseHistory* pForm = DYNAMIC_DOWNCAST(CDlgVendorPurchaseHistory, ((CCMSWinApp*)AfxGetApp())->m_pFormManager->ShowForm(CDlgVendorPurchaseHistory::IDD));
    if (!pForm) return;
    pForm->SetVendor(m_pTransaction->GetVendor());
    pForm->SetTransaction(m_pTransaction);
    pForm->Subscribe(this);
}

void CGrdPurDetail::OnVendorPurchaseHistoryOKMsg(CDlgVendorPurchaseHistory* pDlg)
{
    //Start adding selected items to detail grid.
    CGrdHistoryLower* pSelectedGrid = pDlg->m_pGrdLower;
    long lSelectedCount = pSelectedGrid->GetHistorySelectedCount();
    for (long lIndex = 0; lIndex < lSelectedCount; lIndex++)
    {
        const CGrdHistoryLower::CGridData* pData = pSelectedGrid->GetHistoryRowAt(lIndex);
        if (pData != NULL)
        {
            BASETYPE nLineType = UNKNOWN;
            long nCol, lRow = (long)(m_pTransaction->m_arOrderedDetail.GetCount() - 1);

            switch (pData->GetLineType())
            {
            case HIST_LINE_TYPE_DIRECTEXPENSE: nLineType = DIRECTEXPENSE; break;
            case HIST_LINE_TYPE_INVENTORY: nLineType = INVENTORY; break;
            default: continue;	// only add lines of one of the above types.
            }

            /*
                        ROWENTERINFO clsRowEnterInfo(lRow, stuProps.nOrder, szStock);
                        clsRowEnterInfo.m_szLocation = pData->GetLocation() ;
                        clsRowEnterInfo.m_szMLILoc = pData->GetMLILocation() ;
                        clsRowEnterInfo.m_nForcedLineType = pData->GetLineType() ;
                        clsRowEnterInfo.m_bScannerMode = true;
                        SetCellValueFromEntry(clsRowEnterInfo);
            */
            CString szKey = pData->GetItem();
            nCol = GetDisplayedColumnIndex(POEntryLayout::DetailItems::StockNumber);
            SetCellValueFromEntry(lRow, nCol, szKey, nLineType);

            // begbert 11-12-2008 1-31381 : Copy in the MLI Location from the the lower History grid.
            CString szMLILoc = pData->GetMLILocation();
            nCol = GetDisplayedColumnIndex(POEntryLayout::DetailItems::MLILocation);
            SetCellValueFromEntry(lRow, nCol, szMLILoc, nLineType);

            // begbert 01-02-2007 1-26702 : Copy in the quantity value(s) from the lower History grid.
            CString szQty = pData->GetQty();
            nCol = GetDisplayedColumnIndex(POEntryLayout::DetailItems::QuantityOrdered);
            SetCellValueFromEntry(lRow, nCol, szQty, nLineType);
        }
    }
}

// begbert 04-17-2008 1-29489 : Added this, primarily to catch and handle multi-keyed input from lookups.
BOOL CGrdPurDetail::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    try
    {
        NMHDR* pMessage = (NMHDR*)lParam;
        CString szTextBoxText = _T("");

        switch (pMessage->code)
        {
        case WM_LOOKUP_END:
        {
            CCMSGridAutoFillEdit* pWnd = DYNAMIC_DOWNCAST(CCMSGridAutoFillEdit, FromHandle(pMessage->hwndFrom));
            if (!pWnd || &pWnd->GetGrid() != this)
                break;	// Whoops; it wasn't for us.
            //Defect ID 1-30084, PGP(06-22-2008) Check if there is a valid lookup
            if (pWnd->GetLookup() == NULL)
                break;
            // prep up data for UpdateObjectFromLookup, then have it do the actual work.
            m_uiActiveLookup = pWnd->GetLookup()->GetLookupID();
            m_lLookupRow = GetCurrentRow();
            m_nLookupCol = GetCurrentCol();
            CXMLParams clsXMLData = pWnd->GetLookup()->GetLookupInfo().GetXMLParams(_T("PRIMARYKEY"));
            UpdateObjectFromLookup(clsXMLData);
        }
        break;
        }
    }
    catch (_com_error& e)
    {
        CGBLForm::HandleError(e);
    }

    return __super::OnNotify(wParam, lParam, pResult);
}

// begbert 06-02-2008 : Validate the amounts (can be used in both Purchasing and Receiving Add Items).
// This function assumes that the discount information in the header has already passed validation.
// begbert 06-06-2008 1-30058 : Altered to expect a grid, not a transaction, so that we can recalc the Landing on it.
// begbert 06-12-2008 1-30118 : Altered to update the F/T/D/L as though all the lines
//   were positive before getting the totals, so return lines don't defeat the test.
// begbert 01-16-2009 1-31703 : Corrected it to get all the totals from the combined transaction, and also
//   aded parameters through which to return what the subtotal and total from the combined transaction were.
bool CGrdPurDetail::ValidateBuyerAmounts(CString& szErrMessage, CString& szErrCaption, CGrdRcvDetail* pOriginalGrid /*=NULL*/,
    CCMSDouble* pdSubtotalPosNegAmount /*=NULL*/, CCMSDouble* pdTotalPosNegAmount /*=NULL*/)
{
    CCMSDouble dSubtotalAllPosAmount, dTotalAllPosAmount, dTotalAllPosDiscount, dMaxAmount;
    CCMSDouble dSubtotalPosNegAmount, dTotalPosNegAmount, dTotalNegAmount, dTotalPosAmount;

    if (pOriginalGrid)
    {	// Add in the lines from the original grid, if one exists.
        CCMSTransaction* pOriginalTrans = pOriginalGrid->GetTransaction();

        m_pTransaction->m_arOrderedDetail.InsertAt(0, &pOriginalTrans->m_arOrderedDetail);
        for (int i = 0; i < pOriginalTrans->m_arOrderedDetail.GetCount(); i++)
            m_pTransaction->m_arOrderedDetail[i]->m_pTransaction = m_pTransaction;
    }

    UpdateFreightTaxDiscountLanding(true);	// total it all as if it was all positive

    dSubtotalAllPosAmount = m_pTransaction->GetPOSubtotal();
    dTotalAllPosAmount = m_pTransaction->GetPOAmount();
    dTotalAllPosDiscount = m_pTransaction->GetTotalEstimatedDiscount();

    UpdateFreightTaxDiscountLanding(false);	// Update it with normal (positive/negative) totals.

    dTotalPosNegAmount = m_pTransaction->GetPOAmount();

    if (pdSubtotalPosNegAmount) (*pdSubtotalPosNegAmount) = m_pTransaction->GetPOSubtotal();
    if (pdTotalPosNegAmount) (*pdTotalPosNegAmount) = dTotalPosNegAmount;

    // begbert 10-30-2008 1-31158 : we need to know how much of the PO came from negative totals;
    //   calculate it from (half of) the difference between the totals and the totals if all were positive.
    dTotalNegAmount = (dTotalAllPosAmount - dTotalPosNegAmount) / 2.0;
    dTotalPosAmount = dTotalAllPosAmount - dTotalNegAmount;

    if (pOriginalGrid)
    {	// Remove the previously-added lines from the original grid, if one exists.
        CCMSTransaction* pOriginalTrans = pOriginalGrid->GetTransaction();

        // begbert 06-06-2008 1-30058 : Restore the OriginalTrans's lines' transaction pointers.
        m_pTransaction->m_arOrderedDetail.RemoveAt(0, pOriginalTrans->m_arOrderedDetail.GetCount());
        for (int i = 0; i < pOriginalTrans->m_arOrderedDetail.GetCount(); i++)
            pOriginalTrans->m_arOrderedDetail[i]->m_pTransaction = pOriginalTrans;
        pOriginalGrid->UpdateFreightTaxDiscountLanding(); // begbert 06-06-2008 1-30058 : update the correct grid.

        UpdateFreightTaxDiscountLanding();
    }


    dMaxAmount = CCMSDouble(m_pTransaction->GetBuyerSettings()->dMaxPOAmount);
    if (dTotalAllPosAmount > dMaxAmount)
    {
        szErrCaption = RESSTRING(IDS_PO_EXCEED_BUYER_MAX_POAMOUNT_TITLE);
        // begbert 10-30-2008 1-31158 : Rather than there being a slightly-different message when
        //   in add mode, there is now only one message, which now uses the Pos and Neg totals.
//		szErrMessage = RESSTRING(pOriginalGrid == NULL ? IDS_PO_EXCEED_BUYER_MAX_POAMOUNT_MSG : IDS_PO_EXCEED_BUYER_MAX_POAMOUNT_MSG2);
        szErrMessage = RESSTRING(IDS_PO_EXCEED_BUYER_MAX_POAMOUNT_MSG);
        szErrMessage.Replace(_T("<MAXIMUM_VALUE>"), GetCurrency(dMaxAmount));
        szErrMessage.Replace(_T("<POSITIVE_TOTAL>"), GetCurrency(dTotalPosAmount));
        szErrMessage.Replace(_T("<NEGATIVE_TOTAL>"), GetCurrency(dTotalNegAmount));
        return false;
    }

    dMaxAmount = CCMSDouble(m_pTransaction->GetBuyerSettings()->dMaxDiscAmount);
    if (dTotalAllPosDiscount > dMaxAmount)
    {
        szErrCaption = RESSTRING(IDS_PO_EXCEED_BUYER_MAX_DISCOUNT_TITLE);
        szErrMessage = RESSTRING(IDS_PO_EXCEED_BUYER_MAX_DISCOUNT_MSG);
        szErrMessage.Replace(_T("<DISC_TYPE_TEXT>"), RESSTRING(IDS_AMOUNT));
        szErrMessage.Replace(_T("<MAXIMUM_VALUE>"), GetCurrency(dMaxAmount));
        return false;
    }

    // begbert 08-01-2008 : check the percentage of the subtotal that's discounted against the Max Discount Percent
    dMaxAmount = CCMSDouble(m_pTransaction->GetBuyerSettings()->dMaxDiscPercent);
    if (100.0 * dTotalAllPosDiscount / dSubtotalAllPosAmount > dMaxAmount)
    {
        szErrCaption = RESSTRING(IDS_PO_EXCEED_BUYER_MAX_DISCOUNT_TITLE);
        szErrMessage = RESSTRING(IDS_PO_EXCEED_BUYER_MAX_DISCOUNT_MSG);
        szErrMessage.Replace(_T("<DISC_TYPE_TEXT>"), RESSTRING(IDS_PERCENT));
        szErrMessage.Replace(_T("<MAXIMUM_VALUE>"), GetDouble(dMaxAmount, 2) + RESSTRING(IDS_PROMO_PERCENT));
        return false;
    }

    return true;
}

// begbert 06-06-2008 : created this
bool CGrdPurDetail::AreAnyLinesPosted() const
{
    for (int i = 0; i < m_pTransaction->m_arOrderedDetail.GetCount(); i++)
    {
        if (m_pTransaction->m_arOrderedDetail[i]->HasBeenPosted())
            return true;
    }
    return false;

}

bool CGrdPurDetail::ValidateCostCenters()
{
	std::set<CString> costcenters;
	
	// Load in the legal cost centers.
	CCMSDataAdapter	objDA(L"LOOKUP", L"GL", L"LKP_GL_COSTCENTERS");
	CXMLDocument xmlDoc(L"ROOT"), retDoc;
	CXMLElement xmlRoot = xmlDoc.GetDocumentElement();
	xmlRoot.AddNewChild(L"SORTBY").AddNewChild(L"strCostCenter", 0);
	xmlRoot.AddNewChild(L"ROWCOUNT", 100000);

	CCMSDataAdapterOptions clsOptions;
	clsOptions.m_clsServerInput.m_bShowErrorDialog = false;

	CDataSet objDS;
	if (objDA.GetData(xmlDoc, objDS) == rcSuccess && objDS.TableExists(L"LKP_GL_COSTCENTERS") )
	{
		CDataTable	objDT;
		objDS.GetTable(L"LKP_GL_COSTCENTERS", &objDT);
		
		CDataRow	objDR;
		for (int nRow = 0; nRow < objDT.getRowCount(); nRow++)
		{
			objDT.GetRow(nRow, &objDR);
			costcenters.insert(objDR.XMLStr(L"strCostCenter"));
		}
	}

	// Put blank in because blank is legal too
	costcenters.insert(L"");	

	// Check each line for an illegal cost center.
	for (int i = 0; i < m_pTransaction->m_arOrderedDetail.GetCount(); i++)
    {
        CLTDetailBase* pLine = m_pTransaction->m_arOrderedDetail[i];

		if (costcenters.count(pLine->GetColumnValueString(CostCenter)) == 0)
		{
			CGBLForm::CMSMessageBox(this, 0, 0, MB_OK + MB_ICONERROR, L"The Cost Center that you have specified is not a valid Cost Center.", L"Error", false);
			GotoCell(GetDisplayedColumnIndex(CostCenter), i);
			SetFocus();
			return false;
		}
    }

	return true;
}
