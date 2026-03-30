#include <WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI(); 

const int PIN_UP     = 15;
const int PIN_DOWN   = 27;
const int PIN_LEFT   = 25;
const int PIN_RIGHT  = 26;
const int PIN_START  = 33;
const int PIN_SELECT = 13;

const char* ssid = "Capibot";     
const char* password = "capibotcapibot";    

const char* robotIP = "192.168.4.1";
const int robotPort = 4210;

WiFiUDP udp;

int lastMotorA = 0;
int lastMotorB = 0;
int speedLimit = 255; 

int lastServoAngle = 0;
bool isStopped = false; 

void logAction(String action, String values, uint16_t color) {
  Serial.print("[TX] ");
  Serial.print(action);
  if (values != "") {
    Serial.print(" ");
    Serial.print(values);
  }
  Serial.println();

  tft.fillRect(0, 50, 160, 78, TFT_BLACK); 
  
  tft.setTextSize(2);
  tft.setTextColor(color);
  tft.setCursor(5, 60);
  tft.println(action);
  
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(5, 85);
  tft.println(values);
}

void sendCommand(String cmdType, String cmdValues, uint16_t color) {
  String fullCmd = cmdType;
  if (cmdValues != "") fullCmd += " " + cmdValues;
  
  udp.beginPacket(robotIP, robotPort);
  udp.print(fullCmd + "\n"); 
  udp.endPacket();
  
  logAction(cmdType, cmdValues, color);
}

void setup() {
  Serial.begin(115200);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  tft.init();
  tft.setRotation(1); 
  tft.fillScreen(TFT_BLACK);
  
  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(5, 5);
  tft.println("CAPI BOT");
  tft.drawLine(0, 25, 160, 25, TFT_CYAN);

  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_START, INPUT_PULLUP);
  pinMode(PIN_SELECT, INPUT_PULLUP);

  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(5, 35);
  tft.print("Conectando WiFi...");
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  tft.fillRect(0, 30, 160, 20, TFT_BLACK); 
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(5, 35);
  tft.print("WiFi Conectado!");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) return;

  bool up    = (digitalRead(PIN_UP) == LOW);
  bool down  = (digitalRead(PIN_DOWN) == LOW);
  bool left  = (digitalRead(PIN_LEFT) == LOW);
  bool right = (digitalRead(PIN_RIGHT) == LOW);
  bool btnStart  = (digitalRead(PIN_START) == LOW);
  bool btnSelect = (digitalRead(PIN_SELECT) == LOW);

  if (btnStart && btnSelect) {
    if (!isStopped) {
      sendCommand("STOP", "", TFT_RED);
      isStopped = true;
      lastMotorA = 0;
      lastMotorB = 0;
      lastServoAngle = -1;
    }
  } 
  else {
    isStopped = false;

    int fwd = 0, turn = 0;
    if (up) fwd += 1;
    if (down) fwd -= 1;
    if (left) turn -= 1;
    if (right) turn += 1;

    float a = 0, b = 0;
    if (fwd != 0) {
      a = fwd * speedLimit; b = fwd * speedLimit;
      if (turn > 0) b = b * 0.2;      
      if (turn < 0) a = a * 0.2;      
    } else if (turn != 0) {
      a = (turn > 0) ? speedLimit : -speedLimit;
      b = (turn > 0) ? -speedLimit : speedLimit;
    }

    int motorA = round(a);
    int motorB = round(b);

    if (motorA != lastMotorA || motorB != lastMotorB) {
      lastMotorA = motorA; lastMotorB = motorB;
      sendCommand("MOTORS", String(motorA) + " " + String(motorB), TFT_GREEN);
    }

    int targetAngle = 0;
    if (btnStart) {
      targetAngle = 180;
    } else if (btnSelect) {
      targetAngle = 90;
    }
    
    if (targetAngle != lastServoAngle) {
      sendCommand("SERVO 1", String(targetAngle), TFT_ORANGE);
      lastServoAngle = targetAngle;
    }
  }

  delay(20); 
}
