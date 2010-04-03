#if !defined(AFX_DBACKSELECT_H__0D25C7BD_4C9A_11D3_99C4_000000000000__INCLUDED_)
#define AFX_DBACKSELECT_H__0D25C7BD_4C9A_11D3_99C4_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DBackSelect.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DBackSelect dialog

class DBackSelect : public CDialog
{
// Construction
public:
	DBackSelect(CWnd* pParent = NULL);   // standard constructor

public:
	int m_nBackPicture;
	CString m_sBackPicturePath;

// Dialog Data
	//{{AFX_DATA(DBackSelect)
	enum { IDD = IDD_SELECTBACK };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DBackSelect)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void OnCardsClick( UINT nID );

	// Generated message map functions
	//{{AFX_MSG(DBackSelect)
	afx_msg void OnBackbrowse();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DBACKSELECT_H__0D25C7BD_4C9A_11D3_99C4_000000000000__INCLUDED_)
