#include <Arduino.h>
#include <DHT.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <U8glib-HAL.h>
//#include <U8g2lib.h>
#include "MQ135.h" //MQ135 by GeorgK

//DHT11
#define DHTpin 10
#define DHTTYPE DHT11
//DHT dht(DHTpin, DHTTYPE);
DHT dht(DHTpin, DHTTYPE);



//MQTT
#define MQpin 4
String prevQuality = "";
int prevConductivity = 0;
int prevGas = -1;
String sensor_line1;
String sensor_line2;
String sensor_line3;

//Oled
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< I2C Address 0x3C for 128x64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void setup() {
  
  
  Serial.begin(9600);

  dht.begin();


    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MQpin, INPUT);
}


void loop()
{

 delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  int h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  //float hif = dht.computeHeatIndex(f, h)
  // Compute heat index in Celsius (isFahreheit = false)
  //float hic = dht1.computeHeatIndex(t, h, false);

  
  // Get temperature event and print its value
/*
  Serial.println(" ");
  Serial.print(F("H: "));
  Serial.print(h);
  Serial.print(F("%  T: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(F("HIc: "));
  Serial.print(hic);
  Serial.print(F("°C "));
*/
delay(1000);  // Delay for better readability
// MQ135 Zero point
MQ135 gasSensor = MQ135(A0); // Attach sensor to pin A0
float rzero = gasSensor.getRZero();
//float crzero = gasSensor.getCorrectedRZero(t, h);
float gcf = gasSensor.getCorrectionFactor(t, h);
float grsi = gasSensor.getResistance();
//float gcrsi = gasSensor.getCorrectedResistance(t, h);
int ppm = gasSensor.getPPM();
int cppm = gasSensor.getCorrectedPPM(t, h);
int Temp = t-16;



 //Adafruit OLED

  display.clearDisplay();
  display.setTextColor(WHITE);
/*
  display.setTextSize(1);
  display.setCursor(5, 0);
  display.println(rzero);   //("SmartFab Technologies");

   display.setCursor(60, 0);
  display.println(crzero); 

   display.setCursor(5, 10);
  display.println(grsi); 

   display.setCursor(60, 10);
  display.println(gcrsi); 

   display.setCursor(5, 20);
  display.println(ppm); 

   display.setCursor(60, 20);
  display.println(cppm); 

   display.setCursor(5, 30);
  display.println(gcf); 

 display.setCursor(5, 40);
  display.println(Temp); 

   display.setCursor(60, 40);
  display.println(h);
*/
 

  
  display.setTextSize(1); //Line 1
  display.setCursor(0, 0);
  display.println("Atmosphere Satellite");
  display.drawLine(0, 10, 128, 10, WHITE);
 
  display.setTextSize(2); //Line 2
  display.setCursor(0, 14);
  display.println(Temp);
  display.setCursor(30, 14);
  display.print((char)247); // degree symbol 
  display.println("C");

  display.setCursor(85, 14);
  display.print(h);
  display.println("%");
  display.drawLine(0, 31, 128, 31, WHITE);

  
  display.setTextSize(1); //Set Speed Status
  display.setCursor(0, 35);
  display.println(rzero);
  
  display.setTextSize(1.5); //Count Status
  display.setCursor(64, 35);
  display.println(grsi);

  display.setTextSize(2); //Line 2
  display.setCursor(0, 0);
  display.drawLine(0, 63, 48, 63, WHITE);

  display.setTextSize(2); //Line 2
  display.setCursor(0, 0);
  display.drawLine(64, 63, 128, 63, WHITE);

  display.setTextSize(2); //Motor Speed
  display.setCursor(0, 47);
  display.println(cppm);

  display.setTextSize(2); //Label Count
  display.setCursor(64, 47);
  display.println("NRML");

  

display.display();

 //unsigned long curruntTime = millis();

 
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(80);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(50);                       // wait for a second
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(80); 
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
}
