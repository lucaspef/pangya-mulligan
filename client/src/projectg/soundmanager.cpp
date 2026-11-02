#include "minatl.h"
#include "soundmanager.h"
#include "miles.h"
#include "wview.h"

extern const float SOUND_UNIT_SCALE = 0.32f;

CSoundManager::CSoundManager()
	: m_sfxList(16, 16), m_idleList(16, 16)
{
	Init();
}

CSoundManager::~CSoundManager()
{
	Destroy();
}

void CSoundManager::Init()
{
	m_pMSS = NULL;
	m_bActive = true;
	m_bBgmStopped983 = false;
	m_idleCheckTime = 0.0f;
	m_volume = 1.0f;
	m_bgmVolume = 1.0f;
	m_bgm = 0;
	m_fadeOutBgm = 0;
	m_fadeOutVolume = 1.0f;
	m_fadeOutElapsed = 0.0f;
	m_fadeOutTime = 1.0f;
	m_maxHeight = 160.0f;
	m_fadeHeight = 128.0f;
	m_activeRadius = 960.0f;
	memset(m_bgmName, 0, sizeof(m_bgmName));
}

void CSoundManager::Destroy()
{
	AllDelSfx();
	m_pMSS = NULL;
}

void CSoundManager::SetMSS(WMilesSoundSystem* pMSS)
{
	m_pMSS = pMSS;
}

WMilesSoundSystem* CSoundManager::GetMSS() const
{
	return m_pMSS;
}

void CSoundManager::Activate(bool bActive)
{
	m_bActive = bActive;
	if (m_pMSS)
		m_pMSS->Activate(bActive);
}

extern "C" void __cdecl jp983_rastro(const char* fmt, ...);
extern "C" void* _ReturnAddress(void);
#pragma intrinsic(_ReturnAddress)
void CSoundManager::Process(float dt, WView* pView)
{
	if (!m_pMSS)
		return;

	float height = 0.0f;
	float radiusSquared = m_activeRadius * m_activeRadius;
	float heightRange = m_maxHeight - m_fadeHeight;
	if (m_fadeOutBgm)
	{
		m_fadeOutElapsed += dt;
		if (m_fadeOutElapsed > m_fadeOutTime)
		{
			// B1530: 983 009b6f05 destroi o buffer (009b5a90 -> DestroySoundBuffer), nao so pausa
			jp983_rastro("B1506 fade acabou handle=%d (bgm=%d %s)", m_fadeOutBgm, m_bgm, m_bgmName);
			m_pMSS->DestroySoundBuffer(m_fadeOutBgm);
			m_fadeOutBgm = 0;
			m_fadeOutElapsed = 0.0f;
		}
		else
			m_pMSS->SetVolume(m_fadeOutBgm,
				(1.0f - m_fadeOutElapsed / m_fadeOutTime) * m_fadeOutVolume);
	}

	for (_SFX_INFO* pInfo = m_sfxList.Start(); pInfo; pInfo = m_sfxList.Next())
	{
		if (pInfo->bActive == true && pInfo->b3D == true)
		{
			if (pInfo->flags & 4)
			{
				pInfo->pos = pInfo->pObject->GetPos();
				WVector pos = pInfo->pos * pView->invcamera;
				m_pMSS->SetPosition(pInfo->handle, pos.x, pos.y, pos.z);
			}
			if (!pInfo->IsSet(2) || !pInfo->bPlayed)
			{
				float dy = pView->GetCamera().pivot.y - pInfo->pos.y;
				height = dy > 0.0f ? dy : -dy;
				if (height >= m_fadeHeight)
				{
					float volume =
						(m_maxHeight - height) / heightRange * m_volume;
					m_pMSS->SetVolume(pInfo->handle, Max(0.0f, volume));
				}
				WVector pos = pInfo->pos * pView->invcamera;
				m_pMSS->SetPosition(pInfo->handle, pos.x, pos.y, pos.z);
			}
			if (!pInfo->bPlayed)
			{
				WVector diff;
				diff = pView->GetCamera().pivot - pInfo->pos;
				if ((diff.x * diff.x + diff.z * diff.z < radiusSquared &&
						height < m_maxHeight) ||
					pInfo->IsSet(1))
				{
					m_pMSS->Play(pInfo->handle, pInfo->IsSet(1), true);
					pInfo->bPlayed = true;
				}
			}
		}
	}
	CheckIdleSfx(dt);
}

int CSoundManager::GetSoundFx(char* filename, bool b3D)
{
	if (!m_pMSS || !filename)
		return 0;
	int type;
	if (b3D)
		type = 1;
	else if (strstr(filename, ".a") || strstr(filename, ".mp3"))
		type = 2;
	else
		type = 0;
	return m_pMSS->Load(filename, type);
}

_SFX_INFO* CSoundManager::FindIdleSfx(const char* name)
{
	for (_SFX_INFO* pInfo = m_sfxList.Start(); pInfo; pInfo = m_sfxList.Next())
	{
		if ((!pInfo->bActive || pInfo->bPlayed == true) &&
			!strcmpi(pInfo->name, name))
			if (m_pMSS && !m_pMSS->IsPlaying(pInfo->handle))
				return pInfo;
	}
	return NULL;
}

void CSoundManager::CheckIdleSfx(float dt)
{
	if (!m_bActive)
		return;
	m_idleCheckTime += dt;
	if (m_idleCheckTime > 5.0f)
	{
		m_idleCheckTime = 0.0f;
		unsigned long tick = GetTickCount();
		_SFX_INFO* pInfo;
		for (pInfo = m_sfxList.Start(); pInfo; pInfo = m_sfxList.Next())
		{
			if (tick - pInfo->tick > 50000 && m_pMSS &&
				!m_pMSS->IsPlaying(pInfo->handle))
				m_idleList += pInfo;
		}
		for (pInfo = m_idleList.Start(); pInfo; pInfo = m_idleList.Next())
			DelSfx(pInfo);
		m_idleList.Reset();
	}
}

_SFX_INFO* CSoundManager::LoadSfx(const char* name, const WVector* pos,
	unsigned long flags, const char* alias, const CSoundObject* pObject,
	float minVolume, float maxDistance)
{
	if (!m_pMSS)
		return NULL;
	bool b3D = pos && flags != 5;
	_SFX_INFO* pInfo = FindIdleSfx(name);
	if (!pInfo)
	{
		char filename[MAX_PATH];
		sprintf(filename, "sound/%s.wav", name);
		int handle = GetSoundFx(filename, b3D);
		if (!handle)
			return NULL;
		pInfo = new _SFX_INFO;
		if (!pInfo)
		{
			m_pMSS->DestroySoundBuffer(handle);
			return NULL;
		}
		memset(pInfo, 0, sizeof(*pInfo));
		strcpy(pInfo->name, name);
		pInfo->handle = handle;
		AddSfx(pInfo);
	}
	else if (pInfo->handle) // A910: 983 009b6d27: som reaproveitado para antes de recomecar
	{
		m_pMSS->Stop(pInfo->handle);
		pInfo->bActive = false;
	}
	float volume = m_volume;
	memset(pInfo->alias, 0, sizeof(pInfo->alias));
	if (alias)
		strcpy(pInfo->alias, alias);
	pInfo->flags = flags;
	pInfo->bActive = true;
	pInfo->tick = GetTickCount();
	if (pos && flags != 5)
	{
		pInfo->pos = *pos;
		pInfo->b3D = b3D;
		pInfo->bPlayed = !b3D;
	}
	else
	{
		pInfo->b3D = false;
		pInfo->bPlayed = true;
		if (flags == 5)
		{
			volume *= max(minVolume,
				(maxDistance - (pos->Magnitude())) / maxDistance);
			pInfo->flags = flags = 0;
		}
	}
	pInfo->pObject = (flags & 4) ? pObject : NULL;
	m_pMSS->SetVolume(pInfo->handle, volume);
	return pInfo;
}

void CSoundManager::PlaySfx(const char* name, const WVector* pos,
	unsigned long flags, const char* alias, const CSoundObject* pObject,
	float minVolume, float maxDistance)
{
	_SFX_INFO* pInfo =
		LoadSfx(name, pos, flags, alias, pObject, minVolume, maxDistance);
	if (pInfo && pInfo->bPlayed == true)
		m_pMSS->Play(pInfo->handle, pInfo->IsSet(1), true);
}

void CSoundManager::StopSfx(const char* name)
{
	if (!m_pMSS)
		return;
	if (!name)
	{
		for (_SFX_INFO* pInfo = m_sfxList.Start(); pInfo;
			pInfo = m_sfxList.Next())
		{
			if (pInfo->bActive)
			{
				m_pMSS->Stop(pInfo->handle);
				pInfo->bActive = false;
			}
		}
	}
	else
	{
		_SFX_INFO* pInfo = FindAliasSfx(name);
		if (pInfo && pInfo->handle && pInfo->bActive)
		{
			m_pMSS->Stop(pInfo->handle);
			pInfo->bActive = false;
		}
	}
}

bool CSoundManager::IsPlaying(const char* name)
{
	if (!strlen(name))
		return false;
	_SFX_INFO* pInfo = FindAliasSfx(name);
	if (pInfo && pInfo->handle && m_pMSS)
		return m_pMSS->IsPlaying(pInfo->handle);
	if (m_pMSS && !stricmp(name, m_bgmName))
		return m_pMSS->IsPlaying(m_bgm);
	return false;
}

bool CSoundManager::IsPlayingBGM()
{
	if (m_bgm)
		return true;
	return false;
}

_SFX_INFO* CSoundManager::FindSfx(const char* name)
{
	for (_SFX_INFO* pInfo = m_sfxList.Start(); pInfo; pInfo = m_sfxList.Next())
		if (!strcmpi(pInfo->name, name))
			return pInfo;
	return NULL;
}

_SFX_INFO* CSoundManager::FindAliasSfx(const char* alias)
{
	for (_SFX_INFO* pInfo = m_sfxList.Start(); pInfo; pInfo = m_sfxList.Next())
		if (!strcmpi(pInfo->alias, alias))
			return pInfo;
	return NULL;
}

void CSoundManager::AddSfx(_SFX_INFO* pInfo)
{
	m_sfxList += pInfo;
}

void CSoundManager::DelSfx(const char* name)
{
	_SFX_INFO* pInfo = FindAliasSfx(name);
	if (pInfo)
		DelSfx(pInfo);
}

void CSoundManager::DelSfx(_SFX_INFO* pInfo)
{
	m_sfxList -= pInfo;
	if (m_pMSS)
		m_pMSS->DestroySoundBuffer(pInfo->handle);
	delete pInfo;
}

void CSoundManager::AllDelSfx()
{
	_SFX_INFO* pInfo;
	while (pInfo = m_sfxList.Start())
		DelSfx(pInfo);
	DestroyBGM();
}

void CSoundManager::SetVolume(float volume)
{
	m_volume = volume;
	if (m_pMSS)
		for (_SFX_INFO* pInfo = m_sfxList.Start(); pInfo;
			pInfo = m_sfxList.Next())
			m_pMSS->SetVolume(pInfo->handle, volume);
}

void CSoundManager::SetSFXVolumeWhilePlaying(const char* name, float volume)
{
	_SFX_INFO* pInfo = FindAliasSfx(name);
	if (pInfo && m_pMSS)
		m_pMSS->SetVolume(pInfo->handle, m_volume * volume);
}

float CSoundManager::GetVolume() const
{
	return m_volume;
}

extern "C" void __cdecl jp983_rastro(const char* fmt, ...);
void CSoundManager::PlayBGM(const char* name, bool bLoop, bool bFadeOut)
{
	jp983_rastro("B543 PlayBGM name=%s mss=%p bgm=%d cur=%s", name ? name : "(null)", m_pMSS, m_bgm, m_bgmName);
	if (!m_pMSS)
		return;
	if (!name)
	{
		StopBGM(bFadeOut);
		m_bgmName[0] = 0;
	}
	else
	{
		if (m_bgm && !stricmp(name, m_bgmName))
		{
			if (!m_pMSS->IsPlaying(m_bgm))
			{
				m_pMSS->Resume(m_bgm, !bLoop);
				m_bBgmStopped983 = false;
			}
		}
		else
		{
			int handle = GetSoundFx((char*)name, false);
			jp983_rastro("B543 GetSoundFx=%d", handle);
			if (handle)
			{
				StopBGM(bFadeOut);
				m_bgm = handle;
				strcpy(m_bgmName, name);
				m_pMSS->Play(m_bgm, !bLoop, true);
				m_bBgmStopped983 = false;
				jp983_rastro("B543 PlayBGM handle=%d playing=%d", handle, (int)m_pMSS->IsPlaying(m_bgm));
				SetBGMVolume(m_bgmVolume);
			}
		}
	}
}

void CSoundManager::StopBGM(bool bFadeOut)
{
	// B1506 (temporario): desistir do Practice deixaria a BGM do mapa tocando (Revisor 17:55); quem para e quando
	jp983_rastro("B1506 StopBGM de %p fade=%d bgm=%d %s fadeOut=%d", _ReturnAddress(), (int)bFadeOut, m_bgm, m_bgmName, m_fadeOutBgm);
	// B1530: como o 983 (009b5dc0): limpa o nome e DESTROI os buffers (009b5a90); sem fade destroi tambem a BGM
	// que ainda estava no fade (o nosso so zerava m_fadeOutBgm e ela seguia tocando sem ninguem parar)
	if (m_bgm && m_pMSS)
	{
		memset(m_bgmName, 0, sizeof(m_bgmName));
		if (bFadeOut)
		{
			if (m_fadeOutBgm)
				m_pMSS->DestroySoundBuffer(m_fadeOutBgm);
			m_fadeOutBgm = m_bgm;
			m_fadeOutVolume = m_bgmVolume;
			m_fadeOutElapsed = 0.0f;
			m_bgm = 0;
		}
		else
		{
			if (m_fadeOutBgm)
				m_pMSS->DestroySoundBuffer(m_fadeOutBgm);
			m_fadeOutBgm = 0;
			m_pMSS->DestroySoundBuffer(m_bgm);
			m_bgm = 0;
			m_bBgmStopped983 = true;
		}
	}
}

void CSoundManager::PauseBGM(bool bFadeOut)
{
	if (m_bgm && m_pMSS)
	{
		if (bFadeOut)
		{
			if (m_fadeOutBgm) // B1530: 983 009b5e67 destroi o buffer anterior
				m_pMSS->DestroySoundBuffer(m_fadeOutBgm);
			m_fadeOutBgm = m_bgm;
			m_fadeOutElapsed = 0.0f;
			m_fadeOutVolume = m_bgmVolume;
		}
		else
		{
			m_pMSS->Stop(m_bgm);
			m_bBgmStopped983 = true;
		}
	}
}

void CSoundManager::SetBGMVolume(float volume)
{
	m_bgmVolume = volume;
	if (m_bgm && m_pMSS)
		m_pMSS->SetVolume(m_bgm, volume);
}

void CSoundManager::SetBGMVolumeInBGMArea(float rate)
{
	if (m_bgm && m_pMSS)
		m_pMSS->SetVolume(m_bgm, m_bgmVolume * rate);
}

float CSoundManager::GetBGMVolume() const
{
	return m_bgmVolume;
}

// 983 009b6670: so para os sons, que seguem na lista ate o CheckIdleSfx os destruir; apagar o _SFX_INFO aqui (645)
// perdia o handle do Miles a cada fim de partida ate GetEmptyHandle dar 0, e entao PlayBGM/PlaySfx nao tocavam
void CSoundManager::StopAllSound()
{
	StopSfx(NULL);
}

void CSoundManager::DestroyBGM()
{
	if (m_bgm)
	{
		if (m_pMSS)
			m_pMSS->DestroySoundBuffer(m_bgm);
		m_bgm = 0;
	}
}

void CSoundManager::SetSpeakerType(int type)
{
	if (m_pMSS)
		m_pMSS->SpeakerType((WMilesSoundSystem::w_speaker_type)type);
}
