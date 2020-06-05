/*
*   File Name: imp_divison_for_qsixteen_src.s
*
*   Description: This file works with imp_divison_for_qsixteen.c file to implement 
*   Q16 division for fixed-point reals.
*
*/

    .syntax unified
    .cpu cortex-m4
    .text

// ----------------------------------------------------------
//  Q16 Q16Divide(Q16 dividend, Q16 divisor);
//
//  Description:
//  This function preforms Q16 divison between the dividend and
//  the divisor.
// ----------------------------------------------------------

    .global Q16Divide
    .thumb_func
    .align

Q16Divide:  //R0 = dividend, R1 = divisor
            //R2 = Sign
            EOR R2,R0,R1        //R2 = (Q16) dividend ^ divisor 
            //Checking R0 and R1 if < 0
            EOR R3,R0,R0,ASR 31 //R0 = if(dividend < 0) ? ~dividend    : dividend
            ADD R0,R3,R0,LSR 31 //R0 = if(dividend < 0) ? dividend + 1 : dividend  
            EOR R3,R1,R1,ASR 31 //R1 = if(divisor < 0) ? ~divisor    : divisor
            ADD R1,R3,R1,LSR 31 //R1 = if(divisor < 0) ? divisor + 1 : divisor
            UDIV R3,R0,R1       //R3 = dividend / divisor = quoient
            MLS R0,R3,R1,R0     //R0 = dividend - (divisor * quoient) = remainder
            .rept 16
            LSL R3,R3,1         //R3 = quoient << 1
            LSL R0,R0,1         //R0 = remainder << 1
            CMP R0,R1           //Checking if(remainder >= divisor)
            ITT HS            
            SUBHS R0,R0,R1        //R0 = remainder -= divisor
            ADDHS R3,R3,1         //R3 = quoient++
            .endr
            EOR R3,R3,R2,ASR 31 //R3 = if(sign < 0) ? ~quoient    : quoient
            ADD R0,R3,R2,LSR 31 //R3 = if(sign < 0) ? quoient + 1 : quoient
            BX LR
            .end

            
            