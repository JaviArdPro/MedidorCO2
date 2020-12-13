#include <LiquidCrystal_I2C.h>




//PANTALLA LCD_ Crear el objeto lcd  dirección  0x3F y 16 columnas x 2 filas
// Se conecta a4 y A5, 
LiquidCrystal_I2C lcd(0x3F,16,2);
#define mensaje_lcd(cadena,fila,columna,acero){ if(acero){ lcd.clear();} lcd.setCursor(fila,columna); lcd.print(cadena); }
#define a 5.2735 //factor de escala
#define b -0.3503 //exponente
#define RL 20000 //Resistencia de carga de 20kohm
#define TIEMPOSEG 600 //10 minutos
#define ANALOG_PIN A0
//#define R0 20767.17 //Resistencia constante que se ha obtenido mediante el proceso de calibración
//equivale al valor de la resistencia del sensor cuando se expone a una concentración de 100 ppm amoniaco en aire limpio
float R0 = 20767.17;

/******
 * 
 * SETUP
 */
void setup() {
  // put your setup code here, to run once:

/*//LCD Pantalla
        lcd.init();
        lcd.begin(16,2); 
     //Encender la luz de fondo.
  lcd.backlight();
 
      mensaje_lcd(F("Medicion co2"),0,0,1);
  mensaje_lcd(F("Hola!"),0,1,0);
  mensaje_lcd(F("Calibrando..."),0,1,0);
   mensaje_lcd(F("Co2: "),0,0,1);
 */
  
   Serial.begin(9600);
   Serial.print(F("Resistencia de carga de 20kohm: "));
   
   Serial.println(RL);
    Serial.print(F("Resistencia constante por defecto (hay que calibrar): "));
    Serial.println(R0);
   Serial.print(F("Empezando calibración.. "));
  
  R0 = calibracionR0();
  //delay(4000);
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

  promediolectura(R0);


/*
 mensaje_lcd(promediolectura(R0),5,0,0);
 mensaje_lcd(F("ppm"),13,0,0);
 mensaje_lcd(F("tiempo(h): "),0,1,0);
  mensaje_lcd(millis()/3600000,12,1,0);
 */
 firststep();



}

void firststep(){
    int VALOR,gasval;
  float voltaje;
  VALOR = analogRead(ANALOG_PIN);
  //VALOR is a 10 bit number in the range from 0 to 1023 which represents voltage from 0 to 1 V
  //1023/(value-1)*RL
  voltaje = VALOR * (5.0 / 1023.0);
 // mensaje_lcd(F("ValorA0: "),0,0,1);
 //mensaje_lcd(VALOR,10,0,0);
   Serial.print("ADC: ");
  Serial.print(VALOR);
  Serial.print(" Voltaje: ");
   Serial.print(voltaje);
  gasval= map(VALOR, 0, 1023, 0, 100);
  Serial.print(" GASVAL %:  ");
   Serial.println(gasval);
   delay(2000);
}

double promediolectura(float R0){
  
int VALOR, Valor_Medio; //lectura de salida analógica del sensor MQ135
double SumaValor=0; //suma correspondiente a 5 lectuas
float VOLTAJE; //Conversión de la lectura a un valor de voltaje
double RS; //Resistencia del sensor MQ135 variable con un Rl de 20 Kohmios
double co2; //Concentración de dióxido de carbono calculada con la ecuación obtenida

  for(byte i=0;i<5;i++){
  //If the analog input pin is not connected to anything, the value returned by analogRead() 
  //will fluctuate based on a number of factors (e.g. the values of the other analog inputs, 
  //how close your hand is to the board, etc.).
  VALOR = analogRead(ANALOG_PIN);
  //Serial.print(F("Valor analogico sensor: "));
  //Serial.println(VALOR);
  SumaValor = SumaValor+VALOR;
  delay(1000);
   }

Valor_Medio = SumaValor/5;
Valor_Medio = analogRead(ANALOG_PIN);
  Serial.print(F("Valor medio analógico A0 cada 5 segundos: "));
  Serial.println(Valor_Medio);
VOLTAJE = Valor_Medio * (5.0 /1023.0); //conversión de la lectura en un valor de tensión
Serial.print(F("VOLTAJE: "));
Serial.println(VOLTAJE);


RS = RL*(5.0-VOLTAJE)/VOLTAJE; //Resistencia medida del sensor

// Rs should be between 30Kohm and 200Kohm.
Serial.print(F("RESISTENCIA VARIABLE MEDIDA DEL SENSOR (RS): "));
Serial.println(RS);
Serial.print(F("CALIBRADO DE R0 - Resistencia constante: "));
Serial.println(R0);
co2 = pow((RS/R0)/a,1/b); //calculamos la concentración de los gases con la ecuación obtenida
Serial.print(F("CO2: "));
Serial.println(co2);

  return co2;
}


float calibracionR0(){

int VALOR,ppm_CO2_actual; 
float RSValor_Medio;
float SumaValor=0;
float VOLTAJE;
float RS;
SumaValor=0;
Serial.println(F("PROCESO DE CALIBRACIÓN....segundos restantes:  "));
Serial.println(TIEMPOSEG);
for(int j=0;j<TIEMPOSEG;j++){
   VALOR = analogRead(ANALOG_PIN);
  //SumaValor = SumaValor + VALOR;
  RS = ((float)RL*1023/(float)VALOR) - RL;
  SumaValor=RS+SumaValor;
  
    Serial.print(TIEMPOSEG-j);
    
    Serial.print(" adc: ");
     Serial.print(VALOR);
        Serial.print(" (1023/VALOR)):" );
     Serial.print(1023/VALOR);
     Serial.print(" RL*(1023/VALOR)):" );
     Serial.print(float(RL*(1023/VALOR)));
    Serial.print(" RL: ");
    Serial.print(RL);
    Serial.print(" RS: ");
    Serial.print(RS);
    Serial.print(" Suma RS: ");
    Serial.print(SumaValor);

  Serial.println(" ... ");
  delay(1000);

  /* if(j<tiemposeg-9){
    mensaje_lcd(tiemposeg-j,14,1,0);
  }
   else{
    mensaje_lcd(F("0"),14,1,0);

    mensaje_lcd(tiemposeg-j,15,1,0);
  }*/
}
Serial.print(F("RS VALOR MEDIO: "));
RSValor_Medio = SumaValor/TIEMPOSEG;
Serial.println(RSValor_Medio);

 ppm_CO2_actual = analogRead(ANALOG_PIN);
 R0 = RSValor_Medio/(a*pow(ppm_CO2_actual,b)); //R0 calibrado


Serial.print(F("R0 CALIBRADO: "));
Serial.println(R0);
Serial.println(F("FIN DEL PROCESO DE CALIBRACIÓN R0"));
  return R0;
}
