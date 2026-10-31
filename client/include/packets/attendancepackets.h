// attendancepackets.h - frente C (pacotes). JP 983: bloco do premio de presenca (casos 0x248, 0x249 e 0x24a do
// CTask::OnPacketCommon). Os nomes sao nossos; tamanho e campos medidos no exe 983.
#pragma once

// usa WReceivedPacket: incluir depois dele

// construtor 0047cdb0, leitura 009b9120 (D1 D4 D4 D4 D4 D4). Os nomes dos campos vem do texto de depuracao do caso
// 0x249 (009d5be1): "isGift(", " today(", " X ", " [TID:", " next(", " round(".
struct sAttendanceReward983
{
	bool bGift;					// +0x00
	unsigned long todayTid;		// +0x04
	unsigned long todayCount;	// +0x08
	unsigned long nextTid;		// +0x0c
	unsigned long nextCount;	// +0x10
	unsigned long round;		// +0x14 (-1 no construtor)

	sAttendanceReward983()
	{
		bGift = false;
		todayTid = 0;
		todayCount = 0;
		nextTid = 0;
		nextCount = 0;
		round = 0xffffffff;
	}
	void Decode(WReceivedPacket& packet)
	{
		bGift = packet.Decode1() != 0;
		todayTid = packet.Decode4();
		todayCount = packet.Decode4();
		nextTid = packet.Decode4();
		nextCount = packet.Decode4();
		round = packet.Decode4();
	}
};
C_ASSERT(sizeof(sAttendanceReward983) == 0x18);
