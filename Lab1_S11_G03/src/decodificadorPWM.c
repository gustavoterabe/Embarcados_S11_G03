//Projeto desenvolvido pelos alunos da equipe G03
//Bruno Arrielo Chagas
//Gustavo Terabe Moy
//Lucas Andre Walter

#include <stdint.h>
#include <stdbool.h>
// includes da biblioteca driverlib
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/pin_map.h"
#include "utils/uartstdio.h"

#include "system_TM4C1294.h" 

#define PipeLine 2
#define NumberOfCyclesUP 52 //(9*P)+21+(3*P)+7
#define NumberOfCyclesDOWN 51 //(9*P)+21+(3*P)+6

float CalculaFreq(int t_up,int t_down)//em Mega
{
  int numClocks = t_up*NumberOfCyclesUP+t_down*NumberOfCyclesDOWN+2;
  float f = 24000000/numClocks; 
 // f = (f/1000000);
  return f;
}

float CalcPeriodo(float frequencia)
{
  float T;;
  T= (1/frequencia)*1000000;
  return T;
}

void printFT(float frequencia, float periodo)
{      
    UARTprintf("Frequencia: %d [Hz]\n",frequencia);
    UARTprintf("Periodo: %d [ns]",periodo);
    UARTprintf("\n\n");     
    while(UARTTxBytesFree() != 1024){}
}

int t_upant =  -1, t_downant = -1;

extern void UARTStdioIntHandler(void);

void inicializa_portas(void)
{
  //PORT M PINO 0 PARA O PWM
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM); 
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)); 
  
  GPIOPinTypeGPIOInput(GPIO_PORTM_BASE, GPIO_PIN_0); //Setar valor como pino de input
  GPIOPadConfigSet(GPIO_PORTM_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  
  //PORT J PINO 0 PARA INICIAR MEDIÇÕES e PINO 1 PARA INICIAR TESTES
   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); // Habilita GPIO J (push-button SW1 = PJ0, push-button SW2 = PJ1)
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)); // Aguarda final da habilitação
    
  GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1); // push-buttons SW1 e SW2 como entrada
  GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);  
}

void UARTInit(void){
  // Enable the GPIO Peripheral used by the UART.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));

  // Enable UART0
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));

  // Configure GPIO Pins for UART mode.
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  // Initialize the UART for console I/O.
  UARTStdioConfig(0, 9600, SystemCoreClock);
} // UARTInit

void UART0_Handler(void){
  UARTStdioIntHandler();
} // UART0_Handler

int PegaValorPwm(void)
{
  //aux = GPIOPinRead(GPIO_PORTM_BASE, GPIO_PIN_0);  //MEDIÇÃO REAL
  
  //UTILIZAÇÃO DOS BOTÃO A DIREITA PARA TESTES
 if( GPIOPinRead(GPIO_PORTM_BASE, GPIO_PIN_0)== 1){return 0;} //TESTE
  else{return 1;}
  
}

void imprime_valores(int tup, int tdown)
{
  if(tup != 0 && tdown != 0)
  {
    float aux_conv, aux_t, freq,periodo;
    int ts, f, duty;  
    aux_t = tup + tdown;
    aux_conv = (tup/aux_t)*100;
    duty = (int)aux_conv;
    aux_conv = (1/aux_t)*100;
    freq= CalculaFreq(tup,tdown);
    f = (int)freq;      
    periodo = CalcPeriodo(freq);
    ts = (int)periodo;
    UARTprintf("Caso entre 0 e 100\n");
    UARTprintf("Duty Cycle: %d por cento\n",duty);
    UARTprintf("Periodo: %d [us]\n",ts);
    UARTprintf("Frequencia: %d [MHz]\n",f);
    UARTprintf("\n");     
    while(UARTTxBytesFree() != 1024){}//FORÇA O PROGRAMA A ESPERAR TODOS OS DADOS SEREM PASSADOS PELA UART
  }
  
  if(tup == 0 || tdown == 0)
  {
    UARTprintf("Caso Extremo\n");
    if(tup == 0)
    {
      UARTprintf("Duty Cycle: 0 por cento\n");
    }
    else
    {
      UARTprintf("Duty Cycle: 100 por cento\n");
    }
    UARTprintf("Periodo: INDETERMINADO\n");
    UARTprintf("Frequencia: INDETERMINADO\n");
    UARTprintf("\n");
   }
  t_upant = tup;
  t_downant = tdown;
}

void main(void)
{
  //Inicializa as variaveis
  int v;
  int aux = 0;
  int t_up = 0,t_down = 0;
  int cp = 0; //VARIAVEL PARA FAZER SITUAÇÃO DE 0 E 100% DE DUTY CYCLE
  
  //INICIALIZA PORTAS
  inicializa_portas();
  
  //INICIALIZA UART 
  UARTInit();
  
  //LOOP PARA ESPERAR USUARIO APERTAR BOTAO PARA INICIAR( OLHANDO A PLACA DE FRENTE´É O BOTÃO A ESQUERDA
  while(GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0) == 0x01){}
  
  while(1)
  {
    v = PegaValorPwm();
    if(cp == 10000) //CONDIÇÂO PARA ASSUMIR CASO 0 OU 100% DE DUTY CYCLE
    {
      imprime_valores(t_up, t_down);
      t_up = 0;
      t_down = 0;
      aux = 0;
      cp = 0;
    }
    if(v == 1)
    {
      if(t_down == 0)
      {
        if(t_up == 0)
        {
          aux = 1;
          cp = 0;
        }
        t_up++;
        cp++;
      }
      else
      {
        imprime_valores(t_up, t_down);
       
        t_up = 0;
        t_down = 0;
        aux = 0;
        cp = 0;
      }
    }
    else
    {
      if(aux == 1){t_down++;}
      else{cp++;}
    }
  }  
} // !main