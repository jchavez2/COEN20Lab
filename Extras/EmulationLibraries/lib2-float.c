/*
	This code was written to support the book, "ARM Assembly for Embedded Applications",
	by Daniel W. Lewis. Permission is granted to freely share this software provided
	that this notice is not removed. This latest version of this software is available 
	for download from http://www.engr.scu.edu/~dlewis/book3.

	The following floating-point emulation was inspired by S. K. Raina, "FLIP: A 
	Floating-Point Library for Integer Processors", Laboratoire de l'Informatique du
	Parallélisme, Sept. 12, 2006. Although limited testing has been performed for
	correctness, no claim regarding accuracy or compliance with the IEEE standard 
	are made.
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define	CATCH_NANS				0				// set to 1 to catch exceptions (NaNs)

// Data Types
typedef int32_t 				float32_t ;

typedef struct	// IEEE 754 32-bit Single Precision Format
	{
	uint32_t					sig	:23 ;		// (1).XXX...XX
	uint32_t					exp	: 8 ;		// 0-255
	uint32_t					msb	: 1 ;		// 0=>pos, 1=>neg
	} ieee32_t ;

typedef struct	// Values extracted from representation
	{
	uint32_t					sig ;			// 1.XXX...XX (right justified)
	int32_t						exp ;			// 0-255 (may exceed range)
	uint32_t					msb ;			// 0=>pos, 1=>neg
	} unpacked_t ;

// Symbolic Constants
static const int32_t			IEEE32_1_PT_0	= (1 << 23) ;
static const int32_t			IEEE32_2_PT_0	= (2 << 23) ;
static const int32_t 			IEEE32_EXP_UND	= 0 ;
static const int32_t 			IEEE32_EXP_MIN	= 1 ;
static const int32_t			IEEE32_EXP_BIAS	= 127 ;
static const int32_t			IEEE32_EXP_INT	= 150 ;
static const int32_t 			IEEE32_EXP_MAX	= 254 ;
static const int32_t 			IEEE32_EXP_OVR	= 255 ;

// Private Global Variables
static float32_t				IEEE32_pZRO		= 0x00000000 ;
static float32_t				IEEE32_nZRO		= 0x80000000 ;
static float32_t				IEEE32_pINF		= 0x7F800000 ;
static float32_t				IEEE32_nINF		= 0xFF800000 ;
static float32_t				IEEE32_qNaN		= 0x7FC00000 ;

// Public Function Prototypes
float32_t						AddFloats(float32_t augend, float32_t addend) ;
float32_t						DivFloats(float32_t dividend, float32_t divisor) ;
int32_t							FloatToInt32(float32_t f32) ;
float32_t						Int32ToFloat(int32_t s32) ;
float32_t						MulFloats(float32_t multiplicand, float32_t multiplier) ;
float32_t						SqrtFloat(float32_t radical) ;
float32_t						SubFloats(float32_t minuend, float32_t subtrahend) ;

// Private Function Prototypes
static uint32_t					_AbsVal(int32_t s32) ;
static void						_AddSameSigns(unpacked_t *sum, unpacked_t *op1, unpacked_t *op2) ;
static void						_AddFloats(unpacked_t *sum, unpacked_t *op1, unpacked_t *op2) ;
static void						_AddDiffSigns(unpacked_t *sum, unpacked_t *op1, unpacked_t *op2) ;
static int32_t					_Category(float32_t f32) ;
static void						_CheckOverflow(unpacked_t *p) ;
static void						_CheckUnderflow(unpacked_t *p) ;
static uint32_t					_CountLeadingZeroes(uint32_t u32) ;
static void						_DivFloats(unpacked_t *quo, unpacked_t *op1, unpacked_t *op2) ;
static void						_MulFloats(unpacked_t *prd, unpacked_t *op1, unpacked_t *op2) ;
static void						_NormalizeDecrease(unpacked_t *p) ;
static void						_NormalizeIncrease(unpacked_t *p) ;
static float32_t				_PackFloat(unpacked_t *p) ;
static void						_ShiftRight(unpacked_t *p, uint32_t bits) ;
static float32_t *				_Special(float32_t * const table[6][6], float32_t op1, float32_t op2) ;
static void						_Round2Even(unpacked_t *p) ;
static void						_UnpackFloat(float32_t f32, unpacked_t *p) ;

#if CATCH_NANS != 0
static void						_Exception(char *op, float32_t op1, float32_t op2) ;
#endif

// Macros
#define	IEEE32(f32)				((ieee32_t *) &f32)
#define	IEEE32MAG(f32)			((f32) & 0x7FFFFFFF)

/* Public Functions ------------------------------------- */

float32_t AddFloats(float32_t augend, float32_t addend)
	{
	static float32_t addend_copy, augend_copy ;
	static float32_t * const table[6][6] =
		{
		{&IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_qNaN},
		{&IEEE32_qNaN, &IEEE32_pINF, &IEEE32_qNaN, &IEEE32_pINF, &IEEE32_pINF, &IEEE32_pINF},
		{&IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_nINF, &IEEE32_nINF, &IEEE32_nINF, &IEEE32_nINF},
		{&IEEE32_qNaN, &IEEE32_pINF, &IEEE32_nINF, &IEEE32_pZRO, &IEEE32_pZRO, &addend_copy},
		{&IEEE32_qNaN, &IEEE32_pINF, &IEEE32_nINF, &IEEE32_pZRO, &IEEE32_nZRO, &addend_copy},
		{&IEEE32_qNaN, &IEEE32_pINF, &IEEE32_nINF, &augend_copy, &augend_copy, NULL        }
		} ;
	unpacked_t op1, op2, sum ;
	float32_t *special ;

	if ((special = _Special(table, augend, addend)) != NULL)
		{
#if CATCH_NANS != 0
		if (*special == IEEE32_qNaN) _Exception("+/-", augend, addend) ;
#endif
		augend_copy = augend ;
		addend_copy = addend ;
		return *special ;
		}

	_UnpackFloat(augend, &op1) ;
	_UnpackFloat(addend, &op2) ;
	_AddFloats(&sum, &op1, &op2) ;
	return _PackFloat(&sum) ;
	}

float32_t DivFloats(float32_t dividend, float32_t divisor)
	{
	static float32_t ieee32_zro = 0x00000000 ;
	static float32_t ieee32_inf = 0x7F800000 ;
	static float32_t * const table[6][6] =
		{
		{&IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_qNaN},
		{&IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_pINF, &IEEE32_nINF, &ieee32_inf },
		{&IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_nINF, &IEEE32_pINF, &ieee32_inf },
		{&IEEE32_qNaN, &IEEE32_pZRO, &IEEE32_nZRO, &IEEE32_qNaN, &IEEE32_qNaN, &ieee32_zro },
		{&IEEE32_qNaN, &IEEE32_nZRO, &IEEE32_pZRO, &IEEE32_qNaN, &IEEE32_qNaN, &ieee32_zro },
		{&IEEE32_qNaN, &ieee32_zro,  &ieee32_zro,  &ieee32_inf,  &ieee32_inf,  NULL        }
		} ;
	unpacked_t op1, op2, quo ;
	float32_t *special ;

	quo.msb = (dividend ^ divisor) < 0 ;
	if ((special = _Special(table, dividend, divisor)) != NULL)
		{
#if CATCH_NANS != 0
		if (*special == IEEE32_qNaN) _Exception("/", dividend, divisor) ;
#endif
		IEEE32(ieee32_zro)->msb = quo.msb ;
		IEEE32(ieee32_inf)->msb = quo.msb ;
		return *special ;
		}

	_UnpackFloat(dividend, &op1) ;
	_UnpackFloat(divisor,  &op2) ;
	_DivFloats(&quo, &op1, &op2) ;
	return _PackFloat(&quo) ;
	}

int32_t FloatToInt32(float32_t f32)
	{
	const float32_t int32_max = 0x4F000000 ;
	uint32_t exp, mag ;
	int32_t sig ;

	mag = IEEE32MAG(f32) ;
	if (mag < (IEEE32_EXP_BIAS << 23))	return 0 ;	// fraction
	if (mag > (IEEE32_EXP_OVR  << 23))	return 0 ;	// NaN
	if (mag >= int32_max)							// overflow
		{
		return INT32_MAX ^ ((f32 < 0) ? -1 : 0) ;
		}

	exp = mag >> 23 ;
	sig = IEEE32_1_PT_0 | IEEE32(f32)->sig ;
	if (exp > IEEE32_EXP_INT)	sig <<= exp - IEEE32_EXP_INT ;
	else						sig >>= IEEE32_EXP_INT - exp ;

	return (f32 < 0) ? -sig : +sig ;
	}

float32_t Int32ToFloat(int32_t s32)
	{
	const float32_t flpt_zero = 0x00000000 ;
	const float32_t int32_min = 0xCF000000 ;
	unpacked_t f32 ;
	uint32_t nlz ;

	if (s32 == 0)			return flpt_zero ;
	if (s32 == INT32_MIN)	return int32_min ;

	f32.exp = IEEE32_EXP_INT + 3 ; // +3 for rounding bits
	f32.msb = s32 < 0 ;
	f32.sig = _AbsVal(s32) ;

	nlz = _CountLeadingZeroes(f32.sig) ;
	f32.sig <<= nlz ;
	f32.exp  -= nlz ;

	_ShiftRight(&f32, 8 - 3) ;	// Keep 3 rounding bits
	_Round2Even(&f32) ;
	return _PackFloat(&f32) ;
	}

float32_t MulFloats(float32_t multiplicand, float32_t multiplier)
	{
	static float32_t ieee32_zro = 0x00000000 ;
	static float32_t ieee32_inf = 0x7F800000 ;
	static float32_t * const table[6][6] =
		{
		{&IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_qNaN},
		{&IEEE32_qNaN, &IEEE32_pINF, &IEEE32_nINF, &IEEE32_qNaN, &IEEE32_qNaN, &ieee32_inf },
		{&IEEE32_qNaN, &IEEE32_nINF, &IEEE32_pINF, &IEEE32_qNaN, &IEEE32_qNaN, &ieee32_inf },
		{&IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_pZRO, &IEEE32_nZRO, &ieee32_zro },
		{&IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_qNaN, &IEEE32_nZRO, &IEEE32_pZRO, &ieee32_zro },
		{&IEEE32_qNaN, &ieee32_inf,  &ieee32_inf,  &ieee32_zro,  &ieee32_zro,  NULL        }
		} ;
	unpacked_t op1, op2, prd ;
	float32_t *special ;

	if ((special = _Special(table, multiplicand, multiplier)) != NULL)
		{
#if CATCH_NANS != 0
		if (*special == IEEE32_qNaN) _Exception("*", multiplicand, multiplier) ;
#endif
		IEEE32(ieee32_zro)->msb = (multiplicand ^ multiplier) < 0  ;
		IEEE32(ieee32_inf)->msb = (multiplicand ^ multiplier) < 0  ;
		return *special ;
		}

	_UnpackFloat(multiplicand, &op1) ;
	_UnpackFloat(multiplier,   &op2) ;
	_MulFloats(&prd, &op1, &op2) ;
	return _PackFloat(&prd) ;
	}

float32_t SqrtFloat(float32_t radical)
	{
	static const int32_t ACCURACY = 127 - 17 ;	// 2^-17 = ~10^-5 = .00001
	static float32_t * const table[6] =
		{&IEEE32_qNaN, &IEEE32_pINF, &IEEE32_qNaN, &IEEE32_pZRO, &IEEE32_nZRO, NULL} ;
	unpacked_t sqrt, positive ;
    float32_t *special ;
	uint32_t k ;

	if (radical < 0) return IEEE32_qNaN ;
	if ((special = table[_Category(radical)]) != NULL) return *special ;

	_UnpackFloat(radical, &sqrt) ;
	positive = sqrt ;
	sqrt.exp = (sqrt.exp - IEEE32_EXP_BIAS) / 2 + IEEE32_EXP_BIAS ;

	for (k = 0; k < 100; k++)
		{
		unpacked_t negative, temp, error ;

		negative = positive ;
		negative.msb = 1 ;
		_MulFloats(&temp, &sqrt, &sqrt) ;
		_AddFloats(&error, &temp, &negative) ;
		if (error.exp <= ACCURACY) break ;
		_DivFloats(&temp, &positive, &sqrt) ;
		_AddFloats(&sqrt, &temp, &sqrt) ;
		sqrt.exp-- ;	// sqrt /= 2.0
    	}

    return _PackFloat(&sqrt) ;
	}

float32_t SubFloats(float32_t minuend, float32_t subtrahend)
	{
	return AddFloats(minuend, subtrahend ^ (1 << 31)) ;
	}

/* Private Functions ------------------------------------- */
static uint32_t _AbsVal(int32_t x)
	{
    int32_t t = (x < 0) ? -1 : 0 ;
  	return (x ^ t) - t ;
	}

static void _AddDiffSigns(unpacked_t *sum, unpacked_t *op1, unpacked_t *op2)
	{
	int32_t sig1, sig2, sig ;
	uint32_t nlz ;

    sig1 = op1->sig - 2*op1->msb*op1->sig ;
    sig2 = op2->sig - 2*op2->msb*op2->sig ;
	if ((sig = sig1 + sig2) == 0)
		{
		sum->msb = sum->exp = sum->sig = 0 ;
		return ;
		}

	sum->msb = sig < 0 ;
	sig = _AbsVal(sig) ;

	nlz = _CountLeadingZeroes(sig) - 5 ;
	sum->exp = op1->exp - nlz ;
	sum->sig = sig << nlz ;

	_NormalizeDecrease(sum) ;
	_CheckUnderflow(sum) ;
	_Round2Even(sum) ;
	}

static void _AddSameSigns(unpacked_t *sum, unpacked_t *op1, unpacked_t *op2)
	{
	sum->msb = op1->msb ;
	sum->sig = op1->sig + op2->sig ;
	sum->exp = op1->exp ;

	_NormalizeIncrease(sum) ;
	_CheckOverflow(sum) ;
	_CheckUnderflow(sum) ;
	_Round2Even(sum) ;
	}

static void _AddFloats(unpacked_t *sum, unpacked_t *op1, unpacked_t *op2)
	{
	op1->sig <<= 3 ;
	op2->sig <<= 3 ;

	if (op2->exp > op1->exp)	_ShiftRight(op1, op2->exp - op1->exp) ;
	else						_ShiftRight(op2, op1->exp - op2->exp) ;

	if (op1->msb == op2->msb)	_AddSameSigns(sum, op1, op2) ;
	else						_AddDiffSigns(sum, op1, op2) ;
	}

static int32_t _Category(float32_t f32)
	{
    uint32_t exp = IEEE32(f32)->exp ;
    if ((exp - 1) < IEEE32_EXP_MAX)	return 5 ;		// Normal
    if (IEEE32(f32)->sig != 0)		return 5*!exp ;	// SubNormal or NaN
    return (f32 < 0) + 1 + 2*!exp ;					// +/-0 or +/-Infinity
	}

static void _CheckOverflow(unpacked_t *p)
	{
	if (p->exp > IEEE32_EXP_MAX)
		{
		p->exp = IEEE32_EXP_OVR ;
		p->sig = 0 ;
		}
	}

static void _CheckUnderflow(unpacked_t *p)
	{
	if (p->exp < IEEE32_EXP_MIN)
		{
		_ShiftRight(p, IEEE32_EXP_MIN - p->exp) ;
		p->exp = IEEE32_EXP_UND ;
		}
	}

static uint32_t _CountLeadingZeroes(uint32_t u32)
	{
#if defined (__thumb__)
	uint32_t nlz ;

	__asm(
		"CLZ	%[nlz],%[u32]"
	:	[nlz] "=r" (nlz)
	:	[u32] "r"  (u32)
	) ;

	return nlz ;
#else
	uint32_t nlz, y ;

	nlz = 32 ;
	if (y = u32 >> 16) nlz -= 16, u32 = y ;
	if (y = u32 >>  8) nlz -=  8, u32 = y ;
	if (y = u32 >>  4) nlz -=  4, u32 = y ;
	if (y = u32 >>  2) nlz -=  2, u32 = y ;
	return (u32 >> 1) ? nlz - 2 : nlz - u32 ;
#endif
	}

static void _DivFloats(unpacked_t *quo, unpacked_t *op1, unpacked_t *op2)
	{
	uint32_t sig, exp ;

	exp = op1->exp - op2->exp + IEEE32_EXP_BIAS ;
	sig = (uint32_t) (((uint64_t) op1->sig << 32) / (op2->sig << 6)) ;
	if ((op1->sig - sig * op2->sig) != 0) sig |= 1 ;
	if (sig < (IEEE32_1_PT_0 << 3)) sig <<= 1, exp-- ;
	quo->sig = sig ;
	quo->exp = exp ;

	_CheckOverflow(quo) ;
	_CheckUnderflow(quo) ;
	_Round2Even(quo) ;
	}

static void _MulFloats(unpacked_t *prd, unpacked_t *op1, unpacked_t *op2)
	{
	uint64_t uProd64 ;

	uProd64 = (uint64_t) (op1->sig << 6) * (uint64_t) (op2->sig << 6) ;	
	prd->sig = (uProd64 >> 32) | ((uint32_t) uProd64 != 0) ;
	prd->exp = op1->exp + op2->exp - IEEE32_EXP_BIAS ;
	prd->msb = op1->msb != op2->msb ;

	_NormalizeIncrease(prd) ;
	_CheckOverflow(prd) ;
	_CheckUnderflow(prd) ;
	_Round2Even(prd) ;
	}

static void _NormalizeDecrease(unpacked_t *p)
	{
	while (p->sig < (IEEE32_1_PT_0 << 3) && p->exp > IEEE32_EXP_UND)
		{
		p->sig <<= 1 ;
		p->exp-- ;
		}
	}

static void _NormalizeIncrease(unpacked_t *p)
	{
	if (p->sig >= (IEEE32_2_PT_0 << 3)) _ShiftRight(p, 1) ;
	}

static float32_t _PackFloat(unpacked_t *p)
	{
	return (p->msb << 31) | (p->exp << 23) | (p->sig & 0x7FFFFF) ;
	}

static void _Round2Even(unpacked_t *p)
	{
	static const uint8_t rounding[] = {0,0,0,0,0,8,8,8,0,0,0,0,8,8,8,8} ;
	if (p->sig >= ((IEEE32_2_PT_0 - 1) << 3) + 4) p->exp++ ;
	p->sig = (p->sig + rounding[p->sig & 0xF]) >> 3 ;
	}

static void _ShiftRight(unpacked_t *p, uint32_t bits)
	{
	if (bits < 24+3)
		{
		uint32_t lost = (1 << bits) - 1 ;
		p->sig = (p->sig >> bits) | ((p->sig & lost) != 0) ;
		}
	else p->sig = 1 ;
	p->exp += bits ;
	}

static float32_t *_Special(float32_t * const table[6][6], float32_t op1, float32_t op2)
	{
	uint32_t row, col ;

	row = _Category(op1) ;
	col = _Category(op2) ;
	return (row + col != 10) ? table[row][col] : NULL ;
	}

static void _UnpackFloat(float32_t f32, unpacked_t *p)
	{
	p->msb = IEEE32(f32)->msb ;
	p->sig = IEEE32(f32)->sig ;
	p->exp = IEEE32(f32)->exp ;

	if (p->exp >= IEEE32_EXP_MIN) p->sig |= IEEE32_1_PT_0 ;
	else if (p->sig != 0) // SubNormal
		{
		uint32_t nbits = _CountLeadingZeroes(p->sig) - 8 ;
		p->sig <<= nbits ;
		p->exp = IEEE32_EXP_MIN - nbits ;
		}
	}

#if CATCH_NANS != 0
static void _Exception(char *op, float32_t op1, float32_t op2)
	{
	printf("Exception: %+e %s %+e = NaN!\n", *((float *) &op1), op, *((float *) &op2)) ;
	exit(0) ;
	}
#endif

