#include "minatl.h"
#include "wevent.h"
#include "mss.h"
#include "miles.h"

enum
{
	W_SOUND_2D,
	W_SOUND_3D,
	W_SOUND_STREAM,
	W_SOUND_NONE
};

static WSyncEvent s_event;

// 983 00e7e9f0: g_files/g_offset sao tocados pela thread de stream do MSS e pela principal
struct MilesFileCs983
{
	CRITICAL_SECTION cs;
	MilesFileCs983() { InitializeCriticalSection(&cs); }
	~MilesFileCs983() { DeleteCriticalSection(&cs); }
};
static MilesFileCs983 s_fileCs983;

struct MilesFileLock983
{
	MilesFileLock983() { EnterCriticalSection(&s_fileCs983.cs); }
	~MilesFileLock983() { LeaveCriticalSection(&s_fileCs983.cs); }
};
cFile* WMilesSoundSystem::g_files[16];
int WMilesSoundSystem::g_offset[16];

WMilesSoundSystem::WMilesSoundSystem(HWND__* hWnd, const char* redistDir,
	bool bOption, int param1, int param2, int param3)
	: m_hDriver(NULL),
	  m_sample2DList(16, 16),
	  m_sample3DList(16, 16)
{
	m_pad983_04 = 1.0f;
	m_volume2D = 1.0f;
	m_volume3D = 1.0f;
	m_lastHandle = 0;
	m_hMixer = NULL;
	m_nearDistance = 0.0f;
	m_farDistance = 100.0f;
	m_bFlipX = false;
	m_pad983_19 = false;
	m_bAllowOverlap = false;
	for (int i = 0; i < 1024; ++i)
	{
		m_sound[i].sample2d = NULL;
		m_sound[i].data2d = NULL;
		m_sound[i].sample3d = NULL;
		m_sound[i].data3d = NULL;
		m_sound[i].stream = NULL;
		m_sound[i].streamPos = 0;
		m_sound[i].type = W_SOUND_NONE;
	}
	for (int i = 0; i < 8; ++i)
		m_stream[i] = NULL;
	for (int i = 0; i < 8; ++i)
		m_streamOwner[i] = 0;
	for (int i = 0; i < 16; ++i)
		m_sample2DOwner[i] = 0;
	for (int i = 0; i < 16; ++i)
		m_sample2D[i] = NULL;
	for (int i = 0; i < 16; ++i)
		m_sample3DOwner[i] = 0;
	for (int i = 0; i < 16; ++i)
		m_sample3D[i] = NULL;

	if (redistDir)
		AIL_set_redist_directory(redistDir);

	for (int i = 0; i < 16; ++i)
		g_files[i] = NULL;

	AIL_set_file_callbacks(Open_callback, Close_callback, Seek_callback,
		Read_callback);
	if (!AIL_startup())
	{
		char buf[256];
		// "Unable to initialize sound: %s"
		sprintf(buf,
			K2L_Compatibility("\xbb\xe7\xbf\xee\xb5\xe5\xb8\xa6 \xc3\xca\xb1\xe2\xc8\xad\xc7\xd2 "
			"\xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9: %s"),
			AIL_last_error());
		MessageBox(NULL, buf, "Miles Sound System", MB_OK);
	}
}

bool WMilesSoundSystem::InitMixer()
{
	UINT numDevs = mixerGetNumDevs();
	m_hMixer = NULL;
	if (numDevs)
	{
		if (mixerOpen(&m_hMixer, 0, 0, 0, 0) == MMSYSERR_NOERROR)
		{
			ZeroMemory(&m_mixerLine, sizeof(m_mixerLine));
			m_mixerLine.cbStruct = sizeof(m_mixerLine);
			m_mixerLine.dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT;
			if (mixerGetLineInfo((HMIXEROBJ)m_hMixer, &m_mixerLine,
					MIXER_GETLINEINFOF_COMPONENTTYPE) == MMSYSERR_NOERROR)
			{
				MIXERLINECONTROLS mlc;
				mlc.dwLineID = m_mixerLine.dwLineID;
				ZeroMemory(&m_mixerControl, sizeof(m_mixerControl));
				mlc.pamxctrl = &m_mixerControl;
				m_mixerControl.cbStruct = sizeof(m_mixerControl);
				mlc.cbStruct = sizeof(mlc);
				mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
				mlc.cControls = 1;
				mlc.cbmxctrl = sizeof(MIXERCONTROL);
				if (mixerGetLineControls((HMIXEROBJ)m_hMixer, &mlc,
						MIXER_OBJECTF_HMIXER |
							MIXER_GETLINECONTROLSF_ONEBYTYPE) ==
					MMSYSERR_NOERROR)
					return true;
			}
		}
	}
	return false;
}

void WMilesSoundSystem::SetMixer(float volume)
{
	MIXERCONTROLDETAILS_UNSIGNED value;
	value.dwValue = (DWORD)((m_mixerControl.Bounds.dwMaximum - 1) * volume);
	MIXERCONTROLDETAILS mcd;
	mcd.cbStruct = sizeof(mcd);
	mcd.dwControlID = m_mixerControl.dwControlID;
	mcd.cChannels = 1;
	mcd.cMultipleItems = 0;
	mcd.cbDetails = sizeof(value);
	mcd.paDetails = &value;
	mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mcd,
		MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE);
}

void WMilesSoundSystem::CloseMixer()
{
	if (m_hMixer)
		mixerClose(m_hMixer);
}

WMilesSoundSystem::~WMilesSoundSystem()
{
	CloseMixer();
	ClearAllSamples();
	ClearAllSounds();
	ClearAllStreams();
	{
		MilesFileLock983 lock; // 983 009262ef..00926330, solta antes do AIL_shutdown
		for (unsigned int i = 0; i < 16; ++i)
		{
			if (g_files[i])
			{
				CloseCFile(g_files[i]);
				g_files[i] = NULL;
			}
		}
	}
	if (m_hDriver)
		AIL_digital_handle_release(m_hDriver);
	if (m_hDriver)
		AIL_close_digital_driver(m_hDriver);
	AIL_shutdown();
}

void WMilesSoundSystem::ClearAllStreams()
{
	for (int i = 0; i < 8; ++i)
	{
		if (m_stream[i])
		{
			AIL_close_stream(m_stream[i]);
			m_stream[i] = NULL;
			if (m_streamOwner[i])
			{
				m_sound[m_streamOwner[i]].stream = NULL;
				m_sound[m_streamOwner[i]].streamPos = 0;
				m_streamOwner[i] = 0;
			}
		}
	}
}

void WMilesSoundSystem::ClearAllSamples()
{
	for (int i = 0; i < 16; ++i)
	{
		if (m_sample2D[i])
		{
			AIL_stop_sample(m_sample2D[i]);
			AIL_release_sample_handle(m_sample2D[i]);
			m_sample2D[i] = NULL;
			if (m_sample2DOwner[i] > 0)
			{
				m_sound[m_sample2DOwner[i]].sample2d = NULL;
				m_sample2DOwner[i] = 0;
			}
		}
	}
	memset(m_sample2D, 0, sizeof(m_sample2D));
	for (int i = 0; i < 16; ++i)
	{
		if (m_sample3D[i])
		{
			AIL_stop_sample(m_sample3D[i]);
			AIL_release_sample_handle(m_sample3D[i]);
			m_sample3D[i] = NULL;
			if (m_sample3DOwner[i] > 0)
			{
				m_sound[m_sample3DOwner[i]].sample3d = NULL;
				m_sample3DOwner[i] = 0;
			}
		}
	}
	memset(m_sample3D, 0, sizeof(m_sample3D));
}

void WMilesSoundSystem::ClearAllSounds()
{
	msample2d* sample2d = m_sample2DList.Start();
	while (sample2d)
	{
		AIL_mem_free_lock(sample2d->data);
		delete[] sample2d;
		sample2d = m_sample2DList.Next();
	}
	m_sample2DList.Reset();

	msample3d* sample3d = m_sample3DList.Start();
	while (sample3d)
	{
		AIL_mem_free_lock(sample3d->data);
		delete[] sample3d;
		sample3d = m_sample3DList.Next();
	}
	m_sample3DList.Reset();
}

int WMilesSoundSystem::GetCPUpercent()
{
	return m_hDriver ? AIL_digital_CPU_percent(m_hDriver) : 0;
}

bool WMilesSoundSystem::Init(HWND__* hWnd, int rate, int bits)
{
	// 983 00923e10: sem AIL_set_DirectSound_HWND nem Activate (o Mss32.dll do
	// cliente JP nao exporta o primeiro); mensagem pelo K2L_Compatibility.
	m_hDriver = AIL_open_digital_driver(rate, bits, MSS_MC_USE_SYSTEM_CONFIG,
		AIL_OPEN_DIGITAL_FORCE_PREFERENCE);
	if (!m_hDriver)
	{
		char buf[256];
		// "Unable to open the sound driver: %s"
		sprintf(buf,
			K2L_Compatibility("\xbb\xe7\xbf\xee\xb5\xe5 \xb5\xe5\xb6\xf3\xc0\xcc\xb9\xf6\xb8\xa6 "
			"\xbf\xad \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9: %s"),
			AIL_last_error());
		MessageBox(NULL, buf, "Miles Sound System", MB_OK);
		return false;
	}
	InitMixer();
	return true;
}

void WMilesSoundSystem::SpeakerType(w_speaker_type type)
{
	S32 nchannels;
	S32 nspeakers;
	F32 falloff = 0.0f;
	MSS_MC_SPEC spec;
	switch (type)
	{
	case W_SPEAKER_2CH:
		nspeakers = nchannels = spec = MSS_MC_STEREO;
		break;
	case W_SPEAKER_HEADPHONE:
		nspeakers = nchannels = 2, spec = MSS_MC_HEADPHONES;
		break;
	case W_SPEAKER_SURROUND:
		nchannels = 2, nspeakers = 3, spec = MSS_MC_DOLBY_SURROUND;
		break;
	case W_SPEAKER_4CH:
		nspeakers = nchannels = 4, spec = MSS_MC_40_DISCRETE;
		break;
	case W_SPEAKER_51CH:
		nspeakers = nchannels = 6, spec = MSS_MC_51_DISCRETE;
		break;
	case W_SPEAKER_71CH:
		nspeakers = nchannels = 8, spec = MSS_MC_71_DISCRETE;
		break;
	}
	MSS_SPEAKER* speakers = AIL_speaker_configuration(m_hDriver, &nchannels,
		&nspeakers, &falloff, &spec);
	AIL_set_speaker_configuration(m_hDriver, speakers, 0, 0.0f);
}

void WMilesSoundSystem::SetBalance(int balance)
{
	if (balance < 128)
	{
		m_volume2D = 1.0f;
		m_volume3D = balance < 0 ? 0.0f : balance / 128.0f;
	}
	else
	{
		float volume = balance > 256 ? 1.0f : (256 - balance) / 128.0f;
		m_volume3D = 1.0f;
		m_volume2D = volume;
	}
}

void WMilesSoundSystem::Activate(bool bActive)
{
}

void WMilesSoundSystem::ClearIdleSound(int age)
{
	WList<msample2d*> idle2D(16, 16);
	WList<msample3d*> idle3D(16, 16);

	for (msample2d* sample2d = m_sample2DList.Start(); sample2d;
		sample2d = m_sample2DList.Next())
	{
		if (sample2d->refCount == 0)
		{
			sample2d->idleCount++;
			if (sample2d->idleCount >= age + 1)
				idle2D += sample2d;
		}
	}
	for (msample2d* sample2d = idle2D.Start(); sample2d;
		sample2d = idle2D.Next())
	{
		m_sample2DList -= sample2d;
		AIL_mem_free_lock(sample2d->data);
		delete[] sample2d;
	}

	for (msample3d* sample3d = m_sample3DList.Start(); sample3d;
		sample3d = m_sample3DList.Next())
	{
		if (sample3d->refCount == 0)
		{
			sample3d->idleCount++;
			if (sample3d->idleCount >= age + 1)
				idle3D += sample3d;
		}
	}
	for (msample3d* sample3d = idle3D.Start(); sample3d;
		sample3d = idle3D.Next())
	{
		m_sample3DList -= sample3d;
		AIL_mem_free_lock(sample3d->data);
		delete[] sample3d;
	}
}

int WMilesSoundSystem::Load(char* filename, int type)
{
	int handle = GetEmptyHandle();
	if (handle > 0)
	{
		if (type == W_SOUND_STREAM)
		{
			unsigned int i;
			for (i = 0; m_stream[i]; ++i)
			{
				if (i >= 8)
					break;
				int status = AIL_stream_status(m_stream[i]);
				if (status == SMP_DONE || status == SMP_STOPPED)
					break;
			}
			if (i < 8)
			{
				if (m_stream[i])
				{
					if (m_sound[m_streamOwner[i]].stream == m_stream[i])
						m_sound[m_streamOwner[i]].stream = NULL;
					AIL_close_stream(m_stream[i]);
					m_stream[i] = NULL;
				}
				HSTREAM stream = AIL_open_stream(m_hDriver, filename, 0);
				if (stream)
				{
					m_stream[i] = stream;
					m_streamOwner[i] = handle;
					AIL_set_stream_user_data(stream, i,
						AIL_sample_playback_rate(
							AIL_stream_sample_handle(stream)));
					m_sound[handle].stream = stream;
					m_sound[handle].type = W_SOUND_STREAM;
					SetVolume(handle, 1.0f);
					return handle;
				}
			}
		}
		else if (type == W_SOUND_3D)
		{
			msample3d* sample = LoadSample3D(filename);
			if (!sample)
				return 0;
			m_sound[handle].data3d = sample;
			m_sound[handle].sample3d = NULL;
			m_sound[handle].nearDistance = m_nearDistance;
			m_sound[handle].farDistance = m_farDistance;
			m_sound[handle].type = W_SOUND_3D;
			SetVolume(handle, 1.0f);
			return handle;
		}
		else
		{
			msample2d* sample = LoadSample2D(filename);
			if (!sample)
				return 0;
			m_sound[handle].data2d = sample;
			m_sound[handle].sample2d = NULL;
			m_sound[handle].type = W_SOUND_2D;
			SetVolume(handle, 1.0f);
			return handle;
		}
	}
	return 0;
}

void WMilesSoundSystem::DestroySoundBuffer(int handle)
{
	Stop(handle);
	switch (m_sound[handle].type)
	{
	case W_SOUND_2D:
		Release2D(m_sound[handle].data2d);
		m_sound[handle].data2d = NULL;
		break;
	case W_SOUND_3D:
		Release3D(m_sound[handle].data3d);
		m_sound[handle].data3d = NULL;
		break;
	case W_SOUND_STREAM:
		if (m_sound[handle].stream)
		{
			AIL_close_stream(m_sound[handle].stream);
			for (unsigned int i = 0; i < 8; ++i)
			{
				if (m_stream[i] == m_sound[handle].stream)
				{
					m_stream[i] = NULL;
					if (m_streamOwner[i])
					{
						m_sound[m_streamOwner[i]].stream = NULL;
						m_streamOwner[i] = 0;
					}
					break;
				}
				if (m_streamOwner[i] == handle)
				{
					m_streamOwner[i] = 0;
					AIL_close_stream(m_stream[i]);
					m_stream[i] = NULL;
				}
			}
			m_sound[handle].stream = NULL;
		}
		break;
	}
	m_sound[handle].type = W_SOUND_NONE;
}

void WMilesSoundSystem::Volume(float volume)
{
	SetMixer(volume);
}

void WMilesSoundSystem::Play(int handle, int loop, bool bReplace)
{
	switch (m_sound[handle].type)
	{
	case W_SOUND_2D:
		if (m_sound[handle].data2d->data)
		{
			int last = m_sound[handle].data2d->lastHandle;
			m_sound[handle].data2d->lastHandle = handle;
			if (!m_bAllowOverlap)
			{
				if (bReplace && m_sound[last].type == W_SOUND_2D &&
					m_sound[last].data2d == m_sound[handle].data2d)
					Stop(last);
			}
			HSAMPLE sample = GetAllocateSample(handle, loop != 0);
			if (sample)
			{
				AIL_set_named_sample_file(sample, m_sound[handle].data2d->name,
					m_sound[handle].data2d->data, m_sound[handle].data2d->size,
					0);
				AIL_set_sample_loop_count(sample, loop == 0);
				AIL_set_sample_volume_pan(sample, m_sound[handle].volume, 0.5f);
				AIL_start_sample(sample);
			}
		}
		break;
	case W_SOUND_3D:
	{
		int last = m_sound[handle].data3d->lastHandle;
		m_sound[handle].data3d->lastHandle = handle;
		if (!m_bAllowOverlap)
		{
			if (bReplace && m_sound[last].type == W_SOUND_3D &&
				m_sound[last].data3d == m_sound[handle].data3d)
				Stop(last);
		}
		HSAMPLE sample = GetAllocateSample3D(handle, loop != 0);
		if (sample)
		{
			AIL_set_sample_file(sample, m_sound[handle].data3d->data, 0);
			AIL_set_sample_loop_count(sample, loop == 0);
			AIL_set_sample_3D_distances(sample, m_sound[handle].nearDistance,
				m_sound[handle].farDistance, 0);
			AIL_set_sample_volume_pan(sample, m_sound[handle].volume, 0.5f);
			AIL_set_sample_3D_position(m_sound[handle].sample3d,
				m_sound[handle].pos[0], m_sound[handle].pos[1],
				m_sound[handle].pos[2]);
			AIL_start_sample(sample);
		}
	}
	break;
	case W_SOUND_STREAM:
		if (m_sound[handle].stream)
		{
			AIL_set_stream_loop_count(m_sound[handle].stream, loop == 0);
			HSAMPLE sample = AIL_stream_sample_handle(m_sound[handle].stream);
			AIL_set_sample_volume_pan(sample, m_sound[handle].volume, 0.5f);
			AIL_start_stream(m_sound[handle].stream);
		}
		break;
	}
}

void WMilesSoundSystem::Resume(int handle, int loop)
{
	switch (m_sound[handle].type)
	{
	case W_SOUND_STREAM:
		if (m_sound[handle].stream)
		{
			AIL_set_stream_loop_count(m_sound[handle].stream, loop == 0);
			HSAMPLE sample = AIL_stream_sample_handle(m_sound[handle].stream);
			AIL_set_sample_volume_pan(sample, m_sound[handle].volume, 0.5f);
			AIL_set_stream_position(m_sound[handle].stream,
				m_sound[handle].streamPos);
			AIL_start_stream(m_sound[handle].stream);
		}
		break;
	}
}

void WMilesSoundSystem::Stop(int handle)
{
	switch (m_sound[handle].type)
	{
	case W_SOUND_2D:
		if (m_sound[handle].sample2d)
		{
			for (int i = 0; i < 16; ++i)
			{
				if (m_sample2DOwner[i] == handle)
				{
					m_sample2DOwner[i] = 0;
					AIL_stop_sample(m_sample2D[i]);
				}
			}
			m_sound[handle].sample2d = NULL;
		}
		break;
	case W_SOUND_3D:
		if (m_sound[handle].sample3d)
		{
			for (int i = 0; i < 16; ++i)
			{
				if (m_sample3DOwner[i] == handle)
				{
					m_sample3DOwner[i] = 0;
					AIL_stop_sample(m_sample3D[i]);
				}
			}
			m_sound[handle].sample3d = NULL;
		}
		break;
	case W_SOUND_STREAM:
		if (m_sound[handle].stream) // 983 009240b0
		{
			AIL_pause_stream(m_sound[handle].stream, 1);
			m_sound[handle].streamPos = AIL_stream_position(m_sound[handle].stream);
		}
		break;
	}
}

void WMilesSoundSystem::SetPitch(int handle, int rate)
{
	AIL_set_sample_playback_rate(m_sound[handle].sample2d, rate);
}

int WMilesSoundSystem::GetPitch(int handle)
{
	return AIL_sample_playback_rate(m_sound[handle].sample2d);
}

void WMilesSoundSystem::SetVolume(int handle, float volume)
{
	switch (m_sound[handle].type)
	{
	case W_SOUND_2D:
		m_sound[handle].volume = volume * m_volume2D;
		if (m_sound[handle].sample2d)
			AIL_set_sample_volume_pan(m_sound[handle].sample2d,
				m_sound[handle].volume, 0.5f);
		break;
	case W_SOUND_3D:
		m_sound[handle].volume = volume * m_volume3D;
		if (m_sound[handle].sample3d)
			AIL_set_sample_volume_pan(m_sound[handle].sample3d,
				m_sound[handle].volume, 0.5f);
		break;
	case W_SOUND_STREAM:
		m_sound[handle].volume = volume * m_volume2D;
		if (m_sound[handle].stream)
		{
			HSAMPLE sample = AIL_stream_sample_handle(m_sound[handle].stream);
			AIL_set_sample_volume_pan(sample, m_sound[handle].volume, 0.5f);
		}
		break;
	}
}

void WMilesSoundSystem::SetNearFar(int handle, float nearDist, float farDist)
{
	m_sound[handle].nearDistance = nearDist;
	m_sound[handle].farDistance = farDist;
	if (m_sound[handle].sample3d)
		AIL_set_sample_3D_distances(m_sound[handle].sample3d, nearDist, farDist,
			0);
}

void WMilesSoundSystem::SetPosition(int handle, float x, float y, float z)
{
	m_sound[handle].pos[0] = m_bFlipX ? -x : x;
	m_sound[handle].pos[1] = y;
	m_sound[handle].pos[2] = z;
	if (m_sound[handle].sample3d)
		AIL_set_sample_3D_position(m_sound[handle].sample3d, x, y, z);
}

// B1585 (temporario): por que a BGM aparece parada com o 983 igual aqui; -1 = stream NULL; uma linha por mudanca
extern "C" void __cdecl jp983_rastro(const char* fmt, ...);
#define B1585_TRACE(h, st) { static int s_h, s_st; if (s_h != (h) || s_st != (st)) { s_h = (h); s_st = (st); jp983_rastro("B1585 stream handle=%d status=%d", s_h, s_st); } }
bool WMilesSoundSystem::IsPlaying(int handle)
{
	switch (m_sound[handle].type)
	{
	case W_SOUND_2D:
		if (m_sound[handle].sample2d &&
			AIL_sample_status(m_sound[handle].sample2d) == SMP_PLAYING)
			return true;
		return false;
	case W_SOUND_3D:
		if (m_sound[handle].sample3d &&
			AIL_sample_status(m_sound[handle].sample3d) == SMP_PLAYING)
			return true;
		return false;
	case W_SOUND_STREAM:
		if (m_sound[handle].stream)
		{
			int status = AIL_stream_status(m_sound[handle].stream);
			if (status != SMP_DONE && status != SMP_STOPPED)
				return true;
			B1585_TRACE(handle, status);
		}
		else
			B1585_TRACE(handle, -1);
		break;
	}
	return false;
}

int WMilesSoundSystem::GetPosition(int handle)
{
	switch (m_sound[handle].type)
	{
	case W_SOUND_2D:
		if (m_sound[handle].sample2d &&
			AIL_sample_status(m_sound[handle].sample2d) == SMP_PLAYING)
			return AIL_sample_position(m_sound[handle].sample2d);
		break;
	case W_SOUND_3D:
		return AIL_sample_status(m_sound[handle].sample3d) == SMP_PLAYING
			? AIL_sample_position(m_sound[handle].sample3d)
			: -1;
	case W_SOUND_STREAM:
		if (AIL_stream_status(m_sound[handle].stream) == SMP_PLAYING)
		{
			S32 ms;
			AIL_stream_ms_position(m_sound[handle].stream, NULL, &ms);
			return ms / 1000;
		}
		break;
	}
	return -1;
}

int WMilesSoundSystem::GetLength(int handle)
{
	switch (m_sound[handle].type)
	{
	case W_SOUND_2D:
	case W_SOUND_3D:
		return 0;
	case W_SOUND_STREAM:
	{
		S32 datarate, sndtype, length, memory;
		AIL_stream_info(m_sound[handle].stream, &datarate, &sndtype, &length,
			&memory);
		return length / datarate;
	}
	}
	return -1;
}

WMilesSoundSystem::msample2d* WMilesSoundSystem::LoadSample2D(char* filename)
{
	msample2d* sample = m_sample2DList.Find(filename);
	if (!sample)
	{
		sample = (msample2d*)new char[sizeof(msample2d) + strlen(filename)];
		sample->size = AIL_file_size(filename);
		sample->data = sample->size > 0 ? AIL_file_read(filename, NULL) : NULL;
		sample->refCount = 0;
		sample->lastHandle = 0;
		strcpy(sample->name, filename);
		m_sample2DList.AddItem(sample, sample->name, false);
	}
	sample->refCount++;
	return sample;
}

void WMilesSoundSystem::Release2D(msample2d* sample)
{
	sample->refCount--;
	sample->idleCount = 0;
}

void WMilesSoundSystem::LoadSample3D(msample3d* sample)
{
	int size = AIL_file_size(sample->name);
	sample->data = AIL_file_read(sample->name, NULL);
	void* data;
	switch (AIL_file_type(sample->data, size))
	{
	case AILFILETYPE_ADPCM_WAV:
	{
		AILSOUNDINFO info;
		AIL_WAV_info(sample->data, &info);
		AIL_decompress_ADPCM(&info, &data, NULL);
		AIL_mem_free_lock(sample->data);
		sample->data = data;
	}
	break;
	case AILFILETYPE_MPEG_L3_AUDIO:
		AIL_decompress_ASI(sample->data, size, sample->name, &data, NULL, NULL);
		AIL_mem_free_lock(sample->data);
		sample->data = data;
		break;
	}
}

WMilesSoundSystem::msample3d* WMilesSoundSystem::LoadSample3D(char* filename)
{
	msample3d* sample = m_sample3DList.Find(filename);
	if (!sample)
	{
		sample = (msample3d*)new char[sizeof(msample3d) + strlen(filename)];
		sample->refCount = 0;
		sample->idleCount = 0;
		sample->lastHandle = 0;
		strcpy(sample->name, filename);
		LoadSample3D(sample);
		m_sample3DList.AddItem(sample, sample->name, false);
	}
	sample->refCount++;
	return sample;
}

void WMilesSoundSystem::Release3D(msample3d* sample)
{
	sample->refCount--;
	sample->idleCount = 0;
}

void WMilesSoundSystem::Process()
{
}

int WMilesSoundSystem::GetEmptyHandle()
{
	for (int i = 0; i < 1024; ++i)
	{
		m_lastHandle = (m_lastHandle + 1) & 1023;
		if (m_lastHandle > 0 && m_sound[m_lastHandle].type == W_SOUND_NONE)
			return m_lastHandle;
	}
	return 0;
}

void WMilesSoundSystem::ClearIdleSample()
{
	for (int i = 0; i < 16; ++i)
	{
		if (m_sample2D[i])
		{
			int status = AIL_sample_status(m_sample2D[i]);
			if (status == SMP_DONE || status == SMP_STOPPED)
			{
				if (m_sample2DOwner[i] > 0)
				{
					if (m_sound[m_sample2DOwner[i]].sample2d == m_sample2D[i])
						m_sound[m_sample2DOwner[i]].sample2d = NULL;
					m_sample2DOwner[i] = 0;
				}
				AIL_stop_sample(m_sample2D[i]);
				AIL_release_sample_handle(m_sample2D[i]);
				m_sample2D[i] = NULL;
			}
		}
	}
	for (int i = 0; i < 16; ++i)
	{
		if (m_sample3D[i])
		{
			int status = AIL_sample_status(m_sample3D[i]);
			if (status == SMP_DONE || status == SMP_STOPPED)
			{
				if (m_sample3DOwner[i] > 0)
				{
					if (m_sound[m_sample3DOwner[i]].sample3d == m_sample3D[i])
						m_sound[m_sample3DOwner[i]].sample3d = NULL;
					m_sample3DOwner[i] = 0;
				}
				AIL_stop_sample(m_sample3D[i]);
				AIL_release_sample_handle(m_sample3D[i]);
				m_sample3D[i] = NULL;
			}
		}
	}
}

HSAMPLE WMilesSoundSystem::GetAllocateSample(int handle, bool bLoop)
{
	unsigned int i;
	for (i = 0; i < 16; ++i)
	{
		if (!m_sample2D[i])
		{
			m_sample2D[i] = AIL_allocate_sample_handle(m_hDriver);
			break;
		}
		int status = AIL_sample_status(m_sample2D[i]);
		if (status == SMP_DONE || status == SMP_STOPPED)
			break;
	}
	if (i == 16 || !m_sample2D[i])
	{
		for (i = 0; i < 16; ++i)
		{
			if (m_sample2D[i] && !m_bSample2DLoop[i])
				break;
		}
		if (i == 16)
			return NULL;
	}
	if (m_sample2DOwner[i] > 0)
	{
		if (m_sound[m_sample2DOwner[i]].sample2d == m_sample2D[i])
			m_sound[m_sample2DOwner[i]].sample2d = NULL;
	}
	m_sample2DOwner[i] = handle;
	m_bSample2DLoop[i] = bLoop;
	m_sound[handle].sample2d = m_sample2D[i];
	return m_sample2D[i];
}

HSAMPLE WMilesSoundSystem::GetAllocateSample3D(int handle, bool bLoop)
{
	unsigned int i;
	for (i = 0; i < 16; ++i)
	{
		if (!m_sample3D[i])
		{
			m_sample3D[i] = AIL_allocate_sample_handle(m_hDriver);
			break;
		}
		int status = AIL_sample_status(m_sample3D[i]);
		if (status == SMP_DONE || status == SMP_STOPPED)
			break;
	}
	if (i == 16 || !m_sample3D[i])
	{
		for (i = 0; i < 16; ++i)
		{
			if (m_sample3D[i] && !m_bSample3DLoop[i])
				break;
		}
		if (i == 16)
			return NULL;
	}
	if (m_sample3DOwner[i] > 0)
	{
		if (m_sound[m_sample3DOwner[i]].sample3d == m_sample3D[i])
			m_sound[m_sample3DOwner[i]].sample3d = NULL;
	}
	m_sample3DOwner[i] = handle;
	m_bSample3DLoop[i] = bLoop;
	m_sound[handle].sample3d = m_sample3D[i];
	return m_sample3D[i];
}

unsigned int __stdcall WMilesSoundSystem::Open_callback(const char* filename,
	unsigned int* handle)
{
	MilesFileLock983 lock; // 983 00924980
	for (unsigned int i = 1; i < 16; ++i)
	{
		if (!g_files[i])
		{
			cFile* file = g_resrcmng->GetCFile(filename, 0xffff);
			if (!file)
				return 0;
			g_files[i] = file;
			g_offset[i] = -1;
			*handle = i;
			return 1;
		}
	}
	return 0;
}

void __stdcall WMilesSoundSystem::Close_callback(unsigned int handle)
{
	MilesFileLock983 lock; // 983 00924a50
	CloseCFile(g_files[handle]);
	g_files[handle] = NULL;
}

int __stdcall WMilesSoundSystem::Seek_callback(unsigned int handle, int offset,
	unsigned int type)
{
	MilesFileLock983 lock; // 983 00925481
	switch (type)
	{
	case AIL_FILE_SEEK_CURRENT:
		offset += g_files[handle]->Tell();
		break;
	case AIL_FILE_SEEK_END:
		offset += g_files[handle]->Length();
		break;
	}
	g_offset[handle] = offset;
	return offset;
}

unsigned int __stdcall WMilesSoundSystem::Read_callback(unsigned int handle,
	void* buffer, unsigned int bytes)
{
	MilesFileLock983 lock; // 983 00924ad0
	if (g_offset[handle] != -1)
	{
		g_files[handle]->Seek(g_offset[handle], 0);
		g_offset[handle] = -1;
	}
	return g_files[handle]->Read(buffer, bytes);
}
