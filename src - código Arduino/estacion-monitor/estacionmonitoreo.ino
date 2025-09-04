#include <LiquidCrystal_I2C.h>
#include <virtuabotixRTC.h>
#include <dht.h>
#include <SD.h>
#include <SPI.h>

//Pines
int pinDHT = 9;
int pinLDR = A0;

//Variables
int lecturaDHT = 0;
int lecturaLDR = 0;
float temp = 0.0;
float hum = 0.0;
int menu = 0;

String fecha = "";
String dia = "";
String mes = "";
String year = "";
String hora = "";
String minutos = "";
String segundos = "";

virtuabotixRTC myRTC(6, 7, 8);
LiquidCrystal_I2C lcd(0x27, 16, 2);
dht DHT;
File myFile;

void setup() {
  Serial.begin(9600);
  pinMode(pinLDR, INPUT);
  lcd.init();
  lcd.backlight();

  Serial.println("Inicializando SD");

  if (!SD.begin(10)) {
    Serial.println("Inicializacion Fallida!");
    while (1)
      ;
  }
  Serial.println("Inicializacion OK");

  //myRTC.setDS1302Time(00, 48, 17, 3, 3, 9, 2025);

    /*myRTC.setDS1302Time(
    (__TIME__[6]-'0')*10 + (__TIME__[7]-'0'),    // segundos
    (__TIME__[3]-'0')*10 + (__TIME__[4]-'0'),    // minutos
    (__TIME__[0]-'0')*10 + (__TIME__[1]-'0'),    // horas
    3,                                           // día de la semana (lo podés calcular)
    (__DATE__[4]==' ' ? __DATE__[5]-'0' : (__DATE__[4]-'0')*10+(__DATE__[5]-'0')), // día
    __DATE__,                        // mes a número
    atoi(__DATE__+7)                             // año
  );*/
}

void loop() {
  lecturaDHT = DHT.read11(pinDHT);
  lecturaLDR = analogRead(pinLDR);
  temp = DHT.temperature - 2.56;
  hum = DHT.humidity;
  myRTC.updateTime();

  dia = String(myRTC.dayofmonth);
  mes = String(myRTC.month);
  year = String(myRTC.year);
  hora = String(myRTC.hours);
  minutos = String(myRTC.minutes);
  segundos = String(myRTC.seconds);

  segundos = convertirEntero(segundos);
  minutos = convertirEntero(minutos);
  hora = convertirEntero(hora);
  mes = convertirEntero(mes);
  dia = convertirEntero(dia);

  fecha = dia + "-" + mes + "-" + ".csv";//Sacar los 2 primeros caracteres del anio 2025 -> 25
  Serial.println(fecha);
  // myFile = SD.open("f.csv", FILE_WRITE);
  myFile = SD.open(fecha, FILE_WRITE);
  if (myFile) {
    myFile.print(temp);
    myFile.print(" C");
    myFile.print(";");
    myFile.print(hum);
    myFile.print("%");
    myFile.print(";");
    myFile.print(dia);
    myFile.print("/");
    myFile.print(mes);
    myFile.print("/");
    myFile.print(year);
    myFile.print(";");
    myFile.print(hora);
    myFile.print(":");
    myFile.print(minutos);
    myFile.print(":");
    myFile.print(segundos);
    myFile.println(";");
    myFile.close();
  } else {
    Serial.println("Error abriendo el archivo");
  }
  myFile.close();

  //Monitor Serial
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
      lcd.setCursor(9, 0);
      lcd.print("H:");
      lcd.print(hum);
      lcd.setCursor(0, 1);
      lcd.print("LDR:");
      lcd.print(lecturaLDR);
    default:
      break;
  }
  if (menu >= 1) {
    menu = 0;
  } else {
    menu++;
  }
  delay(2000);
}

String convertirEntero(String variable){
  if (variable.length() == 1){
    variable = "0" + variable;
  }
  return variable;
}

void extensionArchivo(String d, String m, String y){
  
}
