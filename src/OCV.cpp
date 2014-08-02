//
//  OCV.cpp
//  ShadowingStage1
//
//  Created by David Haylock on 28/07/2014.
//
//

#include "OCV.h"
//--------------------------------------------------------------
void CV::setup( int width,int height, int framerate)
{
    // Variables Over
    _width = width;
    _height = height;
    
    signedBlobPaths.resize(10);
    // Grabber initiallization
   
#ifdef DEBUG
    debugVideo.loadMovie("Debug/IRCapture.mp4");
    debugVideo.play();
#else
    vidGrabber.listDevices();
    vidGrabber.setDeviceID(0);
    vidGrabber.setDesiredFrameRate(framerate);
    vidGrabber.initGrabber(width,height);
#endif

    //Allocate the Memory for the CV processes
    colorImg.allocate(width,height);
	grayImage.allocate(width,height);
	grayBg.allocate(width,height);
    grayFloatBg.allocate(width,height);
	grayDiff.allocate(width,height);
    //lastFrame.allocate(width, height);
    //frameDiff.allocate(width, height);
    
    //outputTex.allocate(width, height,GL_RGBA);
    
    backgroundTimer = 0;
    threshold = 10;
    
    //outpix = new unsigned char[width*height * 4];
    
    /*for(int i = 0; i <(width*height * 4);  i++ )
    {
        outpix[i] = 0;
    }
    */
    present = true;
    pixels = new unsigned char[320*240*4];
    
    recordFbo.allocate(width, height,GL_RGBA);
    recordFbo.begin();
    ofClear(0);
    recordFbo.end();
}
//--------------------------------------------------------------
void CV::releaseCamera()
{
    //vidGrabber.close();
}
//--------------------------------------------------------------
void CV::subtractionLoop(bool bLearnBackground, bool useProgressiveLearn, float progressionRate, bool mirrorH, bool mirrorV,int threshold, int blur,int minBlobSize, int maxBlobSize,int maxBlobNum, bool fillHoles, bool useApproximation)
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
        
        grayDiff.absDiff(grayBg, grayImage);
        grayDiff.threshold(threshold);
        contourFinder.findContours(grayDiff, minBlobSize, maxBlobSize, maxBlobNum,fillHoles,useApproximation);
    }
}
void CV::subtractionLoop(bool bLearnBackground, bool useProgressiveLearn, float progressionRate, bool mirrorH, bool mirrorV,int threshold,int blur, int minBlobSize, int maxBlobSize,int maxBlobNum, bool fillHoles, bool useApproximation,float brightness, float contrast)
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
        grayImage.blur(blur);
        grayImage.brightnessContrast(brightness, contrast);
        grayDiff.absDiff(grayBg, grayImage);
        grayDiff.threshold(threshold);
        contourFinder.findContours(grayDiff, minBlobSize, maxBlobSize, maxBlobNum,fillHoles,useApproximation);
    }
}
//--------------------------------------------------------------
void CV::subtractionLoop(bool bLearnBackground,bool mirrorH,bool mirrorV,int threshold, int blur,int minBlobSize, int maxBlobSize,int maxBlobNum,bool fillHoles, bool useApproximation,float brightness,float contrast)
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
        
        if (bLearnBackground == true)
        {
            grayBg = grayImage;
            bLearnBackground = false;
        }
        
        grayImage = colorImg;
        
        
        frameDiff = colorImg;
        frameDiff.absDiff(lastFrame);
        
        grayImage.absDiff(grayBg);
        grayDiff = grayImage;
        
        threshImage = grayDiff;
        
        threshImage.threshold(threshold);
        
        threshImage.blur(blur);
        threshImage.brightnessContrast(0.8, 0.5);
        
        //diffImage += threshImage;
        grayDiff += frameDiff;
        
        frameDiff.threshold(threshold);
        contourFinder.findContours(frameDiff, minBlobSize, maxBlobSize, maxBlobNum,fillHoles,useApproximation);
        
        unsigned char * diffpix = grayDiff.getPixels();
        unsigned char * threshpix = threshImage.getPixels();
        
        
        for (int i = 0; i < (_width*_height); i ++) {
            
            int r = i * 4 + 0;
            int g = i * 4 + 1;
            int b = i * 4 + 2;
            int a = i * 4 + 3;
            
            if( threshpix[i] > 1){
                outpix[r] = ofClamp(diffpix[i]*10, 0, 255);
                outpix[g] = ofClamp(diffpix[i]*10, 0, 255);
                outpix[b] = ofClamp(diffpix[i]*10, 0, 255);
                outpix[a] = threshpix[i];
                
            }else{
                
                outpix[r] = 0;
                outpix[g] = 0;
                outpix[b] = 0;
                outpix[a] = 0;
            }
            
        }
        
        lastFrame = colorImg;
    }
    
    
    outputTex.loadData(outpix,_width,_height, GL_RGBA);
    
    pix.setFromPixels(outpix, _width, _height,4);
    
    if(contourFinder.nBlobs == 0 && ofGetElapsedTimeMillis()-  backgroundTimer >  4000 ){
        
        //could just add to runnign avg here;
        
        cout << "new back";
        grayBg = colorImg;
        present = false;
        
    }
    
    if(contourFinder.nBlobs > 0){
        backgroundTimer = ofGetElapsedTimeMillis();
        present = true;
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
    ofFill();
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
    }
    glReadPixels(0, 0, 320, 240, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    recordFbo.end();
    
    pix.setFromPixels(pixels, 320, 240, 4);
    
    //recordFbo.readToPixels(pix);
    
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
    if (contourFinder.nBlobs > 0)
    {
        return true;
    }
    else
    {
        return false;
        for (int i = 0; i < signedBlobPaths.size(); i++)
        {
            signedBlobPaths[i].clear();
        }
        
    } 
}
//--------------------------------------------------------------
void CV::drawLiveShadow()
{
    ofSetColor(255, 255, 255);
    recordFbo.draw(0,0,ofGetWidth(),ofGetHeight());
}
//--------------------------------------------------------------
void CV::drawAllPaths()
{
    if (contourFinder.nBlobs > 0)
    {
        for (int i = 0; i < contourFinder.nBlobs; i++)
        {
            signedBlobPaths[i].push_back(ofVec2f(contourFinder.blobs[i].centroid));
        }
    }
    
    
    if (!signedBlobPaths.empty())
    {
        for (int i = 0; i < signedBlobPaths.size(); i++)
        {
            ofNoFill();
            ofBeginShape();
            ofSetColor(255, 255, 0);
            for (int p = 0; p < signedBlobPaths[i].size(); p++)
            {
                ofVertex(signedBlobPaths[i][p].x,signedBlobPaths[i][p].y);
            }
            ofEndShape(false);
        }
    }
    
    
}
//--------------------------------------------------------------
void CV::drawLive()
{
    ofSetColor(255);
    ofDrawBitmapStringHighlight("Live",ofGetWidth()-_width+5,15);
	grayImage.draw(ofGetWidth()-_width,0,_width,_height);
}
//--------------------------------------------------------------
void CV::draw()
{
    ofSetColor(255);
    //Draw the CV data
	colorImg.draw(ofGetWidth()-_width,0,_width/2,_height/2);
    ofDrawBitmapStringHighlight("Color Img",ofGetWidth()-_width+5,15);
	grayImage.draw(ofGetWidth()-_width/2,0,_width/2,_height/2);
    ofDrawBitmapStringHighlight("Gray Img",ofGetWidth()-_width/2+5,15);
	grayBg.draw(ofGetWidth()-_width,120,_width/2,_height/2);
    ofDrawBitmapStringHighlight("BG Img",ofGetWidth()-_width+5,135);
	grayDiff.draw(ofGetWidth()-_width/2,120,_width/2,_height/2);
    ofDrawBitmapStringHighlight("Diff Img",ofGetWidth()-_width/2+5,135);
    recordFbo.draw(ofGetWidth()-_width,240,_width/2,_height/2);
    ofDrawBitmapStringHighlight("Buffer Img",ofGetWidth()-_width+5,245);
    //outputTex.draw(ofGetWidth()-_width/2,240,_width/2,_height/2);
    //ofDrawBitmapStringHighlight("Output Texture",ofGetWidth()-_width/2+5,245);
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
                return contourFinder.blobs[i].centroid;
            }
        }
    }
    else
    {
        return ofVec2f(320/2,240/2);
    }
    
}