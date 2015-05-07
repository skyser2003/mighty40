// InfoBar.h: interface for the CInfoBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INFOBAR_H__BE37E2C6_C23D_11D2_97F3_000000000000__INCLUDED_)
#define AFX_INFOBAR_H__BE37E2C6_C23D_11D2_97F3_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// 아래쪽의 상태바
// 멀티 플레잉 게임일 때 채팅창으로도 사용된다

// 공통 기능
// 기루다, 목표점수, 프랜드 텍스트를 세트 할 수 있다

// 상태바 사용
// 현재 표시될 텍스트와 진행 상태 바를 표시할 수 있다

// 채팅창 사용 (ShowChat(true)함수로 채팅창 모드로 바꾼다)
// 입력된 메시지는, 채팅 메시지 핸들러로 전달된다
// (SetChatHandler 함수)

class CInfoBar : public CStatusBar  
{
public:
	CInfoBar();
	virtual ~CInfoBar();
	BOOL Create( CFrameWnd* pParent );
public:
	// 출력을 위한 함수들

	// 초기화
	void Reset()									{ SafeSendMessage( WM_INFOBARUPDATE, 0, 0 ); }

	// 기루다, 목표점수, 프랜드 텍스트 세트
	void SetKiruda( int nKiruda )					{ SafeSendMessage( WM_INFOBARUPDATE, 1, (LPARAM)nKiruda ); }
	void SetMinScore( int nMinScore, int nDefaultMinScore ) { SafeSendMessage( WM_INFOBARUPDATE, 2, (LPARAM)MAKELONG((WORD)nMinScore,(WORD)nDefaultMinScore) ); }
	void SetFriendText( LPCTSTR sFriendText )		{ SafeSendMessage( WM_INFOBARUPDATE, 3, (LPARAM)sFriendText ); }

	// 현재 진행률 표시 : 100 분의 nPercentage 만큼이
	//                    반전되어 표시된다
	void SetProgress( int nPercentage )				{ SafeSendMessage( WM_INFOBARUPDATE, 4, (LPARAM)nPercentage ); }
	// 표시되는 메시지
	void SetText( LPCTSTR sText )					{ SafeSendMessage( WM_INFOBARUPDATE, 5, (LPARAM)sText ); }

	// 채팅 컨트롤을 표시하거나 감춘다
	void ShowChat( bool bShow = true )				{ SafeSendMessage( WM_INFOBARUPDATE, 6, bShow ? 1 : 0 ); }
	// 채팅창에 입력 포커스를 준다 ( 성공시 true )
	bool SetFocusToChat()							{ return SafeSendMessage( WM_INFOBARUPDATE, 7, 0 ) ? true : false; }

	// 채팅 메시지 핸들러를 세트
	// 해제(pfnProc==0) 하기 위해서는 기존의 dwUser 값이
	// 일치해야 한다
	void SetChatHandler( DWORD dwUser,
		void (*pfnProc)( LPCTSTR sMsg, DWORD dwUser ) ) { CHATHANDLERINFO chi = { dwUser, pfnProc }; SafeSendMessage( WM_INFOBARUPDATE, 8, (LPARAM)(LPCVOID)&chi ); }

protected:
	// 오버라이드
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
	afx_msg void OnSize( UINT nType, int cx, int cy );
	virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );
	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	virtual BOOL PreTranslateMessage( MSG *pMsg );
	afx_msg BOOL OnToolTipNeedTextW( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	afx_msg BOOL OnToolTipNeedTextA( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	LRESULT SafeSendMessage( UINT u, WPARAM w, LPARAM l ) { return GetSafeHwnd() ? SendMessage( u, w, l ) : 0; }
	LRESULT OnInfoBarUpdate( WPARAM, LPARAM );
	afx_msg LRESULT OnSetText( WPARAM, LPARAM lParam );
	DECLARE_MESSAGE_MAP()

protected:
	// 오른쪽 텍스트 패인 그리기
	void DrawMainPane( CDC* pDC );

protected:
	// 기루다
	int m_nKiruda;
	// 목표점수, 기본점수
	int m_nMinScore;
	int m_nDefaultMinScore;
	// 진행률
	int m_nPercentage;
	// 프랜드 텍스트
	CString m_sFriend;
	// 채팅창
	CEdit m_cChat;
	// 채팅 모드인가
	bool m_bChat;
	// 현재 기루다, 목표점수, 프랜드 텍스트를 가지고 공약 스트링을 리턴한다
	CString GetGoalString() const;

	// 채팅 핸들러 정보
	struct CHATHANDLERINFO {
		DWORD dwUser;
		void (*pfnProc)( LPCTSTR sMsg, DWORD dwUser );
	} m_handler;

	struct CChatBuffer;
	CChatBuffer* m_pChatBuffer;
};

#endif // !defined(AFX_INFOBAR_H__BE37E2C6_C23D_11D2_97F3_000000000000__INCLUDED_)
