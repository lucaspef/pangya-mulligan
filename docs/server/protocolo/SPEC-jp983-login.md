# JP R7.983 — login server

Cliente JP **R7.983.00** (build de 09/11/2017), redirecionado pelo rugburn para o login do servidor C#
(`157.7.173.48:10803 -> localhost:30101`). Referência: servidor JP da comunidade (`Server/JP/LoginServer`, R7.983):
`LoginServerTcp/LoginServer.cs` (`onAcceptCompleted`, `requestLogin`), `PacketFunc/packet_func_ls.cs` (`pacote001`,
`succes_login`), `Models/pangya_login_st.cs` (`LoginData`). Marcas: **[GB-JP]** referência, **[T]** confirmado no teste
com o cliente real, **[I]** inferido.

## Conexão — igual ao KR 645
- S->C cru **0x0000**: `u32 chave (0..15), u32 uid do servidor` [GB-JP]; mesma cifra do KR. Antes desta camada, o
  cliente JP já decifrava a nossa resposta de senha errada do KR ("Incorrect username or password") [T].
- Por isso KR e JP dividem a porta 30101: o `LoginRouter` (src/Pangya.Login) manda o hello e escolhe a versão pelo
  primeiro 0x0001 (depois das duas strings o KR tem sempre 18 bytes: `u32 2, u8, u32 MemberNo, u64, u8`).

## C->S
| id | layout | uso |
|---|---|---|
| 0x01 | `str id, str senha, u8 n, n × 8 bytes (desconhecidos), str MAC` | login com a senha digitada (MD5 em hexa se ≥ 32 caracteres) [GB-JP] |
| 0x03 | `u32 uid do game server` | escolheu um servidor da lista [GB-JP] |
| 0x04 | — | derrubar a sessão anterior (já logado) [GB-JP] |
| 0x06 / 0x07 / 0x08 | nickname / conferir nickname / primeiro personagem | ainda não tratados no JP |
| 0x0B | chave de login | reconectar depois de cair do game server (ainda não tratado) |

## S->C (ordem do sucesso, `succes_login`)
1. **0x10** `str` chave de login.
2. **0x01** sub 0: `str id, u32 uid, u32 identidade, u8 1, i32 0, u8 1, i32 5, SYSTEMTIME, str chave, u64 0, str nick`
   (a data, a chave e o u64 o JP lê e ignora) [GB-JP].
3. **0x02** `u8 n + n × ServerInfo (92 B)`: `char[40] nome, u32 id, i32 máx, i32 atuais, char[18] endereço, i32 porta,
   u32 propriedade, i32 asas, u32 eventFlags, u32 eventValue, u16 ícone` (o mesmo
   tamanho do `sGameServerInfo` do KR).
4. **0x09** lista de mensageiros (mesmo formato; hoje vazia: o mensageiro do JP não existe).
5. **0x06** macros de chat: 9 × 64 bytes.

Sem o portão da segunda senha (0xDE) do KR. Escolha do servidor: **0x03** `i32 0, str chave do game server`.

Erros do 0x01 (u8 sub): 2 ID inexistente, 6 ID ou senha errados (+ u32 0), 7 bloqueado por tempo (+ i32 horas),
0x0C bloqueado para sempre, 0x0F manutenção, 0xD8 criar nickname (+ i32 -1, i16 0), 0xD9 criar personagem (+ i16 0),
4 já logado (o cliente manda 0x04), 0xE2/16 + i32 código.

## Estado
- Login, lista de servidores e escolha do servidor: implementados (`Pangya.Protocol.JP983/Login`).
- Falta: criação de nickname/personagem pelo JP, reconexão (0x0B), mensageiro e **o game server do JP** (o cliente
  conecta no game server da lista, que hoje só fala KR 645).
