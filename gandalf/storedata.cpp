#include "storedata.h"
#include <Arduino.h>
#include <EEPROM.h>

static int record_pointer;
const size_t stride = sizeof(height_record);
const int storage_size = E2END;

void storeRecord(height_record hr)
{
  if ( (storage_size - 1 - stride) >= record_pointer)
  {
    int tmpAddr = record_pointer;
    storeFloat(tmpAddr, hr.lat);
    tmpAddr+=4;
    storeFloat(tmpAddr, hr.lng);
    tmpAddr+=4;
    storeFloat(tmpAddr, hr.height);

    record_pointer += stride;
  }
}

void readRecords(int startAddress, int num)
{
  for(int index = 0; index < num; index++)
  {
    if(startAddress >storage_size)
    {
      break;
    }
    height_record tmp;
    readRecord(startAddress, &tmp);
    printRecord(&tmp);
    startAddress+=stride;
  }
}

static void printRecord(height_record * hrp)
{
  Serial.print(hrp->lat, 6);
  Serial.print(",");
  Serial.print(hrp->lng, 6);
  Serial.print(",");
  Serial.println(hrp->height,2);
}
float testStoreFloat(float f)
{
  storeFloat(0, f);
  return readFloat(0);
}

void readRecord(int address, height_record * hrp)
{
  hrp->lat = readFloat(address);
  address +=4;
  hrp->lng = readFloat(address);
  address +=4;
  hrp->height = readFloat(address);
}

static void storeFloat(int address, float f)
{
  EEPROM.write(address++, (uint8_t) (*((uint32_t *)&f)>>24) );
  EEPROM.write(address++, (uint8_t) (*((uint32_t *)&f)>>16) );
  EEPROM.write(address++, (uint8_t) (*((uint32_t *)&f)>>8) );
  EEPROM.write(address, (uint8_t) (*((uint32_t *)&f)) );
}

static float readFloat(int address)
{
  uint32_t tmp = 0;
  tmp |= ((uint32_t)EEPROM.read(address++) << 24);
  tmp |= ((uint32_t)EEPROM.read(address++) << 16);
  tmp |= ((uint32_t)EEPROM.read(address++) << 8);
  tmp |= ((uint32_t)EEPROM.read(address));
  
  return *((float *)&tmp);
}

