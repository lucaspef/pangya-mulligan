# JP 983: Grand Zodiac (HIO Event, tipos 11/13) e Chip-in Practice (14)

Fontes: **[EXE ✓]** = bate com a assinatura do exe 983 (`pacotes_recebe_983.tsv` D1/D2/D4/D8/DB/DS, `pacotes_envio_983.tsv`
E…, `pk_cmp.tsv`/`rx_cmp.tsv`); **[EXE~]** = o exe só mostra parte (a leitura fica numa subfunção) ou bate por um handler
com símbolo do 645 (`CHoleInOneGameHandler::On01/On10/On18`); **[REF]** = só a referência AGPL
(`Game/Base/GrandZodiacBase.cs`, `GameModes/GrandZodiac.cs`, `ChipInPractice.cs`, `Event/RoomGrandZodiacEvent.cs`,
`System/GrandZodiacEvent.cs`, `Channel.cs`, `Room.cs`, `FilterRoom.cs`, `Models/grand_zodiac_type.cs`, `PlayerInfo.cs`);
**[IFF]** = `data/pangya_jp.iff`; **[?]** = não confirmado. Ids são os do JP (C->S e S->C).

## 0. Resumo

- **O cliente tem o modo inteiro** (`CGolfRuleHoleInOne`, `CHoleInOneGameHandler`; o 645 também tem, com os mesmos ids
  0x1EC..0x201, 0x12C, 0x12D, 0x137 — `pk_cmp`/`rx_cmp` "igual"). Mapa **0x40 "GRAND ZODIAC"** (Course.iff #21: dados
  `mland`, som `amb_hio1_loop`, par/min/max todos 0 — não é um campo de 18 buracos). [IFF]
- **Regra**: partida por **tempo** (não por buracos). Cada tacada sai do tee; errou = volta ao mesmo tee (0x1EE); acertou
  (hole-in-one/chip-in) = pontos e **próximo buraco** (0x1F4, vento novo). Ganha quem somar mais pontos no tempo.
- **Tipos** (referência, `ROOM_INFO_TYPE`): **11 GZ Intermediário**, **13 GZ Avançado** (salas de **evento** que o servidor
  cria; jogador comum **não** cria: a referência recusa até para GM no `FilterRoom.ValidateForbiddenModes`), **14 Chip-in
  Practice** (sala de 1 jogador que **qualquer um** abre, com o ticket **0x1A00017E "Chip-In Practice Ticket"**, 30 min).
  O 12 não existe. Na sala, `tipo_show = 11` para 11/13/14 e `type_extend = tipo` (`Room.setTipo`).
- **Mínimo jogável recomendado**: Chip-in Practice (14) primeiro (1 jogador, sem sincronização, sem golden beam, sem
  troféu). Depois salas de evento 11/13 (multijogador, contagem, golden beam, jackpot, troféus, pontos GZ).
- **Mapeamento**: **não** dá para reaproveitar o `TourneyGame` (ele acaba ao completar `Holes` buracos e manda 0x6B/0x6A/0x77;
  o GZ é por tempo, buraco "infinito", e o fim é 0x1F2/0x1F3). Proposta: **`GrandZodiacGame : MassGame`** novo no Domain
  (reaproveita o despacho `HandleMass` do KR: 0x11/0x12/0x1B/0x1C) + **`IZodiacOutput`** implementado no JP983 (os S->C
  do GZ não têm id no KR que o tradutor conheça; vão direto em ids do JP).

## 1. Criar e entrar na sala

### 1.1 Chip-in Practice (14): C->S 0x08

Layout **normal** do 0x08 (o mesmo do `makeroomdlg`), sem os 5 bytes do repetir buraco. [EXE ✓ assinatura `E1 E4 E4 E1 E1
E1 E1 E1 E4 ES ES E4`; valores [REF]/[?]]

| off | tipo | campo | chip-in |
|---|---|---|---|
| 0 | u8 | quick | |
| 1 | u32 | tempo da tacada (ms) | [?] (logar) |
| 5 | u32 | tempo do jogo (ms) | **1 800 000** (30 min; a referência recusa outro valor) |
| 9 | u8 | máx jogadores | **1** |
| 10 | u8 | tipo | **14** |
| 11 | u8 | buracos | **1** (`FilterRoom.ValidateHoleCount`) |
| 12 | u8 | mapa | **0x40** [?] (forçar 0x40 no servidor de qualquer jeito) |
| 13 | u8 | modo (ordem) | 0 [?] |
| 14 | u32 | natural/short game (≤ 3) | |
| 18 | str | nome | não vazio (a referência compara com "Single Player Practice Mode" ao trocar o título) |
| … | str | senha | vazia ou ≥ 8 caracteres (sala privada) |
| … | u32 | artefato | |

- Resposta: como a prática (SPEC-jp983-pratica.md §1.2): **0x4A** com **tipo 11** (tipo_show), **0x49** `i16 0 + RoomInfo 221`
  com `+88 tipo_show = 11`, **`+196 type_extend = 14`**, `+87 buracos = 1`, troféu 0; **0x48** slots; **0x9E** clima. A sala
  é privada, fora da lista (`RoomManager`: tipo 14 conta como prática), jogador com `place = 2` (não volta ao lobby). [REF]
- **Ticket**: conferido e **gasto no início** (pedido de começar), não na criação: sem ticket ou qtd < 1 → falha do início
  (a referência manda `pacote049 CREATE_GAME_CREATE_FAILED2`, i.e. **0x49 u8 erro**). Gasta 1 (servidor + banco). [REF]
  [?] se o cliente já tira 1 sozinho (o SSC tira); mandar **0x216** com antes/depois absolutos resolve os dois casos.
- Começo: a referência dispensa "pronto" (14 = 1 jogador); o cliente deve mandar **0x0E** sozinho como na prática [?].
- Sair durante a partida: **C->S 0x131** (vazio) **[EXE ✓]** (`quitwindow FUN_006efeb0` manda 0x130 *ou* 0x131; 0x130 é o
  da prática, 0x131 o do chip-in).

### 1.2 Grand Zodiac de evento (11 Intermediário, 13 Avançado)

Ninguém cria pelo 0x08 (a referência lança "modo proibido" para 11/13). O **servidor** abre as salas no horário do evento
(`GrandZodiacEvent.checkTimeToMakeRoom` → `Channel.makeGrandZodiacEventRoom`) e os jogadores entram pela lista (0x09
normal, `E2 ES` **[EXE ✓]**). [REF]

| campo da sala | Intermediário | Avançado |
|---|---|---|
| tipo / tipo_show / type_extend | 11 / 11 / 11 | 13 / 11 / 13 |
| nome | "HIO Event (Intermediate)" | "HIO Event (Advanced)" |
| mapa / buracos / modo | 0x40 / 1 / 0 | 0x40 / 1 / 0 |
| máx jogadores | 100 | 100 |
| tempo do jogo (`time_30s`) | **7 min** (420 000 ms) [REF; o tempo do oficial é [?]] | idem |
| dono | −2 (servidor), `flag_gm = 1`, `state_flag = 0x100`, troféu = TROFEL_GM_EVENT [REF] | idem |
| salas por canal | `max(1, ceil(jogadores do canal / 200))` de cada tipo | idem |

- **Horários**: tabela no banco da referência (`range_time`: início, fim, tipo `0 todos / 1 só Int / 2 só Adv`). No nosso
  servidor: config `GrandZodiac.Schedule` (lista "HH:MM-HH:MM tipo") ou **comando de GM** (`!zodiac int|adv`). [proposta]
- **Início automático** (`RoomGrandZodiacEvent.waitTimeStart`): 2 min depois de criada **com ≥ 1 jogador**, ou na hora se
  lotar (aviso **0x40 tipo 12** "sala cheia", `u16 0, u16 0, u32 10`); depois **contagem de 10 s** com **0x40 tipo 11**
  `u8 11, str "", str "", u32 segundos` (a cada 30 s / 10 s / 1 s; o mesmo do GP, já usado no `JpSystems.GrandPrix`).
  Esvaziou durante a contagem → volta a esperar. Sala vazia depois do jogo → destruída. [REF]
- Início (antes do 0x76): **0x230** e **0x231** vazios [EXE ✓ vazio] e **0x77** `u32 rate de pang` [EXE ✓ `D4`] a todos. [REF]

## 2. Sequência de pacotes (do início ao fim)

`oid` = guid do jogador na sala (no nosso servidor, o `RoomPlayer.Guid`). "a todos" = broadcast na partida.

### 2.1 Começo e carregamento

| # | dir | id | layout | marca | nota |
|---|---|---|---|---|---|
| 1 | S->C | 0x230, 0x231 | vazios | [EXE ✓] | só na sala de evento [REF]; opcional |
| 2 | S->C | **0x1F9** | `i32 cup início, i32 cup fim, u32 pontos GZ` (12) | [EXE ✓ D4 D4 D4] | **antes do 0x76/0x52**, a cada jogador; início = fim = tamanho do copo (§3.4) |
| 3 | S->C | **0x76** | `u8 tipo_show (11), u32 1, SYSTEMTIME` (21) | [EXE ✓] | como o torneio em massa |
| 4 | S->C | **0x52** | `u8 mapa 0x40, u8 tipo_show 11, u8 modo, u8 buracos 1, u32 troféu, u32 tacada, u32 jogo, 18 × (u32 id, u8 pin, u8 mapa, u8 nº), u32 semente, 18 × caixas` | [EXE ✓] | `GameInit` atual serve, com mapa 0x40 e tipo 11 |
| 5 | C->S | 0x48 | carregando % | [EXE ✓] | como sempre |
| 6 | C->S | **0x1A** | init hole (nº, tee x/z, pin x/z…) | [EXE ✓] | a referência guarda o tee como posição do jogador e o nº do buraco |
| 7 | S->C | 0x9E | `u16 clima, u8 0` | [EXE ✓ D1 D1 D1] | resposta do 0x1A na referência |
| 8 | S->C | 0x5B | `u8 vento, u8 card (1 = vento azul), u16 grau, u8 1 (reseta)` | [EXE ✓ D1×5] | idem |
| 9 | C->S | **0x11** | `u32` (a referência lê como "tamanho do copo") | [EXE ✓ E4 `OnInitFinished`] | terminou de carregar |
| 10 | S->C | **0x1EC** × 10 | `f64 valor` (8) | [EXE ✓ DB(8)] | **10 pacotes**, os **mesmos 10 valores para todos** da partida (§3.5) |
| 11 | C->S | **0x12D** × 10 | `f64` (eco dos 8 bytes) | [EXE ✓ cliente responde `P(0x12d) EB(8)` dentro do case 0x1EC] | só ler/ignorar |
| 12 | S->C | **0x201** | vazio | [EXE ✓ sem leitura] | "terminou de carregar o campo GZ" |
| 13 | C->S | 0x34 | vazio | [EXE ✓ `CGolfRuleHoleInOne::RequestShotReview`] | fim da intro do personagem; a referência só zera contadores, sem resposta |
| 14 | C->S | **0x137** | vazio | [EXE ✓ `CHoleInOneGameHandler::SendTimeOver`] | "pronto para o 1º buraco"; **sincroniza todos** (o nome no 645 é `SendTimeOver` [?]) |
| 15 | S->C | 0x8D | `u32 0` | [EXE ✓ D4] | quando **todos** mandaram 0x137; o GZ não usa o relógio do torneio (o tempo vai no 0x200) |
| 16 | S->C | 0x53 | `i32 oid` (próprio) | [EXE ✓ D4] | começa o buraco |
| 17 | S->C | **0x1EE** | `i32 oid, f32 x, f32 z` (12) | [EXE~ provável = `On01 D4 DB(8)`] | posição (tee) do jogador; a todos no tipo 11, só a ele nos outros [REF] |
| 18 | S->C | **0x1F4** | `u32 1` | [EXE ✓ D4] | "começa/próximo buraco" |
| — | serv | — | inicia o relógio (`time_30s`) e a agenda do golden beam | | |

### 2.2 Cada tacada

| # | dir | id | layout | marca | nota |
|---|---|---|---|---|---|
| 1 | C->S | **0x12** | `u16 opção, [opção 1: u8, i32, i32 power shot], bloco 50` | [EXE ✓] | sem eco (modo em massa). No bloco: **+0x10 u8 acerto_pangya_flag**, **+0x11 u32 special_shot** (setas), +0x1D u32 tempo |
| 2 | C->S | 0x199 | vazio | [EXE ✓] | "tacada enviada" [REF]; ignorar |
| 3 | C->S | **0x1B** | 38 bytes cifrados com a chave da sala (sem a chave no fim) | [REF: exige 38] | decifrado: `i32 oid, f32 x,y,z, u8 estado (+16), u8 bunker, u8, u32 pang (+19), u32 bônus (+23), u32 display (+27; bit 8 = acertou o buraco), u32 shot (+31; bit1 tomahawk, bit2 spike, bit3 cobra), i16 tempo, u8 penalidade GP` |
| 4 | S->C | 0x6E | `i32 oid, u8 buraco, f32 x, f32 z, u32 estado shot, u16 tempo` | [EXE ✓] | a todos (rival). A referência manda a **posição do tee**, não a da bola |
| 5 | S->C | **0x1F5** | `u32 n, n × u32 tipo de bônus` | [EXE~ leitura em subfunção; `On10 D4 D4` provável] | só se acertou e **n > 1**; quando chegaram 0x12 **e** 0x1B (§3.1) |
| 6 | C->S | 0x12F | `bloco 0x57` (fim da bola) | [EXE ✓ EB(0x57)] | referência: repassa **0x1F7** `i32 oid, u8 buraco, 0x57` a todos [EXE ✓ D4 D1 DB(0x57)]; hoje o tradutor cala o 0x12F — pode continuar calado no mínimo |
| 7 | C->S | **0x1C** | fim da tacada | [EXE ✓] | dispara o resto ↓ |
| 8 | S->C | 0xCC | `i32 oid, u8 0` (sem itens de campo) | [EXE ✓ D4 D1 (+DB(0x800) se n > 0)] | só a ele |
| 9a | S->C | **0x1EF** (acertou) | `i32 oid, i32 total antes, i32 pontos do buraco, i32 total novo` (16) | [EXE ✓ D4×4] | **a todos** |
| 9b | S->C | 0x40 tipo 15 (acertou, 10 %) | `u8 15, str nick, u16 0, u32 typeid, u32 qtd` | [EXE ✓ ramo D4 D4] | item que caiu (§3.3); a todos |
| 9c | S->C | **0x1F4** (acertou) | `u32 1` | [EXE ✓] | próximo buraco |
| 9d | S->C | 0x5B (acertou) | vento novo (sorteado), `u8 1` reseta | [EXE ✓] | |
| 9e | S->C | **0x200** (acertou) | `u32 segundos` | [EXE ✓ D4] | a referência manda **o tempo decorrido** (`m_timer.getElapsed()/1000`), apesar do nome "remain" [?] |
| 9f | S->C | 0x1EE (acertou) | `i32 oid, f32 x, f32 z` | [EXE~] | posição guardada (tee do 0x1A) |
| 10 | S->C | **0x1EE** (errou) | `i32 oid, f32 x, f32 z` | [EXE~] | **volta ao tee**; a todos no 11, só a ele nos outros |

[?] Se o cliente manda **0x1A de novo** a cada buraco novo (o tee muda). A referência não depende disso (manda 0x1EE com a
posição do último 0x1A). Logar no 1º teste; se vier, atualizar o tee e **não** responder de novo com 0x9E/0x5B.

### 2.3 Golden beam (só 11/13)

| quando | dir | id | layout | marca |
|---|---|---|---|---|
| decorrido ≥ T − 60 s | S->C | **0x1F0** | vazio | [EXE~ case sem leitura] |
| decorrido ≥ T − 30 s | S->C | **0x1F1** | vazio [REF] (a nota anterior com `u32 typeid, i32 id` era o 0x1FA) | [EXE~] |
| logo depois do 0x1F1, 1 ganhador | S->C | 0x40 **tipo 13** a todos | `u8 13, str nick, u16 0, u32 0x1A000010, u64 jackpot` | [EXE ✓ ramo D4 D8] |
| idem, dividido | S->C | 0x40 **tipo 14** a cada ganhador | `u8 14, str nick, u16 0, u32 0x1A000010, u64 parte` | [EXE ✓] |

### 2.4 Fim

| # | dir | id | layout | marca | nota |
|---|---|---|---|---|---|
| 1 | S->C | 0x8D | `u32 T` (tempo total em ms) | [EXE ✓] | ao acabar o tempo, só a quem ainda joga: "o próprio cliente termina" [REF] |
| 2 | C->S | **0x12C** | vazio | [EXE ✓ `CHoleInOneGameHandler::OnSend12c`] | o cliente terminou (também no `OnProcess`) |
| 2' | C->S | 0x131 | vazio | [EXE ✓] | chip-in: sair antes = fim "sem completar" |
| 3 | S->C | **0x1F2** | vazio | [EXE~ case sem leitura] | a ele, na hora |
| — | serv | — | quando **todos** (sem quem saiu) terminaram: posições, troféus, pontos, EXP (§3) | | |
| 4 | S->C | **0x1FA** | `u32 typeid do troféu, i32 id do item` | [EXE ✓ D4 D4] | só a quem ganhou troféu |
| 5 | S->C | 0x8C | vazio | [EXE ✓] | "acabou o tempo" (`sendTimeIsOver`) |
| 6 | S->C | **0x1F3** | placar (abaixo) | [EXE~ = `On18` do 645: `D4×9 D8×3 D4×4`] | a cada jogador |
| 7 | S->C | 0xC8 | `u64 pang, u64 0` | [EXE ✓ D8] | pang da conta depois de somar |
| 8 | S->C | 0xA7 | `u8 0` | [EXE ✓ D1…] | |
| 9 | S->C | 0xAA | `u16 0, u64 pang, u64 cookie` | [EXE ✓ D2 … DB(0x10)] | |
| 10 | S->C | 0x6B (mascote) | — | | só com mascote [REF]; dispensável |
| 11 | S->C | **0x24F** | `u32 0` | [EXE ✓ D4] | "novo do JP" (torneio, VS, GP, HIO) |
| 12 | S->C | EXP | o pacote de EXP normal (`addExp`) | | só se EXP > 0 |
| 13 | C->S | 0x06 [?] | estatísticas | | a referência não depende; o nosso `BeginGameEnd` fecha sozinho em 20 s |

**0x1F3** (placar):

| off | tipo | campo |
|---|---|---|
| 0 | u32 | 1 = terminou (FINISH), 2 = saiu/sem completar (END_GAME) — do jogador que recebe |
| 4 | u32 | n jogadores (sem quem saiu) |
| 8 | n × 64+8k | `i32 oid, u32 posição, i32 pontos totais, u32 HIOs, i32 tacadas totais, u32 pontos GZ ganhos, i32 EXP, u64 pang, u64 bônus, u64 jackpot, u32 troféu (0 = sem), u32 k, k × (u32 typeid, u32 qtd)` |

Saída no meio (jogador sai da sala): itens usados gravados, pang zerado, **0x40 tipo 2** `u8 2, str nick, u16 0` aos que ficam;
se só restam terminados, fecha o jogo. [REF]

## 3. Regras

### 3.1 Pontos por buraco (referência, ao acertar)

| bônus | condição | pontos | tipo no 0x1F5 |
|---|---|---|---|
| tacada especial | bit cobra/tomahawk/spike no `u32 shot` do 0x1B | +1 | 3 |
| primeira tacada | 1ª tentativa neste buraco (`tacadas do buraco == 1`) | +1 | 2 |
| sem comandos | `u32 special_shot` do 0x12 == 0 (sem setas de spin/curva) | +1 | 4 |
| sem acertar o Pangya | bit 2 do `acerto_pangya_flag` do 0x12 ("errou pangya" na referência; o cliente mostra "HIO without hitting Pangya +3") | +3 | 5 |
| o acerto | sempre | +1 | 1 |

- 0x1F5 lista os tipos na ordem acima; vai só se n > 1. O 0x1EF manda `total antes, pontos do buraco, total + pontos`.
- **Bug da referência**: depois do 0x1EF ela soma ao total **o nº de tacadas** do buraco (`score_hole = tacada_num`), não os
  pontos que acabou de anunciar. Usar **total += pontos do buraco** (o que o cliente mostra; a classificação é por maior
  total, e os pontos GZ = total × fator).
- Também conta como "acerto" o **give up** (`data.giveup > 0`, vindo do 0x31 sem acertar) [REF]; recomendo não dar pontos nem
  HIO nesse caso [?].
- Pang: `u32 pang`/`bônus` do último 0x1B (acumulados no cliente), com a taxa normal no fim. Booster é grátis no GZ
  (0x65 → **0xC7** `f32 velocidade, i32 oid`, só a ele) e cut-in desligado (0xE5 → **0x18D** `u8 0, u16 3`). [REF]

### 3.2 Posição, EXP, pontos GZ, troféus

- **Posição**: maior total primeiro; empate = mesma posição (1, 1, 2…). Quem saiu fica de fora. [REF]
- **EXP** (nível < 70; × taxa do servidor × item de EXP):
  - terminou (0x12C com o tempo cheio) → `floor(45 × (121 − posição) / 100)` (1º = 54);
  - sem completar (0x12C com ≥ 1 min sobrando, 0x131, ou saída) → `HIOs / 2`.
  - Chip-in: a referência zera sempre (tem 15 escrito, mas grava 0). Proposta: 0 no mínimo; 15 depois [?].
- **Pontos GZ** = total × **3,5** (tipo 11) ou × **5,0** (13); somados à conta (`grand_zodiac_pontos`). Chip-in: **não** ganha. [REF]
- **Troféu** (só 11/13), pelo nº de jogadores da partida [REF] / nomes [IFF Match.iff]:

| jogadores | troféus | 1º | 2º | 3º |
|---|---|---|---|---|
| < 20 | nenhum | — | — | — |
| 20–29 | 1 | 0x2D0A6400 | — | — |
| 30–49 | 2 | 0x2D0A6300 | 0x2D0A6400 | — |
| ≥ 50 | 3 | **0x2D0A6200** ホールインワンバトル1位 | **0x2D0A6300** 2位 | **0x2D0A6400** 3位 |

  Vai como item (0x1FA com o id) — no nosso servidor, `SpecialTrophies.AddAsync` (como o GP/!trofeu). Empate na posição dá o
  mesmo troféu.

### 3.3 Item que cai (só 11/13)

A cada acerto, 10 %: **0x1800002C Silent Nerve Stabilizer × 5** (a referência queria 50 % para **0x1800002D Safe Silent × 3**,
mas `rnd.Next(0,1)` sempre dá 0). Aviso 0x40 tipo 15 a todos; o item vai ao inventário no fim e aparece na lista do 0x1F3. [REF/IFF]

### 3.4 Tamanho do copo (0x1F9), pelos pontos GZ da conta

| pontos | < 300 | < 600 | < 1200 | < 1800 | < 4000 | < 5200 | < 7600 | < 10000 | ≥ 10000 |
|---|---|---|---|---|---|---|---|---|---|
| copo | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 |

O 0x1F9 leva `i32 copo, i32 copo, u32 pontos GZ`. [REF] (o 0x11 do cliente devolve um u32 que a referência chama de
"size_cup" e ignora).

### 3.5 Valores do 0x1EC

10 `f64` por partida, iguais para todos: `d = (rand % 1000)/100 + 1`; `v[i] = d + i × ((rand % 10)/10) + 1` (i = 0..9).
O cliente ecoa cada um (0x12D). Significado [?] (provável semente da sequência de tees/pins: por isso os mesmos para todos).

### 3.6 Golden beam e jackpot (só 11/13)

- Janela: decorrido de **T − 60 s** a **T − 30 s** (0x1F0 / 0x1F1). Quem acertar na janela entra na lista.
- No 0x1F1, se a lista não estiver vazia: `jackpot = nº de jogadores × 500 × k`, k ∈ {1..4}. A referência quer 50 %: **um
  ganhador sorteado leva tudo** (0x40 tipo 13 a todos), 50 %: **dividido igual** (tipo 14 a cada um); por bug ela sempre
  escolhe "um ganhador". O jackpot entra no pang no fim e aparece no 0x1F3.
- **Bugs da referência** a não copiar: `Interlocked.Exchange(ref state, 1)` ao conferir (liga o beam em todo acerto) e
  `setPlayerGoldenBeam` só adiciona quem já está na lista.

### 3.7 Chip-in Practice (14)

1 jogador, ticket 0x1A00017E (gasta 1 no início), 30 min, sem golden beam, sem troféu, sem item que cai, sem pontos GZ,
EXP 0 (§3.2), pang do jogo (a referência não aplica a taxa: `requestCalculePang` removido). Sai com 0x131 (= fim sem
completar) ou pelo tempo (0x8D T → 0x12C). Mesmos pacotes de fim (0x1F2, 0x8C, 0x1F3, 0xC8, 0xA7, 0xAA, 0x24F).
Contador de conquista: **0x6C40003F** "Chip-in Training Counter" (+1 ao começar). GZ: **0x6C40003C** "Hio Mode Counter"
(+1 ao começar) e **0x6C400004** (Game Complete) ao terminar com o tempo cheio. [REF/IFF CounterItem]

### 3.8 Onde gastar os pontos GZ (fase posterior)

`PointShop.iff` (12 itens, `i32 ativo, u32 typeid, u32 custo, u32 qtd, u32 flag`): ex. 0x1800002C 30 pts × 5, 0x1800002D 50 × 3,
0x1800002F 300 × 2, 0x1A000136 100 × 400… Pedidos 0x126..0x129 → 0x1E7..0x1EA (SPEC-jp983-sistemas, "Point Shop"); hoje
recusados no `JpTranslator.JpOnly`. [IFF; ligação pontos GZ ↔ Point Shop [?]]

## 4. Como mapear para o servidor

### 4.1 Domain (`src/Pangya.Domain/Rooms/`)

- **`RoomSettings`**: `public ZodiacKind Zodiac { get; set; }` (`None = 0, Intermediate = 11, Advanced = 13, ChipIn = 14`).
  `Hidden` inclui `ChipIn`. Em `Normalize()`, com `Zodiac != None`: `Mode = Tournament`, **`Course = 0x40`**, `Holes = 1`,
  `HoleType = 0`, `MaxPlayers = ChipIn ? 1 : até 100` (hoje o teto em massa é 30 — decidir), `GameTimeMs = ChipIn ?
  1 800 000 : config (420 000)`; `RepeatHole = 0`, `Practice = false`.
- `RoomManager.ValidCourse` recusa 0x40 (a lista de mapas do JP tira o 0x40 de propósito) → o `Normalize` precisa forçar
  depois do `ValidCourse`, ou o `PrepareStart` respeitar `Zodiac`. `FieldItems.For(0x40)` deve dar null.
- **`MassGame`**: `public virtual byte HoleOf(MassPlayer p)` (hoje o `HandleMass` faz cast para `TourneyGame`/`ApproachGame`
  e quebraria com uma 3ª classe) e um gancho para os dados da tacada:
  `public virtual void ShotData(MassPlayer p, ReadOnlySpan<byte> block) { }` (o `HandleMass` chama antes do `Shoot`).
- **`ShotResult`**: acrescentar `uint Display` (+27) e `uint ShotFlags` (+31) — o `DecodeResult` do KR já decifra 37 bytes,
  e os dois cabem.
- `MassGame.For(...)`: com `Zodiac != None` → `new GrandZodiacGame(room, zodiacOutput, sync)`.

**`GrandZodiacGame : MassGame`** (sem LINQ; dicionários por guid):

| membro | o que faz | pacotes (via `IZodiacOutput`) |
|---|---|---|
| ctor | sorteia os 10 `f64`; estado por jogador (`ZodiacPlayer`: tee x/z, tacadas do buraco, total, HIO, flags da última tacada, beam, drops, jackpot, terminado, saiu, pronto 0x137) | — |
| `Loaded(p)` (0x11) | 1ª vez: vento | `Seeds(p, wind, dir, seeds)` → 0x5B, 10 × 0x1EC, 0x201 |
| `HoleData` (0x1A, já existe) | guarda o tee (TeeX/TeeZ) do buraco | — |
| `Ready(p)` (0x137, novo) | marca; todos os humanos não saídos → `Later(T, TimeIsOver)`, `Later(T−60s, BeamOn)`, `Later(T−30s, BeamOff)` (só 11/13) | `FirstHole(p, tee)` → 0x8D 0, 0x53, 0x1EE, 0x1F4 1 |
| `ShotData(p, block)` (0x12) | guarda `block[0x10]` e `u32 block[0x11..]` | — |
| `Shoot(p, _)` | tacadas do buraco +1, total de tacadas +1 | — |
| `Result(p, r)` (0x1B) | guarda pang/bônus/flags; se acertou calcula os bônus (§3.1) e entra no beam se a janela estiver aberta | `RivalPos` (0x6E) e `ScoreTypes(p, lista)` → 0x1F5 se n > 1 |
| `ShotFinished(p)` (0x1C) | errou → volta ao tee; acertou → total, HIO, sorteio de item, vento novo | `EndShot` 0xCC; `HoleScore` 0x1EF (todos); `Drop` 0x40/15; `NextHole(p, wind, dir, segundos, tee)` → 0x1F4, 0x5B, 0x200, 0x1EE; `BackToTee` 0x1EE |
| `TimeIsOver()` | a quem joga | `TimeOver(p, T)` → 0x8D T; reserva: em +15 s força o fim de quem não mandou 0x12C |
| `Finish(p, complete)` (0x12C / 0x131) | `complete = (restante < 60 s)` para o 0x12C; 0x131 = false | `Ended(p)` → 0x1F2; todos → `Results()` e `GameOver(lista)` |
| `PlayerLeft(rp)` | marca saiu | `Left(p)` → 0x40/2; se só sobram terminados, fecha |
| `Results()` | posição, EXP, pontos GZ, troféus, jackpot (§3.2–3.6) | por jogador: 0x1FA, 0x8C, 0x1F3 (o resto no JP) |

### 4.2 KR (`src/Pangya.Protocol.KR645/Game/`)

- `GameHandler.Room.Start()`: com `Settings.Zodiac != None` criar o jogo pela fábrica `ctx.ZodiacOutput?.Invoke(room)` (sem
  fábrica = recusar o início). Gastar o ticket no chip-in antes de criar (falha → 0x47 KR/0x49 JP com erro).
- `HandleMass`: usar `g.HoleOf(me)`; chamar `g.ShotData(me, bloco)` no 0x12; `DecodeResult` preencher `Display/ShotFlags`.
- Recompensa: um caminho como o `BeginGameEnd` com EXP já calculado (a fórmula do GZ não é a do GB): pang + bônus + jackpot,
  EXP, itens (drops), pontos GZ, troféu especial; contadores 0x6C40003C/0x6C40003F/0x6C400004.
- Ids S->C do GZ (0x1EC..0x201, 0x24F) estão fora das faixas do `ServerId` (−1): **não** passar por `MassOutput`/tradutor.

### 4.3 JP983 (`src/Pangya.Protocol.JP983/Game/`)

- **`JpTranslator.Room.MakeRoom`**: aceitar **14** (só com `max = 1`, `buracos = 1`, tempo 1 800 000; logar mapa/modo/tacada);
  continuar recusando 11/13 (só servidor/GM). Passar ao KR um byte de "zodiac" no fim do 0x08 KR (como hoje `practice` e
  `repeat`), `head[10] = Tournament`.
- `RoomSettings` (0x4A) e `PracticeRoom` (RoomInfo 221): com `Zodiac != None` → tipo_show **11** (+88 e o u8 do 0x4A),
  `type_extend` (+196) = 11/13/14, buracos 1, mapa 0x40. `GamePlayers` (0x76): tipo 11; **antes** dele mandar **0x1F9** do
  jogador (copo e pontos GZ da conta) e, na sala de evento, 0x230/0x231/0x77. `GameInit` (0x52): tipo 11.
- `fromClient`: **0x137** → `kr.ZodiacReady()`; **0x12C** → `kr.ZodiacFinish(complete: true)`; **0x131** → `kr.ZodiacFinish(false)`
  (hoje é `null`); **0x12D** → `null` (ler e calar); 0x12F continua calado (ou repassar 0x1F7 depois); 0x65 (booster) e 0xE5
  (cut-in) no GZ: respostas 0xC7 / 0x18D `u8 0, u16 3` [REF].
- **`JpZodiacOutput : IZodiacOutput`** (novo; criado pela fábrica do contexto): manda os pacotes da tabela §4.1 em ids JP
  (`h.Connection.SendTranslated` em cada `rp.Session is GameHandler h`) e, no `GameOver`, por jogador: 0x1FA, 0x8C, 0x1F3,
  depois (após gravar) 0xC8, 0xA7 `u8 0`, 0xAA `u16 0, u64, u64`, 0x24F `u32 0`.
- Agendador/comando das salas de evento (fase 2) no JpSystems, como o `CountdownAsync` do GP: cria sala 11/13 com dono
  servidor, contagem 0x40/11, `StartAsServer()`.

### 4.4 Fases

1. **Mínimo jogável (chip-in, 1 jogador)**: `ZodiacKind`, `Normalize` (mapa 0x40), aceitar o 14 no 0x08, 0x4A/0x49/0x76/0x52 com
   tipo 11, 0x1F9 (copo 9, pontos 0), `GrandZodiacGame` com 0x11→seeds/0x201, 0x137→0x8D/0x53/0x1EE/0x1F4, tacada→0x1EE
   ou 0x1EF/0x1F4/0x5B/0x200/0x1EE, 0x1F5, tempo→0x8D T, 0x12C/0x131→0x1F2, fim 0x8C/0x1F3/0xC8/0xA7/0xAA/0x24F; recompensa
   só pang. Ticket: conferir e gastar (ou config para dispensar no teste).
2. **Evento 11/13**: salas pelo servidor (GM `!zodiac` e horário em config), contagem, várias pessoas (0x1EE a todos no 11),
   golden beam/jackpot, troféus, item que cai, EXP por posição, **pontos GZ na conta** (coluna nova + carregar no login →
   0x1F9 com o copo certo).
3. Point Shop com pontos GZ, contadores de conquista, 0x1F7 (fim da bola dos rivais).

## 5. Riscos e [?]

- **Ordem do carregamento**: a referência responde o 0x1A com 0x9E/0x5B e o 0x11 com 0x1EC×10/0x201; o nosso `TourneyGame`
  manda vento + 0x53 + 0x8D no 0x11. No GZ o **0x53 só pode sair depois de todos os 0x137** (senão o buraco pode começar antes
  da intro/sincronia). Conferir no 1º teste o que o cliente espera entre 0x201 e 0x137.
- **0x137 = `SendTimeOver`** no símbolo do 645: o cliente manda quando acaba alguma contagem dele (intro?) — a referência trata
  como "pronto para o 1º buraco". Sem o 0x1F4 o cliente pode ficar parado.
- **0x200**: decorrido ou restante? A referência manda decorrido em segundos. Se o relógio do cliente andar ao contrário,
  trocar.
- **0x1EE / 0x1F0 / 0x1F1 / 0x1F2 / 0x1F5**: o exe só mostra o case (a leitura é numa subfunção). Layouts da referência; o 0x1EE
  e o 0x1F3 batem com `On01`/`On18` do 645.
- **0x1A a cada buraco**: não sabemos se o cliente reenvia (tee novo). Se não reenviar, o 0x1EE mostra a posição antiga para os
  rivais (só visual).
- **0x08 do chip-in**: o diálogo que manda tipo 14 não foi isolado no exe (o `CLobbyMain::HandleMsg` mistura ramos). Logar o
  pacote inteiro; tempo ≠ 1 800 000 ou buracos ≠ 1 → recusar como a referência.
- **Ticket**: o cliente tira 1 sozinho? (o SSC tira). Mandar 0x216 absoluto.
- **0x1B do JP** tem 38 bytes sem a chave no fim; o `DecodeResult` usa `room.Key` e decifra 37 — conferir que `+27`/`+31`
  (display/shot) saem certos (o torneio JP já usa o mesmo caminho para estado/pang).
- **"miss pangya" (+3)** e give up como acerto: significado/intenção não confirmados.
- **Máximo de jogadores**: evento da referência = 100; o nosso `Normalize` limita modos em massa a 30, e o `RoomInfo`/lista do
  JP com 100 não foi testado.
- **Tempo do evento** (7 min na referência) e a janela do beam (60–30 s antes do fim): valores da referência, não do oficial.
- **KR 645**: o cliente 645 tem o mesmo modo e os mesmos ids; se um dia abrir para ele, o `IZodiacOutput` é quase o mesmo
  (ids +0 nessa faixa, segundo `ids_servidor.tsv`).
- **Fim sem 0x06**: o GZ fecha com 0x12C; se o cliente não mandar 0x06 depois, o `BeginGameEnd` fecha em 20 s (ok), mas o 0xC8
  do fim tem de levar o pang já somado.
