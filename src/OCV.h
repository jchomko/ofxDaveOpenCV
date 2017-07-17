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
//#include "ofxGui.h"
#include "FlyCapture2.h"

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

#include "ofxUI.h"


using namespace ofxCv;

//#define DEBUG

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

    //void JsubtractionLoop(bool bLearnBackground,bool mirrorH,bool mirrorV,int threshold, int blur,int minBlobSize, int maxBlobSize,int maxBlobNum,bool fillHoles, bool useApproximation,float brightness,float contrast);
    void JsubtractionLoop(bool learnBackground, bool bMirrorH, bool bMirrorV, int threshold, int moveThreshold, int fBlur, int gaussBlur, int medianBlur, int iMinBlobSize, int iMaxBlobSize,int iMaxBlobNum, bool bFillHoles, bool bUseApprox,float brightness, float contrast, bool erode, bool dilate);
    
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
    void toggleGui();
    void drawGui();
    void exit();
    void PrintError(Error error);
    void PrintCameraInfo( CameraInfo* pCamInfo );
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
    void drawCameraFullScreen();

private:

    ofxUICanvas *ggui;
    void guiEventCV(ofxUIEventArgs &e);
    void setupCVGui();

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

    ofxCvColorImage         colorImage;
    ofxCvGrayscaleImage     grayImage;
    ofxCvGrayscaleImage     grayBg;
    ofxCvGrayscaleImage     grayDiff;
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
    
    ofxCvGrayscaleImage     cleanFrameDiff;    
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

    //Dawid CV Variables
    // ofxPanel ngui;
    // ofParameter<int> threshold_min;
    // ofParameter<int> pre_blur;
    // ofParameter<int> erosion_size;
    // ofParameter<int> dilation_size;
    // ofParameter<int> max_elem;
    // ofParameter<int> max_kernel_size;
    // ofParameter<int> morph_size;
    // ofParameter<int> morph_iterations;
    // ofParameter<int> post_blur;

    // ofParameter<int> post_erosion_size;
    // ofParameter<int> expand_size;
    // ofParameter<double> expand_sigma1;
    // ofParameter<int> smooth_size;
    // ofParameter<double> smooth_sigma1;
    // ofParameter<double> learningRate;

    int threshold_min;
    int pre_blur;
    int erosion_size;
    int dilation_size;
    int max_elem;
    int max_kernel_size;
    int morph_size; // 32
    int morph_iterations;
    int post_blur;


    int post_erosion_size;
    int expand_size;
    double expand_sigma1;
    int smooth_size;
    double smooth_sigma1;
    double learningRate;
    

};
