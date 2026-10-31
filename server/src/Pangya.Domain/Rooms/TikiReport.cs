using System.Text.Json.Nodes;
using Pangya.Domain.Players;

namespace Pangya.Domain.Rooms;

/// <summary>
/// Relatório do Tiki (JP 983, docs/server/protocolo/SPEC-jp983-tiki-report.md): quem terminou o torneio e não quer esperar os
/// outros usa o pergaminho (0x1A000041), sai na hora levando o pang e recebe o relatório (0x1A000042, 7 dias para abrir)
/// no My Room; o EXP entra quando ele abre. O resultado (linhas de todos) fica nos atributos do item; se ele ainda estiver
/// online quando a partida acabar, as linhas viram as finais.
/// </summary>
public static class TikiReport
{
    public const int ScrollTypeId = 0x1A000041, ReportTypeId = 0x1A000042, MaxReports = 10, MinLevel = 6;
    /// <summary>Bits de dwIdentity que o cliente recusa (GM e afins; FrGameRoomDlg::CheckTikiReport).</summary>
    public const int BlockedIdentity = 2 | 4 | 8;
    public static readonly TimeSpan Lifetime = TimeSpan.FromDays(7);
    /// <summary>Respostas do 0x11A sem registros: 0 falha ao abrir, -1 data inválida (vencido), -2 dados inválidos.</summary>
    public const int Fail = 0, BadDate = -1, BadData = -2, Ok = 1;
    /// <summary>Common[1]*0x8000 + Common[2] = o parâmetro do C->S 0xAB.</summary>
    public const int ParamHigh = 0x8000;

    /// <summary>Uma linha do resultado (sCaddieReportData). Left = saiu no meio (o cliente mostra o estado 3).</summary>
    public sealed record Row(uint Uid, string Nick, int Level, int Score, long Pang, long Bonus, int Mascot, bool Premium,
        int GuildId, string GuildMark, int Trophy, int Medals, bool Left);

    /// <summary>O relatório: data da partida, EXP a creditar ao abrir, troféu da sala, modo e as linhas.</summary>
    public sealed record Data(DateTime Date, int Exp, int MatchTid, int Mode, List<Row> Rows);

    /// <summary>Por que não vale usar o pergaminho agora (null = vale); as regras do cliente que o servidor confere.</summary>
    public static string? Refusal(Player p, Room room, MassGame game, MassPlayer? me)
    {
        var s = room.Settings;
        if (game is not TourneyGame || game.Over || s.Mode is not (GameMode.Tournament or GameMode.Team30s) || s.Hidden
            || s.Zodiac != ZodiacKind.None) return "modo sem relatório";
        if (me is not { Finished: true, Left: false, IsBot: false }) return "não terminou";
        if (p.Level < MinLevel) return $"nível {p.Level}";
        if ((p.IdentityFlags & BlockedIdentity) != 0) return "GM";
        if (p.FindType(ScrollTypeId) is not { Quantity: > 0 }) return "sem pergaminho";
        if (Count(p) >= MaxReports) return $"já tem {MaxReports} relatórios";
        return null;
    }

    public static int Count(Player p)
    {
        int n = 0;
        foreach (var it in p.Items.Values) if (it.TypeId == ReportTypeId) n++;
        return n;
    }

    /// <summary>
    /// Relatórios que saem no login: vencidos (o cliente só limpa a lista com o -1) e sem dados (entregues fora do
    /// pergaminho, como o give-all: sem prazo o card mostra a data zerada e, com o parâmetro 0, o cliente nem manda o 0xAB).
    /// </summary>
    public static List<Item> Stale(Player p, DateTime nowUtc)
    {
        var list = new List<Item>();
        foreach (var it in p.Items.Values)
            if (it.TypeId == ReportTypeId && (it.ExpiresAt is not { } end || end <= nowUtc || Param(it) == 0 || Read(it) == null)) list.Add(it);
        return list;
    }

    /// <summary>Linhas de todos da partida; result = fim (troféus e medalhas), null = agora (quem não terminou vai como está).</summary>
    public static List<Row> Rows(MassGame g, TourneyResult? result, DateTime nowUtc)
    {
        var rows = new List<Row>(g.Players.Count);
        foreach (var p in g.Players)
        {
            var pl = p.RoomPlayer.Player;
            int medals = 0;
            if (result != null)
                foreach (var m in result.Medals) if (m.Player == p && m.Slot is >= 0 and < 6) medals |= 1 << m.Slot;
            var (mascot, premium) = Looks(pl, nowUtc);
            rows.Add(new Row(p.Guid, pl.Nickname, pl.Level, p.Score, p.Pang, p.Bonus, mascot, premium, pl.Guild?.Id ?? 0,
                pl.Guild?.Mark ?? "", result?.TrophyOf(p) ?? 0, medals, p.Left && !p.Reported));
        }
        return rows;
    }

    /// <summary>Mascote equipado e premium de outro jogador (o inventário dele pode estar mudando agora: sem os dois).</summary>
    static (int Mascot, bool Premium) Looks(Player p, DateTime nowUtc)
    {
        try { return (p.Find(p.Equip.MascotId)?.TypeId ?? 0, Premium.IsActive(p, nowUtc)); }
        catch (InvalidOperationException) { return (0, false); }
    }

    /// <summary>Item novo do relatório: Common[1] = 0 e Common[2] = 1..0x7FFF (o parâmetro do 0xAB fica igual a Common[2]).</summary>
    public static Item NewItem(int id, Data d, DateTime nowUtc, Random rng)
    {
        var it = new Item { Id = id, TypeId = ReportTypeId, Quantity = 1, ExpiresAt = nowUtc + Lifetime };
        it.Set("c1", 0);
        it.Set("c2", rng.Next(1, ParamHigh));
        it.Attrs["date"] = d.Date.ToString("O");
        Write(it, d);
        return it;
    }

    /// <summary>Grava exp, troféu da sala, modo e linhas (o fim da partida troca as linhas pelas finais).</summary>
    public static void Write(Item it, Data d)
    {
        it.Set("exp", d.Exp);
        it.Set("match", d.MatchTid);
        it.Set("mode", d.Mode);
        var rows = new JsonArray();
        foreach (var r in d.Rows)
            rows.Add(new JsonObject
            {
                ["uid"] = r.Uid, ["nick"] = r.Nick, ["lv"] = r.Level, ["score"] = r.Score, ["pang"] = r.Pang, ["bonus"] = r.Bonus,
                ["mascot"] = r.Mascot, ["premium"] = r.Premium, ["guild"] = r.GuildId, ["mark"] = r.GuildMark, ["trophy"] = r.Trophy,
                ["medals"] = r.Medals, ["left"] = r.Left,
            });
        it.Attrs["rows"] = rows;
    }

    public static int Param(Item it) => it.Int("c1") * ParamHigh + it.Int("c2");

    /// <summary>O relatório gravado no item; null = dados inválidos.</summary>
    public static Data? Read(Item it)
    {
        try
        {
            if (it.Attrs["rows"] is not JsonArray arr || arr.Count == 0) return null;
            var rows = new List<Row>(arr.Count);
            foreach (var n in arr)
            {
                if (n is not JsonObject o) return null;
                rows.Add(new Row(o["uid"]!.GetValue<uint>(), o["nick"]!.GetValue<string>(), o["lv"]!.GetValue<int>(),
                    o["score"]!.GetValue<int>(), o["pang"]!.GetValue<long>(), o["bonus"]!.GetValue<long>(), o["mascot"]!.GetValue<int>(),
                    o["premium"]!.GetValue<bool>(), o["guild"]!.GetValue<int>(), o["mark"]!.GetValue<string>(), o["trophy"]!.GetValue<int>(),
                    o["medals"]!.GetValue<int>(), o["left"]!.GetValue<bool>()));
            }
            var date = DateTime.Parse(it.Attrs["date"]!.GetValue<string>(), null, System.Globalization.DateTimeStyles.RoundtripKind);
            return new Data(date, Math.Max(it.Int("exp"), 0), it.Int("match"), it.Int("mode"), rows);
        }
        catch (Exception e) when (e is InvalidOperationException or FormatException or NullReferenceException) { return null; }
    }

    /// <summary>Confere o pedido de abrir: Ok, Fail (não é um relatório dele), BadData (parâmetro/dados) ou BadDate (vencido).</summary>
    public static int Check(Item? it, uint param, DateTime nowUtc) =>
        it is not { TypeId: ReportTypeId, Location: ItemLocation.Inventory } ? Fail
        : (uint)Param(it) != param || Read(it) == null ? BadData
        : it.ExpiresAt <= nowUtc ? BadDate : Ok;
}
