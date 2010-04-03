// MFSM 에서 사용하는 편리한 매크로를 정의한다

// redo 플랙을 조사하여 참이면 그 플랙을 끄고 EXPR을 실행
#define CALL_IF_WANT_REDO( PLAYERID, EXPR ) \
	do { \
		if ( m_abWaitingPlayerRedo[PLAYERID] ) { \
			m_abWaitingPlayerRedo[PLAYERID] = false; \
			EXPR; \
		} \
	} while (0)

// 모든 플레이어에게 일괄적으로 Notify 하는 루프 블럭
// 이 내부에서 각 플레이어들에게 CALL_IF_WAIT_REDO 를 호출한다
#define NOTIFY_BLOCK_BEGIN() \
	ResetEvents(); \
	do {
#define NOTIFY_BLOCK_END() \
		Wait(); \
	} while ( m_nWaitingPlayer > 0 );

// 모든 플레이어에게 알린다 (FUNC 은 호출할 함수)
#define PLAYERID _nPlayerID
#define EVENT m_apePlayer[PLAYERID]
#define NOTIFY_ALL( FUNC ) \
	do { \
		NOTIFY_BLOCK_BEGIN() \
		for ( int _nPlayerID = pRule->nPlayerNum-1; _nPlayerID >= 0; _nPlayerID-- ) \
			CALL_IF_WANT_REDO( _nPlayerID, apAllPlayers[_nPlayerID]->FUNC ); \
		NOTIFY_BLOCK_END() \
	} while (0)

// 하나의 플레이어에게 알린다 (FUNC 은 호출할 함수)
#define NOTIFY_TO( PLAYERNUM, FUNC ) \
	do { \
		int _nPlayerID = GetPlayerIDFromNum(PLAYERNUM); \
		int _nCurPlayerID = GetPlayerIDFromNum(nCurrentPlayer); \
		do { \
			ResetEvents(); \
			CALL_IF_WANT_REDO( _nPlayerID, apAllPlayers[_nPlayerID]->FUNC ); \
			int _nNotifiedPlayerID; \
			do { \
				_nNotifiedPlayerID = Wait(); \
				ASSERT( _nNotifiedPlayerID == -1 \
					|| _nNotifiedPlayerID == _nCurPlayerID ); \
			} while ( !m_abWaitingPlayerRedo[_nPlayerID] \
					&& _nNotifiedPlayerID != _nCurPlayerID ); \
		} while ( m_abWaitingPlayerRedo[_nPlayerID] ); \
	} while (0)

// 하나의 플레이어에게 알린다 (FUNC 은 호출할 함수)
#define NOTIFY_TO_ID( PLAYERID, CURPLAYERID, FUNC ) \
	do { \
		int _nPlayerID = PLAYERID; \
		int _nCurPlayerID = CURPLAYERID; \
		do { \
			ResetEvents(); \
			CALL_IF_WANT_REDO( _nPlayerID, apAllPlayers[_nPlayerID]->FUNC ); \
			int _nNotifiedPlayerID; \
			do { \
				_nNotifiedPlayerID = Wait(); \
				ASSERT( _nNotifiedPlayerID == -1 \
					|| _nNotifiedPlayerID == _nCurPlayerID ); \
			} while ( !m_abWaitingPlayerRedo[_nPlayerID] \
					&& _nNotifiedPlayerID != _nCurPlayerID ); \
		} while ( m_abWaitingPlayerRedo[_nPlayerID] ); \
	} while (0)
