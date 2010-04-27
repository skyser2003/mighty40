// PRuleSpecial.cpp : implementation file
// modified(v4.0): 2010.4.2

#include "stdafx.h"
#include "Mighty.h"
#include "PRuleSpecial.h"

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
static int get_reverse( bool bS_AGoalReverse,
    bool bS_A11Reverse, bool bS_AMReverse )
{
	if ( bS_AGoalReverse ) return 0;
	else if ( bS_A11Reverse ) return 1;
	else if ( bS_AMReverse ) return 2;
	else return 0;
}
static void parse_reverse( bool& bS_AGoalReverse,
    bool& bS_A11Reverse, bool& bS_AMReverse,
	int nReverse )
{
	bS_AGoalReverse = bS_A11Reverse = bS_AMReverse = false;
	if ( nReverse == 0 ) bS_AGoalReverse = true;
	else if ( nReverse == 1 ) bS_A11Reverse = true;
	else if ( nReverse == 2 ) bS_AMReverse = true;
}

/////////////////////////////////////////////////////////////////////////////
// PRuleSpecial property page

IMPLEMENT_DYNCREATE(PRuleSpecial, CPropertyPage)

PRuleSpecial::PRuleSpecial(CRule* pRule,bool bReadOnly) : CPropertyPage(PRuleSpecial::IDD)
{
	m_pRule = pRule;
	m_bReadOnly = bReadOnly;

	//{{AFX_DATA_INIT(PRuleSpecial)
	m_nRun = get_run(m_pRule->bS_DoubleForRun, m_pRule->bS_DoubleForDeclaredRun, m_pRule->bS_StaticRun);
	m_bS_NoFriend = m_pRule->bS_DoubleForNoFriend;
	m_bS_NoKiruda = m_pRule->bS_DoubleForNoKiruda;
	m_bS_ReverseRun = m_pRule->bS_DoubleForReverseRun;
	m_nReverseRun = get_reverse(m_pRule->bS_AGoalReverse, m_pRule->bS_A11Reverse, m_pRule->bS_AMReverse);
	//}}AFX_DATA_INIT
}

PRuleSpecial::~PRuleSpecial()
{
}

void PRuleSpecial::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PRuleSpecial)
	DDX_Radio(pDX, IDC_RUN, m_nRun);
	DDX_Radio(pDX, IDC_REVERSERUN, m_nReverseRun);
	DDX_Check(pDX, IDC_S_NOFRIEND, m_bS_NoFriend);
	DDX_Check(pDX, IDC_S_NOKIRUDA, m_bS_NoKiruda);
	DDX_Check(pDX, IDC_S_REVERSERUN, m_bS_ReverseRun);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate ) {
		parse_run(m_pRule->bS_DoubleForRun, m_pRule->bS_DoubleForDeclaredRun, m_pRule->bS_StaticRun, m_nRun);
		m_pRule->bS_DoubleForNoFriend = !!m_bS_NoFriend;
		m_pRule->bS_DoubleForNoKiruda = !!m_bS_NoKiruda;
		m_pRule->bS_DoubleForReverseRun = !!m_bS_ReverseRun;
		parse_reverse(m_pRule->bS_AGoalReverse, m_pRule->bS_A11Reverse, m_pRule->bS_AMReverse, m_nReverseRun);
	}
}


BEGIN_MESSAGE_MAP(PRuleSpecial, CPropertyPage)
	//{{AFX_MSG_MAP(PRuleSpecial)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PRuleSpecial message handlers

int PRuleSpecial::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPropertyPage::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	EnableWindow(!m_bReadOnly);
	return 0;
}
