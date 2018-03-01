#include <stdint.h>

#ifndef __STOREDATA_H
#define __STOREDATA_H
typedef struct height_record_t
{
  float lat;
  float lng;
  float height;
}height_record;

void storeRecord(height_record hr);
static void storeFloat(int address, float f);
static float readFloat(int address);

static void printRecord(height_record * hrp);

float testStoreFloat(float f);

void readRecord(int address, height_record * hrp);
void readRecords(int address, int numRecordsToRead);
#endif
