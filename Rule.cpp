// Rule.cpp: implementation of the CRule class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Rule.h"
#include "card.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define USE_PRESET

static LPCTSTR asPreset[][2] = {
	{ STANDARD_RULE_STRING, _T("표준 5마") },
	{ _T("'1$1M&%)2K5'3,$"), _T("표준 4마") },
	{ _T("&/$1M&#)2K5'3,$"), _T("표준 3마") },
	{ _T(")0$1M&))2K5'3,$"), _T("표준 6마") },
	{ _T("*0$1M&))2K%'3,$"), _T("표준 7마") },
	{ _T("%*$1M&#)2K5'3,$"), _T("표준 2마") },
	{ _T("(0$1M@IYb-A(C@%"), _T("SNUCSE 5마") },
	{ _T(")1$1M@IYb-1(C@%"), _T("SNUCSE 6마") },
	{ _T("*1$1M@IYb-1(C@%"), _T("SNUCSE 7마") },
	{ _T(")0$1M@I7BK]F30$"), _T("관악 6마") },
	{ _T("(1$1M&Z8B+](3@$"), _T("신촌 5마") },
	{ _T("(/$1M@1-bb&'3@$"), _T("경기 5마") },
	{ _T("'0$1M&E7BKU(30$"), _T("부산 4마") },
	{ _T("(/$1M&I8@K5'30$"), _T("대구 5마") },
	{ _T(")0$1M&I7BK]*30$"), _T("대전 6마") },
	{ _T("(/$1M&*9P-5'30$"), _T("광주 5마") },
	{ _T("&/$1M&#%BK](30$"), _T("제주 3마") },
};

// 미리 정의된 표준 룰로 세트
// (리턴값은 그 룰의 이름, NULL 이면 해당 룰이 없음)
// 1 : 표준 5마  2 : 표준 4마  3 : 표준 3마
// 4 : 표준 6마
LPCTSTR CRule::Preset( int nRule )
{
	nRule--;
	if ( nRule < 0 || nRule >= sizeof(asPreset)/sizeof(LPCTSTR[2]) ) {
		return 0;
	}

#ifdef USE_PRESET
	Decode( asPreset[nRule][0] );
#else
	nPlayerNum = 5;
	bHideScore = false;
	bFriend = true;
	bJokerFriend = true;
	bShowFriend = false;
	bAttScoreThrownPoints = false;
	bFriendGetsBeginer = false;
	nMinScore = 13;
	bHighScore = false;
	bBeginerPass = true;
	bRaise2ForKirudaChange = true;
	bPassAgain = false;
	bNoKirudaAdvantage = false;
	bNoKirudaAlways = false;
	bInitMighty = true;
	bInitMightyEffect = true;
	bLastMighty = true;
	bLastMightyEffect = true;
	bOverrideMighty = false;
	bInitJoker = false;
	bInitJokerEffect = false;
	bLastJoker = false;
	bLastJokerEffect = false;
	bInitKiruda = true;
	bInitBeginKiruda = false;
	bInitJokercallEffect = true;
	bJokercallJokerEffect = false;
	bDM_NoPoint = true;
	bDM_AllPoint = false;
	bDM_JokerIsReversePoint = false;
	bDM_Only10 = true;
	bDM_OneEyedJack = false;
	bDM_OnlyMighty = false;
	bDM_OnlyOne = false;
	bS_Use20 = false;
	bs_Use40 = true;
	bs_Call = false;
	bS_Efe = false;
	bS_MEfe = false;
	bS_Base10 = false;
	bS_Base13 = false;
	bS_BaseM = false;
	bSS_Efe = true;
	bSS_Tft = false;
	bS_DoubleForRun = true;
	bS_DoubleForDeclaredRun = false;
	bS_DoubleForReverseRun = false;
	bS_DoubleForNoKiruda = true;
	bS_DoubleForNoFriend = true;
	bS_StaticRun = false;
	bS_AGoalReverse = true;
	bS_A11Reverse = false;
	bS_AMReverse = false;
	nMighty = (int)CCard( SPADE, ACE);
	nAlterMighty = (int)CCard( DIAMOND, ACE );
	nJokercall = (int)CCard( CLOVER, 3 );
	nAlterJokercall = (int)CCard( SPADE, 3 );
#endif
	return asPreset[nRule][1];
}

// 인코드될 문자중 가장 작은 문자
#define BASE_CHAR  35

// 0 에서 63 까지의 정수를 문자 포인터에 삽입
static void PushInt( TCHAR* p, int n )
{
	ASSERT( n >= 0 && n < 64 );
	*p = (TCHAR)(n+BASE_CHAR);
}

// 위의 값을 반대로 추출
static void PopInt( const TCHAR* p, int& n )
{
	int np = (int)*p;
	ASSERT( np >= BASE_CHAR && np < 99 );
	n = np - BASE_CHAR;
}

// 하나의 문자를 0 에서 5 까지의 엔트리가 있는
// 비트맵으로서 한 비트를 세트
static void PushBool( TCHAR* p, int nPos, bool bValue )
{
	int np = (int)*p;
	ASSERT( nPos >= 0 && nPos < 6 );
	ASSERT( np >= BASE_CHAR && np < 127 );	//v4.0
	if ( bValue ) *p = (TCHAR)(((np-BASE_CHAR) | (1 << nPos)) + BASE_CHAR);
	else *p = (TCHAR)(((np-BASE_CHAR) & ~(1 << nPos)) + BASE_CHAR);
}

// 위의 값을 반대로 추출
static void PopBool( const TCHAR* p, int nPos, bool& bValue )
{
	int np = (int)*p;
	ASSERT( nPos >= 0 && nPos < 6 );
	ASSERT( np >= BASE_CHAR && np < 127 );	//v4.0
	bValue = ( (np-BASE_CHAR) & (1 << nPos) ) ? true : false;
}

#define SYNC_RULE_INT( VALUE ) \
	{	PushInt( p++, VALUE ); n = 0; }
#define SYNC_RULE_BOOL( VALUE ) \
	{	PushBool( p, n++, VALUE ); if ( n == 6 ) p++, n=0; }

// 현재 룰을 스트링으로 인코드해서 리턴
CString CRule::Encode() const
{
	TCHAR* pEncode = new TCHAR[64];
	for ( int w = 0; w < 64; w++ ) pEncode[w] = BASE_CHAR;

	TCHAR* p = pEncode;
	int n = 0;

	SYNC_RULE_INT( nPlayerNum );
	SYNC_RULE_INT( nMinScore );
	SYNC_RULE_INT( nMighty );
	SYNC_RULE_INT( nAlterMighty );
	SYNC_RULE_INT( nJokercall );
	SYNC_RULE_INT( nAlterJokercall );
	SYNC_RULE_BOOL( bHideScore );
	SYNC_RULE_BOOL( bFriend );
	SYNC_RULE_BOOL( bJokerFriend );
	SYNC_RULE_BOOL( bShowFriend );
	SYNC_RULE_BOOL( bAttScoreThrownPoints );
	SYNC_RULE_BOOL( bFriendGetsBeginer );
	SYNC_RULE_BOOL( bHighScore );
	SYNC_RULE_BOOL( bBeginerPass );
	SYNC_RULE_BOOL( bRaise2ForKirudaChange );
	SYNC_RULE_BOOL( bPassAgain );
	SYNC_RULE_BOOL( bNoKirudaAdvantage );
	SYNC_RULE_BOOL( bNoKirudaAlways );
	SYNC_RULE_BOOL( bInitMighty );
	SYNC_RULE_BOOL( bInitMightyEffect );
	SYNC_RULE_BOOL( bLastMighty );
	SYNC_RULE_BOOL( bLastMightyEffect );
	SYNC_RULE_BOOL( bOverrideMighty );
	SYNC_RULE_BOOL( bInitJoker );
	SYNC_RULE_BOOL( bInitJokerEffect );
	SYNC_RULE_BOOL( bLastJoker );
	SYNC_RULE_BOOL( bLastJokerEffect );
	SYNC_RULE_BOOL( bInitKiruda );
	SYNC_RULE_BOOL( bInitBeginKiruda );
	SYNC_RULE_BOOL( bInitJokercallEffect );
	SYNC_RULE_BOOL( bJokercallJokerEffect );
	SYNC_RULE_BOOL( bDM_NoPoint );
	SYNC_RULE_BOOL( bDM_AllPoint );
	SYNC_RULE_BOOL( bDM_JokerIsReversePoint );
	SYNC_RULE_BOOL( bDM_Only10 );
	SYNC_RULE_BOOL( bDM_OneEyedJack );
	SYNC_RULE_BOOL( bDM_OnlyMighty );
	SYNC_RULE_BOOL( bDM_OnlyOne );
	SYNC_RULE_BOOL( bS_Use20 );
	SYNC_RULE_BOOL( bS_Use40 );
	SYNC_RULE_BOOL( bS_Call );
	SYNC_RULE_BOOL( bS_Efe );
	SYNC_RULE_BOOL( bS_MEfe );
	SYNC_RULE_BOOL( bS_Base10 );
	SYNC_RULE_BOOL( bS_Base13 );
	SYNC_RULE_BOOL( bS_BaseM );
	SYNC_RULE_BOOL( bSS_Efe );
	SYNC_RULE_BOOL( bSS_Tft );
	SYNC_RULE_BOOL( bS_DoubleForRun );
	SYNC_RULE_BOOL( bS_DoubleForDeclaredRun );
	SYNC_RULE_BOOL( bS_DoubleForReverseRun );
	SYNC_RULE_BOOL( bS_DoubleForNoKiruda );
	SYNC_RULE_BOOL( bS_DoubleForNoFriend );
	SYNC_RULE_BOOL( bS_StaticRun );
	SYNC_RULE_BOOL( bS_AGoalReverse );
	SYNC_RULE_BOOL( bS_A11Reverse );
	SYNC_RULE_BOOL( bS_AMReverse );

	if ( n == 0 ) *p = '\0';
	else *(p+1) = '\0';

	CString ret = pEncode;
	delete[] pEncode;
	return ret;
}

#undef SYNC_RULE_INT
#undef SYNC_RULE_BOOL

#define SYNC_RULE_INT( VALUE ) \
	{	PopInt( p++, VALUE ); n = 0; }
#define SYNC_RULE_BOOL( VALUE ) \
	{	PopBool( p, n++, VALUE ); if ( n == 6 ) p++, n=0; }

// 스트링에서 새로운 룰을 디코드함
void CRule::Decode( LPCTSTR sRule )
{
	const TCHAR* p = sRule;
	int n = 0;

	SYNC_RULE_INT( nPlayerNum );
	SYNC_RULE_INT( nMinScore );
	SYNC_RULE_INT( nMighty );
	SYNC_RULE_INT( nAlterMighty );
	SYNC_RULE_INT( nJokercall );
	SYNC_RULE_INT( nAlterJokercall );
	SYNC_RULE_BOOL( bHideScore );
	SYNC_RULE_BOOL( bFriend );
	SYNC_RULE_BOOL( bJokerFriend );
	SYNC_RULE_BOOL( bShowFriend );
	SYNC_RULE_BOOL( bAttScoreThrownPoints );
	SYNC_RULE_BOOL( bFriendGetsBeginer );
	SYNC_RULE_BOOL( bHighScore );
	SYNC_RULE_BOOL( bBeginerPass );
	SYNC_RULE_BOOL( bRaise2ForKirudaChange );
	SYNC_RULE_BOOL( bPassAgain );
	SYNC_RULE_BOOL( bNoKirudaAdvantage );
	SYNC_RULE_BOOL( bNoKirudaAlways );
	SYNC_RULE_BOOL( bInitMighty );
	SYNC_RULE_BOOL( bInitMightyEffect );
	SYNC_RULE_BOOL( bLastMighty );
	SYNC_RULE_BOOL( bLastMightyEffect );
	SYNC_RULE_BOOL( bOverrideMighty );
	SYNC_RULE_BOOL( bInitJoker );
	SYNC_RULE_BOOL( bInitJokerEffect );
	SYNC_RULE_BOOL( bLastJoker );
	SYNC_RULE_BOOL( bLastJokerEffect );
	SYNC_RULE_BOOL( bInitKiruda );
	SYNC_RULE_BOOL( bInitBeginKiruda );
	SYNC_RULE_BOOL( bInitJokercallEffect );
	SYNC_RULE_BOOL( bJokercallJokerEffect );
	SYNC_RULE_BOOL( bDM_NoPoint );
	SYNC_RULE_BOOL( bDM_AllPoint );
	SYNC_RULE_BOOL( bDM_JokerIsReversePoint );
	SYNC_RULE_BOOL( bDM_Only10 );
	SYNC_RULE_BOOL( bDM_OneEyedJack );
	SYNC_RULE_BOOL( bDM_OnlyMighty );
	SYNC_RULE_BOOL( bDM_OnlyOne );
	SYNC_RULE_BOOL( bS_Use20 );
	SYNC_RULE_BOOL( bS_Use40 );
	SYNC_RULE_BOOL( bS_Call );
	SYNC_RULE_BOOL( bS_Efe );
	SYNC_RULE_BOOL( bS_MEfe );
	SYNC_RULE_BOOL( bS_Base10 );
	SYNC_RULE_BOOL( bS_Base13 );
	SYNC_RULE_BOOL( bS_BaseM );
	SYNC_RULE_BOOL( bSS_Efe );
	SYNC_RULE_BOOL( bSS_Tft );
	SYNC_RULE_BOOL( bS_DoubleForRun );
	SYNC_RULE_BOOL( bS_DoubleForDeclaredRun );
	SYNC_RULE_BOOL( bS_DoubleForReverseRun );
	SYNC_RULE_BOOL( bS_DoubleForNoKiruda );
	SYNC_RULE_BOOL( bS_DoubleForNoFriend );
	SYNC_RULE_BOOL( bS_StaticRun );
	SYNC_RULE_BOOL( bS_AGoalReverse );
	SYNC_RULE_BOOL( bS_A11Reverse );
	SYNC_RULE_BOOL( bS_AMReverse );
}
