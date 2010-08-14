// Board_Layout.cpp : CBoard 중 레이아웃에 관한 함수들 정의
//

#include "stdafx.h"
#include "Mighty.h"
#include "BmpMan.h"
#include "Board.h"
#include "Play.h"
#include "Player.h"
#include "MFSM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// 플레이어번호를 매핑한다 - 아래쪽 플레이어부터
// 시계 방향으로 0 ~ 6(v4.0) 의 표현 방법(절대값)을
// 실제 플레이어 번호(논리값)로 매핑하거나 역변환 한다
int CBoard::MapAbsToLogic( int nAbsPlayer ) const
{
	const CState* pState = m_pMFSM ? m_pMFSM->GetState() : 0;

	ASSERT( pState && pState->nPlayers > nAbsPlayer );

	if ( !pState ) return nAbsPlayer;

	return nAbsPlayer;
}

int CBoard::MapLogicToAbs( int nLogicalPlayer ) const
{
	return MapAbsToLogic( nLogicalPlayer );
}

// 각 플레이어의 이름 위치를 구한다
CRect CBoard::CalcNameRect( int nPlayers, int nPlayer,
	CDC* pDC, LPCTSTR sName, CFont* pFont ) const
{
	// 카드 크기
	int nCardWidth = m_szCard.cx;
	int nCardHeight = m_szCard.cy;

	// 점수 영역과 카드 영역 정보를 가지고 그릴 위치를 정한다
	bool bVert;
	CRect rcScore = CalcRect( nPlayers, CR_SCORE, nPlayer,
		nCardWidth, nCardHeight, -1, 0, &bVert );
	CRect rcHand = CalcRect( nPlayers, CR_HAND, nPlayer,
		nCardWidth, nCardHeight );
	CRect rcScreen; GetClientRect( &rcScreen );

	// 마진은 카드 폭의 1/16
	int nMargin = nCardWidth/16;

	// 그릴 위치
	int x, y;

	// 텍스트 정렬 방법
	UINT ta;

	// 폰트 선택
	CFont* pfntOld;
	if ( pFont )
		pfntOld = pDC->SelectObject( pFont );
	else pfntOld = (CFont*)pDC->SelectStockObject( SYSTEM_FONT );

	// 그려질 크기
	CSize szText = pDC->GetTextExtent( CString(sName) );

	if ( m_pMFSM && m_pMFSM->GetState()->state == msReady ) {

		CRect rc = ( rcHand | rcScore ) & rcScreen;

		ta = TA_CENTER|TA_BOTTOM;

		x = ( rc.left + rc.right ) /2;
		y = ( rc.top + rc.bottom ) /2;
		// 범위를 벗어나는 경우를 대비하여 위치를 조정한다
		x = max( rcScreen.left, x - szText.cx/2 ) + szText.cx/2;
		x = min( rcScreen.right, x + szText.cx/2 ) - szText.cx/2;
		y = max( rcScreen.top, y - szText.cy/2 ) + szText.cy/2;
		y = min( rcScreen.bottom, y + szText.cy/2 ) - szText.cy/2;
		if ( !bVert && rcHand.top > (rcScreen.top+rcScreen.bottom)/2 ) {
			// 아래쪽에 있는 경우
			y = rcHand.top - 3;
		}
	}
	else if ( bVert ) {		// 세로로 긴 경우

		if ( rcHand.left < (rcScreen.left+rcScreen.right)/2 ) {
			// 중앙에서 왼쪽에 있는 경우
			ta = TA_LEFT|TA_TOP;
			x = rcHand.right + nMargin;
			y = nPlayers == 7 ? rcScore.top + nMargin : rcScore.bottom + nMargin ;
		}
		else {
			// 중앙에서 오른쪽에 있는 경우
			ta = TA_RIGHT|TA_TOP;
			x = rcHand.left - nMargin;
			y = nPlayers == 7 ? rcScore.top + nMargin : rcScore.bottom + nMargin ;
		}
	}
	else {	// 가로로 긴 경우

		int xCenter = (rcScreen.left+rcScreen.right)/2;
		int bCenter = rcHand.left < xCenter && xCenter < rcHand.right;

		if ( bCenter ) {
			// 중앙에 있는 경우
			ta = TA_LEFT;
			x = rcHand.right + nMargin;
		}
		else if ( rcHand.left < xCenter ) {
			// 중앙에서 왼쪽에 있는 경우
			ta = TA_RIGHT;
			x = rcScore.left - nMargin;
			if ( x - szText.cx < nMargin ) {
				// 화면을 벗어난다
				ta = TA_LEFT;
				pDC->SetTextAlign( TA_LEFT );
				x = nMargin;
			}
		}
		else {
			// 중앙에서 오른쪽에 있는 경우
			ta = TA_LEFT;
			x = rcScore.right + nMargin;
			if ( x + szText.cx > rcScreen.right - nMargin ) {
				// 화면을 벗어난다
				ta = TA_RIGHT;
				pDC->SetTextAlign( TA_RIGHT );
				x = rcScreen.right - nMargin;
			}
		}

		if ( rcHand.top < (rcScreen.top+rcScreen.bottom)/2 ) {
			// 위쪽에 있는 경우
			ta |= TA_TOP;
			if ( bCenter )
				// 가운데 있는 경우
				y = rcScreen.top + nMargin;
			else // 양쪽에 있는 경우
				y = rcHand.bottom + nMargin;
		}
		else {
			// 아래쪽에 있는 경우
			ta |= TA_BOTTOM;
			if ( bCenter )
				// 가운데 있는 경우
				y = rcScreen.bottom - nMargin;
			else // 양쪽에 있는 경우
				y = rcHand.top - nMargin;
		}
	}
	pDC->SelectObject( pfntOld );

	CRect ret( x, y, x + szText.cx, y + szText.cy );

	if ( ( ta & TA_CENTER ) == TA_CENTER )
		ret.OffsetRect( +szText.cx/2, 0 );
	if ( ( ta & TA_RIGHT ) == TA_RIGHT )
		ret.OffsetRect( -szText.cx, 0 );
	if ( ( ta & TA_BOTTOM ) == TA_BOTTOM )
		ret.OffsetRect( 0, -szText.cy );

	return ret;
}


// 현재 화면 크기에 대한 적당한 카드 줌 크기를 얻는다
CSize CBoard::GetCardZoom( bool bUseZoom ) const
{
	CSize szCard = GetBitmapSize( IDB_SA );

	int nCardWidth = szCard.cx;
	int nCardHeight = szCard.cy;

	if ( !bUseZoom ) // 카드 줌을 사용하지 않으면
		return szCard;		// 항상 원래 카드 크기를 리턴

	int nHZoomBy1000 = 1000;
	int nVZoomBy1000 = 1000;

	CRect rc; GetClientRect( &rc );
	int nMinProperWidth = nCardWidth*5/2 + 2*nCardHeight;
	int nMinProperHeight = nCardHeight*9/2;
	int nMaxProperWidth = nCardWidth*7/2 + 2*nCardHeight;
	int nMaxProperHeight = nCardHeight*11/2;

	if ( rc.Width() < 10 || rc.Height() < 10 )
		// 너무 작으면 원래 카드 크기를 리턴
		return szCard;

	if ( rc.Width() < nMinProperWidth )
		nHZoomBy1000 = 1000*rc.Width()/nMinProperWidth;
	else if ( rc.Width() > nMaxProperWidth )
		nHZoomBy1000 = 1000*rc.Width()/nMaxProperWidth;
	if ( rc.Height() < nMinProperHeight )
		nVZoomBy1000 = 1000*rc.Height()/nMinProperHeight;
	else if ( rc.Height() > nMaxProperHeight )
		nVZoomBy1000 = 1000*rc.Height()/nMaxProperHeight;

	// 축소 필요성이 우선적으로 검사된다
	if ( nHZoomBy1000 < 1000 || nVZoomBy1000 < 1000 ) {
		int nZoom = min( nHZoomBy1000, nVZoomBy1000 );
		return CSize( szCard.cx*nZoom/1000, szCard.cy*nZoom/1000 );
	}
	// 두 방향으로 다 확대 해도 좋은 경우 더 작게 확대하는 쪽
	else if ( nHZoomBy1000 > 1000 && nVZoomBy1000 > 1000 ) {
		int nZoom = min( nHZoomBy1000, nVZoomBy1000 );
		// 확대 줌 값은 0.5 배(==500)의 배수씩으로 스케일
		nZoom = max( ( nZoom + 250 ) / 500 * 500, 1000 );
		return CSize( szCard.cx*nZoom/1000, szCard.cy*nZoom/1000 );
	}
	else return szCard;
}

// 현재 화면 크기에 대한 적당한 폰트들을 새로 만든다
void CBoard::CreateFont()
{
	if ( m_fntSmall.GetSafeHandle() ) m_fntSmall.DeleteObject();
	if ( m_fntMiddle.GetSafeHandle() ) m_fntMiddle.DeleteObject();
	if ( m_fntBig.GetSafeHandle() ) m_fntBig.DeleteObject();

	// 폰트 크기는 작은 글꼴이 14
	LONG lfSmallHeight =
		14 * GetCardZoom(true).cy / GetBitmapSize(IDB_SA).cy;

	LOGFONT lf;
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = 600;
	lf.lfItalic = 0;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0;
	lf.lfCharSet = HANGUL_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH|FF_SWISS;
	_tcscpy( lf.lfFaceName, _T("돋움") );

	// 작은 글꼴
	lf.lfHeight = -lfSmallHeight;
	m_fntSmall.CreateFontIndirect( &lf );
	// 중간 글꼴
	lf.lfHeight = -lfSmallHeight * 3/2;
	m_fntMiddle.CreateFontIndirect( &lf );
	// 큰 글꼴
	lf.lfHeight = -lfSmallHeight * 5/2;
	m_fntBig.CreateFontIndirect( &lf );

	// 고정크기 글꼴
	if ( !m_fntFixedSmall.GetSafeHandle() ) {
		lf.lfHeight = -14;
		m_fntFixedSmall.CreateFontIndirect( &lf );
	}
	if ( !m_fntFixedMiddle.GetSafeHandle() ) {
		lf.lfHeight = -21;
		m_fntFixedMiddle.CreateFontIndirect( &lf );
	}
	if ( !m_fntFixedBig.GetSafeHandle() ) {
		lf.lfHeight = -35;
		m_fntFixedBig.CreateFontIndirect( &lf );
	}
}

// 화면상의 각 사각형 좌표를 얻는다
// nPlayers   : 플레이 하는 플레이어의 수 ( 1 ~ 7 )
// type       : 0 가운데 1 점수영역 2 손에든 카드 3 따는곳
// nPlayer    : 어떤 플레이어에 대한 영역인가 (nType!=0일때만 사용)
// nCardWidth : 카드의 가로 크기
// nCardHeight: 카드의 세로 크기
// nIndex, nAll : 사각형 안에 들어 있는 각 아이템의 위치를
//                전체(nAll)중의 몇번째(nInex)인가에 대한 값으로
//                리턴한다, nIndex == -1 이면 전체 사각형을 리턴 
// pbVertical : 0 이 아니면 이 사각형이 세로로 긴 사각형인지를 리턴한다
// pbDirection: 0 이 아니면 방향을 리턴 ( true:위에서 아래,왼쪽에서 오른쪽 false:그 반대)
CRect CBoard::CalcRect( int nPlayers, CR_TYPE type, int nPlayer, 
	int nCardWidth, int nCardHeight,
	int nIndex, int nAll, bool* pbVertical, bool* pbDirection ) const
{
#define GR_SETBOTTOM( RC, Y )	( (RC).top = (Y) - (RC).Height(), (RC).bottom = (Y) )
#define GR_SETTOP( RC, Y )		( (RC).bottom = (Y) + (RC).Height(), (RC).top = (Y) )
#define GR_SETVCENTER( RC, Y )	( (RC).SetRect( (RC).left, (Y)-(RC).Height()/2, (RC).right, (Y)-(RC).Height()/2+(RC).Height() ) )
#define GR_SETLEFT( RC, X )		( (RC).right = (X) + (RC).Width(), (RC).left = (X) )
#define GR_SETRIGHT( RC, X )	( (RC).left = (X) - (RC).Width(), (RC).right = (X) )
#define GR_SETHCENTER( RC, X )	( (RC).SetRect( (X)-(RC).Width()/2, (RC).top, (X)-(RC).Width()/2+(RC).Width(), (RC).bottom ) )
#define GR_CENTEREDRECT( RC, X, Y, W, H ) RC( (X)-(W)/2, (Y)-(H)/2, (X)-(W)/2+(W), (Y)-(H)/2+(H) )

	// 여기서 사용하는 플레이어 번호는 절대 좌표다
	// 언제나 아래쪽 부터 시계방향으로 0 ~ 6(v4.0) 의 번호를 갖는다
	nPlayer = MapLogicToAbs(nPlayer);

	// 플레이어 수와 플레이어 위치 정보
	// aanLocInfo[인원][플레이어번호] 에 수직, 수평 정보를 포함한다
	// 31 로 AND 하여 수평 정보를 얻을 수 있다
	// 1(left) 2(left-center) 4(center) 8(right-center) 16(right)
	// 32(top) 64( top-vcenter) 128(vcenter)
	//                  256(bottom-vcenter) 512(bottom)
	static const int aanLocInfo[MAX_PLAYERS+1][MAX_PLAYERS] = {
			{ 0, 0, 0, 0, 0, 0, 0 },							// 0 명
			{ 4+512, 0, 0, 0, 0, 0, 0 },						// 1 명
			{ 4+512, 4+32, 0, 0, 0, 0, 0 },						// 2 명
			{ 4+512, 1+64, 16+64, 0, 0, 0, 0 },					// 3 명
			{ 4+512, 1+128, 4+32, 16+128, 0, 0, 0 },			// 4 명
			{ 4+512, 1+256, 2+32, 8+32, 16+256, 0, 0  },		// 5 명
			{ 8+512, 2+512, 1+128, 2+32, 8+32, 16+128, 0 },		// 6 명
			{ 4+512, 1+512, 1+64, 2+32, 8+32, 16+64, 16+512 },	// 7 명
	};

	if( !Mo()->bClockwise ) {
		nPlayer = nPlayer == 0 ? 0 : nPlayers - nPlayer;
	}

	CRect rc; GetClientRect(&rc);
	int xCenter = rc.left + rc.Width()/2;
	int yCenter = rc.top + rc.Height()/2;

	if ( type == CR_CENTER ) {	// 가운데

		int hRc = nCardHeight * 2 + 8;
		int wRc = nCardWidth * 3 - nCardWidth/5;
		// 중심이 가운데 있고 크기가 wRc, hRc 만한 사각형
		CRect GR_CENTEREDRECT( rcCenter, xCenter, yCenter, wRc, hRc );

		// 세부 정보가 필요 없으면 그 사각형을 리턴한다
		if ( nIndex == -1 ) return rcCenter;

		// 세부 정보를 리턴할 사각형
		CRect rcRet( -nCardWidth, -nCardHeight, 0, 0);

		// 위치 정보 ( nPlayer 가 아니라 nIndex 가 사람 위치에 대한 인자임)
		int loc;
		if( Mo()->bClockwise )
			loc = aanLocInfo[nPlayers][nIndex];
		else
			loc = aanLocInfo[nPlayers][nIndex==0?0:nPlayers-nIndex];

			 if ( loc & 1 ) GR_SETLEFT( rcRet, rcCenter.left + 2 );
		else if ( loc & 2 ) GR_SETLEFT( rcRet, rcCenter.left + ( xCenter - rcCenter.left )/3 );
		else if ( loc & 4 ) GR_SETHCENTER( rcRet, xCenter );
		else if ( loc & 8 ) GR_SETRIGHT( rcRet, rcCenter.right - ( rcCenter.right - xCenter )/3 );
		else if ( loc & 16 ) GR_SETRIGHT( rcRet, rcCenter.right - 2 );
			 if ( loc & 32 ) GR_SETTOP( rcRet, rcCenter.top + 2 );
		else if ( loc & 64 ) GR_SETTOP( rcRet, rcCenter.top + 2 );
		else if ( loc & 128 ) GR_SETVCENTER( rcRet, yCenter );
		else if ( loc & 256 ) GR_SETVCENTER( rcRet, yCenter + nCardHeight/7 );
		else if ( loc & 512 ) GR_SETBOTTOM( rcRet, rcCenter.bottom - 2 );

		return rcRet;
	}
	else if ( type == CR_SCORE
		|| type == CR_HAND ) {	// 점수판 , 손에 든 카드

		int loc = aanLocInfo[nPlayers][nPlayer];

		CRect rcHand;
		bool bVert = loc & 1+16 ? true : false;	// 양 끝쪽은 세로로 된 사각형이다
		if ( pbVertical ) *pbVertical = bVert;

		bool bDir = ( loc & 1 ) ? true	// 왼쪽은 아래에서 위
			: ( loc & 16 ) ? false		// 오른쪽은 위에서 아래
			: ( loc & 512 ) ? true		// 아래쪽은 왼쪽에서 오른쪽
			: false;					// 그 외는 오른쪽에서 왼쪽
		if ( pbDirection ) *pbDirection = bDir;

		if ( type == CR_SCORE ) {
			if ( bVert ) {
					if ( rc.Height() < 5*nCardHeight+nCardHeight/2 )
						rcHand.SetRect( -nCardHeight, -( nCardHeight + nCardWidth ), 0, 0 );
					else rcHand.SetRect( -nCardHeight, -( rc.Height()/2 - nCardHeight*7/4 + nCardWidth), 0, 0 );

				if ( rc.Width() < 4*nCardWidth+2*nCardHeight ) {
						 if ( loc & 1 ) GR_SETRIGHT( rcHand, xCenter - nCardHeight - nCardWidth/2 );
					else if ( loc & 16 ) GR_SETLEFT( rcHand, xCenter + nCardHeight + nCardWidth/2 );
				}
				else {
						 if ( loc & 1 ) GR_SETLEFT( rcHand, nCardWidth/2 );
					else if ( loc & 16 ) GR_SETRIGHT( rcHand, rc.right - nCardWidth/2 );
				}
					 if ( loc & 64 ) GR_SETVCENTER( rcHand, yCenter - rcHand.Height()/7 );
				else if ( loc & 128 ) GR_SETVCENTER( rcHand, yCenter );
				else if ( loc & 256 ) GR_SETVCENTER( rcHand, yCenter + rcHand.Height()/7 );
				else if ( loc & 512 ) GR_SETBOTTOM( rcHand, rc.bottom );
			}
			else {
				if ( rc.Width() < 4*nCardWidth+2*nCardHeight )
					rcHand.SetRect( -nCardWidth*7/4, -nCardHeight, 0, 0 );
				else rcHand.SetRect(
					-( (rc.Width()-4*nCardWidth-2*nCardHeight)/3 + nCardWidth*7/4 ), -nCardHeight, 0, 0 );

					 if ( loc & 2 ) GR_SETRIGHT( rcHand, xCenter - ( xCenter - rc.left ) / 6 );
				else if ( loc & 4 ) GR_SETHCENTER( rcHand, xCenter );
				else if ( loc & 8 ) GR_SETLEFT( rcHand, xCenter + ( rc.right - xCenter ) /6 );

				if ( rc.Height() < 5*nCardHeight + nCardHeight/2) {
						 if ( loc & 32 ) GR_SETBOTTOM( rcHand, yCenter - nCardHeight - nCardHeight/4 );
					else if ( loc & 512 ) GR_SETTOP( rcHand, yCenter + nCardHeight + nCardHeight/4 );
				}
				else {
						 if ( loc & 32 ) GR_SETTOP( rcHand, nCardHeight/2 );
					else if ( loc & 512 ) GR_SETBOTTOM( rcHand, rc.bottom - nCardHeight/2 );
				}
			}
		}
		else {

			if ( bVert ) {
				if ( nPlayers == 7 ) {	// 7마에서 왼쪽 오른쪽 플레이어는 nCardWidth만큼 크기를 줄인다 (v4.0 : 2010.4.11)
					if ( rc.Height() < 5*nCardHeight+nCardHeight/2 )
						rcHand.SetRect( -nCardHeight, -nCardHeight*2 , 0, 0 );
					else rcHand.SetRect( -nCardHeight, -( rc.Height()/2 - nCardHeight*3/4 ), 0, 0 );
				}
				else {					// 왼쪽 오른쪽 플레이어들은 가로로 길게 표시 (v3.21)
					if ( rc.Height() < 5*nCardHeight + nCardHeight/2 )
						rcHand.SetRect( -nCardHeight, -( nCardHeight*2 + nCardWidth ), 0, 0 );
					else rcHand.SetRect( -nCardHeight, -( rc.Height()/2 - nCardHeight*3/4 + nCardWidth ), 0, 0 );
				}

				if ( rc.Width() < 4*nCardWidth+2*nCardHeight ) {
						 if ( loc & 1 ) GR_SETRIGHT( rcHand, xCenter - nCardHeight - nCardWidth );
					else if ( loc & 16 ) GR_SETLEFT( rcHand, xCenter + nCardHeight + nCardWidth );
				}
				else {
						 if ( loc & 1 ) GR_SETLEFT( rcHand, 0 );
					else if ( loc & 16 ) GR_SETRIGHT( rcHand, rc.right );
				}
					 if ( loc & 64 ) GR_SETVCENTER( rcHand, yCenter - rcHand.Height()/7 );
				else if ( loc & 128 ) GR_SETVCENTER( rcHand, yCenter );
				else if ( loc & 256 ) GR_SETVCENTER( rcHand, yCenter + rcHand.Height()/7 );
				else if ( loc & 512 ) GR_SETVCENTER( rcHand, rc.bottom - nCardHeight );
			}
			else {
				if ( ( nPlayers == 5 && nPlayer != 0 ) ||	// 5마에서 위 플레이어
					nPlayers == 6 ||						// 6마에서 위, 아래 플레이어
					( nPlayers == 7 && nPlayer != 0 ) ) {	// 7마에서 위 플레이어는 nCardWidth/4만큼 크기를 줄인다 (v4.0 : 2010.4.9-11)
					if ( rc.Width() < 4*nCardWidth+2*nCardHeight )
						rcHand.SetRect( -nCardWidth*13/4, -nCardHeight, 0, 0 );
					else rcHand.SetRect(
						-( (rc.Width()-4*nCardWidth-2*nCardHeight)/2 + nCardWidth*13/4 ), -nCardHeight, 0, 0 );
				}
				else {
					if ( rc.Width() < 4*nCardWidth+2*nCardHeight )
						rcHand.SetRect( -nCardWidth*3-nCardWidth/2, -nCardHeight, 0, 0 );
					else rcHand.SetRect(
						-( (rc.Width()-4*nCardWidth-2*nCardHeight)/2 + nCardWidth*3 + nCardWidth/2 ), -nCardHeight, 0, 0 );
				}

					 if ( loc & 2 ) GR_SETRIGHT( rcHand, xCenter - ( xCenter - rc.left )/40 );
				else if ( loc & 4 ) GR_SETHCENTER( rcHand, xCenter );
				else if ( loc & 8 ) GR_SETLEFT( rcHand, xCenter + ( rc.right - xCenter )/40 );
				if ( rc.Height() < 5*nCardHeight + nCardHeight/2 ) {
						 if ( loc & 32 ) GR_SETBOTTOM( rcHand, yCenter - nCardHeight*7/4 );
					else if ( loc & 512 ) GR_SETTOP( rcHand, yCenter + nCardHeight*7/4 );
				}
				else {
						 if ( loc & 32 ) GR_SETTOP( rcHand, 0 );
					else if ( loc & 512 ) GR_SETBOTTOM( rcHand, rc.bottom );
				}
			}
		}

		if ( nIndex == -1 ) return rcHand;

		// 이제 세부 정보를 리턴
		if ( bVert ) {
			int nMargin = rcHand.Height()-nCardWidth*nAll;
			if ( nMargin >= 0 )
				// 모든 카드가 다 들어간다 - 가운데로 모은다
				return CRect(
					CPoint( rcHand.left,
						bDir ? rcHand.top + nMargin/2 + nCardWidth*nIndex
						: rcHand.bottom - nMargin/2 - nCardWidth*(nIndex+1) ),
					CSize( nCardHeight, nCardWidth ) );
			else // 맨 위부터 차곡차곡
				return CRect(
					CPoint( rcHand.left, rcHand.top
						+ (rcHand.Height()-nCardWidth)
							* (bDir?nIndex:(nAll-nIndex-1)) / (nAll-1) ),
					CSize( nCardHeight, nCardWidth ) );
		}
		else {
			int nMargin = rcHand.Width()-nCardWidth*nAll;
			if ( nMargin >= 0 )
				// 모든 카드가 다 들어간다 - 가운데로 모은다
				return CRect(
					CPoint(
						bDir ? rcHand.left + nMargin/2 + nCardWidth*nIndex
						: rcHand.right - nMargin/2 - nCardWidth*(nIndex+1),
						rcHand.top ),
					CSize( nCardWidth, nCardHeight ) );
			else // 맨 왼쪽부터 차곡차곡
				return CRect(
					CPoint( rcHand.left
						+ (rcHand.Width()-nCardWidth)
							* (bDir?nIndex:(nAll-nIndex-1)) / (nAll-1),
						rcHand.top ),
					CSize( nCardWidth, nCardHeight ) );
		}
	}
	else if ( type == CR_HIDDEN ) {	// 따는 곳

		int loc = aanLocInfo[nPlayers][nPlayer];

		CRect rcRet( -nCardWidth, -nCardHeight, 0, 0);

			 if ( loc & 1 ) GR_SETRIGHT( rcRet, -5 );
		else if ( loc & 2 ) GR_SETRIGHT( rcRet, xCenter - (xCenter - rc.left)*2/5 );
		else if ( loc & 4 ) GR_SETHCENTER( rcRet, xCenter );
		else if ( loc & 8 ) GR_SETLEFT( rcRet, xCenter + (rc.right - xCenter)*2/5 );
		else if ( loc & 16 ) GR_SETLEFT( rcRet, rc.right + 5 );
			 if ( loc & 32 ) GR_SETBOTTOM( rcRet, -5 );
		else if ( loc & 64 ) GR_SETVCENTER( rcRet, yCenter - rc.Height()/10 );
		else if ( loc & 128 ) GR_SETVCENTER( rcRet, yCenter );
		else if ( loc & 256 ) GR_SETVCENTER( rcRet, yCenter + rc.Height()/10 );
		else if ( loc & 512 ) GR_SETTOP( rcRet, rc.bottom + 5 );

		return rcRet;
	}
	ASSERT(0);
	return CRect(0,0,0,0);
}

// 사람이 든 카드 중에서 지정한 인덱스에 있는 카드의
// 실제 사각형 위치를 구함 ( 쉬프트 된 카드 위치까지 고려 )
bool CBoard::CalcRealCardRect( LPRECT prc, int index ) const
{
	const CState* pState = m_pMFSM ? m_pMFSM->GetState() : 0;
	if ( !pState ) return false;
	const CPlayer* pPlayer = pState->apPlayers[0];
	const CCardList* pHand = pPlayer->GetHand();
	if ( !pPlayer->IsHuman() ) return false;

	int nCards = pHand->GetCount();

	CRect rc = CalcRect( pState->nPlayers, CR_HAND, 0,
		m_szCard.cx, m_szCard.cy, index, nCards );

	// 세장 선택중 or 2마 딜 상황이면(v4.0: 2010.4.13)
	if ( ( pState->state == msPrivilege
			|| ( pState->state == msDeal2MA && pState->nPlayers == 2 ) )
			&& GetSelection(index) )
		rc.OffsetRect( 0, -m_szCard.cy/CARD_SHIFT_RATIO_OF_SELECTION );

	*prc = rc;
	return true;
}

// 사람이 든 카드 중에서 지정한 포인트에 있는
// 카드의 POSITION 을 리턴
POSITION CBoard::CardFromPoint( int x, int y ) const
{
	if ( !m_pMFSM ) return 0;
	const CState* pState = m_pMFSM->GetState();
	if ( !pState->apPlayers[0]->IsHuman() ) return 0;

	CRect rc = CalcRect( pState->nPlayers, CR_HAND, 0,
		m_szCard.cx, m_szCard.cy );
	rc.top -= m_szCard.cy / CARD_SHIFT_RATIO_OF_SELECTION;
	if ( !rc.PtInRect(CPoint(x,y)) ) return 0;

	const CCardList* pHand = pState->apPlayers[0]->GetHand();

	int nCards = pHand->GetCount();
	for ( int i = nCards-1; i >= 0; i-- ) {

		// 가장 오른쪽에 있는 카드부터 검사한다
		if ( CalcRealCardRect( &rc, i )
				&& rc.PtInRect( CPoint(x,y) ) )
			// 찾았다
			return pHand->POSITIONFromIndex(i);
	}
	return 0;
}

// FlyCard 보조함수- 주어진 정보로 시작 사각형이나 종료 사각형을 계산한다
// 전달 인자는 FlyCard 와 같다
RECT CBoard::CalcCardRectForFlyCard( int nType, int nPlayer, int nIndex )
{
	static const CR_TYPE aTable[] = {
		CR_CENTER, CR_SCORE, CR_HAND, CR_HIDDEN };

	ASSERT( m_pMFSM );
	if ( !m_pMFSM ) return CRect( 0, 0, 0, 0 );

	const CState* pState = m_pMFSM->GetState();

	if ( nType == 4 ) {
		// 덱의 꼭대기 !!
		// 이 식은 DrawCenterAnimation 함수에서 따 왔으므로 유의
		int nCount, nCardCount = pState->lDeck.GetCount();
		if ( nCardCount < 3 ) nCount = nCardCount;
		else if ( nCardCount < 10 ) nCount = nCardCount*2/3;
		else nCount = m_szCard.cx * (nCardCount-9) / 520 + 6;
		int nMaxCount = m_szCard.cx * (53-9) / 520 + 6;

		CRect rc; GetClientRect( &rc );
		int y = rc.top + rc.Height()/2 - m_szCard.cy/2
			+ nMaxCount*2 - (nCount-1)*2;
		int x = rc.left + rc.Width()/2 - m_szCard.cx/2;
		return CRect( CPoint(x,y), m_szCard );
	}

	// 총 아이템 개수
	int nAll = 0;
	if ( nType == CR_CENTER )
		nAll = pState->nPlayers;
	else if ( nType == CR_SCORE )
		nAll = pState->apPlayers[nPlayer]->GetScore()->GetCount(),
		nAll = max( nAll-1, 0 );
	else if ( nType == CR_HAND )
		nAll = pState->apPlayers[nPlayer]->GetHand()->GetCount()+1;

	// nAll 의 개수가 1 씩 변경되는 것은 Score, Hand 의 인덱스를
	// 지정하는 순간 이미 손에서 카드는 나가버린 상태이며
	// 득점된 카드는 받은 상태다 - 이를 생각하지 않기 위해
	// 이와 같이 하였다

	CRect rc = CalcRect(
		pState->nPlayers, aTable[nType], nPlayer,
		m_szCard.cx, m_szCard.cy, nIndex, nAll );
	if ( nIndex < 0 ) { // 한 가운데로
		rc.left = (rc.left+rc.right)/2 - m_szCard.cx/2;
		rc.right = rc.left + m_szCard.cx;
		rc.top = (rc.top+rc.bottom)/2 - m_szCard.cy/2;
		rc.bottom = rc.top + m_szCard.cy;
	}
	else if ( nType == CR_HAND && nPlayer == 0 )
		rc.OffsetRect( 0, - m_szCard.cy/CARD_SHIFT_RATIO_OF_MOUSE_OVER );

	return rc;
}
