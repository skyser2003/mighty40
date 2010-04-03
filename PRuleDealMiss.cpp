// PRuleDealMiss.cpp : implementation file
//

#include "stdafx.h"
#include "mighty.h"
#include "PRuleDealMiss.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// PRuleDealMiss property page

IMPLEMENT_DYNCREATE(PRuleDealMiss, CPropertyPage)

PRuleDealMiss::PRuleDealMiss(CRule* pRule,bool bReadOnly) : CPropertyPage(PRuleDealMiss::IDD)
{
	m_pRule = pRule;
	m_bReadOnly = bReadOnly;

	//{{AFX_DATA_INIT(PRuleDealMiss)
	m_bDM_AllPoint = m_pRule->bDM_AllPoint;
	m_bDM_Only10 = m_pRule->bDM_Only10;
	m_bDM_JokerIsReversePoint = m_pRule->bDM_JokerIsReversePoint;
	m_bDM_NoPoint = m_pRule->bDM_NoPoint;
	m_bDM_OneEyedJack = m_pRule->bDM_OneEyedJack;
	m_bDM_OnlyMighty = m_pRule->bDM_OnlyMighty;
	m_bDM_OnlyOne = m_pRule->bDM_OnlyOne;
	//}}AFX_DATA_INIT
}

PRuleDealMiss::~PRuleDealMiss()
{
}

void PRuleDealMiss::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PRuleDealMiss)
	DDX_Check(pDX, IDC_DM_ALLPOINT, m_bDM_AllPoint);
	DDX_Check(pDX, IDC_DM_ONLY10, m_bDM_Only10);
	DDX_Check(pDX, IDC_DM_JOKERISREVERSEPOINT, m_bDM_JokerIsReversePoint);
	DDX_Check(pDX, IDC_DM_NOPOINT, m_bDM_NoPoint);
	DDX_Check(pDX, IDC_DM_ONEEYEDJACK, m_bDM_OneEyedJack);
	DDX_Check(pDX, IDC_DM_ONLYMIGHTY, m_bDM_OnlyMighty);
	DDX_Check(pDX, IDC_DM_ONLYONE, m_bDM_OnlyOne);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate ) {
		m_pRule->bDM_AllPoint = !!m_bDM_AllPoint;
		m_pRule->bDM_Only10 = !!m_bDM_Only10;
		m_pRule->bDM_JokerIsReversePoint = !!m_bDM_JokerIsReversePoint;
		m_pRule->bDM_NoPoint = !!m_bDM_NoPoint;
		m_pRule->bDM_OneEyedJack = !!m_bDM_OneEyedJack;
		m_pRule->bDM_OnlyMighty = !!m_bDM_OnlyMighty;
		m_pRule->bDM_OnlyOne = !!m_bDM_OnlyOne;
	}
}


BEGIN_MESSAGE_MAP(PRuleDealMiss, CPropertyPage)
	//{{AFX_MSG_MAP(PRuleDealMiss)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PRuleDealMiss message handlers

int PRuleDealMiss::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPropertyPage::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	EnableWindow(!m_bReadOnly);
	return 0;
}
