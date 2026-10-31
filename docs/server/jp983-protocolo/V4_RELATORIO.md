# V4 — Pacotes de rede e tabelas .iff do JP 983 conferidos contra o fonte (frente C, sessão C4, 11/10/2026)

Refazer tudo: `make v4` (15 s, só leitura do exe 983 e dos nossos .obj) → saída em `out/V4/make_v4.log`.
Asserts: `make layoutcheck U=shared_itemmanager` (31 de tabela .iff) e `U=projectg_shareddoc` (sTradeItem).

## Resumo (o que importa para o jogo funcionar)
1. **Os números (ids) dos pacotes mudaram do KR 645 para o JP 983 e o nosso fonte ainda usa os do 645.**
   - Recebidos (servidor → cliente): 146 ids alinhados; **+0 até 0x09, +2 de 0x2d a 0x107, +4 em 0x116–0x11b, +8 de 0x123 a
     0x15a, −5 de 0x172 a 0x1a2** (`ids_servidor.tsv`; 139 em faixa coerente, 7 isolados para conferir à mão).
   - Enviados (cliente → servidor): 99 ids alinhados; **+0 até 0x82, +3 de 0x88 a 0x9a, +8 de 0xa5 a 0xc2, −2 de 0xd5 a 0xf1**
     (`ids_cliente.tsv`; 95 em faixa, 4 isolados). 57 funções nossas diferem do 983 SÓ no id (`pk_cmp.tsv`, `difere_id`).
   - Efeito: com a numeração do 645 o exe compilado trataria o pacote errado em quase todo `case` acima de 0x2c.
2. **7 structs de tabela .iff com tamanho errado** (de 32 que o fonte lê): sClubSet 0xe4 (983: 0x104), sCourse 0x168 (0x1d0),
   sCadieMagicBox 0x68 (0x8c), sCard 0x178 (0x180), sBall 0x32c (0x330), sMascot 0x12c (0x130), sAuxPart 0xe0 (0xe4).
   Viraram `static_assert` (falham hoje). O 983 lê **71** tabelas; **39 não têm carregador no fonte** (sistemas novos).
3. **4 blocos de pacote com tamanho diferente**: sTradeItem 0xaa (983: 0xac), lista de troca 0x368 (0x428), presente 0x10
   (0x25), registro do CGimmickContainer 0x14 (0x24).
4. **Não há arquivos .iff do cliente 983 no container** (só os do 839 em <kr839>/local/assets/iff). O tamanho de registro
   do 983 foi tirado do código do exe; os arquivos do 839 servem de conferência (ver (b)). Pendência para o usuário, sem
   bloquear: copiar o `pangya_jp.iff` do cliente 983 para conferir pelos arquivos.

## (a) Pacotes
### Método
A API de pacote do exe 983 são 37 endereços (implementação em 005ab… + embrulhos do pimpl em 00a4d…/00a4e…; tabela `API983`
em tools/v4/pk.py). `pk.py` acha toda `call/jmp` para eles (3.927 chamadas, 505 funções, 0 perdidas pelo desmonte linear) e
escreve, por função, a **assinatura** = chamadas em ordem de endereço: `P(id)` cria pacote, `E1 E2 E4 E8 ES EX EB(n) ED`
escrevem, `>DEST` envia, `D1 D2 D4 D8 DS DX DB(n)` leem (`?` = argumento não constante). O mesmo extrator roda nos nossos
.obj (chamadas pelo nome decorado) e as duas assinaturas são comparadas função a função pelo par do comparador (`va983` do
`funcs.tsv` mais novo de cada unidade).
`rx.py` segue o `switch` do tipo do pacote em cada despachante do 983 (árvore cmp/sub/dec + tabela de saltos com tabela de
bytes) e lista id → endereço do caso → leituras no trecho; no fonte, os `case` do 1º nível do `switch (type)`.

### Números
| | |
|---|---|
| Pacotes montados no 983 (`pacotes_envio_983.tsv`) | 624 lugares, **261 ids** distintos (16 lugares sem id constante) |
| Ids recebidos tratados no 983 (`pacotes_recebe_983.tsv`) | 491 casos em 9 despachantes (OnPacketCommon 280, golfe 90, lobby 70, my room 27, avatar 13, loja 7…) |
| Funções nossas com API de pacote | 422 em 127 unidades |
| — assinatura igual à do 983 | **114** (110 + 4 com curinga) |
| — só o id difere | **57** |
| — só tamanho de bloco difere | 4 |
| — campos diferem | 38 (inclui os OnPacket, que no nosso fonte ainda têm o corpo do 645) |
| — par do comparador sem API no 983 (`so_nosso`) | 29 (par errado, ou endereço que não é início de função no mapa) |
| — sem par no comparador (`sem_par`) | 180 |
| Funções do 983 com API de pacote sem função nossa pareada (`pk_983_sem_fonte.tsv`) | 300 (79 em sistema novo do JP; lobbymain 36, realmyroommain 20…) |

Despachantes (ids do 983 × `case` do fonte, `rx_resumo.txt`): nenhum bate — o fonte tem os ids do 645; ver `rx_cmp.tsv`
(estado `so_983`/`so_fonte`/`difere`/`igual` por id, com as leituras dos dois lados).

### Arquivos
- `pacotes_envio_983.tsv` — id, destino (GAME/MSN/SHOP/LOGIN/RANK), campos, endereço, função, unidade. **Tabela id → formato.**
- `pacotes_recebe_983.tsv` — despachante, id, endereço do caso, leituras feitas no trecho do caso.
- `pk983_funcs.tsv` / `pk_nosso_funcs.tsv` — assinatura por função (983 / nossos .obj).
- `pk_cmp.tsv` + `pk_cmp_resumo.txt` — comparação função a função.
- `ids_servidor.tsv`, `ids_cliente.tsv` — conversão id do fonte → id do 983, com votos e `certeza` (faixa/isolado).
- `rx_cmp.tsv` + `rx_resumo.txt` — despachantes, id a id.

## (b) Tabelas .iff
### Método
`iff.py` lê o `CItemManager::Load` do 983 (00a3de50): cada `push "X.iff"` + stricmp é seguido da chamada ao carregador. No
carregador, o tamanho do registro é o imediato que aparece como cópia (`mov ecx,N/4; rep movsd`, com resto) e como passo do
ponteiro (`add reg,N`). O mesmo método no nosso `itemmanager.obj`; depois o compilador confirmou os 31 valores do nosso lado
(`make layoutcheck U=shared_itemmanager`: 24 ok, 7 FALHA, os mesmos 7).
Conferência com os arquivos do **839**: nas tabelas de item o 983 tem +24 bytes (o cabeçalho comum IFF_ITEM_COMMON cresceu
0x18, já anotado pela frente A na sessão 51); nas tabelas simples o tamanho é igual. Isso vale para 53 das 57 tabelas com
arquivo; exceções: SetEffectTable +20, CaddieVoiceTable (medido 0x86 = arquivo), LevelUpPrizeItem e CadieMagicBoxRandom
não medidos (dois candidatos).

### Números (`iff_983.tsv`)
71 tabelas no Load do 983 (57 .iff + 14 .sff): **ok 24, DIFERE 7, não medido 1, sem carregador no fonte 39**.
Sem fonte (para as frentes A/B, com o tamanho do registro já medido): CounterItem 0x118, Achievement 0x600, QuestStuff 0x10c,
QuestItem 0x110, ShopLimitItem 0x3c, PointShop 0x14, ScratchItem.sff 0x30, Bongdari*.sff, BroadCastNotices.sff 0x230,
TwinsItemTable 0x1c, ErrorCodeInfo 0x110, ArtifactManaInfo 0x98, ScratchShop*, Ability 0x50, ClubSetWorkShop* (5),
AttendancePrize* (2), GrandPrix* (5), AddonPart 0x120, SetEffectTable 0x4c, HoleCupDropItem 0x2c, MemorialShop* (4),
AlchemyConfig, CharacterMastery 0x3c, CaddieVoiceTable 0x86.

## Asserts
- `tools/v3/manual.tsv` ganhou 32 linhas (31 structs IFF_STRUCT::* + sTradeItem) → `include/asserts/shared_itemmanager.h`
  (novo) e `projectg_shareddoc.h`. Total do `make layoutcheck`: ver `out/V4/layoutcheck_resumo.txt`.

## Limites / provisório
- Assinatura é a ordem de ENDEREÇO das chamadas, não o fluxo: laços e ramos aparecem achatados. Serve para comparar dois
  binários do mesmo compilador; como "formato do pacote" é aproximado quando há `if`/laço.
- Leituras feitas em função chamada pelo caso (ex.: `OnXxx(packet)`) não entram no trecho do caso.
- A conversão de ids é por alinhamento de assinaturas, não por prova id a id: os 11 `isolado` (ex.: 0x16f → 0x138a, que é
  lixo do leitor de switch) devem ser conferidos; os de `faixa` têm vizinho com o mesmo salto.
- Lado do fonte nos despachantes = texto do .cpp (os OnPacket têm corpo `naked`/funclets herdado do rebang).
- `sem_par` (180) e `so_nosso` (29) não foram comparados; dependem do mapa da frente A.
- O Ghidra não foi usado no resultado (um pseudo-C dos despachantes foi começado e descartado).
