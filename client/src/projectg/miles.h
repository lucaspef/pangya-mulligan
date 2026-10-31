#pragma once

struct _DIG_DRIVER;
struct _SAMPLE;
struct _STREAM;
typedef _DIG_DRIVER* HDIGDRIVER;
typedef _SAMPLE* HSAMPLE;
typedef _STREAM* HSTREAM;

class WMilesSoundSystem
{
public:
	enum w_speaker_type
	{
		W_SPEAKER_2CH,
		W_SPEAKER_HEADPHONE,
		W_SPEAKER_SURROUND,
		W_SPEAKER_4CH,
		W_SPEAKER_51CH,
		W_SPEAKER_71CH
	};

	struct msample2d
	{
		void* data;
		int size;
		int refCount;
		int idleCount;
		int lastHandle;
		char name[1];
	};

	struct msample3d
	{
		void* data;
		int refCount;
		int idleCount;
		int lastHandle;
		char name[1];
	};

	void SetDistance(float nearDist, float farDist)
	{
		m_nearDistance = nearDist;
		m_farDistance = farDist;
	}

	WMilesSoundSystem(HWND__* hWnd, const char* redistDir, bool bOption,
		int param1, int param2, int param3);
	virtual ~WMilesSoundSystem();

	bool Init(HWND__* hWnd, int rate, int bits);
	void Activate(bool bActive);
	static void Process();

	int Load(char* filename, int type);
	void Play(int handle, int loop, bool bReplace);
	void Stop(int handle);
	void Resume(int handle, int loop);
	void DestroySoundBuffer(int handle);
	bool IsPlaying(int handle);
	int GetPosition(int handle);
	int GetLength(int handle);

	void SetVolume(int handle, float volume);
	void SetPitch(int handle, int rate);
	int GetPitch(int handle);
	void SetPosition(int handle, float x, float y, float z);
	void SetNearFar(int handle, float nearDist, float farDist);

	void Volume(float volume);
	void SetBalance(int balance);
	void SpeakerType(w_speaker_type type);
	int GetCPUpercent();

	void ClearIdleSample();
	void ClearIdleSound(int age);
	void ClearAllSamples();
	void ClearAllSounds();
	void ClearAllStreams();

protected:
	bool InitMixer();
	void SetMixer(float volume);
	void CloseMixer();

	int GetEmptyHandle();
	_SAMPLE* GetAllocateSample(int handle, bool bLoop);
	_SAMPLE* GetAllocateSample3D(int handle, bool bLoop);
	msample2d* LoadSample2D(char* filename);
	msample3d* LoadSample3D(char* filename);
	static void LoadSample3D(msample3d* sample);
	void Release2D(msample2d* sample);
	void Release3D(msample3d* sample);

	static unsigned int __stdcall Open_callback(const char* filename,
		unsigned int* handle);
	static void __stdcall Close_callback(unsigned int handle);
	static unsigned int __stdcall Read_callback(unsigned int handle,
		void* buffer, unsigned int bytes);
	static int __stdcall Seek_callback(unsigned int handle, int offset,
		unsigned int type);

	static int g_offset[16];
	static cFile* g_files[16];

	struct msound
	{
		HSAMPLE sample2d;
		msample2d* data2d;
		HSAMPLE sample3d;
		msample3d* data3d;
		HSTREAM stream;
		int streamPos;
		float volume;
		float pos[3];
		float nearDistance;
		float farDistance;
		int type;
	};

	float m_pad983_04; // 983 ctor 00925dd0 grava 1.0f; nenhum leitor no 983 (membros protegidos, unidade 00923b40..00926850)
	float m_nearDistance;
	float m_farDistance;
	float m_volume2D;
	float m_volume3D;
	bool m_bFlipX;
	bool m_pad983_19; // 983 ctor 00925db3 zera junto com m_bFlipX (word +0x18); nenhum leitor no 983
	bool m_bAllowOverlap;
	HDIGDRIVER m_hDriver;
	WList<msample2d*> m_sample2DList;
	HSAMPLE m_sample2D[16];
	int m_sample2DOwner[16];
	bool m_bSample2DLoop[16];
	WList<msample3d*> m_sample3DList;
	HSAMPLE m_sample3D[16];
	int m_sample3DOwner[16];
	bool m_bSample3DLoop[16];
	HSTREAM m_stream[8];
	int m_streamOwner[8];
	msound m_sound[1024];
	int m_lastHandle;
	HMIXER m_hMixer;
	MIXERCONTROL m_mixerControl;
	MIXERLINE m_mixerLine;
};
