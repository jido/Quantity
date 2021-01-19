# Quantity
Number format to represent quantities for display and storage

* Efficient conversion to and from text
* What you see is what you get: it's a decimal format so no internal rounding, no hidden digits
* Supports native equality and comparison
* Also suitable for fixed point, rational numbers (using two quantities), floating point (with additional byte for exponent)
* Numbers from 0 to 1,000,000,000 stored exactly
* Larger numbers in reduced precision, including a value for Infinity

## Format

Uses 32 bits

~~~
sxmmmmmmmmmmkkkkkkkkkkuuuuuuuuuu
~~~

s
  = sign

x
  = extension

m
  = millions

k
  = thousands

u
  = units

* Negative numbers stored using two's complement
* Decimal digits are stored in groups of 3 digits from 000 to 999
* The extension bit allows to represent numbers from a billion onwards as described below

### Examples

One
~~~
00000000000000000000000000000001
~~~

A thousand
~~~
00000000000000000000010000000000
~~~

Speed of light (299,792,458 m/s)
~~~
00010010101111000110000111001010
  \__299___/\__792___/\__458___/
~~~

A billion
~~~
        /10\
01000000101000000000000000000000
 ^ extension bit
~~~

Advogadro constant (6.02214×10²³)
~~~
01111111000010010110100011010110
            \__602___/\__214___/
~~~

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
eeemmmmmmm
~~~

e
  = exponent (10ⁿ)

m
  = millions, ranges from 0 to 99

The exponent ranges from 2 to 8 (`000` to `110`).

The exponent bit pattern `111` is reserved to further extend the quantity by dropping one more decimal digit.
The next extension uses again a three-bit exponent pattern:

~~~
111eeemmmm
~~~

e
  = exponent (10ⁿ) ranges from 10 to 16

m
  = millions, ranges from 0 to 9

The exponent bit pattern `111` is reserved to further extend the quantity by dropping one more decimal digit.
The next extension uses a four-bit exponent pattern:

~~~
111111eeee
~~~

e
  = exponent (10ⁿ) ranges from 18 to 32

At this point there are no bits left to store millions. The quantity only stores units and thousands.

The same sequence of 3, 3 and 4 bits for the exponent repeats for each group of three decimal digits (thousands and units).

The final extension looks like:

~~~
0111111111111111111111111111eeee
~~~

There is no space left to store decimal digits, so the quantity just corresponds to a unit of magnitude 10ⁿ where n goes up to 95.

The exponent bit pattern `1111` is reserved for Infinity.

# Small quantity and large quantity

## Going small

The same design could be applied to 16 bit values, with 14 bits used to store numbers from 0 to 9,999 and an extension bit for larger values:

~~~
sxkkkkuuuuuuuuuu
~~~

s
  = sign

x
  = extension

k
  = thousands

u
  = units

With that scheme, there are three decimal digits avalable to express the speed of light. The speed of light rounded up to three digits is 3×10⁸:

~~~
0101000100101100
      \__300___/
~~~

There are two decimal digits available to express the Advogadro constant which rounds down to 6×10²³:

~~~
0100001000111100
         \_60__/
~~~

Although the quantities are approximated it is still remarkable that 16 bit is enough to store them.

## Going large

With a 64 bit value only **60** bits are usable to store decimal digits. With a 128 bit value only **124** bits are usable to store decimal digits.

One bit is needed to store the sign, so in both cases there are three bits left for the extension:

~~~
sxxxppppppppppttttttttttggggggggggmmmmmmmmmmkkkkkkkkkkuuuuuuuuuu
~~~

s
  = sign
  
x
  = extension

p
  = quadrillions

t
  = trillions

g
  = billions

m
  = millions

k
  = thousands

u
  = units

One way to take advantage of these extra bits is to encode _negative_ exponents which gives a fractional part to the quantity (floating point).

If the extension bits are `000` then the exponents go _down_. If the extension bits are `111` then the exponents go _up_.

Values in-between can be used as exponent themselves without requiring an extension of range.

### Examples

Let's use non-extended exponent values 0 to 5, so that integer quantities up to a quintillion can be written exactly:

One (15 significant figures)
~~~
0000000000011010001100100000000000000000000000000000000000000000
               \__100___/
~~~

Hundred quadrillions (18 significant figures)
~~~
0001000110010000000000000000000000000000000000000000000000000000
    \__100___/
~~~

Advogadro constant (6.02214076×10²³)
~~~
0111000011110000110111010110010111100101100000000000000000000000
       \_60__/\__221___/\__407___/\__600___/
~~~

Electron rest mass (9.1093837015×10⁻³¹ kg)
~~~
0000000000000000000010111110001110111010101001011100100010010110
                        \__910___/\__938___/\__370___/\__150___/
~~~
