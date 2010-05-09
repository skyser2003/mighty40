// SocketBag.cpp: implementation of the CSocketBag class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "Play.h"
#include "Player.h"
#include "MFSM.h"

#include "ZSocket.h"
#include "PlayerSocket.h"

#include "SocketBag.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CSocketBag::CSocketBag()
{
	m_pMFSM = 0;
	m_pServerSocket = 0;
	m_nClients = 0;
}

CSocketBag::~CSocketBag()
{
	if ( m_pServerSocket ) {
		m_pServerSocket->ClearTrigger();
		delete m_pServerSocket;
	}
	for ( int i = 0; i < m_nClients; i++ ) {
		if ( m_aClients[i].pSocket ) {
			m_aClients[i].pSocket->ClearTrigger();
			delete m_aClients[i].pSocket;
		}
		POSITION pos = m_aClients[i].lmsg.GetHeadPosition();
		while (pos) delete m_aClients[i].lmsg.GetNext(pos);
		m_aClients[i].lmsg.RemoveAll();
	}
}

// 클라이언트용 초기화 (각 플레이어에 대해 반복해서 호출한후
// 두번째 버전을 부른다)
void CSocketBag::InitForClient( long uid )
{
	CLIENTITEM* pItem = &m_aClients[m_nClients++];
	ASSERT( m_nClients <= MAX_PLAYERS );

	pItem->uid = uid;
	pItem->pSocket = 0;
	pItem->ppMsg = 0;
	pItem->pEvent = 0;
}

void CSocketBag::InitForClient( CPlayerSocket* pServerSocket )
{
	ASSERT( pServerSocket );
	m_pServerSocket = pServerSocket;
	m_pServerSocket->SetTrigger( (DWORD)(LPVOID)this, (DWORD)-1, SockProc );
}

// 서버용 초기화 (각 플레이어에 대해 반복해서 호출한다)
void CSocketBag::InitForServer( long uid, CPlayerSocket* pSocket )
{
	CLIENTITEM* pItem = &m_aClients[m_nClients++];
	ASSERT( m_nClients <= MAX_PLAYERS );

	pItem->uid = uid;
	pItem->pSocket = pSocket;
	pItem->ppMsg = 0;
	pItem->pEvent = 0;

	if ( pSocket )
		pSocket->SetTrigger( (DWORD)(LPVOID)this, uid, SockProc );
}

// 해당 uid 에 대한 메시지를 Async 하게
// pMsg 에 세트하고 pEvent 를 세트함으로서 알린다
void CSocketBag::GetMsgFor( long uid, CMsg*& pMsg, CEvent* pEvent )
{
	m_cs.Lock();

	CLIENTITEM* pItem = FindUID( uid );

	ASSERT( pItem );
	ASSERT( !pItem->ppMsg && !pItem->pEvent );

	if ( !pItem->lmsg.IsEmpty() ) {	// 이미 메시지가 있다 !
		pMsg = pItem->lmsg.RemoveHead();
		pEvent->SetEvent();
	}
	else {	// 없다 - 나중에 알려준다
		pItem->ppMsg = &pMsg;
		pItem->pEvent = pEvent;
	}

	m_cs.Unlock();
}

// socket callback
void CSocketBag::SockProc( CPlayerSocket* pSocket, CMsg* pMsg,
				DWORD dwUser1, DWORD dwUser2 )
{	((CSocketBag*)(LPVOID)dwUser1)->SockProc( (long)dwUser2, pMsg, pSocket ); }


void CSocketBag::SockProc( long uid, CMsg* pMsg, CPlayerSocket* pSocket )
{
	AUTODELETE_MSG_EX(pMsg,adme);

	if ( pMsg->GetType() == CMsg::mmChat ) {
		// 채팅 메시지

		// MFSM 에게 이벤트로서 전달한다
		m_pMFSM->EventChat( *adme.Detach(), false );
		// 핸들러를 재설정
		pSocket->SetTrigger( (DWORD)(LPVOID)this, (DWORD)uid, SockProc );
		return;
	}
	else if ( pMsg->GetType() == CMsg::mmError
		|| pMsg->GetType() == CMsg::mmDisconnected ) {
		// 접속 종료 메시지

		long lDummy;
		CString sMsg;

		if ( pMsg->GetType() == CMsg::mmError
				&& pMsg->PumpLong( lDummy )
				&& pMsg->PumpString( sMsg ) )
			// 에러메시지가 수신된 경우
			m_pMFSM->EventExit( sMsg );

		else if ( uid == -1 )
			m_pMFSM->EventExit( _T("서버로부터 접속이 종료되었습니다") );

		else {
			CString sMsg;
			sMsg.Format( _T("%s 님이 접속을 종료하였습니다"),
				m_pMFSM->GetState()
				->apAllPlayers[m_pMFSM->GetPlayerIDFromUID(uid)]
				->GetName() );
			m_pMFSM->EventExit( sMsg );
		}
		return;
	}


	long ori_uid = uid;

	if ( uid == -1 ) {	// 서버 소켓 (클라이언트모드)
		// 게임 진행 메시지의 두 번째 원소는 항상 uid 이다
		pMsg->PumpLong( uid );
		pMsg->PumpLong( uid );
		ASSERT( uid >= 0 && uid < m_pMFSM->GetState()->pRule->nPlayerNum );
		pMsg->Rewind();
	}

	m_cs.Lock();

	CLIENTITEM* pItem = FindUID( uid );

	pItem->lmsg.AddTail( *adme.Detach() );

	if ( pItem->ppMsg ) {
		// 이벤트가 걸려있으므로 처리
		*pItem->ppMsg = pItem->lmsg.RemoveHead();
		pItem->pEvent->SetEvent();

		pItem->ppMsg = 0;
		pItem->pEvent = 0;
	}

	m_cs.Unlock();

	// 핸들러를 재설정
	pSocket->SetTrigger( (DWORD)(LPVOID)this, (DWORD)ori_uid, SockProc );
}
