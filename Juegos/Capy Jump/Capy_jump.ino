#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

// --- CONFIGURACIÓN DE PINES ---
const int PIN_LEFT   = 15;  
const int PIN_RIGHT  = 27;  
const int PIN_START  = 13;  
const int PIN_SELECT = 33;  
const int PIN_TFT_BL = 21;  

// --- COLORES ---
#define COLOR_BG    tft.color565(15, 15, 35) // Azul noche oscuro
#define COLOR_PLAT  tft.color565(0, 255, 100) // Verde neón
#define COLOR_TRANS 0x0000 // Transparencia

// --- SPRITE DEL CAPIBARA (16x16 RGB565) ---
const uint16_t capybara_sprite[256] PROGMEM = {
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0001, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0x0001, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0001, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0001, 0xA34A, 0xA34A, 0xA34A, 0x0001, 0xA34A, 0xFFFF, 0x0001, 0xA34A, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0001, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0001, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0x0001, 0x0000, 0x0000, 0x0000,
  0x0001, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0x6204, 0x0001, 0x0000, 0x0000,
  0x0001, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0x0001, 0x0000, 0x0000,
  0x0000, 0x0001, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0x6204, 0x6204, 0x0001, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0001, 0x0001, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0xA34A, 0xA34A, 0xA34A, 0xA34A, 0x0001, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

// --- ESTRUCTURA PLATAFORMA ---
struct Platform {
  float x, y;
  int w;
};

// --- VARIABLES DEL JUEGO ---
#define NUM_PLATS 7
Platform plats[NUM_PLATS];

float capyX = 56;
float capyY = 80;
float capyVY = 0;           // Velocidad vertical
const float gravity = 0.22; // Fuerza de la gravedad
const float bounceForce = -5.0; // Fuerza del salto
const float speedX = 3.5;   // Velocidad al moverse izq/der

int score = 0;
int highScore = 0;

int gameState = 0; // 0=Menu, 1=Jugando, 2=GameOver, 3=Pausa
bool isScreenOn = true;
unsigned long lastButtonTime = 0;

void setup() {
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_START, INPUT_PULLUP);
  pinMode(PIN_SELECT, INPUT_PULLUP);
  
  pinMode(PIN_TFT_BL, OUTPUT);
  digitalWrite(PIN_TFT_BL, HIGH);

  tft.init();
  tft.setRotation(2); 
  tft.setSwapBytes(true); 
  
  showMenu();
}

void showMenu() {
  tft.fillScreen(COLOR_BG);
  tft.pushImage(56, 40, 16, 16, capybara_sprite, COLOR_TRANS);
  
  tft.setTextColor(TFT_WHITE, COLOR_BG);
  tft.drawCentreString("CAPY", 64, 15, 2);
  tft.drawCentreString("JUMP", 64, 70, 2);
  
  tft.setTextColor(TFT_YELLOW, COLOR_BG);
  tft.drawCentreString("PULSA START", 64, 110, 1);
  gameState = 0;
}

void initPlats() {
  // La primera plataforma siempre debajo del jugador
  plats[0].x = 48; plats[0].y = 140; plats[0].w = 32; 
  // Generar las demás de forma escalonada hacia arriba
  for(int i=1; i<NUM_PLATS; i++) {
    plats[i].x = random(0, 128 - 24);
    plats[i].y = 140 - (i * 30);
    plats[i].w = random(20, 32);
  }
}

void resetGame() {
  capyX = 56;
  capyY = 120; // Empezamos un poco alto para caer sobre la primera
  capyVY = 0;
  score = 0;
  
  initPlats();
  
  tft.fillScreen(COLOR_BG);
  gameState = 1;
}

void showPause() {
  tft.fillRect(24, 60, 80, 40, TFT_BLACK);
  tft.drawRect(24, 60, 80, 40, TFT_WHITE);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawCentreString("PAUSA", 64, 72, 2);
}

void gameOver() {
  if (score > highScore) highScore = score;
  
    tft.fillRect(14, 40, 100, 60, TFT_BLACK);
    tft.drawRect(14, 40, 100, 60, TFT_WHITE);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawCentreString("GAME OVER", 64, 45, 2);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawCentreString("Score: " + String(score), 64, 65, 1);
    tft.drawCentreString("Best: " + String(highScore), 64, 80, 1);
  delay(1000); 
}

void fullRedraw() {
  tft.fillScreen(COLOR_BG);
  
  for(int i=0; i<NUM_PLATS; i++) {
     tft.fillRect((int)plats[i].x, (int)plats[i].y, plats[i].w, 4, COLOR_PLAT);
  }
  tft.pushImage((int)capyX, (int)capyY, 16, 16, capybara_sprite, COLOR_TRANS);
  
  tft.setTextColor(TFT_WHITE, COLOR_BG);
  tft.drawNumber(score, 4, 4, 1);
}

void updateLogic() {
  // 1. BORRAR POSICIONES ANTIGUAS
    tft.fillRect((int)capyX, (int)capyY, 16, 16, COLOR_BG);
    for(int i=0; i<NUM_PLATS; i++) {
       tft.fillRect((int)plats[i].x, (int)plats[i].y, plats[i].w, 4, COLOR_BG);
  }

  // 2. FÍSICAS: Gravedad y Movimiento Lateral
  capyVY += gravity;
  capyY += capyVY;

  if (digitalRead(PIN_LEFT) == LOW) capyX -= speedX;
  if (digitalRead(PIN_RIGHT) == LOW) capyX += speedX;

  // 3. EFECTO PAC-MAN (Si sales por la izquierda, entras por la derecha)
  if (capyX < -16) capyX = 128;
  if (capyX > 128) capyX = -16;

  // 4. COLISIONES (Solo rebota cuando cae, es decir, capyVY es positivo)
  if (capyVY > 0) {
    for(int i=0; i<NUM_PLATS; i++) {
      // Dejamos un margen suave para que no caiga justo por un píxel
      if (capyX + 12 > plats[i].x && capyX + 4 < plats[i].x + plats[i].w) {
        if (capyY + 16 >= plats[i].y && capyY + 16 <= plats[i].y + 10) {
           capyVY = bounceForce; // ¡REBOTE!
        }
      }
    }
  }

  // 5. SCROLL INFINITO (Si subes más de la mitad de la pantalla, el mundo baja)
  if (capyY < 60) {
    float scrollOffset = 60 - capyY;
    capyY = 60; // El jugador se queda clavado
    score += (int)scrollOffset; // Ganas puntos por subir
    
    for(int i=0; i<NUM_PLATS; i++) {
      plats[i].y += scrollOffset;
      // Si la plataforma sale por debajo, se recicla arriba
      if (plats[i].y > 160) {
        plats[i].y = random(-20, 0);
        plats[i].x = random(0, 128 - 24);
        plats[i].w = random(20, 32);
      }
    }
  }

  // 6. MUERTE (Si te caes por el abismo)
  if (capyY > 160) {
     gameState = 2;
     return;
  }

  // 7. DIBUJAR NUEVAS POSICIONES
    for(int i=0; i<NUM_PLATS; i++) {
       tft.fillRect((int)plats[i].x, (int)plats[i].y, plats[i].w, 4, COLOR_PLAT);
    }
    tft.pushImage((int)capyX, (int)capyY, 16, 16, capybara_sprite, COLOR_TRANS);

    tft.setTextColor(TFT_WHITE, COLOR_BG);
    tft.drawNumber(score, 4, 4, 1);
}

void loop() {
  unsigned long currentMillis = millis();

  // Control de Pantalla (SELECT)
  if (digitalRead(PIN_SELECT) == LOW && currentMillis - lastButtonTime > 300) {
    isScreenOn = !isScreenOn;
    digitalWrite(PIN_TFT_BL, isScreenOn ? HIGH : LOW);
    lastButtonTime = currentMillis;

        if(gameState == 0) showMenu();
        if(gameState == 1) fullRedraw();
        if(gameState == 2) { fullRedraw(); gameOver(); }
        if(gameState == 3) { fullRedraw(); showPause(); }
  }

  if (gameState == 0) { // MENU
    if (digitalRead(PIN_START) == LOW && currentMillis - lastButtonTime > 300) {
      resetGame();
      lastButtonTime = currentMillis;
    }
  } 
  else if (gameState == 1) { // JUGANDO
    if (digitalRead(PIN_START) == LOW && currentMillis - lastButtonTime > 300) {
        gameState = 3;
        showPause();
        lastButtonTime = currentMillis;
        return;
    }

    updateLogic();
    if(gameState == 2) gameOver();
    
    delay(20); // 50 FPS aprox
  } 
  else if (gameState == 2) { // GAME OVER
    if (digitalRead(PIN_START) == LOW && currentMillis - lastButtonTime > 300) {
      while(digitalRead(PIN_START) == LOW) { delay(10); } 
      showMenu();
      lastButtonTime = millis();
    }
  }
  else if (gameState == 3) { // PAUSA
    if (digitalRead(PIN_START) == LOW && currentMillis - lastButtonTime > 300) {
        fullRedraw();
        gameState = 1;
        lastButtonTime = currentMillis;
    }
  }
}
