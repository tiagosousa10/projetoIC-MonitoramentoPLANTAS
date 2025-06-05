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

O ficheiro `sketch.ino` está dividido em blocos funcionais, com responsabilidades bem definidas para monitorizar os sensores e controlar o sistema de rega de forma inteligente.

---

### 📚 1. Bibliotecas e Definições

```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
```

- `WiFi.h`: gere a ligação Wi-Fi.
- `PubSubClient.h`: comunicação MQTT com o broker.
- `DHT.h`: leitura do sensor de temperatura e humidade DHT22.

---

### 🔐 2. Credenciais e Configuração

```cpp
const char* ssid = "Wokwi-GUEST";
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char* clientID = "esp32_planta_123";
```

Define os dados de rede e do broker MQTT que o ESP32 irá utilizar.

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

Associa os pinos aos sensores e atuadores. A constante `SOIL_THRESHOLD` define o nível crítico de humidade do solo.

---

### 🧱 4. Objetos e Estados Globais

```cpp
enum ModoRega { DESLIGADO, MANUAL, AUTO };
ModoRega modoRega = DESLIGADO;
bool estadoManual = false;
```

- `modoRega`: enumeração com os três modos possíveis de operação.
- `estadoManual`: armazena o estado da bomba no modo manual.

---

### 📩 5. Callback MQTT

```cpp
void callback(char* topic, byte* payload, unsigned int length)
```

Processa mensagens recebidas via MQTT:

- `planta/rega/modo`: altera o modo de rega (`auto`, `manual` ou `desligado`).
- `planta/rega/manual`: ativa/desativa a bomba em modo manual com `"on"` ou `"off"`.

Inclui mensagens de debug via `Serial.println()`.

---

### ⚙️ 6. Setup Inicial

```cpp
void setup()
```

Inicializa:

- comunicação serial;
- sensores e pinos de entrada/saída;
- ligação Wi-Fi;
- cliente MQTT com subscrição aos tópicos relevantes.

---

### 🌡️ 7. Leitura dos Sensores

```cpp
float temp = dht.readTemperature();
float hum = dht.readHumidity();
int luz = analogRead(LDR_PIN);
int solo = analogRead(POT_PIN);
```

Recolhe dados ambientais para avaliação e envio à dashboard.

---

### 📤 8. Publicação dos Dados

```cpp
client.publish("planta/temperatura", ...);
client.publish("planta/humidade_ar", ...);
client.publish("planta/luminosidade", ...);
client.publish("planta/humidade_solo", ...);
```

Envia os dados lidos por MQTT para visualização e armazenamento.

---

### 📊 9. Avaliação do Estado da Planta

```cpp
int avaliarPlanta(float t, float h, int l, int s)
```

Analisa os valores dos sensores e devolve:

- `1` para saudável;
- `2` para alerta;
- `3` para estado crítico.

---

### 🌈 10. Feedback Visual com LED RGB

```cpp
mostrarEstadoLED(estado)
```

Controla as cores do LED RGB com base no estado da planta:

- Verde = saudável  
- Amarelo = alerta  
- Vermelho = crítico

---

### 💧 11. Controlo Inteligente da Rega

```cpp
switch (modoRega) {
  case DESLIGADO:
    // bomba sempre desligada
  case MANUAL:
    // bomba controlada por comando externo
  case AUTO:
    // bomba ativa se humidade do solo < 1500
}
```

Lógica central do sistema:

- `DESLIGADO`: bomba desativa.
- `MANUAL`: bomba acionada por comando MQTT (`on`/`off`).
- `AUTO`: bomba ligada automaticamente com base na humidade do solo.

---

### 🔁 12. Loop Principal

```cpp
void loop()
```

- Mantém a ligação ao broker;
- Lê sensores, publica dados;
- Avalia o estado da planta;
- Controla a bomba de rega de acordo com o modo selecionado;
- Atualiza LEDs e console com feedback contínuo.

## Conclusão

Este projeto demonstra como a Internet das Coisas pode ser aplicada de forma eficaz no cuidado de plantas domésticas, combinando sensores, automação e monitorização remota.
A integração entre o ESP32, MQTT, Node-RED e InfluxDB permite criar uma solução funcional, económica e extensível.

Com um sistema de feedback visual (LED RGB), controlo manual ou automático da rega e visualização em tempo real dos dados ambientais, este projeto oferece uma abordagem acessível e educativa para gestão inteligente de recursos naturais em ambiente doméstico.
