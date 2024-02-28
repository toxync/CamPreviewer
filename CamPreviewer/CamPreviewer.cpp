// CamPreviewer.cpp: 애플리케이션에 대한 클래스 동작을 정의합니다.
#include "pch.h"
#include "framework.h"
#include "CamPreviewer.h"
#include "CamPreviewerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CCamPreviewerApp
BEGIN_MESSAGE_MAP(CCamPreviewerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// CCamPreviewerApp 생성
CCamPreviewerApp::CCamPreviewerApp()
{
	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}

// 유일한 CCamPreviewerApp 개체입니다.
CCamPreviewerApp theApp;

// CCamPreviewerApp 초기화
BOOL CCamPreviewerApp::InitInstance()
{
	// 애플리케이션 매니페스트가 ComCtl32.dll 버전 6 이상을 사용하여 비주얼 스타일을
	// 사용하도록 지정하는 경우, Windows XP 상에서 반드시 InitCommonControlsEx()가 필요합니다.
	// InitCommonControlsEx()를 사용하지 않으면 창을 만들 수 없습니다.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 응용 프로그램에서 사용할 모든 공용 컨트롤 클래스를 포함하도록
	// 이 항목을 설정하십시오.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	CCamPreviewerDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();
	
	return FALSE;
}
