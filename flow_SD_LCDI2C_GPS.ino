#include <SD.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <TinyGPS.h>



byte statusLed    = 13;

byte sensorInterrupt = 0;  // 0 = Analog pin A2
byte sensorPin       = 3;

File flowDT;

LiquidCrystal_I2C lcd(0x3f,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 1; // 제품설명서에 98이라 나와있는데 실험통해 바꿔야할 

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLiters;
unsigned long totalMilliLiters;
unsigned long oldTime;

uint8_t _hour, _minute, _second, _year, _month, _day; // GPS로부터 시간값 읽기
 
#define GPSBAUD 9600
 
TinyGPS gps;
 
float latitude, longitude;

void setup()
{
  
  // Initialize a serial connection for reporting values to the host
  Serial.begin(9600);
   
  // Set up the status LED line as an output
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
  
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pinMode(53,OUTPUT);

  pulseCount        = 0;
  flowRate          = 0.0;  
  flowMilliLiters   = 0;  
  totalMilliLiters  = 0;  
  oldTime           = 0;

  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Sensor is");
  lcd.setCursor(0,1);
  lcd.print("Ready");
  delay(3000);
  lcd.clear();
  
  Serial.println("START..."); //GPS
  Serial1.begin(GPSBAUD); //GPS
  Serial1.setTimeout(10); //GPS
  gps = TinyGPS(); //GPS
  
  while(!SD.begin(53))
  { 
    lcd.setCursor(0,0);
    lcd.print("Insert");
    lcd.setCursor(0,1);
    lcd.print("SD card");
  }
    lcd.clear();
    flowDT=SD.open("flowdata.txt",FILE_WRITE);
    flowDT.println("---------------------------------------");
    flowDT.println("1sec,Flow rate[L/min],Current Liquid Flowing[mL/sec],Output Liquid Quantity[mL");
    flowDT.close();
}

/**
 * Main program loop
 */
void loop()
{  
   String temp = "";
   while(Serial1.available()){
    char c = Serial1.read(); 
   
   flowDT=SD.open("flowdata.txt",FILE_WRITE);
   if(flowDT){      
   if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 

     if(gps.encode(c)){
      getgps(gps);
      }
    
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(sensorInterrupt);
    
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;  
    oldTime = millis();      
    flowMilliLiters = (flowRate / 60) * 1000;  
    totalMilliLiters += flowMilliLiters;

    unsigned int frac;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate_1: ");
    Serial.print(flowRate);  // Print the integer part of the variable
    // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
    Serial.print("L/min");
    // Print the number of litres flowed in this second
    Serial.print("  Current Liquid Flowing: ");             // Output separator
    Serial.print(flowMilliLiters);
    Serial.print("mL/Sec");

    // Print the cumulative total of litres flowed since starting
    Serial.print("  Output Liquid Quantity: ");             // Output separator
    Serial.print(totalMilliLiters);
    Serial.println("mL"); 
        
    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Flow: ");
    lcd.setCursor(6,0);
    lcd.print(flowRate);
    lcd.setCursor(12,0);
    lcd.print("L/m");
    lcd.setCursor(0,1);
    lcd.print("Q: ");
    lcd.setCursor(3,1);
    lcd.print(totalMilliLiters);
    lcd.setCursor(13,1);
    lcd.print("mL");
    
    flowDT.print(", ");
    flowDT.print(flowRate);
    flowDT.print(",");
    flowDT.print(flowMilliLiters);
    flowDT.print(",");
    flowDT.println(totalMilliLiters);
    flowDT.close();   
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
         }
       
      }
    }
  
  if( 0 < temp.length()){
    }
  while(!SD.begin(53))
   {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Insert");
    lcd.setCursor(0,1);
    lcd.print("SD card");
    }
  
}

/*
Insterrupt Service Routine
 */
void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}
// The getgps function will get and print the values we want.
void getgps(TinyGPS &gps)
{
 
  // Same goes for date and time
  int year;
  int k_hour;
  byte month, day, hour, minute, second;
  _year = year-2000;
  _month = month;
  _day = day;
  _hour = hour;
  _minute = minute;
  _second = second;

//  k_hour = (hour,DEC) ;
//  if ( k_hour + 9 >= 24){
//    k_hour = k_hour + 5;
//     }
  
  gps.crack_datetime(&year,&month,&day,&hour,&minute,&second);
  // Print data and time
  Serial.print("Date: "); Serial.print(year); Serial.print("-"); 
  Serial.print(month, DEC); Serial.print("-"); Serial.print(day, DEC);
  Serial.print("  Time: "); Serial.print(hour,DEC); Serial.print(":"); 
  Serial.print(minute, DEC); Serial.print(":"); Serial.println(second, DEC); 
//  Serial.print("."); Serial.println(hundredths, DEC);
//Since month, day, hour, minute, second, and hundr
//    flowDT=SD.open("flowdata.txt",FILE_WRITE);
//   if(flowDT){
    flowDT.print(year);
    flowDT.print("-");
    flowDT.print(month,DEC);
    flowDT.print("-");
    flowDT.print(day,DEC);
    flowDT.print(" ");
    flowDT.print(hour,DEC);
    flowDT.print(":");
    flowDT.print(minute,DEC);
    flowDT.print(":");
    flowDT.print(second,DEC);
    flowDT.close();
//   }
  
}
