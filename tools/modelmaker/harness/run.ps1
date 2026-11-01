# Roda pethost em lote com tempo limite. Uso: run.ps1 -Dir pasta -Pak pasta_do_jogo -Files a.pet,b.mpet [-Bmp]
param([string]$Dir, [string]$Pak, [string[]]$Files, [switch]$Bmp, [int]$Timeout = 60)
Set-Location $Dir
foreach ($f in $Files) {
    $a = @($f)
    if ($Pak) { $a += '--pak', $Pak }
    if ($Bmp) { $a += '--bmp', ((Split-Path $f -Leaf) + '.bmp') }
    $p = Start-Process .\pethost.exe -ArgumentList $a -NoNewWindow -PassThru -RedirectStandardOutput o.txt -RedirectStandardError e.txt
    if (-not $p.WaitForExit($Timeout * 1000)) { $p.Kill(); "TIMEOUT $f"; continue }
    $line = (Get-Content o.txt | Where-Object { $_ -match '^(OK|FALHA|captura)' }) -join ' | '
    "{0,-26} rc={1} {2}" -f $f, $p.ExitCode, $line
}
