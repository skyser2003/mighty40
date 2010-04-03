// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Mighty.h"

#include "InfoBar.h"
#include "BmpMan.h"
#include "Board.h"
#include "MainFrm.h"

#include "Msg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static WPARAM get_size_flag( HWND hWnd )
{
	WINDOWPLACEMENT wp;
	::GetWindowPlacement( hWnd, &wp );
	if ( wp.showCmd == SW_SHOWMINIMIZED )
		return SIZE_MINIMIZED;
	else if ( wp.showCmd == SW_SHOWMAXIMIZED )
		return SIZE_MAXIMIZED;
	else return SIZE_RESTORED;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_MESSAGE( WM_CALLSOCKPROC, OnCallSockProc )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	m_nLockSizing = 0;
	m_nLockClosing = 0;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	if (!m_wndInfoBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	//m_wndInfoBar.SetProgress(30);
	//m_wndInfoBar.ShowChat();

	// 메인 아이콘을 마이티 아이콘으로 바꾼다
	HICON hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
	VERIFY( hIcon );
	SetIcon( hIcon, TRUE );
	SetIcon( 0, FALSE );

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);

	// 옵션에서 위치와 크기 정보를 얻는다
	cs.x = Mo()->rcLast.left;
	cs.y = Mo()->rcLast.top;
	cs.cx = Mo()->rcLast.Width();
	cs.cy = Mo()->rcLast.Height();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// forward focus to the view window
	// m_wndView.SetFocus();

	// forward focus to CInfoBar first !!
	if ( !m_wndInfoBar.SetFocusToChat() )
		m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnDestroy() 
{
	CFrameWnd::OnDestroy();
	
	// TODO: Add your message handler code here

	// 옵션에 마지막 위치를 저장
	WINDOWPLACEMENT wp;
	GetWindowPlacement( &wp );
	Mo()->rcLast = wp.rcNormalPosition;
}

void CMainFrame::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	// TODO: Add your message handler code here and/or call default
	// 최대화 했을 때 정 사각형의 화면을 만든다

	CFrameWnd::OnGetMinMaxInfo(lpMMI);

	CRect rc;
	SystemParametersInfo( SPI_GETWORKAREA, 0, &rc, 0 );
	rc.right = rc.left + rc.Height();

	lpMMI->ptMaxPosition = rc.TopLeft();
	lpMMI->ptMaxSize = (CPoint)rc.Size();
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	if ( !m_nLockSizing )
		CFrameWnd::OnSize(nType, cx, cy);

	// 아래쪽 정보 막대 역시 무효화 해야 한다
	m_wndInfoBar.Invalidate(FALSE);
}

void CMainFrame::OnClose() 
{
	// 닫기 잠금이 있는 경우 닫지 않는다
	if ( m_nLockClosing <= 0 )
		CFrameWnd::OnClose();
}

// 윈도우의 크기 변경을 잠그거나 푼다
void CMainFrame::LockSizing()
{
	m_cs.Lock();

	if ( !m_nLockSizing ) {
		CRect rc(0,0,0,0);
		if ( GetSafeHwnd() ) GetWindowRect( &rc );
		m_szLockBefore = rc.Size();
	}
	m_nLockSizing++;

	m_cs.Unlock();
}

void CMainFrame::UnlockSizing()
{
	m_cs.Lock();

	if ( !--m_nLockSizing && GetSafeHwnd() ) {
		CRect rc; GetWindowRect( &rc );
		if ( m_szLockBefore != rc.Size() )
			// 크기가 바뀌었다면 Update 한다
			SendMessage( WM_SIZE, get_size_flag(GetSafeHwnd()),
				MAKELONG( rc.Width(), rc.Height() ) );
	}

	m_cs.Unlock();
}


// CPlayerSocket 지원, 콜백 함수를 호출해준다
LRESULT CMainFrame::OnCallSockProc( WPARAM, LPARAM lParam )
{
	CMsg* pCallMsg = (CMsg*)lParam;
	AUTODELETE_MSG(pCallMsg);

	long type, toCall, pSock, pMsg, dwUser1, dwUser2;
	if ( !pCallMsg->PumpLong( type )
		|| type != CMsg::mmCallSockProc
		|| !pCallMsg->PumpLong( toCall )
		|| !pCallMsg->PumpLong( pSock )
		|| !pCallMsg->PumpLong( pMsg )
		|| !pCallMsg->PumpLong( dwUser1 )
		|| !pCallMsg->PumpLong( dwUser2 ) ) {
		ASSERT(0); return 0;
	}

	// 함수 호출
	class CPlayerSocket;
	((void (*)( CPlayerSocket* pSocket,
		CMsg* pMsg, DWORD dwUser1, DWORD dwUser2 ))toCall)
	( (CPlayerSocket*)pSock, (CMsg*)(LPVOID)pMsg,
		(DWORD)dwUser1, (DWORD)dwUser2 );

	return 0;
}
