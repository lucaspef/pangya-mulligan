// SPDX-License-Identifier: MIT
// audio.cpp - see the header.
#include "audio.h"
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <string.h>
#include <list>
#include <map>
#include <vector>
#include "pmpfile.h"

struct WavData
{
	std::vector<unsigned char> format;
	std::vector<unsigned char> samples;
	bool ok;
};

struct Voice
{
	HWAVEOUT handle;
	WAVEHDR header;
	std::vector<unsigned char> quieter;		// the samples, when they play below full volume
};

static std::map<std::string, WavData*> g_cache;
static std::list<Voice*> g_voices;
static bool g_muted;
static bool g_levelRestored;
static const size_t MAX_VOICES = 32;

static WavData* LoadWav(const std::string& path)
{
	std::map<std::string, WavData*>::iterator it = g_cache.find(path);
	if (it != g_cache.end())
		return it->second;
	WavData* wav = new WavData;
	wav->ok = false;
	g_cache[path] = wav;
	FILE* f = OpenUtf8(path.c_str(), L"rb");
	if (!f)
		return wav;
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	std::vector<unsigned char> d(size);
	if (size)
		fread(&d[0], 1, size, f);
	fclose(f);
	if (size < 12 || memcmp(&d[0], "RIFF", 4) || memcmp(&d[8], "WAVE", 4))
		return wav;
	size_t p = 12;
	while (p + 8 <= (size_t)size)
	{
		unsigned int len;
		memcpy(&len, &d[p + 4], 4);
		size_t body = p + 8;
		size_t avail = (size_t)size - body;
		size_t use = len < avail ? len : avail;
		if (!memcmp(&d[p], "fmt ", 4))
		{
			wav->format.assign(d.begin() + body, d.begin() + body + use);
			// WAVEFORMATEX wants the cbSize field even when a PCM file leaves it out.
			if (wav->format.size() < sizeof(WAVEFORMATEX))
				wav->format.resize(sizeof(WAVEFORMATEX), 0);
		}
		else if (!memcmp(&d[p], "data", 4))
			wav->samples.assign(d.begin() + body, d.begin() + body + use);
		p = body + len + (len & 1);
	}
	wav->ok = !wav->format.empty() && !wav->samples.empty();
	return wav;
}

static void FreeVoice(Voice* v)
{
	waveOutReset(v->handle);
	waveOutUnprepareHeader(v->handle, &v->header, sizeof(v->header));
	waveOutClose(v->handle);
	delete v;
}

// Distance volume is applied to a copy of the samples. waveOutSetVolume would set the level
// of the whole program, and Windows remembers that level for the executable between runs.
// Compressed formats can't be scaled this way and play at full volume.
static void ScaleSamples(const WavData& wav, float volume, std::vector<unsigned char>* out)
{
	const WAVEFORMATEX* f = (const WAVEFORMATEX*)&wav.format[0];
	if (f->wFormatTag != WAVE_FORMAT_PCM || (f->wBitsPerSample != 8 && f->wBitsPerSample != 16))
		return;
	*out = wav.samples;
	if (f->wBitsPerSample == 8)
	{
		for (size_t i = 0; i < out->size(); ++i)
			(*out)[i] = (unsigned char)(128 + (int)(((int)(*out)[i] - 128) * volume));
		return;
	}
	short* s = (short*)&(*out)[0];
	for (size_t i = 0; i < out->size() / 2; ++i)
		s[i] = (short)(s[i] * volume);
}

void AudioUpdate()
{
	for (std::list<Voice*>::iterator i = g_voices.begin(); i != g_voices.end();)
	{
		if ((*i)->header.dwFlags & WHDR_DONE)
		{
			FreeVoice(*i);
			i = g_voices.erase(i);
		}
		else
			++i;
	}
}

void AudioStopAll()
{
	for (std::list<Voice*>::iterator i = g_voices.begin(); i != g_voices.end(); ++i)
		FreeVoice(*i);
	g_voices.clear();
}

void AudioSetMuted(bool muted)
{
	g_muted = muted;
	if (muted)
		AudioStopAll();
}

int AudioVoices()
{
	return (int)g_voices.size();
}

bool AudioCanPlay(const std::string& path)
{
	WavData* wav = LoadWav(path);
	if (!wav->ok)
		return false;
	return waveOutOpen(0, WAVE_MAPPER, (const WAVEFORMATEX*)&wav->format[0], 0, 0, WAVE_FORMAT_QUERY) == MMSYSERR_NOERROR;
}

bool AudioPlay(const std::string& path, float volume)
{
	if (g_muted || volume <= 0.0f)
		return true;
	AudioUpdate();
	if (g_voices.size() >= MAX_VOICES)
		return true;
	WavData* wav = LoadWav(path);
	if (!wav->ok)
		return false;
	Voice* v = new Voice;
	if (waveOutOpen(&v->handle, WAVE_MAPPER, (const WAVEFORMATEX*)&wav->format[0], 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
	{
		delete v;
		return false;
	}
	memset(&v->header, 0, sizeof(v->header));
	if (volume < 1.0f)
		ScaleSamples(*wav, volume, &v->quieter);
	const std::vector<unsigned char>& samples = v->quieter.empty() ? wav->samples : v->quieter;
	v->header.lpData = (LPSTR)&samples[0];
	v->header.dwBufferLength = (DWORD)samples.size();
	waveOutPrepareHeader(v->handle, &v->header, sizeof(v->header));
	// Earlier builds could leave a very low level saved for this executable.
	if (!g_levelRestored)
	{
		g_levelRestored = true;
		waveOutSetVolume(v->handle, 0xffffffff);
	}
	waveOutWrite(v->handle, &v->header, sizeof(v->header));
	g_voices.push_back(v);
	return true;
}
