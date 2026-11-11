# Alert format

Every sensor (fake sender, Scapy sniffer, Raspberry Pi, Pineapple Pager) sends alerts to the display in this JSON format.

```json
{
  "severity": "high",
  "type": "port_scan",
  "msg": "Port scan on laptop from unknown device"
}
```

## Fields

| Field | Required | Description |
|---|---|---|
| `severity` | yes | `low`, `medium`, or `high` |
| `type` | yes | Machine-readable event type (see below) |
| `msg` | yes | Short human-readable text shown on the HUD. Keep it under ~80 characters (4 lines of 21 on the OLED). |

## Event types

| Type | Meaning |
|---|---|
| `new_device` | Unfamiliar MAC address joined the network |
| `port_scan` | A device is probing ports on another device |
| `unusual_destination` | A device contacted an IP/domain it never has before |
| `beaconing` | A device checks in somewhere at regular intervals |
| `traffic_spike` | A device suddenly sends far more data than normal |
| `deauth` | Wi-Fi deauthentication flood detected |
| `evil_twin` | An access point is impersonating a known network |

## Display endpoints (ESP32)

| Method | Path | Purpose |
|---|---|---|
| `POST` | `/alert` | Send an alert (JSON body above) |
| `GET` | `/status` | Returns `showing_alert`, `alerts_received`, `alerts_dismissed` |
