// BoardWrapper.h : Wrapper of CBoard
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BOARDWRAP_H__6433F789_C1B5_11D2_97F2_000000000000__INCLUDED_)
#define AFX_BOARDWRAP_H__6433F789_C1B5_11D2_97F2_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMFSM;
class DSB;
class CBmpMan;

/////////////////////////////////////////////////////////////////////////////
// CBoardWrap
// CBoard의 Wrapper Window

class CBoardWrap
{
public:
	// 기본 함수
	CBoardWrap( CWnd* pWnd );
	virtual ~CBoardWrap();

	operator HWND()									{ return (HWND)*m_pBoard; }
	operator CWnd*()								{ return m_pBoard; }
	void GetClientRect( LPRECT prc ) const;

	// 랩퍼 얻기
	CBoardWrap* GetWrapper();

public:
	// 현재 CBoard 와 연관된 Mighty FSM 을 세트한다
	// 이것을 세트 하는 순간 정상적인 윈도우의 동작이 시작된다
	void SetMFSM( CMFSM* pMFSM );
	// 현재의 BmpManager 를 얻는다
	CBmpMan* GetBmpMan();
	// 플레이어의 이름을 옵션에서 다시 로드한다
	void ReloadPlayerNames();

public:
	// 게임 화면의 DSB를 갱신한다
	// 지정된 영역의 버퍼를 다시 그린 후 그 범위의 실제 윈도우도
	// 무효화 시킨다
	// bUpdateNow : 즉시 윈도우를 Update 한다
	void UpdateDSB( int x, int y, int cx, int cy, bool bUpdateNow = false );
	void UpdateDSB( LPCRECT prc = 0, bool bUpdateNow = false );
	// 플레이어나 바닥 정보를 갱신한다
	// 이 위의 DSB 들도 함께 갱신된다 ( UpdateDSB 를 내부적으로 호출한다 )
	// nPlayer 가 -1 이면 바닥 정보를 갱신
	// nPlayer 가 -2 이면 모든 정보를 갱신
	// prc 가 0 이면 그 플레이어의 모든 정보를 갱신
	//        그렇지 않으면 그 영역만을 갱신
	// bUpdateNow : 즉시 윈도우를 Update 한다
	// bSelectionRegion : 이 값이 참이면 0 번 플레이어의 선택 영역까지 함께 갱신한다
	void UpdatePlayer( int nPlayer, int x, int y, int cx, int cy, bool bUpdateNow = false, bool bSelectionRegion = false );
	void UpdatePlayer( int nPlayer, LPRECT prc = 0, bool bUpdateNow = false, bool bSelectionRegion = false );

public:
	// 특수 효과

	// 차례 사각형을 해당 플레이어 위치로 이동 (-1 은 없앰)
	void SetTurnRect( int nPlayer );

	// 지정한 시간동안 가만히 있다가 지정한 이벤트를 세트한다
	void WaitWhile( int nMillisecond, CEvent* pEvent );

	// 이 함수를 호출하면 마우스 클릭, 또는 채팅 창으로
	// 사용자가 카드를 선택할 때 까지 대기했다가
	// 지정한 이벤트를 세트한다
	void WaitSelection( CEvent* pEvent, long* pSelectedCard );
	// 위의 상태를 취소한다
	void CancelSelection();

	// 카드를 날리는 에니메이션 후
	// 지정한 이벤트를 세트한다
	// (그림자 효과는 중앙을 낮은 지점으로 생각)
	void FlyCard(
		int nCard,			// 날릴 카드
		int nSpeed,			// 속도 ( 0 : 표준 1 : 매우 빠름 2 : 빠름 )
		int nFrom,			// 시작 위치 ( 0 가운데 1 점수영역 2 손에든 카드 3 따는곳 4 덱의 꼭대기!!)
		int nFromPlayer,	// 시작 위치 ( 플레이어 번호, nFrom!=0일때만 사용  )
		int nFromIndex,		// 몇 번째 카드인가 (-1이면 정 가운데)
		int nTo,			// 끝 위치
		int nToPlayer,
		int nToIndex,
		CEvent* pEvent );

	// 카드 섞는 에니메이션 후
	// 지정한 이벤트를 세트한다
	// 음향은 IDW_SUFFLE 을 연주한다
	void SuffleCards( int nCount /*섞는 회수*/, CEvent* pEvent );

	// 지정한 플레이어를 Deal-Miss 상태로 보여준다
	// nPlayerID 가 -1 인 경우 그것을 해제한다
	// 실제로 화면을 Update 하지는 않는다
	void SetDealMiss( int nPlayerID );
	int GetDealMiss();

	// Privilege 시 해당 인덱스의 카드를 select/deselect 한다
	// 실제로 화면을 Update 하지는 않는다
	void SetSelection( int nIndex, bool bSelect = true );
	bool GetSelection( int nIndex );
	// 전체 버전
	void SetSelection( bool bSelect = true );
	int GetSelectionCount();

// DSB ( shade-box ) 관련 함수
public:
	void AddDSB( DSB* pDSB );		// DSB 리스트에 추가
	void SetDSBTimer( DSB* pDSB );	// DSB 의 타이머를 재설정 (DSB내부 타이머로)
	bool RemoveDSB( DSB* pDSB );	// DSB 리스트에서 삭제
	int GetDSBUnit();				// DSB 에서 사용하는 기준 유닛 크기 (최소폰트의 높이임)
	// 글꼴 ( 0 small 1 medium 2 big, 음수이면 -nSize-1 의 Fixed )
	CFont* GetFont( int nSize );
	// 이 스트링의 화면에서의 크기를 구한다 (지정한 폰트 사용)
	CSize GetTextExtent( int nSize, LPCTSTR s );
	// DSB 를 출력하기 위한 대략적인 플레이어 위치를 계산
	// 내부적으로 그 플레이어의 '점수영역'을 리턴한다
	// ( nPlayerNum 이 -1 이면 전체 클라이언트의 한 가운데 )
	// pbVert 에는 영역이 세로로 긴가 여부를 리턴한다
	RECT CalcPlayerExtent( int nPlayerNum, bool* pbVert = 0 );
	// 해당 플레이어 자리에 채팅 DSB 를 띄운다
	// nPlayerID : 플레이어 ID
	// sMsg    : 메시지
	void FloatSayDSB( int nPlayerID, LPCTSTR sMsg );
	// 해당 플레이어 자리에 공약 DSB 를 띄운다
	// nPlayer : 플레이어 번호 ( -1 이면 현재의 DSB 를 삭제 )
	// goal    : 그 플레이어의 공약
	// goal.nMinScore 가 0 이면 출마 포기, -1 이면 딜미스
	void FloatGoalDSB( int nPlayerID, const CGoal* pGoal );
	// 지정한 DSB 를 선거 DSB 로 지정한다
	// 한번에 하나의 선거 DSB 만 화면에 표시되므로
	// 이전의 선거 DSB 는 제거된다
	// pDSB 가 0 이면 이전의 선거 DSB 만 제거된다
	void SetElectionDSB( DSB* pDSB );

protected:
	friend class CBoard;
	CWnd* m_pBoard;

	// 함수 이름
	enum WRAPPER_FUNC {
		wfGetWrapper, wfSetMFSM, wfGetBmpMan, wfReloadPlayerNames,
		wfUpdateDSB_1, wfUpdateDSB_2,
		wfUpdatePlayer_1, wfUpdatePlayer_2, wfSetTurnRect, wfWaitWhile,
		wfWaitSelection, wfCancelSelection, wfFlyCard,wfSuffleCards,
		wfSetDealMiss, wfGetDealMiss, wfSetSelection_1, wfSetSelection_2,
		wfGetSelection, wfGetSelectionCount, wfAddDSB, wfSetDSBTimer,
		wfRemoveDSB, wfGetDSBUnit, wfGetFont, wfGetTextExtent,
		wfCalcPlayerExtent, wfFloatSayDSB, wfFloatGoalDSB, wfSetElectionDSB
	};

	// 범용적인 함수 인자 공용체
	union WRAPPER_ARG {

		struct { void* p; bool b; } PB;
		struct { int i4[4]; bool b; } I4B;
		struct { int i5[5]; bool b2[2]; } I5B2;
		struct { int i; void* p; bool b2[2]; } IPB2;
		struct { int i; void* p; } IP;
		struct { void* p2[2]; } P2;
		struct { int i8[8]; void* p; } I8P;
		struct { int i; } I;
		struct { int i; bool b; } IB;
		struct { bool b; } B;
		struct { void* p; } P;
		struct { void* p2[2]; int i; } P2I;
	};
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BOARDWRAP_H__6433F789_C1B5_11D2_97F2_000000000000__INCLUDED_)
