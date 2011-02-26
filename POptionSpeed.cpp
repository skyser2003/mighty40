// POptionSpeed.cpp : implementation file
//

#include "stdafx.h"
#include "Mighty.h"
#include "POptionSpeed.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// POptionSpeed property page

IMPLEMENT_DYNCREATE(POptionSpeed, CPropertyPage)

POptionSpeed::POptionSpeed() : CPropertyPage(POptionSpeed::IDD)
{
	//{{AFX_DATA_INIT(POptionSpeed)
	m_nCardMotion = 10 - Mo()->nCardMotion;
	m_nCardSpeed = Mo()->nCardSpeed;
	m_nDelayOneCard = Mo()->nDelayOneCard;
	m_nDelayOneTurn = Mo()->nDelayOneTurn;
	m_bShowDealing = Mo()->bShowDealing;
	m_bSlowMachine = !Mo()->bSlowMachine;
	//}}AFX_DATA_INIT
}

POptionSpeed::~POptionSpeed()
{
}

void POptionSpeed::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(POptionSpeed)
	DDX_Slider(pDX, IDC_CARDMOTION, m_nCardMotion);
	DDX_Slider(pDX, IDC_CARDSPEED, m_nCardSpeed);
	DDX_Slider(pDX, IDC_DELAYONECARD, m_nDelayOneCard);
	DDX_Slider(pDX, IDC_DELAYONETURN, m_nDelayOneTurn);
	DDX_Check(pDX, IDC_SHOWDEALING, m_bShowDealing);
	DDX_Check(pDX, IDC_SLOWMACHINE, m_bSlowMachine);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate ) {
		Mo()->nCardMotion = 10 - m_nCardMotion;
		Mo()->nCardSpeed = m_nCardSpeed;
		Mo()->nDelayOneCard = m_nDelayOneCard;
		Mo()->nDelayOneTurn = m_nDelayOneTurn;
		Mo()->bShowDealing = !!m_bShowDealing;
		Mo()->bSlowMachine = !m_bSlowMachine;
	}
}


BEGIN_MESSAGE_MAP(POptionSpeed, CPropertyPage)
	//{{AFX_MSG_MAP(POptionSpeed)
	ON_BN_CLICKED(IDC_SLOWMACHINE, OnSlowmachine)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// POptionSpeed message handlers

BOOL POptionSpeed::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here

	((CSliderCtrl*)GetDlgItem(IDC_CARDSPEED))->SetRange( 0, 10, TRUE );
	((CSliderCtrl*)GetDlgItem(IDC_CARDMOTION))->SetRange( 0, 10, TRUE );
	((CSliderCtrl*)GetDlgItem(IDC_DELAYONECARD))->SetRange( 0, 10,TRUE );
	((CSliderCtrl*)GetDlgItem(IDC_DELAYONETURN))->SetRange( 0, 10, TRUE );

	OnSlowmachine();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void POptionSpeed::OnSlowmachine() 
{
	BOOL bEnable = ((CButton*)GetDlgItem(IDC_SLOWMACHINE))->GetCheck() & 1 ? TRUE : FALSE;
	GetDlgItem(IDC_CARDSPEED)->EnableWindow(bEnable);
	GetDlgItem(IDC_CARDMOTION)->EnableWindow(bEnable);
	GetDlgItem(IDC_CARDSPEEDGROUP)->EnableWindow(bEnable);
	GetDlgItem(IDC_CARDMOTIONGROUP)->EnableWindow(bEnable);
}
