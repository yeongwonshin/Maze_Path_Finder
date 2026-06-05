#include "ofMain.h"
#include "ofApp.h"

// ============================================================
// main.cpp
// ------------------------------------------------------------
// OpenFrameworks 프로그램의 시작점입니다.
// 이 파일은 알고리즘 자체를 구현하지 않고, 창 크기와 렌더링 모드를
// 설정한 뒤 실제 로직을 담당하는 ofApp 객체를 실행합니다.
// ============================================================

int main() {
    // ofSetupOpenGL(width, height, mode)
    // - width, height : 프로그램 시작 시 생성할 창의 초기 크기입니다.
    // - OF_WINDOW     : 전체 화면이 아니라 일반 창 모드로 실행한다는 의미입니다.
    // 미로 영역과 우측 설명 패널을 동시에 보여주기 위해 1280x820으로 설정했습니다.
    ofSetupOpenGL(1280, 820, OF_WINDOW);

    // ofRunApp(new ofApp())
    // - OpenFrameworks의 이벤트 루프를 시작합니다.
    // - 이후 setup(), update(), draw(), keyPressed(), mousePressed() 등이
    //   프레임 또는 사용자 입력에 맞추어 자동 호출됩니다.
    ofRunApp(new ofApp());
}
