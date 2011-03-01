// Board.h : interface of the CBoard class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BOARD_H__6433F788_C1B5_11D2_97F2_000000000000__INCLUDED_)
#define AFX_BOARD_H__6433F788_C1B5_11D2_97F2_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMFSM;
class DSB;
class DSay;
class CBoardWrap;


/////////////////////////////////////////////////////////////////////////////
// CBoard window
// 마이티 게임의 메인 윈도우

class CBoard : public CWnd
{
public:
	// 기본 함수
	CBoard();
	virtual ~CBoard();

	// 랩퍼 얻기
	CBoardWrap* GetWrapper()						{ return m_pWrap; }

protected:
	// dtor 에서 호출되며, 할당된 자원을 반환한다
	void CleanUp();
	// 현재 CBoard 와 연관된 Mighty FSM 을 세트한다
	// 이것을 세트 하는 순간 정상적인 윈도우의 동작이 시작된다
	void SetMFSM( CMFSM* pMFSM );
	// 현재의 BmpManager 를 얻는다
	CBmpMan* GetBmpMan()							{ return &m_bm; }
	// 플레이어의 이름을 옵션에서 다시 로드한다
	void ReloadPlayerNames();

protected:
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
	// bSelectionRegion : 이 값이 참이면 0 번 플레이어의 선택 영역, 점수 영역까지 함께 갱신한다
	void UpdatePlayer( int nPlayer, int x, int y, int cx, int cy, bool bUpdateNow = false, bool bSelectionRegion = false );
	void UpdatePlayer( int nPlayer, LPRECT prc = 0, bool bUpdateNow = false, bool bSelectionRegion = false );

protected:
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

	// 카드를 날리는 애니메이션 후
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

	// 카드 섞는 애니메이션 후
	// 지정한 이벤트를 세트한다
	// 음향은 IDW_SUFFLE 을 연주한다
	void SuffleCards( int nCount /*섞는 회수*/, CEvent* pEvent );

	// 지정한 플레이어를 Deal-Miss 상태로 보여준다
	// nPlayerID 가 -1 인 경우 그것을 해제한다
	// 실제로 화면을 Update 하지는 않는다
	void SetDealMiss( int nPlayerID )				{ m_nDealMiss = nPlayerID; }
	int GetDealMiss() const							{ return m_nDealMiss; }

	// Privilege 시 해당 인덱스의 카드를 select/deselect 한다
	// 실제로 화면을 Update 하지는 않는다
	void SetSelection( int nIndex, bool bSelect = true ) { ASSERT(nIndex>=0); m_abSelected[nIndex] = bSelect; }
	bool GetSelection( int nIndex ) const				{ ASSERT(nIndex>=0); return m_abSelected[nIndex]; }
	// 전체 버전
	void SetSelection( bool bSelect = true )		{ for ( int i = 0; i < sizeof(m_abSelected)/sizeof(bool); i++ ) m_abSelected[i] = bSelect; }
	int GetSelectionCount() const					{ int n = 0; for ( int i = 0; i < sizeof(m_abSelected)/sizeof(bool); i++ ) n += m_abSelected[i] ? 1 : 0; return n; }

// DSB ( shade-box ) 관련 함수
protected:
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

// Internal Helpers
protected:
	// 타이머 id
	enum TIME_ID {
		tiDSB = 1000,			// DSB 타이머들의 베이스 값
	};

protected:
	// 레이 아웃 관련 함수들

	// 화면상의 각 사각형 좌표를 얻는다
	// nPlayers   : 플레이 하는 플레이어의 수 ( 1 ~ 6 )
	// type       : CR_CENTER 가운데 CR_SCORE 점수영역 CR_HAND 손에든 카드 CR_HIDDEN 따는곳
	// nPlayer    : 어떤 플레이어에 대한 영역인가 (nType!=0일때만 사용)
	// nCardWidth : 카드의 가로 크기
	// nCardHeight: 카드의 세로 크기
	// nIndex, nAll : 사각형 안에 들어 있는 각 아이템의 위치를
	//                전체(nAll)중의 몇번째(nInex)인가에 대한 값으로
	//                리턴한다, nIndex == -1 이면 전체 사각형을 리턴
	// pbVertical : 0 이 아니면 이 사각형이 세로로 긴 사각형인지를 리턴한다
	// pbDirection: 0 이 아니면 방향을 리턴 ( true:위에서 아래,왼쪽에서 오른쪽 false:그 반대)
	enum CR_TYPE { CR_CENTER, CR_SCORE, CR_HAND, CR_HIDDEN };
	CRect CalcRect( int nPlayers, CR_TYPE type, int nPlayer, 
		int nCardWidth, int nCardHeight,
		int nIndex = -1, int nAll = 0,
		bool* pbVertical = 0, bool* pbDirection = 0 ) const;

	// 각 플레이어의 이름 위치를 구한다
	CRect CalcNameRect( int nPlayers, int nPlayer,
		CDC* pDC, LPCTSTR sName, CFont* pFont ) const;

	// 현재 화면 크기에 대한 적당한 카드 줌 크기를 얻는다
	CSize GetCardZoom( bool bUseZoom ) const;

	// 현재 화면 크기에 대한 적당한 폰트들을 새로 만든다
	void CreateFont();

protected:
	// 그리기 관련 함수들

	// 그려질 때 특별한 카드들에 대한 카드 이동량
	// (선택된 카드나 마우스 밑의 카드는 조금 들려짐)
	enum {
		CARD_SHIFT_RATIO_OF_MOUSE_OVER = 20,// 카드의 1/20
		CARD_SHIFT_RATIO_OF_SELECTION = 5	// 카드의 1/5
	};

	// 배경, 손에 든 카드, 득점, 가운데 놓인 카드를 모두 그린다
	void DrawBoard( CDC* pDC, LPCRECT prc );

	// DSB 를 그린다
	// Board 와 같은 좌표계에 그려주지만
	// xOff 와 yOff 로 그려주는 옵셋을 변경할 수 있다
	void DrawDSB( CDC* pDC, bool bBrushOrigin,
		int x, int y, int cx, int cy,
		int xOff = 0, int yOff = 0 );

	// 플레이어의 손에 든 카드를 그린다
	void DrawHand(
		CDC* pDC,
		int nPlayer, int nPlayers,			// 플레이어/플레이어수
		const CCardList* pList,				// 플레이어의 카드
		int x, int y, int cx, int cy,		// 범위
		int xOff, int yOff,					// 실제로 그려질때의 옵셋 변화
		bool bReveal,						// 앞면을 보일것인가
		int nMouseOver,						// 마우스가 위치하는 카드 인덱스
		bool* abSelected = 0,				// 현재 선택된 카드들
		bool bScore = false );				// 손에 든 카드가 아니라 점수 카드를 그린다

	// 가운데 쌓인 카드를 그린다
	void DrawCenter(
		CDC* pDC,
		int nPlayers,						// 플레이어수
		const CCardList* pList,				// 그릴 카드
		int nBeginer,						// 시작 플레이어(아래0부터 시계방향)
		int x, int y, int cx, int cy,		// 그릴 영역
		int xOff, int yOff );				// 실제로 그려질 때의 옵셋 변화

	// 이름을 그린다
	void DrawName(
		CDC* pDC,
		int nPlayers,						// 플레이어수
		LPCTSTR sName,						// 이름
		int nPlayer,						// 그릴 플레이어
		COLORREF col, COLORREF colShadow,	// 글자 색상, 그림자 색상
		CFont* pFont );						// 폰트 (0 이면 시스템 폰트 사용)

	// 가운데 부분의 애니메이션
	// 크기는 (카드폭)*2 X (카드높이)*1.5
	// x, y 는 애니메이션의 한 가운데 위치
	// szCard 는 카드 크기
	// nCount 카드 장수 (nStep==0 일때만 사용)
	// nStep
	// 0 : 가운데 카드가 쌓인 모양
	// 1 : 카드가 양쪽으로 분리되는 중
	// 2 : 카드가 양쪽으로 분리되었음
	// 3 : 카드가 가운데로 합쳐지는 중
	void DrawCenterAnimation( CDC* pDC, bool bBrushOrigin,
		CSize szCard,
		int x, int y, int nStep,
		int nCount = -1 );

	// 카드를 날린다 - 이 함수는 위의 FlyCard 의 저수준 버전
	// 원하는 옵션을 주면 카드를 날리기 시작하면서
	// 핸들을 리턴한다 - 이 핸들을 다시
	// 함수의 인자로 전달하면 다음 핸들을
	// 리턴하면서 조금씩 카드 날리기를 진행한다.
	// 날리기가 끝났으면 0 을 리턴한다
	// 윈도우가 닫혔으면 언제든 자원을 해제하고 0 을 리턴한다
	volatile void* FlyCard(
		volatile void*& h,		// 핸들
		int nCard,				// 날릴 카드
		const CRect& rcBegin,	// 시작되는 카드 사각형
		const CPoint& ptEnd,	// 끝나는 위치 ( 사각형의 좌상단 )
		int nStates,			// 몇번만에 날리기 완료하는가
		int nShadeMode );		// 그림자 효과 (0:아래에서 아래로 1:위에서 아래로
								//              2:아래에서 위로 3:위에서 위로)
	volatile void* FlyCard( volatile void*&, int nStep = 1 );

	// 카드를 섞는 애니메이션을 보인다
	// 이 함수는 위의 SuffleCards 의 저수준 버전으로
	// DrawCenterAnimation 함수를 사용한다
	// FlyCard 와 같은 방식으로 호출한다
	// 윈도우가 닫혔으면 언제든 자원을 해제하고 0 을 리턴한다
	volatile void* SuffleCards(
		volatile void*& h,		// 핸들
		int nCount,				// 섞을 회수
		SIZE szCard );			// 카드 크기
	volatile void* SuffleCards( volatile void*& );

	// FlyCard 보조함수- 주어진 정보로 시작 사각형이나 종료 사각형을 계산한다
	// 전달 인자는 FlyCard 와 같다
	RECT CalcCardRectForFlyCard( int nType, int nPlayer, int nIndex );

protected:
	// 잡다한 기타 함수들

	// 플레이어번호를 매핑한다 - 아래쪽 플레이어부터
	// 시계 방향으로 0 ~ 7(v4.0) 의 표현 방법(절대값)을
	// 실제 플레이어 번호(논리값)로 매핑하거나 역변환 한다
	int MapAbsToLogic( int nAbsPlayer ) const;
	int MapLogicToAbs( int nLogicalPlayer ) const;
	// 사람이 든 카드 중에서 지정한 인덱스에 있는 카드의
	// 실제 사각형 위치를 구함 ( 쉬프트 된 카드 위치까지 고려 )
	bool CalcRealCardRect( LPRECT prc, int index ) const;
	// 사람이 든 카드 중에서 지정한 포인트에 있는
	// 카드의 POSITION 을 리턴
	POSITION CardFromPoint( int x, int y ) const;
	// 지정된 위치의 카드를 들어 올린다
	// (m_abSelected 나 m_nMouseOver 값을 바꾼 후 Update 한다)
	// bMouseOver 가 true 이면 m_nMouseOver 를 변경하고
	// 이전에 올려졌던 카드는 내려준다
	// false 이면 m_abSelected 의 값을 토글하고
	// 결과를 반영한다
	void ShiftCard( int x, int y, bool bMouseOver );
	// 현재 커서 아래쪽의 카드를 낼 수 있는가
	bool CanSelect() const;
	// OnLButtonDown 에 대해서 DSB 처리를 해 주는 함수
	bool CookDSBForLButtonDown( POINT point );
	// OnLButtonDown 에 대해서 카드 선택 처리를 해 주는 함수
	bool CookSelectionForLButtonDown( POINT point );

// Internal Members
protected:
	HCURSOR			m_hArrow;	// 화살표 커서
	HCURSOR			m_hHand;	// 손모양 커서

	CFont			m_fntSmall,	// 현재 폰트
					m_fntMiddle,
					m_fntBig,
					m_fntFixedSmall,
					m_fntFixedMiddle,
					m_fntFixedBig;

	CBitmap			m_pic;		// 실제 윈도우 모양이 그려져 있는 비트맵 버퍼
	CBitmap			m_board;	// 배경, 손, 바닥이 그려져 있는 비트맵
	CSize			m_szPic;	// 이 그림들의 크기

	CBmpMan			m_bm;		// 비트맵 매니저

	CSize			m_szCard;	// 현재 카드 크기

	CMFSM*			m_pMFSM;	// 현재 Mighty FSM

	CEvent*			m_pWaitingSelectionEvent;	// NULL이 아니면 클릭 대기 이벤트 포인터
	long*			m_pWaitingSelectionCard;	// 클릭 대기 상태에서 클릭된 카드를 리턴할 포인터

	int		m_nMouseOver;		// 현재 마우스 아래에 있어서 조금 들려진 카드 (-1은 없는경우)
	bool	m_abSelected[LAST_TURN_2MA+2];	// privilege 때 선택된 카드 (이중 14개만 사용)
	int		m_nDealMiss;		// 딜 미스 한 플레이어 (이 플레이어의 카드 내용은 reveal)

	int		m_nTurnRect;		// 차례 사각형의 위치

	CBoardWrap*	m_pWrap;		// Wrapper 객체
public:	bool m_bDoNotSendMeAnyMoreMessage;	// Wrapper 만이 접근한다

protected:
	// 카드 날리기 관련 멤버
	// * 카드 날리기라고 해서 항상 FlyCard 에만 관련있는 것은 아니고,
	//   카드 섞기, 일정시간 대기하기 등 시간 간격을 두고 시각 효과를
	//   주어야 하는 예약 사건을 포괄한다

	CCriticalSection	m_csFlying;			// 카드 날리기/섞기 큐에 대한 CS

	CList<CMsg*,CMsg*>	m_lFlyingQueue;		// 카드 날리기/섞기 큐
	volatile void*		m_hFlying;			// 현재 날리기 핸들
	volatile void*		m_hSuffling;		// 현재 섞기 핸들
	CEvent				m_eFlyIt;			// 카드 날리기/섞기 쓰레드가 대기하는 이벤트
	volatile bool		m_bFlyProcTerminated;
	volatile bool		m_bFlyProcTerminate;

	static UINT	FlyProc( LPVOID pThis );	// 카드 날리기/섞기 쓰레드 진입 함수
	enum OPERATION { opWaitWhile, opFlyCard, opSuffleCards };

	// Flying큐에 이 메시지를 추가하고 Flying이벤트를 세트한다
	void AddFlyingQueue( CMsg* pMsg )		{ m_csFlying.Lock(); m_lFlyingQueue.AddTail(pMsg); m_csFlying.Unlock(); m_eFlyIt.SetEvent(); }
	// FlyProc 에서 호출되는 실제 동작 함수들
	void DoFlyCard( int, int, int, int, int, int, int, int );
	void DoFlyCard2( int, int, int, int, int, int, int, int );
	void DoSuffleCards( int );

protected:
	// DSB 관련 멤버

	CList<DSB*,DSB*> m_lpDSB;	// DSB 리스트

	DSay*	m_apSayDSB[MAX_PLAYERS];	// 화면에 표시되고 있는 채팅 상자
	DSB*	m_apGoalDSB[MAX_PLAYERS];	// 화면에 표시되고 있는 공약 상자
	DSB*	m_pMasterGoalDSB;			// m_apGoalDSB 중 당선되려 하는 공약 상자
	DSB*	m_pCurrentElectionDSB;		// 화면에 표시되고 있는 현재 선거 상자

	bool		m_bDragDSB;				// DSB 를 드래그 하는 중인가
	DSB*		m_pDragDSB;				// 드래그 하고 있는 DSB
	CPoint		m_pntLastDSB;			// 드래그 하는 DSB 의 지난 좌표
	DSB*		m_pHLDSB;				// Highlighted DSB
	CList<DSB*,DSB*> m_lpModalDSB;		// Modal 상태인 DSB

protected:
	// 저수준 FlyCard 내에서만 사용되는, 핸들 구조체
	struct FLYCARD_INFO {
		int nCard;				// 날릴 카드
		CRect rcBegin;			// 시작되는 카드 사각형
		CPoint ptEnd;			// 끝나는 위치 ( 사각형의 좌상단 )
		int nStates;			// 몇번만에 날리기 완료하는가
		int nShadeMode;			// 그림자 모드
		int nCurShadeDepth;		// 현재의 그림자 높이
		int nCurState;			// 현재 몇번째 단계인가
		int x;					// 현재 x 위치
		int y;					// 현재 y 위치
		int xLast;				// 지난 x 위치
		int yLast;				// 지난 y 위치
		CSize sz;				// 기준 크기 (비트맵의 단위 크기)
	};
	// 저수준 SuffleCards 내에서만 사용되는, 핸들 구조체
	struct SUFFLECARDS_INFO {
		int nCount;				// 총 섞는 회수
		int nCurCount;			// 지금까지 섞은 회수
		int nCurStep;			// 섞기 그림 단계 ( 0-4)
		int xCenter;
		int yCenter;			// 화면 중심 좌표
		bool bBrushOrigin;		// 브러쉬 원점
		CSize sz;				// 화면에 찍히는 크기
	};

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBoard)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL
	virtual int OnToolHitTest( CPoint point, TOOLINFO* pTI ) const;

	// Generated message map functions
protected:
	//{{AFX_MSG(CBoard)
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnUpdateGameExit(CCmdUI* pCmdUI);
	afx_msg void OnGameExit();
	//}}AFX_MSG
	afx_msg LRESULT OnWrapper( WPARAM wParam, LPARAM lParam );
	afx_msg BOOL OnToolTipNeedText( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BOARD_H__6433F788_C1B5_11D2_97F2_000000000000__INCLUDED_)
