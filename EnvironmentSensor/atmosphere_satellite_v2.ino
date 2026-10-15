#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <U8g2lib.h>
#include <MQUnifiedsensor.h>

// ---------------- DHT11 ----------------
#define DHTpin 10
#define DHTTYPE DHT11
DHT dht(DHTpin, DHTTYPE);

// Temperature correction in degrees C (0.0 = show the raw sensor value)
#define TEMP_OFFSET 0.0

// ---------------- MQ-9 ------------------
#define MQ9_PIN A0
#define BOARD "Arduino UNO"
#define VOLTAGE_RESOLUTION 5.0
#define ADC_BIT_RESOLUTION 10
#define MQ9_TYPE "MQ-9"
#define MQ9_RL_KOHM 10.0
#define MQ9_R0_KOHM 10.0

MQUnifiedsensor MQ9(BOARD, VOLTAGE_RESOLUTION, ADC_BIT_RESOLUTION, MQ9_PIN, MQ9_TYPE);

// ---------------- OLED (U8g2 page buffer) ----------------
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ---------------- Variables -------------
float coPPM = 0;
float lpgPPM = 0;
int rawMQ9 = 0;
float Temp = 0;
float h = 0;
bool dhtOK = false;
unsigned long lastDHT = 0;

unsigned long lastBlink = 0;

char tempBuf[8];
char humBuf[8];
char coBuf[10];
char lpgBuf[10];
char adcBuf[8];

void fmtPPM(float value, char *buf) {
  if (isnan(value) || value > 9999.9) value = 9999.9;
  if (value < 0) value = 0;
  dtostrf(value, 1, 1, buf);
}

void doubleBlink() {
  for (byte i = 0; i < 2; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(60);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  delay(500);

  analogReference(DEFAULT);
  dht.begin();
  u8g2.begin();
  u8g2.setContrast(100); //Brightness (0-255)

  MQ9.setRL(MQ9_RL_KOHM);
  MQ9.setR0(MQ9_R0_KOHM);
  MQ9.setRegressionMethod(1);
  MQ9.init();

  Serial.println(F("Atmosphere Satellite ready"));
}

void loop() {
  // ---------- DHT11 (max once every 2 s) ----------
  if (millis() - lastDHT >= 2000) {
    lastDHT = millis();
    float hh = dht.readHumidity();
    float tt = dht.readTemperature();
    dhtOK = !isnan(hh) && !isnan(tt);
    if (dhtOK) {
      h = hh;
      Temp = tt + TEMP_OFFSET;
    }
  }

  // ---------- MQ-9 ----------
  rawMQ9 = analogRead(MQ9_PIN);
  MQ9.update();

  MQ9.setA(1000.5);
  MQ9.setB(-2.186);
  lpgPPM = MQ9.readSensor();

  MQ9.setA(599.65);
  MQ9.setB(-2.244);
  coPPM = MQ9.readSensor();

  // ---------- Prepare strings ----------
  if (dhtOK) {
    dtostrf(Temp, 1, 1, tempBuf);
    snprintf(humBuf, sizeof(humBuf), "%d%%", (int)h);
  } else {
    strcpy(tempBuf, "--");
    strcpy(humBuf, "--%");
  }
  fmtPPM(coPPM, coBuf);
  fmtPPM(lpgPPM, lpgBuf);
  snprintf(adcBuf, sizeof(adcBuf), "%d", rawMQ9);

  const char *status = dhtOK ? "NRML" : "DHT ERR";

  // ---------- Serial ----------
  Serial.print(F("Temp:")); Serial.print(tempBuf);
  Serial.print(F(" Hum:")); Serial.print(humBuf);
  Serial.print(F(" CO:")); Serial.print(coBuf);
  Serial.print(F(" LPG:")); Serial.print(lpgBuf);
  Serial.print(F(" Raw:")); Serial.println(adcBuf);

  // ---------- OLED ----------
  u8g2.firstPage();
  do {
    // Header bar (white bar, dark text)
    u8g2.setDrawColor(1);
    u8g2.drawBox(0, 0, 128, 11);
    u8g2.setDrawColor(0);
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(2, 9, "ATMOSPHERE SAT");
    u8g2.drawStr(126 - u8g2.getStrWidth(status), 9, status);
    u8g2.setDrawColor(1);

    // --- Temperature (left) ---
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(2, 21, "TEMP");
    u8g2.setFont(u8g2_font_10x20_tf);
    u8g2.drawStr(1, 40, tempBuf);
    int w = u8g2.getStrWidth(tempBuf);
    u8g2.drawGlyph(w + 2, 40, 0xB0);   // degree sign
    u8g2.drawStr(w + 10, 40, "C");

    // --- Humidity (right) ---
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(68, 21, "HUMIDITY");
    u8g2.setFont(u8g2_font_10x20_tf);
    u8g2.drawStr(68, 40, humBuf);

    // Dividers
    u8g2.drawVLine(64, 13, 30);
    u8g2.drawHLine(0, 44, 128);
    u8g2.drawVLine(42, 45, 19);
    u8g2.drawVLine(86, 45, 19);

    // --- Gas row: label on top, value below ---
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(2, 53, "CO ppm");
    u8g2.drawStr(46, 53, "LPG ppm");
    u8g2.drawStr(90, 53, "MQ9 raw");

    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(2, 63, coBuf);
    u8g2.drawStr(46, 63, lpgBuf);
    u8g2.drawStr(90, 63, adcBuf);
  } while (u8g2.nextPage());

  if (millis() - lastBlink >= 2000) {
  lastBlink = millis();
  doubleBlink();
  }

  delay(250);
}
