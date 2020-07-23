/*__________________________________________________________________________________
|     	Chave Digital Tecnologia Eletronica Ltda. 
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
|
|       Revisões           :  1.0
|
|                             02/11/2011 - Inserida leitura da calibração do 
|                             oscilador.
|
|                             07/08/2013 - Alterado para rodar na placa full SMD
|                             a única alteração realizada foi a posição dos 
|                             jumpers em relação ao peso dos bits na composição
|                             do endereçamento
|
| __________________________________________________________________________________
*/

#include <hidef.h> 
#include "derivative.h"
/***************************************************************************************
*     Definições do hardware
***************************************************************************************/
#define ADR_D0_DIR              PTBDD_PTBDD1
#define ADR_D1_DIR              PTBDD_PTBDD2
#define ADR_D2_DIR              PTBDD_PTBDD3
#define ADR_D3_DIR              PTCDD_PTCDD0
#define ADR_D4_DIR              PTCDD_PTCDD1

#define	ADR_0_PU				PTBPE_PTBPE1
#define	ADR_1_PU				PTBPE_PTBPE2
#define	ADR_2_PU				PTBPE_PTBPE3
#define	ADR_3_PU				PTCPE_PTCPE0
#define	ADR_4_PU				PTCPE_PTCPE1

#define ADR_D0                  PTBD_PTBD1
#define ADR_D1                  PTBD_PTBD2
#define ADR_D2                  PTBD_PTBD3
#define ADR_D3                  PTCD_PTCD0
#define ADR_D4                  PTCD_PTCD1


#define MODULADOR_DIR           PTADD_PTADD0
#define MODULADOR_PIN           PTAD_PTAD0
#define MODULADOR               MODULADOR_PIN

#define ALARME_DIR              PTADD_PTADD1
#define ALARME_PIN              PTAD_PTAD1

#define SET_MODULADOR           MODULADOR = 1;
#define CLR_MODULADOR           MODULADOR = 0;

#define LONG_CICLE              12
#define SHORT_CICLE             3
#define FUNC_MASK               0xA0    

#define MILI_SECOND             125

#define RISING_EDGE             1
#define FOLLING_EDGE            0

#define START_TIME              6200 //158 //7389 // trimado com ajuda do scope, não mude... 100 ms
#define HIGH_TIME               2483//3290 //                                          40//53 ms
#define LOW_TIME                864  //                                          12 ms
#define STOP_TIME               4327 //                                          70 ms

/***************************************************************************************
*     Constantes do programa
***************************************************************************************/
const unsigned int bit[]   = {RISING_EDGE,FOLLING_EDGE,RISING_EDGE,FOLLING_EDGE};
        
/***************************************************************************************
*     Variaveis globais
***************************************************************************************/
unsigned char ponteiroBorda=0,endereco=0,flagEnd=0;

unsigned int bitVector[30],sinais=0;


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
        
  SOPT1 = 0b00100010;   
  
  ICSTRM = NVICSTRM;
  
  ADR_0_PU = 1;				 
  ADR_1_PU = 1;		 
  ADR_2_PU = 1;		 
  ADR_3_PU = 1;		 
  ADR_4_PU = 1;		 
  
  MODULADOR_PIN = 0x00; //Inicialmente joga os pinos em nível baixo
  ALARME_PIN = 0x00;   
  //---------------------------------------// 
  // Configuração do timer para            //
  // funcionar de acordo com a aplicação   //
  //---------------------------------------//  
  TPM1SC   = 0b00001111;  //
  TPM1C0SC = 0b01010000;  // 
  TPM1C0V  = TPM1CNT + 0xFFFF;//   
  
  MODULADOR_DIR = 1;
  ALARME_DIR = 1;
  //---------------------------------------//
  // Lê os jumpers de endereço do sistema  //
  //---------------------------------------// 
  endereco = 0x01;
  if(!ADR_D0) endereco += 1;
  if(!ADR_D1) endereco += 2;
  if(!ADR_D2) endereco += 4;
  if(!ADR_D3) endereco += 8;
  if(!ADR_D4) endereco += 16;
          
  makeBitStream(endereco);      
  
  //---------------------------------------//
  // Habilitação das interrupções          //
  //---------------------------------------//    
  sinais = 0;
  ponteiroBorda = 0;
  MODULADOR_PIN = 0;
  TPM1C0V = TPM1CNT + START_TIME>>1;                
  EnableInterrupts;
    
    
  for(;;);
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
void interrupt 5 rtiTMP0(void){  


  TPM1C0SC_CH0F = 0x00;
  
  if(ponteiroBorda<16)
    MODULADOR  ^= 1;     
  else
    if(ponteiroBorda>15)
      MODULADOR = 0;
  
  TPM1C0V += bitVector[ponteiroBorda];  
  ponteiroBorda++;
      
  if(ponteiroBorda>21){
    sinais++;
    
    if(sinais>3){
      TPM1C0SC = 0x00;   
      MODULADOR = 0;  
      ALARME_PIN = 1;                   
      __asm STOP;
    }
   
    ponteiroBorda=0;                
  }
   
}
/***************************************************************************************
*     Fim do arquivo
***************************************************************************************/


