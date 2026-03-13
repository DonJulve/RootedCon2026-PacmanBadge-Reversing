#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

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
  if (digitalRead(PIN_SELECT) == LOW && currentMillis - lastButtonTime > 300) {
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
