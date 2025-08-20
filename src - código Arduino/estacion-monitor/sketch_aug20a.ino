#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x21, 16, 2); 
const int pote = A0;
const int led = A1;
const int sensorLuz = A2;
int medicionLuz;
void setup() {
  lcd.init();
  lcd.backlight(); 
  pinMode(sensorLuz,OUTPUT);

}

void clear_second_line() {
    lcd.setCursor(0, 1); // Posiciona el cursor en la segunda línea
    for (int i = 0; i < 16; i++) {
      lcd.print(" "); // Imprime un espacio en blanco para cada posición
    }
}

void loop() {
  
  medicionLuz = analogRead(A2);
  
  int mapeoLuz = map(medicionLuz, 0, 92,1,100);
    lcd.setCursor(0, 0);
  lcd.print("Medidor de Temperatura");
    lcd.setCursor(0, 1);
  	int valor = ((analogRead(pote) * 5.0 / 1024) - 0.5) * 100;  
  lcd.print(valor);
  lcd.print(" C ");
  lcd.print(mapeoLuz);
  lcd.print(" %");


  if (valor > 35){
    	digitalWrite(led,HIGH);
  }else{
   		digitalWrite(led,LOW); 
  }
  delay(1000);
  clear_second_line();
}