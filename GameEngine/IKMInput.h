#ifndef _IKMINPUT_
#define _IKMINPUT_

#include "IInput.h"
#include "VecMath.h"

enum KeyCode
{
	BACKSPACE = 8,
	TAB,
	ENTER = 13,
	ESCAPE = 27,
	SPACE = 32,
	LEFT = 37, UP,RIGHT,DOWN,
	A = 65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z, 
	TILDE = 96,
	F1 = 112, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
	DEL = 127,
};

// todo: remove this
enum MouseCursorState
{
	Default,
	Hand,
	Beam,
};

// Keyboard/Mouse
class IKMInput : public IInputPlugin
{
public:

	//note: the once parameter:
	//true, to only process the event the first time you press, false, returns true the entire time of the press

	// keyboard
	virtual bool KeyDown(int Key, bool once = true) = 0;
    virtual bool KeyUp(int Key, bool once = true) = 0;


	//need to be reworked (ignore these)
	virtual bool IsKeyDown() const = 0;
	virtual int GetKeyDown() const = 0;

	// returns true if you click, button is down
    virtual bool MouseClick(int Button, bool once = true) const = 0;
    virtual bool MouseRelease(int Button, bool once = true) const = 0;

	//gets the mouse position
	virtual void MousePos(int& x, int& y) const = 0;

	//gets mouse position in world space
	virtual const glm::vec2& GetTransformedMousePos() const = 0;

	virtual int MouseX() const = 0; // horizontal
	virtual int MouseY() const = 0; // vertical
	virtual int MouseZ() const = 0; // scroll

	// selection box; returns true if user clicks
	virtual bool GetSelectedRect(Math::AABB& out) = 0; 

protected:

	virtual ~IKMInput() {}

};

#endif //_IKMINPUT_
