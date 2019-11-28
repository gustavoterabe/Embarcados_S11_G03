//Projeto do Laboratorio 3 desenvolvido pelos alunos da equipe G03
//Bruno Arrielo Chagas
//Gustavo Terabe Moy
//Lucas Andre Walter


#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "cmsis_os2.h" // CMSIS-RTOS
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

#define N_ANDARES 15

enum estados_elevadores {Parado, Subindo, Descendo, Emergencia};

typedef struct
{
  int estado;
  int andar_atual;
  int andares_desejados[N_ANDARES-1];
}elevator;

void InicializaPortas(void);
void UARTInit(void);
extern void UARTStdioIntHandler(void);
void UART0_Handler(void);
void set_init_values(elevator *aux);

osThreadId_t ElevadorE_id, ElevadorC_id, ElevadorD_id, Gerencia_Elevador_id, Thread_main_id, Thread_Uart_id;
 
elevator elevadorE, elevadorC, elevadorD;


int oAndar_chamado, oSentido_chamado;


const osThreadAttr_t thread_elevadores_attr = {
  .priority = osPriorityNormal4
};

const osThreadAttr_t thread_gerenciador_attr = {
  .priority = osPriorityNormal3
};

const osThreadAttr_t thread_highpri_attr = {
  .priority = osPriorityNormal5
};


void Thread_ElevadorE(void *arg)
{
  while(1)
  {
    if (elevadorE.andar_atual == elevadorE.andares_desejados[0])
    {
      elevadorE.estado = Parado;
    }
    else
    {
      if(elevadorE.andar_atual < elevadorE.andares_desejados[0])
      {
        elevadorE.estado = Subindo;
      }
      else
      {
        elevadorE.estado = Descendo;
      }
    }
    switch(elevadorE.estado)
    {
    case Parado:
      UARTprintf("ep\r");
      osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
      break;
      
    case Subindo:
      UARTprintf("es\r");
      break;
      
    case Descendo:
      UARTprintf("ed\r");
      break;
      
    }
    osDelay(100);
  }
} // ElevadorE

void Thread_ElevadorC(void *arg)
{
  while(1)
  {
    if (elevadorC.andar_atual == elevadorC.andares_desejados[0])
    {
      
      elevadorC.estado = Parado;
    }
    else
    {
      if(elevadorC.andar_atual < elevadorC.andares_desejados[0])
      {
        elevadorC.estado = Subindo;
      }
      else
      {
        elevadorC.estado = Descendo;
      }
    }
    switch(elevadorC.estado)
    {
    case Parado:
      UARTprintf("cp\r");
      osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
      break;
      
    case Subindo:
      UARTprintf("cs\r");
      break;
      
    case Descendo:
      UARTprintf("cd\r");
      break;
      
    }
    osDelay(100);
  }
} // ElevadorC

void Thread_ElevadorD(void *arg)
{
  while(1)
  {
    if (elevadorD.andar_atual == elevadorD.andares_desejados[0])
    {
      elevadorD.estado = Parado;
    }
    else
    {
      if(elevadorD.andar_atual < elevadorD.andares_desejados[0])
      {
        elevadorD.estado = Subindo;
      }
      else
      {
        elevadorD.estado = Descendo;
      }
    }
    switch(elevadorD.estado)
    {
    case Parado:
      UARTprintf("dp\r");
      osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
      break;
      
    case Subindo:
      UARTprintf("ds\r");
      break;
      
    case Descendo:
      UARTprintf("dd\r");
      break;
      
    }
    osDelay(100);
  }
} // ElevadorD

void Thread_Gerencia_Elevador(void *arg)
{
  while(1)
  {
    if (oSentido_chamado != Parado)
    {
      if (oSentido_chamado == Subindo)
      {
        
      }
      else 
      {
        
      }
    }
    if (elevadorE.estado != Parado)
    {
      //SINAL PARA DESBLOQUEAR THREAD ELEVADOR ESQUERDO
      osThreadFlagsSet(ElevadorE_id, 0x0001);
    }
    if (elevadorC.estado != Parado)
    {
      //SINAL PARA DESBLOQUEAR THREAD ELEVADOR CENTRAL
      osThreadFlagsSet(ElevadorC_id, 0x0001);
    }
    if (elevadorD.estado != Parado)
    {
      //SINAL PARA DESBLOQUEAR THREAD ELEVADOR DIREITA 
      osThreadFlagsSet(ElevadorD_id, 0x0001);
    }
    osDelay(100);
  }
} // Gerencia_Elevador


void Thread_Uart(void *arg)
{
  while(1)
  {
    osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
    
  }
} // Thread_Uart

void Thread_main(void *arg)
{
  //ENVIA TODOS OS ELEVADORES AO TERREO
  ElevadorE_id = osThreadNew(Thread_ElevadorE, (void* )'e', &thread_elevadores_attr);
  ElevadorC_id = osThreadNew(Thread_ElevadorC, (void* )'c', &thread_elevadores_attr);
  ElevadorD_id = osThreadNew(Thread_ElevadorD, (void* )'d', &thread_elevadores_attr);
  Gerencia_Elevador_id = osThreadNew(Thread_Gerencia_Elevador, NULL, &thread_gerenciador_attr);
  
  set_init_values(&elevadorE);
  set_init_values(&elevadorC);
  set_init_values(&elevadorD);
  
  oAndar_chamado = (-1);
  oSentido_chamado = Parado;
  
  osDelay(osWaitForever);
  
  while(1);
} // Thread_main


void main(void){
  SystemInit();
  LEDInit(LED1);
  
  osKernelInitialize();
  
  Thread_main_id = osThreadNew(Thread_main, NULL, &thread_highpri_attr);
  Thread_Uart_id = osThreadNew(Thread_Uart, NULL, &thread_highpri_attr);
  
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

void UARTInit(void)
{
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

void UART0_Handler(void)
{
  UARTStdioIntHandler();
} // UART0_Handler

void set_init_values(elevator *aux)
{
  int i;
  aux->estado = 0; 
  aux->andar_atual = 0;
  for(i=0;i<N_ANDARES-1;i++)
  {
    aux->andares_desejados[i] = -1;
  }
}
