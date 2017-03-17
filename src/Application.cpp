#include <stdafx.h>
#include <application.h>
#include <utility.h>

// Template specialization to initialize the application's singleton pointer
template<>
ssurge::Application * ssurge::Singleton<ssurge::Application>::msSingleton = nullptr;

//No longer needed Ogre::Camera* camPointer = NULL;


ssurge::Application::Application() : mCamera(NULL), mRoot(NULL), mOISInputManager(NULL), 
mOISKeyboard(NULL),mOISMouse(NULL),mWindow(NULL), mLogManager(NULL), mSceneManager(NULL),
mNextBulletID(0), mSpawnTimer(0.0f)
{
	// Initialize the random number generator
	srand((unsigned int)time(NULL));

	// Set the application state
	mState = ApplicationState::created;
}



ssurge::Application::~Application()
{
	// This was probably already called by the run method, but the method
	// is safe to re-call.
	shutdown();
}


	
int ssurge::Application::initialize(std::string window_title)
{
	// Create the Ogre root
#ifdef _DEBUG
	mRoot = new Ogre::Root("plugins_d.cfg");
#else
	mRoot = new Ogre::Root("plugins.cfg");
#endif

	// Create the LogManager.  Note: THIS is the LogManager singleton.
	// When we use LOG_MANAGER->, we're really de-referencing this object.
	mLogManager = new LogManager("ssurge_log.txt");
	if (mLogManager == NULL)
		return 1;
#ifdef _DEBUG
	mLogManager->setLogMask();
#else
	mLogManager->setLogMask(LL_ERROR);
#endif

	// Show the configuration dialog box
	if (!mRoot->showConfigDialog())
		return 2;			// The user cancelled in the dialog box.
	LOG_MANAGER->log("Successfully created Ogre Root", LL_NORMAL);

	// Create the Ogre Window
	mWindow = mRoot->initialise(true, window_title);
	if (!mWindow)
		return 3;
	LOG_MANAGER->log("Successfully created render window", LL_NORMAL);

	// Create the OIS Input Manager (very platform specific)
#ifdef _WIN32
	// Reference: www.ogre3d.org/tikiwiki/tiki-index.php?page=Using+OIS
	OIS::ParamList pl;
	size_t windowHnd = 0;
	std::ostringstream temp_str;
	mWindow->getCustomAttribute("WINDOW", &windowHnd);
	temp_str << windowHnd;
	pl.insert(std::make_pair(std::string("WINDOW"), temp_str.str()));
	mOISInputManager = OIS::InputManager::createInputSystem(pl);
#else
	// We should raise an exception if we get here -- the setup won't be properly run.
#endif

	if (!mOISInputManager)
		return 4;
	LOG_MANAGER->log("Successfully created OIS Input System", LL_NORMAL);

	// Create the other OIS input objects (this part should be cross-platform)
	mOISKeyboard = static_cast<OIS::Keyboard*>(mOISInputManager->createInputObject(OIS::OISKeyboard, true));
	if (!mOISKeyboard)
		return 5;
	mOISKeyboard->setEventCallback(this);
	LOG_MANAGER->log("Successfully created OIS Keyboard object", LL_NORMAL);

	mOISMouse = static_cast<OIS::Mouse*>(mOISInputManager->createInputObject(OIS::OISMouse, true));
	if (!mOISMouse)
		return 5;
	mOISMouse->setEventCallback(this);
	LOG_MANAGER->log("Successfully created OIS Mouse object", LL_NORMAL);

	// Idenitify our resource locations and load those resources.
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("..\\media", "FileSystem");
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	LOG_MANAGER->log("Loaded all resources", LL_NORMAL);

	// Create the Ogre Scene Manager
	mSceneManager = mRoot->createSceneManager("OctreeSceneManager");

	// Call our createScene method
	if (createScene())
	{
		LOG_MANAGER->log("Error creating scene", LL_ERROR);
		return 7;
	}
	LOG_MANAGER->log("Created scene", LL_NORMAL);


	// We're done!  Set our state to initialized.
	mState = ApplicationState::initialized;
	return 0;
}



int ssurge::Application::shutdown()
{
	if (mState == ApplicationState::dead)
		return 0;		// Already cleaned up!

	// Destroy the OIS Input Objects
	if (mOISKeyboard && mOISInputManager)
		mOISInputManager->destroyInputObject(mOISKeyboard);
	mOISKeyboard = NULL;
	LOG_MANAGER->log("Destroyed OIS Keyboard object", LL_NORMAL);

	if (mOISMouse && mOISInputManager)
		mOISInputManager->destroyInputObject(mOISMouse);
	mOISMouse = NULL;
	LOG_MANAGER->log("Destroyed OIS Mouse object", LL_NORMAL);

	// Destroy the OIS Input Manager
	if (mOISInputManager)
		OIS::InputManager::destroyInputSystem(mOISInputManager);
	mOISInputManager = NULL;
	LOG_MANAGER->log("Destroyed OIS Input System", LL_NORMAL);

	/// Destroy the Ogre root (this should clean up any Ogre Objects)
	if (mRoot)
		delete mRoot;
	mRoot = NULL;
	mWindow = NULL;
	LOG_MANAGER->log("Destroyed Ogre root", LL_NORMAL);

	/// Destroy the LogManager
	LOG_MANAGER->log("About to destroy LogManager...goodbye.", LL_NORMAL);
	if (mLogManager)
		delete mLogManager;
	mLogManager = NULL;
	

	mState = ApplicationState::dead;
	return 0;
}



void ssurge::Application::rotateCamera(float mRotSpd, bool x)
{
	if (!x)
	{
		mCamera->yaw(Ogre::Degree(mCamera->getRealUp().x * mRotSpd));
		mCamera->pitch(Ogre::Degree(mCamera->getRealUp().y * mRotSpd));
	}
	else
	{
		mCamera->yaw(Ogre::Degree(mCamera->getRealRight().x * mRotSpd));
		mCamera->pitch(Ogre::Degree(mCamera->getRealRight().y * mRotSpd));
	}

	/*Original Mouse Code
	mCamera->yaw(Ogre::Degree(-me.state.X.rel * mRotSpd));
	mCamera->pitch(Ogre::Degree(-me.state.Y.rel * mRotSpd));
	*/
}



bool ssurge::Application::keyPressed(const OIS::KeyEvent & arg)
{
	char temps[512];
	sprintf_s(temps, 511, "A key was pressed: %d",arg.key);
	LOG_MANAGER->log(std::string(temps), LL_DEBUG);

	//NEW:
	//Hit Backspace to change the camera modes
	if (arg.key == OIS::KC_BACK)
	{
		if (camMode == ssurge::CameraMode::GHOST)
		{
			camMode = ssurge::CameraMode::THIRD;
			std::cout << "THIRD PERSON" << std::endl;
			updateOgreCamera();
		}
		else if (camMode == ssurge::CameraMode::FIRST)
		{
			camMode = ssurge::CameraMode::GHOST;
			std::cout << "GHOST" << std::endl;
		}
		else
		{
			camMode = ssurge::CameraMode::FIRST;
			std::cout << "FIRST PERSON" << std::endl;
			updateOgreCamera();
		}
	}

	if (arg.key == OIS::KC_LSHIFT)
		shiftDown = true;

	if (arg.key == OIS::KC_ESCAPE)
		mState = ApplicationState::readyToQuit;
	return true;
}



bool ssurge::Application::keyReleased(const OIS::KeyEvent & arg)
{
	if (shiftDown)
		shiftDown = false;
	return true;
}



bool ssurge::Application::mousePressed(const OIS::MouseEvent& me, OIS::MouseButtonID id)
{
	if (id == OIS::MB_Left)
	{
		mLMouseDown = true;
	}
	else if (id == OIS::MB_Right)
	{
		mRMouseDown = true;
		//CEGUI::MouseCursor::getSingleton().show();
	}

	return true;
}



bool ssurge::Application::mouseReleased(const OIS::MouseEvent & me, OIS::MouseButtonID id)
{
	if (id == OIS::MB_Left)
	{
		mLMouseDown = false;
	}
	else if (id == OIS::MB_Right)
	{
		mRMouseDown = false;
		//CEGUI::MouseCursor::getSingleton().show();
	}
	return true;
}



bool ssurge::Application::mouseMoved(const OIS::MouseEvent & me)
{
	float mRotSpd = 1.0f;

	if (mRMouseDown && camMode==ssurge::CameraMode::GHOST)
	{
		mCamera->yaw(Ogre::Degree(-me.state.X.rel * mRotSpd));
		mCamera->pitch(Ogre::Degree(-me.state.Y.rel * mRotSpd));
	}

	if (mRMouseDown && camMode == ssurge::CameraMode::THIRD)
	{
		// if not ghost, set the camera around the parent ogre head
		Ogre::Vector3 ogrePos = mSceneManager->getSceneNode("OgreHeadNode")->_getDerivedPosition();

		mCamera->setPosition(ogrePos);
		//mCamera->setOrientation(Ogre::Quaternion::IDENTITY);
		mCamera->yaw(Ogre::Degree(-me.state.X.rel * mRotSpd));
		mCamera->pitch(Ogre::Degree(-me.state.Y.rel * mRotSpd));
		mCamera->moveRelative(Ogre::Vector3(0, 0, mZoom));
	}

	return true;
}



void ssurge::Application::updateCameraPositionRelative(Ogre::Vector3 translation)
{
	/*
	mCamera->setOrientation(orientt);
	mCamera->pitch(Ogre::Radian(Ogre::Math::PI));
	mCamera->roll(Ogre::Radian(Ogre::Math::PI));
	*/

	Ogre::Quaternion orient = mSceneManager->getSceneNode("OgreHeadNode")->_getDerivedOrientation();
	Ogre::Quaternion orientt = mSceneManager->getSceneNode("OgreHeadNode")->getInitialOrientation();

	if (camMode == ssurge::CameraMode::FIRST)
	{
		mSceneManager->getSceneNode("OgreHeadNode")->translate(translation);
		Ogre::Vector3 ogrePos = mSceneManager->getSceneNode("OgreHeadNode")->_getDerivedPosition();
		mCamera->setPosition(ogrePos);

		mCamera->setOrientation(orientt);
		mCamera->yaw(Ogre::Radian(Ogre::Math::PI));

		mCamera->moveRelative(Ogre::Vector3(0, 0, -mZoom));
	}
	else
	{
		mSceneManager->getSceneNode("OgreHeadNode")->translate(translation);
		Ogre::Vector3 ogrePos = mSceneManager->getSceneNode("OgreHeadNode")->_getDerivedPosition();
		mCamera->setPosition(ogrePos);

		//mCamera->setOrientation(orientt);
		//mCamera->pitch(Ogre::Radian(Ogre::Math::PI));
		//mCamera->roll(Ogre::Radian(Ogre::Math::PI));

		mCamera->moveRelative(Ogre::Vector3(0, 0, mZoom));
	}
}



void ssurge::Application::updateOgreCamera()
{
	///Don't need to update if we're floating around the scene.
	if (camMode == ssurge::CameraMode::GHOST)
		return;

	/// The relative FPS vector
	///Redundant as of right now.
	//Ogre::Vector3 FPSRelative(0, 0, 0);

	if (mOISKeyboard->isKeyDown(OIS::KC_UP))
		updateCameraPositionRelative(Ogre::Vector3(0, 0, 1));
	else if (mOISKeyboard->isKeyDown(OIS::KC_DOWN))
		updateCameraPositionRelative(Ogre::Vector3(0, 0, -1));
	else if (mOISKeyboard->isKeyDown(OIS::KC_LEFT))
		updateCameraPositionRelative(Ogre::Vector3(-1, 0, 0));
	else if (mOISKeyboard->isKeyDown(OIS::KC_RIGHT))
		updateCameraPositionRelative(Ogre::Vector3(1, 0, 0));
}



int ssurge::Application::run()
{
	if (mState != ApplicationState::initialized)
		return 1;

	mState = ApplicationState::running;

	mTimer.reset();

	while (mState == ApplicationState::running)
	{
		/// Do updates
		float dt = mTimer.getMicroseconds() * 0.000001f;		/// Converts to seconds.
		
		mTimer.reset();
		if (!update(dt))
			break;

		/// Check input
		mOISKeyboard->capture();
		mOISMouse->capture();

		if (shiftDown)
			mShiftSpeed = -2.5f;
		else
			mShiftSpeed = -0.5f;

		///Move ogre and update the camera
		updateOgreCamera();

		/// Change the third person camera zoom
		if (mOISKeyboard->isKeyDown(OIS::KC_W) && camMode == ssurge::CameraMode::THIRD)
			mZoom--;
		else if (mOISKeyboard->isKeyDown(OIS::KC_S) && camMode == ssurge::CameraMode::THIRD)
			mZoom++;

		if (mOISKeyboard->isKeyDown(OIS::KC_W) && camMode == ssurge::CameraMode::GHOST)
		{
			ssurge::Application::moveCamera(mShiftSpeed, mSceneManager->getCamera(ssurge::Application::mCameraName)->getDirection()*-1);
		}
		else if (mOISKeyboard->isKeyDown(OIS::KC_A) && camMode == ssurge::CameraMode::GHOST)
		{
			ssurge::Application::moveCamera(mShiftSpeed, mSceneManager->getCamera(ssurge::Application::mCameraName)->getRight());
		}
		else if (mOISKeyboard->isKeyDown(OIS::KC_S) && camMode == ssurge::CameraMode::GHOST)
		{
			ssurge::Application::moveCamera(mShiftSpeed, mSceneManager->getCamera(ssurge::Application::mCameraName)->getDirection());
		}
		else if (mOISKeyboard->isKeyDown(OIS::KC_D) && camMode == ssurge::CameraMode::GHOST)
		{
			ssurge::Application::moveCamera(mShiftSpeed, mSceneManager->getCamera(ssurge::Application::mCameraName)->getRight()*-1);
		}
		
		if (mWindow->isClosed())						// true if the close button was pressed
			mState = ApplicationState::readyToQuit;

		// Draw
		mRoot->renderOneFrame(dt);

		// Not technically necessary, but makes this app play nicely with 
		// other processes that are running
		Ogre::WindowEventUtilities::messagePump();
	}

	// Trigger a system shutdown.
	shutdown();

	return 0;
}



int ssurge::Application::createScene()
{
	Ogre::SceneNode * snode;
	Ogre::Entity * ent;

	// Create a camera and a viewport
	
	Ogre::Camera * cam3rdPerson = mSceneManager->createCamera("ThirdPersonCam");
	//NEW:
	Application::mCameraName = "ThirdPersonCam";

	Ogre::Viewport * viewport = mWindow->addViewport(cam3rdPerson);
	viewport->setBackgroundColour(Ogre::ColourValue(0.3f, 0.3f, 0.3f));
	float aspect = (float)mWindow->getWidth() / mWindow->getHeight();
	cam3rdPerson->setAspectRatio(aspect);
	cam3rdPerson->setNearClipDistance(0.1f);
	cam3rdPerson->setFarClipDistance(1000.0f);
	cam3rdPerson->setPosition(0.0f, 50.0f, 250.0f);
	cam3rdPerson->lookAt(0, 0, 0);

	//NEW:
	mCamera = cam3rdPerson;
	//Set the (default) camera mode to ghost
	camMode = ssurge::CameraMode::GHOST;

	// Set shadow option
	mSceneManager->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
	mSceneManager->setShadowColour(Ogre::ColourValue(0.6f, 0.6f, 0.6f));
	mSceneManager->setShadowTextureCount(5);
	mSceneManager->setShadowTextureSize(4096);
	mSceneManager->setAmbientLight(Ogre::ColourValue(0.3f, 0.3f, 0.3f));

	// Create the ogre head and plane and load into the scene
	snode = mSceneManager->getRootSceneNode()->createChildSceneNode("OgreHeadNode");
	ent = mSceneManager->createEntity("OgreHead", "ogrehead.mesh");
	snode->attachObject(ent);

	snode->translate(0, 30, 0);

	for (int i = 0; i < (int)ent->getNumSubEntities(); i++)
		ent->getSubEntity(i)->getMaterial()->setReceiveShadows(false);

	snode = mSceneManager->getRootSceneNode()->createChildSceneNode("FloorNode");
	ent = mSceneManager->createEntity("Floor", "marblefloor.mesh");
	snode->scale(150, 1, 150);
	snode->attachObject(ent);
	
	//NEW:
	shiftDown = false;
	mShiftSpeed = -0.5f;
	mZoom = 100;
	ogrePosition = mSceneManager->getSceneNode("OgreHeadNode")->_getDerivedPosition();
	return 0;
}



int ssurge::Application::update(float dt)
{
	mSpawnTimer += dt;
	if (mSpawnTimer >= 2.0f)
	{
		std::cout << "LIGHT!" << std::endl; 
		Ogre::SceneNode * snode = mSceneManager->getRootSceneNode()->createChildSceneNode();
		Ogre::Light * light = mSceneManager->createLight();
		std::stringstream ss;
		ss << "Bullet" << mNextBulletID++;
		Ogre::Entity * ent = mSceneManager->createEntity(ss.str(), "PlasmaBall.mesh");
		ent->setCastShadows(false);
		snode->attachObject(ent);
		snode->attachObject(light);
		light->setType(Ogre::Light::LT_POINT);
		light->setAttenuation(250.0f, 1.0, 0.01f, 0.0f);
		light->setDiffuseColour(Ogre::ColourValue(rand_float(0.5f, 1), rand_float(0.5f, 1), rand_float(0.5f, 1)));
		light->setShadowFarClipDistance(1000.0f);
		light->setShadowFarDistance(1000.0f);
		snode->setPosition(250, 65, rand_float(-100, 100));
		snode->scale(2, 2, 2);
		mBullets.push_back(snode);
		mSpawnTimer = 0.0f;
	}

	for (int i = (int)mBullets.size() - 1; i >= 0; i--)
	{
		mBullets[i]->translate(Ogre::Vector3(-50.0f * dt, 0, 0));
		Ogre::Vector3 worldPos = mBullets[i]->_getDerivedPosition();
	
		if (worldPos.x < -250)
		{
			Ogre::Entity * ent = static_cast<Ogre::Entity*>(mBullets[i]->getAttachedObject(0));
			Ogre::Light * light = static_cast<Ogre::Light*>(mBullets[i]->getAttachedObject(1));
			mBullets[i]->detachAllObjects();
			mSceneManager->destroyEntity(ent);
			mSceneManager->destroyLight(light);
			mSceneManager->getRootSceneNode()->removeChild(mBullets[i]);
			mSceneManager->destroySceneNode(mBullets[i]);
			mBullets.erase(mBullets.begin() + i);
		}
	}

	return 1;		// Keep rendering
}