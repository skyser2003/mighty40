// ZSocket.h: interface for the CZSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZSOCKET_H__47501A73_93F2_11D3_9AA3_0000212035B8__INCLUDED_)
#define AFX_ZSOCKET_H__47501A73_93F2_11D3_9AA3_0000212035B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// CAsyncSocket 에 대한 환멸과 증오를 모아
// 이 라이브러리를 작성함  1999. 11. 6. 장문성

// 참고 : wsock32.lib(또는 ws2_32.lib) 를 수동으로 링크하고,
//        winsock.h(또는 winsock2.h) 를 include 해야 한다


// 주소 클래스
class CZInetAddr
{
public:
	// 주소
	SOCKADDR_IN m_addr;

	operator const SOCKADDR_IN*() const;

public:
	// 다양한 방법으로 주소를 생성한다
	CZInetAddr( LPCTSTR sAddr = 0, UINT uPort = 0 );
	CZInetAddr( const SOCKADDR_IN* pAddr );
	CZInetAddr& operator=( const SOCKADDR_IN* pAddr );

	// 이 주소의 스트링 표현을 얻는다
	BOOL GetAddr( CString* psAddr = 0, UINT* puPort = 0 );

public:
	// 기타 유틸리티

	// 이 기계의 주소를 얻는다
	static BOOL GetHostAddr( CZInetAddr* pAddr );

protected:
	void Init();
};


// 소켓 클래스 정의
class CZSocket  
{
public:
	// 소켓 핸들
	SOCKET m_hSocket;

public:
	// 빈 소켓 클래스를 생성
	CZSocket();

	// 소켓 클래스를 파괴함
	// Close 를 내부적으로 호출한다
	virtual ~CZSocket();

	// 소켓을 닫음
	virtual BOOL Close();

	// 전형적인 Attach, Detach
	BOOL Attach( SOCKET hSocket );
	SOCKET Detach();

	// 서버용 nType ( SOCK_STREAM 또는 SOCK_DGRAM ) 소켓을 생성
	// uPort, sAddr 로 Bind 함
	// SOCK_STREAM 인 경우, Listen 하고, OnAccept 함수가 호출됨
	// SOCK_DGRAM 인 경우, 즉시 Send(To), Receive(From) 함수 호출가능
	BOOL CreateServerSocket( const SOCKADDR_IN* pAddr, int nType = SOCK_STREAM );

	// 클라이언트용 스트림 소켓을 생성
	BOOL CreateClientSocket( const SOCKADDR_IN* pAddr );

public:
	// 접속 요청을 받아들임
	// 이 함수는 OnAccept 함수내에서 호출해야 Blocking 되지 않는다
	// ( sock 은 빈 CZSocket 객체 )
	BOOL Accept( CZSocket& sock );

	// 이 소켓이 바인드 된 주소를 리턴
	CZInetAddr GetSockName();
	CZInetAddr GetPeerName();

public:
	// 데이터 송수신 함수
	// 실패시 FALSE 를 리턴한다 - 특히 접속이 끊겼을 때도 FALSE 를
	// 리턴한다

	// 데이터를 전송한다
	BOOL Send( LPCVOID pData, DWORD dwSize, DWORD* pdwSent = 0, BOOL bOOB = FALSE );
	BOOL SyncSend( LPCVOID pData, DWORD dwSize, BOOL bOOB = FALSE );
	BOOL SendTo( LPCVOID pData, DWORD dwSize, DWORD* pdwSent = 0,
					const SOCKADDR_IN* pAddr = 0, BOOL bOOB = FALSE );

	// 데이터를 읽어 들인다
	BOOL Receive( LPVOID pData, DWORD dwSize, DWORD* pdwReceived = 0, BOOL bOOB = FALSE );
	BOOL SyncReceive( LPVOID pData, DWORD dwSize, BOOL bOOB = FALSE );
	BOOL ReceiveFrom( LPVOID pData, DWORD dwSize, DWORD* pdwReceived = 0,
						SOCKADDR_IN* pAddr = 0, BOOL bOOB = FALSE );

protected:
	// 가상 함수
	// 주의 : 이 함수들은 주 쓰레드와 다른 쓰레드에서 호출된다

	// 수신된 접속요청이 있을 때
	// (서버소켓이 생성된 후, 다른 소켓이 이 주소로 접속했을 때
	//  호출되며, 여기서 Accept() 함수를 호출 할 수 있다)
	virtual void OnAccept( int nErr );
	// 접속이 성공했을 때
	// (클라이언트 소켓의 Connect() 가 호출 된 후,
	//  서버로부터 응답이 왔을 때)
	virtual void OnConnect( int nErr );
	// 수신될 데이터가 있을 때 (즉 Receive 함수가 즉시 성공할수 있을 때)
	// 또는 접속이 끊어졌을 때 (Read함수가 FALSE 리턴)
	virtual void OnReceive( int nErr );

protected:
	// 숨겨진 멤버들

	// 소켓 관리 쓰레드
	CWinThread* m_pSockThread;
	static UINT Proc( LPVOID pParam );
	UINT Proc();
	volatile BOOL m_bTerminate;
	volatile BOOL m_bTerminated;
	void InvokeThread();

	// 현재 상태
	enum SOCKSTATE {
		empty,	// 아무것도 아닌 상태
		accepting,	// 접속대기중
		connecting,	// 접속중
		waiting,	// 정상 동작상태
	} m_state;

	// 접속해야 할 주소 ( m_state == connecting 일때만 사용 )
	CZInetAddr m_addrConn;

	// 소켓을 select 한다 ( select() 의 간소화된 버전 )
	// 리턴값 : 0 이면 타임아웃, SOCKET_ERROR 면 에러
	//          양수 이면 셋 중의 하나라도 참임
	int Select( BOOL* pbCanRead, BOOL* pbCanWrite, BOOL* pbException, DWORD dwTime = INFINITE );

	// 주어진 주소로 Connect 한다 - m_bTerminate 가
	// 참이 되면 바로 FALSE 를 리턴한다
	BOOL AsyncConnect( const SOCKADDR_IN* pAddr );
};



// 인라인 함수들
__inline CZInetAddr::CZInetAddr( const SOCKADDR_IN* pAddr )
{
	if ( !pAddr ) Init();
	else memcpy( &m_addr, pAddr, sizeof(SOCKADDR_IN) );
}

__inline CZInetAddr& CZInetAddr::operator=( const SOCKADDR_IN* pAddr )
{
	if ( !pAddr ) Init();
	else memcpy( &m_addr, pAddr, sizeof(SOCKADDR_IN) );
	return *this;
}

__inline void CZInetAddr::Init()
{
	memset( &m_addr, 0, sizeof(SOCKADDR_IN) );
	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons( 0 );
	m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
}

__inline CZInetAddr::operator const SOCKADDR_IN*() const
{
	return &m_addr;
}

#endif // !defined(AFX_ZSOCKET_H__47501A73_93F2_11D3_9AA3_0000212035B8__INCLUDED_)
