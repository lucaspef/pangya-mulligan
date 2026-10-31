#pragma once
#include <map>
#include <string>
#include "rtti.h"
class IObject
{
public:
	static const WRTTI m_RTTI;
	// HACK: inline { return &m_RTTI; } in the original (COMDAT kept in taskmanager.obj); a body here
	// shifts PCH IL (frwndmanager order), so only the declaration until taskmanager is converted
	virtual const WRTTI* GetRTTI() const;
	bool IsExactKindOf(const WRTTI* pRTTI) const { return GetRTTI() == pRTTI; }
	bool IsKindOf(const WRTTI* pRTTI) const
	{
		const WRTTI* pBase = GetRTTI();
		while (pBase)
		{
			if (pBase == pRTTI)
				return true;
			pBase = pBase->GetBaseRTTI();
		}
		return false;
	}
	void* DynamicCast(const WRTTI* pRTTI) { return IsKindOf(pRTTI) ? this : 0; }
	virtual ~IObject();

protected:
	IObject();
};
#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h

// HACK: pure in the class (shared with Fresh), but the original has a body (taskmanager.obj emits it
// for the CTaskDoc vtable)
inline const WRTTI* IObject::GetRTTI() const
{
	return &m_RTTI;
}
#endif

class CObjectFactory
{
public:
	typedef IObject* (*ObjectFunctor)();
#ifdef ILFILL0 // HACK: full class only in the ProjectG PCH (Fresh COMDAT order)
	typedef std::map<const std::string, ObjectFunctor> ObjectFuncList;
	typedef std::map<const std::string, IObject*> ObjectTypeList;
	CObjectFactory() { }
	~CObjectFactory() { Reset(); }
	void AddObjectType(IObject* object, const char* name);
	void Reset();
#endif
	void AddObjectFunctor(ObjectFunctor function, const char* name);
#ifndef ILFILL0
	IObject* CreateObject(const char* name);
#else
	IObject* CreateObject(const char* name)
	{
		ObjectFuncList::iterator it = m_objectFuncList.find(name);
		if (it != m_objectFuncList.end())
			return (*it).second();
		return 0;
	}
#endif
#ifdef ILFILL0

private:
	ObjectTypeList m_objectTypeList;
	ObjectFuncList m_objectFuncList;
	// HACK: complete the functor map iterators first (COMDAT order)
	typedef char ObjectFuncIterSize[sizeof(ObjectFuncList::iterator)];
#endif
};

CObjectFactory& ObjectFactory();

#define DECLARE_OBJECT(className) \
public: \
	static const WRTTI m_RTTI; \
	virtual const WRTTI* GetRTTI() const \
	{ \
		return &m_RTTI; \
	} \
	friend IObject* className##MakeInstance();

#define IMPLEMENT_OBJECT(className, baseClass) \
	IObject* className##MakeInstance() \
	{ \
		return new className; \
	} \
	struct __s##className \
	{ \
		__s##className() \
		{ \
			ObjectFactory().AddObjectFunctor(className##MakeInstance, \
				#className); \
		} \
	}; \
	const WRTTI className::m_RTTI(#className, &baseClass::m_RTTI); \
	static __s##className __impl##className;
