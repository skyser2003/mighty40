#if !defined(AFX_POPTIONDSB_H__D5FBFC67_4D4F_11D3_99C7_000000000000__INCLUDED_)
#define AFX_POPTIONDSB_H__D5FBFC67_4D4F_11D3_99C7_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// POptionDSB.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// POptionDSB dialog

class POptionDSB : public CPropertyPage
{
	DECLARE_DYNCREATE(POptionDSB)

// Construction
public:
	POptionDSB();
	~POptionDSB();

// Dialog Data
	//{{AFX_DATA(POptionDSB)
	enum { IDD = IDD_OPTION_DSB };
	//}}AFX_DATA

protected:
	bool m_bDSBOpaque;
	int m_nDSBShadeMode;
	COLORREF m_colDSBText, m_colDSBGray,
		m_colDSBStrong1, m_colDSBStrong2;

	void SelColor( COLORREF& col );

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(POptionDSB)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(POptionDSB)
	afx_msg void OnDsbgraycolselect();
	afx_msg void OnDsbopaque();
	afx_msg void OnDsbshademode();
	afx_msg void OnDsbshademode1();
	afx_msg void OnDsbshademode2();
	afx_msg void OnDsbstrong1colselect();
	afx_msg void OnDsbstrong2colselect();
	afx_msg void OnDsbtextcolselect();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POPTIONDSB_H__D5FBFC67_4D4F_11D3_99C7_000000000000__INCLUDED_)
