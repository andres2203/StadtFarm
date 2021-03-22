
/*
- see if pump stops after 2.5 min

//To Do:
  Sensor_Error = true;  // has to be written in EEPROM for sys reboot when rtc not found
- light sensor temporary deactivated in void loop 05.01.21
- Grow_Light is flickering by night, some electricity pulses?!?
- capac. sensor just for safty
- fs_light sensor: how tho see if light sensor broken, as no feedback, maybe waiting for signal time?
- set summer and winter timer, just add dates in table, set time and refere to command, ex: daysOfTheWeek
- Error_2: add Pump defect, start Timer to see how long it takes to make refil of tank: if Timer < Countdown 5 min 
- Pump timer optimized on 2. shelve!
- after integrating lightsensor inside shelfe add alarm: "if GrowLight == True && Lux < 1000, GrowLight defekt or Lux Sensor defekt (if no signal)"
-- add piep to alarm mode

// Next StadtFarm Generation
- get peristaltic pump for nutrition
- install camera for analyzing and ai
- todo: save: Time, Temp, Lux, on storage
- add (capacitive) level control 
*/

//////////// StadtFarm ////////////
float program_version = 1.001; //
#include <Arduino.h>  // to convert Arduino file to C++

// Pin configuration
#define LEDred 9  // for red color LED on Pin 9
#define LEDblue 11  // for blue color LED on Pin 11
#define LEDgreen 10  // for green color LED on Pin 10
// integer for level control in water storage tank, 0=OK, 1=empty, Pin 13
#define Light_1 2 //This is the Arduino Pin that will control Light on Relay 1
#define Light_2 3
#define Light_3 4
#define Light_4 5
#define ResetPin 12  //For Arduino Reset if Sensors are not working properly


// Aricle Bumbers for replacement and guarantee
int Capac_ArticleNr = 001;  // has to be difined
int GrowLight_ArticleNr = 002;
// int 

// Timer, Useful Constants and Countdown
#include "CountDown.h"  // add the Countdown library
CountDown CD(CountDown::MINUTES);
int CD_Days = 0;  //time values for countdown
int CD_Hours = 0;
int CD_Minutes = 5;  // hytseresis timer to see how long it takes until waterpump starts or maxwater level is reached after capacity value exceeded
int CD_Seconds = 0;
//bool CDbool = true;
//uint32_t start, stop;
#define MILISEC_PER_SEC (1000UL)
#define MILISEC_PER_MIN (MILISEC_PER_SEC * 60UL)
#define MILISEC_PER_HOUR (MILISEC_PER_MIN * 60UL)
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

// Fail and fail-safe parameters
bool Error_1 = false;  // for red LED bling: Moisture-Sensor, Moisture-Switch Error or Outlet is clogged
bool Error_2 = false;  // for red LED continouse: Pump defect, system Temp exceeded 80C
bool Sensor_Error = false;  //for fail-safe operation
int Liquid_level = 0;  // integer for level control in water storage tank, 0=OK, 1=empty, Pin 13, if empty pump stops


//////////// RGB ////////////
int brightness1a = 50;  // Nr between 0 and 255 for brightness of LED
int brightness1b = 150; 
int brightness1c = 250; 
int RGB_off = 0; // 0 for LED of = 0V
long RGBDelay_a = 200;  //Blink frequency
long RGBDelay_b = 500;
bool RGBBlinkState = HIGH;
unsigned long RGBTimer = 0;  // for watering Pump, will be rewriten on every loop
unsigned long RGBTimer_2 = 0;  // timer for Error LED

//////////// Light Relais Module ////////////
int day_start = 12;  // time to start artificial light
int day_end = 22;  // time to end artificial light
int lightning_germs_1 = 4;  // need less sunlight, are more delicate
//int lightning_germs_2 = 6;
//int lightning_plant_1 = 8;
//int lightning_plant_2 = 10;
bool day_time = true;
bool light_switch = false;  // for additonal button to switch light on (maintenance ore else), not working properly!!
unsigned long lux_limit = 400;  // threshold for artificial light
bool light_status = LOW;
// long luxstate;  // lux value for comparison

//////////// Pump Switch Modules ////////////
#define pumpSwitch_1 8 // This is the Arduino Pin that will control pump switch
//#define pumpSwitch_2 3

unsigned long delayPump = MILISEC_PER_MIN * 20; // pump delay time between cycles in Min (positive numbers, 32bit)
unsigned long runnningPump = MILISEC_PER_MIN * 2; // running Timer for pumps in Min. -> security Timer if capacity sensor doesnt work
bool pumpState = HIGH; // bool for switch
unsigned long PumpTimer = 0;

//////////// Moisture ////////////
const int Capac_Water = 320;  // Capacity Senosr value for water
const int Capac_Air = 615;  // Capacity Senosr value for air
const int Capac_WaterValue_Min = 540;  // Dry, Limit moisture, start pump
const int Capac_WaterValue_Full = 390;  // Limit for max Water level for filling

//int Capac_timer_threshold = 20 * SECS_PER_MIN; // 20 minutes, time is need until sensor measures water (could be last shelve)
int SoilMoistureValue = 0;
int hysteresis_Capac_Sensor = 0.05; // 5% histeresis for error management

//////////// Clock RTC DS3231 ////////////
// Date and time functions using a DS3231 RTC connected via I2C and Wire lib, on SCL, SDA Pin
#include <RTClib.h>

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

//////////// Light Sensor BH1750 ////////////
//This is a simple code to test BH1750FVI Light senosr communicate using I2C Protocol this library enable 2 slave device address
//Main address  0x23, secondary address 0x5C, connect this sensor as following:
//VCC >>> 3.3V, SDA >>> A4 --> use of I2C Protocoll, can be added to RTC
//SCL >>> A5 --> use of I2C Protocoll, can be added to RTC
//addr >> A3, Gnd >>>Gnd
#include <BH1750FVI.h>  // Sensor Library

#include <Wire.h> // I2C Library
//#include <TinyWireM.h>  // ATtiny (e.g. Adafruit Trinket, Gemma) I2C library,
//#include <USI_TWI_Master.h>

//long light_sensor_sleep = 300000; // Sleep time between reads 5min (in milliseconds)
//stop_light 600;
BH1750FVI LightSensor;

//////////// EEPROM Storage ////////////  - not in use
// stores analog values from 0-255
//#include <EEPROM.h>  // include the internale arduino storage
//int addr = 0;  // the current address in the EEPROM (i.e. which byte we're going to write to next)

//////////// Display ////////////
unsigned long DisplayTimer = 0;  // will be set current time every loop
unsigned DisplayDelay = 10 * MILISEC_PER_SEC;  // delay between print in sec.

//////////// Reboot ////////////
unsigned long RebootTimer = 0;
unsigned long RebootDelay = MILISEC_PER_HOUR * 24 * 7;  // reboot every 7 days, time in hours

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////SETUP///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  digitalWrite(ResetPin, HIGH);  // Resets Arduino
  pinMode(ResetPin, OUTPUT);
  Serial.begin(9600);  // initialize Serial Port

#ifndef ESP8266
  while(!Serial);  // wait for serial port to connect. Needed for native USB
#endif
  

  if(Sensor_Error){  // fail safe operations, all sensors off!
    Serial.println();
    Serial.print("StadtFarm in Emergency Mode, all sensors off. Please check moisture, water level in storage tank occasionally");
    delay(300);
    setupPump();  // water level has to be controlled manually
    setupRGBLight();
    setupGrowLight();
  }
  else{  // full operational mode
    Serial.println();
    Serial.print("Running StadtFarm Version ");
    Serial.println(program_version);
    Serial.println();
    delay(1000);
    setupPump();
    setupRGBLight();
    setupLightSensor();
    setupGrowLight();
    setupRTC();
    setupWaterLevel();  // water level sensor in storage tank
    
  }
  RebootTimer = millis(); //sets timer to schedule reboot, this is done continuously for system stability
  CD.start(CD_Days, CD_Hours, CD_Minutes, CD_Seconds);  // initiate count down
  DisplayTimer = millis();  // reset Timer for Display
}

//////////// Pump Switch Modules ////////////
void setupPump(){
  pinMode(pumpSwitch_1, OUTPUT);
//  pinMode(pumpSwitch_2, OUTPUT);
  Serial.println("Starting Pump Management");
  delay(300);
  PumpTimer = millis();
  
}

//////////// RGB ////////////
void setupRGBLight(){
  pinMode(LEDred, OUTPUT);
  pinMode(LEDgreen, OUTPUT);
  pinMode(LEDblue, OUTPUT);
  RGBTimer = millis();
}

//////////// Light Relais Module ////////////
void setupGrowLight(){
  pinMode(Light_1, OUTPUT);
  pinMode(Light_2, OUTPUT);
  pinMode(Light_3, OUTPUT);
  pinMode(Light_4, OUTPUT);
  
  Serial.println("Starting Grow Light Module");
  delay(300);
}

//////////// Clock RTC DS3231 ////////////
void setupRTC(){
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
//    Sensor_Error = true;  // has to be written in EEPROM
//    Serial.println("resetting arduino");
//    delay(1000);
//    digitalWrite(ResetPin, LOW); //Resets Arduino
    Serial.println();  // print space
    Serial.flush();
    abort();
    Serial.println("Running Clock");
    Serial.println();  // print space
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    Serial.println(); //print space
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));  //Year, Month, Day, Hour, Min
    }

// When time needs to be re-set on a previously configured device, the
// following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //write current date here  (YYYY, MM, DD, H, MM, S)
// This line sets the RTC with an explicit date & time, for example to set
// January 21, 2020 at 3am you would call:
// rtc.adjust(DateTime(2021, 02, 05, 00, 14, 0));
  Serial.println("Starting Time Module");
  delay(300);
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
  Serial.println("Starting Light Sensor");
  delay(300);
}

//////////// Water Level ////////////
void setupWaterLevel(){
  pinMode(10, INPUT);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////LOOP//////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  RGBLight();
  Display();
  AlarmMode();
  RebootSystem();
  if (! Sensor_Error) {
    DateTime now = rtc.now();  // get current time
    LightSensorReboot();
    PumpMoistustureManagement();
//  RTCSetTime();  // Set Summer/Winter time
    GrowLight();
//  EEPROM_storage();  // temporary not uesd
    Liquid_level = digitalRead(13);  // read pin 13 for water storage tank
    
    if(day_start <= now.hour() && day_end >= now.hour()) {
      day_time = true;  // switch Grow light on
    }else {
      day_time = false;  // switch grow light off
    }
  }
  else {  // fail-safe (fs) operational mode
    fs_pump_management();
    fs_Grow_Light();
  }
}

//////////// System RGB ////////////
void RGBLight(){
  if (Sensor_Error){
    digitalWrite(LEDred, HIGH);
  }
  else {
    if (pumpState == HIGH) {
      if ((millis() - RGBTimer) > RGBDelay_b) {
        RGBTimer = millis();
        RGBBlinkState = !RGBBlinkState;  // for LED blinking
        digitalWrite(LEDblue, RGBBlinkState);  // led on
      }
    }
    else {
      digitalWrite(LEDblue, LOW);
    }
    if (Error_1) {
      if ((millis() - RGBTimer_2) > RGBDelay_b) {
        RGBTimer_2 = millis();
        RGBBlinkState = !RGBBlinkState;
        digitalWrite(LEDred, RGBBlinkState);
       }
    }
    else {
      digitalWrite(LEDred, LOW);
    }
    }
}
//////////// Display ////////////
void Display(){
  if (millis() - DisplayTimer > DisplayDelay){
    DisplayTimer = millis();
    if(Sensor_Error){
      _Display_fail_safe;
    }else {
      _Display_all();
      }
  }
}

void _Display_fail_safe(){
  if (Sensor_Error){
    Serial.println("StadtFarm in fail-safe mode, please replace Sensors");
  }
  if (light_status == HIGH){
    Serial.println("StadtFarm in day mode, grow light on");
  }else {
    Serial.println("StadtFarm in Night Mode");
  }
  if(pumpState == HIGH){
    Serial.print("watering Pump running... ");
    Serial.println();
  }
}

void _Display_all(){
  uint16_t lux = LightSensor.GetLightIntensity();  // Get Lux value
  DateTime now = rtc.now();
  Serial.println();
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
//  Serial.print(':');
//  Serial.print(now.second(), DEC);
  Serial.println();

 
  if (day_time){
    Serial.print("StadtFarm in day mode");
    if(lux > lux_limit){
      Serial.print(": LED-Lights off");
    }
  }else {
    Serial.println("StadtFarm in Night Mode");
  }
    Serial.println();
    Serial.print("Moisture is ");  // Moisture and Pump state
    Serial.print(SoilMoistureValue);  //print the value to serial port
    Serial.print("%RH, ");
    if (Capac_WaterValue_Full <= SoilMoistureValue && SoilMoistureValue <= Capac_WaterValue_Min){
      Serial.println("Moisture is good");
      Error_1 = false;
    }
    if (SoilMoistureValue > Capac_WaterValue_Min) {
      CD.start(CD_Days, CD_Hours, CD_Minutes, CD_Seconds);
      if (CD.remaining() > 0) {
        Serial.println("Moisture is very low");
        Error_1 = false;
      }
      else{
        Error_1 = true;     
        if (pumpState) {
          Serial.println("Pump is still running, pump switch failure or watering pipe clogged");
        }
        else{
          Serial.println("Moistur Sensor failure"); //, please replace ArticleNr: ");   
        }
//          Serial.print(Capac_ArticleNr);
      }
    }
    if (pumpState == HIGH) {
      Serial.println("Pump ist watering");
      }
    if (SoilMoistureValue < Capac_WaterValue_Full) {
      CD.start(CD_Days, CD_Hours, CD_Minutes, CD_Seconds);
      if (CD.remaining() > 0) {
        Serial.println("Moisture is very high");
        Error_1 = false;
      }
      else{
        Serial.println("Moistur Sensor failure or Outlet clogged");
        Error_1 = true;
//    Serial.print(Capac_ArticleNr);
    }
}

  if (Error_2){
    Serial.println("Pump defect, please replace");
  }

//Light mode
  Serial.print("Light intensity: ");
  Serial.print(lux);
  Serial.println(" lux");
  Serial.print("Temperature: ");
  Serial.print(rtc.getTemperature());
  Serial.println(" C");

  if (Liquid_level == 1) {
    Serial.println("Water level is low, please refill water and nutrient solution (recomemndation: clean water tank)");
  }
}

//////////// Alarms  ////////////
void AlarmMode() {
  if (rtc.getTemperature() >= 80){  // Temperature Alarm, get Temp via RTC board
    Serial.println("Board Temperature exceeded critical limit of 80°C, StadtFarm will be stopped for 5 minutes (current Temperature: ");
    Serial.print(rtc.getTemperature());
    Serial.println(" C");
    Error_2 = true;
    delay(5000);
  } else{
    Error_2 = false;
  }
}


//////////// Reboot ////////////
void RebootSystem() {
  if((millis() - RebootTimer) > RebootDelay){
    RebootTimer = millis();
    Serial.println();
    Serial.println("StadtFarm ist rebooting, this is a routine operation, please waite... ");
    delay(10000);
    digitalWrite(ResetPin, LOW); //Resets Arduino
  } else {
  }
}

//////////// Light Sensor BH1750 ////////////
void LightSensorReboot() {
  uint16_t lux = LightSensor.GetLightIntensity();  // Get Lux value
  if (day_time && (lux == 0 || lux > 54000)){
    Serial.print("Light Sensor might not working properly ");  // Checks if Sensoroutput is to low or to hight and resets Arduino
    delay(10);
    Serial.println();
    Serial.print("resetting...");
    delay(2000);  // wait 2 sec before resetting
    digitalWrite(ResetPin, LOW);  //Resets Arduino
    }
  //LightSensor.Sleep(); //Sensor in Sleep Mode
  // if (lux >= 500) {
  //  send(msg.set(lux));
  //  LightSensor.end();
}

//////////// Moisture ////////////
void PumpMoistustureManagement() {
  SoilMoistureValue = analogRead(0); // connect sensor to Analog 0
  digitalWrite(pumpSwitch_1, pumpState);
  if (Liquid_level == 0) {
    if (pumpState == HIGH && (millis() - PumpTimer) >= runnningPump){
      pumpState = LOW;
      PumpTimer = millis();
      }
    if (pumpState == LOW && (millis() - PumpTimer) >= delayPump){
      pumpState = HIGH;
      PumpTimer = millis();
   }
  }
  else {
    pumpState = LOW;
  }

}
  
/* 
  if (SoilMoistureValue <= Capac_WaterValue_Full){
    pumpState = LOW;
  } 
  if (SoilMoistureValue >= Capac_WaterValue_Min){
    if((millis() - PumpTimer) >= runnningPump){
      pumpState = HIGH;
      PumpTimer = millis();
    }
 }
}

*/

/*
//////////// Loop for RTC Module ////////////
// set summer and winter timer
void RTCSetTime(){
  DateTime now = rtc.now();
  time_t t = now();
  RTC_year = year(t);
  RTC_month = month(t);
  RTC_day = day(t);
  RTC_hour = hour(t);
  RTC_minute = minute(t);
  RTC_second = second(t);
  if (RTC_month == 3 || RTC_month == 
// DST 
//  setclock 
//}
*/

//////////// Grow Light Relais Module ////////////
void GrowLight(){
  DateTime now = rtc.now();
  uint16_t lux = LightSensor.GetLightIntensity();  // Get Lux value

    digitalWrite(Light_1, light_status);  //Switch Relay #2 On
    digitalWrite(Light_2, light_status);  //Switch Relay #3 On
    digitalWrite(Light_3, light_status);  //Switch Relay #4 On
    digitalWrite(Light_4, light_status);  //Switch Relay #5 On

  if (light_switch || (day_time == true && lux <= lux_limit)) {
    light_status = HIGH;
//    Serial.println();
//    Serial.print("light on");
  } else {
    light_status = LOW;
//    Serial.println("light off");
  }
}

/*
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
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////// fail-safe Pump Switch Modules ////////////
void fs_pump_management() {
  digitalWrite(pumpSwitch_1, pumpState);
  if (pumpState == HIGH){
    if((millis() - PumpTimer) >= runnningPump){
      pumpState = !pumpState;
      PumpTimer = millis();
      }
  }else{
    if((millis() - PumpTimer) >= delayPump){
      pumpState = !pumpState;
      PumpTimer = millis();
      }
    }
}

//////////// fail-safe Grow Light Relais Module ////////////
void fs_Grow_Light(){
    digitalWrite(Light_1, HIGH);  //Switch Relay #2 On
    digitalWrite(Light_2, HIGH);  //Switch Relay #3 On
    digitalWrite(Light_3, HIGH);  //Switch Relay #4 On
    digitalWrite(Light_4, HIGH);  //Switch Relay #5 On
}
