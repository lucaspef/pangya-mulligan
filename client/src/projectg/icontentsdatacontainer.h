#pragma once

// In the PCH (shareddoc.h): the original emits these inlines after a unit's own templates.
struct IContentsDataContainer
{
	IContentsDataContainer() { }
	virtual ~IContentsDataContainer() { }

	virtual bool Initialize() = 0;
	virtual bool Release() = 0;
	virtual bool Reset() { return true; }
};
