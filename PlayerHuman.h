// PlayerHuman.h: interface for the CPlayerHuman class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYERHUMAN_H__672332B3_DC38_11D2_983D_0000212035B8__INCLUDED_)
#define AFX_PLAYERHUMAN_H__672332B3_DC38_11D2_983D_0000212035B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBoardWrap;

// 사람의 입력을 기반으로 하는 플레이어 객체
// 적당한 리턴 값을 넘겨줄 수 있게 되면
// call-back 윈도우에 WM_PLAYER 메시지를 보내준다
// 이 때 LPARAM 값은 적절한 CMsg* 값
// 이 값은 사용자가 해제(delete) 해야 한다

class CPlayerHuman : public CPlayer
{
public:
	CPlayerHuman(
		int nID,		// 고유 번호 ( 6명의 플레이어중 순서 )
		LPCTSTR sName,	// 이름
		CWnd* pCallback );// call-back window
	virtual ~CPlayerHuman();

	// 이 플레이어는 사람인가
	virtual bool IsHuman() const					{ return true; }

	// CPlayer 인터페이스 구현

	// 한 판이 시작됨 - OnBegin 전, 아직 덱이 분배되지 않은 상태
	virtual void OnInit( CEvent* e );

	// 카드를 나눠 주고 있음 ( nFrom 에서 nTo 로 (-1은 중앙)
	// nCurrentCard 가 이동하였음 )
	// nMode  0 : 카드를 날리지 않고, 단지 전체 화면 갱신 필요
	//        1 : 카드 날리기
	//        2 이상 : Mo()->bShowDealing 에 관계 없이 꼭 날려야 함
	//        3 이상 : 속도를 너무 빠르게 하면 안됨
	//        9 이상 : 카드 선택
	//        10 : 주공의 Privilege 단계임, 속도는 느리게, 소리 연주
	virtual void OnDeal( int nFrom, int nTo, int nMode, int nCurrentCard, CEvent* e );

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
	// 5번 실패하면 (이 경우 알고리즘이 잘못되었거나
	// 사람이 잘 못 선택하는 경우) 임의로 나머지 5명 중
	// 하나가 죽는다 !
	virtual void OnKillOneFromSix( CCard* pcCardToKill,
		CCardList* plcFailedCardsTillNow, CEvent* );

	// 주공이 다른 플레이어를 죽인다
	// bKilled : 참이면 실제로 죽였고, 거짓이면 헛다리 짚었다
	virtual void OnKillOneFromSix( CCard cKill,
		bool bKilled, CEvent* e );

	// 7마에서 당선된 경우 두 사람을 죽여야 한다
	// 이 함수는 그 중 하나만 죽이는 함수로,
	// OnKillOneFromSix와 같다.
	// 5번 실패하면 (이 경우 알고리즘이 잘못되었거나
	// 사람이 잘 못 선택하는 경우) 임의로 나머지 5명 중
	// 하나가 죽는다 !
	virtual void OnKillOneFromSeven( CCard* pcCardToKill,
		CCardList* plcFailedCardsTillNow, CEvent* );

	// 주공이 다른 플레이어를 죽인다
	// bKilled : 참이면 실제로 죽였고, 거짓이면 헛다리 짚었다
	virtual void OnKillOneFromSeven( CCard cKill,
		bool bKilled, CEvent* e );

	// 공약을 듣는다
	// pNewGoal 과 state.goal 에 현재까지의 공약이 기록되어 있다
	// 패스하려면 nMinScore 를 0으로 세트
	// 특히 딜 미스를 원하는 경우 공약을 -1로 세트
	// pNewGoal.nFriend 는 사용하지 않는다
	virtual void OnElection( CGoal* pNewGoal, CEvent* );

	// 다른 사람의 선거 결과를 듣는다
	void OnElecting( int nPlayerID, int nKiruda,
		int nMinScore, CEvent* e );

	// 주공이 정해졌다
	virtual void OnElected( int nPlayerID, CEvent* e );

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

	// 턴이 종료되기 전에, 딴 카드를 회수하는 날리기 단계
	// cCurrentCard 가 현재 날아가는 카드
	virtual void OnTurnEnding( int nWinner, CEvent* e );

	// 한 턴이 끝났음을 알린다
	// 아직 state.plCurrent 등은 그대로 남아있다
	virtual void OnTurnEnd( CEvent* );

	// 카드를 냈음을 알려준다
	// 모든 정보는 State 에 있다
	// nHandIndex 는 이 카드가 손에든 카드중 몇번째 인덱스의 카드였는가
	virtual void OnTurn( CCard c, int eff, int nHandIndex, CEvent* e );

	// 프랜드가 밝혀 졌음
	virtual void OnFriendIsRevealed( int nPlayer, CEvent* e );

	// 게임 종료 ( *pbCont 가 false 이면 세션 종료 )
	virtual void OnEnd( bool* pbCont, CEvent* e );

	// nPlayer 의 작전 시작 ( 생각이 오래걸리는 작업 시작 )
	// nMode : 0 공약 생각중  1 특권모드 처리중  2 낼 카드 생각중
	virtual void OnBeginThink( int nPlayer, int nMode, CEvent* e );

	// 프로그램 종료
	// 이 함수는 하나의 플레이어가 disconnect 할 때,
	// 또는 OnEnd 에서 한명의 플레이어라도 false 를
	// 리턴했을 때 호출된다 - Human 의 경우 세션을 종료한다
	// sReason : 종료되는 이유 (0 이면 즉시 종료)
	virtual void OnTerminate( LPCTSTR sReason );

	// 채팅 메시지 (bSource : 채팅창이 소스)
	virtual void OnChat( int nPlayerID, LPCTSTR sMsg, bool bSource );

protected:
	// 헬퍼 멤버 & 함수

	// 현재 CBoardWrap
	CBoardWrap* m_pBoard;
	// 범용적으로 사용하는 이벤트 객체
	CEvent m_eTemp;
	// 현재 DGoal 이 선택한 goal 객체
	CGoal m_goal;
	// 현재 Board 에 선택된 카드를 보관하는 리스트
	CCardList m_clTemp;

	// 상태 얻기
	const CState* GetState();
	// 탈락시킬 카드를 추천한다
	int GetRecommendedKillCard( const CCardList* plDead );
	// 프랜드 카드를 추천한다
	int GetRecommendedFriend( const CCardList* plDeck );
	// p 에 있는 카드를 뷰에서 선택한다
	void SetCurrentSelectedCard( CCardList* p );
	// 현재 턴 상태에서 적절한 사운드를 연주한다
	void PlayTurnSound();
	// 조커, 조커콜을 처리한다 (취소시 true 리턴)
	bool ProcessSpecialCards( CCard c, int* eff );

	// 채팅창 핸들러
	static void ChatProc( LPCTSTR s, DWORD dwUser );
};

#endif // !defined(AFX_PLAYERHUMAN_H__672332B3_DC38_11D2_983D_0000212035B8__INCLUDED_)
