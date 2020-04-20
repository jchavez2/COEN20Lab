/* File Name: lab1bits.c
*
*   Explanation: This program will Work with Lab1Main.c
*   to implement some of the functions within it.
*
*   Author: Jason Chavez
*/

#include <stdint.h>
#include <stdio.h>

/*
    Bits2Signed: This function assigns 8-digit binary array
    to a unsigned decimal number
*/
int32_t Bits2Signed(int8_t bits[8]) //Converts array of bits to signed int
{
    int32_t n = 0;
     if(bits[7] == 0)
     {
       for(int i = 6; i >= 0; --i)
        {
            n = (2 * n) + bits[i];
        }
     }
     else
    {
        int32_t temp = -128;
        for(int i = 6; i >= 0; --i)
        {
            n = (2 * n) + bits[i];
        }

        n = n + temp;
    }


    return n;
}

/*
    Bits2Unsigned: This function converts a 8-digit binary array to
    a unsigned decimal number.
*/
uint32_t Bits2Unsigned(int8_t bits[8]) //Convert array of bits to unsigned int
{
    int32_t n = 0;

    for(int i = 7; i >= 0; --i)
    {
        n = (2 * n) + bits[i];
    }

    return n;

}

/*
    Increment: This function increments a 8-digit binary number by one.
*/

void Increment(int8_t bits[8]) //Add 1 to value represented by bit pattern.
{
    int i = 0;
    while(bits[i] == 1)
    {
       bits[i] = 0;
        i++;
    }
    bits[i] = 1;
}

/*
    Unsigned2Bits: Assigns a unsigned decimal number to a
    8-digit binary number.

*/

void Unsigned2Bits(uint32_t n, int8_t bits[8]) //Opposite of Bits2Unsigned.
{
    int i;

        for(i = 0; i < 8; ++i)
        {
            bits[i] = n % 2;
            n = n / 2;
        }

}
