// PRuleCard.cpp : implementation file
//

#include "stdafx.h"
#include "mighty.h"
#include "PRuleCard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// PRuleCard property page

IMPLEMENT_DYNCREATE(PRuleCard, CPropertyPage)

PRuleCard::PRuleCard(CRule* pRule,bool bReadOnly) : CPropertyPage(PRuleCard::IDD)
{
	m_pRule = pRule;
	m_bReadOnly = bReadOnly;

	//{{AFX_DATA_INIT(PRuleCard)
	m_nAltJokercall = m_pRule->nAlterJokercall-1;
	m_nAltMighty = m_pRule->nAlterMighty-1;
	m_nJokercall = m_pRule->nJokercall-1;
	m_nMighty = m_pRule->nMighty-1;
	//}}AFX_DATA_INIT
}

PRuleCard::~PRuleCard()
{
}

void PRuleCard::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PRuleCard)
	DDX_CBIndex(pDX, IDC_CARD_ALTJOKERCALL, m_nAltJokercall);
	DDX_CBIndex(pDX, IDC_CARD_ALTMIGHTY, m_nAltMighty);
	DDX_CBIndex(pDX, IDC_CARD_JOKERCALL, m_nJokercall);
	DDX_CBIndex(pDX, IDC_CARD_MIGHTY, m_nMighty);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate ) {
		m_pRule->nAlterJokercall = m_nAltJokercall + 1;
		m_pRule->nAlterMighty = m_nAltMighty + 1;
		m_pRule->nJokercall = m_nJokercall + 1;
		m_pRule->nMighty = m_nMighty + 1;
	}
}


BEGIN_MESSAGE_MAP(PRuleCard, CPropertyPage)
	//{{AFX_MSG_MAP(PRuleCard)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PRuleCard message handlers

int PRuleCard::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPropertyPage::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	EnableWindow(!m_bReadOnly);
	return 0;
}
