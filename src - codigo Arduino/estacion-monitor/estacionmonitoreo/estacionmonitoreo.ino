#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <RTClib.h>
#include <dht.h>
#include <SD.h>
#include <SPI.h>
#include <LowPower.h>
//LDR sin luz 35k Ohm
//LDR luz 500 Ohm
//Consumo calculado aprox -> 330mA

// #define REF_RESISTANCE 6050       // measure this for best results
// #define REF_RESISTANCE 9960       // measure this for best results
// #define LUX_CALC_SCALAR 12518931  // from experiment
// #define LUX_CALC_EXPONENT -1.405  // from experiment
// #define LUX_CALC_EXPONENT -1.360  //Probar
//LUX 196

//Constantes
const int REF_RESISTANCE = 9960;
const long LUX_CALC_SCALAR = 12518931;
const float LUX_CALC_EXPONENT = -1.360;
const float RCarga = 5.0;
const float AlimentacionArduino = 5.0;
const float RoValorAire = 22.20;

//Pines
byte pinDHT = 9;
byte pinCorte = 8;
int pinLDR = A0;
int pinHumo = A1;
int ledError = A2;
int ledOK = A3;
byte pinInterrupcion = 3;

//Variables
int lecturaDHT = 0;
int lecturaLdr = 0;
int lecturaHumo = 0;
float luxPasado = 0;
float temp = 0.0;
float tempPasado = 0.0;

float hum = 0.0;
float humPasado = 0.0;

float voltajeResistencia = 0.0;
float voltajeLdr = 0.0;
float resistenciaLdr = 0.0;
float lux = 0.0;
float ppm = 0.0;
byte menu = 0;
byte tendenciaLux;
byte tendenciaTemp;
byte tendenciaHum;

String fecha = "";
String dia = "";
String mes = "";
String year = "";
String hora = "";
String minutos = "";
String segundos = "";

byte flechaArriba[8] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};
byte flechaAbajo[8] = {
  B00100,  //   *
  B00100,  //   *
  B00100,  //   *
  B00100,  //   *
  B00100,  //   *
  B10101,  // * * *
  B01110,  //  ***
  B00100   //   *
};
byte flechaDerecha[8] = {
  B00100,  //   *
  B00010,  //    *
  B00001,  //     *
  B11111,  // *****
  B00001,  //     *
  B00010,  //    *
  B00100,  //   *
  B00000   //
};

//SD modulo CS-10 SCK-13 MOSI-11 MISO-12

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);
dht DHT;
File myFile;

void setup() {
  // RtcDateTime cdt = RtcDateTime(__DATE__, __TIME__);
  // Rtc.SetDateTime(cdt);
  Serial.begin(9600);
  pinMode(pinLDR, INPUT);
  pinMode(pinHumo, INPUT);
  pinMode(ledError, OUTPUT);
  pinMode(pinCorte, OUTPUT);
  pinMode(ledOK, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(pinInterrupcion), evento, RISING);
  rtc.begin();
  lcd.init();
  lcd.backlight();

  Serial.println("Inicializando SD");

  if (!SD.begin(10)) {
    Serial.println("Inicializacion Fallida!");
    digitalWrite(ledError, HIGH);
    while (1)
      ;
  }
  Serial.println("Inicializacion OK");
  digitalWrite(ledOK, HIGH);
  lcd.createChar(0, flechaArriba);
  lcd.createChar(1, flechaAbajo);
  lcd.createChar(2, flechaDerecha);
}

void loop() {

  digitalWrite(pinCorte, HIGH);
  lecturaLuz();
  lecturaTempHum();
  ppm = obtenerPPM(obtenerRs());

  obtenerFechaHora();
  // imprimirSerial();
  guardarSD(false);

  //LCD 16x2
  switch (menu) {
    case 0:
      lcd.clear();
      lcd.home();
      lcd.print("Fecha:");
      lcd.print(dia);
      lcd.print("/");
      lcd.print(mes);
      lcd.print("/");
      lcd.print(year);
      lcd.setCursor(0, 1);
      lcd.print("Hora:");
      lcd.print(hora);
      lcd.print(":");
      lcd.print(minutos);
      break;
    case 1:
      lcd.home();
      lcd.clear();
      lcd.print("T:");
      lcd.print(temp);
      lcd.write(tendenciaLux);
      lcd.setCursor(9, 0);
      lcd.print("H:");
      lcd.print(hum, 0);
      lcd.write(tendenciaHum);
      lcd.setCursor(0, 1);
      lcd.print("LDR:");
      lcd.print(lux);
      lcd.print(" LUX ");
      lcd.write(tendenciaLux);
      break;
    case 2:
      lcd.home();
      lcd.clear();
      lcd.print("Humo ambiente:");
      lcd.setCursor(0, 1);
      if (ppm < 200) {
        lcd.print("Aire limpio");
      } else {
        lcd.print(ppm);
        lcd.print(" ppm");
      }
      break;
    default:
      break;
  }
  if (menu >= 2) {
    menu = 0;
  } else {
    menu++;
  }
  digitalWrite(pinCorte, LOW);
  for (int i = 0; i < 16; i++) {
    LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
  }
}

String convertirEntero(String variable) {
  if (variable.length() == 1) {
    variable = "0" + variable;
  }
  return variable;
}

void evento() {
  delay(300);
  Serial.println("EVENTO");
  lecturaLuz();
  lecturaTempHum();
  ppm = obtenerPPM(obtenerRs());

  obtenerFechaHora();
  imprimirSerial();
  guardarSD(true);
}

void obtenerFechaHora() {
  DateTime fecha = rtc.now();

  dia = String(fecha.day());
  mes = String(fecha.month());
  year = String(fecha.year());
  hora = String(fecha.hour());
  minutos = String(fecha.minute());
  segundos = String(fecha.second());

  year = year.substring(2);

  segundos = convertirEntero(segundos);
  minutos = convertirEntero(minutos);
  hora = convertirEntero(hora);
  mes = convertirEntero(mes);
  dia = convertirEntero(dia);
}

void guardarSD(bool e) {
  fecha = dia + "-" + mes + "-" + year + ".csv";
  if (SD.exists(fecha)) {
    myFile = SD.open(fecha, FILE_WRITE);
  } else {
    myFile = SD.open(fecha, FILE_WRITE);
    myFile.print("timestamp");
    myFile.print(";");
    myFile.print("temp_c");
    myFile.print(";");
    myFile.print("hum");
    myFile.print(";");
    myFile.print("light");
    myFile.print(";");
    myFile.print("CO PPM");
    myFile.print(";");
    myFile.print("event");
    myFile.println(";");
  }
  if (myFile) {
    myFile.print(dia);
    myFile.print("/");
    myFile.print(mes);
    myFile.print("/");
    myFile.print(year);
    myFile.print(" ");
    myFile.print(hora);
    myFile.print(":");
    myFile.print(minutos);
    myFile.print(":");
    myFile.print(segundos);
    myFile.print(";");
    myFile.print(temp);
    myFile.print(";");
    myFile.print(hum);
    myFile.print(";");
    myFile.print(lux);
    myFile.print(";");
    myFile.print(ppm);
    if (e == true) {
      myFile.print(";");
      myFile.print("evento");
      myFile.println(";");
    } else {
      myFile.println(";");
    }
    myFile.close();
  } else {
    Serial.println("Error abriendo el archivo");
    myFile.close();
  }
}

void imprimirSerial() {
  Serial.print("Fecha: ");
  Serial.print(dia);
  Serial.print("/");
  Serial.print(mes);
  Serial.print("/");
  Serial.println(year);
  Serial.print("Hora: ");
  Serial.print(hora);
  Serial.print(":");
  Serial.println(minutos);

  Serial.print("Temperatura= ");
  Serial.print(temp);
  Serial.println("°C");
  Serial.print("Humedad = ");
  Serial.print(hum);
  Serial.println("%");
}

void lecturaLuz() {
  lecturaLdr = analogRead(pinLDR);
  voltajeResistencia = (float)lecturaLdr / 1023 * 5.0;
  voltajeLdr = 5.0 - voltajeResistencia;
  resistenciaLdr = voltajeLdr / voltajeResistencia * REF_RESISTANCE;

  lux = LUX_CALC_SCALAR * pow(resistenciaLdr, LUX_CALC_EXPONENT);
  tendenciaLux = tendenciaParametros(luxPasado, lux);
  luxPasado = LUX_CALC_SCALAR * pow(resistenciaLdr, LUX_CALC_EXPONENT);
}

void lecturaTempHum() {
  lecturaDHT = DHT.read11(pinDHT);
  temp = DHT.temperature - 2.56;
  tendenciaTemp = tendenciaParametros(tempPasado, temp);
  tempPasado = DHT.temperature - 2.56;
  hum = DHT.humidity;
  tendenciaHum = tendenciaParametros(humPasado, hum);
  humPasado = DHT.humidity;
}

float obtenerRs() {
  lecturaHumo = analogRead(pinHumo);
  float Vout = (float)lecturaHumo * (AlimentacionArduino / 1024.0);
  return (RCarga * AlimentacionArduino / Vout) - RCarga;
}

float obtenerPPM(float RsRoratio) {
  float logRsRo = log10(RsRoratio);
  float ppm = pow(10, (logRsRo - 0.90) / (-0.421));
  // El MQ-2 no mide bien por debajo de 200 ppm, se considera aire limpio.
  return ppm;
}

byte tendenciaParametros(float pasado, float actual) {
  if (pasado == actual) {
    return byte(2);
  } else if (pasado < actual) {
    return byte(0);
  } else {
    return byte(1);
  }
}