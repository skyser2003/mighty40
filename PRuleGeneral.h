#if !defined(AFX_PRULEGENERAL_H__400AD6E7_4FEF_11D3_99CE_000000000000__INCLUDED_)
#define AFX_PRULEGENERAL_H__400AD6E7_4FEF_11D3_99CE_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PRuleGeneral.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// PRuleGeneral dialog

class PRuleGeneral : public CPropertyPage
{
	DECLARE_DYNCREATE(PRuleGeneral)

// Construction
public:
	PRuleGeneral(CRule* pRule=0,bool bReadOnly=false);
	~PRuleGeneral();

	CRule* m_pRule;
	bool m_bReadOnly;

// Dialog Data
	//{{AFX_DATA(PRuleGeneral)
	enum { IDD = IDD_RULE_GENERAL };
	int		m_nPlayerNum;
	BOOL	m_bFriend;
	BOOL	m_bJokerFriend;
	BOOL	m_bShowFriend;
	BOOL	m_bFriendGetsBeginer;
	BOOL	m_bRandomSeat;
	BOOL	m_bAttScoreThrownPoints;
	BOOL	m_bHideScore;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(PRuleGeneral)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(PRuleGeneral)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void setFriendCheckbox(bool enable);

private:
	CButton m_btFriend;
	CButton m_btJokerFriend;
	CButton m_btShowFriend;
	CButton m_btFriendGetsBeginer;
public:
	afx_msg void OnClicked2MA();
	afx_msg void OnClicked3MA();
	afx_msg void OnClicked4MA();
	afx_msg void OnClicked5MA();
	afx_msg void OnClicked6MA();
	afx_msg void OnClicked7MA();
//	afx_msg void OnClickedSaverule();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRULEGENERAL_H__400AD6E7_4FEF_11D3_99CE_000000000000__INCLUDED_)
