using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Protocol.JP983.Login;
using Pangya.Protocol.KR645.Login;

namespace Pangya.Login;

/// <summary>
/// Uma porta de login para mais de uma versão do cliente. O hello (0x0000: chave, uid do servidor) e a cifra são iguais
/// no KR 645 e no JP 983; a versão sai do primeiro pedido de login (0x0001), e dali em diante a conexão fica com o
/// handler daquela versão:
/// - KR: str id, str AuthKey do site, u32 2, u8, u32 MemberNo, u64, u8 — sobram exatamente 18 bytes depois das strings;
/// - JP: str id, str senha, u8 n, n × 8 bytes, str MAC.
/// O 0x000B (re-login com a chave, ao voltar do jogo para a lista de servidores) só o JP manda: vai direto para o JP.
/// </summary>
public sealed class LoginRouter(Connection conn, LoginContext kr, JpLoginContext? jp) : IConnectionHandler
{
    const ushort CLogin = 0x01, CRelogin = 0x0B;
    const int KrTailBytes = 4 + 1 + 4 + 8 + 1;

    readonly LoginHandler krHandler = new(conn, kr);
    IConnectionHandler? chosen;

    /// <summary>O hello é o mesmo nas duas versões: o do KR serve.</summary>
    public ValueTask OnConnectedAsync() => krHandler.OnConnectedAsync();

    public ValueTask OnDisconnectedAsync() => chosen?.OnDisconnectedAsync() ?? ValueTask.CompletedTask;

    public ValueTask OnPacketAsync(PacketReader p)
    {
        if (chosen == null)
        {
            if (p.Id != CLogin && !(p.Id == CRelogin && jp != null)) return krHandler.OnPacketAsync(p);
            chosen = jp != null && (p.Id == CRelogin || IsJp(p)) ? new JpLoginHandler(conn, jp) : krHandler;
            if (chosen != krHandler)
            {
                Log.Info($"{conn} cliente JP 983");
                conn.UseTextEncoding(Core.Text.Cp949.Japanese);        // strings em Shift-JIS
            }
        }
        return chosen.OnPacketAsync(p);
    }

    /// <summary>Olha o pedido de login sem consumir: depois das duas strings, o KR tem sempre 18 bytes.</summary>
    public static bool IsJp(PacketReader p)
    {
        int start = p.Position;
        try
        {
            p.Str(64);
            p.Str(64);
            return p.Remaining != KrTailBytes;
        }
        catch (PacketException) { return false; }
        finally { p.Position = start; }
    }
}
