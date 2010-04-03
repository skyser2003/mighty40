// POptionCard.cpp : implementation file
//

#include "stdafx.h"
#include "Mighty.h"
#include "POptionCard.h"
#include "DBackSelect.h"

#include "InfoBar.h"
#include "BmpMan.h"
#include "Board.h"
#include "BoardWrap.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// POptionCard property page

IMPLEMENT_DYNCREATE(POptionCard, CPropertyPage)

POptionCard::POptionCard() : CPropertyPage(POptionCard::IDD)
{
	m_pBm = Mf()->GetView()->GetWrapper()->GetBmpMan();
	ASSERT( m_pBm );

	//{{AFX_DATA_INIT(POptionCard)
	m_bCardTip = Mo()->bCardTip;
	m_bCardHelp = Mo()->bCardHelp;
	m_bZoomCard = Mo()->bZoomCard;
	//}}AFX_DATA_INIT

	m_nBackPicture = Mo()->nBackPicture;
	m_sBackPicturePath = Mo()->sBackPicturePath;
}

POptionCard::~POptionCard()
{
}

void POptionCard::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(POptionCard)
	DDX_Control(pDX, IDC_SAMPLEBACK, m_cSampleBack);
	DDX_Check(pDX, IDC_CARDTIP, m_bCardTip);
	DDX_Check(pDX, IDC_CARDHELP, m_bCardHelp);
	DDX_Check(pDX, IDC_ZOOMCARD, m_bZoomCard);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate ) {
		Mo()->bCardTip = !!m_bCardTip;
		Mo()->bCardHelp = !!m_bCardHelp;
		Mo()->bZoomCard = !!m_bZoomCard;
		Mo()->nBackPicture = m_nBackPicture;
		Mo()->sBackPicturePath = m_sBackPicturePath;
	}
}


BEGIN_MESSAGE_MAP(POptionCard, CPropertyPage)
	//{{AFX_MSG_MAP(POptionCard)
	ON_BN_CLICKED(IDC_BACKSELECT, OnBackselect)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// POptionCard message handlers

void POptionCard::DrawSample( CDC* pDC )
{
	if ( !m_pBm ) return;

	// 보여주고 있는 카드 뒷면 그림 바꾸기
	CRect rc;
	GetDlgItem(IDC_SAMPLEBACK)->GetWindowRect( &rc );
	ScreenToClient( &rc );

	CSize sz = ::GetBitmapSize( IDB_SA );
	CPoint pt( ( rc.left + rc.right ) / 2, ( rc.top + rc.bottom ) / 2 );
	rc.left = pt.x - sz.cx/2;
	rc.right = rc.left + sz.cx;
	rc.top = pt.y - sz.cy/2;
	rc.bottom = rc.top + sz.cy;
	m_pBm->DrawCard( pDC, 0, rc );
}

void POptionCard::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	DrawSample( &dc );

	// Quote 를 그린다
	dc.SetTextAlign( TA_LEFT | TA_TOP );
	dc.SetBkMode( TRANSPARENT );
	dc.SetTextColor( GetSysColor( COLOR_BTNTEXT ) );
	dc.SelectStockObject( DEFAULT_GUI_FONT );

	// 굵은 폰트
	LOGFONT lf;
	dc.GetCurrentFont()->GetLogFont( &lf );
	lf.lfWeight = FW_BOLD;
	CFont font;
	font.CreateFontIndirect( &lf );

	// 사각형
	CWnd* pWnd = GetDlgItem( IDC_QUOTE );
	CRect rc;
	pWnd->GetWindowRect( &rc );
	ScreenToClient( &rc );
	int left = rc.left, x = rc.left, y = rc.top;

	// 스트링
	CString s;

	// 머크쇼, 자네가 설마 이 티볼트한테
	// 상대가 될 수 있으리라고 생각하나 ?
	//       『로미오와 줄리엣 3막 1장』

	s = _T("머크쇼, 자네가 설");
	dc.TextOut( x, y, s );
	CSize sz = dc.GetTextExtent( s );
	x += sz.cx;

	CFont* pfntOld = dc.SelectObject( &font );
	s = _T("마 이 티");
	dc.TextOut( x, y, s );
	x += dc.GetTextExtent( s ).cx;
	dc.SelectObject( pfntOld );

	s = _T("볼트한테");
	dc.TextOut( x, y, s );
	x = left; y += sz.cy + abs(lf.lfHeight)/4;

	s = _T("상대가 될 수 있으리라고 생각하나 ?");
	dc.TextOut( x, y, s );
	y += sz.cy + abs(lf.lfHeight)/4;

	s = _T("       『로미오와 줄리엣 3막 1장』");
	dc.TextOut( x, y, s );
}

void POptionCard::OnBackselect() 
{
	// TODO: Add your control notification handler code here
	ASSERT( m_pBm );
	if ( !m_pBm ) return;

	DBackSelect dlg;
	if ( dlg.DoModal() == IDOK ) {

		CClientDC dc(*Mf()->GetView()->GetWrapper());

		if ( !m_pBm->LoadBackPicture( &dc,
			dlg.m_nBackPicture, dlg.m_sBackPicturePath ) ) {

			CString sMsg;
			if ( dlg.m_sBackPicturePath.IsEmpty() )
				sMsg.Format( _T("선택된 그림 파일이 없습니다") );
			else
				sMsg.Format( _T("그림 파일 %s 을(를) 읽을 수 없습니다"),
					dlg.m_sBackPicturePath );
			AfxMessageBox( sMsg, MB_OK|MB_ICONEXCLAMATION );
		}
		else {
			m_nBackPicture = dlg.m_nBackPicture;
			m_sBackPicturePath = dlg.m_sBackPicturePath;
			Invalidate();
		}
	}
}

void POptionCard::OnCancel() 
{
	// TODO: Add your specialized code here and/or call the base class

	// 취소하면 다시 뒷면 그림을 원래대로
	CClientDC dc(*Mf()->GetView()->GetWrapper());
	VERIFY( m_pBm->LoadBackPicture( &dc,
		Mo()->nBackPicture, Mo()->sBackPicturePath ) );

	CPropertyPage::OnCancel();
}
