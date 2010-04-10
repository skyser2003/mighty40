// Card.cpp: implementation of the CCard class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "State.h"
#include "Rule.h"
#include "Card.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const CState* CCard::s_pState = 0;

// state 에 기반한 전역적인 정보

int CCard::GetKiruda()
{
	ASSERT(s_pState);
	return s_pState->goal.nKiruda;
}

CCard CCard::GetMighty()
{
	ASSERT(s_pState && s_pState->pRule);
	if ( GetKiruda() == CCard(s_pState->pRule->nMighty).GetShape() )
		// 마이티 모양과 기루다가 같다
		return CCard(s_pState->pRule->nAlterMighty);
	else return CCard(s_pState->pRule->nMighty);
}

CCard CCard::GetJokercall()
{
	ASSERT(s_pState && s_pState->pRule);
	if ( GetKiruda() == CCard(s_pState->pRule->nJokercall).GetShape() )
		// 조커콜 모양과 기루다가 같다
		return CCard(s_pState->pRule->nAlterJokercall);
	else return CCard(s_pState->pRule->nJokercall);
}

CString CCard::GetString( bool bUseTerm ) const
{
	static LPCTSTR s_aShapeTerm[][2] = {
		{ _T(""), _T("") },
		{ _T("삽"), _T("스페이드 ") },
		{ _T("다리"), _T("다이아몬드 ") },
		{ _T("트"), _T("하트 ") },
		{ _T("끌"), _T("클로버 ") } };

	CString sRet;
	if ( bUseTerm ) {

		if ( IsMighty() ) return CString( _T("마이티") );
		else if ( IsJoker() ) return CString( _T("쪼카") );
		else if ( IsJokercall() ) return CString( _T("쪼콜") );
		else {
			if ( IsKiruda() ) sRet = _T("기");
			else sRet = s_aShapeTerm[GetShape()][0];
			if ( IsAce() ) sRet += _T("아");
			else if ( GetNum() == KING ) sRet += _T("카");
			else if ( GetNum() == QUEEN ) sRet += _T("큐");
			else if ( GetNum() == JACK ) sRet += _T("자");
			else if ( GetNum() == 10 ) sRet += _T("10");
			else sRet += (TCHAR)( GetNum() + (int)_T('0') );
		}
	}
	else {

		if ( IsMighty() ) return CString( _T("마이티") );
		else if ( IsJoker() ) return CString( _T("조커") );
		else if ( IsJokercall() ) return CString( _T("조커콜") );
		else {
			if ( IsKiruda() ) sRet = _T("기루다 ");
			else sRet = s_aShapeTerm[GetShape()][1];
			if ( IsAce() ) sRet += _T("A");
			else if ( GetNum() == KING ) sRet += _T("K");
			else if ( GetNum() == QUEEN ) sRet += _T("Q");
			else if ( GetNum() == JACK ) sRet += _T("J");
			else if ( GetNum() == 10 ) sRet += _T("10");
			else sRet += (TCHAR)( GetNum() + (int)_T('0') );
		}
	}
	return sRet;
}

// 선거 공약에 대한 문자열을 리턴한다
// (예: "스페이드13" "곱트" "기노" )
CString CCard::GetGoalString( bool bUseTerm,
	int nKiruda, int nMinScore, int nDefaultMinScore )
{
	static LPCTSTR s_aScoreTerm[] = {
		_T("독"), _T("둘"), _T("셋"), _T("넷"), _T("닷"),
		_T("엿"), _T("곱"), _T("덜"), _T("홉"), _T("풀") };
	static LPCTSTR s_aShapeTerm[][2] = {
		{ _T("노"), _T("노기루다") },
		{ _T("삽"), _T("스페이드 ") },
		{ _T("다리"), _T("다이아몬드 ") },
		{ _T("트"), _T("하트 ") },
		{ _T("끌"), _T("클로버 ") } };

	CString sRet;

	if ( !bUseTerm ) {
		// 그대로 사용한다
		sRet.Format( _T("%s %d"), s_aShapeTerm[nKiruda][1], nMinScore );
		return sRet;
	}
	else {
		// 문제는 최소 점수가 11 보다 작거나 20보다 큰 경우이다
		// 각각 "초" 을 붙이고 "풀" 을 붙인다
		// 30 이상인 경우는 생각하지 않는다
		if ( nMinScore <= 10 ) sRet = _T("초");
		if ( nMinScore > 20 ) sRet = _T("풀");
		sRet += s_aScoreTerm[(nMinScore+9)%10];
		sRet += s_aShapeTerm[nKiruda][0];
	}
	return sRet;
}

/////////////////////////////////////////////////////////////////////////////

CCardList::CCardList( const CCardList& cl ) : CList<CCard,CCard>(5)
{
	for ( POSITION pos = cl.GetHeadPosition(); pos; )
		AddTail( cl.GetNext(pos) );
}

CCardList::CCardList( const CCardList& cl1, const CCardList& cl2 ) : CList<CCard,CCard>(5)
{
	for ( POSITION pos1 = cl1.GetHeadPosition(); pos1; )
		AddTail( cl1.GetNext(pos1) );
	for ( POSITION pos2 = cl2.GetHeadPosition(); pos2; )
		AddTail( cl2.GetNext(pos2) );
}

CCardList& CCardList::operator=( const CCardList& cl )
{
	RemoveAll();
	for ( POSITION pos = cl.GetHeadPosition(); pos; )
		AddTail( cl.GetNext(pos) );
	return *this;
}

// 카드 한 벌을 만든다
// nPlayers : 인원수 (5마 53 장, 4마 43 장, 3마 33 장)
void CCardList::NewDeck( int nPlayers, CCard cJokercall, CCard cAlterJokercall )
{
	// 2마와 6마 이상은 5마와 덱이 같다 (v4.0)
	if ( nPlayers >= 6 || nPlayers == 2 ) nPlayers = 5;

	ASSERT( nPlayers == 3 || nPlayers == 4 || nPlayers == 5 );

	RemoveAll();

	// 두 조커콜(주,보조)의 숫자를 얻어 둔다
	// 카드를 제거할 때 이들이 빠져서는 안된다
	CCard cJC1( cJokercall );
	CCard cJC2( cAlterJokercall );

	// 조커
	AddTail( CCard(JOKER) );

	if ( nPlayers == 5 ) {	// 5마

		// 모든 카드를 추가
		for ( int i = ACE; i <= KING; i++ )
			for ( int j = SPADE; j <= CLOVER; j++ )
				AddTail( CCard( j, i ) );
	}
	else if ( nPlayers == 3 ) {	// 3마

		// 2에서부터 조커콜을 뺀 5종류의 숫자를 뺀다
		int skip = 0;
		for ( int i = ACE; i <= KING; i++ ) {
			if ( i >= 2 && i <= 9 && skip < 5
					&& i != cJC1.GetNum()
					&& i != cJC2.GetNum() ) {
				skip++;
				continue;
			}
			else for ( int j = SPADE; j <= CLOVER; j++ )
				AddTail( CCard( j, i ) );
		}
	}
	else if ( nPlayers == 4 ) {	// 4마

		if ( cJC1.GetNum() == cJC2.GetNum()
				&& cJC1.GetNum() >= 2 && cJC1.GetNum() <= 4 ) {
			// 두 조커콜 모양이 같고 2와 4 사이이다

				// 2, 3, 4 를 빼고 만든 다음 조커콜 2장을 추가
				for ( int i = ACE; i <= KING; i++ )
					if ( i >= 2 && i <= 4 ) continue;
					else for ( int j = SPADE; j <= CLOVER; j++ )
						AddTail( CCard( j, i ) );
				AddTail( cJC1 ); AddTail( cJC2 );
		}
		else {
			// 2 부터 조커콜이 아닌 카드를 빼고
			// 총 10장을 제거한다

			// 모든 카드를 추가
			for ( int i = ACE; i <= KING; i++ )
				for ( int j = SPADE; j <= CLOVER; j++ )
					AddTail( CCard( j, i ) );
			// 제거
			int skip = 0;
			for ( int s = 2; s <= 9 && skip < 10; s++ )
				for ( int t = CLOVER; t >= SPADE && skip < 10; t-- )
					if ( CCard(t,s) != cJC1 && CCard(t,s) != cJC2 ) {
						RemoveAt( Find( CCard(t,s) ) );
						skip++;
					}
		}
	}

	ASSERT( nPlayers == 5 && GetCount() == 53
		|| nPlayers == 4 && GetCount() == 43
		|| nPlayers == 3 && GetCount() == 33 );
}

// 카드를 섞는다
void CCardList::Suffle()
{
	// 새 리스트를 만든 후 그것에
	// 이 리스트의 모든 원소를 앞 뒤로 랜덤하게 옮긴다
	// 이것을 역할을 바꿔서 다시한번 한다

	int nCards = GetCount();

	CCardList lTemp;

	for ( int i = 0; i < 3; i++ ) {
		// 세 번 반복

		// 모든 원소를 lTemp 로 옮긴다
		for ( int j = 0; j < nCards; j++ )
			if ( rand()%2 ) lTemp.AddTail( RemoveHead() );
			else lTemp.AddHead( RemoveHead() );
		// 다시 옮겨 온다
		for ( int k = 0; k < nCards; k++ )
			if ( rand()%2 ) AddTail( lTemp.RemoveHead() );
			else AddHead( lTemp.RemoveHead() );

		ASSERT( lTemp.GetCount() == 0 && GetCount() == nCards );
	}
}

// 카드를 찾는다 ( 없으면 NULL )
POSITION CCardList::Find( CCard c ) const
{
	for ( POSITION pos = GetHeadPosition(); pos; GetNext(pos) )
		if ( GetAt(pos) == c ) return pos;
	return NULL;
}

// 모양의 개수를 구한다
int CCardList::GetCountOf( int nShape ) const
{
	int nCount = 0;
	for ( POSITION pos = GetHeadPosition(); pos; )
		if ( GetNext(pos).GetShape() == nShape )
			nCount++;
	return nCount;
}

// 인덱스로 카드를 얻는다
CCard& CCardList::operator[]( int index )
{
	int i = 0;
	for ( POSITION pos = GetHeadPosition(); pos; i++, GetNext(pos) )
		if ( i == index ) return GetAt(pos);
	ASSERT(0);
	return *(CCard*)0;
}

CCard CCardList::operator[]( int index) const
{
	int i = 0;
	for ( POSITION pos = GetHeadPosition(); pos; i++, GetNext(pos) )
		if ( i == index ) return GetAt(pos);
	ASSERT(0);
	return CCard(0);
}

// 위치에서 카드 인덱스를 구한다 ( 없으면 ASSERT 후 -1 )
int CCardList::IndexFromPOSITION( POSITION pos ) const
{
	POSITION posSearch = GetHeadPosition();

	for ( int index = 0; posSearch; index++ )
		if ( posSearch == pos ) return index;
		else GetNext(posSearch);

	ASSERT(0);
	return -1;
}

// 인덱스로 위치를 구한다
POSITION CCardList::POSITIONFromIndex( int index ) const
{
	POSITION posSearch = GetHeadPosition();

	for ( int i = 0; posSearch; i++, GetNext(posSearch) )
		if ( i == index ) return posSearch;

	return 0;
}

// 기루다로 해볼 만한 모양을 리턴한다 ( 숫자의 합이 가장 큰것 )
int CCardList::GetKirudable() const
{
	int nMaxShape = 0;
	int nMaxSum = 0;

	for ( int i = SPADE; i <= CLOVER; i++ ) {
		// 각 모양의 숫자 합을 구함 ( 2 ... K A 까지 각 1 ~ 13 )
		int nSum = 0;
		for ( POSITION pos = GetHeadPosition(); pos; GetNext(pos) )
			if ( GetAt(pos).GetShape() == i )
				nSum += GetAt(pos).GetPowerNum();
		if ( nSum > nMaxSum ) {
			nMaxSum = nSum;
			nMaxShape = i;
		}
	}
	ASSERT( nMaxShape && nMaxSum );
	return nMaxShape;
}

// 정렬시 필요한 함수
int CCardList::SortComp( CCard c1, CCard c2,
	bool bLeftKiruda, bool bLeftAce ) const
{
	ASSERT( c1 != c2 );

	if ( c1.IsJoker() ) return 1;
	if ( c2.IsJoker() ) return -1;

	if ( IsKiruda(c1) && !IsKiruda(c2) )
		return bLeftKiruda ? 1 : -1;
	if ( !IsKiruda(c1) && IsKiruda(c2) )
		return bLeftKiruda ? -1 : 1;
	if ( c1.GetShape() > c2.GetShape() )
		return -1;
	if ( c1.GetShape() < c2.GetShape() )
		return 1;

	if ( c1.IsAce() && !c2.IsAce() )
		return bLeftAce ? 1 : -1;
	if ( !c1.IsAce() && c2.IsAce() )
		return bLeftAce ? -1 : 1;
	if ( c1.GetNum() > c2.GetNum() )
		return -1;
	if ( c1.GetNum() < c2.GetNum() )
		return 1;

	ASSERT(0);
	return 0;
}

// 카드 정렬
void CCardList::Sort( bool bLeftKiruda, bool bLeftAce )
{
	// 삽입 소트

	CCardList lTemp;

	int nCards = GetCount();
	for ( int i = 0; i < nCards; i++ ) {

		CCard c = RemoveHead();
		POSITION pos = lTemp.GetHeadPosition();

		for ( ; pos; lTemp.GetNext(pos) )
			if ( SortComp( c, lTemp.GetAt(pos),
				bLeftKiruda, bLeftAce ) > 0 ) break;

		if ( pos ) lTemp.InsertBefore( pos, c );
		else lTemp.AddTail( c );
	}
	ASSERT( IsEmpty() && lTemp.GetCount() == nCards );
	*this = lTemp;
}

// 점수 카드의 개수를 구한다
int CCardList::GetPoint() const
{
	int nCount = 0;
	for ( POSITION pos = GetHeadPosition(); pos; )
		if ( GetNext(pos).IsPoint() ) nCount++;
	return nCount;
}

// 승자의 위치를 리턴
// 이 함수는 CCard의 CState 정보중 기루다와 옵션 정보를 사용한다
// nTurn : 현재 턴수
// nJokerShape : 조커가 선인 경우 조커 모양
// bIsJokercall : 조커콜이 선인 경우 조커콜인가
POSITION CCardList::GetWinner( int nTurn, int nJokerShape, bool bIsJokercall ) const
{
	ASSERT( GetCount() );

	// 현재 모양
	int nCurrentShape =
		GetHead().IsJoker() ? nJokerShape : GetHead().GetShape();
	// 선이 조커콜인가
	bool bBeginerIsJokercall =
		GetHead().IsJokercall() && bIsJokercall;

	POSITION posWin = 0, pos = GetHeadPosition();
	for ( ; pos; GetNext(pos) ) {
		if ( !posWin
			|| PowerComp( GetAt(posWin), GetAt(pos), nTurn,
				nCurrentShape, bBeginerIsJokercall ) < 0 )
			posWin = pos;
	}
	ASSERT( posWin );
	return posWin;
}

// 우열을 정할때 필요한 함수
int CCardList::PowerComp( CCard c1, CCard c2, int nTurn,
	int nCurrentShape, bool bBeginerIsJokercall ) const
{
	ASSERT( c1 != c2 );
	const CState* pState = CCard::GetState(); ASSERT( pState );
	const CRule* pRule = pState->pRule; ASSERT( pRule );

	// 마이티 효력
	// 첫판이고 첫판에 마이티 효력 없거나
	// 막판이고 막판에 마이티 효력 없으면 무효
	bool bMightyEffect =
		! ( nTurn == 0 && !pRule->bInitMightyEffect
			|| nTurn == LAST_TURN && !pRule->bLastMightyEffect );
	// 조커콜 효력
	// 첫판이고 첫판에 조커콜 효력 없으면 무효
	bool bJokercallEffect =
		! ( nTurn == 0 && !pRule->bInitJokercallEffect );
	// 조커콜을 당했는가
	// 선이 조커콜을 냈고 조커콜 효력이 있을 때
	bool bJokercalled =
		bBeginerIsJokercall && bJokercallEffect;
	// 조커 효력
	// 첫판이고 첫판에 조커 효력 없거나
	// 막판이고 막판에 조커 효력 없거나
	// 조커콜 당했을때 무효
	bool bJokerEffect =
		! ( nTurn == 0 && !pRule->bInitJokerEffect
			|| nTurn == LAST_TURN && !pRule->bLastJokerEffect
			|| bJokercalled && !pRule->bJokercallJokerEffect );

	// 마이티에 대한 처리
	if ( bMightyEffect ) {
		if ( c1.IsMighty() ) return 1;
		else if ( c2.IsMighty() ) return -1;
	}
	// 조커에 대한 처리
	if ( bJokerEffect ) {
		if ( c1.IsJoker() ) return 1;
		else if ( c2.IsJoker() ) return -1;
	}
	else {
		if ( c1.IsJoker() ) return -1;
		else if ( c2.IsJoker() ) return 1;
	}
	// 기루다에 대한 처리
	if ( IsKiruda(c1) || IsKiruda(c2) ) {
		if ( !IsKiruda(c1) ) return -1;
		else if ( !IsKiruda(c2) ) return 1;
		else return c1.GetPowerNum() > c2.GetPowerNum() ? 1 : -1;
	}
	// 현재 모양에 대한 처리
	else if ( c1.GetShape() != nCurrentShape
		&& c2.GetShape() != nCurrentShape )
		// 모두 현재 모양이 아닌 경우는 비교 의미가 없다
		return 0;
	else if ( c1.GetShape() != nCurrentShape ) return -1;
	else if ( c2.GetShape() != nCurrentShape ) return 1;
	else return c1.GetPowerNum() > c2.GetPowerNum() ? 1 : -1;
}

// 이 객체가 손에 든 카드를 의미할 때 낼 수 있는 카드의
// 리스트를 구한다
// CCard의 CState 정보와 Option 을 사용하며, 현재 턴에
// 대한 정보는 주어져야 한다
void CCardList::GetAvList(
	CCardList* plResult,		// 결과 리턴값
	const CCardList* plCurrent,	// 현재 바닥패
	int nTurn,					// 현재 턴값
	int nJokerShape,			// 조커가 선인 경우 조커 모양
	bool bIsJokercall ) const	// 조커콜이 선인 경우 실제 조커콜인가?
{
	ASSERT( plResult && plCurrent );
	ASSERT( CCard::GetState() && CCard::GetState()->pRule );

	plResult->RemoveAll();

	const CRule* pRule = CCard::GetState()->pRule;

	// 루프를 돌면서 마이티와 조커 소유 여부를 조사한다
	bool bHasMighty = false, bHasJoker = false;

	// 현재 모양은 ?   0 이면 선
	int nCurrentShape = 0;
	if ( plCurrent->GetCount() > 0 ) {
		CCard c = plCurrent->GetHead();
		nCurrentShape = c.IsJoker() ? nJokerShape : c.GetShape();
	}
	// 현재 모양이 하나라도 있을까?
	bool bHasCurrentShape =
		!nCurrentShape ? 0
		: GetCountOf( nCurrentShape ) ? true : false;

	POSITION pos = GetHeadPosition();
	while (pos) {
		CCard c = GetNext(pos);

		if ( c.IsMighty() ) {
			// 마이티

			bHasMighty = true;

			// 첫판, 끝 판에 마이티 낼 수 있는지를 조사
			if ( !( nTurn == 0 && !pRule->bInitMighty
					|| nTurn == LAST_TURN && !pRule->bLastMighty ) )
				// 낼 수 있음
				plResult->AddTail( c );
		}
		else if ( c.IsJoker() ) {
			// 조커

			bHasJoker = true;

			// 조커콜이라면 무조건 조커를 낸다
			if ( plCurrent->GetCount() > 0 // 선이 아니고
				&& plCurrent->GetHead().IsJokercall() // 선이 조커콜이고
				&& bIsJokercall // 이 조커콜이 실제 조커콜이고
				&& ( nTurn != 0 || pRule->bInitJokercallEffect ) ) { // 조커콜 효력있음
				// 무조건!
				plResult->RemoveAll();
				plResult->AddTail( c );
				// 조커콜 대신 마이티 있으면 마이티도 낼수 있는 규칙도 있음
				bHasMighty = !!Find(CCard::GetMighty());
				if ( pRule->bOverrideMighty && bHasMighty ) {
					plResult->AddTail( CCard::GetMighty() );
				}
				break;
			}
			// 첫판, 끝 판에 조커 낼 수 있는지를 조사
			if ( !( nTurn == 0 && !pRule->bInitJoker
					|| nTurn == LAST_TURN && !pRule->bLastJoker ) )
				// 낼 수 있음
				plResult->AddTail( c );
		}
		else {
			if ( bHasCurrentShape ) {
				// 현재 바닥 모양이 있기 때문에 이 모양을
				// 내야 한다
				if ( c.GetShape() == nCurrentShape )
					plResult->AddTail( c );
			}
			else {
				// 현재 바닥 모양이 없기 때문에 아무 모양이나 내도 된다
				if ( IsKiruda(c) ) {
					// 기루다

					// 첫판에 기루다 낼 수 있는지를 조사
					if ( !( nTurn == 0 && !pRule->bInitKiruda )
						&& !( nTurn == 0 && plCurrent->GetCount() == 0
							&& !pRule->bInitBeginKiruda ) )
						// 낼 수 있음
						plResult->AddTail( c );
				}
				else
					// 일반 모양
					plResult->AddTail( c );
			}
		}
	}

	// 만약 마지막에 조커나 마이티를 낼 수 없는데
	// 마지막 턴 이전에 이들을 가지고 있다면
	// 꼭 이들을 내야 한다
	if ( nTurn == LAST_TURN-2
			&& ( bHasMighty && !pRule->bLastMighty )
			&& ( bHasJoker && !pRule->bLastJoker ) ) {
		plResult->RemoveAll();
		plResult->AddTail( CCard::GetMighty() );
		plResult->AddTail( CCard::GetJoker() );
		return;
	}
	else if ( nTurn == LAST_TURN-1
			&& ( ( bHasMighty && !pRule->bLastMighty )
				|| ( bHasJoker && !pRule->bLastJoker ) ) ) {
		plResult->RemoveAll();
		plResult->AddTail(
			( bHasMighty && !pRule->bLastMighty )
				? CCard::GetMighty() : CCard::GetJoker() );
		return;
	}

	// 아무것도 낼 게 없는 상황이 왔다 ??
	if ( plResult->GetCount() == 0 ) {
//		ASSERT(0);
		// 모든 카드가 기루다라든가..
		// 이 경우 아무 카드나 낼 수 있는 특권을 준다 !
		*plResult = *this;
	}
}
