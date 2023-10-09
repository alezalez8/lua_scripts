#include <Arduino.h>

uint8_t get_checksum(char *c, size_t size) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < size; i++) {
        checksum += c[i];
    }
    return checksum;
}

int mass1[] = {1, 3, 5, 7, 9, 11, 13, 15, 17, 20};
int mass2[] = {2, 4, 6, 8, 10, 12, 14, 16, 18, 20};
int mass3[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
int mass4[] = {11, 31, 51, 71, 91, 111, 131, 151, 171, 201};
int mass5[] = {20, 40, 60, 80, 100, 120, 140, 160, 180, 200};
int countLed = 0;
boolean stat = 0;
unsigned long currtime = 0;
int period = 100;


char buffer[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x31,
                 0x34, 0x2E, 0x37, 0x30, 0x08, 0x41, 0x6B,
                 0x3E, 0xC0, 0x41, 0x34, 0x59, 0x20, 0x00};

void setup() { 
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Serial.begin(9600);  
   }

void loop() {
    countLed += 50;
    // float range_val = (float)random(10000) / 100.0;
    // sprintf(buffer, "%05.02f", range_val);
    // buffer[5] = 0x01;
    // buffer[sizeof(buffer) - 1] = get_checksum(buffer, sizeof(buffer) - 1);
    // Serial.println(buffer);
    if(countLed >= 800) {
      stat = !stat;
      digitalWrite(13, stat);
      countLed = 0;
    } 
  //  if(millis() - currtime >= period ) {
  //   currtime = millis();
  //   Serial.println("-----------------------------------------------------------");
  //  }

    printArray(mass1);
    printArray(mass2);
    printArray(mass3);
    //printArray(mass4);
    //printArray(mass5);
    
}

void printArray(int myarray[]) {
  int currentSize = sizeof(myarray) / sizeof(myarray[0]);
  // Serial.print("size = ");
  // Serial.println(currentSize);
  // Serial.print("simply sizeof1 = ");
  // Serial.println(sizeof(myarray));
  // Serial.print("simply sizeof2 = ");
  // Serial.println(sizeof(myarray[0]));
  for(int i = 0; i < 10; i++) {
    Serial.print(myarray[i]);
    Serial.print(" ");
  }
  Serial.println();
  delay(10);
}