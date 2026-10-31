# JP 983: o que o cliente manda na sala/partida e hoje cai no chão (assist, auto command, efeitos de item, marcador, buffs, troca de servidor, ticker, recarga de cometa, premium, intrusão, fim do approach)

12/10/2026. Pesquisa só de leitura; nada foi alterado no repositório. Continua o §3.15/§4 de `missoes-lojas-eventos.md`
(e corrige dois pontos de lá: o 0x26A só tem o código, e os efeitos 0x237/0x24C/0x265/0x266/0x26B/0x280 começam com um
**código**, que a referência esquece no modo versus).

Marcas:
- **[EXE ✓]** conferido no `<pasta do jogo>\ProjectG.exe` (capstone; leitores `0xA4DEB0` u8, `0xA4DEC0` u16,
  `0xA4DED0` u32, `0xA4DF00` bloco, `0xA4E1F0` str; despachante comum `0x009C20A0`, `CGolfTask::OnPacket` `0x0069EEF0`) ou
  na assinatura de `docs/server/jp983-protocolo/pacotes_envio_983.tsv` (E) / `pacotes_recebe_983.tsv` (R).
- **[IFF ✓]** lido no `data/pangya_jp.iff`.
- **[REF]** referência AGPL (`<ref>/Server/JP/GameServer`), só comportamento. Arquivos: `GameServerTcp/GameService.cs`
  (registro 1234-1366), `PacketFunc/packet_func_sv.cs`, `Game/Channel.cs`, `Game/Room.cs`, `Game/Base/{GameBase,VersusBase,
  TourneyBase}.cs`, `Game/System/{PremiumSystem,CometRefillSystem}.cs`.
- **[KR]** nosso código (`src/Pangya.Protocol.KR645/Game`, `src/Pangya.Protocol.JP983/Game`) e `docs/server/protocolo/SPEC-*.md`.
- **[?]** não conferido.

Convenções: `u8/u16/u32/i32/u64/f32` LE; `str` = u16 tamanho + Shift-JIS; "0x216" = itens mudaram (`JpSystems.ItemsChanged`).
**uid = oid aqui**: na nossa sala `RoomPlayer.Guid` = uid da conta (`Room.cs:95`), então o "uid" dos efeitos e o "oid" do
marcador são o mesmo número (`me.Guid`).

---

## 0. Resumo

| # | C->S (JP) | hoje | o que é | resposta | para quem | onde fazer | tamanho |
|---|---|---|---|---|---|---|---|
| 1 | 0x184 / 0x185 | descartados (`ClientId` −1) | Assist liga/desliga na sala; assist no green | 0x216 + 0x26A / 0x26B | só o jogador | `JpSystems` (DB) | P |
| 2 | 0x156 | descartado | Auto Command usado numa tacada | nada (erro: 0x22B) | só o jogador | `JpSystems` | P |
| 3 | 0x138, 0x15C, 0x15D, 0x171, 0x180, 0x181, 0x196–0x199 | descartados | efeitos de asa/patas/anel/brinco/luva | 0x203 … 0x281 | sala (stroke) ou só ele (mass) | tradutor (síncrono, lock da sala) | P/M |
| 4 | 0x12E | descartado | marcador no campo | 0x1F8 | sala toda | tradutor | P |
| 5 | 0xD8 | vira KR 0xDA, sem handler | usar Sweet Yam / balas (buff por tempo) | 0x181 (= KR 0x186) | só o jogador | **KR** (`GameHandler`, KR 0xDA/0x186; o tradutor já converte os ids) | M |
| 6 | 0x119 | descartado | trocar de game server pela lista | 0x1D4 | só o jogador | `JpSystems` (registro) | P |
| 7 | 0x66 / 0x67 | vão como KR 0x66/0x67, sem handler | ticker (letreiro) | 0x96 + 0xC9 a todos / 0xCA | jogador / servidor todo | `JpSystems` + fila global | M |
| 8 | 0xEC | recusa fixa (`JpOnly`) | Comet Refill | 0x197 (= KR 0x19C) | só o jogador | **KR** (KR 0xEE/0x19C) e tirar o override | P |
| 9 | — (servidor manda) | nada | Premium ticket | 0x9A (+0x4000), 0x216, 0x26D | só o jogador | Domain + JP | M/G |
| 10 | 0x9D / 0xCB | 0x9D vira KR 0x9A (sem handler); 0xCB descartado | intrusão; fim do approach | 0x113 / (nada ou 0x24F) | só o jogador | tradutor | P |

---

## 1. Assist (0x184 / 0x185)

### 1.1 Item [IFF ✓]
`0x1BE00016` "Play Assist" (Item.iff; tem linha no Ability.iff). Grupo 6 (item comum), então já vai na lista de itens do
login (`InItemList`: Usable). **O estado do Assist é o próprio item no inventário**: tem o item = ligado.

### 1.2 C->S
- **0x184** (vazio) [EXE ✓ E: `lobbymain.cpp` 0x5045B0; também `CConsole::Command`]: botão Assist da sala.
- **0x185** `u32 typeid` [EXE ✓ E4, 0x47DC50]: na partida, no green, com o Assist ligado.

### 1.3 Referência [REF `Room.requestToggleAssist` Room.cs:5123, `GameBase.requestActiveAssistGreen` GameBase.cs:469]
- 0x184 **sem partida**: se não tem o item → cria (qtd 1, `assist_flag = true`); se tem → apaga (`assist_flag = false`).
  Manda **0x216** (1 item: `u8 2, typeid, id, flag_time, stat, qtd, 25 zeros`) e depois **0x26A** `u32 0, u32 0x1BE00016,
  u32 uid`. Erro: `0x26A u32 código` (0x5200801 criar / 0x5200802 apagar).
- 0x184 **com partida**: só `0x16A u32 0`. O 0x16A do JP é a semente do `srand` [EXE ✓ 0x9D1EA5: lê u32, `srand(v)`, chama
  0x85C7E0, devolve a semente antiga]: não tem a ver com assist. **Não copiar**: na partida, ignorar o 0x184.
- 0x185: `typeid` tem que ser 0x1BE00016, o jogador tem o item e o assist está ligado → **0x26B** `u32 0, u32 typeid,
  u32 uid`; senão `0x26B u32 código` (0x5200101/0x5200102).

### 1.4 S->C [EXE ✓]
- **0x26A** (despachante comum 0x9D693F): lê **só `u32 código`**; ≠0 → mensagem de erro 0x27 com o código. Depois, sempre,
  pergunta ao inventário se tem o Assist (0x63BBA0) e manda msg 0x355 (ligado/desligado) à tela. ⇒ **o 0x216 tem que vir
  antes do 0x26A** (como na ref.). O `typeid, uid` que a ref. põe depois do código não são lidos (inofensivos).
- **0x26B** (`CGolfTask` 0x6A4A2E): lê `u32 código`; 0 → msg 0x753B sub 0x18 com o leitor (o resto, `u32 typeid, u32 uid`
  [REF], é lido por quem trata a mensagem [?: não achei o receptor]); ≠0 → erro.

### 1.5 Como fazer aqui
- `JpSystems` (precisa do banco): `CAssistToggle = 0x184`, `CAssistGreen = 0x185` em `Ids`.
  - 0x184: se `kr.CurrentRoom?.Game` em andamento → ignorar. Senão, sem o item → `ctx.Shop.GiveAsync(p, 0x1BE00016, 1)`
    e `ItemsChanged([new(tid, id, 0, 1)])`; com o item → apagar (o `SpendAsync(0x1BE00016, qtd)` já faz banco + 0x216
    com `depois = 0`). Depois `0x26A u32 0, u32 0x1BE00016, u32 uid`. Erro → `0x26A u32 1`.
  - 0x185: `p.FindType(0x1BE00016)` existe → `0x26B u32 0, u32 typeid, u32 uid`; senão `0x26B u32 1`.
- Só para o jogador. Não há efeito no servidor (o Assist é desenho do cliente).

---

## 2. Auto Command (0x156)

- Item [IFF ✓] `0x1A00019F` "Auto Commander" (Item.iff c[0] = 100 [?: significado]).
- C->S **0x156** vazio [EXE ✓ E, golfrule 0x694948]: depois de cada tacada, se o Auto Command está ativo
  (`[golfrule+0x240] ≠ 0`) e a sala não é do tipo 0xB, o cliente chama 0x681FC0 [?: provável desconto local] e manda o 0x156.
  Ou seja, **um 0x156 por tacada com o comando automático**.
- [REF `GameBase.requestActiveAutoCommand` GameBase.cs:390]: premium não gasta. Senão: tem o item, `qtd ≥ 1`, e o contador
  desta partida (`used_item.v_passive`) < qtd → contador++. **Não responde no sucesso.** Erro: **0x22B** `u32 código`
  [EXE ✓ D4, 0x9D55E1] (0x550002 sem item; 13/14 = limite).
- Fim da partida [REF GameBase.cs:1826-1995]: os passivos com contador > 0 são tirados do banco (`removeItem`) **sem 0x216**
  (o 0x216 do fim é só da oficina) ⇒ o cliente desconta sozinho [?].
- Como fazer: `JpSystems` com um contador por partida (zerar no início da partida); se `qtd − usados ≤ 0` → `0x22B u32 1`;
  senão usados++. No fim da partida, gravar `qtd − usados` no banco sem 0x216 (ou gravar já a cada 0x156, também sem 0x216).
  Premium (§9): não conta. Para o jogador só.

---

## 3. Efeitos de item na partida (asas, patas, anéis, brinco, luva)

### 3.1 Que itens [IFF ✓]
Os efeitos vêm do **Ability.iff** (920 linhas, 80 B; `u32 typeid` em +0, tipos de efeito em seguida): 866 Part.iff
(asas, luvas, patas…), 27 Mascot.iff, 20 AuxPart.iff (anéis 0x7001xxxx (R) e 0x7021xxxx (L)), 4 Item.iff (Fairy's Tears,
Vector Sign 0x1A0001A0, Magabeni Bloom, Play Assist) e 2 ClubSet. Tipos (enum `AbilityEffect` da ref.,
`PangyaAPI.IFF.JP/Models/Flags/Definitions.cs:217`): 9 PAWS_NOT_ACCUMULATE, 11 EARCUFF_DIRECTION_WIND, 12 COMBINE (índice
no SetEffectTable.iff), 21 PAWS_ACCUMULATE, 22 POWER_GAUGE_FREE, 31 MIRACLE_SIGN_RANDOM, etc. **O efeito é calculado no
cliente**; o servidor só valida e repassa. (A ref. guarda um bit por tacada — `setEffectActiveInShot` — para a sua
anti-trapaça/estatística; o nosso KR não valida tacada, não precisa.)

### 3.2 Validação [REF VersusBase.cs:2176-2860]
Para os que mandam typeid: typeid ≠ 0; o jogador tem o item; tem personagem equipado; o item está **equipado** no
personagem (`IntArray("parts", 24)` para Part, `IntArray("aux", 5)` para AuxPart; Mascot = mascote equipado). Patas
(0x15C), 0x196 e 0x199 não trazem nada: só repassar. Tudo só **durante a partida** (`Room.requestActiveX` passa ao jogo;
sem jogo é erro).

### 3.3 Tabela (C->S [EXE ✓ E] → S->C [EXE ✓ R `CGolfTask`] / campos depois do código [REF TourneyBase])

| C->S | campos | o que é | S->C | o exe lê | layout a mandar | versus (stroke) | torneio/mass |
|---|---|---|---|---|---|---|---|
| **0x138** (0x55F0E0) | `u32 typeid` | asa (Part) | **0x203** (0x6A44B4) | `D4 D4` → msg 0x7532 sub 0x19 | `u32 uid, u32 typeid` | sala toda | só ele |
| **0x15C** (golfrule 0x675160) | — | patas (PAWS_NOT_ACCUMULATE) | **0x236** (0x6A4538) | `D4` → sub 0x18 | `u32 uid` | sala toda | só ele |
| **0x15D** (0x4AA7D0) | `u32 typeid, u32 valor, u8 efeito` | anel (AuxPart) | **0x237** (0x6A4566) | `D4` código; 0 → msg 0x753B sub 0x14 com o leitor | `u32 0, u32 uid, u32 typeid, u8 efeito` | sala toda | só ele |
| **0x171** (0x82DA60) | `u32 typeid, u8 ângulo, f32 x` | brinco (vento; Part ou Mascot) | **0x24C** (0x6A4619) | código; 0 → sub 0x15 | `u32 0, u32 typeid, u32 uid, u8 ângulo, f32 x` | sala toda | só ele |
| **0x180** (0xB45740) | `u32 typeid` | luva (Part ou AuxPart) | **0x265** (0x6A4952) | código; 0 → sub 0x16 | `u32 0, u32 typeid, u32 uid` | sala toda | só ele |
| **0x181** (6 lugares, 0x569B00…) | `16 B` = `u32 efeito, u32 anel0, u32 anel1, u32 opção` | "anel no chão" (efeito de 2 anéis/parte/mascote) | **0x266** (0x6A49C0) | código; 0 → sub 0x13 | `u32 0, 16 B (eco), u32 uid` | **só ele** | só ele |
| **0x196** (golfrule 0x6754DC) | — | anel de patas arco-íris (PAWS_ACCUMULATE) | **0x27E** (0x6A471A) | `D4` → sub 0x1B | `u32 uid` | sala toda | só ele |
| **0x197** (0x5B79D0) | `16 B` = `u32 efeito, u32 anel0, u32 anel1, u32 opção` | anel de power gauge (POWER_GAUGE_FREE) | **0x27F** (0x6A4768) | `D4` → sub 0x1D | `u32 uid` | sala toda | só ele |
| **0x198** (0x56ADE0) | `u32 typeid` | miracle sign (MIRACLE_SIGN_RANDOM) | **0x280** (0x6A4A99) | código; 0 → sub 0x19 | `u32 0, u32 typeid, u32 uid` | sala toda | só ele |
| **0x199** (golfrule 0x6754A9) | — | conjunto de anéis com patas (PAWS_NOT_ACCUMULATE) | **0x281** (0x6A4748) | `D4` → sub 0x1F | `u32 uid` | sala toda | só ele |

⚠ **Alerta de layout**: no `VersusBase` da referência o 0x237, 0x24C, 0x265, 0x266 e 0x280 vão **sem** o `u32 0` inicial
(começam pelo uid/typeid); no `TourneyBase` vão com ele. O exe lê **primeiro um código** e só segue com 0 [EXE ✓
0x6A45B7 / 0x6A466A / 0x6A49A3 / 0x6A4A11 / 0x6A4AEA]: com o layout do versus o cliente trataria o uid como erro. **Usar
sempre o layout com `u32 0`.** Erro = `u32 código ≠ 0` sozinho (só ao jogador).
O receptor da msg 0x753B (que lê o resto) não foi achado; a ordem dos campos depois do código é a do TourneyBase [REF/?].

### 3.4 Como fazer aqui
- No **tradutor** (síncrono; padrão do `kr.ZodiacReady()`): `fromClient[id] = r => { Efeito(...); return null; }` para os
  10 ids. Todos estão acima de 0xF1 → nenhum colide com id traduzido do KR.
- Mandar para a sala **sem** passar pelo tradutor (os ids 0x203… não existem no KR: `InGameOutput.Broadcast` usa
  `Connection.Send(body)`, que traduz e jogaria fora): helper no estilo de `JpZodiacOutput.All`:
  ```
  lock (kr.Context.World.Rooms.Sync)
      if (kr.CurrentRoom is { Game: { Over: false } g } room && room.Find(kr) is { } me)
          foreach (var rp in room.Players)
              if (rp.Session is GameHandler h && (g is not MassGame || h == kr)) h.Connection.SendTranslated(make(me.Guid));
  ```
  (`make` cria um `PacketWriter` novo por destinatário: `SendTranslated` consome o writer.) Bots (`Session == null`) ficam
  de fora. Para 0x266 (e para erros) só `conn.SendTranslated`.
- Validação mínima recomendada: item no inventário (`p.FindType(tid)`) e equipado no personagem; se falhar, `u32 1` ao
  jogador (para os que têm código) ou nada (0x203/0x236/0x27x sem código).

---

## 4. Marcador no campo (0x12E → 0x1F8)

- C->S **0x12E** `f32 x, f32 y, f32 z` [EXE ✓ E `EB(4)×3`, `CTutorGuide` golfball.cpp 0x6B2740].
- [REF `VersusBase.requestMarkerOnCourse` VersusBase.cs:2912; `GameBase` versão vazia para os outros modos]: só no versus;
  **0x1F8** `i32 oid, f32 x, f32 y, f32 z` a **todos da sala** (inclusive quem marcou).
- S->C 0x1F8 [EXE ✓ despachante comum 0x9D348A: `D4 DB(4) DB(4) DB(4)`].
- O KR 645 não tem [KR: sem id; `sistemas-jp-modos.md` C.1].
- Fazer no tradutor com o mesmo helper do §3.4 (`oid = me.Guid`), só em `StrokeGame` (no mass: ignorar, como a ref.).

---

## 5. Buff por tempo (0xD8 → 0x181): Sweet Yam e balas

### 5.1 Itens [IFF ✓ `TimeLimitItem.iff`, 6 × 100 B: `u32 ativo, u32 typeid, char[40] nome, char[40] ícone, u32 tipo,
u32 percent, u32 minutos`]

| typeid | nome | tipo (`ItemBuff.eTYPE` da ref.) | percent | minutos | efeito [REF GameBase.cs:2410] |
|---|---|---|---|---|---|
| 0x1A0000B3 | Sweet Yam | 1 YAM_AND_GOLD | 5 | 120 | +5 % EXP |
| 0x1A0000B4 | Golden Sweet Yam | 1 | 50 | 120 | +50 % EXP |
| 0x1A0000E1 | Rainbow Drop Candy | 2 RAINBOW | 0 | 60 | +100 % EXP e +100 % pang |
| 0x1A0000E2 | Delicious Drop Candy | 3 RED | 0 | 60 | +100 % EXP e +100 % pang |
| 0x1A0000E3 | Melon Drop Candy | 4 GREEN | 0 | 60 | +100 % EXP |
| 0x1A0000E4 | Lemon Drop candy | 5 YELLOW | 0 | 60 | +100 % pang |
(percent 0 = 100.) O KR 645 só tem B3/B4 (tabela fixa no cliente; `SPEC-itens-restantes.md` §3).

### 5.2 Pedido e regras
- C->S **0xD8** `u32 typeid` [EXE ✓ E4, `itembuffactor.cpp` 0x8133A0]. O tradutor já o faz **KR 0xDA** (+2), que é o
  mesmo pedido do KR 645 (`SPEC-itens-restantes.md` §3.2) — o KR ainda não trata.
- [REF `Channel.requestUseItemBuff` Channel.cs:14435]: typeid ≠ 0; tem o item com qtd ≥ 1; está no Item.iff e no
  TimeLimitItem.iff; tira 1; se já há buff **do mesmo typeid**, soma `minutos` ao fim (a partir de agora se já venceu);
  senão cria (início agora, fim = agora + minutos). Grava. (O ramo "mesmo efeito, typeid diferente" da ref. nunca roda:
  `findItemBuff` é chamado sem o tipo.) O cliente KR recusa sozinho B3 com B4 ativos; o JP tem a mesma checagem de
  compatibilidade [?].
- Efeito: no fim da partida, somar as porcentagens às taxas de EXP/pang (onde hoje entra
  `CardService.ActiveRate(...)`, `GameHandler.GameEnd.cs:46-47`).
- Vencimento [REF `PlayerManager.checkItemBuff`]: some da lista. O cliente tem a opção 1 para isso (abaixo).

### 5.3 S->C 0x181 [EXE ✓ 0x9D1CC5 + parser `0x813A70`]
Primeiro u32 = opção:
- **0** (lista, login): `u32 0, u8 n, n × ItemBuff(65)` (laço `0x813C63`).
- **1** (venceu): `u32 1, u32 typeid` → tira da lista os buffs com esse typeid (`0x813740`).
- **2** (resposta do 0xD8): `u32 2, u32 n, …`; só com **n = 1** lê `u32 typeid, ItemBuff(65)`, adiciona e mostra "usou";
  o despachante, antes, chama `0x9BDEB0(typeid, …, 1)` (tira 1 do item na lista do cliente) ⇒ **não mandar 0x216/0xA7 antes**
  (desconto em dobro), igual ao KR (`SPEC-itens-restantes.md` §3.3). Depois manda msg 0x26 ao ator "ItemBuff" (fecha o
  "aguarde").
- **outro valor** (os códigos de erro da ref., ex. 0x5500402): ignorado; o diálogo pode ficar esperando [?]. Erro seguro:
  `u32 2, u32 0` (n ≠ 1: não lê mais e manda a msg 0x26 com 0) [EXE ✓ 0x813B0C → 0x813BDB].
- **ItemBuff (65 B)** [REF `ItemBuff.ToArray`, pangya_game_st.cs:6202] = `sSCardAvilityPeriodInfo` do KR (0x41, já existe
  em `PlayerStructs`): `u32 id, u32 typeid, u32 parts_typeid (0), u32 parts_id (0), u32 efeito, u32 valor, u32 slot (0),
  SYSTEMTIME uso, SYSTEMTIME tempo, u32 tipo, u8 ativo (1)`. Na ref. o 2º SYSTEMTIME guarda a **duração em segundos**
  espalhada (`wSecond = s / 0xFFFF`, `wMilliseconds = s % 0xFFFF`); no KR é o **fim** (`useEnd`). O cliente JP usa [?];
  mandar o fim (como o KR) é o palpite mais seguro, conferir na tela (contagem regressiva).

### 5.4 Login
- [REF `cLoginTask.cs:116`]: `pacote181(v_ib)` com opção **0** depois dos cards (0x135/0x144/0x138/0x136/0x137), antes do
  0x96. Aqui: o KR manda **0x186 opção 0** (vira JP 0x181) depois da lista de cards (0x12F).

### 5.5 Como fazer aqui
No **KR** (serve os dois clientes): `GameHandler` trata **0xDA**, responde **0x186** (`ServerId(0x186)` = 0x181 ✓,
mesmo layout), e o login manda 0x186 opção 0. Domain: `ItemBuffService` com tabela por cliente (JP: TimeLimitItem.iff via
`Jp983GameData`; KR: B3/B4 fixos), banco (tid, início, fim). Taxas no `GameEnd`. Nada no tradutor JP.

---

## 6. Trocar de game server (0x119 → 0x1D4)

- C->S **0x119** `u32 id do servidor` [EXE ✓ E4; `serverdlg.cpp` 0xAF4CE0 e `CTaskMain::HandleMsgCommon`].
- [REF `GameService.requestChangeServer` GameService.cs:2606]: o id tem que estar na lista de servidores; quem está no
  lobby do GP só pode ir para servidor com a propriedade GP; gera uma **chave de game nova** para (uid, servidor) e manda
  **0x1D4** `u32 0, str chave`. Erro: reenvia a lista de servidores e canais (sem 0x1D4).
- S->C **0x1D4** [EXE ✓ 0x9D3421]: `u32 código`; **≠ 0 → nada** (o cliente fica onde está; mandar a lista de novo).
  **= 0** → lê `str chave`, grava em `doc+0x11254` e manda msg 0x1B2 à tela (desconecta e conecta no servidor escolhido).
  `doc+0x11254` é o **último campo do 0x02** do game (a "chave do game server") [EXE ✓ 0xA51A3A], que é o que o nosso
  `JpTranslator.Login` usa (`gkey`) e o `JpGameHandler.LoginKeyIsValidAsync` valida.
- **KR hoje**: não tem pedido de troca; o cliente KR reconecta com a chave que já tem. A chave de game
  (`Sessions.IssueGameLoginAsync`) vale 24 h e **não é consumida** no uso (`ValidateGameLoginAsync(..., consume: false)`),
  então a mesma chave serve no outro servidor. Lista: `GameHandler.ServerListAsync` (KR 0x43 → 0x9D).
- Como fazer: `JpSystems` (`CChangeServer = 0x119`, acima de 0xF1, sem colisão): `ctx.Registry.ListAsync("game")`
  contém o id (e, se `GrandPrixLobby`, o servidor tem a propriedade 0x800) → `0x1D4 u32 0, str chave` com a chave atual
  do jogador (guardar o `gkey` do login) ou uma nova (`IssueGameLoginAsync`, que troca as outras — ok, o cliente vai sair).
  Senão `0x1D4 u32 1` + reenviar a lista (devolver `Body(0x43)` ao KR: o próprio KR monta a lista).

---

## 7. Ticker (0x66 / 0x67)

- C->S **0x66** `str mensagem` [EXE ✓ ES; `taskmain.cpp` 0x653B40 e `golfrule.cpp` 0x6704B0, callback do "sim" de um
  diálogo; texto em `[0xE78CC8]+0x5C`]. **0x67** vazio [EXE ✓; `taskmain.cpp` 0x65CFE0 / `golfrule.cpp` 0x681940]: pede a fila.
- [REF `GameService.requestSendTicker` / `requestQueueTicker` GameService.cs:3806-3930]: bloqueio de ticker na conta →
  erro; mensagem vazia ou com injeção → erro; **gasta 1 cookie** [REF; preço oficial [?]]; põe na fila global
  (`hora = agora + 3 s`); responde **0x96** `u64 cookies` [EXE ✓ D8]. Erro: **0x50** `u32 código` (1 genérico, 4 sem cookies)
  [EXE ✓ 0x9C5B58: `u32` com casos 0..9]. O laço do servidor tira um por vez e manda **0xC9** `str nick, str mensagem`
  [EXE ✓ 0x9CC3A3 `DS DS`] **a todos os canais**; cada ticker ocupa ~30 s.
- 0x67 → **0xCA** `u16 na fila, u32 ms restantes (= fila × 30 000)` [EXE ✓ 0x9CC482 `D2 D4`].
- KR: o S->C existe (KR **0xC7** `str nick, str msg`, `COneLineBoard`; `ServerId(0xC7)` = 0xC9 ✓, `SPEC-chat-gm.md`), o
  C->S do KR 645 não foi achado [?]; o servidor KR não tem ticker.
- Como fazer: `JpSystems` (`0x66`, `0x67`; o único JP que vira KR 0x66/0x67 é ele mesmo → sem colisão). Fila global no
  `World` (um `Queue` com lock + timer de 30 s) que manda **KR 0xC7** a todos os conectados (o tradutor converte para 0xC9;
  o KR também ganha). Cookie: `Player.Cookie − custo` no banco (config, padrão 1 como a ref. [?]); 0x96 = `KR 0x94` (o
  `SCookie` do KR vira 0x96 ✓).

---

## 8. Recarga de cometa (0xEC → 0x197)

- C->S **0xEC** `u32 pacote, u32 bola` [EXE ✓ E4 E4, `S5::FrSupplyPackDlg` 0xB34F40] = **KR 0xEE** (o +2 do tradutor já
  converte), mesmo diálogo do KR (`SPEC-itens-restantes.md` §4).
- S->C **0x197** `u8 ok, u32 pacote, u32 bola, u32 total da bola` [EXE ✓ 0x9D26B9 `D1 D4 D4 D4`] = **KR 0x19C**
  (`ServerId(0x19C)` = 0x197 ✓, mesmo layout). A ref. escreve `u16` no total e 10 zeros no erro: errado para o exe (usar
  u32 e 12 zeros). `ok = 0` → fecha e mostra "notify" de erro. `ok ≠ 0`: o cliente tira 1 do pacote e grava o total;
  **não mandar 0x216/0xA7 antes**.
- **Delta ≥ 10** [EXE ✓ `0xB34E30`, igual ao KR]: a animação (`UpdateData`) exige `total − atual ≥ 10`; menos que isso
  fecha o diálogo com erro. 10 "+N" que somam o delta.
- Quantidade: **não há tabela no cliente** (Item.iff do 0x1A000105: c[] todos 0 [IFF ✓]; nada no TimeLimitItem/SubscriptionItem).
  Ref.: faixa min..max por pacote no **banco** (`CometRefillSystem.drawsCometRefill`). Config: `0x1A000105 → min..max` com
  `min ≥ 10` [?: valor oficial]. No JP o 0x1A00014B é "Azfuere" (outro item, c[] ≠ 0) [IFF ✓]: não é pacote — recusar.
- Validações [REF Channel.cs:14612]: tem a bola e o pacote (qtd ≥ 1), bola no Ball.iff, pacote com faixa configurada; tira
  1 pacote, soma na bola. (+ as do cliente KR: bola 0x14…, não 0x14000004, não "Aztec básica".)
- Como fazer: Domain `CometRefillService` + **KR** `GameHandler` 0xEE → 0x19C; depois **apagar** `fromClient[0xEC]` do
  `JpTranslator.JpOnly.cs` (aí o 0xEC passa como KR 0xEE e a resposta volta como 0x197). Sem colisão: o único JP que vira
  KR 0xEE é o 0xEC.

---

## 9. Premium ticket (0x9A bit 0x4000, 0x26D)

- Itens [IFF ✓]: `0x1A100002` "Premium Ticket (Normal)" e `0x1A100003` "(VIP)" (Item.iff; c[1..4] = 25/45/80/800 e
  50/90/160/1600 [?: significado — não são lidos pela ref.]); bola `0x140000D8` "Premium Comet"; mascote `0x4000004C` Lolo
  (Premium). Os club sets 0x10001001/… e o título da ref. não são do 983 (comentados "original não tem").
- Ativação [REF `ItemManager.addItem` 3380-3470, `PremiumSystem.addPremiumUser`]: ao **receber** o ticket (loja/presente,
  com dias); recusa outro ticket enquanto um vale. No login com ticket válido [REF `LoginManager` caso 4 →
  `updatePremiumUser`].
  1. `premium_user = true` → **0x9A** `i32 capability` com o bit **0x4000** [EXE ✓ 0x9CC089 D4]. (Nossa capability =
     `Player.IdentityFlags`, já vai no 0x44 em `JpStructs.cs:61`; o 0x9A vem do KR 0x98 `SIdentity`, +2 ✓.)
  2. **0x216** com a bola premium como **item virtual** (`id = −1`, `flag_time = 0x6A`, qtd 1) e ela vira o cometa
     equipado (`ue.ball_typeid`, `ei.comet`). VIP: + mascote (e club set) por tempo, e 4 caixas 0x1A0002BA pelo correio.
- Vencimento [REF `checkEndTimeTicket`/`removePremiumUser`]: tira o ticket, tira a bola (0x216 com −1), **0x9A** sem o bit,
  **0x26D** `i32 unix do fim` [EXE ✓ 0x9D69FF: `u32` → `0x642BE0(v, 0)` grava no doc], chat 0x40 tipo 7 "não é mais
  premium". Na ref. o 0x26D só vai **no fim** (`SERVER_PREMIUM_TIME_END`), não na ativação.
- Efeitos na partida [REF]: Time Booster (`TourneyBase.cs:915`, `GrandPrix.cs:511`) e Auto Command (§2) **não gastam**;
  criar sala com bot e entrar no GP **sem ticket** (`Room.cs:2160`, 5526 — o 5526 tem `||` em vez de `&&`, bug); Auto
  Caliper grátis por buraco no VIP (`GameBase.cs:1836`); peso extra no sorteio do Golden Time; `premium` = 1 no resultado do
  torneio (`Tourney.cs:1079`). O "+10 % EXP/pang" está **comentado** na ref. (GameBase.cs:2645).
- O que precisa para funcionar: (a) o ticket chegar ao jogador (loja vender ou GM dar) e uma coluna "premium até" na conta;
  (b) no login e ao receber: 0x9A com 0x4000 + 0x216 da bola virtual (a bola não pode ir para o banco); (c) checagem de fim
  (a cada pedido ou timer) com 0x216/0x9A/0x26D/0x40; (d) as regras grátis do item (b) do §2 e do Time Booster. O que o bit
  0x4000 muda na tela do cliente não foi conferido [?]. Vale só se a loja vender o ticket.

---

## 10. Intrusão (0x9D) e fim do approach (0xCB)

### 10.1 0x9D — entrar depois de começar
- C->S **0x9D** [EXE ✓ `intrusion.cpp`]: `u8 0|1, u16 sala` (0x855F20 / 0x8565F0) ou `u8 2, 242 B` (placar,
  `CIntrusion::PacketAnalysis` 0x8572D0). O tradutor já faz **KR 0x9A**, e o layout é o do KR 645 (`SPEC-intrusao-tempo.md`
  §1.3: 242 B = 18 u8 + 18 i32 + 18 i64 + u32 uid + u32 remetente) — o KR **não implementa** (só loga).
- S->C: o JP responde em **0x113** (= KR **0x111**, que hoje é −1 no `ServerId`: precisa da exceção `[0x111] = 0x113`)
  [EXE ✓ despachante 0x9CF9D7 → `CIntrusion::PacketAnalysis` 0x8572D0: `u8 tipo, u8 código` e switch]. Os tipos 3/4/7 levam
  `sRoomInfo`/`sSlotInfo`, que **mudam de tamanho no JP** → precisariam de conversão.
- [REF Channel.cs:4201]: só sala **torneio**, não trancada, em jogo, não cheia; sub 0 → manda o tempo; sub 1 → entra; erro
  → `0x113 u8 6, u8 código`.
- O cliente só manda 0x9D para sala com `bIntrusion` (KR: `SPEC-intrusao-tempo.md` §1.2); o KR nunca liga essa flag, então
  hoje não chega. Mínimo seguro: no tradutor, `fromClient[0x9D]` → sub 0/1 respondem `0x113 u8 6, u8 5` (NOT_ALLOWED) e
  `null`; sub 2 → `null`. Fazer a intrusão de verdade no KR (`SPEC-intrusao-tempo.md` §3) e então converter o 0x111.

### 10.2 0xCB — fim do approach
- C->S **0xCB** `u32 oid` [EXE ✓ E4; `lobbymain.cpp` 0x505380, manda `doc+0x81A`]. Fora das faixas (`ClientId` −1: o −8
  para em 0xCA) ⇒ descartado com log. O par no KR 645 é **0xC3** (0xCB − 8), que o KR também não trata e o approach do KR
  funciona sem ele (`SPEC-modes.md` §2: o fim é do servidor: 0x148, 0x146, 0x149).
- [REF `Approach.requestFinishGame` / `finish_game(…, 0xCB)` Approach.cs:420, 2577]: confere o oid; se o approach ainda
  roda, termina para ele; manda atualização de info/estatística, 0x6B (mascote), **0x244** `u32 0`, **0x24F** `u32 0`
  [EXE ✓ 0x24F D4] e **0xC8** `u64 pang, u64 0`.
- Como fazer: `fromClient[0xCB] = r => null` (silenciar) e testar o fim do approach no JP. Se a tela de resultado do
  approach não voltar para a sala, responder `0x24F u32 0` (o mesmo `SDone` do `JpZodiacOutput`) e `0xC8` [?]. (O 0x244 não
  aparece no despachante comum nem no `CGolfTask` [?].)

---

## 11. Tabela pronta para implementar

| C->S JP | layout C->S | id KR hoje (`ClientId`) | onde tratar | resposta (JP) | destino |
|---|---|---|---|---|---|
| 0x184 | — | −1 (descartado) | `JpSystems` (novo `Ids`) | 0x216 (+/−1 de 0x1BE00016) → **0x26A** `u32 0, u32 0x1BE00016, u32 uid`; erro `0x26A u32 1`; na partida: ignorar | jogador |
| 0x185 | `u32 typeid` | −1 | `JpSystems` ou tradutor | **0x26B** `u32 0, u32 typeid, u32 uid`; erro `u32 1` | jogador |
| 0x156 | — | −1 | `JpSystems` (contador da partida) | nada; erro **0x22B** `u32 1` | jogador |
| 0x138 | `u32 typeid` | −1 | tradutor | **0x203** `u32 uid, u32 typeid` | sala (stroke) / ele (mass) |
| 0x15C | — | −1 | tradutor | **0x236** `u32 uid` | sala / ele |
| 0x15D | `u32 typeid, u32 valor, u8 efeito` | −1 | tradutor | **0x237** `u32 0, u32 uid, u32 typeid, u8 efeito`; erro `u32 1` | sala / ele (erro: ele) |
| 0x171 | `u32 typeid, u8 ângulo, f32 x` | −1 | tradutor | **0x24C** `u32 0, u32 typeid, u32 uid, u8 ângulo, f32 x`; erro `u32 1` | sala / ele |
| 0x180 | `u32 typeid` | −1 | tradutor | **0x265** `u32 0, u32 typeid, u32 uid`; erro `u32 1` | sala / ele |
| 0x181 | `16 B` | −1 | tradutor | **0x266** `u32 0, 16 B, u32 uid`; erro `u32 1` | **só ele** |
| 0x196 | — | −1 | tradutor | **0x27E** `u32 uid` | sala / ele |
| 0x197 | `16 B` | −1 | tradutor | **0x27F** `u32 uid` | sala / ele |
| 0x198 | `u32 typeid` | −1 | tradutor | **0x280** `u32 0, u32 typeid, u32 uid`; erro `u32 1` | sala / ele |
| 0x199 | — | −1 | tradutor | **0x281** `u32 uid` | sala / ele |
| 0x12E | `f32 x, y, z` | −1 | tradutor | **0x1F8** `i32 oid(=Guid), f32 x, y, z` | sala toda (só stroke) |
| 0xD8 | `u32 typeid` | **0xDA** (sem handler) | **KR** `GameHandler` 0xDA | KR **0x186** → JP 0x181: `u32 2, u32 1, u32 tid, 65 B`; erro `u32 2, u32 0`; login `u32 0, u8 n, n × 65`; venceu `u32 1, u32 tid` | jogador |
| 0x119 | `u32 servidor` | −1 | `JpSystems` | **0x1D4** `u32 0, str chave`; erro `0x1D4 u32 1` + lista (KR 0x43) | jogador |
| 0x66 | `str msg` | 0x66 (sem handler) | `JpSystems` + fila global | **0x96** `u64 cookie` (KR 0x94); depois **0xC9** `str nick, str msg` (KR 0xC7) a todos; erro **0x50** `u32 código` | jogador / servidor |
| 0x67 | — | 0x67 (sem handler) | `JpSystems` | **0xCA** `u16 fila, u32 ms` | jogador |
| 0xEC | `u32 pacote, u32 bola` | override `JpOnly` (recusa) | **KR** `GameHandler` 0xEE; apagar o override | KR **0x19C** → JP 0x197: `u8 1, u32 pacote, u32 bola, u32 total` (delta ≥ 10); erro `u8 0` + 12 zeros | jogador |
| 0x9D | `u8 0/1, u16 sala` / `u8 2, 242 B` | 0x9A (sem handler) | tradutor (mínimo) / KR (completo) | **0x113** `u8 6, u8 5` (recusa) | jogador |
| 0xCB | `u32 oid` | −1 | tradutor: `null` | nada (se travar: 0x24F `u32 0` + 0xC8) | jogador |
| — | — | — | Domain + JP | premium: 0x9A (bit 0x4000), 0x216 bola virtual, 0x26D no fim, 0x40 tipo 7 | jogador |

### 11.1 Alertas de colisão de ids (`JpGameHandler.OnPacketAsync` checa `JpSystems.Ids` no id **já traduzido**)
Ids KR que o tradutor produz: 0x00–0x82 (iguais), 0x88–0x9A (−3), 0xA5–0xC2 (−8), 0xCE–0xF1 (+2), 0x6D e 0x7E (exceções).
- **Seguros** para `JpSystems.Ids`: 0x119, 0x12E, 0x138, 0x156, 0x15C, 0x15D, 0x171, 0x180, 0x181, 0x184, 0x185, 0x196–0x199
  (todos > 0xF1); 0x66 e 0x67 (só o próprio JP 0x66/0x67 vira esse id).
- **Não pôr em `JpSystems.Ids`**:
  - **0xD8**: o JP **0xD6** viraria KR 0xD8 (+2). Hoje o 983 não manda 0xD6 (não está em `pacotes_envio_983.tsv`), mas
    não há motivo para arriscar: tratar 0xD8 no KR como 0xDA (já é o que o tradutor faz).
  - **0xEC**: o JP **0xEA** (upgrade de caddie, `upgradecaddiedlg.cpp` 0xAB1830, **é enviado**) vira KR 0xEC (+2) e seria
    engolido. Tratar como KR 0xEE.
  - **0x9D**: hoje nenhum JP vira KR 0x9D, mas fica perto da faixa −3 (até 0x9A); preferir `fromClient[0x9D]` no tradutor.
  - **0xCB**: nenhum JP vira KR 0xCB, mas não precisa de `JpSystems`; `fromClient` basta.
- Lembretes do mesmo tipo já existentes: os ids baixos de `JpSystems` (0xA1, 0xA2, 0xA5, 0xA7, 0xA8, 0xFB) recebem o que o
  tradutor produz de JP 0xAD (→0xA5, silenciado), JP 0xAF (→0xA7) e JP 0xB0 (→0xA8, silenciado). O JP 0xAF não é enviado
  pelo 983 (fora de `pacotes_envio_983.tsv`), então hoje não colide; se um dia aparecer, cairia na resposta de guilda.
- S->C: os efeitos (0x203…0x281), 0x1F8, 0x26A/0x26B, 0x22B, 0x1D4, 0xCA, 0x113 não têm id KR (`ServerId` −1): mandar com
  `SendTranslated` (nunca por `Connection.Send`/`InGameOutput.Broadcast`, que passam pelo tradutor e descartam). Para a
  intrusão completa, acrescentar `[0x111] = 0x113` em `ServerIdExceptions` (com conversão de sRoomInfo/sSlotInfo).

### 11.2 Ordem sugerida
1. Tradutor: efeitos (§3) + marcador (§4) + 0xCB silenciado + 0x9D recusado — um commit, sem banco.
2. `JpSystems`: Assist (§1), Auto Command (§2), troca de servidor (§6).
3. KR + Domain: recarga de cometa (§8, depois apagar o override do 0xEC) e buffs (§5), que servem os dois clientes.
4. Ticker (§7) com fila global. 5. Premium (§9), só se a loja vender o ticket.
Regras do projeto: `src/` sem LINQ (dicionários); `.sh` com LF.

---

## 12. Bolas: lista, gasto e premium

- **Regra do dono**: 1 unidade por buraco concluído, descontada só no fim da partida, com piso em 0. A bola não acaba no meio
  (1 unidade e 3 buracos joga os 3; 5 unidades e 18 buracos joga os 18). Vale em todos os modos, prática inclusive; quem sai
  paga os buracos que fechou (`RoomGame.TakeBallHoles`, uma vez por jogador; partida abortada no 1º buraco não paga). No Grand
  Zodiac e no Chip-in cada acerto conta como buraco.
- **Cliente** [C]: nenhum caminho desconta `m_ballList` sozinho; só o servidor muda a contagem. 0x216 tipo 2 grava `total`
  absoluto em `Common[0]` e 0 apaga (`UpdateBallCount983`, `RemoveBall983` itemupdater.cpp:485/748). Apagou a equipada: o
  cliente manda 0x0B tipo 2 com 0x14000000, ou 0x140000D8 com o bit 14. O servidor já trocou a bola e aceita o pedido (no VIP,
  a 0x140000D8 vira a 0x140000E9: `PlayerActions.EquipBall`).
- **Sem gasto**: COM[0] = 0 no Ball.iff (`IsUnlimitAztec`, itemmanager.cpp:6371): 0x14000000, 2E, 87, D8, E0, E2, E8, E9.
- **Bola padrão na lista**: o My Room só lista o que veio no 0x71 (myroomwidget.cpp:1209). Sem linha no banco, vai uma entrada
  virtual com id −1 (`Item.VirtualBallId`), fora de `Player.Items`: não dá para vender, trocar, mandar nem apagar.
- **Premium**: ligou, a bola do ticket substitui a básica equipada (0x4B); acabou, 0x9A sem o bit, 0x216 e volta à básica. Com
  o bit, o My Room esconde a 0x14000000 e põe a 0x140000D8 no topo (myroomwidget.cpp:1197).
- **Bônus**: as bolas premium não têm efeito no Ball.iff (Bound/Roll e os campos de stat iguais aos da básica; só muda o modelo).
  O bônus é do ticket: +5 % (Normal) ou +10 % (VIP) de pang e EXP (`Premium.RatePercent`, Desc.iff), com qualquer bola.
