// Programmed by Bryce Mehring
// todo: need to clean up the interfaces.

#ifndef _QUADTREE_
#define _QUADTREE_

#include "VecMath.h"
#include "ISpatialObject.h"
#include "IRenderer.h"
#include <unordered_set>
#include <vector>

// The Quadtree manages objects in 2D space

class QuadTree
{
public:

	QuadTree(const Math::FRECT& R, unsigned int nodeCapacity, unsigned int depthLimit);

	// Inserts obj into the quadtree
	// returns true if obj was inserted into the quadtree, else false
	bool Insert(ISpatialObject& obj);

	// Removes obj from the quadtree
	void Erase(ISpatialObject& obj);

	// Returns all objects in the quadtree that collide with pObj or poly via the out parameter
	void QueryNearObjects(const ISpatialObject* pObj, std::vector<ISpatialObject*>& out);
	void QueryNearObjects(const Math::ICollisionPolygon& poly, std::vector<ISpatialObject*>& out);

	// SubDivide every node until the level specified
	void SubDivide(int levels);

	// Returns true if obj lies within the quadtree
	bool IsWithin(ISpatialObject& obj) const;

	// Returns the quadtree rectangle
	const Math::FRECT& GetRect() const;

	// Draws the quadtree rectangles as lines
	void Render(IRenderer& renderer, float width);

private:

	QuadTree(const Math::FRECT& rect, unsigned int nodeCapacity, unsigned int depthLimit, unsigned int height, QuadTree* pPrevious);

	// Inserts obj into the quadtree
	// returns true if obj was inserted into the quadtree, else false
	bool RInsert(ISpatialObject& obj);

	// Returns true if this node is divided
	bool IsDivided() const;

	// Returns true if this node has objects inserted into it
	bool HasObjects() const;

	// Returns true if this part of the quadtree has reached its capacity
	bool IsFull() const;

	// Subdivides the current node into 4 sub nodes
	void SubDivide();

	void QueryNearObjects(const Math::ICollisionPolygon& poly, std::vector<ISpatialObject*>& out, const ISpatialObject* pObj);

private:

	typedef std::unordered_set<ISpatialObject*> LIST_DTYPE;

	friend class NodeIterator;

	Math::CRectangle m_Rect;
	LIST_DTYPE m_Objects;

	std::vector<QuadTree> m_Nodes;
	QuadTree* const m_Previous;

	unsigned int m_iCapacity;
	const unsigned int m_iHeight;
	const unsigned int m_iDepthLimit;
};

#endif // _QUADTREE_
