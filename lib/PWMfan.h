/* 
 * File:        PWMfan.h   
 * Author:      csaiko
 * Comments:    Header file for PWMfan library
 */

#ifndef PWMFAN_H
#define	PWMFAN_H

#ifdef	__cplusplus
extern "C" {
#endif

//function prototypes here
void initPF(void);
void setPFspd(unsigned int pspd);
int getPFrpm(void);
void enablePF();
void disablePF();

#ifdef	__cplusplus
}
#endif

#endif	/* PWMFAN_H */

