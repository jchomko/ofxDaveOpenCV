//--------------------------------------------------------------
//* Name: OCV.h
//* Project: Playable City 2014 Award
//* Author: David Haylock
//* Creation Date: 28-07-2014
//--------------------------------------------------------------

#include <iostream>
#include "ofMain.h"
#include "ofxOpenCv.h"
#include "coordWarping.h"
#include "guiQuad.h"
#include "ofxXmlSettings.h"

#define DEBUG

struct paths
{
    vector <ofVec2f> bPath;
    
};

class CV : public ofBaseApp {
    
public:
    void setup(int width, int height, int framerate);

    // Camera Calibration
    void setupCameraView();
    
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
    void drawCalibration();
    void drawTracking();
    void getAllPaths();
    
    
    // Getters
    ofPixels getRecordPixels();
    int getNumberOfBlobs();
    string getBlobData();
    bool isSomeoneThere();
    bool newFrame();
    ofVec2f getBlobPath();
    vector <ofVec3f> getBlobsCentroid();
    // Setters
    void setColorInverse();
    void setCVMode(int mode);
    void relearnBackground();
    
    void setCalibration(bool setMode);
    bool canDoCalibration;
    vector <ofVec2f> blobPath;
    
    vector <paths> blobPaths;
    
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    
private:
    
    ofxXmlSettings XML;
    ofPoint srcPts[4];
    ofPoint dstPts[4];
    guiQuad cvWarpQuad;
    coordWarping coordWarp;
    
    int _width;
    int _height;
    
    ofFbo recordFbo;
    ofPixels pix;
    //vector<ofVec3f>centroids;
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
    ofxCvGrayscaleImage     grayWarped;
    ofxCvGrayscaleImage     grayPostWarp;
    ofxCvContourFinder      contourFinder;
    ofxCvGrayscaleImage     threshImage;
    ofxCvGrayscaleImage     lastFrame;
    ofxCvGrayscaleImage     frameDiff;
    
    ofTexture           outputTex;
    bool learnBackground;
    bool present;
    int mode;
    long backgroundTimer;
    int threshold;
    unsigned char * outpix;
    unsigned char * pixels;
    
};
//#endif /* defined(__ShadowingStage1__OCV__) */
