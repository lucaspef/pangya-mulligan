using Pangya.Domain.Players;
using Pangya.Domain.Ranking;
using Pangya.Domain.Rooms;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Tests;

/// <summary>Taxa de abandono: faixas do aviso de saída, mínimo de jogos e pena (SPEC-resultado-fim-de-jogo.md §4.5).</summary>
public class QuitRateTests
{
    static Player At(int percent, int level = 6, long games = 100, long id = 1) =>
        new() { AccountId = id, Nickname = "p" + id, Level = level, Stats = new PlayerStats { Games = games, Quits = games * percent / 100, Pangya = 9, Hole = 9 } };

    [Theory]
    [InlineData(20, false, false, false, 0u)]
    [InlineData(21, true, false, false, 0u)]
    [InlineData(30, true, false, false, 0u)]
    [InlineData(31, true, true, false, 1u)]
    [InlineData(40, true, true, false, 1u)]
    [InlineData(41, true, true, true, 2u)]
    public void Thresholds(int percent, bool noRanking, bool noTeam30, bool top, uint tier)
    {
        var p = At(percent);
        Assert.Equal(percent, QuitRate.Of(p));
        Assert.Equal(noRanking, QuitRate.NoRanking(p));
        Assert.Equal(noTeam30, QuitRate.Barred(p, GameMode.Team30s));
        Assert.Equal(top, QuitRate.Barred(p, GameMode.PangBattle));
        Assert.Equal(top, QuitRate.NoTrophy(p));
        Assert.Equal(tier, QuitRate.IconTier(p));
        Assert.False(QuitRate.Barred(p, GameMode.Stroke));
    }

    [Theory]
    [InlineData(6, 50)]                                                    // o cliente esconde até 50 jogos, inclusive
    [InlineData(5, 100)]                                                   // e abaixo do nível 6
    public void BelowMinimumNothingApplies(int level, long games)
    {
        var p = At(100, level, games);
        Assert.Equal(0, QuitRate.Of(p));
        Assert.False(QuitRate.NoRanking(p) || QuitRate.NoTrophy(p) || QuitRate.Barred(p, GameMode.PangBattle) || QuitRate.Manner(p));
        Assert.Equal(0u, QuitRate.IconTier(p));
    }

    [Fact]
    public void FiftyOneGamesIsTheFirstThatCounts() => Assert.True(QuitRate.NoTrophy(At(100, 6, 51)));

    [Fact]
    public void RatePercentIsTruncatedLikeTheClient() =>
        Assert.False(QuitRate.NoRanking(new Player { Level = 6, Stats = new PlayerStats { Games = 1000, Quits = 209 } }));

    [Fact]
    public void GmIsNotBarred()
    {
        var p = new Player { Level = 6, IdentityFlags = 4, Stats = new PlayerStats { Games = 100, Quits = 100 } };
        Assert.False(QuitRate.Barred(p, GameMode.PangBattle));
    }

    [Fact]
    public void RoomsRefuseBarredPlayer()
    {
        var mgr = new RoomManager();
        var room = mgr.Create(new RoomSettings { Mode = GameMode.PangBattle }, 1);
        Assert.Equal(JoinResult.Restricted, RoomManager.CanJoin(room, "", At(41)));
        Assert.Equal(JoinResult.Ok, RoomManager.CanJoin(room, "", At(40)));
    }

    [Fact]
    public void RankingLeavesOutFrom21Percent()
    {
        var kept = At(20);
        var left = At(21, id: 2);
        var snap = RankingService.Build([kept, left], null, DateTime.UtcNow);
        Assert.Contains(1L, snap.Players.Keys);
        Assert.DoesNotContain(2L, snap.Players.Keys);
    }

    [Theory]
    [InlineData(GameMode.Stroke, 1, 10L, 0L, 100)]                         // só a base
    [InlineData(GameMode.Match, 1, 10L, 0L, 350)]
    [InlineData(GameMode.PangBattle, 1, 10L, 0L, 350)]
    [InlineData(GameMode.Team, 1, 10L, 0L, 300)]
    [InlineData(GameMode.Tournament, 1, 10L, 0L, 150)]
    [InlineData(GameMode.Team30s, 1, 10L, 0L, 200)]
    [InlineData(GameMode.GuildMatch, 1, 10L, 0L, 0)]
    [InlineData(GameMode.NewApproach, 30, 100L, 50L, 0)]                   // a janela não mostra pena
    [InlineData(GameMode.Stroke, 6, 50L, 25L, 200)]                        // nível 6: +100; até 50 jogos, a taxa não entra
    [InlineData(GameMode.Stroke, 26, 100L, 25L, 750)]                      // +500 do nível, +150 de (25 - 10) × 10
    [InlineData(GameMode.Stroke, 6, 100L, 0L, 100)]                        // taxa abaixo de 10 desconta
    public void PangPenaltyMatchesTheClient(GameMode mode, int level, long games, long quits, long penalty) =>
        Assert.Equal(penalty, QuitRate.PangPenalty(mode, level, games, quits));

    [Fact]
    public void SlotAndLobbyEntryCarryTheIconTier()
    {
        var p = At(41);
        Assert.Equal(4u, RoomPackets.BriefUser(p).gender);                 // i_male_03
        Assert.Equal(2u, RoomPackets.SlotInfo(new RoomPlayer { Guid = 1, Player = At(31) }).gender);   // i_male_02
        Assert.Equal(1u, RoomPackets.BriefUser(At(2)).manner);
    }
}
