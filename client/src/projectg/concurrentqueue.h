#pragma once

#include <queue>
#include "parallellock.h"

namespace Py
{
	namespace Threading
	{
		template <typename T, typename Container>
		class ConcurrentQueue
		{
		public:
			ConcurrentQueue() { }
			~ConcurrentQueue() { }

			bool Empty() const { return m_queue.empty(); }

			void Push(const T& value)
			{
				SingleLock lock(&m_lock);
				m_queue.push(value);
			}

			bool Pop(T& value)
			{
				SingleLock lock(&m_lock);
				if (!m_queue.empty())
				{
					T front = m_queue.front();
					m_queue.pop();
					value = front;
					return true;
				}
				return false;
			}

		private:
			std::queue<T, Container> m_queue;
			CriticalSection m_lock;
		};
	}
}
