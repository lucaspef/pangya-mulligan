#pragma once

#include <vector>
#include <windows.h>
#include <vfw.h>

namespace ntg
{

	class AviWriter
	{
	public:
		AviWriter();
		virtual ~AviWriter();

		bool open(const char* filename, const tagBITMAPINFOHEADER* bih,
			int quality, int fps);
		void close();
		void write_video(int frames, void* data, int size);
		void write_audio(void* data, int size);
		void write_audio_sync(void* data, int size);
		void set_comp_format(const tagBITMAPINFO* format);
		void set_fcc_handler(unsigned long fcc) { m_fcc_handler = fcc; }

	protected:
		void write_header();
		void write_indexes();
		void reserve_idxcache();
		void index(MMCKINFO* ck, unsigned long flags);

		tagBITMAPINFOHEADER m_bih;
		int m_quality;
		int m_fps;
		unsigned long m_fcc_handler;
		tagBITMAPINFO m_comp_format;
		WAVEFORMAT m_wave_format;
		int m_video_frames;
		int m_video_max_size;
		int m_audio_chunks;
		int m_audio_max_size;
		HMMIO m_hmmio;
		std::vector<AVIINDEXENTRY> m_index;
	};

}
