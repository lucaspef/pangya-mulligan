#pragma once
#include "../../include/engine/wmath.h"
#include "../../include/engine/wlist.h"

class WMilesSoundSystem;
class WView;

class CSoundObject
{
public:
	WVector GetPos() const { return m_pos; }

protected:
	WVector m_pos;
};

struct _SFX_INFO
{
	char name[64];
	char alias[32];
	int handle;
	unsigned long tick;
	WVector pos;
	bool b3D;
	bool bPlayed;
	bool bActive;
	unsigned long flags;
	const CSoundObject* pObject;

	bool IsSet(unsigned long flag) { return (flags & flag) != 0; }
};

class CSoundManager
{
public:
	CSoundManager();
	virtual ~CSoundManager();

	void Init();
	void Destroy();
	void SetMSS(WMilesSoundSystem* pMSS);
	WMilesSoundSystem* GetMSS() const;
	void Activate(bool bActive);

	virtual void Process(float dt, WView* pView);

	int GetSoundFx(char* filename, bool b3D);
	virtual _SFX_INFO* LoadSfx(const char* name, const WVector* pos,
		unsigned long flags, const char* alias, const CSoundObject* pObject,
		float minVolume, float maxDistance);
	virtual void PlaySfx(const char* name, const WVector* pos = NULL,
		unsigned long flags = 0, const char* alias = NULL,
		const CSoundObject* pObject = NULL, float minVolume = 0.5f,
		float maxDistance = 200.0f);
	virtual void StopSfx(const char* name);
	bool IsPlaying(const char* name);
	void SetSFXVolumeWhilePlaying(const char* name, float volume);
	void StopAllSound();

	virtual void PlayBGM(const char* name, bool bLoop, bool bFadeOut);
	virtual void StopBGM(bool bFadeOut);
	virtual void PauseBGM(bool bFadeOut);
	bool IsPlayingBGM();
	const char* GetBGMName() const { return m_bgmName; }
	bool IsBGMStopped983() const { return m_bBgmStopped983; }

	virtual void AllDelSfx();

	void SetVolume(float volume);
	float GetVolume() const;
	void SetBGMVolume(float volume);
	void SetBGMVolumeInBGMArea(float rate);
	float GetBGMVolume() const;
	void SetSpeakerType(int type);

protected:
	virtual void AddSfx(_SFX_INFO* pInfo);
	virtual void DelSfx(const char* name);
	virtual void DelSfx(_SFX_INFO* pInfo);
	_SFX_INFO* FindSfx(const char* name);
	_SFX_INFO* FindAliasSfx(const char* alias);
	_SFX_INFO* FindIdleSfx(const char* name);
	void CheckIdleSfx(float dt);
	void DestroyBGM();

public:
	void SetFadeOutTime(float time) { m_fadeOutTime = time; }

protected:
	WMilesSoundSystem* m_pMSS;
	float m_volume;
	int m_bgm;
	int m_fadeOutBgm;
	float m_fadeOutVolume;
	float m_fadeOutElapsed;
	float m_fadeOutTime;
	float m_bgmVolume;
	char m_bgmName[64];
	float m_fadeHeight;
	float m_maxHeight;
	float m_activeRadius;
	float m_idleCheckTime;
	bool m_bActive;
	bool m_bBgmStopped983; // 983: +0x75; 1 = StopBGM/PauseBGM sem fade (009b5dc0/009b5e40), 0 = PlayBGM tocou/retomou (009b6590); jukebox 007bc1e0 le
	WList<_SFX_INFO*> m_sfxList;
	WList<_SFX_INFO*> m_idleList;
};

extern CSoundManager* g_audio;
