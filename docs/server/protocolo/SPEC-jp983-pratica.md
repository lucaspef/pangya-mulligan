# JP 983: Praticar (tipo 19), Tutorial e Chip-in Practice (14)

Fontes: exe (`pacotes_envio_983.tsv` / `pacotes_recebe_983.tsv`, **[EXE ✓]** = bate com a assinatura, **[?]** = não dá para
confirmar só pela assinatura), referência AGPL (**[REF]**: `Channel.cs` requestMakeRoom 1180-1560 e requestMakeTutorial
11952-12360, `GameModes/Practice.cs`, `Base/TourneyBase.cs`, `CourseManager.cs`, `Room.cs`, `RoomManager.cs`) e o código
atual (`JpTranslator*.cs`, `KrToJp.cs`, KR `GameHandler*.cs`, `MassGame.cs`). Ids "KR" = os do 645 que o tradutor converte.

## 0. Resumo

- **Praticar = sala tipo 19 de 1 jogador que roda a regra de TORNEIO no cliente.** A referência mostra o tipo 19 com
  `tipo_show = 4` (todo tipo > 14 vira 4, `Room.setTipo`) e `type_extend = 19`, e a classe `Practice` herda de `TourneyBase`
  (fluxo "em massa": 0x76 curto, 0x6D/0x6C/0x8C/0x79). **A base no KR é o `GameMode.Tournament` (MassGame/TourneyGame), não
  o Stroke.** Há duas variantes: **prática de campo** (modo de ordem 0..3, layout normal do 0x08) e **repetir buraco**
  (modo de ordem 4 = `M_REPEAT`, +5 bytes no 0x08).
- **Tutorial = offline no cliente** (`CTutorialMain`/`CTutorialTask`). O único C->S é **0xAE** (KR 0xA6) ao terminar cada
  missão; o servidor responde **0x11F** (KR 0x11B). Os ids já passam pelas faixas do tradutor; falta o formato da resposta
  por missão e os prêmios.
- **Chip-in Practice (14)**: não é trivial (GrandZodiacBase, ticket, pacotes 0x1EC..0x1F9). Só bloquear por enquanto
  (hoje o JP 14 cai no **Shuffle Course do KR**, ver Riscos).

## 1. Praticar (tipo 19)

### 1.1 Criar a sala: C->S 0x08 (KR 0x08)

| off | tipo | campo | prática |
|---|---|---|---|
| 0 | u8 | quick (opção) | |
| 1 | u32 | tempo da tacada (ms) | [?] valor que o diálogo manda |
| 5 | u32 | tempo do jogo (ms) | usado como relógio da prática (`Practice.init_game` → `startTime`) |
| 9 | u8 | máx jogadores | **1** (a referência recusa > 1) |
| 10 | u8 | tipo | **19** |
| 11 | u8 | buracos | quantos buracos (no repetir: quantas repetições) |
| 12 | u8 | mapa | |
| 13 | u8 | modo (ordem) | 0..3 = prática de campo; **4 = M_REPEAT** |
| 14 | u8 | **só no modo 4**: buraco a repetir (1..18) | [?] ver abaixo |
| 15 | u32 | **só no modo 4**: "fixed_hole" (flags de pin/vento fixos) | [?] |
| … | u32 | natural/short game (≤ 3) | |
| … | str | nome | o cliente manda "Single Player Practice Mode" (27) |
| … | str | senha | 8 caracteres (sala privada; a referência exige 8) |
| … | u32 | artefato | |

- **[EXE ✓]** três montagens: `makeroomdlg` `E1 E4 E4 E1 E1 E1 E1 E1 E4 ES ES E4` (normal, inclusive prática de campo) e
  `lobbymain/lobbytask FUN_00abaa10` `E1 E4 E4 E1 E1 E1 E1 E1 **E1 E4** E4 ES ES E4` (repetir buraco: u8 + u32 depois do
  modo). Tamanhos que a referência testa (com o id): **63** (campo) e **68** (repetir) = 24 + (2+27) + (2+8) [+5] ✓.
- A referência **ignora** os 5 bytes (`Skip(5)`) e fixa `hole_repeat = 1`, `fixed_hole = 7`. O certo é ler: **u8 buraco**
  e **u32 flags**. Hipótese [?]: 7 = pin+vento+clima fixos (a referência só usa `fixed_hole == 7` → pin sorteado uma vez).
  Logar os dois valores no 1º teste.
- **Bug atual do tradutor** (`MakeRoom`): pula 5 bytes quando **tipo == 19**; tem de ser quando **modo (head[13]) == 4**
  (na prática de campo não há bytes extras; o nome/senha sairiam deslocados).
- A sala de prática é criada **sem passar pelo lobby** (`leaveChannel`: "Sai da Sala Practice que não entra na lobby").

### 1.2 Resposta ao criar (ordem da referência)

1. **0x4A** (KR 0x48) configurações — **[EXE ✓]** `D2 D1 D1 D1 D1 D1 D4 D4 D1 D1 D1 D4 D4 D4 D1 DS`:
   `u16 nº, u8 tipo_show (4), u8 mapa, u8 buracos, u8 modo, [modo 4: u8 buraco_repetido, u32 fixed_hole], u32 natural,
   u8 máx, u8 galeria (30), u8 dormindo, u32 tacada, u32 jogo, u32 troféu, u8 pública (0 + str senha se privada), str nome`.
   O tradutor (`RoomSettings`) hoje **não** insere o u8+u32 do modo 4, nem converte o tipo 19→4.
2. **0x49** (KR 0x47) `i16 0 + RoomInfo 221` **[EXE ✓]** `D1 D1 D4 DB(0xdd)`. Campos que mudam na prática (offsets no 221):
   `+87 buracos`, **`+88 tipo_show = 4`**, `+91 modo (4 no repetir)`, `+101 troféu = 0`, **`+196 type_extend = 19`**
   (a referência: `type_extend = tipo` se tipo ≥ 11, senão 255).
3. **0x48** (KR 0x46) sub 0, slots (só ele; modo em massa = sem personagem).
4. **0x9E** clima `u16, u8 0` (como sala normal).
5. **Não** vai 0x47 (lista de salas, KR 0x45) ao canal nem 0x46 (KR 0x44, "jogador entrou em sala") ao lobby.

### 1.3 Início e partida (igual ao torneio do JP)

O cliente pede o início (C->S **0x0E**, [?] esperado logo depois; a referência dispensa "pronto" para a prática).

| ordem | JP | KR | layout JP | |
|---|---|---|---|---|
| 1 | 0x230, 0x231 | — | vazios (a referência manda no início de toda partida) | [EXE ✓] vazios; o KR não tem |
| 2 | **0x76** | 0x74 | **u8 tipo_show (4), u32 1, SYSTEMTIME** (21 bytes) | [EXE ✓] `D1 D1 D1 D1 D1 DB(0x10)` |
| 3 | **0x52** | 0x50 | u8 mapa, u8 tipo_show (4), u8 modo, u8 buracos, u32 troféu (0), u32 tacada, u32 jogo, 18 × (u32 id, u8 pin, u8 mapa, u8 nº), u32 semente, 18 × caixas | [EXE ✓] `D1 D1 D1 D1 D4 D4 D4 D4 D1 D1 D1` |
| por buraco | 0x5B vento, 0x53, 0x8D | 0x59, 0x51, 0x8B | como o torneio (`MassOutput`) | [EXE ✓] 0x5B `D1×5`, 0x8D `D4` |
| tacada | 0x6E (rival/posição) | 0x6C | já convertido (+u32) | [EXE ✓] |
| fim do buraco | **0x6D** | 0x6B | u32 oid, u8 buraco, u8 tacadas, i32 placar, u64 pang, u64 bônus, u8 (1 terminou / 0 não) | [EXE ✓] `D4 D1 D1 D4 D8 D8 D1` |

**Repetir buraco (modo 4), regras da referência (`CourseManager`):**
- Todos os buracos `i ≤ buracos` usam o **mesmo buraco físico** (`hole_repeat`); a sequência interna é 1..18.
- No 0x52 a referência escreve **o nº da sequência (1, 2, 3…)**, não o buraco repetido; o buraco físico só vai no
  0x4A/caixas (`getModo()==M_REPEAT ? hole_repeat : numero`). [?] Seguir a referência (é o que funciona); alternativa
  se o cliente abrir o buraco errado: mandar `hole_repeat` nos 18.
- **Pin**: sorteado só no 1º buraco e repetido (com `fixed_hole == 7`). **Vento**: sorteado só no 1º e repetido; o 0x5B
  usa o grau "fixo" (`degree.getDegree()`, não o embaralhado).
- C->S **0x141** (sem dados) **[EXE ✓]** (`golfrulebase`, botão "trocar vento"): sorteia de novo o vento do **próximo**
  buraco (`shuffleWindNextHole(hole)`). Sem resposta. O KR não tem par (ClientId(0x141) = −1 → hoje fica no log).
- Caixas/moedas: tipo 1 no repetir, cubo só de 3 em 3 buracos.

### 1.4 Fim (último buraco, sair, tempo)

- **Último buraco feito** (`Practice.checkEndShotOfHole`): **0x199** vazio **[EXE ✓]** (KR 0x19E pela faixa −5; o KR não
  manda) → `finish_practice`: **0x40** `u8 16, str nick, u16 0, i32 placar, u64 pang, u8 assist` (mensagem "terminou"),
  **0x6D** (…, 1), **0x6C** `u32 oid, u8 2` **[EXE ✓]**, depois `finish()`: **0xCE** `u8 0, u16 n, n × u32` (itens do
  campo) **[EXE ✓]** e **0x79** placar `i32 EXP, u32 troféu, u8 troféu ganho, u8 time, medalhas…` **[EXE ✓]** (o tradutor
  já põe as 24 do JP).
- **Sair da prática**: C->S **0x130** (sem dados) **[EXE ✓]** (`quitwindow`). A referência trata como fim de tempo
  (`timeIsOver`): pang calculado, 0x40, **0x6D (…, 0)**, **0x8C** vazio **[EXE ✓]**, 0xCE, 0x79. Diferente do torneio do
  KR: **não** soma par+5 nos buracos que faltaram ("o original não soma"). Sem par no KR (ClientId = −1).
- **Tempo acabou**: o mesmo do 0x130 (0x8C). Relógio = u32 "jogo" do 0x08 (0 = sem limite, como o KR).
- Depois o cliente manda **0x06** (UserInfo 265, já convertido) → `finish_game(6)`: grava pang (+EXP se > 0), 0x45 de
  estatísticas (sem recordes), Treasure Hunter, e **0xC8** `u64 pang, u64 0` **[EXE ✓]** `D8` (KR 0xC6).
- Saída da sala: a sala some com o jogador; ele **não** volta ao lobby (não estava). [?] O que o cliente manda para sair
  (0x0F?) e se aceita 0x44/0x45 de lobby depois — conferir no log.

### 1.5 Regras de recompensa (referência)

- **Pang**: prática de campo = **1/3** do pang e do bônus; repetir buraco = **1/6** (a referência ainda baixa para 10 %
  com bônus > 10 000 e 5 % com > 20 000; isso é invenção dela, opcional).
- **EXP**: 0 (a referência zera para nível < 70; o caddie nunca ganha). Sem troféu, medalha, prêmio de torneio, recorde
  de mapa nem conquista de partida (só os contadores "Hole Repeat"/"Course Practice").
- Spinning cube/moedas **não contam** (só em DEBUG). Itens ativos usados são consumidos normalmente.
- Sala: 1 jogador, privada, fora da lista; ninguém entra (nem convidado).

### 1.6 Mapear para o KR

- JP 19 → `GameMode.Tournament` + **flag de prática** na sala (`RoomSettings.Practice`, `HoleRepeat` u8, `FixedHole` u32;
  `HoleType` aceitar 4 só na prática — hoje o `Normalize` zera > 3).
- `PrepareStart`: no repetir, `HoleOrder[i] = HoleRepeat` para todos e a mesma semente de pin; vento com semente
  própria que só muda no 0x141.
- `TourneyGame`: sem `MatchTid`, sem bots, sem prêmios/medalhas; fim antecipado (0x130) sem par+5; avisar 0x199 no
  último buraco.
- Recompensa: `BeginGameEnd(..., pangRate 33 ou 17, expRate 0)`; sem recordes/estatísticas de mapa.
- Lobby: não mandar `RoomList(1/3)` nem `LobbyUser` para sala de prática; `Rooms.Lobby.Remove` sem efeito.

## 2. Tutorial

- **Entrada**: offline. O cliente não manda nada para começar (o `CTutorialMain` só monta o 0xAE) **[EXE ✓]**
  (`pk_cmp`: `tutorialmain … P(0xae) E1 E1 E4`). Não há sala nem GameMode (o 11/12 do KR não existe no JP: no JP 11/13 são
  Grand Zodiac).
- **C->S 0xAE** (KR 0xA6; −8 da faixa, já passa) **[EXE ✓]** `E1 E1 E4` (`contentsdoc`):
  `u8 finish (≠0 = última missão da categoria), u8 tipo (0 rookie, 1 beginner, 2 advancer), u32 bit`. Bits: rookie no byte 0
  (0x01..0x80, 8 missões), beginner no byte 1 (0x100..0x2000, 6), advancer no byte 2 (0x10000..0x40000, 3).
  Mesma ordem que o KR lê (`TutorialMissionAsync`: u8 último, u8 categoria, u32 bit).
- **S->C 0x11F** (KR 0x11B; +4 da faixa, já passa). Leitores no exe: `taskmanager D1 D1 DB(0xc)` e `CTutorialWidget
  D1 D1 D4 DB(0xc)` **[EXE ✓]**:
  - login (já vai): `u8 3, u8 0, u32 rookie, u32 beginner, u32 advancer` (= `u16 3 + 12` da referência) ✓.
  - **depois de cada missão (referência)**: `u8 tipo (0/1/2), u8 1, u32 flags dessa categoria` (6 bytes). O KR manda o
    formato do login (`u8 3, u8 0, 12`). [?] O widget pode só mostrar "concluído" com `tipo, 1` → converter no tradutor.
- **0x120** `u8` (S->C, "complete quest grade gift") existe no exe, a referência nunca manda; ignorar.
- **Erro** (referência): 0x44 `u8 0xE2, u32 código` — evitar (é o pacote de login); melhor só não responder.
- **Validação da referência** (opcional): recusa bit já feito e missão sem as anteriores (rookie: bits 2/3 exigem rookie ≥ 3,
  etc.; beginner/advancer: `((bit-1) & feito) == bit-1`).
- **Prêmios** (referência, por correio, remetente 0, mensagem "NICE TUTORIAL … CLEAR"):

| categoria | missão (bit no byte) | item × qtd |
|---|---|---|
| rookie | 1 | 0x1A000002 × 3 |
| | 2 | 0x1800000B × 3 |
| | 3 | 0x18000025 × 3 |
| | 4 | 0x18000005 × 3 |
| | 5 | 0x18000004 × 3 |
| | 6 | 0x1800000A × 3 |
| | 7 | 0x18000000 × 3 |
| | 8 | Pang Pouch 0x1A000010 × 1000 |
| | todas (finish ≠ 0) | Papel 0x1C000000 × 1 + Air Knight Lucky Set 0x10000014 × 1 |
| beginner | 1 | 0x1A000002 × 10 |
| | 2 | 0x18000028 × 1 |
| | 3 | 0x18000006 × 1 |
| | 4 | 0x1800000A × 3 |
| | 5 | 0x18000000 × 4 |
| | 6 | 0x18000001 × 3 |
| | todas (= 0x3F00) | 0x18000027 × 10 + Pang Pouch × 10 000 |
| advancer | cada | nada (item vazio) |
| | todas (= 0x70000, finish ≠ 0) | 0x18000000 × 1 + Pang Pouch × 30 000 |

  Conferir se os typeids existem no `pangya_jp.iff` (o correio do JP ainda está em `NotConvertedYet`: 0x210/0x211/0x215).

## 3. Chip-in Practice (Grand Zodiac, tipo 14)

Não é trivial: `ChipInPractice : GrandZodiacBase`, exige ticket **0x1A00017E** (consome 1), tempo fixo **30 min**
(`time_30s` tem de ser 1 800 000), 1 jogador, saída por **0x131** (sem dados) **[EXE ✓]**, pacotes próprios 0x1EC,
0x1EE, 0x1EF, 0x1F2..0x1F5, 0x1F9, 0x200, 0x201, 0x24F, 0xA7, 0xAA. Fica para depois; **por ora recusar o tipo 14** no
0x08 (e 11/13, Grand Zodiac de GM).

## 4. O que implementar (em ordem)

1. **`JpTranslator.MakeRoom`**: pular/ler os 5 bytes quando **modo == 4** (não quando tipo == 19); guardar
   `holeRepeat`/`fixedHole` e mandar ao KR (ex.: modo KR `Tournament` + bytes extras no fim do 0x08 KR, ou um campo novo
   no `RoomSettings`). Recusar tipos 11, 13, 14 (resposta 0x49 com erro). Converter 19 → Tournament + flag Practice.
2. **Domain**: `RoomSettings.Practice`, `HoleRepeat`, `FixedHole`; `Normalize` aceita `HoleType = 4` só na prática;
   `MaxPlayers = 1`; `CanJoin` recusa. `PrepareStart` do repetir (ordem fixa, pin fixo, vento com semente própria).
3. **Saída S->C da sala**: `KrToJp.Room` com `tipo_show 4` e `type_extend 19` (e modo 4); `RoomSettings` (0x4A) com
   `u8 buraco + u32 fixed` no modo 4 e tipo 4; não mandar 0x47/0x46 de lobby para a prática.
4. **Partida**: conferir o **0x76 em massa** (ver Riscos: hoje sai 1 byte curto); 0x52 com `tipo_show 4`, modo 4 e nº
   1..18 no repetir; 0x230/0x231 no início [opcional].
5. **C->S novos**: `0x130` → fim da prática (tempo acabou sem par+5: 0x40, 0x6D(0), 0x8C, 0xCE, 0x79);
   `0x141` → novo vento no próximo buraco; `0x131` → ignorar/recusar.
6. **Fim**: 0x199 no último buraco; recompensa com pang 1/3 ou 1/6 e EXP 0; sem troféu/medalha/prêmio/recorde.
7. **Tutorial**: no tradutor, guardar o `tipo` do último 0xAE e, no 0x11B KR seguinte, mandar `u8 tipo, u8 1, u32 flags
   [tipo]`; no KR, prêmios por correio (tabela acima) e a validação de ordem (opcional).
8. Testes no cliente falso (pela tabela de escritas do exe): 0x08 de 63/68 bytes, 0x4A modo 4, 0xAE → 0x11F.

## 5. Riscos

- **O torneio do JP ainda não foi confirmado no cliente** (`JP983-ESTADO.md`: "0x76 em massa, 0x79 com medalhas"). A
  prática depende dele.
- **0x76 em massa**: o KR monta `u8 0, u8 n, u8 0, u8 0, u8 0, SYSTEMTIME` e o tradutor manda
  `U8(tipo).U8(1).Bytes(k[3..])` = **20 bytes**; a referência manda `u8 tipo, u32 1, SYSTEMTIME` = **21**. A hora sai 1 byte
  deslocada. Conferir antes de testar a prática.
- **JP 14 vira Shuffle Course no KR** (`ModeFromJp` só troca 18↔14): uma sala Chip-in criada pelo cliente JP rodaria o
  Special Shuffle Course. Idem JP 11/13 → TutorialBasic/OfflineGhost do KR.
- Os 5 bytes do repetir (buraco, flags) e o nº no 0x52 no modo 4 não estão confirmados (a referência fixa buraco 1).
- O que o diálogo manda em "tempo do jogo" e se o cliente manda 0x0E sozinho: ver no log.
- Saída da prática sem lobby: o KR `LeaveRoom` pode mandar listas de lobby que o cliente não espera.
- 0x11F por missão: se o widget aceitar o formato do login, o passo 7 vira só prêmios.
- Prêmios do tutorial dependem do correio JP (ainda em `NotConvertedYet`).
