using Pangya.Core.Net;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Pedidos que só o cliente JP faz (sistemas que o KR não tem): por enquanto a resposta mínima válida de cada um
/// (SPEC-jp983-sistemas.md, SPEC-jp983-loja.md), para a tela não ficar esperando. Recusas usam o código 1.
/// </summary>
public sealed partial class JpTranslator
{
    const uint Refused = 1;

    /// <summary>Categoria (0 rookie, 1 beginner, 2 advancer) da última missão do tutorial concluída (0xAE); -1 = nenhuma.</summary>
    int tutorialCategory = -1;

    void AddJpOnly()
    {
        // tutorial (offline no cliente): 0xAE u8 último, u8 categoria, u32 bit -> KR 0xA6 (mesmo layout). A resposta KR 0x11B
        // (u8 3, u8 0, 3 × u32, igual à do login) vira, depois de uma missão, u8 categoria, u8 1, u32 missões dela (referência).
        fromClient[0xAE] = r =>
        {
            if (r.Remaining >= 2) { r.U8(); tutorialCategory = Math.Min((int)r.U8(), 2); }
            return Pass;
        };
        toClient[0x11B] = (jp, k) =>
        {
            int cat = tutorialCategory;
            tutorialCategory = -1;
            if (cat < 0 || k.Length < 2 + 12) return new PacketWriter(jp).Bytes(k);
            return new PacketWriter(jp).U8((byte)cat).U8(1).Bytes(k.AsSpan(2 + cat * 4, 4));
        };
        // abrir a loja: o KR não responde ao 0x5C; o JP pede a hora do servidor (0xBA SYSTEMTIME)
        fromClient[0x5C] = r => { Reply(0xBA, w => w.Time(DateTime.Now)); return Pass; };
        const int ShotEndSize = 0x57;
        const ushort SShotEnd = 0x1F7;
        fromClient[0x140] = r => { Reply(0x20E, w => w.U32(0).U32(0)); return null; };          // atualizar a loja
        // janela de presentes da quest, contadores do evento da oficina e comandos do console de debug: ninguém responde
        // 0xAD u32 guid = prêmio do World Tour: vai para o JpSystems com um id interno (viraria o KR 0xA5, pendentes da guilda)
        fromClient[0xAD] = r => Body(JpSystems.CWorldTourGift, w => w.Bytes(r.Bytes(r.Remaining)));
        // relatório do Tiki: 0xAA usar o pergaminho e 0xAB abrir o relatório (sem par no KR; o KR 0xAA é a caixa S4)
        fromClient[0xAA] = r => Body(JpSystems.CTikiReportUse, w => w.Bytes(r.Bytes(r.Remaining)));
        fromClient[0xAB] = r => Body(JpSystems.CTikiReportOpen, w => w.Bytes(r.Bytes(r.Remaining)));
        foreach (var quiet in new[] { 0xB0, 0x174, 0x175, 0x14C, 0x14D, 0x14E, 0x14F, 0x150, 0x16A, 0x170, 0x18F, 0x190, 0x191, 0x193, 0x194, 0x195 })
            fromClient[quiet] = r => null;
        fromClient[0xFE] = r => null;                                                           // chave web do UCC
        // fim da tacada (u8[0x57]) -> 0x1F7 a toda a sala: i32 guid de quem tacou, u8 buraco, os mesmos 0x57 bytes; o cliente
        // desenha a trajetória dos outros (ShotTrajectoryForMassGame) e posiciona os cubos (referência requestShotEndData)
        fromClient[0x12F] = r =>
        {
            var data = r.Bytes(Math.Min(r.Remaining, ShotEndSize)).ToArray();
            if (data.Length == ShotEndSize && kr.ShotEndTarget() is { } t)
                foreach (var c in t.To) c.SendTranslated(new PacketWriter(SShotEnd, 8 + ShotEndSize).U32(t.Guid).U8(t.Hole).Bytes(data));
            return null;
        };
    }
}
