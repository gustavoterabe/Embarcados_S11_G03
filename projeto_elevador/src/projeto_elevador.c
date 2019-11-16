#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "cmsis_os2.h" // CMSIS-RTOS
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"


osThreadId_t ElevadorE_id, ElevadorC_id, ElevadorD_id, Gerencia_Elevador_id, Thread_main_id, Thread_Uart_id;

volatile int elevadorEestado = 0, elevadorCestado = 0, elevadorDestado = 0, elevadorEandar = 0, elevadorCandar = 0, elevadorDandar = 0; 

void InicializaPortas(void);

const osThreadAttr_t thread_elevadores_attr = {
  .priority = osPriorityNormal4
};

const osThreadAttr_t thread_gerenciador_attr = {
  .priority = osPriorityNormal3
};

const osThreadAttr_t thread_highpri_attr = {
  .priority = osPriorityNormal5
};


void ElevadorE(void *arg)
{
  while(1)
  {
    switch(elevadorEestado)
    {
    case 0:
      //ENVIA SINAL PARAR PARAR
      osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
      break;
      
    case 1:
      //ENVIA SINAL PARA SUBIR
      break;
      
    case 2:
      //ENVIA SINAL PARA DESCER
      break;
      
    }
    osDelay(100);
  }
} // ElevadorE

void ElevadorC(void *arg)
{
  while(1)
  {
    switch(elevadorCestado)
    {
    case 0:
      //ENVIA SINAL PARAR PARAR
      osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
      break;
      
    case 1:
      //ENVIA SINAL PARA SUBIR
      break;
      
    case 2:
      //ENVIA SINAL PARA DESCER
      break;
      
    }
    osDelay(100);
  }
} // ElevadorC

void ElevadorD(void *arg)
{
  while(1)
  {
    switch(elevadorDestado)
    {
    case 0:
      //ENVIA SINAL PARAR PARAR
      osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
      break;
      
    case 1:
      //ENVIA SINAL PARA SUBIR
      break;
      
    case 2:
      //ENVIA SINAL PARA DESCER
      break;
      
    }
    osDelay(100);
  }
} // ElevadorD

void Gerencia_Elevador(void *arg)
{
  while(1)
  {
    if (elevadorEestado != 0)
    {
      //SINAL PARA DESBLOQUEAR THREAD ELEVADOR ESQUERDO
      osThreadFlagsSet(ElevadorE_id, 0x0001);
    }
    if (elevadorCestado != 0)
    {
      //SINAL PARA DESBLOQUEAR THREAD ELEVADOR CENTRAL
      osThreadFlagsSet(ElevadorC_id, 0x0001);
    }
    if (elevadorDestado != 0)
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
    osDelay(osWaitForever);// COMO POR ENQUANTO NÃO ESTA SENDO USADA ENTÃO ESPERARÁ PARA SEMPRE 
  }
} // Thread_Uart

void Thread_main(void *arg)
{
  //ENVIA TODOS OS ELEVADORES AO TERREO
  ElevadorE_id = osThreadNew(ElevadorE, (void* )'e', &thread_elevadores_attr);
  ElevadorC_id = osThreadNew(ElevadorC, (void* )'c', &thread_elevadores_attr);
  ElevadorD_id = osThreadNew(ElevadorD, (void* )'d', &thread_elevadores_attr);
  Gerencia_Elevador_id = osThreadNew(Gerencia_Elevador, NULL, &thread_gerenciador_attr);
 
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
