#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 RTC;

// Define the Pins we are using.
#define PIN_BUTTON_HOUR 10
#define PIN_BUTTON_MIN 9
#define PIN_METER_HOUR 6
#define PIN_METER_MIN 5
#define PIN_METER_SEC 3

// Define the minimum and maximum PWM values for all the meters.
// These values can be used to calibrate the meters.
#define MINIMUM_PWM_VALUE_HOUR 0
#define MAXIMUM_PWM_VALUE_HOUR 220
#define MINIMUM_PWM_VALUE_MIN 0
#define MAXIMUM_PWM_VALUE_MIN 250
#define MINIMUM_PWM_VALUE_SEC 0
#define MAXIMUM_PWM_VALUE_SEC 225

class NeedleManager
{
  public:
  //Constructor
  NeedleManager(int timeBase, int pin)
  : _needlePin(pin),
    _timeBase(timeBase),
    _needleSpeed(100), // units per second
    _currentPosition(0)
  { }

  void updatePosition(int target)
  {
    Serial.print(_targetPosition);
    Serial.print(" : ");
    Serial.println(target);
    _targetPosition = target; 
    int moveDirection = _targetPosition - _currentPosition;
    if (moveDirection > 0)
    {
      _currentPosition += (float)_needleSpeed * (float)_timeBase / 1000.;
      if (_currentPosition > _targetPosition)
        _currentPosition = _targetPosition;
    }
    else if (moveDirection < 0)
    {
      _currentPosition -= (float)_needleSpeed * (float)_timeBase / 1000.;
      if (_currentPosition < _targetPosition)
        _currentPosition = _targetPosition;
    }
    analogWrite(_needlePin,_currentPosition);
  }
  
  private:
  int _needleSpeed;
  int _targetPosition;
  int _currentPosition;
  int _timeBase;
  int _needlePin;
};

int timeBase = 100;
NeedleManager hourNeedle = NeedleManager(timeBase,PIN_METER_HOUR);
NeedleManager minuteNeedle = NeedleManager(timeBase,PIN_METER_MIN);
NeedleManager secondNeedle = NeedleManager(timeBase,PIN_METER_SEC);

void animateMeter(uint8_t pin, uint8_t min, uint8_t max) {
  uint8_t v;
  for (v = min; v <= max; v++) {
    analogWrite(pin, v);
    delay(20);
  }
  analogWrite(pin, min);
  delay(1000);
}

void setup () {
  Serial.begin(9600);
  Wire.begin();
  RTC.begin();
  // Check to see if the RTC is keeping time.  If it is, load the time from your computer.
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // This will reflect the time that your sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  pinMode(PIN_METER_HOUR, OUTPUT);
  pinMode(PIN_METER_MIN, OUTPUT);
  pinMode(PIN_METER_SEC, OUTPUT);
  pinMode(PIN_BUTTON_MIN, INPUT);
  pinMode(PIN_BUTTON_HOUR, INPUT);
  
}

void loop () {
      DateTime now = RTC.now();
      int8_t h = (now.hour() == 0 || now.hour() == 12) ? 12 : now.hour() % 12;
      hourNeedle.updatePosition(map(h, 1, 12, MINIMUM_PWM_VALUE_HOUR, MAXIMUM_PWM_VALUE_HOUR));
      minuteNeedle.updatePosition(map(now.minute(), 0, 60, MINIMUM_PWM_VALUE_MIN, MAXIMUM_PWM_VALUE_MIN));
      secondNeedle.updatePosition(map(now.second(), 0, 60, MINIMUM_PWM_VALUE_SEC, MAXIMUM_PWM_VALUE_SEC));
      delay(timeBase);

      while (digitalRead(PIN_BUTTON_HOUR)) {
        DateTime now = RTC.now();
        RTC.adjust(DateTime(now.year(), now.month(), now.day(), (now.hour() + 1) % 24, now.minute(), 0));  
        int8_t h = (now.hour() == 0 || now.hour() == 12) ? 12 : now.hour() % 12;    
        hourNeedle.updatePosition(map(h, 1, 12, MINIMUM_PWM_VALUE_HOUR, MAXIMUM_PWM_VALUE_HOUR));
        delay(500);
     }
     while (digitalRead(PIN_BUTTON_MIN)) {
        DateTime now = RTC.now();
        RTC.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), (now.minute() + 1) % 60, 0));      
        minuteNeedle.updatePosition(map(now.minute(), 1, 12, MINIMUM_PWM_VALUE_MIN, MAXIMUM_PWM_VALUE_MIN));
        delay(500);
     }
}
