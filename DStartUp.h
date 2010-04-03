// DStartUp.h: interface for the DStartUp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DSTARTUP_H__4643EA36_DCDA_11D2_9840_0000212035B8__INCLUDED_)
#define AFX_DSTARTUP_H__4643EA36_DCDA_11D2_9840_0000212035B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMFSM;
struct COption;


class DStartUp : public DSB  
{
public:
	DStartUp( CBoardWrap* pBoard ) : DSB(pBoard) {}
	virtual ~DStartUp() {}

	// nMode  0 : 초기 메뉴  1 : 멀티플레이어 메뉴
	void Create( int nMode );

public:
	// 현재 옵션에 기초해서 1인용 마이티 MFSM 을 생성(new)한다
	// 성공하면 Board 에 이 MFSM 을 세트
	// AI 파일 로드 실패 시 에러 DSB 를 띄운다 (게임은 진행됨)
	bool CreateOnePlayerGame();

protected:
	// 초기화 ( 생성된 후 호출됨 )
	virtual void OnInit();

	// CBoard 가 호출하는 그리기 함수 - 계승된 shade-box 는 이 함수를
	// 오버라이드 하여 구현해야 한다
	// 베이스 함수는 윈도우 전체에 가득 차는 음영을 그린다
	virtual void OnDraw(
		CDC* pDC,
		bool bBrushOrigin );

	// 핫 스팟을 클릭할 때 불리는 함수
	// LPVOID 값이 0 이면 종료 (X자를 클릭) 요청
	virtual void OnClick( LPVOID pVoid );

	// 핫 스팟이 Highlight 되거나 Normal 로 될 때 호출되는 함수
	// bHighlight 가 참이면 highlight 되었음
	virtual void OnHighlight( HOTSPOT& , bool /*bHighlight*/ );

	// 소리
	virtual void OnClickSound();

protected:
	// 현재 모드
	int m_nMode;
};

#endif // !defined(AFX_DSTARTUP_H__4643EA36_DCDA_11D2_9840_0000212035B8__INCLUDED_)
