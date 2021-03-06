
#include "QuadTree.h"
#include "IRenderer.h"
#include "NodeIterator.h"
#include <algorithm>
#include <stack>
#include <fstream>

using namespace std;

QuadTree::QuadTree(const Math::FRECT& R, unsigned int nodeCapacity, unsigned int depthLimit) :
	m_Rect(R), m_Previous(nullptr), m_iCapacity(nodeCapacity), m_iHeight(0), m_iDepthLimit(depthLimit)
{
	SubDivide();
}

QuadTree::QuadTree(const Math::FRECT& rect, unsigned int nodeCapacity, unsigned int depthLimit, unsigned int height, QuadTree* pPrevious) :
	m_Rect(rect), m_Previous(pPrevious), m_iCapacity(nodeCapacity), m_iHeight(height), m_iDepthLimit(depthLimit)
{
}

bool QuadTree::Insert(ISpatialObject& obj)
{
	if (m_Rect.Intersects(obj.GetCollisionPolygon()))
	{
		return RInsert(obj);
	}

	return false;
}

void QuadTree::Erase(ISpatialObject& obj)
{
	if (IsDivided())
	{
		const Math::ICollisionPolygon& collisionPoly = obj.GetCollisionPolygon();

		// Loop through all of the sub nodes
		for (unsigned int i = 0; i < m_Nodes.size(); ++i)
		{
			QuadTree& subNode = m_Nodes[i];

			if (subNode.m_Rect.Intersects(collisionPoly))
			{
				subNode.Erase(obj);
			}
		}
	}
	else
	{
		m_Objects.erase(&obj);
	}
}

void QuadTree::QueryNearObjects(const ISpatialObject* pObj, std::vector<ISpatialObject*>& out)
{
	QueryNearObjects(pObj->GetCollisionPolygon(), out, pObj);
}

void QuadTree::QueryNearObjects(const Math::ICollisionPolygon& poly, std::vector<ISpatialObject*>& out)
{
	QueryNearObjects(poly, out, nullptr);
}

void QuadTree::SubDivide(int levels)
{
	if(levels > 0)
	{
		if(IsDivided() == false)
		{
			SubDivide();
		}

		for(auto& node : m_Nodes)
		{
			node.SubDivide(levels - 1);
		}
	}
}

bool QuadTree::IsWithin(ISpatialObject& obj) const
{
	return m_Rect.Intersects(obj.GetCollisionPolygon());
}

const Math::FRECT& QuadTree::GetRect() const
{
	return m_Rect.GetRect();
}

void QuadTree::Render(IRenderer& renderer, float width)
{
	for (NodeIterator iter = this; (*iter) != nullptr; ++iter)
	{
		if(iter->HasObjects())
		{
			const Math::FRECT& R = iter->GetRect();

			glm::vec3 pos[5] =
			{
				glm::vec3(R.topLeft.x, R.topLeft.y, -20.0f),
				glm::vec3(R.bottomRight.x, R.topLeft.y, -20.0f),
				glm::vec3(R.bottomRight.x, R.bottomRight.y, -20.0f),
				glm::vec3(R.topLeft.x, R.bottomRight.y, -20.0f),
				glm::vec3(R.topLeft.x, R.topLeft.y, -20.0f),
			};

			renderer.DrawLine(pos, 5, width);
		}
	}
}

bool QuadTree::RInsert(ISpatialObject& obj)
{
	if (IsDivided())
	{
		const Math::ICollisionPolygon& collisionPoly = obj.GetCollisionPolygon();

		// iterate over the near nodes
		for(QuadTree& subNode : m_Nodes)
		{
			if (subNode.m_Rect.Intersects(collisionPoly))
			{
				// if the current node is full
				if (subNode.IsFull() && subNode.m_iHeight < m_iDepthLimit)
				{
					if (!subNode.IsDivided())
					{
						subNode.SubDivide();
					}
				}

				subNode.RInsert(obj);
			}
		}
	}
	else
	{
		m_Objects.insert(&obj);
	}

	return true;
}

bool QuadTree::IsDivided() const
{
	return (!m_Nodes.empty());
}

bool QuadTree::HasObjects() const
{
	return (!m_Objects.empty());
}

bool QuadTree::IsFull() const
{
	return (m_Objects.size() >= m_iCapacity);
}


void QuadTree::SubDivide()
{
	const Math::FRECT& rect = m_Rect.GetRect();

	glm::vec2 middle = rect.Middle();
	const glm::vec2& topLeft = rect.topLeft;
	const glm::vec2& bottomRight = rect.bottomRight;

	Math::FRECT subRects[] =
	{
		Math::FRECT(topLeft,middle),
		Math::FRECT(glm::vec2(middle.x,topLeft.y),glm::vec2(bottomRight.x,middle.y)),
		Math::FRECT(glm::vec2(topLeft.x,middle.y),glm::vec2(middle.x,bottomRight.y)),
		Math::FRECT(middle,bottomRight)
	};

	m_Nodes.reserve(4);

	// Loop over all sub-quadrants, and create them
	for(unsigned int i = 0; i < 4; ++i)
	{
		m_Nodes.push_back(QuadTree(subRects[i], m_iCapacity, m_iDepthLimit, m_iHeight + 1, this));

		for(auto& iter : m_Objects)
		{
			m_Nodes[i].Insert(*iter);
		}
	}

	m_Objects.clear();
}

void QuadTree::QueryNearObjects(const Math::ICollisionPolygon& poly, std::vector<ISpatialObject*>& out, const ISpatialObject* pObj)
{
	std::unordered_set<ISpatialObject*> onceFilter;
	stack<QuadTree*> theStack;

	theStack.push(this);

	while(!theStack.empty())
	{
		QuadTree* pTop = theStack.top();
		theStack.pop();

		// Loop through all of the sub nodes
		for(unsigned int i = 0; i < pTop->m_Nodes.size(); ++i)
		{
			QuadTree& subNode = pTop->m_Nodes[i];

			if(subNode.m_Rect.Intersects(poly))
			{
				if(!subNode.IsDivided())
				{
					auto& objList = subNode.m_Objects;

					for(auto iter = objList.begin(); iter != objList.end(); ++iter)
					{
						if(pObj == nullptr || (&pObj->GetCollisionPolygon()) != (&(*iter)->GetCollisionPolygon()))
						{
							if((*iter)->GetCollisionPolygon().Intersects(poly))
							{
								auto onceFilterIter = onceFilter.insert(*iter);

								if(onceFilterIter.second)
								{
									out.push_back(*iter);
								}
							}
						}
					}
				}
				else
				{
					theStack.push(&subNode);
				}
			}
		}
	}
}




