#include <Arduino.h> // Dependencies
#include <Wire.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* ---------- Define Pins ---------- */
#define SEND_PIN 12         // D6 (GPIO12) IR transmitter
#define DHTPIN 2            // D4 (GPIO2) DHT11 data pin
#define DHTTYPE DHT11       // DHT11 sensor
#define OLED_ADDR 0x3C      // OLED I2C address
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

/* ---------- Define Global objects ---------- */
IRsend irsend(SEND_PIN);
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/* ---------- Mitsubishi AC ON/OFF frames (Obtained from IRrecvDumpV3 example) ---------- */
uint8_t OFF_STATE[18] = {
  0x23, 0xCB, 0x26, 0x01, 0x00, 0x00, 0x18, 0x08,
  0x36, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xE3
};

uint8_t ON_STATE[18] = {
  0x23, 0xCB, 0x26, 0x01, 0x00, 0x20, 0x18, 0x08,
  0x36, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x03
};

/* ---------- AC Control logic ---------- */
enum AcState { AC_UNKNOWN, AC_ON, AC_OFF };
AcState acState = AC_UNKNOWN;
const float ON_THRESHOLD = 26.0;   // AC on threshold
const float OFF_THRESHOLD = 24.8;  // AC off threshold
const uint32_t MIN_SEND_INTERVAL_MS = 15000; // Delay
uint32_t lastSendMs = 0;

/* ---------- Light Sensor ---------- */
int readLightRaw() { return analogRead(A0); }

/* ---------- Functions Definitions ---------- */
void sendAcOn() {
  if (millis() - lastSendMs < MIN_SEND_INTERVAL_MS) return;
  irsend.sendMitsubishiAC(ON_STATE, sizeof(ON_STATE));
  delay(60);
  irsend.sendMitsubishiAC(ON_STATE, sizeof(ON_STATE));
  acState = AC_ON;
  lastSendMs = millis();
  Serial.println(F("[IR] Sent Mitsubishi AC ON"));
}

void sendAcOff() {
  if (millis() - lastSendMs < MIN_SEND_INTERVAL_MS) return;
  irsend.sendMitsubishiAC(OFF_STATE, sizeof(OFF_STATE));
  delay(60);
  irsend.sendMitsubishiAC(OFF_STATE, sizeof(OFF_STATE));
  acState = AC_OFF;
  lastSendMs = millis();
  Serial.println(F("[IR] Sent Mitsubishi AC OFF"));
}

void drawOLED(float t, float h, int light) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print(F("Env Control"));

  display.setCursor(0, 14);
  display.print(F("Temp: "));
  if (isnan(t)) display.print(F("--"));
  else { display.print(t, 1); display.print((char)247); display.print("C"); }

  display.setCursor(0, 26);
  display.print(F("Hum : "));
  if (isnan(h)) display.print(F("--"));
  else { display.print(h, 0); display.print(F("%")); }

  display.setCursor(0, 38);
  display.print(F("Light: "));
  display.print(light);

  display.setCursor(0, 50);
  display.print(F("AC: "));
  if (acState == AC_ON)      display.print(F("ON"));
  else if (acState == AC_OFF)display.print(F("OFF"));
  else                       display.print(F("?"));

  int barW = map(light, 0, 1023, 0, SCREEN_WIDTH);
  display.drawRect(0, 58, SCREEN_WIDTH, 6, SSD1306_WHITE);
  display.fillRect(0, 58, barW, 6, SSD1306_WHITE);

  display.display();
}

/* ---------- Setup Func ---------- */
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println(F("\n=== Env Control (DHT11 + OLED + IR) ==="));
  Serial.println(F("Type 'o' = ON, 'f' = OFF"));

  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("[OLED] SSD1306 init failed!"));
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("OLED Ready"));
    display.display();
  }

  dht.begin();
  irsend.begin();
}

/* ---------- Loop Func ---------- */
void loop() {
  // Manual overrides from Serial
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'o' || c == 'O') sendAcOn();
    else if (c == 'f' || c == 'F') sendAcOff();
  }

  static uint32_t lastRead = 0;
  if (millis() - lastRead >= 2000) { // update constantly
    lastRead = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();
    int lightVal = readLightRaw();

    // Auto control logic
    if (!isnan(t)) {
      if (t >= ON_THRESHOLD && acState != AC_ON)  sendAcOn();
      else if (t <= OFF_THRESHOLD && acState != AC_OFF) sendAcOff();
    }

    drawOLED(t, h, lightVal);

    Serial.print(F("[ENV] T=")); Serial.print(isnan(t) ? -999 : t);
    Serial.print(F("C H=")); Serial.print(isnan(h) ? -999 : h);
    Serial.print(F("% L=")); Serial.print(lightVal);
    Serial.print(F(" AC=")); Serial.println(acState == AC_ON ? "ON" : (acState == AC_OFF ? "OFF" : "?"));
  }
}
