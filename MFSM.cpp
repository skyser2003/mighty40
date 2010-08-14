// MFSM.cpp: implementation of the CMFSM class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "Play.h"
#include "Player.h"
#include "MFSM.h"

#include "SocketBag.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CMFSM::CMFSM( LPCTSTR sRule, CPlayer* _apPlayers[], CSocketBag* pSockBag )
{
	int i;

	m_uid = -1;

	m_bTerminate = false;
	m_bTerminated = true;
	m_bTermRequest = false;

	m_pSockBag = pSockBag;

	m_pServerThread = 0;

	m_bAIThreadTerminate = false;
	m_bAIThreadTerminated = true;
	m_nAIThreadRequest = -1;
	m_pAIThread = 0;

	pRule->Decode( sRule );
	ASSERT( pRule->nPlayerNum <= MAX_PLAYERS );
	nPlayers = pRule->nPlayerNum;

	for ( i = 0; i < MAX_PLAYERS; i++ )
		m_apePlayer[i] = 0;

	for ( i = 0; i < pRule->nPlayerNum; i++ )
		m_apePlayer[i] = new CEvent( FALSE, TRUE );	// manual event
	ResetEvents();

	for ( i = 0; i < pRule->nPlayerNum; i++ ) {
		apPlayers[i] = apAllPlayers[i] = _apPlayers[i];
		apPlayers[i]->SetCurrentMFSM( this );
		apPlayers[i]->Reset();
	}
	for ( i = pRule->nPlayerNum; i < MAX_PLAYERS; i++ )
		apPlayers[i] = apAllPlayers[i] = 0;

	InitStageData( 0 );
}

CMFSM::~CMFSM()
{
	// 쓰레드를 종료한다
	m_bTerminate = true;
	m_eNotify.SetEvent();

	while ( !m_bTerminated ) Sleep(100);
	TRACE("MFSM Service Terminated\n");

	// AI 쓰레드 종료
	m_bAIThreadTerminate = true;
	m_eAIThread.SetEvent();

	while ( !m_bAIThreadTerminated ) Sleep(100);
	TRACE("MFSM AIThread Terminated\n");

	// 플레이어 자동 삭제
	for ( int i = 0; i < MAX_PLAYERS; i++ )
		delete apAllPlayers[i];

	for ( int j = 0; j < pRule->nPlayerNum; j++ )
		delete m_apePlayer[j];

	POSITION pos = m_lChatMsg.GetHeadPosition();
	while (pos) delete m_lChatMsg.GetNext(pos).pMsg;

	delete m_pSockBag;
}


// 플레이어를 표현하는 ID, UID, Num 간의 변환을 한다
// 해당 플레이어가 존재하지 않는 경우 -1 을 리턴한다 (Num 을 리턴하는 함수의 경우)
long CMFSM::GetPlayerUIDFromID( long id ) { return id < 0 ? -1 : (id+m_uid)%pRule->nPlayerNum; }
long CMFSM::GetPlayerIDFromUID( long uid ) { return uid < 0 ? -1 : (uid-m_uid+pRule->nPlayerNum)%pRule->nPlayerNum; }
long CMFSM::GetPlayerIDFromNum( long num ) { return num < 0 || num >= nPlayers ? -1 : apPlayers[num]->GetID(); }
long CMFSM::GetPlayerNumFromID( long id ) { for ( int i = 0; i < nPlayers; i++ ) if ( apPlayers[i]->GetID() == id ) return i; return -1; }
long CMFSM::GetPlayerNumFromUID( long uid ) { return GetPlayerNumFromID( GetPlayerIDFromUID(uid) ); }
long CMFSM::GetPlayerUIDFromNum( long num ) { return GetPlayerUIDFromID( GetPlayerIDFromNum(num) ); }

// 죽은 플레이어가 생겼으니, apPlayers[] 를 재배열한다
void CMFSM::RebuildPlayerArray()
{
	ASSERT( pRule->nPlayerNum >= 6 && nDeadID[0] != -1 && nDeadID[pRule->nPlayerNum-6] != -1 );

	for ( int i = 0, j = 0; i < pRule->nPlayerNum; i++ )
		if ( nDeadID[0] != i && nDeadID[pRule->nPlayerNum-6] != i ) {
			apPlayers[j] = apAllPlayers[i];
			apPlayers[j]->SetPlayerNum(j);
			// 다른 값들을 재조정한다
			if ( nMaster == i ) nMaster = j;
			if ( nBeginer == i ) nBeginer = j;
			if ( nFriend == i ) nFriend = j;
			if ( nCurrentPlayer == i ) nCurrentPlayer = j;
			j++;
		}
		else apAllPlayers[i]->SetPlayerNum(-1);

	ASSERT( nPlayers >= 6 );

	apPlayers[5] = 0;
	apPlayers[6] = 0;
	nPlayers = 5;	// 5 명이라는 확신이 있음
}

// 모든 플레이어의 카드를 옵션에 따라 정렬한다
void CMFSM::SortPlayerHand( bool bLeftKiruda, bool bLeftAce )
{
	if ( Mo()->bNoSort ) return;

	// 카드를 정렬한다
	for ( int i = 0; i < pRule->nPlayerNum; i++ )
		apAllPlayers[i]->GetHand()->Sort(
			bLeftKiruda, bLeftAce );
}

// 플레이어의 이름들을 재설정한다
void CMFSM::SetPlayerNames( LPCTSTR asName[] )
{
	for ( int i = 0; i < pRule->nPlayerNum; i++ )
		apAllPlayers[i]->SetName( asName[i] );
}

// 이 카드(pCard)로 이 플레이어(nPlayerID)가
// 다른 플레이어를 죽일 수 있는가
// 그럴 수 있다면 true, 죽일 수 없고 다른 카드를 찾아야
// 한다면(헛다리) false 를 리턴한다
bool CMFSM::KillTest( int nPlayerID, CCard* pCard, int* pDeadID )
{
	bool bKilled = false;
	for ( int i = 0; i < pRule->nPlayerNum; i++ ) {
		if ( i == nPlayerID ) continue;
		else if ( apAllPlayers[i]->GetHand()->Find( *pCard ) ) {
			bKilled = true;
			*pDeadID = i;
			break;
		}
	}
	return bKilled;
}

// (선거중) 주어진 goalNew 가 선거를 종료시킬 수
// 있는가를 판단한다 - 현재의 goal, abGiveUp, IsDealMiss() 를
// 종합적으로 고려한다
// 리턴값들 - bDealMiss : 딜 미스로 종료되었는가를 판단
//            nNextID : 당선자 ID 또는 다음 출마자 ID
// 선거를 계속 해야 하는지(false), 딜 미스나 당선 확정으로
// 종료되는지(true)를 리턴한다
bool CMFSM::CanBeEndOfElection( bool& bDealMiss, long& nNextID,
	const CGoal& goalNew )
{
	bDealMiss = false;
	nNextID = -1;

	if ( goalNew.nMinScore == -1 ) {	// 딜미스
		bDealMiss = true;
		return true;
	}
	else if ( goalNew.nMinScore == 0 ) {	// 포기
		// 다음 플레이어를 찾아 본다
		long i, nID = -1;
		for ( i = 1; i < pRule->nPlayerNum; i++ ) {
			nID = ( nCurrentPlayer + i ) % pRule->nPlayerNum;
			if ( !abGiveup[nID] ) break;
		}
		if ( i == pRule->nPlayerNum ) {	// 모두 포기 (딜미스)
			bDealMiss = true;
			return true;
		}
		else {	// nID 가 포기하지 않은 다음 플레이어
			nNextID = nID;
			// 이 플레이어 말고 모두 죽었는지 조사
			for ( i = i+1; i < pRule->nPlayerNum; i++ ) {
				nID = ( nCurrentPlayer + i ) % pRule->nPlayerNum;
				if ( !abGiveup[nID] ) return false;
			}
			// 모두 죽었음
			// 한 놈이 남았지만 아직 공약도 안정한 상태라면
			if ( goal.nMinScore == 0 ) return false;
			// 그 외에는 종료
			else return true;
		}
	}
	else {		// 출마
		ASSERT( goalNew.nMinScore > goal.nMinScore
			|| ( pRule->bNoKirudaAlways && goalNew.nMinScore >= goal.nMinScore ));

		// 풀로 올렸다면 즉시 당선
		if ( pRule->bHighScore && goalNew.nMinScore >= HIGHSCORE_MAXLIMIT
					|| goalNew.nMinScore >= MAX_SCORE 
			|| pRule->nPlayerNum == 2 && goalNew.nMinScore >= MAX_SCORE_2MA ) {
			nNextID = nCurrentPlayer;
			return true;
		}

		// 다음 플레이어를 찾아 본다
		long i, nID = -1;
		for ( i = 1; i < pRule->nPlayerNum; i++ ) {
			nID = ( nCurrentPlayer + i ) % pRule->nPlayerNum;
			if ( !abGiveup[nID] ) break;
		}
		if ( i == pRule->nPlayerNum ) {	// 모두 포기 (당선)
			nNextID = nCurrentPlayer;
			return true;
		}
		else {	// nID 가 다음 플레이어
			nNextID = nID;
			return false;
		}
	}
}

// 이 카드에 대한 설명 스트링을 리턴한다
CString CMFSM::GetHelpString( CCard c ) const
{
	static const LPCTSTR s_aNumber[] = {
		_T("2 "), _T("3 "), _T("4 "), _T("5 "),
		_T("6 "), _T("7 "), _T("8 "), _T("9 "), _T("10 "),
		_T("J "), _T("Q "), _T("K "), _T("A ") };
	static const LPCTSTR s_aHighest[] = {
		_T(" (보스카)"), _T(" (최고패)") };

	if ( (int)c == 0 ) return CString();

	bool b = Mo()->bUseTerm;
	int t = b ? 0 : 1;
	CString s = c.GetString(b);

	// 특수 카드들
	if ( c.IsMighty() || c.IsJoker() ) return s;

	// 그 모양에서 이 카드보다 높은 카드의 수를 센다 (마이티 제외)
	CString sHigh;
	int nHighers = 0;
	for ( int i = 12; i > c.GetPowerNum()-1; i-- ) {
		long flag = anUsedCards[c.GetShape()-1];
					//| anUsingCards[c.GetShape()-1];
		if ( !( flag & ( 1 << i ) )
				&& !CCard(c.GetShape(),i==12?ACE:i+2).IsMighty() ) {
			sHigh += s_aNumber[i];
			nHighers++;
		}
	}

	if ( nHighers > 0 ) return s + _T(" ( ") + sHigh + _T(')');
	else return s + s_aHighest[t];
}

// 이 함수는 프로그램 설계가 잘못되어 필요한 추한 함수다
// 플레이어가 호출하면 주공의 카드 중 acDrop 에 해당하는
// 세 장의 카드를 버리고 중간 선거 결과를
// 갱신한다 - MFSM 이 어차피 호출해 주지만
// CPlayerHuman 의 경우 버릴 카드 선택과 프랜드 선택 사이에
// 화면 Update 가 필요하기 때문에 이 함수가 필요하다
void CMFSM::RemoveDroppedCards( CCard acDrop[3], int nNewKiruda )
{
	// 추한만큼 확실히 제약한다
	ASSERT( state == msPrivilege );
	ASSERT( apPlayers[nMaster]->GetHand()->GetCount() == 13 || (nPlayers == 2 && apPlayers[nMaster]->GetHand()->GetCount() == 14 ) );	//v4.0
	ASSERT( lDeck.GetCount() == 0 );

	CCardList* pHand = apPlayers[nMaster]->GetHand();

	for ( int i = 0; i < ( nPlayers == 2 ? 1 : 3 ); i++ ) {

		POSITION pos = pHand->Find(acDrop[i]);
		ASSERT(pos);
		if ( pos ) {
			lDeck.AddTail(acDrop[i]);
			pHand->RemoveAt( pos );
		}
	}
	nThrownPoints = lDeck.GetPoint();

	goal.nKiruda = nNewKiruda;
	SortPlayerHand( Mo()->bLeftKiruda, Mo()->bLeftAce );
}

// 조커콜이 가능한 상황인지 조사하여 그렇다면 주어진
// 인자에 따라 bIsJokercall 을 세트한다
// 또 조커인 경우 nJokerShape 을 세트한다
void CMFSM::SetJokerState( CCard c, int nEffect )
{
	if ( c.IsJokercall() ) {

		if ( IsEffectiveJokercall() )
			bJokercallEffect = nEffect ? true : false;
	}
	else if ( c.IsJoker() ) {

		bJokerUsed = true;

		if ( nCurrentPlayer != nBeginer ) return;
		nJokerShape = SPADE <= nEffect && nEffect <= CLOVER
			? nEffect : SPADE;
	}
}

// 게임 종료시에 보여지는 돈 계산 리포트를 얻는다
// 배열 리턴값의 배열 크기는 플레이를 한 인원 ( 최대 5 )
void CMFSM::GetReport(
	// 리턴값
	bool& bDefWin,			// 방어(여당)측이 승리했는가 ?
	int* pnDefPointed,		// 여당 득점
	int anContrib[],		// 공헌도 (각 편에서 몇 퍼센트 정도의 활약을 했는가)
	CString asCalcMethod[2],// [0] 계산 방법과, 기본 액수의 계산 식 [1] 기타 곱절규칙
	CString asExpr[],		// 돈의 계산 식
	// 값-결과-독립변수
	int anMoney[]			// 호출시에는 원래 가졌던 돈, 리턴될 때는 계산된 돈
) const
{
#ifdef _DEBUG	// 총 돈의 합을 유지한다
	int nAllDebugSum = 0;
	for ( int debug = 0; debug < nPlayers; debug++ )
		nAllDebugSum += anMoney[debug];
#endif

	const int* anScored = aanHistory[0];
	const int* anAssist = aanHistory[1];
	const int* anTurn = aanHistory[3];

	int i;

	int nDefSum = 0, nAttSum = 0;			// 점수, 도움의 합
	int nDefPointed = 0, nAttPointed = 0;	// 점수만의 합

	// 공헌도를 계산하기 위해서 여당의 점수, 도움의 합과
	// 야당의 점수, 도움의 합을 계산해 둔다
	for ( i = 0; i < nPlayers; i++ ) {
		if ( i == nFriend || i == nMaster )
			nDefSum += anScored[i] + anAssist[i],
			nDefPointed += anScored[i];
		else nAttSum += anScored[i] + anAssist[i],
			nAttPointed += anScored[i];
	}

	// 버려진 카드의 점수를 누구에게 넣는가 처리
	if ( pRule->bAttScoreThrownPoints )
		nAttPointed += nThrownPoints;
	else if ( pRule->nPlayerNum != 2 ) nDefPointed += nThrownPoints; //2마에서는 더해지지 않는다 (v4.0 : 2010.4.18)

	// 공헌도 계산
	// 그 팀이 딴 모든 점수와 모든 도움의 합 중에서
	// 그 플레이어가 딴 점수와 도움의 합
	for ( i = 0; i < nPlayers; i++ ) {

		if ( i == nFriend || i == nMaster ) {
			if ( anContrib )
			anContrib[i] = nDefSum == 0 ? 0
				: ( anScored[i] + anAssist[i] ) * 100 / nDefSum;
		}
		else {
			if ( anContrib )
			anContrib[i] = nAttSum == 0 ? 0
				: ( anScored[i] + anAssist[i] ) * 100 / nAttSum;
		}
	}

	// 여당의 득점을 리턴
	if ( pnDefPointed ) *pnDefPointed = nDefPointed;

	// 여당의 승 패 여부를 결정
	// 득점이 목표 점수보다 작으면 패
	if ( pRule->nPlayerNum == 2 ) bDefWin = nDefPointed >= min( goal.nMinScore, MAX_SCORE_2MA );
	else bDefWin = nDefPointed >= min( goal.nMinScore, MAX_SCORE );

	/////////////////////////////////////////////////////////////////////////
	// 점수를 계산한다

	// 목(표점수), 기(본점수), (여당)득(점)
	int m = goal.nMinScore,
		b = pRule->nMinScore,
		s = nDefPointed;

	if ( goal.nKiruda==0 &&
		( ( pRule->bNoKirudaAdvantage && m == b ) || pRule->bNoKirudaAlways ) )
		b--;	// 노기루다로 1 적게 부를 수 있는 경우 기본을 1 감소시킴

	CString& sCalcMethod = asCalcMethod[0];
	CString& sResult = asCalcMethod[1];

	// 먼저 이동된 돈을 구한다
	int nMoved;		// 이동된 기본량 (돈은 아님)
	{
		//여당이 승리한 경우
		if( bDefWin ) {
			// 위험보상 2.0 ( (목-기)*2+(득-목) )
			if ( pRule->bS_Use20 ) {
				sCalcMethod.Format( _T("((%d-%d)x2+(%d-%d))"), m,b,s,m );
				nMoved = (m-b)*2+(s-m);
			}
			// 위험보상 4.0 ( (목-기)*1.5+(득-목) )
			else if ( pRule->bS_Use40 ) {
				sCalcMethod.Format( _T("((%d-%d)x1.5+(%d-%d))"), m,b,s,m );
				nMoved = (m-b)*3/2+(s-m);
			}
			// 부르는게 값 ( (목-기+1)*2 )
			else if ( pRule->bS_Call ) {
				sCalcMethod.Format( _T("((%d-%d+1)x2"), m,b );
				nMoved = (m-b+1)*2;
			}
			// Eye for an Eye ( (득-목) )
			else if ( pRule->bS_Efe ) {
				sCalcMethod.Format( _T("(%d-%d)"), s,m );
				nMoved = (s-m);
			}
			// Modified Eye for an Eye ( (득-목)+1 )
			else if ( pRule->bS_MEfe ) {
				sCalcMethod.Format( _T("(%d-%d+1)"), s,m );
				nMoved = (s-m)+1;
			}
			// Base Ten ( (득-10) )
			else if ( pRule->bS_Base10 ) {
				sCalcMethod.Format( _T("(%d-10)"), s );
				nMoved = (s-10);
			}
			// Base Thirteen ( (득-13) )
			else if ( pRule->bS_Base13 ) {
				sCalcMethod.Format( _T("(%d-13)"), s );
				nMoved = (s-13);
			}
			// Base Min ( (득-기) )
			else if ( pRule->bS_BaseM ) {
				sCalcMethod.Format( _T("(%d-%d)"), s,b );
				nMoved = (s-b);
			}
			else ASSERT(0);
		}
		else {			// 여당 패
			// Eye for an Eye ( (목-득) )
			if ( pRule->bSS_Efe ) {
				sCalcMethod.Format( _T("(%d-%d)"), m,s );
				nMoved = (m-s);
			}
			// Tooth for an Tooth ( (목-득) or (목-13)+(13-득)*2 )
			else if ( pRule->bSS_Tft ) {
				if ( s >= b ) {
					sCalcMethod.Format( _T("(%d-%d)"), m,s );
					nMoved = (m-s);
				}
				else {
					sCalcMethod.Format( _T("(%d-%d)+(%d-%d)x2"), m,b,b,s );
					nMoved = (m-b+(b-s)*2);
				}
			}
			else ASSERT(0);
		}

		// 2배 규칙
		if ( nMoved < 0 ) nMoved = 0;				// 이길 경우 최소한 0점을 획득하게 함

		if ( bDefWin && nDefPointed >= MAX_SCORE 
			|| pRule->nPlayerNum == 2 && nDefPointed >= MAX_SCORE_2MA ) {	// 런
			if ( pRule->bS_StaticRun ) {
				sCalcMethod.Format( _T("%d(고정)"), 20 );	// 고정액
				nMoved = 20;
			}
			else if ( pRule->bS_DoubleForDeclaredRun ) {
				if ( goal.nMinScore >= MAX_SCORE || pRule->nPlayerNum == 2 && goal.nMinScore >= MAX_SCORE_2MA ) {
					sResult += _T(" x2(런)");	// 런
					nMoved *= 2;
				}
			} else if ( pRule->bS_DoubleForRun ) {
				sResult += _T(" x2(런)");		// 선언된 런
				nMoved *= 2;
			}
		}
		if ( !bDefWin && pRule->bS_DoubleForReverseRun ) { // 백런 검사
			if( pRule->bS_AGoalReverse && nAttPointed >= goal.nMinScore ) {
				sResult += _T(" x2(백런)");
				nMoved *= 2;
			}
			else if( pRule->bS_A11Reverse && nAttPointed >= 11 ) {
				sResult += _T(" x2(백런)");
				nMoved *= 2;
			}
			else if( pRule->bS_AMReverse && nAttPointed >= pRule->nMinScore) {
				sResult += _T(" x2(백런)");
				nMoved *= 2;
			}
		}
		if ( pRule->bS_DoubleForNoKiruda			// 노기루다
			&& bDefWin && goal.nKiruda == 0 ) {
			sResult += _T(" x2(노기)");
			nMoved *= 2;
		}
		if ( pRule->bS_DoubleForNoFriend && pRule->bFriend	// 프랜드 제도가 있는 경우 노프 2배가 가능
				&& bDefWin && goal.nFriend == 0 ) {
			sResult += _T(" x2(노프)");
			nMoved *= 2;
		}
	}

	CString sMoved; sMoved.Format(_T(" = [%d]"), nMoved );
	sResult += sMoved;

	// 모은 돈을 임시 저장하는 곳
	int nMoneySum = 0;

	int nAllMoney = BASE_MONEY * pRule->nPlayerNum;
	CString sMultiply;
	if ( Mo()->bMoneyAsSupport ) {
		int nMilli = 10000/nAllMoney%10;
		if ( !nMilli )
			sMultiply.Format( _T("%d.%dx"), 100/nAllMoney, 1000/nAllMoney%10 );
		else sMultiply.Format( _T("%d.%d%dx"), 100/nAllMoney,
			1000/nAllMoney%10, nMilli );
	}
	else sMultiply.Format( _T("%dx"), MONEY_UNIT );

	// 야당이 주거나 내놓을 돈을 계산하고
	// 계산 식을 리턴값에 세트한다
	for ( i = 0; i < nPlayers; i++ )
		if ( i != nMaster && i != nFriend ) {

			if ( asExpr )
				asExpr[i].Format(_T("%s %c %s[기본]"),
					::FormatMoney( anMoney[i], pRule->nPlayerNum ),
					bDefWin ? _T('-') : _T('+'),
					sMultiply );

			if ( bDefWin ) {
				anMoney[i] -= nMoved;
				nMoneySum += nMoved;
			}
			else {
				anMoney[i] += nMoved;
				nMoneySum -= nMoved;
			}
		}

	// 이제 nMoneySum 을 처리한다
	// 돈이 양수만큼 모였으면(여당승) 프랜드와 주공이 나눠갖고
	// 음수만큼 모였으면(야당승) 나눠낸다

	// 프랜드와 주공이 몇 대 몇씩 나눠야 하는가
	int nMasterShare, nFriendShare;
	{
		if ( !pRule->bFriend
			|| !goal.nFriend )	// 프랜드가 없는 경우
			nMasterShare = nPlayers - 1, nFriendShare = 0;	// 1:0
		else if ( nPlayers-2 < 2 )	// 야당이 여당보다 적은 경우
			nMasterShare = 1, nFriendShare = 1;	// 1:1
		else	// 정상적인 경우
			// 프랜드가 1 명치를 갖고 주공이 나머지를 다 갖는다
			nMasterShare = nPlayers - 3,
			nFriendShare = 1;
	}

	// 계산식을 리턴값에 세트

	// 주공
	if ( asExpr )
		asExpr[nMaster].Format(_T("%s %c %s[기본]"),
			::FormatMoney( anMoney[nMaster], pRule->nPlayerNum ),
			bDefWin ? _T('+') : _T('-'),
			sMultiply );

	if ( nMasterShare > 1 ) {
		CString s; s.Format( _T("x%d명"), nMasterShare );
		if ( asExpr ) asExpr[nMaster] += s;
	}

	anMoney[nMaster] +=
		nMoneySum * nMasterShare / (nMasterShare+nFriendShare);

	nMoneySum -=
		nMoneySum * nMasterShare / (nMasterShare+nFriendShare);

	// 프랜드
	if ( pRule->bFriend && goal.nFriend ) {
		if ( asExpr )
			asExpr[nFriend].Format(_T("%s %c %s[기본]"),
				::FormatMoney( anMoney[nFriend], pRule->nPlayerNum ),
				bDefWin ? _T('+') : _T('-'),
				sMultiply );
		anMoney[nFriend] += nMoneySum;
		nMoneySum = 0;
	}

	// 음수가 된 돈을 보정한다
	int nInsuff = 0;
	for ( i = 0; i < nPlayers; i++ )
		if ( anMoney[i] < 0 ) {
			nInsuff += anMoney[i];
			anMoney[i] = 0;
		}

	if ( nInsuff < 0 ) {	// 돈이 모자름

		if ( bDefWin )	// 여당이 이겼으면 주공이 덜받는다
			anMoney[nMaster] += nInsuff;
		else {			// 야당이 이겼으면 조금씩 덜받는다
			int nAttCount =	// 야당명수
				nPlayers - 1 /*주공*/
				- ( ( pRule->bFriend && goal.nFriend )
						? 1 : 0 ); /*프랜드*/
			int nInsuffShare = nInsuff / nAttCount;

			for ( int i = 0; i < nPlayers; i++ )
				if ( i != nMaster && i != nFriend ) {
					// 야당이면 몫을 나눠 가짐
					nAttCount--;
					if ( nAttCount > 0 ) {
						anMoney[i] += nInsuffShare;
						nInsuff -= nInsuffShare;
					}
					else {
						anMoney[i] += nInsuff;
						nInsuff = 0;
					}
				}
			ASSERT( nInsuff == 0 );
		}
	}

#ifdef _DEBUG	// 총 돈의 합계를 비교한다
	int nAllDebugSum2 = 0;
	for ( debug = 0; debug < nPlayers; debug++ )
		nAllDebugSum2 += anMoney[debug];
	ASSERT( nAllDebugSum == nAllDebugSum2 );
#endif
}
