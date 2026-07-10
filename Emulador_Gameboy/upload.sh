#!/bin/bash

PORT=${1:-/dev/ttyUSB0}

echo "1. Creando carpeta 'data' si no existe..."
mkdir -p data

echo "2. Compilando el firmware (usando partitions.csv personalizado)..."
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=custom .
if [ $? -ne 0 ]; then
    echo "Error en la compilación del firmware."
    exit 1
fi

echo "3. Subiendo el firmware por el puerto $PORT..."
arduino-cli upload -p $PORT --fqbn esp32:esp32:esp32:PartitionScheme=custom .
if [ $? -ne 0 ]; then
    echo "Error al subir el firmware."
    exit 1
fi

echo "Buscando mklittlefs y esptool..."
MKLITTLEFS=$(find ~/.arduino15/packages/esp32/tools/mklittlefs -name "mklittlefs" -type f 2>/dev/null | head -n 1)
ESPTOOL=$(find ~/.arduino15/packages/esp32/tools/esptool_py -name "esptool" -type f -o -name "esptool.py" -type f 2>/dev/null | head -n 1)

if [ -z "$MKLITTLEFS" ]; then
    MKLITTLEFS=$(command -v mklittlefs)
fi
if [ -z "$ESPTOOL" ]; then
    ESPTOOL=$(command -v esptool.py || command -v esptool)
fi

if [ -z "$MKLITTLEFS" ]; then
    echo "Error: No se encontró mklittlefs. Instala el core de ESP32 para Arduino o asegúrate de que esté en tu PATH."
    exit 1
fi
if [ -z "$ESPTOOL" ]; then
    echo "Error: No se encontró esptool. Instala el core de ESP32 para Arduino o asegúrate de que esté en tu PATH."
    exit 1
fi

echo "Usando mklittlefs: $MKLITTLEFS"
echo "Usando esptool: $ESPTOOL"

echo "4. Generando imagen LittleFS (tamaño 0x100000 = 1048576 bytes)..."
$MKLITTLEFS -c data -p 256 -b 4096 -s 1048576 littlefs.bin
if [ $? -ne 0 ]; then
    echo "Error al crear la imagen LittleFS."
    exit 1
fi

echo "5. Subiendo los juegos (LittleFS) al ESP32 por el puerto $PORT en la dirección 0x150000..."
$ESPTOOL --chip esp32 --port $PORT --baud 921600 write_flash 0x150000 littlefs.bin
if [ $? -ne 0 ]; then
    echo "Error al subir los juegos. Verifica el puerto."
    exit 1
fi

echo "¡Listo! El firmware y los juegos se han flasheado correctamente."
