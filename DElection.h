// DElection.h: interface for the all election-related DSB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DELECTION_H__B24183A6_F55C_11D2_9894_000000000000__INCLUDED_)
#define AFX_DELECTION_H__B24183A6_F55C_11D2_9894_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMFSM;

// 선거에 관련된 DSB 들 모음


/////////////////////////////////////////////////////////////////////////////
// 선거 공약 DSB
// 이 DSB 는 당선자를 정할 때도 사용되며 당선자가
// 공약을 변경할 때도 사용된다

class DGoal  : public DSB
{
public:
	DGoal( CBoardWrap* pBoard );
	virtual ~DGoal();

	// bPrivilege : 당선자가 골을 수정하는 중인가?
	// pMFSM      : 사람 플레이어의 손에 든 카드와
	//              목표 점수 validity 를 테스트 하기 위해 전달한다
	void Create( bool bPrivilege,
		const CState* pState, CEvent* pEvent, CGoal* pResult );

protected:
	// 초기화 ( 생성된 후 호출됨 )
	virtual void OnInit();

	// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
	// 오버라이드 하여 구현해야 한다
	// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
	virtual void OnDraw(
		CDC* pDC,
		bool bBrushOrigin );

	// 핫 스팟을 클릭할 때 불리는 함수
	// LPVOID 값이 0 이면 종료 (X자를 클릭) 요청
	virtual void OnClick( LPVOID pVoid );

	// 확인 소리를 안나게
	virtual void OnClickSound() {}

protected:
	// 내부 데이터 & 함수

	// Privilege 모드인가
	bool m_bPrivilege;
	// State
	const CState* m_pState;
	// 플레이어의 손에 든 카드
	const CCardList* m_plHand;
	// 목표점수 사각형 위치
	CRect m_rcMinScore;
	// shape 사각형의 표시 속성 배열
	int m_aatdShape[5][2];
	// 상하 화살표의 표시 속성, 표시 색상 배열
	// [0][0] = [상화살표][비선택]
	int m_aatdArrow[2][2];
	COLORREF m_aacolArrow[2][2];
	// 확정 버튼의 색상 & 속성
	COLORREF m_colConfirm;
	int m_tdConfirm, m_tdConfirmSel;
	// 현재 최소 점수
	int m_nMinScore;
	// 현재 표시되는 목표 점수 스트링
	CString m_sMinScore;
	// 현재의 선택된 기루다
	int m_nKiruda;
	// 출마 포기 / 딜미스
	bool m_bDealMiss;

	// 주어진 모양으로 기루다를 바꾼다
	void ChangeKiruda( int nShape );
	// 주어진 값으로 최소 점수를 바꾼다
	// (valid 한 값으로 재조정됨)
	void ChangeMinScore( int nMinScore );

	// Toggler 의 종료 플랙
	volatile bool m_bTerminateToggler;
	volatile bool m_bTogglerTerminated;
	CEvent m_ev;	// 이벤트

	// Board 의 Selection 을 관리할 쓰레드
	// 카드 선택을 토글하는 역할만을 한다
	static UINT TogglerProc( LPVOID _pThis );
};


/////////////////////////////////////////////////////////////////////////////
// 카드 죽이기 DSB

class DKill  : public DSB
{
public:
	DKill( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DKill() {}

	// pcToKill : 이 DSB 가 결과 카드를 저장할 장소 (결과-값 독립변수)
	// plDead   : 지금까지 헛다리 짚은 카드들
	// plHand   : 플레이어의 손에 든 카드
	void Create( CEvent* pEvent, CCard* pcToKill,
		const CCardList* plDead, const CCardList* plHand );

protected:
	// 초기화 ( 생성된 후 호출됨 )
	virtual void OnInit();

	// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
	// 오버라이드 하여 구현해야 한다
	// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
	virtual void OnDraw(
		CDC* pDC, bool bBrushOrigin );

	// 핫 스팟을 클릭할 때 불리는 함수
	// LPVOID 값이 0 이면 종료 (X자를 클릭) 요청
	virtual void OnClick( LPVOID pVoid );

	// 확인 소리를 안나게
	virtual void OnClickSound() {}

protected:
	// 내부 데이터 & 함수

	// 현재 선택된 카드
	CCard* m_pcToKill;
	// 지금까지의 카드들
	const CCardList* m_plDead;
	// 손에 든 카드들
	const CCardList* m_plHand;
	// 현재 선택된 모양
	int m_nShape;
	// 선택된 모양의 속성
	int m_aatdShape[4][2];
	// m_nShape 를 특정 값으로 세트하면서 m_aatdShape 를
	// 재 조정한다
	void SetCurShape( int nShape );
	// m_pcToKill 값을 변경하고 그 결과를 화면에 업데이트 한다
	void SetCurCard( CCard c );
	// 현재 카드를 그릴 영역을 리턴한다
	void CalcCurCardRect( LPRECT pRc );
};


/////////////////////////////////////////////////////////////////////////////
// 당선자 발표 DSB

class DMasterDecl  : public DSB
{
public:
	DMasterDecl( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DMasterDecl() {}

	// bTemp       : true 이면 이 DSB 는 최초의 당선자 발표 DSB
	//               false 이면 이 DSB 는 최종 확정 DSB
	// bHuman      : 사람이 당선자인가
	// bToKill     : 다른 플레이어를 죽여야 하는가
	// bUseFriend  : 프랜드 제도가 있는가
	// sMasterName : 당선자 이름
	// goal        : 공약
	void Create( bool bTemp, bool bHuman, bool bToKill,
		bool bUseFriend, int nDefaultMinScore, const CGoal& goal,
		LPCTSTR sMasterName, LPCTSTR sFriendName, int nTimeOut );

protected:
	// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
	// 오버라이드 하여 구현해야 한다
	// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
	virtual void OnDraw(
		CDC* pDC,
		bool bBrushOrigin );

	// 아무데나 클릭해도 바로 사라지도록 한다
	virtual bool DispatchHotspot( int x, int y, bool bCheckOnly );

protected:
	// 내부 데이터 & 함수

	bool m_bTemp;
	bool m_bHuman;
	bool m_bToKill;
	bool m_bUseFriend;
	int m_nDefaultMinScore;
	CString m_sMasterName;
	CString m_sFriendName;
	CGoal m_goal;
};


/////////////////////////////////////////////////////////////////////////////
// 탈락자 발표 메시지 DSB

class DDeadDecl  : public DSB
{
public:
	DDeadDecl( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DDeadDecl() {}

	// sMaster  : 당선자 이름
	// nCard    : 화면에 표시될 카드
	// bFail    : 이 카드는 탈락 실패 카드인가 (메시지가 조금 달라짐)
	void Create( LPCTSTR sMaster, int nCard,
		bool bFail, int nTimeOut );

protected:
	// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
	// 오버라이드 하여 구현해야 한다
	// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
	virtual void OnDraw(
		CDC* pDC, bool bBrushOrigin );

protected:
	// 내부 데이터 & 함수

	CString m_sMaster;
	int m_nCard;
	bool m_bFail;
};


/////////////////////////////////////////////////////////////////////////////
// 프랜드 DSB
// 이 DSB 는 물론 Board 의 현재 선택 카드와 상호 작용한다
// 즉, 또하나의 쓰레드가 카드 선택 이벤트를 감시하며,
// Board 의 선택 카드 업데이트를 전담한다

class DFriend  : public DSB
{
public:
	DFriend( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DFriend() {}

	// pnFriend : 결과값 (의미는 CGoal::nFriend 참조) (결과-값 독립변수)
	void Create( CEvent* pEvent, int* pnFriend, const CState* pState );

protected:
	// 초기화 ( 생성된 후 호출됨 )
	virtual void OnInit();

	// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
	// 오버라이드 하여 구현해야 한다
	// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
	virtual void OnDraw(
		CDC* pDC, bool bBrushOrigin );

	// 핫 스팟을 클릭할 때 불리는 함수
	// LPVOID 값이 0 이면 종료 (X자를 클릭) 요청
	virtual void OnClick( LPVOID pVoid );

	// 확인 소리를 안나게
	virtual void OnClickSound() {}

protected:
	// 내부 데이터 & 함수

	// 현재 선택된 프랜드
	int* m_pnFriend;
	// 손에 든 카드들
	CCardList m_lHand;
	// 현재 상태
	const CState* m_pState;
	// 현재 선택된 모양
	int m_nShape;
	// 선택된 모양의 속성
	int m_aatdShape[4][2];
	// 선택된 프랜드 카드의 색상, 속성
	COLORREF m_acolFriend[6+MAX_PLAYERS];
	int m_atdFriend[6+MAX_PLAYERS];
	int m_atdFriendSel[6+MAX_PLAYERS];

	// m_nShape 를 특정 값으로 세트하면서 m_aatdShape 를
	// 재 조정한다
	void SetCurShape( int nShape );
	// m_pnFriend 값을 변경하고 그 결과를 화면에 업데이트 한다
	void SetCurFriend( int nFriend );
	// 현재 카드를 그릴 영역을 리턴한다
	void CalcCurCardRect( LPRECT pRc );
};


#endif // !defined(AFX_DELECTION_H__B24183A6_F55C_11D2_9894_000000000000__INCLUDED_)
