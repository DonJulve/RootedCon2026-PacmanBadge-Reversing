#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  
  // Luz
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH); 
  
  // Iniciar la librería
  tft.init();
  tft.setRotation(1); // Modo apaisado (Marauder usa rotación 1 o 3)
  
  // Limpiar pantalla en VERDE
  tft.fillScreen(TFT_GREEN); 
  
  // Escribir texto de victoria
  tft.setTextColor(TFT_BLACK, TFT_GREEN);
  tft.setTextSize(2);
  tft.drawString("MARAUDER", 20, 40);
  tft.drawString("READY!", 30, 70);
}

void loop() {
  // Fin del juego
  delay(1000);
}
