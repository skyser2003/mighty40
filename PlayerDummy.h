// PlayerDummy.h: interface for the CPlayerDummy class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYERDUMMY_H__36075665_D1BC_11F2_931A_0000212035B8__INCLUDED_)
#define AFX_PLAYERDUMMY_H__36075665_D1BC_11F2_931A_0000212035B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// 더미 플레이어 객체
// 관전자 자리에 있는 컴퓨터 플레이어가 해당


class CPlayerDummy : public CPlayer
{
public:
	CPlayerDummy( int nID, CWnd* pCallback = NULL );
	virtual ~CPlayerDummy();

	// CPlayer 인터페이스 구현

	virtual void OnSelect2MA( int* selecting, CCardList* plCard, CEvent* e );
	virtual void OnKillOneFromSix( CCard* pcCardToKill,
		CCardList* plcFailedCardsTillNow, CEvent* );
	virtual void OnElection( CGoal* pNewGoal, CEvent* );
	virtual void OnTurn( CCard* pc, int* pe, CEvent* e );
};

#endif // !defined(AFX_PLAYERDUMMY_H__36075665_D1BC_11F2_931A_0000212035B8__INCLUDED_)
