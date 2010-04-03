// PlayerSocket.cpp: implementation of the CPlayerSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "Msg.h"
#include "ZSocket.h"
#include "PlayerSocket.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


static void call_proc( LPVOID toCall, LPVOID pSock, LPVOID pMsg, DWORD dwUser1, DWORD dwUser2 )
{
	// toCall( this, pMsg, dwUser1, dwUser2 );
	CMsg* pCallMsg = new CMsg( _T("llllll"), CMsg::mmCallSockProc,
		(long)toCall, (long)pSock, (long)pMsg,
		(long)dwUser1, (long)dwUser2 );
	Mw()->PostMessage( WM_CALLSOCKPROC, 0, (LPARAM)(LPVOID)pCallMsg );
}

CPlayerSocket::CPlayerSocket()
{
	m_uid = 0;
	m_pfnProc = 0;
	m_dwUser1 = m_dwUser2 = 0;
}

CPlayerSocket::~CPlayerSocket()
{
	Close();

	m_cs.Lock();
		// 남은 메시지들을 비운다
		POSITION pos;
		pos = m_lpMsg.GetHeadPosition();
		while (pos) delete m_lpMsg.GetNext(pos);
		m_lpMsg.RemoveAll();
	m_cs.Unlock();
}

// 메시지가 생길때 까지 기다렸다가 (이미 가지고 있었다면 즉시)
// pfnProc 를 호출한다 (pMsg 는 pfnProc 의 소유가 됨)
void CPlayerSocket::SetTrigger( DWORD dwUser1, DWORD dwUser2,
	void (*pfnProc)( CPlayerSocket* pSocket, CMsg* pMsg,
					DWORD dwUser1, DWORD dwUser2 ) )
{
	void (*toCall)( CPlayerSocket* pSocket, CMsg* pMsg,
					DWORD dwUser1, DWORD dwUser2 ) = 0;
	CMsg* pMsg = 0;

	m_cs.Lock();

		if ( !m_lpMsg.IsEmpty() && pfnProc ) {

			toCall = pfnProc;
			pMsg = m_lpMsg.RemoveHead();
		}
		else {
			m_pfnProc = pfnProc;
			m_dwUser1 = dwUser1;
			m_dwUser2 = dwUser2;
		}

	m_cs.Unlock();

	if ( toCall ) call_proc( toCall, this, pMsg, dwUser1, dwUser2 );
}

// 메시지를 큐에 넣는다 (보내진 메시지의 소유권은 CPlayerSocket이됨)
void CPlayerSocket::PushMsg( CMsg* pMsg, bool bTop )
{
	void (*toCall)( CPlayerSocket* pSocket, CMsg* pMsg,
					DWORD dwUser1, DWORD dwUser2 ) = 0;

	m_cs.Lock();

		if ( bTop ) m_lpMsg.AddHead( pMsg );
		else m_lpMsg.AddTail( pMsg );

		if ( m_pfnProc ) {	 // 대기중

			toCall = m_pfnProc;
			pMsg = m_lpMsg.RemoveHead();
			m_pfnProc = 0;
		}

	m_cs.Unlock();

	if ( toCall ) call_proc( toCall, this, pMsg, m_dwUser1, m_dwUser2 );
}

BOOL CPlayerSocket::Close()
{
	BOOL bRet = CZSocket::Close();
	PushMsg( new CMsg( _T("l"), CMsg::mmDisconnected ) );
	return bRet;
}

void CPlayerSocket::OnConnect( int nErr )
{
	PushMsg( new CMsg( _T("ll"), CMsg::mmConnected, nErr ) );
}

void CPlayerSocket::OnReceive( int nErr )
{
	if ( nErr ) { ASSERT(0); Close(); return; }

	// 메시지를 받는다
	long len;
	if ( !SyncReceive( &len, sizeof(len) ) ) {
		// 접속 종료
		Close(); return;
	}

	CMsg* pMsg = new CMsg;
	AUTODELETE_MSG_EX(pMsg,adme);

	BYTE* pData = (BYTE*)pMsg->LockBuffer( len );

	if ( !SyncReceive( pData, len ) ) {
		// 접속 종료
		Close(); return;
	}

	pMsg->ReleaseBuffer();

	PushMsg( *adme.Detach() );
}

// 메시지를 보낸다
bool CPlayerSocket::SendMsg( CMsg* pMsg )
{
	long len = pMsg->GetLength();
	return !!SyncSend( &len, sizeof(len) )
		&& !!SyncSend( pMsg->GetBuffer(), len );
}
