/* 
 * File:   wait.h
 * Author: csaiko
 *
 * Created on March 8, 2017, 9:04 AM
 */

#ifndef WAIT_H
#define	WAIT_H

#ifdef	__cplusplus
extern "C" {
#endif
    
    void wait_1us(void);
    void wait_50us(void);
    void wait_1ms(void);
    void low_50us(void);
    void low_1ms(void);
    void msdelay(int delay);
    void usdelay(int delay);
    
#ifdef	__cplusplus
}
#endif

#endif	/* WAIT_H */

