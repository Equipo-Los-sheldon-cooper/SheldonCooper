#include <Wire.h>
#include <MPU6050.h>

MPU6050 MPU;

// 
const int ENA = 2;   
const int IN1 = 3;   
const int IN2 = 4;   


const int ENB = 8;   
const int IN3 = 6;   
const int IN4 = 7;   

#define PWMA ENA
#define AIN1 IN1
#define AIN2 IN2

#define PWMB ENB
#define BIN1 IN3
#define BIN2 IN4


const int TRIG_F = 24;
const int ECHO_F = 22; 

const int TRIG_D = 23; 
const int ECHO_D = 25;

const int TRIG_I = 12; 
const int ECHO_I = 11; 

const unsigned int VELOCIDAD = 180;
const unsigned int UMBRAL_CM = 25;
const unsigned long TIEMPO_MUESTREO = 80;

const unsigned long ESPERA_INICIAL_MS = 3000;
const unsigned long TIEMPO_RETRO_MS = 400;

float ya = 0;
float yainical = 0;

void setup() {
  Wire.begin();
  MPU.initialize();


  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  
  pinMode(TRIG_F, OUTPUT);
  pinMode(ECHO_F, INPUT);

  pinMode(TRIG_D, OUTPUT);
  pinMode(ECHO_D, INPUT);

  pinMode(TRIG_I, OUTPUT);
  pinMode(ECHO_I, INPUT);

  detener();

  delay(ESPERA_INICIAL_MS);

  actualizarya();
  yainical = ya;

  avanzar();
}

void loop() {
  actualizarya();

  unsigned int distF = medirDistanciaCm(TRIG_F, ECHO_F);
  unsigned int distD = medirDistanciaCm(TRIG_D, ECHO_D);
  unsigned int distI = medirDistanciaCm(TRIG_I, ECHO_I);

  if (distF > 0 && distF <= UMBRAL_CM) {
    detener();
    delay(50);

    if (distD > UMBRAL_CM) {
      girarPorGrados(90);
    } else if (distI > UMBRAL_CM) {
      girarPorGrados(-90);
    } else {
      retroceder();
      delay(TIEMPO_RETRO_MS);
    }

    detener();
    delay(80);
    avanzar();
  }

  corregirTrayectoria();
  delay(TIEMPO_MUESTREO);
}

void corregirTrayectoria() {
  float error = ya - yainical;

  if (abs(error) > 0.5) {
    int ajuste = constrain(abs(error) * 10, 15, 50);
    if (error > 0) {
      analogWrite(PWMA, constrain(VELOCIDAD + ajuste, 0, 255));
      analogWrite(PWMB, constrain(VELOCIDAD - ajuste, 0, 255));
    } else {
      analogWrite(PWMA, constrain(VELOCIDAD - ajuste, 0, 255));
      analogWrite(PWMB, constrain(VELOCIDAD + ajuste, 0, 255));
    }
  } else {
    analogWrite(PWMA, VELOCIDAD);
    analogWrite(PWMB, VELOCIDAD);
  }
}

void girarPorGrados(float grados) {
  float inicio = ya;
  unsigned long tiempoInicio = millis();

  if (grados > 0) {
    girarDerecha();
    while ((ya - inicio) < grados && millis() - tiempoInicio < 2000) {
      actualizarya();
      delay(10);
    }
  } else {
    girarIzquierda();
    while ((inicio - ya) < abs(grados) && millis() - tiempoInicio < 2000) {
      actualizarya();
      delay(10);
    }
  }

  detener();
  yainical = ya;
}

void actualizarya() {
  int16_t gx, gy, gz;
  MPU.getRotation(&gx, &gy, &gz);
  
  ya += gz * 0.000060; 
}

void avanzar() {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMA, VELOCIDAD);
  analogWrite(PWMB, VELOCIDAD);
}

void retroceder() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  analogWrite(PWMA, VELOCIDAD);
  analogWrite(PWMB, VELOCIDAD);
}

void girarDerecha() {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  analogWrite(PWMA, VELOCIDAD);
  analogWrite(PWMB, VELOCIDAD);
}

void girarIzquierda() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMA, VELOCIDAD);
  analogWrite(PWMB, VELOCIDAD);
}

void detener() {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
}

unsigned int medirDistanciaCm(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duracion = pulseIn(echoPin, HIGH, 30000UL);
  if (duracion == 0) return 0;

  return duracion / 58;
}
