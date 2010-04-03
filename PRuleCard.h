#if !defined(AFX_PRULECARD_H__E535DD67_509F_11D3_99D0_0000212035B8__INCLUDED_)
#define AFX_PRULECARD_H__E535DD67_509F_11D3_99D0_0000212035B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PRuleCard.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// PRuleCard dialog

class PRuleCard : public CPropertyPage
{
	DECLARE_DYNCREATE(PRuleCard)

// Construction
public:
	PRuleCard(CRule* pRule = 0,bool bReadOnly=false);
	~PRuleCard();

	CRule* m_pRule;
	bool m_bReadOnly;

// Dialog Data
	//{{AFX_DATA(PRuleCard)
	enum { IDD = IDD_RULE_CARD };
	int		m_nAltJokercall;
	int		m_nAltMighty;
	int		m_nJokercall;
	int		m_nMighty;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(PRuleCard)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(PRuleCard)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRULECARD_H__E535DD67_509F_11D3_99D0_0000212035B8__INCLUDED_)
