//--------------------------------------------------------------
// Name: OCV.cpp
// Project: Playable City 2014 Award
// Author: David Haylock
// Creation Date: 28-07-2014
//--------------------------------------------------------------

#include "OCV.h"
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

    debugVideo.loadMovie("Debug/IRCapture.mp4");
    debugVideo.play();
#else
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

       // RunSingleCamera( guid );
    }

    error = cam.Connect(&guid);
    if (error != PGRERROR_OK)
    {
        PrintError( error );

    }

    CameraInfo camInfo;
    error = cam.GetCameraInfo(&camInfo);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
    }

    Property camProp;
    PropertyInfo camPropInfo;

    camProp.type = FRAME_RATE;
    error = cam.GetProperty( &camProp );
    if (error != PGRERROR_OK){
        PrintError( error );
    }
    cout <<  "Frame rate is : " <<  camProp.absValue << endl;

/*
    camProp.type = GAIN;
    error = cam.GetProperty( &camProp );
    if (error != PGRERROR_OK){
        PrintError( error );
   //     return -1;
    }

    camProp.autoManualMode = false;
    camProp.valueA = 16;  //16-64

    error = cam.SetProperty( &camProp );
    if (error != PGRERROR_OK){
        PrintError( error );
        //return -1;
    }
*/


    //camProp.absControl = true;
    //camProp.onePush = false;
    //camProp.onOff = true;
    //camProp.autoManualMode = false;
    //camProp.absValue = 25;

//    error = cam.SetProperty( &camProp, false);

    error = cam.StartCapture();
    if (error != PGRERROR_OK)
    {
        PrintError( error );



    }

    error = cam.StopCapture();

    error = cam.StartCapture();


#endif

    //Allocate the Memory for the CV processes
   // colorImg.allocate(width*2,height*2);

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
//    kinectGray.allocate(width*2,height*2);

    invDiffImage.allocate(width,height);
    cleanFrameDiff.allocate(width,height);

    outputImage.allocate(width, height,OF_IMAGE_GRAYSCALE);

    outpix = new unsigned char[width*height*4];

    for(int i = 0; i <width*height*4;  i++ ){
        outpix[i] = 0;
    }
    backgroundTimer = 0;
    present = true;
    presenceTimer = 0;
    absenceTimer = 0;

    present = false;
    pixels = new unsigned char[_width*_height*4];

    recordFbo.allocate(width, height,GL_RGBA);
    recordFbo.begin();
    ofClear(0);
    recordFbo.end();

    learnBackground = true;
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


}
//--------------------------------------------------------------
void CV::setTrackingBoundaries(int x, int y, int w, int h)
{



}
//--------------------------------------------------------------
void CV::releaseCamera()
{
    error = cam.StopCapture();
    if (error != PGRERROR_OK)
    {
        PrintError( error );

    }

    // Disconnect the camera
    error = cam.Disconnect();
    if (error != PGRERROR_OK)
    {
        PrintError( error );

    }

    //kinect.close();
}
//--------------------------------------------------------------
void CV::subtractionLoop(bool bLearnBackground, bool useProgressiveLearn, float progressionRate, bool mirrorH, bool mirrorV,int threshold, int blur,int minBlobSize, int maxBlobSize,int maxBlobNum, bool fillHoles, bool useApproximation,bool erode,bool dilate)
{

    bool bNewFrame = false;


#ifdef DEBUG
    debugVideo.update();
    bNewFrame = debugVideo.isFrameNew();
#else
    error = cam.RetrieveBuffer( &rawImage );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
	    bNewFrame = false;
        }else{
	    bNewFrame = true;
	}

    //vidGrabber.update();
    //bNewFrame = true; //vidGrabber.isFrameNew();
#endif

    if (bNewFrame)
    {
#ifdef DEBUG
        colorImg.setFromPixels(debugVideo.getPixels(),_width,_height);
#else
        //grayImage.setFromPixels(rawImage.GetData(), 808, 608);
        colorImg.setFromPixels(vidGrabber.getPixels(), _width,_height);
#endif

        colorImg.mirror(mirrorV, mirrorH);
//        grayImage = colorImg;

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
    //learnBackground = bLearnBackground;
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

void CV::progSubLoop(int minBlobSize, int maxBlobSize, int threshold, float blur, float brightness, float contrast)
{
	bool bNewFrame = false;
	vidGrabber.update();
	bNewFrame = vidGrabber.isFrameNew();

   	 if (bNewFrame)
   	 {
		    colorImg.resize(_width*2, _height*2);
		    colorImg.setFromPixels(vidGrabber.getPixels(), _width*2,_height*2);
		    colorImg.resize(_width, _height);
			grayImage = colorImg;

		    frameDiff = grayImage;
        	diffImage = grayImage;

        	//FrameDiff
        	frameDiff.absDiff(lastFrame);
        	frameDiff.threshold(threshold);

        	frameDiff.resize(_width/2, _width/2);
        	//Frame diff Contour Finder
        	contourFinder.findContours(frameDiff, minBlobSize, maxBlobSize, 4,false,true);

        	//Background sub for static background
        	grayBg.blur(blur);

        	diffImage.absDiff(grayBg);

		//diffImage.dilate_3x3();
		    //diffImage.contrastStretch();
		    //diffImage.threshold(threshold);

        	frameDiff.resize(_width, _height);

        	//diffImage += frameDiff;

        	diffImage.blur(blur);

                diffImage.brightnessContrast(brightness,contrast);

		diffImage.invert();

	       	lastFrame = colorImg;

		outputImage.setFromPixels(diffImage.getPixels(), diffImage.getWidth(), diffImage.getHeight(), OF_IMAGE_GRAYSCALE);

		//If no-one has been in the light for awhile, start saving the background
		if((contourFinder.nBlobs == 0 && (ofGetElapsedTimeMillis() - backgroundTimer >  10000)) |  ofGetFrameNum() < 30 ) // | bLearnBackground )
    		{
			lastFrame = colorImg;
		        pastImages.push_back(lastFrame);

        		//Maybe do an average of all 50 images? and then lighten / darken ?
        		if(pastImages.size() > 50)
			{
                		pastImages.erase(pastImages.begin());
         		}

        		if(pastImages.size() > 0)
        		{
				grayFloatBg.addWeighted(pastImages[0], 0.1);
            			grayBg = grayFloatBg;
            			//grayBg = pastImages[0];
            			//grayBg.brightnessContrast(-0.5,0);
            			grayBg.blur(blur);
        		}
        		//bLearnBackground = false;
        		//present = false;
    		}

		//If person stops moving in light
    		if(contourFinder.nBlobs == 0 && ofGetElapsedTimeMillis() - backgroundTimer > 1300)
		{
        		present = false;
    		}

		//If person enters the light
    		if(contourFinder.nBlobs > 0)
    		{
        		backgroundTimer = ofGetElapsedTimeMillis();
        		//While Present
        		present = true;
        		absenceTimer = ofGetElapsedTimeMillis() + 5000;
    		}

	}//End of New Frame

}
//--------------------------------------------------------------
void CV::JsubtractionLoop(bool bLearnBackground,bool mirrorH,bool mirrorV,int imgThreshold, int moveThreshold, int blur, int gaussBlur, int medianBlur, int minBlobSize, int maxBlobSize,int maxBlobNum,bool fillHoles, bool useApproximation,float brightness,float contrast,bool erode,bool dilate)
{
    bool bNewFrame = false;

#ifdef DEBUG
    debugVideo.update();
    bNewFrame = debugVideo.isFrameNew();
#else
   error = cam.RetrieveBuffer( &rawImage );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
        }

    //vidGrabber.update();
    bNewFrame = true; //vidGrabber.isFrameNew();
#endif

    if (bNewFrame)
    {

	#ifdef DEBUG
        	colorImg.setFromPixels(debugVideo.getPixels(),_width,_height);
	#else

        grayImage.resize(808,608);
        grayImage.setFromPixels(rawImage.GetData(), 808, 608);
        grayImage.resize(_width, _height);
        virginGray = grayImage;

	#endif

	//Warping
        // We get back the warped coordinates - scaled to our camera size
	ofPoint * warpedPts = cvWarpQuad.getScaledQuadPoints(_width, _height);
	// Lets warp with those cool coordinates!!!!!
        grayWarped.warpIntoMe(grayImage, warpedPts, dstPts);
	// Lets calculate the openCV matrix for our coordWarping
        coordWarp.calculateMatrix(warpedPts, dstPts);
	//colorImg.brightnessContrast(brightness, contrast);
        //colorImg.blurGaussian(gaussBlur);
        //grayImage = colorImg;

//    gray_mat = grayImage.getCvImage();
//    cv::Rect crop_roi = cv::Rect(_offsetX,_offsetY, _width - _offsetX, _height -_offsetY);
//    crop = gray_mat(crop_roi).clone();
//    grayImage = crop;

        grayImage.blurMedian(medianBlur);

    frameDiff = grayImage;
    diffImage = grayImage;
    invDiffImage = grayImage;

    frameDiff.absDiff(lastFrame);

    cleanFrameDiff = frameDiff;

    frameDiff.threshold(moveThreshold);

    contourFinder.findContours(frameDiff, minBlobSize, maxBlobSize, maxBlobNum, fillHoles, useApproximation);

    diffImage.absDiff(grayBg);

    diffImage += cleanFrameDiff;

   diffImage.brightnessContrast(brightness, contrast);
   diffImage.blur(blur);

    if (erode){
       diffImage.erode();
    }
    if (dilate){
       diffImage.dilate();
    }

    unsigned char * origPix = grayImage.getPixels();
    unsigned char * threshpix = diffImage.getPixels();

        for (int i = 0; i < _width*_height; i ++){
            if( threshpix[i] > imgThreshold) //threshold
      		{
                 outpix[i] = ofClamp(origPix[i]/5, 0, 255);
            }
             else
             {
                outpix[i] = 255;
             }
        }

        lastFrame = virginGray;

	       lastFrame.blurMedian(medianBlur);

        outputImage.setFromPixels(outpix, _width, _height, OF_IMAGE_GRAYSCALE);
    }

    //On Exit
    //this just checks if there's movement, and the presenceFinder contourFinder
    // decides if there's a person there
    //this needs to be more robust - so that it's not timer based, but knows when people are offscreen
    //upping the delay to 1 minute -
    // as recording is motion based, we want to be pretty sure nothing's been in the frame, and that nothing
    //sticks to the frame while people are playing

   // if(contourFinder.nBlobs == 0 && ofGetElapsedTimeMillis() - backgroundTimer >  10000)  // | bLearnBackground )
   if((contourFinder.nBlobs == 0 && (ofGetElapsedTimeMillis() - backgroundTimer >  5000)) |  ofGetFrameNum() < 100 )
   {
	lastFrame = virginGray;
	//lastFrame = kinectGray;
	pastImages.push_back(lastFrame);

	//Maybe do an average of all 50 images? and then lighten / darken ?
	if(pastImages.size() > 50){
        	pastImages.erase(pastImages.begin());
   	 }

	if(pastImages.size() > 0 )
        {
                grayBg = pastImages[0];
	        //grayBg.brightnessContrast(-0.5,0);
	        grayBg.blur(blur);
        }
	learnBackground = false;
        //present = false;
    }

    if(contourFinder.nBlobs == 0 && ofGetElapsedTimeMillis() - backgroundTimer > 1200){
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
void CV::PsubtractionLoop(bool bLearnBackground,bool mirrorH,bool mirrorV,int imgThreshold, int moveThreshold, int blur, int gaussBlur, int medianBlur, int minBlobSize, int maxBlobSize,int maxBlobNum,bool fillHoles, bool useApproximation,float brightness,float contrast,bool erode,bool dilate)
{
  bool bNewFrame = false;

  error = cam.RetrieveBuffer( &rawImage );
  if (error != PGRERROR_OK){
          PrintError( error );
          bNewFrame = false;
      }else{
        bNewFrame = true;
  }

  if (bNewFrame){
      grayImage.resize(808,608);
      grayImage.setFromPixels(rawImage.GetData(), 808, 608);
      grayImage.resize(_width, _height);
      virginGray = grayImage;

      grayImage.blur(blur);

       grayImage.absDiff(grayBg);
       grayImage.dilate();
       grayImage.brightnessContrast(brightness, contrast);
       grayImage.blur(blur);
       grayImage.threshold(threshold);
       imagingContourFinder.findContours(grayImage, 100, 999999, 4, false);

       frameDiff.absDiff(lastFrame);
       frameDiff.threshold(moveThreshold);

       contourFinder.findContours(frameDiff, minBlobSize, maxBlobSize, maxBlobNum, fillHoles, useApproximation);

       //Draw Filled Contours
       if(contourFinder.nBlobs > 0){

          imgBlobPaths.clear();

          for(int b = 0; b < imagingContourFinder.nBlobs; b ++){

               for(int p = 0; p < imagingContourFinder.blobs[b].pts.size(); p ++){
                   imgBlobPath.curveTo(imagingContourFinder.blobs[b].pts[p]);
               }

               imgBlobPath.setFilled(true);
               imgBlobPath.setFillColor(ofColor(0));
               imgBlobPaths.push_back(imgBlobPath);
               imgBlobPath.clear();

               }
            }else{

           imgBlobPaths.clear();
       }
       pathFbo.begin();
           ofClear(255);
           ofBackground(255);

           for(int i = 0; i < imgBlobPaths.size(); i ++){
               imgBlobPaths[i].draw(0, 0);
           }
       pathFbo.end();
       ofPixels output;
       pathFbo.readToPixels(output);
       outputImage.setFromPixels(output.getPixels(), _width, _height, OF_IMAGE_GRAYSCALE);
  }//End bNewFrame

  //On Exit
  //this just checks if there's movement, and the presenceFinder contourFinder
  // decides if there's a person there
  //this needs to be more robust - so that it's not timer based, but knows when people are offscreen
  //upping the delay to 1 minute -
  // as recording is motion based, we want to be pretty sure nothing's been in the frame, and that nothing
  //sticks to the frame while people are playing

 // if(contourFinder.nBlobs == 0 && ofGetElapsedTimeMillis() - backgroundTimer >  10000)  // | bLearnBackground )
 if((contourFinder.nBlobs == 0 && (ofGetElapsedTimeMillis() - backgroundTimer >  5000)) |  ofGetFrameNum() < 100 )
 {
    lastFrame = virginGray;
    //lastFrame = kinectGray;
    pastImages.push_back(lastFrame);

    //Maybe do an average of all 50 images? and then lighten / darken ?
    if(pastImages.size() > 50){
        pastImages.erase(pastImages.begin());
    }

    if(pastImages.size() > 0 ){
        grayBg = pastImages[0];
            //grayBg.brightnessContrast(-0.5,0);
        grayBg.blur(blur);
    }

    learnBackground = false;
          //present = false;
  }

  if(contourFinder.nBlobs == 0 && ofGetElapsedTimeMillis() - backgroundTimer > 1200){
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
	return true;
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
//	showGui = !showGui;
}
//-------------------------------------------------------------
void CV::drawGui()
{
//	 if(showGui){
//		gui.draw();
//	}
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
    ofTranslate(ofGetWidth()-_width*2, ofGetHeight()-_height);
    virginGray.draw(0, 0,_width,_height);
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
    ofPopMatrix();

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
    ofTranslate(ofGetWidth()-_width*2, 0);
    ofSetColor(0, 0, 0);
    ofFill();
    //ofRect(0, 0, _width, _height);
    //ofSetColor(255, 255, 255);
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

    drawCalibration();
    //ofPushMatrix();
    //ofTranslate(0, _height);
    ofSetColor(255);

    ofFill();

    virginGray.draw(ofGetWidth() - 2*_width,0,_width,_height);
    ofDrawBitmapStringHighlight("Virgin Gray",ofGetWidth() - 2*_width, 15);

    recordFbo.draw(ofGetWidth()- 2*_width,_height,_width,_height);
    ofDrawBitmapStringHighlight("Record Fbo",ofGetWidth() - 2*_width, _height+15);

    grayBg.draw(ofGetWidth()-2*_width, _height*2,_width/2,_height/2);
    ofDrawBitmapStringHighlight("Gray Bg", ofGetWidth() - 2*_width, _height*2+15);

    drawTracking();
    //ofPopMatrix();

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

void CV::PrintError (Error error){
    error.PrintErrorTrace();
}

void CV::exit()
{
	//gui.saveToFile("camera_settings.xml");
}
