#if !defined(AFX_POPTIONGENERAL_H__0D25C7B5_4C9A_11D3_99C4_000000000000__INCLUDED_)
#define AFX_POPTIONGENERAL_H__0D25C7B5_4C9A_11D3_99C4_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// POptionGeneral.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// POptionGeneral dialog

class POptionGeneral : public CPropertyPage
{
	DECLARE_DYNCREATE(POptionGeneral)

// Construction
public:
	POptionGeneral();
	~POptionGeneral();

// Dialog Data
	//{{AFX_DATA(POptionGeneral)
	enum { IDD = IDD_OPTION_GENERAL };
	BOOL	m_bUseTerm;
	BOOL	m_bUseSound;
	int		m_nSortMode;
	CString	m_sHumanName;
	BOOL	m_bMoneyAsSupport;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(POptionGeneral)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(POptionGeneral)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POPTIONGENERAL_H__0D25C7B5_4C9A_11D3_99C4_000000000000__INCLUDED_)
