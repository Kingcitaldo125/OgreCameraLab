#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <stdafx.h>
#include <LogManager.h>
#include <Singleton.h>

namespace ssurge
{
/// The various states the application can be in.
enum class ApplicationState { created, initialized, running, readyToQuit, dead };

///The camera modes
enum class CameraMode {GHOST, THIRD, FIRST};

/// Convenience macro to make accessing the singleton a bit less cumbersome
#define APPLICATION ssurge::Application::getSingletonPtr()

/// The Application class is basically the hub of the entire program.
/// It should contain all of (ssurge's) managers and all shutdown
/// and startup code.  Everything else should (eventually) be
/// delegated to the managers.
class Application : public OIS::KeyListener, public OIS::MouseListener, public Singleton<Application>
{
/***** ATTRIBUTES *****/
protected:
	///The Main Scene Camera
	Ogre::Camera * mCamera;

	///The main camera mode
	CameraMode camMode;

	bool keyDown,shiftDown;
	float mShiftSpeed, mZoom;

	/// Mouse look controls
	bool mLMouseDown, mRMouseDown;

	///Ogre Head move controls
	Ogre::Vector3 ogrePosition;

	std::string mCameraName;

	/// The Ogre Root objects
	Ogre::Root * mRoot;

	/// The Ogre-controlled window we're rendering to
	Ogre::RenderWindow * mWindow;

	/// [Temporary] The OIS Input Manager
	OIS::InputManager * mOISInputManager;

	/// [Temporary] The OIS Keyboard object
	OIS::Keyboard * mOISKeyboard;

	/// [Temporary] The OIS Mouse object
	OIS::Mouse * mOISMouse;

	/// The state of the application
	ApplicationState mState;

	/// The LogManager
	LogManager * mLogManager;

	/// The Ogre Scene Manager
	Ogre::SceneManager * mSceneManager;

	/// An Ogre Timer object (used to calculate time between frames)
	Ogre::Timer mTimer;

	/// [temporary, for lab08 and lab11 only] A list of bullet scene nodes.
	std::vector<Ogre::SceneNode*> mBullets;

	/// [temporary, for lab08 and lab11 only] The ID# of the next bullet
	unsigned int mNextBulletID;

	/// Time since the last bullet was spawned.
	float mSpawnTimer;


/***** CONSTRUCTORS / DESTRUCTORS *****/
public:
	/// The Basic constructor.  Doesn't do much -- call initialize to fully initialize the application
	Application();

	/// Frees any memory we allocated and calls the shutdown method.
	virtual ~Application();

	/// Initializes the application.  Call this before calling run.  Returns 0 on success.  If we
	/// think we might sub-class the Application class, it would probably be smart to break this method up.
	virtual int initialize(std::string window_title);

	/// The shutdown method.   Returns 0 on success.
	virtual int shutdown();


/***** CALLBACK METHODS (not meant to be called directly -- called by "someone" else) *****/
protected:
	/// [Temporary] Called when a key is pressed
	virtual bool keyPressed(const OIS::KeyEvent & arg);

	/// [Temporary] Called when a key is released
	virtual bool keyReleased(const OIS::KeyEvent & arg);

	///[Temporary] Called when the mouse is pressed
	virtual bool mousePressed(const OIS::MouseEvent& me, OIS::MouseButtonID id);

	///[Temporary] Called when the mouse is released
	virtual bool mouseReleased(const OIS::MouseEvent& me, OIS::MouseButtonID id);

	///[Temporary] Called when the mouse is moved
	virtual bool mouseMoved(const OIS::MouseEvent& me);


/***** METHODS *****/
public:
	/// Starts the internal game loop.  Returns 0 if we had a clean end to the main loop.
	int run();

protected:
	/// Sets up the scene
	int createScene();

	/// Does any internal updates (called once per frame (before rendering)).  If this method
	/// returns 0, the game shuts down.
	int update(float dt);

	///Moves the camera a certain direction
	void moveCamera(float mSpeed,Ogre::Vector3 direction)
	{mCamera->move(direction*mSpeed);};

	///Rotates the camera by a certain speed
	void rotateCamera(float mRotSpd, bool x);

	/// Updates the camera's position assuming we're in a third person mode
	void updateOgreCamera();

	///A helper function for updateOgreCamera()
	void updateCameraPositionRelative(Ogre::Vector3 translation);
};

}

#endif