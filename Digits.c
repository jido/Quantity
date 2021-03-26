#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Take a number between 0 and 999 and convert it to characters
// Returns number of trailing zeros
int putDigits(int n, char *string) {
   int d0 = (n * 41) >> 12;     // hundreds using 4096 ~= 4100
   n = n - 100 * d0;            // remainder
   int d1 = (n * 205) >> 11;    // tens using 2048 ~= 2050
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
   int log10 = (integer > 0) * (log2 * 3 / 10 + 1);      // log10 ~= log2 * 0.3 -- NOTE: clzll output undefined for 0, so suppress it
   return log10 - (integer < powers10[log10]);           // use a table to adjust value
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
   long result = 0;
   result = groupDigits(integer);         // round integer *down* (truncation) and add to result
   return result * sign;                  // set the sign
}

long quantityFromString(char *string) {
   long possibles[3] = {0};
   long *result = possibles;  // initialised to 0
   long *alt1 = possibles;
   long *alt2 = possibles + 1;
   long *alt3 = possibles + 2;
   char *next;
   
   for (next = string; *next >= '0' && *next <= '9'; ++next)
   {
      int digit = *next - '0';
      *alt1 += digit;
      *alt2 += digit * 10;
      *alt3 <<= 10;           // quantity filled up till the smallest unit, need to shift before adding more digits
      *alt3 += digit * 100;
      result = alt1;
      alt1 = alt2;
      alt2 = alt3;
      alt3 = result;
   }
   return *result;
}

int main(int n, char *args[]) {
   long A, B;
   char *val = "987";
   if (n > 1)
   {
      val = args[1];
   }
   
   A = quantityFromInt(strtoll(val, NULL, 10));
   B = quantityFromString(val);
   printf("quantity: %ld or: %ld\n", A, B);
   char output[] = "Converted to string = _________E%d\n";
   char *line = strchr(output, '_');
   long expon = 0;
   long b = labs(A) & 0x3FFFFFFF;
   int group = (b >> 20) & 0x3FF;
   if (group > 0) putDigits(group, line);
   group = (b >> 10) & 0x3FF;
   if (group > 0) putDigits(group, line + 3);
   putDigits(b & 0x3FF, line + 6);
   printf(output, expon);
}

