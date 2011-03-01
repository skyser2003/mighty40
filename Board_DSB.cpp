// Board_DSB.cpp : DSB-related CBoard function bodies
//

#include "stdafx.h"
#include "Mighty.h"
#include "DSB.h"
#include "DEtc.h"
#include "DElection.h"
#include "Play.h"
#include "Player.h"
#include "MFSM.h"
#include "BmpMan.h"
#include "Board.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// 글꼴 ( 0 small 1 medium 2 big, 음수이면 -nSize-1 의 Fixed )
CFont* CBoard::GetFont( int nSize )
{
	switch ( nSize ) {
	case 1: return &m_fntMiddle;
	case 2: return &m_fntBig;
	case -1: return &m_fntFixedSmall;
	case -2: return &m_fntFixedMiddle;
	case -3: return &m_fntFixedBig;
	default: return &m_fntSmall;
	}
}

// 이 스트링의 화면에서의 크기를 구한다 (지정한 폰트 사용)
CSize CBoard::GetTextExtent( int nSize, LPCTSTR s )
{
	CClientDC dc(this);
	dc.SelectObject( GetFont(nSize) );
	return dc.GetTextExtent( s, _tcslen(s) );
}

// DSB 에서 사용하는 기준 유닛 크기 (최소폰트의 높이임)
int CBoard::GetDSBUnit()
{
	// 아직 폰트가 만들어 지지 않은 경우
	if ( !m_fntSmall.GetSafeHandle() ) return 0;

	LOGFONT lf;
	m_fntSmall.GetLogFont(&lf);
	if ( lf.lfHeight >= -5 ) return max( lf.lfHeight, 5 );
	else return -lf.lfHeight;
}

void CBoard::AddDSB( DSB* pDSB )
{
	ASSERT( GetSafeHwnd() );

	// 맨 밑의 모덜보다 더 밑으로 들어간다
	// Below 면 맨 밑의 일반보다 더 밑으로 들어간다
	POSITION pos = m_lpDSB.GetHeadPosition();
	for ( ; pos; m_lpDSB.GetNext(pos) ) {

		DSB* pCur = m_lpDSB.GetAt(pos);
		int i = pDSB->IsModal() ? 2 : pDSB->IsBelow() ? 0 : 1;
		int j = pCur->IsModal() ? 2 : pCur->IsBelow() ? 0 : 1;

		if ( i >= j ) {
			m_lpDSB.InsertBefore( pos, pDSB );
			break;
		}
	}
	if ( !pos ) m_lpDSB.AddTail(pDSB);

	if ( pDSB->IsModal() ) m_lpModalDSB.AddHead( pDSB );

	pDSB->OnFocus( true );
	pDSB->SetBmpMan( &m_bm );
	pDSB->OnInit();
	pDSB->OnMove();

	SetDSBTimer( pDSB );

	// 사각형의 영역을 다시 그린다
	RECT rc;
	pDSB->GetRect( &rc );
	UpdateDSB( &rc );
}

// DSB 의 타이머를 설정
void CBoard::SetDSBTimer( DSB* pDSB )
{
	// 타이머 아이디로 tiDSB + DSB ID 를 사용한다
	UINT nIDEvent = UINT( (int)tiDSB + pDSB->GetID() );

	if ( !pDSB->IsPermanent() )
		if ( !SetTimer( nIDEvent, pDSB->GetTimeOut(), NULL ) ) {
			// 타이머를 얻을 수 없는 경우
			// 그 시간 만큼 슬립 한 후 없앤다 !
			ASSERT(0);
			Sleep( pDSB->GetTimeOut() );
			pDSB->Destroy();
		}
}

// DSB 를 삭제
bool CBoard::RemoveDSB( DSB* pDSB )
{
	// 해당 dsb 를 찾는다
	POSITION pos = m_lpDSB.GetHeadPosition();
	for ( ; pos; m_lpDSB.GetNext(pos) )
		if ( m_lpDSB.GetAt(pos)->GetID() == pDSB->GetID() ) {

			m_lpDSB.GetAt(pos)->OnFocus(false);
			m_lpDSB.RemoveAt(pos);	// 일단 사각형을 지워야 한다

			// 이 DSB 를 가리키던 포인터들을 모두 해제한다

			// 하일라이트 되었었음
			if ( m_pHLDSB == pDSB )
				m_pHLDSB = 0;
			// 드래그 중이었음
			if ( m_pDragDSB == pDSB ) {
				m_pDragDSB = 0;
				m_bDragDSB = false;
				ReleaseCapture();
				ClipCursor( 0 );
			}
			// 채팅, 공약 DSB 였음
			for ( int t = 0; t < MAX_PLAYERS; t++ )
				if ( m_apSayDSB[t] == pDSB ) {
					m_apSayDSB[t] = 0; break; }
				else if ( m_apGoalDSB[t] == pDSB ) {
					m_apGoalDSB[t] = 0; break; }
			// 당선되려는 공약 DSB 였음
			if ( m_pMasterGoalDSB == pDSB )
				m_pMasterGoalDSB = 0;
			// 선거 DSB 였음
			if ( m_pCurrentElectionDSB == pDSB )
				m_pCurrentElectionDSB = 0;
			// 모달 DSB 였음
			POSITION posModal = m_lpModalDSB.GetHeadPosition();
			for ( ; posModal; m_lpModalDSB.GetNext(posModal) )
				if ( m_lpModalDSB.GetAt(posModal) == pDSB ) {
					m_lpModalDSB.RemoveAt(posModal); break; }

			if ( GetSafeHwnd() ) {
				RECT rc;
				pDSB->GetRect( &rc );
				// 혹시 모르니 타이머 id 도 없앤다
				KillTimer( UINT( (int)tiDSB + pDSB->GetID() ) );
				// 업데이트
				UpdateDSB( &rc, true );
			}
			return true;
		}

	ASSERT(0);
	return false;
}

// DSB 를 출력하기 위한 대략적인 플레이어 위치를 계산
// 내부적으로 그 플레이어의 '점수영역'을 리턴한다
// ( nPlayerNum 이 -1 이면 전체 클라이언트의 한 가운데 )
// pbVert 에는 영역이 세로로 긴가 여부를 리턴한다
RECT CBoard::CalcPlayerExtent( int nPlayerNum, bool* pbVert )
{
	// 그 플레이어의 점수 영역에서 카드 영역을 제외한 다음
	// 전체 클라이언트 영역과 교집합을 리턴한다 !

	const CState* pState = m_pMFSM ? m_pMFSM->GetState() : 0;
	bool bVert = false;
	if ( pbVert ) *pbVert = bVert;	// 임시로 클리어

	CRect rcClient;
	{
		GetClientRect( &rcClient );
		if ( !pState || nPlayerNum < 0 ) return rcClient;
	}

	CRect rcScore = CalcRect( pState->nPlayers, CR_SCORE, nPlayerNum,
					m_szCard.cx, m_szCard.cy, -1, 0, &bVert, 0 );
	if ( nPlayerNum >= pState->nPlayers ) return rcScore;
	CRect rcHand = CalcRect( pState->nPlayers, CR_HAND, nPlayerNum,
					m_szCard.cx, m_szCard.cy );

	if ( pbVert ) *pbVert = bVert;

	// 점수 영역에서 카드 영역을 제외
	if ( bVert ) {
		if ( rcHand.left < rcScore.left ) rcScore.left = rcHand.right;
		else rcScore.right = rcHand.left;
	} else {
		if ( rcHand.top < rcScore.top ) rcScore.top = rcHand.bottom;
		else rcScore.bottom = rcHand.top;
	}
	// 클라이언트로 클립
	rcScore &= rcClient;

	return rcScore;
}

// 해당 플레이어 자리에 채팅 DSB 를 띄운다
// nPlayerID : 플레이어 ID
// sMsg    : 메시지
void CBoard::FloatSayDSB( int nPlayerID, LPCTSTR sMsg )
{
	int nPlayer = -1;	// 플레이어 번호
	const CState* pState = 0;
	if ( m_pMFSM ) {
		pState = m_pMFSM->GetState();
		nPlayer = m_pMFSM->GetPlayerNumFromID( nPlayerID );
	}
	if ( !pState ) return;

	CRect rcRange = CalcPlayerExtent( nPlayer );

	int x = rcRange.left + rcRange.Width()/2;
	int y = rcRange.top + rcRange.Height()/2;

	int nUnit = GetDSBUnit();
	CRect rcClient; GetClientRect( &rcClient );
	int cxDSB = rcClient.Width()*2/5 / nUnit - 2;

	DSay* pDSB = new DSay(m_pWrap);
	pDSB->Create( m_apSayDSB[nPlayerID],
		x, y, cxDSB,
		pState->apAllPlayers[nPlayerID]->GetName(),
		sMsg, Mo()->nChatDSBTimeOut * 2000,
		nPlayerID == 0 ? true : false );

	m_apSayDSB[nPlayerID] = pDSB;
}

// 해당 플레이어 자리에 공약 DSB 를 띄운다
// nPlayer : 플레이어 번호
// goal    : 그 플레이어의 공약
// goal.nMinScore 가 0 이면 출마 포기, -1 이면 딜미스
void CBoard::FloatGoalDSB( int nPlayer, const CGoal* pGoal )
{
	static const UINT arcBitmap[] = {
		IDB_NOTHING, IDB_SPADE, IDB_DIAMOND, IDB_HEART, IDB_CLOVER };

	if ( !m_pMFSM ) return;

	if ( nPlayer == -1 ) {	// 기존의 DSB 를 삭제
		for ( int i = 0; i < MAX_PLAYERS; i++ )
			if ( m_apGoalDSB[i] ) m_apGoalDSB[i]->Destroy();
		m_pMasterGoalDSB = 0;
		return;
	}

	// 새로운 공약이 나타나면 기존의 당선 DSB 를 타임아웃 DSB 로 만든다
	if ( pGoal->nMinScore != 0 && m_pMasterGoalDSB ) {
		m_pMasterGoalDSB->SetTimeout( DELAY_GOALDSBTIMEOUT );
		m_pMasterGoalDSB = 0;
	}

	const CState* pState = m_pMFSM ? m_pMFSM->GetState() : 0;
	CString sName = pState && nPlayer >= 0
			? (LPCTSTR)pState->apAllPlayers[nPlayer]->GetName()
			: _T("");

	DShortMessage* pDSB = new DShortMessage(m_pWrap);

	if ( pGoal->nMinScore < 0 ) {	// 딜미스

		PlaySound( IDW_CARDSETUP, true );
		pDSB->Create( 0, _T("딜 미스 !"),
			true, true, DELAY_DEALMISS, nPlayer, sName );
		if ( m_apGoalDSB[nPlayer] ) m_apGoalDSB[nPlayer]->Destroy();
		m_apGoalDSB[nPlayer] = pDSB;
	}
	else if ( pGoal->nMinScore == 0 ) {	// 출마 포기

		PlaySound( IDW_GIVEUP, true );
		pDSB->Create( IDB_STOP, _T(" 출마 포기"),
			true, true, DELAY_GIVEUPDSBTIMEOUT, nPlayer, sName );
		if ( m_apGoalDSB[nPlayer] ) m_apGoalDSB[nPlayer]->Destroy();
		m_apGoalDSB[nPlayer] = pDSB;
	}
	else {

		// 기본 최소점수
		const CState* pState = m_pMFSM ? m_pMFSM->GetState() : 0;
		int nDefaultMinScore = pState ? pState->pRule->nMinScore : 0;

		PlaySound( IDW_GOAL, true );
		pDSB->Create(
			arcBitmap[pGoal->nKiruda],
			CString(_T(" ")) + CCard::GetGoalString(
				Mo()->bUseTerm, pGoal->nKiruda,
				pGoal->nMinScore ),
			true, true, -1, nPlayer, sName );

		if ( m_apGoalDSB[nPlayer] ) m_apGoalDSB[nPlayer]->Destroy();
		m_apGoalDSB[nPlayer] = pDSB;
		m_pMasterGoalDSB = pDSB;
	}
}

// 지정한 DSB 를 선거 DSB 로 지정한다
// 한번에 하나의 선거 DSB 만 화면에 표시되므로
// 이전의 선거 DSB 는 제거된다
// pDSB 가 0 이면 이전의 선거 DSB 만 제거된다
void CBoard::SetElectionDSB( DSB* pDSB )
{
	if ( m_pCurrentElectionDSB )
		m_pCurrentElectionDSB->Destroy();
	ASSERT( !m_pCurrentElectionDSB );
	m_pCurrentElectionDSB = pDSB;
}
