#if !defined(AFX_DMAIINFO_H__B03ADC43_4F26_11D3_99CB_000000000000__INCLUDED_)
#define AFX_DMAIINFO_H__B03ADC43_4F26_11D3_99CB_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DMaiInfo.h : header file
//

struct MAIDLL_INFO;

/////////////////////////////////////////////////////////////////////////////
// DMaiInfo dialog

class DMaiInfo : public CDialog
{
// Construction
public:
	DMaiInfo( const MAIDLL_INFO* pInfo, LPCTSTR sFileName,
		LPCTSTR sPathName, CString& sSetting,
		CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DMaiInfo)
	enum { IDD = IDD_MAIINFO };
	CString	m_sExplain;
	CString	m_sFileName;
	CString	m_sName;
	//}}AFX_DATA

	CString m_sPathName;
	CString& m_sSetting;
	void OnChangeoption();
	bool AdjustSetting();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DMaiInfo)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DMaiInfo)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DMAIINFO_H__B03ADC43_4F26_11D3_99CB_000000000000__INCLUDED_)
