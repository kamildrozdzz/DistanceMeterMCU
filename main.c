#include "MKL05Z4.h"
#include "frdm_bsp.h"
#include "lcd1602.h"
#include	"pit.h"
#include "TPM.h"
#include "tsi.h"
#include "DAC.h"
#include "klaw.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>


#define EPSILON 0.0001

//definicja zmiennych odpowiedzialnych za obsługę klawiatury
volatile uint8_t S1_press=0;		// "1" - klawisz został wciśnięty "0" - klawisz "skonsumowany"
volatile uint8_t S2_press=0;	
volatile uint8_t S3_press=0;
volatile uint8_t S4_press=0;

//definicja zmiennych służących do odczytywania odległości
volatile uint8_t wynik_ok=0; 		//Zmienna odpowiedzialny za sygnalizację czy zmienna jest gotowa do pobrania przez pętlę główną
volatile float	wynik;					//Wartość odczytana przez czujnik odległości
volatile float	distance;				//Wartość przekazana przez licznik PIT
volatile uint32_t sum = 0;			//Zmienne służące do prawidłowego odczytania "nagromadzonych" wartości
volatile uint32_t avg= 0;

volatile uint32_t d=0;					//Korekcja czętotliwości
volatile float tick, tick_head;

float freq;
volatile float unit = 1; //domyślna wartość w cm
volatile uint8_t w;
volatile float slider = 30;

volatile uint8_t on_off=255;
volatile uint16_t dac;
volatile int16_t Sinus[1024];
volatile uint16_t faza, mod, df;
volatile uint16_t nyquist;

void SysTick_Handler(void)								// Podprogram obsługi przerwania od SysTick'a
{ 
	dac=(Sinus[faza]/100)*100;		// Przebieg sinusoidalny
	DAC_Load_Trig(dac);
	faza+=mod;															// faza - generator cyfrowej fazy
	faza&=MASKA_10BIT;											// rejestr sterujący przetwornikiem, liczący modulo 1024 (N=10 bitów)
}

void PORTA_IRQHandler(void)	// Podprogram obsługi przerwania od klawiszy S1, S2, S3 i S4
{
	uint32_t buf;
	buf=PORTA->ISFR & (S1_MASK | S2_MASK | S3_MASK | S4_MASK);

	switch(buf)
	{
		case S1_MASK:	DELAY(100) //[in]
									if(!(PTA->PDIR&S1_MASK))		// Minimalizacja drgań zestyków
									{
										DELAY(100)
										if(!(PTA->PDIR&S1_MASK))	// Minimalizacja drgań zestyków (c.d.)
										{
											if(!S1_press)
											{
												unit = 0.3937;
												S1_press=1;
											}
										}
									}
									break;
		case S2_MASK:	DELAY(100) //[cm]
									if(!(PTA->PDIR&S2_MASK))		// Minimalizacja drgań zestyków
									{
										DELAY(100)
										if(!(PTA->PDIR&S2_MASK))	// Minimalizacja drgań zestyków (c.d.)
										{
											if(!S2_press)
											{
												unit = 1;
												S2_press=1;
											}
										}
									}
									break;
		case S3_MASK:	DELAY(100) //[m]
									if(!(PTA->PDIR&S3_MASK))		// Minimalizacja drgań zestyków
									{
										DELAY(100)
										if(!(PTA->PDIR&S3_MASK))	// Minimalizacja drgań zestyków (c.d.)
										{
											if(!S3_press)
											{
												unit = 0.01;
												S3_press=1;
											}
										}
									}
									break;
		case S4_MASK:	DELAY(100) //[m]
									if(!(PTA->PDIR&S4_MASK))		// Minimalizacja drgań zestyków
									{
										DELAY(100)
										if(!(PTA->PDIR&S4_MASK))	// Minimalizacja drgań zestyków (c.d.)
										{
											if(!S4_press)
											{
												unit = 10;
												S4_press=1;
											}
										}
									}
									break;
		default:			break;
	}	
	PORTA->ISFR |=  S1_MASK | S2_MASK | S3_MASK | S4_MASK;	// Kasowanie wszystkich bitów ISF
	NVIC_ClearPendingIRQ(PORTA_IRQn);
}
	
void PIT_IRQHandler()
{
	PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;		// Skasuj flagę żądania przerwania
	if(!wynik_ok)				// Sprawdź, czy wynik skonsumowany przez pętlę główną
	{
		distance = avg/sum; //Obliczenie średniej z odczytanych wartości
		avg=0;
		sum=0;
		wynik_ok = 1;
	}
}

void TPM1_IRQHandler(void)
{
	TPM1->SC = 0;		// TPM1 stop
	if((TPM1->STATUS & TPM_STATUS_TOF_MASK))		// Sprawdzenie, czy impuls nie jest za długi
	{
			TPM1->SC = 0;
			wynik=100000;
			d+=1;						// Korekcja częstotliwości wzorcowej
			if(d==8)
				d=0;
	}
	if(TPM1->STATUS & TPM_STATUS_CH1F_MASK)
	{
			wynik=TPM1->CONTROLS[1].CnV;		// Aktualizacja wyniku
			avg += wynik;										// Sumowanie odczytanych wartości
			sum++;													// inkrementacja zmiennej, która będzie służyć do odczytania średniej z wyników
	}
		
	TPM1->STATUS |= TPM_STATUS_CH1F_MASK;		// Kasowanie flag przerwania
	TPM1->STATUS |= TPM_STATUS_TOF_MASK;
	TPM1->SC = d;														// Odtworzenie dzielnika PS
	TPM1->SC |= TPM_SC_TOIE_MASK;						// Odblokowanie przerwania od TOF
	TPM1->SC |= TPM_SC_CMOD(1);							// TMP1 aktywny
}

int main (void)
{
	df=DIV_CORE/MASKA_10BIT;	// Rozdzielczość generatora delta fs
	nyquist=(DIV_CORE/(2*df))-1;
	
	DAC_Init();		// Inicjalizacja prztwornika C/A
	
	for(faza=0;faza<1024;faza++)
		Sinus[faza]=(sin((double)faza*0.0061359231515)*2047.0); // Ładowanie 1024-ech sztuk, 12-bitowych próbek funkcji sisnus do tablicy
	faza=0;		// Ustawienie wartości początkowych generatora cyfrowej fazy i modulatora fazy
	mod=128;
	freq=mod*df;
	
	NVIC_SetPriority(SysTick_IRQn, 1);
	SysTick_Config(SystemCoreClock/DIV_CORE);	// Start licznika SysTick (generatora DDS)
	
	char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
	volatile uint32_t ps_value[]={1,2,4,8,16,32,64,128};
	tick_head=1000.0/SystemCoreClock;

	//inicjalizacja pozostałych funkcji wykorzystywanych w programie
	Klaw_Init();  //inicjalizacja klawiatury
	Klaw_S2_4_Int();	//włączenie przerwań dla klawiszy S1-S4
	LCD1602_Init();		 // Inicjalizacja wyświetlacza LCD
	LCD1602_Backlight(TRUE);
	LCD1602_ClearAll();
	TSI_Init();				// Inicjalizacja pola dotykowego - Slider
	PIT_Init();							// Inicjalizacja licznika PIT0
	InCap_OutComp_Init(); //Inicjalizacja liczniak TPM1
	PWM_Init();				// Inicjalizacja licznika TPM0

	while(1)
	{
		tick=tick_head*ps_value[d];
		w=TSI_ReadSlider();
		if(w!=0){
			slider = w*unit;
		}
		if(wynik_ok)
		{
			LCD1602_ClearAll();
			if(distance==100000)
			{
				LCD1602_SetCursor(0,0);
				LCD1602_Print(" OVERFLOW");		// Impuls zbyt długi
			}else{
			distance*=tick;
			distance = calculateDistance(distance, unit);
			if (distance < (400.0 * unit) && distance > (2.0 * unit)) {
				LCD1602_SetCursor(0,0);
				if (fabs(unit - 1) < EPSILON) {
							sprintf(display,"Alarm: %.1f cm",slider*unit);
							LCD1602_Print(display);
							sprintf(display, "d= %.1f cm", distance);
            } else if ((fabs(unit - 0.01)) < EPSILON) {
							sprintf(display,"Alarm: %.2f m",slider*unit);
							LCD1602_Print(display);
							sprintf(display, "d= %.2f m", distance);
						} else if (fabs(unit - 10) < EPSILON) {
							sprintf(display,"Alarm: %.1f mm",slider*unit);
							LCD1602_Print(display);
              sprintf(display, "d= %.1f mm", distance);
            } else if (fabs(unit - 0.3937) < EPSILON) {
							sprintf(display,"Alarm: %.1f in",slider*unit);
							LCD1602_Print(display);
               sprintf(display, "d= %.1f in", distance);
            }
            LCD1602_SetCursor(0, 1);
            LCD1602_Print(display);
        } else {
            LCD1602_Print("OUT OF RANGE");
        }
				if(distance<slider*unit)							// Zatrzymaj/wznów
				{
					on_off=(~on_off);
					if(on_off){
						SysTick_Config(SystemCoreClock/DIV_CORE);
					}
				}else{
					SysTick_Config(1);
				}
			}
			wynik_ok=0;
			}
		if(S1_press)	// Czy klawisz S1 wciśnięty
		{
			S1_press=0;
		}	
		if(S2_press)	// Czy klawisz S2 wciśnięty
		{
			S2_press=0;
		}
		if(S3_press)	// Czy klawisz S3 wciśnięty
		{
			S3_press=0;
		}
		if(S4_press)	// Czy klawisz S4 wciśnięty
		{
			S4_press=0;
		}
		}
	}
