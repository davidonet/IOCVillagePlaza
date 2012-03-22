//
//  Animation.h
//  IOC_Stadium
//
//  Created by Douglas Stanley on 03|03|2012.
//  Copyright (c) 2012 www.abstractmachine.net. All rights reserved.
//

#pragma once

#include "ofMain.h"
#include "defines.h"

#include "ofxXmlSettings.h"

// Keyframes define positions for the faces
struct Keyframe {
    Keyframe(float _x, float _y, float _t) : x(_x), y(_y), time(_t) {};
    double x,y,time;  
};

struct Players {
    vector<Keyframe> keyframes;
};

struct Scenario {
    string              name;
    ofVec2f             ratio;
    ofRectangle         scale;
    ofRectangle         rectangle;
    ofVideoPlayer       movie;
    vector<Players>     players;
};

// Animation defines movie playback + keyframe superposition
class Animation {
    
public:
    
    // Init
    void setup();
    
    // Loop
    void update();
    void draw();
    
    // visibility
    bool visible;
    bool isVisible() { return visible; }
    void setVisibility(bool _visible);
    
    // timer
    void play();
    void pause();
    void rewind();
    
    // movies
    int current;
    vector<Scenario> scenarios;
    void loadScenarios();
    void select(string _name);
    void select(int index);
    void loadKeyframes(int index);
    
    // Interaction
    ofVec2f             window;
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    
    //
    bool showMesh;
    
};

