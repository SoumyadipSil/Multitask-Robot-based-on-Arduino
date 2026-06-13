#include <SoftwareSerial.h>
#include <AFMotor.h>

SoftwareSerial BT(9, 10); // RX, TX

AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

char c;

void setup() {
  BT.begin(9600);

  motor1.setSpeed(200);
  motor2.setSpeed(200);
  motor3.setSpeed(200);
  motor4.setSpeed(200);

  Stop();
}

void loop() {
  if (BT.available()) {
    c = BT.read();

    // FORWARD
    if (c == 'U') {
      motor1.run(FORWARD);
      motor2.run(FORWARD);
      motor3.run(FORWARD);
      motor4.run(FORWARD);
    }

    // BACKWARD
    else if (c == 'D') {
      motor1.run(BACKWARD);
      motor2.run(BACKWARD);
      motor3.run(BACKWARD);
      motor4.run(BACKWARD);
    }

    // LEFT
    else if (c == 'L') {
      motor1.run(BACKWARD);
      motor2.run(BACKWARD);
      motor3.run(FORWARD);
      motor4.run(FORWARD);
    }

    // RIGHT
    else if (c == 'R') {
      motor1.run(FORWARD);
      motor2.run(FORWARD);
      motor3.run(BACKWARD);
      motor4.run(BACKWARD);
    }

    // STOP
    else if (c == 'S') {
      Stop();
    }
  }
}

void Stop() {
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
}