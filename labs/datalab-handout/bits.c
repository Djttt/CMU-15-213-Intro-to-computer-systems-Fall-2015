/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 *   Xor x^y = (x | y) & ~(x & y)
 *   x | y = ~(~x & ~y)
 */
int bitXor(int x, int y) {
    return ~(~x & ~y) & ~(x & y);
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
    return 1 << 31;
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 2
 */
int isTmax(int x) {
    // ideal:
    // Tmin + Tmax = - 1, Tmax + 1 = Tmin;
    // 确保x != -1
    int y = x + 1;
    int sum = x + y;

    return !(sum + 1) & !!(x + 1);
         
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
    int odd = 0xAA << 24 | 0xAA << 16 | 0xAA << 8 | 0xAA;
    int x_hat = x & odd;
    return !(x_hat ^ odd);
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
    return (~x) + 1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
    int negX = (~x) + 1;
    
    int lowerBound = 0x30;
    int higherBound = 0x39;

    int ge_lower = !(x + (~lowerBound + 1) >> 31);
    int le_higher = !(higherBound + negX >> 31);

    return ge_lower & le_higher;
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
    int mask = ~(!x) + 1;
    return (mask & z) | (~mask & y);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
    int neg_x = (~x) + 1;
    int diff = y + neg_x;
    
    int x_sign = x >> 31 & 1;
    int y_sign = y >> 31 & 1;
    int diff_sign = diff >> 31 & 1;
    
    int sign_diff = x_sign ^ y_sign;
    int x_less_y = sign_diff & x_sign;

    int same_sign = !sign_diff & !diff_sign;

    return same_sign | x_less_y;
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
    // Ideal: for all x != 0, x and -x at least have one in signature has one 
    // and x = 0, x and -x in signature all equals 0.

    int neg_x = (~x) + 1;
    int x_sign = x >> 31 & 1;
    int neg_x_sign = neg_x >> 31 & 1;
    
    return (x_sign | neg_x_sign) ^ 1;

}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {

  int sign, b16, b8, b4, b2, b1, b0;
  sign = x >> 31;
  x = x ^ sign;

  b16 = (!!(x >> 16)) << 4;
  x = x >> b16;
  
  b8 = !!(x >> 8) << 3;
  x = x >> b8;

  b4 = !!(x >> 4) << 2;
  x = x >> b4;

  b2 = !!(x >> 2) << 1;
  x = x >> b2;


  b1 = !!(x >> 1);
  x = x >> b1;

  b0 = x;

  return b16 + b8 + b4 + b2 + b1 + b0 + 1;

}
//float
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
    int sign = uf & 0x80000000;    
    int exp = (uf >> 23) & 0xff;
    int frac = uf & 0x7fffff;

    // Case1: NaN or infinity
    if (exp == 0xFF) {
      return uf;
    }

    // Case2: denormalize
    if (exp == 0x00) {
        frac <<= 1;
        // check if shifting caused normalization
        if (frac & 0x800000) {
          exp = 1;
          frac = frac & 0x7fffff;
        }
        return sign | exp << 23 | frac;
    }

    // Case3: normalized(0 < exp < 255)
    exp += 1;
    if (exp == 0xff) {
      // return infinity number
      return sign | 0xff << 23;
    }    
    return sign | exp << 23 | frac;

}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {
    
    unsigned temp, frac;
    int sign, count, exp, frac_shift, extra, halfway;
    
    unsigned abs_x = x;
    sign = 0;
    // Case 1: if x == 0 
    if (x == 0) {
      return x;
    }
    
    // Case 2: x != 0
    // step 1: find highest 1 in bit-leve。
    // 如果x 为最小的Tmin 没有对应的Tmax， 下面while循环会陷入死循环。
    if (x < 0) {
      sign = 1 << 31;
      abs_x = -x;
    }

    // Step 1: find highest bit 1
    count = 0;
    temp = abs_x;
    while ((temp >>= 1)) {
      count++;
    }
    
    // Step 2: compute exponent.
    exp = count + 127;

    // Step 3: compute frac.
    frac_shift = count - 23;

    // shift right
    if (frac_shift > 0) {
      extra = abs_x & ((1 << frac_shift) - 1);
      frac = abs_x >> frac_shift & 0x7fffff;
      
      // Round to even
      halfway = 1 << (frac_shift - 1);
      if (extra > halfway || (extra == halfway && (frac & 1))) {
        frac += 1;
        // frac 溢出
        if (frac >> 23) {
            exp++;        
        }
      }
    }
    else {
      // shift left
      frac = abs_x << (-frac_shift);
    }

    return sign | exp << 23 | (frac & 0x7fffff);

}
/* 
 * float_f2i - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int float_f2i(unsigned uf) {
    int sign = uf >> 31;
    int exp = (uf >> 23) & 0xff;
    int frac = uf & 0x7fffff;
    int E = exp - 127;
    unsigned result;

    if (exp == 0xFF || E > 31) {
        return 0x80000000u;
    }

    if (E < 0) {
        return 0;
    }

    // 加上隐含的前导 1
    frac |= 1 << 23;
    
    if (E > 23) {
        result = frac << (E - 23);
    } else {
        result = frac >> (23 - E);
    }

    // ❗ 溢出检查（关键：INT_MIN不能用 -result 表示）
    if ((!sign && result >> 31) || (sign && result > 0x80000000)) {
        return 0x80000000u;
    }

    // ❗ 特别注意：return 的时候转换回 int 之前不要让它溢出
    return sign ? (~result + 1) : result;

}
