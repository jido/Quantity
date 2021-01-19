# Quantity
Number format to represent quantities for display and storage

* Efficient conversion to and from text
* Supports equality and comparison
* Also suitable for fixed point, rational numbers (using two quantities), floating point (with additional byte for exponent)
* Numbers from 0 to 1,000,000,000 stored exactly
* Larger numbers in reduced precision

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

* Negative numbers stored using two's complement. INT_MIN is Infinity
* Decimal digits are stored in groups of 3 digits from 000 to 999
* The extension bit allows to represent numbers larger than a billion as described below
