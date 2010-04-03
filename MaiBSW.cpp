// MaiBSW.cpp: implementation of the CMaiBSW class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "mighty.h"

#include "Play.h"
#include "Player.h"

#include "MaiBSW.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// Black Sheep Wall Algorithm





// 기루다를 임의로 세트할 수 있는 기능이 제공되는 카드 리스트

class CMaiBSWCardList : public CCardList  
{
public:
	int m_nKiruda;
	CMaiBSWCardList( int nKiruda = CCard::GetKiruda() ) : m_nKiruda(nKiruda)	{}
	CMaiBSWCardList( const CCardList& cl ) : m_nKiruda(CCard::GetKiruda()) { *((CCardList*)this) = cl; }

public:
	virtual bool IsKiruda( CCard c ) const
	{	return SPADE <= m_nKiruda && m_nKiruda <= CLOVER
			&& c.GetShape() == m_nKiruda; }
};


// 벌점 테이블 구현

class CMaiBSWPenaltyTable
{
public:
	CMaiBSWPenaltyTable();

	// 벌점을 계산한다
	int CalcPenalty(
		int nKiruda,
		const CState* pState,		// 유틸리티 계산을 위한 상태 정보
		int nPlayer,				// 누가 계산하고 있는가
		const CCardList& lc,		// 계산해야 할 카드 리스트
		bool bJokercallEffect,		// 조커콜 효력중인가
		int nJokerShape,			// 조커 모양
		const double adDefProb[] = 0 // 각 플레이어의 여당 확률 0.0~1.0
	) const;

protected:
	// 이 카드의 소모 벌점을 구한다
	int CalcUsingPenalty( int nKiruda, CCard c,
		const long anUsedCards[4], bool bTheyHaveJoker ) const;
	// 이 카드를 살해시켰을 때의 벌점을 구한다
	int CalcKillingPenalty( int nKiruda, CCard c,
		const long anUsedCards[4], bool bTheyHaveJoker ) const;
	// 여당이 이만큼을 잃었을 때 생기는 벌점을 구한다
	int CalcDefLostPenalty( double dRemain, double dLostScore ) const;
	// 야당이 이만큼을 땄을 때 생기는 벌점을 구한다
	int CalcAttGainPenalty( double dGainScore ) const;

	// 벌점 테이블
	int m_nUSK;		// Using 1st Score Kiruda
	int m_nUSK_d;	//       diff
	int m_nUNK;		// Using 1st Non-Score Kiruda
	int m_nUNK_d;	//       diff
	int m_nUSN;		// Using 1st Score Normal Card
	int m_nUSN_d;	//       diff
	int m_nUNN;		// Using 1st Non-Score Normal Card
	int m_nUNN_d;	//       diff
	int m_nUM;		// Using Mighty
	int m_nUJ;		// Using Joker
	int m_nUSJ;		// Using Safe Joker ( Free for Joker Call )
	int m_nULJ;		// Using Last Joker ( Free for Joker Call and Mighty )
	int m_nUEJC;	// Using Effective Jokercall
	int m_nDL30;	// D, Lost 0% ~ 30%
	int m_nDL45;	// D, Lost 30% ~ 45%
	int m_nDL60;	// D, Lost 45% ~ 60%
	int m_nDL75;	// D, Lost 60% ~ 75%
	int m_nDL100;	// D, Lost 75% ~ 100%
	int m_nDLOver;	// D, Lost all
	int m_nAS1;		// A, Score 1 pt
	int m_nAS2;		// A, Score 2 pt
	int m_nAS3;		// A, Score 3 pt
	int m_nAS4;		// A, Score 4 pt
	int m_nAS5;		// A, Score 5 pt
};


// 주공 성향 테이블
// 정적으로 테이블을 관리하는 버전

class CMaiBSWPrideTable
{
public:
	CMaiBSWPrideTable();

	// 상점을 계산한다
	int CalcPride( int nKiruda, const CCardList& lc ) const;

	// 상점에서 목표 점수를 계산한다
	// nPrideFac : 0 ~ 10 으로, 주공이 되려는 성향도를 결정한다
	int PrideToMinScore( int nPride, int nPrideFac = 5 ) const;

protected:
	// 사실상 같은 카드를 정말로 같은 카드로 바꾼다
	void Refine( CCardList* pList ) const;

	// 상점 테이블

	// bias
	int m_nBias;
	// 기루다 개수의 중요도
	int m_nKirudaCountWeight;
	// 기루다 A,K,Q,10 의 중요도
	int m_nKirudaAWeight;
	int m_nKirudaKWeight;
	int m_nKirudaQWeight;
	int m_nKirudaJWeight;
	// 비기루다 K, Q, J, 10 카드의 중요도
	int m_nEtcKWeight;
	int m_nEtcQWeight;
	int m_nEtcJWeight;
	int m_nEtc10Weight;
	// 빈 모양에 의한 가중치
	int m_nEmptyWeight;
	// Mighty 소유의 중요도
	int m_nMightyWeight;
	// Joker 소유의 중요도
	int m_nJokerWeight;
	// Jokercall 소유의 중요도(조커 없을때)
	int m_nJokercallWeight;
};


// BSW algorithm

class CMaiBSWAlgo  
{
public:
	CMaiBSWAlgo( const CMaiBSWPenaltyTable* pPnT, const CMaiBSWPrideTable* pPrT,
		MAIDLL_UPDATE* pUpdate )
		: m_pPenaltyTable(pPnT), m_pPrideTable(pPrT), m_pUpdate(pUpdate) {}

	// 주어진 13 장의 카드에 대해서
	// 공약을 정하고 버릴 3 장의 카드를 리턴한다
	void Goal( int* pnKiruda, int* pnMinScore, CCard acDrop[3],
		const CCardList* pHand,
		int nPrideFac = 5,	// 0 ~ 10 까지, 주공 되려는 성향
		const CCardList* pDeck = 0 );

	// 프랜드 선택 (필요한 모든 인자는 CCard::CState 에서 얻는다 )
	int Friend( int nKiruda, const CCardList* pHand, const CCardList* pDeck = 0 ) const;

	// 죽일 카드 선택
	CCard Kill( const CCardList* pHand, const CCardList* pTillNow ) const;

	// 카드 내기 (필요한 모든 인자는 CCard::CState 에서 얻는다 )
	CCard Turn( int& eff, bool bUseSimulation ) const;

protected:
	// 각 테이블
	const CMaiBSWPenaltyTable* m_pPenaltyTable;
	const CMaiBSWPrideTable* m_pPrideTable;
	// 상태 바 Update 객체
	MAIDLL_UPDATE* m_pUpdate;

	// 같은 능력의 카드를 리턴
	CCard GetEqualCard( CCard c ) const;
	// 재귀 반복 알고리즘으로 카드 내기
	CCard TurnIteration( int& eff ) const;
	// 시물레이션 알고리즘으로 카드 내기
	CCard TurnSimulation( int& eff ) const;
};


// Mai wrapping object

class CMaiBSWWrap : public CPlay  
{
public:
	CMaiBSWWrap( LPCTSTR sOption, MAIDLL_UPDATE* pUpdate );
	virtual ~CMaiBSWWrap();

	// 옵션 얻기
	CString GetOption() const						{ return m_sOption; }
	// 옵션 세트
	void SetOption( HWND hWnd );

	// CPlay 인터페이스 구현

	// 하나의 게임이 시작됨
	// 이 게임에서 사용되는 상태 변수를 알려준다
	// 이 상태 변수는 CCard::GetState() 로도 알 수 있다
	// pState->nCurrentPlayer 값이 바로 자기 자신의
	// 번호이며, 이 값은 자신을 인식하는데 사용된다
	virtual void OnBegin( const CState* pState );

	// 6마에서 당선된 경우 한 사람을 죽여야 한다
	// 죽일 카드를 지정하면 된다 - 단 이 함수는
	// 반복적으로 호출될 수 있다 - 이 경우
	// CCardList 에 지금까지 실패한 카드의 리스트가
	// 누적되어 호출된다
	// 5번 실패하면 (이 경우 알고리즘이 잘못되었거나
	// 사람이 잘 못 선택하는 경우) 임의로 나머지 5명 중
	// 하나가 죽는다 !
	virtual void OnKillOneFromSix(
		CCard* pcCardToKill, CCardList* plcFailedCardsTillNow );
	// 7마에서 당선된 경우 두 사람을 죽여야 한다
	// 이 함수는 그 중 하나만 죽이는 함수로,
	// OnKillOneFromSix와 같다.
	// 5번 실패하면 (이 경우 알고리즘이 잘못되었거나
	// 사람이 잘 못 선택하는 경우) 임의로 나머지 5명 중
	// 하나가 죽는다 !
	virtual void OnKillOneFromSeven(
		CCard* pcCardToKill, CCardList* plcFailedCardsTillNow );

	// 공약을 듣는다
	// pNewGoal 과 state.goal 에 현재까지의 공약이 기록되어 있다
	// 패스하려면 nMinScore 를 현재 공약보다 작은 값으로 세트
	// pNewGoal.nFriend 는 사용하지 않는다
	virtual void OnElection( CGoal* pNewGoal );

	// 당선된 경우
	// pNewGoal 과 state.goal 에 현재까지의 공약이 기록되어 있다
	// pNewGoal 에 새로운 공약을 리턴하고 (nFriend포함)
	// acDrop 에 버릴 세 장의 카드를 리턴한다
	virtual void OnElected( CGoal* pNewGoal, CCard acDrop[3] );

	// 선거가 끝났고 본 게임이 시작되었음을 알린다
	virtual void OnElectionEnd();

	// 카드를 낸다
	// pc 는 낼 카드, pe 는 pc가 조커인경우 카드 모양,
	// pc가 조커콜인경우 실제 조커콜 여부(0:조커콜 아님)
	// 그 외에는 무시
	virtual void OnTurn( CCard* pc, int* pe );

	// 한 턴이 끝났음을 알린다
	// 아직 state.plCurrent 등은 그대로 남아있다
	virtual void OnTurnEnd();

protected:
	// 손에 든 카드를 구한다
	const CCardList* GetHand() const;

	CString m_sOption;
	MAIDLL_UPDATE* m_pUpdate;
	const CState* m_pState;
	int m_nPrideFac;	// 주공 성향
	bool m_bGoalMode;	// 목표점수를 한번에 말하는가
	bool m_bUseSimulation;	// 확장 알고리즘을 사용하는다

	CMaiBSWPenaltyTable* m_pPenaltyTable;
	CMaiBSWPrideTable* m_pPrideTable;
	CMaiBSWAlgo* m_pBSW;
};


// CMaiBSWSetting

class CMaiBSWSetting : public CDialog
{
// Construction
public:
	CMaiBSWSetting(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMaiBSWSetting)
	enum { IDD = IDD_SETTING };
	int		m_nFactor;
	BOOL	m_bGoalMode;
	BOOL	m_bUseSimulation;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMaiBSWSetting)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMaiBSWSetting)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// 4 개의 DLL 에물레이션 함수

// DLL 정보를 얻는다
void CMaiBSW::MaiGetInfo( MAIDLL_INFO* pInfo )
{
	pInfo->sName = _T("'검은 양떼 벽' 알고리즘 (V2.17)");
	pInfo->sExplain = _T("미리 정의된 '벌점 테이블'을 바탕으로, "
						"가능한 모든 경우에 대한 벌점을 계산하여 "
						"이 중 가장 적은 벌점을 가지는 해를 구한다 : "
						"일종의 'Greedy' 알고리즘으로, "
						"이론적으로는 최적 해를 구하지만 "
						"지나친 지레 짐작이 늘 좋은 것만은 아니라는 "
						"사실을 새삼 까닫게 된다 : 주의: "
						"이 알고리즘은 다른 사람의 패를 모두 "
						"읽을 수 있다 !");
}

// 옵션 스트링을 가지고 CPlay 객체를 얻는다
// 단, sOption 이 NULL 일 때는 디폴트 값
CPlay* CMaiBSW::MaiGetPlay( LPCTSTR sOption, MAIDLL_UPDATE* pUpdate )
{
	return new CMaiBSWWrap( sOption, pUpdate );
}

// DLL 정보를 얻는다
// 이 객체에 대한 옵션 다이얼로그 박스를 출력하고
// 옵션을 설정한다
// 설정할 옵션이 없는 AI 인 경우 이 함수는 생략되어도 좋다
// hWnd : 다이얼로그 박스의 부모 윈도우
void CMaiBSW::MaiOption( CPlay* pPlay, HWND hWnd )
{
	((CMaiBSWWrap*)pPlay)->SetOption( hWnd );
}

// CPlay 객체를 되돌려준다
// 옵션 스트링을 리턴함
void CMaiBSW::MaiReleasePlay( CPlay* pPlay, CString* psOption )
{
	*psOption = ((CMaiBSWWrap*)pPlay)->GetOption();
	delete pPlay;
}


/////////////////////////////////////////////////////////////////////////////

static CCard GetJokercall( int nKiruda )
{
	CCard cOri = CCard::GetState()->pRule->nJokercall;
	CCard cAlt = CCard::GetState()->pRule->nAlterJokercall;
	return nKiruda == cOri.GetShape()	// 기루다와 겹침
		? cAlt : cOri;
}

static CCard GetMighty( int nKiruda )
{
	CCard cOri = CCard::GetState()->pRule->nMighty;
	CCard cAlt = CCard::GetState()->pRule->nAlterMighty;
	return nKiruda == cOri.GetShape()	// 기루다와 겹침
		? cAlt : cOri;
}

static bool IsUsed( CCard c, const long anUsedCards[] )
{
	return anUsedCards[c.GetShape()-SPADE]
		& ( 1 << (c.GetPowerNum()-1) ) ? true : false;
}

static bool IsJokercallUsed( int nKiruda, const long anUsedCards[] )
{
	return IsUsed( GetJokercall(nKiruda), anUsedCards );
}

static bool IsMightyUsed( int nKiruda, const long anUsedCards[] )
{
	return IsUsed( GetMighty(nKiruda), anUsedCards );
}

static bool IsKiruda( CCard c, int nKiruda )
{
	return nKiruda && c.GetShape() == nKiruda;
}

static bool IsMighty( CCard c, int nKiruda )
{
	return GetMighty(nKiruda) == c;
}


CMaiBSWPenaltyTable::CMaiBSWPenaltyTable()
{
	// 벌점 테이블
	m_nUSK = 500;
	m_nUSK_d = -50;
	m_nUNK = 200;
	m_nUNK_d = -20;
	m_nUSN = 100;
	m_nUSN_d = -20;
	m_nUNN = 10;
	m_nUNN_d = -1;
	m_nUM = 1999;
	m_nUJ = 1997;
	m_nUSJ = 1998;
	m_nULJ = 2000;
	m_nUEJC = 300;
	m_nDL30 = 200;
	m_nDL45 = 350;
	m_nDL60 = 1000;
	m_nDL75 = 2500;
	m_nDL100 = 5000;
	m_nDLOver = 100000000;
	m_nAS1 = -400;
	m_nAS2 = -500;
	m_nAS3 = -3000;
	m_nAS4 = -5000;
	m_nAS5 = -100000000;
}

// 이 카드의 소모 벌점을 구한다
int CMaiBSWPenaltyTable::CalcUsingPenalty( int nKiruda, CCard c,
	const long anUsedCards[4], bool bTheyHaveJoker ) const
{
	if ( IsMighty(c,nKiruda) ) return m_nUM;

	if ( c.IsJoker() ) {

		if ( !IsJokercallUsed(nKiruda,anUsedCards) ) return m_nUJ;
		else if ( IsMightyUsed(nKiruda,anUsedCards) ) return m_nULJ;
		else return m_nUSJ;
	}
	if ( c.IsJokercall() ) {

		if ( !bTheyHaveJoker ) return m_nUEJC;
	}

	int nShape = c.GetShape()-SPADE;
	int nNumber = c.GetPowerNum()-1;

	int nOrder = 0;

	for ( int i = c.IsPoint() ? 12 : 7; i > nNumber; i-- )
		nOrder += anUsedCards[nShape] & (1<<i) ? 1 : 0;

	return IsKiruda(c,nKiruda)
			? ( c.IsPoint() ? m_nUSK + m_nUSK_d*nOrder
						: m_nUNK + m_nUNK_d*nOrder )
			: ( c.IsPoint() ? m_nUSN + m_nUSN_d*nOrder
						: m_nUNN + m_nUNN_d*nOrder );
}

// 이 카드를 살해시켰을 때의 벌점을 구한다
int CMaiBSWPenaltyTable::CalcKillingPenalty( int nKiruda, CCard c,
	const long anUsedCards[], bool bTheyHaveJoker ) const
{
	return -CalcUsingPenalty( nKiruda, c, anUsedCards, bTheyHaveJoker )/2;
}

// 여당이 이만큼을 잃었을 때 생기는 벌점을 구한다
int CMaiBSWPenaltyTable::CalcDefLostPenalty( double dRemain, double dLostScore ) const
{
	// 잃지 않았을때 0
	if ( dLostScore < 0.001 ) return 0;

	if ( dRemain < 0.0 ) // 이미 졌음
		return m_nDL60; // 그냥 60% 값을 리턴
	else if ( dRemain < 0.001 || dRemain < dLostScore )
		// remain 값이 0 이다
		return m_nDLOver;
	else if ( dLostScore / dRemain < 0.3 + 0.001 )
		return m_nDL30;
	else if ( dLostScore / dRemain < 0.45 + 0.001 )
		return m_nDL45;
	else if ( dLostScore / dRemain < 0.60 + 0.001 )
		return m_nDL60;
	else if ( dLostScore / dRemain < 0.75 + 0.001 )
		return m_nDL75;
	else return m_nDL100;
}

// 야당이 이만큼을 땄을 때 생기는 벌점을 구한다
int CMaiBSWPenaltyTable::CalcAttGainPenalty( double dGainScore ) const
{
	if ( dGainScore < 0.001 ) return 0;
	else if ( dGainScore < 1.001 ) return m_nAS1;
	else if ( dGainScore < 2.001 ) return m_nAS2;
	else if ( dGainScore < 3.001 ) return m_nAS3;
	else if ( dGainScore < 4.001 ) return m_nAS4;
	else return m_nAS5;
}

// 벌점을 계산한다
int CMaiBSWPenaltyTable::CalcPenalty(
	int nKiruda,
	const CState* pState,		// 유틸리티 계산을 위한 상태 정보
	int nPlayer,				// 누가 계산하고 있는가
	const CCardList& lc,		// 계산해야 할 카드 리스트
	bool bJokercallEffect,		// 조커콜 효력중인가
	int nJokerShape,			// 조커 모양
	const double adDefProb[]	// 각 플레이어의 여당 확률 0.0~1.0
) const
{
	// 여당이 조커를 가졌을까? (마이티 프랜드가 아니면 조커를 가진것으로 봄)
	bool bDefHasJoker = pState->goal.nFriend != (int)GetMighty(nKiruda);

	// 확률 정보가 없는 경우 다른 플레이어 정보를 힐끔 봄
	double adProb2[MAX_PLAYERS];
	if ( !adDefProb ) {
		adDefProb = adProb2;
		for ( int i = 0; i < pState->nPlayers; i++ )
			adProb2[i] = pState->IsDefender(i) ? 1.0 : 0.0;
	}
	// pState->anUsedCards 와 pState->anUsingCards 를 OR 한
	// 정보가 필요하다
	long anUsedCards[4];
	{	for ( int i = 0; i < 4; i++ )
			anUsedCards[i] = pState->anUsedCards[i] | pState->anUsingCards[i];
	}

	// 자신이 여당, 야당일 확률
	double dMyProbDef = adDefProb[nPlayer];
	double dMyProbAtt = 1.0 - dMyProbDef;

	// 승자, 승자가 여당, 야당일 확률, 승자가 따는 카드수
	int nWinner = pState->GetWinner( &lc, 0,
		(int)bJokercallEffect, nJokerShape );
	double dWinnerProbDef = adDefProb[nWinner];
	double dWinnerProbAtt = 1.0 - dWinnerProbDef;
	int nPoint = lc.GetPoint();

	// 자신이 여당일 때의 벌점
	double dDefPenalty = 0.0;
	{
		// 각 카드에 대한 소모 & 살해의 벌점을 구한다
		POSITION pos = lc.GetHeadPosition();
		for ( int i = 0; pos; i++ ) {
			int n = (pState->nBeginer+i) % pState->nPlayers;
			CCard c = lc.GetNext(pos);
			// 카드 주인이 여당이면 소모, 야당이면 살해 벌점이므로
			// 기대값을 구한다
			dDefPenalty
				+= (double)CalcUsingPenalty( nKiruda, c, anUsedCards, !bDefHasJoker )
					* adDefProb[n]
				+ (double)CalcKillingPenalty( nKiruda, c, anUsedCards, !bDefHasJoker )
					* (1.0-adDefProb[n]);
		}

		// 여당의 남은 여유분 점수를 구한다
		double dRemained;
		{
			dRemained = (double) max( 0, 20 - pState->goal.nMinScore
				- pState->pRule->bAttScoreThrownPoints ? pState->nThrownPoints : 0 );
			for ( int i = 0; i < pState->nPlayers; i++ )
				dRemained -= (1.0-adDefProb[i])
					* (double)pState->apPlayers[i]
						->GetScore()->GetCount();
		}

		// 잃는 점수에 대한 벌점
		dDefPenalty += dWinnerProbAtt
			* (double)CalcDefLostPenalty( dRemained, (double)nPoint );
	}

	// 자신이 야당일 때의 벌점
	double dAttPenalty = 0.0;
	{
		// 각 카드에 대한 소모 & 살해의 벌점을 구한다
		POSITION pos = lc.GetHeadPosition();
		for ( int i = 0; pos; i++ ) {
			int n = (pState->nBeginer+i) % pState->nPlayers;
			CCard c = lc.GetNext(pos);
			// 카드 주인이 야당이면 소모, 여당이면 살해 벌점이므로
			// 기대값을 구한다
			dAttPenalty
				+= (double)CalcUsingPenalty( nKiruda, c, anUsedCards, bDefHasJoker )
					* (1.0-adDefProb[n])
				+ (double)CalcKillingPenalty( nKiruda, c, anUsedCards, bDefHasJoker )
					* adDefProb[n];
		}

		// 따는 점수에 대한 벌점
		dAttPenalty += dWinnerProbAtt
			* (double)CalcAttGainPenalty( (double)nPoint );
	}

	return (int)( dMyProbDef * dDefPenalty
				+ dMyProbAtt * dAttPenalty );
}


/////////////////////////////////////////////////////////////////////////////
// CMaiBSWPrideTable

CMaiBSWPrideTable::CMaiBSWPrideTable()
{
	// 4 장 + 기 A, {K,Q} 정도 있으면 13 은 부르도록 한다
	// Bias 에서 기루다에 의한 + 가 비기루다에 의한 - 와
	// 수평을 이루도록 한다
	// 또, 조커가 없으면 18 이상을 부르지 못하게 한다

	// bias (기루다 40%,kirudable)
	m_nBias = 1500;

	// (기루다 비율 - 40% ~ 60%) 의 중요도
	m_nKirudaCountWeight = 300;
	// 기루다 A,K,Q,J,10 의 중요도 (K부터는 누적)
	m_nKirudaAWeight = 350;
	m_nKirudaKWeight = 100;
	m_nKirudaQWeight = 40;
	m_nKirudaJWeight = 30;
	// 비기루다 K, Q, J, 10 카드로 인해 감소되는 중요도
	m_nEtcKWeight = 50;
	m_nEtcQWeight = 60;
	m_nEtcJWeight = 70;
	m_nEtc10Weight = 80;
	// 빈 모양에 의한 가중치
	m_nEmptyWeight = 30;
	// Mighty 소유의 중요도 (주공은 Mighty 를 가질 필요는 없음)
	m_nMightyWeight = 100;
	// Joker 소유의 중요도
	m_nJokerWeight = 200;
	// Jokercall 소유의 중요도 (조커 없을때)
	m_nJokercallWeight = 100;
}

// 상점을 계산한다 ( CCardList 의 기루다는 세트되어 있다고 가정 )
int CMaiBSWPrideTable::CalcPride( int nKiruda, const CCardList& lc ) const
{
	POSITION pos;

	CMaiBSWCardList lCard = lc;
	lCard.m_nKiruda = nKiruda;

	ASSERT( lCard.GetCount() <= 10 );

	int nPride = 0;

	// 특별 카드 상점부터 계산 (Refine 에 의해 정보가 소실되기 전에)

	if ( !!lCard.Find( GetMighty(nKiruda) ) )
		nPride += m_nMightyWeight;

	bool bHasJoker;
	if ( false != ( bHasJoker = !!lCard.Find( CCard::GetJoker() ) ) )
		nPride += m_nJokerWeight;

	// 조커나 마이티가 있다면 조커콜은 소용 없다
	if ( !bHasJoker && !!lCard.Find( GetJokercall(nKiruda) )
		&& !lCard.Find( GetMighty(nKiruda) ) )
		nPride += m_nJokercallWeight;

	// 기루다 비율

	if ( nKiruda ) {

		int nKirudaCount = lCard.GetCountOf( nKiruda );
		int nAllCount = lCard.GetCount();
		int ratio = 100 * nKirudaCount / nAllCount;
		if ( ratio >= 60 ) nPride += m_nKirudaCountWeight;
		else if ( ratio >= 40 )
			nPride += ( ratio - 40 ) * m_nKirudaCountWeight / 20;

		if ( ratio >= 40 && lCard.GetKirudable() == nKiruda )
			nPride += m_nBias;

		// 상위 기루다에 의한 감소
		int d = 0;
		if ( !lCard.Find( CCard( nKiruda, ACE ) ) )
			nPride -= m_nKirudaAWeight;
		if ( !lCard.Find( CCard( nKiruda, KING ) ) )
			d += m_nKirudaKWeight,
			nPride -= d;
		if ( !lCard.Find( CCard( nKiruda, QUEEN ) ) )
			d += m_nKirudaQWeight,
			nPride -= d;
		if ( !lCard.Find( CCard( nKiruda, JACK ) ) )
			d += m_nKirudaJWeight,
			nPride -= d;
	}

	// 비기루다 카드 계산

	// ACE 의 가치를 갖지 못한 카드들 (Refine 하여 Ace 가
	// 되지 못한 카드들) 에 대해 각각, ACE 와의 거리로 감점한다

	// 계산하기 위해 같은 효력을 갖는 카드를 진짜로 같게 만든다
	Refine( &lCard );

	int nDem = 0;
	bool abEmpty[] = { true, true, true, true };

	pos = lCard.GetHeadPosition();
	while (pos) {
		CCard c = lCard.GetNext(pos);
		if ( c.IsJoker()
			|| c == GetJokercall(nKiruda)
				&& !lCard.Find(CCard::GetJoker())
				&& !lCard.Find(GetMighty(nKiruda))
			|| c == GetMighty(nKiruda)
			|| c.GetShape() == nKiruda
			|| c.IsAce()
			|| c.GetNum() == KING
				&& c.GetShape() == GetMighty(nKiruda).GetShape() )
			continue;

		abEmpty[c.GetShape()-1] = false;

		int d = 13 - c.GetPowerNum();
		if ( d == 1 ) nDem -= m_nEtcKWeight;
		else if ( d == 2 ) nDem -= m_nEtcQWeight;
		else if ( d == 3 ) nDem -= m_nEtcJWeight;
		else nDem -= m_nEtc10Weight - 4 + d;
	}

	// 빈 모양이 있는 경우 가중치가 부여된다
	for ( int s = 0; s < 4; s++ )
		if ( abEmpty[s] ) nDem += m_nEmptyWeight;

	// 2 개 정도는 프랜드가 막는다고 생각한다
	if ( nDem < -2*(m_nEtc10Weight+12) )
		nDem += 2*(m_nEtc10Weight+12);

	if ( nKiruda )
		nPride += nDem;
	else {
		// 노기루다! (기루다 관련 가중치는 etc 가중치로 대체한다
		nPride += m_nBias + m_nKirudaCountWeight;
		nPride += nDem * 3;
	}

	if ( lCard.GetCount() == 8 )
		nPride = nPride * 10 / 8;

	return nPride;
}

// 사실상 같은 카드를 정말로 같은 카드로 바꾼다
void CMaiBSWPrideTable::Refine( CCardList* pList ) const
{
	pList->Sort( true, false );	// left kiruda, right ace

	CCard cBefore;
	int nCurShape = 0;
	bool bCurScore = false;
	int nCurPNum = -1;

	POSITION pos = pList->GetTailPosition();
	for ( ; pos; pList->GetPrev(pos) ) {
		CCard c = pList->GetAt(pos);

		if ( nCurShape != c.GetShape() ) {
			nCurShape = c.GetShape();
			bCurScore = c.IsPoint();
			nCurPNum = c.GetPowerNum();
			cBefore = c;
		}
		else if ( bCurScore != c.IsPoint() ) {
			bCurScore = c.IsPoint();
			nCurPNum = c.GetPowerNum();
			cBefore = c;
		}
		else if ( nCurPNum - 1 != c.GetPowerNum() ) {
			nCurPNum = c.GetPowerNum();
			cBefore = c;
		}
		else {
			ASSERT( (int)cBefore );
			pList->SetAt( pos, cBefore );
			nCurPNum = c.GetPowerNum();
		}
	}
}

// 상점에서 목표 점수를 계산한다
// nPrideFac : 0 ~ 10 으로, 주공이 되려는 성향도를 결정한다
int CMaiBSWPrideTable::PrideToMinScore( int nPride, int nPrideFac ) const
{
	// -4 ~ +4 까지 factor 가 영향을 주도록 하자
	// 18 까지는 영향을 주지 않도록 한다
	int nRealPride;

	if ( nPrideFac > 5 && nPride < 1850 )
		nRealPride = nPride + ( 1850 - nPride ) / ( 12 - nPrideFac );
	else if ( nPrideFac < 5 && nPride > 1000 )
		nRealPride = nPride + ( nPride - 1200 ) / ( nPrideFac + 2 );
	else nRealPride = nPride;

	int nMinScore = ( nRealPride + 50 ) / 100;

	// 수정: 12 를 부를 수 있다면 13 을 부르도록 한다
	if ( nMinScore + 1 == CCard::GetState()->pRule->nMinScore )
		nMinScore++;

	nMinScore = min( 20, max( 0, nMinScore ) );

	return nMinScore;
}


/////////////////////////////////////////////////////////////////////////////
// CMaiBSWAlgo

#ifdef _DEBUG
static CString GetCardString( CCard c )
{
	static LPCTSTR as1[] = { _T("X"), _T("S"), _T("D"), _T("H"), _T("C") };
	static LPCTSTR as2[] = { _T("X"), _T("A"), _T("2"), _T("3"), _T("4"),
		_T("5"), _T("6"), _T("7"), _T("8"), _T("9"), _T("10"),
		_T("J"), _T("Q"), _T("K") };
	CString sRet;
	if ( c.IsJoker() ) sRet = _T("Joker");
	else sRet.Format(_T("%s%s"),as1[c.GetShape()],as2[c.GetNum()]);
	return sRet;
}
#else
#	define GetCardString(EXPR) _T("")
#endif

// 버릴만한 카드인가를 결정한다
static bool IsUseless( CCard c, int nKiruda )
{
	if ( c == GetMighty(nKiruda) || c.IsJoker()
		|| c.IsAce() || c.GetShape() == nKiruda
		|| c.GetNum() == KING ) return false;
	else return true;
}

// 주어진 13 장의 카드에 대해서
// 공약을 정하고 버릴 3 장의 카드를 리턴한다
void CMaiBSWAlgo::Goal( int* pnKiruda, int* pnMinScore, CCard acDrop[3],
	const CCardList* pHand,
	int nPrideFac,	// 0 ~ 10 까지, 주공 되려는 성향
	const CCardList* pDeck )
{
	CCardList lc = *pHand;
	if ( pDeck )
		lc.AddTail( (CList<CCard,CCard>*)pDeck );

	ASSERT( lc.GetCount() == 13 || lc.GetCount() == 8
		|| lc.GetCount() == 7 || lc.GetCount() == 10 );	//v4.0에서 수정됨

	m_pUpdate->SetProgress( 10 );	// 10 % 완료

	int nMaxPride = 0;
	int nMaxKiruda = 0;
	int anDrop[3] = { 0, 0, 0 };

	// 4종류의 모양에 대해, 각각이 기루다일 경우
	// 최대의 값이 나오는 경우를 계산한다 (노기 포함)

	for ( int nKiruda = 0; nKiruda <= CLOVER; nKiruda++ ) {

		m_pUpdate->SetProgress( 25 + 15 * nKiruda );

		if ( lc.GetCount() != 13 )	{	// 13 장을 모두 보지 않고 있다

			int nPride = m_pPrideTable->CalcPride( nKiruda, lc );

			if ( nPride > nMaxPride ) {
				nMaxPride = nPride;
				nMaxKiruda = nKiruda;
			}
		}
		else {				// 13 장 중에서 10 장 (13C10) 을 뽑아서
							// 결정한다
			for ( int i = 0; i < 13-2; i++ ) {
				if ( !IsUseless( lc.GetAt( lc.POSITIONFromIndex(i) ), nKiruda ) )
					continue;
			for ( int j = i+1; j < 13-1; j++ ) {
				if ( !IsUseless( lc.GetAt( lc.POSITIONFromIndex(j) ), nKiruda ) )
					continue;
			for ( int k = j+1; k < 13-0; k++ ) {
				if ( !IsUseless( lc.GetAt( lc.POSITIONFromIndex(k) ), nKiruda ) )
					continue;

				CCardList l;
				int nPride;

				POSITION pos = lc.GetHeadPosition();
				for ( int r = 0; pos; r++ ) {
					CCard c = lc.GetNext(pos);
					if ( r == i || r == j || r ==k ) continue;
					l.AddTail(c);
				}

				ASSERT( l.GetCount() == 10 );

				nPride = m_pPrideTable->CalcPride( nKiruda, l );

				if ( nPride > nMaxPride ) {
					nMaxPride = nPride;
					nMaxKiruda = nKiruda;
					anDrop[0] = i;
					anDrop[1] = j;
					anDrop[2] = k;
				}
			} } }
		}
	}

	m_pUpdate->SetProgress( 100 );

	// nMaxKiruda 를 기루다로 선택
	*pnKiruda = nMaxKiruda;

	// nMaxPride 에서 목표 점수를 결정
	*pnMinScore = m_pPrideTable->PrideToMinScore(
						nMaxPride, nPrideFac );

	// Drop 카드를 리턴
	acDrop[0] = lc.GetAt( lc.POSITIONFromIndex( anDrop[0] ) );
	acDrop[1] = lc.GetAt( lc.POSITIONFromIndex( anDrop[1] ) );
	acDrop[2] = lc.GetAt( lc.POSITIONFromIndex( anDrop[2] ) );

#ifdef _DEBUG
	TRACE("------player %d--\n",CCard::GetState()->nCurrentPlayer);
	TRACE("Kiruda : %d, Pride : %d\n", *pnKiruda, nMaxPride );
	TRACE("Drop : %s %s %s\n", acDrop[0].GetString(true),
		acDrop[1].GetString(true), acDrop[2].GetString(true) );
#endif
}

int CMaiBSWAlgo::Friend( int nKiruda, const CCardList* pHand, const CCardList* pDeck ) const
{
	// pHand 와 pDeck 를 pHand 로 합친다
	CCardList cl;
	if ( pDeck ) {
		cl = *pHand;
		cl.AddTail( (CList<CCard,CCard>*)pDeck );
		pHand = &cl;
	}

	int nFriend = 0;

	// 프랜드 선택
	int i;
	CCard c;
	do {
		// 먼저, 마이티 !
		c = GetMighty(nKiruda);
		if ( !pHand->Find(c) ) break;

		// 다음, 조커
		c = CCard::GetJoker();
		if ( CCard::GetState()->pRule->bJokerFriend
			&& !pHand->Find(c) ) break;

		// 기A
		c = CCard( nKiruda, ACE );
		if ( !pHand->Find(c) ) break;

		// 기 K
		c = CCard( nKiruda, KING );
		if ( !pHand->Find(c) ) break;

		// 다른 ACE
		for ( i = SPADE; i <= CLOVER; i++ ) {
			c = CCard( i, ACE );
			if ( !pHand->Find(c) ) break;
		}
		if ( i <= CLOVER ) break;

		// 기 Q
		c = CCard( nKiruda, QUEEN );
		if ( !pHand->Find(c) ) break;

	} while(0);

	if ( (int)c ) nFriend = (int)c;
	else {
		// 초구프랜드로 한다.
		nFriend = 1;
	}

	return nFriend;
}

// 죽일 카드 선택
CCard CMaiBSWAlgo::Kill( const CCardList* pHand, const CCardList* pTillNow ) const
{
	// 손에 없는 최고 기루다를 찾는다

	int nKiruda = CCard::GetState()->goal.nKiruda;
	int nFrom, nTo;

	if ( !nKiruda )	// nokiruda
		nFrom = SPADE, nTo = CLOVER;
	else nFrom = nKiruda, nTo = nKiruda;

	for ( int s = nFrom; s <= nTo; s++ )
		if ( !pHand->Find( CCard( s, ACE ) )
			&& !pTillNow->Find( CCard( s, ACE ) ) )
			return (int)CCard( s, ACE );
		else for ( int i = KING; i > 2; i-- )
			if ( !pHand->Find( CCard( s, i ) )
				&& !pTillNow->Find( CCard( s, i ) ) )
				return (int)CCard( s, i );
	ASSERT(0);
	return (int)CCard(nKiruda,2);	// never reached
}


/////////////////////////////////////////////////////////////////////////////
// Turn 헬퍼 함수

// 이 카드가 쓰여서 사라졌는가 조사한다
static bool IsUsed( CCard c )
{
	for ( int i = 0; i < CCard::GetState()->nPlayers; i++ )
		if ( CCard::GetState()->apPlayers[i]->GetHand()
				->Find(c) ) return false;
	if ( CCard::GetState()->lCurrent.Find(c) ) return false;
	return true;
}

// 같은 능력의 카드를 리턴
CCard CMaiBSWAlgo::GetEqualCard( CCard c ) const
{
	const CCardList* pHand =
		CCard::GetState()->apPlayers[
			CCard::GetState()->nCurrentPlayer]->GetHand();

	ASSERT( pHand->Find(c) );

	if ( c.IsJoker() || c.IsMighty() || c.IsJokercall() )
		return c;

	// c 와 같은 집합의 카드들
	CCardList l;

	int from, to;
	if ( c.IsPoint() ) from = 9, to = 13;	// 10 ~ Ace
	else from = 1, to = 8;	// 2 ~ 9

	int i;
	for ( i = c.GetPowerNum(); i <= to; i++ ) {
		CCard d( c.GetShape(), i % 13 + 1 );
		if ( pHand->Find(d) ) l.AddTail(d);
		else if ( !IsUsed(d) ) break;
	}
	for ( i = c.GetPowerNum()-1; i >= from; i-- ) {
		CCard d( c.GetShape(), i % 13 + 1 );
		if ( pHand->Find(d) ) l.AddHead(d);
		else if ( !IsUsed(d) ) break;
	}

#ifdef _DEBUG
	TRACE("%s is same as :\n",c.GetString(false));
	POSITION pd = l.GetHeadPosition();
	while (pd) {
		c = l.GetNext(pd);
		TRACE("%s ",c.GetString(false));
	}
	TRACE("\n");
#endif

	// 가장 높은 카드를 내든지, 랜덤하게 낸다
	if ( !CCard::GetState()->bFriendRevealed || rand()%3 )
		return l.GetTail();
	else return l.GetAt( l.POSITIONFromIndex( rand()%l.GetCount() ) );
}

// 재귀 순환 알고리즘
// 벌점이 가장 낮게 되는 카드와 그 때의 벌점을 구한다
static void RecursiveIteration(
	CCardList& lPredict,	// 예상되는, 이후에 나올 카드들
	CCardList& lCurrent,	// 현재 바닥에 깔린 카드

	bool& bJokercallEffect,	// [in,out] 조커콜 여부
	int& nJokerShape,		// [in,out] 조커 모양

	int nPlayer,			// 현재 플레이어
	CCardList* aAv,			// 현재 플레이어부터의 가능한 카드 리스트
	const CMaiBSWPenaltyTable* pPenaltyTable,
	MAIDLL_UPDATE* pUpdate )
{
	const CState* pState = CCard::GetState();

	// 현재 플레이어 수(nPlayers)와 선(nBeginer),
	// 그리고 실제 알고리즘을 호출한 진짜 플레이어(nCurrentPlayer)를
	// 얻는다
	int nPlayers = pState->nPlayers;
	int nBeginer = pState->nBeginer;
	int nCurrentPlayer = pState->nCurrentPlayer;
	// 앞으로 카드를 더 낼 사람들의 수 (나 포함)
	int nPlayersToPlay = nPlayers - lCurrent.GetCount();

	ASSERT( lPredict.GetCount() == 0 );
	ASSERT( lCurrent.GetCount() == (nPlayer-nBeginer+nPlayers)%nPlayers );
	ASSERT( nCurrentPlayer != nPlayer && aAv
		|| nCurrentPlayer == nPlayer && !aAv );


	if ( !aAv ) {	// aAv 에 av-list 를 할당한다
		aAv = new CCardList[nPlayersToPlay];
		// 선 자신의 av-list
		pState->apPlayers[nPlayer]->GetHand()->GetAvList(
			&aAv[0], &lCurrent, pState->nTurn,
			nJokerShape, bJokercallEffect );
	}

	if ( nBeginer == nPlayer ) {
		// 선인 경우에 이들 값은 초기화 되어있어야 한다
		bJokercallEffect = false;
		nJokerShape = 0;
	}

	// 최저 벌점일 때의 벌점과 카드, 상황 등
	int nMaxPenalty = 0;
	CCard cMax;
	bool bMaxJokercallEffect = bJokercallEffect;
	int nMaxJokerShape = nJokerShape;

	CCardList lTemp;	// 임시 용도
	bool bJokercallTesting = false;

	bool bJumpToHere = false;
	int nProgressAmount = 100 / max(aAv->GetCount(),1);
	int nProgress = 0;

	POSITION pos = aAv->GetHeadPosition();
	while (pos) {

		Sleep(0);
		if ( nPlayer == nCurrentPlayer )
			pUpdate->SetProgress( nProgress += nProgressAmount );

		CCard c = aAv->GetNext(pos);
		POSITION posSave;

lbl_here: bJumpToHere = false;

		if ( nBeginer == nPlayer ) {
			// 이 카드가 선인 경우

			// 조커나 조커 콜인 경우 각 옵션에 대해 모두 시도해야 한다
			if ( c.IsJoker() ) {
				if ( ++nJokerShape > CLOVER ) continue;
				else bJumpToHere = true;
			}
			if ( c.IsJokercall() && pState->IsEffectiveJokercall() ) {
				if ( !bJokercallTesting ) {
					bJokercallTesting = true;
					bJokercallEffect = false;
					bJumpToHere = true;
				} else if ( !bJokercallEffect ) {
					bJokercallEffect = true;
					bJumpToHere = true;
				} else {
					bJokercallEffect = false;
					bJokercallTesting = false;
					continue;
				}
			}
		}

		// 최적 카드 선택

		// c 를 lCurrent 에 놓는 것 처럼 하면서 다음 플레이어들의
		// lPredict 를 구한다
		lCurrent.AddTail(c);
		{
			Sleep(0);

			lTemp.RemoveAll();

			if ( nPlayer == nCurrentPlayer ) {
				// 최초로 호출한 플레이어의 경우
				// 각 플레이어의 av-list 를 구해 놓아야 한다
				for ( int i = 1; i < nPlayersToPlay; i++ )
					pState->apPlayers[(nPlayer+i)%nPlayers]->GetHand()
					->GetAvList( &aAv[i], &lCurrent, pState->nTurn,
						nJokerShape, bJokercallEffect );
			}

			Sleep(0);

			if ( lCurrent.GetCount() < nPlayers )

				RecursiveIteration( lTemp, lCurrent,
					bJokercallEffect, nJokerShape,
					(nPlayer+1)%nPlayers, aAv+1,
					pPenaltyTable, pUpdate );

			posSave = lTemp.GetHeadPosition();
			lTemp.AddHead( (CList<CCard,CCard>*)&lCurrent );
		}
		lCurrent.RemoveTail();

		// 벌점을 계산하고 더 적은 벌점에 대해서 갱신한다
		int nPenalty = pPenaltyTable->CalcPenalty(
			CCard::GetKiruda(), pState,
			nPlayer, lTemp, bJokercallEffect == 0 ? false : true,
			nJokerShape );

		if ( (int)cMax == 0 || nPenalty < nMaxPenalty ) {

			nMaxPenalty = nPenalty;
			cMax = c;
			nMaxJokerShape = nJokerShape;
			bMaxJokercallEffect = bJokercallEffect;
			lPredict.RemoveAll();
			while (posSave) lPredict.AddTail( lTemp.GetNext(posSave) );
			lPredict.AddHead(c);	// 방금 낸 카드는 예측카드의 가장 첫 카드
		}

		if ( bJumpToHere ) goto lbl_here;
	}

	bJokercallEffect = bMaxJokercallEffect;
	nJokerShape = nMaxJokerShape;

	if ( nPlayer == nCurrentPlayer ) {	// 할당한 리스트를 없앤다
		delete[] aAv;
	}

	ASSERT( lCurrent.GetCount() == (nPlayer-nBeginer+nPlayers)%nPlayers );
	ASSERT( lPredict.GetCount() + lCurrent.GetCount() == nPlayers );
}

// 재귀 순환 알고리즘으로 카드 내기
CCard CMaiBSWAlgo::TurnIteration( int& eff ) const
{
	// RecursiveIteration 을 위해 필요한 변수들

	CCardList lPredict;
	CCardList lCurrent = CCard::GetState()->lCurrent;

	bool bJokercallEffect = CCard::GetState()->bJokercallEffect;
	int nJokerShape = CCard::GetState()->nJokerShape;

	// 알고리즘 호출
	RecursiveIteration( lPredict, lCurrent, bJokercallEffect, nJokerShape,
		CCard::GetState()->nCurrentPlayer, 0, m_pPenaltyTable, m_pUpdate );

	CCard c = lPredict.GetHead();

	if ( lCurrent.GetCount() == 0 ) {	// 선
		if ( c.IsJokercall() ) eff = bJokercallEffect ? 1 : 0;
		else if ( c.IsJoker() ) eff = nJokerShape;
	}

#ifdef _DEBUG
	TRACE("[%d] ",CCard::GetState()->nCurrentPlayer);
	POSITION pos = lPredict.GetHeadPosition();
	while (pos) TRACE(" -> %s",GetCardString(lPredict.GetNext(pos)));
	TRACE("\n");
#endif

	return c;
}

// 시물레이션 알고리즘
// 앞으로 벌어질 모든 경우에 대해서, 가장 많이 득점하는 경우를 찾는다
static void Simulation(
	int nTurn,				// 턴
	int nBeginer,			// 선
	int nPlayer,			// 현재 플레이어
	int& nScored,			// 여당이 얻는 점수 (+,-)
	CCard& c,				// 다음 플레이어가 낼 카드
	bool& bJokercallEffect,	// [in,out] 조커콜 효력 여부
	int& nJokerShape,		// [in,out] 조커 모양
	CCardList alHand[],		// 플레이어 손에 든 카드
	CCardList* pCurrent,	// 가운데 쌓인 카드
	CCardList* aAv,			// 각 플레이어가 낼 수 있는 카드들

	int nLevel,
	int nProgress,
	int nProgressL1Range,
	MAIDLL_UPDATE* pUpdate )
{
	if ( nTurn > LAST_TURN ) { // 마지막 턴을 지남 (종료 조건
		nScored = 0;
		c = 0;
		bJokercallEffect = false;
		nJokerShape = 0;
		return;
	}

	const CState* pState = CCard::GetState();

	// 플레이어 수
	int nPlayers = pState->nPlayers;
	int nPlayersToPlay = nPlayers - pCurrent->GetCount();

	// 이사람은 여당인가
	bool bIsDefender = pState->IsDefender(nPlayer);

	// 만일 aAv 가 할당되어 있지 않으면 할당한다
	bool bAvAllocated = false;
	if ( !aAv ) {
		Sleep(0);
		bAvAllocated = true;
		aAv = new CCardList[nPlayersToPlay];
		for ( int i = 0; i < nPlayersToPlay; i++ )
			alHand[(nPlayer+i)%nPlayers].GetAvList(
				&aAv[i], pCurrent, nTurn, nJokerShape, bJokercallEffect );
	}

	if ( nBeginer == nPlayer ) {
		// 선인 경우에 이들 값은 초기화 되어있어야 한다
		bJokercallEffect = false;
		nJokerShape = 0;
	}

	// 가장 유리한 점수를 가질 때의 카드와, 그 점수, 상황 등
	CCard cMax;
	int nMaxScored = 0;
	bool bMaxJokercallEffect = bJokercallEffect;
	int nMaxJokerShape = nJokerShape;

	bool bJumpToHere;
	bool bJokercallTesting = false;

	int nAvCount = max( 1, aAv->GetCount() );
	int nProgressAmount;
	if ( nLevel == 0 ) {
		nProgressAmount = 100 / nAvCount;
		nProgress = -nProgressAmount;
		nProgressL1Range = nProgressAmount;
	} else if ( nLevel == 1 )
		nProgressAmount = nProgressL1Range / nAvCount;
	else nProgressAmount = 0;

	// 낼 수 있는 모든 카드에 대해서 최대의 점수를 구한다
	POSITION pos = aAv->GetHeadPosition();
	while (pos) {

		if ( nLevel == 0 || nLevel == 1 )
			pUpdate->SetProgress( nProgress += nProgressAmount );

		CCard c = aAv->GetNext(pos);
		int nScored;

lbl_here: bJumpToHere = false;

		if ( nBeginer == nPlayer ) {
			// 이 카드가 선인 경우

			// 조커나 조커 콜인 경우 각 옵션에 대해 모두 시도해야 한다
			if ( c.IsJoker() ) {
				if ( ++nJokerShape > CLOVER ) continue;
				else bJumpToHere = true;
			}
			if ( c.IsJokercall() && pState->IsEffectiveJokercall() ) {
				if ( !bJokercallTesting ) {
					bJokercallTesting = true;
					bJokercallEffect = false;
					bJumpToHere = true;
				} else if ( !bJokercallEffect ) {
					bJokercallEffect = true;
					bJumpToHere = true;
				} else {
					bJokercallTesting = false;
					bJokercallEffect = false;
					continue;
				}
			}
		}

		// 최적 카드 선택

		// c 를 pCurrent 에 놓는 것 처럼 꾸민다
		pCurrent->AddTail(c);
		alHand[nPlayer].RemoveAt( alHand[nPlayer].Find(c) );
		{
			Sleep(0);

			CCard cDummy;

			// 조커나 조커콜일 경우 progress 를 표시할 때 같은 카드에 대해
			// 여러번 생각하므로 바가 왔다갔다 하는 경우가 있다
			// 따라서 nLevel 을 조정해서 이들 경우에 한번만 Update 가
			// 되도록 nCurLevel 을 nLevel 값을 대신하여 쓴다
			int nCurLevel = nLevel;
			if ( nPlayer == nBeginer )
				if ( c.IsJoker() && nJokerShape != SPADE
					|| c.IsJokercall() && bJokercallEffect )
					nCurLevel = 1000;	// Update 되지 않는 임의의 레벨

			// 다음 플레이어가 카드를 내는 동작 (재귀 호출)

			// 다음 플레이어는 새로운 턴을 시작하지 않음
			if ( pCurrent->GetCount() < nPlayers )

				Simulation( nTurn, nBeginer, (nPlayer+1)%nPlayers, nScored,
					cDummy, bJokercallEffect, nJokerShape, alHand,
					pCurrent, aAv+1, nCurLevel+1,
					nProgress, nProgressL1Range, pUpdate );

			else {	// 새로운 턴의 시작

				bool bWinnerIsDef;
				int nWinner = pState->GetWinner( pCurrent,
					&bWinnerIsDef, nJokerShape, bJokercallEffect ? 1 : 0,
					nPlayers, nTurn, nBeginer );

				CCardList lNextCurrent;	// 다음 턴에 쓸 가운데 카드
				bool bNextJokercallEffect = false;
				int nNextJokerShape = 0;

				Simulation( nTurn+1, nWinner, nWinner, nScored,
					cDummy, bNextJokercallEffect, nNextJokerShape,
					alHand, &lNextCurrent, 0, nCurLevel+1,
					nProgress, nProgressL1Range, pUpdate );

				if ( bWinnerIsDef ) nScored += pCurrent->GetPoint();
				else nScored -= pCurrent->GetPoint();
			}
		}
		pCurrent->RemoveTail();
		alHand[nPlayer].AddTail(c);

		// 득점을 계산하고 더 좋은 득점에 대해서 갱신한다
		if ( (int)cMax == 0	// 초기화가 안되었거나
			|| bIsDefender && nScored > nMaxScored	// 우리편이 많이 딴 경우
			|| !bIsDefender && nScored < nMaxScored ) {

			nMaxScored = nScored;
			cMax = c;
			nMaxJokerShape = nJokerShape;
			bMaxJokercallEffect = bJokercallEffect;
		}

		if ( bJumpToHere ) goto lbl_here;
	}

	ASSERT( (int)cMax != 0 );

	nScored = nMaxScored;
	c = cMax;
	bJokercallEffect = bMaxJokercallEffect;
	nJokerShape = nMaxJokerShape;

	if ( bAvAllocated ) delete[] aAv;
}

CCard CMaiBSWAlgo::TurnSimulation( int& eff ) const
{
	const CState* pState = CCard::GetState();

	int nScored;
	CCard c;
	bool bJokercallEffect = pState->bJokercallEffect;
	int nJokerShape = pState->nJokerShape;

	CCardList alHand[MAX_PLAYERS];
	for ( int i = 0; i < pState->nPlayers; i++ )
		alHand[i] = *pState->apPlayers[i]->GetHand();

	CCardList lCurrent = pState->lCurrent;

	Simulation( pState->nTurn, pState->nBeginer, pState->nCurrentPlayer,
		nScored, c, bJokercallEffect, nJokerShape, alHand,
		&lCurrent, 0, 0, 0, 0, m_pUpdate );

	if ( pState->nBeginer == pState->nCurrentPlayer ) {	// 선
		if ( c.IsJokercall() ) eff = bJokercallEffect ? 1 : 0;
		else if ( c.IsJoker() ) eff = nJokerShape;
	}

#ifdef _DEBUG
	for ( int j = 0; j < pState->nPlayers; j++ )
		ASSERT( alHand[j].GetCount() == pState->apPlayers[j]->GetHand()->GetCount() );
	ASSERT( lCurrent.GetCount() == pState->lCurrent.GetCount() );
#endif

	return c;
}


// 카드 내기 (필요한 모든 인자는 CCard::CState 에서 얻는다 )
CCard CMaiBSWAlgo::Turn( int& eff, bool bUseSimulation ) const
{
	// 시물레이션을 사용하면, 3 턴부터, 아니면 2 턴부터
	// 시물레이션 알고리즘을 사용한다
	int nSimulatedTurn = bUseSimulation ? 3 : 2;

	CCard c;
	if ( CCard::GetState()->nTurn + nSimulatedTurn > LAST_TURN )
		c = TurnSimulation(eff);
	else c = TurnIteration(eff);

	// 주공의 조커콜이고, 프랜드에게 조커가 있다는 사실을 몰라야 될 때
	// '조커콜 아님' 은 너무 속보이므로 그냥 조커콜로 바꾼다
	if ( c.IsJokercall() ) {

		const CState* pState = CCard::GetState();

		if ( !eff	// 조커콜이 아니고
			&& pState->lCurrent.IsEmpty()	// 선이고
			&& pState->nCurrentPlayer == pState->nMaster	// 주공이고
			&& pState->goal.nFriend == (int)CCard::GetMighty()	// 프랜드에게 조커가 있는지 모르고
			&& pState->nFriend >= 0	// 프랜드가 있고
			&& pState->apPlayers[pState->nFriend]->GetHand()
				->Find( CCard::GetJoker() )	// 프랜드한테 조커가 있고
			)
				eff = 1;
	}

	// c 와 같은 의미의 카드를 낸다
	return GetEqualCard(c);
}


/////////////////////////////////////////////////////////////////////////////
// CMaiBSWWrap

CMaiBSWWrap::CMaiBSWWrap( LPCTSTR sOption, MAIDLL_UPDATE* pUpdate )
{
	m_sOption = sOption;
	m_pUpdate = pUpdate;
	m_pState = 0;

	if ( !sOption || sscanf_s( sOption, _T("%d %d %d"),
		&m_bGoalMode, &m_bUseSimulation, &m_nPrideFac ) != 3 ) {

		m_nPrideFac = 5;
		m_bGoalMode = true;
		m_bUseSimulation = true;
	}

	m_pPenaltyTable = new CMaiBSWPenaltyTable;
	m_pPrideTable = new CMaiBSWPrideTable;
	m_pBSW = new CMaiBSWAlgo( m_pPenaltyTable, m_pPrideTable, m_pUpdate );
}

CMaiBSWWrap::~CMaiBSWWrap()
{
	delete m_pBSW;
	delete m_pPrideTable;
	delete m_pPenaltyTable;
}

// 손에 든 카드를 구한다
const CCardList* CMaiBSWWrap::GetHand() const
{
	return m_pState->apPlayers[m_pState->nCurrentPlayer]->GetHand();
}

// 옵션 세트
void CMaiBSWWrap::SetOption( HWND hWnd )
{
	CMaiBSWSetting dlg( CWnd::FromHandle(hWnd) );
	if ( sscanf_s( m_sOption, _T("%d %d %d"),
		&dlg.m_bGoalMode, &dlg.m_bUseSimulation, &dlg.m_nFactor )
		!= 3 ) {
		dlg.m_bGoalMode = m_bGoalMode ? TRUE : FALSE;
		dlg.m_bUseSimulation = m_bUseSimulation ? TRUE : FALSE;
		dlg.m_nFactor = m_nPrideFac;
	}

	if ( dlg.DoModal() == IDOK ) {

		sprintf( m_sOption.GetBuffer(16), _T("%d %d %d"),
			dlg.m_bGoalMode, dlg.m_bUseSimulation, dlg.m_nFactor );
		m_sOption.ReleaseBuffer();

		m_bGoalMode = !!dlg.m_bGoalMode;
		m_bUseSimulation = !!dlg.m_bUseSimulation;
		m_nPrideFac = dlg.m_nFactor;
	}
}


// 하나의 게임이 시작됨
// 이 게임에서 사용되는 상태 변수를 알려준다
// 이 상태 변수는 CCard::GetState() 로도 알 수 있다
// pState->nCurrentPlayer 값이 바로 자기 자신의
// 번호이며, 이 값은 자신을 인식하는데 사용된다
void CMaiBSWWrap::OnBegin( const CState* pState )
{
	m_pState = pState;
	ASSERT( CCard::GetState() == pState );
}

// 공약을 듣는다
// pNewGoal 과 state.goal 에 현재까지의 공약이 기록되어 있다
// 패스하려면 nMinScore 를 현재 공약보다 작은 값으로 세트
// pNewGoal.nFriend 는 사용하지 않는다
void CMaiBSWWrap::OnElection( CGoal* pNewGoal )
{
	int nKiruda, nMinScore;
	CCard acDrop[3];
	m_pBSW->Goal( &nKiruda, &nMinScore, acDrop,
		GetHand(), m_nPrideFac,
		// 6,7 마의 경우 그 덱을 내가 가진다는 보장이 없다
		m_pState->pRule->nPlayerNum >= 6 ? 0 : &m_pState->lDeck );

	pNewGoal->nFriend = 0;
	pNewGoal->nKiruda = nKiruda;

	if ( m_pState->pRule->nMinScore > nMinScore )	// 포기
		pNewGoal->nMinScore = -1;
	else if ( !m_bGoalMode ) pNewGoal->nMinScore = nMinScore;
	else
		pNewGoal->nMinScore =	// 1 씩 올린다
			pNewGoal->nMinScore < nMinScore && pNewGoal->nMinScore > 0
			? pNewGoal->nMinScore + 1 : m_pState->pRule->nMinScore;
}

// 6 마에서 당선된 경우 한 명을 죽여야 한다
// 죽일 카드를 지정하면 된다 - 단 이 함수는
// 반복적으로 호출될 수 있다 - 이 경우
// CCardList 에 지금까지 실패한 카드의 리스트가
// 누적되어 호출된다
// 5번 실패하면 (이 경우 알고리즘이 잘못되었거나
// 사람이 잘 못 선택하는 경우) 임의로 나머지 5명 중
// 하나가 죽는다 !
void CMaiBSWWrap::OnKillOneFromSix(
	CCard* pcCardToKill, CCardList* plcFailedCardsTillNow )
{
	*pcCardToKill = m_pBSW->Kill(
		GetHand(), plcFailedCardsTillNow );
}
// 7마에서 당선된 경우 두 사람을 죽여야 한다
// 이 함수는 그 중 하나만 죽이는 함수로,
// OnKillOneFromSix와 같다.
// 5번 실패하면 (이 경우 알고리즘이 잘못되었거나
// 사람이 잘 못 선택하는 경우) 임의로 나머지 5명 중
// 하나가 죽는다 !
void CMaiBSWWrap::OnKillOneFromSeven(
	CCard* pcCardToKill, CCardList* plcFailedCardsTillNow )
{
	*pcCardToKill = m_pBSW->Kill(
		GetHand(), plcFailedCardsTillNow );
}

// 당선된 경우
// pNewGoal 과 state.goal 에 현재까지의 공약이 기록되어 있다
// pNewGoal 에 새로운 공약을 리턴하고 (nFriend포함)
// acDrop 에 버릴 세 장의 카드를 리턴한다
void CMaiBSWWrap::OnElected( CGoal* pNewGoal, CCard acDrop[3] )
{
	int nKiruda, nMinScore;
	m_pBSW->Goal( &nKiruda, &nMinScore, acDrop,
		GetHand(), m_nPrideFac, &m_pState->lDeck );
	int nFriend = m_pBSW->Friend( nKiruda, GetHand(), &m_pState->lDeck );

	pNewGoal->nFriend = nFriend;

	if ( pNewGoal->nKiruda != nKiruda ) {

		for ( int n = pNewGoal->nMinScore; n <= nMinScore; n++ ) {

			if ( m_pState->IsValidNewGoal( nKiruda, n ) ) {
				pNewGoal->nMinScore = n;
				pNewGoal->nKiruda = nKiruda;
				return;
			}
		}
	}
}

// 선거가 끝났고 본 게임이 시작되었음을 알린다
void CMaiBSWWrap::OnElectionEnd()
{
}

// 카드를 낸다
// pc 는 낼 카드, pe 는 pc가 조커인경우 카드 모양,
// pc가 조커콜인경우 실제 조커콜 여부(0:조커콜 아님)
// 그 외에는 무시
void CMaiBSWWrap::OnTurn( CCard* pc, int* pe )
{
	*pc = m_pBSW->Turn( *pe, m_bUseSimulation );
}

// 한 턴이 끝났음을 알린다
// 아직 state.plCurrent 등은 그대로 남아있다
void CMaiBSWWrap::OnTurnEnd()
{
}


/////////////////////////////////////////////////////////////////////////////
// CMaiBSWSetting dialog

CMaiBSWSetting::CMaiBSWSetting(CWnd* pParent /*=NULL*/)
	: CDialog(CMaiBSWSetting::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMaiBSWSetting)
	m_nFactor = 0;
	m_bGoalMode = FALSE;
	m_bUseSimulation = FALSE;
	//}}AFX_DATA_INIT
}


void CMaiBSWSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMaiBSWSetting)
	DDX_Slider(pDX, IDC_FACTOR, m_nFactor);
	DDX_Check(pDX, IDC_GOALMODE, m_bGoalMode);
	DDX_Check(pDX, IDC_USESIMULATION, m_bUseSimulation);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMaiBSWSetting, CDialog)
	//{{AFX_MSG_MAP(CMaiBSWSetting)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMaiBSWSetting message handlers

BOOL CMaiBSWSetting::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	((CSliderCtrl*)GetDlgItem(IDC_FACTOR))->SetRange( 0, 10, TRUE );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
