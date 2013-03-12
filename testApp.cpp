#include "testApp.h"

testApp::testApp(bool testing, bool debug) {
	this->testing = testing;
	this->debug = debug;
}

//--------------------------------------------------------------
void testApp::setup(){
	time(&timeSwiped);

	#ifdef _USE_LIVE_VIDEO
        vidGrabber.setVerbose(true);
		vidGrabber.setDeviceID(1);
        vidGrabber.initGrabber(320,240);
	#else
        vidPlayer.loadMovie("fingers.mov");
        vidPlayer.play();
	#endif

    colorImg.allocate(320,240);
	grayImage.allocate(320,240);
	grayBg.allocate(320,240);
	grayDiff.allocate(320,240);

	bLearnBakground = true;
	if(testing) {
		threshold = 180;
	} else {
		threshold = 18;
	}

	previousBlobs = vector<ofxCvBlob>(15);

	photo.loadImage("coffee.JPG");
	photo2.loadImage("carousel.JPG");
	displayingImage1 = true;
	photoDisplayDimensions.x = 0;
	photoDisplayDimensions.y = 0;
	photoDisplayDimensions.width = photo.width;
	photoDisplayDimensions.height = photo.height;
}

//--------------------------------------------------------------
void testApp::update(){
	ofBackground(100,100,100);

    bool bNewFrame = false;

	#ifdef _USE_LIVE_VIDEO
       vidGrabber.update();
	   bNewFrame = vidGrabber.isFrameNew();
    #else
        vidPlayer.update();
        bNewFrame = vidPlayer.isFrameNew();
	#endif

	if (bNewFrame){

		#ifdef _USE_LIVE_VIDEO
            colorImg.setFromPixels(vidGrabber.getPixels(), 320,240);
	    #else
            colorImg.setFromPixels(vidPlayer.getPixels(), 320,240);
        #endif

		grayImage = colorImg;

		if(testing) {
			// we don't want to bgsubtract; we will just look for the brightest things for testing
			grayDiff = grayImage;
			grayDiff.threshold(threshold);
		} else {
			if (bLearnBakground == true){
				grayBg = grayImage;		// the = sign copys the pixels from grayImage into grayBg (operator overloading)
				bLearnBakground = false;
			}

			// take the abs value of the difference between background and incoming and then threshold:
			grayDiff.absDiff(grayBg, grayImage);
			grayDiff.threshold(threshold);
		}

		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
		previousBlobs = contourFinder.blobs;
		if (testing) {
			contourFinder.findContours(grayDiff, 20, (340*240)/3, 2, true);	// find holes
		} else {
			contourFinder.findContours(grayDiff, 20, (340*240)/3, 10, true);	// find holes
		}
	}

}

//--------------------------------------------------------------
void testApp::draw(){
	// figure out what the user did
	float x=0, y=0;
	double pinch=0;
	testApp::Action detected = determineWhatMoved(&x, &y, &pinch);

	if(debug) {
		// draw the incoming, the grayscale, the bg and the thresholded difference
		ofSetHexColor(0xffffff);
		colorImg.draw(20,20);
		grayImage.draw(360,20);
		grayBg.draw(20,280);
		grayDiff.draw(360,280);

		// then draw the contours:
		ofFill();
		ofSetHexColor(0x333333);
		ofRect(360,540,320,240);
		ofSetHexColor(0xffffff);
		contourFinder.draw(360,540);

		// finally, a report:
		ofSetHexColor(0xffffff);
		
		char detectionStr[512];
		sprintf(detectionStr, "\n%s x:%d y:%d pinch:%f\n", actionName(detected), x, y, pinch);
		char reportStr[1024];
		sprintf(reportStr, "bg subtraction and blob detection\npress ' ' to capture bg\nthreshold %i (press: +/-)\nnum blobs found %i, fps: %f%s", threshold, contourFinder.nBlobs, ofGetFrameRate(), detectionStr);
		ofDrawBitmapString(reportStr, 20, 600);
	} else {
		calculatePhotoDisplayDimensions(x, y, pinch);
		if (displayingImage1) {
			photo.draw(photoDisplayDimensions.x, photoDisplayDimensions.y, photoDisplayDimensions.width, photoDisplayDimensions.height);
		} else {
			photo2.draw(photoDisplayDimensions.x, photoDisplayDimensions.y, photoDisplayDimensions.width, photoDisplayDimensions.height);
		}
		ofDrawBitmapString(actionName(detected), 20, 20);
	}
}

double testApp::distanceBetween(ofxCvBlob blob1, ofxCvBlob blob2) {
	ofPoint centre1 = blob1.centroid;
	ofPoint centre2 = blob2.centroid;
	return sqrt(pow((centre1.x - centre2.x),2) + pow((centre1.y - centre2.y),2));
}

bool testApp::swipe(float x, float y) {
	return (x > 5) || (x < -5) || (y > 5) || (y < -5);
}

testApp::Action testApp::determineWhatMoved(float* x, float* y, double* pinch) {
	if( contourFinder.blobs.size() > 2 || contourFinder.blobs.size() == 0 || previousBlobs.size() == 0) {
		return testApp::NONE;
	}
	if (contourFinder.blobs.size() == 2 && previousBlobs.size() >= 2) {
		ofxCvBlob finger = contourFinder.blobs.at(0);
		ofxCvBlob previousFinger = previousBlobs.at(0);
		ofxCvBlob otherFinger = contourFinder.blobs.at(1);
		ofxCvBlob previousOtherFinger = previousBlobs.at(1);

		double previousDistance = distanceBetween(previousFinger, previousOtherFinger);
		double currentDistance = distanceBetween(finger, otherFinger);
		*pinch = previousDistance - currentDistance;

		if(previousDistance > currentDistance) {
			return testApp::PINCH;
		}
		if(previousDistance < currentDistance) {
			return testApp::UNPINCH;
		}
	} else {
		ofxCvBlob finger = contourFinder.blobs.at(0);
		ofxCvBlob previousFinger = previousBlobs.at(0);

		*x = finger.centroid.x - previousFinger.centroid.x;
		*y = finger.centroid.y - previousFinger.centroid.y;

		if(swipe(*x, *y)) return testApp::SWIPE;
		if(*x > 0) return testApp::RIGHT;
		if(*x < 0) return testApp::LEFT;
		if(*y > 0) return testApp::UP;
		if(*y < 0) return testApp::DOWN;
	}
	return testApp::NONE;
}

char* testApp::actionName(testApp::Action action) {
	switch(action) {
	case testApp::NONE:
		return "none";
		break;
	case testApp::LEFT:
		return "left";
		break;
	case testApp::RIGHT:
		return "right";
		break;
	case testApp::UP:
		return "up";
		break;
	case testApp::DOWN:
		return "down";
		break;
	case testApp::PINCH:
		return "pinch";
		break;
	case testApp::UNPINCH:
		return "unpinch";
		break;
	case testApp::SWIPE:
		return "swipe";
		break;
	default:
		return "WTF";
		break;
	}
}

void testApp::calculatePhotoDisplayDimensions(float x, float y, double pinch) {
	// need to determine if we are swiping, show other image
	if(swipe(x, y)) {
		time_t now;
		time(&now);
		if(difftime(now, timeSwiped) < 1) {
			return;
		}
		displayingImage1 = !displayingImage1;
		photoDisplayDimensions.x = 0;
		photoDisplayDimensions.y = 0;
		if(displayingImage1) {
			photoDisplayDimensions.width = photo.width;
			photoDisplayDimensions.height = photo.height;
		} else {
			photoDisplayDimensions.width = photo2.width;
			photoDisplayDimensions.height = photo2.height;
		}
		time(&timeSwiped);
	}
	if(pinch == 0) {
		photoDisplayDimensions.x += x;
		photoDisplayDimensions.y += y;
	} else {
		// note that a negative pinch means scale up, and a positive pinch means scale down
		photoDisplayDimensions.scaleFromCenter(1-(pinch/photoDisplayDimensions.width));
	}
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

	switch (key){
		case ' ':
			bLearnBakground = true;
			break;
		case '+':
			threshold ++;
			if (threshold > 255) threshold = 255;
			break;
		case '-':
			threshold --;
			if (threshold < 0) threshold = 0;
			break;
		case 'd':
			debug = !debug;
			break;
	}
}