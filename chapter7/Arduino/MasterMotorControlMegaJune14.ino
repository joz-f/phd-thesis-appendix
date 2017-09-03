// RAT.SYSTEMS research 2016-2017
// Matthew Jarvis ( matt@mattjarvis.co.uk )
// Julie Freeman
//
// Version: June 
//
// Queen Mary University of London
// Media and Arts Technology Programme
// 

/* 
 Code to operate soft-sobotics according to serial commands, with display and custom control from raspberry pi processing sketch.

The MAX MSP patch has slightly different mapings of the motors to here. 

Arduino => Labels on sculptures (may be differnent to labels in MAX/MSP)
s1m1 => s1m2 
s1m2 => s1m1
s1m3 => s1m3

s2m1 => s2m2
s2m2 => s2m3
s2m3 => s2m1

s3m1 => s3m3
s3m2 => s3m2
s3m3 => s3m1

More details at: https://docs.google.com/document/d/1atB2FiuFpkRDwOGp1yHecGeNe1I8O84x2OAnGpjL3_A/edit?usp=sharing
 
*/


#include <Arduino.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>             // display
#include <Adafruit_SSD1306.h>         // display
#define OLED_RESET 4                  // display  
Adafruit_SSD1306 display(OLED_RESET); // display 
#define BUTTONDELAY

boolean DEBUG_ON = 0;  // used to find bugs!
String DEBUG_PRE = "** ";

boolean active = true;

// =============================================================== HARDWARE VALUES 


// motor & switch pins

//  -------------------------------------------------------------- PINS --- Sculpture 1
int motor1PLSPin = 2;  // speed control through PWM
int motor1DIRPin = 22;  // high to change direction 
int motor1ENAPin = 34;  // high to release current from motor (stops over heating and motor damage)
int s1m1MaxButton = 51;
int s1m1MinButton = 53;

int motor2PLSPin = 3; 
int motor2DIRPin = 26; 
int motor2ENAPin = 36; 
int s1m2MaxButton = 47;
int s1m2MinButton = 49;

int motor3PLSPin = 4; 
int motor3DIRPin = 24; 
int motor3ENAPin = 38;
int s1m3MaxButton = 43;
int s1m3MinButton = 45;

//  -------------------------------------------------------------- PINS --- Sculpture 2
int motor4PLSPin = 5; 
int motor4DIRPin = 28;  
int motor4ENAPin = 35; 
int s2m1MaxButton = 50;
int s2m1MinButton = 52;

int motor5PLSPin = 6;  
int motor5DIRPin = 30;  
int motor5ENAPin = 37; 
int s2m2MaxButton = 46;
int s2m2MinButton = 48;

int motor6PLSPin = 7;  
int motor6DIRPin = 32;  
int motor6ENAPin = 39; 
int s2m3MaxButton = 42;
int s2m3MinButton = 44;

//  -------------------------------------------------------------- PINS --- Sculpture 3
int motor7PLSPin = 9;
int motor7DIRPin = 25;
int motor7ENAPin = 31; 
int s3m1MaxButton = 14; 
int s3m1MinButton = 19; 

int motor8PLSPin = 8; 
int motor8DIRPin = 23;  
int motor8ENAPin = 29; 
int s3m2MaxButton = 15;
int s3m2MinButton = 18;

int motor9PLSPin = 10; 
int motor9DIRPin = 27;  
int motor9ENAPin = 33;  
int s3m3MaxButton = 16; 
int s3m3MinButton = 17;

long loopcount = 0;
// -




// ===================================================================  PROGRAM VALUES 


// ===================================================================  TIMERS

unsigned long previousMillisTimer1 = 0; unsigned long trackValue1 = 1; // speed 1
unsigned long previousMillisTimer2 = 0; const long intervalTimer2 = 2; boolean timer2 = false; unsigned long  trackValue2 = 1; // speed 2 = 1.41 x
unsigned long previousMillisTimer3 = 0; const long intervalTimer3 = 4; boolean timer3 = false; unsigned long  trackValue3 = 1; // speed 3 = 2.7 x
unsigned long previousMillisTimer4 = 0; const long intervalTimer4 = 8; boolean timer4 = false; unsigned long  trackValue4 = 1; // speed 4 = 5.13 x

unsigned long previousMillisTimer5 = 0; const long intervalTimer5 = 10000; boolean timer5 = false; // Display & serial update timer

boolean doingCalibrationStep1 = false;
boolean doingCalibrationStep2 = false;
boolean doingCalibrationStep3 = false;
int calibrationErrorCount;

// Tracking
boolean targetSearchActive = false;
boolean targetSearchActiveS1 = true;
boolean targetSearchActiveS2 = true;
boolean targetSearchActiveS3 = true;


// set to true to test the switches (IE no motor movement)
boolean testButtons = false;


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Sculpture 1
boolean allStopSculture1 = true; 
boolean s1m1calibrated = false;
int s1m1min = -1;       // min of syringe 1
unsigned int s1m1max = -1;       // max of syringe 1
boolean s1m2calibrated = false;
int s1m2min = -1;       // min of syringe 2
unsigned int s1m2max = -1;       // max of syringe 2
boolean s1m3calibrated = false;
int s1m3min = -1;       // min of syringe 3
unsigned int s1m3max = -1;       // max of syringe 3


// tracking
unsigned int s1m1_locationValue = 1;
unsigned int s1m2_locationValue = 1;
unsigned int s1m3_locationValue = 1;
boolean s1atMax = false;
boolean s1atMin = false;


boolean s1motorOn1;  // motor activated
boolean s1motorDir1; // true forward, false reverse
int s1motorSpeed1;   // PWM for motor1
boolean s1m1swMax; // bottom switch state
boolean s1m1swMin; // top switch state
unsigned int s1m1target; // target value for moving to a position
boolean s1m1TSA = false; // target search activation flag

boolean s1motorOn2;  // motor activated
boolean s1motorDir2; // true forward, false reverse
int s1motorSpeed2;   // PWM for motor1
boolean s1m2swMax; // bottom switch state
boolean s1m2swMin; // top switch state
unsigned int s1m2target; // target value for moving to a position
boolean s1m2TSA = false; // target search activation flag


boolean s1motorOn3;  // motor activated
boolean s1motorDir3; // true forward, false reverse
int s1motorSpeed3;   // PWM for motor1
boolean s1m3swMax; // bottom switch state
boolean s1m3swMin; // top switch state
unsigned int s1m3target; // target value for moving to a position
boolean s1m3TSA = false; // target search activation flag


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Sculpture 2
boolean allStopSculture2 = true; 
boolean s2m1calibrated = false;
int s2m1min = -1;       // min of syringe 1
unsigned int s2m1max = -1;       // max of syringe 1
boolean s2m2calibrated = false;
int s2m2min = -1;       // min of syringe 2
unsigned int s2m2max = -1;       // max of syringe 2
boolean s2m3calibrated = false;
int s2m3min = -1;       // min of syringe 3
unsigned int s2m3max = -1;       // max of syringe 3

// tracking
unsigned int s2m1_locationValue = 1;
unsigned int s2m2_locationValue = 1;
unsigned int s2m3_locationValue = 1;
boolean s2atMax = false;
boolean s2atMin = false;

boolean s2motorOn1;  // motor activated
boolean s2motorDir1; // true forward, false reverse
int s2motorSpeed1;   // PWM for motor1
boolean s2m1swMax; // bottom switch state
boolean s2m1swMin; // top switch state
unsigned int s2m1target; // target value for moving to a position
boolean s2m1TSA = false; // target search activation flag

boolean s2motorOn2;  // motor activated
boolean s2motorDir2; // true forward, false reverse
int s2motorSpeed2;   // PWM for motor1
boolean s2m2swMax; // bottom switch state
boolean s2m2swMin; // top switch state
unsigned int s2m2target; // target value for moving to a position
boolean s2m2TSA = false; // target search activation flag

boolean s2motorOn3;  // motor activated
boolean s2motorDir3; // true forward, false reverse
int s2motorSpeed3;   // PWM for motor1
boolean s2m3swMax; // bottom switch state
boolean s2m3swMin; // top switch state
unsigned int s2m3target; // target value for moving to a position
boolean s2m3TSA = false; // target search activation flag


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Sculpture 3
boolean allStopSculture3 = true; 
boolean s3m1calibrated = false;
int s3m1min = -1;       // min of syringe 1
unsigned int s3m1max = -1;       // max of syringe 1
boolean s3m2calibrated = false;
int s3m2min = -1;       // min of syringe 2
unsigned int s3m2max = -1;       // max of syringe 2
boolean s3m3calibrated = false;
int s3m3min = -1;       // min of syringe 3
unsigned int s3m3max = -1;       // max of syringe 3

// tracking
unsigned int s3m1_locationValue = 1;
unsigned int s3m2_locationValue = 1;
unsigned int  s3m3_locationValue = 1;
boolean s3atMax = false;
boolean s3atMin = false;

boolean s3motorOn1;  // motor activated
boolean s3motorDir1; // true forward, false reverse
int s3motorSpeed1;   // PWM for motor1
boolean s3m1swMax; // bottom switch state
boolean s3m1swMin; // top switch state
unsigned int s3m1target; // target value for moving to a position
boolean s3m1TSA = false; // target search activation flag

boolean s3motorOn2;  // motor activated
boolean s3motorDir2; // true forward, false reverse
int s3motorSpeed2;   // PWM for motor1
boolean s3m2swMax; // bottom switch state
boolean s3m2swMin; // top switch state
unsigned int s3m2target; // target value for moving to a position
boolean s3m2TSA = false; // target search activation flag

boolean s3motorOn3;  // motor activated
boolean s3motorDir3; // true forward, false reverse
int s3motorSpeed3;   // PWM for motor1
boolean s3m3swMax; // bottom switch state
boolean s3m3swMin; // top switch state
unsigned int s3m3target; // target value for moving to a position
boolean s3m3TSA = false; // target search activation flag

boolean allCalibrated = false;
boolean ok = true; 
// -
 int speed1, speed2, speed3, speed4;

long buttonLastChecked = 0; // variable to limit the button getting checked every cycle




// DISPLAY stuff

#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

int line= 0;



// Serial stuff
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
String switchDisplayTxt;

    // Convert from String Object to String for seperation processing
 //   char inFromMAX[] = "111111111 222222222 333333333 4444444444 5555555555 6666666666 777777777 888888888";
    char buf[64];
    int targetVals[10] = { };

void setup()
{
 Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println("** Starting up...");
  Serial.println("** ...........................................................................");
  Serial.println("**  ");
  
  Serial.print("** Serial Commands:\n"
                  "**  c - sets calibration of motors and returns high values\n"
                  "**  a - active (on/off)\n"
                  "**  d - switches serial feedback on or off for debugging\n"
                  "**  z - sets the target positions (S1M1 S1M2 S1M3 S2M1 S2M2 S2M3 S3M1 S3M2 S3M3)\n" 
                  "**      example: 'z 16242 16483 16821 18773 19351 19733 16210 15366 14878' \n" 
                  "**  m - sets all motors to the middle (when calibrated)\n"
                  "**  e - sets all motors to a random value (wthen calibrated)\n"
                  "** Continously run with:/n"
                  "**  s - {1-3} - Select sculpture\n"
                  "**  m - {1-3} - Select motor\n"
                  "**  # - {0/1/2} - Stop/start/reverse\n"
                  "**  # - {1-4} - Speed setting\n"
                  "** e.g. 's3m111' to move S3 M1 to max at full speed (until switch).\n"
                  "** Special commands:\n"
                  "**  1 - switch testing mode DISABLED\n"
                  "**  r - STOP ALL and RESET ARDUINO\n"
                  "\n\n\n"
                  );
  
  delay(10);  

  // Change the PWM frequencies
  // https://playground.arduino.cc/Main/TimerPWMCheatsheet
  // http://forum.arduino.cc/index.php?topic=16612#msg121031
  // http://forum.arduino.cc/index.php/topic,72092.0.html
  // http://usethearduino.blogspot.com/2008/11/changing-pwm-frequency-on-arduino.html 
  
  TCCR0B = TCCR0B & B11111000 | B00000010 ; //& 0b11111000 | 0x01; //
  TCCR2B = TCCR2B & B11111000 | B00000010 ; //& 0b11111000 | 0x01; //
  TCCR3B = TCCR3B & B11111000 | B00000010 ; //& 0b11111000 | 0x01; //
  TCCR4B = TCCR4B & B11111000 | B00000010 ; //& 0b11111000 | 0x01; //
  
  // reserve 200 bytes for the serialInputString:
  inputString.reserve(255);
  inputString = "";
  stringComplete = true;
 
  // display setup
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
 // Clear the buffer.
  display.clearDisplay();

  // text display welcome message
  display.setCursor(0,0);
  display.setTextColor(BLACK, WHITE); // 'inverted' text
  display.setTextSize(1);
  display.println("MJ CREATIVE TECH 2017");
  display.setTextColor(WHITE);
  display.println("RAT.systems J.FREEMAN");
  display.setTextSize(2);
  display.println("MOTOR   ");
  display.println("  CONTROL");
  display.println("    SYSTEM");
  display.display();
  
  delay(8000);
  display.clearDisplay();
  display.setTextSize(0);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("NOT CALIBRATED.");
  display.println("SEND 'c' to calibrate");
  display.println("z sets the target pos");
  display.println("s{1-3} Sel sculpture");
  display.println("m{1-3} Sel motor");
  display.println("{0/1/2} Stp/start/rev");
  display.println("{1-4} Speed");
  display.println("      e.g. 's3m111'");

  
  display.display(); 
  

// set up pins
  pinMode(motor1PLSPin, OUTPUT); 
  pinMode(motor1DIRPin, OUTPUT);
  pinMode(motor1ENAPin, OUTPUT);
  pinMode(motor2PLSPin, OUTPUT); 
  pinMode(motor2DIRPin, OUTPUT);
  pinMode(motor2ENAPin, OUTPUT);
  pinMode(motor3PLSPin, OUTPUT); 
  pinMode(motor3DIRPin, OUTPUT);
  pinMode(motor3ENAPin, OUTPUT); 
  pinMode(motor4PLSPin, OUTPUT); 
  pinMode(motor4DIRPin, OUTPUT);
  pinMode(motor4ENAPin, OUTPUT);
  pinMode(motor5PLSPin, OUTPUT); 
  pinMode(motor5DIRPin, OUTPUT);
  pinMode(motor5ENAPin, OUTPUT);
  pinMode(motor6PLSPin, OUTPUT); 
  pinMode(motor6DIRPin, OUTPUT);
  pinMode(motor6ENAPin, OUTPUT); 
  pinMode(motor7PLSPin, OUTPUT); 
  pinMode(motor7DIRPin, OUTPUT);
  pinMode(motor7ENAPin, OUTPUT);
  pinMode(motor8PLSPin, OUTPUT); 
  pinMode(motor8DIRPin, OUTPUT);
  pinMode(motor8ENAPin, OUTPUT);
  pinMode(motor9PLSPin, OUTPUT); 
  pinMode(motor9DIRPin, OUTPUT);
  pinMode(motor9ENAPin, OUTPUT);   
  
  pinMode(s1m1MaxButton, INPUT); 
  pinMode(s1m1MinButton, INPUT);  

  pinMode(s1m2MaxButton, INPUT); 
  pinMode(s1m2MinButton, INPUT);  

  pinMode(s1m3MaxButton, INPUT); 
  pinMode(s1m3MinButton, INPUT);  

  pinMode(s2m1MaxButton, INPUT); 
  pinMode(s2m1MinButton, INPUT);  

  pinMode(s2m2MaxButton, INPUT); 
  pinMode(s2m2MinButton, INPUT);  

  pinMode(s2m3MaxButton, INPUT); 
  pinMode(s2m3MinButton, INPUT);  

  pinMode(s3m1MaxButton, INPUT); 
  pinMode(s3m1MinButton, INPUT);  

  pinMode(s3m2MaxButton, INPUT); 
  pinMode(s3m2MinButton, INPUT);  

  pinMode(s3m3MaxButton, INPUT); 
  pinMode(s3m3MinButton, INPUT);  
  
  // Reset ports
  stopall();

}



// MAIN LOOP
void loop() {
  loopcount++;


  if(testButtons) { buttonTesting();  } // break out of the normal loop to configure the button IDs (only used for initial wiring setup)

if(active) {
 
 //  main loop
 // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Sculpture 1
 // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Motor 1
 if(s1motorOn1) {
  // Serial.println("s1motorOn1 ON");
 digitalWrite(motor1ENAPin, HIGH);
  switch(s1motorSpeed1) {
    case 0:
       digitalWrite(motor1PLSPin, LOW); 
       digitalWrite(motor1PLSPin, HIGH);
       break;
    case 1: 
       //if (timer1) {
          digitalWrite(motor1PLSPin, LOW); 
       //   }
       // if (!timer1) {
          digitalWrite(motor1PLSPin, HIGH);
        //  }
        if (s1motorDir1) { s1m1_locationValue = s1m1_locationValue - trackValue1; } else { s1m1_locationValue = s1m1_locationValue + trackValue1; } 
        break;
     case 2: 
       if (timer2) {
          digitalWrite(motor1PLSPin, LOW); 
          }
        if (!timer2) {
          digitalWrite(motor1PLSPin, HIGH);
          }
        if (s1motorDir1) { s1m1_locationValue = s1m1_locationValue - trackValue2; } else { s1m1_locationValue = s1m1_locationValue + trackValue2; } 
        break;
      case 3: 
        if (timer3) {
          digitalWrite(motor1PLSPin, LOW); 
          }
        if (!timer3) {
          digitalWrite(motor1PLSPin, HIGH);
          }
       if (s1motorDir1) { s1m1_locationValue = s1m1_locationValue - trackValue3; } else { s1m1_locationValue = s1m1_locationValue + trackValue3; } 
       break;
      case 4:
      if (timer4) {
        digitalWrite(motor1PLSPin, LOW); 
      }
      if (!timer4) {
        digitalWrite(motor1PLSPin, HIGH);
      }
      if (s1motorDir1) { s1m1_locationValue = s1m1_locationValue - trackValue4; } else { s1m1_locationValue = s1m1_locationValue + trackValue4; } 
       break;
    } // end switch/case
  
 } else { // release the brakes
 digitalWrite(motor1PLSPin, HIGH);
 digitalWrite(motor1ENAPin, LOW);
 }
 // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Sculpture 1
 // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Motor 2
  if(s1motorOn2) {
  // Serial.println("s1motorOn2 ON");
  digitalWrite(motor2ENAPin, HIGH);
  switch(s1motorSpeed2) {
    case 0: 
       digitalWrite(motor2PLSPin, LOW); 
       digitalWrite(motor2PLSPin, HIGH);
     break;
     case 1:
      // if (timer1) {
          digitalWrite(motor2PLSPin, LOW);
      //   }   
      // if (!timer1) {
     digitalWrite(motor2PLSPin, HIGH);
      //   }
     if (s1motorDir2) { s1m2_locationValue = s1m2_locationValue - trackValue1; } else { s1m2_locationValue = s1m2_locationValue + trackValue1; } 
     break;
     case 2:
       if (timer2) {
          digitalWrite(motor2PLSPin, LOW);
         }   
       if (!timer2) {
     digitalWrite(motor2PLSPin, HIGH);
         }
     if (s1motorDir2) { s1m2_locationValue = s1m2_locationValue - trackValue2; } else { s1m2_locationValue = s1m2_locationValue + trackValue2; } 
     break;
     case 3:
       if (timer3) {
          digitalWrite(motor2PLSPin, LOW);
         }   
       if (!timer3) {
     digitalWrite(motor2PLSPin, HIGH);
         }
     if (s1motorDir2) { s1m2_locationValue = s1m2_locationValue - trackValue3; } else { s1m2_locationValue = s1m2_locationValue + trackValue3; } 
     break;
     case 4:
       if (timer4) {
          digitalWrite(motor2PLSPin, LOW);
         }   
       if (!timer4) {
     digitalWrite(motor2PLSPin, HIGH);
         }
     if (s1motorDir2) { s1m2_locationValue = s1m2_locationValue - trackValue4; } else { s1m2_locationValue = s1m2_locationValue + trackValue4; } 
     break;
    } // end of switch/case
 } else {  // release the brakes
     digitalWrite(motor2PLSPin, HIGH);
     digitalWrite(motor2ENAPin, LOW);
 }
 // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Sculpture 1
 // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Motor 3
 if(s1motorOn3) {
  // Serial.println("s1motorOn3 ON");
   digitalWrite(motor3ENAPin, HIGH);
    switch(s1motorSpeed3) {
      case 0: 
       digitalWrite(motor3PLSPin, LOW); 
       digitalWrite(motor3PLSPin, HIGH);
       break;
      case 1:
       //if (timer1) {
          digitalWrite(motor3PLSPin, LOW);
       //  }   
      // if (!timer1) {
          digitalWrite(motor3PLSPin, HIGH);
        // }
       if (s1motorDir3) { s1m3_locationValue = s1m3_locationValue - trackValue1; } else { s1m3_locationValue = s1m3_locationValue + trackValue1; } 
       break;      
      case 2:
       if (timer2) {
          digitalWrite(motor3PLSPin, LOW);
         }   
       if (!timer2) {
          digitalWrite(motor3PLSPin, HIGH);
         }
       if (s1motorDir3) { s1m3_locationValue = s1m3_locationValue - trackValue2; } else { s1m3_locationValue = s1m3_locationValue + trackValue2; } 

       break;
       case 3:
       if (timer3) {
          digitalWrite(motor3PLSPin, LOW);
         }   
       if (!timer3) {
          digitalWrite(motor3PLSPin, HIGH);
         }
         if (s1motorDir3) { s1m3_locationValue = s1m3_locationValue - trackValue3; } else { s1m3_locationValue = s1m3_locationValue + trackValue3; } 
       break;      
      case 4:
       if (timer4) {
          digitalWrite(motor3PLSPin, LOW);
         }   
       if (!timer4) {
          digitalWrite(motor3PLSPin, HIGH);
         }
         if (s1motorDir3) { s1m3_locationValue = s1m3_locationValue - trackValue4; } else { s1m3_locationValue = s1m3_locationValue + trackValue4; } 
       break;
       
    } // end switch/case 
 } else { //release the brakes
 digitalWrite(motor3PLSPin, HIGH);
 digitalWrite(motor3ENAPin, LOW);
 }
 
 // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Sculpture 2
 // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Motor 1
 if(s2motorOn1) {
 digitalWrite(motor4ENAPin, HIGH);
  switch(s2motorSpeed1) {
    case 0: 
       digitalWrite(motor4PLSPin, LOW); 
       digitalWrite(motor4PLSPin, HIGH);
       break;
    case 1: 
       //if (timer1) {
          digitalWrite(motor4PLSPin, LOW); 
       //   }
       // if (!timer1) {
          digitalWrite(motor4PLSPin, HIGH);
        //  }
        if (s2motorDir1) { s2m1_locationValue = s2m1_locationValue - trackValue1; } else { s2m1_locationValue = s2m1_locationValue + trackValue1; } 
        break;
     case 2: 
       if (timer2) {
          digitalWrite(motor4PLSPin, LOW); 
          }
        if (!timer2) {
          digitalWrite(motor4PLSPin, HIGH);
          }
        if (s2motorDir1) { s2m1_locationValue = s2m1_locationValue - trackValue2; } else { s2m1_locationValue = s2m1_locationValue + trackValue2; } 
        break;
      case 3: 
        if (timer3) {
          digitalWrite(motor4PLSPin, LOW); 
          }
        if (!timer3) {
          digitalWrite(motor4PLSPin, HIGH);
          }
       if (s2motorDir1) { s2m1_locationValue = s2m1_locationValue - trackValue3; } else { s2m1_locationValue = s2m1_locationValue + trackValue3; } 
       break;
      case 4:
      if (timer4) {
        digitalWrite(motor4PLSPin, LOW); 
      }
      if (!timer4) {
        digitalWrite(motor4PLSPin, HIGH);
      }
      if (s2motorDir1) { s2m1_locationValue = s2m1_locationValue - trackValue4; } else { s2m1_locationValue = s2m1_locationValue + trackValue4; } 
       break;
    } // end switch/case
  
 } else { // release the brakes
 digitalWrite(motor4PLSPin, HIGH);
 digitalWrite(motor4ENAPin, LOW);
 }
 // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Sculpture 2
 // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Motor 2
 if(s2motorOn2) {
 digitalWrite(motor5ENAPin, HIGH);
  switch(s2motorSpeed2) {
    case 0: 
       digitalWrite(motor5PLSPin, LOW); 
       digitalWrite(motor5PLSPin, HIGH);
       break;
    case 1: 
       //if (timer1) {
          digitalWrite(motor5PLSPin, LOW); 
         // }
        //if (!timer1) {
          digitalWrite(motor5PLSPin, HIGH);
         // }
          if (s2motorDir2) { s2m2_locationValue = s2m2_locationValue - trackValue1; } else { s2m2_locationValue = s2m2_locationValue + trackValue1; } 
        break;
     case 2: 
       if (timer2) {
          digitalWrite(motor5PLSPin, LOW); 
          }
        if (!timer2) {
          digitalWrite(motor5PLSPin, HIGH);
          }
          if (s2motorDir2) { s2m2_locationValue = s2m2_locationValue - trackValue2; } else { s2m2_locationValue = s2m2_locationValue + trackValue2; } 
        break;
      case 3: 
        if (timer3) {
          digitalWrite(motor5PLSPin, LOW); 
          }
        if (!timer3) {
          digitalWrite(motor5PLSPin, HIGH);
          }
          if (s2motorDir2) { s2m2_locationValue = s2m2_locationValue - trackValue3; } else { s2m2_locationValue = s2m2_locationValue + trackValue3; } 
       break;
      case 4:
      if (timer4) {
        digitalWrite(motor5PLSPin, LOW); 
      }
      if (!timer4) {
        digitalWrite(motor5PLSPin, HIGH);
      }
      if (s2motorDir2) { s2m2_locationValue = s2m2_locationValue - trackValue4; } else { s2m2_locationValue = s2m2_locationValue + trackValue4; } 
       break;
    } // end switch/case
  
 } else { // release the brakes
 digitalWrite(motor5PLSPin, HIGH);
 digitalWrite(motor5ENAPin, LOW);
 }
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Sculpture 2
 // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Motor 3
 if(s2motorOn3) {
 digitalWrite(motor6ENAPin, HIGH);
  switch(s2motorSpeed3) {
    case 0: 
       digitalWrite(motor6PLSPin, LOW); 
       digitalWrite(motor6PLSPin, HIGH);
       break;
    case 1: 
      // if (timer1) {
          digitalWrite(motor6PLSPin, LOW); 
       //   }
       // if (!timer1) {
          digitalWrite(motor6PLSPin, HIGH);
      //    }
          if (s2motorDir3) { s2m3_locationValue = s2m3_locationValue - trackValue1; } else { s2m3_locationValue = s2m3_locationValue + trackValue1; } 
        break;
     case 2: 
       if (timer2) {
          digitalWrite(motor6PLSPin, LOW); 
          }
        if (!timer2) {
          digitalWrite(motor6PLSPin, HIGH);
          }
          if (s2motorDir3) { s2m3_locationValue = s2m3_locationValue - trackValue2; } else { s2m3_locationValue = s2m3_locationValue + trackValue2; } 
        break;
      case 3: 
        if (timer3) {
          digitalWrite(motor6PLSPin, LOW); 
          }
        if (!timer3) {
          digitalWrite(motor6PLSPin, HIGH);
          }
          if (s2motorDir3) { s2m3_locationValue = s2m3_locationValue - trackValue3; } else { s2m3_locationValue = s2m3_locationValue + trackValue3; } 
       break;
      case 4:
      if (timer4) {
        digitalWrite(motor6PLSPin, LOW); 
      }
      if (!timer4) {
        digitalWrite(motor6PLSPin, HIGH);
      }
        if (s2motorDir3) { s2m3_locationValue = s2m3_locationValue - trackValue4; } else { s2m3_locationValue = s2m3_locationValue + trackValue4; } 
       break;
    } // end switch/case
  
 } else { // release the brakes
 digitalWrite(motor6PLSPin, HIGH);
 digitalWrite(motor6ENAPin, LOW);
 }
 
 // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Sculpture 3
 // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Motor 1
 if(s3motorOn1) {
 digitalWrite(motor7ENAPin, HIGH);
  switch(s3motorSpeed1) {
    case 0: 
       digitalWrite(motor7PLSPin, LOW); 
       digitalWrite(motor7PLSPin, HIGH);
       break;
    case 1: 
       //if (timer1) {
          digitalWrite(motor7PLSPin, LOW); 
       //   }
       // if (!timer1) {
          digitalWrite(motor7PLSPin, HIGH);
        //  }
          if (s3motorDir1) { s3m1_locationValue = s3m1_locationValue - trackValue1;} else { s3m1_locationValue = s3m1_locationValue + trackValue1;  } 
        break;
     case 2: 
       if (timer2) {
          digitalWrite(motor7PLSPin, LOW); 
          }
        if (!timer2) {
          digitalWrite(motor7PLSPin, HIGH);
          }
          if (s3motorDir1) { s3m1_locationValue = s3m1_locationValue - trackValue2; } else { s3m1_locationValue = s3m1_locationValue + trackValue2; } 
        break;
      case 3: 
        if (timer3) {
          digitalWrite(motor7PLSPin, LOW); 
          }
        if (!timer3) {
          digitalWrite(motor7PLSPin, HIGH);
          }
          if (s3motorDir1) { s3m1_locationValue = s3m1_locationValue - trackValue3; } else { s3m1_locationValue = s3m1_locationValue + trackValue3; } 
       break;
      case 4:
      if (timer4) {
        digitalWrite(motor7PLSPin, LOW); 
      }
      if (!timer4) {
        digitalWrite(motor7PLSPin, HIGH);
      }
        if (s3motorDir1) { s3m1_locationValue = s3m1_locationValue - trackValue4; } else { s3m1_locationValue = s3m1_locationValue + trackValue4; } 
       break;
    } // end switch/case
  
 } else { // release the brakes
 digitalWrite(motor7PLSPin, HIGH);
 digitalWrite(motor7ENAPin, LOW);
 }
 // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Sculpture 3
 // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Motor 2
 if(s3motorOn2) {
 digitalWrite(motor8ENAPin, HIGH);
  switch(s3motorSpeed2) {
    case 0: 
       digitalWrite(motor8PLSPin, LOW); 
       digitalWrite(motor8PLSPin, HIGH);
       break;
    case 1: 
      // if (timer1) {
          digitalWrite(motor8PLSPin, LOW); 
       //   }
       // if (!timer1) {
          digitalWrite(motor8PLSPin, HIGH);
       //   }
          if (s3motorDir2) { s3m2_locationValue = s3m2_locationValue - trackValue1; } else { s3m2_locationValue = s3m2_locationValue + trackValue1; } 
        break;
     case 2: 
       if (timer2) {
          digitalWrite(motor8PLSPin, LOW); 
          }
        if (!timer2) {
          digitalWrite(motor8PLSPin, HIGH);
          }
          if (s3motorDir2) { s3m2_locationValue = s3m2_locationValue - trackValue2; } else { s3m2_locationValue = s3m2_locationValue + trackValue2; } 
        break;
      case 3: 
        if (timer3) {
          digitalWrite(motor8PLSPin, LOW); 
          }
        if (!timer3) {
          digitalWrite(motor8PLSPin, HIGH);
          }
       if (s3motorDir2) { s3m2_locationValue = s3m2_locationValue - trackValue3; } else { s3m2_locationValue = s3m2_locationValue + trackValue3; } 
       break;
      case 4:
      if (timer4) {
        digitalWrite(motor8PLSPin, LOW); 
      }
      if (!timer4) {
        digitalWrite(motor8PLSPin, HIGH);
      }
        if (s3motorDir2) { s3m2_locationValue = s3m2_locationValue - trackValue4; } else { s3m2_locationValue = s3m2_locationValue + trackValue4; } 
       break;
    } // end switch/case
  
 } else { // release the brakes
 digitalWrite(motor8PLSPin, HIGH);
 digitalWrite(motor8ENAPin, LOW);
 }
 // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Sculpture 3
 // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  Motor 3
 if(s3motorOn3) {
 digitalWrite(motor9ENAPin, HIGH);
  switch(s3motorSpeed3) {
    case 0: 
       digitalWrite(motor9PLSPin, LOW); 
       digitalWrite(motor9PLSPin, HIGH);
       break;
    case 1: 
       //if (timer1) {
          digitalWrite(motor9PLSPin, LOW); 
        //  }
       // if (!timer1) {
          digitalWrite(motor9PLSPin, HIGH);
        //  }
          if (s3motorDir3) { s3m3_locationValue = s3m3_locationValue - trackValue1; } else { s3m3_locationValue = s3m3_locationValue + trackValue1; } 
        break;
     case 2: 
       if (timer2) {
          digitalWrite(motor9PLSPin, LOW);
          }
        if (!timer2) {
          digitalWrite(motor9PLSPin, HIGH);
          }
          if (s3motorDir3) { s3m3_locationValue = s3m3_locationValue - trackValue2; } else { s3m3_locationValue = s3m3_locationValue + trackValue2; } 
        break;
      case 3: 
        if (timer3) {
          digitalWrite(motor9PLSPin, LOW); 
          }
        if (!timer3) {
          digitalWrite(motor9PLSPin, HIGH);
          }
         if (s3motorDir3) { s3m3_locationValue = s3m3_locationValue - trackValue3; } else { s3m3_locationValue = s3m3_locationValue + trackValue3; } 

       break;
      case 4:
      if (timer4) {
        digitalWrite(motor9PLSPin, LOW); 
      }
      if (!timer4) {
        digitalWrite(motor9PLSPin, HIGH);
      }
      if (s3motorDir3) { s3m3_locationValue = s3m3_locationValue - trackValue4; } else { s3m3_locationValue = s3m3_locationValue + trackValue4; } 

       break;
    } // end switch/case
  
 } else { // release the brakes
 digitalWrite(motor9PLSPin, HIGH);
 digitalWrite(motor9ENAPin, LOW);
 }
 

// change directions
if (s1motorDir1) { digitalWrite(motor1DIRPin, LOW); } else { digitalWrite(motor1DIRPin, HIGH); }
if (s1motorDir2) { digitalWrite(motor2DIRPin, LOW); } else { digitalWrite(motor2DIRPin, HIGH); }
if (s1motorDir3) { digitalWrite(motor3DIRPin, LOW); } else { digitalWrite(motor3DIRPin, HIGH); }
if (s2motorDir1) { digitalWrite(motor4DIRPin, LOW); } else { digitalWrite(motor4DIRPin, HIGH); }
if (s2motorDir2) { digitalWrite(motor5DIRPin, LOW); } else { digitalWrite(motor5DIRPin, HIGH); }
if (s2motorDir3) { digitalWrite(motor6DIRPin, LOW); } else { digitalWrite(motor6DIRPin, HIGH); }
if (s3motorDir1) { digitalWrite(motor7DIRPin, LOW); } else { digitalWrite(motor7DIRPin, HIGH); }
if (s3motorDir2) { digitalWrite(motor8DIRPin, LOW); } else { digitalWrite(motor8DIRPin, HIGH); }
if (s3motorDir3) { digitalWrite(motor9DIRPin, LOW); } else { digitalWrite(motor9DIRPin, HIGH); }

}



// ================================================================================================= SWITCHES

// check buttons (only when heading in the directions of one to cut down on PWN interuption)
// ----------------------------------------------------------------- statue 1
// ---------------------------------------------------------- motor1
if (!s1motorDir1 && s1motorOn1) {
  // s1motor1 heading to s1m1swMax
  if(digitalRead(s1m1MaxButton)==HIGH) {
   if(doingCalibrationStep2) { s1m1_locationValue = loopcount; s1m1calibrated = true; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("s1m1calibrated"); } }
   s1m1swMax = true; s1motorOn1 = false; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sw,"); Serial.print("1,1,max,"); Serial.println(s1m1_locationValue); }
   if(s1m1TSA) { if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("OUT OF RANGE: Got to "); Serial.print(s1m1_locationValue); Serial.print(" while looking for ");  Serial.println(s1m1target);} Serial.println("OUT 1"); s1m1_locationValue--; s1m1TSA = false; calibrationErrorCount++; }
   if(doingCalibrationStep2) {
   s1m1max = s1m1_locationValue;    
   if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("s1m1max set to "); Serial.print(s1m1max); Serial.print("."); }
   }
  } else {
   s1m1swMax = false;
  }
}
if (s1motorDir1 && s1motorOn1) {
  if(digitalRead(s1m1MinButton)==HIGH) {
   s1m1swMin = true; s1motorOn1 = false; s1motorDir1= false; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sw,"); Serial.print("1,1,min,"); Serial.println(s1m1_locationValue); }
   s1m1_locationValue = 0;
   s1m1min = 0;   
  } else {
   s1m1swMin = false;
  }
}
// ---------------------------------------------------------- motor2
if (!s1motorDir2 && s1motorOn2) {
  if(digitalRead(s1m2MaxButton)==HIGH) {
   if(doingCalibrationStep2) { s1m2_locationValue = loopcount; s1m2calibrated = true; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("s1m2calibrated");  }}
   s1m2swMax = true; s1motorOn2 = false;  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sw,"); Serial.print("1,2,max,"); Serial.println(s1m2_locationValue); }
   if(s1m2TSA) {  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("OUT OF RANGE: Got to "); Serial.print(s1m2_locationValue); Serial.print(" while looking for ");  Serial.println(s1m2target); } Serial.println("OUT 2"); s1m2_locationValue--; s1m2TSA = false; calibrationErrorCount++; }
   if(doingCalibrationStep2) {
   s1m2max = s1m2_locationValue;    
   if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("s1m2max set to "); Serial.print(s1m2max); Serial.print("."); }
   }
  } else {
   s1m2swMax = false;
  }
}
if (s1motorDir2 && s1motorOn2) {
  if(digitalRead(s1m2MinButton)==HIGH) {
   s1m2swMin = true; s1motorOn2 = false; s1motorDir2= false;  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sw,"); Serial.print("1,2,min,"); Serial.println(s1m1_locationValue); }
   s1m2_locationValue = 0;
   s1m2min = 0;   
  } else {
   s1m2swMin = false;
  }
}
// ---------------------------------------------------------- motor3
if (!s1motorDir3 && s1motorOn3) {
  if(digitalRead(s1m3MaxButton)==HIGH) {
   if(doingCalibrationStep2) { s1m3_locationValue = loopcount; s1m3calibrated = true; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("s1m3calibrated");  } }
   s1m3swMax = true; s1motorOn3=false;  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sw,"); Serial.print("1,3,max,"); Serial.println(s1m3_locationValue); }
   if(s1m3TSA) {  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("OUT OF RANGE: Got to "); Serial.print(s1m3_locationValue); Serial.print(" while looking for ");  Serial.println(s1m3target); } Serial.println("OUT 3"); s1m3_locationValue--; s1m3TSA = false; calibrationErrorCount++; }
   if(doingCalibrationStep2) {
   s1m3max = s1m3_locationValue;    
   if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("s1m3max set to "); Serial.print(s1m3max); Serial.print("."); }
   }
  } else {
   s1m3swMax = false;
  }
}
if (s1motorDir3 && s1motorOn3) {
  if(digitalRead(s1m3MinButton)==HIGH) {
   s1m3swMin=true; s1motorOn3=false; s1motorDir3=false;  
   if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sw,"); Serial.print("1,3,min,"); Serial.print(s1motorOn3); Serial.println(s1m3_locationValue); }
   s1m3_locationValue = 0;
   s1m3min = 0;   
  } else {
   s1m3swMin = false;
  }
}



// ----------------------------------------------------------------- statue 2
// ---------------------------------------------------------- motor1
if (!s2motorDir1 && s2motorOn1) {
  // s2motor1 heading to s2m1swMax
  if(digitalRead(s2m1MaxButton)==HIGH) {
   if(doingCalibrationStep2) { s2m1_locationValue = loopcount; s2m1calibrated = true; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("s2m1calibrated");  } }
   s2m1swMax = true; s2motorOn1 = false;  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sw,"); Serial.print("2,1,max,"); Serial.println(s2m1_locationValue); }
   if(s2m1TSA) {  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("OUT OF RANGE: Got to "); Serial.print(s2m1_locationValue); Serial.print(" while looking for ");  Serial.println(s2m1target); } Serial.println("OUT 4"); s2m1_locationValue--; s2m1TSA = false; calibrationErrorCount++; }
   if(doingCalibrationStep2) { 
   s2m1max = s2m1_locationValue;    
   if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("s2m1max set to "); Serial.print(s2m1max); Serial.print("."); }
   }
  } else {
   s2m1swMax = false;
  }
}
if (s2motorDir1 && s2motorOn1) {
  if(digitalRead(s2m1MinButton)==HIGH) {
   s2m1swMin = true; s2motorOn1=false; s2motorDir1= false;  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sw,"); Serial.print("2,1,min,"); Serial.println(s2m1_locationValue);}
   s2m1_locationValue = 0;
   s2m1min = 0;
  } else {
   s2m1swMin = false;
  }
}
// ---------------------------------------------------------- motor2
if (!s2motorDir2 && s2motorOn2) {
  if(digitalRead(s2m2MaxButton)==HIGH) {
    if(doingCalibrationStep2) { s2m2_locationValue = loopcount; s2m2calibrated = true; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("s2m2calibrated");  } }
   s2m2swMax = true; s2motorOn2 = false;  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sw,"); Serial.print("2,2,max,"); Serial.println(s2m2_locationValue); }
   if(s2m2TSA) {  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("OUT OF RANGE: Got to "); Serial.print(s2m2_locationValue); Serial.print(" while looking for ");  Serial.println(s2m2target); } Serial.println("OUT 5"); s2m2_locationValue--; s2m2TSA = false; calibrationErrorCount++; }
   if(doingCalibrationStep2) {
   s2m2max = s2m2_locationValue;    
   if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("s2m2max set to "); Serial.print(s2m2max); Serial.print("."); }
   }
  } else {
   s2m2swMax = false;
  }
}
if (s2motorDir2 && s2motorOn2) {
  if(digitalRead(s2m2MinButton)==HIGH) {
   s2m2swMin = true; s2motorOn2 = false; s2motorDir2= false;  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sw,"); Serial.print("2,2,min,"); Serial.println(s2m2_locationValue); }
   s2m2_locationValue = 0;
   s2m2min = 0; 
  } else {
   s2m2swMin = false;
  }
}
// ---------------------------------------------------------- motor3
if (!s2motorDir3 && s2motorOn3) {
  if(digitalRead(s2m3MaxButton)==HIGH) {
    if(doingCalibrationStep2) { s2m3_locationValue = loopcount; s2m3calibrated = true; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("s2m3calibrated");  } }
   s2m3swMax = true; s2motorOn3 = false; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sw,"); Serial.print("2,3,max,"); Serial.println(s2m3_locationValue); }
   if(s2m3TSA) {  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("OUT OF RANGE: Got to "); Serial.print(s2m3_locationValue); Serial.print(" while looking for ");  Serial.println(s2m3target); } Serial.println("OUT 6"); s2m3_locationValue--; s2m3TSA = false; calibrationErrorCount++; }
   if(doingCalibrationStep2) {
   s2m3max = s2m3_locationValue;    
   if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("s2m3max set to "); Serial.print(s2m3max); Serial.print(".");} }
   } else {
   s2m3swMax = false;
  }
}
if (s2motorDir3 && s2motorOn3) {
  if(digitalRead(s2m3MinButton)==HIGH) {
    
   s2m3swMin = true; s2motorOn3 = false; s2motorDir3= false;  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sw,"); Serial.print("2,3,min,"); Serial.println(s2m3_locationValue);}
   s2m3_locationValue = 0;
   s2m3min = 0; 
  } else {
   s2m3swMin = false;
  }
}





// ----------------------------------------------------------------- statue 3
// ---------------------------------------------------------- motor1
if (!s3motorDir1 && s3motorOn1) {
  // s1motor1 heading to s1m1swMax
  if(digitalRead(s3m1MaxButton)==HIGH) {
   s3m1swMax = true; s3motorOn1 = false;  
   if(doingCalibrationStep2) { s3m1_locationValue = loopcount; s3m1calibrated = true; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("s3m1calibrated");  } }
   if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sw,"); Serial.print("3,1,max,"); Serial.println(s3m1_locationValue); }
   if(s3m1TSA) {  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("OUT OF RANGE: Got to "); Serial.print(s3m1_locationValue); Serial.print(" while looking for ");  Serial.println(s3m1target);} Serial.println("OUT 7"); s3m1_locationValue--; s3m1TSA = false; calibrationErrorCount++; }
   if(doingCalibrationStep2) { 
   s3m1max = s3m1_locationValue;    
   if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("s3m1max set to "); Serial.print(s3m1max); Serial.print("."); }
   }
  } else {
   s3m1swMax = false;
  }
}

if (s3motorDir1 && s3motorOn1) {
  if(digitalRead(s3m1MinButton)==HIGH) {
   s3m1swMin = true; s3motorOn1 = false; s3motorDir1= false;  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sw,"); Serial.print("3,1,min,"); Serial.println(s3m1_locationValue); }
   s3m1_locationValue = 0; 
   s3m1min = 0;
  } else {
   s3m1swMin = false;
  }
}
// ---------------------------------------------------------- motor2
if (!s3motorDir2 && s3motorOn2) {
  if(digitalRead(s3m2MaxButton)==HIGH) {
    if(doingCalibrationStep2) { s3m2_locationValue = loopcount; s3m2calibrated = true; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("s3m2calibrated");  } }
   s3m2swMax = true; s3motorOn2 = false;  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sw,"); Serial.print("3,2,max,"); Serial.println(s3m2_locationValue); }
   if(s3m2TSA) {  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("OUT OF RANGE: Got to "); Serial.print(s3m2_locationValue); Serial.print(" while looking for ");  Serial.println(s3m2target);} Serial.println("OUT 8"); s3m2_locationValue--; s3m2TSA = false; calibrationErrorCount++; } 
   if(doingCalibrationStep2) { s3m2max = s3m2_locationValue;    
   if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("s3m2max set to "); Serial.print(s3m2max); Serial.print(".");}
   }
  } else {
   s3m2swMax = false;
  }
}
if (s3motorDir2 && s3motorOn2) {
  if(digitalRead(s3m2MinButton)==HIGH) {
   s3m2swMin = true; s3motorOn2 = false; s3motorDir2= false;  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sw,"); Serial.print("3,2,min,"); Serial.println(s3m2_locationValue); }
   s3m2_locationValue = 0;
   s3m2min = 0;
  } else {
   s3m2swMin = false;
  }
}
// ---------------------------------------------------------- motor3
if (!s3motorDir3 && s3motorOn3) {
  if(digitalRead(s3m3MaxButton)==HIGH) {
   if(doingCalibrationStep2) { s3m3_locationValue = loopcount; s3m3calibrated = true; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("s3m3calibrated");  }}
   s3m3swMax = true; s3motorOn3 = false;  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sw,"); Serial.print("3,3,max,"); Serial.println(s3m3_locationValue);}
   if(s3m3TSA) {  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("OUT OF RANGE: Got to "); Serial.print(s3m3_locationValue); Serial.print(" while looking for ");  Serial.println(s3m3target);} Serial.println("OUT 9"); s3m3_locationValue--; s3m3TSA = false; calibrationErrorCount++; }
   if(doingCalibrationStep2) {
   s3m3max = s3m3_locationValue;  
   if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("s3m3max set to "); Serial.print(s3m3max); Serial.print(".");   }
   }

  } else {
   s3m3swMax = false;
  }
}
if (s3motorDir3 && s3motorOn3) {
  if(digitalRead(s3m3MinButton)==HIGH) {
   s3m3_locationValue = 0;
   s3m3min = 0;
   s3m3swMin = true; s3motorOn3 = false; s3motorDir3= false;  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sw,"); Serial.print("3,3,min,"); Serial.println(s3m3_locationValue);}

  } else {
   s3m3swMin = false;
  }
}



 

if (stringComplete) { // this was previously used to update the screen for debuging.
//    display.clearDisplay();
//    display.setTextSize(0);
//    display.setTextColor(WHITE);
//    display.setCursor(0,0);
//    display.println("Got input.");
//    display.display();
//    
//    char test = inputString.charAt(0);
//    char on = String("1").charAt(0);
//    char off = String("0").charAt(0);
//    if(test = on)  {
//    s1motorOn1 = false;
//    display.println("Motor off.");
//    display.display();
//    Serial.print(on);     Serial.print( off);     Serial.println( test);
//    }
//    if(test = off)  {
//    s1motorOn1 = true;
//    display.println("Motor on.");
//    display.display();
//    }    
//    Serial.println(s1motorOn1);
    
    // clear the string:
    inputString = "";
    stringComplete = false;
  }

  

  
  
// update timers for PWM speed control
  unsigned long currentMillis = millis();
  /* Taken out to get max speed from loop. 
  if (currentMillis - previousMillisTimer1 >= intervalTimer1) {
    previousMillisTimer1 = currentMillis;
    timer1 = false;
  } else {
    timer1 = true;
  }
  */
  if (currentMillis - previousMillisTimer2 >= intervalTimer2) {
    previousMillisTimer2 = currentMillis;
    timer2 = false;
  } else {
    timer2 = true;
  }
  if (currentMillis - previousMillisTimer3 >= intervalTimer3) {
    previousMillisTimer3 = currentMillis;
    timer3 = false;
  } else {
    timer3 = true;
  }
  if (currentMillis - previousMillisTimer4 >= intervalTimer4) {
    previousMillisTimer4 = currentMillis;
    timer4 = false;
  } else {
    timer4 = true;
  }
  if (currentMillis - previousMillisTimer5 >= intervalTimer5) {
    previousMillisTimer5 = currentMillis;
    timer5 = false;
  } else {
    timer5 = true;
  }
  


if(doingCalibrationStep1) {
  
      // Move to the bottom switch (position 0)
      if(!s1atMin){ s1toMin(); } 
      if(s1atMin && !s2atMin) { s2toMin(); }
      if(s2atMin && !s3atMin) { s3toMin(); }

      if(!s1atMin) {if(s1m1min==0 && s1m2min==0 && s1m3min==0) { s1atMin = true; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("Statue 1 all at min.");} displayPosLCD(); }}
      if(!s2atMin) {if(s2m1min==0 && s2m2min==0 && s2m3min==0) { s2atMin = true; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("Statue 2 all at min.");} displayPosLCD(); }}
      if(!s3atMin) {if(s3m1min==0 && s3m2min==0 && s3m3min==0) { s3atMin = true; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("Statue 3 all at min.");} displayPosLCD(); }}

  if(s1m1min==0 && s1m2min==0 && s1m3min==0 && s2m1min==0 && s2m2min==0 && s2m3min==0 && s3m1min==0 && s3m2min==0 && s3m3min==0) {
    if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print(" s1m1min:"); Serial.print(s1m1min); 
    Serial.print(" s1m2min:"); Serial.print(s1m2min);
    Serial.print(" s1m2min:"); Serial.print(s1m3min);
    Serial.print(" s2m1min:"); Serial.print(s2m1min);
    Serial.print(" s2m2min:"); Serial.print(s2m2min);
    Serial.print(" s2m3min:"); Serial.print(s2m3min);
    Serial.print(" s3m1min:"); Serial.print(s3m1min);
    Serial.print(" s3m2min:"); Serial.print(s3m2min);
    Serial.print(" s3m3min:"); Serial.println(s3m3min); }
    if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("Ready to calibrate step 2."); }
    doingCalibrationStep1 = false;
    calibrate(2);
  }
}

if(doingCalibrationStep2) {

  if(!s1atMax) { if(s1m1calibrated && s1m2calibrated && s1m3calibrated) { s1atMax = true; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("Statue 1 all at max.");} displayPosLCD(); loopcount=0; }}
  if(!s2atMax) { if(s2m1calibrated && s2m2calibrated && s2m3calibrated) { s2atMax = true; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("Statue 2 all at max.");} displayPosLCD(); loopcount=0; }}
  if(!s3atMax) { if(s3m1calibrated && s3m2calibrated && s3m3calibrated) { s3atMax = true; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("Statue 3 all at max.");} displayPosLCD(); loopcount=0; }}

  //  Move to the top switch store the max number
      if(!s1atMax){ s1toMax(); }
      if(s1atMax && !s2atMax) { s2toMax(); }
      if(s2atMax && !s3atMax) { s3toMax(); }

  if(s1m1calibrated && s1m2calibrated && s1m3calibrated && s2m1calibrated && s2m2calibrated && s2m3calibrated && s3m1calibrated && s3m2calibrated && s3m3calibrated) {
    if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("Calibrated all statues.");}
    
    doingCalibrationStep2 = false;
    displayPos();
    calibrate(3);
  }
}


if(doingCalibrationStep3) {
      if(!s1atMin) { s1toMin(); } 
      if(s1atMin && !s2atMin) { s2toMin(); }
      if(s2atMin && !s3atMin) { s3toMin(); }

      if(!s1atMin) {if(s1m1min==0 && s1m2min==0 && s1m3min==0) { s1atMin = true; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("Statue 1 all at min."); } displayPosLCD(); }}
      if(!s2atMin) {if(s2m1min==0 && s2m2min==0 && s2m3min==0) { s2atMin = true; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("Statue 2 all at min."); } displayPosLCD(); }}
      if(!s3atMin) {if(s3m1min==0 && s3m2min==0 && s3m3min==0) { s3atMin = true; if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("Statue 3 all at min."); } displayPosLCD(); }}
      
      if(s1m1min==0 && s1m2min==0 && s1m3min==0 && s2m1min==0 && s2m2min==0 && s2m3min==0 && s3m1min==0 && s3m2min==0 && s3m3min==0) {
    allCalibrated = true;
    if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("We are calibrated."); }
    Serial.print("MAXVALS "); 
    Serial.print(s1m1max); Serial.print(" "); Serial.print(s1m2max); Serial.print(" "); Serial.print(s1m3max); Serial.print(" ");
    Serial.print(s2m1max); Serial.print(" "); Serial.print(s2m2max); Serial.print(" "); Serial.print(s2m3max); Serial.print(" ");
    Serial.print(s3m1max); Serial.print(" "); Serial.print(s3m2max); Serial.print(" "); Serial.print(s3m3max); Serial.println(" ");
    
    doingCalibrationStep3 = false;
    targetSetMiddle();
}
}


if (targetSearchActive) { targetSearch(); }


// end main loop 
}











/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      if(inputString.length()<=63) {
      //  Serial.println(inputString.length());
      stringComplete = true;
      setValues(inputString);
      } else {
        Serial.print("SERIAL COMMAND TOO LARGE:"); Serial.println(inputString.length());
        inputString="";
      }
      Serial.println(inputString);
    } 
  }
}


void setValues(String values) { // ==================================================================================== Serial Interaction
 if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println(values); }

 if(values.substring(0, 1)=="r") {
   Serial.println("ALL STOP - RESETTING");
   Serial.print("stp,");
   Serial.println(loopcount); 
   displayPos();
   stopall();
  ok = false;
  delay(20);
  asm volatile ("  jmp 0"); // software reset!
 }

if(values.substring(0, 1)=="d") {
  displayPos(); 
 }

if(values.substring(0, 1)=="a") {
 if(active) { active= false; Serial.println("DEACTIVED"); } else { active=true; Serial.println("REACTIVED"); }
}

if(values.substring(0, 1)=="c") { // calibrate position data
  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("CALIBRATION STARTING"); }
  calibrate(1); 
 }

if(values.substring(0, 1)=="z") {
    if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("GOT "); Serial.println(values); }
    buf[0] = (char)0;
    char *p = NULL;
    char *str = NULL;
//    delay(20); // needed to stop the memory writing over it's self too fast
     p = buf;
     if(values.length() < 64) {
    values.toCharArray(buf, sizeof(buf));
    //p = buf;
    if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sizeof(values):"); Serial.println(sizeof(values));}
    if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("values:"); Serial.println(values);}
    if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("buf:"); Serial.println(buf);}
    if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("str:"); Serial.println(str);}
    if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sizeof(buf):"); Serial.println(sizeof(buf));}
    if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("sizeof(str):"); Serial.println(sizeof(str)); }
    int i = 0;
 //   delay(20); // needed to stop the memory writing over it's self too fast
    if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("char count: "); Serial.print(i);  Serial.print(" p:");  Serial.print(p); Serial.print(" str:");  Serial.println(str); }
    while ((str = strtok_r(p, " ", &p)) != NULL) { // delimiter is the space
      targetVals[i] = atol(str);
//      delay(20); // needed to stop the memory writing over it's self too fast
      if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println(targetVals[i]); }
      i++; 
    }
    p = NULL;
    str = NULL;
    } else {
      Serial.println("TOO MUCH DATA TO MOVE");
    }
    
    if(!allCalibrated) {Serial.println("Calibration required before targets can be set.");}
    else {
         targetSearchSet();
     }
    
}
 
if(values.substring(0, 1)=="d") { if(DEBUG_ON) { Serial.print(DEBUG_PRE); DEBUG_ON=false; Serial.print(DEBUG_ON); Serial.println(" - DEBUGGING DISABLED"); } else { DEBUG_ON=true;  Serial.print(DEBUG_ON); Serial.println(" - DEBUGGING ENABLED");}  } // use to set targets and direction

if(values.substring(0, 1)=="t") { // set target(s) (list seperated by commas)
  
  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("Setting search."); }
  //displayPos();
  targetSearchSet();
 
  }
if(values.substring(0, 1)=="m") {
   targetSetMiddle();
}

if(values.substring(0, 1)=="e") { 
  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("Setting radom search numbers and running."); }
  targetSearchSetRandom();
}

if(values.substring(0, 1)=="1") { 
  //testButtons = true; 
  }
 
 if(values.substring(0, 1)=="s") {
   if(values.substring(1, 2)=="1") { // scultpture number
   values = values.substring(2);
  if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print(values);
  Serial.print(",");
  Serial.println(loopcount);}
  if(values.substring(0, 1)=="m") { 
    // set pulse control (on/off)
    if(values.substring(1, 2)=="1") { // motor number 
    s1motorSpeed1 = values.substring(3, 4).toInt(); // set speed
      if(values.substring(2, 3)=="2") {
        writeToScreen("Motor number 1 on RVS "+s1motorSpeed1, 1, 0);
        s1motorOn1 = true;
        s1motorDir1 = true;
      }
      if(values.substring(2, 3)=="1") {
        writeToScreen("Motor number 1 on    ", 1, 0);
        s1motorOn1 = true;
        s1motorDir1 = false;
      }
      if(values.substring(2, 3)=="0") {
        writeToScreen("Motor number 1 off   ", 1, 0);
        s1motorOn1 = false;
        s1motorDir1 = false;
      }
     } // motor number
    if(values.substring(1, 2)=="2") { // motor number
    s1motorSpeed2 = values.substring(3, 4).toInt(); // set speed
      if(values.substring(2, 3)=="2") {
        writeToScreen("M2:  "+String(s1motorSpeed1), 1, 0);
        s1motorOn2 = true;
        s1motorDir2 = true;
      }
      if(values.substring(2, 3)=="1") {
        writeToScreen("Motor number 2 on    "+s1motorSpeed1, 1, 0);
        s1motorOn2 = true;
        s1motorDir2 = false;
      }
      if(values.substring(2, 3)=="0") {
        writeToScreen("Motor number 2 off   "+s1motorSpeed1, 1, 0);
        s1motorOn2 = false;
        s1motorDir2 = false;
      }
     } // motor number
    if(values.substring(1, 2)=="3") { // motor number
    s1motorSpeed3 = values.substring(3, 4).toInt(); // set speed
    if(values.substring(2, 3)=="2") {
        writeToScreen("Motor number 3 on RVS "+s1motorSpeed1, 1, 0);
        s1motorOn3 = true;
        s1motorDir3 = true;
      }
      if(values.substring(2, 3)=="1") {
        writeToScreen("Motor number 3 on    "+s1motorSpeed1, 1, 0);
        s1motorOn3 = true;
        s1motorDir3 = false;
      }
      if(values.substring(2, 3)=="0") {
        writeToScreen("Motor number 3 off   ", 1, 0);
        s1motorOn3 = false;
        s1motorDir3 = false;
      }
     } // motor number
  } // end motor detection

 }// end sculpture1
 
  if(values.substring(1, 2)=="2") { // scultpture number 2
   values = values.substring(2);
    if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println(values); }
  if(values.substring(0, 1)=="m") { 
    // set pulse control (on/off)
    if(values.substring(1, 2)=="1") { // motor number
    s2motorSpeed1 = values.substring(3, 4).toInt(); // set speed
      if(values.substring(2, 3)=="2") {
        writeToScreen("Motor number 1 on RVS", 2, 0);
        s2motorOn1 = true;
        s2motorDir1 = true;
      }
      if(values.substring(2, 3)=="1") {
        writeToScreen("Motor number 1 on    ", 2, 0);
        s2motorOn1 = true;
        s2motorDir1 = false;
      }
      if(values.substring(2, 3)=="0") {
        writeToScreen("Motor number 1 off   ", 2, 0);
        s2motorOn1 = false;
        s2motorDir1 = false;
      }
     } // motor number
    if(values.substring(1, 2)=="2") { // motor number
    s2motorSpeed2 = values.substring(3, 4).toInt(); // set speed
      if(values.substring(2, 3)=="2") {
        writeToScreen("Motor number 2 on RVS", 2, 0);
        s2motorOn2 = true;
        s2motorDir2 = true;
      }
      if(values.substring(2, 3)=="1") {
        writeToScreen("Motor number 2 on    ", 2, 0);
        s2motorOn2 = true;
        s2motorDir2 = false;
      }
      if(values.substring(2, 3)=="0") {
        writeToScreen("Motor number 2 off   ", 2, 0);
        s2motorOn2 = false;
        s2motorDir2 = false;
      }
     } // motor number
    if(values.substring(1, 2)=="3") { // motor number
    s2motorSpeed3 = values.substring(3, 4).toInt(); // set speed
    if(values.substring(2, 3)=="2") {
        writeToScreen("Motor number 3 on RVS", 2, 0);
        s2motorOn3 = true;
        s2motorDir3 = true;
      }
      if(values.substring(2, 3)=="1") {
        writeToScreen("Motor number 3 on    ", 2, 0);
        s2motorOn3 = true;
        s2motorDir3 = false;
      }
      if(values.substring(2, 3)=="0") {
        writeToScreen("Motor number 3 off   ", 2, 0);
        s2motorOn3 = false;
        s2motorDir3 = false;
      }
     } // motor number
  } // end motor detection

 }// end sculpture2
 
   if(values.substring(1, 2)=="3") { // scultpture number 3
   values = values.substring(2);
    if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println(values);}
  if(values.substring(0, 1)=="m") { 
    // set pulse control (on/off)
    if(values.substring(1, 2)=="1") { // motor number
    s3motorSpeed1 = values.substring(3, 4).toInt(); // set speed
      if(values.substring(2, 3)=="2") {
        writeToScreen("Motor number 1 on RVS", 3, 0);
        s3motorOn1 = true;
        s3motorDir1 = true;
      }
      if(values.substring(2, 3)=="1") {
        writeToScreen("Motor number 1 on    ", 3, 0);
        s3motorOn1 = true;
        s3motorDir1 = false;
      }
      if(values.substring(2, 3)=="0") {
        writeToScreen("Motor number 1 off   ", 3, 0);
        s3motorOn1 = false;
        s3motorDir1 = false;
      }
     } // motor number
    if(values.substring(1, 2)=="2") { // motor number
    s3motorSpeed2 = values.substring(3, 4).toInt(); // set speed
      if(values.substring(2, 3)=="2") {
        writeToScreen("Motor number 2 on RVS", 3, 0);
        s3motorOn2 = true;
        s3motorDir2 = true;
      }
      if(values.substring(2, 3)=="1") {
        writeToScreen("Motor number 2 on    ", 3, 0);
        s3motorOn2 = true;
        s3motorDir2 = false;
      }
      if(values.substring(2, 3)=="0") {
        writeToScreen("Motor number 2 off   ", 3, 0);
        s3motorOn2 = false;
        s3motorDir2 = false;
      }
     } // motor number
    if(values.substring(1, 2)=="3") { // motor number
    s3motorSpeed3 = values.substring(3, 4).toInt(); // set speed
    if(values.substring(2, 3)=="2") {
        writeToScreen("Motor number 3 on RVS", 3, 0);
        s3motorOn3 = true;
        s3motorDir3 = true;
      }
      if(values.substring(2, 3)=="1") {
        writeToScreen("Motor number 3 on    ", 3, 0);
        s3motorOn3 = true;
        s3motorDir3 = false;
      }
      if(values.substring(2, 3)=="0") {
        writeToScreen("Motor number 3 off   ", 3, 0);
        s3motorOn3 = false;
        s3motorDir3 = false;
      }
     } // motor number
  } // end motor detection

 }// end sculpture3
 
 
} // end S serach
}

// ===================================================================== TARGET SEARCH




void targetSetMiddle() { // search for target position
  if(allCalibrated) { // only do the following if we have the positions of the motors via calibration
    // We are ready to set targets
    targetSearchActive = true;
    targetSearchActiveS1 = true;
    targetSearchActiveS2 = true;
    targetSearchActiveS3 = true;
    
    s3m1TSA = true; 
    s3motorSpeed1 = 1;
    s3m1target = s3m1max/2;

    s3m2TSA = true; 
    s3motorSpeed2 = 1;
    s3m2target = s3m2max/2;

    s3m3TSA = true; 
    s3motorSpeed3 = 1;
    s3m3target = s3m3max/2;

    s2m1TSA = true; 
    s2motorSpeed1 = 1;
    s2m1target = s1m1max/2;

    s2m2TSA = true; 
    s2motorSpeed2 = 1;
    s2m2target = s2m2max/2;

    s2m3TSA = true; 
    s2motorSpeed3 = 1;
    s2m3target = s2m3max/2;

    s1m1TSA = true; 
    s1motorSpeed1 = 1;
    s1m1target = s1m1max/2;

    s1m2TSA = true; 
    s1motorSpeed2 = 1;
    s1m2target = s1m2max/2;

    s1m3TSA = true; 
    s1motorSpeed3 = 1;
    s1m3target = s1m3max/2;
 
  } else {
  Serial.print("Cannot set targets until calibrated!");
  }
}


void targetSearchSet() { // search for target position
  if(allCalibrated) { // only do the following if we have the positions of the motors via calibration
    // We are ready to set targets
    targetSearchActive = true;
    targetSearchActiveS1 = true;
    targetSearchActiveS2 = true;
    targetSearchActiveS3 = true;
    
    s1m1TSA = true; 
    s1motorSpeed1 = 1;
    s1m1target = targetVals[1];

    s1m2TSA = true; 
    s1motorSpeed2 = 1;
    s1m2target = targetVals[2];

    s1m3TSA = true; 
    s1motorSpeed3 = 1;
    s1m3target = targetVals[3];
    
    s2m1TSA = true; 
    s2motorSpeed1 = 1;
    s2m1target = targetVals[4];

    s2m2TSA = true; 
    s2motorSpeed2 = 1;
    s2m2target = targetVals[5];

    s2m3TSA = true; 
    s2motorSpeed3 = 1;
    s2m3target = targetVals[6];
    
    s3m1TSA = true; 
    s3motorSpeed1 = 1;
    s3m1target = targetVals[7];

    s3m2TSA = true; 
    s3motorSpeed2 = 1;
    s3m2target = targetVals[8];

    s3m3TSA = true; 
    s3motorSpeed3 = 1;
    s3m3target = targetVals[9];
 
  } else {
  Serial.print("Cannot set targets until calibrated!");
  }
}


void targetSearchSetRandom() { // search for target position
  if(allCalibrated) { // only do the following if we have the positions of the motors via calibration
    // We are ready to set targets
    targetSearchActive = true;
    targetSearchActiveS1 = true;
    targetSearchActiveS2 = true;
    targetSearchActiveS3 = true;
    
    s1m1TSA = true; 
    s1motorSpeed1 = 1;
    s1m1target = random(0,s1m1max);;

    s1m2TSA = true; 
    s1motorSpeed2 = 1;
    s1m2target = random(0,s1m2max);;

    s1m3TSA = true; 
    s1motorSpeed3 = 1;
    s1m3target = random(0,s1m3max);;

    s2m1TSA = true; 
    s2motorSpeed1 = 1;
    s2m1target = random(0,s2m1max);;

    s2m2TSA = true; 
    s2motorSpeed2 = 1;
    s2m2target = random(0,s2m2max);;

    s2m3TSA = true; 
    s2motorSpeed3 = 1;
    s2m3target = random(0,s2m3max);;


    s3m1TSA = true; 
    s3motorSpeed1 = 1;
    s3m1target = random(0,s3m1max);

    s3m2TSA = true; 
    s3motorSpeed2 = 1;
    s3m2target = random(0,s3m2max);;

    s3m3TSA = true; 
    s3motorSpeed3 = 1;
    s3m3target = random(0,s3m3max);;

 
  } else {
  Serial.print("Cannot set targets until calibrated!");
  }
}




void targetSearch() { // search for target position
  if(allCalibrated) { // only do the following if we have the positions of the motors via calibration

  // ----- Motor s1m1
  if (s1m1TSA) { // if target search activated, move to target
    if(s1m1_locationValue != s1m1target) {
     if(s1m1_locationValue > s1m1target) {    
             s1motorOn1 = true;
             s1motorDir1 = true;
     } 
     if(s1m1_locationValue < s1m1target) {    
             s1motorOn1 = true;
             s1motorDir1 = false;
     } 
    } else {
      s1motorOn1 = false;
      s1motorDir1 = false;
      s1m1TSA = false;
      //displayPos();
      if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("s1m1 Target found:"); Serial.print(s1m1_locationValue); Serial.print(" / "); Serial.println(s1m1max); }
    }
  } // End Motor s1m1

  
    // ----- Motor s1m2
  if (s1m2TSA) { // if target search activated, move to target
    if(s1m2_locationValue != s1m2target) {
     if(s1m2_locationValue > s1m2target) {    
             s1motorOn2 = true;
             s1motorDir2 = true;
     } 
     if(s1m2_locationValue < s1m2target) {    
             s1motorOn2 = true;
             s1motorDir2 = false;
     } 
    } else {
      s1motorOn2 = false;
      s1motorDir2 = false;
      s1m2TSA = false;
      //displayPos();
      if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("s1m2 Target found:"); Serial.print(s1m2_locationValue); Serial.print(" / "); Serial.println(s1m2max); }
    }
  } // End Motor s1m2

  // ----- Motor s1m3
  if (s1m3TSA) { // if target search activated, move to target
    if(s1m3_locationValue != s1m3target) {
     if(s1m3_locationValue > s1m3target) {    
             s1motorOn3 = true;
             s1motorDir3 = true;
     } 
     if(s1m3_locationValue < s1m3target) {    
             s1motorOn3 = true;
             s1motorDir3 = false;
     } 
    } else {
      s1motorOn3 = false;
      s1motorDir3 = false;
      s1m3TSA = false;
      //displayPos();
      if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("s1m3 Target found:"); Serial.print(s1m3_locationValue); Serial.print(" / "); Serial.println(s1m3max); }
    }
  } // End Motor s1m3

  // ----- Motor s2m1
  if (s2m1TSA) { // if target search activated, move to target
    if(s2m1_locationValue != s2m1target) {
     if(s2m1_locationValue > s2m1target) {    
             s2motorOn1 = true;
             s2motorDir1 = true;
     } 
     if(s2m1_locationValue < s2m1target) {    
             s2motorOn1 = true;
             s2motorDir1 = false;
     } 
    } else {
      s2motorOn1 = false;
      s2motorDir1 = false;
      s2m1TSA = false;
      //displayPos();
      if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("s2m1 Target found:"); Serial.print(s2m1_locationValue); Serial.print(" / "); Serial.println(s2m1max); }
    }
  } // End Motor s2m1

  
    // ----- Motor s2m2
  if (s2m2TSA) { // if target search activated, move to target
    if(s2m2_locationValue != s2m2target) {
     if(s2m2_locationValue > s2m2target) {    
             s2motorOn2 = true;
             s2motorDir2 = true;
     } 
     if(s2m2_locationValue < s2m2target) {    
             s2motorOn2 = true;
             s2motorDir2 = false;
     } 
    } else {
      s2motorOn2 = false;
      s2motorDir2 = false;
      s2m2TSA = false;
      //displayPos();
      if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("s2m2 Target found:"); Serial.print(s2m2_locationValue); Serial.print(" / "); Serial.println(s2m2max); }
    }
  } // End Motor s2m2

  // ----- Motor s2m3
  if (s2m3TSA) { // if target search activated, move to target
    if(s2m3_locationValue != s2m3target) {
     if(s2m3_locationValue > s2m3target) {    
             s2motorOn3 = true;
             s2motorDir3 = true;
     } 
     if(s2m3_locationValue < s2m3target) {    
             s2motorOn3 = true;
             s2motorDir3 = false;
     } 
    } else {
      s2motorOn3 = false;
      s2motorDir3 = false;
      s2m3TSA = false;
      //displayPos();
      if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("s2m3 Target found:"); Serial.print(s2m3_locationValue); Serial.print(" / "); Serial.println(s2m3max); }
    }
  } // End Motor s2m3

  // ----- Motor S3m1
  if (s3m1TSA) { // if target search activated, move to target
    
    if(s3m1_locationValue != s3m1target) {
     if(s3m1_locationValue > s3m1target) {    
             s3motorOn1 = true;
             s3motorDir1 = true;
     } 
     if(s3m1_locationValue < s3m1target) {    
             s3motorOn1 = true;
             s3motorDir1 = false;
     } 
    } else {
      s3motorOn1 = false;
      s3motorDir1 = false;
      s3m1TSA = false;
      //displayPos();
      if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("s3m1 Target found:"); Serial.print(s3m1_locationValue); Serial.print(" / "); Serial.println(s3m1max); }
    }
  } // End Motor S3m1

  
    // ----- Motor S3m2
  if (s3m2TSA) { // if target search activated, move to target
    if(s3m2_locationValue != s3m2target) {
     if(s3m2_locationValue > s3m2target) {    
             s3motorOn2 = true;
             s3motorDir2 = true;
     } 
     if(s3m2_locationValue < s3m2target) {    
             s3motorOn2 = true;
             s3motorDir2 = false;
     } 
    } else {
      s3motorOn2 = false;
      s3motorDir2 = false;
      s3m2TSA = false;
      //displayPos();
      if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("s3m2 Target found:"); Serial.print(s3m2_locationValue); Serial.print(" / "); Serial.println(s3m2max); }
    }
  } // End Motor s3m2

  // ----- Motor s3m3
  if (s3m3TSA) { // if target search activated, move to target
    if(s3m3_locationValue != s3m3target) {
     if(s3m3_locationValue > s3m3target) {    
             s3motorOn3 = true;
             s3motorDir3 = true;
     } 
     if(s3m3_locationValue < s3m3target) {    
             s3motorOn3 = true;
             s3motorDir3 = false;
     } 
    } else {

      s3motorOn3 = false;
      s3motorDir3 = false;
      s3m3TSA = false;
      //displayPos();
      if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("s3m3 Target found:"); Serial.print(s3m3_locationValue); Serial.print(" / "); Serial.println(s3m3max); }
    }
  } // End Motor s3m3



    if((!s1m1TSA && !s1m2TSA && !s1m3TSA) && targetSearchActiveS1 ) { displayPos(); targetSearchActiveS1=false; }
    if((!s2m1TSA && !s2m2TSA && !s2m3TSA) && targetSearchActiveS2 ) { displayPos(); targetSearchActiveS2=false; }
    if((!s3m1TSA && !s3m2TSA && !s3m3TSA) && targetSearchActiveS3 ) { displayPos(); targetSearchActiveS3=false; }
    
    if(!s1m1TSA && !s1m2TSA && !s1m3TSA && !s2m1TSA && !s2m2TSA && !s2m3TSA && !s3m1TSA && !s3m2TSA && !s3m3TSA) { targetSearchActive = false; } // stop target search if all targets found
  } else {
  Serial.print("Cannot move to targets until calibrated!");
  }
  
}



// ===================================================================== CALIBRATION

void calibrate(int step) {  
 
  if(step==1) {
    loopcount = 0;
      // Set flags to say we are calibrating:
      allCalibrated = false;
      s1m1calibrated = false;
      s1m2calibrated = false;
      s1m3calibrated = false;
           
      s2m1calibrated = false;
      s2m2calibrated = false;  
      s2m3calibrated = false;
           
      s3m1calibrated = false;
      s3m2calibrated = false;
      s3m3calibrated = false;

      s1atMax = false;
      s2atMax = false;
      s3atMax = false;
      s1atMin = false;
      s2atMin = false;
      s3atMin = false;

      s1m1min = 1;
      s1m2min = 1;
      s1m3min = 1;
      s2m1min = 1;
      s2m2min = 1;
      s2m3min = 1;
      s3m1min = 1;
      s3m2min = 1;
      s3m3min = 1;
     
      s1m1max = 0;
      s1m2max = 0;
      s1m3max = 0;
      s2m1max = 0;
      s2m2max = 0;
      s2m3max = 0;
      s3m1max = 0;
      s3m2max = 0;
      s3m3max = 0;


      calibrationErrorCount = 0;

      doingCalibrationStep1 = true; 
      
  }
  if(step==2) { 
      loopcount = 0;
      if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("Loopcount1:"); Serial.println(loopcount);  }
      doingCalibrationStep2 = true;  

  
  }

    if(step==3) {  
      
      if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.print("Loopcount2:"); Serial.println(loopcount); }
      // Move back to the bottom switch and print to the serial port and display the config numbers
      doingCalibrationStep1 = false; 
      doingCalibrationStep2 = false;       
      doingCalibrationStep3 = true;
      if(DEBUG_ON) { Serial.print(DEBUG_PRE); Serial.println("Moving to middle all statues."); }
      
      
  }
  if(allCalibrated) {targetSetMiddle();}
}


void s1toMax() { 
      s1atMax=false;
      if(s1m1max<1) {s1motorOn1 = true; s1motorDir1 = false; } 
      if(s1m2max<1) {s1motorOn2 = true; s1motorDir2 = false; }
      if(s1m3max<1) {s1motorOn3 = true; s1motorDir3 = false; } 
      }

void s2toMax() { 
      s2atMax=false;
      if(s2m1max<1) { s2motorOn1 = true; s2motorDir1 = false; }
      if(s2m2max<1) { s2motorOn2 = true; s2motorDir2 = false; }
      if(s2m3max<1) { s2motorOn3 = true; s2motorDir3 = false; }
}

void s3toMax() { 
      s3atMax=false;
      if(s3m1max<1) { s3motorOn1 = true; s3motorDir1 = false; }
      if(s3m2max<1) { s3motorOn2 = true; s3motorDir2 = false; }
      if(s3m3max<1) { s3motorOn3 = true; s3motorDir3 = false; } 
}

void s1toMin() {
      s1atMin = false;
      if(!s1m1min<1) { s1motorOn1 = true; s1motorDir1 = true; }
      if(!s1m2min<1) { s1motorOn2 = true; s1motorDir2 = true; }
      if(!s1m3min<1) { s1motorOn3 = true; s1motorDir3 = true; }
}

void s2toMin() {
      s2atMin = false;
      if(!s2m1min<1) { s2motorOn1 = true; s2motorDir1 = true; }
      if(!s2m2min<1) { s2motorOn2 = true; s2motorDir2 = true; }
      if(!s2m3min<1) { s2motorOn3 = true; s2motorDir3 = true; }
}

void s3toMin() {
      s3atMin = false;
      if(!s3m1min<1) { s3motorOn1 = true; s3motorDir1 = true; }
      if(!s3m2min<1) { s3motorOn2 = true; s3motorDir2 = true; }
      if(!s3m3min<1) { s3motorOn3 = true; s3motorDir3 = true; }
  
}


int writeToScreen(String text, int line, boolean cls) {
  if(cls){ display.clearDisplay(); display.setCursor(0,0); }   
    display.setTextSize(0);
    //display.setTextColor(WHITE);
    display.setTextColor(WHITE, 0);
    if(!line) { display.setCursor(0,0); } else { display.setCursor(0,line*8); line++; } 
    display.println(text);
    display.display();
}


void stopall() {
  
  s1motorOn1 = false;
  s1motorOn2 = false;
  s1motorOn3 = false;
  s2motorOn1 = false;
  s2motorOn2 = false;
  s2motorOn3 = false;
  s3motorOn1 = false;
  s3motorOn2 = false;
  s3motorOn3 = false;
  
  
  digitalWrite(motor1PLSPin, HIGH);  // no pulse
  digitalWrite(motor2PLSPin, HIGH); 
  digitalWrite(motor3PLSPin, HIGH); 
  digitalWrite(motor4PLSPin, HIGH); 
  digitalWrite(motor5PLSPin, HIGH); 
  digitalWrite(motor6PLSPin, HIGH); 
  digitalWrite(motor7PLSPin, HIGH); 
  digitalWrite(motor8PLSPin, HIGH); 
  digitalWrite(motor9PLSPin, HIGH); 
  
  digitalWrite(motor1ENAPin, LOW); // HIGH to release the current (to allow free movement) until we have instructions
  digitalWrite(motor2ENAPin, LOW); 
  digitalWrite(motor3ENAPin, LOW); 
  digitalWrite(motor4ENAPin, LOW); 
  digitalWrite(motor5ENAPin, LOW); 
  digitalWrite(motor6ENAPin, LOW); 
  digitalWrite(motor7ENAPin, LOW); 
  digitalWrite(motor8ENAPin, LOW); 
  digitalWrite(motor9ENAPin, LOW); 
  
  digitalWrite(motor1DIRPin, HIGH);  // reset the direction pin
  digitalWrite(motor2DIRPin, HIGH);
  digitalWrite(motor3DIRPin, HIGH);
  digitalWrite(motor4DIRPin, HIGH);
  digitalWrite(motor5DIRPin, HIGH);
  digitalWrite(motor6DIRPin, HIGH);
  digitalWrite(motor7DIRPin, HIGH);
  digitalWrite(motor8DIRPin, HIGH);
  digitalWrite(motor9DIRPin, HIGH);
  
  ok = true;

}


void displayPos() {
   if(DEBUG_ON) {
   Serial.print("s1m1:"); Serial.print(s1m1_locationValue); Serial.print(" MIN:"); Serial.print(s1m1min); Serial.print(" MAX:"); Serial.print(s1m1max); Serial.print(" SEARCHVAL:"); Serial.println(s1m1target);
   Serial.print("s1m2:"); Serial.print(s1m2_locationValue); Serial.print(" MIN:"); Serial.print(s1m2min); Serial.print(" MAX:"); Serial.print(s1m2max); Serial.print(" SEARCHVAL:"); Serial.println(s1m2target);
   Serial.print("s1m3:"); Serial.print(s1m3_locationValue); Serial.print(" MIN:"); Serial.print(s1m3min); Serial.print(" MAX:"); Serial.print(s1m3max); Serial.print(" SEARCHVAL:"); Serial.println(s1m3target);
   Serial.print("s2m1:"); Serial.print(s2m1_locationValue); Serial.print(" MIN:"); Serial.print(s2m1min); Serial.print(" MAX:"); Serial.print(s2m1max); Serial.print(" SEARCHVAL:"); Serial.println(s2m1target);
   Serial.print("s2m2:"); Serial.print(s2m2_locationValue); Serial.print(" MIN:"); Serial.print(s2m2min); Serial.print(" MAX:"); Serial.print(s2m2max); Serial.print(" SEARCHVAL:"); Serial.println(s2m2target);
   Serial.print("s2m3:"); Serial.print(s2m3_locationValue); Serial.print(" MIN:"); Serial.print(s2m3min); Serial.print(" MAX:"); Serial.print(s2m3max); Serial.print(" SEARCHVAL:"); Serial.println(s2m3target);
   Serial.print("s3m1:"); Serial.print(s3m1_locationValue); Serial.print(" MIN:"); Serial.print(s3m1min); Serial.print(" MAX:"); Serial.print(s3m1max); Serial.print(" SEARCHVAL:"); Serial.println(s3m1target);
   Serial.print("s3m2:"); Serial.print(s3m2_locationValue); Serial.print(" MIN:"); Serial.print(s3m2min); Serial.print(" MAX:"); Serial.print(s3m2max); Serial.print(" SEARCHVAL:"); Serial.println(s3m2target);
   Serial.print("s3m3:"); Serial.print(s3m3_locationValue); Serial.print(" MIN:"); Serial.print(s3m3min); Serial.print(" MAX:"); Serial.print(s3m3max); Serial.print(" SEARCHVAL:"); Serial.println(s3m3target);
   if(!s1m1calibrated) { Serial.println("s1m1 NOT calibrated"); }
   if(!s1m2calibrated) { Serial.println("s1m2 NOT calibrated"); }
   if(!s1m3calibrated) { Serial.println("s1m3 NOT calibrated"); }
   if(!s2m1calibrated) { Serial.println("s2m1 NOT calibrated"); }
   if(!s2m2calibrated) { Serial.println("s2m2 NOT calibrated"); }
   if(!s2m3calibrated) { Serial.println("s2m3 NOT calibrated"); }
   if(!s3m1calibrated) { Serial.println("s3m1 NOT calibrated"); }
   if(!s3m2calibrated) { Serial.println("s3m2 NOT calibrated"); }
   if(!s3m3calibrated) { Serial.println("s3m3 NOT calibrated"); }
   Serial.print("calibrationErrorCount: "); Serial.println(calibrationErrorCount); 
   } else {
    Serial.print("LOCATIONS "); 
    Serial.print(s1m1_locationValue); Serial.print(" "); Serial.print(s1m2_locationValue); Serial.print(" "); Serial.print(s1m3_locationValue); Serial.print(" ");
    Serial.print(s2m1_locationValue); Serial.print(" "); Serial.print(s2m2_locationValue); Serial.print(" "); Serial.print(s2m3_locationValue); Serial.print(" ");
    Serial.print(s3m1_locationValue); Serial.print(" "); Serial.print(s3m2_locationValue); Serial.print(" "); Serial.print(s3m3_locationValue); Serial.println(" ");
    if(calibrationErrorCount>9) { Serial.print("ERRORS "); Serial.print(calibrationErrorCount); Serial.println(" ");}
   }
   
   displayPosLCD();
  
  }



void displayPosLCD() {
   display.clearDisplay();
   display.setCursor(0,0); display.print("1-1:"); display.print(s1m1_locationValue); 
   display.setCursor(0,8); display.print("1-2:"); display.print(s1m2_locationValue); 
   display.setCursor(0,16); display.print("1-3:"); display.print(s1m3_locationValue);
   display.setCursor(0,32); display.print("2-1:"); display.print(s2m1_locationValue);
   display.setCursor(0,40); display.print("2-2:"); display.print(s2m2_locationValue);
   display.setCursor(0,48); display.print("2-3:"); display.print(s2m3_locationValue); 
   display.setCursor(64,0); display.print("3-1:"); display.print(s3m1_locationValue); 
   display.setCursor(64,8); display.print("3-2:"); display.print(s3m2_locationValue); 
   display.setCursor(64,16); display.print("3-3:"); display.print(s3m3_locationValue);
   display.setCursor(64,40); display.print("= CURRENT");
   display.setCursor(64,48); display.print("POSITIONS");
   display.display();
  
  }











void buttonTesting() {
 /*
// ============================================================= Button testing
// sculpture 1
 if(digitalRead(s1m1MaxButton)==HIGH) {
   s1m1swMax = true;
 } else {
   s1m1swMax = false;
 }
 if(digitalRead(s1m1MinButton)==HIGH) {
   s1m1swMin = true;
 } else {
   s1m1swMin = false;
 }
  if(digitalRead(s1m2MaxButton)==HIGH) {
   s1m2swMax = true;
 } else {
   s1m2swMax = false;
 }
 if(digitalRead(s1m2MinButton)==HIGH) {
   s1m2swMin = true;
 } else {
   s1m2swMin = false;
 }
  if(digitalRead(s1m3MaxButton)==HIGH) {
   s1m3swMax = true;
 } else {
   s1m3swMax = false;
 }
 if(digitalRead(s1m3MinButton)==HIGH) {
   s1m3swMin = true;
 } else {
   s1m3swMin = false;
 }
 // sculpture 2
 if(digitalRead(s2m1MaxButton)==HIGH) {
   s2m1swMax = true;
 } else {
   s2m1swMax = false;
 }
 if(digitalRead(s2m1MinButton)==HIGH) {
   s2m1swMin = true;
 } else {
   s2m1swMin = false;
 }
  if(digitalRead(s2m2MaxButton)==HIGH) {
   s2m2swMax = true;
 } else {
   s2m2swMax = false;
 }
 if(digitalRead(s2m2MinButton)==HIGH) {
   s2m2swMin = true;
 } else {
   s2m2swMin = false;
 }
if(digitalRead(s2m3MaxButton)==HIGH) {
   s2m3swMax = true;
 } else {
   s2m3swMax = false;
 }
 if(digitalRead(s2m3MinButton)==HIGH) {
   s2m3swMin = true;
 } else {
   s2m3swMin = false;
 }
 
  // sculpture 3
 if(digitalRead(s3m1MaxButton)==HIGH) {
   s3m1swMax = true;
 } else {
   s3m1swMax = false;
 }
 if(digitalRead(s3m1MinButton)==HIGH) {
   s3m1swMin = true;
 } else {
   s3m1swMin = false;
 }
  if(digitalRead(s3m2MaxButton)==HIGH) {
   s3m2swMax = true;
 } else {
   s3m2swMax = false;
 }
 if(digitalRead(s3m2MinButton)==HIGH) {
   s3m2swMin = true;
 } else {
   s3m2swMin = false;
 }
if(digitalRead(s3m3MaxButton)==HIGH) {
   s3m3swMax = true;
 } else {
   s3m3swMax = false;
 }
 if(digitalRead(s3m3MinButton)==HIGH) {
   s3m3swMin = true;
 } else {
   s3m3swMin = false;
 }
 
 
 
 
   display.clearDisplay();
   if (s3m1swMax) { display.setCursor(0,0); display.println("s3m1swMax");  Serial.println("s3m1swMax");  }
   if (s3m1swMin) { display.setCursor(16,0); display.println("s3m1swMin"); Serial.println("s3m1swMin");  }
   if (s3m2swMax) { display.setCursor(0,16); display.println("s3m2swMax"); Serial.println("s3m2swMax");  }
   if (s3m2swMin) { display.setCursor(16,16); display.println("s3m2swMin"); Serial.println("s3m2swMin");  }
   if (s3m3swMax) { display.setCursor(0,24); display.println("s3m3swMax");  Serial.println("s3m3swMax"); } 
   if (s3m3swMin) { display.setCursor(24,24); display.println("s3m3swMin"); Serial.println("s3m3swMin");  }
   
   if (s2m1swMax) { display.setCursor(0,0); display.println("s2m1swMax");  Serial.println("s2m1swMax"); }
   if (s2m1swMin) { display.setCursor(16,0); display.println("s2m1swMin"); Serial.println("s2m1swMin");  }
   if (s2m2swMax) { display.setCursor(0,16); display.println("s2m2swMax"); Serial.println("s2m2swMax");  }
   if (s2m2swMin) { display.setCursor(16,16); display.println("s2m2swMin"); Serial.println("s2m2swMin");   }
   if (s2m3swMax) { display.setCursor(0,24); display.println("s2m3swMax"); Serial.println("s2m3swMax");  } 
   if (s2m3swMin) { display.setCursor(24,24); display.println("s2m3swMin"); Serial.println("s2m3swMin");  }
   
   if (s1m1swMax) { display.setCursor(0,0); display.println("s1m1swMax"); Serial.println("s1m1swMax");  }
   if (s1m1swMin) { display.setCursor(16,0); display.println("s1m1swMin"); Serial.println("s1m1swMin");  }
   if (s1m2swMax) { display.setCursor(0,16); display.println("s1m2swMax");  Serial.println("s1m2swMax"); }
   if (s1m2swMin) { display.setCursor(16,16); display.println("s1m2swMin"); Serial.println("s1m2swMin");  }
   if (s1m3swMax) { display.setCursor(0,24); display.println("s1m3swMax"); Serial.println("s1m3swMax");  } 
   if (s1m3swMin) { display.setCursor(24,24); display.println("s1m3swMin"); Serial.println("s1m3swMin");  }
   
   
   
   display.display();

*/

}

