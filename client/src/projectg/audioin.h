#pragma once

#include <windows.h>
#include <mmsystem.h>

namespace ntg
{

	class AudioIn
	{
	public:
		AudioIn();
		virtual ~AudioIn();

		bool init();
		void cleanup();
		bool start(DWORD thread_id);
		void stop();

	protected:
		void thread_func();
		static DWORD WINAPI static_thread_func(LPVOID param);

		HWAVEIN m_hwavein;
		WAVEFORMATEX m_format;
		WAVEHDR m_header[4];
		DWORD m_buffer_size;
		DWORD m_target_thread_id;
		HANDLE m_thread;
		DWORD m_thread_id;
	};

}
