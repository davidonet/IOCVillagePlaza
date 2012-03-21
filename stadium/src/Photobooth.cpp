//
//  Photobooth.cpp
//  IOC_Stadium
//
//  Created by Douglas Stanley on 03|03|2012.
//  Copyright (c) 2012 www.abstractmachine.net. All rights reserved.
//

#include "Photobooth.h"
#include "Personae.h"

// function prototypes
/*
 void updateProjectionState();
 ofVec3f ofWorldToScreen(ofVec3f world);
 ofMesh getProjectedMesh(const ofMesh& mesh);
 template <class T> void addTexCoords(ofMesh& to, const vector<T>& from);
 */

// MARK: OpenCV

using namespace ofxCv;
using namespace cv;


// MARK: Init

void Photobooth::setup() {
    
    visible = false;
    present = false;
    
    resetTimer(lastPresence);
    resetTimer(lastAbsence);
    resetTimer(lastTilt);
    resetTimer(lastStateChange);
    
    tiltAngle = 0;
    photoCount = 0;
    
    cameraVisible = false;
    stateVisible = true;
    
    kinected = false;
    
    setupOpenNI();
    setupFaceTracking();
    
    changeState(PHOTO_IDLE);
    
    athlete.loadImage("personae/athlete.png");
    
}


void Photobooth::setupOpenNI() {
    
    ofxXmlSettings xml;
    xml.loadFile("openni/settings.xml");
    
    nearThreshold = xml.getAttribute("openni:depth:near", "value", 100);
    farThreshold  = xml.getAttribute("openni:depth:far" , "value", 1500);
    
	filterFactor = xml.getAttribute("openni:filter" , "value", 0.2f);
    
	contours.setMinAreaRadius(xml.getAttribute("openni:contour:areaRadius:min" , "value", 10));
	contours.setMaxAreaRadius(xml.getAttribute("openni:contour:areaRadius:max" , "value", 150));
    
    contourThreshold = xml.getAttribute("openni:contour:threshold", "value", 127);
    contours.setThreshold(contourThreshold);
    
    hardware.setup();                   // only working on Mac/Linux at the moment (but on Linux you need to run as sudo...)
	hardware.setLedOption(LED_GREEN);   // LED_OFF, LED_GREEN, LED_RED, LED_YELLOW, LED_BLINK_GREEN, LED_BLINK_RED_YELLOW
    hardware.setTiltAngle(tiltAngle);
    
    context.setup();	// all nodes created by code -> NOT using the xml config file at all
                        //recordContext.setupUsingXMLFile();
	depth.setup(&context);
    depth.setDepthColoring(COLORING_GREY);
    
	image.setup(&context);
    
	user.setup(&context);
	user.setSmoothing(filterFactor);				// built in openni skeleton smoothing...
	user.setUseMaskPixels(true);
	user.setUseCloudPoints(false);
	user.setMaxNumberOfUsers(1);					// use this to set dynamic max number of users (cf. MAX_NUMBER_USERS in ofxUserGenerator)
    
	context.toggleRegisterViewport();
	context.toggleMirror();
    
    kinected = true;
    
}


void Photobooth::setupFaceTracking() {
    
    faceTracker.setup();
    foundFace = false;
    contourRectangle = ofRectangle(0,0,0,0);
    contourImage.allocate(depth.getWidth(), depth.getHeight(), OF_IMAGE_COLOR);
    
}


void Photobooth::exit() {
    
    context.shutdown();
    hardware.shutDown();
    
}


// MARK: Loop

void Photobooth::update() {
    
    // if we haven't instantiated the kinect yet, get outta here
    if (!kinected) return;
    
    // if we're hidden, get outta here
    if (!visible) return;
    
#ifdef TARGET_OSX // only working on Mac at the moment
	hardware.update();
#endif
    
    // reset face rectangle
    contourRectangle = ofRectangle(0,0,0,0);
    
    // update main context (Kinect)
    context.update();
    // update depth image (all values)
    depth.update();
    // update RGB image
    image.update();
    // use near/far to mask out unwanted depth
    depthRangeMask.setFromPixels(depth.getDepthPixels(nearThreshold, farThreshold), depth.getWidth(), depth.getHeight(), OF_IMAGE_GRAYSCALE);
    
    // check to see if someone's standing in front of the booth
    detectPresence();
    // scroll camera up to meet their gaze
    adjustTilt();
    
    updateState();
    
}


void Photobooth::updateState() {
    
    
    // depending on state
    switch (state) {
            
        case PHOTO_IDLE:
            
            // check to see if we need to move to next state
            if (present) changeState(PHOTO_HELLO);
            break;
            
        case PHOTO_HELLO:
            
            // check to see if there's no one
            if (!present) changeState(PHOTO_IDLE);
            // if enough time has gone by, move to paparazzi state
            if (timeSince(lastStateChange) > PAPARAZZI_DELAY) changeState(PHOTO_PAPARAZZI);
            break;
            
        case PHOTO_PAPARAZZI:
            
            // try to find a face
            foundFace = detectFace();
            // check to see if there's no one
            if (!foundFace && !present) changeState(PHOTO_IDLE);
            // if enough time has gone off, take a picture
            if ( foundFace && timeSince(lastStateChange) > PHOTO_DELAY) changeState(PHOTO_CLICHE);
            // if we couldn't find a face, and too much time has gone by
            if (!foundFace && timeSince(lastStateChange) > PHOTO_ERROR_DELAY) changeState(PHOTO_ERROR);
            
            break;
            
        case PHOTO_CLICHE:
            
            // try to find a face
            foundFace = detectFace();
            // check to see if there's no one
            if (!foundFace && !present) changeState(PHOTO_IDLE);
            // error checking (rare possibility)
            // if we didn't find a face, forget the rest
            if (!foundFace) {
                changeState(PHOTO_PAPARAZZI);
                break;
            }
            // take a picture
            rememberFace();
            // if we've taken enough pictures, go to the slideshow
            if (++photoCount > PHOTO_COUNT) changeState(PHOTO_SLIDESHOW);
            // otherwise, take another picture
            else /*if (detectFace())*/ changeState(PHOTO_PAPARAZZI);
            break;
            
        case PHOTO_ERROR:
            
            // check to see if there's no one
            if (!present) changeState(PHOTO_IDLE);
            // we won't take many pictures
            if (photoCount < PHOTO_COUNT-2) photoCount = PHOTO_COUNT-2;
            // ok, take the picture
            changeState(PHOTO_CLICHE);
            break;
            
        case PHOTO_SLIDESHOW:
            
            // just wait here
            break;
            
        case PHOTO_POSTURE:
        case PHOTO_PLAY:
            
            // update tracking/recording nodes
            user.update();
            // demo getting pixels from user gen
            //userMask.setFromPixels(user.getUserPixels(), user.getWidth(), user.getHeight(), OF_IMAGE_GRAYSCALE);
            break;
            
        default:
            break;
    }
    
}



// MARK: Behaviors

void Photobooth::detectPresence() {
    
    contours.findContours(depthRangeMask);
    
    // if someone is present
    if (contours.size() > 0) {
        // what's the topmost rectangle
        findTopRectangle();
        // if it's been longer than a second
        if (timeSince(lastAbsence) > ABSENCE_DELAY) {
            // ok, we're present
            present = true;
            // update the presence timer
            resetTimer(lastPresence);
        }
    } else { // no one's detected
             // if it's been longer than a second
        if (timeSince(lastPresence) > PRESENCE_DELAY) {
            // ok, that's long enough to call it an absence
            present = false;
            // record the absence timer
            resetTimer(lastAbsence);
        }
    }
    
}


bool Photobooth::detectFace() {
    
    // reset face tracking back to unfound
    foundFace = false;
    
    // if there aren't any contours
    if (!contours.size()) return false;
    
    // if there's no rect, get outta here
    if (!contourRectangle.width) return false;
    
    // we only need half the height (the head is on top)
    float headProportion = 0.6f; // this might need to be adjusted
    float h = (int)(contourRectangle.height * headProportion);
    ofRectangle captureRectangle = ofRectangle(contourRectangle.x, contourRectangle.y, contourRectangle.width, h);
    
    contourImage.resize(captureRectangle.width, captureRectangle.height);
    
    unsigned char * sourcePixels = image.getPixels();
    unsigned char * destinationPixels = contourImage.getPixels();
    
    int w = depth.getWidth();
    
    for(int y=0; y<captureRectangle.height; y++) {
        for(int x=0; x<captureRectangle.width; x++) {
            // figure out where to copy to/from
            int destinationIndex = (x + (y * captureRectangle.width)) * 3;
            int sourceIndex = ((x+captureRectangle.x) + ((y+captureRectangle.y) * w)) * 3;
            // copying all three colors
            for(int i=0; i<3; i++) {
                destinationPixels[destinationIndex+i] = sourcePixels[sourceIndex+i];
            }
        }
    }
    
    contourImage.update();
    
    // track face
    faceTracker.update(toCv(contourImage));
    
    // did we find a face?
    foundFace = faceTracker.getFound();
    
    return foundFace;
    
}


void Photobooth::eraseFaces() {
    
    // remove faces from vector<ofImage>
    faces.clear();
    
}


// FIXME: fix face extraction (currently eyes+mouth = holes)

void Photobooth::rememberFace() {
    
    ofPushStyle();
    
    //ofMesh imageMesh = faceTracker.getImageMesh();
    //ofMesh objectMesh = faceTracker.getObjectMesh();
    ofMesh meanMesh = faceTracker.getMeanObjectMesh();
    
    ofPolyline faceOutlinePolyline = faceTracker.getMeanObjectFeature(ofxFaceTracker::FACE_OUTLINE);
    ofMesh faceOutlineMesh;
    ofTessellator tessellator;
    tessellator.tessellateToMesh(faceOutlinePolyline, OF_POLY_WINDING_POSITIVE, faceOutlineMesh);
    
    // to create a transparent texture, we need to provide this spectific settings
    // merci à Pierre Rossel ;-)
    ofFbo::Settings settings;  
    settings.width = 200;  
    settings.height = 200;  
    settings.internalformat = GL_RGBA;  
    settings.numSamples = 0;  
    settings.useDepth = false;  
    settings.useStencil = false; 
    
    ofFbo fbo;
    fbo.allocate(settings);
    fbo.begin();
    ofClear(0, 0, 0);               // clear out previous garbage
    
    
    ofPushMatrix();
    // ofSetupScreenOrtho(640, 480, OF_ORIENTATION_DEFAULT, true, -1000, 1000);
    ofSetupScreenOrtho(200, 200, OF_ORIENTATION_DEFAULT, false, -1000, 1000);
    ofTranslate(100, 100);
    ofScale(4,4,4);
    ofSetColor(0, 0, 0);
    faceOutlineMesh.draw();
    contourImage.getTextureReference().bind();
    //objectMesh.draw();
    ofSetColor(255,255,255);
    meanMesh.draw();
    //imageMesh.draw();
    contourImage.getTextureReference().unbind();
    
    
    ofPopMatrix();
    
    fbo.end();
    
    ofPixels pixels;
    fbo.readToPixels(pixels);
    
    faces.push_back(Face());
    faces.back().image = pixels;
    faces.back().mesh  = meanMesh;
    
    ofPopStyle();
    
    faceTracker.reset();
}



void Photobooth::drawFaceIntoFbo(ofMesh &mesh, ofImage &image) {
    
    
    
}


void Photobooth::findTopRectangle() {
    
    // find topmost contour
    contourRectangle = ofRectangle(0,666,0,0);
    // go through all the contours
    for(int i=0; i<contours.size(); i++) {
        // get this area
        cv::Rect thisRectangle = contours.getBoundingRect(i);
        // compare it
        if (thisRectangle.y >= contourRectangle.y) continue;
        // ok, it's bigger
        contourRectangle = ofRectangle(thisRectangle.x,thisRectangle.y,thisRectangle.width,thisRectangle.height);
    }
    
}


void Photobooth::adjustTilt() {
    
    if (!contours.size()) return;
    
    // if for some reason we don't have any contours, get outta here
    if (!contourRectangle.width) return;
    
    // figure out the percentage of this top to the total height
    float percent = contourRectangle.y / (float)depth.getHeight();
    
    // if we've waiting enough since the last tilt
    if (timeSince(lastTilt) > TILT_DELAY) {
        
        // are we too high?
        if (percent < 0.1) {
            setTiltAngle(tiltAngle+1);
        }
        // or are we too low?
        else if (percent > 0.2) {
            setTiltAngle(tiltAngle-1);
        }
        
    }
    
}

void Photobooth::setTiltAngle(int newAngle) {
    
    tiltAngle = min(MAX_TILT,max(MIN_TILT,newAngle));
    hardware.setTiltAngle(tiltAngle);
    resetTimer(lastTilt);
    
}



// MARK: state machine

void Photobooth::changeState(photobooth_state newState) {
    
    switch (newState) {
            
        case PHOTO_IDLE:        // waiting for someone to come
            
            break;
            
        case PHOTO_HELLO:       // saying hello to someone
                                // reset photo count back down to zero (so we can start taking pictures again)
            photoCount = 0;
            // erase all the previous faces
            eraseFaces();
            break;
            
        case PHOTO_PAPARAZZI:   // get into position to start taking photos
            
            break;
            
        case PHOTO_CLICHE:      // flashbulbs going off
            
            break;
            
        default:
            break;
    }
    
    state = newState;
    resetTimer(lastStateChange);
    
}


void Photobooth::toggleVisibility() {
    
    visible = !visible;
    // whenever we change the visibility, we also set the state back to idle
    changeState(PHOTO_IDLE);
    
}



// MARK: Draw

void Photobooth::draw() {
    
    // if we haven't instantiated the kinect yet, get outta here
    if (!kinected) return;
    
    // if we're hidden, get outta here
    if (!visible) return;
    
    if (stateVisible) drawState();
    
    //image.draw(0,0);
    
    // according to idle
    switch (state) {
            
        case PHOTO_IDLE : 
            drawContours();   
            break;
            
        case PHOTO_HELLO : 
            drawContours();   
            break;
            
        case PHOTO_PAPARAZZI : 
            drawContours();   
            drawFace(); 
            drawSlideshow();      
            break;
            
        case PHOTO_CLICHE : 
            drawContours();   
            drawFace();  
            drawSlideshow();     
            break;
            
        case PHOTO_SLIDESHOW:
            drawSlideshow();
            break;
            
        default: 
            break;
            
    }
    
    /*
     
     // draw skeleton
     if (user.getNumberOfTrackedUsers() > 0) user.draw();
     
     // draw tracker
     if(faceTracker.getFound()) {
     ofPolyline polyline = faceTracker.getImageFeature(ofxFaceTracker::FACE_OUTLINE);
     polyline.draw();
     }
     
     */
    
}


void Photobooth::drawState() {
    
    if (!stateVisible) return;
    
    string str = "PHOTO_IDLE\nPHOTO_HELLO\nPHOTO_LANGUAGE_SELECT\nPHOTO_LANGUAGE_SELECTED\nPHOTO_BADGE_REQUEST\nPHOTO_PLEASE_ID\nPHOTO_PAPARAZZI\nPHOTO_CLICHE\nPHOTO_ERROR\nPHOTO_SLIDESHOW\nPHOTO_CLICHE_SELECTED\nPHOTO_POSTURE\nPHOTO_PLAY\nPHOTO_ABORT";
    
    ofPushMatrix();
    ofTranslate(700,20);
    
    ofSetColor(0,0,0);
    ofDrawBitmapString(str, ofPoint(0,0));
    
    ofSetColor(255,0,0);
    int textY = (int)(state*13.5);
    ofTranslate(-5, -5+textY);
    ofSetLineWidth(1);
    ofDisableSmoothing();
    ofLine(-3,-3,0, 0);
    ofLine(-6, 0,1, 0);
    ofLine(-3, 4,0, 1);
    ofPopMatrix();
    
}


void Photobooth::drawFace() {
    
    if (!cameraVisible) return;
    
    // if no faces
    if (!contours.size()) return;
    
    contourImage.draw(contourRectangle.x, contourRectangle.y);
    
    ofPushMatrix();
    ofTranslate(contourRectangle.x, contourRectangle.y);
    faceTracker.draw(true);
    
    ofPopMatrix();
    
}


void Photobooth::drawContours() {
    
    if (!cameraVisible) return;
    
    ofSetColor(255,255,255);
    depthRangeMask.draw(0,0);
    contours.draw();
    
}


void Photobooth::drawSlideshow() {
    
    int step = ofGetWidth() / PHOTO_COUNT;
    
    ofPushMatrix();
    ofTranslate(0, ofGetHeight()-250);
    
    for(int i=0; i<faces.size(); i++) {
        
        int x = i*step + 50;
        int y = 0;
        
        ofSetColor(255, 255, 255);
        athlete.draw(x,y+75);
        faces[i].image.draw(x+15,y-10,100,100);
        
    }
    
    ofPopMatrix();
    
}



// MARK: Interaction

void Photobooth::mousePressed(int x, int y, int button) {
    
    // if we haven't instantiated the kinect yet, get outta here
    if (!kinected) return;
    
    int step = ofGetWidth() / PHOTO_COUNT;
    
    if (PHOTO_SLIDESHOW == state) {
        
        // if we've selected one of the faces
        if (y > ofGetHeight()-250) {
            
            int xIndex = x / step;
            // ok, we selected one
            if (0 < faces.size() && xIndex < faces.size()) {
                Personae::Instance().addFace( faces[xIndex] ); 
            } 
            
            changeState(PHOTO_PLAY);
            
        } else {
            
            changeState(PHOTO_IDLE);
            
        }
        
    }
    
}




/////
/*
 // from ProCamToolkit
 GLdouble modelviewMatrix[16], projectionMatrix[16];
 GLint viewport[4];
 
 void updateProjectionState() {
 glGetDoublev(GL_MODELVIEW_MATRIX, modelviewMatrix);
 glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
 glGetIntegerv(GL_VIEWPORT, viewport);
 }
 
 ofVec3f ofWorldToScreen(ofVec3f world) {
 updateProjectionState();
 GLdouble x, y, z;
 gluProject(world.x, world.y, world.z, modelviewMatrix, projectionMatrix, viewport, &x, &y, &z);
 ofVec3f screen(x, y, z);
 screen.y = ofGetHeight() - screen.y;
 return screen;
 }
 
 ofMesh getProjectedMesh(const ofMesh& mesh) {
 ofMesh projected = mesh;
 for(int i = 0; i < mesh.getNumVertices(); i++) {
 ofVec3f cur = ofWorldToScreen(mesh.getVerticesPointer()[i]);
 cur.z = 0;
 projected.setVertex(i, cur);
 }
 return projected;
 }
 
 template <class T>
 void addTexCoords(ofMesh& to, const vector<T>& from) {
 for(int i = 0; i < from.size(); i++) {
 to.addTexCoord(from[i]);
 }
 }
 */
