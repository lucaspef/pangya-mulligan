#pragma once

#include "taskmanager.h"

class WReceivedPacket;

class CLobbyTask : public CTask
{
	DECLARE_OBJECT(CLobbyTask)

protected:
	CLobbyTask();

public:
	virtual ~CLobbyTask();

protected:
	virtual void Init(const char* wallPaper);
	virtual void Register();
	virtual void Load();
	virtual void Process(float delta);
	virtual void Display();
	virtual int OnPacket(WReceivedPacket& packet);
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnpacketFunclets(); // HACK: EH funclets of the naked OnPacket (nakedify)
	void RbOnPacketBody(WReceivedPacket&
			packet); // HACK: OnPacket from +0x30 on + its switch tables
#endif
};
