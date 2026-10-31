#pragma once

#include <windows.h>
#include <vfw.h>

namespace ntg
{

	class VideoCompressor
	{
	public:
		VideoCompressor();
		virtual ~VideoCompressor();

		DWORD get_fcc_handler() const { return m_compvars.fccHandler; }

		bool init(const BITMAPINFO* format, int quality);
		void cleanup();
		bool start();
		void stop();
		void* compress(void* data, int* size);
		bool get_format(BITMAPINFO* format);

	protected:
		COMPVARS m_compvars;
		BITMAPINFO m_format;
	};

}
