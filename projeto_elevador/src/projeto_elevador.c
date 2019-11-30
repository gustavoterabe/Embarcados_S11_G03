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

#define N_ANDARES 16
#define N_ELEVADORES  3

enum estados_elevadores {Parado, Subindo, Descendo, Emergencia};

typedef struct
{
  int estado;
  int andar_atual;
  int andares_desejados[N_ANDARES];
}elevator;

void InicializaPortas(void);
void UARTInit(void);
extern void UARTStdioIntHandler(void);
void UART0_Handler(void);
void set_init_values(elevator *aux);
void set_values(int *state, int *floor, int *actual_floor, elevator *aux);

osThreadId_t ElevadorE_id, ElevadorC_id, ElevadorD_id, Gerencia_Elevador_id, Thread_main_id, Thread_Uart_id;
 
elevator elevador[N_ELEVADORES]; //os elevador[0],elevador[1] e elevador[2] correspondem ao elevador da esquerda, central e da direita, respectivamente

int oAndar_chamado[N_ELEVADORES], oSentido_chamado[N_ELEVADORES];  


const osThreadAttr_t thread_elevadores_attr = {
  .priority = osPriorityNormal4
};

const osThreadAttr_t thread_gerenciador_attr = {
  .priority = osPriorityNormal3
};

const osThreadAttr_t thread_highpri_attr = {
  .priority = osPriorityNormal5
};

void Thread_Gerencia_Elevador(void *arg)
{ 
  int meu_andar[N_ELEVADORES], meu_estado[N_ELEVADORES], prox_andar[N_ELEVADORES], 
  andar_chamado[N_ELEVADORES],sentido_chamado[N_ELEVADORES];
  while(1)
  {
    //mutex aqui
    for(int i = 0;i<N_ELEVADORES;i++)
    {
      set_values(&meu_estado[i],&prox_andar[i],&meu_andar[i],&elevador[i]);
      andar_chamado[i] = oAndar_chamado[i];
      sentido_chamado[i] = oSentido_chamado[i];
    }
     //fim de mutex
    if(andar_chamado[0] != (-1) && andar_chamado[1] != (-1) && andar_chamado[2] != (-1))
    {
      for(int j = 0;j<3;j++)
      {
        if (andar_chamado[j] != (-1))
        {
          if((meu_estado[j] == Subindo && sentido_chamado[j] == Subindo && meu_andar[j] < andar_chamado[j]) ||
             (meu_estado[j] == Descendo && sentido_chamado[j] == Descendo && meu_andar[j] > andar_chamado[j]))
          {
             //adiciona proximo andar
          }
          else 
          {
             //adiciona ultimo andar
          }
        } 
      }
    }
    if (elevador[0].estado != Parado)
    {
      //SINAL PARA DESBLOQUEAR THREAD ELEVADOR ESQUERDO
      osThreadFlagsSet(ElevadorE_id, 0x0001);
    }
    if (elevador[1].estado != Parado)
    {
      //SINAL PARA DESBLOQUEAR THREAD ELEVADOR CENTRAL
      osThreadFlagsSet(ElevadorC_id, 0x0001);
    }
    if (elevador[2].estado != Parado)
    {
      //SINAL PARA DESBLOQUEAR THREAD ELEVADOR DIREITA 
      osThreadFlagsSet(ElevadorD_id, 0x0001);
    }
    osDelay(100);
  }
} // Gerencia_Elevador

void Thread_ElevadorE(void *arg)
{
  int meu_estado, meu_andar, prox_andar;
  while(1)
  {
    // MUTEX AQUI
    set_values(&meu_estado,&prox_andar,&meu_andar,&elevador[0]);
    //FIM DO MUTEX 
    if (meu_andar == prox_andar)
    {
      meu_estado = Parado;
    }
    else
    {
      if(meu_andar < prox_andar)
      {
        meu_estado = Subindo;
      }
      else
      {
        meu_estado = Descendo;
      }
    }
    switch(meu_estado)
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
  int meu_estado, meu_andar, prox_andar;
  while(1)
  {
    // MUTEX AQUI
    set_values(&meu_estado,&prox_andar,&meu_andar,&elevador[1]);
    //FIM DO MUTEX 
    if (meu_andar == prox_andar)
    {
      meu_estado = Parado;
    }
    else
    {
      if(meu_andar < prox_andar)
      {
        meu_estado = Subindo;
      }
      else
      {
        meu_estado = Descendo;
      }
    }
    switch(meu_estado)
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
  int meu_estado, meu_andar, prox_andar;
  while(1)
  {
    // MUTEX AQUI
    set_values(&meu_estado,&prox_andar,&meu_andar,&elevador[2]);
    
    if (meu_andar == prox_andar)
    {
      meu_estado = Parado;
    }
    else
    {
      if(meu_andar < prox_andar && meu_estado != Subindo)
      {
        meu_estado = Subindo;
      }
      else if(meu_andar > prox_andar && meu_estado != Descendo)
      {
        meu_estado = Descendo;
      }
    }
    //FIM DO MUTEX 
    switch(meu_estado)
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
  
  for(int i = 0;i<N_ELEVADORES;i++)
  {
    set_init_values(&elevador[i]);
    oAndar_chamado[i] = (-1);
    oSentido_chamado[i] = Parado;
  } 
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
  for(i=0;i<N_ANDARES;i++)
  {
    aux->andares_desejados[i] = -1;
  }
}

void set_values(int *state, int *floor, int *actual_floor, elevator *aux)
{
  *state = aux->estado;
  *actual_floor = aux->andar_atual;
  *floor = aux->andares_desejados[0];
}

void adiciona_andar(elevator *aux,int n_andar,bool lugar)
{
  int i, existe = 0, aux_first,aux_second;
  for(i=0;i<N_ANDARES;i++)
  {
    if(aux->andares_desejados[i] == n_andar){existe = 1;}
  }
  i=0;
  if(existe == 0)
  {
    if(lugar == 0)
    {
      while(aux->andares_desejados[i] < -1){i++;}
      aux->andares_desejados[i] = n_andar;
    }
    else
    {
      for(i=0;i<N_ANDARES-1;i++)
      {
        if(i == 0)
        {
          aux_first = aux->andares_desejados[i+1];
          aux->andares_desejados[i+1] = aux->andares_desejados[i];   
          aux->andares_desejados[i] = n_andar;
        }
        else
        {
          aux_second = aux->andares_desejados[i+1];
          aux->andares_desejados[i+1] = aux_first;
          aux_first = aux_second;
        }
      }
    }
  }
  
  
}
