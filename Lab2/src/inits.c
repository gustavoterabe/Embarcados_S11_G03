#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/pin_map.h"
#include "utils/uartstdio.h"
#include "system_TM4C1294.h" 
#include "driverlib/timer.h"
#include "inits.h"


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
  GPIOPadConfigSet(GPIO_PORTL_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);  
}

void TimerInit(void)
{
    // Enable the Timer0 peripheral 
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)){}
    // Enable the port D
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD); 
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD)); 
    
    TimerConfigure(TIMER0_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME_UP | TIMER_CFG_B_CAP_TIME_UP));
    
    //TimerLoadSet(TIMER0_BASE, TIMER_BOTH, 0);
    
    TimerControlEvent(TIMER0_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);
    TimerControlEvent(TIMER0_BASE, TIMER_B, TIMER_EVENT_NEG_EDGE);
    
    GPIOPinConfigure(GPIO_PD0_T0CCP0);
    GPIOPinConfigure(GPIO_PD1_T0CCP1);
    GPIOPinTypeTimer(GPIO_PORTD_BASE,GPIO_PIN_0 | GPIO_PIN_1);
    
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