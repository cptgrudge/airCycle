/*
 * File:   airCycle_main.c
 * Author: csaiko
 * Description: Main driver file for the airCycle program
 * Created on May 3, 2017, 7:52 PM
 */

#include "xc.h"
#include <p24Fxxxx.h>
#include <p24FJ64GA002.h>
#include <stdio.h>
#include "lib/PWMfan.h"
#include "lib/LCD.h"
#include "lib/wait.h"

// CW1: FLASH CONFIG WORD 1 (see PIC24 Family Ref Manual 24.1)
#pragma config ICS = PGx1       // Comm Channel Select (Emulator EMUC1/EMUD1 pins are shared with PGC1/PGD1)
#pragma config FWDTEN = OFF     // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config GWRP = OFF       // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF        // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF     // JTAG Port Enable (JTAG port is disabled)

// CW2: FLASH CONFIG WORD 2 (see PIC24 Family Ref Manual 24.1)
#pragma config POSCMOD = NONE   // Primary Oscillator Select (Primary oscillator disabled. 
                                // Primary Oscillator refers to an external osc connected to the OSC1 and OSC2 pins)
#pragma config I2C1SEL = PRI    // I2C1 Pin Location Select (Use default SCL1/SDA1 pins)
#pragma config IOL1WAY = OFF    // IOLOCK Protection (IOLOCK may be changed via unlocking seq)
#pragma config OSCIOFNC = ON    // Primary Oscillator I/O Function (CLK0/RC15 functions as I/O pin)
#pragma config FCKSM = CSECME   // Clock Switching and Monitor (Clock switching is enabled, Fail-Safe Clock Monitor is enabled)
#pragma config FNOSC = FRCPLL   // Oscillator Select (Fast RC Oscillator with PLL module (FRCPLL))

#define ADC_BUF_SIZE 1024

volatile int readIdx=0, writeIdx=0;
volatile unsigned int buffer[ADC_BUF_SIZE];
volatile int value=0;
volatile float lux = 10.0;
volatile float targetTemp = 0.0;
volatile float currentTemp = 70.0;
volatile char adString[20];

//driver function prototypes
void setup(void);



void __attribute__((__interrupt__,__auto_psv__)) _T5Interrupt(void){
    _T5IF=0;                    //reset interrupt flag
    
    value = buffer[readIdx];    //get value
    
    targetTemp = ((40.0/1024)*value) + 59;
    
    //Write target temperature to upper line
    sprintf(adString,"%2.1fF ta", targetTemp);  //create string
    lcd_setCursor(0,0);                             //set location
    writeString(adString);                          //write string

    //Write current temperature to lower line    
    //sprintf(adString,"%2.1fF cu", currentTemp);  //create string
    //lcd_setCursor(1,0);                             //set location
    //writeString(adString);                          //write string
    
    //Write fan RPM to lower line
    sprintf(adString,"%4.0f RPM", (float)getPFrpm());  //create string
    lcd_setCursor(1,0);                             //set location
    writeString(adString);                          //write string
    
    //Write current lux value to lower line?
    //sprintf(adString,"%3.1f RPM", lux);  //create string
    //lcd_setCursor(1,0);                             //set location
    //writeString(adString);
}

void __attribute__((__interrupt__,__auto_psv__)) _ADC1Interrupt(void) {
    _AD1IF = 0;         // reset interrupt flag
    
    readIdx = writeIdx;

    buffer[writeIdx++] = ADC1BUF0;  // get value from buffer, place in circ buffer
    writeIdx &= (ADC_BUF_SIZE - 1);   // reset writeIdx if over ADC_BUF_SIZE
}

int main(void) {
    
    unsigned int fanSpeed = 0;
    unsigned int fanRPM;
    
    setup();        //setup registers
    initPF();       //init PWM fan
    initDisplay();  //init LCD display
    
    lcd_setCursor(0,0);
    writeString("00000000");
    lcd_setCursor(1,0);
    writeString("00000000");
    
    disablePF();
    
    
    while(1){   //forever loop

        fanRPM = getPFrpm();
        
        if (currentTemp > targetTemp){
            enablePF();
            fanSpeed = (unsigned int)((currentTemp - targetTemp) * 10);
        }
        else
            disablePF();
        
        
        setPFspd(fanSpeed);
        
    }//end while loop
    
    return 0;
    
}//end main

//driver functions

//setup function
//initialize PIC24 stuff
void setup(void){

    // Execute once code here
    CLKDIVbits.RCDIV = 0;   //set RCDIV=1:1 (default 2:1) 32MHz or FCY/2=16M
    AD1PCFG = 0x9ffe;       //control analog or digital operation, set AN0 analog
    
    //set pin modes to output(0) or input(1)
    TRISA = 0x0000; //all output
    TRISB = 0x0080; //set RB4 to output for red LED
                    //set RB5 to output for green LED
                    //set RB8 to output for fan relay control
                    //set RB10 to output for fan PWM control 
                    //set RB7 to input for fan RPM sense (INT0)
                    
    //set initial outputs
    LATA = 0x0000;     //set all outputs to low
    LATB = 0x0030;     //set RB4,RB5 to high

    //set up for LCD screen
    //set up for temp sensor
    I2C2CONbits.I2CEN = 0x0;
    I2C2BRG = 0x9D;
    I2C2CONbits.I2CEN = 0x1;
    IFS3bits.MI2C2IF = 0x0;    
    
    // setup Timer 3 - should be every 200ms
    // for ADC Conversion
    T3CON = 0;
    TMR3 = 0;
    PR3 = 50000;    //200ms
    T3CONbits.TCKPS = 2; // pre 1:64
    
    // setup Timer 5 - should be every 100ms
    // LCD refresh
    T5CON = 0;
    TMR5 = 0;
    PR5 = 25000;
    T5CONbits.TCKPS = 2; // pre 1:64
    _T5IF = 0; // clear flag
    _T5IE = 1; // enable interrupt
    
    // Setup on A/D
    AD1CON1 = 0;
    AD1CON2 = 0;
    AD1CON3 = 0;
    AD1CHS = 0;

    AD1CON1bits.ASAM = 1; // auto-sampling. P21, Sec. 17.7.3.3 in the ADC
       // document says use ASAM=1
    AD1CON1bits.SSRC = 0b010; // Use Timer3 as the conversion trigger
       // We can also use Timer3 / Timer2 as a 32-bit timer to trigger
       // A/D sampling/conversion, because the 32-bit timer
       // will also generate the T3IF event.

    AD1CON3bits.SAMC = 3; // auto-sample time = X * TAD (what is X?)
    AD1CON3bits.ADCS = 2; // clock = Y * Tcy (what is Y?)
    

    // Configure A/D interrupt
    _AD1IF = 0; // clear flag
    _AD1IE = 1; // enable interrupt
    AD1CON1bits.ADON = 1;   //turn on

    // turn on Timer 3, Timer 5
    //T2CONbits.TON = 1;
    T3CONbits.TON = 1;
    T5CONbits.TON = 1;
    
    return;
}//end setup