#if !defined(AFX_DRULE_H__400AD6E8_4FEF_11D3_99CE_000000000000__INCLUDED_)
#define AFX_DRULE_H__400AD6E8_4FEF_11D3_99CE_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DRule.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DRule

class DRule : public CPropertySheet
{
	DECLARE_DYNAMIC(DRule)

// Construction
public:
	DRule(LPCTSTR sRule, bool bReadOnly = FALSE, bool bAutoDelete = FALSE,
		CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

public:
	CRule m_rule;
	bool m_bReadOnly;
	bool m_bAutoDelete;

// Attributes
protected:
	CPropertyPage** m_apPages;
	int m_nPages;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DRule)
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~DRule();

	// Generated message map functions
protected:
	//{{AFX_MSG(DRule)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DRULE_H__400AD6E8_4FEF_11D3_99CE_000000000000__INCLUDED_)
