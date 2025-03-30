#include "MKL05Z4.h"

#define DIV_CORE	4096	// fclk=4096Hz df=8Hz
#define MASKA_10BIT	0x03FF

void DAC_Init(void);
uint8_t DAC_Load_Trig(uint16_t load);
