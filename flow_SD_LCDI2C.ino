#include <SD.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h> 

byte statusLed    = 13;

byte sensorInterrupt = 0;  // 0 = Analog pin A2
byte sensorPin       = 3;

File flowDT;

LiquidCrystal_I2C lcd(0x3f,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 79.46; // 제품설명서에 98이라 나와있는데 실험통해 바꿔야함 ->79.46

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLiters;
unsigned long totalMilliLiters;
unsigned long oldTime;
 
void setup()
{
  
  // Initialize a serial connection for reporting values to the host
  Serial.begin(115200);
   
  // Set up the status LED line as an output
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
  
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pinMode(10,OUTPUT);

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
  
  while(!SD.begin(10))
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
   unsigned long tm = 0;
   flowDT=SD.open("flowdata.txt",FILE_WRITE);
   if(flowDT){      
   if((millis() - oldTime) > 1000)    // Only process counters once per second
  {    
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(sensorInterrupt);
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;  
    oldTime = millis();      
    flowMilliLiters = (flowRate / 60) * 1000;  
    totalMilliLiters += flowMilliLiters;
    pulseCount = 0;pulseCount = 0;
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);    // Reset the pulse counter so we can start incrementing again

    unsigned int frac;
  
    // Print the flow rate for this second in litres / minute
    tm = millis();///60000; // millisec to minute
    Serial.print(tm);
    Serial.print(", Flow rate_1: ");
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
   
         }
     } 
  
  while(!SD.begin(10))
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
