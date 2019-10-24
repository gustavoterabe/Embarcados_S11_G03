//Projeto do Laboratorio 2 desenvolvido pelos alunos da equipe G03
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
#include "driverlib/timer.h"

#define GPIO_PORTL_DATA_R (*((volatile uint32_t *)0x400623FC))
#define N_Amostras 10
const float CLK_System = 120000000;

extern void UARTStdioIntHandler(void);
void UART0_Handler(void);
void Imprime_Valores();
void TimerInit(void);
void PortasInit(void);
void UARTInit(void);
void TIMER0A_Handler(void);
void TIMER0B_Handler(void);

long int tup = 0, tall = 0, tref = 0;
int aux_global = 0;

void main(void)
{
  int amostras = 0,cp = 0;
  //INICIALIZA PORTAS
  PortasInit();
  //INICIALIZA UART 
  UARTInit();
  //INICIALIZAR TIMER
  TimerInit();
  //ESPERA APERTAR SW1 PARA INICIAR O PROGRAMA
  while(GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0) == 0x01){}
  TimerEnable(TIMER0_BASE, TIMER_BOTH);
  while(1)
  {
    cp++;
    if((tup != 0 && tall != 0) || cp > 5000000)
    {
      tup = tup - tref;
      tall = tall - tref;
      if(tup > 0 && tall > 0) 
      {
        while(tall < tup)
        {
          tup = tup - tall;
        }
      }
      if((tup > 0 && tall > 0) || cp > 5000000)
      {
        UARTprintf("\nAmostra %d\n",(amostras+1)); 
        while(UARTTxBytesFree() != 1024){}//FORÇA O PROGRAMA A ESPERAR TODOS OS DADOS SEREM PASSADOS PELA UART     
        Imprime_Valores();
        amostras++;
      }
      if (amostras == N_Amostras )
      {
         while(GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0) == 0x01);
         amostras = 0;    
      }
      tup = 0;
      tall = 0;
      cp = 0;
      aux_global = 0;
      tref = 0;
      TimerIntEnable(TIMER0_BASE,TIMER_CAPA_EVENT);
      TimerIntEnable(TIMER0_BASE,TIMER_CAPB_EVENT);
      TimerEnable(TIMER0_BASE, TIMER_BOTH);
    }
  }
} // !main

void UART0_Handler(void)
{
  UARTStdioIntHandler();
} // UART0_Handler

void Imprime_Valores()
{
  float aux_calc = 0;
  int f, p, duty;
  if(tup > 0 && tall > 0)
  {
    aux_calc = tup*100/tall;
    duty = (int)aux_calc;
    if(duty<100 && duty>0 && CLK_System == 120000000)
    {
      //duty++;
    }
    aux_calc = (float)tall ;
    aux_calc = (aux_calc*1000000000)/CLK_System;
    p = (int)aux_calc;
    aux_calc = CLK_System/(1000*tall);
    f = (int)aux_calc;
    UARTprintf("Duty Cycle = %d por cento\n",duty);
    UARTprintf("Frequencia = %d kHz\n",f);
    UARTprintf("Periodo = %d ns\n",p);
    while(UARTTxBytesFree() != 1024){}//FORÇA O PROGRAMA A ESPERAR TODOS OS DADOS SEREM PASSADOS PELA UART     
  }
  else
  {
    if(tup == 0 || tall == 0)
    {
      if(GPIOPinRead(GPIO_PORTL_BASE, GPIO_PIN_4) == 0x10)
      {
        UARTprintf("Duty Cycle = 100 por cento\n");  
      }
      else
      {
        UARTprintf("Duty Cycle = 0 por cento\n");
      }
      UARTprintf("Frequencia Indeterminada\n");
      UARTprintf("Periodo Indeterminado\n");
      while(UARTTxBytesFree() != 1024){}//FORÇA O PROGRAMA A ESPERAR TODOS OS DADOS SEREM PASSADOS PELA UART     
    }
  }
}// Imprime_Valores

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


void PortasInit(void)
{
//  //PORT K PINOS 7:0 PARA O Display LCD
//  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK); 
//  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)); 
//  
//  GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, 0xFF); //Setar valor como pino de input
//  GPIOPadConfigSet(GPIO_PORTK_BASE, 0xFF, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  
  //PORT J PINO 0 PARA INICIAR MEDIÇÕES e PINO 1 PARA INICIAR TESTES
   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); // Habilita GPIO J (push-button SW1 = PJ0, push-button SW2 = PJ1)
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)); // Aguarda final da habilitação
    
  GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1); // push-buttons SW1 e SW2 como entrada
  GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  
  //PORT L PINO 4 PARA RECEBER O SINAL PWM
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL); 
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL)); 
  
  GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_4); //Setar valor como pino de input
  GPIOPadConfigSet(GPIO_PORTL_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);  
}//PortasInit

void TimerInit(void)
{
    // Enable the Timer0 peripheral 
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)){}
    // Enable the port D
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD); 
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD)); 
    GPIOPinConfigure(GPIO_PD0_T0CCP0);
    GPIOPinConfigure(GPIO_PD1_T0CCP1);
    GPIOPinTypeTimer(GPIO_PORTD_BASE,GPIO_PIN_0 | GPIO_PIN_1);
    
    TimerConfigure(TIMER0_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME_UP | TIMER_CFG_B_CAP_TIME_UP));
    
    TimerControlEvent(TIMER0_BASE, TIMER_A, TIMER_EVENT_NEG_EDGE);
    TimerControlEvent(TIMER0_BASE, TIMER_B, TIMER_EVENT_POS_EDGE);
    
    //TimerLoadSet(TIMER0_BASE, TIMER_BOTH, 0);
    
    TimerIntRegister(TIMER0_BASE, TIMER_A, TIMER0A_Handler);
    TimerIntRegister(TIMER0_BASE, TIMER_B, TIMER0B_Handler);
    TimerIntEnable(TIMER0_BASE,(TIMER_CAPA_EVENT | TIMER_CAPB_EVENT));
    
}//TimerInit


void TIMER0A_Handler(void)
{
  TimerIntClear(TIMER0_BASE,(TIMER_CAPA_EVENT )); 
  if(aux_global == 0)
  {
    TimerIntEnable(TIMER0_BASE,(TIMER_CAPA_EVENT ));
  }
  else
  {
    tup = TimerValueGet(TIMER0_BASE, TIMER_A) ;
  }
}

void TIMER0B_Handler(void)
{
  TimerIntClear(TIMER0_BASE,(TIMER_CAPB_EVENT));
  if(aux_global == 0)
  {
    tref = TimerValueGet(TIMER0_BASE, TIMER_B);
    aux_global = 1;
    TimerIntEnable(TIMER0_BASE,( TIMER_CAPB_EVENT));
  }
  else
  {
    if(aux_global == 1)
    {
      tall = TimerValueGet(TIMER0_BASE, TIMER_B);
      TimerDisable(TIMER0_BASE, TIMER_BOTH);
      aux_global = 2;
    }
  }
}