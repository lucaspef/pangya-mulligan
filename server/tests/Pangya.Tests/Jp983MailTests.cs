using Pangya.Core.Net;
using Pangya.Core.Text;
using Pangya.Domain.Players;
using Pangya.Protocol.JP983.Game;

namespace Pangya.Tests;

/// <summary>Correio do JP 983 (SPEC-jp983-correio-social.md §1), entre duas contas no cliente JP falso.</summary>
public partial class Jp983GameTests
{
    static PacketWriter MailAttach(PacketWriter w, int id, int tid, int qty) => w.I32(id).U32((uint)tid).U8(0).U32((uint)qty).U32(0).Zeros(38);

    [Fact]
    public async Task MailGoesFromOneJpPlayerToAnother()
    {
        await using var env = new JpEnv();
        var (a, accA) = await InChannelWithAsync(env, 10_000, (0x18000004, 10));
        await using var _ = a;
        var (loginB, accB) = await env.NewAccountAsync();
        var players = env.Game.Context.Kr.Players;
        string nickB = (await players.LoadAsync(accB.Id))!.Nickname;

        // busca de nick: 0xA1 u8 0, u32 uid, MemberInfo 297 + uid
        await a.SendAsync(new PacketWriter(0x07).U8(0).Str(nickB));
        var found = await a.ExpectAsync(0xA1);
        Assert.Equal((0, (uint)accB.Id, 0x12D), (found.U8(), found.U32(), found.Remaining));
        Assert.Equal(nickB, Cp949.Read(found.Bytes(44)[22..]));

        // enviar: 3 de 10, taxa 500
        int stack = (await players.LoadAsync(accA.Id))!.FindType(0x18000004)!.Id;
        await a.SendAsync(MailAttach(new PacketWriter(0x145).U32((uint)accA.Id).U32((uint)accB.Id).Str(nickB).Str("").Str("oi")
            .U64(500).U8(1), stack, 0x18000004, 3));
        var left = await a.ExpectAsync(0x216);
        left.Skip(4 + 4 + 1 + 4 + 4 + 4);
        Assert.Equal((10, 7), (left.I32(), left.I32()));
        Assert.Equal((ulong)(10_000 - 500), (await a.ExpectAsync(0xC8)).U64());
        Assert.Equal(0, (await a.ExpectAsync(0x213)).I32());

        // o destinatário: aviso de não lidas no login (KR 0x15E -> JP 0x210)
        var key = await env.S.Sessions.IssueGameLoginAsync(accB.Id);
        await using var b = await env.ConnectGameAsync();
        await b.SendAsync(GameLogin(loginB, accB.Id, key));
        var unread = await b.ExpectAsync(0x210);
        Assert.Equal((0, 1), (unread.I32(), unread.I32()));
        Assert.Equal(JpSystems.MailBriefSize, unread.Remaining);
        int mailId = unread.I32();
        await b.ExpectAsync(0x96);

        await b.SendAsync(new PacketWriter(0x143).U32(1));                            // listar
        var list = await b.ExpectAsync(0x211);
        Assert.Equal((0, 1, 1, 1), (list.I32(), list.I32(), list.I32(), list.I32()));
        Assert.Equal(mailId, list.I32());
        Assert.Equal(JpSystems.MailBriefSize - 4, list.Remaining);
        list.Skip(30);
        Assert.Equal("oi", Cp949.Read(list.Bytes(80)));
        list.Skip(18 + 4);
        Assert.Equal((0, 1u), (list.U8(), list.U32()));                               // não lida, 1 anexo
        list.Skip(4);
        Assert.Equal(0x18000004u, list.U32());

        await b.SendAsync(new PacketWriter(0x144).U32((uint)mailId));                 // ler
        var read = await b.ExpectAsync(0x212);
        Assert.Equal((0, mailId), (read.I32(), read.I32()));
        read.Str();
        Assert.Matches(@"^\d\d/\d\d/\d{4}$", read.Str());
        Assert.Equal("oi", read.Str());
        Assert.Equal((1, 1, JpSystems.MailItemSize), (read.U8(), read.I32(), read.Remaining));

        await b.SendAsync(new PacketWriter(0x146).U32((uint)mailId));                 // pegar o anexo
        var got = await b.ExpectAsync(0x216);
        got.Skip(4);
        Assert.Equal(1u, got.U32());
        Assert.Equal((2, 0x18000004u), (got.U8(), got.U32()));
        got.Skip(4 + 4);
        Assert.Equal((0, 3), (got.I32(), got.I32()));
        Assert.Equal(0, (await b.ExpectAsync(0x214)).I32());
        Assert.Equal(3, (await players.LoadAsync(accB.Id))!.FindType(0x18000004)!.Quantity);

        await b.SendAsync(new PacketWriter(0x147).U32(1).U32((uint)mailId).U32(1));   // apagar: volta a página sem ela
        var deleted = await b.ExpectAsync(0x215);
        Assert.Equal((0, 1, 1, 0), (deleted.I32(), deleted.I32(), deleted.I32(), deleted.I32()));
    }
}
