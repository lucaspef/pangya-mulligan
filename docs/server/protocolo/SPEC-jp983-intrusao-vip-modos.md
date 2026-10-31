# JP 983 — intrusão, Premium VIP e conferência dos modos de jogo

12/10/2026. Marcas: **[EXE ✓]** lido no `<pasta do jogo>\ProjectG.exe` (endereços VA), **[IFF ✓]** lido no
`data/pangya_jp.iff`, **[REF]** referência AGPL (`<ref>/Server/JP/GameServer`, só comportamento), **[?]** não conferido.
Código citado = worktree `heuristic-kirch-45a99b` (nada foi alterado lá).

## 0. Achados que valem para tudo

- **O leitor de pacotes do JP não confere o tamanho** [EXE ✓]. `0xA4DEB0` (u8) → `0x5AB140`, `0xA4DEC0` (u16) → `0x5AB150`,
  `0xA4DED0` (u32) → `0x5AB180`, `0xA4DF00` (bloco) → `0x5AB1C0` (memcpy): todos só fazem `buf[pos]` e `pos += n`, sem
  comparar com o tamanho. Pacote curto = o cliente lê bytes velhos do buffer (não cai na hora, mas lê lixo; se o lixo for uma
  contagem `u8 n`, o laço anda sobre lixo). Pacote longo = o resto é ignorado (inofensivo). Por isso as divergências abaixo
  dão "dados errados" e não queda, exceto onde há contagem.
- **0x76 dos modos em massa: já está certo.** O spec anterior (`sistemas-jp-modos.md`, "Achado importante") descrevia uma
  versão antiga. Hoje `JpTranslator.Room.cs` (`GamePlayers`) faz `U8(tipo).U32(1).Bytes(k[5..21])` = `tipo, 01, 00, 00, 00,
  SYSTEMTIME` = 21 B, e o exe (caso 0x76 em `0x9C22D7`) lê `u8 tipo, u8 n, u8, u8, u8, SYSTEMTIME` [EXE ✓]. Nada a fazer.
- O modo de jogo que o cliente usa nos testes é o "tipo de exibição" (byte 1 do 0x52), guardado em `doc+0x10594` (campo); no
  lobby o tipo da sala está em `doc+0x1094C` [EXE ✓]. Para o JP, 1 = MATCH (a "dupla" do KR), 4 torneio, 5 torneio em
  equipe, 6 guild battle, 7 pang battle, 10 approach; o Special Shuffle (18) aparece como 4 [EXE ✓ / REF `ROOM_INFO_TYPE`].

---

## 1. Intrusão (entrar numa sala de torneio já começada)

### 1.1 O que o cliente JP faz [EXE ✓]
- C->S **0x9D**: `u8 0|1, u16 sala` (`0x855F58` duplo clique, `0x856632` "sim" no diálogo) e `u8 2, 242 B` (`0x85790E`,
  `CIntrusion::PacketAnalysis`). Junto com o sub 1 vai **0x0C** `u8 7, 16 B` (`0x8566F9`). Mesmos layouts do KR
  (`SPEC-intrusao-tempo.md` §1.3). O tradutor já faz JP 0x9D → KR 0x9A (`ClientId`: −3).
- S->C **0x113** → `CIntrusion::PacketAnalysis` `0x8572D0`: lê `u8 tipo, u8 código`; tabela de saltos em `0x857994`
  indexada por `tipo − 3` (tipos 3..10, como o KR). Tamanhos dos structs no JP:
  | tipo | leitura no JP | diferença do KR 0x111 |
  |---|---|---|
  | 3 | `u16 sala, u32 decorrido, u32 limite, RoomInfo 0xDD` (`0x85733C`; grava em `doc+0x108F4`, abre "IntrusionAlarm") | sala 178 → **221** |
  | 4 | sub-função `0x856AA0` com o código (0..4) | ver abaixo |
  | 5 | — (diálogo "muitos usuários") | igual |
  | 6 | — (código já lido = recusa) | igual |
  | 7 | `str nick, RoomInfo 0xDD, u8 n, u32 bônus, n × Slot 0x15C` (`0x857547`) | sala 178 → 221; slot 338 → **348** |
  | 8 | `u8 n` (`0x857969` → `this+0x32`) | igual |
  | 9 | (`0x8577A6`) | igual [?] |
  | 10 | (`0x85795F`) `u32 oid, u8, u8, i32, i64, 242 B` | igual [?] |
  Tipo 4 (`0x856AA0`, tabela `0x8572B0`, código 0..4):
  | código | JP | KR |
  |---|---|---|
  | 0 ENTER | `RoomInfo 0xDD` | 178 → 221 |
  | 1 PLAYERS | `u8 n, n × Slot 0x15C` (`0x856D1B`) | 338 → 348 (só o slot, sem personagem) |
  | 2 | `u32` → `doc+0x10584` (taxa) | igual |
  | 3 RIVALS | `u8, u8, u8` (bits 5/6/7 de `doc+0x832`) + `SYSTEMTIME` só se o conteúdo 0x52 estiver ligado (`0x856E34`) | igual |
  | 4 START | `u8 mapa, u8 tipo, u8 ordem, u8 buracos, u32 troféu (doc+0x10494), u32 tacada, u32 jogo`, **18 × {u32 id, u8 pin, u8 nº}** (6 B, sem o mapa por buraco; laço `0x857035`), depois a semente e as caixas como no 0x52 (`0x9103E0`) [?: conferir a função] | KR: 18 × `{u32 semente, u8 nº}` (5 B) + caixas de 20 B → **converter como o 0x52** (`GameInit`) |
- Tipo 3 também conta quantos "Tiki Report" (0x1A000042) o jogador tem e, com ≥ 10, mostra outra mensagem (`0x857440`):
  cosmético.
- Hoje: `JpTranslator.Effects.cs` intercepta o 0x9D e responde `0x113 u8 6, u8 5` (não permitido); o KR 0x111 não tem id no JP
  (`ServerId(0x111)` = −1).

### 1.2 O que falta no KR (src)
Já feito no KR: relógio da partida (`TourneyGame.ElapsedMs`, 0x8B a cada início de buraco em `MassOutput.HoleStart`, fim do
tempo `TimeIsOver` + 0x8A). Falta toda a parte 3.1 do `SPEC-intrusao-tempo.md`:
1. `RoomPackets.RoomInfo`: `bAvailable = Waiting || IntrusionOpen`, `bIntrusion = IntrusionOpen` (hoje só `Waiting`). Ao abrir e
   ao fechar a janela, 0x45 sub 3 para o lobby; ao fechar, 0x111 `{8, 0, n}` à sala (a referência manda `0x113 u8 8, u8 0,
   u8 n` — `Tourney.cs:452`).
2. `TourneyGame` (`MassGame.cs`): `IntrusionOpen` (modo 4, não prática, sem senha, `ElapsedMs < 300 000`, lugar livre),
   `PlayedUids` (todos que começaram), `IntrudedUids`, `AddLate(RoomPlayer)` (cria o `MassPlayer`, `byGuid`, `Players` — a
   lista já é mutável), e no primeiro `Loaded(p)` do atrasado a sincronização ({10} de cada rival, 0x6A(guid,2) dos que já
   terminaram, {9} só para ele). `MassPlayer`: `Late`, `HolePang[18]` (gravar em `FinishHole`) para o {10}.
3. `RoomManager`: entrada "em jogo" (hoje `CanJoin` recusa `Playing` com 8): um `JoinLate` que só aceita pelo 0x9A.
4. Novo `GameHandler.Intrusion.cs`: C->S **0x9A** sub 0 (valida → `{3,0,sala,decorrido,limite,sRoomInfo}` ou `{6,código}`),
   sub 1 (valida de novo → `Join` + `AddLate` → `{4,0}{4,1}{4,2}{4,3}{4,4}` ao intruso, `{7}` a quem está no campo, 0x45 sub 3),
   sub 2 (log e ignora); 0x0C sub 7 (`MyRoom.cs`) aceito em silêncio. Códigos de recusa: §1.7 do spec KR.
5. `IMassOutput`/`MassOutput`: `IntruderJoined`, `RivalSync`, `QuitOrder`, `ExpMembers`.
6. START {4,4}: extrair de `RoomPackets.GameInit` um método comum (sem o byte de mapa do modo 14) e pôr `MatchTid` no u32
   (o 0x50 do KR ainda manda `GameSeed` ali — risco R6 do spec KR; o tradutor JP já troca por `MatchTid`).

### 1.3 O que o tradutor JP precisa (quando o KR existir)
- `ServerIdExceptions[0x111] = 0x113` (fica na faixa +2) e **apagar** o `fromClient[0x9D]` de `JpTranslator.Effects.cs`
  (aí o 0x9D passa como KR 0x9A).
- `toClient[0x111]` por tipo (`k[0]` tipo, `k[1]` código):
  - 3: `Bytes(k[..12])` + `Room(k[12..190])` (com `PracticeRoom` não precisa: só torneio normal).
  - 4/0: `Bytes(k[..2])` + `Room(k[2..180])`.
  - 4/1: `U8 tipo, U8 1, U8 n` + n × `Slot(338)` (sem personagem).
  - 4/3: igual.
  - 4/4: `u8 mapa, u8 TipoShow(modo), u8 ordem, u8 buracos, u32 MatchTid, u32 tacada, u32 jogo`, 18 × `{u32 semente, u8 semente%3,
    u8 nº}`, `u32 semente do curso`, 18 × `u8 0` — o mesmo que `GameInit` faz, menos o byte de mapa por buraco. [?: conferir
    em `0x9103E0` que o resto é idêntico ao 0x52.]
  - 7: `U8 7, U8 0, Str(nick)` + `Room(178)` + `U8 n, U32 bônus` + n × `Slot(338)` (atenção: o nick é `str` do KR em CP949;
    reescrever como string do JP, como o `0x40`).
  - 2, 5, 6, 8, 9, 10: como vêm.
- Bloquear a intrusão em sala do JP que o KR não conhece (GP, GZ, prática, Special Shuffle): já recusado no KR por modo ≠ 4.

### 1.4 Plano
1. KR: itens 1–6 de §1.2 com testes de unidade (janela, recusas, AddLate, ranking com intruso) e teste TCP (cliente falso KR).
2. JP: §1.3; teste TCP com cliente falso JP conferindo os tamanhos (3 → 2+12+221; 7 → n × 348; 4/4 → 4+12+108+4+18).
3. Teste real com dois clientes JP (um começa o torneio, outro entra pelo duplo clique). Ordem e riscos: R1–R8 do spec KR.

---

## 2. Premium (0x1A100002 Normal / 0x1A100003 VIP)

### 2.1 O que o jogo diz que o ticket dá [IFF ✓ Desc.iff]
- **Normal** (0x1A100002): "Increases Pangs and XP gained by **5 %**. Unlimited use: **Auto Calliper**, Time Boosters, Air
  Notes, Special Shot Commanders, Aztecs P, Club Set".
- **VIP** (0x1A100003): "Increases Pangs and XP gained by **10 %**. Unlimited use: Time Boosters, Air Notes, Special Shot
  Commanders. Bonus: **Club Set Premium, Comet Sakura Premium, Mascot Premium, Auto Caliper Premium**".
- Itens que existem no 983 [IFF ✓]: Lolo (Premium) **0x4000004C** e Weezing (Premium) 0x4000004B (Mascot.iff); Premium Comet
  0x140000D8 e **Sakura (Premium) 0x140000E9** (Ball.iff); **Eternal Voice ClubSet (Premium) 0x10001001** (ClubSet.iff — a
  referência diz "original não tem", mas existe); Premium Gift Box **0x1A0002BA**; Auto Caliper **0x1A000040**; Air Note
  0x1A0000C7; "Auto Calipers" 0x1A0003B2 e "Auto Calipers Compression Pack" 0x1A000348 (o exe não cita nenhum dos dois;
  ver 2.4).
- Premium Gift Box [IFF ✓ Desc]: "Memorial Coin (Premium), 100,000 Pangs, 500,000 Pangs, Platinum Card Ticket, 20 Fairy Tears,
  or 100 Pan Mastery" (um dos seis). Não está em nenhuma tabela de caixa do .iff (só Item.iff e Desc.iff): o conteúdo é do
  servidor (na referência, `BoxSystem` no banco).

### 2.2 O que a referência faz de verdade [REF]
`Game/System/PremiumSystem.cs`, `ItemManager.cs:3290-3470`, `LoginManager.cs:154`, `GameBase.cs:1836-1910`:
- Ativação (`addPremiumUser`, chamada ao receber o ticket com `dias = horas/24`): `premium_user = true`; bola premium
  **virtual** (id −1, `flag_time 0x6A`, vira o cometa equipado); se `isPremium2` (**VIP**): `addPremiumClubSet` e
  `addPremiumMascot(dias)` e manda a lista de itens; depois **0x9A** `i32 capability` e **0x216** com os itens novos.
- **Bugs da referência que anulam o VIP**: `getPremiumBallByTicket`, `getPremiumClubSetByTicket`, `getPremiumMascotByTicket` e
  `getPremiumBoxByTicket` só respondem para o ticket **Normal** (o ramo VIP está comentado). Então no VIP: bola typeid 0
  (não vai), club set typeid 0 (exceção, nada), mascote typeid 0 (nada). E no Normal o club set e o mascote nunca são
  chamados (só no `isPremium2`). Resultado na referência: **o Lolo nunca é entregue**.
- **Caixas pelo correio** (`addPremiumBox`): `getBoxQntdByTicket` = **4 no Normal**, 8 no VIP **comentado**, carta "Premium
  System - Gift Box" via `MailBoxManager.sendMessageWithItem`. **Ninguém chama `addPremiumBox`** (grep no fonte inteiro):
  código morto. Não há "quando" na referência.
- Login (`LoginManager` caso 4 → `updatePremiumUser`): só a bola virtual + 0x9A.
- Vencimento (`removePremiumUser`/`checkEndTimeTicket`): tira a bola (0x216), 0x9A sem o bit, **0x26D** `i32 unix do fim`,
  chat 0x40 tipo 7. Mascote e club set do VIP não são tirados (têm prazo próprio de item).
- **Auto Caliper** (`GameBase.requestFinishItemUsedGame`, :1836): no fim da partida, se `premium_user` e o ticket é premium
  (`isPremium` = Normal **ou** VIP, apesar do comentário "premium 2") e o jogador tinha o caliper na lista de itens passivos,
  regrava a contagem do caliper = nº de buracos jogados (só para a conquista 0x6C400075 / contador do caliper) e **não
  desconta** o caliper do inventário (:1883-1887). Para quem não é premium o caliper entra no desconto de passivos.
- Outros [REF]: Time Booster (`TourneyBase.cs:915` — invertido, só desconta **do premium**: bug; `VersusBase.cs:1917` certo),
  Auto Command (`GameBase.requestActiveAutoCommand` via `pgi.premium_flag`), GP sem ticket (`GrandPrix.cs:511`), sala com bot
  (`Room.cs:2160`). O "+% EXP/pang" está comentado (`GameBase.cs:2651`, `getExpPangRateByTicket` = 0).

### 2.3 Como o cliente usa o Auto Caliper [EXE ✓]
- O cliente **não pede** o caliper ao servidor e **não avisa** o uso: nenhum pacote C->S cita 0x1A000040 fora do modo Ghost
  (`0x82B19F` = 0xC2 sub 3 do `ghostplayer.cpp`).
- No início da partida (`0x6A8340`, se o conteúdo local 0x31 estiver ligado e `doc+0x1048C` ≠ 1/2): monta a lista dos typeids
  de caliper que existem no Item.iff (`0xA20330`: só **0x1A000040**), soma a **quantidade no inventário** (`0x6A8240`, u16
  do item) e grava em `doc+0x10700` (contador) e `doc+0x10704 = 1` (ligado).
- A cada buraco (`0x690CF8`): `contador > 0` → `contador--`; senão desliga (`doc+0x10704 = 0`). (Pula nos tipos 0x14/0x15
  de `doc+0x109B8`.) O HUD lê `doc+0x10704` (`0xB2E6A0`).
- **Não há teste do bit premium (0x4000) nesse caminho**: premium sem caliper no inventário não ganha caliper. Para o
  "unlimited Auto Calliper" funcionar o jogador precisa ter o item; o servidor só deixa de descontar.

### 2.4 O que o nosso servidor faz hoje
- `Domain/Players/Premium.cs` + `JpSystems.Premium.cs`: ticket com prazo, 0x9A com 0x4000, **Premium Comet 0x140000D8** em pilha
  de 999 (para Normal **e** VIP), 0x26D/0x40 tipo 7 no fim; Auto Command e GP grátis (`JpSystems.Effects.cs:50`,
  `JpSystems.GrandPrix.cs:111`). Nada de mascote, club set, caixas, caliper, Time Booster ou % de EXP/pang.
- **Passivos não são descontados por ninguém** (não há consumo de 0x1A000040 no KR: `GameHandler.Play.UseItem` só trata o
  0x17 dos itens ativos). Ou seja, hoje o caliper já é "grátis" para todos (o cliente desconta localmente, mas relogar volta
  a quantidade). Isto é um furo do KR, não só do premium.

### 2.5 Plano (o que fazer, em ordem)
1. **Mascote Lolo (VIP)**: em `Premium` acrescentar `VipMascot = 0x4000004C`; na ativação do VIP (compra/GM — o mesmo ponto do
   `ShopService.cs:194` que soma o prazo do ticket), dar o mascote com prazo = prazo do ticket (`ExpiresAt` igual ao do
   ticket; se já tem, estender). Mandar 0x216 + a lista de mascotes do JP (KR 0xDF → JP 0xE1, já convertido por
   `KrToJp.Mascot`). No vencimento do ticket o mascote vence sozinho pelo prazo do item (o sistema de itens por tempo já
   tira) [?: conferir que o mascote com prazo some no relógio do KR].
2. **Bola do VIP**: o Desc diz "Comet Sakura Premium" → `VipComet = 0x140000E9` para o VIP (Normal continua 0x140000D8).
   `PremiumCheckAsync` escolhe a bola pelo ticket; no fim tira a que deu.
3. **Club set (VIP)** 0x10001001 com prazo, como o mascote [opcional; a referência não entrega].
4. **Caixas 0x1A0002BA pelo correio**: a referência não entrega (código morto). Proposta: ao **comprar/receber** o ticket (não
   no login, para não repetir), `Mail.SendSystemAsync(conta, "@Premium", "Premium Gift Box", [(0x1A0002BA, n)])` com
   `n = 4` (Normal, valor da referência) e `n = 8` (VIP, valor comentado na referência) [?: quantidade oficial]. Uma vez por
   compra. E **abrir a caixa**: o JP abre por C->S 0xEF `u32 tid` (= KR 0xF1, `GameHandler.MagicBox.cs`, hoje responde
   código 4 "não abre" para tudo que não é Spin Cube) → acrescentar um sorteio com a lista do Desc (Memorial Coin (Premium)
   0x1A000272, 100 000 pang, 500 000 pang, Platinum Card Ticket [?: typeid], 20 × Fairy's Tears 0x1A000136, "100 Pang
   Mastery" [?: typeid]) em config, resposta 0x1A2 → JP 0x19D (já mapeado).
5. **Auto Caliper**: implementar no KR o desconto de passivos no fim da partida (caliper = nº de buracos jogados, limitado ao
   que ele tinha; 0x216/0xA5 com a quantidade nova) e **pular o desconto do caliper quando `Premium.IsActive`** (Normal e VIP,
   como a referência). Sem pacote novo do cliente. (Opcional VIP "Auto Caliper Premium": dar um estoque de caliper como a
   bola — mas sem confundir com os comprados; deixar para depois.)
6. **Time Booster grátis** (0xC5/KR, `TIME_BOOSTER_TYPEID 0x1A000011`): não descontar no premium [?: onde o KR desconta].
7. **+5 % / +10 % de EXP e pang**: no `BeginGameEnd` do KR, por um gancho do JP (o KR não sabe de premium) [opcional; a
   referência não aplica].

---

## 3. Modos de jogo no JP: KR × JP × tradutor

Método: para cada pacote que o KR manda no modo (`InGame.cs`, `MassOutput.cs`, `GameHandler.GameEnd.cs`), o id JP pelo
`ServerId`, o que o caso do exe lê (`pacotes_recebe_983.tsv` + desmonte dos casos com condição) e o que o tradutor faz.

### 3.1 Tabela geral (pacotes comuns; ✓ = bate)
| KR → JP | KR manda | JP lê [EXE ✓] | situação |
|---|---|---|---|
| 0x59 → 0x5B vento | 5 B | `D1 D1 D1 D1 D1` | ✓ |
| 0x51 → 0x53 | u32 | D4 | ✓ |
| 0x58 → 0x5A item | u32 ×3 | D4 D4 D4 | ✓ |
| 0x5A → 0x5C estouro | u32 | D4 | ✓ |
| 0x5E → 0x60 drop | 12 B | DB(0xC) | ✓ |
| 0x5F → 0x61 saiu | u32 | D4 | ✓ |
| 0x61 → 0x63 vez | u32 [+u16 no pang battle] | `D4` + `D2` **só se `doc+0x10594 == 7`** (`0x69FD3B`) | ✓ |
| 0x63 → 0x65 próximo buraco | vazio [+u32 no pang battle] | `D4` **só no modo 7** (`0x69FDC3`) | ✓ |
| 0x64 → 0x66 placar | `u8 n, [pang battle: u32, u32], n × 33` | `D1`, `D4 D4` só no modo 7, n × `DB(0x21)` (`0x69FE41`) | ✓ |
| 0x8F → 0x91 fim da dupla | `u8 n, n × 33, u8, u8, u8` | mesmo caso; `D1 D1 D1` só quando o id é 0x91 (`0x69FF3D`) | ✓ |
| 0x9A → 0x9C shot command (eco) | `u8, u32` | D1 D4 | ✓ |
| 0xC5 → 0xC7 time booster | f32, u32 | DB(4) D4 | ✓ |
| 0xF8 → 0xFA itens ganhos | `u16 n, n × {u32, u8, u16 m, m × u32}` | idem (`0x9CC51F`) | ✓ |
| 0x192 → 0x18D cut-in | `u8 0, u16 0` | `u8 f`; f = 1 → `u32, u32, 0xB8 B…`; senão o ramo curto (`0x6A3EF5`) | ✓ |
| 0x6A → 0x6C, 0x6B → 0x6D | 5 B, 27 B | D4 D1 / D4 D1 D1 D4 D8 D8 D1 | ✓ |
| 0x6C → 0x6E posição | `u32, u8, f32, f32, [approach: u32, u32], u16` + tradutor insere `u32 0` após o byte 13 | `D4 D1 DB(8) D4` + `D4 D4` **só se modo == 10** + `D2` (`0x6A09CE`) | ✓ (19 B no torneio, 27 B no approach) |
| 0x8A → 0x8C, 0x8B → 0x8D | vazio, u32 | —, D4 | ✓ |
| 0xCC → 0xCE meus itens | `u8 0, u16 n, n × u32` | D1 D2 D4 | ✓ |

### 3.2 Divergências encontradas (com correção)
| # | modo | id JP (KR) | esperado pelo JP [EXE ✓] | mandado hoje | efeito | correção |
|---|---|---|---|---|---|---|
| D1 | **guild battle** (6) | 0x79 (0x77) | `u32 EXP, u32 troféu, u8, u8,` **`u32 meu pang de guilda, i32 meus pontos`** (só 2 × u32, em `doc+0x10FDC/0x10FE0`), `12 × {u32, u32}` (96 B), sem os 24 B finais (no modo 6 e 10 não lê) — lobby `0xAC3132`, campo `0x6A0EFF` | `… u8, u8, 4 × u32` (pang da vermelha e da azul a mais; o KR 645 lê 4 — `SPEC-guildmatch.md` §3.2) + 96 B + 24 zeros do tradutor | as 12 medalhas/prêmios saem deslocadas 8 B (o 1º slot vira "pang vermelho/azul") | no tradutor, `toClient[0x77]` com `CurrentRoom.Settings.Mode == GuildMatch`: `Bytes(k[..18])` (até "meus pontos") + `Bytes(k[26..122])` (as 12 medalhas), sem os 24 B. A referência (`GuildBattle.cs:689`) manda os 4 × u32 — erro da referência |
| D2 | **approach** (10), no campo | 0x79 (0x77) | no `CGolfTask` com modo 10 (`0x6A0DFD`): **`i32 oid` (−1 = nenhum)**; se ≠ −1 procura o oid e manda C->S 0x2F `u32, u8 5, …` (pede o perfil); depois `12 × 8 B`; sem EXP/troféu, sem os 24 B | o KR manda o 0x77 do torneio no fim do approach (`MassOutput.GameOver` → `SendMassResult`), logo depois do 0x149 (JP 0x151), com o cliente ainda no campo: o EXP vira "oid", as medalhas saem deslocadas 6 B | lixo nas medalhas; se o EXP coincidir com um oid, um 0x2F a mais | a referência **não manda 0x79 no approach** (`Approach.finish` → `sendPlacar` override = 0x14E, depois 0x151). Corrigir no tradutor: modo NewApproach → não mandar (`null`), ou mandar `I32(-1)` + `k[10..106]`. Recomendo `null` (como a referência) e conferir que o cliente manda o 0x06 do fim mesmo assim [?] |
| D3 | todos (fim) | C->S **0x9C** (vazio, `golfrule/golftask` `0x69CCD0`) | a referência responde **0x10E** "últimos 5 jogadores" (`packet09C` → `pacote10E(l5pg)`) | vira KR **0x99** (`CAfterChannel`, ignorado); sem resposta | lista "últimos jogadores" vazia [?: o cliente espera?] | opcional: `fromClient[0x9C]` → responder 0x10E vazio (layout `pacote10E` [?]) e `null` |
| D4 | stroke/dupla/pang battle/torneio | 0x24F, 0x199, 0x16A | 0x24F `u32` (`0x9D5FEA`: posta msg 2 a um ator; ≠ 0 = erro), 0x199 vazio (`doc` flag), 0x16A `u32` semente do efeito do mascote | o KR não manda (não existem no KR) | o stroke já termina sem eles no JP (testado) | opcional: no fim (`toClient[0x64/0x8F/0x77]`), mandar `0x24F u32 0` depois, como a referência (todos os modos) [?] |
| D5 | Treasure Hunter | 0x134 (0x12C) | `n × {u32 uid, u32 tid, i32 id, i32 qtd, u8, u16 qtd_depois/0x8000, u16 qtd_depois%0x8000}` (21 B; `0x9D08DF` → lista → diálogo `0x83BDA0`) | `{u32 uid, u32 tid, u32 id, u16 qtd, u8 0, i32 0, u16 0}` (21 B): bytes 12–15 dão a qtd certa; **"qtd depois" = 0** | se o cliente usar a qtd-depois para o inventário, a pilha mostra 0 até relogar [?] | no tradutor, reescrever cada registro: `u32 uid, u32 tid, i32 id, i32 qtd, u8 0, u16 (total/0x8000), u16 (total%0x8000)` com o total do inventário do KR (`kr.Player.Find(id).Quantity`) |
| D6 | match 1×1 (KR 3) | — | o JP não tem o tipo 3 no `ROOM_INFO_TYPE` (3 = "GAME_TYPE"); o MATCH do JP (1) = a **dupla** do KR (fim 0x91, `Match.cs:1292` da referência) | o KR deixa criar 3 | sala 3 do KR (se aparecer na lista do JP) | recusar o tipo 3 no `MakeRoom` do tradutor (`JpTranslator.Room.cs`) e não listar salas 3 para o JP [?: o diálogo do JP oferece o 3?] |

### 3.3 Por modo (resumo)
- **Dupla / "Match" do JP (KR 1 = JP 1)**: tudo bate (0x63 sem u16, 0x65 vazio, 0x91 com o trailer, 0x7D troca de time
  `D4 D1`, 0x132 pontos do Treasure Hunter). A referência manda também 0x9E (clima `u16, u8`) e 0x40 tipo 16 (chat "terminou",
  `str nick, u16 0, u32 pontos, u64 pang, u8 assist`) — opcionais. C->S 0x35 `u8, u8` igual.
- **Pang battle (7)**: 0x63/0x65/0x66 com os campos condicionais do modo 7 — bate. Nada a fazer.
- **Approach (10)**: 0x6E (27 B), 0x14E/0x150 (`u8 n, n × 24`), 0x14F (13 B de zeros do tradutor), 0x151 vazio — batem.
  Divergência **D2** (0x79). O 0x150 do JP manda C->S 0x2F `u32, u8` (perfil) — o KR já responde ao 0x2F. C->S 0xCB (fim)
  já é descartado no tradutor.
- **Torneio / torneio em equipe (4/5) e Special Shuffle (18 → KR 14, exibido como 4)**: 0x76 (21 B) ✓, 0x52 com `mapa|0x80` e
  mapa por buraco ✓, 0x6E 19 B ✓, 0x79 + 24 B ✓, 0x8D/0x8C ✓. Só JP e opcionais: 0x113 tipo 8 (`u8 8, u8 0, u8 n`, a
  referência manda quando a janela de entrada fecha), 0x198 (`u32 moeda SSC, u32 qtd`, fim do Special Shuffle, para o dono).
- **Guild battle (6)**: 0xBF (`u8 n, n × 9`) ✓, 0xC2 (`D4 D2 D2 D1 D1`) ✓, 0x76 ✓. Divergência **D1** (0x79).
- **Treasure Hunter**: 0x131 (sub 1: `u8 n, n × 5`) ✓, 0x132 `u32` ✓, 0x133 `u8 n, n × 11` ✓, 0x134 21 B com a ressalva **D5**.

### 3.4 Plano por prioridade
1. **D1** (guild battle, 0x79 com 8 B a mais) — uma condição no `toClient[0x77]`; teste de unidade do tradutor com um 0x77
   de modo 6 (saída: 18 + 96 B).
2. **D2** (approach, 0x79 no campo) — `null` no modo 10; testar no cliente que a tela do approach fecha e o EXP é gravado
   (0x06).
3. **D5** (Treasure Hunter, qtd depois) — reescrever o 0x12C com o total do inventário.
4. **D6** — recusar o tipo 3 vindo do JP.
5. Opcionais: 0x24F no fim (D4), 0x10E para o 0x9C (D3), 0x198 do Special Shuffle, 0x113 tipo 8.
