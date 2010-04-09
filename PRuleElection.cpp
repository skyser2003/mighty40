// PRuleElection.cpp : implementation file
//

#include "stdafx.h"
#include "mighty.h"
#include "PRuleElection.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int get_nk( bool bNoKirudaAdvantage, bool bbNoKirudaAlways )
{
	if ( bbNoKirudaAlways ) return 2;
	else if ( bNoKirudaAdvantage ) return 1;
	else return 0;
}
static void parse_nk( bool& bNoKirudaAdvantage, bool& bNoKirudaAlways, 
	int nnk )
{
	bNoKirudaAdvantage = bNoKirudaAlways = false;
	if ( nnk == 1 ) bNoKirudaAdvantage = true;
	else if ( nnk == 2 ) {
		bNoKirudaAdvantage = true;
		bNoKirudaAlways = true;
	}
}

/////////////////////////////////////////////////////////////////////////////
// PRuleElection property page

IMPLEMENT_DYNCREATE(PRuleElection, CPropertyPage)

PRuleElection::PRuleElection(CRule* pRule,bool bReadOnly) : CPropertyPage(PRuleElection::IDD)
{
	m_pRule = pRule;
	m_bReadOnly = bReadOnly;

	//{{AFX_DATA_INIT(PRuleElection)
	m_bBeginerPass = m_pRule->bBeginerPass;
	m_bHighScore = m_pRule->bHighScore;
	m_nMinScore = m_pRule->nMinScore - 4;
	m_bRaise2ForKirudaChange = m_pRule->bRaise2ForKirudaChange;
	m_bPassAgain = m_pRule->bPassAgain;
	m_nNoKirudaAdvantage = get_nk(m_pRule->bNoKirudaAdvantage, m_pRule->bNoKirudaAlways);
	//}}AFX_DATA_INIT
}

PRuleElection::~PRuleElection()
{
}

void PRuleElection::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PRuleElection)
	DDX_Check(pDX, IDC_BEGINERPASS, m_bBeginerPass);
	DDX_Check(pDX, IDC_HIGHSCORE, m_bHighScore);
	DDX_CBIndex(pDX, IDC_MINSCORE, m_nMinScore);
	DDX_Check(pDX, IDC_RAISE2FORKIRUDACHANGE, m_bRaise2ForKirudaChange);
	DDX_Check(pDX, IDC_PASSAGAIN, m_bPassAgain);
	DDX_Radio(pDX, IDC_NK, m_nNoKirudaAdvantage);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate ) {
		m_pRule->bBeginerPass = !!m_bBeginerPass;
		m_pRule->bHighScore = !!m_bHighScore;
		m_pRule->nMinScore = m_nMinScore + 4;
		m_pRule->bRaise2ForKirudaChange = !!m_bRaise2ForKirudaChange;
		m_pRule->bPassAgain = !!m_bPassAgain;
		parse_nk(m_pRule->bNoKirudaAdvantage, m_pRule->bNoKirudaAlways, m_nNoKirudaAdvantage);
	}
}


BEGIN_MESSAGE_MAP(PRuleElection, CPropertyPage)
	//{{AFX_MSG_MAP(PRuleElection)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PRuleElection message handlers

int PRuleElection::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPropertyPage::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	EnableWindow(!m_bReadOnly);
	return 0;
}
