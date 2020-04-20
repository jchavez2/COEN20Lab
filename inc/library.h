// File: library.h

/*
	This code was written to support the book, "ARM Assembly for Embedded Applications",
	by Daniel W. Lewis. Permission is granted to freely share this software provided
	that this notice is not removed. This software is intended to be used with a run-time
    library adapted by the author from the STM Cube Library for the 32F429IDISCOVERY 
    board and available for download from http://www.engr.scu.edu/~dlewis/book3.
*/

#ifndef __LIBRARY_H
#define __LIBRARY_H

#define	HEADER		NULL
#define	ERROR_FLAG	"ERR"

extern void		CallReturnOverhead(void) ;
extern void		ClearDisplay(void) ;
extern void		ClearScreen(int color) ;
extern unsigned	CountCycles(void *function, void *iparams, void *fparams, void *results) ;
extern void		DisplayHeader(char *header) ;
extern void		DisplayFooter(char *footer) ;
extern uint32_t	GetClockCycleCount(void) ;
extern uint32_t	GetRandomNumber(void) ;
extern void		InitializeHardware(char *header, char *footer) ;
extern unsigned	PrintBits(int bin[]) ;
extern void		PrintByte(uint8_t byte) ;
extern int		PushButtonPressed(void) ;
extern void		WaitForPushButton(void) ;

#define		asm		__asm

#endif
