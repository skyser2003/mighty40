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
	m_bRandomSeat = m_pRule->bRandomSeat;
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
	DDX_Check(pDX, IDC_RANDOMSEAT, m_bRandomSeat);
	DDX_Check(pDX, IDC_ATTSCORETHROWNPOINTS, m_bAttScoreThrownPoints);
	DDX_Check(pDX, IDC_HIDESCORE, m_bHideScore);
	DDX_Control(pDX, IDC_FRIEND, m_btFriend);
	DDX_Control(pDX, IDC_JOKERFRIEND, m_btJokerFriend);
	DDX_Control(pDX, IDC_SHOWFRIEND, m_btShowFriend);
	DDX_Control(pDX, IDC_FRIENDGETSBEGINER, m_btFriendGetsBeginer);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate ) {
		m_pRule->nPlayerNum = m_nPlayerNum + 2;
		m_pRule->bFriend = !!m_bFriend && (m_nPlayerNum > 2);
		m_pRule->bJokerFriend = !!m_bJokerFriend;
		m_pRule->bShowFriend = !!m_bShowFriend;
		m_pRule->bFriendGetsBeginer = !!m_bFriendGetsBeginer;
		m_pRule->bRandomSeat = !!m_bRandomSeat;
		m_pRule->bAttScoreThrownPoints = !!m_bAttScoreThrownPoints;
		m_pRule->bHideScore = !!m_bHideScore;
	}
	else if ( m_nPlayerNum == 0 ) OnClicked2MA();
}

BEGIN_MESSAGE_MAP(PRuleGeneral, CPropertyPage)
	//{{AFX_MSG_MAP(PRuleGeneral)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_2MA, OnClicked2MA)
	ON_BN_CLICKED(IDC_3MA, OnClicked3MA)
	ON_BN_CLICKED(IDC_4MA, OnClicked4MA)
	ON_BN_CLICKED(IDC_5MA, OnClicked5MA)
	ON_BN_CLICKED(IDC_6MA, OnClicked6MA)
	ON_BN_CLICKED(IDC_7MA, OnClicked7MA)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PRuleGeneral message handlers

int PRuleGeneral::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPropertyPage::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	EnableWindow(!m_bReadOnly);

	return 0;
}

void PRuleGeneral::OnClicked2MA()
{
	setFriendCheckbox(false);
}
void PRuleGeneral::OnClicked3MA()
{
	setFriendCheckbox(true);
}
void PRuleGeneral::OnClicked4MA()
{
	setFriendCheckbox(true);
}
void PRuleGeneral::OnClicked5MA()
{
	setFriendCheckbox(true);
}
void PRuleGeneral::OnClicked6MA()
{
	setFriendCheckbox(true);
}
void PRuleGeneral::OnClicked7MA()
{
	setFriendCheckbox(true);
}

void PRuleGeneral::setFriendCheckbox(bool enable)
{
	m_btFriend.EnableWindow(enable);
	m_btJokerFriend.EnableWindow(enable);
	m_btShowFriend.EnableWindow(enable);
	m_btFriendGetsBeginer.EnableWindow(enable);
}