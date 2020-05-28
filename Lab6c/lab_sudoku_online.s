

                    .syntax unified
                    .cpu    cortex-m4
                    .text

//Function Header
                    .global PutNibble
                    .thumb_func
                    .align
PutNibble:          LSR     R3,R1,1         //Shifts index to update the Byte to select
                    ADD     R0,R0,R3        //Adds index to R0
                    PUSH    {R4}            //Pushes R4
                    AND     R4,R1,1         //Checks if R1 is odd or even
                    LDR     R3,=15          //Loads 15
                    LDRB    R1,[R0]         //Loads the address of R0
                    CMP     R4,1            //Compares
                    ITE     EQ              //IT Block
                    LSLEQ   R2,R2,4         //Shifts R2 left if equal. Shifting the value! Genuis!
                    LSLNE   R3,R3,4         //Shifts R3 left if equal
                    AND     R1,R1,R3        //Copies the matching values
                    ORR     R1,R1,R2        //Copies any 1
                    STRB    R1,[R0]         //Stores
                    POP     {R4}            //Pops R4
                    BX      LR              //Return

                    .global GetNibble      //Function Header
                    .thumb_func
                    .align
GetNibble:          LSR     R3,R1,1         //Loads
                    ADD     R0,R0,R3        //Adds index
                    AND     R2,R1,1         //Copies matching 1s
                    LDR     R3,=15          //Loads 15
                    LDRB    R1,[R0]         //Loads address of R0
                    CMP     R2,1            //Comapres
                    ITE     EQ              //IT Block
                    LSREQ   R1,R1,4         //Shifts if equal
                    ANDNE   R1,R1,R3        //Copies matching bits
                    MOV     R0,R1           //Copies
                    BX      LR              //Return

                    .end    //Ends it all
