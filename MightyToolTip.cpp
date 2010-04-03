// MightyToolTip.cpp : implementation file
//

#include "stdafx.h"
#include "Mighty.h"
#include "MightyToolTip.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CMightyToolTip

CFont CMightyToolTip::s_font;
CString CMightyToolTip::s_sWndClass;

CMightyToolTip::CMightyToolTip()
{
}

CMightyToolTip::~CMightyToolTip()
{
}


BEGIN_MESSAGE_MAP(CMightyToolTip, CWnd)
	//{{AFX_MSG_MAP(CMightyToolTip)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMightyToolTip message handlers

void CMightyToolTip::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here

	CRect rc; GetWindowRect( &rc );
	ScreenToClient( &rc );

	rc.DeflateRect( TOOLTIP_BORDER_MARGIN, TOOLTIP_BORDER_MARGIN );

	CFont* pfnOld = dc.SelectObject( &s_font );
	dc.SetTextColor( GetSysColor( COLOR_INFOTEXT ) );
	dc.SetBkMode( TRANSPARENT );

	dc.DrawText( m_sText, &rc, DT_LEFT | DT_NOCLIP
						| DT_NOPREFIX | DT_WORDBREAK );

	dc.SelectObject( pfnOld );

	// Do not call CWnd::OnPaint() for painting messages
}

void CMightyToolTip::CalcFittingRect( LPRECT prc )
{
	// 필요한 크기를 계산하고 그 크기로 맞춘다

	CClientDC dc(this);
	CFont* pfnOld = dc.SelectObject( &s_font );
	dc.DrawText( m_sText, prc, DT_CALCRECT | DT_LEFT | DT_NOCLIP
						| DT_NOPREFIX | DT_WORDBREAK );
	dc.SelectObject( pfnOld );
}

// 주어진 스트링으로 팁을 출력한다
// 윈도우가 생성되지 않았으면 새로 생성한다
void CMightyToolTip::Tip( int x, int y, LPCTSTR s, int cx )
{
	m_sText = s;

	if ( !GetSafeHwnd() ) {	// 윈도우가 생성되지 않은 상태

		// 윈도우 클래스를 등록한다
		if ( s_sWndClass.IsEmpty() )
			s_sWndClass = AfxRegisterWndClass(
				0, AfxGetApp()->LoadStandardCursor(IDC_ARROW),
				GetSysColorBrush( COLOR_INFOBK ), 0 );

		// 폰트를 읽어 들인다
		if ( !s_font.GetSafeHandle() ) {
			LOGFONT lf;
			::GetObject( (HGDIOBJ)::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf );
			s_font.CreateFontIndirect( &lf );
		}

		BOOL bRet = CreateEx(
			0, s_sWndClass, 0, WS_POPUP | WS_BORDER,
			x, y, cx, 0, Mw()->GetSafeHwnd(), 0, 0 );
		if ( !bRet ) return;
	}
	else ShowWindow( SW_HIDE );

	// 크기를 다시 세트한다

	CRect rc( x, y, x+cx, y );

	CalcFittingRect( &rc );
	rc.InflateRect( TOOLTIP_BORDER_MARGIN, TOOLTIP_BORDER_MARGIN );

	CSize sz = rc.Size();

	CSize szScreen( GetSystemMetrics( SM_CXSCREEN ), GetSystemMetrics( SM_CYSCREEN ) );
	rc.left = max( 0, min( szScreen.cx - sz.cx, rc.left ) );
	rc.top = max( 0, min( szScreen.cy - sz.cy, rc.top ) );
	rc.right = rc.left + sz.cx;
	rc.bottom = rc.top + sz.cy;

	MoveWindow( &rc );
	ShowWindow( SW_SHOW );
}
