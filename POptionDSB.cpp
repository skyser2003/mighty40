// POptionDSB.cpp : implementation file
//

#include "stdafx.h"
#include "Mighty.h"
#include "POptionDSB.h"
#include "BmpMan.h"
#include "DSB.h"
#include "InfoBar.h"
#include "Board.h"
#include "BoardWrap.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// POptionDSB property page

IMPLEMENT_DYNCREATE(POptionDSB, CPropertyPage)

POptionDSB::POptionDSB() : CPropertyPage(POptionDSB::IDD)
{
	//{{AFX_DATA_INIT(POptionDSB)
	//}}AFX_DATA_INIT
	m_bDSBOpaque = Mo()->bDSBOpaque;
	m_nDSBShadeMode = Mo()->nDSBShadeMode;
	m_colDSBText = (COLORREF)Mo()->nDSBText;
	m_colDSBStrong1 = (COLORREF)Mo()->nDSBStrong1;
	m_colDSBStrong2 = (COLORREF)Mo()->nDSBStrong2;
	m_colDSBGray = (COLORREF)Mo()->nDSBGray;
}

POptionDSB::~POptionDSB()
{
}

void POptionDSB::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(POptionDSB)
	//}}AFX_DATA_MAP
	if ( pDX->m_bSaveAndValidate ) {
		Mo()->bDSBOpaque = m_bDSBOpaque;
		Mo()->nDSBShadeMode = m_nDSBShadeMode;
		Mo()->nDSBText = (int)m_colDSBText;
		Mo()->nDSBStrong1 = (int)m_colDSBStrong1;
		Mo()->nDSBStrong2 = (int)m_colDSBStrong2;
		Mo()->nDSBGray = (int)m_colDSBGray;
		DSB::SetDefaultColor( m_colDSBText, m_colDSBStrong1,
			m_colDSBStrong2, m_colDSBGray );
	}
}


BEGIN_MESSAGE_MAP(POptionDSB, CPropertyPage)
	//{{AFX_MSG_MAP(POptionDSB)
	ON_BN_CLICKED(IDC_DSBGRAYCOLSELECT, OnDsbgraycolselect)
	ON_BN_CLICKED(IDC_DSBOPAQUE, OnDsbopaque)
	ON_BN_CLICKED(IDC_DSBSHADEMODE, OnDsbshademode)
	ON_BN_CLICKED(IDC_DSBSHADEMODE1, OnDsbshademode1)
	ON_BN_CLICKED(IDC_DSBSHADEMODE2, OnDsbshademode2)
	ON_BN_CLICKED(IDC_DSBSTRONG1COLSELECT, OnDsbstrong1colselect)
	ON_BN_CLICKED(IDC_DSBSTRONG2COLSELECT, OnDsbstrong2colselect)
	ON_BN_CLICKED(IDC_DSBTEXTCOLSELECT, OnDsbtextcolselect)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void POptionDSB::SelColor( COLORREF& col )
{
	CColorDialog dlg( col, 0, this );
	if ( dlg.DoModal() == IDOK ) {
		col = dlg.GetColor();
		Invalidate();
	}
}


/////////////////////////////////////////////////////////////////////////////
// POptionDSB message handlers

void POptionDSB::OnDsbgraycolselect() 
{
	// TODO: Add your control notification handler code here
	SelColor( m_colDSBGray );
	Invalidate(FALSE);
}

void POptionDSB::OnDsbopaque() 
{
	// TODO: Add your control notification handler code here
	bool bChecked = ((CButton*)GetDlgItem(IDC_DSBOPAQUE))->GetCheck() == 1;
	m_bDSBOpaque = bChecked;
	Invalidate(FALSE);
}

void POptionDSB::OnDsbshademode() 
{
	// TODO: Add your control notification handler code here
	m_nDSBShadeMode = 0;
	Invalidate(FALSE);
}

void POptionDSB::OnDsbshademode1() 
{
	// TODO: Add your control notification handler code here
	m_nDSBShadeMode = 1;
	Invalidate(FALSE);
}

void POptionDSB::OnDsbshademode2() 
{
	// TODO: Add your control notification handler code here
	m_nDSBShadeMode = 2;
	Invalidate(FALSE);
}

void POptionDSB::OnDsbstrong1colselect() 
{
	// TODO: Add your control notification handler code here
	SelColor( m_colDSBStrong1 );
	Invalidate(FALSE);
}

void POptionDSB::OnDsbstrong2colselect() 
{
	// TODO: Add your control notification handler code here
	SelColor( m_colDSBStrong2 );
	Invalidate(FALSE);
}

void POptionDSB::OnDsbtextcolselect() 
{
	// TODO: Add your control notification handler code here
	SelColor( m_colDSBText );
	Invalidate(FALSE);
}

BOOL POptionDSB::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	// TODO: Add extra initialization here
	((CButton*)GetDlgItem(IDC_DSBSHADEMODE + m_nDSBShadeMode))
		->SetCheck(1);
	((CButton*)GetDlgItem(IDC_DSBOPAQUE))->SetCheck(m_bDSBOpaque?1:0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void POptionDSB::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	// 그릴 영역은 샘플 영역으로 한정 (클립한다)
	CRect rc; GetDlgItem(IDC_DSBSAMPLE)->GetWindowRect(&rc);
	ScreenToClient( &rc );
	dc.IntersectClipRect( &rc );

	// 그릴 폰트 얻기
	CFont* pFont = Mf()->GetView()->GetWrapper()->GetFont(-1);	// 고정
	LOGFONT lf; pFont->GetLogFont( &lf );
	int nUnit = abs( lf.lfHeight );	// 한 줄의 크기

	// 폰트 속성 설정과 선택
	dc.SetTextAlign( TA_LEFT|TA_TOP );
	dc.SelectObject( pFont );

	// 영역의 크기를 계산하기 위해 그릴 텍스트의 최대 크기를 구한다
	CSize szText = dc.GetTextExtent( _T("강조 문자열 1") );
	szText.cx += nUnit*3;	// 양쪽 여백

	// 음영이 그려질 영역 (8 줄의 텍스트가 들어가야 함
	CRect rcShade = rc;
	rcShade.DeflateRect( rc.Width()/8, rc.Height()/8 );
	rcShade.right = max( rcShade.right, rcShade.left + szText.cx );
	rcShade.bottom = max( rcShade.bottom, rcShade.top + nUnit*8 );

	// 바탕색 ( 녹색 ) 채우기
	dc.FillSolidRect( &rc, RGB(0,128,0) );

	// 흰 물체
	CRect rcObject( rc.right - rc.Width()/2,
		rc.top, rc.right, rc.top + rc.Height()*2/3 );
	CRect rcIS = rcShade & rcObject;
	dc.FillSolidRect( &rcObject, RGB(255,255,255) );

	// DSB
	if ( m_nDSBShadeMode == 0 ) {

		CBmpMan* pBM = Mf()->GetView()->GetWrapper()->GetBmpMan();
		ASSERT(pBM);
		if ( pBM ) pBM->DrawShade( &dc, &rcShade, false );
	}
	else if ( m_nDSBShadeMode == 1 ) {
		dc.FillSolidRect( &rcShade, RGB(64,64,64) );
	}
	else {
		dc.FillSolidRect( &rcShade, RGB(0,64,0) );
		dc.FillSolidRect( &rcIS, RGB(128,128,128) );
	}

	// 테두리
	dc.SelectStockObject( WHITE_PEN );
	dc.MoveTo( rcShade.left, rcShade.bottom );
	dc.LineTo( rcShade.left, rcShade.top );
	dc.LineTo( rcShade.right, rcShade.top );
	dc.SelectStockObject( BLACK_PEN );
	dc.LineTo( rcShade.right, rcShade.bottom );
	dc.LineTo( rcShade.left, rcShade.bottom );

	// 글자 쓰기

	if ( m_bDSBOpaque ) {
		dc.SetBkMode( OPAQUE );
		dc.SetBkColor( RGB(0,0,0) );
	}
	else dc.SetBkMode( TRANSPARENT );

	int x = rcShade.left + nUnit;
	int y = rcShade.top + nUnit;
	int d; COLORREF col;

	col = RGB(0,0,0);
	for ( d = 1; d >= 0; d-- ) {
		dc.SetTextColor( col );
		dc.TextOut( x+d, y+d, _T("표준 문자열") );
		col = m_colDSBText;
	}
	y += nUnit+nUnit/2;

	x += nUnit;
	col = RGB(0,0,0);
	for ( d = 1; d >= 0; d-- ) {
		dc.SetTextColor( col );
		dc.TextOut( x+d, y+d, _T("강조 문자열 1") );
		col = m_colDSBStrong1;
	}
	y += nUnit+nUnit/2;

	col = RGB(0,0,0);
	for ( d = 1; d >= 0; d-- ) {
		dc.SetTextColor( col );
		dc.TextOut( x+d, y+d, _T("강조 문자열 2") );
		col = m_colDSBStrong2;
	}
	y += nUnit+nUnit;

	dc.SetTextColor( m_colDSBGray );
	dc.TextOut( x, y, _T("흐린 문자열") );
}
