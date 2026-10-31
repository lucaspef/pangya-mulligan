#pragma once

#include "parallelwork.h"

namespace Py
{
	namespace Threading
	{
		namespace ProcessPriority
		{
			enum Enum
			{
				Idle = 0x40,
				Normal = 0x20,
				High = 0x80,
				RealTime = 0x100,
			};
		}

		namespace ThreadPriority
		{
			enum Enum
			{
				Idle = -15,
				Lowest = -2,
				BelowNormal = -1,
				Normal = 0,
				AboveNormal = 1,
				Highest = 2,
				TimeCritical = 15,
			};
		}

		namespace ThreadPoolPolicy
		{
			enum Enum
			{
				MultiThread = 0,
				Iocp = 1,
				SingleThread = 2,
				Unimplemented = 3,
			};
		}

		class ThreadPoolImpl;

		class ThreadPool
		{
		public:
			ThreadPool(ThreadPoolPolicy::Enum policy, unsigned int threadCount);
			~ThreadPool();

			static int __fastcall SetProcessPriority(
				ProcessPriority::Enum priority);
			static ProcessPriority::Enum __fastcall GetProcessPriority();

			unsigned int GetThreadCount() const;
			unsigned int GetInvokedWorkCount() const;
			ThreadPriority::Enum GetThreadPriority() const;
			int SetThreadPriority(ThreadPriority::Enum priority);
			int WaitForCompletionAll(unsigned long timeout);

		private:
			void InternalInvokeParallelWork(ParallelWork* work,
				void (ParallelWork::*callback)());

			ThreadPoolImpl* m_implementation;
		};
	}
}
