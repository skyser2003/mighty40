// PRuleScore.cpp : implementation file
// modified(v4.0): 2010.4.2

#include "stdafx.h"
#include "mighty.h"
#include "PRuleScore.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int get_Dmethod( bool bS_Use20, bool bS_Use40, bool bS_Call, bool bS_Efe,
	bool bS_MEfe, bool bS_Base10, bool bS_Base13, bool bS_BaseM )
{
	if ( bS_Use20 ) return 0;
	else if ( bS_Use40 ) return 1;
	else if ( bS_Call ) return 2;
	else if ( bS_Efe ) return 3;
	else if ( bS_MEfe ) return 4;
	else if ( bS_Base10 ) return 5;
	else if ( bS_Base13 ) return 6;
	else if ( bS_BaseM ) return 7;
	else return 0;
}
static void parse_Dmethod( bool& bS_Use20, bool& bS_Use40, bool& bS_Call, bool& bS_Efe,
	bool& bS_MEfe, bool& bS_Base10, bool& bS_Base13, bool& bS_BaseM, int nDMethod )
{
	bS_Use20 = bS_Use40 = bS_Call = bS_Efe = bS_MEfe = bS_Base10 = bS_Base13 = bS_BaseM = false;
	if ( nDMethod == 0 ) bS_Use20 = true;
	else if ( nDMethod == 1 ) bS_Use40 = true;
	else if ( nDMethod == 2 ) bS_Call = true;
	else if ( nDMethod == 3 ) bS_Efe = true;
	else if ( nDMethod == 4 ) bS_MEfe = true;
	else if ( nDMethod == 5 ) bS_Base10 = true;
	else if ( nDMethod == 6 ) bS_Base13 = true;
	else if ( nDMethod == 7 ) bS_BaseM = true;
	else bS_Use20 = true;
}
static int get_Amethod( bool bSS_Efe, bool bSS_Tft )
{
	if ( bSS_Efe ) return 0;
	else if ( bSS_Tft ) return 1;
	else return 0;
}
static void parse_Amethod( bool& bSS_Efe, bool& bSS_Tft, int nAMethod )
{
	bSS_Efe = bSS_Tft = false;
	if ( nAMethod == 0 ) bSS_Efe = true;
	else if ( nAMethod == 1 ) bSS_Tft = true;
	else bSS_Efe = true;
}

/////////////////////////////////////////////////////////////////////////////
// PRuleScore property page

IMPLEMENT_DYNCREATE(PRuleScore, CPropertyPage)

PRuleScore::PRuleScore(CRule* pRule,bool bReadOnly) : CPropertyPage(PRuleScore::IDD)
{
	m_pRule = pRule;
	m_bReadOnly = bReadOnly;

	//{{AFX_DATA_INIT(PRuleScore)
	m_nDMethod = get_Dmethod(m_pRule->bS_Use20, m_pRule->bS_Use40, m_pRule->bS_Call, m_pRule->bS_Efe, m_pRule->bS_MEfe, m_pRule->bS_Base10, m_pRule->bS_Base13, m_pRule->bS_BaseM);
	m_nAMethod = get_Amethod(m_pRule->bSS_Efe, m_pRule->bSS_Tft);
	//}}AFX_DATA_INIT
}

PRuleScore::~PRuleScore()
{
}

void PRuleScore::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PRuleScore)
	DDX_Radio(pDX, IDC_S_0, m_nDMethod);
	DDX_Radio(pDX, IDC_SS_0, m_nAMethod);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate ) {
		parse_Dmethod(m_pRule->bS_Use20, m_pRule->bS_Use40, m_pRule->bS_Call, m_pRule->bS_Efe, m_pRule->bS_MEfe, m_pRule->bS_Base10, m_pRule->bS_Base13, m_pRule->bS_BaseM, m_nDMethod);
		parse_Amethod(m_pRule->bSS_Efe, m_pRule->bSS_Tft, m_nAMethod);
	}
}


BEGIN_MESSAGE_MAP(PRuleScore, CPropertyPage)
	//{{AFX_MSG_MAP(PRuleScore)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PRuleScore message handlers

int PRuleScore::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPropertyPage::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	EnableWindow(!m_bReadOnly);
	return 0;
}
