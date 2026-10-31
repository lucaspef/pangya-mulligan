#pragma once
#undef NDEBUG	// a Refresh foi compilada sem NDEBUG (_wassert no 983)
#include <assert.h>
#include <string>
#include <vector>
#include <hash_map>
#include "refresh/lib/rfapi.h"

// JP 983: RFObject.h e RFEventHandler.h da Refresh (00401540–00401bb0; vtables RFObject 00d63048, RFEventHandler 00d63054,
// EventFunctorBase 00cfe638). Nomes pela exportação do 983; `_impl` pelo texto do assert; o resto dos campos é meu.

// códigos de erro da RF (SendEvent, RFDirector::Play…; valores ainda não levantados além do 0)
enum R_ERRORTYPE
{
	R_FAIL983 = 0,
	R_OK983 = 1, // RFEventHandler::SendEvent: o evento existe
	R_NOTFOUND983 = 7, // RFEventHandler::SendEvent: evento sem nome registrado
};

class RF_API RFObject
{
public:
	RFObject() { } // 00401560
	virtual const wchar_t* ToString() const { return L"RFObject"; } // 00401540 (slot 0)
	virtual ~RFObject() { } // 00401550
};

// argumentos de evento (não exportada; vtable 00d113c8, destrutor de apagar 00401640)
class EventArgs
{
public:
	EventArgs() : handled(false) { }
	virtual ~EventArgs() { }

	bool handled; // +0x4
};

// argumentos com a janela que gerou o evento (vtable 00d63068 para RFWnd; destrutor de apagar 00409ef0)
template<class T> class WindowEventArgs_ : public EventArgs
{
public:
	WindowEventArgs_(T* wnd) : window(wnd) { }

	T* window; // +0x8
};

// functor de evento com contagem de referências (exportada; vtable 00cfe638)
class RF_API EventFunctorBase
{
public:
	EventFunctorBase() { } // 00401680
	virtual ~EventFunctorBase() { } // 00401670 (slot 0)
	virtual bool operator()(EventArgs& e) = 0; // slot 1
	virtual void AddRef() = 0; // slot 2
	virtual void Release() = 0; // slot 3
	virtual int GetRefCount() = 0; // slot 4
};

// functor de método (0x10; MemberFunctionFunctor<RFWnd> 0040fc00–0040fc50, vtable 00d63104)
template<class T> class MemberFunctionFunctor : public EventFunctorBase
{
public:
	typedef void (T::*Function)(EventArgs& e);

	MemberFunctionFunctor(Function fn, T* obj) : _function(fn), _object(obj), _refCount(0) { }
	virtual bool operator()(EventArgs& e) { (_object->*_function)(e); return true; }
	virtual void AddRef() { ++_refCount; }
	virtual void Release() { --_refCount; }
	virtual int GetRefCount() { return _refCount; }

	Function _function; // +0x4
	T* _object; // +0x8
	int _refCount; // +0xc
};

// assinante: um functor com uma referência
class RF_API Subscriber
{
public:
	Subscriber() : _impl(0) { } // 00401740
	template<class T> Subscriber(void (T::*fn)(EventArgs& e), T* obj)
		: _impl(new MemberFunctionFunctor<T>(fn, obj))
	{
		_impl->AddRef();
	}
#line 145 RF_INCLUDE_RFEVENTHANDLER_H983
	Subscriber(const Subscriber& subscriber) // 00401750
	{
		assert(subscriber._impl);
		_impl = subscriber._impl;
		_impl->AddRef();
	}
	~Subscriber() // 00401790
	{
		if (_impl)
		{
			_impl->Release();
			if (_impl->GetRefCount() == 0 && _impl)
				delete _impl;
		}
	}
	bool operator()(EventArgs& e) { return (*_impl)(e); } // 004017c0

private:
	EventFunctorBase* _impl;
};

// evento com nome: a lista de assinantes (0x2c)
class RF_API RFEvent
{
public:
	RFEvent(const std::wstring& name) : _name(name) { } // 004017e0
	~RFEvent() { Cleanup(); } // 00401820

	void Cleanup() { _subscribers.clear(); } // 004018f0
	void Subscribe(const Subscriber& subscriber) { _subscribers.push_back(subscriber); } // 00401930
	void ClearSubscribers() { _subscribers.clear(); } // 00401950
	bool operator()(EventArgs& e) // 00401990
	{
		bool handled = false;
		for (unsigned int i = 0; i < _subscribers.size(); i++)
			handled |= _subscribers[i](e);
		return handled;
	}

	std::wstring _name; // +0x00
	std::vector<Subscriber> _subscribers; // +0x1c
};

// objeto com eventos por nome (0x30: vtable + hash_map)
class RF_API RFEventHandler : public RFObject
{
public:
	RFEventHandler(); // 0042f5a0
	virtual const wchar_t* ToString() const { return L"RFEventHandler"; } // 00401a90
	virtual ~RFEventHandler(); // 0042f600

	void AddEvent(const std::wstring& name); // 0042f760
	void RemoveEvent(const std::wstring& name); // 0042f9b0
	bool IsEventPresent(const std::wstring& name) const; // 0042fa20
	void SubscribeEvent(const std::wstring& name, const Subscriber& subscriber); // 0042fa50
	void ClearEventSubscribers(const std::wstring& name); // 0042faa0
	R_ERRORTYPE SendEvent(const std::wstring& name, EventArgs& e); // 0042fb00

	typedef stdext::hash_map<std::wstring, RFEvent> EventMap;
	EventMap _events; // +0x04
};
