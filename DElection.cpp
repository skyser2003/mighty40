// DElection.cpp: implementation of the DElection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "BoardWrap.h"
#include "BmpMan.h"
#include "DSB.h"
#include "DEtc.h"
#include "DElection.h"

#include "Play.h"
#include "Player.h"
#include "MFSM.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// 선거 공약 DSB

DGoal::DGoal( CBoardWrap* pBoard ) : DSB(pBoard)
{
	m_bTerminateToggler = false;
	m_bTogglerTerminated = true;

	m_colConfirm = s_colGray;
	m_tdConfirm = 0;
	m_tdConfirmSel = 0;
}

DGoal::~DGoal()
{
	if ( !m_bTogglerTerminated ) {

		m_bTerminateToggler = true;
		while ( !m_bTogglerTerminated ) Sleep(100);
		// 주의 - 경쟁조건
		// Toggler 가 Board 를 선택 모드로 두었으므로 여기서
		// 이를 리셋한다 !! 왜 이것을 Toggler 가 안하는가 ?
		// Toggler 가 리셋하는 경우 Board 로의 SendMessage 가
		// 리턴되기를 기다리나, 위의 Sleep 은
		// 그 Toggler 가 기다리는 Board의 쓰레드가 수행한다
		// 즉, Dead-Lock 발생, 따라서 여기서 이를 호출한다
		m_pBoard->CancelSelection();
	}
}

// 주어진 모양으로 기루다를 바꾼다
void DGoal::ChangeKiruda( int nShape )
{
	m_nKiruda = nShape;

	for ( int i = 0; i < 5; i++ ) {
		m_aatdShape[i][0] = i == m_nKiruda ? s_tdOutline : s_tdNormal;
		m_aatdShape[i][1] = i == m_nKiruda ? s_tdOutline : TD_SUNKEN;
	}

	// 마이티 용어 사용시 표시될 스트링을 재작성 해야한다
	if ( Mo()->bUseTerm )
		m_sMinScore = CCard::GetGoalString(
			Mo()->bUseTerm, m_nKiruda, m_nMinScore,
			m_pState->pRule->nMinScore );

	CRect rc; GetRect( &rc );
	m_pBoard->UpdateDSB( &rc );
}

// 주어진 값으로 최소 점수를 바꾼다
// (valid 한 값으로 재조정됨)
void DGoal::ChangeMinScore( int nMinScore )
{
	int nDiff = -1;

	while ( !m_pState->IsValidNewGoal( m_nKiruda, nMinScore ) ) {

		// 계속 감소시키다가 1 에 도달해도 valid 하지 않으면
		// 반대로 증가시켜 본다
		if ( nMinScore == 1 ) nDiff = 1;
		nMinScore += nDiff;
	}

	m_nMinScore = nMinScore;

	// 화살표 색상을 변경한다
	for ( int i = 0; i < 2; i++ ) {
		int nExpectedDiff = i == 0 ? 1 : -1;
		if ( m_pState->IsValidNewGoal(
				m_nKiruda, nMinScore+nExpectedDiff ) ) {
			// 이 방향으로 변경 가능함
			m_aacolArrow[i][0] = s_colCyan;
			m_aacolArrow[i][1] = s_colCyan;
			m_aatdArrow[i][0] = s_tdMidiumShade;
			m_aatdArrow[i][1] = s_tdMidiumOutline;
		}
		else { // 불가능함 - disable 한다
			m_aacolArrow[i][0] = s_colGray;
			m_aacolArrow[i][1] = s_colGray;
			m_aatdArrow[i][0] = s_tdMidium;
			m_aatdArrow[i][1] = s_tdMidium;
		}
	}

	// 표시될 스트링을 작성한다
	if ( Mo()->bUseTerm )
		m_sMinScore = CCard::GetGoalString(
			Mo()->bUseTerm, m_nKiruda, nMinScore,
			m_pState->pRule->nMinScore );
	else
		m_sMinScore.Format( _T("%d"), nMinScore );

	// 갱신
	CRect rc; GetRect( &rc );
	m_pBoard->UpdateDSB( &rc );
}


// bPrivilege : 당선자가 골을 수정하는 중인가?
// pMFSM      : 사람 플레이어의 손에 든 카드와
//              목표 점수 validity 를 테스트 하기 위해 전달한다
void DGoal::Create( bool bPrivilege,
	const CState* pState, CEvent* pEvent, CGoal* pResult )
{
	m_bPrivilege = bPrivilege;

	m_pState = pState;
	m_plHand = m_pState->apPlayers[0]->GetHand();

	SetFixed();

	ASSERT(pState);
	ASSERT(m_plHand);

	m_bDealMiss = !m_bPrivilege && m_pState->IsDealMiss(
		m_pState->apPlayers[0]->GetHand() );

	m_nKiruda = pResult->nKiruda;
	m_nMinScore = pResult->nMinScore;

	if ( !m_bPrivilege ) {

		CRect rc = m_pBoard->CalcPlayerExtent( 0 );
		int cxDSB = 15, cyDSB = 8;

		DSB::Create2(
			rc.left+rc.Width()/2 - cxDSB*GetDSBUnit()/2,
			rc.bottom - cyDSB*GetDSBUnit(),
			cxDSB, cyDSB, -1 );
	}
	else {

		int cxDSB = 18, cyDSB = 8+9;

		DSB::Create( 0, -cyDSB/6, cxDSB, cyDSB, -1 );
	}

	SetAction( pEvent, pResult );

	if ( m_bPrivilege ) {
		// Toggler 쓰레드를 시작시킨다
		m_bTogglerTerminated = false;
		VERIFY( AfxBeginThread( TogglerProc, (LPVOID)this ) );
	}
}

// 초기화 ( 생성된 후 호출됨 )
void DGoal::OnInit()
{
	DSB::OnInit();

	// Hotspot 의 기준이 되는 y 좌표
	int y = 0;

	// 선거 중이라면 추천 카드와 최소 가능 점수로 시작한다
	if ( !this->m_bPrivilege ) {

		ChangeKiruda( m_pState->apPlayers[0]
						->GetHand()->GetKirudable() );
		ChangeMinScore( 1 );	// 자동으로 계산됨
	}
	else {	// 그렇지 않으면 원래 주어진 공약으로 시작한다

		ChangeKiruda( m_nKiruda );
		ChangeMinScore( m_nMinScore );

		y = 8;
	}

	// 5 개의 shape 영역

	CRect rcSpade = RegisterHotspot(
		1, y+2, -1, -1, true,
		IDB_SPADE, 0,
		&s_colWhite, &m_aatdShape[1][0], &s_colWhite, &m_aatdShape[1][1],
		(LPVOID)2 );

	RegisterHotspot(
		rcSpade.right+1, rcSpade.top, -1, -1, false,
		IDB_DIAMOND, 0,
		&s_colWhite, &m_aatdShape[2][0], &s_colWhite, &m_aatdShape[2][1],
		(LPVOID)3 );

	RegisterHotspot(
		rcSpade.left, rcSpade.bottom+1, -1, -1, false,
		IDB_HEART, 0,
		&s_colWhite, &m_aatdShape[3][0], &s_colWhite, &m_aatdShape[3][1],
		(LPVOID)4 );

	CRect rcClover = RegisterHotspot(
		rcSpade.right+1, rcSpade.bottom+1, -1, -1, false,
		IDB_CLOVER, 0,
		&s_colWhite, &m_aatdShape[4][0], &s_colWhite, &m_aatdShape[4][1],
		(LPVOID)5 );

	CRect rcNokiruda = RegisterHotspot(
		rcClover.right+1, rcSpade.bottom+1, -1, -1, false,
		IDB_NOTHING, 0,
		&s_colWhite, &m_aatdShape[0][0], &s_colWhite, &m_aatdShape[0][1],
		(LPVOID)1 );

	// < > 버튼

	CRect rc1 = RegisterHotspot(
		12, y+2, -1, -1, true, 0, _T("△"),
		&m_aacolArrow[0][0], &m_aatdArrow[0][0],
		&m_aacolArrow[0][1], &m_aatdArrow[0][1],
		(LPVOID)6 );

	CRect rc2 = RegisterHotspot(
		12, y+4, -1, -1, true, 0, _T("▽"),
		&m_aacolArrow[1][0], &m_aatdArrow[1][0],
		&m_aacolArrow[1][1], &m_aatdArrow[1][1],
		(LPVOID)7 );

	// 출마 / 출마포기 버튼

	if ( !m_bDealMiss ) {

		if ( !m_bPrivilege ) {

			RegisterHotspot(
				2, y+6, -1, -1, true, 0, _T("포기"),
				&s_colWhite, &s_tdShade, &s_colCyan, &s_tdShadeOutline,
				(LPVOID)0 );

			RegisterHotspot(
				9, y+6, -1, -1, true, 0, _T("출마 !!"),
				&s_colWhite, &s_tdShade, &s_colCyan, &s_tdShadeOutline,
				(LPVOID)8 );
		}
		else RegisterHotspot(
				8, y+7, -1, -1, true, 0, _T("확정"),
				&m_colConfirm, &m_tdConfirm,
				&m_colConfirm, &m_tdConfirmSel,
				(LPVOID)9 );
	}
	else {

		RegisterHotspot(
			8, y+6, -1, -1, true, 0, _T("딜미스 !!"),
			&s_colYellow, &s_tdShade, &s_colYellow, &s_tdShadeOutline,
			(LPVOID)0 );

		RegisterHotspot(
			2, y+6, -1, -1, true, 0, _T("출마 !!"),
			&s_colWhite, &s_tdShade, &s_colCyan, &s_tdShadeOutline,
			(LPVOID)8 );
	}

	m_rcMinScore.top = rc1.top;
	m_rcMinScore.bottom = rc2.bottom;
	m_rcMinScore.left = rcNokiruda.right;
	m_rcMinScore.right = rc2.left;

	CRect rcBox; GetRect( &rcBox );
	m_rcMinScore.OffsetRect( - rcBox.TopLeft() );
}

// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
// 오버라이드 하여 구현해야 한다
// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
void DGoal::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	static LPCTSTR asShape[] = {
		_T("노기루다"), _T("스페이드"), _T("다이아몬드"),
		_T("하트"), _T("클로버") };

	DSB::OnDraw( pDC, bBrushOrigin );

	if ( !m_bPrivilege ) {
		PutText( pDC, _T("공약을 정해 주세요"), 0, 0, true,
			s_colWhite, s_tdShade );
	}
	else {
		static LPCTSTR asText[] = {
			_T("당선을 축하드립니다 !!"),
			_T(""),
			_T("버리고 싶은 카드 세 장을"),
			_T("선택한 다음, 아래쪽에 있는"),
			_T("'확정'을 클릭하세요"),
			_T(""),
			_T("지금 기루다와 목표 점수를"),
			_T("바꿀 수 있습니다")
		};
		for ( int i = 0; i < sizeof(asText)/sizeof(LPCTSTR); i++ )
			PutText( pDC, asText[i], -1, i+1, true,
				s_colWhite, s_tdShade );
	}

	CRect rcBox; GetRect( &rcBox );

	// 숫자만 나오는 경우 좀 더 큰 글씨로
	int td = !Mo()->bUseTerm ? TD_SIZEBIG|TD_SHADE : s_tdMidiumShade;
	CSize szText = GetTextExtent(
		Mo()->bUseTerm ? 1 : 2 , m_sMinScore );

	int x = m_rcMinScore.left + m_rcMinScore.Width()/2 - szText.cx/2 + rcBox.left;
	int y = m_rcMinScore.top + m_rcMinScore.Height()/2 - szText.cy/2 + rcBox.top;

	if ( m_bPrivilege ) y += 8;

	PutText( pDC, m_sMinScore, x, y, false, s_colYellow, td );

	if ( !Mo()->bUseTerm )
		PutText( pDC, asShape[m_nKiruda],
			m_rcMinScore.left + rcBox.left,
			y - GetTextExtent( 0, asShape[m_nKiruda] ).cy,
			false, s_colWhite, s_tdShade );
}

// 핫 스팟을 클릭할 때 불리는 함수
// LPVOID 값이 0 이면 종료 (X자를 클릭) 요청
void DGoal::OnClick( LPVOID pVoid )
{
	if ( pVoid == (LPVOID)0xffffffff ) {	// Enter key
		if ( m_bPrivilege ) pVoid = (LPVOID)9;
		else pVoid = (LPVOID)8;
	}

	int nSel = (int)pVoid;

	switch ( nSel ) {
	case 1: case 2:
	case 3: case 4: case 5: {
		DSB::OnClickSound();
		if ( !m_bPrivilege ) ChangeKiruda( nSel-1 );
		else {
			int nKiruda = nSel-1;
			int nMinScore = m_pState->goal.nMinScore;
			while ( !m_pState->IsValidNewGoal( nKiruda, nMinScore )
				&& nMinScore <= HIGHSCORE_MAXLIMIT ) nMinScore++;
			if ( nMinScore > HIGHSCORE_MAXLIMIT ) return;
			else {
				ChangeKiruda( nKiruda );
				if ( m_nMinScore != nMinScore )
					ChangeMinScore( nMinScore );
			}
		}
	} break;
	case 6: {
		DSB::OnClickSound();
		ChangeMinScore( m_nMinScore+1 );
	} break;
	case 7: {
		DSB::OnClickSound();
		ChangeMinScore( m_nMinScore-1 );
	} break;
	case 0: {

		if ( m_bPrivilege ) {

			if ( Mo()->bUseSound ) MessageBeep( MB_ICONEXCLAMATION );
			(new DShortMessage(m_pBoard))->Create(
				0, _T("'확정'을 누르세요"),
				true, false, 2000 );
		}
		else if ( !m_bDealMiss
			&& !m_pState->IsValidNewGoal( m_nKiruda, 0 ) ) {

			if ( Mo()->bUseSound ) MessageBeep( MB_ICONEXCLAMATION );
			(new DShortMessage(m_pBoard))->Create(
				0, _T("선은 첫 턴에 반드시 출마하여야 합니다"),
				true, false, 2000 );
		}
		else {

			CGoal* pGoal = (CGoal*)m_pResult;

			pGoal->nFriend = 0;
			pGoal->nKiruda = m_nKiruda;
			pGoal->nMinScore = m_bDealMiss ? -1 : 0;

			Destroy();
		}
	} break;
	case 8: {

		if ( !m_pState->IsValidNewGoal( m_nKiruda, m_nMinScore ) ) {

			if ( Mo()->bUseSound ) MessageBeep( MB_ICONEXCLAMATION );
			(new DShortMessage(m_pBoard))->Create(
				0, _T("이 공약으로는 출마할 수 없습니다"),
				true, false, 2000 );
		}
		else {

			CGoal* pGoal = (CGoal*)m_pResult;

			pGoal->nFriend = 0;
			pGoal->nKiruda = m_nKiruda;
			pGoal->nMinScore = m_nMinScore;

			Destroy();
		}
	} break;
	case 9: {	// 확정
		ASSERT(m_bPrivilege);

		if ( !m_pState->IsValidNewGoal( m_nKiruda, m_nMinScore ) ) {

			if ( Mo()->bUseSound ) MessageBeep( MB_ICONEXCLAMATION );
			(new DShortMessage(m_pBoard))->Create(
				0, _T("목표점수를 더 올려야 합니다"),
				true, false, 2000 );
		}
		else if ( m_pBoard->GetSelectionCount() != 3 ) {

			if ( Mo()->bUseSound ) MessageBeep( MB_ICONEXCLAMATION );
			(new DShortMessage(m_pBoard))->Create(
				0, _T("먼저, 버릴 카드 세 장을 선택하세요"),
				true, false, 2000 );
		}
		else {
			CGoal* pGoal = (CGoal*)m_pResult;

			pGoal->nFriend = 0;
			pGoal->nKiruda = m_nKiruda;
			pGoal->nMinScore = m_nMinScore;

			Destroy();
		}

	} break;
	}
}

// Board 의 Selection 을 관리할 쓰레드
// 카드 선택을 토글하는 역할만을 한다
UINT DGoal::TogglerProc( LPVOID _pThis )
{
	TRACE("DGoal::TogglerProc began\n");

	DGoal* pThis = (DGoal*)_pThis;

	long nCard = 0;

	pThis->m_pBoard->WaitSelection( &pThis->m_ev, &nCard );

	while ( !pThis->m_bTerminateToggler ) {

		// 0.1 초에 한 번씩 종료 조건을 확인한다
		while ( !pThis->m_bTerminateToggler && !pThis->m_ev.Lock( 100 ) );
		if ( pThis->m_bTerminateToggler ) break;

		// 소리내기
		PlaySound( IDW_GETCARD, true );

		// 선택된 카드를 토글한다
		int nIndex = pThis->m_plHand->IndexFromPOSITION(
			pThis->m_plHand->Find( CCard(nCard) ) );
		pThis->m_pBoard->SetSelection( nIndex,
			!pThis->m_pBoard->GetSelection( nIndex ) );

		// 3 장이 선택되었다면 "확정" 색상을 바꾼다
		ASSERT( pThis->m_bPrivilege );
		COLORREF colOld = pThis->m_colConfirm;

		if ( pThis->m_pBoard->GetSelectionCount() == 3 ) {
			pThis->m_colConfirm = s_colCyan;	// 시안 색으로
			pThis->m_tdConfirm = s_tdShade;
			pThis->m_tdConfirmSel = s_tdShadeOutline;
		}
		else {
			pThis->m_colConfirm = s_colGray;
			pThis->m_tdConfirm = 0;
			pThis->m_tdConfirmSel = 0;
		}

		// 색상이 변경되었다면 Update
		if ( colOld != pThis->m_colConfirm ) {
			pThis->UpdateHotspot(pThis->m_lHotspot.GetTailPosition());
		}

		// 다시 화면 갱신
		pThis->m_pBoard->UpdatePlayer( 0, 0, true, true );
	}
	pThis->m_bTogglerTerminated = true;
	TRACE("DGoal::TogglerProc ended\n");
	return 0;
}



/////////////////////////////////////////////////////////////////////////////
// 카드 죽이기 DSB

// m_nShape 를 특정 값으로 세트하면서 m_aatdShape 를
// 재 조정한다
void DKill::SetCurShape( int nShape )
{
	m_nShape = nShape;
	for ( int i = 0; i < 4; i++ ) {
		m_aatdShape[i][0] = SPADE+i == nShape ? TD_OUTLINE : TD_NORMAL;
		m_aatdShape[i][1] = SPADE+i == nShape ? TD_OUTLINE : TD_SUNKEN;
	}
	CRect rc; GetRect( &rc );
	m_pBoard->UpdateDSB( &rc );
}

// m_pcToKill 값을 변경하고 그 결과를 화면에 업데이트 한다
void DKill::SetCurCard( CCard c )
{
	*m_pcToKill = c;
	CRect rc; CalcCurCardRect( &rc );
	m_pBoard->UpdateDSB( &rc );
}

// pcToKill : 이 DSB 가 결과 카드를 저장할 장소 (결과-값 독립변수)
// plDead   : 지금까지 헛다리 짚은 카드들
void DKill::Create( CEvent* pEvent, CCard* pcToKill, const CCardList* plDead, const CCardList* plHand )
{
	m_pcToKill = pcToKill;
	m_plDead = plDead;
	m_plHand = plHand;
	m_nShape = m_pcToKill && m_pcToKill->GetShape() > 0
				? m_pcToKill->GetShape() : SPADE;

	SetFixed();

	int nUnit = GetDSBUnit();

	// 최소의 크기는 ( 16 * 14 ) * ( card.cy + 32 + nUnit ) 의 카드 선택열이 들어가야 함
	int cxDSB = ( 16*14 + nUnit-1 ) / nUnit;
	int cyDSB = ( GetBitmapSize(IDB_SA).cy + 32 + nUnit + nUnit-1 ) / nUnit;

	// 다른 내용 들어갈 크기
	cxDSB += 5;
	cyDSB += 1;

	CRect rc = m_pBoard->CalcPlayerExtent( 0 );

	DSB::Create2(
		rc.left+rc.Width()/2 - cxDSB*GetDSBUnit()/2,
		rc.bottom - cyDSB*GetDSBUnit(),
		cxDSB, cyDSB, -1 );

	SetAction( pEvent, pcToKill );
}

// 초기화 ( 생성된 후 호출됨 )
void DKill::OnInit()
{
	DSB::OnInit();

	SetCurShape( m_nShape );

	int nDSBUnit = GetDSBUnit();
	CRect rc; GetRect( &rc );

	int x = rc.right - 16*14 - nDSBUnit;
	int xDiff = 16;
	int cx = 16;
	int y = rc.bottom - 32 - nDSBUnit - 3;
	int cy = 32 + 3;
	for ( int i = 0; i < 14; i++, x += xDiff )

		RegisterHotspot(
			x, y, cx, cy, false,
			0, 0, &s_colWhite, &s_tdNormal, &s_colWhite, &s_tdNormal,
			(LPVOID)(i+100) );

	CRect rcCurCard; CalcCurCardRect( &rcCurCard );
	RegisterHotspot(
		rcCurCard.left, rcCurCard.top, rcCurCard.Width(), rcCurCard.Height(),
		false, 0, 0, &s_colWhite, &s_tdNormal, &s_colWhite, &s_tdNormal,
			(LPVOID)0 );

	// 4 개의 Shape 선택 영역

	CRect rcSpade = RegisterHotspot(
		rc.right - 16*14 - nDSBUnit - GetBitmapSize(IDB_SPADE).cx*2 - 8,
		rc.bottom - GetBitmapSize(IDB_SPADE).cy*2 - nDSBUnit, -1, -1, false,
		IDB_SPADE, 0,
		&s_colWhite, &m_aatdShape[0][0], &s_colWhite, &m_aatdShape[0][1],
		(LPVOID)1000 );

	RegisterHotspot(
		rcSpade.right, rcSpade.top, -1, -1, false,
		IDB_DIAMOND, 0,
		&s_colWhite, &m_aatdShape[1][0], &s_colWhite, &m_aatdShape[1][1],
		(LPVOID)1001 );

	RegisterHotspot(
		rcSpade.left, rcSpade.bottom, -1, -1, false,
		IDB_HEART, 0,
		&s_colWhite, &m_aatdShape[2][0], &s_colWhite, &m_aatdShape[2][1],
		(LPVOID)1002 );

	CRect rcClover = RegisterHotspot(
		rcSpade.right, rcSpade.bottom, -1, -1, false,
		IDB_CLOVER, 0,
		&s_colWhite, &m_aatdShape[3][0], &s_colWhite, &m_aatdShape[3][1],
		(LPVOID)1003 );
}

// 현재 카드를 그릴 영역을 리턴한다
void DKill::CalcCurCardRect( LPRECT pRc )
{
	CRect rc; GetRect( &rc );
	int nDSBUnit = GetDSBUnit();
	CSize szCard = GetBitmapSize(IDB_SA);
	pRc->left = rc.right - nDSBUnit - szCard.cx;
	pRc->top = rc.bottom - 32 - nDSBUnit - 5 - szCard.cy;
	pRc->right = pRc->left + szCard.cx;
	pRc->bottom = pRc->top + szCard.cy;
}

// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
// 오버라이드 하여 구현해야 한다
// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
void DKill::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	// 현재 선택된 카드를 그린다
	CRect rcCurCard; CalcCurCardRect( &rcCurCard );
	m_pBoard->GetBmpMan()->DrawCard( pDC, *m_pcToKill, rcCurCard );

	// 이 카드가 손에 있는 카드거나 dead 카드이면 x 표를 한다
	if ( m_plDead->Find( *m_pcToKill ) || m_plHand->Find( *m_pcToKill ) ) {
		CRect rc(
			rcCurCard.left + rcCurCard.Width()/2 - rcCurCard.Width()/3,
			rcCurCard.top + rcCurCard.Height()/2 - rcCurCard.Width()/3,
			rcCurCard.left + rcCurCard.Width()/2 + rcCurCard.Width()/3,
			rcCurCard.top + rcCurCard.Height()/2 + rcCurCard.Width()/3 );

		CPen pen( PS_SOLID, 10, RGB(255,0,0) );
		CPen* pnOld = pDC->SelectObject( &pen );
		pDC->MoveTo( rc.TopLeft() ); pDC->LineTo( rc.BottomRight() );
		pDC->MoveTo( rc.right, rc.top ); pDC->LineTo( rc.left, rc.bottom );
		pDC->SelectObject( pnOld );
	}

	// 13 장의 카드를 그린다
	int nDSBUnit = GetDSBUnit();
	CRect rc; GetRect( &rc );
	int x = rc.right - 16*14 - nDSBUnit;
	int xDiff = 16;
	int cx = 16;
	int y = rc.bottom - 32 - nDSBUnit;
	int cy = 32;
	CCard c;
	for ( int i = 0; i < 14; i++, x += xDiff ) {

		if ( i == 0 ) c = CCard::GetJoker();	// 마지막 장은 조커
		else if ( i == 1 ) c = CCard( m_nShape, ACE );	// 첫장은 에이스
		else c = CCard( m_nShape, KING - (i-2) );

		bool bSel =		// 현재 선택된 Hotspot 인가
			m_posSel && m_lHotspot.GetAt(m_posSel).pVoid == (LPVOID)(100+i)
			&& !m_plDead->Find(c)
			&& !m_plHand->Find(c);
		int yDiff = bSel ? 3 : 0;

		m_pBoard->GetBmpMan()->DrawCardEx(
			pDC, (int)c, x, y - yDiff, cx, cy + yDiff,
			0, 0, cx, cy+yDiff );
	}

	static LPCTSTR asText[] = {
		_T("한 후보를 탈락 시킵니다"),
		_T(""),
		_T("원하는 표적 카드를"),
		_T("아래에서 선택하고"),
		_T("오른쪽의 큰 카드를"),
		_T("클릭하세요")
	};
	for ( int m = 0; m < sizeof(asText)/sizeof(LPCTSTR); m++ )
		PutText( pDC, asText[m], 1, 1+m, true, s_colWhite, s_tdShade );
}

// 핫 스팟을 클릭할 때 불리는 함수
// LPVOID 값이 0 이면 종료 (X자를 클릭) 요청
void DKill::OnClick( LPVOID pVoid )
{
	int n = (int)pVoid;

	if ( n >= 1000 && n <= 1003 ) {
		// 모양 선택
		DSB::OnClickSound();
		SetCurShape( n-1000+SPADE );
		return;
	}
	else if ( n >= 100 && n <= 113 ) {
		// 카드 선택
		DSB::OnClickSound();
		CCard c;
		if ( n == 100 )			// Joker
			c = CCard(JOKER);
		else if ( n == 101 )	// ACE
			c = CCard(m_nShape,ACE);
		else
			c = CCard( m_nShape, 115-n );
		SetCurCard( c );
	}
	else if ( n == 0 || n == 0xffffffff ) {	// 종료

		// 불법 카드를 선택 못하게 하려면 주석을 삭제할것
		//		if ( m_plDead->Find(*m_pcToKill)
		//			|| m_plHand->Find(*m_pcToKill) ) return;
		DSB::OnClick(0);
		return;
	}

	RECT rc;
	GetRect( &rc );
	m_pBoard->UpdateDSB( &rc, true );
}



/////////////////////////////////////////////////////////////////////////////
// 당선자 발표 DSB

// bTemp       : true 이면 이 DSB 는 최초의 당선자 발표 DSB
//               false 이면 이 DSB 는 최종 확정 DSB
// bToKill     : 다른 플레이어를 죽여야 하는가
// bUseFriend  : 프랜드 제도가 있는가
// sMasterName : 당선자 이름
// goal        : 공약
void DMasterDecl::Create( bool bTemp, bool bHuman, bool bToKill,
	bool bUseFriend, int nMinScore, const CGoal& goal,
	LPCTSTR sMasterName, LPCTSTR sFriendName,
	int nTimeOut )
{
	SetFixed();

	m_bTemp = bTemp;
	m_bHuman = bHuman;
	m_bToKill = bToKill;
	m_bUseFriend = bUseFriend;
	m_nDefaultMinScore = nMinScore;
	m_sMasterName = sMasterName;
	m_sFriendName = sFriendName ? sFriendName : _T("");
	m_goal = goal;

	// 당선 DSB 는 조금 위쪽에 나타나도록 한다
	// (화면 절반의 위쪽으로)

	CRect rcClient; m_pBoard->GetClientRect( &rcClient );

	int nUnit = GetDSBUnit();
	int cxDSB = 18;
	int cyDSB = 8;
	int cyDSB2 = cyDSB + max( ( GetBitmapSize(IDB_SA).cy + nUnit-1 )
				/ nUnit + 2, 8 );

	int y = ( rcClient.top + rcClient.bottom ) / 2
				- ( cyDSB + 1 ) * GetDSBUnit();

	if ( !( m_bHuman && m_bTemp || !m_bTemp && !m_bUseFriend ) )
		cyDSB  = cyDSB2;

	DSB::Create2(
		rcClient.left + rcClient.Width()/2 - cxDSB*GetDSBUnit()/2,
		y, cxDSB, cyDSB, nTimeOut );
}

// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
// 오버라이드 하여 구현해야 한다
// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
void DMasterDecl::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	UINT arcShape[] = { IDB_NOTHING, IDB_SPADE, IDB_DIAMOND, IDB_HEART, IDB_CLOVER };
	DSB::OnDraw( pDC, bBrushOrigin );

	// 머리말
	LPCTSTR sTitle;
	if ( m_bTemp ) {	// 임시 당선
		if ( m_bHuman ) sTitle = _T("당선을 축하드립니다 !!");
		else sTitle = _T("당선자가 확정되었습니다 !!");
	} else sTitle = _T("-- 선거 결과 발표 --");

	// 공약 스트링
	CString sGoal(_T(" "));
	sGoal += CCard::GetGoalString( Mo()->bUseTerm,
		m_goal.nKiruda, m_goal.nMinScore, m_nDefaultMinScore );

	PutText( pDC, sTitle, -1, 1, true, s_colWhite, s_tdShade );
	PutText( pDC, m_sMasterName, -1, 3, true, s_colYellow, s_tdShadeOutline );
	PutBitmap( pDC, arcShape[m_goal.nKiruda], sGoal, -1, 5, true, s_colWhite, s_tdShade );

	// 프랜드 영역을 채운다

	// 자신이 당선자이고 임시 당선자를 발표하는 경우
	// 프랜드 영역이 존재하지 않는다
	if ( m_bHuman && m_bTemp
		|| !m_bTemp && !m_bUseFriend ) return;

	// 프랜드를 정해야 하는 경우
	// 프랜드 영역에 그 내용을 표시한다
	if ( !m_bTemp && m_bUseFriend ) {

		// 프랜드를 카드로 정하는 경우
		if ( m_goal.nFriend > 0 && m_goal.nFriend < 54 ) {

			CRect rc; GetRect( &rc );
			CSize szCard = GetBitmapSize(IDB_SA);
			CString sText;
			sText.Format( _T("%s 프랜드"),
				CCard(m_goal.nFriend).GetString( Mo()->bUseTerm ) );

			CRect rcText = PutText( pDC, sText,
				-1, 8, true, s_colCyan, s_tdShade );
			m_pBoard->GetBmpMan()->DrawCard( pDC, m_goal.nFriend,
				rc.left + (rc.Width()-15)/2 - szCard.cx/2,
				rcText.bottom + 2,
				szCard.cx, szCard.cy );
		}
		else {	// 사람 그림자가 나오는 경우

			CRect rc; GetRect( &rc );
			CSize szCard = GetBitmapSize(IDB_SA);
			CString sText;
			if ( m_goal.nFriend < 0 )	// 사람
				sText = m_sFriendName;
			else if ( m_goal.nFriend == 0 )	// 노
				sText = _T("노");
			else if ( m_goal.nFriend == 100 )	// 초구
				sText = _T("초구");

			CRect rcText = PutText( pDC, sText + _T(" 프랜드"),
				-1, 8, true, s_colCyan, s_tdShade );

			CRect rcCurCard( CPoint(
				rc.left + (rc.Width()-15)/2 - szCard.cx/2,
				rcText.bottom + 2 ), szCard );

			DrawBitmap( pDC, IDB_PERSON,
				rcCurCard.left, rcCurCard.top,
				szCard.cx, szCard.cy,
				0, 0, szCard.cx, szCard.cy );

			if ( m_goal.nFriend == 0 ) {	// 노프랜드(X)
				CRect rc(
					rcCurCard.left + rcCurCard.Width()/2 - rcCurCard.Width()/3,
					rcCurCard.top + rcCurCard.Height()/2 - rcCurCard.Width()/3,
					rcCurCard.left + rcCurCard.Width()/2 + rcCurCard.Width()/3,
					rcCurCard.top + rcCurCard.Height()/2 + rcCurCard.Width()/3 );

				CPen pen( PS_SOLID, 10, RGB(255,0,0) );
				CPen* pnOld = pDC->SelectObject( &pen );
				pDC->MoveTo( rc.TopLeft() ); pDC->LineTo( rc.BottomRight() );
				pDC->MoveTo( rc.right, rc.top ); pDC->LineTo( rc.left, rc.bottom );
				pDC->SelectObject( pnOld );
			}
			else {

				// 텍스트 위치 (s,x,y)
				int x = rcCurCard.left + rcCurCard.Width()/2 - GetTextExtent( 0, sText ).cx/2;
				int y = rcCurCard.bottom - GetBitmapSize(IDB_SA).cy / 3;

				PutText( pDC, sText, x, y, false, s_colWhite, s_tdShadeOutline );
			}
		}
		return;
	}

	// 뭔가 말로라도 프랜드 영역을 채워줘야 한다
	LPCTSTR aMsg[6];
	for ( int i = 0; i < sizeof(aMsg)/sizeof(LPCTSTR); i++ )
		aMsg[i] = _T("");

	// 개혁 시작
	if ( m_bToKill ) {
		aMsg[0] = _T("당선자는 정권 초기의");
		aMsg[1] = _T("보복적인 개혁을 주도합니다 !");
		aMsg[3] = _T("비리가 밝혀진 후보의 비자금은");
		aMsg[4] = _T("전액 국고로 환수됩니다");
	}
	// 프랜드 정하기
	else {
		aMsg[0] = _T("당선자는 나머지 카드를 받고");
		if ( m_bUseFriend )
			aMsg[1] = _T("프랜드와 공동 정권을 구성합니다");
		else aMsg[1] = _T("당을 재정비 합니다");
		aMsg[3] = _T("한편 야당 총재들은");
		aMsg[4] = _T("\"독재 타도\"라는 공통 목표 하에");
		aMsg[5] = _T("하나로 야합했습니다");
	}

	for ( int j = 0; j < sizeof(aMsg)/sizeof(LPCTSTR); j++ )
		PutText( pDC, aMsg[j], -1, 8 + j,
			true, s_colWhite, s_tdShade );
}

// 아무데나 클릭해도 바로 사라지도록 한다
bool DMasterDecl::DispatchHotspot( int x, int y, bool bCheckOnly )
{
	if ( m_bTemp ) return DSB::DispatchHotspot( x, y, bCheckOnly );
	else if ( bCheckOnly ) return false;
	else { Destroy(); return true; }
}


/////////////////////////////////////////////////////////////////////////////
// 탈락자 발표 DSB

// sMaster  : 당선자 이름
// nCard    : 화면에 표시될 카드
// bFail    : 이 카드는 탈락 실패 카드인가 (메시지가 조금 달라짐)
void DDeadDecl::Create( LPCTSTR sMaster, int nCard,
	bool bFail, int nTimeOut )
{
	SetFixed();

	m_sMaster = sMaster;
	m_nCard = nCard;
	m_bFail = bFail;

	int nUnit = GetDSBUnit();

	int cxDSB = 17;
	int cyDSB = 7;
	cyDSB += ( GetBitmapSize(IDB_SA).cy + nUnit-1 ) / nUnit;

	DSB::Create( 0, -2, cxDSB, cyDSB, nTimeOut );
}

// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
// 오버라이드 하여 구현해야 한다
// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
void DDeadDecl::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	// 주공 이름
	PutText( pDC, m_sMaster,
		0, 0, true, s_colYellow, s_tdShadeOutline );

	// 설명
	LPCTSTR asMsg[3];
	if ( m_bFail ) {
		asMsg[0] = _T("그러나 해당 카드를 가진");
		asMsg[1] = _T("플레이어가 없습니다");
		asMsg[2] = _T("다시 카드를 선택합니다");
	} else {
		asMsg[0] = _T("");
		asMsg[1] = _T("다음의 카드를 가진");
		asMsg[2] = _T("플레이어를 탈락시킵니다 !!");
	}
	for ( int i = 0; i < 3; i++ )
		PutText( pDC, asMsg[i],
		-1, 1+i, true, s_colWhite, s_tdShade );

	CRect rc; GetRect( &rc );
	CSize szCard = GetBitmapSize(IDB_SA);
	CString sText = CCard(m_nCard).GetString( Mo()->bUseTerm );

	CRect rcItem = PutText( pDC, sText,
		-1, 5, true, s_colCyan, s_tdOutline );
	m_pBoard->GetBmpMan()->DrawCard( pDC, m_nCard,
		rc.left + (rc.Width()-15)/2 - szCard.cx/2,
		rcItem.bottom + 2,
		szCard.cx, szCard.cy );

	if ( m_bFail ) {

		// X 표 그리기
		CRect rcCurCard(
			CPoint( rc.left + (rc.Width()-15)/2 - szCard.cx/2,
					rcItem.bottom + 2 ),
			CSize( szCard.cx, szCard.cy ) );
		CRect rc(
			rcCurCard.left + rcCurCard.Width()/2 - rcCurCard.Width()/3,
			rcCurCard.top + rcCurCard.Height()/2 - rcCurCard.Width()/3,
			rcCurCard.left + rcCurCard.Width()/2 + rcCurCard.Width()/3,
			rcCurCard.top + rcCurCard.Height()/2 + rcCurCard.Width()/3 );

		CPen pen( PS_SOLID, 10, RGB(255,0,0) );
		CPen* pnOld = pDC->SelectObject( &pen );
		pDC->MoveTo( rc.TopLeft() ); pDC->LineTo( rc.BottomRight() );
		pDC->MoveTo( rc.right, rc.top ); pDC->LineTo( rc.left, rc.bottom );
		pDC->SelectObject( pnOld );
	}
}



/////////////////////////////////////////////////////////////////////////////
// 프랜드 DSB

// m_nShape 를 특정 값으로 세트하면서 m_aatdShape 를
// 재 조정한다
void DFriend::SetCurShape( int nShape )
{
	m_nShape = nShape;
	for ( int i = 0; i < 4; i++ ) {
		m_aatdShape[i][0] = SPADE+i == nShape ? TD_OUTLINE : TD_NORMAL;
		m_aatdShape[i][1] = SPADE+i == nShape ? TD_OUTLINE : TD_SUNKEN;
	}
	CRect rc; GetRect( &rc );
	m_pBoard->UpdateDSB( &rc );
}

// m_pnFriend 값을 변경하고 그 결과를 화면에 업데이트 한다
void DFriend::SetCurFriend( int nFriend )
{
	*m_pnFriend = nFriend;

	// 11 개의 예약된 프랜드 카드 중 어디에 해당하는가 검사한다
	int nIndex = -1;
	if ( nFriend == 0 ) nIndex = 5;
	else if ( 0 < nFriend && nFriend < 54 ) {	// 카드
		if ( nFriend == CCard::GetMighty() ) nIndex = 0;
		else if ( nFriend == CCard::GetJoker() ) nIndex = 1;
		else if ( CCard::GetKiruda() )
			if ( nFriend == CCard(CCard::GetKiruda(),ACE) ) nIndex = 2;
			else if ( nFriend == CCard(CCard::GetKiruda(),KING) ) nIndex = 3;
	}
	else if ( nFriend == 100 ) nIndex = 4;	// 초구
	else if ( nFriend < 0 ) nIndex = 6 - ( nFriend + 1 );	// 사람

	for ( int i = 0; i < sizeof(m_acolFriend)/sizeof(COLORREF); i++ )
		if ( i == nIndex ) {
			m_acolFriend[i] = s_colCyan;
			m_atdFriend[i] = s_tdShade;
			m_atdFriendSel[i] = s_tdShadeOutline;
		}
		else {
			bool bEnable = true;

			if ( i < 4 ) {
				CCard c = i == 0 ? CCard::GetMighty()
					: i == 1 ? ( m_pState->pRule->bJokerFriend ? CCard::GetJoker() : CCard(0) )
					: !CCard::GetKiruda() ? CCard(0)
					: i == 2 ? CCard( CCard::GetKiruda(), ACE )
					: CCard( CCard::GetKiruda(), KING );
				if ( c == CCard(0) || m_lHand.Find(c) )
					bEnable = false;
			}

			if ( !bEnable ) {
				m_atdFriend[i] = s_tdNormal;
				m_atdFriendSel[i] = s_tdNormal;
				m_acolFriend[i] = s_colGray;
			}
			else {
				m_atdFriend[i] = s_tdShade;
				m_atdFriendSel[i] = s_tdShadeOutline;
				m_acolFriend[i] = s_colWhite;
			}
		}

	CRect rc; GetRect( &rc );
	m_pBoard->UpdateDSB( &rc );
}

// pcFriend : 이 DSB 가 결과 카드를 저장할 장소 (결과-값 독립변수)
void DFriend::Create( CEvent* pEvent, int* pnFriend, const CState* pState )
{
	ASSERT( m_pnFriend);

	m_pnFriend = pnFriend;
	m_lHand.RemoveAll();
	m_lHand.AddTail(pState->apPlayers[0]->GetHand());
	m_lHand.AddTail(&pState->lDeck);
	m_pState = pState;
	// 디폴트 모양은 기루다 모양
	m_nShape = CCard::GetKiruda() ? CCard::GetKiruda()
		: 0 < *m_pnFriend && *m_pnFriend < 54	// 노기루다면 프랜드 모양
		? CCard(*m_pnFriend).GetShape()
		: CCard::GetMighty().GetShape();

	SetFixed();

	int nUnit = GetDSBUnit();

	// 최소의 크기는 ( 16 * 14 ) * ( card.cy + 32 + nUnit ) 의 카드 선택열이 들어가야 함
	int cxDSB = ( 16*14 + nUnit-1 ) / nUnit;
	int cyDSB = ( GetBitmapSize(IDB_SA).cy + 32 + nUnit + nUnit-1 ) / nUnit;

	// 다른 내용 들어갈 크기
	cxDSB += 5;
	cyDSB += 13;

	DSB::Create( 0, 0, cxDSB, cyDSB, -1 );

	SetAction( pEvent, pnFriend );
}

// 초기화 ( 생성된 후 호출됨 )
void DFriend::OnInit()
{
	DSB::OnInit();
 
	SetCurShape( m_nShape );
	SetCurFriend( *m_pnFriend );

	int nDSBUnit = GetDSBUnit();
	CRect rc; GetRect( &rc );

	// "다른 카드" 영역
	int x = rc.right - 16*14 - nDSBUnit;
	int xDiff = 16;
	int cx = 16;
	int y = rc.bottom - 32 - nDSBUnit - 3;
	int cy = 32 + 3;
	for ( int i = 0; i < 14; i++, x += xDiff )

		RegisterHotspot(
			x, y, cx, cy, false,
			0, 0, &s_colWhite, &s_tdNormal, &s_colWhite, &s_tdNormal,
			(LPVOID)(i+100) );

	CRect rcCurCard; CalcCurCardRect( &rcCurCard );
	RegisterHotspot(
		rcCurCard.left, rcCurCard.top, rcCurCard.Width(), rcCurCard.Height(),
		false, 0, 0, &s_colWhite, &s_tdNormal, &s_colWhite, &s_tdNormal,
			(LPVOID)0 );

	// 4 개의 Shape 선택 영역

	CRect rcSpade = RegisterHotspot(
		rc.right - 16*14 - nDSBUnit - GetBitmapSize(IDB_SPADE).cx*2 - 8,
		rc.bottom - GetBitmapSize(IDB_SPADE).cy*2 - nDSBUnit, -1, -1, false,
		IDB_SPADE, 0,
		&s_colWhite, &m_aatdShape[0][0], &s_colWhite, &m_aatdShape[0][1],
		(LPVOID)1000 );

	RegisterHotspot(
		rcSpade.right, rcSpade.top, -1, -1, false,
		IDB_DIAMOND, 0,
		&s_colWhite, &m_aatdShape[1][0], &s_colWhite, &m_aatdShape[1][1],
		(LPVOID)1001 );

	RegisterHotspot(
		rcSpade.left, rcSpade.bottom, -1, -1, false,
		IDB_HEART, 0,
		&s_colWhite, &m_aatdShape[2][0], &s_colWhite, &m_aatdShape[2][1],
		(LPVOID)1002 );

	CRect rcClover = RegisterHotspot(
		rcSpade.right, rcSpade.bottom, -1, -1, false,
		IDB_CLOVER, 0,
		&s_colWhite, &m_aatdShape[3][0], &s_colWhite, &m_aatdShape[3][1],
		(LPVOID)1003 );

	// 프랜드 카드들
	rc = RegisterHotspot(		// 마이티
		1, 6, -1, -1, true, 0,
		CCard::GetMighty().GetString(Mo()->bUseTerm) + _T(" 프랜드"),
		&m_acolFriend[0], &m_atdFriend[0],
		&m_acolFriend[0], &m_atdFriendSel[0], (LPVOID)10 );
	RegisterHotspot(			// 조커
		rc.left, rc.bottom + nDSBUnit/2, -1, -1, false, 0,
		CCard::GetJoker().GetString(Mo()->bUseTerm) + _T(" 프랜드"),
		&m_acolFriend[1], &m_atdFriend[1],
		&m_acolFriend[1], &m_atdFriendSel[1], (LPVOID)11 );
	rc = RegisterHotspot(			// 기루다 A 프랜드
		9, 6, -1, -1, true, 0,
		Mo()->bUseTerm ? _T("기아 프랜드") : _T("기루다 A 프랜드"),
		&m_acolFriend[2], &m_atdFriend[2],
		&m_acolFriend[2], &m_atdFriendSel[2], (LPVOID)12 );
	RegisterHotspot(			// 기루다 K 프랜드
		rc.left, rc.bottom + nDSBUnit/2, -1, -1, false, 0,
		Mo()->bUseTerm ? _T("기카 프랜드") : _T("기루다 K 프랜드"),
		&m_acolFriend[3], &m_atdFriend[3],
		&m_acolFriend[3], &m_atdFriendSel[3], (LPVOID)13 );
	RegisterHotspot(			// 초구
		1, 10, -1, -1, true, 0,
		_T("초구 프랜드"),
		&m_acolFriend[4], &m_atdFriend[4],
		&m_acolFriend[4], &m_atdFriendSel[4], (LPVOID)14 );
	RegisterHotspot(			// 노 프랜드
		9, 10, -1, -1, true, 0,
		_T("노 프랜드"),
		&m_acolFriend[5], &m_atdFriend[5],
		&m_acolFriend[5], &m_atdFriendSel[5], (LPVOID)15 );

	// 플레이어
	if ( m_pState->apPlayers[0] ) {
		for ( int p = 1; p < m_pState->nPlayers; p++ )
			RegisterHotspot(
				1, 13+p, -1, -1, true, 0,
				m_pState->apPlayers[p]->GetName() + _T(" 프랜드"),
				&m_acolFriend[6+p], &m_atdFriend[6+p],
				&m_acolFriend[6+p], &m_atdFriendSel[6+p], (LPVOID)(16+p) );
	}
}

// 현재 카드를 그릴 영역을 리턴한다
void DFriend::CalcCurCardRect( LPRECT pRc )
{
	CRect rc; GetRect( &rc );
	int nDSBUnit = GetDSBUnit();
	CSize szCard = GetBitmapSize(IDB_SA);
	pRc->left = rc.right - nDSBUnit - szCard.cx;
	pRc->top = rc.bottom - 32 - nDSBUnit - 5 - szCard.cy;
	pRc->right = pRc->left + szCard.cx;
	pRc->bottom = pRc->top + szCard.cy;
}

// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
// 오버라이드 하여 구현해야 한다
// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
void DFriend::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	// 현재 선택된 카드, 또는 플레이어를 그린다
	bool bDrawX;
	CRect rcCurCard; CalcCurCardRect( &rcCurCard );

	if ( 0 < *m_pnFriend && *m_pnFriend < 54 ) {	// 카드
		bDrawX = m_lHand.Find( CCard(*m_pnFriend) )
			|| CCard(*m_pnFriend).IsJoker()	// 조커프랜드 불가
				&& !m_pState->pRule->bJokerFriend
			? true : false;
		m_pBoard->GetBmpMan()->DrawCard( pDC,
			CCard(*m_pnFriend), rcCurCard );
	}
	else {	// 플레이어, 노프랜드, 초구
		ASSERT( *m_pnFriend == 0 || *m_pnFriend == 100
			|| *m_pnFriend < -1 && *m_pnFriend > -m_pState->nPlayers-1 );
		bDrawX = *m_pnFriend == 0;
		DrawBitmap( pDC, IDB_PERSON, rcCurCard.left, rcCurCard.top,
			rcCurCard.Width(), rcCurCard.Height(),
			0, 0, rcCurCard.Width(), rcCurCard.Height() );
		// 텍스트와 위치 (s,x,y)
		CString s = *m_pnFriend == 100 ? _T("초구")
			: *m_pnFriend != 0 ?
				m_pState->apPlayers[-*m_pnFriend-1]->GetName()
				: _T("");
		int x = rcCurCard.left + rcCurCard.Width()/2 - GetTextExtent( 0, s ).cx/2;
		int y = rcCurCard.bottom - GetBitmapSize(IDB_SA).cy / 3;

		PutText( pDC, s, x, y, false, s_colWhite, s_tdShadeOutline );
	}

	// 이 카드가 손에 있는 카드이거나 노프랜드이면 x 표를 한다
	if ( bDrawX ) {
		CRect rc(
			rcCurCard.left + rcCurCard.Width()/2 - rcCurCard.Width()/3,
			rcCurCard.top + rcCurCard.Height()/2 - rcCurCard.Width()/3,
			rcCurCard.left + rcCurCard.Width()/2 + rcCurCard.Width()/3,
			rcCurCard.top + rcCurCard.Height()/2 + rcCurCard.Width()/3 );

		CPen pen( PS_SOLID, 10, RGB(255,0,0) );
		CPen* pnOld = pDC->SelectObject( &pen );
		pDC->MoveTo( rc.TopLeft() ); pDC->LineTo( rc.BottomRight() );
		pDC->MoveTo( rc.right, rc.top ); pDC->LineTo( rc.left, rc.bottom );
		pDC->SelectObject( pnOld );
	}

	// "다른 카드" 영역
	// 13 장의 샘플 카드를 그린다
	int nDSBUnit = GetDSBUnit();
	CRect rc; GetRect( &rc );
	int x = rc.right - 16*14 - nDSBUnit;
	int xDiff = 16;
	int cx = 16;
	int y = rc.bottom - 32 - nDSBUnit;
	int cy = 32;
	CCard c;
	for ( int i = 0; i < 14; i++, x += xDiff ) {

		if ( i == 0 ) c = CCard::GetJoker();	// 마지막 장은 조커
		else if ( i == 1 ) c = CCard( m_nShape, ACE );	// 첫장은 에이스
		else c = CCard( m_nShape, KING - (i-2) );

		bool bSel =		// 현재 선택된 Hotspot 인가
			m_posSel && m_lHotspot.GetAt(m_posSel).pVoid == (LPVOID)(100+i)
			&& !m_lHand.Find(c);
		int yDiff = bSel ? 3 : 0;

		m_pBoard->GetBmpMan()->DrawCardEx(
			pDC, (int)c, x, y - yDiff, cx, cy + yDiff,
			0, 0, cx, cy+yDiff );
	}

	// '다른 카드'
	PutText( pDC, _T("다른 카드:"), rc.left + nDSBUnit,
		rc.bottom - GetBitmapSize(IDB_SPADE).cy*2 - nDSBUnit*5/2,
		false, s_colWhite, s_tdShade );

	// 기타 텍스트
	static LPCTSTR asText[] = {
		_T("프랜드를 선택합니다"),
		_T(""),
		_T("원하는 '프랜드 카드'나 플레이어를"),
		_T("선택하고 여기를 클릭하세요")
	};
	CRect rcText;
	for ( int m = 0; m < sizeof(asText)/sizeof(LPCTSTR); m++ )
		rcText = PutText( pDC, asText[m], 1, 1+m,
							true, s_colWhite, s_tdShade );
	// '여기' 에 대한 화살표
	CPen pnMedium( PS_SOLID, 2, s_colWhite );
	pDC->SelectObject( &pnMedium );
	x = rcText.right + nDSBUnit;
	y = ( rcText.top + rcText.bottom ) / 2;
	pDC->MoveTo( x, y );
	x = ( rcCurCard.left + rcCurCard.right * 3 ) /4;	// 1:3 내분
	pDC->LineTo( x, y );
	y = rcCurCard.top - nDSBUnit/3;
	pDC->LineTo( x, y );
	pDC->LineTo( x - nDSBUnit/2, y - nDSBUnit/2 );
	pDC->MoveTo( x, y );
	pDC->LineTo( x + nDSBUnit/2, y - nDSBUnit/2 );
	pDC->SelectStockObject( WHITE_PEN );

	// '플레이어:'
	PutText( pDC, _T("플레이어:"),
		rc.left+nDSBUnit, rc.top+nDSBUnit*12+nDSBUnit/2, false,
		s_colWhite, s_tdShade );
}

// 핫 스팟을 클릭할 때 불리는 함수
// LPVOID 값이 0 이면 종료 (X자를 클릭) 요청
void DFriend::OnClick( LPVOID pVoid )
{
	int n = (int)pVoid;

	if ( n >= 1000 && n <= 1003 ) {
		// 모양 선택
		DSB::OnClickSound();
		SetCurShape( n-1000+SPADE );
	}
	else if ( n >= 100 && n <= 113 ) {
		// 카드 선택
		DSB::OnClickSound();
		CCard c;
		if ( n == 100 )			// Joker
			c = CCard(JOKER);
		else if ( n == 101 )	// ACE
			c = CCard(m_nShape,ACE);
		else
			c = CCard( m_nShape, 115-n );

		SetCurFriend( (int)c );
	}
	else if ( n >= 10 && n < 16 ) {
		// 특별 카드
		DSB::OnClickSound();
		if ( n == 10 ) SetCurFriend( (int)CCard::GetMighty() );
		else if ( n == 11 ) SetCurFriend( (int)CCard::GetJoker() );
		else if ( n == 12 && CCard::GetKiruda() )
			SetCurFriend( (int)CCard(CCard::GetKiruda(),ACE) );
		else if ( n == 13 && CCard::GetKiruda() )
			SetCurFriend( (int)CCard(CCard::GetKiruda(),KING) );
		else if ( n == 14 ) SetCurFriend( 100 );
		else if ( n == 15 ) SetCurFriend( 0 );
	}
	else if ( n >= 16 && n < 16+MAX_PLAYERS ) {
		// 플레이어
		DSB::OnClickSound();
		SetCurFriend( -( n - 16 ) - 1 );
	}
	else if ( n == 0 || n == 0xffffffff ) {	// 종료

		// 손에 있는 카드를 선택해서는 안된다
		if ( *m_pnFriend > 0 && *m_pnFriend < 54 ) {

			if ( m_lHand.Find(*m_pnFriend) ) {
				// 경고 메시지 표시
				if ( Mo()->bUseSound ) MessageBeep( MB_ICONEXCLAMATION );
				(new DShortMessage(m_pBoard))->Create(
					0, _T("이미 가지고 있거나 버린 카드입니다"),
					true, false, 2000 );
			} else if ( CCard(*m_pnFriend).IsJoker()	// 조커프랜드 불가
				&& !m_pState->pRule->bJokerFriend ) {
				if ( Mo()->bUseSound ) MessageBeep( MB_ICONEXCLAMATION );
				(new DShortMessage(m_pBoard))->Create(
					0, _T("프랜드로 부를수 없는 카드입니다"),
					true, false, 2000 );
			}
			else DSB::OnClick(0);
		}
		else {
			DSB::OnClick(0);
		}
	}
}
