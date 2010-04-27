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
	m_sMightyNetAddress = Mo()->sMightyNetAddress;
	m_nPort = Mo()->nPort;
	m_sAddress = Mo()->sAddress;
	//}}AFX_DATA_INIT
}

POptionComm::~POptionComm()
{
}

void POptionComm::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(POptionComm)
	DDX_Text(pDX, IDC_MIGHTYNETADDRESS, m_sMightyNetAddress);
	DDX_Text(pDX, IDC_PORT, m_nPort);
	DDX_Text(pDX, IDC_ADDRESS, m_sAddress);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate ) {
		Mo()->sMightyNetAddress = m_sMightyNetAddress;
		Mo()->nPort = m_nPort;
		m_sAddress.TrimLeft();
		m_sAddress.TrimRight();
		Mo()->sAddress = m_sAddress;
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
