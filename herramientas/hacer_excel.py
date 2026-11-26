# Arma Entrega_y_pines.xlsx: hoja "Entrega" (que mover, que combinacion, que
# se ve, para palomear) y hoja "Pines" (como va conectado todo).
#   python herramientas\hacer_excel.py
from openpyxl import Workbook
from openpyxl.styles import Alignment, Border, Font, PatternFill, Side
from openpyxl.worksheet.datavalidation import DataValidation
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent
SALIDA = RAIZ / "Entrega_y_pines.xlsx"

GRIS = PatternFill("solid", fgColor="D9D9D9")
GRIS_CLARO = PatternFill("solid", fgColor="F2F2F2")
BLOQUE = PatternFill("solid", fgColor="BFBFBF")
DELGADA = Side(style="thin", color="808080")
CAJA = Border(left=DELGADA, right=DELGADA, top=DELGADA, bottom=DELGADA)
ARRIBA = Alignment(vertical="top", wrap_text=True)
CENTRO = Alignment(horizontal="center", vertical="top", wrap_text=True)
MONO = Font(name="Consolas", size=11)
NEGRITA = Font(bold=True)

# ---------------------------------------------------------------------------
# Hoja 1: Entrega
# Cada practica: (numero, nombre, bloque, selector, que mueves, descripcion,
#                 [(combinacion, que ves en barra/display, que ves en el LCD), ...])
# Selector y combinaciones van COMO LOS VES EN LA TARJETA, de izquierda a derecha.
# ---------------------------------------------------------------------------
PRACTICAS = [
    (0, "Espera (menu)", "Arranque", "0 0 0 0", "Nada",
     "La senal de vida. Si ves la rayita de en medio en el display es que el PIC prendio bien y "
     "esta esperando a que le digas que practica quieres con los 4 switches de la derecha.",
     [("x x x x", "Rayita de en medio  -", "PRACTICAS MIKROC / Elige RA4-RA7")]),

    (1, "PORTA a PORTD", "Pasar de asm a C", "0 0 0 1", "Cualquier switch de A",
     "La primerita de clase: lo que pongas en los switches sale tal cual en la barra. Los 4 focos "
     "de arriba copian el selector, los 4 de abajo los switches de datos. Sirve para ver que todo "
     "esta bien conectado.",
     [("1 0 1 0", "Barra: RD0 y RD2 prendidos, y RD7 (el selector)", "P01 PORTA>PORTD"),
      ("1 1 1 1", "Barra: RD0 a RD3 prendidos, y RD7", "")]),

    (2, "Monitoreo de un bit", "Pasar de asm a C", "0 0 1 0", "RA2 (el tercero)",
     "Nomas le pregunta a UN switch, el RA2, si esta arriba o abajo, y segun eso pinta un numero u "
     "otro en la barra. Los demas switches ni los pela. En ensamblador era BTFSS; en C es un if.",
     [("x x 1 x", "0110 1010  (0x6A)", "bit = 1 -> 0x6A"),
      ("x x 0 x", "0000 0001  (0x01)", "bit = 0 -> 0x01")]),

    (3, "Monitoreo de un bit: pares e impares", "Pasar de asm a C", "0 0 1 1", "RA0 (el primero)",
     "La modificacion: ahora pregunta por RA0. Si esta arriba pinta focos salteados de un lado "
     "(1010 1010) y si esta abajo los del otro lado (0101 0101). Es el ParesImpares.c de clase.",
     [("1 x x x", "1010 1010  (0xAA)", "bit = 1 -> 0xAA"),
      ("0 x x x", "0101 0101  (0x55)", "bit = 0 -> 0x55")]),

    (4, "Monitoreo de palabra (0101)", "Pasar de asm a C", "0 1 0 0", "RA0 a RA3",
     "Ahora pregunta por los 4 switches a la vez (preguntando bit a bit). Solo si estan exactamente "
     "en la combinacion buena pinta el 0x59; con cualquier otra pinta el 0x11. La presentacion dice "
     "0101, pero leido de RA3 a RA0; en la tarjeta se ve al reves: 1 0 1 0.",
     [("1 0 1 0", "0101 1001  (0x59)", "1010 SI -> 0x59"),
      ("cualquier otra", "0001 0001  (0x11)", "otra   -> 0x11")]),

    (5, "Palabra en hexadecimal", "Pasar de asm a C", "0 1 0 1", "RA0 a RA3",
     "Los 4 switches forman un numero del 0 al 15 y el display te lo ensena en hexa (0 a 9 y A a F). "
     "Ojo: RA0 es el que vale 1 y RA3 el que vale 8, o sea el numero se lee de derecha a izquierda.",
     [("0 0 0 0", "0", "0000 = 0 (0)"),
      ("1 0 0 0", "1", "1000 = 1 (1)"),
      ("0 1 0 1", "A", "0101 = A (10)"),
      ("1 1 0 1", "b", "1101 = B (11)"),
      ("1 1 1 1", "F", "1111 = F (15)")]),

    (6, "Palabra a letras (IPN-UPIICSA)", "Pasar de asm a C", "0 1 1 0", "RA0 a RA3",
     "Cada combinacion es una letra de IPN-UPIICSA. Si vas moviendo los switches como contador "
     "(0, 1, 2...) se va armando la palabra. Es el IPNU.c de clase pero con tabla en vez de "
     "un monton de if.",
     [("0 0 0 0", "I", "0000 -> I"), ("1 0 0 0", "P", "1000 -> P"), ("0 1 0 0", "N", "0100 -> N"),
      ("1 1 0 0", "-", "1100 -> -"), ("0 0 1 0", "U", "0010 -> U"), ("1 0 1 0", "P", "1010 -> P"),
      ("0 1 1 0", "I", "0110 -> I"), ("1 1 1 0", "I", "1110 -> I"), ("0 0 0 1", "C", "0001 -> C"),
      ("1 0 0 1", "S", "1001 -> S"), ("0 1 0 1", "A", "0101 -> A"),
      ("1 1 0 1 en adelante", "Apagado", "... -> .")]),

    (7, "Subrutinas de tiempo", "Pasar de asm a C", "0 1 1 1", "RA0 y RA1",
     "Un contador en binario en la barra, y las subrutinas son las funciones de espera: la de 1 s "
     "llama mil veces a la de 1 ms y la de 3 s llama tres veces a la de 1 s. Es lo mismo que los "
     "ciclos anidados de ensamblador pero sin sufrirle. El ultimo modo espera con el Timer0. "
     "En el modo de 3 s tarda hasta 3 s en hacerte caso si cambias de practica.",
     [("0 0 x x", "Cuenta rapidisimo (1 ms): RD0 se ve a medio brillo", "Modo: 1 ms"),
      ("1 0 x x", "Sube 1 cada segundo", "Modo: 1 s"),
      ("0 1 x x", "Sube 1 cada 3 segundos", "Modo: 3 s"),
      ("1 1 x x", "Sube 1 cada segundo (con Timer0)", "Modo: Timer0 1 s")]),

    (8, "Tabla", "Pasar de asm a C", "1 0 0 0", "RA0 y RA1",
     "Un contador que en vez de ensenar el numero lo usa para buscar en una tabla que dibujar en el "
     "display. Cambia de letra cada 0.7 s. Es el CONTADOR.c de clase y el HOLA de ensamblador.",
     [("0 0 x x", "0 1 2 3 ... F", "0..F subiendo"),
      ("1 0 x x", "H O L A - I P n", "HOLA-IPn"),
      ("0 1 x x", "L E O n A r d O", "LEOnArdO"),
      ("1 1 x x", "F E d C ... 0", "F..0 bajando")]),

    (9, "Mostrar un mensaje", "LCD", "1 0 0 1", "RA0",
     "Escribir texto fijo en la pantalla, en el renglon y la columna que tu quieras. Los textos se "
     "cambian hasta arriba de practicas.c.",
     [("0 x x x", "Apagado", "*** UPIICSA *** / **** IPN ****"),
      ("1 x x x", "Apagado", "LEONARDO / Disp. Programab.")]),

    (10, "Generar una animacion", "LCD", "1 0 1 0", "Nada, nomas mira",
     "Se dibujan caracteres propios, cuadrito por cuadrito (5x8 puntos), y se van cambiando para "
     "que parezca que se mueven, como el pac-man de clase. Primero una barra de carga que se llena "
     "con su porcentaje, luego un monito camina hasta un corazon y brinca de gusto.",
     [("x x x x", "Apagado", "Cargando... 0% a 100%, luego el monito caminando")]),

    (11, "Mensaje marquesina", "LCD", "1 0 1 1", "RA0 y RA1",
     "Un letrero mas largo que la pantalla pasa como en las tiendas, y cuando se acaba vuelve a "
     "empezar. El renglon de abajo se queda quieto. RA0 lo acelera y RA1 lo pone al reves.",
     [("0 0 x x", "Apagado", "El texto corre a la izquierda / MARQUESINA"),
      ("1 0 x x", "Apagado", "Igual pero rapido"),
      ("0 1 x x", "Apagado", "Corre a la derecha"),
      ("1 1 x x", "Apagado", "A la derecha y rapido")]),

    (12, "Uso del teclado", "Teclado", "1 1 0 0", "Las teclas",
     "Saber que tecla apretaste. El PIC prende una fila a la vez y se fija que columna le contesta. "
     "La tecla sale en el display, y en la pantalla con su fila y columna: si una tecla sale "
     "cambiada, con eso ves que cable esta cruzado.",
     [("Pica 7", "7", "Tecla 7  F3 C1"),
      ("Pica A", "A", "Tecla A  F1 C4"),
      ("Pica *", "E", "Tecla *  F4 C1"),
      ("Pica #", "F", "Tecla #  F4 C3")]),

    (13, "LCD + teclado", "Teclado", "1 1 0 1", "Las teclas",
     "Lo que tecleas se va escribiendo en la pantalla como en un celular viejito; cuando se llena se "
     "recorre. Arriba te dice la ultima tecla y cuantas veces seguidas la picaste.",
     [("1 2 3", "Apagado", "Tecla:3 Veces:1 / 123"),
      ("* (asterisco)", "Apagado", "Borra la ultima: 12"),
      ("# (gato)", "Apagado", "Borra todo"),
      ("3 3 3", "Apagado", "Tecla:3 Veces:3")]),

    (14, "Aplicacion: chapa con clave", "Aplicacion", "1 1 1 0", "Las teclas",
     "Una chapa electronica que junta todo: metes la clave con el teclado, la pantalla te dice si "
     "pasaste y el display ensena si esta cerrada (C) o abierta (A). La clave se guarda en la "
     "memoria del PIC que no se borra al desconectarlo. Tres claves malas y se bloquea 10 s. "
     "La de fabrica es 1234 (se vuelve 1234 cada que grabas el PIC).",
     [("1 2 3 4 #", "A", "ABIERTA :) / B=cerrar A=clave"),
      ("B", "C", "CHAPA  CERRADA / Clave: ____"),
      ("0 0 0 0 #", "E", "CLAVE MALA / Te quedan: 2"),
      ("Abierta: A, 4 numeros, #", "A", "Clave guardada"),
      ("3 claves malas", "Cuenta 9 ... 0", "BLOQUEADA / Espera 10 s")]),

    (15, "Aplicacion: calculadora", "Aplicacion", "1 1 1 1", "Las teclas",
     "Calculadora de enteros con el teclado. A = mas, B = menos, C = por, D = entre, # = igual, "
     "* = borrar. Numeros de hasta 6 cifras; despues de un resultado le puedes seguir con otra "
     "operacion. La division te da el residuo.",
     [("1 2 C 1 2 #", "Apagado", "12x12 / = 144"),
      ("luego A 6 #", "Apagado", "144+6 / = 150"),
      ("* 7 D 2 #", "Apagado", "7/2 / = 3 r 1"),
      ("* 5 D 0 #", "Apagado", "5/0 / = Error")]),
]


def hoja_entrega(ws):
    ws.title = "Entrega"
    ws["A1"] = "Practicas MikroC - PIC18F45K50 - lista de entrega"
    ws["A1"].font = Font(bold=True, size=14)
    ws["A2"] = ("Selector = los 4 switches de la DERECHA (RA4 RA5 RA6 RA7). Combinacion = los 4 de la "
                "IZQUIERDA (RA0 RA1 RA2 RA3). Todo como lo ves en la tarjeta, de izquierda a derecha. "
                "x = no importa.  Palomea la columna Hecho cuando te la califiquen.")
    ws["A2"].alignment = ARRIBA
    ws.merge_cells("A2:J2")
    ws.row_dimensions[2].height = 33

    titulos = ["Hecho", "#", "Practica", "Bloque", "Selector\nRA4 RA5 RA6 RA7", "Que mueves",
               "Combinacion\nRA0 RA1 RA2 RA3", "Que ves en la barra / display",
               "Que ves en la pantalla (LCD)", "De que va"]
    fila = 4
    for c, t in enumerate(titulos, 1):
        celda = ws.cell(row=fila, column=c, value=t)
        celda.font = NEGRITA
        celda.fill = GRIS
        celda.alignment = CENTRO
        celda.border = CAJA
    ws.row_dimensions[fila].height = 32

    dv = DataValidation(type="list", formula1='"Si,No,Falta revisar"', allow_blank=True)
    ws.add_data_validation(dv)

    fila = 5
    bloque_actual = None
    for num, nombre, bloque, selector, mueve, descripcion, combos in PRACTICAS:
        if bloque != bloque_actual:
            bloque_actual = bloque
            ws.cell(row=fila, column=1, value=bloque.upper()).font = NEGRITA
            for c in range(1, 11):
                ws.cell(row=fila, column=c).fill = BLOQUE
                ws.cell(row=fila, column=c).border = CAJA
            fila += 1
        inicio = fila
        for combo, ve, lcd in combos:
            ws.cell(row=fila, column=7, value=combo).font = MONO
            ws.cell(row=fila, column=8, value=ve)
            ws.cell(row=fila, column=9, value=lcd).font = MONO
            fila += 1
        fin = fila - 1
        valores = {1: "", 2: num, 3: nombre, 4: bloque, 5: selector, 6: mueve, 10: descripcion}
        for c, v in valores.items():
            ws.cell(row=inicio, column=c, value=v)
            if fin > inicio:
                ws.merge_cells(start_row=inicio, start_column=c, end_row=fin, end_column=c)
        ws.cell(row=inicio, column=5).font = Font(name="Consolas", size=11, bold=True)
        ws.cell(row=inicio, column=3).font = NEGRITA
        dv.add(ws.cell(row=inicio, column=1))
        for r in range(inicio, fin + 1):
            for c in range(1, 11):
                celda = ws.cell(row=r, column=c)
                celda.border = CAJA
                celda.alignment = CENTRO if c in (1, 2, 5, 7) else ARRIBA
                if num % 2 == 1:
                    celda.fill = GRIS_CLARO
        # que la descripcion quepa: alto segun lo largo del texto
        lineas = max(len(descripcion) // 55 + 1, 1)
        alto_por_fila = max(15, lineas * 15 / (fin - inicio + 1))
        for r in range(inicio, fin + 1):
            ws.row_dimensions[r].height = max(ws.row_dimensions[r].height or 15, alto_por_fila)

    anchos = [9, 4, 26, 15, 17, 18, 22, 34, 34, 60]
    for i, a in enumerate(anchos, 1):
        ws.column_dimensions[chr(64 + i)].width = a
    ws.freeze_panes = "C5"
    ws.page_setup.orientation = "landscape"
    ws.page_setup.fitToWidth = 1
    ws.page_setup.fitToHeight = 0
    ws.sheet_properties.pageSetUpPr.fitToPage = True
    ws.print_title_rows = "4:4"


# ---------------------------------------------------------------------------
# Hoja 2: Pines
# ---------------------------------------------------------------------------
PIC = [  # pata, nombre, va a, entra/sale, nota
    (1, "MCLR / RE3", "PICkit pin 1 (MCLR) y 10k a 5 V", "-", "Reset. Lo usa el PICkit para grabar."),
    (2, "RA0", "Switch 1 (datos)", "Entrada", "Vale 1 en los numeros"),
    (3, "RA1", "Switch 2 (datos)", "Entrada", "Vale 2"),
    (4, "RA2", "Switch 3 (datos)", "Entrada", "Vale 4. El de la practica 2"),
    (5, "RA3", "Switch 4 (datos)", "Entrada", "Vale 8"),
    (6, "RA4", "Switch 5 (selector)", "Entrada", "Selector, vale 8"),
    (7, "RA5", "Switch 6 (selector)", "Entrada", "Selector, vale 4"),
    (8, "RE0", "LCD pin 4 (RS)", "Salida", ""),
    (9, "RE1", "LCD pin 6 (E)", "Salida", ""),
    (10, "RE2", "Libre", "-", ""),
    (11, "VDD", "5 V", "-", "Junto con la 32"),
    (12, "VSS", "Tierra", "-", "Junto con la 31"),
    (13, "RA7", "Switch 8 (selector)", "Entrada", "Selector, vale 1"),
    (14, "RA6", "Switch 7 (selector)", "Entrada", "Selector, vale 2. OJO: esta antes que RA7 en el chip"),
    (15, "RC0", "LCD pin 11 (D4)", "Salida", ""),
    (16, "RC1", "LCD pin 12 (D5)", "Salida", ""),
    (17, "RC2", "Libre", "-", ""),
    (18, "VUSB3V3", "Nada", "-", "No usamos USB"),
    (19, "RD0", "Barra 1 y segmento a", "Salida", ""),
    (20, "RD1", "Barra 2 y segmento b", "Salida", ""),
    (21, "RD2", "Barra 3 y segmento c", "Salida", ""),
    (22, "RD3", "Barra 4 y segmento d", "Salida", ""),
    (23, "RC4", "Libre", "-", "Solo puede ser entrada"),
    (24, "RC5", "Libre", "-", "Solo puede ser entrada"),
    (25, "RC6", "LCD pin 13 (D6)", "Salida", ""),
    (26, "RC7", "LCD pin 14 (D7)", "Salida", ""),
    (27, "RD4", "Barra 5 y segmento e", "Salida", ""),
    (28, "RD5", "Barra 6 y segmento f", "Salida", ""),
    (29, "RD6", "Barra 7 y segmento g", "Salida", "La rayita de en medio"),
    (30, "RD7", "Barra 8 y punto", "Salida", ""),
    (31, "VSS", "Tierra", "-", ""),
    (32, "VDD", "5 V", "-", ""),
    (33, "RB0", "Teclado pin 5 (columna 1)", "Entrada", "Con pull-up adentro del PIC"),
    (34, "RB1", "Teclado pin 6 (columna 2)", "Entrada", ""),
    (35, "RB2", "Teclado pin 7 (columna 3)", "Entrada", ""),
    (36, "RB3", "Teclado pin 8 (columna 4)", "Entrada", ""),
    (37, "RB4", "Teclado pin 1 (fila 1)", "Salida", ""),
    (38, "RB5", "Teclado pin 2 (fila 2)", "Salida", ""),
    (39, "RB6 / PGC", "Teclado pin 3 (fila 3) y PICkit pin 5", "Salida", "Comparte con el PICkit: no aprietes teclas mientras grabas"),
    (40, "RB7 / PGD", "Teclado pin 4 (fila 4) y PICkit pin 4", "Salida", "Comparte con el PICkit"),
]

LCD = [
    (1, "VSS", "Tierra", ""),
    (2, "VDD", "5 V", ""),
    (3, "V0", "Pata de en medio del potenciometro de 10k", "Contraste. Las otras dos patas a 5 V y tierra. Si no ves nada o ves puros cuadros negros, muevelo"),
    (4, "RS", "RE0 (pata 8)", ""),
    (5, "RW", "Tierra", "Siempre escribimos, nunca leemos"),
    (6, "E", "RE1 (pata 9)", ""),
    (7, "D0", "Nada", "El LCD va en modo de 4 bits: D0 a D3 se quedan al aire"),
    (8, "D1", "Nada", ""),
    (9, "D2", "Nada", ""),
    (10, "D3", "Nada", ""),
    (11, "D4", "RC0 (pata 15)", ""),
    (12, "D5", "RC1 (pata 16)", ""),
    (13, "D6", "RC6 (pata 25)", ""),
    (14, "D7", "RC7 (pata 26)", ""),
    (15, "A (luz +)", "5 V con resistencia de 100 ohm", "Si tu LCD ya trae la resistencia, directo"),
    (16, "K (luz -)", "Tierra", ""),
]

TECLADO = [
    (1, "Fila 1 (1 2 3 A)", "RB4", 37),
    (2, "Fila 2 (4 5 6 B)", "RB5", 38),
    (3, "Fila 3 (7 8 9 C)", "RB6", 39),
    (4, "Fila 4 (* 0 # D)", "RB7", 40),
    (5, "Columna 1 (1 4 7 *)", "RB0", 33),
    (6, "Columna 2 (2 5 8 0)", "RB1", 34),
    (7, "Columna 3 (3 6 9 #)", "RB2", 35),
    (8, "Columna 4 (A B C D)", "RB3", 36),
]

PICKIT = [
    (1, "MCLR / VPP", "Pata 1"),
    (2, "VDD", "Pata 11 o 32 (5 V)"),
    (3, "GND", "Pata 12 o 31"),
    (4, "PGD", "Pata 40 (RB7)"),
    (5, "PGC", "Pata 39 (RB6)"),
    (6, "PGM / LVP", "Nada (LVP va apagado)"),
]

NOTAS = [
    "Que cambio contra lo de ensamblador:",
    "1. El SELECTOR se paso del puerto B a los 4 switches de la derecha del puerto A (RA4 RA5 RA6 RA7), "
    "porque el teclado necesita los 8 pines del puerto B. Se lee igual: de izquierda a derecha.",
    "2. Los switches de B de la tarjeta ya NO se usan: quitales el puente o dejalos todos abajo; si "
    "alguno queda arriba, el teclado vera una tecla apretada que no existe.",
    "3. Los 4 switches de la izquierda (RA0 a RA3) siguen siendo los datos. La barra y el display "
    "siguen en el puerto D, igual que antes.",
    "4. El LCD va donde lo pone el profe en sus programas (RS en RE0, E en RE1, datos en RC0 RC1 RC6 RC7). "
    "Aunque el LCD tenga 16 pines, solo 6 van al PIC; el resto es corriente, tierra, contraste y luz.",
    "5. Las filas del teclado van en RB4 a RB7 y no al reves porque RB6 y RB7 son los cables del PICkit, "
    "que tiene resistencias a tierra ahi: como salidas no estorban, como entradas si.",
    "6. Corriente: el PICkit da poquita. Con la barra, el display y la luz del LCD prendidos puede no "
    "alcanzar (se cae o se reinicia). Si pasa: fuente de 5 V para la tarjeta y en cargar.bat pon VOLTAJE = FUENTE.",
    "7. Si una tecla sale cambiada (aprietas 1 y sale 4), es que filas y columnas quedaron al reves: "
    "la practica 12 te dice que fila y columna leyo el PIC.",
]


def tabla(ws, fila, titulo, encabezados, datos, anchos_texto=None):
    ws.cell(row=fila, column=1, value=titulo).font = Font(bold=True, size=12)
    fila += 1
    for c, t in enumerate(encabezados, 1):
        celda = ws.cell(row=fila, column=c, value=t)
        celda.font = NEGRITA
        celda.fill = GRIS
        celda.border = CAJA
        celda.alignment = CENTRO
    fila += 1
    for d in datos:
        for c, v in enumerate(d, 1):
            celda = ws.cell(row=fila, column=c, value=v)
            celda.border = CAJA
            celda.alignment = CENTRO if c == 1 else ARRIBA
        if len(str(d[-1])) > 45:
            ws.row_dimensions[fila].height = 30
        fila += 1
    return fila + 1


def hoja_pines(ws):
    ws.title = "Pines"
    ws["A1"] = "Acomodo de pines - PIC18F45K50 (40 patas)"
    ws["A1"].font = Font(bold=True, size=14)
    fila = 3
    fila = tabla(ws, fila, "El PIC, pata por pata", ["Pata", "Pin", "Va a", "Entra o sale", "Nota"], PIC)
    fila = tabla(ws, fila, "LCD 16x2 (los 16 pines)", ["Pin LCD", "Nombre", "Se conecta a", "Nota"], LCD)
    fila = tabla(ws, fila, "Teclado 4x4 (pines de izquierda a derecha, viendolo de frente)",
                 ["Pin teclado", "Que es", "Va a", "Pata del PIC"], TECLADO)
    fila = tabla(ws, fila, "PICkit (la flechita es el pin 1)", ["Pin PICkit", "Nombre", "Va a"], PICKIT)
    for n in NOTAS:
        celda = ws.cell(row=fila, column=1, value=n)
        celda.alignment = ARRIBA
        ws.merge_cells(start_row=fila, start_column=1, end_row=fila, end_column=5)
        ws.row_dimensions[fila].height = 15 if n.endswith(":") else 32
        if n.endswith(":"):
            celda.font = NEGRITA
        fila += 1
    for col, a in zip("ABCDE", [12, 22, 40, 14, 60]):
        ws.column_dimensions[col].width = a
    ws.page_setup.fitToWidth = 1
    ws.page_setup.fitToHeight = 0
    ws.sheet_properties.pageSetUpPr.fitToPage = True


def main():
    wb = Workbook()
    hoja_entrega(wb.active)
    hoja_pines(wb.create_sheet())
    wb.save(SALIDA)
    print("Listo:", SALIDA)


if __name__ == "__main__":
    main()
