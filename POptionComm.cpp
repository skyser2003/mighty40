// POptionComm.cpp : implementation file
//

#include "stdafx.h"
#include "mighty.h"
#include "POptionComm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// POptionComm property page

IMPLEMENT_DYNCREATE(POptionComm, CPropertyPage)

static void format_state( CString* pStr )
{
	pStr->Format( _T("%d승%d패\n%d승%d패\n%d승%d패"),
		LOWORD(Mo()->anPlayerState[0]),
		HIWORD(Mo()->anPlayerState[0]),
		LOWORD(Mo()->anPlayerState[1]),
		HIWORD(Mo()->anPlayerState[1]),
		LOWORD(Mo()->anPlayerState[2]),
		HIWORD(Mo()->anPlayerState[2]) );
}

POptionComm::POptionComm() : CPropertyPage(POptionComm::IDD)
{
	//{{AFX_DATA_INIT(POptionComm)
	m_sMightyNetAddress = Mo()->sMightyNetAddress;
	m_nPort = Mo()->nPort;
	m_sAddress = Mo()->sAddress;
	//}}AFX_DATA_INIT
	format_state( &m_sScore );
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
	DDX_Text(pDX, IDC_SCORE, m_sScore);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate ) {
		Mo()->sMightyNetAddress = m_sMightyNetAddress;
		Mo()->nPort = m_nPort;
		m_sAddress.TrimLeft();
		m_sAddress.TrimRight();
		Mo()->sAddress = m_sAddress;
	}
}


BEGIN_MESSAGE_MAP(POptionComm, CPropertyPage)
	//{{AFX_MSG_MAP(POptionComm)
	ON_BN_CLICKED(IDC_RESETSCORE, OnResetscore)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// POptionComm message handlers

void POptionComm::OnResetscore() 
{
	if ( AfxMessageBox( _T("지금까지 쌓아왔던 전적을 삭제하겠습니까?"),
					MB_YESNO | MB_ICONQUESTION )
		== IDYES ) {

		Mo()->anPlayerState[0] = 0;
		Mo()->anPlayerState[1] = 0;
		Mo()->anPlayerState[2] = 0;
		format_state( &m_sScore );
		SetDlgItemText( IDC_SCORE, m_sScore );
	}
}
