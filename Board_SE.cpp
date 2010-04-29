// Board_SE.cpp : Special Effecting implementation for CBoard
//

#include "stdafx.h"
#include "Mighty.h"
#include "BmpMan.h"
#include "Play.h"
#include "Player.h"
#include "MFSM.h"
#include "Board.h"
#include "BoardWrap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// 이 함수를 호출하면 마우스 클릭, 또는 채팅 창으로
// 사용자가 카드를 선택할 때 까지 대기했다가
// 지정한 이벤트를 세트한다
void CBoard::WaitSelection( CEvent* pEvent, long* pSelectedCard )
{
	ASSERT( !m_pWaitingSelectionEvent );

	m_pWaitingSelectionEvent = pEvent;
	m_pWaitingSelectionCard = pSelectedCard;

	// 커서, 카드이동 효과를 보여준다

	// 현재 좌표를 얻는다
	POINT point;
	GetCursorPos( &point );
	ScreenToClient( &point );

	// 카드 이동
	if ( CanSelect() ) {
		ShiftCard( point.x, point.y, true );
		SetCursor( m_hHand );
	}
}

// 위의 상태를 취소한다
void CBoard::CancelSelection()
{
	ASSERT( m_pWaitingSelectionEvent );
	m_pWaitingSelectionEvent = 0;
	m_pWaitingSelectionCard = 0;

	SetCursor( m_hArrow );

	// 다시 올려진 카드를 원상태로
	m_nMouseOver = -1;
	// ShiftCard( (int)0x80000000, (int)0x80000000, false );
}

// 차례 사각형을 해당 플레이어 위치로 이동 (-1 은 없앰)
void CBoard::SetTurnRect( int nPlayer )
{
	if ( !m_pMFSM || !m_pMFSM->GetState() ) {
		m_nTurnRect = nPlayer;
		return;
	}

	int nTurnRect = m_nTurnRect;
	m_nTurnRect = nPlayer;

	if ( nTurnRect != -1	// 기존 사각형을 삭제
		&& nTurnRect < m_pMFSM->GetState()->nPlayers
		&& nTurnRect != m_nTurnRect ) {

		CRect rc;
		{
			CClientDC dc(this);
			rc = CalcNameRect( m_pMFSM->GetState()->nPlayers, nTurnRect,
					&dc, m_pMFSM->GetState()->apPlayers[nTurnRect]->GetName(),
					&m_fntSmall );
		}
		rc.InflateRect( 3, 3 );
		UpdatePlayer( nTurnRect, &rc );
	}
	if ( m_nTurnRect != -1
		&& m_nTurnRect < m_pMFSM->GetState()->nPlayers ) {

		CRect rc;
		{
			CClientDC dc(this);
			rc = CalcNameRect( m_pMFSM->GetState()->nPlayers, m_nTurnRect,
					&dc, m_pMFSM->GetState()->apPlayers[m_nTurnRect]->GetName(),
					&m_fntSmall );
		}
		rc.InflateRect( 3, 3 );
		UpdatePlayer( m_nTurnRect, &rc );
	}
}

// 지정한 시간동안 가만히 있다가 지정한 이벤트를 세트한다
void CBoard::WaitWhile( int nMillisecond, CEvent* pEvent )
{
	// Flying 큐에 추가
	AddFlyingQueue( new CMsg( _T("lll"), opWaitWhile,
		(long)nMillisecond, (long)(LPVOID)pEvent ) );
}

// 카드를 날리는 에니메이션 후
// 지정한 이벤트를 세트한다
// (그림자 효과는 중앙을 낮은 지점으로 생각)
void CBoard::FlyCard( int nCard, int nSpeed,
	int nFrom, int nFromPlayer, int nFromIndex,
	int nTo, int nToPlayer, int nToIndex, CEvent* pEvent )
{
	// Flying 큐에 추가
	AddFlyingQueue( new CMsg( _T("llllllllll"), opFlyCard,
		(long)nCard, (long)nSpeed,
		(long)nFrom, (long)nFromPlayer, (long)nFromIndex,
		(long)nTo, (long)nToPlayer, (long)nToIndex,
		(long)(LPVOID)pEvent ) );
}

// 카드 섞는 에니메이션 후
// 지정한 이벤트를 세트한다
// 음향은 IDW_SUFFLE 을 연주한다
void CBoard::SuffleCards( int nCount /*섞는 회수*/, CEvent* pEvent )
{
	// Flying 큐에 추가
	AddFlyingQueue( new CMsg( _T("lll"), opSuffleCards,
		(long)nCount, (long)(LPVOID)pEvent ) );
}

// 카드 날리기/섞기 쓰레드 진입 함수
UINT CBoard::FlyProc( LPVOID _pThis )
{
	TRACE("FlyProc Began\n");

	// 정밀 타이머를 가장 조밀한 단계로 세트한다
	bool bPeriodBegan = false;
	TIMECAPS tc;
	if ( ::timeGetDevCaps( &tc, sizeof(tc) ) == TIMERR_NOERROR
			&& ::timeBeginPeriod( tc.wPeriodMin ) == TIMERR_NOERROR )
		bPeriodBegan = true;


	CBoard* pThis = (CBoard*)_pThis;

	while ( !pThis->m_bFlyProcTerminate ) {

		// 이벤트를 무한 대기
		VERIFY( pThis->m_eFlyIt.Lock() );

		// 큐를 검사하여 새로운 작업을 시작한다

		CMsg* pMsg = 0;
		AUTODELETE_MSG(pMsg);

		pThis->m_csFlying.Lock();
			if ( pThis->m_lFlyingQueue.GetCount() > 0 )
				pMsg = pThis->m_lFlyingQueue.RemoveHead();
		pThis->m_csFlying.Unlock();

		if ( !pMsg ) continue;	// 아무 일도 없다

		long nOp; VERIFY( pMsg->PumpLong(nOp) );
		switch ( (OPERATION)nOp ) {

		case opWaitWhile:	// 대기
		{
			// 인자를 얻는다
			long nMilliseconds, dummy;
			CEvent* pEvent;

			VERIFY( pMsg->PumpLong(nMilliseconds) );
			VERIFY( pMsg->PumpLong(dummy) );
			pEvent = (CEvent*)(LPVOID)dummy;

			// 그 시간만큼 진짜로 대기한다
			Sleep( nMilliseconds );

			pEvent->SetEvent();

		} break;

		case opFlyCard:		// 카드 날리기
		{
			// 인자를 얻는다
			long nCard, nSpeed, nFrom, nFromPlayer, nFromIndex,
				nTo, nToPlayer, nToIndex, dummy;
			CEvent* pEvent;

			VERIFY( pMsg->PumpLong(nCard) );
			VERIFY( pMsg->PumpLong(nSpeed) );
			VERIFY( pMsg->PumpLong(nFrom) );
			VERIFY( pMsg->PumpLong(nFromPlayer) );
			VERIFY( pMsg->PumpLong(nFromIndex) );
			VERIFY( pMsg->PumpLong(nTo) );
			VERIFY( pMsg->PumpLong(nToPlayer) );
			VERIFY( pMsg->PumpLong(nToIndex) );
			VERIFY( pMsg->PumpLong(dummy) );
			pEvent = (CEvent*)(LPVOID)dummy;

			if ( nSpeed == 1 || Mo()->bSlowMachine )
				pThis->DoFlyCard2( nCard, nSpeed, nFrom, nFromPlayer, nFromIndex,
					nTo, nToPlayer, nToIndex );
			else pThis->DoFlyCard( nCard, nSpeed, nFrom, nFromPlayer, nFromIndex,
					nTo, nToPlayer, nToIndex );

			pEvent->SetEvent();

		} break;

		case opSuffleCards:	// 카드 섞기
		{
			// 인자를 얻는다
			long nCount, dummy;
			CEvent* pEvent;

			VERIFY( pMsg->PumpLong(nCount) );
			VERIFY( pMsg->PumpLong(dummy) );
			pEvent = (CEvent*)(LPVOID)dummy;

			pThis->DoSuffleCards( nCount );

			pEvent->SetEvent();

		} break;

		default:			// 그 외에는 에러
			ASSERT(0);
			break;
		}
	}

	// 타이머를 원위치
	if ( bPeriodBegan )
		VERIFY( ::timeEndPeriod( tc.wPeriodMin ) == TIMERR_NOERROR );

	TRACE("FlyProc Ended\n");
	pThis->m_bFlyProcTerminated = true;
	return 0;
}


// 실제 카드를 섞는다 ( FlyProc 에서만 호출됨 )
void CBoard::DoSuffleCards( int nCount )
{
	ASSERT( !m_hSuffling );

	// 한 단계마다 걸리는 시간
	int nInterval = DELAY_SUFFLESTEP;

	// 카드 섞기 시작
	SuffleCards( m_hSuffling, nCount, m_szCard );

	while ( !m_bFlyProcTerminate && m_hSuffling ) {
		SuffleCards( m_hSuffling );
		Sleep( nInterval );
	}
}

// 실제 카드를 날린다 ( FlyProc 에서만 호출됨 )

// 저사양 버전 (또는 Speed == 1 (최고속도)일때)
void CBoard::DoFlyCard2(
	int nCard,			// 날릴 카드
	int nSpeed,			// 속도 ( 0 : 표준 1 : 매우 빠름 2 : 빠름 )
	int nFrom,			// 시작 위치 ( 0 가운데 1 점수영역 2 손에든 카드 3 따는곳 4 덱 꼭대기!! )
	int nFromPlayer,	// 시작 위치 ( 플레이어 번호, nFrom!=0일때만 사용  )
	int nFromIndex,		// 몇 번째 카드인가 (-1이면 정 가운데)
	int nTo,			// 끝 위치
	int nToPlayer,
	int nToIndex )
{
	ASSERT( !m_hFlying );

	// 시작에서 종료할 때 까지의 카드 윤곽
	CRect rcBegin = CalcCardRectForFlyCard(
						nFrom, nFromPlayer, nFromIndex );
	CRect rcEnd = CalcCardRectForFlyCard(
						nTo, nToPlayer, nToIndex );

	// 음영 방법 ( 가운데가 낮다 )
	int nShadeMode = ( nTo == 0 || nTo == 1 || nTo == 4 ? 0 : 2 )
		| ( nFrom == 0 || nFrom == 1 || nFrom == 4 ? 0 : 1 );

	// 프레임간 간격
	int nFlyingInterval = 0;

	// 총 프레임 수
	int nStates = 2;

	// 날리기 시작
	FlyCard( m_hFlying, nCard, rcBegin, rcEnd.TopLeft(), nStates, nShadeMode );

	while ( !m_bFlyProcTerminate && m_hFlying ) {
		Sleep( nFlyingInterval );
		FlyCard( m_hFlying );
	}

	nSpeed;	//unused
}

void CBoard::DoFlyCard(
	int nCard,			// 날릴 카드
	int nSpeed,			// 속도 ( 0 : 표준 1 : 매우 빠름 2 : 빠름 )
	int nFrom,			// 시작 위치 ( 0 가운데 1 점수영역 2 손에든 카드 3 따는곳 4 덱 꼭대기!! )
	int nFromPlayer,	// 시작 위치 ( 플레이어 번호, nFrom!=0일때만 사용  )
	int nFromIndex,		// 몇 번째 카드인가 (-1이면 정 가운데)
	int nTo,			// 끝 위치
	int nToPlayer,
	int nToIndex )
{
	ASSERT( !m_hFlying );
	ASSERT( nSpeed == 0 || nSpeed == 2 );

	// 시작에서 종료할 때 까지의 카드 윤곽
	CRect rcBegin = CalcCardRectForFlyCard(
						nFrom, nFromPlayer, nFromIndex );
	CRect rcEnd = CalcCardRectForFlyCard(
						nTo, nToPlayer, nToIndex );

	// 음영 방법 ( 가운데가 낮다 )
	int nShadeMode = ( nTo == 0 || nTo == 1 || nTo == 4 ? 0 : 2 )
		| ( nFrom == 0 || nFrom == 1 || nFrom == 4 ? 0 : 1 );

	// 전체 날리는 시간 (ms)
	long tTotalTime = 20 + (10-Mo()->nCardSpeed) * 100;
	if ( nSpeed == 2 ) tTotalTime = tTotalTime * 2/5;	// 40%

	// 총 프레임 수
	int nStates;
	if ( nSpeed == 2 ) nStates = 2 + Mo()->nCardMotion*2;
	else nStates = 5 + Mo()->nCardMotion*5;

	// 현재 프레임
	int nState = 0;
	// 완충버퍼
	long tBuffer = 5;	// 5 millisec
	// 시작시간
	long tBegin = 0;
	// 현재 시각이어야 하는 값
	long tCurrentLogical = 0;
	// 진짜 현재 시각
	long tCurrentReal = 0;
	// 차이
	long tDiff = 0;
	// 빼 먹을 프레임 수
	int nFramesToSkip = 0;

	// 날리기 시작
	FlyCard( m_hFlying, nCard, rcBegin, rcEnd.TopLeft(), nStates, nShadeMode );

	while ( !m_bFlyProcTerminate && m_hFlying ) {

		if ( nState == 0 ) {
			// 첫 프레임은 그냥 그린다
			FlyCard( m_hFlying );
			nState++;
			tBegin = (long)::timeGetTime();
			continue;
		}

		tCurrentLogical = tBegin
			+ tTotalTime * ( nState - 1 ) / ( nStates - 1 );
		tCurrentReal = (long)::timeGetTime();
		tDiff = tCurrentLogical - tCurrentReal;

		if ( tDiff >= tBuffer ) {	// 너무 빠르다 !
			Sleep( 0 );
			continue;
		}
		else if ( -tBuffer < tDiff ) {	// 잘 하고 있다
			nFramesToSkip = 0;
		}
		else {	// 너무 느리다 (스킵)
			nFramesToSkip = -tDiff * nStates / tTotalTime;
		}

		FlyCard( m_hFlying, nFramesToSkip + 1 );

		nState += ( nFramesToSkip + 1 );
		Sleep(0);
	}
}
