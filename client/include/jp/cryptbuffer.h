#pragma once
// JP 983: buffer cifrado com a CryptoAPI (sem RTTI: classe sem virtual; nomes meus). Corpo em 006c6d20–006c6f40, dentro da
// faixa do uiframesetmanager: escrito em src/projectg/jp/uiframesetmanager.cpp (sessão B59, 4 = 4 IGUAL). Medido no 983 (B58):
//   006c6d20 ctor: zera os campos; CryptAcquireContextA(&m_hProv, 0, provider, provType, CRYPT_VERIFYCONTEXT);
//            CryptCreateHash(hashAlg) + CryptHashData(senha, strlen) + CryptDeriveKey(cipherAlg, keyFlags, &m_hKey) → m_bOk;
//            CryptDestroyHash.
//   006c6de0 dtor: delete[] m_pData; CryptDestroyKey; CryptReleaseContext.
//   006c6e30 Encrypt: m_size = n; CryptEncrypt(0, …) só para o tamanho; new[] + memcpy; CryptEncrypt de verdade → m_bOk
//            (falhou: delete[] e m_size = 0).
//   006c6ed0 Decrypt: m_size = n; new[] + memcpy; CryptDecrypt → m_bOk (falhou: delete[] e m_size = 0).
// Parâmetros: um `sCryptParam983` (único uso conhecido: 00e14b44, unidade replayextension).

struct sCryptParam983 // 0x14
{
	const char* provider;  // +0x00 "Microsoft Base Cryptographic Provider v1.0"
	unsigned long provType; // +0x04 PROV_RSA_FULL (1)
	unsigned int hashAlg;   // +0x08 CALG_MD5 (0x8003)
	unsigned int cipherAlg; // +0x0c CALG_RC4 (0x6801)
	unsigned long keyFlags; // +0x10 0x380000 (chave de 56 bits)
};

class CCryptBuffer983 // 0x14
{
public:
	CCryptBuffer983(const sCryptParam983* param, const char* password); // 006c6d20
	~CCryptBuffer983(); // 006c6de0

	void Encrypt(const void* data, unsigned int size); // 006c6e30
	void Decrypt(const void* data, unsigned int size); // 006c6ed0

	unsigned char* GetData() const { return m_pData; } // 006269e0 (cópia fora de linha)
	unsigned int GetSize() const { return m_size; } // 006269f0
	bool IsOk() const { return m_bOk; } // 00626a00

protected:
	unsigned long m_hProv;  // +0x00 HCRYPTPROV
	unsigned long m_hKey;   // +0x04 HCRYPTKEY
	unsigned char* m_pData; // +0x08
	unsigned int m_size;    // +0x0c
	bool m_bOk;             // +0x10
};

// Cifra no construtor (00626a10, cópia fora de linha; destrutor 00626cf0 = jmp 006c6de0)
class CCryptEncoder983 : public CCryptBuffer983
{
public:
	CCryptEncoder983(const sCryptParam983* param, const char* password, const void* data, unsigned int size)
		: CCryptBuffer983(param, password)
	{
		Encrypt(data, size);
	}
};

// Decifra no construtor (00626a80, cópia fora de linha; destrutor 00626d00 = jmp 006c6de0)
class CCryptDecoder983 : public CCryptBuffer983
{
public:
	CCryptDecoder983(const sCryptParam983* param, const char* password, const void* data, unsigned int size)
		: CCryptBuffer983(param, password)
	{
		Decrypt(data, size);
	}
};

C_ASSERT(sizeof(sCryptParam983) == 0x14);
C_ASSERT(sizeof(CCryptBuffer983) == 0x14);
