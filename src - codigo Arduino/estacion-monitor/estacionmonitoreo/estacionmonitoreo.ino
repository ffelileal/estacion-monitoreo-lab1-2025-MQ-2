//  PROYECTO: ESTACIÓN DE MONITOREO AMBIENTAL
//  Grupo 4
//  Integrantes: Arana Braian, Leal Felipe, Romero Ivan, Trujillo Juan
#include <LiquidCrystal_I2C.h>  //Libreria display 11602 I2C
#include <Wire.h>               //Complemento para I2C
#include <RTClib.h>             //Libreria del reloj RTC
#include <dht.h>                //Libreria del sensor DHT11
#include <SD.h>                 //Libreria de la SD
#include <SPI.h>                //Complemento de la libreria SD
#include <LowPower.h>           //Libreria modo sleep

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
byte estadoInterrupcion = 0;
bool interrupcion = false;
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

//Cadenas
String fecha = "";
String dia = "";
String mes = "";
String year = "";
String hora = "";
String minutos = "";
String segundos = "";

//Fecha de tendencia
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

RTC_DS3231 rtc;                      //Objeto para el reloj RTC
LiquidCrystal_I2C lcd(0x27, 16, 2);  //Objeto para display LCD
dht DHT;                             //Objeto para sensor DHT
File myFile;                         //Objeto para archivo SD

void setup() {
  // RtcDateTime cdt = RtcDateTime(__DATE__, __TIME__);//Funcion para calibrar hora
  // Rtc.SetDateTime(cdt);
  pinMode(pinLDR, INPUT);
  pinMode(pinDHT, INPUT);
  pinMode(pinInterrupcion, INPUT);
  pinMode(pinHumo, INPUT);
  pinMode(ledError, OUTPUT);
  pinMode(ledOK, OUTPUT);
  rtc.begin();
  lcd.init();
  lcd.backlight();

  Serial.println("Inicializando SD");

  if (!SD.begin(10)) {
    Serial.println("Inicializacion Fallida!");
    digitalWrite(ledError, HIGH);
    lcd.clear();
    lcd.home();
    lcd.print("SD ERROR");
    while (1)
      ;
  }
  digitalWrite(ledOK, HIGH);
  lcd.createChar(0, flechaArriba);  //Se crea los caracteres para las flechas de tendencia
  lcd.createChar(1, flechaAbajo);
  lcd.createChar(2, flechaDerecha);
  Serial.begin(9600);
}

void loop() {

  Serial.flush();
  lecturaLuz();  //Funcion lectura de luz
  delay(100);
  lecturaTempHum();  //Funcion lectura de temperatura y humedad
  delay(100);
  ppm = obtenerPPM(obtenerRs());  //Funcion que retorna el valor en ppm
  delay(100);

  obtenerFechaHora();     //Funcion para obtener la fecha y hora
  guardarSD(false);       //Funcion para guardar los datos en la SD
  imprimirSerial(false);  //Funcion para imprimir por HC05
  delay(100);

  for (int i = 0; i < 74; i++) {  //Ciclo donde el arduino se pone en sleep cada 8 seg durante 10min
    mostrarDatosLCD();//Funcion que muestra los datos en el display
    delay(100);
    estadoInterrupcion = digitalRead(pinInterrupcion);
    if (estadoInterrupcion == HIGH) {
      interrupcion = true;
    }
    if (interrupcion == true) {
      evento();
      delay(100);
      interrupcion = false;
    }
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
}

void mostrarDatosLCD() {
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
      if (lux < 0.0 || lux > 10000.0) {
        lcd.setCursor(0, 1);
        lcd.print("LDR:");
        lcd.print("FR");
        lecturaLuz();
      } else {
        lcd.setCursor(0, 1);
        lcd.print("LDR:");
        lcd.print(lux);
        lcd.print(" LUX ");
        lcd.write(tendenciaLux);
      }
      break;
    case 2:
      lcd.home();
      lcd.clear();
      lcd.print("Humo ambiente:");
      lcd.setCursor(0, 1);
      ppm = obtenerPPM(obtenerRs());
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
}

String convertirEntero(String variable) {
  if (variable.length() == 1) {
    variable = "0" + variable;
  }
  return variable;
}

void evento() {  //Funcion evento que se ejecuta cuando hay una interrupcion
  delay(300);
  Serial.println("EVENTO");
  lecturaLuz();
  lecturaTempHum();
  ppm = obtenerPPM(obtenerRs());

  obtenerFechaHora();
  imprimirSerial(true);
  guardarSD(true);
}

void obtenerFechaHora() {
  DateTime fecha = rtc.now();  //Se obtiene la cadena de la hora actual, y se empieza a separar para convertir en String

  dia = String(fecha.day());
  mes = String(fecha.month());
  year = String(fecha.year());
  hora = String(fecha.hour());
  minutos = String(fecha.minute());
  segundos = String(fecha.second());

  year = year.substring(2);  //Se elimina 2 caracteres del año

  segundos = convertirEntero(segundos);  //Funcion para agregar un 0 a la fecha y hora
  minutos = convertirEntero(minutos);
  hora = convertirEntero(hora);
  mes = convertirEntero(mes);
  dia = convertirEntero(dia);
}

void guardarSD(bool e) {
  fecha = dia + "-" + mes + "-" + year + ".csv";  //Se crea archivo .csv con la fecha actual
  if (SD.exists(fecha)) {                         //Si el archivo existe se abre, sino se crea el encabezado
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
  if (myFile) {  //Imprime todos las mediciones en la SD
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
    if (e == true) {  //Se imprime para determinar el evento
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

void imprimirSerial(bool e) {
  Serial.print(dia);
  Serial.print("/");
  Serial.print(mes);
  Serial.print("/");
  Serial.print(year);
  Serial.print(" ");
  Serial.print(hora);
  Serial.print(":");
  Serial.print(minutos);
  Serial.print(":");
  Serial.print(segundos);
  Serial.print(";");
  Serial.print(temp);
  Serial.print(";");
  Serial.print(hum);
  Serial.print(";");
  Serial.print(lux);
  Serial.print(";");
  Serial.print(ppm);
  if (e == true) {
    Serial.print(";");
    Serial.print("evento");
    Serial.println(";");
  } else {
    Serial.println(";");
  }
}

void lecturaLuz() {
  lecturaLdr = analogRead(pinLDR);                                    //Lee el valor analogico
  voltajeResistencia = (float)lecturaLdr / 1023.0 * 5.0;              //Se realiza calculo para obtener la tension
  voltajeLdr = 5.0 - voltajeResistencia;                              //Se resta para obtner la caida de tesion del LDR
  resistenciaLdr = voltajeLdr / voltajeResistencia * REF_RESISTANCE;  //Se obtiene la resistencia del LDR

  lux = LUX_CALC_SCALAR * pow(resistenciaLdr, LUX_CALC_EXPONENT);  //Se realiza el calculo para obtener la medicion el lux
  tendenciaLux = tendenciaParametros(luxPasado, lux);              //Funcion para mostrar la tendencia
  luxPasado = LUX_CALC_SCALAR * pow(resistenciaLdr, LUX_CALC_EXPONENT);
}

void lecturaTempHum() {
  //Lectura del sensor
  lecturaDHT = DHT.read11(pinDHT);

  //Medicion de temperatura
  temp = DHT.temperature - 2.5;  //Calculo aprox de la medicion de temperatura
  if (temp < 0.00 || temp > 50.0) {
    lecturaTempHum();
  }
  tendenciaTemp = tendenciaParametros(tempPasado, temp);
  tempPasado = DHT.temperature - 2.5;  //Calculo aprox de la medicion de temperatura
  //Medicion de humedad
  hum = DHT.humidity;
  tendenciaHum = tendenciaParametros(humPasado, hum);
  humPasado = DHT.humidity;
}

float obtenerRs() {
  lecturaHumo = analogRead(pinHumo);
  float Vout = (float)lecturaHumo * (AlimentacionArduino / 1024.0);  //Se obtiene el voltaje del sensor
  return (RCarga * AlimentacionArduino / Vout) - RCarga;
}

float obtenerPPM(float RsRoratio) {
  float logRsRo = log10(RsRoratio);                  //Se hace el log en base 10 del retorno de la funcion obtenerRS()
  float ppm = pow(10, (logRsRo - 0.90) / (-0.421));  //Se realiza el calculo para obtener medicion en ppm
  // El MQ-2 no mide bien por debajo de 200 ppm, se considera aire limpio.
  return ppm;
}

byte tendenciaParametros(float pasado, float actual) {
  if (pasado == actual) {  //Compara el valor anterio y el actual para determinar la flecha de tendencia
    return byte(2);
  } else if (pasado < actual) {
    return byte(0);
  } else {
    return byte(1);
  }
}