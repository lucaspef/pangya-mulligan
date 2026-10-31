#pragma once
#include "taskmanager.h"

class WReceivedPacket;

class CTutorialTask : public CTask
{
	DECLARE_OBJECT(CTutorialTask)
protected:
	CTutorialTask();

public:
	virtual ~CTutorialTask();

protected:
	virtual void Init(const char* wallPaper);
	virtual void Register();
	virtual void Load();
	virtual void PreserveBack(const char* name);
	virtual void RestorePreserved();
	virtual void Destroy();
	virtual void Process(float delta);
	virtual void Display();
	virtual int OnPacket(WReceivedPacket& packet);
};
