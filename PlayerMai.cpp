// PlayerMai.cpp: implementation of the CPlayerMai class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "Play.h"
#include "Player.h"
#include "PlayerMai.h"
#include "MaiBSW.h"
#include "InfoBar.h"
#include "MFSM.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// update 를 위한 CInfoBar 캡슐
void CPlayerMai::MAIDLL_UPDATE_IMPL::SetProgress( int nPercentage )
{
	m_pInfo->SetProgress( nPercentage );
}

void CPlayerMai::MAIDLL_UPDATE_IMPL::SetText( LPCTSTR sText )
{
	m_pInfo->SetText( sText );
}


/////////////////////////////////////////////////////////////////////////////
// CPlayerMai

// 정적 변수들

// dll
CList< CPlayerMai::MAIDLL, CPlayerMai::MAIDLL& > CPlayerMai::s_lMaiDll;
CCriticalSection CPlayerMai::s_csMaiDll;

/////////////////////////////////////////////////////////////////////////////
// CPlayerMai 본체

CPlayerMai::CPlayerMai(
	int nID,		// 고유 번호 ( 6명의 플레이어중 순서 )
	LPCTSTR sName,	// 이름
	CWnd* pCallback,// call-back window
	LPCTSTR sDLLPath,// MAI-DLL 경로
	LPCTSTR sOption) // 옵션 스트링 (null 일 경우 초기값)
	: CPlayer( nID, sName, pCallback )
{
	m_pos = 0;

	// DLL 을 로드하고 바인드 한다
	// 패스가 공백이거나 해당 DLL 로드가 실패했으면
	// 기본 AI 를 로드한다
	if ( !sDLLPath || !*sDLLPath
			|| !LoadMaiDll( sDLLPath, sOption, &m_update ) )
		LoadDefault( sOption, &m_update );
}

CPlayerMai::~CPlayerMai()
{
	VERIFY( UnloadMaiDll() );
}

// DLL 에서 함수 포인터를 얻고 (m_funcs),
// CPlay 객체를 얻어 옴
// 만일 s_lMaiDll 에 이미 같은 DLL 이 있는 경우 ref 값만 증가됨
bool CPlayerMai::LoadMaiDll( LPCTSTR sPath, LPCTSTR sOption, MAIDLL_UPDATE* pUpdate )
{
	s_csMaiDll.Lock();

	// s_lMaiDll 에 같은 이름의 DLL 이 있는가 조사한다
	POSITION pos = s_lMaiDll.GetHeadPosition();
	for ( ; pos; s_lMaiDll.GetNext(pos) ) {
		MAIDLL& dll = s_lMaiDll.GetAt(pos);

		if ( !dll.sPath.CompareNoCase( sPath ) ) {
			// 발견 !
			m_pos = pos;
			m_funcs = dll.funcs;
			m_pPlay = m_funcs.MaiGetPlay( sOption, pUpdate );
			dll.ref++;
			s_csMaiDll.Unlock();
			return true;
		}
	}
	ASSERT( !pos );
	s_csMaiDll.Unlock();

	// 미발견 - 실제로 로드해서 얻는다

	HMODULE hDLL = ::LoadLibrary( sPath );
	if ( !hDLL ) return false;

	m_funcs.MaiGetInfo = (pfMaiGetInfo)::GetProcAddress( hDLL, "MaiGetInfo" );
	m_funcs.MaiGetPlay = (pfMaiGetPlay)::GetProcAddress( hDLL, "MaiGetPlay" );
	m_funcs.MaiOption = (pfMaiOption)::GetProcAddress( hDLL, "MaiGetOption" );
	m_funcs.MaiReleasePlay = (pfMaiReleasePlay)::GetProcAddress( hDLL, "MaiReleasePlay" );

	if ( !m_funcs.MaiGetInfo
		|| !m_funcs.MaiGetPlay
		|| !m_funcs.MaiReleasePlay ) return false;

	// 이제 정보를 기록하고 CPlay 를 얻는다

	MAIDLL m;
	m.hDLL = hDLL;
	m.funcs = m_funcs;
	m.ref = 1;
	m.sPath = sPath;
	m_pPlay = m.funcs.MaiGetPlay( sOption, pUpdate );

	s_csMaiDll.Lock();
	m_pos = s_lMaiDll.AddTail( m );
	s_csMaiDll.Unlock();

	return true;
}

// CPlay 객체를 삭제함 - 만일 해당 DLL 의 ref 가
// 0 이 되는 경우 그 DLL 도 Unload 함
bool CPlayerMai::UnloadMaiDll()
{
	ASSERT( m_pPlay );

	// CPlay 객체를 돌려준다
	CString sOption;
	m_funcs.MaiReleasePlay( m_pPlay, &sOption );
	m_pPlay = 0;

	// 돌려받은 옵션 스트링을 저장한다
//	Mo()->aPlayer[GetID()].sSetting = sOption;
	// 수정 : 옵션 스트링은 저장할 필요가 없다
	//        플레이어 옵션 대화상자에서 저장한다

	if ( m_pos ) {

		s_csMaiDll.Lock();
		MAIDLL& dll = s_lMaiDll.GetAt( m_pos );

		if ( ! --dll.ref ) {
			// 참조 카운터가 0 이 되었다
			if ( !::FreeLibrary( dll.hDLL ) ) {
				ASSERT(0);
				s_csMaiDll.Unlock();
				return false;
			}
			s_lMaiDll.RemoveAt( m_pos );
			m_pos = 0;
		}
		s_csMaiDll.Unlock();
	}
	return true;
}

// 디폴트 AI 를 로드함 ( DLL 읽기 실패시 )
void CPlayerMai::LoadDefault( LPCTSTR sOption, MAIDLL_UPDATE* pUpdate )
{
	m_funcs.MaiGetInfo = CMaiBSW::MaiGetInfo;
	m_funcs.MaiGetPlay = CMaiBSW::MaiGetPlay;
	m_funcs.MaiOption = CMaiBSW::MaiOption;
	m_funcs.MaiReleasePlay = CMaiBSW::MaiReleasePlay;

	m_pos = 0;
	m_pPlay = m_funcs.MaiGetPlay( sOption, pUpdate );
}

// CPlayer 구현

void CPlayerMai::OnSelect2MA( int* selecting, CCardList* plCard, CEvent* e )
{
	CSandClock c(DELAY_KILL);

	// CPlayer::OnSelect2MA( pcShow, pcHide, e );
	// Ib()->SetProgress( 0 );	// 상태바의 진행률 표시를 리셋

	m_pMFSM->RequestCallAIThread( m_pPlay, 3, &m_eTemp, selecting, plCard );

	m_pMFSM->WaitEvent( &m_eTemp );

	Ib()->SetProgress( 0 );
	e->SetEvent();
}

void CPlayerMai::OnKillOneFromSix( CCard* pcCardToKill,
	CCardList* plcFailedCardsTillNow, CEvent* e )
{
	CSandClock c(DELAY_KILL);

	// CPlayer::OnKillOneFromSix( pcCardToKill, plcFailedCardsTillNow, e );
	// Ib()->SetProgress( 0 );	// 상태바의 진행률 표시를 리셋

	m_pMFSM->RequestCallAIThread( m_pPlay, 1, &m_eTemp, pcCardToKill, plcFailedCardsTillNow );

	m_pMFSM->WaitEvent( &m_eTemp );

	Ib()->SetProgress( 0 );
	e->SetEvent();
}

void CPlayerMai::OnElection( CGoal* pNewGoal, CEvent* e )
{
	CSandClock c(DELAY_GOAL);

	// CPlayer::OnElection( pNewGoal, e );

	m_pMFSM->RequestCallAIThread( m_pPlay, 0, &m_eTemp, pNewGoal );

	m_pMFSM->WaitEvent( &m_eTemp );

	Ib()->SetProgress( 0 );
	e->SetEvent();
}

void CPlayerMai::OnTurn( CCard* pc, int* pe, CEvent* e )
{
	CSandClock c( Mo()->nDelayOneCard * 100 );

	// CPlayer::OnTurn( pc, pe, e );

	m_pMFSM->RequestCallAIThread( m_pPlay, 2, &m_eTemp, pc, pe );

	m_pMFSM->WaitEvent( &m_eTemp );

	Ib()->SetProgress( 0 );
	e->SetEvent();
}
