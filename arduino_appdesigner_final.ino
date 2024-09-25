#include <DHT.h>
#include <DHT_U.h>

#define DHT11_TYPE DHT11
#define DHT22_TYPE DHT22

const byte OC1A_PIN = 9;
const byte OC1B_PIN = 10;

const word PWM_FREQ_HZ = 25000; // Frecuencia PWM en Hz (25 kHz)
const word TCNT1_TOP = 16000000 / (2 * PWM_FREQ_HZ); // Valor máximo del contador

int dhtpin1 = 7;  // Pin del DHT11
int dhtpin2 = 8;  // Pin del DHT22

DHT HT1(dhtpin1, DHT11_TYPE);  // Sensor DHT11
DHT HT2(dhtpin2, DHT22_TYPE);  // Sensor DHT22

int dt = 1000;          // Tiempo de espera entre cada ciclo de evaluación

String t1, h1;          // Variables para almacenar el valor leído y luego ser enviadas
String t2, h2;
String velocidad_str = "0";
int velocidad_int = 0;  // Variable que almacena la velocidad del ventilador
int stream = 0;         // Variable para habilitar/deshabilitar el envío de datos dependiendo de la instrucción recibida

String buffer = "";
String message_from_serial = "";
String message_to_serial = "";
bool new_message_from_serial = false; 
String pos;
int num;

void setup() {

  Serial.begin(9600);
  HT1.begin();
  HT2.begin();

  // Configuración del PWM
  pinMode(OC1A_PIN, OUTPUT);
  pinMode(OC1B_PIN, OUTPUT);
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  // Configurar PWM en OC1A y OC1B
  TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);  
  TCCR1B |= (1 << WGM13) | (1 << CS10);  // PWM fase correcta, sin prescaler
  ICR1 = TCNT1_TOP;  // Establecer el valor máximo del temporizador
}

void loop() {
  // Leer la señal de los sensores
  t1 = String(round(HT1.readTemperature()));
  h1 = String(round(HT1.readHumidity()));
  t2 = String(round(HT2.readTemperature()));
  h2 = String(round(HT2.readHumidity()));

  // Inspecciona cíclicamente el puerto serial
  serialEvent();

  if (new_message_from_serial) {
    pos = message_from_serial.indexOf(";");
    num = pos.toInt();

    if (num == 0) {
      velocidad_str = message_from_serial.substring(1, -1);
      velocidad_int = velocidad_str.toInt();
      setPwmDuty(velocidad_int);  // Ajustar la velocidad del ventilador
    }  
  }

  // Enviar datos por el puerto serial
  Serial.print(t1 + "," + h1 + "," + t2 + "," + h2 + "," + velocidad_str + "\n");

  delay(dt);
}

void setPwmDuty(byte duty) {
  // Ajustar el ciclo de trabajo común para OC1A y OC1B
  word dutyCycle = (word)(duty * TCNT1_TOP) / 100;
  OCR1A = dutyCycle;  // Ajustar ciclo de trabajo en OC1A (pin 9)
  OCR1B = dutyCycle;  // Ajustar ciclo de trabajo en OC1B (pin 10)
}

void serialEvent() {
  while (Serial.available()) {
    // Recibir nuevo carácter
    int inchar = Serial.read();
    // Crear un mensaje completo
    buffer += (char) inchar;
    // Si aparece un salto de línea, se guarda lo leído y detiene el "Evento"
    if (inchar == '\n') {
      message_from_serial = buffer;
      new_message_from_serial = true;
      buffer = "";
      break;
    }
  }
}

