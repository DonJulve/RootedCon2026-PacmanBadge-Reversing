#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

// --- CONFIGURACIÓN DE PINES ---
const int PIN_JUMP   = 27;  // Acción: Saltar
const int PIN_START  = 13;  // Iniciar / Reiniciar
const int PIN_SELECT = 33;  // Pantalla ON/OFF
const int PIN_TFT_BL = 21;  // Retroiluminación

// --- COLORES ---
#define COLOR_SKY    tft.color565(135, 206, 235) // Azul cielo
#define COLOR_PIPE   tft.color565(34, 139, 34)   // Verde tubería
#define COLOR_BORDER tft.color565(0, 100, 0)     // Verde oscuro para bordes
#define COLOR_GROUND tft.color565(139, 69, 19)   // Marrón suelo
#define COLOR_TRANS  0x0000                      // Negro será transparente

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

// --- VARIABLES DEL JUEGO ---
float capyY = 60.0;
float capyVel = 0.0;
const float gravity = 0.35;
const float jumpForce = -4.0;
int capyX = 20;

int pipeX = 128;
int pipeWidth = 24;
int gapY = 50;
int gapHeight = 45;
int pipeSpeed = 2;

int score = 0;
int highScore = 0;

int gameState = 0; // 0=Menu, 1=Jugando, 2=GameOver
bool isScreenOn = true;
unsigned long lastButtonTime = 0;

bool lastJumpState = HIGH; 

void setup() {
  pinMode(PIN_JUMP, INPUT_PULLUP);
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
  tft.fillScreen(COLOR_SKY);
  tft.fillRect(0, 140, 128, 20, COLOR_GROUND);
  
  tft.pushImage(56, 40, 16, 16, capybara_sprite, COLOR_TRANS);
  
  tft.setTextColor(TFT_WHITE, COLOR_SKY);
  tft.drawCentreString("FLAPPY", 64, 15, 2);
  tft.drawCentreString("CAPIBARA", 64, 70, 2);
  
  tft.setTextColor(TFT_YELLOW, COLOR_SKY);
  tft.drawCentreString("PULSA START", 64, 110, 1);
  gameState = 0;
}

void resetGame() {
  capyY = 60.0;
  capyVel = 0.0;
  pipeX = 128;
  score = 0;
  gapY = random(20, 90);
  
  tft.fillScreen(COLOR_SKY);
  tft.fillRect(0, 140, 128, 20, COLOR_GROUND);
  gameState = 1;
}

void drawGame() {
  if (capyVel != 0) {
    int oldY = capyY - capyVel;
    tft.fillRect(capyX, oldY, 16, 16, COLOR_SKY); 
  }

  int oldPipeX = pipeX;
  pipeX -= pipeSpeed;
  
  tft.fillRect(oldPipeX + pipeWidth - pipeSpeed, 0, pipeSpeed + 1, gapY, COLOR_SKY);
  tft.fillRect(oldPipeX + pipeWidth - pipeSpeed, gapY + gapHeight, pipeSpeed + 1, 140 - (gapY + gapHeight), COLOR_SKY);

  if (pipeX + pipeWidth < 0) {
    tft.fillRect(0, 0, pipeWidth, 140, COLOR_SKY); 
    pipeX = 128;
    gapY = random(20, 90);
    score++;
    if(score > highScore) highScore = score;
    
    if(score % 5 == 0 && pipeSpeed < 5) pipeSpeed++; 
  }

  tft.fillRect(pipeX, 0, pipeWidth, gapY, COLOR_PIPE);
  tft.drawRect(pipeX, 0, pipeWidth, gapY, COLOR_BORDER);
  tft.fillRect(pipeX, gapY + gapHeight, pipeWidth, 140 - (gapY + gapHeight), COLOR_PIPE);
  tft.drawRect(pipeX, gapY + gapHeight, pipeWidth, 140 - (gapY + gapHeight), COLOR_BORDER);

  tft.pushImage(capyX, (int)capyY, 16, 16, capybara_sprite, COLOR_TRANS);

  tft.setTextColor(TFT_WHITE, COLOR_SKY);
  tft.drawString(String(score), 5, 5, 2);
}

void checkCollisions() {
  if (capyY > 124 || capyY < 0) { 
    gameOver();
  }
  
  if (capyX + 14 > pipeX && capyX < pipeX + pipeWidth) {
    if (capyY + 2 < gapY || capyY + 14 > gapY + gapHeight) { 
      gameOver();
    }
  }
}

void gameOver() {
  gameState = 2;
  tft.fillRect(14, 40, 100, 60, TFT_BLACK);
  tft.drawRect(14, 40, 100, 60, TFT_WHITE);
  
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawCentreString("GAME OVER", 64, 45, 2);
  
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawCentreString("Score: " + String(score), 64, 65, 1);
  tft.drawCentreString("Best: " + String(highScore), 64, 80, 1);
  
  delay(1000); 
}

void loop() {
  unsigned long currentMillis = millis();

  if (digitalRead(PIN_SELECT) == LOW && currentMillis - lastButtonTime > 300) {
    isScreenOn = !isScreenOn;
    digitalWrite(PIN_TFT_BL, isScreenOn ? HIGH : LOW);
    lastButtonTime = currentMillis;
    if (isScreenOn && gameState == 0) showMenu();
  }

  if (gameState == 0) {
    if (digitalRead(PIN_START) == LOW && currentMillis - lastButtonTime > 300) {
      resetGame();
      lastButtonTime = currentMillis;
    }
  } 
  else if (gameState == 1) {
    bool currentJumpState = digitalRead(PIN_JUMP);
    
    if (currentJumpState == LOW && lastJumpState == HIGH && currentMillis - lastButtonTime > 50) {
      capyVel = jumpForce;
      lastButtonTime = currentMillis;
    }
    lastJumpState = currentJumpState;
    
    capyVel += gravity;
    capyY += capyVel;
    
    drawGame();
    checkCollisions();
    
    delay(25);
  } 
  else if (gameState == 2) {
    if (digitalRead(PIN_START) == LOW && currentMillis - lastButtonTime > 300) {
      while(digitalRead(PIN_START) == LOW) { delay(10); }
      showMenu();
      lastButtonTime = millis();
      lastJumpState = HIGH;
    }
  }
}
