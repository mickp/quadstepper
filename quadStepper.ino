#include <TimerOne.h>
/* 
  quadStepper: drives four motors using the SparkFun Quadstepper board bus.
  
  Copyright (C) January 2015, Mick Phillips.  

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
*/

/* 
  === Overview ===
    Each motor is associated with a counter variable. On each iteration
    of the main loop, if a motor's counter is non-zero, a step pulse
    is sent and the count inc(dec)remented to bring ot closer to zero.
    The sign of the counter determines the motor direction.
    A delay is included to keep the frequency of pulses for each motor
    below 1 / MINIMUM_PERIOD_MICROSECONDS .
    The main loop also inspects and acts on a command string received
    over a serial link.  
      
  === Commands ===
    char(id)long(steps) - e.g. 'A-10', 'B22'
      Add 'steps' to count for motor 'id'.
        
    ?
      Reports the number of steps remaining for each motor.
*/
 
  

// step pulse pin
#define STEP_PIN 7
// motor direction pin
#define DIRECTION_PIN 12
// motor enable pins
#define A_PIN 8
#define B_PIN 9
#define C_PIN 10
#define D_PIN 11
// motor step pulse width
#define PULSE_WIDTH_MICROSECONDS 50
// 1 / maximum update frequency
#define MINIMUM_PERIOD_MICROSECONDS 2000
// logic states to enable/disable motors
#define DISABLE LOW
#define ENABLE HIGH
// logic states to motor direction
#define FORWARD HIGH
#define BACKWARD LOW

// remaining steps for each motor
long count_a = 0;
long count_b = 0;
long count_c = 0;
long count_d = 0;

// command input buffer
String inputString = "";
volatile boolean stringComplete = false;

// tick count
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
    ++*count;
  }
}
    
// ticks at last loop exit
unsigned int last_ticks = 0;

void loop() {
  do_motor(A_PIN, &count_a);
  do_motor(B_PIN, &count_b);
  do_motor(C_PIN, &count_c);
  do_motor(D_PIN, &count_d);
  
  // do command processing here
  if (stringComplete) {
    // parse the string
    int motorChar = inputString.charAt(0);
    long motorSteps = inputString.substring(1).toInt();
    
    // update specified motor count or report all counts
    switch (motorChar) {
    case 'a':
    case 'A':
      count_a += motorSteps;
      break;
    case 'b':
    case 'B':
      count_b += motorSteps;
      break;
    case 'c':
    case 'C':
      count_c += motorSteps;
      break;
    case 'd':
    case 'D':
      count_d += motorSteps;
      break;
    case '?':
      // report 
      String output = "";
      output += "A " + String(count_a) + "\n";
      output += "B " + String(count_b) + "\n";
      output += "C " + String(count_c) + "\n";
      output += "D " + String(count_d) + "\n";    
      Serial.print(output);
    }

    inputString= "";
    stringComplete = false;
  }
    
    
  // delay until at least one tick from last loop completion
  int current_ticks = last_ticks;
  while (current_ticks == last_ticks) {
    // need to temporarily disable interrupts to read a variable
    // written to by an interrupt function
    noInterrupts();
    current_ticks = ticks;
    interrupts();
  }
  last_ticks = current_ticks;
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
