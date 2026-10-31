#pragma once

namespace Py
{
	namespace Threading
	{

		namespace ParallelWorkPolicy
		{
			enum Enum
			{
				CompleteAll = 0,
				CancelPending = 1,
			};
		}

		class ParallelWork
		{
		protected:
			ParallelWork(ParallelWorkPolicy::Enum policy);
			virtual ~ParallelWork();

		public:
			bool IsWorkingFirst() const;
			long IncrementLockCount();
			long DecrementLockCount();

		private:
			volatile long m_lockCount;
			volatile long m_workingFirst;
			ParallelWorkPolicy::Enum m_policy;
		};

	}
}
