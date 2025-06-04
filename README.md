# 🌿 Monitoramento de Plantas Domésticas com Feedback em Tempo Real

Projeto desenvolvido para a disciplina de **Internet das Coisas (IoT)** no Instituto Politécnico de Bragança - Junho 2025.

Autores:  
- Tiago Sousa (a44180)  
- Rafael Saraiva (54946)

## 📌 Objetivo

Criar um sistema inteligente para monitorizar plantas domésticas com atuação automática/manual e feedback visual em tempo real. Utiliza sensores ligados a um ESP32 que enviam dados via MQTT para uma dashboard no Node-RED, armazenando informações no InfluxDB.

---

## ⚙️ Arquitetura do Sistema

- **ESP32** conectado a sensores ambientais e atuadores;
- **MQTT** como protocolo de comunicação;
- **Node-RED** com dashboard interativa para visualização e controlo;
- **InfluxDB** para histórico de dados;
- **LED RGB** para sinalização do estado da planta;
- **Modo de rega automático/manual** com controlo via dashboard.

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
- Simulador Wokwi [🔗 Simular](https://wokwi.com/projects/431032509596803073)
- Node-RED + Node-RED Dashboard
- InfluxDB
- MQTT Broker (broker.emqx.io)
- Bibliotecas:
  - [DHT sensor library](https://github.com/adafruit/DHT-sensor-library)
  - [PubSubClient](https://pubsubclient.knolleary.net)

---

## 🔌 Ligações dos Componentes

| Componente     | Pino ESP32 |
|----------------|------------|
| DHT22          | GPIO 14    |
| Potenciómetro  | GPIO 34    |
| LDR            | GPIO 35    |
| LED RGB (R, G, B) | GPIOs 16, 17, 18 |
| LED Vermelho (bomba) | GPIO 23 |
| GND/VCC comum  | GND / 3.3V |

> Diagrama disponível no ficheiro `diagram.json` e visualizável no Wokwi.

---

## 🧠 Lógica do Sistema

### Estados da Planta:

- **Saudável (LED Verde)**  
  - Temperatura: 18–28 °C  
  - Humidade do ar: 40–70%  
  - Humidade do solo > 2500  
  - Luminosidade > 1500  

- **Alerta (LED Amarelo)**  
  - 1 ou 2 parâmetros ligeiramente fora dos valores ideais

- **Crítico (LED Vermelho)**  
  - Temperatura < 10 °C ou > 35 °C  
  - Humidade do ar < 30% ou > 80%  
  - Humidade do solo < 1500  
  - Luminosidade < 1000  

### Controlo da Rega:

- **Modo Automático**  
  - A bomba é acionada quando a humidade do solo < 1500.

- **Modo Manual**  
  - O utilizador liga/desliga a bomba através de botões no dashboard.

---

## 📊 Node-RED Dashboard

Inclui:

- Medidores (gauges) e gráficos para:
  - Temperatura
  - Humidade do ar
  - Humidade do solo
  - Luminosidade
- Texto com estado atual da planta
- Dropdown para modo de rega (automático/manual)
- Botões para ligar/desligar rega no modo manual

> Flows disponíveis em `flows-node-red.json`

---

## 💾 Armazenamento de Dados

- Cada tipo de dado é armazenado em `InfluxDB`:
  - `temperatura_data`
  - `humidade_data`
  - `humidade_solo_data`
  - `luminosidade_data`

---

## ▶️ Como Executar

1. **Clone o repositório**
```bash
git clone https://github.com/tiagosousa10/projetoIC-MonitoramentoPLANTAS.git
