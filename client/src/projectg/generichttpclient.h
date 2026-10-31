#pragma once

#include <vector>
#include <WinInet.h>

#define __SIZE_HTTP_ARGUMENT_NAME 256
#define __SIZE_HTTP_ARGUMENT_VALUE 65536
#define __HTTP_RESPONSE_BUFFER_SIZE 100000

class GenericHTTPClient
{
public:
	struct __GENERIC_HTTP_ARGUMENT
	{
		char szName[__SIZE_HTTP_ARGUMENT_NAME];
		char szValue[__SIZE_HTTP_ARGUMENT_VALUE];
		unsigned long dwType;
	};
	typedef __GENERIC_HTTP_ARGUMENT GenericHTTPArgument;

	enum enumRequestMethod
	{
		RequestUnknown = 0,
		RequestGetMethod = 1,
		RequestPostMethod = 2,
		RequestPostMethodMultiPartsFormData = 3,
	};

	enum enumTypePostArgument
	{
		TypeUnknown = 0,
		TypeNormal = 1,
		TypeBinary = 2,
	};

	GenericHTTPClient();
	virtual ~GenericHTTPClient();

	static enumRequestMethod GetMethod(int nMethod);
	static enumTypePostArgument GetPostArgumentType(int nType);

	int Connect(const char* szAddress, const char* szAgent,
		unsigned short nPort, const char* szUserAccount,
		const char* szPassword);
	int Close();
	void InitilizePostArguments();

	void AddPostArguments(const char* szName, unsigned long nValue);
	void AddPostArguments(const char* szName, const char* szValue, int bBinary);

	int Request(const char* szURL, int nMethod, const char* szAgent);
	int RequestOfURI(const char* szURI, int nMethod);
	int Response(unsigned char* pHeaderBuffer,
		unsigned long dwHeaderBufferLength, unsigned char* pBuffer,
		unsigned long dwBufferLength, unsigned long& dwResultSize);
	const char* QueryHTTPResponse();
	const char* QueryHTTPResponseHeader();

	unsigned long GetLastError();
	const char* GetContentType(const char* szName);
	void ParseURL(const char* szURL, char* szProtocol, char* szAddress,
		unsigned long& dwPort, char* szURI);

protected:
	std::vector<GenericHTTPArgument> _vArguments;

	char _szHTTPResponseHTML[__HTTP_RESPONSE_BUFFER_SIZE];
	char _szHTTPResponseHeader[__HTTP_RESPONSE_BUFFER_SIZE];

	HINTERNET _hHTTPOpen;
	HINTERNET _hHTTPConnection;
	HINTERNET _hHTTPRequest;

	unsigned long _dwError;
	const char* _szHost;
	unsigned long _dwPort;
	unsigned long _dwSecureFlag;

	unsigned long ResponseOfBytes(unsigned char* pBuffer, unsigned long dwSize);
	unsigned long GetPostArguments(char* szArguments, unsigned long dwLength);
	int RequestPost(const char* szURI);
	int RequestPostMultiPartsFormData(const char* szURI);
	int RequestGet(const char* szURI);
	unsigned long AllocMultiPartsFormData(unsigned char*& pInBuffer,
		const char* szBoundary);
	void FreeMultiPartsFormData(unsigned char*& pBuffer);
	unsigned long GetMultiPartsFormDataLength();
};
