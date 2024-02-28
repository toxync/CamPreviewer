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
	// ���� �̸� ���� �ý��� Filter Graph�� Filter Graph Builder(Filter Graph ������ ��ü)�� ������
	HRESULT h_res = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
		IID_ICaptureGraphBuilder2, (void**)&mp_graph_builder);
	if (!SUCCEEDED(h_res))
	{
		AfxMessageBox(L"failed to create Capture Graph Builder");
		return;
	}
	// ���� �̸� ���� ����(CAM ��ġ�� �Կ����� ������ ������ ���)�� ����ϱ� ���� �������̽��� ��
	if (!SUCCEEDED(ap_video_dev->BindToObject(0, 0, IID_IBaseFilter, (void**)&mp_video_capture)))
	{
		AfxMessageBox(L"failed to get video capture filter");
		return;
	}
	// ���� �̸� ���� �ý��� Filter Graph�� Filter Graph Manager(Filter Graph ������ ��ü)�� ������
	if (!SUCCEEDED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder,
		(void**)&mp_filter_graph)))
	{
		AfxMessageBox(L"failed to create Filter Graph Manager");
		return;
	}
	// ������ Filter Graph Manager ��ü�� Filter Graph Builder�� �ʱ�ȭ��
	if (!SUCCEEDED(mp_graph_builder->SetFiltergraph(mp_filter_graph)))
	{
		AfxMessageBox(L"failed to initialize Capture Graph Builder");
		return;
	}
	// ���� �̸� ���� ���͸� Filter Graph�� �߰���(CAM ��ġ�� ���� �̸����� �ý��ۿ� ������)
	if (!SUCCEEDED(mp_filter_graph->AddFilter(mp_video_capture, ap_video_dev_name)))
	{
		AfxMessageBox(L"failed to add video capture filter");
		return;
	}
	// ���� ���� ������ ����ϱ� ���� �������̽��� ��
	h_res = mp_graph_builder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved,
		mp_video_capture, IID_IAMVideoCompression, (void**)&mp_video_comp);
	// ���� ���� �������̽��� MEDIATYPE_Interleaved �������� ���� ���ߴٸ� MEDIATYPE_Video �������� ��
	if (h_res != S_OK)
	{
		h_res = mp_graph_builder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
			mp_video_capture, IID_IAMVideoCompression, (void**)&mp_video_comp);
	}
	// ���� ��ġ�� ���� ���� ���� �������̽��� ��
	h_res = mp_graph_builder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved,
		mp_video_capture, IID_IAMStreamConfig, (void**)&mp_stream_config);
	// ���� ���� ���� �������̽��� MEDIATYPE_Interleaved �������� ���� ���ߴٸ� MEDIATYPE_Video �������� ��
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
	// ���� ��ġ ���� ������ ���õ� �������̽��� ����ٸ� ���� ��ġ ���� ������ ������
	mp_stream_config->GetFormat(&mp_media_type);
	// ������ ���� ��ġ ���� ������ �籸���ؼ� ������ �ٽ� ������
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
	// �ε��� ����
	if (pi_count <= a_index)
	{
		return NULL;
	}
	// �̹� ������ �̵�� ������ �ִٸ� ������
	if (mp_media_type)
	{
		TXYC_DeleteMediaType(mp_media_type);
	}
	BYTE* p_scc = new BYTE[pi_size];
	// ��ġ �̵�� ���� ������ a_index��° �̵�� ������ ��
	mp_stream_config->GetStreamCaps(a_index, &mp_media_type, p_scc);
	// ���� �̵�� ������ ��ġ�� ������
	mp_stream_config->SetFormat(mp_media_type);
	delete[] p_scc;
	return mp_media_type;
}

AM_MEDIA_TYPE* TXYC_PreviewBuilder::SetMediaTypeBySize(int a_width, int a_height)
{
	BITMAPINFOHEADER* p_header;
	// �̹� ������ �̵�� ������ �ִٸ� �ش� �̵�� ������ ȭ�� ũ�⸦ Ȯ����
	if (mp_media_type)
	{
		p_header = &((VIDEOINFOHEADER*)mp_media_type->pbFormat)->bmiHeader;
		// �̹� ������ �̵�� ������ ȭ�� ũ�Ⱑ ��ǥ ������ ȭ�� ũ��� �����ϴٸ�
		// �ش� �̵�� ������ ��ġ�� ������
		if (p_header->biWidth == a_width && p_header->biHeight == a_height)
		{
			mp_stream_config->SetFormat(mp_media_type);
			return mp_media_type;
		}
		// �̹� ������ �̵�� ������ ȭ�� ũ�Ⱑ ��ǥ ������ ȭ�� ũ��� �ٸ��ٸ�
		// �ش� �̵�� ������ ������
		TXYC_DeleteMediaType(mp_media_type);
		mp_media_type = NULL;
	}
	int pi_count = 0, pi_size = 0, alike_index = -1;
	mp_stream_config->GetNumberOfCapabilities(&pi_count, &pi_size);
	AM_MEDIA_TYPE* pmt = NULL;
	BYTE* p_scc = new BYTE[pi_size];
	for (int i = 0; i < pi_count; ++i)
	{
		// ��ġ �̵�� ���� ������ i��° �̵�� ������ ��
		mp_stream_config->GetStreamCaps(i, &pmt, p_scc);
		p_header = &((VIDEOINFOHEADER*)pmt->pbFormat)->bmiHeader;
		if (p_header->biWidth == a_width)
		{
			if (p_header->biHeight == a_height)
			{
				// ���� �̵�� ������ ȭ�� ũ�Ⱑ ��ǥ ������ ȭ�� ũ��� �����ϴٸ�
				// �ش� �̵�� ������ ��ġ�� ������
				mp_stream_config->SetFormat(pmt);
				mp_media_type = pmt;
				break;
			}
			// ���� �̵�� ������ ��ǥ ������ ���� ��ġ�ϴµ� ���̰� ��ġ���� ������
			// ���� �̵�� ������ ������ �̵�� ������ �����ص�
			else
			{
				alike_index = i;
			}
		}
		TXYC_DeleteMediaType(pmt); // ���� �̵�� ������ ������
	}
	// ��ǥ ������ ȭ�� ũ��� ��ġ�ϴ� �̵�� ������ ������ ������ �̵�� ������ �ִٸ�
	// �ش� �̵�� ������ ��ġ�� ������
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
	mh_owner_wnd = ah_wnd; // ���� �̸� ���� �������� �θ� �����츦 ������
	// ���� �̸� ���� �ý��� Filter Graph�� ������
	h_res = mp_graph_builder->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Interleaved,
		mp_video_capture, NULL, NULL);
	if (h_res == VFW_S_NOPREVIEWPIN || h_res == S_OK)
	{
		m_preview_flag = 1;
	}
	// ���� �̸� ���� �ý��� Filter Graph�� MEDIATYPE_Interleaved �������� �������� ���ߴٸ�
	// MEDIATYPE_Video �������� ������
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
	// ���� �̸� ���� �����츦 ����ϱ� ���� �������̽��� ��
	if (NOERROR != mp_filter_graph->QueryInterface(IID_IVideoWindow, (void**)&mp_video_window))
	{
		AfxMessageBox(L"failed to get IVideoWindow");
		m_preview_flag = -1;
		return m_preview_flag;
	}
	// �ý��ۿ� ����� CAM ��ġ�� �����ϴ� ���� ������ FORMAT_DvInfo��� ������ ȭ�� ũ�⸦ ���� ��
	// ����ϱ� ���� IBasicVideo �������̽��� ��
	if (!mp_media_type && mp_stream_config && SUCCEEDED(mp_stream_config->GetFormat(&mp_media_type)))
	{
		if (mp_media_type->formattype == FORMAT_DvInfo)
		{
			mp_video_window->QueryInterface(IID_IBasicVideo, (void**)&mp_basic_video);
		}
	}
	// ���� �̸� ���� �������� ��ġ�� ũ�⸦ ������
	SetVideoWndPos(a_x, a_y, a_cx, a_cy);
	// ���� �̸� ���� �������� �Ӽ��� �θ� �����츦 ������ ���� ���� �̸� ���� �����츦 ǥ����
	mp_video_window->put_Owner((OAHWND)mh_owner_wnd);
	mp_video_window->put_WindowStyle(WS_CHILD);
	mp_video_window->put_Visible(OATRUE);
	if (NOERROR != mp_filter_graph->QueryInterface(IID_IMediaEventEx, (void**)&mp_media_event))
	{
		AfxMessageBox(L"failed to get IMediaEventEx");
		m_preview_flag = -1;
		return m_preview_flag;
	}
	// ���� �̸� ���� �������� �θ� �����츦 �̺�Ʈ ���� ������� ������
	mp_media_event->SetNotifyWindow((OAHWND)mh_owner_wnd, m_notify_id, 0);
	return m_preview_flag;
}

void TXYC_PreviewBuilder::SetVideoWndPos(int a_x, int a_y, int a_cx, int a_cy)
{
	m_preview_x = a_x;
	m_preview_y = a_y;
	// ���� �̸� ���� �������� ũ�⸦ ������
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
	// �̸� ���� ������ ����ϱ� ���� �̵�� ���� �������̽��� ��
	if (!SUCCEEDED(mp_filter_graph->QueryInterface(IID_IMediaControl, (void**)&p_media_control)))
	{
		AfxMessageBox(L"failed to get IMediaControl");
		m_preview_flag = -1;
		return m_preview_flag;
	}
	// �̸� ���� ������ �����Ŵ
	if (FAILED(p_media_control->Run()))
	{
		// �̸� ���� ���� ����� �����ϸ� �̸����� ������ ����� ������Ŵ
		p_media_control->Stop();
	}
	else
	{
		// �̸� ���� ���� ����� �����ϸ� ���� �̸� ���� �������� �ڵ��� ����
		mh_preview_wnd = ::FindWindowEx(mh_owner_wnd, NULL, L"VideoRenderer", NULL);
		m_preview_flag = 2;
	}
	p_media_control->Release(); // �̵�� ���� �������̽� ����
	return m_preview_flag;
}

void TXYC_PreviewBuilder::StopPreview()
{
	if (m_preview_flag != 2)
	{
		return;
	}
	IMediaControl* p_media_control = NULL;
	// �̸� ���� ������ �����ϱ� ���� �̵�� ���� �������̽��� ��
	if (!SUCCEEDED(mp_filter_graph->QueryInterface(IID_IMediaControl, (void**)&p_media_control)))
	{
		AfxMessageBox(L"failed to get IMediaControl");
		return;
	}
	p_media_control->Stop(); // �̸� ���� ������ ����� ������Ŵ
	p_media_control->Release(); // �̵�� ���� �������̽� ����
	m_preview_flag = 1;
}

void TXYC_PreviewBuilder::Capture(HDC ah_dest_dc)
{
	HDC h_dc = ::GetDC(mh_preview_wnd); // ĸó�� ���� �̸� ���� �������� DC�� ��
	// �Ű� ������ �Ѱ� ���� DC�� ���� �̸� ���� �������� ��ü ���� �̹����� ������
	BitBlt(ah_dest_dc, 0, 0, m_preview_size.cx, m_preview_size.cy, h_dc, 0, 0, SRCCOPY);
	// �� ���� �̸� ���� �������� DC�� �ݳ���
	::ReleaseDC(mh_preview_wnd, h_dc);
}

void TXYC_PreviewBuilder::ShowCapturePinOption()
{
	// ���� �̸� ���⸦ ������Ŵ
	StopPreview();
	m_preview_flag = 0;
	// ���� �̸� ���⿡ ����� ��ü���� ������
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
	// �̸� ���� ������ �̵�� ������ ������
	if (mp_media_type)
	{
		TXYC_DeleteMediaType(mp_media_type);
		mp_media_type = NULL;
	}
	// ���� �̸� ���� �ý��� Filter Graph�� ������ ���ͺ��� ���� �̸� ���� ������ ���� ���ͱ��� ��� ���͵��� ������
	RemoveDownStream(mp_video_capture);
	IAMStreamConfig* p_stream_config = NULL;
	ISpecifyPropertyPages* p_spec = NULL;
	CAUUID cauuid = { 0 };
	// ���� ��ġ�� ���� ���� ���� �������̽��� ��
	HRESULT h_res = mp_graph_builder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved,
		mp_video_capture, IID_IAMStreamConfig, (void**)&p_stream_config);
	// ���� ���� ���� �������̽��� MEDIATYPE_Interleaved �������� ���� ���ߴٸ� MEDIATYPE_Video �������� ��
	if (h_res != NOERROR)
	{
		h_res = mp_graph_builder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
			mp_video_capture, IID_IAMStreamConfig, (void**)&p_stream_config);
	}
	// ���� �̸� ���� �ý��ۿ� ����� CAM ��ġ�� ��� ���� �Ӽ��� �������� ���� �������̽��� ��
	if (S_OK == p_stream_config->QueryInterface(IID_ISpecifyPropertyPages, (void**)&p_spec))
	{
		// �Ӽ� ������(property sheet)�� ǥ���� �Ӽ� ������(property page)�� ��
		if (SUCCEEDED(p_spec->GetPages(&cauuid)) && (cauuid.cElems > 0))
		{
			// �� �Ӽ� ������(property page)�� �Ӽ� ������(property sheet)�� ǥ����
			h_res = OleCreatePropertyFrame(mh_owner_wnd, 30, 30, NULL, 1, (IUnknown**)&p_stream_config,
				cauuid.cElems, (GUID*)cauuid.pElems, 0, 0, NULL);
			// �Ӽ� ������(property sheet)�� ������ �Ӽ� ������(property page)���� �� �̻� �ʿ���� ������
			// �Ӽ� ������(property page) ������ ���� ���� �Ҵ�� �迭�� ������
			CoTaskMemFree(cauuid.pElems);
		}
		p_spec->Release();
	}
	p_stream_config->Release();
	// ������Ų ���� �̸� ���⸦ �ٽ� ������
	StartPreview(mh_owner_wnd, m_preview_x, m_preview_y, m_preview_size.cx, m_preview_size.cy);
}

void TXYC_PreviewBuilder::ShowCaptureFilterOption()
{
	ISpecifyPropertyPages* p_spec = NULL;
	CAUUID cauuid = { 0 };
	// ���� �̸� ���� �ý��ۿ� ����� CAM ��ġ�� �ϵ���� �Ӽ��� �������� ���� �������̽��� ��
	if (S_OK == mp_video_capture->QueryInterface(IID_ISpecifyPropertyPages, (void**)&p_spec))
	{
		// �Ӽ� ������(property sheet)�� ǥ���� �Ӽ� ������(property page)�� ��
		if (SUCCEEDED(p_spec->GetPages(&cauuid)) && (cauuid.cElems > 0))
		{
			// �� �Ӽ� ������(property page)�� �Ӽ� ������(property sheet)�� ǥ����
			OleCreatePropertyFrame(mh_owner_wnd, 30, 30, NULL, 1, (IUnknown**)&mp_video_capture,
				cauuid.cElems, (GUID*)cauuid.pElems, 0, 0, NULL);
			// �Ӽ� ������(property sheet)�� ������ �Ӽ� ������(property page)���� �� �̻� �ʿ���� ������
			// �Ӽ� ������(property page) ������ ���� ���� �Ҵ�� �迭�� ������
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
	// ���� ���Ϳ��� ��� ������ �ɵ��� ����� ������
	HRESULT h_res = ap_filter->EnumPins(&p_enum_pins);
	p_enum_pins->Reset();
	while (h_res == NOERROR)
	{
		h_res = p_enum_pins->Next(1, &p_pin, &fetch_cnt);
		if (h_res == S_OK && p_pin)
		{
			// ���� ������ �ɰ� ����� ���� �ִ��� Ȯ����
			p_pin->ConnectedTo(&p_connected_pin);
			if (p_connected_pin)
			{
				// ���� ������ �ɰ� ����� ���� �ִٸ� ����� ���� ������ ��
				h_res = p_connected_pin->QueryPinInfo(&pin_info);
				if (h_res == NOERROR)
				{
					
					if (pin_info.dir == PINDIR_INPUT)
					{
						// ���� ������ ���� ���Ͱ� �ִٸ�(���� ������ �ɰ� ����� ���� �Է� ���̰�
						// ���� ���Ͱ� �ٸ� ���Ϳ��� �����͸� �����ϴ� ���Ͷ��) �ش� ������ ����
						// ����(���� ������ ���� ���ͷκ��� �����͸� �޴� ����)�� �ִ��� Ȯ����
						RemoveDownStream(pin_info.pFilter);
						// ���� ���Ϳ� ���� ������ ���� ���͸� �����ϴ� ��� �ɵ��� ������ ����
						// ���� ������ ���� ���͸� ������
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