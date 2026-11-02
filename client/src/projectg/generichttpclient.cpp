#include "minatl.h"
#include "generichttpclient.h"

#include <tchar.h>

#define __SIZE_HTTP_HEAD_LINE 2048
#define __SIZE_BUFFER 1024
#define __SIZE_HTTP_BUFFER 100000

#define __HTTP_VERB_GET "GET"
#define __HTTP_VERB_POST "POST"
#define __HTTP_ACCEPT_TYPE "*/*"
#define __HTTP_ACCEPT "Accept: */*\r\n"

void PrintLastError(unsigned long dwError)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	LocalFree(lpMsgBuf);
}

GenericHTTPClient::GenericHTTPClient()
{
	_hHTTPOpen = _hHTTPConnection = _hHTTPRequest = NULL;
	_dwSecureFlag = 0;
}

GenericHTTPClient::~GenericHTTPClient()
{
	_hHTTPOpen = _hHTTPConnection = _hHTTPRequest = NULL;
}

GenericHTTPClient::enumRequestMethod GenericHTTPClient::GetMethod(int nMethod)
{
	return nMethod <= 0
		? GenericHTTPClient::RequestGetMethod
		: static_cast<GenericHTTPClient::enumRequestMethod>(nMethod);
}

GenericHTTPClient::enumTypePostArgument GenericHTTPClient::GetPostArgumentType(
	int nType)
{
	return nType <= 0
		? GenericHTTPClient::TypeNormal
		: static_cast<GenericHTTPClient::enumTypePostArgument>(nType);
}

int GenericHTTPClient::Connect(const char* szAddress, const char* szAgent,
	unsigned short nPort, const char* szUserAccount, const char* szPassword)
{
	_hHTTPOpen =
		::InternetOpen(szAgent, INTERNET_OPEN_TYPE_PRECONFIG, "", "", 0);

	if (!_hHTTPOpen)
	{
		_dwError = ::GetLastError();
		return FALSE;
	}

	_hHTTPConnection = ::InternetConnect(_hHTTPOpen, szAddress, nPort,
		szUserAccount, szPassword, INTERNET_SERVICE_HTTP,
		INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE, 0);

	if (!_hHTTPConnection)
	{
		_dwError = ::GetLastError();
		::CloseHandle(_hHTTPOpen);
		return FALSE;
	}

	if (::InternetAttemptConnect(NULL) != ERROR_SUCCESS)
	{
		_dwError = ::GetLastError();
		::CloseHandle(_hHTTPConnection);
		::CloseHandle(_hHTTPOpen);
		return FALSE;
	}

	return TRUE;
}

int GenericHTTPClient::Close()
{
	if (_hHTTPRequest)
		::InternetCloseHandle(_hHTTPRequest);

	if (_hHTTPConnection)
		::InternetCloseHandle(_hHTTPConnection);

	if (_hHTTPOpen)
		::InternetCloseHandle(_hHTTPOpen);

	return TRUE;
}

int GenericHTTPClient::Request(const char* szURL, int nMethod,
	const char* szAgent)
{
	BOOL bReturn = TRUE;
	DWORD dwPort = 0;
	TCHAR szProtocol[__SIZE_BUFFER] = "";
	TCHAR szAddress[__SIZE_BUFFER] = "";
	TCHAR szURI[__SIZE_BUFFER] = "";
	DWORD dwSize = 0;

	ParseURL(szURL, szProtocol, szAddress, dwPort, szURI);

	_dwSecureFlag =
		dwPort == INTERNET_DEFAULT_HTTPS_PORT ? INTERNET_FLAG_SECURE : 0;

	if (Connect(szAddress, szAgent, (unsigned short)dwPort, NULL, NULL))
	{
		if (!RequestOfURI(szURI, nMethod))
		{
			bReturn = FALSE;
		}
		else
		{
			if (!Response((PBYTE)_szHTTPResponseHeader, __SIZE_HTTP_BUFFER,
					(PBYTE)_szHTTPResponseHTML, __SIZE_HTTP_BUFFER, dwSize))
			{
				bReturn = FALSE;
			}
		}
		Close();
	}
	else
	{
		bReturn = FALSE;
	}

	return bReturn;
}

int GenericHTTPClient::RequestOfURI(const char* szURI, int nMethod)
{
	BOOL bReturn = TRUE;

	try
	{
		switch (nMethod)
		{
		case GenericHTTPClient::RequestGetMethod:
		default:
			bReturn = RequestGet(szURI);
			break;
		case GenericHTTPClient::RequestPostMethod:
			bReturn = RequestPost(szURI);
			break;
		case GenericHTTPClient::RequestPostMethodMultiPartsFormData:
			bReturn = RequestPostMultiPartsFormData(szURI);
			break;
		}
	}
	catch (...)
	{
	}

	return bReturn;
}

int GenericHTTPClient::RequestGet(const char* szURI)
{
	CONST TCHAR* szAcceptType[] = { __HTTP_ACCEPT_TYPE, NULL };

	_hHTTPRequest = ::HttpOpenRequest(_hHTTPConnection, __HTTP_VERB_GET, szURI,
		HTTP_VERSION, "", szAcceptType,
		INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE |
			_dwSecureFlag,
		0);

	if (!_hHTTPRequest)
	{
		_dwError = ::GetLastError();
		return FALSE;
	}

	if (!::HttpAddRequestHeaders(_hHTTPRequest, __HTTP_ACCEPT,
			_tcslen(__HTTP_ACCEPT), HTTP_ADDREQ_FLAG_REPLACE))
	{
		_dwError = ::GetLastError();
		return FALSE;
	}

	if (!::HttpSendRequest(_hHTTPRequest, NULL, 0, NULL, 0))
	{
		_dwError = ::GetLastError();
		return FALSE;
	}

	return TRUE;
}

int GenericHTTPClient::RequestPost(const char* szURI)
{
	CONST TCHAR* szAcceptType[] = { __HTTP_ACCEPT_TYPE, NULL };
	TCHAR szPostArguments[__SIZE_BUFFER] = "";
	LPCTSTR szContentType =
		TEXT("Content-Type: application/x-www-form-urlencoded\r\n");

	DWORD dwPostArgumentsLegnth =
		GetPostArguments(szPostArguments, __SIZE_BUFFER);

	_hHTTPRequest = ::HttpOpenRequest(_hHTTPConnection, __HTTP_VERB_POST, szURI,
		HTTP_VERSION, "", szAcceptType,
		INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE |
			INTERNET_FLAG_FORMS_SUBMIT | _dwSecureFlag,
		0);

	if (!_hHTTPRequest)
	{
		_dwError = ::GetLastError();
		return FALSE;
	}

	if (!::HttpAddRequestHeaders(_hHTTPRequest, __HTTP_ACCEPT,
			_tcslen(__HTTP_ACCEPT), HTTP_ADDREQ_FLAG_REPLACE))
	{
		_dwError = ::GetLastError();
		return FALSE;
	}

	if (!::HttpSendRequest(_hHTTPRequest, szContentType, _tcslen(szContentType),
			(LPVOID)szPostArguments, _tcslen(szPostArguments)))
	{
		_dwError = ::GetLastError();
		return FALSE;
	}

	return TRUE;
}

int GenericHTTPClient::RequestPostMultiPartsFormData(const char* szURI)
{
	CONST TCHAR* szAcceptType[] = { __HTTP_ACCEPT_TYPE, NULL };
	LPCTSTR szContentType =
		TEXT("Content-Type: multipart/form-data; "
			 "boundary=--MULTI-PARTS-FORM-DATA-BOUNDARY\r\n");

	PBYTE pPostBuffer = NULL;
	DWORD dwPostBufferLength = AllocMultiPartsFormData(pPostBuffer,
		"--MULTI-PARTS-FORM-DATA-BOUNDARY");

	_hHTTPRequest = ::HttpOpenRequest(_hHTTPConnection, __HTTP_VERB_POST, szURI,
		HTTP_VERSION, "", szAcceptType,
		INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE |
			INTERNET_FLAG_FORMS_SUBMIT | _dwSecureFlag,
		0);

	if (!_hHTTPRequest)
	{
		_dwError = ::GetLastError();
		return FALSE;
	}

	if (!::HttpAddRequestHeaders(_hHTTPRequest, __HTTP_ACCEPT,
			_tcslen(__HTTP_ACCEPT), HTTP_ADDREQ_FLAG_REPLACE))
	{
		_dwError = ::GetLastError();
		return FALSE;
	}

	if (!::HttpAddRequestHeaders(_hHTTPRequest, szContentType,
			_tcslen(szContentType), HTTP_ADDREQ_FLAG_ADD_IF_NEW))
	{
		_dwError = ::GetLastError();
		return FALSE;
	}

	TCHAR szContentLength[__SIZE_BUFFER];
	::ZeroMemory(szContentLength, __SIZE_BUFFER);

	_stprintf(szContentLength, "Content-Length: %d\r\n", dwPostBufferLength);

	if (!::HttpAddRequestHeaders(_hHTTPRequest, szContentLength,
			_tcslen(szContentLength), HTTP_ADDREQ_FLAG_ADD_IF_NEW))
	{
		_dwError = ::GetLastError();
		return FALSE;
	}

	INTERNET_BUFFERS InternetBufferIn = { 0 };
	InternetBufferIn.dwStructSize = sizeof(INTERNET_BUFFERS);
	InternetBufferIn.Next = NULL;

	if (!::HttpSendRequestEx(_hHTTPRequest, &InternetBufferIn, NULL,
			HSR_INITIATE, 0))
	{
		_dwError = ::GetLastError();
		return FALSE;
	}

	DWORD dwOutPostBufferLength = 0;
	if (!::InternetWriteFile(_hHTTPRequest, pPostBuffer, dwPostBufferLength,
			&dwOutPostBufferLength))
	{
		_dwError = ::GetLastError();
		return FALSE;
	}

	if (!::HttpEndRequest(_hHTTPRequest, NULL, HSR_INITIATE, 0))
	{
		_dwError = ::GetLastError();
		return FALSE;
	}

	FreeMultiPartsFormData(pPostBuffer);

	return TRUE;
}

unsigned long GenericHTTPClient::ResponseOfBytes(unsigned char* pBuffer,
	unsigned long dwSize)
{
	static DWORD dwBytes = 0;

	if (!_hHTTPRequest)
	{
		_dwError = ::GetLastError();
		return 0;
	}

	::ZeroMemory(pBuffer, dwSize);
	if (!::InternetReadFile(_hHTTPRequest, pBuffer, dwSize, &dwBytes))
	{
		_dwError = ::GetLastError();
		return 0;
	}

	return dwBytes;
}

int GenericHTTPClient::Response(unsigned char* pHeaderBuffer,
	unsigned long dwHeaderBufferLength, unsigned char* pBuffer,
	unsigned long dwBufferLength, unsigned long& dwResultSize)
{
	BYTE pResponseBuffer[__SIZE_BUFFER] = "";
	DWORD dwNumOfBytesToRead = 0;

	if (!_hHTTPRequest)
	{
		_dwError = ::GetLastError();
		return FALSE;
	}

	::ZeroMemory(pBuffer, dwBufferLength);
	dwResultSize = 0;
	while ((dwNumOfBytesToRead =
				   ResponseOfBytes(pResponseBuffer, __SIZE_BUFFER)) != NULL &&
		dwResultSize < dwBufferLength)
	{
		::CopyMemory((pBuffer + dwResultSize), pResponseBuffer,
			dwNumOfBytesToRead);
		dwResultSize += dwNumOfBytesToRead;
	}

	::ZeroMemory(pHeaderBuffer, dwHeaderBufferLength);
	if (!::HttpQueryInfo(_hHTTPRequest, HTTP_QUERY_RAW_HEADERS_CRLF,
			pHeaderBuffer, &dwHeaderBufferLength, NULL))
	{
		_dwError = ::GetLastError();
		return FALSE;
	}

	return (dwResultSize ? TRUE : FALSE);
}

void GenericHTTPClient::AddPostArguments(const char* szName,
	const char* szValue, int bBinary)
{
	GenericHTTPArgument arg;
	::ZeroMemory(&arg, sizeof(arg));

	_tcscpy(arg.szName, szName);
	_tcscpy(arg.szValue, szValue);

	arg.dwType =
		bBinary ? GenericHTTPClient::TypeBinary : GenericHTTPClient::TypeNormal;

	_vArguments.push_back(arg);
}

void GenericHTTPClient::AddPostArguments(const char* szName,
	unsigned long nValue)
{
	GenericHTTPArgument arg;
	::FillMemory(&arg, sizeof(arg), 0x00);

	_tcscpy(arg.szName, szName);
	_stprintf(arg.szValue, "%d", nValue);

	arg.dwType = GenericHTTPClient::TypeNormal;

	_vArguments.push_back(arg);
}

unsigned long GenericHTTPClient::GetPostArguments(char* szArguments,
	unsigned long dwLength)
{
	std::vector<GenericHTTPArgument>::iterator itArg;

	::ZeroMemory(szArguments, dwLength);
	for (itArg = _vArguments.begin(); itArg < _vArguments.end();)
	{
		_tcscat(szArguments, itArg->szName);
		_tcscat(szArguments, "=");
		_tcscat(szArguments, itArg->szValue);

		if ((++itArg) < _vArguments.end())
		{
			_tcscat(szArguments, "&");
		}
	}

	*(szArguments + dwLength) = 0;

	return _tcslen(szArguments);
}

void GenericHTTPClient::InitilizePostArguments()
{
	_vArguments.clear();
}

void GenericHTTPClient::FreeMultiPartsFormData(unsigned char*& pBuffer)
{
	if (pBuffer)
	{
		::LocalFree(pBuffer);
		pBuffer = NULL;
	}
}

unsigned long GenericHTTPClient::AllocMultiPartsFormData(
	unsigned char*& pInBuffer, const char* szBoundary)
{
	if (pInBuffer)
	{
		::LocalFree(pInBuffer);
		pInBuffer = NULL;
	}

	pInBuffer = (PBYTE)::LocalAlloc(LPTR, GetMultiPartsFormDataLength());
	std::vector<GenericHTTPArgument>::iterator itArgv;

	DWORD dwPosition = 0;
	DWORD dwBufferSize = 0;

	for (itArgv = _vArguments.begin(); itArgv < _vArguments.end(); ++itArgv)
	{
		PBYTE pBuffer = NULL;

		switch (itArgv->dwType)
		{
		case GenericHTTPClient::TypeNormal:
			pBuffer = (PBYTE)::LocalAlloc(LPTR, __SIZE_HTTP_HEAD_LINE * 4);

			_stprintf((TCHAR*)pBuffer,
				"--%s\r\n"
				"Content-Disposition: form-data; name=\"%s\"\r\n"
				"\r\n"
				"%s\r\n",
				szBoundary, itArgv->szName, itArgv->szValue);

			dwBufferSize = _tcslen((TCHAR*)pBuffer);

			break;
		case GenericHTTPClient::TypeBinary:
			DWORD dwNumOfBytesToRead = 0;
			DWORD dwTotalBytes = 0;

			HANDLE hFile = ::CreateFile(itArgv->szValue, GENERIC_READ,
				FILE_SHARE_READ, NULL, OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
				break;

			DWORD dwSize = ::GetFileSize(hFile, NULL);

			pBuffer = (PBYTE)::LocalAlloc(LPTR,
				__SIZE_HTTP_HEAD_LINE * 3 + dwSize + 1);
			BYTE pBytes[__SIZE_BUFFER + 1] = "";

			_stprintf((TCHAR*)pBuffer,
				"--%s\r\n"
				"Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n"
				"Content-Type: %s\r\n"
				"\r\n",
				szBoundary, itArgv->szName, itArgv->szValue,
				GetContentType(itArgv->szValue));

			DWORD dwBufPosition = _tcslen((TCHAR*)pBuffer);

			while (::ReadFile(hFile, pBytes, __SIZE_BUFFER, &dwNumOfBytesToRead,
					   NULL) &&
				dwNumOfBytesToRead > 0 && dwTotalBytes <= dwSize)
			{
				::CopyMemory((pBuffer + dwBufPosition + dwTotalBytes), pBytes,
					dwNumOfBytesToRead);
				::ZeroMemory(pBytes, __SIZE_BUFFER + 1);
				dwTotalBytes += dwNumOfBytesToRead;
			}

			::CopyMemory((pBuffer + dwBufPosition + dwTotalBytes), "\r\n",
				_tcslen("\r\n"));
			::CloseHandle(hFile);

			dwBufferSize = dwBufPosition + dwTotalBytes + _tcslen("\r\n");
			break;
		}

		::CopyMemory((pInBuffer + dwPosition), pBuffer, dwBufferSize);
		dwPosition += dwBufferSize;

		if (pBuffer)
		{
			::LocalFree(pBuffer);
			pBuffer = NULL;
		}
	}

	::CopyMemory((pInBuffer + dwPosition), "--", 2);
	::CopyMemory((pInBuffer + dwPosition + 2), szBoundary, _tcslen(szBoundary));
	::CopyMemory((pInBuffer + dwPosition + 2 + _tcslen(szBoundary)), "--\r\n",
		4);

	return dwPosition + 6 + _tcslen(szBoundary);
}

unsigned long GenericHTTPClient::GetMultiPartsFormDataLength()
{
	std::vector<GenericHTTPArgument>::iterator itArgv;

	DWORD dwLength = 0;

	for (itArgv = _vArguments.begin(); itArgv < _vArguments.end(); ++itArgv)
	{
		switch (itArgv->dwType)
		{
		case GenericHTTPClient::TypeNormal:
			dwLength += __SIZE_HTTP_HEAD_LINE * 4;
			break;
		case GenericHTTPClient::TypeBinary:
			HANDLE hFile =
				::CreateFile(itArgv->szValue, GENERIC_READ, FILE_SHARE_READ,
					NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			if (hFile != INVALID_HANDLE_VALUE)
			{
				dwLength +=
					__SIZE_HTTP_HEAD_LINE * 3 + ::GetFileSize(hFile, NULL);
				::CloseHandle(hFile);
			}
			break;
		}
	}

	return dwLength;
}

const char* GenericHTTPClient::GetContentType(const char* szName)
{
	static TCHAR szReturn[1024] = "";
	LONG dwLen = 1024;
	DWORD dwDot = 0;

	for (dwDot = _tcslen(szName); dwDot > 0; dwDot--)
	{
		if (!_tcsncmp((szName + dwDot), ".", 1))
			break;
	}

	HKEY hKEY;
	LPTSTR szWord = (char*)(szName + dwDot);

	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, szWord, 0, KEY_QUERY_VALUE, &hKEY) ==
		ERROR_SUCCESS)
	{
		if (RegQueryValueEx(hKEY, TEXT("Content Type"), NULL, NULL,
				(LPBYTE)szReturn, (unsigned long*)&dwLen) != ERROR_SUCCESS)
			_tcsncpy(szReturn, "application/octet-stream",
				_tcslen("application/octet-stream"));
		RegCloseKey(hKEY);
	}
	else
	{
		_tcsncpy(szReturn, "application/octet-stream",
			_tcslen("application/octet-stream"));
	}

	return szReturn;
}

unsigned long GenericHTTPClient::GetLastError()
{
	return _dwError;
}

void GenericHTTPClient::ParseURL(const char* szURL, char* szProtocol,
	char* szAddress, unsigned long& dwPort, char* szURI)
{
	DWORD dwPosition = 0;
	BOOL bFlag = FALSE;

	while (_tcslen(szURL) > 0 && dwPosition < _tcslen(szURL) &&
		_tcsncmp((szURL + dwPosition), ":", 1))
		++dwPosition;

	if (!_tcsncmp((szURL + dwPosition + 1), "/", 1))
	{
		if (szProtocol)
		{
			_tcsncpy(szProtocol, szURL, dwPosition);
			szProtocol[dwPosition] = 0;
		}
		bFlag = TRUE;
	}
	else
	{
		if (szProtocol)
		{
			_tcsncpy(szProtocol, "http", 4);
			szProtocol[5] = 0;
		}
	}

	DWORD dwStartPosition = 0;

	if (bFlag)
	{
		dwStartPosition = dwPosition += 3;
	}
	else
	{
		dwStartPosition = dwPosition = 0;
	}

	bFlag = FALSE;
	while (_tcslen(szURL) > 0 && dwPosition < _tcslen(szURL) &&
		_tcsncmp((szURL + dwPosition), "/", 1))
		++dwPosition;

	DWORD dwFind = dwStartPosition;

	for (; dwFind <= dwPosition; dwFind++)
	{
		if (!_tcsncmp((szURL + dwFind), ":", 1))
		{
			bFlag = TRUE;
			break;
		}
	}

	if (bFlag)
	{
		TCHAR sztmp[__SIZE_BUFFER] = "";
		_tcsncpy(sztmp, (szURL + dwFind + 1), dwPosition - dwFind);
		dwPort = _ttol(sztmp);
		_tcsncpy(szAddress, (szURL + dwStartPosition),
			dwFind - dwStartPosition);
	}
	else if (!_tcscmp(szProtocol, "https"))
	{
		dwPort = INTERNET_DEFAULT_HTTPS_PORT;
		_tcsncpy(szAddress, (szURL + dwStartPosition),
			dwPosition - dwStartPosition);
	}
	else
	{
		dwPort = INTERNET_DEFAULT_HTTP_PORT;
		_tcsncpy(szAddress, (szURL + dwStartPosition),
			dwPosition - dwStartPosition);
	}

	if (dwPosition < _tcslen(szURL))
	{
		_tcsncpy(szURI, (szURL + dwPosition), _tcslen(szURL) - dwPosition);
	}
	else
	{
		szURI[0] = 0;
	}
}

const char* GenericHTTPClient::QueryHTTPResponseHeader()
{
	return _szHTTPResponseHeader;
}

const char* GenericHTTPClient::QueryHTTPResponse()
{
	return _szHTTPResponseHTML;
}
