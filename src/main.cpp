#include "ofMain.h"
#include "triptychApp.h"
//#include "ofGLProgrammableRenderer.h"

int main()
{
	//ofSetCurrentRenderer(ofGLProgrammableRenderer::TYPE);
	ofSetupOpenGL(1024, 576, OF_WINDOW);
	ofRunApp( new triptychApp());
}
