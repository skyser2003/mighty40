// BmpMan.h: interface for the CBmpMan class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BMPMAN_H__930A70A3_C49E_11D2_97F8_000000000000__INCLUDED_)
#define AFX_BMPMAN_H__930A70A3_C49E_11D2_97F8_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// 비트맵 관리자
// 비트맵과 관련된 모든 일을 한다 !

class CBmpMan  
{
public:
	CBmpMan();
	virtual ~CBmpMan();

public:
	// 로드 함수
	// 각 함수는 이전에 로드된 이미지를 삭제한다
	// 입력으로 주는 DC 는 생성될 비트맵의 기준이 되는 DC 이다
	// 실패시 false 리턴

	// 뒷그림을 제외한 모든 카드 이미지를 읽어 들인다
	// 이 때 폰트도 함께 세트한다
	// 이 폰트는 카드를 축소 & 확대하여 출력할 때 사용된다
	// 정상 크기의 카드에는 이 폰트를 기반한
	// 표준 크기의 폰트를 사용한다
	bool LoadAllCards( CDC* pDC, CFont* pFont );
	// 뒷그림을 읽는다
	// nIndex 는 뒷 그림 인덱스 (0~10),
	// nIndex == -1 일때 파일에서 읽는다
	bool LoadBackPicture( CDC* pDC, int nIndex, LPCTSTR sFileName = 0 );
	// 배경 그림을 읽는다
	bool LoadBackground( CDC* pDC, LPCTSTR sFileName );

public:
	// 그리기 함수

	// 카드 한 장을 그린다
	// nCard : int 로 치환한 카드
	void DrawCard( CDC* pDC, int nCard, int x, int y, int cx, int cy );
	void DrawCard( CDC* pDC, int nCard, const CRect& rc );
	void DrawCardEx( CDC* pDC, int nCard,
		int xTgt, int yTgt, int cxTgt, int cyTgt,
		int xSrc, int ySrc, int cxSrc, int cySrc );
	void DrawCardEx( CDC* pDC, int nCard,
		const CRect& rcTgt, const CRect& rcSrc );

	// 배경 그림을 그린다
	// 이 함수는 Invalid 영역에 대한 배경 그림 조각을
	// 그리기 위해서 사용된다
	void DrawBackground( bool bUseBackground,
		bool bTile, bool bExpand, COLORREF colBack,
		CDC* pDC, int x, int y, int cx, int cy,
		int xView, int yView, int xOffset = 0, int yOffset = 0 );
	void DrawBackground( bool bUseBackground,
		bool bTile, bool bExpand, COLORREF colBack,
		CDC* pDC, const CRect& rc,
		int xView, int yView, int xOffset = 0, int yOffset = 0 );

	// 음영을 그린다
	// bOrigin : 이 값이 true 이면 0,0 에서, false 이면 1,0 에서 패턴을 그린다 ^^
	void DrawShade( CDC* pDC, int x, int y, int cx, int cy, bool bOrigin );
	void DrawShade( CDC* pDC, const CRect& rc, bool bOrigin );

protected:
	// 내부적으로 사용되는 함수와 변수들

	// 모든 카드 그림을 일렬 횡대로 담고 있는
	// 커다란 비트맵 : 뒷면 비트맵까지 포함한다
	CBitmap m_bmCards;
	// 카드 크기
	CSize m_szCards;
	// 배경 비트맵
	CBitmap m_bmBackground;
	// 배경 비트맵 크기
	CSize m_szBackground;
	// 확대한 배경 비트맵 ( 배경 확대 모드에서 그림이 깨지는것을 방지 )
	CBitmap m_bmExpandedBackground;
	// 확대한 배경 비트맵 크기
	CSize m_szExpandedBackground;
	// 폰트
	CFont* m_pZoomFont;
	CFont m_font;
	// 그림자를 위한 브러쉬와 비트맵
	CBitmap m_bmShadow;
	CBrush m_brShadow;
};

#endif // !defined(AFX_BMPMAN_H__930A70A3_C49E_11D2_97F8_000000000000__INCLUDED_)
