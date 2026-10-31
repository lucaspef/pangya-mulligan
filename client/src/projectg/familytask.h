#pragma once
#include "taskmanager.h"

class CFamilyTask : public CTask
{
	DECLARE_OBJECT(CFamilyTask)

protected:
	CFamilyTask();

public:
	virtual ~CFamilyTask();

protected:
	virtual void Register();
	virtual void Init(const char* wallPaper);
	virtual void Load();
	virtual void Process(float delta);
	virtual void Display();
	virtual int OnPacket(WReceivedPacket& packet);
};
