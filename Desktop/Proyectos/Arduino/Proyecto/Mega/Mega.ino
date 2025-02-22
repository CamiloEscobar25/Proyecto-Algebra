#include <Arduino.h>

// Motor 1 – Izquierdo trasero
#define M1_PWM 6  
#define IN1    7    
#define IN2    8    

// Motor 2 – Derecho trasero
#define M2_PWM 5  
#define IN3    4    
#define IN4    10   

// Motor 3 – Derecho delantero
#define M3_PWM 11 
#define IN5    12   
#define IN6    13   

// Motor 4 – Izquierdo delantero
#define M4_PWM 9  
#define IN7    3    
#define IN8    2    

const int SPEED = 100;

int distancia = 0;
int anguloServo = 90;

char turnCommand = 'N';
bool isTurning = false;   // Indica si el vehículo está girando
unsigned long turnStartTime = 0;  // Tiempo de inicio del giro
const unsigned long turnDuration = 1000;  // Duración del giro en milisegundos


const int InputNodes = 3;   // [BIAS, entrada1, entrada2]
const int HiddenNodes = 4;
const int OutputNodes = 4;
double Hidden[HiddenNodes];
double Output[OutputNodes];

float HiddenWeights[3][4] = {
  {1.8991509504079183, -0.4769472541445052, -0.6483690220539764, -0.38609165249078925},
  {-0.2818610915467527, 4.040695699457223, 3.2291858058243843, -2.894301104732614},
  {0.3340650864625773, -1.4016114422346901, 1.3580053902963762, -0.981415976256285}
};

float OutputWeights[4][4] = {
  {1.136072297461121, 1.54602394937381, 1.6194612259569254, 1.8819066696635067},
  {-1.546966506764457, 1.3951930739494225, 0.19393826092602756, 0.30992504138547006},
  {-0.7755982417649826, 0.9390808625728915, 2.0862510744685485, -1.1229484266101883},
  {-1.2357090352280826, 0.8583930286034466, 0.724702079881947, 0.9762852709700459}
};

void ejecutarMotores(int out1, int out2, int out3, int out4, int carSpeed);
void recibirDatosESP();
void conducir();
void InputToOutput(double In1, double In2, double In3);
void girarIzquierda();
void girarDerecha();

void setup() {
  Serial.begin(9600);    // Inicializar comunicación serial para depuración
  Serial1.begin(9600);   // Inicializar comunicación con el ESP8266
  
  // Configurar pines de motores como salidas
  pinMode(M1_PWM, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(M2_PWM, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(M3_PWM, OUTPUT); pinMode(IN5, OUTPUT); pinMode(IN6, OUTPUT);
  pinMode(M4_PWM, OUTPUT); pinMode(IN7, OUTPUT); pinMode(IN8, OUTPUT);
}

void loop() {
  recibirDatosESP();  // Recibir datos del ESP8266
  
  // Si se ha recibido un comando de giro, se fuerza la maniobra
  if (turnCommand == 'R' && !isTurning) {
    isTurning = true;
    turnStartTime = millis();  // Registrar el tiempo de inicio del giro
    girarDerecha();
  }
  else if (turnCommand == 'L' && !isTurning) {
    isTurning = true;
    turnStartTime = millis();  // Registrar el tiempo de inicio del giro
    girarIzquierda();
  }
  else if (isTurning) {
    if (millis() - turnStartTime >= turnDuration) {
      isTurning = false;  // Finalizar el giro
      turnCommand = 'N';  // Resetear el comando de giro
    }
  }
  else {
    // Si no hay comando de giro forzado, se usa la red neuronal
    conducir();
  }
}


void recibirDatosESP() {
  if (Serial1.available()) {
    String data = Serial1.readStringUntil('\n');
    data.trim(); 
    Serial.print(data);
    Serial.println("'");

    if (data.indexOf("Turn") != -1) {
      if (data.indexOf("Right") != -1) {
        turnCommand = 'R';
        Serial.println("Comando de giro: DERECHA");
      } else if (data.indexOf("Left") != -1) {
        turnCommand = 'L';
        Serial.println("Comando de giro: IZQUIERDA");
      }
    }

    else if (data.indexOf("Distance:") != -1 && data.indexOf("Servo:") != -1) {
      turnCommand = 'N'; // Resetea el comando si se reciben datos numéricos
      int idxDist = data.indexOf("Distance:");
      int idxServo = data.indexOf("Servo:");
      if (idxDist != -1 && idxServo != -1) {
        idxDist += strlen("Distance:");
        int commaIndex = data.indexOf(',', idxDist);
        if (commaIndex == -1) commaIndex = idxServo;
        String strDist = data.substring(idxDist, commaIndex);
        distancia = strDist.toInt();
        
        idxServo += strlen("Servo:");
        String strServo = data.substring(idxServo);
        anguloServo = strServo.toInt();
        
        if (anguloServo < 0 || anguloServo > 180)
          anguloServo = 90;

      }
    }
  }
}

void conducir() {
  double TestInput[3];
  double entrada1 = 0, entrada2 = 0;
  
  // Escalar la distancia al rango [1, -1]
  double distanceVal = (double)distancia;
  if (distanceVal < 0) distanceVal = 0;
  if (distanceVal > 50.0) distanceVal = 50.0;
  entrada1 = ((-2.0 / 50.0) * distanceVal) + 1.0;
  
  // Mapear el ángulo (de 30 a 150) al rango [-1, 1]
  double mappedAngle = map(anguloServo, 30, 150, -100, 100);
  entrada2 = ((double)constrain(mappedAngle, -100, 100)) / 100.0;
  
  // Vector de entrada: [1.0 (bias), entrada1, entrada2]
  TestInput[0] = 1.0;
  TestInput[1] = entrada1;
  TestInput[2] = entrada2;
  
  // Propagar la entrada por la red neuronal
  InputToOutput(TestInput[0], TestInput[1], TestInput[2]);
  
  // Convertir las salidas en valores binarios (0 o 1)
  int out1 = round(fabs(Output[0]));
  int out2 = round(fabs(Output[1]));
  int out3 = round(fabs(Output[2]));
  int out4 = round(fabs(Output[3]));
  
  // Ejecutar los motores con las salidas de la red neuronal
  ejecutarMotores(out1, out2, out3, out4, SPEED);
}

void ejecutarMotores(int out1, int out2, int out3, int out4, int carSpeed) {
  // Motor 1: Izquierdo trasero (se mantiene)
  analogWrite(M1_PWM, carSpeed);
  if (out1 == 1) {
    digitalWrite(IN1, HIGH);  // Adelante
    digitalWrite(IN2, LOW);
  } else {
    digitalWrite(IN1, LOW);   // Atrás
    digitalWrite(IN2, HIGH);
  }
  
  // Motor 2: Derecho trasero (se mantiene)
  analogWrite(M2_PWM, carSpeed);
  if (out2 == 1) {
    digitalWrite(IN3, LOW);  // Adelante
    digitalWrite(IN4, HIGH);
  } else {
    digitalWrite(IN3, HIGH);   // Atrás
    digitalWrite(IN4, LOW);
  }
  
  // Motor 3: Derecho delantero (se mantiene)
  analogWrite(M3_PWM, carSpeed);
  if (out3 == 1) {
    digitalWrite(IN5, HIGH);  // Adelante
    digitalWrite(IN6, LOW);
  } else {
    digitalWrite(IN5, LOW);   // Atrás
    digitalWrite(IN6, HIGH);
  }
  
  // Motor 4: Izquierdo delantero (se mantiene)
  analogWrite(M4_PWM, carSpeed);
  if (out4 == 1) {
    digitalWrite(IN7, LOW);  // Adelante
    digitalWrite(IN8, HIGH);
  } else {
    digitalWrite(IN7, HIGH);   // Atrás
    digitalWrite(IN8, LOW);
  }
}

void girarIzquierda() {
  ejecutarMotores(0, 0, 0, 0, SPEED * 2);
}

void girarDerecha() {
  ejecutarMotores(1, 1, 1, 1, SPEED * 2);
}

void InputToOutput(double In1, double In2, double In3) {
  double TestInput[3] = {In1, In2, In3};
  
  // Calcular activaciones en la capa oculta
  for (int i = 0; i < HiddenNodes; i++) {
    double Accum = 0;
    for (int j = 0; j < InputNodes; j++) {
      Accum += TestInput[j] * HiddenWeights[j][i];
    }
    Hidden[i] = tanh(Accum);  // Función de activación tanh
  }
  
  // Calcular activaciones en la capa de salida
  for (int i = 0; i < OutputNodes; i++) {
    double Accum = 0;
    for (int j = 0; j < HiddenNodes; j++) {
      Accum += Hidden[j] * OutputWeights[j][i];
    }
    Output[i] = tanh(Accum);  // Función de activación tanh
  }
}