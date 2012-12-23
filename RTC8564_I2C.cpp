extern "C" {
  #include <stdlib.h>
  #include <string.h>
  #include <inttypes.h>
}
#include "arduino.h"
#include <I2C.h>
#include "RTC8564_I2C.h"

#define RTC8564_SLAVE_ADRS  (0xA2 >> 1)
#define BCD2Decimal(x)    (((x>>4)*10)+(x&0xf))

// Constructors ////////////////////////////////////////////////////////////////

RTC8564::RTC8564()
  : _seconds(0), _minutes(0), _hours(0), _days(0), _weekdays(0), _months(0), _years(0), _century(0)
{
}

void RTC8564::init(void)
{
  delay(1000);
  I2c.write(RTC8564_SLAVE_ADRS,0x00); //write reg addr 00
  I2c.write(RTC8564_SLAVE_ADRS,0x20); //00 Control 1,S TOP=1
  I2c.write(RTC8564_SLAVE_ADRS,0x00); //01 Control 2
  I2c.write(RTC8564_SLAVE_ADRS,0x00); //02 Seounds
  I2c.write(RTC8564_SLAVE_ADRS,0x00); //03 Minutes
  I2c.write(RTC8564_SLAVE_ADRS,0x09); //04 Hours
  I2c.write(RTC8564_SLAVE_ADRS,0x01); //05 Days
  I2c.write(RTC8564_SLAVE_ADRS,0x01); //06 Weekdays
  I2c.write(RTC8564_SLAVE_ADRS,0x01); //07 Months
  I2c.write(RTC8564_SLAVE_ADRS,0x01); //08 Years
  I2c.write(RTC8564_SLAVE_ADRS,0x00); //09 Minutes Alarm
  I2c.write(RTC8564_SLAVE_ADRS,0x00); //0A Hours Alarm
  I2c.write(RTC8564_SLAVE_ADRS,0x00); //0B Days Alarm
  I2c.write(RTC8564_SLAVE_ADRS,0x00); //0C Weekdays Alarm
  I2c.write(RTC8564_SLAVE_ADRS,0x00); //0D CLKOUT
  I2c.write(RTC8564_SLAVE_ADRS,0x00); //0E Timer control
  I2c.write(RTC8564_SLAVE_ADRS,0x00); //0F Timer
  I2c.write(RTC8564_SLAVE_ADRS,0x00); //00 Control 1, STOP=0
}

// Public Methods //////////////////////////////////////////////////////////////

void RTC8564::begin(void)
{
  I2c.begin();
  if(isvalid() == false) {
    
    init();
    
    if (isInitDatetime()) {
      byte date_time[7];
      date_time[0] = _seconds;
      date_time[1] = _minutes;
      date_time[2] = _hours;
      date_time[3] = _days;
      date_time[4] = _weekdays;
      date_time[5] = _months;
      date_time[6] = _years;
      sync(date_time);
    }
  }
}

void RTC8564::beginWithoutIsValid(void)
{
  I2c.begin();  
  init();
  
  if (isInitDatetime()) {
    byte date_time[7];
    date_time[0] = _seconds;
    date_time[1] = _minutes;
    date_time[2] = _hours;
    date_time[3] = _days;
    date_time[4] = _weekdays;
    date_time[5] = _months;
    date_time[6] = _years;
    sync(date_time);
  }
}

void RTC8564::initDatetime(uint8_t date_time[])
{
  _seconds  = (date_time[0]) ? date_time[0] : 0x00;
  _minutes  = (date_time[1]) ? date_time[1] : 0x00;
  _hours    = (date_time[2]) ? date_time[2] : 0x09;
  _days     = (date_time[3]) ? date_time[3] : 0x01;
  _weekdays = (date_time[4]) ? date_time[4] : 0x01;
  _months   = (date_time[5]) ? date_time[5] : 0x01;
  _years    = (date_time[6]) ? date_time[6] : 0x01;
}

bool RTC8564::isInitDatetime(void)
{
  bool flg = false;
  if ((_seconds  & 0x00) != 0x00) flg = true;
  if ((_minutes  & 0x00) != 0x00) flg = true;
  if ((_hours    & 0x09) != 0x09) flg = true;
  if ((_days     & 0x01) != 0x01) flg = true;
  if ((_weekdays & 0x01) != 0x01) flg = true;
  if ((_months   & 0x01) != 0x01) flg = true;
  if ((_years    & 0x01) != 0x01) flg = true;
  return flg;
}

void RTC8564::sync(uint8_t date_time[],uint8_t size)
{
  I2c.write(RTC8564_SLAVE_ADRS,0x00,0x20);
  I2c.write(RTC8564_SLAVE_ADRS,0x02,date_time,size);
  I2c.write(RTC8564_SLAVE_ADRS,0x00,0x00);
}


void RTC8564::syncInterrupt(unsigned int mode, unsigned long term)
{
  I2c.write(RTC8564_SLAVE_ADRS,0x01,0x11);
  byte buf[2];

  buf[0] = 0x80 | mode;
  buf[1] = term;

  I2c.write(RTC8564_SLAVE_ADRS,0x0E,buf,2);
}

bool RTC8564::available(void)
{
  uint8_t buff[7];

  I2c.read(RTC8564_SLAVE_ADRS,0x02,7);
  for(int i=0; i<7; i++){
    if(I2c.available()){
      buff[i] = I2c.receive();
    }
  }
  _seconds  = buff[0] & 0x7f;
  _minutes  = buff[1] & 0x7f;
  _hours    = buff[2] & 0x3f;
  _days     = buff[3] & 0x3f;
  _weekdays = buff[4] & 0x07;
  _months   = buff[5] & 0x1f;
  _years    = buff[6];
  _century  = (buff[5] & 0x80) ? 1 : 0;
  return (buff[0] & 0x80 ? false : true);
}

bool RTC8564::isvalid(void)
{
  I2c.read(RTC8564_SLAVE_ADRS,0x02,1);
  if(I2c.available()){
    uint8_t buff = I2c.receive();
    return (buff & 0x00 ? false : true);
  }
  return false;
}

bool RTC8564::isInterrupt(void)
{
  I2c.read(RTC8564_SLAVE_ADRS,0x01,1);
  if(I2c.available()){
    return((I2c.receive() & 0x04) != 0x04 ? false : true);
  }
  return false;
}


uint8_t RTC8564::seconds(uint8_t format) const {
  if(format == Decimal) return BCD2Decimal(_seconds);
  return _seconds;
}

uint8_t RTC8564::minutes(uint8_t format) const {
  if(format == Decimal) return BCD2Decimal(_minutes);
  return _minutes;
}

uint8_t RTC8564::hours(uint8_t format) const {
  if(format == Decimal) return BCD2Decimal(_hours);
  return _hours;
}

uint8_t RTC8564::days(uint8_t format) const {
  if(format == Decimal) return BCD2Decimal(_days);
  return _days;
}

uint8_t RTC8564::weekdays() const {
  return _weekdays;
}

uint8_t RTC8564::months(uint8_t format) const {
  if(format == Decimal) return BCD2Decimal(_months);
  return _months;
}

uint8_t RTC8564::years(uint8_t format) const {
  if(format == Decimal) return BCD2Decimal(_years);
  return _years;
}

bool RTC8564::century() const {
  return _century;
}


// Preinstantiate Objects //////////////////////////////////////////////////////

RTC8564 Rtc = RTC8564();