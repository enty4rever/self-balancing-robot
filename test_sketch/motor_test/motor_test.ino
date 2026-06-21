#include <Arduino.h>

#define STBY 16

//Motor A
#define PWM_A 25
#define IN1_A 26
#define IN2_A 27

//Motor B
#define PWM_B 33
#define IN1_B 14
#define IN2_B 13

void setup() {
  Serial.begin(115200);
  pinMode(STBY, OUTPUT);
  pinMode(PWM_A, OUTPUT);
  pinMode(IN1_A, OUTPUT);
  pinMode(IN2_A, OUTPUT);
  pinMode(PWM_B, OUTPUT);
  pinMode(IN1_B, OUTPUT);
  pinMode(IN2_B, OUTPUT);

  digitalWrite(STBY, HIGH); 
}

void driveMotor(int in1, int in2, int pwm, int speed) {
    if (speed == 0) {
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        analogWrite(pwm, 0);
    } 
    else {
        if (speed > 0) {
            digitalWrite(in1, HIGH);
            digitalWrite(in2, LOW);
        } else {
            digitalWrite(in1, LOW);
            digitalWrite(in2, HIGH);
        }
        analogWrite(pwm, abs(speed));
    }
}

void driveBoth(int speed) {
    driveMotor(IN1_A, IN2_A, PWM_A, speed);
    driveMotor(IN1_B, IN2_B, PWM_B, speed);
}

void loop(){
    //Test smooth back and forth run
    Serial.println("Smooth Back and Forth Run test\n");
    delay(1000);

    Serial.println("Forward slow");
    driveBoth(150);
    delay(2000);

    Serial.println("Forward fast");
    driveBoth(255);
    delay(2000);

    Serial.println("Forward slow");
    driveBoth(150);
    delay(2000);

    Serial.println("Stop");
    driveBoth(0);
    delay(1000);

    Serial.println("Backward slow");
    driveBoth(-150);
    delay(2000);

    Serial.println("Backward fast");
    driveBoth(-255);
    delay(2000);

    Serial.println("Backward slow");
    driveBoth(-150);
    delay(2000);

    Serial.println("Stop");
    driveBoth(0);
    Serial.println("Smooth Back and Forth Run test complete\n");
    delay(5000);
    
    //Test direct reversal from full forward to full backward
    Serial.println("Direct Reversal test\n");
    delay(1000);

    Serial.println("Forward fast");
    driveBoth(255);
    delay(2000);

    Serial.println("Backward fast");
    driveBoth(-255);
    delay(2000);

    Serial.println("Hard Stop");
    driveBoth(0);
    Serial.println("Direct Reversal test complete\n");
    delay(5000);
}