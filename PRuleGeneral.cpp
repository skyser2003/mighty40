// PRuleGeneral.cpp : implementation file
//

#include "stdafx.h"
#include "mighty.h"
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
	m_bClockWise = m_pRule->bClockWise;
	m_bHideScore = m_pRule->bHideScore;
	m_bShowFriend = m_pRule->bShowFriend;
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
	DDX_Check(pDX, IDC_CLOCKWISE, m_bClockWise);
	DDX_Check(pDX, IDC_HIDESCORE, m_bHideScore);
	DDX_Check(pDX, IDC_SHOWFRIEND, m_bShowFriend);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate ) {
		m_pRule->nPlayerNum = m_nPlayerNum + 2;
		m_pRule->bClockWise = m_bClockWise ? true : false;
		m_pRule->bHideScore = m_bHideScore ? true : false;
		m_pRule->bShowFriend = m_bShowFriend ? true : false;
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
