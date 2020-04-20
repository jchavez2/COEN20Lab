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
//	This function uses the square passes the parater x as square(x + x).
//	Also this function uses the Branch instruction (B) which branches to the
//  to next function without saving the address to the next instruction into 
//	link register(LR). It is unecssary to use Branch with Link (BL) instruction
//	since this function does not have to return.
// ----------------------------------------------------------


		.global		Square2x
        .thumb_func
        .align

Square2x:
			ADD R0, R0, R0
			B Square

// ----------------------------------------------------------
// int32_t Last(int32_ x) {return x + SquareRoot(x)}
//	This function returns the adds the number passed and the 
//	square root. Within the function the temp registor R5 is used to
//	secure the value in R0 (which is x) once the SquareRoot function
//	is called.
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

