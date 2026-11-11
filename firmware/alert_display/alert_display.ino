// Periphery - Alert display (prototype HUD)
// Connects to Wi-Fi, waits for alerts from your laptop, and shows them on the OLED.
// Press the BOOT button to dismiss the current alert.
//
// Wiring (classic ESP32): VCC -> 3V3, GND -> GND, SDA -> 21, SCL -> 22
// Libraries needed (Tools -> Manage Libraries): U8g2, ArduinoJson

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include <Wire.h>

// Wi-Fi name and password live in secrets.h (not committed to Git).
// Copy secrets.example.h to secrets.h and fill in your network (must be 2.4 GHz).
#include "secrets.h"

// Same screen line as the test program. If you switched to SH1106 there, switch here too.
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
// U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

WebServer server(80);
const int BUTTON_PIN = 0;  // the BOOT button on most ESP32 boards

String alertSeverity = "";
String alertMessage  = "";
bool   hasAlert      = false;
int    alertCount    = 0;
int    dismissCount  = 0;

bool lastButtonState = HIGH;
unsigned long lastButtonChange = 0;

// Show two simple lines (used during startup)
void showMessage(const char* line1, const char* line2) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(0, 20, line1);
  u8g2.drawStr(0, 40, line2);
  u8g2.sendBuffer();
}

// Draw text broken into lines that fit the screen (21 characters per line)
void drawWrapped(const String& text, int x, int y, int maxChars, int maxLines) {
  String current = "";
  int line = 0;
  int start = 0;
  String words = text + " ";

  for (int i = 0; i < words.length(); i++) {
    if (words[i] != ' ') continue;
    String word = words.substring(start, i);
    start = i + 1;
    if (word.length() == 0) continue;

    String candidate = current.length() ? current + " " + word : word;
    if (candidate.length() <= maxChars) {
      current = candidate;
    } else if (current.length() == 0) {
      current = word.substring(0, maxChars);  // a single very long word
    } else {
      if (line < maxLines) u8g2.drawStr(x, y + line * 12, current.c_str());
      line++;
      current = word.substring(0, maxChars);
    }
  }
  if (current.length() && line < maxLines) {
    u8g2.drawStr(x, y + line * 12, current.c_str());
  }
}

void drawScreen() {
  u8g2.clearBuffer();

  if (!hasAlert) {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(0, 14, "Periphery");
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(0, 32, "No alerts");
    String ip = "IP: " + WiFi.localIP().toString();
    u8g2.drawStr(0, 50, ip.c_str());
    u8g2.drawStr(0, 62, "periphery.local");
  } else {
    u8g2.setFont(u8g2_font_6x10_tr);
    // Header bar with the severity, drawn as white box with black text
    u8g2.drawBox(0, 0, 128, 12);
    u8g2.setDrawColor(0);
    String header = "! " + alertSeverity;
    header.toUpperCase();
    u8g2.drawStr(2, 10, header.c_str());
    u8g2.setDrawColor(1);
    // The alert message, up to 4 lines
    drawWrapped(alertMessage, 0, 24, 21, 4);
  }

  u8g2.sendBuffer();
}

// POST /alert  with JSON like {"severity":"high","type":"port_scan","msg":"Port scan on laptop"}
void handleAlert() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"no body\"}");
    return;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  if (err) {
    server.send(400, "application/json", "{\"error\":\"bad json\"}");
    return;
  }

  alertSeverity = (const char*)(doc["severity"] | "info");
  alertMessage  = (const char*)(doc["msg"] | "(no message)");
  hasAlert = true;
  alertCount++;

  Serial.println("Alert: [" + alertSeverity + "] " + alertMessage);
  drawScreen();
  server.send(200, "application/json", "{\"ok\":true}");
}

// GET /status  lets the laptop see what the display is doing (two-way communication)
void handleStatus() {
  JsonDocument doc;
  doc["showing_alert"] = hasAlert;
  doc["alerts_received"] = alertCount;
  doc["alerts_dismissed"] = dismissCount;
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void checkButton() {
  bool reading = digitalRead(BUTTON_PIN);
  // Button reads LOW when pressed. Ignore bounces within 200 ms.
  if (reading == LOW && lastButtonState == HIGH && millis() - lastButtonChange > 200) {
    lastButtonChange = millis();
    if (hasAlert) {
      hasAlert = false;
      dismissCount++;
      Serial.println("Alert dismissed");
      drawScreen();
    }
  }
  lastButtonState = reading;
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  u8g2.begin();

  showMessage("Connecting to WiFi...", WIFI_NAME);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (millis() - startTime > 20000) {
      showMessage("WiFi failed!", "Check name/pass, 2.4GHz");
      delay(4000);
      ESP.restart();
    }
  }

  // Lets your laptop reach the board at http://periphery.local
  if (MDNS.begin("periphery")) {
    MDNS.addService("http", "tcp", 80);
  }

  server.on("/alert", HTTP_POST, handleAlert);
  server.on("/status", HTTP_GET, handleStatus);
  server.begin();

  Serial.print("Periphery ready at http://");
  Serial.println(WiFi.localIP());
  drawScreen();
}

void loop() {
  server.handleClient();
  checkButton();
}
