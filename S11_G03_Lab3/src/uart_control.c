#include "uart_control.h"

#include "system_tm4c1294.h" // CMSIS-Core
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

extern elevator elevador[N_ELEVADORES];
extern osThreadId_t Thread_Uart_RX_id, Thread_Uart_TX_id;
extern osMutexId_t global_var_mut_id;
extern void UARTStdioIntHandler(void);

const int CLK_System = 120000000;

extern char letras[4];
extern osMessageQueueId_t fila_uart_tx_id;

void UARTInit(void)
{
  // Enable UART0
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));                                   
                                        
  // Enable the GPIO Peripheral used by the UART.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
                                        
  UARTDisable(UART0_BASE);

  UARTConfigSetExpClk(UART0_BASE, SystemCoreClock, 115200,
                      (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                      UART_CONFIG_PAR_NONE));

  UARTEnable(UART0_BASE);               
  
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  
  //while(!UARTCharsAvail(UART0_BASE)){}
  
  UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);
  UARTIntEnable(UART0_BASE, UART_INT_RX);
  UARTIntRegister(UART0_BASE, UARTRx_Handler);
  //UARTIntRegister();
} // UARTInit

void UARTRx_Handler(void)
{
  UARTIntClear(UART0_BASE,UART_INT_RX);
  osThreadFlagsSet(Thread_Uart_RX_id, 0x0002);
} // UARTRxHandler

void Thread_Uart_RX(void *arg)
{
  int i;
  char msg[8], c;
  
  while(1)
  {
    osThreadFlagsWait(0x0002, osFlagsWaitAny, osWaitForever);
    
    while((UART0_FR_R&UART_FR_RXFE) == 0)//{UARTCharGet(UART0_BASE);}
    {
      i = 0;
      c = UARTCharGet(UART0_BASE);
      while(c != 0xD)
      {
        if(c != '\n')
        {
          msg[i] = c;
          i++;
        }
        c = UARTCharGet(UART0_BASE);
      }
      while(c != 0xA)
      {
        c = UARTCharGet(UART0_BASE);
      }
      if     (msg[0] == 'e'){osMessageQueuePut(elevador[0].fila_comandos, &msg, 4, osWaitForever);}
      else if(msg[0] == 'c'){osMessageQueuePut(elevador[1].fila_comandos, &msg, 4, osWaitForever);}
      else if(msg[0] == 'd'){osMessageQueuePut(elevador[2].fila_comandos, &msg, 4, osWaitForever);} 
      for(i = 0; i<8 ;i++)
      {
        msg[i] = '\0';
      }
    } 
  }
} // Thread_Uart_RX

void Thread_Uart_TX(void *arg)
{
  char msgs[8];
  uint8_t msg_prioridade;
  uint8_t i;
  while(1)
  {
    osMessageQueueGet(fila_uart_tx_id, &msgs, &msg_prioridade, osWaitForever); 
    i = 0;
    while(msgs[i])
    {
      UARTCharPut(UART0_BASE, msgs[i]);
      i++;
    }
    UARTCharPut(UART0_BASE,0xD);
  }
}

