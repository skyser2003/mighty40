// SocketBag.h: interface for the CSocketBag class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOCKETBAG_H__4795E9E3_91BA_11D3_9A9B_0000212035B8__INCLUDED_)
#define AFX_SOCKETBAG_H__4795E9E3_91BA_11D3_9A9B_0000212035B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMFSM;
class CPlayerSocket;

// 소켓들의 집합 - CPlayerSocket 과 CMFSM 의
// 연결 지원

class CSocketBag  
{
public:
	CSocketBag();
	virtual ~CSocketBag();

	// 반드시 호출할 것
	void SetMFSM( CMFSM* pMFSM ) { m_pMFSM = pMFSM; }

	// 초기화 - 두 버전 중 하나를 반드시 호출해야 한다

	// 클라이언트용 초기화 (각 플레이어에 대해 반복해서 호출한후
	// 두번째 버전을 부른다)
	void InitForClient( long uid );
	void InitForClient( CPlayerSocket* pServerSocket );
	// 서버용 초기화 (각 플레이어에 대해 반복해서 호출한다)
	void InitForServer( long uid, CPlayerSocket* pSocket );

public:
	// 해당 uid 에 대한 메시지를 Async 하게
	// pMsg 에 세트하고 pEvent 를 세트함으로서 알린다
	void GetMsgFor( long uid, CMsg*& pMsg, CEvent* pEvent );

	// 해당 uid 에 대한 소켓을 얻는다 ( 클라이언트 모드라면
	// 무조건 ServerSocket 이 리턴된다 )
	CPlayerSocket* GetSocket( long uid )
	{
		if ( m_pServerSocket ) return m_pServerSocket;
		else return FindUID(uid)->pSocket;
	}

protected:
	// 각 소켓
	CPlayerSocket* m_pServerSocket;
	int m_nClients;
	struct CLIENTITEM {
		long uid;
		CPlayerSocket* pSocket;
		CList<CMsg*,CMsg*> lmsg;	// 메시지 ( AddTail, RemoveHead 큐 )
		CMsg** ppMsg;	// 메시지 트리거 결과
		CEvent* pEvent;	// 메시지 트리거 완료이벤트
	} m_aClients[MAX_CONNECTION];

	CMFSM* m_pMFSM;
	CCriticalSection m_cs;

protected:
	// UID 를 찾는다
	CLIENTITEM* FindUID( long uid )
	{
		for ( int i = 0; i < m_nClients; i++ )
			if ( m_aClients[i].uid == uid ) return &m_aClients[i];
		ASSERT(0); return 0;
	}
	// socket callback
	static void SockProc( CPlayerSocket* pSocket, CMsg* pMsg,
					DWORD dwUser1, DWORD dwUser2 );
	void SockProc( long uid, CMsg* pMsg, CPlayerSocket* pSocket );
};

#endif // !defined(AFX_SOCKETBAG_H__4795E9E3_91BA_11D3_9A9B_0000212035B8__INCLUDED_)
