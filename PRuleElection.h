#if !defined(AFX_PRULEELECTION_H__400AD6E9_4FEF_11D3_99CE_000000000000__INCLUDED_)
#define AFX_PRULEELECTION_H__400AD6E9_4FEF_11D3_99CE_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PRuleElection.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// PRuleElection dialog

class PRuleElection : public CPropertyPage
{
	DECLARE_DYNCREATE(PRuleElection)

// Construction
public:
	PRuleElection(CRule* pRule = 0,bool bReadOnly=false);
	~PRuleElection();

	CRule* m_pRule;
	bool m_bReadOnly;

// Dialog Data
	//{{AFX_DATA(PRuleElection)
	enum { IDD = IDD_RULE_ELECTION };
	BOOL	m_bBeginerPass;
	BOOL	m_bHighScore;
	int		m_nMinScore;
	BOOL	m_bRaise2ForKirudaChange;
	BOOL	m_bPassAgain;
	int		m_nNoKirudaAdvantage;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(PRuleElection)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(PRuleElection)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRULEELECTION_H__400AD6E9_4FEF_11D3_99CE_000000000000__INCLUDED_)
