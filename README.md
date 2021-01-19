# Quantity
Number format to represent quantities for display and storage

* Efficient conversion to and from text
* Supports equality and comparison
* Also suitable for fixed point, rational numbers (using two quantities), floating point (with additional byte for exponent)
* Numbers from 0 to 1,000,000,000 stored exactly
* Larger numbers in reduced precision, including a value for Infinity

## Format

Uses 32 bits

~~~
semmmmmmmmmmkkkkkkkkkkuuuuuuuuuu
~~~

s
  = sign

e
  = extension

m
  = millions

k
  = thousands

u
  = units

* Negative numbers stored using two's complement. `INT_MIN` is Not A Number
* Decimal digits are stored in groups of 3 digits from 000 to 999
* The extension bit allows to represent numbers larger than a billion as described below

### Extended quantity

When the extension bit is set, a larger quantity is stored in reduced precision.

To explain how this works one should look at how decimal digits are stored.

Each group of three digits is stored as a 10-bit integer. Binary representation:

~~~
    0-9 : 0000000000 - 0000001001
  10-99 : 0000001010 - 0001100011
100-999 : 0001100100 - 1111100111
~~~

To extend the range of numbers, the head decimal digit is dropped. This frees up 3 bits which are used to store an exponent:

~~~
xxxmmmmmmm
~~~

x
  = exponent (10ⁿ)

m
  = millions, ranges from 0 to 99

The exponent ranges from 2 to 8 (`000` to `110`).

The exponent bit pattern `111` is reserved to further extend the quantity by dropping one more decimal digit.
The next extension uses again a three-bit exponent pattern:

~~~
111xxxmmmm
~~~

x
  = exponent (10ⁿ) ranges from 10 to 16

m
  = millions, ranges from 0 to 9

The exponent bit pattern `111` is reserved to further extend the quantity by dropping one more decimal digit.
The next extension uses a four-bit exponent pattern:

~~~
111111xxxx
~~~

x
  = exponent (10ⁿ) ranges from 18 to 32

At this point there are no bits left to store millions. The quantity only stores units and thousands.

The same sequence of 3, 3 and 4 bits for the exponent repeats for each group of three decimal digits (thousands and units).

The final extension looks like:

~~~
0111111111111111111111111111xxxx
~~~

There is no space left to store decimal digits, so the quantity just corresponds to a unit of magnitude 10ⁿ where n goes up to 95.

The exponent bit pattern `1111` is reserved for Infinity.
