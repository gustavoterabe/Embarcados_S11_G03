#ifndef Booster_Pack_Micro
#define Booster_Pack_Micro

#include <stdint.h>
#include <stdbool.h>

void DisplayLCD_Init(void);
void Limpa_LCD(void);
void Escreve_LCD(uint32_t valor);
void Escreve_Frase(char *frase1, char*frase2, uint8_t pos1, uint8_t pos2,int limpa);
void Escreve_Number(uint32_t number);
void Imprime_Tecla(uint32_t tecla);

#endif // Booster_Pack_Micro