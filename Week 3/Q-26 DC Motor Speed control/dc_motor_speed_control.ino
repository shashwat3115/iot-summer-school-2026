/*
 * Project: DC Motor Control with L298N Driver
 * Question: Q26
 * Author: [Shashwat Singh]
 */

#include <Arduino.h>

// L298N Pin Configuration
const int IN1_PIN = 2;      // Direction pin 1
const int IN2_PIN = 3;      // Direction pin 2
const int ENA_PIN = 9;      // PWM speed control (pins 3,5,6,9,10,11 on UNO)

// Button pins
const int DIRECTION_BUTTON = 4;  // Toggle forward/reverse
const int POWER_BUTTON = 5;      // Toggle motor on/off

// Speed control
const int SPEED_POT = A0;  // Potentiometer for speed (0-1023 → 0-255)

// State variables
bool motor_running = false;
bool forward_direction = true;  // true=forward, false=reverse
int current_speed = 0;

// Button debounce
unsigned long last_dir_btn_time = 0;
unsigned long last_pwr_btn_time = 0;
const unsigned long DEBOUNCE_TIME = 50;

void setup() {
  // Initialize motor control pins
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(ENA_PIN, OUTPUT);
  
  // Initialize button pins
  pinMode(DIRECTION_BUTTON, INPUT);
  pinMode(POWER_BUTTON, INPUT);
  
  // Initialize Serial
  Serial.begin(9600);
  Serial.println("===== DC Motor Control (L298N) Started =====");
  Serial.println("Potentiometer (A0): Speed 0-100%");
  Serial.println("Button 1 (pin 4): Forward/Reverse");
  Serial.println("Button 2 (pin 5): Motor ON/OFF");
  Serial.println("============================================");
  
  // Set initial motor state (stopped)
  motor_running = false;
  forward_direction = true;
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  analogWrite(ENA_PIN, 0);
}

void loop() {
  unsigned long current_time = millis();
  
  // === READ POTENTIOMETER FOR SPEED ===
  int pot_reading = analogRead(SPEED_POT);
  int speed_pwm = map(pot_reading, 0, 1023, 0, 255);
  int speed_percent = map(speed_pwm, 0, 255, 0, 100);
  
  // === READ DIRECTION BUTTON (Debounced) ===
  if (digitalRead(DIRECTION_BUTTON) == HIGH && (current_time - last_dir_btn_time) > DEBOUNCE_TIME) {
    last_dir_btn_time = current_time;
    forward_direction = !forward_direction;
    
    Serial.print("[DIRECTION] Changed to: ");
    Serial.println(forward_direction ? "FORWARD" : "REVERSE");
  }
  
  // === READ POWER BUTTON (Debounced) ===
  if (digitalRead(POWER_BUTTON) == HIGH && (current_time - last_pwr_btn_time) > DEBOUNCE_TIME) {
    last_pwr_btn_time = current_time;
    motor_running = !motor_running;
    
    Serial.print("[POWER] Motor: ");
    Serial.println(motor_running ? "STARTED" : "STOPPED");
  }
  
  // === CONTROL MOTOR ===
  if (motor_running) {
    // Apply speed via PWM
    analogWrite(ENA_PIN, speed_pwm);
    
    // Set direction via IN1 and IN2
    if (forward_direction) {
      digitalWrite(IN1_PIN, HIGH);
      digitalWrite(IN2_PIN, LOW);
    } else {
      digitalWrite(IN1_PIN, LOW);
      digitalWrite(IN2_PIN, HIGH);
    }
  } else {
    // Motor stopped
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, LOW);
    analogWrite(ENA_PIN, 0);
  }
  
  // === PERIODIC STATUS OUTPUT ===
  static unsigned long last_status_time = 0;
  if ((current_time - last_status_time) >= 500) {
    last_status_time = current_time;
    
    Serial.print("[STATUS] Direction: ");
    Serial.print(forward_direction ? "FORWARD" : "REVERSE");
    Serial.print(" | Speed: ");
    Serial.print(speed_percent);
    Serial.print("% | State: ");
    Serial.print(motor_running ? "RUNNING" : "STOPPED");
    Serial.print(" | PWM: ");
    Serial.println(speed_pwm);
  }
}
