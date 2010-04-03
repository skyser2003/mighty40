// Play.h: interface for the CPlay class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAY_H__E21B2747_CFA8_11D2_9810_000000000000__INCLUDED_)
#define AFX_PLAY_H__E21B2747_CFA8_11D2_9810_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct CState;
struct CGoal;
class CCard;

// 마이티에서 AI, 또는 사람이 취할 수 있는
// 동작들이나 이벤트들을 정의하는 인터페이스

struct CPlay  
{
	virtual ~CPlay() {}

	// 하나의 게임이 시작됨
	// 이 게임에서 사용되는 상태 변수를 알려준다
	// 이 상태 변수는 CCard::GetState() 로도 알 수 있다
	// pState->nCurrentPlayer 값이 바로 자기 자신의
	// 번호이며, 이 값은 자신을 인식하는데 사용된다
	virtual void OnBegin( const CState* pState ) = 0;

	// 공약을 듣는다
	// pNewGoal 과 state.goal 에 현재까지의 공약이 기록되어 있다
	// 패스하려면 nMinScore 를 0으로 세트
	// 특히 딜 미스를 원하는 경우 공약을 -1로 세트
	// pNewGoal.nFriend 는 사용하지 않는다
	virtual void OnElection( CGoal* pNewGoal ) = 0;

	// 6마에서 당선된 경우 한 사람을 죽여야 한다
	// 죽일 카드를 지정하면 된다 - 단 이 함수는
	// 반복적으로 호출될 수 있다 - 이 경우
	// CCardList 에 지금까지 실패한 카드의 리스트가
	// 누적되어 호출된다
	// 5번 실패하면 (이 경우 알고리즘이 잘못되었거나
	// 사람이 잘 못 선택하는 경우) 임의로 나머지 5명 중
	// 하나가 죽는다 !
	virtual void OnKillOneFromSix(
		CCard* pcCardToKill, CCardList* plcFailedCardsTillNow ) = 0;
	// 7마에서 당선된 경우 두 사람을 죽여야 한다
	// 이 함수는 그 중 하나만 죽이는 함수로,
	// OnKillOneFromSix와 같다.
	// 5번 실패하면 (이 경우 알고리즘이 잘못되었거나
	// 사람이 잘 못 선택하는 경우) 임의로 나머지 5명 중
	// 하나가 죽는다 !
	virtual void OnKillOneFromSeven(
		CCard* pcCardToKill, CCardList* plcFailedCardsTillNow ) = 0;

	// 당선된 경우
	// pNewGoal 과 state.goal 에 현재까지의 공약이 기록되어 있다
	// pNewGoal 에 새로운 공약을 리턴하고 (nFriend포함)
	// acDrop 에 버릴 세 장의 카드를 리턴한다
	// * 주의 * 이 함수 이후에 한명이 죽어서 플레이어의
	// 번호(pState->nCurrentPlayer)가 변경되었을 수 있다
	virtual void OnElected( CGoal* pNewGoal, CCard acDrop[3] ) = 0;

	// 선거가 끝났고 본 게임이 시작되었음을 알린다
	// * 주의 * 이 함수 이후에 한명이 죽어서 플레이어의
	// 번호(pState->nCurrentPlayer)가 변경되었을 수 있다
	virtual void OnElectionEnd() = 0;

	// 카드를 낸다
	// pc 는 낼 카드, pe 는 pc가 조커인경우 카드 모양,
	// pc가 조커콜인경우 실제 조커콜 여부(0:조커콜 아님)
	// 그 외에는 무시
	virtual void OnTurn( CCard* pc, int* pe ) = 0;

	// 한 턴이 끝났음을 알린다
	// 아직 state.plCurrent 등은 그대로 남아있다
	virtual void OnTurnEnd() = 0;
};


/////////////////////////////////////////////////////////////////////////////
// Mighty AI DLL 들이 export 해야 하는 함수들과 관련 구조체

// MAI DLL 정보
struct MAIDLL_INFO {
	CString sName;		// MAI 이름
	CString sExplain;	// 설명
};

// 업데이트 함수
// 좋은 AI DLL 은 생각의 진행 상황을 출력해 준다
struct MAIDLL_UPDATE {
	// 현재 생각 단계 ( 100 등분 중 )
	virtual void SetProgress( int nPercentage ) = 0;
	// 출력될 스트링 ( 디폴트는 '생각중..' 같은 류의 메시지 )
	virtual void SetText( LPCTSTR ) = 0;
};

// DLL 정보를 얻는다
typedef void (*pfMaiGetInfo)( MAIDLL_INFO* );

// 옵션 스트링을 가지고 CPlay 객체를 얻는다
// 단, sOption 이 NULL 일 때는 디폴트 값
typedef CPlay* (*pfMaiGetPlay)( LPCTSTR sOption, MAIDLL_UPDATE* );

// 이 객체에 대한 옵션 다이얼로그 박스를 출력하고
// 옵션을 설정한다
// 설정할 옵션이 없는 AI 인 경우 이 함수는 생략되어도 좋다
// hWnd : 다이얼로그 박스의 부모 윈도우
typedef void (*pfMaiOption)( CPlay*, HWND );

// CPlay 객체를 되돌려준다
// 옵션 스트링을 리턴함
typedef void (*pfMaiReleasePlay)( CPlay*, CString* psOption );


#endif // !defined(AFX_PLAY_H__E21B2747_CFA8_11D2_9810_000000000000__INCLUDED_)
