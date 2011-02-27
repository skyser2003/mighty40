// Player.h: interface for the CPlayer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYER_H__E21B2746_CFA8_11D2_9810_000000000000__INCLUDED_)
#define AFX_PLAYER_H__E21B2746_CFA8_11D2_9810_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMsg;
class CMFSM;

// 전적
struct GAME_RECORD {
	int wm;			// win count as a master
	int lm;			// lost count as a master
	int wf;			// win count as a friend
	int lf;			// lost count as a friend
	int wa;			// win count as a attacker
	int la;			// lost count as a attacker
};


// 플레이어의 슈퍼클래스
// 각 플레이어에 대한 정보 (손에 든 카드, 이름 등) 를
// 유지하는 클래스이며, CPlay가 제공하는 인터페이스를 구현한다
// (추가적인 이벤트 정보를 사용하기 때문에 가상함수는 아님)

// 참고 : 플레이어ID  : 그 호스트 내에서의 플레이어의 고유한 ID
//                      6마때 죽은 플레이어까지 함께 포함하며 CState::apAllPlayers
//                      내에서의 인덱스이다
//        플레이어번호: 실제로 게임을 하는 플레이어의 번호
//                      CState::apPlayer 내에서의 인덱스로, 사람이 플레이
//                      하는 경우 0 번은 항상 사람이다
//        플레이어UID : 이 호스트 내에서 플레이어ID 가 0 인 플레이어가
//                      실제 서버에서 갖는 플레이어ID

class CPlayer
{
public:
	CPlayer( int nID,		// 고유 번호 ( 7명의 플레이어중 순서 )
		LPCTSTR sName,		// 이름
		CWnd* pCallback );	// call-back window
	virtual ~CPlayer() {}

	// 각 판마다 해야 하는 초기화 - 손에든 카드, 득점카드,
	// 번호 등을 다시 초기화 한다 (플레이어 번호는 ID로 초기화)
	void Reset();

public:
	// 이 플레이어는 사람인가
	virtual bool IsHuman() const					{ return false; }
	// 이 플레이어는 컴퓨터 AI 인가
	virtual bool IsComputer() const					{ return false; }
	// 이 플레이어는 네트워크인가
	virtual bool IsNetwork() const					{ return false; }

public:
	// 고유 번호 ( 7명의 플레이어+ 13명의 관전자중 순서 ) 를 얻는다
	int GetID() const								{ return m_nID; }
	void SetID( int nPlayer)						{ m_nID = nPlayer; }
	// 플레이어 번호 ( 실제 플레이 하는 플레이어 중 순서 )
	int GetPlayerNum() const						{ return m_nNum; }
	void SetPlayerNum( int nPlayer )				{ m_nNum = nPlayer; }
	// MFSM
	void SetCurrentMFSM( CMFSM* pMFSM )				{ m_pMFSM = pMFSM; }
	// 이름
	CString GetName() const							{ return m_sName; }
	void SetName( LPCTSTR sName )					{ m_sName = sName; }
	// 손에 든 카드와 득점한 카드
	CCardList* GetHand()							{ return &m_lcHand; }
	const CCardList* GetHand() const				{ return &m_lcHand; }
	CCardList* GetScore()							{ return &m_lcScore; }
	const CCardList* GetScore() const				{ return &m_lcScore; }
	// 돈
	int GetMoney() const							{ return m_nMoney; }
	void SetMoney( int nMoney )						{ m_nPrevMoney = m_nMoney; m_nMoney = nMoney; }
	int GetPrevMoney() const						{ return m_nPrevMoney; }
	// 현재 전적
	const GAME_RECORD& GetCurRecord() const			{ return m_recCur; }
	GAME_RECORD& GetCurRecord()						{ return m_recCur; }
	// 통산 전적
	const GAME_RECORD& GetAllRecord() const			{ return m_recAll; }
	GAME_RECORD& GetAllRecord()						{ return m_recAll; }

public:
	// 인터페이스 ( CPlay 확장 )

	// 하나의 게임이 시작됨
	// 이 게임에서 사용되는 상태 변수를 알려준다
	// 이 상태 변수는 CCard::GetState() 로도 알 수 있다
	// pState->nCurrentPlayer 값이 바로 자기 자신의
	// 번호이며, 이 값은 자신을 인식하는데 사용된다
	virtual void OnBegin( const CState* pState, CEvent* );

	// 6마에서 당선된 경우 한 사람을 죽여야 한다
	// 죽일 카드를 지정하면 된다 - 단 이 함수는
	// 반복적으로 호출될 수 있다 - 이 경우
	// CCardList 에 지금까지 실패한 카드의 리스트가
	// 누적되어 호출된다
	virtual void OnKillOneFromSix( CCard* pcCardToKill,
		CCardList* plcFailedCardsTillNow, CEvent* );

	// 공약을 듣는다
	// pNewGoal 과 state.goal 에 현재까지의 공약이 기록되어 있다
	// 패스하려면 nMinScore 를 0으로 세트
	// 특히 딜 미스를 원하는 경우 공약을 -1로 세트
	// pNewGoal.nFriend 는 사용하지 않는다
	virtual void OnElection( CGoal* pNewGoal, CEvent* );

	// 당선된 경우
	// pNewGoal 과 state.goal 에 현재까지의 공약이 기록되어 있다
	// pNewGoal 에 새로운 공약을 리턴하고 (nFriend포함)
	// acDrop 에 버릴 세 장의 카드를 리턴한다
	// * 주의 * 이 함수 이후에 한명이 죽어서 플레이어의
	// 번호(pState->nCurrentPlayer)가 변경되었을 수 있다
	virtual void OnElected( CGoal* pNewGoal, CCard acDrop[3], CEvent* );

	// 선거가 끝났고 본 게임이 시작되었음을 알린다
	// * 주의 * 이 함수 이후에 한명이 죽어서 플레이어의
	// 번호(pState->nCurrentPlayer)가 변경되었을 수 있다
	virtual void OnElectionEnd( CEvent* );

	// 카드를 낸다
	// pc 는 낼 카드, pe 는 pc가 조커인경우 카드 모양,
	// pc가 조커콜인경우 실제 조커콜 여부(0:조커콜 아님)
	// 그 외에는 무시
	virtual void OnTurn( CCard* pc, int* pe, CEvent* );

	// 한 턴이 끝났음을 알린다
	// 아직 state.plCurrent 등은 그대로 남아있다
	virtual void OnTurnEnd( CEvent* );

public:
	// 인터페이스 ( CMFSM 이 호출하는 Notify )

	// 한 판이 시작됨 - OnBegin 전, 아직 덱이 분배되지 않은 상태
	virtual void OnInit( CEvent* e )				{ e->SetEvent(); }
	// 2마에서 카드를 뽑음
	virtual void OnSelect2MA( int* selecting, CCardList* plCard, CEvent* e )	{ selecting, plCard, e->SetEvent(); }
	// 카드를 나눠 주고 있음 ( nFrom 에서 nTo 로 (-1은 중앙)
	// nCurrentCard 가 이동하였음 )
	// nMode  0 : 카드를 날리지 않고, 단지 전체 화면 갱신 필요
	//        1 : 카드 날리기
	//        2 이상 : Mo()->bShowDealing 에 관계 없이 꼭 날려야 함
	//        3 이상 : 속도를 너무 빠르게 하면 안됨
	//        9 이상 : 카드 선택
	//        10 : 주공의 Privilege 단계임, 속도는 느리게, 소리 연주
	virtual void OnDeal( int nFrom, int nTo, int nMode, int nCard, CEvent* e )	{ nFrom, nTo, nMode, nCard, e->SetEvent(); }
	// 다른 사람의 선거 결과를 듣는다
	virtual void OnElecting( int nPlayerID, int nKiruda,
		int nMinScore, CEvent* e )					{ nPlayerID, nKiruda, nMinScore, e->SetEvent(); }
	// 주공이 정해졌다
	virtual void OnElected( int nPlayerID, CEvent* e )	{ nPlayerID, e->SetEvent(); }
	// 카드를 골랐다.
	virtual void OnSelect2MA( int* selecting, CEvent *e )	{ selecting, e->SetEvent(); }
	// 주공이 다른 플레이어를 죽인다
	// bKilled : 참이면 실제로 죽였고, 거짓이면 헛다리 짚었다
	virtual void OnKillOneFromSix( CCard cKill,
		bool bKilled, CEvent* e )					{ cKill, bKilled, e->SetEvent(); }
	// 플레이어를 죽인 후 카드를 섞었다
	virtual void OnSuffledForDead( CEvent* e )		{ e->SetEvent(); }
	// 특권 모드가 다음과 같은 결과로 종료되었다
	virtual void OnPrivilegeEnd( CGoal* pNewGoal, CCard acDrop[3], CEvent* e ) { pNewGoal, acDrop, e->SetEvent(); }
	// 카드를 냈음을 알려준다
	// 모든 정보는 State 에 있다
	// nHandIndex 는 이 카드가 손에든 카드중 몇번째 인덱스의 카드였는가
	virtual void OnTurn( CCard c, int eff, int nHandIndex, CEvent* e )	{ c, eff, nHandIndex, e->SetEvent(); }
	// 턴이 종료되기 전에, 딴 카드를 회수하는 날리기 단계
	// cCurrentCard 가 현재 날아가는 카드
	virtual void OnTurnEnding( int nWinner, CEvent* e )		{ nWinner, e->SetEvent(); }
	// 프랜드가 밝혀 졌음
	virtual void OnFriendIsRevealed( int nPlayer, CEvent* e )	{ nPlayer, e->SetEvent(); }
	// 게임 종료 ( *pbCont 가 false 이면 세션 종료 )
	virtual void OnEnd( bool* pbCont, CEvent* e )			{ *pbCont = true; e->SetEvent(); }
	// nPlayer 의 작전 시작 ( 생각이 오래걸리는 작업 시작 )
	// nMode : 0 공약 생각중  1 특권모드 처리중  2 낼 카드 생각중
	virtual void OnBeginThink( int nPlayer, int nMode, CEvent* e )	{ nPlayer, nMode, e->SetEvent(); }
	// 프로그램 종료
	// 이 함수는 하나의 플레이어가 disconnect 할 때,
	// 또는 OnEnd 에서 한명의 플레이어라도 false 를
	// 리턴했을 때 호출된다 - Human 의 경우 세션을 종료한다
	// sReason : 종료되는 이유 (0 이면 즉시 종료)
	virtual void OnTerminate( LPCTSTR sReason )				{ sReason; }
	// 채팅 메시지 (bSource : 채팅창이 소스)
	virtual void OnChat( int nPlayerID, LPCTSTR sMsg,
						bool bSource )						{ nPlayerID, sMsg, bSource; }

protected:
	int m_nID;
	int m_nNum;
	int m_nMoney;
	int m_nPrevMoney;
	GAME_RECORD m_recCur;
	GAME_RECORD m_recAll;
	CString m_sName;
	CWnd* m_pwndCallback;
	CCardList m_lcHand;
	CCardList m_lcScore;
	CMFSM* m_pMFSM;
	CPlay* m_pPlay;
};

inline CPlayer::CPlayer( int nID, LPCTSTR sName, CWnd* pCallback )
{
	m_nID = nID;
	m_nNum = -1;
	m_nMoney = 0;
	m_nPrevMoney = 0;
	memset( &m_recCur, 0, sizeof(GAME_RECORD) );
	memset( &m_recAll, 0, sizeof(GAME_RECORD) );
	m_sName = sName;
	m_pwndCallback = pCallback;
	m_pMFSM = 0;
	m_pPlay = 0;
}

// 각 판마다 해야 하는 초기화 - 손에든 카드, 득점카드,
// 번호 등을 다시 초기화 한다 (플레이어 번호는 ID로 초기화)
inline void CPlayer::Reset()
{
	m_nNum = GetID();
	m_lcHand.RemoveAll();
	m_lcScore.RemoveAll();
}

inline void CPlayer::OnBegin( const CState* pState, CEvent* e )
{	if ( m_pPlay ) m_pPlay->OnBegin(pState);
	e->SetEvent(); }
inline void CPlayer::OnKillOneFromSix( CCard* pcCardToKill,
	CCardList* plcFailedCardsTillNow, CEvent* e )
{	if ( m_pPlay ) m_pPlay->OnKillOneFromSix(pcCardToKill,plcFailedCardsTillNow);
	e->SetEvent(); }
inline void CPlayer::OnElection( CGoal* pNewGoal, CEvent* e )
{	if ( m_pPlay ) m_pPlay->OnElection(pNewGoal);
	e->SetEvent(); }
inline void CPlayer::OnElected( CGoal* pNewGoal, CCard acDrop[3], CEvent* e )
{	if ( m_pPlay ) m_pPlay->OnElected(pNewGoal,acDrop);
	e->SetEvent(); }
inline void CPlayer::OnElectionEnd( CEvent* e )
{	if ( m_pPlay ) m_pPlay->OnElectionEnd();
	e->SetEvent(); }
inline void CPlayer::OnTurn( CCard* pc, int* pe, CEvent* e )
{	if ( m_pPlay ) m_pPlay->OnTurn(pc,pe);
	e->SetEvent(); }
inline void CPlayer::OnTurnEnd( CEvent* e )
{	if ( m_pPlay ) m_pPlay->OnTurnEnd();
	e->SetEvent(); }

#endif // !defined(AFX_PLAYER_H__E21B2746_CFA8_11D2_9810_000000000000__INCLUDED_)
