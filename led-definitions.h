#define LED_PIN 2               // the Arduino data pin connected to Din on the LED strip
#define NUM_LEDS 144            // number of LEDs in the strip
#define BRIGHTNESS 25           // global brightness (keep it low to reduce power)
#define LED_TYPE WS2811         // type of LEDs
#define COLOR_ORDER GRB         // colour order
#define MAX_POWER_MILLIAMPS 500 // limits the maximum power to the strip so it can be directly driven by an Arduino
