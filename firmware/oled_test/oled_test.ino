// Periphery - OLED test
// Shows a message on the screen to confirm the wiring works.
// Wiring (classic ESP32): VCC -> 3V3, GND -> GND, SDA -> 21, SCL -> 22

#include <U8g2lib.h>
#include <Wire.h>

// Most 0.96" OLEDs use the SSD1306 chip:
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// If the screen shows garbage or a shifted image (common on 1.3" screens),
// put "//" in front of the line above and remove "//" from this line instead:
// U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void setup() {
  u8g2.begin();
}

void loop() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(0, 20, "Periphery");
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(0, 45, "Wiring works!");
  u8g2.sendBuffer();
  delay(1000);
}
