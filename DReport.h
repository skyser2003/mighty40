// DReport.h: interface for the DStartUp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DREPORT_H__4643EA36_DCDA_11D2_9840_0000212035B8__INCLUDED_)
#define AFX_DREPORT_H__4643EA36_DCDA_11D2_9840_0000212035B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMFSM;

class DReport : public DSB  
{
public:
	DReport( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DReport() {}

	void Create( const CMFSM* pMFSM, bool* pbCont, CEvent* pEvent );

protected:
	// 초기화 ( 생성된 후 호출됨 )
	virtual void OnInit();

	// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
	// 오버라이드 하여 구현해야 한다
	// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
	virtual void OnDraw(
		CDC* pDC,
		bool bBrushOrigin );

	// 핫 스팟을 클릭할 때 불리는 함수
	// LPVOID 값이 0 이면 종료 (X자를 클릭) 요청
	virtual void OnClick( LPVOID pVoid );

protected:

	void SetPage( int nPage ) {
		m_nPage = nPage;
		for ( int i = 0; i < 4; i++ ) {
			m_acolPage[i] = i == nPage ? s_colCyan : s_colWhite;
			m_atdPage[i] = i == nPage ? s_tdOutline : s_tdNormal;
		}
	}

	COLORREF m_acolPage[4];
	int m_atdPage[4];

	int m_nPage;

	const CMFSM* m_pMFSM;

	bool m_bDefWin;
	bool m_bWin;
	bool m_bGameOver;
	bool m_bNetworkGame;
	int m_nRank;
	int m_nMe;
	int m_nDefPointed;
	int m_aanHistory[5][MAX_PLAYERS];
	CString m_asCalcMethod[2];
	CString m_asExpr[MAX_PLAYERS];
	int m_anMoneyBefore[MAX_PLAYERS];
	int m_anMoneyAfter[MAX_PLAYERS];
};

#endif // !defined(AFX_DSTARTUP_H__4643EA36_DCDA_11D2_9840_0000212035B8__INCLUDED_)
