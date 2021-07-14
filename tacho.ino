#include <SwitecX25.h>

// standard X25.168 range 315 degrees at 1/3 degree steps
#define STEPS (315*3)
// For motors connected đto digital pins 4,5,6,7
SwitecX25 motor1(STEPS,4,5,6,7);


volatile unsigned long pulseCount = 0, period, lastMicros = 0, currentMicros;

#define TACHO_INPUT_PIN 2

void IRQcounter() {
  bool in = digitalRead(TACHO_INPUT_PIN);
  /* we can get false triggers on falling flank, detect this and reject */
  if (in == LOW) {
      return;
  }
  currentMicros = micros();
  if (currentMicros - lastMicros < 1000) return; /* reject to short pulses */
  period = currentMicros - lastMicros;
  lastMicros = currentMicros;
  pulseCount++;
}

void setup() {
    Serial.begin(9600);

    pinMode(TACHO_INPUT_PIN, INPUT);
    
    attachInterrupt(digitalPinToInterrupt(TACHO_INPUT_PIN), IRQcounter, RISING);

    tacho_init();
    tacho(0);
}

void loop() {
    unsigned long t, startTime = 0, lastPulseCount;
    int rpm;

    startTime = millis();
    
    Serial.print("Starting.");

    while (1) {
        t = millis();
        /* In case we have no pulses within a sec, we assume rpm = 0 */
        if(lastPulseCount != pulseCount) {
            lastPulseCount = pulseCount;
            startTime = t;
        }
        if (t - startTime > 1000) {
            rpm = 0;
        } else {
            rpm = (60*1000000)/(period*4);
        }
        tacho(rpm);

	if (t % 2500 == 0) {
	    Serial.print("  period: ");
	    Serial.print(period);
	    Serial.print("  rpm: ");
	    Serial.println(rpm);
	    Serial.flush();
	}
    }
}

#define ZEROPOS 44
#define MAXPOS 795
#define MAXRPM 7000

void tacho_init() {
    motor1.zero();
    motor1.setPosition(ZEROPOS);
}

void tacho(int rpm)
{
  static int nextPos = 0;
  static int currentPos = 0;

  nextPos = map(rpm, 0, MAXRPM, ZEROPOS, MAXPOS);

  /* Avoid small movements to rejct noise */
  if( abs(nextPos - currentPos) > 3) {
      currentPos = nextPos;
      motor1.setPosition(nextPos);
      motor1.updateBlocking();
  }
}

