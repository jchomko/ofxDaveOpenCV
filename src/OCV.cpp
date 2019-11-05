//--------------------------------------------------------------
// Name: OCV.cpp
// Project: Playable City 2014 Award
// Author: David Haylock
// Creation Date: 28-07-2014
//--------------------------------------------------------------

#include "OCV.h"
// using namespace ofxCv;
// using namespace cv;

//--------------------------------------------------------------
void CV::setup( int _width,int _height, int _framerate){

   
    width = _width;
    height = _height;
    framerate = _framerate;
    
    cout << "width : " << width << " height: " << height << endl;
    cout << "OpenCV version : " << CV_VERSION << endl;
    cout << "Major version : " << CV_MAJOR_VERSION << endl;
    cout << "Minor version : " << CV_MINOR_VERSION << endl;
    cout << "Subminor version : " << CV_SUBMINOR_VERSION << endl;
     

#ifdef DEBUG

    cout << "loading dbug video " << endl;
    debugVideo.load("/root/recordings/test2.mkv");
    debugVideo.setLoopState(OF_LOOP_NORMAL);
    debugVideo.setPosition(0.22);
    debugVideo.play();

#else

    doCameraSetup();
    
#endif

    presenceTimer = 0;
    present = false;
    offsetX = 0;
    offsetY = 0;
    
    //History, threshold, detectShadows 
    pBackSub = cv::createBackgroundSubtractorMOG2(750, 28, false);
    
    morph_size = 4; // 32
    morph_iterations = 1;

    post_blur = 1;
    post_erosion_size = 1;
    expand_size = 1;
    expand_sigma1 = 1;
    
    smooth_size = 3;
    smooth_sigma1 = 2;

    learningRate = -1;

    // setupCVGui();
    bNewFrame = false;  
    getExposure = false;

    minContArea = 120;
    maxContArea = 5000;
    someoneInLight = false;

    setupCVGui();  
    outImage.allocate(width,height, OF_IMAGE_GRAYSCALE);
    pix.allocate(width,height,OF_IMAGE_GRAYSCALE);

}

void CV::resetDebugVideo(){

    #ifdef DEBUG
        debugVideo.setPosition(0.22f);
    #endif

}

void CV::doCameraSetup(){

    //Camera Setup
    FC2Version fc2Version;
    ostringstream version;
    version << "FlyCapture2 library version: " << fc2Version.major << "." << fc2Version.minor << "." << fc2Version.type << "." << fc2Version.build;
    cout << version.str() << endl;
    
    ostringstream timeStamp;
    timeStamp <<"Application build date: " << __DATE__ << " " << __TIME__;
    cout << timeStamp.str() << endl << endl;

    BusManager busMgr;
    unsigned int numCameras;
    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
    }
    
    PGRGuid guid;
    cout << "Number of cameras detected: " << numCameras << endl;
    for (unsigned int i=0; i < numCameras; i++)
    {
        error = busMgr.GetCameraFromIndex(i, &guid);
        if (error != PGRERROR_OK)
        {
            PrintError( error );
        }
        //RunSingleCamera( guid );
    }

    // cout << "Firing bus reset " << endl;
    // error = busMgr.FireBusReset(&guid);
    // if (error != PGRERROR_OK)   {
    //         PrintError( error );
    //     }else{
    //         PrintError( error );
    // }

    // cout << "Number of cameras detected: " << numCameras << endl;
    // for (unsigned int i=0; i < numCameras; i++)
    // {
    //     error = busMgr.GetCameraFromIndex(i, &guid);
    //     if (error != PGRERROR_OK)
    //     {
    //         PrintError( error );
    //     }
    //     //RunSingleCamera( guid );
    // }

    cout << "Attempting to connect to cam " << endl;
    error = cam.Connect(&guid);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
    }

    cout << "Cam connection result "  << endl;
    PrintError( error );

    // NUM_VIDEOMODES
    cout << "Number of videomodes : " << numCameras << endl;

    // error = cam.SetVideoModeAndFrameRate(VIDEOMODE_640x480Y16,FRAMERATE_60);
    error = cam.SetVideoModeAndFrameRate(VIDEOMODE_640x480Y8,FRAMERATE_30);
    // error = cam.SetVideoModeAndFrameRate(VIDEOMODE_320x240YUV422,FRAMERATE_30);

    if( error != PGRERROR_OK )
    {
        PrintError(error);
    }

    CameraInfo camInfo;
    error = cam.GetCameraInfo(&camInfo);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
    }

    cout << "camera model name " << camInfo.modelName << endl;
    string modelName = camInfo.modelName;
    string firefly = "Firefly MV FMVU-03MTM";

    if(modelName == firefly){
        cout << "WE HAVE FIREFLY" << endl;
    }else{
        cout << "no match between " << modelName << " and " << firefly << endl;
    }
    
    PrintCameraInfo(&camInfo);
    Property camProp;
    PropertyInfo camPropInfo;

    camProp.type = FRAME_RATE;
    error = cam.GetProperty( &camProp );
    if (error != PGRERROR_OK){
        PrintError( error );
    }
    cout <<  "Frame rate is : " <<  camProp.absValue << endl;

    //Set Framerate to 50 (if firefly camera)
    // camProp.type = FRAME_RATE;
    camProp.type = FRAME_RATE;
    camProp.autoManualMode = false;
    // camProp.absControl = true;
    camProp.onOff = true;

    camProp.absValue = framerate;
    
    error = cam.SetProperty( &camProp );
    if (error != PGRERROR_OK){
        PrintError( error );
    }
    cout <<  "Setting Framerate : " << endl;


//    camProp.type = FRAME_RATE;
    error = cam.GetProperty( &camProp );
    if (error != PGRERROR_OK){
        PrintError( error );
    }
    cout <<  "Frame rate is : " <<  camProp.absValue << endl;


    camProp.type = AUTO_EXPOSURE;
    camProp.onOff = false;
    error = cam.SetProperty(&camProp);
    if (error != PGRERROR_OK){
    PrintError(error);
    }

    error = cam.StartCapture();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
    }
    cout << "capture started" << endl;

}

void CV::updateCamExposure(bool state){

    if(state == true){
    
        Property camProp;
        camProp.type = AUTO_EXPOSURE;
        camProp.autoManualMode = true;
        camProp.absControl = false;
        camProp.onOff = true;
        error = cam.SetProperty(&camProp);
    
    }else{
    
        Property camProp;
        camProp.type = AUTO_EXPOSURE;
        camProp.autoManualMode = false;
        camProp.absControl = false;
        camProp.onOff = false;
        error = cam.SetProperty(&camProp);
    }
}


 void CV::setupCVGui(){

    ggui = new ofxUICanvas(500, 150,200,600);
    ggui->setColorBack(ofColor::black);
    ggui->addWidgetDown(new ofxUINumberDialer(-1.00f, 1.00f, 0.01f, 1, "learningRate", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 255, 5, 1, "frameDiffThresh", OFX_UI_FONT_MEDIUM));
    
    // ggui->addWidgetDown(new ofxUINumberDialer(0, 255, 1, 0, "threshold_min1", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 1, 0, "pre_blur", OFX_UI_FONT_MEDIUM));
    // ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 3, 0, "erosion_size", OFX_UI_FONT_MEDIUM));
    // ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 8, 0, "dilation_size", OFX_UI_FONT_MEDIUM));
    // ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 4, 0, "max_elem", OFX_UI_FONT_MEDIUM));
    // ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 1, 0, "max_kernel_size", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 4, 0, "morph_size", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 1, 0, "morph_iterations", OFX_UI_FONT_MEDIUM));

    ggui->addWidgetDown(new ofxUINumberDialer(0, 1000, 1, 0, "_minContArea", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 76000, 1, 0, "_maxContArea", OFX_UI_FONT_MEDIUM));
    

    // ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 1, 1, "post_blur", OFX_UI_FONT_MEDIUM));
    // ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 1, 0, "post_erosion_size", OFX_UI_FONT_MEDIUM));
    // ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 1, 2, "expand_size", OFX_UI_FONT_MEDIUM));
    // ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 1, 0, "expand_sigma1", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 1, 0, "smooth_size", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 1, 0, "smooth_sigma1", OFX_UI_FONT_MEDIUM));
    
    ggui->addWidgetDown(new ofxUINumberDialer(0, 8000, 1200, 1, "presenceTimeoutMillis", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 8000, 1200, 1, "maxBrightnessDiff", OFX_UI_FONT_MEDIUM));

    // ggui->addButton("updateExposure", false);
    // ggui->addToggle("T1", false, 44, 44);
    ggui->autoSizeToFitWidgets();
    
    ofAddListener(ggui->newGUIEvent,this, &CV::guiEventCV);
    ggui->loadSettings("GUI/CVSettings.xml");
    ggui->setVisible(true);

}

//--------------------------------------------------------------
void CV::releaseCamera()
{

    #ifndef DEBUG
        error = cam.StopCapture();
        if (error != PGRERROR_OK){
            PrintError( error );
        }else{
           cout << "capture stopped " << endl;
        }

        // Disconnect the camera
        error = cam.Disconnect();
        if (error != PGRERROR_OK){
            PrintError( error );
        }else{
            cout << "cam disconnected " << endl;
        }
    #endif
}

//--------------------------------------------------------------
void CV::DsubtractionLoop(bool mirrorH, bool mirrorV){

    bNewFrame = false;

    //Exposure timer 
    if(getExposure && ofGetElapsedTimeMillis() > exposureTimer){
        getExposure = false;
        // cout << "stopping exposure " << endl;
        updateCamExposure(getExposure);
    }

    #ifdef DEBUG
        
        debugVideo.update();
        bNewFrame = debugVideo.isFrameNew();

    #else
    
        //Check if camera is connected
        if(cam.IsConnected()){
            error = cam.RetrieveBuffer( &rawImage );
            
            if (error != PGRERROR_OK){
                PrintError( error );
            }else{
                bNewFrame = true; 
            }

        //If not run the setup sequence again
        }else{
            doCameraSetup();
        }

    #endif
    
    
    if (bNewFrame && !getExposure){

        #ifdef DEBUG

            colorImage.resize(debugVideo.getWidth(),debugVideo.getHeight());
            colorImage.setFromPixels(debugVideo.getPixels());
            colorImage.resize(width, height);
            colorImage.setImageType(OF_IMAGE_GRAYSCALE);

        #else

            colorImage.setFromPixels(rawImage.GetData(), rawImage.GetCols(), rawImage.GetRows(), OF_IMAGE_GRAYSCALE);

        #endif

        cv::Mat origFrameMat = ofxCv::toCv(colorImage.getPixels()); 
        cv::Mat frameMat = origFrameMat; 


        //Frame Diff presence detection
        if(!lastFrame.empty()){

            cv::absdiff(origFrameMat, lastFrame, frameDiff);
            
            //Threshold
            cv::threshold(frameDiff, frameDiff, frame_diff_thresh, 255, cv::THRESH_BINARY);

            //Set ROI -
            // cv::Rect myROI(offsetX, offsetY, width-(offsetX*2), width-(offsetY*2));
            
            //Crop 
            // cv::Mat croppedFrameDiff = frameDiff(myROI);

            //Find Contours
            std::vector<std::vector<cv::Point> > frameDiffContours;
            cv::Mat fd_cont_hierarchy;

            cv::findContours(frameDiff, frameDiffContours, fd_cont_hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            if(frameDiffContours.size() > 0 ){  //&& ofGetFrameNum() > 100
                someoneInLight = true; 
            }else{
                someoneInLight = false;
            }

        }


        origFrameMat.copyTo(lastFrame);
        
        if( !someoneInLight && ofGetElapsedTimeMillis() - presenceTimer > presence_timeout_millis){

            //start getting exposure - but only after we've had someone present
            //this should maybe happen long after people have left
            //but lets run this for a while now
            if(getExposure == false && present){
                
                // cout << "getting exposure " << endl;
                getExposure = true;

                //Put this delay on a slider
                exposureTimer = ofGetElapsedTimeMillis() + 15000;
                updateCamExposure(getExposure);
            }
            present = false;
        }
        
        //this is not an else-if because we want to always be resetting the timer when someone is prsent,
        if( someoneInLight ){

                presenceTimer = ofGetElapsedTimeMillis();
                //While Present
                present = true;
                //absenceTimer = ofGetElapsedTimeMillis() + 5000;
        }


        //Background Subtraction
        // pre blur 
        cv::medianBlur(frameMat, frameMat, 2*pre_blur+1);

        pBackSub->apply(frameMat, fgMaskMOG2, learningRate);

        fgMaskMOG2.copyTo(maskOut);

        //Keep these incase we need them - ie we have more noise than expected insie the lamp
        // //Erode
        // cv::Mat erosion_kernel = cv::getStructuringElement( cv::MORPH_ELLIPSE,
        //                                             cv::Size( 2*erosion_size + 1, 2*erosion_size+1 ),
        //                                             cv::Point( erosion_size, erosion_size ) );

        // cv::erode(maskOut, maskOut, erosion_kernel);
        
        // // Dilate
        // cv::Mat dilation_kernel = cv::getStructuringElement( cv::MORPH_ELLIPSE,
        //                                                    cv::Size( 2*dilation_size + 1, 2*dilation_size+1 ),
        //                                                    cv::Point( dilation_size, dilation_size ) );
        // cv::dilate(maskOut, maskOut, dilation_kernel);

        // // Morphology - closing holes in the image
        cv::Mat morph_element = getStructuringElement( cv::MORPH_ELLIPSE,
                                                      cv::Size( 2*morph_size + 1, 2*morph_size+1 )); //,
                                                       // cv::Point( morph_size, morph_size ));
                                                        //);
                                                        //was morph_size, morph_size but I don't think it does anything

        cv::morphologyEx( maskOut, maskOut, cv::MORPH_CLOSE, morph_element, cv::Point(-1,-1), morph_iterations, cv::BORDER_CONSTANT );

        // post Erode - this is achieved with the contour finder now
        // cv::Mat post_erosion_kernel = cv::getStructuringElement( cv::MORPH_ELLIPSE,
        //                                                    cv::Size( 2*post_erosion_size + 1, 2*post_erosion_size+1 ),
        //                                                    cv::Point( post_erosion_size, post_erosion_size ) );
        // cv::erode(maskOut, maskOut, post_erosion_kernel);
        
        // cv::medianBlur(maskOut, maskOut, 2*post_blur+1);

        cv::Mat mask2 = cv::Mat(width, height, CV_8UC1);
        mask2 = cv::Scalar::all(0);
        
        cv::Mat bw;
        maskOut.copyTo(bw);
        cv::GaussianBlur(maskOut, maskOut, cv::Size(smooth_size, smooth_size), smooth_sigma1); // smooth edges


        std::vector<std::vector<cv::Point> > contours;
        // cv::Mat cont_hierarchy;
        std::vector<cv::Vec4i> hierarchy;
        // cv::Mat rejected_contours;
        
        cv::findContours(bw, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        std::vector<std::vector<cv::Point> > approx( contours.size() );
        // cout << "number of contours : " << contours.size() << endl;

        for (size_t idx = 0; idx < contours.size(); idx++) {

            cv::approxPolyDP( cv::Mat(contours[idx]),
                              approx[idx],
                              cv::arcLength(cv::Mat(contours[idx]), true)*0.004, true );

            // Skip small or non-convex objects
            //This works pretty well, but just drops frames if somethign is too big
            //Mabye we should use save the contours and re-draw if they're not good
            //but needs to be tested real-world. Add these to the UI 

            //single person from 3m seems to have a max of about 5000
            //But this needs to be tested with multiple people, and more precise headlights
            //|| !cv::isContourConvex(contours[idx])
            auto area = cv::contourArea(contours[idx]);

            if( area > minContArea && area < maxContArea){
                //destination image, contours, contour to draw, colour, linetype
                // cv::drawContours(mask2, approx, idx, cv::Scalar(255), -1);  
                // cv::drawContours(mask2, approx, idx, cv::Scalar(255), -1);  
                cv::drawContours(mask2, approx, idx, cv::Scalar(255), cv::FILLED, 0, hierarchy);  

                // cv::drawContours(mask2, contours, idx, cv::Scalar(255), -1);  
              }else{
                // cout << "rejected contour : " << area << endl;
                // cv::drawContours(rejected_contours, approx, idx, cv::Scalar(255), -1);
            }
    //        cv::convexHull(cv::Mat(approx[idx]), hull[idx]);
//            int minContArea = 100;
//            if (std::fabs(cv::contourArea(hull[idx])) < minContArea)
//                continue;
  //          cv::drawContours(mask2, hull, idx, cv::Scalar(255), -1);
            }


        cv::GaussianBlur(mask2, mask2, cv::Size(smooth_size, smooth_size), smooth_sigma1); // smooth edges
        keyOut2 = mask2;

   

        lastBrightness = imgBrightness;
        imgBrightness = 0;

    	for(int x =0; x < maskOut.rows; x++){
    		for(int y = 0; y < maskOut.cols; y++){
    			imgBrightness += maskOut.ptr<uchar>(y)[x]/255;
    		}
    	}

        // if(abs(lastBrightness-imgBrightness) < maxBrightnessDiff){

        // }

        //Invert images 
        //This one is the 'pure cv' version
        maskOut = ~maskOut;

        //This one is the contour finder version
        mask2 = ~mask2;

        ofxCv::toOf(mask2, pix); 
        outImage.setFromPixels(pix);

   }

}


//--------------------------------------------------------------
bool CV::isFrameNew(){

    #ifdef DEBUG
        return debugVideo.isFrameNew();
    #else
        return bNewFrame;
        // return true;
       //return vidGrabber.isFrameNew();
    #endif

}
//--------------------------------------------------------------
bool CV::isSomeoneThere(){
    return present;
}

//--------------------------------------------------------------
void CV::setTrackingBoundaries(int _offsetX, int _offsetY){
    
    offsetX = _offsetX;
    offsetY = _offsetY;

}

//------------------------------------------------------------
void CV::toggleGui()
{
//  showGui = !showGui;
    ggui->toggleVisible();

}

//--------------------------------------------------------------
void CV::drawLiveShadow(){
    ofSetColor(255, 255, 255);
    recordFbo.draw(0,0,ofGetWidth(),ofGetHeight());
}

//--------------------------------------------------------------
void CV::drawCalibration(){

}
//--------------------------------------------------------------
void CV::drawAllPaths()
{
    
}
//--------------------------------------------------------------
void CV::getAllPaths()
{
    
}
//--------------------------------------------------------------
void CV::drawTracking()
{
     
}
//--------------------------------------------------------------
void CV::drawLive()
{

}
//--------------------------------------------------------------
void CV::draw()
{

    ofFill();
    ofSetColor(255);
    
    float x = 0.f, y = 0.f;
    
    ofPushMatrix();
    ofTranslate(x, y);
    ofDrawBitmapStringHighlight("Original Image",5,15);
    colorImage.draw(0,20,width/2,height/2);
    ofPopMatrix();
    x += width/2;

    ofPushMatrix();
    ofTranslate(x, y);
    ofDrawBitmapStringHighlight("frame diff",5,15);
    ofPixels frameDiffPix; 
    ofImage frameDiffImg;
    ofxCv::toOf(frameDiff, frameDiffPix);
    frameDiffImg.setFromPixels(frameDiffPix);
    frameDiffImg.draw(0,20,width/2,height/2);
    ofPushStyle();
    ofNoFill();
    if ( isSomeoneThere() ){
        ofSetColor(0, 255, 0);
    }else{
        ofSetColor(255,25,0);
    }
    ofDrawRectangle(offsetX/2,20 + (offsetY/2),(width-(offsetX*2))/2,(height-(offsetY*2))/2);
    ofSetColor(255, 255, 255);
    ofPopStyle();
    ofPopMatrix();
    x += width/2;
    

    ofPushMatrix();
    ofTranslate(x, y);
    ofDrawBitmapStringHighlight("mask out",5,15);
    ofPixels maskOutPix; 
    ofImage maskOutImg;
    ofxCv::toOf(frameDiff, maskOutPix);
    maskOutImg.setFromPixels(maskOutPix);
    maskOutImg.draw(0,20,width/2,height/2);
    ofPopMatrix();
    x += width/2;
    
    ofPushMatrix();
    ofTranslate(x, y);
    ofDrawBitmapStringHighlight("contour Finder mask",5,15);
    ofPixels contourFinderPix; 
    ofImage contourFinderImg;
    ofxCv::toOf(keyOut2, contourFinderPix);
    contourFinderImg.setFromPixels(contourFinderPix);
    contourFinderImg.draw(0,20,width/2,height/2);
    ofPopMatrix();
    x += width/2;
    
    
    ofPushMatrix();
    ofTranslate(x, y);
    ofDrawBitmapStringHighlight("backSubImage",5,15);
    ofPixels p; 
    ofImage i;
    ofxCv::toOf(fgMaskMOG2, p);
    i.setFromPixels(p);
    i.draw(0,20,width/2,height/2);

    ofPopMatrix();
    x += width/2;
    
    float brightnessDiff = imgBrightness - lastBrightness;
    ofDrawBitmapStringHighlight("Image Brightness : " + ofToString(imgBrightness) + " Brightness diff : " + ofToString(brightnessDiff), 0, 170);
    ofDrawBitmapStringHighlight("Framerate  : " + ofToString(ofGetFrameRate()), 0, 190);
    

}

void CV::drawCameraFullScreen(){

	 colorImage.draw(0,0, ofGetWidth(),ofGetHeight());
}

//--------------------------------------------------------------
ofPixels CV::getRecordPixels(){
    return pix;
}

ofImage CV::getRecordImage(){
    return outImage;
}

void CV::PrintError (Error error){
    error.PrintErrorTrace();
}

void CV::PrintCameraInfo( CameraInfo* pCamInfo ){
    cout << endl;
    cout << "*** CAMERA INFORMATION ***" << endl;
    cout << "Serial number -" << pCamInfo->serialNumber << endl;
    cout << "Camera model - " << pCamInfo->modelName << endl;
    cout << "Camera vendor - " << pCamInfo->vendorName << endl;
    cout << "Sensor - " << pCamInfo->sensorInfo << endl;
    cout << "Resolution - " << pCamInfo->sensorResolution << endl;
    cout << "Firmware version - " << pCamInfo->firmwareVersion << endl;
    cout << "Firmware build time - " << pCamInfo->firmwareBuildTime << endl << endl;
}

void CV::exit(){
    ggui->saveSettings("GUI/CVSettings.xml");
    delete ggui;
}


void CV::guiEventCV(ofxUIEventArgs &e){

    string name = e.getName();
    int kind = e.getKind();
    cout << "got event from: " << name << endl;
    ofxUINumberDialer * toggle = (ofxUINumberDialer *) e.widget;
    cout << toggle->getValue() << endl;
        
    if (e.getName() == "threshold_min1")
    {
        ofxUINumberDialer * toggle = (ofxUINumberDialer *) e.widget;
        threshold_min = toggle->getValue();
        cout << "threshold value :" << threshold_min << endl;

    }else if (e.getName() == "pre_blur")
    {
        ofxUINumberDialer * toggle = (ofxUINumberDialer *) e.widget;
       
         int s = toggle->getValue();
        if(s%2>0){
            pre_blur = s;
        }
    }else if (e.getName() == "erosion_size")
    {
        ofxUINumberDialer * toggle = (ofxUINumberDialer *) e.widget;
        erosion_size = toggle->getValue();
    }else if (e.getName() == "dilation_size")
    {
        ofxUINumberDialer * toggle = (ofxUINumberDialer *) e.widget;
        dilation_size = toggle->getValue();
    }else if (e.getName() == "max_kernel_size")
    {
        ofxUINumberDialer * toggle = (ofxUINumberDialer *) e.widget;
        max_kernel_size = toggle->getValue();
    }else if (e.getName() == "morph_size")
    {
        ofxUINumberDialer * toggle = (ofxUINumberDialer *) e.widget;
        morph_size = toggle->getValue();
    }else if (e.getName() == "morph_iterations")
    {
        ofxUINumberDialer * toggle = (ofxUINumberDialer *) e.widget;
        morph_iterations = toggle->getValue();
    }else if (e.getName() == "post_blur")
    {
        ofxUINumberDialer * toggle = (ofxUINumberDialer *) e.widget;
       
        int s = toggle->getValue();
        if(s%2>0){
            post_blur = s;
        }
        
    }else if (e.getName() == "post_erosion_size")
    {
        ofxUINumberDialer * toggle = (ofxUINumberDialer *) e.widget;
        post_erosion_size = toggle->getValue();
    }else if (e.getName() == "expand_size")
    {
        ofxUINumberDialer * toggle = (ofxUINumberDialer *) e.widget;
        
        int s = toggle->getValue();
        if(s%2>0){
            expand_size = s;
            cout << "expand size : " << expand_size << endl;

        }

        
    }else if (e.getName() == "expand_sigma1")
    {
        ofxUINumberDialer * toggle = (ofxUINumberDialer *) e.widget;
        expand_sigma1 = toggle->getValue();
    }else if (e.getName() == "smooth_size")
    {
        ofxUINumberDialer * toggle = (ofxUINumberDialer *) e.widget;
        
        int s = toggle->getValue();
        if(s%2>0){
            smooth_size = s;
        }

    }else if (e.getName() == "smooth_sigma1")
    {
        ofxUINumberDialer * toggle = (ofxUINumberDialer *) e.widget;
        smooth_sigma1 = toggle->getValue();
    }else if (e.getName() == "learningRate")
    {
        ofxUINumberDialer * toggle = (ofxUINumberDialer *) e.widget;
        learningRate = toggle->getValue();
    }else if (e.getName() == "frameDiffThresh")
    {
        ofxUINumberDialer * toggle = (ofxUINumberDialer *) e.widget;
        frame_diff_thresh = toggle->getValue();
    }else if (e.getName() == "presenceTimeoutMillis")
    {
        ofxUINumberDialer * toggle = (ofxUINumberDialer *) e.widget;
        presence_timeout_millis = toggle->getValue();
    }else if (e.getName() == "T1"){
    
        ofxUIButton *button = (ofxUIButton *) e.widget; 
        cout << "value: " << button->getValue() << endl;         
        //ofxUIButton * toggle = (ofxUIButton *) e.widget;
        //cout << "updating cam exposture" << endl;
        //updateCamExposure(button->getValue());
    
    }else if( e.getName() == "maxBrightnessDiff"){
    
        ofxUINumberDialer * num = (ofxUINumberDialer *) e.widget;
        maxBrightnessDiff = num->getValue();
    
    }else if( e.getName() == "_minContArea"){
    
        ofxUINumberDialer * num = (ofxUINumberDialer *) e.widget;
        minContArea = num->getValue();
    
    }else if( e.getName() == "_maxContArea"){
    
        ofxUINumberDialer * num = (ofxUINumberDialer *) e.widget;
        minContArea = num->getValue();
    
    }


}
 