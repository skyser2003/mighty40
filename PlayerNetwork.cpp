// PlayerNetwork.cpp: implementation of the CPlayerNetwork class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "mighty.h"

#include "MFSM.h"
#include "ZSocket.h"
#include "PlayerSocket.h"
#include "SocketBag.h"

#include "Play.h"
#include "Player.h"
#include "PlayerNetwork.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CPlayerNetwork::CPlayerNetwork(
		int nID,		// 고유 번호 ( 6명의 플레이어중 순서 )
		LPCTSTR sName,	// 이름
		CWnd* pCallback )// call-back window
	: CPlayer( nID, sName, pCallback )
{
}

CPlayerNetwork::~CPlayerNetwork()
{
}


// 에러 발생 - MFSM 에게 알린다
// 에러코드 헤더 참조
void CPlayerNetwork::Error( int nErr )
{
	CString sMsg;

	if ( IsServer() ) {

		if ( nErr == 1 ) // 접속 종료
			sMsg = GetName() + _T(" 님의 접속이 끊어졌습니다");
		else	// 그 외
			sMsg = GetName() + _T(" 님에게 이상이 발생했습니다");
	}
	else {

		if ( nErr == 1 ) // 접속 종료
			sMsg = _T("서버로부터 접속이 종료되었습니다");
		else sMsg = _T("서버에 이상이 발생했습니다");
	}

	m_pMFSM->EventExit( sMsg );
}

void CPlayerNetwork::SendMsg( CMsg* pMsg )
{
	if ( !GetSocket()->SendMsg( pMsg ) ) Error( 2 );
}

void CPlayerNetwork::OnInit( CEvent* e )
{
	// 서버라면, 현재 덱을 전달해야 한다
	if ( IsServer() ) {
		CMsg msg( _T("llC"), CMsg::mmGameInit, 0,
								&m_pMFSM->GetState()->lDeck );
		SendMsg( &msg );
	}
	e->SetEvent();
}

void CPlayerNetwork::OnElection( CGoal* pNewGoal, CEvent* e )
{
	// mmGameElection 메시지를 받아서 준다
	CMsg* pMsg = 0;
	AUTODELETE_MSG(pMsg);

	GetSB()->GetMsgFor( GetUID(), pMsg, &m_e );
	m_pMFSM->WaitEvent( &m_e );

	long t, u, k, m, f;
	if ( !pMsg->PumpLong( t ) || t != CMsg::mmGameElection
					|| !pMsg->PumpLong( u ) || u != GetUID()
					|| !pMsg->PumpLong( k ) || !pMsg->PumpLong( m ) || !pMsg->PumpLong( f ) )
		Error(3);

	else {
		pNewGoal->nKiruda = k;
		pNewGoal->nMinScore = m;
		pNewGoal->nFriend = f;
	}

	e->SetEvent();
}

void CPlayerNetwork::OnElecting( int nPlayerID, int nKiruda,
	int nMinScore, CEvent* e )
{
	// mmGameElection 메시지를 전달한다
	if ( NeedSendingIfIDIs( nPlayerID ) ) {

		CMsg msg( _T("lllll"), CMsg::mmGameElection,
			GetPlayerUIDFromID( nPlayerID ), (long)nKiruda,
			(long)nMinScore, (long)0 );
		SendMsg( &msg );
	}
	e->SetEvent();
}

void CPlayerNetwork::OnKillOneFromSix( CCard* pcCardToKill,
	CCardList* plcFailedCardsTillNow, CEvent* e )
{
	// mmGameKillOneFromSix 메시지를 받아서 준다
	CMsg* pMsg = 0;
	AUTODELETE_MSG(pMsg);

	GetSB()->GetMsgFor( GetUID(), pMsg, &m_e );
	m_pMFSM->WaitEvent( &m_e );

	long t, u, c;
	if ( !pMsg->PumpLong( t ) || t != CMsg::mmGameKillOneFromSix
					|| !pMsg->PumpLong( u ) || u != GetUID()
					|| !pMsg->PumpLong( c ) )
		Error(3);

	else {
		*pcCardToKill = (int)c;
		plcFailedCardsTillNow;	// unused
	}
	e->SetEvent();
}

void CPlayerNetwork::OnKillOneFromSix( CCard cKill,
	bool bKilled, CEvent* e )
{
	// mmGameKillOneFromSix 메시지를 전달한다
	if ( NeedSendingIfNumIs( m_pMFSM->GetState()->nMaster ) ) {

		CMsg msg( _T("lllll"), CMsg::mmGameKillOneFromSix,
			GetPlayerUIDFromNum( m_pMFSM->GetState()->nMaster ),
			(long)(int)cKill );
		SendMsg( &msg );
	}
	e->SetEvent();
}

// 플레이어를 죽인 후 카드를 섞었다
void CPlayerNetwork::OnSuffledForDead( CEvent* e )
{
	// mmGameSuffledForDead 메시지를 전달한다
	// 서버라면, 현재 덱을 전달해야 한다
	if ( IsServer() ) {
		CMsg msg( _T("llC"), CMsg::mmGameSuffledForDead, 0,
								&m_pMFSM->GetState()->lDeck );
		SendMsg( &msg );
	}
	e->SetEvent();
}

void CPlayerNetwork::OnElected( CGoal* pNewGoal, CCard acDrop[3], CEvent* e )
{
	// mmGamePrivilege 메시지를 받아서 준다
	CMsg* pMsg = 0;
	AUTODELETE_MSG(pMsg);

	GetSB()->GetMsgFor( GetUID(), pMsg, &m_e );
	m_pMFSM->WaitEvent( &m_e );

	long t, u, k, m, f, d0, d1, d2;
	if ( !pMsg->PumpLong( t ) || t != CMsg::mmGamePrivilege
				|| !pMsg->PumpLong( u ) || u != GetUID()
				|| !pMsg->PumpLong( k ) || !pMsg->PumpLong( m )
				|| !pMsg->PumpLong( f ) || !pMsg->PumpLong( d0 )
				|| !pMsg->PumpLong( d1 ) || !pMsg->PumpLong( d2 ) )
		Error(3);

	else {
		// 프랜드(f)는 인코드 되어 있다
		if ( f < 0 ) f = - GetPlayerNumFromUID(-f-1) - 1;

		pNewGoal->nKiruda = k;
		pNewGoal->nMinScore = m;
		pNewGoal->nFriend = f;
		acDrop[0] = d0;
		acDrop[1] = d1;
		acDrop[2] = d2;
	}
	e->SetEvent();
}

void CPlayerNetwork::OnPrivilegeEnd( CGoal* pNewGoal, CCard acDrop[3], CEvent* e )
{
	// mmGamePrivilege 메시지를 전달한다
	if ( NeedSendingIfNumIs( m_pMFSM->GetState()->nMaster ) ) {

		// 프랜드(f) 를 인코드 한다 ( num -> uid )
		int f;
		if ( pNewGoal->nFriend < 0 )
			f = - GetPlayerUIDFromNum( -pNewGoal->nFriend-1 ) - 1;
		else f = pNewGoal->nFriend;

		CMsg msg( _T("llllllll"), CMsg::mmGamePrivilege,
			GetPlayerUIDFromNum( m_pMFSM->GetState()->nMaster ),
			pNewGoal->nKiruda,
			pNewGoal->nMinScore, f,
			(int)acDrop[0], (int)acDrop[1], (int)acDrop[2] );
		SendMsg( &msg );
	}
	e->SetEvent();
}

void CPlayerNetwork::OnTurn( CCard* pc, int* pe, CEvent* e )
{
	// mmGameTurn 메시지를 받아서 준다
	CMsg* pMsg = 0;
	AUTODELETE_MSG(pMsg);

	GetSB()->GetMsgFor( GetUID(), pMsg, &m_e );
	m_pMFSM->WaitEvent( &m_e );

	long t, u, c, f;
	if ( !pMsg->PumpLong( t ) || t != CMsg::mmGameTurn
				|| !pMsg->PumpLong( u ) || u != GetUID()
				|| !pMsg->PumpLong( c ) || !pMsg->PumpLong( f ) )
		Error(3);

	else {
		*pc = c;
		*pe = f;
	}
	e->SetEvent();
}

void CPlayerNetwork::OnTurn( CCard c, int eff, int nHandIndex, CEvent* e )
{
	// mmGameTurn 메시지를 전달한다
	if ( NeedSendingIfNumIs( m_pMFSM->GetState()->nCurrentPlayer ) ) {

		CMsg msg( _T("llll"), CMsg::mmGameTurn,
			GetPlayerUIDFromNum( m_pMFSM->GetState()->nCurrentPlayer ),
			(int)c, eff );
		SendMsg( &msg );
	}
	e->SetEvent();
}

// 채팅 메시지 (bSource : 채팅창이 소스)
void CPlayerNetwork::OnChat( int nPlayerID, LPCTSTR sMsg, bool bSource )
{
	// mmChat 메시지를 전달한다
	if ( NeedSendingIfIDIs( nPlayerID ) ) {

		CMsg msg( _T("lls"), CMsg::mmChat,
			GetPlayerUIDFromID( nPlayerID ), sMsg );
		SendMsg( &msg );
	}
}
