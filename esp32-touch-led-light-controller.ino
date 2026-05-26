#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

/*
====================================================
ESP32 TOUCH LED CONTROLLER
ESP32 + TTP223
====================================================

FEATURES
----------------------------------------------------
✔ Stable LED ON/OFF
✔ No fast blinking issue
✔ Better touch accuracy
✔ Long press support
✔ ntfy notifications
✔ LED state restore
✔ Noise filtering
✔ Lower CPU usage
✔ Lower ESP32 heat
✔ Better memory cleanup
✔ HTTPS optimized
✔ WiFi auto reconnect
✔ Sequential LED handling
✔ Production stable logic
✔ Proper debounce delay
✔ False touch prevention

TOUCH ACTIONS
----------------------------------------------------
👆 Single Touch
→ LED ON/OFF

✋ Long Touch
→ Blink LED 5 times

LED STATUS
----------------------------------------------------
🚀 Boot      -> 3 blinks
📶 WiFi      -> Slow blink
💡 Toggle    -> Stable ON/OFF
✋ Long      -> Blink 5 times
📢 NTFY      -> Tiny blink

WIRING
----------------------------------------------------
TTP223 SIG  -> GPIO4
TTP223 VCC  -> 3V3
TTP223 GND  -> GND

LED
----------------------------------------------------
LED -> GPIO5
====================================================
*/

// =====================================================
// WIFI
// =====================================================

const char* WIFI_SSID     = "";
const char* WIFI_PASSWORD = "";

// =====================================================
// NTFY
// =====================================================

const char* NTFY_URL =
"";

// =====================================================
// GPIO
// =====================================================

#define TOUCH_PIN  4
#define LED_PIN    5

// =====================================================
// TOUCH
// =====================================================

bool lastTouchState = LOW;
bool longTouchDone  = false;
bool ledState       = false;

unsigned long touchStartTime = 0;
unsigned long lastTouchRead  = 0;
unsigned long lastWiFiRetry  = 0;
unsigned long lastToggleTime = 0;

// =====================================================
// TIMING
// =====================================================

const unsigned long TOUCH_DEBOUNCE =
50;

const unsigned long LONG_TOUCH_MS =
1500;

const unsigned long TOGGLE_COOLDOWN =
500;

const unsigned long WIFI_RETRY_MS =
30000;

// =====================================================
// HTTP
// =====================================================

const uint16_t HTTP_TIMEOUT_MS =
5000;

// =====================================================
// SAFE DELAY
// =====================================================

void safeDelay(unsigned long ms) {

  unsigned long start =
    millis();

  while (
    millis() - start < ms
  ) {

    delay(1);

    yield();
  }
}

// =====================================================
// LED CONTROL
// =====================================================

inline void setLED(
  bool state
) {

  ledState = state;

  digitalWrite(
    LED_PIN,
    state
    ? HIGH
    : LOW
  );
}

inline void ledOn() {

  setLED(true);
}

inline void ledOff() {

  setLED(false);
}

// =====================================================
// LED BLINK
// =====================================================

void blinkLED(
  uint8_t times,
  uint16_t onMs,
  uint16_t offMs
) {

  // save LED state
  bool previousState =
    ledState;

  for (
    uint8_t i = 0;
    i < times;
    i++
  ) {

    digitalWrite(
      LED_PIN,
      HIGH
    );

    safeDelay(onMs);

    digitalWrite(
      LED_PIN,
      LOW
    );

    safeDelay(offMs);
  }

  // restore previous state
  digitalWrite(
    LED_PIN,
    previousState
    ? HIGH
    : LOW
  );

  safeDelay(80);
}

// =====================================================
// LED STATES
// =====================================================

void bootLED() {

  blinkLED(3, 90, 90);
}

void wifiLED() {

  blinkLED(1, 180, 120);
}

void ntfyLED() {

  blinkLED(1, 25, 25);
}

// =====================================================
// WIFI
// =====================================================

bool connectWiFi() {

  if (
    WiFi.status() ==
    WL_CONNECTED
  ) {

    return true;
  }

  Serial.println(
    "\n📶 Connecting WiFi..."
  );

  WiFi.disconnect(
    true,
    true
  );

  safeDelay(300);

  WiFi.mode(WIFI_STA);

  // lower heat
  WiFi.setSleep(true);

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );

  unsigned long startTime =
    millis();

  while (
    WiFi.status() !=
    WL_CONNECTED &&
    millis() -
    startTime <
    15000
  ) {

    wifiLED();
  }

  if (
    WiFi.status() ==
    WL_CONNECTED
  ) {

    Serial.println(
      "✅ WiFi Connected"
    );

    Serial.print(
      "🌐 IP: "
    );

    Serial.println(
      WiFi.localIP()
    );

    return true;
  }

  Serial.println(
    "❌ WiFi Failed"
  );

  return false;
}

// =====================================================
// NTFY
// =====================================================

void sendNtfy(
  const String& title,
  const String& body
) {

  ntfyLED();

  if (
    WiFi.status() !=
    WL_CONNECTED
  ) {

    Serial.println(
      "❌ NTFY skipped"
    );

    return;
  }

  WiFiClientSecure client;

  client.setInsecure();

  client.setTimeout(
    HTTP_TIMEOUT_MS
  );

  HTTPClient http;

  http.setReuse(false);

  http.setConnectTimeout(
    HTTP_TIMEOUT_MS
  );

  http.setTimeout(
    HTTP_TIMEOUT_MS
  );

  bool started =
    http.begin(
      client,
      NTFY_URL
    );

  if (!started) {

    Serial.println(
      "❌ NTFY begin failed"
    );

    client.stop();

    return;
  }

  http.addHeader(
    "Title",
    title
  );

  http.addHeader(
    "Tags",
    "bulb"
  );

  int code =
    http.POST(body);

  Serial.print(
    "📢 NTFY: "
  );

  Serial.println(code);

  http.end();

  client.stop();

  safeDelay(100);
}

// =====================================================
// TOUCH HANDLER
// =====================================================

void handleTouch() {

  // debounce
  if (
    millis() -
    lastTouchRead <
    TOUCH_DEBOUNCE
  ) {

    return;
  }

  lastTouchRead =
    millis();

  bool touched =
    digitalRead(
      TOUCH_PIN
    );

  // ===================================================
  // TOUCH START
  // ===================================================

  if (
    touched &&
    !lastTouchState
  ) {

    touchStartTime =
      millis();

    longTouchDone =
      false;
  }

  // ===================================================
  // LONG TOUCH
  // ===================================================

  if (
    touched &&
    !longTouchDone &&
    millis() -
    touchStartTime >=
    LONG_TOUCH_MS
  ) {

    Serial.println(
      "✋ LONG TOUCH"
    );

    longTouchDone =
      true;

    blinkLED(5, 120, 120);

    sendNtfy(
      "✋ Long Touch",
      "💡 LED blinked 5 times"
    );

    // stabilize sensor
    safeDelay(500);
  }

  // ===================================================
  // TOUCH RELEASE
  // ===================================================

  if (
    !touched &&
    lastTouchState
  ) {

    // prevent toggle after long touch
    if (!longTouchDone) {

      // cooldown protection
      if (
        millis() -
        lastToggleTime >
        TOGGLE_COOLDOWN
      ) {

        lastToggleTime =
          millis();

        // toggle once only
        setLED(
          !ledState
        );

        Serial.println(
          ledState
          ? "💡 LED ON"
          : "🌑 LED OFF"
        );

        sendNtfy(
          "👆 LED Toggle",
          ledState
          ? "💡 LED turned ON"
          : "🌑 LED turned OFF"
        );

        // sensor stabilize
        safeDelay(400);
      }
    }
  }

  // ===================================================
  // SAVE TOUCH STATE
  // ===================================================

  lastTouchState =
    touched;
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  pinMode(
    TOUCH_PIN,
    INPUT
  );

  pinMode(
    LED_PIN,
    OUTPUT
  );

  ledOff();

  bootLED();

  bool wifiOK =
    connectWiFi();

  if (wifiOK) {

    sendNtfy(
      "🚀 ESP32 Online",
      "✅ Touch LED controller started"
    );
  }

  Serial.println(
    "\n================================="
  );

  Serial.println(
    "🚀 ESP32 Touch Ready"
  );

  Serial.println(
    "================================="
  );
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  // ===================================================
  // WIFI AUTO RECONNECT
  // ===================================================

  if (
    WiFi.status() !=
    WL_CONNECTED &&
    millis() -
    lastWiFiRetry >
    WIFI_RETRY_MS
  ) {

    lastWiFiRetry =
      millis();

    connectWiFi();
  }

  // ===================================================
  // TOUCH
  // ===================================================

  handleTouch();

  // ===================================================
  // LOW CPU USAGE
  // ===================================================

  delay(2);
}
