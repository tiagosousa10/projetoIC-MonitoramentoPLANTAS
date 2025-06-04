# 🌿 Monitoramento de Plantas Domésticas com Feedback em Tempo Real

Projeto desenvolvido para a disciplina de **Internet das Coisas (IoT)** no Instituto Politécnico de Bragança - Junho 2025.

Autores:  
- Tiago Sousa (a44180)  
- Rafael Saraiva (54946)

---

## 📌 Objetivo

Criar um sistema inteligente para monitorizar plantas domésticas com atuação automática/manual e feedback visual em tempo real. Utiliza sensores ligados a um ESP32 que enviam dados via MQTT para uma dashboard no Node-RED, armazenando informações no InfluxDB.

---

## ⚙️ Arquitetura do Sistema

- **ESP32** conectado a sensores ambientais e atuadores  
- **MQTT** como protocolo de comunicação  
- **Node-RED** com dashboard interativa para visualização e controlo  
- **InfluxDB** para histórico de dados  
- **LED RGB** para sinalização do estado da planta  
- **Modo de rega automático/manual** com controlo via dashboard  

---

## 🔧 Componentes Utilizados

### Hardware

- ESP32 DevKit V4  
- Sensor DHT22 (temperatura e humidade do ar)  
- LDR (luminosidade)  
- Potenciómetro (simula humidade do solo)  
- LED RGB (verde, amarelo, vermelho)  
- LED Vermelho (simula bomba de água)  
- Resistências: 3x 22Ω, 1x 220Ω  

### Software

- Arduino IDE  
- Simulador Wokwi ([🔗 Simular](https://wokwi.com/projects/431032509596803073))  
- Node-RED + Node-RED Dashboard  
- InfluxDB  
- MQTT Broker: `broker.emqx.io`  
- Bibliotecas:
  - [DHT sensor library](https://github.com/adafruit/DHT-sensor-library)
  - [PubSubClient](https://pubsubclient.knolleary.net)

---

## 🔌 Ligações dos Componentes

| Componente          | Pino ESP32   |
|---------------------|--------------|
| DHT22               | GPIO 14      |
| Potenciómetro       | GPIO 34      |
| LDR                 | GPIO 35      |
| LED RGB (R, G, B)   | GPIOs 16/17/18 |
| LED Vermelho (bomba)| GPIO 23      |
| GND / VCC comum     | GND / 3.3V   |

> Diagrama disponível em `diagram.json` (Wokwi)

---

## 🧠 Lógica do Sistema

### Estados da Planta:

- **Saudável (LED Verde)**  
  - Temperatura: 18–28 °C  
  - Humidade do ar: 40–70%  
  - Humidade do solo > 2500  
  - Luminosidade > 1500  

- **Alerta (LED Amarelo)**  
  - 1 ou 2 parâmetros ligeiramente fora dos ideais  

- **Crítico (LED Vermelho)**  
  - Temperatura < 10 °C ou > 35 °C  
  - Humidade do ar < 30% ou > 80%  
  - Humidade do solo < 1500  
  - Luminosidade < 1000  

### Controlo da Rega:

- **Automático:** se humidade do solo < 1500, a bomba é ligada  
- **Manual:** o utilizador aciona a bomba via botão no dashboard  

---

## 📊 Node-RED Dashboard

- Medidores para temperatura, humidade do ar, solo e luminosidade  
- Estado da planta com texto e ícones  
- Dropdown de modo de rega (automático/manual)  
- Botões para ligar/desligar rega no modo manual  

> Flows disponíveis em `flows-node-red.json`

---

## 💾 Armazenamento de Dados

- Os dados são enviados e armazenados no InfluxDB:
  - `temperatura_data`
  - `humidade_data`
  - `humidade_solo_data`
  - `luminosidade_data`

---

## 🧩 Explicação do Código ESP32

O ficheiro `sketch.ino` está dividido em blocos funcionais, cada um com uma função clara no funcionamento do sistema:

### 📚 1. Bibliotecas e Definições

```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
```

- `WiFi.h`: gere a conexão à rede sem fios.  
- `PubSubClient.h`: permite comunicação com o broker MQTT.  
- `DHT.h`: usada para leitura do sensor de temperatura e humidade DHT22.

---

### 🔐 2. Credenciais e Configuração

```cpp
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char* clientID = "esp32_planta_123";
```

Define os dados para ligação Wi-Fi e ao broker MQTT.

---

### 🔌 3. Pinos e Constantes

```cpp
#define DHTPIN 14
#define DHTTYPE DHT22
#define LDR_PIN 35
#define POT_PIN 34
#define RED_PIN 16
#define GREEN_PIN 17
#define BLUE_PIN 18
#define PUMP_PIN 23
#define SOIL_THRESHOLD 1500
```

Configura os pinos usados pelos sensores e atuadores.  
A constante `SOIL_THRESHOLD` define o limite mínimo de humidade do solo.

---

### 🧱 4. Objetos e Variáveis Globais

```cpp
DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

bool modoManual = false;
bool estadoManual = false;
```

Criação de objetos para o sensor DHT, cliente Wi-Fi e cliente MQTT.  
As variáveis `modoManual` e `estadoManual` controlam a lógica de rega.

---

### 📩 5. Callback MQTT

```cpp
void callback(char* topic, byte* payload, unsigned int length)
```

Esta função reage a mensagens recebidas nos tópicos MQTT:
- `"planta/rega/modo"`: muda entre modo automático e manual.
- `"planta/rega/manual"`: ativa ou desativa a bomba (LED vermelho) em modo manual.

---

### ⚙️ 6. Setup

```cpp
void setup() {
  dht.begin();
  WiFi.begin(ssid, password);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  // Configura pinos como OUTPUT
}
```

- Inicializa o sensor DHT.  
- Liga-se ao Wi-Fi.  
- Define o servidor MQTT e a função de callback.  
- Inicializa os pinos de atuadores como saída.

---

### 🌡️ 7. Leitura dos Sensores

```cpp
float temp = dht.readTemperature();
float hum = dht.readHumidity();
int ldr = analogRead(LDR_PIN);
int solo = analogRead(POT_PIN);
```

Recolhe os valores de temperatura, humidade do ar, luminosidade e humidade do solo.

---

### 📤 8. Publicação dos Dados

```cpp
client.publish("planta/temperatura", String(temp).c_str());
client.publish("planta/humidade_ar", String(hum).c_str());
client.publish("planta/luminosidade", String(ldr).c_str());
client.publish("planta/humidade_solo", String(solo).c_str());
```

Publica os dados lidos nos tópicos MQTT para visualização e armazenamento.

---

### 📊 9. Estado da Planta

```cpp
int estado = 1; // 1 = saudável

if (...) estado = 1;
else if (...) estado = 2;
else estado = 3;

client.publish("planta/estado", String(estado).c_str());
```

- Classifica o estado da planta com base em limites definidos.  
- Envia o estado para o Node-RED, que atualiza o LED RGB e o texto na dashboard.

---

### 💧 10. Controlo da Rega

```cpp
if (!modoManual && solo < SOIL_THRESHOLD) {
  digitalWrite(PUMP_PIN, HIGH);
  client.publish("planta/rega", "on");
}
```

- Em modo automático: ativa a bomba se a humidade do solo for baixa.  
- Em modo manual: a bomba é controlada via comandos MQTT externos.

---

### 🌈 11. Feedback Visual com LED RGB

```cpp
digitalWrite(RED_PIN, estado == 3);
digitalWrite(GREEN_PIN, estado == 1);
digitalWrite(BLUE_PIN, estado == 2);
```

Define a cor do LED RGB com base no estado da planta:
- Verde: saudável  
- Azul: alerta  
- Vermelho: crítico  

---

### 🔁 12. Loop Principal

```cpp
void loop() {
  if (!client.connected()) reconnect();
  client.loop();
  // Aguarda 5 segundos entre envios
}
```

- Mantém a ligação com o broker MQTT.  
- Executa continuamente a lógica de leitura e publicação dos dados.

## Conclusão

Este projeto demonstra como a Internet das Coisas pode ser aplicada de forma eficaz no cuidado de plantas domésticas, combinando sensores, automação e monitorização remota.
A integração entre o ESP32, MQTT, Node-RED e InfluxDB permite criar uma solução funcional, económica e extensível.

Com um sistema de feedback visual (LED RGB), controlo manual ou automático da rega e visualização em tempo real dos dados ambientais, este projeto oferece uma abordagem acessível e educativa para gestão inteligente de recursos naturais em ambiente doméstico.
