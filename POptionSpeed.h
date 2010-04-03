#if !defined(AFX_POPTIONSPEED_H__D5FBFC68_4D4F_11D3_99C7_000000000000__INCLUDED_)
#define AFX_POPTIONSPEED_H__D5FBFC68_4D4F_11D3_99C7_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// POptionSpeed.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// POptionSpeed dialog

class POptionSpeed : public CPropertyPage
{
	DECLARE_DYNCREATE(POptionSpeed)

// Construction
public:
	POptionSpeed();
	~POptionSpeed();

// Dialog Data
	//{{AFX_DATA(POptionSpeed)
	enum { IDD = IDD_OPTION_SPEED };
	int		m_nCardMotion;
	int		m_nCardSpeed;
	int		m_nDelayOneCard;
	int		m_nDelayOneTurn;
	BOOL	m_bShowDealing;
	BOOL	m_bSlowMachine;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(POptionSpeed)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(POptionSpeed)
	virtual BOOL OnInitDialog();
	afx_msg void OnSlowmachine();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POPTIONSPEED_H__D5FBFC68_4D4F_11D3_99C7_000000000000__INCLUDED_)
