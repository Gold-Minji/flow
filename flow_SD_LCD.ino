#include<LiquidCrystal.h>
#include<SD.h>

byte statusLed    = 50;

byte sensorInterrupt = 0;  // 0 = Analog pin A2
byte sensorPin       = 2;

// YF-DN40 the hall-effect outputs Hz = 0.54 L/min pulses 
float Calibration = 0.54;

volatile byte pulseCount;  

// 10mins interval at which to read data
const long interval = 600000; 

float flowRate;
unsigned int flowMilliLiters;
unsigned long totalMilliLiters;

unsigned long oldTime = 0;

LiquidCrystal lcd(7,6,5,4,3,8);

File flowDT;

void setup()
{
  
  // Initialize a serial connection for reporting values to the host
  Serial.begin(115200);
    
  // Set up the status LED line as an output
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
  
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLiters   = 0;
  totalMilliLiters  = 0;
  oldTime           = 0;

  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);

  lcd.begin(16,2);
  lcd.clear();

  pinMode(53,OUTPUT);
  
  if(!SD.begin(53))
  {
    lcd.setCursor(0,0);
    lcd.print("Insert");
    lcd.setCursor(0,1);
    lcd.print("SD card");
    Serial.println("There is no SD card");
  }
    Serial.println("Sensor is working");
    flowDT=SD.open("flowdata.txt",FILE_WRITE);
    flowDT.println("------------------------------");//'-', 30개
    flowDT.println("10min,Flow rate[L/min],Current Liquid Flowing[mL/sec],Output Liquid Quantity[mL]");
    flowDT.close();

}

void loop()
{
   unsigned long tm = 0;
   
   flowDT=SD.open("flowdata.txt",FILE_WRITE);
   if(flowDT){

    // Only process counters once per second
   if((millis() - oldTime) > 1000){
    
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(sensorInterrupt);
        
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the Calibration to scale the output
    // based on the number of pulses per second per units of measure (liters/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / Calibration;
    
    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    oldTime = millis();
    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to milliliters.
    flowMilliLiters = (flowRate / 60) * 1000;
    
    // Add the milliliters passed in this second to the cumulative total
    totalMilliLiters += flowMilliLiters;
      
   if((millis() - oldTime) >= 600000)
    {     
    // Print the flow rate for this second in litres / minute
    tm = millis()/60000; // millisec to minute
    Serial.print(tm);
    Serial.print(", Flow rate: ");
    Serial.print(flowRate);  // Print the integer part of the variable
    
    /*Serial.print(".");             // Print the decimal point
    unsigned int frac;
    // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
    frac = (flowRate - int(flowRate)) * 10;
    Serial.print(frac, DEC) ;      // Print the fractional part of the variable*/
    
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
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
    flowDT.print(tm);
    flowDT.print(",");
    flowDT.print(flowRate);
    flowDT.print(",");
    flowDT.print(flowMilliLiters);
    flowDT.print(",");
    flowDT.println(totalMilliLiters);
        
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("flow:");
    lcd.setCursor(6,0);
    lcd.print(flowRate);
    lcd.setCursor(12,0);
    lcd.print("L/m");
    
    lcd.setCursor(0,1);
    lcd.print("current:");
    lcd.setCursor(8,1);
    lcd.print(flowMilliLiters);
    lcd.setCursor(12,1);
    lcd.print("ml/s");
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
void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}
