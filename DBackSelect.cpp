// DBackSelect.cpp : implementation file
//

#include "stdafx.h"
#include "Mighty.h"
#include "DBackSelect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DBackSelect dialog


DBackSelect::DBackSelect(CWnd* pParent /*=NULL*/)
	: CDialog(DBackSelect::IDD, pParent)
{
	//{{AFX_DATA_INIT(DBackSelect)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nBackPicture = Mo()->nBackPicture;
	m_sBackPicturePath = Mo()->sBackPicturePath;
}


void DBackSelect::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DBackSelect)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DBackSelect, CDialog)
	//{{AFX_MSG_MAP(DBackSelect)
	ON_BN_CLICKED(IDC_BACKBROWSE, OnBackbrowse)
	//}}AFX_MSG_MAP
	ON_CONTROL_RANGE(BN_CLICKED,IDC_BACKPIC1,IDC_BACKPIC11,OnCardsClick)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DBackSelect message handlers

void DBackSelect::OnCardsClick( UINT nID )
{
	// 미리 정의된 카드 중 하나를 눌렀을 때
	int nIndex = int( nID - IDC_BACKPIC1 );
	m_nBackPicture = nIndex;
	EndDialog(IDOK);
}

void DBackSelect::OnBackbrowse() 
{
	// TODO: Add your control notification handler code here

	CFileDialog dlg( TRUE,	// open
		_T(".bmp"), NULL,
		OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NONETWORKBUTTON,
		_T("비트맵 파일 (*.bmp)|*.bmp|모든파일 (*.*)|*.*||"),
		this );

	if ( dlg.DoModal() == IDOK ) {
		m_nBackPicture = -1;
		m_sBackPicturePath = dlg.GetPathName();
		EndDialog(IDOK);
	}
}
