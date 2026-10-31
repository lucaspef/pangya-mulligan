using Pangya.Core.Config;
using Pangya.Domain.Mail;
using Pangya.Domain.Rooms;

namespace Pangya.Domain.Players;

/// <summary>Progresso do World Tour de uma conta: um bit por campo completado e se o prêmio já foi pego.</summary>
public sealed record WorldTourProgress(int MapFlag, bool GotGift);

public interface IWorldTourStore
{
    Task<WorldTourProgress?> GetAsync(long account);
    Task SaveAsync(long account, WorldTourProgress progress);
}

/// <summary>
/// World Tour (docs/server/protocolo/SPEC-jp983-world-tour.md): completar 18 buracos (stroke ou torneio, sem prática) em cada um
/// dos 16 campos do mapa acende o bit do campo; com os 16 (0xFFFF) o jogador pega o prêmio uma vez, pela janela do lobby.
/// Bits do cliente (CMapTypeAdapter): campo 0..11 = bit 0..11, campo 13..16 = bit 12..15; os outros campos não contam.
/// </summary>
public sealed class WorldTourService(IWorldTourStore store, MailService? mail, WorldTourConfig config)
{
    public const int AllMaps = 0xFFFF;

    public WorldTourConfig Config { get; } = config;

    /// <summary>Bit do campo no mapa do World Tour (0 = não conta).</summary>
    public static int Bit(byte course) => course switch
    {
        <= 11 => 1 << course,
        >= 13 and <= 16 => 1 << (course - 1),
        _ => 0,
    };

    /// <summary>O evento está valendo hoje?</summary>
    public bool Open(DateOnly today) =>
        Config.Enabled && (Config.Start is not { } s || today >= s) && (Config.End is not { } e || today <= e);

    public async Task<WorldTourProgress> GetAsync(long account) => await store.GetAsync(account) ?? new WorldTourProgress(0, false);

    /// <summary>Fim de partida: devolve o progresso novo se o campo acendeu agora (null = nada mudou).</summary>
    public async Task<WorldTourProgress?> OnGameEndAsync(long account, GameMode mode, bool practice, byte course, int holes, bool finished,
        DateOnly today)
    {
        if (!Open(today) || practice || !finished || holes < 18 || mode is not (GameMode.Stroke or GameMode.Tournament)) return null;
        int bit = Bit(course);
        if (bit == 0) return null;
        var cur = await GetAsync(account);
        if ((cur.MapFlag & bit) != 0) return null;
        var next = cur with { MapFlag = cur.MapFlag | bit };
        await store.SaveAsync(account, next);
        return next;
    }

    /// <summary>Pedido do prêmio (botão "sim" da janela): só com os 16 campos e uma vez; os itens vão por carta.</summary>
    public async Task<bool> ClaimAsync(long account, DateOnly today)
    {
        if (!Open(today)) return false;
        var cur = await GetAsync(account);
        if (cur.MapFlag != AllMaps || cur.GotGift) return false;
        await store.SaveAsync(account, cur with { GotGift = true });
        if (mail != null && Config.Prizes.Count > 0)
        {
            var items = new List<(int, int)>();
            foreach (var it in Config.Prizes) items.Add((it.TypeId, Math.Max(1, it.Quantity)));
            await mail.SendSystemAsync(account, "@Event", "World Tour", items);
        }
        return true;
    }
}
