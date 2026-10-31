using Pangya.Core.Net;
using Pangya.Core.Text;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Últimos jogadores (referência packet09C/pacote10E): C->S 0x9C -> 0x10E com 5 registros de 0x34 (u32 sexo, char[22]
/// nick, char[22] id, u32 uid), o mais recente primeiro; os que faltam vão zerados. O sexo não é guardado: vai 0.
/// </summary>
public sealed partial class JpSystems
{
    public const ushort CRecentPlayers = 0x9C;
    const ushort SRecentPlayers = 0x10E;
    const int RecentCount = 5, NameField = 22;

    async Task RecentPlayersAsync()
    {
        var list = kr.Context.RecentPlayers is { } store ? await store.LastAsync(kr.Player.AccountId, RecentCount) : [];
        var w = new PacketWriter(SRecentPlayers, 8 + RecentCount * 0x34);
        Span<byte> field = stackalloc byte[NameField];
        for (int i = 0; i < RecentCount; i++)
        {
            if (i >= list.Count) { w.Zeros(0x34); continue; }
            var p = list[i];
            w.U32(0);
            field.Clear(); Cp949.Write(field, p.Nickname); w.Bytes(field);
            field.Clear(); Cp949.Write(field, p.Login); w.Bytes(field);
            w.U32((uint)p.AccountId);
        }
        conn.SendTranslated(w);
    }
}
