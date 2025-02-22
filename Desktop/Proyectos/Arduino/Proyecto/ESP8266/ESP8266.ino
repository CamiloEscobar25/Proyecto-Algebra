#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <Servo.h>

#define TRIG D5
#define ECHO D6
#define SERVO_PIN D4

Servo servo;

// Configuración Wi-Fi
const char* ssid = ""; //red de internet
const char* password = ""; //contraseña

// Configuración del servidor MQTT
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_led_topic = "led/control";
const char* mqtt_servo_topic = "sensor/servo";
const char* mqtt_distance_topic = "sensor/distance";

WiFiClient espClient;
PubSubClient client(espClient);

const int ledPin = D1; // Pin donde está conectado el LED

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1); // Ajuste para evitar caracteres corruptos
  
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  
  servo.attach(SERVO_PIN);
  servo.write(90);
  
  WiFi.begin(ssid, password);
  Serial.print("Conectando a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido en tema: ");
  Serial.println(topic);
  
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println("Mensaje: " + message);
  
  // Solo se atiende el tópico de LED
  if (strcmp(topic, mqtt_led_topic) == 0) {
    if (message == "ON") {
      digitalWrite(ledPin, HIGH);
    } else if (message == "OFF") {
      digitalWrite(ledPin, LOW);
    }
  }
}

void reconnect() {
  // Se intenta reconectar solo una vez si no está conectado
  if (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP8266_Client")) {
      Serial.println("Conectado");
      client.subscribe(mqtt_led_topic);
    } else {
      Serial.print("Falló, rc=");
      Serial.println(client.state());
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  long duration;
  int distance;
  
  // Medición del sensor ultrasónico
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  duration = pulseIn(ECHO, HIGH);
  distance = duration * 0.034 / 2;
  
  if (distance < 2 || distance > 400) {
    distance = -1;
  }
  
  if (distance > 0 && distance < 10) {
    buscarCamino();
  }
  
  // Publicar datos en tópicos separados
  client.publish(mqtt_distance_topic, String(distance).c_str());
  Serial1.println(String(distance) + "," + String(servo.read())); // Enviar datos al Mega
  
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" , Servo: ");
  Serial.println(servo.read());
  
  delay(500);
}

void buscarCamino() {
  int distDerecha, distIzquierda;
  
  // Mover servo a la izquierda (0°) y medir
  servo.write(0);
  delay(300);
  distDerecha = medirDistancia();
  
  // Mover servo a la derecha (180°) y medir
  servo.write(180);
  delay(300);
  distIzquierda = medirDistancia();
  
  // Regresar a la posición central (90°)
  servo.write(90);
  delay(300);
  
  // Publicar en MQTT según la dirección elegida
  if (distDerecha > distIzquierda) {
    Serial.println("Turn Right");
    client.publish(mqtt_servo_topic, "Turn Right");
  } else {
    Serial.println("Turn Left");
    client.publish(mqtt_servo_topic, "Turn Left");
  }
}

int medirDistancia() {
  long duration;
  int d;
  
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  duration = pulseIn(ECHO, HIGH);
  d = duration * 0.034 / 2;
  
  if (d < 2 || d > 400) {
    return -1;
  }
  return d;
}

