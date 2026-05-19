/*
 * Smart Assistive Wheelchair — Main Firmware
 * Author: Salimat Oluwatobi Akinwande
 * Institution: RAIN RDA Cohort 18, Ibadan
 *
 * Controls two BLDC hub motors via ZS-X11H V2 drivers
 * using differential drive kinematics from a 2-axis joystick.
 * MPU6050 monitors tilt — if angle > 45°, motors cut and alarm triggers.
 */

#include <Wire.h>
#include <MPU6050.h>

// ── Joystick pins ──────────────────────────────────────────
const int joyXPin = A1;
const int joyYPin = A0;

// ── Left motor (ZS-X11H #1) ────────────────────────────────
const int speedPinL = 9;
const int dirPinL   = 7;

// ── Right motor (ZS-X11H #2) ───────────────────────────────
const int speedPinR = 10;
const int dirPinR   = 8;

// ── Safety ─────────────────────────────────────────────────
const int buzzerPin = 12;
const int ledPin    = 13;
const int TILT_THRESHOLD = 45;

MPU6050 mpu;

void setup() {
  pinMode(speedPinL, OUTPUT);
  pinMode(dirPinL,   OUTPUT);
  pinMode(speedPinR, OUTPUT);
  pinMode(dirPinR,   OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin,    OUTPUT);

  Serial.begin(115200);
  Wire.begin();
  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 not found — safety monitoring disabled");
  }
}

void loop() {
  // ── Read joystick ────────────────────────────────────────
  int joyX = analogRead(joyXPin);
  int joyY = analogRead(joyYPin);

  // ── Map to -255 → +255 ──────────────────────────────────
  int mapY = map(joyY, 0, 1023, -255, 255);
  int mapX = map(joyX, 0, 1023, -255, 255);

  // ── Deadzone ─────────────────────────────────────────────
  if (abs(mapY) < 20) mapY = 0;
  if (abs(mapX) < 20) mapX = 0;

  // ── MPU6050 safety check ─────────────────────────────────
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  float pitch = atan2(ay, az) * 180.0 / PI;
  float roll  = atan2(ax, az) * 180.0 / PI;

  if (abs(pitch) > TILT_THRESHOLD || abs(roll) > TILT_THRESHOLD) {
    // EMERGENCY CUTOFF
    analogWrite(speedPinL, 0);
    analogWrite(speedPinR, 0);
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(ledPin, HIGH);
    Serial.println("ALERT: Tilt detected — motors stopped");
    delay(100);
    return;
  } else {
    digitalWrite(buzzerPin, LOW);
    digitalWrite(ledPin, LOW);
  }

  // ── Differential drive mixing ────────────────────────────
  int leftSpeed  = constrain(mapY + mapX, -255, 255);
  int rightSpeed = constrain(mapY - mapX, -255, 255);

  // ── Drive left motor ─────────────────────────────────────
  digitalWrite(dirPinL, leftSpeed >= 0 ? LOW : HIGH);
  analogWrite(speedPinL, abs(leftSpeed));

  // ── Drive right motor ────────────────────────────────────
  digitalWrite(dirPinR, rightSpeed >= 0 ? LOW : HIGH);
  analogWrite(speedPinR, abs(rightSpeed));

  // ── Serial output for Digital Twin ───────────────────────
  Serial.print(joyX);
  Serial.print(",");
  Serial.println(joyY);

  delay(10);
}
