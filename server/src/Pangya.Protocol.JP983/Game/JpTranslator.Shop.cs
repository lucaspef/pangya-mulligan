using Pangya.Core.Net;
using Pangya.Domain.Players;

namespace Pangya.Protocol.JP983.Game;

/// <summary>Loja, equipamento, My Room, armário, Papel Shop, Caixa Mágica e Self Design (SPEC-jp983-loja.md).</summary>
public sealed partial class JpTranslator
{
    /// <summary>Item da compra/presente: o JP manda 37 bytes (+ pang, cookie e 13 de cupom); o KR lê os 16 primeiros.</summary>
    const int JpBuyItem = 37, KrBuyItem = 16;
    /// <summary>Item do armário: JP 176 = KR 174 com um u16 a mais no offset 111.</summary>
    const int KrStoredItem = 174, JpStoredItem = 176, StoredGap = 111;

    /// <summary>Flag do 0xFB sem limite por dia (referência; -2 = com limite).</summary>
    internal const int PapelNoLimit = -3;

    /// <summary>Cupom gasto na jogada do Papel em andamento (KR 0xD3 antes do 0xD4).</summary>
    int papelCoupon;
    readonly Dictionary<int, (int TypeId, int Quantity)> couponsBefore = [];

    /// <summary>Tipo do último 0x20 (o erro 0x6B do JP leva o tipo; o KR manda só o código).</summary>
    byte lastEquipType;

    void AddShop()
    {
        fromClient[0x1D] = r =>                                                // comprar: u8, u16 n, n × 37, i32 cupom
        {
            byte opt = r.U8();
            int n = r.U16();
            return Body(0x1D, w => { w.U8(opt).U16((ushort)n); BuyItems(r, w, n); });
        };
        fromClient[0x1F] = r =>                                                // presente: str nick, u32 uid, str msg, u8, u16 n, n × 37
        {
            string nick = r.Str(32), msg;
            uint uid = r.U32();
            msg = r.Str(256);
            byte opt = r.U8();
            int n = r.U16();
            return Body(0x1F, w => { w.Str(nick).U32(uid).Str(msg).U8(opt).U16((ushort)n); BuyItems(r, w, n); });
        };
        fromClient[0x20] = Equip;
        fromClient[0x158] = r =>                                               // Caixa Mágica: u16 receita, u32 vezes, u8 n, n × (u32, i32)
        {
            ushort recipe = r.U16();
            uint times = r.U32();
            int n = r.U8();
            return Body(0x7E, w => w.U16(recipe).U8((byte)Math.Clamp(times, 1, 255)).U8((byte)n).Bytes(r.Bytes(Math.Min(n * 8, r.Remaining))));
        };
        fromClient[0xCE] = r => StoredItemsToKr(r, 0xD0);                     // guardar no armário
        fromClient[0xCF] = r => StoredItemsToKr(r, 0xD1);                     // retirar do armário

        toClient[0x69] = (jp, k) => EquipResult(jp, k, kr.Player);
        toClient[0x68] = (jp, k) => k.Length == 4                               // presente: o JP lê sempre u32, u64 pang, u64 cookie
            ? new PacketWriter(jp).Bytes(k).U64((ulong)kr.Player.Pang).U64((ulong)kr.Player.Cookie) : new PacketWriter(jp).Bytes(k);
        toClient[0xE0] = (jp, k) => k.Length == 5                               // mensagem do mascote, erro: + str, u64 pang
            ? new PacketWriter(jp).Bytes(k).Str("").U64((ulong)kr.Player.Pang) : new PacketWriter(jp).Bytes(k);
        toClient[0xA3] = (jp, k) =>                                            // upgrade: no JP o 2º byte é 0 personagem / 1 club set
        {
            if (k.Length > 1) k[1] &= 1;
            return new PacketWriter(jp).Bytes(k);
        };
        toClient[0x109] = (jp, k) => new PacketWriter(jp).U32(0).I64(JpStructs.PapelUnlimited);   // abrir o Papel Shop: u32 ok, i64 limite por dia
        toClient[0xF9] = (jp, k) => new PacketWriter(jp).I32(-1).I32(PapelNoLimit);              // jogadas: -1 restantes, -3 = sem limite
        // Papel com cupom: o JP não usa o 0xD5 (KR 0xD3, "cupom gasto"); como a referência, o id do cupom vai no 0x21B e a
        // pilha nova no 0x216 (sem isso a contagem de cupons na tela não cai)
        fromClient[0x14B] = r =>                                               // jogar: guarda os cupons de antes (KR 0x6D)
        {
            couponsBefore.Clear();
            foreach (var it in kr.Player.Items.Values)
                if (Domain.Shop.LotteryService.CouponKind(it.TypeId) == Domain.Shop.LotteryService.PapelCouponKind)
                    couponsBefore[it.Id] = (it.TypeId, it.Quantity);
            return Body(0x6D, _ => { });
        };
        toClient[0xD3] = (jp, k) => { papelCoupon = k.Length >= 4 ? BitConverter.ToInt32(k, 0) : 0; return null; };
        toClient[0xD4] = (jp, k) =>                                            // resultado: + i32 cupom gasto
        {
            int coupon = papelCoupon;
            papelCoupon = 0;
            if (coupon != 0 && couponsBefore.TryGetValue(coupon, out var before))
                conn.SendTranslated(JpSystems.ItemsChanged([new Domain.Shop.ItemChange(before.TypeId, coupon, before.Quantity, kr.Player.Find(coupon)?.Quantity ?? 0)]));
            return new PacketWriter(jp).Bytes(k.AsSpan(0, 4)).I32(coupon).Bytes(k.AsSpan(4));
        };
        toClient[0xED] = MagicBoxResult;
        toClient[0x16E] = (jp, k) => k.Length >= KrToJp.KrSlotSize + KrToJp.KrCharacterSize   // avatar no My Room: 348 + 513
            ? new PacketWriter(jp).Slot(k.AsSpan(0, KrToJp.KrSlotSize)).Character(k.AsSpan(KrToJp.KrSlotSize, KrToJp.KrCharacterSize), Known, Extras)
            : null;
        toClient[0x172] = (jp, k) =>                                           // página do armário: u16, u16, u8 n, n × 174 -> 176
        {
            int n = k[4];
            var w = new PacketWriter(jp, 8 + n * JpStoredItem).Bytes(k.AsSpan(0, 5));
            for (int i = 0; i < n; i++)
            {
                var it = k.AsSpan(5 + i * KrStoredItem, KrStoredItem);
                w.Bytes(it[..StoredGap]).U16(0).Bytes(it[StoredGap..]);
            }
            return w;
        };
        toClient[0x126] = Ucc;

        // loja pessoal do lounge: item 170 (KR) <-> 172 (JP, u16 a mais no 107); o JP não tem o "u32 tipo" do KR
        fromClient[0x7C] = r =>                                                // publicar: u32 n, n × 172 -> u32 tipo, u32 n, n × 170
        {
            uint n = r.U32();
            return Body(0x7C, w => { w.U32(1).U32(n); TradeItemsToKr(r, w, (int)n); });
        };
        fromClient[0x7D] = r =>                                                // comprar: u32 dono, 172 -> u32, 170
        {
            uint owner = r.U32();
            return Body(0x7D, w => { w.U32(owner); TradeItemsToKr(r, w, 1); });
        };
        toClient[0xE4] = (jp, k) =>                                            // entrar na loja: u32 1, [u32 tipo], char[22], str, u32, u32 n, itens
        {
            if (k.Length < 8 + 22 + 2) return new PacketWriter(jp).Bytes(k);   // recusado: só o código
            int o = 8 + 22, strLen = 2 + BitConverter.ToUInt16(k, o);
            var w = new PacketWriter(jp).Bytes(k.AsSpan(0, 4)).Bytes(k.AsSpan(8, 22 + strLen + 4));
            return TradeItems(w, k, o + strLen + 4);
        };
        toClient[0xE9] = (jp, k) =>                                            // publicada: u32 1, [u32 tipo], char[22], u32 uid, u32 n, itens
            k.Length < 34 ? new PacketWriter(jp).Bytes(k)                         // recusada: só o código
                : TradeItems(new PacketWriter(jp).Bytes(k.AsSpan(0, 4)).Bytes(k.AsSpan(8, 26)), k, 34);
        // compra na loja pessoal (0xEC no JP, FUN_009c20a0 caso 0x9CD65A): u32 código (1 = ok), u8 quem (0 comprador, 1
        // vendedor), u64 pang, item da loja 170 -> 172; o comprador ainda u8 tipo (1..3 = objeto) + sItemInfo 168 -> 196.
        // Recusa: só o código.
        toClient[0xEA] = (jp, k) =>
        {
            const int head = 4 + 1 + 8;
            if (k.Length < head + KrToJp.KrTradeItemSize) return new PacketWriter(jp).Bytes(k);
            var w = new PacketWriter(jp).Bytes(k.AsSpan(0, head)).TradeItem(k.AsSpan(head, KrToJp.KrTradeItemSize));
            int o = head + KrToJp.KrTradeItemSize;
            if (k.Length >= o + 1 + KrToJp.KrItemSize) w.U8(k[o]).Item(k.AsSpan(o + 1, KrToJp.KrItemSize), WorkshopTail);
            return w;
        };
        toClient[0xEB] = (jp, k) =>                                            // item restante: str nick, u32 uid, 170, u32
        {
            int o = 2 + BitConverter.ToUInt16(k, 0) + 4;
            return new PacketWriter(jp).Bytes(k.AsSpan(0, o)).TradeItem(k.AsSpan(o, KrToJp.KrTradeItemSize)).Bytes(k.AsSpan(o + KrToJp.KrTradeItemSize));
        };
    }

    /// <summary>u32 n (no offset nAt) e n itens de loja pessoal 170 -> 172.</summary>
    static PacketWriter TradeItems(PacketWriter w, byte[] k, int nAt)
    {
        int n = (int)BitConverter.ToUInt32(k, nAt);
        w.U32((uint)n);
        for (int i = 0; i < n && nAt + 4 + (i + 1) * KrToJp.KrTradeItemSize <= k.Length; i++)
            w.TradeItem(k.AsSpan(nAt + 4 + i * KrToJp.KrTradeItemSize, KrToJp.KrTradeItemSize));
        return w;
    }

    static void TradeItemsToKr(PacketReader r, PacketWriter w, int n)
    {
        for (int i = 0; i < n && r.Remaining >= KrToJp.TradeItemSize; i++) w.Bytes(KrToJp.TradeItemToKr(r.Bytes(KrToJp.TradeItemSize)));
        r.Skip(r.Remaining);
    }

    /// <summary>Itens da compra: 16 primeiros de cada 37 (o servidor calcula o preço; o cupom do fim não existe no KR).</summary>
    static void BuyItems(PacketReader r, PacketWriter w, int n)
    {
        for (int i = 0; i < n && r.Remaining >= JpBuyItem; i++) w.Bytes(r.Bytes(JpBuyItem)[..KrBuyItem]);
        r.Skip(r.Remaining);
    }

    /// <summary>Armário (guardar/retirar): u8 n, n × 176 -> n × 174.</summary>
    static byte[]? StoredItemsToKr(PacketReader r, ushort krId)
    {
        int n = r.U8();
        return Body(krId, w =>
        {
            w.U8((byte)n);
            for (int i = 0; i < n && r.Remaining >= JpStoredItem; i++)
            {
                var it = r.Bytes(JpStoredItem);
                w.Bytes(it[..StoredGap]).Bytes(it[(StoredGap + 2)..]);
            }
        });
    }

    /// <summary>
    /// 0x20 equipar: o tipo 0 traz o personagem do JP (513 -> 444). Tipos 9 (cut-in: u32 personagem + u32[4]) e 10 (pôster:
    /// u32[2]) só existem no JP: respondidos aqui com o eco (ainda não gravados).
    /// </summary>
    byte[]? Equip(PacketReader r)
    {
        if (r.Remaining < 1) return Pass;
        lastEquipType = r.U8();
        if (lastEquipType is 9 or 10)
        {
            var data = r.Bytes(r.Remaining).ToArray();
            Reply(0x6B, w => w.U8(4).U8(lastEquipType).Bytes(data));
            return null;
        }
        if (lastEquipType == 0 && r.Remaining >= JpStructs.CharacterSize)
            return Body(0x20, w => w.U8(0).Bytes(KrToJp.CharacterToKr(r.Bytes(JpStructs.CharacterSize))));
        return Pass;
    }

    /// <summary>
    /// 0x69 KR -> 0x6B JP (u8 resultado 4 ok / 5 erro, u8 tipo, dados): 0 personagem e 8 mascote convertidos; o JP ainda
    /// lê no 1 o id do caddie e no 3 a bola e o id do club set, que o KR não manda (referência: pacote06B); no erro, o tipo.
    /// </summary>
    PacketWriter EquipResult(ushort jp, byte[] k, Player p)
    {
        var w = new PacketWriter(jp).U8(k[0]);
        if (k.Length < 2) return w.U8(lastEquipType);
        w.U8(k[1]);
        var data = k.AsSpan(2);
        return k[1] switch
        {
            0 when data.Length >= KrToJp.KrCharacterSize => w.Character(data, null, Extras),
            8 when data.Length >= KrToJp.KrMascotSize => w.Mascot(data),
            8 => w.Zeros(JpStructs.MascotSize),
            1 => w.I32(p.Equip.CaddieId),
            3 => w.U32((uint)(p.Equip.BallTypeId != 0 ? p.Equip.BallTypeId : Item.BasicBall)).I32(p.Equip.ClubSetId),
            _ => w.Bytes(data),
        };
    }

    /// <summary>
    /// 0xED KR -> 0x22F JP (Caixa Mágica): KR u32 0, u16 receita, u8 n, n × 0x50 (u32 tid, u32 id, u16 qtd, ...) -> JP u32 0,
    /// u32 receita, u32 n, n × (u32 tid, i32 id, i32 qtd ganha, i32 qtd total, u32 0). Erro: só u32 código.
    /// </summary>
    PacketWriter MagicBoxResult(ushort jp, byte[] k)
    {
        if (k.Length < 7) return new PacketWriter(jp).Bytes(k);
        int n = k[6];
        var w = new PacketWriter(jp).U32(0).U32(BitConverter.ToUInt16(k, 4)).U32((uint)n);
        for (int i = 0; i < n && 7 + (i + 1) * 0x50 <= k.Length; i++)
        {
            int o = 7 + i * 0x50, id = BitConverter.ToInt32(k, o + 4);
            w.Bytes(k.AsSpan(o, 8)).I32(BitConverter.ToUInt16(k, o + 8)).I32(kr.Player.Find(id)?.Quantity ?? 1).U32(0);
        }
        return w;
    }

    /// <summary>Self Design 0x126 KR -> 0x12E JP: só o sub 1 (u8 1, u32 guid, str índice, u8 flag, item) muda: item 168 -> 196.</summary>
    static PacketWriter Ucc(ushort jp, byte[] k)
    {
        if (k[0] != 1 || k.Length < 8) return new PacketWriter(jp).Bytes(k);
        int o = 5 + 2 + BitConverter.ToUInt16(k, 5) + 1;
        return k.Length >= o + KrToJp.KrItemSize ? new PacketWriter(jp).Bytes(k.AsSpan(0, o)).Item(k.AsSpan(o, KrToJp.KrItemSize))
            : new PacketWriter(jp).Bytes(k);
    }
}
