// DElection.cpp: implementation of the D2MA class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "BoardWrap.h"
#include "BmpMan.h"
#include "DSB.h"
#include "DEtc.h"
#include "D2MA.h"

#include "Play.h"
#include "Player.h"
#include "MFSM.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// 카드 고르기 DSB

// pcToKill : 이 DSB 가 결과 카드를 저장할 장소 (결과-값 독립변수)
// plShow   : 보이는 (위에 있는) 카드
// plHide   : 숨겨진 (아래에 있는) 카드
void DSelect2MA::Create( CEvent* pEvent, int* selecting, CCardList* plCard )
{
	m_pselecting = selecting;
	m_pcShow = plCard->GetAt(plCard->POSITIONFromIndex(0));
	m_pcHide = plCard->GetAt(plCard->POSITIONFromIndex(1));

	SetFixed();

	int nUnit = GetDSBUnit();

	// 최소의 크기는 ( 15 * 14 ) * ( card.cy + 18 + nUnit ) 의 카드 선택열이 들어가야 함
	int cxDSB = ( 15*14 + nUnit-1 ) / nUnit;
	int cyDSB = ( GetBitmapSize(IDB_SA).cy + 18 + nUnit + nUnit-1 ) / nUnit;

	// 다른 내용 들어갈 크기
	cxDSB += 5;
	cyDSB += 1;

	CRect rc = m_pBoard->CalcPlayerExtent( 0 );

	DSB::Create2(
		rc.left+rc.Width()/2 - cxDSB*GetDSBUnit()/2,
		rc.bottom - cyDSB*GetDSBUnit(),
		cxDSB, cyDSB, -1 );

	SetAction( pEvent );
}

// 초기화 ( 생성된 후 호출됨 )
void DSelect2MA::OnInit()
{
	DSB::OnInit();

	int nDSBUnit = GetDSBUnit();
	CRect rc; GetRect( &rc );

	CRect rcShowCard; CalcShowCardRect( &rcShowCard );
	RegisterHotspot(
		rcShowCard.left, rcShowCard.top, rcShowCard.Width(), rcShowCard.Height(),
		false, 0, 0, &s_colWhite, &s_tdNormal, &s_colWhite, &s_tdNormal,
			(LPVOID)0 );
	CRect rcHideCard; CalcHideCardRect( &rcHideCard );
	RegisterHotspot(
		rcHideCard.left, rcHideCard.top, rcHideCard.Width(), rcHideCard.Height(),
		false, 0, 0, &s_colWhite, &s_tdNormal, &s_colWhite, &s_tdNormal,
			(LPVOID)1 );
}

// 보이는 카드를 그릴 영역을 리턴한다
void DSelect2MA::CalcShowCardRect( LPRECT pRc )
{
	CRect rc; GetRect( &rc );
	int nDSBUnit = GetDSBUnit();
	CSize szCard = GetBitmapSize(IDB_SA);
	pRc->left = rc.right - nDSBUnit - szCard.cx * 2;
	pRc->top = rc.bottom - 14 - nDSBUnit - 5 - szCard.cy;
	pRc->right = pRc->left + szCard.cx;
	pRc->bottom = pRc->top + szCard.cy;
}
// 숨겨진 카드를 그릴 영역을 리턴한다.
void DSelect2MA::CalcHideCardRect( LPRECT pRc )
{
	CRect rc; GetRect( &rc );
	int nDSBUnit = GetDSBUnit();
	CSize szCard = GetBitmapSize(IDB_SA);
	pRc->left = rc.right - nDSBUnit - szCard.cx;
	pRc->top = rc.bottom - 14 - nDSBUnit - 5 - szCard.cy;
	pRc->right = pRc->left + szCard.cx;
	pRc->bottom = pRc->top + szCard.cy;
}

// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
// 오버라이드 하여 구현해야 한다
// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
void DSelect2MA::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	// 보이는 카드를 그린다
	CRect rcShowCard; CalcShowCardRect( &rcShowCard );
	m_pBoard->GetBmpMan()->DrawCard( pDC, m_pcShow, rcShowCard );
	// 숨겨진 카드를 그린다
	CRect rcHideCard; CalcHideCardRect( &rcHideCard );
#ifdef _DEBUG
	m_pBoard->GetBmpMan()->DrawCard( pDC, m_pcHide, rcHideCard );
#else
	m_pBoard->GetBmpMan()->DrawCard( pDC, CCard(0/*back*/), rcHideCard );
#endif

	static LPCTSTR asText[] = {
		_T("보이는 카드와"),
		_T("숨겨진 카드중 "),
		_T("하나를 선택"),
		_T("하세요"),
	};
	for ( int m = 0; m < sizeof(asText)/sizeof(LPCTSTR); m++ )
		PutText( pDC, asText[m], 1, 3+m, true, s_colWhite, s_tdShade );
}

// 핫 스팟을 클릭할 때 불리는 함수
// LPVOID 값이 0 이면 종료 (X자를 클릭) 요청
void DSelect2MA::OnClick( LPVOID pVoid )
{
	int n = (int)pVoid;

	if ( n == 1 ) {
		// 숨겨진 ( 아래쪽 ) 카드 선택
		DSB::OnClickSound();
		*m_pselecting = 1;
		DSB::OnClick(0);
		return;
	}
	else if ( n == 0 || n == 0xffffffff ) {	// 종료

		DSB::OnClickSound();
		*m_pselecting = 0;
		DSB::OnClick(0);
		return;
	}

	RECT rc;
	GetRect( &rc );
	m_pBoard->UpdateDSB( &rc, true );
}
