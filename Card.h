// Card.h: interface for the CCard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CARD_H__BE37E2C9_C23D_11D2_97F3_000000000000__INCLUDED_)
#define AFX_CARD_H__BE37E2C9_C23D_11D2_97F3_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef LAST_TURN_2MA
#	define LAST_TURN_2MA	12
#endif

#ifndef LAST_TURN
#	define LAST_TURN	9
#endif

#ifndef MIGHTY_EXPORT
#	ifdef _WINDLL
#		define MIGHTY_EXPORT __declspec(dllimport)
#	else
#		define MIGHTY_EXPORT __declspec(dllexport)
#	endif
#endif


struct CState;

// 마이티에서 사용되는 카드
// 카드와 정수 값의 매핑은 0 ~ 52 가 각각
// Nothing SA S2 ... S10 SJ SQ SK DA ... DK HA ... HK CA ... CK
// 이고, 그 이외의 수는 Joker 이다 ( 53 으로 내부적으로 정규화한다 )
// 또 모양과의 매핑은 1 ~ 4 가 각각 S D H C 이고
// 숫자와의 매핑은 1 ~ 13 이 각각 A 2 3 ... K 이다
// 단, 범위를 벗어나면 Joker 이다

#define SPADE 1
#define DIAMOND 2
#define HEART 3
#define CLOVER 4
#define ACE 1
#define JACK 11
#define QUEEN 12
#define KING 13
#define JOKER 53

class MIGHTY_EXPORT CCard  
{
public:
	// 현재 상태 정보에 대한 포인터를 설정 ( 이전 상태 리턴 )
	// 이 상태 정보는, 옵션과 기루다를 얻는데만 사용된다
	static const CState* SetState( const CState* pNewState ) { const CState* pOld = s_pState; s_pState = pNewState; return pOld; }
	static const CState* GetState()					{ return s_pState; }

public:
	// state 에 기반한 전역적인 정보
	static int GetKiruda();
	static CCard GetMighty();
	static CCard GetJoker()							{ return CCard(JOKER); }
	static CCard GetJokercall();

public:
	// 생성자 & 기본 함수
	CCard( int nCard = 0 )							{ n = nCard >= 0 && nCard < JOKER ? nCard : JOKER; }
	CCard( int nShape, int nNumber )				{ if ( nShape >= 1 && nShape <= 4 && nNumber >= 1 && nNumber <= 13 ) n = (nShape-1)*13 + (nNumber-1) + 1; else n = JOKER; }
	CCard( const CCard& c )							: n(c.n) {}
	CCard& operator=( const CCard& c )				{ n = c.n; return *this; }
	operator int() const							{ return n; }
	int GetShape() const							{ if ( n == 53 || n == 0 ) return 0; else return (n-1)/13+1; }
	int GetNum() const								{ if ( n == 53 || n == 0 ) return 0; else return (n-1)%13+1; }

	// 2 3 .. Q K A Joker 를 각각 1 ~ 14 로 매핑한 값을 리턴
	int GetPowerNum() const							{ return GetNum() ? ( GetNum() - 2 + 13 ) % 13 + 1 : 14; }

public:
	// 연산자 & 속성
	bool operator==( const CCard& c ) const			{ return n == c.n; }
	bool operator!=( const CCard& c ) const			{ return n != c.n; }
	bool IsPoint() const							{ return GetNum() >= 10 && GetNum() <= KING || GetNum() == ACE; }
	bool IsKiruda() const							{ int k = GetKiruda(); return SPADE <= k && k <= CLOVER && GetShape() == k; }
	bool IsMighty() const							{ return *this == GetMighty(); }
	bool IsJoker() const							{ return *this == GetJoker(); }
	bool IsJokercall() const						{ return *this == GetJokercall(); }
	bool IsOneEyedJack() const						{ return GetNum() == JACK && (GetShape() == HEART || GetShape() == SPADE); }
	bool IsAce() const								{ return GetNum() == ACE; }
	bool IsNothing() const							{ return n == 0; }

public:
	// 문자열 지원

	// 카드의 이름을 리턴한다
	// (예: "스페이드에이스" "기루다K" "기아" 등)
	CString GetString( bool bUseTerm ) const;
	// 선거 공약에 대한 문자열을 리턴한다
	// (예: "스페이드13" "곱트" "기노" )
	static CString GetGoalString( bool bUseTerm,
		int nKiruda, int nMinScore, int nDefaultMinScore );

protected:
	static const CState* s_pState;
	int n;
};


// 카드 리스트

class MIGHTY_EXPORT CCardList : public CList<CCard,CCard>
{
public:
	CCardList() : CList<CCard,CCard>(5) {}
	CCardList( const CCardList& cl );
	CCardList( const CCardList& cl1, const CCardList& cl2 );
	virtual ~CCardList() {}
	CCardList& operator=( const CCardList& cl );

public:
	// 덱 함수

	// 카드 한 벌을 만든다
	// nPlayers : 인원수 (5마 53 장, 4마 43 장, 3마 33 장)
	void NewDeck( int nPlayers, CCard cJokercall, CCard cAlterJokercall );
	// 카드를 섞는다
	void Suffle();

public:
	// 카드 조작 함수

	// 카드를 찾는다 ( 없으면 NULL )
	POSITION Find( CCard c ) const;
	// 위치에서 카드 인덱스를 구한다 ( 없으면 ASSERT 후 -1 )
	int IndexFromPOSITION( POSITION pos ) const;
	// 인덱스로 위치를 구한다
	POSITION POSITIONFromIndex( int index ) const;
	// 모양의 개수를 구한다
	int GetCountOf( int nShape ) const;
	// 인덱스로 카드를 얻는다
	CCard& operator[]( int index );
	CCard operator[]( int index) const;

public:
	// 유틸리티

	// 기루다로 해볼 만한 모양을 리턴한다 ( 숫자의 합이 가장 큰것 )
	int GetKirudable() const;
	// 카드 정렬
	void Sort( bool bLeftKiruda = false, bool bLeftAce = false );
	// 승자의 위치를 리턴
	// 이 함수는 CCard의 CState 정보중 기루다와 옵션 정보를 사용한다
	// nTurn : 현재 턴수
	// nJokerShape : 조커가 선인 경우 조커 모양
	// bIsJokercall : 조커콜이 선인 경우 조커콜인가
	POSITION GetWinner( int nTurn, int nJokerShape, bool bIsJokercall ) const;
	// 점수 카드의 개수를 구한다
	int GetPoint() const;
	// 이 객체가 손에 든 카드를 의미할 때 낼 수 있는 카드의
	// 리스트를 구한다
	// CCard의 CState 정보와 Option 을 사용하며, 현재 턴에
	// 대한 정보는 주어져야 한다
	void GetAvList(
		CCardList* plResult,		// 결과 리턴값
		const CCardList* plCurrent,	// 현재 바닥패
		int nTurn,					// 현재 턴값
		int nJokerShape,			// 조커가 선인 경우 조커 모양
		bool bIsJokercall ) const;	// 조커콜이 선인 경우 실제 조커콜인가?

protected:
	// 내부적인 함수

	// 정렬시 필요한 함수
	int SortComp( CCard c1, CCard c2,
		bool bLeftKiruda, bool bLeftAce ) const;
	// 우열을 정할때 필요한 함수
	int PowerComp( CCard c1, CCard c2, int nTurn,
		int nCurrentShape, bool bBeginerIsJokercall ) const;

public:
	// 주어진 카드가 기루다인가를 검사
	// 카드의 기루다에 관한 모든 정보는 이 함수로 얻는다
	virtual bool IsKiruda( CCard c ) const
	{	return c.IsKiruda(); }
};

#endif // !defined(AFX_CARD_H__BE37E2C9_C23D_11D2_97F3_000000000000__INCLUDED_)
