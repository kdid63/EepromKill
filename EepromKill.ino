/*
 Arduino EEPROM killer
 John Boxall - http://tronixstuff.com - May 2011
 CC by-sa
 Note: This sketch will destroy your Arduino's EEPROM 
 Do not use with Arduino boards that have non-replaceable microcontrollers
 Sketch assumes DS1307 already contains current date and time
*/

#include <EEPROM.h>
#include <LiquidCrystal.h> 
LiquidCrystal lcd(4,5,6,7,8,9);

#include "Wire.h"
#define DS1307_I2C_ADDRESS 0x68

// all these bytes necessary for time and date data
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
byte sday, smonth, ssecond, sminute, shour;
byte fday, fmonth, fsecond, fminute, fhour;

// to store number of cycles. Should be enough
long cycles=0; // maximum size is 2,147,483,647

int zz=0;

void setup()
{
  lcd.begin(16, 2); // fire up the LCD
  Wire.begin(); // and the I2C bus
}

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return ( (val/10*16) + (val%10) );
}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );
}

// Gets the date and time from the ds1307
void getDateDs1307(byte *second,
byte *minute,
byte *hour,
byte *dayOfWeek,
byte *dayOfMonth,
byte *month,
byte *year)
{
  // Reset the register pointer
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 7);

  // A few of these need masks because certain bits are control bits
  *second     = bcdToDec(Wire.read() & 0x7f);
  *minute     = bcdToDec(Wire.read());
  *hour       = bcdToDec(Wire.read() & 0x3f);  // Need to change this if 12 hour am/pm
  *dayOfWeek  = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month      = bcdToDec(Wire.read());
  *year       = bcdToDec(Wire.read());
}

void killEEPROM()
{
  boolean dead=false;
  // record start time
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  sday=dayOfMonth;
  smonth=month;
  ssecond=second;
  sminute=minute;
  shour=hour;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Cycles so far...");
  do // write and read EEPROM addresses until the first failure
  {
    for (int a=0; a<1024; a++)
    {
      EEPROM.write(a, 170); // write binary 10101010 to each EEPROM address
    }
    for (int a=0; a<1024; a++) // check each address
    {
      zz = EEPROM.read(a);
      if (zz!=170)
      {
        dead=true; // uh-oh, an address has died
        cycles-=2; // easy way out to calculate cycles once died
      }
    }
    cycles++;
    lcd.setCursor(0,1);
    lcd.print(cycles);
    for (int a=0; a<1024; a++)
    {
      EEPROM.write(a, 85); // write binary 01010101 to each EEPROM address
    }
    for (int a=0; a<1024; a++) // check each address
    {
      zz = EEPROM.read(a);
      if (zz!=85)
      {
        dead=true; // uh-oh, an address has died
        --cycles; // easy way out to calculate cycles once died
      }
    }
    cycles++;
    lcd.setCursor(0,1);
    lcd.print(cycles); // update the LCD
  }
  while (dead!=true);
  // so now one address write/read has failed, the game's up
  // record end time
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  fday=dayOfMonth;
  fmonth=month;
  fsecond=second;
  fminute=minute;
  fhour=hour;
  // display final statistics (endless loop)
  do
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Start date/time: ");
    lcd.setCursor(1,1);    
    lcd.print(sday, DEC);
    lcd.print("/");
    lcd.print(smonth, DEC);
    lcd.print(" ");
    lcd.print(shour, DEC);
    lcd.print(":");    
    if (sminute<10)
    {
      lcd.print("0");
    }
    lcd.print(sminute, DEC);
    lcd.print(":");
    if (ssecond<10)
    {
      lcd.print("0");
    }
    lcd.print(ssecond, DEC);
    delay(2000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("End date/time: ");
    lcd.setCursor(1,1);    
    lcd.print(fday, DEC);
    lcd.print("/");
    lcd.print(fmonth, DEC);
    lcd.print(" ");
    lcd.print(fhour, DEC);
    lcd.print(":");
    if (fminute<10)
    {
      lcd.print("0");
    }
    lcd.print(fminute, DEC);
    lcd.print(":");
    if (fsecond<10)
    {
      lcd.print("0");
    }
    lcd.print(fsecond, DEC);
    delay(2000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("  # of cycles:");
    lcd.setCursor(0,1);
    lcd.print(cycles);
    delay(2000);
  }
  while(1>0);
}

void loop()
{ // nice intro display with countdown
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("EEPROM Destroyer");
  lcd.setCursor(0,1);
  for (int a=99; a>0; --a)
  // gives user 99 seconds to change mind before start
  {
    lcd.setCursor(0,1);
    lcd.print("Starting in ");
    lcd.print(a);
    lcd.print("s ");
    delay(995);
  }
  lcd.clear();
  killEEPROM();
}

