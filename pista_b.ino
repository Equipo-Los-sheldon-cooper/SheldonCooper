
const int sL = A0, sC = A1, sR = A2;


const int ENA = 2;
const int IN1 = 3;
const int IN2 = 4;   
const int IN3 = 5;   
const int IN4 = 6;   
const int ENB = 7;   

int umbral = 500;
int banda = 50;
int ultimaDir = 0;
int base = 140;
float Kp = 60.0;

void setup() {
  
  pinMode(sL, INPUT);
  pinMode(sC, INPUT);
  pinMode(sR, INPUT);

  
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  Serial.begin(9600);
  calibrarUmbral(200);
}

void loop() {
  int vL = analogRead(sL);
  int vC = analogRead(sC);
  int vR = analogRead(sR);

  bool leftSensor = vL < umbral;
  bool centerSensor = vC < umbral;
  bool rightSensor = vR < umbral;

  if (leftSensor && rightSensor && !centerSensor) {
    seguirLineaPID(vL, vC, vR);
  } else if (!leftSensor && rightSensor && !centerSensor) {
    girarIzqSuave();
  } else if (leftSensor && !rightSensor && !centerSensor) {
    girarDerSuave();
  } else if (!leftSensor && !rightSensor && !centerSensor) {
    seguirLineaPID(vL, vC, vR);
  } else {
    detener();
  }

  delay(50);
}

void seguirLineaPID(int vL, int vC, int vR) {
  int bL = binariza(vL);
  int bC = binariza(vC);
  int bR = binariza(vR);

  int patron = (bL << 2) | (bC << 1) | bR;
  int error = (-1) * bL + 0 * bC + (1) * bR;

  if (patron == 0b000) {
    if (ultimaDir <= 0) girarIzqSuave();
    else girarDerSuave();
    return;
  }

  if (bL) ultimaDir = -1;
  else if (bR) ultimaDir = 1;
  else if (bC) ultimaDir = 0;

  int correccion = (int)(Kp * error);
  int vIzq = constrain(base - correccion, 0, 255);
  int vDer = constrain(base + correccion, 0, 255);

  adelante(vIzq, vDer);
}

int binariza(int v) {
  if (v < umbral - banda) return 1;
  if (v > umbral + banda) return 0;
  return (v < umbral);
}


void adelante(int vLx, int vRx) {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, vLx);

  
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, vRx);
}

void girarIzqSuave() {
  
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);

  
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, base);
}


void girarDerSuave() {
  
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 0);

  
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, base);
}

void detener() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);


  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 0);
}

void calibrarUmbral(int muestras) {
  int minV = 1023, maxV = 0;
  for (int i = 0; i < muestras; i++) {
    minV = min(minV, analogRead(sL));
    minV = min(minV, analogRead(sC));
    minV = min(minV, analogRead(sR));
    maxV = max(maxV, analogRead(sL));
    maxV = max(maxV, analogRead(sC));
    maxV = max(maxV, analogRead(sR));
    delay(5);
  }
  umbral = (minV + maxV) / 2;
}
