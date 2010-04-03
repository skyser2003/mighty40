#if !defined(AFX_POPTIONAI_H__D5FBFC73_4D4F_11D3_99C7_000000000000__INCLUDED_)
#define AFX_POPTIONAI_H__D5FBFC73_4D4F_11D3_99C7_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// POptionAI.h : header file
//

struct MAIDLL_INFO;

/////////////////////////////////////////////////////////////////////////////
// POptionAI dialog

class POptionAI : public CPropertyPage
{
	DECLARE_DYNCREATE(POptionAI)

// Construction
public:
	POptionAI();
	~POptionAI();

protected:
	CString m_asPlayerName[MAX_PLAYERS-1];
	CString m_asPath[MAX_PLAYERS-1];
	CString m_asFileName[MAX_PLAYERS-1];
	CString m_asSetting[MAX_PLAYERS-1];

	void OnBrowse( UINT nID );
	void OnSetting( UINT nID );
	bool GetDllInfo( LPCTSTR sPath, MAIDLL_INFO* pInfo );
	void ShowInfo( int nPlayer, LPCTSTR sPath, LPCTSTR sFile );

// Dialog Data
	//{{AFX_DATA(POptionAI)
	enum { IDD = IDD_OPTION_AI };
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(POptionAI)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(POptionAI)
	afx_msg void OnResetpath();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POPTIONAI_H__D5FBFC73_4D4F_11D3_99C7_000000000000__INCLUDED_)
