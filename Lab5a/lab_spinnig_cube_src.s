/*
    File Name: lab_spinning_cube_src.s

*/


    .syntax     unified
    .cpu        cortex-m4
    .text

// ----------------------------------------------------------
// void MatrixMultiply(int32_t a[3][3], int32_t b[3][3], int32_t c[3][3])
//  Description:
//  This function preforms a matrix multiplication. It recieves
//  three 2-D arrays: a, b, and c, where a stores the product matrix 
//  of matrices b and c.
// ----------------------------------------------------------

    .global     MatrixMultiply
    .thumb_func
    .align

MatrixMultiply:
        PUSH {R4-R11,LR}
		MOV	R11,R0  //Preserve the address of the first element of array "a" in R11
		MOV	R8,R1   //Preserve the address of the first element of array "b" in R8
		MOV	R9,R2   //Preserve the address of the first element of array "c" in R9
		LDR	R10,=3
        LDR R4,=0   //R4=row <-- 0	
row:    CMP R4,3
        BGE rowDone
        LDR R5,=0       //R5=col <-- 0
     col:   CMP R5,3
            BGE colDone
            MLA R7,R10,R4,R5        //R7 <-- (3 * row + col) 
    		LDR R0,=0               //Consant "0" to load register a[row][col]
            STR R0,[R11,R7,LSL 2]   //a[row][col] <-- 0
            LDR R6,=0           //R6=k <-- 0 
        k:      CMP R6,3
                BGE kDone
                MLA R7,R10,R4,R5        //R7 <-- (3*row + col)
                LDR R0,[R11,R7,LSL 2]   //R0 <-- address of a[row][col]
                MLA R7,R10,R4,R6        //R7 <-- (3 * row + k)
                LDR R1,[R8, R7, LSL 2]  //R1 <-- address of b[row][k] 
                MLA R7,R10,R6,R5        //R7 <--(3*k + col)
                LDR R2,[R9, R7, LSL 2]  //R2 <-- address of c[k][col]
                BL  MultAndAdd
                MLA R7,R10,R4,R5        //R7 <-- (3*row + col)
                STR R0,[R11,R7,LSL 2]   //Return value of MultAndAdd --> memory location of a[row][col] (R11)
                ADD R6,R6,1             // Increment K 
			    B k
    kDone:  ADD R5,R5,1     // Increment col                
            B col
colDone: ADD R4,R4,1    // Increment row 
         B row
rowDone: MOV R0,R11
         MOV R1,R8
         MOV R2,R9
         POP {R4-R11,LR}    //Finished!
         BX LR

.end