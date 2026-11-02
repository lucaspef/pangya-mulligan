#include "minatl.h"
#include "pangfbi.h"
#include "generichttpclient.h"

std::string MultiByteToUtf8_983(const char* text); // 005e5040 (jp/textconv.cpp)

CPangFBI::CPangFBI()
{
}

CPangFBI::~CPangFBI()
{
}

// 983 00898f80: os textos vao ao servidor em UTF-8 (SJIS -> UTF-8 pelo 005e5040), um temporario por argumento
void CPangFBI::ProcessReport(const char* type, const char* data)
{
	if (strcmpi("crime_log", type) == 0)
	{
		int crimeType = 0;
		char szFile[1024];

		char szID[128] = { 0 };
		sscanf(data, "%s %d %s", szID, &crimeType, szFile);

		FILE* fp = fopen(szFile, "rb");

		if (fp == NULL)
			return;
		fclose(fp);

		GenericHTTPClient* pClient = new GenericHTTPClient;

		pClient->InitilizePostArguments();

		pClient->AddPostArguments("id", MultiByteToUtf8_983(szID).c_str(), FALSE);
		pClient->AddPostArguments("type", crimeType);
		pClient->AddPostArguments("file", MultiByteToUtf8_983(szFile).c_str(), TRUE);

		if (pClient->Request(
				"http://157.7.172.65:50007/Report/CrimeReport.aspx",
				3, "MERONG(0.9/;p)"))
		{
			pClient->QueryHTTPResponse();
		}

		delete pClient;
		DeleteFile(szFile);
	}
	else if (strcmpi("app_log", type) == 0)
	{
		int code = 0;
		char szUserID[128] = { 0 };
		char szPacketVer[64];
		char szClientVer[64], szComment[128];

		sscanf(data, "%d %s %s %s %s", &code, szUserID, szPacketVer,
			szClientVer, szComment);

		GenericHTTPClient* pClient = new GenericHTTPClient;
		if (pClient == NULL)
			return;
		pClient->InitilizePostArguments();

		pClient->AddPostArguments("code", code);
		pClient->AddPostArguments("userid", MultiByteToUtf8_983(szUserID).c_str(), FALSE);
		pClient->AddPostArguments("packet_ver", MultiByteToUtf8_983(szPacketVer).c_str(), FALSE);
		pClient->AddPostArguments("client_ver", MultiByteToUtf8_983(szClientVer).c_str(), FALSE);
		pClient->AddPostArguments("comment", MultiByteToUtf8_983(szComment).c_str(), FALSE);

		if (pClient->Request(
				"http://157.7.172.65:50007/Report/ReportError.aspx",
				3, "MERONG(0.9/;p)"))
		{
			pClient->QueryHTTPResponse();
		}

		delete pClient;
	}
}
