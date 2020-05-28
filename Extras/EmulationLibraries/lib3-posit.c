/*
	This code was written to support the book, "ARM Assembly for Embedded Applications",
	by Daniel W. Lewis. Permission is granted to freely share this software provided
	that this notice is not removed. This latest version of this software is available 
	for download from http://www.engr.scu.edu/~dlewis/book3.
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define	POSIT32_ES				2			// number of exponent bits
#define	CATCH_NANS				1			// set to 1 to catch exceptions

typedef int32_t					posit32_t ;

typedef struct	// Interpreted comppnents of real number
	{
    uint32_t					sig ;		// 1.XXX...XX (left justified)
    int32_t						exp ;		// signed 2's comp
    uint32_t					msb ;		// 0=>pos, 1=>neg
	} unpacked_t ;

typedef struct	// IEEE 754 32-bit Single Precision Format
	{
	uint32_t					sig	:23 ;	// (1).XXX...XX
	uint32_t					exp	: 8 ;	// 0-255
	uint32_t					msb	: 1 ;	// 0=>pos, 1=>neg
	} ieee32_t ;

typedef struct	// IEEE 754 64-bit Double Precision Format
	{
	uint64_t					sig	:52 ;	// (1).XXX...XX
	uint64_t					exp	:11 ;	// 0-2047
	uint64_t					msb	: 1 ;	// 0=>pos, 1=>neg
	} ieee64_t ;

// Public Function Prototypes
posit32_t						AddPosits(posit32_t augend, posit32_t addend) ;
posit32_t						DivPosits(posit32_t dividend, posit32_t divisor) ;
posit32_t						Double2Posit(double double64) ;
posit32_t						Float2Posit(float float32) ;
posit32_t						Int32ToPosit(int32_t) ;
posit32_t						MulPosits(posit32_t multiplicand, posit32_t multiplier) ;
int32_t							PositToInt32(int32_t) ;
float							Posit2Float(posit32_t posit) ;
double							Posit2Double(posit32_t posit) ;
posit32_t						SqrtPosit(posit32_t radical) ;
posit32_t						SubPosits(posit32_t minuend, posit32_t subtrahend) ;

// Private Function Prototypes
static uint32_t					_AbsVal(int32_t x) ;
static void						_AddSameSigns(unpacked_t *sum, unpacked_t *op1, unpacked_t *op2) ;
static void						_AddDiffSigns(unpacked_t *sum, unpacked_t *op1, unpacked_t *op2) ;
static void						_AddPosits(unpacked_t *sum, unpacked_t *op1, unpacked_t *op2) ;
static uint32_t					_CountLeadingZeroes(uint32_t uint32) ;
static void						_DivPosits(unpacked_t *quo, unpacked_t *op1, unpacked_t *op2) ;
static void						_MulPosits(unpacked_t *prd, unpacked_t *op1, unpacked_t *op2) ;
static posit32_t				_PackPosit(unpacked_t *unpacked) ;
static void						_ShiftRight(unpacked_t *unpacked, uint32_t bits) ;
static void						_UnpackPosit(posit32_t posit32, unpacked_t *unpacked) ;

#if CATCH_NANS != 0
static void						_Exception(char *op, posit32_t op1, posit32_t op2) ;
#endif

// Private Symbolic Constants
static const int32_t 			IEEE32_EXP_UND	= 0 ;
static const int32_t 			IEEE32_EXP_MIN	= 1 ;
static const int32_t			IEEE32_EXP_BIAS	= 127 ;
static const int32_t 			IEEE32_EXP_MAX	= 254 ;
static const int32_t 			IEEE32_EXP_OVR	= 255 ;
static const uint32_t			IEEE32_INF		= 0x7F800000  ;

static const int64_t 			IEEE64_EXP_UND	= 0 ;
static const int64_t 			IEEE64_EXP_MIN	= 1 ;
static const int64_t			IEEE64_EXP_BIAS	= 1023 ;
static const int64_t 			IEEE64_EXP_MAX	= 2046 ;
static const int64_t 			IEEE64_EXP_OVR	= 2047 ;
static const uint64_t			IEEE64_ZRO		= 0x0000000000000000ULL ;
static const uint64_t			IEEE64_INF		= 0x7FF0000000000000ULL ;

static const posit32_t			POSIT32_ZRO		= 0x00000000 ;
static const posit32_t			POSIT32_INF		= 0x80000000 ;

#define	MINEXP					(-(30-POSIT32_ES)*(1<<POSIT32_ES))
#define	MAXEXP					( (30-POSIT32_ES)*(1<<POSIT32_ES)-1)
#define	IEEE32(f32)				((ieee32_t *) &f32)
#define	IEEE64(f64)				((ieee64_t *) &f64)
#define	MAX(a, b)				(((a) > (b)) ? (a) : (b))
#define	MIN(a, b)				(((a) < (b)) ? (a) : (b))
#define	IN(x, min, max)			((uint32_t) ((x)-(min)) <= ((max)-(min)))

/* Public Functions ------------------------------------- */

int32_t PositToInt32(posit32_t posit)
	{
	unpacked_t unpacked ;

	_UnpackPosit(posit, &unpacked) ;

	if (unpacked.exp <   0)	return 0 ;
	if (unpacked.exp >= 31)
		{
		return INT32_MAX ^ ((posit < 0) ? -1 : 0) ;
		}

	unpacked.sig >>= 31 - unpacked.exp ;
	return unpacked.msb ? -unpacked.sig : unpacked.sig ;
	}

posit32_t Int32ToPosit(int32_t s32)
	{
	static unpacked_t int32_min = {1, 31, 0x80000000} ;
	unpacked_t unpacked ;
	uint32_t nlz ;

	if (s32 == INT32_MIN) return _PackPosit(&int32_min) ;

	unpacked.msb = s32 < 0 ;
	unpacked.sig = _AbsVal(s32) ;
	nlz = _CountLeadingZeroes(unpacked.sig) ;
	unpacked.sig <<= nlz ;
	unpacked.exp = 31 - nlz ;
	return _PackPosit(&unpacked) ;
	}

float Posit2Float(posit32_t posit)
	{
	unpacked_t unpacked ;
	float float32 ;
	int32_t exp ;

	if (posit == POSIT32_ZRO) return *((float *) &posit) ;
	if (posit == POSIT32_INF) return *((float *) &IEEE32_INF) ;

	_UnpackPosit(posit, &unpacked) ;

	IEEE32(float32)->msb = unpacked.msb ;
	exp = unpacked.exp + IEEE32_EXP_BIAS ;
	if (IN(exp, IEEE32_EXP_MIN, IEEE32_EXP_MAX))
		{
		IEEE32(float32)->exp = exp ;
		IEEE32(float32)->sig = unpacked.sig >> 8 ;
		}
	else if (exp > IEEE32_EXP_MAX) // overflow
		{
		IEEE32(float32)->exp = IEEE32_EXP_OVR ;
		IEEE32(float32)->sig = 0 ;
		}
	else // underflow
		{
		uint32_t bits = (IEEE32_EXP_MIN + 8) - exp ;
		if (bits >= 32) IEEE32(float32)->sig = 0 ;
		else IEEE32(float32)->sig = unpacked.sig >> bits ;
		IEEE32(float32)->exp = IEEE32_EXP_UND ;
		}

	return float32 ;
	}

double Posit2Double(posit32_t posit)
	{
	unpacked_t unpacked ;
	double double64 ;
	int32_t exp ;

	if (posit == POSIT32_ZRO) return *((double *) &IEEE64_ZRO) ;
	if (posit == POSIT32_INF) return *((double *) &IEEE64_INF) ;

	_UnpackPosit(posit, &unpacked) ;

	IEEE64(double64)->msb = unpacked.msb ;
	exp = unpacked.exp + IEEE64_EXP_BIAS ;
	if (IN(exp, IEEE64_EXP_MIN, IEEE64_EXP_MAX))
		{
		IEEE64(double64)->exp = exp ;
		IEEE64(double64)->sig = ((uint64_t) unpacked.sig) << 21 ;
		}
	else if (exp > IEEE64_EXP_MAX) // overflow
		{
		IEEE64(double64)->exp = IEEE64_EXP_OVR ;
		IEEE64(double64)->sig = 0 ;
		}
	else // if (exp < IEEE64_EXP_MIN) // underflow
		{
		uint64_t sig  = (uint64_t) unpacked.sig << 20 ;
		IEEE64(double64)->sig = sig >> (IEEE64_EXP_MIN - exp) ;
		IEEE64(double64)->exp = IEEE64_EXP_UND ;
		}

	return double64 ;
	}

posit32_t Float2Posit(float float32)
	{
	unpacked_t unpacked ;
	int32_t mag ;
	uint32_t sig ;

	mag = *((uint32_t *) &float32) & 0x7FFFFFFF ;
	if (mag == 0) return POSIT32_ZRO ;
	if (mag >= (IEEE32_EXP_OVR << 23)) return POSIT32_INF ;

	unpacked.msb = *((int32_t *) &float32) < 0 ;
	sig = mag & 0x7FFFFF ;

	if (mag >= (IEEE32_EXP_MIN << 23))
		{
		unpacked.sig = (1 << 31) | (sig << 8) ;
		unpacked.exp = IEEE32(float32)->exp - IEEE32_EXP_BIAS ;
		}
	else // (unpacked->exp == IEEE32_EXP_UND) SubNormal
		{
		uint32_t  nlz = _CountLeadingZeroes(sig) ;
		unpacked.sig = sig << nlz ;
		if (unpacked.sig == 0) unpacked.exp = IEEE32_EXP_MIN ;
		else unpacked.exp = (IEEE32_EXP_MIN - IEEE32_EXP_BIAS + 8) - nlz ;
		}

	return _PackPosit(&unpacked) ;
	}

posit32_t Double2Posit(double double64)
	{
	union {uint32_t u32[2]; uint64_t u64;} packed, sig ;
	unpacked_t unpacked ;
	int64_t mag ;

	mag = ((*((uint64_t *) &double64)) << 1) >> 1 ;
	if (mag == 0) return POSIT32_ZRO ;

	packed.u64 = *((uint64_t *) &double64) ;
	if ((packed.u32[1] << 1) >= (IEEE64_EXP_OVR << 21)) return POSIT32_INF ;

	unpacked.msb = packed.u32[1] >> 31 ;
	sig.u32[0] = packed.u32[0] ;
	sig.u32[1] = packed.u32[1] & 0xFFFFF ;

	if (IEEE64(double64)->exp >= IEEE64_EXP_MIN)
		{
		sig.u64 <<= 11 ;
		unpacked.sig = (1 << 31) | sig.u32[1] ;
		unpacked.exp = IEEE64(double64)->exp - IEEE64_EXP_BIAS ;
		}
	else // (unpacked->exp == IEEE32_EXP_UND) SubNormal
		{
		uint32_t nlz ;

		if (sig.u32[1] == 0)	nlz = _CountLeadingZeroes(sig.u32[0]) + 32 ;
		else					nlz = _CountLeadingZeroes(sig.u32[1]) ;

		unpacked.sig = (uint32_t) ((sig.u64 << nlz) >> 32) ;
		if (unpacked.sig == 0) unpacked.exp = IEEE64_EXP_MIN ;
		else unpacked.exp = (IEEE64_EXP_MIN - IEEE64_EXP_BIAS + 11) - nlz ;
		}

	return _PackPosit(&unpacked) ;
	}

posit32_t AddPosits(posit32_t augend, posit32_t addend)
	{
	unpacked_t op1, op2, sum ;

	if (augend == POSIT32_INF)
		{
#if CATCH_NANS != 0
		if (addend == POSIT32_INF) _Exception("+", augend, addend) ;
#endif
		return augend ;
		}
	if (augend == POSIT32_ZRO) return addend ;
	if (addend == POSIT32_INF) return addend ;
	if (addend == POSIT32_ZRO) return augend ;

	_UnpackPosit(augend, &op1) ;
	_UnpackPosit(addend, &op2) ;
	_AddPosits(&sum, &op1, &op2) ;
	return _PackPosit(&sum) ;
	}

posit32_t MulPosits(posit32_t multiplicand, posit32_t multiplier)
	{
	static unpacked_t prd = {0, 0, 0} ;
	unpacked_t op1, op2 ;

#if CATCH_NANS != 0
	if (((multiplicand | multiplier) << 1) == 0 && ((multiplicand ^ multiplier) < 0))
		{
		_Exception("*", multiplicand, multiplier) ;
		}
#endif
	if ((multiplicand << 1) == 0) return multiplicand ;
	if ((multiplier   << 1) == 0) return multiplier ;

	_UnpackPosit(multiplicand, &op1) ;
	_UnpackPosit(multiplier,   &op2) ;
	_MulPosits(&prd, &op1, &op2) ;
	return _PackPosit(&prd) ;
	}

posit32_t DivPosits(posit32_t dividend, posit32_t divisor)
	{
	unpacked_t op1, op2, quo ;

#if CATCH_NANS != 0
	if ((dividend | divisor) == POSIT32_ZRO) _Exception("/", dividend, divisor) ;
#endif
	if ((dividend << 1) == 0) return dividend ;
	if ((divisor  << 1) == 0) return POSIT32_INF ;

	_UnpackPosit(dividend, &op1) ;
	_UnpackPosit(divisor,  &op2) ;
	_DivPosits(&quo, &op1, &op2) ;
	return _PackPosit(&quo) ;
	}

posit32_t SqrtPosit(posit32_t radical)
	{
	static const int32_t ACCURACY = -17 ;	// 2^-17 = ~10^-5 = .00001
	unpacked_t sqrt, positive ;
	uint32_t k ;

	if (radical < 0) return POSIT32_INF ;

	_UnpackPosit(radical, &sqrt) ;
	positive = sqrt ;
	sqrt.exp /= 2 ;

	for (k = 0; k < 100; k++)
		{
		unpacked_t negative, temp, error ;

		negative = positive ;
		negative.msb = 1 ;
		_MulPosits(&temp, &sqrt, &sqrt) ;
		_AddPosits(&error, &temp, &negative) ;
		if (error.exp <= ACCURACY) break ;
		_DivPosits(&temp, &positive, &sqrt) ;
		_AddPosits(&sqrt, &temp, &sqrt) ;
		sqrt.exp-- ;	// sqrt /= 2.0
    	}

    return _PackPosit(&sqrt) ;
	}

posit32_t SubPosits(posit32_t minuend, posit32_t subtrahend)
	{
#if CATCH_NANS != 0
	if (minuend == POSIT32_INF && subtrahend == POSIT32_INF) _Exception("-", minuend, subtrahend) ;
#endif
	return AddPosits(minuend, -subtrahend) ;
	}

/* Private Functions ------------------------------------- */

static uint32_t _AbsVal(int32_t x)
	{
    int32_t t = (x < 0) ? -1 : 0 ;
  	return (x ^ t) - t ;
	}

static void _AddDiffSigns(unpacked_t *sum, unpacked_t *op1, unpacked_t *op2)
	{
	uint32_t nlz, sig, msb ;

	sig = op1->sig - op2->sig ;
	if (sig < op1->sig)
		{
		msb = op1->msb ;
		}
	else if (sig != 0)
		{
		sig = -sig ;
		msb = op2->msb ;
		}
	else
		{
		sum->msb = sum->exp = sum->sig = 0 ;
		return ;
		}

	nlz = _CountLeadingZeroes(sig) ;
	sum->msb = msb ;
	sum->sig = sig << nlz ;
	sum->exp = op1->exp - nlz ;
	}

static void _AddSameSigns(unpacked_t *sum, unpacked_t *op1, unpacked_t *op2)
	{
	sum->msb = op1->msb ;
	sum->exp = op1->exp ;
	sum->sig = (op1->sig >> 1) + (op2->sig >> 1) ;

	if ((int32_t) sum->sig < 0)	sum->exp++ ;
	else						sum->sig <<= 1 ;
	}

static void _AddPosits(unpacked_t *sum, unpacked_t *op1, unpacked_t *op2)
	{
	void (*func)(unpacked_t *, unpacked_t *, unpacked_t *) ;

	if (op2->exp > op1->exp) _ShiftRight(op1, op2->exp - op1->exp) ;
	else					 _ShiftRight(op2, op1->exp - op2->exp) ;

	func = (op1->msb == op2->msb) ? _AddSameSigns : _AddDiffSigns ;
	(*func)(sum, op1, op2) ;
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

static void _DivPosits(unpacked_t *quo, unpacked_t *op1, unpacked_t *op2)
	{
	quo->exp = op1->exp - op2->exp ;
	quo->msb = op1->msb ^ op2->msb ;
	quo->sig = (uint32_t) (((uint64_t) (op1->sig >> 1) << 32) / op2->sig) ;
	if ((int32_t) quo->sig >= 0) quo->sig <<= 1, quo->exp-- ;
	}

#if CATCH_NANS != 0
static void _Exception(char *op, posit32_t op1, posit32_t op2)
	{
	printf("Exception: P(%08lX) %s P(%08lX) = NaN!\n", op1, op, op2) ;
	exit(0) ;
	}
#endif

static void _MulPosits(unpacked_t *prd, unpacked_t *op1, unpacked_t *op2)
	{
	union {uint64_t u64; int32_t u32[2];} product ;

	prd->msb = op1->msb ^ op2->msb ;
	prd->exp = op1->exp + op2->exp ;
	product.u64 = (uint64_t) op1->sig * (uint64_t) op2->sig ;
	if (product.u32[1] >= 0) product.u64 <<= 1 ;
	else if (++prd->exp > MAXEXP) return ;
	prd->sig = product.u32[1] ;
	}

static posit32_t _PackPosit(unpacked_t *unpacked)
	{
#if POSIT32_ES != 2
	if (IN(unpacked->exp, MINEXP, MAXEXP))
		{
		uint32_t rs, rs_bits, es_bits ;
		posit32_t posit32 ;
		int32_t reg ;

	 	if (unpacked->sig == 0)	return POSIT32_ZRO ;

	  	reg = unpacked->exp / (1 << POSIT32_ES) ;
		if ((unpacked->exp % (1 << POSIT32_ES)) < 0) reg-- ;
   		rs = MAX(1 - reg, reg + 2) ;

    	es_bits = unpacked->exp - (reg << POSIT32_ES) ;
	   	rs_bits = (reg < 0) ? 1 : -2 ;

    	posit32 = (((rs_bits << POSIT32_ES) | es_bits) << (31 - rs - POSIT32_ES)) & 0x7FFFFFFF ;
		posit32 |= (unpacked->sig << 1) >> (rs + POSIT32_ES + 1) ;
		return unpacked->msb ? -posit32 : posit32 ;
		}

	return (unpacked->exp < MINEXP) ? POSIT32_ZRO : POSIT32_INF ;
#else
	typedef struct { uint32_t bits, word ; } TABLE ;
	static TABLE table[] =
	{
		{32, 0x00000004}, {32, 0x00000005}, {32, 0x00000006}, {32, 0x00000007},
		{31, 0x00000008}, {31, 0x0000000A}, {31, 0x0000000C}, {31, 0x0000000E},
		{30, 0x00000010}, {30, 0x00000014}, {30, 0x00000018}, {30, 0x0000001C},
		{29, 0x00000020}, {29, 0x00000028}, {29, 0x00000030}, {29, 0x00000038},
		{28, 0x00000040}, {28, 0x00000050}, {28, 0x00000060}, {28, 0x00000070},
		{27, 0x00000080}, {27, 0x000000A0}, {27, 0x000000C0}, {27, 0x000000E0},
		{26, 0x00000100}, {26, 0x00000140}, {26, 0x00000180}, {26, 0x000001C0},
		{25, 0x00000200}, {25, 0x00000280}, {25, 0x00000300}, {25, 0x00000380},
		{24, 0x00000400}, {24, 0x00000500}, {24, 0x00000600}, {24, 0x00000700},
		{23, 0x00000800}, {23, 0x00000A00}, {23, 0x00000C00}, {23, 0x00000E00},
		{22, 0x00001000}, {22, 0x00001400}, {22, 0x00001800}, {22, 0x00001C00},
		{21, 0x00002000}, {21, 0x00002800}, {21, 0x00003000}, {21, 0x00003800},
		{20, 0x00004000}, {20, 0x00005000}, {20, 0x00006000}, {20, 0x00007000},
		{19, 0x00008000}, {19, 0x0000A000}, {19, 0x0000C000}, {19, 0x0000E000},
		{18, 0x00010000}, {18, 0x00014000}, {18, 0x00018000}, {18, 0x0001C000},
		{17, 0x00020000}, {17, 0x00028000}, {17, 0x00030000}, {17, 0x00038000},
		{16, 0x00040000}, {16, 0x00050000}, {16, 0x00060000}, {16, 0x00070000},
		{15, 0x00080000}, {15, 0x000A0000}, {15, 0x000C0000}, {15, 0x000E0000},
		{14, 0x00100000}, {14, 0x00140000}, {14, 0x00180000}, {14, 0x001C0000},
		{13, 0x00200000}, {13, 0x00280000}, {13, 0x00300000}, {13, 0x00380000},
		{12, 0x00400000}, {12, 0x00500000}, {12, 0x00600000}, {12, 0x00700000},
		{11, 0x00800000}, {11, 0x00A00000}, {11, 0x00C00000}, {11, 0x00E00000},
		{10, 0x01000000}, {10, 0x01400000}, {10, 0x01800000}, {10, 0x01C00000},
		{ 9, 0x02000000}, { 9, 0x02800000}, { 9, 0x03000000}, { 9, 0x03800000},
		{ 8, 0x04000000}, { 8, 0x05000000}, { 8, 0x06000000}, { 8, 0x07000000},
		{ 7, 0x08000000}, { 7, 0x0A000000}, { 7, 0x0C000000}, { 7, 0x0E000000},
		{ 6, 0x10000000}, { 6, 0x14000000}, { 6, 0x18000000}, { 6, 0x1C000000},
		{ 5, 0x20000000}, { 5, 0x28000000}, { 5, 0x30000000}, { 5, 0x38000000},
		{ 5, 0x40000000}, { 5, 0x48000000}, { 5, 0x50000000}, { 5, 0x58000000},
		{ 6, 0x60000000}, { 6, 0x64000000}, { 6, 0x68000000}, { 6, 0x6C000000},
		{ 7, 0x70000000}, { 7, 0x72000000}, { 7, 0x74000000}, { 7, 0x76000000},
		{ 8, 0x78000000}, { 8, 0x79000000}, { 8, 0x7A000000}, { 8, 0x7B000000},
		{ 9, 0x7C000000}, { 9, 0x7C800000}, { 9, 0x7D000000}, { 9, 0x7D800000},
		{10, 0x7E000000}, {10, 0x7E400000}, {10, 0x7E800000}, {10, 0x7EC00000},
		{11, 0x7F000000}, {11, 0x7F200000}, {11, 0x7F400000}, {11, 0x7F600000},
		{12, 0x7F800000}, {12, 0x7F900000}, {12, 0x7FA00000}, {12, 0x7FB00000},
		{13, 0x7FC00000}, {13, 0x7FC80000}, {13, 0x7FD00000}, {13, 0x7FD80000},
		{14, 0x7FE00000}, {14, 0x7FE40000}, {14, 0x7FE80000}, {14, 0x7FEC0000},
		{15, 0x7FF00000}, {15, 0x7FF20000}, {15, 0x7FF40000}, {15, 0x7FF60000},
		{16, 0x7FF80000}, {16, 0x7FF90000}, {16, 0x7FFA0000}, {16, 0x7FFB0000},
		{17, 0x7FFC0000}, {17, 0x7FFC8000}, {17, 0x7FFD0000}, {17, 0x7FFD8000},
		{18, 0x7FFE0000}, {18, 0x7FFE4000}, {18, 0x7FFE8000}, {18, 0x7FFEC000},
		{19, 0x7FFF0000}, {19, 0x7FFF2000}, {19, 0x7FFF4000}, {19, 0x7FFF6000},
		{20, 0x7FFF8000}, {20, 0x7FFF9000}, {20, 0x7FFFA000}, {20, 0x7FFFB000},
		{21, 0x7FFFC000}, {21, 0x7FFFC800}, {21, 0x7FFFD000}, {21, 0x7FFFD800},
		{22, 0x7FFFE000}, {22, 0x7FFFE400}, {22, 0x7FFFE800}, {22, 0x7FFFEC00},
		{23, 0x7FFFF000}, {23, 0x7FFFF200}, {23, 0x7FFFF400}, {23, 0x7FFFF600},
		{24, 0x7FFFF800}, {24, 0x7FFFF900}, {24, 0x7FFFFA00}, {24, 0x7FFFFB00},
		{25, 0x7FFFFC00}, {25, 0x7FFFFC80}, {25, 0x7FFFFD00}, {25, 0x7FFFFD80},
		{26, 0x7FFFFE00}, {26, 0x7FFFFE40}, {26, 0x7FFFFE80}, {26, 0x7FFFFEC0},
		{27, 0x7FFFFF00}, {27, 0x7FFFFF20}, {27, 0x7FFFFF40}, {27, 0x7FFFFF60},
		{28, 0x7FFFFF80}, {28, 0x7FFFFF90}, {28, 0x7FFFFFA0}, {28, 0x7FFFFFB0},
		{29, 0x7FFFFFC0}, {29, 0x7FFFFFC8}, {29, 0x7FFFFFD0}, {29, 0x7FFFFFD8},
		{30, 0x7FFFFFE0}, {30, 0x7FFFFFE4}, {30, 0x7FFFFFE8}, {30, 0x7FFFFFEC},
		{31, 0x7FFFFFF0}, {31, 0x7FFFFFF2}, {31, 0x7FFFFFF4}, {31, 0x7FFFFFF6},
		{32, 0x7FFFFFF8}, {32, 0x7FFFFFF9}, {32, 0x7FFFFFFA}, {32, 0x7FFFFFFB}
	} ;

	if (IN(unpacked->exp, MINEXP, MAXEXP))
		{
		posit32_t posit32 ;
		TABLE *t ;

		if (unpacked->sig == 0)	return POSIT32_ZRO ;

		t = &table[unpacked->exp - MINEXP] ;
		posit32 = t->word | ((unpacked->sig << 1) >> t->bits) ;
		return unpacked->msb ? -posit32 : posit32 ;
		}

	return (unpacked->exp < MINEXP) ? POSIT32_ZRO : POSIT32_INF ;
#endif
    }

static void _ShiftRight(unpacked_t *unpacked, uint32_t bits)
	{
	unpacked->exp += bits ;
	unpacked->sig = (bits < 32) ? unpacked->sig >> bits : 0 ;
	}

static void _UnpackPosit(posit32_t posit32, unpacked_t *unpacked)
	{
	int32_t k, exp, bits, span ;

    unpacked->msb = posit32 < 0 ;
    if (posit32 < 0) posit32 = -posit32 ;

	bits = posit32 << 1 ;
	if (bits < 0)	span = _CountLeadingZeroes(~bits) ;
	else			span = _CountLeadingZeroes( bits) ;

	k = (bits < 0) ? span - 1 : -span ;
	exp = (uint32_t) (bits << (span + 1)) >> (32 - POSIT32_ES) ;

	unpacked->exp = (k << POSIT32_ES) + exp ;
	unpacked->sig = (1 << 31) | (bits << (span + POSIT32_ES)) ;
	}


