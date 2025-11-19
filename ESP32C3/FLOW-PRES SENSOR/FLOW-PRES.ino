#include <Adafruit_SH110X.h>

#define i2c_Address 0x3c
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define PIN_SENSOR1 0 //PRESSURE SENSOR1
#define PIN_SENSOR2 1 //PRESSURE SENSOR2
#define FLOW_INPUT_PIN 2    // PIN SENSOR (menerima pulse)
#define FLOW_OUTPUT_PIN 10  // PIN OUTPUT SIMULASI (menghasilkan pulse) : JUMPER KE PIN 2

int analogValue1, analogValue2;
float voltage1, voltage2;
float BAR1, BAR2;
float PSI1, PSI2;
float MPa1, MPa2;

volatile unsigned long pulseCount = 0;
unsigned long lastDisplayTime = 0;
unsigned long lastPulseTime = 0;

// 1 liter = 477 pulse
const float PULSES_PER_LITER = 477.0;
const unsigned long PULSE_INTERVAL = 2000;  // 5 detik

void IRAM_ATTR onPulse() {
  pulseCount++;
}

//SIMULASI PULSE GENERATOR
void generatePulse(int count, int delayMicros) {
  for (int i = 0; i < count; i++) {
    digitalWrite(FLOW_OUTPUT_PIN, HIGH);
    delayMicroseconds(delayMicros);
    digitalWrite(FLOW_OUTPUT_PIN, LOW);
    delayMicroseconds(delayMicros);
  }
}

void setup() {
  display.begin(i2c_Address, true);
  Serial.begin(115200);
  pinMode(FLOW_INPUT_PIN, INPUT_PULLUP);
  pinMode(FLOW_OUTPUT_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(FLOW_INPUT_PIN), onPulse, RISING);
}

void loop() {
  unsigned long currentMillis = millis();
  // === Hasilkan 477 pulse setiap 5 detik ===
  if (currentMillis - lastPulseTime >= PULSE_INTERVAL) {
    lastPulseTime = currentMillis;
    generatePulse(477, 500);  // 477 pulse, tiap pulse 500 mikrodetik
    Serial.println(">> Menghasilkan 477 pulse (≈ 1 L)");
  }

  float volumeLiter = pulseCount / PULSES_PER_LITER;
  // === Tampilkan hasil tiap 1 detik ===
  if (currentMillis - lastDisplayTime >= 1000) {
    lastDisplayTime = currentMillis;

    Serial.print("Pulse Terhitung: ");
    Serial.print(pulseCount);
    Serial.print("  | Volume: ");
    Serial.print(volumeLiter, 1);
    Serial.println(" L");
  }

  analogValue1 = analogRead(PIN_SENSOR1);
  voltage1 = (analogValue1 / 4095.0) * 5.0;
  BAR1 = ((voltage1 / 5.0) * 12.0);
  PSI1 = ((voltage1 / 5.0) * 174.0);
  MPa1 = PSI1 * 0.0068948;

  analogValue2 = analogRead(PIN_SENSOR2);
  voltage2 = (analogValue2 / 4095.0) * 5.0;
  BAR2 = ((voltage2 / 5.0) * 12.0);
  PSI2 = ((voltage2 / 5.0) * 174.0);
  MPa2 = PSI2 * 0.0068948;

  // Tampilkan di OLED
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  display.setCursor(0, 0);  //(KOLOM, BARIS)
  display.print("ANALOG :");
  display.setCursor(0, 10);
  display.print("VOLTAGE:");
  /*display.setCursor(0, 20);
  display.print("BAR    :");*/
  display.setCursor(0, 20);
  display.print("PSI    :");
  display.setCursor(0, 30);
  display.print("MPa    :");
  display.setCursor(0, 40);
  display.print("Pulse    :");
  display.setCursor(0, 50);
  display.print("VOLUME :");

  display.setCursor(54, 0);
  display.print(analogValue1);
  display.setCursor(54, 10);
  display.print(voltage1, 1);
  /*display.setCursor(54, 20);
  display.print(BAR1, 1);*/
  display.setCursor(54, 20);
  display.print(PSI1, 1);
  display.setCursor(54, 30);
  display.print(MPa1, 1);
  display.setCursor(54, 40);
  display.print(pulseCount, 1);
  display.setCursor(54, 50);
  display.print(volumeLiter, 1);
  display.print(" L");

  display.setCursor(84, 0);
  display.print("*");
  display.setCursor(84, 10);
  display.print("*");
  display.setCursor(84, 20);
  display.print("*");
  display.setCursor(84, 30);
  display.print("*");
  /*display.setCursor(84, 40);
  display.print("*");*/

  display.setCursor(96, 0);
  display.print(analogValue2);
  display.setCursor(96, 10);
  display.print(voltage2, 1);
  /*display.setCursor(96, 20);
  display.print(BAR2, 1);*/
  display.setCursor(96, 20);
  display.print(PSI2, 1);
  display.setCursor(96, 30);
  display.print(MPa2, 1);

  display.display();
  display.clearDisplay();
}
