#pragma once

struct SpoonParam
{
	void (*ProcMsg)(sockaddr_in& Addr, const char* Buffer, int Len);
	void (*ProcErr)(const char* Msg);
	unsigned short bindPort;
};

void MakeSpoon(const char* IP, int Port, sockaddr_in* Sa);

class UDPSpoon
{
public:
	UDPSpoon();
	virtual ~UDPSpoon();

	bool Open(SpoonParam& Param);
	void Close();
	int Send(const char* Buffer, int Len, sockaddr_in* Addr);

private:
	void Run();
	void Error(const char* Msg);

	friend unsigned int __stdcall SubSpoonThread(void* Param);

	SOCKET m_Socket;
	HANDLE m_hSignal;
	SpoonParam m_Param;
	bool m_bFinish;
	HANDLE m_hThread;
};
