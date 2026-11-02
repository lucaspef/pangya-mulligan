#pragma once

#include <string>
#include <memory>

enum enumClientPacket;

// JP 983: `case` de pacote recebido que ainda está na numeração do KR 645 e não tem id do 983 conhecido
// (tools/f35/rxids.py; lista em out/F3.5/s61/antes/rxids_recebe.tsv). O valor fica fora da faixa de ids para o
// tratador não pegar OUTRO pacote do 983; ao achar o id certo, trocar por ele.
#define PKT645_SEM_983(id) (0x64500000 | (id))

enum eSendTo
{
	TO_GAME,
	TO_MSN,
	TO_SHOP,
	TO_LOGIN,
	TO_RANK,
	TO_COUNT
};

class WMemBlock
{
public:
	const unsigned char* GetBuffer() const { return m_pBuffer; }

	unsigned char* LockBuffer(int offset) { return m_pBuffer + offset; }

	unsigned int GetLength() const { return m_offset; }

	WMemBlock();
	WMemBlock(unsigned int uSize);
	virtual ~WMemBlock();

	int Alloc(unsigned int uSize);
	int ReAlloc(unsigned int uSize);
	int Free();
	void ResetBuffer();

protected:
	unsigned int GetBufferSize() { return m_uSize; }

	unsigned char* m_pBuffer;
	unsigned int m_uSize;
	unsigned int m_offset;
};

namespace _private
{

class WSendPacketImpl : public WMemBlock
{
public:
	WSendPacketImpl();
	// JP 983: uma cópia fora de linha em 00a4da10; embutido nos construtores do WSendPacket (00a4e800, 00a4e8a0)
	WSendPacketImpl(int type)
		: WMemBlock(0x40)
	{
		InitPacket(type);
	}
	virtual ~WSendPacketImpl();

	void InitPacket(int nPacketType);
	int GetPacketType() const;
	int GetMark() const;
	unsigned int GetLengthData();

	void Encode1(unsigned char in);
	void Encode2(unsigned short in);
	void Encode4(unsigned long in);
	void Encode8(__int64 in);
	void EncodeStr(const std::string& str);
	void EncodeEncryptedStr(const std::string& str);
	void EncodeBuffer(const void* src, unsigned short len);
	void EncodeData(int OffSet, char* buffer, int Len);

	bool SetSendPacket(const unsigned char* pBuf, unsigned int len,
		unsigned char ParseKey, unsigned char PubKey);
	// JP 983 (005ab320): o destino vem junto; só o pacote para o servidor de jogo leva o contador
	void MakePacketComplete(int ParseKey, int to);
	void RollBack();

	static unsigned char ms_nPacketCount;

	void MakeCheckSum();

protected:

	int m_nPacketType;
	unsigned char m_nPandoraKey;
};

class WReceivedPacketImpl : public WMemBlock
{
public:
	WReceivedPacketImpl();
	virtual ~WReceivedPacketImpl();

	bool SetRcvPacket(const unsigned char* pBuf, unsigned int len,
		unsigned char ParseKey, unsigned char PubKey);
	bool IsValid(bool rawpacket);
	void Copy(WReceivedPacketImpl* pCopy);
	void Reset();

	bool DecodeCheckSum();
	unsigned char Decode1();
	unsigned short Decode2();
	unsigned long Decode4();
	__int64 Decode8();
	std::string DecodeStr();
	bool DecodeStr(std::string& str);
	std::string DecodeEncryptedStr(unsigned long key);
	void DecodeBuffer(void* pBuffer, int len);

	unsigned char* GetRemainBuffer();
	int GetRemainLength();

	// JP 983: 00a4da70 e 00a4da80 (sem chamador; nomes meus)
	int GetPubKey() const { return m_nPubKey; }
	unsigned int GetRcvLength() const { return m_nLength; }

protected:
	int m_nPubKey;
	int m_nEncLen;
	unsigned int m_nLength;
	unsigned char m_nParseKey;
};

} // namespace _private

// JP 983: WSendPacket e WReceivedPacket são invólucros de 0xc (vtable + std::tr1::shared_ptr para a implementação);
// o código que era delas no KR 645 ficou em _private::WSendPacketImpl / _private::WReceivedPacketImpl
// (005ab070-005abdc0) e os métodos daqui só repassam (00a4da10-00a4ecd0). RTTI do 983:
// .?AVWSendPacketImpl@_private@@, .?AV?$_Ref_count@VWSendPacketImpl@_private@@@tr1@std@@ (out/F3.5/s62).
class WSendPacket
{
public:
	WSendPacket();
	WSendPacket(enumClientPacket type);
	WSendPacket(int type);
	virtual ~WSendPacket();

	// 00a4ec50 / 00a4dc50-00a4dcb0: sem chamador no exe (duas cópias de cada comparação; assinaturas minhas)
	void operator=(const WSendPacket& rhs);
	bool operator==(const WSendPacket& rhs) const;
	bool operator==(WSendPacket& rhs);
	bool operator!=(const WSendPacket& rhs) const;
	bool operator!=(WSendPacket& rhs);

	const unsigned char* GetBuffer() const;
	unsigned char* LockBuffer(int offset);
	unsigned int GetLength() const;
	void ResetBuffer();

	int GetPacketType() const;
	void MakePacketComplete(int ParseKey, int to);
	void RollBack();
	int GetMark() const;

	void Encode1(unsigned char in);
	void Encode2(unsigned short in);
	void Encode4(unsigned long in);
	void Encode8(__int64 in);
	void EncodeStr(const std::string& str);
	void EncodeEncryptedStr(const std::string& str);
	void EncodeBuffer(const void* src, unsigned short len);
	void EncodeData(int OffSet, char* buffer, int Len);

	void InitPacket(int nPacketType);
	bool SetSendPacket(const unsigned char* pBuf, unsigned int len,
		unsigned char ParseKey, unsigned char PubKey);
	unsigned int GetLengthData();
	void MakeCheckSum();

	void Send(eSendTo to);

private:
	std::tr1::shared_ptr<_private::WSendPacketImpl> m_impl;
};

// JP 983 (sessao 63): envio herdado do KR 645 cujo id nao tem lugar conhecido na numeracao do 983 (tools/f35/pkids2.py;
// lista em out/F3.5/s63/pkids2_envio.tsv, decisao `sem983`). Com o numero do 645 o servidor leria OUTRO pacote; ate o id
// certo aparecer (ou a funcao ser reescrita pelo 983), a declaracao usa este tipo: mesma interface, nao monta nem envia.
// Nao existe no exe 983.
class WSendPacketSem983
{
public:
	WSendPacketSem983(int) {}
	void Encode1(unsigned char) {}
	void Encode2(unsigned short) {}
	void Encode4(unsigned long) {}
	void Encode8(__int64) {}
	void EncodeStr(const std::string&) {}
	void EncodeEncryptedStr(const std::string&) {}
	void EncodeBuffer(const void*, unsigned short) {}
	void EncodeData(int, char*, int) {}
	int GetMark() const { return 0; }
	void Send(eSendTo) {}
};

class WReceivedPacket
{
public:
	WReceivedPacket();
	virtual ~WReceivedPacket();

	// 00a4eca0 / 00a4de20-00a4de80: sem chamador no exe (assinaturas minhas)
	void operator=(const WReceivedPacket& rhs);
	bool operator==(const WReceivedPacket& rhs) const;
	bool operator==(WReceivedPacket& rhs);
	bool operator!=(const WReceivedPacket& rhs) const;
	bool operator!=(WReceivedPacket& rhs);

	int GetPubKey() const;			// 00a4dea0 (nome meu)

	unsigned char Decode1();
	unsigned short Decode2();
	unsigned long Decode4();
	__int64 Decode8();
	bool DecodeStr(std::string& str);
	void DecodeBuffer(void* pBuffer, int len);
	bool DecodeCheckSum();

	unsigned int GetRcvLength() const;	// 00a4df20 (nome meu)

	bool IsValid(bool rawpacket);
	bool SetRcvPacket(const unsigned char* pBuf, unsigned int len,
		unsigned char ParseKey, unsigned char PubKey);
	void Copy(WReceivedPacket* pCopy);
	void Reset();

	unsigned char* GetRemainBuffer();
	int GetRemainLength();

	std::string DecodeStr();
	std::string DecodeEncryptedStr(unsigned long key);

private:
	std::tr1::shared_ptr<_private::WReceivedPacketImpl> m_impl;
};
