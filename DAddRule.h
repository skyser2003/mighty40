#if !defined(AFX_DADDRULE)
#define AFX_DADDRULE

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DDetailScore.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DAddRule dialog

class DAddRule : public CDialog
{
	DECLARE_DYNAMIC(DAddRule)

public:
	DAddRule(CWnd* pParent = NULL);   // standard constructor
	virtual ~DAddRule();

	virtual void OnFinalRelease();

// Dialog Data
	enum { IDD = IDD_ADDRULE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
public:
	CString m_sNewRuleName;
	CString GetStr(CString defaultstr);
};

#endif // !defined(AFX_DADDRULE)