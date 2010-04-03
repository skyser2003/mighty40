// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__6433F786_C1B5_11D2_97F2_000000000000__INCLUDED_)
#define AFX_MAINFRM_H__6433F786_C1B5_11D2_97F2_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CMainFrame : public CFrameWnd
{
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:
	// CBoard 뷰를 얻는다
	CBoard* GetView()								{ return &m_wndView; }
	const CBoard* GetView() const					{ return &m_wndView; }

// Operations
public:
	// 윈도우의 종료를 잠그거나 푼다
	void LockClosing()								{ InterlockedIncrement( &m_nLockClosing ); }
	void UnlockClosing()							{ InterlockedDecrement( &m_nLockClosing ); }
	// 윈도우의 크기 변경을 잠그거나 푼다
	void LockSizing();
	void UnlockSizing();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:  // control bar embedded members
	CInfoBar	m_wndInfoBar;
	CBoard		m_wndView;
	long		m_nLockClosing;
	long		m_nLockSizing;
	CSize		m_szLockBefore;
	CCriticalSection m_cs;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnDestroy();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	//}}AFX_MSG
	// CPlayerSocket 참조
	afx_msg LRESULT OnCallSockProc( WPARAM wParam, LPARAM lParam );
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__6433F786_C1B5_11D2_97F2_000000000000__INCLUDED_)
