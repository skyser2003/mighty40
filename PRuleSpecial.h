#ifndef PRULESPECIAL_H
#define PRULESPECIAL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PRuleSpecial.h : header file (v4.0 : 2010.4.2)
//

/////////////////////////////////////////////////////////////////////////////
// PRuleSpecial dialog

class PRuleSpecial : public CPropertyPage
{
	DECLARE_DYNCREATE(PRuleSpecial)

// Construction
public:
	PRuleSpecial(CRule* pRule = 0,bool bReadOnly=false);
	~PRuleSpecial();

	CRule* m_pRule;
	bool m_bReadOnly;

// Dialog Data
	//{{AFX_DATA(PRuleSpecial)
	enum { IDD = IDD_RULE_SPECIAL };
	int		m_nRun;
	BOOL	m_bS_NoFriend;
	BOOL	m_bS_NoKiruda;
	BOOL	m_bS_ReverseRun;
	int		m_nReverseRun;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(PRuleSpecial)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(PRuleSpecial)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(PRULESPECIAL_H)
