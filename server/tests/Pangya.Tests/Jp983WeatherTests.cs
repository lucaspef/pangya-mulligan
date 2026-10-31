using Pangya.Protocol.JP983.Game;

namespace Pangya.Tests;

/// <summary>Clima por buraco no JP 983: nublado só antes de chuva, como no oficial.</summary>
public class Jp983WeatherTests
{
    [Theory]
    [InlineData(new byte[] { 0, 2, 2, 0, 0, 2 }, new byte[] { 1, 2, 2, 0, 1, 2 })]
    [InlineData(new byte[] { 2, 0, 0, 0 }, new byte[] { 2, 0, 0, 0 })]
    [InlineData(new byte[] { 0, 0, 0, 0 }, new byte[] { 0, 0, 0, 0 })]
    public void CloudOnlyPrecedesRain(byte[] weather, byte[] expected)
    {
        JpTranslator.CloudBeforeRain(weather, weather.Length);
        Assert.Equal(expected, weather);
    }
}
