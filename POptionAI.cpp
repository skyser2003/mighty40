// POptionAI.cpp : implementation file
//

#include "stdafx.h"
#include "mighty.h"
#include "POptionAI.h"

#include "Play.h"
#include "Player.h"
#include "PlayerMai.h"
#include "MaiBSW.h"
#include "DMaiInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// POptionAI property page

IMPLEMENT_DYNCREATE(POptionAI, CPropertyPage)

POptionAI::POptionAI() : CPropertyPage(POptionAI::IDD)
{
	//{{AFX_DATA_INIT(POptionAI)
	//}}AFX_DATA_INIT
	for ( int i = 0; i < MAX_PLAYERS-1; i++ ) {
		m_asPlayerName[i] = Mo()->aPlayer[i+1].sName;
		m_asSetting[i] = Mo()->aPlayer[i+1].sSetting;
		m_asPath[i] = Mo()->aPlayer[i+1].sAIDLL;
	}
}

POptionAI::~POptionAI()
{
}

void POptionAI::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(POptionAI)
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_PLAYERNAME0, m_asPlayerName[0]);
	DDX_Text(pDX, IDC_PLAYERNAME1, m_asPlayerName[1]);
	DDX_Text(pDX, IDC_PLAYERNAME2, m_asPlayerName[2]);
	DDX_Text(pDX, IDC_PLAYERNAME3, m_asPlayerName[3]);
	DDX_Text(pDX, IDC_PLAYERNAME4, m_asPlayerName[4]);

	if ( pDX->m_bSaveAndValidate )
		for ( int i = 0; i < MAX_PLAYERS-1; i++ ) {
			Mo()->aPlayer[i+1].sName = m_asPlayerName[i];
			Mo()->aPlayer[i+1].sSetting = m_asSetting[i];
			Mo()->aPlayer[i+1].sAIDLL = m_asPath[i];
		}
	else for ( int i = 0; i < MAX_PLAYERS-1; i++ ) {
		int nIndex = m_asPath[i].ReverseFind(_T('\\'));
		if ( nIndex < 0 ) m_asFileName[i] = m_asPath[i];
		else m_asFileName[i] = m_asPath[i].Mid( nIndex+1 );
		if ( m_asFileName[i].IsEmpty() )
			m_asFileName[i] = _T("<기본 AI>");
	}

	DDX_Text(pDX, IDC_AIPATH0, m_asFileName[0]);
	DDX_Text(pDX, IDC_AIPATH1, m_asFileName[1]);
	DDX_Text(pDX, IDC_AIPATH2, m_asFileName[2]);
	DDX_Text(pDX, IDC_AIPATH3, m_asFileName[3]);
	DDX_Text(pDX, IDC_AIPATH4, m_asFileName[4]);
}


BEGIN_MESSAGE_MAP(POptionAI, CPropertyPage)
	//{{AFX_MSG_MAP(POptionAI)
	ON_BN_CLICKED(IDC_RESETPATH, OnResetpath)
	//}}AFX_MSG_MAP
	ON_CONTROL_RANGE( BN_CLICKED, IDC_AIBROWSE0, IDC_AIBROWSE4, OnBrowse )
	ON_CONTROL_RANGE( BN_CLICKED, IDC_AISETTING0, IDC_AISETTING4, OnSetting )
END_MESSAGE_MAP()


bool POptionAI::GetDllInfo( LPCTSTR sPath, MAIDLL_INFO* pInfo )
{
	HINSTANCE h;
	pfMaiGetInfo pmgi;
	
	if ( !sPath || !*sPath ) {	// default
		h = 0;
		pmgi = CMaiBSW::MaiGetInfo;
	}
	else {
		h = ::LoadLibrary( sPath );
		if ( !h ) return false;
		pmgi = (pfMaiGetInfo)::GetProcAddress( h, "MaiGetInfo" );
	}

	if ( pmgi ) pmgi( pInfo );

	if ( h ) { VERIFY( ::FreeLibrary(h) ); }

	return pmgi ? true : false;
}

/////////////////////////////////////////////////////////////////////////////
// POptionAI message handlers

void POptionAI::OnSetting( UINT nID )
{
	int nPlayer = nID - IDC_AISETTING0;

	ShowInfo( nPlayer, m_asPath[nPlayer], m_asFileName[nPlayer] );
}

void POptionAI::OnBrowse( UINT nID )
{
	int nPlayer = nID - IDC_AIBROWSE0;

	CFileDialog dlg( TRUE,	// open
		_T(".dll"), NULL,
		OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NONETWORKBUTTON,
		_T("Mighty AI 파일 (*.dll)|*.dll|모든파일 (*.*)|*.*||"),
		this );

	// 실행파일이 있는 디렉토리를 연다
	TCHAR filename[MAX_PATH];
	if ( ::GetModuleFileName( NULL, filename, sizeof(filename) ) ) {

		TCHAR* pSlash = _tcsrchr( filename, _T('\\') );
		if ( pSlash ) {
			*pSlash = _T('\0');
			dlg.m_ofn.lpstrInitialDir = filename;
		}
	}

	if ( dlg.DoModal() == IDOK )
		ShowInfo( nPlayer, dlg.GetPathName(), dlg.GetFileName() );
}

void POptionAI::OnResetpath() 
{
	// TODO: Add your control notification handler code here
	for ( int i = 0; i < MAX_PLAYERS-1; i++ ) {
		m_asPath[i].Empty();
		m_asSetting[i].Empty();
	}
	UpdateData(FALSE);
}

void POptionAI::ShowInfo( int nPlayer, LPCTSTR sPath, LPCTSTR sFile )
{
	MAIDLL_INFO mi;
	if ( !GetDllInfo( sPath, &mi ) ) {
		AfxMessageBox( _T("지정한 파일은 Mighty AI DLL 이 아닙니다"), MB_OK|MB_ICONEXCLAMATION );
		return;
	}
	else {
		CString sSetting;
		if ( m_asPath[nPlayer].CompareNoCase( sPath ) )
			sSetting = _T("");
		else sSetting = m_asSetting[nPlayer];

		DMaiInfo info( &mi, sFile, sPath, sSetting );
		if ( info.DoModal() == IDOK )
			m_asSetting[nPlayer] = sSetting;

		if ( m_asPath[nPlayer].CompareNoCase( sPath ) ) {
			// 새로운 DLL 을 로드했음 (세팅을 초기화)
			m_asPath[nPlayer] = sPath;
			m_asFileName[nPlayer] = sFile;
			m_asSetting[nPlayer] = sSetting;
			UpdateData(FALSE);
		}
	}
}
