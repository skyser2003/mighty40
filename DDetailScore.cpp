// DDetailScore.cpp : implementation file
//

#include "stdafx.h"
#include "Mighty.h"
#include "DDetailScore.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static void format_state( int k, CString* pStr )
{
	if ( k == 0 )	// 2¸¶
	pStr->Format( _T("%d½Â%dÆÐ\n\n%d½Â%dÆÐ"),
		LOWORD(Mo()->anPlayerState[k][0]),
		HIWORD(Mo()->anPlayerState[k][0]),
		LOWORD(Mo()->anPlayerState[k][2]),
		HIWORD(Mo()->anPlayerState[k][2]) );
	else
	pStr->Format( _T("%d½Â%dÆÐ\n%d½Â%dÆÐ\n%d½Â%dÆÐ"),
		LOWORD(Mo()->anPlayerState[k][0]),
		HIWORD(Mo()->anPlayerState[k][0]),
		LOWORD(Mo()->anPlayerState[k][1]),
		HIWORD(Mo()->anPlayerState[k][1]),
		LOWORD(Mo()->anPlayerState[k][2]),
		HIWORD(Mo()->anPlayerState[k][2]) );
}

/////////////////////////////////////////////////////////////////////////////
// DDetailScore property page

DDetailScore::DDetailScore(CWnd* pParent /*=NULL*/)
	: CDialog(DDetailScore::IDD, pParent)
{
	//{{AFX_DATA_INIT(POptionComm)
	//}}AFX_DATA_INIT
	for ( int i = 0; i < MAX_PLAYERS - 1; i++ )
		format_state( i, &m_sScore[i] );
}

void DDetailScore::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(POptionComm)
	//}}AFX_DATA_MAP
	for ( int i = 0; i < 6; i++ )
		DDX_Text(pDX, IDC_SCORE1 + i, m_sScore[i]);
}


BEGIN_MESSAGE_MAP(DDetailScore, CDialog)
	//{{AFX_MSG_MAP(DDetailScore)
	ON_BN_CLICKED(IDC_RESETSCORE, OnResetscore)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DDetailScore message handlers

void DDetailScore::OnResetscore() 
{
	if ( AfxMessageBox( _T("Áö±Ý±îÁö ½×¾Æ¿Ô´ø ÀüÀûÀ» »èÁ¦ÇÏ°Ú½À´Ï±î?"),
					MB_YESNO | MB_ICONQUESTION )
		== IDYES ) {

		for ( int i = 0; i < MAX_PLAYERS - 1; i++ ) {
			for ( int j = 0; j < 3; j++ )
				Mo()->anPlayerState[i][j] = 0;
			format_state( i, &m_sScore[i] );
			SetDlgItemText( IDC_SCORE1 + i, m_sScore[i] );
		}
	}
}
