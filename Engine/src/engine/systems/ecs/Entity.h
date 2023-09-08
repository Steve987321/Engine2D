#pragma once
#include <set>

namespace Toad
{

class Entity
{
public:
	Entity(uint32_t id);
	~Entity();
public:
	uint32_t GetID() const;

	virtual void Update() {};
	//template<class T>
	//T* GetComponent()
	//{
	//	// find entity in register
	//	if ()

	//	// find component in entity register array

	//	// return 
	//}

public:
	uint32_t operator++()
	{
		return ++m_id;
	}

private:
	uint32_t m_id;

	static std::set<Entity*> m_entities;

};

}
