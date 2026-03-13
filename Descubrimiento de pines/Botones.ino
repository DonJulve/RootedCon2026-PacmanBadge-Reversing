// Lista de los pines GPIO más seguros y comunes en el ESP32
const int numPines = 16;
int pinesPrueba[numPines] = {0, 2, 4, 12, 13, 14, 15, 25, 26, 27, 32, 33, 34, 35, 36, 39};
int estadoAnterior[numPines];

void setup() {
  Serial.begin(115200);
  Serial.println("Escáner de botones iniciado...");
  Serial.println("Pulsa los botones de tu placa.");

  for (int i = 0; i < numPines; i++) {
    int pin = pinesPrueba[i];
    // Los pines 34, 35, 36 y 39 solo son de entrada y no tienen pull-up interno
    if (pin >= 34) {
      pinMode(pin, INPUT);
    } else {
      pinMode(pin, INPUT_PULLUP); // Activa la resistencia interna
    }
    // Guarda el estado inicial
    estadoAnterior[i] = digitalRead(pin); 
  }
}

void loop() {
  for (int i = 0; i < numPines; i++) {
    int pin = pinesPrueba[i];
    int estadoActual = digitalRead(pin);

    // Si el estado cambia de HIGH a LOW (botón pulsado conectando a GND)
    if (estadoActual == LOW && estadoAnterior[i] == HIGH) {
      Serial.print(">>> ¡BINGO! Has pulsado el botón conectado al GPIO: ");
      Serial.println(pin);
      delay(200); // Pequeña pausa (debounce) para no saturar la pantalla
    }
    
    // Opcional: Si el botón fuera al revés (conecta a VCC)
    else if (estadoActual == HIGH && estadoAnterior[i] == LOW) {
      Serial.print(">>> ¡BINGO! Botón (activo en ALTO) en GPIO: ");
      Serial.println(pin);
      delay(200);
    }

    estadoAnterior[i] = estadoActual;
  }
  delay(10);
}
