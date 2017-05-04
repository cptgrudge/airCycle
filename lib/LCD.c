#include "xc.h"
#include "wait.h"

void lcd_cmd(char Package) {
    I2C2CONbits.SEN = 1; //Initiate Start condition
    while(I2C2CONbits.SEN == 1); // SEN will clear when Start Bit is complete
    
    IFS3bits.MI2C2IF = 0;
    
    I2C2TRN = 0b01111100; // 8-bits consisting of the slave address and the R/nW bit
    
    while(IFS3bits.MI2C2IF == 0);
    IFS3bits.MI2C2IF = 0;
    
    I2C2TRN = 0b00000000; // 8-bits consisting of command byte
    
    while(IFS3bits.MI2C2IF == 0);
    IFS3bits.MI2C2IF = 0;
    
    I2C2TRN = Package; // 8-bits consisting of the data byte
    
    while(IFS3bits.MI2C2IF == 0);
    IFS3bits.MI2C2IF = 0;
    
    I2C2CONbits.PEN = 1;
    
    while(I2C2CONbits.PEN == 1);// PEN will clear when Stop bit is complete
    IFS3bits.MI2C2IF = 0;
}

void initDisplay(void) {
    int i = 0;

    for(i = 0; i < 50; i++) {
        wait_1ms();    // wait 50ms
    }
    
    lcd_cmd(0b00111000); // function set, normal instruction mode
    lcd_cmd(0b00111001); // function set, extended instruction mode
    lcd_cmd(0b00010100); // interval osc
    lcd_cmd(0b01110111); // contrast C3-C0
    lcd_cmd(0b01011101); // C5-C4, Ion, Bon
    lcd_cmd(0b01101100); // follower control
    
    for(i = 0; i < 200; i++) {
        wait_1ms();    // wait 200ms
    }
    
    lcd_cmd(0b00111000); // function set, normal instruction mode
    lcd_cmd(0b00001100); // Display On
    lcd_cmd(0b00000001); // Clear Display 
}

void lcd_setCursor(char x, char y) {
    int address = 0b10000000;
    int position = (((x * 0x40) + y) | address);
    
    lcd_cmd(position);
}

void writeString(const char* str) {
    I2C2CONbits.SEN = 1;
    
    while(I2C2CONbits.SEN == 1);
    IFS3bits.MI2C2IF = 0;
    
    I2C2TRN = 0b01111100;
    
    while(IFS3bits.MI2C2IF == 0);
    IFS3bits.MI2C2IF = 0;
    
    int i = 0;
    while(str[i+1] != '\0') {
        I2C2TRN = 0b11000000;
        
        while(IFS3bits.MI2C2IF == 0);
        IFS3bits.MI2C2IF = 0;
        
        if (str[i] == '*') {
            I2C2TRN = 0b11001001;
        }
        else if (str[i] == '&') {
            I2C2TRN = 0b10100100;
        }
        else if (str[i] == '#') {
            I2C2TRN = 0b11110010;
        }
        else {
            I2C2TRN = str[i];
        }
        
        while(IFS3bits.MI2C2IF == 0);
        IFS3bits.MI2C2IF = 0;
    
        i++;
    }
    I2C2TRN = 0b01000000;
    
    while(IFS3bits.MI2C2IF == 0);
    IFS3bits.MI2C2IF = 0;
    
    I2C2TRN = str[i];
    
    while(IFS3bits.MI2C2IF == 0);
    IFS3bits.MI2C2IF = 0;
    
    I2C2CONbits.PEN = 1;
    
    while(I2C2CONbits.PEN == 1);// PEN will clear when Stop bit is complete
    IFS3bits.MI2C2IF = 0;
}

void lcd_printChar(char Package) {
    I2C2CONbits.SEN = 1; //Initiate Start condition
    while(I2C2CONbits.SEN == 1); // SEN will clear when Start Bit is complete
    
    IFS3bits.MI2C2IF = 0;
    
    I2C2TRN = 0b01111100; // 8-bits consisting of the slave address and the R/nW bit
    
    while(IFS3bits.MI2C2IF == 0);
    IFS3bits.MI2C2IF = 0;
    
    I2C2TRN = 0b01000000; // 8-bits consisting of control byte /w RS=1
    
    while(IFS3bits.MI2C2IF == 0);
    IFS3bits.MI2C2IF = 0;
    
    I2C2TRN = Package; // 8-bits consisting of the data byte
    
    while(IFS3bits.MI2C2IF == 0);
    IFS3bits.MI2C2IF = 0;
    
    I2C2CONbits.PEN = 1;
    
    while(I2C2CONbits.PEN == 1);// PEN will clear when Stop bit is complete
    IFS3bits.MI2C2IF = 0;
}