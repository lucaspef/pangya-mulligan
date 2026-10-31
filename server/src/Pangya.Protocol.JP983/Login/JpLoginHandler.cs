using Pangya.Core.Config;
using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Core.Text;
using Pangya.Domain.Accounts;
using Pangya.Domain.Servers;

namespace Pangya.Protocol.JP983.Login;

/// <summary>Serviços do login do JP 983 (um por processo).</summary>
public sealed class JpLoginContext(AccountService accounts, LoginService login, IServerRegistry registry, LoginConfig config)
{
    public AccountService Accounts { get; } = accounts;
    public LoginService Login { get; } = login;
    public IServerRegistry Registry { get; } = registry;
    public LoginConfig Config { get; } = config;
}

/// <summary>
/// Login server do cliente JP R7.983 (docs/server/protocolo/SPEC-jp983-login.md; referência: servidor JP da comunidade,
/// LoginServer/requestLogin). Mesma conexão do KR (hello 0x0000 com a chave, mesma cifra); muda o pedido de login (a
/// senha vem direto do cliente, sem o site) e as respostas. Nunca fecha a conexão em erro de login.
/// </summary>
public sealed class JpLoginHandler(Connection conn, JpLoginContext ctx) : IConnectionHandler
{
    // ids C->S
    const ushort CLogin = 0x01, CSelectServer = 0x03, CKickPrevious = 0x04, CCreateNick = 0x06, CCheckNick = 0x07,
        CCreateCharacter = 0x08, CRelogin = 0x0B;
    // ids S->C
    const ushort SHello = 0x00, SLoginResult = 0x01, SServerList = 0x02, SGameKey = 0x03, SMacros = 0x06, SMessengerList = 0x09,
        SNickResult = 0x0E, SFirstLogin = 0x0F, SLoginKey = 0x10, SCharacterResult = 0x11;
    /// <summary>0x0E: código 12 = erro com número do ErrorCodeInfo.iff (referência: falha ao salvar o personagem).</summary>
    const int NickErrorInfo = 12;
    // sub-códigos do 0x01 (referência: pacote001)
    const byte Ok = 0x00, BadId = 0x02, WrongPassword = 0x06, BlockedForever = 0x0C, Maintenance = 0x0F, CreateCharacter = 0xD9,
        CreateNick = 0xD8;
    /// <summary>9 macros de chat × 64 bytes (chat_macro_user).</summary>
    const int MacroBytes = 9 * 64;

    Account? account;
    /// <summary>Conta autenticada que ainda cria nickname/personagem (primeiro acesso).</summary>
    Account? pending;
    string gameKey = "", typedLogin = "", checkedNick = "";

    public ValueTask OnConnectedAsync()
    {
        conn.ParseKey = Random.Shared.Next(16);
        conn.SendRaw(new PacketWriter(SHello).U32((uint)conn.ParseKey).U32(ctx.Config.ServerUid));
        return ValueTask.CompletedTask;
    }

    public ValueTask OnDisconnectedAsync() => ValueTask.CompletedTask;

    public async ValueTask OnPacketAsync(PacketReader p)
    {
        switch (p.Id)
        {
            case CLogin: await LoginAsync(p); break;
            case CSelectServer: SelectServer(p.U32()); break;
            case CKickPrevious: break;
            case CRelogin: await ReloginAsync(p.Str(32), p.U32(), p.Str(64)); break;
            case CCheckNick: await CheckNickAsync(p.Str(32)); break;
            case CCreateNick: await CreateNickAsync(p.Str(32)); break;
            case CCreateCharacter: await CreateCharacterAsync(p.I32(), p.U8(), p.U8()); break;
            default: Log.Info($"{conn} JP: pacote de login não tratado 0x{p.Id:X4} ({p.Remaining} bytes)"); break;
        }
    }

    /// <summary>
    /// Pedido de login do JP (LoginData): str id, str senha, u8 n, n × 2 × u32 (desconhecidos), str MAC. A senha chega
    /// como digitada (ou MD5 em hexa, se tiver 32 caracteres; aqui não aceito, as contas guardam só o hash forte).
    /// </summary>
    public static (string Id, string Password, string Mac) ReadLogin(PacketReader p)
    {
        string id = p.Str(32), pwd = p.Str(64);
        int n = p.U8();
        p.Skip(Math.Min(n * 8, p.Remaining));
        string mac = p.Remaining >= 2 ? p.Str(64) : "";
        return (id, pwd, mac);
    }

    async Task LoginAsync(PacketReader p)
    {
        if (account != null || pending != null) return;
        var (id, pwd, mac) = ReadLogin(p);
        var (status, acc) = await ctx.Accounts.AuthenticateAsync(id, pwd, conn.Remote.Address.ToString());
        Log.Info($"{conn} JP login id={id} mac={mac}: {status}");
        switch (status)
        {
            case AuthStatus.Ok:
                var r = await ctx.Login.ContinueAsync(acc!);
                await SendResultAsync(id, r);
                break;
            case AuthStatus.Blocked:
                conn.Send(new PacketWriter(SLoginResult).U8(BlockedForever));
                break;
            case AuthStatus.UnknownLogin or AuthStatus.InvalidInput:
                conn.Send(new PacketWriter(SLoginResult).U8(BadId));
                break;
            default:                                                    // senha errada, tentativas demais
                conn.Send(new PacketWriter(SLoginResult).U8(WrongPassword).U32(0));
                break;
        }
    }

    async Task SendResultAsync(string typedId, LoginResult r)
    {
        switch (r.Outcome)
        {
            case LoginOutcome.Ok:
                var acc = r.Account!;
                account = acc;
                gameKey = r.GameKey!;
                conn.IdleTimeoutSeconds = 0;                            // a conexão de login fica aberta e calada no jogo
                // ordem da referência (succes_login): 0x10 chave, 0x01 ok, 0x02 game servers, 0x09 mensageiros, 0x06 macros
                conn.Send(new PacketWriter(SLoginKey).Str(gameKey));
                conn.Send(LoginOk(typedId, acc, gameKey));
                conn.Send(ServerList(SServerList, await ctx.Registry.ListAsync(Jp983.GameServerType)));
                conn.Send(ServerList(SMessengerList, await ctx.Registry.ListAsync(Messenger.JpMessenger.ServerType)));
                conn.Send(new PacketWriter(SMacros).Zeros(MacroBytes));
                Log.Info($"{conn} JP login ok: {acc.Login} uid={acc.Id}");
                break;
            case LoginOutcome.Blocked:
                conn.Send(new PacketWriter(SLoginResult).U8(BlockedForever));
                break;
            case LoginOutcome.NeedsNickname:                            // primeiro acesso: 0x0F + 0x01 D8 (janela do nickname)
                pending = r.Account;
                typedLogin = typedId;
                conn.Send(FirstLogin(typedId));
                conn.Send(new PacketWriter(SLoginResult).U8(CreateNick).I32(-1).U16(0));
                Log.Info($"{conn} JP: conta sem nickname, abre a criação");
                break;
            case LoginOutcome.NeedsCharacter:                           // 0x0F + 0x01 D9 (escolha do personagem)
                pending = r.Account;
                typedLogin = typedId;
                conn.Send(FirstLogin(typedId));
                conn.Send(new PacketWriter(SLoginResult).U8(CreateCharacter).U16(0));
                Log.Info($"{conn} JP: conta sem personagem, abre a escolha");
                break;
            default:
                conn.Send(new PacketWriter(SLoginResult).U8(WrongPassword).U32(0));
                break;
        }
    }

    /// <summary>0x0F (aviso de primeiro acesso, referência pacote00F): u8 1, str id, u32 0, u32 5, str data, str chave.</summary>
    static PacketWriter FirstLogin(string typedId) => new PacketWriter(SFirstLogin).U8(1).Str(typedId).U32(0).U32(5).Str("").Str("");

    /// <summary>C->S 0x07 str nick -> 0x0E u32 código (0 ok + str nick; 2 em uso; 3 inválido; 1 erro).</summary>
    async Task CheckNickAsync(string nick)
    {
        if (pending == null) return;
        var st = await ctx.Login.CheckNicknameAsync(pending, nick);
        if (st == Domain.Players.NicknameStatus.Ok) checkedNick = nick;
        var w = new PacketWriter(SNickResult).U32((uint)st);
        conn.Send(st == Domain.Players.NicknameStatus.Ok ? w.Str(nick) : w);
    }

    /// <summary>
    /// C->S 0x06 str nick: grava e segue (sem personagem: 0x0F + 0x01 D9; com: login completo). Erro: 0x0E com o código.
    /// </summary>
    async Task CreateNickAsync(string nick)
    {
        if (pending == null) return;
        var st = string.Equals(nick, checkedNick, StringComparison.OrdinalIgnoreCase)
            ? await ctx.Login.CreateNicknameAsync(pending, nick)
            : Domain.Players.NicknameStatus.Error;                     // difere do conferido no 0x07
        Log.Info($"{conn} JP: criar nickname {nick}: {st}");
        if (st != Domain.Players.NicknameStatus.Ok) { conn.Send(new PacketWriter(SNickResult).U32((uint)st)); return; }
        var acc = pending;
        pending = null;
        await SendResultAsync(typedLogin, await ctx.Login.ContinueAsync(acc));
    }

    /// <summary>
    /// C->S 0x08 u32 personagem, u8 cabelo, u8 roupa (referência packet008): cria o personagem padrão -> 0x11 u16 0 e o
    /// login completo; falha: 0x11 e 0x0E 12 (erro).
    /// </summary>
    async Task CreateCharacterAsync(int characterTypeId, byte hair, byte shirt)
    {
        if (pending == null) return;
        bool ok = await ctx.Login.CreateCharacterAsync(pending, characterTypeId, hair, shirt);
        Log.Info($"{conn} JP: criar personagem 0x{characterTypeId:X8} cabelo {hair} roupa {shirt}: {(ok ? "ok" : "recusado")}");
        conn.Send(new PacketWriter(SCharacterResult).U16(0));
        if (!ok) { conn.Send(new PacketWriter(SNickResult).I32(NickErrorInfo).U32(500051)); return; }
        var acc = pending;
        pending = null;
        await SendResultAsync(typedLogin, await ctx.Login.ContinueAsync(acc));
    }

    /// <summary>
    /// C->S 0x0B str id, u32 uid, str chave: volta do jogo para a lista de servidores (o cliente reconecta ao login com a
    /// chave do 0x10). Como a referência (requestTryReLogin/succes_login opção 1): 0x10 chave, 0x02 game servers,
    /// 0x09 mensageiros, sem o 0x01 e sem macros. Chave inválida: 0x01 id inválido.
    /// </summary>
    async Task ReloginAsync(string id, uint uid, string key)
    {
        if (account != null) return;
        var r = await ctx.Login.ReloginAsync(id, key);
        if (r.Outcome != LoginOutcome.Ok || r.Account!.Id != uid)
        {
            Log.Info($"{conn} JP re-login id={id}: {r.Outcome}");
            conn.Send(new PacketWriter(SLoginResult).U8(r.Outcome == LoginOutcome.Blocked ? BlockedForever : BadId));
            return;
        }
        account = r.Account;
        gameKey = key;
        conn.IdleTimeoutSeconds = 0;
        conn.Send(new PacketWriter(SLoginKey).Str(gameKey));
        conn.Send(ServerList(SServerList, await ctx.Registry.ListAsync(Jp983.GameServerType)));
        conn.Send(ServerList(SMessengerList, await ctx.Registry.ListAsync(Messenger.JpMessenger.ServerType)));
        Log.Info($"{conn} JP re-login ok: {account.Login} uid={account.Id}");
    }

    /// <summary>
    /// 0x01 sub 0: str id, u32 uid, u32 identidade, u8 nível(1), i32 0, u8 1, i32 5, str "", str chave, u64 0, str nick.
    /// Leitor do 983 (005a2620): id, u32, u32, u8, u8, u32, u32, str, str (chave), u32, u32, str (displayID); os bytes
    /// "i32 0, u8 1, i32 5" caem em u8 + u32 + u32. Antes ia um SYSTEMTIME no lugar da 1ª str: o ano virava tamanho e o
    /// cliente lia ~2 KB além do pacote (chave e displayID saíam do heap; frente D, D757).
    /// </summary>
    static PacketWriter LoginOk(string typedId, Account acc, string key) =>
        new PacketWriter(SLoginResult).U8(Ok).Str(typedId).U32((uint)acc.Id).U32((uint)acc.IdentityFlags)
            .U8(1).I32(0).U8(1).I32(5)
            .Str("").Str(key).U64(0).Str(acc.Nickname!);

    /// <summary>
    /// C->S 0x03 u32 uid do game server escolhido -> 0x03 i32 0, str chave do game server (o cliente conecta com ela).
    /// </summary>
    void SelectServer(uint serverUid)
    {
        if (account == null) return;
        Log.Info($"{conn} JP: escolheu o game server {serverUid}");
        conn.Send(new PacketWriter(SGameKey).I32(0).Str(gameKey));
    }

    /// <summary>
    /// u8 n + n × ServerInfo (92 bytes, igual ao do KR): char[40] nome, u32 id, i32 máx, i32 atuais, char[18] endereço,
    /// i32 porta, u32 propriedade, i32 asas de anjo, u32 eventFlags, u32 eventValue, u16 ícone.
    /// </summary>
    public static PacketWriter ServerList(ushort id, IReadOnlyList<ServerInfo> servers)
    {
        int n = Math.Min(servers.Count, 255);
        var w = new PacketWriter(id).U8((byte)n);
        Span<byte> name = stackalloc byte[40], addr = stackalloc byte[18];
        for (int i = 0; i < n; i++)
        {
            var s = servers[i];
            Cp949.Write(name, s.Name);
            Cp949.Write(addr, s.Address);
            w.Bytes(name).U32((uint)s.Id).I32(s.MaxUsers).I32(s.CurUsers).Bytes(addr).I32(s.Port).U32((uint)s.Flags).I32(0)
                .U32((uint)s.EventFlags).U32((uint)s.EventValue).U16((ushort)s.IconIndex);
        }
        return w;
    }
}
