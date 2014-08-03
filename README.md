ofxDaveOpenCV
=============

Just a simple class that does all the openCV Things in smaller blocks
Specifically for use within Playable City Project Shadowing

## API
===
// Setup 

<code>
void setup(int width, int height, int framerate);
</code>

// Subtraction Loop

<code>
	void subtractionLoop(bool bLearnBackground, bool useProgressiveLearn,float progressionRate,bool mirrorH,bool mirrorV,int threshold,int blur,int minBlobSize, int maxBlobSize,int maxBlobNum,bool fillHoles, bool useApproximation, float brightness,float contrast,bool erode,bool dilate);
</code>

// Subtraction Loop

<code>
void subtractionLoop(bool bLearnBackground, bool useProgressiveLearn,float progressionRate,bool mirrorH,bool mirrorV,int threshold,int blur,int minBlobSize, int maxBlobSize,int maxBlobNum,bool fillHoles, bool useApproximation,bool erode,bool dilate);
</code>

// Subtraction and recombination Loop

<code>
void subtractionLoop(bool bLearnBackground,bool mirrorH,bool mirrorV,int threshold, int blur,int minBlobSize, int maxBlobSize,int maxBlobNum,bool fillHoles, bool useApproximation,float brightness,float contrast);

</code>

// Renders the Tracked blobs to a FBO, then makes a ofPixels object out of the FBO.

<code>
void readAndWriteBlobData(ofColor backgroundColor,ofColor shadowColor);
</code>

// Draws all the CV Images

<code>
void draw();
</code>

// Draws the Live CV Image

<code>
void drawLive();
</code>

// Unused

<code>
void drawAllPaths();
</code>

// Draws the Live FBO 

<code>
void drawLiveShadow();
</code>

// Gets the FBO's Pixels

<code>
ofPixels getRecordPixels();
</code>

// Gets the Number of tracked Blobs

<code>
int getNumberOfBlobs();
</code>

// Unused

<code>
string getBlobData();
</code>

// Check if someone is there

<code>
bool isSomeoneThere();
</code>

// Is there a new Frame

<code>
bool newFrame();
</code>

// Gets the Blobs Current Coordinates 

<code>
ofVec2f getBlobPath();
</code>