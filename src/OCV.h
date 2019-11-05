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
// #define DEBUG

using namespace FlyCapture2;

struct paths
{
    vector <ofVec2f> bPath;

};

class CV : public ofBaseApp {

public:
       
    void setup(int width, int height, int framerate);


    void DsubtractionLoop(bool mirrorH, bool mirrorV);
    
    void draw();
    void drawLive();
    void drawAllPaths();
    void drawLiveShadow();
    void drawCalibration();
    void drawTracking();
    void getAllPaths();
    void toggleGui();
    
    void exit();
    void releaseCamera();

    //Pt Grey Utils
    void PrintError(Error error);
    void PrintCameraInfo( CameraInfo* pCamInfo );
    Error error;
    Camera cam;
    Image rawImage;
    Image convertedImage;
    void doCameraSetup();


    // Getters
    ofPixels getRecordPixels();
    ofImage getRecordImage();
    
    bool isSomeoneThere();
    void resetDebugVideo();

    bool isFrameNew();
    bool bNewFrame;
    
    // Setters
    void setTrackingBoundaries(int offsetX, int offsetY);
    ofVideoGrabber vidGrabber;

    void drawCameraFullScreen();
    void updateCamExposure(bool state); 
    
    
private:

    ofxUICanvas *ggui;
    void guiEventCV(ofxUIEventArgs &e);
    void setupCVGui();

    int framerate;
    float imgBrightness;
    float lastBrightness;
    
    
    int width;
    int height;
    int offsetX;
    int offsetY;
    ofFbo recordFbo;
    ofPixels pix;
    //vector<ofVec3f>centroids;

#ifdef DEBUG
    ofVideoPlayer           debugVideo;
#endif

    ofImage colorImage;

    cv::Mat lastFrame;
    cv::Mat frameDiff;
    cv::Mat fgMaskMOG2; 
    cv::Mat bgMat;
    cv::Mat maskOut;
    cv::Mat keyOut;
    cv::Mat keyOut2;
    // cv::Mat keyOut3;
    // cv::Mat keyOut4;

    
    cv::Ptr<cv::BackgroundSubtractor> pBackSub;
   
    ofImage             outputImage;
    ofImage             outImage;
    
    bool present;
    long presenceTimer;
    unsigned char * pixels;

    //These need cleaning up
    long exposureTimer;
    bool getExposure;
    
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
