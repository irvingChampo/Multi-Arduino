#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define PIR_PIN 14        
#define TRIG_PIN 13       
#define ECHO_PIN 12       
#define DHT_PIN 4         
#define DHTTYPE DHT11     

const char* ssid = ".";
const char* password = "Samuel1234";

const char* mqtt_server = "44.197.73.155";  
const int mqtt_port = 1883;               
const char* mqtt_user = "samuel";         
const char* mqtt_password = "samuel2004";
const char* mqtt_topic = "esp32.mqtt";     

WiFiClient espClient;
PubSubClient client(espClient);

DHT dht(DHT_PIN, DHTTYPE);
long duration;
int distance;
String pirStatus;

void setup() {
  Serial.begin(9600);

  pinMode(PIR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  dht.begin();

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);

  delay(2000);
  Serial.println("Inicialización completada");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("Conectado");
    } else {
      Serial.print("Error, rc=");
      Serial.print(client.state());
      Serial.println(" Intenta de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

void leerPIR() {
  int pirState = digitalRead(PIR_PIN);
  pirStatus = (pirState == HIGH) ? "Movimiento detectado" : "Sin movimiento";
  Serial.println(pirStatus);
}

void leerUltrasonico() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;

  Serial.print("Distancia: ");
  Serial.print(distance);
  Serial.println(" cm");
}

void leerDHT11() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Error al leer el sensor DHT11");
  } else {
    Serial.print("Humedad: ");
    Serial.print(humidity);
    Serial.print(" %\t");
    Serial.print("Temperatura: ");
    Serial.print(temperature);
    Serial.println(" *C");
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  leerPIR();
  leerUltrasonico();
  leerDHT11();

  String payload = "PIR: " + pirStatus + ", Distancia: " + String(distance) + " cm, Temp: " + String(dht.readTemperature()) + " C, Hum: " + String(dht.readHumidity()) + " %";
  client.publish(mqtt_topic, payload.c_str());

  delay(2000);
}
