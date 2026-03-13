#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

// --- CONFIGURACIÓN DE PINES ---
const int PIN_LEFT   = 15;  
const int PIN_RIGHT  = 27;  
const int PIN_SHOOT  = 25;  
const int PIN_START  = 13;  
const int PIN_SELECT = 33;  
const int PIN_TFT_BL = 21;  

// --- COLORES ---
#define COLOR_BG      TFT_BLACK

// --- SPRITES (16x16 RGB565) ---
const uint16_t sprite_ship[256] PROGMEM = {
  0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x07E0,0x07E0,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x07E0,0x07E0,0x07E0,0x07E0,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x07E0,0xCE59,0xCE59,0x07E0,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x0000,0x0000,0x0000,0x07E0,0x07E0,0xCE59,0xCE59,0x07E0,0x07E0,0x0000,0x0000,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x0000,0x0000,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x0000,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x0000,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x07E0,0x07E0,0x07E0,0xCE59,0xCE59,0xCE59,0xCE59,0xCE59,0xCE59,0x07E0,0x07E0,0x07E0,0x0000,0x0000,
  0x0000,0x07E0,0x07E0,0x07E0,0x07E0,0xCE59,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xCE59,0x07E0,0x07E0,0x07E0,0x07E0,0x0000,
  0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0xCE59,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xCE59,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,
  0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0xCE59,0xCE59,0xCE59,0xCE59,0xCE59,0xCE59,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,
  0x07E0,0x07E0,0x07E0,0x0000,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x0000,0x07E0,0x07E0,0x07E0,
  0x07E0,0x07E0,0x0000,0x0000,0x0000,0x07E0,0x07E0,0x0000,0x0000,0x07E0,0x07E0,0x0000,0x0000,0x0000,0x07E0,0x07E0,
  0x07E0,0x0000,0x0000,0x0000,0x0000,0x0000,0x07E0,0x0000,0x0000,0x07E0,0x0000,0x0000,0x0000,0x0000,0x0000,0x07E0,
  0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
};

const uint16_t sprite_alien[256] PROGMEM = {
  0x0000,0x0000,0x0000,0x0000,0xF800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xF800,0x0000,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x0000,0x0000,0x0000,0xF800,0x0000,0x0000,0x0000,0x0000,0xF800,0x0000,0x0000,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x0000,0x0000,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0x0000,0x0000,0x0000,0x0000,
  0x0000,0x0000,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0x0000,0x0000,
  0x0000,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0x0000,
  0x0000,0xF800,0xF800,0xF800,0x0000,0x0000,0xF800,0xF800,0xF800,0xF800,0x0000,0x0000,0xF800,0xF800,0xF800,0x0000,
  0x0000,0xF800,0xF800,0xF800,0x0000,0xFFFF,0xF800,0xF800,0xF800,0xF800,0xFFFF,0x0000,0xF800,0xF800,0xF800,0x0000,
  0x0000,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0x0000,
  0x0000,0x0000,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0xF800,0x0000,0x0000,
  0x0000,0x0000,0x0000,0xF800,0xF800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xF800,0xF800,0x0000,0x0000,0x0000,
  0x0000,0x0000,0xF800,0xF800,0x0000,0x0000,0xF800,0xF800,0xF800,0xF800,0x0000,0x0000,0xF800,0xF800,0x0000,0x0000,
  0x0000,0xF800,0xF800,0x0000,0x0000,0xF800,0xF800,0x0000,0x0000,0xF800,0xF800,0x0000,0x0000,0xF800,0xF800,0x0000,
  0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
};

// --- ESTRUCTURAS DE DATOS ---
struct Alien { int x; int y; bool alive; };
struct Bullet { int x; int y; bool active; };

// --- VARIABLES DEL JUEGO ---
int shipX = 56;
const int shipY = 140;
int shipSpeed = 3;

#define NUM_ALIENS 12 
Alien aliens[NUM_ALIENS];
int alienDir = 1; 
int alienSpeed = 1;
int alienStepDown = 8;
unsigned long lastAlienMove = 0;
int alienMoveDelay = 400; 

Bullet bullet = {0, 0, false};

int score = 0;
int level = 1;

int gameState = 0; // 0=Menu, 1=Jugando, 2=GameOver, 3=Pausa
bool isScreenOn = true;
unsigned long lastButtonTime = 0;
bool lastShootState = HIGH; 

void setup() {
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_SHOOT, INPUT_PULLUP);
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
  tft.pushImage(56, 40, 16, 16, sprite_alien); 
  tft.setTextColor(TFT_WHITE, COLOR_BG);
  tft.drawCentreString("SPACE", 64, 15, 2);
  tft.drawCentreString("INVADERS", 64, 70, 2);
  tft.setTextColor(TFT_YELLOW, COLOR_BG);
  tft.drawCentreString("PULSA START", 64, 110, 1);
  gameState = 0;
}

void initAliens() {
  int index = 0;
  for(int row=0; row<3; row++) {
    for(int col=0; col<4; col++) {
      aliens[index].x = 10 + (col * 24);
      aliens[index].y = 20 + (row * 20);
      aliens[index].alive = true;
      index++;
    }
  }
}

void resetGame() {
  shipX = 56;
  score = 0;
  level = 1;
  alienMoveDelay = 400;
  bullet.active = false;
  initAliens();
  tft.fillScreen(COLOR_BG);
  gameState = 1;
}

void nextLevel() {
  level++;
  alienMoveDelay = max(50, alienMoveDelay - 50); 
  initAliens();
  bullet.active = false;
  tft.fillScreen(COLOR_BG);
}

void eraseOldPositions(int oldShipX, int oldBulletY) {
  if (oldShipX != shipX) {
    if (shipX > oldShipX) tft.fillRect(oldShipX, shipY, shipX - oldShipX, 16, COLOR_BG);
    else tft.fillRect(shipX + 16, shipY, oldShipX - shipX, 16, COLOR_BG);
  }

  if (oldBulletY != bullet.y && oldBulletY >= 0) {
    tft.fillRect(bullet.x, oldBulletY, 2, 8, COLOR_BG); 
  }
}

void drawGame() {
  tft.pushImage(shipX, shipY, 16, 16, sprite_ship);
  if (bullet.active) {
    tft.fillRect(bullet.x, bullet.y, 2, 6, TFT_YELLOW);
  }
  for(int i=0; i<NUM_ALIENS; i++) {
    if(aliens[i].alive) {
      tft.pushImage(aliens[i].x, aliens[i].y, 16, 16, sprite_alien);
    }
  }
  tft.setTextColor(TFT_GREEN, COLOR_BG);
  tft.drawString("SCORE: " + String(score), 2, 2, 1);
  tft.drawString("LVL: " + String(level), 90, 2, 1);
}

void showPause() {
    tft.fillRect(24, 60, 80, 40, TFT_BLACK);
    tft.drawRect(24, 60, 80, 40, TFT_WHITE);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawCentreString("PAUSA", 64, 72, 2);
}

void updateLogic() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastAlienMove > alienMoveDelay) {
    bool hitWall = false;
    for(int i=0; i<NUM_ALIENS; i++) {
      if(aliens[i].alive) {
        if(aliens[i].x + 16 >= 128 && alienDir == 1) hitWall = true;
        if(aliens[i].x <= 0 && alienDir == -1) hitWall = true;
      }
    }

    for(int i=0; i<NUM_ALIENS; i++) {
      if(aliens[i].alive) tft.fillRect(aliens[i].x, aliens[i].y, 16, 16, COLOR_BG);
    }

    if(hitWall) {
      alienDir *= -1; 
      for(int i=0; i<NUM_ALIENS; i++) {
        if(aliens[i].alive) {
          aliens[i].y += alienStepDown;
          if(aliens[i].y + 16 >= shipY) gameState = 2;
        }
      }
    } else {
      for(int i=0; i<NUM_ALIENS; i++) {
        if(aliens[i].alive) aliens[i].x += (alienDir * 4);
      }
    }
    lastAlienMove = currentMillis;
  }

  if (bullet.active) {
    if (bullet.y <= 4) { 
      tft.fillRect(bullet.x, bullet.y, 2, 8, COLOR_BG);
      bullet.active = false;
    } else {
      bullet.y -= 4; 
    }
  }

  // --- COLISIONES ---
  if (bullet.active) {
    for(int i=0; i<NUM_ALIENS; i++) {
      if(aliens[i].alive) {
        if(bullet.x >= aliens[i].x && bullet.x <= aliens[i].x + 16 &&
           bullet.y <= aliens[i].y + 16 && bullet.y >= aliens[i].y) {
             
             tft.fillRect(bullet.x, bullet.y, 2, 8, COLOR_BG);
             aliens[i].alive = false;
             bullet.active = false;
             score += 10;
             tft.fillRect(aliens[i].x, aliens[i].y, 16, 16, COLOR_BG); 
             break; 
        }
      }
    }
  }

  // --- FIN DE NIVEL ---
  bool allDead = true;
  for(int i=0; i<NUM_ALIENS; i++) if(aliens[i].alive) allDead = false;
  if(allDead && gameState == 1) nextLevel();
}

void gameOver() {
    tft.fillRect(14, 40, 100, 60, TFT_BLACK);
    tft.drawRect(14, 40, 100, 60, TFT_WHITE);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawCentreString("GAME OVER", 64, 45, 2);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawCentreString("Score: " + String(score), 64, 70, 1);
    delay(1000); 
}

void loop() {
  unsigned long currentMillis = millis();

  // Botón SELECT: Apagar/Encender pantalla
  if (digitalRead(PIN_SELECT) == LOW && currentMillis - lastButtonTime > 300) {
    isScreenOn = !isScreenOn;
    digitalWrite(PIN_TFT_BL, isScreenOn ? HIGH : LOW);
    lastButtonTime = currentMillis;
  }

  if (gameState == 0) { // MENU
    if (digitalRead(PIN_START) == LOW && currentMillis - lastButtonTime > 300) {
      resetGame();
      lastButtonTime = currentMillis;
    }
  } 
  else if (gameState == 1) { // JUGANDO
    // Entrar en PAUSA
    if (digitalRead(PIN_START) == LOW && currentMillis - lastButtonTime > 300) {
        gameState = 3;
        showPause();
        lastButtonTime = currentMillis;
        return;
    }

    int oldShipX = shipX;
    int oldBulletY = bullet.y;

    if (digitalRead(PIN_LEFT) == LOW) shipX -= shipSpeed;
    if (digitalRead(PIN_RIGHT) == LOW) shipX += shipSpeed;
    if (shipX < 0) shipX = 0;
    if (shipX > 128 - 16) shipX = 128 - 16;

    bool currentShootState = digitalRead(PIN_SHOOT);
    if (currentShootState == LOW && lastShootState == HIGH && !bullet.active) {
      bullet.x = shipX + 7; 
      bullet.y = shipY - 4;
      bullet.active = true;
    }
    lastShootState = currentShootState;

    updateLogic();
    if(gameState == 1) { 
      eraseOldPositions(oldShipX, oldBulletY);
      drawGame();
    } else if(gameState == 2) {
      gameOver();
    }
    delay(20); 
  } 
  else if (gameState == 2) { // GAME OVER
    if (digitalRead(PIN_START) == LOW && currentMillis - lastButtonTime > 300) {
      showMenu();
      lastButtonTime = currentMillis;
    }
  }
  else if (gameState == 3) { // PAUSA
    if (digitalRead(PIN_START) == LOW && currentMillis - lastButtonTime > 300) {
        tft.fillScreen(COLOR_BG);
        gameState = 1;
        lastButtonTime = currentMillis;
    }
  }
}
