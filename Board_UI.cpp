// Board_UI.cpp : CBoard 중 사용자 인터페이스 관련 부분을 정의
//

#include "stdafx.h"
#include "Mighty.h"
#include "BmpMan.h"
#include "Board.h"
#include "Play.h"
#include "Player.h"
#include "MFSM.h"
#include "DSB.h"
#include "DScoreBoard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// OnLButtonDown 에 대해서 DSB 처리를 해 주는 함수
bool CBoard::CookDSBForLButtonDown( POINT point )
{
	POSITION pos = m_lpDSB.GetHeadPosition();
	for (; pos; m_lpDSB.GetNext(pos) ) {
		DSB* pDSB = m_lpDSB.GetAt(pos);

		// 모달 상태이면 모달 DSB 만 검사한다
		if ( !m_lpModalDSB.IsEmpty()
			&& pDSB != m_lpModalDSB.GetHead() ) continue;

		CRect rc; pDSB->GetRect( &rc );
		if ( rc.PtInRect( point ) )
			// DSB 에 클릭되었는가

			if ( pDSB->DispatchHotspot( point.x, point.y, false) )
				// 클릭이 처리됨
				return true;

			else {
				// 포커스 이동
				if ( m_pDragDSB && m_pDragDSB != pDSB )
					m_pDragDSB->OnFocus( false );

				// 드래깅을 시작 !
				m_bDragDSB = true; m_pDragDSB = pDSB;
				m_pntLastDSB = point;
				SetCapture();
				RECT rcClient; GetClientRect(&rcClient);
				ClientToScreen( &rcClient );
				ClipCursor( &rcClient );

				// 자동으로 사라지지 않게 만든다
				m_pDragDSB->SetPermanent();
				// 맨 앞으로 이동
				m_lpDSB.RemoveAt(pos);
				m_lpDSB.AddHead( pDSB );
				pDSB->OnFocus( true );
				UpdateDSB( rc );
				return true;
			}
	}

	// 모달 상태면 이후의 작업은 방지한다
	if ( !m_lpModalDSB.IsEmpty() ) return true;

	// LButtonDown 처리를 계속
	return false;
}

// OnLButtonDown 에 대해서 카드 선택 처리를 해 주는 함수
bool CBoard::CookSelectionForLButtonDown( POINT point )
{
	ASSERT( m_pWaitingSelectionEvent );
	ASSERT( m_pWaitingSelectionCard );

	if ( !m_pMFSM ) return false;

	if ( !CanSelect() )
		// 선택 불가능하면 무효
		return false;

	POSITION pos = CardFromPoint( point.x, point.y );
	if ( pos ) {
		const CState* pState = m_pMFSM->GetState();
		*m_pWaitingSelectionCard =
			pState->apPlayers[0]->GetHand()->GetAt(pos);
		m_pWaitingSelectionEvent->SetEvent();
	}
	// 찾지 못했다면 엉뚱한 곳을 클릭한 것이다
	// 삑 소리를 낸다
	else if ( Mo()->bUseSound )
		MessageBeep( MB_OK );

	return true;
}

// 지정된 위치의 카드를 들어 올린다
// (m_abSelected 나 m_nMouseOver 값을 바꾼 후 Update 한다)
// bMouseOver 가 true 이면 m_nMouseOver 를 변경하고
// 이전에 올려졌던 카드는 내려준다
// false 이면 m_abSelected 의 값을 토글하고
// 결과를 반영한다
void CBoard::ShiftCard( int x, int y, bool bMouseOver )
{
	ASSERT( m_pMFSM );
	if ( !m_pMFSM ) return;

	const CState* pState = m_pMFSM->GetState();

	// 플레이어가 사람인 경우에만 해당된다
	if ( !pState->apPlayers[0]->IsHuman() ) return;
	// 자기 턴인 경우에만 해당된다
	if ( pState->state != msTurn
		|| pState->nCurrentPlayer != 0 ) return;

	// 손에 든 카드
	CCardList* pHand = pState->apPlayers[0]->GetHand();

	// 올려질 카드의 위치와 인덱스
	int index = -1;
	if ( bMouseOver ) {
		POSITION pos = CardFromPoint( x, y );
		index = pos ? pHand->IndexFromPOSITION( pos ) : -1;
	}

	if ( bMouseOver && m_nMouseOver == index )
		// 이미 올려져 있는 카드이기 때문에 그냥 리턴
		return;

	CRect rcUpdate(0,0,0,0);

	if ( m_nMouseOver >= 0 ) {	// 다른 카드가 올려져 있었음
		CRect rc = CalcRect( pState->nPlayers,
						CR_HAND, 0, m_szCard.cx, m_szCard.cy,
						m_nMouseOver, pHand->GetCount() );
		rc.top -= m_szCard.cy/CARD_SHIFT_RATIO_OF_SELECTION;

		rcUpdate |= rc;	// 이전 영역도 update 한다
	}

	m_nMouseOver = index;

	if ( m_nMouseOver >= 0 ) {	// 새로운 카드를 올린다
		CRect rc = CalcRect( pState->nPlayers,
							CR_HAND, 0, m_szCard.cx, m_szCard.cy,
							m_nMouseOver, pHand->GetCount() );
			rc.top -= m_szCard.cy/CARD_SHIFT_RATIO_OF_SELECTION;
			rcUpdate |= rc;
	}
	UpdatePlayer( 0, &rcUpdate, true );
}

// 현재 커서 아래쪽의 카드를 낼 수 있는가 !
bool CBoard::CanSelect() const
{
	const CState* pState = m_pMFSM ? m_pMFSM->GetState() : 0;

	// 카드 선택중이고 게임이 시작되었어야 하고
	// 플레이어가 사람이어야 함
	if ( !m_pWaitingSelectionEvent
		|| !pState || !pState->apPlayers[0]->IsHuman() )
		return false;

	// 커서의 클라이언트 위치
	POINT pntCursor;
	GetCursorPos( &pntCursor );
	ScreenToClient( &pntCursor );

	// 손에 든 카드
	CCardList* pHand = pState->apPlayers[0]->GetHand();
	// 올려질 카드의 위치와 인덱스
	POSITION pos = CardFromPoint( pntCursor.x, pntCursor.y );

	// 카드 위에 있어야 한다
	if ( !pos ) return false;

	// 만약 특권 상태(3장을 버리는)라면 모든 카드를 선택 가능
	if ( pState->state == msPrivilege ) return true;

	// 그렇지 않으면 낼 수 있는 카드만 선택 가능
	CCardList lAv;
	pHand->GetAvList( &lAv, &pState->lCurrent, pState->nTurn,
		pState->nJokerShape, pState->bJokercallEffect );
	if ( !lAv.Find( pHand->GetAt(pos) ) )
		return false;
	else return true;
}

int CBoard::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const
{
	__declspec(thread) static int s_cLastTooltipCard = 0;

	if ( !m_pMFSM || !Mo()->bCardTip
		|| !Mo()->bUseHintInNetwork
			&& m_pMFSM->GetState()->IsNetworkGame() )
		return -1;

	const CState* pState = m_pMFSM->GetState();

	if ( pState->state != msTurn ) return -1;

	if ( pTI ) {

		pTI->cbSize = sizeof(TOOLINFO);
		pTI->uFlags = TTF_TRANSPARENT;
		pTI->hwnd = GetSafeHwnd();
		pTI->hinst = 0;
		pTI->lpszText = LPSTR_TEXTCALLBACK;
	}

	// 플레이어의 손에 든 카드 팁
	POSITION pos =
		!pState->apPlayers[0]->IsHuman() ? 0
		: CardFromPoint( point.x, point.y );
	if ( pos ) {

		const CCardList* pHand = pState->apPlayers[0]->GetHand();
		CCard c( pHand->GetAt(pos) );

		if ( c != CCard(s_cLastTooltipCard) ) {
			s_cLastTooltipCard = c;
			return -1;
		}

		int nIndex = pHand->IndexFromPOSITION(pos);

		if ( pTI ) {
			pTI->uId = (UINT)(int)c;
			pTI->rect = CalcRect( pState->nPlayers, CR_HAND, 0,
								m_szCard.cx, m_szCard.cy,
								nIndex, pHand->GetCount() );
			if ( nIndex+1 < pHand->GetCount() )
				pTI->rect.right = CalcRect( pState->nPlayers, CR_HAND, 0,
								m_szCard.cx, m_szCard.cy,
								nIndex+1, pHand->GetCount() ).left;
		}
		return 1;
	}

	// 각 플레이어의 이름 & 득점 팁
	int i;
	for ( i = 0; i < pState->nPlayers; i++ ) {

		CRect rc = CalcRect( pState->nPlayers, CR_HAND, i,
						m_szCard.cx, m_szCard.cy )
				| CalcRect( pState->nPlayers, CR_SCORE, i,
						m_szCard.cx, m_szCard.cy );

		if ( rc.PtInRect(point) ) {

			if ( s_cLastTooltipCard != 0 ) {
				s_cLastTooltipCard = 0; return -1;
			}
			if ( pTI ) {
				pTI->rect = rc;
				pTI->uId = 100 + i;
			}
			s_cLastTooltipCard = 0;
			return 1;
		}
	}

	// 가운데 카드
	int nCount = pState->lCurrent.GetCount();
	pos = pState->lCurrent.GetTailPosition();
	for ( i = nCount-1; i >= 0; i-- ) {

		CCard c = pState->lCurrent.GetPrev(pos);

		int n = ( pState->nBeginer + i ) % pState->nPlayers;
		CRect rc = CalcRect( pState->nPlayers, CR_CENTER, 0,
						m_szCard.cx, m_szCard.cy,
						n, pState->nPlayers );
		if ( rc.PtInRect(point) ) {

			if ( c != CCard(s_cLastTooltipCard) ) {
				s_cLastTooltipCard = c;
				return -1;
			}
			if ( pTI ) {
				pTI->uId = (UINT)(int)c;
				pTI->rect = rc;
			}
			return 1;
		}
	}
	
	return -1;
}

BOOL CBoard::OnToolTipNeedText( UINT /*id*/, NMHDR * pNMHDR, LRESULT* /*pResult*/ )
{
	__declspec(thread) static TCHAR s_sBuff[64];

	if ( !m_pMFSM ) return FALSE;

    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;

	const CState* pState = m_pMFSM->GetState();
	int id = (int)pNMHDR->idFrom;
	CString s;

	if ( 100 <= id && id < 150 )		// 플레이어 이름
		s.Format( _T("%s %d 장 득점"),
			pState->apPlayers[id-100]->GetName(),
			pState->apPlayers[id-100]->GetScore()->GetCount() );

	else if ( 1 <= id && id <= 53 )	{	// 카드
		CCard c((int)id);
		s = m_pMFSM->GetHelpString( c );
	}

	else return FALSE;

	strcpy_s( s_sBuff, s );
	pTTT->lpszText=s_sBuff;
	return TRUE;
}

// 플레이어의 이름을 옵션에서 다시 로드한다
void CBoard::ReloadPlayerNames()
{
	// 네트워크 게임 중이라면 변경하지 않는다
	if ( m_pMFSM && m_pMFSM->GetState()->IsNetworkGame() ) return;

	if ( m_pMFSM ) {
		// 게임중

		LPCTSTR asNames[MAX_PLAYERS];
		for ( int i = 0; i < MAX_PLAYERS; i++ )
			asNames[i] = Mo()->aPlayer[i].sName;
		m_pMFSM->SetPlayerNames( asNames );

		Sb()->Update( m_pMFSM->GetState() );
	}
	else Sb()->Update();
}
