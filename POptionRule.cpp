// POptionRule.cpp : implementation file
//

#include "stdafx.h"
#include "Mighty.h"
#include "POptionRule.h"

#include "DRule.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// POptionRule property page

IMPLEMENT_DYNCREATE(POptionRule, CPropertyPage)

POptionRule::POptionRule() : CPropertyPage(POptionRule::IDD)
{
	//{{AFX_DATA_INIT(POptionRule)
	m_sRuleString = Mo()->rule.Encode();
	m_nRulePreset = Mo()->nPreset;
	m_sCustom = Mo()->sCustom;
	//}}AFX_DATA_INIT
}

POptionRule::~POptionRule()
{
}

void POptionRule::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(POptionRule)
	DDX_Text(pDX, IDC_RULESTRING, m_sRuleString);
	DDX_CBIndex(pDX, IDC_RULEPRESET, m_nRulePreset);
	DDX_Control(pDX, IDC_ADDRULE, m_btAddRule);
	DDX_Control(pDX, IDC_REMOVERULE, m_btRemoveRule);
	//}}AFX_DATA_MAP
	
	m_btRemoveRule.EnableWindow(m_nRulePreset > BASIC_PRESET);
}


BEGIN_MESSAGE_MAP(POptionRule, CPropertyPage)
	//{{AFX_MSG_MAP(POptionRule)
	ON_BN_CLICKED(IDC_SHOWRULE, OnShowrule)
	ON_CBN_SELCHANGE(IDC_RULEPRESET, OnSelchangeRulepreset)
	ON_BN_CLICKED(IDC_ADDRULE, &POptionRule::OnClickedAddrule)
	ON_BN_CLICKED(IDC_REMOVERULE, &POptionRule::OnClickedRemoverule)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// POptionRule message handlers

BOOL POptionRule::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	CRule r;
	LPCTSTR s = _T("사용자정의");
	CComboBox* pBox = (CComboBox*)GetDlgItem(IDC_RULEPRESET);
	int i = 1;
	do {
		pBox->AddString( s );
		s = r.Preset(i++);
	} while ( s );

	pBox->SetCurSel( m_nRulePreset );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void POptionRule::OnShowrule() 
{
	// TODO: Add your control notification handler code here
	DRule dlg(m_sRuleString);
	if ( dlg.DoModal() == IDOK ) {

		CString sRule = dlg.m_rule.Encode();
		if ( sRule != m_sRuleString ) {
			// 설정이 바뀌었다면 사용자 정의로 가정한다
			m_nRulePreset = 0;
			m_sCustom = m_sRuleString = sRule;
			UpdateData(FALSE);
		}
	}
}

void POptionRule::OnSelchangeRulepreset() 
{
	
	// TODO: Add your control notification handler code here
	UpdateData( TRUE );

	if ( m_nRulePreset > 0 ) {

		m_btRemoveRule.EnableWindow(m_nRulePreset > BASIC_PRESET);

		CRule rule; rule.Preset( m_nRulePreset );
		m_sRuleString = rule.Encode();
	}
	else {
		m_sRuleString = m_sCustom;
	}

	UpdateData( FALSE );
}

void POptionRule::OnOK() 
{
	// TODO: Add your specialized code here and/or call the base class

	Mo()->sCustom = m_sCustom;
	Mo()->rule.Decode( m_sRuleString );
	Mo()->nPreset = m_nRulePreset;

	CPropertyPage::OnOK();
}


void POptionRule::OnClickedAddrule()
{
	// TODO: Add your control notification handler code here
}


void POptionRule::OnClickedRemoverule()
{
	// TODO: Add your control notification handler code here
}