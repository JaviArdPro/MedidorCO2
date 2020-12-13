#include <LiquidCrystal_I2C.h>




//PANTALLA LCD_ Crear el objeto lcd  dirección  0x3F y 16 columnas x 2 filas
// Se conecta a4 y A5, 
LiquidCrystal_I2C lcd(0x3F,16,2);
#define mensaje_lcd(cadena,fila,columna,acero){ if(acero){ lcd.clear();} lcd.setCursor(fila,columna); lcd.print(cadena); }
#define a 5.2735 //factor de escala
#define b -0.3503 //exponente
#define RL 20000 //Resistencia de carga de 20kohm
//#define R0 20767.17 //Resistencia constante que se ha obtenido mediante el proceso de calibración
//equivale al valor de la resistencia del sensor cuando se expone a una concentración de 100 ppm amoniaco en aire limpio
float R0 = 20767.17;

/******
 * 
 * SETUP
 */
void setup() {
  // put your setup code here, to run once:
//LCD Pantalla
        lcd.init();
        lcd.begin(16,2); 
      //dht.begin();
   Serial.begin(9600);
   Serial.print(F("Resistencia de carga de 20kohm: "));
   
   Serial.println(RL);
    Serial.print(F("Resistencia constante: "));
    Serial.println(R0);
  
  //Encender la luz de fondo.
  lcd.backlight();
  mensaje_lcd(F("Medicion co2"),0,0,1);
  mensaje_lcd(F("Hola!"),0,1,0);
  delay(4000);
  mensaje_lcd(F("Calibrando..."),0,1,0);
  R0 = calibracionR0();
  
  
 
  mensaje_lcd(F("Co2: "),0,0,1);
 /*
    Exponential regression:
  GAS      | a      | b
  CO       | 605.18 | -3.937  
  Alcohol  | 77.255 | -3.18 
  CO2      | 110.47 | -2.862
  Tolueno  | 44.947 | -3.445
  NH4      | 102.2  | -2.473
  Acetona  | 34.668 | -3.369
  */
  
 
}

void loop() {
 mensaje_lcd(promediolectura(R0),5,0,0);
 mensaje_lcd(F("ppm"),13,0,0);
 
}

double promediolectura(float R0){
  
int VALOR, Valor_Medio; //lectura de salida analógica del sensor MQ135
int SumaValor=0; //suma correspondiente a 5 lectuas
float VOLTAJE; //Conversión de la lectura a un valor de voltaje
double RS; //Resistencia del sensor MQ135 variable con un Rl de 20 Kohmios
double co2; //Concentración de dióxido de carbono calculada con la ecuación obtenida



   
 


  for(byte i=0;i<5;i++){
  //If the analog input pin is not connected to anything, the value returned by analogRead() 
  //will fluctuate based on a number of factors (e.g. the values of the other analog inputs, 
  //how close your hand is to the board, etc.).
  VALOR = analogRead(A0);
  //Serial.print(F("Valor analogico sensor: "));
  //Serial.println(VALOR);
  SumaValor = SumaValor+VALOR;
  delay(1000);
   }

Valor_Medio = SumaValor/5;
  Serial.print(F("Valor medio analógico A0: "));
  Serial.println(Valor_Medio);
VOLTAJE = Valor_Medio * (5.0 /1023.0); //conversión de la lectura en un valor de tensión
Serial.print(F("VOLTAJE: "));
Serial.println(VOLTAJE);


RS =(RL/VOLTAJE) * (5.0-VOLTAJE); //Resistencia medida del sensor


Serial.print(F("RESISTENCIA MEDIDA DEL SENSOR: "));
Serial.println(RS);
Serial.print(F("CALIBRADO DE R0 - Resistencia constante: "));
Serial.println(R0);
co2 = pow((RS/R0)/a,1/b); //calculamos la concentración de los gases con la ecuación obtenida
Serial.print(F("CO2: "));
Serial.println(co2);

  return co2;
}


float calibracionR0(){
byte tiemposeg = 60; //mejor 300 seg
int VALOR, Valor_Medio;
int SumaValor=0;
float VOLTAJE;
double RS;
Serial.println(F("PROCESO DE CALIBRACIÓN....segundos restantes:  "));
for(byte j=0;j<tiemposeg;j++){
   VALOR = analogRead(A0);
  //SumaValor = SumaValor + VALOR;
  RS = RL*(1023/VALOR)-RL;
  SumaValor=RS+SumaValor;
  delay(1000);
  Serial.print(tiemposeg-j);
  Serial.print(" ... ");
  if(j<tiemposeg+10){
   mensaje_lcd(tiemposeg-j,14,1,0);
  }
  else{
    mensaje_lcd(tiemposeg-j,15,1,0);
  }
}
Serial.print(F("RS VALOR MEDIO: "));
Valor_Medio = SumaValor/tiemposeg;
Serial.println(Valor_Medio);

 VALOR = analogRead(A0);
 R0 = Valor_Medio/(a*pow(VALOR,b)); //R0 calibrado


Serial.print(F("R0 CALIBRADO: "));
Serial.println(R0);
Serial.println(F("FIN DEL PROCESO DE CALIBRACIÓN R0"));
  return R0;
}
