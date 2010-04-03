// MFSM_AISupp.cpp: implementation of the CMFSM AI Support
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "Play.h"
#include "Player.h"
#include "MFSM.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


void CMFSM::InvokeAIThread()
{
	m_bAIThreadTerminate = false;
	m_bAIThreadTerminated = false;
	m_nAIThreadRequest = -1;
	m_pAIThread = AfxBeginThread( AIThread, this, THREAD_PRIORITY_IDLE );
}

// AI 쓰레드에게, 자신의 함수를 호출하라고 요청한다
// nType : 0 OnElection 1 OnKillOneFromSix / 2 OnTurn
void CMFSM::RequestCallAIThread( CPlay* pPlay, int nType,
					CEvent* pEvent, LPVOID arg1, LPVOID arg2 )
{
	m_csAIThread.Lock();

	m_pAIThreadTargetPlay = pPlay;
	m_peAIThreadRequestedEvent = pEvent;
	m_nAIThreadRequest = nType;
	m_apArgs[0] = arg1;
	m_apArgs[1] = arg2;
	m_eAIThread.SetEvent();

	m_csAIThread.Unlock();
}

void CMFSM::CallAIThread( CPlay* pPlay, int nType,
					CEvent* pEvent, LPVOID arg1, LPVOID arg2 )
{
	switch ( nType ) {
	case 0 : pPlay->OnElection( (CGoal*)arg1 ); break;
	case 1 : pPlay->OnKillOneFromSix( (CCard*)arg1, (CCardList*)arg2 ); break;
	case 2 : pPlay->OnTurn( (CCard*)arg1, (int*)arg2 ); break;
	}
	pEvent->SetEvent();
}

UINT CMFSM::AIThread( LPVOID pVoid )
{	return ((CMFSM*)pVoid)->AIThread(); }

UINT CMFSM::AIThread()
{
	TRACE("AIThread Begin\n");

	while ( !m_bAIThreadTerminate ) {

		m_eAIThread.Lock(1000);

		if ( m_bAIThreadTerminate ) break;

		m_csAIThread.Lock();

		if ( m_nAIThreadRequest != -1 ) {

			CallAIThread( m_pAIThreadTargetPlay, m_nAIThreadRequest,
				m_peAIThreadRequestedEvent, m_apArgs[0], m_apArgs[1] );
			m_nAIThreadRequest = -1;
		}

		m_csAIThread.Unlock();
	}

	TRACE("AIThread Terminated\n");
	m_bAIThreadTerminated = true;
	return 0;
}
