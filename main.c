#include  "msp430g2553.h"


/* Defines WDT SMCLK interval for sensor measurements*/
#define WDT_olcme (DIV_SMCLK_512) //ölçümde kullandigi clock  
/* Defines WDT ACLK interval for delay between measurement cycles*/
#define WDT_gecikme ( DIV_ACLK_512)   // Algilama zamanlamasi ayarlanir.(DIV_SMCLK_64 çok erken, DIV_SMCLK_32768 çok geç algiliyor)

/* Sensor settings*/
#define esik_degeri     30                     // Defines threshold for a key press
/*Set to ~ half the max delta expected*/

/* Definitions for use with the WDT settings*/
#define DIV_ACLK_32768  (WDT_ADLY_1000)     // ACLK/32768
#define DIV_ACLK_8192   (WDT_ADLY_250)      // ACLK/8192
#define DIV_ACLK_512    (WDT_ADLY_16)       // ACLK/512
#define DIV_ACLK_64     (WDT_ADLY_1_9)      // ACLK/64
#define DIV_SMCLK_32768 (WDT_MDLY_32)       // SMCLK/32768
#define DIV_SMCLK_8192  (WDT_MDLY_8)        // SMCLK/8192
#define DIV_SMCLK_512   (WDT_MDLY_0_5)      // SMCLK/512
#define DIV_SMCLK_64    (WDT_MDLY_0_064)    // SMCLK/64

#define LED_1   (0x10)                      // P1.4 LED OUTPUT
#define LED_2   (0x08)                      // P1.3 LED OUTPUT
#define LED_3   (0X02)                      // P1.1 LED OUTPUT
#define LED_4   (0X01)                      // P1.0 LED OUTPUT



unsigned int taban_degeri[4];
unsigned int olculen_deger[4];
int degisim[]={0,0,0,0};
int butona_basildi1=0;
int butona_basildi2=0;
int butona_basildi3=0;
int butona_basildi4=0;
char butona_basildi[]={0,0,0,0};


int k;

/* System Routines*/

void olcum1(void);                   // Measures each capacitive sensor
void olcum2(void);
void olcum3(void);
void olcum4(void);
void LED_yak_sondur(void);                       //LED yakip sondurme fonksiyonu



void main(void)
{


int k;
	unsigned int i, j;

	WDTCTL = WDTPW | WDTHOLD;                 // Stop watchdog timer

	// CLOCK CONFIGURATION
	BCSCTL1 = 0x0087;                         // DCO 1MHz AYARLA
	DCOCTL = 0x0043;                          // DCO 1MHz AYARLA


	BCSCTL3 |= LFXT1S_2;                      // Basic Clock System Control 3 LFXT1 = VLO

	IE1 |= WDTIE;                             // watchdog interrupt etkinlestir (interrupt enable |= wd interrupt enable)

	// PORT CONFIGURATION
        //INPUTS--->KEY1=P2.0  KEY2=P2.2   KEY3=P2.3   KEY4=P2.5
        //OUTPUTS-->LED1=P1.4  LED2=P1.3   LED3=P1.1   LED4=P1.0
	P2SEL = 0x00;                             // digital I/O uçlarini seç (XTAL yerine)
	P1DIR = LED_1 | LED_2 | LED_3 | LED_4 ;   // P1.0, P1.1, P1.3, P1.4 = LEDs
	P1OUT = 0x00;                                   

	__bis_SR_register(GIE);                   // status registerdaki "general interrupt enable" bitini 1 yap (0x0008)
   
	olcum1();                          // baslangic kapasitelerini belirle
        olcum2(); 
        olcum3(); 
        olcum4(); 
        for(k=0;k<4;k++)
        {
        taban_degeri[k] = olculen_deger[k];
        }

	for (i = 15; i > 0; i--)     // baslangic kapasitelerinin ortalamalarini al [15 tane ortalama aliyor(örnekleme sayisi)]
	{
                olcum1();
                taban_degeri[0] = (olculen_deger[0] + taban_degeri[0]) / 2; 
                
                olcum2();
		taban_degeri[1] = (olculen_deger[1] + taban_degeri[1]) / 2;
                
                olcum3();
		taban_degeri[2] = (olculen_deger[2] + taban_degeri[2]) / 2;
                
                olcum4();
		taban_degeri[3] = (olculen_deger[3] + taban_degeri[3]) / 2;
                                               
	}

	/* Main loop starts here*/
	while (1)
	{
              j = esik_degeri ;
                
         
                for(k=0;k<4;k++)
                {       
		butona_basildi[k] = 0;                    // butona basilmadigi durumlar
                }
                olcum1();                        // bütün sensörleri ölç
                olcum2();   
                olcum3();   
                olcum4();
		for(k=0;k<4;k++)
                {
                  degisim[k] = taban_degeri[k] - olculen_deger[k];  		// degisimi hesapla
                }
                
		/* Handle baseline measurment for a base C decrease*/
		for(k=0;k<4;k++)
                {
                if (degisim[k] < 0)                    	// If negative: result increased
		{                               		
			taban_degeri[k] = (taban_degeri[k] + olculen_deger[k]) / 2; // yeni ortalamayi hesapla
			degisim[k] = 0;                    	//bir sonraki adimda önceki durumu algilamamasi için sifirla(SIFIRLAMAZSAN HEP YANAR)
		}
                               
                }
                
                
                for(k=0;k<4;k++)
                {
               
		if (degisim[k] > j)                   	// Butonlara dokunuldugunda degisim farki esik degerini geciyor mu ? 
		{                                     	
			j = degisim[k];
			butona_basildi[k] = 1;                    
		}
		else
			butona_basildi[k] = 0;
                
                }
		
             	
                              
		
                  
		///Butonlara dokunulmazsa taban degerini azalt, daha tutarli olmasi icin daha yavas hesapla
		if (!butona_basildi[0])                       
		{                                       
			taban_degeri[0] = taban_degeri[0] - 1;  
		}                                     	
                 if (!butona_basildi[1])                   
		{                                       
			taban_degeri[1] = taban_degeri[1] - 1;  
		}                                     	
                 if (!butona_basildi[2])                   
		{                                       
			taban_degeri[2] = taban_degeri[2] - 1;  
		}    
                 if (!butona_basildi[3])                       
		{                                       
			taban_degeri[3] = taban_degeri[3] - 1;  
		}    
                
                 WDTCTL = WDT_gecikme;                  // WDT, ACLK, interval timer - (ACLK / Div) / 512 = normal(42.7ms) : slow(341.3ms)        
                
		LED_yak_sondur();                          	// LED yak-sondur
                

		__bis_SR_register(LPM3_bits);
	}
}                                           // End Main

/* Measure count result (capacitance) of each sensor*/
/* Routine setup for four sensors, not dependent on NUM_SEN value!*/

void olcum1(void)
{
	
        TA0CTL = TASSEL_3 + MC_2;                   // TACLK, cont mode
	TA0CCTL1 = CM_3 + CCIS_2 + CAP;             // Pos&Neg,GND,Cap


	
	P2DIR &= ~ BIT0;     // P2.0 is the input used here to get sensor signal
	P2SEL &= ~ BIT0; 	// PxSEL.y = 0 & PxSEL2.y = 1 to enable Timer_A clock source form sensor signal
	P2SEL2 |= BIT0;

	/*Setup Gate Timer*/
	WDTCTL = WDT_olcme; // WDT, SMCLK, interval timer - SCMK/512 = 1MHz/512 = 512us interval
	TA0CTL |= TACLR;                        	// Clear Timer_A TAR
	__bis_SR_register(LPM0_bits + GIE);       	// Wait for WDT interrupt
	TA0CCTL1 ^= CCIS0; // Create SW capture of CCR1. Because capture mode set at Pos&Neg so that switching input signal between VCC & GND will active capture
	olculen_deger[0] = TACCR1;                      // Save result
	WDTCTL = WDTPW + WDTHOLD;               // Stop watchdog timer
	P2SEL2 &= ~BIT0;				// Disable sensor signal line to Timer_A clock source

	TA0CTL = 0;                             	// Stop Timer_A
   
}
void olcum2(void)
{
	
  TA0CTL = TASSEL_3 + MC_2;                   // TACLK, cont mode
	TA0CCTL1 = CM_3 + CCIS_2 + CAP;             // Pos&Neg,GND,Cap



	
	P2DIR &= ~ BIT2;     // P2.2 is the input used here to get sensor signal
	P2SEL &= ~ BIT2; 	// PxSEL.y = 0 & PxSEL2.y = 1 to enable Timer_A clock source form sensor signal
	P2SEL2 |= BIT2;

	/*Setup Gate Timer*/
	WDTCTL = WDT_olcme; // WDT, SMCLK, interval timer - SCMK/512 = 1MHz/512 = 512us interval
	TA0CTL |= TACLR;                        	// Clear Timer_A TAR
	__bis_SR_register(LPM0_bits + GIE);       	// Wait for WDT interrupt
	TA0CCTL1 ^= CCIS0; // Create SW capture of CCR1. Because capture mode set at Pos&Neg so that switching input signal between VCC & GND will active capture
	olculen_deger[1] = TACCR1;                      // Save result
	WDTCTL = WDTPW + WDTHOLD;               // Stop watchdog timer
	P2SEL2 &= ~BIT2;				// Disable sensor signal line to Timer_A clock source

	TA0CTL = 0;                             	// Stop Timer_A
   
}
void olcum3(void)
{
	
  TA0CTL = TASSEL_3 + MC_2;                   // TACLK, cont mode
	TA0CCTL1 = CM_3 + CCIS_2 + CAP;             // Pos&Neg,GND,Cap


	P2DIR &= ~ BIT3;     // P2.3 is the input used here to get sensor signal
	P2SEL &= ~ BIT3; 	// PxSEL.y = 0 & PxSEL2.y = 1 to enable Timer_A clock source form sensor signal
	P2SEL2 |= BIT3;

	/*Setup Gate Timer*/
	WDTCTL = WDT_olcme; // WDT, SMCLK, interval timer - SCMK/512 = 1MHz/512 = 512us interval
	TA0CTL |= TACLR;                        	// Clear Timer_A TAR
	__bis_SR_register(LPM0_bits + GIE);       	// Wait for WDT interrupt
	TA0CCTL1 ^= CCIS0; // Create SW capture of CCR1. Because capture mode set at Pos&Neg so that switching input signal between VCC & GND will active capture
	olculen_deger[2] = TACCR1;                      // Save result
	WDTCTL = WDTPW + WDTHOLD;               // Stop watchdog timer
	P2SEL2 &= ~BIT3;				// Disable sensor signal line to Timer_A clock source

	TA0CTL = 0;                             	// Stop Timer_A
   
}
void olcum4(void)
{
	
        TA0CTL = TASSEL_3 + MC_2;                   // Timer0_A3 Control= Timer A clock source select: 3 - INCLK , Timer A mode control: 2 - Continous up
	TA0CCTL1 = CM_3 + CCIS_2 + CAP;             // Timer0_A3 Capture/Compare Control 1= Capture mode: 1 - both edges, Capture input select: 2 - GND 
                                                    //Capture mode: 1 /Compare mode : 0, Pos&Neg,GND,Cap



	P2DIR &= ~ BIT5;     // P2.5 is the input used here to get sensor signal
	P2SEL &= ~ BIT5; 	// PxSEL.y = 0 & PxSEL2.y = 1 to enable Timer_A clock source form sensor signal
	P2SEL2 |= BIT5;

	/*Setup Gate Timer*/
	WDTCTL = WDT_olcme; // WDT, SMCLK, interval timer - SCMK/512 = 1MHz/512 = 512us interval
	TA0CTL |= TACLR;                        	// Clear Timer_A TAR
	__bis_SR_register(LPM0_bits + GIE);       	// Wait for WDT interrupt
	TA0CCTL1 ^= CCIS0; // Create SW capture of CCR1. Because capture mode set at Pos&Neg so that switching input signal between VCC & GND will active capture
	olculen_deger[3] = TACCR1;                      // Save result
	WDTCTL = WDTPW + WDTHOLD;               // Stop watchdog timer
	P2SEL2 &= ~BIT5;				// Disable sensor signal line to Timer_A clock source

	TA0CTL = 0;                             	// Stop Timer_A
   
}


//Kodun en son kismi, Hangi durumda hangi LED yanacak.
void LED_yak_sondur(void)
{
	if (butona_basildi[1])
	{
		
                P1OUT |= LED_1; // LED 1 yak
	}
	else
	{		
                P1OUT &= ~LED_1; // LED  1 söndür
	}
        if (butona_basildi[0])
	{
		
                P1OUT |= LED_2; // LED 2 yak
	}
	else
	{		
                P1OUT &= ~LED_2; // LED  2 söndür
	}      
        if (butona_basildi[2])
	{
		
                P1OUT |= LED_3; // LED 3 yak
	}
	else
	{		
                P1OUT &= ~LED_3; // LED  3 söndür
	}     
        if (butona_basildi[3])
	{
		
                P1OUT |= LED_4; // LED 4 yak
	}
	else
	{		
                P1OUT &= ~LED_4; // LED  4 söndür
	}      
        
}


// Watchdog Timer Kesme Hizmet Programi
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
{
	TA0CCTL1 ^= CCIS0;                        // Create SW capture of CCR1
	__bic_SR_register_on_exit(LPM3_bits);     // Exit LPM3 on reti
}
