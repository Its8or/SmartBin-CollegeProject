# 🗑️ Lixeira Inteligente com Arduino e Contador de Sacolas

Este projeto implementa uma lixeira inteligente automatizada que utiliza um sensor ultrassônico para abertura automática e um controle manual com botão para monitorar o descarte de sacolas (ou ciclos de abertura manual) em um display LCD.

## 🚀 Funcionalidades

* **Abertura Automática:** O servo motor abre a tampa da lixeira quando um objeto (lixo) é detectado a uma distância inferior a $100 \text{ cm}$. A tampa fecha automaticamente após $3$ segundos.
* **Controle Manual:** Um botão permite abrir ou fechar o servo motor manualmente.
* **Contador de Sacolas (ou Descartes):** O contador é incrementado *apenas* quando o modo manual é **ativado** (transição de FECHADO para ABERTO), permitindo registrar a frequência de descarte.
* **Indicação de Estado (LEDs):**
    * **LED Verde:** Sistema em modo automático (normal).
    * **LED Vermelho:** Sistema em modo manual (aberto pelo botão).
* **Display LCD:** Exibe uma mensagem de inicialização e o total de "Sacolas da semana".

## 🛠️ Componentes Necessários

| Componente | Quantidade | Função |
| :--- | :--- | :--- |
| Arduino (Uno, Nano, etc.) | 1 | Microcontrolador principal. |
| Servo Motor (e.g., SG90) | 1 | Controla a abertura/fechamento da tampa. |
| Sensor Ultrassônico (HC-SR04) | 1 | Mede a distância para detecção automática. |
| Display LCD (16x2 I2C ou SPI) | 1 | Exibe o contador de sacolas e status. (O código usa `Adafruit_LiquidCrystal` que é comumente associada a displays I2C ou SPI, mas o construtor `lcd(0)` sugere uma configuração não padrão, talvez baseada em endereçamento interno ou barramento de dados.) |
| Botão Push-Button | 1 | Para controle manual e contagem. |
| LED Vermelho | 1 | Indica modo manual. |
| LED Verde | 1 | Indica modo automático. |
| Resistores | 2 | Para os LEDs (valor típico de $220 \Omega$). |
| Jumpers (Fios) | Vários | Conexões entre os componentes. |

## 📌 Esquema de Conexão (Pinagem)

| Componente | Pino do Arduino | Função |
| :--- | :--- | :--- |
| **Sensor Ultrassônico - Trigger** | D7 | Envia o pulso. |
| **Sensor Ultrassônico - Echo** | D6 | Recebe o pulso (medida de tempo). |
| **Botão (com PULL-UP)** | D8 | Ativa/Desativa o modo manual. |
| **Servo Motor - Sinal** | D4 | Controle de posição. |
| **LED Vermelho - Sinal** | D2 | Indica modo manual. |
| **LED Verde - Sinal** | D3 | Indica modo automático. |
| **Display LCD** | (Endereçamento/Pinos) | Exibição de dados. |

***Nota sobre o Botão:*** *O botão está configurado como `INPUT_PULLUP`, o que significa que ele deve ser conectado entre o Pino D8 e o **GND** (Terra). O resistor de pull-up é interno, simplificando a fiação.*

## ⚙️ Bibliotecas Necessárias

Certifique-se de que as seguintes bibliotecas estão instaladas no seu IDE do Arduino:

1.  **`Servo.h`**: (Geralmente pré-instalada no IDE do Arduino)
2.  **`Adafruit_LiquidCrystal.h`**: (Instalar via Gerenciador de Bibliotecas. Se você estiver usando um display I2C padrão, talvez precise da biblioteca `LiquidCrystal_I2C`, mas o código está escrito para a versão da Adafruit.)

## 💻 Detalhes do Código

O código está estruturado com as seguintes funções principais:

| Função | Descrição |
| :--- | :--- |
| `setup()` | Inicializa a comunicação serial, o LCD, os pinos e define o servo para $0^\circ$ (fechado). |
| `loop()` | Ciclo principal: lida com o botão, lê a distância, controla o servo e atualiza o display/LEDs. |
| `readDistanceCm()` | Executa a rotina do HC-SR04 e retorna a distância em centímetros. |
| `handleButtonPress()` | Implementa a **detecção de borda de descida** para evitar leituras múltiplas e alterna o estado manual (`g_servoManuallyOpen`). Incrementa o contador. |
| `controlServo()` | Implementa a lógica de controle: se a distância for crítica ($\le 100 \text{ cm}$), abre/fecha em ciclo. Caso contrário, segue o estado manual. |
| `controlarLED()` | Define os estados dos LEDs Verde e Vermelho com base em `g_servoManuallyOpen`. |
