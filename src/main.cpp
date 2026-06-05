#include "ofMain.h"
#include "ofApp.h"

// main.cpp
// OpenFrameworks application entry point.
// The window size is fixed large enough to show the maze, statistics panel,
// and keyboard guide at the same time during the presentation video.
int main() {
    ofSetupOpenGL(1280, 820, OF_WINDOW);
    ofRunApp(new ofApp());
}
