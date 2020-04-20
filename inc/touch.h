// File: touch.h

/*
	This code was written to support the book, "ARM Assembly for Embedded Applications",
	by Daniel W. Lewis. Permission is granted to freely share this software provided
	that this notice is not removed. This software is intended to be used with a run-time
    library adapted by the author from the STM Cube Library for the 32F429IDISCOVERY 
    board and available for download from http://www.engr.scu.edu/~dlewis/book3.
*/

#ifndef __TOUCH_H
#define __TOUCH_H

#ifndef __PIXELS
#define __PIXELS
#define  XPIXELS   240	// The left edge of the screen is at x = 0
#define  YPIXELS   320	// The top edge of the screen is at y = 0
#endif

void TS_Init(void) ;
int	TS_Touched(void) ;	// returns non-zero if touched
int	TS_GetX(void) ;		// returns 0 to XPIXELS - 1
int	TS_GetY(void) ;		// returns 0 to YPIXELS - 1

#endif
