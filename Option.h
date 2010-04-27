// Option.h: interface for the COption class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTION_H__BE37E2C8_C23D_11D2_97F3_000000000000__INCLUDED_)
#define AFX_OPTION_H__BE37E2C8_C23D_11D2_97F3_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// 게임 옵션

struct COption  
{
	// 옵션을 레지스트리에 세이브
	void Save() const;
	// 옵션을 레지스트리에서 로드
	void Load();

	/////////////////////////////////////////////////////////////////////////

	// 일반

	// 시계 방향
	bool bClockwise;
	// 소리
	bool bUseSound;
	// 카드 정렬 안함
	bool bNoSort;
	// 기루다 왼쪽
	bool bLeftKiruda;
	// ACE 왼쪽
	bool bLeftAce;
	// 마이티 용어 사용
	bool bUseTerm;
	// 돈을 지지율로 표시
	bool bMoneyAsSupport;

	// 보스키 ( 4 종류 + 임의 )
	int nBossType;

	// 그림

	// 카드 뒷 그림 ( -1 : 사용자 정의 )
	int nBackPicture;
	// 카드 뒷 그림 경로
	CString sBackPicturePath;
	// 바탕색
	int nBackColor;
	// 야당 바탕 글자색
	int nAttColor;
	// 여당 바탕 글자색
	int nDefColor;
	// DSB 표준 글자색
	int nDSBText;
	// DSB 강조색 1
	int nDSBStrong1;
	// DSB 강조색 2
	int nDSBStrong2;
	// DSB 흐린색
	int nDSBGray;
	// DSB 불투명
	bool bDSBOpaque;
	// DSB 음영 방법
	int nDSBShadeMode;
	// 배경 그림 사용
	bool bUseBackground;
	// 배경 그림 경로
	CString sBackgroundPath;
	// 채우기 방법 ( true : 타일 false : 한번만 )
	bool bTile;
	// 채우기 방법 ( true : 확대 false : 가운데 )
	bool bExpand;
	// 카드 그림 확대/축소
	bool bZoomCard;
	// 카드 설명 사용
	bool bCardHelp;
	// 카드 팁 사용
	bool bCardTip;

	// 진행 속도

	// 카드 이동속도 0 - 10
	int nCardSpeed;
	// 부드러운 카드 이동 0 - 10
	int nCardMotion;
	// 하나 냈을 때의 대기 시간 0 - 10 (x100 millisecond)
	int nDelayOneCard;
	// 한 턴에 대한 대기 시간 0 - 10 (x200 millisecond)
	int nDelayOneTurn;
	// 카드 나눠 주는 에니메이션 보임
	bool bShowDealing;
	// 저사양
	bool bSlowMachine;

	// 규칙

	// 미리 정의된 규칙 - 0 일때 사용자 정의
	int nPreset;
	// 사용자 정의 규칙
	CString sCustom;
	// 실제 규칙 내용
	CRule rule;

	// AI

	// 각 플레이어에 대한 정보
	struct PLAYER {
		// 이름
		CString sName;
		// 사용하는 AI DLL
		CString sAIDLL;
		// 설정 스트링
		CString sSetting;

	} aPlayer[MAX_PLAYERS];
	// 2마 ~ 7마 전적
	int anPlayerState[MAX_PLAYERS-1][3];

	// 통신

	// 서버가 될 때 포트 번호
	int nPort;
	// 서버가 될 때 주소 (일부firewall)
	CString sAddress;
	// 이전에 접속했던 주소 기억
#	define ADDRESSBOOK_SIZE		4
	CString asAddressBook[ADDRESSBOOK_SIZE];
	// Mighty.Net 주소
	CString sMightyNetAddress;
	// 다른 사람들과의 게임 중에 힌트를 볼 수 있음
	bool bUseHintInNetwork;
	// 채팅창이 떠 있는 시간 (x2초)
	int nChatDSBTimeOut;
	// 채팅창 버퍼 크기
	int nChatBufferSize;

	// 윈도우 상태

	// 주 화면
	CRect rcLast;
	// 점수판
	bool bLastScoreboard;		// on/off
	CPoint pntLastScoreboard;	// 위치

	// 처음인가
	bool bInit;
};

#endif // !defined(AFX_OPTION_H__BE37E2C8_C23D_11D2_97F3_000000000000__INCLUDED_)
