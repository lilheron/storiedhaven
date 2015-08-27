/*** working prototype of sensor reading with "motor" output (LED brightness
as a function of proximity to sensor)

TBD:
~smoothed state of each proximity sensor with slowly decaying memory
~function to convert d(proximity) into appropriate motor behavior
~sleep mode: when d(proximity) is near zero over time, motors should burst at periodic intervals
  with fibonacci seq (or similar) durations between bursts, up to some point at which the motors sleep
  until next perturbation, ie when d(proximity) surpasses some threshold
*/

#include "hcrs04.h"
#include <Arduino.h>

int trig[] = {3, 5, 7};
int echo[] = {4, 6, 8};
int motor[] = {10, 11, 12}; //make sure these pins have the ~ symbol for PWM

float proximity[3]; //current sensed distance
float shake_on[3]; //next time to turn motor on
float shake_off[3]; //next time to turn motor off
float dp[3][100]; //array of change in sensor proximity (3) over time (100 cycles of this code)

//fibonacci parameters
int fib_steps = 10; //
int fib_0 = 1;
int fib_1 = 1;

int motor_pulse = 100; //microsecond period of motor on

//sensor modules
hcrs04 sensors[3]; 

/*** asleep state
[0] asleep: after expiration of fib seq goes to sleep until next perturb
[1] awake: awaiting next step in fib, sensor perturbation, or pulse trough to pulse motor
[2] on: motor is currently on

*/

int state[] = {0, 0, 0};
int asleep = 0;
int awake = 1;
int on = 2;

void setup() {
  int i;
  
  //initialize each sensor and motor
  for (i = 0; i < 3; i = i + 1) {
    sensors[i] = hcrs04(trig[i], echo[i]);
    sensors[i].begin();
    pinMode(motor[i], OUTPUT); 
  }

//  Serial.begin(115200);

}

void loop() {
  int i;

  //check the state of each sensor and update motor accordingly
  for (i = 0; i < 3; i = i + 1) {
    
    proximity[i] = sensors[i].read();
    
//    //I used this to control LED brightness via PWM, probably needs work to be more appropriate for motor
//    if (distance[i] < 50) {
//      analogWrite(motor[i], 255-int(distance[i])*5);
//    }
//    else
//    {
//      analogWrite(motor[i],0);
//    }
    
    switch (state[i]) {

      //asleep
      case 0: 
        //check if a perturbation has occurred 
        if (poked(i)) {
          state[i] = awake;
          shake_on[i] = micros();
        }
      
      //awake
      case 1:
        
        if (shake_on[i] > micros()) {
          motor_on(i);
          shake_off[i] = micros() + motor_pulse;  
        }
        else if (shake_off[i] > micros()){
          motor_off(i);
        }
        else {
          state[i] = asleep;  
        }
        
    }
  }
  
//  Serial.print(String(distance[0]) +"; " + String(distance[1]));
//  Serial.println(" cm");
  
  //some minimum buffering, may not be necessary when not doing serial monitor
  delay(10);  

}

bool poked(int i) {
  return (random(10000)==1);
}

void motor_on(int i) {
  digitalWrite(motor[i], HIGH);
}

void motor_off(int i) {
  digitalWrite(motor[i], LOW);
}
