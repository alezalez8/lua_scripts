#define TX_RATE_HZ 10
const uint8_t HEADER_BYTE = 0xFF;

uint32_t last_update_ms = 0;
uint8_t led_state = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  uint32_t now = millis();
  if (now - last_update_ms > 1000 / TX_RATE_HZ) {
    uint16_t sensorValue = random(0, 4096);
    uint8_t checksum = highByte(sensorValue) ^ lowByte(sensorValue);
    Serial.write(HEADER_BYTE);
    Serial.write(highByte(sensorValue));
    Serial.write(lowByte(sensorValue));
    Serial.write(checksum);
    led_state ^= 1;
    digitalWrite(LED_BUILTIN, led_state);
    last_update_ms = now;
  }
}