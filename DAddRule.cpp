// DAddRule.cpp : implementation file
//

#include "stdafx.h"
#include "Mighty.h"
#include "DAddRule.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// DAddRule dialog

IMPLEMENT_DYNAMIC(DAddRule, CDialog)

DAddRule::DAddRule(CWnd* pParent /*=NULL*/)
	: CDialog(DAddRule::IDD, pParent)
{

	EnableAutomation();

	m_sNewRuleName = _T("");
}

DAddRule::~DAddRule()
{
}

void DAddRule::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CDialog::OnFinalRelease();
}

void DAddRule::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NEWRULENAME, m_sNewRuleName);
	DDV_MaxChars(pDX, m_sNewRuleName, 16);
}


BEGIN_MESSAGE_MAP(DAddRule, CDialog)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(DAddRule, CDialog)
END_DISPATCH_MAP()

// Note: we add support for IID_IDAddRule to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .IDL file.

// {657260E6-9357-460E-8534-F0B6BAD345F0}
static const IID IID_IDAddRule =
{ 0x657260E6, 0x9357, 0x460E, { 0x85, 0x34, 0xF0, 0xB6, 0xBA, 0xD3, 0x45, 0xF0 } };

BEGIN_INTERFACE_MAP(DAddRule, CDialog)
	INTERFACE_PART(DAddRule, IID_IDAddRule, Dispatch)
END_INTERFACE_MAP()


// DAddRule message handlers


CString DAddRule::GetStr(CString defaultstr)
{
	m_sNewRuleName = defaultstr;
	if ( this->DoModal() == IDOK )
		return m_sNewRuleName;
	else return "";
}
