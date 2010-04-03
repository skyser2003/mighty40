// Msg.cpp: implementation of the CMsg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mighty.h"
#include "Msg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


void CMsg::Init()
{
	m_nBufferLength = 64;
	m_pBuffer = new char[64];
	m_rptr = m_pBuffer + sizeof(long);
	m_wptr = m_pBuffer + sizeof(long);
	m_pLen = (long*)m_pBuffer;
	*m_pLen = sizeof(long);

	ASSERT( this != (CMsg*)0x00B32DE0 );
}

// 메시지를 생성할 때 printf 처럼 포맷 스트링으로 빠르게
// 메시지를 만든다  예: CMsg( _T("lls"), 10, 10, _T("haha") )
// 포맷 스트링이 0 이면 빈 메시지를 생성한다
// l : long          s : LPCTSTR
// S : CString*      C : CCardList*
// k : checksum (인자 없음)
CMsg::CMsg( LPCTSTR sFormat, ... )
{
	Init();
	if ( !sFormat ) return;

	va_list va;
	va_start( va, sFormat );
	for ( const TCHAR* p = sFormat; *p; p++ )
		switch (*p) {
		case _T('l'): PushLong(va_arg(va,long)); break;
		case _T('s'): PushString(va_arg(va,LPCTSTR)); break;
		case _T('S'): PushString(*va_arg(va,CString*)); break;
		case _T('C'): PushCardList(*va_arg(va,CCardList*)); break;
		case _T('k'): PushChecksum(); break;
		default: ASSERT(0); break;
	}
}

CMsg& CMsg::operator=( const CMsg& msg )
{
	memcpy( LockBuffer( msg.GetLength() ), msg.GetBuffer(), msg.GetLength() );
	m_rptr = msg.m_rptr;
	ReleaseBuffer();
	return *this;
}

CMsg::~CMsg()
{
	delete[] m_pBuffer;
}

// 메시지 버퍼의 길이를 설정하고 버퍼의 주소를 리턴한다
void* CMsg::LockBuffer( int nLen )
{
	ASSERT( nLen >= sizeof(long) );
	EnsureBufferLength( nLen );
	*m_pLen = nLen;
	m_wptr = m_pBuffer+nLen;
	return (void*)m_pBuffer;
}

void CMsg::ReleaseBuffer()
{
}

// 제대로 된 메시지인가 ( 맨 끝의 체크섬을 검사한다 )
bool CMsg::IsValid() const
{
	long cs1 = *(long*)(m_pBuffer+GetLength()-sizeof(long));
	long cs2 = CalcChecksum( m_pBuffer+sizeof(long), GetLength()-sizeof(long)*2);
	return cs1 == cs2;
}

// 체크섬을 맨 끝에 push 한다
void CMsg::PushChecksum()
{
	PushLong( CalcChecksum( m_pBuffer+sizeof(long), GetLength()-sizeof(long) ) );
}

// 정수, 문자열, 카드 리스트를 각각 push 한다
void CMsg::PushLong( long n )
{
	EnsureBufferLength( GetLength() + sizeof(long) );
	*((long*)m_wptr) = n;
	m_wptr += sizeof(long);
	*m_pLen += sizeof(long);
}

void CMsg::PushString( LPCTSTR s )
{
#ifdef _UNICODE
	// 유니코드 사용

	size_t nNeeded = _wcstombsz( 0, s, 0 );

	if ( nNeeded == (size_t)-1 ) {	// error
		ASSERT(0);
		PushString( _T("") );
		return;
	}

	char* buf = new char[nNeeded+1];
	_wcstombsz( buf, s, nNeeded+1 );

	int len = strlen( buf ) + 1;
	EnsureBufferLength( GetLength() + len );
	_mbscpy( m_wptr, buf );
	m_wptr += len;
	*m_pLen += len;

	delete[] buf;
#else
	// 멀티 바이트 사용
	int len = strlen( (const char*)s ) + 1;
	EnsureBufferLength( GetLength() + len );
	strcpy( (char*)m_wptr, (const char*)s );
	m_wptr += len;
	*m_pLen += len;
#endif
}

void CMsg::PushCardList( const CCardList& lc )
{
	PushLong( lc.GetCount() );
	POSITION pos = lc.GetHeadPosition();
	while (pos)
		PushLong( (long)(int)lc.GetNext(pos) );
}

// 정수, 문자열, 카드 리스트를 각각 Pump 한다
// 실패시 false 리턴
bool CMsg::PumpLong( long& l )
{
	if ( m_rptr + sizeof(long) > (const char*)m_wptr )
		return false;
	l = *(long*)m_rptr;
	m_rptr += sizeof(long);
	return true;
}

bool CMsg::PumpString( CString& s )
{
	const char* p;
	for ( p = m_rptr;
			*p && p < m_wptr; p++ );
	if ( *p ) return false;
	s = m_rptr;	// 필요하면 CString 이 유니코드로 자동 변환한다
	m_rptr = p + 1;
	return true;
}

bool CMsg::PumpCardList( CCardList& lc )
{
	const char* p = m_rptr;	// save
	long nCount;
	if ( !PumpLong( nCount ) ) return false;

	lc.RemoveAll();

	int i;
	for ( i = 0; i < nCount; i++ ) {
		long l;
		if ( !PumpLong(l) ) break;
		lc.AddTail( CCard(l) );
	}
	if ( i < nCount ) {	// fail
		m_rptr = p;
		return false;
	}
	else return true;
}

// 버퍼가 nMinLength 보다 작으면 버퍼를 확장한다
void CMsg::EnsureBufferLength( int nMinLength )
{
	if ( m_nBufferLength < nMinLength ) {

		int nWPtr = m_wptr - m_pBuffer;	// m_wptr 을 상대값으로 보존한다
		int nRPtr = m_rptr - m_pBuffer;

		// 64 바이트씩 증가시킨다
		int nIncrease = ( nMinLength - m_nBufferLength - 1 ) / 64 * 64 + 64;
		m_nBufferLength += nIncrease;
		ASSERT( m_nBufferLength >= nMinLength );
		m_pBuffer = (char*)realloc( m_pBuffer, m_nBufferLength );

		ASSERT( m_pBuffer );

		// m_ptr 을 복구
		m_wptr = m_pBuffer + nWPtr;
		m_rptr = m_pBuffer + nRPtr;

		m_pLen = (long*)m_pBuffer;
	}
}

// 지정된 메모리 영역의 체크섬을 계산한다
long CMsg::CalcChecksum( const char* p, int len )
{
	long cs = 0x55555555;
	for ( int i = 0; i < len; i++ )
		cs += (long)*(p+i);
	return cs;
}
