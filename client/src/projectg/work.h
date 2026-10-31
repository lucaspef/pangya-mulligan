#pragma once

class CWork
{
public:
#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
	virtual ~CWork();
#else
	virtual ~CWork() { }
#endif
	virtual void Act() = 0;
};
