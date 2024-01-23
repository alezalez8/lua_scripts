/*
*  -------------- jan 19 2024 ----- by Aleks Shunin
-------------------------------- LED -----------------------------
 * LED 3D Fix (blue)                                              D28

 * -------------------------------- Pin level ----------------------
 * Level 1                                                        D22  D33
 * Level 2                                                        D23  D34 
 * Level 3                                                        D24  D35
 * Level 4                                                        D25  D36
 * Level 5                                                        D26  D37
 * Level 6                                                        D27  D38
 *
 * -------------------------------- Pin control input ---------------
 * MODE detector (to S11 FC)                                      D2
 * Sensitive detector (to S12 FC)                                 D3
 * light IR ON/OFF (to S7 FC)                                     D21
 * RESET/ON/OFF detector (to S8 FC)                               D20

 * -------------------------------- Pin out -------------------------
 * Sensitive detector                                             D4
 * MODE detector                                                  D5
 * light IR ON/OFF                                                D29
 * reset detector                                                 D30
 * ON/OFF detector                                                D31
 * Test LED                                                       D13
 *
 * --------------------------------- Serials ------------------------
 * Serial1 (TX1 - 18, RX1 - 19 )   output to 3DR modem
 * Serial2 (TX1 - 16, RX1 - 17 )   to UART of FC
 */

// ----- level sensors -------
uint32_t timeLevel = 0;  // timer for LEVEL
uint32_t timeLevelPeriod = 19;

// ----- get GPS -------
uint32_t timerGPS = 0;          // timer for LEVEL
uint32_t timerGPSPeriod = 100;  // time of invoke data from sensor of level, mS. default = 100

// ----- sensitive -------
uint32_t timeInvokeSensitive = 0;
uint32_t timeInvokeSensPeriod = 20;

// ------- mode ----------
uint32_t timeInvokeMode = 0;
uint32_t timeInvokeModePeriod = 150;

// -------- power and reset --------
uint32_t timeInvokePower = 0;
uint32_t timeInvokePowerPeriod = 100;

// ------ light on\off -------------
uint32_t timeInvokeLight = 0;
uint32_t timeInvokeLightPeriod = 100;

uint32_t countOfInvokeLevel = 0;
uint32_t dumpCount = 0;
boolean ledOn = false;

const int arraySize = 5;
int levelData[arraySize] = { 1, 1, 2, 2, 3 };

static const uint32_t GPSBaud = 57600;  //

String messageGPS = "";

// ------------ level input --------------------------
// static const uint8_t levelPin1 = 22;
// static const uint8_t levelPin2 = 23;
// static const uint8_t levelPin3 = 24;
// static const uint8_t levelPin4 = 25;
// static const uint8_t levelPin5 = 26;
// static const uint8_t levelPin6 = 27;


// ------------ level input --------------------------
static const uint8_t levelPin1 = 33;
static const uint8_t levelPin2 = 34;
static const uint8_t levelPin3 = 35;
static const uint8_t levelPin4 = 36;
static const uint8_t levelPin5 = 37;
static const uint8_t levelPin6 = 38;

// ------------ led output ----------------------------
const int sateliteFix = 28;
const int testLED = 13;

// ------------ control output -------------------------
const int lightOnOff = 29;
const int resetOutput = 30;
const int powerOnOff = 31;
const int sensitiveOutput = 4;
const int modeDetectorOutput = 5;

// ------------- sygnal from FC --------------------
const int modeDetectorInput = 2;       // mode of detector
const int sensitiveDetectorInput = 3;  // sensitive of detector
const int resetDetectorInput = 20;     // reset_on_off
const int lightIR = 21;                // light on_off

// ----------- for interrupt -----------------------
volatile unsigned long pulseWidth1 = 1100;  // time of PPM of S11 on FC - MODE
volatile unsigned long pulseWidth2 = 1100;  // time of PPM of S12 on FC - SENSITIVE
volatile unsigned long pulseWidth3 = 1100;  // time of PPM of S11 on FC - reset_on_off
volatile unsigned long pulseWidth4 = 1100;  // time of PPM of S12 on FC - light on_off

void setup() {
  pinMode(lightOnOff, OUTPUT);
  pinMode(resetOutput, OUTPUT);
  pinMode(powerOnOff, OUTPUT);
  pinMode(sensitiveOutput, OUTPUT);
  pinMode(modeDetectorOutput, OUTPUT);
  pinMode(sateliteFix, OUTPUT);

  pinMode(testLED, OUTPUT);
  digitalWrite(testLED, ledOn);

  pinMode(modeDetectorInput, INPUT);
  pinMode(sensitiveDetectorInput, INPUT);
  pinMode(resetDetectorInput, INPUT);
  pinMode(lightIR, INPUT);

  pinMode(levelPin1, INPUT_PULLUP);
  pinMode(levelPin2, INPUT_PULLUP);
  pinMode(levelPin3, INPUT_PULLUP);
  pinMode(levelPin4, INPUT_PULLUP);
  pinMode(levelPin5, INPUT_PULLUP);
  pinMode(levelPin6, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(modeDetectorInput), handleInterrupt1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(sensitiveDetectorInput), handleInterrupt2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(resetDetectorInput), handleInterrupt3, CHANGE);
  attachInterrupt(digitalPinToInterrupt(lightIR), handleInterrupt4, CHANGE);

  Serial.begin(GPSBaud);
  Serial1.begin(GPSBaud);  // Connect to 3DR-modem
  Serial2.begin(GPSBaud);  // Connect to FC through the UART pins
}

void loop() {

  String mydata = "";
  // -------------------------------- get GPS and other from flight controller -------------
  if (millis() - timerGPS >= timerGPSPeriod) {
    timerGPS = millis();
    Serial2.write('5');
    while (Serial2.available()) {
      char dataGPS = Serial2.read();
      if (dataGPS == '@') {
        // Serial.println(" ------ STOP --------");
        break;
      }

      mydata += String(dataGPS);
      Serial.print(dataGPS);

      ledOn = !ledOn;
      digitalWrite(testLED, ledOn);
    }
    Serial1.print(mydata);

    String currentArray = dataString(levelData, arraySize);
    Serial.println(currentArray);
    Serial1.println(currentArray);
  }

  // --------------------------- get data from sensors every 19 ms -----------------------
  if (millis() - timeLevel >= timeLevelPeriod) {
    timeLevel = millis();
    getDataLevelNewD();  // getDataLevelNew();    ************* change for port D22 ... D28
  }

  // -------------------------------- mode of detector -----------------------------------
  if (millis() - timeInvokeMode >= timeInvokeModePeriod) {
    timeInvokeMode = millis();
    modeOfDetector(pulseWidth1);
  }

  // ------------------------------- sensitive -------------------------------------------
  if (millis() - timeInvokeSensitive >= timeInvokeSensPeriod) {
    timeInvokeSensitive = millis();
    adjustSensitive(pulseWidth2);
  }

  // -------------------------------- power and reset ------------------------------------
  if (millis() - timeInvokePower >= timeInvokePowerPeriod) {
    timeInvokePower = millis();
    managerOfPowerReset(pulseWidth3);
  }

  // -------------------------------- light on\off ---------------------------------------
  if (millis() - timeInvokeLight >= timeInvokeLightPeriod) {
    timeInvokeLight = millis();
    managerLight(pulseWidth4);
  }
}

//========================================================================= подпрограммы ===================================

// --------------------- POWER ON\OFF AND RESET ------------------------------------------
void managerOfPowerReset(unsigned long timePWM) {
  if (timePWM < 1000) {
    timePWM = 1000;
  }
  if (timePWM > 1950 && timePWM < 2000) {
    timePWM = 1950;
  }
  if (timePWM > 2000) {
    timePWM = 1000;
  }

  if (timePWM >= 950 && timePWM <= 1350) {
    digitalWrite(powerOnOff, 0);
    digitalWrite(resetOutput, 0);
  } else if (timePWM >= 1400 && timePWM <= 1700) {
    digitalWrite(powerOnOff, 1);
    digitalWrite(resetOutput, 0);
  } else if (timePWM > 1750) {
    digitalWrite(powerOnOff, 1);
    digitalWrite(resetOutput, 1);
  }
}

// --------------------- LIGHT ON\OFF ----------------------------------------------------
void managerLight(unsigned long timePWM) {

  if (timePWM < 1000) {
    timePWM = 1000;
  }
  if (timePWM > 1950) {
    timePWM = 1950;
  }

  if (timePWM > 1950 && timePWM < 2000) {
    timePWM = 1950;
  }
  if (timePWM > 2000) {
    timePWM = 1000;
  }
  if (timePWM >= 950 && timePWM <= 1350) {
    digitalWrite(lightOnOff, 0);
  } else if (timePWM >= 1400 && timePWM <= 2000) {
    digitalWrite(lightOnOff, 1);
  }
}

// ----------------------- MODE DETECTOR -------------------------------------------------
void modeOfDetector(unsigned long timePWM) {

  if (timePWM >= 950 && timePWM <= 1350) {
    analogWrite(modeDetectorOutput, 0);
  } else if (timePWM >= 1400 && timePWM <= 1700) {
    analogWrite(modeDetectorOutput, 78);
  } else if (timePWM > 1750) {
    analogWrite(modeDetectorOutput, 255);
  }
}

// ---------------------------- SENSITIVE ------------------------------------------------
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

// --------------------- interrupts  -----------------------------------------------------
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

void handleInterrupt3() {
  static unsigned long startTime3 = 0;
  unsigned long currentTime3 = micros();

  if (digitalRead(resetDetectorInput) == HIGH) {
    startTime3 = currentTime3;
  } else {
    pulseWidth3 = currentTime3 - startTime3;
  }
}

void handleInterrupt4() {
  static unsigned long startTime4 = 0;
  unsigned long currentTime4 = micros();

  if (digitalRead(lightIR) == HIGH) {
    startTime4 = currentTime4;
  } else {
    pulseWidth4 = currentTime4 - startTime4;
  }
}

// ---------------------------------- get data from sensors ------------------------------

static void getDataLevelNew() {
  int currentLevel = 1;
  for (int i = 27; i >= 22; i--) {
    if (digitalRead(i) == LOW) {  // change to HIGH for Кощей 5И
      currentLevel = i - 21;
      break;
    }
  }

  for (int i = 0; i < arraySize - 1; i++) {
    levelData[i] = levelData[i + 1];
  }
  levelData[arraySize - 1] = currentLevel;
}

// ---------------------------------- get data from sensors for D33 - D38 ----------------

static void getDataLevelNewD() {
  int currentLevel = 1;
  for (int i = 38; i >= 33; i--) {
    if (digitalRead(i) == LOW) {
      currentLevel = i - 32;
      break;
    }
  }

  for (int i = 0; i < arraySize - 1; i++) {
    levelData[i] = levelData[i + 1];
  }
  levelData[arraySize - 1] = currentLevel;
}

// ------------------------- print data from level ---------------------------------------
static String dataString(int arr[], int size) {
  String result = " ";

  for (int i = 0; i < size; i++) {

    // Convert each integer to a string and concatenate
    result += String(arr[i]);

    // Add a separator (optional, you can customize this)
    if (i < size - 1) {
      result += " ";
    }
  }

  return result;
}

//-------------------------------------------------------------
static int getNewLevel() {
  int currentLevel = 1;
  if (digitalRead(levelPin1) == 1) {
    currentLevel = 1;
  } else if (digitalRead(levelPin2) == 1) {
    currentLevel = 2;
  } else if (digitalRead(levelPin3) == 1) {
    currentLevel = 3;
  } else if (digitalRead(levelPin4) == 1) {
    currentLevel = 4;
  } else if (digitalRead(levelPin5) == 1) {
    currentLevel = 5;
  } else if (digitalRead(levelPin6) == 1) {
    currentLevel = 6;
  }

  return currentLevel;
}

// --------------------------- new handlers ----------------------------------------------

void printPWM(unsigned long timePWM) {
  Serial.println();
  String message = " Time of PWM =  ";
  Serial.print(message);
  Serial.println(timePWM);
  Serial.println();
}

// ------------------------------------ service ----------------------------------
