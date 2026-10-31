#pragma once

#include "actor.h"

class CMagicCarpetHelper : public IActor
{
public:
	virtual ~CMagicCarpetHelper();

protected:
	DECLARE_OBJECT(CMagicCarpetHelper)

public:
	virtual void OnLoad();
	virtual void OnInit();
	virtual void OnProcess(float delta);
	virtual void OnDisplay();
	virtual void HandleMsg(const MsgObject& msg);

protected:
	CMagicCarpetHelper();

private:
	void AutoVolumeAdjustProcess(const float& delta);
	void PlaySfxMagicCarpetBoundSound();

	bool m_bVolumeDown;
	bool m_bAutoVolume;
};
