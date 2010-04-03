// POptionBackground.cpp : implementation file
//

#include "stdafx.h"
#include "Mighty.h"
#include "POptionBackground.h"

#include "BmpMan.h"
#include "InfoBar.h"
#include "Board.h"
#include "BoardWrap.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int get_back_align( bool bTile, bool bExpand )
{
	return bTile ? 1 : bExpand ? 2 : 0;
}
static void parse_back_align( bool& bTile, bool& bExpand, int nMode )
{
	bTile = nMode == 1;
	bExpand = nMode == 2;
}

/////////////////////////////////////////////////////////////////////////////
// POptionBackground property page

IMPLEMENT_DYNCREATE(POptionBackground, CPropertyPage)

POptionBackground::POptionBackground() : CPropertyPage(POptionBackground::IDD)
{
	//{{AFX_DATA_INIT(POptionBackground)
	//}}AFX_DATA_INIT
	m_nBackAlign = get_back_align( Mo()->bTile, Mo()->bExpand );
	m_bUseBackground = Mo()->bUseBackground;
	m_colDef = (COLORREF)Mo()->nDefColor;
	m_colAtt = (COLORREF)Mo()->nAttColor;
	m_colBack = (COLORREF)Mo()->nBackColor;
	m_sBackgroundPath = Mo()->sBackgroundPath;

	m_pBm = Mf()->GetView()->GetWrapper()->GetBmpMan();
	ASSERT(m_pBm);
}

POptionBackground::~POptionBackground()
{
}

void POptionBackground::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(POptionBackground)
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate ) {
		parse_back_align( Mo()->bTile, Mo()->bExpand, m_nBackAlign );
		Mo()->bUseBackground = m_bUseBackground;
		Mo()->nDefColor = (int)m_colDef;
		Mo()->nAttColor = (int)m_colAtt;
		Mo()->nBackColor = (int)m_colBack;
		Mo()->sBackgroundPath = m_sBackgroundPath;
	}
}

void POptionBackground::SelColor( COLORREF& col )
{
	CColorDialog dlg( col, 0, this );
	if ( dlg.DoModal() == IDOK ) {
		col = dlg.GetColor();
		Invalidate();
	}
}

bool POptionBackground::LoadPic( LPCTSTR sPath )
{
	CClientDC dc( *Mf()->GetView()->GetWrapper() );

	if ( !m_pBm->LoadBackground( &dc, sPath ) ) {

		CString sMsg;
		if ( !sPath || !*sPath )
			sMsg.Format( _T("선택된 그림 파일이 없습니다") );
		else
			sMsg.Format( _T("그림 파일 %s 을(를) 읽을 수 없습니다"), sPath );

		AfxMessageBox( sMsg, MB_OK|MB_ICONEXCLAMATION );
		return false;
	}
	else {
		Invalidate();
		return true;
	}
}

bool POptionBackground::Browse() 
{
	// TODO: Add your control notification handler code here

	CFileDialog dlg( TRUE,	// open
		_T(".bmp"), NULL,
		OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NONETWORKBUTTON,
		_T("비트맵 파일 (*.bmp)|*.bmp|모든파일 (*.*)|*.*||"),
		this );

	if ( dlg.DoModal() == IDOK ) {

		if ( LoadPic( dlg.GetPathName() ) ) {
			m_sBackgroundPath = dlg.GetPathName();
			return true;
		}
	}
	return false;
}

BEGIN_MESSAGE_MAP(POptionBackground, CPropertyPage)
	//{{AFX_MSG_MAP(POptionBackground)
	ON_BN_CLICKED(IDC_USEBACKGROUND, OnUsebackground)
	ON_BN_CLICKED(IDC_ATTCOLSELECT, OnAttcolselect)
	ON_BN_CLICKED(IDC_BACKALIGN, OnBackalign)
	ON_BN_CLICKED(IDC_BACKALIGN1, OnBackalign1)
	ON_BN_CLICKED(IDC_BACKALIGN2, OnBackalign2)
	ON_BN_CLICKED(IDC_BACKGROUNDBROWSE, OnBackgroundbrowse)
	ON_BN_CLICKED(IDC_BACKGROUNDCOLSELECT, OnBackgroundcolselect)
	ON_BN_CLICKED(IDC_DEFCOLSELECT, OnDefcolselect)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// POptionBackground message handlers

void POptionBackground::OnUsebackground() 
{
	// TODO: Add your control notification handler code here

	// 체크박스의 상태에 따라서 나머지 컨트롤들을 Enable/Disable 한다
	bool bChecked = ((CButton*)GetDlgItem(IDC_USEBACKGROUND))->GetState() & 0x0003 ? true : false;

	m_bUseBackground = bChecked;

	// 체크박스가 선택되었으므로 그림을 새로 로드한다
	if ( bChecked && !LoadPic( m_sBackgroundPath ) )
		if ( !Browse() ) {
			m_bUseBackground = false;
			((CButton*)GetDlgItem(IDC_USEBACKGROUND))->SetCheck(0);
		}

	GetDlgItem(IDC_BACKALIGN)->EnableWindow(m_bUseBackground);
	GetDlgItem(IDC_BACKALIGN1)->EnableWindow(m_bUseBackground);
	GetDlgItem(IDC_BACKALIGN2)->EnableWindow(m_bUseBackground);
	GetDlgItem(IDC_BACKGROUNDBROWSE)->EnableWindow(m_bUseBackground);

	Invalidate();
}

void POptionBackground::OnAttcolselect() 
{
	// TODO: Add your control notification handler code here
	SelColor( m_colAtt );
}

void POptionBackground::OnBackalign() 
{
	// TODO: Add your control notification handler code here
	m_nBackAlign = 0;
	Invalidate();
}

void POptionBackground::OnBackalign1() 
{
	// TODO: Add your control notification handler code here
	m_nBackAlign = 1;
	Invalidate();
}

void POptionBackground::OnBackalign2() 
{
	// TODO: Add your control notification handler code here
	m_nBackAlign = 2;
	Invalidate();
}

void POptionBackground::OnBackgroundbrowse() 
{
	// TODO: Add your control notification handler code here
	Browse();
}

void POptionBackground::OnBackgroundcolselect() 
{
	// TODO: Add your control notification handler code here
	SelColor( m_colBack );
}

void POptionBackground::OnDefcolselect() 
{
	// TODO: Add your control notification handler code here
	SelColor( m_colDef );
}
void POptionBackground::OnCancel() 
{
	// TODO: Add your specialized code here and/or call the base class
	// 원상 복구
	if ( m_pBm ) {
		CClientDC dc( *Mf()->GetView()->GetWrapper() );
		m_pBm->LoadBackground( &dc, Mo()->sBackgroundPath );
	}
	CPropertyPage::OnCancel();
}

BOOL POptionBackground::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	// 체크박스
	((CButton*)GetDlgItem(IDC_USEBACKGROUND))
		->SetCheck(m_bUseBackground ? 1 : 0);
	// 라디오 버튼 설정
	((CButton*)GetDlgItem(IDC_BACKALIGN+m_nBackAlign))
		->SetCheck(1);
	// 라디오 버튼 Enable/Disable
	OnUsebackground();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void POptionBackground::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	if ( !m_pBm ) return;

	CRect rc; GetDlgItem(IDC_SAMPLEBACKGROUND)->GetWindowRect( &rc );
	ScreenToClient( &rc );

	bool bTile, bExpand;
	parse_back_align( bTile, bExpand, m_nBackAlign );

	dc.SetViewportOrg( rc.TopLeft() );
	rc.OffsetRect( -rc.TopLeft() );
	m_pBm->DrawBackground( m_bUseBackground, bTile, bExpand,
		(COLORREF)m_colBack, &dc, &rc, rc.Width(), rc.Height() );
	dc.SetViewportOrg( 0, 0 );

	// 여당, 야당 텍스트와 화살표
	CRect rcText, rcCtrl;
	dc.SetBkMode( TRANSPARENT );
	CFont* pfnOld = dc.SelectObject( Mf()->GetView()->GetWrapper()->GetFont(-1) );

	Link( &dc, IDC_DEFCOLSELECT,
		DrawText( &dc,
			rc.left + rc.Width()/4, rc.bottom - rc.Height()/6,
			m_colDef, CString(_T("여당")) ) );

	Link( &dc, IDC_ATTCOLSELECT,
		DrawText( &dc,
			rc.left + rc.Width()*3/4, rc.bottom - rc.Height()/6,
			m_colAtt, CString(_T("야당")) ) );

	CRect rcBackButton; 
	GetDlgItem( IDC_BACKGROUNDCOLSELECT )->GetWindowRect( &rcBackButton );
	ScreenToClient( &rcBackButton );
	Link( &dc, CPoint( rc.right - rc.Width()/5, rc.top + rc.Height()/6 ),
		CPoint( rcBackButton.left, rcBackButton.top+rcBackButton.Height()/2 ) );

	dc.SelectObject( pfnOld );
}

RECT POptionBackground::DrawText( CDC* pDC,
	int x, int y, COLORREF col,
	const CString& sText )
{
	CRect rc( x, y, x, y );
	// calc
	pDC->DrawText( sText, &rc, DT_CALCRECT|DT_LEFT|DT_NOCLIP|DT_SINGLELINE );
	// shade
	pDC->SetTextColor( RGB(0,0,0) );
	pDC->DrawText( sText, rc+CPoint(1,0), DT_CENTER|DT_SINGLELINE|DT_VCENTER|DT_NOCLIP );
	pDC->DrawText( sText, rc+CPoint(-1,0), DT_CENTER|DT_SINGLELINE|DT_VCENTER|DT_NOCLIP );
	pDC->DrawText( sText, rc+CPoint(0,1), DT_CENTER|DT_SINGLELINE|DT_VCENTER|DT_NOCLIP );
	pDC->DrawText( sText, rc+CPoint(0,-1), DT_CENTER|DT_SINGLELINE|DT_VCENTER|DT_NOCLIP );
	// text
	pDC->SetTextColor( col );
	pDC->DrawText( sText, &rc, DT_CENTER|DT_SINGLELINE|DT_VCENTER|DT_NOCLIP );

	return rc;
}

void POptionBackground::Link( CDC* pDC, UINT idCtrl, const CRect& rc )
{
	CRect rcCtrl;
	GetDlgItem( idCtrl )->GetWindowRect( &rcCtrl );
	ScreenToClient( &rcCtrl );

	Link( pDC, CPoint( rcCtrl.left+rcCtrl.Width()/2, rcCtrl.top ),
		CPoint( rc.left + rc.Width()/2, rc.bottom ) );
}

void POptionBackground::Link( CDC* pDC, CPoint pnt1, CPoint pnt2 )
{
	int nOldR2 = pDC->SetROP2( R2_NOT );
	pDC->MoveTo( pnt1 );
	pDC->LineTo( pnt2 );
	pDC->SetROP2( nOldR2 );
}
