#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

/*
 * светодиоды: красный - начало записи и конец                    pin А1(19)
 * синий - готовность ЖПС, кол-во спутников больше 6              pin 3
 * оранжевый, резерв                                              pin А0(18)
 * зеленый, резерв                                                pin А2(20)
 * кнопка 3, запись старт/стоп                                    pin 2
 * кнопка 1, резерв                                               pin 5
 * кнопка 2, резерв                                               pin 6
 * 
 * 
 * D9    - mode
 * D46   - sensit
 * 
 * 
 * =======================================================================
 * вход для остановки записи и сброса на флэшку                   pin A3(21)
 * при переходе пина в 0 - запись и сброс
 * =======================================================================
 * вход для старшего разряда датчика уровня, резерв               pin 7 
 * 
 * 
 */

/* -------------------------------- LED -----------------------------
 * LED POWER (green)                                              D27
 * LED 3D Fix (blue)                                              D28
 * LED RECORD (red)                                               D29
 * LED detector ON/OFF (green)                                    D30
 * LED RESET detector (red)                                       D31
 *
 * -------------------------------- Pin level ----------------------
 * Level 1                                                        D21
 * Level 2                                                        D22
 * Level 3                                                        D23
 * Level 4                                                        D24
 * Level 5                                                        D25
 * Level 6                                                        D26
 *
 * -------------------------------- Pin control input ---------------
 * MODE detector (to S11 FC)                                      D2
 * Sensitive detector (to S12 FC)                                 D3
 * RECORD logger ON/OFF (to S7 FC)                                D4
 * RESET detector (to S8 FC)                                      D5
 * reserve  (to S9 FC)                                            D5
 * ==================================================================
 * вход для остановки записи и сброса на флэшку                   D17
 * при переходе пина в 0 - запись и сброс
 * 
 * -------------------------------- Pin out -------------------------
 * Sensitive detector                                             D8 // D46
 * MODE detector                                                  D9
 * ON/OFF detector                                                D10  
 * 
 */



uint32_t timerGPS = 0;      // timer for GPS
uint32_t timerLevel = 0;    // timer for LEVEL
#define T_PERIOD_GPS 103    // time of invoke data from GPS, mS default = 103
#define T_PERIOD_LEVEL 3.7  // time of invoke data from sensor of level, mS. default = 3.7


uint32_t timeInvokeSensitive = 0;
uint32_t timeInvokeSensPeriod = 15;



static const uint32_t GPSBaud = 57600;    // 115200 9600 57600
static const uint32_t timeZoneOfset = 3;  // ofset for hour


// ------------ level input --------------------------
static const uint8_t levelPin1 = 21;
static const uint8_t levelPin2 = 22;
static const uint8_t levelPin3 = 23;
static const uint8_t levelPin4 = 24;
static const uint8_t levelPin5 = 25;
static const uint8_t levelPin6 = 26;

// ------------ led output ----------------------------
const int power = 27;
const int sateliteFix = 28;
const int record = 29;
const int powerDetector = 30;
const int resetDetectorLed = 31;

// ------------ control output -------------------------
const int resetOutput = 7;
const int sensitiveOutput = 8; // D46
const int modeDetectorOutput = 9;
const int testPWM = 46;

// ------------- sygnal from FC --------------------
 const int modeDetectorInput = 2; // mode of detector
 const int sensitiveDetectorInput = 3; // sensitive of detector
 const int resetDetectorInput = 5;
 const int recordInput = 4;

 // ----------- for interrupt -----------------------
 volatile unsigned long pulseWidth1 = 0; // time of PPM of S11 on FC - MODE
 volatile unsigned long pulseWidth2 = 0; // time of PPM of S12 on FC - SENSITIVE


const int stopRecordLowLevelPin = A3;  // button pin - stop record when power off
//const int buttonRecOnOff = 2;               // button pin - RECORD/STOP
//const int ledRecord = A1;              // LED pin - RECORD/STOP
//const int ledSat = 13;                  // LED pin - 3D Fix  
const int chipSelect = 53;              // CS line card reader *4*
bool buttonState = 0;
bool buttonStop = 0;
bool recordState = 0;
int countOfInvokeLevel = 0; 
//const int testFC = A2;  // checking sygnal from FC
String nameOfLogFile = "";

// The TinyGPSPlus object
TinyGPSPlus gps;
File myFile;



void setup() {
  pinMode(chipSelect, OUTPUT);
  //--------------------------- old
  //pinMode(buttonRecOnOff, INPUT);
  pinMode(stopRecordLowLevelPin, INPUT_PULLUP);
  //pinMode(ledRecord, OUTPUT);
  //pinMode(ledSat, OUTPUT);
  //pinMode(testFC, OUTPUT);
  //----------------------------- 

  pinMode(power, OUTPUT);
  digitalWrite(power, HIGH);

  pinMode(sateliteFix, OUTPUT);
  pinMode(record, OUTPUT);
  pinMode(powerDetector, OUTPUT);
  pinMode(resetOutput, OUTPUT);
  pinMode(resetDetectorLed, OUTPUT);
  pinMode(resetDetectorInput, INPUT_PULLUP);
  pinMode(recordInput, INPUT_PULLUP);
  pinMode(modeDetectorInput, INPUT);
  pinMode(sensitiveDetectorInput, INPUT);

  pinMode(testPWM, OUTPUT);

  pinMode(levelPin1, INPUT);
  pinMode(levelPin2, INPUT);
  pinMode(levelPin3, INPUT);
  pinMode(levelPin4, INPUT);
  pinMode(levelPin5, INPUT);
  pinMode(levelPin6, INPUT);


  attachInterrupt(digitalPinToInterrupt(modeDetectorInput), handleInterrupt1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(sensitiveDetectorInput), handleInterrupt2, CHANGE);


  Serial.begin(57600);
  //  while (!Serial) {
  //    ; // wait for serial port to connect. Needed for native USB port only
  //  }
  Serial1.begin(GPSBaud);  // Connect to GPS module through the UART pins
  Serial2.begin(GPSBaud);     
  

  // Working with SD card
  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println(" initialization failed!");
    while (1);
  }
 
  Serial.println(" initialization done.");

  //==========================================================================
}

void loop() {  
  SatteliteFix(); // check if satt > 5 -> ledSat ON 
 // ifReset();
//  modeOfDetector(pulseWidth1);

// uint32_t timeInvokeSensitive = 0;
// uint32_t timeInvokeSensPeriod = 15;

if(millis() - timeInvokeSensitive >= timeInvokeSensPeriod){
  timeInvokeSensitive =millis();
  adjustSensitive(pulseWidth2);
}

 // adjustSensitive(pulseWidth2);
//+++++++++++++++++++++

buttonState = digitalRead(recordInput); // check  REC from FC
//buttonState = HIGH;



  //------------------------- SD-card open-close -----------------------------------
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == 1) {
    if (recordState == 0) {
      // open the file. note that only one file can be open at a time
      if(gps.satellites.value() >= 5) {
        getFileName(gps.date, gps.time);
        myFile = SD.open(nameOfLogFile, FILE_WRITE);
      } else{
      myFile = SD.open("emptyGPS.txt", FILE_WRITE);
      }
     // myFile = SD.open(nameOfLogFile, FILE_WRITE);
      // if the file opened okay, write to it:
      if (myFile) {
        Serial.println("logs.txt is opened for writing");
      } else {
        Serial.println("error opening test.txt");
      }

      recordState = 1;
      digitalWrite(record, HIGH);
      delay(1500);
    } //---------------------------------------------------------
     /* else {
         // close the file:
         if (myFile) {
         myFile.close();
         Serial.println("logs.txt is closed");
         }

        recordState = 0;
        digitalWrite(ledRecord, LOW);
        }  */
    //-----------------------------------------------------------
  } else if(buttonState == 0 && recordState == 1){
 if (myFile) {
        myFile.close();
        Serial.println("logs.txt is closed");
      }

      recordState = 0;
      digitalWrite(record, LOW);  }
  //------------------------------ end SD-card -------------------------------

 // reset


  //====================================invoke gps and level===================

  if (millis() - timerGPS >= T_PERIOD_GPS) {
    timerGPS = millis();
        
    if (myFile) myFile.println();
    getGPS();
    ifReset();
    modeOfDetector(pulseWidth1);
    
  }


  if (millis() - timerLevel >= T_PERIOD_LEVEL) {
    timerLevel = millis();
    getDataLevel();
    //getNewLevel();
  }


  smartDelay(2);

  // if (millis() > 5000 && gps.charsProcessed() < 10) {
  //   Serial.println(F("No GPS data received: check wiring"));
  //   if (myFile) myFile.println(F("No GPS data received: check wiring"));
  // }
}



//========================================================================= подпрограммы ===================================
static void SatteliteFix() {
  if(gps.satellites.value() >= 5){
    digitalWrite(sateliteFix, HIGH);

  } else {
    digitalWrite(sateliteFix, LOW);
  }
}


static void getDataLevel() {
  //uint8_t level_value = getLevel(!digitalRead(levePin1), !digitalRead(levePin2), !digitalRead(levePin3));  
 // printInt(level_value, true, 3);  // value from level sensor
  uint8_t level_value = getNewLevel();



  printInt(level_value, true, 3); 
}



static void getGPS() {
  Serial.println();
  Serial2.println();
  bool iGPSValid = gps.satellites.isValid();
  printInt(gps.satellites.value(), iGPSValid, 5);
  printFloat(gps.location.lat(), iGPSValid, 13, 8);
  printFloat(gps.location.lng(), iGPSValid, 14, 8);
  printDateTime(gps.date, gps.time, gps.speed);  
}

//-------------------------------------------------------------
static int getLevel(bool value1, bool value2, bool value3) {
  String binaryString;
  binaryString += value3;
  binaryString += value2;
  binaryString += value1;
  int decimalValue = binary_to_int(binaryString.c_str());  // convert to decimal
  return decimalValue + 1;
}
//-------------------------------------------------------------
static int getNewLevel(){
  int currentLevel = 1;
  if(digitalRead(levelPin1) == 1) {currentLevel = 1;}
  else if(digitalRead(levelPin2) == 1) {currentLevel = 2;}
  else if(digitalRead(levelPin3) == 1) {currentLevel = 3;}
  else if(digitalRead(levelPin4) == 1) {currentLevel = 4;}
  else if(digitalRead(levelPin5) == 1) {currentLevel = 5;}
  else if(digitalRead(levelPin6) == 1) {currentLevel = 6;}
 
  return currentLevel;
}

int binary_to_int(char *binary_string) {
  int total = 0;
  while (*binary_string) {
    total *= 2;
    if (*binary_string++ == '1') total += 1;
  }
  return total;
}

//-------------------------------------------------------------
// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (Serial1.available())
      gps.encode(Serial1.read());
  } while (millis() - start < ms);
}

//-------------------------------------------------------------
static void printInt(unsigned long val, bool valid, int len) {
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len - 1] = ' ';
  Serial.print(sz);       // **************************14.07.23***************************************
  Serial2.print(sz);      //         000000000000000000000000000000000000000000000000000000000000
 
  if (myFile) myFile.print(sz);
  // smartDelay(0);
}

//-------------------------------------------------------------
static void printLevel(unsigned long val, bool valid, int len) {
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len - 1] = ' ';
  Serial.print(sz);
  Serial2.print(sz);           //         000000000000000000000000000000000000000000000000000000000000     // **************************14.07.23*************************************** 
 
  if (myFile) myFile.print(sz);
  // smartDelay(0);
}

//-------------------------------------------------------------
static void printFloat(float val, bool valid, int len, int prec) {
  if (!valid) {
    while (len-- > 1)
      Serial.print('*');        // **************************14.07.23***************************************
      Serial.print(' ');        // **************************14.07.23***************************************
      Serial2.print('*');      //         000000000000000000000000000000000000000000000000000000000000
      Serial2.print(' ');      //         000000000000000000000000000000000000000000000000000000000000
   
      
  } else {
     Serial.print(val, prec);   // **************************14.07.23***************************************
     Serial2.print(val, prec);      //         000000000000000000000000000000000000000000000000000000000000
  
    if (myFile) myFile.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1);  // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3
                           : vi >= 10  ? 2
                                       : 1;
    for (int i = flen; i < len; ++i)
      Serial.print(' ');          // **************************14.07.23***************************************
      Serial2.print(' ');      //         000000000000000000000000000000000000000000000000000000000000
    
    if (myFile) myFile.print(' ');
  }
  //  smartDelay(0);
}

//-------------------------------------------------------------
static void printDateTime(TinyGPSDate &d, TinyGPSTime &t, TinyGPSSpeed &s) {
  int currentHour = 0;  // correction current hour
  int dayOfset = 0;     // correction current day

  if (!d.isValid()) {
    Serial.print(F("**********   "));
  } else {
    currentHour = t.hour() + timeZoneOfset;  // correction current hour
    if (currentHour >= 24) {
      currentHour = currentHour - 24;
      dayOfset = 1;
    }

    char sz[32];
    sprintf(sz, "%02d/%02d/%02d   ", d.month(), d.day() + dayOfset, d.year());
    Serial.print(sz);       // **************************14.07.23***************************************
    Serial2.print(sz);      //         000000000000000000000000000000000000000000000000000000000000
    
    if (myFile) myFile.print(sz);
  }

  if (!t.isValid()) {
    Serial.print(F("********   "));
  } else {

    // adjustTime(offset * SECS_PER_HOUR);
    char sz[32];

    sprintf(sz, "%02d:%02d:%02d:%02d   ", currentHour, t.minute(), t.second(), t.centisecond());
    Serial.print(sz);       // **************************14.07.23***************************************
    Serial2.print(sz);      //         000000000000000000000000000000000000000000000000000000000000
    //softSerial.print(sz);
    if (myFile) myFile.print(sz);
  }
}
static void getFileName(TinyGPSDate &d, TinyGPSTime &t){
  int currentHour = 0;  // correction current hour
  int dayOfset = 0;     // correction current day
  currentHour = t.hour() + timeZoneOfset;  // correction current hour
    if (currentHour >= 24) {
      currentHour = currentHour - 24;
      dayOfset = 1;
    }

    char nameOfFile[32];
    //sprintf(nameOfFile, "%02d%02d%02d_%02d_%02d_%02d.txt", d.day(), d.month(), d.year(), currentHour, t.minute(), t.second());   
    sprintf(nameOfFile, "%02d%02d%02d.txt", currentHour, t.minute(), t.second());   
    nameOfLogFile = String(nameOfFile);      

}

// --------------------------- new handlers ---------------------------------------
void ifReset(){
  boolean isRes = digitalRead(resetDetectorInput);
  digitalWrite(resetOutput, isRes);
  digitalWrite(resetDetectorLed, isRes);  
}

// --------------------- mode of minedetector --------------------------------------
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

void printPWM(unsigned long timePWM){
  Serial.println();
  String message = " Time of PWM =  ";
  Serial.print(message);
  Serial.println(timePWM);
  Serial.println();
}

void modeOfDetector(unsigned long timePWM){

   if (timePWM >= 950 && timePWM <= 1350) {    
     analogWrite(modeDetectorOutput, 0);      
    } else if (timePWM >= 1400 && timePWM <= 1700) {      
      analogWrite(modeDetectorOutput, 78);
    } else if( timePWM > 1750) {    
      analogWrite(modeDetectorOutput, 255);
    }
}

void adjustSensitive(unsigned long timePWM){
  
  if(timePWM < 1000) {
     timePWM = 1000;
  } if(timePWM > 1950) {
    timePWM = 1950;
  }

  int valueOfSensitive = map(timePWM, 1000, 1950, 0, 255);
  analogWrite(sensitiveOutput, valueOfSensitive);
  analogWrite(testPWM, valueOfSensitive);  
}

// ------------------------------------ service ----------------------------------
