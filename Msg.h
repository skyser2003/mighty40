// Msg.h: interface for the CMsg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MSG_H__97BF9F53_DAD5_11D2_9835_000000000000__INCLUDED_)
#define AFX_MSG_H__97BF9F53_DAD5_11D2_9835_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCardList;

// 네트워크로 전송되는 메시지 객체
// 전통적인 메시지 인코딩 방법인 push/pop 형태로 사용한다
// 메시지 버퍼의 맨 앞 4 바이트는 메시지의 길이가 포함
// 맨 끝 4 바이트는 (PushChecksum()을 호출한 경우) 체크섬

// 참고 : 플레이어ID  : 그 호스트 내에서의 플레이어의 고유한 ID
//                      6마때 죽은 플레이어까지 함께 포함하며 CState::apAllPlayers
//                      내에서의 인덱스이다
//        플레이어번호: 실제로 게임을 하는 플레이어의 번호
//                      CState::apPlayer 내에서의 인덱스로, 사람이 플레이
//                      하는 경우 0 번은 항상 사람이다
//        플레이어UID : 이 호스트 내에서 플레이어ID 가 0 인 플레이어가
//                      실제 서버에서 갖는 플레이어ID

class CMsg  
{
public:
	// 메시지 타입
	// 메시지의 첫 정보는 항상 4바이트 정수로, 그 값은
	// 이 메시지 타입이며, 그 메시지의 종류를 말해준다
	enum MSG_TYPE {

		mmNothing = 0,

		// 일반 메시지들

		/////////////////////////////////////////////////////////////////////
		// 접속 메시지
		// Socket 이 Connect 되었을 때 최초로 더해지는 메시지
		// 형식 : WSAE 에러 (l)
		mmConnected = -1,

		/////////////////////////////////////////////////////////////////////
		// 접속 종료 메시지
		// Socket 이 Disconnect 되었을 때 최종적으로 더해지는 메시지
		// 형식 :
		mmDisconnected = -2,

		/////////////////////////////////////////////////////////////////////
		// 에러 메시지
		// 에러가 발생했을 때 그 내용을 반환
		// 형식 : 에러내용(s)
		mmError = -3,

		// 접속 관련 메시지들 

		/////////////////////////////////////////////////////////////////////
		// 초기화 메시지
		// 서버가 accept 하면서 응답으로 보내며, Major 버전값을 송신한다
		// 형식 : MajorVersion (l), 플레이어 수 (l)
		mmInit = 1,

		/////////////////////////////////////////////////////////////////////
		// 새로운 플레이어 등록 요청 메시지
		// 클라이언트가 서버에 접속하여, 새로운 플레이어로서
		// 자신을 등록 요청한다 - 이 응답으로는 mmUID 또는 mmError 가
		// 될 수 있다
		// 형식 : 이름(s), 전적(lx3), 돈(l)
		mmNewPlayer = 2,

		/////////////////////////////////////////////////////////////////////
		// UID 지정 메시지
		// mmNewPlayer 에 대해서, 그 사람의 UID 를 지정해 주는 메시지
		// 형식 : UID(l)
		mmUID = 3,

		/////////////////////////////////////////////////////////////////////
		// 게임 시작 준비 메시지
		// mmNewPlayer 에 대한 응답으로, 현재 접속되어 있는
		// 사람들과 상태 정보를 전달한다 (DConnect.h 참조)
		// 형식 : 게임 규칙 번호(l)
		// + 게임 규칙(규칙번호가 0 이면 rule string, 아니면 규칙이름)(s)
		// + { i 번 플레이어의 이름(s), 상태(s), 컴퓨터인가 여부(l), 전적&돈(lx4) }
		//    x 사람수 만큼 i 를 반복
		mmPrepare = 4,

		/////////////////////////////////////////////////////////////////////
		// 상태 변경 메시지
		// mmPrepare 메시지로 전달된 정보 중 변경된 내용을 전달
		// 형식 : 변경된 플레이어의 uid(l), 이름(s), 상태(s),
		//        컴퓨터인가 여부(l)
		mmChanged = 5,

		/////////////////////////////////////////////////////////////////////
		// 게임 시작 메시지
		// 형식 :
		mmBeginGame = 6,

		// 게임 메시지

		/////////////////////////////////////////////////////////////////////
		// 게임 시작 메시지
		// 형식 : 보낸 사람 UID(l), 덱 (C)
		mmGameInit = 100,

		/////////////////////////////////////////////////////////////////////
		// 공약 메시지
		// 형식 : 보낸 사람 UID(l), 기루다(l), 목표점수(l), 프랜드(l)
		mmGameElection = 101,

		/////////////////////////////////////////////////////////////////////
		// 죽이기 메시지
		// 형식 : 보낸 사람 UID(l), 죽일 카드(l)
		mmGameKillOneFromSix = 102,

		/////////////////////////////////////////////////////////////////////
		// 죽인후 카드 섞기 메시지
		// 형식 : 보낸 사람 UID(l), 덱 (C)
		mmGameSuffledForDead = 103,

		/////////////////////////////////////////////////////////////////////
		// 당선된 주공의 특권 메시지
		// 형식 : 보낸 사람 UID(l), 공약(lx3), 버릴카드(lx3)
		mmGamePrivilege = 104,

		/////////////////////////////////////////////////////////////////////
		// 턴
		// 형식 : 보낸 사람 UID(l), 카드(l), 효과(l)
		mmGameTurn = 105,

		/////////////////////////////////////////////////////////////////////
		// 2 마에서 카드 고르기
		// 형식 : 보낸 사람 UID(l), 고른 여부(l)
		mmGet2MA = 106,

		/////////////////////////////////////////////////////////////////////
		// 채팅 메시지
		// 형식 : 보낸 사람 UID(l), 메시지 내용(s)
		mmChat = 1000,

		// 기타

		/////////////////////////////////////////////////////////////////////
		// CPlayerSocket 과 CMainFrame 사이에서,
		// Call-back 함수 정보를 전달하는 로컬 메시지
		// 형식 : 각 클래스 참조
		mmCallSockProc = 10000,

	};

public:
	// 메시지를 생성할 때 printf 처럼 포맷 스트링으로 빠르게
	// 메시지를 만든다  예: CMsg( _T("llsk"), 10, 10, _T("haha") )
	// l : long          s : LPCTSTR
	// S : CString*      C : CCardList*
	// k : checksum (인자 없음)
	CMsg( LPCTSTR sFormat, ... );
	CMsg()											{ Init(); }
	CMsg( const CMsg& msg )							{ Init(); *this = msg; }
	CMsg& operator=( const CMsg& msg );
	virtual ~CMsg();

public:
	// 메시지의 길이를 구한다
	int GetLength() const							{ return *m_pLen; }
	// 메시지의 내용이 들어 있는 버퍼의 주소를 구한다
	// 이 버퍼의 시작부에는 버퍼의 길이가 들어있다
	const void* GetBuffer() const					{ return (void*)m_pBuffer; }
	// 메시지 버퍼의 길이를 설정하고 버퍼의 주소를 리턴한다
	// 이 주소의 맨 앞에는 버퍼의 길이가 들어있어야 한다
	void* LockBuffer( int nLen );
	void ReleaseBuffer();
	// 메시지의 가장 앞부분에 있는 long 데이터를 읽는다
	// 이 값은 보통 메시지 타입이다
	long GetType() const							{ ASSERT( *m_pLen >= sizeof(long)*2 ); return *m_pLen >= sizeof(long)+sizeof(long) ? *(long*)(m_pBuffer+sizeof(long)) : (long)mmNothing; }

public:
	// 제대로 된 메시지인가 ( 맨 끝의 체크섬을 검사한다 )
	bool IsValid() const;
	// 체크섬을 맨 끝에 push 한다
	void PushChecksum();

public:
	// 정수, 문자열, 카드 리스트를 각각 push 한다
	void PushLong( long );
	void PushString( LPCTSTR s);
	void PushCardList( const CCardList& );
	// 정수, 문자열, 카드 리스트를 각각 Pump 한다
	// 실패시 false 리턴
	bool PumpLong( long& );
	bool PumpString( CString& );
	bool PumpCardList( CCardList& );
	// 읽기(Pump) 포인터를 처음으로 리셋한다
	void Rewind()									{ m_rptr = m_pBuffer+sizeof(long); }

protected:
	// 내부 데이터와 보조 함수
	int m_nBufferLength;// 할당된 버퍼 크기
	char* m_pBuffer;	// 버퍼
	long* m_pLen;		// 메시지 길이 (버퍼중 맨 앞의 4바이트)
	char* m_wptr;		// 현재 쓰기 포인터 ( 내용의 끝을 가리킨다 )
	const char* m_rptr;	// 현재 읽기 포인터

	// 초기화
	void Init();
	// 버퍼가 nMinLength 보다 작으면 버퍼를 확장한다
	void EnsureBufferLength( int nMinLength );
	// 지정된 메모리 영역의 체크섬을 계산한다
	static long CalcChecksum( const char* p, int len );
};

#endif // !defined(AFX_MSG_H__97BF9F53_DAD5_11D2_9835_000000000000__INCLUDED_)
