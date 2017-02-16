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
//#include "ofxMacamPs3Eye.h"

//#include "ofxCv.h"
#include "ofxCv/Utilities.h"
#include "ofxCv/Helpers.h"

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/videoio.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/video/background_segm.hpp>

using namespace ofxCv;

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
    void JsubtractionLoop(bool bLearnBackground,bool mirrorH,bool mirrorV,int threshold, int blur,int minBlobSize, int maxBlobSize,int maxBlobNum,bool fillHoles, bool useApproximation,float brightness,float contrast);
    
    // Dawid's loop
    void DsubtractionLoop(bool mirrorH, bool mirrorV);
    
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
    //ofxMacamPs3Eye          vidGrabber;
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
    
    ofxCvGrayscaleImage     diffImage;
    ofxCvGrayscaleImage     threshImage;
    ofxCvGrayscaleImage     lastFrame;
    ofxCvGrayscaleImage     frameDiff;
    ofxCvGrayscaleImage     presentImage;
    ofxCvGrayscaleImage     virginGray;
    
    cv::Mat fgMaskMOG2; //fg mask fg mask generated by MOG2 method
    cv::Mat bgMat;
    cv::Mat maskOut;
    cv::Mat keyOut;
    cv::Mat keyOut2;
    cv::Mat keyOut3;
    cv::Mat keyOut4;
    cv::BackgroundSubtractorMOG2 mog;
    
    vector<ofxCvGrayscaleImage> pastImages;
    ofImage             outputImage;
    
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
//#endif /* defined(__ShadowingStage1__OCV__) */
