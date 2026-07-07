# Pacman Badge - RootedCon 2026 Reverse Engineering

Este repositorio contiene todo el trabajo de ingeniería inversa, modificación y mejora realizado sobre la placa **Pacman Badge** de la **RootedCon 2026**.

![Pacman Badge Project](https://img.shields.io/badge/RootedCon-2026-yellow?style=for-the-badge)
![Platform](https://img.shields.io/badge/Platform-ESP32-blue?style=for-the-badge)

## 📁 Estructura del Proyecto

El proyecto está dividido en varios componentes clave que cubren desde el hardware original hasta el desarrollo de firmware personalizado:

### Juegos
Convierte tu badge en una **consola arcade portátil**.
- **`Julve_arcade.ino`**: Firmware principal que implementa un sistema de menús y múltiples juegos.
- Incluye: `2048`, `Arkanoid`, `Capy Jump`, `Flappy Capibara`, `Highway Capy`, `Snake`, `Space Invaders` y `Tetris`.
- Cuenta con un binario pre-compilado (`Julve_arcade.bin`) listo para flashear.

### Marauder
Implementación propia de **ESP32 Marauder** adaptada específicamente para este hardware.
- Versiones disponibles para **Marauder 1.0** y **Marauder 2.0**.
- Capacidades de auditoría WiFi y Bluetooth directamente en el badge.

### Emulador Gameboy
Port completo de un emulador de Gameboy para el hardware específico de la RootedCon.
- Basado en el excelente proyecto [esp32-gameboy](https://github.com/lualiliu/esp32-gameboy) de **lualiliu**.
- Configuración de pines adaptada al D-pad y botones de la placa.
- Pantalla rotada 180 grados y corrección de colores para el driver ST7735/ST7789 configurada.
- Arreglo de pull-ups internos para los botones y combinación de botones virtuales (el botón *Start* se activa pulsando *A + B + UP* a la vez, y *Select* con *A + B + DOWN* debido a la limitación de botones físicos).

### Preguntas
Análisis y hackeo del sistema original de desafíos del evento.
- **Emulación del servidor**: Script `server.py` que emula perfectamente el servidor de la RootedCon para enviar preguntas a la placa.
- **Hackeo de claves**: El sistema original ha sido analizado para permitir el cambio de claves y la interceptación de comunicaciones.
- Incluye `backup_preguntas.bin` con el estado del firmware original durante el hacking.

### Descubrimiento de Pines
Documentación técnica del hardware:
- Mapeo completo de GPIOs para botones y pantalla (`Pines.txt`).
- Configuración de `User_Setup.h` para la librería TFT_eSPI (Pantalla ST7789/ST7735).
- Scripts de prueba para verificar periféricos (`Pantalla.ino`, `Botones.ino`).

---

## Guía de Uso

### Firmware Original (Backup)
Si deseas volver al estado oficial o investigar el firmware de fábrica:
- **`backup_full.bin`**: Volcado completo (8MB) de la memoria flash original.

**Para dumpear el backup:**
```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 read_flash 0 ALL backup_full.bin
```

**Para restaurar el backup:**
```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 write_flash 0x0 backup_full.bin
```

### Instalación de Juegos / Arcade
Para cargar el sistema arcade:
1. Abre `Julve_arcade.ino` en el IDE de Arduino.
2. Configura la placa como **ESP32 Dev Module**.
3. Asegúrate de tener instaladas las librerías necesarias (TFT_eSPI, etc.).
4. Flashea directamente a la placa.

### Emulador de Gameboy
Para jugar a juegos de Gameboy en la placa, sigue estos pasos detallados:

#### 1. Preparar la ROM del juego
1. Navega a la carpeta `esp32-gameboy`.
2. Consigue una ROM original de Gameboy Clásica (extensión `.gb`). *(Nota: Este emulador no es compatible con juegos de Gameboy Color `.gbc` ni Gameboy Advance `.gba`)*.
3. Convierte el archivo `.gb` a un archivo de cabecera de C (`gbrom.h`) que el ESP32 pueda compilar junto al código. Para ello, usa el script de Python proporcionado:
   ```bash
   # Sintaxis: python3 ./bin2h.py -b <path to your GD ROM you downloaded> -c gbrom.h -v gb_rom
   python3 ./bin2h.py -b "Kirby's_Dream_Land.gb" -c gbrom.h -v gb_rom
   ```
   *Esto generará el archivo `gbrom.h` (puede pesar varios megabytes, es normal) que contiene el código en hexadecimal del juego. **Asegúrate de que la variable final sea exactamente `gb_rom`**.*

#### 2. Compilar y Flashear
Debido a que la ROM del juego se incrusta directamente en el código del programa, el tamaño superará la partición por defecto del ESP32. **Es obligatorio cambiar el esquema de particiones a "Huge APP (3MB No OTA)"**.

**Opción A: Desde el IDE de Arduino**
1. Abre `esp32-gameboy/esp32-gameboy.ino`.
2. En el menú, ve a `Herramientas` -> `Partition Scheme` y selecciona **Huge APP (3MB No OTA/1MB SPIFFS)**.
3. Asegúrate de tener instalada la librería **GFX Library for Arduino** (probado con v1.6.6) desde el Gestor de Librerías.
4. Dale a Subir.

**Opción B: Desde línea de comandos (`arduino-cli`)**
```bash
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=huge_app esp32-gameboy.ino
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32:PartitionScheme=huge_app esp32-gameboy.ino
```

#### 3. Controles en la Badge
Dado que la placa de la RootedCon carece de algunos botones dedicados que tendría una Gameboy real, los controles se han adaptado (orientación: placa boca abajo):
- **Cruceta (D-Pad)**: Mapeada a los botones direccionales de la placa.
- **A / B**: Mapeados a los pulsadores de acción disponibles.
- **Start / Select**: Como faltan botones físicos en la placa, usa estas combinaciones:
  - Presiona a la vez **A + B + UP (Arriba)** para pulsar *Start*.
  - Presiona a la vez **A + B + DOWN (Abajo)** para pulsar *Select*.
- **Rotar Pantalla (Modo Demo / Jugar)**: Puedes girar la pantalla 180 grados en cualquier momento (ideal para llevar la placa colgando del cuello y luego girarla para jugar). Solo tienes que **mantener pulsados a la vez los 4 botones de la cruceta + A + B durante 3 segundos**.

### Flasheo de Marauder
Utiliza el script documentado en `flash_marauder.txt` o mediante `arduino-cli`:
```bash
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=huge_app Descargas.ino
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32:PartitionScheme=huge_app Descargas.ino
```

---

## 🛠️ Especificaciones Técnicas
- **MCU**: ESP32 (WROOM module).
- **Pantalla**: TFT Color (Librería configurada en `User_Setup.h`).
- **Input**: Botones físicos mapeados en la carpeta de descubrimiento de pines (Teniendo en cuenta que es poniendo la badge boca abajo).

---
*Desarrollado y reverseado por DonJulve durante la RootedCon 2026.*
