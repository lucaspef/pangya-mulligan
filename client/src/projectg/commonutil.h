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

// A929: sem argumentos o texto nao e formato; a conversao de FormatText faz a 2a sobrecarga ganhar nesse caso
struct FormatText { const char* text; FormatText(const char* s) : text(s) {} };
char* __cdecl MakeStr(FormatText format, ...);
char* __cdecl MakeStr(const char* text);
#if _MSC_VER >= 1800
#include "formatcheck.h"
// A930: formato de pak que nao casa com os argumentos sai como texto, nao derruba
template <class A0, class... A> inline char* MakeStr(const char* format, A0 a0, A... rest)
{
	if (FormatFits(format, a0, rest...))
		return MakeStr(FormatText(format), a0, rest...);
	return MakeStr(format);
}
#endif
unsigned long ModifyColor(unsigned long color, float bright);
float Random(float min, float max, int seed = -1);
class WView;
// JP 983 (0098edc0): valor em pixels da tela de 480 de altura escalado pela altura da vista (medida scltype 4 do uiframeset)
float ScaleByViewHeight983(WView* pView, float value);
// JP 983 (0098cd30): inverso do ScaleByViewHeight983 (pixels da tela -> pixels da tela de 480 de altura)
float UnscaleByViewHeight983(WView* pView, float value);
// JP 983 (0098c310): le uma "palavra entre aspas" do script; token maior que dst e cortado, nunca passa do fim
char* GetStrN(char* src, char* dst, size_t dstSize);
const char* GetStrConstN(const char* src, char* dst, size_t dstSize);
template <size_t N> inline char* GetStr(char* src, char (&dst)[N]) { return GetStrN(src, dst, N); }
template <size_t N> inline const char* GetStrConst(const char* src, char (&dst)[N]) { return GetStrConstN(src, dst, N); }
const char* GetCharsTillSpaceN(const char* src, char* dst, size_t dstSize);
template <size_t N> inline const char* GetCharsTillSpace(const char* src, char (&dst)[N]) { return GetCharsTillSpaceN(src, dst, N); }
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
