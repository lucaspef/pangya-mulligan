#pragma once

#include <queue>
#include <windows.h>
#include "aviwriter.h"
#include "videoin.h"
#include "vcomp.h"

namespace ntg
{

	class VideoIn;
	class Writer;
	class Compressor;
	class AudioIn;

	class AviRecorder
	{
	public:
		AviRecorder();
		virtual ~AviRecorder();

		bool init(void* hwnd);
		void cleanup();
		bool start(const char* filename, int quality, int scale, int fps);
		void stop();
		void record();
		void unit_test();

	protected:
		void* m_hwnd;
		VideoIn* m_video_in;
		AudioIn* m_audio_in;
		Compressor* m_compressor;
		Writer* m_writer;
		int m_fps;
		int m_scale;
		bool m_recording;
		bool m_first_frame;
		unsigned long m_last_time;
	};

}
