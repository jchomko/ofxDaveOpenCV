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

    canDoCalibration = false;
    blobPaths.resize(10);

    cout << "OpenCV version : " << CV_VERSION << endl;
    cout << "Major version : " << CV_MAJOR_VERSION << endl;
    cout << "Minor version : " << CV_MINOR_VERSION << endl;
    cout << "Subminor version : " << CV_SUBMINOR_VERSION << endl;
     

#ifdef DEBUG

    cout << "loading dbug video " << endl;
    debugVideo.load("/root/recordings/test2.mkv");
    debugVideo.setLoopState(OF_LOOP_NORMAL);
    // debugVideo.setPosition(0.22);
    debugVideo.play();

#else

    doCameraSetup();
    
#endif

    //Allocate the Memory for the CV processes
   // colorImg.allocate(width*2,height*2);

    // cout << "Allocating Color Image" << endl;
    // grayImage.allocate(width,height);
    // cout << "Allocating Gray Image" << endl;
    // grayBg.allocate(width,height);
    // cout << "Allocating Back Image" << endl;
    // grayFloatBg.allocate(width,height);
    // cout << "Allocating Float Image" << endl;
    // grayDiff.allocate(width,height);
    // cout << "Allocating Diff Image" << endl;
    // grayWarped.allocate(width,height);
    // cout << "Allocating Warped Image" << endl;
    // lastFrame.allocate(width, height);
    // cout << "Allocating Last Image" << endl;
    // backSubImage.allocate(width,height);
    // cout << "Allocating Diff Image" << endl;
    // frameDiff.allocate(width,height);
    // cout << "Allocating Frame Diff Image" << endl;
    // threshImage.allocate(width,height);
    // cout << "Allocating Thresh Image" << endl;
    // virginGray.allocate(width,height);
    // cout << "Allocating VirginGray Image" << endl;
    // colorImage.allocate(width,height);

    // colorImage.allocate(width, height);

    // backSubContourImage.allocate(width, height);
    // outputGrayscale.allocate(width, height);

    cout << "width : " << width << " height: " << height << endl;
    
//    kinectGray.allocate(width*2,height*2);
    
    //fgrayImage.allocate(width,height);
    //flastGrayImage.allocate(width,height);

//    invbackSubImage.allocate(width,height);
    // cleanFrameDiff.allocate(width,height);

    // outputImage.allocate(width, height,OF_IMAGE_GRAYSCALE);

    outpix = new unsigned char[width*height*4];
    
    for(int i = 0; i <width*height*4;  i++ ){
        outpix[i] = 0;
    }

    backgroundTimer = 0;
    present = true;
    presenceTimer = 0;
    absenceTimer = 0;

    present = false;
    
    pixels = new unsigned char[width*height*4];

    recordFbo.allocate(width, height,GL_RGBA);
    recordFbo.begin();
    ofClear(0);
    recordFbo.end();

    // pathFbo.allocate(width, height);
    learnBackground = true;
    startLearn = true;

    offsetX = 0;
    offsetY = 0;
    
    //History, threshold, detectShadows 
    pBackSub = cv::createBackgroundSubtractorMOG2(500, 28, false);
    // pBackSub = cv::createBackgroundSubtractorKNN();

    // mog.initialize(cvSize(width, height), CV_8UC3); // (100, 16, false);
        
    //Works but isn't that much better than anything else 

    // pBackSub = cv::createBackgroundSubtractorMOG2();
    // pBackSub = cv::BackgroundSubtractorGSOC();


    // // //Dawid Variables to put into gui
    //Original 
    // threshold_min = 200;
    // pre_blur = 1;
    // erosion_size = 3;
    // dilation_size = 8;
    // // max_elem = 4;
    // // max_kernel_size = 11;
    // morph_size = 4; // 32
    // morph_iterations = 1;
    // post_blur = 1;
    // post_erosion_size = 1;
    // expand_size = 1;
    // expand_sigma1 = 0;
    // smooth_size = 1;
    // smooth_sigma1 = 1;
    // learningRate = -1;
    
    //New Ones
    // threshold_min = 200;
    // pre_blur = 1;
    // erosion_size = 1;
    // dilation_size = 2;
    // max_elem = 4;
    // max_kernel_size = 11;

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

    //gui->toggleVisible();

    setupCVGui();  
    outImage.allocate(width,height, OF_IMAGE_GRAYSCALE);

    someoneInLight = false;

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
    
    // error = cam.StopCapture();
    // error = cam.StartCapture();
    // updateCamExposure(true);

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

    // ofxGuiSetFont("AtlasGrotesk-Regular-Web.ttf",10,true,true);
    // ofxGuiSetTextPadding(4);
    // ofxGuiSetDefaultWidth(300);
    // ofxGuiSetDefaultHeight(18);
    // ngui.setup("panel");
    //ngui.add(pre_blur.set("threshold_min",200,0,255));
    
    ggui = new ofxUICanvas(500, 150,200,600);
    ggui->setColorBack(ofColor::black);
    ggui->addWidgetDown(new ofxUINumberDialer(0, 255, 1, 0, "threshold_min1", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 1, 0, "pre_blur", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 3, 0, "erosion_size", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 8, 0, "dilation_size", OFX_UI_FONT_MEDIUM));
    // ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 4, 0, "max_elem", OFX_UI_FONT_MEDIUM));
    // ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 1, 0, "max_kernel_size", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 4, 0, "morph_size", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 1, 0, "morph_iterations", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 1, 1, "post_blur", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 1, 0, "post_erosion_size", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 1, 2, "expand_size", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 1, 0, "expand_sigma1", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 1, 0, "smooth_size", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 10, 1, 0, "smooth_sigma1", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(-1.00f, 1.00f, 0.01f, 1, "learningRate", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 255, 5, 1, "frameDiffThresh", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 8000, 1200, 1, "presenceTimeoutMillis", OFX_UI_FONT_MEDIUM));
    ggui->addWidgetDown(new ofxUINumberDialer(0, 8000, 1200, 1, "maxBrightnessDiff", OFX_UI_FONT_MEDIUM));

    // ggui->addButton("updateExposure", false);
    ggui->addToggle("T1", false, 44, 44);
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
//void CV::DsubtractionLoop(bool bLearnBackground, bool useProgressiveLearn, float progressionRate, bool mirrorH, bool mirrorV,int threshold,int blur, int minBlobSize, int maxBlobSize,int maxBlobNum, bool fillHoles, bool useApproximation,float brightness, float contrast,bool erode,bool dilate)
void CV::DsubtractionLoop(bool mirrorH, bool mirrorV)
{
   // double learningRate = -1.0;
    //  The value between 0 and 1 that indicates how fast the background model is learnt. 
    //  Negative parameter value makes the algorithm to use some automatically chosen learning rate. 
    // 0 means that the background model is not updated at all, 1 means that the background model is completely reinitialized from the last frame.

    //Stop getting exposure
    if(getExposure && ofGetElapsedTimeMillis() > exposureTimer){
        getExposure = false;
        cout << "stopping exposure " << endl;
        updateCamExposure(getExposure);
    }

    int w = 320;
    int h = 240;
    //width = 808;
    //height = 608;
    // --
    //cout << threshold_min << endl;
    bNewFrame = false;

#ifdef DEBUG
    
    debugVideo.update();
    bNewFrame = debugVideo.isFrameNew();

#else
     if(cam.IsConnected()){
        error = cam.RetrieveBuffer( &rawImage );
        
        if (error != PGRERROR_OK){
            PrintError( error );
        }

        bNewFrame = true; 

    }else{
        doCameraSetup();
    }

#endif
    //Dont run processing while we're getting exposure 
    if (bNewFrame && !getExposure){

        #ifdef DEBUG
        colorImage.resize(debugVideo.getWidth(),debugVideo.getHeight());
        colorImage.setFromPixels(debugVideo.getPixels());
        colorImage.resize(width, height);
        colorImage.setImageType(OF_IMAGE_GRAYSCALE);

        // grayImage = colorImage;
    
    #else

        // colorImage.setImageType(OF_IMAGE_GRAYSCALE);
        // grayImage.resize(rawImage.GetCols(), rawImage.GetRows());
        colorImage.setFromPixels(rawImage.GetData(), rawImage.GetCols(), rawImage.GetRows(), OF_IMAGE_GRAYSCALE);
        // grayImage.resize(width, height);


        // cout << "image size cols : " << rawImage.GetCols() << " rows : " << rawImage.GetRows() << endl;
        // cout << "system set to "  << width << ", " << height << endl;
        //cv::cvtColor(grayImage, colorImage, cv::COLOR_GRAY2BGR);
        // colorImage.setFromGrayscalePlanarImages(grayImage, grayImage, grayImage);
    #endif

        // frameDiff = grayImage;
        // frameDiff.absDiff(lastFrame);
        // frameDiff.threshold(frame_diff_thresh);

        // //Frame diff Contour Finder - for decteing presence
        // //contourFinder.findContours(frameDiff, minBlobSize, maxBlobSize, maxBlobNum,fillHoles,useApproximation);
        // contourFinder.findContours(frameDiff, 50, 50000, 1,false,true);

        // //Save the last frame
        // lastFrame = grayImage;

        // //Calling only once to save calculations
        // bool someoneInLight = isSomeoneInTheLight();

        // //Is someone in the light is a function that checks the contourfinder, and is broken out so that it can be called from ofApp.cpp
        // if( !someoneInLight && ofGetElapsedTimeMillis() - backgroundTimer > presence_timeout_millis){

        //             //start getting exposure - but only after we've had someone present
        //             //this should maybe happen long after people have left
        //             //but lets run this for a while now
        //             if(getExposure == false && present){
        //                 cout << "getting exposure " << endl;
        //                 getExposure = true;
        //                 exposureTimer = ofGetElapsedTimeMillis() + 1000;
        //                 updateCamExposure(getExposure);
        //             }
        //            present = false;
        // }
        
        // //this is not an else-if because we want to always be resetting the timer when someone is prsent,
        // if( someoneInLight ){

        //         backgroundTimer = ofGetElapsedTimeMillis();
        //         //While Present
        //         present = true;
        //         //absenceTimer = ofGetElapsedTimeMillis() + 5000;
        // }

        cv::Mat origFrameMat = ofxCv::toCv(colorImage.getPixels()); 

        // cv::Mat(grayImage.getCvImage());
        cv::Mat frameMat = origFrameMat; 


        //Presence detection based on motion 
        // cv::absdiff(origFrameMat, lastFrame, frameDiff);
        // lastFrame = origFrameMat;
    
        // pre blur - doesn't exist in lastest cv
        // cv::medianBlur(frameMat, frameMat, 2*pre_blur+1);

        pBackSub->apply(frameMat, fgMaskMOG2, learningRate);

       	// mog( frameMat, fgMaskMOG2, learningRate );
        // mog.getBackgroundImage(bgMat);
        
        fgMaskMOG2.copyTo(maskOut);

        // cv::threshold(maskOut, maskOut, threshold_min, 255, cv::THRESH_BINARY); // value around 200 removes shadows

        // cv::threshold(maskOut, maskOut, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU); // denoise binary image

        // Erode
        // cv::Mat erosion_kernel = cv::getStructuringElement( cv::MORPH_ELLIPSE,
        //                                             cv::Size( 2*erosion_size + 1, 2*erosion_size+1 ),
        //                                             cv::Point( erosion_size, erosion_size ) );

        // cv::erode(maskOut, maskOut, erosion_kernel);
        // // Dilate
        // cv::Mat dilation_kernel = cv::getStructuringElement( cv::MORPH_ELLIPSE,
        //                                                    cv::Size( 2*dilation_size + 1, 2*dilation_size+1 ),
        //                                                    cv::Point( dilation_size, dilation_size ) );
        // cv::dilate(maskOut, maskOut, dilation_kernel);

        // // Morphology
        //Turn this off to see if it makes the contour better
        cv::Mat morph_element = getStructuringElement( cv::MORPH_ELLIPSE,
                                                      cv::Size( 2*morph_size + 1, 2*morph_size+1 )); //,
                                                       // cv::Point( morph_size, morph_size ));
                                                        //);
                                                        //was morph_size, morph_size but I don't think it does anything

        cv::morphologyEx( maskOut, maskOut, cv::MORPH_CLOSE, morph_element, cv::Point(-1,-1), morph_iterations, cv::BORDER_CONSTANT );

        // post Erode
        
        // cv::Mat post_erosion_kernel = cv::getStructuringElement( cv::MORPH_ELLIPSE,
        //                                                    cv::Size( 2*post_erosion_size + 1, 2*post_erosion_size+1 ),
        //                                                    cv::Point( post_erosion_size, post_erosion_size ) );
        // cv::erode(maskOut, maskOut, post_erosion_kernel);

        // cv::medianBlur(maskOut, maskOut, 2*post_blur+1);


        cv::Mat mask1 = cv::Mat(w, h, CV_8UC1);
        mask1 = cv::Scalar::all(0);
        maskOut.copyTo(mask1);


        cv::Mat mask2 = cv::Mat(h, w, CV_8UC1);
        mask2 = cv::Scalar::all(0);

        // maskOut.copyTo(mask2);
        // mask2 = cv::Scalar::all(0);


        // This contour finder doesn't seem to add anything
        cv::Mat bw;
        maskOut.copyTo(bw);

        //So if we were going to be really fast, we would just save arrays of contours, and then draw them when we need them
        //Not save whole images 
        //But that's maybe too much 

        std::vector<std::vector<cv::Point> > contours;
        cv::Mat cont_hierarchy;
        
        // cv::findContours(bw, contours, cont_hierarchy, cv::RETR_FLOODFILL, cv::CHAIN_APPROX_SIMPLE);
        cv::findContours(bw, contours, cont_hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        
        std::vector<std::vector<cv::Point> > approx( contours.size() );
         // std::vector<std::vector<cv::Point> > hull( contours.size() );

        if(contours.size() > 0){
            someoneInLight = true;
        }else{
            someoneInLight = false;
        }

        if( !someoneInLight && ofGetElapsedTimeMillis() - backgroundTimer > presence_timeout_millis){

                    //start getting exposure - but only after we've had someone present
                    //this should maybe happen long after people have left
                    //but lets run this for a while now
                    if(getExposure == false && present){
                        cout << "getting exposure " << endl;
                        getExposure = true;
                        exposureTimer = ofGetElapsedTimeMillis() + 1000;
                        updateCamExposure(getExposure);
                    }
                   present = false;
        }
        
        //this is not an else-if because we want to always be resetting the timer when someone is prsent,
        if( someoneInLight ){

                backgroundTimer = ofGetElapsedTimeMillis();
                //While Present
                present = true;
                //absenceTimer = ofGetElapsedTimeMillis() + 5000;
        }

//         for (size_t idx = 0; idx < contours.size(); idx++) {
// //            cv::vector<cv::Point> approx;

//             cv::approxPolyDP( cv::Mat(contours[idx]),
//                               approx[idx],
//                               cv::arcLength(cv::Mat(contours[idx]), true)*0.002, true );

//             // Skip small or non-convex objects
//             //This works pretty well, but just drops frames if somethign is too big
//             //Mabye we should use save the contours and re-draw if they're not good
//             //but needs to be tested real-world. Add these to the UI 

//             int minContArea = 120;
//             //single person from 3m seems to have a max of about 5000
//             //But this needs to be tested with multiple people, and more precise headlights

//             int maxContArea = 5000;
//             //|| !cv::isContourConvex(contours[idx])
//             auto area = cv::contourArea(contours[idx]);

//             if( area > minContArea &&  area < maxContArea){
//                 //destination image, contours, contour to draw, colour, linetype
//                 cv::drawContours(mask2, approx, idx, cv::Scalar(255), -1);  
//                 // cv::drawContours(mask2, contours, idx, cv::Scalar(255), -1);  

//                 // cout << "drawing: " << cv::contourArea(contours[idx]) << endl;
//               }else{
//                 // cout << "not drawing: " << cv::contourArea(contours[idx]) << endl;
//             }
//     //        cv::convexHull(cv::Mat(approx[idx]), hull[idx]);
// //            int minContArea = 100;
// //            if (std::fabs(cv::contourArea(hull[idx])) < minContArea)
// //                continue;
//   //          cv::drawContours(mask2, hull, idx, cv::Scalar(255), -1);
//             }


        cv::GaussianBlur(mask2, mask2, cv::Size(smooth_size, smooth_size), smooth_sigma1); // smooth edges
        keyOut2 = mask2;

        // cv::GaussianBlur(maskOut, maskOut, cv::Size(expand_size, expand_size), expand_sigma1); // expand edges
/*      cv::GaussianBlur(mask2, mask2, cv::Size(expand_size, expand_size), expand_sigma1); // expand edges
        mask2.convertTo(mask2, CV_32FC1);
        cv::GaussianBlur(mask2, mask2, cv::Size(smooth_size, smooth_size), smooth_sigma1); // smooth edges
*/
	    
        //Generate a white image
        // cv::Mat whiteMat = cv::Mat(origFrameMat.size(),CV_32FC1);
        // whiteMat = cv::Scalar::all(1.0);

        /*
        * Prepare keyOut
        */
        // cv::GaussianBlur(mask1, mask1, cv::Size(expand_size, expand_size), expand_sigma1); // expand edges
        // mask1.convertTo(mask1, CV_32FC1);

        // cv::GaussianBlur(mask1, mask1, cv::Size(smooth_size, smooth_size), smooth_sigma1); // smooth edges

        // if (keyOut.rows < origFrameMat.rows || keyOut.cols < origFrameMat.cols) {
        //     keyOut = cv::Mat(origFrameMat.size(),CV_32FC1);
        // }
 
 	    //multiply the white image by the mask image
        // keyOut = whiteMat.mul(mask1);

    	//convert to 8 bit single-channel array

        // keyOut.convertTo(keyOut, CV_8UC1);

        //subtract(src1, src2, destination)
	    //subtract the key from a black white image
	    // cv::subtract(cv::Scalar::all(255),keyOut,keyOut);

	    //we're skipping this for now i'm not sure why
        /*
         * Prepare keyOut2
         */
        /*
	    if (keyOut2.rows < origFrameMat.rows || keyOut2.cols < origFrameMat.cols) {
            keyOut2 = cv::Mat(origFrameMat.size(),CV_32FC1);
        }
        keyOut2 = whiteMat.mul(mask2);
        keyOut2.convertTo(keyOut2, CV_8UC1);
        cv::subtract(cv::Scalar::all(255),keyOut2,keyOut2);
	    */

	    //This section dectects presence to see if we should record the images we're capturing
        //Calculate frame difference
        
	//Draw Mat to get pixels
	//This seems a bit much
        // recordFbo.begin();
        //    ofSetColor(255, 255, 255);
        //    //drawMat(keyOut2, 0, 20,width/2,height/2);
        //    drawMat( keyOut , 0,0 ,width,height);
        //    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        // recordFbo.end();

        //  pix.resize(808,608);
 //    lastBrightness = imgBrightness;
 //    imgBrightness = 0;

	// for(int x =0; x < keyOut.rows; x++){
	// 	for(int y = 0; y < keyOut.cols; y++){
	// 		imgBrightness += keyOut.ptr<uchar>(y)[x]/255;
	// 	}
	// }

 //    // if(abs(lastBrightness-imgBrightness) < maxBrightnessDiff){

        // ofPixels op = keyOut.data;
        //Is someone in the light is a function that checks the contourfinder, and is broken out so that it can be called from ofApp.cpp
        
        
        //Invert image 
        maskOut = ~maskOut;

        ofxCv::toOf(maskOut, pix); 
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
bool CV::isSomeoneThere()
{
    if (present == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}
//--------------------------------------------------------------
void CV::setTrackingBoundaries(int _offsetX, int _offsetY)
{
    offsetX = _offsetX;
    offsetY = _offsetY;
}
//--------------------------------------------------------------
bool CV::isSomeoneInTheLight()
{
    bool b = false;

    // if (contourFinder.nBlobs > 0)
    // {
    //     for (int i = 0; i < contourFinder.nBlobs; i++)
    //     {
    //         if (contourFinder.blobs[i].centroid.x >= offsetX && contourFinder.blobs[i].centroid.x <= width-(offsetX) && contourFinder.blobs[i].centroid.y >= offsetY && contourFinder.blobs[i].centroid.y <= height-(offsetY))
    //         {
    //             b = true;
    //         }
    //         else
    //         {
    //             b = false;
    //         }
    //     }
    // }
    // else
    // {
    //     b = false;
    // }

    return b;
}
//--------------------------------------------------------------
// int CV::getNumberOfBlobs()
// {
//     // return contourFinder.nBlobs;
// }
//--------------------------------------------------------------
void CV::relearnBackground()
{
    learnBackground = true;
    startLearn = true;
    if (startLearn == true)
    {
//        grayBg = grayWarped;
        startLearn = false;
    }
}
//------------------------------------------------------------
void CV::toggleGui()
{
//  showGui = !showGui;
    ggui->toggleVisible();

}
//-------------------------------------------------------------
void CV::drawGui()
{
//   if(showGui){
//      gui.draw();
//  }
}
//--------------------------------------------------------------
void CV::drawLiveShadow()
{
    ofSetColor(255, 255, 255);
    recordFbo.draw(0,0,ofGetWidth(),ofGetHeight());
}
//--------------------------------------------------------------
void CV::setCalibration(bool setMode)
{
    canDoCalibration = setMode;
}
//--------------------------------------------------------------
void CV::drawCalibration()
{
    ofPushStyle();
    ofSetColor(255);
    ofPushMatrix();
    ofTranslate(ofGetWidth()-width*2, ofGetHeight()-height);
    // virginGray.draw(0, 0,width,height);
    ofDrawBitmapStringHighlight("Warper",0+5,15);
    ofSetColor(255);
    // grayWarped.draw(width,0,width,height);
    ofDrawBitmapStringHighlight("Warped Img",width+5,15);

    // if (canDoCalibration == true)
    // {
    //     cvWarpQuad.draw(0, 0, width, height,0,255,0,2);
    // }
    // else
    // {
    //     cvWarpQuad.draw(0, 0, width, height,255,0,0,2);
    // }

    ofPopStyle();
    ofPopMatrix();

}
//--------------------------------------------------------------
void CV::drawAllPaths()
{
    // for (int i = 0; i < blobPath.size(); i++)
    // {
    //     ofNoFill();
    //     ofBeginShape();
    //     ofSetColor(255, 255, 0);
    //     for (int p = 0; p < blobPath.size(); p++)
    //     {
    //         ofVertex(blobPath[p].x,blobPath[p].y);
    //     }
    //     ofEndShape(false);
    // }
}
//--------------------------------------------------------------
void CV::getAllPaths()
{
    // for (int i = 0; i < contourFinder.nBlobs; i++)
    // {
    //     blobPaths[0].bPath.push_back(contourFinder.blobs[0].centroid);
    // }
}
//--------------------------------------------------------------
void CV::drawTracking()
{
    ofPushMatrix();
    ofPushStyle();
    ofTranslate(ofGetWidth()-width*2, 0);
    ofSetColor(0, 0, 0);
    ofFill();
    //ofRect(0, 0, width, height);
    //ofSetColor(255, 255, 255);
    // contourFinder.draw(0,0,width,height);

    // ofNoFill();
    // if (isSomeoneInTheLight()) {
    //     ofSetColor(0, 255, 0);
    // }
    // else
    // {
    //     ofSetColor(255,25,0);
    // }
    // ofRect(offsetX,offsetY,width-(offsetX*2),height-(offsetY*2));

    // ofSetColor(255, 255, 255);

    // for (int i = 0; i < contourFinder.nBlobs; i++)
    // {
    //     ofDrawBitmapStringHighlight(ofToString(i), contourFinder.blobs[i].boundingRect.x+5,contourFinder.blobs[i].boundingRect.y+10);
    //     ofSetColor(255, 0, 0);
    //     ofCircle(contourFinder.blobs[i].centroid, 3);
    // }

    // if (!blobPaths.empty())
    // {
    //     ofNoFill();

    //     for (int o = 0; o < blobPaths.size(); o++)
    //     {
    //         ofBeginShape();
    //         for (int i = 0; i < blobPaths[o].bPath.size(); i++)
    //         {
    //             ofVertex(blobPaths[o].bPath[i].x,blobPaths[o].bPath[i].y);
    //         }
    //         ofEndShape(false);
    //     }
    // }
    ofPopStyle();
    ofPopMatrix();

    float brightnessDiff = imgBrightness - lastBrightness;
    ofDrawBitmapStringHighlight("Image Brightness : " + ofToString(imgBrightness) + " Brightness diff : " + ofToString(brightnessDiff), 0, 170);
    ofDrawBitmapStringHighlight("Framerate  : " + ofToString(ofGetFrameRate()), 0, 190);
       
}
//--------------------------------------------------------------
void CV::drawLive()
{
    
//    ofSetColor(255);
//    ofDrawBitmapStringHighlight("Live",ofGetWidth()-width+5,15);
//    grayImage.draw(ofGetWidth()-width,0,width,height);
//    ofPushMatrix();
//    ofTranslate(ofGetWidth()-width, 0);
//    ofBeginShape();
//    ofNoFill();
//    ofSetColor(255, 0, 0);
//    for (int i = 0; i < 4; i++)
//    {
//        if (i == 0) {
//            ofVertex(dstPts[0]);
//            ofVertex(dstPts[3]);
//        }
//        ofVertex(dstPts[i]);
//
//    }
//    ofEndShape(false);
//    ofPopMatrix();
}
//--------------------------------------------------------------
void CV::draw()
{

    drawCalibration();
    //ofPushMatrix();
    //ofTranslate(0, height);
    ofFill();
    ofSetColor(255);
    
    
    float x = 0.f, y = 0.f;
    
    ofPushMatrix();
    ofTranslate(x, y);
    ofDrawBitmapStringHighlight("colorImage",5,15);
    colorImage.draw(0,20,width/2,height/2);
    ofPopMatrix();
    x += width/2;
    
    
    ofPushMatrix();
    ofTranslate(x, y);
    ofDrawBitmapStringHighlight("outImage",5,15);
    outImage.draw(0,20,width/2,height/2);
    ofPopMatrix();
    x += width/2;
    
//     ofPushMatrix();
//     ofTranslate(x, y);
//     ofDrawBitmapStringHighlight("grayBg",5,15);
//     grayBg.draw(0,20,width/2,height/2);
//     ofPopMatrix();
//     x += width/2;
    
    ofPushMatrix();
    // ofTranslate(x, y);
    ofDrawBitmapStringHighlight("contour Finder mask",5,15);
    ofPixels contourFinderPix; 
    ofImage contourFinderImg;
    ofxCv::toOf(keyOut2, contourFinderPix);
    contourFinderImg.setFromPixels(contourFinderPix);
    contourFinderImg.draw(0,20,width/2,height/2);
    // contourFinderImg.draw(0,0,ofGetWidth(),ofGetHeight());

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
    
    
    
//     ofPushMatrix();
//     ofTranslate(x, y);
//     ofDrawBitmapStringHighlight("fgMask",5,15);
//     // drawMat(fgMaskMOG2, 0, 20,width/2,height/2);
//     ofPopMatrix();
// //    x += width/2;
    
    
//     ofPushMatrix();
//     ofTranslate(x, y + height/2 + 20);
//     ofDrawBitmapStringHighlight("bgMat",5,15);
//     // drawMat(bgMat, 0, 20,width/2,height/2);
//     ofPopMatrix();
//     x += width/2;
    
    // ofPushMatrix();
    // ofTranslate(x, y);
    // ofDrawBitmapStringHighlight("maskOut",5,15);
    // // drawMat(maskOut, 0, 20,width/2,height/2);
    // ofPopMatrix();

    
//     ofPushMatrix();
//     ofTranslate(x, y + height/2 + 20);
//     ofDrawBitmapStringHighlight("keyOut",5,15);
//     // drawMat(keyOut, 0, 20,width,height);
//     ofPopMatrix();
    
//     ofPushMatrix();
//     ofTranslate(x, y + (height+ 20)*2);
//     ofDrawBitmapStringHighlight("keyOut2",5,15);
//     // drawMat(keyOut2, 0, 20,width,height);
//     ofPopMatrix();
    
  
   
//     virginGray.draw(ofGetWidth() - 2*width,0,width,height);
//     ofDrawBitmapStringHighlight("Virgin Gray",ofGetWidth() - 2*width, 15);
//     //Draws on top of Virgin Gray
//     drawTracking();
    
    
    recordFbo.draw(ofGetWidth()- 2*width,height,width,height);
    ofDrawBitmapStringHighlight("Record Fbo",ofGetWidth() - 2*width, height+15);

    drawTracking();
   
    
    //ofPopMatrix();
//ngui.draw();

}

void CV::drawCameraFullScreen(){
	
	 // grayImage.draw(0,0, ofGetWidth(),ofGetHeight());

}
//--------------------------------------------------------------
ofPixels CV::getRecordPixels()
{
    return pix;
}
//--------------------------------------------------------------
// ofVec2f CV::getBlobPath()
// {
//     if (contourFinder.nBlobs > 0)
//     {
//         for (int i = 0; i < contourFinder.nBlobs; i++)
//         {
//             //If the Blob has an Area larger than 100 pixels find it and return its centroid
//             if (contourFinder.blobs[i].area >= 100)
//             {
//                 blobPath.push_back(contourFinder.blobs[0].centroid);
//                 return contourFinder.blobs[0].centroid;
//             }
//         }
//     }
//     else
//     {
//         //return ofVec2f(320/2,240/2);
//     }
// }
// //--------------------------------------------------------------
// void CV::mouseDragged(int x, int y, int button){
//     cvWarpQuad.updatePoint(x, y, 0,0,width,height);
// }
// //--------------------------------------------------------------
// void CV::mousePressed(int x, int y, int button){
//     cvWarpQuad.selectPoint(x, y,0,0,width,height,30);

// }
// //--------------------------------------------------------------
// void CV::mouseReleased(int x, int y, int button){
//     cvWarpQuad.saveToFile("quad-settings.xml");
// }
/*
cvWarpQuad.updatePoint(x, y, 10+200,ofGetHeight()/2-camH, 640,480);
cvWarpQuad.selectPoint(x, y, 10+200,ofGetHeight()/2-camH,640,480,30);
cvWarpQuad.saveToFile("quad-settings.xml");
/--------------------------------------------------------------
vector<ofVec3f> CV::getBlobsCentroid()
{
    centroids.clear();
    if (contourFinder.nBlobs > 0)
    {
        for (int i = 0; i < contourFinder.nBlobs; i++)
        {
            //If the Blob has an Area larger than 100 pixels find it and return its centroid
            if (contourFinder.blobs[i].area >= 100)
            {
                centroids.assign(i,contourFinder.blobs[i].centroid);
            }
        }
    }
    return centroids;
}
*/

void CV::PrintError (Error error){
    error.PrintErrorTrace();
   // cout << error << endl;

}

void CV::PrintCameraInfo( CameraInfo* pCamInfo )
{
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

void CV::exit()
{
    ggui->saveSettings("GUI/CVSettings.xml");
    delete ggui;
}


void CV::guiEventCV(ofxUIEventArgs &e)
{
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
    
    }

}
 