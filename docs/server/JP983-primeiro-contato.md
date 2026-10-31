# JP 983: primeiro contato com o servidor C# (09/10/2026)

Nota da sessão do rebang/KR 839 para a sessão do servidor. Objetivo do usuário: **fazer login com o cliente JP 983**
no nosso servidor C#, como primeiro passo da camada `Protocol.JP983` ("S9 no futuro" no PLANO.md).

## Cliente de teste
- Pasta: `<pasta do jogo>` (cópia de trabalho; o original está em
  `D:\BACKUPS\Backup_DGO_2026-09-28\C\pangya-clients\JP_Building\extracted\Client-out\Client`).
- Exe: dump do usuário (`ProjectG.exe`, 20 MB) do build **JP.R7.983.00 de 09/11/2017**. O exe protegido de origem é
  byte a byte igual ao usado pelo servidor privado PangYa Relic.
- Redirecionamento via rugburn (`ijl15.dll` + `rugburn.json`, versão do usuário com patches próprios):
  login `157.7.173.48:10803 → localhost:30101`; `Translation/Read.aspx` e `patch/updatelist` → `localhost:30090`.
- `ABRIR_JP983_CS.bat` sobe o web do JP (`web-jp-30090.ps1`, servindo a pasta `www` do JP_Building) e abre o cliente.
- Registro: usa `HKLM\SOFTWARE\WOW6432Node\Ntreev\Pangya_JP`, criado pela instalação do Relic (não alterar).

## O que acontece hoje
- O cliente abre, carrega a tabela de textos (interface em inglês) e mostra a tela de login (tema de Halloween).
- Ao logar (conta `royal`): conecta em `127.0.0.1:30101` (confirmado no `gglog.txt`) e mostra
  **"Incorrect username or password"**. Ou seja, o servidor respondeu, mas a camada KR645 não entende o login do JP.
- O cliente também tenta `127.0.0.1:8080` algumas vezes e desiste (serviço local desconhecido). Antes, uma regra
  `8080 → 10903` no rugburn fazia isso virar um loop infinito; a regra foi retirada.

## Login do JP 983 (pela referência da comunidade)
Fonte: `<ref>/Server/JP/LoginServer` no WSL (luismk, JP R7.983), `Models/pangya_login_st.cs` (`LoginData`)
e `LoginServerTcp/LoginServer.cs` (`requestLogin`, linha ~558).

Pacote do cliente **0x0001**:
```
PStr  id            (ushort tamanho + bytes)
PStr  password      (ushort tamanho + bytes)
byte  opt_count
uint32[opt_count*2] desconhecidos
PStr  mac_address
```
- Erro de senha na referência: `pacote001(session, 15)`.
- Conferir também na referência: a cifra/chave da sessão (pacote inicial do servidor), as respostas de sucesso
  (lista de servidores, mensagem de logado) e os IDs de pacote do JP (`PacketIDClient`/`PacketIDServer`).

## Sugestão de caminho
1. Registrar os bytes crus que o cliente JP manda no login (log em hex no servidor C#) e comparar com `LoginData`.
2. Criar `Pangya.Protocol.JP983` com o login (0x0001 + respostas), reaproveitando o núcleo de domínio.
3. Testar com `<pasta do jogo>\ABRIR_JP983_CS.bat` até a lista de servidores; depois o game server.
