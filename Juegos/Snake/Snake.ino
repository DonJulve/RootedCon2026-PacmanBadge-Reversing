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
