using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;
using Pangya.Domain.Shop;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Grand Prix, fase 2 (docs/server/protocolo/SPEC-jp983-grandprix.md): entrar (0x179 -> sala de torneio do GP ou 0x253 código),
/// sair (0x17A -> 0x254), contagem regressiva e início no horário (0x40 tipo 11), e o fim (posição contando os bots,
/// 0x258 pódio, 0x25C troféu, 0x216 prêmios, 0x25A GP concluído). O começo da partida (0x253, bots 0x256) é do tradutor.
/// </summary>
public sealed partial class JpSystems : IGrandPrixSink
{
    public const ushort CGpEnter = 0x179, CGpLeave = 0x17A;
    const ushort SGpEnterResult = 0x253, SGpLeft = 0x254, SGpPodium = 0x258, SGpCleared = 0x25A, SGpTrophy = 0x25C, SNotice = 0x40, SGpHoleTime = 0x259;
    const byte NoticeCountdown = 11;
    const int PangPouch = 0x1A000010;

    /// <summary>GPs concluídos (Link -> melhor posição), carregados no login: o 0x250 e as travas (Lock_YN) usam.</summary>
    public Dictionary<int, int> GrandPrixCompleted { get; private set; } = [];

    IGrandPrixData? GpData => ctx.Data as IGrandPrixData;

    async Task LoadGrandPrixAsync()
    {
        if (gpStore != null) GrandPrixCompleted = await gpStore.CompletedAsync(kr.Player.AccountId);
        kr.GrandPrix = this;
    }

    /// <summary>
    /// 0x250 (entrou no lobby do GP): u32 0, u32 nº de tipos de evento ativos (0), u32 n, n × (u32 Link, u32 melhor
    /// posição), f32 média de placar do jogador.
    /// </summary>
    public void GrandPrixLobby(PacketWriter w)
    {
        w.U32(0).U32(0).U32((uint)GrandPrixCompleted.Count);
        foreach (var (link, pos) in GrandPrixCompleted) w.U32((uint)link).U32((uint)pos);
        var s = kr.Player.Stats;
        w.F32(s.Hole > 0 ? 72f + (float)s.TotalScore / s.Hole * 18 : 0f);
    }

    /// <summary>0x179 u32 id do GP -> a sala (0x49/0x4A/0x48 pelo KR) ou 0x253 u32 código.</summary>
    async Task GrandPrixEnterAsync(int id)
    {
        if (GpData is not { } data || !data.GrandPrix.TryGetValue(id, out var d))
        {
            conn.SendTranslated(new PacketWriter(SGpEnterResult).U32(GrandPrix.NotFound));
            return;
        }
        uint code = GrandPrix.CanEnter(kr.Player, d, GrandPrixCompleted, DateTime.Now);
        if (code != GrandPrix.Ok)
        {
            conn.SendTranslated(new PacketWriter(SGpEnterResult).U32(code));
            Log.Info($"{conn} JP: Grand Prix 0x{id:X} recusado (0x{code:X})");
            return;
        }
        if (kr.EnterGrandPrix(d) is not { } entered)
        {
            conn.SendTranslated(new PacketWriter(SGpEnterResult).U32(GrandPrix.Full));
            return;
        }
        // GM não espera o horário: sala criada por ele começa em 5 s
        if (entered.Created && entered.Room.GrandPrix?.StartsAt is { } at) _ = CountdownAsync(entered.Room, (kr.Player.IdentityFlags & JpTranslator.GmFlag) != 0 ? DateTime.Now.AddSeconds(5) : at);
        await Task.CompletedTask;
    }

    /// <summary>
    /// Contagem até o horário do GP (0x40 u8 11, str "", str "", u32 segundos: a cada 30 s, depois 10 s, no fim 1 s) e o
    /// início pelo servidor. Sem ninguém na sala, ela já foi fechada e nada acontece.
    /// </summary>
    async Task CountdownAsync(Room room, DateTime at)
    {
        try
        {
            while (true)
            {
                int left = (int)Math.Ceiling((at - DateTime.Now).TotalSeconds);
                List<GameHandler> handlers = [];
                lock (ctx.World.Rooms.Sync)
                {
                    if (room.State != RoomState.Waiting || room.Players.Count == 0 || ctx.World.Rooms.Get(room.Index) != room) return;
                    foreach (var rp in room.Players) if (rp.Session is GameHandler h) handlers.Add(h);
                }
                if (left <= 0)
                {
                    if (handlers.Count > 0) handlers[0].StartAsServer();
                    return;
                }
                foreach (var h in handlers)
                    h.Connection.SendTranslated(new PacketWriter(SNotice).U8(NoticeCountdown).Str("").Str("").U32((uint)left));
                int step = left > 40 ? 30 : left > 10 ? 10 : 1;
                await Task.Delay(TimeSpan.FromSeconds(Math.Min(step, left)));
            }
        }
        catch (Exception e) { Log.Error($"GP: falha na contagem da sala {room.Index}", e); }
    }

    /// <summary>0x17A u8, i16, chave -> sai da sala e 0x254 u32 0, i16 -1 (volta ao lobby do GP).</summary>
    void GrandPrixLeave()
    {
        kr.LeaveCurrentRoom();
        conn.SendTranslated(new PacketWriter(SGpLeft).U32(0).I16(-1));
    }

    /// <summary>0x259 u32 0: o tempo do buraco acabou (o servidor já deu o buraco como desistência).</summary>
    public void HoleTimeOver() => conn.SendTranslated(new PacketWriter(SGpHoleTime).U32(0));

    /// <summary>0x199 (vazio): fechou o último buraco.</summary>
    public void LastHole() => conn.SendTranslated(new PacketWriter(0x199));

    /// <summary>Ticket do GP gasto no início (0x216 com a quantidade nova).</summary>
    public Task ConsumeTicketAsync(GrandPrixDef d) =>
        Domain.Players.Premium.IsActive(kr.Player, DateTime.UtcNow) ? Task.CompletedTask : SpendAsync(d.TicketTypeId, d.TicketQty);

    /// <summary>Gasta qty do item (ticket do GP, do Chip-in) e manda o 0x216 com a quantidade nova.</summary>
    public async Task SpendAsync(int typeId, int qty)
    {
        var p = kr.Player;
        if (typeId == 0 || qty <= 0 || p.FindType(typeId) is not { } t) return;
        int before = t.Quantity, after = Math.Max(0, before - qty);
        var ch = new Domain.Players.PlayerChanges();
        var edited = t.Clone();
        edited.Quantity = after;
        if (after == 0) ch.Removed.Add(t.Id); else ch.Updated.Add(edited);
        await ctx.Players.Store.ApplyAsync(p.AccountId, ch);
        if (after == 0) p.Items.Remove(t.Id); else p.Items[t.Id] = edited;
        conn.SendTranslated(ItemsChanged([new ItemChange(t.TypeId, t.Id, before, after)]));
    }

    /// <summary>Fim do GP para este jogador: posição (com os bots), pódio, troféu e prêmios, e a lista de concluídos.</summary>
    public async Task FinishedAsync(GrandPrixRun run, GrandPrixEntry me, IReadOnlyList<GrandPrixEntry> players)
    {
        try
        {
            var d = run.Def;
            int PositionOf(GrandPrixEntry e)
            {
                var others = new List<(int, long)>();
                foreach (var b in run.Bots) others.Add((b.Score, b.Pang));
                foreach (var o in players) if (o.Guid != e.Guid) others.Add((o.Score, o.Pang));
                return GrandPrix.Position(e.Score, e.Pang, others);
            }
            int pos = PositionOf(me);

            // pódio: humanos entre os 3 primeiros
            var podium = new List<(GrandPrixEntry E, int Pos)>();
            foreach (var e in players) if (PositionOf(e) is var ep && ep <= 3) podium.Add((e, ep));
            podium.Sort((a, b) => a.Pos.CompareTo(b.Pos));
            var w = new PacketWriter(SGpPodium, 8 + podium.Count * 222).U32(0).U8((byte)podium.Count);
            foreach (var (e, ep) in podium) PodiumEntry(w, e, ep);
            conn.SendTranslated(w);

            // prêmios: participação + posição (top 3) e o troféu
            var rewards = new List<(int TypeId, int Qty)>();
            foreach (var (tid, n, _) in d.Rewards) rewards.Add((tid, n));
            var rank = GpData?.GrandPrixRankRewards(d.Link) ?? [];
            if (pos <= rank.Count && pos <= 3)
            {
                var rr = rank[pos - 1];
                foreach (var (tid, n, _) in rr.Items) rewards.Add((tid, n));
                if (rr.Trophy != 0 && !QuitRate.NoTrophy(kr.Player))
                {
                    conn.SendTranslated(new PacketWriter(SGpTrophy).U32(0).U32((uint)rr.Trophy));
                    if (kr.Context.SpecialTrophies is { } trophies)
                    {
                        await trophies.AddAsync(kr.Player.AccountId, rr.Trophy, 1);
                        TrophyWon(rr.Trophy, 1);
                    }
                }
            }
            await GiveAsync(rewards);

            if (gpStore != null && (!GrandPrixCompleted.TryGetValue(d.Link, out var best) || pos < best))
            {
                await gpStore.SaveAsync(kr.Player.AccountId, d.Link, pos);
                GrandPrixCompleted[d.Link] = pos;
                conn.SendTranslated(new PacketWriter(SGpCleared).U32(0).U32((uint)d.Link).U32((uint)pos));
            }
            Log.Info($"{conn} JP: Grand Prix 0x{d.Id:X} terminado em {pos}º (placar {me.Score}, {run.Bots.Count} bots)");
        }
        catch (Exception e) { Log.Error($"{conn} falha no fim do Grand Prix", e); }
    }

    /// <summary>Prêmios direto no inventário (saco de pang vira pang) e um 0x216 com o que mudou.</summary>
    async Task GiveAsync(List<(int TypeId, int Qty)> rewards)
    {
        var p = kr.Player;
        var changes = new List<ItemChange>();
        long pang = 0;
        foreach (var (tid, n) in rewards)
        {
            if (tid == 0 || n <= 0) continue;
            if (tid == PangPouch) { pang += n; continue; }
            int before = p.FindType(tid)?.Quantity ?? 0;
            var (code, granted) = await ctx.Shop.GiveAsync(p, tid, n);
            if (code != ShopCode.Ok) continue;
            foreach (var g in granted)
                changes.Add(new ItemChange(g.TypeId, g.Id, g.TypeId == tid ? before : 0, p.Find(g.Id)?.Quantity ?? Math.Max(1, n)));
        }
        if (pang > 0)
        {
            long total = p.Pang + pang;
            await ctx.Players.Store.ApplyAsync(p.AccountId, new Domain.Players.PlayerChanges { Pang = total });
            p.Pang = total;
            conn.SendTranslated(new PacketWriter(SPang).U64((ulong)p.Pang).U64(0));
        }
        if (changes.Count > 0) conn.SendTranslated(ItemsChanged(changes));
    }

    /// <summary>Entrada do pódio (222): u32 uid, u32 posição, u8 cabelo, u8 camisa, u32[24] peças, u32[5] anéis, u32[24] ids.</summary>
    static void PodiumEntry(PacketWriter w, GrandPrixEntry e, int pos)
    {
        w.U32((uint)e.Player.AccountId).U32((uint)pos);
        if (e.Player.Character is not { } ch) { w.Zeros(222 - 8); return; }
        using var t = new PacketWriter(0, 460).Struct(PlayerStructs.Character(ch));
        var k = t.Body[2..];
        w.U8(k[104]).U8((byte)(k[105] & 0x0F)).Bytes(k.Slice(8, 96)).Bytes(k.Slice(112, 20)).Bytes(k.Slice(348, 96));
    }
}
