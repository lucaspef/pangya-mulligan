#pragma once

#include <windows.h>

namespace _client
{
	namespace _private
	{
		template <class T>
		class CLock
		{
		public:
			CLock(T& cs) : m_cs(cs)
			{
				m_cs.lock();
			}
			~CLock()
			{
				m_cs.unlock();
			}

		private:
			T& m_cs;
		};
	}




	class CCriticalSection
	{
	public:
		CCriticalSection() { InitializeCriticalSection(&m_cs); }

		virtual ~CCriticalSection() { DeleteCriticalSection(&m_cs); }

		void lock() { EnterCriticalSection(&m_cs); }
		void unlock() { LeaveCriticalSection(&m_cs); }

	private:
		CRITICAL_SECTION m_cs;
	};
}
