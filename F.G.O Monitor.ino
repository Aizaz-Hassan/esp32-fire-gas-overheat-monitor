#include "secrets.h"   // BLYNK_TEMPLATE_ID, BLYNK_TEMPLATE_NAME, BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS

#define BLYNK_PRINT Serial

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// WIFI CONFIGURATION
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASS;

// PIN CONFIGURATION
#define MQ2_ANALOG_PIN  36   // MQ-2 Analog Out (AO) connected to GPIO 36
#define DHTPIN          18   // DHT11 Data pin connected to D18
#define BUZZER_PIN       4   // Buzzer connected to D4
#define RED_LED_PIN     12   // RED LED connected to D12
#define GREEN_LED_PIN   14   // GREEN LED connected to D14
#define FLAME_SENSOR    34   // Flame Sensor Analog Out connected to D34

// SENSOR CONFIGURATION & THRESHOLDS
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int GAS_THRESHOLD = 900;
const float HIGH_TEMP_THRESHOLD = 45.0;
const float OVERHEAT_THRESHOLD = 55.0;
int threshold = 2000;                // Analog threshold for fire visibility

BlynkTimer timer;

void setup() {
  Serial.begin(115200);

  // Initializing Peripherals
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  // Safe State (Buzzer HIGH = OFF)
  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, HIGH);

  // Initializing Sensors & Screen
  dht.begin();
  lcd.init();
  lcd.backlight();

  // Boot Screen
  lcd.setCursor(2, 0); lcd.print("SYSTEM BOOT");
  lcd.setCursor(1, 1); lcd.print("F.G.O MONITOR");

  // CONNECT TO WIFI AND BLYNK
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Send data to Blynk every 1 second (1000ms)
  timer.setInterval(1000L, sendDataToBlynk);

  delay(1000);
  lcd.clear();
}

void loop() {
  Blynk.run();
  timer.run();
}

// Sends data to blynk
void sendDataToBlynk() {
  // Read Values from sensors
  int gasValue = analogRead(MQ2_ANALOG_PIN);
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int flameValue = analogRead(FLAME_SENSOR);
  // CHECK if sensors are giving valid data
  bool dhtError = isnan(temperature) || isnan(humidity);
  bool mq2Error = (gasValue == 0 || gasValue == 4095);

  if (dhtError || mq2Error) {
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);    // Keep buzzer silent during fault

    lcd.setCursor(0, 0);
    if (dhtError && mq2Error) lcd.print("CRITICAL ERROR! ");
    else if (dhtError)        lcd.print("DHT11 ERROR ");
    else                      lcd.print("MQ-2 ERROR ");
    return;
  }
  int gasPercentage = map(gasValue, 0, 4095, 0, 100);
  int flamePercentage = map(flameValue, 0, 4095, 0, 100);
  // Print sensors data to serial monitor
  Serial.print("Gas: "); Serial.print(gasValue);
  Serial.print(" | Temp: "); Serial.print(temperature);
  Serial.print("C | Hum: "); Serial.print(humidity);
  Serial.print(" | FlameIR: "); Serial.println( flameValue);

  // TRANSMISSION VIA BLYNK VIRTUAL CHANNELS
  Blynk.virtualWrite(V0, temperature);  // Matches V0 Temperature Widget
  Blynk.virtualWrite(V1, humidity);     // Matches V1 Humidity Widget
  Blynk.virtualWrite(V2, gasPercentage );     // Matches V2 Gas Gauge
  Blynk.virtualWrite(V3, flamePercentage);   // Matches V3 FlameIR Gauge

  // ANALYZE SENSOR READINGS AND TRIGGER ALERTS

  // CONDITION 1: FIRE ALERT (Instant Optical drop OR Combined Smoke + surrounding Heat)
  if (flameValue < threshold || (gasValue >= GAS_THRESHOLD && temperature >= HIGH_TEMP_THRESHOLD)) {
    triggerAlarm("!! FIRE ALERT !!","Evacuate the area");
    Blynk.virtualWrite(V4, 1);
  }

  // CONDITION 2: LPG GAS LEAK (High Gas concentration detected)
  else if (gasValue >= GAS_THRESHOLD && temperature < HIGH_TEMP_THRESHOLD) {
    triggerAlarm("! LPG GAS LEAK !", "Ventilate Room!");
    Blynk.virtualWrite(V5, 1);
  }

  // CONDITION 3: OVERHEATING (Abnormal Temperature rise without fire buildup)
  else if (gasValue < GAS_THRESHOLD && temperature >= OVERHEAT_THRESHOLD) {
    triggerAlarm("! OVERHEATING !", "Cooling Needed!");
    Blynk.virtualWrite(V6, 1);
  }

  // CONDITION 4: SAFE STATE
  else {
    clearAlarm(temperature, gasValue, humidity, flameValue);
    Blynk.virtualWrite(V4, 0);
    Blynk.virtualWrite(V5, 0);
    Blynk.virtualWrite(V6, 0);
  }
}

// --- HARDWARE CONTROL FUNCTION ---

void triggerAlarm(String line1, String line2) {
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN, LOW);     // LOW = SOUND ON

  lcd.setCursor(0, 0); lcd.print(line1);
  lcd.setCursor(0, 1); lcd.print(line2);
}

void clearAlarm(float temp, int gasValue, float hum, int flameValue) {
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, HIGH);    // HIGH = SOUND OFF
  digitalWrite(GREEN_LED_PIN, HIGH);

  // DISPLAY DATA ON LCD
  lcd.setCursor(0, 0);
  lcd.print("SAFE:   IR:" + String(flameValue) + "   ");

  lcd.setCursor(0, 1);
  lcd.print("T:" + String((int)temp) + "C G:" + String(gasValue) + " H:" + String((int)hum) + "  ");
}
