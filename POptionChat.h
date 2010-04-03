#if !defined(AFX_POPTIONCHAT_H__400AD6E4_4FEF_11D3_99CE_000000000000__INCLUDED_)
#define AFX_POPTIONCHAT_H__400AD6E4_4FEF_11D3_99CE_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// POptionChat.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// POptionChat dialog

class POptionChat : public CPropertyPage
{
	DECLARE_DYNCREATE(POptionChat)

// Construction
public:
	POptionChat();
	~POptionChat();

// Dialog Data
	//{{AFX_DATA(POptionChat)
	enum { IDD = IDD_OPTION_CHAT };
	UINT	m_nChatBufferSize;
	int		m_nChatDSBTimeOut;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(POptionChat)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(POptionChat)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POPTIONCHAT_H__400AD6E4_4FEF_11D3_99CE_000000000000__INCLUDED_)
