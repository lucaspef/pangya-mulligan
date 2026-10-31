# JP 983 — Oficina de club set (ClubSet WorkShop), completa

12/10/2026. Fontes: referência JP da comunidade (C#, AGPL; **só comportamento**; ids já do JP):
`ref/GameServer/Game/Channel.cs:8858-8875` (evento) e `:10402-11945` (oficina), `Game/Base/GameBase.cs:1762-2090, 2388-2551`
(EXP do taco no jogo), `Game/GameModes/*.cs` (quem chama), `Models/pangya_game_st.cs` (structs), `PangyaAPI.IFF.JP/...`
(modelos IFF). Desmonte do `<pasta do jogo>\ProjectG.exe` (capstone; leitores `0xA4DEB0` u8, `0xA4DEC0` u16,
`0xA4DED0` u32, `0xA4E1F0` str; despachante comum `0x009C20A0`) e `docs/server/jp983-protocolo/*.tsv`. Dados: `data/pangya_jp.iff`
(o zip do ProjectG96x.pak; conferido com python).

Legenda: **[EXE ✓]** conferido no exe 983 · **[DADO ✓]** conferido no pangya_jp.iff · **[REF]** só a referência ·
**[?]** não confirmado / inferência. Convenção: little-endian; `str` = u16 tamanho + bytes; stats na ordem
**0 força (power), 1 controle, 2 impacto/precisão (accuracy), 3 spin, 4 curva** (nomes das imagens da UI:
`clubmade_remodel_power/control/impact/spin/curve*.tga` em `ProjectG951a.pak`).

---

## 0. Resumo

- A oficina mexe em **slots** do club set (o teto de upgrades com pang), não nos stats diretamente. Cada club guarda
  `ws.c[5]` (slots extras da oficina), `mastery` (pontos de maestria), `recovery` (cancelamentos usados) e, no fio,
  `level`/`rank`.
- **Rank e nível vêm da soma dos slots**: `soma = Σ(Slot do ClubSet.iff + ws.c)`; se `30 ≤ soma ≤ 59`:
  `rank = (soma−30)/5` (0=E, 1=D, 2=C, 3=B, 4=A, 5=S) e `nível = (soma−30)%5` (0..4); fora disso rank −1. **[EXE ✓]**
  (função do cliente `0x497F60`, tabela de 30 entradas `0,0,0,0,0,1,…,5`; letras pelas imagens `clubmade_rank_e..s.tga`).
- **Subir nível** (0x164) = +1 slot num stat sorteado (pesos de `ClubSetWorkShopLevelUpProb.iff`), limitado por
  `ClubSetWorkShopLevelUpLimit.iff`; gasta material (Club Modification Kit ou o card "Abbot Elemental Shard").
  Confirmar (0x165) mantém; cancelar (0x166) desfaz e gasta 1 ponto de recuperação (máx. `total_recovery` = 5).
- **Subir rank** (0x167) = +1 slot num stat escolhido pelo nº de cards + gasta maestria
  (`ClubSetWorkShopRankUpExp.iff`); no rank S ganha +1 extra no `rank_s_stat`. Clubs com `flag_transformar` têm
  **6,67 %** de virar um taco especial (0x241 → 0x168/0x169).
- **Maestria no jogo**: por **buraco terminado** (inclusive desistência), `+⌊10 × ws.rate × taxa_servidor × taxa_club⌋`
  no taco equipado no início; gravada no fim (ou ao sair) e avisada com **0x216 tipo 0xCC** (bloco de 23 B).
- Os `.sff` da oficina (`ChangeProb`, `ChangeItem`) **não estão** no `pangya_jp.iff`; a referência não os lê e
  sorteia a transformação com valores fixos. Fallback: o mesmo.

---

## 1. Ganho de maestria dentro do jogo

### 1.1 Fórmula [REF] (`GameBase.requestUpdateItemUsedGame`, GameBase.cs:1775)
No início da partida (`requestInitItemUsedGame`, :2388): `club_typeid = club equipado`, `count = 0`,
`club_rate = ClubSet.work_shop.rate` (f32, IFF 0xF0; 1.0 na maioria, 1.5 em alguns, 0.0 no "Caddies Set").
Taxas em "por cento" com base 100: `rate.club = 100 + bônus` (ver §3).

A cada buraco **terminado** (bola no buraco **ou** desistência — `acerto_hole || giveup`):

```
count += (u32)( K × 10 × club_rate × T(srv_club) × T(rate.club) )     // truncado a cada buraco
T(v) = v ≤ 0 ? 1.0 : v / 100
```

| modo | K | quem chama por buraco | obs. |
|---|---|---|---|
| Versus (stroke), Match, Pang Battle, Tourney, Guild Battle, Grand Prix, **Practice** | 1 × 10 | `finishHole` de cada modo | GameBase |
| Special Shuffle Course | 1.5 × 10 | SpecialShuffleCourse.cs:406 | override |
| Approach | **sem 10 e sem club_rate**: `count += (u32)(T(srv)×T(club))` (=1/buraco) | Approach.cs:505 | override |
| Grand Zodiac, Chip-in Practice | **0** (override não soma maestria) | GrandZodiacBase.cs:726 | — |

`srv_club` = `rate.club_mastery` da config do servidor (`CLUBMASTERYRATE` no ini; ≤0 → 1.0).
Exemplo: 18 buracos, taco rate 1.0, sem bônus = **180**; com "Drops of Ebot" (+200 → 3.0×) = 540.

### 1.2 Quando grava e o que manda [REF] (`requestFinishItemUsedGame`, GameBase.cs:2023-2090)
- Chamado no fim da partida de cada jogador **e também quando ele sai no meio** (`deletePlayer` dos modos) — os buracos já
  jogados contam. Uma vez só (`finish_item_used`).
- Se `count > 0`: acha o club **pelo typeid** no inventário (`findWarehouseItemByTypeid`, o 1º desse typeid),
  `ws.mastery += count`, grava (`ProcUpdateClubSetWorkshop`, flag 0) e põe no **0x216** um item `tipo 0xCC` com o bloco
  da oficina (§5.2). O 0x216 sai junto com o dos itens gastos na partida, antes das telas de resultado (`requestFinishData`).
- Não há pacote próprio de "ganhou X de maestria": o cliente vê o total novo pelo 0x216. [?] se a tela de resultado do 983
  mostra o ganho (nenhum campo de maestria achado nos pacotes de fim de jogo da referência).
- Mastery de clubs que não evoluem (`tipo −1`, ex.: Air Knight Set, rate 1.5) **acumula** e serve como **origem** de
  transferência (0x16C não confere o `tipo` da origem).

---

## 2. Conceitos e invariantes

| campo (por club) | o que é | faixa |
|---|---|---|
| `ws.c[5]` (i16) | slots ganhos na oficina por stat | ≥ 0 |
| `mastery` (u32) | pontos de maestria | ≥ 0 |
| `recovery` (u32) | cancelamentos de nível já usados (0x166) | 0 .. `total_recovery` (5) |
| `level` | nível dentro do rank (fio: u8 no 0x216, i32 no item 196) | ver §7.3 |
| `rank` | rank atual (fio: u32/i32) | 0..5 (−1 inválido) |

- Teto de upgrade com pang do stat `i`: `(Slot[i] − Attr[i]) + ws.c[i]` [REF] (`PlayerInfo.cs:1240`,
  `Channel.requestClubSetStatsUpdate`). Hoje `PlayerActions.UpgradeAsync` usa só `Slot − Attr` → precisa somar `ws.c`.
- Rank/nível: §0 (cliente calcula igual a partir de `Slot` do IFF + `ws.c` do item — `CConsole` 0x72550C soma
  `ClubSet.iff+0xDA..0xE2` com os 5 i16 do item **[EXE ✓]**).
- Nível 4 → o próximo +1 é um **rank-up** (cobra maestria). A referência **não** confere isso no 0x164 (confia no
  cliente). [?] recomendado recusar 0x164 quando `nível == 4` (senão o rank sobe de graça).

---

## 3. Itens que alteram a maestria / usados na oficina

| typeid | nome (IFF) | papel | fonte |
|---|---|---|---|
| **0x1A000338** | Drops of Ebot (a ref chama "Banana Club Mastery Boost") | passivo: `rate.club += 200` (3×) enquanto estiver no inventário; consumo pelo uso informado pelo cliente [?] | [REF] `passive_item_club_boost`, `passive_item` |
| **0x70210009** | Graphite Ring (L) (anel "Kurafaito") | equipado no personagem: `rate.club += 10` (+10 %) | [REF] GameBase.cs:2547 |
| card especial com `efeito == 34` | (cards de subgrupo 2 equipados sem dono) | `rate.club += efeito_qntd` | [REF] GameBase.cs:2464 |
| — | config do servidor `club_mastery` | multiplicador global `T(srv)` | [REF] |
| **0x1A00020F** | Club Modification Kit | material do **subir nível** (qtd 1) | [EXE ✓] console 0x7254B8 |
| **0x7C800026** | Abbot Elemental Shard (card) | material do subir nível com bônus de chance (qtd 1..5) | [EXE ✓] console 0x725487 |
| **0x7C800041** | (card "Soren" na tradução) | cards do **subir rank** (qtd 0..4 escolhe o stat) | [EXE ✓] console 0x72579B |
| **0x1A000210** | Abbot Coating Spray | **recuperar** pontos de recuperação (0x16B) | [EXE ✓] console 0x72587F |
| **0x1A000211** | UCIM Chip | **transferir maestria**: 300 pontos por chip (0x16C) | [EXE ✓] console 0x7258DF; [REF] 300 |
| **0x1A000247** | Titan Cleansing Powder (Soft) | **reset** sem devolução (0x16D) | [EXE ✓] 0x725987; [REF] |
| **0x1A00024B** | Titan Cleansing Powder (Hard) | **reset** devolvendo 50 % da maestria e do pang gastos | [EXE ✓] 0x725997; [REF] |

Notas [REF]: o 0x164 aceita **qualquer** item do grupo Item ou Card (não confere o typeid); o 0x16B aceita qualquer item;
o 0x16C, qualquer typeid como "chip". Recomendo restringir aos typeids acima.

---

## 4. Tabelas IFF

Todas no zip `data/pangya_jp.iff` (cabeçalho `u16 n, u16 bind, u32 versão=13`, registros fixos). Conferido **[DADO ✓]**;
tamanhos batem com `iff_983.tsv` **[EXE ✓]**.

### 4.1 ClubSet.iff (0x104) — bloco `work_shop` (0xE4..0x103)

| off | tipo | campo | uso |
|---|---|---|---|
| 0xD0 | i16[5] | Attr (stats base) | — |
| 0xDA | i16[5] | Slot (slots base, inclui o Attr) | rank/nível; teto de upgrade = Slot−Attr+ws.c |
| 0xE4 | i32 | `tipo` | −1 = não evolui (recusa 0x164/0x167/0x16B; destino do 0x16C); 0..4 = linha de Prob/Limit |
| 0xE8 | u32 | `rank_s_stat` | stat que ganha +1 extra ao chegar no rank S (valores 0,1,3,4) |
| 0xEC | u32 | `total_recovery` | máx. de cancelamentos (5 nos que evoluem; 0 nos −1) |
| 0xF0 | f32 | `rate` | multiplicador da maestria por buraco (1.0 / 1.5 / 0.0) |
| 0xF4 | u32 | `tipo_rank_s` | linha do RankUpExp (0..3) |
| 0xF8 | u32 | `flag_transformar` | 1 = pode virar taco especial no rank-up |
| 0xFC | u32 | desconhecido (sempre 0) | — |
| 0x100 | u32 | `text_pangya` (logo/família) | a ref agrupa os especiais por ele (93/94/95) |

Dados notáveis: os 3 especiais e suas 5 versões por rank (soma de Slot 35/40/45/50/55 = rank 1..5, nível 0):
Wingtross Evo-Knight `0x1000005D/62/65/68/6B` (família 93), Giga Yard Totem Pole `0x1000005E/63/66/69/6C` (94),
Duostar Manapikal `0x1000005F/64/67/6A/6D` (95); `tipo 4`, `tipo_rank_s 3`. Com `flag_transformar = 1`:
`0x10000001–07, 0D–10, 1B, 61` (Air Knight 2, Batter Up!, Handyman, Spike Hammer, SY/SE Afterburner, DuoFeather,
Samurai/Blessed Mithril/Cursed Obsidian Blades, Microphone, Marine Blues, Papel Training). `tipo −1`: Air Knight Set,
Lucky AK 0x14, Special AK 0x57, Sengoku/Songoku/Paradox/Eternal VIP, Caddies Set.

### 4.2 ClubSetWorkShopLevelUpProb.iff (0x18 = 24 B) — 5 registros
`0x00 u32 tipo · 0x04 u32[5] peso por stat`. (A ref lê 6 u32 depois do tipo: erro dela; são 5.)

| tipo | força | controle | impacto | spin | curva | Σ | % (F/Ct/I/S/Cv) |
|---|---|---|---|---|---|---|---|
| 0 | 500 | 200 | 800 | 500 | 600 | 2600 | 19,2 / 7,7 / 30,8 / 19,2 / 23,1 |
| 1 | 400 | 150 | 600 | 450 | 550 | 2150 | 18,6 / 7,0 / 27,9 / 20,9 / 25,6 |
| 2 | 300 | 100 | 400 | 400 | 400 | 1600 | 18,75 / 6,25 / 25 / 25 / 25 |
| 3 | 200 | 50 | 200 | 300 | 250 | 1000 | 20 / 5 / 20 / 30 / 25 |
| 4 | 100 | 10 | 100 | 100 | 100 | 410 | 24,4 / 2,4 / 24,4 / 24,4 / 24,4 |

(Percentuais com todos os stats elegíveis; os que já bateram o limite saem do sorteio.)

### 4.3 ClubSetWorkShopLevelUpLimit.iff (0x14 = 20 B) — 30 registros (tipo 0..4 × rank 0..5)
`0x00 u32 tipo · 0x04 u32 rank · 0x08 u16[5] limite de (Slot+ws.c) por stat · 0x12 u16 option (sempre 0)`.

| tipo\rank | 0 | 1 | 2 | 3 | 4 | 5 |
|---|---|---|---|---|---|---|
| 0 | 14,12,12,5,5 | 15,12,13,6,6 | 16,12,14,6,6 | 18,13,15,7,7 | 20,13,16,8,8 | 20,13,16,8,8 |
| 1 | 14,12,12,5,5 | 16,12,13,6,6 | 17,12,14,6,6 | 19,13,15,7,7 | 20,13,16,8,8 | 20,13,16,8,8 |
| 2 | 14,12,12,5,5 | 15,12,13,6,6 | 16,13,14,6,6 | 18,13,15,7,7 | 20,13,16,8,8 | 20,13,16,8,8 |
| 3 | 14,12,12,6,5 | 15,12,13,7,6 | 16,12,14,7,6 | 18,13,15,8,7 | 20,13,16,8,8 | 20,13,16,8,8 |
| 4 | 15,13,13,6,6 | 17,13,14,6,6 | 19,13,15,7,7 | 21,13,17,8,8 | 22,14,18,9,9 | 22,14,18,9,9 |

Subir nível usa a linha do **rank atual**; subir rank confere o stat contra a linha do **rank novo**.

### 4.4 ClubSetWorkShopRankUpExp.iff (0x1C = 28 B) — 4 registros
`0x00 u32 tipo (= tipo_rank_s do club) · 0x04 u32[6] maestria para chegar no rank 0..5` (índice = rank novo).
(A ref lê só 5 → o índice 5 (S) estoura: erro dela.)

| tipo_rank_s | E(0) | D(1) | C(2) | B(3) | A(4) | S(5) | total E→S |
|---|---|---|---|---|---|---|---|
| 0 | 0 | 900 | 2 800 | 11 000 | 20 200 | 68 000 | 102 900 |
| 1 | 0 | 1 000 | 4 800 | 15 000 | 32 500 | 75 000 | 128 300 |
| 2 | 0 | 900 | 2 800 | 11 000 | 20 200 | 68 000 | 102 900 |
| 3 | 0 | 1 200 | 5 300 | 17 600 | 35 000 | 90 000 | 149 100 |

### 4.5 ClubSetWorkShopChangeProb.sff (0x1C) e ClubSetWorkShopChangeItem.sff (0x14)
- **Não estão** no `pangya_jp.iff` (zip lista só GrandPrixAIOptionalData.sff e MemorialShopCoinItem.sff) **[DADO ✓]**.
  Nos paks cuja tabela consegui ler (XOR 0x71: 826, 951a, 960–965) também não há `.sff` soltos; os outros paks usam
  outra chave e não foram abertos [?].
- O exe tem os carregadores **[EXE ✓]**: `MakeWorkShopChangeProb983` (0x9D9F10) copia cada registro de 7×u32 para
  **um único** lugar (`this+0x3AC`; fica o último); `MakeWorkShopChangeItem983` (0xA32590) monta um vetor de registros
  5×u32 (`this+0x3C8`). Significado dos campos [?] (provável: chance da transformação / lista de tacos especiais;
  a UI tem `clubmade_text_luckychance.tga` e `clubmade_text_transform.tga`). Sem eles o cliente fica com tabela vazia
  — o 0x241 não leva dados e o 0x242 traz o typeid novo, então o servidor não depende deles.
- A referência **não lê** esses arquivos: sorteio fixo (Channel.cs:11396) `{0x1000005D: 250, 0x1000005E: 250,
  0x1000005F: 250, nada: 10 500}` → **2,22 % cada, 6,67 % no total**. Fallback recomendado: o mesmo, configurável.

---

## 5. Pacotes

### 5.1 Tabela C->S / S->C

C->S conferido em `pacotes_envio_983.tsv` (senders em `realmyroommain.cpp` 0x4CE120…0x4CE440) **[EXE ✓]**.
S->C: todos no despachante comum; cada um lê `u32 código`; ≠0 → caixa de erro genérica (msg 0x27 com o código);
=0 → a janela da oficina lê o resto. Leituras extras conferidas nos tratadores **[EXE ✓]**.

| C->S | layout (exe) | resposta OK (ordem) | S->C layout (exe) | erro base |
|---|---|---|---|---|
| **0x164** subir nível | `u32 typeid material, u16 qtd, i32 id club` (E4 E2 E4) | 0x216 (material, tipo 2) → **0x23D** | `u32 0, u32 stat` (→0x4EA850) | 0x5300200 |
| **0x165** confirmar nível | vazio | 0x216 (club 0xCC) → **0x23E** | `u32 0, u32 stat, i32 id club` (→0x4EB130) | 0x5300300 |
| **0x166** cancelar nível | vazio | 0x216 (club 0xCC) → **0x23F** | `u32 0, i32 id club` (→0x4EC6D0) | 0x5300250 |
| **0x167** subir rank | `u32 typeid card, u16 qtd, i32 id club` (E4 E2 E4) | 0x216 (card + club 0xCC) → **0x240** *ou* **0x241** | 0x240: `u32 0, u32 stat, i32 id` (→0x4C8540); 0x241: **vazio** (→0x4C6470) | 0x5300350 |
| **0x168** transformar: sim | vazio | 0x216 (club velho −1, club novo) → **0x242** | `u32 0, u32 typeid novo, i32 id novo` (→0x4C64C0) | 0x5300450 |
| **0x169** transformar: não | vazio | **0x243** | `u32 0, u32 stat, i32 id club` (→0x4C7360) | 0x5300400 |
| **0x16B** recuperar pontos | `u32 typeid item, i32 id club` (E4 E4) | 0x216 (item + club 0xCC) → **0x246** | `u32 0` (→0x4C3DD0, não lê mais) | 0x5300150 |
| **0x16C** transferir maestria | `u32 typeid chip, i32 id origem, i32 id destino, u32 qtd` (E4×4) | 0x216 (chip + origem 0xCC + destino 0xCC) → **0x245** | `u32 0` (→0x4C3E20) | 0x5300100 |
| **0x16D** resetar | `u32 typeid pó, i32 id club` (E4 E4) | [0xC8 se devolveu pang] → 0x216 (pó, club 0xCC, club 0xC9) → **0x247** | `u32 0, u32 typeid club, i32 id club` (→0x49BC40) | 0x5300500 |
| **0x172** evento da oficina | vazio (ícone do topo, `topiconmanager.cpp`) | **0x24E** | `u32 código` + resto lido pelo script (msg 0x753C) [?] | — |
| **0x173** contagem do evento | `u32` (só console/`textout.cpp`) | **0x24B** | `u32 código` e **nada mais** (só no `CLobbyTask`, 0xAC51A5) | — |
| 0x16A | `u8` (só no console) | — | — | — |

Notas:
- **0x244** (a ref manda `u32 0` no fim de toda partida) **não tem tratador** no 983 (ausente do `pacotes_recebe`) → não mandar.
- **0x24D/0x24E/0x24F** caem no **mesmo caso** do exe (0x9D5FEA): fecham a janela de espera, leem `u32 código` e postam o
  evento de UI 0x753C com índice 0/1/2 (+ o pacote) — o resto é lido pelo script da UI (`workshop.lua`?) [?]. A ref manda
  **0x24F `u32 0`** no fim de toda partida (VS, Tourney, GP, GB, Approach, GZ) — "progresso do evento".
- 0x24E da ref (`packet_func_sv.pacote24E`): `i32 0, i32 3000 (buracos por fase), i32 0 (buracos jogados), u8 100
  (=3000/30, máx da barra), u8 0 (barra atual), u8 10 (barraMax), u8 10` = 16 B. [REF]; além do `u32` inicial nada
  conferido. 0x24B da ref: `i32 0, 16 × u8 (1..16)` — o 983 só lê o `u32` (sobra ignorada). A ref nem registra o 0x173.
- Stubs atuais (`JpTranslator.JpOnly.cs:52-54`) mandam `u32 Refused, u32 0, u32 0`: o cliente só lê o código → ok.
  0x172/0x173 hoje silenciados: **responder 0x24E `u32 0` + 16 B da ref** e **0x24B `u32 0`** [?] (sem isso a janela de
  espera do ícone pode não fechar).

### 5.2 0x216 — item com a oficina **[EXE ✓]** (leitor do item `0x9BFC90`)
`u32 hora_unix, u32 n`, depois por item:

| off | tipo | campo |
|---|---|---|
| 0 | u8 | tipo (2 = quantidade, 0xC9 = stats `c`, 0xCC = oficina, 0xCD/0xCE ver abaixo) |
| 1 | u32 | typeid |
| 5 | i32 | id |
| 9 | u32 | flag de tempo |
| 13 | u32 | qtd antes |
| 17 | u32 | qtd depois |
| 21 | i32 | qtd (diferença) ou tempo |
| 25 | i16[5] | `c` (upgrades no 0xC9; zeros nos outros) |
| 35 | str | u16 0 (UCC idx) |
| 37 | u32, u32, u32, u8 | (13 B; zeros) |
| **50** | **i16[5]** | **ws.c** — *só se* `(typeid & 0xFC000000) == 0x10000000` **e** tipo == 0xCC |
| 60 | u32 | **mastery** |
| 64 | u8 | **level** |
| 65 | u32 | **rank** |
| 69 | u32 | **recovery** |

= 50 B por item, **73 B** com a oficina. O exe também lê extras para `tipo 0xCD` com typeid de personagem
(`typeid & 0xFC000000 == 0x04000000`: +u32) e `tipo 0xCE` com caddie (`== 0x1C000000`: +u8 +u32) — úteis para a
maestria de personagem e o caddie.

Na referência: material/consumível = tipo 2 com `qtd antes/depois/−qtd`; club com oficina = tipo 0xCC com qtd zerada;
no reset o club vai **duas vezes** (0xCC com a oficina zerada e 0xC9 com `c` = upgrades zerados).

### 5.3 Códigos de erro [REF] (`0x53` + 5 dígitos; o cliente só mostra a caixa genérica)

| pacote | código → motivo |
|---|---|
| 0x245 (transferir) | 101 club origem/destino não tem · 102 club não está no IFF · 103 destino `tipo −1` · 104 sem chip · 105 chips insuficientes · 106 chips demais para a maestria · 107 falha ao remover chip · 108 destino já é rank S |
| 0x246 (recuperar) | 151 sem o item · 152 qtd 0 · 153 sem o club · 154 club fora do IFF · 155 `tipo −1` · 156 falha ao remover · 157 `recovery == 0` (nada a recuperar) |
| 0x23D (subir nível) | 201 material de grupo desconhecido · 202 não tem o material · 203 qtd insuficiente · 204 material fora do IFF · 205 sem o club · 206 club fora do IFF · 207 `tipo −1` · 208 falha ao remover · 209 sem Limit/Prob · 210 club com rank −1 · 211 sem linha do rank |
| 0x23F (cancelar) | 251 club pendente sumiu · 252 stat pendente > 4 · 253 club fora do IFF · 254 `recovery ≥ total_recovery` |
| 0x23E (confirmar) | 301 sem o club pendente · 302 club fora do IFF · 303 stat > 4 |
| 0x240 (subir rank) | 351 sem o card · **0x5300532** qtd de card insuficiente (typo da ref) · 353 sem o club · 354 fora do IFF · 355 `tipo −1` · 209/211 como acima · 356 qtd de card > 4 · 357 falha ao remover card · 358 stat já no limite do rank novo · 359 sem RankUpExp · 360 maestria insuficiente · 361 rank desconhecido · 362 `rank_s_stat > 4` · 363–365 família/rank do especial não achados |
| 0x243 / 0x242 | 401 sem o club · 402 fora do IFF · 403 stat > 4 / 451 sem o club · 452 fora do IFF · 453 especial fora do IFF · 454 falha ao remover · 455 falha ao criar · 456 falha ao adicionar |
| 0x247 (reset) | 501 sem o pó · 502 qtd 0 · 503 sem o club · 504 fora do IFF · 505 sem RankUpExp · 506 typeid do pó inválido / rank −1 · 507 falha ao remover |

---

## 6. Fluxos (comportamento da referência + ajustes recomendados)

### 6.1 Subir nível — 0x164 → 0x165 | 0x166
1. Material: grupo **Item** (qtd ≥ `qtd`) ou **Card** (`qtd` cards). Com card (Abbot Elemental Shard) a `qtd` também
   escolhe o stat favorecido e o bônus: `qtd 1→impacto, 2→curva, 3→força, 4→spin, 5→controle` (outro → impacto),
   `bônus = qtd × 200` no peso desse stat [REF] (interpretação da qtd como seletor da UI: [?]).
2. Club precisa existir, estar no IFF, `tipo ≠ −1`. Linha do Limit = `(tipo, rank atual)`.
3. Sorteio ponderado entre os stats com `Slot[i] + ws.c[i] < limite[i]`, peso `Prob[tipo][i]` (+ bônus).
   **Bug da ref**: se nenhum stat é elegível, usa o stat 0 mesmo assim → **recusar** (210/211).
4. Gasta o material, `ws.c[stat]++` **já grava** (flag F_UP_LEVEL=2) e guarda na sessão `{club_id, stat}`.
5. Manda 0x216 (só o material) e **0x23D `0, stat`**. O cliente mostra o resultado e pergunta.
6. **0x165**: nada muda (já gravado); 0x216 com o club 0xCC + **0x23E `0, stat, id`**; conquista `0x6C4000A2`.
7. **0x166**: exige `recovery < total_recovery`; `ws.c[stat]--`, `recovery++`, grava (flag 3); 0x216 club 0xCC +
   **0x23F `0, id`**. Se o cliente cair entre 0x164 e a resposta, o +1 fica (é o "confirmar" implícito).
- Recomendado: recusar se `nível == 4` (o próximo passo é rank-up) [?]; limpar o pendente depois de 0x165/0x166.

### 6.2 Subir rank — 0x167 → 0x240 | (0x241 → 0x168 | 0x169)
1. `qtd` de cards (0..4; >4 → 356) escolhe o stat: **`0→impacto, 1→curva, 2→força, 3→spin, 4→controle`** (sem card =
   impacto). Card não é obrigatório (qtd 0).
2. `rank novo = rank atual + 1`; linha do Limit `(tipo, rank novo)`; o stat precisa ter `Slot+ws.c < limite`.
3. `mastery ≥ RankUpExp[tipo_rank_s][rank novo]` (senão 360).
4. Gasta os cards; se rank novo == 5: `ws.c[rank_s_stat]++` (bônus S); `ws.c[stat]++`; `recovery = 0`;
   `rank = rank novo`; `level = calcLevel`; `mastery −= custo`; grava (flag 4); 0x216 (card + club 0xCC).
5. Se `flag_transformar == 1`: sorteio §4.5. Se saiu um especial: acha na família (`text_pangya` igual) a versão com
   `rank(Slot) == rank novo`; se o jogador **não tem** esse typeid: guarda `{club_id, stat, typeid}` e manda **0x241
   (vazio)** — sem 0x240. Se já tem (ou não saiu): segue normal.
6. Normal: **0x240 `0, stat, id`**; conquistas `0x6C4000A3` (+`0x6C4000A7` se S).
7. **0x168**: remove o club (tipo 2, −1), cria o especial (`id` novo, oficina zerada: `level = tipo` do especial,
   mastery 0, pcl 0) → 0x216 (os dois) + **0x242 `0, typeid novo, id novo`**; conquista `0x6C4000A4`.
   **Atenção [REF]**: perde a maestria restante e os upgrades com pang do club velho. [?] melhor copiar `mastery`.
8. **0x169**: nada muda (o rank-up já valeu) → **0x243 `0, stat, id`**; conquista `0x6C4000A3`.

### 6.3 Recuperar — 0x16B
Item qtd ≥1, club `tipo ≠ −1`, `recovery > 0`. Gasta 1 → `recovery = 0` → grava (flag 1) → 0x216 (item + club 0xCC)
→ **0x246 `0`**; conquista `0x6C4000A6`.

### 6.4 Transferir — 0x16C
`chip`, `origem`, `destino`, `qtd`. Chip qtd ≥ `qtd`; ambos no inventário e no IFF; destino `tipo ≠ −1` e rank ≠ 5.
`mastery = min(qtd × 300, origem.mastery)`; `destino += mastery`, `origem −= mastery`; gasta `qtd` chips; 0x216 (chip,
origem 0xCC, destino 0xCC) → **0x245 `0`**; conquista `0x6C4000A5`.
- A checagem "chips demais" da ref (`qtd×300 > mastery && ⌈mastery/300⌉ > qtd`) nunca dispara; recomendado recusar se
  `qtd > ⌈origem.mastery/300⌉` (106).
- **Bug da ref**: grava só a origem no banco (o destino perde a maestria no próximo login). Gravar os dois na mesma transação.
- Origem `tipo −1` é permitida (é assim que a maestria do Air Knight Set é aproveitada).

### 6.5 Resetar — 0x16D
Só `0x1A000247` (Soft) ou `0x1A00024B` (Hard). Club no IFF com rank base e atual válidos.
- Hard: `devolve_m = ⌊0,5 × Σ RankUpExp[tipo_rank_s][r] para r = rank_base+1 .. rank⌋` (rank_base = rank do `Slot` do IFF)
  e `devolve_p = ⌊0,5 × Σ preço Enchant de cada upgrade com pang (stat i, níveis 0..pcl[i]−1)⌋`;
  `mastery += devolve_m`; pang += devolve_p e manda **0xC8 `u64 pang, i64 devolvido`**.
- Ambos: gasta 1 pó; `ws.c = 0`, `level = 0`, `rank = 0`, `recovery = 0` **e `pcl` (upgrades com pang) = 0**; a
  maestria fica. Grava oficina (flag 5) e stats; 0x216 (pó, club 0xCC, club 0xC9 com `c` zerado) → **0x247 `0, typeid, id`**.
- Recomendado [?]: gravar `rank`/`level` calculados do `Slot` base em vez de 0.

### 6.6 Evento — 0x172/0x173
Sem lógica na ref (valores fixos). Mínimo: 0x24E como §5.1 e 0x24B `u32 0`. Opcional: 0x24F `u32 0` no fim da partida
(o 983 trata; hoje só o Grand Zodiac manda).

---

## 7. Como o cliente vê nível/rank/maestria

### 7.1 Inventário do login — WarehouseItem JP (196 B) **[REF]**, fim do item
`KrToJp.Item` hoje escreve **`Zeros(28)`** no fim. Layout da ref (`WarehouseItem.ToArray`, pangya_game_st.cs:5660):

| off | tipo | campo |
|---|---|---|
| 168 | i16 | `flag` (do banco; sempre 0 na prática) [?] |
| 170 | i16[5] | **ws.c** |
| 180 | u32 | **mastery** |
| 184 | u32 | **recovery** |
| 188 | i32 | **level** |
| 192 | i32 | **rank** |

(Atenção: ordem **mastery, recovery, level, rank** aqui; no 0x216 é **mastery, level(u8), rank, recovery**.)
O cliente calcula rank/nível sozinho a partir de `Slot` + `ws.c` (§0) — o `rank`/`level` do item são informativos [?].

### 7.2 ClubSetInfo dos equipados (28 B) [?] — divergência
- Ref (`ClubSetInfo.ToArray`, LoginManager.cs:398-406): `i32 id, u32 typeid, i16[5] = pcl (upgrades com pang),
  i16[5] = Slot do IFF + ws.c (teto)`.
- Nosso `JpStructs.EquipedItems` (:150): `i32 id, u32 typeid, Zeros(10), i16[5] pcl`.
- Um dos dois está trocado. Conferir no cliente: club com upgrades e com oficina → ver se a barra de slots/stats do
  equipado bate. Recomendo seguir a ref (`pcl` primeiro, `Slot+ws.c` depois) e testar.

### 7.3 `level` — semântica confusa na ref [?]
Ao criar o club a ref grava `level = work_shop.tipo` ("Cv 1 e etc"; −1 nos que não evoluem); no rank-up passa a
`calcLevel` (0..4); subir nível não mexe nele; reset zera. Recomendado: mandar **sempre calculado** (`(soma−30)%5`,
ou −1/0xFF se `tipo == −1`) e conferir no cliente o que a UI mostra (`clubmade_remodel_clubtype1.tga` sugere "tipo").

---

## 8. Persistência (por club set)

| dado | onde (proposta) | obs. |
|---|---|---|
| `ws.c[5]` | `items.attrs.ws.c` (jsonb) | i16 |
| `mastery` | `items.attrs.ws.m` | u32 |
| `recovery` | `items.attrs.ws.r` | u32 |
| `level`, `rank` | **derivados** (não gravar) ou `ws.lv`/`ws.rk` se o cliente precisar do valor "tipo" | §7.3 |
| `pcl[5]` | já existe (`attrs.pcl`) | zerado pelo reset |
| pendente do nível `{club_id, stat}` e da transformação `{club_id, stat, typeid}` | **memória da sessão** | a ref não persiste |

Sem migração: `items.attrs jsonb` (`003_players.sql:22`) já guarda atributos. Gravações em uma transação por pedido
(`PlayerChanges` com os itens alterados + pang).

---

## 9. Divergências/bugs da referência (não copiar)
1. LevelUpProb lida com 6 u32 (são 5) e RankUpExp com 5 (são 6) — o índice S estoura.
2. Subir nível sem stat elegível sobe o stat 0.
3. Transferir grava só a origem.
4. Checagem de "chips demais" impossível de disparar.
5. Subir nível não confere `nível == 4` (rank sobe sem gastar maestria) [?].
6. Transformação descarta maestria e upgrades do club velho.
7. 0x173 não registrado; 0x244 mandado mas o 983 não trata.
8. Erro 0x5300532 (typo de 0x5300352).
9. Maestria creditada ao 1º club com o typeid (não pelo id).

---

## 10. Plano de implementação neste repositório

### 10.1 Dados (JP) — `src/Pangya.Protocol.JP983/Jp983GameData.cs`
- Ler `ClubSet.iff` 0xD0/0xDA/0xE4..0x100 num `Dictionary<int, ClubWorkshopInfo>` (tipo, rankSStat, totalRecovery,
  rate, tipoRankS, transform, family, attr[5], slot[5]); `LevelUpProb` → `Dictionary<int,int[5]>`; `LevelUpLimit` →
  `Dictionary<(tipo,rank), int[5]>`; `RankUpExp` → `Dictionary<int,int[6]>`; família → `Dictionary<int family,
  Dictionary<int rank, int typeid>>`. Sem LINQ em `src/` (regra 13).
- Expor por uma interface do Domain (`IClubWorkshopData`) implementada pelo Jp983GameData; o KR devolve null (sem oficina).
- Testes de dados (`Jp983DataTests`): Prob tipo 0 = 500,200,800,500,600; Limit (4,4) = 22,14,18,9,9; RankUpExp tipo 3 =
  0,1200,5300,17600,35000,90000; Air Knight Set tipo −1/rate 1.5; família 93 tem 5 ranks.

### 10.2 Domain — `src/Pangya.Domain/Players/ClubWorkshop.cs` (novo)
- `ClubWorkshop` (estática): `Read(Item)`/`Write(Item, …)` sobre `attrs.ws`; `Rank(slot, c)`, `Level(slot, c)` (tabela
  30..59); `SlotCap(stat)`.
- `ClubWorkshopService(IPlayerStore store, IClubWorkshopData data, Random rng, WorkshopConfig cfg)` com métodos que
  devolvem `(uint Code, …)` e as `ItemChange`s: `UpLevelAsync`, `ConfirmLevel`, `CancelLevelAsync`, `UpRankAsync`
  (→ resultado `Ok | Offer(typeid)`), `TransformAsync(bool confirm)`, `RecoverAsync`, `TransferAsync`, `ResetAsync`,
  `AddMasteryAsync(Player, clubId, holes, mode, rateClub)`. Estado pendente num objeto por conexão (`WorkshopSession`).
  Códigos = §5.3. Grava antes de mudar a memória (padrão do `PlayerActions`).
- `PlayerActions.UpgradeAsync`: `cap += ws.c[stat]` (e o mesmo no `Jp983GameData.PlayStats`? não — ws.c é slot, não stat).
- Maestria: `MasteryPerHole(mode, rate, srvRate, clubRate)` pura (§1.1) + config `Rewards.ClubMasteryRate` (100).
  Bônus: inventário com 0x1A000338 (+200), anel 0x70210009 equipado (+10), cards com efeito 34.
  Decisões a tomar: contar **prática** (ref conta) e **quem sai** (ref conta os buracos já jogados; hoje `Rewards` dá 0).

### 10.3 Protocolo JP — `src/Pangya.Protocol.JP983/Game/JpSystems.Workshop.cs` (novo, padrão do JpSystems)
- Ids `0x164–0x169, 0x16B–0x16D, 0x172, 0x173` em `JpSystems.Ids`; tirar os stubs de `JpTranslator.JpOnly.cs:52-59`.
- `ItemsChanged` ganha a variante 0xCC: depois dos 50 B, `i16[5] ws.c, u32 mastery, u8 level, u32 rank, u32 recovery`
  (só para typeid de club set) e a 0xC9 (`c` = pcl).
- Respostas e ordem da §5.1 (0x216 antes da resposta; 0xC8 antes do 0x216 no reset).
- `KrToJp.Item`: em vez de `Zeros(28)`, escrever o bloco §7.1 do club (precisa do `Item`, não só dos bytes do KR:
  passar os atributos ou remendar depois de converter). Conferir `JpStructs.EquipedItems` (§7.2).
- Fim de partida: novo sink no `GameHandler` (como `Achievements`) — `ClubMastery?.OnGameEndAsync(p, holes, mode)` —
  que o JP implementa: `AddMasteryAsync` + 0x216 0xCC. Opcional: 0x24F `u32 0` depois.
- Conquistas: `0x6C4000A2..A7` via `RecordAsync` (`AchievementCounters`: novos nomes `ClubSlot`, `ClubRankUp`,
  `ClubTransform`, `ClubTransfer`, `ClubRecovery`, `ClubRankS`).

### 10.4 Banco
Nada novo (jsonb). Se quiser consulta/ranking de maestria: índice em `(attrs->'ws'->>'m')` depois.

### 10.5 Testes (`tests/Pangya.Tests`)
- Unitários do Domain (RNG fixo): rank/nível nas fronteiras 30/34/35/59/60; subir nível só entre elegíveis e recusa sem
  elegível/nível 4; cancelar até 5 e depois 254; rank-up cobra RankUpExp certo, aplica bônus S e recusa sem maestria (360);
  transformação com RNG forçado → oferta só se não possui; transferência min(qtd×300, origem) e grava os dois; reset
  hard devolve 50 % (pang via Enchant + maestria) e soft não; maestria por buraco nos modos (VS 10, SSC 15, Approach 1,
  GZ 0) e com bônus 3×/1,1×.
- `Jp983WorkshopTests` com o `Jp983Client` (cliente falso montando pelos layouts do exe): 0x164→0x216(50 B)+0x23D;
  0x165→0x216(73 B)+0x23E; 0x167 com transformação→0x241 vazio→0x168→0x242; 0x16D Hard→0xC8+0x216(3 itens)+0x247;
  0x172→0x24E; login com club da oficina → bytes 168..195 do item 196.
- No cliente real (confirmação do usuário): abrir a oficina, subir nível/cancelar, rank-up, reset; ver a barra de
  slots do club equipado (§7.2) e o que aparece no lugar do `level` (§7.3).
