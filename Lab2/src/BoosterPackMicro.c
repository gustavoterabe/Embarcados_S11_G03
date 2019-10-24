#include "BoosterPackMicro.h"
// includes da biblioteca driverlib
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/pin_map.h"
#include "string.h"

#include "system_TM4C1294.h"

void Ativa_Rs(void);
void Desativa_Rs(void);
void Pulse_Enable(void);

void Pulse_Enable()
{
    double i = 0;
    for(i = 0; i < 120000*7; i++);
    GPIOPinWrite(GPIO_PORTM_BASE, 0x04, 0x04);
    for(i = 0; i < 120000*7; i++);
    GPIOPinWrite(GPIO_PORTM_BASE, 0x07, 0x03);
}

void Ativa_Rs()
{
    GPIOPinWrite(GPIO_PORTM_BASE, 0x01, 0x01);
}

void Desativa_Rs()
{
    GPIOPinWrite(GPIO_PORTM_BASE, 0x07, 0x06);
}


void DisplayLCD_Init(void)
{
        
  Desativa_Rs();
  GPIOPinWrite(GPIO_PORTK_BASE, 0xFF, 0x38);
  Pulse_Enable();
  GPIOPinWrite(GPIO_PORTK_BASE, 0xFF, 0x0C);
  Pulse_Enable();
  GPIOPinWrite(GPIO_PORTK_BASE, 0xFF, 0x06);
  Pulse_Enable();
  Limpa_LCD();
}

void Limpa_LCD(void)
{
  Desativa_Rs();
  GPIOPinWrite(GPIO_PORTK_BASE, 0xFF, 0x01);
  Pulse_Enable();
  Ativa_Rs();
}

void Escreve_LCD(uint32_t valor)
{
  GPIOPinWrite(GPIO_PORTK_BASE, 0xFF, valor);
  Pulse_Enable();
}

void Escreve_Frase(char *frase1, char*frase2, uint8_t pos1, uint8_t pos2,int limpa)
{
  int i;
  if(limpa == 1)
  {
    Limpa_LCD();
  }
  Desativa_Rs();
  GPIOPinWrite(GPIO_PORTK_BASE, 0xFF, pos1);
  Pulse_Enable();
  Ativa_Rs();
  for(i=0;i<strlen(frase1);i++)
  {
    Escreve_LCD(frase1[i]);
  }
  Desativa_Rs();
  GPIOPinWrite(GPIO_PORTK_BASE, 0xFF, pos2);
  Pulse_Enable();
  Ativa_Rs();
  for(i=0;i<strlen(frase2);i++)
  {
    Escreve_LCD(frase2[i]);
  }
}

void Escreve_Number(uint32_t number)
{
  int aux_num, aux = 0,i = 0, i2 = 0;
  Desativa_Rs();
  GPIOPinWrite(GPIO_PORTK_BASE, 0xFF, 0xCE);
  Pulse_Enable();
  Ativa_Rs();
  aux_num = number;
  while(number != aux || i == -1)
  {
    i2=0;
    while(aux_num > 10*(i+1))
    {
      i++;
    }
    while(aux_num > i*(i2+1))
    {
      i2++;
    }
    aux = aux + i2*i;
    if(aux != -1){Imprime_Tecla(i2);}
    aux_num = aux_num - i2*i;
    i--;
  }	
}

void Imprime_Tecla(uint32_t tecla)
{
  switch(tecla)
  {
    case 1:
      Escreve_LCD('1');
      break;
    case 2:
      Escreve_LCD('2');
      break;
    case 3:
      Escreve_LCD('3');
      break;
    case 4:
      Escreve_LCD('4');
      break;
    case 5:
      Escreve_LCD('5');
      break;
    case 6:
      Escreve_LCD('6');
      break;
    case 7:
      Escreve_LCD('7');
      break;
    case 8:
      Escreve_LCD('8');
      break;
    case 9:
      Escreve_LCD('9');
      break;
    case 0:
      Escreve_LCD('0');
      break;
    case 10:
      Escreve_LCD('1');
      Escreve_LCD('0');
      break;
  }
}