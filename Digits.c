#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Take a number between 0 and 999 and convert it to characters
// Returns number of trailing zeros
int putDigits(int n, char *string) {
   int d0 = (n * 41) >> 12;     // hundreds using 4096 ~= 4100
   n = n - 100 * d0;            // remainder
   int d1 = (n * 51 + 25) >> 9; // tens using 512 ~= 510 and rounding
   int d2 = n - 10 * d1;        // remainder
   string[0] = '0' + d0;
   string[1] = '0' + d1;
   string[2] = '0' + d2;
   return (d0 == 0 && d1 == 0 && d2 == 0) + (d1 == 0 && d2 == 0) + (d2 == 0);
}

unsigned long long powers10[] = {
   1, 10, 100, 1000, 10000, 100000ll, 1000000ll, 10000000ll, 100000000ll, 1000000000ll, 10000000000ll,
   100000000000ll, 1000000000000ll, 10000000000000ll, 100000000000000ll, 
   1000000000000000ll,
   10000000000000000ll,
   100000000000000000ll, 
   1000000000000000000ll, 
   10000000000000000000ull
};

int log10int(long long integer) {
   int llong_bit = sizeof(long long) * 8;                // assume 8 bit per byte
   int log2 = llong_bit - __builtin_clzll(integer) - 1;  // log2(n) ~= 63 - (count of leading zeros)
   int log10 = (log2 * 3 / 10 + 1) * (integer > 0);      // log10 ~= log2 * 0.3 -- NOTE: clzll output undefined for 0, so suppress it
   //printf("Log2=%d Log10=%d adjust? %c\n", log2, log10, '=' + (integer < powers10[log10]));
   return log10 - (integer < powers10[log10]);           // use a table to adjust value
}

inline int extended_exp(int n) {
   //printf("log10 = %d\n", n+1);
   int factor29 = (n * 141) >> 10;     // use 4096 ~= 141*29 and some truncation magic for
   // series 2...8,  10...16, 18...32, 34...40, 42...48, 50...64, 66...72, 74...80, 82...96
   return (n > 7) * (n - 7 + factor29 - (factor29 >> 2));
}

inline int exp_offset(int n) {
   int factor29 = (n * 141) >> 10;     // use 4096 ~= 141*29 and extended_exp() for
   // series 0...6,  0...6, 0...14, 0...6, 0...6, 0...14, 0...6, 0...6, 0...14
   return (n > 7) * (8 * (factor29 - ((factor29 & 3) == 0)) - 6);
}

inline int exp_position(int n) {
   int factor29 = (n * 141) >> 10;     // use 4096 ~= 141*29 and some truncation magic for
   // series 27, ..., 27, 24, ..., 24, 20, ..., 17, ..., 14, ..., 10, ..., 7, ..., 4, ..., 0, ...
   return 30 - 10 * (factor29 - (factor29 >> 2)) / 3;
}

inline int exp_size(int n) {
   int factor29 = (n * 141) >> 10;     // use 4096 ~= 141*29 and some truncation magic for
   // series 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, ...
   return 4 - (((factor29 + 1) >> 1) & 1);
}

long groupDigits(long integer) {
   long div = integer / 1000;
   long groups = integer - div * 1000;
   integer = div;
   div /= 1000;
   groups |= (integer - div * 1000) << 10;
   integer = div;
   div /= 1000;
   groups |= (integer - div * 1000) << 20;
   return groups;
}

long quantityFromInt(long long integer) {
   // Note: calculations are designed to be close to constant time
   int sign = (integer >= 0) - (integer < 0);
   integer *= sign;                       // make it a positive value
   int elog = log10int(integer) - 1;
   int eexp = extended_exp(elog);
   int offs = eexp - exp_offset(elog);
   int epos = exp_position(elog);
   int hipos = epos + exp_size(elog);
   printf("Exponent offset: %d position: %d then: %d\n", offs, epos, hipos);
   unsigned long long pext = powers10[eexp];
   long result = 0;
   result |= (eexp > 0) * (0x7FFFFFFF ^ ((1 << hipos) - 1));   // fill in extension bits
   printf("filling in: %lx\n", result);
   result |= offs << epos;                // insert exponent offset
   result |= groupDigits(integer / pext); // round integer *down* (truncation) and add to result
   return result * sign;                  // set the sign
}

int main(int n, char *args[]) {
   long A = 987;

   if (n > 1)
   {
      char *val = args[1];
       A = strtoll(val, NULL, 10);
   }
   A = quantityFromInt(A);
   printf("quantity: %ld\n", A);
   char output[] = "Converted to string = _________E%d\n";
   char *line = strchr(output, '_');
   long expon = 0;
   long b = labs(A) & 0x3FFFFFFF;
   if (b != labs(A))
   {
      int base_exp[] = {2, 10, 18, 32, 40, 48, 62, 80, 88, 96};
      int llong_bit = sizeof(long long) * 8;       // assume 8 bit per byte
      int xlog = llong_bit - __builtin_clzll(0x3FFFFFFF - b) - 1;
      int group = xlog / 10;
      int rem = xlog - 10 * group;
      int subgroup = rem * 3 / 10;                 // 0...3 -> subgroup 0, 4...6 -> subgroup 1, 7...9 -> subgroup 2
      int shift = group * 10 + 3 * subgroup + (subgroup > 0);  // 10*group + 0, 4, 7
      int sexp = ((b >> shift) & (15 >> (subgroup > 0)))       // stored exponent bits
      expon = base_exp[8 - 3 * group - subgroup] + sexp;
      b &= (1 << shift) - 1;
   }
   putDigits((b >> 20) & 0x3FF, line);
   putDigits((b >> 10) & 0x3FF, line + 3);
   putDigits(b & 0x3FF, line + 6);
   printf(output, expon);
}

