# Instrucciones para utilizar XINU RTOS en AVR

Para poder compilar **XINU RTOS** debe instalar los siguientes paquetes:

```
bison
flex
gawk
build-essential
```

(en su Linux pueden llamarse distinto).

# Descargar XINU RTOS bare metal

```
wget https://se.fi.uncoma.edu.ar/pse/referencia/xinu_avr_pse.tar.gz
```

# Configurar y compilar

```
tar xvzf xinu_avr_pse.tar.gz
cd xinu-avr-pse/compile
```

Editar el `Makefile` en ese directorio, y seleccionar una de las dos opciones
que especifiquen su plataforma:

Opción 1:
```
# PLATFORM      =       -Dlgt8f328p
# HZ            =       4000000
```

Opción 2:
```
# PLATFORM      =       -DATMEGA
# HZ            =       16000000
```

Si su arduino tiene un chip atmega original, descomentar la opción dos
(quitar el #, y dejar comentada la opción 1).
Para los que tengan el **Arduino Nano** de este año descomentar la opción uno.

Esta configuración permitirá compilar adecuadamente el driver del *clock*
que usa **XINU RTOS** y el driver del *serial*.

# Compilar y verificar

Estando en el directorio de trabajo `xinu-avr-pse/compile/`:

```
make clean
make
make flash
```
