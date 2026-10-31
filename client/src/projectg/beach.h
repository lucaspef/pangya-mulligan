#pragma once

#include "scenemanager.h"

class CBeach : public CWavelet
{
public:
	CBeach() { }
	virtual ~CBeach() { }

	virtual void Process(float dt);
	virtual void Render();
	virtual void ReadAttribute(const char* name);
};
