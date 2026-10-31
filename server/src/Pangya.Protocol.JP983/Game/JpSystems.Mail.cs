using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Mail;
using Pangya.Domain.Players;
using Pangya.Domain.Shop;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Correio do JP (docs/server/protocolo/SPEC-jp983-correio-social.md §1): C->S 0x143..0x147 com o MailService do Domain (o mesmo
/// do KR). Respostas com i32 código no começo (0 = ok; erro: só o código). O aviso de não lidas (JP 0xE4 -> KR 0xE6 ->
/// 0x15E) e a carta nova (KR 0x31 -> JP 0x33) passam pelo tradutor.
/// </summary>
public sealed partial class JpSystems
{
    public const ushort CMailList = 0x143, CMailRead = 0x144, CMailSend = 0x145, CMailTake = 0x146, CMailDelete = 0x147;
    const ushort SMailList = 0x211, SMailRead = 0x212, SMailSent = 0x213, SMailTaken = 0x214, SMailDeleted = 0x215;
    /// <summary>Carta da lista (196) e anexo (55) do JP.</summary>
    public const int MailBriefSize = 196, MailItemSize = 55;
    /// <summary>Erros (referência): 1 sem anexo / carta não existe, 2 não coube no inventário.</summary>
    const int MailError = 1;

    async ValueTask<bool> HandleMailAsync(PacketReader p)
    {
        if (ctx.Mail is not { } mail) { p.Skip(p.Remaining); conn.SendTranslated(new PacketWriter(ReplyId(p.Id)).I32(MailError)); return true; }
        switch (p.Id)
        {
            case CMailList: await MailListAsync(mail, SMailList, (int)p.U32()); return true;
            case CMailRead: await MailReadAsync(mail, (int)p.U32()); return true;
            case CMailTake: await MailTakeAsync(mail, (int)p.U32()); return true;
            case CMailDelete:
            {
                int n = (int)Math.Min(p.U32(), 100u);
                var ids = new List<int>(n);
                for (int i = 0; i < n && p.Remaining >= 4; i++) ids.Add((int)p.U32());
                int page = p.Remaining >= 4 ? (int)p.U32() : 1;
                p.Skip(p.Remaining);
                int failed = await mail.Store.DeleteAsync(kr.Player.AccountId, ids);
                if (failed > 0) conn.SendTranslated(new PacketWriter(SMailDeleted).I32(MailError));
                else await MailListAsync(mail, SMailDeleted, page);           // a página de novo, sem as apagadas
                return true;
            }
            case CMailSend: await MailSendAsync(mail, p); return true;
            default: return false;
        }
    }

    static ushort ReplyId(int request) => request switch
    {
        CMailList => SMailList, CMailRead => SMailRead, CMailSend => SMailSent, CMailTake => SMailTaken, _ => SMailDeleted,
    };

    /// <summary>
    /// Anexo do JP (55): i32 id, u32 typeid, u8 flag de tempo, u32 qtd, u32 tempo, 24 bytes (upgrades/pang/cookie na
    /// referência: zerados), char[9] UCC, 3 bytes, i16 tipo.
    /// </summary>
    public static PacketWriter MailItem(PacketWriter w, int id, int typeId, int quantity) =>
        w.I32(id).U32((uint)typeId).U8(0).U32((uint)quantity).U32(0).Zeros(24 + 9 + 3).I16(0);

    /// <summary>
    /// Carta da lista (196): i32 id, char[30] remetente, char[80] começo do texto, 18 bytes, u32 leituras, u8 lida,
    /// u32 anexos que faltam pegar, 1º anexo (55; zeros se não houver).
    /// </summary>
    public static PacketWriter MailBrief(PacketWriter w, MailLetter m)
    {
        int pending = 0;
        MailItem? first = null;
        foreach (var it in m.Items) if (!it.Taken) { pending++; first ??= it; }
        w.I32(m.Id).Fixed(m.SenderNick, 30).Fixed(m.Message.Length > 79 ? m.Message[..79] : m.Message, 80).Zeros(18).U32(0)
            .U8((byte)(m.Read ? 1 : 0)).U32((uint)pending);
        return first == null ? w.Zeros(MailItemSize) : MailItem(w, first.Id, first.TypeId, first.Quantity);
    }

    /// <summary>0x143 u32 página -> 0x211 (ou 0x215 depois de apagar) i32 0, i32 página, i32 páginas, i32 n, n × 196.</summary>
    async Task MailListAsync(MailService mail, ushort reply, int page)
    {
        page = Math.Max(page, 1);
        var (items, total) = await mail.Store.ListAsync(kr.Player.AccountId, page, MailService.PerPage);
        int pages = Math.Max(1, (total + MailService.PerPage - 1) / MailService.PerPage);
        var w = new PacketWriter(reply, 24 + items.Count * MailBriefSize).I32(0).I32(page).I32(pages).I32(items.Count);
        foreach (var m in items) MailBrief(w, m);
        conn.SendTranslated(w);
    }

    /// <summary>
    /// 0x144 u32 carta -> 0x212 i32 0, i32 id, str remetente, str data (dd/MM/yyyy), str texto, u8 tem anexos, i32 n,
    /// n × 55 (55 zeros se n = 0, como a referência); marca lida.
    /// </summary>
    async Task MailReadAsync(MailService mail, int id)
    {
        var m = await mail.Store.GetAsync(kr.Player.AccountId, id);
        if (m == null) { conn.SendTranslated(new PacketWriter(SMailRead).I32(MailError)); return; }
        await mail.Store.MarkReadAsync(kr.Player.AccountId, id);
        var pending = new List<MailItem>();
        foreach (var it in m.Items) if (!it.Taken) pending.Add(it);
        var w = new PacketWriter(SMailRead, 96 + m.Message.Length * 2 + Math.Max(1, pending.Count) * MailItemSize).I32(0).I32(m.Id)
            .Str(m.SenderNick).Str(m.CreatedAt.ToLocalTime().ToString("dd/MM/yyyy")).Str(m.Message)
            .U8((byte)(pending.Count > 0 ? 1 : 0)).I32(pending.Count);
        if (pending.Count == 0) w.Zeros(MailItemSize);
        foreach (var it in pending) MailItem(w, it.Id, it.TypeId, it.Quantity);
        conn.SendTranslated(w);
    }

    /// <summary>
    /// 0x146 u32 carta -> 0x216 (itens que entraram) + 0xC8 (se veio pang) + 0x214 i32 0; erro: 0x214 i32 código.
    /// </summary>
    async Task MailTakeAsync(MailService mail, int id)
    {
        var p = kr.Player;
        var (ok, placed, pang) = await mail.TakeAsync(p, id);
        if (!ok) { conn.SendTranslated(new PacketWriter(SMailTaken).I32(MailError)); return; }
        var changes = new List<ItemChange>(placed.Count);
        foreach (var (m, it) in placed)
        {
            int after = it.IsConsumable ? it.Quantity : 1;
            changes.Add(new ItemChange(it.TypeId, it.Id, it.IsConsumable ? Math.Max(0, after - m.Quantity) : 0, after));
        }
        if (changes.Count > 0) conn.SendTranslated(ItemsChanged(changes));
        if (pang > 0) conn.SendTranslated(new PacketWriter(SPang).U64((ulong)p.Pang).U64(0));
        conn.SendTranslated(new PacketWriter(SMailTaken).I32(0));
        Log.Info($"{conn} JP: pegou {placed.Count} anexo(s) da carta {id}{(pang > 0 ? $" e {pang} pang" : "")}");
    }

    /// <summary>
    /// 0x145 u32 meu uid, u32 destino, str nick, str "", str texto, u64 taxa (o servidor calcula), u8 n, n × 55 ->
    /// 0x216 (o que saiu) + 0xC8 pang + 0x213 i32 0; erro: 0x213 i32 código (MailCode). Destinatário online: KR 0x31.
    /// </summary>
    async Task MailSendAsync(MailService mail, PacketReader r)
    {
        var p = kr.Player;
        r.U32();
        long to = r.U32();
        r.Str(32);
        r.Str(64);
        string text = r.Str(MailService.MaxMessage + 1);
        r.U64();
        int n = r.U8();
        var attach = new List<(int Id, int TypeId, int Count)>(n);
        for (int i = 0; i < n && r.Remaining >= MailItemSize; i++)
        {
            int id = r.I32(), tid = (int)r.U32();
            r.U8();
            int count = (int)r.U32();
            r.Skip(MailItemSize - 13);
            attach.Add((id, tid, count));
        }
        r.Skip(r.Remaining);
        if (n > MailService.MaxItems) { conn.SendTranslated(new PacketWriter(SMailSent).I32((int)MailCode.TooManyItems)); return; }
        var before = new Dictionary<int, int>(attach.Count);
        foreach (var (id, _, _) in attach) before[id] = p.Find(id)?.Quantity ?? 0;
        var (code, left) = await mail.SendAsync(p, to, text, attach);
        if (code != MailCode.Ok) { conn.SendTranslated(new PacketWriter(SMailSent).I32((int)code)); return; }
        var changes = new List<ItemChange>(left.Count);
        foreach (var it in left)
        {
            int was = before.GetValueOrDefault(it.Id);
            changes.Add(new ItemChange(it.TypeId, it.Id, it.IsConsumable ? was : 1, it.Quantity));
        }
        if (changes.Count > 0) conn.SendTranslated(ItemsChanged(changes));
        conn.SendTranslated(new PacketWriter(SPang).U64((ulong)p.Pang).U64(0));
        conn.SendTranslated(new PacketWriter(SMailSent).I32(0));
        if (ctx.World.Find(to) is GameHandler h) h.Connection.Send(new PacketWriter(0x31));   // carta nova (KR 0x31: cada um traduz)
        Log.Info($"{conn} JP: mandou carta para {to} com {attach.Count} anexo(s)");
    }
}
