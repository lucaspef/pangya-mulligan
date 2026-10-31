# JP 983: missão diária, Point/Tiki Shop, eventos e pedidos C->S que ainda não tratamos

12/10/2026. Fontes:
- **[EXE ✓]** = conferido no `<pasta do jogo>\ProjectG.exe` (capstone, ImageBase 0x400000; leitores `0xA4DEB0` u8,
  `0xA4DEC0` u16, `0xA4DED0` u32, `0xA4DF00` bloco; despachante `0x009C20A0`) ou nas tabelas `docs/server/jp983-protocolo/*.tsv`.
- **[REF]** = servidor de referência da comunidade (`<ref>/Server/JP/GameServer`, AGPL: só comportamento; ids já do JP).
- **[IFF ✓]** = lido no `data/pangya_jp.iff` (o mesmo que `Jp983GameData` carrega).
- **[?]** = não conferido.
Convenção: `u8/u16/u32/i32/u64` little-endian; `str` = u16 tamanho + bytes (Shift-JIS); "0x216" = atualização de itens do JP
(`u32 hora_unix, u32 n, n × 50 B`, ver `sistemas-jp-modos.md` §B). Já cobertos por outros specs e fora daqui: oficina de
taco, cards/maestria, guilda, Grand Prix, conquistas (só o que a missão diária usa delas).

---

## 0. Resumo e prioridades

| # | sistema | o cliente 983 tem a tela? | estado hoje | vale? | tamanho |
|---|---|---|---|---|---|
| 1 | **Missão diária** (0x151–0x154) | sim (ícone "OneDayQuest" e botão Quest na lista de salas) | abre vazia (0x225 sem quests); aceitar/prêmio/desistir recusam | **sim, alta**: usa os contadores das conquistas que já temos | M |
| 2 | **Point Shop** (Legacy Tiki, 0x126–0x129) | sim (janela 0x2E0) | **0x1E7 manda 1 → a janela nem abre** (mostra erro) | **sim, alta** (correção de 1 linha + lógica pequena) | P/M |
| 3 | **Tiki Shop novo** (0x18D) | sim | recusa | sim, média: dá TP, que paga maestria (CharacterMastery) e 4 receitas da Caixa Mágica | M |
| 4 | Recarga de cometa (0xEC → 0x197) | sim (`S5::FrSupplyPackDlg`) | descartado (JP 0xEC vira KR 0xEE, que ninguém trata) | sim, baixa/fácil | P |
| 5 | Premium ticket (0x9A/0x26D) | sim (item na loja: 0x1A100002/3) | nada | média: depende de regras de jogo (time booster, auto command, GP sem ticket) | M |
| 6 | Avisos 0x1D3 (Golden Time, GZ, aviso simples) | sim | nada | baixa (cosmético) | P |
| 7 | Golden Time / Bot GM Event / evento do GZ | sim (só avisos + sala) | nada | baixa: precisa de agendador, correio e (GZ) da lógica do GZ | M/G |
| 8 | Prêmio de login | não tem tela (vai pelo correio) | parcial (presença diária já dá itens pelo correio) | baixa: config + correio | P |
| 9 | Assist (0x184/0x185) e efeitos de anel/asa/luva (0x138…0x199) | sim (sala e partida) | **descartados** (fora das faixas do tradutor) | média (o botão Assist da sala não responde) | M |
| 10 | Caixa Mágica (Cadie Cauldron 0x158 → 0x22F) | sim | **feito** (0x158 → KR 0x7E → 0xED → 0x22F, dados do JP) | — | — |
| 11 | Treasure Hunter (0x131–0x134) | sim | **feito pelo KR** (só id) | — | — |
| 12 | World Tour, aniversário, missão de evento, bingo, Valentine, point event | diálogos existem | descartados | não (eventos de época, sem dados) | — |
| 13 | Drop por buraco, moedas/caixas no campo, missões do approach | sim | nada / mínimo | baixa: dependem de tabelas que o cliente não tem (banco da referência) | G |

Correções imediatas (1 linha cada, `JpTranslator.JpOnly.cs`):
1. **0x126 → 0x1E7 `u32 0`** (hoje `1`). [EXE ✓ 0x9D2DE4] o caso lê `u32`; **≠0** monta um texto e mostra mensagem 0x23
   (erro), **=0** abre a janela 0x2E0 (Point Shop). Com `1` a Point Shop nunca abre.
2. **0xEC (recarga de cometa)**: hoje cai em KR 0xEE (sem handler) e o diálogo fica esperando; responder ao menos
   `0x197 u8 0` + 12 zeros (recusa; ver §4.2).

---

## 1. Missão diária (Daily Quest)

### 1.1 Dados [IFF ✓]
`QuestItem.iff` (registro 0x110 = 272 B; 257 linhas) — layout (`PangyaAPI.IFF.JP/Models/Data/QuestItem.cs`, conferido nos dados):

| off | campo |
|---|---|
| 0x00 | u32 ativo, u32 typeid (0x78xxxxxx), char[64] nome, … (cabeçalho comum de 0xC0) |
| 0xC0 | u32 ? (sempre 0) |
| 0xC4 | u32 **tipo** = dificuldade (1 fácil, 2 média, 3 difícil) |
| 0xC8 | u32 nº de etapas (sempre 1 nas diárias) |
| 0xCC | u32[10] typeids de QuestStuff (0x748xxxxx) |
| 0xF4 | u32[2] **prêmio** typeid, 0xFC u32[2] quantidade, 0x104 u32[2] tempo (dias; 0 nas diárias) |
| 0x10C | u32 ? (0) |

`QuestStuff.iff` (0x10C): 0xC0 u32[5] contadores (0x6C4000xx), 0xD4 i32[5] quantidades (negativa = "no máximo"), 0xE8 u32[3]
prêmios, 0xF4 u32[3] qtd, 0x100 u32[3] tempo. **Nas diárias o prêmio do QuestStuff é vazio: o prêmio vem do QuestItem.**
(Já lido por `Jp983GameData` para as conquistas; falta ler o QuestItem.)

Quais são diárias (regra da referência, `DailyQuestManager.updateDailyQuest`): `((typeid & 0xFFFFFF) >> 16) < 0x40`, isto é,
**0x78000000–0x783FFFFF**. Contagem no JP 983: **101 fáceis, 97 médias, 46 difíceis** (244). As 13 restantes (0x7840xxxx,
0x78800001) não são sorteadas: 0x7840000x = "limpe N quests" (nenhum código da referência usa) e **0x78800001 "Clear quests
10 times"** (contador 0x6C400039 × 10; prêmio 1 × 0x1A0001C4 "Daily Quest Box"), que a referência cria como conquista
permanente no jogador novo.

Exemplos [IFF ✓]:
- 0x78000004 (fácil) "-5 ou melhor": QuestStuff 0x74800001 = {GameComplete ×1, NormalGame ×1, Score ×−5}; prêmio 0x1A000011
  (Time Booster) × 50.
- 0x7800000E (média) "36 buracos": {HolesPlayed ×36, NormalGame ×1}; prêmio 0x1A000040 (Auto Caliper) × 100.
- 0x78000051 (difícil) "63 buracos": {HolesPlayed ×63, NormalGame}; prêmio 0x18000006 × 20.

**Contadores que as 244 diárias usam** (principal = 1º do QuestStuff; condições = os outros) comparados com o que
`AchievementCounters`/`GameHandler.GameEnd` já incrementam:
- já temos: GameComplete (144 quests), Pangya, HolesPlayed, Yards, Albatross, HIO, RoomMaster/GameStart, personagens; nas
  condições: NormalGame (223), HolesPlayed, Match/Stroke/Tourney, todos os campos, Score, PangEarned.
- **faltam** (nenhum código incrementa): Birdie 0x6C400009 (9 quests), Eagle 0x6C400008 (6), Field Coin 0x6C400037 (8),
  Spin Cube 0x6C400036 (5), Spincube Open 0x6C400054 (5), Cardpack open 0x6C400078 (5), Club Master Transfer 0x6C4000A5 (5),
  Long Putt 20/25/30 y 0x6C400033/34/35 (8), Grand Prix classe 1/2/3 0x6C4000AB/AC/AD (6), condição Grand Prix 0x6C4000A9 (6).
  Os de buraco (birdie, eagle, long putt) cabem no saco do fim da partida (`GameStats` tem os totais? [?]); os de moeda/cubo
  dependem do sistema de moedas (§4.9) e ficam impossíveis até lá → **excluir do sorteio as quests cujo contador principal
  ninguém incrementa** (lista acima), senão o jogador recebe uma missão impossível.

### 1.2 Escolha das 3 quests e virada do dia [REF]
- Um registro global por dia (`DailyQuestInfo`: data + 3 typeids): no 1º acesso do dia sorteia **uma fácil, uma média,
  uma difícil** (uniforme em cada grupo). Todos os jogadores do dia recebem as mesmas 3.
- Virada: data local do servidor mudou (meia-noite). No **0x151** do jogador, se a data das quests dele (`current_date`) é de
  um dia anterior: remove as quests **ainda não aceitas** (status 1 = pendente) e cria as 3 novas como pendentes; as já
  aceitas (status 3) **continuam** até pegar o prêmio ou desistir.
- Proposta para nós (sem tabela global): typeids do dia = sorteio com semente `aaaammdd` (determinístico, mesmo resultado em
  todos os servidores), restrito às quests "possíveis" (§1.1). Gravar por jogador: data, 3 typeids, data do último aceite e,
  por quest, status + valor do contador.

### 1.3 Ids (o cliente identifica a quest pelo id, não pelo typeid)
A referência usa o id da linha no banco. Nós não temos linhas por conquista (ids = índice + 1, contadores = (índice + 1) ×
16 + slot, `AchievementService.CounterId`). Proposta sem colisão: **quest diária id = 100000 + dia × 4 + slot (1..3)**
(`dia` = dias desde 2020-01-01), **contador = id × 16 + 1**; a "Clear 10" fixa em id 99999, contador 99999 × 16 + 1. Cabe em
i32 e não colide com as conquistas (≤ ~3300).

### 1.4 Pacotes
Pedidos do cliente [EXE ✓ `pacotes_envio_983.tsv`, `pk_cmp.tsv`]: **0x151** vazio (`EventIcon_OneDayQuest983`,
`FrRoomListDlg::OnQuestBtnUp`, ícones do topo); **0x152** aceitar e **0x154** desistir `i32 n, n × i32 id` (`E4 E4 ED`);
**0x153** pegar prêmio `i32 n, i32 id` (`E4 E4`, lobbymain). Respostas [EXE ✓ 0x9D5116/51A8/523A/52CC]: as quatro leem
`u32 código`; **≠0 → mensagem 0x27 com o código**; =0 → o pacote inteiro vai para a janela 0x753A (a mesma das conquistas: o
0x220 também vai para 0x753A) com o subcódigo 2 (0x225), 3 (0x226), 5 (0x227), 6 (0x228). O resto do layout é lido lá
dentro (não seguido) → corpo [REF].

| passo | S->C (ordem) | layout exato [REF `packet_func_sv.cs:7746-7870`, `DailyQuestManager.cs`] |
|---|---|---|
| abrir (0x151) | **0x216** | dia novo: n = nº de quests criadas; por quest `u8 2, u32 typeid da quest (0x78…), i32 id, u32 0, i32 0 (antes), i32 1 (depois), i32 1, 25 × 0`. Mesmo dia: `u32 agora, u32 0` |
| | **0x225** | `i32 0, u32 data_atual (unix UTC), u32 data_aceite (unix UTC, 0 = nunca), u32 3 (nº de quests; fixo na ref.), u32[3] typeids, i32 nr, nr × i32 id removido` (as pendentes do dia anterior). Erro: só `i32 código` |
| aceitar (0x152) | **0x216** | por contador criado: `u8 2, u32 typeid do contador (0x6C…), i32 id do contador, u32 0, i32 0, i32 0, i32 0, 25 × 0` |
| | **0x226** | `i32 0, i32 n, n × AchInfo`; AchInfo = `u8 1 (ativo), u32 typeid da quest, i32 id, i32 status (3), u32 nq, nq × {u32 typeid do QuestStuff, u32 typeid do contador, i32 id do contador, u32 concluída_em (0)}` (o mesmo registro do 0x21E/0x220, já escrito em `JpSystems.WriteInfo`). Erro: `i32 1, i32 0` |
| progresso | 0x216 / **0x22E** / **0x220** | o mesmo caminho das conquistas (`AchievementService.RecordAsync` → `JpSystems`): 0x216 com o contador (antes/depois), e ao fechar `0x22E u32 n, n × {u32 typeid da quest, u32 typeid do QuestStuff}` + `0x220 i32 0, u32 n, n × AchInfo` com `concluída_em ≠ 0` e status 4. **Não dá prêmio ao fechar** (o prêmio do QuestStuff é vazio); fechar uma diária soma **+1 no contador 0x6C400039** (Quest Clear), que move a "Clear 10" |
| prêmio (0x153) | **0x216** | os itens do prêmio do QuestItem (`u8 2, typeid, id, flag_tempo, antes, depois, qtd`) **e** cada contador da quest zerado: `u8 2, u32 typeid do contador, i32 id, u32 0, i32 valor, i32 0, i32 −valor, 25 × 0` |
| | **0x227** | `i32 0, i32 n, n × i32 id`. Erro: `i32 código, i32 0`. Contador 0x6C40009F +1 por quest (menos a "Clear 10"). A quest some (a "Clear 10" volta ao início: contador 0, não concluída) |
| desistir (0x154) | **0x216** | contadores zerados (como acima) |
| | **0x228** | `i32 0, i32 n, n × i32 id`. Erro: **só `i32 código`** (hoje mandamos `código, 0`: inofensivo) |

Login [REF `cLoginTask`/`AchievementManager`]: as quests do jogador (pendentes, aceitas, a "Clear 10") vão junto com as
conquistas no **0x21E** e os contadores delas no **0x21D**. Sem isso, depois de relogar o cliente não conhece os ids das
quests aceitas.

Regras de validação [REF]: `n ≤ 0` → erro (5000 aceitar, 5005 prêmio, 5010 desistir); id que não é do jogador é ignorado;
prêmio só se todas as etapas fecharam [?: a ref. não confere; conferir antes de dar]. Prêmio que não cabe (pilha cheia):
erro 1500.

O 0xB0 C->S (`FrQuestGift::OnGiftListLBtnDown`, `u32`) é da mesma janela e **nenhum servidor trata** (nem a ref.): só logar.

### 1.5 O que precisa no nosso código
1. `Jp983GameData`: ler `QuestItem.iff` (tipo, QuestStuff, prêmio) e expor as diárias por dificuldade.
2. Domain `DailyQuestService` (versão-independente): sorteio do dia, estado por jogador, aceitar/prêmio/desistir; contadores
   pelo mesmo `CounterBag` das conquistas (a diária só conta se aceita; ref.: só contador de quest com status 3 anda).
3. Banco: `daily_quest(account, day, tid1, tid2, tid3, accepted_at)` + `daily_quest_state(account, quest_id, typeid, status,
   counter)`; ou reaproveitar o `IAchievementStore` com uma chave de "conquista" = id da quest.
4. `JpSystems.DailyQuest.cs`: 0x151–0x154 no lugar das respostas mínimas do `JpOnly`; incluir as quests no 0x21E/0x21D.

---

## 2. Point Shop (Legacy Tiki) e Tiki Shop novo

### 2.1 Dados [IFF ✓]
- **Bloco Tiki no cabeçalho comum de todo item** (offset 0x84, 24 B; `IFFTikiShopData`): `u32 itens_por_lote (Tiki_Qnt_Pts),
  u32 TP_por_lote (Tiki_Pts), u16 milhagem (MP por unidade), u16 prob_bônus, i16 bônus_mín, i16 bônus_máx, u32 tipo (1 normal,
  2 cookie/CP, 3 raro), u32 pang (custo da troca)`. Item "trocável" = tipo 1..3 **e** pang > 0 **e** milhagem > 0. Exemplos:
  Bomber Comet `(35, 1, 30, 100, 1, 50, 2, 200)` (35 bolas = 1 TP; 30 MP cada); card N `(2, 1, 500, …, 3, 200)`; peça comum
  `(0, 0, 15, 100, 50, 100, 1, 200)`. Milhares de peças, ~230 cards, ~100 club sets, ~80 bolas, ~300 itens têm o bloco.
- **PointShop.iff** (0x14 = `u32 ativo, u32 typeid, u32 TP, u32 qtd, u32 flag`; 12 linhas, a "vitrine" da Point Shop):

| typeid | item | TP | qtd | flag |
|---|---|---|---|---|
| 0x1800002C | Silent Nerve Stabilizer | 30 | 5 | 0 |
| 0x1800002D | Safe Silent | 50 | 3 | 1 |
| 0x1800002F | Wind Strength Boost | 300 | 2 | 1 |
| 0x1A000136 | Fairy's Tears | 100 | 400 | 0 |
| 0x1A0000AD / 0x1A0000AE | Mochi Chocolate / Mochi Montblanc | 150 | 20 / 30 | 0 |
| 0x1A0000E1 / E2 / E3 / E4 | Rainbow / Delicious / Melon / Lemon Drop Candy | 100 / 30 / 15 / 10 | 1 / 2 / 3 / 3 | 0 |
| 0x1A00019F / 0x1A0001A0 | Auto Commander / Vector Sign | 300 / 200 | 100 / 100 | 0 |

- **TikiPointTable.iff** (0x30; 3 linhas: `u32 índice, u8 tipo, char[35] nome, u32 a, u32 b` = (1,10), (20,30), (50,100)),
  **TikiRecipe.iff** (0x34; 10 linhas, `u32 ativo, u8 tipo, char[35], u32[3]` = 5/5/1/1/1/2/2/2/3/3), **TikiSpecialTable.iff**
  (0x3C; 2 linhas, `u32, u8, char[35], u32 qtd, u32[4] typeid` = (4, 0x15B3…) e (2, 0x37, 0x37)). Só o cliente usa (faixas de
  bônus/textos da tela); **nenhum código da referência lê** → não precisamos delas no servidor [?: a TikiSpecialTable tem
  valores que não são typeids válidos].
- Itens-moeda [IFF ✓]: **0x1A0002A6 "Tiki Points (TP)"** e **0x1A0002A7 "MP Items"** (milhagem). Onde o TP é gasto no
  983: **CharacterMastery.iff** (140 linhas usam 0x1A0002A6: custo de expandir maestria, 0x187) e **4 receitas da Caixa
  Mágica** (TP × 10 → Fairy's Tears; TP × 3 → Silent Nerve Stabilizer; TP × 3 → Safe Silent; TP × 5 → Wind Strength Boost).
  Ou seja: o Tiki Shop novo alimenta maestria e Caixa Mágica (que já funciona).
- **Os "pontos" da Point Shop antiga NÃO são o item TP**: são um contador próprio do jogador (`m_legacy_tiki_pts` no banco,
  `CmdLegacyTikiShopInfo`). Precisa de uma coluna nova.

### 2.2 Point Shop antiga (Legacy) [REF `Channel.cs:16175-16637`]
| C->S | layout [EXE ✓ `E1 E4 E4 E4 E4` / `E1 E4 E4 E4`] | resposta |
|---|---|---|
| **0x126** abrir | vazio | **0x1E7** `u32 0` (≠0 = mensagem de erro) [EXE ✓] |
| **0x127** saldo | vazio | **0x1E8** `u32 0, u32 pontos` [EXE ✓ D4 D4] |
| **0x128** itens → pontos | `u8 n, n × {u32 typeid, i32 id, i32 lotes, u32 tp_mostrado}` | 0x216 (itens gastos) + **0x1E9** `u32 0, u32 pontos_novos` |
| **0x129** pontos → itens | `u8 n, n × {u32 typeid, i32 lotes, u32 tp_mostrado}` | 0x216 (itens ganhos) + **0x1EA** `u32 0, u32 pontos_novos` |

Regras:
- 0x128: por item, base = `findCommon(typeid)`; precisa ser "trocável" (§2.1) senão 0x5200905; consome
  `itens_por_lote (0→1) × lotes` unidades daquele item (caddie/mascote: não pode ser por tempo, mascote só 1; peça/club set
  não pode estar equipado nem à venda na loja pessoal: 0x5201010); ganha `TP_por_lote (0→1) × lotes`. Total 0 → 0x5200905.
  O `tp_mostrado` do cliente é ignorado (recalcular). Contador de conquista **0x6C400086 +1**.
- 0x129: por item, precisa estar na PointShop.iff (senão 0x5200902); custo `TP × lotes`, recebe `qtd × lotes`; custo >
  saldo → 0x5200906; falha ao entregar → 0x5200907. Contador 0x6C400086 +1.
- Erro: o mesmo id com `u32 código` (genérico 1). Bloqueio por GM (flag do jogador) → código 1.

### 2.3 Tiki Shop novo (0x18D) [REF `Channel.cs:9027-9298`]
- C->S **0x18D** `u32 n (1..5), n × {u32 typeid, i32 id, u32 qtd}` [EXE ✓ `E4 E4 E4 E4`].
- Por item (mesmas validações do 0x128): soma `pang += pang_do_item` (**uma vez por linha, não × qtd**), `MP += milhagem × qtd`,
  `bônus_mín/máx += do item`, `prob = prob do último item`. Contador por tipo: 0x6C4000BE (normal), BF (CP), C0 (raro).
- Bônus: sorteia `r % (prob × 3 + 1) < prob` (≈ 25 % com prob 100) → `bônus = mín + r % (máx − mín)`; contador 0x6C4000C1.
- MP: `total = MP_ganho + MP_que_tinha + bônus`; **TP ganho = total / 1000** (a ref. só converte se `> 1000`; usar `≥`);
  o item MP fica com `total % 1000`; o item TP soma o ganho (contador 0x6C4000C2 += TP).
- Gasta o pang (0xC8 `u64 pang, u64 gasto`), 0x216 (itens trocados, MP, TP), **0x274** `u32 0, u32 MP_ganho (sem bônus),
  u32 bônus` [EXE ✓ D4 D4 D4]. Erro: `0x274 u32 código` (0x5200900 genérico; 0x52000901 sem o item; 0x5200902 não existe;
  0x5200903 falha ao remover; 0x5200904 falha ao dar; 0x5200905 não trocável; 0x5201010 à venda).
- Pang insuficiente: a ref. não confere (bug); conferir antes.

### 2.4 Plano
1. `JpOnly`: 0x1E7 `u32 0` já (abre a janela com saldo 0).
2. Domain `TikiService`: dados do bloco Tiki (ler o offset 0x84 em `Jp983GameData.Common`), PointShop.iff, saldo legado
   (coluna `legacy_tiki_points` na conta), as duas trocas. Reaproveitar `PlayerActions.Free/IsBusy` (como a Caixa Mágica).
3. `JpSystems.Tiki.cs`: 0x126–0x129, 0x18D (sai do `JpOnly`).

---

## 3. Sistemas e eventos, um a um

### 3.1 Premium ticket [REF `System/PremiumSystem.cs`, `LoginManager` caso 4]
- Itens [IFF ✓]: ticket 0x1A100002 (Normal) / 0x1A100003 (VIP); bola 0x140000D8 "Premium Comet"; mascote 0x4000004C "Lolo
  (Premium)"; club set 0x10001001; caixa de presente 0x1A0002BA (4 por carta). O título 0x39800296 da ref. **não existe**
  no 983.
- Ativação: ao **receber** o ticket (compra/presente; `ItemManager.addItem` recusa um 2º enquanto vale) e no login com ticket
  válido. Manda **0x9A `i32 capability`** com o bit `0x4000` (PREMIUM_USER) [EXE ✓ 0x9CC089: grava a capability no jogador]
  + **0x26D `i32 unix do fim`** [EXE ✓ 0x9D69FF: grava o fim no jogador] + 0x216 com a bola premium (item virtual, id −1,
  tipo 0x6A, equipada como cometa). VIP: + club set e mascote por tempo.
- Ao vencer (checado a cada pedido): tira o ticket e a bola (0x216), 0x9A sem o bit, 0x26D, chat 0x40 tipo 7.
- Efeitos [REF]: Time Booster e Auto Command de graça (não consomem), GP e Bot sem ticket, Auto Caliper do VIP.
- Vale? Médio. Só faz sentido se a loja vender o ticket. Mínimo: nada (o cliente não pede).

### 3.2 Avisos 0x1D3 [EXE ✓ `D4 D4 DS`]
`u32 n, n × {u32 tipo, str XML}`. Tipos (`eBROADCAST_TYPES`): 0 escondido (`<BroadCastReservedNoticesIdx>…`), 1 cubo raro,
2 bolsa de pang, **11** Golden Time começa hoje, 12 começa a rodada, 13 muita gente, 14 ganhadores, 15 fim da rodada,
16 fim do dia, 17 fim do evento, 18 sem ganhadores, **20** texto simples (como aviso de GM), **21** Grand Zodiac começa.
XML [REF `GameService.cs:2000-2310`]: `<PARAMS><EVENTSTARTTIME>aaaa-mm-dd hh:mm:ss.mmm</EVENTSTARTTIME><TYPEID>n</TYPEID>
<QTY>n</QTY>[<NICKNAME>…</NICKNAME>…][<EVENTNEXTTIME>…</EVENTNEXTTIME>]</PARAMS>`; GZ: `<PARAMS><RunningTime>min</RunningTime>
<BroadCastReservedNoticesIdx>531</BroadCastReservedNoticesIdx></PARAMS>`; fim do GT: `…Idx>53<…`. Vai para todos os canais.
Útil já: tipo 20 como "aviso do servidor" alternativo ao 0x42.

### 3.3 Golden Time [REF `System/GoldenTimeSystem.cs`, `GameService.makeListOfPlayersToGoldenTime`]
- Evento agendado (banco: eventos ONE_DAY/INTERVAL/FOREVER, rodadas com hora, itens com peso). Cada rodada sorteia um item;
  5 min antes avisa (tipo 11), na hora avisa (12), junta quem está jogando ou no lounge, sorteia ganhadores (peso 100 + 100 se
  jogando + 100 se premium; nº = jogadores / N × taxa, mín. 1), avisa (14 com nicks), manda o item por **carta**, fogos na
  cabeça de quem está no lobby [?: pacote], e avisa fim de rodada/dia/evento (15/16/17) ou sem ganhadores (18).
- Tela: só os avisos (texto fixo do cliente por tipo). Vale: baixa; precisa de agendador global + correio JP (já existe).

### 3.4 Bot GM Event [REF `System/BotGMEventSystem.cs`, `Channel.makeBotGMEventRoom`]
- Em janelas de horário: cria uma sala **torneio**, 18 buracos, campo aleatório, modo Shuffle, 250 jogadores, 35 min,
  nome fixo; avisa com tipo 20 (2×) o canal, a duração e os prêmios (1–2 itens sorteados por peso); quem termina recebe por
  carta. Vale: baixa (depende do torneio do JP estar confirmado; ver o bug do 0x76 em massa).

### 3.5 Evento do Grand Zodiac [REF `System/GrandZodiacEvent.cs`, `Channel.makeGrandZodiacEventRoom`]
- Em janelas de horário: aviso 0x1D3 tipo 21 (uma vez) e, em cada canal, salas "HIO Event (Intermediate/Advanced)" (tipos de
  sala GZ_INT/GZ_ADV, 1 buraco, campo GZ, 100 jogadores, 7 min). Depende do modo GZ inteiro (`specs/grand-zodiac.md`; hoje só
  o Chip-in Practice). Vale: depois do GZ.

### 3.6 Caixa Mágica (Cadie Cauldron) — **já feito**
- 0x158 JP `u16 receita, u32 vezes, u8 n, n × (u32 tid, i32 id)` [EXE ✓ `E2 E4 E1 E4 E4`] → KR 0x7E (vezes limitado a 255)
  → `MagicBoxService` com a **CadieMagicBox.iff do JP** (0x8C, `Jp983GameData.MagicBox`) → KR 0xA5/0x71 (→ JP 0xA7/0x73) +
  0xED → **0x22F** `u32 0, u32 receita, u32 n, n × (u32 tid, i32 id, i32 qtd ganha, i32 total, u32 0)` [EXE ✓ D4 D4 D4]
  (`JpTranslator.Shop.MagicBoxResult`). A ref. manda 0x216 no lugar de 0xA7/0x73: equivalente.
- Com o TP do §2.3, as 4 receitas com 0x1A0002A6 passam a ser usáveis sem mudar nada.

### 3.7 Recarga de cometa (Comet Refill / Supply Pack) [REF `Channel.requestCometRefill`, `System/CometRefillSystem.cs`]
- C->S **0xEC** `u32 typeid do pacote, u32 typeid da bola` [EXE ✓ `E4 E4`, `s5/supplypack.cpp`]. Pacote 0x1A000105 "Comet
  Refill Pack" [IFF ✓]; o 0x1A00014B (no KR, 2º pacote de recarga) no JP é um item chamado "Azfuere" [?: conferir se ainda
  é pacote].
- Regra: tem a bola e ≥ 1 pacote; a quantidade vem de uma faixa min..max por pacote (**tabela do banco** da ref.; sem
  tabela no cliente) → tira 1 pacote, soma na bola.
- S->C **0x197**: `u8 1, u32 pacote, u32 bola, u32 total_da_bola` [EXE ✓ 0x9D26B9: `D1 D4 D4 D4`; a ref. escreve `u16` no fim:
  2 bytes a menos]; erro `u8 0` + 12 zeros. O cliente já desconta o pacote e grava o total (não mandar 0xA7 antes, como no KR:
  `SPEC-caixas-reciclagem.md` §0 / `SPEC-itens-restantes.md`).
- Fazer no Domain para o KR também (o KR 645 tem o mesmo diálogo: C->S 0xEE / S->C 0x19C). Faixa pela config
  (ex.: 0x1A000105 → 30..30) [?: valor oficial].

### 3.8 Caixas (Box system) [REF `System/BoxSystem.cs`]
- Sorteio ponderado de caixas cujo conteúdo vem do **banco** da ref. (`CmdBoxInfo`). No nosso servidor as caixas já existem
  no KR (`SpinCubeService`, caixas de evento): JP 0xEF (Spin Cube/caixa de papel) → KR 0xF1; JP 0xB2 (caixa do My Room) → KR
  0xAA. As respostas JP (0x19D, 0x129 + 0xAA) estão em `SPEC-jp983-loja.md` §2.10; conferir a conversão lá. Nada novo aqui.

### 3.9 Missões do approach [REF `System/ApproachMissionSystem.cs`]
- 50 % de chance por buraco de sortear uma missão de uma tabela do **banco** (`numero, box, tipo, players, condition1`); vai
  no 0x14F (`u8 nº, u8 caixas, u8 tipo, i32 condição, i32 ?, str nick`). Hoje mandamos 13 zeros (sem missão). Os textos das
  missões estão no cliente (por número), mas a tabela de números válidos não [?]. Vale: baixa.

### 3.10 World Tour [EXE ✓ `pk_cmp.tsv`] / [REF `System/WorldTourSystem.cs`]
- Cliente: `FrWorldTourEventGiftDlg::OnLBDownYesButton` manda **C->S 0xAD `u32`** (hoje vira KR 0xA5, sem handler). O que
  abre o diálogo (S->C) não foi achado [?].
- Ref.: só grava no banco "campo X concluído" (stroke/torneio de 18 buracos completos, sem short game) entre datas; **não
  responde ao 0xAD** e o laço de fim está comentado. Sem tela de progresso conhecida. Vale: não.

### 3.11 Treasure Hunter — **já feito pelo KR**
0x131–0x134 JP = KR 0x129–0x12C (+8), mesmo layout (`sistemas-jp-modos.md` C.8). A ref. tem o mesmo modelo (pontos por
buraco: HIO/albatross 100, eagle 50, birdie 30, par 15, bogey 10, …; no Special Shuffle a tabela é invertida; barras dos
campos sobem 30 min a 30 min; caixas por faixa de pontos). Só o "qtd depois" do 0x134 [?].

### 3.12 Prêmio de login [REF `System/LoginRewardSystem.cs`]
- Eventos do banco: `dias_para_prêmio`, tipo (N vezes / sempre), item, data de fim. No login: conta 1 dia por dia novo; ao
  chegar no número de dias dá o item **por carta** ("Login Reward System - <nome>") e zera (ou encerra após N vezes).
- Sem pacote próprio. Nós já temos a presença diária com correio (`AttendanceService`); um "evento de login" cabe como
  config extra do mesmo serviço. Vale: baixa.

### 3.13 Drop por buraco [REF `System/DropSystem.cs`]
- Ao terminar cada buraco: itens do campo (tabela do **banco**, por curso: tipo "todo buraco", "a cada N", "último buraco",
  probabilidades por 3/6/9/18 buracos; ×2 com "char motion", +1 com o artefato Rainbow Magic Hat, ×1,2 com asas de anjo),
  artefatos de mana (itens `ItemType == 4`), ticket do GP (100 % em 9/18 buracos até 50) e ticket do Special Shuffle; pang dos
  artefatos. Vai no 0xCC JP (`u8/i32 …, 128 × 16 B`). A tabela `HoleCupDropItem.iff` (0x2C, 87 linhas) existe no cliente e
  **não é lida pela ref.** [?: pode ser a tabela oficial do drop no copo]. Vale: baixa, depois do GP.

### 3.14 Moedas e caixas no campo (Coin cube) — ver `sistemas-jp-modos.md` C.3
Posições aprendidas pelo servidor (`CoinCubeLocationUpdateSystem`), sem tabela no cliente; contadores 0x6C400036/37/54 das
missões dependem disto. Vale: baixa/grande.

### 3.15 Assist e efeitos de itens na partida (fora do tema, mas são pedidos descartados hoje)
| C->S | o que é | resposta [REF] |
|---|---|---|
| **0x184** liga/desliga Assist (sala) | item 0x1BE00016 "Play Assist" | 0x216 (o item) + **0x26A** `u32 0, u32 0x1BE00016, u32 uid` (+ 0x16A `u32 0`?); erro `0x26A u32 código` [EXE ✓ 0x26A D4] |
| **0x185** Assist no green `u32 typeid` | | **0x26B** `u32 0, u32 typeid, u32 uid` [EXE ✓ D4] |
| **0x156** Auto Command | gasta 0x1A00019F (premium não gasta) | só erro: **0x22B** `u32 código` [EXE ✓ D4] |
| 0x138 asa `u32` | | 0x203 `u32 uid, u32 typeid` |
| 0x15C patas / 0x15D anel / 0x171 brinco / 0x180 luva / 0x181 anel no chão | efeitos de AuxPart | 0x236 / 0x237 / 0x24C / 0x265 / 0x266 |
| 0x196–0x199 anéis do JP | | 0x27E / 0x27F / 0x280 / 0x281 |
Todos ficam fora das faixas do tradutor e são descartados; nenhum trava a partida [?], mas o efeito do item não aparece para
os outros. Assunto para o spec de partida.

---

## 4. Pedidos C->S que o 983 manda e o servidor não trata

Método: ids de `pacotes_envio_983.tsv` (destino GAME/?/vazio) menos os tratados no `JpTranslator*` (`fromClient`),
`JpSystems.Ids` e os que, depois de `ClientId`, caem num `case` do KR (`GameHandler*.cs`). "→ KR x" = o tradutor repassa e o
KR não trata (só loga); "descartado" = sem par (o tradutor loga e joga fora). Função da ref. pelo despachante
(`GameService.cs:1361`).

**Missão diária / conquistas**
- 0xB0 `u32` (FrQuestGift, janela de quest) → KR 0xA8: ninguém trata (nem a ref.). Logar.

**Point/Tiki** — todos com resposta mínima no `JpOnly` (corrigir o 0x1E7).

**Loja e itens**
- 0x1E `u8, u8, u32, u32` (vender na loja, `shopmain`) → KR 0x1E: ref. não trata. [?] resposta.
- 0x9E (atualizar cupons do Gacha, `browser`) descartado → ref. `requestUpdateGachaCoupon`: **0x102** `u32 cupom, u32 cupom
  parcial, u64 pang, u64 cookie`. Fácil: `0x102 0, 0, pang, cookie`.
- 0xEC recarga de cometa → KR 0xEE: ver §3.7.
- 0xDF `u32 ×3` (FrTicketExchangeDlg) → KR 0xE1: troca de tickets, sem dados (`SPEC-caixas-reciclagem.md`).
- 0x68 `u32` (compound/downgrade) → KR 0x68: ref. não trata.
- 0xD8 `u32` (buff por tempo, `itembuffactor`) → KR 0xDA: ref. `requestUseItemBuff` → **0x181** `u32 2, u32 1, u32 typeid,
  ItemBuff 65`; erro `0x181 u32 código`. Médio (Yam/gold, rainbow).
- 0x9A `u8, i32, str` (mensagem do mascote PC bang) → KR 0x97: ref. → 0xE2 `u8, i32 id, str, u64 pang`.
- 0xEA `u32` (upgrade de caddie, `upgradecaddiedlg`) → KR 0xEC: ref. não trata.

**Avisos, web e servidor**
- 0x119 `u32 servidor` (trocar de game server, `serverdlg`) descartado → ref. **0x1D4** `u32 código, str chave` [EXE ✓ D4 DS].
  Médio: sem ele "trocar de servidor" pela lista de servidores do lobby não funciona [?].
- 0x83 `u8` (canal+lobby de outro, `serverdlg`) descartado → ref. `requestEnterOtherChannelAndLobby` (só lê).
- 0xA1 `i8` (entrou em página web), 0xA2 (saiu da guilda web) descartados → ref.: 0xA2 manda 0xC8 + atualiza guilda.
- 0x8C / 0x9E / 0xA0 / 0xA4 (`browser`) → ref. não trata (exceto 0x9E acima).
- 0x88 `16 B` (resposta GameGuard), 0x89, 0xF4 (heartbeat) descartados: inofensivo (ref. só lê; o KR tem heartbeat em 0xF6).
- 0x66 `str` / 0x67 (ticker = mensagem no letreiro) → KR 0x66/0x67 não tratados → ref. 0x66 gasta cookie e manda 0x96 +
  0x50; 0x67 → **0xCA** `u16 fila, u32 ms restantes`. Baixa.

**Eventos de época (sem dados; só logar)**
- 0x21 `str, u32` (point event, também vai ao MSN), 0x8D `u32` / 0x8E (point event) → KR 0x8A/0x8B;
- 0x6E / 0x6F `u32` (Valentine); 0xAD `u32` (World Tour / aniversário); 0x15B (aniversário); 0xE1 / 0xE2 `u32` (missão
  de evento) → KR 0xE3/0xE4; 0xE3 `u32` (bingo), 0xED (bingo do aniversário do Kooh) → KR 0xE5/0xEF; 0x192 (evento Arin 2014).

**Partida** (§3.15): 0x138, 0x156, 0x15C, 0x15D, 0x171, 0x180, 0x181, 0x184, 0x185, 0x196–0x199; e 0x12E `3 × f32`
(marcador no campo; ref. → 0x1F8 `i32 oid, f32 x, y, z` para a sala, `sistemas-jp-modos.md` C.1), 0x9D (entrar depois de
começar) → KR 0x9A, 0xCB `u32` (approach pronto) descartado.

**Oficina** (outro spec): 0x174 / 0x175 (contadores do evento da oficina) descartados.

**Console de debug** (`CConsole::Command`, só com o console do cliente ligado; ignorar): 0x14C–0x150, 0x16A, 0x170, 0x18F,
0x190, 0x191, 0x193, 0x194, 0x195.

---

## 5. Plano de implementação (ordem)

1. **Correções de 1 linha** (`JpTranslator.JpOnly.cs`): 0x1E7 `u32 0`; 0xEC → `0x197 u8 0 + 12 zeros`; 0x9E → 0x102
   `0, 0, pang, cookie`; 0xB0/0x174/0x175/console → silenciar (null) para não poluir o log. Teste: abrir a Point Shop
   (mostra 0 pontos e a vitrine da PointShop.iff).
2. **Missão diária** (§1.5), em 3 commits: (a) `Jp983GameData.QuestItems` + Domain `DailyQuestService` com testes (sorteio
   determinístico, aceitar, contador anda só se aceita, fechar → 0x6C400039, prêmio zera contadores, virada do dia);
   (b) banco (migration) + login (0x21E/0x21D com as quests); (c) `JpSystems.DailyQuest` (0x151–0x154). Antes: incluir
   no saco do fim da partida os contadores de buraco que faltam (birdie, eagle, long putt 20/25/30) e excluir do sorteio as
   quests de moeda/cubo/GP/cardpack/club master enquanto ninguém os incrementa.
3. **Point Shop + Tiki Shop** (§2.4): bloco Tiki em `Jp983GameData`, coluna de saldo legado, `TikiService`, `JpSystems.Tiki`.
   Teste: trocar 35 Bomber Comet por 1 ponto (0x128), trocar pontos por Lemon Drop (0x129); Tiki novo: trocar peças até
   1000 MP → 1 TP; usar o TP numa receita da Caixa Mágica.
4. **Recarga de cometa** (§3.7) no Domain (serve KR 0xEE/0x19C e JP 0xEC/0x197).
5. **Assist 0x184/0x185** e 0xD8 (buffs) — com o spec de partida.
6. **Premium** (§3.1), se a loja for vender o ticket.
7. Avisos 0x1D3 tipo 20 (aviso simples) e, depois do GZ completo, o evento do GZ; Golden Time / Bot GM / prêmio de login
   por último (agendador + correio).

Regras do projeto a lembrar: `src/` sem LINQ (dicionários); arquivos gerados no Windows com LF quando `.sh`.
