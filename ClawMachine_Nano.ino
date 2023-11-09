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

#define START_UP_CALIB_TIME_MS 3000
#define CALIB_MSG_DELAY_MICROSECONDS 200 //for resending msg
#define CALIB_MSG_WRITE_READ_MILISECONDS 10 //amount of time between reads

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

/// @brief Fills part of the actual Adafruit_NeoPixel led strip SLOWLY
/// @param r red
/// @param g green
/// @param b blue
/// @param startLed from this LED
/// @param endLed to this LED
void FillStripPartSlow(uint8_t r, uint8_t g, uint8_t b, uint16_t startLed, uint16_t endLed)
{

  for(uint16_t i=startLed; i<=endLed; i++) 
  {
    strip.setPixelColor(i, strip.Color(r, g, b));
    delay(40);
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

int zCurrentlyAt = -1; //-1 will represent error state

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

  //Calibration init
  int time = millis();
  while(millis()-time < START_UP_CALIB_TIME_MS)
  {
    //maybe some skip possibilities here later
    //probably if something pressed then set a boolean
  }
  doCalibration();
  FillStripPartSlow(180, 0, 255, 0, LED_LAST);
  FillStripPartSlow(1, 1, 1, 0, LED_LAST);

}

void MoveLeft()
{
  msgMove.setLeft();
  FillStripPart(0, 255, 0, LED_LEFT_MIN, LED_LEFT_MAX);
}

void MoveRight()
{
  msgMove.setRight();
  FillStripPart(0, 255, 0, LED_RIGHT_MIN, LED_RIGHT_MAX);

}

void MoveUp()
{
  msgMove.setUp();
  FillStripPart(0, 255, 0, LED_UP_MIN, LED_UP_MAX);

}

void MoveDown()
{
  msgMove.setDown();
  FillStripPart(0, 255, 0, LED_DOWN_MIN, LED_DOWN_MAX);
}

//stops inputs for a while
void ClawAction()
{
  /*
  msgMove.setClawDown();
  msgMove.sendMsg(COMMUNICATION_MOVEMENT);
  LedClawAction(); //contains delay have to send msg beforehand!
  digitalWrite(CLAW_PIN, HIGH); //Closes claw
  FillStripPart(255, 255, 255, 0, LED_LAST); //White light to see everything nice and clear
  msgMove.setClawUp();
  msgMove.sendMsg(COMMUNICATION_MOVEMENT);
  delay(2000); //will see the numbers
  //will need some iteration
  msgMove.setLeft();
  msgMove.setDown(); 
  //end of iteration
  digitalWrite(CLAW_PIN, LOW); //Opens claw
  */
}

bool containsGivenBits(uint8_t inThis, uint8_t contained)
{
  uint8_t temp = inThis & contained;
  if(temp == contained)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void doCalibration()
{
  INIT_CALIB:
  msgMove.initCalibration();
  msgMove.sendMsg(COMMUNICATION_CALIBRATION);
  delayMicroseconds(CALIB_MSG_DELAY_MICROSECONDS); //watch out for LTO might need to change Arduino.h
  msgMove.readFromSlave();
  //check if msg got through if not send again
  if (msgMove.getMovementState().calibState != Claw_Calibration::CLAW_CALIB_INIT) goto INIT_CALIB;



  START_TOP_CALIB:
  msgMove.startTopCalib();
  msgMove.sendMsg(COMMUNICATION_CALIBRATION);
  delayMicroseconds(CALIB_MSG_DELAY_MICROSECONDS);
  msgMove.readFromSlave();
  //check if msg got through if not send again
  if (!containsGivenBits(msgMove.getMovementState().calibState, Claw_Calibration::CLAW_CALIB_TOP_STATE_IN_PROGRESS)) goto START_TOP_CALIB;
  //give some indication that calibration has started
  FillStripPart(19, 0, 255, 0, LED_LAST);

  while(!containsGivenBits(msgMove.getMovementState().calibState, Claw_Calibration::CLAW_CALIB_TOP_DONE | Claw_Calibration::CLAW_CALIB_BAD))
  {
    delay(CALIB_MSG_WRITE_READ_MILISECONDS); //we only read after some ms so and repeat, 
    msgMove.readFromSlave();
    refreshButtonState();
    msgMove.sendMsg(COMMUNICATION_MOVEMENT); //we also have to put out the movement commands
  }
  //we exit while loop when something is bad or CALIB TOP IS DONE

  if(containsGivenBits(msgMove.getMovementState().calibState, Claw_Calibration::CLAW_CALIB_BAD))
  {
    //do some error handling stuff
    FillStripPart(1, 1, 1, 0, LED_LAST);
  }
  else if(containsGivenBits(msgMove.getMovementState().calibState, Claw_Calibration::CLAW_CALIB_TOP_DONE))
  {
    zCurrentlyAt = 0; //do we need this here?
    START_DOWN_CALIB:
    msgMove.startDownCalib();
    msgMove.sendMsg(COMMUNICATION_CALIBRATION);
    delayMicroseconds(CALIB_MSG_DELAY_MICROSECONDS);
    msgMove.readFromSlave();
    //check if msg got through if not send again
    if (!containsGivenBits(msgMove.getMovementState().calibState, Claw_Calibration::CLAW_CALIB_DOWN_STATE_IN_PROGRESS)) goto START_DOWN_CALIB;
    //give indication of that calibration is in the next phase
    FillStripPart(80, 0, 255, 0, LED_LAST);

    while(!containsGivenBits(msgMove.getMovementState().calibState, Claw_Calibration::CLAW_CALIB_DOWN_DONE | Claw_Calibration::CLAW_CALIB_BAD))
    {
      delay(CALIB_MSG_WRITE_READ_MILISECONDS);
      msgMove.readFromSlave();
      refreshButtonState();
      msgMove.sendMsg(COMMUNICATION_MOVEMENT);
    }
    //we exit while loop when something is bad or CALIB DOWN IS DONE
    if(containsGivenBits(msgMove.getMovementState().calibState, Claw_Calibration::CLAW_CALIB_BAD))
    {
      //do some error handling stuff
      FillStripPart(1, 1, 1, 0, LED_LAST);

    }
    else if(containsGivenBits(msgMove.getMovementState().calibState, Claw_Calibration::CLAW_CALIB_DOWN_DONE))
    {
      //when we get here TOP is already done, and DOWN was just done
      CALIB_DONE:
      msgMove.finishCalibration(); //after this is sent UNO can move normaly (x and y direction enabled again)
      msgMove.sendMsg(COMMUNICATION_CALIBRATION);
      //now we should check if UNO got the message
      delayMicroseconds(CALIB_MSG_DELAY_MICROSECONDS);
      msgMove.readFromSlave();
      if (!containsGivenBits(msgMove.getMovementState().calibState, Claw_Calibration::CLAW_CALIB_DOWN_DONE | Claw_Calibration::CLAW_CALIB_TOP_DONE)) goto CALIB_DONE;
    }
  }
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
  msgMove.sendMsg(COMMUNICATION_MOVEMENT);
  msgMove.readFromSlave();  //maybe can be removed from here if it only provides Z direction data
  FillStripPart(1, 1, 1, 0, LED_LAST);
}