#include <TinyGPS++.h>
#include <SoftwareSerial.h>
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
#define LED_SEP_VAL_MM 6.92f
//distance from base of pole to first led
#define BASE_LED_OFFSET_MM 17
#define NUM_LEDS 144
#define BTN_LEFT 12
#define BTN_RIGHT 13
#define LED_PIN 14
#define BATTERY_PIN A3


#define MIN_VOLTAGE_THRESHOLD 3.70f


int pos = NUM_LEDS - 1;
static const int RXPin = 2, TXPin = 3;
static const uint32_t GPSBaud = 9600;
const int chipSelect = 24;


uint16_t pause = 500;
#define MIN_LED_PAUSE 25
#define MAX_LED_PAUSE 500


//TODO: READ SETTINGS FILE? color/brightness

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);


void error(int error_code)
{
  clearLeds();

  DEBUG_PRINT("Error = ")
  DEBUG_PRINTLN(error_code)
  while (1)
  {
    for (int i = 0; i < (error_code + 1); i++)
    {
      strip.setPixelColor(0, strip.Color(100, 0, 0));
      strip.show();
      delay(200);
      strip.setPixelColor(0, strip.Color(0, 0, 0));
      strip.show();
      delay(200);
    }

    delay(2000);
  }
}

typedef enum ErrorCodes
{
  ERR_LOWBATTERY,
  ERR_NOSDCARD,
  ERR_NOGPS
};

void setup() {

  //Set up buttons
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BATTERY_PIN, INPUT);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(50, 0, 0));
    strip.show();
    delay(5);
  }

  delay(1000);

  clearLeds();

  //Set up UART and softare UART for GPS
  Serial.begin(115200);
  ss.begin(GPSBaud);

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


  DEBUG_PRINT("GPS\t\t[");
  unsigned long startTime = millis();

  //If gps is connected properly we should get a valid sentence from it
  while (gps.sentencesWithFix() >= 1)
  {
    smartDelay(1000);

    if ( (millis() - startTime) > 10000) {
      DEBUG_PRINTLN(" ]");
      error(ERR_NOGPS);
    }
  }
  DEBUG_PRINTLN("X]");


  /*while (!gps.location.isValid())
    {
    printFloat(gps.location.lat(), true, 11, 6);
    printFloat(gps.location.lng(), true, 12, 6);
    smartDelay(1000);
    DEBUG_PRINTLN(".")
    }

    DEBUG_PRINTLN("GOT FIX")*/

}

void clearLeds()
{
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));

  }
  strip.show();
}

void drawGPSStatus()
{
  if (gps.location.isValid()) {
    strip.setPixelColor(0, strip.Color(0, 150, 0));
  }
  else
  {
    strip.setPixelColor(0, strip.Color(0, 0, 150));
  }
  strip.show();
}
void drawPos(int pos)
{
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    if (i == pos)
    {
      strip.setPixelColor(i, strip.Color(50, 50, 50));
    }
    else {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
  }
  strip.show();
}
uint8_t writeHeightToSD(float height)
{

  char latStr[16];
  char lonStr[16];
  char heightStr[10];

  dtostrf(gps.location.lat(), 12, 6, latStr);
  dtostrf(gps.location.lng(), 12, 6, lonStr);
  dtostrf(height, 8, 2, heightStr);
  char output[64];

  TinyGPSDate dt = gps.date;
  TinyGPSTime tm = gps.time;

  int v = 0;
  if(gps.location.isValid())
  {
    v = 1;
  }
  sprintf(output, "%02d:%02d:%02d:%02d:%02d: %02d, %s, %s, %s, %d",
          dt.year(), dt.month(), dt.day(),
          tm.hour(), tm.minute(), tm.second(),
          latStr, lonStr, heightStr, v);

  File dataFile = SD.open("HEIGHT.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.println(output);
    dataFile.close();
    return 1;
  }
  else
  {
    error(ERR_NOSDCARD);
  }
  return 0;

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

    return 0;
  }
  DEBUG_PRINTLN("X]")

  File dataFile = SD.open("GPS.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.println("*Powered on");
    dataFile.close();
    DEBUG_PRINTLN("Test Write Succesfull")
    return 1;
  }
  return 0;
}



void loop() {

  doMeasuringStick();

  if (!checkBatteryVoltage())
  {
    error(ERR_LOWBATTERY);
  }

}


void doMeasuringStick()
{

  //Wait for Left button to be pressed
  while (digitalRead(BTN_LEFT) == 1)
  {
    smartDelay(250);
    //clearLeds();
    drawGPSStatus();
  }

  //Wait for Left button to be released
  while (digitalRead(BTN_LEFT) == 0) {
    smartDelay(10);
  }

  pause = MAX_LED_PAUSE;

  while (1)
  {
    if (digitalRead(BTN_LEFT) == 0)
    {

      smartDelay(pause);
      if (digitalRead(BTN_RIGHT) == 0)
      {
        break;
      }
      if (pause > MIN_LED_PAUSE)
      {
        pause = pause / 2;
      }
      //Serial.println("Button up");
      pos++;
    }
    else if (digitalRead(BTN_RIGHT) == 0)
    {
      smartDelay(pause);
      if (digitalRead(BTN_LEFT) == 0)
      {
        break;
      }
      if (pause > MIN_LED_PAUSE)
      {
        pause = pause / 2;
      }
      //Serial.println("Button down");
      pos--;
    }
    else {
      pause = MAX_LED_PAUSE;
    }

    pos = pos % (NUM_LEDS);

    if (pos < 0 )
    {
      pos = NUM_LEDS - 1;
    }
    Serial.print(pos);
    Serial.print(",");
    Serial.println(calculateHeight(pos));
    drawPos(pos);
    

  }

  float height = calculateHeight(pos);
  Serial.println(height);
  writeHeightToSD(height);
  clearLeds();
  smartDelay(1000);
}

static float calculateHeight(int led_pos)
{
  return BASE_LED_OFFSET_MM + (((NUM_LEDS -1 )- pos) * LED_SEP_VAL_MM);
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

