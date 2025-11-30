#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// Replace with your desired AP credentials
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

WebServer server(80);

void setup() {
  Serial.begin(115200);

  // Set motor pins as outputs
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(pwm1, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(pwm2, OUTPUT);

  // Attach servo pins
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);

  // Start ESP32 as a Wi-Fi access point
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);


  // --- Web Server Endpoints ---

  // Servo 1
  server.on("/servo1", []() {
    if (server.hasArg("pos")) {
      String posStr = server.arg("pos");
      int pos = posStr.toInt();
      servo1.write(pos);
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Missing 'pos' parameter");
    }
  });

  // Servo 2
  server.on("/servo2", []() {
    if (server.hasArg("pos")) {
      String posStr = server.arg("pos");
      int pos = posStr.toInt();
      servo2.write(pos);
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Missing 'pos' parameter");
    }
  });

  // Motor control
  server.on("/forward", []() {
    Serial.println("forward");
    moveForward();
    server.send(200, "text/plain", "OK");
  });

  server.on("/backward", []() {
    Serial.println("backward");
    moveBackward();
    server.send(200, "text/plain", "OK");
  });

  server.on("/left", []() {
    Serial.println("left");
    turnLeft();
    server.send(200, "text/plain", "OK");
  });

  server.on("/right", []() {
    Serial.println("right");
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
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(pwm1, 255);
  analogWrite(pwm2, 255);
}

void turnRight() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(pwm1, 255);
  analogWrite(pwm2, 255);
}

void stopMotors() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  analogWrite(pwm1, 0);
  analogWrite(pwm2, 0);
}