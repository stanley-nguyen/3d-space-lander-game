#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include  "ofxAssimpModelLoader.h"
#include "Octree.h"
#include <glm/gtx/intersect.hpp>
#include "ParticleEmitter.h"
#include "Particle.h"


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent2(ofDragInfo dragInfo);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void drawAxis(ofVec3f);
		void initLightingAndMaterials();
		void savePicture();
		void toggleWireframeMode();
		void togglePointsDisplay();
		void toggleSelectTerrain();
		void setCameraTarget();
		bool mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point);
		bool raySelectWithOctree(ofVec3f &pointRet);
		glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 p , glm::vec3 n);
		void spawnLander();

		ofImage background;

		ofEasyCam *masterCam;
		ofEasyCam cam;
		ofEasyCam trackCam;
		ofEasyCam landerCam;

		ofxAssimpModelLoader mars, lander;
		ofLight light;
		Box boundingBox, landerBounds;
		Box testBox;
		vector<Box> colBoxList;
		bool bLanderSelected = false;
		Octree octree;
		TreeNode selectedNode;
		glm::vec3 mouseDownPos, mouseLastPos;
		bool bInDrag = false;
		bool landerCollided = false;
		int collisionCount;

		// lights
		//
		ofLight keyLight, fillLight, rimLight, landerLight;
		ofxFloatSlider keyArea;
		ofxFloatSlider keyAmbient;
		ofxFloatSlider keyDiffuse;
		ofxFloatSlider keySpecular;

		ofxFloatSlider fillScale;
		ofxFloatSlider fillSpot;
		ofxFloatSlider fillAmbient;
		ofxFloatSlider fillDiffuse;
		ofxFloatSlider fillSpecular;

		ofxFloatSlider rimScale;
		ofxFloatSlider rimSpot;
		ofxFloatSlider rimAmbient;
		ofxFloatSlider rimDiffuse;
		ofxFloatSlider rimSpecular;

		void setLights();

		// lander physics
		//
		glm::vec3 landerVel = glm::vec3(0, 0, 0);
		glm::vec3 landerAcc = glm::vec3(0, 0, 0);
		glm::vec3 landerForce = glm::vec3(0, 0, 0);
		float thrustForce = 0;
		float angVel = 0;
		float angAcc = 0;
		float angForce = 0;
		float landerAngle = 0;
		glm::vec3 gravity = glm::vec3(0, -1.625, 0);
		float restitution = 0.85;
		float mass = 1.0;
		float damping = 0.99f;
		bool aglToggle = false;
		void integrate();
		void checkCollisions();
		float calculateAGL();
		float landerFuel = 120; // amount of fuel in seconds
		float altitude;

		// particle and shaders
		//
		ParticleEmitter explosionEmitter;
		ParticleEmitter landEmitter;
		ParticleEmitter thrustEmitter;
		ofTexture particleTex;
		ofVbo vboExplosion;
		ofVbo vboLand;
		ofVbo vboThrust;
		ofShader shader;
		void loadVbo(ParticleEmitter &e, ofVbo &vbo);

		// sounds
		//
		ofSoundPlayer thrustSound;
		ofSoundPlayer landSound;
		ofSoundPlayer explosionSound;
		ofSoundPlayer bgmSound;

		// landing area
		//
		Box landingArea;

		ofxIntSlider numLevels;
		ofxToggle bTimingInfo;
		ofxPanel gui;
		void drawHud();

		bool gameOver;
		bool gameWon;
		void checkWon();

		bool bAltKeyDown;
		bool bCtrlKeyDown;
		bool bWireframe;
		bool bDisplayPoints;
		bool bPointSelected;
		bool bHide;
		bool pointSelected = false;
		bool bDisplayLeafNodes = false;
		bool bDisplayOctree = false;
		bool bDisplayBBoxes = false;
		
		bool bLanderLoaded;
		bool bTerrainSelected;
	
		ofVec3f selectedPoint;
		ofVec3f intersectPoint;

		vector<Box> bboxList;

		const float selectionRange = 4.0;
};
