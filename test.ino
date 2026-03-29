#include <Servo.h>

const int shock_pin = 2;
const int servo_pin = 4;
const int hw_505_pin = A0;

Servo lock_servo;

void setup() {
  Serial.begin(9600);

  pinMode(shock_pin, INPUT);

  lock_servo.attach(servo_pin);
  lock_servo.write(0);   // keep locked position

  Serial.println("=== VaultBox tilt calibration ===");
  Serial.println("Leave the box still and observe A0 values.");
  Serial.println("Then tilt/move it and observe how values change.");
  Serial.println("Format: TILT_RAW:<value>,SHOCK:<value>");
}

void loop() {
  int tilt_raw = analogRead(hw_505_pin);
  int shock_state = digitalRead(shock_pin);

  Serial.print("TILT_RAW:");
  Serial.print(tilt_raw);
  Serial.print(",SHOCK:");
  Serial.println(shock_state);

  delay(200);
}