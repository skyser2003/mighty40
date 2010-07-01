// Mighty.h : main header file for the MIGHTY application
//

#if !defined(AFX_MIGHTY_H__6433F782_C1B5_11D2_97F2_000000000000__INCLUDED_)
#define AFX_MIGHTY_H__6433F782_C1B5_11D2_97F2_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

// 변하지 않는 상수들
#define LAST_TURN_2MA 12
#define LAST_TURN 9
#define MAX_PLAYERS 7
#define MAX_SCORE_2MA 13
#define MAX_SCORE 20
#define HIGHSCORE_MAXLIMIT 25
#define DECK_SIZE 53
#define MIGHTY_VERSION 40

#define BASE_MONEY 100	// 1 인당 기본 점수 단위
#define MONEY_UNIT 10	// BASE_MONEY 1 당 실제 돈

#define WM_BOARDWRAPPER		(WM_USER+10)
#define WM_UPDATESCOREBOARD	(WM_USER+11)
#define WM_TOOLTIPUPDATE	(WM_USER+12)
#define WM_INFOBARUPDATE	(WM_USER+13)
#define WM_CALLSOCKPROC		(WM_USER+14)

// 툴 팁의 기본 폭
#define DEFAULT_TIP_CX	256
// 툴팁 테두리 마진
#define TOOLTIP_BORDER_MARGIN 5

// 여러가지 지연 시간들 (nDelayOneTurn : 0 ~ 10)

// 섞는 단계별 걸리는 시간 (0.2초)
#define DELAY_SUFFLESTEP		200
// 셋업이 된 후 나눠주기까지의 지연 시간
#define DELAY_SETUP_SUFFLING	( Mo()->nDelayOneTurn * 200 + 500 )
// 딜미스때 카드를 잠시 보여주는 시간
#define DELAY_DEALMISS			( Mo()->nDelayOneTurn * 200 * 2 + 500 )
// 당선자가 정해진 후 확정 메시지를 보여주기까지의 시간
#define DELAY_MASTERDECL		( Mo()->nDelayOneTurn * 200 * 2 + 500 )
// 프랜드가 밝혀 진 후 알림 메시지를 보여주는 기간
#define DELAY_FRIENDCARD		( 4000 )
// 최종 공약을 보여주는 시간
#define DELAY_ELECTIONEND		( Mo()->nDelayOneTurn * 200 * 2 + 2000 )
// 탈락 메시지와 그 결과를 보여주기 까지의 대기 시간
#define DELAY_KILL_AND_EFFECT	( Mo()->nDelayOneTurn * 200 * 2 + 500 )
// AI에서 탈락 메시지를 보내기 전 대기 시간
#define DELAY_KILL				( 0 )
// AI에서 공약 메시지를 보내기 전 대기 시간
#define DELAY_GOAL				( Mo()->nDelayOneTurn * 200 * 1 + 500 )
// 공약 DSB 의 타임아웃
#define DELAY_GOALDSBTIMEOUT	( Mo()->nDelayOneTurn * 200 * 2 + 500 )
// 출마포기 DSB 의 타임아웃
#define DELAY_GIVEUPDSBTIMEOUT	( Mo()->nDelayOneTurn * 200 * 2 + 500 )
// 조커/조커콜 DSB 를 보여주는 시간
#define DELAY_JOKERDSB			( Mo()->nDelayOneTurn * 200 * 5 )

#include "resource.h"       // main symbols
#include "state.h"
#include "rule.h"
#include "option.h"
#include "card.h"
#include "msg.h"


class CInfoBar;
class CMightyToolTip;
class DScoreBoard;

/////////////////////////////////////////////////////////////////////////////
// CMightyApp:
// See Mighty.cpp for the implementation of this class
//

class CMightyApp : public CWinApp
{
public:
	CMightyApp();
	~CMightyApp();

	COption* GetOption() { return &m_option; }
	CInfoBar* GetInfoBar() { return m_pInfoBar; }
	CMightyToolTip* GetToolTip() { return m_pToolTip; }
	DScoreBoard* GetScoreBoard() { return m_pScoreBoard; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMightyApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation

public:
	//{{AFX_MSG(CMightyApp)
	afx_msg void OnAppAbout();
	afx_msg void OnAppOption();
	afx_msg void OnUpdateAppScoreboard(CCmdUI* pCmdUI);
	afx_msg void OnAppScoreboard();
	afx_msg void OnHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	COption m_option;
	CInfoBar* m_pInfoBar;
	CMightyToolTip* m_pToolTip;
	DScoreBoard* m_pScoreBoard;
	bool m_bInBoss;
};

// 옵션을 얻는다
inline COption* Mo()
{
	return ((CMightyApp*)AfxGetApp())->GetOption();
}

// 메인 윈도우를 얻는다
inline CWnd* Mw()
{
	return AfxGetApp()->m_pMainWnd;
}
// 메인 프레임을 얻는다
class CMainFrame;
inline CMainFrame* Mf()
{
	return (CMainFrame*)AfxGetApp()->m_pMainWnd;
}

// 상태막대를 얻는다
inline CInfoBar* Ib()
{
	return ((CMightyApp*)AfxGetApp())->GetInfoBar();
}

// 툴팁 윈도우를 얻는다
inline CMightyToolTip* Tt()
{
	return ((CMightyApp*)AfxGetApp())->GetToolTip();
}

// 점수판을 얻는다
inline DScoreBoard* Sb()
{
	return ((CMightyApp*)AfxGetApp())->GetScoreBoard();
}

// 돈을 포맷한다
inline CString FormatMoney( int nMoney, int nPlayers )
{
	CString sRet;
	if ( Mo()->bMoneyAsSupport )
		sRet.Format( _T("%d.%d%%"), nMoney * 100 / BASE_MONEY / nPlayers,
					nMoney * 1000 / BASE_MONEY / nPlayers % 10 );
	else sRet.Format( _T("%d"), nMoney * MONEY_UNIT );
	return sRet;
}

// 유틸리티들

// 비트맵의 크기를 얻는다
CSize GetBitmapSize( UINT idBitmap );
// 비트맵을 그린다
void DrawBitmap( CDC* pDC, UINT idBitmap,
		int xTarget, int yTarget,
		int cxTarget = -1, int cyTarget = -1,
		int xSource = -1, int ySource = -1,
		int cxSource = -1, int cySource = -1 );
// 사운드를 연주한다
// ( bStop 이 참이면 이전의 소리를 멈추고 연주 )
void PlaySound( UINT idWave, BOOL bStop = FALSE );


// CCriticalSection 에 VERIFY 기능을 추가한다
class CMightyCriticalSection
{
public:
	void Lock() const { VERIFY( ((CCriticalSection&)m_cs).Lock() ); }
	void Unlock() const { VERIFY( ((CCriticalSection&)m_cs).Unlock() ); }
private:
	CCriticalSection m_cs;
};
#define CCriticalSection CMightyCriticalSection

// 포인터를 자동으로 delete 하는 클래스
template<class TYPE>
struct CAutoDelete
{
	CAutoDelete( TYPE** ptr ) : pp(ptr) {}
	~CAutoDelete() { if ( pp ) delete *pp; }
	void Attach( TYPE** t ) { delete *pp; pp = t; }
	TYPE** Detach() { TYPE** t = pp; pp = 0; return t; }
	TYPE** pp;
};

#define AUTODELETE_MSG( PTR ) CAutoDelete<CMsg> _adm##PTR( &PTR )
#define AUTODELETE_MSG_EX( PTR, VALUE ) CAutoDelete<CMsg> VALUE( &PTR )

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIGHTY_H__6433F782_C1B5_11D2_97F2_000000000000__INCLUDED_)
