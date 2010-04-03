// PlayerHuman.cpp: implementation of the CPlayerHuman class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "Play.h"
#include "player.h"
#include "PlayerHuman.h"

#include "BmpMan.h"
#include "Board.h"
#include "InfoBar.h"
#include "MFSM.h"

#include "BoardWrap.h"
#include "DSB.h"
#include "DEtc.h"
#include "DElection.h"
#include "DReport.h"
#include "DScoreBoard.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// CPlayerHuman

CPlayerHuman::CPlayerHuman(
		int nID,		// 고유 번호 ( 7명의 플레이어중 순서 )
		LPCTSTR sName,	// 이름
		CWnd* pCallback )// call-back window
	: CPlayer( nID, sName, pCallback )
{
	m_pBoard = (CBoardWrap*)((CBoard*)pCallback)->GetWrapper();

	// 채팅창 핸들러
	Ib()->SetChatHandler( (DWORD)(LPVOID)this, ChatProc );
}

CPlayerHuman::~CPlayerHuman()
{
	Ib()->SetChatHandler( (DWORD)(LPVOID)this, 0 );

	if ( m_pMFSM->GetState()->IsNetworkGame() ) {
		// 네트워크 게임이었다면, 전적을 영구적으로 Update 한다
		Mo()->anPlayerState[0] = MAKELONG( m_recAll.wm, m_recAll.lm );
		Mo()->anPlayerState[1] = MAKELONG( m_recAll.wf, m_recAll.lf );
		Mo()->anPlayerState[2] = MAKELONG( m_recAll.wa, m_recAll.la );
	}
}

// 헬퍼 함수들

const CState* CPlayerHuman::GetState()
{
	return m_pMFSM->GetState();
}

// 탈락시킬 카드를 추천한다
int CPlayerHuman::GetRecommendedKillCard( const CCardList* plDead )
{
	// 손에 없는 최고 기루다를 찾는다

	int nKiruda = GetState()->goal.nKiruda;
	int nFrom, nTo;

	if ( !nKiruda )	// nokiruda
		nFrom = SPADE, nTo = CLOVER;
	else nFrom = nKiruda, nTo = nKiruda;

	for ( int s = nFrom; s <= nTo; s++ )
		if ( !GetHand()->Find( CCard( s, ACE ) )
			&& !plDead->Find( CCard( s, ACE ) ) )
			return (int)CCard( s, ACE );
		else for ( int i = KING; i > 2; i-- )
			if ( !GetHand()->Find( CCard( s, i ) )
				&& !plDead->Find( CCard( s, i ) ) )
				return (int)CCard( s, i );
	ASSERT(0);
	return (int)CCard(nKiruda,2);	// never reached
}

static bool find_both( const CCardList* p1, const CCardList* p2, CCard c )
{	return p1->Find(c) || p2->Find(c); }

// 프랜드 카드를 추천한다
int CPlayerHuman::GetRecommendedFriend( const CCardList* plDeck )
{
	int i;
	CCard c;
	const CCardList* plHand = GetHand();

	// 먼저, 마이티 !
	c = CCard::GetMighty();
	if ( !find_both( plHand, plDeck, c ) ) return c;

	// 다음, 조커
	c = CCard::GetJoker();
	if ( GetState()->pRule->bJokerFriend
		&& !find_both( plHand, plDeck, c ) ) return c;

	// 기A
	c = CCard( CCard::GetKiruda(), ACE );
	if ( !find_both( plHand, plDeck, c ) ) return c;

	// 기 K
	c = CCard( CCard::GetKiruda(), KING );
	if ( !find_both( plHand, plDeck, c ) ) return c;

	// 다른 ACE
	for ( i = SPADE; i <= CLOVER; i++ ) {
		c = CCard( i, ACE );
		if ( !find_both( plHand, plDeck, c ) ) return c;
	}

	// 기 Q
	c = CCard( CCard::GetKiruda(), QUEEN );
	if ( !find_both( plHand, plDeck, c ) ) return c;

	return 1;	// 초구
}

// p 에 있는 카드를 뷰에서 선택한다
void CPlayerHuman::SetCurrentSelectedCard( CCardList* p )
{
	m_pBoard->SetSelection( false );
	// 이제 카드 리스트를 보면서 다시 선택 카드 배열을 세트한다
	POSITION pos = p->GetHeadPosition();
	while (pos) m_pBoard->SetSelection(
		GetHand()->IndexFromPOSITION(
			GetHand()->Find( p->GetNext(pos) ) ) );
}

// 현재 턴 상태에서 적절한 사운드를 연주한다
void CPlayerHuman::PlayTurnSound()
{
	CCard c( GetState()->cCurrentCard );

	// 마이티 사운드
	if ( c.IsMighty() )
		PlaySound( IDW_MIGHTY, true );
	// 조커콜 사운드
	else if ( c.IsJokercall()
		&& GetState()->nBeginer == GetState()->nCurrentPlayer ) {

		if ( GetState()->bJokercallEffect ) {
			// 진짜 조커콜
			(new DShortMessage(m_pBoard))->Create(
				0, Mo()->bUseTerm ? _T("쪼카컴 !") : _T("조커 콜 !"),
				true, false, DELAY_JOKERDSB );
			PlaySound( IDW_JOKERCALL, true );
		}
		else {
			// 조커콜 아님
			(new DShortMessage(m_pBoard))->Create(
				0, Mo()->bUseTerm ? _T("쪼콜 아님") : _T("조커콜 아님"),
				true, false, DELAY_JOKERDSB );
			PlaySound( IDW_CARD );
		}
	}
	// 죽은 조커 사운드
	else if ( c.IsJoker() && GetState()->bJokercallEffect )
		PlaySound( IDW_KILLEDJOKER );	// 죽은 조커는 소리 우선순위가 낮게
	// 조커 사운드
	else if ( c.IsJoker() ) {

		if ( GetState()->nBeginer == GetState()->nCurrentPlayer ) {
			// 선인 경우
			static LPCTSTR s_asJokerShape[2][5] = {
				{ _T(" 삽컴 !"), _T(" 다리컴 !"), _T(" 트컴 !"), _T(" 끌컴 !") },
				{ _T(" 스페이드 !"), _T(" 다이아몬드 !"), _T(" 하트 !"), _T(" 클로버 !") }
			};

			(new DShortMessage(m_pBoard))->Create(
				IDB_SPADE + GetState()->nJokerShape - SPADE,
				s_asJokerShape[Mo()->bUseTerm?0:1][GetState()->nJokerShape-SPADE],
				true, true, DELAY_JOKERDSB );
		}
		PlaySound( IDW_JOKER, true );
	}
	// 찍기 (겐)
	else if ( c.IsKiruda() && GetState()->lCurrent.GetCount() > 0
		&& !GetState()->lCurrent.GetHead().IsJoker()
		&& !GetState()->lCurrent.GetHead().IsKiruda() ) {
		PlaySound( IDW_KIRUDA, true );
	}
	// 일반
	else PlaySound( IDW_CARD );
}

// 조커, 조커콜을 처리한다 (취소시 true 리턴)
bool CPlayerHuman::ProcessSpecialCards( CCard c, int* eff )
{
	const CState* pState = GetState();

	if ( c.IsJoker() )	{	// 조커인 경우

		if ( pState->nCurrentPlayer != pState->nBeginer )
			// 선이 아니면 상관 없다
			return false;

		static LPCTSTR s_asJokerShape[2][6] = {
			{ _T("기루다컴 !"), _T("삽컴 !"), _T("다리컴 !"), _T("트컴 !"), _T("끌컴 !"), _T("취소") },
			{ _T("기루다"), _T("스페이드"), _T("다이아몬드"), _T("하트"), _T("클로버"), _T("취소") }
		};

		// 현재 마우스 위치
		CPoint pnt; ::GetCursorPos(&pnt);
		((CWnd*)*m_pBoard)->ScreenToClient(&pnt);

		// 선택 상자를 표시
		long nSel = 0;
		DSelect* pSelDSB = new DSelect(m_pBoard);
		pSelDSB->Create( pnt.x+5, pnt.y,
			s_asJokerShape[Mo()->bUseTerm?0:1], 6,
			&m_eTemp, &nSel );

		m_pMFSM->WaitEvent( &m_eTemp );

		m_pBoard->UpdatePlayer( 0 );

		if ( nSel == 5 || nSel == -1 ) return true;	// 취소

		if ( nSel == 0 ) {
			nSel = GetState()->goal.nKiruda;
			if ( nSel == 0 ) return true;	// 노기루다
		}

		*eff = nSel;

		return false;
	}
	else if ( c.IsJokercall() ) {	// 조커콜인 경우

		// 일단 조커콜이 가능한 상황인가를 본다

		if ( pState->nCurrentPlayer != pState->nBeginer )
			// 선이 아니면 조커콜 불가능
			return false;
		if ( pState->bJokerUsed )
			// 조커가 이미 나와버린 경우
			return false;
		if ( !pState->pRule->bInitJokercallEffect && pState->nTurn == 0 )
			// 첫턴 조커콜 효력없음
			return false;

		static LPCTSTR s_asJokercall[2][3] = {
			{ _T("쪼카컴 !"), _T("쪼콜아님"), _T("취소") },
			{ _T("조커 콜 !"), _T("조커 콜 아님"), _T("취소") }
		};

		// 현재 마우스 위치
		CPoint pnt; ::GetCursorPos(&pnt);
		((CWnd*)*m_pBoard)->ScreenToClient(&pnt);

		// 선택 상자를 표시
		long nSel = 0;
		DSelect* pSelDSB = new DSelect(m_pBoard);
		pSelDSB->Create( pnt.x+5, pnt.y,
			s_asJokercall[Mo()->bUseTerm?0:1], 3,
			&m_eTemp, &nSel );

		m_pMFSM->WaitEvent( &m_eTemp );

		m_pBoard->UpdatePlayer( 0 );

		*eff = nSel == 0 ? 1 : 0;

		if ( nSel == 2 || nSel == -1 ) // 취소
			return true;
		else return false;
	}
	else return false;	// 그 외의 카드
}

// 채팅창 핸들러
void CPlayerHuman::ChatProc( LPCTSTR s, DWORD dwUser )
{
	CPlayerHuman* pThis = (CPlayerHuman*)(LPVOID)dwUser;
	CMsg* pMsg = new CMsg( _T("lls"), CMsg::mmChat,
		pThis->m_pMFSM->GetPlayerUIDFromID( pThis->GetID() ), s );
	pThis->m_pMFSM->EventChat( pMsg, true );
}


// CPlayer 인터페이스 구현

// 한 판이 시작됨 - OnBegin 전, 아직 덱이 분배되지 않은 상태
void CPlayerHuman::OnInit( CEvent* e )
{
	// 변수 초기화
	m_clTemp.RemoveAll();

	// Board 를 초기화
	m_pBoard->SetDealMiss(-1);
	m_pBoard->SetSelection(false);
	m_pBoard->SetElectionDSB( 0 );
	m_pBoard->SetTurnRect( -1 );
	Ib()->Reset();

	// 점수판을 갱신
	Sb()->Update(GetState());

	// 짝! 소리를 낸다
	PlaySound( IDW_CARDSETUP, true );
	m_pBoard->UpdatePlayer( -2 );

	// 조금 기다린다 (DELAY_SETUP_SUFFLING)
	if ( Mo()->bShowDealing )
		Sleep( DELAY_SETUP_SUFFLING );

	// 카드를 섞기 시작한다
	// 다 섞이면 e 가 세트될 것이다
	if ( Mo()->bShowDealing )
		m_pBoard->SuffleCards( 2, e );
	else e->SetEvent();
}

// 카드를 나눠 주고 있음 ( nFrom 에서 nTo 로 (-1은 중앙)
// nCurrentCard 가 이동하였음 )
// nMode  0 : 카드를 날리지 않고, 단지 전체 화면 갱신 필요
//        1 : 카드 날리기
//        2 이상 : Mo()->bShowDealing 에 관계 없이 꼭 날려야 함
//        3 이상 : 속도를 너무 빠르게 하면 안됨
//        9 이상 : 카드 선택
//        10 : 주공의 Privilege 단계임, 속도는 느리게, 소리 연주
void CPlayerHuman::OnDeal( int nFrom, int nTo, int nMode, int nCard, CEvent* e )
{
	if ( nMode == 0 ) {
		m_pBoard->SetTurnRect( -1 );
		m_pBoard->UpdatePlayer( -2 );
		e->SetEvent();
		return;
	}

	// 소스(nFrom)의 그림을 다시 갱신한다
	m_pBoard->UpdatePlayer( nFrom, 0, true );

	if ( nMode == 10 )
		PlaySound( IDW_GETCARD, true );

	if ( Mo()->bShowDealing ) {
		// 카드를 날려야 하면

		int nSpeed =  nMode == 10 ? 0 : nMode >= 3  ? 2 : 1;

		int nFlyingCard = 0;
		// 버린 카드 야당 득점이면 주공이 세장 가지는 카드를 보여준다
		if ( nMode == 10 && GetState()->pRule->bAttScoreThrownPoints )
			nFlyingCard = nCard;

		// 카드를 날린다
		m_pBoard->FlyCard( nFlyingCard, nSpeed,
			nFrom == -1 ? 4 : 2, nFrom, -1,
			nTo == -1 ? 4 : 2, nTo, -1, &m_eTemp );

		// 다 날릴때까지 기다린다
		m_pMFSM->WaitEvent( &m_eTemp );
	}

	bool bUpdateSelectionRegion = false;
	if ( nMode >= 9 && nTo == 0 && GetState()->nMaster == 0
			&& GetState()->apPlayers[0]->IsHuman() ) {
		m_clTemp.AddTail(nCard);
		SetCurrentSelectedCard( &m_clTemp );
		bUpdateSelectionRegion = true;
	}

	// 카드가 무사히 도착했으므로 목적지(to)를 다시 그린다
	Sleep(0);
	m_pBoard->UpdatePlayer( nTo, 0, false, bUpdateSelectionRegion );
	e->SetEvent();
}

// 하나의 게임이 시작됨
// 이 게임에서 사용되는 상태 변수를 알려준다
// 이 상태 변수는 CCard::GetState() 로도 알 수 있다
// pState->nCurrentPlayer 값이 바로 자기 자신의
// 번호이며, 이 값은 자신을 인식하는데 사용된다
void CPlayerHuman::OnBegin( const CState* pState, CEvent* e )
{
	// 솨악! 하는 시작 소리를 낸다
	PlaySound( IDW_BEGIN, true );

	// 카드를 정렬하고 이름을 다시 그리기 위해서 전체 화면을 갱신한다 !
	m_pMFSM->SortPlayerHand( Mo()->bLeftKiruda, Mo()->bLeftAce );
	m_pBoard->UpdatePlayer( -2 );

	// 시작한다는 DSB 를 띄운다
	// 선거 회수를 메시지 박스로 출력한다
	CString sMsg;
	if ( GetState()->nGameNum == 1 ) sMsg = _T("초대 선거");
	else sMsg.Format( _T("제 %d 대 대선"), GetState()->nGameNum );

	DShortMessage* pDSB = new DShortMessage( m_pBoard );
	pDSB->Create( 0, sMsg, true, false, -1 );
	m_pBoard->SetElectionDSB( pDSB );

	CPlayer::OnBegin( pState, e );
}

// 6마에서 당선된 경우 한 사람을 죽여야 한다
// 죽일 카드를 지정하면 된다 - 단 이 함수는
// 반복적으로 호출될 수 있다 - 이 경우
// CCardList 에 지금까지 실패한 카드의 리스트가
// 누적되어 호출된다
// 5번 실패하면 (이 경우 알고리즘이 잘못되었거나
// 사람이 잘 못 선택하는 경우) 임의로 나머지 5명 중
// 하나가 죽는다 !
void CPlayerHuman::OnKillOneFromSix( CCard* pcCardToKill,
	CCardList* plcFailedCardsTillNow, CEvent* e )
{
	// 죽이기 DSB 를 표시
	DKill* pDSB = new DKill(m_pBoard);
	*(CCard*)pcCardToKill =
		GetRecommendedKillCard(
			(CCardList*)plcFailedCardsTillNow );
	pDSB->Create( e, (CCard*)pcCardToKill,
		(CCardList*)plcFailedCardsTillNow, GetHand() );
}

// 주공이 다른 플레이어를 죽인다
// bKilled : 참이면 실제로 죽였고, 거짓이면 헛다리 짚었다
void CPlayerHuman::OnKillOneFromSix( CCard cKill,
	bool bKilled, CEvent* e )
{
	const CState* pState = m_pMFSM->GetState();

	// 일단 조커콜 소리를 낸다
	PlaySound( IDW_JOKERCALL, true );

	// DSB 를 표시
	DDeadDecl* pDecl = new DDeadDecl( m_pBoard );
	pDecl->Create(
		pState->apAllPlayers[pState->nMaster]->GetName(),
		(int)cKill, false, -1 );
	m_pBoard->SetElectionDSB( pDecl );

	// 실패 !!
	if ( !bKilled ) {

		// 잠시 지연
		Sleep( DELAY_KILL_AND_EFFECT );

		// 뽀록 소리를 낸다
		PlaySound( IDW_GIVEUP, true );

		// 실패 DSB 를 표시
		DDeadDecl* pDecl = new DDeadDecl( m_pBoard );
		pDecl->Create(
			pState->apAllPlayers[pState->nMaster]->GetName(),
			(int)cKill, true, -1 );
		m_pBoard->SetElectionDSB( pDecl );

		// 또다시 지연
		Sleep( DELAY_KILL_AND_EFFECT );
	}
	e->SetEvent();
}
	// 7마에서 당선된 경우 두 사람을 죽여야 한다
	// 이 함수는 그 중 하나만 죽이는 함수로,
	// OnKillOneFromSix와 같다.
	// 5번 실패하면 (이 경우 알고리즘이 잘못되었거나
	// 사람이 잘 못 선택하는 경우) 임의로 나머지 5명 중
	// 하나가 죽는다 !
void CPlayerHuman::OnKillOneFromSeven( CCard* pcCardToKill,
	CCardList* plcFailedCardsTillNow, CEvent* e )
{
	// 죽이기 DSB 를 표시
	DKill* pDSB = new DKill(m_pBoard);
	*(CCard*)pcCardToKill =
		GetRecommendedKillCard(
			(CCardList*)plcFailedCardsTillNow );
	pDSB->Create( e, (CCard*)pcCardToKill,
		(CCardList*)plcFailedCardsTillNow, GetHand() );
}

// 7마에서 주공이 다른 플레이어를 죽인다
// bKilled : 참이면 실제로 죽였고, 거짓이면 헛다리 짚었다
void CPlayerHuman::OnKillOneFromSeven( CCard cKill,
	bool bKilled, CEvent* e )
{
	const CState* pState = m_pMFSM->GetState();

	// 일단 조커콜 소리를 낸다
	PlaySound( IDW_JOKERCALL, true );

	// DSB 를 표시
	DDeadDecl* pDecl = new DDeadDecl( m_pBoard );
	pDecl->Create(
		pState->apAllPlayers[pState->nMaster]->GetName(),
		(int)cKill, false, -1 );
	m_pBoard->SetElectionDSB( pDecl );

	// 실패 !!
	if ( !bKilled ) {

		// 잠시 지연
		Sleep( DELAY_KILL_AND_EFFECT );

		// 뽀록 소리를 낸다
		PlaySound( IDW_GIVEUP, true );

		// 실패 DSB 를 표시
		DDeadDecl* pDecl = new DDeadDecl( m_pBoard );
		pDecl->Create(
			pState->apAllPlayers[pState->nMaster]->GetName(),
			(int)cKill, true, -1 );
		m_pBoard->SetElectionDSB( pDecl );

		// 또다시 지연
		Sleep( DELAY_KILL_AND_EFFECT );
	}
	e->SetEvent();
}

// 공약을 듣는다
// pNewGoal 과 state.goal 에 현재까지의 공약이 기록되어 있다
// 패스하려면 nMinScore 를 0으로 세트
// 특히 딜 미스를 원하는 경우 공약을 -1로 세트
// pNewGoal.nFriend 는 사용하지 않는다
void CPlayerHuman::OnElection( CGoal* pNewGoal, CEvent* e )
{
	DGoal* pDSB = new DGoal(m_pBoard);
	pDSB->Create( false, m_pMFSM->GetState(), e, pNewGoal );
}

// 다른 사람의 선거 결과를 듣는다
void CPlayerHuman::OnElecting( int nPlayerID, int nKiruda,
	int nMinScore, CEvent* e )
{
	m_goal.nKiruda = nKiruda;
	m_goal.nMinScore = nMinScore;
	m_goal.nFriend = 0;
	m_pBoard->FloatGoalDSB( nPlayerID, &m_goal );

	bool bDealMiss;
	long nNextID;

	m_pMFSM->CanBeEndOfElection( bDealMiss, nNextID, m_goal );

	if ( bDealMiss ) {	// 딜미스

		PlaySound( IDW_CARDSET );

		if ( nMinScore == -1 ) {	// 개인 딜미스
			m_pBoard->SetDealMiss(nPlayerID);
			m_pBoard->UpdatePlayer( nPlayerID, 0 );
		}
		else { // 전체 딜미스
			DShortMessage* pSM = new DShortMessage(m_pBoard);
			pSM->Create( 0, _T("딜 미스 !"), true, true, DELAY_DEALMISS );
			m_pBoard->SetElectionDSB( pSM );
		}
		Sleep(DELAY_DEALMISS);
	}

	e->SetEvent();
}

// 주공이 정해졌다
void CPlayerHuman::OnElected( int nPlayerID, CEvent* e )
{
	const CState* pState = GetState();

	Ib()->SetText( pState->apAllPlayers[nPlayerID]->GetName()
		+ _T(" 님이 당선되었습니다") );

	// 마이티, 조커콜이 바뀌었을지도 모르기 때문에
	// 보이는 카드를 갱신한다
	m_pBoard->UpdatePlayer( -2 );

	Sleep( DELAY_MASTERDECL );

	// 짠~ 소리를 내고
	PlaySound( IDW_NOTIFY, true );

	// DSB 를 하나 띄운다

	DMasterDecl* pDecl = new DMasterDecl( m_pBoard );
	pDecl->Create(
		true,			// bTemp
		nPlayerID == 0,	// bHuman
		pState->pRule->nPlayerNum == 6,	// bToKill
		pState->pRule->bFriend,			// bUseFriend
		pState->pRule->nMinScore,		// nDefaultMinScore
		pState->goal,	// goal
		pState->apAllPlayers[nPlayerID]->GetName(),	// sMasterName
		0,	// sFriendName
		-1 );	// nTimeOut
	m_pBoard->SetElectionDSB( pDecl );
	// 이전에 떠있던 목표 상자는 지운다
	m_pBoard->FloatGoalDSB( -1, 0 );

	Ib()->SetKiruda( pState->goal.nKiruda );
	Ib()->SetMinScore( pState->goal.nMinScore, pState->pRule->nMinScore );
	Ib()->SetFriendText( _T("-미정-") );

	Sleep( DELAY_MASTERDECL );

	e->SetEvent();
}

// 당선된 경우
// pNewGoal 과 state.goal 에 현재까지의 공약이 기록되어 있다
// pNewGoal 에 새로운 공약을 리턴하고 (nFriend포함)
// acDrop 에 버릴 세 장의 카드를 리턴한다
// * 주의 * 이 함수 이후에 한명이 죽어서 플레이어의
// 번호(pState->nCurrentPlayer)가 변경되었을 수 있다
void CPlayerHuman::OnElected( CGoal* pNewGoal, CCard acDrop[3], CEvent* e )
{
	// 목표 수정 & 버릴카드 고르기
	DGoal* pGoalDSB = new DGoal(m_pBoard);
	pGoalDSB->Create( true, m_pMFSM->GetState(), &m_eTemp, pNewGoal );
	m_pBoard->SetElectionDSB( pGoalDSB );

	m_pMFSM->WaitEvent( &m_eTemp );

	// 버릴 카드를 알아온다
	int i, j;
	POSITION pos = GetHand()->GetHeadPosition();
	for ( i = j = 0; pos; i++ ) {
		CCard c = GetHand()->GetNext(pos);
		if ( m_pBoard->GetSelection(i) )
			acDrop[j++] = c;
	}
	ASSERT(j==3);

	// 화면을 업데이트 ( 소리도 함께 )
	PlaySound( IDW_CARDSET, true );
	m_pBoard->SetSelection( false );
	m_pMFSM->RemoveDroppedCards( acDrop, pNewGoal->nKiruda );
	m_pBoard->UpdatePlayer( -2 );
	Ib()->SetKiruda( pNewGoal->nKiruda );
	Ib()->SetMinScore( pNewGoal->nMinScore, GetState()->pRule->nMinScore );

	// 프랜드를 정한다
	if ( m_pMFSM->GetState()->pRule->bFriend ) {
		// 프랜드 제도가 있을때만

		DFriend* pFriendDSB = new DFriend(m_pBoard);
		// 추천 프랜드와 함께
		pNewGoal->nFriend = GetRecommendedFriend( &GetState()->lDeck );
		pFriendDSB->Create( e, &pNewGoal->nFriend, GetState() );
	}
	else {	// 없으면 수동으로 이벤트를 세트
		pNewGoal->nFriend = 0;
		e->SetEvent();
	}
}

// 선거가 끝났고 본 게임이 시작되었음을 알린다
// * 주의 * 이 함수 이후에 한명이 죽어서 플레이어의
// 번호(pState->nCurrentPlayer)가 변경되었을 수 있다
void CPlayerHuman::OnElectionEnd( CEvent* e )
{
	// 공지 상자
	PlaySound( IDW_NOTIFY, true );
	m_pBoard->UpdatePlayer( -2 );
	DMasterDecl* pDecl = new DMasterDecl(m_pBoard);
	pDecl->Create( false,	// bTemp
		GetState()->nDeadID != 0 && GetState()->nMaster == 0,
		GetState()->pRule->nPlayerNum == 6,
		GetState()->pRule->bFriend,
		GetState()->pRule->nMinScore,
		GetState()->goal,
		GetState()->apPlayers[GetState()->nMaster]->GetName(),
		GetState()->nFriend >= 0 ?
			GetState()->apPlayers[GetState()->nFriend]->GetName()
			: _T(""),
		DELAY_ELECTIONEND );
	m_pBoard->SetElectionDSB( pDecl );

	// 상태 바에 선거 정보를 출력한다

	Ib()->Reset();
	Ib()->SetKiruda( GetState()->goal.nKiruda );
	Ib()->SetMinScore( GetState()->goal.nMinScore, GetState()->pRule->nMinScore );

	int nFriend = GetState()->goal.nFriend;
	CString sFriend;
	if ( nFriend == 0 ) sFriend = _T("노");
	else if ( nFriend == 100 ) sFriend = _T("초구");
	else if ( nFriend < 0 ) sFriend = GetState()->apPlayers[GetState()->nFriend]->GetName();
	else sFriend = CCard(nFriend).GetString( Mo()->bUseTerm );
	sFriend += _T(" 프랜드");
	Ib()->SetFriendText( sFriend );

	e->SetEvent();
}

// 카드를 낸다
// pc 는 낼 카드, pe 는 pc가 조커인경우 카드 모양,
// pc가 조커콜인경우 실제 조커콜 여부(0:조커콜 아님)
// 그 외에는 무시
void CPlayerHuman::OnTurn( CCard* pc, int* pe, CEvent* e )
{
	*pe = 0;
	long nCard = int(*pc);

	do {
		m_pBoard->WaitSelection( &m_eTemp, &nCard );
		m_pMFSM->WaitEvent( &m_eTemp );

		*pc = CCard(nCard);

		m_pBoard->CancelSelection();

		// 조커나 조커콜이면 특수한 처리를 더 한다
	} while ( ProcessSpecialCards( *pc, pe ) );

	e->SetEvent();
}

// 카드를 냈음을 알려준다
// 모든 정보는 State 에 있다
// nHandIndex 는 이 카드가 손에든 카드중 몇번째 인덱스의 카드였는가
void CPlayerHuman::OnTurn( CCard, int, int nHandIndex, CEvent* e )
{
	// 카드를 날린다
	m_pBoard->FlyCard( GetState()->cCurrentCard, 0,
		2, GetState()->nCurrentPlayer,
			GetState()->nCurrentPlayer ? -1 : nHandIndex,
		0, -1, GetState()->nCurrentPlayer,
		&m_eTemp );

	// 플레이어 갱신
	m_pBoard->UpdatePlayer( GetState()->nCurrentPlayer );

	m_pMFSM->WaitEvent( &m_eTemp );

	// 적절한 ! 소리를 출력한다
	PlayTurnSound();

	// 다시 가운데 영역의 갱신
	m_pBoard->UpdatePlayer( -1 );

	e->SetEvent();
}

// 턴이 종료되기 전에, 딴 카드를 회수하는 날리기 단계
// cCurrentCard 가 현재 날아가는 카드
void CPlayerHuman::OnTurnEnding( int nWinner, CEvent* e )
{
	// 상태바 갱신
	if ( GetState()->apPlayers[nWinner]->GetHand()->GetCount() == 0 ) {
		m_pBoard->SetTurnRect( -1 );
		Ib()->SetText( _T("게임 종료") );
	}
	else {
		m_pBoard->SetTurnRect( nWinner );
		Ib()->SetText( GetState()->apPlayers[nWinner]->GetName()
		+ _T(" 님이 이번 턴을 이겼습니다") );
	}

	CCard c(GetState()->cCurrentCard);

	if ( c.IsPoint() && GetState()->nMaster != nWinner
		&& ( GetState()->nFriend != nWinner
			|| !GetState()->bFriendRevealed ) ) {
		// 점수카드이고 야당이면 득점 영역으로

		m_pBoard->FlyCard( (int)c, 2,
			0, -1,
			( GetState()->nBeginer + GetState()->lCurrent.GetCount() )
					% GetState()->nPlayers,
			1, nWinner, -1,
			&m_eTemp );

		m_pBoard->UpdatePlayer( -1 );

		m_pMFSM->WaitEvent( &m_eTemp );

		PlaySound( IDW_SCORE, true );
		m_pBoard->UpdatePlayer( nWinner, 0, false, true );
	}
	else {	// 점수 카드가 아닌 경우

		m_pBoard->FlyCard( (int)c, 2,
			0, -1,
			( GetState()->nBeginer + GetState()->lCurrent.GetCount() )
					% GetState()->nPlayers,
			3, nWinner, -1,
			&m_eTemp );

		m_pBoard->UpdatePlayer( -1 );

		m_pMFSM->WaitEvent( &m_eTemp );
	}

	e->SetEvent();
}

// 한 턴이 끝났음을 알린다
// 아직 state.plCurrent 등은 그대로 남아있다
void CPlayerHuman::OnTurnEnd( CEvent* e )
{
	Sleep( Mo()->nDelayOneTurn * 200 );
	e->SetEvent();
}

// 프랜드가 밝혀 졌음
void CPlayerHuman::OnFriendIsRevealed( int nPlayer, CEvent* e )
{
	// 프랜드가 밝혀졌는지를 감시함

	// 이름 색상을 갱신하고 도움말을 표시
	m_pBoard->UpdatePlayer( -2 );
	if ( nPlayer >= 0 ) {

		(new DShortMessage(m_pBoard))->Create(
			0, _T("프랜드"), true, false, DELAY_FRIENDCARD, nPlayer );
	}
	else {

		(new DShortMessage(m_pBoard))->Create(
			0, _T("주공이 초구가 되어서 노프랜드로 진행합니다"), true, false, DELAY_FRIENDCARD,
			GetState()->nMaster );
	}
	Ib()->SetText(_T("프랜드가 밝혀 졌습니다"));

	e->SetEvent();
}

// 게임 종료 ( *pbCont 가 false 이면 세션 종료 )
void CPlayerHuman::OnEnd( bool* pbCont, CEvent* e )
{
	Sb()->Update(GetState());

	// 리포트 대화상자를 표시한다
	(new DReport(m_pBoard))->Create( m_pMFSM, pbCont, e );
}

// nPlayer 의 작전 시작 ( 생각이 오래걸리는 작업 시작 )
// nMode : 0 공약 생각중  1 특권모드 처리중  2 낼 카드 생각중
void CPlayerHuman::OnBeginThink( int nPlayer, int nMode, CEvent* e )
{
	LPCTSTR asMsg[] = { _T(" 님의 공약 결정 중"),
		_T(" 님이 당을 재정비하고 있습니다"), _T(" 님 차례입니다") };
	m_pBoard->SetTurnRect( nPlayer );
	Ib()->SetText( m_pMFSM->GetState()->apPlayers[nPlayer]->GetName()
					+ asMsg[nMode] );
	e->SetEvent();
}

// 프로그램 종료
// 이 함수는 하나의 플레이어가 disconnect 할 때,
// 또는 OnEnd 에서 한명의 플레이어라도 false 를
// 리턴했을 때 호출된다 - Human 의 경우 세션을 종료한다
// sReason : 종료되는 이유 (0 이면 즉시 종료)
void CPlayerHuman::OnTerminate( LPCTSTR sReason )
{
	if ( sReason && *sReason ) {
		// 종료 이유를 표시하는 대화 상자를 출력
		DMessageBox* pBox = new DMessageBox(m_pBoard);
		pBox->SetAction( &m_eTemp );
		pBox->SetModal();
		pBox->Create( true, 1, &sReason );
		if ( Mo()->bUseSound ) ::MessageBeep( MB_ICONEXCLAMATION );

		m_pMFSM->WaitEvent( &m_eTemp );
	}

	m_pBoard->SetMFSM(0);
}

// 채팅 메시지 (bSource : 채팅창이 소스)
void CPlayerHuman::OnChat( int nPlayerID, LPCTSTR sMsg, bool bSource )
{
	bSource;	// unused
	m_pBoard->FloatSayDSB( nPlayerID, sMsg );
}
