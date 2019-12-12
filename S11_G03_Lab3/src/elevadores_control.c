#include "elevadores_control.h"

#include "system_tm4c1294.h" // CMSIS-Core
#include "cmsis_os2.h" // CMSIS-RTOS
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"

extern char letras[4];

extern elevator elevador[N_ELEVADORES];
extern osThreadId_t Elevador_id[N_ELEVADORES];
extern osThreadId_t Gerencia_Elevador_id[N_ELEVADORES];
extern osMutexId_t function_mutex_id;
extern int oAndar_chamado[N_ELEVADORES], oSentido_chamado[N_ELEVADORES];
extern osMessageQueueId_t fila_uart_tx_id;
extern osTimerId_t timer_das_portas_id[N_ELEVADORES];

char vetor_botao_interno[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
char vetor_botao_externo[] = {'0','1','2','3','4','5','6','7','8','9'};

void Thread_Elevador(void *arg)
{
  int n;
  n = (int)arg;
  set_init_values(&elevador[n]);
  elevador[n].andares_subindo[0] = 1;
  char order[8];
  while(1)
  {   
    switch(elevador[n].estado)
    {
    case Inicial:
      osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
      elevador[n].andares_subindo[0] = 0;
      monta_comando(letras[n], "r", order);
      osMessageQueuePut(fila_uart_tx_id, order ,4 ,osWaitForever);
      elevador[n].estado = Parado;
      break;
    case Parado:
      osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
      if(elevador[n].andar_atual < elevador[n].prox_andar  && elevador[n].prox_andar  != (-1))
      {
        monta_comando(letras[n], "f", order);
        osMessageQueuePut(fila_uart_tx_id, order ,4 ,osWaitForever);
        while(elevador[n].estado_porta != Fechada);
        monta_comando(letras[n], "s", order);
        osMessageQueuePut(fila_uart_tx_id, order ,4 ,osWaitForever);
        elevador[n].estado = Subindo;
      }
      else if(elevador[n].andar_atual > elevador[n].prox_andar && elevador[n].prox_andar  != (-1))
      {
        monta_comando(letras[n], "f", order);
        osMessageQueuePut(fila_uart_tx_id, order ,4 ,osWaitForever);
        while(elevador[n].estado_porta != Fechada);
        monta_comando(letras[n], "d", order);
        osMessageQueuePut(fila_uart_tx_id, order ,4 ,osWaitForever);
        elevador[n].estado_anterior = elevador[n].estado;
        elevador[n].estado = Descendo;
      }
      else
      {
        elevador[n].estado = Parado;
      }
      break;
    case Subindo:
      if ((elevador[n].andar_atual == elevador[n].prox_andar) && elevador[n].prox_andar  != (-1))
      {
        osDelay(120);
        monta_comando(letras[n], "p", order);
        osMessageQueuePut(fila_uart_tx_id, order ,5 ,osWaitForever);
        if(elevador[n].prox_andar != (-1))
        {
          sprintf(order,"%cD%c", letras[n], vetor_botao_interno[elevador[n].prox_andar]);
          osMessageQueuePut(fila_uart_tx_id, order ,4 ,osWaitForever);
        }
        elevador[n].estado_anterior = elevador[n].estado;
        elevador[n].estado = Esperando;
        elevador[n].andares_subindo[elevador[n].prox_andar] = 0;
        elevador[n].andares_descendo[elevador[n].prox_andar] = 0;
      }
      else if (elevador[n].prox_andar == (-1))
      {
        elevador[n].estado = Parado;
      }
      break;
    case Descendo:
      if ((elevador[n].andar_atual == elevador[n].prox_andar) && elevador[n].prox_andar != (-1))
      {
        monta_comando(letras[n], "p", order);
        osMessageQueuePut(fila_uart_tx_id, order ,4 ,osWaitForever);
        if(elevador[n].prox_andar != (-1))
        {
          sprintf(order,"%cD%c", letras[n], vetor_botao_interno[elevador[n].prox_andar]);
          osMessageQueuePut(fila_uart_tx_id, order ,4 ,osWaitForever);
          elevador[n].andares_subindo[elevador[n].prox_andar] = 0;
          elevador[n].andares_descendo[elevador[n].prox_andar] = 0;
        }
        elevador[n].estado_anterior = elevador[n].estado;
        elevador[n].estado = Esperando;
      }
      else if (elevador[n].prox_andar == (-1))
      {
        elevador[n].estado = Parado;
      }
      break;
    case Esperando:
        monta_comando(letras[n], "a", order);
        osMessageQueuePut(fila_uart_tx_id, order ,4 ,osWaitForever);
//        osTimerStart(timer_das_portas_id[n], 5000);
//        osThreadFlagsWait(0x0003, osFlagsWaitAny | 0x0003, osWaitForever);
        osDelay(5000);
        osMutexAcquire(function_mutex_id, osWaitForever); 
        Acha_Proximo_Andar(&elevador[n]);
        osMutexRelease(function_mutex_id);
        if (elevador[n].prox_andar == (-1))
        {
          elevador[n].estado_anterior = elevador[n].estado;
          elevador[n].estado = Parado;
        }
        else if (elevador[n].andar_atual > elevador[n].prox_andar)
        {
          elevador[n].estado_anterior = elevador[n].estado;
          elevador[n].estado = Descendo;
          monta_comando(letras[n], "f", order);
          osMessageQueuePut(fila_uart_tx_id, order ,4 ,osWaitForever);
          while(elevador[n].estado_porta != Fechada);
          monta_comando(letras[n], "d", order);
          osMessageQueuePut(fila_uart_tx_id, order ,4 ,osWaitForever);
        }
        else if (elevador[n].andar_atual < elevador[n].prox_andar)
        {
          elevador[n].estado_anterior = elevador[n].estado;
          elevador[n].estado = Subindo;
          monta_comando(letras[n], "f", order);
          osMessageQueuePut(fila_uart_tx_id, order ,4 ,osWaitForever);
          while(elevador[n].estado_porta != Fechada);
          monta_comando(letras[n], "s", order);
          osMessageQueuePut(fila_uart_tx_id, order ,4 ,osWaitForever);
        }
        else
        {
          elevador[n].estado_anterior = elevador[n].estado;
          elevador[n].estado = Parado;
        }
      break;
    }
    osMutexAcquire(function_mutex_id, osWaitForever); 
    Acha_Proximo_Andar(&elevador[n]);
    osMutexRelease(function_mutex_id);
  }
} // Thread_Elevador



void Gerenciador(void *arg)
{ 
  int n = 0, i;
  char  comand[8];
  int aux_num;
  uint8_t prioridade_elevador;
  i = (int)arg;
  while(1)
  {
    if(osMessageQueueGet(elevador[i].fila_comandos, &elevador[i].comandos, &prioridade_elevador, 1000) == osOK)
    {
      aux_num = (-1);
      n = 0;
      if(elevador[i].comandos[1] == 'A'){elevador[i].estado_porta = Aberta;}
      else if(elevador[i].comandos[1] == 'F'){elevador[i].estado_porta = Fechada;}
      else if(elevador[i].comandos[1] == 'I')
      {
      while((elevador[i].comandos[2] != vetor_botao_interno[n]) && n < N_ANDARES)
        {
          n++;
        }
        if(n<N_ANDARES)
        {
          if(elevador[i].andar_atual != n)
          {
            adiciona_andar(&elevador[i], n);
            sprintf(comand,"%cL%c", letras[i], vetor_botao_interno[n]);
            osMessageQueuePut(fila_uart_tx_id, comand ,4 ,osWaitForever); 
          }
        }
      }
      else if(elevador[i].comandos[1] == 'E')
      {
        sscanf(elevador[i].comandos,"%*c%*c%d%*c", &aux_num);
        if(aux_num>=0 && aux_num<=15)
        {
          if      (elevador[i].comandos[4] == 's' ){elevador[i].andares_subindo[aux_num] = 1;}
          else if (elevador[i].comandos[4] == 'd' ){elevador[i].andares_descendo[aux_num] = 1;}
        }
      }
      else
      {
        sscanf(elevador[i].comandos,"%*c%d", &aux_num);
        if(aux_num<=15 && aux_num >= 0){elevador[i].andar_atual = aux_num;}
      }
    }
    
    if(elevador[i].estado == Parado || elevador[i].estado == Inicial)
    {
      osThreadFlagsSet(Elevador_id[i], 0x0001);
    } 
  }
} // Thread_Gerencia_Elevador

void set_init_values(elevator *aux)
{
  int i;
  aux->estado = Inicial;
  aux->estado_anterior = Parado;
  aux->andar_atual = 0;
  aux->prox_andar = (-1);
  aux->estado_porta = Fechada;
  aux->prox_sentido = Parado;
  for(i=0;i<N_ANDARES;i++)
  {
    aux->andares_subindo[i] = 0;
    aux->andares_descendo[i] = 0;
  }
}

void adiciona_andar(elevator *aux_elevador, int andar)
{
    if (aux_elevador->andar_atual < andar)
    {
      aux_elevador->andares_subindo[andar] = 1;
    }
    else if (aux_elevador->andar_atual > andar)
    {
      aux_elevador->andares_descendo[andar] = 1;
    }
}

void monta_comando(char elevador,char *comando, char* repositorio)
{
  int i = 0;
  repositorio[0] = elevador;
  while(comando[i])
  {
    repositorio[1+i] = comando[i];
    i++;
  }
}

void Acha_Proximo_Andar(elevator *aux)
{
  int i = -1, i2 = N_ANDARES-1;
  if (aux->estado == Inicial)
  {
    i = 0;
  }
  else if(aux->estado == Subindo)
  {
    i = aux->andar_atual;
    if(aux->prox_sentido == Subindo)
    {
      while(aux->andares_subindo[i] != 1 && i <= N_ANDARES){i++;}
    }
    else if(aux->prox_sentido == Descendo)
    {
      while(aux->andares_descendo[i] != 1 <= N_ANDARES){i++;}
    }
    else
    {
      i = (-1);
    }
  }
  else if (aux->estado == Descendo)
  {
    i = aux->andar_atual;
    if(aux->prox_sentido == Subindo)
    {
      while(aux->andares_subindo[i] != 1 && i >= (-1)){i--;}
    }
    else if(aux->prox_sentido == Descendo)
    {
      while(aux->andares_descendo[i] != 1 && i >= (-1)){i--;}
    }
    else
    {
      i = (-1);
    }
  }
  else if(aux->estado == Esperando)
  {
    i = aux->andar_atual;
    if(aux->prox_sentido == Subindo)
    {
      while(aux->andares_subindo[i] != 1 && i <= (N_ANDARES-1)){i++;}
    }
    else if(aux->prox_sentido == Descendo)
    {
      while(aux->andares_descendo[i] != 1 && i >= 0){i--;}
    }
    else
    {
      i = (-1);
    }
  }
  else if (aux->estado == Parado)
  {
    i = 0;
    i2 = N_ANDARES-1;
    while(aux->andares_subindo[i]  != 1 && i <= (N_ANDARES-1)){i++;}
    while(aux->andares_descendo[i2] != 1 && i2 >= 0){i2--;}
    if(i2 > (-1) && i < N_ANDARES)
    {
      if(((aux->andar_atual) - i2) < (i - (aux->andar_atual)))
      {
        i = i2;
        aux->prox_sentido = Descendo;
      }
      else
      {
        aux->prox_sentido = Subindo;
      }
    }
    else if(i2 > (-1) && i >= N_ANDARES)
    {
      i = i2;
      aux->prox_sentido = Descendo;
    }
    else if(i2 <=(-1) && i < N_ANDARES)
    {
      aux->prox_sentido = Subindo;
    }
    else
    {
      i = (-1);
    }
  }
  if(i<0 || i>=N_ANDARES)
  {
    i = (-1);
  }
  aux->prox_andar = i;
}

//
//void timer_portas(void *arg)
//{
//  int aux = (int)arg;
//  osThreadFlagsSet(Elevador_id[aux], 0x0003);
//} // callback

 