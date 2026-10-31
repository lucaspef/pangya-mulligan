namespace Pangya.Domain.Players;

/// <summary>
/// Nível do caddie: "level" 0..3 (a janela mostra 1..4) e "exp" do nível atual. No fim da partida o caddie equipado ganha a
/// mesma EXP do jogador; para subir são 520, 680 e 840 (s_bonusExp, shareddoc.cpp:616) e a sobra passa para o próximo. No
/// máximo não ganha mais (a janela só mostra a barra abaixo do 4). O cliente lê nível e EXP da lista de caddies (0x6F) do
/// login e não atualiza um caddie que já conhece, então o nível novo aparece ao entrar de novo.
/// </summary>
public static class CaddieLevels
{
    public const int Max = 3;

    public static int Need(int level) => 520 + 160 * level;

    /// <summary>Cópia do caddie com a EXP somada (e os níveis que subiu); null = nada muda.</summary>
    public static Item? Gain(Item caddie, int exp)
    {
        int level = Math.Clamp(caddie.Int("level"), 0, Max), cur = Math.Max(caddie.Int("exp"), 0);
        if (exp <= 0 || level >= Max) return null;
        cur += exp;
        while (level < Max && cur >= Need(level)) { cur -= Need(level); level++; }
        if (level >= Max) cur = 0;
        var edit = caddie.Clone();
        edit.Set("level", level);
        edit.Set("exp", cur);
        return edit;
    }
}
