// POptionBosskey.cpp : implementation file
//

#include "stdafx.h"
#include "Mighty.h"
#include "POptionBosskey.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// POptionBosskey property page

IMPLEMENT_DYNCREATE(POptionBosskey, CPropertyPage)

POptionBosskey::POptionBosskey() : CPropertyPage(POptionBosskey::IDD)
{
	//{{AFX_DATA_INIT(POptionBosskey)
	m_nBossType = Mo()->nBossType;
	//}}AFX_DATA_INIT
}

POptionBosskey::~POptionBosskey()
{
}

void POptionBosskey::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(POptionBosskey)
	DDX_Radio(pDX, IDC_BOSSTYPE, m_nBossType);
	//}}AFX_DATA_MAP
	if ( pDX->m_bSaveAndValidate )
		Mo()->nBossType = m_nBossType;
}


BEGIN_MESSAGE_MAP(POptionBosskey, CPropertyPage)
	//{{AFX_MSG_MAP(POptionBosskey)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// POptionBosskey message handlers
