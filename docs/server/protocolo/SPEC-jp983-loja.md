# JP 983 × KR 645 — loja, economia, inventário, My Room e loja pessoal do lounge

Pesquisa de protocolo (12/10/2026). Pesquisa que guiou a camada JP (Pangya.Protocol.JP983/Game/JpTranslator.Shop.cs).

**Fontes e legenda**
- **[EXE]** = `docs/server/jp983-protocolo/pacotes_envio_983.tsv` (o que o cliente 983 escreve: E1/E2/E4/E8/ES/EB(n)) e
  `pacotes_recebe_983.tsv` / `rx_cmp.tsv` (o que ele lê: D1/D2/D4/D8/DS/DB(n)). As leituras vêm **na ordem dos endereços,
  com todos os ramos misturados**: uma sequência como `D4 D8 D8 DB(0x10)` lista o que algum ramo lê, não a ordem exata.
- **[REF]** = referência JP (C#), `GameServer/Game/Channel.cs`, `PacketFunc/packet_func_sv.cs`,
  `Game/Manager/PersonalShopManager.cs`, `Game/PersonalShop.cs`, `Models/pangya_game_st.cs`.
- **[KR]** = o nosso código: `src/Pangya.Protocol.KR645/Game/GameHandler.*.cs`, `Structs.g.cs`.
- **[TR]** = o tradutor de hoje (`JpTranslator.cs`, `KrToJp.cs`).
- **[?]** = incerto (falta conferir no cliente).
- Os ids são hexadecimais. "KR→JP" é o id de saída que o tradutor calcula hoje. "fora" quer dizer que o id cai fora das faixas
  e o pacote é descartado.

## 0. Resumo: o que está errado no tradutor e o que falta

### Pacotes que passam hoje só trocando o id, mas com layout errado (o cliente lê bytes errados)

| sentido | pacote JP | problema | conserto |
|---|---|---|---|
| C->S | **0x1D** comprar | O item do JP tem 37 bytes; o KR lê 16. A partir do 2º item, e no cupom do fim, os campos ficam deslocados. | converter (§1.2) |
| C->S | **0x1F** presente | mesmo problema (37 → 16) | converter |
| C->S | **0x7C / 0x7D** loja pessoal | item de 172 → KR 170; o 0x7C do JP não tem o `u32 tipo` | converter (§4) |
| S->C | **0xE6 / 0xEB / 0xED** (KR 0xE4/0xE9/0xEB) | sTradeItem do KR 170 → JP 172; o 0xE6 e o 0xEB do JP não têm o `u32 tipo` | converter ou pôr em `NotConvertedYet` |
| S->C | **0x16D** (KR 0x172) página do armário | sStoredItemInfo do KR 174 → JP 176 | converter |
| S->C | **0x12E** sub 1 (KR 0x126) Self Design | sItemInfo do KR 168 → JP 196 (já está bloqueado) | converter só o sub 1 |
| S->C | **0x6B** erro (KR 0x69 `u8 5`) | O JP lê `u8 erro, u8 tipo`; o KR manda só o primeiro byte. | O tradutor guarda o tipo do último 0x20 e completa. |
| S->C | **0xE2** erro do mascote (KR 0xE0 `u8 1, u32 id`) | O JP lê sempre `u8, i32, str, u64`. | completar com `str ""` e `u64 pang` |
| S->C | **0x6A** erro do presente (KR 0x68 `u32 código`) | O JP lê sempre `u32, u64, u64` (a referência sempre manda). | completar com pang e cookie |
| S->C | **0xA5** upgrade (KR 0xA3) | O 2º byte no KR é o pedido cru (bit0 club set, bit1 descer); no JP é `0` personagem / `1` club set. | `k[1] & 1` |

### Ids que faltam nas faixas (hoje caem "sem par" / "sem id")

| JP | KR | motivo |
|---|---|---|
| C->S 0xCC..0xD2 | 0xCE..0xD4 (**+2**) | Armário: a faixa "+2 de 0xD3" começa cedo demais. 0xCC–0xD2 têm a mesma assinatura do KR (pk_cmp: igual). JP 0xCB não é do armário. |
| C->S **0x158** | 0x7E + conversão | Caixa Mágica (`recycledlg`) |
| C->S **0x14B** | 0x6D | Jogar no Papel Shop (`bongdarishopdlg`) |
| S->C 0x16C | KR 0x171 (−5) | A faixa −5 começa em 0x172; o 0x171 (armário aberto) também é −5 ([REF] 0x16C = resultado da senha). |
| S->C 0x168 | KR 0x16E (−6) [?] | Avatar no My Room: o JP lê `DB(0x15c) DB(0x201)` = jogador na sala 348 + personagem 513 ([REF] requestEnterMyRoom → 0x168). |
| S->C 0x10B | KR 0x109 + conversão | Abrir o Papel Shop: o JP lê `D4 D4 D4` = `u32 erro, i64 limite`; o KR manda `u32 -1, u32, u32`, que o JP leria como erro. |
| S->C 0x21B | KR 0xD4 + conversão | Resultado do Papel Shop (o JP não usa 0xD6) |
| S->C 0x22F | KR 0xED + conversão | Resultado da Caixa Mágica (o JP não lê 0xEF [?]) |

### Sistemas que o KR não tem (novos no JP)

0x140 → 0x20E (atualizar a loja), 0x5C → 0xBA (hora), 0x1E (vender na loja), 0x20 tipos 9 (cut-in) e 10 (pôster),
0x155 (cards compostos Lolo), 0x18A–0x18C (cards no personagem), 0x186 (Big Papel), 0x17F (Memorial), 0x126–0x129 e
0x18D (Tiki Shop), 0x164–0x16D (oficina de club set), 0xEC (recarga de cometa) e o pacote genérico de itens **0x216**.
O mínimo para cada um está no §6.

---

## 1. Loja

### 1.1 Abrir, consultar cookies e "hora"

| função | JP C->S [EXE] | JP S->C | KR | conversão |
|---|---|---|---|---|
| abrir a loja (`CShopMain::OpenLayout`) | **0x5C** vazio, depois **0x3D** vazio, depois **0xFE** `u32` | 0x5C → **0xBA** `SYSTEMTIME` (16) [REF: sendDateTimeToSession; EXE lê `DB(0x10)`]. 0x3D → 0x96. 0xFE: sem resposta [REF: carrega UCC]. | 0x5C: sem resposta. 0x3D → 0x94 `u64 cookie`. 0xFE do KR é guilda (o JP 0xFE não tem par, então é descartado). | 0x5C: mandar 0xBA com a hora local (novo; recomendado, [?] se é obrigatório). 0x3D: igual, só o id (0x94 → 0x96). |
| atualizar a loja (`SendShopRefresh983`) | **0x140** vazio | **0x20E** `u32 0, u32 0` [REF; EXE lê `D4 D4`] | — | novo: responder 0x20E com 0, 0 |
| cookies | **0x3D** vazio | **0x96** `u64 cookie` [EXE `D8`] | 0x3D → 0x94 | igual, só o id |

### 1.2 Comprar — C->S 0x1D

| | JP 983 | KR 645 |
|---|---|---|
| pedido | **0x1D** `u8 opção (aluguel), u16 n, n × BuyItem(37), i32 id do cupom` [EXE `E1 E2 EB(0x25) E4`; REF requestBuyItemShop] | 0x1D `u8, u16 n, n × sBuyItem(16)` |
| item | **BuyItem 37 (0x25)**: `i32 id, u32 typeid, i16 dias, i16 ItemType, u32 qtd, u32 pang, u32 cookie, u8[13]` | **sBuyItem 16**: `i32 Idx, u32 TypeCode, u16 DayCount, i16 sChildItemIndex, u32 ItemCount` |
| itens entregues | **0xAA** `u16 n, n × 38` [EXE `D2 DB(0x26)`]; para o tid 0x1A000042 (ticket report) mais `u16, u16, SYSTEMTIME` no registro [EXE `D2 D2 DB(0x10)`]. A REF põe `u64 pang, u64 cookie` no fim (o EXE não mostra D8 [?]; bytes a mais no fim não fazem mal). | 0xA8 `u16 n, n × sBuyItemResult(38)` (o mesmo layout: `u32 tid, u32 id, u16 tempo, u8 tipo, u16 qtd, SYSTEMTIME fim, char[9] ucc`) |
| resultado | **0x68** `u32 código` (0: `+ u64 pang, u64 cookie`). O EXE lê `D4 D4 D4 D4 D4 D8 D8 DB(0x10) DB(0x10)`: os códigos 9/0xB têm SYSTEMTIME (igual ao KR). | 0x66, mesmo layout |
| cupom | **0x216** (pacote genérico, §6.1) com o cupom gasto [REF] | sem cupom |

**Conversão C->S 0x1D:** ler `u8, u16 n`; para cada item, copiar os 16 primeiros bytes dos 37 (os campos coincidem; pang,
cookie e os 13 bytes não são usados porque o servidor calcula o preço); descartar o `i32 cupom` (o KR não tem cupom de
desconto) [?: aceitar o cupom fica para depois].

**S->C:** 0xA8 → 0xAA: igual, só o id; opcionalmente, pôr `u64 pang, u64 cookie` no fim, como a referência. 0x66 → 0x68: igual, só o id.
Os códigos de erro do KR (2 sem pang, 3 código errado, 4 já tem, 0x13 não está à venda, 0x15 itens demais, 0x17 sem
cookie) são os da tabela do cliente da Ntreev. Os códigos da REF (1..11) são palpites dela. Manter os do KR [?].

**Item por tempo depois da compra:** o cliente manda 0x20 tipo 7 (como no KR, §2.1). No `CShopMain::Net_StartPeriodItem`
do JP, o pk_cmp mostra `P(0x17f) E4` no lugar de `P(0x20) E1 E4`, mas 0x17F é o Memorial na REF/`bongdarishopdlg`.
Provavelmente o símbolo foi mal resolvido. Conferir no cliente [?].

### 1.3 Presente — C->S 0x1F

| | JP | KR |
|---|---|---|
| pedido | **0x1F** `str nick, u32 uid, str mensagem, u8, u16 n, n × BuyItem(37)` [EXE `ES E4 ES E1 E2 EB(0x25)`]. A REF lê `u16` no lugar do nick (bug dela). | 0x1F `str nick, u32 uid, str msg, u8, u16 n, n × sBuyItem(16)` |
| resposta | **0x6A** `u32 código, u64 pang, u64 cookie` **sempre** (também no erro) [REF; EXE lê `D4 D8 D8 … DB(0x10) DB(0x10) D4`] | 0x68 `u32 0, u64, u64` / erro só `u32` |

**Conversão:** C->S: itens 37 → 16 (como no 0x1D). S->C 0x68 → 0x6A: no erro, completar com `u64 pang, u64 cookie`.
Atenção: o presente vira carta no correio, e o correio do JP é outro (0x210–0x215, item 0xC4). O aviso de carta nova e a
caixa não funcionam até a camada de correio do JP existir.

### 1.4 Vender / apagar item

| função | JP | KR | conversão |
|---|---|---|---|
| apagar item (`deleteitem.cpp`, `mascotinfodlg.cpp`) | **0x64** `u32 typeid, u32 qtd` [EXE] → **0xC5** `u8 1, u32 typeid, u32 qtd, i32 id` / `i8 -1` [REF; EXE lê `D1 D4 D4 D4`] | 0x64, mesmo layout → 0xA5 (→ JP 0xA7) + 0xA8 `u16 0` (→ JP 0xAA) | C->S igual. Resposta: o JP espera **0xC5** (= KR 0xC3, que o nosso KR não manda) [?]. Mínimo: além do 0xA7, mandar 0xC5 `u8 1, tid, qtd, id`. |
| vender na loja (`CShopMain::OnSellDlgResult`) | **0x1E** `u8, u8, u32, u32` [EXE]. Resposta desconhecida (a REF não trata) [?]. | KR: 0x1E "repay" sem chamador (SPEC-pacotes-restantes) | novo: sem implementação de referência. Fica no log. |

### 1.5 Aluguel (estender/apagar)

| | JP | KR | conversão |
|---|---|---|---|
| estender | **0xE6** `i32 id` [EXE `realmyroommainui`] → **0x18F** `u8 0, u32 tid, i32 id` / `u8 1`; a REF também manda **0xC8** `u64 pang, u64 custo` (o aluguel é cobrado) | 0xE8 → 0x194, mesmo layout (o KR estende de graça) | igual, só o id (0xE6 → 0xE8; 0x194 → 0x18F). Os ids já estão nas faixas. |
| apagar | **0xE7** `i32 id` → **0x190**, mesmo layout | 0xE9 → 0x195 | igual, só o id |

---

## 2. Inventário e My Room

### 2.1 Equipar — C->S 0x20 → S->C 0x6B (KR 0x69)

O EXE escreve (`realmyroommainui`/`shopmain`/`shoptask`/`shareddoc`) `u8 tipo` + dados. Resposta 0x6B: `u8 código (4 ok),
u8 tipo, dados (só se 4)` [REF pacote06B; EXE lê `D1 D1 DB(0x201) DB(0x28) D4 DB(0x18) DB(0x3e) D4 DB(0x10) DB(0x8) D4`].

| tipo | JP C->S | JP 0x6B dados | KR | conversão |
|---|---|---|---|---|
| 0 roupa | `EB(0x201)` personagem 513 | personagem 513 | sCharacterInfo 444 | já feita ([TR] `CharacterToKr` / `Character`) |
| 1 caddie | `u32 id` | `i32 id do caddie` | o KR não manda dados | já feita (o [TR] completa) |
| 2 itens | `EB(0x28)` u32[10] | u32[10] | igual | igual |
| 3 bola + club set | `u32 bola, u32 id` (`E4 E4`) | `u32 bola, i32 id do club set` | o KR não manda | já feita |
| 4 skins | `EB(0x18)` u32[6] | u32[6] | igual | igual |
| 5 personagem | `u32 id` | `i32 id` | igual | igual |
| 7 item por tempo (`Net_StartPeriodItem`) | `u32 id` (`E1 E4`) [?: o número 7 vem do KR] | sem resposta no KR; a REF não trata o 7 (devolve erro 1) | KR: `default` sem resposta | igual (conferir no cliente) |
| 8 mascote | `u32 id` | mascote 62 | mascote 63 | já feita (`Mascot`) |
| **9 cut-in** (`Net_EquipCutIn983`) | `u32 id do personagem, EB(0x10)` = u32[4] typeids do cut-in. **É este o "0x20 de 23 bytes"** (2 de id + 1 + 4 + 16): não é item por tempo. | `i32 id do personagem, u32[4] cut-in` (20) [REF; EXE `D4 DB(0x10)`] | o KR cai no `default` (sem resposta, o cliente fica esperando) | **novo**: gravar o cut-in no personagem (o CharacterInfo 513 tem os 16 bytes do cut-in em 440..455, ver `KrToJp.Character`) e responder 0x6B tipo 9 |
| **10 pôster** (`Net_EquipPoster983`) | `EB(0x8)` u32[2] typeids (móveis) | u32[2] | — | **novo**: gravar no UserEquip (o 0x72 do JP tem 2 pôsteres no fim: hoje vão zerados) e responder 0x6B tipo 10 |

**Erro:** o KR manda 0x69 `u8 5` (1 byte); o JP lê `u8 erro, u8 tipo`. O tradutor deve lembrar o tipo do último 0x20 do
cliente e mandar `u8 5, u8 tipo` (a REF usa 1 ou o código de erro e sempre manda o tipo).

### 2.2 Troca rápida (0x0B/0x0C → 0x4B)
Já convertido (`QuickEquip`: `i32 0` + tipo + uid + struct). Igual ao SPEC.

### 2.3 Entrar no My Room, móveis, privacidade

| função | JP C->S | JP S->C | KR | conversão |
|---|---|---|---|---|
| entrar (`MakeRealMyRoom`) | **0xB5** `u32 eu, u32 dono` | **0x12B** `u32 1, u32 dono, MyRoomConfig 103` = `D4 DB(0x6b)` | 0xAD → 0x123 `u32 1, sRealMyRoomAuthority 107` (u32 dono, u16, u8 privado, char[100] senha) | igual, só o id (−8 / +8) |
| presença (`SendRealMyRoomEnter983`) | **0xB7** `u32 uid, u8 1` | **0x168** jogador na sala 348 + personagem 513 [REF `PlayerRoomInfoEx.ToArrayEx`; EXE `DB(0x15c) DB(0x201)`], depois **0x12D** `u32 1, u16 n, n × móvel 27` [EXE `D4 D2 DB(0x1b)`] | 0xAF → 0x16E (sSlotInfo 338 + sCharacterInfo 444) + 0x125 (móveis 27) | 0x16E → **0x168** (fora da faixa; mapear à mão [?: −6]) com `Slot` + `Character`; 0x125 → 0x12D: igual, só o id |
| salvar móveis (`RMR_UpdateDB`) | **0xB6** `u16 n, EB(0x1b)` | **0x12C** `u32` [EXE `D4`] | 0xAE → 0x124 | igual, só o id |
| privacidade | **0xB8** `EB(0x6b)` | — | 0xB0 | igual, só o id |
| lugar / usar móvel | **0xC1** `u8`, **0xD7** `u32, u32` | — | 0xB9 / 0xD9 | igual, só o id |

### 2.4 Upgrade de atributo (personagem / club set) — C->S 0x4B

| | JP | KR |
|---|---|---|
| pedido | **0x4B** `u8 opção, u8 atributo, i32 id` (opção: bit0 = club set, bit1 = descer) [REF; EXE `E1 E1 E4`, o `EB(0x201)` que aparece no fortunedlg é outra coisa] | 0x4B, mesmo layout |
| resposta | **0xA5** `u8 resultado (1 subiu, 2 desceu, 0 erro), u8 0 personagem / 1 club set, u8 atributo, i32 id, i64 pang` [REF; EXE `CRealMyRoomTask` lê `D1 D1 D1 D4 … D8`] | 0xA3 `u8 resultado (1 Up, 2 Down, 3..6 erros) [+ u8 opção crua, u8 atributo, u32 id, i64 custo]` |

**Conversão 0xA3 → 0xA5:** `k[1] = k[1] & 1`. Os erros 3..6 do KR: a REF manda só `u8 0` [?]. Mínimo: mandar o código do KR
(o cliente deve mostrar "falhou"); se não mostrar, trocar para 0.
**Oficina de club set** (0x164–0x16D → 0x23D–0x246 + 0x216, nível/rank/maestria) é sistema novo (§6).

### 2.5 Mensagem do mascote

| | JP | KR | conversão |
|---|---|---|---|
| pedido | **0x73** `i32 id, str` [EXE `E4 ES`] (e 0x9A `u8, u32, str` do PC bang) | 0x73 | igual |
| resposta | **0xE2** `u8 4, i32 id, str mensagem, u64 pang`; erro `i8 -1, i32 -1, u16 0, u64 pang` [REF; EXE `D1 D4 DS D8`] | 0xE0 `u8 4, u32, str, u64` / erro `u8 1, u32 id` | no erro, completar com `str "" , u64 pang` |

### 2.6 Caddie (férias, recontratar)
- 0x6B `u32 caddie, u8` (aviso de férias): igual ao KR, sem resposta.
- 0x39 `u32 caddie` → **0x93** `u8 2, i32 id, u64 pang` / `u8 1` [REF]. O KR responde 0x91 com o mesmo layout → +2 = 0x93: igual, só o id.

### 2.7 Armário (Dolfini Locker)

Struct: **DolfiniLockerItem JP 176 (0xB0)** = `u64 índice + TradeItem 168`. **sStoredItemInfo KR 174** = `u32 dwStorageID + sTradeItem 170`.
Os dois têm 8 bytes de cabeçalho (KR: u32 storage, i32 iIndex; JP: u64). Depois disso, o mesmo TradeItem com a diferença
do §4 (o JP tem um u16 a mais em 107 do TradeItem). Então: **JP = KR[0..111) + u16 0 + KR[111..174)**, e na volta tira
os 2 bytes em 111.

| função | JP C->S [EXE `itemstorage.cpp`] | KR C->S | JP S->C (KR→JP) | conversão |
|---|---|---|---|---|
| abrir com senha | **0xCC** `str` | 0xCE | **0x16C** `u32` (0 ok, 0x75 senha errada) [REF]; o cliente então pede 0xD5 e 0xCD [EXE] | id C->S +2 (hoje não tem par); KR 0x171 → 0x16C (fora da faixa: mapear −5) |
| página | **0xCD** `u32, u16 página` | 0xCF | **0x16D** `u16 páginas, u16 página, u8 n, n × 176` [EXE `D2 D2 D1 DB(0xb0)`] | id +2; S->C (KR 0x172) **itens 174 → 176** |
| guardar | **0xCE** `u8 n(=1), 176` | 0xD0 `u8, 174` | **0x16E** `u32` [EXE `D4`] (a REF também manda 0xEC e 0x139) | id +2; C->S 176 → 174 |
| retirar | **0xCF** `u8 n(=1), 176` | 0xD1 | **0x16F** `u32` | idem |
| criar senha | **0xD0** `str` | 0xD2 | **0x176** `u32` | id +2 (KR 0x17B → 0x176 ok) |
| trocar senha | **0xD1** `str velha, str nova` | 0xD3 | **0x174** `u32` | id +2 (KR 0x179 → 0x174) |
| modo de entrada | **0xD2** `u8, str` | 0xD4 | **0x173** `u32, u8` [EXE `D4 D1`] | id +2 (KR 0x178 → 0x173) |
| estado | **0xD3** vazio | 0xD5 | **0x170** `u32 0, u32 tem senha` [EXE `D4 D4`] | igual (já na faixa) |
| depositar/sacar pang | **0xD4** `u8, u64` | 0xD6 | **0x171** `u32` + 0xC8 + **0x172** `u64` | igual |
| pang do armário | **0xD5** vazio | 0xD7 | **0x172** `u64` | igual |

O KR atualiza o inventário com 0xA5/0x71 (→ 0xA7/0x73, que o JP lê). A REF usa 0xEC (`u32 n, u8 1 guardou/0 retirou,
u64 pang, n × [u32 0 + TradeItem 168] (+ u8 3 + WarehouseItem 196 ao retirar)`). O caminho do KR deve bastar [?].

### 2.8 Cards

| função | JP C->S | JP S->C | KR | conversão |
|---|---|---|---|---|
| listas do login | — | **0x135** limpa, **0x138** `u32, u16 n, n × sCards 58`, **0x136** limpa, **0x137** `u16 n, n × 65` | 0x12D/0x130/0x12E/0x12F (+8) | igual, só o id (o [TR] manda vazio hoje) |
| abrir pacote | **0xCA** `u32 tid, i32 id` [EXE `cardboxdlg`/`ccardbook`] | **0x154** `u32 0, sCards 58 (o pacote), u8 n, n × (sCards 58 + u32 1)` / `u32 1` [EXE `D4 DB(0x3a) D1 DB(0x3a) D4`] | 0xC2 → 0x14C, mesmo layout | igual, só o id |
| usar card especial | **0xBD** `u32 tid` | **0x160** `u32 0, 58 = (u32 id, tid, tid da peça, id da peça, slot, u32 1, SYSTEMTIME início, fim, u16)` [REF]. EXE lê `D4 D4 D1 DB(0x3a)`: o erro do JP pode ter `u32 + u8` [?] | 0xB5 → 0x158 (sucesso igual; erro `u32 1, u8 1`) | igual, só o id (o erro do KR já manda u32 + u8) |
| encaixar card na peça | **0xC8** `EB(0x3a)` [EXE ccardbook] | 0x160 | 0xC0 → 0x158 | igual, só o id |
| remover (removedor) | **0xE0** `u32 tid, u32 id, u32 tid da peça, u32 id da peça` [EXE cardremove] | **0x188** `u8 1, u32 id da peça, u32 tid, u8?` / `u8 0, u8 0` [EXE `D1 D4 D4 D1`] | 0xE2 → 0x18D | igual, só o id |
| **card no personagem** (slots 1–4 personagem, 5–8 caddie, 9–12 NPC) | **0x18A** / **0x18B** (com patcher) `CardEquip 20 = u32 tid do personagem, i32 id, u32 tid do card, i32 id, u32 slot`; **0x18C** `CardRemove 20 = u32 tid do personagem, i32 id, u32 tid do removedor, i32 id, u32 slot` | 0x216 (itens) + **0x271/0x272/0x273** `u32 código, u32 tid` | — | **novo** (os cards do personagem ficam nos 48 bytes do fim do CharacterInfo 513 (465..512), hoje zerados) |
| **compor (Lolo)** | **0x155** `u64 pang, u32[3] tids` (+ `u8 tipo` na Ex) [REF; EXE `E8 E4…`] | 0xC8, 0x216, **0x229** `u32 tipo`, **0x22A** `u32 código, u32 tid` [EXE `D4 D4`] | — | **novo** |

### 2.9 Self Design (UCC)

| | JP | KR | conversão |
|---|---|---|---|
| chave de upload | **0xC9** `u8 sub, u32 uid, u8 conteúdo, u32 guid [, str]` [EXE overlaycursor] → **0x153** `u8 0, u8 conteúdo, u32 guid, str chave, u8 1` [EXE `D1 D1 D4 DS D1…`] | 0xC1 → 0x14B | igual, só o id |
| registrar/consultar/copiar | **0xB9** `u8 sub …` (sub 0: `u32 tid, str índice, str nome`; 1: `u32 guid, u8`; 2: `u32 tid, str, u16, u32`; 3: `u32 tid, str`) [EXE] | 0xB1, mesmos subs | C->S igual |
| resposta | **0x12E**: sub 0 `u8 0, u8 ok, u32 guid, u32 tid, str, str`; **sub 1 `u8 1, u32 guid, str índice, u8 flag, WarehouseItem 196`**; sub 2 `u32 tid, str, u16, u32, u32, u32, str, u16, u8`; sub 3 `u32 tid, str, u8 ok` [EXE `…D1 DB(0xc4)…`] | 0x126 (sub 1 com sItemInfo 168) | tirar 0x12E do `NotConvertedYet`; converter só o sub 1 (`KrToJp.Item` no fim); os outros: igual |

### 2.10 Caixas, cupons, bolsa da sorte, Spin Cube

| função | JP C->S | JP S->C | KR | conversão |
|---|---|---|---|---|
| **Spin Cube** / caixa com chave (`frrandomboxprogress`, `specialboxdlg`) | **0xEF** `u32 tid da caixa` [REF requestOpenBoxMail: cubo e caixa de papel] | 0x216 + 0xA7 + 0xAA `u16 0/1 … + u64 pang, u64 cookie` + **0x19D** `u32 código, u32 caixa, u32 prêmio, i32 qtd` [EXE `D4 D4 D4 D4`] | 0xF1 → 0xA5 + (0xC6/0x71/0xA5) + 0x1A2, mesmo layout | igual, só o id (0xEF → 0xF1; 0x1A2 → 0x19D). Se o prêmio for para o correio, o aviso do KR (0x15E) não chega no JP. |
| caixa de evento S4 / caixas do My Room (`OnOpenS4EventGiftDlgResult`) | **0xB2** `u32 tid` | REF: **0x129** `u8 0, u32 tid da caixa, i32 qtd restante, u32 n, n × (u32 tid, i32 id, i32 qtd, u8[8])` (20) + 0xAA [EXE `CRealMyRoomTask` 0x129 `D1 D1 D4 D4 D4 DB(0x14)`] | 0xAA → 0xA5 + 0xC6 + 0x1A2 | o id já bate (0xB2 → 0xAA). O JP também lê 0x19D no My Room; se a tela não fechar, responder no formato 0x129 [?] |
| envelope de ano novo | **0x93** `u32 id` | KR 0xDA → 0xDC `u32 id` (0xDC não aparece nas leituras do EXE [?]) | 0x90 | igual, só o id [?] |
| caixa de evento | **0x94** `u32 id` | KR 0xCB → **0xCD**: o EXE (`CRealMyRoomTask`) lê `D1 D4 D4 D2`; o KR manda só `u8 6/1` [?] | 0x91 | completar com `u32 0, u32 0, u16 0` [?] |
| **bolsa da sorte** | o JP **não manda 0x59** (nenhuma escrita no EXE). O `CRealMyRoomMain::OnFortuneDlgResult` manda **0x151** vazio, que na REF é a missão diária. [?] | (KR 0x121 → fora; parece o 0x129 do JP com um u32 a menos) | 0x59 → 0x121 `u8 0, u32 id, u32 1, 20 bytes` | Não existe como no KR. Se o item 0x1A000004 existir no iff do JP, abrir pelo 0xB2/0x129 [?]. |
| cupom (Gacha) | 0x9E [REF requestUpdateGachaCoupon] | 0x102 / 0x44 | — | fora do escopo |

### 2.11 Caixa Mágica do caddie (troca de materiais)

| | JP | KR |
|---|---|---|
| pedido | **0x158** `u16 receita (seq), u32 vezes, u8 n, n × (u32 tid, i32 id)` [EXE `recycledlg`: `E2 E4 E1 E4 E4`; REF requestCadieCauldronExchange] | **0x7E** `u16 receita, u8 vezes, u8 n, n × (u32 tid, u32 id)` |
| resposta | **0x22F** `u32 código, u32 receita, u32 n, n × (u32 tid, i32 id, i32 qtd ganha, i32 qtd total, u32 flag de tempo)` (20) [REF; EXE `D4 D4 D4`] + 0x216 com os materiais gastos | **0xED** `u32 0, u16 receita, u8 n, n × sRecycleItem 0x50 (u32 tid, u32 id, u16 qtd, zeros)` + 0xA5/0x71 antes |

**Conversão:** C->S 0x158 → 0x7E: `u16`, `u32 → u8` (limitado a 1..255), `u8 n`, materiais iguais.
S->C 0xED → **0x22F**: `u32 código, u32 receita (do u16), u32 n`, e por item `u32 tid, i32 id, i32 qtd (u16 do KR), i32 qtd
total (do inventário: `kr.Player.Find(id)`), u32 0`. Erro: `u32 código`. O 0xA5/0x71 do KR continuam (→ 0xA7/0x73) [?: a
REF usa o 0x216].

### 2.12 Papel Shop

| função | JP | KR | conversão |
|---|---|---|---|
| abrir | **0x98** vazio (`OnUnderBar_BongdariShopUp`) → **0x10B** `u32 0 ok, i64 jogadas restantes no dia (-1 = sem limite)` [REF; EXE `D4 D4 D4`] | 0x95 → 0x109 `u32 -1, u32 0, u32 0` | C->S igual (−3). S->C **0x109 → 0x10B** com conversão: `u32 0, i64 -1` (o `u32 -1` do KR viraria código de erro) |
| jogar | **0x14B** vazio (`bongdarishopdlg`) | 0x6D | **mapear JP 0x14B → KR 0x6D** |
| resultado | **0x21B** `u32 código, i32 id do cupom usado (0), u32 n, n × (u32 cor, u32 tid, u32 id, u32 qtd, u32 tipo) (20), u64 pang, u64 cookie` [EXE `D4 D4 D4 DB(0x14) D8 D8`]. Antes: 0x216 com os itens, 0xFB `i32 restantes, i32 …` [EXE `D4 D4`]. | 0xD3 `u32 cupom` (→ 0xD5 ok) + **0xD4** `u32 código, u32 n, n × sBonusBall 20 (i32 cor, u32 tid, u32 id, u32 qtd, u8 classe, u8 0, u16 0), u64, u64` + 0xF9 `u32, u32` (→ 0xFB ok) | **0xD4 → 0x21B**: pôr `i32 cupom` depois do código (o tradutor guarda o u32 do 0xD3 anterior; ou 0); a bola de 20 bytes já bate (`u32 tipo` = u8 classe + 3 zeros). A REF diz que, com o id certo, o cliente não duplica o item do 0x216. Se o item não aparecer no inventário, mandar também 0x73 [?]. |
| Big Papel | **0x186** vazio → 0xC8, 0x216, 0xFB, **0x26C** (mesmo layout do 0x21B) | — | novo |
| Memorial | **0x17F** `u32 tid da moeda` → 0x216 + **0x264** `u32 código, u32 n, n × (i32 tipo, u32 tid, u32 qtd)` [EXE `D4 D4 D4 D4 D4`] | — | novo |

### 2.13 Raspadinha (Scratchy)

| | JP [EXE `scratchdlg`] | KR | conversão |
|---|---|---|---|
| raspar | **0x70** vazio | 0x70 → 0xD3 + 0xDB | igual |
| resultado | **0xDD** `u32 código, u32 n, n × 20` [EXE `D4 D4 DB(0x14)`]; **0xD5** `u32` (cartão gasto) | 0xDB / 0xD3 | igual, só o id |
| serial | **0x71** `u32, EB(0xd)` → **0xDE**: o EXE lê `D4 D4 D4`; o KR manda `u32 2` | 0x71 → 0xDC | completar com `u32 0, u32 0` [?] |
| recarregar | (0x72 não aparece no EXE do JP) | 0x72 | — |
| A REF não implementa a raspadinha. |

### 2.14 Tiki Shop (só no JP; o KR tem a Caixa Mágica no lugar)
- Antigo: **0x126** abrir → **0x1E7**; **0x127** pontos → **0x1E8**; **0x128** `u8 n, n × item` → 0x216 + **0x1E9**;
  **0x129** `u8 n, n × TP` → 0x216 + **0x1EA**. **0x18D** `u32 n, n × (u32 tid, i32 id, u32 qtd)` → 0xC8 + 0x216 +
  **0x274**. Sistema novo (§6).

---

## 3. Itens no inventário: pacotes comuns

| JP | layout | KR | conversão |
|---|---|---|---|
| **0x73** lista/novo item | `u16 total, u16 n, n × WarehouseItem 196` | 0x71 168 | já feita (`Item`) |
| **0xA7** contagem | `u8 n, n × (u32 tid, i32 id, u16 qtd)` [EXE `D1 D4 D4 D2`] | 0xA5 | igual, só o id |
| **0xC8** pang | `u64 pang, u64 gasto` (o EXE lê só `D8`) | 0xC6 | igual |
| **0x96** cookie | `u64` | 0x94 | igual |
| **0xE1** mascotes, **0x70/0x71/0x72** | já feitos | | |
| **0x7A** (KR 0x78 caixa de presentes) | o JP **não lê nada** (rx_cmp: "difere"; os presentes viraram correio 0x210+) | 0x78 no login | inofensivo |
| **0x216** (só no JP) | §6.1 | — | não é preciso enquanto o 0x73 e o 0xA7 servirem [?] |

---

## 4. Lounge: loja pessoal e troca

**Struct do item.** sTradeItem KR **170** = `i32 iIndex, u32 tid, u32 guid, i32 qtd, u8 tipo, u16 tempo, i64 preço,
u32 custo de upgrade, i16[5], u16 flag, char[9] ucc, u16 seq, u8 status, u32[4][3] cards (48), u16 slots personagem,
u16 slots caddie, char[41] nome, char[22] copiador`.
PersonalShopItem JP **172 (0xAC)** = `u32 índice + TradeItem 168`, com os mesmos campos até o offset 107 (`u8 tipo + u16
tempo` = `u8[3]` da REF); **no 107 entra `u16 slots NPC`**; depois nome 41 e copiador 22.
**Conversão: JP = KR[0..107) + u16 0 + KR[107..170); na volta, tirar os 2 bytes em 107.**

| função | JP C->S [EXE `shareddoc`/`tradedlg`] | KR C->S | JP S->C [EXE] | KR S->C | conversão |
|---|---|---|---|---|---|
| reabrir | 0x74 vazio | 0x74 | **0xE3** `u32 1, str nick` (`D4 DS`) | 0xE1 | igual, só o id |
| fechar | 0x75 vazio | 0x75 | **0xE4** `u32 1, str nick, u32 uid` | 0xE2 | igual |
| editar | 0x76 vazio | 0x76 | **0xE5** `u32 1, str nick, u32 uid` | 0xE3 | igual |
| entrar na loja | 0x77 `u32 dono` | 0x77 | **0xE6** `u32 1, char[22] nick, str título, u32 uid, u32 n, n × 172` (`D4 DB(0x16) DS D4 D4 DB(0xac)`) | 0xE4 `u32 1, **u32 tipo**, char[22], str, u32, u32 n, n × 170` | **tirar o `u32 tipo`, itens 170 → 172** |
| sair | 0x78 `u32` | 0x78 | **0xE7** `u32` | 0xE5 | igual |
| título | 0x79 `str` | 0x79 | **0xE8** `u32 1, str título, u32 uid, str nick` (`D4 DS D4 DS`) | 0xE6 | igual |
| visitantes | 0x7A | 0x7A | **0xE9** `u32 1, u32 n` | 0xE7 | igual |
| renda | 0x7B | 0x7B | **0xEA** `u32 1, u64` | 0xE8 | igual |
| publicar | **0x7C** `u32 n, n × 172` (`E4 EB(0xac)`, sem tipo; REF `openShop`) | 0x7C `u32 tipo, u32 n, n × 170` | **0xEB** `u32 1, char[22] nick, u32 uid, u32 n, n × 172` (`D4 DB(0x16) D4 D4 DB(0xac)`) | 0xE9 `u32 1, **u32 tipo**, char[22], u32, u32 n, n × 170` | C->S: **pôr `u32 1` (tipo)**, itens 172 → 170. S->C: **tirar o tipo**, itens 170 → 172. |
| comprar | **0x7D** `u32 dono, 172` | 0x7D `u32, 170` | **0xEC** `u32 1, u8 1 vendedor / 0 comprador, u64 pang, 172, u8 grupo, struct` [EXE `D4 D1 D8 DB(0xac) D1 DB(0xc4)/DB(0xc4)/DB(0x19)/DB(0x3a)`]. O grupo é 1 = item (ITEM, empilhável), 3 = outros, 5 = card; a struct é WarehouseItem 196 / caddie 25 / card 58. **A REF manda o grupo e a struct ao vendedor também.** | 0xEA vendedor `u32 1, u8 1, u64, 170`; comprador `u32 1, u8 0, u64, 170, u8 1 pilha / 2 novo, sItemInfo 168` | C->S: 172 → 170. S->C (já em `NotConvertedYet`): item 170 → 172; byte depois: o grupo do JP [?: o `1/2` do KR pode ser outra coisa]; sItemInfo 168 → 196; ao vendedor, completar com `u8 grupo + 196` do que sobrou (`kr.Player.Find(id)`) [?] |
| item restante | — | — | **0xED** `str nick, u32 uid, 172, u32 1/3` (`DS D4 DB(0xac) D4`) | 0xEB | item 170 → 172 |
| venda em pacote | — | 0x11A → 0x1D7 | — | — | o KR 0x1D7 cai fora (não vai); o JP 0x11A não aparece |

**Estado do avatar no lounge:** JP **0xEB** C->S `u32 guid` (`avatar_main`) → KR 0xED ✓ → KR 0x19B `u32 guid, 5 × f32` →
JP **0x196** (−5 ✓). A REF manda `u32 oid + 4 × f32` (zoom, cabeça, velocidade, ?); o EXE lê `D4 DB(0x4)` (em laço).
Mandar 5 f32 deve ser inofensivo [?].

**Troca direta** (`privatetrade.cpp`): JP **0xD9** `u8 sub, u32, str [, EB(0x428)…]` → KR 0xDB (+2 ✓). O KR desliga a troca
direta (só fica no log). A estrutura de 0x428 bytes não foi analisada.

---

## 5. Tabela rápida de ids (escopo deste documento)

C->S (JP → KR):

| JP | KR | estado |
|---|---|---|
| 0x1D, 0x1F | 0x1D, 0x1F | **converter** os itens 37 → 16 |
| 0x20 | 0x20 | tipos 9/10 novos; o tipo 0 já é convertido |
| 0x3D, 0x5C | 0x3D, 0x5C | 0x5C: responder 0xBA |
| 0x4B, 0x64, 0x6B, 0x70, 0x71, 0x73, 0x74–0x7B | iguais | ok (resposta do 0x64: §1.4) |
| 0x7C, 0x7D | 0x7C, 0x7D | **converter** (§4) |
| 0x93, 0x94, 0x95, 0x96, 0x98 | 0x90, 0x91, 0x92, 0x93, 0x95 | ok (−3) |
| 0xB2, 0xB5–0xB9, 0xBD, 0xC1, 0xC8–0xCA | 0xAA, 0xAD–0xB1, 0xB5, 0xB9, 0xC0–0xC2 | ok (−8) |
| **0xCC–0xD2** | **0xCE–0xD4** | **faltam** (+2); 0xCE/0xCF convertem 176 → 174 |
| 0xD3–0xD5, 0xD7, 0xD9, 0xE0, 0xE6, 0xE7, 0xEB, 0xEF | 0xD5–0xD7, 0xD9, 0xDB, 0xE2, 0xE8, 0xE9, 0xED, 0xF1 | ok (+2) |
| **0x14B** | **0x6D** | falta |
| **0x158** | **0x7E** | falta + conversão |
| 0x140, 0x1E, 0x155, 0x17F, 0x186, 0x18A–0x18D, 0x126–0x129, 0x164–0x16D, 0xEC | — | novos |

S->C (KR → JP):

| KR | JP | estado |
|---|---|---|
| 0x66, 0x68, 0x69, 0x71, 0x94, 0xA3, 0xA5, 0xA8, 0xC6, 0xE0 | 0x68, 0x6A, 0x6B, 0x73, 0x96, 0xA5, 0xA7, 0xAA, 0xC8, 0xE2 | 0x6A/0x6B/0xA5/0xE2 precisam de ajuste (§0) |
| 0xCB, 0xD3, 0xDA, 0xDB, 0xDC, 0xF9 | 0xCD, 0xD5, 0xDC, 0xDD, 0xDE, 0xFB | ok; 0xCD/0xDE [?] |
| 0xD4 | **0x21B** (não 0xD6) | remapear + conversão |
| 0xED | **0x22F** (não 0xEF) | remapear + conversão |
| 0x109 | **0x10B** | remapear + conversão |
| 0xE1–0xEB | 0xE3–0xED | 0xE6, 0xEB, 0xEC, 0xED: itens 170 → 172 (e o tipo) |
| 0x123, 0x124, 0x125, 0x126, 0x12D–0x130, 0x14B, 0x14C, 0x158 | 0x12B, 0x12C, 0x12D, 0x12E, 0x135–0x138, 0x153, 0x154, 0x160 | ok; 0x12E sub 1: converter |
| 0x16E | **0x168** [?] | remapear + Slot/Character |
| 0x171 | **0x16C** | remapear (−5) |
| 0x172–0x17B | 0x16D–0x176 | 0x16D: itens 174 → 176 |
| 0x18D, 0x194, 0x195, 0x19B, 0x1A2 | 0x188, 0x18F, 0x190, 0x196, 0x19D | ok |
| 0x121 (bolsa), 0x1D7 (pacote) | — | sem par no JP |

---

## 6. Sistemas que só o JP tem: o mínimo para funcionar

### 6.1 Pacote genérico de itens 0x216 (S->C)
`u32 hora unix, u32 n, n × stItem216 (50)`: `u8 tipo, u32 typeid, i32 id, u32 flag de tempo, i32 qtd antes, i32 qtd
depois, i32 qtd (ou tempo), i16[5] c, char[9] ucc, u8 seq, u32 tid do card, u8 slot do card`. A REF usa este pacote para
tudo que muda itens nos sistemas novos. O EXE lê só o cabeçalho `D4 D4` (o laço ficou fora da análise [?]). **Mínimo:**
um escritor no tradutor/handler JP a partir de `Item` (antes/depois), usado pelos sistemas abaixo.

### 6.2 Por sistema (o que cada um precisa no mínimo)
| sistema | C->S | S->C | mínimo |
|---|---|---|---|
| hora da loja | 0x5C | 0xBA SYSTEMTIME | responder (1 linha) |
| atualizar a loja | 0x140 | 0x20E `u32 0, u32 0` | responder (1 linha) |
| cut-in | 0x20 tipo 9 | 0x6B tipo 9 | gravar u32[4] no personagem; sair no CharacterInfo (440..455) |
| pôster | 0x20 tipo 10 | 0x6B tipo 10 | gravar u32[2] no equipamento; sair no UserEquip (fim) |
| vender na loja | 0x1E `u8, u8, u32, u32` | ? | log; descobrir a resposta no cliente [?] |
| cards no personagem | 0x18A/0x18B/0x18C | 0x216 + 0x271/0x272/0x273 | Domain: 12 slots de card por personagem; sair nos 48 bytes do CharacterInfo |
| Lolo (compor cards) | 0x155 | 0xC8, 0x216, 0x229, 0x22A | sorteio por raridade (dados do JP) |
| Big Papel / Memorial | 0x186 / 0x17F | 0x26C / 0x264 (+0x216, 0xFB) | dá para reaproveitar o LotteryService com as tabelas do JP |
| Tiki Shop | 0x126–0x129, 0x18D | 0x1E7–0x1EA, 0x274 | pontos Tiki + troca; tabela do JP |
| oficina de club set | 0x164–0x16D | 0x23D–0x246 (+0x216, 0xC8) | o ClubSetInfo 28 já tem os slots; falta nível/rank/maestria no Domain |
| recarga de cometa | 0xEC `u32 item, u32 bola` (→ KR 0xEE, que não existe) | 0x197 `u8 1, u32, u32, u16` | pequeno |

Todos dependem dos **dados do JP** (preços, flags da loja, tabelas): JP983-ESTADO §1.

## 7. Ordem sugerida (o mínimo para a loja e o inventário funcionarem)
1. **Conversões C->S 0x1D/0x1F** (hoje compram errado) e bloquear ou converter 0xE6/0xEB/0xED/0x16D/0x7C/0x7D.
2. Ajustes de erro: 0x6A, 0x6B (tipo), 0xE2, 0xA5.
3. Armário: faixa C->S 0xCC–0xD2 (+2), KR 0x171 → 0x16C, conversão 174 ↔ 176.
4. My Room: KR 0x16E → 0x168. Self Design: 0x12E sub 1.
5. Papel Shop (0x98/0x10B, 0x14B/0x21B), Caixa Mágica (0x158/0x22F).
6. Novos pequenos: 0x5C → 0xBA, 0x140 → 0x20E, 0x20 tipos 9 e 10.
7. Loja pessoal (§4) e, depois, os sistemas do §6.2.
