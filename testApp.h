#pragma once

#include <vector>
#include <math.h>

#include "ofMain.h"
#include "ofxOpenCv.h"

#define _USE_LIVE_VIDEO		// uncomment this to use a live camera
						    // otherwise, we'll use a movie file

class testApp : public ofBaseApp{

	public:
		testApp(bool testing, bool debug);

		enum Action { NONE, LEFT, RIGHT, UP, DOWN, PINCH, UNPINCH, SWIPE };

		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);

		bool swipe(float x, float y);

		double distanceBetween(ofxCvBlob blob1, ofxCvBlob blob2);
		Action determineWhatMoved(float* x, float* y, double* pinch);
		char* actionName(Action action);
		void calculatePhotoDisplayDimensions(float x, float y, double pinch);

        #ifdef _USE_LIVE_VIDEO
		  ofVideoGrabber 		vidGrabber;
		#else
		  ofVideoPlayer 		vidPlayer;
		#endif

        ofxCvColorImage			colorImg;
        ofxCvGrayscaleImage 	grayImage;
		ofxCvGrayscaleImage 	grayBg;
		ofxCvGrayscaleImage 	grayDiff;

		ofImage					photo;
		ofImage					photo2;
		bool					displayingImage1;
		ofRectangle				photoDisplayDimensions;

        ofxCvContourFinder 		contourFinder;
		vector<ofxCvBlob>		previousBlobs;

		int 					threshold;
		bool					bLearnBakground;
		bool					testing;
		bool					debug;

		time_t					timeSwiped;
};

