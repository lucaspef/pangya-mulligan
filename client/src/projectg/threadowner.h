#pragma once

namespace Py
{
	namespace Threading
	{
		class ThreadOwner
		{
		public:
			void SetThreadName(const wchar_t* name);
			bool StartThread(unsigned long interval);
			bool PauseThread();
			bool ResumeThread();
			void FinishEvent();
			void TerminateThread();
			void FinishThread();

		protected:
			ThreadOwner(const wchar_t* name);
			virtual ~ThreadOwner();

			virtual bool OnThreadProc() { return false; }
			virtual void OnUpdateThreadProc() { }

		private:
			static unsigned int __stdcall ThreadProc(void* pParam);

			HANDLE m_hThread;
			HANDLE m_hFinishEvent;
			unsigned long m_interval;
			wchar_t m_name[32];
		};
	}
}
