// DStartUp.cpp: implementation of the DStartUp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"
#include "DSB.h"
#include "DStartUp.h"
#include "DEtc.h"

#include "MFSM.h"
#include "Play.h"
#include "Player.h"
#include "PlayerHuman.h"
#include "PlayerMai.h"

#include "ZSocket.h"
#include "BoardWrap.h"
#include "DConnect.h"
#include "InfoBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// nMode  0 : 초기 메뉴  1 : 멀티플레이어 메뉴
void DStartUp::Create( int nMode )
{
	m_nMode = nMode;

	if ( m_nMode == 0 )	// 초기 메뉴
		DSB::Create( 0, 0, 20, 20, -1 );
	else
		DSB::Create( 0, 0, 18, 17, -1 );

	// 이 DSB 가 생성되는 상태는, 게임이 없는 초기 상태이므로
	// 상태바를 리셋한다
	Ib()->ShowChat( false );
	Ib()->Reset();
}

// 초기화 ( 생성된 후 호출됨 )
void DStartUp::OnInit()
{
	DSB::OnInit();

	RegisterHotspot(
		3, 4, -1, -1, true, 0,
		m_nMode == 0 ? _T("나홀로 마이티") : _T("여기서 하기"),
		&s_colWhite, &s_tdMidium, &s_colCyan, &s_tdMidiumOutline,
		(LPVOID)1 );
	RegisterHotspot(
		3, 7, -1, -1, true, 0,
		m_nMode == 0 ? _T("다함께 마이티") : _T("다른곳으로 접속"),
		&s_colWhite, &s_tdMidium, &s_colCyan, &s_tdMidiumOutline,
		(LPVOID)2 );
	RegisterHotspot(
		3, 13, -1, -1, true, 0,
		m_nMode == 0 ? _T("마이티 끝내기") : _T("이전 메뉴로"),
		&s_colWhite, &s_tdMidium, &s_colCyan, &s_tdMidiumOutline,
		(LPVOID)0 );

	if ( m_nMode == 0 )
		RegisterHotspot(
			3, 10, -1, -1, true, 0,
			_T("마이티는..."),
			&s_colWhite, &s_tdMidium, &s_colCyan, &s_tdMidiumOutline,
			(LPVOID)3 );
	else
		RegisterHotspot(
			3, 10, -1, -1, true, 0,
			_T("Mighty.Net"),
			&s_colGray, &s_tdMidium, &s_colGray, &s_tdMidium,
			(LPVOID)3 );
}

// 핫 스팟이 Highlight 되거나 Normal 로 될 때 호출되는 함수
// bHighlight 가 참이면 highlight 되었음
void DStartUp::OnHighlight( HOTSPOT& hs, bool bHighlight )
{
	if ( !bHighlight ) Ib()->Reset();
	else if ( m_nMode == 0 ) {

		switch ((int)hs.pVoid) {
		case 0: Ib()->SetText( _T("마이티 게임을 종료합니다") ); break;
		case 1: Ib()->SetText( _T("혼자서 컴퓨터와 마이티 게임을 합니다") ); break;
		case 2: Ib()->SetText( _T("네트워크를 통해서 다른 사용자와 마이티 게임을 합니다") ); break;
		case 3: Ib()->SetText( _T("마이티 정보를 보여줍니다") ); break;
		}
	}
	else {

		switch ((int)hs.pVoid) {
		case 0: Ib()->SetText( _T("이전 메뉴로 돌아갑니다") ); break;
		case 1: Ib()->SetText( _T("딜러가 되어 다른 사용자의 접속을 기다립니다") ); break;
		case 2: Ib()->SetText( _T("다른 딜러에게 접속합니다") ); break;
		case 3: Ib()->SetText( _T("Mighty.Net 에 접속합니다") ); break;
		}
	}
}

// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
// 오버라이드 하여 구현해야 한다
// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
void DStartUp::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	if ( m_nMode == 0 ) {

		PutText( pDC, _T("마이티 게임을 시작합니다 !!"), 2, 1, true,
			s_colYellow, s_tdShade );
		PutText( pDC, _T("Mighty Network Ver 3.2"), 3, 16, true,
			s_colCyan, s_tdShade );
		PutText( pDC, _T("Copyright 1999 장문성"), 3, 17, true,
			s_colCyan, s_tdShade );
	}
	else {

		PutText( pDC, _T("다른 사용자와 함께 합니다"), 2, 1, true,
			s_colYellow, s_tdShade );
	}
}

// 핫 스팟을 클릭할 때 불리는 함수
// LPVOID 값이 0 이면 종료 (X자를 클릭) 요청
void DStartUp::OnClick( LPVOID pVoid )
{
	if ( m_nMode == 0 ) {	// 초기 메뉴

		// 이 다이얼로그가 pVoid==0 으로 닫히면
		// 초기메뉴일 경우 게임 종료임
		if ( !pVoid )
			Mw()->PostMessage( WM_CLOSE );

		switch ( (int)pVoid ) {

		case 0: DSB::OnClick(0); break;

		case 0xffffffff:
		case 1: {
			VERIFY( CreateOnePlayerGame() );
			break;
		}
		case 2:
			(new DStartUp(m_pBoard))->Create( 1 );
			DSB::OnClick(0); break;
			break;
		case 3:
			(new DAbout(m_pBoard))->Create();
			break;
		}
	}
	else {	// 멀티플레이어 메뉴

		// 이 다이얼로그가 pVoid==0 으로 닫히면
		// 초기 메뉴를 호출
		if ( !pVoid )
			(new DStartUp(m_pBoard))->Create( 0 );

		switch ( (int)pVoid ) {

		case 0: DSB::OnClick(0); break;

		case 0xffffffff:
		case 1:		// 서버가 됨
			Ib()->ShowChat();	// 채팅창 표시
			(new DConnect(m_pBoard))->Create( 0 );
			DSB::OnClick(0);
			break;
		case 2:		// 다른 곳으로 접속
			Ib()->ShowChat();	// 채팅창 표시
			(new DConnecting(m_pBoard))->Create();
			DSB::OnClick(0);
			break;
		case 3:		// Mighty.Net
			break;
		}
	}
}

void DStartUp::OnClickSound()
{
	PlaySound( IDW_CHULK, true );
}

// 현재 옵션에 기초해서 1인용 마이티 MFSM 을 생성(new)한다
// AI 파일 로드 실패 시 에러 DSB 를 띄우고
// 0 을 리턴한다
bool DStartUp::CreateOnePlayerGame()
{
	int nPlayers = Mo()->rule.nPlayerNum;

	CPlayer* apPlayers[MAX_PLAYERS];

	// Human Player 생성
	apPlayers[0] = new CPlayerHuman(
		0, Mo()->aPlayer[0].sName, *m_pBoard );

	// AI Players 생성
	bool bFailed = false;
	CString sFailedDLL;
	for ( int i = 1; i < nPlayers; i++ ) {

		CPlayerMai* pPlayer = new CPlayerMai(
			i, Mo()->aPlayer[i].sName, *m_pBoard,
			Mo()->aPlayer[i].sAIDLL, Mo()->aPlayer[i].sSetting );
		apPlayers[i] = pPlayer;
		if ( !pPlayer->IsDLLWorking() ) {
			bFailed = true;
			sFailedDLL = Mo()->aPlayer[i].sAIDLL;
		}
	}

	// 돈을 적당히 준다
	for ( int j = 0; j < nPlayers; j++ )
		apPlayers[j]->SetMoney( BASE_MONEY );

	// 이제 SetMFSM 을 하는 순간 this 를 비롯한 모든 DSB 가 사라진다
	// 경고 메시지를 띄우기 위해 m_pBoard 를 보관하자
	CBoardWrap* pBoard = m_pBoard;

	// 새로운 MFSM 을 생성한다
	CMFSM* pMFSM = new CMFSM( Mo()->rule.Encode(), apPlayers );
	pMFSM->Init( 0 );

	m_pBoard->SetMFSM(pMFSM);

	// DLL 로드가 실패했을 때 경고 메시지를 준다
	if ( bFailed && !sFailedDLL.IsEmpty() ) {

		LPCTSTR asLine[8];
		asLine[0] = _T("다음의 AI DLL 을 읽는데 실패했습니다");
		asLine[1] = _T("");
		asLine[2] = sFailedDLL;
		asLine[3] = _T("");
		asLine[4] = _T("옵션 대화상자의 AI 탭에서 이 파일의");
		asLine[5] = _T("경로를 다시 확인하세요");
		asLine[6] = _T("");
		asLine[7] = _T("임시로 기본 AI 를 읽어들입니다");
		DMessageBox* pMB = new DMessageBox(pBoard);	// 위의 pBoard 를 사용
		if ( Mo()->bUseSound ) MessageBeep( MB_ICONEXCLAMATION );
		pMB->Create( true, 8, asLine );
	}

	return true;
}
