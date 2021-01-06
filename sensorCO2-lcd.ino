#include <LiquidCrystal_I2C.h>




//PANTALLA LCD_ Crear el objeto lcd  dirección  0x3F y 16 columnas x 2 filas
// Se conecta a4 y A5, 
LiquidCrystal_I2C lcd(0x3F,16,2);
#define mensaje_lcd(cadena,fila,columna,acero){ if(acero){ lcd.clear();} lcd.setCursor(fila,columna); lcd.print(cadena); }
#define a 5.2735 //factor de escala, obtenido de valores medidos de forma objetiva por otros autores en una regresión exponencial
#define b -0.3503 //exponente, obtenido de valores medidos de forma objetiva por otros autores en una regresión exponencial
//#define a 56.0820 //factor de escala
//#define b -5.9603 //exponente
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
//minimo valor para  Rs/Ro
#define minRsRo  0.358
//maximo valor Rs/Ro
#define maxRsRo  2.428
#define RL 20000 //Resistencia de carga de 10kohm a 47 kohm, en nuestro datasheet es de 20kohm
//Resistance value of MQ-135 is difference to various kinds and various concentration gases. So,When using
//this components, sensitivity adjustment is very necessary. we recommend that you calibrate the detector for
//100ppm NH3 or 50ppm Alcohol concentration in air and use value of Load resistancethat( RL) about 20 KΩ(10KΩ
// to 47 KΩ).
#define TIEMPOSEG 600 //10 minutos para establecer el valor medio de R0 en la calibración
#define ANALOG_PIN A0
//#define R0 20767.17 //Resistencia constante que se ha obtenido mediante el proceso de calibración
//equivale al valor de la resistencia del sensor cuando se expone a una concentración de 100 ppm amoniaco en aire limpio
//float R0 = 20767.17;
//primera pasad float R0 = 261760;
//segunda pasada CALIBRADO DE R0, para 10k rl - Resistencia constante: 134342.73
//tercera pasada para 20k rl, 272295.93
//cuarta pasada pra 20k de rl, ro 306396.93 14-12-2020
//364853.43
//quinta pasada relevante  a las 24 horas -CALIBRADO DE R0 - Resistencia constante: 294741.31 con valor adc 412
//sexta psada con 10 minutos con valor adc 412 - 295056.37


//float R0 = 364853.43; //indicar el último valor obtenido tras la calibración para tu sensor

float ppmprom=0;
//float R0 = 577609.75; //valor tras más de 48 horas encendido el sensor de temperatura con valor co2adc de 400
float R0 = 789873.12; //valor tras recalibrado el 6 de enero 2021

//float R0 = 364853.43;
/******
 * 
 * SETUP
 */
void setup() {
  // put your setup code here, to run once:

//LCD Pantalla
        lcd.init();
        lcd.begin(16,2); 
     //Encender la luz de fondo.
  lcd.backlight();
 
      mensaje_lcd(F("Medicion co2"),0,0,1);
  mensaje_lcd(F("Hola!"),0,1,0);
  delay(2000);
  //mensaje_lcd(F("Calibrando..."),0,1,0);
  delay(2000);
    mensaje_lcd(F("Co2: "),0,0,1);

  
   Serial.begin(9600);
   Serial.print(F("Resistencia de carga: "));
   
   Serial.println(RL);
    Serial.print(F("Resistencia constante por defecto (si no hay que calibrar): "));
    Serial.println(R0);
   Serial.print(F("Empezando calibración.. "));
    mensaje_lcd(F("Inicializando.."),0,1,0);
   delay(30000); //aunque este calibrado y precalentado, se recomienda aguardar 30 segundos después de estar apagado
R0 = calibracionR0(); //Solo se ejecuta al principio una o dos veces, y se establece valor constante en define, comentar si no es necesario
  //delay(4000);


delay(1000);
mensaje_lcd(F("co2:"),0,1,1);
}

void loop() {

  //promediolectura(R0);
  if(millis() % 300 == 0){
    mensaje_lcd(F("Co2: "),0,0,1);
  }
 ppmprom =promediolectura(R0);
 
  if (ppmprom != 0){
    mensaje_lcd(promediolectura(R0),5,0,0);
     mensaje_lcd(F("Co2: "),0,0,0);
  }
 else{ //FUERA DE RANGO
  mensaje_lcd("indef",5,0,1);
  mensaje_lcd(F("Co2: "),0,0,0);
 }
    
    
 mensaje_lcd(F("ppm"),13,0,0);
 mensaje_lcd(F("t(h):"),0,1,0);
  mensaje_lcd(millis()/3600000,5,1,0);
   mensaje_lcd(F("adc:"),8,1,0);
 //mensaje_lcd(mediumvalue_ADC(),12,1,0);
 
 firststep();



}

void firststep(){
    int VALOR,gasval;
  float voltaje;
  VALOR = analogRead(ANALOG_PIN);
  //VALOR is a 10 bit number in the range from 0 to 1023 which represents voltage from 0 to 1 V
  //1023/(value-1)*RL
  voltaje = VALOR * (5.0 / 1023.0);
 // mensaje_lcd(F("ValorA0: "),0,1,0);
 //mensaje_lcd(VALOR,10,0,0);
   Serial.print("ADC: ");
  Serial.print(VALOR);
  mensaje_lcd(VALOR,12,1,0);
  Serial.print(" Voltaje: ");
   Serial.print(voltaje);
  gasval= map(VALOR, 0, 1023, 0, 100);
  Serial.print(" GASVAL %:  ");
   Serial.println(gasval);
   delay(2000);
}

/*
 * 
 * 250-400ppm  Normal background concentration in outdoor ambient air
400-1,000ppm  Concentrations typical of occupied indoor spaces with good air exchange
1,000-2,000ppm  Complaints of drowsiness and poor air.
2,000-5,000 ppm   Headaches, sleepiness and stagnant, stale, stuffy air. Poor concentration, loss of attention, increased heart rate and slight nausea may also be present.
5,000   Workplace exposure limit (as 8-hour TWA) in most jurisdictions.
>40,000 ppm   Exposure may lead to serious oxygen deprivation resulting in permanent brain damage, coma, even death. 
 */

float mediumvalue_ADC(){
int VALOR, SumaValor;
float Valor_Medio;
   for(byte i=0;i<5;i++){
  //If the analog input pin is not connected to anything, the value returned by analogRead() 
  //will fluctuate based on a number of factors (e.g. the values of the other analog inputs, 
  //how close your hand is to the board, etc.).
  VALOR = analogRead(ANALOG_PIN);
  //Serial.print(F("Valor analogico sensor: "));
  //Serial.println(VALOR);
  SumaValor = SumaValor+ VALOR;
  delay(1000);
   }
   Valor_Medio = (float) SumaValor/5.0;
   return Valor_Medio;
}

float promediolectura(float R0){
  
float Valor_Medio; //lectura de salida analógica del sensor MQ135
float SumaValor=0; //suma correspondiente a 5 lectuas
float VOLTAJE; //Conversión de la lectura a un valor de voltaje
float RS; //Resistencia del sensor MQ135 variable con un Rl de 20 Kohmios
float co2=0; //Concentración de dióxido de carbono calculada con la ecuación obtenida

 


Valor_Medio = mediumvalue_ADC();

//Valor_Medio = analogRead(ANALOG_PIN);
  Serial.print(F("Valor medio analógico A0 cada 5 segundos: "));
  Serial.println(Valor_Medio);
VOLTAJE = Valor_Medio * (5.0 /1023.0); //conversión de la lectura en un valor de tensión
Serial.print(F("VOLTAJE: "));
Serial.println(VOLTAJE);
Serial.print(F("RESISTENCIA DE CARGA, RL: "));
Serial.println(RL);

//The datasheet states that Rs should be between 30Kohm and 200Kohm.
RS = RL*(5.0-VOLTAJE)/VOLTAJE; //Resistencia medida del sensor

// Rs should be between 30Kohm and 200Kohm.
Serial.print(F("RESISTENCIA VARIABLE MEDIDA DEL SENSOR (RS): "));
Serial.println(RS);
Serial.print(F("CALIBRADO DE R0 - Resistencia constante: "));
Serial.println(R0);

 float rSrO = (float)RS / (float)R0;
  Serial.print("Rs/Ro: ");
  Serial.println(rSrO);

  if(rSrO < maxRsRo && rSrO > minRsRo) {

co2 = pow((RS/R0)/a,1/b); //calculamos la concentración de los gases con la ecuación obtenida
//250-350 ppm: background (normal) outdoor air level
//350-1,000 ppm: typical level found in occupied spaces with good air exchange
Serial.print(F("CO2: "));
Serial.println(co2);

  }
  else{

  Serial.print("Rs/Ro FUERA DE RANGO");
  }
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
   mensaje_lcd("Calibrando...",1,1,0);
for(int j=0;j<TIEMPOSEG;j++){
   VALOR = analogRead(ANALOG_PIN);
  //SumaValor = SumaValor + VALOR;
  RS = ((float)RL*1023/(float)VALOR) - (float) RL;
  SumaValor=RS+SumaValor;
  
    Serial.print(TIEMPOSEG-j);
    
    Serial.print(" adc: ");
     Serial.print(VALOR);
        Serial.print(" (1023/VALOR)):" );
     Serial.print(1023/VALOR);
     Serial.print(" RL*(1023/VALOR)):" );
     Serial.print(float(RL*(1023/(float)VALOR)));
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
RSValor_Medio = (float) SumaValor/TIEMPOSEG;
Serial.println((float) RSValor_Medio);
delay(1000);
//ppm_CO2_actual = analogRead(ANALOG_PIN);
 ppm_CO2_actual = 400; //este valor se obtiene de https://www.esrl.noaa.gov/gmd/ccgg/trends/#mlo

 R0 = (float) RSValor_Medio/(a*pow(ppm_CO2_actual,b)); //R0 calibrado


Serial.print(F("R0 CALIBRADO: "));
Serial.println(R0);
Serial.println(F("FIN DEL PROCESO DE CALIBRACIÓN R0"));
mensaje_lcd("  Calibrado",1,1,0);
delay(2000);
mensaje_lcd("",1,1,1);
mensaje_lcd(F("Co2: "),0,0,1);
  return R0;
}
