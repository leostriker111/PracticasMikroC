# Instala (o actualiza) las practicas de MikroC en esta compu.
#
#   irm https://raw.githubusercontent.com/leostriker111/PracticasMikroC/main/instalar.ps1 | iex
#
# Pregunta en que carpeta (Enter = C:\Users\<tu usuario>\PracticasMikroC), baja
# el repo con git si lo hay o como .zip si no, y deja un acceso directo
# "Practicas MikroC" en el escritorio que abre cargar.bat.
# Si ya estaba instalado, lo actualiza sin tocar tu rutas.ini.

$ErrorActionPreference = 'Stop'
$repo = 'leostriker111/PracticasMikroC'

$porOmision = Join-Path $env:USERPROFILE 'PracticasMikroC'
$destino = $env:PRACTICAS_MIKROC
if (-not $destino) {
    $r = Read-Host "En que carpeta lo instalo? (Enter = $porOmision)"
    if ($r) { $destino = $r.Trim('"') } else { $destino = $porOmision }
}
if ($destino -match 'OneDrive') {
    Write-Host 'Ojo: dentro de OneDrive la compilacion se sube a la nube y a veces se traba.' -ForegroundColor Yellow
}

$git = Get-Command git -ErrorAction SilentlyContinue
if ($git -and (Test-Path -LiteralPath (Join-Path $destino '.git'))) {
    Write-Host "Actualizando $destino ..."
    git -C $destino pull --ff-only
} elseif ($git -and -not (Test-Path -LiteralPath $destino)) {
    Write-Host "Bajando a $destino ..."
    git clone "https://github.com/$repo.git" $destino
} else {
    Write-Host "Bajando el .zip a $destino ..."
    $zip = Join-Path $env:TEMP 'PracticasMikroC.zip'
    $tmp = Join-Path $env:TEMP 'PracticasMikroC_zip'
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    Invoke-WebRequest -UseBasicParsing "https://github.com/$repo/archive/refs/heads/main.zip" -OutFile $zip
    if (Test-Path -LiteralPath $tmp) { Remove-Item -LiteralPath $tmp -Recurse -Force }
    Expand-Archive -LiteralPath $zip -DestinationPath $tmp
    $dentro = Get-ChildItem -LiteralPath $tmp -Directory | Select-Object -First 1
    New-Item -ItemType Directory -Force -Path $destino | Out-Null
    Copy-Item -Path (Join-Path $dentro.FullName '*') -Destination $destino -Recurse -Force
    Remove-Item -LiteralPath $zip -Force
    Remove-Item -LiteralPath $tmp -Recurse -Force
}

# acceso directo en el escritorio
$escritorio = [Environment]::GetFolderPath('Desktop')
$lnk = Join-Path $escritorio 'Practicas MikroC.lnk'
$sh = New-Object -ComObject WScript.Shell
$a = $sh.CreateShortcut($lnk)
$a.TargetPath = Join-Path $destino 'cargar.bat'
$a.WorkingDirectory = $destino
$a.Description = 'Compilar y grabar las practicas de MikroC al PIC18F45K50'
$a.Save()

Write-Host ''
Write-Host "Listo en $destino" -ForegroundColor Green
Write-Host "  - cargar.bat (o el acceso directo 'Practicas MikroC' del escritorio): compilar y grabar"
Write-Host '  - practicas.c: el programa, se abre en MikroC'
Write-Host '  - Entrega_y_pines.xlsx: que mover, que combinacion y que se ve; y las conexiones'
Start-Process explorer.exe $destino
