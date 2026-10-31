using System.Text.Json;
using System.Text.Json.Serialization;

namespace Pangya.Core.Config;

/// <summary>
/// Configuração de todos os servidores, lida de um arquivo JSON (padrão: config/pangya.json).
/// Nada de IP, porta, banco ou segredo fica no código. Campos desconhecidos no arquivo são erro
/// (evita erro de digitação passar despercebido).
/// </summary>
public sealed class PangyaConfig
{
    /// <summary>Servidores que este processo roda (se a linha de comando não disser): web, login, game, messenger.</summary>
    public string[] Run { get; set; } = ["web", "login", "game"];
    public NetworkConfig Network { get; set; } = new();
    public WebConfig Web { get; set; } = new();
    public LoginConfig Login { get; set; } = new();
    public GameConfig Game { get; set; } = new();
    /// <summary>Game server do cliente JP 983 (outra porta e outro mundo; null = não sobe). Roda com o nome "game-jp".</summary>
    public GameConfig? GameJp { get; set; }
    public DatabaseConfig Database { get; set; } = new();
    public LimitsConfig Limits { get; set; } = new();
    public LoggingConfig Logging { get; set; } = new();
    public DataConfig Data { get; set; } = new();
    public NewPlayerConfig NewPlayer { get; set; } = new();
    public LotteryConfig Lottery { get; set; } = new();
    /// <summary>Gacha do JP (site do jogo).</summary>
    public GachaConfig Gacha { get; set; } = new();
    public MemorialConfig Memorial { get; set; } = new();
    /// <summary>Conteúdo das caixas do My Room (o cliente não traz).</summary>
    public BoxConfig Boxes { get; set; } = new();
    public AttendanceConfig Attendance { get; set; } = new();
    public MessengerConfig Messenger { get; set; } = new();
    /// <summary>Mensageiro do cliente JP 983 (outra porta: o hello é outro). Roda como "messenger-jp", junto do "game-jp".</summary>
    public MessengerConfig? MessengerJp { get; set; }
    public RankingConfig Ranking { get; set; } = new();
    /// <summary>Ranking do cliente JP 983 (outra porta: hello e ids são outros). Roda como "ranking-jp", junto do "game-jp".</summary>
    public RankingConfig? RankingJp { get; set; }

    static readonly JsonSerializerOptions Json = new()
    {
        PropertyNameCaseInsensitive = true,
        ReadCommentHandling = JsonCommentHandling.Skip,
        AllowTrailingCommas = true,
        UnmappedMemberHandling = JsonUnmappedMemberHandling.Disallow,
        Converters = { new JsonStringEnumConverter() },
    };

    /// <summary>Caminho padrão: variável PANGYA_CONFIG ou config/pangya.json.</summary>
    public static string DefaultPath => Environment.GetEnvironmentVariable("PANGYA_CONFIG") ?? "config/pangya.json";

    public static PangyaConfig Load(string? path = null)
    {
        path ??= DefaultPath;
        if (!File.Exists(path))
            throw new FileNotFoundException($"Arquivo de configuração não encontrado: {Path.GetFullPath(path)} (copie config/pangya.example.json)");
        return Parse(File.ReadAllText(path));
    }

    public static PangyaConfig Parse(string json)
    {
        var cfg = JsonSerializer.Deserialize<PangyaConfig>(json, Json) ?? throw new InvalidDataException("Configuração vazia");
        cfg.Validate();
        return cfg;
    }

    void Validate()
    {
        if (string.IsNullOrWhiteSpace(Database.ConnectionString))
            throw new InvalidDataException("Database.ConnectionString é obrigatório");
        Limits.Validate();
    }
}

public sealed class NetworkConfig
{
    /// <summary>IP onde os servidores escutam.</summary>
    public string BindIp { get; set; } = "0.0.0.0";
    /// <summary>IP anunciado ao cliente (lista de servidores).</summary>
    public string PublicIp { get; set; } = "127.0.0.1";
}

/// <summary>Servidor HTTP: login do cliente (LoginForGame.aspx) e página de cadastro.</summary>
public sealed class WebConfig
{
    public int Port { get; set; } = 80;
    /// <summary>Login com conta inexistente cria a conta. SÓ PARA TESTES: nunca ligue num servidor aberto.</summary>
    public bool AutoRegister { get; set; }
    /// <summary>Painel de administração em /admin (só contas GM). Desligado por padrão.</summary>
    public bool AdminEnabled { get; set; }
    /// <summary>IPs que podem abrir o painel (padrão: só a própria máquina).</summary>
    public string[] AdminAllowedIps { get; set; } = ["127.0.0.1", "::1"];
}

public sealed class LoginConfig
{
    /// <summary>O cliente 645 conecta na 10101 (fixo no exe); o 642 live sorteia 10101/10102.</summary>
    public int[] Ports { get; set; } = [10101];
    /// <summary>Identificador mandado no hello.</summary>
    public uint ServerUid { get; set; } = 10101;
}

/// <summary>Um game server (lobby, canais, salas...). Vários processos podem rodar, cada um com seu Id e porta.</summary>
/// <summary>Drop por buraco (JP): itens com chance em ‰ ao acertar o buraco (LastHoleOnly = só no último).</summary>
public sealed class HoleDropsConfig
{
    public bool Enabled { get; set; } = true;
    public int ManaPerMille { get; set; } = 15;
    public int SpecialShufflePerMille { get; set; } = 20;
    public int GrandPrixTicketMax { get; set; } = 50;
    /// <summary>
    /// Itens do copo; MinStars = só em mapas com essa dificuldade ou mais (estrelas do Course.iff, 1..5): mapas difíceis
    /// somam itens melhores aos comuns.
    /// </summary>
    public List<HoleDropItem> Items { get; set; } =
    [
        // todos os mapas
        new() { TypeId = 0x1A0000AC, PerMille = 30 },                       // Copper Medal (receitas da Caixa Mágica)
        new() { TypeId = 0x18000000, PerMille = 15 },                       // Spin Mastery
        new() { TypeId = 0x18000001, PerMille = 15 },                       // Curve Mastery
        new() { TypeId = 0x18000004, PerMille = 15 },                       // Strength Boost
        new() { TypeId = 0x18000006, PerMille = 10 },                       // Silent Wind
        new() { TypeId = 0x1A000011, PerMille = 15 },                       // Time Booster
        // 3 estrelas ou mais
        new() { TypeId = 0x1A0000AB, PerMille = 20, MinStars = 3 },         // Silver coin
        new() { TypeId = 0x18000025, PerMille = 12, MinStars = 3 },         // Power Milk
        new() { TypeId = 0x1A000040, PerMille = 12, MinStars = 3 },         // Auto Caliper
        new() { TypeId = 0x18000028, PerMille = 10, MinStars = 3 },         // Safety
        // 4 estrelas ou mais
        new() { TypeId = 0x1A0000AA, PerMille = 12, MinStars = 4 },         // Gold Medal
        new() { TypeId = 0x1A00020F, PerMille = 4, MinStars = 4 },          // Club Modification Kit
        new() { TypeId = 0x1A000208, PerMille = 4, MinStars = 4 },          // Papel Box
        new() { TypeId = 0x1A000273, PerMille = 4, MinStars = 4 },          // Memorial Coin (Normal)
        new() { TypeId = 0x1A000004, PerMille = 3, MinStars = 4 },          // Lucky Bag of Fortune
        // último buraco: medalha de ouro (mais chance nos mapas difíceis)
        new() { TypeId = 0x1A0000AA, PerMille = 100, LastHoleOnly = true },
        new() { TypeId = 0x1A0000AA, PerMille = 150, LastHoleOnly = true, MinStars = 4 },
    ];
}

/// <summary>Prêmio de login por evento: a cada Days dias com login, o item por carta (NTimes: Times vezes; Forever: sempre).</summary>
public sealed class LoginRewardEvent
{
    /// <summary>Chave do progresso no banco: não reaproveitar.</summary>
    public int Id { get; set; }
    public string Name { get; set; } = "";
    public int Days { get; set; } = 7;
    public bool Forever { get; set; }
    public int Times { get; set; } = 1;
    public int TypeId { get; set; }
    public int Quantity { get; set; } = 1;
    public DateOnly? Start { get; set; }
    public DateOnly? End { get; set; }
}

/// <summary>Golden Time: horários das rodadas (relógio do servidor), ganhadores a cada 100 concorrentes e os itens (peso).</summary>
public sealed class GoldenTimeConfig
{
    public bool Enabled { get; set; }
    public List<string> Rounds { get; set; } = ["20:00", "20:30", "21:00", "21:30"];
    public double WinnersPer100 { get; set; } = 1;
    public List<GoldenTimeItem> Items { get; set; } =
    [
        new() { TypeId = 0x1A000011, Quantity = 10, Weight = 30 },          // Time Booster
        new() { TypeId = 0x1A000105, Quantity = 3, Weight = 20 },           // Comet Refill Pack
        new() { TypeId = 0x1A000208, Quantity = 1, Weight = 15 },           // Papel Box
        new() { TypeId = 0x1A000273, Quantity = 1, Weight = 15 },           // Memorial Coin (Normal)
        new() { TypeId = 0x1A00015B, Quantity = 3, Weight = 10 },           // Spinning Cube
        new() { TypeId = 0x1A000080, Quantity = 1, Weight = 7 },            // Gacha Ticket
        new() { TypeId = 0x1A000272, Quantity = 1, Weight = 3 },            // Memorial Coin (Premium)
    ];
}

/// <summary>Bot GM Event: horários de abertura (relógio do servidor), quantos prêmios (mín..máx) e a tabela (peso).</summary>
public sealed class BotGmEventConfig
{
    public bool Enabled { get; set; }
    public List<string> Windows { get; set; } = ["21:00"];
    public int MinPrizes { get; set; } = 1;
    public int MaxPrizes { get; set; } = 2;
    public List<BotGmPrize> Prizes { get; set; } =
    [
        new() { TypeId = 0x1A000273, Quantity = 1, Weight = 30 },           // Memorial Coin (Normal)
        new() { TypeId = 0x1A000208, Quantity = 1, Weight = 25 },           // Papel Box
        new() { TypeId = 0x1A00015C, Quantity = 5, Weight = 20 },           // Key of Fortune
        new() { TypeId = 0x1A000080, Quantity = 1, Weight = 15 },           // Gacha Ticket
        new() { TypeId = 0x1A000272, Quantity = 1, Weight = 5 },            // Memorial Coin (Premium)
    ];
}

public sealed class BotGmPrize
{
    public int TypeId { get; set; }
    public int Quantity { get; set; } = 1;
    public int Weight { get; set; } = 1;
}

/// <summary>Gacha do JP (site /on_client/gacha): os prêmios e o peso de cada um.</summary>
public sealed class GachaConfig
{
    public List<GachaItem> Items { get; set; } =
    [
        new() { TypeId = 0x18000000, Quantity = 5, Weight = 10 },           // Spin Mastery
        new() { TypeId = 0x18000001, Quantity = 5, Weight = 10 },           // Curve Mastery
        new() { TypeId = 0x18000004, Quantity = 5, Weight = 10 },           // Strength Boost
        new() { TypeId = 0x18000005, Quantity = 5, Weight = 10 },           // Miracle Sign
        new() { TypeId = 0x1A000011, Quantity = 5, Weight = 10 },           // Time Booster
        new() { TypeId = 0x18000025, Quantity = 5, Weight = 10 },           // Power Milk
        new() { TypeId = 0x1A00000F, Quantity = 3, Weight = 10 },           // Angel Candy
        new() { TypeId = 0x1A00015B, Quantity = 1, Weight = 4 },            // Spinning Cube
        new() { TypeId = 0x1A00015C, Quantity = 3, Weight = 4 },            // Key of Fortune
        new() { TypeId = 0x1A000105, Quantity = 1, Weight = 4 },            // Comet Refill Pack
        new() { TypeId = 0x1A000208, Quantity = 1, Weight = 4 },            // Papel Box
        new() { TypeId = 0x1A000273, Quantity = 1, Weight = 4 },            // Memorial Coin (Normal)
        new() { TypeId = 0x14000020, Quantity = 30, Weight = 4 },           // Rainbow Comet
        new() { TypeId = 0x1A000272, Quantity = 1, Weight = 3 },            // Memorial Coin (Premium)
        new() { TypeId = 0x40000004, Quantity = 1, Days = 30, Weight = 2 }, // El Monito (30 dias)
        new() { TypeId = 0x10000012, Quantity = 1, Weight = 1 },            // Lucky Air Knight Set
    ];
}

/// <summary>World Tour (JP): ligado, datas (opcionais) e os itens do prêmio dos 16 campos (vão por carta).</summary>
public sealed class WorldTourConfig
{
    public bool Enabled { get; set; } = true;
    public DateOnly? Start { get; set; }
    public DateOnly? End { get; set; }
    public List<GachaItem> Prizes { get; set; } =
    [
        new() { TypeId = 0x1A000272, Quantity = 1 },                        // Memorial Coin (Premium)
        new() { TypeId = 0x1A000080, Quantity = 3 },                        // Gacha Ticket
        new() { TypeId = 0x1A00015B, Quantity = 5 },                        // Spinning Cube
    ];
}

public sealed class GachaItem
{
    public int TypeId { get; set; }
    public int Quantity { get; set; } = 1;
    public int Days { get; set; }
    public int Weight { get; set; } = 1;
}

public sealed class GoldenTimeItem
{
    public int TypeId { get; set; }
    public int Quantity { get; set; } = 1;
    public int Days { get; set; }
    public int Weight { get; set; } = 1;
}

public sealed class HoleDropItem
{
    public int TypeId { get; set; }
    public int PerMille { get; set; }
    public int Quantity { get; set; } = 1;
    public bool LastHoleOnly { get; set; }
    /// <summary>Dificuldade mínima do mapa (estrelas 1..5 do Course.iff); 0 = todos.</summary>
    public int MinStars { get; set; }
}

public sealed class GameConfig
{
    /// <summary>Game server JP: eventos de prêmio de login (padrão: 7 dias = Memorial Coin (Premium), uma vez).</summary>
    public List<LoginRewardEvent> LoginRewards { get; set; } =
        [new() { Id = 1, Name = "Boas-vindas", Days = 7, Times = 1, TypeId = 0x1A000272, Quantity = 1 }];
    /// <summary>Game server JP: Bot GM Event (desligado por padrão; o GM abre com "!botgm").</summary>
    public BotGmEventConfig BotGmEvent { get; set; } = new();
    /// <summary>Game server JP: Golden Time (desligado por padrão; o GM sorteia uma rodada com "!goldentime").</summary>
    public GoldenTimeConfig GoldenTime { get; set; } = new();
    /// <summary>Game server JP: World Tour (16 campos completados = prêmio).</summary>
    public WorldTourConfig WorldTour { get; set; } = new();
    /// <summary>Game server JP: drop por buraco.</summary>
    public HoleDropsConfig HoleDrops { get; set; } = new();
    /// <summary>Game server JP: horários do evento do Grand Zodiac ("HH:MM int|adv", relógio do servidor).</summary>
    public string[] ZodiacSchedule { get; set; } = [];
    /// <summary>Id na lista de servidores (único entre os game servers).</summary>
    public int Id { get; set; } = 20201;
    public string Name { get; set; } = "PangYa";
    public int Port { get; set; } = 20201;
    public int MaxUsers { get; set; } = 1000;
    public ChannelConfig[] Channels { get; set; } = [new()];
    /// <summary>Espera do bot antes de tacar (segundos).</summary>
    public double BotDelaySeconds { get; set; } = 4;
    /// <summary>
    /// Game server KR: comprime (LZO) os pacotes grandes. O cliente 645 aceita e precisa disso com inventário grande;
    /// false só para clientes de teste que não descomprimem (o cliente falso do emulador Python).
    /// </summary>
    public bool CompressPackets { get; set; } = true;
    /// <summary>Se nem todos mandarem "pronto para o tee", o servidor libera depois deste tempo (segundos).</summary>
    public double TeeFallbackSeconds { get; set; } = 15;
    /// <summary>
    /// Stroke (VS): buracos terminados para o jogador que ficou sozinho (os adversários saíram) poder continuar; o cliente
    /// pergunta. Antes disso a partida acaba ali.
    /// </summary>
    public int VersusSoloAfterHoles { get; set; } = 9;
    /// <summary>JP: chance de chuva em cada buraco (‰); os artefatos de chuva somam +50/+100/+200.</summary>
    public int RainPerMille { get; set; } = 150;
    /// <summary>true = o bot só passa a vez (estouro de tempo) em vez de tacar.</summary>
    public bool BotPasses { get; set; }
    /// <summary>Mapas permitidos (número do curso). Vazio = os ativos no Course.iff, como o cliente mostra.
    /// Ex.: liberar o Wiz City (19) quando o pak dele estiver instalado no cliente.</summary>
    public int[] Courses { get; set; } = [];
    /// <summary>Precisão do bot, 0..1 (1 = sem erro aleatório de mira/força).</summary>
    public float BotAccuracy { get; set; } = 0.85f;
    /// <summary>
    /// Velocidade da bola do bot no VS, como o Time Booster (S->C 0xC5 para todos; o item do cliente usa 3). 0 ou 1 = normal.
    /// </summary>
    public float BotFastForward { get; set; } = 2;
    /// <summary>Espera depois da tacada do bot até acelerar (a bola já em voo; antes disso o cliente volta para 1×).</summary>
    public double BotFastForwardDelaySeconds { get; set; } = 1.5;
    public RewardConfig Rewards { get; set; } = new();
    /// <summary>
    /// Propriedade do servidor (flags) que o cliente JP lê na lista do login e no 0x44: 0x800 (2048) = Grand Prix (o
    /// Start Game oferece "Grand Prix / jogo normal"). O KR não usa.
    /// </summary>
    public uint Property { get; set; }
    /// <summary>
    /// Eventos do servidor (só JP; vai na lista do login e na do lobby). Só visual: nenhum bit muda taxa. Na linha do servidor:
    /// 0x10 icon_expevent (x=113), 0x04 icon_expevent translúcido (x=128), 0x02 icon_pangevent (x=143), 0x08 icon_angelwing e
    /// 0x80 icon_masteryevent (ambos x=158). 0x20: icon_pangevent nas salas dos mapas de <see cref="EventValue"/>. 0x40: servidor
    /// de sala de conversa (só troca o texto dos avisos de controle do servidor).
    /// </summary>
    public uint EventFlags { get; set; }
    /// <summary>Máscara de mapas do bit 0x20 de <see cref="EventFlags"/>: bit n = mapa n; 4294967295 = todos.</summary>
    public uint EventValue { get; set; }
    /// <summary>Mascote da linha do servidor (só JP; m_serverIconMap do cliente: 0 list_bongdal, 1 list_pipin, 2 list_titangboo...).</summary>
    public ushort IconIndex { get; set; }
    public TreasureHunterConfig TreasureHunter { get; set; } = new();
}

/// <summary>
/// Treasure Hunter (SPEC-treasure-hunter.md): barra de pontos da partida (stroke/team) e caixas de prêmio no fim.
/// Prêmios só de itens 0x18..0x1B (o cliente soma no inventário dele); 0x1A000010 = pang.
/// </summary>
public sealed class TreasureHunterConfig
{
    public bool Enabled { get; set; } = true;
    /// <summary>Taxa de caixas em % (100 = a tabela do GB).</summary>
    public int RatePercent { get; set; } = 100;
    public TreasurePrize[] Prizes { get; set; } =
    [
        new() { TypeId = 0x1A000010, Min = 50, Max = 300, Weight = 40 },     // pang
        new() { TypeId = 0x18000004, Min = 1, Max = 2, Weight = 10 },       // 체력 보조제 (power shot)
        new() { TypeId = 0x18000006, Min = 1, Max = 1, Weight = 8 },        // 사일런트 윈드
        new() { TypeId = 0x1A000011, Min = 1, Max = 3, Weight = 10 },       // 타임부스터
        new() { TypeId = 0x1A000040, Min = 1, Max = 2, Weight = 8 },        // 오토 캘리퍼스
        new() { TypeId = 0x18000002, Min = 1, Max = 1, Weight = 6 },        // 럭키 팡야
        new() { TypeId = 0x18000025, Min = 1, Max = 1, Weight = 5 },        // 체력 보충제 (gauge +33)
        new() { TypeId = 0x18000027, Min = 1, Max = 1, Weight = 3 },        // 체력 강화제 (power shot +15)
    ];
}

public sealed class TreasurePrize
{
    public int TypeId { get; set; }
    public int Min { get; set; } = 1;
    public int Max { get; set; } = 1;
    public int Weight { get; set; } = 1;
}

/// <summary>
/// Mensageiro (amigos, online, conversa; docs/server/protocolo/SPEC-messenger.md). Roda no mesmo processo do game server, que
/// confirma quem está logado. O cliente descarta servidores com usuários ≥ máximo − 150: MaxUsers alto.
/// </summary>
public sealed class MessengerConfig
{
    public int Id { get; set; } = 30303;
    public string Name { get; set; } = "Messenger";
    public int Port { get; set; } = 30303;
    public int MaxUsers { get; set; } = 3000;
    public int MaxFriends { get; set; } = 50;
}

/// <summary>
/// Ranking (docs/server/protocolo/SPEC-ranking.md). Roda no processo do game (que manda o endereço no 0xA0 e confirma quem
/// pede). O cliente só aceita IP numérico (Network.PublicIp). Retrato recalculado ao subir e todo dia na hora RefreshHour.
/// </summary>
public sealed class RankingConfig
{
    public int Id { get; set; } = 30474;
    public string Name { get; set; } = "Ranking";
    public int Port { get; set; } = 30474;
    /// <summary>Hora local (0..23) do recálculo diário.</summary>
    public int RefreshHour { get; set; } = 5;
}

/// <summary>Recompensa de fim de partida (o pang informado pelo cliente é limitado por buraco).</summary>
public sealed class RewardConfig
{
    /// <summary>Taxa de EXP do servidor em % (100 = normal), como a taxa do servidor GB.</summary>
    public int ExpRate { get; set; } = 100;
    public int MaxPangPerHole { get; set; } = 1000;
    /// <summary>
    /// Bots contam como jogadores para os troféus do torneio (que só saem com 10+ jogadores). false = fiel ao original;
    /// true serve para testar sozinho com bots.
    /// </summary>
    public bool TrophiesCountBots { get; set; }
    /// <summary>Taxa da maestria dos tacos (oficina do JP) em % (100 = normal).</summary>
    public int ClubMasteryRate { get; set; } = 100;
    /// <summary>Recarga de cometa (pacote 0x1A000105): bolas somadas, de Min a Max (o cliente exige ao menos 10).</summary>
    public int CometRefillMin { get; set; } = 30;
    public int CometRefillMax { get; set; } = 50;
}

public sealed class ChannelConfig
{
    public string Name { get; set; } = "Canal 1";
    public int MaxUsers { get; set; } = 100;
}

/// <summary>O que um jogador novo recebe (typeids do pangya.iff).</summary>
public sealed class NewPlayerConfig
{
    public long Pang { get; set; } = 100_000;
    public long Cookie { get; set; }
    /// <summary>Air Knight: o club set que o tutorial dá (golfruletutorial.cpp:125).</summary>
    public int ClubSet { get; set; } = 0x10000000;
    /// <summary>Pangya Aztec: a bola básica.</summary>
    public int Ball { get; set; } = 0x14000000;
    public int BallCount { get; set; } = 100;
}

public sealed class DatabaseConfig
{
    public string ConnectionString { get; set; } = "";
}

public sealed class LoggingConfig
{
    /// <summary>Debug, Info, Warn ou Error. Debug mostra cada pacote.</summary>
    public Logging.LogLevel Level { get; set; } = Logging.LogLevel.Info;
    /// <summary>Pasta dos arquivos de log (vazio = só console).</summary>
    public string Dir { get; set; } = "logs";
    /// <summary>Grava também o conteúdo de cada pacote em hexa, decifrado, num arquivo por conexão em &lt;Dir&gt;/pacotes.</summary>
    public bool PacketHex { get; set; }
}

public sealed class DataConfig
{
    /// <summary>Dados do jogo do cliente (ZIP de tabelas .iff).</summary>
    public string IffPath { get; set; } = "data/pangya.iff";
    /// <summary>Dados do cliente JP 983 (pangya_jp.iff do pak mais novo); sem o arquivo o game JP não filtra itens.</summary>
    public string JpIffPath { get; set; } = "data/pangya_jp.iff";
}

/// <summary>Limites contra abuso.</summary>
public sealed class LimitsConfig
{
    public int MaxConnectionsPerIp { get; set; } = 10;
    public int MaxPacketsPerSecond { get; set; } = 100;
    public int MaxPacketSize { get; set; } = 8192;
    public int MaxLoginAttemptsPerMinute { get; set; } = 10;
    /// <summary>Inatividade antes do login (conexões que não se identificam).</summary>
    public int IdleTimeoutSeconds { get; set; } = 120;
    /// <summary>Inatividade depois do login no game server (jogador parado no lobby). A conexão de login autenticada não expira.</summary>
    public int SessionIdleTimeoutSeconds { get; set; } = 900;

    internal void Validate()
    {
        if (MaxConnectionsPerIp < 1 || MaxPacketsPerSecond < 1 || MaxPacketSize < 16 || MaxLoginAttemptsPerMinute < 1 || IdleTimeoutSeconds < 5)
            throw new InvalidDataException("Limits: valores inválidos");
    }
}

/// <summary>
/// Papel Shop (봉다리) e raspadinha: o cliente não tem tabela de prêmios, então tudo vem daqui
/// (docs/server/protocolo/SPEC-papel-raspadinha.md §1.5 e §2.6). Pools padrão = silhuetas do cliente KR.
/// </summary>
public sealed class LotteryConfig
{
    /// <summary>Preço de uma jogada do Papel Shop sem cupom.</summary>
    public long PapelPrice { get; set; } = 900;
    /// <summary>Peso de sair 1, 2, 3, 4 e 5 bolas.</summary>
    public int[] PapelBallWeights { get; set; } = [35, 30, 20, 10, 5];
    /// <summary>Chance (em 1000) de cada bola ser de cookie e rara.</summary>
    public int PapelCookiePerMille { get; set; } = 140;
    public int PapelRarePerMille { get; set; } = 10;
    /// <summary>
    /// Big Papel (só no cliente JP 983): sempre BigPapelBalls bolas, só em pang (sem cupom), mesmos prêmios do Papel. O
    /// preço não está no cliente (a referência lê do banco): padrão 10 jogadas.
    /// </summary>
    public long BigPapelPrice { get; set; } = 9000;
    public int BigPapelBalls { get; set; } = 10;
    /// <summary>Peso de a raspadinha dar 0, 1 e 2 itens.</summary>
    public int[] ScratchCountWeights { get; set; } = [30, 69, 1];
    public int ScratchCookiePerMille { get; set; } = 180;
    public int ScratchRarePerMille { get; set; } = 20;

    public PrizeConfig[] Normal { get; set; } =
    [
        new(0x18000008, 11), new(0x18000007, 11), new(0x18000001, 11), new(0x18000000, 13), new(0x18000004, 12),
        new(0x18000005, 11), new(0x1A000028, 12, 1, 1), new(0x1A00003D, 9, 1, 1), new(0x1A000041, 10, 1, 1),
    ];
    public PrizeConfig[] Cookie { get; set; } =
    [
        new(0x1800000E, 7), new(0x1800000B, 7), new(0x1800000A, 7), new(0x18000009, 7), new(0x18000006, 6),
        new(0x1A00004F, 7), new(0x1A000002, 5), new(0x1A000011, 5), new(0x14000005, 5), new(0x14000003, 5),
        new(0x14000002, 5), new(0x14000001, 5), new(0x14000020, 4), new(0x18000010, 5), new(0x18000011, 5),
        new(0x18000012, 5), new(0x1A000040, 4), new(0x18000028, 3), new(0x18000027, 3),
    ];
    /// <summary>Raros (peças, club sets...): vazio = a chance de raro vira item de cookie.</summary>
    public PrizeConfig[] Rare { get; set; } = [];
}

/// <summary>
/// Memorial Shop (cliente JP 983): chance de raro por moeda e prêmios de consolação. O cliente traz moedas e raros
/// (MemorialShopCoinItem/RareItem), mas não a tabela de consolação (a referência lê do banco): ela vem daqui.
/// </summary>
public sealed class MemorialConfig
{
    /// <summary>Chance (%) de sair um raro: moedas comuns e moedas com bônus (Premium).</summary>
    public int RarePercent { get; set; } = 20;
    public int PremiumRarePercent { get; set; } = 55;
    /// <summary>Consolação das moedas comuns, especiais e de personagem.</summary>
    public PrizeConfig[] Normal { get; set; } =
    [
        new(0x18000000, 13), new(0x18000001, 11), new(0x18000004, 12), new(0x18000005, 11), new(0x18000007, 11),
        new(0x18000008, 11), new(0x1A000028, 10, 1, 2), new(0x1A00003D, 9, 1, 2), new(0x1A000041, 10, 1, 2),
    ];
    /// <summary>Consolação das moedas com bônus (Premium); vazio = a mesma das comuns.</summary>
    public PrizeConfig[] Premium { get; set; } =
    [
        new(0x1800000E, 7, 2, 5), new(0x1800000B, 7, 2, 5), new(0x1800000A, 7, 2, 5), new(0x18000009, 7, 2, 5),
        new(0x1A000011, 5, 1, 3), new(0x1A000040, 4, 1, 2), new(0x18000028, 3, 1, 2), new(0x18000027, 3, 1, 2),
    ];
}

/// <summary>
/// Presença diária (cliente JP 983): prêmio de cada dia e, a cada EveryNth dias, da lista especial. O cliente não traz a
/// tabela (a referência lê do banco).
/// </summary>
public sealed class AttendanceConfig
{
    public int EveryNth { get; set; } = 10;
    /// <summary>
    /// Itens que acompanham todo login do dia, até o teto no inventário (a referência: tickets do Grand Prix, que não
    /// vendem na loja). Min = quantidade por dia, Max = teto.
    /// </summary>
    public PrizeConfig[] DailyTickets { get; set; } = [new(0x1A000264, 1, 3, 50)];
    public PrizeConfig[] Daily { get; set; } =
    [
        new(0x1A000010, 20, 300, 1000), new(0x18000000, 10, 1, 3), new(0x18000001, 10, 1, 3), new(0x18000004, 10, 1, 3),
        new(0x18000005, 10, 1, 3), new(0x1A000011, 8, 1, 2), new(0x1A000028, 8, 1, 2), new(0x18000008, 10, 1, 3),
    ];
    /// <summary>A cada EveryNth dias: Papel Box, Memorial Coin, mais pang.</summary>
    public PrizeConfig[] Special { get; set; } =
    [
        new(0x1A000208, 10, 1, 1), new(0x1A000273, 10, 1, 1), new(0x1A000010, 10, 3000, 5000),
    ];
}

/// <summary>
/// Caixas do My Room (Lucky Pouch, Pangya Box, aniversários...; o JP 983 abre 230 delas): o cliente não traz o que vem
/// dentro (o oficial tinha no banco). Default vale para toda caixa sem lista própria em Boxes. 0x1A000010 = pang.
/// </summary>
public sealed class BoxConfig
{
    public PrizeConfig[] Default { get; set; } =
    [
        new(0x1A000010, 40, 500, 3000),                                     // pang
        new(0x18000004, 10, 1, 3), new(0x18000006, 8, 1, 2), new(0x1A000011, 10, 1, 3), new(0x1A000040, 8, 1, 2),
        new(0x18000002, 6, 1, 1), new(0x18000025, 5, 1, 2), new(0x1A000208, 3, 1, 1), new(0x1A000273, 2, 1, 1),
    ];
    public BoxContents[] Boxes { get; set; } = [];
}

/// <summary>Conteúdo de uma caixa (typeid da caixa e os prêmios possíveis).</summary>
public sealed class BoxContents
{
    public int TypeId { get; set; }
    public PrizeConfig[] Prizes { get; set; } = [];
}

/// <summary>Um prêmio possível: typeid, peso e quantidade (sorteada entre Min e Max; itens não empilháveis = 1).</summary>
public sealed class PrizeConfig
{
    public PrizeConfig() { }
    public PrizeConfig(int typeId, int weight, int min = 1, int max = 3) { TypeId = typeId; Weight = weight; Min = min; Max = max; }
    public int TypeId { get; set; }
    public int Weight { get; set; } = 1;
    public int Min { get; set; } = 1;
    public int Max { get; set; } = 3;
}
