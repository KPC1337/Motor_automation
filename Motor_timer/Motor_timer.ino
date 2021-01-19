#include <Wire.h>
#include <DS3231.h>
#include <RotaryEncoder.h>
#include <OneButton.h>
#include <SPI.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

// Setup a RotaryEncoder for pins 2 and 3:
RotaryEncoder encoder(2, 3);

OneButton button(A1, true, true);

DS3231 clock;
RTCDateTime dt;

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1

SSD1306AsciiAvrI2c display;

bool relayStatus = 0;

uint8_t menuItem = 1;
bool subMenuItem = 0;
uint8_t timeSetItem = 1;
bool settingMode = 0;
uint8_t page = 1;
uint8_t frame = 1;
bool isInfoScreen = 0;

// index 0 is alarm1 strt time,1 is alarm2 strt, 2 is clock set, 3 is alarm 1 duration, 4 is alrm 2 drtn
uint8_t timerHH[5] = {0,0,0,0,0}, timerMM[5] = {0,0,0,0,0}, timerSS[5] = {0,0,0,0,0}; 
bool timer1Status = 0;
bool timer2Status = 0;
bool countDown1 = 0,countDown2 = 0;
uint32_t timerDuration[2] = {0,0};

bool lightStatus = 0;

void setup()
{
  Serial.begin(9600);
  
  pinMode(LED_BUILTIN, OUTPUT);

#if RST_PIN >= 0
  display.begin(&Adafruit128x32, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  display.begin(&Adafruit128x32, I2C_ADDRESS);
#endif // RST_PIN >= 0

  display.setFont(Adafruit5x7);

  display.clear();

  // first row
  display.println("Motor timer");

  // second row
  display.set2X();
  display.println("v1");

  delay(1000); // Pause for 2 seconds

  // Clear the buffer
  display.clear();

  // Initialize DS3231
  Serial.println("Initialize DS3231");;
  clock.begin();

  button.attachClick(select);
  button.attachLongPressStart(mainMenu);

  // Disarm alarms and clear alarms for this example, because alarms is battery backed.
  // Under normal conditions, the settings should be reset after power and restart microcontroller.
  //clock.armAlarm1(false);
  //clock.armAlarm2(false);
  //clock.clearAlarm1();
  //clock.clearAlarm2();
 
  // Manual (Year, Month, Day, Hour, Minute, Second)
  //clock.setDateTime(2014, 4, 25, 0, 0, 0);
    
  // Set Alarm - Every 01h:10m:30s in each day
  // setAlarm1(Date or Day, Hour, Minute, Second, Mode, Armed = true)
  // clock.setAlarm1(0, 1, 10, 30, DS3231_MATCH_H_M_S);
  
  // Set Alarm1 - Every 20s in each minute
  //setAlarm1(Date or Day, Hour, Minute, Second, Mode, Armed = true)
  //clock.setAlarm1(0, 0, 0, 20, DS3231_MATCH_S);
  
  // Check alarm settings
  checkAlarms();

  display.clear();

drawMenu();
}

//=============================================================================
void checkAlarms()
{
  RTCAlarmTime a1;  
  RTCAlarmTime a2;

  if (clock.isArmed1())
  {
    a1 = clock.getAlarm1();
    timerHH[0]=a1.hour;
    timerMM[0]=a1.minute;
    timerSS[0]=a1.second;

    Serial.print("Alarm1 is Armed ");
    timer1Status = 1;
    
  } else
  {
    Serial.println("Alarm1 is disarmed.");
    timer1Status = 0;
    timerHH[0]=0;
    timerMM[0]=0;
    timerSS[0]=0;
  }

  if (clock.isArmed2())
  {
    a2 = clock.getAlarm2();
    timerHH[1]=a2.hour;
    timerMM[1]=a2.minute;
    timerSS[1]=a2.second;

    Serial.print("Alarm2 is armed ");
    timer2Status = 1;
  } else
  {
    Serial.println("Alarm2 is disarmed.");
    timer2Status = 0;
    timerHH[1]=0;
    timerMM[1]=0;
    timerSS[1]=0;
  }
}

//====================================================================================================

void select()
{
  switch(page){
    case 1:
    if(menuItem<=5){
      page = 2;
      if(menuItem == 4){
        timerHH[2]=dt.hour;
        timerMM[2]=dt.minute;
        timerSS[2]=dt.second;
      }
    }
    else if(menuItem == 6){
      lightStatus = !lightStatus;
      display.invertDisplay(lightStatus);
    }
    break;
    case 2:
      switch(menuItem){
        case 1:
          page = 3;  
          break;
    
        case 2:
          page = 3;
          break;
    
        case 3:
          if(subMenuItem){ 
            relayStatus = 0;  
          }
          else{
            relayStatus = 1; 
          }
          break;

        case 4:
          if(timeSetItem<=3){
            settingMode = !settingMode;
          }
          else if(timeSetItem == 4){
              clock.setDateTime(dt.year, dt.month, dt.day, timerHH[2], timerMM[2], timerSS[2]);
              page = 1;
          }
          else if(timeSetItem == 5){
            page =1;
            timeSetItem = 1;
            checkAlarms();
          }
          break;
      }
    break;
    case 3:
      if(timeSetItem<=3){
      settingMode = !settingMode;
      }
      else if(timeSetItem == 4){
        timeSetHandler(menuItem,subMenuItem);
        page = 2;
      }
      else if(timeSetItem == 5){
        page =2;
        timeSetItem = 1;
        checkAlarms();
      }
    break;
  }
  drawMenu();
}

void mainMenu()
{
  if(page == 1 && menuItem == 1){
    timer1Status = !timer1Status;
    clock.armAlarm1(timer1Status);
    if(!timer1Status){
      clock.armAlarm1(false);
      clock.clearAlarm1();
    }
  }
  else if(page == 1 && menuItem == 2){
    timer2Status = !timer2Status;
    clock.armAlarm2(timer2Status);
    if(!timer2Status){
      clock.armAlarm2(false);
      clock.clearAlarm2();
    }
  }
  page = 1;
  isInfoScreen = 0;
  settingMode = 0;
  Serial.println("Main menu");
  drawMenu();
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

  
  
  if (pos != newPos) {
    if(newPos>pos) {
      Serial.print("CW");
      
      if(page == 1){
        menuItem++;
        switch(menuItem){
          case 1:
          frame = 1;
          break;
          
          case 2:
          frame = 1;
          break;
          
          case 3:
          frame = 2;
          break;
          
          case 4:
          frame = 2;
          break;
          
          case 5:
          frame = 3;
          break;
          
          case 6:
          frame = 3;
          break; 

          default:
          menuItem = 6;
          break;          
        }
      }
      
      else if(page == 2 && menuItem <= 3){
         subMenuItem = !subMenuItem;
      }
      else if(page == 2 && menuItem ==4){
          if(settingMode){
            switch(timeSetItem){
              case 1:
              timerHH[2]++;
              if(timerHH[2] > 23){
                timerHH[2] = 23;
              }
              break;
              case 2:
              timerMM[2]++;
              if(timerMM[2] > 59){
                timerMM[2] = 59;
              }
              break;
              case 3:
              timerSS[2]++;
              if(timerSS[2] > 59){
                timerSS[2] = 59;
              }
              break;
            }
          }
          else{
            timeSetItem ++;
            if(timeSetItem > 5){
              timeSetItem =5;
            }
          }
        }
        else if(page == 3){
          int timerNumber;
          if(menuItem == 1){
            if(subMenuItem){
              timerNumber = 3;
              }
              else{
                timerNumber = 0;
              }
            if(settingMode){
            switch(timeSetItem){
              case 1:
              timerHH[timerNumber]++;
              if(timerHH[timerNumber] > 23){
                timerHH[timerNumber] = 23;
              }
              break;
              case 2:
              timerMM[timerNumber]++;
              if(timerMM[timerNumber] > 59){
                timerMM[timerNumber] = 59;
              }
              break;
              case 3:
              timerSS[timerNumber]++;
              if(timerSS[timerNumber] > 59){
                timerSS[timerNumber] =59;
              }
              break;
            }
            }
          else{
            timeSetItem ++;
            if(timeSetItem > 5){
              timeSetItem =5;
            }
          }
          
          }
          else if(menuItem == 2){
            if(subMenuItem){
              timerNumber = 4;
              }
              else{
                timerNumber = 1;
              }
            if(settingMode){
            switch(timeSetItem){
              case 1:
              timerHH[timerNumber]++;
              if(timerHH[timerNumber] > 23){
                timerHH[timerNumber] =23;
              }
              break;
              case 2:
              timerMM[timerNumber]++;
              if(timerMM[timerNumber] > 59){
                timerMM[timerNumber] = 59;
              }
              break;
              case 3:
              timerSS[timerNumber]++;
              if(timerSS[timerNumber] < 59){
                timerSS[timerNumber] = 59;
              }
              break;
            }
            }
          else{
            timeSetItem ++;
            if(timeSetItem > 5){
              timeSetItem = 5;
            }
          }          
          }            
        }
    }
    
    if(newPos<pos) {
      Serial.print("CCW");
      if(page == 1){
        menuItem--;
          switch(menuItem){
          case 0:
          menuItem=1;
            
          case 1:
          frame = 1;
          break;
          
          case 2:
          frame = 1;
          break;
          
          case 3:
          frame = 2;
          break;
          
          case 4:
          frame = 2;
          break;
          
          case 5:
          frame = 3;
          break;
          
          case 6:
          frame = 3;
          break; 

          default:
          menuItem = 0;
          break;          
        }
        }
        else if(page == 2 && menuItem <= 3){
        subMenuItem = !subMenuItem;
        }
        else if(page == 2 && menuItem ==4){
          if(settingMode){
            switch(timeSetItem){
              case 1:
              if(timerHH[2] > 0){
                timerHH[2]--;
              }
              break;
              case 2:
              if(timerMM[2] > 0){
                timerMM[2]--;
              }
              break;
              case 3:              
              if(timerSS[2] > 0){
                timerSS[2]--;
              }
              break;
            }
          }
          else{
            timeSetItem --;
            if(timeSetItem < 1){
              timeSetItem =1;
            }
          }
        }
        else if(page == 3){
          if(menuItem == 1){
            int timerNumber;
            if(subMenuItem){
              timerNumber = 3;
              }
              else{
                timerNumber = 0;
              }
            if(settingMode){
            switch(timeSetItem){
              case 1:
              if(timerHH[timerNumber] > 0){
                timerHH[timerNumber]--;
              }
              break;
              case 2:
              if(timerMM[timerNumber] > 0){
                timerMM[timerNumber]--;
              }
              break;
              case 3:
              if(timerSS[timerNumber] > 0){
                timerSS[timerNumber]--;
              }
              break;
            }
            }
          else{
            timeSetItem --;
            if(timeSetItem < 1){
              timeSetItem =1;
            }
          }
          
          }
          else if(menuItem == 2){
            int timerNumber = 4;
            if(subMenuItem){
               timerNumber = 4;
              }
              else{
                timerNumber = 1;
              }
            if(settingMode){
            switch(timeSetItem){
              case 1:
              timerHH[timerNumber]--;
              if(timerHH[timerNumber] < 0){
                timerHH[timerNumber] =0;
              }
              break;
              case 2:
              timerMM[timerNumber]--;
              if(timerMM[timerNumber] < 0){
                timerMM[timerNumber] =0;
              }
              break;
              case 3:
              timerSS[timerNumber]--;
              if(timerSS[timerNumber] < 0){
                timerSS[timerNumber] =0;
              }
              break;
            }
            }
          else{
            timeSetItem --;
            if(timeSetItem < 1){
              timeSetItem =1;
            }
          }
          
          }
            
          }
      }
    Serial.println();
    pos = newPos;
    drawMenu();
  }

  if (lastSec != sec) {
    //Serial.print(clock.dateFormat("d-m-Y h:i:s - l", dt));
    Serial.print(dt.year);   Serial.print("-");
    Serial.print(dt.month);  Serial.print("-");
    Serial.print(dt.day);    Serial.print(" ");
    Serial.print(dt.hour);   Serial.print(":");
    Serial.print(dt.minute); Serial.print(":");
    Serial.print(dt.second); Serial.println("");
    if(isInfoScreen){
      drawMenu();
    }
    if(timer1Status && countDown1){
      timerDuration[0]--;
      if(timerDuration[0] == 0){
        countDown1 = 0;
        relayStatus =0;
      }
    }
    else if(timer2Status && countDown2){
      timerDuration[1]--;
      if(timerDuration[1] == 0){
        countDown2 = 0;
        relayStatus = 0;
      }
    }
    lastSec = sec;
  } 

  // Call isAlarm1(false) if you want clear alarm1 flag manualy by clearAlarm1();
  if (clock.isAlarm1() && clock.isArmed1())
  {
    Serial.println("ALARM 1 TRIGGERED!");
    relayStatus = 1; 
    countDown1 = 1;
  }

  // Call isAlarm2(false) if you want clear alarm1 flag manualy by clearAlarm2();
  if (clock.isAlarm2() && clock.isArmed2())
  { 
    Serial.println("ALARM 2 TRIGGERED!");
    relayStatus = 1;
    countDown2 = 1;
  }
 
}

  void drawMenu()
  { display.set1X();
    display.setFont(Verdana12);
  if (page==1) 
  {    
    display.clear();
    //display.setCol(WHITE);

    switch(frame){
    case 1:
    if(menuItem == 1){
      display.setCursor(0, 10);
      display.print(">Timer 1: ");
      statusDisplay(timer1Status);
      display.setCursor(0, 25);
      display.print(" Timer 2: ");
      statusDisplay(timer2Status);
    } 
    else{
      display.setCursor(0, 10);
      display.print(" Timer 1: ");
      statusDisplay(timer1Status);
      display.setCursor(0, 25);
      display.print(">Timer 2: ");
      statusDisplay(timer2Status);
    }
    break;        
    
    case 2:
      if(menuItem == 3){
      display.setCursor(0, 10);
      display.print(">Manual ");
      statusDisplay(relayStatus);
      display.setCursor(0, 25);
      display.println(" Set time");
    } 
    else{
      display.setCursor(0, 10);
      display.print(" Manual ");
      statusDisplay(relayStatus);
      display.setCursor(0, 25);
      display.println(">Set time");
    } 
    break;
    
    case 3:
     if(menuItem == 5){
      display.setCursor(0, 10);
      display.println(">Info screen");
      display.setCursor(0, 25);
      display.print(" Light ");
      statusDisplay(lightStatus);
    } 
    else{
      display.setCursor(0, 10);
      display.println(" Info screen");
      display.setCursor(0, 25);
      display.print(">Light ");
      statusDisplay(lightStatus);;
    }
    break;
    }
   // display.display();
 }
 else if(page==2) 
 { 
  display.clear(); 
   if(menuItem < 3){
          if(subMenuItem){
          display.setCursor(0, 10);
          display.println(" Start time");
          display.setCursor(0, 25);
          display.println(">Duration");
       }
       else{
          display.setCursor(0, 10);
          display.println(">Start time");
          display.setCursor(0, 25);
          display.println(" Duration");
       }    

   }
   else if(menuItem == 3){
       if(subMenuItem){
        display.setCursor(0, 10);
        display.println(" ON");
        display.setCursor(0, 25);
        display.println(">OFF");
       }
       else{
        display.setCursor(0, 10);
        display.println(">ON");
        display.setCursor(0, 25);
        display.println(" OFF");
       } 
   }
   
   else if (menuItem == 4) 
   {
   setTime(2);
   } 
   else if (menuItem == 5) 
   {
   infoScreen();
   isInfoScreen = 1;
   }  
  
  }
  else if(page == 3){
    if(menuItem == 1){
      if(!subMenuItem){
      setTime(0);
      }else{
      setTime(3);  
      }
    }
    else if(menuItem == 2){
      if(!subMenuItem){
      setTime(1);
      }else{
      setTime(4);  
      }
    }
  }

}
  
void statusDisplay(bool state){
  if(state){
    display.println("ON");
  }
  else{
    display.println("OFF");
  }
}

void setTime(int timerNo){
  display.clear();
  switch(timeSetItem){
    case 1:
    display.setCursor(0, 10);
    display.print("<");
    display.print(timerHH[timerNo]);
    display.print(":");
    display.print(timerMM[timerNo]);
    display.print(":");
    display.print(timerSS[timerNo]);
    display.println("");
    display.print("  OK  ");
    display.print("Cancel");
    break;
    
    case 2:
    display.setCursor(0, 10);
    display.print(timerHH[timerNo]);
    display.print(":");
    display.print("<");
    display.print(timerMM[timerNo]);
    display.print(":");
    display.print(timerSS[timerNo]);
    display.println("");
    display.print("  OK  ");
    display.print("Cancel");
    break;

    case 3:
    display.setCursor(0, 10);
    display.print(timerHH[timerNo]);
    display.print(":");
    display.print(timerMM[timerNo]);
    display.print(":");
    display.print("<");
    display.print(timerSS[timerNo]);
    display.println("");
    display.print("  OK  ");
    display.print("Cancel");
    break;
    
    case 4:
    display.setCursor(0, 10);
    display.print(timerHH[timerNo]);
    display.print(":");
    display.print(timerMM[timerNo]);
    display.print(":");
    display.print(timerSS[timerNo]);
    display.println("");
    display.print("  <OK  ");
    display.print("Cancel");
    break;    
    case 5:
    display.setCursor(0, 10);
    display.print(timerHH[timerNo]);
    display.print(":");
    display.print(timerMM[timerNo]);
    display.print(":");
    display.print(timerSS[timerNo]);
    display.println("");
    display.print("  OK  ");
    display.print("<Cancel");
    break;      
  }
}

void infoScreen(){
  display.setCursor(0, 10);
  display.print(dt.hour);
  display.print(":");
  display.print(dt.minute);
  display.print(":");
  display.print(dt.second);
  display.print("  ");
  display.print(dt.year);
  display.print("-");
  display.print(dt.month);
  display.print("-");
  display.print(dt.day);
  
  display.println("");
  display.print("Tmr 1: ");
  if(timer1Status){
    display.print("ON ");
  }
  else{
    display.print("OFF ");
  }
  display.print("Tmr 2: ");
  if(timer2Status){
    display.print("ON ");
  }
  else{
    display.print("OFF ");
  }
}

void setAlarmDuration(int alarmNo){
  timerDuration[alarmNo-1] = ((timerHH[alarmNo-1]*60*60)+(timerMM[alarmNo-1]*60)+timerSS[alarmNo-1]);
}

void timeSetHandler(uint8_t menu,bool subMenu){
  if(subMenu){
    if(menu == 1){
      timerDuration[0] = ((timerHH[3]*60*60)+(timerMM[3]*60)+timerSS[3]);
    }
    else if(menu == 2){
      timerDuration[1] = ((timerHH[4]*60*60)+(timerMM[4]*60)+timerSS[4]); 
    }
  }
  else{
    if(menu == 1){
      clock.setAlarm1(0, timerHH[0], timerMM[0], timerSS[0], DS3231_MATCH_H_M_S);
    }
    else if(menu == 2){
      clock.setAlarm2(0, timerHH[1], timerMM[1], timerSS[1], DS3231_MATCH_H_M_S);
    }
  }
}
