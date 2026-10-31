#pragma once
#include <map>
#include "controller.h"
#include "conditionalauxpartobject.h"

// JP 983: RTTI .?AVCConditionalAuxpartController@@, vtable 00d0229c, 0x30 bytes (operator new em 004b0d60).
// Dois mapas por typeid da peça: a condição (devolve o `kind` do efeito ou -1) e a ação do slot 9.
class CConditionalAuxpartController : public IController
{
	DECLARE_OBJECT(CConditionalAuxpartController)
public:
	typedef std::tr1::function<int(int)> Condition;
	typedef std::tr1::function<void(CConditionalAuxpartObject*, int)> Action;

	CConditionalAuxpartController(); // 004b09f0
	virtual ~CConditionalAuxpartController(); // 004b0c80
	virtual bool On02(); // 004ae230
	virtual bool On03(); // 004ae240
	virtual bool On04(); // 004adaa0
	virtual bool On05(); // 004af2e0
	virtual bool On06(float arg); // 004ae250
	virtual bool On07(int arg); // 004af320
	virtual bool On09(); // 004af090
	virtual bool On10(int arg); // 004af360

	CConditionalAuxpartObject* GetControlled(); // 004ad4d0
	void Check(CConditionalAuxpartObject* pObject, int playerIndex); // 004af110

private:
	std::map<unsigned long, Condition> m_conditions; // +0x10
	std::map<unsigned long, Action> m_actions; // +0x20
};
