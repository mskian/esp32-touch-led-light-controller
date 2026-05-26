# ESP32 Touch LED Light Controller

A Simple ESP32 touch LED Light controller using TTP223 touch sensor.  

- Stable touch detection
- Long touch actions
- ntfy push notifications
- WiFi auto reconnect
- HTTPS support
- Low CPU usage

## Features

- Stable LED ON/OFF
- False touch prevention
- Long press support
- WiFi auto reconnect
- HTTPS optimized requests
- ntfy notifications
- Sequential LED blinking
- Lower ESP32 heat
- Better memory cleanup
- Production stable logic
- Debounce protection
- Low CPU usage

## Hardware Required

| Component           | Quantity |
| ------------------- | -------- |
| ESP32 Board         | 1        |
| TTP223 Touch Sensor | 1        |
| LED                 | 1        |
| Resistor (220Ω)     | 1        |
| Jumper Wires        | Some     |

## Wiring

#### TTP223 Touch Sensor

| TTP223 | ESP32 |
| ------ | ----- |
| SIG    | GPIO4 |
| VCC    | 3V3   |
| GND    | GND   |

#### LED

| LED      | ESP32 |
| -------- | ----- |
| Positive | GPIO5 |
| Negative | GND   |


#### Touch Actions

| Action       | Result            |
| ------------ | ----------------- |
| Single Touch | Toggle LED        |
| Long Touch   | Blink LED 5 Times |

#### LED Status

| Status            | LED Action |
| ----------------- | ---------- |
| Boot              | 3 Blinks   |
| WiFi Connecting   | Slow Blink |
| LED Toggle        | ON/OFF     |
| Long Touch        | 5 Blinks   |
| ntfy Notification | Tiny Blink |

## Open in Arduino IDE

- Configure WiFi

```sh
const char* WIFI_SSID     = "YOUR_WIFI";
const char* WIFI_PASSWORD = "YOUR_PASSWORD";
```

- Configure ntfy

```sh
const char* NTFY_URL =
"https://ntfy.sh/your-topic";
```

## License

MIT
