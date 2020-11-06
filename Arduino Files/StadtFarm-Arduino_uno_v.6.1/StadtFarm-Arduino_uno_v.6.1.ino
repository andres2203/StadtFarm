// To Do:
// todo: save: Time, Temp, Lux, on storage
// todo: get peristaltic pump for nutrition
// todo: install camera for analyzing and ai


//////////// StadtFarm ////////////
float program_version = 1.10;

// Useful Constants
#define MILISEC_PER_SEC (1000UL)
#define MILISEC_PER_MIN (MILISEC_PER_SEC * 60UL)
#define MILISEC_PER_HOUR (MILISEC_PER_MIN * 60UL)
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

int ResetPin = 12; //For Arduino Reset if Sensors are not working properly
//#include <Arduino.h>
// #include <SoftwareSerial.h>


//////////// RGB ////////////
#define LEDred 9 // for red color LED on Pin 9
#define LEDblue 11 // for blue color LED on Pin 11
#define LEDgreen 10 // for green color LED on Pin 10

int brightness1a = 50; // Nr between 0 and 255 for brightness of LED
int brightness1b = 150; 
int brightness1c = 250; 
int RGB_off = 0; // 0 for LED of = 0V
long RGBDelay_a = 200; //Blink frequency
long RGBDelay_b = 500;
bool RGBBlinkState = HIGH;
unsigned long RGBTimer = 0; // will be rewriten on every loop


//////////// Light Relais Module ////////////
#define Light_1 2 //This is the Arduino Pin that will control Light on Relay 1
#define Light_2 3
#define Light_3 4
#define Light_4 5

int day_start = 9; // time to start artificial light
int day_end = 20; // time to end artificial light
bool day_time = true;
unsigned long lux_limit = 400; // threshold for artificial light
bool light_status = LOW;
// long luxstate; // lux value for comparison
//volatile byte relayState = LOW;

//////////// Pump Switch Modules ////////////
#define pumpSwitch_1 8 // This is the Arduino Pin that will control pump switch
//#define pumpSwitch_2 3

unsigned long delayPump = MILISEC_PER_MIN * 60; // pump delay time between cycles in Min (positive numbers, 32bit)
unsigned long runnningPump = MILISEC_PER_MIN * 20; // running Timer for pumps in Min.
bool pumpState = HIGH; // bool for switch
unsigned long Timer = 0;

//////////// Clock RTC DS3231 ////////////
// Date and time functions using a DS3231 RTC connected via I2C and Wire lib, on SCL, SDA Pin
#include "RTClib.h"
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

//////////// Light Sensor BH1750 ////////////
//This is a simple code to test BH1750FVI Light senosr communicate using I2C Protocol this library enable 2 slave device address
//Main address  0x23, secondary address 0x5C, connect this sensor as following:
//VCC >>> 3.3V, SDA >>> A4 --> use of I2C Protocoll, can be added to RTC
//SCL >>> A5 --> use of I2C Protocoll, can be added to RTC
//addr >> A3, Gnd >>>Gnd
#include <BH1750FVI.h> // Sensor Library
#include <Wire.h> // I2C Library
//long light_sensor_sleep = 300000; // Sleep time between reads 5min (in milliseconds)
//stop_light 600;
BH1750FVI LightSensor;

//////////// EEPROM Storage ////////////
// stores analog values from 0-255
#include <EEPROM.h>  // include the internale arduino storage
int addr = 0;  // the current address in the EEPROM (i.e. which byte we're going to write to next)

//////////// Display ////////////
unsigned long DisplayTimer = 0; //will be set current time every loop
unsigned DisplayDelay = 2000L; //delay between reloading display

//////////// Reboot ////////////
unsigned long RebootTimer = 0;
unsigned long RebootDelay = MILISEC_PER_HOUR * 24 * 7; // reboot every 7 days, time in hours

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////SETUP///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  digitalWrite(ResetPin, HIGH); //Resets Arduino
  pinMode(ResetPin, OUTPUT);
  Serial.begin(9600);//initialize Serial Port
  #ifndef ESP8266
  while(!Serial); // wait for serial port to connect. Needed for native USB
  #endif

  Serial.println();
  Serial.print("Running StadtFarm Version ");
  Serial.println(program_version);
  Serial.println();
  delay(1000);

  setupPump();
  setupRGBLight();
  setupGrowLight();
  setupRTC();
  setupLightSensor();
  setupDisplay();
  
  setupDelay();
  setupReboot();
}

//////////// Light Sensor BH1750 ////////////
void setupLightSensor(){
  LightSensor.begin();
//Set the address for this sensor you can use 2 different address
//Device_Address_H "0x5C", Device_Address_L "0x23"
//you must connect Addr pin to A3 .
  LightSensor.SetAddress(Device_Address_H);//Address 0x5C
  // To adjust the slave on other address , uncomment this line
  // lightMeter.SetAddress(Device_Address_L); //Address 0x5C
/* set the Working Mode for this sensor
Select the following Mode:
Continuous_H_resolution_Mode
Continuous_H_resolution_Mode2
Continuous_L_resolution_Mode
OneTime_H_resolution_Mode
OneTime_H_resolution_Mode2
OneTime_L_resolution_Mode
The data sheet recommanded To use Continuous_H_resolution_Mode */
  LightSensor.SetMode(Continuous_H_resolution_Mode);
  Serial.println("starting Light Sensor");
  delay(500);
}

//////////// Clock RTC DS3231 ////////////
void setupRTC(){
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.println(); //print space
    Serial.flush();
    abort();
    Serial.println("Running Clock");
    Serial.println(); //print space
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    Serial.println(); //print space
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }

// When time needs to be re-set on a previously configured device, the
// following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //write current date here
// This line sets the RTC with an explicit date & time, for example to set
// January 21, 2020 at 3am you would call:
// rtc.adjust(DateTime(2020, 10, 26, 8, 44, 0)); //Year, Month, Day, Hour, Min
  Serial.println("starting Time module...");
  delay(500);
}

//////////// Light Relais Module ////////////
void setupGrowLight(){
  pinMode(Light_1, OUTPUT);
  pinMode(Light_2, OUTPUT);
  pinMode(Light_3, OUTPUT);
  pinMode(Light_4, OUTPUT);
  
  digitalWrite(Light_1, light_status);
  digitalWrite(Light_2, light_status);
  digitalWrite(Light_3, light_status);
  digitalWrite(Light_4, light_status);
  Serial.println("starting Grow Light module");
  delay(500);
}

//////////// Pump Switch Modules ////////////
void setupPump() {
  pinMode(pumpSwitch_1, OUTPUT);
//  pinMode(pumpSwitch_2, OUTPUT);

  Timer = millis();
  Serial.println("starting pump managment");
  delay(500);
}

//////////// RGB ////////////
void setupRGBLight(){
  pinMode(LEDred, OUTPUT);
  pinMode(LEDgreen, OUTPUT);
  pinMode(LEDblue, OUTPUT);
  RGBTimer = millis();
}

//////////// Display ////////////
void setupDisplay(){
  DisplayTimer = millis();
}

//////////// Delay ////////////
void setupDelay(){
//  delay(5000);
}

//////////// Reboot ////////////
void setupReboot(){
  RebootTimer = millis(); //sets current time to schedule reboot, this is done continuously for system stability
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////LOOP//////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  DateTime now = rtc.now();
  if(day_start <= now.hour() && day_end >= now.hour()){
    day_time = true;
  }else{
    day_time = false;
  }
  
  pumpManagement();
  LightSensorModule();
  RGBLight();
  GrowLight();
  loop_RTC();
  EEPROM_storage();
  Alarms();
  Display();
  
  RebootSystem();
}

//////////// Light Sensor BH1750 ////////////
void LightSensorModule() {
  uint16_t lux = LightSensor.GetLightIntensity();// Get Lux value
  if (lux == 0 || lux > 54000){
    Serial.print("Light Sensor might not working properly "); // Checks if Sensoroutput is to low or to hight and resets Arduino
    delay(10);
    Serial.println();
    Serial.print("resetting...");
    delay(2000); // wait 2 sec before resetting
    digitalWrite(ResetPin, LOW); //Resets Arduino
    }
  //LightSensor.Sleep(); //Sensor in Sleep Mode

  // if (lux >= 500) {
  //  send(msg.set(lux));
  //  LightSensor.end();
}

//////////// Grow Light Relais Module ////////////
void GrowLight(){
  DateTime now = rtc.now();
 
  uint16_t lux = LightSensor.GetLightIntensity();// Get Lux value

  if (day_time && lux <= lux_limit){
    light_status = HIGH;
    digitalWrite(Light_1, light_status);     //Switch Relay #2 On
    digitalWrite(Light_2, light_status);     //Switch Relay #3 On
    digitalWrite(Light_3, light_status);     //Switch Relay #4 On
    digitalWrite(Light_4, light_status);     //Switch Relay #5 On
  }else {
    light_status = LOW;
    digitalWrite(Light_1, light_status);     //Switch Relay #2 OFF (in NO (Normally open) Mode)
    digitalWrite(Light_2, light_status);     //Switch Relay #3 OFF (in NO (Normally open) Mode)
    digitalWrite(Light_3, light_status);     //Switch Relay #4 OFF (in NO (Normally open) Mode)
    digitalWrite(Light_4, light_status);     //Switch Relay #5 OFF (in NO (Normally open) Mode)
    }
}


//////////// Loop for RTC Module ////////////
// set summer and winter timer
void loop_RTC(){
  DateTime now = rtc.now();
// DST 
//  setclock 
  
}

//daysOfTheWeek


//////////// Pump Switch Modules ////////////
void pumpManagement() {
  digitalWrite(pumpSwitch_1, pumpState);

  if (pumpState == HIGH){
    if((millis() - Timer) >= runnningPump){
      pumpState = !pumpState;
      Timer = millis();
      }
  }else{
    if((millis() - Timer) >= delayPump){
      pumpState = !pumpState;
      Timer = millis();
      }
    }
}

//////////// System RGB ////////////
void RGBLight(){
  if (pumpState == HIGH){
    if ((millis() - RGBTimer) > RGBDelay_b){
      RGBTimer = millis();
      RGBBlinkState = !RGBBlinkState;
      digitalWrite(LEDblue, RGBBlinkState); // led on
    }
  }else {
    digitalWrite(LEDblue, LOW);
    }
}

//////////// Display ////////////
void Display(){
  if (millis() - DisplayTimer > DisplayDelay){
    DisplayTimer = millis();
    _Display();
  }
}


void _Display(){
  uint16_t lux = LightSensor.GetLightIntensity();  // Get Lux value
//  DateTime now = rtc.now();
  RTCtime();
  if (day_time){
    Serial.print("StadtFarm in day mode");
    if(lux > lux_limit){
      Serial.println(": LED-Lights off");
    }else{
      Serial.println();
    }
    }else{
    Serial.println("StadtFarm in Night Mode");
  }
  
  Serial.print("Light intensity: ");
  Serial.print(lux);
  Serial.println(" lux");

  Serial.print("Temperature: ");
  Serial.print(rtc.getTemperature());
  Serial.println(" C");

  if(pumpState == HIGH){
    Serial.println("watering Pump running... ");
  }
}

//////////// EEPROM_storage ////////////
void EEPROM_storage(){
  int temperature_in = rtc.getTemperature() / 4;
  
  EEPROM.write(addr, temperature_in);  // write temperature in storage
  addr = addr + 1;
// EEPROM.write(addr, 

  if(addr == EEPROM.length()){
    addr = 0;
  }
}

//////////// Alarms  ////////////
void Alarms(){
  if (rtc.getTemperature() >= 80){
    Serial.println("Board Temperature exceeded critical limit of 80°C, StadtFarm will be stopped for 5 minutes (current Temperature: ");
    Serial.print(rtc.getTemperature());
    Serial.println(" C");
    delay(5000);
  }
}

//////////// Reboot ////////////
void RebootSystem(){
  if((millis() - RebootTimer) > RebootDelay){
    RebootTimer = millis();
    Serial.println();
    Serial.println("StadtFarm ist rebooting, this is a routine operation, please waite... ");
    delay(10000);
    digitalWrite(ResetPin, LOW); //Resets Arduino
  }
}

//////////// Clock RTC DS3231 ////////////
void RTCtime(){
  Serial.println();
  DateTime now = rtc.now();
  
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(", ");    
  Serial.print(now.day(), DEC);  
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.year(), DEC);
  Serial.print("  Time:");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
}


  
/*
analogWrite(LEDred, brightness1a); // red led on
  delay(Delay_long_a); // pause
  analogWrite(LEDred, RGB_off); // led off
  delay(Delay_long_a); // pause
  analogWrite(LEDred, brightness1a); // led on
  delay(Delay_long_a); // pause
  analogWrite(LEDred, RGB_off); // led off
  analogWrite(LEDgreen, brightness1a); 
  delay(Delay_long_a); // pause
  analogWrite(LEDgreen, RGB_off); // led off
  delay(Delay_long_a); // pause
  analogWrite(LEDgreen, brightness1a); // led on
  delay(Delay_long_a); // pause
  analogWrite(LEDgreen, RGB_off); // led off
  analogWrite(LEDblue, brightness1a); // led on
  delay(Delay_long_a); // pause
  analogWrite(LEDblue, RGB_off); // led off
  delay(Delay_long_a); // pause
  analogWrite(LEDblue, brightness1a); // led on
  delay(Delay_long_a); // pause
  analogWrite(LEDblue, RGB_off); // led off


analogWrite(LEDred, brightness1a); // yellow light
analogWrite(LEDgreen, brightness1a); 
delay(Delay_long_a); //pause
analogWrite(LEDred, RGB_off); // led off
analogWrite(LEDgreen, RGB_off);
//    analogWrite(LEDblue, dunkel); // led off
//    delay(p); // pause
analogWrite(LEDblue, brightness1a);
delay(Delay_long_a); //pause
analogWrite(LEDblue, RGB_off); // led off
//    delay(p); // pause
//    analogWrite(LEDgreen, brightness1a);
//    delay(p); // pause
//    analogWrite(LEDred, RGB_off); // red off

*/
