// Rule.h: interface for the CRule class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RULE_H__6433F792_C1B5_11D2_97F2_000000000000__INCLUDED_)
#define AFX_RULE_H__6433F792_C1B5_11D2_97F2_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STANDARD_RULE_STRING _T("(0$1M&WO*7H'5#")

// 마이티 게임 규칙

struct CRule  
{
	CRule() { Preset( 1 ); }

	// 미리 정의된 표준 룰로 세트
	// (리턴값은 그 룰의 이름, NULL 이면 해당 룰이 없음)
	// 1 : 표준 5마  2 : 표준 4마  3 : 표준 3마
	// 4 : 표준 6마
	LPCTSTR Preset( int nRule );

	// 현재 룰을 스트링으로 인코드해서 리턴
	CString Encode() const;
	// 스트링에서 새로운 룰을 디코드함
	void Decode( LPCTSTR sRule );

	/////////////////////////////////////////////////////////////////////////

	// 마이티 인원

	// 인원 ( 3-6 )
	int nPlayerNum;
	// 시계방향인가
	bool bClockWise;

	// 선거

	// 최소 기본 점수 ( 5 - 18 )
	int nMinScore;
	// 노기루다일때 기본점수보다 1 적게 부를 수 있음
	bool bNoKirudaAdvantage;
	// 기본점수를 20 보다 높이 부를 수 있음 (25까지==HIGHSCORE_MAX_LIMIT)
	bool bHighScore;
	// 다음판의 첫 출마자는 이전판의 프랜드인가 (false:전판선)
	bool bFriendGetsBeginer;
	// 선이 패스할 수 있는가
	bool bBeginerPass;
	// 기루다 바꿀 때 추가로 올리는 점수가 2 인가 (false:1)
	bool bRaise2ForKirudaChange;
	// 노기루다에서 바꾸거나 노기루다로 바꿀 때는 1 만 올려도 되는가
	bool bRaise1ForNoKirudaChange;
	// 예약됨
	bool bReserved;
	// 프랜드가 있는가
	bool bFriend;
	// 조커를 프랜드로 부를 수 있는가
	bool bJokerFriend;
	// 버린 카드는 야당의 득점으로 치는가
	bool bAttScoreThrownPoints;

	// 제약 카드 & 효력

	// 첫턴에 마이티 낼 수 있음 & 효력
	bool bInitMighty;
	bool bInitMightyEffect;
	// 막턴에 마이티 낼 수 있음 & 효력
	bool bLastMighty;
	bool bLastMightyEffect;
	// 조커콜에 조커대신 마이티 낼 수 있음
	bool bOverrideMighty;
	// 첫턴에 조커 낼 수 있음 & 효력
	bool bInitJoker;
	bool bInitJokerEffect;
	// 막턴에 조커 낼 수 있음 & 효력
	bool bLastJoker;
	bool bLastJokerEffect;
	// 첫턴에 기루다 낼 수 있음
	bool bInitKiruda;
	// 첫턴에 선이 기루다 낼 수 있음
	bool bInitBeginKiruda;
	// 첫턴에 조커콜 효력 있음
	bool bInitJokercallEffect;
	// 조커콜시 조커 효력 있음
	bool bJokercallJokerEffect;

	// 딜 미스 조건

	// 점수카드가 없을때
	bool bDM_NoPoint;
	// 모두 점수카드일때
	bool bDM_AllPoint;
	// 위의 두 경우 마이티를 점수카드로 계산
	bool bDM_MightyIsPoint;
	// 위의 두 경우 조커를 점수카드로 계산
	bool bDM_JokerIsPoint;
	// 조커를 역 점수카드로 계산
	bool bDM_JokerIsReversePoint;
	// 점수카드 10
	bool bDM_Only10;
	// 애꾸눈 J 1장
	bool bDM_OneEyedJack;
	// 마이티 1장
	bool bDM_OnlyMighty;

	// 점수 (목:목표점수 기:기본점수 득:여당득점)

	// Mighty 2.0 호환 ( (목-기)*2+(득-목) vs (목-득) )
	bool bS_Use20;
	// Eye for an Eye ( (득-목) vs (목-득) )
	bool bS_Efe;
	// Modified Eye for an Eye ( (득-목)+1 vs (목-득) )
	bool bS_MEfe;
	// Base Ten ( (득-10) vs (목-득) )
	bool bS_Base10;
	// Base Thirteen ( (득-기) vs (목-득) )
	bool bS_Base13;
	// 런은 2배
	bool bS_DoubleForRun;
	// 선언된 런만 2배 ( bS_DoubleForRun 을 오버라이드함 )
	bool bS_DoubleForDeclaredRun;
	// 백런은 2배
	bool bS_DoubleForReverseRun;
	// 노기루다 2배
	bool bS_DoubleForNoKiruda;
	// 노프랜드 2배씩 더 주기 ( 실질적으로 5마에서 4배, 4마에서 6배가 됨 )
	bool bS_DoubleForNoFriend;
	// 런일때 고정액 있음 ( ..ForRun 을 오버라이드함 )
	// 이 값은 20 으로 한다
	bool bS_StaticRun;

	// 진행

	// 첫 득점때 프랜드 밝힘
	bool bShowFriend;
	// 득점 카드는 뒤집어 놓음
	bool bHideScore;

	// 카드 정의

	// 마이티
	int nMighty;
	// 대체 마이티
	int nAlterMighty;
	// 조커콜
	int nJokercall;
	// 대체 조커콜
	int nAlterJokercall;
};

#endif // !defined(AFX_RULE_H__6433F792_C1B5_11D2_97F2_000000000000__INCLUDED_)
