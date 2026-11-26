# ---------------------------------------------------------------------------
#  cargar.ps1 - compila practicas.c con MikroC y lo graba al PIC18F45K50.
#  Lo abre cargar.bat; no hace falta correrlo a mano.
#
#  Las rutas (MikroC, ipecmd, pk2cmd) se guardan en rutas.ini junto a
#  cargar.bat. Si una ruta ya no existe (otra compu, otra version de MPLAB),
#  la vuelve a buscar sola y, si no la encuentra, te abre una ventana para
#  que la escojas. Por eso ya no truena cuando cambias de compu.
#
#  Uso directo (sin menu):
#     cargar.bat compilar [parte]     parte = 0 (todo) .. 4
#     cargar.bat grabar  [archivo.hex]
#     cargar.bat todo    [parte]      compila y graba
#     cargar.bat leer                 baja lo que tiene el PIC a build\leido.hex
#     cargar.bat borrar
# ---------------------------------------------------------------------------
param([string]$accion = '', [string]$extra = '')

$ErrorActionPreference = 'Stop'
$herramientas = Split-Path -Parent $MyInvocation.MyCommand.Path
$raiz  = Split-Path -Parent $herramientas
$build = Join-Path $raiz 'build'
$ini   = Join-Path $raiz 'rutas.ini'
$PIC   = '18F45K50'

# Palabra de configuracion del 18F45K50 (la sacamos de XC8 de Microchip):
#   oscilador interno con RA6/RA7 libres, 16 MHz sin PLL, sin perro guardian,
#   power-up timer, MCLR activo, PORTB digital al arrancar, LVP apagado
#   (si no, RB5 no sirve para el teclado), sin proteccion de codigo.
#   0x300000..0x30000D = 00 28 58 3C FF D1 81 FF 0F C0 0F E0 0F 40
$CONFIG_45K50 = @(':020000040030CA', ':0E0000000028583CFFD181FF0FC00FE00F40D9')

# --------------------------- rutas.ini ------------------------------------
function Leer-Rutas {
    $r = @{ MIKROC = ''; IPECMD = ''; PK2CMD = ''; PK3REGISTRO = '';
            PROGRAMADOR = 'PK3'; VOLTAJE = '4.5'; HEX = '' }
    if (Test-Path -LiteralPath $ini) {
        foreach ($l in Get-Content -LiteralPath $ini) {
            if ($l -match '^\s*([A-Z0-9]+)\s*=\s*(.*)$') { $r[$Matches[1]] = $Matches[2].Trim() }
        }
    }
    return $r
}

function Guardar-Rutas($r) {
    $t = @('# Rutas de cargar.bat. Puedes editarlo a mano o usar la opcion "Cambiar rutas".',
           '# PROGRAMADOR = PK3 (PICkit 3 y clones 3.5), PK4, o PK2 (clones de PICkit 2 con pk2cmd)',
           '# VOLTAJE     = 4.5 / 5.0 (el PICkit le da corriente a la tarjeta) o FUENTE (la tarjeta tiene la suya)')
    foreach ($k in 'MIKROC','IPECMD','PK2CMD','PK3REGISTRO','PROGRAMADOR','VOLTAJE','HEX') { $t += "$k=$($r[$k])" }
    Set-Content -LiteralPath $ini -Value $t -Encoding ASCII
}

# --------------------------- buscar programas ------------------------------
function Elegir-Archivo([string]$titulo, [string]$filtro, [string]$carpeta) {
    Add-Type -AssemblyName System.Windows.Forms
    $d = New-Object System.Windows.Forms.OpenFileDialog
    $d.Title = $titulo
    $d.Filter = $filtro
    if ($carpeta -and (Test-Path -LiteralPath $carpeta)) { $d.InitialDirectory = $carpeta }
    if ($d.ShowDialog() -eq [System.Windows.Forms.DialogResult]::OK) { return $d.FileName }
    return ''
}

function Programas-Raiz {
    $l = @()
    foreach ($v in @($env:ProgramFiles, ${env:ProgramFiles(x86)}, 'C:\', 'D:\')) {
        if ($v -and (Test-Path -LiteralPath $v)) { $l += $v }
    }
    return $l
}

function Buscar-MikroC {
    foreach ($base in Programas-Raiz) {
        foreach ($sub in @('Mikroelektronika\mikroC PRO for PIC', 'mikroC PRO for PIC', 'Mikroelektronika\mikroC PRO for PIC 2')) {
            $d = Join-Path $base $sub
            if (Test-Path -LiteralPath $d) {
                $exe = Get-ChildItem -LiteralPath $d -Filter 'mikroCPIC*.exe' -ErrorAction SilentlyContinue | Select-Object -First 1
                if ($exe) { return $exe.FullName }
            }
        }
        $d = Join-Path $base 'Mikroelektronika'
        if (Test-Path -LiteralPath $d) {
            $exe = Get-ChildItem -LiteralPath $d -Recurse -Depth 2 -Filter 'mikroCPIC*.exe' -ErrorAction SilentlyContinue | Select-Object -First 1
            if ($exe) { return $exe.FullName }
        }
    }
    return ''
}

function Buscar-Ipecmd {
    # La 6.20 primero: es la ultima que reconoce el PICkit 3 sin trucos.
    $mx = Join-Path $env:ProgramFiles 'Microchip\MPLABX'
    if (-not (Test-Path -LiteralPath $mx)) { return '' }
    $vers = Get-ChildItem -LiteralPath $mx -Directory | Sort-Object Name
    $pref = $vers | Where-Object { $_.Name -eq 'v6.20' }
    $resto = $vers | Where-Object { $_.Name -ne 'v6.20' } | Sort-Object Name -Descending
    foreach ($v in @($pref) + @($resto)) {
        if (-not $v) { continue }
        $exe = Join-Path $v.FullName 'mplab_platform\mplab_ipe\ipecmd.exe'
        if (Test-Path -LiteralPath $exe) { return $exe }
    }
    return ''
}

function Buscar-Pk2cmd {
    $c = Get-Command pk2cmd -ErrorAction SilentlyContinue
    if ($c) { return $c.Source }
    foreach ($base in Programas-Raiz) {
        foreach ($sub in @('Microchip\PICkit 2 v2', 'PICkit 2 v2', 'pk2cmd', 'PICkitminus')) {
            $exe = Join-Path (Join-Path $base $sub) 'pk2cmd.exe'
            if (Test-Path -LiteralPath $exe) { return $exe }
        }
    }
    $loc = Join-Path $env:LOCALAPPDATA 'pic-movil'
    if (Test-Path -LiteralPath $loc) {
        $e = Get-ChildItem -LiteralPath $loc -Recurse -Filter 'pk2cmd.exe' -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($e) { return $e.FullName }
    }
    return ''
}

function Buscar-Pk3Registro {
    foreach ($c in @((Join-Path $herramientas 'pk3-registro.jar'),
                     (Join-Path $env:USERPROFILE 'Downloads\PIC18F45K50_Prueba\herramientas\mplab-pk3\pk3-registro.jar'),
                     (Join-Path $env:USERPROFILE 'Downloads\PIC18F45K50_Practicas\herramientas\mplab-pk3\pk3-registro.jar'))) {
        if (Test-Path -LiteralPath $c) { return $c }
    }
    return ''
}

# Devuelve una ruta que SI existe: la guardada, la que encuentre solo, o la
# que escojas en la ventana. '' si cancelas.
function Asegurar([hashtable]$r, [string]$clave, [scriptblock]$buscar, [string]$titulo, [string]$filtro) {
    if ($r[$clave] -and (Test-Path -LiteralPath $r[$clave])) { return $r[$clave] }
    if ($r[$clave]) { Write-Host "  La ruta guardada de $clave ya no existe: $($r[$clave])" -ForegroundColor Yellow }
    $p = & $buscar
    if (-not $p) {
        Write-Host "  No lo encontre solo. Escogelo en la ventana que se abre: $titulo" -ForegroundColor Yellow
        $p = Elegir-Archivo $titulo $filtro $env:ProgramFiles
    }
    if ($p) { $r[$clave] = $p; Guardar-Rutas $r; Write-Host "  $clave = $p" }
    return $p
}

# --------------------------- el .hex --------------------------------------
# Pone la palabra de configuracion buena dentro del .hex (quita la que haya
# dejado MikroC). Asi el PIC arranca bien aunque en el proyecto de MikroC no
# se haya tocado "Edit Project", y aunque grabes con otro programa ese .hex.
function Arreglar-Config([string]$hex) {
    $lineas = Get-Content -LiteralPath $hex
    $salida = New-Object System.Collections.Generic.List[string]
    $alta = 0
    foreach ($l in $lineas) {
        if ($l -notmatch '^:([0-9A-Fa-f]{2})([0-9A-Fa-f]{4})([0-9A-Fa-f]{2})') { $salida.Add($l); continue }
        $n = [Convert]::ToInt32($Matches[1], 16)
        $dir = [Convert]::ToInt32($Matches[2], 16)
        $tipo = [Convert]::ToInt32($Matches[3], 16)
        if ($tipo -eq 4) { $alta = [Convert]::ToInt32($l.Substring(9, 4), 16); $salida.Add($l); continue }
        if ($tipo -eq 1) { continue }                       # el fin se pone abajo
        if ($tipo -eq 0) {
            $abs = ($alta * 65536) + $dir
            if ($abs -lt 0x30000E -and ($abs + $n) -gt 0x300000) { continue }   # config vieja
        }
        $salida.Add($l)
    }
    foreach ($c in $CONFIG_45K50) { $salida.Add($c) }
    $salida.Add(':00000001FF')
    Set-Content -LiteralPath $hex -Value $salida -Encoding ASCII
}

# --------------------------- compilar -------------------------------------
function Compilar([hashtable]$r, [string]$parte) {
    if ($parte -notmatch '^[0-4]$') { $parte = '0' }
    $mk = Asegurar $r 'MIKROC' { Buscar-MikroC } 'el compilador mikroCPIC1618.exe (carpeta de mikroC PRO for PIC)' 'mikroC|mikroCPIC*.exe|Programas|*.exe'
    if (-not $mk) { Write-Host 'Sin MikroC no se puede compilar.' -ForegroundColor Red; return $false }
    $dirMk = Split-Path -Parent $mk
    $defs = Join-Path $dirMk 'defs'
    $uses = Join-Path $dirMk 'uses\P18'
    if (-not (Test-Path -LiteralPath $uses)) { $uses = Join-Path $dirMk 'Uses\P18' }

    New-Item -ItemType Directory -Force -Path $build | Out-Null
    # copia de practicas.c con la PARTE escogida (el original no se toca)
    $src = Get-Content -LiteralPath (Join-Path $raiz 'practicas.c')
    $src = $src -replace '^#define PARTE\s+\d', "#define PARTE $parte"
    Set-Content -LiteralPath (Join-Path $build 'practicas.c') -Value $src -Encoding ASCII

    $libs = @()
    foreach ($n in '__Lib_Math.mcl','__Lib_MathDouble.mcl','__Lib_System.mcl','__Lib_Delays.mcl','__Lib_LcdConsts.mcl','__Lib_Lcd.mcl') {
        if (Test-Path -LiteralPath (Join-Path $uses $n)) { $libs += $n }
    }
    if ($libs.Count -lt 4) {
        # otra version de MikroC con otros nombres: que se lleve todo lo de Lcd, Delays, System y Math
        $libs = Get-ChildItem -LiteralPath $uses -Filter '__Lib_*.mcl' |
                Where-Object { $_.Name -match '^__Lib_(Math|MathDouble|System|Delays|Lcd|LcdConsts)\.mcl$' } |
                ForEach-Object { $_.Name }
    }
    $proy = Join-Path $build 'practicas.mcppi'
    $argumentos = @('-MSF', '-DBG', "-pP$PIC", '-O11111114', '-fo16',
                    "-N`"$proy`"", "-SP`"$defs\`"", "-SP`"$uses\`"", "-SP`"$build\`"",
                    '"practicas.c"') + ($libs | ForEach-Object { "`"$_`"" })
    Write-Host ''
    Write-Host "Compilando (PARTE $parte) con $mk" -ForegroundColor Cyan
    $hex = Join-Path $build 'practicas.hex'
    if (Test-Path -LiteralPath $hex) { Remove-Item -LiteralPath $hex }
    Push-Location $build
    try {
        $p = Start-Process -FilePath $mk -ArgumentList $argumentos -NoNewWindow -Wait -PassThru
    } finally { Pop-Location }
    if (-not (Test-Path -LiteralPath $hex)) {
        Write-Host ''
        Write-Host 'No salio build\practicas.hex.' -ForegroundColor Red
        Write-Host 'Lee los errores de arriba. Si dice "demo limit", compila por partes (opcion 1 y escoge 1..4).'
        Write-Host 'Si MikroC no se deja desde aqui: abre practicas.c en MikroC, compila (Ctrl+F9)'
        Write-Host 'y usa la opcion "Grabar otro .hex" con el .hex que te deje MikroC.'
        return $false
    }
    Arreglar-Config $hex
    Write-Host "Listo: $hex (con la palabra de configuracion puesta)" -ForegroundColor Green
    return $true
}

# --------------------------- programador ----------------------------------
function Correr-Programador([hashtable]$r, [string[]]$opciones) {
    $prog = $r.PROGRAMADOR.ToUpper()
    if ($prog -eq 'PK2') {
        $pk2 = Asegurar $r 'PK2CMD' { Buscar-Pk2cmd } 'pk2cmd.exe (PICkit 2)' 'pk2cmd|pk2cmd.exe|Programas|*.exe'
        if (-not $pk2) { return 1 }
        $a = @("-PPIC$PIC")
        if ($r.VOLTAJE.ToUpper() -ne 'FUENTE') { $a += '-T'; $a += "-A$($r.VOLTAJE)" }
        $a += $opciones
        Write-Host "pk2cmd $($a -join ' ')" -ForegroundColor DarkGray
        Push-Location (Split-Path -Parent $pk2)       # pk2cmd busca PK2DeviceFile.dat junto a el
        try { & $pk2 @a | Out-Host; $code = $LASTEXITCODE } finally { Pop-Location }
        return $code
    }
    $ipe = Asegurar $r 'IPECMD' { Buscar-Ipecmd } 'ipecmd.exe (MPLAB X\...\mplab_platform\mplab_ipe)' 'ipecmd|ipecmd.exe|Programas|*.exe'
    if (-not $ipe) { return 1 }
    $tp = '-TPPK3'
    if ($prog -eq 'PK4') { $tp = '-TPPK4' }
    $a = @($tp, "-P$PIC")
    if ($r.VOLTAJE.ToUpper() -ne 'FUENTE') { $a += "-W$($r.VOLTAJE)" }
    $a += $opciones
    $ver = Split-Path -Leaf (Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $ipe)))
    $usarJar = ($prog -eq 'PK3' -and $ver -ne 'v6.20' -and $ver -match '^v6\.(2[5-9]|[3-9])')
    if ($usarJar) {
        # MPLAB 6.25 o mas nueva ya no enlista el PICkit 3: se le agrega el registro.
        $jar = $r.PK3REGISTRO
        if (-not ($jar -and (Test-Path -LiteralPath $jar))) { $jar = Buscar-Pk3Registro; $r.PK3REGISTRO = $jar; Guardar-Rutas $r }
        $mx = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $ipe))
        $java = Get-ChildItem -LiteralPath (Join-Path $mx 'sys\java') -Recurse -Filter 'java.exe' -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($jar -and $java) {
            $cp = (Join-Path (Split-Path -Parent $ipe) 'ipecmd.jar') + ';' + $jar
            Write-Host "ipecmd ($ver + pk3-registro) $($a -join ' ')" -ForegroundColor DarkGray
            & $java.FullName '-Dfile.encoding=UTF-8' '-cp' $cp 'com.microchip.mplab.ipecmd.IPECMD' @a | Out-Host
            return $LASTEXITCODE
        }
        Write-Host "  Ojo: MPLAB $ver puede no ver el PICkit 3. Si falla, instala MPLAB X 6.20." -ForegroundColor Yellow
    }
    Write-Host "ipecmd $($a -join ' ')" -ForegroundColor DarkGray
    & $ipe @a | Out-Host
    return $LASTEXITCODE
}

function Grabar([hashtable]$r, [string]$hex) {
    if (-not $hex) { $hex = Join-Path $build 'practicas.hex' }
    if (-not (Test-Path -LiteralPath $hex)) {
        Write-Host "No existe $hex. Compila primero o escoge otro .hex." -ForegroundColor Red
        return $false
    }
    Write-Host ''
    Write-Host "Grabando $hex" -ForegroundColor Cyan
    Write-Host 'Cierra MPLAB X / IPE si estan abiertos: solo un programa a la vez puede usar el PICkit.'
    # -M programa todo, -Y verifica, -OL suelta el reset (sin esto el PIC se queda pausado)
    if ($r.PROGRAMADOR.ToUpper() -eq 'PK2') { $code = Correr-Programador $r @("-F$hex", '-M', '-Y', '-R') }
    else { $code = Correr-Programador $r @("-F$hex", '-M', '-Y', '-OL') }
    if ($code -eq 0) { Write-Host 'Grabado y corriendo.' -ForegroundColor Green; return $true }
    Write-Host "El programador termino con codigo $code. Revisa el mensaje de arriba." -ForegroundColor Red
    return $false
}

function Leer([hashtable]$r) {
    New-Item -ItemType Directory -Force -Path $build | Out-Null
    $out = Join-Path $build 'leido.hex'
    $code = Correr-Programador $r @("-GF$out")
    if ($code -eq 0) { Write-Host "Lo que tenia el PIC quedo en $out" -ForegroundColor Green }
}

function Borrar([hashtable]$r) {
    $code = Correr-Programador $r @('-E')
    if ($code -eq 0) { Write-Host 'PIC borrado.' -ForegroundColor Green }
}

# --------------------------- cambiar rutas --------------------------------
function Cambiar-Rutas([hashtable]$r) {
    while ($true) {
        Write-Host ''
        Write-Host '--- Rutas y programador ---' -ForegroundColor Cyan
        Write-Host "  1  MikroC       : $($r.MIKROC)"
        Write-Host "  2  ipecmd       : $($r.IPECMD)"
        Write-Host "  3  pk2cmd       : $($r.PK2CMD)"
        Write-Host "  4  Programador  : $($r.PROGRAMADOR)   (PK3 = PICkit 3 / clon 3.5, PK4, PK2)"
        Write-Host "  5  Voltaje      : $($r.VOLTAJE)   (4.5, 5.0 o FUENTE si la tarjeta tiene la suya)"
        Write-Host '  6  Buscar todo otra vez solo'
        Write-Host '  0  Regresar'
        $o = Read-Host 'Opcion'
        switch ($o) {
            '1' { $p = Elegir-Archivo 'mikroCPIC1618.exe' 'mikroC|mikroCPIC*.exe|Programas|*.exe' $env:ProgramFiles; if ($p) { $r.MIKROC = $p } }
            '2' { $p = Elegir-Archivo 'ipecmd.exe' 'ipecmd|ipecmd.exe|Programas|*.exe' $env:ProgramFiles; if ($p) { $r.IPECMD = $p } }
            '3' { $p = Elegir-Archivo 'pk2cmd.exe' 'pk2cmd|pk2cmd.exe|Programas|*.exe' $env:ProgramFiles; if ($p) { $r.PK2CMD = $p } }
            '4' { $p = Read-Host 'PK3, PK4 o PK2'; if ($p -match '^(?i)PK[234]$') { $r.PROGRAMADOR = $p.ToUpper() } }
            '5' { $p = Read-Host '4.5, 5.0 o FUENTE'; if ($p -match '^(?i)(fuente|[3-5]\.[0-9])$') { $r.VOLTAJE = $p.ToUpper() } }
            '6' { $r.MIKROC = Buscar-MikroC; $r.IPECMD = Buscar-Ipecmd; $r.PK2CMD = Buscar-Pk2cmd; $r.PK3REGISTRO = Buscar-Pk3Registro }
            '0' { Guardar-Rutas $r; return }
        }
        Guardar-Rutas $r
    }
}

# --------------------------- menu -----------------------------------------
$r = Leer-Rutas
switch ($accion.ToLower()) {
    'compilar' { if (Compilar $r $extra) { exit 0 } else { exit 1 } }
    'grabar'   { if (Grabar $r $extra) { exit 0 } else { exit 1 } }
    'todo'     { if ((Compilar $r $extra) -and (Grabar $r '')) { exit 0 } else { exit 1 } }
    'leer'     { Leer $r; exit 0 }
    'borrar'   { Borrar $r; exit 0 }
}

while ($true) {
    Write-Host ''
    Write-Host '=========== PRACTICAS MIKROC - PIC18F45K50 ===========' -ForegroundColor Cyan
    Write-Host "  Programador: $($r.PROGRAMADOR)   Voltaje: $($r.VOLTAJE)"
    Write-Host '  1  Compilar practicas.c'
    Write-Host '  2  Grabar build\practicas.hex al PIC'
    Write-Host '  3  Compilar y grabar'
    Write-Host '  4  Grabar otro .hex (por ejemplo el que hizo el MikroC de la escuela)'
    Write-Host '  5  Descargar lo que tiene el PIC (respaldo a build\leido.hex)'
    Write-Host '  6  Borrar el PIC'
    Write-Host '  7  Cambiar rutas / programador / voltaje'
    Write-Host '  8  Abrir la carpeta build'
    Write-Host '  0  Salir'
    $o = Read-Host 'Opcion'
    switch ($o) {
        '1' { $pa = Read-Host 'Parte (Enter = 0 todo; 1 asm a C, 2 LCD, 3 teclado, 4 apps)'; [void](Compilar $r $pa) }
        '2' { [void](Grabar $r '') }
        '3' { $pa = Read-Host 'Parte (Enter = 0 todo)'; if (Compilar $r $pa) { [void](Grabar $r '') } }
        '4' {
            $h = Elegir-Archivo 'El .hex a grabar' 'Archivo hex|*.hex' $(if ($r.HEX) { Split-Path -Parent $r.HEX } else { $raiz })
            if ($h) {
                $r.HEX = $h; Guardar-Rutas $r
                $c = Read-Host 'Le pongo la palabra de configuracion buena? (S/n)'
                if ($c -notmatch '^(?i)n') {
                    New-Item -ItemType Directory -Force -Path $build | Out-Null
                    $copia = Join-Path $build ('cfg_' + (Split-Path -Leaf $h))
                    Copy-Item -LiteralPath $h -Destination $copia -Force
                    Arreglar-Config $copia
                    $h = $copia
                }
                [void](Grabar $r $h)
            }
        }
        '5' { Leer $r }
        '6' { $c = Read-Host 'Seguro que lo borro? (s/N)'; if ($c -match '^(?i)s') { Borrar $r } }
        '7' { Cambiar-Rutas $r }
        '8' { New-Item -ItemType Directory -Force -Path $build | Out-Null; Start-Process explorer.exe $build }
        '0' { exit 0 }
    }
}
