/* 
 * File:        PWMfan.c   
 * Author:      csaiko
 * Comments:    C file for PWMfan library
 * Description: This library implements control of a CoolerMaster
 *              "Blade Master 80" Pulse Width Modulation computer case fan
 *              through a PIC24 microcontroller.
 * 
 *              PIC resources setup and used by the library are the INTO interrupt,
 *              output compare, and timer 2. Pins used are 15 (RP6) for output compare,
 *              16 (INT0) for INT0 interrupt, and 24 (RB12) for relay control.  
 */

#include "xc.h"
#include "wait.h"

#define RPM_BUF_SIZE 128    // PWM fan RPM buffer size

//global variables
volatile unsigned int rpmBuf[RPM_BUF_SIZE]; // circular buffer to hold the RPM
volatile int rpmIdx=0;                      // circular buffer index
volatile unsigned int start, end;           // start, end variables
volatile unsigned long int numCycles = 0;   // period of the external signal
                                            // in clock cycles

//function definitions here

//  INT0 Capture Interrupt Service Routine
//  Description:    Used by the PWMfan library
//  Precondition:   A rising edge on the PIC INT0 pin has been recieved. 
//  Postcondition:  The current RPM has been stored within the rpmBuf buffer
//                  and the rpmIdx index incremented.
void __attribute__((__interrupt__,__auto_psv__)) _INT0Interrupt(void)
{
    
    _INT0IF = 0;    // reset INT0 interrupt
    
    start = end;    //set start to the last end found
    end = TMR3;     //set end to where TMR3 is at
    
    if (end > start)                //record TMR3 difference
        numCycles = end - start;
    else
        numCycles = (PR3 - start) + end;
    
    numCycles = numCycles << 6;     //record number of cycles in one period
                                    //timer 3 is 1:64 prescalar
    
    numCycles = numCycles << 1;     //number of cycles in one revolution
    
    rpmBuf[rpmIdx++] = 960000000/numCycles;     //store RPM, increment index
    rpmIdx &= (RPM_BUF_SIZE - 1);               //reset index

}

//  initPF function  
//  Description:    Initializes PIC settings for interaction with the PWM fan
//  Precondition:   PIC is active and running, ready to run program
//  Postcondition:  Output compare, INT0 pin, and Timer 2 are set up to work
//                  with the PWM fan.
void initPF(void){
    
    // setup PPS
    __builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS
    RPOR3bits.RP6R = 18; // Use Pin RP6 (Pin 15) for Output Compare 1 (Table 10-3)
    __builtin_write_OSCCONL(OSCCON | 0x40); // lock PPS
        
    //setup INT0
    _INT0EP = 0;  // detect rising edge.
    _INT0IF = 0;  // reset INT0 interrupt flag
    _INT0IE = 1;  // enable INT0 interrupt
    
    // setup output compare 1
    OC1CON = 0x0000;        // reset, OC1 off
    OC1CONbits.OCTSEL = 0;  // Use timer 2
    OC1CONbits.OCM = 0b110; // Output compare on, PWM, no fault
    OC1R = 224;           // PWM fan start, high pulse 1.4us (35%)
    OC1RS = 224;
    
    // setup timer 2
    T2CON = 0x0010; // turn off, 1:1 prescalar, Tcy clock source
    PR2 = 639;  // combined with 1:1 => 4us, PWM period
    TMR2 = 0;    
    T2CONbits.TON = 1;  //turn timer 2 on
    
    return;
}// end initPF


//  setPWMfanSpd function  
//  Description:    Sets the speed for the PWM fan.
//  Precondition:   PWM fan is operational.
//  Postcondition:  The fan speed is adjusted to a percentage of maximum,
//                  supplied by the function call.
void setPFspd(unsigned int pspd){

    if (pspd > 100) pspd = 100; //if user entered a fanSpeed greater than 100,
                                //correct to 100%
    pspd *= 638;
    pspd /= 100;
    
    OC1RS = pspd;
    
    return;
}//end setPFspd


//  getPFrpm function
//  Description:    Used to ascertain the current RPM of the PWM fan.
//  Precondition:   None.
//  Postcondition:  The RPM of the PWM fan is returned, to the least multiple
//                  of 100.
int getPFrpm(void){

    unsigned int rpm = 0;
    
    if (!_RB10) return 0;        //fan is off, just return 0
    
    int index = rpmIdx;         //get current rpmBuf index
    
    rpm = rpmBuf[index];    //get last RPM
    
    rpm /= 100;             //set to least 100 rpm
    rpm *= 100;
    
    return rpm;
    
}// end getPFrpm


//  enablePF function
//  Description:    Used to enable the PWM fan.
//  Precondition:   PIC setup() has been run.
//  Postcondition:  RB12 is set to 1, PWM fan relay is active.
void enablePF(){
    asm("bset LATB,#12");
    return;
}// end enablePF


//  disablePF function
//  Description:    Used to disable the PWM fan.
//  Precondition:   PIC setup() has been run.
//  Postcondition:  RB12 is set to 0, PWM fan relay is inactive.
void disablePF(){
    setPFspd(0);
    asm("bclr LATB,#12");
    return;
}// end disablePF