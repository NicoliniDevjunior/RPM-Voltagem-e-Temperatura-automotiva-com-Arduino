/* 
 *  https://blogmasterwalkershop.com.br/arduino/como-usar-com-arduino-sensor-de-temperatura-ntc-10k-3950-prova-dagua-do-tipo-sonda
 *  https://www.arduino.cc/reference/pt/language/functions/external-interrupts/attachinterrupt/
 *  https://www.arduino.cc/reference/tr/language/functions/interrupts/nointerrupts/
 *  
*/
#include <LCD.h> //Biblioteca do LCD
#include <LiquidCrystal_I2C.h> // //Biblioteca do modulo I2c LCD
#include <Wire.h>
#include <Thermistor.h>// Biblioteca do sensor de temperatura

// Configuração do display LCD
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

#define pinNTC A0 // faz a leitura Sensor de temperatura

byte pulso_bobina = 2; // Entrada digital, ligar no negativo da Bobina ou saida da central para o tacometro
//int pinoSensor = A0; //pino que está ligado o terminal central do LM35 (porta analogica 0)
int pinoTensao = A1; // PINO ANALÓGICO (A1) EM QUE O SENSOR ESTÁ CONECTADO

// ------ Sensor temperatura---------
//float temperatura = 0;
Thermistor temp1(pinNTC); //VARIÁVEL DO TIPO THERMISTOR, INDICANDO A VARIAVEL pulso_bobina 
float temperatura; // Armazena o valor do sensor de temperatura

//========================================================================================//
int valorLido = 0; //valor lido na entrada analogica
int valorTensao = 0; //valor lido na entrada analogica
float tensao = 0; // Armazena o valor da tensão medida
int cont_pulso = 0; // Armazena a contagem de pulsos da bobina de ignição

// --- Protótipo das Funções ---
long average_volt();   //Retorna a média de 100 leituras de tensão
void volts();          //Calcula a tensão em Volts

//========================================================================================//
// --- Variávei Globais ---
long store;          //Armazena a média de 100 leituras de tensão
long t_Volts = 0;    //Armazena tensão em Volts
//========================================================================================//

void setup() {

  Serial.begin(9600);// Frequencia de comunicação serial Monitor

  lcd.begin(16, 2); // Configuração do tamanho de linhas e colunas do LCD
  
  lcd.print("CPU Bordo"); 
  delay(3000); //Espera 3 segundo 
  lcd.clear(); // limpa valores impressos no LCD

  pinMode(pulso_bobina, INPUT); // Define o pino como entrada
  pinMode(pinoTensao, INPUT);   // Define o pino como entrada
  //pinMode(pinoSensor, INPUT); 
  
  digitalWrite(pulso_bobina, HIGH); // Define pino bobina como nivel ALTO 
  
  cont_pulso = 0; // Define variavel de contagem de pulsos da bobina com 0

  //Configurando a interrupção,FALLING acionar a interrupção quando o estado do pino for de HIGH para LOW apenas
  attachInterrupt(0, PULSO_BOBINA, FALLING); // Nome da função que será chamada e executada quando ocorrer a interrupção 
  noInterrupts(); // Desativa as interrupções. 
  
  delay (1000); // Espera 1 segundo 
}

void loop(){
  
    unsigned long tempo = millis(); // Armazena o tempo atual em Mile segundos
    
    while(tempo +300 > millis()){ // Provoca uma interrupção no programa a cada 300 mile segundos
        interrupts(); // Reativa as interrupções. Interrupções permitem que certas tarefas importantes aconteçam em segundo.
    }
  
    cont_pulso = cont_pulso * 100 ; // Multiplica a contagem de pulsos por 100 e armazena na variavel
    
     /*
    valorLido = analogRead(pinoSensor);
    temperatura = (valorLido * 0.00488); // 5V / 1023 = 0.00488 (precisão do A/D)
    temperatura = temperatura * 100;
     */
    
    /*
    valorTensao = analogRead(pinoTensao);
    tensao = valorTensao * 0.00488; 
    tensao = tensao * 4 ;
    */
    
    temperatura = temp1.getTemp(); //VARIÁVEL DO TIPO INTEIRO QUE RECEBE O VALOR DE TEMPERATURA CALCULADO PELA BIBLIOTECA
    volts(); // Chama função para medir tensão 
    
    lcd.setCursor(0, 0);
    lcd.print("V:");
    lcd.print(t_Volts/100); // Imprime o valor da primeira casa decimal no LCD
    lcd.print("."); 
    lcd.print(t_Volts%10)/100; //Imprime o valor da Segunda casa decimal no LCD
    lcd.print("  ");
    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.print(temperatura);//IMPRIME NO LCD A TEMPERATURA MEDIDA
    lcd.print(" ");
    lcd.setCursor(8, 1);
    lcd.print("RPM:");
    lcd.print(cont_pulso); // Imprime a contagem de pulsos mostrando o RPM no LCD
    lcd.print("   ");
    
    cont_pulso = 0; // Define variavel de contagem de pulsos da bobina com 0
}

void PULSO_BOBINA(){
  cont_pulso ++; // Adiciona mais 1 a contagem
}

//========================================================================================//

// ================== Calcula Tensão em Volts ============================================//
void volts(){                       //Função para cálculo da tensão em Volts
     store = average_volt();        //Recebe o valor médio da tensão retornado pela função average_volts()
     t_Volts = (store*3890)/1023;   //Converte o valor para Volts  
} 

// ------------- Calcula a Média (Filtro Digital) -------------------------
long average_volt(){ //Função que calcula a média de 100 leituras de tensão{                                              //Garante maior precisão na medida
    
     unsigned char i;               //Variável de iterações
     long volt_store = 0;           //Variável local para armazenar o valor da tensão
  
     for(i=0; i<100; i++){                     //Somatório de 100 leituras
        volt_store += analogRead(pinoTensao);  //temp_store = temp_store + analogRead(v_input) (faz o somatório das 100 iterações)
     }

     return(volt_store/100);   //retorna a média das iterações
     
} //end average_volt

/*

   Descrição: Mede a tensão na entrada A0.

   Projetou-se um divisor de tensão para medir tensões de 0 a 40V.

   O display é atualizado a cada estouro do TMR2
 
   
 Cálculos:

   Overflow do Timer2:
   
    T2_OVF = Timer2_cont x prescaler x ciclo de máquina
   
    Ciclo de máquina = 1/Fosc = 1/16E6 = 62,5ns = 62,5E-9s
   
    T2_OVF = (256 - 196) x 1024 x 62,5E-9 = 3,84ms 
   
   
   Divisor de tensão:
   
            R2
   Vo = --------- x Vi
         R2 + R1
         
             R2
   R1 = ( -------- x Vi ) - R2
             Vo
             
             
   Vi = 40V   Vo = 5V   R2(arb) = 27k
   
   
           27000
   R1 = ( ------- x 40 ) - 27000  =  189000 = 189k
             5
             
   Para R1, utiliza-se os valores comerciais de 150k e 39k ligados em série

*/
//========================================================================================//
