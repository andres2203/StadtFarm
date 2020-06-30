//StadtFarm ver. 1.0
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Other
    int ResetPin = 12; //For Arduino Reset if Sensors are not working properly
//    #include <Arduino.h>
//    #include <SoftwareSerial.h>


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RGB

    int LEDred = 9; // for red color LED on Pin 9
    int LEDblue = 11; // for blue color LED on Pin 11
    int LEDgreen= 10; // for green color LED on Pin 10

    int brightness1a = 50; // Nr between 0 and 255 for brightness of LED
    int brightness1b = 50; 
    int brightness1c = 50; 
    int RGB_off = 0; // 0 for LED of = 0V


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Light Relais Module

    int Light_1 = 2; //This is the Arduino Pin that will control Light on Relay 1
    int Light_2 = 3;
    int Light_3 = 4;
    int Light_4 = 5;
    int SunlightRelais = Light_1 && Light_2 && Light_3 && Light_4; //controll all Light relais at once
    long luxstate; // lux value for comparison
    volatile byte relayState = LOW;

// Timer Variables
    long lastDebounceTime = 0;  
    long Delay_long_a = 500; // delay 0,5 sec
    long Delay_long_b = 1000; // delay 1 sec
    long Delay_long_c = 10000; // delay 10 sec
    long Delay_long = 10000;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pump Relais Module

    int PumpRelay = 7; // This is the Arduino Pin that will control Relay
    long pumpdelay = 5000; // delay of pumping 5sec

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Clock RTC DS3231

// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
// SCL = A4, SDA = A5 // Arduino UNO on SCL, SDA Pin
    #include "RTClib.h"
    RTC_DS3231 rtc;
    char daysOfTheWeek[7][12] = {"Sun", "Mod", "Tue", "Wed", "Thu", "Fri", "Sat"};


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Light Sensor BH1750
/*
  This is a simple code to test BH1750FVI Light senosr
  communicate using I2C Protocol
  this library enable 2 slave device address
  Main address  0x23
  secondary address 0x5C
  connect this sensor as following :
  VCC >>> 3.3V
  SDA >>> A4 --> use of I2C Protocoll, can be added to RTC
  SCL >>> A5 --> use of I2C Protocoll, can be added to RTC
  addr >> A3
  Gnd >>>Gnd
*/
    #include <BH1750FVI.h> // Sensor Library
    #include <Wire.h> // I2C Library
    //long light_sensor_sleep = 300000; // Sleep time between reads 5min (in milliseconds)
    //stop_light 600;

    BH1750FVI LightSensor;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Delay for loop Arduino
    void _delay(float seconds) {
      long endTime = millis() + seconds * 1000; //delay 1 sec
      while(millis() < endTime) _loop();
    }


////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Other
    digitalWrite(ResetPin, HIGH); //Resets Arduino
//    delay(1000);
    pinMode(ResetPin, OUTPUT);
    Serial.begin(9600);//initialize Serial Port
//    Serial.println();
//    Serial.println("Resetting Arduino because of Sensor error"); //print reset to know the program has been reset and 
//    delay(1000);


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RGB

    pinMode(LEDred, OUTPUT);
    pinMode(LEDgreen, OUTPUT);
    pinMode(LEDblue, OUTPUT);


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Light Relais Module
    pinMode(SunlightRelais, OUTPUT);
//    Serial.println("Running artifical Light");


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Clock RTC DS3231

   Serial.begin(9600);
   #ifndef ESP8266
    while (!Serial); // wait for serial port to connect. Needed for native USB
   #endif
   if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
    Serial.println("Running Clock");
    }

    if (rtc.lostPower()) {
      Serial.println("RTC lost power, let's set the time!");
      // When time needs to be set on a new device, or after a power loss, the
      // following line sets the RTC to the date & time this sketch was compiled
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      // This line sets the RTC with an explicit date & time, for example to set
      // January 21, 2014 at 3am you would call:
      // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }

// When time needs to be re-set on a previously configured device, the
// following line sets the RTC to the date & time this sketch was compiled
// rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
// This line sets the RTC with an explicit date & time, for example to set
// January 21, 2020 at 3am you would call:
// rtc.adjust(DateTime(2020, 1, 21, 3, 0, 0)); //Year, Month, Day, Hour, Min


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pump Relais Module

//int PumpRelay = 7; // This is the Arduino Pin that will control Relay
//long pumpdelay = 5000; // delay of pumping 5sec

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Light Sensor BH1750

    Serial.begin(9600);
    LightSensor.begin();
  /*
Set the address for this sensor
you can use 2 different address
Device_Address_H "0x5C"
Device_Address_L "0x23"
you must connect Addr pin to A3 .
*/
    LightSensor.SetAddress(Device_Address_H);//Address 0x5C
// To adjust the slave on other address , uncomment this line
// lightMeter.SetAddress(Device_Address_L); //Address 0x5C
//-----------------------------------------------
  /*
   set the Working Mode for this sensor
   Select the following Mode:
    Continuous_H_resolution_Mode
    Continuous_H_resolution_Mode2
    Continuous_L_resolution_Mode
    OneTime_H_resolution_Mode
    OneTime_H_resolution_Mode2
    OneTime_L_resolution_Mode

    The data sheet recommanded To use Continuous_H_resolution_Mode
  */
    LightSensor.SetMode(Continuous_H_resolution_Mode);
//    Serial.println("Running Light Sensor");


////////////////////////////////////////////////////////////////////////////////////////////////////////////

Serial.println("Running StadtFarm V. 1.0.0 ....");

}    

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void _loop() {
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////



void loop() {

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Other


////////////////////////////////////////////////////////////////////////////////////////////////////////////


// System Test: RGB

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
    analogWrite(LEDblue, brightness1a); // led off
    delay(Delay_long_a); // pause
    analogWrite(LEDblue, RGB_off); // led off

/*
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


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Clock RTC DS3231

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
    
    Serial.print("Board Temperature: ");
    Serial.print(rtc.getTemperature());
    Serial.println(" C");



////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Light Sensor BH1750

//    LightSensor.begin();
//    delay(1000);
    uint16_t lux = LightSensor.GetLightIntensity();// Get Lux value
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lux");
   
    if (lux == 0 || lux > 54000){
      Serial.print("Light Sensor might not working properly "); // Checks if Sensoroutput is to low or to hight and resets Arduino
      delay(10);
      Serial.println();
      Serial.print("resetting...");
      delay(1000);
      digitalWrite(ResetPin, LOW); //Resets Arduino
    };


//    delay(1000);
//    LightSensor.Sleep(); //Sensor in Sleep Mode

//  if (lux >= 500) {
//    send(msg.set(lux));
//    LightSensor.end();;


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Light Relais Module

//  Serial.print(now.hour());
  delay(1000);
  if ((10 <= now.hour() && 22 >= now.hour()) && lux <= 400 == true) {
    digitalWrite(SunlightRelais, HIGH);     //Switch Relay #4 On
//    digitalWrite(LED, HIGH);          // switch test LED 13 on
    //delay(1000);                      //Wait 1 Second
    } else{
      digitalWrite(SunlightRelais, LOW);    //Switch Relay #4 OFF (in NO (Normaly open) Mode)
//      digitalWrite(LED, LOW);          // switch test LED 13 on
      //delay(1000);                      //Wait 1 Second
      }


    Serial.println(); //print space

    delay(5000);

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
