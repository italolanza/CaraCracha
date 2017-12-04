// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include <RTClib.h>
#include <Time.h>

RTC_DS1307 RTC;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup () {
  //while (!Serial); // for Leonardo/Micro/Zero

  Serial.begin(9600);
  if (! RTC.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
     //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
 // RTC.adjust(DateTime(1510590150));
}

void loop () {
    DateTime data = RTC.now();
    
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
   
    delay(5000);
}
