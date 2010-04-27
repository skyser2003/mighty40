// PRuleConstrain.cpp : implementation file
//

#include "stdafx.h"
#include "Mighty.h"
#include "PRuleConstrain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// PRuleConstrain property page

IMPLEMENT_DYNCREATE(PRuleConstrain, CPropertyPage)

PRuleConstrain::PRuleConstrain(CRule* pRule,bool bReadOnly) : CPropertyPage(PRuleConstrain::IDD)
{
	m_pRule = pRule;
	m_bReadOnly = bReadOnly;

	//{{AFX_DATA_INIT(PRuleConstrain)
	m_bInitBeginKiruda = m_pRule->bInitBeginKiruda;
	m_bInitJoker = m_pRule->bInitJoker;
	m_bInitJokercallEffect = m_pRule->bInitJokercallEffect;
	m_bInitJokerEffect = m_pRule->bInitJokerEffect;
	m_bInitKiruda = m_pRule->bInitKiruda;
	m_bInitMighty = m_pRule->bInitMighty;
	m_bInitMightyEffect = m_pRule->bInitMightyEffect;
	m_bJokercallJokerEffect = m_pRule->bJokercallJokerEffect;
	m_bLastJoker = m_pRule->bLastJoker;
	m_bLastJokerEffect = m_pRule->bLastJokerEffect;
	m_bLastMighty = m_pRule->bLastMighty;
	m_bLastMightyEffect = m_pRule->bLastMightyEffect;
	m_bOverrideMighty = m_pRule->bOverrideMighty;
	//}}AFX_DATA_INIT
}

PRuleConstrain::~PRuleConstrain()
{
}

void PRuleConstrain::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PRuleConstrain)
	DDX_Check(pDX, IDC_INITBEGINKIRUDA, m_bInitBeginKiruda);
	DDX_Check(pDX, IDC_INITJOKER, m_bInitJoker);
	DDX_Check(pDX, IDC_INITJOKERCALLEFFECT, m_bInitJokercallEffect);
	DDX_Check(pDX, IDC_INITJOKEREFFECT, m_bInitJokerEffect);
	DDX_Check(pDX, IDC_INITKIRUDA, m_bInitKiruda);
	DDX_Check(pDX, IDC_INITMIGHTY, m_bInitMighty);
	DDX_Check(pDX, IDC_INITMIGHTYEFFECT, m_bInitMightyEffect);
	DDX_Check(pDX, IDC_JOKERCALLJOKEREFFECT, m_bJokercallJokerEffect);
	DDX_Check(pDX, IDC_LASTJOKER, m_bLastJoker);
	DDX_Check(pDX, IDC_LASTJOKEREFFECT, m_bLastJokerEffect);
	DDX_Check(pDX, IDC_LASTMIGHTY, m_bLastMighty);
	DDX_Check(pDX, IDC_LASTMIGHTYEFFECT, m_bLastMightyEffect);
	DDX_Check(pDX, IDC_OVERRIDEMIGHTY, m_bOverrideMighty);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate ) {
		m_pRule->bInitBeginKiruda = !!m_bInitBeginKiruda;
		m_pRule->bInitJoker = !!m_bInitJoker;
		m_pRule->bInitJokercallEffect = !!m_bInitJokercallEffect;
		m_pRule->bInitJokerEffect = !!m_bInitJokerEffect;
		m_pRule->bInitKiruda = !!m_bInitKiruda;
		m_pRule->bInitMighty = !!m_bInitMighty;
		m_pRule->bInitMightyEffect = !!m_bInitMightyEffect;
		m_pRule->bJokercallJokerEffect = !!m_bJokercallJokerEffect;
		m_pRule->bLastJoker = !!m_bLastJoker;
		m_pRule->bLastJokerEffect = !!m_bLastJokerEffect;
		m_pRule->bLastMighty = !!m_bLastMighty;
		m_pRule->bLastMightyEffect = !!m_bLastMightyEffect;
		m_pRule->bOverrideMighty = !!m_bOverrideMighty;
	}
}


BEGIN_MESSAGE_MAP(PRuleConstrain, CPropertyPage)
	//{{AFX_MSG_MAP(PRuleConstrain)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PRuleConstrain message handlers

int PRuleConstrain::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPropertyPage::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	EnableWindow(!m_bReadOnly);
	return 0;
}
