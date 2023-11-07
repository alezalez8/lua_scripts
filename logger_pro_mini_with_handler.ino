#include <SPI.h>
#include <SD.h>

 /*
 * -------------------------------- Pin level ----------------------
 * Level 1                              D7                           D21
 * Level 2                              D8                           D22
 * Level 3                              D9                           D23
 * Level 4                              D10                          D24
 * Level 5                              D11                          D25
 * Level 6                              D12                          D26
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
uint32_t timerLevel = 0;    // timer for LEVEL
#define T_PERIOD_LEVEL 5  // time of invoke data from sensor of level, mS. default = 3.7


uint32_t timeInvokeSensitive = 0;
uint32_t timeInvokeSensPeriod = 15;

uint32_t timeInvokeMode = 0;
uint32_t timeInvokeModePeriod = 50;
int  currentLevel = 1;



// ------------ level input --------------------------
static const uint8_t levelPin1 = 7;
static const uint8_t levelPin2 = 8;
static const uint8_t levelPin3 = 9;
static const uint8_t levelPin4 = 10;
static const uint8_t levelPin5 = 11;
static const uint8_t levelPin6 = 12;


// ------------ control output ------------------------
const int resetOutput = 13;
const int sensitiveOutput = 3; 
const int modeDetectorOutput = 2;


// ------------- sygnal from FC ------------------------
 const int modeDetectorInput = 2; // mode of detector
 const int sensitiveDetectorInput = 3; // sensitive of detector
 const int resetDetectorInput = 4;
 

 // ----------- for interrupt -----------------------
 volatile unsigned long pulseWidth1 = 0; // time of PPM of S11 on FC - MODE
 volatile unsigned long pulseWidth2 = 0; // time of PPM of S12 on FC - SENSITIVE



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


  Serial.begin(57600);
  
}

void loop() {  

if(millis() - timeInvokeSensitive >= timeInvokeSensPeriod){
  timeInvokeSensitive =millis();
  adjustSensitive(pulseWidth2);
}

if(millis() - timeInvokeMode >= timeInvokeModePeriod) {
  timeInvokeMode = millis();
  modeOfDetector(pulseWidth1);
}


  if (millis() - timerLevel >= T_PERIOD_LEVEL) {
    timerLevel = millis();
    getDataLevelDump();

    Serial.println(currentLevel);
  }



}

//========================================================================= подпрограммы ===================================

static void getDataLevel() {
if(digitalRead(levelPin1) == 6) {currentLevel = 6;}
  else if(digitalRead(levelPin5) == 1) {currentLevel = 5;}
  else if(digitalRead(levelPin4) == 1) {currentLevel = 4;}
  else if(digitalRead(levelPin3) == 1) {currentLevel = 3;}
  else if(digitalRead(levelPin2) == 1) {currentLevel = 2;}
  else if(digitalRead(levelPin1) == 1) {currentLevel = 1;}

  uint8_t level_value = currentLevel;
  printInt(level_value, 3); 
}


static void getDataLevelDump(){
  if(currentLevel >= 6) {
    currentLevel = 0;
  }
   currentLevel++;

    uint8_t level_value = currentLevel;
  //printInt(level_value, 3); 
  return currentLevel;

}

//-----------------------------------------------------------
int binary_to_int(char *binary_string) {
  int total = 0;
  while (*binary_string) {
    total *= 2;
    if (*binary_string++ == '1') total += 1;
  }
  return total;
}


//-------------------------------------------------------------
static void printInt(unsigned long val, int len) {
  char sz[32] = "*****************";
  sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len - 1] = ' ';
  //Serial.println(sz);  
  Serial.write(sz);    
 
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
}

//-------------------------------------------------------------
static void printFloat(float val, bool valid, int len, int prec) {
  if (!valid) {
    while (len-- > 1)
      Serial.print('*');        // **************************14.07.23***************************************
      Serial.print(' ');        // **************************14.07.23***************************************
   
      
  } else {
     Serial.print(val, prec);   // **************************14.07.23***************************************
  
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1);  // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3
                           : vi >= 10  ? 2
                                       : 1;
    for (int i = flen; i < len; ++i)
      Serial.print(' ');          // **************************14.07.23***************************************
    
  }
}


// --------------------------- new handlers ---------------------------------------
void ifReset(){
  boolean isRes = digitalRead(resetDetectorInput);
  digitalWrite(resetOutput, isRes);
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
}

// ------------------------------------ service ----------------------------------
