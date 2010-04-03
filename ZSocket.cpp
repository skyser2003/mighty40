// ZSocket.cpp: implementation of the CZSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <atlconv.h>

#include "ZSocket.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// CZInetAddr

CZInetAddr::CZInetAddr( LPCTSTR sAddr, UINT uPort )
{
	USES_CONVERSION;

	Init();

	m_addr.sin_port = htons( (u_short)uPort );

	if ( !sAddr || !*sAddr ) return;

	LPSTR szAddr = T2A( (LPTSTR)sAddr );

	m_addr.sin_addr.s_addr = inet_addr( szAddr );

	if ( m_addr.sin_addr.s_addr == INADDR_NONE ) {
		// string notation

		LPHOSTENT lphost;
		lphost = gethostbyname( szAddr );

		if ( lphost != NULL )
			m_addr.sin_addr.s_addr =
				((LPIN_ADDR)lphost->h_addr)->s_addr;
		else {
			WSASetLastError(WSAEINVAL);
		}
	}
}

// 이 주소의 스트링 표현을 얻는다
BOOL CZInetAddr::GetAddr( CString* psAddr, UINT* puPort )
{
	if ( puPort ) *puPort = (UINT)ntohs( m_addr.sin_port );
	if ( psAddr ) {
		LPCSTR a = (LPCSTR)inet_ntoa( m_addr.sin_addr );
		if ( !a ) return FALSE;
		else *psAddr = a;
	}
	return TRUE;
}

// 이 기계의 주소를 얻는다
BOOL CZInetAddr::GetHostAddr( CZInetAddr* pAddr )
{
	USES_CONVERSION;

	char* name = new char[1024];
	int failed = 0;
	for(;;){

		if ( gethostname( name, 1024 ) == 0 ) break;	// success

		if ( ::WSAGetLastError() != WSAEINPROGRESS ) {
			failed = 1; break;	// fail
		}

		Sleep(1000);	// try again
	}

	if ( !failed && pAddr )
		*pAddr = CZInetAddr( A2T(name) );

	delete[] name;
	return failed ? FALSE : TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CZSocket

CZSocket::CZSocket()
{
	m_hSocket = INVALID_SOCKET;
	m_pSockThread = 0;
	m_bTerminate = FALSE;
	m_bTerminated = TRUE;
	m_state = empty;
}

CZSocket::~CZSocket()
{
	// 가상함수에서 자신을 delete 할 수 없다
	ASSERT( AfxGetThread() != m_pSockThread );
	VERIFY( Close() );
}


// 소켓을 닫음
BOOL CZSocket::Close()
{
	if ( m_hSocket != INVALID_SOCKET ) {
		if ( closesocket( m_hSocket ) != 0 )
			return FALSE;
		m_hSocket = INVALID_SOCKET;
	}

	if ( m_pSockThread ) {	// 쓰레드를 죽여야 함

		m_bTerminate = TRUE;

		if ( AfxGetThread() != m_pSockThread ) {
			// 소켓 쓰레드가 자신이 아닐 때
			::WaitForSingleObject( m_pSockThread->m_hThread, INFINITE );
			delete m_pSockThread;
			m_pSockThread = 0;
			m_state = empty;
		}
	}

	return TRUE;
}

// 전형적인 Attach, Detach
BOOL CZSocket::Attach( SOCKET hSocket )
{
	ASSERT( m_hSocket == INVALID_SOCKET );
	ASSERT( m_state == empty );

	if ( !Close() ) return FALSE;

	m_hSocket = INVALID_SOCKET;

	return TRUE;
}

SOCKET CZSocket::Detach()
{
	if ( !m_bTerminated ) {	// 쓰레드를 죽여야 함

		m_bTerminate = TRUE;
		while ( !m_bTerminated ) Sleep(1000);
		m_pSockThread = 0;

		m_bTerminate = FALSE;
		m_bTerminated = TRUE;
	}

	SOCKET hSocket = m_hSocket;
	m_hSocket = INVALID_SOCKET;

	m_state = empty;

	return hSocket;
}

// 서버용 nType ( SOCK_STREAM 또는 SOCK_DGRAM ) 소켓을 생성
// uPort, sAddr 로 Bind 함
// SOCK_STREAM 인 경우, Listen 하고, OnAccept 함수가 호출됨
// SOCK_DGRAM 인 경우, 즉시 Send(To), Receive(From) 함수 호출가능
BOOL CZSocket::CreateServerSocket( const SOCKADDR_IN* pAddr, int nType )
{
	ASSERT( m_state == empty && m_hSocket == INVALID_SOCKET );
	ASSERT( pAddr );

	m_hSocket = socket( AF_INET, nType, 0 );
	if ( m_hSocket == INVALID_SOCKET ) return FALSE;

	if ( bind( m_hSocket,
		(const SOCKADDR*)pAddr,
		sizeof(SOCKADDR_IN) ) != 0 ) {
		// 실패
		VERIFY( closesocket( m_hSocket ) == 0 );
		m_hSocket = 0;
		return FALSE;
	}

	if ( nType == SOCK_STREAM ) {
		// 스트림 소켓이므로 listen 까지 해 준다
		VERIFY( listen( m_hSocket, 5 ) == 0 );
		m_state = accepting;
	}
	else {
		ASSERT( nType == SOCK_DGRAM );
		m_state = waiting;
	}

	InvokeThread();

	return TRUE;
}

// 클라이언트용 스트림 소켓을 생성
BOOL CZSocket::CreateClientSocket( const SOCKADDR_IN* pAddr )
{
	ASSERT( m_state == empty && m_hSocket == INVALID_SOCKET );

	m_hSocket = socket( AF_INET, SOCK_STREAM, 0 );
	if ( m_hSocket == INVALID_SOCKET ) return FALSE;

	m_state = connecting;
	m_addrConn = CZInetAddr( pAddr );

	InvokeThread();

	return TRUE;
}

// 접속 요청을 받아들임
// 이 함수는 OnAccept 함수내에서 호출해야 Blocking 되지 않는다
// ( sock 은 빈 CZSocket 객체 )
BOOL CZSocket::Accept( CZSocket& sock )
{
	ASSERT( m_state == accepting && sock.m_state == empty
		&& sock.m_hSocket == INVALID_SOCKET );

	SOCKET s = accept( m_hSocket, 0, 0 );
	if ( s == INVALID_SOCKET ) return FALSE;

	// 접속된 s 에 대해 소켓 시스템을 구성한다
	sock.m_hSocket = s;
	sock.m_state = waiting;
	sock.InvokeThread();

	return TRUE;
}

// 이 소켓이 바인드 된 주소를 리턴
CZInetAddr CZSocket::GetSockName()
{
	SOCKADDR_IN addr;
	int len = sizeof(addr);

	if ( getsockname( m_hSocket, (SOCKADDR*)&addr, &len ) != 0 )
		return CZInetAddr();
	else return CZInetAddr( &addr );
}

CZInetAddr CZSocket::GetPeerName()
{
	SOCKADDR_IN addr;
	int len = sizeof(addr);

	if ( getpeername( m_hSocket, (SOCKADDR*)&addr, &len ) != 0 )
		return CZInetAddr();
	else return CZInetAddr( &addr );
}


// 데이터 송수신 관련

// 데이터를 전송한다
BOOL CZSocket::Send( LPCVOID pData, DWORD dwSize, DWORD* pdwSent, BOOL bOOB )
{
	ASSERT( m_state == waiting && m_hSocket != INVALID_SOCKET );

	int nSent = send( m_hSocket, (const char*)pData, (int)dwSize, bOOB ? MSG_OOB : 0 );

	if ( nSent == SOCKET_ERROR ) return FALSE;

	if ( pdwSent ) *pdwSent = (DWORD)nSent;
	return TRUE;
}

BOOL CZSocket::SyncSend( LPCVOID pData, DWORD dwSize, BOOL bOOB )
{
	DWORD dwSent;

	while ( dwSize != 0 ) {

		if ( !Send( pData, dwSize, &dwSent, bOOB ) ) {

			if ( WSAGetLastError() == WSAEINPROGRESS
				|| WSAGetLastError() == WSAEWOULDBLOCK ) {
				Sleep(1000);	// 좀 있다 다시 해야 한다
				continue;
			}
			else return FALSE;
		}

		dwSize -= dwSent;
		(const char*&)pData += dwSent;
	}
	return TRUE;
}

BOOL CZSocket::SendTo( LPCVOID pData, DWORD dwSize, DWORD* pdwSent,
						const SOCKADDR_IN* pAddr, BOOL bOOB )
{
	ASSERT( m_state == waiting && m_hSocket != INVALID_SOCKET );

	if ( !pAddr ) return Send( pData, dwSize, pdwSent, bOOB );

	int nSent = sendto( m_hSocket, (const char*)pData, (int)dwSize, bOOB ? MSG_OOB : 0,
						(const SOCKADDR*)pAddr, sizeof(SOCKADDR_IN) );

	if ( nSent == SOCKET_ERROR ) return FALSE;

	if ( pdwSent ) *pdwSent = (DWORD)nSent;
	return TRUE;
}

// 데이터를 읽어 들인다
BOOL CZSocket::Receive( LPVOID pData, DWORD dwSize, DWORD* pdwReceived, BOOL bOOB )
{
	ASSERT( m_state == waiting && m_hSocket != INVALID_SOCKET );

	int nRecv = recv( m_hSocket, (char*)pData, (int)dwSize, bOOB ? MSG_OOB : 0 );

	if ( nRecv == SOCKET_ERROR || nRecv == 0 ) return FALSE;

	if ( pdwReceived ) *pdwReceived = (DWORD)nRecv;
	return TRUE;
}

BOOL CZSocket::SyncReceive( LPVOID pData, DWORD dwSize, BOOL bOOB )
{
	DWORD dwReceived;

	while ( dwSize != 0 ) {

		if ( !Receive( pData, dwSize, &dwReceived, bOOB ) ) {

			if ( WSAGetLastError() == WSAEINPROGRESS
				|| WSAGetLastError() == WSAEWOULDBLOCK ) {
				Sleep(1000);	// 좀 있다 다시 해야 한다
				continue;
			}
			else return FALSE;
		}

		dwSize -= dwReceived;
		(char*&)pData += dwReceived;
	}
	return TRUE;
}

BOOL CZSocket::ReceiveFrom( LPVOID pData, DWORD dwSize, DWORD* pdwReceived,
							SOCKADDR_IN* pAddr, BOOL bOOB )
{
	ASSERT( m_state == waiting && m_hSocket != INVALID_SOCKET );

	if ( !pAddr ) return Receive( pData, dwSize, pdwReceived, bOOB );

	int lenAddr = sizeof(SOCKADDR_IN);

	int nRecv = recvfrom( m_hSocket, (char*)pData, (int)dwSize, bOOB ? MSG_OOB : 0, (SOCKADDR*)pAddr, &lenAddr );

	if ( nRecv == SOCKET_ERROR ) return FALSE;

	if ( pdwReceived ) *pdwReceived = (DWORD)nRecv;
	return TRUE;
}

// 가상 함수들

void CZSocket::OnAccept( int ) {}
void CZSocket::OnConnect( int ) {}
void CZSocket::OnReceive( int ) {}

// 스레드 관련

// 소켓을 select 한다 ( select() 의 간소화된 버전 )
// 리턴값 : 0 이면 타임아웃, SOCKET_ERROR 면 에러
//          양수 이면 셋 중의 하나라도 참임
int CZSocket::Select( BOOL* pbCanRead, BOOL* pbCanWrite, BOOL* pbException, DWORD dwTime )
{
	int nfds = m_hSocket+1;

	fd_set rfs, wfs, efs;
	FD_ZERO( &rfs ); FD_SET( m_hSocket, &rfs );
	FD_ZERO( &wfs ); FD_SET( m_hSocket, &wfs );
	FD_ZERO( &efs ); FD_SET( m_hSocket, &efs );

	timeval tv;
	tv.tv_sec = dwTime / 1000;
	tv.tv_usec = dwTime % 1000 * 1000;

	int ret = select( nfds,
				pbCanRead ? &rfs : 0,
				pbCanWrite ? &wfs : 0,
				pbException ? &efs : 0,
				dwTime == INFINITE ? 0 : &tv );

	if ( ret == SOCKET_ERROR ) return ret;

	if ( pbCanRead )
		*pbCanRead = FD_ISSET( m_hSocket, &rfs ) ? TRUE : FALSE;
	if ( pbCanWrite )
		*pbCanWrite = FD_ISSET( m_hSocket, &wfs ) ? TRUE : FALSE;
	if ( pbException )
		*pbException = FD_ISSET( m_hSocket, &efs ) ? TRUE : FALSE;

	return ret;
}


// 주어진 주소로 Connect 한다 - m_bTerminate 가
// 참이 되면 바로 FALSE 를 리턴한다
BOOL CZSocket::AsyncConnect( const SOCKADDR_IN* pAddr )
{
	// 소켓을 NB 모드로 세트한다 (connect 를 위해)
	u_long arg = 1;
	if ( ioctlsocket( m_hSocket, FIONBIO, &arg ) != 0 ) return FALSE;

	BOOL bRet = FALSE;
	int nErrCode = 0;

	if ( connect( m_hSocket, (const SOCKADDR*)pAddr, sizeof(SOCKADDR_IN) ) == 0 ) {
		// connection 성공 !
		bRet = TRUE; goto lblExit;
	}

	nErrCode = WSAGetLastError();
	if ( nErrCode != WSAEWOULDBLOCK ) {
		// WSAEWOULDBLOCK 이 아니면 진짜 실패한것이다
		goto lblExit;
	}

	// 블럭된 상태 - 접속되기를 기다리자
	// m_bTerminate 를 계속해서 검사해야 한다 (1000 밀리초마다하자)
	{
		BOOL bCanWrite = FALSE;
		BOOL bException = FALSE;

		while ( !m_bTerminate && !bCanWrite ) {

			if ( Select( 0, &bCanWrite, &bException, 1000 )
				== INVALID_SOCKET || bException ) {
				// 접속 실패 !
				nErrCode = WSAEINVAL;
				goto lblExit;
			}
		}

		bRet = TRUE;
		ASSERT( m_bTerminate || bCanWrite );
		goto lblExit;
	}

lblExit:
	if ( m_bTerminate ) {
		if ( !nErrCode ) nErrCode = WSAEINTR;
		bRet = FALSE;
	}

	// 소켓 모드를 원위치로
	arg = 0;
	if ( m_hSocket != INVALID_SOCKET )
		VERIFY( ioctlsocket( m_hSocket, FIONBIO, &arg ) == 0 );
	// 리턴값
	WSASetLastError( nErrCode );
	return bRet;
}

void CZSocket::InvokeThread()
{
	ASSERT( m_pSockThread == 0 );
	ASSERT( m_bTerminated == TRUE );

	m_bTerminated = FALSE;
	m_bTerminate = FALSE;

	m_pSockThread = AfxBeginThread( Proc, this, THREAD_PRIORITY_NORMAL, 0,
									CREATE_SUSPENDED );
	m_pSockThread->m_bAutoDelete = FALSE;
	m_pSockThread->ResumeThread();
}

UINT CZSocket::Proc( LPVOID pParam )
{	return ((CZSocket*)pParam)->Proc(); }


// 주 쓰레드
UINT CZSocket::Proc()
{
	// 접속을 위해서 쓰레드가 시작되었다
	if ( m_state == connecting ) {

		BOOL bRet = AsyncConnect( m_addrConn );
		if ( m_bTerminate ) goto lblExit;

		if ( !bRet ) {	// 실패
			if ( !WSAGetLastError() ) WSASetLastError(WSAEINVAL);
			OnConnect( WSAGetLastError() );
			goto lblExit;
		}
		else {
			OnConnect( 0 );	// 성공
			m_state = waiting;
		}
	}
	// 접속응답을 위해 쓰레드가 시작되었다
	if ( m_state == accepting ) {

		while ( !m_bTerminate ) {	// 무한루프, 서비스

			BOOL bCanRead = FALSE;
			BOOL bException = FALSE;

			if ( Select( &bCanRead, 0, &bException, 1000 )
				== INVALID_SOCKET || bException ) {	// 실패

				if ( m_bTerminate ) goto lblExit;
				if ( !WSAGetLastError() ) WSASetLastError(WSAEINVAL);
				OnAccept( WSAGetLastError() );
			}

			if ( m_bTerminate ) goto lblExit;

			if ( bCanRead ) {
				// 접속 요청을 받아들임
				OnAccept( 0 );
			}
		}
		goto lblExit;
	}
	// 일반적인 send/recv 서비스
	if ( m_state == waiting ) {

		while ( !m_bTerminate ) {	// 무한루프, 서비스

			BOOL bCanRead = FALSE;
			BOOL bException = FALSE;

			if ( Select( &bCanRead, 0, &bException, 1000 )
				== INVALID_SOCKET || bException ) {	// 실패

				if ( m_bTerminate ) goto lblExit;
				if ( !WSAGetLastError() ) WSASetLastError(WSAEINVAL);
				OnReceive( WSAGetLastError() );
			}

			if ( m_bTerminate ) goto lblExit;

			if ( bCanRead ) {
				// 읽기 가능
				OnReceive( 0 );
			}
		}
		goto lblExit;
	}

lblExit:
	m_bTerminated = TRUE;
	m_bTerminate = FALSE;
	return 0;
}
