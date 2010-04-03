#if !defined(AFX_PRULEDEALMISS_H__E535DD65_509F_11D3_99D0_0000212035B8__INCLUDED_)
#define AFX_PRULEDEALMISS_H__E535DD65_509F_11D3_99D0_0000212035B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PRuleDealMiss.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// PRuleDealMiss dialog

class PRuleDealMiss : public CPropertyPage
{
	DECLARE_DYNCREATE(PRuleDealMiss)

// Construction
public:
	PRuleDealMiss(CRule* pRule = 0,bool bReadOnly=false);
	~PRuleDealMiss();

	CRule* m_pRule;
	bool m_bReadOnly;

// Dialog Data
	//{{AFX_DATA(PRuleDealMiss)
	enum { IDD = IDD_RULE_DEALMISS };
	BOOL	m_bDM_AllPoint;
	BOOL	m_bDM_Only10;
	BOOL	m_bDM_JokerIsReversePoint;
	BOOL	m_bDM_NoPoint;
	BOOL	m_bDM_OneEyedJack;
	BOOL	m_bDM_OnlyMighty;
	BOOL	m_bDM_OnlyOne;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(PRuleDealMiss)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(PRuleDealMiss)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRULEDEALMISS_H__E535DD65_509F_11D3_99D0_0000212035B8__INCLUDED_)
