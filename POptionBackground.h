#if !defined(AFX_POPTIONBACKGROUND_H__0D25C7BE_4C9A_11D3_99C4_000000000000__INCLUDED_)
#define AFX_POPTIONBACKGROUND_H__0D25C7BE_4C9A_11D3_99C4_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// POptionBackground.h : header file
//

class CBmpMan;

/////////////////////////////////////////////////////////////////////////////
// POptionBackground dialog

class POptionBackground : public CPropertyPage
{
	DECLARE_DYNCREATE(POptionBackground)

// Construction
public:
	POptionBackground();
	~POptionBackground();

// Dialog Data
	//{{AFX_DATA(POptionBackground)
	enum { IDD = IDD_OPTION_BACKGROUND };
	//}}AFX_DATA
	int m_nBackAlign;
	COLORREF m_colDef, m_colAtt, m_colBack;
	bool m_bUseBackground;
	CString m_sBackgroundPath;
	CBmpMan* m_pBm;

protected:
	void SelColor( COLORREF& col );
	bool LoadPic( LPCTSTR sPath );
	bool Browse();
	RECT DrawText( CDC* pDC, int x, int y, COLORREF col,
		const CString& sText );
	void Link( CDC* pDC, UINT idCtrl, const CRect& rc );
	void Link( CDC* pDC, CPoint pnt1, CPoint pnt2 );

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(POptionBackground)
	public:
	virtual void OnCancel();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(POptionBackground)
	afx_msg void OnUsebackground();
	afx_msg void OnAttcolselect();
	afx_msg void OnBackalign();
	afx_msg void OnBackalign1();
	afx_msg void OnBackalign2();
	afx_msg void OnBackgroundbrowse();
	afx_msg void OnBackgroundcolselect();
	afx_msg void OnDefcolselect();
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POPTIONBACKGROUND_H__0D25C7BE_4C9A_11D3_99C4_000000000000__INCLUDED_)
