@echo off
rem ---------------------------------------------------------------------------
rem  cargar.bat - compila practicas.c con MikroC y lo graba en el PIC18F45K50.
rem
rem  Doble clic y sale un menu. Tambien se puede sin menu:
rem     cargar compilar [parte]      parte 0 = todo, 1..4 = por partes (demo de MikroC)
rem     cargar grabar   [archivo.hex]
rem     cargar todo     [parte]      compila y graba
rem     cargar leer                  baja lo que tiene el PIC a build\leido.hex
rem     cargar borrar
rem
rem  Las rutas de MikroC / MPLAB / pk2cmd se guardan en rutas.ini. Si cambias
rem  de compu y ya no existen, las busca solo o te deja escogerlas.
rem  Todo el trabajo lo hace herramientas\cargar.ps1.
rem ---------------------------------------------------------------------------
setlocal
cd /d "%~dp0"
powershell -NoProfile -STA -ExecutionPolicy Bypass -File "%~dp0herramientas\cargar.ps1" %*
set CODIGO=%ERRORLEVEL%
if not "%~1"=="" pause
exit /b %CODIGO%
