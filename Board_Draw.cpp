// Board_Layout.cpp : CBoard 중 레이아웃에 관한 함수들 정의
//

#include "stdafx.h"
#include "Mighty.h"

#include "MFSM.h"
#include "BmpMan.h"
#include "DSB.h"
#include "Board.h"
#include "BoardWrap.h"
#include "Play.h"
#include "Player.h"

#include "InfoBar.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// 플레이어의 손에 든 카드를 그린다
void CBoard::DrawHand(
	CDC* pDC,
	int nPlayer, int nPlayers,			// 플레이어/플레이어수
	const CCardList* pList,				// 플레이어의 카드
	int x, int y, int cx, int cy,		// 범위
	int xOff, int yOff,					// 실제로 그려질때의 옵셋 변화
	bool bReveal,						// 앞면을 보일것인가
	int nMouseOver,						// 마우스가 위치하는 카드 인덱스
	bool* abSelected,					// 현재 선택된 카드들
	bool bScore )						// 손에 든 카드가 아니라 점수 카드를 그린다
{
	// 디버그 모드에서는 모든 카드를 보인다
#ifdef _DEBUG
	bReveal = true;
#endif

	if ( !m_pMFSM ) return;

	// 범위 사각형 ( 이 범위를 가지고 그릴 카드를 최적화 한다 )
	CRect rcClip( x, y, x+cx, y+cy );

	// 카드 크기
	int nCardWidth = m_szCard.cx;
	int nCardHeight = m_szCard.cy;

	// 전체 카드 수
	int nCards = pList->GetCount();

	// 루프 시작

	POSITION pos = pList->GetHeadPosition();
	for ( int index = 0; pos; index++ ) {
		// 현재 카드
		CCard c = pList->GetNext(pos);
		if ( !bReveal ) c = CCard(0/*back*/);

		// 현재 카드의 사각형
		CRect rcCard = CalcRect(
			nPlayers, (bScore?CR_SCORE:CR_HAND), nPlayer, nCardWidth, nCardHeight,
			index, nCards );

		BOOL bOverlapped = !( rcClip & rcCard ).IsRectEmpty();

		// 아래쪽 플레이어(사람)에 대해서는 nMouseOver 와 abSelected
		// 인자를 사용하여 특수한 처리를 한다
		if ( !bScore ) {
			// 위로 들려진 카드에 대한 처리
			if ( abSelected && abSelected[index] ) // 많이 들림
				rcCard.OffsetRect( 0,
					-nCardHeight/CARD_SHIFT_RATIO_OF_SELECTION );
			else if ( index == nMouseOver ) // 살짝 들림
				rcCard.OffsetRect( 0,
					-nCardHeight/CARD_SHIFT_RATIO_OF_MOUSE_OVER );
		}

		if ( !bOverlapped
			&& ( rcClip & rcCard ).IsRectEmpty() ) continue;

		// 카드 사각형에 카드를 그린다
		m_bm.DrawCard( pDC, c, rcCard + CPoint(xOff,yOff) );
	}
}

// 가운데 쌓인 카드를 그린다
void CBoard::DrawCenter(
	CDC* pDC,
	int nPlayers,						// 플레이어수
	const CCardList* pList,				// 그릴 카드
	int nBeginer,						// 시작 플레이어(아래0부터 시계방향)
	int x, int y, int cx, int cy,		// 그릴 영역
	int xOff, int yOff )				// 실제로 그려질 때의 옵셋 변화
{
	if ( !m_pMFSM ) return;

	// 클립핑 영역
	CRect rcClip( x, y, x+cx, y+cy );

	// 카드 크기
	int nCardWidth = m_szCard.cx;
	int nCardHeight = m_szCard.cy;

	// 루프 내에서 이전 카드가 그려졌었는가
	// (그려졌었다면 이 카드도 반드시 그려야한다)
	bool bPrevDrawn = false;

	// 루프 시작
	POSITION pos = pList->GetHeadPosition();
	for ( int i = 0; pos; i++ ) {
		CCard c = pList->GetNext(pos);

		int nPlayer = ( nBeginer + i + nPlayers ) % nPlayers;

		// 그릴 사각형과 실제로 그려지는 사각형
		CRect rcCard = CalcRect( nPlayers, CR_CENTER, 0, nCardWidth, nCardHeight,
			nPlayer, 0, 0 );

		if ( bPrevDrawn || !( rcCard & rcClip ).IsRectEmpty() ) {

			bPrevDrawn = true;

			// 카드 사각형에 카드를 그린다
			m_bm.DrawCard( pDC, c, rcCard + CPoint(xOff,yOff) );
		}
	}
}

// 이름을 그린다
void CBoard::DrawName(
	CDC* pDC,
	int nPlayers,						// 플레이어수
	LPCTSTR sName,						// 이름
	int nPlayer,						// 그릴 플레이어
	COLORREF col, COLORREF colShadow,	// 글자 색상, 그림자 색상
	CFont* pFont )						// 폰트 (디폴트는 시스템 폰트)
{
	CRect rcNameRect = CalcNameRect( nPlayers, nPlayer, pDC, sName, pFont );

	int x = rcNameRect.left;
	int y = rcNameRect.top;

	CFont* pfntOld = pDC->SelectObject( pFont );

	pDC->SetBkMode( TRANSPARENT );
	pDC->SetTextAlign( TA_TOP | TA_LEFT );

	pDC->SetTextColor( colShadow );
	pDC->TextOut( x+1, y, CString(sName) );
	pDC->TextOut( x-1, y, CString(sName) );
	pDC->TextOut( x, y+1, CString(sName) );
	pDC->TextOut( x, y-1, CString(sName) );

	pDC->SetTextColor( col );
	pDC->TextOut( x, y, CString(sName) );

	pDC->SelectObject( pfntOld );

	if ( m_nTurnRect == nPlayer ) {

		pDC->SelectStockObject( NULL_BRUSH );

		rcNameRect.InflateRect( 2, 2, 3, 3 );

		CPen penShad( PS_SOLID, 0, colShadow );
		pDC->SelectObject( &penShad );
		pDC->Rectangle( &rcNameRect );

		rcNameRect.OffsetRect( -1, -1 );

		CPen pen( PS_SOLID, 0, col );
		pDC->SelectObject( &pen );
		pDC->Rectangle( &rcNameRect );

		pDC->SelectStockObject( WHITE_PEN );
	}
}

// 가운데 부분의 에니메이션
// 크기는 (카드폭)*2 X (카드높이)*1.5
// x, y 는 에니메이션의 한 가운데 위치
// szCard 는 카드 크기
// nCount 카드 장수 (nStep==0 일때만 사용)
// nStep
// 0 : 가운데 카드가 쌓인 모양
// 1 : 카드가 양쪽으로 분리되는 중
// 2 : 카드가 양쪽으로 분리되었음
// 3 : 카드가 가운데로 합쳐지는 중
void CBoard::DrawCenterAnimation( CDC* pDC, bool bBrushOrigin,
								 CSize szCard, int x, int y,
								 int nStep, int nCardCount )
{
	if ( !m_pMFSM ) return;

	// 가운데 0장 쌓였다면 아무것도 그리지 않는다
	if ( !nStep && !nCardCount ) return;

	CSize szRealCard = GetBitmapSize( IDB_SA );

	// nCount 는 실제 그려지는 카드의 장 수로 매핑한다
	int nCount;
	if ( nCardCount == -1 ) nCardCount = 53;

	if ( nCardCount < 3 ) nCount = nCardCount;
	else if ( nCardCount < 10 ) nCount = nCardCount*2/3;
	else nCount = szCard.cx * (nCardCount-9) / 520 + 6;

	int nMaxCount = szCard.cx * (53-9) / 520 + 6;

	// 1 번 스텝은 특별히 그림자가 필요 !
	if ( nStep == 1 )
		m_bm.DrawShade( pDC,
			x + szCard.cx/4, y,
			szCard.cx/2, szCard.cy/2 + nCount*2,
			bBrushOrigin );

	// 이제 카드를 전부 그린다
	for ( int i = 0; i < nCount; i++ ) {
		int a, b;

		// 이 식은 Board.cpp 의 CalcCardRectForFlyCard 에서도
		// 사용되었으므로 유의
		b = y - szCard.cy/2 + nMaxCount*2 - i*2;

		if ( nStep == 0 ) a = x - szCard.cx/2;
		else if ( nStep == 2 && i % 2 ) a = x - szCard.cx, b = b + i*2 - i/2*2;
		else if ( nStep == 2 ) a = x, b = b + i*2 - i/2*2;
		else if ( nStep == 1 && i < nCount/2
				|| nStep == 3 && i % 2 ) a = x - szCard.cx*3/4;
		else a = x - szCard.cx/4;

		m_bm.DrawCard( pDC, 0, a, b, szCard.cx, szCard.cy );
	}
}

// 손에 든 카드, 득점, 가운데 놓인 카드를 모두 그린다
void CBoard::DrawBoard( CDC* pDC, LPCRECT prc )
{
	const CState* pState = m_pMFSM ? m_pMFSM->GetState() : 0;

	// 클립 사각형
	CRect rcClip( *prc );

	// 바탕 그리기
	CRect rcClient; GetClientRect( &rcClient );
	m_bm.DrawBackground( Mo()->bUseBackground,
		Mo()->bTile, Mo()->bExpand, (COLORREF)Mo()->nBackColor,
		pDC, rcClip, rcClient.Width(), rcClient.Height() );

	if ( !pState ) return;	// 그릴것이 아무것도 없음

	// 실제 플레이어 수
	int nPlayers = pState->nPlayers;

	// 실제로 사람이 플레이 하고 있는가
	bool bHuman = pState->apPlayers[0]->IsHuman();

	// 득점 카드가 그려졌는가
	// 득점 카드가 그려졌다면 그 플레이어의 손에 든 카드도
	// 함께 그려야 한다
	bool bScoreDrawn[MAX_PLAYERS];

	// 득점한 카드
	{
		for ( int i = 0; i < nPlayers; i++ ) {

			bool bReveal = !bHuman || !pState->pRule->bHideScore;

			CRect rcScore = CalcRect(
					nPlayers, CR_SCORE, i, m_szCard.cx, m_szCard.cy );

			CRect rcDraw;
			if ( rcDraw.IntersectRect( &rcScore, &rcClip ) ) {
				bScoreDrawn[i] = true;
				DrawHand( pDC, i, nPlayers,
					pState->apPlayers[i]->GetScore(),
					rcDraw.left, rcDraw.top, rcDraw.Width(), rcDraw.Height(),
					0, 0, bReveal, 0, 0, true );
			}
			else bScoreDrawn[i] = false;
		}
	}

	// 가운데 영역
	{
		CRect rcDraw;
		CRect rcCenter = CalcRect(
			nPlayers, CR_CENTER, 0, m_szCard.cx, m_szCard.cy );

		if ( rcDraw.IntersectRect( &rcCenter, &rcClip ) ) {

			// 현재 게임 중이면 가운데에
			// '바닥 카드 리스트(lCurrent)'를 그린다
			if ( pState && pState->state == msTurn )

				DrawCenter( pDC, nPlayers,
					&pState->lCurrent, pState->nBeginer,
					rcDraw.left, rcDraw.top, rcDraw.Width(), rcDraw.Height(),
					0, 0 );

			// 그렇지 않으면 덱을 그린다
			// (카드를 섞는 중에는 그리지 않는다)
			else if ( pState && !m_hSuffling ) {
				CRect rc; GetClientRect( &rc );
				DrawCenterAnimation(
					pDC, false, m_szCard,
					rc.left+rc.Width()/2, rc.top+rc.Height()/2,
					0, pState->lDeck.GetCount() );
			}
		}
	}

	// 손에 든 카드
	{
		for ( int i = 0; i < nPlayers; i++ ) {

			bool bReveal = !bHuman || i == 0 || i == m_nDealMiss
				? true : false;
#ifdef _DEBUG
//bReveal = true;
#endif
			int nMouseOver = !bHuman || i != 0 ? -1 : m_nMouseOver;
			bool* abSelected =
				!bHuman || i != 0 || pState->state != msPrivilege
				? 0 : m_abSelected; 

			CRect rcHand = CalcRect(
					nPlayers, CR_HAND, i, m_szCard.cx, m_szCard.cy );

			// 카드가 위로 뽑혀 있는 경우를 대비
			rcHand.top -= m_szCard.cy / CARD_SHIFT_RATIO_OF_SELECTION;

			if ( bScoreDrawn[i] || !( rcHand & rcClip ).IsRectEmpty() )

				DrawHand( pDC, i, nPlayers,
					pState->apPlayers[i]->GetHand(),
					rcHand.left, rcHand.top, rcHand.Width(), rcHand.Height(),
					0, 0, bReveal, nMouseOver, abSelected, false );
		}
	}

	// 이름
	{
		for ( int i = 0; i < nPlayers; i++ ) {

			// 여당인가
			bool bDef = pState->nMaster == i
				|| pState->bFriendRevealed && pState->nFriend == i;
			DrawName( pDC, nPlayers, pState->apPlayers[i]->GetName(), i,
				bDef ? Mo()->nDefColor : Mo()->nAttColor,
				RGB(0,0,0), &m_fntSmall );
		}
	}
}

// DSB 를 그린다
// Board 와 같은 좌표계에 그려주지만
// xOff 와 yOff 로 그려주는 옵셋을 변경할 수 있다
void CBoard::DrawDSB( CDC* pDC, bool bBrushOrigin,
						int x, int y, int cx, int cy,
						int xOff, int yOff )
{
	CRect rcClipBox( x, y, x+cx, y+cy );

	// 마지막 DSB 부터 꺼꾸로 그려 준다
	// 선두의 DSB 가 z order 중 최 상단이므로
	POSITION pos = m_lpDSB.GetTailPosition();
	while (pos) {
		DSB* pDSB = m_lpDSB.GetPrev(pos);
		CRect rc; pDSB->GetRect( &rc );

		if ( !( rcClipBox & rc ).IsRectEmpty() ) {

			// pDSB 의 옵셋을 잠시 새로 그릴 옵셋으로 맞춘다
			// DSB 는 내부적으로 자신의 정보를 통한 절대 좌표를 사용하기 때문이다
			pDSB->Offset( xOff, yOff );
			pDSB->OnDraw( pDC, bBrushOrigin );
			pDSB->Offset( -xOff, -yOff );
		}
	}
}

// 플레이어나 바닥 정보, 가운데 정보, 이름 등을 갱신한다
// 이 위의 DSB 들도 함께 갱신된다
// nPlayer 가 -1 이면 바닥 정보를 갱신
// nPlayer 가 -2 이면 모든 정보를 갱신
// prc 가 0 이면 그 플레이어의 모든 정보를 갱신
//        그렇지 않으면 그 영역만을 갱신
// bUpdateNow : 즉시 윈도우를 Update 한다
// bSelectionRegion : 이 값이 참이면 0 번 플레이어의 선택 영역, 점수 영역까지 함께 갱신한다
void CBoard::UpdatePlayer( int nPlayer, int x, int y, int cx, int cy, bool bUpdateNow, bool bSelectionRegion )
{
	CRect rc( x, y, x+cx, y+cy );
	UpdatePlayer( nPlayer, &rc, bUpdateNow, bSelectionRegion );
}

void CBoard::UpdatePlayer( int nPlayer, LPRECT prc, bool bUpdateNow, bool bSelectionRegion )
{
	// MFSM 값은 세트되어 있어야 한다
	// 그렇지 않을 경우 nPlayer == -2 (전체) 만이 가능하다
	const CState* pState = m_pMFSM ? m_pMFSM->GetState() : 0;

	ASSERT( pState || nPlayer == -2 );
	if ( !pState && nPlayer != -2 ) return;

	CRect rc;
	{
		CClientDC dcClient(this);
		CDC dcMem; dcMem.CreateCompatibleDC( &dcClient );
		dcMem.SelectObject( &m_board );

		if ( prc == NULL ) {

			if ( nPlayer == -2 )
				// nPlayer == -2 이면 전체 사각형
				rc.SetRect( 0, 0, m_szPic.cx, m_szPic.cy );

			else if ( nPlayer == -1 )
				// nPlayer == -1 이면 가운데 사각형
				rc = CalcRect(
					pState->nPlayers, CR_CENTER, 0,
					m_szCard.cx, m_szCard.cy );

			else {
				// 특정 플레이어

				// 손에 든 카드
				rc = CalcRect(
						pState->nPlayers, CR_HAND, nPlayer,
						m_szCard.cx, m_szCard.cy );
				if ( nPlayer == 0 )	// 0번 플레이어의 카드 이동량 고려
					if ( bSelectionRegion )
						rc.top -= m_szCard.cy/CARD_SHIFT_RATIO_OF_SELECTION;
					else rc.top -= m_szCard.cy/CARD_SHIFT_RATIO_OF_MOUSE_OVER;

				// 점수 카드
				if ( bSelectionRegion )
					rc |= CalcRect( pState->nPlayers, CR_SCORE, nPlayer,
									m_szCard.cx, m_szCard.cy );
			}
			prc = &rc;
		}

		// 보드 그리기
		dcMem.IntersectClipRect( prc );
		DrawBoard( &dcMem, prc );
	}
	// DSB 그리기
	UpdateDSB( prc, bUpdateNow );
}

// 게임 화면을 갱신한다
// 지정된 영역의 버퍼를 다시 그린 후 그 범위의 실제 윈도우도
// 무효화 시킨다
void CBoard::UpdateDSB( int x, int y, int cx, int cy, bool bUpdateNow )
{
	CRect rc( x, y, x+cx, y+cy );
	UpdateDSB( &rc, bUpdateNow );
}

void CBoard::UpdateDSB( LPCRECT prc, bool bUpdateNow )
{
	{
		CClientDC dcClient(this);
		CDC dcPic; dcPic.CreateCompatibleDC( &dcClient );
		dcPic.SelectObject( &m_pic );

		CRect rc;
		if ( prc == NULL ) {
			rc.SetRect( 0, 0, m_szPic.cx, m_szPic.cy );
			prc = &rc;
		}

		dcPic.IntersectClipRect( prc );

		// 보드 정보 복사해 오기
		{
			CDC dcBoard; dcBoard.CreateCompatibleDC( &dcClient );
			dcBoard.SelectObject( &m_board );

			dcPic.BitBlt( prc->left, prc->top,
				prc->right-prc->left, prc->bottom-prc->top,
				&dcBoard, prc->left, prc->top, SRCCOPY );
		}

		m_csFlying.Lock();

		// 카드를 섞는 중이면 그 그림도 함께 그린다
		if ( m_pMFSM && m_hSuffling ) {

			SUFFLECARDS_INFO* pInfo = (SUFFLECARDS_INFO*)m_hSuffling;

			// 바탕 그리기
			CRect rcClient; GetClientRect( &rcClient );
			CRect rcDraw(	pInfo->xCenter - pInfo->sz.cx,
							pInfo->yCenter - pInfo->sz.cy,
							pInfo->xCenter + pInfo->sz.cx,
							pInfo->yCenter + pInfo->sz.cy );
			m_bm.DrawBackground( Mo()->bUseBackground,
				Mo()->bTile, Mo()->bExpand, (COLORREF)Mo()->nBackColor,
				&dcPic, &rcDraw, rcClient.Width(), rcClient.Height() );
			// 섞는 그림 그리기
			DrawCenterAnimation( &dcPic,
				pInfo->bBrushOrigin, m_szCard,
				pInfo->xCenter, pInfo->yCenter,
				pInfo->nCurStep );
		}
		// 카드를 날리는 중이면 그 카드도 함께 그린다
		if ( m_pMFSM && m_hFlying ) {

			FLYCARD_INFO* pInfo = (FLYCARD_INFO*)m_hFlying;

			// 그림자
			if ( pInfo->nCurState <= pInfo->nStates )
				m_bm.DrawShade( &dcPic,
					pInfo->x + pInfo->nCurShadeDepth,
					pInfo->y + pInfo->nCurShadeDepth,
					pInfo->rcBegin.Width(), pInfo->rcBegin.Height(), false );

			// 카드
			m_bm.DrawCard( &dcPic, pInfo->nCard,
				pInfo->x, pInfo->y,
				pInfo->rcBegin.Width(), pInfo->rcBegin.Height() );
		}

		m_csFlying.Unlock();

		// DSB 그리기
		DrawDSB( &dcPic, false, prc->left, prc->top,
			prc->right - prc->left, prc->bottom - prc->top);
	}

	InvalidateRect( prc, FALSE );
	if ( bUpdateNow ) {
		UpdateWindow();
		GdiFlush();
	}
}

// 카드를 날린다 !
// 원하는 옵션을 주면 카드를 날리기 시작하면서
// 핸들을 리턴한다 - 이 핸들을 다시
// 함수의 인자로 전달하면 다음 핸들을
// 리턴하면서 조금씩 카드 날리기를 진행한다.
// 날리기가 끝났으면 0 을 리턴한다
// 윈도우가 닫혔으면 언제든 자원을 해제하고 0 을 리턴한다
volatile void* CBoard::FlyCard(
	volatile void*& h,		// 핸들
	int nCard,				// 날릴 카드
	const CRect& rcBegin,	// 시작되는 카드 사각형
	const CPoint& ptEnd,	// 끝나는 위치 ( 사각형의 좌상단 )
	int nStates,			// 몇번만에 날리기 완료하는가
	int nShadeMode )		// 그림자 모드
{
#define SHADE_OFFSET 10		// 그림자 옵셋
	ASSERT( nStates > 0 || rcBegin.TopLeft() == ptEnd );

	// 새로운 상태 정보를 만든다
	FLYCARD_INFO* pInfo = new FLYCARD_INFO;
	pInfo->nCard = nCard;
	pInfo->rcBegin = rcBegin;
	pInfo->ptEnd = ptEnd;
	pInfo->nStates = nStates;
	pInfo->nShadeMode = nShadeMode;
	pInfo->nCurState = -1;
	pInfo->nCurShadeDepth = pInfo->nShadeMode % 2 ?
		SHADE_OFFSET * pInfo->rcBegin.Height()/m_szCard.cy : 0;
	pInfo->x = pInfo->xLast = rcBegin.left;
	pInfo->y = pInfo->yLast = rcBegin.top;
	pInfo->sz.cx = rcBegin.Width()+SHADE_OFFSET;
	pInfo->sz.cy = rcBegin.Height()+SHADE_OFFSET;

	// 뷰의 크기 변경을 방지함
	Mf()->LockSizing();

	m_csFlying.Lock();
		h = (void*)pInfo;
	m_csFlying.Unlock();

	return pInfo;
}

volatile void* CBoard::FlyCard( volatile void*& h, int nStep )
{
	ASSERT(h);
	if ( !h ) return 0;

	// 쓰레드 간 충돌을 피하기 위해 마셜링 한다
	CBoardWrap* pWrap = GetWrapper();

	m_csFlying.Lock();

	FLYCARD_INFO* pInfo = (FLYCARD_INFO*)h;

	if ( !GetSafeHwnd() ) {
		// 윈도우가 갑자기 사라졌음 !
		delete pInfo;
		h = 0;
		m_csFlying.Unlock();
		return h;
	}

	CSize szRealCard = GetBitmapSize( IDB_SA );

	pInfo->nCurState = min( pInfo->nCurState + nStep,
							pInfo->nStates + 1 );

	// 그림자 깊이
	if ( pInfo->nShadeMode == 1 )		// 위에서 아래로
		pInfo->nCurShadeDepth =
			SHADE_OFFSET*(pInfo->nStates-pInfo->nCurState)
			*pInfo->rcBegin.Height()/pInfo->nStates/m_szCard.cy;
	else if ( pInfo->nShadeMode == 2 )	// 아래에서 위로
		pInfo->nCurShadeDepth =
			SHADE_OFFSET*pInfo->nCurState
			*pInfo->rcBegin.Height()/pInfo->nStates/m_szCard.cy;
	else if ( pInfo->nShadeMode == 3 )	// 위에서 위로
		pInfo->nCurShadeDepth = SHADE_OFFSET
			*pInfo->rcBegin.Height()/m_szCard.cy;
	else pInfo->nCurShadeDepth = 0;

	pInfo->xLast = pInfo->x;
	pInfo->yLast = pInfo->y;

	// 이번에 찍을 좌표를 계산
	pInfo->x = pInfo->rcBegin.left
			+ (pInfo->ptEnd.x-pInfo->rcBegin.left)
				* pInfo->nCurState / ( pInfo->nStates + 1 );
	pInfo->y = pInfo->rcBegin.top
			+ (pInfo->ptEnd.y-pInfo->rcBegin.top)
				* pInfo->nCurState / ( pInfo->nStates + 1 );

	// 그릴 사각형 (지난 사각형과 현재 사각형의 합집합)
	CRect rcDraw(
		min( pInfo->xLast, pInfo->x ),
		min( pInfo->yLast, pInfo->y ),
		max( pInfo->xLast, pInfo->x )+pInfo->sz.cx,
		max( pInfo->yLast, pInfo->y )+pInfo->sz.cy );

	// 카드 날리기가 종료되었다
	if ( pInfo->nCurState > pInfo->nStates ) {

		m_csFlying.Unlock();

		// 마지막에도 한번 그려준다 ( update 를 즉시 하지는 않는다 )
		pWrap->UpdateDSB( &rcDraw );

		// 자원을 반환
		m_csFlying.Lock();
			h = 0;
			delete pInfo;
		m_csFlying.Unlock();

		// 뷰의 크기 변경 방지를 해제
		Mf()->UnlockSizing();

		return 0;
	}

	m_csFlying.Unlock();

	// 그린다 !!
	pWrap->UpdateDSB( &rcDraw, true );

	ASSERT( pInfo->nStates > 0 );

	return h;
}

// 카드를 섞는 에니메이션을 보인다
// 이 함수는 위의 SuffleCards 의 저수준 버전으로
// DrawCenterAnimation 함수를 사용한다
// FlyCard 와 같은 방식으로 호출한다
// 윈도우가 닫혔으면 언제든 자원을 해제하고 0 을 리턴한다
volatile void* CBoard::SuffleCards(
	volatile void*& h,		// 핸들
	int nCount,				// 섞을 회수
	SIZE szCard )			// 카드 크기
{
	// 화면 사각형
	CRect rcClient;
	GetClientRect( &rcClient );

	SUFFLECARDS_INFO* pInfo = new SUFFLECARDS_INFO;
	pInfo->nCount = nCount + 1;
	pInfo->sz.cx = szCard.cx * 2;
	pInfo->sz.cy = szCard.cy * 3/2;
	pInfo->nCurCount = 0;
	pInfo->nCurStep = 3;
	pInfo->xCenter = rcClient.left + rcClient.Width()/2;
	pInfo->yCenter = rcClient.top + rcClient.Height()/2;
	pInfo->bBrushOrigin = ( pInfo->xCenter + pInfo->sz.cx/2
							+ pInfo->yCenter + pInfo->sz.cy/2 )
							% 2 ? false : true;

	// 뷰의 크기 변경을 방지
	Mf()->LockSizing();

	// 상태 바에 진행바를 그리기 시작한다
	Ib()->SetText( _T("카드를 섞는 중입니다...") );
	Ib()->SetProgress( 0 );

	m_csFlying.Lock();
		h = pInfo;
	m_csFlying.Unlock();

	return pInfo;
}

volatile void* CBoard::SuffleCards( volatile void*& h )
{
	ASSERT(h);
	if ( !h ) return 0;

	// 쓰레드 간 충돌을 피하기 위해 마셜링 한다
	CBoardWrap* pWrap = GetWrapper();

	m_csFlying.Lock();

	SUFFLECARDS_INFO* pInfo = (SUFFLECARDS_INFO*)h;

	if ( ( ++pInfo->nCurStep %= 4 ) == 0 )
		pInfo->nCurCount++;

	if ( !GetSafeHwnd() ) {
		// 윈도우가 갑자기 사라졌음 !
		delete pInfo;
		h = 0;
		m_csFlying.Unlock();
		return 0;
	}

	// 그릴 사각형
	CRect rcDraw(
		CPoint(	pInfo->xCenter - pInfo->sz.cx/2,
				pInfo->yCenter - pInfo->sz.cy/2 ),
		CSize(	pInfo->sz.cx, pInfo->sz.cy ) );

	m_csFlying.Unlock();

	// 그린다 !!
	pWrap->UpdateDSB( &rcDraw, true );

	if ( pInfo->nCurCount >= pInfo->nCount ) {
		// 지정된 회수만큼 섞고 이제 마칠 때가 되었다

		// 자원을 반환
		m_csFlying.Lock();
			h = 0;
			delete pInfo;
		m_csFlying.Unlock();

		// 뷰의 크기 변경 방지를 해제
		Mf()->UnlockSizing();
		// 상태바 원위치
		Ib()->Reset();

		return 0;
	}
	else {
		ASSERT( pInfo->nCount > 0 );

		// 소리 출력
		if ( pInfo->nCurStep == 3 )
			PlaySound( IDW_SUFFLE, true );

		// 상태바 출력
		Ib()->SetProgress(
			( (pInfo->nCurCount-1)*4 + pInfo->nCurStep )
				* 100 / ( (pInfo->nCount-1)*4 ) );

		return h;
	}
}
