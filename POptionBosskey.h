#if !defined(AFX_POPTIONBOSSKEY_H__0D25C7BB_4C9A_11D3_99C4_000000000000__INCLUDED_)
#define AFX_POPTIONBOSSKEY_H__0D25C7BB_4C9A_11D3_99C4_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// POptionBosskey.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// POptionBosskey dialog

class POptionBosskey : public CPropertyPage
{
	DECLARE_DYNCREATE(POptionBosskey)

// Construction
public:
	POptionBosskey();
	~POptionBosskey();

// Dialog Data
	//{{AFX_DATA(POptionBosskey)
	enum { IDD = IDD_OPTION_BOSSKEY };
	int		m_nBossType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(POptionBosskey)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(POptionBosskey)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POPTIONBOSSKEY_H__0D25C7BB_4C9A_11D3_99C4_000000000000__INCLUDED_)
