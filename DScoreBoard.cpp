// DScoreBoard.cpp : implementation file
//

#include "stdafx.h"
#include "Mighty.h"
#include "DScoreBoard.h"

#include "Play.h"
#include "Player.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// DScoreBoard dialog

#define ALLCATEGORIES 10
#define HALFCATEGORIES 6


DScoreBoard::DScoreBoard(CWnd* pParent /*=NULL*/)
	: CDialog(DScoreBoard::IDD, pParent)
{
	m_bShowing = false;
	m_nPos = 0;
	m_nPosMax = 0;

	m_nCategories = HALFCATEGORIES;

	//{{AFX_DATA_INIT(DScoreBoard)
	//}}AFX_DATA_INIT

	raw_Update(0);
}


void DScoreBoard::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DScoreBoard)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DScoreBoard, CDialog)
	//{{AFX_MSG_MAP(DScoreBoard)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_SIZE()
	ON_WM_SIZING()
	//}}AFX_MSG_MAP
	ON_MESSAGE( WM_UPDATESCOREBOARD, OnUpdateScoreBoard )
END_MESSAGE_MAP()

// 스코어 보드를 보이거나 감춘다
// 윈도우가 생성되지 않았다면 생성한다
void DScoreBoard::Show( bool bShow )
{
	// 옵션에 반영한다
	Mo()->bLastScoreboard = bShow;

	// 윈도우가 생성되지 않았다면 생성한다
	if ( !GetSafeHwnd() ) {
		VERIFY( Create(IDD) );
		SetWindowPos(  0,
			Mo()->pntLastScoreboard.x,
			Mo()->pntLastScoreboard.y,
			0, 0,
			SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE
			|(bShow?SWP_SHOWWINDOW:SWP_HIDEWINDOW) );
		m_bShowing = bShow;
	}

	if ( !bShow ) {	// 감추기
		if ( m_bShowing ) { // 이미 보여지고 있었다면
			ShowWindow(SW_HIDE);	// 감춘다
			m_bShowing = false;
		}
	}
	else {
		if ( !m_bShowing ) {// 화면에 보여줘야 하는 상황
			ShowWindow(SW_SHOWNOACTIVATE);
			m_bShowing = true;
		}
	}

	if ( Mw() ) Mw()->SetActiveWindow();
}

// 현재 스코어를 Update 한다

void DScoreBoard::Update( const CState* pState )
{	SendMessage( WM_UPDATESCOREBOARD, 0, (LPARAM)(LPVOID)pState ); }
LRESULT DScoreBoard::OnUpdateScoreBoard( WPARAM, LPARAM lParam )
{	raw_Update( (const CState*)(LPVOID)lParam ); return 0; }

static void format_ratio( CString& s, int w, int l )
{
	int rat = !(w+l) ? 0 : w * 1000 / (w+l);
	s.Format(_T("%d/%d (%d.%d%%)"), w, l, rat / 10, rat % 10 );
}

void DScoreBoard::raw_Update( const CState* pState )
{
	static LPCTSTR s_asTab[ALLCATEGORIES] = {
		_T("이름"), 0,
		_T("주공전적"), _T("프랜드전적"), _T("야당전적"), _T("종합전적"),
		_T("통산주공전적"), _T("통산프랜드전적"), _T("통산야당전적"), _T("통산종합전적"),
	};

	int i, j;

	if ( pState && pState->IsNetworkGame() )
		m_nCategories = ALLCATEGORIES;
	else m_nCategories = HALFCATEGORIES;

	for ( i = 0; i < m_nCategories; i++ )
		if ( i != 1 ) m_aasData[i][MAX_PLAYERS] = s_asTab[i];
		else if ( Mo()->bMoneyAsSupport )
			m_aasData[i][MAX_PLAYERS] = _T("지지율");
		else m_aasData[i][MAX_PLAYERS] = _T("비자금");

	if ( !pState ) {
		for ( i = 0; i < MAX_PLAYERS; i++ ) {
			m_aasData[0][i] = Mo()->aPlayer[i].sName;
			for ( j = 1; j < m_nCategories; j++ ) m_aasData[j][i].Empty();
		}
	}
	else {
		for ( i = 0; i < pState->pRule->nPlayerNum; i++ ) {

			const GAME_RECORD& grCur =
				pState->apAllPlayers[i]->GetCurRecord();
			const GAME_RECORD& grAll =
				pState->apAllPlayers[i]->GetAllRecord();

			m_aasData[0][i] = pState->apAllPlayers[i]->GetName();
			m_aasData[1][i] = ::FormatMoney( pState->apAllPlayers[i]->GetMoney(),
								pState->pRule->nPlayerNum );
			format_ratio( m_aasData[2][i], grCur.wm, grCur.lm );
			format_ratio( m_aasData[3][i], grCur.wf, grCur.lf );
			format_ratio( m_aasData[4][i], grCur.wa, grCur.la );
			format_ratio( m_aasData[5][i],
				grCur.wm + grCur.wf + grCur.wa,
				grCur.lm + grCur.lf + grCur.la );

			if ( m_nCategories > HALFCATEGORIES ) {
				format_ratio( m_aasData[6][i], grAll.wm, grAll.lm );
				format_ratio( m_aasData[7][i], grAll.wf, grAll.lf );
				format_ratio( m_aasData[8][i], grAll.wa, grAll.la );
				format_ratio( m_aasData[9][i],
					grAll.wm + grAll.wf + grAll.wa,
					grAll.lm + grAll.lf + grAll.la );
			}
		}
		for ( ; i < MAX_PLAYERS; i++ )
			for ( j = 0; j < m_nCategories; j++ )
				m_aasData[j][i].Empty();
	}


	if ( GetSafeHwnd() ) {
		RecalcScrollBar();
		Invalidate();
	}
}

/////////////////////////////////////////////////////////////////////////////
// DScoreBoard message handlers

void DScoreBoard::OnDestroy() 
{
	CDialog::OnDestroy();

	CRect rc; GetWindowRect( &rc );
	Mo()->pntLastScoreboard = rc.TopLeft();
}

void DScoreBoard::OnClose() 
{
	m_bShowing = false;
	Mo()->bLastScoreboard = false;

	CDialog::OnClose();
}

// 주어진 스트링 배열중 가장 긴 스트링을 포함하는
// 폭을 리턴한다
int DScoreBoard::CalcMaxWidth( CString as[], int nCount, CDC* pDC )
{
	int nMax = 0;
	for ( int i = 0; i < nCount; i++ ) {
		CSize sz = pDC->GetTextExtent( as[i] );
		nMax = max( nMax, sz.cx );
	}
	return nMax;
}

void DScoreBoard::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CRect rc; GetClientRect( &rc );

	int nNewPos = m_nPos;

	switch ( nSBCode ) {
	case SB_LEFT:
	case SB_LINELEFT:	nNewPos -= rc.Width()/8; break;
	case SB_RIGHT:
	case SB_LINERIGHT:	nNewPos += rc.Width()/8; break;
	case SB_PAGELEFT:	nNewPos -= rc.Width(); break;
	case SB_PAGERIGHT:	nNewPos += rc.Width(); break;
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:	nNewPos = nPos; break;
	}

	m_nPos = max( 0, min( nNewPos, m_nPosMax ) );

	RecalcScrollBar();
	Invalidate();

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void DScoreBoard::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	RecalcScrollBar();
}

// 스크롤바를 갱신
void DScoreBoard::RecalcScrollBar()
{
	// m_nMaxPos 를 갱신한다 (전체 스크롤 크기를 알아냄)
	{
		CClientDC dc(this);
		dc.SelectStockObject( DEFAULT_GUI_FONT );
		int nBorder = ::GetSystemMetrics( SM_CXVSCROLL );
		m_nPosMax = nBorder;
		for ( int i = 0; i < m_nCategories; i++ )
			m_nPosMax += CalcMaxWidth( m_aasData[i], MAX_PLAYERS+1, &dc )
						+ nBorder;
	}

	CRect rc; GetClientRect( &rc );

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	si.nMax = m_nPosMax;
	si.nMin = 0;
	si.nPage = rc.Width();
	si.nPos = m_nPos;

	SetScrollInfo( SB_HORZ, &si );
}

static int get_y( CWnd* pWnd, UINT ctlID, int* pCy )
{
	CRect rc;
	pWnd->GetDlgItem( ctlID )->GetWindowRect( &rc );
	pWnd->ScreenToClient( &rc );
	*pCy = rc.Height();
	return rc.top;
}

void DScoreBoard::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	dc.SetWindowOrg( CPoint(m_nPos,0) );
	dc.SetTextAlign( TA_TOP | TA_LEFT );
	dc.SetBkMode( TRANSPARENT );
	dc.SetROP2( R2_COPYPEN );
	dc.SetTextColor( ::GetSysColor( COLOR_BTNTEXT ) );
	dc.SelectStockObject( DEFAULT_GUI_FONT );

	LOGFONT lf;
	dc.GetCurrentFont()->GetLogFont( &lf );
	lf.lfWeight = FW_BOLD;
	CFont fontBold;
	fontBold.CreateFontIndirect( &lf );

	// TODO: Add your message handler code here
	
	// Do not call CDialog::OnPaint() for painting messages

	int i;
	int nBorder = ::GetSystemMetrics( SM_CXVSCROLL );
	int x = nBorder;

	int y[MAX_PLAYERS+1], cy[MAX_PLAYERS+1];
	y[MAX_PLAYERS] = get_y( this, IDC_SB_TAB, &cy[MAX_PLAYERS] );
	for ( i = 0; i < MAX_PLAYERS; i++ )
		y[i] = get_y( this, IDC_SB_0 + i, &cy[i] );

	for ( int cat = 0; cat < m_nCategories; cat++ ) {

		// 타이틀
		dc.TextOut( x, y[MAX_PLAYERS], m_aasData[cat][MAX_PLAYERS] );

		for ( i = 0; i < MAX_PLAYERS; i++ ) {

			if ( i == 0 ) dc.SelectObject( &fontBold );
			dc.TextOut( x, y[i], m_aasData[cat][i] );
			if ( i == 0 ) dc.SelectStockObject( DEFAULT_GUI_FONT );
		}

		x += CalcMaxWidth( m_aasData[cat], MAX_PLAYERS+1, &dc )
			+ nBorder;
	}
}

void DScoreBoard::OnSizing(UINT fwSide, LPRECT pRect) 
{
	// 세로 크기가 안변하도록 한다
	CDialog::OnSizing(fwSide, pRect);
	CRect rc; GetWindowRect( &rc );
	pRect->bottom = pRect->top + rc.Height();
	// 가로 크기는 최대 값을 설정한다
	int nMaxWidth = m_nPosMax
		+ ::GetSystemMetrics( SM_CXSIZEFRAME ) * 2 + 2;
	pRect->right = pRect->left + min( pRect->right - pRect->left,
										nMaxWidth );
}
