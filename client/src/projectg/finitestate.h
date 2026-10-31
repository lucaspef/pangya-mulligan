#pragma once

#include <map>

class FiniteState
{
	friend class FiniteStateMachine;

private:
	FiniteState(unsigned long stateID);
	virtual ~FiniteState();

	void addTransition(unsigned long input, unsigned long outputID);
	void deleteTransition(unsigned long input);
	unsigned long outputState(unsigned long input);
	unsigned long getCount();

	unsigned long getStateID() { return m_stateID; }

	unsigned long m_stateID;
	std::map<unsigned long, unsigned long> m_transitionMap;
};
