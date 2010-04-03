// DRule.cpp : implementation file
//

#include "stdafx.h"
#include "mighty.h"
#include "DRule.h"
#include "MightyToolTip.h"

#include "PRuleGeneral.h"
#include "PRuleElection.h"
#include "PRuleConstrain.h"
#include "PRuleDealMiss.h"
#include "PRuleScore.h"
#include "PRuleCard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DRule

IMPLEMENT_DYNAMIC(DRule, CPropertySheet)

DRule::DRule(LPCTSTR sRule, bool bReadOnly, bool bAutoDelete,
	CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(_T("마이티 규칙"), pParentWnd, iSelectPage)
{
	m_bReadOnly = bReadOnly;
	m_bAutoDelete = bAutoDelete;

	m_nPages = 6;
	m_apPages = new CPropertyPage*[m_nPages];
	m_rule.Decode( sRule );

	m_apPages[0] = new PRuleGeneral(&m_rule,bReadOnly);
	m_apPages[1] = new PRuleElection(&m_rule,bReadOnly);
	m_apPages[2] = new PRuleConstrain(&m_rule,bReadOnly);
	m_apPages[3] = new PRuleDealMiss(&m_rule,bReadOnly);
	m_apPages[4] = new PRuleScore(&m_rule,bReadOnly);
	m_apPages[5] = new PRuleCard(&m_rule,bReadOnly);

	for ( int i = 0; i < m_nPages; i++ )
		AddPage( m_apPages[i] );

	m_psh.dwFlags |= PSH_NOAPPLYNOW;
	m_psh.dwFlags &= ~PSH_HASHELP;
}

DRule::~DRule()
{
	for ( int i = 0; i < m_nPages; i++ )
		delete m_apPages[i];
	delete[] m_apPages;
}


BEGIN_MESSAGE_MAP(DRule, CPropertySheet)
	//{{AFX_MSG_MAP(DRule)
	ON_WM_CREATE()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DRule message handlers

int DRule::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPropertySheet::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO: Add your specialized creation code here
	ModifyStyleEx( 0, WS_EX_CONTEXTHELP );

	return 0;
}

BOOL DRule::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	// TODO: Add your message handler code here and/or call default

	// 툴 팁을 표시
	CString sTip;
	if ( !sTip.LoadString( pHelpInfo->iCtrlId ) )
		sTip = _T("이 항목에 대한 도움말은 없습니다");

	Tt()->Tip(
		pHelpInfo->MousePos.x+TOOLTIP_BORDER_MARGIN*2,
		pHelpInfo->MousePos.y+TOOLTIP_BORDER_MARGIN*2,
		sTip );

	return TRUE;
}

void DRule::PostNcDestroy() 
{
	Mw()->SetFocus();

	CPropertySheet::PostNcDestroy();

	if ( m_bAutoDelete ) delete this;
}
