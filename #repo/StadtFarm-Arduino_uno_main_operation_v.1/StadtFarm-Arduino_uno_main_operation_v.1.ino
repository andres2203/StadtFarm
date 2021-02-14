#define MILISEC_PER_SEC (1000UL)
#define MILISEC_PER_MIN (MILISEC_PER_SEC * 60UL)
#define MILISEC_PER_HOUR (MILISEC_PER_MIN * 60UL)
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

bool Sensor_Error = true;


//////////// RGB ////////////
#define LEDred 9 // for red color LED on Pin 9
#define LEDblue 11 // for blue color LED on Pin 11
#define LEDgreen 10 // for green color LED on Pin 10
// Build in LED : pinMode(LED_BUILTIN, OUTPUT);

int brightness1a = 50; // Nr between 0 and 255 for brightness of LED
int brightness1b = 150; 
int brightness1c = 250; 
int RGB_off = 0; // 0 for LED of = 0V
int RGBDelay_a = 200; //Blink frequency
int RGBDelay_b = 500;
bool RGBBlinkState = HIGH;
unsigned long RGBTimer = 0; //will be rewriten on every loop



//////////// Light Relais Module ////////////
#define Light_1 2 //This is the Arduino Pin that will control Light on Relay 1
#define Light_2 3
#define Light_3 4
#define Light_4 5

bool light_status = HIGH;


//////////// Timer Variables ////////////
long lastDebounceTime = 0;  
long Delay_long_a = 500; // delay 0,5 sec
long Delay_long_b = 1000; // delay 1 sec
long Delay_long_c = 10000; // delay 10 sec
long Delay_long = 10000;
//const long LEDInterval = 100; // interval at which to blink


// Pump Switch Modules ////////////
#define pumpSwitch_1 8 // This is the Arduino Pin that will control pump switch
//#define pumpSwitch_2 3


int delayPump = MILISEC_PER_HOUR * 3; //pump delay time between cycles in ms
int runnningPump = MILISEC_PER_MIN * 2; // running Timer for pumps in ms
bool pumpState = HIGH; // bool for switch
unsigned long Timer = 0;



// Display ////////////
unsigned long DisplayTimer = 0; //will be set current time every loop
int DisplayDelay = 2000; //delay between reloading display

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////SETUP///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {

  Serial.begin(9600);//initialize Serial Port
  #ifndef ESP8266
  while(!Serial); // wait for serial port to connect. Needed for native USB
  #endif


  setupPump();
  setupRGBLight();
  setupGrowLight();
  setupDisplay();
 

}





//////////// Light Relais Module ////////////
void setupGrowLight(){
  pinMode(Light_1, OUTPUT);
  pinMode(Light_2, OUTPUT);
  pinMode(Light_3, OUTPUT);
  pinMode(Light_4, OUTPUT);
  

  Serial.println("Starting Grow Light Module");
  delay(500);
}

//////////// Pump Switch Modules ////////////
void setupPump() {
  pinMode(pumpSwitch_1, OUTPUT);
  Serial.println("Starting Emergency Watering");
  delay(500);
  Timer = millis();
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



////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////LOOP//////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


void loop() {
  pumpManagement();
  GrowLight();
  RGBLight();
  Alarms();
  Display();
  }



//////////// Grow Light Relais Module ////////////
void GrowLight(){

    digitalWrite(Light_1, HIGH);  //Switch Relay #2 On
    digitalWrite(Light_2, HIGH);  //Switch Relay #3 On
    digitalWrite(Light_3, HIGH);  //Switch Relay #4 On
    digitalWrite(Light_4, HIGH);  //Switch Relay #5 On
}


//////////// Pump Switch Modules ////////////
void pumpManagement() {
  digitalWrite(pumpSwitch_1, pumpState);

  if (pumpState == HIGH){
    if((millis() - Timer) >= runnningPump){
      pumpState = LOW;
      Timer = millis();
      }
   if (pumpState == LOW){
      if((millis() - Timer) >= delayPump){
        pumpState = HIGH;
        Timer = millis();
      }
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
  if (light_status == HIGH){
    Serial.println("StadtFarm in day mode, grow light on");
  }else {
    Serial.println("StadtFarm in Night Mode");
  }
  Serial.println(millis);
  if(pumpState == HIGH){
    Serial.print("watering Pump running... ");
    Serial.println();
  }
}

//////////// Alarms  ////////////
void Alarms(){

}
