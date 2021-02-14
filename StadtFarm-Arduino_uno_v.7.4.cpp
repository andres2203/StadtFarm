  /*
- see if pump stops after 2.5 min

//To Do:
- light sensor temporary deactivated in void loop 05.01.21
- level control for storage tank
  - build potentiometer https://create.arduino.cc/projecthub/zezarandrade/tank-control-with-arduino-a4d47f
  - capacity measurement by copper wire (20€)
- Grow_Light is flickering by night, some electricity pulses?!?
- set summer and winter timer, just add dates in table, set time and refere to command, ex: daysOfTheWeek
- Error_2: add Pump defect, start Timer to see how long it takes to make refil of tank: if Timer < Countdown 5 min 
- Pump timer optimized on 2. shelve!
-- add piep to alarm mode

// Next StadtFarm Generation
- get peristaltic pump for nutrition
- install camera for analyzing and ai
- todo: save: Time, Temp, Lux, on storage
- add (capacitive) level control 
*/

//////////// StadtFarm ////////////
float program_version = 1.00;
#include <Arduino.h>  // to convert Arduino file to C++

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
#define ResetPin 12  //For Arduino Reset if Sensors are not working properly

//////////// RGB ////////////
#define LEDred 9  // for red color LED on Pin 9
#define LEDblue 11  // for blue color LED on Pin 11
#define LEDgreen 10  // for green color LED on Pin 10

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
#define Light_1 2 //This is the Arduino Pin that will control Light on Relay 1
#define Light_2 3
#define Light_3 4
#define Light_4 5

int day_start = 10;  // time to start artificial light
int day_end = 20;  // time to end artificial light
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

unsigned long delayPump = MILISEC_PER_HOUR * 2; // pump delay time between cycles in hours (positive numbers, 32bit)
unsigned long runnningPump = MILISEC_PER_MIN * 1.5; // running Timer for pumps in Min. -> security Timer if capacity sensor doesnt work
bool pumpState = HIGH; // bool for switch
unsigned long Timer = 0;

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
#include <BH1750FVI.h> // Sensor Library

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
  digitalWrite(ResetPin, HIGH);  //Resets Arduino
  pinMode(ResetPin, OUTPUT);
  Serial.begin(9600);  //initialize Serial Port
  #ifndef ESP8266
  while(!Serial);  // wait for serial port to connect. Needed for native USB
  #endif
  Serial.println();
  Serial.print("Running StadtFarm Version ");
  Serial.println(program_version);
  Serial.println();
  delay(1000);
  setupRGBLight();
  setupPump();
  setupGrowLight();
  setupRTC();
  setupLightSensor();
  setupDisplay();
  
  setupDelay();
  setupReboot();
}
//////////// Light Sensor BH1750 ////////////
void setup_Light_Sensor() {
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
//////////// Clock RTC DS3231 ////////////
void setup_RTC() {
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.println();  // print space
    Serial.flush();
    Sensor_Error = true;
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
 rtc.adjust(DateTime(2021, 02, 05, 00, 14, 0));
  Serial.println("Starting Time Module");
  delay(300);
}

//////////// Light Relais Module ////////////
void setup_Grow_Light() {
  pinMode(Light_1, OUTPUT);
  pinMode(Light_2, OUTPUT);
  pinMode(Light_3, OUTPUT);
  pinMode(Light_4, OUTPUT);
  
  Serial.println("Starting Grow Light Module");
  delay(300);
}
//////////// Pump Switch Modules ////////////
void setup_Pump() {
  pinMode(pumpSwitch_1, OUTPUT);
//  pinMode(pumpSwitch_2, OUTPUT);
  Serial.println("Starting Pump Management");
  delay(300);
  Timer = millis();
}
//////////// RGB ////////////
void setup_RGB_Light() {
  pinMode(LEDred, OUTPUT);
  pinMode(LEDgreen, OUTPUT);
  pinMode(LEDblue, OUTPUT);
  RGBTimer = millis();
}
//////////// Display ////////////
void setup_Display() {
  DisplayTimer = millis();
}
//////////// Delay ////////////
void setup_Delay() {
//  delay(5000);
}
//////////// Count Down ////////////
void setup_Count_Down() {
  CD.start(CD_Days, CD_Hours, CD_Minutes, CD_Seconds);
}
//////////// Reboot ////////////
void setup_Reboot(){
  RebootTimer = millis(); //sets current time to schedule reboot, this is done continuously for system stability
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////LOOP//////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  RGB_Light();
  Alarms_mode();
  Display();
  Count_Down();
  Reboot_System();
  if (Sensor_Error) {  // fail-safe (fs) operational mode
    fs_pump_management();
    fs_Grow_Light();
  }else {
      Light_Sensor_Reboot();
      Pump_Moistusture_Management();
//    RTC_set_summer_winter_time();
//    EEPROM_storage();  // temporary not uesd
      DateTime now = rtc.now();
      if(day_start <= now.hour() && day_end >= now.hour()) {
        day_time = true;
      }
      else {
      day_time = false;
      }
  }
}
//////////// Light Sensor BH1750 ////////////
void Light_Sensor_Reboot() {
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
//////////// Grow Light Relais Module ////////////
void Grow_Light() {
  DateTime now = rtc.now();
  uint16_t lux = LightSensor.GetLightIntensity();  // Get Lux value

    digitalWrite(Light_1, light_status);  //Switch Relay #2 On
    digitalWrite(Light_2, light_status);  //Switch Relay #3 On
    digitalWrite(Light_3, light_status);  //Switch Relay #4 On
    digitalWrite(Light_4, light_status);  //Switch Relay #5 On

  if (light_status){
    light_status = HIGH;
  } else if (day_time == true && lux <= lux_limit){
    light_status = HIGH;
  } else {
    light_status = LOW;
  }
}

//////////// fail-safe Grow Light Relais Module ////////////
void fs_Grow_Light() {
    digitalWrite(Light_1, HIGH);  //Switch Relay #2 On
    digitalWrite(Light_2, HIGH);  //Switch Relay #3 On
    digitalWrite(Light_3, HIGH);  //Switch Relay #4 On
    digitalWrite(Light_4, HIGH);  //Switch Relay #5 On
}
/*
//////////// Loop for RTC Module ////////////
// set summer and winter timer
void RTC_set_summer_winter_time(){
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
//////////// Moisture ////////////
void Pump_Moistusture_Management() {
  SoilMoistureValue = analogRead(0); // connect sensor to Analog 0
  digitalWrite(pumpSwitch_1, pumpState);
  if (SoilMoistureValue <= Capac_WaterValue_Full){
    pumpState = LOW;
  } 
  if (SoilMoistureValue >= Capac_WaterValue_Min){
    if((millis() - Timer) >= runnningPump){
      pumpState = HIGH;
      Timer = millis();
    }
 }
}
//////////// fail-safe Pump Switch Modules ////////////
void fs_pump_management() {
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
void RGB_Light() {
  if (Sensor_Error){
    digitalWrite(LEDred, HIGH);
  }else{
    if (pumpState == HIGH){
      if ((millis() - RGBTimer) > RGBDelay_b){
        RGBTimer = millis();
        RGBBlinkState = !RGBBlinkState;  // for LED blinking
        digitalWrite(LEDblue, RGBBlinkState);  // led on
      } else {
//        digitalWrite(LEDblue, LOW);
      }
    }
    if (Error_1){
      if ((millis() - RGBTimer_2) > RGBDelay_b){
        RGBTimer_2 = millis();
        RGBBlinkState = !RGBBlinkState;
        digitalWrite(LEDred, RGBBlinkState);
       }
    }
    }
  }
}
//////////// CountDown ////////////
void Count_Douwn() {
  CD.start(CD_Days, CD_Hours, CD_Minutes, CD_Seconds);
}

//////////// Display ////////////
void Display() {
  if (millis() - DisplayTimer > DisplayDelay){
    DisplayTimer = millis();
    if(Sensor_Error){
      _Display_fail_safe
    }else {
      _Display_all();
      }
  }
}

void _Display_fail_safe() {
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

void _Display_all() {
  uint16_t lux = LightSensor.GetLightIntensity();  // Get Lux value
//  DateTime now = rtc.now();
  RTCtime();
  if (day_time){
    Serial.println("StadtFarm in day mode");
    if(lux > lux_limit){
      Serial.print(": LED-Lights off");
    }
  }else {
    Serial.println("StadtFarm in Night Mode");
  }
// Moisture and Pump state
    Serial.print("Moisture is ");
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
}

//////////// EEPROM_storage ////////////
/*
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

//////////// Alarms  ////////////
void Alarms_mode() {
  if (rtc.getTemperature() >= 80){
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
void Reboot_System() {
  if((millis() - RebootTimer) > RebootDelay){
    RebootTimer = millis();
    Serial.println();
    Serial.println("StadtFarm ist rebooting, this is a routine operation, please waite... ");
    delay(10000);
    digitalWrite(ResetPin, LOW); //Resets Arduino
  } else {
  }
}

//////////// Clock RTC DS3231 ////////////
void RTCtime() {
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
//  Serial.print(':');
//  Serial.print(now.second(), DEC);
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



Contribution to:   
Rob Tillaart, CountDown library
https://github.com/RobTillaart/CountDow

RTClib
https://github.com/adafruit/RTClib

*/
