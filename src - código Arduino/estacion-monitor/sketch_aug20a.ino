#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x21, 16, 2); 
const int pote = A0;
const int led = 9;
const int sensorLuz = A2;
const int sensorHumo = A1;
const int boton = 2;
int estadoHumo;
int medicionLuz;
int contadorMenu = 0;
float medicionHumo = 0;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight(); 
  pinMode(sensorLuz,OUTPUT);
  pinMode(sensorHumo,OUTPUT);
  pinMode(boton,INPUT);
}

void clear_second_line() {
    lcd.setCursor(0, 1); // Posiciona el cursor en la segunda línea
    for (int i = 0; i < 16; i++) {
      lcd.print(" "); // Imprime un espacio en blanco para cada posición
    }
}

void clear_first_line() {
    lcd.setCursor(0, 0); // Posiciona el cursor en la segunda línea
    for (int i = 0; i < 16; i++) {
      lcd.print(" "); // Imprime un espacio en blanco para cada posición
    }
}


void loop() {
  
  	medicionLuz = analogRead(sensorLuz);
  	estadoHumo = analogRead(sensorHumo);
    medicionHumo = estadoHumo * (5.0/1023.0);

	int mapeoLuz = map(medicionLuz, 0, 92,1,100);
  	int estadoBoton = digitalRead(boton);
  
  	lcd.home();
  	lcd.print("Estacion");
 	int valor = ((analogRead(pote) * 5.0 / 1024) - 0.5) * 100;  
  	//lcd.print(valor);
  	//lcd.print(" C ");
  	//lcd.print(" H");
  	//lcd.print(medicionHumo);
	//Serial.println(estadoBoton);

  if (valor > 35){
 	digitalWrite(led,HIGH);
  }else{
   	digitalWrite(led,LOW); 
  }
  if(estadoBoton == HIGH){
  	contadorMenu++;
  }else if(contadorMenu > 4){
    contadorMenu = 0;
  }
  switch (contadorMenu){
    case 0:
        clear_second_line();
        lcd.setCursor(0, 1);
        lcd.print("Temperatura: ");
        lcd.print(valor);
        lcd.print(" C");
        break;

    case 1:
        clear_second_line();
        lcd.setCursor(0, 1);
        lcd.print("Humo: ");
        lcd.print(medicionHumo);
        break;

    case 2:
        clear_second_line();
        lcd.setCursor(0, 1);
        lcd.print("Luz ambiente: ");
        lcd.print(mapeoLuz);
        lcd.print(" LUX");
        break;

    case 3:
        clear_second_line();
        lcd.setCursor(0, 1);
        lcd.print("Humedad: ");
        lcd.print("No hay");
        break;
}
  delay(300);
  //clear_second_line();
}
