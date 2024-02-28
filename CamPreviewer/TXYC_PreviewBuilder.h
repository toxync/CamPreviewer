#ifndef _TXYC_PREVIEW_BUILDER_H_
#define _TXYC_PREVIEW_BUILDER_H_

// Ŭ���� �ۼ���: ������(bac6536@naver.com)
// �� Ŭ������ �ٸ� �����ڰ� �ۼ��� Ŭ������ ������� �ۼ��Ǿ���
// ���� Ŭ���� �ۼ���: �輺��(tipsware@naver.com)
// ���� Ŭ���� ���� �ּ�: https://cafe.naver.com/mythread

#include <dshow.h>

// ���� �̸� ���� ������� �θ� ������ ������ ��ſ� ����� �޽��� ��ȣ
#define NOTIFY_ID 12000

// �̵�� ������ �����ϴ� �Լ�
void TXYC_DeleteMediaType(AM_MEDIA_TYPE* ap_media_type, UINT8 a_self_delete = 1);

class TXYC_PreviewBuilder
{
private:
	// ���� �̸� ���� �ý��� Filter Graph�� Filter Graph Builder(Filter Graph ������ ��ü)
	ICaptureGraphBuilder2* mp_graph_builder = NULL;
	// ���� �̸� ���� �ý��� Filter Graph�� Filter Graph Manager(Filter Graph ������ ��ü)
	IGraphBuilder* mp_filter_graph = NULL;
	// ���� �̸� ���� ����(CAM ��ġ�� �Կ����� ������ ������ ���) �������̽�
	IBaseFilter* mp_video_capture = NULL;
	IAMVideoCompression* mp_video_comp = NULL; // ���� ���� ���� �������̽�
	IAMStreamConfig* mp_stream_config = NULL; // CAM ��ġ�� ���� ���� ���� �������̽�
	IVideoWindow* mp_video_window = NULL; // ���� �̸� ���� ������ �������̽�
	IBasicVideo* mp_basic_video = NULL; // CAM ��ġ�� �����ϴ� ������ �⺻ ���� �������̽�
	// �̸� ���� ������ ����ϴ� �������� �߻��� �̺�Ʈ�� ����ϱ� ���� �������̽�
	IMediaEventEx* mp_media_event = NULL;
	AM_MEDIA_TYPE* mp_media_type = NULL; // ����� ���õ� �̵�� ����
	// ���� �̸� ���� �������� �θ� ������ �ڵ�� ���� �̸� ���� �������� �ڵ�
	HWND mh_owner_wnd = NULL, mh_preview_wnd = NULL;
	SIZE m_preview_size = { 300,200 }; // �̸� ���� ������ ȭ�� ũ��
	UINT m_notify_id = NOTIFY_ID; // �̸� ���� ������ ���� ���濡 ����� �޽��� ID
	int m_preview_x = 0, m_preview_y = 0; // �̸� ���� ������ ��� ���� ��ǥ(x, y)
	// �̸� ���� ������ ��� ����(-1: ����, 0: �̸� ���� �غ� �� ��, 1: �̸� ���� ������, 2: �̸� ���� �����)
	char m_preview_flag = 0;
public:
	TXYC_PreviewBuilder(IMoniker* ap_video_dev, const wchar_t* ap_video_dev_name);
	~TXYC_PreviewBuilder();
	// �̸� ���� ������ ȭ�� ũ�⸦ ���ϴ� �Լ�
	// (�̵�� ������ ������ -1, �̵�� ���� ������ �߸������� 0, ���������� ��������� 1�� ��ȯ)
	char GetMediaSize(SIZE* ap_size);
	// �Ű� ������ �־��� �ε����� ��ġ�� �̵�� ������ ��ġ�� �����ϴ� �Լ�
	AM_MEDIA_TYPE* SetMediaTypeByIndex(int a_index);
	// �Ű� ������ �־��� ȭ�� ũ�⿡ �ش��ϴ� �̵�� ������ ��ġ�� �����ϴ� �Լ�
	AM_MEDIA_TYPE* SetMediaTypeBySize(int a_width, int a_height);
	// ���� �̸� ���� ��� �غ� �۾��� �����ϴ� �Լ�
	char SetPreviewMode(HWND ah_wnd, int a_x, int a_y, int a_cx, int a_cy, UINT a_msg_id = NOTIFY_ID);
	// ���� �̸� ���� �������� ũ�⸦ �����ϴ� �Լ�
	void SetVideoWndPos(int a_x, int a_y, int a_cx, int a_cy);
	// ���� �̸� ���⸦ �����ϴ� �Լ�
	char StartPreview(HWND ah_wnd, int a_x, int a_y, int a_cx, int a_cy);
	// ���� �̸� ���⸦ �����ϴ� �Լ�
	void StopPreview();
	// ��� ���� �̸� ���� ������ ĸó�ϴ� �Լ�
	void Capture(HDC ah_dest_dc);
	// ���� �̸� ���� �ý��ۿ� ����� CAM ��ġ�� ��� ���� �Ӽ� �����츦 �����ϴ� �Լ�
	void ShowCapturePinOption();
	// ���� �̸� ���� �ý��ۿ� ����� CAM ��ġ�� �ϵ���� �Ӽ� �����츦 �����ϴ� �Լ�
	void ShowCaptureFilterOption();
	// Filter Graph�� ������ ���ͺ��� �Ű� ������ �Ѱ� ���� ������ ���� ���ͱ����� ��� ���͵��� �����ϴ� �Լ�
	void RemoveDownStream(IBaseFilter* ap_filter);
	// ���� �̸� ���� ��� ���θ� ��ȯ�ϴ� �Լ�
	inline UINT8 IsCamPreviewRunning()
	{
		return m_preview_flag == 2;
	}
	// ���� ������ �̵�� ������ ��ȯ�ϴ� �Լ�
	inline AM_MEDIA_TYPE* GetMediaType()
	{
		return mp_media_type;
	}
	// �̸� ���� ������ ȭ�� ũ�⸦ ��ȯ�ϴ� �Լ�
	inline SIZE* GetPreviewSize()
	{
		return &m_preview_size;
	}
	// ���� �̸� ���� �������� �ڵ��� ��ȯ�ϴ� �Լ�
	inline HWND GetPreviewHwnd()
	{
		return mh_preview_wnd;
	}
	// ���� ���� ���� �������̽��� ��ȯ�ϴ� �Լ�
	inline IAMStreamConfig* GetStreamConfig()
	{
		return mp_stream_config;
	}
};

#endif // !_TXYC_PREVIEW_BUILDER_H_
