//
//  OCV.h
//  ShadowingStage1
//
//  Created by David Haylock on 28/07/2014.
//
//

//#ifndef __ShadowingStage1__OCV__
//#define __ShadowingStage1__OCV__

#include <iostream>
#include "ofMain.h"
#include "ofxOpenCv.h"

#define DEBUG

class CV {
    
public:
    void setup(int width, int height, int framerate);
    
    // Specifically the PS3 eye needs releasing to ensure the locking mechanism is unlocked
    void releaseCamera();
    
    // Loop with Brightness and Contrast
    void subtractionLoop(bool bLearnBackground, bool useProgressiveLearn,float progressionRate,bool mirrorH,bool mirrorV,int threshold,int blur,int minBlobSize, int maxBlobSize,int maxBlobNum,bool fillHoles, bool useApproximation, float brightness,float contrast,bool erode,bool dilate);
    
    // Loop without Brightness and Contrast
    void subtractionLoop(bool bLearnBackground, bool useProgressiveLearn,float progressionRate,bool mirrorH,bool mirrorV,int threshold,int blur,int minBlobSize, int maxBlobSize,int maxBlobNum,bool fillHoles, bool useApproximation,bool erode,bool dilate);
    
    // John's Loop
    void subtractionLoop(bool bLearnBackground,bool mirrorH,bool mirrorV,int threshold, int blur,int minBlobSize, int maxBlobSize,int maxBlobNum,bool fillHoles, bool useApproximation,float brightness,float contrast);
    
    void readAndWriteBlobData(ofColor backgroundColor,ofColor shadowColor);
    void draw();
    void drawLive();
    void drawAllPaths();
    void drawLiveShadow();
    
    // Getters
    ofPixels getRecordPixels();
    int getNumberOfBlobs();
    string getBlobData();
    bool isSomeoneThere();
    bool newFrame();
    ofVec2f getBlobPath();
    
    // Setters
    void setColorInverse();
    void setCVMode(int mode);
    
    
    
    vector <vector <ofVec2f> > signedBlobPaths;
    
private:
    
    int _width;
    int _height;
    
    ofFbo recordFbo;
    ofPixels pix;
    
#ifdef DEBUG
    ofVideoPlayer           debugVideo;
#else
    ofVideoGrabber          vidGrabber;
#endif
    
    ofxCvColorImage			colorImg;
    ofxCvGrayscaleImage 	grayImage;
    ofxCvGrayscaleImage 	grayBg;
    ofxCvGrayscaleImage 	grayDiff;
    ofxCvFloatImage         grayFloatBg; // for the progressive learning
    ofxCvContourFinder      contourFinder;
    ofxCvGrayscaleImage     threshImage;
    ofxCvGrayscaleImage     lastFrame;
    ofxCvGrayscaleImage     frameDiff;
    
    ofTexture           outputTex;
    bool present;
    int mode;
    long backgroundTimer;
    int threshold;
    unsigned char * outpix;
    unsigned char * pixels;
    
};
//#endif /* defined(__ShadowingStage1__OCV__) */
