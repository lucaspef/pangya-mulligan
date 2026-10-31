using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Rooms;

namespace Pangya.Protocol.KR645.Game;

/// <summary>
/// Intrusão: entrar num torneio já começado (docs/server/protocolo/SPEC-intrusao-tempo.md §1). 0x9A sub 0 (duplo clique) ->
/// 0x111 {3: sala, decorrido, limite} ou {6: código}; sub 1 ("sim") -> entra na sala e na partida: {4,0} sala, {4,1}
/// jogadores, {4,2} taxa, {4,3} rivais, {4,4} início; quem está no campo recebe {7}; no 1º carregamento o intruso recebe
/// {10} de cada rival, 0x6A dos que terminaram e {9}. Sub 2 (placar mandado pelo cliente) é ignorado.
/// </summary>
public sealed partial class GameHandler
{
    const ushort CIntrusion = 0x9A, SIntrusion = 0x111;
    const byte IntrConfirm = 3, IntrEnter = 4, IntrDenied = 6, IntrJoined = 7;

    bool HandleIntrusion(PacketReader p)
    {
        if (p.Id != CIntrusion) return false;
        byte sub = p.Remaining >= 1 ? p.U8() : (byte)2;
        if (sub > 1 || p.Remaining < 2) { p.Skip(p.Remaining); return true; }
        int index = p.U16();
        p.Skip(p.Remaining);
        lock (Rooms.Sync)
        {
            var r = Rooms.Get(index);
            byte code = r?.Game is TourneyGame t ? t.IntrusionRefusal(Player.AccountId) : (byte)5;
            if (room != null) code = 5;                                    // já está numa sala
            if (code != 0)
            {
                conn.Send(new PacketWriter(SIntrusion).U8(IntrDenied).U8(code));
                return true;
            }
            var game = (TourneyGame)r!.Game!;
            var s = r.Settings;
            if (sub == 0)
            {
                conn.Send(new PacketWriter(SIntrusion, 200).U8(IntrConfirm).U8(0).U16((ushort)r.Index).U32(game.ElapsedMs).U32(s.GameTimeMs)
                    .Struct(RoomPackets.RoomInfo(r)));
                return true;
            }
            Enter(r, game);
        }
        return true;
    }

    /// <summary>Entra na sala e na partida (sob o lock).</summary>
    void Enter(Room r, TourneyGame game)
    {
        var rp = RoomManager.Join(r, new RoomPlayer { Guid = (uint)Player.AccountId, Player = Player, Session = this });
        room = r;
        slotsResent = false;
        Rooms.Lobby.Remove(this);
        var me = game.AddLate(rp);
        conn.Send(new PacketWriter(SIntrusion, 200).U8(IntrEnter).U8(0).Struct(RoomPackets.RoomInfo(r)));
        var w = new PacketWriter(SIntrusion, 8 + r.Players.Count * 0x152).U8(IntrEnter).U8(1).U8((byte)r.Players.Count);
        foreach (var x in r.Players) w.Struct(RoomPackets.SlotInfo(x));
        conn.Send(w);
        conn.Send(new PacketWriter(SIntrusion).U8(IntrEnter).U8(2).U32(100));
        conn.Send(new PacketWriter(SIntrusion).U8(IntrEnter).U8(3).U8(0).U8(0).U8(0).Struct(PlayerStructs.SystemTime(DateTime.Now)));
        conn.Send(StartPacket(r));
        foreach (var x in r.Players)                                         // quem está no campo: alguém entrou
            if (x.Session is GameHandler h && h != this)
            {
                var j = new PacketWriter(SIntrusion, 300 + r.Players.Count * 0x152).U8(IntrJoined).U8(0).Str(Player.Nickname)
                    .Struct(RoomPackets.RoomInfo(r)).U8((byte)r.Players.Count).U32((uint)r.Players.Count);
                foreach (var y in r.Players) j.Struct(RoomPackets.SlotInfo(y));
                h.Connection.Send(j);
            }
        LobbyUser(LobbyUserUpdate);
        LobbyRoom(3, r);
        Log.Info($"{conn} entrou na partida em andamento da sala {r.Index} ({game.ElapsedMs / 1000} s)");
        _ = me;
    }

    /// <summary>{4,4}: o 0x50 sem o mapa por buraco, com o troféu (Match.iff) no lugar da semente.</summary>
    static PacketWriter StartPacket(Room r)
    {
        using var init = RoomPackets.GameInit(r);
        var body = init.Body[2..].ToArray();
        BitConverter.TryWriteBytes(body.AsSpan(4), (uint)RoomPackets.MatchTid(r));
        return new PacketWriter(SIntrusion, body.Length + 4).U8(IntrEnter).U8(4).Bytes(body);
    }
}
