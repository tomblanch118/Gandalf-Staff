/**
   5/7 buttons white/yellow
  6 LEDS green

  GPS
  yellow 3
  green 4



*/

#include <TinyGPS++.h>
#include <SoftwareSerial.h>
//#include "FastLED.h"
#include <Adafruit_NeoPixel.h>

#include <SD.h>
#include <SPI.h>

#define __DEBUG

#ifdef __DEBUG
#define DEBUG_PRINT(x) Serial.print(x);
#define DEBUG_PRINTLN(x) Serial.println(x);
#else
#define DEBUG_PRINT(x) ;
#define DEBUG_PRINTLN(x) ;
#endif

//distance between leds on strip
#define LED_SEP_VAL_MM 6.6944f
//distance from base of pole to first led
#define BASE_LED_OFFSET_MM 20
#define NUM_LEDS 144
#define BTN_PIN 12
#define BTN2_PIN 13
#define LED_PIN 14
#define BATTERY_PIN A3


#define MIN_VOLTAGE_THRESHOLD 3.70f
int pos = NUM_LEDS - 1;
static const int RXPin = 2, TXPin = 3;
static const uint32_t GPSBaud = 9600;
const int chipSelect = 24;


//Adafruit_NeoPixel strip = Adafruit_NeoPixel(144, 12, NEO_GRB + NEO_KHZ800);

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);


void error(int error_code)
{
  //TODO: flash lights in number of times based on the error code or something
  DEBUG_PRINT("Error ")
  DEBUG_PRINTLN(error_code)
  while (1);
}

typedef enum ErrorCodes
{
  ERR_LOWBATTERY,
  ERR_NOSDCARD,
  ERR_NOGPS
};

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void setup() {

  //Set up buttons
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
  pinMode(BATTERY_PIN, INPUT);

  //strip.begin();
  //strip.show(); // Initialize all pixels to 'off'

  // colorWipe(strip.Color(255, 0, 0), 50); // Red

  //Set up UART and softare UART for GPS
  Serial.begin(115200);
  ss.begin(GPSBaud);

  //TODO: try opening the file to make sure we can write?
  //Check if there is an SD Card
  if (!init_sd_card())
  {
    error(ERR_NOSDCARD);
  }

  //Check Voltage is above reasonable threshold
  if (!checkBatteryVoltage())
  {
    error(ERR_LOWBATTERY);
  }

  //If gps is connected properly we should get a valid sentence from it
  while (gps.sentencesWithFix() >= 1)
  {
    smartDelay(1000);
  }
  DEBUG_PRINTLN("GPS\t\t[X]");

  while (!gps.location.isValid())
  {
    printFloat(gps.location.lat(), true, 11, 6);
    printFloat(gps.location.lng(), true, 12, 6);
    smartDelay(1000);
    DEBUG_PRINTLN(".")
  }

  DEBUG_PRINTLN("GOT FIX")

  Serial.println( freeRam ());
  File dataFile = SD.open("datalug.txt", FILE_WRITE);
  Serial.println( freeRam ());
  if (dataFile) {
    dataFile.println("TEST STRING PLS IGNORE 1");
    dataFile.println("TEST STRING PLS IGNORE 2");
    dataFile.println("TEST STRING PLS IGNORE 3");
    dataFile.println("TEST STRING PLS IGNORE 4");
    dataFile.close();
    Serial.println("DONE");

  }
  Serial.println("?");

}

inline uint8_t checkBatteryVoltage()
{
  DEBUG_PRINT("Battery\t\t")
  float batteryVoltage = getBatteryVoltage();
  DEBUG_PRINTLN(batteryVoltage)

  if (batteryVoltage >= MIN_VOLTAGE_THRESHOLD)
  {
    return 1;
  }
  return 0;
}

inline float getBatteryVoltage()
{
  const int numberVoltageSamples = 10;

  int16_t averagedADC = 0;

  for (int i = 0; i < numberVoltageSamples; i++)
  {
    averagedADC += analogRead(BATTERY_PIN);
  }

  float averagedBatteryADC = (float)averagedADC / (float)numberVoltageSamples;

  float batteryVoltage = 0.02f + (averagedBatteryADC * 0.004286133f);

  return batteryVoltage;
}

uint8_t init_sd_card()
{
  DEBUG_PRINT("SDCard\t\t[")
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    DEBUG_PRINTLN(" ]")
    // don't do anything more:
    return 0;
  }
  DEBUG_PRINTLN("X]")
  return 1;
}



void loop() {
  //powerDown();
  doMeasuringStick();
  //is button pressed

}


void doMeasuringStick()
{
  // pos = NUM_LEDS - 1;
  int  ramp = 500;

  while (digitalRead(BTN_PIN) == 1)
  {
    smartDelay(100);
    if (gps.location.isValid()) {
      //SHOW GREEN
    }
    else
    {
      //SHOW RED
    }
  }

  //printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
  //printFloat(gps.hdop.hdop(), gps.hdop.isValid(), 6, 1);
  if (gps.location.isValid())
  {
    printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
    printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);

    //hr.lat = gps.location.lat();
    //   hr.lng = gps.location.lng();
  }
  else
  {
    // hr.lat = -9999.99f;
    // hr.lng = -9999.99f;
  }

  while (digitalRead(BTN_PIN) == 0) {
    smartDelay(10);
  }


  //TODO: led speed should ramp down towards each end and bounce rather than wrap around
  while ( (digitalRead(BTN_PIN) == 1 ) || (digitalRead(BTN2_PIN) == 1) ) {
    //Serial.println("Waiting for btn");
    // drawPos(pos);

    if (digitalRead(BTN_PIN) == 0)
    {
      Serial.println("Button up");
      pos++;
      smartDelay(50);
    }
    else if (digitalRead(BTN2_PIN) == 0)
    {
      Serial.println("Button down");
      pos--;
      smartDelay(50);
    }
    pos = pos % (NUM_LEDS - 1);

    if (pos < 0 )
    {
      pos = NUM_LEDS - 1;
    }

  }

  smartDelay(1000);
}

static float calculateHeight(int led_pos)
{
  return BASE_LED_OFFSET_MM + ((NUM_LEDS - pos) * LED_SEP_VAL_MM);
}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

static void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(',');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i = flen; i < len; ++i)
      Serial.print(',');
  }
  smartDelay(0);
}

static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len - 1] = ' ';
  Serial.print(sz);
  smartDelay(0);
}

static void printDateTime(TinyGPSDate & d, TinyGPSTime & t)
{
  if (!d.isValid())
  {
    Serial.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
  }

  if (!t.isValid())
  {
    Serial.print(F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    Serial.print(sz);
  }

  printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}

static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i = 0; i < len; ++i)
    Serial.print(i < slen ? str[i] : ' ');
  smartDelay(0);
}
