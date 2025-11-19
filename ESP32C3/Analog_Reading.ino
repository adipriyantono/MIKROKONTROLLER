// Pin analog input
const int analogPin = 0;  // gunakan pin A0 (bisa A0–A7 di Nano)

int analogValue = 0;  // variabel untuk menyimpan hasil ADC
float voltage = 0.0;  // variabel untuk konversi ke volt
float pressure = 0.0; 

void setup() {
  Serial.begin(115200);
}

void loop() {
  // Baca nilai ADC (0–1023)
  analogValue = analogRead(analogPin);

  // Konversi ke volt (V = (ADC / 1023) * 5V)
  voltage = (analogValue / 4095.0) * 5.0;
  pressure = ((voltage / 5.0) * 12.0);

  // Tampilkan hasil ke Serial Monitor
  Serial.print("ADC Value: ");
  Serial.print(analogValue );
  Serial.print("   Voltage: ");
  Serial.print(voltage, 2);  // tampilkan 2 angka di belakang koma
  Serial.print(" V");
  Serial.print("   Pressure: ");
  Serial.print(pressure, 2);  // tampilkan 2 angka di belakang koma
  Serial.println(" BAR");
}
