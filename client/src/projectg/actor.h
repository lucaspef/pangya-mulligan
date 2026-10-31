#pragma once
#include "msg983.h"

#include <string>
#include <vector>
#include <map>

class IActor;
class CTask;
class CTaskDoc;

#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
#include "msgobject.h"
#else
#ifndef MSGSEL983_DEFINED
#define MSGSEL983_DEFINED
template<class T> struct MsgSel983 { };
#endif
class MsgObject
{
public:
	MsgObject() { }
	MsgObject(const IActor* _sender, int _message, int _param1, int _param2,
		int _param3, int _param4, int _param5)
		: sender(_sender),
		  message(_message),
		  param1(_param1),
		  param2(_param2),
		  param3(_param3),
		  param4(_param4),
		  param5(_param5)
	{
		time = 0;
	}

	// JP 983: leitores de parametro por tipo, fora de linha (004bb1d0...004bb300 chamam 004bad80...004baee0 com um seletor
	// vazio de 1 byte na pilha) - pedido B14/B15. Nomes nossos.
	template<class T> T Param2() const;
	template<class T> T Param2(MsgSel983<T>) const;
	template<class T> T Param3() const;
	template<class T> T Param3(MsgSel983<T>) const;
	template<class T> T Param4() const;
	template<class T> T Param4(MsgSel983<T>) const;

	const IActor* sender;
	int message;
	int param1;
	int param2;
	int param3;
	int param4;
	int param5;
	unsigned long time;
};
template<class T> T MsgObject::Param2() const { return Param2(MsgSel983<T>()); }
template<class T> T MsgObject::Param2(MsgSel983<T>) const { return (T)param2; }
template<class T> T MsgObject::Param3() const { return Param3(MsgSel983<T>()); }
template<class T> T MsgObject::Param3(MsgSel983<T>) const { return (T)param3; }
template<class T> T MsgObject::Param4() const { return Param4(MsgSel983<T>()); }
template<class T> T MsgObject::Param4(MsgSel983<T>) const { return (T)param4; }
#endif

struct sActor
{
	IActor* actor;
	bool flag;
	const char* name;
};

// HACK: instantiating allocator<const char*> ahead of CActorFactory matches the
// original template emission order in actor.cpp
typedef std::allocator<const char*>::size_type ActorClassSize;

class CActorFactory
{
public:
	CActorFactory();
	~CActorFactory();

	void AddActorClass(const char* className);
	IActor* CreateActor(const char* className);

private:
	std::map<const std::string, std::string> m_actorTypeList;
	std::vector<const char*> m_actorClassList;
};

CActorFactory& ActorFactory();

// 983: segunda fábrica, só do JP (estático 00f00774, criado em 00a49100): nome da classe -> objeto já criado. Os
// documentos do JP guardam o handler aqui (Add 00a49000 no registro da classe) e o pegam pelo nome (Get 00a48210).
// Não é o CActorFactory (00a471a0); nomes da classe e dos métodos são meus (o 983 não tem RTTI dela).
class CActorRegistry983
{
public:
	CActorRegistry983(); // 00a48f90
	~CActorRegistry983(); // 00a48da0

	IObject* Find(const char* className); // 00a47fe0: o objeto ou NULL
	bool Exists(const char* className); // 00a480e0
	IObject* Get(const char* className); // 00a48210 (repassa ao Find)
	bool Remove(const char* className); // 00a48e20: apaga o objeto e a entrada (sem chamador no 983)
	bool Add(const char* className, IObject* pObject); // 00a49000: falso se o nome já existe

private:
	std::map<std::string, IObject*> m_objects;
};

CActorRegistry983& ActorRegistry983(); // 00a49100

// HACK: instantiates map::operator[] in the PCH so that actor.cpp emits the
// _Tree helpers in the original order; never called
inline void __ActorPchFill(std::map<const std::string, std::string>& m,
	const std::string& s)
{
	m[s];
}

#define IMPLEMENT_ACTOR(className, baseClass) \
	IObject* className##MakeInstance() \
	{ \
		return new className; \
	} \
	struct __imp##className \
	{ \
		__imp##className() \
		{ \
			ObjectFactory().AddObjectFunctor(className##MakeInstance, \
				#className); \
			ActorFactory().AddActorClass(#className); \
		} \
	}; \
	const WRTTI className::m_RTTI(#className, &baseClass::m_RTTI); \
	static __imp##className __impl##className;

class IActor : public IObject
{
	friend class CTask;
	friend IActor* operator<<(IActor* pActor, const MsgObject& msg);

public:
	static const WRTTI m_RTTI;
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; }

	IActor();
	virtual ~IActor();

	int SendMsg(const IActor* sender, const char* target, int message,
		int param1, int param2, int param3, unsigned long time);
	int PostMsg(const IActor* sender, const char* target, int message,
		int param1, int param2, int param3, unsigned long time);
	void AddActor(const char* className, const char* name, bool bSkip,
		const std::string& option);
	IActor* GetActor(const char* name) const;

	static unsigned long m_defaultSkipLayer;
	static unsigned long* m_pLayer;

	virtual void OnPreLoadInit() { }
	virtual void OnLoad() { }
	virtual void OnInit() { }
	virtual void OnReload() { }
	virtual void OnProcess(float delta);
	virtual void OnDisplay() { }
	virtual void OnDestroy() { }
	virtual void OnPreserveBack(const char* name) { }
	virtual void OnRestorePreserved() { }

protected:
	virtual void HandleMsg(const MsgObject& msg);
	virtual void StoreDelayedMsg(const MsgObject& msg) const;

	enum eDispPriority
	{
		DISP_PRIORITY_0,
		DISP_PRIORITY_1,
		DISP_PRIORITY_2,
		DISP_PRIORITY_3,
		DISP_PRIORITY_4,
		DISP_PRIORITY_5
	};

	virtual eDispPriority GetDisplayPriority() { return m_dispPriority; }
	virtual bool Skipped() const;

	CTaskDoc* GetDocument() const;
	CTask* GetTask() const;
	void AddSkipList(unsigned long layer);
	void RemoveSkipList(unsigned long layer);

	WFlags m_flag;
	eDispPriority m_dispPriority;
	unsigned long m_skipLayer;
	CTask* m_pTask;
	CTaskDoc* m_pDoc;
};

inline IActor* operator<<(IActor* pActor, const MsgObject& msg)
{
	if (pActor == NULL)
		return NULL;
	pActor->HandleMsg(msg);
	return pActor;
}
