#include <Servo.h>
#include <Wire.h>
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

Servo radarServo;

const int trigPin = 9;
const int echoPin = 10;
const int servoPin = 3;

int radarAngle = 0;
int radarDistance = 0;

long getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0)
    return 400;

  return duration * 0.034 / 2;
}

void drawRadar() {
  u8g2.clearBuffer();

  // Radar circles
  u8g2.drawCircle(64, 63, 20);
  u8g2.drawCircle(64, 63, 35);
  u8g2.drawCircle(64, 63, 50);

  float rad = radarAngle * PI / 180.0;

  int x = 64 + 50 * cos(rad);
  int y = 63 - 50 * sin(rad);

  // Sweep line
  u8g2.drawLine(64, 63, x, y);

  // Object marker
  if (radarDistance < 100) {
    int objR = map(radarDistance, 0, 100, 0, 50);

    int ox = 64 + objR * cos(rad);
    int oy = 63 - objR * sin(rad);

    u8g2.drawDisc(ox, oy, 2);
  }

  u8g2.setFont(u8g2_font_6x12_tf);

  u8g2.setCursor(0, 10);
  u8g2.print("Angle:");
  u8g2.print(radarAngle);

  u8g2.setCursor(0, 24);
  u8g2.print("Dist:");
  u8g2.print(radarDistance);
  u8g2.print("cm");

  u8g2.sendBuffer();
}

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  radarServo.attach(servoPin);

  u8g2.begin();
}

void loop() {
  for (int angle = 0; angle <= 180; angle += 2) {
    radarServo.write(angle);
    delay(20);
    radarDistance = getDistance();
    radarAngle = angle;
    drawRadar();
  }

  for (int angle = 180; angle >= 0; angle -= 2) {
    radarServo.write(angle);
    delay(40);
    radarDistance = getDistance();
    radarAngle = angle;
    drawRadar();
  }
}