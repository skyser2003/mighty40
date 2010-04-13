// PlayerMai.h: interface for the CPlayerMai class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYERMAI_H__96075965_D2BC_11D2_981A_0000212035B8__INCLUDED_)
#define AFX_PLAYERMAI_H__96075965_D2BC_11D2_981A_0000212035B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CInfoBar;
class CMsg;

// MAI-DLL 을 기반으로 하는 플레이어 객체
// 적당한 리턴 값을 넘겨줄 수 있게 되면
// call-back 윈도우에 WM_PLAYER 메시지를 보내준다
// 이 때 LPARAM 값은 적절한 CMsg* 값
// 이 값은 사용자가 해제 해야 한다


class CPlayerMai : public CPlayer
{
public:
	CPlayerMai(
		int nID,		// 고유 번호 ( 6명의 플레이어중 순서 )
		LPCTSTR sName,	// 이름
		CWnd* pCallback,// call-back window
		LPCTSTR sDLLPath,// MAI-DLL 경로
		LPCTSTR sOption);// 옵션 스트링 (null 일 경우 초기값)
	virtual ~CPlayerMai();

	// DLL 이 제대로 로드되었는가는 이 함수로 알 수 있다
	bool IsDLLWorking() const						{ return m_pos ? true : false; }

	// 이 플레이어는 컴퓨터 AI 인가
	virtual bool IsComputer() const					{ return true; }

	// CPlayer 인터페이스 구현

	virtual void OnSelect2MA( int* selecting, CCard* pcShow, CEvent* e );
	virtual void OnKillOneFromSix( CCard* pcCardToKill,
		CCardList* plcFailedCardsTillNow, CEvent* );
	virtual void OnElection( CGoal* pNewGoal, CEvent* );
	virtual void OnTurn( CCard* pc, int* pe, CEvent* e );

protected:
	// 헬퍼 멤버 & 함수

	// update 를 위한 CInfoBar 캡슐
	class MAIDLL_UPDATE_IMPL : public MAIDLL_UPDATE
	{
	public:
		MAIDLL_UPDATE_IMPL() : m_pInfo(Ib()) {}
		void SetProgress( int nPercentage );
		void SetText( LPCTSTR sText );
	protected:
		CInfoBar* m_pInfo;
	};

	// DLL exported 함수 포인터
	struct MAI_FUNCTIONS {
		pfMaiGetInfo		MaiGetInfo;
		pfMaiGetPlay		MaiGetPlay;
		pfMaiOption			MaiOption;
		pfMaiReleasePlay	MaiReleasePlay;
	} m_funcs;

	// update 객체
	MAIDLL_UPDATE_IMPL m_update;

	// 이벤트
	CEvent m_eTemp;

	// 정적으로 DLL 들을 관리한다
	struct MAIDLL {
		CString sPath;			// DLL 패스
		HMODULE hDLL;			// 핸들
		MAI_FUNCTIONS funcs;	// 함수들
		int ref;				// 참조 카운터
	};
	static CList< MAIDLL, MAIDLL& > s_lMaiDll;
	static CCriticalSection s_csMaiDll;

	// s_lMaiDll 내에서의 이 DLL 로의 포인터
	// 이 값이 0 이면 이 PlayerMai 객체는 invalid 한 것이다
	POSITION m_pos;

	// DLL 에서 함수 포인터를 얻고 (m_funcs),
	// CPlay 객체를 얻어 옴
	// 만일 s_lMaiDll 에 이미 같은 DLL 이 있는 경우 ref 값만 증가됨
	bool LoadMaiDll( LPCTSTR sPath, LPCTSTR sOption, MAIDLL_UPDATE* );
	// CPlay 객체를 삭제함 - 만일 해당 DLL 의 ref 가
	// 0 이 되는 경우 그 DLL 도 Unload 함
	bool UnloadMaiDll();
	// 디폴트 AI 를 로드함 ( DLL 읽기 실패시 )
	void LoadDefault( LPCTSTR sOption, MAIDLL_UPDATE* );

	// 최소 nMillisecToEnsure 를 기다리게 하는 모래시계
	struct CSandClock {
		CSandClock( long nMillisecToEnsure )
			: m_nMillisecToEnsure( nMillisecToEnsure ) {
			m_dwLastTick = ::GetTickCount(); }
		~CSandClock() {
			long diff = (long)( ::GetTickCount() - m_dwLastTick );
			if ( diff < m_nMillisecToEnsure )
				Sleep( min( m_nMillisecToEnsure - diff, 5000 ) );
			else Sleep(0); }
		long m_nMillisecToEnsure;
		DWORD m_dwLastTick;
	};
	friend CSandClock;
};

#endif // !defined(AFX_PLAYERMAI_H__96075965_D2BC_11D2_981A_0000212035B8__INCLUDED_)
