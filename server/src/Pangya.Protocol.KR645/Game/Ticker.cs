using Pangya.Core.Logging;
using Pangya.Core.Net;

namespace Pangya.Protocol.KR645.Game;

/// <summary>
/// Letreiro (ticker; docs/server/protocolo/SPEC-jp983-efeitos.md §7): uma fila do mundo; cada mensagem passa no topo da tela de
/// todos os conectados (0xC7 str nick, str mensagem; JP 0xC9) e ocupa ~30 s. O pedido (só o JP manda) custa cookies.
/// </summary>
public sealed class TickerBoard(Domain.Game.GameWorld world)
{
    public static readonly TimeSpan Slot = TimeSpan.FromSeconds(30);
    public const int MaxLength = 100;
    const ushort STicker = 0xC7;

    readonly Queue<(string Nick, string Text)> queue = new();
    bool running;

    /// <summary>Mensagens esperando (sem a que está passando).</summary>
    public int Count { get { lock (queue) return queue.Count; } }

    public void Enqueue(string nick, string text)
    {
        lock (queue)
        {
            queue.Enqueue((nick, text.Length > MaxLength ? text[..MaxLength] : text));
            if (running) return;
            running = true;
        }
        _ = RunAsync();
    }

    async Task RunAsync()
    {
        try
        {
            await Task.Delay(TimeSpan.FromSeconds(3));                          // referência: começa 3 s depois
            while (true)
            {
                (string Nick, string Text) m;
                lock (queue)
                {
                    if (queue.Count == 0) { running = false; return; }
                    m = queue.Dequeue();
                }
                foreach (var s in world.Online)
                    if (s is GameHandler h) h.Connection.Send(new PacketWriter(STicker).Str(m.Nick).Str(m.Text));
                Log.Info($"letreiro: {m.Nick}: {m.Text}");
                await Task.Delay(Slot);
            }
        }
        catch (Exception e)
        {
            lock (queue) running = false;
            Log.Error("letreiro: falha", e);
        }
    }
}
