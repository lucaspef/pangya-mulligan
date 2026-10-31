// V3: layout do 983 das classes vistas por src/projectg/jp/pktviews983.cpp (gerado por tools/v3/gen.py; não editar à mão).
// unidade: src/projectg/jp/pktviews983.cpp
V3_SIZE(CAIPlayerInfoContainer983, 0x34) // 983: construtor 00915b90 (vetor +0x24..+0x2c zerado por último) + vtable 00d3c658 (.?AVCAIPlayerInfoContainer@@; icontentsdoc.cpp: 0x34) (C-PKT lote 68)
V3_OFF(CAIPlayerInfoContainer983, m_lineMap, 0x4) // 983: construtor 00915b90 grava a cabeça em +8 e o tamanho em +0xc; Initialize 00910aa0 limpa [this+4] (0090f130) (C-PKT lote 68)
V3_OFF(CAIPlayerInfoContainer983, m_infoMap, 0x14) // 983: 00910b80 insere em [this+0x14]; GetCount983 009091b0 devolve [ecx+0x1c] (tamanho) (C-PKT lote 68)
V3_OFF(CAIPlayerInfoContainer983, m_npcIds, 0x24) // 983: 00910b80 push_back em [esi+0x24]/[esi+0x28]/[esi+0x2c]; GetNpcId983 00909180 lê [ecx+0x24] (C-PKT lote 68)
V3_SIZE(CGaugeBar983, 0x4c) // 983: push 0x4c; call operator new em 0082ef8f, construtor 00923410 (mapa=3; 3 new); unidade por construtor
V3_OFF(CGaugeBar983, m_cur, 0x0) // 983: anotação do cabeçalho include/jp/gaugebar983.h:35
V3_OFF(CGaugeBar983, m_target, 0x4) // 983: anotação do cabeçalho include/jp/gaugebar983.h:36
V3_OFF(CGaugeBar983, m_value, 0x8) // 983: anotação do cabeçalho include/jp/gaugebar983.h:37
V3_OFF(CGaugeBar983, m_min, 0xc) // 983: anotação do cabeçalho include/jp/gaugebar983.h:38
V3_OFF(CGaugeBar983, m_max, 0x10) // 983: anotação do cabeçalho include/jp/gaugebar983.h:39
V3_OFF(CGaugeBar983, m_pOverlay, 0x14) // 983: anotação do cabeçalho include/jp/gaugebar983.h:40
V3_OFF(CGaugeBar983, m_pRes, 0x1c) // 983: anotação do cabeçalho include/jp/gaugebar983.h:41
V3_OFF(CGaugeBar983, m_b20, 0x20) // 983: anotação do cabeçalho include/jp/gaugebar983.h:42
V3_OFF(CGaugeBar983, m_f24, 0x24) // 983: anotação do cabeçalho include/jp/gaugebar983.h:43
V3_OFF(CGaugeBar983, m_x, 0x28) // 983: anotação do cabeçalho include/jp/gaugebar983.h:44
V3_OFF(CGaugeBar983, m_y, 0x2c) // 983: anotação do cabeçalho include/jp/gaugebar983.h:45
V3_OFF(CGaugeBar983, m_margin, 0x30) // 983: anotação do cabeçalho include/jp/gaugebar983.h:46
V3_OFF(CGaugeBar983, m_34, 0x34) // 983: anotação do cabeçalho include/jp/gaugebar983.h:47
V3_OFF(CGaugeBar983, m_w, 0x38) // 983: anotação do cabeçalho include/jp/gaugebar983.h:48
V3_OFF(CGaugeBar983, m_h, 0x3c) // 983: anotação do cabeçalho include/jp/gaugebar983.h:49
V3_OFF(CGaugeBar983, m_f40, 0x40) // 983: anotação do cabeçalho include/jp/gaugebar983.h:50
V3_OFF(CGaugeBar983, m_f44, 0x44) // 983: anotação do cabeçalho include/jp/gaugebar983.h:51
V3_OFF(CGaugeBar983, m_b48, 0x48) // 983: anotação do cabeçalho include/jp/gaugebar983.h:52
V3_OFF(CGaugeBar983, m_b49, 0x49) // 983: anotação do cabeçalho include/jp/gaugebar983.h:53
V3_OFF(CGaugeBar983, m_b4a, 0x4a) // 983: anotação do cabeçalho include/jp/gaugebar983.h:54
V3_SIZE(sAIDocInfo983, 0x28) // 983: 00910b80 DecodeBuffer(&registro, 0x28) e passo 0x28 no vetor (C-PKT lote 68)
V3_OFF(sAIDocInfo983, m_08, 0x08) // 983: anotação do cabeçalho include/jp/aiplayer.h:31
V3_OFF(sAIDocInfo983, m_pang, 0x10) // 983: anotação do cabeçalho include/jp/aiplayer.h:33
V3_OFF(sAIDocInfo983, m_bonusPang, 0x18) // 983: anotação do cabeçalho include/jp/aiplayer.h:34
V3_SIZE(sAiBinHeader983, 0x2c) // 983: 0090f720 Read(&cabeçalho, 0x2c); construtor 00907970 zera +0..+0x28 (C-PKT lote 68)
V3_OFF(sAiBinHeader983, version, 0x4) // 983: 0090f720 cmp [ebp-0xa0], 0x72 (cabeçalho em ebp-0xa4) (C-PKT lote 68)
V3_OFF(sAiBinHeader983, nodeNum, 0x24) // 983: 0090f720 reserve/laço com [ebp-0x80] (cabeçalho em ebp-0xa4) (C-PKT lote 68)
V3_ANOT(sAiBinHeader983, attachNum, 0x28) // anot: anotação do cabeçalho include/packets/aiplayerinfocontainer.h:45
V3_SIZE(sAiBinLine983, 0x28) // 983: 0090f720 Read(&linha, 0x28) (C-PKT lote 68)
V3_OFF(sAiBinLine983, count, 0x20) // 983: 0090f720 laço dos pontos com [ebp-0x58] (linha em ebp-0x78) (C-PKT lote 68)
V3_ANOT(sAiBinLine983, reserved, 0x24) // anot: anotação do cabeçalho include/packets/aiplayerinfocontainer.h:53
