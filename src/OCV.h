//--------------------------------------------------------------
// Name: OCV.h
// Project: Playable City 2014 Award
// Author: David Haylock
// Creation Date: 28-07-2014
//--------------------------------------------------------------

#include <iostream>
#include "ofMain.h"
#include "ofxOpenCv.h"
#include "coordWarping.h"
#include "guiQuad.h"
#include "ofxXmlSettings.h"
//#include "ofxKinect.h"
//#include "ofxV4L2Settings.h"
#include "ofxGui.h"
#include "FlyCapture2.h"

//#include "ofxMacamPs3Eye.h"

#define DEBUG

using namespace FlyCapture2;


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
    void JsubtractionLoop(bool bLearnBackground,bool mirrorH,bool mirrorV,int imgThreshold,int moveThreshold, int blur, int gaussBlur, int medianBlur, int minBlobSize, int maxBlobSize,int maxBlobNum,bool fillHoles, bool useApproximation,float brightness,float contrast, bool erode, bool dilate);

    //Path sub loop
    void PsubtractionLoop(bool bLearnBackground,bool mirrorH,bool mirrorV,int imgThreshold,int moveThreshold, int blur, int gaussBlur, int medianBlur, int minBlobSize, int maxBlobSize,int maxBlobNum,bool fillHoles, bool useApproximation,float brightness,float contrast, bool erode, bool dilate);

  //ProgSubLoop
	void progSubLoop(int minBlobSize, int maxBlobSize, int threshold, float blur, float brightness, float contrast);
    void readAndWriteBlobData(ofColor backgroundColor,ofColor shadowColor);
    void draw();
    void drawLive();
    void drawAllPaths();
    void drawLiveShadow();
    void drawCalibration();
    void drawTracking();
    void getAllPaths();
    void toggleGui();
    void drawGui();
    void exit();
	void PrintError(Error error);
    // Getters
    ofPixels getRecordPixels();
    int getNumberOfBlobs();
    string getBlobData();
    bool isSomeoneThere();
    bool isSomeoneInTheLight();


    bool newFrame();
    ofVec2f getBlobPath();
    vector <ofVec3f> getBlobsCentroid();
    // Setters

    void setTrackingBoundaries(int x, int y, int w, int h);
    void setTrackingBoundaries(int offsetX, int offsetY);
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

    Error error;
    Camera cam;
    Image rawImage;
    Image convertedImage;
	ofVideoGrabber vidGrabber;

    ofFbo       pathFbo;
    ofPath      imgBlobPath;
    vector<ofPath> imgBlobPaths;

private:

    ofxXmlSettings XML;
    ofPoint srcPts[4];
    ofPoint dstPts[4];
    guiQuad cvWarpQuad;
    coordWarping coordWarp;

    int _width;
    int _height;
    int _offsetX;
    int _offsetY;
    ofFbo recordFbo;
    ofPixels pix;
    //vector<ofVec3f>centroids;

#ifdef DEBUG
    ofVideoPlayer           debugVideo;
#else

#endif

    ofxCvColorImage			colorImg;
    ofxCvGrayscaleImage 	grayImage;
    ofxCvGrayscaleImage 	grayBg;
    ofxCvGrayscaleImage 	grayDiff;
    ofxCvFloatImage         grayFloatBg; // for the progressive learning
    ofxCvGrayscaleImage     grayWarped;
    ofxCvGrayscaleImage     grayPostWarp;
    ofxCvShortImage     kinectGray;

    ofxCvContourFinder      contourFinder;
    ofxCvContourFinder      imagingContourFinder;

    ofxCvGrayscaleImage     diffImage;
    ofxCvGrayscaleImage     threshImage;
    ofxCvGrayscaleImage     lastFrame;
    ofxCvGrayscaleImage     frameDiff;
    ofxCvGrayscaleImage     presentImage;
    ofxCvGrayscaleImage     virginGray;
    ofxCvGrayscaleImage     invDiffImage;
    ofxCvGrayscaleImage	    cleanFrameDiff;
    vector<ofxCvGrayscaleImage> pastImages;
    ofImage             outputImage;
    ofxCvFloatImage     fgrayImage;
    //ofxCvFloatImage     flastGrayImage;



    ofTexture           outputTex;
    bool learnBackground;
    bool startLearn;
    bool present;
    int mode;
    long backgroundTimer;
    int threshold;
    long absenceTimer;
    long presenceTimer;
    unsigned char * outpix;
    unsigned char * pixels;

};
