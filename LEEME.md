<p align="center">
  <a href="README.md">English</a> · <b>Español</b>
</p>

<div align="center">

# PracticasMikroC

### Todas las prácticas de MikroC del PIC18F45K50 en un solo programa, con un selector.

Las prácticas de ensamblador pasadas a C, el LCD, el teclado 4x4 y dos aplicaciones,
más un `.bat` que compila y graba sin que se rompan las rutas al cambiar de compu.

![C](https://img.shields.io/badge/C-MikroC%20PRO%20for%20PIC-555?style=flat-square)
![PIC](https://img.shields.io/badge/PIC-18F45K50-555?style=flat-square)
![licencia](https://img.shields.io/badge/licencia-MIT-555?style=flat-square)
![pruebas](https://img.shields.io/github/actions/workflow/status/leostriker111/PracticasMikroC/pruebas.yml?branch=main&style=flat-square&label=simulador)
![plataforma](https://img.shields.io/badge/plataforma-Windows-555?style=flat-square)

</div>

## Qué es

Un solo programa en C para MikroC PRO for PIC con las 15 prácticas del curso de
Dispositivos Programables. Se graba una vez y con los switches escoges cuál corre: los 4
de la derecha del puerto A son el **selector** y los 4 de la izquierda son los **datos**
de cada práctica. Al mover el selector, la pantalla LCD te dice en qué práctica estás y
qué tienes que mover.

Junto viene `cargar.bat`, que compila con MikroC y graba con el PICkit (el 3, su clon
"3.5", el 4 o un clon de PICkit 2), y un Excel con la lista de entrega y el acomodo de
pines.

## Para qué y hasta dónde

**Para qué.** Para no tener 15 proyectos sueltos de MikroC y 15 `.hex`: un programa, una
grabada, y en la revisión nomás mueves switches.

**Qué abarca.** Pasar de ensamblador a C (monitoreo de un bit, monitoreo de palabra,
palabra en hexa, subrutinas de tiempo, tabla), LCD (mensaje, animación, marquesina),
teclado (uso del teclado, LCD + teclado) y la aplicación final (chapa con clave y
calculadora).

**Qué no hace a propósito.** No usa la librería `Keypad` de MikroC: el teclado se lee a
mano, porque la librería pide resistencias externas en el teclado y así se brinca ese
problema. Tampoco usa interrupciones: cada práctica es un ciclo infinito, como en clase.

**A dónde va.** El tercer departamental trae el LM35; esa práctica cabe en el mismo
esquema (sobran RE2 y RC2, y el ADC del PIC).

## Índice

**Para usarlo**
- [Instalación](#instalación)
- [Las prácticas y el selector](#las-prácticas-y-el-selector)
- [Conexiones](#conexiones)
- [Compilar y grabar con cargar.bat](#compilar-y-grabar-con-cargarbat)
- [Si usas el MikroC de la escuela](#si-usas-el-mikroc-de-la-escuela)
- [Si algo no se ve bien](#si-algo-no-se-ve-bien)

**Para meterle mano**
- [De qué está hecho](#de-qué-está-hecho)
- [Las carpetas](#las-carpetas)
- [El camino de un cambio de selector](#el-camino-de-un-cambio-de-selector)
- [Problemas, y las decisiones que forzaron](#problemas-y-las-decisiones-que-forzaron)
- [Probarlo en la PC](#probarlo-en-la-pc)

## Instalación

En PowerShell:

```powershell
irm https://raw.githubusercontent.com/leostriker111/PracticasMikroC/main/instalar.ps1 | iex
```

Te pregunta en qué carpeta lo pone (Enter = `C:\Users\<tú>\PracticasMikroC`), lo baja
con git si lo hay o como `.zip` si no, y deja en el escritorio un acceso directo
**Practicas MikroC** que abre `cargar.bat`. Si lo vuelves a correr, actualiza sin tocar
tus rutas.

Lo que necesitas en la compu:

| Para | Qué |
|---|---|
| Compilar | MikroC PRO for PIC (la demo sirve, ver [la demo](#la-demo-de-mikroc)) |
| Grabar con PICkit 3 / clon 3.5 / PICkit 4 | MPLAB X **6.20** (trae `ipecmd`; la 6.25 ya no enlista el PICkit 3) |
| Grabar con clon de PICkit 2 | `pk2cmd` con un `PK2DeviceFile.dat` que traiga el 18F45K50 |

## Las prácticas y el selector

El selector se lee **de izquierda a derecha como lo ves en la tarjeta**: RA4 vale 8, RA5
vale 4, RA6 vale 2 y RA7 vale 1. `0111` es la práctica 7.

| Selector | # | Práctica | Qué mueves |
|---|---|---|---|
| `0000` | 0 | Espera (sale la rayita `-`) | nada |
| `0001` | 1 | PORTA → PORTD | cualquier switch |
| `0010` | 2 | Monitoreo de un bit | RA2 |
| `0011` | 3 | Monitoreo de un bit: pares e impares | RA0 |
| `0100` | 4 | Monitoreo de palabra (0101) | RA0..RA3 |
| `0101` | 5 | Palabra en hexadecimal | RA0..RA3 |
| `0110` | 6 | Palabra a letras (IPN-UPIICSA) | RA0..RA3 |
| `0111` | 7 | Subrutinas de tiempo (1 ms, 1 s, 3 s, Timer0) | RA0 RA1 |
| `1000` | 8 | Tabla (0..F, HOLA-IPn, nombre) | RA0 RA1 |
| `1001` | 9 | LCD: mostrar un mensaje | RA0 |
| `1010` | 10 | LCD: animación | nada |
| `1011` | 11 | LCD: marquesina | RA0 (rápido) RA1 (al revés) |
| `1100` | 12 | Uso del teclado | teclas |
| `1101` | 13 | LCD + teclado | teclas |
| `1110` | 14 | Aplicación: chapa con clave | teclas |
| `1111` | 15 | Aplicación: calculadora | teclas |

Cada combinación, qué se ve en la barra y qué en el LCD está en la hoja **Entrega** de
[`Entrega_y_pines.xlsx`](Entrega_y_pines.xlsx), con una columna para palomear.

<details>
<summary><b>Las dos aplicaciones, tecla por tecla</b></summary>
<br>

**Chapa con clave (14).** Teclea 4 números y `#`; `*` borra. Si es la clave, el LCD dice
`ABIERTA :)` y el display una `A`. Abierta: `B` la cierra, `A` cambia la clave (4
números y `#`). La clave se guarda en la EEPROM del PIC, así que no se olvida al
desconectarlo; vuelve a `1234` cada que grabas el PIC. Tres claves malas y se bloquea 10
s con cuenta regresiva en el display.

**Calculadora (15).** `A` = más, `B` = menos, `C` = por, `D` = entre, `#` = igual, `*` =
borrar. Números de hasta 6 cifras. Después de un resultado puedes seguir con otra
operación. La división da el residuo (`= 3 r 1`). Si el resultado no cabe en 32 bits
dice `Muy grande`, y entre cero dice `Error`.

</details>

## Conexiones

| Qué | Pines del PIC |
|---|---|
| Switches de datos | RA0 RA1 RA2 RA3 |
| Switches del selector | RA4 RA5 RA6 RA7 |
| Teclado, columnas 1 a 4 | RB0 RB1 RB2 RB3 (pines 5 a 8 del teclado) |
| Teclado, filas 1 a 4 | RB4 RB5 RB6 RB7 (pines 1 a 4 del teclado) |
| LCD RS / E | RE0 / RE1 (RW a tierra) |
| LCD D4 D5 D6 D7 | RC0 RC1 RC6 RC7 (D0..D3 al aire) |
| Barra + display (a..g, punto) | RD0..RD7 |

Pata por pata, los 16 pines del LCD, el teclado y el PICkit están en la hoja **Pines**
del Excel.

**Lo que cambió contra el programa de ensamblador:** el selector se pasó del puerto B a
RA4..RA7, porque el teclado ocupa todo el puerto B. Los switches de B de la tarjeta ya
no se usan: quítales el puente o déjalos todos abajo.

## Compilar y grabar con cargar.bat

Doble clic a `cargar.bat` (o al acceso directo):

```
=========== PRACTICAS MIKROC - PIC18F45K50 ===========
  Programador: PK3   Voltaje: 4.5
  1  Compilar practicas.c
  2  Grabar build\practicas.hex al PIC
  3  Compilar y grabar
  4  Grabar otro .hex (por ejemplo el que hizo el MikroC de la escuela)
  5  Descargar lo que tiene el PIC (respaldo a build\leido.hex)
  6  Borrar el PIC
  7  Cambiar rutas / programador / voltaje
  8  Abrir la carpeta build
  0  Salir
```

**Las rutas ya no truenan.** Se guardan en `rutas.ini`. Si una ya no existe (otra compu,
otra versión de MPLAB), la busca sola en los lugares de siempre; si no la encuentra, te
abre una ventana para que escojas el `.exe`. En la opción 7 cambias todo a mano.

**Voltaje.** Por omisión el PICkit le da 4.5 V a la tarjeta: a 5.0 la tarjeta jalaba más
de lo que el PICkit daba y se caía. Si la tarjeta tiene su propia fuente pon `FUENTE`
(dos fuentes juntas no).

**La palabra de configuración** la mete `cargar.bat` dentro del `.hex` después de
compilar: oscilador interno, sin perro guardián, MCLR activo, puerto B digital y **LVP
apagado** (si no, RB5 no sirve para el teclado). Así da igual lo que diga el proyecto de
MikroC.

Sin menú, para la terminal:

```powershell
.\cargar.bat todo        # compila y graba
.\cargar.bat compilar 2  # solo la parte del LCD (para la demo de MikroC)
.\cargar.bat grabar C:\otra\carpeta\algo.hex
```

## Si usas el MikroC de la escuela

1. **Project → New Project → Standard project**. Device `P18F45K50`, clock `16.000000`
   MHz. Agrega `practicas.c`.
2. **Library Manager**: palomea `Lcd` y `Lcd_Constants`. Nada más.
3. **Project → Edit Project** (la palabra de configuración, por si grabas desde otro
   programa que no sea `cargar.bat`):

| Opción | Valor |
|---|---|
| Oscillator Selection | Internal oscillator (INTOSCIO) |
| PLL / CPU divide | PLL apagado, sin dividir |
| Power-up Timer | Enabled |
| Brown-out Reset | Disabled |
| Watchdog Timer | Disabled |
| MCLR Pin | Enabled |
| PORTB A/D | Digital (PBADEN off) |
| Single-Supply ICSP (LVP) | **Disabled** |
| Extended Instruction Set | Disabled |

4. **Build** (Ctrl+F9). El `.hex` queda junto al proyecto; grábalo con la opción 4 de
   `cargar.bat`, que de paso le pone la palabra de configuración buena.

### La demo de MikroC

La versión sin licencia solo compila programas chicos. Si sale *demo limit*, en
`practicas.c` cambia `#define PARTE 0` a `1` (asm a C), `2` (LCD), `3` (teclado) o `4`
(aplicaciones) y graba por partes; con `cargar.bat` nomás escoges la parte al compilar.
Las prácticas que no van en la parte grabada dicen `No va en PARTE n` y una `E`.

## Si algo no se ve bien

| Qué pasa | Por qué | Qué hacer |
|---|---|---|
| No prende nada, ni la rayita | el PIC se quedó en reset o sin corriente | graba con `cargar.bat` (lleva `-OL`, que suelta el reset) |
| El LCD prende pero sin letras, o puros cuadros | contraste | mueve el potenciómetro del pin 3 |
| Todo sale al revés | tus LEDs prenden con 0 | `#define INVERTIR_SALIDA 1` |
| Sale una práctica que no pediste | selector leído al revés | revisa que RA4 sea el de la izquierda del selector |
| Aprietas 1 y sale 4 | filas y columnas del teclado al revés | la práctica 12 te dice qué fila y columna leyó |
| Salen teclas solas | un switch de B quedó arriba | bájalos o quita el puente de B |
| Funciona un rato y se cae | el PICkit no da tanta corriente | fuente de 5 V y `VOLTAJE = FUENTE` |
| "Invalid command response" al grabar | MPLAB X tiene agarrado el PICkit | ciérralo; un programa a la vez |
| Tarda en cambiar de práctica | estabas en la 7 modo 3 s | espera a que acabe la cuenta |

<br>

---

<div align="center">

## 🔧 Para quien le meta mano

*Todo lo de arriba es lo que hace. Todo lo de abajo es cómo lo hace.*

</div>

---

## Dónde ayudaría una mano

- **Probarlo con la demo de MikroC** y anotar cuánto mide cada `PARTE` en palabras de
  programa: no se ha podido medir sin MikroC instalado.
- **La práctica del LM35** del tercer departamental: RE2 está libre y es la entrada
  analógica AN7, así que cabe sin mover nada más (habría que hacerle lugar en el selector).
- Confirmar los parámetros de línea de comandos de `mikroCPIC1618.exe` en la versión de
  la escuela (ver [problemas](#problemas-y-las-decisiones-que-forzaron)).

## De qué está hecho

C de MikroC (C89 con sus extras: `sbit`, `Delay_ms`, librería `Lcd`), un script de
PowerShell 5.1 para cargar, un Python para el Excel, y un simulador en C para probar en
la PC.

| La parte difícil | Quién la hace |
|---|---|
| Saber qué práctica corre y salirse luego luego | `sigue()` y `esperar()` en `practicas.c` |
| Leer el teclado sin resistencias externas | `teclado_inicio()` y `escanear()` |
| Que la clave no se olvide | `ee_leer()` / `ee_escribir()` con registros directos |
| La palabra de configuración | `Arreglar-Config` en `herramientas\cargar.ps1` |
| Que las rutas no truenen | `Asegurar` en `herramientas\cargar.ps1` |
| Probar todo sin el PIC | `pruebas\simulador.c` |

## Las carpetas

```
practicas.c              el programa: las 15 prácticas y el selector
cargar.bat               el menú de compilar y grabar
instalar.ps1             el instalador de una línea
Entrega_y_pines.xlsx     lista de entrega + acomodo de pines
herramientas/
  cargar.ps1             lo que hace cargar.bat por dentro
  hacer_excel.py         genera el Excel
pruebas/
  simulador.c            finge ser el PIC: switches, teclado, LCD, Timer0, EEPROM
  correr.ps1             compila el simulador con gcc y corre los escenarios
build/                   lo que sale al compilar (no se sube)
rutas.ini                tus rutas (no se sube)
```

## El camino de un cambio de selector

1. `main()` llama a `selector_estable()`: lee RA4..RA7 dos veces con 20 ms entre lecturas
   hasta que coincidan (los switches rebotan).
2. Limpia la barra y el LCD y pinta el título y la pista de la práctica.
3. Espera 0.8 s para que alcances a leer, revisando el selector.
4. Entra a la práctica, que es un ciclo `while (sigue())`.
5. `sigue()` compara el selector con la práctica actual; si cambió dos lecturas
   seguidas, regresa 0, la práctica termina y `main()` vuelve al paso 1.
6. Las esperas largas usan `esperar(ms)`, que revisa el selector cada 16 ms y se sale
   antes si lo moviste.

## Problemas, y las decisiones que forzaron

**El teclado se comía el puerto B.** El programa de ensamblador tenía el selector en
RB0..RB4. El teclado necesita 8 pines y el único puerto completo libre con pull-ups
internos es el B. El selector se fue a RA4..RA7, y los datos quedaron en RA0..RA3; por
eso el selector bajó de 5 a 4 bits (16 prácticas exactas).

**RB6 y RB7 son del PICkit.** El PICkit 3 tiene resistencias a tierra en PGC/PGD; si ahí
fueran columnas (entradas con pull-up), leerían siempre 0 y saldrían teclas fantasma con
el PICkit conectado, que es como siempre está porque da la corriente. Por eso las filas
(salidas) van en RB4..RB7 y las columnas en RB0..RB3.

**¿Pull-up o pull-down?** En protoboard el teclado no trae resistencias; en algunas
entrenadoras los pines de B traen resistencias a tierra. `teclado_inicio()` lee las
columnas al arrancar: si salen en 1 escanea con la fila en 0, y si salen en 0 apaga los
pull-ups y escanea con la fila en 1. El mismo `.hex` jala en los dos.

**La palabra de configuración vive en el proyecto de MikroC, no en el `.c`.** MikroC no
tiene `#pragma config`. Para no depender de que alguien haya llenado *Edit Project*,
`cargar.ps1` quita del `.hex` lo que haya en 0x300000..0x30000D y pone los 14 bytes
buenos, que se sacaron compilando la configuración con XC8 de Microchip.

**LVP.** El 18F45K50 sale de fábrica con programación de bajo voltaje encendida, y con
eso RB5 deja de ser un pin normal. Se apaga en la palabra de configuración.

**MikroC no deja mezclar apuntadores a RAM y a memoria de programa.** Por eso hay
`lcd_linea()` para textos `const` y `lcd_linea_ram()` para los que se arman en RAM.

**La línea de comandos de MikroC.** `cargar.ps1` usa la forma documentada
(`-pP18F45K50 -fo16 -N... -SP... practicas.c __Lib_*.mcl`). Esta compu no tiene MikroC,
así que no se ha corrido de verdad; si en la versión de la escuela falla, compila en el
IDE y graba con la opción 4.

**`char` sin signo.** En MikroC `char` va de 0 a 255; en gcc lleva signo. El simulador
se compila con `-funsigned-char`; sin eso, `ee_leer(0) == 0xA5` salía falso en la PC y
parecía que la chapa olvidaba la clave.

## Probarlo en la PC

`pruebas\simulador.c` incluye `practicas.c` tal cual (sin los renglones `sbit`) y finge
los registros del PIC: switches, teclado con filas y columnas, LCD de 2x16 con su CGRAM,
Timer0 y EEPROM. Cada escenario pone el selector, teclea y revisa qué quedó en la barra
y en el LCD.

```powershell
powershell -ExecutionPolicy Bypass -File pruebas\correr.ps1
```

Necesita el gcc de MSYS2 (`C:\msys64\ucrt64\bin`). Hoy: **119 revisiones, 0 fallas**.

## Licencia

MIT.

## Proyectos relacionados

- **KTool** — *empieza por ahí si tu práctica es de lógica combinacional.* Simplifica
  mapas de Karnaugh y genera el `.c` para el mismo PIC18F45K50.

## Sobre qué se construyó

No es un fork. Se basó en los programas de clase (PortAB.c, ParesImpares.c,
Pregunta4Bits.c, CONTADOR.c, IPNU.c, los de LCD y teclado), en el programa de
ensamblador anterior del mismo curso y en la hoja de datos del PIC18F45K50.
