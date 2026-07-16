#include <TFT_eSPI.h>
#include <Bounce2.h>
#include "logo.h"
#include "wheel.h"
#include "attr_small.h"
#include "attr_large.h"

TFT_eSPI tft = TFT_eSPI();

const int PIN_UP = 26;
const int PIN_DOWN = 25;
const int PIN_LEFT = 27;
const int PIN_RIGHT = 15;
const int PIN_START = 13; // Botón A (Acción 1)
const int PIN_SELECT = 33; // Botón B (Acción 2)
const int PIN_BL = 21; // Pin de la retroiluminación

Bounce2::Button btnUp = Bounce2::Button();
Bounce2::Button btnDown = Bounce2::Button();
Bounce2::Button btnLeft = Bounce2::Button();
Bounce2::Button btnRight = Bounce2::Button();
Bounce2::Button btnA = Bounce2::Button();
Bounce2::Button btnB = Bounce2::Button();

// 10 pasos de setup + Start Battle + Playing
enum GameState { 
  SETUP_P1_B1, SETUP_P1_B2, SETUP_P1_B3, SETUP_P1_S1, SETUP_P1_S2, 
  SETUP_P2_B1, SETUP_P2_B2, SETUP_P2_B3, SETUP_P2_S1, SETUP_P2_S2, 
  START_BATTLE, PLAYING 
};
GameState currentState = SETUP_P1_B1;

// Colores personalizados
#define COLOR_GOLD   0xFEA0
#define COLOR_SILVER 0xC618
#define COLOR_BRONZE 0xCA22

#define ATTR_PYRUS   TFT_RED
#define ATTR_AQUOS   TFT_BLUE
#define ATTR_SUBTERRA 0x8200 // Marrón
#define ATTR_HAOS    TFT_WHITE
#define ATTR_DARKUS  0x4208 // Gris oscuro
#define ATTR_VENTUS  TFT_GREEN

uint16_t attributes[] = {ATTR_PYRUS, ATTR_AQUOS, ATTR_SUBTERRA, ATTR_HAOS, ATTR_DARKUS, ATTR_VENTUS};
String attrNames[] = {"Pyrus", "Aquos", "Subterra", "Haos", "Darkus", "Ventus"};

struct PlayerState {
  int gPower = 0;
  int gateCardsWon = 0; // Puntuación
  
  // Inventario real (true = disponible, false = gastada)
  bool gateCards[3] = {true, true, true}; 
  bool bakugans[3] = {true, true, true};
  bool abilities[3] = {true, true, true}; 
  bool supports[2] = {true, true};
  
  // Atributos y formas individuales
  int attributeIdx[3] = {0, 0, 0}; 
  int supportAttrs[2] = {0, 0};
  int supportTypes[2] = {0, 0}; // 0=Trampa, 1=Armamento, 2=Vehículo
  
  int cursor = 0; // Navegación de 0 a 12
};
PlayerState players[2];
int activePlayer = 0;

bool needsRedraw = true;
bool fullRedraw = true;
bool fullResetDone = false;

unsigned long lastIncrementTime = 0;
bool isHoldingUp = false;
bool isHoldingDown = false;

void drawSetup();
void drawPlayer(int pIdx);
void drawIcon(int x, int y, int type, uint16_t color, int attrIdx, bool available, bool highlight);

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_BL, OUTPUT);
  digitalWrite(PIN_BL, HIGH);
  
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  
  // Mostrar logo en el arranque
  tft.setSwapBytes(true);
  tft.pushImage(0, 16, 128, 128, bakugan_logo);
  delay(3000); // 3 segundos de logo
  tft.fillScreen(TFT_BLACK);

  btnUp.attach(PIN_UP, INPUT_PULLUP);
  btnDown.attach(PIN_DOWN, INPUT_PULLUP);
  btnLeft.attach(PIN_LEFT, INPUT_PULLUP);
  btnRight.attach(PIN_RIGHT, INPUT_PULLUP);
  btnA.attach(PIN_START, INPUT_PULLUP);
  btnB.attach(PIN_SELECT, INPUT_PULLUP);

  btnUp.setPressedState(LOW);
  btnDown.setPressedState(LOW);
  btnLeft.setPressedState(LOW);
  btnRight.setPressedState(LOW);
  btnA.setPressedState(LOW);
  btnB.setPressedState(LOW);

  btnUp.interval(15);
  btnDown.interval(15);
  btnLeft.interval(15);
  btnRight.interval(15);
  btnA.interval(15);
  btnB.interval(15);
}

void modifyCursorItem(int delta) {
  if (players[0].gateCardsWon >= 3 || players[1].gateCardsWon >= 3) return;
  
  PlayerState& p = players[activePlayer];
  if (p.cursor == 0) { // Poder G
    p.gPower += delta;
    if (p.gPower < 0) p.gPower = 0;
    return;
  }

  bool setAvailable = (delta > 0);
  fullRedraw = true;

  if (p.cursor == 1) { // Puntuación
    p.gateCardsWon += (delta > 0) ? 1 : -1;
    p.gateCardsWon = constrain(p.gateCardsWon, 0, 3);
  }
  else if (p.cursor >= 2 && p.cursor <= 4) { // Cartas Portal
    p.gateCards[p.cursor - 2] = setAvailable;
  }
  else if (p.cursor >= 5 && p.cursor <= 7) { // Bakugans
    p.bakugans[p.cursor - 5] = setAvailable;
    
    // Auto-recarga de Bakugans y soportes al quedarse a 0
    if (!setAvailable && !p.bakugans[0] && !p.bakugans[1] && !p.bakugans[2]) {
      p.bakugans[0] = true;
      p.bakugans[1] = true;
      p.bakugans[2] = true;
      p.supports[0] = true;
      p.supports[1] = true;
    }
  }
  else if (p.cursor >= 8 && p.cursor <= 10) { // Cartas Poder
    p.abilities[p.cursor - 8] = setAvailable;
  }
  else if (p.cursor >= 11 && p.cursor <= 12) { // Soportes
    p.supports[p.cursor - 11] = setAvailable;
  }
}

void loop() {
  btnUp.update();
  btnDown.update();
  btnLeft.update();
  btnRight.update();
  btnA.update();
  btnB.update();

  // FASE DE CONFIGURACIÓN INICIAL
  if (currentState != PLAYING && currentState != START_BATTLE) {
    int pIdx = (currentState <= SETUP_P1_S2) ? 0 : 1;
    bool isSupport = (currentState == SETUP_P1_S1 || currentState == SETUP_P1_S2 || 
                      currentState == SETUP_P2_S1 || currentState == SETUP_P2_S2);
    
    int bIdx = 0;
    if (currentState == SETUP_P1_B1 || currentState == SETUP_P2_B1) bIdx = 0;
    if (currentState == SETUP_P1_B2 || currentState == SETUP_P2_B2) bIdx = 1;
    if (currentState == SETUP_P1_B3 || currentState == SETUP_P2_B3) bIdx = 2;
    
    int sIdx = 0;
    if (currentState == SETUP_P1_S1 || currentState == SETUP_P2_S1) sIdx = 0;
    if (currentState == SETUP_P1_S2 || currentState == SETUP_P2_S2) sIdx = 1;

    // Arriba/Abajo cambian el color
    if (btnUp.pressed()) {
      if (!isSupport) {
        players[pIdx].attributeIdx[bIdx]--;
        if (players[pIdx].attributeIdx[bIdx] < 0) players[pIdx].attributeIdx[bIdx] = 5;
      } else {
        players[pIdx].supportAttrs[sIdx]--;
        if (players[pIdx].supportAttrs[sIdx] < 0) players[pIdx].supportAttrs[sIdx] = 5;
      }
      needsRedraw = true;
    }
    if (btnDown.pressed()) {
      if (!isSupport) {
        players[pIdx].attributeIdx[bIdx]++;
        if (players[pIdx].attributeIdx[bIdx] > 5) players[pIdx].attributeIdx[bIdx] = 0;
      } else {
        players[pIdx].supportAttrs[sIdx]++;
        if (players[pIdx].supportAttrs[sIdx] > 5) players[pIdx].supportAttrs[sIdx] = 0;
      }
      needsRedraw = true;
    }

    // Izquierda/Derecha cambian la forma (solo en Soportes)
    if (btnLeft.pressed() && isSupport) {
       players[pIdx].supportTypes[sIdx]--;
       if (players[pIdx].supportTypes[sIdx] < 0) players[pIdx].supportTypes[sIdx] = 2;
       needsRedraw = true;
    }
    if (btnRight.pressed() && isSupport) {
       players[pIdx].supportTypes[sIdx]++;
       if (players[pIdx].supportTypes[sIdx] > 2) players[pIdx].supportTypes[sIdx] = 0;
       needsRedraw = true;
    }

    if (btnA.pressed()) { 
      currentState = (GameState)(currentState + 1); 
      if (currentState == START_BATTLE) {
        tft.fillScreen(TFT_BLACK);
        tft.setSwapBytes(true);
        tft.pushImage(0, 16, 128, 128, bakugan_wheel);
        delay(2000);
        currentState = PLAYING; 
        activePlayer = 0;
      }
      fullRedraw = true;
      needsRedraw = true;
    }
    
    if (needsRedraw && currentState != PLAYING) {
      drawSetup();
      needsRedraw = false;
    }
    return;
  }

  // FASE DE JUEGO (PLAYING)
  
  if (btnLeft.pressed()) {
    players[activePlayer].cursor--;
    if (players[activePlayer].cursor < 0) players[activePlayer].cursor = 12;
    fullRedraw = true;
    needsRedraw = true;
  }
  if (btnRight.pressed()) {
    players[activePlayer].cursor++;
    if (players[activePlayer].cursor > 12) players[activePlayer].cursor = 0;
    fullRedraw = true;
    needsRedraw = true;
  }
  
  if (btnA.pressed()) {
    activePlayer = 1 - activePlayer;
    fullRedraw = true;
    needsRedraw = true;
  }
  
  if (btnB.pressed()) {
    players[0].gPower = 0;
    players[1].gPower = 0;
    fullRedraw = true;
    needsRedraw = true;
  }
  if (btnB.isPressed() && btnB.currentDuration() > 2000) {
    if (!fullResetDone) {
      // Soft-Reset: Mantiene los atributos, resetea estado de la partida
      for (int i=0; i<2; i++) {
        players[i].gPower = 0;
        players[i].gateCardsWon = 0;
        for(int j=0; j<3; j++) {
           players[i].gateCards[j] = true;
           players[i].bakugans[j] = true;
           players[i].abilities[j] = true;
        }
        players[i].supports[0] = true;
        players[i].supports[1] = true;
        players[i].cursor = 0;
      }
      activePlayer = 0;
      
      tft.fillScreen(TFT_RED); 
      delay(200);
      
      tft.fillScreen(TFT_BLACK);
      tft.setSwapBytes(true);
      tft.pushImage(0, 16, 128, 128, bakugan_wheel);
      delay(2000);
      
      fullRedraw = true;
      needsRedraw = true;
      fullResetDone = true;
    }
  }
  if (btnB.released()) {
    fullResetDone = false;
  }

  if (btnUp.pressed()) { 
    modifyCursorItem(10); 
    isHoldingUp = true; 
    lastIncrementTime = millis(); 
    needsRedraw = true; 
  }
  if (btnUp.released()) { isHoldingUp = false; }
  
  if (btnUp.isPressed() && isHoldingUp && players[activePlayer].cursor == 0) {
    if (millis() - lastIncrementTime > 400) {
      static unsigned long lastTick = 0;
      if (millis() - lastTick > 100) {
        lastTick = millis();
        modifyCursorItem(50);
        needsRedraw = true;
      }
    }
  }

  if (btnDown.pressed()) { 
    modifyCursorItem(-10); 
    isHoldingDown = true; 
    lastIncrementTime = millis(); 
    needsRedraw = true; 
  }
  if (btnDown.released()) { isHoldingDown = false; }
  
  if (btnDown.isPressed() && isHoldingDown && players[activePlayer].cursor == 0) {
    if (millis() - lastIncrementTime > 400) {
      static unsigned long lastTick = 0;
      if (millis() - lastTick > 100) {
        lastTick = millis();
        modifyCursorItem(-50);
        needsRedraw = true;
      }
    }
  }

  if (needsRedraw) {
    drawPlayer(activePlayer);
    needsRedraw = false;
  }
}

void drawSetup() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  int pIdx = (currentState <= SETUP_P1_S2) ? 0 : 1;
  bool isSupport = (currentState == SETUP_P1_S1 || currentState == SETUP_P1_S2 || 
                    currentState == SETUP_P2_S1 || currentState == SETUP_P2_S2);

  int bIdx = 0;
  if (currentState == SETUP_P1_B1 || currentState == SETUP_P2_B1) bIdx = 0;
  if (currentState == SETUP_P1_B2 || currentState == SETUP_P2_B2) bIdx = 1;
  if (currentState == SETUP_P1_B3 || currentState == SETUP_P2_B3) bIdx = 2;
  int sIdx = 0;
  if (currentState == SETUP_P1_S1 || currentState == SETUP_P2_S1) sIdx = 0;
  if (currentState == SETUP_P1_S2 || currentState == SETUP_P2_S2) sIdx = 1;

  String title = (pIdx == 0) ? "YO - " : "RIVAL - ";
  title += isSupport ? ("Soporte " + String(sIdx + 1)) : ("Bakugan " + String(bIdx + 1));
  tft.drawString(title, 64, 30, 2);
  
  uint16_t color = isSupport ? attributes[players[pIdx].supportAttrs[sIdx]] : attributes[players[pIdx].attributeIdx[bIdx]];
  String name = isSupport ? attrNames[players[pIdx].supportAttrs[sIdx]] : attrNames[players[pIdx].attributeIdx[bIdx]];
  
  if (isSupport) {
     int type = players[pIdx].supportTypes[sIdx];
     String typeName = (type == 0) ? "Trampa" : (type == 1) ? "Armamento" : "Vehiculo";
     
     if (type == 0) drawSpider(64, 80, 20, color, true);
     else if (type == 1) drawSwordAndShield(64, 80, 20, color, true);
     else if (type == 2) drawTank(64, 80, 20, color, true);
     
     tft.setTextColor(TFT_WHITE, TFT_BLACK);
     tft.drawString(typeName, 64, 110, 2);
  } else {
     tft.setSwapBytes(true);
     tft.pushImage(44, 60, 40, 40, attr_large_images[players[pIdx].attributeIdx[bIdx]]);
  }
  
  tft.setTextColor(color, TFT_BLACK);
  tft.drawString(name, 64, 130, 2);
  
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  if (isSupport) {
    tft.drawString("Iz/De: Forma", 64, 145, 1);
    tft.drawString("A para confirmar", 64, 155, 1);
  } else {
    tft.drawString("A para confirmar", 64, 150, 1);
  }
}

void drawSpider(int cx, int cy, int r, uint16_t color, bool filled) {
  int body_r = max(2, (r * 5) / 10);
  int head_r = max(1, (r * 3) / 10);
  
  // Cuerpo
  if (filled) {
    tft.fillCircle(cx, cy + (r/4), body_r, color);
    tft.fillCircle(cx, cy - (r/4), head_r, color);
  } else {
    tft.drawCircle(cx, cy + (r/4), body_r, color);
    tft.drawCircle(cx, cy - (r/4), head_r, color);
  }
  
  // Patas (radiando desde el centro)
  int bx = cx;
  int by = cy; 
  
  // Izquierda
  tft.drawLine(bx - (body_r/2), by - (r/4), cx - r, cy - r, color);
  tft.drawLine(bx - body_r, by, cx - r, cy - (r/3), color);
  tft.drawLine(bx - body_r, by + (r/4), cx - r, cy + (r/3), color);
  tft.drawLine(bx - (body_r/2), by + (r/2), cx - r, cy + r, color);

  // Derecha
  tft.drawLine(bx + (body_r/2), by - (r/4), cx + r, cy - r, color);
  tft.drawLine(bx + body_r, by, cx + r, cy - (r/3), color);
  tft.drawLine(bx + body_r, by + (r/4), cx + r, cy + (r/3), color);
  tft.drawLine(bx + (body_r/2), by + (r/2), cx + r, cy + r, color);
  
  // Engrosar patas si es la vista grande
  if (filled && r > 10) {
    tft.drawLine(bx - (body_r/2), by - (r/4) + 1, cx - r, cy - r + 1, color);
    tft.drawLine(bx - body_r, by + 1, cx - r, cy - (r/3) + 1, color);
    tft.drawLine(bx - body_r, by + (r/4) + 1, cx - r, cy + (r/3) + 1, color);
    tft.drawLine(bx - (body_r/2), by + (r/2) + 1, cx - r, cy + r + 1, color);
    
    tft.drawLine(bx + (body_r/2), by - (r/4) + 1, cx + r, cy - r + 1, color);
    tft.drawLine(bx + body_r, by + 1, cx + r, cy - (r/3) + 1, color);
    tft.drawLine(bx + body_r, by + (r/4) + 1, cx + r, cy + (r/3) + 1, color);
    tft.drawLine(bx + (body_r/2), by + (r/2) + 1, cx + r, cy + r + 1, color);
  }
}

void drawSwordAndShield(int cx, int cy, int r, uint16_t color, bool filled) {
  int ymid = (r * 2) / 10;
  int sr = (r * 7) / 10; // Shield radius (smaller than r so sword is visible)
  
  // Sword (Blade from bottom-left to top-right)
  tft.drawLine(cx - r, cy + r, cx + r, cy - r, color);
  tft.drawLine(cx - r + (r/3), cy + r, cx - r, cy + r - (r/3), color); // Crossguard
  if (filled) {
     tft.drawLine(cx - r + 1, cy + r, cx + r, cy - r + 1, color);
     tft.drawLine(cx - r, cy + r - 1, cx + r - 1, cy - r, color);
  }

  // Shield
  if (filled) {
    tft.fillRect(cx - sr, cy - sr, sr*2 + 1, sr + ymid + 1, color);
    tft.fillTriangle(cx - sr, cy + ymid, cx + sr, cy + ymid, cx, cy + sr + (r/3), color);
  } else {
    tft.drawLine(cx - sr, cy - sr, cx + sr, cy - sr, color);
    tft.drawLine(cx - sr, cy - sr, cx - sr, cy + ymid, color);
    tft.drawLine(cx + sr, cy - sr, cx + sr, cy + ymid, color);
    tft.drawLine(cx - sr, cy + ymid, cx, cy + sr + (r/3), color);
    tft.drawLine(cx + sr, cy + ymid, cx, cy + sr + (r/3), color);
  }
}

void drawTank(int cx, int cy, int r, uint16_t color, bool filled) {
  int w2 = (r * 6) / 10;
  int h2 = (r * 5) / 10;
  int cannon_w = r;
  int cannon_h = max(1, r / 5);
  int offset = r / 4;
  
  if (filled) {
    tft.fillRoundRect(cx - r, cy + offset, r*2 + 1, (r*3/4) + 1, offset, color); // Orugas
    tft.fillRect(cx - w2, cy - h2 + offset, w2*2 + 1, h2 + 1, color); // Cabina
    tft.fillRect(cx, cy - h2 + offset + 1, cannon_w + 1, cannon_h, color); // Cañón fino
  } else {
    tft.drawRoundRect(cx - r, cy + offset, r*2 + 1, (r*3/4) + 1, offset, color);
    tft.drawRect(cx - w2, cy - h2 + offset, w2*2 + 1, h2 + 1, color);
    tft.drawRect(cx, cy - h2 + offset + 1, cannon_w + 1, cannon_h, color);
  }
}

void drawIcon(int x, int y, int type, uint16_t color, int attrIdx, bool available, bool highlight) {
  if (highlight) {
    tft.drawRect(x-2, y-8, 14, 18, TFT_GREEN);
  }
  if (type == 0) { // Rectángulo
    if (available) tft.fillRect(x, y-6, 10, 14, color);
    else tft.drawRect(x, y-6, 10, 14, TFT_DARKGREY);
  } else if (type == 1) { // Imagen Bakugan
    if (available) {
      tft.setSwapBytes(true);
      if (attrIdx >= 0 && attrIdx <= 5) {
        tft.pushImage(x, y-5, 10, 10, attr_small_images[attrIdx]);
      } else {
        tft.fillCircle(x+5, y, 5, color);
      }
    }
    else tft.drawCircle(x+5, y, 5, TFT_DARKGREY);
  } else if (type == 2) { // Trampa (Araña)
    drawSpider(x+5, y, 5, available ? color : TFT_DARKGREY, available);
  } else if (type == 3) { // Armamento (Espada y Escudo)
    drawSwordAndShield(x+5, y, 5, available ? color : TFT_DARKGREY, available);
  } else if (type == 4) { // Vehículo (Tanque)
    drawTank(x+5, y, 5, available ? color : TFT_DARKGREY, available);
  }
}

void drawPlayer(int pIdx) {
  PlayerState& p = players[pIdx];

  if (players[0].gateCardsWon >= 3 || players[1].gateCardsWon >= 3) {
     if (fullRedraw) {
       tft.fillScreen(TFT_BLACK);
       tft.setTextColor(TFT_YELLOW, TFT_BLACK);
       tft.setTextDatum(MC_DATUM);
       
       if (players[0].gateCardsWon >= 3) {
         tft.drawString("!YO GANO!", 64, 50, 4);
       } else {
         tft.drawString("!RIVAL", 64, 40, 4);
         tft.drawString("GANA!", 64, 66, 4);
       }
       
       tft.setTextColor(TFT_WHITE, TFT_BLACK);
       tft.drawString("Manten B", 64, 105, 2);
       tft.drawString("para reset", 64, 125, 2);
     }
     fullRedraw = false;
     return;
  }

  if (fullRedraw) {
    tft.fillScreen(TFT_BLACK);
    
    tft.setTextColor(pIdx == 0 ? TFT_GREEN : TFT_RED, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(pIdx == 0 ? "YO" : "RIVAL", 64, 5, 4);

    tft.setTextDatum(TL_DATUM);

    // Fila 1: Puntuación -> Cursor 1
    if (p.cursor == 1) tft.setTextColor(TFT_GREEN, TFT_BLACK); 
    else tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString("Ganadas:", 2, 85, 2);
    for (int i=0; i<3; i++) {
      int x = 60 + i*16;
      bool won = (i < p.gateCardsWon); 
      if (won) tft.fillRect(x, 86, 10, 14, TFT_YELLOW);
      else tft.drawRect(x, 86, 10, 14, TFT_DARKGREY);
    }
    if (p.cursor == 1) tft.drawRect(56, 85, 52, 16, TFT_GREEN); 

    // Fila 2: Cartas Portal -> Cursor 2, 3, 4
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString("Portal:", 2, 105, 2);
    drawIcon(60, 113, 0, COLOR_GOLD, -1, p.gateCards[0], p.cursor == 2);
    drawIcon(76, 113, 0, COLOR_SILVER, -1, p.gateCards[1], p.cursor == 3);
    drawIcon(92, 113, 0, COLOR_BRONZE, -1, p.gateCards[2], p.cursor == 4);

    // Fila 3: Bakugans -> Cursor 5, 6, 7
    tft.drawString("Bakugan:", 2, 125, 2);
    drawIcon(60, 133, 1, attributes[p.attributeIdx[0]], p.attributeIdx[0], p.bakugans[0], p.cursor == 5);
    drawIcon(76, 133, 1, attributes[p.attributeIdx[1]], p.attributeIdx[1], p.bakugans[1], p.cursor == 6);
    drawIcon(92, 133, 1, attributes[p.attributeIdx[2]], p.attributeIdx[2], p.bakugans[2], p.cursor == 7);

    // Fila 4: Poder + Soporte -> Cursor 8,9,10 y 11,12
    tft.drawString("Mano:", 2, 145, 2);
    drawIcon(45, 153, 0, TFT_RED, -1, p.abilities[0], p.cursor == 8);
    drawIcon(60, 153, 0, TFT_GREEN, -1, p.abilities[1], p.cursor == 9);
    drawIcon(75, 153, 0, TFT_BLUE, -1, p.abilities[2], p.cursor == 10);
    
    drawIcon(95, 153, p.supportTypes[0] + 2, attributes[p.supportAttrs[0]], p.supportAttrs[0], p.supports[0], p.cursor == 11);
    drawIcon(110, 153, p.supportTypes[1] + 2, attributes[p.supportAttrs[1]], p.supportAttrs[1], p.supports[1], p.cursor == 12);
  }

  // Poder G
  if (p.cursor == 0) {
    if (fullRedraw) tft.fillRect(4, 38, 120, 36, TFT_GREEN);
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
  } else {
    if (fullRedraw) tft.fillRect(4, 38, 120, 36, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
  }
  
  tft.setTextPadding(110); 
  tft.setTextDatum(MC_DATUM);
  tft.drawString(String(p.gPower) + " G", 64, 56, 4);
  tft.setTextPadding(0);

  fullRedraw = false;
}
