#if !defined(AFX_DOPTION_H__0D25C7B6_4C9A_11D3_99C4_000000000000__INCLUDED_)
#define AFX_DOPTION_H__0D25C7B6_4C9A_11D3_99C4_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DOption.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DOption

class DOption : public CPropertySheet
{
	DECLARE_DYNAMIC(DOption)

// Construction
public:
	DOption( CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
protected:
	CPropertyPage** m_apPages;
	int m_nPages;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DOption)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~DOption();

	// Generated message map functions
protected:
	//{{AFX_MSG(DOption)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DOPTION_H__0D25C7B6_4C9A_11D3_99C4_000000000000__INCLUDED_)
