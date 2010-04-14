// DConnect.h: interface for the DConnect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DCONNECT_H__2BCC6B66_8ECD_11D3_9A8E_000000000000__INCLUDED_)
#define AFX_DCONNECT_H__2BCC6B66_8ECD_11D3_9A8E_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPlayerSocket;
class DConnectPopup;


// 에러 메시지를 출력한 후에, pToKill 을 없애고 DStartUp 을 출력한다
class DConnectFail : public DMessageBox
{
public:
	DConnectFail( CBoardWrap* pBoard )
		: DMessageBox( pBoard ) {}
	void Create( LPCTSTR s1, LPCTSTR s2, LPCTSTR s3, DSB* pToKill ) {
		m_pToKill = pToKill;
		SetModal();
		LPCTSTR s[3] = { s1, s2, s3 };
		DMessageBox::Create( true, s3 ? 3 : s2 ? 2 : 1, s );
		::MessageBeep( MB_ICONEXCLAMATION );
	}
	virtual void OnClick( LPVOID ) {
		if ( m_pToKill ) m_pToKill->Destroy();
		(new DStartUp(m_pBoard))->Create(0);
		Destroy();
	}
protected:
	DSB* m_pToKill;
};


class DConnecting;

// 접속창에서 사용하는 콤보박스의 에디트를 Subclassing 하는 클래스
struct DConnectingComboEdit : public CEdit
{
	DConnecting* m_pParent;
	afx_msg void OnChar( UINT nChar, UINT nRepCnt, UINT nFlags );
	void SubclassCombobox( CComboBox& box );
	DECLARE_MESSAGE_MAP()
};

// 접속 창
class DConnecting : public DSB
{
public:
	DConnecting( CBoardWrap* pBoard ) : DSB(pBoard), m_pSocket(0) {}
	virtual ~DConnecting();

	void Create();

public:
	CPlayerSocket* DetachSocket()					{ CPlayerSocket* pRet = m_pSocket; m_pSocket = 0; return pRet; }

protected:
	// 초기화 ( 생성된 후 호출됨 )
	virtual void OnInit();

	// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
	// 오버라이드 하여 구현해야 한다
	// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
	virtual void OnDraw(
		CDC* pDC,
		bool bBrushOrigin );

	// 핫 스팟을 클릭할 때 불리는 함수
	// LPVOID 값이 0 이면 종료 (X자를 클릭) 요청
	virtual void OnClick( LPVOID pVoid );

	virtual void Destroy();

protected:
	CPlayerSocket* m_pSocket;
	// ok 버튼 색상
	COLORREF m_colOk;
	// 콤보
	CComboBox m_combo;
	DConnectingComboEdit m_edit;	// m_combo 의 Edit 창
	// 소켓 핸들러
	static void SockProc( CPlayerSocket* pSocket, CMsg* pMsg,
							DWORD dwUser1, DWORD dwUser2 );
	void SockProc( CMsg* pMsg );
	void Fail( LPCTSTR sMsg );

	friend DConnectingComboEdit;
};


// 다른 플레이어의 참여를 기다리는 서버 창
// 또는 서버에 접속하여 서버 상태를 보는 클라이언트 창
class DConnect : public DSB, public CZSocket  
{
public:
	DConnect( CBoardWrap* pBoard );
	virtual ~DConnect();

	// pServerSocket 이 0 이면 이 DSB 는 서버용 접속DSB
	// 0 이 아니면, pServerSocket 을 서버로 하는
	// 클라이언트 DSB
	void Create( CPlayerSocket* pServerSocket );

protected:
	// 초기화 ( 생성된 후 호출됨 )
	virtual void OnInit();

	// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
	// 오버라이드 하여 구현해야 한다
	// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
	virtual void OnDraw(
		CDC* pDC,
		bool bBrushOrigin );

	// 핫 스팟을 클릭할 때 불리는 함수
	// LPVOID 값이 0 이면 종료 (X자를 클릭) 요청
	virtual void OnClick( LPVOID pVoid );

protected:

	// 서버 여부
	bool m_bServer;
	CPlayerSocket* m_pServerSocket;
	// 자신의 UID
	long m_uid;

	// 호스트 주소
	CString m_sAddress;
	// 사용되는 규칙
	CString m_sRule;
	CRule m_rule;
	// 현재 접속되어 있는 사람들의 정보
	struct PLAYERINFO {
		CString sName;	// 이름
		CString sInfo;	// 전적(사람), 또는 AI명(컴퓨터)
		long dfa[4];	// 전적 & 돈
		bool bComputer;	// 컴퓨터인가?
		CPlayerSocket* pSocket;
	}
	m_aInfo[MAX_PLAYERS];
	// 채팅창 내용
	TCHAR (*m_asChatData)[256];
	COLORREF *m_acolChatData;
	DWORD m_nChatDataBegin;	// (현재 위치)
	DWORD m_nChatDataEnd;
	// 채팅창 사각형
	CRect m_rcChat;
	// 세모 마크의 색상
	COLORREF m_acolMark[MAX_PLAYERS]; //v4.0 : 2010.4.6 - 서버 만들었다가 끌 때 튕기는 버그 수정
	// 팝업 메뉴
	DConnectPopup* m_pPopup;
	// 블랙리스트 (이사람들은 접속을 못하게 막음)
	CList<CString,CString&> m_lBlackList;

protected:
	// 유틸리티 (공용)

	// m_acolMark 를 update 함
	void UpdateMarks();
	// 회복불가능한 에러가 발생했을 때 호출
	void Fail( LPCTSTR sMsg1, LPCTSTR sMsg2 = 0 );

	// 시작버튼을 핫스팟에 등록
	void RegisterOk();
	// 사람에 붙는 기호를 등록
	void RegisterMarks();
	// 규칙을 등록
	void RegisterRule();

	// 채팅 메시지를 생성(new)
	CMsg* CreateChatMsg( long uid, LPCTSTR sMsg );
	// uid 위치의 플레이어를 컴퓨터로 치환
	void SetComputer( long uid, int money );

	// 게임의 시작 (서버로서)
	bool BeginServer();
	// 게임의 시작 (클라이언트로서)
	bool BeginClient();

protected:
	// 서버에서 호출하는 유틸리티들

	// 현재 참가한 플레이어들에게 메시지를 전송
	void SendToAll( CMsg* pMsg, long uidExcept = -1 );
	// 현재 m_aInfo 상황을 기술하는 CMsg 객체를 생성(new)
	// 즉, mmPrepare 메시지
	CMsg* CreateStateMsg();
	// m_aInfo 의 i 번째 플레이어에 대한 CMsg 객체를 생성(new)
	// 즉, mmChanged 메시지
	CMsg* CreatePlayerInfoMsg( long uid );
	// mmInit 메시지를 생성(new)
	CMsg* CreateInitMsg();
	// mmUID 메시지를 생성(new)
	CMsg* CreateUIDMsg( long uid );
	// 특정 플레이어에 대한 소켓 호출이 실패
	// (Computer 로 바꾸고 다른 플레이어에게 전달)
	// ( bAccessDenied 가 참이면, 이 플레이어는
	// 추방에 의해 삭제되는것임 )
	void FailedForPlayer( long uid, bool bAccessDenied = false );
	// 플레이어를 하나 추가
	// 리턴되는 값은 uid, -1 이면 빈공간이 없음, -2 이면 허용 거부
	// pMsg : mmNewPlayer 메시지
	long AddPlayer( CMsg* pMsg, CPlayerSocket* pSocket = 0 );
	// 플레이어를 삭제 ( bAccessDenied 가 참이면, 이 플레이어는
	// 추방에 의해 삭제되는것임 )
	void RemovePlayer( long uid, bool bAccessDenied = false );

	// 접속 요청
	virtual void OnAccept( int nErr );

	// 소켓 핸들러
	static void ServerSockProc( CPlayerSocket* pSocket, CMsg* pMsg,
								DWORD dwUser1, DWORD dwUser2 );
	void ServerSockProc( long uid, CMsg* pMsg, CPlayerSocket* pSocket );

protected:
	// 클라이언트에서 호출하는 유틸리티들

	// mmNewPlayer 메시지를 생성
	CMsg* CreateNewPlayerMsg();
	// mmUID 메시지를 수신하여 Update
	bool ReceiveUIDMsg( CMsg* pMsg );
	// mmPrepare 메시지를 수신
	bool ReceiveStateMsg( CMsg* pMsg );
	// mmChanged 메시지를 수신
	bool ReceivePlayerInfoMsg( CMsg* pMsg );
	// mmChat 메시지를 수신
	bool ReceiveChatMsg( CMsg* pMsg );

	// 소켓 핸들러
	static void ClientSockProc( CPlayerSocket* pSocket, CMsg* pMsg,
								DWORD dwUser1, DWORD dwUSer2 );
	void ClientSockProc( CMsg* pMsg );

protected:
	// 채팅창 관련

	// 채팅 화면에 sMsg 를 그린다 (uid==-1 이면 전역 메시지)
	void Chat( LPCTSTR sMsg, long uid = -1, bool bUpdate = true );

	// 채팅창 핸들러
	static void ChatProc( LPCTSTR sMsg, DWORD dwUser );
	void ChatProc( LPCTSTR sMsg );

public:
	// DConnectPopup 이 호출하는 함수

	// 플레이어에 붙어있는 세모 마크 위치를 얻는다
	CPoint GetMarkPos( long uid ) const
	{	return DSBtoDP( CPoint( 1, ( m_rule.nPlayerNum == 6 ? 5 : m_rule.nPlayerNum == 7 ? 4 : 7 ) + uid*3 + 1 ) ); }
	// m_pPopup 을 클리어한다
	void ClearPopup() { m_pPopup = 0; }
	// OnClick 을 호출한다
	void Click( LPVOID pVoid ) { OnClick( pVoid ); }
};


// DConnect 에서 나오는 선택 메뉴
class DConnectPopup : public DSelect
{
public:
	DConnectPopup( DConnect* pOuter, CBoardWrap* pBoard )
		: DSelect(pBoard), m_pOuter(pOuter) {}
	virtual ~DConnectPopup()
	{	// 예 213 : uid==1 && result==3
		m_pOuter->Click( (LPVOID)( 200 + m_uid*10 + m_nResult ) );
		m_pOuter->ClearPopup();
	}

	void Create( long uid )
	{
		m_uid = uid;
		CPoint pt = m_pOuter->GetMarkPos( uid );
		SetModal();
		DSelect::Create( pt.x, pt.y, s_asText, 3, 0, &m_nResult );
	}

protected:
	DConnect* m_pOuter;
	long m_nResult;
	long m_uid;
	static LPCTSTR s_asText[];
};


#endif // !defined(AFX_DCONNECT_H__2BCC6B66_8ECD_11D3_9A8E_000000000000__INCLUDED_)
