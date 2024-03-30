#include <FastLED.h>
#include "led-definitions.h"

const uint8_t UPDATES_PER_SECOND = 100;
const uint16_t TIME_PER_EFFECT = 5000;

CRGB leds[NUM_LEDS];

int g_counter = 0; // this is reset to zero when the effect changes, so can be used to reset the effect

// according to the OMS style guide:
// orange     0xEC6936
// blue       0x2097C6
// darkOrange 0xBA532B
// darkBlue   0x167195
const CRGB OtleyMakerSpaceOrange = 0x803000;
const CRGB OtleyMakerSpaceBlue = 0x000080;

// the colour of the "BlinkenLights" LEDs
const CRGB BlinkenLightsColour = CRGB::OrangeRed;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

typedef void (*Effect)();

void setup()
{
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_POWER_MILLIAMPS);
}

void loop()
{
  Effect effect = GetCurrentEffect();
  effect();
  g_counter++;
  FastLED.delay(1000 / UPDATES_PER_SECOND);
}

Effect myEffects[] = {
    // TestCard,
    // WhiteDot,
    // OtleyMakerSpaceLoop,
    // OtleyMakerSpaceLoopWithSparkles,
    // BlinkenLights,
    // BlinkenLightsLoop,
    // Red,
    // HotComet,
    // ForestColors,
    // ColourSparkles,
    // RainbowLoop,
    // RainbowLoopWithSparkles,
    // Dots,
    RandomColoursLoop,
};

void RandomColoursLoop()
{
  static CHSV hsv[NUM_LEDS]; // uses 3 bytes per LED

  if (g_counter == 0)
  {
    for (uint8_t i = 0; i < NUM_LEDS; i++)
    {
      hsv[i] = CHSV(random8(), 255, 255);
    }
  }

  for (uint8_t i = 0; i < NUM_LEDS; i++)
  {
    hsv[i].hue++;
    leds[i] = hsv[i];
  }
}

void Dots()
{
  const uint8_t numDots = 25;
  static uint16_t dotPosition[numDots];
  static uint16_t dotSpeed[numDots];
  static CRGB dotColour[numDots];
  const uint16_t numLeds = NUM_LEDS << 8;

  // initialise random dot positions, speeds and colours
  if (g_counter == 0)
  {
    for (uint8_t i = 0; i < numDots; i++)
    {
      dotColour[i] = CHSV(random8(), 255, 255);       // random colour
      dotPosition[i] = (i * NUM_LEDS / numDots) << 8; // space the dots evenly on the strip

      // ensure each dot has a unique speed
      while (true)
      {
        bool found = false;
        // todo: generate negative speeds
        uint16_t speed = 80 + rand() % 50;
        for (uint8_t j = 0; j < i; j++)
        {
          if (dotSpeed[j] == speed)
          {
            found = true;
          }
        }
        if (!found)
        {
          dotSpeed[i] = speed;
          break;
        }
      }
    }
  }

  fill_solid(leds, NUM_LEDS, CRGB::Black);

  for (uint8_t i = 0; i < numDots; i++)
  {
    int l = dotPosition[i] >> 8;
    leds[l] = dotColour[i];
    dotPosition[i] = (dotPosition[i] + dotSpeed[i]) % numLeds;
  }
}

Effect GetCurrentEffect()
{
  static uint8_t effectIndex = 0;
  static unsigned long targetTime = TIME_PER_EFFECT;

  if (millis() > targetTime)
  {
    targetTime += TIME_PER_EFFECT;
    uint8_t numEffects = sizeof(myEffects) / sizeof(myEffects[0]);
    effectIndex = (effectIndex + 1) % numEffects;
    if (numEffects > 1)
    {
      // don't reset the global counter if there is only one effect
      g_counter = 0;
    }
  }

  return myEffects[effectIndex];
}

void WhiteDot()
{
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  leds[g_counter % NUM_LEDS] = CRGB::White;
}

void OtleyMakerSpaceLoop()
{
  static uint8_t colourIndex = 0;
  fill_palette_circular(leds, NUM_LEDS, colourIndex++, OtleyMakerSpacePalette());
}

void OtleyMakerSpaceLoopWithSparkles()
{
  OtleyMakerSpaceLoop();
  AddSparkles(5);
}

CRGBPalette16 OtleyMakerSpacePalette()
{
  const CRGB o = OtleyMakerSpaceOrange;
  const CRGB b = OtleyMakerSpaceBlue;
  const CRGB _ = CRGB::Black;
  return CRGBPalette16(o, o, o, _, b, b, b, _, o, o, o, _, b, b, b, _);
}

void RainbowLoop()
{
  static uint8_t colourIndex = 0;
  fill_palette_circular(leds, NUM_LEDS, colourIndex++, RainbowColors_p);
}

void RainbowLoopWithSparkles()
{
  RainbowLoop();
  AddSparkles(5);
}

void Red()
{
  fill_solid(leds, NUM_LEDS, CRGB::Red);
}

void ColourSparkles()
{
  const uint8_t delay = 10;
  const uint8_t numSparkles = 20;

  if (g_counter % delay == 0)
  {
    FastLED.clear();
    for (uint8_t i = 0; i < numSparkles; i++)
    {
      int l = rand() % NUM_LEDS;
      leds[l] = CHSV(random8(), 255, 255);
    }
  }
}

void BlinkenLightsLoop()
{
  const uint8_t delay = 5;         // delay between lights scrolling
  const uint8_t probability = 100; // 0-255, higher = more lights

  // initialise the strip
  if (g_counter == 0)
  {
    BlinkenLights();
  }

  if (g_counter % delay == 0)
  {
    for (uint8_t l = NUM_LEDS - 1; l > 0; l--)
    {
      leds[l] = leds[l - 1];
    }
    leds[0] = random8() < probability
                  ? BlinkenLightsColour
                  : CRGB::Black;
  }
}

void BlinkenLights()
{
  const uint8_t delay = 100;       // delay between changes
  const uint8_t probability = 100; // 0-255, higher = more lights

  if (g_counter % delay == 0)
  {
    for (uint8_t l = 0; l < NUM_LEDS; l++)
    {
      leds[l] = random8() < probability
                    ? BlinkenLightsColour
                    : CRGB::Black;
    }
  }
}

void AddSparkles(int numSparkles)
{
  for (uint8_t i = 0; i < numSparkles; i++)
  {
    int l = rand() % NUM_LEDS;
    leds[l] = CRGB::White;
  }
}

void HotComet()
{
  static uint8_t colourIndex = 0;
  fill_palette_circular(leds, NUM_LEDS, colourIndex--, HeatColors_p, 255, LINEARBLEND_NOWRAP);
}

void ForestColors()
{
  static uint8_t colourIndex = 0;
  fill_palette_circular(leds, NUM_LEDS, colourIndex--, ForestColors_p, 255, LINEARBLEND);
}

void TestCard()
{
  CRGBPalette16 palette = CRGBPalette16(
      CRGB::Black, CRGB::Black,
      CRGB::Red, CRGB::Red,
      CRGB::Green, CRGB::Green,
      CRGB::Yellow, CRGB::Yellow,
      CRGB::Blue, CRGB::Blue,
      CRGB::Magenta, CRGB::Magenta,
      CRGB::Cyan, CRGB::Cyan,
      CRGB::White, CRGB::White);
  fill_palette_circular(leds, NUM_LEDS, 1, palette, 255, NOBLEND);
}

// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM = {
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,

    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,

    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black};
