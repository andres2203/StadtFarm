//////////// Moisture ////////////
const int Capac_Water = 296;
const int Capac_Air = 480;
//const int Capac_WaterValue_Min = ;  // Dry, Limit moisture, start pump
//const int Capac_WaterValue_Full = ;  // Limit for max Water level for filling
//int intervalls = (AirValue - WaterValue)/3; // could be devided in more intervalls
int Capac_Min_Timer_limit = 0;
int Capac_Full_Timer_limit = 0;
int Capac_timer_threshold = 20 * 60; // 20 minutes, time is need until sensor measures water (could be last shelve)
int SoilMoistureValue = 0;




#define Level 12  // for water level indicator
#define LED 13//bool Level = HIGH;


void setup() {
  Serial.begin(9600); // open serial port, set the baud rate as 9600 bps

  pinMode (13, OUTPUT);
  digitalWrite(Level, HIGH);
  pinMode (12, OUTPUT);
}
void loop() {
 // int val;
  SoilMoistureValue = analogRead(0); //connect sensor to Analog 0
/*
  if (SoilMoistureValue < (WaterValue + intervalls))
  {
    Serial.println("Very wet");
  }
  else if(SoilMoistureValue > (Capac_WaterValue_Min + intervalls) && SoilMoistureValue < (Capac_WaterValue_Full - intervalls))
  {
    Serial.println("Wet");
  }
  else if(SoilMoistureValue > (AirValue - intervalls))
  {
    Serial.println("Dry");
  }
  */
  
  Serial.print(SoilMoistureValue); //print the value to serial port
  Serial.println("%RH");
  delay(2000);

//  if Level == LOW
}
