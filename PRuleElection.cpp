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

/////////////////////////////////////////////////////////////////////////////
// PRuleElection property page

IMPLEMENT_DYNCREATE(PRuleElection, CPropertyPage)

PRuleElection::PRuleElection(CRule* pRule,bool bReadOnly) : CPropertyPage(PRuleElection::IDD)
{
	m_pRule = pRule;
	m_bReadOnly = bReadOnly;

	//{{AFX_DATA_INIT(PRuleElection)
	m_bAttScoreThrownPoints = m_pRule->bAttScoreThrownPoints;
	m_bBeginerPass = m_pRule->bBeginerPass;
	m_bFriend = m_pRule->bFriend;
	m_bFriendGetsBeginer = m_pRule->bFriendGetsBeginer;
	m_bHighScore = m_pRule->bHighScore;
	m_bJokerFriend = m_pRule->bJokerFriend;
	m_nMinScore = m_pRule->nMinScore - 5;
	m_bNoKirudaAdvantage = m_pRule->bNoKirudaAdvantage;
	m_bRaise1ForNoKirudaChange = m_pRule->bRaise1ForNoKirudaChange;
	m_bRaise2ForKirudaChange = m_pRule->bRaise2ForKirudaChange;
	//}}AFX_DATA_INIT
}

PRuleElection::~PRuleElection()
{
}

void PRuleElection::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PRuleElection)
	DDX_Check(pDX, IDC_ATTSCORETHROWNPOINTS, m_bAttScoreThrownPoints);
	DDX_Check(pDX, IDC_BEGINERPASS, m_bBeginerPass);
	DDX_Check(pDX, IDC_FRIEND, m_bFriend);
	DDX_Check(pDX, IDC_FRIENDGETSBEGINER, m_bFriendGetsBeginer);
	DDX_Check(pDX, IDC_HIGHSCORE, m_bHighScore);
	DDX_Check(pDX, IDC_JOKERFRIEND, m_bJokerFriend);
	DDX_CBIndex(pDX, IDC_MINSCORE, m_nMinScore);
	DDX_Check(pDX, IDC_NOKIRUDAADVANTAGE, m_bNoKirudaAdvantage);
	DDX_Check(pDX, IDC_RAISE1FORNOKIRUDACHANGE, m_bRaise1ForNoKirudaChange);
	DDX_Check(pDX, IDC_RAISE2FORKIRUDACHANGE, m_bRaise2ForKirudaChange);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate ) {
		m_pRule->bAttScoreThrownPoints = !!m_bAttScoreThrownPoints;
		m_pRule->bBeginerPass = !!m_bBeginerPass;
		m_pRule->bFriend = !!m_bFriend;
		m_pRule->bFriendGetsBeginer = !!m_bFriendGetsBeginer;
		m_pRule->bHighScore = !!m_bHighScore;
		m_pRule->bJokerFriend = !!m_bJokerFriend;
		m_pRule->nMinScore = m_nMinScore + 5;
		m_pRule->bNoKirudaAdvantage = !!m_bNoKirudaAdvantage;
		m_pRule->bRaise1ForNoKirudaChange = !!m_bRaise1ForNoKirudaChange;
		m_pRule->bRaise2ForKirudaChange = !!m_bRaise2ForKirudaChange;
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
