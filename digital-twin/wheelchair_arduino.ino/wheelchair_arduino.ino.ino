const int joyXPin = A1;
const int joyYPin = A0;
const int speedPinL = 9;
const int dirPinL = 7;
const int speedPinR = 10;
const int dirPinR = 8;

void setup() {
  pinMode(speedPinL, OUTPUT);
  pinMode(dirPinL, OUTPUT);
  pinMode(speedPinR, OUTPUT);
  pinMode(dirPinR, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  int joyX = analogRead(joyXPin);
  int joyY = analogRead(joyYPin);

  // Deadzone
  int mapY = map(joyY, 0, 1023, -255, 255);
  int mapX = map(joyX, 0, 1023, -255, 255);
  if (abs(mapY) < 20) mapY = 0;
  if (abs(mapX) < 20) mapX = 0;

  // Differential drive mixing
  int leftSpeed  = constrain(mapY + mapX, -255, 255);
  int rightSpeed = constrain(mapY - mapX, -255, 255);

  // Left motor
  digitalWrite(dirPinL, leftSpeed >= 0 ? LOW : HIGH);
  analogWrite(speedPinL, abs(leftSpeed));

  // Right motor
  digitalWrite(dirPinR, rightSpeed >= 0 ? LOW : HIGH);
  analogWrite(speedPinR, abs(rightSpeed));

  // Send to serial for digital twin
  Serial.print(joyX);
  Serial.print(",");
  Serial.println(joyY);

  delay(50);
}