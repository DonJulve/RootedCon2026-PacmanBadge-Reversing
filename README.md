# Pacman Badge - RootedCon 2026 Reverse Engineering

Este repositorio contiene todo el trabajo de ingeniería inversa, modificación y mejora realizado sobre la placa **Pacman Badge** de la **RootedCon 2026**.

![Pacman Badge Project](https://img.shields.io/badge/RootedCon-2026-yellow?style=for-the-badge)
![Platform](https://img.shields.io/badge/Platform-ESP32-blue?style=for-the-badge)

## 📁 Estructura del Proyecto

El proyecto está dividido en varios componentes clave que cubren desde el hardware original hasta el desarrollo de firmware personalizado:

### 🎮 [Juegos](file:///home/julve/Pacman%20Badge/Juegos)
Convierte tu badge en una **consola arcade portátil**.
- **`Julve_arcade.ino`**: Firmware principal que implementa un sistema de menús y múltiples juegos.
- Incluye: `2048`, `Arkanoid`, `Capy Jump`, `Flappy Capibara`, `Highway Capy`, `Snake`, `Space Invaders` y `Tetris`.
- Cuenta con un binario pre-compilado (`Julve_arcade.bin`) listo para flashear.

### ⚓ [Marauder](file:///home/julve/Pacman%20Badge/Marauder%202.0)
Implementación propia de **ESP32 Marauder** adaptada específicamente para este hardware.
- Versiones disponibles para **Marauder 1.0** y **Marauder 2.0**.
- Capacidades de auditoría WiFi y Bluetooth directamente en el badge.

### ❓ [Preguntas](file:///home/julve/Pacman%20Badge/Preguntas)
Análisis y hackeo del sistema original de desafíos del evento.
- **Emulación del servidor**: Script `server.py` que emula perfectamente el servidor de la RootedCon para enviar preguntas a la placa.
- **Hackeo de claves**: El sistema original ha sido analizado para permitir el cambio de claves y la interceptación de comunicaciones.
- Incluye `backup_preguntas.bin` con el estado del firmware original durante el hacking.

### 🔌 [Descubrimiento de Pines](file:///home/julve/Pacman%20Badge/Descubrimiento%20de%20pines)
Documentación técnica del hardware:
- Mapeo completo de GPIOs para botones y pantalla (`Pines.txt`).
- Configuración de `User_Setup.h` para la librería TFT_eSPI (Pantalla ST7789/ST7735).
- Scripts de prueba para verificar periféricos (`Pantalla.ino`, `Botones.ino`).

---

## 🚀 Guía de Uso

### 📦 Firmware Original (Backup)
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

### 🛠️ Instalación de Juegos / Arcade
Para cargar el sistema arcade:
1. Abre `Julve_arcade.ino` en el IDE de Arduino.
2. Configura la placa como **ESP32 Dev Module**.
3. Asegúrate de tener instaladas las librerías necesarias (TFT_eSPI, etc.).
4. Flashea directamente a la placa.

### 📡 Flasheo de Marauder
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
