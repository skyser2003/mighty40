// DConnect.cpp: implementation of the DConnect class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"

#include "BoardWrap.h"
#include "DSB.h"
#include "DEtc.h"
#include "DStartUp.h"

#include "MFSM.h"
#include "ZSocket.h"
#include "PlayerSocket.h"
#include "SocketBag.h"

#include "DConnect.h"
#include "DRule.h"
#include "Option.h"
#include "InfoBar.h"


#include "Play.h"
#include "Player.h"
#include "PlayerHuman.h"
#include "PlayerNetwork.h"
#include "PlayerMai.h"
#include "PlayerDummy.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// 채팅 버퍼 라인수
#define CHAT_LINES  6

// 유틸리티들

static CString format_score( int d, int f, int a )
{
	CString sRet;
	sRet.Format( _T("%d/%d %d/%d %d/%d"),
		LOWORD(d), HIWORD(d), LOWORD(f), HIWORD(f), LOWORD(a), HIWORD(a) );
	return sRet;
}

static CString get_name( LPCTSTR sPath )
{
	if ( !sPath || !*sPath ) return CString(_T("<기본 AI>"));
	LPCTSTR sSlash = _tcsrchr( sPath, _T('\\') );
	if ( !sSlash ) return CString(sPath);
	else return CString( sSlash + 1 );
}

static void get_host( CString& s )
{
	CZInetAddr addr;
	if ( !CZInetAddr::GetHostAddr( &addr )
		|| !addr.GetAddr( &s ) ) {
		s = _T("Unknown");
	}
}

// 콤보박스에서 주소를 파싱해 온다
static void parse_address( CComboBox& combo, CString& sAddr, UINT& uPort )
{
	CString sFull;
	combo.GetWindowText( sFull );

	LPCTSTR sColon = _tcschr( sFull, _T(':') );
	if ( !sColon ) {
		// : 이 없으므로 sFull 이 곧 sAddr 이다
		sAddr = sFull;
		uPort = (UINT) Mo()->nPort;
	}
	else {
		// : 까지를 복사한다
		_tcsncpy( sAddr.GetBuffer( sColon - sFull ), sFull,
					sColon - sFull );
		sAddr.ReleaseBuffer( sColon - sFull );
		uPort = (UINT) _ttoi( sColon + 1 );
	}
}

static CString create_entermsg( LPCTSTR sName, long d, long f, long a, bool isSpectator )
{
	int w = (int)LOWORD(d)+(int)LOWORD(f)+(int)LOWORD(a);
	int l = (int)HIWORD(d)+(int)HIWORD(f)+(int)HIWORD(a);

	CString sEnterMsg;
	if(isSpectator)
		sEnterMsg.Format( _T("%s 님이 관전자로 입장하였습니다"), sName);
	else
	{
		sEnterMsg.Format( _T("%s 님이 입장하였습니다 - ")
			_T("주공:%d승%d패 프랜드:%d승%d패 야당:%d승%d패 승률:%d%%"),
			sName, LOWORD(d), HIWORD(d),
			LOWORD(f), HIWORD(f), LOWORD(a), HIWORD(a),
			(w+l) == 0 ? 0 : w*100/(w+l) );
	}
	return sEnterMsg;
}

static void update_addrlist( LPCTSTR sAddr )
{
	int i;
	// sAddr 과 같은 주소가 있나 조사한 후, 있으면 그 주소를 없앤다
	// 없으면 맨 끝의 주소를 없앤다
	for ( i = ADDRESSBOOK_SIZE - 1; i >= 0; i-- )
		if ( Mo()->asAddressBook[i] == sAddr ) break;

	if ( i < 0 ) i = ADDRESSBOOK_SIZE - 1;

	for ( ; i > 0; i-- )
		Mo()->asAddressBook[i] = Mo()->asAddressBook[i-1];

	Mo()->asAddressBook[0] = sAddr;
}


LPCTSTR DConnectPopup::s_asText[] = {
	_T("취소"), _T("이 플레이어를 추방"), _T("이 게임에서 영구적으로 추방")
};

LPTSTR DConnectPopup::s_atText1[] = {
	_T("취소"), _T("관전자 접속 거부"), _T("관전자 전부 추방")
};

LPTSTR DConnectPopup::s_atText2[] = {
	_T("취소"), _T("관전자 접속 허용"), _T("관전자 전부 추방")
};

/////////////////////////////////////////////////////////////////////////////
// DConnecting

BEGIN_MESSAGE_MAP( DConnectingComboEdit, CEdit )
	ON_WM_CHAR()
END_MESSAGE_MAP()

// Combo 에서 엔터가 눌렸을때 OnClick 으로 처리
void DConnectingComboEdit::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	if ( (TCHAR)nChar  == _T('\x0d') )
		m_pParent->OnClick( (LPVOID)0xffffffff );
	else if ( (TCHAR)nChar == _T('\x1b') )
		m_pParent->OnClick( (LPVOID)0 );
	else CEdit::OnChar( nChar, nRepCnt, nFlags );
}

void DConnectingComboEdit::SubclassCombobox( CComboBox& box )
{
	HWND hWnd = ::GetWindow( box.GetSafeHwnd(), GW_CHILD );
	while ( hWnd ) {
		if ( ::SendMessage( hWnd, WM_GETDLGCODE, 0, 0 ) & DLGC_WANTCHARS ) {
			VERIFY( SubclassWindow( hWnd ) );
			break;
		}
		else ::GetWindow( box.GetSafeHwnd(), GW_HWNDNEXT );
	}
}

DConnecting::~DConnecting()
{
	m_combo.DestroyWindow();
	delete m_pSocket;
}

void DConnecting::Create()
{
	// 접속 콤보상자를 생성한다
	m_combo.Create( WS_CHILD | WS_VISIBLE
		| CBS_AUTOHSCROLL | CBS_DROPDOWN | CBS_HASSTRINGS,
		CRect(0,0,0,0), *m_pBoard, 0 );
	m_combo.SetFocus();

	m_edit.m_pParent = this;
	m_edit.SubclassCombobox( m_combo );

	// 콤보 박스에, 주소 히스토리를 삽입한다 (최근것부터)
	for ( int i = 0; i < ADDRESSBOOK_SIZE; i++ )
		if ( !Mo()->asAddressBook[i].IsEmpty() )
			m_combo.AddString( Mo()->asAddressBook[i] );
	m_combo.SetCurSel( 0 );

	m_colOk = s_colCyan;

	SetFixed();
	SetModal();
	DSB::Create( 0, 0, 20, 13, -1 );
}

void DConnecting::Destroy()
{
	m_combo.ShowWindow( SW_HIDE );
	DSB::Destroy();
}

// 초기화 ( 생성된 후 호출됨 )
void DConnecting::OnInit()
{
	DSB::OnInit();

	// v4.0에서 추가(2010.1.7)
	m_spectator = false;
	RegisterHotspot( 3, 9, -1, -1, true, 0, _T("접속 형태: 플레이어"),
		&s_colWhite, &s_tdShade, &s_colWhite, &s_tdOutline,
		(LPVOID)1 );
	RegisterHotspot( 4, 11, -1, -1, true, 0, _T("  확인  "),
		&m_colOk, &s_tdShade, &m_colOk, &s_tdOutline,
		(LPVOID)0xffffffff );
	RegisterHotspot( 12, 11, -1, -1, true, 0, _T("  취소  "),
		&s_colCyan, &s_tdShade, &s_colCyan, &s_tdOutline,
		(LPVOID)0 );
}

// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
// 오버라이드 하여 구현해야 한다
// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
void DConnecting::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	CRect rcExam;

	if ( !m_pSocket ) {	// 접속중이 아닐 때

		// 설명
		PutText( pDC, _T("접속할 서버의 주소를 입력하세요"), 2, 1, true,
			s_colWhite, s_tdShade );
		rcExam = PutText( pDC, _T("형식: 주소(:포트번호)"), 2, 3, true,
			s_colWhite, s_tdShade );
		PutText( pDC, _T("예1: 123.45.123.45"), 2, 4, true,
			s_colWhite, s_tdShade );
		PutText( pDC, _T("예2: www.snu.ac.kr:4111"), 2, 5, true,
			s_colWhite, s_tdShade );
	}
	else {	// 접속중

		rcExam = PutText( pDC, _T("아래 주소로 접속중..."), 2, 3, true,
			s_colWhite, s_tdShade );
	}

	// 콤보 박스를 적절한 위치로 옮긴다
	CRect rcLast;
	m_combo.GetWindowRect( &rcLast );
	m_combo.GetParent()->ScreenToClient( &rcLast );

	CRect rc; GetRect(&rc);
	CRect rcCombo( CPoint( rc.left + rc.Width()/8,
						rcExam.bottom + GetDSBUnit()*3 ),
					CSize( rc.Width()*3/4, rc.Height() ) );

	if ( rcLast.TopLeft() != rcCombo.TopLeft() ) {
		m_combo.SetWindowPos( 0, rcCombo.left, rcCombo.top,
			rcCombo.Width(), rcCombo.Height(),
			SWP_NOZORDER | SWP_NOREDRAW | SWP_SHOWWINDOW | SWP_NOACTIVATE );
	}
}

// 핫 스팟을 클릭할 때 불리는 함수
// LPVOID 값이 0 이면 종료 (X자를 클릭) 요청
void DConnecting::OnClick( LPVOID pVoid )
{
	switch ( (int)pVoid ) {

	case 0: {
		(new DStartUp(m_pBoard))->Create( 1 );
		if ( m_pSocket ) {
			m_pSocket->ClearTrigger();
			delete m_pSocket; m_pSocket = 0;
		}
		Destroy();
		break;
	}

	case 0xffffffff: {	// 시작 !

		// 주소
		CString sAddr; UINT uPort;
		parse_address( m_combo, sAddr, uPort );
		CZInetAddr addr( sAddr, uPort );

		// 새로운 소켓을 생성
		if ( m_pSocket ) {
			ASSERT(0);
			m_pSocket->ClearTrigger();
			delete m_pSocket;
		}
		m_pSocket = new CPlayerSocket();

		if ( !m_pSocket->CreateClientSocket(addr) ) {
			// 연결 실패
			delete m_pSocket; m_pSocket = 0;
			(new DConnectFail(m_pBoard))->Create(
				_T("연결이 실패하였습니다 !"), 0, 0, 0 );
			Destroy();
		}
		else {
			// 연결 시도중 (DSB 를 Disable 한다)
			m_combo.EnableWindow( FALSE );
			Ib()->SetFocusToChat();
			m_colOk = s_colGray;	// ok 버튼을 회색으로

			CRect rc; GetRect( &rc );
			m_pBoard->UpdateDSB( &rc );

			// 핸들러를 설치
			m_pSocket->SetTrigger( (DWORD)(LPVOID)this, 0, SockProc );
		}

		break;
	}
	case 1: {	// 접속 형태 변경
		m_spectator = !m_spectator;
		DeleteHotspot( (LPVOID)1 );
		if(m_spectator)
			RegisterHotspot( 3, 9, -1, -1, true, 0, _T("접속 형태: 관전자"),
				&s_colYellow, &s_tdShade, &s_colYellow, &s_tdOutline,
				(LPVOID)1 );
		else RegisterHotspot( 3, 9, -1, -1, true, 0, _T("접속 형태: 플레이어"),
				&s_colWhite, &s_tdShade, &s_colWhite, &s_tdOutline,
				(LPVOID)1 );

		CRect rc; GetRect( &rc );
		m_pBoard->UpdateDSB( &rc );
	}
	}
}

// 소켓 핸들러
void DConnecting::SockProc( CPlayerSocket* pSocket, CMsg* pMsg,
						DWORD dwUser1, DWORD dwUser2 )
{
	ASSERT( pSocket == ((DConnecting*)(LPVOID)dwUser1)->m_pSocket );
	pSocket, dwUser2;	// unused
	((DConnecting*)(LPVOID)dwUser1)->SockProc( pMsg );
}

void DConnecting::Fail( LPCTSTR sMsg )
{
	if ( m_pSocket ) {
		m_pSocket->ClearTrigger();
		delete m_pSocket; m_pSocket = 0;
	}

	(new DConnectFail(m_pBoard))->Create(
		sMsg ? sMsg : _T("접속이 종료되었습니다"), 0, 0, 0 );
	Destroy();
}

void DConnecting::SockProc( CMsg* pMsg )
{
	AUTODELETE_MSG(pMsg);

	if ( pMsg->GetType() == CMsg::mmConnected ) {
		// 단계 1 : 접속 결과
		long nErr;
		if ( !pMsg->PumpLong( nErr )
			|| !pMsg->PumpLong( nErr )
			|| nErr != 0 )
			Fail( _T("연결이 실패하였습니다 !") );
		else {
			CString sAddr; m_combo.GetWindowText( sAddr );
			update_addrlist( sAddr );	// 주소록에 기록

			m_pSocket->SetTrigger( (DWORD)(LPVOID)this, 0, SockProc );
		}
	}
	else if ( pMsg->GetType() == CMsg::mmInit ) {
		// 단계 2 : 서버로 부터의 응답
		// 이제 DConnect DSB 를 띄울 수 있다
		long nVer, nPlayers;
		if ( !pMsg->PumpLong( nVer )
			|| !pMsg->PumpLong( nVer )
			|| !pMsg->PumpLong( nPlayers ) 
			|| nVer != MIGHTY_VERSION )
			Fail( _T("서버쪽의 버전이 다릅니다 !") );
		else {
			(new DConnect(m_pBoard))->Create( DetachSocket(), nPlayers, m_spectator );
			Destroy();
		}
	}
	else if ( pMsg->GetType() == CMsg::mmDisconnected ) {
		// 서버로부터의 접속 종료
		Fail( _T("서버로부터 접속이 끊어졌습니다 !") );
	}
	else if ( pMsg->GetType() == CMsg::mmError ) {
		// 서버로부터의 에러 메시지
		CString sMsg;
		long lDummy;
		if ( pMsg->PumpLong( lDummy )
			&& pMsg->PumpString( sMsg ) )
			Fail( sMsg );
		else Fail( _T("서버로부터 에러코드가 수신되었습니다 !") );
	}
	else {
		// 모르는 이상한 메시지
		ASSERT(0);
		m_pSocket->SetTrigger( (DWORD)(LPVOID)this, 0, SockProc );
	}
}


/////////////////////////////////////////////////////////////////////////////
// DConnect

DConnect::DConnect( CBoardWrap* pBoard ) : DSB(pBoard)
{
	m_asChatData = new TCHAR[CHAT_LINES][256];
	m_acolChatData = new COLORREF[CHAT_LINES];

	m_bServer = true;
	m_pServerSocket = 0;
	m_uid = 0;

	for ( int i = 0; i < MAX_CONNECTION; i++ ) {
		m_aInfo[i].bComputer = true;
		m_aInfo[i].pSocket = 0;
	}
	UpdateMarks();

	m_nChatDataBegin = 0;
	m_nChatDataEnd = 0;

	// 채팅창 사각형 ( 31 x CHAT_LINES )
	m_rcChat.SetRect( 1, 26, 32, 26+CHAT_LINES );

	m_pPopup = 0;
}

DConnect::~DConnect()
{
	// 채팅창 핸들러를 클리어
	Ib()->SetChatHandler( (DWORD)(LPVOID)this, 0 );
	if ( m_pPopup )
		m_pPopup->Destroy();
	ASSERT( !m_pPopup );	// popup 이 스스로 지웠다

	if ( m_pServerSocket ) {
		// 핸들러를 끄고 소켓을 닫는다
		m_pServerSocket->ClearTrigger();
		delete m_pServerSocket; m_pServerSocket = 0;
	}
	m_pServerSocket = 0;

	for ( int i = 0; i < MAX_CONNECTION; i++ )
		if ( m_aInfo[i].pSocket ) {
			m_aInfo[i].pSocket->ClearTrigger();
			delete m_aInfo[i].pSocket;
		}

	delete[] m_asChatData;
	delete[] m_acolChatData;
}

void DConnect::Create( CPlayerSocket* pServerSocket, long players, bool spectatorOnly )
{
	// 일단 생성은 하고 나서, 그 과정중에 실패한 것이 있으면
	// Fail() 을 호출한다 (Fail()은 생성된 상태를 가정하므로)
	bool bFailed = false;
	m_nSpectators = 0;

	UINT uPort;

	if ( pServerSocket ) {
		m_pServerSocket = pServerSocket;
		m_bServer = false;
	}
	else m_bServer = true;

	if ( m_bServer ) {

		// 접속 소켓을 생성
		CZInetAddr addr( Mo()->sAddress, Mo()->nPort );
		if ( !CreateServerSocket(addr) )
			bFailed = true;

		// 이 호스트의 주소를 얻는다
		uPort = (UINT)Mo()->nPort;
		CString sAddr;
		get_host( sAddr );
		m_sAddress.Format( _T("%s:%u"), sAddr, uPort );

		// 규칙 스트링
		if ( Mo()->nPreset == 0 ) m_sRule = _T("사용자정의");
		else { CRule r; m_sRule = r.Preset( Mo()->nPreset ); }
		m_rule.Decode( Mo()->rule.Encode() );

		// 사람 정보 ( 처음에는 자신만 빼고 모두 컴퓨터들 )

		m_aInfo[0].bComputer = false;
		m_aInfo[0].sName = Mo()->aPlayer[0].sName;
		m_aInfo[0].sInfo = format_score(
			Mo()->anPlayerState[m_rule.nPlayerNum-2][0], Mo()->anPlayerState[m_rule.nPlayerNum-2][1],
			Mo()->anPlayerState[m_rule.nPlayerNum-2][2] );
		
		m_aInfo[0].pSocket = 0;

		m_aInfo[0].dfa[0] = Mo()->anPlayerState[m_rule.nPlayerNum-2][0];
		m_aInfo[0].dfa[1] = Mo()->anPlayerState[m_rule.nPlayerNum-2][1];
		m_aInfo[0].dfa[2] = Mo()->anPlayerState[m_rule.nPlayerNum-2][2];
		m_aInfo[0].dfa[3] = BASE_MONEY;

		for ( int i = 1; i < MAX_CONNECTION; i++ )
			SetComputer( i, BASE_MONEY );

		// 등장메시지를 채팅창으로 출력한다
		CString sEnterMsg = create_entermsg( m_aInfo[0].sName,
			Mo()->anPlayerState[m_rule.nPlayerNum-2][0], 
			Mo()->anPlayerState[m_rule.nPlayerNum-2][1], 
			Mo()->anPlayerState[m_rule.nPlayerNum-2][2],
			false );
		Chat( sEnterMsg, -1, false );

		UpdateMarks();
	}
	else {	// !m_bServer

		// 접속된 호스트의 주소를 얻는다
		CZInetAddr addr = m_pServerSocket->GetPeerName();

		CString sAddr( _T("Unknown") );
		uPort = 0;
		VERIFY( addr.GetAddr( &sAddr, &uPort ) );

		m_sAddress.Format( _T("%s:%u"), sAddr, uPort );

		// 호스트에게, mmNewPlayer 메시지를 보낸다
		CMsg* pNewPlayerMsg = CreateNewPlayerMsg( players, spectatorOnly );
		AUTODELETE_MSG(pNewPlayerMsg);
		if ( !m_pServerSocket->SendMsg( pNewPlayerMsg ) )
			// 실패 !
			bFailed = true;
	}

	// 채팅창 핸들러를 세트
	Ib()->SetChatHandler( (DWORD)(LPVOID)this, ChatProc );

	DSB::Create( 0, 0, 33, 33, -1 );


	if ( bFailed ) {
		if ( m_bServer ) {
			CString sPort; sPort.Format( _T("%u"), uPort );
			Fail( _T("소켓을 다음 포트로 바인드 할 수 없었습니다 !"), sPort );
		}
		else {
			Fail( _T("서버에 메시지를 보낼 수 없습니다 !") );
		}
	}
	else {	// 성공, client handler 를 세트한다
		if ( !m_bServer ) {
			m_pServerSocket->SetTrigger( (DWORD)(LPVOID)this, 0, ClientSockProc );
		}
	}
}

// 초기화 ( 생성된 후 호출됨 )
void DConnect::OnInit()
{
	DSB::OnInit();

	if ( m_bServer ) {
		// 서버용 hotspot 들을 등록
		RegisterOk();
		RegisterMarks();
		RegisterRule();
		RegisterSpec();
	}

	RegisterHotspot(
		27, 23, -1, -1, true, 0,
		_T("취소"),
		&s_colCyan, &s_tdMidium, &s_colCyan, &s_tdMidiumOutline,
		(LPVOID)0 );
}

// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
// 오버라이드 하여 구현해야 한다
// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
void DConnect::OnDraw(
	CDC* pDC,
	bool bBrushOrigin )
{
	DSB::OnDraw( pDC, bBrushOrigin );

	// 주소
	PutText( pDC, m_sAddress,
			-1, 1, true, s_colYellow, s_tdMidiumOutline );
	// 참가자
	int y = m_rule.nPlayerNum >= 6 ? 5 :7;
	PutText( pDC, _T("참가자 :"),
			2, y - 2, true, s_colWhite, s_tdShade );
	for ( int i = 0; i < m_rule.nPlayerNum; i++ ) {

		// 컴퓨터는 흰색, 사람은 노란색
		COLORREF col = m_aInfo[i].bComputer ? s_colWhite : s_colYellow;

		// '나'는 테두리, 나머지는 그림자
		int deco = i == m_uid ? s_tdOutline : s_tdShade;
		PutText( pDC, m_aInfo[i].sName,
			3, y+i*3, true, col, deco );
		PutText( pDC, m_aInfo[i].sInfo, 4, y+i*3+1, true,
			s_colGray, s_tdNormal );
	}

	// 규칙
	PutText( pDC, _T("적용 규칙 : "),
			25, y - 2, true, s_colWhite, s_tdShade );

	// 관전자
	if ( !m_bServer )
		PutText( pDC, _T("관전자 : "),
				25, y + 6, true, s_colWhite, s_tdShade );
	
	wsprintf(m_specstr, "%d 명", m_nSpectators);

	PutText( pDC, m_specstr,
			25, y + 8, true, s_colWhite, s_tdShade );

	// 채팅창
	int nChatLines = (int)( m_nChatDataEnd - m_nChatDataBegin );
	for ( int t = 0; t < nChatLines; t++ ) {
		int nIndex = ( m_nChatDataBegin + t ) % CHAT_LINES;
		PutText( pDC, m_asChatData[nIndex],
			m_rcChat.left, m_rcChat.top+t, true,
			m_acolChatData[nIndex], s_tdNormal );
	}

	CRect rcBound;
	CPen pnGray( PS_SOLID, 0, s_colGray );
	pDC->SelectStockObject( NULL_BRUSH );
	pDC->SelectObject( &pnGray );

	// 테두리 (채팅창)
	rcBound = DSBtoDP( m_rcChat );
	rcBound.InflateRect( GetDSBUnit()/2, GetDSBUnit()/2 );
	pDC->RoundRect( &rcBound,
		CPoint( GetDSBUnit(), GetDSBUnit() ) );

	// 테두리 (참가자)
	rcBound = DSBtoDP( CRect( 1, y, 24, 25 ) );
	rcBound.InflateRect( GetDSBUnit()/2, GetDSBUnit()/2, 0, GetDSBUnit()/3 );
	pDC->RoundRect( &rcBound,
		CPoint( GetDSBUnit(), GetDSBUnit() ) );

	pDC->SelectStockObject( WHITE_PEN );

	// 테두리 (확인)
	if ( m_bServer ) {

		rcBound = DSBtoDP( CRect( 25, 20, 32, 22 ) );
		rcBound.InflateRect( GetDSBUnit()/2, GetDSBUnit()/3, GetDSBUnit()/2, -GetDSBUnit()/4 );
		pDC->RoundRect( &rcBound,
			CPoint( GetDSBUnit(), GetDSBUnit() ) );
	}

	// 테두리 (취소)
	rcBound = DSBtoDP( CRect( 25, 23, 32, 25 ) );
	rcBound.InflateRect( GetDSBUnit()/2, GetDSBUnit()/3, GetDSBUnit()/2, -GetDSBUnit()/4 );
	pDC->RoundRect( &rcBound,
		CPoint( GetDSBUnit(), GetDSBUnit() ) );
}

// 핫 스팟을 클릭할 때 불리는 함수
// LPVOID 값이 0 이면 종료 (X자를 클릭) 요청
void DConnect::OnClick( LPVOID pVoid )
{
	switch ( (int)pVoid ) {

	case 0: {
		(new DStartUp(m_pBoard))->Create( 1 );
		Destroy();
		break;
	}

	case 500: {	// 규칙을 보여줌
		// read-only, auto-delete
		DRule* pDRule = new DRule( m_rule.Encode(), true, true );
		pDRule->Create(
			*m_pBoard,
			WS_VISIBLE | WS_DLGFRAME
			| WS_CLIPCHILDREN | WS_CLIPSIBLINGS
			| WS_SYSMENU | WS_CAPTION );
		pDRule->SetIcon( AfxGetApp()->LoadIcon( IDR_MAINFRAME ), TRUE );
		break;
	}

	case 600: {	// 규칙 저장
		CString rulename = CRule::AttemptSaveRule( m_rule.Encode(), m_sRule );
		if ( rulename != "" )
			DeleteHotspot( (LPVOID)600 );
		break;
	}
	case 0xffffffff: {	// 시작 !
		if ( m_bServer ) {
			CMsg msgBegin( _T("l"), CMsg::mmBeginGame );
			SendToAll( &msgBegin );	// 모두에게 알림
			VERIFY( BeginServer() );
		}
		break;
	}

	default: {
		if ( (int)pVoid >= 100 && (int)pVoid < 100 + MAX_PLAYERS ) {
			// 플레이어앞의 세모 마크
			long uid = (int)pVoid - 100;
			ASSERT( !m_pPopup );
			if ( uid != 0 )
				(m_pPopup = new DConnectPopup(this,m_pBoard))
								->Create( uid );
		}
		else if ( (int)pVoid >= 200 && (int)pVoid < 300 ) {
			// DConnectPopup 이 전달해 준 리턴값
			long uid = ((int)pVoid - 200 ) / 10;
			long result = ((int)pVoid - 200 ) % 10;

			// 사람이 아니면, 무시한다
			if ( m_aInfo[uid].bComputer ) break;

			if ( result == 1 ) 	// 일시 추방
				FailedForPlayer( uid, true );
			else if ( result == 2 ) {	// 영구 추방
				// 블랙리스트에 넣는다
				m_lBlackList.AddTail( m_aInfo[uid].sName );
				FailedForPlayer( uid, true );
			}
		}
		else if ( (int)pVoid == 1000 ) {	// 관전자 관리
			(m_pPopup = new DConnectPopup(this,m_pBoard))
							->Create( Mo()->bObserver ? -1 : -2 );
		}
		else if ( (int)pVoid >= 190 && (int)pVoid < 200 ) {
			int result = (int)pVoid - 190;
			if ( result == 1 ) {	// 관전자 차단/허용
				Mo()->bObserver = !Mo()->bObserver;
			}
			else if ( result == 2 ) {	// 관전자 전부 추방
				int i;
				for ( i = m_rule.nPlayerNum; i < MAX_CONNECTION; i++ ) {
					if ( !m_aInfo[i].bComputer )
						FailedForPlayer ( i, true );
				}
			}
		}
		break;
	}
	}
}

// m_acolMark 를 update 함
void DConnect::UpdateMarks()
{
	for ( int i = 0; i < MAX_PLAYERS; i++ )
		if ( !m_bServer || m_aInfo[i].bComputer )
			m_acolMark[i] = s_colGray;
		else m_acolMark[i] = s_colCyan;
}

// 회복불가능한 에러가 발생했을 때 호출
void DConnect::Fail( LPCTSTR sMsg1, LPCTSTR sMsg2 )
{
	// 메시지 상자를 띄운다
	// 이 상자는 자동으로 this 를 죽인다
	(new DConnectFail(m_pBoard))->Create(
		_T("에러 !!"), sMsg1, sMsg2, this );
}

// 시작버튼을 핫스팟에 등록
void DConnect::RegisterOk()
{
	RegisterHotspot(
		27, 20, -1, -1, true, 0,
		_T("시작"),
		&s_colCyan, &s_tdMidium, &s_colCyan, &s_tdMidiumOutline,
		(LPVOID)0xffffffff );
}

// 사람에 붙는 기호를 등록
void DConnect::RegisterMarks()
{
	int y = m_rule.nPlayerNum >= 6 ? 5 : 7;
	for ( int i = 0; i < m_rule.nPlayerNum; i++ )
		RegisterHotspot(
			1, y+i*3, -1, -1, true, 0, _T("▽"),
			&m_acolMark[i], &s_tdNormal,
			&m_acolMark[i], &s_tdOutline, (LPVOID)( i + 100 ) );
}

// 규칙을 등록
void DConnect::RegisterRule()
{
	RegisterHotspot(
		25, 7 - ( m_rule.nPlayerNum >= 6 ? 2 : 0 ), -1, -1, true, 0,
		m_sRule,
		&s_colCyan, &s_tdShade, &s_colCyan, &s_tdOutline, (LPVOID)500 );

	if ( CRule::RuleExists( m_rule.Encode() ) == "" )
		RegisterHotspot(
			25, 9 - ( m_rule.nPlayerNum >= 6 ? 2 : 0 ), -1, -1, true, 0,
			_T("규칙 저장"),
			&s_colYellow, &s_tdShade, &s_colYellow, &s_tdOutline, (LPVOID)600 );
}

void DConnect::RegisterSpec()
{
	RegisterHotspot(
		25, m_rule.nPlayerNum >= 6 ? 11 : 13, -1, -1, true, 0, _T("관전자 : "),
		&s_colCyan, &s_tdShade,
		&s_colCyan, &s_tdOutline, (LPVOID)( 1000 ) );
}

// 채팅 메시지를 생성(new)
CMsg* DConnect::CreateChatMsg( long uid, LPCTSTR sMsg )
{
	return new CMsg( _T("lls"), CMsg::mmChat, uid, sMsg );
}

// uid 위치의 플레이어를 컴퓨터로 치환
void DConnect::SetComputer( long uid, int money )
{
	if ( m_aInfo[uid].pSocket ) {
		m_aInfo[uid].pSocket->ClearTrigger();
		delete m_aInfo[uid].pSocket;
	}

	if(!m_aInfo[uid].bComputer)
	{
		m_aInfo[uid].bComputer = true;
		if(m_nSpectators > 0) m_nSpectators--;
	}
	if(uid < MAX_PLAYERS)
	{
		m_aInfo[uid].sName = Mo()->aPlayer[uid].sName + _T(" (컴퓨터)");
		m_aInfo[uid].sInfo = get_name( Mo()->aPlayer[uid].sAIDLL );
	}
	else
	{
		m_aInfo[uid].sName = _T("관전 Dummy");
		m_aInfo[uid].sInfo = get_name( Mo()->aPlayer[0].sAIDLL );
	}
	m_aInfo[uid].pSocket = 0;

	m_aInfo[uid].dfa[0] = 0;
	m_aInfo[uid].dfa[1] = 0;
	m_aInfo[uid].dfa[2] = 0;
	m_aInfo[uid].dfa[3] = money;
}

// 현재 참가한 플레이어들에게 메시지를 전송
void DConnect::SendToAll( CMsg* pMsg, long uidExcept )
{
	for ( int i = 1; i < MAX_CONNECTION; i++ )
		if ( uidExcept != i && !m_aInfo[i].bComputer ) {
			// 사람이면 전달
			if ( m_aInfo[i].pSocket )
				if ( !m_aInfo[i].pSocket->SendMsg( pMsg ) )
					FailedForPlayer(i);
		}
}

// 현재 m_aInfo 상황을 기술하는 CMsg 객체를 생성(new)
// 즉, mmPrepare 메시지
CMsg* DConnect::CreateStateMsg()
{
	int i;
	CString sFormat = _T("lss");

	CMsg* k = new CMsg( sFormat,
		CMsg::mmPrepare, m_rule.Encode(), m_sRule );
	for(i = 0; i < MAX_CONNECTION; i++)
	{
		k->PushString(m_aInfo[i].sName);
		k->PushString(m_aInfo[i].sInfo);
		k->PushLong(m_aInfo[i].bComputer ? 1 : 0);
		k->PushLong(m_aInfo[i].dfa[0]);
		k->PushLong(m_aInfo[i].dfa[1]);
		k->PushLong(m_aInfo[i].dfa[2]);
		k->PushLong(m_aInfo[i].dfa[3]);
	}
	return k;
}

// m_aInfo 의 i 번째 플레이어에 대한 CMsg 객체를 생성(new)
// 즉, mmChanged 메시지
CMsg* DConnect::CreatePlayerInfoMsg( long uid )
{
	return new CMsg( _T("llssl"),
		CMsg::mmChanged, uid,
		m_aInfo[uid].sName, m_aInfo[uid].sInfo, m_aInfo[uid].bComputer ? 1 : 0 );
}

// mmInit 메시지를 생성(new)
CMsg* DConnect::CreateInitMsg( long players )
{
	return new CMsg( _T("lll"), CMsg::mmInit, MIGHTY_VERSION, players );
}

// mmUID 메시지를 생성(new)
CMsg* DConnect::CreateUIDMsg( long uid )
{
	return new CMsg( _T("ll"), CMsg::mmUID, uid );
}

// 특정 플레이어에 대한 소켓 호출이 실패
// (Computer 로 바꾸고 다른 플레이어에게 전달)
// ( bAccessDenied 가 참이면, 이 플레이어는
// 추방에 의해 삭제되는것임 )
void DConnect::FailedForPlayer( long uid, bool bAccessDenied )
{
	RemovePlayer( uid, bAccessDenied );
	CMsg* pMsg = CreatePlayerInfoMsg( uid );
	AUTODELETE_MSG(pMsg);
	SendToAll( pMsg );
}

// 플레이어를 하나 추가
// 리턴되는 값은 uid, -1 이면 빈공간이 없음, -2 이면 같은 이름 존재,
// -3 이면 허용 거부, -4 이면 관전자 거부
// pMsg : mmNewPlayer 메시지
long DConnect::AddPlayer( CMsg* pMsg, CPlayerSocket* pSocket )
{
	int i;
	// 이름, 전적을 추출한다
	// spectatorOnly: 관전자만 하고 싶은 것으로 들어왔다 (2011.1.7)
	CString sName; long spectatorOnly; long r[3]; long p;

	long m;
	VERIFY( pMsg->PumpLong( m ) && m == CMsg::mmNewPlayer );

	if ( !pMsg->PumpString( sName )
		|| !pMsg->PumpLong( spectatorOnly )
		|| !pMsg->PumpLong( r[0] ) || !pMsg->PumpLong( r[1] ) || !pMsg->PumpLong( r[2] )
		|| !pMsg->PumpLong( p ) ) return -3;


	// 블랙리스트에 있는 인물인가 조사한다
	POSITION pos = m_lBlackList.GetHeadPosition();
	while (pos)
		if ( m_lBlackList.GetNext(pos) == sName ) return -3;

	// 관전자 거부인데 관전자로 들어왔다
	if ( !Mo()->bObserver && spectatorOnly )
		return -4;
	
	// 빈자리를 찾는다
	long uid;
	for ( uid = 0; uid < MAX_CONNECTION; uid++ )
	{
		if ( m_aInfo[uid].bComputer )
		{
			if ( !(spectatorOnly && uid < m_rule.nPlayerNum) )
				break;
		}
		// 같은 닉네임 불가인데 같은 닉네임이 존재하면
		else if ( !Mo()->bSameName && m_aInfo[uid].sName == sName ) return -2;
	}
	// 관전 가능한 경우 20명, 불가능한 경우 플레이어 수보다 많은 경우 자리가 꽉찼다는 표시
	if ( uid >= 20 || (( !Mo()->bObserver) && uid >= m_rule.nPlayerNum ) )
		return -1;

	m_aInfo[uid].sName = sName;
	m_aInfo[uid].sInfo = format_score( r[0], r[1], r[2] );
	m_aInfo[uid].bComputer = false;
	m_aInfo[uid].pSocket = pSocket;
	for ( i = 0; i < 3; i++ )
		m_aInfo[uid].dfa[i] = r[i];
	m_aInfo[uid].dfa[3] = BASE_MONEY;

	// 등장메시지를 채팅창으로 출력한다
	CString sEnterMsg = create_entermsg(
							m_aInfo[uid].sName, r[0], r[1], r[2], uid >= m_rule.nPlayerNum );

	CMsg* pEnterMsg = CreateChatMsg( -1, sEnterMsg );
	AUTODELETE_MSG(pEnterMsg);

	SendToAll( pEnterMsg );

	Chat( sEnterMsg, -1, false );
	
	if(uid >= m_rule.nPlayerNum)
		m_nSpectators++;

	// 화면을 Update
	UpdateMarks();
	CRect rc; GetRect( &rc );
	m_pBoard->UpdateDSB( &rc );

	return uid;
}

// 플레이어를 삭제
void DConnect::RemovePlayer( long uid, bool bAccessDenied )
{
	ASSERT( !m_aInfo[uid].bComputer );

	if ( bAccessDenied ) {	// 추방 에러 메시지를 보낸다
		CMsg msgOut( _T("ls"), CMsg::mmError,
					_T("게임에서 추방당했습니다 !!") );
		m_aInfo[uid].pSocket->SendMsg( &msgOut );
	}

	CString sName = m_aInfo[uid].sName;

	SetComputer( uid, BASE_MONEY);

	// 퇴장메시지를 채팅창으로 출력한다
	CString sOutMsg;
	sOutMsg.Format(
		!bAccessDenied ? _T("%s 님이 퇴장하였습니다")
		: _T("%s 님이 게임에서 추방당했습니다 !!"), sName );

	CMsg* pChatMsg = CreateChatMsg( -1, sOutMsg );
	AUTODELETE_MSG(pChatMsg);
	SendToAll( pChatMsg );

	Chat( sOutMsg, -1, false );

	// 화면을 Update
	UpdateMarks();
	CRect rc; GetRect( &rc );
	m_pBoard->UpdateDSB( &rc );
}

// 접속 요청
void DConnect::OnAccept( int nErr )
{
	// 실패라면 ? (무시한다)
	if ( nErr ) { ASSERT(0); return; }

	// 접속을 받는다
	CPlayerSocket* pSocket = new CPlayerSocket();
	if ( !Accept( *pSocket ) ) {
		ASSERT(0);
		delete pSocket;
		return;
	}

	// mmInit 으로 응답하고, 핸들러를 세트
	CMsg* pMsg = CreateInitMsg( m_rule.nPlayerNum );
	AUTODELETE_MSG(pMsg);

	if ( !pSocket->SendMsg( pMsg ) ) {
		ASSERT(0);
		delete pSocket;
		return;
	}

	// 0 번 uid (anonymous) 로 핸들러를 세트
	pSocket->SetTrigger( (DWORD)(LPVOID)this, 0, ServerSockProc );
}

// 소켓 핸들러
void DConnect::ServerSockProc( CPlayerSocket* pSocket, CMsg* pMsg,
								DWORD dwUser1, DWORD dwUser2 )
{
	((DConnect*)(LPVOID)dwUser1)->ServerSockProc(
		(long)dwUser2, pMsg, pSocket );
}

void DConnect::ServerSockProc( long uid, CMsg* pMsg, CPlayerSocket* pSocket )
{
	AUTODELETE_MSG(pMsg);

	if ( uid == 0 ) {
		// Anonymous Socket
		// 현재 uid 링크 없이 접속만 되어 있는 상태

		if ( pMsg->GetType() == CMsg::mmDisconnected ) {
			// 접속이 끊겼다 - 이 소켓에 대해서는 잊는다
			delete pSocket;
			return;
		}
		else if ( pMsg->GetType() == CMsg::mmNewPlayer ) {
			// 등록 요청
			uid = AddPlayer( pMsg, pSocket );

			if ( uid < 0 ) {
				CMsg msgErr( _T("ls"), CMsg::mmError,
					uid == -1 ? _T("이미 인원이 다 찼습니다") : 
					uid == -2 ? _T("같은 이름이 존재합니다") :
					uid == -3 ? _T("접근이 거부되었습니다") :
					uid == -4 ? _T("관전자를 허용하지 않습니다")
					: _T("유저가 불량합니다") );
				VERIFY( pSocket->SendMsg( &msgErr ) );
				delete pSocket;
				return;
			}
			else {
				// 등록 성공 !

				// 다른사람들에게 알림
				CMsg* pInfoMsg = CreatePlayerInfoMsg( uid );
				AUTODELETE_MSG(pInfoMsg);

				SendToAll( pInfoMsg, uid );	// 그 사람만 빼고

				// 그 사람에게는 따로, UID 와 전체 데이터를 준다

				CMsg* pUIDMsg = CreateUIDMsg( uid );
				CMsg* pAllInfo = CreateStateMsg();

				AUTODELETE_MSG(pUIDMsg);
				AUTODELETE_MSG(pAllInfo);

				if ( !pSocket->SendMsg( pUIDMsg )
					|| !pSocket->SendMsg( pAllInfo ) )
					FailedForPlayer( uid );
				else	// 정식으로 uid 로 핸들러를 설정
					pSocket->SetTrigger(
						(DWORD)(LPVOID)this, uid, ServerSockProc );

				return;	// SetTrigger 가 uid 0 으로 재설정되지 않도록
			}
		}
	}
	else {	// uid 가 1 ~ nPlayerNum 까지
			// m_aInfo 에 등록된 플레이어가 보낸 메시지

		if ( pMsg->GetType() == CMsg::mmDisconnected ) {
			// 접속 종료
			FailedForPlayer( uid );
			return;
		}
		else if ( pMsg->GetType() == CMsg::mmChat ) {
			// 채팅 메시지
			if ( !ReceiveChatMsg( pMsg ) ) {
				FailedForPlayer( uid );
				return;
			}
			else SendToAll( pMsg );
		}
	}

	// 그 uid 에 대해서 메시지를 또 받을 수 있도록
	// 핸들러를 재설정
	pSocket->SetTrigger( (DWORD)(LPVOID)this, uid, ServerSockProc );
}

// mmNewPlayer 메시지를 생성
CMsg* DConnect::CreateNewPlayerMsg( long players, bool spectatorOnly )
{
	return new CMsg( _T("lslllll"), CMsg::mmNewPlayer,
		Mo()->aPlayer[0].sName,
		spectatorOnly ? 1 : 0,
		Mo()->anPlayerState[players-2][0], Mo()->anPlayerState[players-2][1], Mo()->anPlayerState[players-2][2],
		0 // BASE_MONEY 또는 자기돈?
	);
}

static long lDummy;

// mmUID 메시지를 수신하여 Update
bool DConnect::ReceiveUIDMsg( CMsg* pMsg )
{
	if ( !pMsg->PumpLong( lDummy )
		|| !pMsg->PumpLong( m_uid ) ) {
		ASSERT(0);
		return false;
	}
	else return true;
}

// mmPrepare 메시지를 수신
bool DConnect::ReceiveStateMsg( CMsg* pMsg )
{
	int i;
	CString sPreset;
	CString sRule;

	if ( !pMsg->PumpLong( lDummy )
		|| !pMsg->PumpString( sRule )
		|| !pMsg->PumpString( sPreset ) ) return false;

	// 규칙
	m_rule.Decode( sRule );
	m_sRule = sPreset;

	// 규칙 정보가 등록되지 않았다면 등록한다
	if ( !FindHotspot( (LPVOID)500 ) ) RegisterRule();

	// 사람 정보
	long bComputer;
	m_nSpectators = 0;
	for ( i = 0; i < MAX_CONNECTION; i++ ) {
		if ( !pMsg->PumpString( m_aInfo[i].sName )
			|| !pMsg->PumpString( m_aInfo[i].sInfo )
			|| !pMsg->PumpLong( bComputer )
			|| !pMsg->PumpLong( m_aInfo[i].dfa[0] )
			|| !pMsg->PumpLong( m_aInfo[i].dfa[1] )
			|| !pMsg->PumpLong( m_aInfo[i].dfa[2] )
			|| !pMsg->PumpLong( m_aInfo[i].dfa[3] ) ) return false;
		m_aInfo[i].bComputer = !!bComputer;
		if(i >= m_rule.nPlayerNum && !bComputer)
			m_nSpectators++;
	}

	// Mark 들이 등록되지 않았다면 등록한다
	if ( !FindHotspot( (LPVOID)100 ) ) RegisterMarks();
	UpdateMarks();

	CRect rc; GetRect(&rc);
	m_pBoard->UpdateDSB( &rc );

	return true;
}

// mmChanged 메시지를 수신
bool DConnect::ReceivePlayerInfoMsg( CMsg* pMsg )
{
	long uid, bComputer;

	if ( !pMsg->PumpLong( lDummy )
		|| !pMsg->PumpLong( uid )
		|| !pMsg->PumpString( m_aInfo[uid].sName )
		|| !pMsg->PumpString( m_aInfo[uid].sInfo )
		|| !pMsg->PumpLong( bComputer ) ) return false;

	if(uid >= m_rule.nPlayerNum)
	{
		if(!m_aInfo[uid].bComputer) m_nSpectators--;
		if(!bComputer) m_nSpectators++;
	}
	m_aInfo[uid].bComputer = !!bComputer;

	UpdateMarks();

	CRect rc; GetRect(&rc);
	m_pBoard->UpdateDSB( &rc );

	return true;
}

// mmChat 메시지를 수신
bool DConnect::ReceiveChatMsg( CMsg* pMsg )
{
	long uid;
	CString sMsg;
	if ( !pMsg->PumpLong( uid )
		|| !pMsg->PumpLong( uid )
		|| !pMsg->PumpString( sMsg ) ) return false;
	Chat( sMsg, uid );
	return true;
}

// 소켓 핸들러
void DConnect::ClientSockProc( CPlayerSocket* pSocket, CMsg* pMsg,
							DWORD dwUser1, DWORD dwUSer2 )
{	((DConnect*)(LPVOID)dwUser1)->ClientSockProc( pMsg ); }

void DConnect::ClientSockProc( CMsg* pMsg )
{
	AUTODELETE_MSG(pMsg);

	if ( pMsg->GetType() == CMsg::mmDisconnected ) {
		// 접속 종료
		Fail( _T("서버로부터 접속이 끊어졌습니다 !") );
		return;
	}
	else if ( pMsg->GetType() == CMsg::mmUID ) {
		// UID 값을 얻는다
		if ( !ReceiveUIDMsg( pMsg ) )
			// Close 를 호출하면, 다음번 Trigger 메시지에서
			// mmDisconnected 가 될 것이다
			m_pServerSocket->Close();
	}
	else if ( pMsg->GetType() == CMsg::mmPrepare ) {
		// 준비 (모든 상태를 전달)
		if ( !ReceiveStateMsg( pMsg ) )
			m_pServerSocket->Close();
	}
	else if ( pMsg->GetType() == CMsg::mmChanged ) {
		// 일부 정보 변경
		if ( !ReceivePlayerInfoMsg( pMsg ) )
			m_pServerSocket->Close();
	}
	else if ( pMsg->GetType() == CMsg::mmBeginGame ) {
		// 게임 시작 !
		m_pServerSocket->ClearTrigger();
		VERIFY( BeginClient() );
		return;
	}
	else if ( pMsg->GetType() == CMsg::mmChat ) {
		// 채팅 메시지
		if ( !ReceiveChatMsg( pMsg ) )
			m_pServerSocket->Close();
	}
	else if ( pMsg->GetType() == CMsg::mmError ) {
		// 에러 메시지
		m_pServerSocket->Close();
		CString sMsg;
		if ( pMsg->PumpLong( lDummy )
			&& pMsg->PumpString( sMsg ) )
			Fail( sMsg );
		else Fail( _T("서버로부터 에러코드가 수신되었습니다 !") );
		return;
	}
	else {
		// 모르는 메시지
		ASSERT(0);
	}

	m_pServerSocket->SetTrigger( (DWORD)(LPVOID)this, 0, ClientSockProc );
}

// 채팅창 핸들러
void DConnect::ChatProc( LPCTSTR sMsg, DWORD dwUser )
{	((DConnect*)(LPVOID)dwUser)->ChatProc( sMsg ); }

void DConnect::ChatProc( LPCTSTR sMsg )
{
	CMsg* pChatMsg = CreateChatMsg( m_uid, sMsg );
	AUTODELETE_MSG(pChatMsg);

	if ( m_bServer ) { 	// 서버라면 모두에게 보낸다
		SendToAll( pChatMsg );
		// 자신에게는 안보내지므로 직접 그린다
		Chat( sMsg, m_uid );
	}
	else {	// 클라이언트라면, mmChat 메시지로 메아리될것이다
		m_pServerSocket->SendMsg( pChatMsg );
	}
}

// 채팅 화면에 sMsg 를 그린다 (uid==-1 이면 전역 메시지)
void DConnect::Chat( LPCTSTR sMsg, long uid, bool bUpdate )
{
	int i;

	CString sFullMsg;
	if ( uid == -1 )
		sFullMsg = CString(_T("## ")) + sMsg + _T(" ##");
	else sFullMsg = m_aInfo[uid].sName + _T(": ") + sMsg;

	// 다음줄로 넘어가기도 하므로, Parse 하여 실제 필요한 라인 수를 구한다
	size_t nMaxWidth;	// 라인의 최대 폭
	int n = ParseString( 0, m_rcChat.Width()-1, sFullMsg, nMaxWidth );

	LPTSTR* asMsg = new LPTSTR[n];
	for ( i = 0; i < n; i++ ) asMsg[i] = new TCHAR[nMaxWidth];

	ParseString( asMsg, m_rcChat.Width()-1, sFullMsg, nMaxWidth );

	// 각 라인을 추가한다
	for ( i = 0; i < n; i++ ) {

		strcpy( m_asChatData[m_nChatDataEnd % CHAT_LINES], asMsg[i] );
		m_acolChatData[m_nChatDataEnd % CHAT_LINES] =
			uid == -1 ? s_colCyan
			: uid == m_uid ? s_colYellow
			: s_colWhite;
		m_nChatDataEnd++;
	}

	if ( m_nChatDataEnd - m_nChatDataBegin > CHAT_LINES )
		m_nChatDataBegin = m_nChatDataEnd - CHAT_LINES;

	for ( i = 0; i < n; i++ ) delete[] asMsg[i];
	delete[] asMsg;

	if ( bUpdate ) {
		CRect rc = DSBtoDP( m_rcChat );
		m_pBoard->UpdateDSB( &rc );
	}
}

// 게임의 시작 (서버로서)
bool DConnect::BeginServer()
{
	int i;
	ASSERT( m_uid == 0 );

	// 이 소스는 DStartUp.cpp 참조

	int nPlayers = m_rule.nPlayerNum;

	CPlayer* apPlayers[MAX_CONNECTION];

	// Human Player 생성
	apPlayers[0] = new CPlayerHuman(
		0, m_aInfo[0].sName, *m_pBoard );

	// Network Players / AI Players 생성
	bool bFailed = false;
	CString sFailedDLL;
	for ( i = 1; i < MAX_CONNECTION; i++ ) {

		if ( m_aInfo[i].bComputer ) {	// AI Player
			if ( i < nPlayers ) {
				CPlayerMai* pPlayer = new CPlayerMai(
					i, m_aInfo[i].sName, *m_pBoard,
					Mo()->aPlayer[i].sAIDLL, Mo()->aPlayer[i].sSetting );
				apPlayers[i] = pPlayer;
				if ( !pPlayer->IsDLLWorking() ) {
					bFailed = true;
					sFailedDLL = Mo()->aPlayer[i].sAIDLL;
				}
			}
			else {			// 관전자 자리에 있는 더미
				CPlayerDummy* pPlayer = new CPlayerDummy(i, *m_pBoard );
				apPlayers[i] = pPlayer;
			}
		}
		else {	// NetworkPlayer

			CPlayerNetwork* pPlayer = new CPlayerNetwork(
				i, m_aInfo[i].sName, *m_pBoard );
			apPlayers[i] = pPlayer;
		}
	}

	// 돈과 전적을 배분
	for ( i = 0; i < nPlayers; i++ ) {
		apPlayers[i]->SetMoney( m_aInfo[i].dfa[3] );
		apPlayers[i]->GetAllRecord().wm = LOWORD( m_aInfo[i].dfa[0] );
		apPlayers[i]->GetAllRecord().lm = HIWORD( m_aInfo[i].dfa[0] );
		apPlayers[i]->GetAllRecord().wf = LOWORD( m_aInfo[i].dfa[1] );
		apPlayers[i]->GetAllRecord().lf = HIWORD( m_aInfo[i].dfa[1] );
		apPlayers[i]->GetAllRecord().wa = LOWORD( m_aInfo[i].dfa[2] );
		apPlayers[i]->GetAllRecord().la = HIWORD( m_aInfo[i].dfa[2] );
	}

	// 이제 SetMFSM 을 하는 순간 this 를 비롯한 모든 DSB 가 사라진다
	// 경고 메시지를 띄우기 위해 m_pBoard 를 보관하자
	CBoardWrap* pBoard = m_pBoard;

	// 새로운 SocketBag 과 MFSM 을 생성한다
	CSocketBag* pSB = new CSocketBag();
	CMFSM* pMFSM = new CMFSM( m_rule.Encode(), apPlayers, pSB );

	pSB->SetMFSM( pMFSM );
	for ( int uid = 0; uid < MAX_CONNECTION; uid++ )
		if ( !m_aInfo[uid].bComputer ) {
			pSB->InitForServer( uid, m_aInfo[uid].pSocket );
			m_aInfo[uid].pSocket = 0;	// detach
		}

	pMFSM->Init( 0 );

	pBoard->SetMFSM(pMFSM);

	// DLL 로드가 실패했을 때 경고 메시지를 준다
	if ( bFailed && !sFailedDLL.IsEmpty() ) {

		LPCTSTR asLine[8];
		asLine[0] = _T("다음의 AI DLL 을 읽는데 실패했습니다");
		asLine[1] = _T("");
		asLine[2] = sFailedDLL;
		asLine[3] = _T("");
		asLine[4] = _T("옵션 대화상자의 AI 탭에서 이 파일의");
		asLine[5] = _T("경로를 다시 확인하세요");
		asLine[6] = _T("");
		asLine[7] = _T("임시로 기본 AI 를 읽어들입니다");
		DMessageBox* pMB = new DMessageBox(pBoard);
		if ( Mo()->bUseSound ) MessageBeep( MB_ICONEXCLAMATION );
		pMB->Create( true, 8, asLine );
	}

	return true;
}

// 게임의 시작 (클라이언트로서)
bool DConnect::BeginClient()
{
	int j;
	int nPlayers = m_rule.nPlayerNum;

	CPlayer* apPlayers[MAX_CONNECTION];

	// Human / Network Players 생성
	for ( j = 0; j < MAX_CONNECTION; j++ ) {
		int i = ( m_uid >= nPlayers ? j : j < nPlayers ? ( m_uid + j ) % nPlayers : j );
		
		if ( i == m_uid )
			apPlayers[j] = new CPlayerHuman(
				j, m_aInfo[i].sName, *m_pBoard );
		else
			apPlayers[j] = new CPlayerNetwork(
				j, m_aInfo[i].sName, *m_pBoard );
	}

	// 돈과 전적을 배분
	for ( j = 0; j < nPlayers; j++ ) {
		int i = m_uid >= nPlayers ? j : ( m_uid + j ) % nPlayers;
		apPlayers[j]->SetMoney( m_aInfo[i].dfa[3] );
		apPlayers[j]->GetAllRecord().wm = LOWORD( m_aInfo[i].dfa[0] );
		apPlayers[j]->GetAllRecord().lm = HIWORD( m_aInfo[i].dfa[0] );
		apPlayers[j]->GetAllRecord().wf = LOWORD( m_aInfo[i].dfa[1] );
		apPlayers[j]->GetAllRecord().lf = HIWORD( m_aInfo[i].dfa[1] );
		apPlayers[j]->GetAllRecord().wa = LOWORD( m_aInfo[i].dfa[2] );
		apPlayers[j]->GetAllRecord().la = HIWORD( m_aInfo[i].dfa[2] );
	}

	// 새로운 SocketBag 과 MFSM 을 생성한다
	CSocketBag* pSB = new CSocketBag();
	CMFSM* pMFSM = new CMFSM( m_rule.Encode(), apPlayers, pSB );

	pSB->SetMFSM( pMFSM );

	for ( int uid = 0; uid < nPlayers; uid++ )
		pSB->InitForClient( uid );
	pSB->InitForClient( m_pServerSocket );
	ASSERT( m_pServerSocket );
	m_pServerSocket = 0;	// detach

	pMFSM->Init( m_uid );

	m_pBoard->SetMFSM(pMFSM);

	return true;
}
