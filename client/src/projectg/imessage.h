#pragma once

#define MAX_MSG_SIZE 8192

class MemBlock
{
public:
	const unsigned char* GetBuffer() const { return m_Buffer; }

	unsigned char* LockBuffer(int offset) { return &m_Buffer[offset]; }

	unsigned int GetLength() const { return m_offset; }

	MemBlock();
	MemBlock(unsigned int uSize);
	virtual ~MemBlock();

	int Free();
	int Alloc(unsigned int uSize);
	int ReAlloc(unsigned int uSize);
	void Reset();

protected:
	unsigned int GetBufferSize() const { return m_uSize; }

	unsigned char m_Buffer[MAX_MSG_SIZE];
	unsigned char* m_pBuffer;
	unsigned int m_uSize;
	unsigned int m_offset;
};

class ISendMsg : public MemBlock
{
public:
	ISendMsg();
	ISendMsg(unsigned short nPaketType);
	virtual ~ISendMsg();

	void Encode1(unsigned char in);
	void Encode2(unsigned short in);
	void Encode4(unsigned long in);
	void EncodeStr(const char* data);
	void EncodeBuffer(const void* src, unsigned short len);
	void FixData(int OffSet, char* Buffer, int Len);

	virtual void MakePacketComplete();

	virtual void MakeCheckSum();

	virtual void RollBack() { }

protected:
	int m_nPacketType;
	unsigned char m_nPandoraKey;
};

class IRecvMsg : public MemBlock
{
public:
	IRecvMsg();
	virtual ~IRecvMsg();

	unsigned char Decode1();
	unsigned short Decode2();
	unsigned long Decode4();
	std::string DecodeStr();
	void DecodeBuffer(void* pBuffer, int len);
	bool DecodeCheckSum();

	unsigned int GetLength() const { return m_nLength; }
	bool IsValid(bool rawpacket);
	void SetRcvPacket(const char* pBuf, int Len);

	virtual void DecodeComplete() { }

protected:
	unsigned int m_nLength;
};
