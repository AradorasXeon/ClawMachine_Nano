#include <Adafruit_NeoPixel.h>
#include "communication.h"

#ifdef __AVR__
  #include <avr/power.h>
#endif

//comments here are my wire colors
#define CLAW_PIN 2  //not colored --> soldered directly to transistor
#define LED_PIN 3   //dark green
#define BUTTON 8    //yellow
#define LEFT 9      //blue
#define RIGHT 10    //purple
#define UP 11       //green
#define DOWN 12     //white

//nth LED, we can use these to light up appropaite LEDs for left, right, up, down movements
#define LED_LEFT_MIN 0
#define LED_LEFT_MAX 10
#define LED_RIGHT_MIN 11
#define LED_RIGHT_MAX 20
#define LED_UP_MIN 21
#define LED_UP_MAX 30
#define LED_DOWN_MIN 31
#define LED_DOWN_MAX 40

#define LED_LAST 88
#define LED_BRIGHTNESS 50


// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_LAST, LED_PIN, NEO_GRB + NEO_KHZ800); // Constructor: number of LEDs, pin number, LED type
//88 LEDs @50 strength white about 700 mA
//88 LEDs @80 strength white about 1070 mA
//88 LEDs @100 strength white about 1290 mA
//100 LEDs @100 strength white about 1310 mA

Move msgMove(true);

/// @brief Fills part of the actual Adafruit_NeoPixel led strip
/// @param r red
/// @param g green
/// @param b blue
/// @param startLed from this LED
/// @param endLed to this LED
void FillStripPart(uint8_t r, uint8_t g, uint8_t b, uint16_t startLed, uint16_t endLed)
{

  for(uint16_t i=startLed; i<=endLed; i++) 
  {
    strip.setPixelColor(i, strip.Color(r, g, b));
    delay(1);
  }

  strip.show();
}

void LedClawAction()
{
  for(int i = 0; i<20; i++) //20 for test SHOULD TIME IT after claw touches down
  {
    FillStripPart(255, 0, 255, 0, LED_LAST);
    delay(50);
  }
}

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
  strip.setBrightness(LED_BRIGHTNESS);
  FillStripPart(1, 1, 1, 0, LED_LAST);

  strip.show(); // Initialize all pixels to 'off'
}

void MoveLeft()
{
  msgMove.setLeft();
  FillStripPart(0, 255, 0, LED_LEFT_MIN, LED_LEFT_MAX);
}

void MoveRight()
{
  msgMove.setRight();
  FillStripPart(255, 0, 0, LED_RIGHT_MIN, LED_RIGHT_MAX);

}

void MoveUp()
{
  msgMove.setUp();
  FillStripPart(0, 0, 255, LED_UP_MIN, LED_UP_MAX);

}

void MoveDown()
{
  msgMove.setDown();
  FillStripPart(0, 255, 255, LED_DOWN_MIN, LED_DOWN_MAX);
}

//stops inputs for a while
void ClawAction()
{
  msgMove.setClawDown();
  msgMove.sendMsg();
  LedClawAction(); //contains delay have to send msg beforehand!
  digitalWrite(CLAW_PIN, HIGH); //Closes claw
  FillStripPart(255, 255, 0, 0, LED_LAST); //White light to see everything nice and clear
  msgMove.setClawUp();
  msgMove.sendMsg();
  delay(2000); //will see the numbers
  //will need some iteration
  msgMove.setLeft();
  msgMove.setDown();
  //end of iteration
  digitalWrite(CLAW_PIN, LOW); //Opens claw
}

void refreshButtonState()
{
  //default zeroing:
  if(digitalRead(BUTTON) == HIGH)
  {
    ClawAction();
  }
   
  if(digitalRead(LEFT) == HIGH)
  {
    MoveLeft();
  }
   
  if(digitalRead(RIGHT) == HIGH)
  {
    MoveRight();
  }
   
  if(digitalRead(UP) == HIGH)
  {
    MoveUp();
  }
   
  if(digitalRead(DOWN) == HIGH)
  {
    MoveDown();
  }
   
}

void loop() 
{
  msgMove.setDefaultValues();
  refreshButtonState();
  msgMove.sendMsg();
  FillStripPart(1, 1, 1, 0, LED_LAST);
}