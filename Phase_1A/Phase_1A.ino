int yLedPin = 2;
int rLedPin = 33;
int gLedPin = 3;

int switch1Pin = 20;
int switch2Pin = 21;

int pot1Pin = 15;
int pot2Pin = 13;

int loops = 0;
int brightness = 255;

enum State { STATE_OFF, STATE_ON, STATE_RUN, STATE_SLEEP, STATE_DIAGNOSTIC };
enum RunState { RUN_FADE, RUN_BLINK1, RUN_BLINK2 };
enum SleepState { SLEEP_FADE, SLEEP_BLINK };

State currState;
RunState runState;

SleepState sleepState;
int sleepBlinkCnt = 0;

int diagnosticBlinkCnt = random(10);

void setup() {
  pinMode(yLedPin, OUTPUT);
  pinMode(rLedPin, OUTPUT);
  pinMode(gLedPin, OUTPUT);

  pinMode(switch1Pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(switch1Pin), switch1ISR, RISING);
  pinMode(switch2Pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(switch2Pin), switch2ISR, FALLING);

  currState = STATE_DIAGNOSTIC;
  runState = RUN_FADE;
  
  sleepState = SLEEP_BLINK;

  Serial.begin(9600);
}

void loop() {
  
  switch(currState) {
    case STATE_OFF:
      clearLeds();
      currState = STATE_ON;
      break;
      
    case STATE_ON:
      blinkDigLed(gLedPin, 10);
      break;
      
    case STATE_RUN:
      
      switch (runState) {
        case RUN_FADE:
          if (fadeLed(gLedPin, 6) == 0) {
            runState = RUN_BLINK1;
          }
          break;
        case RUN_BLINK1:
          blinkAnalogLed(gLedPin, 1);
          runState = RUN_BLINK2;
          break;
        case RUN_BLINK2:
          blinkAnalogLed(gLedPin, 1);
          runState = RUN_FADE;
          break;
      }
      break;
      
    case STATE_SLEEP:
      
      switch (sleepState) {
        case SLEEP_BLINK:
          if (sleepBlinkCnt < 3) {
            sleepBlinkCnt++;
            blinkAnalogLed(yLedPin, 4);
          } else {
            sleepBlinkCnt = 0;
          sleepState = SLEEP_FADE;
          }
          break;
        case SLEEP_FADE:
          if (fadeLed(yLedPin, 1) == 0) {
            sleepState = SLEEP_BLINK;       // REPEAT sleep cycle
          }
          break;
      }
      break;      

    case STATE_DIAGNOSTIC:
      if (diagnosticBlinkCnt-- > 0) {
        blinkDigLed(rLedPin, 3);
      } else {
        changeState(STATE_SLEEP, currState);
      }
      break;
    default:
      Serial.print("Everything is broken");
  }
}

void changeState(State next, State prev) {
  if (prev == STATE_RUN) {
    runState = RUN_FADE;
  }
  if (prev == STATE_SLEEP) {
    sleepState = SLEEP_BLINK;
    sleepBlinkCnt = 0;
  }
  if (prev == STATE_DIAGNOSTIC) {
    diagnosticBlinkCnt = random(10);
  }

  currState = next;
}

void switch1ISR() {
  
}

void switch2ISR() {
  
}

void clearLeds() {
  digitalWrite(rLedPin, LOW);
  digitalWrite(gLedPin, LOW);
  analogWrite(yLedPin, 0);
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






  

