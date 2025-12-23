#include <hmi2.h>

Hmi2 hmi2;

// ================= PIN CONFIG =================
#define ENCODER_CLK 9
#define ENCODER_DT 10
#define SENSOR_BARANG 1
#define REJECT_PIN 8

// ================= PARAMETER =================
#define REJECT_STEP 5
#define REJECT_TIME_MS 100
#define MAX_QUEUE 10

// ================= STATE =================
enum SystemState {
  IDLE,
  COUNTING,
  REJECTING
};

SystemState currentState = IDLE;

// ================= ENCODER =================
volatile long encoderValue = 0;
int lastCLKState;

// ================= SENSOR =================
bool lastSensorState = HIGH;

// ================= QUEUE TARGET =================
long rejectTargetQueue[MAX_QUEUE];
int queueHead = 0;
int queueTail = 0;
int barangQueue = 0;

// ================= COUNTER =================
unsigned long rejectNG = 0;

// ================= TIMER =================
unsigned long rejectStartTime = 0;

// ================= INTERRUPT =================
void IRAM_ATTR readEncoder() {
  int clk = digitalRead(ENCODER_CLK);
  if (clk != lastCLKState) {
    encoderValue++;  // encoder hanya MAJU
  }
  lastCLKState = clk;
}

void KONEKSI_BLUETOOTH() {
  Serial.begin(9600);
  hmi2.init(Serial);
}


// ================= SETUP =================
void setup() {
  KONEKSI_BLUETOOTH();

  hmi2.setDisplayID(1);

  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(SENSOR_BARANG, INPUT_PULLUP);
  pinMode(REJECT_PIN, OUTPUT);

  digitalWrite(REJECT_PIN, LOW);
  hmi2.setBoolean(0, 0, LOW);

  lastCLKState = digitalRead(ENCODER_CLK);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), readEncoder, CHANGE);
}

// ================= LOOP =================
void loop() {

  // ===== SAFE ENCODER COPY =====
  long encoderCopy;
  noInterrupts();
  encoderCopy = encoderValue;
  interrupts();

  // ===== SENSOR EDGE DETECTION =====
  bool sensorNow = digitalRead(SENSOR_BARANG);

  if (lastSensorState == HIGH && sensorNow == LOW) {

    if (barangQueue < MAX_QUEUE) {
      rejectTargetQueue[queueTail] = encoderCopy + REJECT_STEP;
      queueTail = (queueTail + 1) % MAX_QUEUE;
      barangQueue++;

      // Serial.print("Barang Masuk | Target = ");
      // Serial.println(encoderCopy + REJECT_STEP);
    }
  }
  lastSensorState = sensorNow;

  switch (currentState) {

    // ================= IDLE =================
    case IDLE:
      if (barangQueue > 0) {
        currentState = COUNTING;
      }
      break;

    // ================= COUNTING =================
    case COUNTING:
      if (encoderCopy >= rejectTargetQueue[queueHead]) {
        digitalWrite(REJECT_PIN, HIGH);
        hmi2.setBoolean(0, 0, HIGH);
        rejectStartTime = millis();
        currentState = REJECTING;
        //Serial.println("REJECT ON");
      }
      break;

    // ================= REJECTING =================
    case REJECTING:
      if (millis() - rejectStartTime >= REJECT_TIME_MS) {
        digitalWrite(REJECT_PIN, LOW);
        hmi2.setBoolean(0, 0, LOW);

        queueHead = (queueHead + 1) % MAX_QUEUE;
        barangQueue--;
        rejectNG++;

        //Serial.println("REJECT DONE");

        currentState = (barangQueue > 0) ? COUNTING : IDLE;
      }
      break;
  }

  // // ================= MONITOR =================
  // static unsigned long lastPrint = 0;
  // if (millis() - lastPrint > 500) {
  //   Serial.println("----- STATUS -----");
  //   Serial.print("Encoder    : ");
  //   Serial.println(encoderCopy);
  //   Serial.print("Queue      : ");
  //   Serial.println(barangQueue);
  //   Serial.print("Reject NG  : ");
  //   Serial.println(rejectNG);
  //   Serial.println("------------------");
  //   lastPrint = millis();
  // }

  hmi2.setCursor(0, 0);
  hmi2.print("ENCODER :");
  hmi2.print(encoderCopy);
  hmi2.setCursor(0, 1);
  hmi2.print("ANTRIAN :");
  hmi2.print(barangQueue);
  hmi2.update();
}
