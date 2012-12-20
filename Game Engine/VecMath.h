// Programmed by Bryce Mehring
// todo: need to insert this into every file...

#ifndef _VECMATH_
#define _VECMATH_

#include "d3dx9math.h"
#include "DxPolygon.h"
#include <string>
#include <vector>

namespace Math
{

// Basic rect, using floats
struct FRECT
{
	FRECT() {}
	FRECT(const D3DXVECTOR2& topLeft, const D3DXVECTOR2& bottomRight) : topLeft(topLeft), bottomRight(bottomRight)
	{
	}

	D3DXVECTOR2 Middle() const { return (topLeft + bottomRight) * 0.5f; }

	bool IsPointWithin(const D3DXVECTOR2& pos) const
	{
		return (pos.x >= topLeft.x) && (pos.x <= bottomRight.x) && (pos.y >= bottomRight.y) && (pos.y <= topLeft.y);
	}
	
	RECT Rect() const
	{
		RECT R = {(long)topLeft.x,(long)topLeft.y,(long)bottomRight.x,(long)bottomRight.y};
		return R;
	}

	float Width() const
	{
		return (bottomRight.x - topLeft.x);
	}

	float Height() const
	{
		return (topLeft.y - bottomRight.y);
	}

	// sets new center
	FRECT& operator =(const D3DXVECTOR2& pos)
	{
		D3DXVECTOR2 diff = (bottomRight - topLeft)*0.5f;
		topLeft = pos - diff;
		bottomRight = pos + diff;

		return *this;
	}

	// todo: rename to leftTop, rightBottom;
	D3DXVECTOR2 topLeft;
	D3DXVECTOR2 bottomRight;
};

// Basic circle structure
struct Circle
{
	Circle() {}
	Circle(const D3DXVECTOR2& c, float r) : center(c), r(r)
	{
	}

	bool IsPointWithin(const D3DXVECTOR2& pos) const
	{
		 return (D3DXVec2LengthSq(&(pos - center)) < (r*r));
	}

	Circle& operator =(const D3DXVECTOR2& c)
	{
		center = c;
		return *this;
	}

	D3DXVECTOR2 center;
	float r;

};

// returns the reflection vector, if dir points in the dir of movement, use (-dir).
D3DXVECTOR2 Reflect(const D3DXVECTOR2& dir, const D3DXVECTOR2& normal);
D3DXVECTOR3 Reflect(const D3DXVECTOR3& dir, const D3DXVECTOR3& normal);

// returns a random float in the range of [a,b]
float GetRandFloat(float a, float b);
unsigned int GetRandInt(unsigned int a, unsigned int b);

// clamps x into the range of [a,b]
template< class T >
T Clamp(T x, T a, T b)
{
	return x < a ? a : (x > b ? b : x);
}

bool InRange(float value, float min, float max);

bool Equals(float a, float b, float diff = 0.0001f);

unsigned int LOG2(unsigned int i);
unsigned int LOG10(unsigned int i);

// If rays interest, the return value is not -1
float RayCircleInsersection(const D3DXVECTOR2& c, float r, const D3DXVECTOR2& pos, const D3DXVECTOR2& dir);

//bool Intersects(const std::vector<D3DXVECTOR3>& poly1, const std::vector<D3DXVECTOR3>& poly2);

// ray casting algorithm
bool IsPointInPolygon(const D3DXVECTOR2* pArray, unsigned int length, POINT P);
bool IsPointInPolygon(const D3DXVECTOR3* pArray, unsigned int length, POINT P);

bool Sat(const std::vector<D3DXVECTOR3>& poly1, const std::vector<D3DXVECTOR3>& poly2);

float PongRayTrace(D3DXVECTOR2 pos, D3DXVECTOR2 dir, float fLeftBound); 

bool IsPrime(unsigned int);

void RegisterScriptVecMath(class asIScriptEngine* pEngine);

std::string ConvertTo(unsigned int uiInputNumber, unsigned int uiTargetBase);

unsigned int NumDigits(unsigned int uiNumber);


// collision interface

class ICollisionPolygon
{
public:

	enum Type
	{
		CircleType,
		RectangleType,
	};

	virtual ~ICollisionPolygon() {}

	// returns true if pOther interests this, else false
	virtual bool Intersects(const ICollisionPolygon& other) const = 0;
	virtual void GetNormal(const D3DXVECTOR2& pos, D3DXVECTOR2& out) const = 0;

	// returns the type of the collision object
	virtual Type GetType() const = 0;

};

// collision circle

class CCircle : public ICollisionPolygon
{
public:

	// todo: default ctor?
	// ctor
	CCircle(const Circle& circle);

	virtual Type GetType() const { return ICollisionPolygon::CircleType; }
	virtual bool Intersects(const ICollisionPolygon& other) const;
	virtual void GetNormal(const D3DXVECTOR2& pos, D3DXVECTOR2& out) const;
	
	// circle access
	const Circle& GetCircle() const { return m_circle; }
	Circle& GetCircle() { return m_circle; }

private:

	Circle m_circle;

};

// todo: create another CRectangle class that is renderable
// collision rectangle
class CRectangle : public ICollisionPolygon
{
public:

	// ctror
	CRectangle() {}
	CRectangle(const FRECT& rect);

	virtual Type GetType() const { return ICollisionPolygon::RectangleType; }
	virtual bool Intersects(const ICollisionPolygon& other) const;
	virtual void GetNormal(const D3DXVECTOR2& pos, D3DXVECTOR2& out) const;

	// rect access
	const FRECT& GetRect() const { return m_rect; }
	FRECT& GetRect() { return m_rect; }

	void Render(IRenderer&);

private:

	FRECT m_rect;
	DxSquare m_polygon;

};

ICollisionPolygon* CreateCollionPolygon(const std::vector<D3DXVECTOR2>& poly);

} // math

#endif // _VECMATH_