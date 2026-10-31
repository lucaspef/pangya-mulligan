# JP 983 × KR 645: correio, perfil, guilda, mensageiro, ranking e pacotes do login

Pesquisa de 12/10/2026. Base para as próximas etapas (correio, perfil, mensageiro e ranking do JP).

Marcas:
- **[EXE]**: leitura ou escrita do exe 983 (`docs/server/jp983-protocolo/pacotes_recebe_983.tsv` = R, `pacotes_envio_983.tsv` = E).
- **[R]**: referência JP (`<ref>/Server/JP/...`, que funciona com o 983).
- **[KR]**: código atual (`src/Pangya.Protocol.KR645/...`) e as SPEC do KR.
- **[?]**: incerto, conferir no cliente.

Notação: `str` = u16 tamanho + bytes. `char[n]` = string fixa. Os ids JP são os do cliente 983.

## 0. Resumo

| Tema | Situação | Como resolver |
|---|---|---|
| Correio | O JP usa C->S **0x143–0x147** e S->C **0x210–0x216**. Nenhum deles passa pelas faixas do tradutor. | Camada JP própria (ou conversão no `JpTranslator`) com o `MailService` do Domain. Structs: carta da lista 194→**196**, anexo 53→**55**. |
| Perfil | Todos os ids caem nas faixas: KR 0x14E–0x156 → JP 0x156–0x15E (+8), KR 0x87 → JP 0x89. O que muda são as structs. | Montar o perfil JP a partir do Domain (`JpStructs`), interceptando o C->S 0x2F no JP. |
| Subida de nível | KR 0x10D cai fora das faixas. No JP é **0x10F**, com outro layout. | Conversão explícita. |
| Guilda | O 983 **não tem gestão de guilda no cliente**: nenhum C->S 0xFE–0x116 de guilda é escrito no exe. A guilda ("club") é uma página web. | 0xFB → 0x1AD (chave web), 0xA1/0xA2 (entrar e sair da web) e o site `on_client/club`. |
| Mensageiro | Mesmos ids C->S. S->C = KR +2 (hello **0x2E**, login **0x2F**, sub **0x30**). O amigo tem 141→**145** bytes e o sub 0x115 mudou. | Listener JP próprio (registro "messenger-jp"), lista no login 0x09 e no game 0x8B→0xFC. |
| Ranking | Mesmos C->S. S->C = KR **+0x1194** (hello 0x1388). A ficha usa o personagem de 513. | Listener JP próprio (porta e registro separados) e endereço no game 0x47→0xA2. |
| Pacotes do login | 0x16E = presença (→0x248), 0x140 = entrar na loja (→0x20E), 0xFB = chave web (→0x1AD), 0xFE = UCC (→0x1B1). | Respostas mínimas na §6. |
| "Lounge 0x16E" | O KR 0x16E é o **avatar do My Room**. No JP ele é o **0x168**. | `Slot` + `Character` (as conversões já existem). |

**Achado sobre o tradutor de hoje.** Nenhum pacote some por engano: os pacotes de correio do KR saem com id errado (0x13E–0x145 → 0x146–0x14D), mas só existem se o KR receber 0xBB–0xBF. Esses ids vêm do JP 0xC3–0xC7 (−8), que o 983 **não envia** [EXE E]. Mesmo assim, ponha os ids KR 0x13E–0x145, 0x15C–0x15E e 0x16E num bloqueio explícito ao fazer a camada JP. O JP 0x146–0x14D e o 0x164/0x165 ainda são lidos (correio antigo: 0x147/0x149/0x14B/0x14D `D1`, 0x165 `D4`) [EXE R].

---

## 1. Correio

### 1.1 Structs JP

**Anexo (EmailInfo.item, 0x37 = 55)** [R `pangya_game_st.cs:4420`] [EXE: `DB(0x37)` no 0x212 e no 0x145]

| off | tipo | campo (nome da referência) | KR sMailIncludeItem (53) |
|---|---|---|---|
| 0 | i32 | id (guid do item) | 0 dwIDX |
| 4 | u32 | typeid | 4 dwTID |
| 8 | u8 | flag_time | 8 btItemType |
| 9 | u32 | qntd | 9 iCount |
| 13 | u32 | tempo_qntd | 13 iTimeCount |
| 17 | u64 | "pang" [?] | 17 soCom[5] (10) + slots 2+2 + dwSetTID 4 + iSetItemCount 4 = 22 bytes |
| 25 | u64 | "cookie" [?] | ↑ |
| 33 | i32 | "gm_id" [?] | ↑ |
| 37 | u32 | "flag_gift" [?] | ↑ (o KR acaba aqui, no 38) |
| 41 | char[9] | ucc_img_mark | 39 csUCCIndex |
| 50 | 3 bytes | ? | 48 reserved30 (4) + 52 reserved34 (1) |
| 53 | i16 | type | — |

Conversão KR→JP: copiar os bytes 0..16, zerar 17..40 [?] (os nomes da referência são palpites: os upgrades do taco, `soCom`, se perdem), UCC 39→41 e zerar o resto. No JP→KR (anexos do 0x145): copiar 0..16, UCC 41→39 e zerar o resto. **Conferir no cliente** um taco com upgrade enviado por carta.

**Carta da lista (MailBox, 0xC4 = 196)** [R `pangya_game_st.cs:4571`] [EXE: `DB(0xc4)` em 0x210/0x211/0x215]

| off | tipo | campo | KR sMailInfoBrief (194) |
|---|---|---|---|
| 0 | i32 | id | igual |
| 4 | char[30] | remetente | KR char[22] + início do reserved |
| 34 | char[80] | trecho do texto | reserved (o KR manda zeros) |
| 114 | 18 bytes | ? (zeros) | reserved |
| 132 | u32 | nº de leituras | reserved |
| 136 | u8 | lida | **0x88 bRead, mesmo offset** |
| 137 | u32 | nº de anexos | **0x89 itemCount, mesmo offset** |
| 141 | anexo 55 | 1º anexo (zeros se não houver) | 0x8D anexo 53 |

Conversão: os bytes 0..140 são iguais. Basta converter o anexo 53→55. O trecho do texto (34..113) pode vir do Domain (`MailLetter.Message`): o KR deixa vazio. 20 cartas por página e até 300 não lidas no aviso [R `NUM_OF_EMAIL_PER_PAGE`, `LIMIT_OF_UNREAD_EMAIL`].

### 1.2 Pacotes

| ação | JP C->S | KR C->S | resposta KR | resposta JP |
|---|---|---|---|---|
| listar | **0x143** `u32 página` (1-based) [EXE E: `E4`, post_recivedlg ×5] | 0xBC `u32 página` | 0x140 `u32 páginas, u32 página, u32 n, n×194` / 0x141 `u8 código` | **0x211** `i32 0, i32 página, i32 páginas, i32 n, n×196` [EXE R `D4 D4 D4 D4 DB(0xc4)`]; erro: `i32 código` (sem o resto). **Atenção: a ordem página/páginas é invertida em relação ao KR.** |
| ler | **0x144** `u32 id` [R enum; EXE E: id calculado em `FUN_005b3d30`, `E4`] | 0xBD | 0x142 `u32 id, str de, str data, str texto, u8 tem anexo, [u32 n, n×53]` / 0x143 `u8` | **0x212** `i32 0, i32 id, str de, str data, str texto, u8 flag, i32 n, n×55` [EXE R `D4 D4 DS DS DS D1 D4 DB(0x37)`]. A referência põe 55 zeros quando n = 0. O u8 seria "mostra os itens" (1/0) [R, ?]. Data no formato `dd/MM/yyyy` [R]. Erro: `i32 código`. |
| pegar anexo | **0x146** `u32 id` [R enum; EXE: mesmo `FUN_005b3d30`] | 0xBF | 0x144 `u32 n, n×53` (+0xC6 pang) / 0x145 `u8` | **0x216** itens novos (§1.3) + **0x214** `i32 0` [EXE R `D4`]. Erros da referência: 1 sem anexo, 2 falha ao pôr no inventário, 3 item inválido. Pang: **0xC8** `u64 pang, u64 gasto` (KR 0xC6 +2, já passa pelo tradutor). |
| apagar | **0x147** `u32 n, n×u32 id, u32 página` [EXE E `E4 E4 E4`] | 0xBE (mesmo layout) | 0x15C vazio / 0x15D `u32 9` | **0x215** `i32 0, i32 página, i32 páginas, i32 n, n×196`: a página **de novo, já sem as apagadas** [EXE R]. Erro: `i32 código`. O KR não tem a lista aqui: o handler JP precisa listar de novo. |
| enviar | **0x145** [EXE E `E4 E4 ES ES ES E8 E1 EB(0x37)`, taskmain `OnPostSendDlgResult983`] | 0xBB `u32 eu, u32 destino, u8 n, i64 taxa, str "", str texto, n×53` | 0x13E `u32 n, n×53` (o que saiu) + 0xC6 / 0x13F `u8 código` | **0x213** `i32 0` [EXE R: `D4`, e no sucesso o cliente manda **MSN 0x29 `u32, u32`** avisando o destinatário]. A referência manda antes **0x216** (itens que saíram) e **0xC8**. Erro: `i32 código`. |
| aviso de não lidas | **0xE4** sem dados (o cliente manda ao receber **0x33** ou **0x38** [EXE R]) | 0xE6 (já passa pelo tradutor: +2) | 0x15E `u32 n, n×194` | **0x210** `i32 0, i32 n, n×196` [EXE R `D4 D4 DB(0xc4)`]. A referência manda o 0x210 também no login (cache do correio) e quando chega carta para quem está online. |
| carta nova (online) | — | — | 0x31 vazio | **0x33** vazio (KR 0x31 +2, já passa) → o cliente pede com 0xE4. Ou mande o 0x210 direto, como a referência. |
| busca de nick | 0x07 `u8, str nick` (igual) | 0x07 | 0x9F `u8 0, u32 uid, sPangYaUserInfo 269` | **0xA1** `u8 0, u32 uid, bloco 301` [EXE R `D1 D4 DB(0x12d)`]. Bloco = MemberInfo JP 297 + u32 uid (§2.1). **A referência manda só 297**, o que deixa o cliente lendo 4 bytes além. Hoje o KR 0x9F sai como 0xA1 com 269: **converter** [?]. |

Corpo do 0x145 (JP): `u32 meu uid, u32 uid destino, str nick destino, str "" [?], str texto, u64 taxa, u8 n, n×55`. A referência lê o segundo `str` como `u16` (= string vazia). Taxa no cliente: 100 sem item, 500 × n com item [R]. Até 4 itens [R].

### 1.3 0x216: inventário mudou (correio, presente) [R `pacote216`] [EXE R: só `D4 D4` no despachante, o laço não aparece]
`i32 hora (unix), i32 n`, e por item: `u8 tipo, u32 typeid, i32 id, u32 flag_time, i32 qtd antes, i32 qtd depois, i32 qtd ou tempo`. Se tipo == 2: `+ str ucc, u32 status ucc, u32 seq ucc, 5 bytes 0`. A referência usa tipo 2 em quase tudo [R, ?]. Com n = 0, a referência escreve só `i32 option` depois da hora.

### 1.4 Como implementar
1. **No JP** (recomendado): um `JpMail` no `JpGameHandler`/`JpTranslator.FromClient` que trata 0x143–0x147 e 0xE4 direto no `MailService`/`Store` (os mesmos métodos do `GameHandler.Mail.cs`: `ListAsync`, `GetAsync`/`MarkReadAsync`, `TakeAsync`, `DeleteAsync`, `SendAsync`, `UnreadAsync`) e responde em JP. O handler é assíncrono e o tradutor é síncrono: tratar no `JpGameHandler.OnPacketAsync` antes do `Kr.OnPacketAsync`, como a checagem do login.
2. No login do game JP, mande o **0x210** com as não lidas (o KR já manda 0x15E; o tradutor pode convertê-lo para 0x210 em vez de descartar).
3. Conversões que o tradutor pode fazer sozinho, sem a camada JP: KR 0x15E → JP 0x210 (`i32 0` + n + carta 194→196). As outras respostas dependem do pedido (página, id), então ficam melhor na camada JP.
4. A subida de nível manda carta (§2.4): sem o correio JP, o presente fica "invisível" no JP.

---

## 2. Perfil de outro jogador

### 2.1 Fluxo
C->S **0x2F** `u32 uid, u8 temporada` [EXE E `E4 E1`: user_info, scoreboard, golftask]. É o mesmo do KR e já passa (≤0x82).

Ordem que a referência manda (`GameService.requestPlayerInfo`) e o que o KR manda hoje (`GameHandler.MyRoom.cs:UserInfoAsync`):

| JP | layout JP [EXE R] | tamanho | KR (id → JP pelo +8) | conversão |
|---|---|---|---|---|
| **0x157** | `u8 temp, u32 uid, u16 sala, MemberInfo 297, u32 uid, u32 pontos de guilda` (`D1 D4 D2 DB(0x12d) D4`: o exe lê 297+uid como bloco de 301) | 312 | 0x14F `u8, u32, u16 sala, sPangYaUserInfo 269 (já termina com dwUID), u32 pontos` | 269 → 297+4. Mapa abaixo. |
| **0x15E** | `u32 uid, CharacterInfo 513` (`D4 DB(0x201)`) | 517 | 0x156 `u32, sCharacterInfo 444` | `KrToJp.Character` (está em `NotConvertedYet`: falta só ligar o conversor) |
| **0x156** | `u8, u32, UserEquip 116` (`D1 D4 DB(0x74)`) | 121 | 0x14E `u8, u32, sUserEquip 108` | mesma ordem; + 8 zeros (2 pôsteres) no fim |
| **0x158** | `u8, u32, UserInfo 265` (`D1 D4 DB(0x109)`) | 270 | 0x150 `u8, u32, stats 235` | ver §2.2 |
| **0x15D** | `u32 uid, GuildInfo 77` (`D4 DB(0x4d)`) | 81 | 0x155 `u32, GUILD_INFO 297` | GuildInfo JP: `i32 id, u8 líder?, char[20] nome, char[12] emblema, u32 nº emblema, u64 ?, u64 pang, 16 bytes ?, u32 pontos` [R] |
| **0x15C** (×3) | `u8 tipo, u32 uid, i32 n, n×43 normal, i32 m, m×43 assist` (`D1 D4 D4 DB(0x2b) D4 DB(0x2b)`) | | 0x154 `u8 tipo, u32 uid, u16 n, n×43` | cabeçalho diferente (i32 + 2ª lista). Tipos [R]: temporada (normal), 0x33/0x0A natural, 0x34/0x0B grand prix. Ref: 22 mapas (`JpStructs.MapCount`). |
| **0x15B** | `u8, u32, u16 n, n×4` (troféus de guilda) | | 0x153 (igual) | só id |
| **0x15A** | `u8, u32, u16 n, n×12` (troféus especiais `i32 id, u32 typeid, i32 qtd`) | | 0x152 (igual) | só id |
| **0x159** | `u8, u32, troféus 78` (`D1 D4 DB(0x4e)`) | | 0x151 (igual) | só id |
| **0x257** | `u8, u32, u16 n, n×12` (troféus do Grand Prix) (`D1 D4 D2 DB(0xc)`) | | — | novo: mandar n = 0 |
| **0x89** | `u32 código` (1 ok, 3 = GM: sem permissão, 0 = erro sem resto), `[u8 temp, u32 uid]` (`D4 D1 D4`) | | 0x87 (igual) | só id (já passa: KR 0x87 +2) |

**MemberInfo JP (297) a partir do sPangYaUserInfo KR (269)** [R `pangya_game_st.cs:721`; KR `Structs.g.cs:36`]:

| JP off | campo JP | KR off | campo KR |
|---|---|---|---|
| 0 | char[22] id | 0 | sID |
| 22 | char[22] nick | 22 | sNick |
| 44 | char[17] guilda | 44 | sGuild char[21] (cortar) |
| 61 | char[12] emblema | 65 | szEmblemName |
| 73 | char[35] comentário | — | zeros |
| 108 | u32 escola | 77 | school |
| 112 | u32 capacidade | 81 | dwIdentity |
| 116 | u32 galeria | 85 | dwGalleryGuid |
| 120 | i32 oid | 89 | dwGuid |
| 124 | u32[3] ranking | 93 | dwRank[3] |
| 136 | u32 guilda | 105 | dwGuildId |
| 140 | u32 nº do emblema | 109 | dwEmblemVer [?] |
| 144 | u8 estado | 113 | bits (tutorial, sexo…) [?] |
| 145 | u16 login (2 = não é o primeiro) | — | 2 |
| 147 | u16[3] papel shop | 115 | i16 ×3 |
| 153 | u32 pontos de evento | 121 | dwPointPointEvent |
| 157 | u64 bloqueio | 125 | dwFlagBlock (u32) + 0 |
| 165 | u32 canalização | 133 | nChannelingFlag |
| 169 | char[128] id de exibição | 137 | sDisplayID |
| 297 | (u32 uid, fora do MemberInfo) | 265 | dwUID |

O mesmo bloco de 301 serve ao 0xA1 (busca de nick do correio e do presente).

### 2.2 Estatísticas KR 235 → JP 265
Prefixo comparado com `JpStructs.UserInfo` e `sPangYaUserStatistics`:
- KR 0..43 = JP 0..43.
- KR hole in one u16 @44 → JP **i32** @44. KR bunker u16 @46 → JP i16 @48. KR fairway @48 → JP @50. KR albatross u16 @52 → JP **i32** @54.
- KR 54..188 → JP 58..192 (+4): hole in, putt in, distâncias, EXP, nível, pang, placar, melhores placares 6, melhor pang 6×8, jogos, 3×u32, ladder 5, combo 3.
- Final [?]: KR `skins i64 + 3 u32 + strike i32 (24) + allIn u16 + eventValue + eventFlag + temporada + i64 soma` × JP `pang battle 8+24, i16, i32 desconexões, medalhas 24, i32 escola antiga, i32 jogos na temporada, i16`.

Para o próprio jogador o tradutor já refaz do Domain (KStats). **Recomendação**: tratar o 0x2F no `JpGameHandler` e montar os 12 pacotes do Domain com `JpStructs` (`MemberInfo`, `UserInfo`, `Trophies`, `UserEquip`, `KrToJp.Character`, `MapStat`), usando a mesma busca do alvo do KR (online, bot, banco). Isso evita converter byte a byte as estatísticas e o MemberInfo, e dá os 22 mapas.

### 2.3 Ordem
A referência manda 0x157 primeiro (cria a entrada) e o 0x89 por último. O KR manda 0x14F primeiro e o 0x87 por último: a mesma ordem.

### 2.4 Subida de nível no fim da partida
- KR **0x10D** `sLevelUpDone {u8 feito, u8 nível, u8 tipo}`: fora das faixas, hoje não vai.
- JP **0x10F** [R `Player.cs:157`, enum `SERVER_LEVELUP_ITEM`] [EXE R `D4` no despachante; o resto deve ser lido pela janela, ?]: `i32 0 (ok), u8 quantos níveis subiu, u8 nível novo`.
- Conversão: o KR só manda o nível novo. Ou o KR passa o "de" (por exemplo, guardar `levelBefore` no handler) ou o tradutor lembra o último nível enviado (`Player.Level` antes do fim). Na dúvida, mande 1 [?].
- O presente vai por carta (KR `LevelUpGiftsAsync` + 0x31): depende do correio JP (§1).
- Fora da partida (EXP de carta, ticket report), a referência atualiza com **0x1D9** `u32 nível, i32 EXP` [EXE R `D4 D4`].
- Cuidado: o JP 0x10D é outro pacote (`OPEN_EVENT_GIFTBOX`, `D1 DB(0x14)`). Nunca mande o KR 0x10D com id +0/+2.

---

## 3. Guilda no JP 983

- Os C->S do KR 0xFE–0x113 **não têm par**. O 983 não escreve nenhum C->S de gestão de guilda (busca em `pacotes_envio_983.tsv`: só 0x128/0x129 do `guildactor.cpp`, que é o ator de guilda no jogo). Também não há leitura de 0x1B5–0x1D1 no exe [EXE]. O enum da referência (`CLIENT_NEW_GUILD_MNGR_* 0x101–0x116`, `SERVER_NEW_GUILD_MNGR_* 0x1B5–0x1D1`) tem nomes "próprios" do autor ("a partir daqui são meus próprios nomes"), e a referência **não registra handler** para eles.
- No JP a guilda ("クラブ", club) é **web**: `https://www5.pangya.jp/on_client/club/login.aspx?ID=%s&KID=%s` [EXE `rede_983.tsv`, usada por `CTaskMain::FUN_00659240` e `FUN_008aa150`]. O emblema é baixado de `http://www5.pangya.jp/upload/club/mark/`. O rugburn reescreve as URLs (UrlRewrites; porta web 30090, segundo o dono).
- Pacotes do game em volta da web:
  - **0xFB** (sem dados) → **0x1AD** `i32 1, str chave web` (erro: `i32 0, u16 0`) [R `packet0FB`; EXE R `D4 DS`]. É o `KID` das URLs.
  - **0xA1** `u8 lugar` [EXE E `E1`, browser.cpp] = "entrou na página web" [R `requestEnterWebLinkState`: só guarda o lugar]. Hoje é descartado: 0xA1 fica entre 0x9D e 0xAD, fora das faixas C->S. Não tem resposta.
  - **0xA2** (sem dados) = "saiu da guilda web" [R `requestExitedFromWebGuild`]: o servidor relê o pang (→ **0xC8** `u64 pang, u64 0`) e as mudanças da guilda feitas no site. A referência avisa o mensageiro por um canal interno: 0x01 aceito / 0x02 saiu.
  - 0xA0 e 0xA4 (browser.cpp) [EXE E]: sem handler na referência [?].
- Dados da guilda que o cliente mostra: MemberInfo (guilda, emblema, nº do emblema), jogador no lobby (+u32 nº do emblema), sala, perfil 0x15D. Tudo isso já existe nos conversores.
- **Conclusão**: para o JP, o `GameHandler.Guild.cs` só serve como Domain (`GuildService`). A interface é um site (`/on_client/club/…`) no servidor web, autenticado pela chave do 0x1AD. O chat de guilda continua no mensageiro (C->S MSN 0x25).

---

## 4. Mensageiro JP

### 4.1 Como o cliente acha o servidor
- Login **0x09**: `u8 n, n × ServerInfo 92` [EXE R CLobbyTask `D1 DB(0x5c)`]. É o mesmo formato do `JpLoginHandler.ServerList`. Hoje vai `u8 0`: troque por `ServerList(SMessengerList, Registry.ListAsync("messenger-jp"))`.
- Game C->S **0x8B** (sem dados) [EXE E msnunit.cpp, GAME] → S->C **0xFC** `u8 n, n×92` [EXE R]. Pelo tradutor: JP 0x8B → KR 0x88 → KR 0xFA → JP 0xFC. **Já funciona**, mas o KR lista o registro `"messenger"` (o mensageiro do KR, hello 0x2C). O JP precisa de outro tipo de registro (`"messenger-jp"`), passado pelo `GameContext` do mundo JP. O mesmo vale para o ranking (§5).
- Porta: configurável. Hoje o KR usa 30303 (`config/pangya.json`). O JP precisa de **outra porta**, porque o servidor fala primeiro (hello) e não sabe qual cliente conectou.

### 4.2 Conexão e login
| | KR 645 (`MessengerHandler`) | JP 983 |
|---|---|---|
| hello (cru) | `0x2C u8 0, u8 0, u32 chave` | **`0x2E u8 1, u8 1, u32 chave`** [R `onAcceptCompleted`] (mesma cifra) |
| login C->S | `0x12 u32 uid, str nick` | igual [EXE E `E4 ES`] |
| resposta | `0x2D u8 0, u32 uid` (erro `u8 2`) | **`0x2F u8 0, u32 uid`** (erro `u8 1`) [R] [EXE R `D1 D4`, e o cliente responde com MSN **0x23** posição 75 + **0x14** lista] |
| sub-pacotes | `0x2E u16 sub, …` | **`0x30 u16 sub, …`** [R] [EXE R 0x30: `D2` + leituras dos subs] |
| outros S->C | 0x2F–0x3C (guilda 0x38/0x39/0x3A…) | +2 (0x31–0x3E) [?]. A referência usa 0x3B e 0x3C na entrada e na saída de membro da guilda. |

Validação do login: a referência confere uid + nick no banco e pergunta ao auth server se a conta está online. O nosso `MessengerContext.findGame` deve apontar para o **mundo JP** (`JpGameServer`).

### 4.3 C->S (ids iguais ao KR) [EXE E, destino MSN]
`0x12 E4 ES` login · `0x13 E2` (0x11F: lista para correio/presente) · `0x14` lista · `0x16` sair · `0x17 ES` busca · `0x18 E4 ES` pedir amizade · `0x19 E4` aceitar · `0x1A/0x1B E4` bloquear/desbloquear · `0x1C E4 ES` apagar · `0x1D E1` status · `0x1E E4 ES` conversa · `0x1F E4 ES` apelido · `0x21 ES E4` · `0x22 ES` (trocou o nick, depois do game 0x108) · `0x23 EB(0x4b)` posição · `0x24 E4` (convite para sala) · `0x25 ES` chat de guilda · `0x26 ES ES E4 E1` · `0x27 ES E4` · `0x28 E4 E1 E2 E4 ES E4` (convite para sala de guilda) · `0x29 E4 E4` (avisa carta/presente para uid; mandado depois do game 0x213 e do 0x11E) e `0x29 ES ES` · **`0x16B E4`** (novo, depois do game 0x117/0x119) [?].

### 4.4 Structs e sub-pacotes que mudaram
- **Amigo = 145 bytes** (KR sFriend 141) [EXE R: `DB(0x91)` nos subs do 0x30] [R `FriendInfo` 65 + `ChannelPlayerInfo` 75 + 5 bytes]. A referência: `char[22] nick, char[11] apelido, u32 uid, i32 ×7` (o 7º "só tem no JP"), posição 75, `u8 status (5 = offline), u8 ?, u8 nível (na guilda: 1 mestre/0 membro), u8 bits de estado, u8 bits de tipo`. Para o KR (`Guid, Server, GuildId, char[12] emblema` = 24 bytes) há **4 bytes a mais** antes da posição. Insira 4 zeros: depois do `dwGuildId` (como o nº do emblema no jogador do lobby) ou antes da posição [?]. Se a guilda for 0, as duas opções dão o mesmo resultado. Testar com um colega de guilda que tenha emblema.
- **0x115 (status) = 84 bytes**: `u32 uid, u32 status, u8 1, posição 75` [EXE R: `DB(0x54)` no 0x30] [R `requestUpdatePlayerState`]. No KR era `u32 status, u32 uid` (a SPEC-messenger do KR já avisava: "layout de versão nova"). A referência manda o 0x115 aos amigos no 0x1D e no 0x23, e ao próprio jogador antes da lista.
- **0x102 (lista)**: `u8 página, u16 total, u16 n, n×145` [EXE R `D1 D2 D2 DB(0x91)`]. 30 por página [R].
- **0x123 (posição)**: `u32, u32 uid, posição 75` [EXE R `D4 D4 DB(0x4b)`]. Continua existindo. A referência não usa: põe a posição no 0x115.
- **0x103 (bilhetes)**: `u8 n, n×109` [EXE R `D1 DB(0x6d)`]. Igual.
- 0x104 / 0x106: levam o amigo de 145 (converter). 0x109/0x10A/0x10B/0x10C/0x10D/0x10F/0x117/0x119: iguais [R].
- 0x113 (conversa): `u32 uid, str nick, str msg, u8 0/1` (igual). Erro da referência: `0x113 i32 -1`. Erro do KR: 0x114 `u8 3, u32 uid` [?].
- 0x10E (amigo entrou): a referência não manda. O aviso vai pelo 0x115 com status 4 [?].
- Sequência de leituras do 0x30 no exe, para conferir: `D2 | D1 D4 DS D4 D4 DB(0x91) DB(0x91) P(0x19) | D4 DS DS D1 D1 D4×9 DS DB(0x91) D4 D4 D1 D2 D2 DB(0x91) P(0x1d) | D4 D4 D4 D4 D1 DB(0x6d) D1 D1 D4 D4 DB(0x4b) DB(0x54) P(0x16) | D4 D4 D4 DS D4 P(0xa5)>GAME`.

### 4.5 Game ↔ mensageiro (já passa pelo tradutor, conferir os structs)
- Game C->S 0x3C sub 0x11F (lista pelo game) → o KR responde com 0x2E/0x102 **na conexão do game** → JP 0x30 (+2) ✓. O amigo de 141 precisa virar 145.
- Bilhete 0x3C/0x111 → KR 0x93 → JP **0x95** `u16, u32, u64` [EXE R `D2 D4 D8`] ✓.
- KR 0xB0 (bilhetes no lobby, `u8 n, n×109`) → JP **0xB2**, que o exe lê como `D4 D4 D4 DB(0x6d)` (CLobbyTask): **layout diferente** [?]. Bloquear até conferir.

### 4.6 Plano
`JpMessengerHandler` = o `MessengerHandler` do KR com: hello 0x2E/1/1; respostas 0x2F/0x30; amigo 145; 0x115 no layout novo (com posição); registro `"messenger-jp"`; e `findGame` no mundo JP. O resto da lógica (`FriendService`, bilhetes, guilda) é igual.

---

## 5. Ranking JP
| | KR (`RankingHandler`) | JP |
|---|---|---|
| endereço | game C->S 0x47 → S->C 0xA0 `str ip, u32 porta` | C->S 0x47 [EXE E `OnUnderBar_RankingUp`] → **0xA2** `str ip, i32 porta` [EXE R `DS D4`] [R]. Já passa (KR 0xA0 +2), mas aponta para o ranking KR: o JP precisa do endereço do ranking JP. |
| hello (cru) | `0x1F4 u32 chave, u8 5, str "645"` | **`0x1388 u32 chave, u8 5, str data`** [R: `UtilTime.formatDateLocal(0)`]. O formato da data e o uso dela no cliente são [?]. |
| C->S | 0x00 página, 0x01 ficha, 0x02 busca | **iguais** [EXE E: `? RANK E4 ES E1 E1 E1 E1 E4 E1` (0x00), `0x1 RANK E4 ES E1`, `0x2 RANK E1 E4 ES E1 E1 E1 E1 E4`] |
| S->C | 0x1F5 página, 0x1F6 ficha, 0x1F7 recalculando, 0x1F8 busca | **+0x1194**: 0x1389, 0x138A, 0x138B, 0x138C [R; SPEC-ranking KR] |
| página / busca | cabeçalho `u8 res, u8 tipo, u8 sub, u8 período, u8 classe, u32 pág, u32 págs, u16 n`, linha `u32 uid, u32 atual, u32 anterior, i32 valor, u16 nível, u8 classe, str id, str nick` | iguais. A linha JP é `u8 nível, u8 período, u8 classe` [R `playerInfoToPacket`]: os mesmos 3 bytes (nível < 256). Minha posição: `u8 0/1/2 + linha` (igual). |
| ficha | `u8 0, u32 uid, char[22], char[22], u8 nível, u8 0, sCharacterInfo 444, u16 0, 5×{u32,u32,i32}` | **0x138A** `u8 0, u32 uid, char[22] id, char[22] nick, u16 nível, CharacterInfo 513, u8 tem posições (0 = sim / 1 = não), N×{u32 atual, u32 anterior, i32 valor}` [R]. O JP lê um bloco fixo? O u8 do JP ocupa o lugar do u16 do KR [?]. Conversão do personagem: `KrToJp.Character`. |

O ranking não tem porta fixa no exe (vem do 0xA2). Hoje o KR usa 30474. O JP precisa de **listener separado** (outro hello e outros ids). A lógica (`RankingService`, retrato) é a mesma.

---

## 6. C->S do login que hoje são descartados

| JP C->S | onde o exe manda [EXE E] | o que é [R] | resposta | mínimo para não travar |
|---|---|---|---|---|
| **0x16E** (sem dados) | taskmain `FUN_00657200`, quitwindow, serverdlg, console | `requestCheckAttendanceReward`: presença diária | **0x248** `i32 0, AttendanceRewardInfo 21` = `u8 login (0 = ganhou hoje, 1 = já marcou), u32 tid hoje, u32 qtd hoje, u32 tid amanhã, u32 qtd amanhã, u32 contador` [EXE R `D4 P(0x16f)>GAME`: com login 0 o cliente manda **0x16F** → **0x249** `i32 0, ARI 21` (EXE R `D4`)]. A referência põe o prêmio no correio ("Your Attendance rewards have arrived!"), a cada 10 dias uma caixa papel, e dá 3 tickets de Grand Prix no primeiro login do dia. | `0x248 i32 0, u8 1, 20 zeros` [?] (sem prêmio; conferir se a janela abre vazia) |
| **0x140** (sem dados) | `CTaskMain::FUN_0065f9b0` (golfdoc) e shopmain | `requestEnterShop` | **0x20E** `i32 0, i32 0` [EXE R `D4 D4`] [R: o 2º seria valor/erro, "?"] | sempre `0x20E 0, 0`. Sem isso, a loja pode ficar esperando [?]. |
| **0xFB** (sem dados) | `FUN_008a9fa0` (perto do código da URL `on_client/club`) | chave web (`CmdGeraWebKey`) | **0x1AD** `i32 1, str chave` (erro: `i32 0, u16 0`) [EXE R `D4 DS`] | gerar a chave (a mesma do login serve [?]). É o `KID` das páginas web (guilda, gacha, pagamento, tutorial…). |
| **0xFE** `u32` | itemmanager (despachante) e shopmain | `UCCSystem.HandleUCCLoad`: UCCs (self design) do inventário. A referência não lê o u32. | **0x1B1** `u32 hora, u32 tamanho, dados` [EXE R `D4 D4 DB(?)`]. Os dados são `UCC_Load_Ctx[]` (`u32 typeid, i32 id, char[9] ucc`) comprimidos em LZO [R]. Sem UCC, a referência manda um bloco fixo: `u64 0x190132DC55, u64 0x2211000000, 13 zeros, u32 0x1100`. | copiar o bloco fixo da referência [?]. O cliente pode viver sem resposta (a referência comenta "no send response"). |

Os quatro **não existem no KR**: estão todos acima de 0xEF, então `ClientId` devolve −1 e o tradutor descarta. Trate-os no `JpGameHandler` antes do tradutor.

### Lounge / avatar
- O "avatar KR 0x16E" é o **avatar do My Room** (`GameHandler.MyRoom.cs:MyRoomPresenceAsync`, depois de C->S 0xAF = JP **0xB7** `E4 E1`, que já passa: −8). No JP é **0x168** `PlayerRoomInfo 348 + CharacterInfo 513` [EXE R CRealMyRoomTask `DB(0x15c) DB(0x201)`] [R `requestEnterMyRoom`: `pri.ToArrayEx()`]. Conversão: `KrToJp.Slot` (338→348) + `KrToJp.Character` (444→513). Tirar o 0x168 do `NotConvertedYet` e mapear KR 0x16E → JP 0x168 (fora das regras de faixa). Os móveis (KR 0x125 → JP **0x12D** `u32 1, u16 n, n×27`) já passam [EXE R `D4 D2 DB(0x1b)`].
- Lounge (sala de avatar): C->S JP **0xEB** `u32` [EXE E avatar_main] → KR 0xED (+2) ✓ → KR **0x19B** `u32 guid, 5×f32` → JP **0x196** (−5) ✓. O exe lê `D4 DB(0x4)` [EXE R] e a referência manda `i32 oid + 4×f32` (zoom da câmera, cabeça, velocidade, ?) [R `StateCharacterLounge`]. A contagem de floats é 4 ou 5 [?]. Ações do avatar: KR 0xC2 → JP **0xC4** [EXE R ntAvatarChatTask `D4 D1 DB(4)×3 D4 DB(4)×3 DS D4`] (conferir os subs).

---

## 7. Lista do que fazer (ordem sugerida)
1. `JpGameHandler`: responder 0x16E/0x140/0xFB/0xFE (§6). Rápido e tira avisos do login.
2. Correio JP (§1): 0x143–0x147, 0xE4 → 0x210–0x216, 0x210 no login, anexo 55, carta 196, busca de nick 0xA1 com 301.
3. Perfil (§2): tratar o 0x2F no JP a partir do Domain; subida de nível 0x10F.
4. My Room: KR 0x16E → JP 0x168.
5. Mensageiro JP (§4) e o login 0x09 com o registro `"messenger-jp"`. Separar o registro do game JP (0x8B→0xFC).
6. Ranking JP (§5): listener, hello 0x1388, ids +0x1194, ficha com 513. 0x47→0xA2 com o endereço JP.
7. Guilda (§3): chave web 0x1AD + site `on_client/club` (fora do protocolo do jogo).

Pontos [?] que pedem teste no cliente: os 24 bytes do meio do anexo (17..40); onde ficam os 4 bytes extras do amigo; o layout do JP 0xB2; o u8 ou u16 da ficha do ranking; a contagem de floats do 0x196; e o resto do 0x10F/0x216, que o exe lê fora do despachante.
