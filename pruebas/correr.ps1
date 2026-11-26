# Prueba practicas.c en la PC, sin el PIC.
# Quita los renglones "sbit ... at ..." (solo los entiende MikroC), compila el
# simulador con el gcc de MSYS2 y corre todos los escenarios.
#   powershell -ExecutionPolicy Bypass -File pruebas\correr.ps1

$ErrorActionPreference = 'Stop'
$aqui = Split-Path -Parent $MyInvocation.MyCommand.Path
$raiz = Split-Path -Parent $aqui

$gcc = $null
foreach ($c in @('C:\msys64\ucrt64\bin\gcc.exe', 'C:\msys64\mingw64\bin\gcc.exe')) {
    if (Test-Path $c) { $gcc = $c; break }
}
if (-not $gcc) {
    $g = Get-Command gcc -ErrorAction SilentlyContinue
    if ($g) { $gcc = $g.Source }
}
if (-not $gcc) { Write-Host 'No encontre gcc (MSYS2). Instalalo o pon gcc en el PATH.'; exit 2 }
$env:PATH = (Split-Path -Parent $gcc) + ';' + $env:PATH

$fuente = Get-Content -LiteralPath (Join-Path $raiz 'practicas.c')
$sinSbit = $fuente | Where-Object { $_ -notmatch '^\s*sbit\s' }
Set-Content -LiteralPath (Join-Path $aqui 'practicas_pc.c') -Value $sinSbit -Encoding ASCII

$exe = Join-Path $aqui 'simulador.exe'
# -funsigned-char: en MikroC 'char' no lleva signo (0..255); en gcc si.
& $gcc -std=c99 -O1 -funsigned-char -Wall -Wno-unused-variable -Wno-pointer-sign -Wno-discarded-qualifiers `
    -Wno-unused-but-set-variable -Wno-char-subscripts -Wno-unused-label `
    -I $aqui -o $exe (Join-Path $aqui 'simulador.c')
if ($LASTEXITCODE -ne 0) { Write-Host 'No compilo el simulador.'; exit 1 }
& $exe
exit $LASTEXITCODE
