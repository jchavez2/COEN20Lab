/*
*   File Name: lab6c_src.s
*
*/

    .syntax     unified
    .cpu        cortex-m4
    .text       

// ----------------------------------------------------------
// void PutNibble(void *nibbles, uint32_t which, uint32_t value);
//
//  Description:
//  This function takes a nibble (the status of cells in the sudoku game)
//  and puts a value inside.
// ----------------------------------------------------------  

    .global     PutNibble
    .thumb_func
    .align

PutNibble:
        PUSH {R4} 
        LSR R4,R1,1         //R3 <-- selects "which" byte in the nibbles array to select
        LDRB R3,[R0,R4]     //R3 <-- Contains the 8-bit Byte that contains one of two nibbles 
        AND R1,R1,1         //R1 <-- selects "which" 4-bit nibble to select
        CMP R1,1            //R1 <-- Compares the "which" bit in R4 and detemines if the value is even or odd
        ITE EQ          
        BFIEQ R3,R2,4,4     //If R1 is odd, then use BFI to correctly insert value in R2 into temp register R3
        BFINE R3,R2,0,4     //Else, Use BFI to correctly insert value in Rw into temp register R3.
        STRB R3,[R0,R4]     //R0 <-- Store back the updated vale into the nibbles array.
        POP {R4}
        BX LR



// ----------------------------------------------------------
// uint32_t GetNibble(void *nibbles, uint32_t which);
//
//  Description:
//  This fuction obtains a "nibble" (which is a 4-bit number) stored in an
//  array of nibbles.
// ----------------------------------------------------------  

    .global     GetNibble
    .thumb_func
    .align

GetNibble:
        LSR R2,R1,1     //R2 <-- selects "which" byte in the nibbles array to select
        LDRB R2,[R0,R2] //R2 <-- updates the nibbles address by R2 then stores the contents into R0
        AND R1,R1,1     //R1 <-- selects "which" 4-bit nibble to select
        CMP R1,1        //R1 <-- Compares the "which" bit in R4 and detemines if the value is even or odd
        ITE EQ        
        UBFXEQ R0,R2,4,4    //If R1 is odd, Store value (Register R2) from the second half of Nibble into R0
        UBFXNE R0,R2,0,4    //Else, Store value (Register R2) from the first half of nibble into R0
        BX LR
        .end
