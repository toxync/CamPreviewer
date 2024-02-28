#include "pch.h"
#include "TXYC_PreviewBuilder.h"

#pragma comment(lib, "strmiids.lib")

void TXYC_DeleteMediaType(AM_MEDIA_TYPE* ap_media_type, UINT8 a_self_delete)
{
	if (ap_media_type->cbFormat)
	{
		CoTaskMemFree((void*)ap_media_type->pbFormat);
		ap_media_type->cbFormat = 0;
		ap_media_type->pbFormat = NULL;
	}
	if (ap_media_type->pUnk)
	{
		ap_media_type->pUnk->Release();
		ap_media_type->pUnk = NULL;
	}
	if (a_self_delete)
	{
		CoTaskMemFree(ap_media_type);
	}
}

TXYC_PreviewBuilder::TXYC_PreviewBuilder(IMoniker* ap_video_dev, const wchar_t* ap_video_dev_name)
{
	// 영상 미리 보기 시스템 Filter Graph의 Filter Graph Builder(Filter Graph 생성용 객체)를 생성함
	HRESULT h_res = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
		IID_ICaptureGraphBuilder2, (void**)&mp_graph_builder);
	if (!SUCCEEDED(h_res))
	{
		AfxMessageBox(L"failed to create Capture Graph Builder");
		return;
	}
	// 영상 미리 보기 필터(CAM 장치가 촬영중인 영상을 얻어오는 기능)를 사용하기 위한 인터페이스를 얻어냄
	if (!SUCCEEDED(ap_video_dev->BindToObject(0, 0, IID_IBaseFilter, (void**)&mp_video_capture)))
	{
		AfxMessageBox(L"failed to get video capture filter");
		return;
	}
	// 영상 미리 보기 시스템 Filter Graph의 Filter Graph Manager(Filter Graph 관리용 객체)를 생성함
	if (!SUCCEEDED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder,
		(void**)&mp_filter_graph)))
	{
		AfxMessageBox(L"failed to create Filter Graph Manager");
		return;
	}
	// 생성된 Filter Graph Manager 객체로 Filter Graph Builder를 초기화함
	if (!SUCCEEDED(mp_graph_builder->SetFiltergraph(mp_filter_graph)))
	{
		AfxMessageBox(L"failed to initialize Capture Graph Builder");
		return;
	}
	// 영상 미리 보기 필터를 Filter Graph에 추가함(CAM 장치를 영상 미리보기 시스템에 연결함)
	if (!SUCCEEDED(mp_filter_graph->AddFilter(mp_video_capture, ap_video_dev_name)))
	{
		AfxMessageBox(L"failed to add video capture filter");
		return;
	}
	// 영상 압축 정보를 사용하기 위한 인터페이스를 얻어냄
	h_res = mp_graph_builder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved,
		mp_video_capture, IID_IAMVideoCompression, (void**)&mp_video_comp);
	// 압축 정보 인터페이스를 MEDIATYPE_Interleaved 형식으로 얻지 못했다면 MEDIATYPE_Video 형식으로 얻어냄
	if (h_res != S_OK)
	{
		h_res = mp_graph_builder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
			mp_video_capture, IID_IAMVideoCompression, (void**)&mp_video_comp);
	}
	// 영상 장치의 설정 정보 관리 인터페이스를 얻어냄
	h_res = mp_graph_builder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved,
		mp_video_capture, IID_IAMStreamConfig, (void**)&mp_stream_config);
	// 설정 정보 관리 인터페이스를 MEDIATYPE_Interleaved 형식으로 얻지 못했다면 MEDIATYPE_Video 형식으로 얻어냄
	if (h_res != NOERROR)
	{
		h_res = mp_graph_builder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
			mp_video_capture, IID_IAMStreamConfig, (void**)&mp_stream_config);
	}
	if (!mp_stream_config)
	{
		AfxMessageBox(L"failed to get IAMStreamConfig");
		return;
	}
	// 영상 장치 설정 정보와 관련된 인터페이스를 얻었다면 영상 장치 설정 정보를 가져옴
	mp_stream_config->GetFormat(&mp_media_type);
	// 가져온 영상 장치 설정 정보를 재구성해서 강제로 다시 설정함
	mp_stream_config->SetFormat(mp_media_type);
}

TXYC_PreviewBuilder::~TXYC_PreviewBuilder()
{
	if (mp_media_type)
	{
		TXYC_DeleteMediaType(mp_media_type);
	}
	if (mp_filter_graph)
	{
		mp_filter_graph->Release();
		mp_filter_graph = NULL;
	}
	if (mp_video_comp)
	{
		mp_video_comp->Release();
		mp_video_comp = NULL;
	}
	if (mp_stream_config)
	{
		mp_stream_config->Release();
		mp_stream_config = NULL;
	}
	if (mp_video_window)
	{
		mp_video_window->Release();
		mp_video_window = NULL;
	}
	if (mp_basic_video)
	{
		mp_basic_video->Release();
		mp_basic_video = NULL;
	}
	if (mp_media_event)
	{
		mp_media_event->Release();
		mp_media_event = NULL;
	}
	if (mp_graph_builder)
	{
		mp_graph_builder->Release();
		mp_graph_builder = NULL;
	}
	if (mp_video_capture)
	{
		mp_video_capture->Release();
		mp_video_capture = NULL;
	}
}

char TXYC_PreviewBuilder::GetMediaSize(SIZE* ap_size)
{
	if (!mp_media_type)
	{
		return -1;
	}
	if (mp_media_type->formattype == FORMAT_VideoInfo)
	{
		ap_size->cx = HEADER(mp_media_type->pbFormat)->biWidth;
		ap_size->cy = HEADER(mp_media_type->pbFormat)->biHeight;
		if (ap_size->cy < 0)
		{
			ap_size->cy *= -1;
		}
		return 1;
	}
	else if (mp_media_type->formattype == FORMAT_DvInfo && mp_basic_video)
	{
		HRESULT h_res1 = 0, h_res2 = 0;
		h_res1 = mp_basic_video->get_VideoWidth(&ap_size->cx);
		h_res2 = mp_basic_video->get_VideoHeight(&ap_size->cy);
		if (SUCCEEDED(h_res1) && SUCCEEDED(h_res2))
		{
			return 1;
		}
	}
	return 0;
}

AM_MEDIA_TYPE* TXYC_PreviewBuilder::SetMediaTypeByIndex(int a_index)
{
	int pi_count = 0, pi_size = 0;
	mp_stream_config->GetNumberOfCapabilities(&pi_count, &pi_size);
	// 인덱스 오류
	if (pi_count <= a_index)
	{
		return NULL;
	}
	// 이미 구성된 미디어 정보가 있다면 제거함
	if (mp_media_type)
	{
		TXYC_DeleteMediaType(mp_media_type);
	}
	BYTE* p_scc = new BYTE[pi_size];
	// 장치 미디어 설정 정보의 a_index번째 미디어 정보를 얻어냄
	mp_stream_config->GetStreamCaps(a_index, &mp_media_type, p_scc);
	// 얻은 미디어 정보를 장치에 설정함
	mp_stream_config->SetFormat(mp_media_type);
	delete[] p_scc;
	return mp_media_type;
}

AM_MEDIA_TYPE* TXYC_PreviewBuilder::SetMediaTypeBySize(int a_width, int a_height)
{
	BITMAPINFOHEADER* p_header;
	// 이미 구성된 미디어 정보가 있다면 해당 미디어 정보의 화면 크기를 확인함
	if (mp_media_type)
	{
		p_header = &((VIDEOINFOHEADER*)mp_media_type->pbFormat)->bmiHeader;
		// 이미 구성된 미디어 정보의 화면 크기가 목표 영상의 화면 크기와 동일하다면
		// 해당 미디어 정보를 장치에 설정함
		if (p_header->biWidth == a_width && p_header->biHeight == a_height)
		{
			mp_stream_config->SetFormat(mp_media_type);
			return mp_media_type;
		}
		// 이미 구성된 미디어 정보의 화면 크기가 목표 영상의 화면 크기와 다르다면
		// 해당 미디어 정보를 제거함
		TXYC_DeleteMediaType(mp_media_type);
		mp_media_type = NULL;
	}
	int pi_count = 0, pi_size = 0, alike_index = -1;
	mp_stream_config->GetNumberOfCapabilities(&pi_count, &pi_size);
	AM_MEDIA_TYPE* pmt = NULL;
	BYTE* p_scc = new BYTE[pi_size];
	for (int i = 0; i < pi_count; ++i)
	{
		// 장치 미디어 설정 정보의 i번째 미디어 정보를 얻어냄
		mp_stream_config->GetStreamCaps(i, &pmt, p_scc);
		p_header = &((VIDEOINFOHEADER*)pmt->pbFormat)->bmiHeader;
		if (p_header->biWidth == a_width)
		{
			if (p_header->biHeight == a_height)
			{
				// 현재 미디어 정보의 화면 크기가 목표 영상의 화면 크기와 동일하다면
				// 해당 미디어 정보를 장치에 설정함
				mp_stream_config->SetFormat(pmt);
				mp_media_type = pmt;
				break;
			}
			// 현재 미디어 정보와 목표 영상의 폭은 일치하는데 높이가 일치하지 않으면
			// 현재 미디어 정보를 유사한 미디어 정보로 저장해둠
			else
			{
				alike_index = i;
			}
		}
		TXYC_DeleteMediaType(pmt); // 현재 미디어 정보를 제거함
	}
	// 목표 영상의 화면 크기와 일치하는 미디어 정보는 없지만 유사한 미디어 정보가 있다면
	// 해당 미디어 정보를 장치에 설정함
	if (!mp_media_type && alike_index != -1)
	{
		mp_stream_config->GetStreamCaps(alike_index, &mp_media_type, p_scc);
		mp_stream_config->SetFormat(mp_media_type);
	}
	delete[] p_scc;
	return mp_media_type;
}

char TXYC_PreviewBuilder::SetPreviewMode(HWND ah_wnd, int a_x, int a_y, int a_cx, int a_cy, UINT a_msg_id)
{
	if (m_preview_flag > 0)
	{
		return m_preview_flag;
	}
	HRESULT h_res = 0;
	m_notify_id = a_msg_id;
	mh_owner_wnd = ah_wnd; // 영상 미리 보기 윈도우의 부모 윈도우를 설정함
	// 영상 미리 보기 시스템 Filter Graph를 생성함
	h_res = mp_graph_builder->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Interleaved,
		mp_video_capture, NULL, NULL);
	if (h_res == VFW_S_NOPREVIEWPIN || h_res == S_OK)
	{
		m_preview_flag = 1;
	}
	// 영상 미리 보기 시스템 Filter Graph를 MEDIATYPE_Interleaved 형식으로 생성하지 못했다면
	// MEDIATYPE_Video 형식으로 생성함
	else
	{
		h_res = mp_graph_builder->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
			mp_video_capture, NULL, NULL);
		if (h_res == VFW_S_NOPREVIEWPIN || h_res == S_OK)
		{
			m_preview_flag = 1;
		}
		else
		{
			AfxMessageBox(L"failed to build video preview filter graph");
			m_preview_flag = -1;
			return m_preview_flag;
		}
	}
	// 영상 미리 보기 윈도우를 사용하기 위한 인터페이스를 얻어냄
	if (NOERROR != mp_filter_graph->QueryInterface(IID_IVideoWindow, (void**)&mp_video_window))
	{
		AfxMessageBox(L"failed to get IVideoWindow");
		m_preview_flag = -1;
		return m_preview_flag;
	}
	// 시스템에 연결된 CAM 장치가 제공하는 영상 형식이 FORMAT_DvInfo라면 영상의 화면 크기를 구할 때
	// 사용하기 위한 IBasicVideo 인터페이스를 얻어냄
	if (!mp_media_type && mp_stream_config && SUCCEEDED(mp_stream_config->GetFormat(&mp_media_type)))
	{
		if (mp_media_type->formattype == FORMAT_DvInfo)
		{
			mp_video_window->QueryInterface(IID_IBasicVideo, (void**)&mp_basic_video);
		}
	}
	// 영상 미리 보기 윈도우의 위치와 크기를 설정함
	SetVideoWndPos(a_x, a_y, a_cx, a_cy);
	// 영상 미리 보기 윈도우의 속성과 부모 윈도우를 설정한 다음 영상 미리 보기 윈도우를 표시함
	mp_video_window->put_Owner((OAHWND)mh_owner_wnd);
	mp_video_window->put_WindowStyle(WS_CHILD);
	mp_video_window->put_Visible(OATRUE);
	if (NOERROR != mp_filter_graph->QueryInterface(IID_IMediaEventEx, (void**)&mp_media_event))
	{
		AfxMessageBox(L"failed to get IMediaEventEx");
		m_preview_flag = -1;
		return m_preview_flag;
	}
	// 영상 미리 보기 윈도우의 부모 윈도우를 이벤트 수신 윈도우로 설정함
	mp_media_event->SetNotifyWindow((OAHWND)mh_owner_wnd, m_notify_id, 0);
	return m_preview_flag;
}

void TXYC_PreviewBuilder::SetVideoWndPos(int a_x, int a_y, int a_cx, int a_cy)
{
	m_preview_x = a_x;
	m_preview_y = a_y;
	// 영상 미리 보기 윈도우의 크기를 설정함
	if (a_cx)
	{
		m_preview_size.cx = a_cx;
		m_preview_size.cy = a_cy;
	}
	else
	{
		GetMediaSize(&m_preview_size);
	}
	mp_video_window->SetWindowPosition(m_preview_x, m_preview_y, m_preview_size.cx, m_preview_size.cy);
}

char TXYC_PreviewBuilder::StartPreview(HWND ah_wnd, int a_x, int a_y, int a_cx, int a_cy)
{
	if (SetPreviewMode(ah_wnd, a_x, a_y, a_cx, a_cy) != 1)
	{
		return m_preview_flag;
	}
	IMediaControl* p_media_control = NULL;
	// 미리 보기 영상을 재생하기 위한 미디어 제어 인터페이스를 얻어냄
	if (!SUCCEEDED(mp_filter_graph->QueryInterface(IID_IMediaControl, (void**)&p_media_control)))
	{
		AfxMessageBox(L"failed to get IMediaControl");
		m_preview_flag = -1;
		return m_preview_flag;
	}
	// 미리 보기 영상을 재생시킴
	if (FAILED(p_media_control->Run()))
	{
		// 미리 보기 영상 재생에 실패하면 미리보기 영상의 재생을 중지시킴
		p_media_control->Stop();
	}
	else
	{
		// 미리 보기 영상 재생에 성공하면 영상 미리 보기 윈도우의 핸들을 구함
		mh_preview_wnd = ::FindWindowEx(mh_owner_wnd, NULL, L"VideoRenderer", NULL);
		m_preview_flag = 2;
	}
	p_media_control->Release(); // 미디어 제어 인터페이스 해제
	return m_preview_flag;
}

void TXYC_PreviewBuilder::StopPreview()
{
	if (m_preview_flag != 2)
	{
		return;
	}
	IMediaControl* p_media_control = NULL;
	// 미리 보기 영상을 중지하기 위한 미디어 제어 인터페이스를 얻어냄
	if (!SUCCEEDED(mp_filter_graph->QueryInterface(IID_IMediaControl, (void**)&p_media_control)))
	{
		AfxMessageBox(L"failed to get IMediaControl");
		return;
	}
	p_media_control->Stop(); // 미리 보기 영상의 재생을 중지시킴
	p_media_control->Release(); // 미디어 제어 인터페이스 해제
	m_preview_flag = 1;
}

void TXYC_PreviewBuilder::Capture(HDC ah_dest_dc)
{
	HDC h_dc = ::GetDC(mh_preview_wnd); // 캡처할 영상 미리 보기 윈도우의 DC를 얻어냄
	// 매개 변수로 넘겨 받은 DC에 영상 미리 보기 윈도우의 전체 영역 이미지를 복사함
	BitBlt(ah_dest_dc, 0, 0, m_preview_size.cx, m_preview_size.cy, h_dc, 0, 0, SRCCOPY);
	// 얻어낸 영상 미리 보기 윈도우의 DC를 반납함
	::ReleaseDC(mh_preview_wnd, h_dc);
}

void TXYC_PreviewBuilder::ShowCapturePinOption()
{
	// 영상 미리 보기를 중지시킴
	StopPreview();
	m_preview_flag = 0;
	// 영상 미리 보기에 사용한 객체들을 제거함
	if (mp_video_window)
	{
		mp_video_window->put_Owner(NULL);
		mp_video_window->put_Visible(FALSE);
		mp_video_window->Release();
		mp_video_window = NULL;
	}
	if (mp_media_event)
	{
		mp_media_event->Release();
		mp_media_event = NULL;
	}
	if (mp_basic_video)
	{
		mp_basic_video->Release();
		mp_basic_video = NULL;
	}
	// 미리 보기 영상의 미디어 정보를 제거함
	if (mp_media_type)
	{
		TXYC_DeleteMediaType(mp_media_type);
		mp_media_type = NULL;
	}
	// 영상 미리 보기 시스템 Filter Graph의 마지막 필터부터 영상 미리 보기 필터의 다음 필터까지 모든 필터들을 제거함
	RemoveDownStream(mp_video_capture);
	IAMStreamConfig* p_stream_config = NULL;
	ISpecifyPropertyPages* p_spec = NULL;
	CAUUID cauuid = { 0 };
	// 영상 장치의 설정 정보 관리 인터페이스를 얻어냄
	HRESULT h_res = mp_graph_builder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved,
		mp_video_capture, IID_IAMStreamConfig, (void**)&p_stream_config);
	// 설정 정보 관리 인터페이스를 MEDIATYPE_Interleaved 형식으로 얻지 못했다면 MEDIATYPE_Video 형식으로 얻어냄
	if (h_res != NOERROR)
	{
		h_res = mp_graph_builder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
			mp_video_capture, IID_IAMStreamConfig, (void**)&p_stream_config);
	}
	// 영상 미리 보기 시스템에 연결된 CAM 장치의 출력 영상 속성을 가져오기 위한 인터페이스를 얻어냄
	if (S_OK == p_stream_config->QueryInterface(IID_ISpecifyPropertyPages, (void**)&p_spec))
	{
		// 속성 윈도우(property sheet)에 표시할 속성 데이터(property page)를 얻어냄
		if (SUCCEEDED(p_spec->GetPages(&cauuid)) && (cauuid.cElems > 0))
		{
			// 얻어낸 속성 데이터(property page)를 속성 윈도우(property sheet)에 표시함
			h_res = OleCreatePropertyFrame(mh_owner_wnd, 30, 30, NULL, 1, (IUnknown**)&p_stream_config,
				cauuid.cElems, (GUID*)cauuid.pElems, 0, 0, NULL);
			// 속성 윈도우(property sheet)가 닫히면 속성 데이터(property page)들은 더 이상 필요없기 때문에
			// 속성 데이터(property page) 정보를 가진 동적 할당된 배열을 해제함
			CoTaskMemFree(cauuid.pElems);
		}
		p_spec->Release();
	}
	p_stream_config->Release();
	// 중지시킨 영상 미리 보기를 다시 실행함
	StartPreview(mh_owner_wnd, m_preview_x, m_preview_y, m_preview_size.cx, m_preview_size.cy);
}

void TXYC_PreviewBuilder::ShowCaptureFilterOption()
{
	ISpecifyPropertyPages* p_spec = NULL;
	CAUUID cauuid = { 0 };
	// 영상 미리 보기 시스템에 연결된 CAM 장치의 하드웨어 속성을 가져오기 위한 인터페이스를 얻어냄
	if (S_OK == mp_video_capture->QueryInterface(IID_ISpecifyPropertyPages, (void**)&p_spec))
	{
		// 속성 윈도우(property sheet)에 표시할 속성 데이터(property page)를 얻어냄
		if (SUCCEEDED(p_spec->GetPages(&cauuid)) && (cauuid.cElems > 0))
		{
			// 얻어낸 속성 데이터(property page)를 속성 윈도우(property sheet)에 표시함
			OleCreatePropertyFrame(mh_owner_wnd, 30, 30, NULL, 1, (IUnknown**)&mp_video_capture,
				cauuid.cElems, (GUID*)cauuid.pElems, 0, 0, NULL);
			// 속성 윈도우(property sheet)가 닫히면 속성 데이터(property page)들은 더 이상 필요없기 때문에
			// 속성 데이터(property page) 정보를 가진 동적 할당된 배열을 해제함
			CoTaskMemFree(cauuid.pElems);
		}
		p_spec->Release();
	}
}

void TXYC_PreviewBuilder::RemoveDownStream(IBaseFilter* ap_filter)
{
	IPin* p_pin = NULL, * p_connected_pin = NULL;
	IEnumPins* p_enum_pins = NULL;
	ULONG fetch_cnt = 0;
	PIN_INFO pin_info = { 0 };
	if (!ap_filter)
	{
		return;
	}
	// 현재 필터에서 사용 가능한 핀들의 목록을 생성함
	HRESULT h_res = ap_filter->EnumPins(&p_enum_pins);
	p_enum_pins->Reset();
	while (h_res == NOERROR)
	{
		h_res = p_enum_pins->Next(1, &p_pin, &fetch_cnt);
		if (h_res == S_OK && p_pin)
		{
			// 현재 필터의 핀과 연결된 핀이 있는지 확인함
			p_pin->ConnectedTo(&p_connected_pin);
			if (p_connected_pin)
			{
				// 현재 필터의 핀과 연결된 핀이 있다면 연결된 핀의 정보를 얻어냄
				h_res = p_connected_pin->QueryPinInfo(&pin_info);
				if (h_res == NOERROR)
				{
					
					if (pin_info.dir == PINDIR_INPUT)
					{
						// 현재 필터의 다음 필터가 있다면(현재 필터의 핀과 연결된 핀이 입력 핀이고
						// 현재 필터가 다른 필터에게 데이터를 전송하는 필터라면) 해당 필터의 다음
						// 필터(현재 필터의 다음 필터로부터 데이터를 받는 필터)가 있는지 확인함
						RemoveDownStream(pin_info.pFilter);
						// 현재 필터와 현재 필터의 다음 필터를 연결하는 모든 핀들의 연결을 끊고
						// 현재 필터의 다음 필터를 제거함
						mp_filter_graph->Disconnect(p_connected_pin);
						mp_filter_graph->Disconnect(p_pin);
						mp_filter_graph->RemoveFilter(pin_info.pFilter);
					}
					pin_info.pFilter->Release();
				}
				p_connected_pin->Release();
			}
			p_pin->Release();
		}
	}
	if (p_enum_pins)
	{
		p_enum_pins->Release();
	}
}