#include <SPI.h>
#include <SD.h>

/*
 * -------------------------------- Pin level ----------------------
 * Level 1                              D7                          
 * Level 2                              D8                          
 * Level 3                              D9                           
 * Level 4                              D10                          
 * Level 5                              D11                          
 * Level 6                              D12                         
 *
 * -------------------------------- Pin control input ---------------
 * Mode switch input                                                 D2
 * Sensitive input                                                   D3

 *
 * -------------------------------- Pin out -------------------------
 * Sensitive detector                                                D6
 * MODE detector                                                     D5
 * ON/OFF detector                                                   D10

*/
uint32_t timerLevel = 0;  // timer for LEVEL
uint32_t timerLevelTest = 0;
#define T_PERIOD_LEVEL 35  // time of invoke data from sensor of level, mS.
#define T_PERIOD_LEVEL_TEST 200

uint32_t timeInvokeSensitive = 0;
uint32_t timeInvokeSensPeriod = 15;

uint32_t timeInvokeMode = 0;
uint32_t timeInvokeModePeriod = 50;
int currentLevel = 1;

//const int arraySize = 20;
const int arraySize = 10;    // add 31.12.2023 instead line 36
int dataArray[arraySize];
int dataIndex = 0;

bool ledOn = false;

// ------------ level input --------------------------
static const uint8_t levelPin1 = 7;
static const uint8_t levelPin2 = 8;
static const uint8_t levelPin3 = 9;
static const uint8_t levelPin4 = 10;
static const uint8_t levelPin5 = 11;
static const uint8_t levelPin6 = 12;

// ------------ control output ------------------------
const int resetOutput = 13;
const int sensitiveOutput = 6;
const int modeDetectorOutput = 5;

// ------------- sygnal from FC ------------------------
const int modeDetectorInput = 2;       // mode of detector
const int sensitiveDetectorInput = 3;  // sensitive of detector
const int resetDetectorInput = 4;

// ----------- for interrupt -----------------------
volatile unsigned long pulseWidth1 = 0;  // time of PPM of S11 on FC - MODE
volatile unsigned long pulseWidth2 = 0;  // time of PPM of S12 on FC - SENSITIVE

void setup() {

  pinMode(resetOutput, OUTPUT);
  pinMode(resetDetectorInput, INPUT_PULLUP);
  pinMode(modeDetectorInput, INPUT);
  pinMode(sensitiveDetectorInput, INPUT);

  pinMode(levelPin1, INPUT);
  pinMode(levelPin2, INPUT);
  pinMode(levelPin3, INPUT);
  pinMode(levelPin4, INPUT);
  pinMode(levelPin5, INPUT);
  pinMode(levelPin6, INPUT);

  attachInterrupt(digitalPinToInterrupt(modeDetectorInput), handleInterrupt1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(sensitiveDetectorInput), handleInterrupt2, CHANGE);
 
  arrayInit();

  //randomSeed(analogRead(0));
  Serial.begin(57600);
}

void loop() {

  // invoke for handler PWM from FC

  if (millis() - timeInvokeSensitive >= timeInvokeSensPeriod) {
    timeInvokeSensitive = millis();
    adjustSensitive(pulseWidth2);
  }

  if (millis() - timeInvokeMode >= timeInvokeModePeriod) {
    timeInvokeMode = millis();
    modeOfDetector(pulseWidth1);
  }

  // -------------------------- ask FC about reading this data ------------
  if (Serial.available() > 0) {
    char readyToPass = Serial.read();
    if (readyToPass == '5') {
      ledOn = !ledOn;
      blinkLed(ledOn);
      sendArray();
      clearBuffer();
    }
  }

  // if (millis() - timerLevelTest >= T_PERIOD_LEVEL_TEST) {
  //   timerLevelTest = millis();
  //   clearBuffer();
  //   ledOn = !ledOn;
  //   blinkLed(ledOn);
  //   sendArray();
  // }


  // -------------------- read data every 35 ms ----------------------------
  if (millis() - timerLevel >= T_PERIOD_LEVEL) {
    timerLevel = millis();
    getDataLevelNew();
  }
}

//======================подпрограммы =================================

static void getDataLevel() {

  for (int i = 12; i >= 7; i--) {
    if (digitalRead(i) == HIGH) {
      for (int j = 0; j < arraySize - 1; j++) {
        dataArray[j] = dataArray[j + 1];
      }
      dataArray[arraySize - 1] = i - 6;
      break;
    }
  }
}

static void getDataLevelNew() {
  int currentLevel = 1;
  for (int i = 12; i >= 7; i--) {
    if (digitalRead(i) == HIGH) {
      currentLevel = i - 6;
      break;
    }
  }

  for (int i = 0; i < arraySize - 1; i++) {
    dataArray[i] = dataArray[i + 1];
  }
  dataArray[arraySize - 1] = currentLevel;
}

static void arrayInit() {
  for (int i = 0; i < arraySize; i++) {
    dataArray[i] = 1;
  }
}

static void getDataLevelDumpTwo() {
  int randomLevel = random(1, 7);

  for (int i = 12; i >= 7; i--) {
    if (randomLevel == i - 6) {
      for (int j = 0; j < arraySize - 1; j++) {
        dataArray[j] = dataArray[j + 1];
      }
      dataArray[arraySize - 1] = i - 6;
      break;
    }
  }
}

//---------------------------------- new dump 3 ---------------------------------
static void getDataLevelDumpThree(int countOfPacket) {
  int randomLevel = random(1, 7);
  int randomPosition = random(0, arraySize);
  for (int i = 0; i < arraySize; i++) {
    dataArray[i] = 1;
  }
  if (countOfPacket == 3) {
  }


  for (int i = 12; i >= 7; i--) {
    if (randomLevel == i - 6) {
      for (int j = 0; j < arraySize - 1; j++) {
        dataArray[j] = dataArray[j + 1];
      }
      dataArray[arraySize - 1] = i - 6;
      break;
    }
  }
}

//---------------------------------- end new dump 3 -----------------------------

void clearBuffer() {
  while (Serial.available()) {
    char cl = Serial.read();
  }
}

//-----------------------------------------------------------
void sendArray() {
  // Serial.println();

  for (int i = 0; i < arraySize; i++)

  {
    Serial.print(dataArray[i]);
    // Serial.print("  i=");
    // Serial.print(i);
    // Serial.print(";  ");
  }
  //Serial.println();
  // memset(dataArray, 1, sizeof(dataArray));
  memset(dataArray, 1, arraySize);
  //Serial.print();
}

// -------- new handlers -------------------------------------
void ifReset() {
  boolean isRes = digitalRead(resetDetectorInput);
  digitalWrite(resetOutput, isRes);
}
// -------- mode of minedetector -------------------------------
void handleInterrupt1() {

  static unsigned long startTime1 = 0;
  unsigned long currentTime1 = micros();

  if (digitalRead(modeDetectorInput) == HIGH) {
    startTime1 = currentTime1;
  } else {
    pulseWidth1 = currentTime1 - startTime1;
  }
}
void handleInterrupt2() {

  static unsigned long startTime2 = 0;
  unsigned long currentTime2 = micros();

  if (digitalRead(sensitiveDetectorInput) == HIGH) {
    startTime2 = currentTime2;
  } else {
    pulseWidth2 = currentTime2 - startTime2;
  }
}
void printPWM(unsigned long timePWM) {
  Serial.println();
  String message = " Time of PWM =  ";
  Serial.print(message);
  Serial.println(timePWM);
  Serial.println();
}
void modeOfDetector(unsigned long timePWM) {

  if (timePWM >= 950 && timePWM <= 1350) {
    analogWrite(modeDetectorOutput, 0);
  } else if (timePWM >= 1400 && timePWM <= 1700) {
    analogWrite(modeDetectorOutput, 78);
  } else if (timePWM > 1750) {
    analogWrite(modeDetectorOutput, 255);
  }
}
void adjustSensitive(unsigned long timePWM) {

  if (timePWM < 1000) {
    timePWM = 1000;
  }
  if (timePWM > 1950) {
    timePWM = 1950;
  }

  int valueOfSensitive = map(timePWM, 1000, 1950, 0, 255);
  analogWrite(sensitiveOutput, valueOfSensitive);
}

void blinkLed(bool onoff) {
  digitalWrite(resetOutput, onoff);
}
