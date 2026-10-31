#pragma once
#include "frform.h"

class FrReadyToAll : public FrForm
{
public:
	DECLARE_OBJECT(FrReadyToAll)

	FrReadyToAll();
	virtual ~FrReadyToAll();

protected:
	void OnReadyBtnUp();
	void OnWaitBtnUp();

	DECLARE_FRESH_MSGMAP()
};

class FrReadyToMaster : public FrForm
{
public:
	DECLARE_OBJECT(FrReadyToMaster)

	FrReadyToMaster();
	virtual ~FrReadyToMaster();

protected:
	virtual void OnProc(const float dt);

	void OnStartBtnUp();
	void OnExitBtnUp();

	DECLARE_FRESH_MSGMAP()
};
