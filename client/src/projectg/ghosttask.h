#pragma once
class CGhostTask : public CTask
{
	DECLARE_OBJECT(CGhostTask)

public:
	virtual ~CGhostTask() { }

protected:
	CGhostTask();

	virtual void Register();
	virtual void Init(const char* wallPaper);
	virtual void Process(float delta);
	virtual void Display();
	virtual int OnPacket(WReceivedPacket& packet);
	virtual void Load();
};
