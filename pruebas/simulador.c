/* Simulador en la PC para probar practicas.c sin el PIC.
   Finge los registros del PIC18F45K50, el LCD 2x16, el teclado 4x4, el
   Timer0 y la EEPROM, y corre escenarios: "pon el selector en 14, teclea
   1234#, revisa que el LCD diga ABIERTA".

   Lo compila pruebas\correr.ps1 (gcc de MSYS2). No se graba en el PIC.     */

#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <stdlib.h>

typedef unsigned char u8;

/* ------------------------------ registros -------------------------------- */
u8 porta_v, LATA, LATB, LATC, LATD, LATE;
u8 TRISA, TRISB, TRISC, TRISD, TRISE;
u8 ANSELA, ANSELB, ANSELC, ANSELD, ANSELE;
u8 OSCCON, CM1CON0, CM2CON0, WPUB, INTCON2, T0CON, TMR0H, EEADR, EECON2;
u8 *mock_portb_p(void);
u8 *mock_eecon1(void);
u8 *mock_eedata(void);
u8 *mock_intcon(void);
u8 *mock_tmr0l(void);
u8 *mock_porta(void);
#define PORTA  (*mock_porta())
#define PORTB  (*mock_portb_p())
#define EECON1 (*mock_eecon1())
#define EEDATA (*mock_eedata())
#define INTCON (*mock_intcon())
#define TMR0L  (*mock_tmr0l())

/* ------------------------------ tiempo ----------------------------------- */
long long ahora_us;          /* tiempo simulado */
long long fin_us;
jmp_buf salida;

typedef struct { long t_ms; char tipo; int valor; } Evento;
Evento *eventos; int n_eventos, sig_evento;
int tecla_hw = -1;           /* indice 0..15 de la tecla apretada, -1 ninguna */
int hw_pulldown = 0;         /* 1: la tarjeta trae resistencias a tierra en el teclado */

#define MAX_FOTOS 40
typedef struct { u8 latd; char r1[17]; char r2[17]; int hay; } Foto;
Foto fotos[MAX_FOTOS];
void tomar_foto(int n);

void procesar_eventos(void) {
    while (sig_evento < n_eventos && eventos[sig_evento].t_ms * 1000LL <= ahora_us) {
        Evento *e = &eventos[sig_evento++];
        if (e->tipo == 'A') porta_v = (u8)e->valor;
        if (e->tipo == 'K') tecla_hw = e->valor;
        if (e->tipo == 'F') tomar_foto(e->valor);
    }
}

void avanzar(long long us) {
    ahora_us += us;
    procesar_eventos();
    if (ahora_us >= fin_us) longjmp(salida, 1);
}
void Delay_ms(unsigned int ms) { avanzar((long long)ms * 1000); }
void Delay_us(unsigned int us) { avanzar(us); }

/* Leer un puerto le cuesta unos microsegundos al PIC; sin esto los ciclos que
   solo leen switches (practicas 1 a 6) nunca dejarian avanzar el reloj.     */
u8 *mock_porta(void) { avanzar(2); return &porta_v; }

/* ------------------------------ puerto B --------------------------------- */
/* Filas RB4..RB7 (salidas mientras se escanea), columnas RB0..RB3. */
u8 portb_valor;
u8 *mock_portb_p(void) {
    u8 v = 0;
    int c, fila;
    int pullups = (WPUB & 0x0F) && !(INTCON2 & 0x80);
    for (c = 0; c < 4; c++) {
        int bit;
        if (hw_pulldown) bit = 0;                 /* la resistencia a tierra le gana */
        else bit = pullups ? 1 : 1;               /* flotando: en la PC lo tomamos 1 */
        if (tecla_hw >= 0 && tecla_hw % 4 == c) {
            fila = tecla_hw / 4;
            if (!(TRISB & (0x10 << fila)))        /* esa fila es salida: manda */
                bit = (LATB >> (4 + fila)) & 1;
        }
        if (bit) v |= (u8)(1 << c);
    }
    for (fila = 0; fila < 4; fila++)
        if (LATB & (0x10 << fila)) v |= (u8)(0x10 << fila);
    portb_valor = v;
    return &portb_valor;
}

/* ------------------------------ EEPROM ----------------------------------- */
u8 eeprom[256];
u8 eecon1_v, eedata_v;
void ee_pendiente(void) {
    if (eecon1_v & 0x01) { eedata_v = eeprom[EEADR]; eecon1_v &= (u8)~0x01; }
    if (eecon1_v & 0x02) {
        if (eecon1_v & 0x04) eeprom[EEADR] = eedata_v;
        eecon1_v &= (u8)~0x02;
    }
}
u8 *mock_eecon1(void) { ee_pendiente(); return &eecon1_v; }
u8 *mock_eedata(void) { ee_pendiente(); return &eedata_v; }

/* ------------------------------ Timer0 ----------------------------------- */
u8 intcon_v, tmr0l_v;
int t0_armado, t0_lecturas;
u8 *mock_tmr0l(void) { t0_armado = 1; t0_lecturas = 0; return &tmr0l_v; }
u8 *mock_intcon(void) {
    avanzar(1);                        /* que un ciclo de espera no cuelgue la PC */
    if (t0_armado && (T0CON & 0x80)) {
        t0_lecturas++;
        if (t0_lecturas >= 3) {        /* las 2 primeras son del "borra la bandera" */
            /* 16 bits, 1:128 a 4 MHz de instrucciones: (65536 - TMR0) * 32 us */
            long cuentas = 65536L - (((long)TMR0H << 8) | tmr0l_v);
            t0_armado = 0;
            intcon_v |= 0x04;
            avanzar(cuentas * 32);
        }
    }
    return &intcon_v;
}

/* ------------------------------ LCD -------------------------------------- */
#define _LCD_CLEAR 1
#define _LCD_RETURN_HOME 2
#define _LCD_CURSOR_OFF 12
u8 ddram[2][40];
u8 cgram[64];
int lcd_f, lcd_c, lcd_cg, lcd_cg_dir;

void Lcd_Init(void) { memset(ddram, ' ', sizeof ddram); lcd_f = lcd_c = 0; lcd_cg = 0; }
void Lcd_Cmd(u8 c) {
    if (c == _LCD_CLEAR) { memset(ddram, ' ', sizeof ddram); lcd_f = lcd_c = 0; lcd_cg = 0; }
    else if (c == _LCD_RETURN_HOME) { lcd_f = lcd_c = 0; lcd_cg = 0; }
    else if (c >= 64 && c < 128) { lcd_cg = 1; lcd_cg_dir = c - 64; }
    else if (c >= 128) { lcd_cg = 0; lcd_f = (c & 0x40) ? 1 : 0; lcd_c = c & 0x3F; }
    avanzar(2000);
}
void Lcd_Chr_Cp(u8 ch) {
    if (lcd_cg) { cgram[lcd_cg_dir & 63] = ch; lcd_cg_dir++; }
    else { if (lcd_c < 40) ddram[lcd_f][lcd_c] = ch; lcd_c++; }
    avanzar(50);
}
void Lcd_Chr(u8 f, u8 c, u8 ch) { lcd_cg = 0; lcd_f = f - 1; lcd_c = c - 1; Lcd_Chr_Cp(ch); }
void Lcd_Out_Cp(char *s) { while (*s) Lcd_Chr_Cp((u8)*s++); }
void Lcd_Out(u8 f, u8 c, char *s) { lcd_cg = 0; lcd_f = f - 1; lcd_c = c - 1; Lcd_Out_Cp(s); }

/* renglon visible; caracteres propios 0..7 -> 'a'..'h', 255 -> '#' */
void renglon(int f, char *out) {
    int i;
    for (i = 0; i < 16; i++) {
        u8 ch = ddram[f][i];
        if (ch < 8) out[i] = (char)('a' + ch);
        else if (ch == 255) out[i] = '#';
        else out[i] = (char)ch;
    }
    out[16] = 0;
}
void tomar_foto(int n) {
    fotos[n].latd = LATD;
    renglon(0, fotos[n].r1);
    renglon(1, fotos[n].r2);
    fotos[n].hay = 1;
}

/* ------------------------------ el programa ------------------------------ */
#define main pic_main
#include "practicas_pc.c"
#undef main

/* ------------------------------ escenarios ------------------------------- */
int fallas = 0, revisiones = 0;

void correr(Evento *ev, int n, long dura_ms) {
    eventos = ev; n_eventos = n; sig_evento = 0;
    ahora_us = 0; fin_us = dura_ms * 1000LL;
    memset(fotos, 0, sizeof fotos);
    porta_v = 0; tecla_hw = -1; t0_armado = 0; intcon_v = 0;
    procesar_eventos();
    if (setjmp(salida) == 0) pic_main();
}

void revisa_d(const char *que, int foto, u8 esperado) {
    revisiones++;
    if (!fotos[foto].hay) { printf("  FALLA %s: no hubo foto %d\n", que, foto); fallas++; return; }
    if (fotos[foto].latd != esperado) {
        printf("  FALLA %s: LATD = 0x%02X, se esperaba 0x%02X\n", que, fotos[foto].latd, esperado);
        fallas++;
    }
}
void revisa_lcd(const char *que, int foto, int fila, const char *esperado) {
    char e[17];
    const char *real;
    revisiones++;
    snprintf(e, sizeof e, "%-16s", esperado);
    real = fila == 1 ? fotos[foto].r1 : fotos[foto].r2;
    if (!fotos[foto].hay || strcmp(real, e) != 0) {
        printf("  FALLA %s: renglon %d = \"%s\", se esperaba \"%s\"\n", que, fila, real, e);
        fallas++;
    }
}

/* selector (como lo ves: RA4 RA5 RA6 RA7) + datos (RA3..RA0 como numero) */
int A(int sel, int datos) {
    int v = datos & 0x0F;
    if (sel & 8) v |= 0x10;
    if (sel & 4) v |= 0x20;
    if (sel & 2) v |= 0x40;
    if (sel & 1) v |= 0x80;
    return v;
}
int K(char c) { int i; for (i = 0; i < 16; i++) if (MAPA_TECLAS[i] == c) return i; return -1; }

/* teclea una cadena empezando en t ms: 120 ms apretada, 120 ms suelta */
int teclear(Evento *ev, int n, long t, const char *s) {
    while (*s) {
        ev[n].t_ms = t;       ev[n].tipo = 'K'; ev[n].valor = K(*s); n++;
        ev[n].t_ms = t + 120; ev[n].tipo = 'K'; ev[n].valor = -1;    n++;
        t += 240; s++;
    }
    return n;
}
#define EV(t_, ti_, v_) do { ev[n].t_ms = (t_); ev[n].tipo = (ti_); ev[n].valor = (v_); n++; } while (0)

int main(void) {
    static Evento ev[400];
    int n, i;
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("0 espera\n");
    n = 0; EV(0,'A',A(0,0)); EV(500,'F',0);
    correr(ev, n, 600);
    revisa_d("guion", 0, 0x40); revisa_lcd("titulo", 0, 1, "PRACTICAS MIKROC");

    printf("1 PORTA->PORTD\n");
    n = 0; EV(0,'A',A(1,5)); EV(1500,'F',0);
    correr(ev, n, 1600);
    revisa_d("copia", 0, (u8)A(1,5)); revisa_lcd("titulo", 0, 1, "P01 PORTA>PORTD");

    printf("2 monitoreo de un bit\n");
    n = 0; EV(0,'A',A(2,0x04)); EV(1500,'F',0); EV(1600,'A',A(2,0x0B)); EV(1800,'F',1);
    correr(ev, n, 1900);
    revisa_d("RA2=1", 0, 0x6A); revisa_lcd("RA2=1", 0, 2, "bit = 1 -> 0x6A");
    revisa_d("RA2=0", 1, 0x01); revisa_lcd("RA2=0", 1, 2, "bit = 0 -> 0x01");

    printf("3 pares / impares\n");
    n = 0; EV(0,'A',A(3,1)); EV(1500,'F',0); EV(1600,'A',A(3,0x0E)); EV(1800,'F',1);
    correr(ev, n, 1900);
    revisa_d("RA0=1", 0, 0xAA); revisa_d("RA0=0", 1, 0x55);

    printf("4 monitoreo de palabra\n");
    n = 0; EV(0,'A',A(4,0x5)); EV(1500,'F',0); EV(1600,'A',A(4,0xA)); EV(1800,'F',1);
    EV(1900,'A',A(4,0xD)); EV(2100,'F',2);
    correr(ev, n, 2200);
    revisa_d("0101", 0, 0x59); revisa_lcd("0101", 0, 2, "1010 SI -> 0x59");
    revisa_d("1010", 1, 0x11); revisa_d("1101", 2, 0x11);

    printf("5 palabra en hex\n");
    for (i = 0; i < 16; i++) {
        n = 0; EV(0,'A',A(5,i)); EV(1200,'F',0);
        correr(ev, n, 1300);
        revisa_d("hex", 0, DIGITOS[i]);
    }
    n = 0; EV(0,'A',A(5,0xB)); EV(1200,'F',0); correr(ev, n, 1300);
    revisa_lcd("hex B", 0, 2, "1101 = B (11)");

    printf("6 palabra a letras\n");
    n = 0; EV(0,'A',A(6,4)); EV(1200,'F',0); EV(1300,'A',A(6,12)); EV(1500,'F',1);
    correr(ev, n, 1600);
    revisa_d("U", 0, 0x3E); revisa_lcd("U", 0, 2, "0010 -> U"); revisa_d("apagado", 1, 0x00);

    printf("7 subrutinas\n");
    /* modo 1 s: el contador sube 1 por segundo */
    n = 0; EV(0,'A',A(7,1)); EV(1500,'F',0); EV(4500,'F',1);
    correr(ev, n, 4600);
    revisiones++;
    if ((u8)(fotos[1].latd - fotos[0].latd) != 3) { printf("  FALLA 1 s: %d -> %d\n", fotos[0].latd, fotos[1].latd); fallas++; }
    revisa_lcd("modo 1s", 0, 2, "Modo: 1 s");
    /* modo 3 s */
    n = 0; EV(0,'A',A(7,2)); EV(1000,'F',0); EV(7100,'F',1);
    correr(ev, n, 7200);
    revisiones++;
    if ((u8)(fotos[1].latd - fotos[0].latd) != 2) { printf("  FALLA 3 s: %d -> %d\n", fotos[0].latd, fotos[1].latd); fallas++; }
    /* Timer0 1 s */
    n = 0; EV(0,'A',A(7,3)); EV(1500,'F',0); EV(4500,'F',1);
    correr(ev, n, 4600);
    revisiones++;
    if ((u8)(fotos[1].latd - fotos[0].latd) != 3) { printf("  FALLA Timer0: %d -> %d\n", fotos[0].latd, fotos[1].latd); fallas++; }
    revisa_lcd("modo T0", 0, 2, "Modo: Timer0 1 s");
    /* 1 ms: en 1 s cuenta muchas veces */
    n = 0; EV(0,'A',A(7,0)); EV(1000,'F',0); EV(1100,'F',1);
    correr(ev, n, 1200);
    revisiones++;
    if ((u8)(fotos[1].latd - fotos[0].latd) < 50) { printf("  FALLA 1 ms: %d -> %d\n", fotos[0].latd, fotos[1].latd); fallas++; }
    /* el selector se puede mover a media espera de 3 s */
    n = 0; EV(0,'A',A(7,2)); EV(2000,'A',A(0,0)); EV(5500,'F',0);
    correr(ev, n, 5600);
    revisa_d("sale de la 7", 0, 0x40);

    printf("8 tabla\n");
    n = 0; EV(0,'A',A(8,0));
    for (i = 0; i < 18; i++) EV(800 + 350 + i * 700, 'F', i);
    correr(ev, n, 800 + 350 + 18 * 700);
    for (i = 0; i < 18; i++) revisa_d("0..F", i, DIGITOS[i % 16]);
    n = 0; EV(0,'A',A(8,1));
    for (i = 0; i < 8; i++) EV(800 + 350 + i * 700, 'F', i);
    correr(ev, n, 800 + 350 + 8 * 700);
    for (i = 0; i < 8; i++) revisa_d("HOLA-IPn", i, TABLA_HOLA[i]);
    revisa_lcd("HOLA", 0, 2, "HOLA-IPn");
    n = 0; EV(0,'A',A(8,3)); EV(1150,'F',0); correr(ev, n, 1200);
    revisa_d("F..0", 0, DIGITOS[15]);

    printf("9 mensaje LCD\n");
    n = 0; EV(0,'A',A(9,0)); EV(1500,'F',0); EV(1600,'A',A(9,1)); EV(2000,'F',1);
    correr(ev, n, 2100);
    revisa_lcd("UPIICSA", 0, 1, "*** UPIICSA ***"); revisa_lcd("IPN", 0, 2, "  **** IPN **** ");
    revisa_lcd("nombre", 1, 1, "    LEONARDO"); revisa_lcd("materia", 1, 2, "Disp. Programab.");

    printf("10 animacion\n");
    n = 0; EV(0,'A',A(10,0)); EV(800 + 41 * 45, 'F', 0); EV(800 + 85 * 45 + 700 + 15 * 310, 'F', 1);
    correr(ev, n, 800 + 85 * 45 + 700 + 15 * 310 + 100);
    printf("     [%s] [%s]\n     [%s] [%s]\n", fotos[0].r1, fotos[0].r2, fotos[1].r1, fotos[1].r2);
    revisiones++;
    if (strncmp(fotos[0].r2, "########", 8) != 0) { printf("  FALLA barra a la mitad\n"); fallas++; }
    revisiones++;
    if (cgram[6 * 8 + 2] != 0x1F) { printf("  FALLA corazon en la CGRAM\n"); fallas++; }

    printf("11 marquesina\n");
    n = 0; EV(0,'A',A(11,0)); EV(800 + 120, 'F', 0); EV(800 + 120 + 255, 'F', 1);
    correr(ev, n, 800 + 120 + 255 + 50);
    revisa_lcd("inicio", 0, 1, "UPIICSA - IPN   ");
    revisa_lcd("un paso", 1, 1, "PIICSA - IPN   *");
    revisa_lcd("renglon 2", 0, 2, "  MARQUESINA");
    n = 0; EV(0,'A',A(11,2)); EV(800 + 120 + 255, 'F', 0);
    correr(ev, n, 800 + 120 + 300);
    revisiones++;
    if (fotos[0].r1[1] != 'U' || fotos[0].r1[0] != ' ') { printf("  FALLA al reves: [%s]\n", fotos[0].r1); fallas++; }

    printf("12 uso del teclado\n");
    n = 0; EV(0,'A',A(12,0)); n = teclear(ev, n, 1000, "7"); EV(1300,'F',0);
    n = teclear(ev, n, 1500, "#"); EV(1800,'F',1);
    n = teclear(ev, n, 2000, "A"); EV(2300,'F',2);
    correr(ev, n, 2400);
    revisa_d("7", 0, DIGITOS[7]); revisa_lcd("7", 0, 2, "Tecla 7  F3 C1");
    revisa_d("#", 1, DIGITOS[15]); revisa_lcd("#", 1, 2, "Tecla #  F4 C3");
    revisa_d("A", 2, DIGITOS[10]);
    /* tarjeta con resistencias a tierra en el teclado */
    hw_pulldown = 1;
    n = 0; EV(0,'A',A(12,0)); n = teclear(ev, n, 1000, "5"); EV(1300,'F',0);
    correr(ev, n, 1400);
    revisa_d("pull-down 5", 0, DIGITOS[5]); revisa_lcd("pull-down", 0, 2, "Tecla 5  F2 C2");
    hw_pulldown = 0;

    printf("13 LCD + teclado\n");
    n = 0; EV(0,'A',A(13,0)); n = teclear(ev, n, 1000, "122*5"); EV(2400,'F',0);
    EV(2500,'K',K('9')); EV(3200,'K',-1); EV(3400,'F',1);             /* dejada apretada */
    n = teclear(ev, n, 3500, "99"); EV(4100,'F',2);
    n = teclear(ev, n, 4200, "#"); EV(4500,'F',3);
    n = teclear(ev, n, 4600, "ABCD1234567890ABCD"); EV(9100,'F',4);
    correr(ev, n, 9400);
    revisa_lcd("125", 0, 2, "125"); revisa_lcd("veces", 0, 1, "Tecla:5 Veces:1");
    revisa_lcd("sostenida", 1, 2, "1259");
    revisa_lcd("veces 3", 2, 1, "Tecla:9 Veces:3");
    revisa_lcd("borrar", 3, 2, "");
    revisa_lcd("se recorre", 4, 2, "CD1234567890ABCD");

    printf("14 chapa\n");
    memset(eeprom, 0xFF, sizeof eeprom);
    n = 0; EV(0,'A',A(14,0)); EV(1000,'F',0);
    n = teclear(ev, n, 1100, "12"); EV(1700,'F',1);
    n = teclear(ev, n, 1800, "34#"); EV(2700,'F',2);
    n = teclear(ev, n, 2800, "A"); n = teclear(ev, n, 3100, "4321#"); EV(4400,'F',3);
    EV(5800,'F',4);
    n = teclear(ev, n, 6000, "B"); EV(6300,'F',5);
    n = teclear(ev, n, 6400, "1234#"); EV(7700,'F',6);
    n = teclear(ev, n, 9300, "4321#"); EV(10600,'F',7);
    correr(ev, n, 10700);
    revisa_lcd("cerrada", 0, 1, "CHAPA  CERRADA"); revisa_d("C", 0, 0x39);
    revisa_lcd("asteriscos", 1, 2, "Clave: **__");
    revisa_lcd("abierta", 2, 1, "ABIERTA :)"); revisa_d("A", 2, 0x77);
    revisa_lcd("nueva", 3, 1, "Clave guardada");
    revisa_lcd("abierta otra vez", 4, 1, "ABIERTA :)");
    revisa_lcd("cerrar", 5, 1, "CHAPA  CERRADA");
    revisa_lcd("la vieja ya no", 6, 1, "CLAVE MALA"); revisa_lcd("intentos", 6, 2, "Te quedan: 2");
    revisa_lcd("la nueva si", 7, 1, "ABIERTA :)");
    revisiones++;
    if (eeprom[0] != 0xA5 || memcmp(eeprom + 1, "4321", 4) != 0) { printf("  FALLA EEPROM\n"); fallas++; }
    /* se acuerda de la clave despues de "desconectar" (otra corrida) */
    n = 0; EV(0,'A',A(14,0)); n = teclear(ev, n, 1100, "4321#"); EV(2500,'F',0);
    correr(ev, n, 2600);
    revisa_lcd("clave guardada", 0, 1, "ABIERTA :)");
    /* tres malas: se bloquea */
    n = 0; EV(0,'A',A(14,0));
    n = teclear(ev, n, 1000, "0000#"); n = teclear(ev, n, 4000, "0000#"); n = teclear(ev, n, 7000, "0000#");
    EV(9800,'F',0); EV(14900,'F',1); EV(20200,'F',2);
    correr(ev, n, 20300);
    revisa_lcd("bloqueada", 0, 1, "BLOQUEADA"); revisa_lcd("cuenta", 0, 2, "Espera 10 s");
    revisa_lcd("cuenta 5", 1, 2, "Espera  5 s");
    revisa_lcd("vuelve", 2, 1, "CHAPA  CERRADA");

    printf("15 calculadora\n");
    n = 0; EV(0,'A',A(15,0));
    n = teclear(ev, n, 1000, "12C12#"); EV(2600,'F',0);
    n = teclear(ev, n, 2700, "A6#"); EV(3500,'F',1);
    n = teclear(ev, n, 3600, "*7D2#"); EV(4900,'F',2);
    n = teclear(ev, n, 5000, "*5D0#"); EV(6300,'F',3);
    n = teclear(ev, n, 6400, "*999999C999999#"); EV(10200,'F',4);
    n = teclear(ev, n, 10300, "*3B8#"); EV(11500,'F',5);
    n = teclear(ev, n, 11600, "*9AB4#"); EV(13000,'F',6);
    correr(ev, n, 13100);
    revisa_lcd("12x12", 0, 1, "12x12"); revisa_lcd("=144", 0, 2, "= 144");
    revisa_lcd("sigue", 1, 1, "144+6"); revisa_lcd("=150", 1, 2, "= 150");
    revisa_lcd("7/2", 2, 2, "= 3 r 1");
    revisa_lcd("/0", 3, 2, "= Error");
    revisa_lcd("grande", 4, 2, "= Muy grande");
    revisa_lcd("negativo", 5, 2, "= -5");
    revisa_lcd("cambia op", 6, 1, "9-4"); revisa_lcd("9-4", 6, 2, "= 5");

    printf("\n%d revisiones, %d fallas\n", revisiones, fallas);
    return fallas ? 1 : 0;
}
