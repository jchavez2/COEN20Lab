/*
	File Name: lab_functions_src.s
	This progarm will run the functions: Add, Less1, Square, Last.
	Using assembly

*/

.syntax     unified
		.cpu        cortex-m4
        .text

// ----------------------------------------------------------
// int32_t Less1(int32_t x) {return a - 1}
// ----------------------------------------------------------

		.global		Less1
        .thumb_func
        .align
Less1:		
			SUB R0,R0,1
			BX LR

// ----------------------------------------------------------
// int32_t Add(int32_t x, int32_t y) {return a + b}
// ----------------------------------------------------------

		.global		Add
        .thumb_func
        .align

Add:
		ADD R0,R0,R1
		BX LR
// ----------------------------------------------------------
// int32_t Square2x(int32_t x) {return Square(x + x)}
// ----------------------------------------------------------

		.global		Square2x
        .thumb_func
        .align

Square2x:
			ADD R0, R0, R0
			B Square

// ----------------------------------------------------------
// int32_t Last(int32_ x) {return x + SquareRoot(x)}
// ----------------------------------------------------------

		.global		Last
        .thumb_func
        .align
Last:	
			PUSH {R5, LR}
			MOV R5,R0
			BL SquareRoot
			ADD R0,R0,R5
			POP {R5, LR}
			BX LR

			.end

