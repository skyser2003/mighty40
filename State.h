// State.h: interface for the CState class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STATE_H__BE37E2CA_C23D_11D2_97F3_000000000000__INCLUDED_)
#define AFX_STATE_H__BE37E2CA_C23D_11D2_97F3_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifndef MAX_PLAYERS
#	define MAX_PLAYERS		7
#endif
#ifndef MAX_CONNECTION
#	define MAX_CONNECTION	20
#endif

#ifndef MIGHTY_EXPORT
#	ifdef _WINDLL
#		define MIGHTY_EXPORT __declspec(dllimport)
#	else
#		define MIGHTY_EXPORT __declspec(dllexport)
#	endif
#endif

struct CRule;
class CPlayer;
class CCard;
class CCardList;


// 마이티 게임의 여러 상태 ( FSM 에서 사용 )
enum MIGHTY_STATE {

	msReady,					// 모든 데이터가 만들어지고 새 판이 초기화 됨
	msDeal2MA,					// 2마에서 카드를 가짐
	msElection,					// 선거가 시작됨
	msPrivilege,				// 주공이 나머지 3장을 가짐 ( 가지기 직전 기루다 바꾸는 상황 포함 )
	msTurn,						// 카드를 내는 중
	msEnd,						// 게임 끝, 점수가 계산되고 출력됨
};


// 주공이 선언한 현재 목표

struct CGoal
{
	// 기루다 ( 노기루다일때 0 )
	int nKiruda;
	// 최소득점 ( 이 값이 0이면 아직 공약이 만들어지지 않은 것이다 )
	int nMinScore;
	// 프랜드 ( <0 -(플레이어번호)-1, <54&&>0 카드, ==0 노프랜드, == 100 초구 )
	int nFriend;
};


// 게임의 현재 상태를 나타내는 구조체

struct MIGHTY_EXPORT CState  
{
	// 현재 상태

	MIGHTY_STATE state;


	// 게임 범위

	// 사용되는 규칙
	CRule* pRule;
	// 지금까지의 게임 회수
	int nGameNum;
	// 모든 플레이어들 ( 최대 20명=MAX_CONNECTION )
	CPlayer* apAllPlayers[MAX_CONNECTION];
	// 덱
	CCardList& lDeck;
	// 바닥카드들
	CCardList& lCurrent;
	// 히스토리 ( 득점, 도움, 실점, 점유율 )
	int aanHistory[4][MAX_PLAYERS];
	// 자리 섞는 옵션에 필요한 보내줄 위치 정보 ( v4.0 : 2011.2.27 )
	int changed[MAX_PLAYERS];

	// 판 범위

	// 실제로 게임을 하는 플레이어들 ( 최대 7명 )
	CPlayer* apPlayers[MAX_CONNECTION];
	// 실제로 게임을 하는 플레이어들의 수
	int nPlayers;
	// 각 플레이어의 선거 기권 여부
	bool abGiveup[MAX_PLAYERS];
	// 6,7 마에서 죽는 카드 불렀을 때 실패한 카드들
	CCardList& lDead;
	// 주공 ( 또는 주공 예정자 )
	int nMaster;
	// 프랜드 ( 없을때 -1, 초구프랜드이고 결정 안됐을때 -2 )
	int nFriend;
	// 6,7 마 탈락자ID ( 아직 아무도 탈락 안됐을때 -1 )
	int nDeadID[MAX_PLAYERS-5];
	// 프랜드가 밝혀졌는가
	bool bFriendRevealed;
	// 현재 목표
	CGoal goal;
	// 버릴 세 장의 카드
	int acDrop[3];
	// 버려진 덱에 들어 있는 점수 카드 수
	int nThrownPoints;
	// 조커가 나왔는가 ( 이 턴 바닥에 있는 카드 포함 )
	bool bJokerUsed;
	// 지금까지 등장한 카드들 플랙 ( 이 턴 바닥에 있는 카드 제외 )
	// SPADE~CLOVER 까지 0 번 비트(TWO)부터 11(KING), 12번 비트(ACE) 까지
	long anUsedCards[4];
	// 바닥에 있는 카드 플랙 (anUsedCards 와 OR 가능)
	long anUsingCards[4];

	// 턴 범위

	// 현재 턴 ( 0 ~ LAST_TURN ), -1 이면 선거중
	int nTurn;
	// 시작 플레이어
	int nBeginer;
	// 조커콜 효력중
	bool bJokercallEffect;
	// 조커 모양
	int nJokerShape;

	// 카드 범위

	// 현재 선택한 카드 ( 턴에서 쓰거나 죽일 때 )
	int cCurrentCard;
	// 현재 플레이어
	int nCurrentPlayer;


	// 헬퍼 함수들

	// 생성자/소멸자
	CState();
	virtual ~CState();
	// 각 판 범위의 데이터들을 초기화 한다 (프랜드 등)
	// 단, pRule, apPlayers[] 는 초기화 하지 않는다 (미리 정해져있어야 함)
	void InitStageData(	int _nGameNum, int _nBeginer = 0 );
	// 자리를 섞고 딜러 위치를 정한다
	void SuffleSeat( int& nBeginer );
	// 매턴의 끝 (lCurrent 가 사라지기 전) 에서 호출하여 히스토리 정보를 유지한다
	void WriteHistory();

	// 현재 goal 에 대해서, 다음의 기루다와
	// 최소 점수의 조합이 가능한가를 리턴한다
	// nNewMinScore 가 0 이면 포기 가능 여부를, -1 이면
	// DealMiss 가능 여부를 묻는다
	bool IsValidNewGoal( int nNewKiruda, int nNewMinScore ) const;

	// 새로운 프랜드 후보 값을 주면 적합한 경우 그 값을 그대로,
	// 아니면 0(노프랜드)를 리턴한다
	int GetValidFriend( int nFriend ) const;

	// 주어진 카드패가 딜미스인가 검토한다
	bool IsDealMiss( const CCardList* pl ) const;

	// 해당 플레이어가 여당인가를 판단한다
	bool IsDefender( int nPlayer ) const;

	// 현재 조커콜이 조커콜 효력을 낼 수 있는지를 검사한다
	bool IsEffectiveJokercall() const;

	// 주어진 카드 리스트에 대해 승자 플레이어 번호를 알아낸다
	// 음의 값은 현재 state 값으로 지정한다
	// pbWinnerIsDef : 승자가 여당인가 여부를 리턴
	int GetWinner( const CCardList* pList,
		bool* pbWinnerIsDef = 0,
		int nJokerShape = -1, int bJokercallEffect = -1,
		int nPlayers = -1, int nTurn = -1,
		int nBeginer = -1, int nMaster = -1, int nFriend = -1 ) const;

	// 이 게임은 네트워크를 동반한 게임인가
	bool IsNetworkGame() const;
};

#endif // !defined(AFX_STATE_H__BE37E2CA_C23D_11D2_97F3_000000000000__INCLUDED_)
