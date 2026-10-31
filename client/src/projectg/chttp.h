#pragma once

#include <stdio.h>
#include <WinInet.h>

class cHttp
{
public:
	cHttp();
	~cHttp();

	bool Init(HWND__* hWnd);
	bool Open(const char* url);
	void Close();

	bool DownLoad(const char* filename, const char* urlFile,
		const char* localPath, const char* url);
	bool DownLoad2(const char* filename, const char* urlFile,
		const char* localPath, const char* url, unsigned long fileSize);
	char* DownLoad(const char* file, const char* url, unsigned long& size);

	template <typename T>
	T* DownLoad(const char* file, const char* url, unsigned long& size)
	{
		char szURL[260];
		T buffer[10240];
		unsigned long dwSize;
		unsigned long dwRead;

		T* pData = new T[512000];
		T* pPos = pData;

		Close();

		sprintf(szURL, "%s%s", url, file);

		m_hSession = InternetOpenA("cHttp DownLoad", 0, NULL, NULL, 0);
		if (m_hSession == NULL)
		{
			delete[] pData;
			return NULL;
		}

		m_hUrl = InternetOpenUrlA(m_hSession, szURL, NULL, 0, 0x04000000, 0);
		if (m_hUrl == NULL)
		{
			InternetCloseHandle(m_hSession);

			delete[] pData;
			return NULL;
		}

		unsigned long dwTotal = 0;
		do
		{
			if (!InternetQueryDataAvailable(m_hUrl, &dwSize, 0, 0))
			{
				break;
			}

			if (dwSize <= 0)
				break;
			if (dwSize > 10240)
				dwSize = 10240;

			if (!InternetReadFile(m_hUrl, buffer, dwSize, &dwRead))
			{
				break;
			}

			memcpy(pPos, buffer, dwRead);
			pPos += dwRead;

			dwTotal += dwRead;
			if (dwTotal > 512000)
			{
				OutputDebugStringA(
					"memory buffer is overflowed, require more memory.\n");
				break;
			}

		} while (dwRead != 0);

		Close();

		size = dwTotal;

		return pData;
	}

protected:
	void SendLog(unsigned int msg, long lParam);
	unsigned long GetFileSize(const char* filename);

	HINTERNET m_hSession;
	HINTERNET m_hUrl;
	HWND__* m_hWnd;
	char m_szLog[260];
	unsigned long m_dwDownloaded;
};
