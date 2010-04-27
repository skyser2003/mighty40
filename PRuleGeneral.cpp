// PRuleGeneral.cpp : implementation file
//

#include "stdafx.h"
#include "Mighty.h"
#include "PRuleGeneral.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// PRuleGeneral property page

IMPLEMENT_DYNCREATE(PRuleGeneral, CPropertyPage)

PRuleGeneral::PRuleGeneral(CRule* pRule,bool bReadOnly) : CPropertyPage(PRuleGeneral::IDD)
{
	m_pRule = pRule;
	m_bReadOnly = bReadOnly;

	//{{AFX_DATA_INIT(PRuleGeneral)
	m_nPlayerNum = m_pRule->nPlayerNum - 2;
	m_bFriend = m_pRule->bFriend;
	m_bJokerFriend = m_pRule->bJokerFriend;
	m_bShowFriend = m_pRule->bShowFriend;
	m_bFriendGetsBeginer = m_pRule->bFriendGetsBeginer;
	m_bAttScoreThrownPoints = m_pRule->bAttScoreThrownPoints;
	m_bHideScore = m_pRule->bHideScore;
	//}}AFX_DATA_INIT
}

PRuleGeneral::~PRuleGeneral()
{
}

void PRuleGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PRuleGeneral)
	DDX_Radio(pDX, IDC_2MA, m_nPlayerNum);
	DDX_Check(pDX, IDC_FRIEND, m_bFriend);
	DDX_Check(pDX, IDC_JOKERFRIEND, m_bJokerFriend);
	DDX_Check(pDX, IDC_SHOWFRIEND, m_bShowFriend);
	DDX_Check(pDX, IDC_FRIENDGETSBEGINER, m_bFriendGetsBeginer);
	DDX_Check(pDX, IDC_ATTSCORETHROWNPOINTS, m_bAttScoreThrownPoints);
	DDX_Check(pDX, IDC_HIDESCORE, m_bHideScore);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate ) {
		m_pRule->nPlayerNum = m_nPlayerNum + 2;
		m_pRule->bFriend = !!m_bFriend;
		m_pRule->bJokerFriend = !!m_bJokerFriend;
		m_pRule->bShowFriend = m_bShowFriend ? true : false;
		m_pRule->bFriendGetsBeginer = !!m_bFriendGetsBeginer;
		m_pRule->bAttScoreThrownPoints = !!m_bAttScoreThrownPoints;
		m_pRule->bHideScore = m_bHideScore ? true : false;
	}
}


BEGIN_MESSAGE_MAP(PRuleGeneral, CPropertyPage)
	//{{AFX_MSG_MAP(PRuleGeneral)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PRuleGeneral message handlers

int PRuleGeneral::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPropertyPage::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	EnableWindow(!m_bReadOnly);
	return 0;
}
