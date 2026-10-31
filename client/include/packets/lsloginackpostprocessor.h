// lsloginackpostprocessor.h - frente C (pacotes). JP 983: resposta do servidor de login ao pedido de login
// (pacote 0x1 recebido; caso 0x1 do CLobbyTask::OnPacket, 00ac1e9a).
// A classe e o nome dela vêm da RTTI do 983 (vtable 00d0ce50); os nomes dos métodos são nossos.
// No 645 este código era o corpo do `case 1` do CLobbyTask::OnPacket.
#pragma once

class CTask;
class WReceivedPacket;

class CLSLoginAckPostProcessor
{
public:
	CLSLoginAckPostProcessor(CTask* pTask);	// 005a11f0
	virtual ~CLSLoginAckPostProcessor();	// 005a1210 (com delete: 005a1270)

	// 0 = tratado; outro valor = código de erro que o chamador manda a todos com a mensagem 0x27
	int Process(WReceivedPacket& packet);	// 005a2620

private:
	static bool __stdcall IsNicknameCode(unsigned char code);	// 005a1220: 0xd8, 0xd9
	static bool __stdcall IsSecondPwdCode(unsigned char code);	// 005a1240: 0xdc..0xde
	static bool __stdcall IsPlainCode(unsigned char code);	// 005a14b0: nem 0xe2, nem os dois grupos acima

	int OnFail(unsigned char code, WReceivedPacket& packet);	// 005a25d0
	int OnPlainCode(unsigned char code, WReceivedPacket& packet);	// 005a1630
	int OnNicknameCode(unsigned char code, WReceivedPacket& packet);	// 005a14f0
	int OnServerError(unsigned char code, WReceivedPacket& packet);	// 005a1ff0

	__forceinline void ReadBlockInfo(WReceivedPacket& packet);	// trecho repetido em 005a2620, 005a1630 e 005a1ff0 (em linha no 983)

	CTask* m_pTask;	// +0x04
};
