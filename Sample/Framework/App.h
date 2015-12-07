/*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "stdafx.h"
#include "DebugInfo.h"

class App : public Singleton<App>
{
public:
    App();
    virtual ~App();

	void 
		SetCachePath(const char * path) { mCachePath = path; }
	const char *  
		GetCachePath() { return mCachePath.c_str(); }
	virtual void 
		Init(HINSTANCE hInst, HWND hWnd, int w, int h);
    virtual void 
		Update();
	virtual void 
		Shutdown();
	virtual void 
		Pause();
	virtual void 
		Resume();
	virtual void 
		Resize(int w, int h);

	virtual void 
		InjectMouseMove(float _x, float _y);
	virtual void 
		InjectMouseWheel(float _z);
	virtual void 
		InjectMouseDown(int _id, float _x, float _y);
	virtual void 
		InjectMouseUp(int _id, float _x, float _y);
	virtual void 
		InjectKeyDown(int _key, uchar_t _text);
	virtual void 
		InjectKeyUp(int _key);

	virtual void 
		InjectTouchDown(int _id, float _x, float _y);
	virtual void 
		InjectTouchUp(int _id, float _x, float _y);
	virtual void 
		InjectTouchMove(int _id, float _x, float _y);
	virtual void 
		InjectTouchCancel(int _id, float _x, float _y);

	virtual void 
		OnSetupResource() = 0;
	virtual void 
		OnInit() = 0;
	virtual void 
		OnUpdate() = 0;
	virtual void 
		OnShutdown() = 0;
	virtual void 
		OnPause() = 0;
	virtual void 
		OnResume() = 0;
	virtual void 
		OnResize(int w, int h) {}
	virtual void 
		OnDragFile(const char * filename) {}

	void 
		MapScreenUnit(float & x, float & y);

	HINSTANCE 
		AppInst() { return mInst; }
	HWND 
		AppWnd() { return mWnd; }

protected:
	FixedString256 mCachePath;
	HINSTANCE mInst;
	HWND mWnd;

	Root * mRoot;
	ResourceManager * mResourceManager;
	GLRenderSystem * mRenderSystem;
#ifndef NO_AUDIO
	ALAudioSystem * mAudioSystem;
#endif
	MGUI::Engine * mUIEngine;
	ParticleFX * mParticleFX;
	World * mWorld;
	PhyWorld * mPhyWorld;
	DebugInfo * mDebugInfo;

#ifndef NO_INPUT
	DIInputSystem * mInputSystem;
#endif

	bool mPause;
	float mFPSLimit;
	float mInternalLastTime;
};

#ifdef M_PLATFORM_WIN32
#ifdef APP_EXPORT
#define APP_ENTRY __declspec(dllexport)
#endif
#endif

#ifndef APP_ENTRY
#define APP_ENTRY
#endif

extern "C" {

	APP_ENTRY void CreateApplication(App ** ppApp);

	typedef void (*CREATE_APPLICATION_FUNCTION)(App ** ppApp);
};


