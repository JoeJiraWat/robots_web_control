#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* ssid = "RobotControl";
const char* password = "robot1234";

// Motor Left
const int in1 = 14; 
const int in2 = 13;
const int pwm1 = 12;

// Motor Right
const int in3 = 27;
const int in4 = 33;
const int pwm2 = 32;

// Servo pins
const int servo1Pin = 26;
const int servo2Pin = 23;

Servo servo1;
Servo servo2;

int currentServo1Pos = 90;
int currentServo2Pos = 90;


void smoothServoWrite(Servo &servo, int &currentPos, int targetPos, int delayTime) {
  if (targetPos > currentPos) {
    for (int pos = currentPos; pos <= targetPos; pos += 1) {
      servo.write(pos);
      delay(delayTime);
    }
  } else {
    for (int pos = currentPos; pos >= targetPos; pos -= 1) {
      servo.write(pos);
      delay(delayTime);
    }
  }
  currentPos = targetPos;
}

WebServer server(80);

void setup() {
  Serial.begin(115200);

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(pwm1, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(pwm2, OUTPUT);

  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);
  
  servo1.write(currentServo1Pos);
  servo2.write(currentServo2Pos);


  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // Servo 1
  server.on("/servo1", []() {
    if (server.hasArg("pos")) {
      int targetPos = server.arg("pos").toInt();
      if (server.hasArg("smooth")) {
        smoothServoWrite(servo1, currentServo1Pos, targetPos, 15);
      } else {
        servo1.write(targetPos);
        currentServo1Pos = targetPos;
      }
      server.send(200, "text/plain", "OK");
    } else server.send(400, "text/plain", "Missing pos");
  });

  // Servo 2
  server.on("/servo2", []() {
    if (server.hasArg("pos")) {
      int targetPos = server.arg("pos").toInt();
      if (server.hasArg("smooth")) {
        smoothServoWrite(servo2, currentServo2Pos, targetPos, 15);
      } else {
        servo2.write(targetPos);
        currentServo2Pos = targetPos;
      }
      server.send(200, "text/plain", "OK");
    } else server.send(400, "text/plain", "Missing pos");
  });

  // Motor control (แก้ให้ตรงทิศทาง)
  server.on("/forward", []() {
    moveForward();
    server.send(200, "text/plain", "OK");
  });

  server.on("/backward", []() {
    moveBackward();
    server.send(200, "text/plain", "OK");
  });

  server.on("/left", []() {    // ← แก้ให้เลี้ยวซ้ายจริง
    turnLeft();
    server.send(200, "text/plain", "OK");
  });

  server.on("/right", []() {   // ← แก้ให้เลี้ยวขวาจริง
    turnRight();
    server.send(200, "text/plain", "OK");
  });

  server.on("/stop", []() {
    stopMotors();
    server.send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
  server.handleClient();
}

void moveForward() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(pwm1, 255);
  analogWrite(pwm2, 255);
}

void moveBackward() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(pwm1, 255);
  analogWrite(pwm2, 255);
}

void turnLeft() {  
  // ล้อซ้ายถอยหลัง, ล้อขวาเดินหน้า → เลี้ยวซ้าย
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(pwm1, 255);
  analogWrite(pwm2, 255);
}

void turnRight() { 
  // ล้อซ้ายเดินหน้า, ล้อขวาถอยหลัง → เลี้ยวขวา
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(pwm1, 255);
  analogWrite(pwm2, 255);
}

void stopMotors() {
  digitalWrite(in1, LOW)
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  analogWrite(pwm1, 0);
  analogWrite(pwm2, 0);
}
