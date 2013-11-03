
#include "QuadTree.h"
#include "IRenderer.h"
#include "NodeIterator.h"
#include <algorithm>
#include <stack>
#include <set>
#include <queue>
#include <fstream>

using namespace std;

QuadTree::QuadTree(const Math::FRECT& rect, unsigned int nodeCapacity, unsigned int height, QuadTree* pPrevious) : 
	m_Rect(rect), m_iHeight(height), m_Previous(pPrevious), m_iCapacity(nodeCapacity)
{
}

QuadTree::QuadTree(const Math::FRECT& R, unsigned int nodeCapacity) : m_Rect(R), m_Previous(nullptr), m_iHeight(0), m_iCapacity(nodeCapacity)
{
	SubDivide();
}

bool QuadTree::IsWithin(ISpatialObject& obj) const
{
	return m_Rect.Intersects(obj.GetCollisionPolygon());
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

	// this will divide the current rect into MAX_NODES new rectangles
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

	// Loop over all rects, and create them
	for(unsigned int i = 0; i < 4; ++i)
	{
		m_Nodes.push_back(QuadTree(subRects[i],m_iCapacity,m_iHeight + 1,this));

		for(auto& iter : m_Objects)
		{
			m_Nodes[i].Insert(*iter);
		}
	}

	m_Objects.clear();
}

void QuadTree::Render(IRenderer& renderer)
{
	for(NodeIterator iter = this; (*iter) != nullptr; ++iter)
	{
		if(iter->HasObjects())
		{
			const Math::FRECT& R = iter->GetRect();

			::glm::vec3 pos[5] =
			{
				glm::vec3(R.topLeft.x,R.topLeft.y,-20.0f),
				glm::vec3(R.bottomRight.x,R.topLeft.y,-20.0f),
				glm::vec3(R.bottomRight.x,R.bottomRight.y,-20.0f),
				glm::vec3(R.topLeft.x,R.bottomRight.y,-20.0f),
				glm::vec3(R.topLeft.x,R.topLeft.y,-20.0f),
			};

			renderer.DrawLine(pos,5);
		}
	}
}

void QuadTree::Erase(ISpatialObject& obj)
{
	if(IsDivided())
	{
		const Math::ICollisionPolygon& poly = obj.GetCollisionPolygon();

		// Loop through all of the sub nodes
		for(unsigned int i = 0; i < m_Nodes.size(); ++i)
		{
			QuadTree& subNode = m_Nodes[i];

			if(subNode.m_Rect.Intersects(poly))
			{
				subNode.Erase(obj);
			}
		}
	}
	else
	{
		//m_Objects.erase(&obj);

		auto iter = std::find(m_Objects.begin(),m_Objects.end(),&obj);

		if(iter != m_Objects.end())
		{
			m_Objects.erase(iter);
		}
	}
}

bool QuadTree::Insert(ISpatialObject& obj)
{
	// If the point is within the the root
	if(IsWithin(obj))
	{
		RInsert(obj);
	}

	return true;
}

void QuadTree::RInsert(ISpatialObject& obj)
{
	// find the near nodes to pObj

	if(IsDivided())
	{
		std::vector<QuadTree*> nodes;

		FindNearNodes(obj.GetCollisionPolygon(),nodes);

		// as we iterate over the near nodes
		for(unsigned int i = 0; i < nodes.size(); ++i)
		{
			QuadTree* pNode = nodes[i];

			const Math::FRECT& subR = nodes[i]->m_Rect.GetRect();

			// if the current node is full
			if(pNode->IsFull() && subR.Height() > 4.0f)
			{
				if(!pNode->IsDivided())
				{
					pNode->SubDivide();
				}
			}

			pNode->RInsert(obj);
		}
	}
	else
	{
		m_Objects.push_back(&obj);
		//m_Objects.insert(&obj);
	}

}

void QuadTree::FindNearNodes(const Math::ICollisionPolygon& poly, std::vector<QuadTree*>& out)
{
	out.clear();

	// Loop through all of the sub nodes
	for(unsigned int i = 0; i < m_Nodes.size(); ++i)
	{
		QuadTree& subNode = m_Nodes[i];

		if(subNode.m_Rect.Intersects(poly))
		{
			out.push_back(&subNode);
		}
	}
}

void QuadTree::QueryNearObjects(const ISpatialObject* pObj, std::vector<ISpatialObject*>& out)
{
	QueryNearObjects(pObj->GetCollisionPolygon(),out,pObj);
}

void QuadTree::QueryNearObjects(const Math::ICollisionPolygon& poly, std::vector<ISpatialObject*>& out)
{
	QueryNearObjects(poly,out,nullptr);
}

void QuadTree::QueryNearObjects(const Math::ICollisionPolygon& poly, std::vector<ISpatialObject*>& out, const ISpatialObject* pObj)
{
	std::set<ISpatialObject*> onceFilter;
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
						if(pObj == nullptr || (pObj != nullptr && (&pObj->GetCollisionPolygon()) != (&(*iter)->GetCollisionPolygon())))
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




