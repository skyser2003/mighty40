// Board.cpp : implementation of the CBoard class
//

#include "stdafx.h"
#include "Mighty.h"
#include "BmpMan.h"
#include "DSB.h"
#include "DEtc.h"
#include "DElection.h"
#include "DStartUp.h"
#include "Play.h"
#include "Player.h"
#include "MFSM.h"
#include "Board.h"
#include "BoardWrap.h"
#include "InfoBar.h"
#include "DScoreBoard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CBoard

CBoard::CBoard()
{
	m_hArrow = 0;
	m_hHand = 0;

	m_szPic.cx = m_szPic.cy = 0;
	m_szCard.cx = m_szCard.cy = 0;

	m_pMFSM = 0;

	m_pWaitingSelectionEvent = 0;
	m_pWaitingSelectionCard = 0;

	m_nMouseOver = -1;
	for ( int i = 0; i < (LAST_TURN+1)*2; i++ )
		m_abSelected[i] = false;
	m_nDealMiss = -1;

	m_nTurnRect = -1;

	m_pWrap = 0;
	m_bDoNotSendMeAnyMoreMessage = false;

	m_hFlying = 0;
	m_hSuffling = 0;
	m_bFlyProcTerminated = false;
	m_bFlyProcTerminate = false;

	for ( int j = 0; j < MAX_PLAYERS; j++ ) {
		m_apSayDSB[j] = 0;
		m_apGoalDSB[j] = 0;
	}
	m_pMasterGoalDSB = 0;
	m_pCurrentElectionDSB = 0;

	m_bDragDSB = false;
	m_pDragDSB = 0;
	m_pntLastDSB.x = m_pntLastDSB.y = 0x80000000;
	m_pHLDSB = 0;

	// FlyProc 쓰레드를 띄운다
	VERIFY( AfxBeginThread( FlyProc, (LPVOID)this ) );
}

CBoard::~CBoard()
{
	CleanUp();
}

// dtor 에서 호출되며, 할당된 자원을 반환한다
void CBoard::CleanUp()
{
	// 혹시 대기하고 있을지도 모르는 이벤트들을 세트해 준다
	m_eFlyIt.SetEvent();
	if ( m_pWaitingSelectionEvent ) {
		m_pWaitingSelectionCard = 0;
		m_pWaitingSelectionEvent->SetEvent();
	}

	if ( m_pMFSM ) {

		HANDLE hMFSMThread = m_pMFSM->DuplicateThreadHandle();
		m_pMFSM->EventExit();

		if ( hMFSMThread ) {
			VERIFY( ::WaitForSingleObject( hMFSMThread, INFINITE )
					== WAIT_OBJECT_0 );
			VERIFY( ::CloseHandle( hMFSMThread ) );
		}
		m_pMFSM = 0;
	}

	// 쓰레드를 죽인다
	if ( !m_bFlyProcTerminated ) {
		m_bFlyProcTerminate = true;
		m_eFlyIt.SetEvent();
		while ( !m_bFlyProcTerminated ) Sleep(100);
	}

	// 모든 DSB 를 삭제한다
	ASSERT( m_lpDSB.IsEmpty() );
	while ( m_lpDSB.GetCount() > 0 )
		m_lpDSB.GetTail()->Destroy();

	// 날리다 만 카드 정보와 섞다 만 카드 정보를 제거한다
	if ( m_hFlying )
		FlyCard( m_hFlying );
	if ( m_hSuffling )
		SuffleCards( m_hSuffling );

	// 비트맵 객체를 삭제한다
	if ( m_pic.GetSafeHandle() )
		m_pic.DeleteObject();
	if ( m_board.GetSafeHandle() )
		m_board.DeleteObject();

	// 폰트 객체를 삭제한다
	if ( m_fntSmall.GetSafeHandle() )
		m_fntSmall.DeleteObject();
	if ( m_fntMiddle.GetSafeHandle() )
		m_fntMiddle.DeleteObject();
	if ( m_fntBig.GetSafeHandle() )
		m_fntBig.DeleteObject();
	if ( m_fntFixedSmall.GetSafeHandle() )
		m_fntFixedSmall.DeleteObject();
	if ( m_fntFixedMiddle.GetSafeHandle() )
		m_fntFixedMiddle.DeleteObject();
	if ( m_fntFixedBig.GetSafeHandle() )
		m_fntFixedBig.DeleteObject();

	// 랩퍼 삭제
	delete m_pWrap; m_pWrap = 0;
}

void CBoard::OnDestroy() 
{
	CWnd ::OnDestroy();

	// TODO: Add your message handler code here

	// * 주의 *

	// 이 부분은 멀티 쓰레드의 경쟁 조건 문제와 관련된다
	// Wrapper 는 m_bDoNotSendMeAnyMoreMessage (이하 bNoMore ) 가
	// 세트 되는 순간부터 Board 에 메시지를 보내지 않게 되므로,
	// 아래쪽에 있는 bNoMore 가 세트되는 순간 까지는 메시지가
	// 오게 된다 - 그러나 이 함수가 리턴되면 PostNcDestroy에 의해
	// Board 는 삭제 될 것이므로 bNoMore 가 세트되는 그 순간까지
	// 남아 있는 WM_BOARDWRAPPER 메시지가 보내지는 경우 문제가
	// 발생한다 - 따라서 PeekMessage 로 그 메시지들을 삭제하는
	// 방법을 사용하였다

#ifdef _DEBUG
	Sleep(2000);	// 사각지대를 넓혀서 에러가 발생할 확률을 높인다
#endif

	// 모든 DSB 를 삭제한다
	while ( m_lpDSB.GetCount() > 0 )
		m_lpDSB.GetTail()->Destroy();

	m_bDoNotSendMeAnyMoreMessage = true;
	m_bFlyProcTerminate = true;

	// 남은 WM_BOARDWRAPPER 요청을 모두 없앤다
	MSG msg;
	while ( ::PeekMessage( &msg, GetSafeHwnd(),
		WM_BOARDWRAPPER, WM_BOARDWRAPPER, PM_REMOVE ) ) {
		::TranslateMessage( &msg );
		::DispatchMessage( &msg );
	}
}

BEGIN_MESSAGE_MAP(CBoard,CWnd )
	//{{AFX_MSG_MAP(CBoard)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_TIMER()
	ON_WM_LBUTTONUP()
	ON_WM_DESTROY()
	ON_WM_CHAR()
	ON_UPDATE_COMMAND_UI(ID_GAME_EXIT, OnUpdateGameExit)
	ON_COMMAND(ID_GAME_EXIT, OnGameExit)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_BOARDWRAPPER, OnWrapper)
	ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnToolTipNeedText )
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBoard message handlers

BOOL CBoard::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.style |= WS_CLIPCHILDREN;
	cs.lpszClass = AfxRegisterWndClass(
		CS_HREDRAW|CS_VREDRAW, 
		NULL, NULL, NULL);

	return TRUE;
}

void CBoard::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect rcClip; dc.GetClipBox( &rcClip );

	CDC dcMem; dcMem.CreateCompatibleDC( &dc );
	dcMem.SelectObject( &m_pic );

	// 버퍼에서 복사해 온다
	dc.BitBlt(
		rcClip.left, rcClip.top,
		rcClip.Width(), rcClip.Height(),
		&dcMem, rcClip.left, rcClip.top,
		SRCCOPY );
}

int CBoard::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd ::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO: Add your specialized creation code here

	// 랩퍼 생성
	m_pWrap = new CBoardWrap(this);

	CClientDC dc(this);

	// 커서 읽기
	if ( !m_hArrow ) m_hArrow = AfxGetApp()->LoadStandardCursor( IDC_ARROW );
	if ( !m_hHand ) m_hHand = AfxGetApp()->LoadCursor( IDC_HAND );

	// 폰트 생성
	CreateFont();

	// 모든 카드 그림 읽기
	VERIFY( m_bm.LoadAllCards( &dc, &m_fntSmall ) );

	// 뒷면 그림 읽기
	if ( Mo()->nBackPicture != -1 )
		VERIFY( m_bm.LoadBackPicture( &dc, Mo()->nBackPicture ) );
	else {
		if ( !m_bm.LoadBackPicture( &dc, -1, Mo()->sBackPicturePath ) ) {
			AfxMessageBox( _T("뒷면 그림 파일을 읽을 수 없습니다\n옵션에서 확인해 보세요"),
				MB_OK|MB_ICONEXCLAMATION );
			Mo()->nBackPicture = 0;
			VERIFY( m_bm.LoadBackPicture( &dc, Mo()->nBackPicture ) );
		}
	}

	// 배경 그림 읽기
	if ( Mo()->bUseBackground ) {
		if ( !m_bm.LoadBackground( &dc, Mo()->sBackgroundPath ) ) {
			AfxMessageBox( _T("배경 그림 파일을 읽을 수 없습니다\n옵션에서 확인해 보세요"),
				MB_OK|MB_ICONEXCLAMATION );
			Mo()->bUseBackground = false;
		}
	}

	// 툴팁 가능하게
	VERIFY( EnableToolTips() );

	return 0;
}

void CBoard::OnSize(UINT nType, int cx, int cy) 
{
	CWnd ::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here

	// 적당한 카드 크기를 미리 계산해서 가지고 있는다
	m_szCard = GetCardZoom( Mo()->bZoomCard );

	// 폰트를 다시 만든다
	CreateFont();

	// 원래 있던 버퍼가 충분히 크다면 새로 만들 필요가 없다
	if ( !m_pic.GetSafeHandle() ||
		 !m_board.GetSafeHandle() ||
			m_szPic.cx < cx || m_szPic.cy < cy ) {

		CClientDC dc(this);

		if ( m_pic.GetSafeHandle() )
			m_pic.DeleteObject();
		m_pic.CreateCompatibleBitmap( &dc, cx, cy );

		if ( m_board.GetSafeHandle() )
			m_board.DeleteObject();
		m_board.CreateCompatibleBitmap( &dc, cx, cy );

		m_szPic.cx = cx; m_szPic.cy = cy;
	}

	// 모든 DSB 의 OnMove 를 호출한다
	POSITION pos = m_lpDSB.GetHeadPosition();
	while (pos) m_lpDSB.GetNext(pos)->OnMove();

	// 그림을 그린다
	UpdatePlayer( -2 );
}

void CBoard::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	// DSB 를 검사한다
	if ( CookDSBForLButtonDown( point ) ) ;	// 다른 처리를 건너뜀

	// 카드를 선택중이다
	else if ( m_pWaitingSelectionEvent
		&& CookSelectionForLButtonDown( point ) ) ;

	else ;

	CWnd ::OnLButtonDown(nFlags, point);
}

void CBoard::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	// DSB 드래그 중이면 하던 드래그를 계속한다
	if ( m_bDragDSB ) {

		CRect  rc; m_pDragDSB->GetRect( &rc );

		// DSB 를 드래그
		m_pDragDSB->Offset( point - m_pntLastDSB );

		// 이전 사각형과 새로운 사각형의 합집합을 invalidate
		CRect rcUpdate = rc;
		m_pDragDSB->GetRect( &rc );
		rcUpdate.SetRect(
			min( rcUpdate.left, rc.left ), min( rcUpdate.top, rc.top ),
			max( rcUpdate.right, rc.right ), max( rcUpdate.bottom, rc.bottom ) );
		UpdateDSB( &rcUpdate, true );
		m_pntLastDSB = point;
	}

	// 카드를 선택하기 위해 대기하는 경우
	// 마우스 아래쪽에 있는 카드를 조금씩 올려준다
	// (모달 상태가 아닌 경우에만)
	else if ( m_lpModalDSB.IsEmpty() && CanSelect() )
		ShiftCard( point.x, point.y, true );

	// 카드가 왠지 올라가 있는 경우 내려준다
	else if ( m_pMFSM && m_nMouseOver != -1 )
		ShiftCard( 0x80000000, 0x80000000, true );

	CWnd ::OnMouseMove(nFlags, point);
}

void CBoard::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	if ( m_bDragDSB ) {	// DSB 를 드래그중
		m_bDragDSB = false;
		m_pDragDSB = 0;
		ReleaseCapture();
		ClipCursor( 0 );
	}

	CWnd ::OnLButtonUp(nFlags, point);
}

BOOL CBoard::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/) 
{
	// TODO: Add your message handler code here and/or call default

	// DSB 를 검사해서 핫 스팟 위에 커서가 있다면
	// 커서를 손 모양으로 바꾼다

	POINT point;	// 현재 위치
	GetCursorPos( &point );
	ScreenToClient( &point );

	POSITION pos = m_lpDSB.GetHeadPosition();
	for (; pos; m_lpDSB.GetNext(pos) ) {
		DSB* pDSB = m_lpDSB.GetAt(pos);

		// 모달 상태이면 모달 DSB 만 검사한다
		if ( !m_lpModalDSB.IsEmpty()
			&& pDSB != m_lpModalDSB.GetHead() ) continue;

		CRect rc; pDSB->GetRect( &rc );
		if ( rc.PtInRect( point ) ) {
			// pDSB 위에 커서가 존재
			if ( pDSB->DispatchHotspot( point.x, point.y, true) ) {
				// 핫 스팟 위에 존재
				if ( m_pHLDSB && m_pHLDSB != pDSB )
					// 하일라이트 중이었던 DSB 를 노멀 상태로
					m_pHLDSB->DispatchHotspot( 0x80000000, 0x80000000, true );
				SetCursor( m_hHand );
				m_pHLDSB = pDSB;
				return TRUE;
			}
			break;
		}
	}

	if ( m_pHLDSB )	// 하일라이트 중이었던 DSB 를 노멀 상태로
		// 설마 이 좌표에 핫 스팟이 있진 않겠지?
		m_pHLDSB->DispatchHotspot( 0x80000000, 0x80000000, true );
	if ( pos )
		m_pHLDSB = m_lpDSB.GetAt(pos);
	else m_pHLDSB = 0;

	// 모달 상태가 아니고 카드를 선택 중
	if ( m_lpModalDSB.IsEmpty() && CanSelect() )
		SetCursor( m_hHand );
	// 그 외의 경우
	else SetCursor( m_hArrow );

	return TRUE;
}

void CBoard::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default

	if ( nIDEvent >= (int)tiDSB ) {
		// DSB 시효 만료

		// 해당 DSB 를 찾는다
		DSB* pDSB = 0;
		POSITION pos = m_lpDSB.GetHeadPosition();
		while (pos) {
			DSB* p = m_lpDSB.GetNext(pos);
			if ( p->GetID() + tiDSB == (int)nIDEvent ) {
				pDSB = p;
				break;
			}
		}
		ASSERT( pDSB );

		// 뒷 처리 (타이머를 죽이고 DSB를 죽임)
		VERIFY( KillTimer( nIDEvent ) );
		if ( pDSB && !pDSB->IsPermanent() )
			pDSB->Destroy();
	}

	CWnd ::OnTimer(nIDEvent);
}

void CBoard::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	if ( nChar == (UINT)'\x001b' ) {
		// ESC 를 눌렀다
		if ( !m_lpModalDSB.IsEmpty() )
			m_lpModalDSB.GetHead()->OnESC();
		if ( !m_lpDSB.IsEmpty() )
			m_lpDSB.GetHead()->OnESC();
	}
	else if ( nChar == (UINT)'\x000d' ) {
		// Enter 를 눌렀다
		if ( !m_lpModalDSB.IsEmpty() )
			m_lpModalDSB.GetHead()->OnEnter();
		if ( !m_lpDSB.IsEmpty() )
			m_lpDSB.GetHead()->OnEnter();
	}
	CWnd ::OnChar(nChar, nRepCnt, nFlags);
}


// 현재 Mighty FSM 을 세트한다
// 이것을 세트 하는 순간 정상적인 윈도우의 동작이 시작된다
void CBoard::SetMFSM( CMFSM* pMFSM )
{
	m_nTurnRect = -1;	// 차례 사각형, 삭제한다

	// 현재 떠 있는 모든 DSB 들을 삭제
	POSITION pos = m_lpDSB.GetHeadPosition();
	while (pos) m_lpDSB.GetNext(pos)->Destroy();
	ASSERT( m_lpDSB.IsEmpty() );

	// 선택 대기 상태도 종료한다
	if ( m_pWaitingSelectionEvent )
		CancelSelection();

	m_pMFSM = pMFSM;

	if ( !m_pMFSM ) {
		// MFSM 이 사라진 결과면 시작 DSB 를 띄운다
		// 초기 DSB
		DStartUp* pDSB = new DStartUp(GetWrapper());
		pDSB->Create( 0 );
		Sb()->Update();
	}

	UpdatePlayer( -2, 0 );
}

void CBoard::OnUpdateGameExit(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_pMFSM ? TRUE : FALSE );
}

void CBoard::OnGameExit() 
{
	if ( m_pMFSM ) {
		m_pMFSM->EventExit();
		m_pMFSM = 0;
	}
}
