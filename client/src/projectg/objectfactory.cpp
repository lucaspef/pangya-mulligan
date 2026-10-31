#include "minatl.h"
#include "objectfactory.h"

const WRTTI IObject::m_RTTI("IObject", 0);

IObject::IObject()
{
}

IObject::~IObject()
{
}

// VC10: sem as especializações do construtor de std::pair do VC7.1

CObjectFactory& ObjectFactory()
{
	static CObjectFactory s_factory;
	return s_factory;
}

void CObjectFactory::Reset()
{
	for (ObjectTypeList::iterator it = m_objectTypeList.begin();
		it != m_objectTypeList.end(); ++it)
	{
		if ((*it).second)
			delete (*it).second;
	}
	m_objectTypeList.erase(m_objectTypeList.begin(), m_objectTypeList.end());
	m_objectFuncList.erase(m_objectFuncList.begin(), m_objectFuncList.end());
}

void CObjectFactory::AddObjectType(IObject* object, const char* name)
{
	m_objectTypeList[name] = object;
}

void CObjectFactory::AddObjectFunctor(ObjectFunctor function, const char* name)
{
	m_objectFuncList[name] = function;
}
