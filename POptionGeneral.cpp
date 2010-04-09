// POptionGeneral.cpp : implementation file
//

#include "stdafx.h"
#include "Mighty.h"
#include "POptionGeneral.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// 유틸리티 함수들
static int get_sort_mode( bool bNoSort, bool bLeftKiruda, bool bLeftAce )
{
	return bNoSort ? 0
		: bLeftKiruda ?   bLeftAce ? 1 : 2
						: bLeftAce ? 3 : 4;
}
static void parse_sort_mode( bool& bNoSort, bool& bLeftKiruda, bool& bLeftAce, int nMode )
{
	if ( nMode == 0 ) { bNoSort = true; return; }
	bNoSort = false;
	bLeftKiruda = nMode <= 2;
	bLeftAce = !!(nMode % 2);
}

/////////////////////////////////////////////////////////////////////////////
// POptionGeneral property page

IMPLEMENT_DYNCREATE(POptionGeneral, CPropertyPage)

POptionGeneral::POptionGeneral() : CPropertyPage(POptionGeneral::IDD)
{
	//{{AFX_DATA_INIT(POptionGeneral)
	m_bClockwise = Mo()->bClockwise;
	m_bUseTerm = Mo()->bUseTerm;
	m_bUseSound = Mo()->bUseSound;
	m_nSortMode = get_sort_mode( Mo()->bNoSort, Mo()->bLeftKiruda, Mo()->bLeftAce );
	m_sHumanName = Mo()->aPlayer[0].sName;
	m_bMoneyAsSupport = Mo()->bMoneyAsSupport;
	//}}AFX_DATA_INIT
}

POptionGeneral::~POptionGeneral()
{
}

void POptionGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(POptionGeneral)
	DDX_Check(pDX, IDC_CLOCKWISE, m_bClockwise);
	DDX_Check(pDX, IDC_USETERM, m_bUseTerm);
	DDX_Check(pDX, IDC_USESOUND, m_bUseSound);
	DDX_Radio(pDX, IDC_SORTMODE, m_nSortMode);
	DDX_Text(pDX, IDC_HUMANNAME, m_sHumanName);
	DDV_MaxChars(pDX, m_sHumanName, 24);
	DDX_Check(pDX, IDC_MONEYASSUPPORT, m_bMoneyAsSupport);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate ) {
		parse_sort_mode( Mo()->bNoSort, Mo()->bLeftKiruda,
			Mo()->bLeftAce, m_nSortMode );
		Mo()->bClockwise = !!m_bClockwise;
		Mo()->bUseTerm = !!m_bUseTerm;
		Mo()->bUseSound = !!m_bUseSound;
		Mo()->aPlayer[0].sName = m_sHumanName;
		Mo()->bMoneyAsSupport = !!m_bMoneyAsSupport;
	}
}


BEGIN_MESSAGE_MAP(POptionGeneral, CPropertyPage)
	//{{AFX_MSG_MAP(POptionGeneral)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
