#if !defined(AFX_PRULECONSTRAIN_H__EF3A11B5_505B_11D3_99CF_0000212035B8__INCLUDED_)
#define AFX_PRULECONSTRAIN_H__EF3A11B5_505B_11D3_99CF_0000212035B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PRuleConstrain.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// PRuleConstrain dialog

class PRuleConstrain : public CPropertyPage
{
	DECLARE_DYNCREATE(PRuleConstrain)

// Construction
public:
	PRuleConstrain(CRule* pRule = 0,bool bReadOnly=false);
	~PRuleConstrain();

	CRule* m_pRule;
	bool m_bReadOnly;

// Dialog Data
	//{{AFX_DATA(PRuleConstrain)
	enum { IDD = IDD_RULE_CONSTRAIN };
	BOOL	m_bInitBeginKiruda;
	BOOL	m_bInitJoker;
	BOOL	m_bInitJokercallEffect;
	BOOL	m_bInitJokerEffect;
	BOOL	m_bInitKiruda;
	BOOL	m_bInitMighty;
	BOOL	m_bInitMightyEffect;
	BOOL	m_bJokercallJokerEffect;
	BOOL	m_bLastJoker;
	BOOL	m_bLastJokerEffect;
	BOOL	m_bLastMighty;
	BOOL	m_bLastMightyEffect;
	BOOL	m_bOverrideMighty;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(PRuleConstrain)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(PRuleConstrain)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRULECONSTRAIN_H__EF3A11B5_505B_11D3_99CF_0000212035B8__INCLUDED_)
