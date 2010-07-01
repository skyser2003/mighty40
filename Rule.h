// Rule.h: interface for the CRule class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RULE_H__6433F792_C1B5_11D2_97F2_000000000000__INCLUDED_)
#define AFX_RULE_H__6433F792_C1B5_11D2_97F2_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STANDARD_RULE_STRING _T("(0$1M&))2O%,C5%")

#define BASIC_PRESETS 17

// 마이티 게임 규칙

struct CRule  
{
	CRule() { Preset( 1 ); }

	// 규칙을 로드한다.
	// 사용자가 추가한 기본 규칙이 저장되어 있는
	// 파일이 있다면, 이 파일로부터 규칙을 로드한다.
	static void LoadPreset();
	// 사용자가 추가한 규칙을 기본 규칙이 있는 파일로 저장한다.
	static void SavePreset();
	// 기본 규칙 목록에 사용자 정의 규칙을 추가한다.
	static void AddPreset( CString ruleString, CString ruleName );
	// 기본 규칙 목록에서 사용자 정의 규칙을 제거한다.
	static void RemovePreset( int rulenum );

	// 미리 정의된 표준 룰로 세트
	// (리턴값은 그 룰의 이름, NULL 이면 해당 룰이 없음)
	// 1 : 표준 5마  2 : 표준 4마  3 : 표준 3마
	// 4 : 표준 6마
	LPCTSTR Preset( int nRule );

	// 룰의 이름을 리턴
	static CString GetName( int nRule );
	// 현재 룰을 스트링으로 인코드해서 리턴
	CString Encode() const;
	// 스트링에서 새로운 룰을 디코드함
	void Decode( LPCTSTR sRule );

	/////////////////////////////////////////////////////////////////////////

	// 마이티 인원

	// 인원 ( 2-7 : v4.0 )
	int nPlayerNum;

	// 진행

	// 득점 카드는 뒤집어 놓음
	bool bHideScore;
	// 프랜드가 있는가
	bool bFriend;
	// 조커를 프랜드로 부를 수 있는가
	bool bJokerFriend;
	// 첫 득점때 프랜드 밝힘
	bool bShowFriend;
	// 버린 카드는 야당의 득점으로 치는가
	bool bAttScoreThrownPoints;
	// 다음판의 첫 출마자는 이전판의 프랜드인가 (false:전판선)
	bool bFriendGetsBeginer;

	// 선거

	// 최소 기본 점수 ( 4 - 20 )
	int nMinScore;
	// 기본점수를 20 보다 높이 부를 수 있음 (25까지==HIGHSCORE_MAX_LIMIT)
	bool bHighScore;
	// 선이 패스할 수 있는가
	bool bBeginerPass;
	// 기루다 바꿀 때 추가로 올리는 점수가 2 인가 (false:1)
	bool bRaise2ForKirudaChange;
	// 패스한 사람이 다시 부를 수 있는가
	bool bPassAgain;
	// 노기루시 처음과 바꿀 때 -1
	bool bNoKirudaAdvantage;
	// 노기루시 위와 다른 경우 -1
	bool bNoKirudaAlways;

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
	// 점수카드 1장
	bool bDM_OnlyOne;
	// 모두 점수카드일때
	bool bDM_AllPoint;
	// 조커를 역 점수카드로 계산
	bool bDM_JokerIsReversePoint;
	// 마이티 1장
	bool bDM_OnlyMighty;
	// 점수카드 10
	bool bDM_Only10;
	// 애꾸눈 J 1장
	bool bDM_OneEyedJack;
	// 10, 애꾸눈 잭과 조커&마이티 중복 적용
	bool bDM_Duplicate;

	// 점수 (목:목표점수 기:기본점수 득:여당득점)

	// 여당
	// 위험보상 2.0 ( (목-기)*2+(득-목) )
	bool bS_Use20;
	// 위험보상 4.0 ( (목-기)*1.5+(득-목) )
	bool bS_Use40;
	// 부르는게 값 ( (목-기+1)*2 )
	bool bS_Call;
	// Eye for an Eye ( (득-목) )
	bool bS_Efe;
	// Modified Eye for an Eye ( (득-목)+1 )
	bool bS_MEfe;
	// Base Ten ( (득-10) )
	bool bS_Base10;
	// Base Thirteen ( (득-13) )
	bool bS_Base13;
	// Base Min ( (득-기) )
	bool bS_BaseM;

	// 야당
	// Eye for an Eye( (목-득) )
	bool bSS_Efe;
	// Tooth for an Tooth( (득>=기)?(목-득):(목-기)+2*(기-득) )
	bool bSS_Tft;

	// 특수 보상

	// 런은 2배
	bool bS_DoubleForRun;
	// 선언된 런만 2배 ( bS_DoubleForRun 을 오버라이드함 )
	bool bS_DoubleForDeclaredRun;
	// 백런은 2배
	bool bS_DoubleForReverseRun;
	// 노기루다 2배
	bool bS_DoubleForNoKiruda;
	// 노프랜드 1.5배씩 더 주기 ( 실질적으로 5마에서 3배, 4마에서 4.5배가 됨 )
	bool bS_DoubleForNoFriend;
	// 런일때 고정액 있음 ( ..ForRun 을 오버라이드함 )
	// 이 값은 20 으로 한다
	bool bS_StaticRun;
	// 백런조건
	// 야당이 여당의 공약 이상을 땄을 때
	bool bS_AGoalReverse;
	// 야당이 11장 이상을 땄을 때
	bool bS_A11Reverse;
	// 야당이 기본 이상을 땄을 때
	bool bS_AMReverse;

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
