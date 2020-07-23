/*__________________________________________________________________________________
|	Chave Digital Tecnologia Eletronica Ltda. 
|       
|       Blumenau - SC
|       www.chavedigital.com.br
| __________________________________________________________________________________
|
|       This source code was developed by Chave Digital and cannot be copied, in part 
|       or in whole, or used, except when legally licensed by Chave Digital
|       or its distributors.
|
|       Este código é propriedade da Chave Digital e não pode ser copiado, em parte 
|       ou em todo, ou utilizado, exceto quando for legalmente licenciado pela 
|       Chave Digital ou por um de seus distribuidores.
| __________________________________________________________________________________
|
|       Produto            :  Acionador manual APUS
|       Módulo de software :  Firmware do mcu de endereçamento
|       Arquivo            :  main.c
|       Descrição          :  executivo de cliente
| 
|       Autor              :  Marcos Aquino
|       Data criação       :  20/07/2010
|                             12/10/2010 - Portado para o PIC16F716 para a placa face simples.
|
|       Revisões           :  1.0
|
|
| __________________________________________________________________________________
*/

#define	_4MHZ_
//#define	_358MHZ_

#include <htc.h>
/***************************************************************************************
*     Definições do hardware
***************************************************************************************/
#define ADR_D0_DIR              TRISB0
#define ADR_D1_DIR              TRISB1
#define ADR_D2_DIR              TRISB2
#define ADR_D3_DIR              TRISB3
#define ADR_D4_DIR              TRISB4

#define ADR_D0                  RB0
#define ADR_D1                  RB1
#define ADR_D2                  RB2
#define ADR_D3                  RB3
#define ADR_D4                  RB4


#define MODULADOR_DIR           TRISA1
#define MODULADOR_PIN           RA1
#define MODULADOR               RA1

#define ALARME_DIR              TRISA0
#define ALARME_PIN              RA0

#define SET_MODULADOR           MODULADOR = 1;
#define CLR_MODULADOR           MODULADOR = 0;

#define LONG_CICLE              12
#define SHORT_CICLE             3
#define FUNC_MASK               0xA0    

#ifdef _358MHZ_
	#define START_TIME              (22375 >> 1)//(25000 >> 1) // trimado com ajuda do scope, não mude... 100 ms
	#define HIGH_TIME               (8950  >> 1)//(10000 >> 1) //                                          40 ms
	#define LOW_TIME                (2685  >> 1)//(3000  >> 1) //                                          12 ms
	#define STOP_TIME               (15662 >> 1)//(17500 >> 1) //                                          70 ms   
#endif

#ifdef _4MHZ_
	#define START_TIME              (25000 >> 1) // trimado com ajuda do scope, não mude... 100 ms
	#define HIGH_TIME               (10000 >> 1) //                                          40 ms
	#define LOW_TIME                (3000  >> 1) //                                          12 ms
	#define STOP_TIME               (17500 >> 1) //                                          70 ms       
#endif

/***************************************************************************************
*     Variaveis globais
***************************************************************************************/
unsigned char ponteiroBorda=0,endereco=0,flagEnd=0;

unsigned int bitVector[30],sinais=0,temp;

unsigned char moduladorState=1;
/***************************************************************************************
*     Protótipos de função
***************************************************************************************/
void makeBitStream(unsigned char address);

/***************************************************************************************
*     Implementação
***************************************************************************************/

/***************************************************************************************
*     Descrição   :   Função principal
*     Parametros  :   (nenhum)
*     Retorno     :   (nenhum)
***************************************************************************************/
void main(void) {
        
  OPTION = 0b01001111;
  INTCON = 0b11000000;
  PIE1   = 0b00000100;
  
  MODULADOR_PIN = 0x00; //Inicialmente joga os pinos em nível baixo
  ALARME_PIN = 0x00;   

  //---------------------------------------// 
  // Configuração do timer para            //
  // funcionar de acordo com a aplicação   //
  //---------------------------------------//  
  T1CON   = 0b00110001;
  CCPR1L  = CCPR1H =0;
  CCP1CON = 0b00001010;
  
  ADCON1 = 0x07;
  MODULADOR_DIR = 0;
  ALARME_DIR    = 0;
  //---------------------------------------//
  // Lê os jumpers de endereço do sistema  //
  //---------------------------------------// 
  endereco = 0x00;
  if(!ADR_D0) endereco |= 0x01;
  if(!ADR_D1) endereco |= 0x02;
  if(!ADR_D2) endereco |= 0x04;
  if(!ADR_D3) endereco |= 0x08;
  if(!ADR_D4) endereco |= 0x10;

  endereco++;
          
  makeBitStream(endereco);      
  
  //---------------------------------------//
  // Habilitação das interrupções          //
  //---------------------------------------//    
  sinais = 0;
  ponteiroBorda = 0;
  MODULADOR_PIN = 0;
  
  temp = TMR1H<<8 | TMR1L;
  temp+= START_TIME>>1;
  CCPR1H = temp>>8;
  CCPR1L = temp;
  CCP1IF = 0;

  ei();
    
  for(;;){
    #asm
    CLRWDT
    #endasm
  }
}
/***************************************************************************************
*     Descrição   :   Cria o strem de bordas para envio na interrupção
*     Parametros  :   nenhum
*     Retorno     :   nenhum
***************************************************************************************/
void makeBitStream(unsigned char address){
  unsigned char i,paridade=0;

  bitVector[0] = START_TIME;
  bitVector[1] = HIGH_TIME;
  bitVector[2] = LOW_TIME;
  bitVector[3] = HIGH_TIME;
  bitVector[4] = LOW_TIME;
 
  for(i=0;i<8;i++){
    if(address&(0x80>>i)){    
      bitVector[i+5] = HIGH_TIME;
      paridade++;
    }
    else
      bitVector[i+5] = LOW_TIME;
  }
 
  bitVector[13] = (!(paridade&0x01))?(HIGH_TIME):(LOW_TIME);
  bitVector[14] = STOP_TIME;
  bitVector[15] = STOP_TIME;
  bitVector[16] = STOP_TIME;
  bitVector[17] = STOP_TIME;
  bitVector[18] = STOP_TIME;
  bitVector[19] = STOP_TIME;
  bitVector[20] = STOP_TIME;
  bitVector[21] = STOP_TIME;
  bitVector[22] = STOP_TIME;
  bitVector[23] = STOP_TIME;
  bitVector[24] = STOP_TIME;
  bitVector[25] = STOP_TIME;
  bitVector[26] = STOP_TIME;
  bitVector[27] = STOP_TIME;
  bitVector[28] = STOP_TIME;
  bitVector[29] = STOP_TIME;
}
/***************************************************************************************
*     Descrição   :   Faz o tratamento da interrupção do canal zero do timer 1
*     Parametros  :   (nenhum)
*     Retorno     :   (nenhum)
***************************************************************************************/
static void interrupt rtiTimer(void){

  if(!CCP1IF)
    return;

  CCP1IF = 0;
  
  if(ponteiroBorda<16){
     if(moduladorState)
       RA1 = 1;
     else
       RA1 = 0;

     moduladorState ^= 0x01;
  }
  else
    if(ponteiroBorda>15){
      MODULADOR = 0;
      moduladorState = 0x01;
    }
  
  temp = TMR1H<<8 | TMR1L;
  temp += bitVector[ponteiroBorda]; 
  CCPR1H = temp>>8;
  CCPR1L = temp;
  CCP1IF = 0;

  ponteiroBorda++;
      
  if(ponteiroBorda>21){
    sinais++;
    
    if(sinais>3){
      di();
      MODULADOR = 0;  
      ALARME_PIN = 1;  
      #asm                 
      STOP;
      #endasm
      for(;;);
    }
    ponteiroBorda=0;                
  }
}
/***************************************************************************************
*     Fim do arquivo
***************************************************************************************/


