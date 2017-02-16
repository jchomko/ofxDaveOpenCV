//--------------------------------------------------------------
//* Name: OCV.cpp
//* Project: Playable City 2014 Award
//* Author: David Haylock
//* Creation Date: 28-07-2014
//--------------------------------------------------------------

#include "OCV.h"

using namespace ofxCv;



//--------------------------------------------------------------
void CV::setup( int width,int height, int framerate)
{
    // Variables Over
    _width = width;
    _height = height;
    
    canDoCalibration = false;
    blobPaths.resize(10);
    // Grabber initiallization
   
#ifdef DEBUG
    debugVideo.loadMovie("video.mp4");
    debugVideo.play();
#else
    vidGrabber.listDevices();
    vidGrabber.setDeviceID(0);
    vidGrabber.setDesiredFrameRate(framerate);
    vidGrabber.initGrabber(width,height);
    //vidGrabber.setFlicker(0); /* 0 - no flicker, 1 - 50hz, 2 - 60hz */
#endif

    //Allocate the Memory for the CV processes
    colorImg.allocate(width,height);
    cout << "Allocating Color Image" << endl;
	grayImage.allocate(width,height);
    cout << "Allocating Gray Image" << endl;
	grayBg.allocate(width,height);
    cout << "Allocating Back Image" << endl;
    grayFloatBg.allocate(width,height);
    cout << "Allocating Float Image" << endl;
	grayDiff.allocate(width,height);
    cout << "Allocating Diff Image" << endl;
    grayWarped.allocate(width,height);
    cout << "Allocating Warped Image" << endl;
    lastFrame.allocate(width, height);
    cout << "Allocating Last Image" << endl;
    diffImage.allocate(width,height);
    cout << "Allocating Diff Image" << endl;
    frameDiff.allocate(width,height);
    cout << "Allocating Frame Diff Image" << endl;
    threshImage.allocate(width,height);
    cout << "Allocating Thresh Image" << endl;
    virginGray.allocate(width,height);
    cout << "Allocating VirginGray Image" << endl;

    outputImage.allocate(width, height,OF_IMAGE_GRAYSCALE);
    outpix = new unsigned char[width*height*4];
    
    for(int i = 0; i <width*height*4;  i++ ){
        outpix[i] = 0;
    }
    backgroundTimer = 0;
    present = true;
    presenceTimer = 0;
    absenceTimer = 0;
    
    present = true;
    pixels = new unsigned char[_width*_height*4];
    
    recordFbo.allocate(width, height,GL_RGBA);
    recordFbo.begin();
    ofClear(0);
    recordFbo.end();

    //learnBackground = true;
    startLearn = true;
    
    _offsetX = 0;
    _offsetY = 0;
    
    
    srcPts[0].set(0, 0);
	srcPts[1].set(_width, 0);
	srcPts[2].set(_width, _height);
	srcPts[3].set(0, _height);
	
    dstPts[0].set(0, 0);
	dstPts[1].set(_width, 0);
	dstPts[2].set(_width, _height);
	dstPts[3].set(0, _height);
    
    cvWarpQuad.setup("Quad_");
    cvWarpQuad.setup("Masker-Quad");
    cvWarpQuad.setQuadPoints(srcPts);
    cvWarpQuad.readFromFile("quad-settings.xml");
    
//    cv::Ptr::  cv::BackgroundSubtractorMOG2()
    
    //create Background Subtractor objects
    //pMOG2 = new cv::BackgroundSubtractorMOG2(); //MOG2 approach
    
    mog.initialize(cvSize(_width, _height), CV_8UC3); // (100, 16, false);
    
}
//--------------------------------------------------------------
void CV::setTrackingBoundaries(int x, int y, int w, int h)
{
 
    
    
}
//--------------------------------------------------------------
void CV::releaseCamera()
{
    #ifndef DEBUG
    vidGrabber.close();
    #endif
}
//--------------------------------------------------------------
void CV::subtractionLoop(bool bLearnBackground, bool useProgressiveLearn, float progressionRate, bool mirrorH, bool mirrorV,int threshold, int blur,int minBlobSize, int maxBlobSize,int maxBlobNum, bool fillHoles, bool useApproximation,bool erode,bool dilate)
{
    
    bool bNewFrame = false;
    
    
#ifdef DEBUG
    debugVideo.update();
    bNewFrame = debugVideo.isFrameNew();
#else
    vidGrabber.update();
    bNewFrame = vidGrabber.isFrameNew();
#endif

    if (bNewFrame)
    {
#ifdef DEBUG
        colorImg.setFromPixels(debugVideo.getPixels(),_width,_height);
#else
        colorImg.setFromPixels(vidGrabber.getPixels(), _width,_height);
#endif
    
        colorImg.mirror(mirrorV, mirrorH);
        grayImage = colorImg;
    
        if (useProgressiveLearn == true)
        {
            grayFloatBg.addWeighted(grayImage, progressionRate);
            grayBg = grayFloatBg;
        }
        else
        {
            if (learnBackground == true)
            {
                grayBg = grayWarped;
                learnBackground = false;
                //startLearn = false;
            }
        }
        
        // We get back the warped coordinates - scaled to our camera size
		ofPoint * warpedPts = cvWarpQuad.getScaledQuadPoints(_width, _height);
        
		// Lets warp with those cool coordinates!!!!!
		grayWarped.warpIntoMe(grayImage, warpedPts, dstPts);
		
		// Lets calculate the openCV matrix for our coordWarping
		coordWarp.calculateMatrix(warpedPts, dstPts);
        
        if (erode)
        {
            grayImage.erode();
        }
        if (dilate)
        {
            grayImage.dilate();
        }
        
        grayWarped.blurGaussian(blur);
        grayDiff.absDiff(grayBg, grayWarped);
        grayDiff.threshold(threshold);
        contourFinder.findContours(grayDiff, minBlobSize, maxBlobSize, maxBlobNum,fillHoles,useApproximation);
    }
    learnBackground = bLearnBackground;
}
//--------------------------------------------------------------
void CV::subtractionLoop(bool bLearnBackground, bool useProgressiveLearn, float progressionRate, bool mirrorH, bool mirrorV,int threshold,int blur, int minBlobSize, int maxBlobSize,int maxBlobNum, bool fillHoles, bool useApproximation,float brightness, float contrast,bool erode,bool dilate)
{
    
    bool bNewFrame = false;
    
#ifdef DEBUG
    debugVideo.update();
    bNewFrame = debugVideo.isFrameNew();
#else
    vidGrabber.update();
    bNewFrame = vidGrabber.isFrameNew();
#endif
    
    if (bNewFrame)
    {
        
#ifdef DEBUG
        colorImg.setFromPixels(debugVideo.getPixels(),_width,_height);
#else
        colorImg.setFromPixels(vidGrabber.getPixels(), _width,_height);
#endif
        colorImg.mirror(mirrorV, mirrorH);
        grayImage = colorImg;
        
        if (useProgressiveLearn == true)
        {
            grayFloatBg.addWeighted(grayImage, progressionRate);
            grayBg = grayFloatBg;
        }
        else
        {
            if (bLearnBackground == true)
            {
                grayBg = grayImage;
                bLearnBackground = false;
            }
        }
        
        if (erode)
        {
            grayImage.erode();
        }
        if (dilate)
        {
            grayImage.dilate();
        }
        
        grayImage.blurGaussian(blur);
        grayImage.brightnessContrast(brightness, contrast);
        grayDiff.absDiff(grayBg, grayImage);
        grayDiff.threshold(threshold);
        contourFinder.findContours(grayDiff, minBlobSize, maxBlobSize, maxBlobNum,fillHoles,useApproximation);
    }
}
//--------------------------------------------------------------
void CV::JsubtractionLoop(bool bLearnBackground,bool mirrorH,bool mirrorV,int threshold, int blur,int minBlobSize, int maxBlobSize,int maxBlobNum,bool fillHoles, bool useApproximation,float brightness,float contrast)
{
 
    bool bNewFrame = false;

#ifdef DEBUG
    debugVideo.update();
    bNewFrame = debugVideo.isFrameNew();
#else
    vidGrabber.update();
    bNewFrame = vidGrabber.isFrameNew();
#endif
    
    if (bNewFrame)
    {
        
#ifdef DEBUG
        colorImg.setFromPixels(debugVideo.getPixels(),_width,_height);
#else
        colorImg.setFromPixels(vidGrabber.getPixels(), _width,_height);
#endif
        colorImg.mirror(mirrorV, mirrorH);
        //colorImg.invert();
    
        /* We get back the warped coordinates - scaled to our camera size
		ofPoint * warpedPts = cvWarpQuad.getScaledQuadPoints(_width, _height);
        
		// Lets warp with those cool coordinates!!!!!
		grayWarped.warpIntoMe(grayImage, warpedPts, dstPts);
		
		// Lets calculate the openCV matrix for our coordWarping
		coordWarp.calculateMatrix(warpedPts, dstPts);
        */
        //virginGray = colorImg;
        grayImage = colorImg;

        //frameDiff = colorImg;
        grayImage.brightnessContrast(brightness, contrast);

	    frameDiff = grayImage;	
        diffImage = grayImage;	

        //frameDiff.brightnessContrast(brightness, contrast);
        
        //FrameDiff
        frameDiff.absDiff(lastFrame);
        frameDiff.threshold(threshold);
        
         //Frame diff Contour Finder
        contourFinder.findContours(frameDiff, minBlobSize, maxBlobSize, maxBlobNum,fillHoles,useApproximation);
        

        //Background sub for static background
        //grayImage.absDiff(backImage);
        
        //diffImage = colorImg;
        
        //diffImage.absDiff(grayBg);
       
        //diffImage += frameDiff;
        
       
        
        //Contour fining
        //frameDiff.threshold(threshold);
        //frameDiff.adaptiveThreshold(240);

       
        //unsigned char * diffpix = grayImage.getPixels();
        
        //unsigned char * threshpix = diffImage.getPixels();
        
        //Image creation
       // diffImage = grayBg;
        //diffImage -= grayBg;
        //diffImage.invert();
        
        //diffImage.threshold(threshold);

        diffImage = frameDiff;

        diffImage.blur(blur);

        diffImage.adaptiveThreshold(threshold);
        
        //diffImage.adaptiveThreshold(blur);
        diffImage.blur(blur);


        diffImage.invert();
        
        //diffImage.dilate();

        //diffImage.invert();

        //        int c = 0;
        
        // for (int i = 0; i < _width*_height*4; i ++)
        // {
        //     if( threshpix[i] > threshold)
        //     { // used to be 6
        //         outpix[i] = ofClamp(diffpix[i]/5, 0, 255);
        //     }
        //     else
        //     {
        //         outpix[i] = 255;
        //     }
        // }
        
        // virginGray = diffImage;
        //lastFrame = colorImg;
	    //lastFrame.brightnessContrast(brightness, contrast);
        //outputImage = diffImage;
        outputImage.setFromPixels(diffImage.getPixels(), diffImage.getWidth(), diffImage.getHeight(), OF_IMAGE_GRAYSCALE);


        lastFrame = colorImg;
        pastImages.push_back(lastFrame);

        //outputImage.setFromPixels(diffImage.getPixels())
        //outputImage.setFromPixels(outpix, _width, _height, OF_IMAGE_GRAYSCALE);
        //pix.setFromPixels(outputImage.getPixels(), 320, 240, 4);
        //pix.setFromPixels(outpix, 320, 240, 4);
    }
    
    //For better bacgkround subtraction I'm saving past images and using them to subtract
    
    if(pastImages.size() > 50)
    {
        pastImages.erase(pastImages.begin());
    }
    
    //On Exit
    //this just checks if there's movement, and the presenceFinder contourFinder
    // decides if there's a person there
    //this needs to be more robust - so that it's not timer based, but knows when people are offscreen
    
    if(contourFinder.nBlobs == 0 && ofGetElapsedTimeMillis() - backgroundTimer >  2000 )
    {
        if(pastImages.size() > 0)
        {
            grayBg = pastImages[0];
        }
    
        present = false;
    }
    
    
    
    if(contourFinder.nBlobs > 0)
    {
        backgroundTimer = ofGetElapsedTimeMillis();
        //While Present
        present = true;
        absenceTimer = ofGetElapsedTimeMillis() + 5000;
 
    }

}







//--------------------------------------------------------------
//void CV::DsubtractionLoop(bool bLearnBackground, bool useProgressiveLearn, float progressionRate, bool mirrorH, bool mirrorV,int threshold,int blur, int minBlobSize, int maxBlobSize,int maxBlobNum, bool fillHoles, bool useApproximation,float brightness, float contrast,bool erode,bool dilate)
void CV::DsubtractionLoop(bool mirrorH, bool mirrorV)
{
    double learningRate = -1.0;
    //  The value between 0 and 1 that indicates how fast the background model is learnt. Negative parameter value makes the algorithm to use some automatically chosen learning rate. 0 means that the background model is not updated at all, 1 means that the background model is completely reinitialized from the last frame.
    
    int threshold_min = 200;
    
    int pre_blur = 5;
    
    int erosion_size = 5;
    int dilation_size = 32;
    int const max_elem = 2;
    int const max_kernel_size = 21;
    
    int morph_size = 16; // 32
    int morph_iterations = 2;
    
    int post_blur = 5;
    
    // --
    
    bool bNewFrame = false;
    
#ifdef DEBUG
    debugVideo.update();
    bNewFrame = debugVideo.isFrameNew();
#else
    vidGrabber.update();
    bNewFrame = vidGrabber.isFrameNew();
#endif
    
    if (bNewFrame)
    {
        
#ifdef DEBUG
        colorImg.setFromPixels(debugVideo.getPixels(),_width,_height);
#else
        colorImg.setFromPixels(vidGrabber.getPixels(), _width,_height);
#endif
        colorImg.mirror(mirrorV, mirrorH);
        grayImage = colorImg;
        
        
        cv::Mat origFrameMat = cv::Mat(colorImg.getCvImage());
        
        cv::Mat frameMat = cv::Mat(colorImg.getCvImage());
        
        // pre blur
        cv::medianBlur(frameMat, frameMat, pre_blur);
        
        mog(frameMat, fgMaskMOG2, learningRate);
        mog.getBackgroundImage(bgMat);
        
        fgMaskMOG2.copyTo(maskOut);
        
        cv::threshold(maskOut, maskOut, threshold_min, 255, cv::THRESH_BINARY); // value around 200 removes shadows
        cv::threshold(maskOut, maskOut, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU); // denoise binary image
        
        
        // Erode
        cv::Mat erosion_kernel = cv::getStructuringElement( cv::MORPH_ELLIPSE,
                                                    cv::Size( 2*erosion_size + 1, 2*erosion_size+1 ),
                                                    cv::Point( erosion_size, erosion_size ) );

        
        cv::erode(maskOut, maskOut, erosion_kernel);
        
        // Dilate
        cv::Mat dilation_kernel = cv::getStructuringElement( cv::MORPH_ELLIPSE,
                                                           cv::Size( 2*erosion_size + 1, 2*erosion_size+1 ),
                                                           cv::Point( erosion_size, erosion_size ) );
        
        
        
        
        cv::dilate(maskOut, maskOut, dilation_kernel);
        
        // Morphology
        cv::Mat morph_element = getStructuringElement( cv::MORPH_ELLIPSE,
                                                      cv::Size( 2*morph_size + 1, 2*morph_size+1 ),
                                                      cv::Point( morph_size, morph_size ) );

        
        cv::morphologyEx( maskOut, maskOut, cv::MORPH_CLOSE, morph_element, cv::Point(-1,-1), morph_iterations, cv::BORDER_CONSTANT );
        
        
        cv::medianBlur(maskOut, maskOut, post_blur);
        
        
        cv::Mat mask1 = cv::Mat(_width, _height, CV_8UC1);
        mask1 = cv::Scalar::all(0);
        maskOut.copyTo(mask1);
        
        cv::Mat mask2 = cv::Mat(_width, _height, CV_8UC1);
        mask2 = cv::Scalar::all(0);
        
        maskOut.copyTo(mask2);
        mask2 = cv::Scalar::all(0);
        
        cv::Mat bw;
        maskOut.copyTo(bw);

        std::vector<std::vector<cv::Point> > contours;
        cv::Mat cont_hierarchy;
        cv::findContours(bw, contours, cont_hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        
        std::vector<std::vector<cv::Point> > approx( contours.size() );
        std::vector<std::vector<cv::Point> > hull( contours.size() );
        for (size_t idx = 0; idx < contours.size(); idx++) {
//            cv::vector<cv::Point> approx;
            cv::approxPolyDP( cv::Mat(contours[idx]),
                              approx[idx],
                              cv::arcLength(cv::Mat(contours[idx]), true)*0.02, true );
            
            // Skip small or non-convex objects
            int minContArea = 100;
//            if (std::fabs(cv::contourArea(contours[idx])) < minContArea || !cv::isContourConvex(contours[idx]))
//                continue;
            
            cv::drawContours(maskOut, approx, idx, cv::Scalar(255), -1);
            
            cv::convexHull(cv::Mat(approx[idx]), hull[idx]);
//            int minContArea = 100;
//            if (std::fabs(cv::contourArea(hull[idx])) < minContArea)
//                continue;
            
            cv::drawContours(mask2, hull, idx, cv::Scalar(255), -1);
        }
        
        
        
        // ---
        
        cv::GaussianBlur(maskOut, maskOut, cv::Size(21, 21), 11.0); // expand edges
        
        // ---
        
        cv::GaussianBlur(mask2, mask2, cv::Size(21, 21), 11.0); // expand edges
        
        mask2.convertTo(mask2, CV_32FC1);
        cv::GaussianBlur(mask2, mask2, cv::Size(21, 21), 11.0); // smooth edges
        
        // ---
        
        cv::Mat whiteMat = cv::Mat(origFrameMat.size(),CV_32FC1);
        whiteMat = cv::Scalar::all(1.0);
        
        
        /*
         * Prepare keyOut
         */
        cv::GaussianBlur(mask1, mask1, cv::Size(21, 21), 11.0); // expand edges
        
        mask1.convertTo(mask1, CV_32FC1);
        cv::GaussianBlur(mask1, mask1, cv::Size(21, 21), 11.0); // smooth edges
        
        if (keyOut.rows < origFrameMat.rows || keyOut.cols < origFrameMat.cols) {
            keyOut = cv::Mat(origFrameMat.size(),CV_32FC1);
        }
        keyOut = whiteMat.mul(mask1);
        keyOut.convertTo(keyOut, CV_8UC1);
        cv::subtract(cv::Scalar::all(255),keyOut,keyOut);
        
        /*
         * Prepare keyOut2
         */
        if (keyOut2.rows < origFrameMat.rows || keyOut2.cols < origFrameMat.cols) {
            keyOut2 = cv::Mat(origFrameMat.size(),CV_32FC1);
        }
        keyOut2 = whiteMat.mul(mask2);
        keyOut2.convertTo(keyOut2, CV_8UC1);
        cv::subtract(cv::Scalar::all(255),keyOut2,keyOut2);

        
        /*
         * Blending with frame image
         * This you probably dont need in the final app
         */
        
        /*
        keyOut3 = cv::Scalar::all(0); // clear
        origFrameMat.copyTo(keyOut3, maskOut); // copy using mask
        
        // --
        
        keyOut4 = cv::Scalar::all(0); // clear
        keyOut4.convertTo(keyOut4, CV_32FC3);
        
        origFrameMat.convertTo(origFrameMat, CV_32FC3, 1.0/255.0);
        
        cv::Mat bg = cv::Mat(origFrameMat.size(),CV_32FC3);
        bg = cv::Scalar(0,0,0);
        
        {
            vector<cv::Mat> ch_img(3);
            vector<cv::Mat> ch_bg(3);
            cv::split(origFrameMat,ch_img);
            cv::split(bg,ch_bg);
            ch_img[0]=ch_img[0].mul(mask2)+ch_bg[0].mul(1.0-mask2);
            ch_img[1]=ch_img[1].mul(mask2)+ch_bg[1].mul(1.0-mask2);
            ch_img[2]=ch_img[2].mul(mask2)+ch_bg[2].mul(1.0-mask2);
            cv::merge(ch_img,keyOut4);
        }
        keyOut4.convertTo(keyOut4, CV_8UC3);
        */
    }
}








//--------------------------------------------------------------
void CV::readAndWriteBlobData(ofColor backgroundColor,ofColor shadowColor)
{
    
    // Clear the Pixel object
    //pix.clear();
    
    // Compile the FBO
    recordFbo.begin();
    ofSetColor(backgroundColor);
    /*ofFill();
    ofRect(0, 0, _width, _height);
    for (int i = 0; i < contourFinder.nBlobs; i++)
    {
        ofSetColor(shadowColor);
        ofBeginShape();
        for (int k = 0 ; k < contourFinder.blobs[i].nPts; k++)
        {
            ofVertex(contourFinder.blobs[i].pts[k].x, contourFinder.blobs[i].pts[k].y);
        }
        ofEndShape(true);
    }*/
    ofSetColor(255, 255, 255);
    outputImage.draw(0, 0, _width,_height);
    glReadPixels(0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    recordFbo.end();
    
    pix.setFromPixels(pixels, _width, _height, 4);
    
    //recordFbo.readToPixels(pix);
    getAllPaths();
    if (!isSomeoneThere())
    {
        for (int i = 0; i < 10; i++)
        {
            blobPaths[i].bPath.clear();
        }
    }
    
}
//--------------------------------------------------------------
bool CV::newFrame()
{
#ifdef DEBUG
    return debugVideo.isFrameNew();
#else
    return vidGrabber.isFrameNew();
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
void CV::setTrackingBoundaries(int offsetX, int offsetY)
{
    _offsetX = offsetX;
    _offsetY = offsetY;
}
//--------------------------------------------------------------
bool CV::isSomeoneInTheLight()
{
    if (contourFinder.nBlobs > 0)
    {
        for (int i = 0; i < contourFinder.nBlobs; i++)
        {
            if (contourFinder.blobs[i].centroid.x >= _offsetX && contourFinder.blobs[i].centroid.x <= _width-(_offsetX) && contourFinder.blobs[i].centroid.y >= _offsetY && contourFinder.blobs[i].centroid.y <= _height-(_offsetY))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    else
    {
        return false;
    }
}
//--------------------------------------------------------------
int CV::getNumberOfBlobs()
{
    return contourFinder.nBlobs;
}
//--------------------------------------------------------------
void CV::relearnBackground()
{
    startLearn = true;
    if (startLearn == true)
    {
        grayBg = grayWarped;
        startLearn = false;
    }
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
    colorImg.draw(0, 0,_width,_height);
    ofDrawBitmapStringHighlight("Warper",0+5,15);
    ofSetColor(255);
    grayWarped.draw(_width,0,_width,_height);
    ofDrawBitmapStringHighlight("Warped Img",_width+5,15);
    
    if (canDoCalibration == true)
    {
        cvWarpQuad.draw(0, 0, _width, _height,0,255,0,2);
    }
    else
    {
        cvWarpQuad.draw(0, 0, _width, _height,255,0,0,2);
    }
    
    ofPopStyle();
}
//--------------------------------------------------------------
void CV::drawAllPaths()
{
    for (int i = 0; i < blobPath.size(); i++)
    {
        ofNoFill();
        ofBeginShape();
        ofSetColor(255, 255, 0);
        for (int p = 0; p < blobPath.size(); p++)
        {
            ofVertex(blobPath[p].x,blobPath[p].y);
        }
        ofEndShape(false);
    }
}
//--------------------------------------------------------------
void CV::getAllPaths()
{
    for (int i = 0; i < contourFinder.nBlobs; i++)
    {
        blobPaths[0].bPath.push_back(contourFinder.blobs[0].centroid);
    }
}
//--------------------------------------------------------------
void CV::drawTracking()
{
    ofPushMatrix();
    ofTranslate(_width, 0);
    ofSetColor(0, 0, 0);
    ofFill();
    ofRect(0, 0, _width, _height);
    ofSetColor(255, 255, 255);
    contourFinder.draw(0,0,_width,_height);
   
    ofNoFill();
    if (isSomeoneInTheLight()) {
        ofSetColor(0, 255, 0);
    }
    else
    {
        ofSetColor(255,25,0);
    }
    ofRect(_offsetX,_offsetY,_width-(_offsetX*2),_height-(_offsetY*2));
  
    ofSetColor(255, 255, 255);
    
    for (int i = 0; i < contourFinder.nBlobs; i++)
    {
        ofDrawBitmapStringHighlight(ofToString(i), contourFinder.blobs[i].boundingRect.x+5,contourFinder.blobs[i].boundingRect.y+10);
        ofSetColor(255, 0, 0);
        ofCircle(contourFinder.blobs[i].centroid, 3);
    }
    
    if (!blobPaths.empty())
    {
        ofNoFill();
        
        for (int o = 0; o < blobPaths.size(); o++)
        {
            ofBeginShape();
            for (int i = 0; i < blobPaths[o].bPath.size(); i++)
            {
                ofVertex(blobPaths[o].bPath[i].x,blobPaths[o].bPath[i].y);
            }
            ofEndShape(false);
        }
    }
    
    ofPopMatrix();
    
}
//--------------------------------------------------------------
void CV::drawLive()
{
    ofSetColor(255);
    ofDrawBitmapStringHighlight("Live",ofGetWidth()-_width+5,15);
	grayImage.draw(ofGetWidth()-_width,0,_width,_height);
    ofPushMatrix();
    ofTranslate(ofGetWidth()-_width, 0);
    ofBeginShape();
    ofNoFill();
    ofSetColor(255, 0, 0);
    for (int i = 0; i < 4; i++)
    {
        if (i == 0) {
            ofVertex(dstPts[0]);
            ofVertex(dstPts[3]);
        }
        ofVertex(dstPts[i]);
       
    }
    ofEndShape(false);
    ofPopMatrix();
}
//--------------------------------------------------------------
void CV::draw()
{
    ofFill();
    ofSetColor(255);
    
    float x = 0.f, y = 0.f;
    
    ofPushMatrix();
    ofTranslate(x, y);
    ofDrawBitmapStringHighlight("colorImg",5,15);
    colorImg.draw(0,20,_width/2,_height/2);
    ofPopMatrix();
    x += _width/2;
    
    
    /*
    ofPushMatrix();
    ofTranslate(x, y);
    ofDrawBitmapStringHighlight("grayImage",5,15);
    grayImage.draw(0,20,_width/2,_height/2);
    ofPopMatrix();
    x += _width/2;
    
    ofPushMatrix();
    ofTranslate(x, y);
    ofDrawBitmapStringHighlight("grayBg",5,15);
    grayBg.draw(0,20,_width/2,_height/2);
    ofPopMatrix();
    x += _width/2;
    
    ofPushMatrix();
    ofTranslate(x, y);
    ofDrawBitmapStringHighlight("frameDiff",5,15);
    frameDiff.draw(0,20,_width/2,_height/2);
    ofPopMatrix();
    x += _width/2;
    
    ofPushMatrix();
    ofTranslate(x, y);
    ofDrawBitmapStringHighlight("grayDiff",5,15);
    grayDiff.draw(0,20,_width/2,_height/2);
    ofPopMatrix();
    x += _width/2;
     */
    
    
    ofPushMatrix();
    ofTranslate(x, y);
    ofDrawBitmapStringHighlight("fgMask",5,15);
    drawMat(fgMaskMOG2, 0, 20,_width/2,_height/2);
    ofPopMatrix();
//    x += _width/2;
    
    
    ofPushMatrix();
    ofTranslate(x, y + _height/2 + 20);
    ofDrawBitmapStringHighlight("bgMat",5,15);
    drawMat(bgMat, 0, 20,_width/2,_height/2);
    ofPopMatrix();
    x += _width/2;
    
    ofPushMatrix();
    ofTranslate(x, y);
    ofDrawBitmapStringHighlight("maskOut",5,15);
    drawMat(maskOut, 0, 20,_width/2,_height/2);
    ofPopMatrix();
//    x += _width/2;
    
    ofPushMatrix();
    ofTranslate(x, y + _height/2 + 20);
    ofDrawBitmapStringHighlight("keyOut",5,15);
    drawMat(keyOut, 0, 20,_width/2,_height/2);
    ofPopMatrix();
    
    ofPushMatrix();
    ofTranslate(x, y + (_height/2 + 20)*2);
    ofDrawBitmapStringHighlight("keyOut2",5,15);
    drawMat(keyOut2, 0, 20,_width/2,_height/2);
    ofPopMatrix();
    
    ofPushMatrix();
    ofTranslate(x + (_width/2 + 20)*1, y + (_height/2 + 20)*1);
    ofDrawBitmapStringHighlight("keyOut3",5,15);
    drawMat(keyOut3, 0, 20,_width/2,_height/2);
    ofPopMatrix();
    
    ofPushMatrix();
    ofTranslate(x + (_width/2 + 20)*1, y + (_height/2 + 20)*2);
    ofDrawBitmapStringHighlight("keyOut4",5,15);
    drawMat(keyOut4, 0, 20,_width/2,_height/2);
    ofPopMatrix();

    
    return;
    
    
    
    
//    drawCalibration();
    ofPushMatrix();
    ofTranslate(0, _height);
    ofSetColor(255);
	
    ofFill();
    ofDrawBitmapStringHighlight("Color Img",0+5,15);
	grayImage.draw(_width/2,0,_width/2,_height/2);  // Gray Warped
    ofDrawBitmapStringHighlight("Gray Img",_width/2+5,15);
	grayBg.draw(0,120,_width/2,_height/2);
    ofDrawBitmapStringHighlight("BG Img",5,135);
	frameDiff.draw(_width/2,120,_width/2,_height/2);
    grayDiff.draw(_width/2,120,_width/2,_height/2);
    ofDrawBitmapStringHighlight("Diff Img",_width/2+5,135);
    recordFbo.draw(0,_height,_width,_height);
    ofDrawBitmapStringHighlight("Buffer Img",5,255);

    diffImage.draw(240,0,_width/2,_height/2);
    drawTracking();
    ofDrawBitmapStringHighlight("Contour Finder Img",_width+5,15);
    ofPopMatrix();
}
//--------------------------------------------------------------
ofPixels CV::getRecordPixels()
{
    
    return pix;
}
//--------------------------------------------------------------
ofVec2f CV::getBlobPath()
{
    if (contourFinder.nBlobs > 0)
    {
        for (int i = 0; i < contourFinder.nBlobs; i++)
        {
            //If the Blob has an Area larger than 100 pixels find it and return its centroid
            if (contourFinder.blobs[i].area >= 100)
            {
                blobPath.push_back(contourFinder.blobs[0].centroid);
                return contourFinder.blobs[0].centroid;
            }
        }
    }
    else
    {
        //return ofVec2f(320/2,240/2);
    }
}
//--------------------------------------------------------------
void CV::mouseDragged(int x, int y, int button){
    cvWarpQuad.updatePoint(x, y, 0,0,_width,_height);
}
//--------------------------------------------------------------
void CV::mousePressed(int x, int y, int button){
    cvWarpQuad.selectPoint(x, y,0,0,_width,_height,30);

}
//--------------------------------------------------------------
void CV::mouseReleased(int x, int y, int button){
    cvWarpQuad.saveToFile("quad-settings.xml");
}
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
