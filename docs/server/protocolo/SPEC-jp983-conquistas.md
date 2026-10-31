# Conquistas (Achievements), Achieve Points e Memorial LV — JP 983

Fontes: `data/pangya_jp.iff` (zip, lido com python `zipfile`; scripts em `<scripts locais>/ach/*.py`), referência AGPL só leitura
(`<ref>/Server/JP/GameServer`, abreviada **[R]**, caminhos relativos a ela; só comportamento), tsv do exe (**[EXE]**).
`[?]` = não confirmado.

Números-chave: **201 conquistas**, **1541 quests** de conquista (todas dão **10 pontos**), **15410 pontos no total**,
**195 contadores** no CounterItem.iff (+ 0x6C000001 "Achievement Point"). Jogador novo na [R]: 202 conquistas
(201 + QuestItem 0x78800001), 1542 quests, **241 contadores**.

---

## 1. Tabelas (dentro do zip)

Cabeçalho de toda tabela: `u16 n, u16 bind, u32 versão (=13)`; registros a partir do byte 8, tamanho fixo `(len-8)/n`.
Common JP de 0xC0 (ver `docs/server/protocolo/SPEC-jp983-iff.md` §1): `+0x00 u8 ativo (+3 pad)`, `+0x04 u32 typeid`, `+0x08 char[64] nome cp932`,
`+0x48 u8 nível`, `+0x49 char[40] ícone`.

| tabela | n | bind | registro | typeid | ativo |
|---|---|---|---|---|---|
| CounterItem.iff | 196 | 0 | 0x118 (280) | 0x6C400001..0x6C4000C4 e 0x6C000001 | 1 em todos |
| QuestStuff.iff | 1862 | 0 | 0x10C (268) | 0x7480xxxx | 1 em todos |
| Achievement.iff | 201 | 0 | 0x600 (1536) | 0x4C000001..0x4D80xxxx | 1 em todos |
| QuestItem.iff (missões diárias) | 257 | 0 | 0x110 (272) | 0x78xxxxxx | — |

### 1.1 CounterItem.iff (0x118)
| off | tipo | campo | observação |
|---|---|---|---|
| 0x00 | common 0xC0 | typeid / nome | ex.: 0x6C400001 "Game Counter", 0x6C400004 "Game Complete Counter" |
| 0xC0 | u8[88] | — | **zero em todos os 196** |

Só serve para nome/existência. **0x6C4000C5** (Mystic Ruins, usado pela conquista do curso 21) **não existe** na tabela.

### 1.2 QuestStuff.iff (0x10C)
| off | tipo | campo | conferido (0x748002C4 "Competition 10 times") |
|---|---|---|---|
| 0x00 | common | typeid 0x7480xxxx, nome = texto da condição (inglês) | |
| 0xC0 | u32[5] | typeid dos contadores | `6C40001F, 6C400004, 6C400002, 0, 0` |
| 0xD4 | i32[5] | quantidade exigida (negativa = "no máximo", ex. score -36) | `10, 1, 1, 0, 0` |
| 0xE8 | u32[3] | prêmio typeid | `6C000001, 0, 0` |
| 0xF4 | u32[3] | prêmio qtd | `10, 0, 0` |
| 0x100 | u32[3] | prêmio tempo | **0 em todas** |

Contadores por quest: 3 → 1235, 1 → 346, 5 → 178, 2 → 92, 4 → 11.
Prêmios das quests de conquista: 0x6C000001 ×10 em **todas as 1541**; além disso 365 prêmios em item, os mais comuns:
0x1A000273 Memorial Coin (Normal) qtd 1–15 (53×), 0x1A000010 Pang Pouch 5000/10000/50000 (30×), 0x1A000272 Memorial Coin (Premium) (15×),
"Mana" de artefato 0x1A0001A4–0x1A0001FB qtd 1, Card Tickets 0x7CC00001/2/3/6 qtd 2–20, Club Modification Kit 0x1A00020F, etc.

### 1.3 Achievement.iff (0x600)
| off | tipo | campo | conferido (0x4C000001) |
|---|---|---|---|
| 0x00 | common | typeid; nome (143 de 201 viraram `???`); **0x49 ícone** `chal_<cat>_<n>` | `chal_0_1` |
| 0xC0 | u32 | **quest base** (0 em 7 conquistas) | 0x748002BC |
| 0xC4 | u32 | **categoria** 0..6 (= ícone `chal_N`, = `(typeid>>22)&7`) | 0 |
| 0xC8 | char[129]×10 | nome de cada nível (índice = mesmo índice de 0x5D4) | "Starting the Competition I" … "Endless Competition III" |
| 0x5D2 | i16 | "Tipo" [?] | 0 em todas |
| 0x5D4 | u32[10] | quests (QuestStuff) em ordem de nível | 0x748002C4 … 0x748002BC, 0 |
| 0x5FC | u32 | Option [?] | 0 em todas |

Categoria (0xC4): 0 modos (19), 1 personagem/caddie/mascote (27), 2 registros (48), 3 campos (21), 4 itens/cartas/memorial (37),
5 desafios/sequências (20), 6 sistemas/lojas (29). Quests por conquista: 9 (116), 6 (56), 5 (8), 8 (7), 3 (6), 7 (6), 4 (1), 1 (1).

**Regras que os dados mostram (valem para as 201):**
- Quando há quest base (194), ela é **sempre a última** da lista e **todas as quests da conquista usam o mesmo contador "principal"**
  = `QuestStuff(base).contador[0]`; os níveis só mudam a quantidade (10, 50, 100, 250…). É um contador **acumulado**.
- Os outros contadores da quest são **condições do mesmo evento**: 0x6C400004 "Game Complete" ×1 e 0x6C400002 "Normal Game" ×1
  aparecem em 1057/1080 quests ⇒ "conta só se a partida foi até o fim". Outros secundários: 0x6C4000A9 GP (97), 0x6C400005 (18), 0x6C4000BB short game (13)…
- Em 5 conquistas com base o principal é o próprio 0x6C400004 e o secundário diz o tipo: 0x4C00000A–0D (jogos de 18/9/6/3 buracos:
  `[6C400004 ×10, 6C40006C ×1, 6C400002 ×1]`) e 0x4D400014 "perfect short gamer" `[6C400004 ×1, 6C40000C ≤ -36, 6C4000BB ×1]`.
- Sem quest base (7: 0x4C800016–1B e 0x4C80001E): contador[0] = 0x6C400004 ×1 e a condição real é secundária, **de uma partida só**:
  distância máx. ≥ 300…600 (0x6C400057), chip-in máx. (0x6C400058), putt máx. (0x6C400077), combo (0x6C40004B), pang da partida (0x6C40000D).
  Cada quest tem o seu próprio contador.

QuestItem 0x78800001 "Clear quests 10 times" (a [R] cria junto como conquista): 1 quest 0x748001C2 = contador 0x6C400039 ×10, prêmio 0x1A0001C4 ×1.

---

## 2. Comportamento da referência

### 2.1 Criação (jogador novo)
`LoginManager.cs:566-580` → `AchievementManager.initAchievement(uid, true)` → `CreateAllAchievements` (`AchievementManager.cs:871-892`):
QuestItem 0x78800001 + **todas as conquistas do IFF, status 3 (ativo)**. Por conquista (`createAchievement`, `:579-699`):
- linha da conquista (`ProcInsertNewAchievement uid, nome, typeid, 1, status`) → id;
- uma linha por quest (`ProcInsertNewQuest uid, nome, ach_id, quest_typeid, counter_typeid`), em que `counter_typeid = contador[0]`
  **só se** a conquista não tem base **ou** a quest é a base; senão 0 (sem contador próprio);
- depois as quests sem contador apontam para o contador da base (`:670-683`).
Status: 1 pendente, 2 excluído, 3 ativo, 4 concluído. Conquistas ficam 3 até todas as quests fecharem → 4.

### 2.2 Avanço (`Game/System/AchievementSystem.cs`)
Um objeto `AchievementSystem` **por evento** (por partida, ou por ação no canal): `incrementCounter(tid[, n])` acumula num saco
`{typeid → valor}` (só typeids do CounterItem.iff; **typeid fora da tabela é ignorado**, `:161-198`), e `finish_and_update` (`:233-395`) avalia:

1. contadores mudados = os do saco com valor ≠ 0 (`:816-819`);
2. para cada um, cada conquista **status 3** que tem um contador com esse typeid → `checkAchievement` (`:822-847`, `:963-1065`);
3. para cada quest **não fechada** com contador (`checkQuestClear`, `:889-961`): todos os contadores da quest precisam estar no saco;
   o principal compara `guardado + saco` com a quantidade (negativa: `≤`); os secundários comparam **só o valor do saco** (evento atual);
   secundário falhou → não conta nada; principal com quantidade 1 não atingida → não conta;
4. fechou: soma o delta uma vez no contador (`CHECK_CHANGE_COUNTER_AND_INCREMENT_AND_SAVE`, `:217-230`), `clear_date = agora` (`:1041`),
   prêmios (`:1028-1038`): **0x6C000001 → `incrementPoint(qtd)`** (só memória); o resto vai para a lista de itens;
   se a quest não é a base mas usava o contador da base, cria um **contador novo congelado** com o valor atual (`:984-1018`);
   todas fechadas → status 4 (`:1047-1051`);
5. não fechou mas progrediu → soma o delta **só se** a conquista tem base ou é QuestItem (`:1054-1059`). Ou seja, **o contador só é
   gravado quando alguma quest casa** (contadores fora de conquista não existem);
6. missões diárias fechadas no mesmo evento somam 0x6C400039 (`:849-883`).

**Entrega dos prêmios**: direto no inventário, sem correio (`ItemManager.addItem`, `:286-311`); Pang Pouch vira pang e não entra na lista.

### 2.3 Pontos e Memorial LV
- Pontos = soma das qtd de 0x6C000001 das quests fechadas. Na carga (`AchievementManager.cs:288-306`) a [R] tem **bug**: soma o
  **typeid** (0x6C000001) em vez da qtd. Durante a sessão soma certo (`AchievementSystem.cs:1034`). Não é gravado no banco.
- `MemorialSystem.cs:177-188`: `LV = pontos == 0 ? 0 : min(24, (pontos-1)/300)` (`MEMORIAL_LEVEL_MAX = 24`, `:467`). LV 1 a partir de 301
  (31 quests), LV 24 a partir de 7201 (721 quests). Só serve para filtrar o gacha (banco `pangya_new_memorial_level`), e a [R] nem filtra (`:350`).
- **Nenhum pacote da [R] leva pontos ou LV ao cliente.** A janela do Papel/Memorial (`FrBongdariShopDlg`) manda **C->S 0x157** [EXE
  `bongdarishopdlg.cpp` FUN_008be2e0 e FUN_008bc2a0]; a resposta é 0x22D com o valor e a data de cada quest. Conclusão: **o cliente calcula**
  "Achieve Points" (somando os prêmios 0x6C000001 do QuestStuff.iff das quests com data ≠ 0) e o "Memorial LV" [?] — falta conferir no exe/cliente.

---

## 3. Contadores incrementados pela referência

`v` = quantidade passada (sem `v` = +1). Tudo de partida vai para `pgi.sys_achieve` e só é avaliado no `finish_and_update` do fim
(Versus.cs:224/615, Match.cs:260/1903, Tourney.cs:1588/1677, GrandPrix.cs:384/2461, PangBattle.cs:253/2122, GuildBattle.cs:803,
GrandZodiac.cs:390, SpecialShuffleCourse.cs:576, ChipInPractice.cs:316, Practice.cs:516).

### 3.1 Início da partida — `GameBase.initAchievement` (`Game/Base/GameBase.cs:3056-3128`), chamado em cada modo
| typeid | o quê | linha |
|---|---|---|
| 0x6C400002 | Normal Game (toda partida) | 3070 |
| 0x6C4000BB | short game | 3073 |
| 0x6C400098 / 0x6C400099 | dono da sala / dono com artefato | 3077 / 3080 |
| 0x6C40000F–18, 0x6C400040, 0x6C4000B9, BA, C4 | personagem equipado (`getCharacterCounterTypeId`, AchievementSystem.cs:426-478) | 3086 |
| 0x6C400019–1C, 41–45 | caddie (`getCaddieCounterTypeId`, :481-539) | 3095 |
| 0x6C400046–49 | mascote (`getMascotCounterTypeId`, :542-564) | 3104 |
| 0x6C400020–31, 6D, A1, C5 | campo (`getCourseCounterTypeId(map & 0x7F)`, :567-640; curso 12 sem contador) | 3110 |
| 0x6C400069/6A/6B/6C | 3/6/9/18 buracos | 3115 |

### 3.2 Modo (no construtor do modo, logo após o initAchievement)
| typeid | modo | arquivo:linha |
|---|---|---|
| 0x6C40001D | Versus (stroke) | GameModes/Versus.cs:56 |
| 0x6C40001E | Match | Match.cs:111 |
| 0x6C40001F | Torneio / SSC | Tourney.cs:64, SpecialShuffleCourse.cs:59 |
| 0x6C40003A / 0x6C40003B | Guild battle (time / guilda) | GuildBattle.cs:57 / 60 |
| 0x6C40003C | Grand Zodiac (HIO) | GrandZodiac.cs:30 |
| 0x6C40003F | Chip-in practice | ChipInPractice.cs:29 |
| 0x6C4000A9 + classe (AA rookie, AB beginner, AC junior, AD event-aba, AE GP evento) | Grand Prix | GrandPrix.cs:177/181 (classe :138-161) |
| 0x6C4000B7 / 0x6C4000B8 | GP 1 jogador só com IA / 30 jogadores sem IA | GrandPrix.cs:1200-1217 |
| 0x6C400005 por buraco; 0x6C40003D repeat / 0x6C40003E curso | Practice (sem initAchievement) | Practice.cs:94 / 172 |

### 3.3 Por buraco
| typeid | o quê | linha |
|---|---|---|
| 0x6C400006 HIO, 07 Albatross, 08 Eagle, 09 Birdie, 0A Par | score do buraco (`getScoreCounterTypeId`) | GameBase.cs:2136-2138 (PangBattle.cs:907-911) |
| 0x6C400075 + 0x6C400050 | booster de tempo usado | TourneyBase.cs:953-955, VersusBase.cs:1955-1957, GrandPrix.cs:549-551 |

### 3.4 Por tacada que entrou (`update_sync_shot_achievement`, GameBase.cs:3180-3260; chamado em TourneyBase.cs:2164, VersusBase.cs:3305, Match.cs:770, GrandPrix.cs:1175)
| typeid | o quê |
|---|---|
| 0x6C400035/34/33/32 | putt que entrou ≥ 30/25/20/17 y (cumulativos: 30 y soma os 4) |
| 0x6C40006F beam impact, 64 spin front, 65 spin back, 66 curva, 67 tomahawk, 68 spike, 6E cobra | buraco feito com a tacada |
| 0x6C40005B | chip-in sem tacada especial |
| 0x6C400059 | entrou errando o pangya (não putt) |
| 0x6C400051 / 0x6C400052 | power shot / double power shot (toda tacada, mesmo sem entrar) |

### 3.5 Fim da partida
| typeid | o quê | linha |
|---|---|---|
| 0x6C40004C OB, 4E bunker, 55 tacadas+putts, 05 buracos, 56 jardas totais, 0B pangya, 0D pang da partida, 0C score | totais da partida, valor `v` (`records_player_achievement`, chamado em requestSaveInfo GameBase.cs:1732 e PangBattle.cs:796) | GameBase.cs:3141-3174 |
| 0x6C400057 / 58 / 77 | melhor drive / chip-in / long putt da partida (`v` = jardas truncadas) | 3159-3165 |
| 0x6C40004B | combo da partida (`v`) | GameBase.cs:1752, PangBattle.cs:816 |
| 0x6C400053 | tickets SSC que caíram (`v`) | GameBase.cs:1357 |
| 0x6C40004F + por item (`getActiveItemCounterTypeId`, 0x6C400070–74, 8E–96, BD) | itens ativos usados (`v`) | GameBase.cs:1929-1934 |
| 0x6C400075 + por item (`getPassiveItemCounterTypeId`: 50, 76, 97) | itens passivos usados (`v`) | GameBase.cs:1874-1879 |
| 0x6C40005D–63 | sequência de 2+ buracos com o mesmo score (+1 por sequência; birdie, par, bogey, dbogey, eagle, alba, HIO) | GameBase.cs:3300-3350 |
| 0x6C40009A / 9B / 9C / 9D | buracos com chuva / 2 / 3 / 4+ seguidos (`v`) | GameBase.cs:3385-3390 / 3282-3291 |
| 0x6C4000B6 / 0x6C4000AF | torneio top 3 / 1º lugar | TourneyBase.cs:2586-2590 |
| **0x6C400004** | Game Complete — partida terminada (também no give-up/time-out ≥3) | Versus.cs:221/291, Match.cs:257/535, Tourney.cs:309, GrandPrix.cs:381/779, PangBattle.cs:250/497, GuildBattle.cs:397, GrandZodiac.cs:117, SSC.cs:232, Approach.cs:399 |

### 3.6 Fora da partida (um objeto por pedido, `finish_and_update` no fim do handler, **depois** da resposta da ação)
| typeid | evento | arquivo:linha |
|---|---|---|
| 0x6C4000A0 | login do dia (prêmio de presença, 1/dia) | System/AttendanceRewardSystem.cs:194 |
| 0x6C40004A / 0x6C400081 | jogar Papel (normal e Big) / ganhou raro | Channel.cs:17214, 17405 / 17205, 17395 |
| 0x6C4000B2 / B3 | moeda Memorial normal / especial | Channel.cs:13476 / 13480 |
| 0x6C4000B5 / B4 | raro do Memorial tipo 0–2 / ≥3 | Channel.cs:13576 / 13580 |
| 0x6C400082 | Cadie Magic Box (troca) | Channel.cs:7613 |
| 0x6C400083 | compra em loja de jogador | PersonalShop.cs:574 |
| 0x6C400084 / 85 | upgrade / downgrade de taco | Channel.cs:8959 / 8993 |
| 0x6C400084 / 85 / C3 | stats de personagem +/− / expandir maestria | Channel.cs:7804 / 7918 / 8124 |
| 0x6C400087 / 88 | equipar carta (2 handlers) / tirar carta | Channel.cs:8356, 8599 / 8838 |
| 0x6C400078 + 79/7A/7B(+80 Abbot)/A8 + 7C+raridade (+7F extra se secreta) | abrir pacote de cartas | Channel.cs:13825-13875 |
| 0x6C400089 + 0x6C40008A+raridade | fusão de cartas (Lolo) | Channel.cs:14089-14092 |
| 0x6C40009E | usar carta especial | Channel.cs:14402 |
| 0x6C400054 | abrir Spin Cube (caixa do correio) | Channel.cs:12731 |
| 0x6C400086 | Point shop / Tiki legado | Channel.cs:16391, 16585 |
| 0x6C4000BE/BF/C0, C1, C2(`v`) | Tiki shop: reciclar normal/CP/raro, bônus, pontos | Channel.cs:9132-9138, 9160, 9239 |
| 0x6C4000A2, A3 (+A7 rank S), A4, A5, A6 | ClubSet workshop | Channel.cs:11063, 11479/11476, 11626, 10580, 10735; 0xA3 também 11696 |
| 0x6C40009F | missão diária: pegou prêmio (não conta a de 10) | Manager/DailyQuestManager.cs:399 |
| 0x6C400039 | missão diária fechada (interno do sistema) | AchievementSystem.cs:850 |

**Sem incremento na [R]** (conquistas que nunca andam): 0x6C400036 Spin Cube (em jogo), 0x6C400037 Field Coin, 0x6C4000C5 Mystic Ruins
(não está no CounterItem.iff). A [R] parece trocar 0xC1 "Cumilate TP" com 0xC2 "Extra Bonus Mileage" [?].

### 3.7 Cobertura
Contador "real" de cada conquista (principal; ou o secundário quando o principal é 0x6C400004):

| grupo | conquistas (de 201) |
|---|---|
| início da partida (personagem, caddie, mascote, campo, nº de buracos, sala) | 55 |
| fim da partida (totais, recordes, sequências, chuva, itens usados, Game Complete) | 53 |
| modo (versus, match, torneio, GP, guild, zodiac, practice, top 3/1º) | 28 |
| tacada (sync shot) | 13 |
| buraco (HIO/alba/eagle/birdie/par) | 5 |
| **fora da partida** (Papel, Memorial, cartas, tiki, clubset, login, loja) | 47 |

O essencial é **0x6C400002 no início + 0x6C400004 no fim de toda partida**: aparecem em ~1060 quests de **143 conquistas** (sem eles
essas 143 não andam). Com o grupo início + fim + modo (≈ 25 typeids fixos + 4 tabelas de mapeamento) cobrem-se 136/201.

---

## 4. Pacotes e ordem

| pacote | dir | layout | quando |
|---|---|---|---|
| 0x21D | S->C | `u32 0, u32 total, u32 n, n × {u8 ativo=1, u32 counter_tid, i32 counter_id, i32 valor}` (13 B) | login, **sempre** (mesmo vazio) |
| 0x21E | S->C | `u32 0, u32 total, u32 n, n × {u8 ativo=1, u32 ach_tid, i32 ach_id, i32 status, u32 q, q × {u32 quest_tid, u32 counter_tid, i32 counter_id, u32 clear_unix}}` (sem contador: 8 B zero) | login, só se houver |
| 0x157 | C->S | `u32 uid` (próprio ou de outro; [EXE] Papel/Memorial, perfil, my room, messenger, console) | |
| 0x22D | S->C | `u32 0, u32 total, u32 n, n × {u32 ach_tid, i32 ach_id, u32 q, q × {u32 quest_tid, i32 valor do contador da quest, u32 clear_unix}}` | resposta ao 0x157 |
| 0x22C | S->C | `i32 0` ok / `1` erro | depois dos 0x22D |
| 0x216 | S->C | `u32 unix, u32 n, n × 50 B {u8 2, u32 counter_tid, i32 counter_id, i32 0, i32 antes, i32 depois, i32 delta, 25×0}` | progresso (contadores) |
| 0x216 | S->C | mesmo formato, itens de prêmio (`stat` do item, qtd) | se houve prêmio em item |
| 0x22E | S->C | `u32 n, n × {u32 ach_tid, u32 quest_tid}` (pode ir n=0) | quests fechadas |
| 0x220 | S->C | `i32 0, u32 n, n × AchInfo` (igual ao item do 0x21E) | conquistas que mudaram |

- **Login** (`cLoginTask.cs:80-110`): … 0x102 → 0x131 → **0x21D → 0x21E** → 0xF1 → 0x135 → 0x144 → 0x138 …
- **0x157** (`packet_func_sv.cs:4828-4875`): 0x22D (pedaços) + 0x22C(0); erro → 0x22C(1). Bugs da [R]: lista vazia → **não responde nada**;
  jogador offline → manda duas vezes. Hoje (n=0 + 0x22C) o cliente mostra erro ⇒ mandar a lista real.
- **Depois de evento** (`finish_and_update`): só se alguma conquista mudou: 0x216 contadores → 0x216 itens → 0x22E → 0x220.
  Na partida: depois do 0x06B do mascote/tesouro e **antes do 0x244** (Versus.cs:600-620). No canal: depois da resposta da ação.
- **Pedaços** (`AchievementManager.cs:406-421`): `n*196 < 900` → um pacote; senão 20 por pacote, **cada pedaço com total = n = tamanho do pedaço**
  (igual ao inventário 0x73 que já usamos, "total = n como o GB"). [EXE] do 0x21D: "ao completar o total liga a UI". [?] se o cliente
  acumula os pedaços ou recomeça a lista — testar (alternativa: total = tudo, n = pedaço). Tamanhos típicos: 0x21E ≈ 161 B/conquista de 9 níveis (≈ 32 KB no total), 0x22D ≈ 120 B.

---

## 5. Proposta (Domain versão-independente + camada JP)

### 5.1 Ids sem banco
O cliente só precisa de ids estáveis e coerentes entre 0x21D/0x21E/0x216/0x22D. Dá para derivar do IFF, sem criar 1700 linhas por conta:
- `ach_id = índice no Achievement.iff + 1` (QuestItem 0x78800001 = 1000, se for usar);
- `counter_id = ach_id * 16 + slot` (slot 0 = contador da base; 1..10 = contador próprio da quest, nas 7 sem base).
A "cópia congelada" da [R] para níveis já fechados pode ser omitida (aponta para o contador da base) [?] ou emulada com `value_at_clear`.

### 5.2 Banco (migration nova, PostgreSQL como o resto)
```sql
create table achievement_counters (
    account_id     bigint not null references accounts(id) on delete cascade,
    achievement_tid int   not null,
    slot           smallint not null default 0,     -- 0 = acumulado da conquista; 1..10 = quest sem base
    value          bigint not null default 0,
    primary key (account_id, achievement_tid, slot));
create table achievement_quests_cleared (
    account_id     bigint not null references accounts(id) on delete cascade,
    achievement_tid int   not null,
    quest_tid      int   not null,
    value_at_clear bigint,                          -- opcional: o "contador congelado" da referência
    cleared_at     timestamptz not null default now(),
    primary key (account_id, quest_tid));
```
Status (3/4), pontos e LV são **derivados** (não gravar): concluída = todas as quests na tabela; pontos = Σ prêmio 0x6C000001.

### 5.3 Domain (`Pangya.Domain/Achievements/`)
- `IAchievementData` (como `IMemorialData`; implementado por `Jp983GameData`, null no KR 645):
  `IReadOnlyList<AchievementDef> Achievements` — `AchievementDef(int TypeId, int Index, int Category, int BaseQuest, QuestDef[] Quests)`,
  `QuestDef(int TypeId, (int Tid, int Qty)[] Counters, (int Tid, int Qty, int Time)[] Rewards, int Points)`; pré-calcular por conquista
  o contador principal e um índice `counterTid → conquistas` (dicionários, sem LINQ).
- `AchievementCounters` (constantes, como `Trophy.cs`): os typeids e as tabelas personagem/caddie/mascote/curso/score/itens do §3.
- `CounterBag` (por evento): `Increment(int counterTid, int amount = 1)` (ignora 0 e tid desconhecido).
- `AchievementService`:
  - `Task<AchievementBook> LoadAsync(long accountId)` (sessão);
  - `Task<AchievementChanges> CommitAsync(Player p, AchievementBook book, CounterBag bag)` — aplica as regras do §2.2:
    para cada tid do saco → conquistas não concluídas cujo principal é esse tid → por quest aberta: secundários contra o saco
    (qtd < 0 ⇒ `≤`); com base: `valor += bag[tid]` uma vez e fecha todo nível atingido; sem base: fecha se `bag[tid]` atinge a qtd.
    Grava contador/quests, entrega prêmios (0x6C000001 só soma pontos; itens pelo mesmo caminho de concessão do Memorial; Pang Pouch → pang).
  - `AchievementChanges { List<(int Id,int Tid,long Before,long After,long Delta)> Counters; List<(int AchTid,int QuestTid)> Cleared;
    List<int> ChangedAchievements; List<GrantedItem> Items; int PointsGained }`, vazio = nada a mandar.
  - `int Points(book)`, `int MemorialLevel(points) => points == 0 ? 0 : Math.Min(24, (points - 1) / 300)` (usar no `MemorialService`).
- Ganchos (o saco vive no estado da partida do jogador; o handler é compartilhado KR/JP):
  início: `GameHandler` ao começar (0x02, sala, personagem/caddie/mascote, curso, nº de buracos, modo);
  buraco: `GameHandler.Play.cs` (score do buraco, tacadas especiais, power shot);
  fim: `GameHandler.GameEnd.cs` `FinishAsync` (totais de `lastGameStats`, 0x04, sequências, top 3) → `CommitAsync`;
  canal: Papel/Memorial (`JpSystems.cs`), cartas, magic box, spin cube, workshop, login diário → saco próprio + `CommitAsync`.
- Notificação: interface opcional na conexão (`IAchievementSink.Send(AchievementChanges)`), implementada só pelo JP.

### 5.4 Camada JP (`Pangya.Protocol.JP983`)
- `Jp983GameData`: ler Achievement/QuestStuff/CounterItem (offsets do §1) → `IAchievementData`.
- Login: 0x21D (sempre) + 0x21E logo depois do 0x131 e antes do 0xF1.
- `fromClient[0x157]` (hoje `r => null` em `JpTranslator.JpOnly.cs:40`): 0x22D (pedaços) + 0x22C(0); outro jogador: carregar o livro dele; erro: 0x22C(1).
- `IAchievementSink`: 0x216 (contadores, tipo 2) → 0x216 (itens) → 0x22E → 0x220; na partida antes do 0x244.

### 5.5 Riscos e pontos [?]
1. Semântica do `total` nos pedaços (0x21D/21E/22D) — testar no cliente com > 20 itens.
2. "Achieve Points"/"Memorial LV" calculados pelo cliente a partir do 0x22D — confirmar na janela do Papel/Memorial (se não mudar, procurar campo no exe).
3. Contador congelado dos níveis já fechados: valor exibido pode ficar estranho se apontar para o acumulado.
4. 0x216 com contadores (tipo 2, typeid 0x6C…) junto do inventário: é o que a [R] faz; conferir que o cliente não cria item.
5. Não copiar bugs da [R]: pontos somando typeid na carga; 0x157 sem resposta com lista vazia e resposta dupla offline; contadores 0x36/0x37/0xC5
   nunca incrementados; 0xC1/0xC2 possivelmente trocados; "+1 extra" em 0x6C40007F para carta secreta.
6. Conquistas 0x4D400014 (score ≤ −36 em short game) e as de valor por partida (distância/combo/pang) dependem de o saco ter o **valor da partida**, não acumulado.
7. Dar prêmios na hora (inventário) pode estourar limites/duplicar com 0x216 se o fluxo de concessão já mandar o seu — usar um caminho só.
8. Missões diárias (QuestItem, 0x6C400039/0x6C40009F) ficam fora até existir o sistema.
