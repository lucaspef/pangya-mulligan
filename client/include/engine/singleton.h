#pragma once

#include "baseobject.h"

template <class T>
class WSingleton : public BaseObject
{
public:
	WSingleton() { m_pInstance = static_cast<T*>(this); }
	WSingleton(const WSingleton& obj);
	virtual ~WSingleton() { m_pInstance = NULL; }

	static T* Instance() { return m_pInstance; }
	static bool IsInstantiated() { return m_pInstance ? true : false; }

protected:
	static T* m_pInstance;
};

template <class T>
T* WSingleton<T>::m_pInstance = NULL;
