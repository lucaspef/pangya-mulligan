# JP 983: cards, maestria e slots de card do personagem

Pesquisa de protocolo feita em 12/10/2026. Nada no repositório foi alterado.

**Fontes e legenda**
- **[EXE ✓]**: conferido no desmonte do `<pasta do jogo>\ProjectG.exe` (capstone; leitores `0xA4DEB0` u8,
  `0xA4DEC0` u16, `0xA4DED0` u32, `0xA4DEE0` u64, `0xA4DF00` bloco(n), `0xA4E1F0` str; criação de pacote `0xA4DDE0`/`0xA4E800`;
  despachante comum `0x009C20A0`). Os endereços citados são do exe.
- **[TSV]**: `docs/server/jp983-protocolo/pacotes_envio_983.tsv` / `pacotes_recebe_983.tsv` / `pk_cmp.tsv`. As leituras desses arquivos
  vêm com **todos os ramos misturados**. Por exemplo, o `D4 D4 D1 DB(0x3a)` do 0x160 **não** é a ordem real (ver §2.3).
- **[REF]**: a referência JP da comunidade (C#, AGPL; usada só para comportamento). Arquivos: `ref/GameServer/Game/Channel.cs`
  7632–8800 e 13712–14435, `Game/System/CardSystem.cs`, `Repository/CmdCardPack.cs`, `PacketFunc/packet_func_sv.cs`
  6490–6530, `Models/pangya_game_st.cs`, `PangyaAPI.Network/Models/Pangya_St.cs` (CharacterInfo) e
  `PangyaAPI.IFF.JP/Models/Data/*.cs`.
- **[IFF]**: `data/pangya_jp.iff` (zip), lido com `<scripts locais>/iffjp/lib.py`. Os scripts deste estudo estão em
  `<scripts locais>/cards/` (`d.py` desmonta, `m1..m8.py` despejam os IFF).
- **[KR]**: o nosso código, ou seja, `GameHandler.MyRoom.cs`, `GameHandler.cs` (SendCards), `Domain/Shop/CardService.cs` e
  `Domain/Players/PlayerActions.cs`. **[TR]**: o tradutor (`JpTranslator*.cs`, `KrToJp.cs`, `JpStructs.cs`).
- **[?]**: não confirmado. **[suposição]**: escolha nossa, porque falta dado oficial.

Convenções: little-endian; `str` = u16 tamanho + bytes; SYSTEMTIME = 16 B; ids em hexadecimal. O "grupo" de um typeid é
`tid >> 26` (1 personagem, 2 peça, 4 club set, 7 caddie) e o "subtipo" de um card é `(tid >> 22) & 0xF`.

---

## 0. Resumo

1. **Muito do card do KR já serve ao JP só com a troca de id.** Isso vale para as listas do login (0x135–0x138), para abrir
   pacote (0xCA → 0x154), para o card especial (0xBD → 0x160), para encaixar pela caderneta (0xC8 → 0x160) e para o removedor
   da caderneta (0xE0 → 0x188). O tradutor já faz isso pelas faixas (+8 / −8 / +2 / −5). Os layouts batem byte a byte
   **[EXE ✓]**. O que falta nesses pacotes é **dado**, não layout: as séries dos pacotes do JP têm `Volume = 0` no Card.iff, e
   os cards NPC (subtipo 5) ficam fora do sorteio (§2.2).
2. O **0x160 bate** com o 0x158 do KR e com a referência: `u32 0` + bloco de 58 B. A dúvida antiga
   ("D4 D4 D1 DB(0x3a) não bate") vinha da mistura de ramos do TSV (§2.3).
3. **Os slots de card do personagem são um sistema novo do JP.** São 12 slots dentro do CharacterInfo 513 (bytes 465..512): 1–4
   personagem, 5–8 caddie, 9–12 NPC. Os pedidos são **0x18A** (encaixar), **0x18B** (encaixar com patcher, só nos slots 4 e 8)
   e **0x18C** (remover com removedor; o card **volta** ao inventário). A resposta é o **0x216** tipo **0xCB** mais o
   0x271/0x272/0x273. O KR não tem equivalente.
4. **Maestria (0x187)** e **stat ±1 (0x188/0x189)** também são novos. O 0x188/0x189 sai da tela de maestria **[EXE ✓]
   0x5C9D78**: `u32 stat` + CharacterInfo 513. O cliente ainda tem o caminho velho do KR, **0x4B → 0xA5** (já implementado,
   mesmo layout; manda o CharacterInfo no fim quando uma opção está ligada). A referência usa o 0x4B só para club set.
5. **Lolo (0x155)**: o pedido é `u64 pang, u32[3] typeids` **[EXE ✓ 0x486F70]**. Custa N 1000, R 2000, SR 5000 por card. SC
   é proibido. O resultado é sorteado entre os cards das séries 1–5. As respostas são 0xC8 + 0x216 + **0x229** `u32 raridade`
   + **0x22A** `u32 0, u32 typeid`.
6. **O 0x216 carrega tudo do personagem** **[EXE ✓ leitor 0x9BFC90]**:
   - tipo **0xC9**: stats `i16[5]`;
   - tipo **0xCB**: card no slot (`u32 typeid` em +45, `u8 slot` em +49);
   - tipo **0xCD**: maestria (um `u32` **extra** depois dos 50 B, só quando o typeid é de personagem).

   Uma entrada errada aborta o resto do pacote e mostra um erro no cliente (§3).

---

## 1. Onde fica no personagem JP (CharacterInfo 513)

**[REF `Pangya_St.cs` CharacterInfo; EXE ✓]**. O cliente grava a maestria em `char+0x1CD` e os cards em
`char + slot*4 + 0x1CD` (handlers do 0x216, `0x588D30` e `0x588C00`). Os stats ficam em `char+0x1C8` (`0x588BA0`).

| offset | tamanho | campo | hoje no [TR] (`KrToJp.Character`) |
|---|---|---|---|
| 0 | u32 | typeid | ok |
| 4 | i32 | id | ok |
| 8..11 | 4 × u8 | cabelo, camisa, presente, compra | ok |
| 12 | u32[24] | typeids das peças | ok |
| 108 | u32[24] | ids das peças | ok |
| 204 | 216 | Self Design | ok |
| 420 | u32[5] | anéis (AuxPart) | ok |
| 440 | u32[4] | cut-in | zeros |
| **456** | u8[5] | **PCL** (stat upgrades: força, controle, precisão, spin, curva) | ok (`k[106..110]`) |
| **461** | u32 | **maestria** (quantas etapas do CharacterMastery foram liberadas, 0..10) | **`U32(0)`** |
| **465** | u32[4] | **cards de personagem**, slots 1–4 (typeid; 0 = vazio) | **zeros** |
| **481** | u32[4] | **cards de caddie**, slots 5–8 | **zeros** |
| **497** | u32[4] | **cards NPC**, slots 9–12 | **zeros** |

A ida contrária (`CharacterToKr`, usada no 0x20 tipo 0 = trocar roupa) descarta 456..512. Isso não é problema, porque esses
campos não devem vir do cliente: o servidor guarda maestria e cards à parte (§6).

### 1.1 Como os slots são liberados (a tela de cards do personagem)

**[EXE ✓]** A tela tem **10** quadros. A função `0x5B8020` converte o quadro (1..10) no slot do protocolo:

| quadro na tela | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
|---|---|---|---|---|---|---|---|---|---|---|
| slot (protocolo) | 1 | 2 | 3 | 5 | 6 | 7 | 9 | 10 | **4** | **8** |

- **Slots 1, 2, 3 (personagem) e 5, 6 (caddie)**: livres.
- **Slots 4 (personagem) e 8 (caddie)**: os quadros 9 e 10 da tela. `0x5B8000` devolve "precisa de patcher" para quadros
  9–10, e então o cliente manda **0x18B** em vez de 0x18A (`0x5BBB59`). Cada encaixe gasta 1 **Bonus Card Patcher**
  `0x1A00018F` **[IFF; REF `CLUB_PATCHER_TYPEID`]**. A referência recusa o 0x18A nos slots 4/8 e o 0x18B fora deles.
- **Slot 7 (caddie)**: o quadro 6 só abre com uma **peça vestida que tenha `Part.iff CaddieSlot (u16 @0x22A) ≠ 0`**. O
  cliente confere em `0x6332F0` → `0x5B8010` (quadro 6). O IFF tem 231 peças assim, como orelhas e headsets "(N)/(H)". A
  referência confere a mesma coisa em `isEquipedPartSlotThirdCaddieCardSlot`.
- **Slots 9, 10 (NPC)**: aceitam card de subtipo 5. A tela não mostra os slots 11 e 12, mas a referência aceita 9–12.
- `Part.iff CharacterSlot (@0x228)` ≠ 0 em 2 peças ("Snow Black Fur Hat (A)", "Cutie Twintail (HR)"). Nem o cliente nem a
  referência usam esse campo para liberar slot **[?]**.
- **A maestria não libera slots de card.** Ela libera **slots de stat** (o limite de upgrades), §4.2.

---

## 2. Cards: pacote por pacote

### 2.1 Listas no login: 0x135 / 0x138 / 0x136 / 0x137

| JP | layout | KR | conversão |
|---|---|---|---|
| **0x135** | vazio (limpa a lista de cards) | 0x12D (`SCardsClear`) | +8, igual |
| **0x138** | `i32 0, u16 n, n × Card 58` [EXE ✓ `D4 D2 DB(0x3a)`] | 0x130 (`SCards`): `U32(0).U16(n)` + `sCards` 58 | +8, igual |
| **0x136** | vazio (limpa a lista de cards ativos) | 0x12E | +8, igual |
| **0x137** | `u16 n, n × CardEquip 65` [EXE ✓ `D2 DB(0x41)`] | 0x12F: `U16(n)` + `sSCardAvilityPeriodInfo` 65 | +8, igual |

- A ordem do KR (`GameHandler.SendCards`: 12D, 130, 12E, 12F) é a mesma da referência (`cLoginTask.cs:105–113`: 135, 138,
  136, 137).
- **Card 58** = `i32 id, u32 typeid, 12 zeros, i32 qtd (@0x14), 32 zeros, u16 1 (@0x38)`. É idêntico ao `sCards` do KR
  (`u8 type = 1` + `u8 0`).
- **CardEquip 65** = `u32 id, u32 typeid, u32 typeid da peça, u32 id da peça, u32 efeito, u32 valor do efeito, u32 slot,
  SYSTEMTIME início, SYSTEMTIME fim, u32 subtipo, u8 ativo`. É idêntico ao `sSCardAvilityPeriodInfo` do KR.
- **Já sai hoje**: `ServerId` faz +8 de 0x123 a 0x15A, e nenhum `toClient` intercepta esses ids. O conteúdo vem de
  `Jp983GameData.Cards` (Card.iff do JP), então os typeids já são do JP.
- **Falta**: os cards encaixados no personagem (§3) também entram no 0x137, com `typeid da peça = typeid do personagem`, `id
  da peça = id do personagem`, `slot = 1..12`, `subtipo = (tid>>22)&0xF` e `ativo = 1` **[REF CardEquipInfoEx]**. Guardados
  como `ItemLocation.ActiveCard` (§6), eles entram sozinhos no `PlayerStructs.ActiveCards` (e, portanto, no 0x137 e no 0x76).

### 2.2 Abrir pacote: C->S 0xCA → 0x154

**C->S 0xCA** `u32 typeid do pacote, i32 id`. Fica igual nas três telas (`cardboxdlg 0x8C1EA0`, `ccardbook 0x7F3AC0`,
`realmyroommainui 0x610AE0`) **[EXE ✓]**. O tradutor faz −8 e entrega como **KR 0xC2** (`CardOpenAsync`).

**S->C 0x154** (KR 0x14C, +8) **[EXE ✓ 0x9C9E6D]**:
```
u32 código                      0 = ok; 1 = erro (o cliente só trata 1; outro código é ignorado em silêncio)
Card 58 (o pacote)              o cliente acha pelo typeid (+4) e tira 1; a qtd do bloco é ignorada
u8  n
n × { Card 58, u32 k }          card já na lista: qtd += k; card novo: entra com a qtd do bloco
```
- O KR escreve exatamente isto: `U32(0).Struct(pacote, qtdAntes).U8(n)` + n × `{sCards(card, 1), U32(1)}`. Card repetido
  vem 2 vezes com k = 1, e o cliente soma certo. Erro: `U32(1)` ✓.
- Se a tela "card box" estiver aberta, o cliente passa o pacote inteiro para `0x8C3080`, que lê a mesma coisa.

**O que o [KR] precisa mudar para o JP** (`CardService.Draw`):
1. **Série dos pacotes do JP.** No Card.iff do JP, os pacotes (subtipo 3) têm `Volume = 0`. A regra do KR
   (`(tid & 0xFF) == 4 ? 2 : 1`) erra a partir do Vol.3. Os dados de sorteio da referência vêm de duas tabelas do banco que
   não temos (`pangya_new_card_pack`: quantidade, série, rate_N/R/SR/SC; `pangya_new_cards`: typeid, probabilidade, raridade)
   **[REF CmdCardPack]**. A tabela abaixo é uma proposta **[suposição]**:

| pacote | typeid | série (Volume dos cards) | cards |
|---|---|---|---|
| Card Pack Vol.1 | 0x7CC00000 | 1 | 3 |
| Card Pack Vol.2 | 0x7CC00004 | 2 | 3 |
| Card Pack Vol.3 | 0x7CC00005 | 3 | 3 |
| Card Pack Vol.4 | 0x7CC00007 | 4 | 3 |
| Card Pack Vol.5 | 0x7CC00012 | 5 | 3 |
| Grand Prix / estação / colaborações (0x7CC00008..1C) | | [?] (os cards de colaboração têm Volume 100) | 3 |
| Gold / Silver / Bronze Card Ticket | 0x7CC00001/02/03 | qualquer, raridade ≥ 2/1/0 (regra do KR) | 1 |
| **Platinum Card Ticket** | 0x7CC00006 | [?] (proposta: raridade 3) | 1 |
| Pangya Card Box #1..#4 (subtipo 4) | 0x7D000000..03 | Volume 1..4 (já vem do IFF) | 3 |

2. **Cards NPC (subtipo 5)**: hoje o filtro `c.SubType >= CardInfo.SubPack` os exclui. O JP tem 26 cards NPC, e 17 deles estão
   na série 4. Para incluí-los, troque o filtro por `SubType is 0 or 1 or 2 or 5`.
3. Distribuição no JP **[IFF]**:

   | subtipo | N | R | SR | SC |
   |---|---|---|---|---|
   | 0 personagem | 28 | 16 | 12 | 19 |
   | 1 caddie | 14 | 11 | 18 | 15 |
   | 2 especial | 33 | 27 | 10 | 2 |
   | 5 NPC | 6 | 8 | 6 | 6 |

   O peso do KR por raridade (70/22/7/1) continua como **[suposição]**. Na referência, o peso de cada card é
   `prob × rate[raridade]/100`.
4. Erros: a referência responde sempre `u32 1` (o comentário diz que o cliente não aceita outro código). Isso bate com o
   exe, que ignora outros códigos.

### 2.3 Card especial (por tempo ou instantâneo): C->S 0xBD → 0x160

**C->S 0xBD** `u32 typeid do card`. Sai de `ccardbook OnCardDlgResult 0x7F4930` / `0x7F3BE0` **[EXE ✓]**. O tradutor faz −8
e entrega como **KR 0xB5** (`CardUseAsync`).

**S->C 0x160** (KR 0x158, +8) **[EXE ✓ 0x9C9791]**. O código é lido primeiro e decide o resto:

| código | o cliente lê depois | efeito |
|---|---|---|
| **0** | **bloco 58**: `u32 id, u32 typeid, u32 typeid da peça, u32 id da peça, u32 slot, u32 1, SYSTEMTIME início, SYSTEMTIME fim, u16 0` | tira 1 da pilha com o mesmo **typeid** (+4); monta a entrada ativa com efeito/valor do Card.iff (@0xF4/@0xF6) |
| **1** | `u8 x` | mensagem: `x == 1` → texto `0xD14CD8`, senão `0xD405C4` |
| **0x35** | `u32 v` | mensagem `0xD405E8` com o valor |
| outro | nada | ignorado (a tela fica como está) |

- É **exatamente** o que o KR escreve (`CardResult`: `U32(0)` + 6 × u32 + 2 × SYSTEMTIME + `U16(0)`; erro `U32(1).U8(1)`) e o
  que a referência escreve. Não há o que converter.
- O que o cliente faz com o bloco (`cardType = (tid>>22)&0xF`):
  - **2 (especial)**:
    - efeito 1 (EXP): soma o valor a `[player+0x919]`;
    - efeitos 1, 4 (pang) e 17 (pang aleatório): **não** viram entrada ativa (são instantâneos; o pang novo chega pelo
      0xC8 que o KR manda como 0xC6);
    - os demais: com a opção `0x6F` ligada, o cliente **apaga a entrada ativa de outro especial com o mesmo efeito** (busca na
      lista `0x37EC` por `tid & 0x3C00000 == 0x800000` e efeito igual) e põe a nova.
  - **0 / 1 / 5** (encaixe pela caderneta, §2.4): o slot vira local (`slot −4` para caddie, `−8` para NPC) e a entrada da
    peça é atualizada.
- **Prazo e vencimento.** O prazo vem do `SYSTEMTIME fim` do bloco (e do 0x137 no login). O cliente conta o tempo sozinho, e
  nem a referência nem o KR mandam pacote quando vence. O servidor só precisa:
  - parar de aplicar o efeito (`CardService.Active` / `ActiveRate` já filtram por `ExpiresAt`);
  - não mandar o card vencido no 0x137 do próximo login (já é assim).
- **Diferença de regra (KR × REF)** para o mesmo efeito já ativo:
  - **REF**: com o **mesmo typeid**, **soma** o tempo (`fim = max(agora, fim) + minutos`); com typeid **diferente**, troca o
    card e recomeça o prazo.
  - **KR**: sempre apaga o antigo e cria um novo com `minutos` a partir de agora.

  Para o JP, adotar a da referência (`UseSpecialAsync`: estender `ExpiresAt` quando o typeid for o mesmo). O cliente troca a
  entrada de qualquer jeito, então não há conflito de tela.
- Duração: `Card.iff UseTime (u16 @0x170)` em minutos (por exemplo, 0x7C800003 = 120). Se for 0, o KR usa 60 min.
- Efeitos instantâneos (REF): 1 EXP += valor; 4 pang += valor; 17 pang aleatório `100 + rand % (valor − 100) + 1`. O KR usa
  `rand(1..valor)`, o que é uma diferença pequena. Os efeitos com prazo são 2, 3, 5..16, 18..34 (lista em
  `Channel.cs:14280–14313`). Efeito 0 ou desconhecido dá erro.

### 2.4 Encaixar pela caderneta (cards em peças): C->S 0xC8 → 0x160

- **C->S 0xC8** `bloco 58` (`CCardSlot::OnItemAttachCardDlgResult 0x7F3CD0`) **[EXE ✓]**. Os primeiros 20 B são `u32 id do
  card, u32 typeid, u32 typeid da peça, u32 id da peça, u32 slot`. O tradutor faz −8 e entrega como **KR 0xC0**
  (`CardAttachAsync`, que lê 20 e pula o resto). A resposta é 0x160 (§2.3).
- **A referência não trata o 0xC8** (não há `addPacketCall(0xC8)` no `GameService.cs`). No JP, card de personagem e de caddie
  vai para os **slots do personagem** (§3). Esse caminho da caderneta é o do KR e parece morto na interface do JP **[?]**.
  Recomendação: deixar o KR como está (inofensivo) e conferir no cliente se algum botão ainda manda 0xC8.

### 2.5 Removedor da caderneta: C->S 0xE0 → 0x188

- **C->S 0xE0** `u32 typeid do removedor, u32 id, u32 typeid da peça, u32 id da peça` (`FrCardRemove::OnUseCardRemoveConfirm
  0x7FE510`) **[EXE ✓]**. O tradutor faz +2 e entrega como **KR 0xE2** (`CardRemoveAsync`: destrói os cards da peça).
- **S->C 0x188** (KR 0x18D, −5) **[EXE ✓ 0x9D1D0C]**. O primeiro byte decide o resto:
  - **`u8 1`** (ok) → `u32 id da peça, u32 typeid do removedor`. O cliente confere se o removedor está na sua tabela
    `0xE1D1D8` = {0x1A0000C2, 0x1A0000CD, 0x1A0000CE, 0x1A0000CF}, a mesma do KR. Só **0x1A0000C2 "Card Remover"** existe no
    Item.iff do JP. Com id da peça 0, o cliente marca o estado 2.
  - **`u8 0`** (erro) → `u8 código` (código 9 dá uma mensagem própria; os outros, a genérica).
  - O KR manda `U8(0).U8(0)` ✓. A pendência antiga "u8?" do SPEC-jp983-loja §2.8 fica resolvida.
- A referência também não trata o 0xE0. O removedor do JP que importa é o do **0x18C** (§3.3).

### 2.6 Lolo (compor cards): C->S 0x155 → 0xC8 + 0x216 + 0x229 + 0x22A

**C->S 0x155** **[EXE ✓ 0x486F70]**: `u64 pang, u32 typeid[3]` (20 B). O TSV mostra só `E8 E4` porque as 3 escritas são
um laço. O console manda `E8 E4 ED`, que não interessa.

**Regras [REF `requestLoloCardCompose` + `CardSystem.drawsLoloCardCompose`]**:
- Os 3 typeids precisam existir no Card.iff. Cada um precisa estar no inventário com qtd ≥ 1; o mesmo typeid repetido consome
  várias unidades da mesma pilha. Nenhum pode estar à venda na loja pessoal.
- **SC (raridade 3) é proibido**.
- **Custo** por card: N 1000, R 2000, SR 5000. A soma precisa ser **igual** ao `pang` que o cliente mandou, senão dá erro.
- **Sorteio**: entram todos os cards das séries 1..5 (pacotes com `volume == i`). Peso = `prob` do banco; cards não-N
  ganham `+ 3 × (r+1) × 20`, onde **r é a raridade do último card** do pedido (a referência sobrescreve `lcc.tipo` a cada card).
  Como não temos o `prob`, a proposta **[suposição]** é: pool = cards finais com `Volume` 1..5 e subtipo 0/1/2/5; peso
  base 70/22/7/1 por raridade; bônus `+ 3 × (r+1)` nos não-N. Tudo configurável.
- Efeitos: tira os 3 cards, cobra o pang e põe 1 card novo (pilha +1 ou pilha nova).

**Respostas, nesta ordem [REF; EXE ✓ 0x229 / 0x22A]**:
1. **0xC8** `u64 pang atual` (+ `u64 gasto`, que o cliente não lê: o 0xC8 do JP é `D8`).
2. **0x216** (§3.4): uma entrada tipo 2 por card gasto (antes/depois/−qtd) e uma para o card ganho.
3. **0x229** `u32 raridade do card ganho` (0..3). O cliente toca a animação (mensagem `0x325`).
4. **0x22A** `u32 0, u32 typeid ganho`. Erro: **0x22A** `u32 código ≠ 0` (o cliente fecha a espera e abre a caixa de erro
   com o código). Os códigos da referência são `0x5400151..0x5400159`, e o genérico é `0x5400150`. Bloqueio por flag de
   conta: `0x790001`.

Hoje o [TR] responde `0x22A (1, 0)` (recusa). No KR não há nada equivalente.

### 2.7 Tabela-resumo dos cards

| função | JP C->S | JP S->C | KR | conversão |
|---|---|---|---|---|
| listas do login | — | 0x135, 0x138, 0x136, 0x137 | 0x12D, 0x130, 0x12E, 0x12F | só o id (+8), **já sai** |
| abrir pacote | 0xCA `u32, i32` | 0x154 | 0xC2 → 0x14C | só o id; **mudar o sorteio** (séries do JP, NPC) |
| card especial | 0xBD `u32` | 0x160 | 0xB5 → 0x158 | só o id; **regra de estender o prazo** |
| encaixar na peça (caderneta) | 0xC8 `bloco 58` | 0x160 | 0xC0 → 0x158 | só o id (caminho velho, a REF ignora) |
| removedor (caderneta) | 0xE0 `4 × u32` | 0x188 | 0xE2 → 0x18D | só o id (a REF ignora) |
| encaixar no personagem | **0x18A** `5 × u32` | 0x216 (2 + 0xCB) + **0x271** | — | **novo** |
| encaixar com patcher | **0x18B** `5 × u32` | 0x216 + **0x272** | — | **novo** |
| remover do personagem | **0x18C** `5 × u32` | 0x216 + **0x273** | — | **novo** |
| Lolo | **0x155** `u64, u32[3]` | 0xC8, 0x216, **0x229**, **0x22A** | — | **novo** |

---

## 3. Cards no personagem (0x18A / 0x18B / 0x18C)

### 3.1 C->S 0x18A encaixar / 0x18B encaixar com patcher **[EXE ✓ 0x5BBAB0; REF CardEquip]**
```
u32 typeid do personagem   (o equipado: [player+0xA2A] → mapa 0x3704)
i32 id do personagem
u32 typeid do card
i32 id do card (pilha)
u32 slot 1..12             (quadro da tela convertido por 0x5B8020, §1.1)
```
Regras [REF `requestCharacterCardEquip` / `...WithPatcher`]:
- O personagem precisa ser do jogador com o mesmo typeid. O card precisa ser do jogador (pilha qtd ≥ 1), existir no
  Card.iff e não estar à venda na loja pessoal.
- **0x18A**: os slots 4 e 8 são recusados (exigem 0x18B). O slot 7 exige uma peça vestida com `CaddieSlot ≠ 0`.
- **0x18B**: só os slots 4 e 8. Gasta 1 `0x1A00018F` (Bonus Card Patcher). NPC é recusado.
- Subtipo do card por faixa de slot: **1–4 → 0** (personagem), **5–8 → 1** (caddie), **9–12 → 5** (NPC).
- O slot precisa estar **vazio**: a referência recusa ("já tem card"), não troca. Para trocar, o jogador remove antes (0x18C).
- Efeitos: pilha do card −1 (e −1 patcher no 0x18B); grava o typeid no slot; cria a entrada ativa (CardEquip: typeid da
  peça = personagem, slot = 1..12, efeito/valor do Card.iff, subtipo, ativo 1, **sem prazo**).

Respostas, nesta ordem:
1. **0x216** (§3.4) com:
   - tipo 2 do card (qtd −1);
   - no 0x18B, também tipo 2 do patcher;
   - **tipo 0xCB**: `typeid do personagem, id do personagem, c[5] = 0, +45 = typeid do card, +49 = slot`.
2. **0x271** (0x18A) / **0x272** (0x18B): `u32 0, u32 typeid do card` **[EXE ✓ 0x9D6BDD / 0x9D6C9D]**. O cliente chama
   `0x5BB820(typeid, 1)` e `0x5BBDB0(id do personagem)`, que redesenha os 10 quadros lendo `char+0x1D1+4×(slot−1)`.
   Erro: `u32 código ≠ 0` → caixa de erro com o código.

### 3.2 Códigos de erro [REF] (o cliente só mostra o número)

| pedido | código |
|---|---|
| bloqueado por flag de conta (`char_mastery`) | 0x790001 |
| **0x18A** | 0x5200751 personagem; 0x5200752 card; 0x5200753 slot 4/8 sem patcher; 0x5200754 slot 7 sem peça; 0x5200755 slot inválido; 0x5200756 subtipo errado; 0x5200757 card fora do IFF; 0x5200758 falha ao gastar; 0x5200759 slot ocupado; 0x5201010 card à venda; genérico 0x5200750 |
| **0x18B** | 0x5200801..0x5200812 (0x5200810 / 0x5200811 = sem patcher / sem qtd; 0x5200803 = fora dos slots 4/8; 0x5200804 = NPC; 0x5200812 = ocupado); genérico 0x5200800 |
| **0x18C** | 0x5200851..0x5200859 (0x5200854 = slot vazio; 0x5200852 / 0x5200855 = removedor); genérico 0x5200850 |

Esses códigos **não** estão na `ErrorCodeInfo.iff` (que usa decimais como 5200001, 5500001...; quase tudo "System Error").
O cliente mostra a caixa genérica com o número. Para o nosso servidor basta `1` em qualquer recusa, ou os códigos da
referência se quisermos rastrear.

### 3.3 C->S 0x18C remover **[EXE ✓ 0x5BBC50; REF CardRemove]**
```
u32 typeid do personagem, i32 id do personagem, u32 typeid do removedor, i32 id do removedor, u32 slot 1..12
```
- O removedor precisa ser do jogador (`id` e `typeid` batendo, qtd ≥ 1). Na prática é 0x1A0000C2. A referência **não**
  restringe a lista de removedores, mas o cliente só oferece os da tabela `0xE1D1D8`.
- O slot precisa ter card.
- Efeitos: removedor −1; **o card volta ao inventário (pilha +1)**, ao contrário do 0xE2 do KR, que destrói; o slot fica 0; a
  entrada ativa sai.
- Respostas:
  1. **0x216** com:
     - tipo 2 do removedor (−1);
     - tipo 2 do card devolvido (antes/depois/+1);
     - **tipo 0xCB** do personagem com `+45 = 0` (vazio) e `+49 = slot`.
  2. **0x273** `u32 0, u32 typeid do card removido`. Erro: `u32 código`.

### 3.4 Pacote 0x216: layout exato e variantes **[EXE ✓ leitor 0x9BFC90, aplicador 0x58C160]**

`u32 hora unix, u32 n` e depois n entradas. O cliente aplica as entradas **uma a uma**. Se uma falhar, ele mostra a caixa
de erro (mensagem `0x27`) com o código e **para** (`0x9D43CD`).

Entrada básica (50 B):

| off | tipo | campo | card no personagem (0xCB) | stats (0xC9) | maestria (0xCD) |
|---|---|---|---|---|---|
| 0 | u8 | tipo (2 = item comum) | 0xCB | 0xC9 | 0xCD |
| 1 | u32 | typeid | personagem | personagem (ou club set) | personagem |
| 5 | i32 | id | personagem | id | personagem |
| 9 | u32 | flag de tempo | 0 | 0 | 0 |
| 13 | i32 | qtd antes | 0 | 0 | 0 |
| 17 | i32 | qtd depois | 0 | 0 | 0 |
| 21 | i32 | qtd (ou tempo) | 0 | 0 | 0 |
| 25 | i16[5] | c | 0 | **PCL[0..4]** (o cliente usa o byte baixo) | 0 |
| 35 | str | Self Design (u16 tamanho + bytes; vazio = `00 00`) | `00 00` | `00 00` | `00 00` |
| 37 | u32 | ? (a REF zera) | 0 | 0 | 0 |
| 41 | u32 | ? (a REF zera) | 0 | 0 | 0 |
| **45** | u32 | **typeid do card** | **card (0 = remove)** | 0 | 0 |
| **49** | u8 | **slot do card** | **1..12** (0 dá o erro 0x1E856A) | 0 | 0 |

Extras **depois** dos 50 B. O cliente só os lê quando o tipo **e** o grupo do typeid batem:

| tipo | grupo (`tid>>26`) | extra |
|---|---|---|
| 0xCC (oficina de club set) | 4 (club set) | `i16[5], u32, u8, u32, u32` (23 B) |
| **0xCD (maestria)** | **1 (personagem)** | **`u32 maestria`** (4 B) → `char+0x1CD` |
| 0xCE | 7 (caddie) | `u8, u32` (5 B) |

O que o cliente faz com cada tipo (switch em `0x58C160`):
- **2**: item comum.
- **0xC9**: stats. Grupo 1 → `char+0x1C8..0x1CC`; grupo 4 → club. Outro grupo dá o erro 0x1E854E.
- **0xCA**: não suportado (0x1E854D).
- **0xCB**: card no slot. Grupo 1 → personagem (`0x588C00`: grava o typeid, monta a entrada ativa e redesenha); grupos 2 e 4
  → peça / club (`0x588D60` / `0x589070`) [?]. Outro grupo dá 0x1E854F. Personagem não encontrado dá 0x1E8569.
- **0xCC**: oficina de club set.
- **0xCD**: só grupo 1; senão dá 0x1E8567. Personagem não encontrado dá 0x1E856B.
- **0xCE**: caddie.

**Bate com a referência.** Ela escreve `c` (i16[5]) + `10 zeros` + `u32 typeid do card` + `u8 slot`, porque os "10 zeros"
são `str vazia (2) + u32 + u32`. Para 0xCD ela escreve 25 zeros + `u32 maestria` = 54 B ✓.

O escritor de hoje, `JpSystems.ItemsChanged`, só faz o tipo 2 com 25 zeros. É preciso generalizá-lo (§7).

---

## 4. Maestria e stats do personagem

### 4.1 C->S 0x187 expandir maestria **[EXE ✓ 0x5C16C0: `[char+0]`, `[char+4]`]**
`u32 typeid do personagem, i32 id do personagem` (8 B).

Regras [REF `requestCharacterMasteryExpand`]:
- O personagem precisa ser do jogador. O CharacterMastery do typeid precisa existir.
- `maestria + 1 ≤ nº de etapas` (10 por personagem).
- `etapa[maestria].seq == maestria + 1`.
- **`etapa.level ≤ nível do jogador`** (o índice de nível 0..70; as etapas pedem 0, 11, 21, 31 ou 46).
- **Condições** (até 5): o tipo de cada uma se decide pelo grupo do typeid.
  - **Item** (`0x1A0002A6` "Tiki Points (TP)"): qtd no inventário ≥ exigida, e **é gasta**.
  - **QuestStuff** (`0x748xxxxx`, por exemplo "Play 10 games with Nuri"): a etapa de conquista precisa estar **concluída**
    (`AchievementBook.Cleared` tem o typeid). **Não** é gasta.
  - Outro grupo dá o erro 0x5200656.
- Efeitos: gasta os TP; `maestria + 1`; grava.

Respostas:
1. **0x216** com:
   - tipo 2 do TP (−qtd);
   - **tipo 0xCD** do personagem + `u32 maestria nova`.
2. **0x26E** `u32 0` **[EXE ✓ 0x9D6A1A: lê só o código; com 0 redesenha a maestria (`0x5CC980` / `0x5C59E0`)]**.

Erros (`0x26E u32 código`): 0x5200651 personagem; 0x5200652 sem IFF; 0x5200653 já no máximo; 0x5200654 seq; 0x5200655
nível; 0x5200656 condição desconhecida; 0x5200657 / 0x5200658 item; 0x5200659..0x5200661 quest; 0x5200662 falha ao gastar;
genérico 0x5200650.

### 4.2 C->S 0x188 stat +1 / 0x189 stat −1 **[EXE ✓ 0x5C9D78]**
`u32 stat (0 força, 1 controle, 2 precisão, 3 spin, 4 curva), CharacterInfo 513` (517 B). O cliente escolhe 0x188 ou 0x189
na mesma função, então o layout é igual nos dois. O TSV só tem o console, e o 0x189 aparece sem campos. A referência usa
do CharacterInfo apenas `typeid` e `id`.

**+1 (0x188)** [REF `requestCharacterStatsUp`]:
- **Limite de upgrades** do stat:
  - `Character.iff PCL[stat]` (u8 @0x174; no JP é igual ao Attr i16 @0x160, por exemplo Nuri 9/11/6/2/2);
  - **+1 por etapa de maestria liberada com `etapa.stats − 1 == stat`**;
  - \+ slots das peças vestidas, dos anéis, da tabela de conjunto e **dos cards de personagem (slots 1–4: `Card.iff COM
    i16[5]` @0xEA)**;
  - \+ (só força) `(nível − 1) / 5`.

  A referência recusa só quando `pcl[stat] > limite`, o que deixa passar um a mais; o certo parece `pcl[stat] + 1 ≤ limite`
  **[?]**. O cliente também confere.
- **Custo**: `Enchant.iff` com chave `0x34000000 | stat<<20 | pcl[stat]` → `ReqdPang` (u32 @0x08; 0x34000000 = 2100,
  0x34000001 = 4200...; níveis até 50 na força e 30 nos outros). Sem a linha → erro.
- Efeitos: cobra o pang; `pcl[stat] + 1`.
- Respostas:
  1. **0xC8** `u64 pang atual` (+ `u64 gasto`);
  2. **0x216** com **um** item tipo **0xC9** (typeid e id do personagem, `c[5] = PCL`);
  3. **0x26F** `u32 0, u32 stat` **[EXE ✓ 0x9D6AB6: com 0 chama `0x5C9B00(stat, 1)`]**.

**−1 (0x189)** [REF `requestCharacterStatsDown`]:
- `stat ≤ 4`; `pcl[stat] > 0`. **Não devolve pang.**
- Respostas: 0x216 (0xC9) + **0x270** `u32 0, u32 stat` (`0x5C9B00(stat, 0)`).

Erros: 0x26F com 0x5200501..0x5200507 (genérico 0x5200500); 0x270 com 0x5200551..0x5200554 (genérico 0x5200550).

Conquistas [REF]: stat +1 → contador 0x6C400084; −1 → 0x6C400085; maestria → 0x6C4000C3.

### 4.3 O caminho velho: 0x4B → 0xA5 (já existe no KR)
- **C->S 0x4B** `u8 tipo (bit0 = club, bit1 = descer), u8 stat, u32 id` e, **só se a opção 0x7C estiver ligada e não for
  club**, `+ CharacterInfo 513` **[EXE ✓ 0xAA73B0]**. O KR lê os 6 bytes e pula o resto (`GameHandler.MyRoom.cs:51`).
- **S->C 0xA5** (KR 0xA3, +2) **[EXE ✓ 0x5A8674]**: `u8 res (1 = subiu, 2 = desceu)`, depois `u8 tipo, u8 stat, u32 id,
  u64 pang gasto`. O cliente faz ±1 em `char+0x10+0x1C8` (PCL) ou no club e subtrai o pang. É idêntico ao KR.
- A referência trata o 0x4B **só para club set** (opt 1/3; o comentário diz "0/2 é personagem da season passada", e para
  esses ela nem responde). Recomendação: manter o KR como está. A tela de maestria do JP manda 0x188/0x189, e o 0x4B de
  personagem é só um caminho de reserva. O limite do KR para personagem é 255 (sem conferir), o que fica aceitável enquanto
  o 0x188 não existe.

---

## 5. Tabelas IFF (pangya_jp.iff)

### 5.1 Card.iff (0x180 = 384 B por registro; 264 registros) **[IFF; SPEC-jp983-iff.md]**

| off | tipo | campo | lido hoje (`Jp983GameData.Cards`) |
|---|---|---|---|
| 0x00 | u8 | ativo | `Active(r)` |
| 0x04 | u32 | typeid (subtipo = bits 22..25) | sim |
| 0x08 | char[64] | nome | — |
| 0xC0 | u8 | raridade (0 N, 1 R, 2 SR, 3 SC) | sim |
| 0xC1 | char[40] | imagem | — |
| **0xEA** | **i16[5]** | **COM: slots de stat que o card de personagem dá** | **não** (é preciso para o limite do 0x188) |
| 0xF4 | u16 | efeito | sim |
| 0xF6 | u16 | valor do efeito | sim |
| 0xF8 / 0x120 / 0x148 | char[40] × 3 | SubIcon / SlotImg / BuffImg | — |
| 0x170 | u16 | duração (min) | sim |
| 0x172 | u16 | série (Volume): 1..5, 100 = colaboração/evento; 0 nos pacotes | sim |
| 0x174 | u32 | posição na caderneta | não |
| 0x178 | u32 | flag1 (a REF usa para nomear o efeito; 1/3/5/7/10/11/12/15) [?] | não |
| 0x17C | u32 | flag2 [?] | não |

Subtipos no JP: 0 personagem (75), 1 caddie (59), 2 especial (72), 3 pacote/ticket (28), 4 caixa (4), **5 NPC (26)**. O
`CardInfo` do Domain não tem a constante `SubNpc = 5`.

### 5.2 CharacterMastery.iff (0x3C = 60 B; 140 registros = 14 personagens × 10 etapas) **[IFF; REF CharacterMastery]**

| off | tipo | campo |
|---|---|---|
| 0x00 | u32 | ativo |
| 0x04 | u32 | typeid do personagem |
| 0x08 | u32 | seq (1..10) |
| 0x0C | u32 | stat que ganha +1 slot (1 força .. 5 curva = stat + 1) |
| 0x10 | u32 | nível mínimo do jogador (índice 0..70: 0, 11, 21, 31, 46) |
| 0x14 | u32[5] | condição (typeid: item 0x1A0002A6 TP, ou QuestStuff 0x748xxxxx) |
| 0x28 | u32[5] | quantidade |

Exemplo (Nuri 0x04000000):

| seq | stat | nível | condição |
|---|---|---|---|
| 1 | força | 0 | TP 2 |
| 2 | força | 0 | TP 5 + "Play 10 games with Nuri" |
| 3 | força | 11 | TP 10 |
| 4 | spin | 0 | TP 15 |
| 5 | controle | 21 | TP 20 + 50 jogos |
| 6 | curva | 0 | TP 30 |
| 7 | curva | 0 | TP 35 |
| 8 | força | 31 | TP 40 |
| 9 | força | 0 | TP 45 |
| 10 | controle | 46 | TP 100 + 250 jogos |

As versões "R" (Nuri R 0x0400000B, Hana R, Cecilia R) têm as suas próprias linhas e quests. O despejo completo sai de
`<scripts locais>/cards/m1.py`.

### 5.3 Enchant.iff (0x10 = 16 B; 175 registros) **[IFF]**
`0x00 u32 ativo, 0x04 u32 typeid = 0x34000000 | stat<<20 | nível, 0x08 u32 ReqdPang, 0x0C u8 nível mínimo`. É igual ao KR,
e `IGameData.UpgradePrice(stat, cur)` já o lê.

### 5.4 Outras
- **Character.iff** (JP): Attr `u16[5]` @0x160 e PCL `u8[5]` @0x174 (base do limite de upgrades).
- **Part.iff**: `CharacterSlot u16 @0x228` (2 peças), `CaddieSlot u16 @0x22A` (231 peças: libera o slot 7),
  `NPCSlot u16 @0x22C` (0 em todas).
- **Item.iff**: 0x1A00018F "Bonus Card Patcher", 0x1A0000C2 "Card Remover", 0x1A0002A6 "Tiki Points (TP)", 0x1A0003A9 /
  0x1A0003B9 tickets de troca de card [?].
- **QuestStuff.iff / Achievement.iff**: as quests de "jogar N vezes com X" (0x748...) já são lidas pelo
  `IAchievementData.AchievementQuests`.
- **Não há "CardPack.iff"**: as probabilidades dos pacotes e do Lolo vêm do banco da referência
  (`pangya_new_card_pack` / `pangya_new_cards`), que não temos. Ver §2.2 e §2.6.
- **ErrorCodeInfo.iff** (272 B: `u32 ativo, u32 código, u32 tipo, char[260] texto`): não cobre os códigos de card nem de
  maestria.

---

## 6. Persistência

Nada de tabela nova: tudo cabe no modelo de `Item` (atributos JSON) que já existe.

| dado | onde | observação |
|---|---|---|
| pilhas de card | `Item` do grupo Card, `Quantity` | já existe |
| card especial com prazo | `Item` com `Location = ActiveCard`, `ExpiresAt`, atributo `start` | já existe; muda só a regra de estender (§2.3) |
| **card no slot do personagem** | `Item` com `Location = ActiveCard`, atributos `parts_tid` = typeid do personagem, `parts_id` = id do personagem, **`slot` = 1..12** (numeração global, como no CardEquip do JP), sem `ExpiresAt` | entra sozinho no 0x137 e no 0x76 (`PlayerStructs.ActiveCards`); o CharacterInfo 465..512 é montado a partir destes itens. **Atenção**: o `AttachAsync` do KR grava o slot de caddie como 1..4 (`nslot`); os do JP gravam 5..8. Distinguir pelo `parts_tid` ser de personagem (grupo 1) ou guardar `slot_kind` |
| **maestria** | atributo `mastery` (int) no `Item` do personagem | novo |
| PCL | atributo `pcl` (int[5]) no `Item` do personagem | já existe (`PlayerActions.UpgradeAsync`) |
| TP | `Item` 0x1A0002A6 (consumível) | já existe como item |
| quest concluída | `AchievementBook.Cleared` (`IAchievementStore`) | já existe |
| patcher / removedor | `Item` consumível | já existe |

Gravar sempre por `IPlayerStore.ApplyAsync(PlayerChanges)` antes de mexer no `Player` em memória (padrão do
`CardService`). Item à venda na loja pessoal (`p.Reserved`) não pode ser gasto (§2.6, §3.1).

---

## 7. Plano de implementação neste repositório

### 7.1 O que reaproveitar do KR (sem mudar layout)
- Listas do login, abrir pacote, card especial, caderneta e removedor (§2.1–2.5): **nada a fazer no tradutor**. Só testes.
- `PlayerActions.UpgradeAsync` / 0x4B → 0xA5: continua valendo como caminho velho.
- `IGameData.UpgradePrice` (Enchant.iff), `AchievementBook.Cleared`, `IPlayerStore.ApplyAsync`, `PlayerStructs.ActiveCards`.

### 7.2 Mudanças no Domain (`src/Pangya.Domain/Shop/CardService.cs` e vizinhos)
1. `CardInfo`: acrescentar `SubNpc = 5` e o campo `Com` (i16[5]). `Jp983GameData.Cards` passa a ler @0xEA.
   `Kr645GameData` lê o COM do KR @186 (opcional).
2. `CardService.Draw`:
   - aceitar o subtipo 5;
   - **série do pacote por tabela** (dicionário typeid → (série, n, raridade mínima) no `IGameData` ou num JSON de `data/`;
     sem LINQ, regra 13);
   - Platinum Ticket.
3. `CardService.UseSpecialAsync`: **estender** o `ExpiresAt` quando o typeid ativo for o mesmo (§2.3).
4. **Novo `CharacterCardService`** (ou métodos no `CardService`):
   - `EquipAsync(p, charTid, charId, cardTid, cardId, slot, withPatcher)`: regras do §3.1 (slot 4/8 = patcher, slot 7 = peça
     com CaddieSlot, subtipo por faixa, slot vazio). Devolve as mudanças para o 0x216.
   - `RemoveAsync(p, charTid, charId, removerTid, removerId, slot)`: devolve o card ao inventário.
   - `CharacterCards(p, charId) → int[12]`, para montar o CharacterInfo.
   - Precisa de `IGameData`: `PartCaddieSlot(tid)` (Part.iff @0x22A no JP, @506 no KR).
5. **Novo `MasteryService`**:
   - `ExpandAsync(p, charTid, charId, book)`: CharacterMastery + TP + quests (§4.1);
   - `StatAsync(p, charId, stat, up)`: limite com maestria e cards; custo pelo Enchant (§4.2). Pode estender
     `PlayerActions.UpgradeAsync` com um `limit` calculado.
   - Dados novos no `IGameData` (ou numa interface só do JP, `IJpCharacterData`): `Mastery(charTid) → List<MasteryStep>`,
     `CharacterBasePcl(charTid)`.
6. **Novo `LoloService.ComposeAsync(p, pang, tids[3])`**: §2.6. Devolve `(cardGanho, raridade, mudanças, pangGasto)` ou o
   código de recusa.

### 7.3 Mudanças no JP983
1. **`JpSystems.ItemsChanged` generalizado.** Aceitar entradas `(tipo, typeid, id, antes, depois, delta, c[5], cardTid,
   cardSlot, extraU32?)`, escrevendo `str` vazia + 2 × u32 zerados antes do typeid do card, e o `u32` extra só para 0xCD
   com typeid de personagem. Fábricas: `Item(…)` (tipo 2), `Stats(ch)` (0xC9), `CardSlot(ch, cardTid, slot)` (0xCB),
   `Mastery(ch, m)` (0xCD).
2. Novos handlers em `JpSystems` (que já tem `conn`, `kr`, `ctx`), substituindo os stubs de `JpTranslator.JpOnly.cs:56–58`:

   | pedido | lê | responde |
   |---|---|---|
   | 0x187 | 8 B | 0x216 + 0x26E |
   | 0x188 / 0x189 | `u32 stat` + 513 (só tid e id) | 0xC8 (no +1) + 0x216 (0xC9) + 0x26F / 0x270 |
   | 0x18A / 0x18B / 0x18C | 20 B | 0x216 + 0x271 / 0x272 / 0x273 |
   | 0x155 | `u64 pang` + 3 × u32 | 0xC8 + 0x216 + 0x229 + 0x22A |

   Na recusa: o pacote de resposta com `u32 1` (ou o código da referência). Contadores de conquista pelo
   `RecordAsync(bag)` que já existe:

   | evento | contador |
   |---|---|
   | stat +1 / −1 | 0x6C400084 / 0x6C400085 |
   | maestria | 0x6C4000C3 |
   | encaixar / remover card no personagem | 0x6C400087 / 0x6C400088 |
   | pacote aberto | 0x6C400078 |
   | card por subtipo | 0x6C400079..7B, 0x6C4000A8 |
   | card por raridade | 0x6C40007C + raridade |
   | Lolo | 0x6C400089 e 0x6C40008A + raridade |
   | especial usado | 0x6C40009E |

   Os contadores de pacote e especial podem ir para o KR, via o saco de conquistas do 0xC2 / 0xB5.
3. **`KrToJp.Character`** escreve a maestria (461) e os 12 typeids de card (465..512). A struct do KR não traz isso, então
   o tradutor precisa de uma consulta por `(typeid, id)` do personagem:
   - **Do próprio jogador**: `kr.Player.Find(id)` (já usado em `JpTranslator.Shop.cs:216`).
   - **De outros jogadores** (sala 0x48/0x4B, 0x76, perfil 0x15E): o id do item é único no banco, então dá para consultar
     pelo `ctx.World` (jogadores online). Proposta: um delegate `Func<int, CharacterExtras?>` no `JpTranslator`, ao lado do
     `Known`, passado a todos os `w.Character(...)`. Sem isso, os cards dos outros não aparecem e os stats deles saem sem
     os cards no cliente.
   - `JpStructs.Equipped` (`JpStructs.cs:146`) tem o `Item` na mão e pode preencher direto.
4. `JpTranslator.JpOnly.cs`: tirar 0x187–0x18C e 0x155 dos stubs quando os handlers existirem. Os ids S->C 0x229/0x22A e
   0x26E–0x273 são só do JP: escrever direto, sem passar pelo `ServerId`.

### 7.4 Testes (xUnit; `tests/Pangya.Tests`, com `JpEnv` e o cliente falso `Jp983Client`)

**Layout contra o exe** (sem servidor):
```csharp
Assert.Equal([0x41], Jp983Client.Blocks(0x137));
Assert.Equal([0x3A], Jp983Client.Blocks(0x138));
Assert.Equal([0x3A, 0x3A], Jp983Client.Blocks(0x154));
Assert.Equal([0x3A], Jp983Client.Blocks(0x160));
Jp983Client.Build(0xCA, "FUN_008c1ea0", packTid, packId);                 // E4 E4
Jp983Client.Build(0xBD, "FUN_007f4930", cardTid);                         // E4
Jp983Client.Build(0x187, "FUN_005c16c0", charTid, charId);                // E4 E4
Jp983Client.Build(0x18A, "FUN_005bbab0", charTid, charId, cardTid, cardId, slot);
Jp983Client.Build(0x18C, "FUN_005bbc50", charTid, charId, remTid, remId, slot);
Jp983Client.Build(0x188, null, stat, new byte[513]);                      // E4 EB(0x201) (o TSV só tem o console)
```
O 0x189 e o 0x155 não têm a sequência certa no TSV (o 0x189 sem campos; o 0x155 com o laço colapsado em `E8 E4`).
Montar com `new PacketWriter(0x189).U32(stat).Bytes(ci513)` e `new PacketWriter(0x155).U64(pang).U32(a).U32(b).U32(c)`,
com um comentário apontando o endereço do exe (0x5C9D78 / 0x486F70).

**Ponta a ponta** (padrão do `Jp983SystemsTests.JpOnlyRequestsGetAnAnswer`):
1. **Login com cards**: semear 1 pilha e 1 especial ativo → `ExpectAsync(0x135)`, `0x138` (n = 1; qtd @0x14),
   `0x136`, `0x137` (n = 1; datas).
2. **Abrir pacote**: dar 0x7CC00005 (Vol.3) → 0xCA → 0x154 `u32 0`, 3 cards **todos de Volume 3**. Pacote sem série
   conhecida → `u32 1`.
3. **Especial**: 0x7C800003 (efeito 2, 120 min) → 0xBD → 0x160 com fim − início = 120 min. Repetir → o fim soma +120 (a
   regra nova). Pang (0x7C800009) → 0x160 + 0xC8 com o pang novo.
4. **Card no personagem**:
   - 0x18A slot 1 com card de personagem → 0x216 (n = 2: tipo 2 −1 e 0xCB com typeid @45 e slot @49) + 0x271 `0, tid`;
   - relogar → CharacterInfo (0x70) com o typeid em 465 e o 0x137 com a entrada `slot = 1`;
   - 0x18A no slot 4 → recusa; 0x18B no slot 4 sem patcher → recusa; com patcher → ok e patcher −1;
   - slot 7 sem peça CaddieSlot → recusa;
   - 0x18C → card volta (+1), removedor −1, 0xCB com typeid 0.
5. **Maestria**: personagem com maestria 0, TP 2 → 0x187 → 0x216 (TP −2 e 0xCD com `u32 1` no fim = 54 B) + 0x26E `0`.
   Sem TP → 0x26E ≠ 0. A etapa 2 sem a quest concluída → recusa; com `AchievementBook.Cleared[0x74800330]` → ok.
6. **Stats**: 0x188 força → 0xC8 (pang −2100) + 0x216 0xC9 (c[0] = 1) + 0x26F `0, 0`; 0x189 → 0x270 e sem devolver pang.
   No limite (PCL base + maestria) → recusa.
7. **Lolo**: 3 cards N (pang 3000) → 0xC8 (−3000) + 0x216 (3 × −1, +1) + 0x229 (raridade) + 0x22A `0, tid`. Pang errado
   → 0x22A ≠ 0. Card SC → recusa.
8. **Unidade do 0x216**: o escritor gera 50 B por entrada e 54 B na 0xCD, com `str` vazia em +35 (`00 00`) e o typeid do
   card em +45.

Rodar com `dotnet test --filter Jp983`. Com o cliente real (game-jp na 30202): abrir a tela de cards do personagem,
encaixar, relogar e conferir que o card continua no quadro e que os stats mudam.

---

## 8. Pendências [?]
- Os botões do JP que ainda mandam 0xC8 (encaixe pela caderneta) e 0xE0: conferir no cliente.
- A série dos pacotes de evento (GP, estações, colaborações), o Platinum Ticket e as probabilidades oficiais de pacote e
  Lolo (vêm do banco da referência, que não temos).
- `flag1` / `flag2` do Card.iff (@0x178 / @0x17C).
- A off-by-one do limite de stat da referência (`>` em vez de `>=`).
- As opções `0x6F` e `0x7C` do cliente (`0xA7A8D0`), que ligam a troca de especial do mesmo efeito e o CharacterInfo no fim
  do 0x4B. Descobrir de onde vêm (login?).
- Os tipos 0xCB com grupo 2 (peça) e 4 (club) no 0x216: o cliente aceita, mas a referência não usa.
