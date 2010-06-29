#if !defined(AFX_POPTIONRULE_H__D5FBFC69_4D4F_11D3_99C7_000000000000__INCLUDED_)
#define AFX_POPTIONRULE_H__D5FBFC69_4D4F_11D3_99C7_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// POptionRule.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// POptionRule dialog

class POptionRule : public CPropertyPage
{
	DECLARE_DYNCREATE(POptionRule)

// Construction
public:
	POptionRule();
	~POptionRule();

// Dialog Data
	//{{AFX_DATA(POptionRule)
	enum { IDD = IDD_OPTION_RULE };
	CString	m_sRuleString;
	int		m_nRulePreset;
	//}}AFX_DATA
	CString m_sCustom;


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(POptionRule)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(POptionRule)
	afx_msg void OnShowrule();
	afx_msg void OnSelchangeRulepreset();
	afx_msg void OnClickedAddrule();
	afx_msg void OnClickedRemoverule();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CButton m_btAddRule;
	CButton m_btRemoveRule;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POPTIONRULE_H__D5FBFC69_4D4F_11D3_99C7_000000000000__INCLUDED_)
