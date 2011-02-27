// MFSM.h: interface for the CMFSM class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MFSM_H__FDD28386_D3C4_11D2_981F_000000000000__INCLUDED_)
#define AFX_MFSM_H__FDD28386_D3C4_11D2_981F_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPlayer;
class CSocketBag;
struct CPlay;

// Mighty Final State Machine
// 마이티의 게임 자체를 진행하는 매우 추상화 된 루틴
// Init 을 호출하면 독자적인 쓰레드를 띄워서 진행된다

class CMFSM  : private CState
{
public:
	// * 주의 *
	// 플레이어들은 이 MFSM 이 파괴되는 순간 자동적으로
	// 이 MFSM 이 delete 한다
	CMFSM( LPCTSTR sRule, CPlayer* _apPlayers[], CSocketBag* pSockBag = 0 );

public:
	// 서비스를 시작한다
	// 새로운 쓰레드가 곧바로 시작된다
	// uid : 이 MFSM 의 uid ( 0 이면 이 MFSM 은 반드시 서버이다 )
	void Init( long uid );
	// 서버 쓰레드의 핸들을 Duplicate 한다 (Wait 하기 위해)
	HANDLE DuplicateThreadHandle();

	// 현재 상태를 얻는다
	const CState* GetState() const					{ return (const CState*)this; }
	// 소켓집합을 얻는다
	CSocketBag* GetSockBag()						{ return m_pSockBag; }
	// 서버인가?
	bool IsServer() const							{ return m_uid == 0; }

public:
	// 플레이어를 표현하는 ID, UID, Num 간의 변환을 한다
	// 해당 플레이어가 존재하지 않는 경우 -1 을 리턴한다 (Num 을 리턴하는 함수의 경우)
	long GetPlayerUIDFromID( long id );
	long GetPlayerIDFromUID( long uid );
	long GetPlayerIDFromNum( long num );
	long GetPlayerNumFromID( long id );
	long GetPlayerNumFromUID( long uid );
	long GetPlayerUIDFromNum( long num );

public:
	// 유틸리티 함수들

	// 모든 플레이어의 카드를 옵션에 따라 정렬한다
	void SortPlayerHand( bool bLeftKiruda, bool bLeftAce );

	// 플레이어의 이름들을 재설정한다
	void SetPlayerNames( LPCTSTR asName[] );

	// 이 카드(pCard)로 이 플레이어(nPlayerID)가
	// 다른 플레이어를 죽일 수 있는가
	// 그럴 수 있다면 true, 죽일 수 없고 다른 카드를 찾아야
	// 한다면(헛다리) false 를 리턴한다
	// pDeadID 는 죽일 플레이어를 리턴
	bool KillTest( int nPlayerID, CCard* pCard, int* pDeadID );

	// (선거중) 주어진 goalNew 가 선거를 종료시킬 수
	// 있는가를 판단한다 - 현재의 goal, abGiveUp, IsDealMiss() 를
	// 종합적으로 고려한다
	// 리턴값들 - bDealMiss : 딜 미스로 종료되었는가를 판단
	//            nNextID : 당선자 ID 또는 다음 출마자 ID
	// 선거를 계속 해야 하는지(false), 딜 미스나 당선 확정으로
	// 종료되는지(true)를 리턴한다
	bool CanBeEndOfElection( bool& bDealMiss, long& nNextID,
		const CGoal& goalNew );

	// 이 카드에 대한 설명 스트링을 리턴한다
	CString GetHelpString( CCard c ) const;

	// 게임 종료시에 보여지는 돈 계산 리포트를 얻는다
	// 배열 리턴값의 배열 크기는 플레이를 한 인원 ( 최대 5 )
	void GetReport(
		// 리턴값
		bool& bDefWin,			// 방어(여당)측이 승리했는가 ?
		int* pnDefPointed,		// 여당 득점
		int anContrib[],		// 공헌도 (각 편에서 몇 퍼센트 정도의 활약을 했는가)
		CString asCalcMethod[2],// [0] 계산 방법과, 기본 액수의 계산 식 [1] 기타 곱절규칙
		CString asExpr[],		// 돈의 계산 식
		// 값-결과-독립변수
		int anMoney[]			// 호출시에는 원래 가졌던 돈, 리턴될 때는 계산된 돈
	) const;

public:
	// 이벤트 발생을 알림

	// 종료 메시지
	void EventExit( LPCTSTR sMsg = 0 );
	// 채팅 메시지 ( bSource 가 참이면, 채팅창에서 만들어진 메시지 )
	void EventChat( CMsg* pMsg, bool bSource );

public:
	// 오직 CPlayer 만 부르는 함수들

	// 이전에 시킨 작업이 완료되었음을 MFSM 에게 알린다
	void Notify( int nPlayerID );
	// 해당 이벤트를 Lock(=Wait)한다 - 종료 조건시 CMFSM 자신이
	// throw 된다
	void WaitEvent( CEvent* pEvent );

	// 이 함수는 프로그램 설계가 잘못되어 필요한 추한 함수다
	// 플레이어가 호출하면 주공의 카드 중 acDrop 에 해당하는
	// 세 장의 카드를 버리고 중간 선거 결과를
	// 갱신한다 - MFSM 이 어차피 호출해 주지만
	// CPlayerHuman 의 경우 버릴 카드 선택과 프랜드 선택 사이에
	// 화면 Update 가 필요하기 때문에 이 함수가 필요하다
	void RemoveDroppedCards( CCard acDrop[3], int nNewKiruda );

	// 특히 CPlayerMAI 만 부르는 함수들

	// AI 쓰레드에게, 자신의 함수를 호출하라고 요청한다
	// nType : 0 OnElection / 1 OnKillOneFromSix / 2 OnTurn
	void RequestCallAIThread( CPlay* pPlay, int nType,
				CEvent* pEvent, LPVOID arg1 = 0, LPVOID arg2 = 0 );

protected:
	// 0번 플레이어의 서버에서의 ID
	long m_uid;

	// 쓰레드에게 사건을 알리는 이벤트
	CEvent m_eNotify;
	// 사건들
	volatile bool m_bTerminate;	// 종료하라
	// 서비스 쓰레드가 이 값을 세트함으로서 응답
	volatile bool m_bTerminated;
	// soft 한 종료 요청
	volatile bool m_bTermRequest;
	CString m_sTermRequestReason;
	// 채팅 메시지 요청
	CCriticalSection m_csChatMsg;
	struct CHATITEM { CMsg* pMsg; bool bSource; };
	CList<CHATITEM,CHATITEM&> m_lChatMsg;
	// 쓰레드 진입점
	static UINT ServiceProc( LPVOID pThis );
	// 서버 쓰레드
	UINT Server();
	CWinThread* m_pServerThread;

	// 소켓들
	CSocketBag* m_pSockBag;
	// 소켓 이벤트를 대기하기 위한 이벤트 객체
	CEvent m_eSock;

	// 플레이어들에 대한 모든 이벤트 ( 수동 이벤트들임 )
	CEvent* m_apePlayer[MAX_CONNECTION];
	// 각 플레이어에 대해 요청 함수를 호출해야 하는지 여부
	bool m_abWaitingPlayerRedo[MAX_CONNECTION];
	// 현재 대기중인 플레이어에 대한 이벤트와 개수
	CEvent* m_apeWaitingPlayer[MAX_CONNECTION+1];	// 이벤트(m_eNotify 포함)
	int m_nWaitingPlayer;						// 개수
	// 이벤트를 리셋하고, m_apeWaitingPlayer 등을 초기화 한다
	void ResetEvents();
	// 이벤트를 대기하며, 예외를 처리하는 함수
	// 대기중인 플레이어의 이벤트를 감시, Set 되면 m_apeWaitingPlayer
	// 에서 그 이벤트를 삭제한다
	// 이 함수는 매우 중요하며 자세한 것은 실제 구현을 참조
	// 리턴값은 방금 event 가 세트된 플레이어의 PlayerID 또는 -1 (없음)
	int Wait();

	// AI 쓰레드 관련
	void InvokeAIThread();
	CWinThread* m_pAIThread;	// 그 쓰레드
	static UINT AIThread( LPVOID pVoid );
	UINT AIThread();
	CEvent m_eAIThread;
	volatile bool m_bAIThreadTerminate;
	volatile bool m_bAIThreadTerminated;
	volatile int m_nAIThreadRequest;
	LPVOID m_apArgs[2];
	CEvent* m_peAIThreadRequestedEvent;
	CPlay* m_pAIThreadTargetPlay;
	CCriticalSection m_csAIThread;
	void CallAIThread( CPlay* pPlay, int nType,
				CEvent* pEvent, LPVOID arg1, LPVOID arg2 );

	// 기타 보조 함수

	// 소멸자
	virtual ~CMFSM();

	// 죽은 플레이어가 생겼으니, apPlayers[] 를 재배열한다
	void RebuildPlayerArray();
	// m_eNotify 이벤트를 처리
	void ProcessNotifyEvents();
	// 채팅 메시지를 처리
	void ProcessChatMessage();
	// 조커콜이 가능한 상황인지 조사하여 그렇다면 주어진
	// 인자에 따라 bIsJokercall 을 세트한다
	// 또 조커인 경우 nJokerShape 을 세트한다
	void SetJokerState( CCard c, int nEffect );
	// 자리를 섞는다 ( 자리 섞는 옵션이 켜진 경우에만 : 2011.2.27 )
	void SuffleSeat( int& nBeginer );
	// 서버에서 자리를 얻어오고 딜러 위치를 정한다
	void GetSeatFromServer( int& nBeginer );
	// 서버에서 덱을 얻어온다
	void GetDeckFromServer();
};

#endif // !defined(AFX_MFSM_H__FDD28386_D3C4_11D2_981F_000000000000__INCLUDED_)
