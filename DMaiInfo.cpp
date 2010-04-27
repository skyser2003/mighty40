// DMaiInfo.cpp : implementation file
//

#include "stdafx.h"
#include "Mighty.h"
#include "DMaiInfo.h"

#include "Play.h"
#include "Player.h"
#include "PlayerMai.h"
#include "MaiBSW.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DMaiInfo dialog


DMaiInfo::DMaiInfo( const MAIDLL_INFO* pInfo, LPCTSTR sFileName,
	LPCTSTR sPathName, CString& sSetting,
	CWnd* pParent /*=NULL*/)
	: m_sSetting(sSetting), CDialog(DMaiInfo::IDD, pParent)
{
	//{{AFX_DATA_INIT(DMaiInfo)
	m_sExplain = pInfo->sExplain;
	m_sFileName = sFileName;
	m_sName = pInfo->sName;
	//}}AFX_DATA_INIT
	m_sPathName = sPathName;
}


void DMaiInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DMaiInfo)
	DDX_Text(pDX, IDC_MAIINFO_EXPLAIN, m_sExplain);
	DDX_Text(pDX, IDC_MAIINFO_FILENAME, m_sFileName);
	DDX_Text(pDX, IDC_MAIINFO_NAME, m_sName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DMaiInfo, CDialog)
	//{{AFX_MSG_MAP(DMaiInfo)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
	ON_CONTROL( BN_CLICKED, IDC_CHANGEOPTION, OnChangeoption )
END_MESSAGE_MAP()


bool DMaiInfo::AdjustSetting()
{
	HINSTANCE h;
	pfMaiGetPlay pmgp;
	pfMaiOption pmo;
	pfMaiReleasePlay pmrp;

	if ( m_sPathName.IsEmpty() ) {	// default
		h = 0;
		pmgp = CMaiBSW::MaiGetPlay;
		pmo = CMaiBSW::MaiOption;
		pmrp = CMaiBSW::MaiReleasePlay;
	}
	else {

		h = ::LoadLibrary( m_sPathName );
		if ( !h ) return false;

		pmgp = (pfMaiGetPlay)::GetProcAddress( h, "MaiGetPlay" );
		pmo = (pfMaiOption)::GetProcAddress( h, "MaiOption" );
		pmrp = (pfMaiReleasePlay)::GetProcAddress( h, "MaiReleasePlay" );
	}

	if ( !pmgp || !pmrp ) {
		if ( h ) { VERIFY( ::FreeLibrary(h) ); }
		return false;
	}

	if ( !pmo )
		AfxMessageBox( _T("이 DLL 에는 설정할 옵션이 없습니다"),
			MB_OK|MB_ICONINFORMATION );
	else {
		CPlay* pPlay = pmgp( m_sSetting, 0 );
		if ( pPlay ) {
			pmo( pPlay, GetSafeHwnd() );
			pmrp( pPlay, &m_sSetting );
		}
	}

	if ( h ) { VERIFY( ::FreeLibrary(h) ); }
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// DMaiInfo message handlers

void DMaiInfo::OnChangeoption()
{
	if ( !AdjustSetting() )
		AfxMessageBox( _T("잘못된 AI DLL 이 지정되어 있습니다\n다시 확인해 보세요"),
			MB_OK|MB_ICONEXCLAMATION );
}
