#include <Adafruit_NeoPixel.h>
#include "communication.h"

#ifdef __AVR__
  #include <avr/power.h>
#endif

//comments here are my wire colors
#define CLAW_PIN 2  //not colored
#define LED_PIN 3   //dark green
#define BUTTON 8    //yellow
#define LEFT 9      //blue
#define RIGHT 10    //purple
#define UP 11       //green
#define DOWN 12     //white


// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

Adafruit_NeoPixel strip = Adafruit_NeoPixel(88, LED_PIN, NEO_GRB + NEO_KHZ800); // Constructor: number of LEDs, pin number, LED type
Move msgMove; 



const int TIME = 3500;

void setup() 
{
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(CLAW_PIN, OUTPUT);
  pinMode(BUTTON, INPUT);
  pinMode(LEFT, INPUT);
  pinMode(RIGHT, INPUT);
  pinMode(UP, INPUT);
  pinMode(DOWN, INPUT);

  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(CLAW_PIN, LOW);


  strip.begin();
  strip.setBrightness(50);
  strip.show(); // Initialize all pixels to 'off'
}

void refreshButtonState()
{
  //default zeroing:
  msgMove.setDefaultValues();
  if(digitalRead(BUTTON)) msgMove.setClaw();
  if(digitalRead(LEFT)) msgMove.setLeft();
  if(digitalRead(RIGHT)) msgMove.setRight();
  if(digitalRead(UP)) msgMove.setUp();
  if(digitalRead(DOWN)) msgMove.setDown();
  msgMove.sendMsg();
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    //mondjuk ide írhatsz valamit, ami inputot figyeli
    delay(wait);
  }
}


void loop() 
{
  refreshButtonState();
  // Some example procedures showing how to display to the pixels:
  colorWipe(strip.Color(255, 0, 0), 50); // Red
  colorWipe(strip.Color(0, 255, 0), 50); // Green
  colorWipe(strip.Color(0, 0, 255), 50); // Blue
}