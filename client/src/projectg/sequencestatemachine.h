#pragma once

#include <list>
#include <string>

class CSequenceStateMachine;

class CStateUnit
{
	friend class CSequenceStateMachine;

public:
	virtual const char* GetClassNameA() const = 0;
	virtual ~CStateUnit() { }

private:
	virtual void OnProcess(float delta) = 0;

public:
	virtual void OnRender() = 0;
	bool operator<(const CStateUnit* pUnit) const
	{
		return m_order < pUnit->m_order;
	}

protected:
	CStateUnit()
		: m_order(0)
	{
	}

	void OnPrintLog(const char* log);
	void SetOwner(CSequenceStateMachine* pOwner) { m_pOwner = pOwner; }

private:
	virtual bool OnFinished() = 0;

	CSequenceStateMachine* m_pOwner;
	int m_order;
};

class CSequenceStateMachine
{
public:
	CSequenceStateMachine(const void* pOwner, const char* name);
	~CSequenceStateMachine();

	void PrintLog(const CStateUnit* pUnit, const char* log);

	bool Process(float delta);
	void Render();

	void Clear();
	void Reset();

	void Insert(int order, CStateUnit* pUnit);
	void BackInsert(CStateUnit* pUnit);

	CStateUnit* GetCurrentState();
	CStateUnit* GetPreviousState();
	CStateUnit* GetNextState();
	bool SkipCurrentState();

private:
	void PrintLog(const char* format, ...);

	std::list<CStateUnit*> m_states;
	std::list<CStateUnit*>::iterator m_current;
	int m_lastOrder;
	const void* m_pOwner;
	std::string m_name;
	// 983: ctor 0082d280 zera +0x34/+0x38; unidade e dono (CGhostPlayer +0x30) nao leem
	unsigned long m_pad983_34[2];
};
