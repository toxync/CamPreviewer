// CamPreviewerDlg.cpp: 구현 파일
#include "pch.h"
#include "framework.h"
#include "CamPreviewer.h"
#include "CamPreviewerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CCamPreviewerDlg 대화 상자
CCamPreviewerDlg::CCamPreviewerDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_CAMPREVIEWER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCamPreviewerDlg::MakeDeviceList()
{
	ICreateDevEnum* p_create_dev_enum = NULL;
	IMoniker* p_moniker = NULL;
	IEnumMoniker* p_enum_mnkr = NULL;
	IPropertyBag* p_property_bag = NULL;
	ULONG fetch_cnt = 0;
	VARIANT var = { 0 };
	int index = 0;
	var.vt = VT_BSTR;
	// 시스템에 연결된 장치 정보를 얻는 COM 객체 생성
	HRESULT h_res = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void**)&p_create_dev_enum);
	if (!SUCCEEDED(h_res))
	{
		return;
	}
	// 영상 장치와 관련된 장치 정보를 구성하는 인터페이스를 얻어냄
	h_res = p_create_dev_enum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &p_enum_mnkr, 0);
	if (!SUCCEEDED(h_res))
	{
		return;
	}
	p_enum_mnkr->Reset();
	// 영상 장치 목록을 탐색함
	while (h_res = p_enum_mnkr->Next(1, &p_moniker, &fetch_cnt), h_res == S_OK)
	{
		h_res = p_moniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&p_property_bag);
		if (SUCCEEDED(h_res))
		{
			// 영상 장치의 이름을 얻어냄
			h_res = p_property_bag->Read(L"FriendlyName", &var, NULL);
			if (h_res == NOERROR)
			{
				// 정상적으로 영상 장치 이름을 얻었다면 얻어낸 장치 이름을 리스트 박스에 추가하고
				// 해당 장치를 사용하기 위한 인터페이스를 추가한 리스트 박스 항목에 저장함
				index = m_cam_list.AddString(var.bstrVal);
				m_cam_list.SetItemDataPtr(index, p_moniker);
				SysFreeString(var.bstrVal);
				p_moniker->AddRef();
			}
			p_property_bag->Release();
		}
	}
	p_enum_mnkr->Release();
	// 첫 번째 영상 장치를 기본으로 선택함
	if (m_cam_list.GetCount())
	{
		m_cam_list.SetCurSel(0);
	}
	p_create_dev_enum->Release();
}

void CCamPreviewerDlg::TraceColorFromPreview(HDC ah_dc)
{
	UINT8* p_pos = mp_image_pattern, * p_end_pattern = mp_image_pattern + m_image_pattern_size;
	UINT8 r = 0, g = 0, b = 0;
	while (p_pos < p_end_pattern)
	{
		r = *(p_pos + 2);
		g = *(p_pos + 1);
		b = *p_pos;
		// 현재 픽셀의 색상이 선택된 추적 색상인지 확인함
		switch (m_trace_color)
		{
		case 0: // 빨간색 추적
			// R 값이 100보다 크고 G, B 값이 R 값의 70% 이하라면 현재 픽셀을 빨간색으로 판정
			if ((r > 100) && (7 * r > 10 * g) && (7 * r > 10 * b))
			{
				*((UINT*)p_pos) = 0xFFFF0000;
			}
			else
			{
				*((UINT*)p_pos) = 0xFF000000;
			}
			break;
		case 1: // 파란색 추적
			// G 값이 60보다 크고 R, B 값이 G 값의 85% 이하라면 현재 픽셀을 초록색으로 판정
			if ((g > 60) && (17 * g > 20 * r) && (17 * g > 20 * b))
			{
				*((UINT*)p_pos) = 0xFF00FF00;
			}
			else
			{
				*((UINT*)p_pos) = 0xFF000000;
			}
			break;
		case 2: // 초록색 추적
			// B 값이 100보다 크고 R, G 값이 B 값의 75% 이하라면 현재 픽셀을 파란색으로 판정
			if ((b > 100) && (3 * b > 4 * r) && (3 * b > 4 * g))
			{
				*((UINT*)p_pos) = 0xFF0000FF;
			}
			else
			{
				*((UINT*)p_pos) = 0xFF000000;
			}
			break;
		}
		// 현재 픽셀의 확인이 끝났으면 다음 픽셀로 이동함
		p_pos += 4;
	}
	// 추적할 색상만 남긴 이미지를 출력함
	m_mem_image.Draw(ah_dc, m_result_rect);
}

void CCamPreviewerDlg::TracePatternFromPreview(HDC ah_dc)
{
	UINT* p_check_pos = (UINT*)mp_check_pattern, temp = 0;
	UINT8* p_pos = mp_image_pattern, * p_end_pattern = mp_image_pattern + m_image_pattern_size;
	UINT8 r = 0, g = 0, b = 0, old_r = 0, old_g = 0, old_b = 0, diff_r = 0, diff_g = 0, diff_b = 0;
	// 첫 번째 픽셀의 색상값을 백업함
	old_r = *p_pos++;
	old_g = *p_pos++;
	old_b = *p_pos++;
	p_pos++;
	while (p_pos < p_end_pattern)
	{
		r = *(p_pos + 2);
		g = *(p_pos + 1);
		b = *p_pos;
		diff_r = r - old_r;
		diff_g = g - old_g;
		diff_b = b - old_b;
		// 현재 픽셀과 이전 픽셀의 R, G, B 값 모두 80 이상 차이가 난다면 현재 픽셀을 Edge로 판단함
		if ((diff_r > EDGE_DTCN) && (diff_g > EDGE_DTCN) && (diff_b > EDGE_DTCN))
		{
			switch (m_trace_pattern)
			{
			case 0: // 일반 패턴 추적
				*((UINT*)p_pos) = 0xFFFFFFFF;
				break;
			case 1: // 누적 패턴 추적
				// 누적 패턴 이미지의 현재 픽셀 평균 밝기를 구함
				temp = (R_RGB(*p_check_pos) + G_RGB(*p_check_pos) + B_RGB(*p_check_pos)) / 3;
				// 누적 패턴 이미지의 현재 픽셀 평균 밝기가 최대값인지 확인함
				if (temp < (256 - EDGE_INC_VAL))
				{
					// 누적 패턴 이미지의 현재 픽셀 평균 밝기를 가중치만큼 증가시킴
					temp += EDGE_INC_VAL;
					// 변경된 누적 패턴 이미지의 현재 픽셀 평균 밝기를 적용시킴
					*p_check_pos = 0xFF000000 | (temp << 16) | (temp << 8) | temp;
				}
				break;
			}
		}
		else
		{
			// 현재 픽셀이 Edge가 아니라면 검은색으로 설정함
			*p_check_pos = *((UINT*)p_pos) = 0xFF000000;
		}
		// 현재 픽셀은 다음 픽셀의 이전 픽셀이 되기 때문에 현재 픽셀의 색상값을 백업함
		old_r = r;
		old_g = g;
		old_b = b;
		// 현재 픽셀의 확인이 끝났으면 다음 픽셀로 이동함
		++p_check_pos;
		p_pos += 4;
	}
	// Edge만 남긴 이미지를 출력함
	switch (m_trace_pattern)
	{
	case 0: // 일반 패턴 추적 결과 이미지 출력
		m_mem_image.Draw(ah_dc, m_result_rect);
		break;
	case 1: // 누적 패턴 추적 결과 이미지 출력
		m_check_image.Draw(ah_dc, m_result_rect);
		break;
	}
}

void CCamPreviewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CAM_LIST, m_cam_list);
}

BEGIN_MESSAGE_MAP(CCamPreviewerDlg, CDialog)
	ON_COMMAND_RANGE(IDC_COLOR_RADIO, IDC_PTRN_RADIO, &CCamPreviewerDlg::OnSwitchTraceTarget)
	ON_COMMAND_RANGE(IDC_R_RADIO, IDC_B_RADIO, &CCamPreviewerDlg::OnSwitchTraceColor)
	ON_COMMAND_RANGE(IDC_BSC_PTRN_RADIO, IDC_ADV_PTRN_RADIO, &CCamPreviewerDlg::OnSwitchTracePattern)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CCamPreviewerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CCamPreviewerDlg::OnBnClickedCancel)
	ON_WM_SYSCOMMAND()
	ON_WM_CREATE()
	ON_WM_GETMINMAXINFO()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_ENTERSIZEMOVE()
	ON_WM_EXITSIZEMOVE()
	ON_BN_CLICKED(IDC_CAM_TRIGGER_BTN, &CCamPreviewerDlg::OnBnClickedCamTriggerBtn)
	ON_BN_CLICKED(IDC_TRACE_TRIGGER_BTN, &CCamPreviewerDlg::OnBnClickedTraceTriggerBtn)
	ON_BN_CLICKED(IDC_BSC_SETUP_BTN, &CCamPreviewerDlg::OnBnClickedBscSetupBtn)
	ON_BN_CLICKED(IDC_ADV_SETUP_BTN, &CCamPreviewerDlg::OnBnClickedAdvSetupBtn)
END_MESSAGE_MAP()

// CCamPreviewerDlg 메시지 처리기
BOOL CCamPreviewerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); // COM 라이브러리 초기화
	// 클라이언트 영역 기준 영상 미리 보기 윈도우 영역 좌표를 구함
	GetDlgItem(IDC_PREVIEW_RECT)->GetWindowRect(&m_preview_rect);
	ScreenToClient(&m_preview_rect);
	// 클라이언트 영역 기준 영상 처리 결과 출력 영역 좌표를 구함
	GetDlgItem(IDC_RESULT_RECT)->GetWindowRect(&m_result_rect);
	ScreenToClient(&m_result_rect);
	int width = m_preview_rect.right - m_preview_rect.left;
	int height = m_preview_rect.bottom - m_preview_rect.top;
	// 미리 보기 영상의 화면 크기와 동일한 크기로 이미지 객체를 생성
	m_mem_image.Create(width, height, 32);
	m_check_image.Create(width, height, 32);
	// 이미지 객체의 DC를 관리하기 위한 CDC 객체에 이미지 객체의 DC를 연결
	m_mem_dc.Attach(m_mem_image.GetDC());
	m_check_dc.Attach(m_check_image.GetDC());
	BITMAP bmp_info = { 0 }, check_bmp_info = { 0 };
	GetObject((HBITMAP)m_mem_image, sizeof(BITMAP), &bmp_info);
	GetObject((HBITMAP)m_check_image, sizeof(BITMAP), &check_bmp_info);
	// 이미지 객체의 비트맵 패턴 시작 주소를 얻어냄
	mp_image_pattern = (UINT8*)bmp_info.bmBits;
	mp_check_pattern = (UINT8*)check_bmp_info.bmBits;
	// 이미지 객체의 비트맵 패턴 크기를 얻어냄
	m_image_pattern_size = bmp_info.bmWidthBytes * bmp_info.bmHeight;
	// 클라이언트 영역 테두리와 컨트롤 가이드 영역 사이의 간격을 구함
	m_guide_spacing = m_preview_rect.left;
	// 영상 미리 보기 윈도우 영역과 영상 처리 결과 출력 영역의 상단(top) 좌표를 구함
	m_rect_top = m_preview_rect.top;
	// 영상 미리 보기 윈도우 영역과 영상 처리 결과 출력 영역 사이의 간격을 구함
	m_rect_spacing = m_result_rect.left - m_preview_rect.right;
	// 라디오 버튼 그룹들의 초기 선택 버튼을 설정함
	((CButton*)GetDlgItem(IDC_COLOR_RADIO))->SetCheck(TRUE);
	((CButton*)GetDlgItem(IDC_R_RADIO))->SetCheck(TRUE);
	((CButton*)GetDlgItem(IDC_BSC_PTRN_RADIO))->SetCheck(TRUE);
	MakeDeviceList(); // 사용 가능한 영상 장치 목록 생성
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.
void CCamPreviewerDlg::OnPaint()
{
	CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.
	if (IsIconic())
	{
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		m_mem_image.Draw(dc, m_result_rect);
		// CDialog::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CCamPreviewerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCamPreviewerDlg::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// CDialog::OnOK();
}

void CCamPreviewerDlg::OnBnClickedCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// CDialog::OnCancel();
}

void CCamPreviewerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	// ESC 키의 입력으로 윈도우를 닫는 동작은 금지하지만 닫기 버튼이나 'ALT + F4' 단축키의 입력으로
	// 윈도우를 닫는 동작은 허용함
	if ((nID & 0xFFF0) == SC_CLOSE)
	{
		EndDialog(IDCANCEL);
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

int CCamPreviewerDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  여기에 특수화된 작성 코드를 추가합니다.
	// 윈도우의 DPI 값을 통해 확대 배율을 구함
	m_dpi = ::GetDpiForWindow(m_hWnd);
	double scale_factor = (double)m_dpi / 96.0;
	// 확대 배율이 적용된 윈도우의 최소 폭과 높이를 구함
	m_min_cx = (int)((double)MIN_CX * scale_factor);
	m_min_cy = (int)((double)MIN_CY * scale_factor);
	return 0;
}

void CCamPreviewerDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	// 윈도우의 최소 크기를 설정함
	lpMMI->ptMinTrackSize.x = m_min_cx;
	lpMMI->ptMinTrackSize.y = m_min_cy;
	CDialog::OnGetMinMaxInfo(lpMMI);
}

void CCamPreviewerDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 영상 미리 보기가 실행중이라면 영상 미리 보기를 종료함
	if (m_preview_flag)
	{
		OnBnClickedCamTriggerBtn();
	}
	int count = m_cam_list.GetCount();
	// 사용하던 영상 장치 인터페이스 모두 해제
	for (int i = 0; i < count; ++i)
	{
		((IMoniker*)m_cam_list.GetItemDataPtr(i))->Release();
	}
	CoUninitialize(); // COM 라이브러리 사용 중지
	// CDC 객체에 연결된 이미지 객체의 DC를 연결 해제함
	m_mem_dc.Detach();
	m_check_dc.Detach();
	// 이미지 객체 DC의 핸들값 반환하고 이미지 객체에 만들어진 비트맵 제거함
	m_mem_image.ReleaseDC();
	m_check_image.ReleaseDC();
	m_mem_image.Destroy();
	m_check_image.Destroy();
}

void CCamPreviewerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	// 윈도우의 크기를 변경하는 중이 아닐 때에만 추적 작업을 수행함
	if (nIDEvent == TRACE_TIMER_ID && !m_resize_flag)
	{
		// 영상 미리 보기가 실행중일 때에만 추적 작업을 수행함
		if (!mp_cam_preview)
		{
			return;
		}
		// 미리 보기 영상 화면을 캡처함
		mp_cam_preview->Capture(m_mem_dc.m_hDC);
		CClientDC dc(this);
		// 색상 추적, 패턴 추적 중에서 수행할 영상 처리 작업을 선택함
		switch (m_trace_target)
		{
		case 0: // 색상 추적
			TraceColorFromPreview(dc.m_hDC);
			break;
		case 1: // 패턴 추적
			TracePatternFromPreview(dc.m_hDC);
			break;
		}
	}
	else
	{
		CDialog::OnTimer(nIDEvent);
	}
}

void CCamPreviewerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 윈도우가 최소화된 상태거나 메모리 DC 이미지 객체가 생성되지 않았다면 아무런 작업도 하지 않음
	if (!(cx * cy) || m_mem_image.IsNull())
	{
		return;
	}
	int x = 0, y = 0, width = 0, height = 0;
	// 현재 윈도우의 크기에 맞는 영상 미리 보기 윈도우 영역을 설정함
	width = (cx - m_guide_spacing * 2 - m_rect_spacing) / 2;
	height = cy - m_guide_spacing - m_rect_top;
	GetDlgItem(IDC_PREVIEW_RECT)->SetWindowPos(NULL, 0, 0, width, height, SWP_NOMOVE);
	GetDlgItem(IDC_PREVIEW_RECT)->GetWindowRect(&m_preview_rect);
	ScreenToClient(&m_preview_rect);
	// 현재 윈도우의 크기에 맞는 영상 처리 결과 출력 영역을 설정함
	x = m_preview_rect.right + m_rect_spacing;
	y = m_rect_top;
	width = cx - m_guide_spacing - x;
	GetDlgItem(IDC_RESULT_RECT)->SetWindowPos(NULL, x, y, width, height, 0);
	GetDlgItem(IDC_RESULT_RECT)->GetWindowRect(&m_result_rect);
	ScreenToClient(&m_result_rect);
	// 영상 미리 보기가 실행중이라면 영상 미리 보기 윈도우의 크기를 조절함
	x = m_preview_rect.left;
	y = m_preview_rect.top;
	width = m_preview_rect.right - m_preview_rect.left;
	height = m_preview_rect.bottom - m_preview_rect.top;
	if (m_preview_flag)
	{
		mp_cam_preview->SetVideoWndPos(x, y, width, height);
	}
	// CDC 객체에 연결된 이미지 객체의 DC를 연결 해제
	m_mem_dc.Detach();
	m_check_dc.Detach();
	// 이미지 객체 DC의 핸들값 반환하고 이미지 객체에 만들어진 비트맵 제거
	m_mem_image.ReleaseDC();
	m_check_image.ReleaseDC();
	m_mem_image.Destroy();
	m_check_image.Destroy();
	// 변경된 영상 미리 보기 윈도우 영역에 맞는 이미지 객체를 생성
	m_mem_image.Create(width, height, 32);
	m_check_image.Create(width, height, 32);
	// 이미지 객체의 DC를 관리하기 위한 CDC 객체에 이미지 객체의 DC를 연결
	m_mem_dc.Attach(m_mem_image.GetDC());
	m_check_dc.Attach(m_check_image.GetDC());
	BITMAP bmp_info = { 0 }, check_bmp_info = { 0 };
	GetObject((HBITMAP)m_mem_image, sizeof(BITMAP), &bmp_info);
	GetObject((HBITMAP)m_check_image, sizeof(BITMAP), &check_bmp_info);
	// 이미지 객체의 비트맵 패턴 시작 주소를 얻어냄
	mp_image_pattern = (UINT8*)bmp_info.bmBits;
	mp_check_pattern = (UINT8*)check_bmp_info.bmBits;
	// 이미지 객체의 비트맵 패턴 크기를 얻어냄
	m_image_pattern_size = bmp_info.bmWidthBytes * bmp_info.bmHeight;
}

void CCamPreviewerDlg::OnEnterSizeMove()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	m_resize_flag = 1; // 윈도우 크기 변경 진행중
	CDialog::OnEnterSizeMove();
}

void CCamPreviewerDlg::OnExitSizeMove()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	m_resize_flag = 0; // 윈도우 크기 변경 중지됨
	CDialog::OnExitSizeMove();
}

void CCamPreviewerDlg::OnSwitchTraceTarget(UINT a_ctrl_id)
{
	m_trace_target = (a_ctrl_id - IDC_COLOR_RADIO);
}

void CCamPreviewerDlg::OnSwitchTraceColor(UINT a_ctrl_id)
{
	m_trace_color = (a_ctrl_id - IDC_R_RADIO);
}

void CCamPreviewerDlg::OnSwitchTracePattern(UINT a_ctrl_id)
{
	m_trace_pattern = (a_ctrl_id - IDC_BSC_PTRN_RADIO);
}

void CCamPreviewerDlg::OnBnClickedCamTriggerBtn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// 영상 미리 보기 실행
	if (!m_preview_flag)
	{
		int index = m_cam_list.GetCurSel();
		// 영상 미리 보기가 이미 실행중이거나 영상 장치 목록 리스트 박스에서 선택된 영상 장치가 없다면
		// 영상 미리 보기 실행 작업을 수행하지 않음
		if (index == LB_ERR || mp_cam_preview)
		{
			AfxMessageBox(L"preview start error");
			return;
		}
		CString str;
		int x = 0, y = 0, cx = 0, cy = 0;
		m_cam_list.GetText(index, str); // 선택한 영상 장치의 이름을 얻어냄
		// 선택한 영상 장치의 이름과 인터페이스로 영상 미리 보기 시스템을 제공할 객체를 생성함
		mp_cam_preview = new TXYC_PreviewBuilder((IMoniker*)m_cam_list.GetItemDataPtr(index), str);
		// 미리 보기 영상을 재생함
		x = m_preview_rect.left;
		y = m_preview_rect.top;
		cx = m_preview_rect.right - m_preview_rect.left;
		cy = m_preview_rect.bottom - m_preview_rect.top;
		mp_cam_preview->StartPreview(m_hWnd, x, y, cx, cy);
		m_preview_flag = 1; // 영상 미리 보기 실행 상태를 실행중으로 설정함
		GetDlgItem(IDC_CAM_TRIGGER_BTN)->SetWindowText(L"촬영 중지");
	}
	// 영상 미리 보기 종료
	else
	{
		// 영상 미리 보기가 이미 종료된 상태라면 영상 미리 보기 종료 작업을 수행하지 않음
		if (!mp_cam_preview)
		{
			AfxMessageBox(L"preview stop error");
			return;
		}
		// 영상 미리 보기가 종료됐을 때 추적 작업이 실행중이라면 추적 작업을 중단함
		if (m_trace_flag)
		{
			OnBnClickedTraceTriggerBtn();
		}
		mp_cam_preview->StopPreview(); // 미리 보기 영상의 재생을 중지함
		// 미리 보기 영상을 제공하던 객체 제거한 다음 다시 사용할 수 있도록 초기화함
		delete mp_cam_preview;
		mp_cam_preview = NULL;
		m_preview_flag = 0; // 영상 미리 보기 실행 상태를 중지됨으로 설정함
		GetDlgItem(IDC_CAM_TRIGGER_BTN)->SetWindowText(L"촬영 시작");
	}
}

void CCamPreviewerDlg::OnBnClickedTraceTriggerBtn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// 영상 미리 보기가 실행중일 때에만 추적 작업을 수행함
	if (!mp_cam_preview)
	{
		return;
	}
	// 추적 작업 실행
	if (!m_trace_flag)
	{
		SetTimer(TRACE_TIMER_ID, 50, NULL);
		m_trace_flag = 1; // 추적 작업 실행 상태를 실행중으로 설정함
		GetDlgItem(IDC_TRACE_TRIGGER_BTN)->SetWindowText(L"추적 중지");
	}
	// 추적 작업 종료
	else
	{
		KillTimer(TRACE_TIMER_ID);
		m_trace_flag = 0; // 추적 작업 실행 상태를 중지됨으로 설정함
		UINT8* p_pos = mp_image_pattern, * p_end_pattern = mp_image_pattern + m_image_pattern_size;
		// 추적 작업이 종료됐으면 영상 처리 결과 이미지의 모든 픽셀을 검은색으로 초기화함
		while (p_pos < p_end_pattern)
		{
			*p_pos++ = 0;
			*p_pos++ = 0;
			*p_pos++ = 0;
			*p_pos++ = 0xFF;
		}
		p_pos = mp_check_pattern;
		while (p_pos < p_end_pattern)
		{
			*p_pos++ = 0;
			*p_pos++ = 0;
			*p_pos++ = 0;
			*p_pos++ = 0xFF;
		}
		CClientDC dc(this);
		m_mem_image.Draw(dc, m_result_rect);
		GetDlgItem(IDC_TRACE_TRIGGER_BTN)->SetWindowText(L"추적 시작");
	}
}

void CCamPreviewerDlg::OnBnClickedBscSetupBtn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// 영상 미리 보기가 실행중일 때에만 CAM 장치의 출력 영상 속성 윈도우를 생성함
	if (!mp_cam_preview)
	{
		return;
	}
	// CAM 장치의 출력 영상 속성 윈도우를 생성하기 전에 추적 작업을 수행중이었다면 추적 작업 수행 상태를
	// 기억해두고 추적 작업을 중단함
	char old_trace_flag = m_trace_flag;
	if (old_trace_flag)
	{
		OnBnClickedTraceTriggerBtn();
	}
	// CAM 장치의 출력 영상 속성 윈도우를 생성함
	mp_cam_preview->ShowCapturePinOption();
	// CAM 장치의 출력 영상 속성 윈도우를 생성하기 전에 추적 작업을 수행중이었다면 추적 작업을 다시 수행함
	if (old_trace_flag)
	{
		OnBnClickedTraceTriggerBtn();
	}
}

void CCamPreviewerDlg::OnBnClickedAdvSetupBtn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// 영상 미리 보기가 실행중일 때에만 CAM 장치의 하드웨어 속성 윈도우를 생성함
	if (!mp_cam_preview)
	{
		return;
	}
	// CAM 장치의 하드웨어 속성 윈도우를 생성함
	mp_cam_preview->ShowCaptureFilterOption();
}
