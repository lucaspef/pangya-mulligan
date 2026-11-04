using Pangya.Core.Net;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Aniversário da Arin 2014: só abre a janela e anda pelas etapas. Respostas: 0x27A lista (u32 erro, u32 n, n × {u32 id,
/// u32 valor, u32 meta, u8 feita, u8 premiada}; com menos de 5 missões o cliente não abre), 0x279 etapa 3, 0x278 etapa 5,
/// 0x27B prêmio (u32 erro, u32 id), 0x27C mapa. Missões e prêmios oficiais não são conhecidos: ninguém conclui nem ganha nada.
/// </summary>
public sealed partial class JpSystems
{
    public const ushort CArinStart = 0x190, CArinTickets = 0x191, CArinOpen = 0x192, CArinReward = 0x193, CArinFinal = 0x194;
    const ushort SArinStep5 = 0x278, SArinStep3 = 0x279, SArinList = 0x27A, SArinReward = 0x27B, SArinFinal = 0x27C;
    const int ArinMissions = 5, ArinFirstMission = 0x15;

    /// <summary>Eventos sem servidor (Kooh): o 0x278 com erro fecha a espera e mostra a falha.</summary>
    public const ushort CMissionEvent = 0xE1, CBingoEvent = 0xE3, CKoohEvent = 0xED;

    bool EventUnavailable(PacketReader r)
    {
        r.Skip(r.Remaining);
        conn.SendTranslated(new PacketWriter(SArinStep5).U32(1));
        return true;
    }

    bool HandleArin(PacketReader r)
    {
        uint reward = r.Id == CArinReward ? r.U32() : 0;
        r.Skip(r.Remaining);
        switch (r.Id)
        {
            case CArinOpen:
                var w = new PacketWriter(SArinList, 8 + ArinMissions * 14).U32(0).U32(ArinMissions);
                for (int i = 0; i < ArinMissions; i++) w.U32((uint)(ArinFirstMission + i)).U32(0).U32(1).U8(0).U8(0);
                conn.SendTranslated(w);
                break;
            case CArinStart: conn.SendTranslated(new PacketWriter(SArinStep3).U32(0)); break;
            case CArinTickets: conn.SendTranslated(new PacketWriter(SArinStep5).U32(0)); break;
            case CArinReward: conn.SendTranslated(new PacketWriter(SArinReward).U32(0).U32(reward)); break;
            case CArinFinal: conn.SendTranslated(new PacketWriter(SArinFinal).U32(0)); break;
        }
        return true;
    }
}
