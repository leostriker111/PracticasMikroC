<p align="center">
  <b>English</b> · <a href="LEEME.md">Español</a>
</p>

<div align="center">

# PracticasMikroC

### Every MikroC lab for the PIC18F45K50 in one program, picked with a selector.

The assembly labs ported to C, the LCD, the 4x4 keypad and two small applications,
plus a `.bat` that compiles and programs the chip without breaking when you move to
another computer.

![license](https://img.shields.io/badge/license-MIT-555?style=flat-square)
![C](https://img.shields.io/badge/C-MikroC%20PRO%20for%20PIC-555?style=flat-square)
![tests](https://img.shields.io/github/actions/workflow/status/leostriker111/PracticasMikroC/pruebas.yml?branch=main&style=flat-square&label=simulator)
![PIC](https://img.shields.io/badge/PIC-18F45K50-555?style=flat-square)
![platform](https://img.shields.io/badge/platform-Windows-555?style=flat-square)

</div>

## What it is

A single C program for MikroC PRO for PIC holding the 15 labs of a *Programmable
Devices* course (UPIICSA, IPN). You flash it once and choose which lab runs with DIP
switches: the right four switches of port A are the **selector**, the left four are the
**data** each lab reads. Whenever the selector changes, the 16x2 LCD shows the lab's
name and what to move.

It ships with `cargar.bat`, which compiles with MikroC and programs through a PICkit
(3, its "3.5" clones, 4, or a PICkit 2 clone), and a spreadsheet with the hand-in list
and the full pinout. The documentation and all on-screen text are in Spanish; the
[Spanish README](LEEME.md) is the complete one.

## Purpose and scope

**Why.** Fifteen separate MikroC projects and fifteen `.hex` files are a pain during a
lab review. One program, one flash, and you just flip switches.

**What it covers.** Assembly-to-C ports (single-bit and word monitoring, hex word, delay
subroutines with Timer0, lookup tables), LCD (static message, custom-character
animation, scrolling marquee), keypad (key reading, keypad + LCD), and two final
applications: a keypad lock that stores its code in EEPROM, and an integer calculator.

**What it leaves out on purpose.** It does not use MikroC's `Keypad` library: the keypad
is scanned by hand so it works without external resistors. No interrupts either; each
lab is an endless loop, as taught in class.

## Contents

**Using it**
- [Install](#install)
- [The labs](#the-labs)
- [Wiring](#wiring)
- [Compile and program](#compile-and-program)

**Working on it**
- [How it is built](#how-it-is-built)
- [Problems, and the decisions they forced](#problems-and-the-decisions-they-forced)
- [Testing on a PC](#testing-on-a-pc)

## Install

In PowerShell:

```powershell
irm https://raw.githubusercontent.com/leostriker111/PracticasMikroC/main/instalar.ps1 | iex
```

It asks for a folder, downloads the repo (git or `.zip`) and puts a *Practicas MikroC*
shortcut on the desktop. Running it again updates in place and keeps your saved paths.

You need MikroC PRO for PIC to compile (the free demo works, by parts), and MPLAB X
**6.20** for `ipecmd` (6.25 no longer lists the PICkit 3) or `pk2cmd` for a PICkit 2
clone.

## The labs

The selector reads left to right as seen on the board: RA4 = 8, RA5 = 4, RA6 = 2,
RA7 = 1.

| Selector | # | Lab | Input |
|---|---|---|---|
| `0000` | 0 | Idle (a `-` on the display) | – |
| `0001` | 1 | PORTA → PORTD | any switch |
| `0010` | 2 | Single-bit monitoring | RA2 |
| `0011` | 3 | Single-bit: odd / even | RA0 |
| `0100` | 4 | Word monitoring (0101) | RA0..RA3 |
| `0101` | 5 | Word as a hex digit | RA0..RA3 |
| `0110` | 6 | Word to letters (IPN-UPIICSA) | RA0..RA3 |
| `0111` | 7 | Delay subroutines (1 ms, 1 s, 3 s, Timer0) | RA0 RA1 |
| `1000` | 8 | Lookup table (0..F, HOLA-IPn, a name) | RA0 RA1 |
| `1001` | 9 | LCD message | RA0 |
| `1010` | 10 | LCD animation | – |
| `1011` | 11 | LCD marquee | RA0 speed, RA1 direction |
| `1100` | 12 | Keypad | keys |
| `1101` | 13 | LCD + keypad | keys |
| `1110` | 14 | App: keypad lock | keys |
| `1111` | 15 | App: calculator | keys |

Every input combination and its expected output is in the **Entrega** sheet of
[`Entrega_y_pines.xlsx`](Entrega_y_pines.xlsx).

## Wiring

| What | PIC pins |
|---|---|
| Data switches | RA0..RA3 |
| Selector switches | RA4..RA7 |
| Keypad columns 1–4 | RB0..RB3 (keypad pins 5–8) |
| Keypad rows 1–4 | RB4..RB7 (keypad pins 1–4) |
| LCD RS / E | RE0 / RE1 (RW to ground) |
| LCD D4..D7 | RC0 RC1 RC6 RC7 (4-bit mode) |
| LED bar + 7-segment (a..g, dp) | RD0..RD7 |

Pin by pin, including all 16 LCD pins and the PICkit header: **Pines** sheet.

## Compile and program

Double-click `cargar.bat` for a menu: compile, program, compile and program, program any
other `.hex`, read the chip back, erase it, and change paths. Paths live in
`rutas.ini`; if a saved path no longer exists, the script searches the usual install
folders and, failing that, opens a file picker. After compiling it rewrites the
configuration words inside the `.hex` (internal oscillator, no watchdog, MCLR on, PORTB
digital, **LVP off** so RB5 works), so the MikroC project settings don't matter.

With the MikroC demo, set `#define PARTE` to 1–4 (or pick the part in the menu) and
flash one block at a time.

<br>

---

<div align="center">

## 🔧 For developers

*Everything above is what the program does. Everything below is how it does it.*

</div>

---

## How it is built

| Hard part | Where |
|---|---|
| Knowing which lab runs and leaving it at once | `sigue()`, `esperar()` in `practicas.c` |
| Keypad with no external resistors | `teclado_inicio()`, `escanear()` |
| Remembering the lock code | `ee_leer()` / `ee_escribir()`, raw EEPROM registers |
| Configuration words | `Arreglar-Config` in `herramientas/cargar.ps1` |
| Paths that survive another PC | `Asegurar` in `herramientas/cargar.ps1` |
| Testing without hardware | `pruebas/simulador.c` |

## Problems, and the decisions they forced

- **The keypad needs a full port with pull-ups**, and only port B has them, so the
  selector moved from RB0..RB4 (the old assembly program) to RA4..RA7, and shrank to 4
  bits.
- **RB6/RB7 are PGC/PGD**, and the PICkit pulls them to ground. Inputs with weak
  pull-ups there would read 0 forever, so the keypad *rows* (outputs) sit on RB4..RB7.
- **Pull-up or pull-down?** At start-up the program reads the idle columns and picks the
  scanning polarity, so the same `.hex` works on a breadboard and on trainer boards with
  pull-down resistors on port B.
- **MikroC keeps configuration words in the project file**, not in the source. The
  loader replaces whatever is at 0x300000–0x30000D with 14 bytes produced by Microchip's
  XC8 from named settings.
- **The MikroC command line was not run here** (no MikroC on this machine); it follows
  the documented form. If it fails on another version, build in the IDE and use the
  "program another .hex" option.

## Testing on a PC

`pruebas/simulador.c` includes `practicas.c` unchanged (minus the `sbit` lines) and
fakes the PIC: switches, a row/column keypad, a 2x16 LCD with CGRAM, Timer0 and EEPROM.
Each scenario sets the selector, types keys and checks the LED bar and the LCD.

```powershell
powershell -ExecutionPolicy Bypass -File pruebas\correr.ps1
```

Needs gcc (MSYS2 or any MinGW). Current result: 119 checks, 0 failures. CI runs the same
script on every push.

## License

MIT.

## Related projects

- **KTool** — *start there if your lab is combinational logic.* Simplifies Karnaugh maps
  and emits C for the same PIC18F45K50.

## Built on

Not a fork. Based on the course's MikroC examples, the earlier assembly program for the
same course, and the PIC18F45K50 datasheet.
