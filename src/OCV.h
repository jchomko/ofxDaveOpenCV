//--------------------------------------------------------------
// Name: OCV.h
// Project: Playable City 2014 Award
// Author: David Haylock
// Creation Date: 28-07-2014
//--------------------------------------------------------------
#include <iostream>

#include "FlyCapture2.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>

//ofxCv utilities
#include "Utilities.h"

#include "ofxUI.h"
#include "ofxXmlSettings.h"

//Uncomment to activate debug mode with video
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
    // void setupCameraView();

    // Specifically the PS3 eye needs releasing to ensure the locking mechanism is unlocked
    void releaseCamera();

    //void JsubtractionLoop(bool bLearnBackground,bool mirrorH,bool mirrorV,int threshold, int blur,int minBlobSize, int maxBlobSize,int maxBlobNum,bool fillHoles, bool useApproximation,float brightness,float contrast);
    // void JsubtractionLoop(bool learnBackground, bool bMirrorH, bool bMirrorV, int threshold, int moveThreshold, int fBlur, int gaussBlur, int medianBlur, int iMinBlobSize, int iMaxBlobSize,int iMaxBlobNum, bool bFillHoles, bool bUseApprox,float brightness, float contrast, bool erode, bool dilate);
    
    // Dawid's loop
    void DsubtractionLoop(bool mirrorH, bool mirrorV);
    
    void draw();
    void drawLive();
    void drawAllPaths();
    void drawLiveShadow();
    void drawCalibration();
    void drawTracking();
    void getAllPaths();
    void toggleGui();
    // void drawGui();
    void exit();

    //Pt Grey Utils
    void PrintError(Error error);
    void PrintCameraInfo( CameraInfo* pCamInfo );
   
    // Getters
    ofPixels getRecordPixels();
    ofImage getRecordImage();
    
    // int getNumberOfBlobs();
    // string getBlobData();
    bool isSomeoneThere();
    // bool isSomeoneInTheLight();
    void resetDebugVideo();

    bool bNewFrame;
    bool isFrameNew();
    // ofVec2f getBlobPath();
    // vector <ofVec3f> getBlobsCentroid();
   
    // Setters
    void setTrackingBoundaries(int offsetX, int offsetY);
    // void setColorInverse();
    // void setCVMode(int mode);
    // void relearnBackground();

    // void setCalibration(bool setMode);
    // bool canDoCalibration;
    // vector <ofVec2f> blobPath;
    // vector <paths> blobPaths;
    
    // void mouseDragged(int x, int y, int button);
    // void mousePressed(int x, int y, int button);
    // void mouseReleased(int x, int y, int button);

    Error error;
    Camera cam;
    Image rawImage;
    Image convertedImage;
    
    ofVideoGrabber vidGrabber;

    void doCameraSetup();

    ofFbo       pathFbo;
    // ofPath      imgBlobPath;
    // vector<ofPath> imgBlobPaths;

    void drawCameraFullScreen();
    void updateCamExposure(bool state); 
    
    int framerate;

    float imgBrightness;
    float lastBrightness;
    
private:

    ofxUICanvas *ggui;
    void guiEventCV(ofxUIEventArgs &e);
    void setupCVGui();

    // ofxXmlSettings XML;
    // ofPoint srcPts[4];
    // ofPoint dstPts[4];
    // guiQuad cvWarpQuad;
    // coordWarping coordWarp;

    int width;
    int height;
    int offsetX;
    int offsetY;
    ofFbo recordFbo;
    ofPixels pix;
    //vector<ofVec3f>centroids;

#ifdef DEBUG
    ofVideoPlayer           debugVideo;
#else

#endif

    ofImage colorImage;
    
    cv::Mat lastFrame;
    cv::Mat frameDiff;
    cv::Mat fgMaskMOG2; 
    cv::Mat bgMat;
    cv::Mat maskOut;
    cv::Mat keyOut;
    cv::Mat keyOut2;
    cv::Mat keyOut3;
    cv::Mat keyOut4;

    
    cv::Ptr<cv::BackgroundSubtractor> pBackSub;
   
    ofImage             outputImage;
    ofImage             outImage;
    
    // ofTexture           outputTex;
    // bool learnBackground;
    // bool startLearn;
    // int mode;
    // long backgroundTimer;
    // int threshold;
    // long absenceTimer;
    // unsigned char * outpix;
    
    bool present;
    long presenceTimer;
    unsigned char * pixels;

    long exposureTimer;
    bool getExposure;


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
    int frame_diff_thresh;
    int presence_timeout_millis;
    int maxBrightnessDiff;

    bool someoneInLight;
    int minContArea;
    int maxContArea;

};
