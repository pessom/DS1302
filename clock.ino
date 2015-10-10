#import <Arduino.h>
#include "DS1302.h"

DS1302 rtc;
void setup()
{
  Serial.begin(9600);
//  RTC_Init(int seconds, int minutes, int hours, int dayofweek, int dayofmonth, int month, int year)
  int seconds = 0;
  int minutes = 37;
  int hours = 20;
  int dayofweek = 6;
  int dayofmonth = 10;
  int month = 10;
  int year = 2015;
//  RTC_Init(seconds, minutes, hours, dayofweek, dayofmonth, month, year);
}

void loop()
{
  RTC_Read();
  delay(5000); //Delay between reading
}
