<#
.SYNOPSIS
    Instala um pak do mapmaker numa pasta de teste do jogo e abre o cliente com um updatelist que o inclui.

.DESCRIPTION
    Só paks listados no updatelist entram no cliente (IntegrityPakCRC), então a lista precisa ser refeita. Se você usa
    client/tools/run-local.ps1 não precisa disto: copie o pak para a pasta do jogo e rode o run-local, que refaz a lista.
    Este script serve para quando o web da porta 30091 pertence a outra pasta: sobe um web em 30092 que entrega a lista
    nova e abre o jogo com JP983_WEB apontando para ele. O jogo pede elevação, que descarta variáveis; por isso elas são
    definidas dentro do processo elevado.

.PARAMETER Pak        O projectg_zz_mulligan.pak gerado.
.PARAMETER GameDir    Pasta de teste do jogo. Nunca use a sua cópia original.
.PARAMETER Updlist    updlist.exe do build do cliente.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File tools\mapmaker\instalar.ps1 -Pak saida\projectg_zz_mulligan.pak -GameDir D:\Testes\PangYa -Updlist client\build\vs2022\Release\updlist.exe
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$Pak,
    [Parameter(Mandatory = $true)][string]$GameDir,
    [Parameter(Mandatory = $true)][string]$Updlist
)
$ErrorActionPreference = 'Stop'
if (Get-Process ProjectG -ErrorAction SilentlyContinue) { throw 'Há um ProjectG aberto. Feche antes de instalar.' }
$GameDir = (Resolve-Path -LiteralPath $GameDir).Path.TrimEnd('\')
$work = Join-Path $env:TEMP 'mapmaker-web'
New-Item -ItemType Directory -Force -Path $work | Out-Null
Copy-Item -LiteralPath $Pak -Destination $GameDir -Force
$list = Join-Path $work 'updatelist'
& $Updlist --generate $GameDir $list
if ($LASTEXITCODE -ne 0) { throw 'updlist falhou.' }
Start-Process -WindowStyle Hidden python -ArgumentList "-I", "`"$PSScriptRoot\serve_updatelist.py`"", "`"$list`""
Start-Sleep -Seconds 2
$launch = Join-Path $work 'abrir.ps1'
@"
`$env:JP983_WEB = 'http://localhost:30092'
Start-Process -FilePath '$GameDir\ProjectG.exe' -WorkingDirectory '$GameDir'
"@ | Set-Content -LiteralPath $launch -Encoding UTF8
Start-Process powershell -Verb RunAs -WindowStyle Hidden -ArgumentList '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "`"$launch`""
Write-Host 'Jogo aberto. Ao terminar, feche o jogo e o python do web (porta 30092).'
