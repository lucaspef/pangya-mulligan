using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Domain.Shop;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Pedidos da sala/partida que precisam do banco (docs/server/protocolo/SPEC-jp983-efeitos.md §1, §2, §6): Assist (o item
/// 0x1BE00016 no inventário = ligado; 0x184 -> 0x216 + 0x26A, 0x185 -> 0x26B), Auto Command (0x156 por tacada: gasta 1
/// Auto Commander; o cliente desconta sozinho; erro 0x22B) e trocar de game server pela lista (0x119 -> 0x1D4 u32 0, str chave).
/// </summary>
public sealed partial class JpSystems
{
    public const ushort CAssistToggle = 0x184, CAssistGreen = 0x185, CAutoCommand = 0x156, CChangeServer = 0x119;
    const ushort SAssist = 0x26A, SAssistGreen = 0x26B, SAutoCommand = 0x22B, SChangeServer = 0x1D4;
    public const int AssistTypeId = 0x1BE00016, AutoCommanderTypeId = 0x1A00019F;

    async Task<bool> HandleEffectsAsync(PacketReader r)
    {
        var p = kr.Player;
        switch (r.Id)
        {
            case CAssistToggle:
            {
                r.Skip(r.Remaining);
                if (kr.CurrentRoom?.Game is { Over: false }) return true;              // na partida: ignorar (a referência só manda a semente)
                if (p.FindType(AssistTypeId) is { } on) await SpendAsync(AssistTypeId, on.Quantity);   // desliga: tira o item (0x216)
                else
                {
                    var (code, granted) = await ctx.Shop.GiveAsync(p, AssistTypeId, 1);
                    if (code != ShopCode.Ok || granted.Count == 0) { conn.SendTranslated(new PacketWriter(SAssist).U32(1)); return true; }
                    conn.SendTranslated(ItemsChanged([new ItemChange(AssistTypeId, granted[0].Id, 0, 1)]));
                }
                conn.SendTranslated(new PacketWriter(SAssist).U32(0).U32(AssistTypeId).U32((uint)p.AccountId));
                Log.Info($"{conn} JP: Assist {(p.FindType(AssistTypeId) != null ? "ligado" : "desligado")}");
                return true;
            }
            case CAssistGreen:
            {
                uint tid = r.Remaining >= 4 ? r.U32() : 0;
                r.Skip(r.Remaining);
                conn.SendTranslated(tid == AssistTypeId && p.FindType(AssistTypeId) != null
                    ? new PacketWriter(SAssistGreen).U32(0).U32(tid).U32((uint)p.AccountId) : new PacketWriter(SAssistGreen).U32(1));
                return true;
            }
            case CAutoCommand:
            {
                r.Skip(r.Remaining);
                if (Premium.IsActive(p, DateTime.UtcNow)) return true;               // premium: não gasta
                if (p.FindType(AutoCommanderTypeId) is not { Quantity: > 0 } item) { conn.SendTranslated(new PacketWriter(SAutoCommand).U32(1)); return true; }
                var e = item.Clone();
                e.Quantity--;
                var ch = new PlayerChanges();
                if (e.Quantity == 0) ch.Removed.Add(e.Id); else ch.Updated.Add(e);
                await ctx.Players.Store.ApplyAsync(p.AccountId, ch);               // sem 0x216: o cliente desconta sozinho
                if (e.Quantity == 0) p.Items.Remove(e.Id); else p.Items[e.Id] = e;
                return true;
            }
            case CChangeServer:
            {
                int id = r.Remaining >= 4 ? (int)r.U32() : 0;
                r.Skip(r.Remaining);
                bool known = false;
                if (ctx.Registry is { } reg)
                    foreach (var s in await reg.ListAsync(Jp983.GameServerType)) if (s.Id == id) { known = true; break; }
                if (!known) { conn.SendTranslated(new PacketWriter(SChangeServer).U32(1)); return true; }
                var key = await ctx.Sessions.IssueGameLoginAsync(p.AccountId);
                conn.SendTranslated(new PacketWriter(SChangeServer).U32(0).Str(key));
                Log.Info($"{conn} JP: troca para o game server {id}");
                return true;
            }
        }
        return false;
    }
}
