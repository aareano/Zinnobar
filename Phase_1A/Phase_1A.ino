#include "TimerOne.h"

int bLedPin = 5;
int rLedPin = 33;
int gLedPin = 3;

int switch1Pin = 20;
int switch2Pin = 21;

int pot1Pin = 15;
int pot2Pin = 13;

int brightness = 255;

// machine state data
enum State { STATE_OFF, STATE_ON, STATE_RUN, STATE_SLEEP, STATE_DIAGNOSTIC };
State currState;

// run state data
enum RunState { RUN_FADE, RUN_BLINK1, RUN_BLINK2 };
RunState runState;
bool blueLedFast = false; // false => 500000, true => 100000.
bool interruptSet = false;

// sleep state data
enum SleepState { SLEEP_FADE, SLEEP_BLINK };
SleepState sleepState;
int sleepBlinkCnt = 0;

int diagnosticBlinkCnt = random(10);

void setup() {
  pinMode(bLedPin, OUTPUT);
  pinMode(rLedPin, OUTPUT);
  pinMode(gLedPin, OUTPUT);

  pinMode(switch1Pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(switch1Pin), switch1ISR, RISING);
  pinMode(switch2Pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(switch2Pin), switch2ISR, FALLING);

  currState = STATE_RUN;

  // run state data
  runState = RUN_FADE;

  // sleep state data
  sleepState = SLEEP_BLINK;

  Serial.begin(9600);
}

void loop() {
  
  switch(currState) {
    case STATE_OFF:
      state_off();
      break;
    case STATE_ON:
      state_on();
      break;
    case STATE_RUN:
      state_run();
      break;
    case STATE_SLEEP:
      state_sleep();
      break;
    case STATE_DIAGNOSTIC:
      state_diagnostic();
      break;
    default:
      Serial.print("Everything is broken");
  }
}

void state_off() {
  clearLeds();
  currState = STATE_ON;
}

void state_on() {
  blinkDigLed(gLedPin, 10);
}

void state_run() {
  // blink blue LED
  if (interruptSet == false) {
    Timer1.initialize(500000);   // variable not working
    Timer1.pwm(9, 512);
    Timer1.attachInterrupt(toggleBlue);
    interruptSet = true;
  }

  int rate = 1;
  
  switch (runState) {
    case RUN_FADE:
      if (fadeLed(gLedPin, 6) == 0) {
        runState = RUN_BLINK1;
      }
      break;
    case RUN_BLINK1:
      rate = analogRead(pot1Pin) > 512 ? 1 : 9;
      blinkAnalogLed(gLedPin, rate);
      runState = RUN_BLINK2;
      break;
    case RUN_BLINK2:
      rate = analogRead(pot1Pin) > 512 ? 1 : 9;
      blinkAnalogLed(gLedPin, rate);
      runState = RUN_FADE;
      break;
  }
}

void state_sleep() {
  switch (sleepState) {
    case SLEEP_BLINK:
      if (sleepBlinkCnt < 3) {
        sleepBlinkCnt++;
        blinkAnalogLed(bLedPin, 4);
      } else {
        sleepBlinkCnt = 0;
        sleepState = SLEEP_FADE;
      }
      break;
    case SLEEP_FADE:
      if (fadeLed(bLedPin, 1) == 0) {
        sleepState = SLEEP_BLINK;       // REPEAT sleep cycle
      }
      break;
  }
}

void state_diagnostic() {
  if (diagnosticBlinkCnt-- > 0) {
    blinkDigLed(rLedPin, 3);
  } else {
    changeState(STATE_SLEEP, currState);
  }
}


////////////////// HELPER FUNCTIONS /////////////////////


void changeState(State next, State prev) {
  if (prev == STATE_RUN) {
    runState = RUN_FADE;
    interruptSet = false;
    Timer1.detachInterrupt();
  } else if (prev == STATE_SLEEP) {
    sleepState = SLEEP_BLINK;
    sleepBlinkCnt = 0;
  } else if (prev == STATE_DIAGNOSTIC) {
    diagnosticBlinkCnt = random(10);
  }
  
  currState = next;
}

void switch1ISR() {
  Serial.print("switch1ISR\n");
  if (!blueLedFast) {
    Serial.print("changing blue blink rate\n");
    Timer1.detachInterrupt();
    blueLedFast = true;
    
    Timer1.initialize(100000);   // variable not working
    Timer1.pwm(9, 512);
    Timer1.attachInterrupt(toggleBlue);
  }
}

void switch2ISR() {
  if (blueLedFast) {
    digitalWrite(rLedPin, HIGH);
  }
}

void clearLeds() {
  digitalWrite(rLedPin, LOW);
  digitalWrite(gLedPin, LOW);
  analogWrite(bLedPin, 0);
}

void toggleBlue() {
  if (analogRead(bLedPin) > 512) {
    analogWrite(bLedPin, 0);
  } else {
    float level = analogRead(pot2Pin);
    Serial.println(level);
    analogWrite(bLedPin, level);
  }
}

// only takes digital pins
void blinkDigLed(int pin, int freq) {
  int ledDelay = 500 / freq;
  digitalWrite(pin, HIGH);
  delay(ledDelay);
  digitalWrite(pin, LOW);
  delay(ledDelay);
}

// only takes digital pins
void blinkAnalogLed(int pin, int freq) {
  int ledDelay = 500 / freq;
  analogWrite(pin, 255);
  delay(ledDelay);
  analogWrite(pin, 0);
  delay(ledDelay);
}

int fadeLed(int pin, int timeConst) {
  int fadeAmount = 255 / timeConst / 10;
  analogWrite(pin, brightness);
  
  brightness = brightness - fadeAmount;

  delay(100);

  if (brightness <= 0) {
    brightness = 255; // reset brightness
    return 0;
  } else {
    return 1;
  }
}






  

