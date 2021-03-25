# Quantity
Number format to represent quantities for display and storage

* Efficient conversion to and from text
* What you see is what you get: it's a decimal format which means _no internal rounding, no hidden digits_
* Arbitrary precision
* Supports native equality and comparison of like numbers
* Includes value for _Infinity_

## Small quantity (32 bit)

Bit layout:

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

* Numbers from _0_ to _999,999,999_
* Negative numbers stored using two's complement
* Decimal digits are stored in groups of three digits from **000** to **999**
* The _extension_ bit allows to represent numbers from a billion onwards as described below

### Examples

One
~~~
00000000000000000000000000000001
~~~

One thousand
~~~
00000000000000000000010000000000
            \___1____/\__000___/
~~~

Speed of light (299,792,458 m/s)
~~~
00010010101111000110000111001010
  \__299___/\__792___/\__458___/
~~~


## Large quantity

When the _extension bit_ is set a larger quantity is stored using as many bits as required.

As previously, each group of three digits is stored as a 10-bit integer up to **999** (_in binary:_ 1111100111)

The extended format starts with a header:

~~~
sxxxnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn
~~~

s
  = sign

x
  = extension

n
  = number of _chunks_ (44 bits)

A _chunk_ consists of eight groups of three digits for a total of **24 digits**. Each chunk occupies _80 bits = 10 bytes_.

The chunks follow immediately the header. The number of bytes occupied by the quantity is rounded up to the nearest multiple of four, which means that two bytes are wasted when _n_ is even.

## Extensions

The default extension value is **100** (binary).

The number of chunks is encoded using _44 bits_ which corresponds to a quantity with up to **4×10¹⁴ digits**.

Other extension values put some of those bits to a different use:

~~~
100 : default
101 : with exponent
110 : 64 bit floating point
111 : floating point
~~~

### Exponent extension

The format recognises a special case where all the end digits of the quantity are _zeros_.

Instead of using more chunks to write these zeros, the first 16 bits of _n_ are used to store an _exponent_ which indicates the number of trailing zeros.

Header bit layout:

~~~
sxxxeeeeeeeeeeeeeeeennnnnnnnnnnnnnnnnnnnnnnnnnnn
~~~

s
  = sign

x
  = extension

e
  = exponent

n
  = number of _chunks_

**Example:**

Advogadro constant (6.02214076×10²³)
~~~
010100000000000010000000000000000000000000000001
    \__exponent____/\_____number of chunks_____/
10010110100011010110000100110000000000000000000000000000000000000000000000000000
\__602___/\__214___/\__076___/
~~~

### Floating point extension

The above exponent extension could be used to write floating point quantities by deciding a fixed position for the decimal point and adjust the exponent by that much (_exponent bias_). 

However, that is not an efficient method if the quantities have a wide range of magnitude.

Instead it is preferable to fix the position of the decimal point _after the first digit_ and use chunks for the fractional part only.

In the floating point extension, the exponent is immediately followed by the first digit of the quantity.

Bit layout:

~~~
64 bit:
sxxxeeeeeeeeeeeeeeeeddddmmmmmmmmmmµµµµµµµµµµppppppppppnnnnnnnnnn

Variable length:
sxxxeeeeeeeeeeeeeeeeddddnnnnnnnnnnnnnnnnnnnnnnnn ...
~~~

s
  = sign

x
  = extension (110 or 111)

e
  = exponent

d
  = first digit (1 to 9)

m
  = millis

µ
  = micros

p
  = picos

n
  = nanos (64 bit) or number of chunks

**Example:**

Electron mass at rest (9.1093837015×10⁻³¹ kg)
~~~
~~~
