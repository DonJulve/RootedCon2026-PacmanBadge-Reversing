#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

const int PIN_START_GLOBAL  = 13;
const int PIN_SELECT_GLOBAL = 33;
const int PIN_TFT_BL_GLOBAL = 21;

int activeGame = -1; // -1 means menu
unsigned long lastGlobalButtonTime = 0;
int menuSelection = 0;
bool isGlobalScreenOn = true;

namespace Game_2048 {
uint16_t getColor(int value);
void setup();
void showMenu();
void spawnTile();
void resetGame();
void drawBoard();
void drawUI();
void fullRedraw();
bool moveTiles(int dr, int dc);
bool checkGameOver();
void gameOver();
void loop();

// #include <TFT_eSPI.h>

// TFT_eSPI tft = TFT_eSPI();

// --- CONFIGURACIÓN DE PINES ---
const int PIN_LEFT   = 15;  
const int PIN_RIGHT  = 27;  
const int PIN_UP     = 25;  
const int PIN_DOWN   = 26;  
const int PIN_START  = 13;  
const int PIN_SELECT = 33;  
const int PIN_TFT_BL = 21;  

// --- COLORES SEGÚN EL NÚMERO ---
uint16_t getColor(int value) {
  switch (value) {
    case 0: return tft.color565(205, 193, 180);
    case 2: return tft.color565(238, 228, 218);
    case 4: return tft.color565(237, 224, 200);
    case 8: return tft.color565(242, 177, 121);
    case 16: return tft.color565(245, 149, 99);
    case 32: return tft.color565(246, 124, 95);
    case 64: return tft.color565(246, 94, 59);
    case 128: return tft.color565(237, 207, 114);
    case 256: return tft.color565(237, 204, 97);
    case 512: return tft.color565(237, 200, 80);
    case 1024: return tft.color565(237, 197, 63);
    case 2048: return tft.color565(237, 194, 46);
    default: return tft.color565(60, 58, 50); // Números mayores
  }
}

// --- VARIABLES DEL JUEGO ---
int board[4][4];
unsigned long score = 0;
int gameState = 0; // 0=Menu, 1=Jugando, 2=GameOver
bool isScreenOn = true;
unsigned long lastButtonTime = 0;

void setup() {
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_START, INPUT_PULLUP);
  pinMode(PIN_SELECT, INPUT_PULLUP);
  
  pinMode(PIN_TFT_BL, OUTPUT);
  digitalWrite(PIN_TFT_BL, HIGH);

  tft.init();
  tft.setRotation(2); 
  showMenu();
}

void showMenu() {
  tft.fillScreen(tft.color565(250, 248, 239)); // Fondo claro
  tft.setTextColor(tft.color565(119, 110, 101));
  tft.drawCentreString("2048", 64, 40, 4);
  tft.drawCentreString("PULSA START", 64, 100, 2);
  gameState = 0;
}

void spawnTile() {
  int empty[16][2];
  int count = 0;
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (board[r][c] == 0) {
        empty[count][0] = r;
        empty[count][1] = c;
        count++;
      }
    }
  }
  if (count > 0) {
    int r = random(count);
    // 90% probabilidad de 2, 10% probabilidad de 4
    board[empty[r][0]][empty[r][1]] = (random(10) == 0) ? 4 : 2;
  }
}

void resetGame() {
  score = 0;
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      board[r][c] = 0;
    }
  }
  spawnTile();
  spawnTile();
  fullRedraw();
  gameState = 1;
}

void drawBoard() {
  int gridSize = 128;
  int cellSize = 32; // 128 / 4
  int offsetY = 32;  // Espacio arriba para la UI

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      int val = board[r][c];
      int x = c * cellSize;
      int y = offsetY + (r * cellSize);
      
      // Fondo de la celda
      tft.fillRect(x, y, cellSize, cellSize, getColor(val));
      tft.drawRect(x, y, cellSize, cellSize, tft.color565(187, 173, 160)); // Borde
      
      // Dibujar número si no es 0
      if (val > 0) {
        if (val <= 4) tft.setTextColor(tft.color565(119, 110, 101));
        else tft.setTextColor(TFT_WHITE);
        
        tft.drawCentreString(String(val), x + 16, y + 8, 2);
      }
    }
  }
}

void drawUI() {
  tft.fillRect(0, 0, 128, 32, tft.color565(187, 173, 160));
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("SCORE", 64, 2, 1);
  tft.drawCentreString(String(score), 64, 12, 2);
}

void fullRedraw() {
  tft.fillScreen(tft.color565(250, 248, 239));
  drawUI();
  drawBoard();
}

bool moveTiles(int dr, int dc) {
  bool moved = false;
  bool merged[4][4] = {false};

  // Determinar el orden de recorrido según la dirección
  int rStart = (dr == 1) ? 3 : 0;
  int rEnd = (dr == 1) ? -1 : 4;
  int rStep = (dr == 1) ? -1 : 1;

  int cStart = (dc == 1) ? 3 : 0;
  int cEnd = (dc == 1) ? -1 : 4;
  int cStep = (dc == 1) ? -1 : 1;

  for (int r = rStart; r != rEnd; r += rStep) {
    for (int c = cStart; c != cEnd; c += cStep) {
      if (board[r][c] != 0) {
        int nr = r;
        int nc = c;

        // Deslizar hasta chocar
        while (nr + dr >= 0 && nr + dr < 4 && nc + dc >= 0 && nc + dc < 4 && board[nr + dr][nc + dc] == 0) {
          nr += dr;
          nc += dc;
        }

        // Comprobar fusión
        if (nr + dr >= 0 && nr + dr < 4 && nc + dc >= 0 && nc + dc < 4 && 
            board[nr + dr][nc + dc] == board[r][c] && !merged[nr + dr][nc + dc]) {
          
          board[nr + dr][nc + dc] *= 2;
          score += board[nr + dr][nc + dc];
          board[r][c] = 0;
          merged[nr + dr][nc + dc] = true;
          moved = true;
        } 
        else if (nr != r || nc != c) {
          // Solo mover
          board[nr][nc] = board[r][c];
          board[r][c] = 0;
          moved = true;
        }
      }
    }
  }
  return moved;
}

bool checkGameOver() {
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (board[r][c] == 0) return false; // Hay huecos libres
      if (r < 3 && board[r][c] == board[r + 1][c]) return false; // Se pueden fusionar en vertical
      if (c < 3 && board[r][c] == board[r][c + 1]) return false; // Se pueden fusionar en horizontal
    }
  }
  return true; // No hay movimientos posibles
}

void gameOver() {
  gameState = 2;
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

  // Control SELECT (Pantalla)
  static unsigned long selectPressStart = 0;
  static bool selectHasTriggered = false;
  bool doToggleScreen = false;
  if (digitalRead(PIN_SELECT) == LOW) {
    if (selectPressStart == 0) selectPressStart = currentMillis;
    if (!selectHasTriggered && currentMillis - selectPressStart >= 1000 && currentMillis - lastButtonTime > 300) {
      doToggleScreen = true;
      selectHasTriggered = true;
    }
  } else {
    selectPressStart = 0;
    selectHasTriggered = false;
  }
  if (doToggleScreen) {
    isScreenOn = !isScreenOn;
    digitalWrite(PIN_TFT_BL, isScreenOn ? HIGH : LOW);
    lastButtonTime = currentMillis;
        if(gameState == 0) showMenu();
        if(gameState == 1) fullRedraw();
        if(gameState == 2) { fullRedraw(); gameOver(); }
  }

  if (gameState == 0) { // MENU
    if (digitalRead(PIN_START) == LOW && currentMillis - lastButtonTime > 300) {
      resetGame();
      lastButtonTime = currentMillis;
    }
  } 
  else if (gameState == 1) { // JUGANDO
    bool moved = false;
    
    // Leer cruceta (con antirrebote)
    if (digitalRead(PIN_UP) == LOW && currentMillis - lastButtonTime > 200) {
      moved = moveTiles(-1, 0); lastButtonTime = currentMillis;
    } else if (digitalRead(PIN_DOWN) == LOW && currentMillis - lastButtonTime > 200) {
      moved = moveTiles(1, 0); lastButtonTime = currentMillis;
    } else if (digitalRead(PIN_LEFT) == LOW && currentMillis - lastButtonTime > 200) {
      moved = moveTiles(0, -1); lastButtonTime = currentMillis;
    } else if (digitalRead(PIN_RIGHT) == LOW && currentMillis - lastButtonTime > 200) {
      moved = moveTiles(0, 1); lastButtonTime = currentMillis;
    }

    if (moved) {
      spawnTile();
      drawUI();
      drawBoard();
      
      if (checkGameOver()) {
        gameOver();
      }
    }
  } 
  else if (gameState == 2) { // GAME OVER
    if (digitalRead(PIN_START) == LOW && currentMillis - lastButtonTime > 300) {
      while(digitalRead(PIN_START) == LOW) { delay(10); } 
      showMenu();
      lastButtonTime = millis();
    }
  }
}

} // namespace Game_2048

namespace Game_Arkanoid {
void setup();
void showMenu();
void initBricks();
void resetGame();
void resetBall();
void nextLevel();
void drawAllBricks();
void eraseOldPositions(int oldPaddleX, float oldBallX, float oldBallY);
void drawGame();
void showPause();
void updateLogic();
void gameOver();
void loop();

// #include <TFT_eSPI.h>

// TFT_eSPI tft = TFT_eSPI();

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
  static unsigned long selectPressStart = 0;
  static bool selectHasTriggered = false;
  bool doToggleScreen = false;
  if (digitalRead(PIN_SELECT) == LOW) {
    if (selectPressStart == 0) selectPressStart = currentMillis;
    if (!selectHasTriggered && currentMillis - selectPressStart >= 1000 && currentMillis - lastButtonTime > 300) {
      doToggleScreen = true;
      selectHasTriggered = true;
    }
  } else {
    selectPressStart = 0;
    selectHasTriggered = false;
  }
  if (doToggleScreen) {
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

} // namespace Game_Arkanoid
#undef COLOR_BG
#undef NUM_BRICKS
#undef COLOR_BALL
#undef ROWS
#undef COLS
#undef COLOR_PADDLE

namespace Game_Capy_jump {
void setup();
void showMenu();
void initPlats();
void resetGame();
void showPause();
void gameOver();
void fullRedraw();
void updateLogic();
void loop();

// #include <TFT_eSPI.h>

// TFT_eSPI tft = TFT_eSPI();

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
  static unsigned long selectPressStart = 0;
  static bool selectHasTriggered = false;
  bool doToggleScreen = false;
  if (digitalRead(PIN_SELECT) == LOW) {
    if (selectPressStart == 0) selectPressStart = currentMillis;
    if (!selectHasTriggered && currentMillis - selectPressStart >= 1000 && currentMillis - lastButtonTime > 300) {
      doToggleScreen = true;
      selectHasTriggered = true;
    }
  } else {
    selectPressStart = 0;
    selectHasTriggered = false;
  }
  if (doToggleScreen) {
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

} // namespace Game_Capy_jump
#undef COLOR_BG
#undef COLOR_TRANS
#undef COLOR_PLAT
#undef NUM_PLATS

namespace Game_Flappy_capibara {
void setup();
void showMenu();
void resetGame();
void drawGame();
void checkCollisions();
void gameOver();
void loop();

// #include <TFT_eSPI.h>

// TFT_eSPI tft = TFT_eSPI();

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

  static unsigned long selectPressStart = 0;
  static bool selectHasTriggered = false;
  bool doToggleScreen = false;
  if (digitalRead(PIN_SELECT) == LOW) {
    if (selectPressStart == 0) selectPressStart = currentMillis;
    if (!selectHasTriggered && currentMillis - selectPressStart >= 1000 && currentMillis - lastButtonTime > 300) {
      doToggleScreen = true;
      selectHasTriggered = true;
    }
  } else {
    selectPressStart = 0;
    selectHasTriggered = false;
  }
  if (doToggleScreen) {
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

} // namespace Game_Flappy_capibara
#undef COLOR_TRANS
#undef COLOR_PIPE
#undef COLOR_BORDER
#undef COLOR_SKY
#undef COLOR_GROUND

namespace Game_Highway_capy {
void setup();
void showMenu();
void spawnEnemy(int i, int startY);
void resetGame();
void eraseAndDraw();
void updateLogic();
void loop();

// #include <TFT_eSPI.h>

// TFT_eSPI tft = TFT_eSPI();

// --- CONFIGURACIÓN DE PINES ---
const int PIN_LEFT   = 15;  
const int PIN_RIGHT  = 27;  
const int PIN_ACCEL  = 25;  
const int PIN_BRAKE  = 26;  
const int PIN_START  = 13;  
const int PIN_SELECT = 33;  
const int PIN_TFT_BL = 21;  

// --- COLORES ---
#define COLOR_ROAD   tft.color565(80, 80, 80)
#define COLOR_GRASS  tft.color565(34, 139, 34)
#define COLOR_TRANS  0x0000 // Negro transparente

// --- SPRITES (16x16) ---
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

const uint16_t sprite_car[256] PROGMEM = { 
  0x0000,0x0000,0x0000,0x0000,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0000,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x0000,0x0001,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x0001,0x0000,0x0000,0x0000,
  0x0000,0x0001,0x0001,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x0001,0x0001,0x0000,
  0x0000,0x0001,0x0001,0x001F,0x001F,0xCE59,0xCE59,0xCE59,0xCE59,0xCE59,0xCE59,0x001F,0x001F,0x0001,0x0001,0x0000,
  0x0000,0x0001,0x0001,0x001F,0x001F,0xCE59,0xCE59,0xCE59,0xCE59,0xCE59,0xCE59,0x001F,0x001F,0x0001,0x0001,0x0000,
  0x0000,0x0000,0x0000,0x0001,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x0001,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x0000,0x0001,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x0001,0x0000,0x0000,0x0000,
  0x0000,0x0001,0x0001,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x0001,0x0001,0x0000,
  0x0000,0x0001,0x0001,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x0001,0x0001,0x0000,
  0x0000,0x0001,0x0001,0x001F,0x001F,0xCE59,0xCE59,0xCE59,0xCE59,0xCE59,0xCE59,0x001F,0x001F,0x0001,0x0001,0x0000,
  0x0000,0x0001,0x0001,0x001F,0x001F,0xCE59,0xCE59,0xCE59,0xCE59,0xCE59,0xCE59,0x001F,0x001F,0x0001,0x0001,0x0000,
  0x0000,0x0000,0x0000,0x0001,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x0001,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x0000,0x0001,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x001F,0x0001,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x0000,0x0001,0x001F,0x001F,0xF800,0x001F,0x001F,0xF800,0x001F,0x001F,0x0001,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x0000,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0000,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
};

struct Enemy {
  int x;
  float y;
  float speed;
};

// --- VARIABLES ---
int capyX = 56, oldCapyX = 56;
const int capyY = 130;
#define NUM_ENEMIES 3
Enemy enemies[NUM_ENEMIES];
float oldEnemyY[NUM_ENEMIES];

int gear = 1;
float gameSpeed = 2.0;
float lineOffset = 0;
unsigned long score = 0;
int gameState = 0; 
bool isScreenOn = true;
unsigned long lastButtonTime = 0, lastGearTime = 0;

void setup() {
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_ACCEL, INPUT_PULLUP);
  pinMode(PIN_BRAKE, INPUT_PULLUP);
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
  tft.fillScreen(COLOR_ROAD);
  tft.fillRect(0, 0, 16, 160, COLOR_GRASS);
  tft.fillRect(112, 0, 16, 160, COLOR_GRASS);
  tft.pushImage(56, 40, 16, 16, capybara_sprite, COLOR_TRANS);
  tft.setTextColor(TFT_WHITE, COLOR_ROAD);
  tft.drawCentreString("HIGHWAY CAPY", 64, 20, 2);
  tft.setTextColor(TFT_YELLOW, COLOR_ROAD);
  tft.drawCentreString("PULSA START", 64, 100, 1);
  gameState = 0;
}

void spawnEnemy(int i, int startY) {
  enemies[i].x = random(18, 94);
  enemies[i].y = startY;
  enemies[i].speed = random(5, 15) / 10.0;
}

void resetGame() {
  capyX = 56; oldCapyX = 56;
  score = 0; gear = 1; gameSpeed = 2.0;
  for(int i=0; i<NUM_ENEMIES; i++) {
    spawnEnemy(i, -30 - (i * 60));
    oldEnemyY[i] = enemies[i].y;
  }
  tft.fillScreen(COLOR_ROAD);
  tft.fillRect(0, 0, 16, 160, COLOR_GRASS);
  tft.fillRect(112, 0, 16, 160, COLOR_GRASS);
  gameState = 1;
}

void eraseAndDraw() {
  // 1. Borrar rastro anterior del Capibara con rectángulos de carretera
  if (capyX != oldCapyX) {
    tft.fillRect(oldCapyX, capyY, 16, 16, COLOR_ROAD);
  }

  // 2. Borrar rastro de enemigos
  for (int i = 0; i < NUM_ENEMIES; i++) {
    if ((int)enemies[i].y != (int)oldEnemyY[i]) {
      // Borramos la caja entera del enemigo en su posición vieja
      tft.fillRect(enemies[i].x, (int)oldEnemyY[i], 16, 16, COLOR_ROAD);
    }
  }

  // 3. Dibujar líneas de la carretera (solo si se mueven)
  static int lastLineY[4];
  for(int i=0; i<4; i++) {
     int y = ((int)lineOffset + i*40) % 160;
     tft.fillRect(62, (y + 140) % 160, 4, 20, COLOR_ROAD); // Borrar
     tft.fillRect(62, y, 4, 20, TFT_WHITE); // Dibujar
  }

  // 4. Dibujar Sprites en nuevas posiciones
  for (int i = 0; i < NUM_ENEMIES; i++) {
    if (enemies[i].y > -16 && enemies[i].y < 160) {
      tft.pushImage(enemies[i].x, (int)enemies[i].y, 16, 16, sprite_car, COLOR_TRANS);
    }
  }
  tft.pushImage(capyX, capyY, 16, 16, capybara_sprite, COLOR_TRANS);

  // 5. UI Superior (Score y Gear)
  tft.fillRect(0, 0, 128, 12, COLOR_GRASS);
  tft.setTextColor(TFT_WHITE);
  tft.drawNumber(score, 2, 2);
  tft.drawRightString("G:" + String(gear), 126, 2, 1);
}

void updateLogic() {
  unsigned long now = millis();
  
  if (digitalRead(PIN_ACCEL) == LOW && now - lastGearTime > 250) {
    if (gear < 3) gear++;
    lastGearTime = now;
  }
  if (digitalRead(PIN_BRAKE) == LOW && now - lastGearTime > 250) {
    if (gear > 1) gear--;
    lastGearTime = now;
  }

  gameSpeed = (gear == 1) ? 1.5 : (gear == 2 ? 3.5 : 6.0);
  score += gear;
  lineOffset += gameSpeed;
  if (lineOffset >= 160) lineOffset = 0;

  for(int i=0; i<NUM_ENEMIES; i++) {
    oldEnemyY[i] = enemies[i].y;
    enemies[i].y += (enemies[i].speed + gameSpeed);
    
    if (enemies[i].y > 160) spawnEnemy(i, -20);

    // Colisión
    if (abs(capyX - enemies[i].x) < 12 && abs(capyY - (int)enemies[i].y) < 12) {
      gameState = 2;
    }
  }
}

void loop() {
  unsigned long currentMillis = millis();

  static unsigned long selectPressStart = 0;
  static bool selectHasTriggered = false;
  bool doToggleScreen = false;
  if (digitalRead(PIN_SELECT) == LOW) {
    if (selectPressStart == 0) selectPressStart = currentMillis;
    if (!selectHasTriggered && currentMillis - selectPressStart >= 1000 && currentMillis - lastButtonTime > 300) {
      doToggleScreen = true;
      selectHasTriggered = true;
    }
  } else {
    selectPressStart = 0;
    selectHasTriggered = false;
  }
  if (doToggleScreen) {
    isScreenOn = !isScreenOn;
    digitalWrite(PIN_TFT_BL, isScreenOn ? HIGH : LOW);
    lastButtonTime = currentMillis;
  }

  if (gameState == 0) {
    if (digitalRead(PIN_START) == LOW) resetGame();
  } 
  else if (gameState == 1) {
    oldCapyX = capyX;
    if (digitalRead(PIN_LEFT) == LOW) capyX -= 3;
    if (digitalRead(PIN_RIGHT) == LOW) capyX += 3;
    capyX = constrain(capyX, 16, 96);

    updateLogic();
    eraseAndDraw();
    
    if (gameState == 2) {
      tft.fillRect(20, 60, 88, 40, TFT_BLACK);
      tft.drawRect(20, 60, 88, 40, TFT_WHITE);
      tft.setTextColor(TFT_RED);
      tft.drawCentreString("CRASH!", 64, 65, 2);
      tft.setTextColor(TFT_WHITE);
      tft.drawCentreString("Score: " + String(score), 64, 85, 1);
    }
    delay(20); // FPS constante
  } 
  else if (gameState == 2) {
    if (digitalRead(PIN_START) == LOW && currentMillis - lastButtonTime > 500) {
      showMenu();
      lastButtonTime = currentMillis;
    }
  }
}

} // namespace Game_Highway_capy
#undef COLOR_ROAD
#undef COLOR_TRANS
#undef COLOR_GRASS
#undef NUM_ENEMIES

namespace Game_Snake {
void setup();
void showMenu();
void spawnFood();
void resetGame();
int getPx(int gridX);
int getPy(int gridY);
void drawFood();
void drawUI();
void fullRedraw();
void showPause();
void gameOver();
void updateLogic();
void loop();

// #include <TFT_eSPI.h>

// TFT_eSPI tft = TFT_eSPI();

// --- CONFIGURACIÓN DE PINES ---
const int PIN_LEFT   = 15;  
const int PIN_RIGHT  = 27;  
const int PIN_UP     = 25;  
const int PIN_DOWN   = 26;
const int PIN_START  = 13;
const int PIN_SELECT = 33;
const int PIN_TFT_BL = 21;  

// --- COLORES ---
#define COLOR_BG    TFT_BLACK
#define COLOR_SNAKE TFT_GREEN
#define COLOR_HEAD  tft.color565(0, 255, 0) // Verde más brillante
#define COLOR_FOOD  TFT_RED
#define COLOR_UI    TFT_DARKGREY

// --- CONFIGURACIÓN DEL GRID ---
#define BLOCK_SIZE 8
#define GRID_W 16  // 128 / 8
#define GRID_H 18  // Dejamos 2 bloques (16px) arriba para la UI. Total 160 / 8 = 20

// --- VARIABLES DEL JUEGO ---
int snakeX[300], snakeY[300]; // Coordenadas del cuerpo
int snakeLen = 3;

int dirX = 1, dirY = 0;       // Dirección actual
int nextDirX = 1, nextDirY = 0; // Dirección buffer (evita giros suicidas rápidos)

int foodX, foodY;
int score = 0;

unsigned long lastMoveTime = 0;
int moveDelay = 150; // Velocidad inicial (menos es más rápido)

int gameState = 0; // 0=Menu, 1=Jugando, 2=GameOver, 3=Pausa
bool isScreenOn = true;
unsigned long lastButtonTime = 0;

void setup() {
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
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
  
  tft.setTextColor(TFT_GREEN, COLOR_BG);
  tft.drawCentreString("SNAKE", 64, 40, 4); // Fuente grande
  
  // Adorno serpiente menú
  tft.fillRect(40, 80, 48, 8, COLOR_SNAKE);
  tft.fillRect(80, 72, 8, 8, COLOR_HEAD);
  tft.fillRect(88, 72, 4, 4, TFT_RED); // Lengua
  
  tft.setTextColor(TFT_YELLOW, COLOR_BG);
  tft.drawCentreString("PULSA START", 64, 120, 1);
  gameState = 0;
}

void spawnFood() {
  bool valid = false;
  while (!valid) {
    foodX = random(0, GRID_W);
    foodY = random(0, GRID_H);
    valid = true;
    // Comprobar que no aparezca dentro de la serpiente
    for (int i = 0; i < snakeLen; i++) {
      if (snakeX[i] == foodX && snakeY[i] == foodY) {
        valid = false;
        break;
      }
    }
  }
  drawFood();
}

void resetGame() {
  snakeLen = 3;
  // Posición inicial centrada
  snakeX[0] = GRID_W / 2; snakeY[0] = GRID_H / 2;
  snakeX[1] = snakeX[0] - 1; snakeY[1] = snakeY[0];
  snakeX[2] = snakeX[0] - 2; snakeY[2] = snakeY[0];
  
  dirX = 1; dirY = 0;
  nextDirX = 1; nextDirY = 0;
  score = 0;
  moveDelay = 150;
  
  spawnFood();
  
  gameState = 1;
  fullRedraw();
}

// Para convertir las coordenadas de la cuadrícula a píxeles en pantalla
int getPx(int gridX) { return gridX * BLOCK_SIZE; }
int getPy(int gridY) { return (gridY * BLOCK_SIZE) + 16; } // +16px para la UI superior

void drawFood() {
  tft.fillRect(getPx(foodX), getPy(foodY), BLOCK_SIZE, BLOCK_SIZE, COLOR_FOOD);
}

void drawUI() {
  tft.fillRect(0, 0, 128, 16, COLOR_UI);
  tft.setTextColor(TFT_WHITE, COLOR_UI);
  tft.drawString("SCORE: " + String(score), 4, 4, 1);
}

// Dibuja TODO el tablero (se usa al quitar pausa o encender pantalla)
void fullRedraw() {
  tft.fillScreen(COLOR_BG);
  drawUI();
  drawFood();
  
  for (int i = 0; i < snakeLen; i++) {
    uint16_t color = (i == 0) ? COLOR_HEAD : COLOR_SNAKE;
    tft.fillRect(getPx(snakeX[i]), getPy(snakeY[i]), BLOCK_SIZE, BLOCK_SIZE, color);
    tft.drawRect(getPx(snakeX[i]), getPy(snakeY[i]), BLOCK_SIZE, BLOCK_SIZE, COLOR_BG);
  }
}

void showPause() {
  tft.fillRect(24, 60, 80, 40, TFT_BLACK);
  tft.drawRect(24, 60, 80, 40, TFT_WHITE);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawCentreString("PAUSA", 64, 72, 2);
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

void updateLogic() {
  // 1. Aplicar la dirección cacheada
  dirX = nextDirX;
  dirY = nextDirY;

  // 2. Calcular nueva posición de la cabeza
  int nextX = snakeX[0] + dirX;
  int nextY = snakeY[0] + dirY;

  // 3. Comprobar colisiones contra los bordes (Muerte)
  if (nextX < 0 || nextX >= GRID_W || nextY < 0 || nextY >= GRID_H) {
    gameState = 2;
    return;
  }

  // 4. Comprobar colisiones contra sí misma
  // No comprobamos la última pieza (snakeLen-1) porque se va a mover
  for (int i = 1; i < snakeLen; i++) {
    if (nextX == snakeX[i] && nextY == snakeY[i]) {
      gameState = 2;
      return;
    }
  }

  // 5. ¿Ha comido la manzana?
  bool ateFood = (nextX == foodX && nextY == foodY);
  int tailX = snakeX[snakeLen - 1];
  int tailY = snakeY[snakeLen - 1];

  if (ateFood) {
    snakeLen++;
    score += 10;
    if(moveDelay > 50) moveDelay -= 2; // Aumentar velocidad progresivamente
    spawnFood();
    drawUI();
  }

  // 6. Desplazar todo el cuerpo en el array
  for (int i = snakeLen - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }
  
  // 7. Establecer nueva cabeza
  snakeX[0] = nextX;
  snakeY[0] = nextY;

  // 8. Dibujado ultra eficiente
    // Si NO ha comido, borramos la cola vieja
    if (!ateFood) {
      tft.fillRect(getPx(tailX), getPy(tailY), BLOCK_SIZE, BLOCK_SIZE, COLOR_BG);
    }
    
    // El cuello (antigua cabeza) pasa a ser verde normal
    tft.fillRect(getPx(snakeX[1]), getPy(snakeY[1]), BLOCK_SIZE, BLOCK_SIZE, COLOR_SNAKE);
    tft.drawRect(getPx(snakeX[1]), getPy(snakeY[1]), BLOCK_SIZE, BLOCK_SIZE, COLOR_BG);
    
    // Pintamos la nueva cabeza verde brillante
    tft.fillRect(getPx(snakeX[0]), getPy(snakeY[0]), BLOCK_SIZE, BLOCK_SIZE, COLOR_HEAD);
    tft.drawRect(getPx(snakeX[0]), getPy(snakeY[0]), BLOCK_SIZE, BLOCK_SIZE, COLOR_BG);
}

void loop() {
  unsigned long currentMillis = millis();

  // Botón SELECT: Apagar/Encender pantalla
  static unsigned long selectPressStart = 0;
  static bool selectHasTriggered = false;
  bool doToggleScreen = false;
  if (digitalRead(PIN_SELECT) == LOW) {
    if (selectPressStart == 0) selectPressStart = currentMillis;
    if (!selectHasTriggered && currentMillis - selectPressStart >= 1000 && currentMillis - lastButtonTime > 300) {
      doToggleScreen = true;
      selectHasTriggered = true;
    }
  } else {
    selectPressStart = 0;
    selectHasTriggered = false;
  }
  if (doToggleScreen) {
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
    // PAUSA
    if (digitalRead(PIN_START) == LOW && currentMillis - lastButtonTime > 300) {
        gameState = 3;
        showPause();
        lastButtonTime = currentMillis;
        return;
    }

    // LEER CONTROLES (Buffered)
    // No dejamos que cambie de sentido 180º para que no se coma a sí misma instantáneamente
    if (digitalRead(PIN_UP) == LOW    && dirY == 0) { nextDirX = 0;  nextDirY = -1; }
    if (digitalRead(PIN_DOWN) == LOW  && dirY == 0) { nextDirX = 0;  nextDirY = 1;  }
    if (digitalRead(PIN_LEFT) == LOW  && dirX == 0) { nextDirX = -1; nextDirY = 0;  }
    if (digitalRead(PIN_RIGHT) == LOW && dirX == 0) { nextDirX = 1;  nextDirY = 0;  }

    // MOVER SERPIENTE AL RITMO DEL TICK
    if (currentMillis - lastMoveTime > moveDelay) {
      updateLogic();
      if(gameState == 2) gameOver();
      lastMoveTime = currentMillis;
    }
    
  } 
  else if (gameState == 2) { // GAME OVER
    if (digitalRead(PIN_START) == LOW && currentMillis - lastButtonTime > 300) {
      while(digitalRead(PIN_START) == LOW) { delay(10); } // Antirrebote
      showMenu();
      lastButtonTime = millis();
    }
  }
  else if (gameState == 3) { // PAUSA
    if (digitalRead(PIN_START) == LOW && currentMillis - lastButtonTime > 300) {
        fullRedraw();
        gameState = 1;
        lastMoveTime = currentMillis; // Evitar salto brusco al volver
        lastButtonTime = currentMillis;
    }
  }
}

} // namespace Game_Snake
#undef COLOR_BG
#undef COLOR_UI
#undef BLOCK_SIZE
#undef COLOR_FOOD
#undef GRID_H
#undef COLOR_HEAD
#undef GRID_W
#undef COLOR_SNAKE

namespace Game_Space_invaders {
void setup();
void showMenu();
void initAliens();
void resetGame();
void nextLevel();
void eraseOldPositions(int oldShipX, int oldBulletY);
void drawGame();
void showPause();
void updateLogic();
void gameOver();
void loop();

// #include <TFT_eSPI.h>

// TFT_eSPI tft = TFT_eSPI();

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
  static unsigned long selectPressStart = 0;
  static bool selectHasTriggered = false;
  bool doToggleScreen = false;
  if (digitalRead(PIN_SELECT) == LOW) {
    if (selectPressStart == 0) selectPressStart = currentMillis;
    if (!selectHasTriggered && currentMillis - selectPressStart >= 1000 && currentMillis - lastButtonTime > 300) {
      doToggleScreen = true;
      selectHasTriggered = true;
    }
  } else {
    selectPressStart = 0;
    selectHasTriggered = false;
  }
  if (doToggleScreen) {
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

} // namespace Game_Space_invaders
#undef COLOR_BG
#undef NUM_ALIENS

namespace Game_Tetris {
void drawUI();
void spawnPiece();
void drawBlock(int x, int y, int colorIndex);
void drawBoard();
void drawCurrentPiece(bool erase );
bool checkCollision(int offsetX, int offsetY, int testRotation);
void rotatePiece();
void mergePiece();
void clearLines();
void gameOver();
void setup();
void loop();

// #include <TFT_eSPI.h>

// TFT_eSPI tft = TFT_eSPI();

// --- CONFIGURACIÓN DE PINES ---
const int PIN_MOVE_LEFT  = 15;  
const int PIN_MOVE_RIGHT = 27;  
const int PIN_ROTATE     = 25;  
const int PIN_DROP       = 26;  

const int PIN_START  = 13;
const int PIN_SELECT = 33;
const int PIN_TFT_BL = 21;

// --- CONFIGURACIÓN DEL JUEGO ---
#define BLOCK_SIZE 8
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20
#define OFFSET_X 4
#define OFFSET_Y 0

int board[BOARD_WIDTH][BOARD_HEIGHT] = {0};

// Matrices corregidas para evitar desplazamientos al rotar
const int pieces[7][4][4][4] = {
  { // I - Cyan (Solo 2 posiciones)
    {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
    {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}},
    {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
    {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}}
  },
  { // O - Amarillo (No rota)
    {{0,2,2,0},{0,2,2,0},{0,0,0,0},{0,0,0,0}},
    {{0,2,2,0},{0,2,2,0},{0,0,0,0},{0,0,0,0}},
    {{0,2,2,0},{0,2,2,0},{0,0,0,0},{0,0,0,0}},
    {{0,2,2,0},{0,2,2,0},{0,0,0,0},{0,0,0,0}}
  },
  { // J - Azul
    {{3,0,0,0},{3,3,3,0},{0,0,0,0},{0,0,0,0}},
    {{0,3,3,0},{0,3,0,0},{0,3,0,0},{0,0,0,0}},
    {{0,0,0,0},{3,3,3,0},{0,0,3,0},{0,0,0,0}},
    {{0,3,0,0},{0,3,0,0},{3,3,0,0},{0,0,0,0}}
  },
  { // L - Naranja
    {{0,0,4,0},{4,4,4,0},{0,0,0,0},{0,0,0,0}},
    {{0,4,0,0},{0,4,0,0},{0,4,4,0},{0,0,0,0}},
    {{0,0,0,0},{4,4,4,0},{4,0,0,0},{0,0,0,0}},
    {{4,4,0,0},{0,4,0,0},{0,4,0,0},{0,0,0,0}}
  },
  { // S - Verde (Solo 2 posiciones)
    {{0,5,5,0},{5,5,0,0},{0,0,0,0},{0,0,0,0}},
    {{0,5,0,0},{0,5,5,0},{0,0,5,0},{0,0,0,0}},
    {{0,5,5,0},{5,5,0,0},{0,0,0,0},{0,0,0,0}},
    {{0,5,0,0},{0,5,5,0},{0,0,5,0},{0,0,0,0}}
  },
  { // Z - Rojo (Solo 2 posiciones)
    {{6,6,0,0},{0,6,6,0},{0,0,0,0},{0,0,0,0}},
    {{0,0,6,0},{0,6,6,0},{0,6,0,0},{0,0,0,0}},
    {{6,6,0,0},{0,6,6,0},{0,0,0,0},{0,0,0,0}},
    {{0,0,6,0},{0,6,6,0},{0,6,0,0},{0,0,0,0}}
  },
  { // T - Morado
    {{0,7,0,0},{7,7,7,0},{0,0,0,0},{0,0,0,0}},
    {{0,7,0,0},{0,7,7,0},{0,7,0,0},{0,0,0,0}},
    {{0,0,0,0},{7,7,7,0},{0,7,0,0},{0,0,0,0}},
    {{0,7,0,0},{7,7,0,0},{0,7,0,0},{0,0,0,0}}
  }
};

uint16_t colors[8] = {
  TFT_BLACK, TFT_CYAN, TFT_YELLOW, TFT_BLUE, 
  TFT_ORANGE, TFT_GREEN, TFT_RED, TFT_PURPLE
};

// Variables del juego
int currentPieceId; 
int currentRotation = 0; 
int currentX = 3, currentY = 0;

unsigned long lastDropTime = 0;
int baseDropDelay = 600; 
int currentDropDelay = 600;

// Variables de Puntuación
unsigned long score = 0;
int linesCleared = 0;
int level = 1;

// Variables de Estado
bool isPaused = false;
bool isScreenOn = true;
unsigned long lastButtonTime = 0; 

// --- FUNCIONES BÁSICAS ---

void drawUI() {
  tft.fillRect(86, 0, 42, 160, TFT_BLACK); 
  tft.setTextColor(TFT_WHITE);
  
  tft.drawString("SCORE", 90, 10, 1);
  tft.setTextColor(TFT_YELLOW);
  tft.drawString(String(score), 90, 20, 1);
  
  tft.setTextColor(TFT_WHITE);
  tft.drawString("LEVEL", 90, 40, 1);
  tft.setTextColor(TFT_CYAN);
  tft.drawString(String(level), 90, 50, 1);
  
  tft.setTextColor(TFT_WHITE);
  tft.drawString("LINES", 90, 70, 1);
  tft.setTextColor(TFT_GREEN);
  tft.drawString(String(linesCleared), 90, 80, 1);
}

void spawnPiece() {
  currentPieceId = random(0, 7);
  currentRotation = 0;
  currentX = 3; currentY = 0;
}

void drawBlock(int x, int y, int colorIndex) {
  tft.fillRect(OFFSET_X + x * BLOCK_SIZE, OFFSET_Y + y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, colors[colorIndex]);
  if(colorIndex != 0) {
    tft.drawRect(OFFSET_X + x * BLOCK_SIZE, OFFSET_Y + y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, TFT_DARKGREY);
  }
}

void drawBoard() {
  for (int x = 0; x < BOARD_WIDTH; x++) {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
      drawBlock(x, y, board[x][y]);
    }
  }
  // Redibujar siempre el marco exterior por si las piezas lo tapan
  tft.drawRect(OFFSET_X - 1, OFFSET_Y, BOARD_WIDTH * BLOCK_SIZE + 2, BOARD_HEIGHT * BLOCK_SIZE, TFT_WHITE);
}

void drawCurrentPiece(bool erase = false) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      int val = pieces[currentPieceId][currentRotation][j][i];
      if (val != 0) {
        drawBlock(currentX + i, currentY + j, erase ? 0 : val);
      }
    }
  }
}

bool checkCollision(int offsetX, int offsetY, int testRotation) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      int val = pieces[currentPieceId][testRotation][j][i];
      if (val != 0) {
        int nx = currentX + i + offsetX;
        int ny = currentY + j + offsetY;
        if (nx < 0 || nx >= BOARD_WIDTH || ny >= BOARD_HEIGHT || (ny >= 0 && board[nx][ny] != 0)) {
          return true;
        }
      }
    }
  }
  return false;
}

void rotatePiece() {
  int nextRotation = (currentRotation + 1) % 4;
  if (!checkCollision(0, 0, nextRotation)) {
    drawCurrentPiece(true); 
    currentRotation = nextRotation; 
    drawCurrentPiece(false); 
  }
}

void mergePiece() {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      int val = pieces[currentPieceId][currentRotation][j][i];
      if (val != 0 && (currentY + j >= 0)) {
        board[currentX + i][currentY + j] = val;
      }
    }
  }
}

void clearLines() {
  int linesThisTurn = 0;
  for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
    bool full = true;
    for (int x = 0; x < BOARD_WIDTH; x++) {
      if (board[x][y] == 0) full = false;
    }
    if (full) {
      linesThisTurn++;
      for (int yy = y; yy > 0; yy--) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
          board[x][yy] = board[x][yy - 1];
        }
      }
      for (int x = 0; x < BOARD_WIDTH; x++) board[x][0] = 0;
      y++; 
    }
  }
  
  if (linesThisTurn > 0) {
    if (linesThisTurn == 1) score += 40 * level;
    else if (linesThisTurn == 2) score += 100 * level;
    else if (linesThisTurn == 3) score += 300 * level;
    else if (linesThisTurn == 4) score += 1200 * level;
    
    linesCleared += linesThisTurn;
    level = (linesCleared / 10) + 1; 
    baseDropDelay = max(100, 600 - ((level - 1) * 50)); 
    
    drawBoard();
    drawUI();
  }
}

void gameOver() {
  tft.fillRect(OFFSET_X, 60, BOARD_WIDTH * BLOCK_SIZE, 40, TFT_RED);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("GAME OVER", OFFSET_X + (BOARD_WIDTH * BLOCK_SIZE)/2, 65, 2);
  tft.drawCentreString("Score: " + String(score), OFFSET_X + (BOARD_WIDTH * BLOCK_SIZE)/2, 85, 1);
  
  delay(1000);
  

  while(digitalRead(PIN_START) == HIGH) { delay(10); } 

  while(digitalRead(PIN_START) == LOW) { delay(10); } 
  
  memset(board, 0, sizeof(board));
  score = 0;
  linesCleared = 0;
  level = 1;
  baseDropDelay = 600;
  isPaused = false;
  
  lastButtonTime = millis();
  
  tft.fillScreen(TFT_BLACK);
  drawBoard();
  drawUI();
  spawnPiece();
}

// --- SETUP Y LOOP ---

void setup() {
  pinMode(PIN_MOVE_LEFT, INPUT_PULLUP);
  pinMode(PIN_MOVE_RIGHT, INPUT_PULLUP);
  pinMode(PIN_ROTATE, INPUT_PULLUP);
  pinMode(PIN_DROP, INPUT_PULLUP);
  pinMode(PIN_START, INPUT_PULLUP);
  pinMode(PIN_SELECT, INPUT_PULLUP);
  
  pinMode(PIN_TFT_BL, OUTPUT);
  digitalWrite(PIN_TFT_BL, HIGH); 

  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);
  
  drawBoard();
  drawUI();
  spawnPiece();
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Manejo del apagado de pantalla (SELECT)
  static unsigned long selectPressStart = 0;
  static bool selectHasTriggered = false;
  bool doToggleScreen = false;
  if (digitalRead(PIN_SELECT) == LOW) {
    if (selectPressStart == 0) selectPressStart = currentMillis;
    if (!selectHasTriggered && currentMillis - selectPressStart >= 1000 && currentMillis - lastButtonTime > 300) {
      doToggleScreen = true;
      selectHasTriggered = true;
    }
  } else {
    selectPressStart = 0;
    selectHasTriggered = false;
  }
  if (doToggleScreen) {
    isScreenOn = !isScreenOn;
    digitalWrite(PIN_TFT_BL, isScreenOn ? HIGH : LOW);
    
    if (isScreenOn && !isPaused) {
      drawBoard();
      drawCurrentPiece(false);
      drawUI();
    }
    lastButtonTime = currentMillis;
  }

  // 2. Manejo de la Pausa (START)
  if (digitalRead(PIN_START) == LOW && currentMillis - lastButtonTime > 300) {
    isPaused = !isPaused;
    
    if (isPaused) {
      tft.fillRect(OFFSET_X, 60, BOARD_WIDTH * BLOCK_SIZE, 30, TFT_BLACK);
      tft.drawRect(OFFSET_X, 60, BOARD_WIDTH * BLOCK_SIZE, 30, TFT_YELLOW);
      tft.setTextColor(TFT_YELLOW);
      tft.drawCentreString("PAUSA", OFFSET_X + (BOARD_WIDTH * BLOCK_SIZE)/2, 68, 2);
    } else {
      tft.fillRect(OFFSET_X, 60, BOARD_WIDTH * BLOCK_SIZE, 30, TFT_BLACK); 
      drawBoard();
      drawCurrentPiece(false);
      lastDropTime = millis(); 
    }
    lastButtonTime = currentMillis;
  }

  if (isPaused) return;

  // 3. Controles de Movimiento
  if (digitalRead(PIN_MOVE_LEFT) == LOW) {
    drawCurrentPiece(true);
    if (!checkCollision(-1, 0, currentRotation)) currentX--;
    drawCurrentPiece(false);
    delay(120);
  }
  
  if (digitalRead(PIN_MOVE_RIGHT) == LOW) {
    drawCurrentPiece(true);
    if (!checkCollision(1, 0, currentRotation)) currentX++;
    drawCurrentPiece(false);
    delay(120);
  }
  
  if (digitalRead(PIN_ROTATE) == LOW) {
    rotatePiece();
    delay(150);
  }
  
  if (digitalRead(PIN_DROP) == LOW) {
    currentDropDelay = 50; 
  } else {
    currentDropDelay = baseDropDelay;
  }

  // 4. Gravedad de la pieza
  if (currentMillis - lastDropTime > currentDropDelay) {
    drawCurrentPiece(true);
    if (!checkCollision(0, 1, currentRotation)) {
      currentY++;
    } else {
      drawCurrentPiece(false);
      mergePiece();
      clearLines();
      spawnPiece();
      
      if(checkCollision(0,0, currentRotation)) {
        gameOver();
      }
    }
    drawCurrentPiece(false);
    lastDropTime = currentMillis;
  }
}

} // namespace Game_Tetris
#undef BOARD_HEIGHT
#undef BOARD_WIDTH
#undef OFFSET_X
#undef OFFSET_Y
#undef BLOCK_SIZE


void drawMainMenu() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawCentreString("JULVE ARCADE", 64, 10, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  String menuItems[] = {
    "2048",
    "Arkanoid",
    "Capy Jump",
    "Flappy Capibara",
    "Highway Capy",
    "Snake",
    "Space Invaders",
    "Tetris",
  };
  int numGames = 8;
  
  for (int i=0; i<numGames; i++) {
    if (i == menuSelection) {
      tft.setTextColor(TFT_BLACK, TFT_WHITE);
    } else {
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft.drawCentreString(String(i+1) + ". " + menuItems[i], 64, 30 + (i*10), 1);
  }
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawCentreString("Usa UP/DOWN y START", 64, 140, 1);
}

void menuLoop() {
  unsigned long currentMillis = millis();
  // Basic menu navigation using UP (25) and DOWN (26)
  if (digitalRead(25) == LOW && currentMillis - lastGlobalButtonTime > 300) {
    menuSelection--;
    if (menuSelection < 0) menuSelection = 7;
    lastGlobalButtonTime = currentMillis;
    // Redraw menu selection indicator
    drawMainMenu();
  }
  if (digitalRead(26) == LOW && currentMillis - lastGlobalButtonTime > 300) {
    menuSelection++;
    if (menuSelection > 7) menuSelection = 0;
    lastGlobalButtonTime = currentMillis;
    drawMainMenu();
  }
  if (digitalRead(PIN_START_GLOBAL) == LOW && currentMillis - lastGlobalButtonTime > 300) {
    activeGame = menuSelection;
    lastGlobalButtonTime = currentMillis;
    isGlobalScreenOn = true; // Refresh global state to avoid sync issues when entering game
    digitalWrite(PIN_TFT_BL_GLOBAL, HIGH);
    // Call the setup of the active game
    if (activeGame == 0) Game_2048::setup();
    if (activeGame == 1) Game_Arkanoid::setup();
    if (activeGame == 2) Game_Capy_jump::setup();
    if (activeGame == 3) Game_Flappy_capibara::setup();
    if (activeGame == 4) Game_Highway_capy::setup();
    if (activeGame == 5) Game_Snake::setup();
    if (activeGame == 6) Game_Space_invaders::setup();
    if (activeGame == 7) Game_Tetris::setup();
  }
  
  // Toggle Backlight with SELECT button in the menu
  static unsigned long selectPressStartGlobal = 0;
  static bool selectHasTriggeredGlobal = false;
  bool doToggleScreenGlobal = false;
  if (digitalRead(PIN_SELECT_GLOBAL) == LOW) {
    if (selectPressStartGlobal == 0) selectPressStartGlobal = currentMillis;
    if (!selectHasTriggeredGlobal && currentMillis - selectPressStartGlobal >= 1000 && currentMillis - lastGlobalButtonTime > 300) {
      doToggleScreenGlobal = true;
      selectHasTriggeredGlobal = true;
    }
  } else {
    selectPressStartGlobal = 0;
    selectHasTriggeredGlobal = false;
  }
  if (doToggleScreenGlobal) {
    isGlobalScreenOn = !isGlobalScreenOn;
    digitalWrite(PIN_TFT_BL_GLOBAL, isGlobalScreenOn ? HIGH : LOW);
    lastGlobalButtonTime = currentMillis;
  }
}

void setup() {
  pinMode(PIN_START_GLOBAL, INPUT_PULLUP);
  pinMode(PIN_SELECT_GLOBAL, INPUT_PULLUP);
  pinMode(25, INPUT_PULLUP); // UP
  pinMode(26, INPUT_PULLUP); // DOWN
  pinMode(PIN_TFT_BL_GLOBAL, OUTPUT);
  digitalWrite(PIN_TFT_BL_GLOBAL, HIGH);
  
  tft.init();
  tft.setRotation(2);
  
  drawMainMenu();
}

void loop() {
  unsigned long currentMillis = millis();
  // Check for global exit (START + SELECT)
  if (digitalRead(PIN_START_GLOBAL) == LOW && digitalRead(PIN_SELECT_GLOBAL) == LOW) {
    if (activeGame != -1) {
      activeGame = -1;
      drawMainMenu();
      lastGlobalButtonTime = currentMillis + 500; // Debounce
      delay(500);
      return;
    }
  }
  
  if (activeGame == -1) {
    menuLoop();
  } else {
    if (activeGame == 0) Game_2048::loop();
    if (activeGame == 1) Game_Arkanoid::loop();
    if (activeGame == 2) Game_Capy_jump::loop();
    if (activeGame == 3) Game_Flappy_capibara::loop();
    if (activeGame == 4) Game_Highway_capy::loop();
    if (activeGame == 5) Game_Snake::loop();
    if (activeGame == 6) Game_Space_invaders::loop();
    if (activeGame == 7) Game_Tetris::loop();
  }
}
