#pragma once

#define IPC_MEMORY_SIZE 4096

class cIPCMemServer
{
public:
	cIPCMemServer();
	~cIPCMemServer();

	bool Create(const char* szName);
	bool Send(void* pData, int nSize, int nOffset);
	bool Recv(void* pData, int nSize, int nOffset);
	void ShutDown();

private:
	HANDLE m_hMapFile;
};

class cIPCMemClient
{
public:
	cIPCMemClient();
	~cIPCMemClient();

	bool Connection(const char* szName);
	bool Send(void* pData, int nSize);
	bool Recv(void* pData, int nSize);
	void Close();

private:
	HANDLE m_hMapFile;
};
