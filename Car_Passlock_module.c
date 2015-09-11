/*
***********************************************************************
 ECE 362 - Mini_Project
***********************************************************************

***********************************************************************
*/
#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <mc9s12c32.h>
 
/* All funtions after main should be initialized here */
char inchar(void);
void outchar(char x);
void tdisp();
void shiftout(char x);
void lcdwait(void);
void send_byte(char x);
void send_i(char x);
void chgline(char x);
void print_c(char x);
void pmsglcd(char[]);
void change_direction(int atdc);
int atd_convert();
/* Variable declarations */                           
char goteam  = 0;  // "go team" flag (used to start reaction timer)
char leftpb = 0;  // left pushbutton flag
char rghtpb = 0;  // right pushbutton flag
char prevpb = 0;  // previous pushbutton state
char prevpb1 = 0;
char runstp = 0;  // run/stop flag
int random = 0;  // random variable (2 bytes)
int react = 0;  // reaction time (3 packed BCD digits)
int counterw=0;
int atdc=0;
int atdi=2000;
int atdia=2000;
int atdca=0;
char atdflag=0;

// PASSWORD VARIABLE DECLARATIONS--------------------------

int temp[3] = {0,0,0};
int our_pass[3] = {1,2,3};
int flag2 = 0;
int flag = 0;
int counter_pass = 0;
int i = 0;
  int tempo = 2;
  int temp2 = 0;
//---------------------------------------------------------


/* ASCII character definitions */
#define CR 0x0D // ASCII return character   
/* LCD COMMUNICATION BIT MASKS */
#define RS 0x04  // RS pin mask (PTT[2])
#define RW 0x08  // R/W pin mask (PTT[3])
#define LCDCLK 0x10 // LCD EN/CLK pin mask (PTT[4])
/* LCD INSTRUCTION CHARACTERS */
#define LCDON 0x0F // LCD initialization command
#define LCDCLR 0x01 // LCD clear display command
#define TWOLINE 0x38 // LCD 2-line enable command
#define CURMOV 0xFE // LCD cursor move instruction
#define LINE1 = 0x80 // LCD line 1 cursor position
#define LINE2 = 0xC0 // LCD line 2 cursor position \

/* LED BIT MASKS */
#define GREEN 0x20
#define RED 0x40
#define YELLOW 0x80
        
/*
***********************************************************************
 Initializations
***********************************************************************
*/
void  initializations(void) {
/* Set the PLL speed (bus clock = 24 MHz) */
  CLKSEL = CLKSEL & 0x80; // disengage PLL from system
  PLLCTL = PLLCTL | 0x40; // turn on PLL
  SYNR = 0x02;            // set PLL multiplier
  REFDV = 0;              // set PLL divider
  while (!(CRGFLG & 0x08)){  }
  CLKSEL = CLKSEL | 0x80; // engage PLL
/* Disable watchdog timer (COPCTL register) */
  COPCTL = 0x40;   //COP off, RTI and COP stopped in BDM-mode
/* Initialize asynchronous serial port (SCI) for 9600 baud, no interrupts */
  SCIBDH =  0x00; //set baud rate to 9600
  SCIBDL =  0x9C; //24,000,000 / 16 / 156 = 9600 (approx)  
  SCICR1 =  0x00; //$9C = 156
  SCICR2 =  0x0C; //initialize SCI for program-driven operation
  DDRB   =  0x10; //set PB4 for output mode
  PORTB  =  0x10; //assert DTR pin on COM port
         
         
/* Add additional port pin initializations here */
  DDRAD = 0;   //program port AD for input mode
  ATDDIEN = 0xf0; //program PAD7 and PAD6 pins as digital inputs
  DDRM_DDRM5=1;
  DDRM_DDRM4=1;
  DDRT=0xff;
   
  ATDCTL2 = 0x80;
  ATDCTL3 = 0x10;
  ATDCTL4 = 0x85;
  //ATDCTL5 = 0x85;
  
  
/* Initialize SPI for baud rate of 6 Mbs */
  SPIBR=0x01;
  SPICR1=0x50;
  SPICR2=0x08;
/* Initialize digital I/O port pins */
  //Initialising PWM
  
  /*MODRR=0x02;//pt3 as ch3   0000 1000
  PWME=0x02; //enable pwm ch3
  PWMPOL=0x00;//active low polarity 
	PWMCTL=0;//no concatenate (8) 
  PWMCAE=0;// left-aligned output mode 
	PWMPER1=0xff;   //period ff
  PWMDTY1=0x00;      //duty cycle
  PWMCLK=0x80;       //clock a for cho 0
  PWMSCLA=0x00;
	PWMPRCLK=0x07;  //clock a =12m    */
  
  
  PTT_PTT6=1;
  PTT_PTT7=0;
  send_i(LCDON);
  send_i(TWOLINE);
  send_i(LCDCLR);
  lcdwait();
  

 
  
  
/* Initialize RTI for 2.048 ms interrupt rate */ 
  RTICTL=0x50 ;
  CRGINT_RTIE=1;
  
/* Initialize TIM Ch 7 (TC7) for periodic interrupts every 1.000 ms
     - enable timer subsystem
     - set channel 7 for output compare
     - set appropriate pre-scale factor and enable counter reset after OC7
     - set up channel 7 to generate 0.01 ms interrupt rate
     - initially disable TIM Ch 7 interrupts      
*/
  TSCR1=0x80;
  TIOS = 0x80;
  TSCR2=0x0c;
  TC7=15; 
  TSCR2_TCRE=1;
  //TIE=0xff;
  TIE_C7I = 0;
  
  

}
                
/*
***********************************************************************
 Main
***********************************************************************
*/
void main(void) {
  DisableInterrupts;
 initializations();               
 EnableInterrupts;
 
 //shiftout(0x01);
 

    
      while(flag == 0 && counter_pass < 3)
      {       
  
    chgline(0x80);
    pmsglcd("Enter Password");
      
    if(rghtpb == 0) 
    {
      chgline(0xc0 + counter_pass);

      tempo = atd_convert() ;
  
      print_c(tempo);

     
    }
    
    if(rghtpb == 1) 
    {
     rghtpb = 0;
     temp[counter_pass] = atd_convert() - 48;
     counter_pass++;
     
    }
    if(counter_pass == 3) {
      
    for(i = 0; i < 3; i++) 
    {
        if(our_pass[i] == temp[i]) 
        {
          if(flag2 != 1) 
          {
            flag = 1; 
          } 
        }
        else 
        {
          flag2 = 1;
          flag = 0;
          counter_pass = 0;
          temp[0] = 0;
          temp[1] = 0;
          temp[2] = 0;
          chgline(0xc0);
          send_i(LCDCLR);
        }
    }
    
    
    }
  }
send_i(LCDCLR);   
pmsglcd("Success");
TIE_C7I = 1;
    
    //print_c(tempo);

  for(;;) {

/* write your code here */

    

  }  
  
    
  

    

 /* loop forever */
  
}/* do not leave main */
 

/*
***********************************************************************
 RTI interrupt service routine: RTI_ISR
  Initialized for 2.048 ms interrupt rate
  Samples state of pushbuttons (PAD7 = left, PAD6 = right)
  If change in state from "high" to "low" detected, set pushbutton flag
     leftpb (for PAD7 H -> L), rghtpb (for PAD6 H -> L)
     Recall that pushbuttons are momentary contact closures to ground
  Also, increments 2-byte variable "random" each time interrupt occurs
  NOTE: Will need to truncate "random" to 12-bits to get a reasonable delay 
***********************************************************************
*/
interrupt 7 void RTI_ISR(void)
{
   CRGFLG = CRGFLG | 0x80;

    if(PORTAD0_PTAD7==0){
      if(prevpb==1){
        leftpb=1;
      }
    }
    prevpb = PORTAD0_PTAD7;
    if(PORTAD0_PTAD6==1){
      if(prevpb1==0){
        rghtpb = 1;
      }
    }
    prevpb1 = PORTAD0_PTAD6;
}

/*
*********************************************************************** 
  TIM Channel 7 interrupt service routine
  Initialized for 1.00 ms interrupt rate
  Increment (3-digit) BCD variable "react" by one
***********************************************************************
*/
interrupt 15 void TIM_ISR(void)
{
 // clear TIM CH 7 interrupt flag
  TFLG1 = TFLG1 | 0x80; 
   //if(flag == 1) {
    

  //Sampling Speed For Enable
    
   atdia++; //Increment atdi
        
  if(atdia == 2000) {
   atdia = 0; //reset
   if(atdca <= 1760) {
      change_direction(atdc);

  
   } else if(atdca > 1800) {
        PTT_PTT0 = 0;
   PTT_PTT1 = 0;
   PTT_PTT2 = 0;
   PTT_PTT3 = 0; 
        
   }
   
   atdca=0;     //reset atdca
  }
  if(PORTAD0_PTAD4 == 1) {
    atdca++;
    
  } 
  
  //Increments atdc to change the direction
  
    atdi++; //Increment atdi
    
  if(atdi == 2000) {
   atdi = 0;      //reset atdi counter
   atdc = 0;      //reset atdc counter
  }
  if(PORTAD0_PTAD5 == 1) {
    atdc++;
    
  }

  // }
  
}

/*
*********************************************************************** 
CHANGE DIRECTION FUNCTION
***********************************************************************
*/

void change_direction(int atdc) {
    
   if(atdc <= 140) {
   PTT_PTT0 = 0;
   PTT_PTT1 = 1;
   PTT_PTT2 = 1;
   PTT_PTT3 = 0;
   
   } else if(atdc > 140 && atdc < 150) {

   PTT_PTT0 = 1;
   PTT_PTT1 = 0;
   PTT_PTT2 = 1;
   PTT_PTT3 = 0;
       
   } else if(atdc >= 150) {
     
   PTT_PTT0 = 1;
   PTT_PTT1 = 0;
   PTT_PTT2 = 0;
   PTT_PTT3 = 1;    
   } 
}

/*
*********************************************************************** 
  ATD CONVERT FUNCTIONS CONVERTS ANALOG TO DIGITAL
***********************************************************************
*/

int atd_convert()
{
          
          ATDCTL5_MULT=1;
          while(ATDSTAT0_SCF == 0){
          }
          
          
	if(ATDDR0H < 25)
	{
		return 48;
	}

	else if(ATDDR0H > 26 && ATDDR0H < 50)
	{
		return 49;
	}

	else if(ATDDR0H > 50 && ATDDR0H < 75)
	{
		return 50;
	}

	else if(ATDDR0H > 75 && ATDDR0H < 100)
	{
		return 51;
	}

	else if(ATDDR0H > 100 && ATDDR0H < 125)
	{
		return 52;
	}

	else if(ATDDR0H > 125 && ATDDR0H < 150)
	{
		return 53;
	}

	else if(ATDDR0H > 150 && ATDDR0H < 175)
	{
		return 54;
	}

	else if(ATDDR0H > 175 && ATDDR0H < 200)
	{
		return 55;
	}

	else if(ATDDR0H > 200 && ATDDR0H < 225)
	{
		return 56;
	}

	else if(ATDDR0H > 225 && ATDDR0H < 255)
	{
		return 57;
	}


}
/*
*********************************************************************** 
  tdisp: Display "RT = NNN ms" on the first line of the LCD and display 
         an appropriate message on the second line depending on the 
         speed of the reaction.  
         
         Also, this routine should set the green LED if the reaction 
         time was less than 250 ms.
         NOTE: The messages should be less than 16 characters since
               the LCD is a 2x16 character LCD.
***********************************************************************
*/
 
void tdisp()
{

}

/*
***********************************************************************
  shiftout: Transmits the character x to external shift 
            register using the SPI.  It should shift MSB first.  
             
            MISO = PM[4]
            SCK  = PM[5]
***********************************************************************
*/
 
void shiftout(char x)
{
   int counterr=0;
   while(SPISR_SPTEF!=1){
   }
   SPIDR=x;
   while(counterr!=30){
    counterr=counterr+1;
   }
  // read the SPTEF bit, continue if bit is 1
  // write data to SPI data register
  // wait for 30 cycles for SPI data to shift out
}
/*
***********************************************************************
  lcdwait: Delay for approx 2 ms
***********************************************************************
*/
void lcdwait()
{
 int counterun=0;
 int counterdo=0;
 int countertre=0;
 while(counterun!=999){
  counterun=counterun+1;
  while(counterdo!=999){
    counterdo=counterdo+1;
    while(countertre!=999){
      countertre=countertre+1;
    }
  }
 }
}
/*
*********************************************************************** 
  send_byte: writes character x to the LCD
***********************************************************************
*/
void send_byte(char x)
{
    shiftout(x);
    PTT_PTT4=1;
    PTT_PTT4=0;
    PTT_PTT4=1;
    lcdwait();
    
     // shift out character
     // pulse LCD clock line low->high->low
     // wait 2 ms for LCD to process data
}
/*
***********************************************************************
  send_i: Sends instruction byte x to LCD  
***********************************************************************
*/
void send_i(char x)
{
  PTT_PTT5=0;
  send_byte(x);
  
        // set the register select line low (instruction data)
        // send byte
}
/*
***********************************************************************
  chgline: Move LCD cursor to position x
  NOTE: Cursor positions are encoded in the LINE1/LINE2 variables
***********************************************************************
*/
void chgline(char x)
{
 send_i(CURMOV);
 send_i(x);
}
/*
***********************************************************************
  print_c: Print (single) character x on LCD            
***********************************************************************
*/
 
void print_c(char x)
{
 PTT_PTT5=1;
 send_byte(x);
}
/*
***********************************************************************
  pmsglcd: print character string str[] on LCD
***********************************************************************
*/
void pmsglcd(char str[])
{
 char counterp=0;
 while(str[counterp]!=0x00){
 print_c(str[counterp]);
 counterp=counterp+1;
 }
}
/*
***********************************************************************
 Character I/O Library Routines for 9S12C32 (for debugging only)
***********************************************************************
 Name:         inchar
 Description:  inputs ASCII character from SCI serial port and returns it
 Example:      char ch1 = inchar();
***********************************************************************
*/
char inchar(void) {
  /* receives character from the terminal channel */
        while (!(SCISR1 & 0x20)); /* wait for input */
    return SCIDRL;
}
/*
***********************************************************************
 Name:         outchar
 Description:  outputs ASCII character x to SCI serial port
 Example:      outchar('x');
***********************************************************************
*/
void outchar(char x) {
  /* sends a character to the terminal channel */
    while (!(SCISR1 & 0x80));  /* wait for output buffer empty */
    SCIDRL = x;
}