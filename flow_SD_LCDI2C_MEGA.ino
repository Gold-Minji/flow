#include <LiquidCrystal_I2C.h>
#include<SD.h>
#include <Wire.h>

byte statusLed    = 13;

byte sensorInterrupt  = A2;  // 0 = Analog pin A2
byte sensorPin1       = 3;
byte sensorPin2       = 2;

// YF-DN40 the hall-effect outputs Hz = 0.54 L/min pulses 
float Calibration = 79.46;

volatile byte pulseCount_1;


// 10mins interval at which to read data
const long interval = 600000; 

float flowRate_1;


unsigned int flowMilliLiters_1;


unsigned long totalMilliLiters_1;


unsigned long oldTime = 0;

LiquidCrystal_I2C lcd(0x3F,16,2);

File flowDT;

void setup()
{
   
  // Initialize a serial connection for reporting values to the host
  Serial.begin(115200);
    
  // Set up the status LED line as an output
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
  
  pinMode(sensorPin1, INPUT);
  digitalWrite(sensorPin1, HIGH);


  pulseCount_1        = 0;  
  flowRate_1          = 0.0;
  flowMilliLiters_1   = 0;  
  totalMilliLiters_1  = 0;
 
  oldTime           = 0;

  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(sensorInterrupt, pulseCounter_1, FALLING);

  lcd.init();
  lcd.backlight();

  // SD카드 출력모드 설정,아두이노 mega는 53번핀. 
  pinMode(53,OUTPUT);
  while(!SD.begin(4))
  {
    lcd.setCursor(0,0);
    lcd.print("Insert");
    lcd.setCursor(0,1);
    lcd.print("SD card");
  }
    flowDT=SD.open("flowdata1.txt",FILE_WRITE);
    flowDT.println("------------------------------");//'-', 30개
    flowDT.println("10min,Flow rate[L/min],Current Liquid Flowing[mL/sec],Output Liquid Quantity[mL]");
    flowDT.close();
    
}

void loop()
{
   unsigned long tm = 0;
   
   flowDT=SD.open("flowdata1.txt",FILE_WRITE);
   
   if(flowDT){

    // Only process counters once per second
   if((millis() - oldTime) > 1000){
    
    detachInterrupt(sensorInterrupt);

    flowRate_1 = ((1000.0 / (millis() - oldTime)) * pulseCount_1) / Calibration;
    
    oldTime = millis();
   
    flowMilliLiters_1 = (flowRate_1 / 60) * 1000;
    
    totalMilliLiters_1 += flowMilliLiters_1;
      
   if((millis() - oldTime) >= 600000)
    {     
    
    tm = millis()/60000; // millisec to minute
    Serial.print(tm);
    Serial.print(",  Flow rate_1: ");
    Serial.print(flowRate_1);  // Print the integer part of the variable
    
    /*Serial.print(".");             // Print the decimal point
    unsigned int frac;
    // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
    frac = (flowRate - int(flowRate)) * 10;
    Serial.print(frac, DEC) ;      // Print the fractional part of the variable*/
    
    Serial.print("L/min");    
    Serial.print("  Current Liquid Flowing: ");        
    Serial.print(flowMilliLiters_1);
    Serial.print("mL/Sec");

    Serial.print("  Output Liquid Quantity: ");             
    Serial.print(totalMilliLiters_1);
    Serial.println("mL"); 

    pulseCount_1 = 0;
    
    attachInterrupt(sensorInterrupt, pulseCounter_1, FALLING);
    flowDT.print(tm);
    flowDT.print(",");
    flowDT.print(flowRate_1);
    flowDT.print(",");
    flowDT.print(flowMilliLiters_1);
    flowDT.print(",");
    flowDT.println(totalMilliLiters_1);

        
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("flow:");
    lcd.setCursor(6,0);
    lcd.print(flowRate_1);
    lcd.setCursor(12,0);
    lcd.print("L/m");
    
    lcd.setCursor(0,1);
    lcd.print("flow:");
    lcd.setCursor(6,1);
    lcd.print(flowMilliLiters_1);
    lcd.setCursor(12,1);
    lcd.print("mL/s");
    } 
}
   flowDT.close();
 
   while(!SD.begin(10))
   {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Insert");
    lcd.setCursor(0,1);
    lcd.print("SD card");
    }
}}
/*
Insterrupt Service Routine
 */
void pulseCounter_1()
{
  // Increment the pulse counter
  pulseCount_1++;
}
