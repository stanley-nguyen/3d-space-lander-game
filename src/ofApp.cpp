
//--------------------------------------------------------------
//
//  Kevin M. Smith
//
//  Octree Test - startup scene
// 
//
//  Student Name:   Stanley Nguyen
//  Date: 12/12/2024


#include "ofApp.h"
#include "Util.h"


//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup(){
	background.load("images/stars-space-night-640dae.png");
	thrustSound.load("sounds/thrusters-loop.wav");
	landSound.load("sounds/jumpland.wav");						// https://opengameart.org/content/jump-landing-sound
	explosionSound.load("sounds/DeathFlash.flac");				// https://opengameart.org/content/big-explosion
	bgmSound.load("sounds/ObservingTheStar.ogg");				// https://opengameart.org/content/another-space-background-track
	bgmSound.setLoop(true);
	bgmSound.setVolume(0.3);
	bgmSound.play();

	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;
	bTerrainSelected = true;
//	ofSetWindowShape(1024, 768);
	masterCam = &cam;
	masterCam->setDistance(10);
	masterCam->setNearClip(.1);
	masterCam->setFov(65.5);   // approx equivalent to 28mm in 35mm format
	ofSetVerticalSync(true);
	masterCam->disableMouseInput();
	ofEnableSmoothing();
	ofEnableDepthTest();

	// texture loading
	//
	ofDisableArbTex();     // disable rectangular textures

	// load textures
	//
	if (!ofLoadImage(particleTex, "images/dot.png")) {
		cout << "Particle Texture File: images/dot.png not found" << endl;
		ofExit();
	}

	// load the shader
	//
#ifdef TARGET_OPENGLES
	shader.load("shaders_gles/shader");
#else
	shader.load("shaders/shader");
#endif

	// setup landing area
	//
	landingArea = Box(Vector3(140, -1.5, 130), Vector3(150, 5, 140));

	ofEnableLighting();

	// setup lights
	// 
	landerLight.setup();
	landerLight.enable();

	keyLight.setup();
	keyLight.enable();
	keyLight.setAreaLight(1, 1);
	keyLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	keyLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	keyLight.setSpecularColor(ofFloatColor(1, 1, 1));

	keyLight.setPosition(landingArea.center().x(), landingArea.center().y(), landingArea.center().z());

	fillLight.setup();
	fillLight.enable();
	fillLight.setSpotlight();
	fillLight.setScale(.05);
	fillLight.setSpotlightCutOff(15);
	fillLight.setAttenuation(2, .001, .001);
	fillLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	fillLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	fillLight.setSpecularColor(ofFloatColor(1, 1, 1));
	//fillLight.rotate(-10, ofVec3f(1, 0, 0));
	//fillLight.rotate(-45, ofVec3f(0, 1, 0));
	fillLight.setPosition(75, 50, 175);

	rimLight.setup();
	rimLight.enable();
	rimLight.setSpotlight();
	rimLight.setScale(.05);
	rimLight.setSpotlightCutOff(30);
	rimLight.setAttenuation(.2, .001, .001);
	rimLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	rimLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	rimLight.setSpecularColor(ofFloatColor(1, 1, 1));
	//rimLight.rotate(180, ofVec3f(0, 1, 0));
	rimLight.setPosition(200, 50, 65);

	// setup camera views
	//
	trackCam.setPosition(150, 20, 100);
	trackCam.lookAt(lander.getPosition());

	// setup rudimentary lighting 
	//
	initLightingAndMaterials();

	//mars.loadModel("geo/mars-low-5x-v2.obj");
	mars.loadModel("geo/moon-houdini.obj");
	mars.setScaleNormalization(false);

	// create sliders for testing
	//
	gui.setup();
	gui.add(numLevels.setup("Number of Octree Levels", 5, 1, 10));
	gui.add(bTimingInfo.setup("Timing Info", false));
	gui.add(keyArea.setup("Key Area Light", 1, 0, 1));
	gui.add(keyAmbient.setup("Key Ambient Color", 0.1, 0, 1));
	gui.add(keyDiffuse.setup("Key Diffuse Color", 1.8, 0, 2));
	gui.add(keySpecular.setup("Key Specular Color", 1, 0, 2));

	gui.add(fillScale.setup("Fill Light Scale", 0.05, 0.0, 0.1));
	gui.add(fillSpot.setup("Fill Spotlight Cutoff", 10, 0, 30));
	gui.add(fillAmbient.setup("Fill Ambient Color", 0.1, 0, 1));
	gui.add(fillDiffuse.setup("Fill Diffuse Color", 0.75, 0, 2));
	gui.add(fillSpecular.setup("Fill Specular Color", 1, 0, 2));

	gui.add(rimScale.setup("Rim Light Scale", 0.05, 0.0, 0.1));
	gui.add(rimSpot.setup("Rim Spotlight Cutoff", 10, 0, 30));
	gui.add(rimAmbient.setup("Rim Ambient Color", 0.1, 0, 1));
	gui.add(rimDiffuse.setup("Rim Diffuse Color", 0.85, 0, 2));
	gui.add(rimSpecular.setup("Rim Specular Color", 1, 0, 1));
	bHide = true;

	// setup emitters and forces
	//
	explosionEmitter.sys->addForce(new ImpulseRadialForce(5000));
	explosionEmitter.sys->addForce(new GravityForce(gravity));
	explosionEmitter.sys->addForce(new TurbulenceForce(ofVec3f(-1000, -1000, -1000), ofVec3f(1000, 1000, 1000)));
	explosionEmitter.setVelocity(ofVec3f(0, 0, 0));
	explosionEmitter.setOneShot(true);
	explosionEmitter.setEmitterType(RadialEmitter);
	explosionEmitter.setGroupSize(2000);
	explosionEmitter.setParticleRadius(40);
	explosionEmitter.setLifespan(2.5);

	landEmitter.sys->addForce(new RingForce(500));
	landEmitter.sys->addForce(new GravityForce(gravity));
	landEmitter.sys->addForce(new TurbulenceForce(ofVec3f(-10, -10, -10), ofVec3f(10, 10, 10)));
	landEmitter.setVelocity(ofVec3f(0, 0, 0));
	landEmitter.setOneShot(true);
	landEmitter.setEmitterType(RadialEmitter);
	landEmitter.setGroupSize(1000);
	landEmitter.setParticleRadius(20);
	landEmitter.setLifespan(2.5);

	thrustEmitter.sys->addForce(new GravityForce(gravity));
	thrustEmitter.sys->addForce(new TurbulenceForce(ofVec3f(-50, -100, -50), ofVec3f(50, -50, 50)));
	thrustEmitter.setVelocity(ofVec3f(0, 0, 0));
	thrustEmitter.setOneShot(false);
	thrustEmitter.setEmitterType(DirectionalEmitter);
	thrustEmitter.setGroupSize(250);
	thrustEmitter.setParticleRadius(3);
	thrustEmitter.setLifespan(0.5);
	thrustEmitter.setRate(25);
	
	//  Create Octree for testing.
	//
	octree.create(mars.getMesh(0), 10);

	cout << "Number of Verts: " << mars.getMesh(0).getNumVertices() << endl;

	testBox = Box(Vector3(3, 3, 0), Vector3(5, 5, 2));

	spawnLander();
}

// load vertex buffer in preparation for rendering
//
void ofApp::loadVbo(ParticleEmitter &emitter, ofVbo &vbo) {
	if (emitter.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < emitter.sys->particles.size(); i++) {
		points.push_back(emitter.sys->particles[i].position);
		sizes.push_back(ofVec3f(emitter.particleRadius));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	vbo.clear();
	vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {

	setLights();

	// update emitters
	//
	explosionEmitter.update();
	landEmitter.setPosition(lander.getPosition());
	landEmitter.update();

	glm::vec3 thrustOffset(0, 1, 0);
	thrustEmitter.setPosition(lander.getPosition() + thrustOffset);
	thrustEmitter.update();

	// update fuel amount if thrust activates
	//
	if (thrustEmitter.started && landerFuel > 0) {
		landerFuel -= 1.0 / ofGetFrameRate();
	}

	// stop emitter if no more fuel
	//
	if (landerFuel <= 0) {
		thrustEmitter.stop();
		landerFuel = 0;
	}

	// update cameras
	//
	trackCam.setPosition(150, 20, 100);
	trackCam.lookAt(lander.getPosition());
	landerCam.setPosition(lander.getPosition() + glm::vec3(0, 0, 0));
	landerCam.setOrientation(ofQuaternion(-90, ofVec3f(1, 0, 0)) *								// set cam to face down
							 ofQuaternion(lander.getRotationAngle(0) - 90, ofVec3f(0, 1, 0)));	// set cam to rotate with lander

	// rotate lander according to its angle
	//
	lander.setRotation(0, landerAngle, 0, 1, 0);

	checkCollisions();
	integrate();

	checkWon();

	if (aglToggle)
		altitude = calculateAGL();
	else
		altitude = -1;
}
//--------------------------------------------------------------
void ofApp::draw() {

	// draw the background image
	//
	ofBackground(ofColor::black);
	glDepthMask(GL_FALSE);
	background.setAnchorPercent(0.5, 0.5);
	background.draw(ofGetWidth()/2, ofGetHeight()/2, ofGetWidth(), ofGetHeight());
	glDepthMask(GL_TRUE);

	loadVbo(explosionEmitter, vboExplosion);
	loadVbo(landEmitter, vboLand);
	loadVbo(thrustEmitter, vboThrust);

	masterCam->begin();
	ofPushMatrix();
	if (bWireframe) {                    // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		mars.drawWireframe();
		if (bLanderLoaded) {
			lander.drawWireframe();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}
	else {
		ofEnableLighting();              // shaded mode
		mars.drawFaces();
		ofMesh mesh;
		if (bLanderLoaded) {
			lander.drawFaces();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
			if (bDisplayBBoxes) {
				ofNoFill();
				ofSetColor(ofColor::white);
				for (int i = 0; i < lander.getNumMeshes(); i++) {
					ofPushMatrix();
					ofMultMatrix(lander.getModelMatrix());
					ofRotate(-90, 1, 0, 0);
					Octree::drawBox(bboxList[i]);
					ofPopMatrix();
				}
			}

			if (bLanderSelected) {

				ofVec3f min = lander.getSceneMin() + lander.getPosition();
				ofVec3f max = lander.getSceneMax() + lander.getPosition();

				Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
				ofSetColor(ofColor::white);
				Octree::drawBox(bounds);

				// draw colliding boxes
				//
				ofSetColor(ofColor::lightBlue);
				for (int i = 0; i < colBoxList.size(); i++) {
					Octree::drawBox(colBoxList[i]);
				}
			}
		}
	}
	if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));

	if (bDisplayPoints) {                // display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
		mars.drawVertices();
	}

	// highlight selected point (draw sphere around selected point)
	//
	if (bPointSelected) {
		ofSetColor(ofColor::blue);
		ofDrawSphere(selectedPoint, .1);
	}


	// recursively draw octree
	//
	ofDisableLighting();
	int level = 0;
	//	ofNoFill();

	if (bDisplayLeafNodes) {
		octree.drawLeafNodes(octree.root);
		cout << "num leaf: " << octree.numLeaf << endl;
    }
	else if (bDisplayOctree) {
		ofNoFill();
		ofSetColor(ofColor::white);
		octree.draw(numLevels, 0);
	}

	// if point selected, draw a sphere
	//
	if (pointSelected) {
		ofVec3f p = octree.mesh.getVertex(selectedNode.points[0]);
		ofVec3f d = p - masterCam->getPosition();
		ofSetColor(ofColor::lightGreen);
		ofDrawSphere(p, .02 * d.length());
	}

	// draw landing area
	//
	//Octree::drawBox(landingArea);

	ofPopMatrix();
	masterCam->end();

	// draw shaders
	//
	glDepthMask(GL_FALSE);

	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnablePointSprites();

	shader.begin();
	masterCam->begin();

	// draw emitters
	//
	particleTex.bind();

	// draw explosion emitter with random red/orange color
	//
	ofSetColor(255, ofRandom(255), 0);
	vboExplosion.draw(GL_POINTS, 0, explosionEmitter.sys->particles.size());
	// draw landing emitter with gray color
	//
	ofSetColor(90, 90, 90);
	vboLand.draw(GL_POINTS, 0, landEmitter.sys->particles.size());
	// draw thrust emitter with random yellow/orange color
	//
	ofSetColor(ofRandom(200, 255), ofRandom(175), 0);
	vboThrust.draw(GL_POINTS, 0, thrustEmitter.sys->particles.size());

	particleTex.unbind();

	masterCam->end();
	shader.end();

	if (!bHide) gui.draw();
	drawHud();

	ofDisablePointSprites();
	ofDisableBlendMode();
	ofEnableAlphaBlending();

	glDepthMask(GL_TRUE);
}

void ofApp::drawHud() {
	ofSetColor(ofColor::white);
	string fuelString = "REMAINING FUEL: " + ofToString(landerFuel) + " (SECONDS)";
	ofDrawBitmapString(fuelString, 10, 50);
	string aglVal;
	if (aglToggle)
		aglVal = ofToString(altitude);
	else
		aglVal = "DISABLED";
	string aglString = "Altitude (AGL): " + aglVal;
	ofDrawBitmapString(aglString, 10, 60);

	if (gameOver && !gameWon) {
		string gameOverString = "Game Over!";
		ofDrawBitmapString(gameOverString, ofGetWindowWidth() / 2 - 40, ofGetWindowHeight() / 2 - 30);
		string retryString = "Press R to retry";
		ofDrawBitmapString(retryString, ofGetWindowWidth() / 2 - 65, ofGetWindowHeight() / 2);
	}

	if (gameWon && !gameOver) {
		string gameWonString = "You win!";
		ofDrawBitmapString(gameWonString, ofGetWindowWidth() / 2 - 35, ofGetWindowHeight() / 2 - 30);
		string retryString = "Press R to retry";
		ofDrawBitmapString(retryString, ofGetWindowWidth() / 2 - 65, ofGetWindowHeight() / 2 + 20);
	}
}

// 
// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
	

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}

void ofApp::setLights() {
	keyLight.setAreaLight(keyArea, keyArea);
	keyLight.setAmbientColor(ofFloatColor(keyAmbient));
	keyLight.setDiffuseColor(ofFloatColor(keyDiffuse));
	keyLight.setSpecularColor(ofFloatColor(keySpecular));

	fillLight.setScale(fillScale);
	fillLight.setSpotlightCutOff(fillSpot);
	fillLight.setAmbientColor(ofFloatColor(fillAmbient));
	fillLight.setDiffuseColor(ofFloatColor(fillDiffuse));
	fillLight.setSpecularColor(ofFloatColor(fillSpecular));

	rimLight.setScale(rimScale);
	rimLight.setSpotlightCutOff(rimSpot);
	rimLight.setAmbientColor(ofFloatColor(rimAmbient, rimAmbient, rimAmbient, 1.0));
	rimLight.setDiffuseColor(ofFloatColor(rimDiffuse, rimDiffuse, rimDiffuse, 1.0));
	rimLight.setSpecularColor(ofFloatColor(rimSpecular, rimSpecular, rimSpecular, 1.0));

	landerLight.setAmbientColor(ofFloatColor(0.7, 0.7, 0.7));
	landerLight.setDiffuseColor(ofFloatColor(0.3, 0.3, 0.3));
	landerLight.setSpecularColor(ofFloatColor(0.6, 0.6, 0.6));
	glm::vec3 landingCenter(landingArea.center().x(), landingArea.center().y(), landingArea.center().z());
	rimLight.lookAt(landingCenter);
	fillLight.lookAt(landingCenter);
	glm::vec3 lightOffset(0, 5, 0);
	landerLight.setPosition(lander.getPosition() + lightOffset);
}

void ofApp::keyPressed(int key) {

	switch (key) {
	case 'B':
	case 'b':
		bDisplayBBoxes = !bDisplayBBoxes;
		break;
	case 'C':
	case 'c':
		if (masterCam->getMouseInputEnabled()) masterCam->disableMouseInput();
		else masterCam->enableMouseInput();
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		bHide = !bHide;
		break;
	case 'L':
	case 'l':
		bDisplayLeafNodes = !bDisplayLeafNodes;
		break;
	case 'O':
	case 'o':
		bDisplayOctree = !bDisplayOctree;
		break;
	case 'r':
		landerFuel = 120;
		lander.setPosition(0, 1.5, 0);
		landerForce = glm::vec3(0, 0, 0);
		landerVel = glm::vec3(0, 0, 0);
		gameOver = false;
		gameWon = false;
		break;
	case 's':
		savePicture();
		break;
	case 't':
		setCameraTarget();
		break;
	case 'u':
		break;
	case 'v':
		togglePointsDisplay();
		break;
	case 'V':
		break;
	case 'w':
		toggleWireframeMode();
		break;
	case OF_KEY_ALT:
		masterCam->enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_DEL:
		break;
	case OF_KEY_UP:
		thrustForce += 60;
		break;
	case OF_KEY_LEFT:
		angForce += 90;
		break;
	case OF_KEY_RIGHT:
		angForce -= 90;
		break;
	case OF_KEY_DOWN:
		thrustForce -= 60;
		break;
	case ' ':
		if (landerFuel <= 0) return;

		landerForce += glm::vec3(0, 40, 0);

		if (!thrustSound.isPlaying())
			thrustSound.play();
		if (!thrustEmitter.started) {
			thrustEmitter.sys->reset();
			thrustEmitter.start();
		}
		break;
	case 'A':
	case 'a':
		aglToggle = !aglToggle;
		break;
	case '1':
		masterCam = &cam;
		break;
	case '2':
		masterCam = &trackCam;
		break;
	case '3':
		masterCam = &landerCam;
		break;
	case 'k':
		if (landerLight.getIsEnabled())
			landerLight.disable();
		else
			landerLight.enable();
		break;
	default:
		break;
	}
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {

	switch (key) {
	
	case OF_KEY_ALT:
		masterCam->disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_UP:
	case OF_KEY_DOWN:
		thrustForce = 0;
		break;
	case ' ':
		thrustSound.stop();
		thrustEmitter.stop();
		thrustEmitter.sys->reset();
		break;
	default:
		break;

	}
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

	
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (masterCam->getMouseInputEnabled()) return;

	// if moving camera, don't allow mouse interaction
//
	if (masterCam->getMouseInputEnabled()) return;

	// if rover is loaded, test for selection
	//
	float startTime = ofGetElapsedTimeMillis();
	if (bLanderLoaded) {
		glm::vec3 origin = masterCam->getPosition();
		glm::vec3 mouseWorld = masterCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		bool hit = bounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
		if (hit) {
			bLanderSelected = true;
			mouseDownPos = getMousePointOnPlane(lander.getPosition(), masterCam->getZAxis());
			mouseLastPos = mouseDownPos;
			bInDrag = true;
		}
		else {
			ofVec3f p;
			raySelectWithOctree(p);
			if (bTimingInfo)
				cout << "Time to select point using ray selection: " << ofGetElapsedTimeMillis() - startTime << " ms" << endl;
			bLanderSelected = false;
		}
	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
		if (bTimingInfo)
			cout << "Time to select point using ray selection: " << ofGetElapsedTimeMillis() - startTime << " ms" << endl;
	}
}

bool ofApp::raySelectWithOctree(ofVec3f &pointRet) {
	ofVec3f mouse(mouseX, mouseY);
	ofVec3f rayPoint = masterCam->screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - masterCam->getPosition();
	rayDir.normalize();
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));

	pointSelected = octree.intersect(ray, octree.root, selectedNode);

	if (pointSelected) {
		pointRet = octree.mesh.getVertex(selectedNode.points[0]);
	}
	return pointSelected;
}




//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (masterCam->getMouseInputEnabled()) return;

	if (bInDrag) {

		glm::vec3 landerPos = lander.getPosition();

		glm::vec3 mousePos = getMousePointOnPlane(landerPos, masterCam->getZAxis());
		glm::vec3 delta = mousePos - mouseLastPos;
	
		landerPos += delta;
		lander.setPosition(landerPos.x, landerPos.y, landerPos.z);
		mouseLastPos = mousePos;

		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		colBoxList.clear();
		octree.intersect(bounds, octree.root, colBoxList);

		// check how many boxes are colliding
		//
		//collisionCount = colBoxList.size();
		////cout << collisionCount << endl;
		//if (collisionCount > 0) {
		//	landerCollided = true;
		//}
	

		/*if (bounds.overlap(testBox)) {
			cout << "overlap" << endl;
		}
		else {
			cout << "OK" << endl;
		}*/


	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bInDrag = false;
}



// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {

}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent2(ofDragInfo dragInfo) {

	ofVec3f point;
	mouseIntersectPlane(ofVec3f(0, 0, 0), masterCam->getZAxis(), point);
	if (lander.loadModel(dragInfo.files[0])) {
		lander.setScaleNormalization(false);
//		lander.setScale(.1, .1, .1);
	//	lander.setPosition(point.x, point.y, point.z);
		lander.setPosition(1, 1, 0);

		bLanderLoaded = true;
		for (int i = 0; i < lander.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
		}

		cout << "Mesh Count: " << lander.getMeshCount() << endl;
	}
	else cout << "Error: Can't load model" << dragInfo.files[0] << endl;
}

bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point) {
	ofVec2f mouse(mouseX, mouseY);
	ofVec3f rayPoint = masterCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
	ofVec3f rayDir = rayPoint - masterCam->getPosition();
	rayDir.normalize();
	return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {
	if (lander.loadModel(dragInfo.files[0])) {
		bLanderLoaded = true;
		lander.setScaleNormalization(false);
		lander.setPosition(0, 0, 0);
		cout << "number of meshes: " << lander.getNumMeshes() << endl;
		bboxList.clear();
		for (int i = 0; i < lander.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
		}

		//		lander.setRotation(1, 180, 1, 0, 0);

				// We want to drag and drop a 3D object in space so that the model appears 
				// under the mouse pointer where you drop it !
				//
				// Our strategy: intersect a plane parallel to the camera plane where the mouse drops the model
				// once we find the point of intersection, we can position the lander/lander
				// at that location.
				//

				// Setup our rays
				//
		glm::vec3 origin = masterCam->getPosition();
		glm::vec3 camAxis = masterCam->getZAxis();
		glm::vec3 mouseWorld = masterCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
		float distance;

		bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);
		if (hit) {
			// find the point of intersection on the plane using the distance 
			// We use the parameteric line or vector representation of a line to compute
			//
			// p' = p + s * dir;
			//
			glm::vec3 intersectPoint = origin + distance * mouseDir;

			// Now position the lander's origin at that intersection point
			//
			glm::vec3 min = lander.getSceneMin();
			glm::vec3 max = lander.getSceneMax();
			float offset = (max.y - min.y) / 2.0;
			lander.setPosition(intersectPoint.x, intersectPoint.y - offset, intersectPoint.z);

			// set up bounding box for lander while we are at it
			//
			landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		}
	}


}

//  intersect the mouse ray with the plane normal to the camera 
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
	// Setup our rays
	//
	glm::vec3 origin = masterCam->getPosition();
	glm::vec3 camAxis = masterCam->getZAxis();
	glm::vec3 mouseWorld = masterCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
	float distance;

	bool hit = glm::intersectRayPlane(origin, mouseDir, planePt, planeNorm, distance);

	if (hit) {
		// find the point of intersection on the plane using the distance 
		// We use the parameteric line or vector representation of a line to compute
		//
		// p' = p + s * dir;
		//
		glm::vec3 intersectPoint = origin + distance * mouseDir;

		return intersectPoint;
	}
	else return glm::vec3(0, 0, 0);
}

void ofApp::spawnLander() {
	//if (lander.loadModel(dragInfo.files[0])) {

	lander.loadModel("geo/lander.obj");
	bLanderLoaded = true;
	lander.setScaleNormalization(false);
	cout << "number of meshes: " << lander.getNumMeshes() << endl;
	bboxList.clear();
	for (int i = 0; i < lander.getMeshCount(); i++) {
		bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
	}

	// set starting location at origin
	//
	glm::vec3 min = lander.getSceneMin();
	glm::vec3 max = lander.getSceneMax();
	lander.setPosition(0, 1, 0);
	// set up bounding box for lander
	//
	landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
	landerForce = glm::vec3(0);
}

void ofApp::integrate() {
	// init current framerate (or you can use ofGetLastFrameTime())
	//
	float framerate = ofGetFrameRate();
	float dt = 1.0 / framerate;
	if (glm::isinf(dt)) dt = 0;

	// 
	// 3d motion physics
	//

	// random turbulence force on lane
	//
	float turbVal = 1;
	glm::vec3 turbulenceForce(ofRandom(-turbVal, turbVal), 0, ofRandom(-turbVal, turbVal));

	// calculate forward direction and apply thrust force to it
	//
	glm::vec3 forwardForce = glm::rotate(glm::vec3(1, 0, 0), glm::radians(lander.getRotationAngle(0)), glm::vec3(0, 1, 0)) * (thrustForce * dt);
	if (glm::any(glm::isnan(forwardForce))) forwardForce = glm::vec3(0, 0, 0);

	landerForce += gravity * mass + (forwardForce + turbulenceForce);
	landerAcc = landerForce / mass;
	landerVel += landerAcc * dt;
	if (glm::any(glm::isnan(landerVel))) landerVel = glm::vec3(0, 0, 0);

	//
	// angular motion physics
	//
	angAcc = angForce / mass;
	angVel += angAcc * dt;
	if (glm::isnan(angVel)) angVel = 0;

	// calculate and apply new position to lander
	//
	glm::vec3 currPos = lander.getPosition();
	glm::vec3 newPos = glm::vec3(currPos + landerVel*dt);
	lander.setPosition(newPos.x, newPos.y, newPos.z);

	// apply rotation to lander
	//
	landerAngle += angVel * dt;

	// apply damping
	//
	landerVel *= damping;
	angVel *= damping;

	// zero out forces
	//
	landerForce = glm::vec3(0, 0, 0);
	angForce = 0;
}

void ofApp::checkCollisions() {
	// only check when lander is descending
	//
	if (landerVel.y >= 0) return;

	ofVec3f min = lander.getSceneMin() + lander.getPosition();
	ofVec3f max = lander.getSceneMax() + lander.getPosition();

	Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

	colBoxList.clear();
	octree.intersect(bounds, octree.root, colBoxList);

	// check if lander is still in collision
	//
	collisionCount = colBoxList.size();
	if (collisionCount > 10) {
		// apply impulse function
		//
		ofVec3f norm = ofVec3f(0, 1, 0);  // just use vertical for normal for now
		ofVec3f lVel(landerVel.x, landerVel.y, landerVel.z);
		ofVec3f f = (restitution + 1.0) * (-lVel.dot(norm) * norm);
		landerForce += ofGetFrameRate() * f;

		// if impact force is high enough, trigger explosion
		//
		if (lVel.y < -2 && !gameWon) {
			explosionSound.play();
			glm::vec3 explosionOffset(0, 2.5, 0);
			explosionEmitter.setPosition(lander.getPosition() + explosionOffset);
			explosionEmitter.sys->reset();
			explosionEmitter.start();
			landerForce += glm::vec3(ofRandom(-100, 100), 100000, ofRandom(-100, 100));
			gameOver = true;
		}
		else if (lVel.y < -1) {
			landSound.play();
			landEmitter.sys->reset();
			landEmitter.start();
		}
	}
}

float ofApp::calculateAGL() {
	// construct ray shooting down starting from lander
	//
	Vector3 landerPos(lander.getPosition().x, lander.getPosition().y, lander.getPosition().z);
	Ray landerRay(Vector3(landerPos), Vector3(0, -1, 0));

	TreeNode selected;

	// store intersected node in selected
	//
	octree.intersect(landerRay, octree.root, selected);

	// fetch intersected point from mesh and calculate distance from lander
	//
	for (int i = 0; i < selected.points.size(); i++) {
		return glm::distance(lander.getPosition(), mars.getMesh(0).getVertex(selected.points[i]));
	}

	// if no intersection, return -1
	//
	return -1;
}

void ofApp::checkWon() {
	// calculate distance between landing area and lander
	//
	glm::vec3 landingCenter(landingArea.center().x(), landingArea.center().y(), landingArea.center().z());
	float dist = glm::distance(lander.getPosition(), landingCenter);

	// player wins when lander is stationary and on the landing area
	//
	if (landerVel.x > -1 && landerVel.y > -1 && landerVel.z > -1 &&
		landerVel.x < 1 && landerVel.y < 1 && landerVel.z < 1 && dist < 5 && !gameOver) {
		gameWon = true;
	}
}
