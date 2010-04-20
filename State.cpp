// State.cpp: implementation of the CState class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "State.h"
#include "Card.h"
#include "Play.h"
#include "Player.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CState::CState() :
	pRule(new CRule),
	lDeck(*(new CCardList)),
	lCurrent(*(new CCardList)),
	lDead(*(new CCardList))
{}

CState::~CState()
{
	CCard::SetState(0);

	delete pRule;
	delete &lDeck;
	delete &lCurrent;
	delete &lDead;
}

// 각 판 범위의 데이터들을 초기화 한다 (프랜드 등)
void CState::InitStageData( int _nGameNum, int _nBeginer )
{
	CCard::SetState( this );

	state = msReady;

	nGameNum = _nGameNum;

	// 덱 구성
	lDeck.RemoveAll();
	lDeck.NewDeck( pRule->nPlayerNum,
		CCard(pRule->nJokercall), CCard(pRule->nAlterJokercall) );

	for ( int s = 0; s < 4; s++ )
		lDeck.Suffle();

	lCurrent.RemoveAll();

	memset( aanHistory, 0, sizeof( aanHistory ) );

	// 아직 한 명이 죽지 않았으므로 모든 플레이어가 게임에 참가
	nPlayers = pRule->nPlayerNum;
	for ( int i = 0; i < pRule->nPlayerNum; i++ ) {
		apAllPlayers[i]->Reset();
		apPlayers[i] = apAllPlayers[i];
		apPlayers[i]->SetPlayerNum(i);
		abGiveup[i] = false;
	}

	lDead.RemoveAll();

	nMaster = -1;
	nFriend = -1;
	nDeadID[0] = -1;
	nDeadID[1] = -1;

	bFriendRevealed = false;

	goal.nKiruda = 0;
	goal.nMinScore = 0;
	goal.nFriend = 0;
	acDrop[0] = acDrop[1] = acDrop[2] = 0;
	nThrownPoints = 0;

	bJokerUsed = false;

	for ( int u = 0; u < 4; u++ )
		anUsedCards[u] = anUsingCards[u] = 0;

	nTurn = -1;
	nBeginer = _nBeginer;
	bJokercallEffect = false;
	nJokerShape = 0;

	cCurrentCard = 0;
	nCurrentPlayer = nBeginer;
}

// 매턴의 끝 (lCurrent 가 사라지기 전) 에서 호출하여 히스토리 정보를 유지한다
void CState::WriteHistory()
{
	int* anScored = aanHistory[0];
	int* anAssist = aanHistory[1];
	int* anLost = aanHistory[2];
	int* anShare = aanHistory[3];

	// 바닥 카드 장수는 플레이어 수와 같아야 한다
	ASSERT( lCurrent.GetCount() == nPlayers );

	int i;

	// 점유율 ( 선을 잡은 수 비율 (%) )
	anShare[nBeginer] += 10;

	// 승자 플레이어 번호와 승자의 여당 여부
	int nWinner; bool bWinnerDef;
	nWinner = GetWinner( &lCurrent, &bWinnerDef );

	// 각 플레이어에 대한 정보를 갱신한다
	POSITION posCurrent = lCurrent.GetHeadPosition();
	for ( i = 0; i < nPlayers; i++ ) {

		// 현재 플레이어의 인덱스는? 누가 선이었는가로 결정된다
		int nPlayer = ( nBeginer + i ) % nPlayers;
		// 이 플레이어가 낸 카드
		CCard cCard = lCurrent.GetNext(posCurrent);

		// 이 플레이어의 역할에 따라서
		// 승자 : 득점 ( 얻은 점수 카드 수 )
		// 승자와 같은 편 : 도움 ( 같은 편이 얻게 한 점수 )
		// 승자와 다른 편 : 실점 ( 다른 편이 얻게 한 점수 )
		// 를 갱신한다

		if ( nPlayer == nWinner ) {
			// 승자
			if ( nPlayers == 2 ) 
				anScored[nPlayer]++;
			else 
				anScored[nPlayer] += lCurrent.GetPoint();
		}
		else if ( bWinnerDef && ( nPlayer == nMaster
								|| nPlayer == nFriend )
				|| !bWinnerDef && ( nPlayer != nMaster
								&& nPlayer != nFriend ) )
			// 승자와 같은 편
			anAssist[nPlayer] += cCard.IsPoint() ? 1 : 0;
		else // 승자와 다른 편
			anLost[nPlayer] += cCard.IsPoint() ? 1 : 0;
	}
}

// 현재 goal 에 대해서, 다음의 기루다와
// 최소 점수의 조합이 가능한가를 리턴한다
// nNewMinScore 가 0 이면 포기 가능 여부를, -1 이면
// DealMiss 가능 여부를 묻는다
bool CState::IsValidNewGoal( int nNewKiruda, int nNewMinScore ) const
{
	// 이 상황이 선거중인가, 특권 상황인가를 판단한다
	bool bInElection = state == msElection ? true : false;
	bool bBlindKirudaChange =
		state == msPrivilege
		&& apPlayers[nCurrentPlayer]->GetHand()->GetCount() < 13;

	ASSERT( state == msElection || state == msPrivilege );

	// 딜 미스 여부를 판단
	if ( nNewMinScore == -1 )
		// 특권 상황이라면 딜 미스 불가
		if ( !bInElection )	return false;
		else if ( IsDealMiss( apPlayers[nCurrentPlayer]->GetHand() ) )
			return true;
		else return false;

	// 포기 가능 여부를 판단
	if ( nNewMinScore == 0 ) {
		// 특권 상황이라면 포기 불가
		if ( !bInElection )	return false;
		// 선이고, 아직 공약이 안만들어진 상태이며, 선의
		// 불출마를 용납하지 않는 경우
		else if ( goal.nMinScore == 0 && nCurrentPlayer == nBeginer
				&& !pRule->bBeginerPass ) return false;
		// 그 외에는 포기할 수 있음
		else return true;
	}

	// 최소점수 제약 검사
	int nMinLimit;
	if ( pRule->bNoKirudaAdvantage && nNewKiruda == 0 )
		nMinLimit = pRule->nMinScore-1;
	else nMinLimit = pRule->nMinScore;
	if ( nNewMinScore < nMinLimit ) return false;

	// 최대점수 제약 검사
	int nMaxLimit;
	if ( pRule->nPlayerNum == 2 ) {
		if ( pRule->nMinScore > MAX_SCORE_2MA )
			nMaxLimit = pRule->nMinScore;
		else
			nMaxLimit = MAX_SCORE_2MA;
	}
	else {
		if ( pRule->bHighScore ) nMaxLimit = HIGHSCORE_MAXLIMIT;
		else nMaxLimit = MAX_SCORE;
	}
	if ( nMaxLimit < nNewMinScore ) return false;

	// 이전 출마자 공약 제약 검사
	if ( bInElection ) {
		int temp = goal.nMinScore;
		if ( pRule->bNoKirudaAlways ) {		// 노기는 항상 1 적게 부르는 규칙일 때
			if ( nNewKiruda == 0 ) temp--;					// 이 공약이 노기인 경우 temp를 1 감소시킴.
			if ( goal.nKiruda == 0 && temp < 19 ) temp++;	// 이전 공약이 노기이면 노기인 경우 temp를 1 증가시킴. 단, 노기 19인 경우 기루 20 부를 수 있음
		}
		if ( nNewMinScore <= temp ) return false;
	}

	// 선거 중에는 더이상의 제약은 없음
	if ( bInElection ) return true;

	// 기루다를 바꾸는 경우
	if ( goal.nKiruda != nNewKiruda ) {

		// 올려야 하는 점수
		int nMinDiff;
		if ( !pRule->bRaise2ForKirudaChange
			|| bBlindKirudaChange )
			nMinDiff = 1;
		else nMinDiff = 2;
		if ( pRule->bNoKirudaAdvantage && ( nNewKiruda == 0 ) ) nMinDiff--;

		if ( min( goal.nMinScore + nMinDiff, nMaxLimit )
			<= min( nNewMinScore, nMaxLimit ) )
			return true;
		else return false;
	}

	if ( goal.nMinScore <= nNewMinScore )
		return true;
	else return false;
}

// 새로운 프랜드 후보 값을 주면 적합한 경우 그 값을 그대로,
// 아니면 0(노프랜드)을 리턴한다
int CState::GetValidFriend( int nFriend ) const
{
	// 아예 프랜드라는 제도가 없는 경우
	if ( !pRule->bFriend ) return 0;

	// 노프랜드, 초구 프랜드
	if ( nFriend == 0 || nFriend == 100 ) return nFriend;

	// 플레이어
	if ( nFriend < 0 ) {
		if ( -nFriend-1 < nPlayers
			&& -nFriend-1 != nMaster ) return nFriend;
		else { ASSERT(0); return 0; }
	}

	// 카드
	if ( !( 0 < nFriend && nFriend < 54 ) ) { ASSERT(0); return 0; }
	CCard c(nFriend);

	// 조커프랜드 불가
	if ( c.IsJoker() && !pRule->bJokerFriend ) { ASSERT(0); return 0; }

	if ( apPlayers[nMaster]->GetHand()->Find(c)
		|| lDeck.Find(c) ) return 0;
	else return nFriend;
}

// 주어진 카드패가 딜미스인가 검토한다
bool CState::IsDealMiss( const CCardList* pl ) const
{
	bool bHasMighty = pl->Find( CCard::GetMighty() ) ? true : false;
	bool bHasJoker = pl->Find( CCard::GetJoker() ) ? true : false;
	int nPoints = pl->GetPoint();

	// 10 한 장
	if ( pRule->bDM_Only10 && nPoints == 1 ) {
		POSITION pos10 = pl->GetHeadPosition();
		while (pos10) if ( pl->GetNext(pos10).GetNum() == 10 ) return true;
	}

	// 애꾸눈 J 한 장
	if ( pRule->bDM_OneEyedJack && nPoints == 1 ) {
		POSITION posJ = pl->GetHeadPosition();
		while (posJ) if ( pl->GetNext(posJ).IsOneEyedJack() ) return true;
	}

	// 조커 -1점
	if ( pRule->bDM_JokerIsReversePoint && bHasJoker ) nPoints--;

	// 마이티 0점
	if ( pRule->bDM_OnlyMighty && bHasMighty ) nPoints--;

	if ( pRule->bDM_Duplicate ) {
		// 10 한 장
		if ( pRule->bDM_Only10 && nPoints == 1 ) {
			POSITION pos10 = pl->GetHeadPosition();
			while (pos10) if ( pl->GetNext(pos10).GetNum() == 10 ) return true;
		}
		// 애꾸눈 J 한 장
		if ( pRule->bDM_OneEyedJack && nPoints == 1 ) {
			POSITION posJ = pl->GetHeadPosition();
			while (posJ) if ( pl->GetNext(posJ).IsOneEyedJack() ) return true;
		}
	}

	// 이제 전반적인 점수 카드의 수로 계산을 시작

	// 점수카드가 없을 때 딜미스
	if ( pRule->bDM_NoPoint) {
		if ( pRule->bDM_OnlyOne && nPoints <= 1 ) return true;
		if ( nPoints <= 0 ) return true;
	}

	// 모두 점수카드일때 딜미스
	if ( pRule->bDM_AllPoint && nPoints >= pl->GetCount() ) return true;

	// 그 외에는 딜미스가 아님
	return false;
}

// 주어진 카드 리스트에 대해 승자 플레이어 번호를 알아낸다
// 음의 값은 현재 state 값으로 지정한다
int CState::GetWinner( const CCardList* pList,
	bool* pbWinnerIsDef,
	int _nJokerShape, int _bJokercallEffect,
	int _nPlayers, int _nTurn,
	int _nBeginer, int _nMaster, int _nFriend ) const
{
	POSITION posWin = pList->GetWinner(
		_nTurn == -1 ? nTurn : _nTurn,
		_nJokerShape == -1 ? nJokerShape : _nJokerShape,
		_bJokercallEffect == -1 ? bJokercallEffect : (_bJokercallEffect?true:false) );
	int nRet = ( (_nBeginer == -1 ? nBeginer : _nBeginer)
				+ pList->IndexFromPOSITION( posWin ) )
		% ( _nPlayers == -1 ? nPlayers : _nPlayers );

	if ( pbWinnerIsDef )
		*pbWinnerIsDef = 
			nRet == ( _nMaster == -1 ? nMaster : _nMaster )
			|| nRet == ( _nFriend == -1 ? nFriend : _nFriend );

	return nRet;
}

// 해당 플레이어가 여당인가를 판단한다
bool CState::IsDefender( int nPlayer ) const
{
	return nPlayer == nMaster || nPlayer == nFriend;
}

// 현재 조커콜이 조커콜 효력을 낼 수 있는지를 검사한다
bool CState::IsEffectiveJokercall() const
{
	if ( nTurn == 0 && !pRule->bInitJokercallEffect ) return false;
	if ( bJokerUsed ) return false;
	if ( lCurrent.GetCount() > 0 ) return false;
	return true;
}

// 이 게임은 네트워크를 동반한 게임인가
bool CState::IsNetworkGame() const
{
	for ( int i = 0; i < nPlayers; i++ )
		if ( apPlayers[i]->IsNetwork() ) return true;
	return false;
}
