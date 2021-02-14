// Pump Switch Modules ////////////
#define pumpSwitch_1 8 // This is the Arduino Pin that will control pump switch
//#define pumpSwitch_2 3

unsigned long delayPump = 600000; //pump delay time between cycles in ms (positive numbers, 32bit)
unsigned long runnningPump = 600000; // running Timer for pumps in ms
bool pumpState = HIGH; // bool for switch
unsigned long Timer = 0;


void setup() {
    Serial.begin(9600);//initialize Serial Port
      #ifndef ESP8266
  while(!Serial); // wait for serial port to connect. Needed for native USB
  #endif

  // put your setup code here, to run once:
  pinMode(pumpSwitch_1, OUTPUT);
//  pinMode(pumpSwitch_2, OUTPUT);

//  digitalWrite(pumpSwitch_1, pumpState);
//  digitalWrite(pumpSwitch_2, LOW);

  Timer = millis();
  Serial.println(Timer);
//  delay(500);
}


void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(pumpSwitch_1, pumpState);

  if (pumpState == HIGH){
    if((millis() - Timer) >= runnningPump){
      pumpState = !pumpState;
      Serial.println("Pump stop");
      Timer = millis();
      Serial.println(Timer/1000);

      }
    }else{
    if((millis() - Timer) >= delayPump){
      pumpState =! pumpState;
      Serial.println("pump run");
      Timer = millis();
      Serial.println(Timer/1000);

      }
    }
}
