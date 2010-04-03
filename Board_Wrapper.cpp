// Board_Wrapper.cpp : Wrapper proxy/stub
//

#include "stdafx.h"
#include "Mighty.h"

#include "BoardWrap.h"
#include "BmpMan.h"
#include "Board.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


LRESULT CBoard::OnWrapper( WPARAM wParam, LPARAM lParam )
{
	CBoardWrap::WRAPPER_ARG& a = *(CBoardWrap::WRAPPER_ARG*)lParam;

	switch ( wParam ) {

	case CBoardWrap::wfGetWrapper:
		return (LRESULT)GetWrapper();
	case CBoardWrap::wfSetMFSM:
		SetMFSM( (CMFSM*)a.PB.p ); return 0;
	case CBoardWrap::wfGetBmpMan:
		return (LRESULT)GetBmpMan();
	case CBoardWrap::wfReloadPlayerNames:
		ReloadPlayerNames(); return 0;
	case CBoardWrap::wfUpdateDSB_1:
		UpdateDSB( a.I4B.i4[0], a.I4B.i4[1], a.I4B.i4[2], a.I4B.i4[3], a.I4B.b ); return 0;
	case CBoardWrap::wfUpdateDSB_2:
		UpdateDSB( (LPCRECT)a.PB.p, a.PB.b ); return 0;
	case CBoardWrap::wfUpdatePlayer_1:
		UpdatePlayer( a.I5B2.i5[0], a.I5B2.i5[1], a.I5B2.i5[2], a.I5B2.i5[3], a.I5B2.i5[4], a.I5B2.b2[0], a.I5B2.b2[1] ); return 0;
	case CBoardWrap::wfUpdatePlayer_2:
		UpdatePlayer( a.IPB2.i, (LPRECT)a.IPB2.p, a.IPB2.b2[0], a.IPB2.b2[1] ); return 0;
	case CBoardWrap::wfSetTurnRect:
		SetTurnRect( a.I.i ); return 0;
	case CBoardWrap::wfWaitWhile:
		WaitWhile( a.IP.i, (CEvent*)a.IP.p ); return 0;
	case CBoardWrap::wfWaitSelection:
		WaitSelection( (CEvent*)a.P2.p2[0], (long*)a.P2.p2[1] ); return 0;
	case CBoardWrap::wfCancelSelection:
		CancelSelection(); return 0;
	case CBoardWrap::wfFlyCard:
		FlyCard( a.I8P.i8[0], a.I8P.i8[1], a.I8P.i8[2], a.I8P.i8[3],
			a.I8P.i8[4], a.I8P.i8[5], a.I8P.i8[6], a.I8P.i8[7],
			(CEvent*)a.I8P.p ); return 0;
	case CBoardWrap::wfSuffleCards:
		SuffleCards( a.IP.i, (CEvent*)a.IP.p ); return 0;
	case CBoardWrap::wfSetDealMiss:
		SetDealMiss( a.I.i ); return 0;
	case CBoardWrap::wfGetDealMiss:
		return (LPARAM)GetDealMiss();
	case CBoardWrap::wfSetSelection_1:
		SetSelection( a.IB.i, a.IB.b ); return 0;
	case CBoardWrap::wfSetSelection_2:
		SetSelection( a.B.b ); return 0;
	case CBoardWrap::wfGetSelection:
		return (LPARAM)GetSelection( a.I.i );
	case CBoardWrap::wfGetSelectionCount:
		return (LPARAM)GetSelectionCount();
	case CBoardWrap::wfAddDSB:
		AddDSB( (DSB*)a.P.p ); return 0;
	case CBoardWrap::wfSetDSBTimer:
		SetDSBTimer( (DSB*)a.P.p ); return 0;
	case CBoardWrap::wfRemoveDSB:
		return (LPARAM)RemoveDSB( (DSB*)a.P.p );
	case CBoardWrap::wfGetDSBUnit:
		return (LPARAM)GetDSBUnit();
	case CBoardWrap::wfGetFont:
		return (LPARAM)GetFont( a.I.i );
	case CBoardWrap::wfGetTextExtent:
		*(CSize*)a.P2I.p2[0] = GetTextExtent( a.P2I.i, (LPCTSTR)a.P2I.p2[1] );
		return 0;
	case CBoardWrap::wfCalcPlayerExtent:
		*(RECT*)a.P2I.p2[0] = CalcPlayerExtent( a.P2I.i, (bool*)a.P2I.p2[1] );
		return 0;
	case CBoardWrap::wfFloatSayDSB:
		FloatSayDSB( a.IP.i, (LPCTSTR)a.IP.p ); return 0;
	case CBoardWrap::wfFloatGoalDSB:
		FloatGoalDSB( a.IP.i, (const CGoal*)a.IP.p ); return 0;
	case CBoardWrap::wfSetElectionDSB:
		SetElectionDSB( (DSB*)a.P.p ); return 0;
	default:
		ASSERT(0);
	}

	return 0;
}


static CFont s_fntTemp;

CBoardWrap::CBoardWrap( CWnd* pBoard )
{
	m_pBoard = pBoard;
	if ( !s_fntTemp.GetSafeHandle() ) {
		LOGFONT lf;
		::GetObject( GetStockObject(SYSTEM_FONT), sizeof(lf), &lf );
		s_fntTemp.CreateFontIndirect( &lf );
	}
}
CBoardWrap::~CBoardWrap()
{
}
void CBoardWrap::GetClientRect( LPRECT prc ) const
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) *prc = CRect(0,0,10,10);
	else m_pBoard->GetClientRect( prc );
}
CBoardWrap* CBoardWrap::GetWrapper()
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return 0;
	return (CBoardWrap*)m_pBoard->SendMessage( WM_BOARDWRAPPER, wfGetWrapper, 0 );
}
void CBoardWrap::SetMFSM( CMFSM* pMFSM )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return;
	WRAPPER_ARG wa;
	wa.PB.p = pMFSM;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfSetMFSM, (LPARAM)&wa );
}
CBmpMan* CBoardWrap::GetBmpMan()
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) { ASSERT(0); return 0; }
	return (CBmpMan*)m_pBoard->SendMessage( WM_BOARDWRAPPER, wfGetBmpMan, 0 );
}
void CBoardWrap::ReloadPlayerNames()
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) { ASSERT(0); return; }
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfReloadPlayerNames, 0 );
}
void CBoardWrap::UpdateDSB( int x, int y, int cx, int cy, bool bUpdateNow )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return;
	WRAPPER_ARG wa;
	wa.I4B.i4[0] = x; wa.I4B.i4[1] = y;
	wa.I4B.i4[2] = cx; wa.I4B.i4[3] = cy;
	wa.I4B.b = bUpdateNow;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfUpdateDSB_1, (LPARAM)&wa );
}
void CBoardWrap::UpdateDSB( LPCRECT prc, bool bUpdateNow )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return;
	WRAPPER_ARG wa;
	wa.PB.p = (void*)prc; wa.PB.b = bUpdateNow;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfUpdateDSB_2, (LPARAM)&wa );
}
void CBoardWrap::UpdatePlayer( int nPlayer, int x, int y, int cx, int cy, bool bUpdateNow, bool bSelectionRegion )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return;
	WRAPPER_ARG wa;
	wa.I5B2.i5[0] = nPlayer;
	wa.I5B2.i5[0] = x; wa.I5B2.i5[1] = y;
	wa.I5B2.i5[2] = cx; wa.I5B2.i5[3] = cy;
	wa.I5B2.b2[0] = bUpdateNow; wa.I5B2.b2[1] = bSelectionRegion;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfUpdatePlayer_1, (LPARAM)&wa );
}
void CBoardWrap::UpdatePlayer( int nPlayer, LPRECT prc, bool bUpdateNow, bool bSelectionRegion )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return;
	WRAPPER_ARG wa;
	wa.IPB2.i = nPlayer; wa.IPB2.p = prc;
	wa.IPB2.b2[0] = bUpdateNow; wa.IPB2.b2[1] = bSelectionRegion;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfUpdatePlayer_2, (LPARAM)&wa );
}
void CBoardWrap::SetTurnRect( int nPlayer )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return;
	WRAPPER_ARG wa;
	wa.I.i = nPlayer;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfSetTurnRect, (LPARAM)&wa );
}
void CBoardWrap::WaitWhile( int nMillisecond, CEvent* pEvent )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) { pEvent->SetEvent(); return; }
	WRAPPER_ARG wa;
	wa.IP.i = nMillisecond; wa.IP.p = pEvent;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfWaitWhile, (LPARAM)&wa );
}
void CBoardWrap::WaitSelection( CEvent* pEvent, long* pSelectedCard )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) { *pSelectedCard = 0; pEvent->SetEvent(); return; }
	WRAPPER_ARG wa;
	wa.P2.p2[0] = pEvent; wa.P2.p2[1] = pSelectedCard;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfWaitSelection, (LPARAM)&wa );
}
void CBoardWrap::CancelSelection()
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfCancelSelection, 0 );
}
void CBoardWrap::FlyCard( int nCard, int nSpeed, int nFrom, int nFromPlayer, int nFromIndex, int nTo, int nToPlayer, int nToIndex, CEvent* pEvent )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) { pEvent->SetEvent(); return; }
	WRAPPER_ARG wa;
	wa.I8P.i8[0] = nCard; wa.I8P.i8[1] = nSpeed;
	wa.I8P.i8[2] = nFrom; wa.I8P.i8[3] = nFromPlayer; wa.I8P.i8[4] = nFromIndex;
	wa.I8P.i8[5] = nTo; wa.I8P.i8[6] = nToPlayer; wa.I8P.i8[7] = nToIndex;
	wa.I8P.p = pEvent;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfFlyCard, (LPARAM)&wa );
}
void CBoardWrap::SuffleCards( int nCount, CEvent* pEvent )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) { pEvent->SetEvent(); return; }
	WRAPPER_ARG wa;
	wa.IP.i = nCount; wa.IP.p = pEvent;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfSuffleCards, (LPARAM)&wa );
}
void CBoardWrap::SetDealMiss( int nPlayerID )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return;
	WRAPPER_ARG wa;
	wa.I.i = nPlayerID;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfSetDealMiss, (LPARAM)&wa );
}
int CBoardWrap::GetDealMiss()
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return -1;
	return (int)m_pBoard->SendMessage( WM_BOARDWRAPPER, wfGetDealMiss, 0 );
}
void CBoardWrap::SetSelection( int nIndex, bool bSelect )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return;
	WRAPPER_ARG wa;
	wa.IB.i = nIndex; wa.IB.b = bSelect;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfSetSelection_1, (LPARAM)&wa );
}
bool CBoardWrap::GetSelection( int nIndex )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return false;
	WRAPPER_ARG wa;
	wa.I.i = nIndex;
	return !!m_pBoard->SendMessage( WM_BOARDWRAPPER, wfGetSelection, (LPARAM)&wa );
}
void CBoardWrap::SetSelection( bool bSelect )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return;
	WRAPPER_ARG wa;
	wa.B.b = bSelect;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfSetSelection_2, (LPARAM)&wa );
}
int CBoardWrap::GetSelectionCount()
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return 0;
	return (int)m_pBoard->SendMessage( WM_BOARDWRAPPER, wfGetSelectionCount, 0 );
}
void CBoardWrap::AddDSB( DSB* pDSB )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return;
	WRAPPER_ARG wa;
	wa.P.p = pDSB;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfAddDSB, (LPARAM)&wa );
}
void CBoardWrap::SetDSBTimer( DSB* pDSB )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return;
	WRAPPER_ARG wa;
	wa.P.p = pDSB;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfSetDSBTimer, (LPARAM)&wa );
}
bool CBoardWrap::RemoveDSB( DSB* pDSB )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) { return true; }
	WRAPPER_ARG wa;
	wa.P.p = pDSB;
	return !!m_pBoard->SendMessage( WM_BOARDWRAPPER, wfRemoveDSB, (LPARAM)&wa );
}
int CBoardWrap::GetDSBUnit()
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return 1;
	return (int)m_pBoard->SendMessage( WM_BOARDWRAPPER, wfGetDSBUnit, 0 );
}
CFont* CBoardWrap::GetFont( int nSize )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return &s_fntTemp;
	WRAPPER_ARG wa;
	wa.I.i = nSize;
	return (CFont*)m_pBoard->SendMessage( WM_BOARDWRAPPER, wfGetFont, (LPARAM)&wa );
}
CSize CBoardWrap::GetTextExtent( int nSize, LPCTSTR s )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return CSize(10,10);
	CSize sz;
	WRAPPER_ARG wa;
	wa.P2I.p2[0] = &sz;
	wa.P2I.i = nSize;
	wa.P2I.p2[1] = (void*)s;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfGetTextExtent, (LPARAM)&wa );
	return sz;
}
RECT CBoardWrap::CalcPlayerExtent( int nPlayerNum, bool* pbVert )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return CRect(0,0,10,10);
	RECT rc;
	WRAPPER_ARG wa;
	wa.P2I.p2[0] = &rc;
	wa.P2I.i = nPlayerNum;
	wa.P2I.p2[1] = pbVert;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfCalcPlayerExtent, (LPARAM)&wa );
	return rc;
}
void CBoardWrap::FloatSayDSB( int nPlayerID, LPCTSTR sMsg )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return;
	WRAPPER_ARG wa;
	wa.IP.i = nPlayerID; wa.IP.p = (void*)sMsg;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfFloatSayDSB, (LPARAM)&wa );
}
void CBoardWrap::FloatGoalDSB( int nPlayerID, const CGoal* pGoal )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return;
	WRAPPER_ARG wa;
	wa.IP.i = nPlayerID; wa.IP.p = (void*)pGoal;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfFloatGoalDSB, (LPARAM)&wa );
}
void CBoardWrap::SetElectionDSB( DSB* pDSB )
{
	if ( ((CBoard*)m_pBoard)->m_bDoNotSendMeAnyMoreMessage ) return;
	WRAPPER_ARG wa;
	wa.P.p = pDSB;
	m_pBoard->SendMessage( WM_BOARDWRAPPER, wfSetElectionDSB, (LPARAM)&wa );
}
