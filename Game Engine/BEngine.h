
#pragma once

#include "Interfaces.h"
#include "Singleton.h"
#include "asVM.h"
#include <Windows.h>

using namespace std;


// ************************
// This really should just manage everything, do not implement it here
// All of the different sub parts of the engine should be independent, they should be their own dll.
// Then this class would load the dll's dynamically and be the leader of the game without knowing its implementation details.
// ***********************

// this class should not be a singleton, I should create another class that manages 
// game engines

class IBaseEngine : public AngelScript::IScripted
{
public:

	IBaseEngine(HINSTANCE hInstance,const string& winCaption);
	virtual ~IBaseEngine();

	void MsgProc(UINT msg, WPARAM wPraram, LPARAM lparam);

	HINSTANCE GetHINSTANCE() const;
	HWND GetWindowHandle() const;

	virtual void RegisterScript();
	virtual int Run() = 0;

protected:

	// ====== data members ======
	
	// win32 window
	HWND m_hWindowHandle;
	HINSTANCE m_hInstance;
	
	float m_fDT;
	float m_fStartCount;
	float m_fSecsPerCount;

	RECT m_rect;

	bool m_bPaused;

	// ====== helper functions ======

	/*bool Begin();
	bool End();
	void Present
	bool IsDeviceLost();
	void OnLostDevice();
	void OnResetDevice();
	*/

	void StartCounter();
	void EndCounter();

	bool Update();

private:

	void RedirectIOToConsole();
	void InitializeWindows(HINSTANCE hInstance, const string& winCaption);
};

extern IBaseEngine* g_pEngine;



