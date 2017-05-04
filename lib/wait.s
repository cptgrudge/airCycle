.equ __P24FJ64GA002,1 ;required "boilerplate" (BP)
.include "p24Fxxxx.inc" ;BP

#include "xc.inc" ;BP

.text ;BP (put the following data in ROM(program memory))
; This is a library, thus it can *not* contain a _main function: the C file will
; deine main(). However, we
; we will need a .global statement to make available ASM functions to C code.
; All functions utilized outside of this file will need to have a leading
; underscore (_) and be included in a comment delimited list below.
.global _wait_1us,_wait_50us,_wait_1ms,_low_50us,_low_1ms
    
_wait_1us:	    ; 1us delay
    repeat  #11	    ; 1 cycle to load and prep
    nop		    ; 11+1 cycles to execute NOP 12 times
    return	    ; 3 cycles for return
		    ; 16 cycles total (1us)

_wait_50us:	    ; 50us delay
    repeat  #795    ; 1 cycle to load and prep
    nop		    ; 795+1 cycles to execute NOP 796 times
    return	    ; 3 cycles for return
		    ; 800 cycles total (50us) 
		    
_wait_1ms:	    ; 1ms delay
    repeat  #15995  ; 1 cycle to load and prep
    nop		    ; 15995+1 cycles to execute NOP 15996 times
    return	    ; 3 cycles for return
		    ; 16000 cycles total (1ms)
		    
_low_50us:	    ; >50us low
    bclr LATA, #0   ; 1 cycle to set RA0 low 
    repeat  #800    ; 1 cycle to load and prep
    nop		    ; 800+1 cycles to execute NOP 801 times
    return	    ; 3 cycles for return
		    ; 806 cycles total (50.375us)

_low_1ms:	    ; >1ms low
    bclr LATA, #0   ; 1 cycle to set RA0 low
    repeat  #16000  ; 1 cycle to load and prep
    nop		    ; 16000+1 cycles to execute NOP 16001 times
    return	    ; 3 cycles for return
		    ; 16006 cycles total (1.000375ms)
		    