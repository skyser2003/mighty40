// DOption.cpp : implementation file
//

#include "stdafx.h"
#include "Mighty.h"
#include "MightyToolTip.h"
#include "DOption.h"

#include "POptionGeneral.h"
#include "POptionBosskey.h"
#include "POptionCard.h"
#include "POptionBackground.h"
#include "POptionDSB.h"
#include "POptionSpeed.h"
#include "POptionRule.h"
#include "POptionAI.h"
#include "POptionComm.h"
#include "POptionChat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DOption

IMPLEMENT_DYNAMIC(DOption, CPropertySheet)

DOption::DOption(CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(_T("마이티 옵션"), pParentWnd, iSelectPage)
{
	m_nPages = 10;
	m_apPages = new CPropertyPage*[m_nPages];

	m_apPages[0] = new POptionGeneral;
	m_apPages[1] = new POptionBosskey;
	m_apPages[2] = new POptionCard;
	m_apPages[3] = new POptionBackground;
	m_apPages[4] = new POptionDSB;
	m_apPages[5] = new POptionSpeed;
	m_apPages[6] = new POptionRule;
	m_apPages[7] = new POptionAI;
	m_apPages[8] = new POptionComm;
	m_apPages[9] = new POptionChat;

	for ( int i = 0; i < m_nPages; i++ )
		AddPage( m_apPages[i] );

	m_psh.dwFlags |= PSH_NOAPPLYNOW;
	m_psh.dwFlags &= ~PSH_HASHELP;
}

DOption::~DOption()
{
	for ( int i = 0; i < m_nPages; i++ )
		delete m_apPages[i];
	delete[] m_apPages;
}


BEGIN_MESSAGE_MAP(DOption, CPropertySheet)
	//{{AFX_MSG_MAP(DOption)
	ON_WM_CREATE()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int DOption::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
//  수정: 탭을 스크롤 하게 했더니 오히려 불편했다 !
//	EnableStackedTabs( FALSE );

	if (CPropertySheet::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	ModifyStyleEx( 0, WS_EX_CONTEXTHELP );

	return 0;
}

BOOL DOption::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	// TODO: Add your message handler code here and/or call default

	// 툴 팁을 표시
	CString sTip;
	if ( !sTip.LoadString( pHelpInfo->iCtrlId ) )
		sTip = _T("이 아이템에 대한 도움말은 없습니다\n")
				_T("근처에 있는 다른 아이템을 클릭해 보십시오");

	Tt()->Tip(
		pHelpInfo->MousePos.x+TOOLTIP_BORDER_MARGIN*2,
		pHelpInfo->MousePos.y+TOOLTIP_BORDER_MARGIN*2,
		sTip );

//	return CPropertySheet::OnHelpInfo(pHelpInfo);
	return TRUE;
}
