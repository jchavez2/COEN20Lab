/*
*   File Name: lab_floating_point_quads.s
*
*   Description: This lab teaches us floating points and how they can utlized
*   to make calculations for quadratics functions. 
*
*/

    .syntax unified
    .cpu    cortex-m4
    .text

// ----------------------------------------------------------
// float Discriminant(float a, float b, float c) ;
//
//  Description:
//  This function computes the value of the discriminant,
//  b^2-4ac. Function Root1 and Root2 should call this function.
// ----------------------------------------------------------

    .global Discriminant
    .thumb_func
    .align

Discriminant:   //S0 = a, S1 = b, S2 = c
                VMUL.F32 S4,S0,S2           //S4 = a * c
                VMOV S5,4.0           
                VMUL.F32 S4,S4,S5           //S4 = 4 * ac
                VMUL.F32 S5,S1,S1           //S5 = b * b = b^2
                VSUB.F32 S0,S5,S4           //S0 = (b^2) - 4ac
                BX LR

// ----------------------------------------------------------
// float Quadratic(float x, float a, float b, float c) ;
//
//  Description:
//  This function computes the quadratic ax^2 + bx + c
// ----------------------------------------------------------

    .global Quadratic
    .thumb_func
    .align

Quadratic:  //S0 = x, S1 = a, S2 = b, S3 = c
           VMUL.F32 S5,S0,S0    //S5 = x * x = x^2
           VMUL.F32 S1,S1,S5    //S1 = a * (x^2) = a(x^2)
           VMUL.F32 S2,S2,S0    //S2 = b * x = bx
           VADD.F32 S0,S1,S2    //S0 = (b * x) + a(x^2)
           VADD.F32 S0,S0,S3    //S0 = [a(x^2) + bx] + c
           BX LR



// ----------------------------------------------------------
// float Root1(float a, float b, float c) ;
//
//  Description:
//  This function computes the root using the quadratic formula.
//  However it adds -b to the squareroot of the Discriminant.
// ----------------------------------------------------------

    .global Root1
    .thumb_func
    .align

Root1:  //S0 = a, S1 = b, S2 = c
        PUSH {LR}
        VPUSH {S16,S17}
        VMOV S16,S0             //Moving (Copying) parameters to temp FP registers to preseve their values
        VMOV S17,S1
        BL Discriminant      
        // S0 = Discriminant, S16 = a, S17 = b, S18 = c
        VSQRT.F32 S0,S0         //S0 = (Discriminant ^ (1/2)) or Squareroot(Discriminant)    
        VNEG.F32 S17,S17        //S17 = -b
        VADD.F32 S0,S17,S0      //S0  = -b + (Squareroot(Discriminant))
        VADD.F32 S16,S16,S16    //S16 = a + a = 2a
        VDIV.F32 S0,S0,S16      //S0  = (-b + Squareroot(Discriminant)) / 2a
        VPOP {S16,S17}
        POP {PC}

// ----------------------------------------------------------
// float Root2(float a, float b, float c) ;
//
//  Description:
//  This function computes the root using the quadratic formula.
//  However it subtracts -b by the squareroot of the Discriminant.
// ----------------------------------------------------------

    .global Root2
    .thumb_func
    .align

Root2:  //S0 = a, S1 = b, S2 = c
        PUSH {LR}
        VPUSH {S16, S17}
        VMOV S16,S0             //Moving (Copying) parameters to Variable FP registers to preseve their values
        VMOV S17,S1
        BL Discriminant      
        // S0 = Discriminant, S16 = a, S17 = b, S18 = c
        VSQRT.F32 S0,S0         //S0 = (Discriminant ^ (1/2)) or Squareroot(Discriminant)            
        VNEG.F32 S17,S17        //S17 = -b
        VSUB.F32 S0,S17,S0      //S0  = -b - (Squareroot(Discriminant))
        VADD.F32 S16,S16,S16    //S16 = a + a = 2a
        VDIV.F32 S0,S0,S16      //S0  = (-b - Squareroot(Discriminant)) / 2a
        VPOP {S16, S17}
        POP {PC}
        .end
