#pragma once

#include <map>
#include "finitestate.h"

class FiniteStateMachine
{
public:
	FiniteStateMachine();
	virtual ~FiniteStateMachine();

	void addStateTransition(unsigned long stateID, unsigned long input,
		unsigned long outputID);
	void deleteTransition(unsigned long stateID, unsigned long input);
	unsigned long getOutputState(unsigned long input);
	void setCurrentState(unsigned long stateID);
	unsigned long getCurrentStateID();
	void stateTransition(int input);

private:
	std::map<unsigned long, FiniteState*> m_stateMap;
	FiniteState* m_pCurrentState;
};
