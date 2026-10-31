# Papel Shop, Big Papel e Memorial Shop no cliente JP 983

Fontes:
- [EXE]: `docs/server/jp983-protocolo/pacotes_recebe_983.tsv` e `pacotes_envio_983.tsv`.
- [REF]: servidor AGPL de referência, `GameServer/Game/Channel.cs` 17040-17520 e 13412-13705, `Game/System/PapelSystem.cs`, `Game/System/MemorialSystem.cs`. Usei só o comportamento, sem copiar código.
- [IFF]: `data/pangya_jp.iff`, lido com python sobre o zip real.
- [LOG]: `logs/pangya-20261012.log` 16:42.
- [?] marca o que não está confirmado.

## 0. Leituras do exe (S->C) e escritas (C->S)

| id JP | sentido | leituras/escritas no exe | uso |
|---|---|---|---|
| 0x98 | C->S | vazio (`lobbymain`, `taskmain`) | abrir o Papel Shop (KR 0x95) |
| 0x14B | C->S | vazio (`FrBongdariShopDlg`) | jogar normal (KR 0x6D) |
| 0x186 | C->S | vazio (`FrBongdariShopDlg`) | jogar Big Papel (o KR não tem) |
| 0x17F | C->S | `E4` (`bongdarishopdlg`) | Memorial: u32 typeid da moeda |
| 0x10B | S->C | `D4 D4 D4` | abrir o Papel Shop |
| 0xD5 | S->C | `D4` | cupom gasto (KR 0xD3) |
| 0xFB | S->C | `D4 D4` | jogadas restantes (KR 0xF9) |
| 0xC8 | S->C | `D8` | pang |
| 0x216 | S->C | `D4 D4` (o laço de itens ficou fora da análise) | itens alterados (genérico, só no JP) |
| 0x21B | S->C | `D4 D4 D4 DB(0x14) D8 D8` | resultado do Papel normal |
| 0x26C | S->C | `D4 D4 D4 DB(0x14) D8 D8` | resultado do Big Papel (mesmo layout do 0x21B) |
| 0x264 | S->C | `D4 D4 D4 D4 D4` | resultado do Memorial |

## 1. Papel Shop normal

### 1.1 Abrir: C->S 0x98 (vazio) -> S->C 0x10B [EXE ✓ tamanho]
- O exe lê `u32, u32, u32` (12 bytes).
- A REF escreve `u32 0` (ok), seguido de `i64 limit_count`.
  - `limit_count` é um u16. Quando não há limite, vale 0xFFFF, e a parte alta do i64 fica 0.
  - No erro, a REF manda `i64 -1, u32 código`, uma ordem invertida que parece bug.
- Pelo paralelo com o KR 0x109 (`u32 bônus, u32 bônus restantes, u32 ignorado`), o mais provável é que o JP leia `u32 código, u32 bônus (-1 = sem bônus), u32 bônus restantes` [?].
- Hoje mandamos `u32 0, i64 -1` (= `0, -1, -1`). O cliente abriu a janela [LOG: 0x98 -> 0x10B de 14 bytes]. Está ok.
- Alternativa equivalente à do KR, que não muda nada visível quando não há limite: `u32 0, u32 -1, u32 0`.
- A REF não bloqueia ao abrir, porque o Memorial fica na mesma janela. Os erros só aparecem ao jogar.

### 1.2 Jogar: C->S 0x14B (vazio)
Resposta da REF, nesta ordem:
1. **0x216**: itens ganhos (empilhados por typeid) e o cupom com quantidade -1.
2. **0xFB** `i32 restantes, i32 flag`:
   - com limite diário: `restantes, -2`;
   - sem limite: `-1, -3`.
3. **0x21B**:
   ```
   u32 código                   // 0 = ok
   i32 id do cupom gasto        // 0 = pagou em pang
   u32 n                        // nº de bolas
   n × 20 bytes:
     u32 cor                    // 0 azul, 1 verde, 2 vermelha (aleatória na REF)
     u32 typeid
     u32 id do item             // a REF manda 0 no normal; ver 1.4
     u32 qtd
     u32 tipo                   // 0 comum, 1 cookie, 2 raro
   u64 pang                     // absoluto
   u64 cookie                   // absoluto
   ```
   - No erro, só o `u32 código`.

O que mandamos hoje [LOG 16:42:27]:
- 0x14B -> KR 0x6D.
- **0xD5** (6 bytes): o KR 0xD3 com o cupom.
- **0x21B** (70 bytes = 2 + 12 + 2×20 + 16: 2 bolas).
- **0xFB** (10 bytes = `-1, -1`).

Avaliação de cada pacote:
- **0xFB de 10 bytes: está certo para o JP** [EXE ✓ `D4 D4`]. A REF manda `-1, -3` quando não há limite. Pelo KR, o 2º campo é "bônus" e -1 = sem bônus. Os dois valores funcionam enquanto o 1º for -1, porque o aviso "acabaram as jogadas" só aparece com `usable < 2 && usable != -1`. Pode ficar como está, ou passar a `-1, -3` para seguir a REF [?].
- **0x21B**: o layout bate com o exe. A bola de 20 bytes do KR (`u8 classe, u8 0, u16 0` no fim) já é o `u32 tipo` do JP.
  - Diferença: o tradutor põe **`i32 cupom = 0`** e manda o cupom pelo 0xD5.
  - A REF não manda 0xD5: põe o id do cupom no 0x21B e tira o cupom pelo 0x216.
  - Risco: se o cliente JP desconta o cupom pelo id do 0x21B **e** pelo 0xD5, ele desconta 2. Com id 0 no 0x21B isso não acontece. Hoje o risco é o contrário: se o JP ignorar o 0xD5, ele não desconta nenhum [?]. Conferir no cliente: jogar com cupom e ver se a contagem cai 1.
- **0x10B**: ok (ver 1.1).

### 1.3 Achado: 0x44 com o Papel zerado
- No 0x44 (login do game), o tradutor manda `U16(0).Zeros(6)` (`JpTranslator.cs`, toClient[0x42]). Isso dá `u16[3] papel = 0, 0, 0`, ou seja, jogadas = 0.
- O KR manda `0xFFFF, 0xFFFF, 0` (`GameHandler.cs:181`).
- Até a primeira jogada, o painel do JP pode mostrar 0 jogadas ou o aviso de fim de jogadas [?].
- Correção: mandar `0xFFFF, 0xFFFF, 0` também no JP.

### 1.4 Inventário e o id do item
- No KR, o cliente cria o item local a partir do id da bola. É o que fazemos hoje, sem 0x216.
- Comentário da REF: o id na bola é obrigatório; sem ele o cliente adiciona 2 vezes (o do 0x216 e o da bola). Ou seja, **o JP também cria o item a partir do 0x21B**, e com o id certo ele junta com o do 0x216.
- Como mandamos o id real e não mandamos 0x216, o caminho atual deve bastar [?]. Conferir que a pilha local soma a quantidade.

### 1.5 Limite por dia (o "i64")
REF (opcional, `limitted_per_day` no banco):
- O jogador tem `remain`, `current` e `limit` (u16; 0xFFFF = sem limite).
- Na troca do dia: `remain = limit`. Se não usou tudo no dia anterior, `limit` sobe para 50 e depois para 100. `current` volta a 0.
- Cada jogada (normal ou Big) faz `current++` e `remain--`. Ao zerar, `limit` cai para 30.
- Sem limite, nada disso roda: 0xFB vai com `-1`.

Proposta: começar sem limite, igual ao KR. Se quiser limite, criar `LotteryConfig.PapelDailyLimit` (0 = sem limite) e um contador por jogador no Domain.

### 1.6 Sorteio da REF (normal)
- **Bolas**: 1 a 4. O código pretende 1 a 5, mas o limite superior é exclusivo.
- **Tabela de itens**: vem do banco (`pangya_papel_shop_item`: typeid, probabilidade, número da rodada ou -1, tipo 0/1/2, ativo), sorteada por roleta ponderada.
- **Cor**: aleatória entre 0 e 2.
- **Quantidade**: raro = 1; os demais, 1 a 3.
- **Re-sorteio**: quando o jogador já tem um item que não empilha (ou um CaddieItem).
- **Cupom**: tabela `pangya_papel_shop_coupon` (typeid, ativo). Tem prioridade sobre o pang.
- **Preço**: `price_normal`, também do banco.
- **Nível mínimo**: 1.
- **Arquivos do cliente**: o exe carrega `BongdariShopNormalItem.sff` (registro 0x10) e `BongdariShopRareItem.sff` (0x30), mas **eles não estão no zip**, e a REF também não lê arquivo nenhum. O nosso `LotteryConfig` (pools, pesos e preço de 900) continua sendo a fonte.

### 1.7 Erros
- REF:
  - Ela manda os 16 bits baixos do código: 0x0101 limite do dia, 0x0102 sem pang/cupom, 0x0103 falha no sorteio, 0x0104 item inválido, 0x0105 falha ao tirar o cupom, 0x0106 falha ao gravar, 0x0108 nível < 1, 0x0001 bloqueado.
  - Erro fora do canal: 0x5900100 inteiro.
- KR: 2 = "dinheiro insuficiente", 3 = "item inválido", outro = silencioso.
- A mensagem que o JP mostra para cada código não foi medida [?]. Manter 2 e 3 até testar.

### 1.8 Diferenças em relação ao KR 645
| | KR 645 | JP 983 |
|---|---|---|
| abrir | 0x95 -> 0x109 | 0x98 -> 0x10B, com conversão |
| jogar | 0x6D | 0x14B, fora das faixas (exceção já mapeada) |
| resultado | 0xD4, sem cupom | 0x21B, com `i32 cupom` depois do código (exceção 0xD4 -> 0x21B já mapeada) |
| cupom gasto | 0xD3 | 0xD5 (a REF usa 0x216) |
| jogadas | 0xF9 | 0xFB (+2) |
| Big Papel e Memorial | não existem | 0x186 e 0x17F |

## 2. Big Papel: C->S 0x186 (vazio) -> 0x26C

### 2.1 Resposta da REF
1. **0xC8** `u64 pang, u64 0` (o exe lê só o `D8`).
2. **0x216**: itens ganhos, empilhados.
3. **0xFB**:
   - com limite: `remain, current` (no normal, o 2º campo é a flag -2);
   - sem limite: `-1, -3`.
4. **0x26C**: mesmo layout do 0x21B [EXE ✓]:
   - `u32 0`;
   - **`i32 0`**: o Big não aceita cupom;
   - `u32 n` (= 10) e n × (cor, typeid, id, qtd, tipo);
   - `u64 pang`, `u64 cookie`.
   - No erro, só o `u32 código` (mesmos códigos do 1.7).

### 2.2 Regras da REF
- Nível ≥ 1. Passa pelo mesmo limite diário do normal e conta como uma jogada.
- **Custo**: `price_big`, que vem do banco. **Só pang**, sem cupom.
  - O valor não está no cliente nem na REF [?]. Proposta: `LotteryConfig.BigPapelPrice` configurável. Um padrão razoável é 10 × `PapelPrice` (9000) [?].
- **Bolas**: sempre **10**.
- **Prêmios**: mesma tabela do normal (banco), com multiplicadores do ini do servidor para cookie e raro (`PAPEL_COOKIE_ITEM_RATE`, `PAPEL_rate_RATE`). No normal, a REF usa 1.0 fixo.
- **Cor e quantidade**: iguais ao normal. Re-sorteia o que não empilha e o jogador já tem.
- **Tabela de prêmios**: o iff não tem nenhuma; a REF lê do banco.
  - Para nós: o mesmo `LotteryConfig` (Normal, Cookie, Rare). Se quiser um Big com mais raros, opcionalmente `BigPapelRarePerMille` e `BigPapelCookiePerMille`.

### 2.3 Riscos
- Não se sabe se o cliente aceita n = 10 sem limite no buffer. No KR, o normal ignora n fora de 1..5 [?].
- Com 10 itens, o 0x26C tem 2 + 12 + 200 + 16 = 230 bytes.

## 3. Memorial Shop: C->S 0x17F `u32 typeid da moeda` -> 0x216 + 0x264

### 3.1 Pacotes
- **0x264** [EXE ✓ `D4 D4 D4 D4 D4`]:
  ```
  u32 código      // 0 = ok
  u32 n
  n × 12 bytes:
    i32 tipo      // RareType 0..4; consolação = -1
    u32 typeid
    u32 qtd
  ```
  - No erro, só o `u32 código`.
  - Não leva id de item. **Quem cuida do inventário é o 0x216** (itens ganhos + moeda com -1). Por isso o 0x216 é pré-requisito do Memorial.
- **0x216** (a REF manda antes do 0x264):
  - Cabeçalho: `u32 hora unix, u32 n`.
  - n × stItem216 de 50 bytes:

    | offset | tipo | campo |
    |---|---|---|
    | 0x00 | u8 | tipo (2 = remover/atualizar na REF) |
    | 0x01 | u32 | typeid |
    | 0x05 | i32 | id |
    | 0x09 | u32 | flag de tempo |
    | 0x0D | i32 | qtd antes |
    | 0x11 | i32 | qtd depois |
    | 0x15 | i32 | qtd, ou tempo se houver |
    | 0x19 | 25 bytes | `i16[5]`, ucc, etc., zerados |

  - O laço não foi medido no exe [?: SPEC-jp983-loja §6.1].

### 3.2 Arquivos no zip (`data/pangya_jp.iff`)
Cabeçalho de cada tabela: `u16 n, u16 bind, u32 versão (13)`. Os registros começam no byte 8. Little-endian.

**MemorialShopCoinItem.sff**
- n = 14, bind = 65, registro **0x40 (64)**. Total 904 = 8 + 14 × 64 ✓.

| offset | tipo | campo |
|---|---|---|
| 0x00 | u32 | Active (1; uma moeda tem 0xFFFFFFFF) |
| 0x04 | u32 | typeid da moeda (Item.iff, grupo 6) |
| 0x08 | u32 | tipo: 0 Normal, 1 Premium, 2 Special, 3 Character |
| 0x0C | u32 | probabilidade/bônus (0 ou 100) |
| 0x10 | u32 | gacha min |
| 0x14 | u32 | gacha max (0/0 = sem faixa) |
| 0x18 | i32[10] | filtros (FilterType) |

Moedas existentes (todos os registros):

| typeid | nome (Item.iff) | tipo | prob | faixa gacha | filtros | pool (regra da REF) |
|---|---|---|---|---|---|---|
| 0x1A000273 | Memorial Coin (Normal) | 0 | 0 | — | — | 705 |
| 0x1A000272 | Memorial Coin (Premium) | 1 | 100 | — | — | 705 |
| 0x1A000274 | Memorial Coin (Special) | 2 | 0 | — | 16 (Halloween) | 11 |
| 0x1A000275 | Memorial Coin (2008 Memories) | 2 | 0 | 18–29 | — | 44 |
| 0x1A000276 | Memorial Coin (Club Set) | 2 | 0 | — | 5 (ClubSet) | 15 |
| 0x1A000277 | Memorial Coin (Girls' Swimwear) | 2 | 0 | — | 14, 18 | 494 |
| 0x1A0002AB | Memorial Coin (NL) | 3 | 0 | 1–82 | 28, 11 | **0** |
| 0x1A0002B0 | Memorial Coin (M) | 3 | 0 | 1–82 | 23, 11 | **0** |
| 0x1A0002B5 | Memorial Coin (N) | 3 | 0 | 1–82 | 19, 11 | **0** |
| 0x1A0002B9 | Memorial Coin (Halloween) | 2 | 0 | 1–95 | 16 | 11 |
| 0x1A0002C0 | Memorial Coin (A) | 3 | 0 | 1–82 | 25, 11 | **0** |
| 0x1A0002D9 | Memorial Coin (AZ) | 3 | 0 | 1–82 | 21, 11 | **0** |
| 0x1A0002E6 | Memorial Coin (KZ) | 3 | 0 | 1–82 | 26, 11 | **0** |
| 0x1A0002F3 | Memorial Coin (K) | 3 | 100 | 1–82 | 24, 24 | **0**; Active = 0xFFFFFFFF |

- FilterType da REF: 0 normal, 1–4 estações, 5 clubset, 6 setitem, 7 orelha, 8 asa, 9 luva, 10 anel direito, 11 anel esquerdo, 12 caddie, 13 mascote, 14 férias de verão, 15 natal, 16 halloween, 17 homem, 18 mulher, 19–28 personagens (Nuri, Hana, Azer, Cecilia, Max, Kooh, Arin, Kaz, Lucia, Nell…), 29+ Spika e variantes "_R".
- **As moedas de personagem não têm nenhum raro** com a regra da REF: nenhum raro usa os filtros 19–28, e os 8 com filtro 11 ficam fora da faixa 1–82. Na REF, essas moedas dão erro ("coin is empty").

**MemorialShopRareItem.iff**
- n = 705, bind = 68, registro **0x5C (92)**. Total 64868 = 8 + 705 × 92 ✓.

| offset | tipo | campo |
|---|---|---|
| 0x00 | i32 | Active (704 × 1, 1 × -1) |
| 0x04 | u32 | gacha number (1..136: "rodada" do gacha) |
| 0x08 | u32 | gacha count (136 em todos) |
| 0x0C | u32 | typeid do prêmio (todos existem nas tabelas de item ✓) |
| 0x10 | u32 | peso (999 / 1000 / 1001 / 666 / 333) |
| 0x14 | i32 | RareType: 0 comum, 1 normal asa fechada, 2 normal asa aberta, 3 raro asa fechada, 4 raro asa aberta |
| 0x18 | i32[10] | filtros (FilterType) |
| 0x40 | char[28] | código ("S01".."S6xx") |

Exemplos decodificados:

| Active | gacha | typeid | peso | tipo | filtros | código | nome |
|---|---|---|---|---|---|---|---|
| -1 | 1 | 0x10000080 | 999 | 2 | [5] | S01 | Gold Spike Hammer Set |
| 1 | 2 | 0x70010080 | 999 | 2 | [10] | S02 | Crimson Ring (R) |
| 1 | 3 | 0x24200000 | 1000 | 1 | [6, 17, 2, 14] | S03 | Greenline Swimset (N) |
| 1 | 5 | 0x10000008 | 1000 | 1 | [5] | S10 | Toy Hammer Set |
| 1 | 6 | 0x40000005 | 999 | 2 | [13] | S11 | Popo Hato Mascot |
| 1 | 7 | 0x0800080E | 1001 | 0 | [6, 17] | S12 | Angel Shoumei (N) |
| 1 | 82 | 0x0801680B | 333 | 4 | [8, 17] | — | White Guardian Wing (N) |

Estatísticas:
- **Por tipo**: 0 = 31, 1 = 249, 2 = 266, 3 = 124, 4 = 35.
- **Peso por tipo** (moeda Normal): 4,8% / 38,9% / 41,5% / 12,9% / 1,8%.
- **Grupos**:

  | grupo | itens |
  |---|---|
  | SetItem | 490 |
  | Part | 156 |
  | Mascot | 20 |
  | ClubSet | 15 |
  | AuxPart | 13 |
  | Caddie | 10 |
  | Usable | 1 |

**O que falta no zip**
- `MemorialShopNormalItem.sff`: o exe carrega, registro 0x48. É a tabela de consolação do cliente; o layout ainda não foi medido.
- `MemorialShopRareItem.sff`: registro 0x5C.
- `BongdariShop*.sff`.

### 3.3 Sorteio da REF (descrito, sem código)
1. **Montar o pool da moeda** (na carga):
   - Para cada raro: se a moeda tem faixa (min/max ≠ 0/0) e o gacha number do raro está fora dela, pula.
   - Se a moeda não tem filtros, entra.
   - Se tem, entra quando algum filtro do raro (≠ 0) está na lista da moeda.
   - Quantidade = 1.
   - A REF não olha o Active de nenhuma das duas tabelas.
2. **Nível do Memorial**: pontos de conquista / 300, até 24, com a faixa de gacha no banco (`pangya_new_memorial_level`). **Na prática a REF ignora** (todo raro do pool entra).
3. **Roleta**: cada raro do pool entra com o seu peso. Seja L a soma dos pesos.
4. **Taxa**:
   - `r = MEMORIAL_RATE / 100` (ini do servidor; 100 = 1,0).
   - Se a moeda tem probabilidade > 0: `r += prob × 4 / 100`. Na Premium, r = 1 + 4 = 5.
   - `L' = L × 4 / r`.
5. **Consolação**: os itens de consolação do mesmo tipo (Premium usa o grupo 1; as outras moedas, o grupo 0) entram na roleta com peso `L' / nº de consolações` cada.
   - Resultado: chance de raro = 1 / (1 + 4/r).
   - **20%** nas moedas normal, special e character (r = 1); **≈ 55,6%** na Premium (r = 5).
6. **Sorteio**:
   - Sorteia **1** prêmio.
   - Se for consolação, ela é um conjunto e entrega todos os itens dele (tipo -1).
   - Se for raro, o item é re-sorteado quando não empilha (ou é CaddieItem) e o jogador já tem.
7. **Entrega**:
   - SetItem é expandido nas peças, pulando as que o jogador já tem.
   - Mascote com prazo: a quantidade vira dias.
   - CaddieItem sem o caddie dá erro.
   - Item único que o jogador já tem dá erro.
   - Gasta 1 moeda.
   - Grava log de raro (tipo > 0).
   - Incrementa conquistas: 0x6C4000B2 normal, 0x6C4000B3 special, 0x6C4000B5 tipo 0–2, 0x6C4000B4 tipo ≥ 3.
8. **Pacotes**: 0x216 (itens + moeda -1), depois 0x264 com os prêmios sorteados: tipo, typeid e qtd do conjunto ou do raro.

### 3.4 Consolação sem banco (proposta)
O zip não tem a tabela, e a REF usa `ProcGetMemorialNormalItemInfo`, com linhas `(set_typeid, tipo 0/1, item_typeid, qtd)`.

Proposta:
- **(a) [recomendado]** `MemorialConfig.Consolation`: lista de conjuntos, cada um com `Premium` (bool) e `Items` [(typeid, qtd)], na mesma forma de `PrizeConfig`. Padrão:
  - grupo 0: consumíveis do pool `Normal` do Papel (0x18000000–08, 1 a 3 cada);
  - grupo 1 (Premium): consumíveis do pool `Cookie` (0x1800000E, 0x1A000011, 0x1A000040, …) em conjuntos de 2 ou 3 itens.
- **(b)** Usar os 31 raros de tipo 0 ("comum") como consolação. Não é recomendado: são roupas (Angel Shoumei, Baseball Set) e já fazem parte do pool raro.
- **(c)** Mais tarde, extrair `MemorialShopNormalItem.sff` (0x48) de um pak do cliente. Os nomes não aparecem em claro nos paks/exe do `E:\PangYa_Client_JP_987` (busca ASCII sem resultado).
- **Moedas de personagem** (pool vazio): em vez de erro, dar só consolação. Ou, opcionalmente, aceitar os raros com o filtro do personagem ignorando a faixa 1–82 [?].
- Taxa de raro configurável em `MemorialConfig.RarePercent` (padrão 20) e `PremiumRarePercent` (padrão 55), em vez do cálculo com o ini.

### 3.5 Códigos de erro (REF, 16 bits baixos)
| código | causa |
|---|---|
| 0x0001 | bloqueado |
| 0x0301 | typeid 0 |
| 0x0302 | não é item (grupo 6) |
| 0x0303 | não tem a moeda |
| 0x0304 | moeda fora do iff |
| 0x0305 | moeda fora do sistema |
| 0x0306 | nada sorteado |
| 0x0307 | item não inicializa |
| 0x0308 | set vazio |
| 0x0309 | CaddieItem sem caddie |
| 0x0310 | já tem o item |
| 0x0311 | falha ao tirar a moeda |
| 0x0312 | falha ao gravar |
| 0x6300300 inteiro | erro de outro sistema (ex.: moeda sem itens) |

- O texto que o JP mostra para cada código não foi medido [?].
- Hoje recusamos tudo com `0x264 u32 1`.

## 4. O que implementar (em ordem)

1. **[EXE ✓] 0x44: Papel `0xFFFF, 0xFFFF, 0`** no tradutor JP (`JpTranslator.cs`, toClient[0x42], hoje `Zeros(6)`). Só a camada JP; uma linha.
2. **[?] Papel normal com cupom**: conferir no cliente se 0xD5 + `i32 0` no 0x21B desconta o cupom 1 vez.
   - Se não descontar, passar a guardar o id do 0xD3/0xD5 no tradutor, pôr no 0x21B e parar de mandar o 0xD5.
   - Só a camada JP.
3. **[EXE ✓ cabeçalho / ? laço] Escritor do 0x216** (stItem216 50) na camada JP, a partir das mudanças do `ShopDraft` (typeid, id, qtd antes/depois, prazo).
   - Domain: expor a lista de mudanças do commit, ou seja, `Granted` + os itens editados com a quantidade antiga. Não depende de versão.
   - Pré-requisito do Memorial; serve também para Lolo, oficina e cards.
4. **[EXE ✓] Big Papel** (0x186 -> 0xC8? + 0xFB + 0x26C).
   - Domain: `LotteryService.PlayBigPapelAsync(Player)` (10 bolas, só pang, `BigPapelPrice`, mesmos pools, mesma transação, mesmo resultado `PapelResult`) e `LotteryConfig.BigPapelPrice` e `BigPapelBalls = 10`.
   - JP: handler JP-only assíncrono. Hoje o `fromClient` do tradutor é síncrono, então é preciso um gancho no `JpGameHandler.OnPacketAsync` para ids só do JP que chamam o Domain.
   - Resposta: `0x26C u32 0, i32 0, u32 n, n × bola, u64 pang, u64 cookie` + `0xFB -1, -1` (ou `-1, -3`).
   - Sem 0x216 se o id na bola já basta, como no normal [?].
5. **[EXE ✓] Memorial Shop**.
   - Domain (sem versão):
     - `MemorialService` com uma interface de dados `IMemorialData`: moedas (typeid, tipo, prob, faixa, filtros) e raros (typeid, peso, tipo, gacha, filtros).
     - `MemorialConfig`: consolação, taxas, regra das moedas de personagem.
     - Fluxo: monta os pools uma vez; sorteia 1; entrega pelo `ShopService.GrantAsync` (já expande SetItem e pula peças possuídas); gasta 1 moeda; tudo na mesma transação.
     - Mascote: quantidade vira dias. CaddieItem precisa do caddie (o `GrantAsync` já devolve Fail).
     - Resultado: `MemorialResult(code, List<(int tipo, int tid, int qtd)>, mudanças)`.
   - JP: `Jp983GameData` lê `MemorialShopCoinItem.sff` e `MemorialShopRareItem.iff` (offsets do 3.2) e implementa `IMemorialData`. O KR 645 não tem Memorial (null).
   - Handler JP-only para o 0x17F: manda 0x216 e depois 0x264.
6. **[?] Limite diário** (opcional): campos no `Player` e config; 0x10B, 0xFB e 0x44 com os valores reais.
7. **[?] Códigos de erro do JP**: testar 2/3 do KR e os códigos da REF (0x102, 0x303…) no cliente e anotar as mensagens.

### Riscos
- **0x216**: o laço não foi medido no exe; o layout de 50 bytes vem só da REF. Se estiver errado, o cliente pode cair ao ler. Testar primeiro com 1 item.
- **Item duplicado**: se o 0x216 for mandado junto com bolas que têm id, o cliente pode criar o item 2 vezes (aviso da REF). Escolher um caminho só por pacote.
- **Big Papel com 10 bolas**: não se sabe o limite de n que o cliente aceita.
- **Moedas de personagem**: pool vazio no iff (decisão de produto).
- **Active = -1**: aparece em 1 raro e 1 moeda e o significado é incerto. A REF ignora; proposta: tratar como ativo.
- **Consolação inventada**: o cliente pode mostrar silhuetas e textos próprios da `MemorialShopNormalItem.sff`, diferentes do que entregamos (só visual, imagino) [?].
- **Pools do Papel**: o exe tem as `BongdariShop*.sff`, que não estão no zip. As silhuetas da janela podem não bater com os nossos pools (só visual).
