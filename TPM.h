#include "MKL05Z4.h"

#define MOD_10Hz	65535		// MOD dla czêstotliwoœci 10Hz
#define ZEGAR 655360

void PWM_Init(void);
void InCap_OutComp_Init(void);
float calculateDistance(float echoTime, float unit);