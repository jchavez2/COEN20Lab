/*
    File Name: src_copy.s
    
*/

        .syntax     unified
	.cpu        cortex-m4
        .text

// ----------------------------------------------------------
// int32_t MxPlusB(int32_t x, int32_t mtop, int32_t mbtm, int32_t b)
// Description:
//  This function calculates the quiotent and rounding for the opreation
//  mtop/mbtm. First the rounding is processed with a number of opertions
//  then the quiotent is calculated by taking the dvnd (mtop) and adding the
//  rounding then divding it by the dvsr (mbtm). Once the quotient is processed,
//  it is used as the slope to the liner function given by the function. 
// ----------------------------------------------------------

	.global		MxPlusB
        .thumb_func
        .align
MxPlusB:
        // Register Deatils:
        // R0 = x, R1 = mtop, R2 = mbtm, R3= b, R4=R5="Unloaded Varible Register"
        PUSH {R4,R5}

        //Calculate "mtop = x*btm":
        MUL  R1,R1,R0 // R1 <-- R1*R0

        //Caculate Rounding:
        MUL  R4,R1,R2 // R4 <-- R4*R2
        ASR  R4,R4,31 // R4 <-- R4<<31 "Shift bits right"
        MUL  R4,R4,R2 // R4 <-- R4*R2
        LSL  R4,R4,1  // R4 <-- R4>>1  "Shift bits left" 
        ADD R4,R4,R2  // R4 <-- R4+R2
        LDR R5,=2   
        SDIV R4,R4,R5 // R4 <-- R4/R2

        //Caculate Quiotent:
        ADD R4,R1,R4  // R4 <-- R4+R1
        SDIV R4,R4,R2 // R4 <-- R4/R2
        ADD R0,R4,R3  // R0 <-- R4+R3
        
        POP {R4,R5}
        BX LR

.end


