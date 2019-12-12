//Projeto do Laboratorio 3 desenvolvido pelos alunos da equipe G03
//Bruno Arrielo Chagas
//Gustavo Terabe Moy
//Lucas Andre Walter

//O que falta fazer 
//lista de messagens - IMPORTANTE!!!!!!
//Demais funções da UART - TENDO onde o elevador está ja da pra enganar
//Matar metade do universo para controlar a quantidade de recursos mesmo sabendo que isso nao resolveria o problema pois com o tempo a polução voltaria para a mesma quantidade que a anterior 

#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "uart_control.h" // device drivers
#include "cmsis_os2.h" // CMSIS-RTOS
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "elevadores_control.h"


void InicializaPortas(void);

osThreadId_t Elevador_id[N_ELEVADORES], Gerenciadores_id[N_ELEVADORES], Thread_Uart_RX_id, Thread_Uart_TX_id;
osMutexId_t function_mutex_id;
osMessageQueueId_t  fila_uart_tx_id;
osTimerId_t timer_das_portas_id[N_ELEVADORES];

elevator elevador[N_ELEVADORES]; //os elevador[0],elevador[1] e elevador[2] correspondem ao elevador da esquerda, central e da direita, respectivamente

const osMutexAttr_t Function_Mutex_attr = {"FuctionMutex", osMutexRecursive | osMutexPrioInherit, NULL, 0U};

int oAndar_chamado[N_ELEVADORES], oSentido_chamado[N_ELEVADORES];
char letras[4]={'e','c','d'};

void main(void){
  SystemInit();
  UARTInit();
  osKernelInitialize();
  
  fila_uart_tx_id = osMessageQueueNew(8, 8, NULL);// 8x8 = 64 bits
  elevador[0].fila_comandos = osMessageQueueNew(8, 8, NULL);// 8x8 = 64 bits
  elevador[1].fila_comandos = osMessageQueueNew(8, 8, NULL);// 8x8 = 64 bits
  elevador[2].fila_comandos = osMessageQueueNew(8, 8, NULL);// 8x8 = 64 bits
  
  Thread_Uart_TX_id = osThreadNew(Thread_Uart_TX, NULL, NULL);
  Thread_Uart_RX_id = osThreadNew(Thread_Uart_RX, NULL, NULL);
  
  osThreadSetPriority(Thread_Uart_TX_id, osPriorityHigh);
  osThreadSetPriority(Thread_Uart_RX_id, osPriorityHigh);
  
  Elevador_id[0] = osThreadNew(Thread_Elevador, (void* )0, NULL);
  Elevador_id[1] = osThreadNew(Thread_Elevador, (void* )1, NULL);
  Elevador_id[2] = osThreadNew(Thread_Elevador, (void* )2, NULL);
//  
//  timer_das_portas_id[0] = osTimerNew(timer_portas, osTimerOnce,(void* ) 0, NULL);
//  timer_das_portas_id[1] = osTimerNew(timer_portas, osTimerOnce,(void* ) 1, NULL);
//  timer_das_portas_id[2] = osTimerNew(timer_portas, osTimerOnce,(void* ) 2, NULL);
  
  Gerenciadores_id[0] = osThreadNew(Gerenciador, (void* )0, NULL);
  Gerenciadores_id[1] = osThreadNew(Gerenciador, (void* )1, NULL);
  Gerenciadores_id[2] = osThreadNew(Gerenciador, (void* )2, NULL);
  
  function_mutex_id  = osMutexNew(&Function_Mutex_attr);
  
  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1);
} // main

void InicializaPortas(void)
{
   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); // Habilita GPIO J (push-button SW1 = PJ0, push-button SW2 = PJ1)
   while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)); // Aguarda final da habilitação
    
   GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1); // push-buttons SW1 e SW2 como entrada
   GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}






