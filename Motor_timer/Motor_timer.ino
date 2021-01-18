#include <Wire.h>
#include <DS3231.h>
#include <RotaryEncoder.h>
#include <OneButton.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Setup a RotaryEncoder for pins 2 and 3:
RotaryEncoder encoder(2, 3);

OneButton button(A1, true, true);

DS3231 clock;
RTCDateTime dt;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool relayStatus = 0;

uint8_t menuItem = 1;
bool subMenuItem = 0;
uint8_t page = 1;
uint8_t frame = 1;

//uint8_t timer1H = 21, timer1M = 22, timer1S = 23;
bool timer1Status = 0;
//uint8_t timer2H = 21, timer2M = 22, timer2S = 23;
bool timer2Status = 0;

bool lightStatus = 1;

//String menuItem1 = "Motor On Time";
//String menuItem2 = "Motor Off Time";
//String menuItem3 = "Auto: ON";
//String menuItem4 = "Manual control";
//String menuItem5 = "Set Time";
//String menuItem6 = "Light: ON";

void setup()
{
  Serial.begin(9600);
  
  pinMode(LED_BUILTIN, OUTPUT);

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(2000);

  // Initialize DS3231
  Serial.println("Initialize DS3231");;
  clock.begin();

  button.attachClick(select);
  button.attachLongPressStart(mainMenu);

  // Disarm alarms and clear alarms for this example, because alarms is battery backed.
  // Under normal conditions, the settings should be reset after power and restart microcontroller.
  clock.armAlarm1(false);
  clock.armAlarm2(false);
  clock.clearAlarm1();
  clock.clearAlarm2();
 
  // Manual (Year, Month, Day, Hour, Minute, Second)
  //clock.setDateTime(2014, 4, 25, 0, 0, 0);

  // Set Alarm - Every second.
  // DS3231_EVERY_SECOND is available only on Alarm1.
  // setAlarm1(Date or Day, Hour, Minute, Second, Mode, Armed = true)
  // clock.setAlarm1(0, 0, 0, 0, DS3231_EVERY_SECOND);

  // Set Alarm - Every full minute.
  // DS3231_EVERY_MINUTE is available only on Alarm2.
  // setAlarm2(Date or Day, Hour, Minute, Second, Mode, Armed = true)
  // clock.setAlarm2(0, 0, 0, 0, DS3231_EVERY_MINUTE);
  
  // Set Alarm1 - Every 20s in each minute
  //setAlarm1(Date or Day, Hour, Minute, Second, Mode, Armed = true)
  clock.setAlarm1(0, 0, 0, 20, DS3231_MATCH_S);
  
  // Check alarm settings
  checkAlarms();

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(5, 0);
  // Display static text
  display.println("Motor On Time");
  display.setCursor(5, 10);
  display.println("Motor Off Time");
  display.display();
}

//=============================================================================
void checkAlarms()
{
  RTCAlarmTime a1;  
  RTCAlarmTime a2;

  if (clock.isArmed1())
  {
    a1 = clock.getAlarm1();

    Serial.print("Alarm1 is triggered ");
    switch (clock.getAlarmType1())
    {
      case DS3231_EVERY_SECOND:
        Serial.println("every second");
        break;
      case DS3231_MATCH_S:
        Serial.print("when seconds match: ");
        Serial.println(clock.dateFormat("__ __:__:s", a1));
        break;
      case DS3231_MATCH_M_S:
        Serial.print("when minutes and sencods match: ");
        Serial.println(clock.dateFormat("__ __:i:s", a1));
        break;
      case DS3231_MATCH_H_M_S:
        Serial.print("when hours, minutes and seconds match: ");
        Serial.println(clock.dateFormat("__ H:i:s", a1));
        break;
      case DS3231_MATCH_DT_H_M_S:
        Serial.print("when date, hours, minutes and seconds match: ");
        Serial.println(clock.dateFormat("d H:i:s", a1));
        break;
      case DS3231_MATCH_DY_H_M_S:
        Serial.print("when day of week, hours, minutes and seconds match: ");
        Serial.println(clock.dateFormat("l H:i:s", a1));
        break;
      default: 
        Serial.println("UNKNOWN RULE");
        break;
    }
  } else
  {
    Serial.println("Alarm1 is disarmed.");
  }

  if (clock.isArmed2())
  {
    a2 = clock.getAlarm2();

    Serial.print("Alarm2 is triggered ");
    switch (clock.getAlarmType2())
    {
      case DS3231_EVERY_MINUTE:
        Serial.println("every minute");
        break;
      case DS3231_MATCH_M:
        Serial.print("when minutes match: ");
        Serial.println(clock.dateFormat("__ __:i:s", a2));
        break;
      case DS3231_MATCH_H_M:
        Serial.print("when hours and minutes match:");
        Serial.println(clock.dateFormat("__ H:i:s", a2));
        break;
      case DS3231_MATCH_DT_H_M:
        Serial.print("when date, hours and minutes match: ");
        Serial.println(clock.dateFormat("d H:i:s", a2));
        break;
      case DS3231_MATCH_DY_H_M:
        Serial.println("when day of week, hours and minutes match: ");
        Serial.print(clock.dateFormat("l H:i:s", a2));
        break;
      default: 
        Serial.println("UNKNOWN RULE"); 
        break;
    }
  } else
  {
    Serial.println("Alarm2 is disarmed.");
  }
}

//====================================================================================================

void select()
{
  if (page == 1 && menuItem <= 5) {    
    page = 2;
  }
   
  else if(page == 1 && menuItem == 6){
   lightStatus = !lightStatus;
  }
   
 else if (page==2 && menuItem == 1) { 
   if(subMenuItem){ 
    setAlarmDuration(1);  
   }
   else{
    setAlarmTime(1);
   }
  }

   else if (page==2 && menuItem == 2) 
  {
   if(subMenuItem){ 
    setAlarmDuration(2);  
   }
   else{
    setAlarmTime(2);
   }
  }
  else if (page==2 && menuItem == 3) 
  {
    if(subMenuItem){ 
    relayStatus = 0;  
   }
   else{
    relayStatus = 1; 
   }
  }  
}

void mainMenu()
{
  page = 1;
  Serial.println("Main menu");
}

void loop()
{   
  digitalWrite(LED_BUILTIN,relayStatus);
  button.tick();
  
  static int lastSec = 0; 
  dt = clock.getDateTime();
  int sec = dt.second;

  static int pos = 0;
  encoder.tick(); 
  int newPos = encoder.getPosition();   

  drawMenu();
  
  if (pos != newPos) {
    
    if(newPos>pos) {
      Serial.print("CW");
      
      if(page == 1){
        menuItem++;
        if(menuItem > 6){
          menuItem = 6;
        }
      }
      
      else if(page == 2 && menuItem <= 3){
         subMenuItem = !subMenuItem;
      }
    }
    
    if(newPos<pos) {
      Serial.print("CCW");
      if(page == 1){
        menuItem--;
        if(menuItem < 1){
          menuItem = 1;
        }
        else if(page == 2 && menuItem <= 3){
        subMenuItem = !subMenuItem;
        }
      }
    }
    Serial.println();
    pos = newPos;
  }

  if (lastSec != sec) {
    //Serial.print(clock.dateFormat("d-m-Y h:i:s - l", dt));
    Serial.print(dt.year);   Serial.print("-");
    Serial.print(dt.month);  Serial.print("-");
    Serial.print(dt.day);    Serial.print(" ");
    Serial.print(dt.hour);   Serial.print(":");
    Serial.print(dt.minute); Serial.print(":");
    Serial.print(dt.second); Serial.println("");
    lastSec = sec;
  } 

  // Call isAlarm1(false) if you want clear alarm1 flag manualy by clearAlarm1();
  if (clock.isAlarm1())
  {
    Serial.println("ALARM 1 TRIGGERED!");
    relayStatus = !relayStatus; 
  }

  // Call isAlarm2(false) if you want clear alarm1 flag manualy by clearAlarm2();
  if (clock.isAlarm2())
  {
    Serial.println("ALARM 2 TRIGGERED!");
  }
 
}

  void drawMenu()
  {
    
  if (page==1) 
  {    
    display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(WHITE);

    if(menuItem%2 == 0){
      display.setCursor(0, 10);
      display.println(" ");
      display.setCursor(0, 25);
      display.println(">");
    } 
    else{
        display.setCursor(0, 10);
        display.println(">");
        display.setCursor(0, 25);
        display.println(" ");
    }

    if(frame ==1)
    {   
      display.setCursor(0, 10);
      display.print(" Timer 1: ");
      statusDisplay(timer1Status);
      display.setCursor(0, 25);
      display.print(" Timer 2: ");
      statusDisplay(timer2Status);
    }
    else if(frame == 2)
    {
      display.setCursor(0, 10);
      display.println(" Manual ");
      display.setCursor(0, 25);
      display.println(" Set time");
    }
    else if(frame == 3)
    {
      display.setCursor(0, 10);
      display.println(" Info screen");
      display.setCursor(0, 25);
      display.print(" Light ");
      statusDisplay(lightStatus);
    }
    display.display();
 }
 else if (page==2 && menuItem <= 3) 
 { 
   if(menuItem != 3){   
   display.setCursor(0, 10);
   display.print(" Start time");
   display.setCursor(0, 25);
   display.println(" Duration");
   }
   else{
   display.setCursor(0, 10);
   display.print(" ON");
   display.setCursor(0, 25);
   display.println(" OFF");
   }
       if(subMenuItem){
       display.setCursor(0, 10);
       display.println(" ");
       display.setCursor(0, 25);
       display.println(">");
       }
       else{
       display.setCursor(0, 10);
       display.println(">");
       display.setCursor(0, 25);
       display.println(" ");
       }
  }

   else if (page==2 && menuItem == 4) 
  {
   setTime();
  }
  else if (page==2 && menuItem == 5) 
  {
   infoScreen();
  }  
  display.display();
}
  
void statusDisplay(bool state){
  if(state){
    display.println("ON");
  }
  else{
    display.println("OFF");
  }
}

void setTime(){
  
}

void infoScreen(){
  
}

void setAlarmTime(int alarmNo){
  
}

void setAlarmDuration(int alarmNo){
  
}
