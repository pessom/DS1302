#ifndef DS1302_H
#define DS1302_H

#define DS1302_CE_PIN     11   //Reset: 'RST'
#define DS1302_IO_PIN     12   //DATA:  'DAT'
#define DS1302_SCLK_PIN   13   //CLOCK: 'CLK'

#define bcd2bin(h, l)    (((h) * 10) + (l))
#define bin2bcd_h(x)     ((x) / 10)
#define bin2bcd_l(x)     ((x) % 10)

#define DS1302_SECONDS           0x80
#define DS1302_MINUTES           0x82
#define DS1302_HOURS             0x84
#define DS1302_DATE              0x86
#define DS1302_MONTH             0x88
#define DS1302_DAY               0x8A
#define DS1302_YEAR              0x8C
#define DS1302_ENABLE            0x8E
#define DS1302_TRICKLE           0x90
#define DS1302_CLOCK_BURST       0xBE
#define DS1302_CLOCK_BURST_WRITE 0xBE
#define DS1302_CLOCK_BURST_READ  0xBF
#define DS1302_RAMSTART          0xC0
#define DS1302_RAMEND            0xFC
#define DS1302_RAM_BURST         0xFE
#define DS1302_RAM_BURST_WRITE   0xFE
#define DS1302_RAM_BURST_READ    0xFF

#define DS1302_D0 0
#define DS1302_D1 1
#define DS1302_D2 2
#define DS1302_D3 3
#define DS1302_D4 4
#define DS1302_D5 5
#define DS1302_D6 6
#define DS1302_D7 7

#define DS1302_READBIT DS1302_D0
#define DS1302_RC      DS1302_D6
#define DS1302_CH      DS1302_D7
#define DS1302_AM_PM   DS1302_D5
#define DS1302_12_24   DS1302_D7
#define DS1302_WP      DS1302_D7
#define DS1302_ROUT0   DS1302_D0
#define DS1302_ROUT1   DS1302_D1
#define DS1302_DS0     DS1302_D2
#define DS1302_DS1     DS1302_D2
#define DS1302_TCS0    DS1302_D4
#define DS1302_TCS1    DS1302_D5
#define DS1302_TCS2    DS1302_D6
#define DS1302_TCS3    DS1302_D7



typedef struct DS1302
{
  uint8_t Seconds:4;
  uint8_t Seconds10:3;
  uint8_t CH:1;
  uint8_t Minutes:4;
  uint8_t Minutes10:3;
  uint8_t reserved1:1;
  union
  {
    struct
    {
      uint8_t Hour:4;
      uint8_t Hour10:2;
      uint8_t reserved2:1;
      uint8_t hour_12_24:1;
    } h24;
    struct
    {
      uint8_t Hour:4;
      uint8_t Hour10:1;
      uint8_t AM_PM:1;
      uint8_t reserved2:1;
      uint8_t hour_12_24:1;
    } h12;
  };
  uint8_t Date:4;
  uint8_t Date10:2;
  uint8_t reserved3:2;
  uint8_t Month:4;
  uint8_t Month10:1;
  uint8_t reserved4:3;
  uint8_t Day:3;
  uint8_t reserved5:5;
  uint8_t Year:4;
  uint8_t Year10:4;
  uint8_t reserved6:7;
  uint8_t WP:1;
};


extern DS1302 rtc;



void _DS1302_start(void)
{
  digitalWrite(DS1302_CE_PIN, LOW);
  pinMode(DS1302_CE_PIN, OUTPUT);  
  digitalWrite(DS1302_SCLK_PIN, LOW);
  pinMode(DS1302_SCLK_PIN, OUTPUT);
  pinMode(DS1302_IO_PIN, OUTPUT);
  digitalWrite(DS1302_CE_PIN, HIGH);
  delayMicroseconds(4);
}


void _DS1302_stop(void)
{
  digitalWrite(DS1302_CE_PIN, LOW);
  delayMicroseconds(4);
}


uint8_t _DS1302_toggleread(void)
{
  uint8_t i, data;
  data = 0;
  for(i = 0; i <= 7; i++)
  {
    digitalWrite(DS1302_SCLK_PIN, HIGH);
    delayMicroseconds(1);
    digitalWrite(DS1302_SCLK_PIN, LOW);
    delayMicroseconds(1);
    bitWrite(data, i, digitalRead(DS1302_IO_PIN));
  }

  return(data);
}


void _DS1302_togglewrite(uint8_t data, uint8_t release)
{
  for(int i = 0; i <= 7; i++)
  { 
    digitalWrite(DS1302_IO_PIN, bitRead(data, i));  
    delayMicroseconds(1);
    digitalWrite(DS1302_SCLK_PIN, HIGH);     
    delayMicroseconds(1);
    if(release && i == 7)
    {
      pinMode(DS1302_IO_PIN, INPUT);

      //For Arduino 1.0.3, removing the pull-up is no longer needed.
      //Setting the pin as 'INPUT' will already remove the pull-up.
      //digitalWrite (DS1302_IO, LOW); //remove any pull-up  
    }
    else
    {
      digitalWrite(DS1302_SCLK_PIN, LOW);
      delayMicroseconds(1);
    }
  }
}


uint8_t DS1302_read(int address)
{
  uint8_t data;
  bitSet(address, DS1302_READBIT);  
  _DS1302_start();
  _DS1302_togglewrite(address, true);  
  data = _DS1302_toggleread();
  _DS1302_stop();

  return(data);
}


void DS1302_write(int address, uint8_t data)
{
  bitClear(address, DS1302_READBIT);   
  _DS1302_start();
  _DS1302_togglewrite(address, false); 
  _DS1302_togglewrite(data, false); 
  _DS1302_stop();  
}


void DS1302_clock_burst_read(uint8_t *p)
{
  _DS1302_start();
  _DS1302_togglewrite(DS1302_CLOCK_BURST_READ, true);  
  for(int i = 0; i < 8; i++)
  {
    *p++ = _DS1302_toggleread();
  }
  _DS1302_stop();
}


void DS1302_clock_burst_write(uint8_t *p)
{
  _DS1302_start();
  _DS1302_togglewrite(DS1302_CLOCK_BURST_WRITE, false);  
  for(int i = 0; i < 8; i++)
  {
    _DS1302_togglewrite(*p++, false);  
  }
  _DS1302_stop();
}



void RTC_Init(int seconds, int minutes, int hours, int dayofweek, int dayofmonth, int month, int year)
{
  DS1302_write(DS1302_ENABLE, 1);
  DS1302_write(DS1302_TRICKLE, 0x00);
  memset((char *) &rtc, 0, sizeof(rtc));
  rtc.Seconds    = bin2bcd_l(seconds);
  rtc.Seconds10  = bin2bcd_h(seconds);
  rtc.CH         = 0;
  rtc.Minutes    = bin2bcd_l(minutes);
  rtc.Minutes10  = bin2bcd_h(minutes);
  //rtc.h12.Hour   = bin2bcd_l(hours);
  //rtc.h12.Hour10 = bin2bcd_h(hours);
  //rtc.h12.AM_PM  = 0;         //AM = 0, PM = 1
  //rtc.h12.hour_12_24 = 1;       //1 for 24 hour format, 0 for 12 hour format
  rtc.h24.Hour   = bin2bcd_l(hours);
  rtc.h24.Hour10 = bin2bcd_h(hours);
  rtc.h24.hour_12_24 = 0;         //0 for 24 hour format, 1 for 12 hour format
  rtc.Date       = bin2bcd_l(dayofmonth);
  rtc.Date10     = bin2bcd_h(dayofmonth);
  rtc.Month      = bin2bcd_l(month);
  rtc.Month10    = bin2bcd_h(month);
  rtc.Day        = dayofweek + 1;     //Format Français (Lundi = 1er Jour de la semaine)
  //rtc.Day        = dayofweek;     //English Format (Sunday = 1st Day of the week)
  rtc.Year       = bin2bcd_l(year - 2000);
  rtc.Year10     = bin2bcd_h(year - 2000);
  rtc.WP = 0;  
  DS1302_clock_burst_write((uint8_t *) &rtc);
}


void RTC_Read()
{
  char buffer[80];
  DS1302_clock_burst_read((uint8_t *) &rtc);
  sprintf(buffer, "Time = %02d:%02d:%02d, ", \
      bcd2bin(rtc.h24.Hour10, rtc.h24.Hour), \
      bcd2bin(rtc.Minutes10, rtc.Minutes), \
      bcd2bin(rtc.Seconds10, rtc.Seconds));
    Serial.print(buffer);

    sprintf(buffer, "Date(day of month) = %d, Month = %d, " \
      "Day(day of week) = %d, Year = %d", \
      bcd2bin(rtc.Date10, rtc.Date), \
      bcd2bin(rtc.Month10, rtc.Month), \
      rtc.Day, \
      2000 + bcd2bin(rtc.Year10, rtc.Year));
    Serial.println(buffer);
}


#endif
