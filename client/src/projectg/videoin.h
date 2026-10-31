#pragma once
#include <windows.h>

namespace ntg
{
	class VideoFrame
	{
	public:
		VideoFrame()
			: m_data(NULL), m_used(0)
		{
		}
		virtual ~VideoFrame()
		{
			if (m_data)
				delete[] m_data;
		}
		unsigned char* m_data;
		int m_frames;
		int m_used;
	};

	class VideoIn
	{
	public:
		virtual ~VideoIn();
		virtual bool init(void* hwnd) = 0;
		virtual void cleanup() = 0;
		virtual void capture() = 0;
		virtual VideoFrame* get_frame() = 0;
		virtual const BITMAPINFOHEADER* get_format() = 0;
	};
}
