// InfoBar.cpp: implementation of the CInfoBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"
#include "InfoBar.h"

#include "BmpMan.h"
#include "Board.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// 채팅 버퍼 구현
struct CInfoBar::CChatBuffer : CList<CString,CString&>
{
	void Push( LPCTSTR s );	// 새로운 스트링이 입력됨
	CString Up();	// 하나 위로
	CString Down();	// 하나 아래로

	CChatBuffer() { m_pos = 0; }
	POSITION m_pos;
};

void CInfoBar::CChatBuffer::Push( LPCTSTR s )
{
	m_pos = 0;
	if ( !IsEmpty() && !GetTail().Compare(s) ) return;

	AddTail( CString(s) );
	while ( GetCount() > Mo()->nChatBufferSize )
		RemoveHead();
}

CString CInfoBar::CChatBuffer::Up()
{
	if ( !m_pos ) {
		m_pos = GetTailPosition();
		return m_pos ? GetAt(m_pos) : CString();
	}
	else {
		if ( GetHeadPosition() == m_pos ) return GetAt(m_pos);
		else {
			GetPrev(m_pos);	ASSERT(m_pos);
			return GetAt(m_pos);
		}
	}
}

CString CInfoBar::CChatBuffer::Down()
{
	if ( !m_pos ) return CString();
	GetNext(m_pos);
	if ( !m_pos ) return CString();
	else return GetAt(m_pos);
}


/////////////////////////////////////////////////////////////////////////////
// CInfoBar

BEGIN_MESSAGE_MAP(CInfoBar,CStatusBar )
	ON_WM_SIZE()
	ON_MESSAGE( WM_INFOBARUPDATE, OnInfoBarUpdate )
	ON_MESSAGE( WM_SETTEXT, OnSetText )
	ON_NOTIFY_EX( TTN_NEEDTEXTW, 0, OnToolTipNeedTextW )
	ON_NOTIFY_EX( TTN_NEEDTEXTA, 0, OnToolTipNeedTextA )
END_MESSAGE_MAP()


CInfoBar::CInfoBar()
{
	m_bChat = false;
	m_nMinScore = m_nDefaultMinScore = 0;
	m_nKiruda = 0;
	m_nPercentage = 0;
	m_pChatBuffer = new CChatBuffer;

	m_handler.pfnProc = 0;
	m_handler.dwUser = 0;
}

CInfoBar::~CInfoBar()
{
	delete m_pChatBuffer;
}

BOOL CInfoBar::Create( CFrameWnd* pParent )
{
	if ( !CreateEx( pParent, 0,
			WS_CHILD | WS_VISIBLE | CBRS_BOTTOM
			| WS_CLIPCHILDREN,
			AFX_IDW_STATUS_BAR ) )
		return FALSE;

	EnableToolTips();

	// 4개의 패인 : 기루다, 목표점수, 프랜드(진행바), 텍스트
	int aWidths[4];

	// 기루다와 목표점수가 들어갈만한 크기를 확보한다
	CSize sizeBitmap = GetBitmapSize(IDB_SPADE);

	int nHorz, nVert, nSpacing;
	GetStatusBarCtrl().GetBorders( nHorz, nVert, nSpacing );

	CClientDC dc(this);
	aWidths[0] = sizeBitmap.cx + nSpacing;
	aWidths[1] = aWidths[0]
		+ dc.GetTextExtent( _T("WW"), 2 ).cx + nSpacing;
	CString s(_T("스페이드"));
	aWidths[2] = aWidths[1] + dc.GetTextExtent(s).cx * 3 / 2 + nSpacing;	// 4.0에서 약간 키움
	aWidths[3] = -1;

	// 패인을 만들고 속성을 설정
	SetFont( CFont::FromHandle( (HFONT)GetStockObject(SYSTEM_FONT) ) );
	GetStatusBarCtrl().SetMinHeight( sizeBitmap.cy );
	GetStatusBarCtrl().SetParts( 4, aWidths );
	GetStatusBarCtrl().SetText( 0, 0, SBT_OWNERDRAW );
	GetStatusBarCtrl().SetText( _T(""), 1, 0 );
	GetStatusBarCtrl().SetText( _T(""), 2, SBT_OWNERDRAW );
	GetStatusBarCtrl().SetText( _T(""), 3, SBT_NOBORDERS );

	Reset();

	return TRUE;
}

LRESULT CInfoBar::OnInfoBarUpdate( WPARAM wParam, LPARAM lParam )
{
	switch (wParam) {

	case 0:	// Reset()
	{
		SetKiruda( 0 );
		SetProgress( 0 );
		SetMinScore( 0, 0 );
		SetFriendText( _T("") );
		CString sText; sText.LoadString( AFX_IDS_IDLEMESSAGE );
		SetText( sText );

	} break;

	case 1:	// SetKiruda()
	{
		int nKiruda = (int)lParam;
		ASSERT(nKiruda>=0 && nKiruda<=CLOVER);
		m_nKiruda = nKiruda;

		if ( GetSafeHwnd() ) {
			Invalidate(FALSE);
			UpdateWindow();
		}

	} break;

	case 2:	// SetMinScore()
	{
		m_nMinScore = (int)LOWORD(lParam);
		m_nDefaultMinScore = (int)HIWORD(lParam);
		CString sText;
		if ( m_nMinScore > 0 )
			sText.Format( _T("%d"), m_nMinScore );
		else sText.Empty();

		GetStatusBarCtrl().SetText( sText, 1, 0 );

	} break;

	case 3:	// SetFriendText()
	{
		LPCTSTR sFriendText = (LPCTSTR)lParam;
		m_sFriend = sFriendText;

		if ( GetSafeHwnd() ) {
			Invalidate(FALSE);
			UpdateWindow();
		}

	} break;

	case 4:	// SetProgress()
	{
		int nPercentage = (int)lParam;

		ASSERT(nPercentage>=0 && nPercentage<=100);
		m_nPercentage = max( 0, min( nPercentage, 100 ) );

		if ( GetSafeHwnd() ) {
			// 그부분만 Update
			CRect rcProgress;
			GetItemRect( 2, &rcProgress );
			InvalidateRect( &rcProgress, FALSE );
			UpdateWindow();
		}

	} break;

	case 5:	// SetText()
	{
		LPCTSTR sText = (LPCTSTR)lParam;
		GetStatusBarCtrl().SetText( sText, 3, SBT_NOBORDERS );

	} break;

	case 6:	// ShowChat()
	{
		bool bShow = lParam != 0 ? true : false;

		if ( m_bChat && bShow || !m_bChat && !bShow ) {
			// 상태를 변화할 필요가 없을 때임
		}
		else if ( m_bChat && !bShow ) {
			// 감춘다
			if ( m_cChat.GetSafeHwnd() )
				m_cChat.ShowWindow( SW_HIDE );
			m_bChat = false;
		}
		else {
			// 보인다
			if ( !m_cChat.GetSafeHwnd() ) {
				// 컨트롤이 안만들어 졌었다면 새로 만든다
				CRect rcItem;
				GetItemRect( 3, &rcItem );
				m_cChat.Create(
					WS_CHILD | ES_AUTOHSCROLL | ES_LEFT,
					rcItem, this, 1000);
				m_cChat.ModifyStyleEx( 0, WS_EX_STATICEDGE, SWP_DRAWFRAME );
			}
			m_cChat.ShowWindow( SW_SHOW );
			m_cChat.SetFocus();
			m_bChat = true;
		}

	} break;

	case 7:	// SetFocusToChat()
	{
		if ( m_bChat && m_cChat.GetSafeHwnd() ) {
			m_cChat.SetFocus();
			return 1;
		}
		else return 0;

	} break;

	case 8:	// SetChatHandler()
	{
		const CHATHANDLERINFO* pCHI =
			(const CHATHANDLERINFO*)(LPCVOID)lParam;

		ASSERT( pCHI );

		if ( !pCHI->pfnProc ) {	// 핸들러 해제중
			// dwUser 값이 같아야 해제 가능하다
			if ( pCHI->dwUser == m_handler.dwUser ) {
				m_handler.pfnProc = 0;
				m_handler.dwUser = 0;
			}
		}
		else m_handler = *pCHI;

	} break;

	default:
	{
		ASSERT(0);

	} break;

	}

	return 0;
}

// 각 패인 아이템을 그린다
void CInfoBar::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	static const UINT arcBitmap[] = {
		IDB_NOTHING, IDB_SPADE, IDB_DIAMOND, IDB_HEART, IDB_CLOVER };

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rc = lpDrawItemStruct->rcItem;

	if ( lpDrawItemStruct->itemID == 0 ) {
		// 왼쪽 패인
		DrawBitmap( pDC, arcBitmap[m_nKiruda], rc.left, rc.top );
	}
	else {
		// 오른쪽 패인 그리기
		DrawMainPane( pDC );
	}
}

// 오른쪽 텍스트 패인 그리기
void CInfoBar::DrawMainPane( CDC* pDC )
{
	CRect rc; GetItemRect( 2, &rc );
	rc.DeflateRect( 1, 1 );	// 실제 그려지는 영역은 아이템 테두리를 제외

	if ( m_nPercentage == 0 ) {
		// 프랜드 텍스트 출력 후 종료
		pDC->FillSolidRect( &rc, GetSysColor( COLOR_BTNFACE ) );
		pDC->SetBkMode( TRANSPARENT );
		pDC->SetTextAlign( TA_TOP | TA_LEFT );
		pDC->SetTextColor( GetSysColor(COLOR_3DHILIGHT) );
		pDC->TextOut( rc.left+2, rc.top+2, m_sFriend );
		pDC->SetTextColor( GetSysColor(COLOR_3DSHADOW) );
		pDC->TextOut( rc.left+1, rc.top+1, m_sFriend );
		return;
	}

	pDC->SetBkMode( OPAQUE );
	pDC->SetTextAlign( TA_CENTER | TA_TOP );

	CString sText;
	sText.Format( _T("%d%%"), m_nPercentage );

	// 반전 사각형
	CRect rcInv( rc.left, rc.top,
		rc.left + (rc.right-rc.left)*m_nPercentage/100,
		rc.bottom );
	// 정상 사각형
	CRect rcNormal( rcInv.right, rc.top,
		rc.right, rc.bottom );

	pDC->IntersectClipRect( &rcNormal );

	pDC->SetTextColor( GetSysColor( COLOR_BTNTEXT ) );
	pDC->SetBkColor( GetSysColor( COLOR_BTNFACE ) );
	pDC->ExtTextOut( (rc.left+rc.right)/2, rc.top+2,
		ETO_OPAQUE,
		&rc, sText, NULL );

	pDC->SelectClipRgn( NULL );
	pDC->IntersectClipRect( &rcInv );

	// 반전된 부분
	pDC->SetTextColor( GetSysColor( COLOR_CAPTIONTEXT ) );
	pDC->SetBkColor( GetSysColor( COLOR_ACTIVECAPTION ) );
	pDC->ExtTextOut( (rc.left+rc.right)/2, rc.top+2,
		ETO_CLIPPED|ETO_OPAQUE,
		&rcInv, sText, NULL );
}

// 이상하게 CStatusBar 는 TTN_NEEDTEXT 가 아니라 TTN_NEEDTEXTW 를 보낸다!!!
int CInfoBar::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const
{
	CRect rc; GetClientRect( &rc );
	if ( !rc.PtInRect(point) ) return -1;

	if ( pTI ) {
		pTI->cbSize = sizeof(TOOLINFO);
		pTI->uFlags = TTF_TRANSPARENT;
		pTI->hwnd = GetSafeHwnd();
		pTI->uId = 0;
		pTI->hinst = 0;
		pTI->lpszText = LPSTR_TEXTCALLBACK;
		CRect rc1, rc2;
		GetItemRect( 0, &rc1 ); GetItemRect( m_bChat ? 2 : 3, &rc2 );
		pTI->rect.left = rc1.left;
		pTI->rect.top = rc1.top;
		pTI->rect.right = rc2.right;
		pTI->rect.bottom = rc2.bottom;
	}
	return 1;
}

BOOL CInfoBar::OnToolTipNeedTextW( UINT /*id*/, NMHDR * pNMHDR, LRESULT * /*pResult*/ )
{
	TOOLTIPTEXTW *pTTT = (TOOLTIPTEXTW *)pNMHDR;
#ifdef _UNICODE
	wcscpy( pTTT->szText, GetGoalString() );
#else
	if ( _mbstowcsz( pTTT->szText, GetGoalString(), sizeof(pTTT->szText)/sizeof(WCHAR) )
		== (size_t)-1 ) wcscpy( pTTT->szText, L"마이티 네트워크" );
#endif
	return TRUE;
}

BOOL CInfoBar::OnToolTipNeedTextA( UINT /*id*/, NMHDR * pNMHDR, LRESULT * /*pResult*/ )
{
	TOOLTIPTEXTA *pTTT = (TOOLTIPTEXTA *)pNMHDR;
#ifdef _UNICODE
	if ( _wcstombsz( pTTT->szText, GetGoalString(), sizeof(pTTT->szText)/sizeof(char) )
		== (size_t)-1 ) strcpy( pTTT->szText, "마이티 네트워크" );
#else
	strcpy( pTTT->szText, GetGoalString() );
#endif
	return TRUE;
}

// 현재 기루다, 목표점수, 프랜드 텍스트를 가지고 공약 스트링을 리턴한다
CString CInfoBar::GetGoalString() const
{
	if ( m_nMinScore ) {
		CString sGoal = CCard::GetGoalString( Mo()->bUseTerm, m_nKiruda,
			m_nMinScore );
		sGoal += _T(" ") + m_sFriend;
		return sGoal;
	}
	else return CString(_T("아직 공약이 정해지지 않았습니다"));
}

// 크기 변경 되었을때
void CInfoBar::OnSize( UINT nType, int cx, int cy )
{
	// 채팅창 크기를 새롭게 갱신한다
	if ( m_bChat && m_cChat.GetSafeHwnd() ) {
		CRect rcItem;
		GetItemRect( 3, &rcItem );
		m_cChat.MoveWindow( &rcItem, TRUE );
	}
	CStatusBar::OnSize( nType, cx, cy );
}

BOOL CInfoBar::PreTranslateMessage( MSG *pMsg )
{
	if ( m_bChat && pMsg->hwnd == m_cChat.GetSafeHwnd()
		&& pMsg->message == WM_KEYDOWN
		&& ( pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN ) ) {

		if ( pMsg->wParam == VK_UP )
			m_cChat.SetWindowText( m_pChatBuffer->Up() );
		else m_cChat.SetWindowText( m_pChatBuffer->Down() );

		m_cChat.SetSel( 0, -1 );

		return TRUE;
	}
	else return CStatusBar::PreTranslateMessage( pMsg );
}

// 채팅창에서 오는 커맨드 메시지 처리
afx_msg BOOL CInfoBar::OnCommand( WPARAM wParam, LPARAM lParam )
{
	if ( wParam == IDOK ) {
		// 사용자가 엔터를 입력했다
		// 현재 입력된 메시지를 뷰로 송신한다

		// 메시지를 만든다
		CString s;
		m_cChat.GetWindowText( s );

		// 메시지에 아무 내용도 없는 경우
		s.TrimLeft(); s.TrimRight();
		if ( s.GetLength() == 0 ) return TRUE;

		// 핸들러를 호출
		if ( m_handler.pfnProc )
			m_handler.pfnProc( s, m_handler.dwUser );

		// 버퍼에 저장
		m_pChatBuffer->Push( s );

		m_cChat.SetWindowText( _T("") );
		return TRUE;
	}
	else return CStatusBar::OnCommand( wParam, lParam );
}

LRESULT CInfoBar::OnSetText( WPARAM, LPARAM lParam )
{
	LPCTSTR lpsz = (LPCTSTR)lParam;
	SetText( lpsz );
	return TRUE;
}
