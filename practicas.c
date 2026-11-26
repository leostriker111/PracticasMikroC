/* ##########################################################################
   PRACTICAS DEL CURSO EN UN SOLO PROGRAMA  -  PIC18F45K50  -  MikroC PRO for PIC
   ##########################################################################

   Un solo programa con todas las practicas de MikroC. Un SELECTOR (los 4
   switches de la derecha del puerto A: RA4 RA5 RA6 RA7) escoge cual corre.
   Los otros 4 switches (RA0 RA1 RA2 RA3) son los DATOS de cada practica.

   LISTA DE ENTREGA                          SELECTOR (RA4 RA5 RA6 RA7)
   ----------------------------------------  --------------------------
    0  Espera / menu                          0000
   Pasar de asm a C
    1  PORTA -> PORTD (la primera de clase)   0001
    2  Monitoreo de un bit (RA2)              0010
    3  Monitoreo de un bit: pares/impares     0011
    4  Monitoreo de palabra (0101)            0100
    5  Palabra en hexadecimal (0..F)          0101
    6  Palabra a letras (IPN-UPIICSA)         0110
    7  Subrutinas de tiempo (1ms 1s 3s T0)    0111
    8  Tabla (0..F, HOLA-IPN, nombre)         1000
   LCD
    9  Mostrar un mensaje                     1001
   10  Animacion (caracteres propios)         1010
   11  Mensaje marquesina                     1011
   Teclado
   12  Uso del teclado                        1100
   13  LCD + teclado                          1101
   Aplicacion
   14  Chapa electronica con clave            1110
   15  Calculadora                            1111

   El selector se lee DE IZQUIERDA A DERECHA, como lo ves en la tarjeta:
   RA4 vale 8, RA5 vale 4, RA6 vale 2, RA7 vale 1.   Ej: 0111 = practica 7.

   CONEXIONES (la hoja "Pines" del Excel trae el detalle con dibujo)
     RA0..RA3  switches de datos            (cada uno con su resistencia, ON = 1)
     RA4..RA7  switches del selector        (igual, ON = 1)
     RB0..RB3  teclado: columnas C1..C4     (entradas, pull-ups internos)
     RB4..RB7  teclado: filas R1..R4        (salidas)
     RC0 RC1 RC6 RC7  LCD D4 D5 D6 D7       (el LCD va en modo de 4 bits)
     RE0 RE1   LCD RS y EN                  (RW del LCD va a tierra)
     RD0..RD7  barra de LEDs + display de 7 segmentos (a b c d e f g punto)
     RE3       MCLR (pata 1, la usa el PICkit)
     RB6 RB7   tambien son PGC/PGD del PICkit: por eso ahi van FILAS
               (salidas) y no columnas; ver teclado_inicio().

   PROYECTO DE MIKROC
     Microcontrolador P18F45K50, reloj 16 MHz (16.000000).
     Librerias que usa (Library Manager): Lcd y Lcd_Constants. Nada mas;
     el teclado y la EEPROM van hechos a mano aqui abajo.
     La palabra de configuracion (oscilador interno, sin perro guardian,
     LVP apagado...) la pone cargar.bat dentro del .hex. Si grabas desde
     otro programa, ponla en Project > Edit Project igual que en LEEME.md.

   VERSION DEMO DE MIKROC
     La demo solo compila programas chicos (2K palabras). Si al compilar
     sale "Demo limit", cambia PARTE (abajo) a 1, 2, 3 o 4 y graba por partes.
   ########################################################################## */


/* ============================ AJUSTES ==================================== */

#define PARTE 0              /* 0 = todo junto
                                1 = solo "pasar de asm a C" (practicas 1 a 8)
                                2 = solo LCD (9 a 11)
                                3 = solo teclado (12 y 13)
                                4 = solo aplicaciones (14 y 15)              */

#define SWITCH_ON_ES_1   1   /* 1: switch en ON = 1 (switches de la tarjeta).
                                0: switches sueltos a tierra con pull-up.    */
#define INVERTIR_SALIDA  0   /* 1 si tus LEDs prenden con 0 (todo sale al reves) */

#define VELOCIDAD_MARQUESINA  250   /* ms por paso en la marquesina */

const char NOMBRE_LCD[] = "LEONARDO";            /* practica 9, renglon 2  */
const char CLAVE_DE_FABRICA[] = "1234";          /* practica 14            */


/* ===================== CONEXIONES DEL LCD (MikroC) ======================= */
/* Las mismas de los programas de clase. Para escribir en un pin de salida en
   la familia PIC18 se usa el LAT (latch), no el PORT.                       */
sbit LCD_RS at LATE0_bit;
sbit LCD_EN at LATE1_bit;
sbit LCD_D4 at LATC0_bit;
sbit LCD_D5 at LATC1_bit;
sbit LCD_D6 at LATC6_bit;
sbit LCD_D7 at LATC7_bit;

sbit LCD_RS_Direction at TRISE0_bit;
sbit LCD_EN_Direction at TRISE1_bit;
sbit LCD_D4_Direction at TRISC0_bit;
sbit LCD_D5_Direction at TRISC1_bit;
sbit LCD_D6_Direction at TRISC6_bit;
sbit LCD_D7_Direction at TRISC7_bit;


/* ============================== TABLAS =================================== */

/* Display de 7 segmentos de catodo comun: bit0 = a ... bit6 = g, bit7 = punto.
   Es la tabla DIGITOS de CONTADOR.c de clase.                              */
const char DIGITOS[16] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,   /* 0 1 2 3 4 5 6 7 */
    0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71    /* 8 9 A b C d E F */
};

/* "HOLA-IPn" (la de "Contador y tabla" de ensamblador) */
const char TABLA_HOLA[8] = { 0x76, 0x3F, 0x38, 0x77, 0x40, 0x30, 0x73, 0x54 };

/* "LEOnArdO" */
const char TABLA_NOMBRE[8] = { 0x38, 0x79, 0x3F, 0x54, 0x77, 0x50, 0x5E, 0x3F };

/* "IPN-UPIICSA": la de IPNU.c de clase, una letra por cada valor de RA3..RA0 */
const char TABLA_IPN[11] = { 0x06, 0x73, 0x54, 0x40, 0x3E, 0x73,
                             0x06, 0x06, 0x39, 0x6D, 0x77 };
const char LETRAS_IPN[] = "IPN-UPIICSA";

/* Teclado 4x4 hexadecimal, como se ve de frente:
       C1  C2  C3  C4
   R1   1   2   3   A
   R2   4   5   6   B
   R3   7   8   9   C
   R4   *   0   #   D                                                       */
const char MAPA_TECLAS[16] = { '1','2','3','A',
                               '4','5','6','B',
                               '7','8','9','C',
                               '*','0','#','D' };

/* Titulo (renglon 1) y pista (renglon 2) que salen en el LCD al entrar a
   cada practica. 16 letras maximo, sin acentos (el LCD no los tiene).       */
const char TITULOS[16][17] = {
    "PRACTICAS MIKROC",
    "P01 PORTA>PORTD ",
    "P02 Monitor 1bit",
    "P03 Pares/Impar ",
    "P04 Palabra 0101",
    "P05 Palabra HEX ",
    "P06 IPN-UPIICSA ",
    "P07 Subrutinas  ",
    "P08 Tabla       ",
    "P09 Mensaje LCD ",
    "P10 Animacion   ",
    "P11 Marquesina  ",
    "P12 Uso teclado ",
    "P13 LCD+Teclado ",
    "P14 Chapa clave ",
    "P15 Calculadora "
};
const char PISTAS[16][17] = {
    "Elige RA4-RA7   ",
    "Mueve RA0-RA3   ",
    "Mueve RA2       ",
    "Mueve RA0       ",
    "RA0-RA3 = 1010  ",
    "Mueve RA0-RA3   ",
    "Mueve RA0-RA3   ",
    "RA0 RA1 = modo  ",
    "RA0 RA1 = modo  ",
    "RA0 = otro msj  ",
    "Nomas mira      ",
    "RA0 vel RA1 dir ",
    "Pica una tecla  ",
    "Escribe algo    ",
    "Clave + #       ",
    "A+ B- Cx D/ #=  "
};

const char MARQUESINA[] =
    "UPIICSA - IPN   *   Dispositivos Programables   *   "
    "Practica de marquesina en MikroC con el PIC18F45K50   *   ";

/* Textos sueltos. En MikroC conviene tenerlos como const (van a la memoria
   de programa) para no gastar la RAM; lcd_linea() los copia al renglon.     */
const char T_VACIO[]      = "";
const char T_NO_PARTE[]   = "No va en PARTE  ";
const char T_RA_1[]       = "bit = 1 -> ";
const char T_RA_0[]       = "bit = 0 -> ";
const char T_CORRECTA[]   = "1010 SI -> 0x59";
const char T_INCORRECTA[] = "otra   -> 0x11";
const char T_MODO_1MS[]   = "Modo: 1 ms      ";
const char T_MODO_1S[]    = "Modo: 1 s       ";
const char T_MODO_3S[]    = "Modo: 3 s       ";
const char T_MODO_T0[]    = "Modo: Timer0 1 s";
const char T_HEX_ASC[]    = "0..F subiendo   ";
const char T_HOLA[]       = "HOLA-IPn        ";
const char T_NOMBRE[]     = "LEOnArdO        ";
const char T_HEX_DESC[]   = "F..0 bajando    ";
const char T_UPIICSA[]    = "*** UPIICSA *** ";
const char T_IPN[]        = "  **** IPN ****  ";
const char T_DISP[]       = "Disp. Programab.";
const char T_CARGANDO[]   = "Cargando...     ";
const char T_CAMINA[]     = "A caminar!      ";
const char T_FIN[]        = "   Listo :)     ";
const char T_MARQ_2[]     = "  MARQUESINA    ";
const char T_APRIETA[]    = "Aprieta tecla   ";
const char T_CHAPA[]      = "CHAPA  CERRADA  ";
const char T_ABIERTA[]    = "ABIERTA :)      ";
const char T_OPC_ABIERTA[]= "B=cerrar A=clave";
const char T_CLAVE_MALA[] = "CLAVE MALA      ";
const char T_BLOQUEADA[]  = "BLOQUEADA       ";
const char T_NUEVA[]      = "Clave nueva:    ";
const char T_GUARDADA[]   = "Clave guardada  ";
const char T_ERROR[]      = "= Error         ";
const char T_GRANDE[]     = "= Muy grande    ";


/* ======================= VARIABLES GLOBALES ============================== */
char practica;        /* la practica que esta corriendo (0..15)              */
char buf[17];         /* un renglon del LCD armado en RAM (16 letras + fin)  */
char teclado_pullup;  /* 1: tecla apretada = 0 (pull-ups).  0: = 1 (pull-downs) */
char tecla_ant;       /* la ultima tecla entregada, para que cuente una vez  */
char tecla_suelta;    /* cuantas lecturas seguidas lleva sin tecla           */


/* ######################################################################### */
/*                      RUTINAS QUE USAN TODAS                                */
/* ######################################################################### */

/* mostrar: saca un byte a la barra y al display (puerto D).
   Existe para que TODAS las practicas escriban por un solo lugar: si tus LEDs
   resultan prender con 0, se cambia INVERTIR_SALIDA y se arreglan todas.    */
void mostrar(char x) {
#if INVERTIR_SALIDA
    LATD = ~x;
#else
    LATD = x;
#endif
}

/* leer_switches: los 8 switches de A con ON = 1.
   Si los switches fueran a tierra (ON = 0) se voltean aqui y nada mas cambia. */
char leer_switches(void) {
    char a;
    a = PORTA;
#if SWITCH_ON_ES_1 == 0
    a = ~a;
#endif
    return a;
}

/* datos: RA3..RA0 como numero, igual que en clase (PORTA & 0x0F).
   RA0 es el bit de menos peso: en la tarjeta (RA0 a la izquierda) el patron
   "0101" de la presentacion se ve como 1 0 1 0.                             */
char datos(void) {
    return leer_switches() & 0x0F;
}

/* leer_selector: RA4..RA7 volteados para que se lea de izquierda a derecha
   como lo ves en la tarjeta (RA4 vale 8 ... RA7 vale 1). Sin voltearlo, 0001
   en la tarjeta seria la practica 8 y nadie le atinaria.                    */
char leer_selector(void) {
    char a, s;
    a = leer_switches();
    s = 0;
    if (a & 0x10) s = s | 8;     /* RA4 */
    if (a & 0x20) s = s | 4;     /* RA5 */
    if (a & 0x40) s = s | 2;     /* RA6 */
    if (a & 0x80) s = s | 1;     /* RA7 */
    return s;
}

/* selector_estable: lee el selector hasta que dos lecturas separadas 20 ms
   coincidan. Sin esto, mientras mueves los switches (rebote, o un switch a
   medio camino) el programa brincaria a practicas que no pediste.          */
char selector_estable(void) {
    char s1, s2;
    do {
        s1 = leer_selector();
        Delay_ms(20);
        s2 = leer_selector();
    } while (s1 != s2);
    return s1;
}

/* sigue: 1 mientras el selector siga en esta practica.
   Cada practica es un ciclo infinito (como en clase) pero pregunta esto en
   cada vuelta; si no, para cambiar de practica habria que resetear el PIC.
   Pide dos lecturas distintas seguidas para no salirse por un rebote.      */
char sigue(void) {
    if (leer_selector() == practica) return 1;
    Delay_ms(20);
    if (leer_selector() == practica) return 1;
    return 0;
}

/* esperar: espera "ms" milisegundos revisando el selector.
   Regresa 0 si lo movieron (para que la practica se salga luego luego) y 1
   si se completo la espera. Delay_ms solo acepta constantes, por eso se
   arma con Delay_ms(1) en un ciclo.                                         */
char esperar(unsigned int ms) {
    unsigned int i;
    for (i = 0; i < ms; i++) {
        Delay_ms(1);
        if ((i & 15) == 0) {
            if (!sigue()) return 0;
        }
    }
    return 1;
}


/* ---------------------------- LCD ---------------------------------------- */

/* lcd_linea: escribe un texto de la memoria de programa en un renglon,
   rellenando con espacios hasta 16. Asi se borra lo que habia antes sin usar
   _LCD_CLEAR (que hace parpadear toda la pantalla).                        */
void lcd_linea(char fila, const char *t) {
    char i;
    i = 0;
    while (i < 16 && t[i] != 0) { buf[i] = t[i]; i++; }
    while (i < 16) { buf[i] = ' '; i++; }
    buf[16] = 0;
    Lcd_Out(fila, 1, buf);
}

/* lcd_linea_ram: lo mismo pero para textos armados en RAM (numeros, lo que
   escribes con el teclado). En el PIC18 la RAM y la memoria de programa son
   dos cosas distintas y MikroC no deja mezclar los apuntadores.            */
void lcd_linea_ram(char fila, char *t) {
    char i;
    char r[17];
    i = 0;
    while (i < 16 && t[i] != 0) { r[i] = t[i]; i++; }
    while (i < 16) { r[i] = ' '; i++; }
    r[16] = 0;
    Lcd_Out(fila, 1, r);
}

/* titulo: nombre y pista de la practica actual. Es lo primero que ves al
   mover el selector: confirma que el PIC leyo bien el numero.              */
void titulo(void) {
    lcd_linea(1, TITULOS[practica]);
    lcd_linea(2, PISTAS[practica]);
}

/* lcd_caracter: guarda un dibujo de 5x8 en el espacio "n" (0..7) de la CGRAM
   del LCD. Despues se pinta con Lcd_Chr(fila, col, n). Es lo de CustomChar()
   de clase, pero con el numero de espacio como parametro.                  */
void lcd_caracter(char n, const char *dibujo) {
    char i;
    Lcd_Cmd(64 + n * 8);
    for (i = 0; i < 8; i++) Lcd_Chr_Cp(dibujo[i]);
    Lcd_Cmd(_LCD_RETURN_HOME);
}

/* numero_a_texto: entero con signo a texto decimal ("-125"). Hace lo de
   LongToStr de la libreria pero sin rellenar con espacios a la izquierda y
   sin tener que agregar la libreria Conversions.                           */
void numero_a_texto(long n, char *t) {
    char tmp[12];
    char i, j;
    i = 0;
    j = 0;
    if (n < 0) { t[j] = '-'; j++; n = -n; }
    do {
        tmp[i] = '0' + (n % 10);
        i++;
        n = n / 10;
    } while (n > 0);
    while (i > 0) { i--; t[j] = tmp[i]; j++; }
    t[j] = 0;
}

/* hex_letra: 0..15 a '0'..'9','A'..'F' para ensenar valores en el LCD. */
char hex_letra(char v) {
    if (v < 10) return '0' + v;
    return 'A' + v - 10;
}

/* binario4: escribe RA0 RA1 RA2 RA3 como los ves en la tarjeta ("1010"). */
void binario4(char v, char *t) {
    char i;
    for (i = 0; i < 4; i++) {
        if (v & (1 << i)) t[i] = '1'; else t[i] = '0';
    }
    t[4] = 0;
}


/* ---------------------------- TECLADO ------------------------------------ */

/* teclado_inicio: deja el puerto B listo para el teclado.
   Columnas (RB0..RB3) de entrada con los pull-ups internos del PIC: sin
   ellos, sin tecla apretada los pines quedan flotando y salen teclas
   fantasma. Filas (RB4..RB7) de entrada mientras no se escanea.
   Luego se fija si las columnas se leen en 1: si se leen en 0 es que la
   tarjeta trae resistencias a tierra (pull-down) en esos pines, y entonces
   se escanea al reves (fila en 1, tecla apretada = 1). Asi jala en el
   protoboard y en la entrenadora sin cambiar el codigo.
   Las filas van en RB4..RB7 porque RB6/RB7 son PGC/PGD: el PICkit tiene
   resistencias a tierra ahi y le ganaria a un pull-up interno; como salidas
   no le pasa nada.                                                         */
void teclado_inicio(void) {
    LATB = 0x00;
    TRISB = 0xFF;
    WPUB = 0x0F;                 /* pull-ups solo en RB0..RB3            */
    INTCON2 = INTCON2 & 0x7F;    /* RBPU = 0: enciende los pull-ups de B */
    Delay_ms(5);
    if ((PORTB & 0x0F) == 0x00) {
        teclado_pullup = 0;
        WPUB = 0x00;
    } else {
        teclado_pullup = 1;
    }
    tecla_ant = 0;
    tecla_suelta = 0;
}

/* escanear: regresa la tecla apretada en este instante ('0'..'9','A'..'D',
   '*','#') o 0 si no hay ninguna. Activa UNA fila a la vez (las otras se
   quedan como entrada, asi dos teclas apretadas no ponen dos salidas en
   corto) y lee las 4 columnas.                                             */
char escanear(void) {
    char fila, cols, col;
    for (fila = 0; fila < 4; fila++) {
        TRISB = 0xFF;
        if (teclado_pullup) LATB = 0x00; else LATB = 0xF0;
        TRISB = ~(0x10 << fila);         /* solo esta fila como salida */
        Delay_us(30);
        cols = PORTB & 0x0F;
        TRISB = 0xFF;
        if (teclado_pullup) cols = (~cols) & 0x0F;
        if (cols != 0) {
            col = 0;
            while ((cols & 1) == 0) { cols = cols >> 1; col++; }
            return MAPA_TECLAS[fila * 4 + col];
        }
    }
    return 0;
}

/* tecla: entrega cada tecla UNA sola vez por apreton (como Keypad_Key_Click
   de la libreria, pero sin quedarse esperando: asi la practica puede seguir
   revisando el selector). Confirma con una segunda lectura a los 20 ms y
   pide 3 lecturas seguidas sin tecla antes de aceptar otra, porque al
   soltar el boton tambien rebota.                                          */
char tecla(void) {
    char k;
    k = escanear();
    if (k == 0) {
        if (tecla_suelta < 3) tecla_suelta++;
        if (tecla_suelta >= 3) tecla_ant = 0;
        return 0;
    }
    tecla_suelta = 0;
    if (k == tecla_ant) return 0;
    Delay_ms(20);
    if (escanear() != k) return 0;
    tecla_ant = k;
    return k;
}

/* valor_tecla: '0'..'9' -> 0..9, 'A'..'D' -> 10..13, '*' -> 14, '#' -> 15.
   Sirve para ensenar cualquier tecla en un solo digito del display.        */
char valor_tecla(char k) {
    if (k >= '0' && k <= '9') return k - '0';
    if (k >= 'A' && k <= 'D') return k - 'A' + 10;
    if (k == '*') return 14;
    return 15;
}


/* ---------------------------- EEPROM ------------------------------------- */
/* La chapa guarda su clave en la EEPROM del PIC para que no se olvide al
   desconectarlo. Va con registros directos (como en ensamblador) para no
   depender de otra libreria. La secuencia 0x55 / 0xAA es obligatoria: es el
   "candado" del PIC para que un programa loco no escriba la EEPROM.        */
char ee_leer(char direccion) {
    EEADR = direccion;
    EECON1 = 0x00;            /* EEPGD = 0, CFGS = 0: la EEPROM de datos */
    EECON1 = EECON1 | 0x01;   /* RD = 1: lee                             */
    return EEDATA;
}

void ee_escribir(char direccion, char dato) {
    EEADR = direccion;
    EEDATA = dato;
    EECON1 = 0x04;            /* WREN = 1: permite escribir              */
    INTCON = INTCON & 0x7F;   /* GIE = 0 durante la secuencia            */
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1 = EECON1 | 0x02;   /* WR = 1: arranca la escritura (~4 ms)   */
    while (EECON1 & 0x02) { }
    EECON1 = EECON1 & 0xFB;   /* WREN = 0                                */
}


/* ######################################################################### */
/*                              PRACTICAS                                     */
/* ######################################################################### */

/* --- 0. Espera: un guion en el display. Si lo ves, el PIC arranco bien. --- */
void p00_espera(void) {
    mostrar(0x40);
    while (sigue()) {
        if (!esperar(50)) return;
    }
}

/* Practica que no entra en la PARTE que se compilo (ver AJUSTES). */
void p_no_incluida(void) {
    char t[17];
    char i;
    for (i = 0; T_NO_PARTE[i] != 0; i++) t[i] = T_NO_PARTE[i];
    t[14] = '0' + PARTE;
    t[15] = 0;
    lcd_linea_ram(2, t);
    mostrar(0x79);                /* una E de "error" en el display */
    while (sigue()) {
        if (!esperar(50)) return;
    }
}


#if PARTE == 0 || PARTE == 1
/* =================== PASAR DE ASM A C (practicas 1 a 8) =================== */

/* --- 1. PORTA -> PORTD: "lo que tengo en el puerto A lo veo en el D".
   En clase era PORTB = PORTA; aqui la barra esta en el D. Los 4 LEDs de
   arriba copian el selector, los 4 de abajo los switches de datos.        */
void p01_puertos(void) {
    while (sigue()) {
        mostrar(leer_switches());
    }
}

/* --- 2. Monitoreo de un bit: "si RA2 = 1 -> 0110 1010, si es 0 -> 0000 0001".
   En ensamblador era BTFSS PORTA,2 y dos BRA; en C es un if.              */
void p02_un_bit(void) {
    char sw, ant;
    ant = 2;
    while (sigue()) {
        sw = leer_switches();
        if (sw & 0x04) {                 /* PORTA.F2 == 1 */
            mostrar(0x6A);
        } else {
            mostrar(0x01);
        }
        if (((sw >> 2) & 1) != ant) {    /* el LCD solo se reescribe si cambio */
            ant = (sw >> 2) & 1;
            if (ant) {
                lcd_linea(2, T_RA_1);
                Lcd_Out(2, 12, "0x6A");
            } else {
                lcd_linea(2, T_RA_0);
                Lcd_Out(2, 12, "0x01");
            }
        }
    }
}

/* --- 3. Modificacion: pares e impares con RA0 (ParesImpares.c de clase).
   RA0 = 1 -> 1010 1010 (0xAA)       RA0 = 0 -> 0101 0101 (0x55)          */
void p03_pares_impares(void) {
    char sw, ant;
    ant = 2;
    while (sigue()) {
        sw = leer_switches();
        if (sw & 0x01) {                 /* PORTA.F0 == 1 */
            mostrar(0xAA);
        } else {
            mostrar(0x55);
        }
        if ((sw & 1) != ant) {
            ant = sw & 1;
            if (ant) {
                lcd_linea(2, T_RA_1);
                Lcd_Out(2, 12, "0xAA");
            } else {
                lcd_linea(2, T_RA_0);
                Lcd_Out(2, 12, "0x55");
            }
        }
    }
}

/* --- 4. Monitoreo de palabra ("preguntando bit a bit", Pregunta4Bits.c):
   si RA3..RA0 = 0101 -> 0101 1001 (0x59); si es otra -> 0001 0001 (0x11).
   En la tarjeta (RA0 a la izquierda) el 0101 se pone como 1 0 1 0.
   Cada condicion del && es una pregunta del metodo arbol de ensamblador.  */
void p04_palabra(void) {
    char sw, ant, ok;
    ant = 2;
    while (sigue()) {
        sw = leer_switches();
        if ((sw & 0x01) && !(sw & 0x02) && (sw & 0x04) && !(sw & 0x08)) {
            mostrar(0x59);
            ok = 1;
        } else {
            mostrar(0x11);
            ok = 0;
        }
        if (ok != ant) {
            ant = ok;
            if (ok) lcd_linea(2, T_CORRECTA); else lcd_linea(2, T_INCORRECTA);
        }
    }
}

/* --- 5. Palabra en hexadecimal: el numero de RA3..RA0 sale en el display
   como 0..F, con la tabla DIGITOS. En el LCD se ve el patron y su valor.  */
void p05_palabra_hex(void) {
    char v, ant;
    char t[17];
    ant = 0xFF;
    while (sigue()) {
        v = datos();
        mostrar(DIGITOS[v]);
        if (v != ant) {
            ant = v;
            binario4(v, t);              /* "RA0 RA1 RA2 RA3" como en la tarjeta */
            t[4] = ' '; t[5] = '='; t[6] = ' ';
            t[7] = hex_letra(v);
            t[8] = ' '; t[9] = '(';
            if (v >= 10) { t[10] = '1'; t[11] = '0' + v - 10; t[12] = ')'; t[13] = 0; }
            else         { t[10] = '0' + v; t[11] = ')'; t[12] = 0; }
            lcd_linea_ram(2, t);
        }
    }
}

/* --- 6. Palabra a letras: IPN-UPIICSA segun el valor de RA3..RA0 (IPNU.c
   de clase). 0 = I, 1 = P, 2 = N, 3 = -, 4 = U ... 10 = A; de 11 a 15 se
   apaga. En clase era una escalera de if / else if; aqui es la tabla.      */
void p06_palabra_letras(void) {
    char v, ant;
    char t[17];
    ant = 0xFF;
    while (sigue()) {
        v = datos();
        if (v <= 10) mostrar(TABLA_IPN[v]); else mostrar(0x00);
        if (v != ant) {
            ant = v;
            binario4(v, t);
            t[4] = ' '; t[5] = '-'; t[6] = '>'; t[7] = ' ';
            if (v <= 10) t[8] = LETRAS_IPN[v]; else t[8] = '.';
            t[9] = 0;
            lcd_linea_ram(2, t);
        }
    }
}

/* --- 7. Subrutinas de tiempo. Un contador en binario en la barra; RA0 y RA1
   escogen la subrutina de espera:
       RA0 RA1 en la tarjeta:  0 0 -> 1 ms   1 0 -> 1 s   0 1 -> 3 s
                               1 1 -> 1 s con el Timer0
   En ensamblador eran ciclos anidados (un registro no pasa de 255); en C la
   subrutina de 1 s llama 1000 veces a la de 1 ms, y la de 3 s tres veces a
   la de 1 s: el mismo anidado, pero se lee de corrido.                    */
void retardo_1ms(void) {
    Delay_ms(1);
}

void retardo_1s(void) {
    unsigned int i;
    for (i = 0; i < 1000; i++) retardo_1ms();
}

void retardo_3s(void) {
    retardo_1s();
    retardo_1s();
    retardo_1s();
}

/* Timer0 de 16 bits, reloj interno, prescaler 1:128.
   Un paso = 4 / 16 MHz * 128 = 32 us.  1 s / 32 us = 31250 pasos.
   TMR0 = 65536 - 31250 = 34286 = 0x85EE (la formula de la presentacion).
   T0CON = 1000 0110: TMR0ON = 1, 16 bits, reloj interno, prescaler 1:128.  */
void t0_1s(void) {
    T0CON = 0x86;
    TMR0H = 0x85;                /* primero la parte alta: entra junto con la baja */
    TMR0L = 0xEE;
    INTCON = INTCON & 0xFB;      /* TMR0IF = 0 */
    while ((INTCON & 0x04) == 0) { }   /* espera a que se desborde */
    INTCON = INTCON & 0xFB;
}

void p07_subrutinas(void) {
    char cuenta, modo, ant;
    cuenta = 0;
    ant = 0xFF;
    while (sigue()) {
        mostrar(cuenta);
        modo = datos() & 0x03;
        if (modo != ant) {
            ant = modo;
            if (modo == 0) lcd_linea(2, T_MODO_1MS);
            if (modo == 1) lcd_linea(2, T_MODO_1S);
            if (modo == 2) lcd_linea(2, T_MODO_3S);
            if (modo == 3) lcd_linea(2, T_MODO_T0);
        }
        if (modo == 0) retardo_1ms();
        if (modo == 1) retardo_1s();
        if (modo == 2) retardo_3s();
        if (modo == 3) t0_1s();
        cuenta++;
    }
}

/* --- 8. Tabla: un contador recorre una tabla y lo que sale se ve en el
   display (CONTADOR.c y "Contador y tabla" de clase). RA0 y RA1 escogen:
       RA0 RA1:  0 0 -> 0..F subiendo     1 0 -> HOLA-IPn
                 0 1 -> LEOnArdO          1 1 -> F..0 bajando              */
void p08_tabla(void) {
    char modo, ant, i, n;
    ant = 0xFF;
    i = 0;
    while (sigue()) {
        modo = datos() & 0x03;
        if (modo != ant) {
            ant = modo;
            i = 0;
            if (modo == 0) lcd_linea(2, T_HEX_ASC);
            if (modo == 1) lcd_linea(2, T_HOLA);
            if (modo == 2) lcd_linea(2, T_NOMBRE);
            if (modo == 3) lcd_linea(2, T_HEX_DESC);
        }
        if (modo == 0) { mostrar(DIGITOS[i]);      n = 16; }
        if (modo == 1) { mostrar(TABLA_HOLA[i]);   n = 8;  }
        if (modo == 2) { mostrar(TABLA_NOMBRE[i]); n = 8;  }
        if (modo == 3) { mostrar(DIGITOS[15 - i]); n = 16; }
        if (!esperar(700)) return;
        i++;
        if (i >= n) i = 0;
    }
}
#endif


#if PARTE == 0 || PARTE == 2
/* ========================= LCD (practicas 9 a 11) ========================= */

/* --- 9. Mostrar un mensaje. RA0 = 0: el de clase (UPIICSA / IPN).
   RA0 = 1: tu nombre y la materia. Los textos estan hasta arriba.        */
void p09_mensaje(void) {
    char modo, ant;
    char t[17];
    char i, j, largo;
    ant = 0xFF;
    Lcd_Cmd(_LCD_CLEAR);
    while (sigue()) {
        modo = datos() & 0x01;
        if (modo != ant) {
            ant = modo;
            if (modo == 0) {
                lcd_linea(1, T_UPIICSA);
                lcd_linea(2, T_IPN);
            } else {
                /* el nombre centrado en el renglon 1 */
                largo = 0;
                while (NOMBRE_LCD[largo] != 0 && largo < 16) largo++;
                for (i = 0; i < 16; i++) t[i] = ' ';
                j = (16 - largo) / 2;
                for (i = 0; i < largo; i++) t[j + i] = NOMBRE_LCD[i];
                t[16] = 0;
                lcd_linea_ram(1, t);
                lcd_linea(2, T_DISP);
            }
        }
        if (!esperar(30)) return;
    }
}

/* --- 10. Animacion con caracteres propios (CGRAM, como el PAC-MAN de clase).
   Primero una barra de carga que se llena de columna en columna (5 por
   cuadrito), luego un monito que camina hasta un corazon. Se repite.      */
const char DIB_BARRA1[8] = { 0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10 };
const char DIB_BARRA2[8] = { 0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18 };
const char DIB_BARRA3[8] = { 0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C };
const char DIB_BARRA4[8] = { 0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E };
const char DIB_MONO_A[8] = { 0x0E,0x0E,0x04,0x0E,0x15,0x04,0x0A,0x11 };  /* piernas abiertas */
const char DIB_MONO_B[8] = { 0x0E,0x0E,0x04,0x0E,0x15,0x04,0x0A,0x0A };  /* piernas juntas   */
const char DIB_CORAZON[8]= { 0x00,0x0A,0x1F,0x1F,0x0E,0x04,0x00,0x00 };
const char DIB_BRAZOS[8] = { 0x0E,0x0E,0x15,0x0E,0x04,0x04,0x0A,0x11 };  /* brazos arriba   */

void p10_animacion(void) {
    char paso, cuadro, resto, col, i;
    char t[17];
    Lcd_Cmd(_LCD_CLEAR);
    lcd_caracter(0, DIB_BARRA1);
    lcd_caracter(1, DIB_BARRA2);
    lcd_caracter(2, DIB_BARRA3);
    lcd_caracter(3, DIB_BARRA4);
    lcd_caracter(4, DIB_MONO_A);
    lcd_caracter(5, DIB_MONO_B);
    lcd_caracter(6, DIB_CORAZON);
    lcd_caracter(7, DIB_BRAZOS);
    while (sigue()) {
        /* 1) barra de carga: 16 cuadritos x 5 columnas = 80 pasos */
        Lcd_Cmd(_LCD_CLEAR);
        lcd_linea(1, T_CARGANDO);
        for (paso = 1; paso <= 80; paso++) {
            cuadro = (paso - 1) / 5;          /* cuadrito que se esta llenando */
            resto = paso - cuadro * 5;        /* columnas llenas en el (1..5)  */
            if (resto == 5) Lcd_Chr(2, cuadro + 1, 255);   /* 255 = cuadro lleno de fabrica */
            else            Lcd_Chr(2, cuadro + 1, resto - 1);
            if ((paso % 8) == 0) {            /* el porcentaje cada 10 % */
                numero_a_texto((long)paso * 100 / 80, t);
                i = 0; while (t[i] != 0) i++;
                t[i] = '%'; t[i + 1] = 0;
                Lcd_Out(1, 13, t);
            }
            if (!esperar(40)) return;
        }
        if (!esperar(500)) return;

        /* 2) el monito camina por el renglon 2 hasta el corazon */
        Lcd_Cmd(_LCD_CLEAR);
        lcd_linea(1, T_CAMINA);
        Lcd_Chr(2, 16, 6);
        for (col = 1; col <= 15; col++) {
            if (col > 1) Lcd_Chr(2, col - 1, ' ');
            if (col & 1) Lcd_Chr(2, col, 4); else Lcd_Chr(2, col, 5);
            if (!esperar(300)) return;
        }
        /* 3) llego: brinca de gusto tres veces */
        lcd_linea(1, T_FIN);
        for (i = 0; i < 3; i++) {
            Lcd_Chr(2, 15, 7);
            if (!esperar(300)) return;
            Lcd_Chr(2, 15, 4);
            if (!esperar(300)) return;
        }
        if (!esperar(800)) return;
    }
}

/* --- 11. Mensaje marquesina: un texto mas largo que la pantalla pasa por el
   renglon 1 como letrero de tienda. Se arma una "ventana" de 16 letras que
   empieza cada vez una letra mas adelante; al llegar al final da la vuelta.
   (Con _LCD_SHIFT_LEFT de clase se recorre toda la pantalla, pero solo
   caben 40 letras por renglon y el renglon 2 se mueve tambien.)
   RA0 = 1: mas rapido.   RA1 = 1: al reves (de izquierda a derecha).      */
void p11_marquesina(void) {
    unsigned int largo, inicio, k;
    char i, sw;
    Lcd_Cmd(_LCD_CLEAR);
    lcd_linea(2, T_MARQ_2);
    largo = 0;
    while (MARQUESINA[largo] != 0) largo++;
    inicio = 0;
    while (sigue()) {
        for (i = 0; i < 16; i++) {
            k = inicio + i;
            if (k >= largo) k = k - largo;
            buf[i] = MARQUESINA[k];
        }
        buf[16] = 0;
        Lcd_Out(1, 1, buf);
        sw = datos();
        if (sw & 0x02) {                  /* RA1: al reves */
            if (inicio == 0) inicio = largo - 1; else inicio--;
        } else {
            inicio++;
            if (inicio >= largo) inicio = 0;
        }
        if (sw & 0x01) {                  /* RA0: rapido */
            if (!esperar(VELOCIDAD_MARQUESINA / 3)) return;
        } else {
            if (!esperar(VELOCIDAD_MARQUESINA)) return;
        }
    }
}
#endif


#if PARTE == 0 || PARTE == 3
/* ====================== TECLADO (practicas 12 y 13) ======================= */

/* --- 12. Uso del teclado: la tecla que aprietas sale en el display (0..9,
   A..D, * = E, # = F) y en el LCD con su fila y columna. Si al apretar el 1
   sale otra cosa, el LCD te dice que fila/columna leyo el PIC: con eso se
   ve si un cable del teclado quedo cruzado.                               */
void p12_uso_teclado(void) {
    char k, n, fila, col;
    char t[17];
    mostrar(0x40);
    while (sigue()) {
        k = tecla();
        if (k != 0) {
            mostrar(DIGITOS[valor_tecla(k)]);
            for (n = 0; n < 16; n++) if (MAPA_TECLAS[n] == k) break;
            fila = n / 4 + 1;
            col = n % 4 + 1;
            t[0] = 'T'; t[1] = 'e'; t[2] = 'c'; t[3] = 'l'; t[4] = 'a'; t[5] = ' ';
            t[6] = k; t[7] = ' '; t[8] = ' ';
            t[9] = 'F'; t[10] = '0' + fila; t[11] = ' ';
            t[12] = 'C'; t[13] = '0' + col; t[14] = 0;
            lcd_linea_ram(2, t);
        }
        if (!esperar(10)) return;
    }
}

/* --- 13. LCD + teclado: lo que escribes aparece en el renglon 2 (se recorre
   cuando pasa de 16). En el renglon 1 la ultima tecla y cuantas veces
   seguidas la apretaste (como el "Programa 1" de teclado de clase).
   * = borra la ultima letra       # = borra todo                          */
void p13_lcd_teclado(void) {
    char k, ultima, veces, largo, i;
    char texto[17];
    char t[17];
    char n[6];
    largo = 0;
    texto[0] = 0;
    ultima = 0;
    veces = 0;
    Lcd_Cmd(_LCD_CLEAR);
    lcd_linea(1, T_APRIETA);
    while (sigue()) {
        k = tecla();
        if (k != 0) {
            if (k == ultima) { if (veces < 99) veces++; } else { ultima = k; veces = 1; }
            if (k == '*') {
                if (largo > 0) largo--;
            } else if (k == '#') {
                largo = 0;
            } else {
                if (largo == 16) {            /* lleno: se recorre una a la izquierda */
                    for (i = 0; i < 15; i++) texto[i] = texto[i + 1];
                    largo = 15;
                }
                texto[largo] = k;
                largo++;
            }
            texto[largo] = 0;
            /* renglon 1: "Tecla:7 Veces:12" */
            t[0]='T'; t[1]='e'; t[2]='c'; t[3]='l'; t[4]='a'; t[5]=':'; t[6]=k; t[7]=' ';
            t[8]='V'; t[9]='e'; t[10]='c'; t[11]='e'; t[12]='s'; t[13]=':'; t[14]=0;
            numero_a_texto(veces, n);
            i = 0;
            while (n[i] != 0) { t[14 + i] = n[i]; i++; }
            t[14 + i] = 0;
            lcd_linea_ram(1, t);
            lcd_linea_ram(2, texto);
        }
        if (!esperar(10)) return;
    }
}
#endif


#if PARTE == 0 || PARTE == 4
/* ===================== APLICACIONES (practicas 14 y 15) =================== */

/* --- 14. Chapa electronica con clave. Junta todo lo del semestre: teclado
   para meter la clave, LCD para los mensajes, display para el estado y la
   EEPROM para que la clave no se olvide al desconectar.
     Cerrada:  teclea 4 numeros y #.  * borra.         Display: C
     Abierta:  B = cerrar, A = cambiar la clave.       Display: A
     3 claves malas: se bloquea 10 s con cuenta regresiva en el display.
   La clave de fabrica es CLAVE_DE_FABRICA. En la EEPROM: direccion 0 = 0xA5
   si ya hay clave guardada, direcciones 1 a 4 = los 4 numeros.            */
char clave[5];

void cargar_clave(void) {
    char i;
    if (ee_leer(0) == 0xA5) {
        for (i = 0; i < 4; i++) clave[i] = ee_leer(1 + i);
    } else {
        for (i = 0; i < 4; i++) clave[i] = CLAVE_DE_FABRICA[i];
    }
    clave[4] = 0;
}

void guardar_clave(char *nueva) {
    char i;
    for (i = 0; i < 4; i++) {
        clave[i] = nueva[i];
        ee_escribir(1 + i, nueva[i]);
    }
    ee_escribir(0, 0xA5);
}

/* Pinta "Clave: **__" con un asterisco por numero ya tecleado. */
void pintar_clave(char n, char ocultar, char *escrita) {
    char t[17];
    char i;
    t[0]='C'; t[1]='l'; t[2]='a'; t[3]='v'; t[4]='e'; t[5]=':'; t[6]=' ';
    for (i = 0; i < 4; i++) {
        if (i < n) { if (ocultar) t[7 + i] = '*'; else t[7 + i] = escrita[i]; }
        else t[7 + i] = '_';
    }
    t[11] = 0;
    lcd_linea_ram(2, t);
}

void p14_chapa(void) {
    char k, n, intentos, estado, i, ok;
    char escrita[5];
    char t[17];
    /* estado: 0 = cerrada pidiendo clave, 1 = abierta, 2 = escribiendo clave nueva */
    cargar_clave();
    estado = 0;
    n = 0;
    intentos = 3;
    Lcd_Cmd(_LCD_CLEAR);
    lcd_linea(1, T_CHAPA);
    pintar_clave(0, 1, escrita);
    mostrar(0x39);                                   /* C de cerrada */
    while (sigue()) {
        k = tecla();
        if (k != 0) {
            if (estado == 1) {                       /* ---- abierta ---- */
                if (k == 'B') {
                    estado = 0; n = 0;
                    lcd_linea(1, T_CHAPA);
                    pintar_clave(0, 1, escrita);
                    mostrar(0x39);
                } else if (k == 'A') {
                    estado = 2; n = 0;
                    lcd_linea(1, T_NUEVA);
                    pintar_clave(0, 0, escrita);
                }
            } else {                                 /* ---- pidiendo clave (0 o 2) ---- */
                if (k >= '0' && k <= '9' && n < 4) {
                    escrita[n] = k;
                    n++;
                    pintar_clave(n, estado == 0, escrita);
                } else if (k == '*') {
                    if (n > 0) n--;
                    pintar_clave(n, estado == 0, escrita);
                } else if (k == '#' && n == 4) {
                    if (estado == 2) {               /* guarda la nueva */
                        guardar_clave(escrita);
                        lcd_linea(1, T_GUARDADA);
                        if (!esperar(1200)) return;
                        estado = 1;
                        lcd_linea(1, T_ABIERTA);
                        lcd_linea(2, T_OPC_ABIERTA);
                    } else {                         /* revisa la clave */
                        ok = 1;
                        for (i = 0; i < 4; i++) if (escrita[i] != clave[i]) ok = 0;
                        if (ok) {
                            estado = 1;
                            intentos = 3;
                            lcd_linea(1, T_ABIERTA);
                            lcd_linea(2, T_OPC_ABIERTA);
                            mostrar(0x77);           /* A de abierta */
                        } else {
                            intentos--;
                            lcd_linea(1, T_CLAVE_MALA);
                            t[0]='T'; t[1]='e'; t[2]=' '; t[3]='q'; t[4]='u'; t[5]='e';
                            t[6]='d'; t[7]='a'; t[8]='n'; t[9]=':'; t[10]=' ';
                            t[11] = '0' + intentos; t[12] = 0;
                            lcd_linea_ram(2, t);
                            mostrar(0x79);           /* E de error */
                            if (!esperar(1500)) return;
                            if (intentos == 0) {     /* bloqueo con cuenta regresiva */
                                lcd_linea(1, T_BLOQUEADA);
                                for (i = 10; i > 0; i--) {
                                    t[0]='E'; t[1]='s'; t[2]='p'; t[3]='e'; t[4]='r'; t[5]='a';
                                    t[6]=' ';
                                    if (i >= 10) { t[7]='1'; t[8]='0'; } else { t[7]=' '; t[8]='0' + i; }
                                    t[9]=' '; t[10]='s'; t[11]=0;
                                    lcd_linea_ram(2, t);
                                    mostrar(DIGITOS[i - 1]);   /* 9 .. 0 */
                                    if (!esperar(1000)) return;
                                }
                                intentos = 3;
                            }
                            n = 0;
                            lcd_linea(1, T_CHAPA);
                            pintar_clave(0, 1, escrita);
                            mostrar(0x39);
                        }
                    }
                }
            }
        }
        if (!esperar(10)) return;
    }
}

/* --- 15. Calculadora de enteros. Teclas:
     0..9 numeros    A = +    B = -    C = x    D = /    # = igual    * = borrar
   Renglon 1: la operacion ("125x8").  Renglon 2: el resultado ("= 1000").
   Despues de un resultado puedes seguir con otra operacion sobre el.
   La division sale entera con su residuo ("= 12 r 3").                    */
char calc_linea[17];
char calc_largo;

void calc_agregar(char c) {
    if (calc_largo < 16) {
        calc_linea[calc_largo] = c;
        calc_largo++;
        calc_linea[calc_largo] = 0;
    }
}

void p15_calculadora(void) {
    char k, op, estado, digitos_b, digitos_a, i, j, error;
    long a, b, r, residuo;
    char t[17];
    char n[12];
    /* estado: 0 = primer numero, 1 = segundo numero, 2 = ya hay resultado */
    a = 0; b = 0; op = 0; estado = 0; digitos_a = 0; digitos_b = 0;
    calc_largo = 0; calc_linea[0] = 0;
    Lcd_Cmd(_LCD_CLEAR);
    lcd_linea(1, PISTAS[15]);
    while (sigue()) {
        k = tecla();
        if (k != 0) {
            if (k == '*') {                                  /* borrar todo */
                a = 0; b = 0; op = 0; estado = 0; digitos_a = 0; digitos_b = 0;
                calc_largo = 0; calc_linea[0] = 0;
                lcd_linea_ram(1, calc_linea);
                lcd_linea(2, T_VACIO);
            } else if (k >= '0' && k <= '9') {               /* un numero */
                if (estado == 2) {                           /* empieza otra cuenta */
                    a = 0; b = 0; op = 0; estado = 0; digitos_a = 0; digitos_b = 0;
                    calc_largo = 0; calc_linea[0] = 0;
                    lcd_linea(2, T_VACIO);
                }
                if (estado == 0 && digitos_a < 6) {
                    a = a * 10 + (k - '0');
                    digitos_a++;
                    calc_agregar(k);
                } else if (estado == 1 && digitos_b < 6) {
                    b = b * 10 + (k - '0');
                    digitos_b++;
                    calc_agregar(k);
                }
                lcd_linea_ram(1, calc_linea);
            } else if (k >= 'A' && k <= 'D') {               /* una operacion */
                if (estado == 2) {                           /* sigue sobre el resultado */
                    numero_a_texto(a, n);
                    calc_largo = 0; calc_linea[0] = 0;
                    for (i = 0; n[i] != 0; i++) calc_agregar(n[i]);
                    estado = 0; digitos_a = 1;
                    lcd_linea(2, T_VACIO);
                }
                if (estado == 1 && digitos_b == 0) {         /* cambia la operacion */
                    calc_largo--;
                    calc_linea[calc_largo] = 0;
                    estado = 0;
                }
                if (estado == 0 && digitos_a > 0) {
                    if (k == 'A') op = '+';
                    if (k == 'B') op = '-';
                    if (k == 'C') op = 'x';
                    if (k == 'D') op = '/';
                    calc_agregar(op);
                    estado = 1; b = 0; digitos_b = 0;
                }
                lcd_linea_ram(1, calc_linea);
            } else if (k == '#' && estado == 1 && digitos_b > 0) {   /* igual */
                error = 0;
                residuo = 0;
                /* Un long llega a 2147483647. Se revisa ANTES de operar si el
                   resultado cabe: si se pasa, el numero "da la vuelta" y saldria
                   un negativo sin sentido. (b nunca es negativo; a si puede,
                   si viene de un resultado anterior.)                        */
                if (op == '+') {
                    if (a > 2147483647 - b) error = 2; else r = a + b;
                }
                if (op == '-') {
                    if (a < -2147483647 + b) error = 2; else r = a - b;
                }
                if (op == 'x') {
                    if (a < 0) r = -a; else r = a;            /* r = |a| de paso */
                    if (r != 0 && b > 2147483647 / r) error = 2; else r = a * b;
                }
                if (op == '/') {
                    if (b == 0) error = 1;
                    else { r = a / b; residuo = a % b; }
                }
                if (error == 1) {
                    lcd_linea(2, T_ERROR);
                    a = 0;
                } else if (error == 2) {
                    lcd_linea(2, T_GRANDE);
                    a = 0;
                } else {
                    t[0] = '='; t[1] = ' ';
                    numero_a_texto(r, n);
                    i = 0;
                    while (n[i] != 0) { t[2 + i] = n[i]; i++; }
                    i = i + 2;
                    if (residuo != 0) {
                        t[i] = ' '; t[i + 1] = 'r'; t[i + 2] = ' ';
                        i = i + 3;
                        numero_a_texto(residuo, n);
                        j = 0;
                        while (n[j] != 0 && i < 16) { t[i] = n[j]; i++; j++; }
                    }
                    t[i] = 0;
                    lcd_linea_ram(2, t);
                    a = r;
                }
                estado = 2;
            }
        }
        if (!esperar(10)) return;
    }
}
#endif


/* ######################################################################### */
/*                         CONFIGURACION Y MAIN                               */
/* ######################################################################### */

/* configurar: todo lo que el PIC necesita antes de la primera practica.
   - OSCCON = 0x72: oscilador interno a 16 MHz. Sin esto arranca a 1 MHz y
     todos los Delay_ms durarian 16 veces mas.
   - ANSELx = 0: en el 45K50 los pines arrancan como ANALOGICOS y asi leen
     0 siempre. (En el 4550 de las presentaciones era ADCON1 = 0x0F y
     CMCON = 7; esos registros no existen en el 45K50.)
   - Comparadores apagados, salidas en 0 ANTES de volverlas salidas.        */
void configurar(void) {
    OSCCON = 0x72;
    ANSELA = 0x00;
    ANSELB = 0x00;
    ANSELC = 0x00;
    ANSELD = 0x00;
    ANSELE = 0x00;
    CM1CON0 = 0x00;
    CM2CON0 = 0x00;

    LATA = 0x00; LATB = 0x00; LATC = 0x00; LATD = 0x00; LATE = 0x00;
    TRISA = 0xFF;       /* switches: entradas                          */
    TRISB = 0xFF;       /* teclado: se maneja en teclado_inicio/escanear */
    TRISC = 0x30;       /* RC4/RC5 solo pueden ser entradas (USB)       */
    TRISD = 0x00;       /* barra + display: salidas                     */
    TRISE = 0x00;       /* RS y EN del LCD                              */
    INTCON = 0x00;      /* sin interrupciones                           */
}

void main() {
    configurar();
    Delay_ms(50);                   /* el LCD necesita su tiempo al prender */
    Lcd_Init();
    Lcd_Cmd(_LCD_CLEAR);
    Lcd_Cmd(_LCD_CURSOR_OFF);
    teclado_inicio();

    while (1) {
        practica = selector_estable();
        mostrar(0x00);
        Lcd_Cmd(_LCD_CLEAR);
        titulo();
        if (practica != 0) {
            if (!esperar(800)) continue;   /* 0.8 s para leer el titulo */
        }
        tecla_ant = 0;

        switch (practica) {
            case 0:  p00_espera(); break;
#if PARTE == 0 || PARTE == 1
            case 1:  p01_puertos(); break;
            case 2:  p02_un_bit(); break;
            case 3:  p03_pares_impares(); break;
            case 4:  p04_palabra(); break;
            case 5:  p05_palabra_hex(); break;
            case 6:  p06_palabra_letras(); break;
            case 7:  p07_subrutinas(); break;
            case 8:  p08_tabla(); break;
#endif
#if PARTE == 0 || PARTE == 2
            case 9:  p09_mensaje(); break;
            case 10: p10_animacion(); break;
            case 11: p11_marquesina(); break;
#endif
#if PARTE == 0 || PARTE == 3
            case 12: p12_uso_teclado(); break;
            case 13: p13_lcd_teclado(); break;
#endif
#if PARTE == 0 || PARTE == 4
            case 14: p14_chapa(); break;
            case 15: p15_calculadora(); break;
#endif
            default: p_no_incluida(); break;
        }
    }
}
