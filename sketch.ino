#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// --- Wi-Fi ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// --- MQTT ---
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char* clientID = "esp32_planta_123";

// --- Pinos ---
#define DHTPIN 14
#define DHTTYPE DHT22
#define LDR_PIN 35
#define POT_PIN 34
#define RED_PIN 16
#define GREEN_PIN 17
#define BLUE_PIN 18
#define PUMP_PIN 23
#define SOIL_THRESHOLD 1500

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

bool modoManual = false;
bool estadoManual = false;

// MQTT Callback
void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String msg = String((char*)payload);
  String topico = String(topic);

  if (topico == "planta/rega/modo") {
    modoManual = (msg == "manual");
    Serial.println("[INFO] Modo de rega definido para: " + msg);
  } else if (topico == "planta/rega/manual") {
    estadoManual = (msg == "on");
    digitalWrite(PUMP_PIN, estadoManual ? HIGH : LOW);  // Aplica imediatamente
    Serial.println("[INFO] Rega manual alterada para: " + msg);
  }
}

void setup_wifi() {
  delay(10);
  Serial.println("[INFO] Ligando WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[INFO] WiFi conectado!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("[INFO] Conectando MQTT...");
    if (client.connect(clientID)) {
      Serial.println(" conectado.");
      client.subscribe("planta/rega/modo");
      client.subscribe("planta/rega/manual");
    } else {
      Serial.print("Erro: ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(LDR_PIN, INPUT);
  pinMode(POT_PIN, INPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  Serial.println("[INFO] Sistema inicializado com sucesso.");
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int luz = analogRead(LDR_PIN);
  int solo = analogRead(POT_PIN);

  Serial.println("------ LEITURA ------");
  Serial.printf("Temperatura: %.2f °C\n", temp);
  Serial.printf("Humidade do Ar: %.2f %%\n", hum);
  Serial.printf("Luminosidade: %d\n", luz);
  Serial.printf("Humidade do Solo: %d\n", solo);

  client.publish("planta/temperatura", String(temp).c_str());
  client.publish("planta/humidade_ar", String(hum).c_str());
  client.publish("planta/luminosidade", String(luz).c_str());
  client.publish("planta/humidade_solo", String(solo).c_str());

  int estado = avaliarPlanta(temp, hum, luz, solo);
  mostrarEstadoLED(estado);
  client.publish("planta/estado", String(estado).c_str());

  // Lógica de rega
  if (modoManual) {
    digitalWrite(PUMP_PIN, estadoManual ? HIGH : LOW);
    client.publish("planta/rega", estadoManual ? "on" : "off");
    Serial.println("[INFO] Modo manual: bomba " + String(estadoManual ? "ligada" : "desligada"));
  } else {
    bool regar = (solo < SOIL_THRESHOLD);
    digitalWrite(PUMP_PIN, regar ? HIGH : LOW);
    client.publish("planta/rega", regar ? "on" : "off");
    Serial.println("[INFO] Modo auto: bomba " + String(regar ? "ligada" : "desligada"));
  }

  delay(5000);
}

int avaliarPlanta(float t, float h, int l, int s) {
  int alerta = 0;
  if (t < 10 || t > 35) return 3;
  if (t < 18 || t > 28) alerta++;
  if (h < 30 || h > 80) return 3;
  if (h < 40 || h > 70) alerta++;
  if (s < 1500) return 3;
  if (s < 2500) alerta++;
  if (l < 1000) return 3;
  if (l < 1500) alerta++;
  return (alerta >= 2) ? 2 : 1;
}

void mostrarEstadoLED(int estado) {
  switch (estado) {
    case 1:
      digitalWrite(RED_PIN, LOW);
      digitalWrite(GREEN_PIN, HIGH);
      digitalWrite(BLUE_PIN, LOW);
      break;
    case 2:
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(GREEN_PIN, HIGH);
      digitalWrite(BLUE_PIN, LOW);
      break;
    case 3:
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(BLUE_PIN, LOW);
      break;
  }
}
