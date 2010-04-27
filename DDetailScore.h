#if !defined(AFX_PDETAILSCORE)
#define AFX_PDETAILSCORE

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DDetailScore.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DDetailScore dialog

class DDetailScore : public CDialog
{
// Construction
public:
	DDetailScore(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(POptionComm)
	enum { IDD = IDD_DETAILSCORE };
	CString	m_sScore[6];
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(POptionComm)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(POptionComm)
	afx_msg void OnResetscore();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PDETAILSCORE)
