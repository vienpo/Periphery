"""
Periphery - Fake alert sender
Sends a random fake security alert to the ESP32 display every few seconds,
and reports when you dismiss one with the BOOT button.

Run:   python3 send_alerts.py
   or: python3 send_alerts.py 192.168.1.42    (use the IP shown on the OLED if periphery.local doesn't work)
Stop:  Ctrl + C
"""

import json
import random
import sys
import time
import urllib.error
import urllib.request

HOST = sys.argv[1] if len(sys.argv) > 1 else "periphery.local"
BASE_URL = f"http://{HOST}"
SECONDS_BETWEEN_ALERTS = 8

FAKE_ALERTS = [
    {"severity": "high",   "type": "port_scan",           "msg": "Port scan on laptop from unknown device"},
    {"severity": "medium", "type": "new_device",          "msg": "New device joined: Espressif chip"},
    {"severity": "high",   "type": "beaconing",           "msg": "TV beaconing to 45.33.x.x every 60s"},
    {"severity": "medium", "type": "unusual_destination", "msg": "Phone contacted a never-seen domain"},
    {"severity": "high",   "type": "deauth",              "msg": "Deauth flood detected on home WiFi"},
    {"severity": "low",    "type": "traffic_spike",       "msg": "Laptop upload spike: 400 MB in 2 min"},
    {"severity": "high",   "type": "evil_twin",           "msg": "Fake AP copying your network name"},
]


def post(path, data):
    body = json.dumps(data).encode("utf-8")
    req = urllib.request.Request(
        BASE_URL + path, data=body, headers={"Content-Type": "application/json"}, method="POST"
    )
    with urllib.request.urlopen(req, timeout=5) as resp:
        return json.loads(resp.read())


def get(path):
    with urllib.request.urlopen(BASE_URL + path, timeout=5) as resp:
        return json.loads(resp.read())


def main():
    print(f"Sending fake alerts to {BASE_URL} (Ctrl + C to stop)\n")
    last_dismissed = None

    while True:
        try:
            alert = random.choice(FAKE_ALERTS)
            post("/alert", alert)
            print(f"-> sent  [{alert['severity'].upper()}] {alert['msg']}")

            # Check the display's status each second until the next alert
            for _ in range(SECONDS_BETWEEN_ALERTS):
                time.sleep(1)
                status = get("/status")
                dismissed = status["alerts_dismissed"]
                if last_dismissed is not None and dismissed > last_dismissed:
                    print("<- you dismissed the alert on Periphery")
                last_dismissed = dismissed

        except (urllib.error.URLError, TimeoutError, ConnectionError) as e:
            print(f"Can't reach Periphery at {BASE_URL} ({e}).")
            print("Check the ESP32 is on, or run with the IP shown on the screen:")
            print("    python3 send_alerts.py 192.168.x.x")
            time.sleep(5)
        except KeyboardInterrupt:
            print("\nStopped.")
            break


if __name__ == "__main__":
    main()
