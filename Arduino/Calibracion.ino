const int sensorACS712 = A0; // Pin analógico del sensor

void setup() {
  Serial.begin(9600);
  Serial.println("Calibrando el sensor ACS712...");
  Serial.println("Asegúrate de NO tener ningún aparato conectado.");
  delay(3000); // Esperar 3 segundos
}

void loop() {
  long suma = 0;
  const int muestras = 200;

  for (int i = 0; i < muestras; i++) {
    suma += analogRead(sensorACS712);
    delay(5);
  }

  int valorReposo = suma / muestras;

  Serial.print("Voltaje de reposo (voltaje cero): ");
  Serial.println(valorReposo); // <- Este valor lo usarás en el código principal

  while (true); // Pausa infinita
}
  