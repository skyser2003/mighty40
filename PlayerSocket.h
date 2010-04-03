// PlayerSocket.h: interface for the CPlayerSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYERSOCKET_H__2BCC6B65_8ECD_11D3_9A8E_000000000000__INCLUDED_)
#define AFX_PLAYERSOCKET_H__2BCC6B65_8ECD_11D3_9A8E_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMsg;

// CZSocket 을 기반으로 하는 마이티 소켓
// 추가되는 기능은,
// 메시지 송수신(메시지 리스트 버퍼 이용)
// 트리거(콜백함수) 세트기능
// 콜백함수의 쓰레드를 주 쓰레드에서 호출하도록 함
// (MFC 의 제약 때문)
// 이 소켓은 메인 프레임 윈도우의 WM_CALLSOCKPROC 메시지와
// 연동한다

class CPlayerSocket : public CZSocket
{
public:
	CPlayerSocket();
	virtual ~CPlayerSocket();

public:
	// UID 값
	long GetUID() const								{ return m_uid; }
	void SetUID( long uid )							{ m_uid = uid; }

public:
	// 메시지가 생길때 까지 기다렸다가 (이미 가지고 있었다면 즉시)
	// pfnProc 를 호출한다 (pMsg 는 pfnProc 의 소유가 됨)
	void SetTrigger( DWORD dwUser1, DWORD dwUser2,
		void (*pfnProc)( CPlayerSocket* pSocket, CMsg* pMsg,
						DWORD dwUser1, DWORD dwUser2 ) );
	// 위에서 세트한 핸들러를 해제한다
	void ClearTrigger() { SetTrigger( 0, 0, 0 ); }

	// 메시지를 큐에 넣는다 (보내진 메시지의 소유권은 CPlayerSocket이됨)
	// bTop 이 참이면, 스택의 맨 위에 넣는다 (직후의 GetMsg 에서 이 메시지가
	// 얻어진다 )
	void PushMsg( CMsg*, bool bTop = false );

	// 메시지를 보낸다
	bool SendMsg( CMsg* pMsg );

	// 소켓 종료
	virtual BOOL Close();

protected:
	long m_uid;
	CCriticalSection m_cs;
	CList<CMsg*,CMsg*> m_lpMsg;

	void (*m_pfnProc)( CPlayerSocket* pSocket, CMsg* pMsg,
						DWORD dwUser1, DWORD dwUser2 );
	DWORD m_dwUser1;
	DWORD m_dwUser2;

	virtual void OnReceive( int nErr );
	virtual void OnConnect( int nErr );
};

#endif // !defined(AFX_PLAYERSOCKET_H__2BCC6B65_8ECD_11D3_9A8E_000000000000__INCLUDED_)
