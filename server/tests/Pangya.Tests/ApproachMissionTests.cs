using Pangya.Domain.Rooms;

namespace Pangya.Tests;

/// <summary>Missões do approach (SPEC-jp983-eventos-drops.md §4): sorteio e julgamento.</summary>
public class ApproachMissionTests
{
    static readonly (uint, uint, uint, int)[] Hole =
    [
        (1, 35, 12_000, 2),      // 3,5 y
        (2, 0, 15_000, 1),       // chip-in
        (3, 120, 5_000, 3),      // 12 y
        (4, ApproachMissions.Out, 0, 255),
    ];

    [Fact]
    public void MissionsJudgeByDistanceTimeAndRank()
    {
        Assert.Equal([2u], ApproachMissions.Judge(new(3, 2, 2, 0, 0), Hole).Keys);                 // chip-in
        Assert.Equal([3u], ApproachMissions.Judge(new(7, 1, 2, 3, 0), Hole).Keys);                 // 3º lugar
        var near = ApproachMissions.Judge(new(14, 1, 2, 5, 0), Hole);                               // < 5 y: caixas pela distância
        Assert.Equal((3, 1), (near[2], near[1]));
        Assert.False(near.ContainsKey(3));
        Assert.Equal([3u], ApproachMissions.Judge(new(20, 1, 2, 0, 0), Hole).Keys);                // mais longe
        Assert.Equal([2u], ApproachMissions.Judge(new(24, 1, 2, 0, 0), Hole).Keys);                // mais tempo sobrando
        Assert.Equal(3, ApproachMissions.Judge(new(1, 1, 1, 0, 0), Hole).Count);                    // 1º com 0 y (par): todos
        Assert.Empty(ApproachMissions.Judge(new(1, 1, 1, 1, 0), Hole));
        Assert.Null(ApproachMissions.Roll(4, new Random(1), chancePercent: 0));
        var m = ApproachMissions.Roll(4, new Random(2), chancePercent: 100)!;
        Assert.InRange(m.No, 1, 31);
    }
}
