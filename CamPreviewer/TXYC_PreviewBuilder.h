#ifndef _TXYC_PREVIEW_BUILDER_H_
#define _TXYC_PREVIEW_BUILDER_H_

// 클래스 작성자: 박정현(bac6536@naver.com)
// 이 클래스는 다른 개발자가 작성한 클래스를 기반으로 작성되었음
// 원본 클래스 작성자: 김성엽(tipsware@naver.com)
// 원본 클래스 배포 주소: https://cafe.naver.com/mythread

#include <dshow.h>

// 영상 미리 보기 윈도우와 부모 윈도우 사이의 통신에 사용할 메시지 번호
#define NOTIFY_ID 12000

// 미디어 정보를 삭제하는 함수
void TXYC_DeleteMediaType(AM_MEDIA_TYPE* ap_media_type, UINT8 a_self_delete = 1);

class TXYC_PreviewBuilder
{
private:
	// 영상 미리 보기 시스템 Filter Graph의 Filter Graph Builder(Filter Graph 생성용 객체)
	ICaptureGraphBuilder2* mp_graph_builder = NULL;
	// 영상 미리 보기 시스템 Filter Graph의 Filter Graph Manager(Filter Graph 관리용 객체)
	IGraphBuilder* mp_filter_graph = NULL;
	// 영상 미리 보기 필터(CAM 장치가 촬영중인 영상을 얻어오는 기능) 인터페이스
	IBaseFilter* mp_video_capture = NULL;
	IAMVideoCompression* mp_video_comp = NULL; // 영상 압축 정보 인터페이스
	IAMStreamConfig* mp_stream_config = NULL; // CAM 장치의 설정 정보 관리 인터페이스
	IVideoWindow* mp_video_window = NULL; // 영상 미리 보기 윈도우 인터페이스
	IBasicVideo* mp_basic_video = NULL; // CAM 장치가 제공하는 영상의 기본 영상 인터페이스
	// 미리 보기 영상을 출력하는 과정에서 발생한 이벤트를 사용하기 위한 인터페이스
	IMediaEventEx* mp_media_event = NULL;
	AM_MEDIA_TYPE* mp_media_type = NULL; // 영상과 관련된 미디어 정보
	// 영상 미리 보기 윈도우의 부모 윈도우 핸들과 영상 미리 보기 윈도우의 핸들
	HWND mh_owner_wnd = NULL, mh_preview_wnd = NULL;
	SIZE m_preview_size = { 300,200 }; // 미리 보기 영상의 화면 크기
	UINT m_notify_id = NOTIFY_ID; // 미리 보기 영상의 상태 변경에 사용할 메시지 ID
	int m_preview_x = 0, m_preview_y = 0; // 미리 보기 영상의 출력 시작 좌표(x, y)
	// 미리 보기 영상의 재생 상태(-1: 오류, 0: 미리 보기 준비 안 됨, 1: 미리 보기 중지됨, 2: 미리 보기 재생중)
	char m_preview_flag = 0;
public:
	TXYC_PreviewBuilder(IMoniker* ap_video_dev, const wchar_t* ap_video_dev_name);
	~TXYC_PreviewBuilder();
	// 미리 보기 영상의 화면 크기를 구하는 함수
	// (미디어 정보가 없으면 -1, 미디어 정보 형식이 잘못됐으면 0, 정상적으로 수행됐으면 1을 반환)
	char GetMediaSize(SIZE* ap_size);
	// 매개 변수로 주어진 인덱스에 위치한 미디어 정보를 장치에 설정하는 함수
	AM_MEDIA_TYPE* SetMediaTypeByIndex(int a_index);
	// 매개 변수로 주어진 화면 크기에 해당하는 미디어 정보를 장치에 설정하는 함수
	AM_MEDIA_TYPE* SetMediaTypeBySize(int a_width, int a_height);
	// 영상 미리 보기 재생 준비 작업을 수행하는 함수
	char SetPreviewMode(HWND ah_wnd, int a_x, int a_y, int a_cx, int a_cy, UINT a_msg_id = NOTIFY_ID);
	// 영상 미리 보기 윈도우의 크기를 변경하는 함수
	void SetVideoWndPos(int a_x, int a_y, int a_cx, int a_cy);
	// 영상 미리 보기를 실행하는 함수
	char StartPreview(HWND ah_wnd, int a_x, int a_y, int a_cx, int a_cy);
	// 영상 미리 보기를 중지하는 함수
	void StopPreview();
	// 재생 중인 미리 보기 영상을 캡처하는 함수
	void Capture(HDC ah_dest_dc);
	// 영상 미리 보기 시스템에 연결된 CAM 장치의 출력 영상 속성 윈도우를 생성하는 함수
	void ShowCapturePinOption();
	// 영상 미리 보기 시스템에 연결된 CAM 장치의 하드웨어 속성 윈도우를 생성하는 함수
	void ShowCaptureFilterOption();
	// Filter Graph의 마지막 필터부터 매개 변수로 넘겨 받은 필터의 다음 필터까지의 모든 필터들을 제거하는 함수
	void RemoveDownStream(IBaseFilter* ap_filter);
	// 영상 미리 보기 재생 여부를 반환하는 함수
	inline UINT8 IsCamPreviewRunning()
	{
		return m_preview_flag == 2;
	}
	// 현재 설정된 미디어 정보를 반환하는 함수
	inline AM_MEDIA_TYPE* GetMediaType()
	{
		return mp_media_type;
	}
	// 미리 보기 영상의 화면 크기를 반환하는 함수
	inline SIZE* GetPreviewSize()
	{
		return &m_preview_size;
	}
	// 영상 미리 보기 윈도우의 핸들을 반환하는 함수
	inline HWND GetPreviewHwnd()
	{
		return mh_preview_wnd;
	}
	// 설정 정보 관리 인터페이스를 반환하는 함수
	inline IAMStreamConfig* GetStreamConfig()
	{
		return mp_stream_config;
	}
};

#endif // !_TXYC_PREVIEW_BUILDER_H_
