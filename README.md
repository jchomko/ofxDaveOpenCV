ofxDaveOpenCV
=============

Just a simple class that does all the openCV Things in smaller blocks
Specifically for use within Playable City Project Shadowing

API
===
// Setup 
void setup(int width, int height, int framerate);

// Subtraction Loop
void subtractionLoop(bool bLearnBackground, bool useProgressiveLearn,float progressionRate,bool mirrorH,bool mirrorV,int threshold,int blur,int minBlobSize, int maxBlobSize,int maxBlobNum,bool fillHoles, bool useApproximation, float brightness,float contrast,bool erode,bool dilate);

// Subtraction Loop
void subtractionLoop(bool bLearnBackground, bool useProgressiveLearn,float progressionRate,bool mirrorH,bool mirrorV,int threshold,int blur,int minBlobSize, int maxBlobSize,int maxBlobNum,bool fillHoles, bool useApproximation,bool erode,bool dilate);

// Subtraction and recombination Loop
void subtractionLoop(bool bLearnBackground,bool mirrorH,bool mirrorV,int threshold, int blur,int minBlobSize, int maxBlobSize,int maxBlobNum,bool fillHoles, bool useApproximation,float brightness,float contrast);

// Renders the Tracked blobs to a FBO, then makes a ofPixels object out of the FBO.
void readAndWriteBlobData(ofColor backgroundColor,ofColor shadowColor);

// Draws all the CV Images
void draw();

// Draws the Live CV Image
void drawLive();

// Unused
void drawAllPaths();

// Draws the Live FBO 
void drawLiveShadow();

// Gets the FBO's Pixels
ofPixels getRecordPixels();

// Gets the Number of tracked Blobs
int getNumberOfBlobs();

// Unused
string getBlobData();

// Check if someone is there
bool isSomeoneThere();

// Is there a new Frame
bool newFrame();

// Gets the Blobs Current Coordinates 
ofVec2f getBlobPath();