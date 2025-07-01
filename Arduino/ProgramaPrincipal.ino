#include <WiFiS3.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiServer.h>
WiFiServer servidor(80);  // Puerto HTTP

// ====== Wi-Fi ======
char ssid[] = "A15 de Percy";      // Cambia esto
char pass[] = "percyto12*";  // Cambia esto

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Sensor y LEDs
const int sensorACS712 = A0;
const int ledVerde = 7;
const int ledAmarillo = 6;
const int ledRojo = 5;

const float V_RMS = 220.0;
const float sensibilidad = 0.185;  // para ACS712 5A
const int voltajeCero = 583;

void setup() {
  servidor.begin();  // Iniciar servidor

  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Conectando WiFi");
  lcd.setCursor(0, 1);
  lcd.print("Espere...");

  WiFi.begin(ssid, pass);
  Serial.print("Intentando conectar a ");
  Serial.println(ssid);

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(1000);
    Serial.print(".");
    lcd.setCursor(intentos % 16, 1);
    lcd.print(".");
    intentos++;
  }

  delay(2000);

  lcd.clear();
  IPAddress ip = WiFi.localIP();
  if (WiFi.status() == WL_CONNECTED && ip != IPAddress(0, 0, 0, 0)) {
    Serial.println("\n✅ Conectado a WiFi");
    Serial.print("📡 IP: ");
    Serial.println(ip);
    lcd.setCursor(0, 0);
    lcd.print("Conectado WiFi");
    lcd.setCursor(0, 1);
    lcd.print(ip);
  } else {
    Serial.println("\n❌ Conectado sin IP");
    lcd.setCursor(0, 0);
    lcd.print("Error DHCP");
    lcd.setCursor(0, 1);
    lcd.print("IP no asignada");
  }

  pinMode(ledVerde, OUTPUT);
  pinMode(ledAmarillo, OUTPUT);
  pinMode(ledRojo, OUTPUT);
}

void loop() {
  // Medición
  const int muestras = 200;
  long suma = 0;
  for (int i = 0; i < muestras; i++) {
    suma += analogRead(sensorACS712);
    delayMicroseconds(50);
  }

  int promedio = suma / muestras;
  float voltaje = promedio * 5.0 / 1024.0;
  float voltajeReferencia = voltajeCero * 5.0 / 1024.0;
  float corriente = (voltaje - voltajeReferencia) / sensibilidad;
  float corrienteRMS = abs(corriente);
  if (corrienteRMS < 0.05) corrienteRMS = 0.0;
  float potencia = V_RMS * corrienteRMS;
  if (potencia < 5.0) potencia = 0.0;

  // LCD
  lcd.setCursor(0, 0);
  lcd.print("Pot:");
  lcd.setCursor(5, 0);
  lcd.print(potencia, 2);
  lcd.print(" W   ");
  lcd.setCursor(0, 1);
  lcd.print("Corr:");
  lcd.setCursor(6, 1);
  lcd.print(corrienteRMS, 2);
  lcd.print(" A   ");

  // LEDs
  if (potencia < 45) {
    digitalWrite(ledVerde, HIGH);
    digitalWrite(ledAmarillo, LOW);
    digitalWrite(ledRojo, LOW);
  } else if (potencia >= 55 && potencia < 500) {
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledAmarillo, HIGH);
    digitalWrite(ledRojo, LOW);
  } else {
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledAmarillo, LOW);
    digitalWrite(ledRojo, HIGH);
  }

  // 📡 Servir datos por WiFi
  WiFiClient cliente = servidor.available();
  if (cliente) {
    String request = cliente.readStringUntil('\r');
    cliente.flush();

    if (request.indexOf("GET /datos") != -1) {
      cliente.println("HTTP/1.1 200 OK");
      cliente.println("Content-Type: text/plain");
      cliente.println("Access-Control-Allow-Origin: *");
      cliente.println("Connection: close");
      cliente.println();
      cliente.print(corrienteRMS, 2);
      cliente.print(",");
      cliente.print(potencia, 2);
    }

    cliente.stop();
  }

  delay(200);
}

