#pragma once

enum
{
	LOAD_PUPPET = 0,
	LOAD_TEXTURE,
	LOAD_OVERLAY,
};

struct sParam
{
	char filename[32];
	void* result;
	int type;
	bool bComplete;
	unsigned long startTime;
};

class CThreadLoad : public WSingleton<CThreadLoad>
{
public:
	CThreadLoad();
	virtual ~CThreadLoad();

	void LoadPuppet(const char* filename);
	void LoadTexture(const char* filename);
	void LoadOverlay(const char* filename);
	void StopLoading();

private:
	void Load(const char* filename, int type);

	HANDLE m_hThread;
};
