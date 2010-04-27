// POptionChat.cpp : implementation file
//

#include "stdafx.h"
#include "Mighty.h"
#include "POptionChat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// POptionChat property page

IMPLEMENT_DYNCREATE(POptionChat, CPropertyPage)

POptionChat::POptionChat() : CPropertyPage(POptionChat::IDD)
{
	//{{AFX_DATA_INIT(POptionChat)
	m_nChatBufferSize = (UINT)Mo()->nChatBufferSize;
	m_nChatDSBTimeOut = Mo()->nChatDSBTimeOut;
	//}}AFX_DATA_INIT
}

POptionChat::~POptionChat()
{
}

void POptionChat::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(POptionChat)
	DDX_Text(pDX, IDC_CHATBUFFERSIZE, m_nChatBufferSize);
	DDX_Slider(pDX, IDC_CHATDSBTIMEOUT, m_nChatDSBTimeOut);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate ) {
		Mo()->nChatBufferSize = (int)m_nChatBufferSize;
		Mo()->nChatDSBTimeOut = (int)m_nChatDSBTimeOut;
	}
}


BEGIN_MESSAGE_MAP(POptionChat, CPropertyPage)
	//{{AFX_MSG_MAP(POptionChat)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// POptionChat message handlers

BOOL POptionChat::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here

	((CSpinButtonCtrl*)GetDlgItem(IDC_CHATBUFFERSIZESPIN))
		->SetRange( 1, 16 );
	((CSliderCtrl*)GetDlgItem(IDC_CHATDSBTIMEOUT))
		->SetRange( 0, 10, TRUE );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
