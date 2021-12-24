// input pin to open valve
#define PWM_PIN_IN 3
// input pin for safety button
#define SAFETY_PIN_IN 2
// input valves pins
#define INLET_VALVE_PIN_1 8
#define INLET_VALVE_PIN_2 9

// highlight pins for highlight when controller is safety mode is turned on
#define HIGHLIGHT_PIN 5

#define LOW_LOW_BOUNDARY 900
#define LOW_HIGH_BOUNDARY 1100
#define HIGH_LOW_BOUNDARY 1900
#define HIGH_HIGH_BOUNDARY 2100

// duration im ms of open valves
#define OPEN_VALVE_DURATION 150

// timeout on enable safety if signal was lost
#define NO_SIGNAL_SAFETY_TIMEOUT 50000

volatile unsigned long ulstart_time = 0;
volatile boolean bnew_throtle_signal = false;
volatile int ithrottle_len = 0;

volatile unsigned long ulstart_time_2 = 0;
volatile boolean bnew_throtle_signal_2 = false;
volatile int ithrottle_len_2 = 0;
volatile long last_signal_time_2 = 0;

boolean bwas_low = false;
boolean is_safety_enabled = true;

long current_time = 0;


void setup() {
    digitalWrite(INLET_VALVE_PIN_1, LOW);
    digitalWrite(INLET_VALVE_PIN_2, LOW);
    digitalWrite(HIGHLIGHT_PIN, LOW);

    pinMode(INLET_VALVE_PIN_1, OUTPUT);
    pinMode(INLET_VALVE_PIN_2, OUTPUT);
    pinMode(HIGHLIGHT_PIN, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(PWM_PIN_IN), calc_pwm, CHANGE);
    attachInterrupt(digitalPinToInterrupt(SAFETY_PIN_IN), calc_pwm_safety, CHANGE);
}


void loop() {
    current_time = micros();
    if (bnew_throtle_signal) {
        if(in_boundary(ithrottle_len, LOW_LOW_BOUNDARY, LOW_HIGH_BOUNDARY)) {
            bwas_low = true;
        } else if (in_boundary(ithrottle_len, HIGH_LOW_BOUNDARY, HIGH_HIGH_BOUNDARY) && bwas_low && !is_safety_enabled) {
            bwas_low = false;
            fire(OPEN_VALVE_DURATION);
        }

        bnew_throtle_signal = false;
    }

    if(bnew_throtle_signal_2) {
      if(in_boundary(ithrottle_len_2, LOW_LOW_BOUNDARY, LOW_HIGH_BOUNDARY)) {
        is_safety_enabled = true;
        digitalWrite(HIGHLIGHT_PIN, LOW);
      } else {
        is_safety_enabled = false;
        digitalWrite(HIGHLIGHT_PIN, HIGH);
      }
      bnew_throtle_signal_2 = false;
    }

    if(current_time - last_signal_time_2 > NO_SIGNAL_SAFETY_TIMEOUT) {
      is_safety_enabled = true;
      digitalWrite(HIGHLIGHT_PIN, LOW);
    }
}


void calc_pwm() {
    if (digitalRead(PWM_PIN_IN) == HIGH) {
        ulstart_time = micros();
    } else {
        if (ulstart_time && (bnew_throtle_signal == false)) {
            ithrottle_len = (int)(micros() - ulstart_time);
            ulstart_time = 0;

            bnew_throtle_signal = true;
        }
    }
}


void calc_pwm_safety() {
    long current_micros = micros();

    if (digitalRead(SAFETY_PIN_IN) == HIGH) {
        ulstart_time_2 = current_micros;
    } else {
        if (ulstart_time_2 && (bnew_throtle_signal_2 == false)) {
            ithrottle_len_2 = (int)(current_micros - ulstart_time_2);
            ulstart_time_2 = 0;

            bnew_throtle_signal_2 = true;
            last_signal_time_2 = current_micros;
        }
    }
}


void fire(unsigned int duration) {
    digitalWrite(INLET_VALVE_PIN_1, HIGH);
    digitalWrite(INLET_VALVE_PIN_2, HIGH);

    delay(duration);

    digitalWrite(INLET_VALVE_PIN_1, LOW);
    digitalWrite(INLET_VALVE_PIN_2, LOW);
}


boolean in_boundary(int value, int low, int high) {
    return value > low && value < high;
}
