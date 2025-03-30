#include "TPM.h"
void PWM_Init()
{
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;			// Dołączenie sygnału zegara do portu B
	// Wybór odpowiedniej roli pinu portu B
	PORTB->PCR[8] |= PORT_PCR_MUX(2);		// LED R 	TPM0_CH3
	PORTB->PCR[9] |= PORT_PCR_MUX(2);		// LED G	TPM0_CH2
	PORTB->PCR[10] |= PORT_PCR_MUX(2);	// LED B	TPM0_CH1
	
	SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;				// Dołączenie sygnału zegara do TPM0
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);			  // Wybierz źródło taktowania TPMx MCGFLLCLK=41943040Hz
	
	TPM0->SC &= ~TPM_SC_CPWMS_MASK;					//	TPM0 w trybie zliczanie "w przód"
	TPM0->SC |= TPM_SC_PS(6);								//	Dzielnik zegara wejściowegoo równy 64; zegar=655360Hz
	TPM0->MOD = 0xFFFF;													//	Rejestr MODULO=65535 - fwy=10Hz
	TPM0->CONTROLS[1].CnSC = TPM_CnSC_MSB_MASK|TPM_CnSC_ELSA_MASK;	//	TPM0, kanał 1 (LED niebieski) tryb "Edge-aligned PWM Low-true pulses (set Output on match, clear Output on reload)"
	TPM0->CONTROLS[1].CnV = 0x0000;					// Wsółczynnik wypełnienia początkowo 0
	TPM0->CONTROLS[2].CnSC = TPM_CnSC_MSB_MASK|TPM_CnSC_ELSA_MASK;	//	TPM0, kanał 2 (LED zielony) tryb "Edge-aligned PWM Low-true pulses (set Output on match, clear Output on reload)"
	TPM0->CONTROLS[2].CnV = 0x0000;					// Wsółczynnik wypełnienia początkowo 0
	TPM0->CONTROLS[3].CnSC = TPM_CnSC_MSB_MASK|TPM_CnSC_ELSA_MASK;	//	TPM0, kanał 3 (LED czerwony) tryb "Edge-aligned PWM Low-true pulses (set Output on match, clear Output on reload)"
	TPM0->CONTROLS[3].CnV = 0x0000;					// Wsółczynnik wypełnienia początkowo 0
	TPM0->SC |= TPM_SC_CMOD(1);							// Włącz licznik TPM0
	
	float k_curr=0.02;					// Początkowy współczynnik wypełnienia K=0.02%, aby sygnał aktywacyjny TRIG trwał 10us
	uint16_t	ampl;
	
	TPM0->MOD = MOD_10Hz;		// Początkowa wartość MOD dla f=20Hz
	ampl=((int)MOD_10Hz*k_curr)/100;	// Współczynnik wypełnienia k przeliczony dla aktualnego MOD - realna warotść CnV
	TPM0->CONTROLS[3].CnV = ampl; 	// Sygnał o nowym k poadany na diodę R LED oraz HC-SR04
}

void InCap_OutComp_Init()
{
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;			// Dołączenie sygnału zegara do portu B
	// Wybór odpowiedniej roli pinu
	PORTB->PCR[0] |= PORT_PCR_MUX(2);		// PTB0 - EXTRG_IN
	PORTB->PCR[13] |= PORT_PCR_MUX(2);	// PTB13 - TPM1_CH1 (Input Capture)

	SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;				// Dołączenie sygnału zegara do modułów TPM1
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);		// Wybierz źródło taktowania TPMx MCGFLLCLK=41943040Hz
	
	TPM1->SC &= (~TPM_SC_CPWMS_MASK);				//	TPM1 w trybie "up counting"
	TPM1->SC &= (~TPM_SC_PS_MASK);					//	Dzielnik zegara wejściowegoo równy 1; zegar = 41943040Hz
	TPM1->CONF |= (TPM_CONF_CSOT_MASK | TPM_CONF_CROT_MASK | TPM_CONF_CSOO_MASK);	// TPM1: wyzwalanie sprzętowe, reload po wyzwoleniu, stop po OVERFLOW
	TPM1->CONF &= (~TPM_CONF_TRGSEL_MASK);				// Wyzwalanie zewnętrznym sygnałem EXTRG_IN
	TPM1->CONTROLS[1].CnSC = (TPM_CnSC_ELSB_MASK | TPM_CnSC_CHIE_MASK);// Input Capture w kanale nr 1 na opadającym zboczu, aktywne przerwanie dla tego trybu
	
	TPM1->SC |= TPM_SC_TOIE_MASK;					// Odblokowanie przerwania od OVERFLOW dla TPM1
	NVIC_ClearPendingIRQ(TPM1_IRQn);
  NVIC_EnableIRQ(TPM1_IRQn);
	
	TPM1->SC |= TPM_SC_CMOD(1);		// Włączenie TPM1
}

float calculateDistance(float echoTime, float unit) {
    //float distance = 0.0001*((echoTime * 343.0) / 2)*1000 * unit; // Obliczenie dystansu w odpowiednich jednostkach
		float distance = echoTime/58*1000*unit;
	return distance;
}