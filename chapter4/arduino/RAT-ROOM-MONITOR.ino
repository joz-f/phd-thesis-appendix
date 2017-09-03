/*********************************************************************
This is a sketch to collect data for the RAT.SYSTEMS project
Julie Freeman & Matthew Jarvis 2016

Set Sample window to change the update speed 50 mS = 20Hz

*********************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <dht11.h>

dht11 DHT11; // temp & humidity library
#define DHT11PIN 2 // temp & humidity pin

// If using software SPI (the default case):

String data; // the data is sent comma deliminated as Light (0/1), Light value (0-1024), Humidity (%), Temp (ªC), Amplitude, Sound level in volts

// Light detection
int LDR = 0;                  // Analog pin to which LDR is connected, here we set it to 0 so it means A0
int LDRValue = 0;             // variable to store LDR values
int light_sensitivity = 200;  // This is the approx value of light surrounding your 
int lightActive = 0;


// sound detection 
const int sampleWindow = 50;  // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
#define AUDIOIN   1           // audio in port

// display details
#define OLED_MOSI   9 
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
#define XPOS 0
#define YPOS 1
#define DELTAY 2
// check right library loaded (for display)
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// Let's begin!
void setup()   {                
  Serial.begin(9600); // Serial speed

  // start the display driver
  display.begin(SSD1306_SWITCHCAPVCC); 

  // Clear the buffer.
  display.clearDisplay();

  // text display welcome message
  display.setCursor(0,0);
  display.setTextColor(BLACK, WHITE); // 'inverted' text
  display.setTextSize(1);
  display.println("MJ CREATIVE TECH 2016");
  display.setTextColor(WHITE);
  display.println("RAT.systems J.FREEMAN");
  display.setTextSize(2);
  display.println("ROOM   ");
  display.println("MONITORING");
  display.println("    SYSTEM");
  display.setTextColor(WHITE);
  display.print("0x"); display.println(0xDEADBEEF, HEX);
  display.display();
  delay(2000);
  display.clearDisplay();

  // check temp sensor is working
  int chk = DHT11.read(DHT11PIN);
  //Serial.print("Read temp sensor: ");
  //switch (chk)
  //{
  //  case 0: Serial.println("OK"); break;
  //  case -1: Serial.println("Checksum error"); break;
  //  case -2: Serial.println("Time out error"); break;
  //  default: Serial.println("Unknown error"); break;
  //}


}



// ....and lets grab data, display it and send to serial!
void loop() {


    // test audio level
    unsigned long startMillis= millis();  // Start of sample window
    unsigned int peakToPeak = 0;   // peak-to-peak level
    unsigned int signalMax = 0;
    unsigned int signalMin = 1024;
 
   // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(AUDIOIN);
      if (sample < 1024)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
   peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
   double volts = (peakToPeak * 3.3) / 1024;  // convert to volts
 

    // test light level
    LDRValue = analogRead(LDR);      //reads the ldr’s value through LDR 
    //Serial.println(LDRValue);       //prints the LDR values to serial monitor
    
    // test temp & humidity
    int chk = DHT11.read(DHT11PIN);

   
    display.setCursor(0,0);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("RAT.SYSTEMS 2016");
    display.println("");
    display.print("Light: ");
    if(LDRValue > light_sensitivity) { display.print("ON  "); lightActive=1; } else {display.print("OFF "); lightActive = 0;}
    display.print("v=");
    display.println(LDRValue);
    display.print("Humidity (%): ");
    display.println((float)DHT11.humidity, 0);
    display.print("Temperature ");
    display.print((float)DHT11.temperature, 0);
    display.println(" C");
    display.print("Amplitude: ");
    display.print(peakToPeak);
    display.println(" @20Hz");
    display.print(volts);
    display.print(" volts");
    display.display();
    Serial.print( lightActive ); 
    Serial.print( "," );
    Serial.print( LDRValue );
    Serial.print( "," );
    Serial.print((float)DHT11.humidity, 0);
    Serial.print( "," );
    Serial.print((float)DHT11.temperature, 0);
    Serial.print( "," );
    Serial.print( peakToPeak );
    Serial.print( "," );
    Serial.println( volts );
    display.clearDisplay();
}





void testdrawchar(void) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  for (uint8_t i=0; i < 168; i++) {
    if (i == '\n') continue;
    display.write(i);
    if ((i > 0) && (i % 21 == 0))
      display.println();
  }    
  display.display();
}




/*-----( Declare User-written Functions )-----*/
//
//Celsius to Fahrenheit conversion
double Fahrenheit(double celsius)
{
  return 1.8 * celsius + 32;
}

//Celsius to Kelvin conversion
double Kelvin(double celsius)
{
  return celsius + 273.15;
}

// dewPoint function NOAA
// reference: http://wahiduddin.net/calc/density_algorithms.htm 
double dewPoint(double celsius, double humidity)
{
  double A0= 373.15/(273.15 + celsius);
  double SUM = -7.90298 * (A0-1);
  SUM += 5.02808 * log10(A0);
  SUM += -1.3816e-7 * (pow(10, (11.344*(1-1/A0)))-1) ;
  SUM += 8.1328e-3 * (pow(10,(-3.49149*(A0-1)))-1) ;
  SUM += log10(1013.246);
  double VP = pow(10, SUM-3) * humidity;
  double T = log(VP/0.61078);   // temp var
  return (241.88 * T) / (17.558-T);
}

// delta max = 0.6544 wrt dewPoint()
// 5x faster than dewPoint()
// reference: http://en.wikipedia.org/wiki/Dew_point
double dewPointFast(double celsius, double humidity)
{
  double a = 17.271;
  double b = 237.7;
  double temp = (a * celsius) / (b + celsius) + log(humidity/100);
  double Td = (b * temp) / (a - temp);
  return Td;
}



