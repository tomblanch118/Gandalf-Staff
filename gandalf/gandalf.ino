#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include "FastLED.h"
#include "storedata.h"

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    6
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    144
#define BRIGHTNESS          250
#define FRAMES_PER_SECOND  120

//distance between leds on strip
#define LED_SEP_VAL_MM 6.6944f
//distance from base of pole to first led
#define BASE_LED_OFFSET_MM 20
#define BTN_PIN 7

static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);
CRGB leds[NUM_LEDS];


void setup() {
  pinMode(BTN_PIN, INPUT);
  Serial.begin(115200);
  ss.begin(GPSBaud);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  unsigned long start = millis();

#define BUFF_SIZE 10
  char buff[BUFF_SIZE];
  int bp = 0;

  Serial.print("> ");
  while (millis() < ( start + 2000UL )) {
    if (bp >= BUFF_SIZE - 1 )
    {
      break;
    }
    if (Serial.available())
    {
      buff[bp++] = Serial.read();
    }
  }
  buff[bp] = 0;
  int a = atoi(buff);
  Serial.println(a);
  readRecords(0, a);

}

int pos = NUM_LEDS - 1;
int ramp = 500;

height_record hr;

void loop() {
  pos = NUM_LEDS - 1;
  ramp = 500;

  while (digitalRead(BTN_PIN) == 1)
  {
    smartDelay(100);
    if (gps.location.isValid()) {
      leds[0] = CRGB(0, 255, 0);
      FastLED.show();
    }
    else
    {
      leds[0] = CRGB(255, 0, 0);
      FastLED.show();
    }
  }

  //printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
  //printFloat(gps.hdop.hdop(), gps.hdop.isValid(), 6, 1);
  if (gps.location.isValid())
  {
    printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
    printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);

    hr.lat = gps.location.lat();
    hr.lng = gps.location.lng();
  }
  else
  {
    hr.lat = -9999.99f;
    hr.lng = -9999.99f;
  }

  while (digitalRead(BTN_PIN) == 0) {
    smartDelay(10);
  }


  //TODO: led speed should ramp down towards each end and bounce rather than wrap around
  while (digitalRead(BTN_PIN) == 1 ) {
    for ( int i = 0; i < NUM_LEDS; i++) {
      if (i == pos) {
        leds[i] = CRGB(255, 255, 255);
      }
      else
      {
        leds[i] = 0;
      }
    }
    FastLED.show();
    smartDelay(ramp);
    if (ramp > 30)
    {
      ramp *= 0.75;
    }

    if (ramp < 30)
    {
      ramp = 30;
    }

    pos--;
    if (pos < 0 )
    {
      pos = NUM_LEDS - 1;
    }
  }

  hr.height = calculateHeight(pos);
  storeRecord(hr);
  Serial.print(hr.height);
  Serial.println();

  smartDelay(1000);


  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = 0;
  }
  FastLED.show();

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
