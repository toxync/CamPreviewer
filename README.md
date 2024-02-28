# CamPreviewer

MFC와 DirectShow로 제작된 미리보기 동영상 영상처리 소프트웨어입니다.

## 핵심 기능 요약

* **컴퓨터에 연결된 카메라의 미리보기 동영상 촬영**
* **촬영중인 미리보기 동영상에서 색상이나 패턴을 추적**
* **미리보기 동영상이나 카메라의 설정값 변경**

## 기능 설명

### 촬영중인 미리보기 동영상에서 색상 추적

* 촬영중인 미리보기 동영상에서 R, G, B 중 하나의 색상을 추적합니다.
* '추적 색상' 라디오 버튼 그룹의 선택 상태에 따라 촬영중인 미리보기 동영상에서 추적할 색상이 변경됩니다.

![ColorTraceTest](https://raw.githubusercontent.com/toxync/CamPreviewer/master/demo_images/color_trace_test.gif)

### 촬영중인 미리보기 동영상에서 패턴 추적

* 촬영중인 미리보기 동영상의 테두리 패턴을 추적합니다.
* '추적 패턴' 라디오 버튼 그룹의 선택 상태에 따라 촬영중인 미리보기 동영상의 테두리 패턴 검출 방식이 변경됩니다.

![PatternTraceTest](https://raw.githubusercontent.com/toxync/CamPreviewer/master/demo_images/pattern_trace_test.gif)

### 미리보기 동영상의 설정값 변경

* '기본 영상 설절' 버튼을 클릭하면 촬영중인 미리보기 동영상의 설정값을 변경할 수 있습니다.
* 미리보기 동영상의 설정값 변경은 미리보기 동영상이 촬영중인 경우에만 가능합니다.

![PinOptionTest](https://raw.githubusercontent.com/toxync/CamPreviewer/master/demo_images/pin_option_test.gif)

### 카메라 설정값 변경

* '고급 영상 설정' 버튼을 클릭하면 미리보기 영상을 촬영하는 카메라의 설정값을 변경할 수 있습니다.
* 카메라 설정값 변경은 미리보기 동영상이 촬영중인 경우에만 가능합니다.

![FilterOptionTest](https://raw.githubusercontent.com/toxync/CamPreviewer/master/demo_images/filter_option_test.gif)
