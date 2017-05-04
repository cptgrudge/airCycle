/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

#ifndef LCD_H
#define	LCD_H

#ifdef	__cplusplus
extern "C" {
#endif

void lcd_cmd(char Package);
void setup(void);
void initDisplay(void);
void lcd_setCursor(char x, char y);
void lcd_printChar(char Package);
void writeString(const char * str);
    
#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */

