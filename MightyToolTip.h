#if !defined(AFX_MIGHTYTOOLTIP_H__0D25C7B9_4C9A_11D3_99C4_000000000000__INCLUDED_)
#define AFX_MIGHTYTOOLTIP_H__0D25C7B9_4C9A_11D3_99C4_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MightyToolTip.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMightyToolTip window

class CMightyToolTip : public CWnd
{
// Construction
public:
	CMightyToolTip();

public:
	// 주어진 스트링으로 팁을 출력한다
	// 윈도우가 생성되지 않았으면 새로 생성한다
	void Tip( int x, int y, LPCTSTR s, int cx = DEFAULT_TIP_CX );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMightyToolTip)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMightyToolTip();

protected:
	CString m_sText;	// 보여지는 텍스트
	void CalcFittingRect( LPRECT prc );

	// Generated message map functions
protected:
	//{{AFX_MSG(CMightyToolTip)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	static CFont s_font;
	static CString s_sWndClass;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIGHTYTOOLTIP_H__0D25C7B9_4C9A_11D3_99C4_000000000000__INCLUDED_)
