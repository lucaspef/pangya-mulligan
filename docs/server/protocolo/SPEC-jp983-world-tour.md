# JP 983: World Tour

Fonte: o código reconstruído do cliente (rebang, só leitura): `taskmanager.cpp` (casos 0x119/0x11A de
`CTask::OnPacketCommon`), `worldtoureventdlg.cpp`, `lobbymain.cpp` (`OnToppage_WorldTourEvent*`),
`shared/globalgamedefine.h` (`CMapTypeAdapter`). Os ids do código são os do KR: no JP o S->C fica **+4** (KR 0x119 → JP
**0x11D**, caso `0x9CFD87`: trava 0x1F, `D4 D1`; KR 0x11A → JP 0x11E). O JP 0x119 é outro pacote (`u32` = 1 ou mostra
"Failed"): mandar o World Tour com 0x119 derrubava a entrada no canal (13/10/2026). O botão "sim" manda o 0xAD (`pk_cmp.tsv`). A referência AGPL só grava "campo concluído" e não responde ao 0xAD.

## Pacotes
- **S->C 0x11D** (KR 0x119) `u32 mapFlag, u8 gotGift`: preenche o contêiner `CWorldTourEvent` (conteúdo local 0x1E). O cliente liga
  `CanGift` quando `mapFlag == 0xFFFF && !gotGift`. Mandamos no login e quando um campo acende.
- **S->C 0x11E** (KR 0x11A) `u8`: 0 = volta para "pode pegar" (0xFFFF, sem prêmio); ≠0 = manda o MSN 0x27 do presente. Não usamos.
- **C->S 0xAD** `u32 guid`: botão "sim" do `FrWorldTourEventGiftDlg`. O cliente fecha a janela com resultado 1 e se marca
  como premiado sozinho (sem esperar resposta). O tradutor troca o id (o 0xAD viraria o KR 0xA5) por um interno
  (`JpSystems.CWorldTourGift`).

## Bits (CMapTypeAdapter::eMapFlag)
Campo 0..11 → bit 0..11 (Blue Lagoon .. Pink Wind); campo 12 não entra; campo 13..16 → bit 12..15 (Deep Inferno, Ice
Spa, Lost Seaway, Eastern Valley). Wiz City (19) tem bit 0x40000 mas fica fora do 0xFFFF; não conta.

## Regras do servidor
- Campo completo = 18 buracos terminados em stroke ou torneio, sem prática (como a referência).
- Prêmio: uma vez por conta, só com os 16 campos; vai por carta ("World Tour"). Itens e datas em `GameJp.WorldTour`
  (padrão: Memorial Coin Premium, 3 Gacha Ticket, 5 Spinning Cube) — os itens oficiais não são conhecidos [?].
- Banco: migração 022 `world_tour(account_id, map_flag, got_gift)`.
- **Desligado no cliente JP 983** (conferido no exe, ver abaixo): o botão chama `IsLocalContent(0x1F)` (a numeração do JP é
  outra) e o bit 0x1F não é ligado. O servidor manda o 0x119, mas o cliente não mostra nada. Fica pronto para um cliente
  com o conteúdo ligado.

## Conteúdos locais do JP 983 (`IsLocalContent`)
`IsLocalContent` = `0xA7A8D0` (ecx = id, bitset de 0xA7 bits em `0xF02E08`). Só `InitLocalizeSystem` (`0xA7A9B0`, chamado
pelo WinMain em `0xA72A66`) escreve no bitset; nenhuma outra função o altera. Simulando essa função, ficam ligados:
1–0xA, 0xD, 0xE, 0x12, 0x14, 0x1E, 0x23, 0x27–0x35, 0x39, 0x3C, 0x3E, 0x3F, 0x42, 0x45–0x47, 0x4C, 0x4E, 0x4F, 0x51–0x55,
0x57–0x5A, 0x5E, 0x60–0x62, 0x64, 0x65, 0x67, 0x68, 0x6A–0x6D, 0x6F, 0x70, 0x75, 0x7A, 0x81, 0x84, 0x92, 0x94, 0x9A, 0x9B,
0x9F, 0xA0, 0xA3–0xA5. **A numeração não é a do código KR** (rebang): os botões de evento da página inicial (tabela em
`0xE0D930`, funções instaladas por `0xCCFC60`) consultam:

| botão | id no JP | ligado? |
|---|---|---|
| Point Event | 0x16 | não |
| World Tour | 0x1F | não |
| Find Ebort | 0x21 | não |
| Halloween 2007 | 0x22 | não |
| Christmas 2007 | 0x4D | não |
| Ticket Exchange | 0x77 | não |
| Mission Event | 0x80 | não |
| Bingo | 0x82 | não |
| Treasure Island | 0x89 | não |
| Aniversário do Kooh | 0x8B | não |

Ou seja: no JP 983 esses eventos existem no código, mas o próprio cliente os deixa escondidos.

