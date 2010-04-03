// DSB.cpp: implementation of the DSB class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"
#include "DSB.h"

#include "BoardWrap.h"
#include "BmpMan.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

COLORREF DSB::s_colWhite = RGB(255,255,255);
COLORREF DSB::s_colCyan = RGB(0,255,255);
COLORREF DSB::s_colYellow = RGB(255,255,0);
COLORREF DSB::s_colGray = RGB(192,192,192);
COLORREF DSB::s_colLightGray = RGB(192,192,192);
bool DSB::s_bColorLoaded = false;
const int DSB::s_tdNormal = 0;
const int DSB::s_tdShade = (TD_SHADE|TD_SUNKEN);
const int DSB::s_tdOutline = TD_OUTLINE;
const int DSB::s_tdShadeOutline = (TD_SHADE|TD_OUTLINE);
const int DSB::s_tdShadeOpaque = (TD_SHADE|TD_OPAQUE);
const int DSB::s_tdOutlineOpaque = (TD_OUTLINE|TD_OPAQUE);
const int DSB::s_tdShadeOutlineOpaque = (TD_SHADE|TD_OUTLINE|TD_OPAQUE);
const int DSB::s_tdMidium = TD_SIZEMIDIUM;
const int DSB::s_tdMidiumShade = (TD_SIZEMIDIUM|TD_SHADE|TD_SUNKEN);
const int DSB::s_tdMidiumOutline = (TD_SIZEMIDIUM|TD_OUTLINE);
const int DSB::s_tdMidiumShadeOutline = (TD_SIZEMIDIUM|TD_SHADE|TD_OUTLINE);

CList<int,int> DSB::s_lID;
CList<DSB*,DSB*> DSB::s_lPtr;
CCriticalSection DSB::s_csDSB;

DSB::GarbageCollector DSB::s_gc;

// DSB 의 기본 색성을 세트한다
void DSB::SetDefaultColor(
	COLORREF colText, COLORREF colStrong1,
	COLORREF colStrong2, COLORREF colGray )
{
	s_colWhite = colText;
	s_colCyan = colStrong1;
	s_colYellow = colStrong2;
	s_colGray = colGray;
	s_colLightGray =
		RGB( ( GetRValue(colText) + GetRValue(colGray) ) / 2,
			( GetGValue(colText) + GetGValue(colGray) ) / 2,
			( GetBValue(colText) + GetBValue(colGray) ) / 2 );
}


DSB::DSB( CBoardWrap* pBoard ) : m_pBoard(pBoard)
{
	m_rcDSB.SetRectEmpty();
	m_pBmpMan = 0;
	m_nTimeOut = -1;
	m_pntOffset.x = m_pntOffset.y = 0;
	m_posSel = 0;
	m_bFixed = false;
	m_bModal = false;
	m_bBelow = false;

	// 새로운 ID 를 할당한다 ( s_lID 에 있는 최대값 + 1 )
	s_csDSB.Lock();
	int nMaxID = -1;
	POSITION pos = s_lID.GetHeadPosition();
	while (pos) {
		int nID = s_lID.GetNext(pos);
		nMaxID = max(nID,nMaxID);
	}
	m_nID = nMaxID+1;
	m_pEvent = 0;
	m_pResult = 0;
	s_lID.AddTail(m_nID);
	s_lPtr.AddTail( this );	// 포인터 리스트에도 추가
	s_csDSB.Unlock();
}

DSB::~DSB()
{
	// 그 ID 를 s_lID 에서 삭제한다
	s_csDSB.Lock();
	POSITION pos = s_lID.GetHeadPosition();
	for (; pos; s_lID.GetNext(pos) )
		if ( s_lID.GetAt(pos) == m_nID ) {
			s_lID.RemoveAt(pos);
			break;
		}
	ASSERT( pos );
	// 포인터 리스트에서도 삭제
	pos = s_lPtr.Find( this ); ASSERT(pos);
	s_lPtr.RemoveAt(pos);
	s_csDSB.Unlock();

	if ( m_pEvent ) m_pEvent->SetEvent();
}

// 새로운 상자를 생성한다
// CBoard 에 자신을 등록하고 새로 그린다
// 위치 xDSB, yDSB 는 좌상단 모서리가 아니라
// 스크린 중심에서 상자 중심이 어느정도 벗어났는가
// 하는 값이다 !!!
void DSB::Create(
	int xDSB, int yDSB,				// 위치
	int cxDSB, int cyDSB,			// 크기
	int nTimeOut )					// 수명 (밀리초:-1은 무한)
{
	if ( !s_bColorLoaded ) {
		s_bColorLoaded = true;
		DSB::SetDefaultColor( (COLORREF)Mo()->nDSBText, (COLORREF)Mo()->nDSBStrong1,
			(COLORREF)Mo()->nDSBStrong2, (COLORREF)Mo()->nDSBGray );
	}
	ASSERT( m_pBoard );

	if ( (HWND)*m_pBoard == 0 ) delete this;
	else {
		m_nTimeOut = nTimeOut;
		m_rcDSB.SetRect( xDSB, yDSB, cxDSB, cyDSB );
		m_pBoard->AddDSB( this );
	}
}

// 새로운 상자를 생성한다
// CBoard 에 자신을 등록하고 새로 그린다
// x, y 는 DP 좌표
void DSB::Create2(
	int x, int y,					// 위치
	int cxDSB, int cyDSB,			// DSB 단위로 계산한 크기
	int nTimeOut )					// 수명 (밀리초:-1은 무한)
{
	// 과연 x, y 에 DSB 를 표시하려면 중심에서
	// 어느 정도를 이동해야 할까

	int nMyUnit = GetDSBUnit();
	int nBoardUnit = m_pBoard->GetDSBUnit();

	// 실제 크기
	int cx = cxDSB*nMyUnit, cy = cyDSB*nMyUnit;
	// CBoard 크기
	CRect rc; m_pBoard->GetClientRect(&rc);
	// 화면을 벗어나게 되면 위치를 보정한다
	x = max( rc.left, min( rc.right - cx, x ) );
	y = max( rc.top, min( rc.bottom - cy, y ) );
	// 그냥 이동 안했을 때 표시되는 위치
	int xBefore = rc.left + rc.Width()/2 - cx/2,
		yBefore = rc.top + rc.Height()/2 - cy/2;
	// 이동해야 하는 양
	int xToMove = x - xBefore, yToMove = y - yBefore;
	// 이동량을 DSB 단위로
	int xToMoveDSB = (xToMove+nBoardUnit-1) / nBoardUnit;
	int yToMoveDSB = (yToMove+nBoardUnit-1) / nBoardUnit;
	// 차이는 offset 으로 보정한다
	m_pntOffset.x = xToMove - xToMoveDSB*nBoardUnit;
	m_pntOffset.y = yToMove - yToMoveDSB*nBoardUnit;

	DSB::Create( xToMoveDSB, yToMoveDSB, cxDSB, cyDSB, nTimeOut );
}

// 상자를 삭제하고 자신을 delete 한다
// CBoard 에서 등록을 지우고 새로 그린다
void DSB::Destroy()
{
	OnDestroy();
	ASSERT( m_pBoard );
	VERIFY( m_pBoard->RemoveDSB( this ) );
	delete this;
}

// 이 상자는 뷰의 크기가 변해도
// 크기가 변하지 않도록 만든다
// ( Create 전에 이 함수를 호출해야 한다 )
void DSB::SetFixed()
{
	m_bFixed = true;
}

// 마우스로 이 사각형의 이 점(CBoard기준 절대좌표)을
// 클릭하거나 지나갈 때 적절한 효과를 준다
// 클릭(bCheckOnly==true)했다면 핫 스팟 이벤트를
// 디스패치 하며, 마우스만 지나간다면(bCheckOnly==false)
// 그 핫 스팟을 하이라이트 한다
// 만일 어떤 핫 스팟에 걸려 있다면 참을,
// 그렇지 않다면 거짓을 리턴한다
bool DSB::DispatchHotspot( int x, int y, bool bCheckOnly )
{
	// 이 DSB 의 사각형
	CRect rcBox; GetRect( &rcBox );

	// 오른쪽 위의 x 버튼은 특별하게 처리한다
	if ( m_nTimeOut < 0 ) {
		// timeout 이 무한대일 때만 x 버튼이 나타난다
		CRect rcX( CPoint( rcBox.left + rcBox.Width() - 17, rcBox.top + 5 ),
					CSize( 14, 14 ) );
		if ( rcX.PtInRect( CPoint(x,y) ) ) {
			if ( !bCheckOnly ) {
				PlaySound( IDW_BEEP );
				OnClick( 0 );	// 종료 코드
			}
			return true;
		}
	}

	// 다른 핫 스팟을 찾는다
	POSITION pos = m_lHotspot.GetHeadPosition();
	for (; pos; m_lHotspot.GetNext(pos) ) {

		// 10000, 10000 기준 사각형 좌표
		CRect rc10000 = m_lHotspot.GetAt(pos).rc;
		// 이것을 DP 좌표로 바꾼다
		CRect rc(	rcBox.left + rcBox.Width() * rc10000.left/10000,
					rcBox.top + rcBox.Height() * rc10000.top/10000,
					rcBox.left + rcBox.Width() * rc10000.right/10000,
					rcBox.top + rcBox.Height() * rc10000.bottom/10000 );

		if ( rc.PtInRect( CPoint(x,y) )
				&& *m_lHotspot.GetAt(pos).pCol != s_colGray ) {
			// 그 영역에 있고 disabled 되지 않은 핫스팟

			if ( !bCheckOnly ) {
				OnClickSound();
				OnClick( m_lHotspot.GetAt(pos).pVoid );
			}
			else {
				// 기존의 하일라이트 된 핫 스팟을 노멀로 바꾸고
				// 이 핫 스팟을 하일라이트 한다
				POSITION posLastSel = m_posSel;
				m_posSel = pos;
				if ( posLastSel != m_posSel ) {
					if ( posLastSel )
						OnHighlight( m_lHotspot.GetAt( posLastSel ), false );
					if ( m_posSel )
						OnHighlight( m_lHotspot.GetAt( m_posSel ), true );
					UpdateHotspot( posLastSel, m_posSel );
				}
			}
			return true;
		}
	}
	// 하일라이트 되었던 핫 스팟을 노멀로 바꾼다
	POSITION posLastSel = m_posSel;
	m_posSel = 0;
	if ( posLastSel ) {
		OnHighlight( m_lHotspot.GetAt( posLastSel ), false );
		UpdateHotspot( posLastSel );
	}
	return false;
}

// 타이머를 세트 ( SetPermanent 와 반대 함수 )
void DSB::SetTimeout( int nTimeOut )
{
	m_nTimeOut = nTimeOut;
	m_pBoard->SetDSBTimer( this );

	CRect rc; GetRect( &rc );
	m_pBoard->UpdateDSB( &rc, true );
}

// 핫 스팟을 등록한다
// 이 영역을 클릭하면 CBoard 는 지정된 값을 가지고
// 이 클래스의 OnClick 을 호출한다
// 위치는 상대적인 (Box의 좌상단이 0,0)
// DSB 좌표
RECT DSB::RegisterHotspot(
	int x, int y,					// 위치
	int cx, int cy,					// 크기 (-1,-1 이면 자동 계산)
	bool bDSB,						// 위치 좌표가 DSB 좌표인가
	UINT idBitmap, LPCTSTR s,		// 그릴 비트맵과 쓸 문자열 (없으면 각각 -1, 0 )
	const COLORREF* pCol,
	const int* pDeco,				// 문자열 색상과 장식 & 크기
	const COLORREF* pColSel,
	const int* pDecoSel,			// 마우스가 지나갈 때의 색상과 장식 & 크기
	LPVOID pVoid )					// OnClick 에 넘겨지는 값
{
	HOTSPOT hs;

	// 이 DSB 의 사각형
	CRect rcDSB;
	GetRect( &rcDSB );

	if ( bDSB ) {
		CPoint pnt = DSBtoDP( CPoint(x,y) );
		x = pnt.x; y = pnt.y;
	}

	// 핫 스팟 영역을 계산한다
	CRect rcSpot;
	if ( cx == -1 || cy == -1 ) {
		CClientDC dc(*m_pBoard);
		rcSpot = idBitmap ?
				PutBitmap( &dc, idBitmap, s, 0, 0, false, *pCol, *pDeco, true )
			:	PutText( &dc, s, 0, 0, false, *pCol, *pDeco, true );
	}
	else {
		int nUnit = bDSB ? GetDSBUnit() : 1;
		rcSpot = CRect( CPoint(x,y), CSize(cx*nUnit,cy*nUnit) );
	}

	// 사각형을 상대적인 10000, 10000 기준 좌표로 변환한다
	hs.rc.left = ( x - rcDSB.left ) * 10000 / rcDSB.Width();
	hs.rc.top = ( y - rcDSB.top ) * 10000 / rcDSB.Height();
	hs.rc.right = hs.rc.left + rcSpot.Width() * 10000 / rcDSB.Width();
	hs.rc.bottom = hs.rc.top + rcSpot.Height() * 10000 / rcDSB.Height();

	hs.s = s;
	hs.idBitmap = idBitmap;
	hs.pCol = pCol;
	hs.pDeco = pDeco;
	hs.pColSel = pColSel;
	hs.pDecoSel = pDecoSel;
	hs.pVoid = pVoid;

	m_lHotspot.AddTail(hs);

	return rcSpot + CPoint(x,y);
}

// 주어진 pVoid 값을 가지는 핫 스팟을 찾는다
POSITION DSB::FindHotspot( LPVOID pVoid )
{
	POSITION pos = m_lHotspot.GetHeadPosition();
	for (; pos; m_lHotspot.GetNext(pos) )
		if ( m_lHotspot.GetAt(pos).pVoid == pVoid )
			return pos;
	return 0;
}

// 핫 스팟을 Update 하고 Invalidate 한다
void DSB::UpdateHotspot( POSITION pos1, POSITION pos2 )
{
	CRect rcBox; GetRect( &rcBox );
	CRect rc( 0, 0, 0, 0 );

	if ( pos1 ) {
		// 10000, 10000 기준 사각형 좌표
		CRect rc10000 = m_lHotspot.GetAt(pos1).rc;
		// 이것을 DP 좌표로 바꾼다
		rc |= CRect(rcBox.left + rcBox.Width() * rc10000.left/10000,
					rcBox.top + rcBox.Height() * rc10000.top/10000,
					rcBox.left + rcBox.Width() * rc10000.right/10000 + 3,
					rcBox.top + rcBox.Height() * rc10000.bottom/10000 + 3 );
	}
	if ( pos2 ) {
		// 10000, 10000 기준 사각형 좌표
		CRect rc10000 = m_lHotspot.GetAt(pos2).rc;
		// 이것을 DP 좌표로 바꾼다
		rc |= CRect(rcBox.left + rcBox.Width() * rc10000.left/10000,
					rcBox.top + rcBox.Height() * rc10000.top/10000,
					rcBox.left + rcBox.Width() * rc10000.right/10000 + 3,
					rcBox.top + rcBox.Height() * rc10000.bottom/10000 + 3 );
	}

	if ( !rc.IsRectEmpty() ) m_pBoard->UpdateDSB(&rc);
}

// 이 상자의 실제 DP 사각형 좌표를 계산한다
void DSB::GetRect( LPRECT prc )
{
	CRect rcClient; m_pBoard->GetClientRect( &rcClient );

	// 위치는 CBoard 의 단위를 써야 하고
	// 크기는 (고정크기DSB일지도 모르므로) 자신의 단위를 써야 한다
	int nMyUnit = GetDSBUnit();
	int nBoardUnit = m_pBoard->GetDSBUnit();

	prc->left = -4 + rcClient.left + rcClient.Width()/2
		+ m_rcDSB.left*nBoardUnit - m_rcDSB.right*nMyUnit/2 + m_pntOffset.x;
	prc->top = -4 + rcClient.top + rcClient.Height()/2
		+ m_rcDSB.top*nBoardUnit - m_rcDSB.bottom*nMyUnit/2 + m_pntOffset.y;
	prc->right = 4 + prc->left + m_rcDSB.right*nMyUnit;
	prc->bottom = 4 + prc->top + m_rcDSB.bottom*nMyUnit;
}

// 상대적인 DSB 좌표를 윈도우 디바이스 좌표로 변환한다
POINT DSB::DSBtoDP( POINT p ) const
{
	int nMyUnit = GetDSBUnit();
	int nBoardUnit = m_pBoard->GetDSBUnit();
	CRect rcClient; m_pBoard->GetClientRect(&rcClient);

	POINT r;

	r.x = rcClient.left + rcClient.Width()/2
		+ m_rcDSB.left*nBoardUnit - m_rcDSB.right*nMyUnit/2
		+ p.x*nMyUnit + m_pntOffset.x;
	r.y = rcClient.top + rcClient.Height()/2
		+ m_rcDSB.top*nBoardUnit - m_rcDSB.bottom*nMyUnit/2
		+ p.y*nMyUnit + m_pntOffset.y;

	return r;
}

RECT DSB::DSBtoDP( const RECT& rc ) const
{
	CPoint p1( rc.left, rc.top );
	CPoint p2( rc.right, rc.bottom );
	return CRect( DSBtoDP(p1), DSBtoDP(p2) );
}

// 좌표를 이동시킨다
void DSB::Offset( int x, int y )
{
	// 원래 이 함수는 다음과 같은 효과를 내는 함수다
	m_pntOffset += CPoint(x,y);

	// 그러나 offset 값이 커지게 되면 DSB 기준 좌표와
	// 어긋나게 되므로 offset 값의 여분( DSBUnit 을 넘는 양 )
	// 은 현재 m_rcDSB 에 반영해 주어야 한다

	int xOverflowDSBs = m_pntOffset.x / m_pBoard->GetDSBUnit();
	int yOverflowDSBs = m_pntOffset.y / m_pBoard->GetDSBUnit();

	m_rcDSB.left += xOverflowDSBs;
	m_rcDSB.top += yOverflowDSBs;

	m_pntOffset.x -= xOverflowDSBs * m_pBoard->GetDSBUnit();
	m_pntOffset.y -= yOverflowDSBs * m_pBoard->GetDSBUnit();

	OnMove();
}

// 뷰의 폰트를 얻는다
CFont* DSB::GetFont( int nSize )
{
	return m_pBoard->GetFont( m_bFixed ? -nSize-1 : nSize );
}

int DSB::GetDSBUnit() const
{
	// 고정 크기는 14 !! -> 가장 적당했음
	return m_bFixed ? 14 : m_pBoard->GetDSBUnit();
}

// 이 스트링의 화면에서의 크기를 구한다 (지정한 폰트 사용)
CSize DSB::GetTextExtent( int nSize, LPCTSTR s )
{
	CClientDC dc(*m_pBoard);
	dc.SelectObject( GetFont(nSize) );
	return dc.GetTextExtent( s, _tcslen(s) );
}

// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
// 오버라이드 하여 구현해야 한다
// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
void DSB::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	CRect rc; GetRect(&rc);
	int x = rc.left, y = rc.top, cx = rc.Width(), cy = rc.Height();

	// 배경 칠하기
	FillBack( pDC, &rc, m_pBmpMan, Mo()->nDSBShadeMode, bBrushOrigin );

	// 오른쪽 위의 X 버튼
	if ( m_nTimeOut < 0 ) {
		pDC->SelectStockObject( WHITE_PEN );
		pDC->MoveTo( x+cx-5, y+15 );
		pDC->LineTo( x+cx-15-1, y+5-1 );
		pDC->MoveTo( x+cx-15, y+15 );
		pDC->LineTo( x+cx-5+1, y+5-1 );
		pDC->MoveTo( x+cx-5, y+15+1 );
		pDC->LineTo( x+cx-15-1, y+5+1-1 );
		pDC->MoveTo( x+cx-15, y+15+1 );
		pDC->LineTo( x+cx-5+1, y+5+1-1 );
	}

	// 볼륨을 주는 테두리
	CPen pnGray( PS_SOLID, 1, RGB(128,128,128) );
	pDC->SelectObject( &pnGray );
	pDC->MoveTo( x+cx-1, y );
	pDC->LineTo( x, y );
	pDC->LineTo( x, y+cy-1 );

	CPen pnDGray( PS_SOLID, 1, RGB(64,64,64) );
	pDC->SelectObject( &pnDGray );
	pDC->MoveTo( x, y+cy-1 );
	pDC->LineTo( x+cx-1, y+cy-1 );
	pDC->LineTo( x+cx-1, y );

	pDC->SelectStockObject( WHITE_PEN );
	pDC->MoveTo( x+cx-2, y+1 );
	pDC->LineTo( x+1, y+1 );
	pDC->LineTo( x+1, y+cy-2 );

	pDC->SelectStockObject( BLACK_PEN );
	pDC->MoveTo( x+1, y+cy-2 );
	pDC->LineTo( x+cx-2, y+cy-2 );
	pDC->LineTo( x+cx-2, y+1 );

	// 핫 스팟들을 그린다
	POSITION pos = m_lHotspot.GetHeadPosition();
	for (; pos; m_lHotspot.GetNext(pos) ) {
		HOTSPOT& hs = m_lHotspot.GetAt(pos);

		// 10000, 10000 좌표에서 DP 좌표로 변환
		int xDraw = x + cx * hs.rc.left/10000;
		int yDraw = y + cy * hs.rc.top/10000;

		// 그린다
		if ( !hs.idBitmap && !hs.s ) ;
		else if ( !hs.idBitmap )
			PutText( pDC, hs.s, xDraw, yDraw, false,
				pos == m_posSel ? *hs.pColSel : *hs.pCol,
				pos == m_posSel ? *hs.pDecoSel : *hs.pDeco );
		else
			PutBitmap( pDC, hs.idBitmap, hs.s,
				xDraw, yDraw, false,
				pos == m_posSel ? *hs.pColSel : *hs.pCol,
				pos == m_posSel ? *hs.pDecoSel : *hs.pDeco );
	}
}

// 글자를 쓴다
RECT DSB::PutText(
	CDC* pDC,
	LPCTSTR s,				// 쓸 문자
	int x, int y,			// 위치
	bool bDSB,				// x, y 가 DSB 좌표인가
	COLORREF col,			// 색상
	int deco,				// 문자 장식 & 크기
	bool bCheckOnly )
{
	bool bCenter = bDSB && x < 0;
	bool bMiddle = bDSB && y < 0;

	int nUnit = GetDSBUnit();

	// 어떤 글씨든 DSB 자체에 클립된다
	CRect rc; GetRect(&rc);

	// DSB 좌표를 실 좌표로 변환한다
	if ( bDSB ) {
		CPoint pnt = DSBtoDP( CPoint(x,y) );
		if ( bCenter ) x = rc.left + rc.Width()/2 - 15/2;
		else x = pnt.x;
		if ( bMiddle ) y = rc.top + rc.Height()/2 - nUnit/2;
		else y = pnt.y;
	}

	CFont* pFont = GetFont( (int) (deco&3) );
	pDC->SelectObject( pFont );

	// 리턴될 사각형 (범위)
	CRect ret( CPoint(x,y), pDC->GetTextExtent( s ) );
	if ( bCenter ) ret.OffsetRect( -ret.Width()/2, 0 );

	// 클립 영역을 벗어나면 안그린다
	{
		CRect rg(ret);
		rg.InflateRect( 3, 3, 3, 3 );
		CRect rcClip;
		pDC->GetClipBox( &rcClip );
		if ( ( rg & rcClip ).IsRectEmpty() )
			return ret;
	}

	if ( !bCheckOnly ) {	// 실제로 그려야 한다

		pDC->SetTextAlign(
			( bCenter ? TA_CENTER : TA_LEFT ) | TA_TOP );
		pDC->SetBkMode( TRANSPARENT );

		if ( ( deco & TD_OPAQUE ) || Mo()->bDSBOpaque ) {	// 불투명
			CRect rcFill = ret & rc;
			pDC->FillSolidRect( &rcFill, RGB(0,0,0) );
		}

		// 그림자 효과
		if ( deco & TD_SHADE ) {
			pDC->SetTextColor( RGB(0,0,0) );
			pDC->ExtTextOut( x+1, y+1, ETO_CLIPPED, &rc, s, NULL );
			if ( deco & TD_OUTLINE )
				pDC->ExtTextOut( x+2, y+2, ETO_CLIPPED,
						&rc, s, NULL ); // 좀더 두꺼운 그림자
		}
		// 테두리 효과
		if ( deco & TD_OUTLINE ) {
			pDC->SetTextColor( RGB(
				GetRValue(col)*3/5, GetGValue(col)*3/5, GetBValue(col)*3/5 ) );
			pDC->ExtTextOut( x+1, y, ETO_CLIPPED, &rc, s, NULL );
			pDC->ExtTextOut( x-1, y, ETO_CLIPPED, &rc, s, NULL );
			pDC->ExtTextOut( x, y+1, ETO_CLIPPED, &rc, s, NULL );
			pDC->ExtTextOut( x, y-1, ETO_CLIPPED, &rc, s, NULL );
		}

		// 글씨 쓰기
		pDC->SetTextColor( col );
		pDC->ExtTextOut( x, y, ETO_CLIPPED, &rc, s, NULL );
	}

	return ret;
}

// 비트맵을 그린다
RECT DSB::PutBitmap(
	CDC* pDC,
	UINT id,				// 그릴 비트맵
	LPCTSTR s,				// 비트맵 우측에 쓰일 글자
	int x, int y,			// 위치
	bool bDSB,				// x, y 가 DSB 좌표인가
	COLORREF col,			// 색상
	int deco,				// 문자 장식 & 크기
	bool bCheckOnly )
{
	ASSERT( id );

	bool bCenter = bDSB && x < 0;
	bool bMiddle = bDSB && y < 0;

	CRect rc; GetRect(&rc);

	int nUnit = GetDSBUnit();
	CSize szRealBitmap = GetBitmapSize(id);
	CSize szBitmap( szRealBitmap.cx + 3, szRealBitmap.cy + 3 );

	CFont* pFont = GetFont( (int) (deco&3) );
	pDC->SelectObject( pFont );

	CSize szText;
	if ( s ) szText = pDC->GetTextExtent( s, _tcslen(s) );
	else szText.cx = szText.cy = 0;

	// 리턴될 사각형 ( 일단 크기만 결정한다 )
	CRect rcRet( 0, 0, szBitmap.cx + szText.cx,
		max( szText.cy, szBitmap.cy ) );

	// DSB 좌표일 경우를 대비해서 x, y 의 DSB 좌표를 미리 계산해 둔다
	CPoint pnt = DSBtoDP( CPoint(x,y) );

	// 사각형을 적절한 위치로 옮긴다
	if ( bCenter ) rcRet.OffsetRect( rc.left + rc.Width()/2-rcRet.Width()/2 - 15/2, 0 );
	else if ( bDSB ) rcRet.OffsetRect( pnt.x, 0 );
	else rcRet.OffsetRect( x, 0 );
	if ( bMiddle ) rcRet.OffsetRect( 0, rc.top + rc.Height()/2 - (rcRet.Height()+1)/2 );
	else if ( bDSB ) rcRet.OffsetRect( 0, pnt.y );
	else rcRet.OffsetRect( 0, y );

	// 텍스트를 그린다 ( 리턴될 사각형을 얻는다
	if ( s && !bCheckOnly ) {
		PutText( pDC, s,
			rcRet.left+szBitmap.cx, rcRet.top + (szBitmap.cy-nUnit)/2,
			false, col, deco, false );
	}

	if ( !bCheckOnly ) {		// 실제로 그려야 한다

		CRect rcBitmap( CPoint( rcRet.left+2, rcRet.top+2 ),
						GetBitmapSize(id) );
		if ( ( rcBitmap & rc ) == rcBitmap ) {

			DrawBitmap( pDC, id, rcRet.left+2, rcRet.top+2 );

			if ( deco & TD_SUNKEN ) {
				pDC->SelectStockObject( BLACK_PEN );
				pDC->MoveTo( rcRet.left+szBitmap.cx, rcRet.top );
				pDC->LineTo( rcRet.left, rcRet.top );
				pDC->LineTo( rcRet.left, rcRet.top+szBitmap.cy);
				pDC->SelectStockObject( WHITE_PEN );
				pDC->LineTo( rcRet.left+szBitmap.cx, rcRet.top+szBitmap.cy );
				pDC->LineTo( rcRet.left+szBitmap.cx, rcRet.top );
			}
			if ( deco & TD_OUTLINE ) {
				CPen pen( PS_SOLID, 1, col );
				CPen* penOld = pDC->SelectObject( &pen );
				pDC->SelectStockObject( NULL_BRUSH );
				pDC->Rectangle(
					rcRet.left, rcRet.top,
					rcRet.right+1, rcRet.bottom+1 );
				pDC->SelectObject( penOld );
			}
		}
	}

	return rcRet;
}

// ParseString 에서 사용하는 내부 함수
// TCHAR 포인터가 가리키는 글자의 다음 글자를 리턴한다
// 종료, 또는 에러시 0 리턴
static const TCHAR* next_tchar( const TCHAR* p )
{
	if ( !p || !*p ) return 0;

#ifdef _UNICODE
	return p+1;
#else
	// TCHAR is char

	int nType;
	size_t nOffset = 0;

	nType = _mbsbtype( (const unsigned char*)p, nOffset );
	if ( nType == _MBC_SINGLE ) return p+1;
	else if ( nType != _MBC_LEAD ) return 0;
	else do {
		nType = _mbsbtype( (const unsigned char*)p, ++nOffset );
		if ( nType == _MBC_ILLEGAL ) return 0;
	} while ( nType == _MBC_TRAIL );

	return p+nOffset;
#endif
}

// 긴 스트링을 몇 줄로 자른다
// cLine     : 각 줄의 대략적인 길이 (DSB 단위)
// asLine    : 0 이 아니면 여기에 각 부분 스트링을 복사한다
// s         : 자를 긴 스트링
// nMaxWidth : 실제로 필요한 각 라인의 최대 버퍼 크기
// 리턴값 -1 은 에러, 그밖에는, 실제로 필요한 라인 수
int DSB::ParseString( LPTSTR* asLine, int cLine,
					  LPCTSTR s, size_t& nMaxWidth )
{
	CClientDC dc(*m_pBoard);
	dc.SelectObject( GetFont(0) );

	nMaxWidth = 0;
	int nMaxLineSize = cLine * m_pBoard->GetDSBUnit();

	int nCurLine = 0;	// 현재 라인

	const TCHAR* pHead = s;	// 현재 라인의 시작 포인터
	const TCHAR* pPrev = s;	// 이전 포인터
	const TCHAR* pCur;	// 현재 포인터

	do {
		for(;;) {
			pCur = next_tchar(pPrev);

			if ( !pCur || !*pCur ) break;
			if ( dc.GetTextExtent( pHead, int(pCur-pHead) ).cx
				> nMaxLineSize ) break;

			pPrev = pCur;
		}

		if ( pCur && *pCur ) {
			// 아직 안끝났음
			size_t chars = size_t( pCur - pHead );
			size_t len = chars * sizeof(TCHAR);
			if ( nMaxWidth < len+1 ) nMaxWidth = chars+1;
			if ( asLine ) {
				memcpy( asLine[nCurLine], pHead, len );
				asLine[nCurLine][chars] = _T('\0');
			}
			nCurLine++;
			pPrev = pHead = pCur;
		}
		else {
			size_t chars = _tcslen( pHead );
			if ( nMaxWidth < chars+1 ) nMaxWidth = chars+1;
			if ( asLine ) {
				strcpy( asLine[nCurLine], pHead );
			}
			nCurLine++;
		}
	} while ( pCur && *pCur );

	return nCurLine;
}
