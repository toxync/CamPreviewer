// CamPreviewerDlg.h: 헤더 파일
#pragma once
#include "TXYC_PreviewBuilder.h"

// 윈도우에게 허용된 최소 폭(확대 배율 100% 기준)
#define MIN_CX 680
// 윈도우에게 허용된 최소 높이(확대 배율 100% 기준)
#define MIN_CY 400
// 미리 보기 영상의 영상 처리 작업 타이머 ID
#define TRACE_TIMER_ID 1
// 패턴 추적 작업에서 Edge가 맞는지 확인하기 위해 사용할 값
#define EDGE_DTCN 80
// 누적 패턴 추적 작업에서 Edge로 판정된 픽셀의 색상 값에 더할 가중치
#define EDGE_INC_VAL 25
// RGB 색상값에서 R 값만 추출하는 연산
#define R_RGB(rgb) (UINT8)(rgb >> 16)
// RGB 색상값에서 G 값만 추출하는 연산
#define G_RGB(rgb) (UINT8)(rgb >> 8)
// RGB 색상값에서 B 값만 추출하는 연산
#define B_RGB(rgb) (UINT8)rgb

// CCamPreviewerDlg 대화 상자
class CCamPreviewerDlg : public CDialog
{
private:
	TXYC_PreviewBuilder* mp_cam_preview = NULL;
	CListBox m_cam_list; // 사용가능한 영상 장치들의 목록을 저장한 리스트 박스
	CDC m_mem_dc; // 메모리 DC로 사용할 이미지 객체의 DC를 관리하기 위한 객체
	CDC m_check_dc; // 패턴 누적 추적에 사용할 이미지 객체의 DC르 관리하기 위한 객체
	CImage m_mem_image; // 메모리 DC로 사용할 이미지 객체
	CImage m_check_image; // 패턴 누적 추적에 사용할 이미지 객체
	RECT m_preview_rect = { 0 }; // 영상 미리 보기 윈도우의 영역
	RECT m_result_rect = { 0 }; // 영상 처리 결과 출력 영역
	UINT m_dpi = 0; // 확대 배율을 계산하기 위해 사용할 현재 윈도우의 DPI 값
	UINT m_image_pattern_size = 0; // 메모리 DC로 사용할 이미지 객체의 비트맵 패턴 크기
	UINT8* mp_image_pattern = NULL; // 메모리 DC로 사용할 이미지 객체의 비트맵 시작 주소
	UINT8* mp_check_pattern = NULL; // 패턴 누적 추적에 사용할 이미지 객체의 비트맵 시작 주소
	// 확대 배율이 적용된 윈도우의 최소 폭과 높이
	int m_min_cx = 0, m_min_cy = 0;
	// 클라이언트 영역 테두리와 컨트롤 가이드 영역 사이의 간격
	int m_guide_spacing = 0;
	// 영상 미리 보기 윈도우 영역과 영상 처리 결과 출력 영역 사이의 간격
	int m_rect_spacing = 0;
	// 영상 미리 보기 윈도우 영역과 영상 처리 결과 출력 영역의 상단(top) 좌표
	int m_rect_top = 0;
	// 영상 미리 보기 실행 상태(0: 영상 미리 보기 중지됨, 1: 영상 미리 보기 실행중)
	char m_preview_flag = 0;
	// 윈도우의 크기 변경 상태(0: 윈도우 크기 변경 중지됨, 1: 윈도우 크기 변경 진행중)
	char m_resize_flag = 0;
	// 미리 보기 영상의 영상 처리 작업 수행 여부(0: 영상 처리 수행 안 함, 1: 영상 처리 수행)
	char m_trace_flag = 0;
	// 미리 보기 영상에서 추적할 대상(0: 색상 추적, 1: 패턴 추적)
	char m_trace_target = 0;
	// 미리 보기 영상에서 추적할 색상(0: 빨간색, 1: 초록색, 2: 파란색)
	char m_trace_color = 0;
	// 미리 보기 영상에서 추적할 패턴 방식(0: 일반 패턴 추적, 1: 누적 패턴 추적)
	char m_trace_pattern = 0;
// 생성입니다.
public:
	CCamPreviewerDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.
	// 사용 가능한 영상 장치들의 목록을 생성하는 함수
	void MakeDeviceList();
	// 미리 보기 영상에서 색상을 추적하는 함수
	void TraceColorFromPreview(HDC ah_dc);
	// 미리 보기 영상에서 패턴을 추적하는 함수
	void TracePatternFromPreview(HDC ah_dc);
// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CAMPREVIEWER_DIALOG };
#endif
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.
// 구현입니다.
protected:
	HICON m_hIcon;
	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEnterSizeMove();
	afx_msg void OnExitSizeMove();
	afx_msg void OnSwitchTraceTarget(UINT a_ctrl_id);
	afx_msg void OnSwitchTraceColor(UINT a_ctrl_id);
	afx_msg void OnSwitchTracePattern(UINT a_ctrl_id);
	afx_msg void OnBnClickedCamTriggerBtn();
	afx_msg void OnBnClickedTraceTriggerBtn();
	afx_msg void OnBnClickedBscSetupBtn();
	afx_msg void OnBnClickedAdvSetupBtn();
};
