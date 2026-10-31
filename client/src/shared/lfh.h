#pragma once

#include <windows.h>

class LFHInstaller
{
public:
	typedef unsigned long(__stdcall* PFN_GETPROCESSHEAPS)(
		unsigned long numberOfHeaps, void** processHeaps);
	typedef int(__stdcall* PFN_HEAPQUERYINFORMATION)(void* heapHandle,
		int heapInformationClass, void* heapInformation,
		unsigned long heapInformationLength, unsigned long* returnLength);
	typedef int(__stdcall* PFN_HEAPSETINFORMATION)(void* heapHandle,
		int heapInformationClass, void* heapInformation,
		unsigned long heapInformationLength);
	typedef int(__stdcall* PFN_ISDEBUGGERPRESENT)();

	struct _Handles
	{
		_Handles(unsigned long count, PFN_GETPROCESSHEAPS pfnGetProcessHeaps)
			: m_handles(new void*[count])
		{
			pfnGetProcessHeaps(count, m_handles);
		}

		~_Handles() { delete[] m_handles; }

		void* operator[](unsigned int index) { return m_handles[index]; }

		void** m_handles;
	};

	struct _Modules
	{
		HINSTANCE m_hKernel32;
		PFN_GETPROCESSHEAPS m_pfnGetProcessHeaps;
		PFN_HEAPQUERYINFORMATION m_pfnHeapQueryInformation;
		PFN_HEAPSETINFORMATION m_pfnHeapSetInformation;
		PFN_ISDEBUGGERPRESENT m_pfnIsDebuggerPresent;

		_Modules()
		{
			m_hKernel32 = LoadLibraryA("Kernel32.dll");

			if (m_hKernel32)
			{
				m_pfnGetProcessHeaps = (PFN_GETPROCESSHEAPS)GetProcAddress(
					m_hKernel32, "GetProcessHeaps");
				m_pfnHeapQueryInformation =
					(PFN_HEAPQUERYINFORMATION)GetProcAddress(m_hKernel32,
						"HeapQueryInformation");
				m_pfnHeapSetInformation =
					(PFN_HEAPSETINFORMATION)GetProcAddress(m_hKernel32,
						"HeapSetInformation");
				m_pfnIsDebuggerPresent = (PFN_ISDEBUGGERPRESENT)GetProcAddress(
					m_hKernel32, "IsDebuggerPresent");
			}
		}

		~_Modules()
		{
			if (m_hKernel32)
				FreeLibrary(m_hKernel32);
		}

		bool IsValid() const
		{
			if (m_hKernel32 == NULL)
				return false;

			return m_pfnGetProcessHeaps && m_pfnHeapQueryInformation &&
				m_pfnHeapSetInformation && m_pfnIsDebuggerPresent;
		}

		bool InstallLFH()
		{
			if (!IsValid())
				return false;

			if (m_pfnIsDebuggerPresent())
				return true;

			unsigned long numHeaps = m_pfnGetProcessHeaps(0, NULL);

			if (numHeaps == 0)
			{
				return false;
			}

			_Handles heaps(numHeaps, m_pfnGetProcessHeaps);

			for (unsigned long i = 0; i < numHeaps; ++i)
			{
				unsigned long info, len;
				if (!m_pfnHeapQueryInformation(heaps[i], 0, &info, sizeof(info),
						&len))
					return false;

				if (info == 1)
				{
					info = 2;
					if (!m_pfnHeapSetInformation(heaps[i], 0, &info,
							sizeof(info)))
						return false;
				}
			}

			return true;
		}
	};

	static bool install()
	{
		_Modules modules;

		return modules.InstallLFH();
	}
};
