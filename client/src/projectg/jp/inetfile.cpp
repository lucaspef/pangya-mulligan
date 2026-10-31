// JP 983: unidade 00a4c160–00a4cc70 — o chttp.cpp do 983 (00a4c140 e antes = objectfactory; 00a4cc70 = WClientSocket). O
// `cHttp` do 645 (src/projectg/chttp.cpp) sumiu: três classes WinINet num namespace anônimo (RTTI .?AVCINet@?A0xa62399dc@@,
// CINetFile, CINetBuffer) + as funções soltas de include/jp/inetfile.h. Nomes de método meus (sessão B117).
//
// Curiosidade do 983: a vtable que o construtor da CINetBuffer grava (00d41ba4, COL "CINetBuffer") tem o destrutor e o Run
// da CINetFile, e vice-versa (00d41c40); as chamadas diretas usam as funções certas e o Run nunca é chamado pela vtable.
#include "minatl.h"
#include <wininet.h>
#include "inetfile.h"

#pragma comment(lib, "wininet.lib")

static __declspec(thread) unsigned long s_httpLastError983;

namespace
{
	class CINet
	{
	public:
		CINet() // 00a4c670 (cópia fora de linha sem chamador)
			: m_hInternet(NULL),
			  m_hUrl(NULL),
			  m_headers("Pragma: no-cache\r\nCache-Control: no-cache\r\n")
		{
		}
		// 00a4c3f0 (fora de linha para o EH); deleting 00a4c440. N�o chama o Close: zera na ordem inversa
		virtual ~CINet()
		{
			if (m_hUrl)
				InternetCloseHandle(m_hUrl);
			if (m_hInternet)
				InternetCloseHandle(m_hInternet);
			m_hUrl = NULL;
			m_hInternet = NULL;
		}

		virtual bool Run(const std::string& url) = 0;
		virtual void Close() // 00a4c160
		{
			if (m_hUrl)
				InternetCloseHandle(m_hUrl);
			if (m_hInternet)
				InternetCloseHandle(m_hInternet);
			m_hInternet = NULL;
			m_hUrl = NULL;
		}

		int Open(const std::string& url);

	protected:
		HINTERNET m_hInternet; // +0x04
		HINTERNET m_hUrl; // +0x08
		std::string m_headers; // +0x0c
	};

	// 0x44 bytes
	class CINetFile : public CINet
	{
	public:
		CINetFile(const std::string& path);

		virtual bool Run(const std::string& url);

		bool Save();

	private:
		std::string m_path; // +0x28
	};

	// 0x44 bytes
	class CINetBuffer : public CINet
	{
	public:
		CINetBuffer() // 00a4c6a0 (cópia fora de linha sem chamador)
		{
		}

		virtual bool Run(const std::string& url);

		std::string m_buffer; // +0x28
	};
}

// 00a4c1a0
unsigned long HttpGetLastError983()
{
	return s_httpLastError983;
}

// 00a4c1c0: abre a URL sem cache e devolve o código HTTP (400 se não abriu)
int CINet::Open(const std::string& url)
{
	Close();
	DeleteUrlCacheEntryA(url.c_str());

	DWORD status = 400;
	m_hInternet = InternetOpenA("cHttp DownLoad", 0, NULL, NULL, 0);
	if (m_hInternet)
	{
		m_hUrl = InternetOpenUrlA(m_hInternet, url.c_str(), m_headers.c_str(), m_headers.size(),
			INTERNET_FLAG_NO_CACHE_WRITE, 0);
		if (m_hUrl)
		{
			DWORD size = sizeof(status);
			HttpQueryInfoA(m_hUrl, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &size, NULL);
		}
	}
	return status;
}

// 00a4c270: grava o corpo no arquivo e põe nele a data Last-Modified do servidor
bool CINetFile::Save()
{
	HANDLE hFile = CreateFileA(m_path.c_str(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	__try
	{
		char buffer[0x2000];
		DWORD size;
		for (;;)
		{
			size = 0;
			if (!InternetReadFile(m_hUrl, buffer, sizeof(buffer), &size))
				return false;

			if (size == 0)
			{
				SYSTEMTIME st;
				size = sizeof(st);
				if (HttpQueryInfoA(m_hUrl, HTTP_QUERY_LAST_MODIFIED | HTTP_QUERY_FLAG_SYSTEMTIME, &st, &size, NULL))
				{
					FILETIME ft;
					if (SystemTimeToFileTime(&st, &ft))
						SetFileTime(hFile, NULL, NULL, &ft);
				}
				return true;
			}

			DWORD written;
			if (!WriteFile(hFile, buffer, size, &written, NULL))
				return false;
		}
	}
	__finally
	{
		CloseHandle(hFile);
	}
}

// 00a4c4b0: 304 = o arquivo já está em dia
bool CINetFile::Run(const std::string& url)
{
	int status = Open(url);
	if (status == 304)
		return true;
	if (status == 200)
		return Save();
	return false;
}

// 00a4c5d0: lê o corpo inteiro para m_buffer (repete a leitura que falhar)
bool CINetBuffer::Run(const std::string& url)
{
	if (Open(url) != 200)
		return false;

	char buffer[0x2000];
	for (;;)
	{
		DWORD size = 0;
		if (!InternetReadFile(m_hUrl, buffer, sizeof(buffer), &size))
			continue;
		if (size == 0)
			return true;
		m_buffer.append(buffer, size);
	}
}

// 00a4c720: pede só se mudou (If-Modified-Since com a data do arquivo local) e tira o somente-leitura
CINetFile::CINetFile(const std::string& path)
	: m_path(path)
{
	WIN32_FIND_DATAA fd;
	HANDLE hFind = FindFirstFileA(m_path.c_str(), &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return;
	FindClose(hFind);

	SYSTEMTIME st;
	if (FileTimeToSystemTime(&fd.ftLastWriteTime, &st))
	{
		const char* months[] = { "YYY", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov",
			"Dec" };
		const char* days[] = { "DDD", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
		char header[128];
		sprintf(header, "If-Modified-Since: %s, %02d %s %d %02d:%02d:%02d GMT\r\n", days[st.wDayOfWeek], st.wDay,
			months[st.wMonth], st.wYear, st.wHour, st.wMinute, st.wSecond);
		m_headers.append(header, strlen(header));
	}

	if (fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
		SetFileAttributesA(m_path.c_str(), fd.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
}

// 00a4c9d0
bool __fastcall HttpDownloadFile983(const std::string& url, const std::string& path)
{
	CINetFile net(path);
	int status = net.Open(url);
	if (status == 304 || (status == 200 && net.Save()))
		return true;

	// (sic) o 983 regrava a variável com ela mesma: o fonte devia querer o GetLastError do Windows
	s_httpLastError983 = HttpGetLastError983();
	return false;
}

// 00a4ca80
bool __fastcall HttpDownloadFileTo983(const std::string& name, const std::string& urlName, const std::string& dir,
	const std::string& baseUrl)
{
	return HttpDownloadFile983(baseUrl + urlName, dir + name);
}

// 00a4cb40
std::string __fastcall HttpDownloadText983(const std::string& file, const std::string& url)
{
	CINetBuffer net;
	if (!net.Run(url + file))
	{
		s_httpLastError983 = HttpGetLastError983();
		return "";
	}
	return net.m_buffer;
}
