# JP 983 — sistemas que só o JP tem e pacotes do login que ainda faltam

Pesquisa de 12/10/2026 (desmonte do ProjectG.exe 983 e servidor de referência da comunidade, AGPL-3.0). A camada JP
responde hoje o mínimo válido a cada pedido destes sistemas (`JpTranslator.JpOnly.cs`), para a tela não ficar esperando:

| Pedido JP | Resposta JP | Sistema | Hoje |
|---|---|---|---|
| 0x5C | 0xBA (SYSTEMTIME) | abrir a loja | hora do servidor, e o pedido segue para o KR |
| 0x140 | 0x20E | atualizar a loja | vazio |
| 0x16E / 0x16F | 0x248 / 0x249 | presença diária | já registrada hoje, sem prêmio |
| 0x157 | 0x22D + 0x22C | conquistas | lista vazia |
| 0x151 | 0x216 + 0x225 | missão diária | sem missões |
| 0x152 / 0x153 / 0x154 | 0x226 / 0x227 / 0x228 | aceitar / prêmio / desistir | recusa (1) |
| 0x126–0x129, 0x18D | 0x1E7–0x1EA, 0x274 | Tiki Shop | recusa |
| 0x164–0x16D | 0x23D–0x247 | oficina de club set | recusa |
| 0x187–0x18C | 0x26E–0x273 | maestria e cards no personagem | recusa |
| 0x155 | 0x22A | compor cards (Lolo) | recusa |
| 0x17F | 0x264 | Memorial Shop | recusa |
| 0x186 | 0x26C | Big Papel | recusa |
| 0x172, 0x173, 0xFB, 0xFE, 0x12F | — | ícone de evento, chaves web, fim da tacada | ignorados |

Modos: o Special Shuffle é o modo 18 no JP e 14 no KR (tipo de exibição 4); conversão em `KrToJp.ModeToJp/ModeFromJp`.

## A. Pacotes do login do JP que ainda não mandamos

Fontes: `ref/GameServer/Game/cLoginTask.cs:51-131` (`sendCompleteData`), `Game/Manager/LoginManager.cs:94-736` (respostas do banco
que também mandam pacote), builders em `PacketFunc/packet_func_sv.cs:6260-7940`. Leitura no exe conferida **no
desmonte do ProjectG.exe 983** (`<pasta do jogo>\ProjectG.exe`, despachante `CTask::OnPacketCommon` 0x009C20A0;
leitores: `0xA4DEB0`=u8, `0xA4DEC0`=u16, `0xA4DED0`=u32, `0xA4DF00`=bloco(n)) — marcado **[EXE]**.
Ids já são do JP (todos fora das faixas do tradutor, exceto 0xB4/0xF1/0x101/0x102, que o KR não manda nesse papel).

### Ordem da referência (depois do 0x44 de sucesso)
`LoginManager` caso 2 (logo depois de ler o equipamento, ANTES do 0x44): **0x1A9**. Caso 5: **0x11F**. Caso 6: **0x101**.
Caso 10→32: **0x210** (correio não lido). Caso 33: 0xD4 (caddie de férias, só se houver). Caso 34: 0xB2 (recados
offline, só se houver… a referência tem o `if` invertido). Caso 4: premium (0x9A/0x26D, ver B.e).
`sendCompleteData`: 0x44 → 0x70 → 0x71 → 0x73 → 0xE1 → 0x72 → 0x4D → **0x102** → 0x131 → **0x21D** (contadores das
conquistas, sempre) → **0x21E** (conquistas, só se houver) → **0xF1** → 0x135 → **0x144** → 0x138 → 0x136 → 0x137 →
**0x13F** → **0x181** → 0x96 → **0x169**(5) → **0x169**(0) → **0xB4**(5) → **0xB4**(0) → **0x158** → **0x25D**(5) →
**0x25D**(0) → [prêmio de login, se o evento estiver ligado: ver B.c].

Nenhum é necessário para chegar ao lobby (já confirmado no cliente real sem eles). Abaixo, "obrigatório" = o que
uma tela precisa para não ficar vazia/travada.

| id | layout exato | leitura no exe | mínimo válido | papel / obrigatório? |
|---|---|---|---|---|
| **0x1A9** | `u8 ligado, [u32 ttl_ms se ligado]` (ref. manda `u8 1, u32 getBotTTL()`) | [EXE] `u8`; se ≠0 `u32` e cria um timer; se 0 apaga o timer (0x9D2A39) | `A9 01 00` (desligado) | anti-bot (tempo de vida); opcional. Com 1 o cliente cria um temporizador [?] (não achei pacote de resposta tratado na referência) |
| **0x101** | ref.: `u8 0` | [EXE] o `case` cai direto no fim do switch (0x9D78A7): **ignorado** | não mandar (ou `01 01 00`) | inútil no 983 |
| **0x102** | `u32 cupom normal, u32 cupom parcial, u64 pang, u64 cookie` (24 B) | [EXE] D4 D4 D8 D8 | `0,0,pang,cookie` | cupons do Gacha JP + saldo; recomendável (atualiza pang/cookie do HUD) |
| **0xF1** | ref.: `u8 0` | [EXE] não lê nada; liga uma flag de "login completo", mostra avisos e, se houver um "ir para a sala" pendente (seguir amigo do mensageiro), manda **C->S 0xB4 `u8 opção, u16 nº sala`** | `F1 00` (+ `00` inofensivo) | sinal de "mensageiro/login prontos"; **mandar**. O C->S 0xB4 JP vira KR 0xAC (−8) = `CDirectMove` (u8, u16 sala) no nosso KR: mesma semântica, ok; a referência só loga |
| **0x144** | ref.: `u8 0` | [EXE] `u8 n`, `n × 67 B` (0x43, vai para uma lista; conteúdo [?]) | `44 01 00` | "pacote novo do JP" sem uso na referência; mandar vazio |
| **0x13F** | ref.: `u8 0` | [EXE] só no `CLobbyTask` (0xAC4A1D): `u8 n, n × u8` | `3F 01 00` | lista de u8 [?]; no login (antes do lobby) é ignorado; opcional |
| **0x181** | `i32 opção`; opção 0: `u8 n, n × ItemBuff 65`; opção 2: `u32 n, n × (u32 typeid, ItemBuff 65)`; outra: `u8 0` | [EXE] `u32`; se 2 → `u32`, se 1 → `u32 typeid`; depois re-lê o pacote em 0x813A70 (blocos de 0x41 = 65 B) | `81 01 00 00 00 00 00` (opção 0, n=0) | buffs por tempo (Yam/gold, rainbow…). ItemBuff 65 = `u32 id, u32 typeid, u32 typeid peça, u32 id peça, u32 efeito, u32 qtd efeito, u32 slot, SYSTEMTIME uso 16, SYSTEMTIME tempo 16 (s*0xFFFF|ms), u32 tipo, u8 usado` (`pangya_game_st.cs:6179`) |
| **0x169** | `u8 temporada, TrofelInfo 78` | [EXE] `u8` (0 = temporadas antigas, 4 ou 5 = atual; outro valor: descarta) + 78 | `69 01 05` + 78×0, e `69 01 00` + 78×0 | troféus (perfil/My Room). Referência manda 2: (5, atual) e (0, resto) |
| **0xB4** | `u8 temporada, u8 0, u8 n, n × (i32 id, u32 typeid, u32 qtd)` (a ref. escreve `i16 temporada` + `u8 n`) | [EXE] u8 temporada (0 ou 4..5), u8 ignorado, u8 n, n×12 | `B4 00 05 00 00` e `B4 00 00 00 00` | troféus especiais. **Atenção**: S->C 0xB4 ≠ C->S 0xB4 |
| **0x158** | `u8 temporada, u32 uid, UserInfo 265` | [EXE] D1 D4 DB(0x109); procura o jogador pelo uid | `58 01 00`, uid, estatísticas (as mesmas do bloco 0x44) | estatísticas do próprio jogador (perfil); o mesmo id é o perfil de outro jogador (hoje bloqueado: falta conversão) |
| **0x25D** | `u8 temporada, u32 total, u32 n, n × 12 (i32 id, u32 typeid, u32 qtd)`; páginas de 20 se > ~buffer | [EXE] D1 D4 D4 DB(0xC); temporada 0 ou 4..5 | `5D 02 05` + `0`+`0`, e com temporada 0 | troféus do Grand Prix |
| **0x210** | `i32 erro, i32 n, n × MailBox 196` | [EXE] `u32 erro` (≠0: mostra mensagem 0x27 com o código), `u32 n`, n×0xC4 | `10 02` + `0`(u32) + `0`(u32) | correio não lido no login. MailBox 196 = `i32 id, char[30] de, char[80] texto, 18 B ?, u32 visitas, u8 lido, u32 nº itens, item 55` (`pangya_game_st.cs:4571`) |
| **0x21D** | `u32 0 (ok), u32 total, u32 n, n × (u8 ativo, u32 typeid, i32 id, i32 valor)` (13 B) | [EXE] `u32 erro` (≠0 → mensagem 0x27), `u32 total`, `u32 n`, … ao completar o total, liga a UI | `1D 02` + 3×`u32 0` | contadores das conquistas; ver B.a. A referência manda sempre |
| **0x21E** | `u32 0, u32 total, u32 n, n × conquista` (ver B.a) | [EXE] mesmo cabeçalho | `1E 02` + 3×`u32 0` | conquistas; a referência só manda se houver |

Outros que o login JP já recebe do cliente e precisam de resposta (hoje descartados, ESTADO §8):
- C->S **0xFB** (vazio) → S->C **0x1AD** `i32 opção (1 ok/0 erro), pstr chave web` ([EXE] D4 DS). Mínimo: `AD 01 00 00 00 00 00 00`
  (opção 0 + string vazia u16 0). Chave para as páginas web (guilda/ranking web).
- C->S **0xFE** `u32` → S->C **0x1B1** `u32 hora_unix, u32 tamanho, bytes` (contexto UCC/Self Design: `u32 typeid, i32 id,
  char[9] idx` por item). [EXE] (0x9D3354): lê `u32 a, u32 len`; se `len>0 && a>0` lê `len` bytes e guarda; se os dois
  são 0, limpa. **Mínimo: `B1 01` + `u32 0` + `u32 0`.** (A referência manda um "padrão" fixo de 33 B; não precisa.)
- C->S **0x140** (vazio; entrar na loja) → S->C **0x20E** `u32 0, u32 0` ([EXE] D4 D4). Sem isso a loja pode não abrir [?].
- C->S **0x16E** (vazio; presença) → ver B.c (S->C 0x248).
