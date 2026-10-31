#pragma once

#include <windows.h>

namespace Py
{
	namespace Threading
	{
		class __declspec(novtable) IParallelLock
		{
		public:
			IParallelLock() { }
			virtual void Lock() = 0;
			virtual void Unlock() = 0;
		};

		class SingleLock
		{
		public:
			SingleLock(IParallelLock* pLock)
				: m_pLock(pLock)
			{
				m_pLock->Lock();
			}

			~SingleLock() { m_pLock->Unlock(); }

		private:
			IParallelLock* m_pLock;
		};

		class CriticalSection : public IParallelLock
		{
		public:
			CriticalSection() { InitializeCriticalSection(&m_cs); }
			~CriticalSection() { DeleteCriticalSection(&m_cs); }
			virtual void Lock() { EnterCriticalSection(&m_cs); }
			virtual void Unlock() { LeaveCriticalSection(&m_cs); }

		private:
			CRITICAL_SECTION m_cs;
		};
	}
}
