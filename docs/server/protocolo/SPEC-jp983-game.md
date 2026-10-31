# JP R7.983 — game server

Cliente JP **R7.983.00**. Fontes: análise do exe (`docs/server/jp983-protocolo`: **[EXE]** = leitura/escrita conferida no binário),
referência JP da comunidade (`Server/JP/GameServer`, AGPL-3.0: `GameService.requestLogin`, `cLoginTask.sendCompleteData`,
`packet_func_sv.cs`, `pangya_game_st.cs`; **[GB-JP]**), **[T]** confirmado com o cliente real, **[I]** inferido.

## Arquitetura (desde 12/10: tradutor)
- Porta, mundo (canais/salas/lobby) e registro **próprios**: `Pangya.Game/JpGameServer` (nome `game-jp` no `Run`, seção
  `GameJp`; porta de desenvolvimento 30202), registrado como `game-jp` (o login do JP lista só esses; o do KR, só `game`).
- A lógica é a do **game server do KR** (`KR645.Game.GameHandler`: lobby, salas, partida, loja, My Room). O
  `JpGameHandler` faz o hello do JP e liga a compressão; o `JpTranslator` (um por conexão, `Connection.Translator`)
  traduz ids e structs nos dois sentidos; `KrToJp` converte as structs por faixas de bytes.
- **Ids** (relatório V4, conferidos com os pares que o cliente já aceitou): S->C KR→JP +0 até 0x09, **+2 de 0x2d a 0x107**,
  +4 de 0x116 a 0x11b, +8 de 0x123 a 0x15a, −5 de 0x172 a 0x1a2. C->S JP→KR: +0 até 0x82, −3 de 0x8b a 0x9d, −8 de 0xad a
  0xca, +2 de 0xd3 a 0xef. Fora disso: não vai (log "sem id no JP" / "sem par no KR"). Atenção: `ids_*.tsv` desta cópia
  comparam com o rebang já renumerado (quase tudo +0) e não servem para KR→JP.
- Pacote do KR cujo par no JP lê struct que mudou e que ainda não tem conversão não vai (`NotConvertedYet`, log
  "sem conversão"): é melhor que o cliente não receber do que ler fora do pacote e cair.

## Structs que mudaram (KR 645 → JP 983)
| struct | KR | JP | conversão |
|---|---|---|---|
| personagem | 444 | 513 | campos reordenados; + cut-in, maestria, cards (`KrToJp.Character` / `CharacterToKr` no 0x20) |
| item (inventário) | 168 | 196 | iguais até 23 e de 82 a 115; nome 41→40+venda; + uid de quem copiou; + oficina do club set |
| mascote | 63 | 62 | u16 horas + compra → i16 tipo |
| club set (equipado) | 18 | 28 | i32 id, u32 typeid, i16[5] slots, i16[5] upgrades |
| equipamento | 108 | 116 | + 2 pôsteres (refeito do Domain) |
| canal | 77 | 85 | + propriedade 16, nível mín/máx |
| jogador no lobby | 196 | 200 | + u32 nº do emblema no offset 50 |
| sala | 178 | 221 | nome 64 (sem senha), guildas no formato JP, + artefato, natural, grand prix |
| dados do jogador | 2962 | 0x31C6 | refeito do Domain (`JpStructs.PlayerBlock`) |
| jogador na sala | 338 | 348 (+ personagem 513) | guilda 21→20, + u32 nº do emblema, + f32 média e 3 bytes no fim |
| club set (sala) | 18 | 28 | + i16[5] slots antes dos upgrades |
| estatísticas | 235 | 265 | refeitas do Domain (0x45); **falta** 0x158 (perfil) |
| tacada | 46 (+4) | 50 | os 46 do KR + f32 zona de impacto (o KR já repassa os 4 do fim) |
| resultado da tacada | 37 | 38 | + u8 penalidade do Grand Prix |

## My Room e lobby
- Equipar (C->S 0x20, igual ao KR; tipo 0 traz o personagem JP de 513) -> **0x6B** (KR 0x69): `u8 4 ok / 5 erro, u8 tipo,
  dados`. O JP lê no tipo 1 o id do caddie e no tipo 3 a bola + o id do club set, que o KR não manda: o tradutor
  completa (sem isso os tacos e o mascote só apareciam depois no My Room).
- "Start Game": C->S **0x81** -> 0xF5 (KR 0xF3), **0x47** salas `u8 n, u8 sub, u16 0xFFFF, n × 221`, **0x46** jogadores
  `u8 sub, u8 n, n × 200`.

## Sala e partida (stroke)
- Criar sala C->S **0x08**: o JP põe u32 natural/short game depois do modo (+5 bytes no modo 19) e u32 artefato no fim:
  saem antes do KR.
- **0x49** (KR 0x47): `i16 0 + sala 221` ou `u8 erro`. **0x4A** (KR 0x48): u16 sala, modo, mapa, buracos, ordem, **u32
  natural**, máx, galeria, dormindo, u32 tacada, u32 jogo, u32 troféu, **u8 pública (+ str senha)**, str título.
- **0x48** (KR 0x46): u8 sub, u16 **0xFFFF** (a referência sempre), [u8 n], jogador 348 (+ personagem 513), **u8 0 no fim**
  também nos subs 1 e 3 (referência).
- **0x76** (KR 0x74): u8 **modo da sala** (o KR manda 0), u8 n, por jogador o bloco 0x31C6 (do Domain, com o índice da
  sala e o guid), SYSTEMTIME, u8 m, m × card 65.
- **0x52** (KR 0x50): mapa, modo, ordem, buracos, **u32 troféu** (o KR põe a semente aqui), u32 tacada, u32 jogo, 18 ×
  (u32 id, **u8 pin 0..2**, **u8 mapa**, u8 nº), **u32 semente**, 18 × u8 caixas/moedas (0 por enquanto: o formato das
  caixas do JP — u32 tipo, u32 id, u32 flag, u32 mapa, u8 nº, u8 índice, u16, f32 × 3… — ainda não está convertido, então
  o Wiz City fica sem itens). Pin = semente do buraco % 3 (a referência sorteia; aqui é o mesmo para toda a sala).
- Tacada C->S **0x12**: u16 opção [+9 do power shot] + 50 -> KR `u16 0 + 50`; eco **0x55** u32 + 50 (o do bot ganha 4
  zeros). Resultado C->S **0x1B**: 38 bytes XOR chave da sala (sem a chave no fim; o KR já usa a da sala); eco **0x64**
  decifrado com 38.
- Iguais ao KR (só o id): 0x53 começo do buraco, 0x5B vento, 0x63 vez (u32), 0x65 próximo buraco (vazio), 0x66 placar.

## Compressão e buffer do cliente (crash do 1º teste, 12/10)
- O cliente guarda o que recebe num buffer de **0x3FFFC** bytes (networksystem, 0x00A4D07F): se enche antes de processar,
  lança exceção C++ e fecha (`exception.log`: CxxThrowException a partir de 0x00A4D0C4, no FD_READ) [EXE][T]. A conta de
  teste com ~5000 itens mandava ~950 KB de 0x73 de uma vez.
- Por isso o game JP **comprime** como o oficial: flag **0** no lugar do 1, tamanho = corpo original (3 dígitos base 255) e
  o corpo em **LZO1X** (`Core/Crypto/Lzo1x`, conferido contra a liblzo2; `CCompressBuffer::Decompress` usa
  `lzo1x_decompress`). Com isso o login de 5000 itens fica abaixo de 128 KB (teste). O KR continua sem compressão.
- Itens que o cliente JP não tem no `pangya_jp.iff` (`Data.JpIffPath`, do ProjectG966.pak) não são mandados (a conta
  `royal` tinha 158 do KR: mascotes 0x40000009…, club sets, bolas, peças); equipados desconhecidos vão como 0.

## Entrada até o canal
1. S->C cru **0x3F**: `u8 1, u8 1, u8 chave (0..15), str IP do cliente` [GB-JP]. Mesma cifra do KR.
2. C->S **0x02**: `str id, u32 uid, u32 uid Ntreev, u16 comando, str chave do login, str versão, u32 versão de pacote
   (2017110200), str MAC, str chave do game server` [EXE: `ES E4 E4 E2 ES ES E4 ES ES`, gameunit.cpp]. O nosso login manda
   a mesma chave nos dois (0x10 e 0x03); o game confere a do game server, o uid e o id.
   Erro: **0x44** `u8 1, u8 0` e fecha.
3. S->C, nesta ordem [GB-JP]:
   - **0x44** `u8 0xD3, u8 0` (carregando).
   - **0x44** `u8 0, str versão ("JP.R7.983.00", ≤ 12), bloco do jogador (0x31C6), SYSTEMTIME, u16 estado, u16[3] papel
     shop, u32 pontos de evento, u64 bloqueios do servidor, i32 contagem, u32 propriedade do servidor` [EXE: `DS DB(0x31c6)
     DB(0x10) D1 D1 D2 D2 D2 D4 D8 D4 D4`].
     Bloco: MemberInfoEx 299 (`u16 sala, char[22] id, char[22] nick, char[17] guilda, char[12] emblema, char[35] comentário,
     u32 escola, u32 capacidade, u32 galeria, i32 oid, u32[3] ranking, u32 guilda, u32 nº emblema, u8 estado, u16 login,
     u16[3] papel, u32 evento, u64 bloqueio, u32 canalização, char[128] id de exibição`) + u32 uid + UserInfo 265 + troféus 78
     + UserEquip 116 + 12 × 22 MapStatistics de 43 (normal, natural, grand prix, 9 temporadas) + equipados 628 (zerados,
     como a referência).
   - **0x70** personagens `u16 total, u16 n, n × 513` · **0x71** caddies `n × 25` · **0x73** itens `n × 196` (páginas de 50)
     · **0xE1** mascotes `u8 n, n × 62` · **0x72** equipamento 116 [EXE: os DB(n) batem].
   - **0x4D** canais `u8 n, n × 85` (`char[64] nome, i16 máx, i16 atuais, u8 id, u32 tipo, u32 propriedade (16), u32 nível
     mín, u32 nível máx`).
   - **0x96** cookie `u64`; **0x11F** tutorial `u16 3, u32[3]`; **0x131** Treasure Hunter `u8 1, u8 22, 22 × (u8, u32)`;
     cards **0x135**, **0x138** `u32 0, u16 0`, **0x136**, **0x137** `u16 0` (vazios por enquanto).
   - A referência manda ainda 0x1A9, 0x101, 0x102, 0xF1, 0x144, 0x13F, 0x181, 0x169, 0xB4, 0x158, 0x25D, 0x210…
     (conquistas, temporadas, correio): ainda não mandados; ver no teste se o cliente precisa de algum.
4. C->S **0x04** `u8 canal` -> **0x4E** `u8 1` ok / 2 cheio / 3 não existe (`+ i32 código` só no erro 6).

## Estado
- Entrada até o canal: implementada, testada com o cliente falso (`tests/Pangya.Tests/Jp983GameTests.cs`, que monta os
  pacotes pela tabela de escritas do exe e confere os blocos lidos). Falta o teste no cliente real.
- Lobby, My Room, sala, início da partida e a tacada pelo tradutor (cliente falso). Falta: confirmar no cliente real a
  partida até o fim; caixas/moedas do Wiz City; perfil (0x158/0x15E); loja (0xAA, 0x68).
