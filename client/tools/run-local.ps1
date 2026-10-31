<#
.SYNOPSIS
    Sobe o PangYa Mulligan localmente: confere a pasta do jogo, instala o exe compilado, gera o updatelist,
    sobe o servidor no WSL e abre o jogo elevado com as variáveis JP983_*.

.DESCRIPTION
    Passos completos em client/COMPILAR.md, seção "Rodar". Vem da sua cópia do jogo: os *.pak, as DLLs e, se existir,
    Translation/Read.aspx. Nada disso entra no git: o updatelist gerado fica em server/data/gameweb/, ignorado.

    O exe exige administrador, e a elevação do UAC descarta as variáveis de um prompt comum. Por isso o script se
    reabre elevado e define JP983_* nesse processo, que abre o jogo. O registro também é gravado só nele.

.PARAMETER GameDir       Pasta do jogo, com os ProjectG*.pak.
.PARAMETER BuildDir      Saída do build. Padrão: client\build\vs2022\Release.
.PARAMETER Login         JP983_LOGIN, host:porta. Vazio usa o padrão do exe, 127.0.0.1:30101.
.PARAMETER Web           JP983_WEB. Vazio usa o padrão do exe, http://localhost:30091. Use localhost, não 127.0.0.1.
.PARAMETER Www           JP983_WWW. Vazio usa o padrão do exe, http://localhost:30080.
.PARAMETER NoCopy        Não copia ProjectG.exe e wangreal.dll para a pasta do jogo.
.PARAMETER NoServer      Não sobe o servidor: use quando ele já está no ar ou roda em outra máquina.
.PARAMETER NoGame        Faz tudo, menos abrir o jogo.
.PARAMETER Launch        Interno: etapa elevada, que grava o registro e abre o jogo.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File client\tools\run-local.ps1 -GameDir "D:\Jogos\PangYa"
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$GameDir,
    [string]$BuildDir,
    [string]$Login,
    [string]$Web,
    [string]$Www,
    [switch]$NoCopy,
    [switch]$NoServer,
    [switch]$NoGame,
    [switch]$Launch
)
$ErrorActionPreference = 'Stop'
$GameDir = (Resolve-Path -LiteralPath $GameDir).Path.TrimEnd('\')
$repo = (Resolve-Path "$PSScriptRoot\..\..").Path
if (-not $BuildDir) { $BuildDir = Join-Path $repo 'client\build\vs2022\Release' }
$regKey = 'HKLM:\SOFTWARE\WOW6432Node\Ntreev\Pangya_Jp'         # o jogo é 32 bits: lê Software\Ntreev\Pangya_Jp nesta visão
$launchLog = Join-Path $env:TEMP 'pangya-run-local.log'

function Get-BasePak {
    $paks = @(Get-ChildItem -LiteralPath $GameDir -Filter '*+.pak' | Where-Object { $_.Name -like 'ProjectG*' })
    if ($paks.Count -ne 1) { throw "Esperava um ProjectG*+.pak em $GameDir; achei $($paks.Count). Esta pasta tem o cliente JP completo?" }
    $paks[0].Name
}

# Etapa elevada: registro, variáveis e jogo
if ($Launch) {
    try {
        $pak = Get-BasePak
        $current = (Get-ItemProperty -Path $regKey -Name IntegratedPak -ErrorAction SilentlyContinue).IntegratedPak
        if (-not $current) {
            New-Item -Path $regKey -Force | Out-Null
            New-ItemProperty -Path $regKey -Name IntegratedPak -Value $pak -PropertyType String -Force | Out-Null
            Write-Host "[run-local] registro: IntegratedPak=$pak"
        }
        elseif ($current -ne $pak) { Write-Warning "IntegratedPak é '$current', mas a pasta tem '$pak'. Não alterei." }
        if ($Login) { $env:JP983_LOGIN = $Login }
        if ($Web) { $env:JP983_WEB = $Web }
        if ($Www) { $env:JP983_WWW = $Www }
        $game = Start-Process -FilePath (Join-Path $GameDir 'ProjectG.exe') -WorkingDirectory $GameDir -PassThru
        "ProjectG.exe pid $($game.Id)" | Set-Content -LiteralPath $launchLog
    }
    catch {
        "$_" | Set-Content -LiteralPath $launchLog
        exit 1
    }
    exit 0
}

# 1. Pasta do jogo e dependências
$pak = Get-BasePak
foreach ($dll in 'mss32.dll', 'ijl15.dll') {
    if (-not (Test-Path -LiteralPath (Join-Path $GameDir $dll))) { throw "Falta $dll em $GameDir. Ela vem da sua cópia do jogo." }
}
if (-not (Test-Path "$env:WINDIR\SysWOW64\d3dx9_43.dll")) {
    throw 'Falta d3dx9_43.dll de 32 bits. Instale o DirectX End-User Runtimes de junho de 2010 (client/COMPILAR.md).'
}
Write-Host "[run-local] pasta do jogo ok, pak base $pak"

# 2. Exe compilado; o que estava na pasta fica em .antes-mulligan, uma vez só
$updlist = Join-Path $BuildDir 'updlist.exe'
if (-not (Test-Path -LiteralPath $updlist)) { throw "Falta $updlist. Compile a solução antes (client/COMPILAR.md)." }
if (-not $NoCopy) {
    foreach ($file in 'ProjectG.exe', 'wangreal.dll') {
        $source = Join-Path $BuildDir $file
        $target = Join-Path $GameDir $file
        if (-not (Test-Path -LiteralPath $source)) { throw "Falta $source. Compile a solução antes." }
        if ((Test-Path -LiteralPath $target) -and -not (Test-Path -LiteralPath "$target.antes-mulligan")) {
            Copy-Item -LiteralPath $target -Destination "$target.antes-mulligan"
        }
        Copy-Item -LiteralPath $source -Destination $target -Force
    }
    Write-Host '[run-local] ProjectG.exe e wangreal.dll copiados'
}

# 3. updatelist com o CRC do que está na pasta: vale até a próxima compilação
$webDir = Join-Path $repo 'server\data\gameweb\patch'
New-Item -ItemType Directory -Force -Path $webDir | Out-Null
& $updlist --generate $GameDir (Join-Path $webDir 'updatelist')
if ($LASTEXITCODE -ne 0) { throw 'updlist falhou.' }

# 4. Servidor no WSL; tools/start.sh não sobe duas vezes
if (-not $NoServer) {
    $serverDir = (& wsl.exe -u root wslpath -a ($repo.Replace('\', '/') + '/server')).Trim()
    $gameWsl = (& wsl.exe -u root wslpath -a $GameDir.Replace('\', '/')).Trim()
    & wsl.exe -u root -- env "GAME_DIR=$gameWsl" bash "$serverDir/tools/start.sh"
    if ($LASTEXITCODE -ne 0) { throw 'tools/start.sh falhou. Veja server/logs/server.out e docs/server/COMECANDO.md.' }
}

# 5. Web do jogo no ar
$webUrl = if ($Web) { $Web.TrimEnd('/') } else { 'http://localhost:30091' }
for ($try = 1; ; $try++) {
    try {
        $null = Invoke-WebRequest -UseBasicParsing -Uri "$webUrl/patch/updatelist" -TimeoutSec 3
        break
    }
    catch {
        if ($try -ge 30) { throw "$webUrl/patch/updatelist não respondeu. O Run da configuração do servidor tem `"game-web`"?" }
        Start-Sleep -Seconds 2
    }
}
Write-Host "[run-local] web do jogo ok em $webUrl"
if ($NoGame) { return }

# 6. Jogo, sempre por um processo elevado
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
$argList = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "`"$PSCommandPath`"", '-GameDir', "`"$GameDir`"", '-Launch')
foreach ($pair in @(@('-Login', $Login), @('-Web', $Web), @('-Www', $Www))) {
    if ($pair[1]) { $argList += $pair[0], "`"$($pair[1])`"" }
}
$start = @{ FilePath = 'powershell.exe'; ArgumentList = $argList; PassThru = $true; WindowStyle = 'Hidden' }
if (-not $isAdmin) { $start.Verb = 'RunAs' }
$proc = Start-Process @start                     # sem -Wait: ele esperaria também o jogo, processo filho
$null = $proc.Handle                             # no PowerShell 5.1, ExitCode só existe com o handle guardado
$proc.WaitForExit()
if ($proc.ExitCode -ne 0) { throw "Não abriu o jogo: $(Get-Content -LiteralPath $launchLog -ErrorAction SilentlyContinue)" }
Write-Host "[run-local] jogo aberto: $(Get-Content -LiteralPath $launchLog)"
Write-Host '[run-local] para encerrar o servidor: wsl -u root bash tools/stop.sh, dentro de server/'
