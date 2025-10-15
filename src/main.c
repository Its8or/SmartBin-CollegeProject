#include <Servo.h>

int distance = 0;
int bottom = 8;
int contadorSacolas = 0;
int lastStatus = LOW;
Servo servo_4;

// Button Status
int button_status = LOW;
int keep_button_status = LOW;


long readUltrasonicDistance(int triggerPin, int echoPin)
{
  pinMode(triggerPin, OUTPUT);  
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
 
  digitalWrite(triggerPin, HIGH); // envia um pulso
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT); // vai receber o pulso que vem do trigger
  
  return pulseIn(echoPin, HIGH); // retorna o distance em m/s
} 

void setup()
{
  Serial.begin(9600);
  servo_4.attach(4, 500, 2500);
  servo_4.attach(4);
  pinMode(bottom, INPUT_PULLUP);
}

void loop()
{
  button_status = digitalRead(bottom);
  if(lastStatus == HIGH && button_status == LOW){
    contadorSacolas++;
    Serial.println(contadorSacolas);
    delay(50); //debounce
  }
  lastStatus = button_status;
  
  distance = 0.01723 * readUltrasonicDistance(7, 6);
 // Serial.println(distance);
  
  if (button_status == HIGH) {
    keep_button_status = !keep_button_status;
    delay(50); //debounce
  }
 
  if(distance <= 100){
    servo_4.write(90);
    delay(5000);
    servo_4.write(0);
  } else {
  	servo_4.write(0);
  }
  
  if(keep_button_status == HIGH){
    servo_4.write(90);
  } else {
    servo_4.write(0);
  }
} 
