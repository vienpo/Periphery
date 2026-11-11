# Periphery

A heads-up security HUD that detects network threats and displays alerts on smart glasses.

> Current stage: prototype. An ESP32 + OLED stands in for the glasses, and a Python script sends fake alerts.

<!-- Add a photo of the OLED showing an alert: docs/images/prototype.jpg -->

## How it works

```
send_alerts.py (laptop)  --POST /alert (JSON)-->  ESP32  -->  OLED shows alert
send_alerts.py (laptop)  <--GET /status--------   ESP32  <--  BOOT button dismisses
```

Sensors detect events and send short JSON alerts (see [docs/alert-format.md](docs/alert-format.md)). The display shows them and reports when you dismiss one.

## Hardware

- ESP32 dev board (classic ESP32)
- 0.96" SSD1306 or 1.3" SH1106 128x64 I2C OLED
- 4 female-to-female jumper wires

| OLED pin | ESP32 pin |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

## Setup

1. Install the Arduino IDE, ESP32 board support (Espressif Systems), and the **U8g2** and **ArduinoJson** libraries.
2. Optional: upload `firmware/oled_test/oled_test.ino` to confirm the wiring.
3. In `firmware/alert_display/`, copy `secrets.example.h` to `secrets.h` and fill in your 2.4 GHz Wi-Fi name and password.
4. Upload `firmware/alert_display/alert_display.ino`. The OLED shows its IP address.
5. With your laptop on the same network, run:
   ```
   python3 sensors/send_alerts.py <ip-shown-on-oled>
   ```
6. Press BOOT on the ESP32 to dismiss an alert.

## Roadmap

- [x] ESP32 + OLED prototype display with two-way communication
- [ ] Real detection on the laptop with Scapy (new devices, port scans)
- [ ] Raspberry Pi network sniffer (Zeek / Suricata)
- [ ] ESP32 control hub (IR, sub-GHz, relays)
- [ ] Even Realities G2 glasses app (Even Hub SDK)
- [ ] WiFi Pineapple Pager for Wi-Fi-layer detection

## Disclaimer

This project is for monitoring networks and devices you own or are authorized to test. Only transmit to your own devices, on legal frequencies and power.
