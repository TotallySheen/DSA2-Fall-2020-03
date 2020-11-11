#include "MyOctant.h"
using namespace Simplex;
uint MyOctant::m_uIdealEntityCount = 0;
uint MyOctant::m_uMaxLevel = 0;
uint MyOctant::m_uOctantCount = 0;
void MyOctant::Init(void)
{
	m_uID = 0; 
	m_uLevel = 0;
	m_uChildren = 0;

	m_fSize = 0.0f;

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f); 
	m_v3Max = vector3(0.0f);

	m_pParent = nullptr;
	m_pChild[8] = {};

	m_pRoot = nullptr;
}

MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	if (a_nMaxLevel > 0)
	{
		// creating this octant first
		Init();
		m_v3Center = vector3(0);
		m_fSize = 70.0f;
		// creating the max and min
		m_v3Max = vector3(m_fSize / 2);
		m_v3Min = vector3(-m_fSize / 2);
		m_uOctantCount++;
		m_pRoot = this;
		ConstructTree(m_pRoot,a_nMaxLevel);
	}
	AssignIDtoEntity();
	ConstructList();
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uMaxLevel = a_nMaxLevel;
}
MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	Init();
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;
	// creating the max and min
	m_v3Max = vector3(m_fSize);
	m_v3Min = vector3(-m_fSize);
	m_uID = m_uOctantCount;
	m_uOctantCount++;
}
MyOctant::MyOctant(MyOctant const& other)
{
	m_uOctantCount = other.m_uOctantCount;
	m_uMaxLevel = other.m_uMaxLevel;
	m_uIdealEntityCount = other.m_uIdealEntityCount;

	m_uID = other.m_uID; 
	m_uLevel = other.m_uLevel;
	m_uChildren = other.m_uChildren;

	m_fSize = other.m_fSize;

	m_pMeshMngr = other.m_pMeshMngr;
	m_pEntityMngr = other.m_pEntityMngr;

	m_v3Center = other.m_v3Center; 
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;

	m_pParent = other.m_pParent;
	m_pChild[8] = other.m_pChild[8];

	m_EntityList = other.m_EntityList;

	m_pRoot = other.m_pRoot;
	m_lChild = other.m_lChild;
}
MyOctant& MyOctant::operator=(MyOctant const& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyOctant temp(other);
		Swap(temp);
	}
	return *this;
}
MyOctant::~MyOctant(void)
{
	Release();
}
void MyOctant::Swap(MyOctant& other)
{
	std::swap(m_uOctantCount, other.m_uOctantCount);
	std::swap(m_uMaxLevel,other.m_uMaxLevel);
	std::swap(m_uIdealEntityCount,other.m_uIdealEntityCount);

	std::swap(m_uID,other.m_uID);
	std::swap(m_uLevel,other.m_uLevel);
	std::swap(m_uChildren,other.m_uChildren);

	std::swap(m_fSize,other.m_fSize);

	std::swap(m_pMeshMngr,other.m_pMeshMngr);
	std::swap(m_pEntityMngr,other.m_pEntityMngr);

	std::swap(m_v3Center,other.m_v3Center);
	std::swap(m_v3Min,other.m_v3Min);
	std::swap(m_v3Max,other.m_v3Max);

	std::swap(m_pParent,other.m_pParent);
	std::swap(m_pChild[8],other.m_pChild[8]);

	std::swap(m_EntityList,other.m_EntityList);

	std::swap(m_pRoot,other.m_pRoot);
	std::swap(m_lChild,other.m_lChild);
}
float MyOctant::GetSize(void)
{
	return m_fSize;
}
vector3 MyOctant::GetCenterGlobal(void)
{
	return m_v3Center;
}
vector3 MyOctant::GetMinGlobal(void)
{
	return m_v3Min;
}
vector3 MyOctant::GetMaxGlobal(void)
{
	return m_v3Max;
}
bool MyOctant::IsColliding(uint a_uRBIndex)
{
	MyRigidBody* rb = m_pEntityMngr->GetEntity(a_uRBIndex)->GetRigidBody();
	vector3 v3MinO = rb->GetMinGlobal();
	vector3 v3MaxO = rb->GetMaxGlobal();
	//check if boxes are colliding
	bool bColliding = true;
	if (this->m_v3Max.x < v3MinO.x) //this to the right of other
		bColliding = false;
	if (this->m_v3Min.x > v3MaxO.x) //this to the left of other
		bColliding = false;

	if (this->m_v3Max.y < v3MinO.y) //this below of other
		bColliding = false;
	if (this->m_v3Min.y > v3MaxO.y) //this above of other
		bColliding = false;

	if (this->m_v3Max.z < v3MinO.z) //this behind of other
		bColliding = false;
	if (this->m_v3Min.z > v3MaxO.z) //this in front of other
		bColliding = false;

	return bColliding;
}
void MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	m_pChild[a_nIndex]->Display(a_v3Color);
}
void MyOctant::Display(vector3 a_v3Color)
{
	// recursively displaying children
	if (m_uChildren != 0) 
	{
		for (uint i = 0; i < m_uChildren; ++i)
		{
			m_pChild[i]->Display(a_v3Color);
		}
	}
	// not displaying if subdivisions is 0
	if (m_uMaxLevel > 0)
	{
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color);
	}
}
void MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	if (m_uChildren == 0)
	{
		Display(a_v3Color);
	}
	else
	{
		for (uint i = 0; i < m_uChildren; i++)
		{
			m_pChild[i]->DisplayLeafs();
		}
	}
}
void MyOctant::ClearEntityList(void)
{
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->ClearEntityList();
	}
	m_EntityList.clear();
}
void MyOctant::Subdivide(void)
{
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->ConstructTree(m_pRoot,2);
	}
}
MyOctant* MyOctant::GetChild(uint a_nChild)
{
	return m_pChild[a_nChild];
}
MyOctant* MyOctant::GetParent(void)
{
	return m_pParent;
}
bool MyOctant::IsLeaf(void)
{
	return m_uChildren == 0;
}
bool MyOctant::ContainsMoreThan(uint a_nEntities)
{
	return a_nEntities > m_EntityList.size();
}
void MyOctant::KillBranches(void)
{
	// looping through children
	for (uint i = 0; i < m_uChildren; ++i)
	{
		m_pChild[i]->KillBranches();
		m_pChild[i]->Release();
	}
}
void MyOctant::ConstructTree(MyOctant* a_pRoot, uint a_nMaxLevel)
{
	// recursively creating the subidivisons
	if (a_nMaxLevel > 1)
	{
		m_uChildren = 8;
		// creating the 8 children
		m_pChild[0] = new MyOctant(vector3(((m_v3Center.x + m_v3Min.x) / 2), ((m_v3Center.y + m_v3Min.y) / 2), ((m_v3Center.y + m_v3Min.y) / 2)), m_fSize / 2);
		m_pChild[1] = new MyOctant(vector3(-((m_v3Center.x + m_v3Min.x) / 2), ((m_v3Center.y + m_v3Min.y) / 2), ((m_v3Center.y + m_v3Min.y) / 2)), m_fSize / 2);
		m_pChild[2] = new MyOctant(vector3(((m_v3Center.x + m_v3Min.x) / 2), -((m_v3Center.y + m_v3Min.y) / 2), ((m_v3Center.y + m_v3Min.y) / 2)), m_fSize / 2);
		m_pChild[3] = new MyOctant(vector3(-((m_v3Center.x + m_v3Min.x) / 2), -((m_v3Center.y + m_v3Min.y) / 2), ((m_v3Center.y + m_v3Min.y) / 2)), m_fSize / 2);
		m_pChild[4] = new MyOctant(vector3(((m_v3Center.x + m_v3Min.x) / 2), ((m_v3Center.y + m_v3Min.y) / 2), -((m_v3Center.y + m_v3Min.y) / 2)), m_fSize / 2);
		m_pChild[5] = new MyOctant(vector3(-((m_v3Center.x + m_v3Min.x) / 2), ((m_v3Center.y + m_v3Min.y) / 2), -((m_v3Center.y + m_v3Min.y) / 2)), m_fSize / 2);
		m_pChild[6] = new MyOctant(vector3(((m_v3Center.x + m_v3Min.x) / 2), -((m_v3Center.y + m_v3Min.y) / 2), -((m_v3Center.y + m_v3Min.y) / 2)), m_fSize / 2);
		m_pChild[7] = new MyOctant(vector3(-((m_v3Center.x + m_v3Min.x) / 2), -((m_v3Center.y + m_v3Min.y) / 2), -((m_v3Center.y + m_v3Min.y) / 2)), m_fSize / 2);
		// looping through them
		for (uint i = 0; i < m_uChildren; ++i)
		{
			m_pChild[i]->m_pParent = this;
			m_pChild[i]->m_pRoot = a_pRoot;
			m_pChild[i]->ConstructTree(m_pRoot, a_nMaxLevel - 1);
		}
	}
}
void MyOctant::AssignIDtoEntity(void)
{
	// finding the leaves
	for (uint i = 0; i < m_uChildren; ++i)
	{
		m_pChild[i]->AssignIDtoEntity();
	}
	if (m_uChildren == 0)
	{
		// traversing through every entity
		uint entities = m_pEntityMngr->GetEntityCount();
		for (uint index = 0; index < entities; ++index)
		{
			// if the object is in this node
			if (IsColliding(index))
			{
				// registering this node as the dimension for the entity
				m_EntityList.push_back(index);
				m_pEntityMngr->AddDimension(index, m_uID);
			}
		}
	}
}
uint MyOctant::GetOctantCount(void)
{
	return m_uOctantCount;
}

void MyOctant::Release(void)
{
	m_pMeshMngr = nullptr;
	m_pEntityMngr = nullptr;
	m_uOctantCount = 0;
	m_uIdealEntityCount = 0;
	m_uMaxLevel = 0;
	m_uOctantCount = 0;
}
void MyOctant::ConstructList(void)
{
	// recursively looping down to leaves
	for (uint i = 0; i < m_uChildren; ++i)
	{
		m_pChild[i]->ConstructList();
	}
	// if any entities exist, add to the list
	if (m_EntityList.size() > 0)
	{
		m_pRoot->m_lChild.push_back(this);
	}
}