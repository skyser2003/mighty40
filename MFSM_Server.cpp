// MFSM_Server.cpp: implementation of the CMFSM Server Thread
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "Play.h"
#include "Player.h"
#include "MFSM.h"
#include "MFSM_Notify.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// 서버 쓰레드

UINT CMFSM::Server()
{
	TRACE( "Server Began\n" );
	int i;

	try {

		nBeginer = GetPlayerIDFromUID( 0 );

// 게임 시작
lblMightyBegin:

		// 현재 state 를 초기화 한다
		InitStageData( nGameNum + 1, nBeginer );

		// 클라이언트인 경우 서버에서 카드 배열을 얻어온다
		if ( !IsServer() ) GetDeckFromServer();

		// 각 플레이어의 초기화
		for ( i = 0; i < pRule->nPlayerNum; i++ ) {
			nCurrentPlayer = GetPlayerNumFromID(i);
			NOTIFY_TO( i, OnInit(EVENT) );
		}

		// 카드를 나눠 준다
		// 두장, 세장, 세장, 두장 의 순서
		{
			int nCardsPerOnePlayer = pRule->nPlayerNum == 6 ? 8 : 10;
			int nAllCards = nCardsPerOnePlayer * nPlayers;
			for ( i = 0; i < nAllCards; i++ ) {

				nCurrentPlayer = (nBeginer+i)%nPlayers;
				cCurrentCard = lDeck.RemoveTail();
				apPlayers[nCurrentPlayer]->GetHand()->AddTail( cCurrentCard );
				int nHandCount = apPlayers[nCurrentPlayer]
									->GetHand()->GetCount();

				// 두장, 세장, 세장, 두장 일 때 각 플레이어에게 알림
				bool bShow = nHandCount == 2 || nHandCount == 5
					|| nHandCount == 8 || nHandCount == 10;
				if ( bShow ) NOTIFY_ALL( OnDeal( -1, nCurrentPlayer, 1, cCurrentCard, EVENT ) );
			}
		}

		// 선거를 시작한다
		state = msElection;

		// 시작했음을 각 플레이어에게 알린다
		for ( i = 0; i < pRule->nPlayerNum; i++ ) {
			nCurrentPlayer = GetPlayerNumFromID(i);
			NOTIFY_TO( i, OnBegin( GetState(), EVENT ) );
		}

		nCurrentPlayer = nBeginer;

		for(;;) {

			NOTIFY_ALL( OnBeginThink( nCurrentPlayer, 0, EVENT ) );

			// 새로운 목표치를 얻는다
			CGoal goalNew = goal;
			NOTIFY_TO( nCurrentPlayer, OnElection( &goalNew, EVENT ) );

			// 제대로 된 목표치인지 검사한다
			if ( !IsValidNewGoal( goalNew.nKiruda, goalNew.nMinScore ) ) {
				goalNew.nMinScore = 0;	// 포기한 것으로 간주
				if ( !IsValidNewGoal( goalNew.nKiruda, goalNew.nMinScore ) ) {
					// 포기해서는 안되는 상황 (선이 포기 못하는 경우)
					goalNew.nKiruda = 0;
					goalNew.nMinScore = pRule->nMinScore;
					ASSERT( IsValidNewGoal( goalNew.nKiruda, goalNew.nMinScore ) );
				}
			}

			// 모두에게 알린다
			NOTIFY_ALL( OnElecting( nCurrentPlayer,
				goalNew.nKiruda, goalNew.nMinScore, EVENT ) );

			// 이 목표치로서 선거가 끝나는가?
			bool bEnd, bDealMiss;
			long nNextID;
			bEnd = CanBeEndOfElection( bDealMiss, nNextID, goalNew );

			if ( bDealMiss ) {		// 딜미스
				state = msEnd; goto lblCleanup;
			}
			else if ( goalNew.nMinScore == 0 )	// 포기
				abGiveup[nCurrentPlayer] = true;
			else goal = goalNew;				// 출마

			if ( bEnd ) {		// 이것으로 끝
				nMaster = nNextID;	// 당선자
				break;
			}
			else		// 선거를 계속함
				nCurrentPlayer = nNextID;
		}

		// 당선 확정
		// 주공 플레이가 시작된다
		state = msPrivilege;

		// 선(nBeginer), 현재플레이어(nCurrentPlayer) 모두
		// 주공으로 설정
		nCurrentPlayer = nBeginer = nMaster;

		NOTIFY_ALL( OnBeginThink( nMaster, 0, EVENT ) );

		// 일단 손에 든 카드를 재정렬하고 모든 플레이어에게 알린다
		if ( !Mo()->bNoSort )
		for ( i = 0; i < nPlayers; i++ )
			apPlayers[i]->GetHand()->Sort( Mo()->bLeftKiruda, Mo()->bLeftAce );
		NOTIFY_ALL( OnElected( nMaster, EVENT ) );

		// 6 마 라면 한 명의 플레이어를 죽이는 과정이 추가된다
		if ( pRule->nPlayerNum == 6 ) {

			CCard cKill(0);
			bool bKilled = false;
			do {

				cKill = CCard(0);
				NOTIFY_TO( nMaster,
					OnKillOneFromSix( &cKill, &lDead, EVENT ) );

				if ( cKill == CCard(0) ) {	// 잘못된 카드 값
					ASSERT(0); cKill = CCard(SPADE,ACE);
				}

				bKilled = KillTest( nMaster, &cKill, &nDeadID );
				if ( !bKilled ) lDead.AddTail( cKill );

				NOTIFY_ALL( OnKillOneFromSix( cKill, bKilled, EVENT ) );

			} while ( !bKilled );

			// 죽은 플레이어의 카드를 나눠 가진다

			CCardList* plcDead = apAllPlayers[nDeadID]->GetHand();
			int nCards = plcDead->GetCount(); ASSERT( nCards == 8 );

			// 회수단계
			for ( i = 0; i < nCards; i++ ) {

				nCurrentPlayer = (nBeginer+i)%nPlayers;
				cCurrentCard = plcDead->RemoveTail();
				lDeck.AddTail( cCurrentCard );
				NOTIFY_ALL( OnDeal( nDeadID, -1, 3, cCurrentCard, EVENT ) );
			}

			RebuildPlayerArray();

			// 가운데 모은 카드를 섞고 실제로 하나의 플레이어를 죽인다
			lDeck.Suffle();
			// 클라이언트인 경우 서버에서 다시 섞인 카드 배열을 얻어온다
			if ( !IsServer() ) GetDeckFromServer();
			// 각 플레이어에게 알린다
			NOTIFY_ALL( OnSuffledForDead(EVENT) );

			// 모두에게 알린다
			NOTIFY_ALL( OnDeal( -1, -1, 0, 0, EVENT ) );

			nCards = 10;	// 이제 10 장을 나눠 준다
			for ( i = 0; i < nCards; i++ ) {

				nCurrentPlayer = (nBeginer+i)%nPlayers;
				cCurrentCard = lDeck.RemoveTail();
				CCardList* pHand = apPlayers[nCurrentPlayer]->GetHand();
				pHand->AddTail( cCurrentCard );
				if ( !Mo()->bNoSort )
					pHand->Sort( Mo()->bLeftKiruda, Mo()->bLeftAce );

				NOTIFY_ALL( OnDeal( -1, nCurrentPlayer, 9, cCurrentCard, EVENT ) );
			}
		}	// if ( pRule->nPlayerNum == 6 ) 의 끝

		// 주공은 나머지 카드를 가진다
		{
			nCurrentPlayer = nMaster;
			CCardList* pHand = apPlayers[nMaster]->GetHand();

			int nCards = lDeck.GetCount();
			for ( i = 0; i < nCards; i++ ) {

				cCurrentCard = lDeck.RemoveTail();
				pHand->AddTail( cCurrentCard );
				if ( !Mo()->bNoSort )
					pHand->Sort( Mo()->bLeftKiruda, Mo()->bLeftAce );

				NOTIFY_ALL( OnDeal( -1, nCurrentPlayer, 10, cCurrentCard, EVENT ) );
			}
		}

		// 주공에게 특권 상태를 처리하도록 한다
		{
			NOTIFY_ALL( OnBeginThink( nMaster, 1, EVENT ) );

			CGoal goalNew = goal;
			CCard ac[3];
			NOTIFY_TO( nMaster, OnElected( &goalNew, ac, EVENT ) );
			NOTIFY_ALL( OnPrivilegeEnd( &goalNew, ac, EVENT ) );

			// 결과를 반영한다

			// 석장의 카드를 버린다
			if ( apPlayers[nMaster]->GetHand()->GetCount() > 10 ) {
				CCardList* pHand = apPlayers[nMaster]->GetHand();
				POSITION pos = pHand->GetTailPosition();
				for ( i = 0; i < 3; i++ ) {
					if ( !pHand->Find(ac[i]) ) { // 잘못된 카드
						ASSERT(0);
						bool bDup;
						do {
							ac[i] = pHand->GetPrev(pos);	// 아무 카드나 빼 버린다!!
							bDup = false;
							for ( int j = 0; j < i; j++ )
								if ( ac[j] == ac[i] ) bDup = true;
						} while (bDup);
					}
				}
				RemoveDroppedCards( ac, goalNew.nKiruda );
				ASSERT( pHand->GetCount() == 10
					&& lDeck.GetCount() == 3 );
			}

			// 목표 점수, 프랜드

			if ( !IsValidNewGoal( goalNew.nKiruda, goalNew.nMinScore ) ) {
				ASSERT(0);
				goalNew.nKiruda = goal.nKiruda;
				goalNew.nMinScore = goal.nMinScore;
			}

			goalNew.nFriend = GetValidFriend( goalNew.nFriend );
			goal = goalNew;
			if ( goal.nFriend == 0 ) { nFriend = -1; bFriendRevealed = true; }
			else if ( goal.nFriend == 100 ) nFriend = -2;
			else if ( goal.nFriend < 0 ) { nFriend = -goal.nFriend-1; bFriendRevealed = true; }
			else {
				CCard c(goal.nFriend);
				for ( i = 0; i < nPlayers; i++ )
					if ( apPlayers[i]->GetHand()->Find(c) ) {
						nFriend = i; break;
					}
				if ( i >= nPlayers ) { nFriend = -1; ASSERT(0); bFriendRevealed = true; }
			}
		}

		// 본격적인 게임의 시작
		state = msTurn;

		// 이를 알린다
		for ( i = 0; i < pRule->nPlayerNum; i++ ) {
			nCurrentPlayer = GetPlayerNumFromID(i);
			NOTIFY_TO_ID( i, i, OnElectionEnd( EVENT ) );
		}

		// 가장 중요한 게임 루프 !
		// 10 번 반복함
		for ( nTurn = 0; nTurn <= LAST_TURN; nTurn++ ) {

			nJokerShape = 0;
			bJokercallEffect = false;

			// 각 플레이어마다 반복
			for ( int cnt = 0; cnt < nPlayers; cnt++ ) {

				nCurrentPlayer = ( nBeginer + cnt ) % nPlayers;
				cCurrentCard = 0;

				// 카드를 낸다
				CCard c(0);
				int eff = -1;

				NOTIFY_ALL( OnBeginThink( nCurrentPlayer, 2, EVENT ) );

				NOTIFY_TO( nCurrentPlayer, OnTurn( &c, &eff, EVENT ) );

				CCardList* pCurHand = apPlayers[nCurrentPlayer]->GetHand();

				// 제대로가 아니면 다른 아무 카드나 내게 한다 !
				CCardList clAv;
				pCurHand->GetAvList( &clAv, &lCurrent, nTurn,
										nJokerShape, bJokercallEffect );
				if ( !clAv.Find( c ) ) { c = clAv.GetHead(); eff = 0; }

				cCurrentCard = (int)c;

				// 조커/조커콜에 관련된 변수를 설정한다
				SetJokerState( c, eff );
				// 낸 카드 플랙을 세트한다
				if ( !c.IsJoker() ) anUsingCards[c.GetShape()-SPADE]
					|= (1<<(c.GetPowerNum()-1));

				// 실제로 카드를 낸다
				POSITION pos = pCurHand->Find(c); ASSERT(pos);
				int nHandIndex = pCurHand->IndexFromPOSITION(pos);
				pCurHand->RemoveAt( pos );
				lCurrent.AddTail( c );

				bool bFriendIsJustRevealed = false;
				if ( 0 < goal.nFriend && goal.nFriend < 54
						&& c == CCard(goal.nFriend) ) {
					// 프랜드가 밝혀짐, 프랜드 득점을 모두 없앤다
					ASSERT( nFriend >= 0 );
					bFriendIsJustRevealed = bFriendRevealed = true;
					lDeck.AddTail( apPlayers[nFriend]->GetScore() );
					apPlayers[nFriend]->GetScore()->RemoveAll();
				}

				// 모두에게 알린다
				NOTIFY_ALL( OnTurn( c, eff, nHandIndex, EVENT ) );

				if ( bFriendIsJustRevealed )
					NOTIFY_ALL( OnFriendIsRevealed( nFriend, EVENT ) );
			}

			// 한 판의 종료

			// 사용된 카드 플랙을 세트
			for ( i = 0; i < 4; i++ ) {
				anUsedCards[i] |= anUsingCards[i];
				anUsingCards[i] = 0;
			}

			// 승자를 조사한다
			int nWinner = GetWinner( &lCurrent );

			nCurrentPlayer = nWinner;	// 현재 플레이어는 승자임
			cCurrentCard = 0;
			CCardList* pWinnerScore = apPlayers[nWinner]->GetScore();

			// 초구 프랜드의 경우 이 승자가 프랜드가 된다
			if ( nFriend == -2 ) {							ASSERT( nTurn == 0 );
				bFriendRevealed = true;
				if ( nWinner == nMaster ) nFriend = -1;	// 노프랜드
				else nFriend = nWinner;
				NOTIFY_ALL( OnFriendIsRevealed( nFriend, EVENT ) );
			}

			// 결과를 히스토리에 기록
			WriteHistory();

			// 모두에게 알린다
			NOTIFY_ALL( OnTurnEnd( EVENT ) );

			// 득점 카드를 계산, 처리하고 승자를 다음 선으로 한다
			while (!lCurrent.IsEmpty()) {

				CCard c( cCurrentCard = (int)lCurrent.RemoveTail() );

				if ( c.IsPoint() && GetState()->nMaster != nWinner
					&& ( GetState()->nFriend != nWinner
						|| !GetState()->bFriendRevealed ) )
					pWinnerScore->AddTail(c);
				else lDeck.AddTail(c);

				NOTIFY_ALL( OnTurnEnding( nWinner, EVENT ) );
			}
			lCurrent.RemoveAll();
			nBeginer = nWinner;
		}

		// 게임 끝
		state = msEnd;
		{
			// 종료 여부를 기록하는 플랙
			bool abCont[MAX_PLAYERS];
			for ( i = 0; i < pRule->nPlayerNum; i++ ) abCont[i] = true;

			// 점수와 전적을 Update 한다
			{
				bool bDefWin; CString as[2];	// dummy var
				int anMoney[MAX_PLAYERS];
				for ( i = 0; i < nPlayers; i++ )
					anMoney[i] = apPlayers[i]->GetMoney();

				GetReport( bDefWin, 0, 0, as, 0, anMoney );

				// 전적을 Update
				for ( i = 0; i < nPlayers; i++ ) {
					GAME_RECORD& grCur = apPlayers[i]->GetCurRecord();
					GAME_RECORD& grAll = apPlayers[i]->GetAllRecord();

					if ( bDefWin ) {
						if ( i == nMaster ) grCur.wm++, grAll.wm++;
						else if ( i == nFriend ) grCur.wf++, grAll.wf++;
						else grCur.la++, grAll.la++;
					} else {
						if ( i == nMaster ) grCur.lm++, grAll.lm++;
						else if ( i == nFriend ) grCur.lf++, grAll.lf++;
						else grCur.wa++, grAll.wa++;
					}
				}

				for ( i = 0; i < nPlayers; i++ )
					apPlayers[i]->SetMoney( anMoney[i] );

				NOTIFY_ALL( OnEnd( &abCont[PLAYERID], EVENT ) );
			}

			// 종료를 하게 되었는지 검사한다
			bool bCont = true;
			for ( i = 0; i < pRule->nPlayerNum; i++ )
				if ( !abCont[i] ) { bCont = false; break; }

			if ( !bCont )  { // 종료
				for ( i = pRule->nPlayerNum - 1; i >= 0; i-- )
					apAllPlayers[i]->OnTerminate( m_sTermRequestReason );
				throw this;
			}

			// nBeginer 에 다음 선을 세트한다
			// (다음 선은 죽은 플레이어까지 포함되므로
			//  nBeginer 는 PlayerID 로 세트되어야 한다)
			if ( pRule->bFriendGetsBeginer && nFriend >= 0 )
				nBeginer = GetPlayerIDFromNum(nFriend);
			else nBeginer = GetPlayerIDFromNum(nMaster);
		}

lblCleanup:
		goto lblMightyBegin;

	}
	catch ( CMFSM* ) {
		TRACE("caught terminating exception\n");
	}

	m_bTerminated = true;
	TRACE( "Server Ended\n" );
	return 0;
}
