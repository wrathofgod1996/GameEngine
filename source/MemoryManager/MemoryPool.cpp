/*
	Programmed by Bryce Mehring
	1/24/2011
*/

#include "MemoryPool.h"
#include "Heap.h"
#include <cassert>
#include <fstream>

using namespace std;


MemoryPool::MemoryPool(unsigned int size, unsigned int n) : m_pNode(nullptr),
	m_pNodeHead(nullptr), m_pNodeTail(nullptr), m_iSize(size+sizeof(Node*)), m_iLength(n), m_iBlocks(0)
{
	// Create first memory block
	LinkMemoryBlock(AllocMemoryBlock());
}

MemoryPool::~MemoryPool()
{
	// free all pools
	FreePool();
}

void MemoryPool::FreePool()
{
	// Iterate over all memory blocks and delete them
	Node* pIter = m_pNodeHead;

	while(pIter != nullptr)
	{
		Node* pTemp = pIter;
		pIter = pIter->pNext;

		::operator delete(pTemp);
	}

	m_pNode = m_pNodeHead = m_pNodeTail = nullptr;
	m_iBlocks = 0;
}

void* MemoryPool::Allocate()
{
	CheckNextNodeForMemory();
	
	// Acquire the pointer to the memory block
	void* pMem = reinterpret_cast<char*>(m_pNode->pMemoryBlock) + sizeof(Node*);

	// iterate to the next memory block
	m_pNode->pMemoryBlock = m_pNode->pMemoryBlock->pNext;
	m_pNode->iSize--;

	// return the memory
	return pMem;
}

void MemoryPool::Deallocate(void* pMem)
{
	if(pMem)
	{
		MemoryBlock* pBlock = reinterpret_cast<MemoryBlock*>(static_cast<char*>(pMem) - sizeof(Node*));
		Node* pNode = pBlock->pNode;

		// fix up the linked list, add pBlock to the front of the list
		pBlock->pNext = pNode->pMemoryBlock;
		pNode->pMemoryBlock = pBlock;
		pNode->iSize++;

		if(pNode->iSize > (m_iLength / 3))
		{
			m_pNode = pNode;
		}

		//m_pNode = pNode;
		/*}
		else
		{
			// sending error info to the output window in the compiler

#ifdef _DEBUG
			char buffer[128];
			sprintf_s(buffer,"\nMemory was not allocated from the memory pool: 0x%p\n\n",pMem);
			
			OutputDebugString(buffer);
#endif
		}*/
	}
}

char* MemoryPool::AllocMemoryBlock()
{
	// +sizeof(Node) is to allow room for the pointers to link the list
	unsigned int iBlockArraySize = BlockArraySize() + sizeof(Node);

	// Alloc memory from the system
	char* pMem = static_cast<char*>(::operator new(iBlockArraySize));

	//memset(pMem,0,iBlockArraySize);

	++m_iBlocks;

	// return memory
	return pMem;
}

void MemoryPool::LinkMemoryBlock(char* pMem)
{
	// ===== link the list =====

	// link to the beginning
	if(m_pNodeHead)
	{
		m_pNodeHead->pPrevious = reinterpret_cast<Node*>(pMem);
		m_pNodeHead->pPrevious->pNext = m_pNodeHead;
		m_pNodeHead = m_pNodeHead->pPrevious;
	}
	else
	{
		m_pNodeHead = m_pNodeTail = reinterpret_cast<Node*>(pMem);
		m_pNodeHead->pNext = nullptr;
	}

	m_pNode = m_pNodeHead;
	m_pNode->pPrevious = nullptr;

	// ===== link the memory block =====

	// ignore the header info in each node
	pMem += sizeof(Node);

	// m_pNode->pMemoryBlock is the first node in the list
	m_pNode->pPool = this;
	m_pNode->pMemoryBlock = reinterpret_cast<MemoryBlock*>(pMem);
	m_pNode->iSize = m_iLength;

	// iterator that iterates over the memory block, pMem
	MemoryBlock* pIter = m_pNode->pMemoryBlock;

	// Loop m_iLength - 1 times, since the first node in the list is
	// m_pNode->pMemoryBlock
	for(unsigned int i = 1; i < m_iLength; ++i)
	{
		pMem += m_iSize;

		// add the next part of the buffer
		pIter->pNext = reinterpret_cast<MemoryBlock*>(pMem); // next pointer

		pIter->pNode = m_pNode;

		// increment the iterator
		pIter = pIter->pNext;
	}

	//x todo: need to check this, this might be incorrect
	// this is the end of the linked list
	pIter->pNode = m_pNode; // this was the problem:
	pIter->pNext = nullptr;
	
}

unsigned int MemoryPool::BlockArraySize() const
{
	// does this make sense
	return (m_iSize * m_iLength);
}

bool MemoryPool::FreeSpace(Node* pNode) const
{
	// memory is free while the pointer is still valid
	return (pNode != nullptr) && (pNode->pMemoryBlock != nullptr);
}

unsigned int MemoryPool::GetBlocks() const
{
	return m_iBlocks;
}

unsigned int MemoryPool::GetSize() const
{
	return m_iSize;
}

unsigned int MemoryPool::GetLength() const
{
	return m_iLength;
}

unsigned int MemoryPool::GetMemoryUsage() const
{
	return m_iBlocks*(m_iLength*m_iSize+sizeof(Node));
}

// I need to comment more about this function
void MemoryPool::Move(Node* pTo, Node* pNode)
{
	Erase(pNode);
	Insert(pTo,pNode);
}

// I need to comment more about this function
void MemoryPool::Erase(Node* pNode)
{
	if(pNode->pPrevious == nullptr)
	{
		m_pNodeHead = m_pNodeHead->pNext;
	}
	else
	{
		pNode->pPrevious->pNext = pNode->pNext;
	}

	if(pNode->pNext == nullptr)
	{
		m_pNodeTail = m_pNodeTail->pPrevious;
	}
	else
	{
		pNode->pNext->pPrevious = pNode->pPrevious;
	}
}

// I need to comment more about this function
// this function inserts pNode before pWhere
void MemoryPool::Insert(Node* pWhere, Node* pNode)
{
	pNode->pPrevious = pWhere->pPrevious;
	pNode->pNext = pWhere;

	if(pWhere->pPrevious == nullptr)
	{
		// insert in the front
		pWhere->pPrevious = pNode;

		m_pNodeHead = m_pNodeHead->pPrevious; // Set m_pFirst to the new node
	}
	else
	{
		pWhere->pPrevious->pNext = pNode;
		pWhere->pPrevious = pNode;
	}
}

// I need to comment this function
// This method defrags the memory blocks
// with insertion sort
unsigned int MemoryPool::SortBlocks()
{
	unsigned int count = 0; // counts the number of nodes that were out of order
	Node* pIter = m_pNodeHead;

	// Loop n - 1 nodes
	while(pIter->pNext != nullptr)
	{
		// Save next pointer, because it could get erased
		Node* pNext = pIter->pNext;
		
		// If the nodes are out of order
		if((pNext->iSize > pIter->iSize))
		{
			// Find the new node to insert after
			Node* pPrevIter = pIter;
			while((pPrevIter != nullptr) && !(pPrevIter->iSize > pNext->iSize))
			{
				pPrevIter = pPrevIter->pPrevious;
			}

			if(pPrevIter == nullptr)
			{
				pPrevIter = m_pNodeHead;
			}

			// Move the node to the new location in the list
			Move(pPrevIter,pNext);

			count++;
		}

		// Increment list
		pIter = pNext;
	}

	return count;
}

void MemoryPool::Compact()
{
	// Iterate over all memory blocks and delete them, if they are full, but leaves one free block
	Node* pIter = m_pNodeHead;

	while(pIter != nullptr)
	{
		Node* pPrevious = pIter;
		pIter = pIter->pNext;

		if(pPrevious->iSize == m_iLength && pPrevious != m_pNodeHead)
		{
			pPrevious->pPrevious->pNext = pIter;

			if(pIter != nullptr)
			{
				pIter->pPrevious = pPrevious->pPrevious;
			}

			operator delete(pPrevious);

			m_iBlocks--;
		}
	}
}

void MemoryPool::CheckNextNodeForMemory()
{
	// If the current node is full
	if(FreeSpace(m_pNode) == false)
	{
		if(FreeSpace(m_pNode->pNext))
		{
			m_pNode = m_pNode->pNext;
		}
		// V this fixed the memory leak V
		else if(FreeSpace(m_pNode->pPrevious))
		{
			m_pNode = m_pNode->pPrevious;
		}
		else
		{
			// add a new block to the list
			LinkMemoryBlock(AllocMemoryBlock());
		}
	}
}

void MemoryPool::MemoryDump() const
{
	fstream outfile("memory_dump.txt",ios::out | ios::binary);
	unsigned int iBlockArraySize = BlockArraySize() + sizeof(Node);

	if(outfile)
	{
		const char* newLine = "\n";
		
		for(Node* pIter = m_pNodeHead; pIter != nullptr; pIter = pIter->pNext)
		{
			outfile.write(newLine,sizeof(newLine));
			outfile.write((char*)pIter,iBlockArraySize);
		}

		outfile.close();
		//system("memory_dump.txt");
	}
}
