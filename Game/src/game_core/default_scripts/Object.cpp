#include "pch.h"
#include "Object.h"

Object::Object()
{
}

const char* Object::GetName() const
{
	return m_name;
}
