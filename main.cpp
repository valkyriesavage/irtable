#include "ofMain.h"
#include "ofAppGlutWindow.h"

#include "testApp.h"

//========================================================================
int main( ){

    ofAppGlutWindow window;
	ofSetupOpenGL(&window, 1024, 768, OF_WINDOW);			// <-------- setup the GL context

	bool testing = false;
	bool debug = false;
    ofRunApp(new testApp(testing, debug));
}
