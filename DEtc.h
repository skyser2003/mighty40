// DEtc.h: interface for the DEtc class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DETC_H__39C76F83_DE75_11D2_984A_000000000000__INCLUDED_)
#define AFX_DETC_H__39C76F83_DE75_11D2_984A_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// 잡다한 DSB 들을 여기에 몰아 놓았다


/////////////////////////////////////////////////////////////////////////////
// about box
class DAbout : public DSB
{
public:
	DAbout( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DAbout() {}

	void Create();

protected:
	// 초기화 ( 생성된 후 호출됨 )
	virtual void OnInit();

	// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
	// 오버라이드 하여 구현해야 한다
	// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
	virtual void OnDraw(
		CDC* pDC,
		bool bBrushOrigin );
};


/////////////////////////////////////////////////////////////////////////////
// message box
class DMessageBox : public DSB
{
public:
	DMessageBox( CBoardWrap* pBoard );
	virtual ~DMessageBox();

	// nLines 줄의 메시지와 ok 버튼을 출력한다
	// nTimeOut 이 -1 이 아니면 ok 버튼은 없다
	void Create( bool bCenter, int nLines, LPCTSTR asLine[],
		int nTimeOut = -1 );

protected:
	// 초기화 ( 생성된 후 호출됨 )
	virtual void OnInit();

	// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
	// 오버라이드 하여 구현해야 한다
	// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
	virtual void OnDraw(
		CDC* pDC,
		bool bBrushOrigin );

protected:
	bool m_bCenter;
	CString* m_asLine;
	int m_nLines;
};

/////////////////////////////////////////////////////////////////////////////
// short message box (짧은 메시지를 잠깐 표시했다가 사라짐)
// 잠깐동안 비트맵 + 문자열 스트링을 화면의 가운데에
// 지정한 시간동안 표시한다
class DShortMessage : public DSB
{
public:
	DShortMessage( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DShortMessage() {}

	// idBitmap : 비트맵 ( 없으면 0 )
	// s        : 메시지 ( 없으면 0 )
	// bCenter  : 메시지가 상자의 가운데 출력되는가?
	// bFixed   : Fixed DSB 인가
	// nTimeOut : 타임아웃 (millisecond)
	// nPlayer  : 표시될 플레이어 번호 ( -1 이면 가운데 )
	// sName    : 이 플레이어의 이름 ( 0 이면 이름 표시안됨 )
	void Create( UINT idBitmap, LPCTSTR s,
		bool bCenter, bool bFixedSize,
		int nTimeOut, int nPlayer = -1, LPCTSTR sName = 0 );

protected:
	// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
	// 오버라이드 하여 구현해야 한다
	// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
	virtual void OnDraw(
		CDC* pDC,
		bool bBrushOrigin );

protected:
	UINT m_idBitmap;
	CString m_s;
	bool m_bCenter;
	CString m_sName;
};


/////////////////////////////////////////////////////////////////////////////
// 몇 개의 텍스트 중에서 하나를 선택하는 모달 DSB
class DSelect : public DSB
{
public:
	DSelect( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DSelect() {}

	// x, y     : 다이얼로그가 생겨야 할 위치 (CBoard 기준좌표)
	// asText   : 선택할 텍스트 배열
	// nText    : asText 의 아이템 개수
	// pResult  :선택 결과의 인덱스 (-1은 취소)
	void Create( int x, int y,
		LPCTSTR asText[], int nText, CEvent* pEvent, long* pResult );

protected:
	// 초기화 ( 생성된 후 호출됨 )
	virtual void OnInit();

	// 핫 스팟을 클릭할 때 불리는 함수 ( 스스로 호출한다 )
	// LPVOID 값이 0 이면 종료 (X자를 클릭) 요청
	virtual void OnClick( LPVOID pVoid );

protected:
	LPCTSTR* m_asText;
	int m_nText;
};


/////////////////////////////////////////////////////////////////////////////
// select jokershape box ( 첫 조커의 모양을 결정하는 상자)
// 이 DSB 는 플레이어가 첫 조커를 냈을 때
// 그 모양을 결정하기 위해 표시된다
// pShape 에 리턴되는 값 : 0 (취소), 또는 SPADE/DIAMOND/HEART/CLOVER
class DSelectJokerShape : public DSB
{
public:
	DSelectJokerShape( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DSelectJokerShape() {}

	// x, y           : 다이얼로그가 생겨야 할 위치 (CBoard 기준좌표)
	void Create( int x, int y,
		CEvent* pEvent, long* pShape);

protected:
	// 초기화 ( 생성된 후 호출됨 )
	virtual void OnInit();

	// 핫 스팟을 클릭할 때 불리는 함수 ( 스스로 호출한다 )
	// LPVOID 값이 0 이면 종료 (X자를 클릭) 요청
	virtual void OnClick( LPVOID pVoid );
};


/////////////////////////////////////////////////////////////////////////////
// 플레이어의 Chat 내용을 출력해 주는 상자를 구현

class DSay  : public DSB
{
public:
	DSay( CBoardWrap* pBoard );
	virtual ~DSay();

	// x, y  : 상자의 중심 값 (DP 좌표)
	//         상자가 pBoard의 클라이언트 영역을 벗어 날 경우
	//         안쪽으로 끌여들여 져서 맞추어 진다
	// cLine : 한 줄에 들어가는 글자의 대략적인 수 (DSB유닛)
	//         (이 줄을 넘어가면 줄바꿈 됨)
	// sWho  : 말한 사람의 이름으로, 맨 윗줄에 강조체로 표시된다
	// sMsg  : 말한 내용
	// nTimeOut : 유지 시간 (millisecond)
	// pDSB  : Board 에 떠 있던 이 플레이어의 이전 대화 상자
	// bSetBelow : 맨 밑으로 갈 것인가
	void Create( DSay* pDSB, int x, int y,
		int cLine, LPCTSTR sWho, LPCTSTR sMsg, int nTimeOut,
		bool bSetBelow = false );

protected:
	// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
	// 오버라이드 하여 구현해야 한다
	// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
	virtual void OnDraw(
		CDC* pDC,
		bool bBrushOrigin );

protected:
	// 내부 데이터 & 함수
	CString m_sWho;		// 말한 사람
	TCHAR** m_asLine;	// 말한 내용 (라인 단위로 파싱 된 상태)
	int m_nLines;		// 위의 배열의 크기

	CString	m_asLast[2];// 지난 DSB 의 마지막 2 줄
	int m_nLastLines;	// 지난 DSB 가 몇줄인가 (0~2)
};


#endif // !defined(AFX_DETC_H__39C76F83_DE75_11D2_984A_000000000000__INCLUDED_)
