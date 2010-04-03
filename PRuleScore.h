#if !defined(AFX_PRULESCORE_H__E535DD66_509F_11D3_99D0_0000212035B8__INCLUDED_)
#define AFX_PRULESCORE_H__E535DD66_509F_11D3_99D0_0000212035B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PRuleScore.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// PRuleScore dialog

class PRuleScore : public CPropertyPage
{
	DECLARE_DYNCREATE(PRuleScore)

// Construction
public:
	PRuleScore(CRule* pRule = 0,bool bReadOnly=false);
	~PRuleScore();

	CRule* m_pRule;
	bool m_bReadOnly;

// Dialog Data
	//{{AFX_DATA(PRuleScore)
	enum { IDD = IDD_RULE_SCORE };
	int		m_nRun;
	int		m_nMethod;
	BOOL	m_bS_NoFriend;
	BOOL	m_bS_NoKiruda;
	BOOL	m_bS_ReverseRun;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(PRuleScore)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(PRuleScore)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRULESCORE_H__E535DD66_509F_11D3_99D0_0000212035B8__INCLUDED_)
