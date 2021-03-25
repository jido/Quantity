# Quantity
Number format to represent quantities for display and storage

* Efficient conversion to and from text
* What you see is what you get: it's a decimal format which means _no internal rounding, no hidden digits_
* Arbitrary precision
* Supports native equality and comparison of like numbers
* Includes value for _Infinity_

## Small quantity (32 bit)

**Bit layout**

~~~
sxmmmmmmmmmmkkkkkkkkkkuuuuuuuuuu
~~~

s
  = sign

x
  = extension bit (0)

m
  = millions

k
  = thousands

u
  = units

* Numbers from _0_ to _999,999,999_
* Infinity has all bits set except for sign
* Negative quantities stored using two's complement
* If only the sign bit is set it indicates an invalid quantity (NaN)
* Decimal digits are stored in groups of three digits from **000** to **999**
* The _extension_ bit allows to represent quantities from a billion onwards as described below

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

Positive infinity
~~~
01111111111111111111111111111111
~~~

## Large quantity (variable length)

When the _extension bit_ is set a larger quantity is stored using as many bits as required.

As previously, each group of three digits is stored as a 10-bit integer up to **999** (_in binary:_ 1111100111)

The extended format starts with a **header**:

~~~
sxxxnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn
~~~

s
  = sign

x
  = extension bits

n
  = number of _chunks_ (44 bits)

A _chunk_ consists of eight groups of three digits for a total of **24 digits**. Each chunk occupies _80 bits = 10 bytes_ in memory.

The chunks follow immediately the header. The number of bytes occupied by the quantity is rounded up to the nearest multiple of four, which means that two bytes padding are added when _n_ is even.

## Extensions

The default extension value is **100** (binary).

The number of chunks is encoded using _44 bits_ which corresponds to a quantity with up to **4×10¹⁴ digits**.

Other extension values put some of those bits to a different use:

~~~
100 : default
101 : with exponent
110 : 64 bit floating point
111 : variable length floating point
~~~

### Exponent extension

The format recognises a special case where all the end digits of the quantity are _zeros_.

Instead of using more chunks to write these zeros, the first 16 bits of _n_ are used to store an _exponent_ which indicates the number of trailing zeros.

**Header bit layout**

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

**Example**

Advogadro constant (6.02214076×10²³)
~~~
010100000000000010000000000000000000000000000001
    \__exponent____/\_____number of chunks_____/
10010110100011010110000100110000000000000000000000000000000000000000000000000000
\__602___/\__214___/\__076___/
~~~

### Floating point extension

The above exponent extension could be used to write floating point quantities by deciding a maximum number of figures after the decimal point and adjust the exponent by that much (_exponent bias_). 

However, that is not an efficient method if the quantities have a wide range of magnitude.

Instead it is preferable to fix the position of the decimal point _after the first digit_ and use chunks for the fractional part only.

In the floating point extension, the exponent is immediately followed by the first digit of the quantity.

**Bit layout**

~~~
64 bit:
sxxxeeeeeeeeeeeeeeeeddddmmmmmmmmmmµµµµµµµµµµnnnnnnnnnnpppppppppp

Variable length header:
sxxxeeeeeeeeeeeeeeeeddddnnnnnnnnnnnnnnnnnnnnnnnn
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
  = nanos _(64 bit)_ or number of chunks _(variable length)_

**Example**

Electron mass at rest (9.1093837015×10⁻³¹ kg)
~~~
0110011111111110000110010001101101010111111110101111010111110100
    \___exponent___/\9_/\__109___/\__383___/\__701___/\__500___/
~~~

### Variations

An implementation could allow only _fixed length_ quantities, which are **32 bit small quantity** and **64 bit floating point quantity**. In case of a 32 bit quantity a second value could be added to write a _fraction (x/y)_.

If _28 bits_ are always enough to store the number of chunks, the default extended format without exponent can be dropped altogether. Then quantities can have up to **6×10⁹ non-zero digits**.

If there is no need for an _explicit fractional part_, the variable length floating point extension can be dropped. The 64 bit floating point extension can be made to behave like the exponent extension by using _exponent bias = +12_.

# Practical considerations

## Endianness

Quantities should always be in **big endian** format for storage or exchange. This is because the textual representation of a quantity puts the most significant digit first.

However, it is acceptable to use **little endian** format for local usage. The byte length of a quantity is always a multiple of _four_, so it is practical to handle the data as a sequence of _32 bit values_ (four bytes) using the platform natural endianness.

## Compression

The format does not define a compression scheme.

If the data contains patterns (repeated digits or sequence of digits), it is likely it will benefit from compression. This can be applied on top of the format as seen fit.

## Normalisation

To make comparison more efficient quantities should be normalised, which generally means they should be written using as less bytes as possible.

## Operations

Quantities do not define any operation other than equality, comparison and conversion to and from other formats such as text.

One reason is that combining quantities with very different magnitude can result in extreme memory consumption, for example _10¹⁰⁰⁰⁰⁰ + 1_ does not have a space-efficient representation.
