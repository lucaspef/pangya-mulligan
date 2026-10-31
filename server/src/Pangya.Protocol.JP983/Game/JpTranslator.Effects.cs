using Pangya.Core.Net;
using Pangya.Domain.Rooms;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Efeitos de item na partida (docs/server/protocolo/SPEC-jp983-efeitos.md §3–§4, §10): o efeito é calculado no cliente; o
/// servidor só repassa (stroke: a sala toda; torneio/mass: só quem ativou). Ids só do JP: vão por SendTranslated.
/// Os com código (0x237, 0x24C, 0x265, 0x266, 0x280) levam u32 0 antes (o exe lê o código primeiro).
/// </summary>
public sealed partial class JpTranslator
{
    void AddEffects()
    {
        fromClient[0x138] = r => { uint t = r.U32(); Effect(false, oid => new PacketWriter(0x203).U32(oid).U32(t)); return null; };     // asa
        fromClient[0x15C] = r => { Effect(false, oid => new PacketWriter(0x236).U32(oid)); return null; };                              // patas
        fromClient[0x15D] = r =>                                                                                                         // anel
        {
            uint t = r.U32(); r.U32(); byte e = r.U8();
            if (!Has(t)) return Fail(0x237);
            Effect(false, oid => new PacketWriter(0x237).U32(0).U32(oid).U32(t).U8(e));
            return null;
        };
        fromClient[0x171] = r =>                                                                                                         // brinco (vento)
        {
            uint t = r.U32(); byte angle = r.U8(); float x = r.F32();
            if (!Has(t)) return Fail(0x24C);
            Effect(false, oid => new PacketWriter(0x24C).U32(0).U32(t).U32(oid).U8(angle).F32(x));
            return null;
        };
        fromClient[0x180] = r =>                                                                                                         // luva
        {
            uint t = r.U32();
            if (!Has(t)) return Fail(0x265);
            Effect(false, oid => new PacketWriter(0x265).U32(0).U32(t).U32(oid));
            return null;
        };
        fromClient[0x181] = r =>                                                                                                         // anel no chão: só ele
        {
            var b = r.Bytes(Math.Min(16, r.Remaining)).ToArray();
            Effect(true, oid => new PacketWriter(0x266).U32(0).Bytes(b).Zeros(16 - b.Length).U32(oid));
            return null;
        };
        fromClient[0x196] = r => { Effect(false, oid => new PacketWriter(0x27E).U32(oid)); return null; };                              // patas arco-íris
        fromClient[0x197] = r => { r.Skip(r.Remaining); Effect(false, oid => new PacketWriter(0x27F).U32(oid)); return null; };         // power gauge
        fromClient[0x198] = r =>                                                                                                         // miracle sign
        {
            uint t = r.U32();
            if (!Has(t)) return Fail(0x280);
            Effect(false, oid => new PacketWriter(0x280).U32(0).U32(t).U32(oid));
            return null;
        };
        fromClient[0x199] = r => { Effect(false, oid => new PacketWriter(0x281).U32(oid)); return null; };                              // anéis com patas
        // marcador no campo: 0x1F8 i32 oid, f32 x, y, z a toda a sala (só no stroke, como a referência)
        fromClient[0x12E] = r =>
        {
            float x = r.F32(), y = r.F32(), z = r.F32();
            Effect(false, oid => new PacketWriter(0x1F8).U32(oid).F32(x).F32(y).F32(z), strokeOnly: true);
            return null;
        };
        fromClient[0xCB] = r => null;                                                    // fim do approach (o KR também ignora o 0xC3)
    }

    bool Has(uint typeId) => typeId != 0 && kr.Player.FindType((int)typeId) != null;

    byte[]? Fail(ushort id)
    {
        Reply(id, w => w.U32(1));
        return null;
    }

    /// <summary>Repassa o efeito: na partida em andamento; stroke = a sala toda, mass = só quem ativou (onlyMe força só ele).</summary>
    void Effect(bool onlyMe, Func<uint, PacketWriter> make, bool strokeOnly = false)
    {
        lock (kr.Context.World.Rooms.Sync)
        {
            if (kr.CurrentRoom is not { Game: { Over: false } g } room || room.Find(kr) is not { } me) return;
            bool mass = g is MassGame;
            if (strokeOnly && mass) return;
            foreach (var rp in room.Players)
                if (rp.Session is GameHandler h && (h == kr || !onlyMe && !mass)) h.Connection.SendTranslated(make(me.Guid));
        }
    }
}
