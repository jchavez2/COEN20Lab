/*
*   File Name: lab_zellers_rule_src.s
*
*   Description: 
*
*/

    .syntax unified
    .cpu    cortex-m4
    .text

// ----------------------------------------------------------
// uint32_t Zeller1(uint32_t k, uint32_t m, uint32_t D, uint32_t C); 
//
//  Description:
//  This function calculates the day if week by reciving a date.
//  This function uses both Multiply and Divide Instrcutions.
// ----------------------------------------------------------

    .global Zeller1
    .thumb_func
    .align

Zeller1:    //R0 = k, R1 = m, R2 = D, R3 = C
        PUSH {R4-R6}
        LDR R4,=1
        LDR R5,=13
        LDR R6,=5
        ADD R0,R0,R2        //R0 <-- k + C
        MUL R5,R5,R1        //R5 <-- (13 * m) 
        SUB R5,R5,R4        //R5 <-- 13m - 1
        UDIV R5,R5,R6       //R5 <-- (13m - 1) / 5 
        ADD R0,R0,R5        //R0 <-- f + (13m - 1) / 5
        LSL R4,R4,2         //R4 <-- 1 << 2
        UDIV R5,R2,R4       //R5 <-- D / 4
        ADD R0,R0,R5        //R0 <-- f + (D / 4)
        UDIV R5,R3,R4       //R5 <-- C / 4
        ADD R0,R0,R5        //R0 <-- f + (C / 4)
        SUB R0,R0,R3,LSL 1  //R0 <-- f - (C << 1)
        LDR R3,=7
        SDIV R4,R0,R3       //R4 <-- f / 7
        MLS R0,R4,R3,R0     //R0 <-- f - 7(f / 7)
        CMP R0,0            //Check to see if the remainder is less than zero (r < 0)
        IT  LT              
        ADDLT R0,R0,7       //If it is than Add 7 ( r += 7)
        POP {R4-R6}
        BX LR





// ----------------------------------------------------------
// uint32_t Zeller2(uint32_t k, uint32_t m, uint32_t D, uint32_t C); 
//
//  Description:
//  This function calculates the day if week by reciving a date.
//  This function uses only uses Multiply Instrcutions and no Divide
//  Instructions.
// ----------------------------------------------------------

    .global Zeller2
    .thumb_func
    .align

Zeller2:    //R0 = k, R1 = m, R2 = D, R3 = C
        PUSH {R4-R6}
        LDR R4,=13
        LDR R5,=0xCCCCCCCD      //Constant to load into R5 to ensure quotion is correct.
        LDR R6,=1               //For 13m
        MUL R4,R1,R4            //R4 <-- Store: 13 * m
        SUB R4,R4,R6            //R4 <-- Store: 13m - 1
        UMULL R6,R5,R5,R4       //Calculation for (13m - 1) / 5
        LSR R4,R5,2
        ADD R0,R0,R4            //R0 <-- Store: [(13m - 1) / 5] + k
        ADD R0,R0,R2,LSR 2      //R0 <-- Store: f + (D>>2)
        ADD R0,R0,R3,LSR 2      //R0 <-- Store: f + (C>>2)
        ADD R0,R0,R2            //R0 <-- Store: f + D
        SUB R0,R0,R3,LSL 1      //R0 <-- Store: f - (C<<2)
        LDR R1,=613566757       //Calculation for the Magic Constant
        SMMUL R4,R0,R1          //R4 <-- MS half of Double Length Multiply
        LDR R3,=7               
        MLS R0,R4,R3,R0         //R0 <-- f - 7 * (f / 7)
        CMP R0,0                //Check to see if the remainder is less than zero (r < 0)
        IT  LT             
        ADDLT R0,R0,7           //If it is than Add 7 ( r += 7) 
        POP {R4-R6}
        BX LR

// ----------------------------------------------------------
// uint32_t Zeller3(uint32_t k, uint32_t m, uint32_t D, uint32_t C); 
//
//  Description:
//  This function calculates the day if week by reciving a date.
//  This function uses only uses Divide Instrcutions and no Multiply
//  Instructions.
// ----------------------------------------------------------

    .global Zeller3
    .thumb_func
    .align
    
Zeller3:
       PUSH {R4-R6}
        LDR R4,=1
        LDR R5,=13
        LDR R6,=5
        ADD R0,R0,R2        //R0 <-- k + D
        ADD R5,R1,R1,LSL 3  //R5 <-- 9m
        ADD R5,R5,R1,LSL 2  //R5 <-- 13m
        SUB R5,R5,R4        //R5 <-- 13m - 1
        UDIV R5,R5,R6       //R5 <-- (13m - 1) / 5 
        ADD R0,R0,R5        //R0 <-- f + ((13m - 1) / 5)
        LSL R4,R4,2         //R4 <-- 1 << 2
        UDIV R5,R2,R4       //R5 <-- D / 4
        ADD R0,R0,R5        //R0 <-- f + (D / 4)
        UDIV R5,R3,R4       //R5 <-- C / 4
        ADD R0,R0,R5        //R0 <-- f + (C / 4)
        SUB R0,R0,R3,LSL 1  //R0 <-- f - (C << 1)
        LDR R3,=7
        SDIV R4,R0,R3       //R4 <-- f / 7
        LSL R3,R4,3         //R3 <-- 8 * (f / 7)
        SUB R3,R3,R4        //R3 <-- 7 * (f / 7)
        SUB R0,R0,R3        //R0 <-- f - 7 * (f / 7)
        CMP R0,0            //Check to see if the remainder is less than zero (r < 0)
        IT  LT              
        ADDLT R0,R0,7       //If it is than Add 7 ( r += 7)
        POP {R4-R6}
        BX LR
        .end
