// POptionComm.cpp : implementation file
//

#include "stdafx.h"
#include "Mighty.h"
#include "POptionComm.h"
#include "DDetailScore.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// POptionComm property page

IMPLEMENT_DYNCREATE(POptionComm, CPropertyPage)

POptionComm::POptionComm() : CPropertyPage(POptionComm::IDD)
{
	//{{AFX_DATA_INIT(POptionComm)
	m_bObserver = Mo()->bObserver;
	m_bSameName = Mo()->bSameName;
	m_nPort = Mo()->nPort;
	m_sAddress = Mo()->sAddress;
	m_nChatDSBTimeOut = Mo()->nChatDSBTimeOut;
	//}}AFX_DATA_INIT
}

POptionComm::~POptionComm()
{
}

void POptionComm::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(POptionComm)
	DDX_Check(pDX, IDC_OBSERVER, m_bObserver);
	DDX_Check(pDX, IDC_SAMENAME, m_bSameName);
	DDX_Text(pDX, IDC_PORT, m_nPort);
	DDX_Text(pDX, IDC_ADDRESS, m_sAddress);
	DDX_Slider(pDX, IDC_CHATDSBTIMEOUT, m_nChatDSBTimeOut);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate ) {
		Mo()->bObserver = !!m_bObserver;
		Mo()->bSameName = !!m_bSameName;
		Mo()->nPort = m_nPort;
		m_sAddress.TrimLeft();
		m_sAddress.TrimRight();
		Mo()->sAddress = m_sAddress;
		Mo()->nChatDSBTimeOut = (int)m_nChatDSBTimeOut;
	}
}

BEGIN_MESSAGE_MAP(POptionComm, CDialog)
	//{{AFX_MSG_MAP(DDetailScore)
	ON_BN_CLICKED(IDC_DETAILSCORE, OnDetailScore)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// POptionComm message handlers

void POptionComm::OnDetailScore() 
{
	DDetailScore dlg;
	dlg.DoModal();
}


BOOL POptionComm::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// TODO:  Add extra initialization here

	((CSliderCtrl*)GetDlgItem(IDC_CHATDSBTIMEOUT))
		->SetRange( 0, 10, TRUE );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
