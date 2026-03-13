#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

// --- CONFIGURACIÓN DE PINES ---
const int PIN_LEFT   = 15;  
const int PIN_RIGHT  = 27;  
const int PIN_ACTION = 25;  // Lanzar la bola
const int PIN_START  = 13;  // Iniciar / Pausa
const int PIN_SELECT = 33;  // Pantalla ON/OFF
const int PIN_TFT_BL = 21;  

// --- COLORES ---
#define COLOR_BG      TFT_BLACK
#define COLOR_PADDLE  tft.color565(200, 200, 200) // Gris claro
#define COLOR_BALL    TFT_RED
uint16_t rowColors[5] = {TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_CYAN};

// --- ESTRUCTURAS ---
struct Brick {
  int x, y, w, h;
  bool active;
  uint16_t color;
};

// --- VARIABLES DEL JUEGO ---
// Pala
int paddleW = 32;
int paddleH = 4;
int paddleX = (128 - paddleW) / 2;
const int paddleY = 145;
int paddleSpeed = 4;

// Bola
float ballX, ballY;
float ballDX, ballDY;
int ballR = 2;
bool ballActive = false;
float baseSpeed = 2.5;

// Ladrillos
#define ROWS 5
#define COLS 6
#define NUM_BRICKS (ROWS * COLS)
Brick bricks[NUM_BRICKS];

// Estado
int score = 0;
int level = 1;
int lives = 3;

int gameState = 0; // 0=Menu, 1=Jugando, 2=GameOver, 3=Pausa
bool isScreenOn = true;
unsigned long lastButtonTime = 0;
bool lastActionState = HIGH; 

void setup() {
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_ACTION, INPUT_PULLUP);
  pinMode(PIN_START, INPUT_PULLUP);
  pinMode(PIN_SELECT, INPUT_PULLUP);
  
  pinMode(PIN_TFT_BL, OUTPUT);
  digitalWrite(PIN_TFT_BL, HIGH);

  tft.init();
  tft.setRotation(2); 
  
  showMenu();
}

void showMenu() {
  tft.fillScreen(COLOR_BG);
  tft.setTextColor(TFT_CYAN, COLOR_BG);
  tft.drawCentreString("ARKANOID", 64, 30, 2);
  
  // Adorno
  tft.fillRect(34, 60, 20, 8, TFT_RED);
  tft.fillRect(56, 60, 20, 8, TFT_GREEN);
  tft.fillRect(78, 60, 20, 8, TFT_BLUE);
  tft.fillCircle(64, 80, ballR, TFT_WHITE);
  tft.fillRect(48, 95, 32, 4, COLOR_PADDLE);

  tft.setTextColor(TFT_YELLOW, COLOR_BG);
  tft.drawCentreString("PULSA START", 64, 120, 1);
  gameState = 0;
}

void initBricks() {
  int brickW = 18;
  int brickH = 8;
  int padding = 2;
  int offsetX = 5; 
  int offsetY = 20; 

  int i = 0;
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      bricks[i].x = offsetX + c * (brickW + padding);
      bricks[i].y = offsetY + r * (brickH + padding);
      bricks[i].w = brickW;
      bricks[i].h = brickH;
      bricks[i].active = true;
      bricks[i].color = rowColors[r];
      i++;
    }
  }
}

void resetGame() {
  score = 0;
  level = 1;
  lives = 3;
  paddleW = 32;
  baseSpeed = 2.5;
  initBricks();
  resetBall();
  
  tft.fillScreen(COLOR_BG);
  drawAllBricks();
  gameState = 1;
}

void resetBall() {
  ballActive = false;
  paddleX = (128 - paddleW) / 2;
  ballX = paddleX + paddleW / 2;
  ballY = paddleY - ballR - 1;
}

void nextLevel() {
  level++;
  baseSpeed += 0.5; 
  if(paddleW > 16) paddleW -= 4; 
  
  initBricks();
  resetBall();
  tft.fillScreen(COLOR_BG);
  drawAllBricks();
}

void drawAllBricks() {
  for (int i = 0; i < NUM_BRICKS; i++) {
    if (bricks[i].active) {
      tft.fillRect(bricks[i].x, bricks[i].y, bricks[i].w, bricks[i].h, bricks[i].color);
      tft.drawRect(bricks[i].x, bricks[i].y, bricks[i].w, bricks[i].h, COLOR_BG); 
    }
  }
}

void eraseOldPositions(int oldPaddleX, float oldBallX, float oldBallY) {
  if (oldPaddleX != paddleX) {
    if (paddleX > oldPaddleX) tft.fillRect(oldPaddleX, paddleY, paddleX - oldPaddleX, paddleH, COLOR_BG);
    else tft.fillRect(paddleX + paddleW, paddleY, oldPaddleX - paddleX, paddleH, COLOR_BG);
  }

  if (oldBallX != ballX || oldBallY != ballY) {
    tft.fillRect((int)oldBallX - ballR, (int)oldBallY - ballR, ballR*2 + 1, ballR*2 + 1, COLOR_BG); 
  }
}

void drawGame() {
  tft.fillRect(paddleX, paddleY, paddleW, paddleH, COLOR_PADDLE);
  tft.fillCircle((int)ballX, (int)ballY, ballR, COLOR_BALL);

  tft.setTextColor(TFT_WHITE, COLOR_BG);
  tft.drawString("PTS:" + String(score), 2, 2, 1);
  tft.drawString("LVL:" + String(level), 55, 2, 1);
  tft.drawString("HP:" + String(lives), 100, 2, 1);
}

void showPause() {
  tft.fillRect(24, 60, 80, 40, TFT_BLACK);
  tft.drawRect(24, 60, 80, 40, TFT_WHITE);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawCentreString("PAUSA", 64, 72, 2);
}

void updateLogic() {
  if (!ballActive) {
    ballX = paddleX + paddleW / 2;
    ballY = paddleY - ballR - 1;
    return;
  }

  ballX += ballDX;
  ballY += ballDY;

  // Paredes Laterales
  if (ballX - ballR <= 0) {
    ballX = ballR;
    ballDX = -ballDX;
  } else if (ballX + ballR >= 128) {
    ballX = 128 - ballR;
    ballDX = -ballDX;
  }

  // Techo
  if (ballY - ballR <= 10) { 
    ballY = ballR + 10;
    ballDY = -ballDY;
  }

  // Suelo
  if (ballY + ballR >= 160) {
    lives--;
    if (lives <= 0) {
      gameState = 2; 
    } else {
      tft.fillCircle((int)ballX, (int)ballY, ballR, COLOR_BG); 
      resetBall();
    }
    return;
  }

  // Pala
  if (ballY + ballR >= paddleY && ballY - ballR <= paddleY + paddleH) {
    if (ballX >= paddleX && ballX <= paddleX + paddleW) {
      ballY = paddleY - ballR - 1; 
      ballDY = -abs(ballDY); 
      
      float hitPoint = ((ballX - paddleX) / paddleW) - 0.5; 
      ballDX = hitPoint * (baseSpeed * 2.0); 
      
      float currentSpeed = sqrt((ballDX*ballDX) + (ballDY*ballDY));
      ballDX = (ballDX / currentSpeed) * baseSpeed;
      ballDY = (ballDY / currentSpeed) * baseSpeed;
      if(ballDY > -1.0) ballDY = -1.0; 
    }
  }

  // Ladrillos
  bool allCleared = true;
  for (int i = 0; i < NUM_BRICKS; i++) {
    if (bricks[i].active) {
      allCleared = false;
      if (ballX + ballR >= bricks[i].x && ballX - ballR <= bricks[i].x + bricks[i].w &&
          ballY + ballR >= bricks[i].y && ballY - ballR <= bricks[i].y + bricks[i].h) {
        
        bricks[i].active = false;
        score += (5 - (i / COLS)) * 10; 
        
        tft.fillRect(bricks[i].x, bricks[i].y, bricks[i].w, bricks[i].h, COLOR_BG);
        ballDY = -ballDY;
        break; 
      }
    }
  }

  if (allCleared) {
    nextLevel();
  }
}

void gameOver() {
  tft.fillRect(14, 50, 100, 60, TFT_BLACK);
  tft.drawRect(14, 50, 100, 60, TFT_WHITE);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawCentreString("GAME OVER", 64, 55, 2);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawCentreString("Score: " + String(score), 64, 80, 1);
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

    int oldPaddleX = paddleX;
    float oldBallX = ballX;
    float oldBallY = ballY;

    if (digitalRead(PIN_LEFT) == LOW) paddleX -= paddleSpeed;
    if (digitalRead(PIN_RIGHT) == LOW) paddleX += paddleSpeed;
    if (paddleX < 0) paddleX = 0;
    if (paddleX > 128 - paddleW) paddleX = 128 - paddleW;

    bool currentActionState = digitalRead(PIN_ACTION);
    if (currentActionState == LOW && lastActionState == HIGH && !ballActive) {
      ballActive = true;
      ballDX = random(-10, 10) / 10.0;
      if(ballDX == 0.0) ballDX = 0.5; // Evita que caiga en línea recta pura
      ballDY = -baseSpeed;
    }
    lastActionState = currentActionState;

    updateLogic();
    
    if(gameState == 1) {
      eraseOldPositions(oldPaddleX, oldBallX, oldBallY);
      drawGame();
    } else if (gameState == 2) {
      gameOver();
    }
    
    delay(15); 
  } 
  else if (gameState == 2) { // GAME OVER
    if (digitalRead(PIN_START) == LOW && currentMillis - lastButtonTime > 300) {
      while(digitalRead(PIN_START) == LOW) { delay(10); } // Evitar doble pulsación
      showMenu();
      lastButtonTime = currentMillis;
    }
  }
  else if (gameState == 3) { // PAUSA
    if (digitalRead(PIN_START) == LOW && currentMillis - lastButtonTime > 300) {
        tft.fillScreen(COLOR_BG);
        drawAllBricks(); // Al quitar la pausa, repintamos los ladrillos y el fondo negro
        gameState = 1;
        lastButtonTime = currentMillis;
    }
  }
}
