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
	DDX_Control(pDX, IDC_RULEPRESET, m_cbRulePreset);
	//}}AFX_DATA_MAP

	m_btRemoveRule.EnableWindow(m_nRulePreset > BASIC_PRESETS);
}


BEGIN_MESSAGE_MAP(POptionRule, CPropertyPage)
	//{{AFX_MSG_MAP(POptionRule)
	ON_BN_CLICKED(IDC_SHOWRULE, OnShowrule)
	ON_CBN_SELCHANGE(IDC_RULEPRESET, OnSelchangeRulepreset)
	ON_BN_CLICKED(IDC_ADDRULE, OnClickedAddrule)
	ON_BN_CLICKED(IDC_REMOVERULE, OnClickedRemoverule)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// POptionRule message handlers

BOOL POptionRule::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	CRule r;
	LPCTSTR s = _T("사용자정의");
	int i = 1;
	do {
		m_cbRulePreset.AddString( s );
		s = r.Preset(i++);
	} while ( s );

		m_cbRulePreset.SetCurSel( m_nRulePreset );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void POptionRule::OnShowrule() 
{
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
	
	UpdateData( TRUE );

	if ( m_nRulePreset > 0 ) {

		m_btRemoveRule.EnableWindow(m_nRulePreset > BASIC_PRESETS);

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
	Mo()->sCustom = m_sCustom;
	Mo()->rule.Decode( m_sRuleString );
	Mo()->nPreset = m_nRulePreset;

	CPropertyPage::OnOK();
}


void POptionRule::OnClickedAddrule()
{
	CString rulename = CRule::AttemptSaveRule( m_sRuleString, "규칙이름" );
	if ( rulename != "" ) {
		m_cbRulePreset.AddString( rulename );
		m_cbRulePreset.SetCurSel( m_cbRulePreset.GetCount() - 1 );
		UpdateData();
		m_btRemoveRule.EnableWindow( m_nRulePreset > BASIC_PRESETS );
	}
}

void POptionRule::OnClickedRemoverule()
{
	CString msgboxstr = "\'";
	CString name = CRule::GetName ( m_nRulePreset - 1 );
	name.TrimLeft(); name.TrimRight();
	msgboxstr += name;
	msgboxstr += "\' 규칙을 삭제하시겠습니까?";
	if ( AfxMessageBox( msgboxstr,
					MB_YESNO | MB_ICONQUESTION )
		== IDYES ) {
		CRule::RemovePreset( m_nRulePreset - BASIC_PRESETS - 1 );
		m_cbRulePreset.DeleteString( m_nRulePreset );
		m_cbRulePreset.SetCurSel( m_nRulePreset - 1 );
		UpdateData();
		m_btRemoveRule.EnableWindow( m_nRulePreset > BASIC_PRESETS );
	}
}