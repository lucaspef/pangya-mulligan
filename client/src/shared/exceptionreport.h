#pragma once

#define EXCEPTION_WITH_USER_INFO 0xe0000001

long __stdcall RecordExceptionInfo(EXCEPTION_POINTERS* data);
void __cdecl SecurityErrorHandler(int code, void* unused);

class CExceptionReport
{
	friend long __stdcall RecordExceptionInfo(EXCEPTION_POINTERS* data);
	friend void __cdecl SecurityErrorHandler(int code, void* unused);

private:
	void Init();
	void Shutdown();

	void StartLog(const char* filename);
	void LogPrintf(const char* fmt, ...) const;
	void EndLog();

	void CollectSystemInfo();
	void DumpHeaderInfo(EXCEPTION_POINTERS* pExceptionInfo);
	void EndStackLog();
	void DumpHeader(EXCEPTION_POINTERS* data) const;
	void DumpSystemInfo() const;
	void DumpErrorMessage(const CONTEXT* contextRecord,
		const EXCEPTION_RECORD* exceptionRecord) const;
	void DumpRegisters(const CONTEXT* contextRecord) const;
	void PrintStack(unsigned long begin, unsigned long end) const;
	void IntelStackWalk(CONTEXT* ptrContext) const;
	void ImageHelpStackWalk(CONTEXT* ptrContext) const;
	void DumpStackTrace(const CONTEXT* contextRecord) const;
	void DumpMemory(const CONTEXT* contextRecord) const;
	void DumpExceptionReport(EXCEPTION_POINTERS* data);

	bool m_initialized;
	HANDLE m_logFileHandle;
	long m_enable;
	char m_crashTime[200];
	char m_osName[200];
	char m_userName[200];
	char m_computerName[200];
	SYSTEM_INFO m_systemInfo;
	MEMORYSTATUS m_memInfo;

	static __declspec(thread) HANDLE m_curThread;
	static __declspec(thread) unsigned long m_curThreadId;

public:
	static void SetThreadInfo(HANDLE hThread, unsigned long dwThreadId);
	static HANDLE GetCurrentThread();
	static unsigned long GetCurrentThreadId();

	CExceptionReport() { Init(); }

	~CExceptionReport() { Shutdown(); }

	int Enable();
	int Disable();
	int IsEnabled() const;

	void Crash() const;
};

extern CExceptionReport g_exceptionReport;

#pragma optimize("", off)
inline void CExceptionReport::Crash() const
{
	*(unsigned long*)0xdeaddead = 0xdeaddead;
}
#pragma optimize("", on)
