// DSB.h: interface for the DSB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DSB_H__14D84F86_D8F9_11D2_982F_000000000000__INCLUDED_)
#define AFX_DSB_H__14D84F86_D8F9_11D2_982F_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBoardWrap;
class CBmpMan;

// Shade Box class
// 이 클래스는 CBoard 위에 떠 있는 음영 상자로,
// CBoard 에 등록해서 사용한다

class DSB  
{
public:
	DSB( CBoardWrap* pBoard );
protected:
	virtual ~DSB();

public:
	// 이 DSB 의 고유한 ID 를 리턴한다
	int GetID() const								{ return m_nID; }

	// 이 DSB 가 사라지면서 세트할 이벤트와,
	// 결과값 저장 주소를 설정한다
	void SetAction( CEvent* pEvent = 0, void* pResult = 0 )	{ m_pEvent = pEvent; m_pResult = pResult; }

	// DSB 의 기본 색성을 세트한다
	static void SetDefaultColor(
		COLORREF colText = RGB(255,255,255),
		COLORREF colStrong1 = RGB(0,255,255),
		COLORREF colStrong2 = RGB(255,255,0),
		COLORREF colGray = RGB(192,192,192) );

public:
	// 새로운 상자를 생성한다
	// CBoard 에 자신을 등록하고 새로 그린다
	// 위치 xDSB, yDSB 는 좌상단 모서리가 아니라
	// 스크린 중심에서 상자 중심이 어느정도 벗어났는가
	// 하는 값이다 !!!
	void Create(
		int xDSB, int yDSB,				// 위치 ( DSB 단위 )
		int cxDSB, int cyDSB,			// DSB 단위로 계산한 크기
		int nTimeOut );					// 수명 (밀리초:-1은 무한)

	// 새로운 상자를 생성한다
	// CBoard 에 자신을 등록하고 새로 그린다
	// x, y 는 DP 좌표
	void Create2(
		int x, int y,					// 위치
		int cxDSB, int cyDSB,			// DSB 단위로 계산한 크기
		int nTimeOut );					// 수명 (밀리초:-1은 무한)

	// 상자를 삭제하고 자신을 delete 한다
	// CBoard 에서 등록을 지우고 새로 그린다
	virtual void Destroy();

	// 이 상자는 뷰의 크기가 변해도
	// 크기가 변하지 않도록 만든다
	// ( Create 전에 이 함수를 호출해야 한다 )
	void SetFixed();

	// 이 상자는 modal 모드에서 동작한다
	void SetModal()									{ m_bModal = true; }
	bool IsModal()									{ return m_bModal; }
	// 이 상자는 Below 모드에서 동작한다
	void SetBelow()									{ m_bBelow = true; }
	bool IsBelow()									{ return m_bBelow; }

	// 이 상자의 실제 DP 사각형 좌표를 계산한다
	void GetRect( LPRECT prc );

public:
	// CBoard 가 호출하는 함수들

	// 음영을 그릴 Bitmap manager 를 설정해 준다
	// ( Create 내에서 호출됨 )
	void SetBmpMan( CBmpMan* pBM )					{ m_pBmpMan = pBM; }

	// 마우스로 이 사각형의 이 점(CBoard기준 절대좌표)을
	// 클릭하거나 지나갈 때 적절한 효과를 준다
	// 클릭(bCheckOnly==true)했다면 핫 스팟 이벤트를
	// 디스패치 하며, 마우스만 지나간다면(bCheckOnly==false)
	// 그 핫 스팟을 하이라이트 한다
	// 만일 어떤 핫 스팟에 걸려 있다면 참을,
	// 그렇지 않다면 거짓을 리턴한다
	virtual bool DispatchHotspot( int x, int y, bool bCheckOnly );

	// 타임아웃을 재지 않고 영구히 떠 있는 속성으로 만든다
	void SetPermanent()								{ m_nTimeOut = -1; }
	bool IsPermanent() const						{ return m_nTimeOut == -1; }

	// 타이머를 세트 ( SetPermanent 와 반대 함수 )
	void SetTimeout( int nTimeOut );

	// 타이머를 리턴 (millisecond)
	int GetTimeOut() const							{ return m_nTimeOut; }

	// 좌표를 이동시킨다
	void Offset( POINT pnt )						{ Offset(pnt.x,pnt.y); }
	void Offset( SIZE pnt )							{ Offset(pnt.cx,pnt.cy); }
	void Offset( int x, int y );

	// 특정 키에 대한 반응
	// ESC 를 누르면 x 버튼을 누른 효과를 낸다
	virtual void OnESC()							{ OnClick( (LPVOID)0 ); }
	// Enter 를 누르면 0xffffffff 를 누른 효과를 낸다
	virtual void OnEnter()							{ OnClick( (LPVOID)0xffffffff ); }

public:
	// 가상 함수들

	// 초기화 ( 생성된 후 CBoard 가 호출함 )
	// 계승된 DSB 들은 이 함수에서 Hotspot 들을 등록한다
	virtual void OnInit() {}

	// 이동되었을 때 CBoard 가 호출하는 함수
	virtual void OnMove() {}

	// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
	// 오버라이드 하여 구현해야 한다
	// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
	virtual void OnDraw(
		CDC* pDC,
		bool bBrushOrigin );

	// 포커스가 변경될 때 호출하는 함수
	// bFocus 가 참이면 포커스가 설정 된 것이며, 거짓이면 포커스가 해제된 것이다
	virtual void OnFocus( bool /*bFocus*/ ) {}

	// 핫 스팟을 클릭할 때 불리는 함수 ( 스스로 호출한다 )
	// LPVOID 값이 0 이면 종료 (X자를 클릭) 요청
protected:
	virtual void OnClick( LPVOID pVoid )			{ if ( !pVoid || pVoid == (LPVOID)0xffffffff ) Destroy(); }
	virtual void OnClickSound()						{ PlaySound( IDW_BEEP ); }

	// 핫 스팟이 Highlight 되거나 Normal 로 될 때 호출되는 함수
	// bHighlight 가 참이면 highlight 되었음
	struct HOTSPOT;
	virtual void OnHighlight( HOTSPOT& , bool /*bHighlight*/ ) {}

	// Destroy 될 때 불리는 함수
	virtual void OnDestroy()						{}

protected:
	// 초기화와 운영에 필요한 함수들

	// 텍스트 속성 정의 상수
#define		TD_NORMAL		0
#define		TD_SIZESMALL	0	// 크기 (소)
#define		TD_SIZEMIDIUM	1	// 크기 (중)
#define		TD_SIZEBIG		2	// 크기 (대)
#define		TD_SHADE		4	// 그림자 효과
#define		TD_OUTLINE		8	// 외곽선 (흐린 색깔로)
#define		TD_SUNKEN		16	// 비트맵이 움푹 패인 효과
#define		TD_OPAQUE		32	// 불투명한 글씨

	// 핫 스팟을 등록한다
	// 이 영역을 클릭하면 CBoard 는 지정된 값을 가지고
	// 이 클래스의 OnClick 을 호출한다
	// 위치는 상대적인 (Box의 좌상단이 0,0)
	// DSB 좌표
	// 리턴값은 hotspot 의 사각형 영역 상대적 DP 좌표
	RECT RegisterHotspot(
		int x, int y,					// 위치
		int cx, int cy,					// 크기 (-1,-1 이면 자동 계산)
		bool bDSB,						// 위치 좌표가 DSB 좌표인가
		UINT idBitmap, LPCTSTR s,		// 그릴 비트맵과 쓸 문자열 (없으면 각각 0 )
		const COLORREF* pCol,
		const int* pDeco,				// 문자열 색상과 장식 & 크기
		const COLORREF* pColSel,
		const int* pDecoSel,			// 마우스가 지나갈 때의 색상과 장식 & 크기
		LPVOID pVoid );					// OnClick 에 넘겨지는 값

	// v4.0: 핫 스팟을 제거한다 (2011.1.7)
	void DeleteHotspot( LPVOID pVoid );

	// 핫 스팟을 Update 하고 Invalidate 한다
	void UpdateHotspot( POSITION pos1, POSITION pos2 = 0 );

	// 주어진 pVoid 값을 가지는 핫 스팟을 찾는다
	POSITION FindHotspot( LPVOID pVoid );

	// 상대적인 DSB 좌표를 윈도우 디바이스 좌표로 변환한다
	POINT DSBtoDP( POINT ) const;
	RECT DSBtoDP( const RECT& ) const;

	// 뷰의 DSB unit 을 얻는다
	int GetDSBUnit() const;
	// 뷰의 폰트를 얻는다
	CFont* GetFont( int nSize );
	// 이 스트링의 화면에서의 크기를 구한다 (지정한 폰트 사용)
	CSize GetTextExtent( int nSize, LPCTSTR s );

protected:
	// 그리기 함수들

	// 글자를 쓴다
	// 리턴값은 그려진 범위 (절대DP)
	// 단, x 나 y 가 임의의 음수이고 bDSB 가 참이면
	// 가운데 정렬 되어 그려진다 !
	RECT PutText(
		CDC* pDC,
		LPCTSTR s,				// 쓸 문자
		int x, int y,			// 위치 (절대 DP)
		bool bDSB,				// x, y 가 상대적인 DSB 좌표인가
		COLORREF col,			// 색상
		int deco,				// 문자 장식 & 크기
		bool bCheckOnly = false );	// 그리지는 않고 단지 범위만을 계산해서 리턴한다

	// 비트맵을 그리고 그 우측에 문자를 쓴다
	// 리턴값은 그려진 범위
	// 단, x 나 y 가 임의의 음수이고 bDSB 가 참이면
	// 가운데 정렬 되어 그려진다 !
	RECT PutBitmap(
		CDC* pDC,
		UINT id,				// 그릴 비트맵
		LPCTSTR s,				// 비트맵 우측에 쓰일 글자
		int x, int y,			// 위치
		bool bDSB,				// x, y 가 DSB 좌표인가
		COLORREF col,			// 색상
		int deco,				// 문자 장식 & 크기
		bool bCheckOnly = false );	// 그리지는 않고 단지 범위만을 계산해서 리턴한다

	// 배경을 그린다
	// nDSBShadeMode : 칠하기 모드 ( 1 : 불투명 2 : 블렌드  그외 : 병치 )
	static void FillBack( CDC* pDC, LPCRECT prc,
		CBmpMan* pBmpMan, int nDSBShadeMode, bool bBrushOrigin );

	// 긴 스트링을 몇 줄로 자른다
	// cLine     : 각 줄의 대략적인 길이 (DSB 단위)
	// asLine    : 0 이 아니면 여기에 각 부분 스트링을 복사한다
	// s         : 자를 긴 스트링
	// nMaxWidth : 실제로 필요한 각 라인의 최대 버퍼 크기
	// 리턴값 -1 은 에러, 그밖에는, 실제로 필요한 라인 수
	int ParseString( LPTSTR* asLine, int cLine,
					LPCTSTR s, size_t& nMaxWidth );

protected:
	CPoint m_pntOffset;
	CRect m_rcDSB;
	CBoardWrap* m_pBoard;
	CBmpMan* m_pBmpMan;
	int m_nTimeOut;
	bool m_bModal;
	bool m_bBelow;
	POSITION m_posSel;	// 선택되어 있는 핫 스팟
	struct HOTSPOT {
		CRect rc;		// 범위 (전체를 10000,10000 으로 했을 때의 영역)
		UINT idBitmap;	// 그릴 비트맵
		CString s;		// 그릴 문자열
		const COLORREF* pCol;	// 문자 색상
		const COLORREF* pColSel;// 마우스가 지나갈 때의 문자 색상
		const int* pDeco;		// 문자 장식 & 크기
		const int* pDecoSel;	// 마우스가 지나갈 때의 문자 장식 & 크기
		LPVOID pVoid;	// 넘겨 주는 값
	};
	CList<HOTSPOT,HOTSPOT&> m_lHotspot;

	bool m_bFixed;

	int m_nID;

	CEvent* m_pEvent;
	void* m_pResult;

protected:
	// 고유 ID 리스트
	static CList<int,int> s_lID;
	// 포인터 리스트
	static CList<DSB*,DSB*> s_lPtr;
	// 위의 두 전역 변수의 CS
	static CCriticalSection s_csDSB;
	// s_lPtr 에 삭제되지 않고 남은 모든 DSB 를 지우기 위한
	// 헬퍼 클래스
	static struct GarbageCollector {
		virtual ~GarbageCollector() {
			s_csDSB.Lock();
			for ( POSITION pos = s_lPtr.GetHeadPosition(); pos; )
				delete s_lPtr.GetNext(pos);
			s_lPtr.RemoveAll();
			s_lID.RemoveAll();
			s_csDSB.Unlock();
		}
	} s_gc;

	friend struct DSB::GarbageCollector;

	// 미리 정의된 색상 & 효과들
	static COLORREF s_colWhite;
	static COLORREF s_colCyan;
	static COLORREF s_colYellow;
	static COLORREF s_colGray;
	static COLORREF s_colLightGray;
	static bool s_bColorLoaded;
	static const int s_tdNormal;
	static const int s_tdShade;
	static const int s_tdOutline;
	static const int s_tdShadeOutline;
	static const int s_tdShadeOpaque;
	static const int s_tdOutlineOpaque;
	static const int s_tdShadeOutlineOpaque;
	static const int s_tdMidium;
	static const int s_tdMidiumShade;
	static const int s_tdMidiumOutline;
	static const int s_tdMidiumShadeOutline;
};

#endif // !defined(AFX_DSB_H__14D84F86_D8F9_11D2_982F_000000000000__INCLUDED_)
