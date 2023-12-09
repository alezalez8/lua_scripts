#include <SPI.h>
#include <SD.h>

/*
                      
  * -------------------------------- Pin control input ---------------
 * OFF/ON/Reset input                                                D2
 * Light switch OFF/ON                                               D3

 *
 * -------------------------------- Pin out -------------------------
 * Power ON                                                          D5
 * Reset                                                             D6
 * Light ON/OFF                                                      D7

*/
uint32_t timerLevel = 0;  // timer for LEVEL
uint32_t timerLevelTest = 0;
#define T_PERIOD_LEVEL 10  // time of invoke data from sensor of level, mS.
#define T_PERIOD_LEVEL_TEST 200

uint32_t timeInvokeSensitive = 0;
uint32_t timeInvokeSensPeriod = 15;

uint32_t timeInvokeMode = 0;
uint32_t timeInvokeModePeriod = 50;
int currentLevel = 1;

const int arraySize = 20;
int dataArray[arraySize];
int dataIndex = 0;

bool ledOn = false;


// ------------ control output ------------------------
const int powerOn = 5;
const int detectorReset = 6;
const int lightOn = 7;
const int testReset = 13;

// ------------- sygnal from FC ------------------------
const int detectorOnReset = 2;  // power manager of detector
const int lightPower = 3;       // light on/off


// ----------- for interrupt -----------------------
volatile unsigned long pulseWidth1 = 0;  // time of PPM of S11 on FC - MODE
volatile unsigned long pulseWidth2 = 0;  // time of PPM of S12 on FC - SENSITIVE

void setup() {

  pinMode(detectorReset, OUTPUT);
  pinMode(powerOn, OUTPUT);
  pinMode(lightOn, OUTPUT);

  pinMode(detectorOnReset, INPUT_PULLUP);
  pinMode(lightPower, INPUT_PULLUP);

  pinMode(testReset, OUTPUT);


  attachInterrupt(digitalPinToInterrupt(detectorOnReset), handleInterrupt1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(lightPower), handleInterrupt2, CHANGE);
}

void loop() {

  // managerOfPowerReset
  // managerLight

  if (millis() - timeInvokeSensitive >= timeInvokeSensPeriod) {
    timeInvokeSensitive = millis();
    managerLight(pulseWidth2);
  }

  if (millis() - timeInvokeMode >= timeInvokeModePeriod) {
    timeInvokeMode = millis();
    managerOfPowerReset(pulseWidth1);
  }
}

//======================подпрограммы =================================

void handleInterrupt1() {

  static unsigned long startTime1 = 0;
  unsigned long currentTime1 = micros();

  if (digitalRead(detectorOnReset) == HIGH) {
    startTime1 = currentTime1;
  } else {
    pulseWidth1 = currentTime1 - startTime1;
  }
}
void handleInterrupt2() {

  static unsigned long startTime2 = 0;
  unsigned long currentTime2 = micros();

  if (digitalRead(lightPower) == HIGH) {
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

void managerOfPowerReset(unsigned long timePWM) {
  if (timePWM < 1000) {
    timePWM = 1000;
  }
  if (timePWM > 1950) {
    timePWM = 1950;
  }

  if (timePWM >= 950 && timePWM <= 1350) {
    digitalWrite(powerOn, 0);
    digitalWrite(detectorReset, 0);

  } else if (timePWM >= 1400 && timePWM <= 1700) {
    digitalWrite(powerOn, 1);
    digitalWrite(detectorReset, 0);
    digitalWrite(testReset, 0);

  } else if (timePWM > 1750) {
    digitalWrite(powerOn, 1);
    digitalWrite(detectorReset, 1);
    digitalWrite(testReset, 1);
  }
}




void managerLight(unsigned long timePWM) {

  if (timePWM < 1000) {
    timePWM = 1000;
  }
  if (timePWM > 1950) {
    timePWM = 1950;
  }
  if (timePWM >= 950 && timePWM <= 1350) {
    digitalWrite(lightOn, 0);

  } else if (timePWM >= 1400 && timePWM <= 1700) {
    digitalWrite(lightOn, 1);

  } else if (timePWM > 1750) {
    digitalWrite(lightOn, 1);
  }
}

// void blinkLed(bool onoff) {
//   digitalWrite(resetOutput, onoff);
// }
