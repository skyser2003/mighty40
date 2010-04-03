#if !defined(AFX_POPTIONCOMM_H__400AD6E1_4FEF_11D3_99CE_000000000000__INCLUDED_)
#define AFX_POPTIONCOMM_H__400AD6E1_4FEF_11D3_99CE_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// POptionComm.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// POptionComm dialog

class POptionComm : public CPropertyPage
{
	DECLARE_DYNCREATE(POptionComm)

// Construction
public:
	POptionComm();
	~POptionComm();

// Dialog Data
	//{{AFX_DATA(POptionComm)
	enum { IDD = IDD_OPTION_COMM };
	CString	m_sMightyNetAddress;
	UINT	m_nPort;
	CString	m_sAddress;
	BOOL	m_bUseHintInNetwork;
	CString	m_sScore;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(POptionComm)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(POptionComm)
	afx_msg void OnResetscore();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POPTIONCOMM_H__400AD6E1_4FEF_11D3_99CE_000000000000__INCLUDED_)
