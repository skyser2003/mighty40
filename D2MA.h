// D2MA.h: interface for the all 2MA-related DSB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_D2MA)
#define AFX_D2MA


/////////////////////////////////////////////////////////////////////////////
// 카드 고르기 DSB

class DSelect2MA  : public DSB
{
public:
	DSelect2MA( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DSelect2MA() {}

	// pcToKill : 이 DSB 가 결과 카드를 저장할 장소 (결과-값 독립변수)
	// plShow   : 보이는 (위에 있는) 카드
	// plHide   : 숨겨진 (아래에 있는) 카드
	void Create( CEvent* pEvent, int* selecting, CCard* pcShow );

protected:
	// 초기화 ( 생성된 후 호출됨 )
	virtual void OnInit();

	// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
	// 오버라이드 하여 구현해야 한다
	// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
	virtual void OnDraw(
		CDC* pDC, bool bBrushOrigin );

	// 핫 스팟을 클릭할 때 불리는 함수
	// LPVOID 값이 0 이면 종료 (X자를 클릭) 요청
	virtual void OnClick( LPVOID pVoid );

	// 확인 소리를 안나게
	virtual void OnClickSound() {}

protected:
	// 내부 데이터 & 함수

	// 선택될 카드 포인터
	int* m_pselecting;
	// 보이는 카드
	CCard* m_pcShow;
	// 보이는 카드를 그릴 영역을 리턴한다
	void CalcShowCardRect( LPRECT pRc );
	// 숨겨진 카드를 그릴 영역을 리턴한다
	void CalcHideCardRect( LPRECT pRc );
};

#endif // !defined(AFX_D2MA)