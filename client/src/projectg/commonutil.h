#pragma once

#include <string>
#include "singleton.h"
#ifdef _ATL_STATIC_REGISTRY // HACK: ProjectG PCH only (Fresh includes this header through frelement.h; its COMDAT order must not see these)
#include <stack>
#include "exception.h"
#endif

class cFile;
class WResourceManager;

namespace std
{
	template <class _Ty, class _Container>
	class stack;
}

char* __cdecl MakeStr(const char* format, ...);
unsigned long ModifyColor(unsigned long color, float bright);
float Random(float min, float max, int seed = -1);
class WView;
// JP 983 (0098edc0): valor em pixels da tela de 480 de altura escalado pela altura da vista (medida scltype 4 do uiframeset)
float ScaleByViewHeight983(WView* pView, float value);
// JP 983 (0098cd30): inverso do ScaleByViewHeight983 (pixels da tela -> pixels da tela de 480 de altura)
float UnscaleByViewHeight983(WView* pView, float value);
unsigned long htoi(const char* str);
bool str_Replace(std::string& str, const std::string& from,
	const std::string& to);

template <class T>
void sequence_delete(T first, T last)
{
	while (first != last)
		delete *first++;
}

class WFile
{
public:
	WFile();
	WFile(const char* filename);
	virtual ~WFile();

	cFile* Open(const char* filename, unsigned int option);
	virtual void Close();
	virtual int Read(void* buffer, int size);
	virtual int GetByte();
	virtual int Tell();
	virtual void Seek(int offset, int origin);
	virtual int Length();

	static void SetResourceManager(WResourceManager* pResMng);

protected:
	const char* m_filename;
	cFile* m_pFile;

	static WResourceManager* m_pResMng;
};

template <class T>
class CStackableSingleton : public WSingleton<T>
{
public:
	virtual ~CStackableSingleton() { }

#ifdef _ATL_STATIC_REGISTRY
	static void PushInstance();
	static void PopInstance();

	static std::stack<T*, std::list<T*> > m_InstanceStack;
#endif

protected:
	CStackableSingleton() { }
};

#ifdef _ATL_STATIC_REGISTRY
template <class T>
void CStackableSingleton<T>::PushInstance()
{
	m_InstanceStack.push(WSingleton<T>::m_pInstance);
	WSingleton<T>::m_pInstance = NULL;
}

template <class T>
void CStackableSingleton<T>::PopInstance()
{
	if (m_InstanceStack.empty())
	{
		WSingleton<T>::m_pInstance = NULL;
	}
	else
	{
		WSingleton<T>::m_pInstance = m_InstanceStack.top();
		m_InstanceStack.pop();
	}
}
#endif
