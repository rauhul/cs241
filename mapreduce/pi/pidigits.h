/**
 * MapReduce
 * CS 241 - Fall 2016
 */

/*
   This is a collection of routines useful in computing pi using
   Bailey-Borwein-Plouffe type calculations on CUDA graphics cards.

   Here are the main methods this provides:
     static u64 CudaPi::modPow2(u64 exponent, u64 modulo)
      - Returns (2^exponent) % modulo

     static u64 CudaPi::fraction(u64 numerator, u64 divisor)
      - Returns the top 64 bits of the fractional part of a division:
        (((numerator / divisor) mod 1) * 2^64)

   The code may be compiled as regular C or as CUDA.  The C code
   is not terribly efficient, but the CUDA version contains CUDA-specific
   optimizations, like inline PTX assembly.


   Copyright (C) 2012 Ed Karrels, ed.karrels@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>

#ifndef __CUDA_PI__
#define __CUDA_PI__

#ifdef __CUDACC__
#include "cuda.h"
#define HD __host__ __device__
#define DV __device__
#else
#define HD
#define DV
#endif

#ifdef _WIN32
#include <windows.h>
// #include <intrin.h>
// #pragma intrinsic (__UMULH)
#else
#include <sys/time.h>
#endif

// convenience typedefs
typedef long long unsigned u64;
typedef long long s64;

#if !defined(__CUDACC__) && defined(__x86_64) && __GNUC__ >= 4 &&              \
    __GNUC_MINOR__ >= 6
#define GCC_INT128
#endif

/*
  Low-level primitives for extended-precision math.
  Routines included:
    join64()
    split64()
    addCarryOut()
    addCarryInOut()
    addCarryIn()
    addCarryOut64()
    addCarryInOut64()
    addCarryIn64()
    multHi()
    multFull64()
    multHi64()
    multFullAddCarryOut64()
    countLeadingZeros64()
    highestSetBit64()
*/
class CudaPiMath {
public:
  // combine two 32-bit numbers into one 64 bit number
  HD static u64 join64(unsigned hi, unsigned lo) {
#if !defined(__CUDACC__) || !defined(__CUDA_ARCH__)
    return ((u64)hi << 32) | lo;
#else
    return join64_ptx(hi, lo);
#endif
  }

  // split one 64-bit number into two 32-bit numbers
  HD static void split64(u64 x, unsigned &hi, unsigned &lo) {
#if !defined(__CUDACC__) || !defined(__CUDA_ARCH__)
    lo = (unsigned)x;
    hi = (unsigned)(x >> 32);
#else
    split64_ptx(x, hi, lo);
#endif
  }

  // add with carry out (set carry as if it were a carry bit, but ignore
  // its incoming value)
  HD static unsigned addCarryOut(unsigned a, unsigned b, int &carry) {
    unsigned result = a + b;
    carry = (result < a);
    return result;
  }

  // add with carry in and carry out
  HD static unsigned addCarryInOut(unsigned a, unsigned b, int &carry) {
    unsigned result = a + b + carry;
    if (carry)
      carry = (result <= a);
    else
      carry = (result < a);
    return result;
  }

  // add with carry in (use the carry bit, but don't set it afterwards)
  HD static unsigned addCarryIn(unsigned a, unsigned b, int &carry) {
    return a + b + carry;
  }

  // add with carry out for 64 bit integers
  HD static u64 addCarryOut64(u64 a, u64 b, int &carry) {
    u64 result = a + b;
    carry = (result < a);
    return result;
  }

  // add with carry in and out for 64 bit integers
  HD static u64 addCarryInOut64(u64 a, u64 b, int &carry) {
    u64 result = a + b + carry;
    if (carry)
      carry = (result <= a);
    else
      carry = (result < a);
    return result;
  }

  // add with carry in for 64 bit integers
  HD static u64 addCarryIn64(u64 a, u64 b, int &carry) { return a + b + carry; }

  // subtract with borrow out; use the carry bit like a borrow bit
  HD static unsigned subBorrowOut(unsigned a, unsigned b, int &carry) {
    unsigned result = a - b;
    carry = (result > a);
    return result;
  }

  // subtract with borrow in and out
  HD static unsigned subBorrowInOut(unsigned a, unsigned b, int &carry) {
    unsigned result = a - b - carry;
    if (carry)
      carry = (result >= a);
    else
      carry = (result > a);
    return result;
  }

  // subtract with borrow in
  HD static unsigned subBorrowIn(unsigned a, unsigned b, int &carry) {
    return a - b - carry;
  }

  // multiply two 32-bit numbers and return the high 32 bits of the result
  HD static unsigned multHi(unsigned f1, unsigned f2) {
#if !defined(__CUDACC__) || !defined(__CUDA_ARCH__)
    return (unsigned)(((u64)f1 * f2) >> 32);

#else
    return multHi_intrinsic(f1, f2);
#endif
  }

  // multiply two 64-bit numbers and return the high 64 bits of the result
  HD static u64 multHi64(u64 f1, u64 f2) {

// CUDA intrinsic
#if defined(__CUDACC__) && defined(__CUDA_ARCH__)
    return multHi64_intrinsic(f1, f2);
#else

// Windows intrinsic
#ifdef _WIN64
    // __mul128, __mulh, _umul128
    // return __umulh(f1, f2);
    return UnsignedMultiplyHigh(f1, f2);
#else

// GNU __int128 type
// no significant speedup
#ifdef GCC_INT128
    unsigned __int128 big = f1;
    big *= f2;
    return (u64)(big >> 64);
#else

    u64 resultHi, resultLo;
    multFull64(f1, f2, resultHi, resultLo);
    return resultHi;

#endif
#endif
#endif
  }

  // multiply two 64-bit numbers and return both the high and low parts
  // of the result
  HD static void multFull64(u64 f1, u64 f2, u64 &resultHi, u64 &resultLo) {

// CUDA
#if defined(__CUDACC__) && defined(__CUDA_ARCH__)
    multFull64_intrinsic(f1, f2, resultHi, resultLo);
#else

// Windows intrinsic
#ifdef _WIN64
    resultLo = f1 * f2;
    resultHi = multHi64(f1, f2);
#else

// GNU __int128 type
// 44% faster
#ifdef GCC_INT128
    unsigned __int128 big = f1;
    big *= f2;
    resultLo = (u64)big;
    resultHi = (u64)(big >> 64);
#else

    unsigned a, b, c, d, result0, result1, result2, result3;
    int carry;

    split64(f1, a, b);
    split64(f2, c, d);

    result0 = b * d;
    result1 = multHi(b, d);
    result1 = addCarryOut(a * d, result1, carry);
    result2 = addCarryIn(multHi(a, d), 0, carry);
    result1 = addCarryOut(b * c, result1, carry);
    result2 = addCarryInOut(multHi(b, c), result2, carry);
    result3 = addCarryIn(0, 0, carry);
    result2 = addCarryOut(a * c, result2, carry);
    result3 = addCarryIn(multHi(a, c), result3, carry);

    resultLo = join64(result1, result0);
    resultHi = join64(result3, result2);

#endif
#endif
#endif
  }

  // combined multiply-add operation, with carry-out
  //   (resultHi,resultLo) += (f1*f2)
  HD static void multFullAddCarryOut64(u64 f1, u64 f2, u64 &resultHi,
                                       u64 &resultLo, int &carry) {
    u64 tmpLo, tmpHi;
    multFull64(f1, f2, tmpHi, tmpLo);
    resultLo = addCarryOut64(tmpLo, resultLo, carry);
    resultHi = addCarryInOut64(tmpHi, resultHi, carry);
  }

  // return the number of leading zeros at the top of a 64 bit number
  // For example if the input was an 8 bit number, 00011111 would return 3.
  HD static int countLeadingZeros64(u64 x) {
#if defined(__CUDACC__) && defined(__CUDA_ARCH__)
    return countLeadingZeros64_intrinsic(x);
#else

#ifdef __GNUC__
    return __builtin_clzll(x);
#else

    unsigned y, n;
    n = 64;
    y = x >> 32;
    if (y != 0) {
      n -= 32;
      x = y;
    }
    y = x >> 16;
    if (y != 0) {
      n -= 16;
      x = y;
    }
    y = x >> 8;
    if (y != 0) {
      n -= 8;
      x = y;
    }
    y = x >> 4;
    if (y != 0) {
      n -= 4;
      x = y;
    }
    y = x >> 2;
    if (y != 0) {
      n -= 2;
      x = y;
    }
    y = x >> 1;
    if (y != 0)
      return n - 2;
    return n - x;
#endif
#endif
  }

  // Return the highest set bit in a 64 bit number.
  // For example, if the input was an 8 bit number, 00011111 would
  // return 00010000.
  HD static u64 highestSetBit64(u64 x) {
    return 1ull << (63 - countLeadingZeros64(x));
  }

private:
#ifdef __CUDACC__

  // CUDA PTX or intrinsic routines that would be much less inefficient
  // if done in straight C.

  __device__ static u64 join64_ptx(unsigned hi, unsigned lo) {
    u64 x;
    asm("mov.b64 %0,{%1,%2};\n" : "=l"(x) : "r"(lo), "r"(hi));
    return x;
  }

  __device__ static void split64_ptx(u64 x, unsigned &hi, unsigned &lo) {
    unsigned lo_, hi_;
    asm("mov.b64 {%0,%1},%2;\n" : "=r"(lo_), "=r"(hi_) : "l"(x));
    lo = lo_;
    hi = hi_;
  }

  __device__ static unsigned multHi_intrinsic(unsigned a, unsigned b) {
    return __umulhi(a, b);
  }

  __device__ static void multFull64_intrinsic(u64 f1, u64 f2, u64 &resultHi,
                                              u64 &resultLo) {
    resultLo = f1 * f2;
    resultHi = __umul64hi(f1, f2);
  }

  __device__ static u64 multHi64_intrinsic(u64 a, u64 b) {
    return __umul64hi(a, b);
  }

  __device__ static int countLeadingZeros64_intrinsic(u64 x) {
    return __clzll(x);
  }

#endif
};

class Int128 {
  unsigned dataword0, dataword1, dataword2, dataword3;

public:
  // by default, don't set anything
  HD Int128() {}

  HD Int128(u64 init) {
    setPart0(init);
    setPart1(0);
  }

  HD Int128(u64 hi, u64 lo) {
    setPart0(lo);
    setPart1(hi);
  }

  HD Int128(unsigned w3, unsigned w2, unsigned w1, unsigned w0) {
    word0() = w0;
    word1() = w1;
    word2() = w2;
    word3() = w3;
  }

  HD Int128(const Int128 &other) {
    dataword0 = other.dataword0;
    dataword1 = other.dataword1;
    dataword2 = other.dataword2;
    dataword3 = other.dataword3;
  }

  // access the data as 64-bit integers
  HD void setPart0(u64 x) {
    unsigned hi, lo;
    CudaPiMath::split64(x, hi, lo);
    word0() = lo;
    word1() = hi;
  }
  HD void setPart1(u64 x) {
    unsigned hi, lo;
    CudaPiMath::split64(x, hi, lo);
    word2() = lo;
    word3() = hi;
  }

  HD u64 getPart0() const { return CudaPiMath::join64(dataword1, dataword0); }
  HD u64 getPart1() const { return CudaPiMath::join64(dataword3, dataword2); }

  // access the data as 32-bit integers
  HD unsigned &word0() { return dataword0; }
  HD unsigned &word1() { return dataword1; }
  HD unsigned &word2() { return dataword2; }
  HD unsigned &word3() { return dataword3; }

  // comparison operators

  HD bool operator==(Int128 x) {
    return word0() == x.word0() && word1() == x.word1() &&
           word2() == x.word2() && word3() == x.word3();
  }

  HD bool operator==(u64 x) {
    return word0() == (unsigned)x && word1() == (unsigned)(x >> 32) &&
           word2() == 0 && word3() == 0;
  }

  HD bool operator!=(Int128 x) { return !(*this == x); }

  HD bool operator!=(u64 x) { return !(*this == x); }

  HD bool operator<(Int128 x) {
    if (word3() == x.word3()) {
      if (word2() == x.word2()) {
        if (word1() == x.word1()) {
          return word0() < x.word0();
        } else {
          return word1() < x.word1();
        }
      } else {
        return word2() < x.word2();
      }
    } else {
      return word3() < x.word3();
    }
  }

  HD bool operator>(Int128 that) { return that < *this; }

  HD bool operator<=(Int128 that) { return !(that < *this); }

  HD bool operator>=(Int128 that) { return !(*this < that); }

  // shift operators

  HD Int128 operator<<(unsigned s) {
    Int128 result;
    s &= 127; // modulo 128
    if (s <= 0)
      return *this;
    if (s >= 64) {
      result.setPart0(0);
      result.setPart1(getPart0() << (s - 64));
    } else {
      result.setPart0(getPart0() << s);
      result.setPart1((getPart1() << s) | (getPart0() >> (64 - s)));
    }
    return result;
  }

  HD Int128 operator>>(unsigned s) {
    Int128 result;
    s &= 127; // modulo 128
    if (s == 0)
      return *this;
    if (s >= 64) {
      result.setPart1(0);
      result.setPart0(getPart1() >> (s - 64));
    } else {
      result.setPart1(getPart1() >> s);
      result.setPart0((getPart0() >> s) | (getPart1() << (64 - s)));
    }
    return result;
  }

  HD Int128 operator>>=(unsigned s) { return *this = *this >> s; }

  HD Int128 operator<<=(unsigned s) { return *this = *this << s; }

  // addition operator

  HD Int128 operator+(unsigned x) {
    Int128 result;
#if !defined(__CUDACC__) || !defined(__CUDA_ARCH__)
    /*
    int carry;
    result.word0() = CudaPiMath::addCarryOut  (word0(), x, carry);
    result.word1() = CudaPiMath::addCarryInOut(word1(), 0, carry);
    result.word2() = CudaPiMath::addCarryInOut(word2(), 0, carry);
    result.word3() = CudaPiMath::addCarryIn   (word3(), 0, carry);
    */

    result.word0() = word0() + x;
    result.word1() = word1() + (result.word0() < word0());
    result.word2() = word2() + (result.word1() < word1());
    result.word3() = word3() + (result.word2() < word2());

/*
result.dataword0 = dataword0 + x;
result.dataword1 = dataword1 + (result.dataword0 < dataword0);
result.dataword2 = dataword2 + (result.dataword1 < dataword1);
result.dataword3 = dataword3 + (result.dataword2 < dataword2);
*/
#else
    add32_asm(result, *this, x);
#endif
    return result;
  }

  HD Int128 operator+(Int128 x) {
    Int128 result;
#if !defined(__CUDACC__) || !defined(__CUDA_ARCH__)

    int carry;
    result.word0() = CudaPiMath::addCarryOut(word0(), x.word0(), carry);
    result.word1() = CudaPiMath::addCarryInOut(word1(), x.word1(), carry);
    result.word2() = CudaPiMath::addCarryInOut(word2(), x.word2(), carry);
    result.word3() = CudaPiMath::addCarryIn(word3(), x.word3(), carry);

/*
result.word0() = word0() + x.word0();
result.word1() = word1() + x.word1() + (result.word0() < word0());
result.word2() = word2() + x.word2() + ((result.word0() < word0()) ?
result.word1() <= word1() : result.word1() < word1());
result.word3() = word3() + x.word3() + (((result.word0() < word0()) ?
result.word1() <= word1() : result.word1() < word1()) ? result.word2() <=
word2() : result.word2() < word2());
*/

#else
    // this slows down the code a bit on Fermi cards
    // (might just be the old compiler -- test this with 5.0 compiler)
    add128_asm(result, *this, x);
#endif
    return result;
  }

  HD Int128 operator+=(unsigned x) { return *this = *this + x; }

  HD Int128 operator+=(Int128 x) { return *this = *this + x; }

  HD Int128 operator-(Int128 x) {
    Int128 result;
#if !defined(__CUDACC__) || !defined(__CUDA_ARCH__)
    int borrow;
    result.word0() = CudaPiMath::subBorrowOut(word0(), x.word0(), borrow);
    result.word1() = CudaPiMath::subBorrowInOut(word1(), x.word1(), borrow);
    result.word2() = CudaPiMath::subBorrowInOut(word2(), x.word2(), borrow);
    result.word3() = CudaPiMath::subBorrowIn(word3(), x.word3(), borrow);
#else
    sub128_asm(result, *this, x);
#endif
    return result;
  }

  HD Int128 operator-=(Int128 x) { return *this = *this - x; }

  HD static Int128 mult64(u64 a, u64 b) {
    u64 tmpHi, tmpLo;
    CudaPiMath::multFull64(a, b, tmpHi, tmpLo);
    return Int128(tmpHi, tmpLo);
  }

#ifdef __CUDACC__
  __device__ static void add32_asm(Int128 &result, Int128 &a, unsigned b) {
    asm("add.cc.u32  %0, %4, %8;\n\t"
        "addc.cc.u32 %1, %5, 0;\n\t"
        "addc.cc.u32 %2, %6, 0;\n\t"
        "addc.u32    %3, %7, 0;\n\t"
        : "=r"(result.word0()), "=r"(result.word1()), "=r"(result.word2()),
          "=r"(result.word3())
        : "r"(a.word0()), "r"(a.word1()), "r"(a.word2()), "r"(a.word3()),
          "r"(b));
  }

  __device__ static void add128_asm(Int128 &result, Int128 &a, Int128 &b) {
    asm("add.cc.u32  %0, %4, %8;\n\t"
        "addc.cc.u32 %1, %5, %9;\n\t"
        "addc.cc.u32 %2, %6, %10;\n\t"
        "addc.u32    %3, %7, %11;\n\t"
        : "=r"(result.word0()), "=r"(result.word1()), "=r"(result.word2()),
          "=r"(result.word3())
        : "r"(a.word0()), "r"(a.word1()), "r"(a.word2()), "r"(a.word3()),
          "r"(b.word0()), "r"(b.word1()), "r"(b.word2()), "r"(b.word3()));
  }

  __device__ static void sub128_asm(Int128 &result, Int128 &a, Int128 &b) {
    asm("sub.cc.u32  %0, %4, %8;\n\t"
        "subc.cc.u32 %1, %5, %9;\n\t"
        "subc.cc.u32 %2, %6, %10;\n\t"
        "subc.u32    %3, %7, %11;\n\t"
        : "=r"(result.word0()), "=r"(result.word1()), "=r"(result.word2()),
          "=r"(result.word3())
        : "r"(a.word0()), "r"(a.word1()), "r"(a.word2()), "r"(a.word3()),
          "r"(b.word0()), "r"(b.word1()), "r"(b.word2()), "r"(b.word3()));
  }

  __device__ static void mult128_asm(Int128 a, Int128 b, Int128 &result_hi,
                                     Int128 &result_lo) {
    // write_mult_asm.py 2 4 full 'a.word%d()' 'b.word%d()'
    // 'result_lo.word%d(),result_hi.word%d()'
    asm("{\n\t"
        ".reg .u32 tmp;\n\t"
        "mul.lo.u32       %0, %8, %12;\n\t"
        "mul.lo.u32       %1, %8, %13;\n\t"
        "mul.lo.u32       %2, %8, %14;\n\t"
        "mul.lo.u32       %3, %8, %15;\n\t"

        "mul.hi.u32       tmp, %8, %12;\n\t"
        "add.cc.u32       %1, %1, tmp;\n\t"
        "mul.hi.u32       tmp, %8, %13;\n\t"
        "addc.cc.u32      %2, %2, tmp;\n\t"
        "mul.hi.u32       tmp, %8, %14;\n\t"
        "addc.cc.u32      %3, %3, tmp;\n\t"
        "mul.hi.u32       tmp, %8, %15;\n\t"
        "addc.u32         %4, 0, tmp;\n\t"

        "mul.lo.u32       tmp, %9, %12;\n\t"
        "add.cc.u32       %1, %1, tmp;\n\t"
        "mul.lo.u32       tmp, %9, %13;\n\t"
        "addc.cc.u32      %2, %2, tmp;\n\t"
        "mul.lo.u32       tmp, %9, %14;\n\t"
        "addc.cc.u32      %3, %3, tmp;\n\t"
        "mul.lo.u32       tmp, %9, %15;\n\t"
        "addc.u32         %4, %4, tmp;\n\t"

        "mul.hi.u32       tmp, %9, %12;\n\t"
        "add.cc.u32       %2, %2, tmp;\n\t"
        "mul.hi.u32       tmp, %9, %13;\n\t"
        "addc.cc.u32      %3, %3, tmp;\n\t"
        "mul.hi.u32       tmp, %9, %14;\n\t"
        "addc.cc.u32      %4, %4, tmp;\n\t"
        "mul.hi.u32       tmp, %9, %15;\n\t"
        "addc.u32         %5, 0, tmp;\n\t"

        "mul.lo.u32       tmp, %10, %12;\n\t"
        "add.cc.u32       %2, %2, tmp;\n\t"
        "mul.lo.u32       tmp, %10, %13;\n\t"
        "addc.cc.u32      %3, %3, tmp;\n\t"
        "mul.lo.u32       tmp, %10, %14;\n\t"
        "addc.cc.u32      %4, %4, tmp;\n\t"
        "mul.lo.u32       tmp, %10, %15;\n\t"
        "addc.u32         %5, %5, tmp;\n\t"

        "mul.hi.u32       tmp, %10, %12;\n\t"
        "add.cc.u32       %3, %3, tmp;\n\t"
        "mul.hi.u32       tmp, %10, %13;\n\t"
        "addc.cc.u32      %4, %4, tmp;\n\t"
        "mul.hi.u32       tmp, %10, %14;\n\t"
        "addc.cc.u32      %5, %5, tmp;\n\t"
        "mul.hi.u32       tmp, %10, %15;\n\t"
        "addc.u32         %6, 0, tmp;\n\t"

        "mul.lo.u32       tmp, %11, %12;\n\t"
        "add.cc.u32       %3, %3, tmp;\n\t"
        "mul.lo.u32       tmp, %11, %13;\n\t"
        "addc.cc.u32      %4, %4, tmp;\n\t"
        "mul.lo.u32       tmp, %11, %14;\n\t"
        "addc.cc.u32      %5, %5, tmp;\n\t"
        "mul.lo.u32       tmp, %11, %15;\n\t"
        "addc.u32         %6, %6, tmp;\n\t"

        "mul.hi.u32       tmp, %11, %12;\n\t"
        "add.cc.u32       %4, %4, tmp;\n\t"
        "mul.hi.u32       tmp, %11, %13;\n\t"
        "addc.cc.u32      %5, %5, tmp;\n\t"
        "mul.hi.u32       tmp, %11, %14;\n\t"
        "addc.cc.u32      %6, %6, tmp;\n\t"
        "mul.hi.u32       tmp, %11, %15;\n\t"
        "addc.u32         %7, 0, tmp;\n\t"

        "}\n\t"
        : "=r"(result_lo.word0()), "=r"(result_lo.word1()),
          "=r"(result_lo.word2()), "=r"(result_lo.word3()),
          "=r"(result_hi.word0()), "=r"(result_hi.word1()),
          "=r"(result_hi.word2()), "=r"(result_hi.word3())
        : "r"(a.word0()), "r"(a.word1()), "r"(a.word2()), "r"(a.word3()),
          "r"(b.word0()), "r"(b.word1()), "r"(b.word2()), "r"(b.word3()));
  }

  __device__ static void mult128lo_asm(Int128 a, Int128 b, Int128 &result_lo) {
    // write_mult_asm.py 2 4 lo 'a.word%d()' 'b.word%d()' 'result_lo.word%d()'
    asm("{\n\t"
        ".reg .u32 tmp;\n\t"
        "mul.lo.u32       %0, %4, %8;\n\t"
        "mul.lo.u32       %1, %4, %9;\n\t"
        "mul.lo.u32       %2, %4, %10;\n\t"
        "mul.lo.u32       %3, %4, %11;\n\t"

        "mul.hi.u32       tmp, %4, %8;\n\t"
        "add.cc.u32       %1, %1, tmp;\n\t"
        "mul.hi.u32       tmp, %4, %9;\n\t"
        "addc.cc.u32      %2, %2, tmp;\n\t"
        "mul.hi.u32       tmp, %4, %10;\n\t"
        "addc.u32         %3, %3, tmp;\n\t"

        "mul.lo.u32       tmp, %5, %8;\n\t"
        "add.cc.u32       %1, %1, tmp;\n\t"
        "mul.lo.u32       tmp, %5, %9;\n\t"
        "addc.cc.u32      %2, %2, tmp;\n\t"
        "mul.lo.u32       tmp, %5, %10;\n\t"
        "addc.u32         %3, %3, tmp;\n\t"

        "mul.hi.u32       tmp, %5, %8;\n\t"
        "add.cc.u32       %2, %2, tmp;\n\t"
        "mul.hi.u32       tmp, %5, %9;\n\t"
        "addc.u32         %3, %3, tmp;\n\t"

        "mul.lo.u32       tmp, %6, %8;\n\t"
        "add.cc.u32       %2, %2, tmp;\n\t"
        "mul.lo.u32       tmp, %6, %9;\n\t"
        "addc.u32         %3, %3, tmp;\n\t"

        "mad.hi.u32       %3, %6, %8, %3;\n\t"

        "mad.lo.u32       %3, %7, %8, %3;\n\t"
        "}\n\t"
        : "=r"(result_lo.word0()), "=r"(result_lo.word1()),
          "=r"(result_lo.word2()), "=r"(result_lo.word3())
        : "r"(a.word0()), "r"(a.word1()), "r"(a.word2()), "r"(a.word3()),
          "r"(b.word0()), "r"(b.word1()), "r"(b.word2()), "r"(b.word3()));
  }

  __device__ static void mult128hi_asm(Int128 a, Int128 b, Int128 &result_hi) {
    // write_mult_asm.py 2 4 hi 'a.word%d()' 'b.word%d()' 'result_hi.word%d()'
    asm("{\n\t"
        ".reg .u32 tmp;\n\t"
        ".reg .u32 tmp1, tmp2, tmp3;\n\t"

        "mul.lo.u32       tmp1, %4, %9;\n\t"
        "mul.lo.u32       tmp2, %4, %10;\n\t"
        "mul.lo.u32       tmp3, %4, %11;\n\t"

        "mul.hi.u32       tmp, %4, %8;\n\t"
        "add.cc.u32       tmp1, tmp1, tmp;\n\t"
        "mul.hi.u32       tmp, %4, %9;\n\t"
        "addc.cc.u32      tmp2, tmp2, tmp;\n\t"
        "mul.hi.u32       tmp, %4, %10;\n\t"
        "addc.cc.u32      tmp3, tmp3, tmp;\n\t"
        "mul.hi.u32       tmp, %4, %11;\n\t"
        "addc.u32         %0, 0, tmp;\n\t"

        "mul.lo.u32       tmp, %5, %8;\n\t"
        "add.cc.u32       tmp1, tmp1, tmp;\n\t"
        "mul.lo.u32       tmp, %5, %9;\n\t"
        "addc.cc.u32      tmp2, tmp2, tmp;\n\t"
        "mul.lo.u32       tmp, %5, %10;\n\t"
        "addc.cc.u32      tmp3, tmp3, tmp;\n\t"
        "mul.lo.u32       tmp, %5, %11;\n\t"
        "addc.u32         %0, %0, tmp;\n\t"

        "mul.hi.u32       tmp, %5, %8;\n\t"
        "add.cc.u32       tmp2, tmp2, tmp;\n\t"
        "mul.hi.u32       tmp, %5, %9;\n\t"
        "addc.cc.u32      tmp3, tmp3, tmp;\n\t"
        "mul.hi.u32       tmp, %5, %10;\n\t"
        "addc.cc.u32      %0, %0, tmp;\n\t"
        "mul.hi.u32       tmp, %5, %11;\n\t"
        "addc.u32         %1, 0, tmp;\n\t"

        "mul.lo.u32       tmp, %6, %8;\n\t"
        "add.cc.u32       tmp2, tmp2, tmp;\n\t"
        "mul.lo.u32       tmp, %6, %9;\n\t"
        "addc.cc.u32      tmp3, tmp3, tmp;\n\t"
        "mul.lo.u32       tmp, %6, %10;\n\t"
        "addc.cc.u32      %0, %0, tmp;\n\t"
        "mul.lo.u32       tmp, %6, %11;\n\t"
        "addc.u32         %1, %1, tmp;\n\t"

        "mul.hi.u32       tmp, %6, %8;\n\t"
        "add.cc.u32       tmp3, tmp3, tmp;\n\t"
        "mul.hi.u32       tmp, %6, %9;\n\t"
        "addc.cc.u32      %0, %0, tmp;\n\t"
        "mul.hi.u32       tmp, %6, %10;\n\t"
        "addc.cc.u32      %1, %1, tmp;\n\t"
        "mul.hi.u32       tmp, %6, %11;\n\t"
        "addc.u32         %2, 0, tmp;\n\t"

        "mul.lo.u32       tmp, %7, %8;\n\t"
        "add.cc.u32       tmp3, tmp3, tmp;\n\t"
        "mul.lo.u32       tmp, %7, %9;\n\t"
        "addc.cc.u32      %0, %0, tmp;\n\t"
        "mul.lo.u32       tmp, %7, %10;\n\t"
        "addc.cc.u32      %1, %1, tmp;\n\t"
        "mul.lo.u32       tmp, %7, %11;\n\t"
        "addc.u32         %2, %2, tmp;\n\t"

        "mul.hi.u32       tmp, %7, %8;\n\t"
        "add.cc.u32       %0, %0, tmp;\n\t"
        "mul.hi.u32       tmp, %7, %9;\n\t"
        "addc.cc.u32      %1, %1, tmp;\n\t"
        "mul.hi.u32       tmp, %7, %10;\n\t"
        "addc.cc.u32      %2, %2, tmp;\n\t"
        "mul.hi.u32       tmp, %7, %11;\n\t"
        "addc.u32         %3, 0, tmp;\n\t"

        "}\n\t"
        : "=r"(result_hi.word0()), "=r"(result_hi.word1()),
          "=r"(result_hi.word2()), "=r"(result_hi.word3())
        : "r"(a.word0()), "r"(a.word1()), "r"(a.word2()), "r"(a.word3()),
          "r"(b.word0()), "r"(b.word1()), "r"(b.word2()), "r"(b.word3()));
  }

#endif

  /*
    Full 128-bit multiply -> 256 bit result
           A B
           C D
        ------
          --BD
        --AD
        --BC
      --AC
  */
  HD static void mult128(Int128 a, Int128 b, Int128 &result_hi,
                         Int128 &result_lo) {
#if !defined(__CUDACC__) || !defined(__CUDA_ARCH__)
    int carry;
    // --BD
    result_lo = mult64(a.getPart0(), b.getPart0());

    // --AC
    result_hi = mult64(a.getPart1(), b.getPart1());

    // --AD
    Int128 tmp = mult64(a.getPart1(), b.getPart0());
    result_lo.setPart1(
        CudaPiMath::addCarryOut64(result_lo.getPart1(), tmp.getPart0(), carry));
    result_hi.setPart0(CudaPiMath::addCarryInOut64(result_hi.getPart0(),
                                                   tmp.getPart1(), carry));
    result_hi.setPart1(
        CudaPiMath::addCarryIn64(result_hi.getPart1(), 0, carry));

    // --BC
    tmp = mult64(a.getPart0(), b.getPart1());
    result_lo.setPart1(
        CudaPiMath::addCarryOut64(result_lo.getPart1(), tmp.getPart0(), carry));
    result_hi.setPart0(CudaPiMath::addCarryInOut64(result_hi.getPart0(),
                                                   tmp.getPart1(), carry));
    result_hi.setPart1(
        CudaPiMath::addCarryIn64(result_hi.getPart1(), 0, carry));

#else

    mult128_asm(a, b, result_hi, result_lo);

#endif
  }

  HD static Int128 mult128hi(Int128 a, Int128 b) {
    Int128 result_hi;
#if !defined(__CUDACC__) || !defined(__CUDA_ARCH__)
    Int128 result_lo;
    mult128(a, b, result_hi, result_lo);
#else
    mult128hi_asm(a, b, result_hi);
#endif
    return result_hi;
  }

  HD static Int128 mult128lo(Int128 a, Int128 b) {
    Int128 result;
#if !defined(__CUDACC__) || !defined(__CUDA_ARCH__)
    result.setPart0(a.getPart0() * b.getPart0());
    result.setPart1(CudaPiMath::multHi64(a.getPart0(), b.getPart0()) +
                    a.getPart0() * b.getPart1() + a.getPart1() * b.getPart0());
#else
    mult128lo_asm(a, b, result);
#endif
    return result;
  }

  // multiply by a 64 bit value
  HD Int128 operator*(u64 x) {
    Int128 result = mult64(getPart0(), x);
    result.setPart1(result.getPart1() + getPart1() * x);
    return result;
  }

  // multiply by a 128 bit value
  HD Int128 operator*(Int128 x) { return mult128lo(*this, x); }

  HD Int128 operator/(u64 divisor) {
    Int128 quotient;
    u64 modulo;
    divMod(divisor, quotient, modulo);
    return quotient;
  }

  HD Int128 operator%(u64 divisor) {
    Int128 quotient;
    u64 modulo;
    divMod(divisor, quotient, modulo);
    return modulo;
  }

  HD void divMod(u64 divisor, Int128 &quotient, u64 &modulo) {
    if (divisor == 0) {
      modulo = 0xffffffffffffffffull;
      quotient.setPart0(0xffffffffffffffffull);
      quotient.setPart1(0xffffffffffffffffull);
      return;
    }

    if (divisor == 1) {
      modulo = 0;
      quotient.setPart0(getPart0());
      quotient.setPart1(getPart1());
      return;
    }

    divide(divisor, quotient, modulo);
  }

  // Use Newton-Raphson approximation to compute 2**128 / divisor
  HD static Int128 recipNewton(u64 divisor) {

    Int128 inva(0xffffffffffffffffuLL, -divisor);
    float frecip;
    Int128 x;

    int z = CudaPiMath::countLeadingZeros64(divisor);
    // 8388607 == 2**23 - 1
    // undershoot a bit (ha) to insure the estimate is low
    // ~2^(23+63-z) = ~2^(86-z) / divisor
    frecip = (8388607.0f * ((u64)1 << (63 - z))) / divisor;
    // want 2^128 / divisor, have 2^(86-z) / divisor,
    // so shift by 128 - (86-z) = z + 42
    x = Int128(0, (u64)frecip) << (z + 42);
    x = x + Int128::mult128hi(x, inva * x);
    x = x + Int128::mult128hi(x, inva * x);
    x = x + Int128::mult128hi(x, inva * x);

    // final check
    u64 check = (Int128(0) - (x * divisor)).getPart0();
    if (check >= divisor)
      x += 1;

    return x;
  }

  /*
    Divide a Int128 by a 64-bit value using Newton-Raphson approximmation.

    1/a =
    x(n+1) = x(n) * (2 - a * x(n))
           = 2*x(n) - a * x(n) * x(n)
           = x(n) + x(n) - a * x(n) * x(n)
           = x(n) + x(n) * (1 - a * x(n))
          ~= x(n) + x(n) * (- a * x(n))
  */
  HD void divide(u64 divisor, Int128 &quotient, u64 &modulo) {

    Int128 recip = recipNewton(divisor);
    quotient = mult128hi(recip, *this);
    modulo = (*this - quotient * divisor).getPart0();
    if (modulo >= divisor) {
      modulo -= divisor;
      quotient += 1;
    }
  }

  HD char *toString(char buf[35]) {
    sprintf(buf, "0x%08x%08x%08x%08x", dataword3, dataword2, dataword1,
            dataword0);
    return buf;
  }
};

class CudaPi {

  // Use Newton approximation method to compute -m^-1 (mod R)
  // see Crandall & Pomerance, "Prime Numbers", Exercise 9.12
  HD static u64 inverse(u64 m, u64 R) {
    u64 mask = R - 1;
    u64 x = 8 - (m & 7);
    while (true) {
      u64 x1 = (x * (2 + m * x)) & mask;
      if (x1 == x)
        return x;
      x = x1;
    }
  }

#ifdef GCC_INT128
#define MONTGOMERY_INT128 unsigned __int128
#else
#define MONTGOMERY_INT128 Int128
#endif

  HD static u64 montgomeryMult(u64 factor1, u64 factor2, u64 m, u64 minv,
                               u64 mask, int s) {
// printf("montMult %llu * %llu\n", factor1, factor2);

// 2x speedup
#ifdef GCC_INT128

    unsigned __int128 x = factor1;
    x *= factor2;

    u64 xnp = (minv * (u64)x) & mask;

    // xnp has up to 's' bits, as does m
    // s < 64, so it will fit in an Int128
    unsigned __int128 tmp = xnp;
    tmp = ((tmp * m) + x) >> s;
    u64 result = (u64)tmp;
    if (result >= m)
      result -= m;

#else

    Int128 x = Int128::mult64(factor1, factor2);
    u64 xnp = (minv * x.getPart0()) & mask;

    // xnp has up to 's' bits, as does m
    // s < 64, so it will fit in an Int128
    Int128 tmp = xnp;
    tmp = ((tmp * m) + x) >> s;
    u64 result = tmp.getPart0();
    if (result >= m)
      result -= m;

#endif
    return result;
  }

public:
  // Returns (2^exponent) % modulo
  HD static u64 modPow2(u64 exponent, u64 modulo) {
    if (exponent == 0)
      return 1;

    // for odd numbers we can use Montgomery reduction
    if ((modulo & 1) == 1) {
      return montgomeryModPow2(exponent, modulo);
    }

    Int128 result = 2;
    for (u64 bit = CudaPiMath::highestSetBit64(exponent) >> 1; bit > 0;
         bit >>= 1) {
      result = result * result;
      if (result >= modulo)
        result = result % modulo;
      if (exponent & bit) {
        result <<= 1;
        if (result >= modulo)
          result = result - modulo;
      }
    }
    return result.getPart0();
  }

  /*
    Compute (2^p)%m
    m is assumed to be odd.  Behavior is undefined if not.

    This uses Montgomery reduction to compute repeated modulo
    operations.  The multiplicative inverse of 2 (mod m) is used to
    up-convert the modular base to the next highest power of two.  For
    example, 1001 -> 1024.  Since the multiplicative inverse is
    required, 2 must be coprime with m, so m must be odd.  With a
    power of two as the modular base, all modulo operations can be
    implemented as bit masks, greatly speeding them up.
  */
  HD static u64 montgomeryModPow2(u64 p, u64 m) {

    // R = 2^s
    //   = least power of 2 greater than m
    int s = 64 - CudaPiMath::countLeadingZeros64(m);
    u64 R = (u64)1 << s;
    u64 mask = R - 1;

    // minv = -m^-1 (mod R)
    // this uses about 10% of the modpow() time
    u64 minv = inverse(m, R);

    u64 factor = ((u64)2 << s) % m;

    u64 total = R % m;

    u64 bit = CudaPiMath::highestSetBit64(p);
    // printf("  total: %llu\n", total);
    while (bit) {
      total = montgomeryMult(total, total, m, minv, mask, s);
      // printf("   totalx: %llu\n", total);
      if (p & bit) {
        total = montgomeryMult(total, factor, m, minv, mask, s);
        // printf("   totaly: %llu\n", total);
      }
      bit >>= 1;
    }

    total = montgomeryMult(total, 1, m, minv, mask, s);

    return total;
  }

  /*
    Returns the top 64 bits of the fractional part of a division:
    (((numerator / divisor) mod 1) * 2^64)

    For example:
      1/2 -> 0x8000000000000000
      1/3 -> 0x5555555555555555
      12345678/99999999 -> 0x1f9add15df348715
  */
  HD static u64 fraction(u64 numerator, u64 divisor) {
// GNU __int128 type
// 5% faster
#ifdef GCC_INT128
    unsigned __int128 big = numerator;
    big <<= 64;
    return (u64)(big / divisor);
#else
    Int128 numerator128(numerator, 0);
    return (numerator128 / divisor).getPart0();
#endif
  }

  // handy timer function - returns a relative time in seconds
  static double timer() {
#ifdef _WIN32
    LARGE_INTEGER counter, freq;
    if (!QueryPerformanceFrequency(&freq)) {
      printf("Error: High-resolution performance counter not supported.\n");
      return 0;
    }
    if (!QueryPerformanceCounter(&counter)) {
      printf("Error calling QueryPerformanceCounter\n");
      return 0;
    }

    return counter.QuadPart / (double)freq.QuadPart;
#else
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec * 0.000001;
#endif // _WIN32
  }

#ifdef __CUDACC__
  /*
    Atomically add a 64 bit value addend to *dest.
    This is supported in CUDA 1.2 and later by the atomicAdd() function,
    so this function only has to do something special for pre-1.2 versions.
  */
  __device__ static void atomicAdd64(u64 *dest, u64 addend) {
#if __CUDA_ARCH__ >= 120
    atomicAdd(dest, addend);
#else
    unsigned *destu = (unsigned *)dest;
    unsigned addendLo = (unsigned)addend;
    unsigned addendHi = (unsigned)(addend >> 32);
    unsigned old1, sum, old2;
    int carry;

    // low word (assumes little endian)
    do {
      old1 = destu[0];
      sum = old1 + addendLo;
      old2 = atomicCAS(destu, old1, sum);
      carry = sum < addendLo;
    } while (old1 != old2);

    // high word
    do {
      old1 = destu[1];
      sum = old1 + addendHi + carry;
      old2 = atomicCAS(destu + 1, old1, sum);
    } while (old1 != old2);
#endif // arch>=120
  }

  // add and remove integer part atomically
  __device__ static void atomicAddDoubleFraction(double *dest, double addend) {
    u64 old1, old2;
    do {
      double oldVal = *dest;
      double newVal = oldVal + addend;
      newVal -= (int)newVal;
      old1 = __double_as_longlong(oldVal);
      old2 = atomicCAS((u64 *)dest, old1, __double_as_longlong(newVal));
    } while (old1 != old2);
  }

#endif // __CUDACC__
};

#endif //  __CUDA_PI__
