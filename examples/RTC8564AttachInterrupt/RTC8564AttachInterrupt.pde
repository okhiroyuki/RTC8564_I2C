/*
 * sketch name : RTC8564AttachInterrupt
 * summary     : RTC8564 print datetime
 */

#include <I2C.h>
#include <RTC8564_I2C.h>

/* RTC timer start datetime */
#define RTC_SEC  0x00  // seconds
#define RTC_MIN  0x00  // minute
#define RTC_HOUR 0x22  // hour
#define RTC_DAY  0x01  // day
#define RTC_WEEK 0x02  // weed(00:Sun 〜 06:Sat)
#define RTC_MON  0x03  // month
#define RTC_YEAR 0x11  // year
byte date_time[7] = {
   RTC_SEC
  ,RTC_MIN
  ,RTC_HOUR
  ,RTC_DAY
  ,RTC_WEEK
  ,RTC_MON
  ,RTC_YEAR
};

/* periodic timer interval unit  0:4096Hz,1:64Hz,2:seconds(1Hz),1:minute(1/60Hz) */
#define RTC_INTERRUPT_MODE 2

/* measurement interval(RTC interrupt interval) */
#define RTC_INTERRUPT_TERM 1

/* external interrupt pin */
#define RTC_INTERRUPT_PIN  2

/* measurements enable flag */
volatile int flg = LOW;

void setup() {
  
  Serial.begin(9600);
  
  // RTC datetime set
  Rtc.initDatetime(date_time);
  
  // RTC start
  Rtc.beginWithoutIsValid();
  
  // RTC periodic timer interval check continuity
  Rtc.syncInterrupt(RTC_INTERRUPT_MODE, RTC_INTERRUPT_TERM);
  
  // interrupt init
  pinMode(RTC_INTERRUPT_PIN, INPUT);
  digitalWrite(RTC_INTERRUPT_PIN, HIGH);
  attachInterrupt(0, printDatetime, FALLING);
}

void loop() {
  if (flg) {
    ReadRTC();
    flg = LOW;
  }
}

void printDatetime(void)
{
  flg = HIGH;
}

void ReadRTC()
{
  Rtc.available();
  Serial.print(Rtc.years(), HEX);
  Serial.print("/");
  Serial.print(Rtc.months(), HEX);
  Serial.print("/");
  Serial.print(Rtc.days(), HEX);
  Serial.print(" ");
  Serial.print(Rtc.hours(), HEX);
  Serial.print(":");
  Serial.print(Rtc.minutes(), HEX);
  Serial.print(":");
  Serial.print(Rtc.seconds(), HEX);
  Serial.print(" ");
  Serial.println((int)Rtc.weekdays());
}
