# Pacman Badge - RootedCon 2026 Reverse Engineering

Este repositorio contiene todo el trabajo de ingeniería inversa, modificación y mejora realizado sobre la placa **Pacman Badge** de la **RootedCon 2026**.

![Pacman Badge Project](https://img.shields.io/badge/RootedCon-2026-yellow?style=for-the-badge)
![Platform](https://img.shields.io/badge/Platform-ESP32-blue?style=for-the-badge)

## Estructura del Proyecto

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

### Mando del Robot
Controlador para el evento de pelea de robots de la RootedCon.
- Utiliza la badge como un mando a distancia por WiFi UDP para controlar los motores y servos.
- Código fuente disponible en la carpeta `Mando robot`.

### Asistente Bakugan
Un avanzado asistente de combate interactivo para jugar a Bakugan Battle Brawlers en mesa real.
- **`Asistente_Bakugan.ino`**: Firmware que maneja estados de juego, puntuaciones y el inventario.
- Soporta configuración individualizada de atributos, con gráficos en tiempo real para Bakugans (esferas) y piezas de soporte (Trampas como Arañas, Armamentos como Espada/Escudo y Vehículos como Tanques).
- Incluye animaciones personalizadas (Logo oficial y Rueda de atributos) procesadas desde Python al ESP32 mediante el script `conversor_imagenes.py`.
- Cuenta con un binario pre-compilado unificado (`Asistente_Bakugan.bin`) listo para flashear.

### Emulador Gameboy
Port completo de un emulador de Gameboy para el hardware específico de la RootedCon. Incorpora un **menú multijuegos impulsado por LittleFS** y carga ultrarrápida mediante **Memory Mapping** nativo.
- Basado en el excelente proyecto [esp32-gameboy](https://github.com/lualiliu/esp32-gameboy) de **lualiliu**.
- Configuración de pines adaptada al D-pad y botones de la placa.
- Pantalla rotada 180 grados y corrección de colores para el driver ST7735/ST7789 configurada.
- Arreglo de pull-ups internos para los botones y combinación de botones virtuales (el botón *Start* se activa pulsando *A + B + UP* a la vez, y *Select* con *A + B + DOWN* debido a la limitación de botones físicos).
- Menú de selección interactivo en el que alojar varias ROMs sin necesidad de recompilar.

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

### Mando del Robot
Para controlar un robot en la pelea de robots del evento, es necesario configurar las credenciales correctas para conectarse a tu robot:
1. Abre el archivo `Mando robot/Mando_robot.ino` en el IDE de Arduino.
2. Localiza las variables de configuración de red (alrededor de la línea 15):
   ```cpp
   const char* ssid = "Capibot";     
   const char* password = "capibotcapibot";    
   ```
3. Cambia el `ssid` y `password` por las credenciales WiFi específicas de tu robot.
4. Flashea el código a tu badge. Utiliza los botones direccionales para moverte y los botones Start/Select virtuales para las acciones del servo.

### Asistente Bakugan
Para flashear el asistente de combate a la badge, necesitas el esquema de partición extendido debido a las altas resoluciones de las imágenes incluidas (Logo y Rueda de Atributos).
1. Sitúate en el directorio `Asistente_Bakugan`.
2. *(Opcional)* Si cambias las imágenes (`logo.png` o `wheel.png`), ejecuta antes `python3 conversor_imagenes.py` para regenerar las cabeceras `.h`.
3. Flashea el código a la placa indicando el particionado `huge_app`:
   ```bash
   arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=huge_app Asistente_Bakugan.ino
   
   arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32:PartitionScheme=huge_app Asistente_Bakugan.ino
   ```
   **Alternativa rápida (Binario pre-compilado):**
   Si prefieres no compilar, puedes flashear directamente el binario unificado con `esptool.py`:
   ```bash
   esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 write_flash 0x0 Asistente_Bakugan.bin
   ```
4. **Controles en partida:**
   - **Cruceta (Fase de Setup):** 
     - **Arriba / Abajo:** Cambia el Atributo (color) del Bakugan o pieza de soporte.
     - **Izquierda / Derecha:** Selecciona la forma geométrica para la pieza de soporte (Trampa, Armamento, Vehículo).
   - **Cruceta (En partida):**
     - **Izquierda / Derecha:** Mueve el cursor a través de los 14 objetos de tu inventario.
     - **Arriba / Abajo:** *Abajo* gasta el objeto seleccionado (lo apaga). *Arriba* lo recupera.
   - **Botón A:** Confirma tu selección en el Setup inicial y alterna entre la pantalla de **"Yo"** y **"Rival"** durante el combate.
   - **Botón B:** 
     - **Pulsación leve:** Pone a 0 el Poder G (ideal para empezar una nueva ronda dentro de la misma partida).
     - **Mantener 2 segundos:** Hace un **Soft-Reset** (reinicia la partida y recupera todo el inventario al instante para empezar una revancha sin volver a elegir colores). Para reconfigurar atributos desde 0, apaga y enciende la placa físicamente.

### Emulador de Gameboy
Para jugar a juegos de Gameboy en la placa, sigue estos pasos detallados:

#### 1. Preparar las ROMs
1. Navega a la carpeta `Emulador_Gameboy`.
2. Crea una carpeta llamada `data` en el interior (si no existe) y copia ahí todas tus ROMs de Gameboy Clásica (extensión `.gb`).
   *(Nota: Este emulador solo es compatible con juegos de Gameboy clásica, no emula la Gameboy Color).*

#### 2. Compilar, Flashear y Subir Juegos
Hemos automatizado el proceso con un script para flashear el código, el sistema de particiones y los propios juegos empaquetados en LittleFS.

**Desde línea de comandos (Recomendado):**
Ejecuta el script proporcionado indicando el puerto de tu placa (por defecto `/dev/ttyUSB0`):
```bash
cd Emulador_Gameboy
chmod +x upload.sh
./upload.sh /dev/ttyUSB0
```
*Este script compila el emulador con el esquema de particiones custom, lo flashea, empaqueta la carpeta `data` usando `mklittlefs` y la sube al ESP32 vía `esptool`.*

> **Nota para contribuidores/clonadores:** El script `upload.sh` está preparado para repositorios genéricos. Busca automáticamente las rutas de `mklittlefs` y `esptool.py` en las carpetas de instalación estándar de Arduino (Linux/macOS) o en tu PATH global. No necesitas editar las rutas del script.

**Desde el IDE de Arduino:**
1. Abre `Emulador_Gameboy.ino`.
2. En `Herramientas` -> `Partition Scheme`, selecciona **Custom Partition Scheme (CSV)**.
3. Sube el sketch dándole al botón normal de subir.
4. Para subir los juegos, usa la herramienta **ESP32 LittleFS Data Upload** del IDE.

#### 3. Controles en la Badge
Dado que la placa de la RootedCon carece de botones dedicados, los controles se han adaptado:
- **Menú Inicial:** Utiliza la cruceta para navegar por tu lista de ROMs y **B** (acción secundaria) para lanzar el juego.
- **Cruceta (D-Pad)**: Mapeada a los botones direccionales de la placa.
- **A / B**: Mapeados a los pulsadores de acción.
- **Start / Select virtuales**: 
  - Presiona a la vez **A + B + UP (Arriba)** para pulsar *Start*.
  - Presiona a la vez **A + B + DOWN (Abajo)** para pulsar *Select*.
- **Rotar Pantalla**: Puedes girar la pantalla 180 grados (ideal para llevar la placa colgada). Mantén pulsados **Izquierda + Derecha + A + B durante 3 segundos**.

### Flasheo de Marauder
Utiliza el script documentado en `flash_marauder.txt` o mediante `arduino-cli`:
```bash
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=huge_app Descargas.ino

arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32:PartitionScheme=huge_app Descargas.ino
```

---

## Especificaciones Técnicas
- **MCU**: ESP32 (WROOM module).
- **Pantalla**: TFT Color (Librería configurada en `User_Setup.h`).
- **Input**: Botones físicos mapeados en la carpeta de descubrimiento de pines (Teniendo en cuenta que es poniendo la badge boca abajo).

---
*Desarrollado y reverseado por DonJulve durante la RootedCon 2026.*
