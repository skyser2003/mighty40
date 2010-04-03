#if !defined(AFX_POPTIONCARD_H__0D25C7BC_4C9A_11D3_99C4_000000000000__INCLUDED_)
#define AFX_POPTIONCARD_H__0D25C7BC_4C9A_11D3_99C4_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// POptionCard.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// POptionCard dialog

class CBmpMan;

class POptionCard : public CPropertyPage
{
	DECLARE_DYNCREATE(POptionCard)

// Construction
public:
	POptionCard();
	~POptionCard();

// Dialog Data
	//{{AFX_DATA(POptionCard)
	enum { IDD = IDD_OPTION_CARD };
	CStatic	m_cSampleBack;
	BOOL	m_bCardTip;
	BOOL	m_bCardHelp;
	BOOL	m_bZoomCard;
	//}}AFX_DATA
	int m_nBackPicture;
	CString m_sBackPicturePath;

protected:
	CBmpMan* m_pBm;
	void DrawSample( CDC* pDC );

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(POptionCard)
	public:
	virtual void OnCancel();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(POptionCard)
	afx_msg void OnBackselect();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POPTIONCARD_H__0D25C7BC_4C9A_11D3_99C4_000000000000__INCLUDED_)
