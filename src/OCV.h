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
    void setTrackingBoundaries(int offsetX, int offsetY);
    void DsubtractionLoop(bool mirrorH, bool mirrorV);
    
    bool isSomeoneThere();
    bool isFrameNew();
    
    ofPixels getRecordPixels();
    ofImage getRecordImage();

    void draw();
    void drawCameraFullScreen();
    void updateCamExposure(bool state); 
    void toggleGui();
    void releaseCamera();
    void exit();
    
    //Pt Grey Utils
    void doCameraSetup();
    void PrintError(Error error);
    void PrintCameraInfo( CameraInfo* pCamInfo );
    Error error;
    Camera cam;
    Image rawImage;
    Image convertedImage;
    
    bool bNewFrame;
    
    //Debug stuff
    ofVideoGrabber vidGrabber;
    void resetDebugVideo();
    
    
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
    
    cv::Ptr<cv::BackgroundSubtractor> pBackSub;
   
    ofImage             outputImage;
    ofImage             outImage;
    
    bool present;
    long presenceTimer;
    unsigned char * pixels;

    long exposureTimer;
    bool getExposure;  
    long stopGettingExposureTimer;
    bool stopExposure;
    bool someoneInLight;

    //Cv Settings
    int pre_blur;
    int morph_size; 
    int morph_iterations;
    int smooth_size;
    double smooth_sigma1;
    double learningRate;
    int frame_diff_thresh;
    int presence_timeout_millis;
    int maxBrightnessDiff;
    int minContArea;
    int maxContArea;

    int erosion_size;
    int dilation_size;



};
