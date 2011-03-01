// PlayerDummy.cpp: implementation of the CPlayerDummy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "Play.h"
#include "Player.h"
#include "PlayerDummy.h"
#include "MFSM.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlayerDummy 본체

CPlayerDummy::CPlayerDummy( int nID, CWnd* pCallback )
	: CPlayer( nID, _T("System"), pCallback )
{
}

CPlayerDummy::~CPlayerDummy()
{
}

// CPlayer 구현

void CPlayerDummy::OnSelect2MA( int* selecting, CCardList* plCard, CEvent* e )
{
	e->SetEvent();
}

void CPlayerDummy::OnKillOneFromSix( CCard* pcCardToKill,
	CCardList* plcFailedCardsTillNow, CEvent* e )
{
	e->SetEvent();
}

void CPlayerDummy::OnElection( CGoal* pNewGoal, CEvent* e )
{
	e->SetEvent();
}

void CPlayerDummy::OnTurn( CCard* pc, int* pe, CEvent* e )
{
	e->SetEvent();
}
