#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

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
  if (digitalRead(PIN_SELECT) == LOW && currentMillis - lastButtonTime > 300) {
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
