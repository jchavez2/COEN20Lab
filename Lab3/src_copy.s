/*
	File Name: src_copy.s
	This progarm will run the functions: UseLDRB, UseLDRH, UseLDR.
	UseLDRD, LDRM in Assembly. These functions consist of the function
    calls LDR and STR which will load a rgesiter with a value then pass
    that value to a location into memory.

*/

        .syntax     unified
	.cpu        cortex-m4
        .text

// ----------------------------------------------------------
// void UseLDRB(void *dst, void *src)
// Description:
// Stores a 8-bit value (Byte) of src in R3 then post increments
// the address within R1. It then takes the value stored in R3 and
// stores it in the memory location pointed by the second pointer dst
// which is the address that stored in R0. This process is repeated 
// 512 times. 
// ----------------------------------------------------------

	.global		UseLDRB
        .thumb_func
        .align
UseLDRB:		
        .rept 512
            LDRB R3,[R1],1
            STRB R3,[R0],1
        .endr
        BX LR

// ----------------------------------------------------------
// void UseLDRH(void *dst, void *src)
// Description:
// Stores a 16-bit value (2 Bytes) of src in R3 then post increments
// the address within R1. It then takes the value stored in R3 and
// stores it in the memory location pointed by the second pointer dst
// which is the address that stored in R0. This process is repeated 
// 256 times. 
// ----------------------------------------------------------

        .global		UseLDRH
        .thumb_func
        .align
UseLDRH:
        .rept 256    
                LDRH R2,[R1],2
                STRH R2,[R0],2
        .endr
        BX LR
// ----------------------------------------------------------
// void UseLDR(void *dst, void *src)
// Description:
// Stores a 32-bit value (4 Bytes) of src in R3 then post increments
// the address within R1. It then takes the value stored in R3 and
// stores it in the memory location pointed by the second pointer dst
// which is the address that stored in R0. This process is repeated 
// 128 times. 
// ----------------------------------------------------------
        
        
        .global		UseLDR
        .thumb_func
        .align
UseLDR:
        .rept 128
            LDR R2,[R1],4
            STR R2,[R0],4
        .endr
            BX LR
        
// ----------------------------------------------------------
// void UseLDRD(void *dst, void *src)
// Description:
// Stores a 64-bit value (8 Bytes) of src in R3 then post increments
// the address within R1. It then takes the value stored in R3 and
// stores it in the memory location pointed by the second pointer dst
// which is the address that stored in R0. This process is repeated 
// 64 times. 
// ----------------------------------------------------------
        
        
        .global         UseLDRD	
        .thumb_func
        .align
UseLDRD:
        .rept 64
            LDRD R3,R2,[R1],8
            STRD R3,R2,[R0],8
        .endr
            BX LR
      

// ----------------------------------------------------------
// void UseLDM(void *dst, void *src)
// Description:
// Stores a 256-bit value (32 Bytes) starting at the address of src (stored in R1)
// and places each 32-bit partition into R2-R9. It then takes these values  and
// stores it in the memory location in the address stored in dst (stored in R0)
// This process is repeated 16 times. The repeat function rewrites the code 15 times 
// as well.
// ----------------------------------------------------------
        
        
        .global		UseLDM
        .thumb_func
        .align
UseLDM:
            PUSH {R4-R9}
            .rept 16
                LDMIA R1!,{R2-R9}
                STMIA R0!,{R2-R9}
            .endr
            POP {R4-R9}
            BX LR

.end

