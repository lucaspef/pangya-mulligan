# JP 983: eventos e sorteios que na referência dependem do banco (Golden Time, Bot GM Event, drop por buraco, missões do approach, prêmio de login, gacha)

Marcas: **[EXE ✓]** conferido no `ProjectG.exe` 983 (endereços do exe) · **[IFF ✓]** conferido no `data/pangya_jp.iff` ·
**[REF]** comportamento da referência AGPL (só regra; código não copiado) · **[?]** não confirmado · **[proposta]** valor nosso
(a referência lê do banco, que não temos).

Scripts usados (pasta de scripts locais, não são do repo): `ev/pe.py` (leitor do PE + capstone), `ev/m3.py` (textos das missões),
`ev/it.py`, `ev/mb.py`, `ev/use.py`, `ev/names.py`, `ev/desc.py` (iff); `tools/disa.py`, `tools/xref.py`.

---

## 0. Resumo

| sistema | pacotes (JP) | o que o cliente tem | o que falta (tabela do banco) | esforço / valor |
|---|---|---|---|---|
| Golden Time | 0x1D3 tipos 11–18 (XML), 0x4B sub 6 (fogos) | textos fixos por tipo, parser de XML | eventos, horários, itens com peso → **config** | médio / baixo-médio |
| Bot GM Event | 0x1D3 tipo 20, 0x40 tipos 11/12 | nada específico (sala de torneio comum) | janelas de horário, prêmios → **config** | médio (depende do torneio JP) |
| Drop por buraco | **0xCC** `i32 oid, u8 n, [n>0: 128 × 16 B]` | `HoleCupDropItem.iff` = **catálogo visual** (typeid → sprite), não é tabela de chance | chances por curso → **config** | médio / médio |
| Missões do approach | **0x14F** `u8 nº, u8 caixas, u8 tipo, i32 c1, i32 c2, pstr nick` | **31 missões** com texto (fábrica @0x8556b0) | quais usar, caixas, nº mín. de jogadores → **config** | médio |
| Prêmio de login | só correio | nada | eventos → **config** + store | baixo |
| Gacha | 0xFB→0x1AD, 0xA1 `u8 2`, **0x9E** (fechou) → **0x102** | URL `https://www5.pangya.jp/on_client/gacha/login.aspx?ID=%s&KID=%s` | página inteira (site) + lineup → **config** | médio |

Bug que bloqueia os fogos do Golden Time (e hoje já afeta os itens SP do lounge no JP): ver §1.5.

---

## 1. Golden Time

### 1.1 Regras [REF `GoldenTimeSystem.cs`, `GameService.makeListOfPlayersToGoldenTime`, `Channel.getAllEligibleToGoldenTime`]
- Evento = tipo (`ONE_DAY` = uma data; `INTERVAL` = de data a data, todo dia; `FOREVER` = todo dia), lista de **horários de
  rodada** (hh:mm do dia), lista de **itens** `{typeid, qtd, dias (qtd_time), peso}`, `rate_of_players` (padrão 1).
- No início do dia (ao escolher o evento) sorteia **um item por rodada** por peso (peso ≤ 1 vira aleatório 1..99; sem repetir
  se houver itens suficientes).
- Laço global do game server: a cada tick checa a rodada.
  1. Entre 5 min antes e a hora da rodada (primeira mensagem): **0x1D3 tipo 11** (início do dia/rodada anunciada) e sai.
  2. Na hora: **tipo 12** (começou a rodada) com hora, item e qtd.
  3. Junta os elegíveis de todos os canais: jogador num lobby, **dentro de sala**; não conta Practice, GZ Practice nem Grand
     Prix Rookie/tutorial (aba 0 normal); **Lounge conta sempre**; outros tipos de sala só **jogando** (partida em curso).
  4. Ninguém: **tipo 18** (sem ganhadores). Se participantes > `max_user` do servidor: **tipo 13** (muita gente).
  5. Ganhadores: `n = (participantes / 100) × rate`, mínimo 1, no máximo todos. Sorteio sem reposição com peso
     `100 + 100 (jogando) + 100 (premium)`. (A ref. marca `is_premium = true` para todos — bug; usar o ticket premium.)
  6. **Tipo 14** com os nicks; **fogos** em quem está no lounge (§1.4); item **por carta** (correio) com texto
     `Golden Time - Round(<hora>): item[ <nome> ]`. Quem saiu antes da entrega perde o prêmio.
  7. Próxima rodada: nenhuma → **tipo 17** (`<BroadCastReservedNoticesIdx>53</…>`); no mesmo dia → **tipo 15** (hora atual,
     próxima hora, item e qtd da próxima); outro dia → **tipo 16** (hora atual, data da próxima).

### 1.2 0x1D3 [EXE ✓ caso @0x9d28c7 `D4 D4 DS`; despachante por tipo @0x8ac1f2]
`u32 n, n × {u32 tipo, pstr texto}`. Vai para todos os canais. Despacho no cliente (tabela @0x8ac8b0/@0x8ac8e4):

| tipo | parser chamado pelo cliente | tags lidas | ref. manda |
|---|---|---|---|
| 1 / 2 (cubo raro / bolsa de pang) | @0x8aa600 (TYPEID, BOX_TYPEID), @0x8ab4b0 | `TYPEID`, `BOX_TYPEID`, `QTY` | `<PARAMS>…</PARAMS>` |
| 11 | data @0x8ab3a0 + TYPEID/QTY | `EVENTSTARTTIME`, `TYPEID`, `QTY` | idem |
| 12 | idem | idem | idem |
| 13 | data | `EVENTSTARTTIME` | idem |
| 14 | data + TYPEID/QTY + **@0x8abe30 NICKNAME** (lista) | + `NICKNAME`×n | idem |
| 15 | data ×2 | `EVENTSTARTTIME`, `EVENTNEXTTIME` | + `TYPEID`, `QTY` (ignorados) |
| 16 | data ×3 [?: a 3ª leitura] | `EVENTSTARTTIME`, `EVENTNEXTTIME` | idem |
| 17 | só o texto fixo | — | `<PARAMS><BroadCastReservedNoticesIdx>53</…></PARAMS>` |
| 18 | data | `EVENTSTARTTIME` | idem |
| **20** | **texto puro** (@0x41ad70) | — (sem XML) | string simples |
| 21 | @0x8aba40 | `RunningTime` (min) | GZ |
| 10000 | @0x8ab680 `MESSAGE` + @0x8ab8f0 [?] | `MESSAGE` | — |
| 0, 3–10, 19 | ignorados pelo despachante | | |

- Data: o cliente faz `sscanf("%d-%d-%d %d:%d:%d.%d")` [EXE ✓ 0xd36188] → mandar `aaaa-mm-dd hh:mm:ss.mmm` (hora local do
  servidor = hora que o jogador vê) [REF].
- Tags existentes no exe [EXE ✓ 0xd3613c…0xd36214]: `PARAMS TYPEID BOX_TYPEID TIMES NEXTTIMES EVENTSTARTTIME
  EVENTNEXTTIME QTY MESSAGE RunningTime NICKNAME WinnerEventNotice`.
- `QTY` = dias se o item é por tempo, senão quantidade [REF].
- Exemplo tipo 14: `<PARAMS><EVENTSTARTTIME>2026-10-12 20:00:00.000</EVENTSTARTTIME><TYPEID>436207633</TYPEID><QTY>10</QTY><NICKNAME>a</NICKNAME><NICKNAME>b</NICKNAME></PARAMS>`
  (TYPEID em decimal).

### 1.3 Proposta de config [proposta; itens IFF ✓]
```jsonc
"GameJp": {
  "GoldenTime": {
    "Enabled": false,
    "Events": [
      { "Name": "Golden Time de fim de semana", "Type": "Interval", "Start": "2026-10-17", "End": "2026-12-31",
        "Weekdays": ["Sat", "Sun"],                     // nosso: filtro opcional por dia da semana
        "Rounds": ["20:00", "20:30", "21:00", "21:30"],
        "WinnersPer100": 1,                              // rate_of_players
        "Items": [                                       // TypeId, Qty, Days (0 = não é por tempo), Weight
          { "TypeId": 436207633, "Qty": 10, "Weight": 30 },   // 0x1A000011 Time Booster
          { "TypeId": 436207877, "Qty": 3,  "Weight": 20 },   // 0x1A000105 Comet Refill Pack
          { "TypeId": 436208136, "Qty": 1,  "Weight": 15 },   // 0x1A000208 Papel Box
          { "TypeId": 436208243, "Qty": 1,  "Weight": 15 },   // 0x1A000273 Memorial Coin (Normal)
          { "TypeId": 436207963, "Qty": 3,  "Weight": 10 },   // 0x1A00015B Spinning Cube
          { "TypeId": 436207744, "Qty": 1,  "Weight": 7 },    // 0x1A000080 Gacha Ticket
          { "TypeId": 436208242, "Qty": 1,  "Weight": 3 }     // 0x1A000272 Memorial Coin (Premium)
        ] }
    ]
  }
}
```
Tipos: `OneDay` (só `Start`), `Interval` (`Start`..`End`), `Forever`. Um evento por dia (o primeiro que valer).

### 1.4 Fogos na cabeça [REF `Channel.sendFireWorksWinnerGoldenTime` → `pacote04B(p, TC_ITEM_EFFECT_LOUNGE=6, 0, TE_TWILIGHT=3)`]
- **JP 0x4B** (= KR 0x49 +2) para a sala (lounge) inteira: `i32 0 (código), u8 6, u32 oid do ganhador, u32 3, 4 bytes`.
  O cliente lê o caso 6 como `D4 D4` + `DB(4)` e trata os 4 bytes como **f32** convertido para inteiro (`fld`; `ftol`
  @0x9c47c0) [EXE ✓]. A ref. manda `i32 1`; o nosso KR manda `f32` (valor do item SP). Mandar `f32 1.0`.
- Só para ganhadores que estão numa sala do tipo **lounge** (no lobby/jogo não há fogos).
- No nosso servidor é o mesmo pacote do item SP "brilho" (`GameHandler.Lounge.SpPacket(guid, SpShine=3, valor)`), então o
  Golden Time pode reaproveitar `InGameOutput.Broadcast(room, SpPacket(...))`.

### 1.5 Bug no tradutor (pré-requisito) [código atual]
`JpTranslator.Room.QuickEquip` (KR 0x49 → JP 0x4B) faz `k[0] switch { 3 club, 4 char, 1/2 bytes, _ => Mascot(d) }`:
o **sub 6** (efeito SP, 8 bytes) cai em `Mascot(d)`, que faz `k[..43]` → `ArgumentOutOfRangeException` [?: conferir no
log; o efeito SP do lounge no JP nunca chegaria]. Correção: `6 => w.Bytes(d)` (e `5 => Mascot`, `7 => vazio`).
Teste: KR `0x49 06 guid u32 3 f32 1.0` → JP `0x4B 00000000 06 guid 03000000 0000803F`.

---

## 2. Bot GM Event

### 2.1 Regras [REF `BotGMEventSystem.cs`, `GameService.makeBotGMEventRoom`, `Channel.makeBotGMEventRoom`, `RoomBotGMEvent.cs`]
- Banco: janelas `{início hh:mm:ss, fim hh:mm:ss, canal}` e prêmios `{typeid, qtd, dias, peso}`.
- Dentro de uma janela (uma vez por janela): sorteia **1 ou 2** prêmios por peso (sem repetir se houver mais itens que o nº);
  manda **0x1D3** com **2 × tipo 20** (texto simples): `Bot GM Event comecou, sala criada no canal "<canal>", o jogo comeca em
  2 minutos. Os premios sao <nome>(<qtd>|<n>day), …`; cria a sala no canal da janela (se ainda não houver uma).
- Sala: **torneio**, 18 buracos, curso aleatório (sorteio entre os cursos, sem 17 e 0x40), modo **Shuffle** (sorteio de
  buracos), **250** jogadores, **35 min**, nome `Bot GM Event`, `flag_gm = 1`, `state_flag 0x100`, troféu 0x2D0A3B00
  (**não existe** no `pangya_jp.iff` [IFF ✓: "???"] → usar 0); vento de canal rookie se o canal for rookie.
- Início: **2 min** depois de criada (ou na hora, se lotar: **0x40 tipo 12** "sala cheia" `u8 12, u16 0, u16 0, u32 10`),
  depois contagem **0x40 tipo 11** `u8 11, pstr "", pstr "", u32 segundos` (30 s em 30 s, depois 10 s, depois 1 s), começa
  com quem estiver (1 jogador = + bot visual). Ninguém → destrói.
- Fim: a cada jogador da sala manda **todos** os prêmios sorteados numa carta `Bot GM Event(<data>): item[ <1º item> ]`.
  Sala destruída 2 min depois. Nada de drop de sala (`isDropRoom = false`).
- Nós já temos o mesmo esqueleto no `JpZodiacEvent` (abrir sala do servidor, esperar 2 min, contagem 0x40 tipo 11):
  `JpBotGmEvent` = cópia com `RoomSettings { Mode = Tournament, Holes = 18, Shuffle, MaxPlayers = 250, TimeLimit 35 min }`.

### 2.2 Proposta de config [proposta; itens IFF ✓]
```jsonc
"BotGmEvent": {
  "Enabled": false,
  "Windows": [ { "Start": "21:00", "End": "21:10", "Channel": 0 } ],    // índice do canal do GameJp
  "Prizes": { "Min": 1, "Max": 2, "Items": [
    { "TypeId": 436208243, "Qty": 1,  "Weight": 30 },   // 0x1A000273 Memorial Coin (Normal)
    { "TypeId": 436208136, "Qty": 1,  "Weight": 25 },   // 0x1A000208 Papel Box
    { "TypeId": 436207964, "Qty": 5,  "Weight": 20 },   // 0x1A00015C Key of Fortune
    { "TypeId": 436207744, "Qty": 1,  "Weight": 15 },   // 0x1A000080 Gacha Ticket
    { "TypeId": 436208242, "Qty": 1,  "Weight": 5 },    // 0x1A000272 Memorial Coin (Premium)
    { "TypeId": 268435474, "Qty": 1,  "Days": 7, "Weight": 5 }  // 0x10000012 Lucky Air Knight Set (7 dias)
  ] }
}
```
GM: `!botgm` abre agora (como `!zodiac`). Texto do aviso pela config (padrão em inglês; o tipo 20 mostra a string como veio,
em Shift-JIS/ASCII).

---

## 3. Drop por buraco ("Hole Cup")

### 3.1 HoleCupDropItem.iff: **não é a tabela de drop** [IFF ✓ / EXE ✓]
- Registro 0x2C: `u32 typeid` + `char[40] sprite` (87 linhas). Lido por `@0xa3eb22` → `@0xa2b8d0`, que põe num mapa por
  typeid em `iffmgr+0x484` [EXE ✓]. **Não tem chance, curso nem quantidade**: é o catálogo do que pode cair no copo e o
  efeito a mostrar. O consumidor do mapa não foi achado [?]; o efeito do fim do buraco (@0x683e00…0x6840c2) ainda tem um
  `switch` fixo por typeid (0x1A000046..0x1A000134: `aeboat-0x.spr`, `coin_medal0x.spr`, `magic_stone_show.spr`,
  `lightstone.spr`, `Event_Flower01.spr`; senão `xmas_prize.spr`) e uma linha de chat "<item> 획득" [EXE ✓].
- Conteúdo (todos `Item.iff`, ItemType 0, quase todos de evento sazonal; 54 deles estão na `NonVisibleItemTable`, ou seja,
  a troca era fora do cliente). Úteis hoje: **Gold Medal 0x1A0000AA** e **Copper Medal 0x1A0000AC** (5 receitas cada na Caixa
  Mágica: Karen EX/RX, Lolo Mascot, Schwarz Raptor Ring, A.H.M.I.E TYPE-S-00, sets Air Knight of Round…) [IFF ✓ `ev/mb.py`].
  Sazonais: Abbots 0x1A000044–47, Candy Box 0x1A000049–4C, 4 deuses 0x1A000143–146, cristais 0x1A000150–153, Xmas
  X/M/A/S/Star 0x1A0002E7–EB, números Kooh 1..25 0x1A0001D9–F1, ingredientes 0x1A000255–259, Fukuo 0x1A000393–396 etc.
- A descrição do Rainbow Magic Hat confirma o nome: "You get 1 additional **Hall Cup item** while playing" [IFF ✓ Desc].

### 3.2 Pacote S->C **0xCC** [EXE ✓ `CGolfTask` caso @0x6a39c6; `rx_cmp`: igual ao 0xCC do JP de comparação]
```
i32 oid do jogador, u8 n
se n > 0: 0x800 bytes = 128 × DropItem(16)       (se n == 0 o cliente não lê mais nada)
DropItem: u32 typeid, u8 curso (&0x7F), u8 buraco, i16 qtd, u64 tipo     [REF game_type.DropItem]
tipo: 1 NORMAL_QNTD, 2 QNTD_MULTIPLE_500 (pang pouch: qtd × 500), 3 COIN_EDGE_GREEN, 4 COIN_GROUND, 5 CUBE
```
- O cliente (@0x635dd0) acha o jogador pelo oid (`+0x4b4`) e **acrescenta** os n primeiros registros à lista dele
  (`+0x58a`, contador `+0x589`), até 128 no total [EXE ✓]. Logo: **mandar só os drops novos** de cada buraco.
- KR 645: o mesmo pacote é **0xCA** com registro de **12 B** (`u32 tid, u8[2], u16 qtd, i32 tipo`, 0x600)
  (`SPEC-resultado-fim-de-jogo.md` §2.5). A faixa do tradutor já leva KR 0xCA → JP 0xCC (+2); falta a conversão
  `toClient[0xCA]`: copia `i32, u8 n` e, se n > 0, cada registro `8 B + i32 tipo` → `8 B + u64 tipo`, 128 registros.
- Quando [REF `TourneyBase.requestReplySyncShotData`/`drawDropItem`, `VersusBase` igual]: logo após a tacada que **entrou no
  buraco** (`acerto_hole`), antes da resposta do sync da tacada; só para o próprio jogador (`session_send`).
- Crédito [REF `requestSaveDrop`]: no **fim da partida** (stroke/torneio/GP/SSC/…): soma por typeid (pang pouch ×500),
  `addItem`, **0x216** com os itens; entram também na lista de itens ganhos da tela de resultado (JP 0xFA = KR 0xF8, já feita).
  Quem sai no meio não recebe [REF: só no fim].

### 3.3 O que a ref. sorteia por buraco [REF `GameBase.requestInitDrop`, `DropSystem.cs`]
| fonte | regra | observação |
|---|---|---|
| pang do artefato | só no **último buraco de jogo de 18**: Wicked Broomstick 0x1A0001B4 = 1 bolsa (×500), Teorite Ore 0x1A0001B6 = 2, Rednose Wizberry 0x1A0001B8 = 3, Magani Flower 0x1A0001BA = 6, Roger K 0x1A0001BC = 1..6 (< 25 jogadores) ou 1..1002 | bate com o texto do `ArtifactManaInfo.iff`: "+500/+1000/+1500/+3000 (18H)" [IFF ✓]. ≥ 30 bolsas → **0x40 tipo 10** (jackpot) `u8 10, pstr nick, u16 0, i32 pang` à partida |
| tabela do curso (banco) | por curso: `tipo 0` todo buraco (chance em ‰ de 1000), `tipo 1` a cada N buracos (`seq % N == 0`), `tipo 2` só no último buraco (chance por 3/6/9/18 buracos); qtd ×2 com "char motion", +1 com Rainbow Magic Hat 0x1A0001BE; chance × taxa de drop do item ativo (>100 %) e ×1,2 com asas de anjo (só se o jogador não tem ícone de "desistente") | **bug da ref.**: `dir.v_drop.AddRange(dir.v_drop)` — o drop do curso nunca entra |
| mana | sorteia entre os 18 itens `ItemType == 4` [IFF ✓: 0x1A0001A5, A7, A9, AB, AD, AF, B1, B3, B5, B7, B9, BB, BD, BF, C1, C3, 0x1A0001F9, FB = mana de cada artefato, `ArtifactManaInfo` artefato → artefato+1], peso 200 cada + "nada" = soma/taxa | com taxa 1 dá **50 % por buraco** (alto demais) |
| ticket do GP 0x1A000264 | só no **último buraco**, fora de sala GP, se tem < **50**: jogo de 9/18 buracos = 100 % (18H dá 2, ou 1 se tem 49; 9H dá 1); jogos menores 1/3 de chance de 1 | o Desc do item confirma ("ao terminar o modo normal; teto 50") [IFF ✓] |
| ticket do Special Shuffle 0x1A0000F7 | todo buraco: 200 / (200 + 2·200/taxa) ≈ **33 %** com taxa 1; ×2 char motion, +1 chapéu; conta a conquista 0x6C400053 | alto demais |

### 3.4 Proposta de config [proposta; itens IFF ✓]
```jsonc
"HoleDrops": {
  "Enabled": true,
  "Modes": ["Stroke", "Tournament", "Team", "GrandPrix", "SpecialShuffle"],   // não em prática/approach/GZ
  "ManaPerMille": 15,                // 1,5 % por buraco, 1 mana sorteada entre as 18
  "SpecialShufflePerMille": 20,      // 2 % por buraco
  "GrandPrixTicket": { "Max": 50, "Holes18": 2, "Holes9": 1, "ShortChancePerMille": 333 },
  "ArtifactPang": true,              // tabela fixa da ref. (= ArtifactManaInfo)
  "Items": [                          // vale para todo curso; "Course": n restringe
    { "TypeId": 436207788, "Kind": "Every",    "PerMille": 30, "Qty": 1 },            // 0x1A0000AC Copper Medal
    { "TypeId": 436207786, "Kind": "Every",    "PerMille": 8,  "Qty": 1 },            // 0x1A0000AA Gold Medal
    { "TypeId": 436207786, "Kind": "LastHole", "PerMille": [50, 100, 200, 400], "Qty": 1 }  // 3/6/9/18 buracos
  ],
  "Seasonal": [                       // desligados; ligar com datas
    { "Name": "Natal", "Start": "12-15", "End": "12-31", "Items": [
      { "TypeId": 436208359, "Kind": "Every", "PerMille": 40 },   // 0x1A0002E7 X
      { "TypeId": 436208360, "Kind": "Every", "PerMille": 40 },   // 0x1A0002E8 M
      { "TypeId": 436208361, "Kind": "Every", "PerMille": 40 },   // 0x1A0002E9 A
      { "TypeId": 436208362, "Kind": "Every", "PerMille": 40 },   // 0x1A0002EA S
      { "TypeId": 436208363, "Kind": "Every", "PerMille": 10 } ] }, // 0x1A0002EB Star
    { "Name": "Quatro deuses", "Items": [ { "TypeId": 436207939, "Kind": "Every", "PerMille": 10 } ] }  // 0x1A000143… (repetir 144–146)
  ]
}
```
Coleções sazonais sem receita no cliente só fazem sentido com uma troca
nossa (Caixa Mágica não tem as receitas: §3.1); por isso ficam desligadas.

---

## 4. Missões do approach

### 4.1 Pacote **0x14F** [EXE ✓ `CGolfTask` caso @0x6a3b74]
`u8 nº` — se **0**, o cliente não lê mais nada (sem missão). Senão: `u8 caixas, u8 tipo, i32 c1, i32 c2, pstr nick`
(texto ≤ 0x80). Fluxo: `@0x8556b0(nº, c1, c2, nick)` cria o objeto da missão (`switch` 1..**31**, `cmp 0x1e`) e monta o
texto; depois grava nº (@0x65fdd0), caixas (@0x65fde0), tipo (@0x65fdf0) e texto (@0x65fda0) [EXE ✓].
Hoje mandamos `0x147 u8 0` no KR → o tradutor troca por 13 zeros no JP (ok: o cliente só lê o 1º byte).
Na ref. vai junto com o início do buraco do approach (depois do 0x53 de cada um e das taxas), um por buraco, para todos.
`tipo` [REF `eMISSION_TYPE`]: 1 co-op (todos que cumprem), 2 free-for-all (individual).

### 4.2 As 31 missões do cliente [EXE ✓ `ev/m3.py`; textos-chave em coreano no exe, traduzidos aqui]
c1/c2 = como o cliente usa no texto [EXE ✓ nos casos abertos]; "ref." = a ref. sabe julgar [REF `Approach.mission_win`].

| nº | texto (tradução) | c1 / c2 | ref. |
|---|---|---|---|
| 1 | distância do 1º colocado é **par/ímpar** | c1: 0 par, ≠0 ímpar | sim (todos ganham) |
| 2 | soma das distâncias do 1º ao 3º é par/ímpar | c1 par/ímpar | não |
| 3 | quem fizer OUT (chip-in) | — | sim |
| 4 | todos tacam antes de 20 s | — | não |
| 5 | 1º ao 3º são todos **mulher/homem** | c1 sexo | não |
| 6 | o 1º colocado jogou com o mascote **Cocoa/Puff/Billy/Lemmy** | c1 1..4 → 0x40000002, 0x40000001, 0x40000003, 0x40000000 [IFF ✓] | sim |
| 7 | quem ficar em **%d**º | c1 = posição (1..n) | sim |
| 8 | distância de todos é par/ímpar | c1 | não |
| 9 | se o 1º for mulher/homem, todos do mesmo sexo | c1 | não |
| 10 | todos com o personagem **%s** e < 10 y | c1 = typeid (servidor), **c2 = índice do personagem** (o cliente usa c2 no texto @0x854620) | sim |
| 11 | mais jogadores com distância par que ímpar (ou o inverso) | c1 | sim |
| 12 | maioria fez OUT | — | sim |
| 13 | quem ficar a ≤ 1 y é mulher/homem | c1 | não |
| 14 | distância < **N y** (1..5) | c1 = 1..5 | sim (caixas 3 se ≤1, 2 se ≤3) |
| 15 | o **%d**º tem distância par/ímpar | c1 posição, c2 par/ímpar | sim |
| 16 | 4+ jogadores fizeram OUT | — | sim |
| 17 | soma das distâncias de todos < **%d y** | **c2** (ref.: 150 se < 15 jogadores, senão 250) [?: o texto usa %d de c1 ou c2] | sim |
| 18 | alguém parou em exatamente **%d.%d** y | c1 inteiro (1..N), c2 décimo (0..8) | sim |
| 19 | soma das distâncias dos homens/mulheres é par/ímpar | c1 sexo, c2 paridade | não |
| 20 | quem ficar **mais longe** | — | sim |
| 21 | tempo restante de todos < **%d** s | c1 | não |
| 22 | tempo restante do 1º < 10 s | — | sim |
| 23 | < 20 y usando o caddie **Papel/Pippin/Titan Boo/Dolfini/Lolo/Quma/Tiki/Cadie** | c1 0..7 → 0x1C000000..07 ou 0x1C000010..16 [IFF ✓] | sim |
| 24 | quem tacar **mais rápido** | — | sim |
| 25 | distância par/ímpar (cada um) | c1 | sim |
| 26 | quem tacar **por último** | — | sim |
| 27 | média de distância por sexo: o lado menor ganha | — | não |
| 28 | média de tempo restante por sexo: o lado maior ganha | — | não |
| 29 | se **[nick]** fizer chip-in | c1 = uid, nick | sim |
| 30 | chip-in usando **Cobra/Spike/Tomahawk** (power shot) | c1 0..2 | não |
| 31 | quem fizer chip-in **primeiro** | — | sim |

Unidade da distância: a ref. compara `distance` com yardas inteiras [?: conferir a unidade do nosso `ApproachGame`].

### 4.3 Proposta de tabela [proposta]
Sortear com 50 % de chance por buraco (ref.: `(rnd%1000)/10 <= 50` ≈ 51 %), entre as linhas ativas cujo `MinPlayers` <
jogadores (ref.: `players < n`).
```jsonc
"ApproachMissions": { "ChancePercent": 50, "Missions": [
  { "No": 1,  "Boxes": 1, "Type": 1, "MinPlayers": 2, "C1Max": 2 },
  { "No": 3,  "Boxes": 2, "Type": 2, "MinPlayers": 1 },
  { "No": 6,  "Boxes": 1, "Type": 1, "MinPlayers": 2, "C1Max": 4 },
  { "No": 7,  "Boxes": 1, "Type": 2, "MinPlayers": 3 },                // c1 = 1..n
  { "No": 10, "Boxes": 1, "Type": 2, "MinPlayers": 2 },                // personagem sorteado do Character.iff
  { "No": 11, "Boxes": 1, "Type": 1, "MinPlayers": 4, "C1Max": 2 },
  { "No": 12, "Boxes": 2, "Type": 1, "MinPlayers": 4 },
  { "No": 14, "Boxes": 1, "Type": 2, "MinPlayers": 1, "C1Max": 5 },    // caixas 3/2/1 pela distância
  { "No": 15, "Boxes": 1, "Type": 1, "MinPlayers": 3, "C1Max": 2 },
  { "No": 16, "Boxes": 2, "Type": 1, "MinPlayers": 6 },
  { "No": 17, "Boxes": 1, "Type": 1, "MinPlayers": 4 },
  { "No": 18, "Boxes": 2, "Type": 1, "MinPlayers": 4, "C1Max": 5 },
  { "No": 20, "Boxes": 1, "Type": 2, "MinPlayers": 3 },
  { "No": 22, "Boxes": 1, "Type": 1, "MinPlayers": 2 },
  { "No": 23, "Boxes": 1, "Type": 2, "MinPlayers": 2, "C1Max": 8 },
  { "No": 24, "Boxes": 1, "Type": 2, "MinPlayers": 3 },
  { "No": 25, "Boxes": 1, "Type": 2, "MinPlayers": 1, "C1Max": 2 },
  { "No": 26, "Boxes": 1, "Type": 2, "MinPlayers": 3 },
  { "No": 29, "Boxes": 2, "Type": 1, "MinPlayers": 4 },
  { "No": 31, "Boxes": 2, "Type": 2, "MinPlayers": 3 }
] }
```
Ficam de fora (o cliente mostra, mas não há regra de julgamento nem na ref.): 2, 4, 5, 8, 9, 13, 19, 21, 27, 28, 30 —
fáceis de acrescentar depois (regras óbvias pelo texto). As caixas ganhas somam às caixas de posição (`rank_box`) e viram
prêmios no fim (Treasure Hunter `drawApproachBox`, ref.) [REF].

---

## 5. Prêmio de login por evento [REF `LoginRewardSystem.cs`]
- Eventos: `{id, nome, dias_para_prêmio, tipo N_TIME|FOREVER, n_vezes, item {typeid, qtd, dias}, fim}`; por jogador
  `{count_days, count_seq, is_clear, update_date}`.
- No login (após entrar no game server): para cada evento ativo (vencido → encerra): 1º login cria o registro com
  `count_days = 1`; depois, só conta se é **dia novo** (`hoje > update_date`). Ao chegar em `dias_para_prêmio`:
  `count_seq++`; N_TIME: zera os dias até `n_vezes`, então `is_clear`; FOREVER: zera sempre. Prêmio **por carta**:
  `Login Reward System - "<nome>": item[ <nome do item> ]`. Sem pacote próprio (o aviso de carta nova é o do correio).
- Nós: `AttendanceService` (presença diária do JP) já roda no login; o evento cabe como outro serviço pequeno com store
  próprio (os contadores são por evento).

Proposta [proposta; IFF ✓]:
```jsonc
"LoginRewards": [
  { "Id": 1, "Name": "Boas-vindas", "Days": 7,  "Type": "NTimes", "Times": 1, "TypeId": 436208242, "Qty": 1 },   // Memorial Coin (Premium)
  { "Id": 2, "Name": "Fiel",        "Days": 30, "Type": "Forever",             "TypeId": 436207745, "Qty": 1 },   // 0x1A000081 Gacha Ticket (+5)
  { "Id": 3, "Name": "Semana de evento", "Days": 1, "Type": "NTimes", "Times": 7, "End": "2026-11-01",
    "TypeId": 436207633, "Qty": 5 }                                                                              // Time Booster
]
```
O `Id` é a chave dos contadores (não reaproveitar).

---

## 6. Gacha

### 6.1 Cliente [EXE ✓]
- Botão do gacha (`CTaskMain` @0x658e70): ajusta o navegador para **800×600** (@0x4949a0) e chama
  `RequestOpen983("https://www5.pangya.jp/on_client/gacha/login.aspx?ID=%s&KID=%s", 1, lugar **2**)` (@0x8a9fa0) →
  **C->S 0xFB** → **S->C 0x1AD** `u32 1, pstr chave` → `sprintf(url, login, chave)` → navegador + **C->S 0xA1 `u8 2`**.
  É o mesmo caminho do club (`SPEC-jp983-guilda.md` §1.1; nosso `JpSystems.Guild` já responde 0xFB/0x1AD e guarda o 0xA1).
- Ao fechar (`SendWebPageClose983` @0x868320, lugar 2): **C->S 0x9E** (vazio) [EXE ✓ `browser.cpp`].
- **S->C 0x102** [EXE ✓ caso @0x9d1f07 `D4 D4 D8 D8`]: `u32 tickets, u32 parciais, u64 pang, u64 cookie` → grava em
  `doc+0x1ec`, `doc+0x1f0`, pang `+0x91e`, cookie `+0x3858` e atualiza a tela.
- Outras URLs de gacha no exe (`rede_983.tsv`): `http://www.pangya.jp/shop/ika/gacha_index.aspx` e
  `…/shop/event/shizuku/gacha_index.aspx` (dados, sem chamador encontrado: páginas antigas da loja web).

### 6.2 Referência [REF `Channel.requestUpdateGachaCoupon`, `CmdCouponGacha`, `ItemManager`]
- 0x9E → relê do banco a quantidade de **0x1A000080** (Gacha Ticket) e **0x1A000083** (Gacha Ticket Partial), acerta o
  inventário em memória e manda 0x102. No login manda 0x102 (`cLoginTask`). Ao receber 0x1A000080 (compra/presente)
  soma e manda 0x102. O gacha em si era o site oficial (não há página na ref.).

### 6.3 Itens [IFF ✓ + Desc.iff]
| typeid | nome | Desc (resumo) |
|---|---|---|
| 0x1A000080 | Gacha Ticket | joga no Gacha |
| 0x1A000081 | Gacha Ticket (+5) | joga e recebe 4 itens + 1 ticket de graça |
| 0x1A000082 | Gacha Ticket (+11) | 10 itens + 1 ticket de graça |
| 0x1A0003B3 | Gacha Ticket (+11) | "Gacha and Pouch", uma jogada por ticket |
| 0x1A000083 | Gacha Ticket Partial | "Gacha and Pouch"; **uma jogada a cada 10** |
| 0x1A00031A / 0x1A00034E | Gacha Exchange Coin (P)/(B) | 7 / 4 peças viram um ticket (sem receita na CadieMagicBox [IFF ✓]) |
| 0x1A000208 | Papel Box | prêmio da presença a cada 10 dias (abre: item de "Gacha and Pouch" + 30 Fortune Keys) |

### 6.4 Proposta de site no `src/Pangya.Web` [proposta]
**Arquivo** `src/Pangya.Web/GachaSite.cs` (molde: `ClubSite.cs`). Base `/on_client/gacha`, cookie `gacha`.
- `GET /on_client/gacha/login.aspx?ID=&KID=` → `Sessions.ValidateClubWebAsync(KID)` (renomear para `ValidateWebKeyAsync`:
  a chave do 0x1AD é a mesma para club e gacha) e confere `ID` = login → cookie → `302 /on_client/gacha/home`.
- `GET /home` → tickets/parciais (do jogador **online** no game-jp), lineup com chances e botões "1 jogada", "10 jogadas"
  (10 tickets) e "usar 10 parciais".
- `POST /spin` `kind=ticket|partial|plus5|plus11, n` → resultado (lista de itens ganhos) + saldo.
- `GET /history` (opcional) → últimas 50 jogadas (do log/audit).
- HTML simples, sem JS moderno (IE embutido), Shift-JIS ou ASCII [?: como no club].

**Onde o sorteio roda**: o jogador está online no game-jp com o `Player` em memória; o painel admin recusa mexer em
inventário de quem está online ("a sessão aberta sobrescreveria"). Então o site **não grava inventário**: chama uma ponte
no mundo do JP (`GameWorld.Find(accountId)` → `IGameSession`), que executa na fila da sessão:
1. `Domain/Shop/GachaService.SpinAsync(Player, kind, n)`: confere/gasta tickets no `Player` (ShopService, como a Caixa
   Mágica), sorteia por peso (sem LINQ), entrega com `ShopService.GiveAsync` (empilha), grava, registra no Audit.
2. Guarda os itens alterados numa lista "a mandar quando fechar".
3. No **0x9E**: manda **0x216** com os itens alterados (tickets e prêmios) e **0x102** com as contagens reais
   (hoje o `JpTranslator.JpOnly` responde `0, 0`). Também mandar 0x102 no fim do login JP.
- Entrega alternativa (config `Delivery: "Mail"`): prêmio por carta (`MailService.SendSystemAsync`), útil para itens que não
  empilham ou com inventário cheio. Padrão: inventário, e carta se falhar.
- Precisa: `Program.cs` passar o `World` do **game-jp** ao `WebServer` (hoje só passa o do KR: `game?.World`);
  `IGameSession` ganhar `Task<T> RunAsync<T>(Func<Player, Task<T>>)` (ou um `GachaBridge` no `JpGameContext`).
- rugburn: regra `https://www5.pangya.jp/on_client/gacha` → `http://<host>:30080/on_client/gacha` (como a do club;
  não editar arquivos da pasta do cliente — só documentar).

Lineup padrão [proposta; IFF ✓] (`"Gacha": { "Items": [...] }`, peso; qtd fixa ou faixa):
| tier | itens | peso |
|---|---|---|
| comum (70) | 0x18000000 Spin Mastery ×5, 0x18000001 Curve Mastery ×5, 0x18000004 Strength Boost ×5, 0x18000005 Miracle Sign ×5, 0x1A000011 Time Booster ×5, 0x18000025 Power Milk ×5, 0x1A00000F Angel Candy ×3 | 10 cada |
| médio (24) | 0x1A00015B Spinning Cube ×1, 0x1A00015C Key of Fortune ×3, 0x1A000105 Comet Refill Pack ×1, 0x1A000208 Papel Box ×1, 0x1A000273 Memorial Coin (Normal) ×1, 0x14000020 Rainbow Comet ×30 | 4 cada |
| raro (6) | 0x1A000272 Memorial Coin (Premium) ×1 (3), 0x40000004 El Monito (mascote, 30 dias) (2), 0x10000012 Lucky Air Knight Set (1) | 3/2/1 |
Regra de "+5 / +11": `plus5` = 5 prêmios por um 0x1A000081 (4 + 1), `plus11` = 11 por um 0x1A000082 [proposta, pela Desc].
Parcial: 10 × 0x1A000083 = 1 jogada [IFF ✓ Desc]. Sem garantia de raro ("pity") por padrão.

---

## 7. Plano de implementação

Ordem sugerida: (1) correção do 0x4B sub 6; (2) drop por buraco (serve KR e JP); (3) cupons/gacha; (4) Golden Time;
(5) missões do approach; (6) prêmio de login; (7) Bot GM Event.

| # | arquivos | o que fazer | testes (`tests/Pangya.Tests`) |
|---|---|---|---|
| 1 | `Protocol.JP983/Game/JpTranslator.Room.cs` | `QuickEquip`: `6 => w.Bytes(d)`, `5 => Mascot`, `7 => w` | `Jp983GameTests`: KR 0x49 sub 6 → JP 0x4B com `i32 0` e 8 B iguais |
| 2a | `Domain/Rooms/HoleDrops.cs` (novo), `Core/Config/PangyaConfig.cs` (`HoleDropsConfig` em `GameConfig`) | sorteio puro: entrada = curso, buraco, seq, nº de buracos, artefato, taxa do item, asas, nº de jogadores, tickets que tem; saída = lista `DropItem`. Corrigir o bug da ref. (drop do curso). Random injetável | `HoleDropsTests`: semente fixa; GP ticket 100 % no último de 18 e teto 50; Rainbow Hat +1; pang do artefato só no 18º; mana só ItemType 4; ‰ estatístico em 100 000 sorteios |
| 2b | `Protocol.KR645/Game/InGame.cs` / `MassOutput.cs` (fim do buraco), `GameHandler.GameEnd.cs` | ao entrar no buraco: KR **0xCA** `u32 guid, u8 n, 128×12 B` só ao dono; guardar na lista do jogador; no fim: creditar (somar, pang ×500), 0xA5/0x216, incluir no 0xF8/0xCC de itens ganhos | `GameEndTests`: drop aparece no 0xF8 e no inventário; sai no meio = nada |
| 2c | `Protocol.JP983/Game/JpTranslator.Room.cs` | `toClient[0xCA]`: 12 B → 16 B (`i32 tipo` → `u64`), 0x600 → 0x800, n=0 → 5 bytes | `Jp983GameTests`: tamanho 5 + 0x800, typeid/qtd/tipo nos offsets |
| 3a | `Protocol.JP983/Game/JpTranslator.JpOnly.cs`, `JpGameHandler` (login) | 0x9E e login → 0x102 com contagem real de 0x1A000080/0x1A000083 + pang/cookie | `Jp983SystemsTests`: 0x9E → 0x102 `3, 12, pang, cookie` |
| 3b | `Domain/Shop/GachaService.cs` (novo), `Core/Config` (`GachaConfig` em `GameConfig`), `Domain/Game/GameWorld.cs` (`IGameSession.RunAsync`), `Web/GachaSite.cs`, `Web/WebServer.cs`, `Server/Program.cs` (World do game-jp), `Domain/Auth/Sessions.cs` (nome genérico) | §6.4 | `GachaTests` (serviço: gasta 1 ticket / 10 parciais, sem ticket = erro, pesos); `Jp983GachaTests` (site em memória: login.aspx com KID válido/ inválido, spin, 0x9E manda 0x216 + 0x102) |
| 4 | `Protocol.JP983/Game/JpGoldenTime.cs` (novo, molde `JpZodiacEvent`), `Domain/Game/GoldenTime.cs` (regras puras: rodadas, sorteio de item e de ganhadores), `Core/Config` (`GoldenTimeConfig`), `Game/JpGameServer.cs` (`ScheduleAsync`) | relógio injetável; 0x1D3 tipos 11–18 (XML §1.2, TYPEID decimal, data `aaaa-mm-dd hh:mm:ss.mmm`), carta, fogos (0x49 sub 6 no lounge); GM `!goldentime now` | `GoldenTimeTests`: elegíveis (lounge sim, sala parada não, prática não), nº de ganhadores (1 por 100 × rate, mín. 1), pesos; `Jp983GoldenTimeTests`: XML de cada tipo e ordem dos avisos com relógio falso |
| 5 | `Domain/Rooms/ApproachMissions.cs` (novo: sorteio + julgamento das 20 missões), `Protocol.KR645/Game/MassOutput.cs` (`HoleStart`: KR 0x147 com a missão), `Protocol.JP983/Game/JpTranslator.Room.cs` (tirar o `Zeros(13)`; passar `u8 nº, u8 caixas, u8 tipo, i32, i32, pstr`), config `ApproachMissions` | conferir no KR 645 o layout do 0x147 (texto "0x147 types 1..27", `SPEC-modes.md`) antes de mandar ao KR; se diferir, mandar a missão só para o JP | `ApproachMissionTests`: cada nº com um cenário que ganha e um que perde; c1/c2 dentro das faixas; 0x14F JP byte a byte |
| 6 | `Domain/Players/LoginRewards.cs` (novo, + `ILoginRewardStore`), `Data` (tabela `login_reward_progress(account, event, days, seq, clear, last_day)` + migração), `Core/Config` (`LoginRewardConfig[]`), chamada no login do game (KR e JP) | regra §5 com relógio injetável; carta pelo `MailService.SendSystemAsync` | `LoginRewardTests`: 1º login conta 1; mesmo dia não conta; N vezes → clear; Forever reinicia; evento vencido não dá |
| 7 | `Protocol.JP983/Game/JpBotGmEvent.cs` (molde `JpZodiacEvent`), `Core/Config` (`BotGmEventConfig`), `JpGameServer` | sala de torneio 18 buracos/Shuffle/250/35 min, aviso 0x1D3 2× tipo 20, espera 2 min, contagem 0x40 tipo 11, prêmios por carta no fim; GM `!botgm` | `Jp983BotGmEventTests`: aviso com canal e prêmios; sala criada uma vez por janela; prêmios por carta ao terminar |

Regras do projeto: sem LINQ em `src/` (dicionários/laços), sorteios com `Random` injetado, nada de tabela dentro do código
quando couber na config (os padrões acima vão no `PangyaConfig` como valores iniciais e no `pangya.example.json`).

## 8. Pendências [?]
- Consumidor do mapa `HoleCupDropItem` (`iffmgr+0x484`): onde o sprite é usado (copo vs. lista do resultado).
- 0x1D3 tipo 16: o que é a 3ª data; tipo 10000 (`MESSAGE`).
- Unidade de distância das missões do approach no nosso `ApproachGame`; c1 vs c2 na missão 17.
- Layout do 0x147 no KR 645 (para não mandar missão ao cliente KR com formato errado).
- Se o cliente processa 0x216/0x102 com o navegador aberto (por isso a proposta manda só no 0x9E).
