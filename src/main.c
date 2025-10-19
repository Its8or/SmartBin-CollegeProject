#include <Servo.h>
#include <Adafruit_LiquidCrystal.h>

// ========================== Definições Globais e Pinos ==========================

const int SENSORGAS_PIN = A0; // recebe analog sinal do sensor de gas

const int RLED_SYS_PIN = 2;  // LED Vermelho do sistema da lixeira
const int GLED_SYS_PIN = 3;  // LED Verde do sistema da lixeira
const int SERVO_PIN = 4;     // controle Motor Servo
const int BUZZER_PIN = 5;    // controla o Buzzer Piezo
const int ECHO_PIN = 6;      // Conecta Sersor ultrassônico; Recebe som
const int TRIGGER_PIN = 7;   // Conecta Sersor ultrassônico; Emite som
const int BUTTON_PIN = 8;    // Pino 8 (usando INPUT_PULLUP)
const int RLED_GAS_PIN = 9;  // LED Vermelho (sensor de gas)
const int GLED_GAS_PIN = 10; // LED Verde (sensor de gas)

Adafruit_LiquidCrystal lcd(0);

// ========================== Variáveis Globais de Estado ==========================
// prefixo 'g_' apenas para mostrar se eh variavel global

unsigned long g_previousMillis = 0;
const long g_interval = 1000; // Intervalo de 1 segundo (1000 ms)

int g_gasLevel = 0;
// Variável para armazenar sacolas da semana
int g_contadorSacolas = 0;
// Estado anterior do botão (necessário para detectar a borda de descida/pressionamento)
int g_lastButtonStatus = LOW;
// Estado que define se o servo foi aberto manualmente pelo botão
bool g_servoManuallyOpen = false;
Servo g_servo;

// ========================== prototipagem de funcoes ==========================

/**
 * @brief Inicializa Leds(sistema e sensor de gas), Botao e Buzzer.
 */
void initializePINs();

/**
 * @brief Mede a distância em centímetros usando o sensor ultrassônico.
 * @param triggerPin Pino para enviar o pulso.
 * @param echoPin Pino para receber o pulso.
 * @return long Distância medida em centímetros.
 */
long readDistanceCm(int triggerPin, int echoPin);

/**
 * @brief Lida com a leitura do botão:
 * 1. Alterna o estado de abertura manual do servo (g_servoManuallyOpen).
 * 2. Incrementa o contador de sacolas (g_contadorSacolas) apenas ao ATIVAR o modo manual.
 */
void handleButtonPress();

/**
 * @brief Controla os LEDs (Vermelho/Verde) do sistema padrao
 * com base no estado de abertura manual do servo.
 */
void controlMainSystem();

/**
 * @brief Controla a posição do servo com base na distância lida e no estado manual.
 * @param distance Distancia atual medida em centímetros.
 */
void controlServo(long distance);

/**
 * @brief Controla os LEDs (Vermelho/Verde) do sensor de gas
 * com base no estado de abertura manual do servo
 * ativa o buzzer quando flag vermelha esta ativada.
 * @param gasFlag valor para poder controlar os lEDs do sensor de gás
 */
void controlGasSystem(int gasFlag);

/**
 * @brief Le sensor de gas e da feedback se ha gases de decomposicao.
 * @param gasLevel valor que informa a distancia do sensor para o gas.
 * @return retorna valores referentes a bandeira de deteccao de
 * gases de decomposicao (1=Verde, 2=Amarelo, 3=Vermelho)
 */
int gasWarningFlag(int gasLevel);

// ========================== SetUp e Loop ==========================

void setup()
{
    Serial.begin(9600);
    lcd.begin(16, 2);
    initializePINs(); // Inicializa LEDs, Botao
    lcd.print("Sacolas da ");
    lcd.setCursor(0, 1);
    lcd.print("semana:");

    // Inicializa o Servo
    g_servo.attach(SERVO_PIN);
    g_servo.write(0); // Garante que o servo comece FECHADO

    Serial.println("Sistema inicializado.");
}

void loop()
{
    // 1. Lida com o botão (servo manual e contador)
    handleButtonPress();

    // 2. Lê a distância
    long currentDistance = readDistanceCm(TRIGGER_PIN, ECHO_PIN);
    controlServo(currentDistance);

    // 3. Lê o Gás
    g_gasLevel = analogRead(A0); // Leitura do sensor de gas
    int gasFlag = gasWarningFlag(g_gasLevel);

    // ========================== Debug log ==========================
    Serial.print("Dist: ");
    Serial.print(currentDistance);
    Serial.print("cm | Gas (A0): ");
    Serial.print(g_gasLevel);
    Serial.print(" | Flag: ");
    Serial.println(gasFlag);
    // =================================================================

    // 4. Atualiza os LEDs e o LCD
    controlMainSystem();
    controlGasSystem(gasFlag);

    lcd.setCursor(8, 1);
    lcd.print(g_contadorSacolas);

    delay(100); // Adiciona um pequeno delay no loop para facilitar a leitura serial
}

// -------------------------------------------------------------------
// 					3. DEFINIÇÃO COMPLETA DAS FUNÇÕES
// -------------------------------------------------------------------

void initializePINs()
{
    // Configurações dos LEDs Sistema
    pinMode(GLED_SYS_PIN, OUTPUT);
    pinMode(RLED_SYS_PIN, OUTPUT);

    // Configurações do Buzzer
    pinMode(BUZZER_PIN, OUTPUT);

    // Configurações dos LEDs Sensor de Gas
    pinMode(GLED_GAS_PIN, OUTPUT);
    pinMode(RLED_GAS_PIN, OUTPUT);

    // Configurações do Botão
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

long readDistanceCm(int triggerPin, int echoPin)
{
    // Configura o pino Trigger para enviar o pulso
    pinMode(triggerPin, OUTPUT);
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);

    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);

    // Configura o pino Echo para receber o pulso e medir o tempo
    pinMode(echoPin, INPUT);
    long duration = pulseIn(echoPin, HIGH);

    // Converte a duração para distância em cm (usando 0.0343 cm/µs)
    return duration * 0.0343 / 2;
}

void handleButtonPress()
{
    int currentButtonStatus = digitalRead(BUTTON_PIN);

    // Detecção de borda de descida (transição de HIGH para LOW)
    // botão foi pressionado (usando INPUT_PULLUP)
    if (g_lastButtonStatus == HIGH && currentButtonStatus == LOW)
    {
        // Salva o estado atual ANTES de alternar
        bool oldServoManuallyOpen = g_servoManuallyOpen;

        // 1. Alterna o estado de abertura manual do Servo
        g_servoManuallyOpen = !g_servoManuallyOpen;

        // 2. Incrementa o contador de sacolas
        // A contagem so ocorre na transicao de FECHADO (false) para ABERTO (true)
        if (oldServoManuallyOpen == false && g_servoManuallyOpen == true)
        {
            g_contadorSacolas++;
        }

        Serial.print("Sacolas: ");
        Serial.println(g_contadorSacolas);

        // Pequeno atraso para debounce do botão
        delay(50);
    }

    g_lastButtonStatus = currentButtonStatus;
}

void controlServo(long distance)
{
    const int DISTANCE_THRESHOLD = 20; // Distancia limite em cm
    const int OPEN_ANGLE = 90;
    const int CLOSED_ANGLE = 0;

    if (distance <= DISTANCE_THRESHOLD)
    {
        // Caso de detecção de lixo: abre, espera, e fecha
        g_servo.write(OPEN_ANGLE);
        // trocar delay. Procurar forma melhor de nao bloquear codigo
        delay(3000);
        g_servo.write(CLOSED_ANGLE);
        // Reseta o estado manual apos o ciclo automático
        g_servoManuallyOpen = false;
    }
    else if (g_servoManuallyOpen)
    {
        // Abre se o botao foi pressionado para abertura manual
        g_servo.write(OPEN_ANGLE);
    }
    else
    {
        // Fecha por padrão
        g_servo.write(CLOSED_ANGLE);
    }
}

int gasWarningFlag(int gasLevel)
{
    // Bandeiras: 1 = Verde (Bom), 2 = Amarelo (Comeco de decomposicao), 3 = Vermelho (Troque a Sacola AGORA)

    // Perigo (Vermelho) - Se a leitura for alta
    if (gasLevel >= 30) // 30 ou mais
    {
        return 3;
    }
    // Atenção (Amarelo) - Se a leitura estiver na faixa média
    else if (gasLevel >= 21) // Entre 21 e 29
    {
        return 2;
    }
    // Bom (Verde) - Se a leitura estiver na faixa baixa
    // Isso cobre todos os valores de 0 até 579
    else // gasLevel < 580
    {
        return 1;
    }
}

void controlGasSystem(int gasFlag)
{
    // Bandeira Vermelha
    // (Lixo deve ser trocado AGORA)
    if (gasFlag == 3)
    {
        // Vermelho LIGADO, Verde DESLIGADO
        digitalWrite(GLED_GAS_PIN, LOW);
        digitalWrite(RLED_GAS_PIN, HIGH);
        tone(BUZZER_PIN, 300, 5000); // play 300Hz for 5 seconds
    }
    // Bandeira Amarela
    // (Lixo comecando a se decompor, troque a sacola...)
    else if (gasFlag == 2)
    {
        // Amarelo (Vermelho + Verde LIGADOS)
        digitalWrite(GLED_GAS_PIN, HIGH);
        digitalWrite(RLED_GAS_PIN, HIGH);
        tone(BUZZER_PIN, 100, 2000); //  play 300Hz for 2 seconds
    }
    // Bandeira Verde (Bom)
    else
    { // gasFlag deve ser 1
        // Verde LIGADO, Vermelho DESLIGADO
        digitalWrite(GLED_GAS_PIN, HIGH);
        digitalWrite(RLED_GAS_PIN, LOW);
        noTone(BUZZER_PIN);
    }
}

void controlMainSystem()
{
    // LED Vermelho LIGADO e Verde DESLIGADO quando está em modo manual/ativado
    if (g_servoManuallyOpen)
    {
        digitalWrite(RLED_SYS_PIN, HIGH);
        digitalWrite(GLED_SYS_PIN, LOW);
    }
    else
    {
        digitalWrite(RLED_SYS_PIN, LOW);
        digitalWrite(GLED_SYS_PIN, HIGH);
    }
}
