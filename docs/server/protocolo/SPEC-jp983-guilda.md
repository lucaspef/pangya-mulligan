# JP 983: guilda ("クラブ", club)

Pesquisa de 12/10/2026. Nada foi alterado no repositório. Continua a §3 de `correio-perfil-social.md` (copiada em
`docs/server/protocolo/SPEC-jp983-correio-social.md`) e a corrige num ponto: o 983 **tem** uma parte de guilda no cliente
(convites e pedidos de entrada, pelo mensageiro), além do site.

Marcas:
- **[EXE ✓]**: conferido no exe `<pasta do jogo>\ProjectG.exe` (desmontagem com capstone; endereços abaixo) ou
  igual à assinatura de `docs/server/jp983-protocolo/pacotes_recebe_983.tsv` (R) / `pacotes_envio_983.tsv` (E).
- **[REF]**: referência AGPL (`<ref>/Server/JP/...`), só comportamento.
- **[KR]**: código atual (`src/Pangya.Protocol.KR645/Game/GameHandler.Guild.cs`, `src/Pangya.Domain/Guilds`,
  `src/Pangya.Web/WebServer.cs`) e `docs/server/protocolo/SPEC-guilda.md`.
- **[?]**: incerto, conferir no cliente.

Notação: `str` = u16 tamanho + bytes; `char[n]` = string fixa; `doc` = objeto global do cliente (`[0xe47e2c]`). Ids são os
do JP. Ferramentas usadas: `<scripts locais>/tools/disa.py exe ini-fim` (desmonta e mostra strings) e `<scripts locais>/tools/xref.py
exe alvo` (quem chama / quem usa o endereço).

---

## 0. Resumo

| O que | KR 645 | JP 983 |
|---|---|---|
| Criar, checar nome, trocar nome, lista/busca, informações, membros, histórico, cargo, mensagem pessoal, aviso, apresentação, sair, expulsar, dissolver, emblema | no **game server**, C->S 0xFE–0x113, S->C 0x1B3–0x1C8 (resposta = pedido + 0xB5) | **no site** `https://www5.pangya.jp/on_client/club/...`, dentro do navegador do jogo. **Nenhum** C->S 0xFE–0x116 de guilda no exe, nenhuma leitura de 0x1B3–0x1D1 [EXE ✓] |
| Entrar no site | — | 0xFB → **0x1AD** `u32 1, str chave` → abre `login.aspx?ID=<login>&KID=<chave>` (800×600) e manda **0xA1** `u8 5`; ao fechar manda **0xA2** (vazio) [EXE ✓] |
| Pedidos de entrada e convites pendentes (para o mestre/submestre) | lista de membros com cargo 9 (0x10F → 0x1C4), aprovar/recusar 0x10B/0x10C | **0xA5** (vazio) → **0x116** `u16 n, n × 55`; aprovar/recusar **0xA7** `u32 uid, u32 guilda, u8 sim` → **0x117** [EXE ✓] |
| Convidar um jogador | não existe | mensageiro: **MSN 0x26** `str meuLogin, str loginAlvo, u32 guilda, u8 1` (u8 0 = cancelar o convite) → sub **0x126** (resultado); o convidado recebe sub **0x127** [EXE ✓] |
| Convites recebidos | não existe | **0x118** `u16 n, n × 44` (o servidor empurra; não há C->S que peça); aceitar/recusar **0xA8** `u32, u32, u8 sim` → **0x119** [EXE ✓] |
| Avisar mudança na guilda | 0x1BD GUILD_USER_INFO; 0x3B/0x3C (emblema/nome); MSN 0x38/0x39/0x3A | nenhum desses existe no JP [EXE ✓]. O cliente só tem: sub **0x128** `u32 guilda` (gestor relê a lista com 0xA5) e o que vem no MemberInfo/lista do lobby/sala/perfil |
| Emblema | upload HTTP pelo cliente (0x112 → POST `/Guild/upload.asp` → 0x113); download `/_Files/GuildMark/<marca>.png` | **sem upload no cliente**. Download `http://www5.pangya.jp/upload/club/mark/<guildUID>_<nº do emblema>.png` (formato `"%d_%d.png"`); a string da marca não é usada [EXE ✓]. Upload = página do site |
| RSS da guilda | `dwGuildId != 0` dispara HTTP RSS (callback vazio) | `MemberInfo.guild_uid != 0` no 0x44 → 0xFB → 0x1AD → GET `https://www5.pangya.jp/on_client/club/rss.aspx?ID=%s&KID=%s` salvo como `guildlast.xml` [EXE ✓] |
| Chat de guilda | MSN 0x25 → sub 0x113 flag 1 | igual (MSN 0x25 `ES`, ChatManager) [EXE] |
| GuildMatch (modo 6) | 0xBD/0xC0, sGuildRoomInfo | JP 0xBF/0xC2 (+2, mesmo layout) [EXE "igual"]; sala com nº do emblema u16 em vez da marca |

**Consequência para o tradutor**: quase nada é "tradução". Os C->S de guilda do JP (0xA5, 0xA7, 0xA8, 0xA1, 0xA2,
0xFB e os do mensageiro 0x26/0x27/0x16B) não têm par no KR; viram handlers JP que chamam o `GuildService` do Domain. Os S->C
de guilda do KR (0x1B3–0x1C8, 0x1BD, 0x3B, 0x3C, MSN 0x38–0x3A) não têm lugar no JP e devem ser descartados. O que precisa
de conversão de layout são as structs que carregam a guilda (MemberInfo, lista do lobby, sala, perfil 0x15D).

---

## 1. Site da guilda (club)

### 1.1 Abrir [EXE ✓]

1. Botão "club" (`CTaskMain` `FUN_00659240` @0x659240; outro em @0x594bc0) chama `CWebPageConnector::RequestOpen983(url,
   1, lugar)` (@0x8a9fa0) com `url = "https://www5.pangya.jp/on_client/club/login.aspx?ID=%s&KID=%s"` (0xd0b7a0) e
   lugar **5** (no outro, **10**). Ele guarda a URL e manda **C->S 0xFB** (sem dados).
2. **S->C 0x1AD** `u32 código, str chave` (`D4 DS`, caso @0x9d32c0). Com código 1 e o conector existindo, chama
   @0x8aa150 `(doc+0x7a2 = meu login, chave)`: `sprintf(buf, url, login, chave)`.
   - Se a URL guardada é exatamente `https://www5.pangya.jp/on_client/club` (sem login.aspx), **não** abre navegador:
     chama o leitor de RSS (@0x9a7740 com `doc+0x82a` = id da minha guilda). É o caminho do login (§1.3).
   - Senão: `CBrowser::OpenWeb983(buf, 1, lugar)` (@0x870d90) → navegador 800×600 e **C->S 0xA1** `u8 lugar` (`E1`).
3. Código ≠ 1: não faz nada (sem caixa de erro). [EXE ✓]
4. Fechar o navegador (`SendWebPageClose983` @0x868320, tabela por lugar @0x868588): lugar 2 → 0x9E; 3 → só texto
   (item do pacote está no presente); 4 → 0xA0 `u8 1`; **5 → 0xA2** (vazio); 6 → 0xA4 + 0xA2; 11 → 0x3D; 7..10, 12, 14
   → nada. Então o club (lugar 5) fecha com **0xA2**; o lugar 10 (o segundo botão) fecha sem pacote [?: o que é o lugar 10].

### 1.2 O que o servidor faz [REF + proposta]

| JP C->S | layout | o que a referência faz | proposta |
|---|---|---|---|
| **0xFB** | vazio | `CmdGeraWebKey(uid)` → **0x1AD** `u32 1, str chave` (erro: `u32 0, u16 0`) [REF packet0FB] | chave de uso web (curta duração), ligada à conta. Hoje o `JpTranslator.JpOnly` engole o 0xFB (`quiet`) → o club nunca abre |
| **0xA1** | `u8 lugar` | `requestEnterWebLinkState`: guarda o lugar (o jogador fica "no navegador": não pode ser convidado para sala, `place != 0`) [REF] | guardar no handler JP; sem resposta. Hoje cai em "sem par no KR" (log) |
| **0xA2** | vazio | `requestExitedFromWebGuild`: relê o pang (se mudou → **0xC8** `u64 pang, u64 0`) e aplica as mudanças da guilda feitas no site (tabela `GuildUpdateActivityInfo`: aceito / saiu / expulso): atualiza a guilda do jogador, manda a linha do lobby (0x46, opção 3) e avisa o mensageiro (canal interno 0x01 aceito / 0x02 saiu / 0x03 expulso) [REF] | recarregar `Player.Guild` do Domain (`GuildService.TagAsync`), mandar 0x46 sub 3 (lista do lobby) a quem está no canal, `MessengerContext.RefreshListAsync` para quem mudou, e 0xC8 se o pang mudou (o site pode cobrar pang) |

Rotas do site (o exe só conhece as duas primeiras; o resto é nosso, servido pelo `Pangya.Web`):
- `GET /on_client/club/login.aspx?ID=<login>&KID=<chave>` → confere a chave (conta = ID), cria cookie de sessão, mostra
  a guilda do jogador ou a lista/busca de guildas.
- `GET /on_client/club/rss.aspx?ID=&KID=` → RSS (o cliente grava em `guildlast.xml`). Pode responder um RSS vazio válido.
  Falha de rede: o cliente não reclama [?: conferir; no KR o callback é vazio].
- Páginas internas (nossas): criar, checar nome, trocar nome, aviso, apresentação, lista/busca (15 por página), entrar
  (pedido), cancelar pedido, aprovar/recusar, cargo (delegar/promover/rebaixar), mensagem pessoal, sair, expulsar,
  dissolver, histórico, emblema (upload), convidar. Todas chamam o `GuildService` existente (as regras e códigos do KR
  servem: `GuildCode`). Strings em **Shift-JIS** se a página for lida pelo IE do cliente [?: o IE embutido aceita UTF-8 com
  `<meta charset>`; o KR usa só ASCII por causa do Windows coreano].
- Kits: no KR criar/renomear/emblema gastam os itens 0x1A00012F/130/131. No JP o club é criado no site; não há sinal de
  kit [?: conferir se os typeids existem no `pangya_jp.iff`]. Proposta: no site, sem kit (ou cobrar pang, e então 0xC8 no 0xA2).

### 1.3 Login com guilda [EXE ✓]
No 0x44 (dados do jogador, caso @0x9c395e) o cliente copia o MemberInfoEx para `doc+0x7a0` (u16 sala) / `doc+0x7a2`
(MemberInfo 297 + u32 uid). Logo depois (@0x9c3d43): se `guild_uid` (MemberInfo+136 = `doc+0x82a`) ≠ 0, chama
`RequestOpen983("https://www5.pangya.jp/on_client/club", 1, 5)` → **0xFB** → 0x1AD → RSS (§1.1 passo 2). Ou seja: com
guilda, todo login manda 0xFB. Sem resposta 0x1AD nada acontece (é o que temos hoje).

### 1.4 rugburn
As URLs são `https://www5.pangya.jp/...` e `http://www5.pangya.jp/upload/club/mark/`; o `rugburn.json` atual só reescreve
Translation e updatelist. Regras novas (o V5 avisa que `https://` precisa de regra própria):
- `https://www5.pangya.jp/on_client/club` → `http://localhost:30080/on_client/club` (o navegador é o WebBrowser OLE, que o
  rugburn também engancha) [?: rugburn troca https→http?].
- `http://www5.pangya.jp/upload/club/mark/` → `http://localhost:30080/upload/club/mark/`.

---

## 2. Convites e pedidos de entrada (no cliente)

Tudo isto fica no **mensageiro** (`messengerdlg.cpp`): janela `FrGuildManageDlg` com duas listas — "길드가입 승낙 (%d)"
(pedidos de entrada, tipo 2) e "길드초대 대기자 (%d)" (convidados que ainda não responderam, tipo 3) — e `FrAddGuildMemeberDlg`
("digite o login do amigo"; o texto diz que dá para convidar quem está **offline**). Os textos do exe são chaves coreanas
traduzidas pelo `Translation/Read.aspx`. [EXE ✓ strings @0xd3b804..0xd3baa0]

### 2.1 Estado do cliente [EXE ✓]
- `doc+0x82a` = **id da minha guilda** (MemberInfo+136). O 0x119 grava aqui; o 0x128, o MSN 0x26 e a imagem do emblema
  leem daqui.
- `doc+0x82e` = **nº do emblema** (MemberInfo+140).
- `doc+0x832` = MemberInfo+144 (`u8 estado`): bit 3 = sexo (@0x8ff926). **Bits 0–1 = cargo no club**: o cliente só deixa
  gerir (sub 0x128 @0x9c83a3, janela @0x901a95) se `(estado & 3)` for **1 ou 2** (1 mestre, 2 submestre [?: qual é qual]).
  A referência chama esses bits de "channel"/"visible" e não os usa para guilda [REF] — divergência; seguir o exe.
  Hoje `JpStructs.MemberInfo` manda `estado = 0` → o JP nunca mostra as ferramentas de gestão.
- Lista de pedidos/convidados (registros de 55) em `doc+0x10fe8`; convites recebidos (44) em `doc+0x10ff8`.

### 2.2 Registros

**Pendente da guilda (0x37 = 55)** — 0x116 [EXE ✓ @0x8fafc0, @0x8fab90, @0x8fb340]:

| off | tipo | campo |
|---|---|---|
| 0 | u32 | uid do jogador (chave da linha; −1 = linha de cabeçalho) |
| 4 | u32 | id da guilda [?: só é devolvido no 0xA7] |
| 8 | char[22] | login (vai como "alvo" no MSN 0x26 de cancelar convite) |
| 30 | char[22] | nick (é o que a linha desenha) |
| 52 | u8 | ícone (`& 7`, tabela de ícones da janela; inclui `i_male_03`/`i_female_03`) [?: sexo/nível] |
| 53 | u16 | **tipo**: 2 = pediu para entrar (lista "가입 승낙"), 3 = convidado esperando (lista "초대 대기자"); outro = não aparece |

Mesmo tamanho do `GUILD_USER_MSN_LIST` do KR (55), mas outro layout (KR: `u32 uid, u32 guilda, u8 sexo, char[22] login,
char[22] nick, u8 ?, u8 online`). Não reaproveitar a struct do KR.

**Convite recebido (0x2C = 44)** — 0x118 e sub 0x127 [EXE ✓ @0x9c82c4, @0x901b60]:

| off | tipo | campo |
|---|---|---|
| 0 | u32 | B (≠ −1; o 0xA8 manda em 2º) — proposta: id do convite |
| 4 | u32 | A (chave da janela; o 0xA8 manda em 1º) — proposta: id da guilda |
| 8 | char[36] | nome da guilda (mostrado como "%s길드에 초대 받으셨습니다." = "você foi convidado para a guilda %s") |

[?] qual dos dois é a guilda: o exe só mostra que A é a chave da janela e B não pode ser −1. Com A = guilda e B = id do
convite, os dois aparecem como no 0x127 (`u32 A, u32 B, str nome`).

### 2.3 Pacotes do game server

| JP C->S | layout [EXE] | quem manda | resposta (S->C) | layout [EXE ✓] |
|---|---|---|---|---|
| **0xA5** | vazio | `FUN_008fa3f0`: ao abrir a janela de gestão (@0x8fa850) e quando o mensageiro manda sub 0x126 ok ou 0x128 da minha guilda (só gestor) | **0x116** | `u16 n, n × 55` (`D2 DB(0x37)` @0x9cfb3d). Limpa a lista e reconstrói; atualiza a janela |
| **0xA7** | `u32 uid, u32 guilda, u8 aprovar` (`E4 E4 E1`, `FUN_008fb120`; os dois u32 vêm do registro de 55) | botões "길드가입승낙" (1) / "길드가입거절" (0) @0x8fb7b0 | **0x117** | `u32 código` (1 = ok) `[, u32 guilda]` (`D4 D4`). Com 1: manda **MSN 0x16B** `u32` (o 2º u32) [?: guilda]. Senão: caixa com o texto do código (§2.5) |
| **0xA8** | `u32 A, u32 B, u8 aceitar` (`E4 E4 E1`, `CMessengerInfo::OnGuildInviteDlgResult983` @0x901b60; A e B do registro de 44) | caixa "길드초대승낙/거절" | **0x119** | `u32 código, u8 entrou, u32 guilda` (`D4 D1 D4` @0x9cfc76). Código ≠ 1: caixa (§2.5). Com 1 e `entrou` ≠ 0: `doc+0x82a = guilda` e **MSN 0x27** `str meuLogin, u32 guilda`. Sempre (com 1): limpa a lista de convites e manda **MSN 0x16B** `u32 guilda` |
| — | — | o servidor empurra | **0x118** | `u16 n, n × 44` (`D2 DB(0x2c)` @0x9cfc0a). Limpa e reconstrói a lista de convites. Nenhum C->S pede (o enum da referência tem 0xA9 "REQ_INVITATION_LIST", mas o 983 não o envia) → mandar no login (depois do 0x44) e quando chegar convite novo com o jogador no game |

A referência tem os nomes no enum (`CLIENT_GUILD_REQ_WAITING_LIST 0xA5`, `ACCEPT_MEMBER 0xA7`, `ACCEPT_INVITATION 0xA8`,
`SERVER_RESPONSE_GUILD_WAITING_LIST 0x116` … `0x119`) mas **não registra handler** para eles [REF] — não há
comportamento de referência; o que está aqui vem do exe.

### 2.4 Pacotes do mensageiro

C->S (MSN, mesmos ids no servidor do mensageiro) [EXE ✓]:

| id | layout | quem manda | o que fazer |
|---|---|---|---|
| **0x26** | `str meuLogin (doc+0x7a2), str loginAlvo, u32 minhaGuilda (doc+0x82a), u8 op` | `FrAddGuildMemeberDlg` @0x8f9fa0 com op **1** (convidar, login digitado); botão "길드초대거절" da lista de pendentes @0x8fb340 com op **0** (cancelar o convite do registro tipo 3; o login vem do registro+8) | validar (gestor, alvo existe, sem guilda, não convidado, ≤ 10 convites pendentes na guilda, ≤ 5 guildas convidando o alvo), gravar/apagar o convite; responder sub **0x126** ao autor; se op 1 e o alvo estiver online: sub **0x127** ao alvo (e/ou 0x118 pelo game) |
| **0x27** | `str meuLogin, u32 guilda` | depois do 0x119 com `entrou` | "entrei na guilda": avisar os membros online (lista do mensageiro: sub 0x102 recarregada; o JP não tem o 0x39/0x3B de "fulano entrou") |
| **0x16B** | `u32 guilda` | depois do 0x117 ok e do 0x119 ok | "a guilda mudou": mandar sub **0x128** `u32 guilda` aos gestores online (eles relêem com 0xA5) e recarregar a lista (0x102) dos membros online |

**Atenção — colisão com o KR**: no `MessengerHandler` do KR, 0x26 e 0x27 não são tratados hoje (só até 0x25), mas a
SPEC-messenger do KR descreve 0x26 = repasse de convite de sala e 0x27 = aviso de presente, com outros layouts. O
`JpMessengerTranslator.FromClient` passa tudo como veio: os três (0x26, 0x27, 0x16B) têm de ser tratados no lado JP antes
do handler do KR.

S->C: sub-pacotes novos do **0x30** (sub-switch do caso 0x30 @0x9c70b0: `sub − 0x102`, tabela @0x9d7c9c/@0x9d7c38; o
`pacotes_recebe_983.tsv` não os lista) [EXE ✓]:

| sub | layout | efeito |
|---|---|---|
| **0x126** | `u32 código` | 1: "길드초대를 성공적으로 진행하였습니다." (convite feito) e, se a janela de gestão está aberta, manda 0xA5. Outro: caixa (§2.5) |
| **0x127** | `u32 A, u32 B, str nomeGuilda` | cria registro de 44 `{B, A, nome}` na lista de convites e mostra "%s길드에 초대 받으셨습니다." |
| **0x128** | `u32 guilda` | se `guilda == doc+0x82a` e `(estado & 3)` ∈ {1, 2}: manda **0xA5** |

Os S->C de guilda do mensageiro do KR **não existem no JP**: o despachante comum do JP (`FUN_009c20a0`, tabela @0x9d795c
para 0x2F..0x8D) não tem casos 0x3A–0x3E [EXE ✓]. Então KR 0x38/0x39/0x3A (que o `JpMessenger.ServerId` leva para
0x3A/0x3B/0x3C) e o 0x3B/0x3C da referência são ignorados pelo cliente. Os colegas de guilda aparecem na lista de amigos
(sub 0x102, amigo de 145 com o byte "nível na guilda" e o tipo 2 = guilda [REF `requestAcceptGuildMember`]).

### 2.5 Códigos de erro (0x117, 0x119, sub 0x126) [EXE ✓ @0x9b9470, tabela @0x9b94d4]

| código | texto (chave coreana) | sentido |
|---|---|---|
| 1 | — | ok |
| 3 | 해당 유저가 존재하지 않습니다. | jogador não existe |
| 4 | (padrão) 실패하였습니다. | falha |
| 5 | 해당 유저는 이미 다른 길드에 가입되어 있습니다. | já está em outra guilda |
| 6 | 이미 초대된 유저입니다. | já convidado |
| 7 | 길드 관리자만 길드초대를 할수 있습니다. | só gestor convida |
| 8 | 최대 길드초대 인원수는 10명 입니다. | no máximo 10 convites pendentes por guilda |
| 9 | 최대 5개의 길드만 초대할수 있습니다. | um jogador recebe convite de no máximo 5 guildas |
| 10 | 해당 길드는 존재하지 않습니다. | guilda não existe |
| 11 | 길드 가입 요청을 하지 않았습니다. | não pediu para entrar (aprovar quem não está pendente) |
| outro | 실패하였습니다. | falha |

Mapa sugerido `GuildCode` (KR) → JP: `NoAccount`→3, `AlreadyInGuild`→5, `NoPermission`/`NotManager`/`NotMaster`→7,
`NoGuild`→10, `NotWaiting`→11, `Full`→4, resto→4.

---

## 3. Structs que carregam a guilda (KR → JP)

| struct | KR | JP | o que muda para a guilda |
|---|---|---|---|
| MemberInfo (0x44, perfil 0x157, busca 0xA1) | sPangYaUserInfo 269: `sGuild[21]@44, szEmblemName[12]@65, dwGuildId@105, dwEmblemVer@109` | 297: `guilda char[17]@44, emblema char[12]@61, guild_uid@136, **nº do emblema u32@140**, **estado u8@144 (bits 0–1 = cargo)**` | `JpStructs.MemberInfo` (do Domain) manda nº 0 e estado 0: preencher nº do emblema e `estado |= cargo` (1/2 só para gestor; 0 para membro) [?: valor do membro comum — 0 ou 3; o cliente só testa 1/2]. O conversor do perfil (`JpTranslator.Profile` MemberInfo 269→297) copia `dwEmblemVer`→nº: no KR é o "guildPang" copiado [KR SPEC-guilda] → trocar por nº do emblema |
| GUILD_USER_INFO (fim do 0x42) | 281 | **não existe** no 0x44 do JP | — |
| GUILD_INFO (0x1B6, 0x3B, 0x3C, perfil 0x155) | 297: `u32 uid, char[21] nome, i32 pang, i32 pontos, i32 membros, char[12] marca, notice[101], intro[101], i32 cargo, u32 mestre, char[22] nick, SYSTEMTIME` | **GuildInfo 77** (só no perfil 0x15D) [REF `GuildInfo`, EXE `D4 DB(0x4d)`]: `i32 uid, u8 líder, char[20] nome, char[12] marca, u32 nº emblema, u64 ?, u64 pang, 16 bytes ?, u32 pontos` | `JpTranslator.ProfileGuild` já existe; corrigir: `u8 líder` (1 se o alvo é o mestre [?: ou cargo]) e nº do emblema (hoje 0) |
| lista do lobby (KR 0x44 → JP 0x46) | sBriefUserInfo 196 | PlayerLobbyInfo 200: `guild_uid@46`, **`u32 nº do emblema@50`** | `KrToJp.LobbyUser` insere `U32(0)`: pôr o nº. O cliente baixa a imagem quando `guild_uid != 0` (@0xaf9bc7) |
| slot da sala (KR 0x46 → JP 0x48) | sSlotInfo 338 | PlayerRoomInfo 348: guilda 21→20 + u32 nº do emblema depois da marca | `KrToJp.Slot`: pôr o nº |
| sala (sRoomInfo → RoomInfo 221) | sGuildRoomInfo 74: `u32 id[2], char[21] nome[2], char[12] marca[2]` | `i32 id[2], char[12] marca[2] (o JP não usa: "fica 0" [REF]), u16 nº emblema[2], char[20] nome[2]` | `KrToJp.Room` manda os u16 = 0: pôr o nº |
| lista de guildas, membros, histórico | GUILD_LIST 196, GUILD_USER_LIST 81, GUILD_HISTORY 49 | **não existem** (site) | — |
| pendente / convite | — (cargo 9 na lista de membros) | 55 / 44 (§2.2) | novos |
| amigo do mensageiro | sFriend 141 (dwGuildId, emblema 12, bit GuildFriend) | 145 (+4 bytes antes da posição; `u8 nível: 1 = mestre, 0 = membro`; `flag 2 = guilda`) [REF] | `JpMessenger.Friend` insere 4 zeros; candidato natural: nº do emblema [?] |
| troféus de guilda (perfil) | 0x153 `u8, u32, u16 n, n×4` | 0x15B igual | só id (já passa) |
| pontos de guilda no perfil | 0x14F último u32 | 0x157 último u32 | já convertido |

**Nº do emblema** (`guild_mark_img_no` / `index_mark_emblem` [REF]) é novo do JP e é o que o cliente usa no nome do
arquivo. Proposta: o id do último upload aplicado (o KR já gera `EmblemTicket.Id` e a marca `'g' + id em hex`; o nº = esse
id, ou `int.Parse(Mark[1..], hex)`), 0 = sem emblema. Não precisa de coluna nova se a marca continuar `g<hex>`; com
coluna nova (`guilds.mark_no`) fica mais claro.

---

## 4. Emblema

### 4.1 KR hoje [KR]
`GameHandler.Guild`: 0x112 `u32 guilda` → `GuildService.StartEmblemAsync` → **0x1C7** `u32 1, u32 EMBLEM_IDX, str marca`;
o cliente faz POST multipart para `/Guild/upload.asp` (campos `EMBLEM_IDX`, `GUILD_IDX`, `EMBLEM`, arquivo; o
`Pangya.Web` confere ticket/conta/guilda/nome, PNG 32 bits ≤ 22×20, ≤ 8 KB, grava `<dados>/GuildMark/<marca>.png` e
responde `PANGYA_UPDATE_OK`) → 0x113 → `FinishEmblemAsync` (gasta o kit 0x1A000130) → **0x1C8** `u32 1` e **0x3B**
GUILD_INFO aos membros online. Download: `GET /_Files/GuildMark/<marca>.png`.

### 4.2 JP [EXE ✓]
- **Download**: `NetResourceManager::Init` (@0x9abb90) guarda `http://www5.pangya.jp/upload/club/mark/` (tipo `emblem`)
  e o carregador @0x9ab3e0 monta `"%d_%d.png"` com `(guild_uid, nº do emblema)` — chamado por ~60 lugares (lobby @0xaf9bdf
  com `PlayerLobbyInfo+0x2e/+0x32`, mensageiro @0x8ff8a8 com `doc+0x82a/doc+0x82e`, salas, placares…). Cache local em
  `./ntsaved/`. A string da marca (char[12]) **não** entra no nome.
- **Upload**: não há no cliente (só a SelfDesign tem `UploadFiles.aspx`). É página do site.
- Tamanho da imagem do JP: não medido [?]. Manter as regras do KR (`ValidEmblemPng`) até ver o cliente desenhar.

### 4.3 Proposta (Pangya.Web)
- `GET /upload/club/mark/{guildId}_{no}.png` → o mesmo arquivo do KR (`GuildMark/<marca>.png`), achando a guilda pelo id
  e conferindo `no` (ou ignorando `no`: ele só serve para invalidar o cache do cliente).
- Página de upload no site (`/on_client/club/emblem`): form multipart, mesma validação, grava como o KR e aplica na hora
  (sem o passo 0x113), incrementando o nº. Membros online: não há pacote de "emblema mudou" no JP; a imagem nova aparece
  quando o cliente recebe o nº novo (próximo 0x44, lista do lobby 0x46 sub 3, sala). Mandar 0x46 sub 3 para os membros
  que estão num canal.

---

## 5. Tabela para o tradutor

### 5.1 fromClient (JP → KR / handler JP)

| JP C->S | hoje | KR equivalente | ação |
|---|---|---|---|
| 0xFB `vazio` | engolido (`quiet`) | — | handler JP: chave web → **0x1AD** `u32 1, str chave` (erro `u32 0, u16 0`). Não vai ao KR (`return null`) |
| 0xA1 `u8 lugar` | "sem par no KR" | — | handler JP: guardar `lugar` (usado para "não pode ser convidado para sala"); `return null` |
| 0xA2 `vazio` | "sem par no KR" | (KR não tem; o KR atualiza tudo na hora pelo 0x1BD) | handler JP: recarregar guilda e pang (§1.2); `return null` |
| 0xA5 `vazio` | "sem par" | 0x10F `u32 guilda, u32 página` → 0x1C4 (só os de cargo 9) | handler JP: `GuildService` → **0x116** com os pedidos (cargo 9 → tipo 2) e convites pendentes (tipo 3). Sem guilda ou não gestor: `u16 0` |
| 0xA7 `u32 uid, u32 guilda, u8 sim` | "sem par" | 0x10B/0x10C `u32 guilda, u32 uid` → 0x1C0/0x1C1 `u32 1` (+ 0x1BD ao alvo) | handler JP: `AnswerRequestAsync(eu, guilda, uid, sim)` → **0x117** `u32 código, u32 guilda`; se aprovado e o alvo online: atualizar `Player.Guild` dele, 0x46 sub 3, mensageiro (`GuildJoinedAsync` recarrega a lista) |
| 0xA8 `u32 A, u32 B, u8 sim` | "sem par" | — (o KR só tem pedido do jogador, não convite) | handler JP: aceitar/recusar convite → **0x119** `u32 código, u8 entrou, u32 guilda`; ao entrar: `Player.Guild`, 0x46 sub 3, 0x118 de novo (lista limpa) |
| 0xFE..0x116 | `ClientId` = −1 | 0xFE..0x113 (guilda do KR) | nada: o JP não os usa para guilda (0xFE = UCC). **Nunca** encaminhar ids JP para o handler de guilda do KR |
| MSN 0x26/0x27/0x16B | passam ao `MessengerHandler` do KR | (KR 0x26/0x27 = outros pacotes) | tratar no `JpMessenger` (§2.4) |

### 5.2 toClient (KR → JP)

| KR S->C | `ServerId` hoje | ação |
|---|---|---|
| 0x1B3–0x1C8 (respostas de guilda), 0x1BD | −1 (descartado com log) | ok; nunca acontecem no JP (o JP não manda os pedidos). Se `PushGuildStateAsync` do KR for chamado para um jogador JP (ex.: um KR aprova um JP), o 0x1BD é descartado → o handler JP deve ouvir a mudança de outro jeito (§6 passo 5) |
| 0x3B / 0x3C (GUILD_INFO: emblema/nome) | +2 → 0x3D/0x3E, que o JP **não trata** | `toClient[0x3B] = toClient[0x3C] = null` (não mandar lixo) e, no lugar, 0x46 sub 3 com o nº/nome novos |
| 0x155 perfil (GUILD_INFO 297) | +8 → 0x15D com `ProfileGuild` | já converte 297→77; preencher `u8 líder`, nº do emblema |
| 0x14F perfil (MemberInfo) | +8 → 0x157 (269→297+4) | nº do emblema e bits de cargo no `estado` |
| 0x44 lobby (sBriefUserInfo) | +2 → 0x46 (`LobbyUser` 196→200) | nº do emblema em vez de `U32(0)` |
| 0x46/0x48 slot, sala | `Slot`, `Room` | nº do emblema |
| 0x153 troféus de guilda | +8 → 0x15B | passa |
| 0xBD / 0xC0 GuildMatch | +2 → 0xBF / 0xC2 | passa (rx "igual": `D1 DB(0x9)`, `D4 D2 D2 D1 D1`) |
| MSN 0x38/0x39/0x3A | `JpMessenger.ServerId` +2 → 0x3A/0x3B/0x3C (não tratados) | descartar no `JpMessengerTranslator` (o 0x102 já leva os colegas) |
| MSN 0x2E/0x113 chat de guilda | +2 → 0x30/0x113 | passa |

Ids novos só do JP (montados no handler JP, não pelo tradutor): game **0x116, 0x117, 0x118, 0x119, 0x1AD**; MSN 0x30 subs
**0x126, 0x127, 0x128**. Nenhum colide com ids que o tradutor produz (o +4 do KR 0x116–0x11B cai em 0x11A–0x11F; o +2
termina no 0x109).

---

## 6. Plano de implementação

Ordem pensada para ter algo visível cedo (o club abre) e deixar os convites por último.

1. **Chave web e entrada/saída** (`JpTranslator.JpOnly` ou um `JpSystems.Guild.cs`):
   - `ISessionStore`: `IssueWebKeyAsync(accountId)` / `ConsumeWebKeyAsync(login, key)` (ou reaproveitar o que o login web
     já usa); 0xFB → 0x1AD. Tirar 0xFB do `quiet`.
   - 0xA1: guardar o lugar (campo no tradutor/handler); 0xA2: recarregar `Player.Guild` (`GuildService.TagAsync`), pang
     do banco (0xC8 se mudou), 0x46 sub 3 no canal, `ctx.Messenger.RefreshListAsync`.
2. **Dados do emblema e cargo nas structs**: `GuildTag` ganha `MarkNo` (derivado da marca `g<hex>` ou coluna nova);
   `JpStructs.MemberInfo` (nº + `estado` bits 0–1), `KrToJp.LobbyUser`/`Slot`/`Room` (o tradutor não tem o Domain do outro
   jogador: ou o KR passa a escrever o nº no campo `dwEmblemVer`/espaço livre do sBriefUserInfo — hoje o KR põe o pang ali —
   ou o tradutor procura `ctx.World.Find(uid)?.Player.Guild`), `ProfileGuild`.
3. **Site** (`Pangya.Web`): `/on_client/club/login.aspx`, `rss.aspx`, páginas de gestão chamando o `GuildService` (sem LINQ
   em `src/`), `GET /upload/club/mark/{id}_{no}.png`, upload pelo site. Regras do `rugburn.json` (§1.4).
4. **Pedidos de entrada no cliente**: 0xA5 → 0x116 (cargo 9 → tipo 2), 0xA7 → 0x117 (`AnswerRequestAsync`), MSN 0x16B →
   sub 0x128 aos gestores. O pedido em si é feito no site (`RequestJoinAsync`); ao gravar, avisar os gestores online
   (sub 0x128 pelo mensageiro JP).
5. **Convites** (novo no Domain): tabela `guild_invites (id, guild_id, account_id, inviter_id, created_at)`; métodos
   `InviteAsync(eu, guilda, loginAlvo)` (códigos §2.5: 10 por guilda, 5 por alvo, alvo sem guilda, só gestor),
   `CancelInviteAsync`, `AnswerInviteAsync(alvo, convite, sim)` (entra como membro, cargo 3; histórico "Approved"),
   `InvitesOfAsync(conta)` (para o 0x118) e `PendingOfAsync(guilda)` (tipo 3 do 0x116). MSN 0x26 → sub 0x126/0x127;
   0x118 no login (depois do 0x44) e quando o convite chega com o jogador no game; 0xA8 → 0x119; MSN 0x27/0x16B.
   Quem não está no JP (cliente KR) não vê convites — o KR não tem a tela; manter o convite só para contas JP ou deixá-lo
   pendente.
6. **Limpeza**: `toClient[0x3B]/[0x3C] = null` no JP; descartar MSN 0x38–0x3A no `JpMessengerTranslator`; MSN
   0x26/0x27/0x16B tratados antes do handler do KR.
7. **Mudanças vindas do KR**: quando um jogador KR (mestre) aprova/expulsa um JP, o KR chama `PushGuildStateAsync`, que
   manda 0x1BD (descartado no JP). Fazer o tradutor JP converter o 0x1BD em "recarregar guilda + 0x46 sub 3" em vez de
   descartar (o corpo traz o GUILD_USER_INFO novo: id, nome, marca, cargo).

### 6.1 Pontos [?] para testar no cliente
- Cargo nos bits 0–1 do `estado`: 1 = mestre e 2 = submestre (ou o contrário); valor do membro comum.
- Ordem A/B do registro de convite (44) e do sub 0x127.
- O 2º u32 do 0x117 (guilda ou uid aprovado) e o u32 do MSN 0x16B.
- Byte de ícone (+52) do registro de 55.
- rugburn reescrevendo `https://` para o servidor local; IE embutido com UTF-8.
- Tamanho máximo da imagem do emblema no JP.
- Se a lista do lobby (0x46 sub 3) atualiza também o `doc+0x7a2` do próprio jogador (senão a guilda nova só aparece no
  próximo login; a referência só manda o 0x46).

---

## 7. Testes (cliente JP falso `tests/Pangya.Tests/Jp983Client.cs`)

Usar `Jp983Client.Build(id, função, valores…)` para montar os C->S com os campos que o exe escreve e
`Jp983Client.Reads/Blocks` para conferir os S->C contra o exe. Os subs 0x126–0x128 não estão no `pacotes_recebe_983.tsv`
(sub-switch): conferir os layouts direto no teste.

1. **Chave web**: `Build(0xFB, "FUN_008a9fa0")` → `ExpectAsync(0x1AD)`: `U32() == 1`, `Str()` não vazia;
   `Reads(0x1AD) == "D4 DS"`. A chave abre `/on_client/club/login.aspx?ID=<login>&KID=<chave>` no `WebServer` de teste
   (200) e não abre com chave errada/de outra conta.
2. **0xA1/0xA2**: `Build(0xA1, "FUN_00870d90", (byte)5)` não gera resposta nem log "sem par"; mudar o pang no banco,
   `new PacketWriter(0xA2)` → `0xC8` com o pang novo; entrar numa guilda pelo `GuildService` (como o site faria),
   0xA2 → 0x46 sub 3 com `guild_uid` e nº do emblema na linha (offsets 46/50 do PlayerLobbyInfo 200).
3. **Login com guilda**: conta mestre de uma guilda com emblema; no 0x44 o MemberInfo tem `guild_uid@136`, nº `@140` ≠ 0,
   `estado & 3 == 1`. (Estende os testes de `Jp983GameTests`.)
4. **Pedidos**: A mestre, B pede pelo `GuildService.RequestJoinAsync`; A manda `new PacketWriter(0xA5)` → 0x116:
   `Blocks(0x116) == [0x37]`, n = 1, registro `uid = B`, login@8, nick@30, tipo@53 = 2.
   `Build(0xA7, "FUN_008fb120", uidB, gid, (byte)1)` → 0x117 `U32() == 1`, B vira membro (`MembershipAsync`), e B (se
   conectado JP) recebe 0x46 sub 3. Recusar (`(byte)0`) → 1 e B sem guilda. Aprovar quem não pediu → código 11.
5. **Convite pelo mensageiro** (dois clientes JP no game e no messenger-jp): A manda
   `Build(0x26, "FUN_008f9fa0", loginA, loginC, (uint)gid, (byte)1)` → A recebe 0x30 sub 0x126 `u32 1`; C recebe sub
   0x127 `u32, u32, str nome`; no game, C recebe 0x118 (`Blocks(0x118) == [0x2c]`). C manda
   `Build(0xA8, "FUN_00901b60", a, b, (byte)1)` → 0x119 `u32 1, u8 1, u32 gid`; C é membro. Recusar → `u8 0`, sem guilda.
   Cancelar (`op 0`, `FUN_008fb340`) some com o tipo 3 do 0x116.
6. **Erros do convite**: login inexistente → 3; alvo já em guilda → 5; repetido → 6; membro comum convidando → 7; 11º
   convite → 8; 6ª guilda convidando o mesmo alvo → 9.
7. **Emblema**: depois do upload pelo site, `GET /upload/club/mark/{gid}_{no}.png` devolve o PNG; o nº no MemberInfo, na
   lista do lobby, no slot e no perfil 0x15D (`Blocks(0x15D) == [0x4d]`, nº @37 do GuildInfo) é o mesmo.
8. **Tradutor**: um KR 0x3B/0x3C/0x1BD não chega ao cliente JP (nenhum pacote 0x3D/0x3E); MSN 0x39/0x3A do KR não chegam
   como 0x3B/0x3C; `JpTranslator.ClientId(0xA5/0xA7/0xA8) == -1` continua (tratados antes, pelo `fromClient`).
9. **Perfil**: 0x2F de um jogador com guilda → 0x15D com `u8 líder = 1` para o mestre e pang/pontos certos.
