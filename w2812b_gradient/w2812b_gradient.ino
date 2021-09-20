#include <FastLED.h>

//number of total LEDS in this diorama
#define NUM_LEDS 16
//data pin used to control the NEOPIXELS
#define LEDPIN 6

//which LEDS in the strip, make up the lava lake.
#define LAVA_START_LED 8
#define LAVA_END_LED 13

//which LEDS in the strip, light the devil statue
#define DEVIL_START_LED 0
#define DEVIL_END_LED 7

enum STATES {BLENDING_LAVA_COLORS, SELECT_LAVA_COLORS};    
STATES STATE;

DEFINE_GRADIENT_PALETTE(heatmap_gp) {
  0, 255, 255, 0,
  63, 255, 0, 0,
  64, 128, 128, 0,
  127, 255, 0, 0,
  255, 0, 0, 0
};

CRGBPalette16 LAVA_PALETTE = heatmap_gp;

//We have two arrays leds is the value that is currently displayed on the leds
CRGB leds[NUM_LEDS];
//leds_to hold the leds leds values where we are going to blend the LEDS pixels towards
CRGB leds_to[NUM_LEDS];

//pick a random lava color, for each of the lava leds, and set them in the leds_to array
STATES randomLavaColorFromPalette(CRGB leds[], CRGB leds_to[], int lavaStart, int lavaEnd){
  for(int i = LAVA_START_LED; i <= LAVA_END_LED; i++) {
    leds_to[i] = ColorFromPalette(LAVA_PALETTE, random(0, 255));
  }
  return BLENDING_LAVA_COLORS;
}

//convenience method to set the right key/fill light for the devil statue
void rightDevilStrip(CRGB leds[], CRGB color){
    for (int i=(DEVIL_END_LED+1)/2; i<=DEVIL_END_LED; i++) leds[i] = color;
}

//convenience method to set the left key/fill light for the devil statue
void leftDevilStrip(CRGB leds[], CRGB color){
    for (int i=DEVIL_START_LED; i<(DEVIL_END_LED+1)/2; i++) leds[i] = color;
}

//move the cur value towards the taget value an amount at a time.
//the target value needs to be a multiple of amount! Otherwise you won't reach the target value and we keep blending
void stepBlendSourceToDest( uint8_t& cur, const uint8_t target, uint8_t amount)
{
  if( cur == target) return;
  
  if( cur < target ) {
    uint8_t delta = target - cur;
    delta = scale8_video( delta, amount);
    cur += delta;
  } else {
    uint8_t delta = cur - target;
    delta = scale8_video( delta, amount);
    cur -= delta;
  }
}

// Blend one CRGB color toward another CRGB color by a given amount.
// It is important that the color distance, is a multiple of the amount!
// Using amount 1, is guaranteed to work, always!
CRGB fadeTowardColor( CRGB& cur, const CRGB& target, uint8_t amount)
{
  stepBlendSourceToDest( cur.red,   target.red,   amount);
  stepBlendSourceToDest( cur.green, target.green, amount);
  stepBlendSourceToDest( cur.blue,  target.blue,  amount);
  
  return cur;
}

STATES blendTheColors(){
   bool areLedsTheSame = true;
    for (int i=LAVA_START_LED; i <= LAVA_END_LED; i++){
      
      if (leds[i] != leds_to[i]){
        fadeTowardColor(leds[i], leds_to[i], 1);
        areLedsTheSame = false;
      }
      
    } 
    //there are no differences in leds and leds_to anymore, so we can pick new random colors for the lava
    return (areLedsTheSame) ? SELECT_LAVA_COLORS : BLENDING_LAVA_COLORS;
}

void setup() {  
  FastLED.addLeds<NEOPIXEL, LEDPIN>(leds, NUM_LEDS); 

  //set the two led strips that light the Devil in the diorama, to red and yellow
  leftDevilStrip(leds, CRGB(128, 128, 0));
  rightDevilStrip(leds, CRGB(255, 0, 0));

  //initialize the LAVA leds with a random color from the LAVA_PALETTE
  for(int i=LAVA_START_LED; i<=LAVA_END_LED; i++) leds[i] = ColorFromPalette(LAVA_PALETTE, random(0, 255));
  
  //initialize the next lava colors to blend towards (this sets state into BLENDING_LAVA_COLORS);
  STATE = randomLavaColorFromPalette(leds, leds_to, LAVA_START_LED, LAVA_END_LED); 
}

void loop() {
  //Select the a for each of the lava leds a random color from the pallete
  if (STATE == SELECT_LAVA_COLORS) { 
    EVERY_N_MILLISECONDS(800) {
      STATE = randomLavaColorFromPalette(leds, leds_to, LAVA_START_LED, LAVA_END_LED); //moves state into BLENDING_LAVA_COLORS
    }
  }
  
  //fade the lava towards the newly selected random colors
  if (STATE == BLENDING_LAVA_COLORS){
    STATE = blendTheColors(); //moves state into BLENDING_LAVA_COLORS, if any of the leds still dont match leds_to, otherwise it moves into SELECT_LAVA_COLORS
  }

  FastLED.show();
}
