#include <Servo.h>
#include <Adafruit_LiquidCrystal.h>

// --- Definições Globais e Pinos ---
const int TRIGGER_PIN = 7;
const int ECHO_PIN 	  = 6;
const int BUTTON_PIN  = 8; // Pino 8 (usando INPUT_PULLUP)
const int SERVO_PIN   = 4;
const int LED_RED 	  = 2; // LED Vermelho
const int LED_GREEN   = 3; // LED Verde


// --- Variáveis de Estado ---
int g_contadorSacolas = 0;
// Estado anterior do botão (necessário para detectar a borda de descida/pressionamento)
int g_lastButtonStatus = LOW;	
// Estado que define se o servo foi aberto manualmente pelo botão
bool g_servoManuallyOpen = false;	
Servo g_servo;


Adafruit_LiquidCrystal lcd(0);

// -------------------------------------------------------------------
// 				1. PROTÓTIPOS DE FUNÇÕES (DECLARAÇÃO)
// -------------------------------------------------------------------

/**
* @brief Inicializa todos os pinos (LEDs, Botão).
*/
void inicializarPinos();

/**
* @brief Controla os LEDs (Vermelho/Verde) com base no estado de abertura manual do servo.
*/
void controlarLED();

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
 * @brief Controla a posição do servo com base na distância lida e no estado manual.
 * @param distance Distância atual medida em centímetros.
 */
void controlServo(long distance);

// -------------------------------------------------------------------
// 					2. SETUP E LOOP
// -------------------------------------------------------------------

void setup()
{
 	Serial.begin(9600);
 	lcd.begin(16, 2);
	inicializarPinos(); // Inicializa LEDs, Botão e Servo
 	lcd.print("Inicializando...");
 	delay(3000);
 	lcd.clear();
	lcd.print("Sacolas da ");
 	lcd.setCursor(0,1);
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

 // 3. Controla o servo com base na distância e estado
 controlServo(currentDistance);
 
 // 4. Atualiza o LED e o LCD
 controlarLED();
 	lcd.setCursor(8,1);
	lcd.print(g_contadorSacolas);
}

// -------------------------------------------------------------------
// 					3. DEFINIÇÃO COMPLETA DAS FUNÇÕES
// -------------------------------------------------------------------

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
 // Indica que o botão foi pressionado (usando INPUT_PULLUP)
 if (g_lastButtonStatus == HIGH && currentButtonStatus == LOW) {
 	
 	// Salva o estado atual ANTES de alternar
 	bool oldServoManuallyOpen = g_servoManuallyOpen;
 	
 	// 1. Alterna o estado de abertura manual do Servo
 	g_servoManuallyOpen = !g_servoManuallyOpen;
 	
 	// 2. Incrementa o contador de sacolas:
 	// A contagem só ocorre na transição de FECHADO (false) para ABERTO (true)
 	if (oldServoManuallyOpen == false && g_servoManuallyOpen == true) {
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
 const int DISTANCE_THRESHOLD = 100; // Distância limite em cm
 const int OPEN_ANGLE = 90;
 const int CLOSED_ANGLE = 0;
 
 if (distance <= DISTANCE_THRESHOLD) {
 	// Caso de detecção de lixo: abre, espera, e fecha
 	g_servo.write(OPEN_ANGLE);
 	delay(3000);	
 	g_servo.write(CLOSED_ANGLE);
 	// Reseta o estado manual após o ciclo automático
 	g_servoManuallyOpen = false;	
 } else if (g_servoManuallyOpen) {
 	// Abre se o botão foi pressionado para abertura manual
 	g_servo.write(OPEN_ANGLE);
 } else {
 	// Fecha por padrão
 	g_servo.write(CLOSED_ANGLE);
 }
}

// Função para configurar os pinos (inclui LEDs e Botão)
void inicializarPinos()
{
 // Configurações dos LEDs
 pinMode(LED_GREEN, OUTPUT);
 pinMode(LED_RED, OUTPUT);
 
 // Configurações do Botão
 pinMode(BUTTON_PIN, INPUT_PULLUP); 
}

// Função para escrever o estado na saída do LED (Lógica: Vermelho ON -> Manual, Verde ON -> Normal)
void controlarLED()
{
    // LED Vermelho LIGADO e Verde DESLIGADO quando está em modo manual/ativado
    if (g_servoManuallyOpen) {
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, LOW);
    } 
    // LED Verde LIGADO e Vermelho DESLIGADO no estado normal/desativado
    else {
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_GREEN, HIGH);
    }
}
