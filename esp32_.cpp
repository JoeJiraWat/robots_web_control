#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// Replace with your desired AP credentials
const char* ssid = "RobotControl";
const char* password = "robot1234";

// Motor Left
const int in1 = 35;
const int in2 = 33;
const int pwm1 = 32;

// Motor Right
const int in3 = 14;
const int in4 = 13;
const int pwm2 = 12;

// Servo pins
const int servo1Pin = 25;
const int servo2Pin = 26;

// --- PWM Configuration ---
const int PWM_CHANNEL_LEFT = 0;
const int PWM_CHANNEL_RIGHT = 1;
const int PWM_FREQ = 20000;
const int PWM_RES = 8;

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

  // Configure PWM channels
  // ledcSetup(PWM_CHANNEL_LEFT, PWM_FREQ, PWM_RES);
  // ledcSetup(PWM_CHANNEL_RIGHT, PWM_FREQ, PWM_RES);

  // Attach PWM channels to GPIO pins
  // ledcAttachPin(pwm1, PWM_CHANNEL_LEFT);
  // ledcAttachPin(pwm2, PWM_CHANNEL_RIGHT);
  ledcAttachChannel(pwm1, PWM_FREQ, PWM_RES, PWM_CHANNEL_LEFT);
  ledcAttachChannel(pwm2, PWM_FREQ, PWM_RES, PWM_CHANNEL_RIGHT);

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
    moveForward();
    server.send(200, "text/plain", "OK");
  });

  server.on("/backward", []() {
    moveBackward();
    server.send(200, "text/plain", "OK");
  });

  server.on("/left", []() {
    turnLeft();
    server.send(200, "text/plain", "OK");
  });

  server.on("/right", []() {
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
  ledcWrite(PWM_CHANNEL_LEFT, 255);
  ledcWrite(PWM_CHANNEL_RIGHT, 255);
}

void moveBackward() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  ledcWrite(PWM_CHANNEL_LEFT, 255);
  ledcWrite(PWM_CHANNEL_RIGHT, 255);
}

void turnLeft() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  ledcWrite(PWM_CHANNEL_LEFT, 255);
  ledcWrite(PWM_CHANNEL_RIGHT, 255);
}

void turnRight() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  ledcWrite(PWM_CHANNEL_LEFT, 255);
  ledcWrite(PWM_CHANNEL_RIGHT, 255);
}

void stopMotors() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  ledcWrite(PWM_CHANNEL_LEFT, 0);
  ledcWrite(PWM_CHANNEL_RIGHT, 0);
}