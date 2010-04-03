// PRuleScore.cpp : implementation file
//

#include "stdafx.h"
#include "mighty.h"
#include "PRuleScore.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int get_run( bool bS_DoubleForRun,
	bool bS_DoubleForDeclaredRun, bool bS_StaticRun )
{
	if ( bS_StaticRun ) return 3;
	else if ( bS_DoubleForDeclaredRun ) return 2;
	else if ( bS_DoubleForRun ) return 1;
	else return 0;
}
static void parse_run( bool& bS_DoubleForRun,
	bool& bS_DoubleForDeclaredRun, bool& bS_StaticRun,
	int nRun )
{
	bS_DoubleForRun = bS_DoubleForDeclaredRun = bS_StaticRun = false;
	if ( nRun == 1 ) bS_DoubleForRun = true;
	else if ( nRun == 2 ) bS_DoubleForDeclaredRun = true;
	else if ( nRun == 3 ) bS_StaticRun = true;
}
static int get_method( bool bS_Use20, bool bS_Efe,
	bool bS_MEfe, bool bS_Base10, bool bS_Base13 )
{
	if ( bS_Use20 ) return 0;
	else if ( bS_Efe ) return 1;
	else if ( bS_MEfe ) return 2;
	else if ( bS_Base10 ) return 3;
	else if ( bS_Base13 ) return 4;
	else return 0;
}
static void parse_method( bool& bS_Use20, bool& bS_Efe,
	bool& bS_MEfe, bool& bS_Base10, bool& bS_Base13, int nMethod )
{
	bS_Use20 = bS_Efe = bS_MEfe = bS_Base10 = bS_Base13 = false;
	if ( nMethod == 1 ) bS_Use20 = true;
	else if ( nMethod == 2 ) bS_Efe = true;
	else if ( nMethod == 3 ) bS_MEfe = true;
	else if ( nMethod == 4 ) bS_Base10 = true;
	else if ( nMethod == 5 ) bS_Base13 = true;
	else bS_Use20 = true;
}

/////////////////////////////////////////////////////////////////////////////
// PRuleScore property page

IMPLEMENT_DYNCREATE(PRuleScore, CPropertyPage)

PRuleScore::PRuleScore(CRule* pRule,bool bReadOnly) : CPropertyPage(PRuleScore::IDD)
{
	m_pRule = pRule;
	m_bReadOnly = bReadOnly;

	//{{AFX_DATA_INIT(PRuleScore)
	m_nRun = get_run(m_pRule->bS_DoubleForRun, m_pRule->bS_DoubleForDeclaredRun, m_pRule->bS_StaticRun);
	m_nMethod = get_method(m_pRule->bS_Use20, m_pRule->bS_Efe, m_pRule->bS_MEfe, m_pRule->bS_Base10, m_pRule->bS_Base13);
	m_bS_NoFriend = m_pRule->bS_DoubleForNoFriend;
	m_bS_NoKiruda = m_pRule->bS_DoubleForNoKiruda;
	m_bS_ReverseRun = m_pRule->bS_DoubleForReverseRun;
	//}}AFX_DATA_INIT
}

PRuleScore::~PRuleScore()
{
}

void PRuleScore::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PRuleScore)
	DDX_Radio(pDX, IDC_RUN, m_nRun);
	DDX_Radio(pDX, IDC_S_0, m_nMethod);
	DDX_Check(pDX, IDC_S_NOFRIEND, m_bS_NoFriend);
	DDX_Check(pDX, IDC_S_NOKIRUDA, m_bS_NoKiruda);
	DDX_Check(pDX, IDC_S_REVERSERUN, m_bS_ReverseRun);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate ) {
		parse_run(m_pRule->bS_DoubleForRun, m_pRule->bS_DoubleForDeclaredRun, m_pRule->bS_StaticRun,m_nRun);
		parse_method(m_pRule->bS_Use20, m_pRule->bS_Efe, m_pRule->bS_MEfe, m_pRule->bS_Base10, m_pRule->bS_Base13, m_nMethod);
		m_pRule->bS_DoubleForNoFriend = !!m_bS_NoFriend;
		m_pRule->bS_DoubleForNoKiruda = !!m_bS_NoKiruda;
		m_pRule->bS_DoubleForReverseRun = !!m_bS_ReverseRun;
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
