// MFSM_Event.cpp: implementation of the CMFSM Events & Waits
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "Play.h"
#include "Player.h"
#include "MFSM.h"
#include "MFSM_Notify.h"
#include "SocketBag.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// 서버 쓰레드의 핸들을 Duplicate 한다 (Wait 하기 위해)
HANDLE CMFSM::DuplicateThreadHandle()
{
	if ( m_pServerThread ) {

		HANDLE hRet = 0;
		if ( ::DuplicateHandle(
			GetCurrentProcess(), m_pServerThread->m_hThread,
			GetCurrentProcess(), &hRet,
			0, FALSE, DUPLICATE_SAME_ACCESS ) ) return hRet;
	}
	ASSERT(0);
	return 0;
}


// 서비스를 시작한다
// 새로운 쓰레드가 곧바로 시작된다
// uid : 이 MFSM 의 uid ( 0 이면 이 MFSM 은 반드시 서버이다 )
void CMFSM::Init( long uid )
{
	m_uid = uid;

	InitStageData( 0 );

	InvokeAIThread();

	m_bTerminated = false;
	m_pServerThread = AfxBeginThread( ServiceProc, (LPVOID)this );

	ASSERT( m_pServerThread );
}

// 쓰레드 진입점
UINT CMFSM::ServiceProc( LPVOID _pThis )
{
	CMFSM* pThis = (CMFSM*)_pThis;

	// 쓰레드 변수인 random seed 를 초기화
	srand((long)time(0));

	UINT ret = pThis->Server();

	// 서비스 스레드가 종료되면 스스로 자신을 지운다
	delete pThis;
	return ret;
}

// m_eNotify 이벤트를 처리
void CMFSM::ProcessNotifyEvents()
{
	if ( m_bTerminate ) {
		// 종료
		// m_bTermRequest 는 참일수도, 아닐수도 있다
		// 즉, 아래 else if 문을 위로 올리면 무한루프

		// AI 쓰레드를 죽인다

		m_bAIThreadTerminate = true;
		m_eAIThread.SetEvent();

		while ( !m_bAIThreadTerminated ) Sleep(100);

		throw this;	// 종료 예외를 던진다 !!
		ASSERT(0);
	}
	else if ( m_bTermRequest ) {
		// 종료 요청
		for ( int i = pRule->nPlayerNum - 1; i >= 0; i-- )
			apAllPlayers[i]->OnTerminate( m_sTermRequestReason );

		m_bTerminate = true;
		m_eNotify.SetEvent();
		Wait();
	}
	else {
		// 채팅 메시지
		m_csChatMsg.Lock();
			bool bChat = !m_lChatMsg.IsEmpty();
		m_csChatMsg.Unlock();

		if ( bChat ) {
			ProcessChatMessage();
		}
		else {
			ASSERT(0);
		}
	}
}

// 채팅 메시지를 처리
void CMFSM::ProcessChatMessage()
{
	// 하나씩 메시지를 빼 와서 보낸다
	for(;;) {

		m_csChatMsg.Lock();
			BOOL bEmpty = m_lChatMsg.IsEmpty();
			CHATITEM ci = { 0, 0 };
			if ( !bEmpty ) ci = m_lChatMsg.RemoveHead();
		m_csChatMsg.Unlock();

		if ( bEmpty ) break;

		// 뽑아온 메시지 정보
		bool bSource = ci.bSource;
		CMsg* pMsg = ci.pMsg;
		long uid; CString sChat;

		AUTODELETE_MSG(pMsg);

		if ( !pMsg->PumpLong( uid )
			|| !pMsg->PumpLong( uid )
			|| !pMsg->PumpString( sChat ) )
			continue;	// 잘못된 메시지

		long nPlayerID = GetPlayerIDFromUID( uid );

		// 모두에게 뿌린다
		for ( int i = pRule->nPlayerNum - 1; i >= 0; i-- )
			apAllPlayers[i]->OnChat( nPlayerID, sChat, bSource );
	}
}

// 종료 메시지
void CMFSM::EventExit( LPCTSTR sMsg )
{
	m_sTermRequestReason = sMsg;
	m_bTermRequest = true;
	m_eNotify.SetEvent();
}

// 채팅 메시지 ( bSource 가 참이면, 채팅창에서 만들어진 메시지 )
void CMFSM::EventChat( CMsg* pMsg, bool bSource )
{
	CHATITEM ci;
	ci.pMsg = pMsg;
	ci.bSource = bSource;

	m_csChatMsg.Lock();
		m_lChatMsg.AddTail( ci );
	m_csChatMsg.Unlock();

	m_eNotify.SetEvent();
}

// 서버에서 덱을 얻어온다
// mmGameInit의 경우 앉는 자리도 얻어온다 (v4.0: 2011.1.14)
void CMFSM::GetDeckFromServer()
{
	int i;

	ASSERT( !IsServer() && m_pSockBag );

	CMsg* pMsg = 0;
	AUTODELETE_MSG(pMsg);

	// mmGameInit 메시지에 덱 정보가 있다
	m_pSockBag->GetMsgFor( 0, pMsg, &m_eSock );
	WaitEvent( &m_eSock );

	long nType;
	long uid;
	CCardList lServerDeck;

	if ( pMsg->PumpLong( nType )
			&& ( nType == CMsg::mmGameInit
				|| nType == CMsg::mmGameSuffledForDead )
			&& pMsg->PumpLong( uid )
			&& uid == 0
			&& pMsg->PumpCardList( lServerDeck )
			&& lDeck.GetCount() == lServerDeck.GetCount() )
	{
		lDeck = lServerDeck;
		if ( nType == CMsg::mmGameInit )
		{
			for ( i = 0; i < nPlayers; i++ )
			{
				if( !pMsg->PumpLong( uid ) ) goto hell;
				apPlayers[i] = apAllPlayers[uid];
			}
			return;
		}
		else return;
	}

hell:
	// 잘못된 메시지가 왔음
	ASSERT(0);
	EventExit( _T("서버로부터 예상치 못한 응답이 도착했습니다") );
}

// 오직 CPlayer 만 부르는 함수
// 이전에 시킨 작업이 완료되었음을 MFSM 에게 알린다
void CMFSM::Notify( int nPlayerID )
{
#ifdef _DEBUG
	ASSERT( m_apePlayer[nPlayerID] );
	for ( int i = 0; i < m_nWaitingPlayer; i++ )
		if ( m_apePlayer[nPlayerID] == m_apeWaitingPlayer[i] ) {
			m_apePlayer[nPlayerID]->SetEvent();
			return;
		}
	ASSERT(0);
#else
	m_apePlayer[nPlayerID]->SetEvent();
#endif
}

// 이벤트를 리셋하고, m_apeWaitingPlayer 등을 초기화 한다
void CMFSM::ResetEvents()
{
	m_nWaitingPlayer = 0;
	for ( int i = 0; i < MAX_CONNECTION; i++ ) {
		m_abWaitingPlayerRedo[i] = true;
		m_apeWaitingPlayer[i] = m_apePlayer[i];
		m_apePlayer[i]->ResetEvent();
	}
	m_apeWaitingPlayer[MAX_CONNECTION] = &m_eNotify;
	m_nWaitingPlayer = MAX_CONNECTION;
}

// 이벤트를 대기하며, 예외를 처리하는 함수
// 대기중인 플레이어의 이벤트를 감시, Set 되면 m_apeWaitingPlayer
// 에서 그 이벤트를 삭제한다
// 리턴값은 방금 event 가 세트된 플레이어의 PlayerID 또는 -1 (없음)
int CMFSM::Wait()
{
	// 먼저 m_eNotify 를 검사한다
lblCheckNotify:
	while ( m_eNotify.Lock(0) )
		ProcessNotifyEvents();

	// 나머지 이벤트와 m_eNotify 를 검사한다
	DWORD ret;
	{
		CMultiLock ml( (CSyncObject**)m_apeWaitingPlayer, m_nWaitingPlayer+1 );
		ret = ml.Lock( INFINITE, FALSE );
	}

	if ( ret == WAIT_OBJECT_0 + m_nWaitingPlayer ) { // m_eNotify
		ProcessNotifyEvents();
		goto lblCheckNotify;
	}

	else if ( WAIT_OBJECT_0 <= ret
		&& ret < WAIT_OBJECT_0 + m_nWaitingPlayer ) {
		// 어느 하나가 세트되었음

		int i;
		int nIndex = int( ret - WAIT_OBJECT_0 );
		CEvent* pEvent = m_apeWaitingPlayer[nIndex];

		// m_apeWaitingPlayer[] 에서 그 이벤트를 삭제
		for ( i = nIndex; i < m_nWaitingPlayer; i++ )
			m_apeWaitingPlayer[i] = m_apeWaitingPlayer[i+1];
		m_nWaitingPlayer--;

		// 어떤 플레이어의 이벤트인지 검사
		for ( i = 0; i < MAX_CONNECTION; i++ )
			if ( m_apePlayer[i] == pEvent )
				break;

		if ( i >= MAX_CONNECTION ) {
			ASSERT(0);
			return -1;
		}

		return i;
	}
	else {
		ASSERT(0);
		return -1;
	}
}

// 해당 이벤트를 Lock(=Wait)한다 - 종료 조건시 CMFSM 자신이
// throw 된다
void CMFSM::WaitEvent( CEvent* pEvent )
{
	// 먼저 m_eNotify 를 검사한다
	while ( m_eNotify.Lock(0) )
		ProcessNotifyEvents();

	// 나머지 이벤트와 m_eNotify 를 검사한다
	CSyncObject* apObj[2];
	apObj[0] = pEvent; apObj[1] = &m_eNotify;
	CMultiLock ml( apObj, 2 );

	DWORD ret;

	for(;;){
		ret = ml.Lock( INFINITE, FALSE );
		if ( ret == WAIT_OBJECT_0 + 1 ) { // m_eNotify
			ProcessNotifyEvents();
			continue;
		}
		else break;
	}
	ASSERT( ret == WAIT_OBJECT_0 );
}
