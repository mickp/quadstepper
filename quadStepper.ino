#include <TimerOne.h>

#define STEP_PIN 7
#define A_PIN 8
#define B_PIN 9
#define C_PIN 10
#define D_PIN 11
#define DIRECTION_PIN 12
#define PULSE_WIDTH_MICROSECONDS 50
#define MINIMUM_PERIOD_MICROSECONDS 2000
#define DISABLE LOW
#define ENABLE HIGH
#define FORWARD HIGH
#define BACKWARD LOW

long count_a = 1000;
long count_b = 0;
long count_c = 0;
long count_d = 0;

String inputString = "";
volatile boolean stringComplete = false;

volatile unsigned int ticks = 0;

void setup () {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(A_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  pinMode(C_PIN, OUTPUT);
  pinMode(D_PIN, OUTPUT);
  Timer1.initialize();
  Timer1.start();
  Timer1.attachInterrupt(timer_tick, MINIMUM_PERIOD_MICROSECONDS);
  Serial.begin(9600);
}

void timer_tick(void){
  ticks++;
}
  

void do_motor(int pin, long *count){
  if(*count > 0){
    // enable motor
    digitalWrite(pin, ENABLE);
    // +ve direction
    digitalWrite(DIRECTION_PIN, FORWARD);
    // send a step pulse
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(PULSE_WIDTH_MICROSECONDS);
    digitalWrite(STEP_PIN, LOW);
    // disable motor A
    digitalWrite(pin, DISABLE);
    // decrement the count
    --*count;
  } else if (*count < 0) {
    // enable motor
    digitalWrite(pin, ENABLE);
    // -ve direction
    digitalWrite(DIRECTION_PIN, BACKWARD);
    // send a step pulse
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(PULSE_WIDTH_MICROSECONDS);
    digitalWrite(STEP_PIN, LOW);
    // disable motor A
    digitalWrite(pin, DISABLE);
    // increment the count
    --*count;
  }
}
    
void loop() {
  noInterrupts();
  int start_ticks = ticks;
  interrupts();
  do_motor(A_PIN, &count_a);
  do_motor(B_PIN, &count_b);
  do_motor(C_PIN, &count_c);
  do_motor(D_PIN, &count_d);
  int current_ticks = start_ticks;
  while (current_ticks == start_ticks) {
    noInterrupts();
    current_ticks = ticks;
    interrupts();
  }
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    } 
  }
}    