// PlayerNetwork.h: interface for the CPlayerNetwork class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYERNETWORK_H__C199C143_91F5_11D3_9A9C_000000000000__INCLUDED_)
#define AFX_PLAYERNETWORK_H__C199C143_91F5_11D3_9A9C_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CSocketBag;
class CPlayerSocket;


// 네트워크로 연결된 플레이어 객체

class CPlayerNetwork  : public CPlayer
{
public:
	CPlayerNetwork(
		int nID,		// 고유 번호 ( 6명의 플레이중 순서 )
		LPCTSTR sName,	// 이름
		CWnd* pCallback );	// call-back window
	virtual ~CPlayerNetwork();

	// 이 플레이어는 네트워크인가
	virtual bool IsNetwork() const					{ return true; }

public:
	// 인터페이스 ( CPlay 확장 )

	virtual void OnKillOneFromSix( CCard* pcCardToKill,
		CCardList* plcFailedCardsTillNow, CEvent* );
	virtual void OnElection( CGoal* pNewGoal, CEvent* );
	virtual void OnElected( CGoal* pNewGoal, CCard acDrop[3], CEvent* );
	virtual void OnTurn( CCard* pc, int* pe, CEvent* );

public:
	// 인터페이스 ( CMFSM 이 호출하는 Notify )

	// 한 판이 시작됨 - OnBegin 전, 아직 덱이 분배되지 않은 상태
	virtual void OnInit( CEvent* e );
	// 다른 사람의 선거 결과를 듣는다
	virtual void OnElecting( int nPlayerID, int nKiruda,
		int nMinScore, CEvent* e );
	// 주공이 다른 플레이어를 죽인다
	// bKilled : 참이면 실제로 죽였고, 거짓이면 헛다리 짚었다
	virtual void OnKillOneFromSix( CCard cKill,
		bool bKilled, CEvent* e );
	// 플레이어를 죽인 후 카드를 섞었다
	virtual void OnSuffledForDead( CEvent* e );
	// 특권 모드가 다음과 같은 결과로 종료되었다
	virtual void OnPrivilegeEnd( CGoal* pNewGoal, CCard acDrop[3], CEvent* e );
	// 카드를 냈음을 알려준다
	// 모든 정보는 State 에 있다
	// nHandIndex 는 이 카드가 손에든 카드중 몇번째 인덱스의 카드였는가
	virtual void OnTurn( CCard c, int eff, int nHandIndex, CEvent* e );
	// 채팅 메시지 (bSource : 채팅창이 소스)
	virtual void OnChat( int nPlayerID, LPCTSTR sMsg, bool bSource );

protected:
	CSocketBag* GetSB()								{ return m_pMFSM->GetSockBag(); }
	CPlayerSocket* GetSocket()						{ return GetSB()->GetSocket( GetUID() ); }
	bool IsServer() const							{ return m_pMFSM->IsServer(); }
	long GetUID() const								{ return m_pMFSM->GetPlayerUIDFromID(GetID()); }
	long GetPlayerUIDFromID( long nPlayerID ) const	{ return m_pMFSM->GetPlayerUIDFromID(nPlayerID); }
	long GetPlayerUIDFromNum( long nPlayerNum ) const	{ return m_pMFSM->GetPlayerUIDFromNum(nPlayerNum); }
	long GetPlayerNumFromUID( long nUID ) const		{ return m_pMFSM->GetPlayerNumFromUID(nUID); }
	long GetPlayerIDFromUID( long nUID ) const		{ return m_pMFSM->GetPlayerIDFromUID( nUID ); }
	long GetPlayerIDFromNum( long nPlayerNum ) const	{ return m_pMFSM->GetPlayerIDFromNum(nPlayerNum); }

	// 결과 메시지를 전달해야 하는가를 검사한다
	// MFSM 이 서버일 때 : 내가 송신자(nPlayerID)가 아니라면...
	// MFSM 이 클라이언트일 때 :
	//    내가 서버(uid==0)와 연결되어 있고 이 메시지가
	//    사람(id==0)이 송신자라면...
	// 참을 리턴한다
	bool NeedSendingIfIDIs( int nPlayerID ) const
	{
		return IsServer() && GetID() != nPlayerID
			|| GetUID() == 0 && nPlayerID == 0;
	}
	bool NeedSendingIfNumIs( int nPlayerNum ) const
	{
		return NeedSendingIfIDIs( GetPlayerIDFromNum( nPlayerNum ) );
	}

	// 에러 발생 - MFSM 에게 알린다
	// nErr  :  1 : 접속 종료
	//          2 : Send 실패
	//          3 : 이상한 메시지
	void Error( int nErr );

	void SendMsg( CMsg* pMsg );

	// 이벤트 객체 ( 소켓의 이벤트를 대기함 )
	CEvent m_e;
};

#endif // !defined(AFX_PLAYERNETWORK_H__C199C143_91F5_11D3_9A9C_000000000000__INCLUDED_)
