// Option.cpp: implementation of the COption class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"
#include "Option.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define SYNC_OPTION_B( NAME, VALUE, DEFVALUE ) \
	AfxGetApp()->WriteProfileInt( sSection, NAME, (int)VALUE )
#define SYNC_OPTION_I( NAME, VALUE, DEFVALUE ) \
	AfxGetApp()->WriteProfileInt( sSection, NAME, VALUE )
#define SYNC_OPTION_S( NAME, VALUE, DEFVALUE ) \
	AfxGetApp()->WriteProfileString( sSection, NAME, VALUE )

// 옵션을 레지스트리에 세이브
void COption::Save() const
{
	int i;
	CString sSection;

	sSection = _T("General");

	SYNC_OPTION_B( _T("Sound"), bUseSound, true );
	SYNC_OPTION_B( _T("NoSort"), bNoSort, false );
	SYNC_OPTION_B( _T("LeftKiruda"), bLeftKiruda, true );
	SYNC_OPTION_B( _T("LeftAce"), bLeftAce, true );
	SYNC_OPTION_B( _T("UseTerm"), bUseTerm, false );
	SYNC_OPTION_B( _T("MoneyAsSupport"), bMoneyAsSupport, true );

	sSection = _T("Bosskey");

	SYNC_OPTION_I( _T("BossType"), nBossType, 0 );

	sSection = _T("Picture");

	SYNC_OPTION_I( _T("Back"), nBackPicture, 0 );
	SYNC_OPTION_S( _T("BackPath"), sBackPicturePath, _T("") );
	SYNC_OPTION_I( _T("BackColor"), nBackColor, (int)RGB(0,128,0) );
	SYNC_OPTION_I( _T("AttColor"), nAttColor, (int)RGB(255,255,255) );
	SYNC_OPTION_I( _T("DefColor"), nDefColor, (int)RGB(0,255,255) );
	SYNC_OPTION_I( _T("DSBText"), nDSBText, (int)RGB(255,255,255) );
	SYNC_OPTION_I( _T("DSBStrong1"), nDSBStrong1, (int)RGB(0,255,255) );
	SYNC_OPTION_I( _T("DSBStrong2"), nDSBStrong2, (int)RGB(255,255,0) );
	SYNC_OPTION_I( _T("DSBGray"), nDSBGray, (int)RGB(128,128,128) );
	SYNC_OPTION_B( _T("DSBOpaque"), bDSBOpaque, false );
	SYNC_OPTION_I( _T("DSBShadeMode"), nDSBShadeMode, 0 );
	SYNC_OPTION_B( _T("UseBackground"), bUseBackground, false );
	SYNC_OPTION_S( _T("BackgroundPath"), sBackgroundPath, _T("") );
	SYNC_OPTION_B( _T("Tiled"), bTile, true );
	SYNC_OPTION_B( _T("Expand"), bExpand, true );
	SYNC_OPTION_B( _T("ZoomCard"), bZoomCard, true );
	SYNC_OPTION_B( _T("CardHelp"), bCardHelp, true );
	SYNC_OPTION_B( _T("CardTip"), bCardTip, true );

	sSection = _T("Speed");

	SYNC_OPTION_I( _T("CardSpeed"), nCardSpeed, 5 );
	SYNC_OPTION_I( _T("CardMotion"), nCardMotion, 5 );
	SYNC_OPTION_I( _T("DelayOneCard"), nDelayOneCard, 5 );
	SYNC_OPTION_I( _T("DelayOneTurn"), nDelayOneTurn, 5 );
	SYNC_OPTION_B( _T("ShowDealing"), bShowDealing, true );
	SYNC_OPTION_B( _T("SlowMachine"), bSlowMachine, false );

	sSection = _T("Rule");

	SYNC_OPTION_I( _T("Preset"), nPreset, 1 );
	SYNC_OPTION_S( _T("Custom"), sCustom, STANDARD_RULE_STRING );

	for ( i = 0; i < MAX_PLAYERS; i++ ) {
		sSection.Format( _T("Player%d"), i );

		SYNC_OPTION_S( _T("Name"), aPlayer[i].sName, 0 );
		SYNC_OPTION_S( _T("AIDLL"), aPlayer[i].sAIDLL, 0 );
		SYNC_OPTION_S( _T("Setting"), aPlayer[i].sSetting, 0 );
	}

	sSection = _T("Player0");
	for ( i = 0; i < 3; i++ ) {
		CString sEntry; sEntry.Format( _T("State%d"), i );
		SYNC_OPTION_I( sEntry, anPlayerState[i], 0 );
	}

	sSection = _T("Communication");

	SYNC_OPTION_I( _T("Port"), nPort, 4111 );
	SYNC_OPTION_S( _T("Address"), sAddress, _T("") );
	for ( i = 0; i < ADDRESSBOOK_SIZE; i++ ) {
		CString sEntry; sEntry.Format( _T("Addr%d"), i );
		SYNC_OPTION_S( sEntry, asAddressBook[i], _T("") );
	}
	SYNC_OPTION_S( _T("MightyNet"), sMightyNetAddress, _T("") );
	SYNC_OPTION_B( _T("UseHintInGame"), bUseHintInNetwork, true );
	SYNC_OPTION_I( _T("ChatTimeOut"), nChatDSBTimeOut, 5 );
	SYNC_OPTION_I( _T("ChatBufferSize"), nChatBufferSize, 5 );

	sSection = _T("Windows");

	SYNC_OPTION_I( _T("LastL"), rcLast.left, 100 );
	SYNC_OPTION_I( _T("LastT"), rcLast.top, 30 );
	SYNC_OPTION_I( _T("LastR"), rcLast.right, 600 );
	SYNC_OPTION_I( _T("LastB"), rcLast.bottom, 550 );
	SYNC_OPTION_B( _T("Scoreboard"), bLastScoreboard, false );
	SYNC_OPTION_I( _T("ScoreboardLastX"), pntLastScoreboard.x, 540 );
	SYNC_OPTION_I( _T("ScoreboardLastY"), pntLastScoreboard.y, 50 );
	SYNC_OPTION_B( _T("Init"), bInit, true );
}

#undef SYNC_OPTION_B
#undef SYNC_OPTION_I
#undef SYNC_OPTION_S

#define SYNC_OPTION_B( NAME, VALUE, DEFVALUE ) \
	VALUE = ( AfxGetApp()->GetProfileInt( sSection, NAME, (int)DEFVALUE ) ? true : false )
#define SYNC_OPTION_I( NAME, VALUE, DEFVALUE ) \
	VALUE = AfxGetApp()->GetProfileInt( sSection, NAME, DEFVALUE )
#define SYNC_OPTION_S( NAME, VALUE, DEFVALUE ) \
	VALUE = AfxGetApp()->GetProfileString( sSection, NAME, DEFVALUE )

// 옵션을 레지스트리에서 로드
void COption::Load()
{
	static LPCTSTR s_sPlayerName[] = {
		_T(""),
		_T("대중"), _T("영삼"), _T("태우"), _T("두환"), _T("정희") };

	int i;
	CString sSection;

	sSection = _T("General");

	SYNC_OPTION_B( _T("Sound"), bUseSound, true );
	SYNC_OPTION_B( _T("NoSort"), bNoSort, false );
	SYNC_OPTION_B( _T("LeftKiruda"), bLeftKiruda, true );
	SYNC_OPTION_B( _T("LeftAce"), bLeftAce, true );
	SYNC_OPTION_B( _T("UseTerm"), bUseTerm, false );
	SYNC_OPTION_B( _T("MoneyAsSupport"), bMoneyAsSupport, true );

	sSection = _T("Bosskey");

	SYNC_OPTION_I( _T("BossType"), nBossType, 0 );

	sSection = _T("Picture");

	SYNC_OPTION_I( _T("Back"), nBackPicture, 0 );
	SYNC_OPTION_S( _T("BackPath"), sBackPicturePath, _T("") );
	SYNC_OPTION_I( _T("BackColor"), nBackColor, (int)RGB(0,128,0) );
	SYNC_OPTION_I( _T("AttColor"), nAttColor, (int)RGB(255,255,255) );
	SYNC_OPTION_I( _T("DefColor"), nDefColor, (int)RGB(0,255,255) );
	SYNC_OPTION_I( _T("DSBText"), nDSBText, (int)RGB(255,255,255) );
	SYNC_OPTION_I( _T("DSBStrong1"), nDSBStrong1, (int)RGB(0,255,255) );
	SYNC_OPTION_I( _T("DSBStrong2"), nDSBStrong2, (int)RGB(255,255,0) );
	SYNC_OPTION_I( _T("DSBGray"), nDSBGray, (int)RGB(128,128,128) );
	SYNC_OPTION_B( _T("DSBOpaque"), bDSBOpaque, false );
	SYNC_OPTION_I( _T("DSBShadeMode"), nDSBShadeMode, 2 );
	SYNC_OPTION_B( _T("UseBackground"), bUseBackground, false );
	SYNC_OPTION_S( _T("BackgroundPath"), sBackgroundPath, _T("") );
	SYNC_OPTION_B( _T("Tiled"), bTile, true );
	SYNC_OPTION_B( _T("Expand"), bExpand, true );
	SYNC_OPTION_B( _T("ZoomCard"), bZoomCard, true );
	SYNC_OPTION_B( _T("CardHelp"), bCardHelp, true );
	SYNC_OPTION_B( _T("CardTip"), bCardTip, true );

	sSection = _T("Speed");

	SYNC_OPTION_I( _T("CardSpeed"), nCardSpeed, 5 );
	SYNC_OPTION_I( _T("CardMotion"), nCardMotion, 5 );
	SYNC_OPTION_I( _T("DelayOneCard"), nDelayOneCard, 5 );
	SYNC_OPTION_I( _T("DelayOneTurn"), nDelayOneTurn, 5 );
	SYNC_OPTION_B( _T("ShowDealing"), bShowDealing, true );
	SYNC_OPTION_B( _T("SlowMachine"), bSlowMachine, false );

	sSection = _T("Rule");

	SYNC_OPTION_I( _T("Preset"), nPreset, 1 );
	SYNC_OPTION_S( _T("Custom"), sCustom, STANDARD_RULE_STRING );
	if ( nPreset ) rule.Preset( nPreset );
	else rule.Decode( sCustom );

	for ( i = 0; i < MAX_PLAYERS; i++ ) {
		sSection.Format( _T("Player%d"), i );

		SYNC_OPTION_S( _T("Name"), aPlayer[i].sName, s_sPlayerName[i] );
		SYNC_OPTION_S( _T("AIDLL"), aPlayer[i].sAIDLL, i % 2 ? _T("Xiao.dll") : _T("") );
		SYNC_OPTION_S( _T("Setting"), aPlayer[i].sSetting, _T("") );
	}

	// 플레이어 이름이 비어있으면 디폴트 사용자 이름으로 바꾼다
	if ( aPlayer[0].sName.IsEmpty() ) {
		DWORD dwNameLen = 0;
		::GetUserName( 0, &dwNameLen );
		if ( dwNameLen == 0 ) aPlayer[0].sName = _T("아무개");
		else {
			::GetUserName( aPlayer[0].sName.GetBuffer( dwNameLen ), &dwNameLen );
			aPlayer[0].sName.ReleaseBuffer();
		}
	}

	sSection = _T("Player0");
	for ( i = 0; i < 3; i++ ) {
		CString sEntry; sEntry.Format( _T("State%d"), i );
		SYNC_OPTION_I( sEntry, anPlayerState[i], 0 );
	}

	sSection = _T("Communication");

	SYNC_OPTION_I( _T("Port"), nPort, 4111 );
	SYNC_OPTION_S( _T("Address"), sAddress, _T("") );
	int j = 0;
	for ( i = 0; i < ADDRESSBOOK_SIZE; i++ ) {
		CString sEntry; sEntry.Format( _T("Addr%d"), i );
		SYNC_OPTION_S( sEntry, asAddressBook[j], _T("") );
		if ( !asAddressBook[j].IsEmpty() ) j++;
	}
	for ( ; j < ADDRESSBOOK_SIZE; j++ )
		asAddressBook[j].Empty();
	SYNC_OPTION_S( _T("MightyNet"), sMightyNetAddress, _T("") );
	SYNC_OPTION_B( _T("UseHintInGame"), bUseHintInNetwork, true );
	SYNC_OPTION_I( _T("ChatTimeOut"), nChatDSBTimeOut, 5 );
	SYNC_OPTION_I( _T("ChatBufferSize"), nChatBufferSize, 5 );

	sSection = _T("Windows");

	SYNC_OPTION_I( _T("LastL"), rcLast.left, 100 );
	SYNC_OPTION_I( _T("LastT"), rcLast.top, 30 );
	SYNC_OPTION_I( _T("LastR"), rcLast.right, 600 );
	SYNC_OPTION_I( _T("LastB"), rcLast.bottom, 550 );
	SYNC_OPTION_B( _T("Scoreboard"), bLastScoreboard, true );
	SYNC_OPTION_I( _T("ScoreboardLastX"), pntLastScoreboard.x, 600 );
	SYNC_OPTION_I( _T("ScoreboardLastY"), pntLastScoreboard.y, 30 );
	SYNC_OPTION_B( _T("Init"), bInit, true );

	// bUseHintInNetwork 은 항상 false 이다
	bUseHintInNetwork = false;
}
