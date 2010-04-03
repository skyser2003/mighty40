// DEtc.cpp: implementation of the DEtc class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"
#include "BoardWrap.h"
#include "DSB.h"
#include "DEtc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// DAbout

void DAbout::Create()
{
	SetFixed();
	DSB::Create( 0, 0, 18, 10, -1 );
}

// 초기화 ( 생성된 후 호출됨 )
void DAbout::OnInit()
{
	DSB::OnInit();

	RegisterHotspot(
		8, 8, -1, -1, true, 0,
		_T("확인"),
		&s_colWhite, &s_tdShade, &s_colCyan, &s_tdShadeOutline,
		(LPVOID)0 );
}

// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
// 오버라이드 하여 구현해야 한다
// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
void DAbout::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	CRect rc = PutBitmap( pDC, IDB_MIGHTY,
		_T(" 마이티 네트워크 3.2"), 1, 1, true,
		s_colWhite, s_tdShade );

	rc = PutText( pDC, _T("Copyright (C) 1999 장문성"),
		rc.left, rc.bottom + rc.Height()/2, false,
		s_colWhite, s_tdShade );
	PutText( pDC, _T("sw6ueyz@hitel.net"),
		rc.left, rc.bottom + rc.Height()/4, false,
		s_colWhite, s_tdShade );
}


/////////////////////////////////////////////////////////////////////////////
// DMessageBox

DMessageBox::DMessageBox( CBoardWrap* pBoard ) : DSB(pBoard)
{
	m_nLines = 0;
	m_asLine = 0;
}

DMessageBox::~DMessageBox()
{
	delete[] m_asLine;
}

// nLines 줄의 메시지와 ok 버튼을 출력한다
// nTimeOut 이 -1 이 아니면 ok 버튼은 없다
void DMessageBox::Create( bool bCenter, int nLines,
	LPCTSTR asLine[], int nTimeOut )
{
	int nUnit = GetDSBUnit();

	m_bCenter = bCenter;
	m_nLines = nLines;
	m_asLine = new CString[nLines];

	// 스트링을 m_asLine 에 복사하면서
	// 최대 가로 크기를 구한다
	int nMaxWidth = 0;
	for ( int i = 0; i < nLines; i++ ) {
		m_asLine[i] = asLine[i];
		int nWidth = GetTextExtent( 0, asLine[i] ).cx;
		if ( nMaxWidth < nWidth )
			nMaxWidth = nWidth;
	}

	// 실제 DSB 의 크기를 계산하고 DSB 를 생성
	int xDSB = ( nMaxWidth + nUnit - 1 ) / nUnit + 2;
	int yDSB = nLines + 2 + ( nTimeOut == -1 ? 2 : 0 );

	DSB::Create( 0, 0, xDSB, yDSB, nTimeOut );
}

// 초기화 ( 생성된 후 호출됨 )
void DMessageBox::OnInit()
{
	DSB::OnInit();

	if ( m_nTimeOut == -1 )

		RegisterHotspot(
			m_rcDSB.right/2-1, m_rcDSB.bottom-2, -1, -1, true, 0,
			_T("확인"),
			&s_colWhite, &s_tdShade, &s_colCyan, &s_tdShadeOutline,
			(LPVOID)0 );
}

// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
// 오버라이드 하여 구현해야 한다
// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
void DMessageBox::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	for ( int i = 0; i < m_nLines; i++ )
		PutText( pDC, m_asLine[i],
			( m_bCenter ? -1 : 1 ),
			i + 1, true, s_colWhite, s_tdShade );
}


/////////////////////////////////////////////////////////////////////////////
// DShortMessage

// idBitmap : 비트맵 ( 없으면 0 )
// s        : 메시지 ( 없으면 0 )
// bCenter  : 메시지가 상자의 가운데 출력되는가?
// bFixed   : Fixed DSB 인가
// nTimeOut : 타임아웃 (millisecond)
// nPlayer  : 표시될 플레이어 번호 ( -1 이면 가운데 )
// sName    : 이 플레이어의 이름 ( 0 이면 이름 표시안됨 )
void DShortMessage::Create( UINT idBitmap, LPCTSTR s,
		bool bCenter, bool bFixedSize,
		int nTimeOut, int nPlayer, LPCTSTR sName )
{
	if ( bFixedSize ) SetFixed();

	m_idBitmap = idBitmap;
	m_s = s ? s : _T("");
	m_bCenter = bCenter;
	m_sName = sName ? sName : _T("");

	// 필요한 크기를 DSB 단위로 계산한다

	int nUnit = GetDSBUnit();

	SIZE szBitmap;
	if ( idBitmap == 0 ) szBitmap = CSize(0,0);
	else szBitmap = GetBitmapSize(idBitmap);

	int nLen = s ? GetTextExtent(0,s).cx : 0;
	int nNameLen = sName ? GetTextExtent( 0, sName ).cx : 0;
	if ( nNameLen > nLen ) nLen = nNameLen;

	int cxDSB = ( szBitmap.cx + 4 + nLen + nUnit - 1 + 18 /* X버튼 */ ) / nUnit;
	int cyDSB = max( ( szBitmap.cy + 4 + nUnit - 1 ) / nUnit, 2 );
	if ( sName ) cyDSB += 2;

	// 표시될 위치를 계산한다
	if ( nPlayer == -1 )
		DSB::Create( 0, 0, cxDSB, cyDSB, nTimeOut );
	else {
		bool bVert;
		CRect rcClient; m_pBoard->GetClientRect( &rcClient );
		CRect rcRange = m_pBoard->CalcPlayerExtent( nPlayer, &bVert );

		DSB::Create2(
			rcRange.left+rcRange.Width()/2 - cxDSB*nUnit/2,
			bVert || rcRange.bottom < rcClient.top + rcClient.Height()/2
				? rcRange.top+rcRange.Height()/2 - cyDSB*nUnit/2
				: rcRange.bottom - cyDSB*nUnit,
			cxDSB, cyDSB, nTimeOut );
	}
}

// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
// 오버라이드 하여 구현해야 한다
// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
void DShortMessage::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	if ( !m_sName.IsEmpty() ) {

		CRect rcBox; GetRect( &rcBox );
		CRect rcName = PutText( pDC, m_sName,
			0, 0, true, s_colYellow, s_tdShadeOutline );

		if ( m_idBitmap == 0 ) {
			CRect rc = PutText( pDC, m_s, m_bCenter ? -1 : 0, -1, true,
					s_colWhite, s_tdShade, true );
			PutText( pDC, m_s,
				rc.left,
				( rcBox.bottom + rcName.bottom )/2 - rc.Height()/2,
				false, s_colWhite, s_tdShade );
		}
		else {
			CRect rc = PutBitmap( pDC, m_idBitmap, m_s,
					m_bCenter ? -1 : 0, 1, true,
					s_colWhite, s_tdShade, true );
			PutBitmap( pDC, m_idBitmap, m_s,
				rc.left,
				( rcBox.bottom + rcName.bottom )/2 - rc.Height()/2,
				false, s_colWhite, s_tdShade );
		}
	}
	else {

		if ( m_idBitmap == 0 )
			PutText( pDC, m_s, m_bCenter ? -1 : 0, -1, true,
					s_colWhite, s_tdShade );
		else PutBitmap( pDC, m_idBitmap, m_s,
					m_bCenter ? -1 : 0, -1, true,
					s_colWhite, s_tdShade );
	}
}


/////////////////////////////////////////////////////////////////////////////
// DSelect

// x, y     : 다이얼로그가 생겨야 할 위치 (CBoard 기준좌표)
// asText   : 선택할 텍스트 배열
// nText    : asText 의 아이템 개수
// pResult  :선택 결과의 인덱스 (-1은 취소)
void DSelect::Create( int x, int y,
	LPCTSTR asText[], int nText, CEvent* pEvent, long* pResult )
{
	m_asText = asText;
	m_nText = nText;

	int nMaxWidth = 0;
	for ( int i = 0; i < m_nText; i++ )
		nMaxWidth = max( nMaxWidth, GetTextExtent(0,asText[i]).cx );

	int nUnit = GetDSBUnit();
	int nMaxWidthDSB = ( nMaxWidth + nUnit - 1 ) / nUnit + 2;

	SetModal();

	DSB::Create2( x, y, nMaxWidthDSB, nText, -1 );

	SetAction( pEvent, pResult );
}

// 초기화 ( 생성된 후 호출됨 )
void DSelect::OnInit()
{
	DSB::OnInit();

	for ( int i = 0; i < m_nText; i++ )

		RegisterHotspot(
			0, i, m_rcDSB.right-2, 1, true, 0,
			m_asText[i],
			&s_colWhite, &s_tdShade, &s_colCyan, &s_tdShadeOutline,
			(LPVOID)(i+1) );
}

// 핫 스팟을 클릭할 때 불리는 함수 ( 스스로 호출한다 )
// LPVOID 값이 0 이면 종료 (X자를 클릭) 요청
void DSelect::OnClick( LPVOID pVoid )
{
	if ( pVoid == (LPVOID)0xffffffff ) pVoid = 0;	// Enter Key

	if ( m_pResult ) *(long*)m_pResult = (long)pVoid - 1;
	Destroy();
}


/////////////////////////////////////////////////////////////////////////////
// DSelectJokerShape

// x, y           : 다이얼로그가 생겨야 할 위치 (CBoard 기준좌표)
// bUseTerm       : 용어를 쓸것인가 (조커콜/조커콜아님 쪼카컴!/쪼콜아님)
// nPlayer, nCard : mmTurn 메시지를 만드는데 필요한 값
//                  카드를 낸 플레이어, 낸 카드
void DSelectJokerShape::Create( int x, int y,
	CEvent* pEvent, long* pShape )
{
	SetFixed();
	SetModal();
	DSB::Create2( x, y, 5, 4, -1 );
	SetAction( pEvent, pShape );
}

// 초기화 ( 생성된 후 호출됨 )
void DSelectJokerShape::OnInit()
{
	DSB::OnInit();

	RegisterHotspot(
		0, 0, -1, -1, true, IDB_SPADE, 0,
		&s_colWhite, &s_tdOutline, &s_colWhite, &s_tdShade,
		(LPVOID)1 );
	RegisterHotspot(
		2, 0, -1, -1, true, IDB_DIAMOND, 0,
		&s_colWhite, &s_tdOutline, &s_colWhite, &s_tdShade,
		(LPVOID)2 );
	RegisterHotspot(
		0, 2, -1, -1, true, IDB_HEART, 0,
		&s_colWhite, &s_tdOutline, &s_colWhite, &s_tdShade,
		(LPVOID)3 );
	RegisterHotspot(
		2, 2, -1, -1, true, IDB_CLOVER, 0,
		&s_colWhite, &s_tdOutline, &s_colWhite, &s_tdShade,
		(LPVOID)4 );
}

// 핫 스팟을 클릭할 때 불리는 함수 ( 스스로 호출한다 )
// LPVOID 값이 0 이면 종료 (X자를 클릭) 요청
void DSelectJokerShape::OnClick( LPVOID pVoid )
{
	if ( pVoid == (LPVOID)0xffffffff ) pVoid = 0;	// Enter Key

	*(long*)m_pResult = (int)pVoid;
	Destroy();
}


/////////////////////////////////////////////////////////////////////////////
// DSay

DSay::DSay( CBoardWrap* pBoard ) : DSB(pBoard)
{
	m_asLine = 0;
}

DSay::~DSay()
{
	if ( m_asLine ) {
		for ( int i = 0; i < m_nLines; i++ )
			delete[] m_asLine[i];
		delete[] m_asLine;
	}
}

// x, y  : 상자의 중심 값 (DP 좌표)
//         상자가 pBoard의 클라이언트 영역을 벗어 날 경우
//         안쪽으로 끌여들여 져서 맞추어 진다
// cLine : 한 줄에 들어가는 글자의 대략적인 수 (DSB유닛)
//         (이 줄을 넘어가면 줄바꿈 됨)
// sWho  : 말한 사람의 이름으로, 맨 윗줄에 강조체로 표시된다
// sMsg  : 말한 내용
// nTimeOut : 유지 시간 (millisecond)
// pDSB  : Board 에 떠 있던 이 플레이어의 이전 대화 상자
// bSetBelow : 맨 밑으로 갈 것인가
void DSay::Create( DSay* pDSB, int x, int y,
				  int cLine, LPCTSTR sWho, LPCTSTR sMsg, int nTimeOut,
				  bool bSetBelow )
{
	int i;
	int nUnit = GetDSBUnit();

	// Below 모드로 동작 (非Below 모드 상자보다 무조건 아래로)

	// 수정: 이 기능을 넣었더니 대화가 너무 안보이는 문제가 발생
	//       다시 맨 위로 가게 했다
	if ( bSetBelow ) SetBelow();


	// 먼저 이 스트링이 몇 줄 정도 되는 스트링인가를 조사한다
	size_t nMaxWidth;
	m_nLines = ParseString( 0, cLine, sMsg, nMaxWidth );
	if ( m_nLines < 0 )	{ // 부적절한 스트링
		delete this;
		return;
	}

	// 이전 DSB 의 마지막 두 줄을 뽑아낸다 !
	m_nLastLines = 0;
	if ( pDSB ) {

		for ( int i = 0; i < 2; i++ ) {

			if ( pDSB->m_nLines+pDSB->m_nLastLines > i ) {
				m_asLast[i] =
					pDSB->m_nLines > i
					? pDSB->m_asLine[pDSB->m_nLines-i-1]
					: pDSB->m_asLast[i-pDSB->m_nLines];
				m_nLastLines++;
			}
			size_t len = _tcslen( m_asLast[i] );
			if ( nMaxWidth < len+1 ) nMaxWidth = len+1;
		}
	}

	// 메모리를 할당하고 실제로 스트링을 파스 한다
	m_asLine = new TCHAR*[m_nLines];
	for ( i = 0; i < m_nLines; i++ )
		m_asLine[i] = new TCHAR[nMaxWidth];

	ParseString( m_asLine, cLine, sMsg, nMaxWidth );

	// 나머지 정보 복사
	m_sWho = sWho;

	// 상자의 폭은 cLine DSB단위
	int cxDSB = cLine + 2;
	int cx = cxDSB * nUnit;

	// 상자의 높이를 구한다 - DSB 단위로 줄 수 + 3 이다
	int cyDSB = m_nLines + 2 + ( m_nLastLines > 0 ? m_nLastLines : 1 );
	int cy = cyDSB * nUnit;

	// 상자의 위치는?
	int l = x - cx/2;
	int t = y - cy/2;

	// 상자를 생성한다
	DSB::Create2( l, t, cxDSB, cyDSB, nTimeOut );

	// 이전에 있던 상자는 지운다
	if ( pDSB ) pDSB->Destroy();
}

// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
// 오버라이드 하여 구현해야 한다
// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
void DSay::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	// 이름 출력
	PutText( pDC, m_sWho, 0, 0, true, s_colYellow, s_tdShadeOutline );

	// 지난 글 출력
	for ( int j = 0; j < m_nLastLines; j++ )
			PutText( pDC, m_asLast[j], 1, -j+m_nLastLines, true,
				s_colLightGray, s_tdNormal );

	// 내용 출력
	for ( int i = 0; i < m_nLines; i++ )
		PutText( pDC, m_asLine[i], 1, 1+i+m_nLastLines+(m_nLastLines>0?0:1), true,
			s_colWhite, s_tdShade );
}
