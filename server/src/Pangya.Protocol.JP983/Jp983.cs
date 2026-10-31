namespace Pangya.Protocol.JP983;

/// <summary>Constantes da versão do cliente JP "R7.983.00" (build de 09/11/2017).</summary>
public static class Jp983
{
    /// <summary>Versão anunciada no 0x44 (no máximo 12 caracteres; a referência usa a do cliente).</summary>
    public const string ClientVersion = "JP.R7.983.00";
    /// <summary>Tipo do game server JP no registro de servidores (o login do JP só lista estes).</summary>
    public const string GameServerType = "game-jp";
    /// <summary>Tipos de sala que só o JP tem (referência: ROOM_INFO_TYPE): prática e Grand Prix.</summary>
    public const byte PracticeType = 19, GrandPrixType = 20;
}
