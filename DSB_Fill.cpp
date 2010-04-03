// DSB_Fill.cpp: implementation of the DSB Filling function
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"
#include "BmpMan.h"
#include "DSB.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


static CImageList s_imgl;
static CSize s_szShade;
static bool s_bBlendingFailed = false;
static bool s_bImageListCreated = false;

static bool CreateBlendImageList()
{
	// 한번 실패는 영원한 실패
	if ( s_bBlendingFailed ) return false;

	s_szShade = ::GetBitmapSize( IDB_SHADE );

	if ( !s_imgl.Create( s_szShade.cx, s_szShade.cy,
						ILC_COLOR16 | ILC_MASK, 0, 1 ) ) {
		s_bBlendingFailed = true;
		return false;
	}

	CBitmap bmShade;
	bmShade.LoadBitmap( IDB_SHADE );

	// 빨간색이 마스크 색상
	s_imgl.Add( &bmShade, RGB(255,0,0) );

	VERIFY( bmShade.DeleteObject() );

	s_bImageListCreated = true;
	return true;
}

static bool DsbBlend( CDC* pDC, const CRect& rc )
{
	if ( s_bBlendingFailed ) return false;

	// 256 색 이하면 실패한다
	if ( pDC->GetDeviceCaps( NUMCOLORS ) != -1 ) {
		s_bBlendingFailed = true;
		return false;
	}

	if ( !s_bImageListCreated )
		if ( !CreateBlendImageList() )
			return false;

	// 클립 영역을 얻는다
	CRect rcClip;
	pDC->GetClipBox( &rcClip );
	rcClip &= rc;

	// rc.left, rc.top 부터 s_szShade 의 배수가 되도록
	// rcClip.left, rcClip.top 을 변경한다
	rcClip.left = ( rcClip.left - rc.left )
				/ s_szShade.cx * s_szShade.cx + rc.left;
	rcClip.top = ( rcClip.top - rc.top )
				/ s_szShade.cy * s_szShade.cy + rc.top;

	// 타일 형태로 쉐이드를 채운다
	int nYRem = rcClip.Height();
	for ( int y = rcClip.top; nYRem > 0; y += s_szShade.cy, nYRem -= s_szShade.cy ) {
		int cy = min( s_szShade.cy, nYRem );
		int nXRem = rcClip.Width();
		for ( int x = rcClip.left; nXRem > 0; x += s_szShade.cx, nXRem -= s_szShade.cx ) {
			int cx = min( s_szShade.cx, nXRem );
			ImageList_DrawEx(
				s_imgl.GetSafeHandle(), 0, pDC->GetSafeHdc(),
				x, y, cx, cy, CLR_NONE, CLR_NONE,
				ILD_BLEND50 );
		}
	}

	return true;
}

// 배경을 그린다
// nDSBShadeMode : 칠하기 모드 ( 1 : 불투명 2 : 블렌드  그외 : 병치 )
void DSB::FillBack( CDC* pDC, LPCRECT prc,
	CBmpMan* pBmpMan, int nDSBShadeMode, bool bBrushOrigin )
{

	if ( nDSBShadeMode == 1 ) // 불투명
		pDC->FillSolidRect( prc, RGB(64,64,64) );
	else if ( nDSBShadeMode == 2 // 반투명
		&& DsbBlend( pDC, *prc ) ) ;
	else // 병치, 또는 Blending 실패
		pBmpMan->DrawShade( pDC,
			prc->left, prc->top,
			prc->right - prc->left,
			prc->bottom - prc->top, bBrushOrigin );
}
